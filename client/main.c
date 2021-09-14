/*--------------------------------------------------------------------------------------*/
/*-------------------------------------- Authors: --------------------------------------*/
/*----------------------------- Addi Katzav, ID: 313903015 -----------------------------*/
/*----------------------------- Aviram Dick, ID: 205640832 -----------------------------*/
/*----------------------------------- Project: client ----------------------------------*/
/* Description: The main function mostly handles with checking command line arguments 	*/
/* and then sending it to the main thread.                                           	*/
/* This is the client's project. The game is implemented here at a point of view of the */
/* client. At each stage we send and receive the information to/from the client and		*/
/* execute an order according to the current stage.										*/		
/*--------------------------------------------------------------------------------------*/


/* ----------------- */
/* Library includes: */
/* ----------------- */

/* ----------------- */
/* Project includes: */
/* ----------------- */
//#include "global_misc.h"
#include "client_main.h"

/* ------------------------ */
/* function implemantations */
/* ------------------------ */

int main(int argc, char *argv[])
{
	if (argc != 4)
	{
		printf_s("Usage Error:\tIncorrect number of arguments\n\n");
		return EXIT_FAILURE;
	}
	char *server_ip = argv[1], *user_name = argv[3], *port_num_str = argv[2];
	
	return client_main(server_ip, port_num_str, user_name);
	
}