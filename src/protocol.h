#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stddef.h>
#include <stdint.h>
#include "forge.h"

#define LAYER_TCP 		0
#define LAYER_UDP 		1

#define REQUEST_CODE_SIZE_BITS 	5
#define USER_ID_SIZE_BITS 	11
#define REQUEST_CODE_MASK 	0xF800
#define USER_ID_MASK 		0x3FF

#define MP_TCP_PORT 		7777
#define MP_UDP_PORT 		6666

#define MP_UDP_BLOCK_SIZE 	512
#define MP_NET_BUFFER_SIZE 	512

#define MP_HEADER_FIELD_SIZE	4

#define MP_FIELD_CR_UUID	0
#define MP_FIELD_THREAD 	1
#define MP_FIELD_NUMBER 	2
#define MP_FIELD_DATALEN 	3

typedef enum request_code
{
    SIGNUP = 0,
    POST,
    FETCH,
    SUBSCRIBE, 
    DOWNLOAD,
    UNKNOWN
} request_code_t;

request_code_t string_to_code(const char * str);

typedef uint16_t uuid_t;

/*
 * Message and file uploading are done by
 * the same method, hence the post type and
 * structure.
 */

enum post_type
{
	MESSAGE = 0,
	MFILE
};

struct post
{
	uint16_t type;
	uint16_t thread;

	char * data;
};

struct session 
{
	uuid_t uid;
	char * username;	
};

struct session * copy_session(struct session *);
void free_session(struct session*);

struct host
{
	int tcp_sock;
	int * udp_sock;

	size_t udp_sock_size;
};

void fill_header(struct header *, uint16_t, uid_t, uint16_t, uint16_t, uint16_t);
struct packet * melt_tcp_packet(const char*);

uint16_t get_rq_code(uint16_t);
uid_t get_uuid(uint16_t);

uint16_t fusion(uint16_t, uint16_t);

struct packet * mp_signup(char*);
struct packet * mp_upload_post(const struct session*, const struct post*);
struct packet * mp_request_threads(const struct session*, uint16_t, uint16_t);
struct packet * mp_subscribe(const struct session*, uint16_t);

#endif
