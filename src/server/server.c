#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#include "server.h"


int load_server (struct sockaddr_in serv_addr) 
{
    int serv_socket;

    if ((serv_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("[!] Failed to create socket...\n");
        return STATUS_ERROR;
    }
    
    /*if (check_socket(&serv_socket) != 0) {
        return STATUS_ERROR;
    }
    */
    memset(&serv_addr, 0x00, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    /*
    if ((check_bind(&serv_socket, (struct sockaddr *) &serv_addr)) != 0) {
        return STATUS_ERROR;
    }
    */
    if ((bind(serv_socket, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) < 0) {
        perror("[!] Failed to bind socket to address...\n");
        return STATUS_ERROR;
    }
    return serv_socket;
}


void * handler_client (void * p_client_socket) 
{
    int cli_socket = *(int *) p_client_socket;
    char buff[BUFF_SIZE];

    memset(buff, 0x00, BUFF_SIZE);
    
    while (recv(cli_socket, buff, BUFF_SIZE, 0) > 0) {
        
        printf("Received Message: %s\n", buff);

        //echo message to the client
        send(cli_socket, buff, strlen(buff), 0);

        memset(buff, 0x00, BUFF_SIZE);
    }

    /* //Alternative version
    int fd, rd;
    
    rd = read (sock_cli, buff, BUFF_SIZE-1);
    if (rd < 0) {
        perror("Error reading from socket !\n");
        exit(1);
    }

    printf("Here is the message: %s\n", buff);
    
    fd = write (sock_cli, "I got your message\n", 19);
    if (fd < 0) {
        perror("Error writing to socket !\n");
        exit(1);
    }
    */
    close(cli_socket);
    pthread_exit(NULL);
}


int main (int argc, char **argv) 
{
    int serv_socket, cli_socket;
    struct sockaddr_in serv_addr, cli_addr;
    pthread_t threads[MAX_CLIENTS];
    //int cli_socks[MAX_CLIENTS];

    if ((serv_socket = load_server(serv_addr)) == STATUS_ERROR) {
        perror("[!] Error loading the server...\n");
        exit(1);
    }

    if (listen(serv_socket, MAX_CLIENTS) < 0) {
        perror("[!] Failed to listen for incoming connections...\n");
        exit(2);
    }
        
    printf("[*] Server listening for incoming connections !\n");

    while (1) {

        socklen_t cli_len = sizeof(cli_addr);

        if ((cli_socket = accept(serv_socket, (struct sockaddr *) &cli_addr, &cli_len)) < 0) {
            perror("[!] Failed to accept client connections\n");
            continue;
        }

        //spawn client thread
        int thread_index;
        for (thread_index = 0; thread_index < MAX_CLIENTS; thread_index++) {
            if (threads[thread_index] == 0) {
                if (pthread_create(&threads[thread_index], NULL, handler_client, &cli_socket) == 0) {
                    printf("[*] Client connected, assigned thread => %d\n", thread_index);
                } else {
                    perror("[!] Failed to create thread...\n");
                }
            }
            break;
        }

        if (thread_index == MAX_CLIENTS) {
            printf("[!] Maximum number of clients reached...\n");
            close (cli_socket);
        }
    }

    close(serv_socket);

    return 0;
}


/*
int handler_signal (int sig)
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
*/

int check_socket (int *serv_socket)
{
    int serv_sock = *serv_socket;
    if (serv_sock < 0) {
        perror("Socked failed !\n");
        printf("Error code: %d\n", errno);
        free(serv_socket);
        return STATUS_ERROR;
    }
    return STATUS_SUCCESS;
}


int check_bind (int *serv_socket, struct sockaddr_in *serv_addr)
{
    if ((bind(*serv_socket, (struct sockaddr *) serv_addr, sizeof(*serv_addr))) < 0) {
        perror("Bind Failed !\n");
        printf("Error code: %d\n", errno);
        close(*serv_socket);
        free(serv_socket);
        return STATUS_ERROR;
    }
    return STATUS_SUCCESS;
}


int check_listen (int *serv_socket, int nb_connections)
{
    if ((listen(*serv_socket, nb_connections)) < 0) {
        perror("Listen Failed !\n");
        printf("Error code: %d\n", errno);
        close(*serv_socket);
        free(serv_socket);
        return STATUS_ERROR;
    }
}


int check_accept (int *serv_socket, int *cli_socket, struct sockaddr *cli_addr)
{
    if ((*cli_socket = accept(*serv_socket, (struct sockaddr *) cli_addr, (socklen_t *) sizeof(cli_addr))) < 0) {
        perror("Accept failed !\n");
        printf("Error code: %d\n", errno);
        close(*serv_socket);
        free(serv_socket);
        exit(1);
    }
}

//pthread_mutex_t list_lock;

//extern struct node *head;
//extern struct node *current;

//Thread function
/*
int * handler_connection (void *p_client_socket) 
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


int * thread_func (void *arg) 
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
