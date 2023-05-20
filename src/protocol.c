#include "protocol.h"
#include "utils/vector.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

struct host * make_host()
{
	struct host * h = malloc(sizeof(struct host));
	memset(h, 0x0, sizeof(struct host));

	h->udp_socks = make_vector(NULL, NULL, sizeof(int));

	return h;
}

void free_host(struct host * h)
{
	free_vector(h->udp_socks);
	free(h);
}

void fill_header(struct header * hd, const struct mp_header mhd)
{
	memset(hd, 0x0, sizeof(struct header));
	hd->fields = malloc(sizeof(MP_HEADER_FIELD_SIZE * FIELD_SIZE));
	
	uint16_t cu = fusion(mhd.rc, mhd.uuid);

	hd->fields[MP_FIELD_CR_UUID] = htons(cu);
	hd->fields[MP_FIELD_THREAD] = htons(mhd.nthread);
	hd->fields[MP_FIELD_NUMBER] = htons(mhd.n);
	hd->fields[MP_FIELD_DATALEN] = htons(mhd.len);
}

void melt_header(struct mp_header * mhd, const struct header * hd)
{
	memset(mhd, 0x0, sizeof(struct mp_header));

	uint16_t lfield = ntohs(hd->fields[MP_FIELD_CR_UUID]);

	mhd->rc = get_rq_code(lfield);
	mhd->uuid = get_uuid(lfield);

	mhd->nthread = ntohs(hd->fields[MP_FIELD_THREAD]);
	mhd->n = ntohs(hd->fields[MP_FIELD_NUMBER]);
	mhd->len = ntohs(hd->fields[MP_FIELD_DATALEN]);
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

	return 0;
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

struct post * copy_post(const struct post * pt)
{
	struct post * copy = malloc(sizeof(struct post));
	memset(copy, 0x0, sizeof(struct post));

	copy->type = pt->type;
	copy->uuid = pt->uuid;

	size_t dps = strlen(pt->data);

	copy->data = malloc(dps);
	memmove(copy->data, pt->data, dps);

	return copy;
}

void free_post(struct post * pt)
{
	if(pt->data) free(pt->data);
	free(pt);
}

struct thread * copy_thread(const struct thread * th)
{
	struct thread * copy = malloc(sizeof(struct thread));
	memset(copy, 0x0, sizeof(struct thread));

	copy->seed = th->seed;
	copy->addr = th->addr;

	copy->posts = copy_vector(th->posts);

	return copy;
}

void free_thread(struct thread * th)
{
	free_vector(th->posts);
	free(th);
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

void print_post(const struct post * pt)
{
	
}
