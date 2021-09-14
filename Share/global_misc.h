/*----------------------------------------- global_misc.h ----------------------------------------*/
/* Description: This header file contains all the macros definitions for both server and client   */
/*------------------------------------------------------------------------------------------------*/
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#ifndef __GLOBAL_MISC_H__
#define __GLOBAL_MISC_H__

	/* ----------------- */
	/* Library includes: */
	/* ----------------- */
	#include <stdio.h>
	#include <stdlib.h>
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#pragma comment(lib, "ws2_32.lib")
	#include <windows.h>
	#include <conio.h>
	
	
	/* ----------------- */
	/* Project includes: */
	/* ----------------- */
	#include "SharedHardCodedData.h"
	
	/* ----------------- */
	/* Project defines:  */
	/* ----------------- */
	

	
	#define STRINGS_ARE_EQUAL( Str1, Str2 ) ( strcmp( (Str1), (Str2) ) == 0 )
	
	/* --------------------------------- */
	/* enums and structures declaration: */
	/* --------------------------------- */
	typedef enum _message_types {
		CLIENT_REQUEST,
		CLIENT_VERSUS,
		CLIENT_SETUP,
		CLIENT_PLAYER_MOVE,
		CLIENT_DISCONNECT,
		SERVER_MAIN_MENU,
		SERVER_APPROVED,
		SERVER_DENIED,
		SERVER_INVITE,
		SERVER_SETUP_REQUEST,
		SERVER_PLAYER_MOVE_REQUEST,
		SERVER_GAME_RESULTS,
		SERVER_WIN,
		SERVER_DRAW,
		SERVER_NO_OPPONENTS,
		SERVER_OPPONENT_QUIT
	} message_types;
	
	typedef enum { TRNS_FAILED, TRNS_DISCONNECTED, TRNS_NOP_MSG, TRNS_SUCCEEDED } t_transfer_result;

	typedef enum _non_blocking_ret_val {
		CONNECTION_TIMEOUT,
		CONNECTION_FAILED,
		CONNECTION_ERROR,
		CONNECTION_SUCCEEDED
	} non_blocking_ret_val;

	/* ----------------------- */
	/* functions declarations: */
	/* ----------------------- */

	//	Description: This function deinitiliazes the winsock, and prints an error if one occurs
	//	while deinitializing.
	//	Paramaters: None.
	//	Returns: 0 if succeeded, SOCKET_ERROR if failed.
	int deinit_winsock();
	//	----------------------------------------------------------------------------------------

	//	Description: This function creates a winsock SOCKET and reports an error if one occured
	//				 during the creation.
	//	Paramaters: None.
	//	Returns: A new socket to connect to if succeeded, INVALID_SOCKET if failed.
	SOCKET create_socket();
	//	----------------------------------------------------------------------------------------

	//	Description: This function closes the socket specified and prints an error if one occured
	//				 during closing the socket
	//	Paramaters: SOCKET m_socket - The socket to close.
	//	Returns: 0 if succeeded, SOCKET_ERROR if failed.
	int close_socket(SOCKET m_socket);
	//	----------------------------------------------------------------------------------------

	//	Description: This function closes the socket specified and prints an error if one occured
	//				 during closing the socket. It also deinitializes the winsock and prints an
	//				 error if one occured.
	//	Paramaters: SOCKET m_socket - The socket to close.
	//	Returns: 0 if succeeded, SOCKET_ERROR if failed.
	int close_socket_and_deinit_wsa(SOCKET m_socket);
	//	----------------------------------------------------------------------------------------

	//	Description: This function closes the socket specified and prints an error if one occured
	//				 during closing the socket. It also creates a new socket to connect to. All in
	//				 this function uses for re-connection to the server if the connection is lost
	//				 or timed-out.
	//	Paramaters: SOCKET m_socket - The socket to close.
	//	Returns: A new socket to connect to if succeeded, INVALID_SOCKET if failed.
	SOCKET close_and_create_socket(SOCKET m_socket);
	//	----------------------------------------------------------------------------------------

	//	Description: Using this function one can set the timeout of the socket for sending
	//				 or receiving messages. It also prints an error if one occured.
	//	Paramaters: SOCKET m_socket - The socket to set options to.
	//				int timeout - The timeout specification.
	//				BOOL send - If TRUE set send timeout, if FALSE set receive timeout.
	//	Returns: A new socket to connect to if succeeded, SOCKET_ERROR if failed.
	int set_socket_timeout(SOCKET m_socket, int timeout, BOOL send);
	//	----------------------------------------------------------------------------------------

	t_transfer_result send_string(const char *str, SOCKET m_socket);
	//	----------------------------------------------------------------------------------------
	t_transfer_result send_buffer(const char *buffer, int bytes_to_send, SOCKET m_socket);
	//	----------------------------------------------------------------------------------------
	t_transfer_result receive_buffer(char *output_buffer, int bytes_to_receive, SOCKET m_socket);
	//	----------------------------------------------------------------------------------------
	t_transfer_result receive_string(char **output_str, SOCKET m_socket);
	//	----------------------------------------------------------------------------------------
	int parse_message(char *msg, char *message_type, char *param1, char *param2, char *param3, char *param4);
	//	----------------------------------------------------------------------------------------
	int set_blocking(SOCKET m_socket, BOOL is_blocking);
	//	----------------------------------------------------------------------------------------

	//	Description: Check if port number received from the user is legit and convert it from
	//				 a string to an integer.
	//	Paramaters: char *argv_port - The argv containing the port number to connect to.
	//	Returns: port number if succeeded, -1 if failed.
	int check_port_num(const char *argv_port);
	//	----------------------------------------------------------------------------------------

	//	Description: Initialize WSA.
	//	Paramaters: NONE.
	//	Returns: EXIT_SUCCESS if succeeded, EXIT_FAILURE if failed.
	int init_winsock();
	//	----------------------------------------------------------------------------------------
	error_code_t waiting_for_sync_object(HANDLE handle_to_sync_obj, sync_object_t sync_object_type, DWORD wait_time);
	//	----------------------------------------------------------------------------------------
	error_code_t release_sync_object(HANDLE handle_to_sync_obj, sync_object_t sync_object_type);
	//	----------------------------------------------------------------------------------------
	void map_sync_object_to_str(sync_object_t sync_object_type, char *error_str);
	//	----------------------------------------------------------------------------------------

	//	Description: Check if a given file exists given the path.
	//	Paramaters: LPCTSTR szPath - string containing path to file.
	//	Returns: TRUE if file exists, FALSE if file doesn't exist.
	BOOL FileExists(LPCTSTR szPath);
	//  ----------------------------------------------------------------------------------------

	//	Description: Check if a given file exists given the path, and delete it.
	//	Paramaters: NONE,
	//	Returns: EXIT_SUCCESS if succeeded, EXIT_FAILURE if failed.
	int reset_file_state();
	//  ----------------------------------------------------------------------------------------
	BOOL check_EOF(DWORD dword_bytes_read, DWORD read_result);

#endif // !__GLOBAL_MISC_H__