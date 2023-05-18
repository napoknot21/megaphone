#include "protocol.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

void fill_header
(
		struct header * hd, 
		uint16_t code, 
		uid_t uid, 
		uint16_t nthread, 
		uint16_t n, 
		uint16_t datalen
)
{
	memset(hd, 0x0, sizeof(struct header));
	hd->fields = malloc(sizeof(MP_HEADER_FIELD_SIZE * FIELD_SIZE));
	
	uint16_t cu = fusion(code, uid);

	hd->fields[MP_FIELD_CR_UUID] = htons(cu);
	hd->fields[MP_FIELD_THREAD] = htons(nthread);
	hd->fields[MP_FIELD_NUMBER] = htons(n);
	hd->fields[MP_FIELD_DATALEN] = htons(datalen);
}

uint16_t get_rq_code(uint16_t field)
{
	return (field & REQUEST_CODE_MASK) >> USER_ID_SIZE_BITS;
}

uid_t get_uuid(uint16_t field)
{
	return field & USER_ID_MASK;
}

request_code_t string_to_code(const char * str)
{
	if(!strcmp(str, "signup")) 
		return SIGNUP;
	else if(!strcmp(str, "post")) 
		return POST;
	else if(!strcmp(str, "fetch"))
		return FETCH;
	else if(!strcmp(str, "subscribe"))
		return SUBSCRIBE;
	else if(!strcmp(str, "download"))
		return DOWNLOAD;	

	return UNKNOWN;
}

uint16_t fusion(uint16_t u, uint16_t v)
{
    uint16_t field;

    field = u << USER_ID_SIZE_BITS & REQUEST_CODE_MASK;
    field = field | (v & USER_ID_MASK);

    return field;
}

struct packet * melt_tcp_packet(const char * bytes)
{
    struct packet * p = make_packet(); 
    
    uint16_t lfield;
    memmove(&lfield, bytes, 2); 

    lfield = ntohs(lfield);
    uint16_t code = get_rq_code(lfield);

    if(code > DOWNLOAD)
    {
        free_packet(p);
        printf("[packet] received data is corrupted!\n");
        
        return NULL;
    }
    else if(code != SIGNUP)
    {
        p->header.size = MP_HEADER_FIELD_SIZE * FIELD_SIZE;
        p->header.fields = malloc(p->header.size);
        memmove(&p->header.fields, bytes, p->header.size);

	for(size_t i = 0; i < MP_HEADER_FIELD_SIZE; i++) p->header.fields[i] = ntohs(p->header.fields[i]);
    }

    return p;
}

struct session * copy_session(struct session * model)
{
	struct session * copy = malloc(sizeof(struct session));
	memset(copy, 0x0, sizeof(struct session));

	copy->uid = model->uid;

	size_t us = strlen(model->username);
	copy->username = malloc(us + 1);
	
	copy->username[us] = 0x0;
	memmove(copy->username, model->username, us + 1);

	return copy;
}

void free_session(struct session * se)
{
	free(se->username);
	free(se);
}
