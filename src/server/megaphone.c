#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "megaphone.h"
#include "../utils/vector.h"

uint16_t i = 0;

struct vector * sessions;
struct vector * mp_threads;

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

	mp_threads = make_vector(
		(void* (*)(void*)) copy_thread,
		(void (*)(void*)) free_session,
		sizeof(struct session)
	);

}

void mp_close()
{
	free_vector(sessions);
	free_vector(mp_threads);
}

struct packet * mp_signup(char * username)
{
    struct packet * p = make_packet();

    struct session se;
    memset(&se, 0x0, sizeof(se));

    se.uid = gen_id();
    se.username = username;

    push_back(sessions, (void *) &se);

    printf("[i] %s signed-up!", username);

    uint16_t lfield = fusion(SIGNUP, se.uid);
    p->header.fields = malloc(FIELD_SIZE);
    p->header.size = 1;

    p->header.fields[MP_FIELD_CR_UUID] = htons(lfield);

    return p;
}

struct packet * mp_upload_post(const struct session * se, struct post * pt, uint16_t thread)
{
	struct packet * p = make_packet();

	if(thread < mp_threads->size)
	{
		/*
		 * The thread already exists,
		 * we just push back the new
		 * post on it.
		 */

		struct thread * th = at(mp_threads, thread);
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
		push_back(mp_threads, &th);

		printf("[i] Thread %ld has been created!\n", mp_threads->size);
	}

	return p;
}

struct packet * mp_request_threads(const struct session * se, uint16_t thread, uint16_t * n)
{
	struct packet * p = NULL;
	size_t size = 0;

	size_t bth = !thread ? 0 : thread - 1;
	size_t sth = !thread ? mp_threads->size : 1;

	if(!thread)
	{	
		for(size_t k = 0; k < sth; k++)
		{
			struct thread * th = at(mp_threads, k);
			size_t ps = th->posts->size;
		
			size += *n <= ps ? *n : ps;
		}
	}
	else if(thread - 1 < mp_threads->size)
	{
		struct thread * th = at(mp_threads, thread - 1);
		size_t ps = th->posts->size;
			
		size = *n <= ps ? *n : ps;
	}
	else
	{
		bth = sth = 0;
	}

	p = malloc((size + 1) * sizeof(struct packet));	
	memset(p, 0x0, (size + 1) * sizeof(struct packet));

	struct mp_header mhd = {FETCH, se->uid, !thread ? sth : thread, thread || !(*n) ? size : *n, 0};
	forge_header(MP_SERVER_SIDE, &p[0].header, mhd);

	for(uint16_t i = bth; i < bth + sth; i++)
	{
		struct thread * th = at(mp_threads, i);
		
		size_t ps = th->posts->size;
		size_t beg = *n <= ps ? ps - *n : 0;

		for(size_t k = beg; k < ps; k++)
		{
			struct post * post = at(th->posts, k);
			struct mp_post_header mph;

			memset(&mph, 0x0, sizeof(mph));

			mph.nthread = i + 1;
			memmove(mph.origin, &th->seed, 2);
			memmove(mph.pseudo, &post->uuid, 2);
			mph.len = strlen(post->data);
		}
	}

	*n = size + 1;

	return p;
}

struct packet * mp_subscribe(const struct session * se, uint16_t thread)
{
	struct packet * p = make_packet();
	
	if(thread && thread <= mp_threads->size)
	{
		struct thread * th = at(mp_threads, thread);

		p->header.fields = malloc(11 * FIELD_SIZE);
		uint16_t cu = fusion(SUBSCRIBE, se->uid);

		p->header.fields[MP_FIELD_CR_UUID] = htons(cu);
		p->header.fields[MP_FIELD_THREAD] = htons(thread);
		p->header.fields[MP_FIELD_NUMBER] = htons(MP_MULTICAST_PORT);
		memmove(p->header.fields + 6, &th->addr, 16);
	}

	return p;
}

int file_exists(uint16_t nthread, const char * filename)
{
	return 0;
}

struct packet * mp_process_data(const char * data, size_t * sp)
{
    struct packet * recv_p = melt_tcp_packet(data);
    struct packet * send_p = NULL;

    uint16_t lfield = recv_p->header.fields[MP_FIELD_CR_UUID];

    uint16_t code = get_rq_code(lfield);
    uid_t id = get_uuid(lfield);

    struct session * se = get_session(id);
    *sp = 1;

    if(!se) return send_p;
    
    switch (code)
    {
    case SIGNUP: 
	send_p = mp_signup(recv_p->data);
        break;

    case POST:
        uint16_t thread = recv_p->header.fields[MP_FIELD_THREAD];	
	struct post pt = {MESSAGE, se->uid, recv_p->data};

        send_p = mp_upload_post(se, &pt, thread);
	break;

    case FETCH: 
        send_p = mp_request_threads(
			se, 
			recv_p->header.fields[MP_FIELD_THREAD], 
			&recv_p->header.fields[MP_FIELD_NUMBER]
	);

	*sp = recv_p->header.fields[MP_FIELD_NUMBER];

	break;

    case SUBSCRIBE: 
        send_p = mp_subscribe(se, recv_p->header.fields[0]);
        break;

    case UPLOAD_FILE:
	
	if(file_exists(recv_p->header.fields[MP_FIELD_THREAD], recv_p->data))
	{
		/*
		 * If the file already exists on this thread
		 * it cannot be uploaded.
		 */

		send_p = make_packet();
	}
	else
	{
		send_p = recv_p;
		send_p->header.fields[MP_FIELD_NUMBER] = MP_UDP_PORT;

		/*
		 * Otherwise we start listening
		 * on the giver port.
		 */
	}
	break;

    case DOWNLOAD_FILE: 
	
	break;

    default:
	printf("[-] This request code is unknown!\n"); 
    }

    return send_p;
}
