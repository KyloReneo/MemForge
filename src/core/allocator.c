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
void *fmalloc(size_t size) {}
void fmfree(void *ptr)
{
    if (ptr == NULL)
    {
        return;
    }
}
void *fmcalloc(size_t n, size_t size) {}
void *fmrealloc(void *ptr, size_t) {}