#ifndef SERVER_H
#define SERVER_H

#define BUFF_SIZE 4096
#define SOCKET_ERROR (-1)
#define SOCKET_SUCCESS 0
#define SERVER_BACKLOG 100
#define SERVER_PORT 7070 //tmp
#define THREAD_POOL_SIZE 5
#define IP_ADDR "127.0.0.1"

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

pthread_t thread_pool[THREAD_POOL_SIZE];
pthread_mutex_t mutex = PTHREAD_MUTEXT_INITIALIZER;
//Let's thread wait until something happens and it can do something useful
pthread_cond_t condition_var = PTHREAD_COND_INITIALIZER;  

void * handle_connection (void *p_client_socket);
int check (int exp, const char *msg);
void * thread_func (void *arg);

#endif
