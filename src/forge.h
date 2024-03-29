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
struct packet * copy_packet(const struct packet*);
void free_packet(struct packet*);

const char * bufferize(const struct header*);
char * forge_tcp_packet(const struct packet*, size_t*);

unsigned get_interface(int, int);

#endif
