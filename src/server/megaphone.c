#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "megaphone.h"
#include "../utils/vector.h"

uint16_t i = 0;
struct vector * sessions;

struct session * get_session(const uid_t uuid)
{
	for(size_t k = 0; k < sessions->size; k++)
	{
		struct session * se = at(sessions, k);
		if(se->uid == uuid) return se;
	}

	return NULL;
}

uid_t gen_id() 
{
	return i + 1 < pow(2, 11) ? ++i : 0;
}

struct packet * mp_signup(char * username)
{
    struct packet * p = make_packet();

    struct session se;
    memset(&se, 0x0, sizeof(se));

    se.uid = gen_id();
    se.username = username;

    push_back(sessions, (void *) &se);

    printf("[i] %s subscribed!", username);

    uint16_t lfield = fusion(SIGNUP, se.uid);
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
    if(!sessions)
    {
	    sessions = make_vector(
			    (void* (*)(void*)) copy_session,
			    (void (*)(void*)) free_session, 
			    sizeof(struct session)
	    );
    }

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
