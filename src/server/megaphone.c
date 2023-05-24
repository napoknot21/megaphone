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
	printf("Getting session ... %ld\n", sessions->size);
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

struct in6_addr gen_multicast_addr()
{
	struct in6_addr addr;

	if(!mp_threads->size)
	{
		inet_pton(AF_INET6, "ff12::0", &addr);
	}
	else
	{
		struct thread * th = at(mp_threads, mp_threads->size - 1);
		addr = th->addr;

		addr.s6_addr[15] = (addr.s6_addr[0] + 1) % 0x10000;
	}

	char str[INET6_ADDRSTRLEN];
	memset(str, 0x0, INET6_ADDRSTRLEN);

	inet_ntop(AF_INET6, &addr, str, INET6_ADDRSTRLEN);
	printf("[i] %s has been generated!\n", str);

	return addr;
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
		(void (*)(void*)) free_thread,
		sizeof(struct thread)
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

    push_back(sessions, &se);

    printf("[i] %s signed-up!\n", username);

    struct mp_header mhd = {SIGNUP, se.uid, 0, 0, 0};
    forge_header(MP_SERVER_SIDE, &p->header, mhd); 

    return p;
}

struct packet * mp_upload_post(const struct session * se, struct post * pt, uint16_t thread)
{
	struct packet * p = make_packet();
	struct mp_header mhd = {POST, se->uid, 0, 0, 0};	

	printf("Posting post of length %ld\n", pt->len);

	thread = !thread ? 1 : thread;

	if(thread - 1 < mp_threads->size)
	{
		/*
		 * The thread already exists,
		 * we just push back the new
		 * post on it.
		 */

		struct thread * th = at(mp_threads, thread - 1);

		push_back(th->posts, pt);
	
		mhd.nthread = thread;
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
		th.addr = gen_multicast_addr();
		th.posts = make_vector(
				(void* (*)(void*)) copy_post, 
				(void (*)(void*)) free_post, 
				sizeof(struct post)
		);

		push_back(th.posts, pt);	
		push_back(mp_threads, &th);

		mhd.nthread = mp_threads->size;

		printf("[i] Thread %ld has been created!\n", mp_threads->size);
	}

	forge_header(MP_SERVER_SIDE, &p->header, mhd);

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
			
		size = *n && *n <= ps ? *n : ps;
	}
	else
	{
		bth = sth = 0;
	}

	printf("[i] Preparing %ld posts...\n", size);

	p = malloc((size + 1) * sizeof(struct packet));	
	memset(p, 0x0, (size + 1) * sizeof(struct packet));

	struct mp_header mhd = {
		FETCH, 
		se->uid, 
		!thread ? sth : thread, 
		thread || !(*n) ? size : *n, 
		0
	};

	forge_header(MP_SERVER_SIDE, &p[0].header, mhd);	

	size_t packet_pos = 1;

	for(size_t i = bth; i < bth + sth; i++)
	{
		struct thread * th = at(mp_threads, i);
		
		size_t ps = th->posts->size;
		size_t beg = *n && *n <= ps ? ps - *n : 0;	

		for(size_t k = beg; k < ps; k++)
		{
			struct post * post = at(th->posts, k);
			struct mp_post_header mph;

			memset(&mph, 0x0, sizeof(mph));

			mph.nthread = i + 1;
			memmove(mph.origin, &th->seed, 2);
			memmove(mph.pseudo, &post->uuid, 2);
			mph.len = post->len;

			printf("Post of length %d %s\n", mph.len, post->data);

		 	p[packet_pos].data = malloc(post->len);
			memmove(p[packet_pos].data, post->data, post->len);

			p[packet_pos].size = post->len;
			forge_post_header(&p[packet_pos].header, mph);

			printf("Header len field: %d\n", ntohs(p[packet_pos].header.fields[11]));
			packet_pos++;
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

struct packet * mp_process_data(struct packet * recv_p, size_t * sp)
{
    struct packet * send_p = NULL;
    struct mp_header mhd;

    melt_header(MP_CLIENT_SIDE, &mhd, &recv_p->header); 

    struct session * se = NULL;

    if(mhd.uuid)
    {
	    se = get_session(mhd.uuid);
    }

    if(!se && mhd.rc > SIGNUP)
    {
	    *sp = 0;
	    printf("[-] Session uuid is unknown and this is not a sign-up request!");
	    return send_p;
    }

    *sp = 1; 
    
    switch(mhd.rc)
    {
    case SIGNUP:
	printf("[i] Sign-up request!\n"); 
	send_p = mp_signup(recv_p->data);
        break;

    case POST:
       	printf("[i] Post request!\n");  
      	
	struct post pt = {MESSAGE, se->uid, recv_p->data, mhd.len};
        send_p = mp_upload_post(se, &pt, mhd.nthread);
	break;

    case FETCH: 
	printf("[i] Fetch request!\n"); 
        send_p = mp_request_threads(
			se, 
			mhd.nthread, 
			&mhd.n
	);

	*sp = mhd.n;

	break;

    case SUBSCRIBE:
	printf("[i] Subscribe request!\n"); 
        send_p = mp_subscribe(se, recv_p->header.fields[0]);
        break;

    case UPLOAD_FILE:
	printf("[i] Upload request!\n"); 	
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
