#ifndef SERVER_H
#define SERVER_H

#define BUFF_SIZE 4096
#define MAX_CLIENTS  10

#define SERVER_PORT 7000 //tmp
#define LOCAL_ADDR4 "127.0.0.1"
#define LOCAL_ADDR6 "::1"

typedef struct sockaddr_in SA_IN;

void run_server (void);
void * handler_client (void *p_client_socket);
void remove_client (int cli_sock);

//main functions
int create_socket (void);
void bind_socket (int serv_socket);
void listen_socket (int serv_socket);
int accept_connection (int sock_fd, SA_IN *cli_addr);
void close_socket (int sock_fd);

// Check functions
/*
int check_socket (int *serv_socket);
int check_bind (int *serv_socket, SA_IN *serv_addr);
int check_listen (int *serv_socket, int nb_connections);
int check_accept (int *serv_socket, int *cli_socket, SA *cli_addr);
*/
#endif
