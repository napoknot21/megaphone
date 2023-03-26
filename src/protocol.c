#include "forge.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

const struct header make_header(request_code_t code, uuid_t id, uint16_t * fields, size_t size) {
    const struct header header = {.code = code, .id = id, .fields = fields, .size = size};
    return header;
}

const struct packet make_packet(struct header header, const char * data, size_t size) {
    const struct packet packet = {.header = header, .data = data, .size = size};
    return packet;
}

const char* make_buf(request_code_t code, uuid_t id, uint16_t * fields, size_t size_h, const char * data, size_t size_p) {
    const struct header header = make_header(code, id, fields, size_h);
    const struct packet packet = make_packet(header, data, size_p);
    const char* buf = forge_tcp_packet(&packet);
    return buf;
}

const char* make_insc_buf(request_code_t code, uuid_t id, uint16_t * fields, size_t size_h) {
    const struct header header = make_header(code, id, fields, size_h);
    const char* buf = bufferize_header(&header);
    return buf;
}