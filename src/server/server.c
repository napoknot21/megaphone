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
#include <signal.h>

#include "server.h"

int serv_socket;

int load_server () {
    
    struct sockaddr_in6 serv_addr;

    serv_socket = socket(PF_INET6, SOCK_STREAM, 0);
    check_socket(&serv_socket);

    memset(&serv_addr, 0x0, sizeof(serv_addr))
    serv_addr.sin6_family = AF_INET6;
    serv_addr.sin6_port = htons(SERVER_PORT);
    serv_addr.sin6_addr = in6addr_any;

    check_bind(&serv_socket, (struct sockaddr *) &serv_addr);

    check_listen(&serv_socket, 0);

    return 0;
}











void handle_signal (int sig)
{
    printf("Caught signal %d", sig);
    
    //Closing sockets
    if (close(serv_socket) == 0) {
        puts("Socket closed !\n");
        exit(0);
    }

    perror("An error occurred while closing the socket !\n");
    printf("Error code: %d", errno);
    exit(1);
}


void check_socket (int *serv_socket)
{
    int serv_socket = *serv_socket;
    if (serv_socket < 0) {
        perror("Socked failed !\n");
        printf("Error code: %d\n", errno);
        free(serv_socket);
        exit(1);
    }
}


void check_bind (int *serv_socket, struct sockaddr_in *serv_addr)
{
    if ((bind(*serv_socket, (struct sockaddr *) serv_addr, sizeof(*serv_addr))) < 0) {
        perror("Bind Failed !\n");
        printf("Error code: %d\n", errno);
        close(*serv_socket);
        free(serv_socket);
        exit(1);
    }
}


void check_listen (int *serv_socket, int nb_connections)
{
    if ((listen(*serv_socket, nb_connections)) < 0) {
        perror("Listen Failed !\n");
        printf("Error code: %d\n", errno);
        close(*serv_socket);
        free(serv_socket);
        exit(1);
    }
}


void check_accept (int *serv_socket, int *cli_socket, struct sockaddr *cli_addr)
{
    if ((*cli_socket = accept(*serv_socket, (struct sockaddr *) cli_addr, (socklen_t *) sizeof(cli_addr))) < 0) {
        perror("Accept failed !\n");
        printf("Error code: %d\n", errno);
        close(*serv_socket);
        free(serv_socket);
        exit(1);
    }
}

int main (int argc, char **argv) {


    return 0;
}

//#include "queue.h"

//pthread_mutex_t list_lock;

//extern struct node *head;
//extern struct node *current;

//Thread function
/*
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
*/
