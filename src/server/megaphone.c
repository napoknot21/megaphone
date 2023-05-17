#include "../protocol.h"
#include "../forge.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <math.h>

uint16_t i = 0;

// char** tab_fil = init_tab();

int make_id() {
    if(i + 1 < pow(2, 11)) {
        return i + 1;
    }
    return -1;
}

struct packet * mp_signup(const char * username)
{
    struct packet * p = malloc(sizeof(struct packet));
	
	memset(p, 0x0, sizeof(struct packet));
	memset(&p->header, 0x0, sizeof(p->header));

	p->header.code = SIGNUP;
    p->header.id = make_id();

	return p;
}

struct packet * mp_upload_post(const struct session * se, const struct post * p)
{
    // Envoie du post au client
    int b = 1;
    if(b) {
        return NULL;
    }
    memset(p, 0x0, sizeof(struct post));
    return p;
    // NULL si bon
    // sinon packet set à 0
}

struct packet * mp_request_threads(const struct session * se, uint16_t thread, uint16_t n)
{

}

struct packet * mp_subscribe(const struct session * se, uint16_t thread)
{
}

struct packet * mp_process_data(const char * data)
{
    struct packet * p = melt_tcp_packet(data);
    struct packet * new_p = NULL;

    if(p->header.id == -1) {
        memset(p, 0x0, sizeof(struct packet));
        return p;
    }
    struct session * s = malloc(sizeof(struct session));
    memset(s, 0x0, sizeof(struct session));
    s->uid = p->header.id;
    
    switch (p->header.code)
    {
    case SIGNUP:
        /* code */
        new_p = mp_signup(p->data);
        break;
    case POST:
        /* code */
        struct post msg = {MESSAGE, p->header.fields[0], p->data};
        new_p = mp_upload_post(s, &msg);

        break;
    case FETCH:
        /* code */
        new_p = mp_request_threads(s, p->header.fields[0], p->header.fields[1]);
        break;
    case SUBSCRIBE:
        /* code */
        new_p = mp_subscribe(s, p->header.fields[0]);
        break;
    case DOWNLOAD:
        /* code */
        break;
    default:
		printf("[-] This request code is unknown!\n");
        break;
    }

    return p;
}
