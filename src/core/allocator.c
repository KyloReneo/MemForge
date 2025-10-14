#include "../../include/memforge/memforge_config.h"

// Global allocator state
// We have to define some states to implement the allocation logic

// Global state
bool memforge_initialized = false;
fmalloc_stats_t memforge_stats = {0};
static heap_segment_t *heap_segments = NULL;
static block_header_t *free_lists[FMALLOC_SIZE_CLASS_COUNT] = {NULL};
static bool debug_enabled = false;

// Core allocation functions
void *fmalloc(size_t size)
{
    if (!memforge_initialized)
    {
        memforge_init();
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