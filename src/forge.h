#ifndef FORGE_H
#define FORGE_H

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>

#define FIELD_SIZE 		2
#define TCP_BYTE_BLOCK_SIZE 	512

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

#endif
