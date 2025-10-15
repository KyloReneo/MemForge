#ifndef MEMFORGE_CONFIG_H
#define MEMFORGE_CONFIG_H

#include <stddef.h>
#include <stdbool.h>

// ============================================================================
// COMPILE-TIME CONFIGURATION
// ============================================================================

// future work!

// ============================================================================
// ALLOCATOR CONSTANTS
// ============================================================================

// Memory alignment (typically 8 or 16 bytes for most systems)
#define MEMFORGE_ALIGNMENT 8
// Alignment macros
#define MEMFORGE_ALIGN(size) (((size) + (MEMFORGE_ALIGNMENT - 1)) & ~(MEMFORGE_ALIGNMENT - 1))
#define MEMFORGE_IS_ALIGNED(ptr) (((uintptr_t)(ptr) & (MEMFORGE_ALIGNMENT - 1)) == 0)

// Minimum allocation size (must accommodate block header + minimal data)
#define MEMFORGE_MIN_ALLOC_SIZE 16

// Default mmap threshold based on glibc (larger allocations use mmap directly)
#define MEMFORGE_DEFAULT_MMAP_THRESHOLD (128 * 1024) // 128KB

// Size class definitions for segregated free lists
#define MEMFORGE_SIZE_CLASS_COUNT 16
extern const size_t memforge_size_classes[MEMFORGE_SIZE_CLASS_COUNT];

// Magic number for memory corruption detection
#define MEMFORGE_MAGIC_NUMBER 0xDEADBEEF

// ============================================================================
// DEBUGGING AND SAFETY FEATURES
// ============================================================================

// Enable debug logging (can be disabled for production)
#ifdef MEMFORGE_DEBUG
#define DEBUG_LOGGING 1
#else
#define DEBUG_LOGGING 0
#endif

// Safety checks (can be disabled for maximum performance)
#define MEMFORGE_SAFETY_CHECKS 1

// Thread safety configuration
#define MEMFORGE_THREAD_SAFE 1

// ============================================================================
// PERFORMANCE TUNING
// ============================================================================

// Initial heap size
#define MEMFORGE_INITIAL_HEAP_SIZE (128 * 1024) // 128KB

// Maximum heap size before using mmap exclusively
#define MEMFORGE_MAX_HEAP_SIZE (16 * 1024 * 1024) // 16MB

// Arena count for multi-threaded performance
#define MEMFORGE_DEFAULT_ARENA_COUNT 4

#endif

// Old configuration

// // Configuration constants
// #define MEMFORGE_PAGE_SIZE 4096
// #define MEMFORGE_MMAP_THRESHOLD (128 * 1024) // 128KB
// #define MEMFORGE_MIN_ALLOC_SIZE 16
// #define MEMFORGE_ALIGNMENT 8
// #define MEMFORGE_MAGIC_NUMBER 0xDEADBEEF

// // Alignment macros
// #define MEMFORGE_ALIGN(size) (((size) + (MEMFORGE_ALIGNMENT - 1)) & ~(MEMFORGE_ALIGNMENT - 1))
// #define MEMFORGE_IS_ALIGNED(ptr) (((uintptr_t)(ptr) & (MEMFORGE_ALIGNMENT - 1)) == 0)

// // Size classes for small allocations
// #define MEMFORGE_SIZE_CLASSES {16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192}
// #define MEMFORGE_SIZE_CLASS_COUNT 10

// // Allocation strategies
// typedef enum allocation_strategies
// {
//     MEMFORGE_STRATEGY_FIRST_FIT = 0,
//     MEMFORGE_STRATEGY_BEST_FIT,
//     MEMFORGE_STRATEGY_HYBRID
// } memforge_allocation_strategy_t;

// // Arena mapping strategies
// typedef enum arena_strategies
// {
//     MEMFORGE_ARENA_DEFAULT = 0,      // Auto-select based on heuristics
//     MEMFORGE_ARENA_PER_THREAD,       // One arena per thread (best performance)
//     MEMFORGE_ARENA_ROUND_ROBIN,      // Round-robin distribution (good balance)
//     MEMFORGE_ARENA_CONTENTION_AWARE, // Smart allocation based on contention
//     MEMFORGE_ARENA_SINGLE,           // Single arena (minimal memory usage)
//     MEMFORGE_ARENA_CUSTOM            // User-provided mapping function
// } memforge_arena_strategy_t;

// // Block header structure
// typedef struct block_header
// {
//     size_t size;
//     bool is_free;
//     bool is_mapped;
//     struct block_header *next;
//     struct block_header *prev;
//     uint32_t magic;
// #ifdef MEMFORGE_DEBUG
//     const char *file;
//     int line;
// #endif
// } block_header_t;

// #define BLOCK_HEADER_SIZE sizeof(block_header_t)

// // Heap segment structure
// typedef struct heap_segment
// {
//     void *base;
//     size_t size;
//     struct heap_segment *next;
//     struct heap_segment *prev;
// } heap_segment_t;

// // Arena structure for threading
// typedef struct memforge_arena
// {
//     heap_segment_t *heap_segments;
//     struct block_header *free_lists[MEMFORGE_SIZE_CLASS_COUNT];
//     size_t contention_count;
//     struct memforge_arena *next;
// } memforge_arena_t;

// // Statistics structure
// typedef struct memforge_stats
// {
//     size_t total_mapped;
//     size_t total_allocated;
//     size_t total_freed;
//     size_t current_usage;
//     size_t peak_usage;
//     size_t allocation_count;
//     size_t free_count;
//     size_t mmap_count;
//     size_t sbrk_count;
// } memforge_stats_t;

// // Arena configuration structure
// typedef struct memforge_arena_config
// {
//     memforge_arena_strategy_t strategy;
//     size_t max_arenas;
//     size_t contention_threshold;
//     memforge_arena_t *(*custom_mapper)(void); // User-provided mapping function
//     bool enable_thread_cache;
//     size_t thread_cache_size;
// } memforge_arena_config_t;

// // Configuration struct
// typedef struct memforge_config
// {
//     size_t mmap_threshold;
//     size_t page_size;
//     int strategy;
//     int debug_level;
//     bool thread_safe;
//     memforge_arena_config_t arena_config;
// } memforge_config_t;

// #endif