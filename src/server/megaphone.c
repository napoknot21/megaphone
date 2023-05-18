#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "megaphone.h"

uint16_t i = 0;

int make_id() {
	return i + 1 < pow(2, 11) ? ++i : 0;
}

struct packet * mp_signup(char * username)
{
    struct packet * p = make_packet();

    uint16_t lfield = fusion(SIGNUP, make_id());
    p->header.fields = malloc(FIELD_SIZE);
    p->header.size = 1;

    p->header.fields[MP_FIELD_CR_UUID] = htons(lfield);

    return p;
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
    struct packet * recv_p = melt_tcp_packet(data);
    struct packet * send_p = NULL;

    uint16_t lfield = recv_p->header.fields[MP_FIELD_CR_UUID];

    uint16_t code = get_rq_code(lfield);
    uid_t id = get_uuid(lfield);

    struct session * se = get_session(id);

    if(!se) return send_p;
    
    switch (code)
    {
    case SIGNUP: 
	send_p = mp_signup(recv_p->data);
        break;

    case POST: 
	struct post pt = {MESSAGE, recv_p->header.fields[MP_FIELD_THREAD], recv_p->data};
        send_p = mp_upload_post(se, &pt);
	break;

    case FETCH: 
        send_p = mp_request_threads(
			se, 
			recv_p->header.fields[MP_FIELD_THREAD], 
			recv_p->header.fields[MP_FIELD_NUMBER]
	);
	break;

    case SUBSCRIBE: 
        send_p = mp_subscribe(se, recv_p->header.fields[0]);
        break;

    case DOWNLOAD: break;

    default:
	printf("[-] This request code is unknown!\n");
        break;
    }

    return send_p;
}
