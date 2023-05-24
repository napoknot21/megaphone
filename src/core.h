#ifndef CORE_H
#define CORE_H

#include <sys/socket.h>
#include <stdint.h>

struct sockaddr format_addr(int family, const char*, uint16_t);

#endif
