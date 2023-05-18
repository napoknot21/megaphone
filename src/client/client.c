#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "../forge.h"
#include "../protocol.h"
#include "client.h"

/*
 * This method manages to allocate the
 * tcp socket in the client structure.
 */

int set_socket(int * sock, int domain, int protocol, const char * distant, uint16_t port)
{
	*sock = socket(domain, protocol, 0);

	struct sockaddr sa;
	memset(&sa, 0x0, sizeof(sa));
	sa.sa_family = domain;

	in_port_t nport = htons(port);
	socklen_t sockaddr_size = 0;	

	switch(domain)
	{
	case AF_INET:		
		inet_pton(AF_INET, distant, sa.sa_data + 2);
		sockaddr_size = sizeof(struct sockaddr_in);
		break;

	case AF_INET6:			
		memset(sa.sa_data, 0x0, sizeof(sa.sa_data));
		inet_pton(AF_INET6, distant, sa.sa_data + 6);
		sockaddr_size = sizeof(struct sockaddr_in6);	
		break;
	}

	memmove(sa.sa_data, &nport, 2);

	if(protocol == SOCK_DGRAM) return 0;

	int status = connect(*sock, &sa, sockaddr_size);

	if(!status)
	{
		printf("[+] Connecting by TCP/IP over %s:%d\n", distant, port);
	}
	else
	{
		printf("[-] An error occured while connecting by TCP/IP!\n");
	}

	return status;
}

/*
 * This method manages to send a packet
 * structure through TCP/IP layer. It gives
 * back the raw received data.
 */

int client_send_dataflow(const struct host * cl, const struct packet * p, char * rcv)
{
	const char * data = forge_tcp_packet(p);
	int status = send(cl->tcp_sock, data, strlen(data), 0);	

	if(status)
	{
		printf("[-] An error occured while sending packet!\n");
		return status;
	}

	ssize_t bytes, size = 0;
	ssize_t reserve = MP_NET_BUFFER_SIZE;

	do {
		if(size >= reserve)
		{
			reserve += MP_NET_BUFFER_SIZE;
			rcv = realloc(rcv, reserve);
		}

		bytes = recv(cl->tcp_sock, rcv, MP_NET_BUFFER_SIZE, 0);
		size += bytes;

	} while(bytes);

	return status;	
}

/* A séparer en plusieurs fonction TODO */

/**
 * @brief Récupère l'adresse du serveur et s'y connecte.
 * @param hostname est le nom du serveur.
 * @param port est le numéro du port sur lequel se connecter depuis le client.
 * @param sock est la socket permettant de communiquer avec le serveur.
 * @param addr est l'adresse du serveur.
 * @param addrlen est la taille de l'adresse.
 * @return -1 si le serveur n'a pas été trouver, -2 si la socket ne s'est pas crée, 0 si tout c'est bien passé.
 */
int get_server_addr(char* hostname, char* port, int * sock, struct sockaddr_in6** addr, int* addrlen) {
    struct addrinfo hints;
    struct addrinfo *r;
    struct addrinfo *p;
    int ret;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_V4MAPPED | AI_ALL;

    if ((ret = getaddrinfo(hostname, port, &hints, &r)) != 0 || r == NULL){
        fprintf(stderr, "erreur getaddrinfo : %s\n", gai_strerror(ret));
        return -1;
    }

    *addrlen = sizeof(struct sockaddr_in6);
    p = r;
    while( p != NULL ){
        if((*sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) > 0){
            if(connect(*sock, p->ai_addr, *addrlen) == 0)
	        break;
      
            close(*sock);
        }

        p = p->ai_next;
    }

    if (p == NULL) return -2;

    //on stocke l'adresse de connexion
    *addr = (struct sockaddr_in6 *) p->ai_addr;

    //on libère la mémoire allouée par getaddrinfo 
    freeaddrinfo(r);
    
    return 0;
}

/*
 * This method parses the line and
 * returns a table of strings, corresponding
 * to the command followed by its arguments.
 */

char ** parse_line(const char * line, size_t llen, size_t * argc)
{
	*argc = 0;	

	for(size_t k = 0; k < llen; k++)
	{
		if(line[k] == 0x20 || line[k] == '\n') (*argc)++;
	}	

	char ** argv = malloc(*argc * sizeof(char*));
	size_t beg = 0;	

	for(size_t i = 0, end = 0; end < llen; end++)
	{
		if(line[end] == 0x20 || line[end] == '\n')
		{	
			argv[i] = malloc(end - beg + 1);
			argv[i][end - beg] = 0;
			memmove(argv[i], line + beg, end - beg);
			beg = end + 1;
	
			i++;	
		}
	}	

	return argv;
}

/*
 * This method closes both of the
 * client sockets (TCP & UDP)
 */

void close_client(struct host * cl)
{
	close(cl->tcp_sock);
	
	for(size_t k = 0; k < cl->udp_sock_size; k++)
	{
		close(cl->udp_sock[k]);
	}
}

/*
 * This method manages the interaction
 * between the user and the client.
 */

void mp_shell()
{
	struct host cl;
	memset(&cl, 0x0, sizeof(cl));

	/*
	 * There we add TCP/IP socket
	 */
	
	if(set_socket(&cl.tcp_sock, AF_INET, SOCK_STREAM, DEFAULT_BOOTSTRAP, MP_TCP_PORT))
	{
		printf("[-] Error while initializing client!\n");
		return;
	}

	struct session se; 
	memset(&se, 0x0, sizeof(se));

	char * data = NULL;
	size_t argc, llin = 0;

	do {
		if(data) free(data);
	
		printf("megaphone $ ");
		getline(&data, &llin, stdin);
	
		char ** argv = parse_line(data, strlen(data), &argc);	

		if(argc <= 1) continue;

		request_code_t rc = string_to_code(argv[0]);
		printf("[!] Request Code: %d\n", rc);
		struct packet * p = mp_request_for(&se, rc, argc - 1, argv + 1);

		char * rcv = malloc(MP_NET_BUFFER_SIZE);
		client_send_dataflow(&cl, p, rcv);

		mp_recv(&se, rc, rcv);

		free(p);
		free(rcv);

		for(size_t k = 0; k < argc; k++)
		{
			free(argv[k]);
		}

		free(argv);	

	} while(strcmp(data, "quit\n"));

	close_client(&cl);
}

int main(int argc, char ** argv) 
{
       mp_shell();
       return 0;
}
