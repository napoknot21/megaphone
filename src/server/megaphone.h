#ifndef MEGAPHONE_H
#define MEGAPHONE_H

#include "../protocol.h"
#include "../forge.h"

#define DEFAULT_BOOTSTRAP "127.0.0.1"

struct session * get_session(const uid_t);
struct packet * mp_process_data(const char*, size_t*);

struct packet * mp_process_data(const char * data);

#endif
