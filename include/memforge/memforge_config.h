#ifndef MEMFORGE_CONFIG_H
#define MEMFORGE_CONFIG_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

// Configuration constants
#define FMALLOC_PAGE_SIZE 4096
#define FMALLOC_MMAP_THRESHOLD (128 * 1024)  // 128KB
#define FMALLOC_MIN_ALLOC_SIZE 16
#define FMALLOC_ALIGNMENT 8
#define FMALLOC_MAGIC_NUMBER 0xDEADBEEF

// Alignment macros
#define FMALLOC_ALIGN(size) (((size) + (FMALLOC_ALIGNMENT-1)) & ~(FMALLOC_ALIGNMENT-1))
#define FMALLOC_IS_ALIGNED(ptr) (((uintptr_t)(ptr) & (FMALLOC_ALIGNMENT-1)) == 0)

// Size classes for small allocations
#define FMALLOC_SIZE_CLASSES {16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192}
#define FMALLOC_SIZE_CLASS_COUNT 10

// Allocation strategies
typedef enum {
    FMALLOC_STRATEGY_FIRST_FIT = 0,
    FMALLOC_STRATEGY_BEST_FIT,
    FMALLOC_STRATEGY_HYBRID
} fmalloc_strategy_t;

// Block header structure
typedef struct block_header {
    size_t size;
    bool is_free;
    bool is_mapped;
    struct block_header* next;
    struct block_header* prev;
    uint32_t magic;
    #ifdef FMALLOC_DEBUG
    const char* file;
    int line;
    #endif
} block_header_t;

#define BLOCK_HEADER_SIZE sizeof(block_header_t)

// Heap segment structure
typedef struct heap_segment {
    void* base;
    size_t size;
    struct heap_segment* next;
    struct heap_segment* prev;
} heap_segment_t;

// Statistics structure
typedef struct fmalloc_stats {
    size_t total_mapped;
    size_t total_allocated;
    size_t total_freed;
    size_t current_usage;
    size_t peak_usage;
    size_t allocation_count;
    size_t free_count;
    size_t mmap_count;
    size_t sbrk_count;
} fmalloc_stats_t;

#endif 