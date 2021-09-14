/*--------------------------------- SharedHardCodedData.h ---------------------------------*/
/* Description: This header file contains all the macros definitions for server and client */
/*-----------------------------------------------------------------------------------------*/

#ifndef __SHARED_HARD_CODED_DATA_H__
#define __SHARED_HARD_CODED_DATA_H__

	/* ----------------- */
	/*  Project macros:	 */
	/* ----------------- */
	#define MAX_USERNAME_LEN 20
	#define MAX_PLAYERS 2
	#define MAX_CONNECTIONS 3
	#define MAX_MSG_LEN 50
	#define MAX_USER_CHOICE_STR_LEN 50
	
	#define CLIENT_CONNECT_TIMEOUT 15
	#define CLIENT_SEND_TIMEOUT 15000 //2F - change back to 15 sec
	#define CLIENT_RECV_TIMEOUT 15000	//2F - change back to 15 sec
	
	#define WAIT_TIME_FOR_HUMAN 600000
	#define SERVER_SEND_TIMEOUT 15000 //2F - change back to 15 sec
	#define SERVER_RECV_TIMEOUT 15000 //2F - change back to 15 sec
	
	#define MAX_ERROR_STR_LEN 40
	#define MUTEX_WAIT_TIMEOUT 15000	//2F - change back to 15 sec
	#define EVENT_WAIT_TIMEOUT 15000	//2F - change back to 15 sec
	
	#define MUTEX_HUMAN_WAIT_TIMEOUT 600000
	#define EVENT_HUMAN_WAIT_TIMEOUT 600000
	
	/* ----------------- */
	/*   Project enums:	 */
	/* ----------------- */

	typedef enum _error_code {	//	This enum is used for error codes as return values of releasing or waiting for a mutex or a semaphore.
		SUCCESS,
		OPEN_FAILED,
		CREATE_FAILED,
		WAITING_FAILED,
		WAITING_TIMEOUT,
		ABANDONED,
		RELEASE_FAILED
	} error_code_t;
	
	typedef enum _sync_object {	// This enum represent the synchroneous object that we are going to wait for, in order to correctly print an error when it occurs.
		MUTEX,
		FIRST_EVENT,
		SECOND_EVENT,
		EXIT_THREAD,
		ANY_THREAD
	} sync_object_t;

#endif // !__SHARED_HARD_CODED_DATA_H__