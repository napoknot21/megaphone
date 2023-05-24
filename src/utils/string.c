#include "string.h"
#include <string.h>

#define DEFAULT_STRING_CAPACITY 64
#define DEFAULT_STRING_REALLOCATION_FACTOR 2

struct string * make_string()
{
	struct string * str = malloc(sizeof(struct string));
	memset(str, 0x0, sizeof(struct string));

	str->vec = make_vector(
			NULL,
			NULL,
			sizeof(char)
			);
	
	return str;
}

struct string * copy_string(const struct string * model)
{
	struct string * copy = make_string();
	copy->vec = copy_vector(model->vec);

	return copy;
}

void free_string(struct string * str)
{
	free_vector(str->vec);
	free(str);
}

void string_push_back(struct string * str, const char * data, size_t len)
{
	for(size_t i = 0; i < len; i++) push_back(str->vec, &data[i]);
}

char string_pop_back(struct string * str)
{
	char * lst = at(str->vec, str->vec->size - 1);	
	pop_back(str->vec);
	
	return *lst;
}

char * string_at(const struct string * str, size_t i)
{
	return at(str->vec, i);
}
