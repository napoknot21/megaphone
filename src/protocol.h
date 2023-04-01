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

typedef struct post
{
	uint16_t type;
	unsigned thread;
	const char * data;
} post_t;

#endif
