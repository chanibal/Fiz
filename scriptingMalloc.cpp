#include <cstdlib>
#include <malloc.h>
#include <cstring>
#include <cinttypes>

#ifdef ESP_PLATFORM
#include <esp_log.h>
#include <esp_heap_caps.h>
#else
#define ESP_LOGV( tag, format, ... )  printf("V %s: " format "\n", tag, ##__VA_ARGS__)
#define ESP_LOGD( tag, format, ... )  printf("D %s: " format "\n", tag, ##__VA_ARGS__)
#define ESP_LOGI( tag, format, ... )  printf("I %s: " format "\n", tag, ##__VA_ARGS__)
#define ESP_LOGW( tag, format, ... )  printf("W %s: " format "\n", tag, ##__VA_ARGS__)
#define ESP_LOGE( tag, format, ... )  printf("E %s: " format "\n", tag, ##__VA_ARGS__)
#define heap_caps_malloc(size, caps) malloc(size)
#define heap_caps_free(ptr) free(ptr)
#define ESP_LOG_BUFFER_HEXDUMP(tag, buffer, bufferSize, level) do {} while(0)
#endif



// Warning: do not include scriptingMalloc.h in this file

#define MEMDEBUG
#define MEMDEBUG_VERBOSE

#ifdef MEMDEBUG_VERBOSE
#define info(...) ESP_LOGI ("scripting", ##__VA_ARGS__)
#define error(...) ESP_LOGI ("scripting", ##__VA_ARGS__)
#else
#define info(...) do {} while(0)
#define error(...) do {} while(0)
#endif

#ifndef MEMDEBUG
extern "C" bool scripting_memreport() { return true; }
void memory_alloced(void* ptr, size_t size) {}
void memory_dealloced(void* ptr) {}
size_t memory_size(void* ptr) { return 0; }
#else


#include <map>
std::map<void*, size_t> memory_sizes{};

size_t scripting_memory = 0;
int scripting_allocations = 0;
int scripting_deallocations = 0;
size_t scripting_memory_high = 0;

size_t memory_size(void* ptr)
{
    if(!ptr) 
    {
        error("Memory size of nullptr?");
        return 0;
    }
    const auto s = memory_sizes.find(ptr);
    if(s == memory_sizes.end())
    {
        ESP_LOGW("scripting", "Memory size not found for %p", ptr);
        exit(6);
        return 0;
    }

    return s->second;
}


void memory_alloced(void* ptr, size_t size)
{
    if(!ptr) 
    {
        error("Allocing memory size of nullptr?");
        return;
    }
    scripting_memory += size;
    scripting_allocations++;
    if(scripting_memory_high < scripting_memory) scripting_memory_high = scripting_memory;
    memory_sizes[ptr] = size;
}


void memory_dealloced(void* ptr)
{
    if(!ptr) 
    {
        error("Deallocing memory size of nullptr?");
        return;
    }
    const auto size = memory_size(ptr);
    scripting_memory -= size;
    scripting_deallocations++;
    memory_sizes.erase(ptr);
}


extern "C" bool scripting_memreport()
{
    // Do not replace with info(...)
    ESP_LOGI("scripting", "Memory size %" PRIdMAX " bytes (top: %" PRIdMAX ") with %d/%d allocations/deallocations", scripting_memory, scripting_memory_high, scripting_allocations, scripting_deallocations);
    return scripting_memory == 0 && scripting_allocations == scripting_deallocations;
}
#endif



extern "C" void* scripting_calloc(size_t nmeb, size_t size)
{
    void* ptr = calloc(nmeb, size);
    memory_alloced(ptr, nmeb * size);
    info("Calloced %" PRIdMAX " bytes at %p", nmeb * size, ptr);
    return ptr;
}


extern "C" void* scripting_malloc(size_t size)
{
    void* ptr = malloc(size);
    memory_alloced(ptr, size);
    info("Malloced % " PRIdMAX " bytes at %p", size, ptr);
    return ptr;
}


extern "C" void scripting_free(void *ptr)
{
    info("Free %" PRIdMAX " bytes at %p", memory_size(ptr), ptr);
    memory_dealloced(ptr);
    free(ptr);

    if(scripting_allocations == scripting_deallocations) info("Everything clear!");
}


extern "C" void* scripting_realloc(void *ptr, size_t size)
{
    info("Realloc/free %" PRIdMAX " bytes at %p", memory_size(ptr), ptr);
    memory_dealloced(ptr);

    ptr = realloc(ptr, size);

    info("Realloc/alloc %" PRIdMAX " bytes at %p", size, ptr);
    memory_alloced(ptr, size);

    return ptr;
}


extern "C" char* scripting_strdup(const char *str)
{
    const auto copy = strdup(str);
    const auto length = strlen(copy);

    memory_alloced(reinterpret_cast<void*>(copy), length);
    info("Strdup %" PRIdMAX " bytes from %p to %p", length, str, copy);

    return copy;
}


extern "C" char* scripting_strndup(const char *str, const size_t length)
{
    const auto copy = strndup(str, length);

    memory_alloced(reinterpret_cast<void*>(copy), strlen(copy));
    info("Strndup %" PRIdMAX " bytes from %p to %p", length, str, copy);

    return copy;
}
