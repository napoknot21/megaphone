#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>

struct string
{
	char * data;

	size_t capacity;
	size_t size;
};

struct string * make_string();
void string_push_back(struct string *, const char *, size_t);
char pop_back();
void free_string(struct string *);

#endif
