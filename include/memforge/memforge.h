#ifndef MEMFORGE_H
#define MEMFORGE_H

#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // ============================================================================
    // PUBLIC API CONFIGURATION
    // ============================================================================

    // Memory allocation strategies
    typedef enum allocation_strategies
    {
        MEMFORGE_STRATEGY_FIRST_FIT,
        MEMFORGE_STRATEGY_BEST_FIT,
        MEMFORGE_STRATEGY_HYBRID
    } memforge_strategy_t;

    // Arena mapping strategies
    typedef enum arena_strategies
    {
        MEMFORGE_ARENA_DEFAULT = 0,      // Auto-select based on heuristics
        MEMFORGE_ARENA_PER_THREAD,       // One arena per thread (best performance)
        MEMFORGE_ARENA_ROUND_ROBIN,      // Round-robin distribution (good balance)
        MEMFORGE_ARENA_CONTENTION_AWARE, // Smart allocation based on contention
        MEMFORGE_ARENA_SINGLE,           // Single arena (minimal memory usage)
        MEMFORGE_ARENA_CUSTOM            // User-provided mapping function
    } memforge_arena_strategy_t;

    // Configuration structure for allocator customization
    typedef struct config
    {
        size_t page_size;
        size_t mmap_threshold;
        memforge_strategy_t strategy;
        bool thread_safe;
        bool debug_enabled;
        size_t arena_count;
    } memforge_config_t;

    // Statistics structure for monitoring allocator performance
    typedef struct stats
    {
        size_t total_allocated;
        size_t total_freed;
        size_t current_usage;
        size_t peak_usage;
        size_t allocation_count;
        size_t free_count;
        size_t mmap_count;
        size_t heap_expansions;
    } memforge_stats_t;

    // ============================================================================
    // PUBLIC API FUNCTIONS
    // ============================================================================

    // Core allocation functions (replace standard library)

    void *memforge_malloc(size_t size);
    void memforge_free(void *ptr);
    void *memforge_calloc(size_t n, size_t size);
    void *memforge_realloc(void *ptr, size_t);

    // Allocator lifecycle management

    int memforge_init(const memforge_config_t *config);
    void memforge_cleanup(void);
    void memforge_reset(void);

    // Memory alignment utilities

    void *align(size_t alignment, size_t size);
    int memgorge_posix_align(void **memptr, size_t alignment, size_t size);
    void *memforge_aligned_alloc(size_t alignment, size_t size);
    size_t memforge_get_alignment(void *ptr);

    // Configuration and statistics

    void memforge_get_stats(memforge_stats_t *stats);
    void memforge_set_strategy(memforge_strategy_t strategy);
    void memforge_set_mmap_threshold(size_t threshold);

    // Debugging and diagnostics

    void memforge_dump_heap(void);
    bool memforge_validate_heap(void);
    void memforge_enable_debug(bool enable);

    // Advanced features

    void memforge_compact(void);
    size_t memforge_usable_size(void *ptr);

    // Utility functions
    size_t memforge_malloc_usable_size(void *ptr);
    void memforge_malloc_stats(void);
    int memforge_malloc_trim(size_t pad);
    void memforge_malloc_info(int options, FILE *stream);

    // // Debug and control functions
    // void fmalloc_debug(int level);
    // void fmalloc_verify(void);
    // int fmalloc_set_state(void *state);
    // void *fmalloc_get_state(void);

#ifdef __cplusplus
}
#endif

#endif