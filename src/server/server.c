#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <perror.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>

#include "server.h"
#include "queue.h"

//Thread function
void * handler_connection (void *p_client_socket) 
{
    int threadnum = *(int *)p_client_socket;
    int sock_desc;
    struct sockaddr_in serv_addr;
    char sendBuff[BUFF_SIZE+1], clientBuff[BUFF_SIZE+1];
    
    if ((sock_desc = recv(client_socket, clientBuff, 2*BUFF_SIZE+1, 0)) < 0) {
        printf("Failed creating socket\n");
    }
    
    bzero((char *) $serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(IP_ADDR);

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
