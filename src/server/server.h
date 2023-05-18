#ifndef SERVER_H
#define SERVER_H

#include <stdint.h>

#define BUFF_SIZE 4096 //tmp value
#define MAX_CLIENTS 10

#define LOCAL_ADDR "127.0.0.1"

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
 * @brief: Initialize the socket connection in UDP mode
 * @param sock : The server socket
 * @param domain : the host domain (AF_INET, AF_INET6, etc...)
 * @param protocol : The type of connection (TCP[SOCK_STREAM] or UDP[SOCK_DGRAM])
 * @param distant : string with the IP address
 * @param port : port number
 */ 
int create_socket (int *sock, int domain, int protocol, const char * distant, uint16_t port);


/**
 * @brief : Functions that accept connections from clients
 * @param sock_fd : socket that will be closed
 * @param cli_addr : Pointer to the sockaddr_in structure
 * @return : 0 if the socket is accepeted, else other integer
 */
int accept_connection (int sock_fd, SA_IN *cli_addr);


#endif
