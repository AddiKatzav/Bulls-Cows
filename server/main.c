/*--------------------------------------------------------------------------------------*/
/*-------------------------------------- Authors: --------------------------------------*/
/*----------------------------- Addi Katzav, ID: 313903015 -----------------------------*/
/*----------------------------- Aviram Dick, ID: 205640832 -----------------------------*/
/*----------------------------------- Project: server ----------------------------------*/
/* Description: The main function mostly handles with checking command line arguments 	*/
/* and then sending it to the main thread.                                           	*/
/* This project implements the game from the server point of view. The server accepts   */
/* new connections from clients and then when two clients are connected the server      */   
/* starts a game between them.                                                          */        
/*--------------------------------------------------------------------------------------*/


/* ----------------- */
/* Library includes: */
/* ----------------- */

/* ----------------- */
/* Project includes: */
/* ----------------- */
//#include "global_misc.h"
#include "server_main.h"

/* ------------------------ */
/* function implemantations */
/* ------------------------ */

int main(int argc, char *argv[])
{
    int port_num;

    if (argc != 2)
    {
        printf_s("Usage Error:\tIncorrect number of arguments\n\n");
        return EXIT_FAILURE;
    }

    if ((port_num = check_port_num(argv[1])) == -1)
        return EXIT_FAILURE;

	return server_main(port_num);
	
}

