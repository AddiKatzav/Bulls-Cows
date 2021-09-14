/*------------------------------------ server_main.h -----------------------------------*/
/* Description: This source file contains all the functions that are used by the server */
/*--------------------------------------------------------------------------------------*/

/* ----------------- */
/* Project includes: */
/* ----------------- */
#include "server_main.h"


SOCKET main_socket = INVALID_SOCKET;				// main socket to listen on.
HANDLE *thread_handles = NULL;						// an array of the threads' handles.
thread_paramaters **g_thread_params = NULL;			// an array of pointers to the paramaters' structs.
HANDLE *exit_thread = NULL;							// pointer to the thread who is in chrage of listening to exit request.
int *ready_to_play = NULL;							// pointer to an integer counting the number of players that want to play against opponent.
int connected_clients = 0;							// number of connected clients to the server.

/* ------------------------ */
/* function implemantations */
/* ------------------------ */

int server_main(int server_port)
{
	int ind = 0, server_setup_res = 0;
	HANDLE thread_mutex = NULL, first_event = NULL, second_event = NULL;
	BOOL done = FALSE, third = FALSE;


	if (NULL == (thread_handles = (HANDLE *)malloc(sizeof(HANDLE) * MAX_CONNECTIONS))) {
		printf_s("Could not allocate room in memory for THREAD HANDLES ARRAY. Terminating...\n");
		return EXIT_FAILURE;
	}
	init_thread_handles(thread_handles);
	if (NULL == (g_thread_params = (thread_paramaters**)malloc(sizeof(thread_paramaters*) * MAX_CONNECTIONS))) {
		printf_s("Could not allocate room in memory for THREAD PARAMATERS ARRAY. Terminating...\n");
		free(thread_handles);
		return EXIT_FAILURE;
	}
	init_thread_params(g_thread_params);
	if (NULL == (exit_thread = (HANDLE*)malloc(sizeof(HANDLE)))) {
		printf_s("Could not allocate room in memory for thread. Terminating...\n");
		free_and_close_handles(NULL, NULL, NULL, NULL, thread_handles, g_thread_params, NULL);
		return EXIT_FAILURE;
	}
	if (EXIT_FAILURE == create_exit_check_thread(exit_thread)) { free_and_close_handles(NULL, NULL, NULL, NULL, thread_handles, g_thread_params, NULL);	return EXIT_FAILURE; }
	if (EXIT_FAILURE == reset_file_state()){ free_and_close_handles(NULL, NULL, NULL, NULL, thread_handles, g_thread_params, exit_thread);	return EXIT_FAILURE; }
	if (EXIT_FAILURE == init_winsock()) { free_and_close_handles(NULL, NULL, NULL, NULL, thread_handles, g_thread_params, exit_thread);	return EXIT_FAILURE; }
	if (INVALID_SOCKET == (main_socket = create_socket())) {
		free_and_close_handles(NULL, NULL, NULL, NULL, thread_handles, g_thread_params, exit_thread);
		deinit_winsock();
		return EXIT_FAILURE;
	}
	if (EXIT_FAILURE == (server_setup_res = setup_server_connection(main_socket, server_port))) return EXIT_FAILURE;
	if (NULL == (thread_mutex = CreateMutexA(NULL, FALSE, NULL))) {
		printf_s("Creating mutex failed, with error: %li. Terminating...\n", GetLastError());
		close_socket_and_deinit_wsa(main_socket);
		free_and_close_handles(thread_mutex, NULL, NULL, NULL, thread_handles, g_thread_params, exit_thread);
		return EXIT_FAILURE;
	}
	if (NULL == (first_event = CreateEventA(NULL, FALSE, FALSE, NULL))) { // Create an auto-reset event, initial state is non-signaled
		printf_s("Creating first_event failed, with error: %li. Terminating...\n", GetLastError());
		free_and_close_handles(thread_mutex, thread_mutex, NULL, NULL, thread_handles, g_thread_params, exit_thread);
		close_socket_and_deinit_wsa(main_socket);
		return EXIT_FAILURE;
	}
	if (NULL == (second_event = CreateEventA(NULL, FALSE, FALSE, NULL))) { // Create an auto-reset event, initial state is non-signaled
		printf_s("Creating second_event failed, with error: %li. Terminating...\n", GetLastError());
		free_and_close_handles(thread_mutex, first_event, NULL, NULL, thread_handles, g_thread_params, exit_thread);
		close_socket_and_deinit_wsa(main_socket);
		return EXIT_FAILURE;
	}
	if (NULL == (ready_to_play = malloc(sizeof(int)))) {
		printf_s("Could not allocate room in memory for socket. Terminating...\n");
		free_and_close_handles(thread_mutex, first_event, second_event, NULL, thread_handles, g_thread_params, exit_thread);
		close_socket_and_deinit_wsa(main_socket);
		return EXIT_FAILURE;
	}
	*ready_to_play = 0;
	printf_s("Waiting for a client to connect...\n");
	int ret_val;
	while (!done)
	{
		SOCKET *accept_socket = malloc(sizeof(SOCKET));	// 2F - free this in service thread!
		if (NULL == accept_socket) { // 2F - close socket, deinit winsock, return
			printf_s("Could not allocate room in memory for socket. Terminating...\n");
			ret_val = EXIT_FAILURE;
			break;
		}

		*accept_socket = accept(main_socket, NULL, NULL);
		if (*accept_socket == INVALID_SOCKET)
		{
			free(accept_socket);
			ret_val = EXIT_SUCCESS;
			break;
		}
		connected_clients += 1;
		if ((set_socket_timeout(*accept_socket, SERVER_SEND_TIMEOUT, TRUE)) || (set_socket_timeout(*accept_socket, SERVER_RECV_TIMEOUT, FALSE))) {
			close_socket(*accept_socket);
			free(accept_socket);
			ret_val = EXIT_FAILURE;
			break;
		}
		if (EXIT_FAILURE == refresh_threads_array(thread_handles, g_thread_params, &connected_clients)) break; 
		ind = find_first_unused_thread_slot(thread_handles, g_thread_params, &connected_clients);

		if (ind == -1)	// -1 returned when one of the threads didn't 
		{
			close_socket(*accept_socket);
			free(accept_socket);
			ret_val = EXIT_FAILURE;
			break;
		}
		else if (connected_clients > MAX_PLAYERS)
			third = TRUE;
		else {
			third = FALSE;
			printf_s("Client Connected.\n");
		}

		if (NULL == (*(g_thread_params + ind) = fill_thread_params(third, accept_socket, thread_mutex, first_event, second_event, ready_to_play))) {
			close_socket(*accept_socket);
			ret_val = EXIT_FAILURE;
			break;
		}
		if (NULL == (*(thread_handles + ind) = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)service_thread, *(g_thread_params + ind), 0, NULL))) {	//2F check if successful
			printf_s("Error no.%d, when creating one of the threads, Terminating...\n", GetLastError());
			close_socket(*accept_socket);
			ret_val = EXIT_FAILURE;
			break;
		}
	}

	free_and_close_handles(thread_mutex, first_event, second_event, ready_to_play, thread_handles, g_thread_params, exit_thread);
	if (EXIT_FAILURE == deinit_winsock())	return EXIT_FAILURE;
	return EXIT_SUCCESS;
}
//	----------------------------------------------------------------------------------------
static int find_first_unused_thread_slot(HANDLE *thread_handles, thread_paramaters **thread_params, int *num_of_players)
{
	int ind;

	for (ind = 0; ind < MAX_CONNECTIONS; ind++)
	{
		if (*(thread_handles + ind) == NULL) {	// 2F - make sure that after you finish with a thread you assign NULL to it
			break;
		}
		else
		{
			// poll to check if thread finished running:
			DWORD res = WaitForSingleObject(*(thread_handles + ind), 0);
			if (res == WAIT_OBJECT_0) // this thread finished running
			{
				CloseHandle(*(thread_handles + ind));	//2F - close gracefully
				free(*(thread_params + ind));
				*(thread_handles + ind) = NULL;
				*num_of_players -= 1;
				break;
			}
			else if (res == WAIT_FAILED) {	// wait failed - not safe
				printf_s("Thread wait failed. Critical Error, terminating...\n");
				CloseHandle(*(thread_handles + ind));	//2F - close gracefully
				ind = -1;
				*num_of_players -= 1;
				break;
			}
			else if (res == WAIT_ABANDONED) {	// abandoned
				printf_s("Thread abandoned!!! Critical Error, terminating...\n");
				CloseHandle(*(thread_handles + ind));
				free(*(thread_params + ind));
				ind = -1;
				*num_of_players -= 1;
				break;
			}
		}
	}

	return ind;
}
//	----------------------------------------------------------------------------------------
void init_thread_handles(HANDLE *thread_handles)
{
	for (int ind = 0; ind < MAX_CONNECTIONS; ind++)
		*(thread_handles + ind) = NULL;
}
//	----------------------------------------------------------------------------------------
thread_paramaters *fill_thread_params(BOOL third, SOCKET *s, HANDLE thread_mutex, HANDLE first_event, HANDLE second_event, int *versus_players)
{
	thread_paramaters *p_thread_paramaters = NULL;
	if (NULL == ((p_thread_paramaters = malloc(sizeof(thread_paramaters)))))
	{
		printf_s("Could not allocate room for thread paramaters. Terminating...\n");
		return NULL;
	}
	p_thread_paramaters->third = third;
	p_thread_paramaters->t_socket = s;
	p_thread_paramaters->thread_mutex = thread_mutex;
	p_thread_paramaters->first_event = first_event;
	p_thread_paramaters->second_event = second_event;
	p_thread_paramaters->ready_to_play = versus_players;
	return p_thread_paramaters;
}
//	----------------------------------------------------------------------------------------
void init_thread_params(thread_paramaters *thread_params[])
{
	for (int ind = 0; ind < MAX_CONNECTIONS; ind++)
		*(thread_params + ind) = NULL;
}
//	----------------------------------------------------------------------------------------
static DWORD service_thread(LPVOID lp_param)
{
	thread_paramaters *p_thread_params;
	if (NULL == lp_param)
		return EXIT_FAILURE;

	p_thread_params = (thread_paramaters *)lp_param;

 	if (p_thread_params->third == TRUE) {	// Third player connected
		service_third_player(*(p_thread_params->t_socket)); // 2F check return value
		return EXIT_SUCCESS;
	}

	// Players routine
	char user_name[MAX_USERNAME_LEN], user_initial_number[MAX_USER_CHOICE_STR_LEN];
	char opponent_name[MAX_USERNAME_LEN], opponent_initial_number[MAX_USER_CHOICE_STR_LEN];
	BOOL done = FALSE;
	server_stage stage = GET_USERNAME_SEND_APPROVAL;
	
	while (!done) {
		server_manage_game(*(p_thread_params->t_socket), &stage, p_thread_params, user_name, opponent_name, user_initial_number, opponent_initial_number);
		if (stage == SERVER_KILL_CLIENT_THREAD || stage == SERVER_ERROR || stage == SHOW_CLIENT_OUT)	break;
	}

	if (stage == SERVER_ERROR || stage == SERVER_KILL_CLIENT_THREAD || stage == SHOW_CLIENT_OUT) {
		close_socket(*(p_thread_params->t_socket));
		free((p_thread_params->t_socket));
		int ind = 0;
		for (ind = 0; ind < MAX_CONNECTIONS; ind++) {
			if (p_thread_params == g_thread_params[ind])
				break;
		}
		free(p_thread_params);
		//free(*(g_thread_params + ind));
		p_thread_params = NULL;
		*(g_thread_params + ind) = NULL;
		if (stage == SERVER_ERROR || stage == SERVER_KILL_CLIENT_THREAD)
			return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
//	----------------------------------------------------------------------------------------
int setup_server_connection(SOCKET main_socket, int server_port)
{
	unsigned long address = inet_addr(SERVER_ADDRESS_STR);
	if (address == INADDR_NONE) { // HANDLED PROPERLY
		printf_s("The string \"%s\" cannot be converted into an ip address. ending program.\n", SERVER_ADDRESS_STR);
		close_socket_and_deinit_wsa(main_socket);
		return EXIT_FAILURE;
	}

	SOCKADDR_IN service;
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = address;
	service.sin_port = htons(server_port);

	int bind_res = bind(main_socket, (SOCKADDR*)&service, sizeof(service));
	if (bind_res == SOCKET_ERROR) {
		printf_s("bind() failed with error %ld. Ending program\n", WSAGetLastError());
		close_socket_and_deinit_wsa(main_socket);
		return EXIT_FAILURE;
	}

	int listen_res = listen(main_socket, SOMAXCONN);
	if (listen_res == SOCKET_ERROR) {
		printf_s("Failed listening on socket, error %ld.\n", WSAGetLastError());
		close_socket_and_deinit_wsa(main_socket);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
//	----------------------------------------------------------------------------------------
void server_manage_game(SOCKET client_socket, server_stage *stage, thread_paramaters *thread_params, char user_name[], char opponent_name[], char *user_initial_number, char *opponent_initial_number)
{
	char msg_tokens[MSG_TOKENS_NUM][MAX_MSG_LEN];	// four paramaters and one message type
	
	switch (*stage) {
	case (GET_USERNAME_SEND_APPROVAL):
		*stage = stage_get_username_send_approval(client_socket, *stage, msg_tokens, user_name);
		break;
	case (SEND_DISPLAY):
		*stage = stage_send_menu_display(client_socket, *stage, thread_params, msg_tokens);
		break;
	case (CLIENT_WAITS_FOR_OPPONENT):
		*stage = stage_find_opponent(client_socket, *stage, msg_tokens, thread_params, user_name);
		break;
	case (RECEIVE_INITIAL_GUESS_FROM_CLIENT):
		*stage = stage_receive_initial_choice(client_socket, *stage, msg_tokens, user_initial_number, thread_params);
		break;
	case (GET_TO_KNOW):
		*stage = stage_get_to_know(*stage, thread_params, user_name, opponent_name, user_initial_number, opponent_initial_number);
		break;
	case (GAME_ON): // all strings are filled so far
		*stage = stage_play_round(client_socket, *stage, thread_params, user_name, opponent_name, user_initial_number, opponent_initial_number);
		break;
	case (REPORT_WINNER):
	case (REPORT_DRAW):
		*stage = SEND_DISPLAY;
		break;
	}

}
//	----------------------------------------------------------------------------------------
void free_and_close_handles(HANDLE handle1, HANDLE handle2, HANDLE handle3, int *ptr, HANDLE *handles_array, thread_paramaters **thread_params, HANDLE *exit_thread)
{
	DWORD exit_code;
	if (handle1)
		CloseHandle(handle1);
	if (handle2)
		CloseHandle(handle2);
	if (handle3)
		CloseHandle(handle3);
	if (ptr)
		free(ptr);
	if (thread_params) {
		for (int ind = 0; ind < MAX_CONNECTIONS; ind++) {
			if (*(handles_array + ind) != NULL) {
				GetExitCodeThread(*(handles_array + ind), &exit_code);
				if (exit_code == STILL_ACTIVE)
					continue;
				else {
					CloseHandle(*(thread_handles + ind));
					//free(*(thread_params + ind));
					*(thread_params + ind) = NULL;
				}
			}
		}
	}
	if (handles_array)
		free(handles_array);
	if (exit_thread) {
		CloseHandle(*exit_thread);
		free(exit_thread);
	}
		
}
//	----------------------------------------------------------------------------------------
int create_exit_check_thread(HANDLE *exit_thread)
{
	if (NULL == (*exit_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)check_for_exit_routine, NULL, 0, NULL))) {
		printf_s("Error no.%d, when creating exit thread, Terminating...\n", GetLastError());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
//	----------------------------------------------------------------------------------------
static DWORD check_for_exit_routine()
{
	char str[MAX_USER_CHOICE_STR_LEN];
	while (TRUE)
	{
		if (_kbhit()) {
			gets_s(str, MAX_USER_CHOICE_STR_LEN);
			if (STRINGS_ARE_EQUAL(str, "exit") || STRINGS_ARE_EQUAL(str, "EXIT")) {
				break;
			}
		}
		else {
			//if (EXIT_FAILURE == refresh_threads_array_middle(thread_handles, NULL, ready_to_play)) return EXIT_FAILURE;
			Sleep(CHECK_EXIT_TIME);
		}
	}
	close_socket(main_socket);
	printf_s("Closing server...\n");
	return EXIT_SUCCESS;
}
