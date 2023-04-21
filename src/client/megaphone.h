#ifndef MEGAPHONE_H
#define MEGAPHONE_H

#include "../protocol.h"

#define DEFAULT_PORT 7777
#define DEFAULT_BOOTSTRAP "127.0.0.1"

struct session {

	uuid_t uid;
	long long time;

};

/*
 * Megaphone's packet builder
 */

struct packet * mp_signup(const char*);
struct packet * mp_upload_post(const struct session*, const struct post*);
struct packet * mp_request_threads(const struct session*, uint16_t, uint16_t);
struct packet * mp_subscribe(const struct session*, uint16_t);

struct packet * mp_request_for(const struct session*, const request_code_t, size_t argc, char ** argv);
int mp_recv(struct session*, const request_code_t, const char*);

#endif
