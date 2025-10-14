#include "../../include/memforge/memforge.h"
#include "../../include/memforge/memforge_config.h"

#include <stdio.h>
#include <stdlib.h>

// Global state definitions

// Initialization flag
bool memforge_initialized = false;

// Statistics
memforge_stats_t memforge_stats = {0};

// Default config
const memforge_config_t default_memforge_config = {
    .mmap_threshold = MEMFORGE_MMAP_THRESHOLD,
    .page_size = MEMFORGE_PAGE_SIZE,
    .strategy = MEMFORGE_STRATEGY_HYBRID,
    .debug_level = 0,
    .thread_safe = true};

// future works
// main arena

// Environment variable tuning
static memforge_config_t custom_environment_tuning(void)
{
    // Custom config
    memforge_config_t custom_memforge_config = {0};

    char *env = NULL;

    // Mmap threshold
    env = getenv("MEMFORGE_MMAP_THRESHOLD");
    // Default : glibc threshold, 128 KB
    if (env != NULL)
    {
        long threshold = atol(env);
        if (threshold >= 0)
        {
            custom_memforge_config.mmap_threshold = (size_t)threshold;
            debug_log("Enviroment tuning log: MEMFORGE_MMAP_THRESHOLD = %zu", threshold);
        }
    }

    // Page size
    env = getenv("MEMFORGE_PAGE_SIZE");
    // Default : 4096
    if (env != NULL)
    {
        long page = atol(env);
        if (page >= 0)
        {
            custom_memforge_config.page_size = (size_t)page;
            debug_log("Enviroment tuning log: MEMFORGE_PAGE_SIZE = %zu", page);
        }
    }

    // Strategy
    env = getenv("MEMFORGE_STRATEGY_HYBRID");
    // Default : Hybrid
    if (env != NULL)
    {
        long tegy = atol(env);
        if (tegy >= 0)
        {
            custom_memforge_config.strategy = (size_t)tegy;
            debug_log("Enviroment tuning log: MEMFORGE_STRATEGY_HYBRID = %zu", tegy);
        }
    }

    // MEMFORGE_DEBUG
    env = getenv("MEMFORGE_DEBUG");
    // Default : 0
    if (env != NULL)
    {
        custom_memforge_config.debug_level = atoi(env);
        debug_log("Environment tuning log: debug_level = %d", custom_memforge_config.debug_level);
    }

    env = NULL;

    return custom_memforge_config;
}

// Configuration setup
const memforge_config_t setup_config(void)
{

    // Detecting system page size ----> future work!

    const memforge_config_t custom = custom_environment_tuning();

    // Choosing config
    if (custom.mmap_threshold != default_memforge_config.mmap_threshold ||
        custom.page_size != default_memforge_config.page_size ||
        custom.strategy != default_memforge_config.strategy ||
        custom.debug_level != default_memforge_config.debug_level)
    {
        debug_log("Setup_config log: Custom config is set: %zu", custom);

        return custom;
    }
    else
    {
        debug_log("Setup_config log: Default config is set: %zu", default_memforge_config);
        return default_memforge_config;
    }
}

int memforge_init(const memforge_config_t *config)
{
    // Already initialized
    if (memforge_initialized)
    {
        debug_log("memforge_init log: Already initialized!");
        return 0;
    }

    // Set initialize statistics
    memforge_stats.total_mapped = 0;
    memforge_stats.total_allocated = 0;
    memforge_stats.total_freed = 0;
    memforge_stats.current_usage = 0;
    memforge_stats.peak_usage = 0;
    memforge_stats.allocation_count = 0;
    memforge_stats.free_count = 0;
    memforge_stats.mmap_count = 0;
    memforge_stats.sbrk_count = 0;

    // // Initialize free lists
    // for (size_t i = 0; i < MEMFORGE_SIZE_CLASS_COUNT; i++)
    // {
    //     free_lists[i] = NULL;
    // }

    // heap_segments = NULL;
    // memforge_initialized = true;

    // printf("Memory allocator initialized\n");
    // return 0;
}

// void memforge_cleanup(void)
// {
//     if (!memforge_initialized)
//     {
//         return;
//     }

//     // Free all heap segments
//     heap_segment_t *seg = heap_segments;
//     while (seg != NULL)
//     {
//         heap_segment_t *next = seg->next;
//         system_free_mmap(seg, sizeof(heap_segment_t));
//         seg = next;
//     }
//     heap_segments = NULL;

//     memforge_initialized = false;
//     printf("Memory allocator cleaned up\n");
// }