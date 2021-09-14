/*------------------------------------ client_main.c -----------------------------------*/
/* Description: This source file contains all the functions that are used by the client */
/*--------------------------------------------------------------------------------------*/

/* ----------------- */
/* Project includes: */
/* ----------------- */
#include "client_main.h"


/* ------------------------ */
/* function implemantations */
/* ------------------------ */


int client_main(const char *ip_addr, const char *port_num_str, const char *user_name)
{
	int wsa_start_res = 0, port_num = 0, connection;
	BOOL client_connected = FALSE, done = FALSE;
	client_stage stage = ESTABLISH_CONNECTION;

	if ((port_num = check_port_num(port_num_str)) == -1)
		return EXIT_FAILURE;

	// Init WSA
	if (EXIT_FAILURE == init_winsock())	return EXIT_FAILURE;

	// Create socket for the first time
	SOCKET m_socket = create_socket();
	if (m_socket == INVALID_SOCKET) {
		if (SOCKET_ERROR == WSACleanup())	printf_s("Could not de-initialize WSA, with error %ld\n", WSAGetLastError());
		return EXIT_FAILURE;
	}

	// Connect to the server for the first time
	connection = client_setup_connection(ip_addr, port_num_str, port_num, m_socket, stage, TRUE);
	if (connection != CLIENT_CONTINUE) {
		if (close_socket_and_deinit_wsa(m_socket))	return EXIT_FAILURE;
		return EXIT_SUCCESS;
	}
	printf_s("Connected to server on %s:%s\n", ip_addr, port_num_str);

	// Set socket timeout for sending and receiving information
	if ((set_socket_timeout(m_socket, CLIENT_SEND_TIMEOUT, TRUE)) || (set_socket_timeout(m_socket, CLIENT_RECV_TIMEOUT, FALSE))) {
		if (close_socket_and_deinit_wsa(m_socket))	return EXIT_FAILURE;	// deinit_winsock() prints the error
		return EXIT_SUCCESS;
	}

	// Start broadcasting in and out
	while (!done) {
		stage = client_play_game(m_socket, stage, user_name);
		if (stage == CLIENT_ERROR || stage == CLIENT_WANT_OUT)	break;
		if (INVALID_SOCKET == (m_socket = close_and_create_socket(m_socket))) return EXIT_FAILURE;
		connection = client_setup_connection(ip_addr, port_num_str, port_num, m_socket, stage, FALSE);
		stage = ESTABLISH_CONNECTION;	// if SERVER_DENIED has occured, go back to the establish_connection stage after printing the right message
		if (connection != CLIENT_CONTINUE) { 
			if (close_socket_and_deinit_wsa(m_socket))	return EXIT_FAILURE;
			return EXIT_SUCCESS;
		}
		printf_s("Connected to server on %s:%s\n", ip_addr, port_num_str);
	}
	if (close_socket_and_deinit_wsa(m_socket))	return EXIT_FAILURE;
	if (stage == CLIENT_ERROR)					return EXIT_FAILURE;
	else										return EXIT_SUCCESS;
}
//	----------------------------------------------------------------------------------------
int client_setup_connection(const char *ip_addr, const char *port_num_str , const int port_num, SOCKET m_socket, client_stage stage, BOOL first_time)
{
	BOOL client_connected = FALSE;
	char user_choice[MAX_USER_CHOICE_STR_LEN];
	non_blocking_ret_val connection_ret_val;

	if (stage == ESTABLISH_CONNECTION && (!first_time)) {
		printf_s("Failed connecting to server on %s:%s.\nChoose what to do next:\n1. Try to reconnect\n2. Exit\n", ip_addr, port_num_str);
		gets_s(user_choice, MAX_USER_CHOICE_STR_LEN);
		if ((STRINGS_ARE_EQUAL(user_choice, "2"))) {
			return CLIENT_EXIT_REQ;
		}
	}
	else if ((stage == SERVER_APPROVAL) && (!first_time)) {
		printf_s("Server on %s:%s denied the connection request.\nChoose what to do next:\n1. Try to reconnect\n2. Exit\n", ip_addr, port_num_str);
		gets_s(user_choice, MAX_USER_CHOICE_STR_LEN);
		if ((STRINGS_ARE_EQUAL(user_choice, "2"))) {
			return CLIENT_EXIT_REQ;
		}
	}

	while (client_connected == FALSE) {
		connection_ret_val = connect_non_blocking(m_socket, ip_addr, (u_short)port_num);
		if (connection_ret_val == CONNECTION_TIMEOUT || connection_ret_val == CONNECTION_FAILED) {
			if (stage == ESTABLISH_CONNECTION)	printf_s("Failed connecting to server on %s:%s.\nChoose what to do next:\n1. Try to reconnect\n2. Exit\n", ip_addr, port_num_str);
			else if (stage == SERVER_APPROVAL)	printf_s("Server on %s:%s denied the connection request.\nChoose what to do next:\n1. Try to reconnect\n2. Exit\n", ip_addr, port_num_str);
			gets_s(user_choice, MAX_USER_CHOICE_STR_LEN);
			if ((STRINGS_ARE_EQUAL(user_choice, "2"))) {
				return CLIENT_EXIT_REQ;
			}
		}
		else if (connection_ret_val == CONNECTION_ERROR) {
			return CLIENT_FORCE_EXIT;
		}
		else client_connected = TRUE;
	}
	return CLIENT_CONTINUE;
}
//	----------------------------------------------------------------------------------------
client_stage client_play_game(SOCKET m_socket, client_stage stage, const char *user_name)
{
	BOOL done = FALSE;
	stage = SERVER_APPROVAL;
	char opponent_user_name[MAX_USERNAME_LEN];
	while (!done) {
		stage = make_move(stage, m_socket, user_name, opponent_user_name);
		if (stage == CLIENT_WANT_OUT || stage == CLIENT_ERROR || stage == SERVER_APPROVAL || stage == ESTABLISH_CONNECTION)
			done = TRUE;
	}

	return stage;
}
//	----------------------------------------------------------------------------------------
client_stage make_move(client_stage cur_stage, SOCKET m_socket, const char *param, char *opponent_user_name)
{
	char *accepted_str = NULL, message_type[MAX_MSG_LEN], param1[MAX_MSG_LEN], param2[MAX_MSG_LEN], param3[MAX_MSG_LEN], param4[MAX_MSG_LEN], user_choice[MAX_USER_CHOICE_STR_LEN], **p_accepted = &accepted_str;
	t_transfer_result comm_res;
	int set_res;

	switch (cur_stage) {

	case (SERVER_APPROVAL):	// #5
		if (TRNS_FAILED == (comm_res = communicate_with_server(m_socket, cur_stage, param, p_accepted)))		return CLIENT_ERROR; /* Fatal error */
		else if (TRNS_DISCONNECTED == comm_res)																	return ESTABLISH_CONNECTION; /* Disconnection */

		parse_message(accepted_str, message_type, param1, param2, param3, param4);
		if (STRINGS_ARE_EQUAL("SERVER_DENIED", message_type)) { printf_s("%s Disconnecting.\n", param1);		return ESTABLISH_CONNECTION; }
		else if (STRINGS_ARE_EQUAL("SERVER_APPROVED", message_type))											return DISPLAY_MAIN_MENU;
		else if (STRINGS_ARE_EQUAL("SERVER_OPPONENT_QUIT", message_type))										return OPPONENT_QUIT;
		break;

	case (DISPLAY_MAIN_MENU): // #6
		
		if (TRNS_FAILED == (comm_res = communicate_with_server(m_socket, cur_stage, NULL, p_accepted)))		return CLIENT_ERROR; /* Fatal error */
		else if (TRNS_DISCONNECTED == comm_res)																	return ESTABLISH_CONNECTION; /* Disconnection */
		
		parse_message(accepted_str, message_type, param1, param2, param3, param4);
		
		if (STRINGS_ARE_EQUAL("SERVER_MAIN_MENU", message_type)) {
			printf_s("Choose what to do next:\n1. Play against another client\n2. Quit\n");						
			if (NULL == gets_s(user_choice, MAX_USER_CHOICE_STR_LEN)) { printf_s("Wrong input choice. Terminating...");	return CLIENT_ERROR; }
			if (STRINGS_ARE_EQUAL("2", user_choice)) {
				cur_stage = CLIENT_WANT_OUT;
				accepted_str = NULL;
				if (TRNS_FAILED == (comm_res = communicate_with_server(m_socket, cur_stage, NULL, p_accepted)))		return CLIENT_ERROR; /* Fatal error */
				else if (TRNS_DISCONNECTED == comm_res)																	return ESTABLISH_CONNECTION; /* Disconnection */
				else return CLIENT_WANT_OUT;
			}
			return WAIT_OPPONENT;
		}
		if (STRINGS_ARE_EQUAL("SERVER_OPPONENT_QUIT", message_type))											return DISPLAY_MAIN_MENU;
		break;

	case (WAIT_OPPONENT): // #7
		if (set_res = set_socket_timeout(m_socket, CLIENT_RECV_TIMEOUT * 2, FALSE))								return CLIENT_ERROR;
		if (TRNS_FAILED == (comm_res = communicate_with_server(m_socket, cur_stage, NULL, p_accepted)))			return CLIENT_ERROR; /* Fatal error */
		else if (TRNS_DISCONNECTED == comm_res)																	return ESTABLISH_CONNECTION; /* Disconnection */
		if (set_res = set_socket_timeout(m_socket, CLIENT_RECV_TIMEOUT, FALSE))									return CLIENT_ERROR;

		parse_message(accepted_str, message_type, param1, param2, param3, param4);
		if (STRINGS_ARE_EQUAL("SERVER_NO_OPPONENTS", message_type))												return DISPLAY_MAIN_MENU;
		else if (STRINGS_ARE_EQUAL("SERVER_INVITE", message_type)) { printf_s("Game is on!\n"); strcpy_s(opponent_user_name, MAX_USERNAME_LEN, param1);				return WAIT_FOR_SETUP_REQUEST; }
		else if (STRINGS_ARE_EQUAL("SERVER_OPPONENT_QUIT", message_type))										return DISPLAY_MAIN_MENU;
		break;

	case (WAIT_FOR_SETUP_REQUEST): // #9
		if (TRNS_FAILED == (comm_res = communicate_with_server(m_socket, cur_stage, NULL, p_accepted)))		return CLIENT_ERROR; /* Fatal error */
		else if (TRNS_DISCONNECTED == comm_res)																	return ESTABLISH_CONNECTION; /* Disconnection */
		parse_message(accepted_str, message_type, param1, param2, param3, param4);

		if (STRINGS_ARE_EQUAL("SERVER_SETUP_REQUEST", message_type)) { printf_s("Choose your 4 digits:");		return CHOOSE_FOUR_DIGITS; }
		else if (STRINGS_ARE_EQUAL("SERVER_OPPONENT_QUIT", message_type))										return DISPLAY_MAIN_MENU;
		break;

	case (CHOOSE_FOUR_DIGITS):	// #9 too
		if (NULL == gets_s(user_choice, MAX_USER_CHOICE_STR_LEN)) { printf_s("Wrong input choice. Terminating...");	return CLIENT_ERROR; }
		strcpy_s(param1, MAX_USER_CHOICE_STR_LEN, user_choice);
		if (TRNS_FAILED == (comm_res = communicate_with_server(m_socket, cur_stage, param1, p_accepted)))		return CLIENT_ERROR; /* Fatal error */
		else if (TRNS_DISCONNECTED == comm_res)																	return ESTABLISH_CONNECTION; /* Disconnection */

		return GUESS_OPPONENTS_DIGITS;
		break;

	case (GUESS_OPPONENTS_DIGITS):
		if (set_res = set_socket_timeout(m_socket, WAIT_TIME_FOR_HUMAN, FALSE))									return CLIENT_ERROR;
		if (TRNS_FAILED == (comm_res = communicate_with_server(m_socket, cur_stage, param1, p_accepted)))		return CLIENT_ERROR; /* Fatal error */
		else if (TRNS_DISCONNECTED == comm_res)																	return ESTABLISH_CONNECTION; /* Disconnection */
		if (set_res = set_socket_timeout(m_socket, CLIENT_RECV_TIMEOUT, FALSE))									return CLIENT_ERROR;
		
		parse_message(accepted_str, message_type, param1, param2, param3, param4);
		if (STRINGS_ARE_EQUAL("SERVER_PLAYER_MOVE_REQUEST", message_type)) { printf_s("Choose your guess:");	return SEND_GUESS_TO_SERVER_AND_GET_ANSWER; }
		else if (STRINGS_ARE_EQUAL("SERVER_OPPONENT_QUIT", message_type)) { printf_s("Opponent quit.\n"); return DISPLAY_MAIN_MENU; }
		else if (STRINGS_ARE_EQUAL("SERVER_WIN", message_type)) {
			printf_s("%s won!\nopponents number was %s\n", param1, param2);										return DISPLAY_MAIN_MENU;
		}
		else if (STRINGS_ARE_EQUAL("SERVER_DRAW", message_type)) {
			printf_s("It's a tie\n");																			return DISPLAY_MAIN_MENU;
		}

	case (SEND_GUESS_TO_SERVER_AND_GET_ANSWER):
		if (NULL == gets_s(user_choice, MAX_USER_CHOICE_STR_LEN)) { printf_s("Wrong input choice. Terminating...\n");	return CLIENT_ERROR; }
		strcpy_s(param1, MAX_USER_CHOICE_STR_LEN, user_choice);

		if (set_res = set_socket_timeout(m_socket, WAIT_TIME_FOR_HUMAN, FALSE))									return CLIENT_ERROR;
		if (TRNS_FAILED == (comm_res = communicate_with_server(m_socket, cur_stage, param1, p_accepted)))		return CLIENT_ERROR; /* Fatal error */
		else if (TRNS_DISCONNECTED == comm_res)																	return ESTABLISH_CONNECTION; /* Disconnection */
		if (set_res = set_socket_timeout(m_socket, CLIENT_RECV_TIMEOUT, FALSE))									return CLIENT_ERROR;

		parse_message(accepted_str, message_type, param1, param2, param3, param4);
		if (STRINGS_ARE_EQUAL("SERVER_GAME_RESULTS", message_type)) {
			printf_s("Bulls: %s\nCows: %s\n%s played: %s\n", param1, param2, param3, param4);					return GUESS_OPPONENTS_DIGITS; }

		else if (STRINGS_ARE_EQUAL("SERVER_OPPONENT_QUIT", message_type))										return DISPLAY_MAIN_MENU;
		break;
	}
	
	free(accepted_str);
	return cur_stage;
}
//	----------------------------------------------------------------------------------------
t_transfer_result communicate_with_server(SOCKET m_socket, client_stage stage, const char *param, char **accepted_str)
{
	t_transfer_result send_res, receive_res;
	char *str_to_send = malloc(CLIENT_MAX_MSG_LEN * sizeof(char));
	if (str_to_send == NULL) {
		printf_s("Failed allocating room in memory for a message buffer. Terminating...\n");
		return EXIT_FAILURE;
	}
	if (EXIT_FAILURE == client_format_message(stage, str_to_send, param))	return TRNS_FAILED;

	
	if ((stage != DISPLAY_MAIN_MENU) && (stage != WAIT_FOR_SETUP_REQUEST) && (stage != GUESS_OPPONENTS_DIGITS)) {
		send_res = send_string(str_to_send, m_socket);
		if (send_res == TRNS_FAILED) {
			free(str_to_send);
			free(*accepted_str);
			return TRNS_FAILED;
		}
		else if (send_res == TRNS_DISCONNECTED) {
			free(str_to_send);
			free(*accepted_str);
			return TRNS_DISCONNECTED;
		}
	}

	if ((stage != CLIENT_WANT_OUT) && (stage != CHOOSE_FOUR_DIGITS)) {
		receive_res = receive_string(accepted_str, m_socket);
		if (receive_res == TRNS_FAILED)
		{
			free(str_to_send);
			free(*accepted_str);
			return TRNS_FAILED;
		}
		else if (receive_res == TRNS_DISCONNECTED)
		{
			free(str_to_send);
			free(*accepted_str);
			return TRNS_DISCONNECTED;
		}
	}

	free(str_to_send);
	return TRNS_SUCCEEDED;
}
//	----------------------------------------------------------------------------------------
int client_format_message(client_stage stage, char *send_buffer, const char *param1)
{
	switch (stage) {
	case (SERVER_APPROVAL):
		if (param1 == NULL) {
			printf_s("NULL paramater1 sent. Terminating...\n");
			return EXIT_FAILURE;
		}
		sprintf_s(send_buffer, CLIENT_MAX_MSG_LEN, "CLIENT_REQUEST:%s\n", param1);
		break;
	case (WAIT_OPPONENT):
		sprintf_s(send_buffer, CLIENT_MAX_MSG_LEN, "CLIENT_VERSUS\n");
		break;
	case (CHOOSE_FOUR_DIGITS):
		if (param1 == NULL) {
			printf_s("NULL paramater1 sent. Terminating...\n");
			return EXIT_FAILURE;
		}
		sprintf_s(send_buffer, CLIENT_MAX_MSG_LEN, "CLIENT_SETUP:%s\n", param1);
		break;
	case (SEND_GUESS_TO_SERVER_AND_GET_ANSWER):
		if (param1 == NULL) {
			printf_s("NULL paramater1 sent. Terminating...\n");
			return EXIT_FAILURE;
		}
		sprintf_s(send_buffer, CLIENT_MAX_MSG_LEN, "CLIENT_PLAYER_MOVE:%s\n", param1);
		break;
	default:
		sprintf_s(send_buffer, CLIENT_MAX_MSG_LEN, "CLIENT_NOP\n");
		break;
	}
	return EXIT_SUCCESS;
}
//	----------------------------------------------------------------------------------------
