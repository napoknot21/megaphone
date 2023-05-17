#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stddef.h>
#include <stdint.h>

#define LAYER_TCP 0
#define LAYER_UDP 1

#define REQUEST_CODE_SIZE_BITS 5
#define USER_ID_SIZE_BITS 11
#define FIELD_SIZE 2
#define REQUEST_CODE_MASK 0xF800
#define USER_ID_MASK 0x3FF
#define TCP_BYTE_BLOCK_SIZE 512

#define MP_TCP_PORT 7777
#define MP_UDP_PORT 6666

#define MP_UDP_BLOCK_SIZE 512
#define MP_NET_BUFFER_SIZE 512

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

struct session {

	uuid_t uid;
	long long time;

};

struct host
{
	int tcp_sock;
	int udp_sock;
};

/*
 * Megaphone's packet builder
 */

struct packet * mp_signup(const char*);
struct packet * mp_upload_post(const struct session*, const struct post*);
struct packet * mp_request_threads(const struct session*, uint16_t, uint16_t);
struct packet * mp_subscribe(const struct session*, uint16_t);

#endif
