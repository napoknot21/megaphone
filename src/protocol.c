#include "forge.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

request_code_t string_to_code(const char * str)
{
	if(!strcmp(str, "signup")) 
		return SIGNUP;
	else if(!strcmp(str, "post")) 
		return POST;
	else if(!strcmp(str, "fetch"))
		return FETCH;
	else if(!strcmp(str, "subscribe"))
		return SUBSCRIBE;
	else if(!strcmp(str, "download"))
		return DOWNLOAD;	

	return UNKNOWN;
}
