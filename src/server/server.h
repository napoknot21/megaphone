#ifndef SERVER_H
#define SERVER_H

#define BUFF_SIZE 4096
#define SOCKET_ERROR (-1)
#define SOCKET_SUCCESS 0
#define SERVER_BACKLOG 100
#define SERVER_PORT 7070 //tmp

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

void * handle_connection (void *p_client_socket);
int check (int exp, const char *msg);

#endif
