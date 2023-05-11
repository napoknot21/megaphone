#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "server.h"
#include "../protocol.h"
#include "../utils/string.h"

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_t threads[MAX_CLIENTS];
int handlers = 0;

void run () 
{
    int serv_socket, cli_socket;    
    pthread_t thread_id;
    socklen_t addrlen;

    serv_socket = create_socket();
    bind_socket(serv_socket);
    listen_socket(serv_socket);

    printf("[*] Server started on port %d...\n", SERVER_PORT);

    while (1) {

        struct sockaddr_in cli_addr;
        addrlen = sizeof(cli_addr);
 
        if ((cli_socket = accept(serv_socket, (struct sockaddr *)&cli_addr, &addrlen)) == -1) {
            perror("[!] Accept failed...\n");
            continue;
        }

        printf("[*] New connection from %s:%d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

        pthread_mutex_lock(&lock);

        if (handlers >= MAX_CLIENTS) {
            printf("[!] Maximum number of clients reached, closing connections...\n");
            close_socket(cli_socket);
        } else {
            threads[handlers++] = cli_socket;
            if ((pthread_create(&thread_id, NULL, handler_client, (void *) &cli_socket)) != 0) {
                perror("[!] pthread_create failed...\n");
                exit(EXIT_FAILURE);
            }

        }

        pthread_mutex_unlock(&lock);

    }
}


void remove_client(int cli_sock) 
{
    pthread_mutex_lock(&lock);

    int i;
    for (i = 0; i < handlers; i++) {
        if (threads[i] == cli_sock) {
            break;
        }
    }

    if (i == handlers) {
        fprintf(stderr, "[!] Client socket %d not found...\n", cli_sock);
        pthread_mutex_unlock(&lock);
        return;
    }

    handlers--;
    
    while (i < handlers) {
        threads[i] = threads[i + 1];
        i++;
    }
    
    pthread_mutex_unlock(&lock);
    close_socket(cli_sock);
}


void * handler_client (void * p_sock) 
{
    int sock;
    memmove(&sock, p_sock, sizeof(int));

    char block[TCP_BYTE_BLOCK_SIZE];
    memset(block, 0x0, TCP_BYTE_BLOCK_SIZE);
    
    struct string * data = make_string();
    ssize_t length;

    while ((length = recv(sock, block, TCP_BYTE_BLOCK_SIZE, 0)) > 0) 
    {
	string_push_back(data, block, length);
	if(length < TCP_BYTE_BLOCK_SIZE)
	{
		break;
	}
    }

    string_push_back(data, "\0", 1);

    struct packet * back_packet = mp_process_data(data->data);

    free_string(data);

    remove_client(sock);
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

/*
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
*/

void close_socket (int sock_fd)
{
    if (close(sock_fd) < 0) {
        perror("[!] Close failed...\n");
        exit(EXIT_FAILURE);
    }
}


int main (int argc, char **argv) 
{
    run();
    //pthread_mutex_destroy(&lock);
    return 0;
}
