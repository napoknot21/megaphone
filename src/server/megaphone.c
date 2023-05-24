#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "megaphone.h"
#include "../utils/vector.h"
#include "../utils/string.h"

uint16_t i = 0;

struct vector * sessions;
struct vector * mp_threads;

struct session * get_session(const uid_t uuid)
{
	printf("[*] looking for session %d\n", uuid);
	for(size_t k = 0; k < sessions->size; k++)
	{
		struct session * se = at(sessions, k);
		if(se->uid == uuid) 
		{
			printf("[+] session found\n");
			return se;
		}
	}

	printf("[-] no such session exists\n");
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
	printf("[i] multicast address %s has been generated!\n", str);

	return addr;
}

void mp_init()
{
	sessions = make_vector(
		(void* (*)(const void*)) copy_session,
		(void (*)(void*)) free_session, 
		sizeof(struct session)
	);

	mp_threads = make_vector(
		(void* (*)(const void*)) copy_thread,
		(void (*)(void*)) free_thread,
		sizeof(struct thread)
	);

}

void mp_close()
{
	free_vector(sessions);
	free_vector(mp_threads);
}

struct packet * mp_signup(char * username, size_t len)
{
    struct packet * p = make_packet();

    printf("[*] sign-up request with pseudonyme %s\n", username);

    struct session se;
    memset(&se, 0x0, sizeof(se));

    se.uid = gen_id();
    se.username = malloc(len + 1);

    memset(se.username, 0x0, len + 1);
    memmove(se.username, username, len);

    push_back(sessions, &se); 

    struct mp_header mhd = {SIGNUP, se.uid, 0, 0, 0};
    forge_header(MP_SERVER_SIDE, &p->header, mhd); 

    return p;
}

struct packet * mp_upload_post(const struct session * se, struct post * pt, uint16_t thread)
{
	struct packet * p = make_packet();
	struct mp_header mhd = {POST, se->uid, 0, 0, 0};	

	printf("[*] post request on thread %d of length %ld.\n", thread, pt->len);

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
		printf("[*] new post has been uploaded on thread %d!\n", thread);
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
				(void* (*)(const void*)) copy_post, 
				(void (*)(void*)) free_post, 
				sizeof(struct post)
		);

		th.files = make_vector(
				(void* (*)(const void*)) copy_string,
				(void (*)(void*)) free_string,
				sizeof(struct string)
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
		
			size += *n && *n <= ps ? *n : ps;
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

	printf("[*] preparing %ld posts to send\n", size);

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

		 	p[packet_pos].data = malloc(post->len);
			memmove(p[packet_pos].data, post->data, post->len);

			p[packet_pos].size = post->len;
			forge_post_header(&p[packet_pos].header, mph);
	
			packet_pos++;
		}
	}

	*n = size + 1;

	return p;
}

struct packet * mp_subscribe(const struct session * se, uint16_t thread)
{
	struct packet * p = make_packet();
	printf("[i] subscribe request on thread %d.\n", thread);
	
	if(thread && thread <= mp_threads->size)
	{
		struct thread * th = at(mp_threads, thread - 1);
		struct mp_header mhd = {SUBSCRIBE, se->uid, thread, 0, 0};

		forge_header(MP_SERVER_SIDE, &p->header, mhd);
		p->size = 16;

		p->data = malloc(p->size);
		memmove(p->data, &th->addr, 16);
	}

	return p;
}

int file_exists(uint16_t nthread, const char * filename)
{
	printf("[*] looking for file %s on thread %d\n", filename, nthread);
	if(nthread && nthread <= mp_threads->size)
	{
		struct thread * th = at(mp_threads, nthread - 1);

		for(size_t i = 0; i < th->files->size; i++)
		{
			struct string * str = at(th->files, i);
			if(!strcmp(filename, (char*) str->vec->data))
			{
				printf("[+] %s exists on thread %d\n", filename, nthread);
				return 1;
			}
		}

		printf("[-] %s does not exist on thread %d\n", filename, nthread);	
	}
	else
	{
		printf("[-] thread %d does not exist!\n", nthread);
	}

	return 0;
}

struct packet * mp_io_file(const struct session * se, struct mp_header mhd, char * data, size_t len)
{
	struct packet * p = make_packet();
	char * filename = malloc(len + 1);

	memset(filename, 0x0, len + 1);
	memmove(filename, data, len);

	int fe = file_exists(mhd.nthread, filename);

	if((mhd.rc == UPLOAD_FILE && fe) || (mhd.rc == DOWNLOAD_FILE && !fe))
	{
		/*
		 * If the file already exists on this thread
		 * it cannot be uploaded.
		 */	
	}
	else
	{
		forge_header(MP_SERVER_SIDE, &p->header, mhd);

		/*
		 * Otherwise it starts 
		 * listening
		 * on the giver port. Creates the thread if it
		 * does not exist.
		 */
	}

	free(filename);

	return p;
}

struct packet * mp_process_data(struct packet * recv_p, size_t * sp)
{
    struct packet * send_p = NULL;
    struct mp_header mhd, smhd;

    melt_header(MP_CLIENT_SIDE, &mhd, &recv_p->header); 

    struct session * se = NULL;

    if(mhd.uuid)
    {
	    se = get_session(mhd.uuid);
    }

    if(!se && mhd.rc > SIGNUP)
    {
	    *sp = 0;
	    printf("[-] session uuid is unknown and this is not a sign-up request\n");
	    return send_p;
    }

    *sp = 1; 
    
    switch(mhd.rc)
    {
    case SIGNUP:
	printf("[i] Sign-up request!\n"); 
	send_p = mp_signup(recv_p->data, recv_p->size);
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
        send_p = mp_subscribe(se, mhd.nthread);
        break;

    case UPLOAD_FILE:
	printf("[i] Upload request!\n"); 	
	
	smhd.rc = UPLOAD_FILE;
	smhd.uuid = se->uid;
	smhd.nthread = mhd.nthread;
	smhd.n = MP_UDP_PORT;
	smhd.len = 0;

	send_p = mp_io_file(se, smhd, recv_p->data, recv_p->size);
	break;

    case DOWNLOAD_FILE: 	
	
	smhd.rc = DOWNLOAD_FILE;
	smhd.uuid = se->uid;
	smhd.nthread = mhd.nthread;
	smhd.n = 0;
	smhd.len = 0;

	send_p = mp_io_file(se, smhd, recv_p->data, recv_p->size);
	break;

    default:
	printf("[-] This request code is unknown!\n"); 
    	*sp = 0;
    }

    return send_p;
}

void udp_stage(int family, const char * to_ip, const struct packet * context)
{
	struct mp_header mhd;
	melt_header(MP_CLIENT_SIDE, &mhd, &context->header);

	if(mhd.rc < UPLOAD_FILE) return;

	printf("[*] UDP stage on %s\n", to_ip);

	struct session * se = get_session(mhd.uuid);

	char * filename = malloc(context->size + 1);

	memset(filename, 0x0, context->size + 1);
	memmove(filename, context->data, context->size);

	switch(mhd.rc)
	{
	case UPLOAD_FILE:
		
		/*
		 * Starting UDP listening
		 * to receive uploaded data
		 */

		download(filename, MP_UDP_PORT);
		struct thread * th = at(mp_threads, mhd.nthread - 1);

		struct string * filename_str = make_string();
		string_push_back(filename_str, filename, strlen(filename));
		
		push_back(th->files, filename_str);	
		printf("[+] %s successfully added to thread %d\n", filename, mhd.nthread);

		free(filename_str);

		break;

	case DOWNLOAD_FILE:

		/*
		 * Starting UDP socket
		 * to send data
		 */

		upload(family, to_ip, mhd.n, se, filename);

		break;
	}
}
