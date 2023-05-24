#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include "vector.h"

struct string
{
	struct vector * vec;
};

struct string * make_string();
struct string * copy_string(const struct string*);

void string_push_back(struct string *, const char *, size_t);
char string_pop_back();

char * at_string(const struct string*, size_t);

void free_string(struct string *);

#endif
