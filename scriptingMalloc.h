// Hack for more control of memory in scripting
// Warning: do not include this .h outside of scripting engine implementation - not in it's public headers
#pragma once

#ifdef __cplusplus
#error Should be only used in C includes of scripting engine
#endif

#include <stddef.h>
#define malloc scripting_malloc
#define free scripting_free
#define calloc scripting_calloc
#define realloc scripting_realloc
#define strdup scripting_strdup
#define strndup scripting_strndup

void* scripting_calloc(size_t nmeb, size_t size);
void* scripting_malloc(size_t size);
void  scripting_free(void *ptr);
void* scripting_realloc(void *ptr, size_t size);
char* scripting_strdup(const char *str);
char* scripting_strndup(const char *str, const size_t length);
// bool scripting_memreport();
