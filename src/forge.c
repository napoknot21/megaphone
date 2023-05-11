#include "forge.h"
#include <stdio.h>

size_t header_size(const struct header * header)
{
    return sizeof(uint16_t) * (1 + header->size);
}

const char * bufferize_header(const struct header * header)
{
    char * buffer = malloc(header_size(header));

    uint16_t lead_field;

    lead_field = header->code << USER_ID_SIZE_BITS & REQUEST_CODE_MASK;
    lead_field = lead_field | (header->id & USER_ID_MASK);

    memmove(buffer, &lead_field, sizeof(lead_field));

    for(size_t k = 0; k < header->size; k++)
    {
        memmove(buffer + (k + 1) * FIELD_SIZE, header->fields + k, FIELD_SIZE);
    }

    return (const char *) buffer;
}

const char * forge_tcp_packet(const struct packet * packet)
{
    size_t data_size = packet->size;
    data_size = !(data_size & 1) ? data_size : data_size + 1;

    size_t hd_size = header_size(&packet->header);

    printf("[!] Packet Forge:\n\tHeader size \t-- %ld\n\tData size \t-- %ld\n", hd_size, data_size);

    char * buffer = malloc(data_size + hd_size);
    const char * header_bytes = bufferize_header(&packet->header);

    memmove(buffer, header_bytes, hd_size);
    memmove(buffer + hd_size, packet->data, data_size);

    return (const char*) buffer;
}

struct packet * melt_tcp_packet(const char * bytes)
{
    struct packet * packet = malloc(sizeof(struct packet));
    memset(&packet->header, 0x0, sizeof(struct header));
    
    uint16_t lead_field;
    memmove(&lead_field, bytes, 2);
    
    packet->header.code = (lead_field & REQUEST_CODE_MASK) >> USER_ID_SIZE_BITS;
    packet->header.id = lead_field & USER_ID_MASK;

    if(packet->header.code > DOWNLOAD)
    {
        free(packet);
        printf("[packet] received data is corrupted!\n");
        
        return NULL;
    }

    switch(packet->header.code)
    {
    case SIGNUP:
        packet->header.fields = NULL;
        packet->header.size = 0;
        packet->data = NULL;
        break;

    default:
        packet->header.size = 3 * sizeof(uint16_t);
        packet->header.fields = malloc(packet->header.size);
        memmove(&packet->header + 6, bytes + 2, packet->header.size);
    }

    return packet;
}

void free_packet(struct packet * p)
{
	free(p->header.fields);
	free(p->data);
	free(p);
}
