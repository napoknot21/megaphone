#ifndef CLIENT_H
#define CLIENT_H

#include "megaphone.h"

#define UDP_BLOCK 512
#define TCP_RECV_BLOCK 512

struct client
{
	int tcp_sock;
	int udp_sock;	
};

int set_tcp_socket(struct client*, int, const char*, uint16_t);
int set_udp_socket(struct client*, int, const char*, uint16_t);

int client_send_dataflow(const struct client*, const struct packet*, char*);
int client_send_datagram(int, const struct packet*);

#endif
