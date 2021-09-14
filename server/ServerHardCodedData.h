/*------------------------------- ServerHardCodedData.h --------------------------------*/
/* Description: This header file contains all the macros definitions for the server!    */
/*--------------------------------------------------------------------------------------*/

#ifndef __SERVER_HARD_CODED_DATA_H__
#define __SERVER_HARD_CODED_DATA_H__

#define SERVER_ADDRESS_STR "0.0.0.0"
#define SERVER_MAX_MSG_LEN 54
#define MSG_TOKENS_NUM 5

#define CHECK_EXIT_TIME 1


typedef enum _server_stage {
	GET_USERNAME_SEND_APPROVAL,
	SEND_DISPLAY,
	CLIENT_WAITS_FOR_OPPONENT,
	RECEIVE_INITIAL_GUESS_FROM_CLIENT,
	GET_TO_KNOW,
	GAME_ON,
	REPORT_RESULTS,
	CANDIDATE_TO_WIN,
	REPORT_WINNER,
	REPORT_DRAW,
	SHOW_CLIENT_OUT,			// Client is disconnected from the server
	SERVER_KILL_CLIENT_THREAD,	// Thread error
	SERVER_ERROR,				// Unforgiveable failure, close all
	OPPONENT_QUIT
} server_stage;

typedef struct _thread_paramaters {
	BOOL third;
	SOCKET *t_socket;
	HANDLE thread_mutex;
	HANDLE first_event;
	HANDLE second_event;
	int *ready_to_play;
} thread_paramaters;

typedef struct _exit_paramaters {
	thread_paramaters **thread_params;
	HANDLE *thread_handles;
	HANDLE *exit_thread;
} exit_paramaters;

#endif // !__SERVER_HARD_CODED_DATA_H__

