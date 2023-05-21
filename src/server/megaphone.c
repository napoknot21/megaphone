#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "megaphone.h"
#include "../utils/vector.h"

uint16_t i = 0;

struct vector * sessions;
struct vector * threads;

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

void mp_init()
{
	sessions = make_vector(
		(void* (*)(void*)) copy_session,
		(void (*)(void*)) free_session, 
		sizeof(struct session)
	);

	threads = make_vector(
		(void* (*)(void*)) copy_thread,
		(void (*)(void*)) free_session,
		sizeof(struct session)
	);

}

void mp_close()
{
	free_vector(sessions);
	free_vector(threads);
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

struct packet * mp_upload_post(const struct session * se, struct post * pt, uint16_t thread)
{
	struct packet * p = make_packet();

	if(thread < threads->size)
	{
		/*
		 * The thread already exists,
		 * we just push back the new
		 * post on it.
		 */

		struct thread * th = at(threads, thread);
		push_back(th->posts, pt);
	
		printf("[i] New post has been uploaded on thread %d!\n", thread);
	}
	else
	{
		/*
		 * The thread doesn't exist yet,
		 * we instanciate it before pushing
		 * back the new post on it.
		 */

		struct thread th;
		memset(&th, 0x0, sizeof(th));

		th.seed = se->uid;
	//	th.addr = gen_multicast_addr();
		th.posts = make_vector(
				(void* (*)(void*)) copy_post, 
				(void (*)(void*)) free_post, 
				sizeof(struct post)
		);

		push_back(th.posts, pt);
		push_back(threads, &th);

		printf("[i] Thread %ld has been created!\n", threads->size);
	}

	return p;
}

struct packet * mp_request_threads(const struct session * se, uint16_t thread, uint16_t n)
{
	struct packet * p;

	if(thread < threads->size)
	{
		struct thread * th = at(threads, thread);
		
		size_t ps = th->posts->size;
		size_t beg = n <= ps ? ps - n : 0;

		for(size_t k = beg; k < ps; k++)
		{
			struct post * pt = at(th->posts, k);

		}
	}

	return p;
}

struct packet * mp_subscribe(const struct session * se, uint16_t thread)
{
	struct packet * p = make_packet();
	return p;
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

    case POST: ;
        uint16_t thread = recv_p->header.fields[MP_FIELD_THREAD];	
	struct post pt = {MESSAGE, se->uid, recv_p->data};

        send_p = mp_upload_post(se, &pt, thread);
	
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

    case UPLOAD_FILE: break;

    case DOWNLOAD_FILE: break;

    default:
	printf("[-] This request code is unknown!\n");
        break;
    }

    return send_p;
}
