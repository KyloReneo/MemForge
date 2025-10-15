#ifndef MEMFORGE_INTERNAL_H
#define MEMFORGE_INTERNAL_H

#include "memforge_config.h"
#include <pthread.h>
#include <stddef.h>
#include <stdbool.h>

// ============================================================================
// INTERNAL DATA STRUCTURES
// ============================================================================

// Block header structure - stored before each allocation
typedef struct block_header
{
    size_t size;               // Size of user data area
    struct block_header *next; // Next block in free list or heap
    struct block_header *prev; // Previous block (for coalescing)
    bool is_free;              // Whether block is allocated or free
    bool is_mapped;            // Whether block is mmap'd (not from heap)
    unsigned int magic;        // Magic number for corruption detection
} block_header_t;

#define BLOCK_HEADER_SIZE MEMFORGE_ALIGN(sizeof(block_header_t))

// Heap segment tracking
typedef struct heap_segment
{
    void *base;
    size_t size;
    struct heap_segment *next;
} heap_segment_t;

// Memory arena for thread-local allocation
typedef struct memforge_arena
{
    pthread_mutex_t lock;                                  // Arena-specific lock
    block_header_t *free_lists[MEMFORGE_SIZE_CLASS_COUNT]; // Segregated free lists
    heap_segment_t *heap_segments;                         // Heap segments owned by this arena
    size_t allocated;                                      // Bytes allocated in this arena
    size_t freed;                                          // Bytes freed in this arena
} memforge_arena_t;

// ============================================================================
// GLOBAL STATE DECLARATIONS
// ============================================================================

extern memforge_config_t memforge_config;
extern memforge_stats_t memforge_stats;
extern memforge_arena_t *memforge_main_arena;
extern memforge_arena_t **memforge_arenas;
extern bool memforge_initialized;
extern size_t memforge_size_classes[MEMFORGE_SIZE_CLASS_COUNT];

// ============================================================================
// INTERNAL FUNCTION DECLARATIONS
// ============================================================================

// Initialization
int memforge_init_default_config(void);
int memforge_init_arenas(void);

// System memory management
void *system_alloc_mmap(size_t size);
void *system_alloc_sbrk(size_t size);
void system_free_mmap(void *ptr, size_t size);

// Heap management
heap_segment_t *heap_segment_create(void *base, size_t size);
void heap_segment_destroy(heap_segment_t *segment);

// Arena management
memforge_arena_t *get_current_arena(void);
memforge_arena_t *arena_create(void);
void arena_destroy(memforge_arena_t *arena);

// Utility functions
void debug_log(const char *format, ...);
bool is_power_of_two(size_t x);
size_t next_power_of_two(size_t x);

// Memory validation
bool block_validate(block_header_t *block);
bool heap_validate(void);

// Platform-specific threading
int thread_get_id(void);

#endif // MEMFORGE_INTERNAL_H

// Old internal private functions

// // Platform abstraction
// void *system_alloc_sbrk(size_t size);
// void *system_alloc_mmap(size_t size);
// void system_free_sbrk(void *ptr, size_t size);
// void system_free_mmap(void *ptr, size_t size);
// size_t system_page_size(void);
// int system_init(void);
// void system_cleanup(void);

// // Threading
// memforge_arena_t *get_current_arena(void);
// memforge_arena_t *arena_create(void);
// void *arena_malloc(size_t size);
// void arena_free(void *ptr);

// // Free list management
// void free_list_init(memforge_arena_t *arena);
// void free_list_add(memforge_arena_t *arena, block_header_t *block);
// void free_list_remove(memforge_arena_t *arena, block_header_t *block);
// block_header_t *free_list_find(memforge_arena_t *arena, size_t size);
// size_t get_size_class(size_t size);

// // Block management
// block_header_t *block_split(block_header_t *block, size_t size);
// block_header_t *block_coalesce(block_header_t *block);
// bool block_validate(block_header_t *block);

// // Heap management
// heap_segment_t *heap_segment_create(void *base, size_t size);
// void heap_segment_destroy(heap_segment_t *segment);
// void heap_verify(void);

// // Size class management
// size_t size_class_get(size_t size);
// size_t size_class_from_index(size_t index);

// // Environment and configuration
// void apply_environment_tuning(void);
// void setup_default_config(void);

// // Debug utilities
// #ifdef MEMFORGE_DEBUG
// void debug_log(const char *format, ...);
// void debug_dump_block(block_header_t *block);
// void debug_dump_heap(void);
// #else
// #define debug_log(...)
// #define debug_dump_block(block)
// #define debug_dump_heap()
// #endif

// #endif