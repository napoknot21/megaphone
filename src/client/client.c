#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <pthread.h>
#include "../forge.h"
#include "../protocol.h"
#include "client.h"

int if_index = 0;

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
		close(*sock);
	}

	return status;
}

int set_udp_socket (int * sock, const char* mc_addr, uint16_t mc_port)
{
	*sock = socket(AF_INET6, SOCK_DGRAM, 0);

	struct sockaddr_in6 addr;
	memset(&addr, 0x0, sizeof(addr));	
	
	addr.sin6_family = AF_INET6;
  	addr.sin6_port = htons(mc_port);
	
	int status = bind(*sock, (struct sockaddr*) & addr, sizeof(struct sockaddr_in6));

	struct ipv6_mreq group;
	memset(&group, 0x0, sizeof(group));
		
	status &= inet_pton(AF_INET6, mc_addr, &group.ipv6mr_multiaddr.s6_addr);
	group.ipv6mr_interface = if_index;

	status &= setsockopt(*sock, IPPROTO_IPV6, IPV6_JOIN_GROUP, &group, sizeof(group));

	if(!status)
	{
    		close(*sock);	
	}

	return status;

}

fd_set * make_fd_set(struct host * cl, int* max)
{
	fd_set * rset = malloc(sizeof(fd_set));
	FD_ZERO(rset);
	*max = ((int *) (cl->udp_socks->data))[0];
	for (int i = 0; i < cl->udp_socks->size; i++)
	{
		if (((int *) cl->udp_socks->data)[i] > *max)
		{
			*max = ((int *) (cl->udp_socks->data))[i];
		}
		int * us = at(cl->udp_socks, i);
		FD_SET(*us, rset);
	}

	return rset;
}

void * client_recv_dataflow(void * arg) 
{
	struct host * cl = (struct host *) arg;
	struct timeval t;

	memset(&t, 0x0, sizeof(struct timeval));

	t.tv_usec = 100;

	while(1)
	{
		int max;
		fd_set * rset = make_fd_set(cl, &max);
		char buf[MP_UDP_BLOCK_SIZE];
		memset(buf, 0x0, sizeof(buf));

		int to_read = select(max+1, rset, NULL, 0, &t);
		recv(to_read, buf, strlen(buf), 0);
		printf(buf);
		free(rset);
	}	
	
	return NULL;
}

int communication_udp(struct host * cl) 
{
	pthread_t thread;
	if (pthread_create(&thread, NULL, client_recv_dataflow, cl) == -1)
	{
		perror("pthread_create");
		return -1;
	}
	pthread_join(thread,NULL);
	return 0;
}

/*
 * This method manages to send a packet
 * structure through TCP/IP layer. It gives
 * back the raw received data.
 */

int client_send_dataflow(const struct host * cl, const struct packet * p, char ** rcv)
{
	const char * data = forge_tcp_packet(p);
	int status = send(cl->tcp_sock, data, strlen(data), 0);	

	if(status)
	{
		printf("[-] An error occured while sending packet!\n");
		return status;
	}	

	uint16_t lfield;
	recv(cl->tcp_sock, &lfield, FIELD_SIZE, 0);

	lfield = ntohs(lfield);
	uint16_t rc = get_rq_code(lfield);
	
	size_t size;

	if(rc == SUBSCRIBE)
	{
		size = FIELD_SIZE * MP_HEADER_FIELD_SIZE;		
	}
	else
	{
		size = FIELD_SIZE * (MP_HEADER_FIELD_SIZE - 1);	
	}

	*rcv = malloc(size);
	memmove(*rcv, &lfield, FIELD_SIZE);
	size_t bytes = recv(cl->tcp_sock, *rcv + FIELD_SIZE, size, 0);

	return bytes;	
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
	
	for(size_t k = 0; k < cl->udp_socks->size; k++)
	{
		int * fd = (int *) at(cl->udp_socks, k);
		close(*fd);
	}

	free_host(cl);
}

/*
 * This method manages the interaction
 * between the user and the client.
 */

void mp_shell()
{
	struct host * cl = make_host();
	if_index = get_interface(AF_INET6, IFF_MULTICAST);	

	/*
	 * There we add TCP/IP socket
	 */
	
	if(set_socket(&cl->tcp_sock, AF_INET, SOCK_STREAM, DEFAULT_BOOTSTRAP, MP_TCP_PORT))
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
		char ** rcv = malloc(sizeof(char*));

		client_send_dataflow(cl, p, rcv);

		mp_recv(cl, &se, p, *rcv);

		free(p);
		free(*rcv);
		free(rcv);

		for(size_t k = 0; k < argc; k++)
		{
			free(argv[k]);
		}

		free(argv);	

	} while(strcmp(data, "quit\n"));

	close_client(cl);	
}

int main(int argc, char ** argv) 
{
       mp_shell();
       return 0;
}
