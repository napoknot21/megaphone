#ifndef SERVER_H
#define SERVER_H

#define BUFF_SIZE 4096 //tmp value
#define MAX_CLIENTS 10

#define SERVER_PORT 7000 //tmp value
#define LOCAL_ADDR4 "127.0.0.1"
#define LOCAL_ADDR6 "::1"

typedef struct sockaddr_in SA_IN;

/**
 * @brief: Load and run the server
 */
void run_server (void);


/**
 * @brief: Client handler function for the send/recv communication
 * @param p_client_socket: Pointer to the client socket
 */
void * handler_client (void *p_client_socket);


/**
 * @brief: Removes a client (socket) from the list of connections
 * @param cli_sock : Socket for the client
 */
void remove_client (int cli_sock);


/**
 * @brief: Create a socket (for the server)
 * @return: In success, the socket number else, a exit value
 */
int create_socket (void);


/**
 * @brief : Bind the socket server function
 * @param serv_socket : The server socket
 */
void bind_socket (int serv_socket);


/**
 * @brief : Implementation of the listen functions
 * @param serv_socket : The server socket
 */
void listen_socket (int serv_socket);


/**
 * @brief : Functions that accept connections from clients
 * @param sock_fd : socket that will be closed
 * @param cli_addr : Pointer to the sockaddr_in structure
 * @return : 0 if the socket is accepeted, else other integer
 */
int accept_connection (int sock_fd, SA_IN *cli_addr);


/**
 * @brief : Close the socked
 * @param sock_fd : The socket to close
 */
void close_socket (int sock_fd);

// Check functions
/*
int check_socket (int *serv_socket);
int check_bind (int *serv_socket, SA_IN *serv_addr);
int check_listen (int *serv_socket, int nb_connections);
int check_accept (int *serv_socket, int *cli_socket, SA *cli_addr);
*/
#endif
