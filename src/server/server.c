#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <perror.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "server.h"
#include "queue.h"


void * handle_connection (void *p_client_socket) 
{
    int client_socket = *p_client_socket;
    free(p_client_socket);
    char buff[BUFF_SIZE];

    return NULL;
}


int check (int exp, const char *msg) {
    if (exp == SOCKETERROR) {
        perror("msg");
        exit(1);
    }
    return exp;
}


void * thread_func (void *arg) 
{
    while (true) {
        int *pclient = dequeue();
        if (pclient != NULL) {
            // We have a connection
            handle_connection(pclient);

        }
    }
}
