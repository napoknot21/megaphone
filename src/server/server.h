#ifndef SERVER_H
#define SERVER_H

#define STATUS_ERROR (-1)
#define STATUS_SUCCESS 0

#define BUFF_SIZE 4096
#define MAX_CLIENTS  10

#define SERVER_PORT 7070 //tmp
#define LOCAL_ADDR4 "127.0.0.1"
#define LOCAL_ADDR6 "::1"

//pthread_t thread_pool[THREAD_POOL_SIZE];
//pthread_mutex_t mutex = PTHREAD_MUTEXT_INITIALIZER;
//pthread_cond_t condition_var = PTHREAD_COND_INITIALIZER;  

int load_server (void);
void * handle_client (void *p_client_socket);
void handle_signal (int sig);

// Check functions
void check_socket (int *serv_socket);
void check_bind (int *serv_socket, (struct sockaddr_in) *serv_addr);
void check_listen (int *serv_socket, int nb_connections);
void check_accept (int *serv_socket, int *cli_socket, SA *cli_addr);

#endif
