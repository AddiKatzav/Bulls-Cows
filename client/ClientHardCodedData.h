/*------------------------------- ClientHardCodedData.h --------------------------------*/
/* Description: This header file contains all the macros definitions for the client!    */
/*--------------------------------------------------------------------------------------*/

#ifndef __CLIENT_HARD_CODED_DATA_H__
#define __CLIENT_HARD_CODED_DATA_H__


	#define CLIENT_MAX_MSG_LEN 50

	typedef enum _client_state { CLIENT_CONTINUE, CLIENT_EXIT_REQ, CLIENT_FORCE_EXIT, CLIENT_SERVER_RESPONSE_TIMEOUT } client_state;

#endif // !__CLIENT_HARD_CODED_DATA_H__

