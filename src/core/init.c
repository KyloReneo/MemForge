// src/core/init.c
#include "../../include/memforge/memforge.h"
#include "../../include/memforge/memforge_config.h"
#include "../core/allocator.c"
#include <stdio.h>

int memforge_init(void)
{
    if (initialized)
    {
        return 0;
    }

    // Set initialize statistics
    stats.total_mapped = 0;
    stats.total_allocated = 0;
    stats.total_freed = 0;
    stats.current_usage = 0;
    stats.peak_usage = 0;
    stats.allocation_count = 0;
    stats.free_count = 0;

    // Initialize free lists
    for (size_t i = 0; i < FMALLOC_SIZE_CLASS_COUNT; i++)
    {
        free_lists[i] = NULL;
    }

    heap_segments = NULL;
    initialized = true;

    printf("Memory allocator initialized\n");
    return 0;
}

void memforge_cleanup(void)
{
    if (!initialized)
    {
        return;
    }

    // Free all heap segments
    heap_segment_t *seg = heap_segments;
    while (seg != NULL)
    {
        heap_segment_t *next = seg->next;
        system_free_mmap(seg, sizeof(heap_segment_t));
        seg = next;
    }
    heap_segments = NULL;

    initialized = false;
    printf("Memory allocator cleaned up\n");
}