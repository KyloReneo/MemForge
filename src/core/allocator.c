#include "../../include/memforge/memforge_internal.h"
#include "init.c";

// Core allocation functions

// Forge memory allocation
void *fmalloc(size_t size)
{
    const memforge_config_t config = setup_config();
    if (!memforge_initialized)
    {
        memforge_init(&config);
    }

    // malloc(0) behavior based on glibc doc : If size is 0, then malloc() returns a unique pointer value that can later be successfully passed to free().
    if (size == 0)
    {
        size = 1;
    }
}
void fmfree(void *ptr)
{
    if (ptr == NULL)
    {
        return;
    }
}
void *fmcalloc(size_t n, size_t size) {}
void *fmrealloc(void *ptr, size_t) {}