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
	struct packet * p = malloc(sizeof(struct packet)*(n+1));

	memset(p, 0x0, sizeof(struct packet));
	memset(&p->header, 0x0, sizeof(struct header));

	struct thread * th = at(threads, thread);

	ssize_t nb_msg = th->posts->size < n ? th->posts->size : n;

	p[0].header.fields[MP_FIELD_THREAD] = htonl(thread);
	sprintf(p[0].data, "Voici les %ld premier messages du fil %d", nb_msg, thread);
	p[0].size = strlen(p[0].data);

	for (int i = 0; i < nb_msg; i++)
	{
		p[i+1].header.fields[MP_FIELD_THREAD] = htonl(thread);
		p[i+1].data = (char*) (th->posts)[i].data;
		p[i+1].size = strlen(p[i+1].data);
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

    case DOWNLOAD: break;

    default:
	printf("[-] This request code is unknown!\n");
        break;
    }

    return send_p;
}
