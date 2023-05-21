#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stddef.h>
#include <stdint.h>
#include "forge.h"
#include "utils/vector.h"

#define LAYER_TCP 		0
#define LAYER_UDP 		1

#define MP_CLIENT_SIDE		0
#define MP_SERVER_SIDE		1

#define REQUEST_CODE_SIZE_BITS 	5
#define USER_ID_SIZE_BITS 	11
#define REQUEST_CODE_MASK 	0xF800
#define USER_ID_MASK 		0x3FF

#define MP_TCP_PORT 		7777
#define MP_UDP_PORT 		6666
#define MP_MULTICAST_PORT	1122

#define MP_UDP_BLOCK_SIZE 	512

#define MP_HEADER_FIELD_SIZE	4

#define MP_FIELD_CR_UUID	0
#define MP_FIELD_THREAD 	1
#define MP_FIELD_NUMBER 	2
#define MP_FIELD_DATALEN 	3

typedef enum request_code
{
    SIGNUP = 1,
    POST,
    FETCH,
    SUBSCRIBE, 
    UPLOAD_FILE,
    DOWNLOAD_FILE
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
	uid_t uuid;	

	char * data;
};

struct post * copy_post(const struct post*);
void free_post(struct post*);
void print_post(const struct post *);

struct thread
{
	uid_t seed;
	struct in6_addr addr;

	struct vector * posts;
};

struct thread * copy_thread(const struct thread*);
void free_thread(struct thread*);

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
	struct vector * udp_socks;	
};

struct host * make_host();
void free_host(struct host*);

struct mp_header
{
	uint16_t rc;
	uid_t uuid;

	uint16_t nthread;
	uint16_t n;
	uint16_t len;
};

struct mp_post_header
{
	uint16_t nthread;
	char origin[10];
	char pseudo[10];
	uint16_t len;
};

struct mp_udp_header
{
	uint16_t rc;
	uid_t uuid;
	uint16_t n;
};

void mp_init();
void mp_close();

void forge_header(int, struct header*, const struct mp_header);
void melt_header(int, struct mp_header*, const struct header*);

void forge_post_header(struct header*, const struct mp_post_header);
void melt_post_header(struct mp_post_header*, const struct header*);

struct packet * melt_tcp_packet(const char*);

uint16_t get_rq_code(uint16_t);
uid_t get_uuid(uint16_t);

uint16_t fusion(uint16_t, uint16_t);

struct packet * mp_signup(char*);
struct packet * mp_upload_post(const struct session*, struct post*, uint16_t);
struct packet * mp_request_threads(const struct session*, uint16_t, uint16_t*);
struct packet * mp_subscribe(const struct session*, uint16_t);

#endif
