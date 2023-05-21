#ifndef MEGAPHONE_H
#define MEGAPHONE_H

#include "../protocol.h"
#define DEFAULT_BOOTSTRAP "127.0.0.1"

struct packet * mp_request_for(const struct session*, const request_code_t, size_t, char **);
int mp_recv(const struct host *, struct session*, const char*);

#endif
