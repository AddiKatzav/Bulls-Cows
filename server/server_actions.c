/*--------------------------------- server_actions.c -----------------------------------*/
/* Description: This module contains all the stages that need to be made at the server  */
/*				side of the program, with a few extra auxiliary functions				*/
/*--------------------------------------------------------------------------------------*/

/* ----------------- */
/* Project includes: */
/* ----------------- */
#include "server_actions.h"


/* ------------------------ */
/* function implemantations */
/* ------------------------ */

server_stage stage_get_username_send_approval(SOCKET client_socket, server_stage stage, char msg_tokens[][MAX_MSG_LEN], char received_user_name[])
{
	char *accepted_str = NULL;
	char **p_accepted = &accepted_str;
	t_transfer_result comm_res;


	if (TRNS_DISCONNECTED == (comm_res = recv_msg_from_client(client_socket, stage, p_accepted)))	return SERVER_ERROR;
	if (EXIT_FAILURE == parse_message(accepted_str, msg_tokens[0], msg_tokens[1], msg_tokens[2], msg_tokens[3], msg_tokens[4])) { free(accepted_str); return SERVER_ERROR; }
	free(accepted_str);
	*p_accepted = NULL;
	if (STRINGS_ARE_EQUAL("CLIENT_REQUEST", msg_tokens[0])) {
		strcpy_s(received_user_name, MAX_USERNAME_LEN, msg_tokens[1]);
		if (TRNS_DISCONNECTED == (comm_res = send_msg_to_client(client_socket, stage, msg_tokens, FALSE, NULL, NULL, NULL, NULL)))	return SERVER_ERROR;
		return SEND_DISPLAY;
	}
	else if (STRINGS_ARE_EQUAL("CLIENT_DISCONNECT", msg_tokens[0]))
		return SHOW_CLIENT_OUT;
	else {
		printf_s("Wrong message type!\n");
		return SERVER_ERROR;
	}
	
}
//	----------------------------------------------------------------------------------------
server_stage stage_send_menu_display(SOCKET client_socket, server_stage stage, thread_paramaters *thread_params, char msg_tokens[][MAX_MSG_LEN])
{
	char *accepted_str = NULL;
	char **p_accepted = &accepted_str;
	t_transfer_result comm_res;

	// send menu display request to client
	if (TRNS_DISCONNECTED == (comm_res = send_msg_to_client(client_socket, stage, msg_tokens, FALSE, NULL, NULL, NULL, NULL)))	return SERVER_KILL_CLIENT_THREAD;
	
	if (set_socket_timeout(client_socket, WAIT_TIME_FOR_HUMAN, FALSE)) {
		printf_s("Server could not set socket receive timeout\n");
		return SERVER_KILL_CLIENT_THREAD;
	}

	// receive human choise to play against another player or exit
	if (TRNS_DISCONNECTED == (comm_res = recv_msg_from_client(client_socket, stage, p_accepted)))	return SERVER_KILL_CLIENT_THREAD;
	parse_message(accepted_str, msg_tokens[0], msg_tokens[1], msg_tokens[2], msg_tokens[3], msg_tokens[4]);
	free(accepted_str);
	*p_accepted = NULL;

	if (set_socket_timeout(client_socket, SERVER_RECV_TIMEOUT, FALSE)) {
		printf_s("Server could not set socket receive timeout\n");
		return SERVER_KILL_CLIENT_THREAD;
	}
	if (STRINGS_ARE_EQUAL("CLIENT_VERSUS", msg_tokens[0])) {
		*(thread_params->ready_to_play) += 1;
		return CLIENT_WAITS_FOR_OPPONENT;
	}
	else if (STRINGS_ARE_EQUAL("CLIENT_DISCONNECT", msg_tokens[0]))
		return SHOW_CLIENT_OUT;
	else 
		return SERVER_ERROR;

}
//	----------------------------------------------------------------------------------------
server_stage stage_find_opponent(SOCKET client_socket, server_stage stage, char msg_tokens[][MAX_MSG_LEN], thread_paramaters *thread_params, char user_name[])
{
	t_transfer_result comm_res;
	if (*(thread_params->ready_to_play) >= MAX_PLAYERS) {
		if (SUCCESS != release_sync_object(thread_params->first_event, FIRST_EVENT))
			return SERVER_KILL_CLIENT_THREAD;
		if (TRNS_DISCONNECTED == (comm_res = send_msg_to_client(client_socket, stage, msg_tokens, TRUE, user_name, NULL, NULL, NULL)))	return SERVER_KILL_CLIENT_THREAD;
		return RECEIVE_INITIAL_GUESS_FROM_CLIENT;
	}
	else {
		if (SUCCESS != waiting_for_sync_object(thread_params->first_event, FIRST_EVENT, EVENT_WAIT_TIMEOUT))
			return SERVER_KILL_CLIENT_THREAD;
		if (*(thread_params->ready_to_play) >= MAX_PLAYERS) {
			if (TRNS_DISCONNECTED == (comm_res = send_msg_to_client(client_socket, stage, msg_tokens, TRUE, user_name, NULL, NULL, NULL)))	return SERVER_KILL_CLIENT_THREAD;
			return RECEIVE_INITIAL_GUESS_FROM_CLIENT;
		}
		if (TRNS_DISCONNECTED == (comm_res = send_msg_to_client(client_socket, stage, msg_tokens, FALSE, NULL, NULL, NULL, NULL)))	return SERVER_KILL_CLIENT_THREAD;
		return SEND_DISPLAY;
	}
}
//	----------------------------------------------------------------------------------------
server_stage stage_receive_initial_choice(SOCKET client_socket, server_stage stage, char msg_tokens[][MAX_MSG_LEN], char *user_initial_choice, thread_paramaters *thread_params)
{
	t_transfer_result comm_res;
	char *accepted_str = NULL;
	char **p_accepted = &accepted_str;

	// Check if opponent quit
	if (EXIT_FAILURE == refresh_threads_array_middle(thread_handles, thread_params, (thread_params->ready_to_play))) return EXIT_FAILURE;
	if (*(thread_params->ready_to_play) < MAX_PLAYERS) {
		if (TRNS_DISCONNECTED == (comm_res = send_msg_to_client(client_socket, OPPONENT_QUIT, msg_tokens, FALSE, NULL, NULL, NULL, NULL)))	return SERVER_KILL_CLIENT_THREAD;
		return SEND_DISPLAY;
	}

	// request initial choice
	if (TRNS_DISCONNECTED == (comm_res = send_msg_to_client(client_socket, stage, msg_tokens, FALSE, NULL, NULL, NULL, NULL)))	return SERVER_KILL_CLIENT_THREAD;
	if (set_socket_timeout(client_socket, WAIT_TIME_FOR_HUMAN, FALSE)) {
		printf_s("Server could not set socket receive timeout\n");
		return SERVER_KILL_CLIENT_THREAD;
	}

	// receive inital choice
	if (TRNS_DISCONNECTED == (comm_res = recv_msg_from_client(client_socket, stage, p_accepted)))	return SERVER_KILL_CLIENT_THREAD;
	if (EXIT_FAILURE == parse_message(accepted_str, msg_tokens[0], msg_tokens[1], msg_tokens[2], msg_tokens[3], msg_tokens[4])) return SERVER_KILL_CLIENT_THREAD;
	free(accepted_str);
	*p_accepted = NULL;

	if (set_socket_timeout(client_socket, SERVER_RECV_TIMEOUT, FALSE)) {
		printf_s("Server could not set socket receive timeout\n");
		return SERVER_KILL_CLIENT_THREAD;
	}
	
	if (STRINGS_ARE_EQUAL("CLIENT_SETUP", msg_tokens[0])) {
		strcpy_s(user_initial_choice, MAX_USER_CHOICE_STR_LEN, msg_tokens[1]);
		// Check if opponent quit
		if (EXIT_FAILURE == refresh_threads_array_middle(thread_handles, thread_params, (thread_params->ready_to_play))) return EXIT_FAILURE;
		if (*(thread_params->ready_to_play) < MAX_PLAYERS) {
			if (TRNS_DISCONNECTED == (comm_res = send_msg_to_client(client_socket, OPPONENT_QUIT, msg_tokens, FALSE, NULL, NULL, NULL, NULL)))	return SERVER_KILL_CLIENT_THREAD;
			return SEND_DISPLAY;
		}
		return GET_TO_KNOW;
	}
	else if (STRINGS_ARE_EQUAL("CLIENT_DISCONNECT", msg_tokens[0])) 
		return SHOW_CLIENT_OUT;

	return GET_TO_KNOW;
}
//	----------------------------------------------------------------------------------------
server_stage stage_get_to_know(server_stage stage, thread_paramaters *thread_params, char *user_name, char *opponent_name, char *user_initial_choice, char *opponent_initial_choice)
{
	//t_transfer_result comm_res;
	BOOL file_was_created_here = FALSE;
	DWORD mutex_timeout = MUTEX_WAIT_TIMEOUT, event_timeout = EVENT_WAIT_TIMEOUT;
	DWORD file_access = GENERIC_READ | GENERIC_WRITE, file_share = FILE_SHARE_READ | FILE_SHARE_WRITE;
	char *file_name = "GameSession.txt";
	HANDLE game_session_file = NULL;
	//if (TRNS_DISCONNECTED == (comm_res = send_msg_to_client(client_socket, stage, msg_tokens, FALSE, NULL, NULL, NULL, NULL)))	return SERVER_KILL_CLIENT_THREAD;
	if (SUCCESS != waiting_for_sync_object(thread_params->thread_mutex, MUTEX, MUTEX_WAIT_TIMEOUT))
		return SERVER_KILL_CLIENT_THREAD;
	if (!FileExists(file_name)) {
		if (INVALID_HANDLE_VALUE == (game_session_file = CreateFile(TEXT(file_name), file_access, file_share, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL))) {
			printf_s("Could not create \'GameSession.txt\' file. error no.%li. Terminating...\n", GetLastError());
			return SERVER_KILL_CLIENT_THREAD;
		}
		file_was_created_here = TRUE;
	}
	else {
		if (INVALID_HANDLE_VALUE == (game_session_file = CreateFile(TEXT(file_name), file_access, file_share, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL))) {
			printf_s("Could not create \'GameSession.txt\' file. error no.%li. Terminating...\n", GetLastError());
			return SERVER_KILL_CLIENT_THREAD;
		}
	}
	if (SUCCESS != release_sync_object(thread_params->thread_mutex, MUTEX))
		return SERVER_KILL_CLIENT_THREAD;

	if (!file_was_created_here) {
		if (SUCCESS != waiting_for_sync_object(thread_params->first_event, FIRST_EVENT, EVENT_HUMAN_WAIT_TIMEOUT))
			return SERVER_KILL_CLIENT_THREAD;
		if (SUCCESS != waiting_for_sync_object(thread_params->thread_mutex, MUTEX, MUTEX_WAIT_TIMEOUT))
			return SERVER_KILL_CLIENT_THREAD;

		if (SERVER_KILL_CLIENT_THREAD == read_opponent_info(game_session_file, opponent_name, opponent_initial_choice)) return SERVER_KILL_CLIENT_THREAD;
		if (SERVER_KILL_CLIENT_THREAD == write_my_info_to_file(game_session_file, user_name, user_initial_choice))	return SERVER_KILL_CLIENT_THREAD;

		if (SUCCESS != release_sync_object(thread_params->thread_mutex, MUTEX))
			return SERVER_KILL_CLIENT_THREAD;
		if (SUCCESS != release_sync_object(thread_params->second_event, SECOND_EVENT))
			return SERVER_KILL_CLIENT_THREAD;
	}
	else {
		if (SUCCESS != waiting_for_sync_object(thread_params->thread_mutex, MUTEX, MUTEX_WAIT_TIMEOUT))
			return SERVER_KILL_CLIENT_THREAD;

		if (SERVER_KILL_CLIENT_THREAD == write_my_info_to_file(game_session_file, user_name, user_initial_choice))			return SERVER_KILL_CLIENT_THREAD;

		if (SUCCESS != release_sync_object(thread_params->thread_mutex, MUTEX))
			return SERVER_KILL_CLIENT_THREAD;
		if (SUCCESS != release_sync_object(thread_params->first_event, FIRST_EVENT))
			return SERVER_KILL_CLIENT_THREAD;
		if (SUCCESS != waiting_for_sync_object(thread_params->second_event, SECOND_EVENT, EVENT_HUMAN_WAIT_TIMEOUT))
			return SERVER_KILL_CLIENT_THREAD;
		if (SUCCESS != waiting_for_sync_object(thread_params->thread_mutex, MUTEX, MUTEX_WAIT_TIMEOUT))
			return SERVER_KILL_CLIENT_THREAD;

		if (SERVER_KILL_CLIENT_THREAD == read_opponent_info(game_session_file, opponent_name, opponent_initial_choice))		return SERVER_KILL_CLIENT_THREAD;

		if (SUCCESS != release_sync_object(thread_params->thread_mutex, MUTEX))
			return SERVER_KILL_CLIENT_THREAD;
	}

	if (!CloseHandle(game_session_file)) {	// close handle
		printf_s("Failed at closing file \'GameSession.txt\' handle. Terminating...\n");
		return SERVER_KILL_CLIENT_THREAD;
	}
	if (file_was_created_here) {
		if (reset_file_state())	// delete file
			return SERVER_KILL_CLIENT_THREAD;
		if (SUCCESS != release_sync_object(thread_params->first_event, FIRST_EVENT))
			return SERVER_KILL_CLIENT_THREAD;
	}
	else {
		if (SUCCESS != waiting_for_sync_object(thread_params->first_event, FIRST_EVENT, EVENT_WAIT_TIMEOUT))
			return SERVER_KILL_CLIENT_THREAD;
	}
	// Check if opponent quit

	return GAME_ON;

}
//	----------------------------------------------------------------------------------------
server_stage stage_play_round(SOCKET client_socket, server_stage stage, thread_paramaters *thread_params, char *user_name, char *opponent_name, char *user_initial_choice, char *opponent_initial_choice)
{
	BOOL file_was_created_here = FALSE;
	DWORD mutex_timeout = MUTEX_WAIT_TIMEOUT, event_timeout = EVENT_WAIT_TIMEOUT, file_access = GENERIC_READ | GENERIC_WRITE, file_share = FILE_SHARE_READ | FILE_SHARE_WRITE;
	char *file_name = "GameSession.txt", user_current_guess[MAX_USER_CHOICE_STR_LEN], opponent_current_guess[MAX_USER_CHOICE_STR_LEN], msg_tokens[MSG_TOKENS_NUM][MAX_MSG_LEN];
	HANDLE game_session_file = NULL;
	t_transfer_result comm_res;

	// Check if opponent quit
	if (EXIT_FAILURE == refresh_threads_array_middle(thread_handles, thread_params, (thread_params->ready_to_play))) return EXIT_FAILURE;
	if (*(thread_params->ready_to_play) < MAX_PLAYERS) {
		if (TRNS_DISCONNECTED == (comm_res = send_msg_to_client(client_socket, OPPONENT_QUIT, msg_tokens, FALSE, NULL, NULL, NULL, NULL)))	return SERVER_KILL_CLIENT_THREAD;
		return SEND_DISPLAY;
	}

	if (SUCCESS != waiting_for_sync_object(thread_params->thread_mutex, MUTEX, MUTEX_WAIT_TIMEOUT))
		return SERVER_KILL_CLIENT_THREAD;
	if (!FileExists(file_name)) {
		if (INVALID_HANDLE_VALUE == (game_session_file = CreateFile(TEXT(file_name), file_access, file_share, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL))) {
			printf_s("Could not create \'GameSession.txt\' file. error no.%li. Terminating...\n", GetLastError());
			return SERVER_KILL_CLIENT_THREAD;
		}
		file_was_created_here = TRUE;
	}
	else {
		if (INVALID_HANDLE_VALUE == (game_session_file = CreateFile(TEXT(file_name), file_access, file_share, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL))) {
			printf_s("Could not create \'GameSession.txt\' file. error no.%li. Terminating...\n", GetLastError());
			return SERVER_KILL_CLIENT_THREAD;
		}
	}
	if (SUCCESS != release_sync_object(thread_params->thread_mutex, MUTEX))
		return SERVER_KILL_CLIENT_THREAD;

	// Check if opponent quit
	if (EXIT_FAILURE == refresh_threads_array_middle(thread_handles, thread_params, (thread_params->ready_to_play))) return EXIT_FAILURE;
	if (*(thread_params->ready_to_play) < MAX_PLAYERS) {
		if (TRNS_DISCONNECTED == (comm_res = send_msg_to_client(client_socket, OPPONENT_QUIT, msg_tokens, FALSE, NULL, NULL, NULL, NULL)))	return SERVER_KILL_CLIENT_THREAD;
		return SEND_DISPLAY;
	}
	
	if (TRNS_DISCONNECTED == (comm_res = send_msg_to_client(client_socket, stage, msg_tokens, FALSE, NULL, NULL, NULL, NULL)))	return SERVER_KILL_CLIENT_THREAD;

	if (!file_was_created_here) {
		if (SUCCESS != waiting_for_sync_object(thread_params->first_event, FIRST_EVENT, EVENT_HUMAN_WAIT_TIMEOUT))
			return SERVER_KILL_CLIENT_THREAD;
		if (SUCCESS != waiting_for_sync_object(thread_params->thread_mutex, MUTEX, MUTEX_HUMAN_WAIT_TIMEOUT))
			return SERVER_KILL_CLIENT_THREAD;

		if (SERVER_KILL_CLIENT_THREAD == read_opponent_guess(game_session_file, opponent_current_guess))	return SERVER_KILL_CLIENT_THREAD;
		if (SERVER_KILL_CLIENT_THREAD == write_my_guess_to_file(client_socket, stage, game_session_file, msg_tokens, user_current_guess, thread_params))	return SERVER_KILL_CLIENT_THREAD;

		if (SUCCESS != release_sync_object(thread_params->thread_mutex, MUTEX))
			return SERVER_KILL_CLIENT_THREAD;
		if (SUCCESS != release_sync_object(thread_params->second_event, SECOND_EVENT))
			return SERVER_KILL_CLIENT_THREAD;
	}
	else {
		if (SUCCESS != waiting_for_sync_object(thread_params->thread_mutex, MUTEX, MUTEX_HUMAN_WAIT_TIMEOUT))
			return SERVER_KILL_CLIENT_THREAD;

		if (SERVER_KILL_CLIENT_THREAD == write_my_guess_to_file(client_socket, stage, game_session_file, msg_tokens, user_current_guess, thread_params))	return SERVER_KILL_CLIENT_THREAD;

		if (SUCCESS != release_sync_object(thread_params->thread_mutex, MUTEX))
			return SERVER_KILL_CLIENT_THREAD;
		if (SUCCESS != release_sync_object(thread_params->first_event, FIRST_EVENT))
			return SERVER_KILL_CLIENT_THREAD;
		if (SUCCESS != waiting_for_sync_object(thread_params->second_event, SECOND_EVENT, EVENT_HUMAN_WAIT_TIMEOUT))
			return SERVER_KILL_CLIENT_THREAD;
		if (SUCCESS != waiting_for_sync_object(thread_params->thread_mutex, MUTEX, MUTEX_WAIT_TIMEOUT))
			return SERVER_KILL_CLIENT_THREAD;

		if (SERVER_KILL_CLIENT_THREAD == read_opponent_guess(game_session_file, opponent_current_guess))	return SERVER_KILL_CLIENT_THREAD;

		if (SUCCESS != release_sync_object(thread_params->thread_mutex, MUTEX))
			return SERVER_KILL_CLIENT_THREAD;
	}

	if (SERVER_KILL_CLIENT_THREAD == (stage = stage_process_results(client_socket, REPORT_RESULTS, opponent_name, opponent_initial_choice, user_current_guess, opponent_current_guess))) {
		CloseHandle(game_session_file);
		return SERVER_KILL_CLIENT_THREAD;
	}
	if (SERVER_KILL_CLIENT_THREAD == (stage = stage_draw_or_win(client_socket, stage, game_session_file, opponent_initial_choice, file_was_created_here, thread_params, user_name, opponent_name))) {
		CloseHandle(game_session_file);
		return SERVER_KILL_CLIENT_THREAD;
	}
	if (stage == REPORT_WINNER || stage == REPORT_DRAW)
		*(thread_params->ready_to_play) -= 1;

	//while (1); //	----------------------------------------------------------------------------------------
	if (!CloseHandle(game_session_file)) {	// close handle
		printf_s("Failed at closing file \'GameSession.txt\' handle. Terminating...\n");
		return SERVER_KILL_CLIENT_THREAD;
	}
	if (file_was_created_here) {
		if (reset_file_state())	// delete file
			return SERVER_KILL_CLIENT_THREAD;
		if (SUCCESS != release_sync_object(thread_params->first_event, FIRST_EVENT))
			return SERVER_KILL_CLIENT_THREAD;
	}
	else {
		if (SUCCESS != waiting_for_sync_object(thread_params->first_event, FIRST_EVENT, EVENT_WAIT_TIMEOUT))
			return SERVER_KILL_CLIENT_THREAD;
	}

	// Check if opponent quit
	if (EXIT_FAILURE == refresh_threads_array_middle(thread_handles, thread_params, (thread_params->ready_to_play))) return EXIT_FAILURE;
	if (*(thread_params->ready_to_play) < MAX_PLAYERS) {
		if (TRNS_DISCONNECTED == (comm_res = send_msg_to_client(client_socket, OPPONENT_QUIT, msg_tokens, FALSE, NULL, NULL, NULL, NULL)))	return SERVER_KILL_CLIENT_THREAD;
		return SEND_DISPLAY;
	}

	return stage;
}
//	----------------------------------------------------------------------------------------
t_transfer_result recv_msg_from_client(SOCKET client_socket, server_stage stage, char **accepted_str)
{
	t_transfer_result receive_res;
	receive_res = receive_string(accepted_str, client_socket);
	if (receive_res == TRNS_FAILED)
	{
		free(*accepted_str);
		*accepted_str = NULL;
		return TRNS_FAILED;
	}
	else if (receive_res == TRNS_DISCONNECTED)
	{
		free(*accepted_str);
		*accepted_str = NULL;
		return TRNS_DISCONNECTED;
	}

	return EXIT_SUCCESS;
}
//	----------------------------------------------------------------------------------------
t_transfer_result send_msg_to_client(SOCKET client_socket, server_stage stage, char msg_tokens[][MAX_MSG_LEN], BOOL opponent_exists, char *param1, char *param2, char *param3, char *param4)
{
	t_transfer_result send_res;
	char str_to_send[SERVER_MAX_MSG_LEN];
	if (EXIT_FAILURE == server_format_message_to_send(stage, str_to_send, msg_tokens, opponent_exists, param1, param2, param3, param4))	return TRNS_FAILED;


	send_res = send_string(str_to_send, client_socket);
	if (send_res == TRNS_FAILED) {
		return TRNS_FAILED;
	}
	else if (send_res == TRNS_DISCONNECTED) {
		return TRNS_DISCONNECTED;
	}

	return EXIT_SUCCESS;
}
//	----------------------------------------------------------------------------------------
server_stage write_my_guess_to_file(SOCKET client_socket, server_stage stage, HANDLE game_session_file, char msg_tokens[][MAX_MSG_LEN], char *user_current_guess, thread_paramaters *thread_params)
{
	char *accepted_str = NULL;
	char **p_accepted = &accepted_str;
	server_stage ret_val = GAME_ON;
	t_transfer_result comm_res;


	//send_msg_to_client(client_socket, stage, msg_tokens, FALSE, NULL, NULL, NULL, NULL);
	if (set_socket_timeout(client_socket, WAIT_TIME_FOR_HUMAN, FALSE)) {
		printf_s("Server could not set socket receive timeout\n");
		return SERVER_KILL_CLIENT_THREAD;	
	}

	if (TRNS_DISCONNECTED == (comm_res = recv_msg_from_client(client_socket, stage, p_accepted)))
		return SERVER_KILL_CLIENT_THREAD;
	parse_message(accepted_str, msg_tokens[0], msg_tokens[1], msg_tokens[2], msg_tokens[3], msg_tokens[4]);
	free(accepted_str);
	*p_accepted = NULL;

	if (set_socket_timeout(client_socket, SERVER_RECV_TIMEOUT, FALSE)) {
		printf_s("Server could not set socket receive timeout\n");
		return SERVER_KILL_CLIENT_THREAD;	// deinit_winsock() prints the error
	}

	if (STRINGS_ARE_EQUAL("CLIENT_PLAYER_MOVE", msg_tokens[0])) {
		strcpy_s(user_current_guess, MAX_USER_CHOICE_STR_LEN, msg_tokens[1]);
		ret_val = GAME_ON;
	}
	else if (STRINGS_ARE_EQUAL("CLIENT_DISCONNECT", msg_tokens[0])) {
		return SHOW_CLIENT_OUT;
	}
	
	// Write to file
	
	strcat_s(user_current_guess, MAX_MSG_LEN, "\n");
	if (EXIT_FAILURE == write_to_file(game_session_file, user_current_guess))
		return SERVER_KILL_CLIENT_THREAD;
	return ret_val;

}
//	----------------------------------------------------------------------------------------
server_stage read_opponent_guess(HANDLE file_name, char *opponent_current_guess)
{
	if (EXIT_FAILURE == read_from_file(file_name, opponent_current_guess))
		return SERVER_KILL_CLIENT_THREAD;
	return GAME_ON;
}
//	----------------------------------------------------------------------------------------
int write_to_file(HANDLE file, char *str_to_write)
{
	DWORD num_bytes_to_write = strlen(str_to_write) + 1, dword_bytes_written = 1, dword_error = 0;
	BOOL write_result = TRUE;
	DWORD tmp = SetFilePointer(file, 0, NULL, FILE_CURRENT);
	
	if (write_result = (WriteFile(file, str_to_write, num_bytes_to_write, &dword_bytes_written, NULL)));
	if (0 != (dword_error = GetLastError())) {
		printf_s("Couldn't write to file, error number %d, terminating program.\n", (int)dword_error);
		return EXIT_FAILURE;
	}
	if (INVALID_SET_FILE_POINTER == SetFilePointer(file, 0, NULL, FILE_BEGIN)) {
		printf_s("Could not set file pointer to next position. Terminating...\n");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
//	----------------------------------------------------------------------------------------
int read_from_file(HANDLE file, char *read_str)
{
	int dynamic_str_index = 0;
	char in_buffer[1] = { 0 };
	DWORD num_bytes_to_read = 1, dword_bytes_read = 1, output_num = 0;
	DWORD dword_error = GetLastError(), file_pos = 0;
	BOOL read_result = TRUE, is_EOF = FALSE;
	DWORD tmp = SetFilePointer(file, 0, NULL, FILE_CURRENT);
	
	while (TRUE) {
		if (TRUE == (read_result = ReadFile(file, in_buffer, num_bytes_to_read, &dword_bytes_read, NULL)));
		if ((dword_error = GetLastError()) != 0) {
			printf_s("Couldn't read from file, error number %d, Terminating...\n", (int)dword_error);
			return EXIT_FAILURE;
		}
		if (TRUE == (is_EOF = check_EOF(dword_bytes_read, read_result))) // Reached the EOF
			break;
		if (in_buffer[0] == '\n')  
			break;
		read_str[dynamic_str_index++] = in_buffer[0];  // Save the mission location, digit-by-digit, from the priorities file.
	}

	read_str[dynamic_str_index] = '\0';

	if (is_EOF || (in_buffer[0] == '\n')) { // Reached the EOF or '\n'
		file_pos = SetFilePointer(file, 0, NULL, FILE_BEGIN);
		if (INVALID_SET_FILE_POINTER == file_pos)
		{
			printf_s("Could not set file pointer to next position. Terminating...\n");
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}
//	----------------------------------------------------------------------------------------
server_stage write_my_info_to_file(HANDLE game_session_file, char *user_name, char *user_initial_choice)
{
	char write_str[MAX_MSG_LEN];
	sprintf_s(write_str, MAX_MSG_LEN, "%s:%s\n", user_name, user_initial_choice);
	if (EXIT_FAILURE == write_to_file(game_session_file, write_str))
		return SERVER_KILL_CLIENT_THREAD;
	return GAME_ON;
}
//	----------------------------------------------------------------------------------------
server_stage read_opponent_info(HANDLE game_session_file, char *opponent_name, char *opponent_initial_choice)
{
	char read_str[MAX_MSG_LEN];
	if (EXIT_FAILURE == read_from_file(game_session_file, read_str))
		return SERVER_KILL_CLIENT_THREAD;
	sprintf_s(read_str, MAX_MSG_LEN, "%s\n", read_str);
	parse_info(read_str, opponent_name, opponent_initial_choice);
	return GAME_ON;
}
//	----------------------------------------------------------------------------------------
int server_format_message_to_send(server_stage stage, char *send_buffer, char msg_tokens[][MAX_MSG_LEN], BOOL opponent_exists, char *param1, char *param2, char *param3, char *param4)
{
	switch (stage) {
	case(GET_USERNAME_SEND_APPROVAL):
		if (!opponent_exists)
			sprintf_s(send_buffer, SERVER_MAX_MSG_LEN, "SERVER_APPROVED\n");
		else
			sprintf_s(send_buffer, SERVER_MAX_MSG_LEN, "SERVER_DENIED:%s\n", param1);
		break;
	case (SEND_DISPLAY):
		sprintf_s(send_buffer, SERVER_MAX_MSG_LEN, "SERVER_MAIN_MENU\n");
		break;
	case (CLIENT_WAITS_FOR_OPPONENT):
		if (opponent_exists) {
			if (param1 == NULL) {
				printf_s("NULL paramater1 sent. Terminating...\n");
				return EXIT_FAILURE;
			}
			sprintf_s(send_buffer, SERVER_MAX_MSG_LEN, "SERVER_INVITE:%s\n", param1);
		}
		else
			sprintf_s(send_buffer, SERVER_MAX_MSG_LEN, "SERVER_NO_OPPONENTS\n");
		break;
	case (RECEIVE_INITIAL_GUESS_FROM_CLIENT):
		sprintf_s(send_buffer, SERVER_MAX_MSG_LEN, "SERVER_SETUP_REQUEST\n");
		break;
	case (GAME_ON):
		sprintf_s(send_buffer, SERVER_MAX_MSG_LEN, "SERVER_PLAYER_MOVE_REQUEST\n");
		break;
	case (REPORT_RESULTS):
		if (param1 == NULL || param2 == NULL || param3 == NULL || param4 == NULL) {
			printf_s("NULL paramater sent. Terminating...\n");
			return EXIT_FAILURE;
		}
		sprintf_s(send_buffer, SERVER_MAX_MSG_LEN, "SERVER_GAME_RESULTS:%s;%s;%s;%s\n", param1, param2, param3, param4);
		break;
	case (REPORT_DRAW):
		sprintf_s(send_buffer, SERVER_MAX_MSG_LEN, "SERVER_DRAW\n");
		break;
	case (REPORT_WINNER):
		if (param1 == NULL || param2 == NULL) {
			printf_s("NULL paramater sent. Terminating...\n");
			return EXIT_FAILURE;
		}
		sprintf_s(send_buffer, SERVER_MAX_MSG_LEN, "SERVER_WIN:%s;%s\n", param1, param2);
		break;
	case (OPPONENT_QUIT):
		sprintf_s(send_buffer, SERVER_MAX_MSG_LEN, "SERVER_OPPONENT_QUIT\n");
		break;
	}
	
	return EXIT_SUCCESS;
}
//	----------------------------------------------------------------------------------------
int parse_info(char *info_str, char *opponent_name, char *opponent_initial_choice)
{
	int i = 0;
	if (info_str == NULL)
		return EXIT_FAILURE;
	while (TRUE) {
		while ((*info_str != ':') && (*info_str != '\n') && (*info_str != '\r')) {
			*(opponent_name + i) = *info_str;
			info_str++;
			i++;
		}
		*(opponent_name + i) = '\0';
		if ((*info_str == '\n') || (*info_str == '\r'))	// end of msg, no paramaters
			break;
		i = 0;
		info_str++;

		while ((*info_str != '\n') && (*info_str != '\r')) {
			*(opponent_initial_choice + i) = *info_str;
			info_str++;
			i++;
		}
		*(opponent_initial_choice + i) = '\0';
		if ((*info_str == '\n') || (*info_str == '\r'))	// end of msg, no paramaters
			break;
	}
	return EXIT_SUCCESS;
}
//	----------------------------------------------------------------------------------------
server_stage stage_process_results(SOCKET client_socket, server_stage stage, char *opponent_name, char *opponent_initial_choice, char *user_current_guess, char *opponent_current_guess)
{
	int cows = 0, bulls = 0;
	server_stage ret_val;
	char bulls_str[MAX_USER_CHOICE_STR_LEN], cows_str[MAX_USER_CHOICE_STR_LEN];
	t_transfer_result comm_res;

	check_bulls_and_cows(user_current_guess, opponent_initial_choice, &bulls, &cows);
	if (bulls == 4)		ret_val = CANDIDATE_TO_WIN;
	else				ret_val = REPORT_RESULTS;

	sprintf_s(bulls_str, MAX_USER_CHOICE_STR_LEN, "%d", bulls);
	sprintf_s(cows_str, MAX_USER_CHOICE_STR_LEN, "%d", cows);

	if (TRNS_DISCONNECTED == (comm_res = send_msg_to_client(client_socket, stage, NULL, FALSE, bulls_str, cows_str, opponent_name, opponent_current_guess)))	return SERVER_KILL_CLIENT_THREAD;

	return ret_val;
}
//	----------------------------------------------------------------------------------------
server_stage stage_draw_or_win(SOCKET client_socket, server_stage stage, HANDLE file, char *opponent_initial_choice, BOOL thread_created_file, thread_paramaters *thread_params, char *user_name, char *opponent_name)
{
	DWORD mutex_timeout = MUTEX_WAIT_TIMEOUT, event_timeout = EVENT_WAIT_TIMEOUT;
	char user_current_stage[MAX_MSG_LEN], opponent_current_stage[MAX_MSG_LEN];
	format_stage_write_buffer(stage, user_current_stage);
	t_transfer_result comm_res;
	

	if (!thread_created_file) {
		if (SUCCESS != waiting_for_sync_object(thread_params->first_event, FIRST_EVENT, EVENT_WAIT_TIMEOUT))
			return SERVER_KILL_CLIENT_THREAD;
		if (SUCCESS != waiting_for_sync_object(thread_params->thread_mutex, MUTEX, MUTEX_WAIT_TIMEOUT))
			return SERVER_KILL_CLIENT_THREAD;

		if (SERVER_KILL_CLIENT_THREAD == read_opponent_stage_from_file(file, opponent_current_stage))	return SERVER_KILL_CLIENT_THREAD;
		if (SERVER_KILL_CLIENT_THREAD == write_stage_to_file(file, stage, user_current_stage))	return SERVER_KILL_CLIENT_THREAD;

		if (SUCCESS != release_sync_object(thread_params->thread_mutex, MUTEX))
			return SERVER_KILL_CLIENT_THREAD;
		if (SUCCESS != release_sync_object(thread_params->second_event, SECOND_EVENT))
			return SERVER_KILL_CLIENT_THREAD;
	}
	else {
		if (SUCCESS != waiting_for_sync_object(thread_params->thread_mutex, MUTEX, MUTEX_WAIT_TIMEOUT))
			return SERVER_KILL_CLIENT_THREAD;

		if (SERVER_KILL_CLIENT_THREAD == write_stage_to_file(file, stage, user_current_stage))	return SERVER_KILL_CLIENT_THREAD;

		if (SUCCESS != release_sync_object(thread_params->thread_mutex, MUTEX))
			return SERVER_KILL_CLIENT_THREAD;
		if (SUCCESS != release_sync_object(thread_params->first_event, FIRST_EVENT))
			return SERVER_KILL_CLIENT_THREAD;
		if (SUCCESS != waiting_for_sync_object(thread_params->second_event, SECOND_EVENT, EVENT_WAIT_TIMEOUT))
			return SERVER_KILL_CLIENT_THREAD;
		if (SUCCESS != waiting_for_sync_object(thread_params->thread_mutex, MUTEX, MUTEX_WAIT_TIMEOUT))
			return SERVER_KILL_CLIENT_THREAD;

		if (SERVER_KILL_CLIENT_THREAD == read_opponent_stage_from_file(file, opponent_current_stage))	return SERVER_KILL_CLIENT_THREAD;

		if (SUCCESS != release_sync_object(thread_params->thread_mutex, MUTEX))
			return SERVER_KILL_CLIENT_THREAD;
	}

	if (STRINGS_ARE_EQUAL("CANDIDATE_TO_WIN", user_current_stage) && STRINGS_ARE_EQUAL("CANDIDATE_TO_WIN", opponent_current_stage)) {
		stage = REPORT_DRAW;
		if (TRNS_DISCONNECTED == (comm_res = send_msg_to_client(client_socket, stage, NULL, FALSE, NULL, NULL, NULL, NULL)))	return SERVER_KILL_CLIENT_THREAD;
	}
	else if (STRINGS_ARE_EQUAL("CANDIDATE_TO_WIN", user_current_stage)) {
		stage = REPORT_WINNER;
		if (TRNS_DISCONNECTED == (comm_res = send_msg_to_client(client_socket, stage, NULL, FALSE, user_name, opponent_initial_choice, NULL, NULL)))	return SERVER_KILL_CLIENT_THREAD;
	}
	else if (STRINGS_ARE_EQUAL("CANDIDATE_TO_WIN", opponent_current_stage)) {
		stage = REPORT_WINNER;
		if (TRNS_DISCONNECTED == (comm_res = send_msg_to_client(client_socket, stage, NULL, FALSE, opponent_name, opponent_initial_choice, NULL, NULL)))	return SERVER_KILL_CLIENT_THREAD;
	}
	else stage = GAME_ON;

	return stage;

}
//	----------------------------------------------------------------------------------------
server_stage write_stage_to_file(HANDLE file, server_stage stage, char *user_current_stage)
{
	if (EXIT_FAILURE == write_to_file(file, user_current_stage))
		return SERVER_KILL_CLIENT_THREAD;
	return SEND_DISPLAY;
}
//	----------------------------------------------------------------------------------------
server_stage read_opponent_stage_from_file(HANDLE file, char *opponent_current_stage)
{
	if (EXIT_FAILURE == read_from_file(file, opponent_current_stage))
		return SERVER_KILL_CLIENT_THREAD;
	return SEND_DISPLAY;
}
//	----------------------------------------------------------------------------------------
void format_stage_write_buffer(server_stage stage, char *user_current_stage)
{
	switch (stage) {
	case (CANDIDATE_TO_WIN):
		sprintf_s(user_current_stage, MAX_MSG_LEN, "CANDIDATE_TO_WIN");
		break;
	case(REPORT_RESULTS):
		sprintf_s(user_current_stage, MAX_MSG_LEN, "REPORT_RESULTS");
		break;
	}
}
//	----------------------------------------------------------------------------------------
void check_bulls_and_cows(char *user_current_guess, char *opponent_initial_choice, int *p_bulls, int *p_cows) {
	int i = 0, j = 0;
	for (i = 0; i < 4; i++) {
		char my_guess_dig = user_current_guess[i];
		for (j = 0; j < 4; j++) {
			if (my_guess_dig == opponent_initial_choice[j]) {
				if (i == j)
					*p_bulls += 1;  
				else
					*p_cows += 1;
			}
		}
	}
}
//	----------------------------------------------------------------------------------------
void service_third_player(SOCKET client_socket)
{
	send_msg_to_client(client_socket, GET_USERNAME_SEND_APPROVAL, NULL, TRUE, "Server full.", NULL, NULL, NULL);	// 2F - GET RESULT
}
//	----------------------------------------------------------------------------------------
int refresh_threads_array(HANDLE *thread_handles, thread_paramaters **thread_params, int *connected_players)
{
	int ind = 0;
	DWORD exit_code;

	for (ind = 0; ind < MAX_CONNECTIONS; ind++)
	{
		if (*(thread_handles + ind) != NULL) {
			// poll to check if thread finished running:
			GetExitCodeThread(*(thread_handles + ind), &exit_code);
			if (exit_code == STILL_ACTIVE)
				continue;
			else {
				*connected_players -= 1;
				if (!CloseHandle(*(thread_handles + ind)))
					return EXIT_FAILURE;
				//free(*(thread_handles + ind));
				*(thread_handles + ind) = NULL;
				free(*(thread_params + ind));
				*(thread_params + ind) = NULL;
			}
		}
	}
	return EXIT_SUCCESS;
}
//	----------------------------------------------------------------------------------------
int refresh_threads_array_middle(HANDLE *thread_handles, thread_paramaters *thread_params, int *versus_players)
{
	int ind = 0;
	DWORD exit_code;

	for (ind = 0; ind < MAX_CONNECTIONS; ind++)
	{
		if (*(thread_handles + ind) != NULL) {
			// poll to check if thread finished running:
			GetExitCodeThread(*(thread_handles + ind), &exit_code);
			if (exit_code == STILL_ACTIVE)
				continue;
			else {
				*versus_players -= 1;
			}
		}
	}
	return EXIT_SUCCESS;
}