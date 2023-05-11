#include "string.h"
#include <string.h>

#define DEFAULT_STRING_CAPACITY 64
#define DEFAULT_STRING_REALLOCATION_FACTOR 2

struct string * make_string()
{
	struct string * str = malloc(sizeof(struct string));

	memset(str, 0x0, sizeof(struct string));

	str->data = malloc(DEFAULT_STRING_CAPACITY);
	str->capacity = DEFAULT_STRING_CAPACITY;
	str->size = 0;

	return str;
}

void free_string(struct string * str)
{
	free(str->data);
	free(str);
}

void string_push_back(struct string * str, const char * data, size_t len)
{
	while(str->size + len > str->capacity)
	{
		str->capacity *= DEFAULT_STRING_REALLOCATION_FACTOR;
		str->data = realloc(str->data, str->capacity);
	}

	memmove(str->data + str->size, data, len);
	str->size += len;
}

char string_pop_back(struct string * str)
{
	char copy = 0;
	
	if(str->size)
	{
		size_t pos = str->size - 1;

		memmove(&copy, str->data + pos, 1);
		memset(str->data + pos, 0x0, 1);
	}

	return copy;
}
