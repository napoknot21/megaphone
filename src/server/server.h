#ifndef SERVER_H
#define SERVER_H

#include <stddef.h>
#include <stdint.h>

#include "../protocol.h"
#include "megaphone.h"

#define BUFF_SIZE 4096 //tmp value
#define MAX_CLIENTS 10

#define LOCAL_ADDR "127.0.0.1"
#define NOTIFICATION_INTERVAL 10

typedef struct sockaddr_in SA_IN;

/**
 * @brief: Load and run the server
 */
void run_server (void);


/**
 * @brief: Initialize the socket connection in UDP mode
 * @param sock : The sock pointer to the socket server TCP/UDP
 * @param domain : the host domain (AF_INET, AF_INET6, etc...)
 * @param protocol : The type of connection (TCP[SOCK_STREAM] or UDP[SOCK_DGRAM])
 * @param distant : string with the IP address
 * @param port : port number
 * @return 0 in success and the status error in case of failure
 */ 
int create_socket (
    int *sock, 
    int domain, int protocol, 
    const char * distant,
    uint16_t port
);


/**
 * @brief Create a UDP socket for the server
 * @param sock : The sock pointer to the socket server UDP
 * @param mc_addr : Multicast address
 * @param port : The port number
 * @return 0 in success and the status error in case of failure
 */
int create_udp_socket (
    int *sock,
    const char *mc_addr,
    uint16_t port
);


//TODO: Check/test this function 
/**
 * @brief Add a socket to the UPD group
 * @param sock : The sock pointer
 * @param mc_addr : Multicast address
 */
void join_multicast_group (
    int *sock,
    const char *mc_addr
);


/**
 * @brief: Client handler function for the send/recv communication
 * @param p_client_socket: Pointer to the client socket
 */
void * handler_client (void *p_client_socket);


/**
 * @brief: Removes a client (socket) from the list of connections
 * @param sock : the socket to be removed
 */
void remove_client (int *sock);


/**
 * @brief: Send a notification the client
 * @param arg : the socket 
 */
 void * send_notifications(void *arg);


/**
 * @brief : Functions that accept connections from clients
 * @param sock_fd : socket that will be closed
 * @param cli_addr : Pointer to the sockaddr_in structure
 * @return : 0 if the socket is accepted, else other integer
 */
int accept_tcp_connection (struct host *serv);


/**
 * @brief : The function that close all sockets for a host structure
 * @param serv : The host structure (client or server)
 */
void close_server (struct host * serv);


#endif
