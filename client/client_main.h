/*------------------------------------ client_main.h -----------------------------------*/
/* Description: This header file contains all the functions that are used by the client */
/*--------------------------------------------------------------------------------------*/

#ifndef __CLIENT_MAIN_H__
#define __CLIENT_MAIN_H__

	/* ----------------- */
	/* Library includes: */
	/* ----------------- */
	
	/* ----------------- */
	/* Project includes: */
	/* ----------------- */
	#include "global_misc.h"
	#include "ClientHardCodedData.h"

	typedef enum _client_stage {	// An enum to indicate the stage of the current stage of the game at the client's side.
		ESTABLISH_CONNECTION,		
		SEND_USERNAME,
		SERVER_APPROVAL,
		DISPLAY_MAIN_MENU,		
		WAIT_OPPONENT,				// Send CLIENT_VERSUS, waiting 30 seconds for response: 
		WAIT_FOR_SETUP_REQUEST,
		CHOOSE_FOUR_DIGITS,
		GUESS_OPPONENTS_DIGITS,
		SEND_GUESS_TO_SERVER_AND_GET_ANSWER,
		ANNOUNCE_WINNER,
		ANNOUNCE_DRAW,
		CLIENT_WANT_OUT,
		CLIENT_ERROR,
		OPPONENT_QUIT
	} client_stage;

	/* ----------------------- */
	/* functions declarations: */
	/* ----------------------- */

	//	Description: This function is the main of the client. it executes the orders instructed
	//	in the project for the client side - connect, send and rcv.
	//	Paramaters: char *ip_addr - The ip address of the server to connect to.
	//              char *port_num_str - A string containing the port number to connect to.
	//              char *user_name - The username of the client.
	//				int num_of_threads - Number of threads to execute the program with.
	//	Returns: EXIT_SUCCESS if succeeded, EXIT_FAILURE if failed.
	int client_main(const char *ip_addr, const char *port_num_str, const char *user_name);
	//	----------------------------------------------------------------------------------------

	//	Description: This function sets-up a connection from the client to the server and keeps
	//				 trying so long the user doesn't wish to exit the program.
	//	Paramaters: char *ip_addr - The ip address of the server to connect to.
	//              char *port_num_str - A string containing the port number to connect to.
	//              char *user_name - The username of the client.
	//				int num_of_threads - Number of threads to execute the program with.
	//				client_stage stage - Current stage of the program.
	//				BOOL first_time - Check if this is the first connection attempt.
	//	Returns: CLIENT_CONTINUE if succeeded and the client wishes to re-connect, CLIENT_EXIT_REQ
	//			 if the user wishes to disconnect.
	int client_setup_connection(const char *ip_addr, const char *port_num_str, const int port_num, SOCKET m_socket, client_stage stage, BOOL first_time);
	//	----------------------------------------------------------------------------------------

	//	Description: This function sets-up a connection from the client to the server.
	//	Paramaters: SOCKET sock - A socket to connect upon.
	//				const char *ip_addr - The IP address of the server to connect to.
	//              u_short port_num - The port number of the server to connect to.
	//	Returns: CONNECTION_ERROR if an error occured, CONNECTION_TIMEOUT if timeout occured
	//			 and CONNECTION_SUCCEEDED if succesfuly connected.
	int connect_non_blocking(SOCKET sock, const char *ip_addr, u_short port_num);
	//	----------------------------------------------------------------------------------------

	//	Description: Communicate with the server - send and then receive messages according to
	//				 the stage the client program is at.
	//	Paramaters: SOCKET m_socket - Connected socket to server.
	//				client_stage stage - Current stage of the program - the "game".
	//              const char *param - A paramater to send to the server.
	//              char **accepted_str - A pointer to a string to accept the received string from the server to.
	//	Returns: TRNS_FAILED if failed, TRNS_DISCONNECTED if timeout or failed too, TRNS_SUCCEEDED if succeeded.
	t_transfer_result communicate_with_server(SOCKET m_socket, client_stage stage, const char *param, char **accepted_str);
	//	----------------------------------------------------------------------------------------

	//	Description: Format a message to send to the server according to the stage the game is at,
	//				 and according to the user response.
	//	Paramaters: client_stage stage - Current stage of the program - the "game".
	//				char *send_buffer - The message to send.
	//              const char *param1 - Extra paramater to send.
	//	Returns: EXIT_SUCCESS if succeeded, EXIT_FAILURE if failed.
	int client_format_message(client_stage stage, char *send_buffer, const char *param1);
	//	----------------------------------------------------------------------------------------

	//	Description: Play the game in loops - each iteration play the current stage of the game,
	//				 and then advance the game.
	//	Paramaters: client_stage stage - Current stage of the program - the "game".
	//				SOCKET m_socket - Connected socket to server.
	//              const char *user_name - Player's username.
	//	Returns: The stage of the program when the user exits or an indication that an error occured.
	client_stage client_play_game(SOCKET m_socket, client_stage stage, const char *user_name);
	//	----------------------------------------------------------------------------------------

	//	Description: Implementing each move cases and all of the advances possible for the game status.
	//	Paramaters: client_stage cur_stage - Current stage of the program - the "game".
	//				SOCKET m_socket - Connected socket to server.
	//				const char *param - An extra paramater to send to server.
	//              char *opponent_user_name - Opponent's username.
	//	Returns: The stage of the program when the user exits or an indication that an error occured.
	client_stage make_move(client_stage cur_stage, SOCKET m_socket, const char *param, char *opponent_user_name);
	//	----------------------------------------------------------------------------------------


#endif // !__CLIENT_MAIN_H__