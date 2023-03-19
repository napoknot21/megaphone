#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

#include "server.h"

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void run_server () 
{
    int serv_socket, cli_socket;
    pthread_t threads[MAX_CLIENTS];
    int client_count = 0;

    serv_socket = create_socket();
    bind_socket(serv_socket);
    listen_socket(serv_socket);

    printf("[*] Server started on port %d...\n", SERVER_PORT);

    while (1) {

        struct sockaddr_in cli_addr;
        socklen_t addrlen = sizeof(cli_addr);

        cli_socket = accept_connection (serv_socket, &cli_addr);

        printf("[*] New connection from %s:%d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

        if (client_count >= MAX_CLIENTS) {
            printf("[!] Maximum number of clients reached, closing connections...\n");
            close_socket(cli_socket);
            continue;
        }

        if ((pthread_create(&threads[client_count], NULL, handler_client, (void *) &cli_socket)) != 0) {
            perror("[!] pthread_create failed...\n");
            exit(EXIT_FAILURE);
        }

        client_count++;
    }

}


void * handler_client (void * p_client_socket) 
{
    int cli_socket = *(int *) p_client_socket;
    
    char buff[BUFF_SIZE];
    memset(buff, 0x00, BUFF_SIZE);
    int bytes_recv;

    while ((bytes_recv = recv(cli_socket, buff, BUFF_SIZE, 0)) > 0) {
        
        buff[bytes_recv] = '\0';
        printf("Received Message: %s\n", buff);

        pthread_mutex_lock(&lock);

        //echo message to the client
        if (send(cli_socket, buff, strlen(buff), 0) == -1) {
            perror("[!] Send message failed...\n");
            close_socket(cli_socket);
            free(p_client_socket);
            pthread_mutex_unlock(&lock);
            return NULL;
        }

        memset(buff, 0x00, BUFF_SIZE);
        pthread_mutex_unlock(&lock);
    }

    if (bytes_recv == 0) {
        printf("[!] Client disconnected\n");
    } else {
        perror("[!] Recive failed\n");
    }

    close_socket (cli_socket);
    free(p_client_socket);
    pthread_exit(NULL);
}


int create_socket () 
{
    int sock_fd;
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("[!] Socket failed...\n");
        exit(EXIT_FAILURE);
    }
    return sock_fd;
}


void bind_socket (int serv_socket) 
{
    struct sockaddr_in serv_addr;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(SERVER_PORT);

    if (bind(serv_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("[!] Bind failed...\n");
        exit(EXIT_FAILURE);
    }
}


void listen_socket (int serv_socket) 
{
    if (listen(serv_socket, MAX_CLIENTS) < 0) {
        perror("[!] Listen failed...\n");
        exit(EXIT_FAILURE);
    }
}


int accept_connection (int sock_fd, struct sockaddr_in *cli_addr)
{
    int cli_sock;
    socklen_t addrlen = sizeof(*cli_addr);

    if ((cli_sock = accept(sock_fd, (struct sockaddr *)cli_addr, &addrlen)) < 0) {
        perror("[!] Accepted failed...\n");
        exit(EXIT_FAILURE);
    }

    return cli_sock;
}


void close_socket (int sock_fd)
{
    if (close(sock_fd) < 0) {
        perror("[!] Close failed...\n");
        exit(EXIT_FAILURE);
    }
}


int main (int argc, char **argv) 
{
    run_server();
    return 0;
}
