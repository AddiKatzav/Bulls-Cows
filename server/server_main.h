/*------------------------------------ server_main.h -----------------------------------*/
/* Description: This header file contains all the functions that are used by the server */
/*--------------------------------------------------------------------------------------*/

#ifndef __SERVER_MAIN_H__
#define __SERVER_MAIN_H__

	/* ----------------- */
	/* Library includes: */
	/* ----------------- */
	
	
	/* ----------------- */
	/* Project includes: */
	/* ----------------- */
	#include "server_actions.h"
	#include "global_misc.h"
	#include "ServerHardCodedData.h"
	
	/* ----------------------- */
	/* functions declarations: */
	/* ----------------------- */

	//	Description: This function is the main of the server. it listens on the servers IP address
	//				 and accepts new connection. Then it opens a new thread for them. The function
	//				 listens on the port for unlimited time.
	//	Paramaters: int server_port - The server's port specified as a cmd line argument
	//	Returns: EXIT_SUCCESS if succeeded, EXIT_FAILURE if failed.
	int server_main(int server_port);
	//	----------------------------------------------------------------------------------------

	//	Description: When a new thread is opened for a new accepted connection this function
	//				 goes through the threads' list and looks for an open spot for a new 
	//				 connection. It polls created thread to see if it already finished,
	//				 or finished with error.
	//	Paramaters: HANDLE *thread_handles - Array of handles.
	//              thread_paramaters **thread_params - Threads' paramaters aray.
	//              int *num_of_players - number of connected sockets to the server.
	//	Returns: First free index in threads_handles array if succeeded, -1 if failed or abandoned.
	static int find_first_unused_thread_slot(HANDLE *thread_handles, thread_paramaters **thread_params, int *num_of_players);
	//	----------------------------------------------------------------------------------------
	void init_thread_handles(HANDLE *thread_handles);
	//	----------------------------------------------------------------------------------------
	thread_paramaters *fill_thread_params(BOOL third, SOCKET *s, HANDLE thread_mutex, HANDLE first_event, HANDLE second_event, int *versus_players);
	//	----------------------------------------------------------------------------------------
	void init_thread_params(thread_paramaters *thread_params[]);
	//	----------------------------------------------------------------------------------------

	//	Description: Give service to the incoming client connection - main loop iterating over the
	//				 stages of the game, giving services to the client for each step.
	//	Paramaters: LPVOID lp_param - Void pointer to the paramaters to thread struct. Need to be casted.
	//	Returns: EXIT_SUCCESS if succeeded, EXIT_FAILURE if failed.
	static DWORD service_thread(LPVOID lp_param);
	//	----------------------------------------------------------------------------------------
	int setup_server_connection(SOCKET main_socket, int server_port);
	//	----------------------------------------------------------------------------------------

	//	Description: Manage the game - at each stage of progress of the game, advance the game according
	//				 to the events in the game.
	//	Paramaters: SOCKET client_socket - The socket that the client is connected to, at the server.
	//				server_stage *stage - A pointer to the client's current stage.
	//				thread_paramaters *thread_params - A pointer to the thread paramaters.
	//				char user_name[] - The username of the player.
	//				char opponent_name[] - The opponent's username.
	//				char *user_initial_number - The initial 4 digits of the player.
	//				char *opponent_initial_number - The initial 4 digits of the opponent.
	//	Returns: EXIT_SUCCESS if succeeded, EXIT_FAILURE if failed.
	void server_manage_game(SOCKET client_socket, server_stage *stage, thread_paramaters *thread_params, char user_name[], char opponent_name[], char *user_initial_number, char *opponent_initial_number);
	//	----------------------------------------------------------------------------------------
	void free_and_close_handles(HANDLE handle1, HANDLE handle2, HANDLE handle3, int *ptr, HANDLE *handles_array, thread_paramaters **thread_params, HANDLE *exit_thread);
	//	----------------------------------------------------------------------------------------

	//	Description: A thread that consistently checks if an exit string has been written at the
	//				 server console.
	//	Paramaters: NONE.
	//	Returns: EXIT_SUCCESS if succeeded, EXIT_FAILURE if failed.
	static DWORD check_for_exit_routine();
	//	----------------------------------------------------------------------------------------
	int create_exit_check_thread(HANDLE *exit_thread);
	//	----------------------------------------------------------------------------------------


#endif // !__SERVER_MAIN_H__