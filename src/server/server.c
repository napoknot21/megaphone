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
#include "../client/megaphone.h"
#include "../protocol.h"
#include "../utils/string.h"

/*

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_t threads[MAX_CLIENTS];
int handlers = 0;


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
*/

/**
 * @brief: Initialize the socket connection in TCP mode
 * @param serv : the host serv structure
 * @param domain : the host domain (AF_INET, AF_INET6, etc...)
 * @param distant : string with the IP address
 * @param port : port number
 */ /*
int create_socket_tcp (struct host *serv, int domain, const char * distant, uint16_t port) 
{
    serv->tcp_sock = socket (domain, SOCK_STREAM, 0); 

    if (serv->tcp_sock) {
        fprintf("[!] An error occurred creating socket...\n");
        return -1;
    }

    struct sockaddr serv_addr;
    memset(&serv_addr, 0x00, sizeof(serv_addr));
    serv_addr.sa_family = domain;

    in_port_t nport = htons(port);
    socklen_t sockaddr_size = 0;

    switch (domain) {

        case AF_INET :
            inet_pton(AF_INET, distant, serv_addr.sa_data+2);
            sockaddr_size = sizeof(struct sockaddr_in);
            break;    

        case AF_INET6 :
            memset(serv_addr.sa_data, 0x00, sizeof(serv_addr).sa_data);
            inet_pton(AF_INET6, distant, serv_addr.sa_data+6);
            sockaddr_size = sizeof(struct sockaddr_in6);
            break;
    }

    memmove(serv_addr.sa_data, &nport, 2);

    
    int statusBind = bind (serv->tcp_sock, (struct sockaddr *) &serv_addr, sockaddr_size);

    if (statusBind) {
        printf("[*] Server successfully bound !");
    } else {
        printf("[!] An error occurred while binding on the server !\n");
    }

    return statusBind;

}

*/

/**
 * @brief: Initialize the socket connection in UDP mode
 * @param serv : the host serv structure
 * @param domain : the host domain (AF_INET, AF_INET6, etc...)
 * @param distant : string with the IP address
 * @param port : port number
 */ /*
int create_socket_udp (struct host *serv, int domain, const char *distant, uint16_t port)
{
    serv->udp_sock = socket (domain, SOCK_DGRAM, 0);

    if (serv->udp_sock) {
        printf("[!] An error occurred creating socket...\n");
        return -1;
    }

    struct sockaddr serv_addr;
    memset(&serv_addr, 0x00, sizeof(serv_addr));
    serv_addr.sa_family = domain;

    in_port_t nport = htons(port);
    socklen_t sockaddr_size = 0;

    switch (domain) {

        case AF_INET :
            inet_pton(AF_INET, distant, serv_addr.sa_data+2);
            sockaddr_size = sizeof(struct sockaddr_in);
            break;    

        case AF_INET6 :
            memset(serv_addr.sa_data, 0x00, sizeof(serv_addr).sa_data);
            inet_pton(AF_INET6, distant, serv_addr.sa_data+6);
            sockaddr_size = sizeof(struct sockaddr_in6);
            break;
    }

    memmove(serv_addr.sa_data, &nport, 2);

    int statusBind = bind (serv->tcp_sock, (struct sockaddr *) &serv_addr, sockaddr_size);

    if (statusBind) {
        printf("[*] Server successfully bound !");
    } else {
        printf("[!] An error occurred while binding on the server !\n");
    }

    return statusBind;

}


int listen_server_tcp (struct host *serv, uint16_t port) 
{
    int status = listen(serv->tcp_sock, 0);
    
    if (!status) {
        printf("[*] Server is listening on port: %d\n", port);
    } else {
        printf("[!] An error occurred the server is listening...\n", socket_server);
    }

    return status;
}


int listen_server_udp (struct host *serv, uint16_t port)
{
    int status = listen(serv->udp_sock, 0);

    if (!status) {
        printf("[*] Server is listening on port: %d\n", port);
    } else {
        printf("[!] An error occurred the server is listening...\n", socket_server);
    }

    return status;
}


void close_socket (struct host *serv)
{
    close(serv->tcp_sock);
	close(serv->udp_sock);
}



void run_server () 
{
    struct host serv;
    memset(&serv, 0x00, sizeof(serv));

    if ((&serv, AF_INET, DEFAULT_BOOTSTRAP, MP_TCP_PORT) != 0) {
        printf("[!] Error while initializing server !\n");
		return;

    }
    
    int serv_socket, cli_socket;    
    pthread_t thread_id;
    socklen_t addrlen;

    printf("[*] Server started on port %d...\n", );

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



int main (int argc, char **argv) 
{
    run_server();
    //pthread_mutex_destroy(&lock);
    return 0;
}

*/