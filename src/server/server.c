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
#include "../utils/string.h"


pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_t threads[MAX_CLIENTS];
int handlers = 0;
int if_index = 0;


int create_socket (int *sock, int domain, int protocol, const char * distant, uint16_t port) 
{
    *sock = socket (domain, protocol, 0);

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

    if (protocol == SOCK_STREAM) return 0;

    int statusBind = bind (*sock, (struct sockaddr *) &serv_addr, sockaddr_size);

    if (statusBind) {

        printf("[!] Error binding...\n");
        close(*sock);
        return statusBind;
    
    }

    int status = listen (*sock, 0);

    if (!status) {
        printf("[*] Server listening at %s:%d...\n", distant, port);
    } else {
        printf("[*] An error occurred during listening by TCP/IP...\n");
    }

    return status;
}



int create_udp_socket (int *sock, const char *mc_addr, uint16_t port) 
{
    *sock = socket (AF_INET6, SOCK_DGRAM, 0);

    if (*sock < 0) {
        printf("[!] Error creating socket UDP...\n");
        return *sock;
    }

    // Enable SO_REUSEADDR to allow multiple instances of this application to receive copies of the multicast datagrams.
    int reuse = 1;
    if (setsockopt(*sock, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0) {
        perror("Setting SO_REUSEADDR error");
        close(*sock);
        return -1;
    }

    struct sockaddr_in6 addr_serv;
    memset(&addr_serv, 0x00, sizeof(addr_serv));

    addr_serv.sin6_family = AF_INET6;
    addr_serv.sin6_port = htons (port);

    int status = bind (*sock, (struct sockaddr *)&addr_serv, sizeof(struct sockaddr_in6));

    struct ipv6_mreq group;
    memset(&group, 0x00, sizeof(struct ipv6_mreq));

    status &= inet_pton (AF_INET6, mc_addr, &group.ipv6mr_multiaddr.s6_addr);
    group.ipv6mr_interface = if_index;

    status &= setsockopt(*sock, IPPROTO_IPV6, IPV6_JOIN_GROUP, &group, sizeof(struct ipv6_mreq));

    /*
    if (setsockopt(udp_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group)) < 0) {
        perror("[!] Adding multicast group error...\n");
        close(udp_sock);
        exit(1);
    }
    */

    if (!status) {
        printf("[!] Error occurred while setting up the multicast group...\n");
        close (*sock);  
    } 

    return status;
}



void join_multicast_group(int *sock, const char* mc_addr)
{
    struct ip_mreq group;
    memset(&group, 0x00, sizeof(group));

    group.imr_multiaddr.s_addr = inet_addr(mc_addr);
    group.imr_interface.s_addr = htonl(INADDR_ANY);
    
    int status = setsockopt(*sock, IPPROTO_IPV6, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group));
        
    if (!status) {

        printf("[*] Adding multicast group error !\n");
    
    } else {

        printf("[!] An error has occurred while trying to add multicast group...\n"); 
        close(*sock);
        exit(1);

    }

}

void remove_client (int *sock)
{
    pthread_mutex_lock(&lock);

    int i;
    for (i = 0; i < handlers; i++) {
        if (threads[i] == *sock) {
            break;
        }
    }

    if (i == handlers) {
        fprintf(stderr, "[!] Client socket %d not found...\n", *sock);
        pthread_mutex_unlock(&lock);
        return;
    }

    handlers--;
    
    while (i < handlers) {
        threads[i] = threads[i + 1];
        i++;
    }
    
    pthread_mutex_unlock(&lock);
    close(*sock);
}


void * handler_client (void * p_sock) 
{
    int sock;
    memmove(&sock, (int *) p_sock, sizeof(int));

    char block[TCP_BLOCK_SIZE];
    memset(block, 0x0, TCP_BLOCK_SIZE);
    
    struct string * data = make_string();
    ssize_t length;

    while ((length = recv(sock, block, TCP_BLOCK_SIZE, 0)) > 0) 
    {
	    string_push_back(data, block, length);
	    if(length < TCP_BLOCK_SIZE)
	    {
		    break;
	    }
    }

    string_push_back(data, "\0", 1);

    size_t packet_len = 1;
    struct packet * back_packet = mp_process_data(data->data, &packet_len);

    free_string(data);

    remove_client(&sock);
    pthread_exit(NULL);

    return NULL;
}



int accept_tcp_connection (struct host *serv) 
{
    struct sockaddr_in client_address;

    memset(&client_address, 0x00, sizeof(client_address));
    socklen_t addr_len = sizeof(client_address);

    int client_sock = accept(serv->tcp_sock, (struct sockaddr*)&client_address, &addr_len);
    
    if (client_sock < 0) {
        perror("Accept error");
        exit(1);
    }

    return client_sock;
}


void run () 
{
    
    struct host serv;
    memset(&serv, 0x00, sizeof(serv));

    if (create_socket(&serv.tcp_sock, AF_INET, SOCK_STREAM, DEFAULT_BOOTSTRAP, MP_TCP_PORT) != 0) {
        printf("[!] Error while initializing server !\n");
		return;
    }
    
    pthread_t thread_id;
    socklen_t addrlen;

    while (1) {

        struct sockaddr_in cli_addr;
        addrlen = sizeof(cli_addr);

        int *cli_socket = malloc(sizeof(int)); 
        if ((*cli_socket = accept(serv.tcp_sock, (struct sockaddr *)&cli_addr, &addrlen)) == -1) {
            perror("[!] Accept failed...\n");
            free(cli_socket);
            continue;
        }

        printf("[*] New connection from %s:%d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

        pthread_mutex_lock(&lock);

        if (handlers >= MAX_CLIENTS) {

            printf("[!] Maximum number of clients reached, closing connections...\n");
            close(*cli_socket);
            free(cli_socket);
        
        } else {

            threads[handlers++] = *cli_socket;
            
            if ((pthread_create(&thread_id, NULL, handler_client, cli_socket)) != 0) {
                perror("[!] pthread_create failed...\n");
                exit(EXIT_FAILURE);
            }

            pthread_detach(thread_id);
        }

        pthread_mutex_unlock(&lock);
    }

    close_server(&serv);
}


void close_server (struct host * serv)
{
	close(serv->tcp_sock);
	
	for(size_t k = 0; k < serv->udp_socks->size; k++)
	{
		int * fd = at(serv->udp_socks, k);
		close(*fd);
	}

	free_host(serv);
}



int main (int argc, char **argv) 
{
    //run();
    pthread_mutex_destroy(&lock);
    return 0;
}
