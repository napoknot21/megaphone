#ifndef SERVER_H
#define SERVER_H

#include <stddef.h>
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
 * @brief: Initialize the socket connection in UDP mode
 * @param serv : The host struct for the server
 * @param domain : the host domain (AF_INET, AF_INET6, etc...)
 * @param protocol : The type of connection (TCP[SOCK_STREAM] or UDP[SOCK_DGRAM])
 * @param distant : string with the IP address
 * @param port : port number
 * @return 0 in success and the status error in case of failure
 */ 
int create_socket (
    struct host *serv, 
    int domain, int protocol, 
    const char * distant, 
    uint16_t port
);


/**
 * @brief Create a TCP socket for the server
 * @param serv : The host structure for the server
 * @param domain : The host domain (AF_INET, AF_INET6, etc...)
 * @return 0 in success and the status error in case of failure
 */
int create_tcp_socket (struct host *serv, int domain);


/**
 * @brief: Client handler function for the send/recv communication
 * @param p_client_socket: Pointer to the client socket
 */
void * handler_client (void *p_client_socket);


/**
 * @brief: Removes a client (socket) from the list of connections
 * @param cl : host struct pointer for the client
 */
void remove_client (struct host *cl);




/**
 * @brief: 
 *
 *
 *
 */
 int create_udp_sockets (struct host *serv, int domain);


/**
 * @brief : Functions that accept connections from clients
 * @param sock_fd : socket that will be closed
 * @param cli_addr : Pointer to the sockaddr_in structure
 * @return : 0 if the socket is accepeted, else other integer
 */
int accept_connection (int sock_fd, SA_IN *cli_addr);


/**
 * @brief : The function that close all sockets for a host structure
 * @param cl : The host structure (client or server)
 */
void close_socket (struct host *cl);

#endif
