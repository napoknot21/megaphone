#include "../protocol.h"

struct packet * mp_signup(const char * username)
{
}

struct packet * mp_upload_post(const struct session * se, const struct post * p)
{
}

struct packet * mp_request_threads(const struct session * se, uint16_t thread, uint16_t n)
{
}

struct packet * mp_subscribe(const struct session * se, uint16_t thread)
{
}

struct packet * mp_process_data(const char * data)
{
}
