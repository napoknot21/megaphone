#ifndef FORGE_H
#define FORGE_H

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>

#define FIELD_SIZE		2
#define UDP_BLOCK_SIZE 		512
#define TCP_BLOCK_SIZE		512

struct header
{
    uint16_t * fields;
    size_t size;
};

struct packet
{
    struct header header;
    
    char * data;
    size_t size;
};

struct packet * make_packet();
void free_packet(struct packet*);

const char * bufferize(const struct header*);
const char * forge_tcp_packet(const struct packet*);

unsigned get_interface(int, int);

size_t send_udp(struct sockaddr_in6, char*);
size_t recv_udp(struct sockaddr_in6, char*);

#endif
