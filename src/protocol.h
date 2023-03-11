#ifndef PROTOCOL_H
#define PROTOCOL_H

#define LAYER_TCP 0
#define LAYER_UDP 1

#define REQUEST_CODE_SIZE_BITS 5
#define USER_ID_SIZE_BITS 11
#define FIELD_SIZE 2

#define REQUEST_CODE_MASK 0xF800
#define USER_ID_MASK 0x3FF

#include <stdint.h>

typedef enum request_code
{
    SIGNUP = 0,
    POST,
    FETCH,
    SUBSCRIBE,
    UPLOAD,
    DOWNLOAD
} request_code_t;

typedef uint16_t uuid_t;

#endif