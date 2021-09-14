/*--------------------------------- server_actions.h -----------------------------------*/
/* Description: This module contains all the stages that need to be made at the server  */
/*				side of the program, with a few extra auxiliary functions				*/
/*--------------------------------------------------------------------------------------*/

#ifndef __SERVER_ACTIONS_H__
#define __SERVER_ACTIONS_H__

	/* ----------------- */
	/* Library includes: */
	/* ----------------- */

	/* ----------------- */
	/* Project includes: */
	/* ----------------- */
	#include "global_misc.h"
	#include "ServerHardCodedData.h"

	extern HANDLE *thread_handles;
	extern int *ready_to_play;
	extern int connected_clients;

	/* ----------------------- */
	/* functions declarations: */
	/* ----------------------- */

	//	Description: In this stage we get the player's username and the server approval to play the game.
	//				 listens on the port for unlimited time.
	//	Paramaters: SOCKET client_socket - A socket that is connected to the client.
	//				server_stage stage - The current stage of the server.
	//				char msg_tokens[][MAX_MSG_LEN] - An array of strings to hold the parsed incoming message from the client.
	//				char received_user_name[] - A string to obtain the player's username.
	//	Returns: Next stage to execute.
	server_stage stage_get_username_send_approval(SOCKET client_socket, server_stage stage, char msg_tokens[][MAX_MSG_LEN], char received_user_name[]);
	//	--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	//	Description: In this stage we send the client an order to display the main menu to the user.
	//	Paramaters: SOCKET client_socket - A socket that is connected to the client.
	//				server_stage stage - The current stage of the server.
	//				thread_paramaters *thread_params - A pointer to the current thread paramaters.
	//				char msg_tokens[][MAX_MSG_LEN] - An array of strings to hold the parsed incoming message from the client.
	//	Returns: Next stage to execute.
	server_stage stage_send_menu_display(SOCKET client_socket, server_stage stage, thread_paramaters *thread_params, char msg_tokens[][MAX_MSG_LEN]);
	//	--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	//	Description: In this stage we find an opponent to play against for the client's thread. If one doesn't exist we get him back to the main menu.
	//	Paramaters: SOCKET client_socket - A socket that is connected to the client.
	//				server_stage stage - The current stage of the server.
	//				char msg_tokens[][MAX_MSG_LEN] - An array of strings to hold the parsed incoming message from the client.
	//				thread_paramaters *thread_params - A pointer to the current thread paramaters.
	//				char user_name[] - the username of the player.
	//	Returns: Next stage to execute.
	server_stage stage_find_opponent(SOCKET client_socket, server_stage stage, char msg_tokens[][MAX_MSG_LEN], thread_paramaters *thread_params, char user_name[]);
	//	--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	//	Description: In this stage we recive the initial choice of the 4 digits from the client.
	//	Paramaters: SOCKET client_socket - A socket that is connected to the client.
	//				server_stage stage - The current stage of the server.
	//				char msg_tokens[][MAX_MSG_LEN] - An array of strings to hold the parsed incoming message from the client.
	//				char *user_initial_guess - A string containing the user's inital choice.
	//				thread_paramaters *thread_params - A pointer to the current thread paramaters.
	//	Returns: Next stage to execute.
	server_stage stage_receive_initial_choice(SOCKET client_socket, server_stage stage, char msg_tokens[][MAX_MSG_LEN], char *user_initial_guess, thread_paramaters *thread_params);
	//	--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	//	Description: In this stage we exchange information between two players - each's username and inital choice.
	//	Paramaters: server_stage stage - The current stage of the server.
	//				thread_paramaters *thread_params - A pointer to the current thread paramaters.
	//				char user_name[] - the username of the player.
	//				char opponent_name[] - the username of the opponent.
	//				char *user_initial_choice - A string containing the user's inital choice.
	//				char *opponent_initial_choice - A string containing the opponents's inital choice.
	//	Returns: Next stage to execute.
	server_stage stage_get_to_know(server_stage stage, thread_paramaters *thread_params, char *user_name, char *opponent_name, char *user_initial_choice, char *opponent_initial_choice);
	//	--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	//	Description: In this stage we execute a round of a given guess and report the results to the players. The synchronization is done using events and mutex.
	//	Paramaters: SOCKET client_socket - A socket that is connected to the client.
	//				server_stage stage - The current stage of the server.
	//				thread_paramaters *thread_params - A pointer to the current thread paramaters.
	//				char user_name[] - the username of the player.
	//				char opponent_name[] - the username of the opponent.
	//				char *user_initial_choice - A string containing the user's inital choice.
	//				char *opponent_initial_choice - A string containing the opponents's inital choice.
	//	Returns: Next stage to execute.
	server_stage stage_play_round(SOCKET client_socket, server_stage stage, thread_paramaters *thread_params, char *user_name, char *opponent_name, char *user_initial_choice, char *opponent_initial_choice);
	//	--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	//	Description: In this stage we process the results of a given guess against the initial choice.
	//	Paramaters: SOCKET client_socket - A socket that is connected to the client.
	//				server_stage stage - The current stage of the server.
	//				char opponent_name[] - the username of the opponent.
	//				char *opponent_initial_choice - A string containing the opponents's inital choice.
	//				char *user_current_guess - A string containing the user's current guess.
	//				char *opponent_current_guess - A string containing the opponent's current guess.
	//	Returns: Next stage to execute.
	server_stage stage_process_results(SOCKET client_socket, server_stage stage, char *opponent_name, char *opponent_initial_choice, char *user_current_guess, char *opponent_current_guess);
	//	--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	// From this stage forward all the functions are quite straight-forward.

	t_transfer_result recv_msg_from_client(SOCKET client_socket, server_stage stage, char **accepted_str);
	//	--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	t_transfer_result send_msg_to_client(SOCKET client_socket, server_stage stage, char msg_tokens[][MAX_MSG_LEN], BOOL opponent_exists, char *param1, char *param2, char *param3, char *param4);
	//	--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	int server_format_message_to_send(server_stage stage, char *send_buffer, char msg_tokens[][MAX_MSG_LEN], BOOL opponent_exists, char *param1, char *param2, char *param3, char *param4);
	//	--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	server_stage read_opponent_guess(HANDLE file_name, char *opponent_current_guess);
	//	--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	server_stage write_my_guess_to_file(SOCKET client_socket, server_stage stage, HANDLE game_session_file, char msg_tokens[][MAX_MSG_LEN], char *user_current_guess, thread_paramaters *thread_params);
	//	--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	int write_to_file(HANDLE file, char *output_str);
	//	--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	int read_from_file(HANDLE file, char *read_str);
	//	--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	server_stage write_my_info_to_file(HANDLE game_session_file, char *user_name, char *user_initial_choice);
	//	--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	server_stage read_opponent_info(HANDLE game_session_file, char *opponent_name, char *opponent_initial_choice);
	//	--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	int parse_info(char *info_str, char *opponent_name, char *opponent_initial_choice);
	//	--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void check_bulls_and_cows(char *user_current_guess, char *opponent_initial_choice, int *p_bulls, int *p_cows);
	//	--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	server_stage stage_draw_or_win(SOCKET client_socket, server_stage stage, HANDLE file, char *opponent_initial_choice, BOOL thread_created_file, thread_paramaters *thread_params, char *user_name, char *opponent_name);
	//	--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	server_stage write_stage_to_file(HANDLE file, server_stage stage, char *user_current_stage);
	//	--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	server_stage read_opponent_stage_from_file(HANDLE file, char *opponent_current_stage);
	//	--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void format_stage_write_buffer(server_stage stage, char *user_current_stage);
	//	--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void service_third_player(SOCKET client_socket);
	//	--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	int refresh_threads_array(HANDLE *thread_handles, thread_paramaters **thread_params, int *connected_players);
	//	----------------------------------------------------------------------------------------
	int refresh_threads_array_middle(HANDLE *thread_handles, thread_paramaters *thread_params, int *versus_players);
	//	----------------------------------------------------------------------------------------

#endif // !__SERVER_ACTIONS_H__
