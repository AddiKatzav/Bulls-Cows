/*-----------------------------------------global_misc.h------------------------------------------*/
/* Description: This header file contains all the macros definitions for both server and client   */
/*------------------------------------------------------------------------------------------------*/

/* ----------------- */
/* Project includes: */
/* ----------------- */
#include "global_misc.h"


/* ------------------------ */
/* function implemantations */
/* ------------------------ */

int init_winsock()
{
	WSADATA wsa_data;
	int startup_res = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (startup_res != NO_ERROR) {	//HANDLED PROPERLY
		printf_s("Error %ld at WSAStartup( ) for Server, ending program.\n", WSAGetLastError());	// Tell the user that we could not find a usable WinSock DLL.                                  
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
//	----------------------------------------------------------------------------------------
int deinit_winsock()
{
	if (WSACleanup() == SOCKET_ERROR) {
		printf("Failed to deinitialize Winsocket, error %ld. Ending program.\n", WSAGetLastError());
		return SOCKET_ERROR;
	}
	return 0;
}
//	----------------------------------------------------------------------------------------
SOCKET create_socket()
{
	SOCKET m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_socket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		//if (SOCKET_ERROR == WSACleanup())	printf_s("Could not de-initialize WSA, with error %ld\n", WSAGetLastError());
		return INVALID_SOCKET;
	}
	return m_socket;
}
//	----------------------------------------------------------------------------------------
int close_socket(SOCKET m_socket)
{
	int close_res = closesocket(m_socket);
	if (close_res == SOCKET_ERROR) {
		printf_s("Error no.%ld when trying to close socket\n", WSAGetLastError());
		return SOCKET_ERROR;
	}
	return 0;
}
//	----------------------------------------------------------------------------------------
int close_socket_and_deinit_wsa(SOCKET m_socket)
{
	if (close_socket(m_socket) || deinit_winsock())
		return SOCKET_ERROR;
	return 0;
}
//	----------------------------------------------------------------------------------------
SOCKET close_and_create_socket(SOCKET m_socket)
{
	int close_res = close_socket(m_socket);
	if (close_res == SOCKET_ERROR) {
		deinit_winsock();
		return INVALID_SOCKET;
	}
	SOCKET new_socket = create_socket();
	if (new_socket == INVALID_SOCKET) {
		deinit_winsock();
		return INVALID_SOCKET;
	}
	if ((set_socket_timeout(new_socket, CLIENT_SEND_TIMEOUT, TRUE)) || (set_socket_timeout(new_socket, CLIENT_RECV_TIMEOUT, FALSE))) {
		close_socket_and_deinit_wsa(new_socket);
		return INVALID_SOCKET;	// deinit_winsock() prints the error
	}
	return new_socket;
}
//	----------------------------------------------------------------------------------------
int set_socket_timeout(SOCKET m_socket, int timeout, BOOL send)
{
	int set_sock_res;
	if (send)	set_sock_res = setsockopt(m_socket, SOL_SOCKET, SO_SNDTIMEO, (char *)&(timeout), sizeof(int));
	else		set_sock_res = setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&(timeout), sizeof(int));

	if (set_sock_res)
		printf_s("Failed to set socket option, with error no.%ld. Terminating...\n", WSAGetLastError());
	return set_sock_res;
}
//	----------------------------------------------------------------------------------------
t_transfer_result send_string(const char *str, SOCKET m_socket)
{
	/* Send the the request to the server on socket */
	int total_string_size_in_bytes;
	t_transfer_result send_res;

	/* The request is sent in two parts. First the Length of the string (stored in
	   an int variable), then the string itself. */

	total_string_size_in_bytes = (int)(strlen(str) + 1); // terminating zero also sent	

	send_res = send_buffer((const char *)(&total_string_size_in_bytes), (int)(sizeof(total_string_size_in_bytes)) /* sizeof(int)*/, m_socket);

	if (send_res != TRNS_SUCCEEDED) return send_res;

	send_res = send_buffer((const char *)(str), (int)(total_string_size_in_bytes), m_socket);

	return send_res;
}
//	----------------------------------------------------------------------------------------
t_transfer_result send_buffer(const char *buffer, int bytes_to_send, SOCKET m_socket)
{
	const char* curr_place_ptr = buffer;
	int bytes_transferred;
	int remaining_bytes_to_send = bytes_to_send;

	while (remaining_bytes_to_send > 0)
	{
		/* send does not guarantee that the entire message is sent */
		bytes_transferred = send(m_socket, curr_place_ptr, remaining_bytes_to_send, 0);
		if ((bytes_transferred == SOCKET_ERROR) && (WSAGetLastError() != WSAETIMEDOUT)) {
			printf_s("send() failed, error %ld\n", WSAGetLastError());
			return TRNS_DISCONNECTED;
		}
		else if ((bytes_transferred == SOCKET_ERROR) && (WSAGetLastError() == WSAETIMEDOUT)) {
			printf_s("send() timeout, error %ld\n", WSAGetLastError());
			return TRNS_DISCONNECTED;
		}
		else if (bytes_transferred == 0)														return TRNS_NOP_MSG; // send() returns zero if connection was gracefully disconnected.

		remaining_bytes_to_send -= bytes_transferred;
		curr_place_ptr += bytes_transferred; // <ISP> pointer arithmetic
	}

	return TRNS_SUCCEEDED;
}
//	----------------------------------------------------------------------------------------
t_transfer_result receive_string(char **output_str, SOCKET m_socket)
{
	/* Recv the the request to the server on socket sd */
	int total_string_size_in_bytes;
	t_transfer_result recv_res;
	char *str_buffer = NULL;

	if ((output_str == NULL) || (*output_str != NULL))
	{
		/*printf("The first input to ReceiveString() must be "
			"a pointer to a char pointer that is initialized to NULL. For example:\n"
			"\tchar* Buffer = NULL;\n"
			"\tReceiveString( &Buffer, ___ )\n");*/
		return TRNS_FAILED;
	}

	/* The request is received in two parts. First the Length of the string (stored in
	   an int variable ), then the string itself. */

	recv_res = receive_buffer((char *)(&total_string_size_in_bytes), (int)(sizeof(total_string_size_in_bytes)) /*4 bytes*/, m_socket);

	if (recv_res != TRNS_SUCCEEDED) return recv_res;

	str_buffer = (char*)malloc(total_string_size_in_bytes * sizeof(char));
	if (str_buffer == NULL) {
		printf_s("Failed allocating room in memory for a message buffer. Terminating...\n");
		return TRNS_FAILED;
	}

	recv_res = receive_buffer((char *)(str_buffer), (int)(total_string_size_in_bytes), m_socket);

	if (recv_res == TRNS_SUCCEEDED)
	{
		*output_str = str_buffer;
	}
	else
	{
		free(str_buffer);
	}

	return recv_res;
}
//	----------------------------------------------------------------------------------------
t_transfer_result receive_buffer(char *output_buffer, int bytes_to_receive, SOCKET m_socket)
{
	char* curr_place_ptr = output_buffer;
	int bytes_just_transferred;
	int remaining_bytes_to_receive = bytes_to_receive;

	while (remaining_bytes_to_receive > 0)
	{
		/* send does not guarantee that the entire message is sent */
		bytes_just_transferred = recv(m_socket, curr_place_ptr, remaining_bytes_to_receive, 0);
		if ((bytes_just_transferred == SOCKET_ERROR) && (WSAGetLastError() != WSAETIMEDOUT))
		{
			printf("recv() failed, error %d\n", WSAGetLastError());
			return TRNS_DISCONNECTED;
		}
		else if ((bytes_just_transferred == SOCKET_ERROR) && (WSAGetLastError() == WSAETIMEDOUT)) {

			printf("recv() timeout, error %d\n", WSAGetLastError());
			return TRNS_DISCONNECTED;
		}
		else if (bytes_just_transferred == 0)														return TRNS_NOP_MSG; // recv() returns zero if connection was gracefully disconnected.

		remaining_bytes_to_receive -= bytes_just_transferred;
		curr_place_ptr += bytes_just_transferred; // <ISP> pointer arithmetic
	}

	return TRNS_SUCCEEDED;
}
//	----------------------------------------------------------------------------------------
int set_blocking(SOCKET m_socket, BOOL is_blocking)
{
	unsigned long non_blocking = 1;
	unsigned long blocking = 0;
	int result = ioctlsocket(m_socket, FIONBIO, is_blocking ? &blocking : &non_blocking);
	if (result == SOCKET_ERROR)
		printf_s("Error no.%ld in using ioctlsocket(). Terminating...\n", WSAGetLastError());
	return result;
}
//	----------------------------------------------------------------------------------------
int parse_message(char *msg, char *message_type, char *param1, char *param2, char *param3, char *param4) 
{
	int i = 0;
	if (msg == NULL)
		return EXIT_FAILURE;
	while (TRUE) {
		while ((*msg != ':') && (*msg != '\n')) {
			*(message_type + i) = *msg;
			msg++;
			i++;
		}
		*(message_type + i) = '\0';
		if (*msg == '\n')	// end of msg, no paramaters
			break;
		i = 0;
		msg++;

		while ((*msg != ';') && (*msg != '\n')) {
			*(param1 + i) = *msg;
			msg++;
			i++;
		}
		*(param1 + i) = '\0';
		if (*msg == '\n')	// end of msg, 1 paramater
			break;
		i = 0;
		msg++;

		while ((*msg != ';') && (*msg != '\n')) {
			*(param2 + i) = *msg;
			msg++;
			i++;
		}
		*(param2 + i) = '\0';
		if (*msg == '\n')	// end of msg, 2 paramaters
			break;
		i = 0;
		msg++;

		while ((*msg != ';') && (*msg != '\n')) {
			*(param3 + i) = *msg;
			msg++;
			i++;
		}
		*(param3 + i) = '\0';
		if (*msg == '\n')	// end of msg, 3 paramaters
			break;
		i = 0;
		msg++;

		while ((*msg != ';') && (*msg != '\n')) {
			*(param4 + i) = *msg;
			msg++;
			i++;
		}
		*(param4 + i) = '\0';
		if (*msg == '\n')	// end of msg, 1 paramater
			break;
		i = 0;
		msg++;
	}
	//free(msg);
	return EXIT_SUCCESS;
}
//	----------------------------------------------------------------------------------------
non_blocking_ret_val connect_non_blocking(SOCKET sock, const char *ip_addr, u_short port_num)
{
	int ret = 0;

	SOCKADDR_IN server = { 0 };
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(ip_addr);
	server.sin_port = htons(port_num);

	// ipaddr valid?
	if (server.sin_addr.s_addr == INADDR_NONE) {
		printf_s("Error in converting ip address. Terminating...\n");
		return CONNECTION_ERROR;
	}

	// put socket in non-blocking mode...
	if (SOCKET_ERROR == set_blocking(sock, FALSE))
		return SOCKET_ERROR;

	if (connect(sock, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			// connection failed
			//close_socket_and_deinit_wsa(sock);
			printf_s("Making the connect function a non-blocking one failed, %ld. Terminating...\n", WSAGetLastError());
			return CONNECTION_ERROR;
		}

		// connection pending

		fd_set setW;

		FD_ZERO(&setW);
		FD_SET(sock, &setW);
		//FD_ZERO(&setE);
		//FD_SET(sock, &setE);

		TIMEVAL time_out = { 0 };
		time_out.tv_sec = CLIENT_CONNECT_TIMEOUT;
		time_out.tv_usec = 0;

		ret = select(0, NULL, &setW, NULL, &time_out);
		if (ret <= 0)
		{
			// select() failed or connection timed out
			if (ret == 0) {
				WSASetLastError(WSAETIMEDOUT);
				return CONNECTION_TIMEOUT;
			}
			//close_socket(sock);
			printf_s("select() function has unexpectedly failed. Terminating...\n");
			return CONNECTION_ERROR;
		}

		//if (FD_ISSET(sock, &setE))
		//{
		//	// connection failed
			//int err = 0;
			//getsockopt(sock, SOL_SOCKET, SO_ERROR, &err, sizeof(err));
			//WSASetLastError(err);
		//	printf_s("Connection to server has failed. Terminating...\n");
		//	return CONNECTION_FAILED;
		//}
	}

	// connection successful

	// put socket in blocking mode...
	if (SOCKET_ERROR == set_blocking(sock, TRUE))
		return CONNECTION_ERROR;

	return CONNECTION_SUCCEEDED;
}
//	----------------------------------------------------------------------------------------
int check_port_num(const char *argv_port)
{
	int port_num = 0;
	port_num = atoi(argv_port);
	if ((port_num == 0) && strcmp("0", argv_port) || port_num < 0)
	{
		printf_s("Wrong input port number. Terminating...\n");
		return -1;
	}
	return port_num;
}
//	----------------------------------------------------------------------------------------
error_code_t waiting_for_sync_object(HANDLE handle_to_sync_obj, sync_object_t sync_object_type, DWORD wait_time)
{
	DWORD wait_res;
	char error_str[MAX_ERROR_STR_LEN] = { 0 };

	map_sync_object_to_str(sync_object_type, error_str);

	if (WAIT_OBJECT_0 != (wait_res = WaitForSingleObject(handle_to_sync_obj, wait_time))) {
		switch (wait_res) {
		case (WAIT_TIMEOUT):	// Enter if you waited for too long to increment readers amount.
			printf_s("%s wait timeout\n", error_str);
			return WAITING_TIMEOUT;
		case (WAIT_FAILED):
			printf_s("%s wait failed. Critical Error, terminating...\n", error_str);
			return WAITING_FAILED;
		case (WAIT_ABANDONED):
			printf_s("%s was abandoned. Fatal Error, terminating...\n", error_str);
			return ABANDONED;
		}
	}
	return SUCCESS;
}
//	----------------------------------------------------------------------------------------
error_code_t release_sync_object(HANDLE handle_to_sync_obj, sync_object_t sync_object_type)
{
	BOOL release_res;
	char error_str[MAX_ERROR_STR_LEN] = { 0 };
	map_sync_object_to_str(sync_object_type, error_str);

	switch (sync_object_type) {
	case (MUTEX):
		if (FALSE == (release_res = ReleaseMutex(handle_to_sync_obj))) {
			printf_s("Fatal Error. Could not release %s. Terminating...\n", error_str);
			return RELEASE_FAILED;
		}
		break;
	case (FIRST_EVENT):
	case (SECOND_EVENT):
		if (!SetEvent(handle_to_sync_obj)) {
			printf_s("Fatal Error. Could not set %s. Terminating...\n", error_str);
			return RELEASE_FAILED;
		}
		break;
	}

	return SUCCESS;
}
//	----------------------------------------------------------------------------------------
void map_sync_object_to_str(sync_object_t sync_object_type, char *error_str)
{
	switch (sync_object_type) {
	case (MUTEX):
		sprintf_s(error_str, MAX_ERROR_STR_LEN, "mutex");
		break;
	case (FIRST_EVENT):
		sprintf_s(error_str, MAX_ERROR_STR_LEN, "first_event");
		break;
	case (SECOND_EVENT):
		sprintf_s(error_str, MAX_ERROR_STR_LEN, "second_event");
		break;
	case (ANY_THREAD):
		sprintf_s(error_str, MAX_ERROR_STR_LEN, "thread of client");
		break;
	case (EXIT_THREAD):
		sprintf_s(error_str, MAX_ERROR_STR_LEN, "EXIT_THREAD");
		break;
	}
}
//	----------------------------------------------------------------------------------------
BOOL FileExists(LPCTSTR szPath)
{
	DWORD dwAttrib = GetFileAttributes(szPath);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}
//	----------------------------------------------------------------------------------------
int reset_file_state()
{
	if (TRUE == FileExists("GameSession.txt")) {
		if (!(DeleteFileA("GameSession.txt"))) {
			printf_s("Deleting existing file failed, error number: %li. Terminating...\n", GetLastError());
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}
//	----------------------------------------------------------------------------------------
BOOL check_EOF(DWORD dword_bytes_read, DWORD read_result)
{
	if ((0 == dword_bytes_read) && (TRUE == read_result))
		return TRUE;
	return FALSE;
}