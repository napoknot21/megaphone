#ifndef MEGAPHONE_H
#define MEGAPHONE_H

#include "../protocol.h"

#define DEFAULT_PORT 7777
#define DEFAULT_BOOTSTRAP "127.0.0.1"

struct packet * mp_request_for(const struct session*, const request_code_t, size_t, char **);
int mp_recv(struct session*, const request_code_t, const char*);

#endif
