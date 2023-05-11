#ifndef FORGE_H
#define FORGE_H

#include "protocol.h"
#include <string.h>
#include <stdlib.h>

struct header
{
    request_code_t code;
    uuid_t id;

    uint16_t * fields;
    size_t size;
};

struct packet
{
    struct header header;
    
    char * data;
    size_t size;
};

void free_packet(struct packet *);

size_t header_size(const struct header *);
const char * bufferize_header(const struct header *);

const char * forge_tcp_packet(const struct packet *);
struct packet * melt_tcp_packet(const char *);

#endif
