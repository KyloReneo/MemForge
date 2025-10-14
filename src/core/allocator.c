#include "../../include/memforge/memforge_config.h"

// Global allocator state
// We have to define some states to implement the allocation logic
static bool initialized = false;
static heap_segment_t *heap_segments = NULL;
static block_header_t *free_lists[FMALLOC_SIZE_CLASS_COUNT] = {NULL};
static fmalloc_stats_t stats = {0};
static bool debug_enabled = false;