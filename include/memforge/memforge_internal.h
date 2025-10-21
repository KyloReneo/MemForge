/**
 * @file internal.h
 * @brief MemForge internal data structures and private API
 *
 * This header contains internal data structures, global state declarations,
 * and private function prototypes used by the MemForge memory allocator.
 * These are not part of the public API and should not be used directly
 * by application code.
 *
 * @warning This header is for internal use only. Use the public API in
 *          memforge.h for normal allocator operations.
 *
 * @author KyloReneo
 * @date 2025
 * @license GPLv3.0
 */

#ifndef MEMFORGE_INTERNAL_H
#define MEMFORGE_INTERNAL_H

#include "memforge_config.h"
#include <pthread.h>
#include <stddef.h>
#include <stdbool.h>

// ============================================================================
// INTERNAL DATA STRUCTURES
// ============================================================================

/**
 * @brief Block header structure stored before each allocation
 *
 * This structure precedes every memory allocation in the heap and contains
 * metadata needed for memory management. The header is invisible to users
 * and is used internally for block tracking, free list management, and
 * corruption detection.
 *
 * @struct block_header
 *
 * @var block_header::size
 * Size of the user data area in bytes (does not include header size)
 *
 * @var block_header::next
 * Pointer to the next block in free list or heap chain
 *
 * @var block_header::prev
 * Pointer to the previous block (enables bidirectional traversal for coalescing)
 *
 * @var block_header::is_free
 * Flag indicating whether the block is currently allocated or free
 *
 * @var block_header::is_mapped
 * Flag indicating whether block was allocated via mmap (true) or heap (false)
 *
 * @var block_header::magic
 * Magic number for memory corruption detection and validation
 *
 * @note The actual user data starts immediately after this header
 * @see BLOCK_HEADER_SIZE
 */
typedef struct block_header
{
    size_t size;               /**< Size of user data area in bytes */
    struct block_header *next; /**< Next block in free list or heap */
    struct block_header *prev; /**< Previous block (for coalescing) */
    bool is_free;              /**< Whether block is allocated or free */
    bool is_mapped;            /**< Whether block is mmap'd (not from heap) */
    unsigned int magic;        /**< Magic number for corruption detection */
} block_header_t;

/**
 * @def BLOCK_HEADER_SIZE
 * @brief Size of block header with proper memory alignment
 *
 * Calculates the actual size of block_header_t including alignment padding.
 * This ensures all block headers are properly aligned for performance and
 * architecture requirements.
 *
 * @see MEMFORGE_ALIGN
 * @see block_header_t
 */
#define BLOCK_HEADER_SIZE MEMFORGE_ALIGN(sizeof(block_header_t))

/**
 * @brief Heap segment tracking structure
 *
 * Tracks contiguous regions of memory obtained from the operating system.
 * Each segment represents one chunk of memory that has been added to the
 * heap, either via sbrk() or mmap().
 *
 * @struct heap_segment
 *
 * @var heap_segment::base
 * Base address of the memory segment
 *
 * @var heap_segment::size
 * Total size of the segment in bytes
 *
 * @var heap_segment::next
 * Pointer to the next segment in the linked list
 *
 * @note Segments are managed as a linked list for easy traversal
 */
typedef struct heap_segment
{
    void *base;                /**< Base address of the memory segment */
    size_t size;               /**< Total size of the segment in bytes */
    struct heap_segment *next; /**< Next segment in linked list */
} heap_segment_t;

/**
 * @brief Memory arena for thread-local allocation
 *
 * Arenas provide isolated memory pools that can be used by different threads
 * to reduce contention. Each arena maintains its own free lists and heap
 * segments, enabling parallel allocation operations.
 *
 * @struct memforge_arena
 *
 * @var memforge_arena::lock
 * Mutex for thread-safe access to this arena
 *
 * @var memforge_arena::free_lists
 * Array of segregated free lists organized by size class
 *
 * @var memforge_arena::heap_segments
 * Linked list of heap segments owned by this arena
 *
 * @var memforge_arena::allocated
 * Total bytes allocated through this arena (statistics)
 *
 * @var memforge_arena::freed
 * Total bytes freed through this arena (statistics)
 *
 * @note In single-threaded mode, only the main arena is used
 * @see MEMFORGE_SIZE_CLASS_COUNT
 */
typedef struct memforge_arena
{
    pthread_mutex_t lock;                                  /**< Arena-specific lock */
    block_header_t *free_lists[MEMFORGE_SIZE_CLASS_COUNT]; /**< Segregated free lists */
    heap_segment_t *heap_segments;                         /**< Heap segments owned by this arena */
    size_t allocated;                                      /**< Bytes allocated in this arena */
    size_t freed;                                          /**< Bytes freed in this arena */
} memforge_arena_t;

// ============================================================================
// GLOBAL STATE DECLARATIONS
// ============================================================================

/**
 * @var memforge_config_t memforge_config
 * @brief Global allocator configuration settings
 *
 * Stores runtime configuration parameters including page size, allocation
 * strategy, thread safety settings, and debugging options.
 *
 * @see memforge_config_t
 */
extern memforge_config_t memforge_config;

/**
 * @var memforge_stats_t memforge_stats
 * @brief Global allocation statistics tracker
 *
 * Maintains runtime statistics for monitoring allocator performance
 * including allocation counts, memory usage, and operational metrics.
 *
 * @see memforge_stats_t
 */
extern memforge_stats_t memforge_stats;

/**
 * @var memforge_arena_t* memforge_main_arena
 * @brief Primary memory arena for single-threaded operations
 *
 * The main arena handles allocations when thread safety is disabled or
 * serves as fallback when thread-specific arenas are unavailable.
 * Always exists when the allocator is initialized.
 */
extern memforge_arena_t *memforge_main_arena;

/**
 * @var memforge_arena_t** memforge_arenas
 * @brief Array of memory arenas for multi-threaded operation
 *
 * In thread-safe mode, multiple arenas are created to reduce contention.
 * Each thread can be assigned to a specific arena to enable parallel
 * allocation operations without locking.
 *
 * @note Array size is determined by memforge_config.arena_count
 */
extern memforge_arena_t **memforge_arenas;

/**
 * @var bool memforge_initialized
 * @brief Initialization state flag
 *
 * Indicates whether the allocator has been successfully initialized.
 * Prevents double-initialization and ensures proper cleanup sequence.
 */
extern bool memforge_initialized;

/**
 * @var size_t memforge_size_classes[MEMFORGE_SIZE_CLASS_COUNT]
 * @brief Size classes for segregated free lists
 *
 * Array of size boundaries used to categorize allocations into different
 * free lists. This enables faster allocation by reducing search space.
 *
 * @see MEMFORGE_SIZE_CLASS_COUNT
 * @see get_size_class()
 */
extern size_t memforge_size_classes[MEMFORGE_SIZE_CLASS_COUNT];

// ============================================================================
// INTERNAL FUNCTION DECLARATIONS
// ============================================================================

// Initialization functions
/**
 * @brief Initializes default configuration values
 *
 * Sets up default allocator configuration based on system properties
 * and compile-time defaults. Called automatically during allocator
 * initialization.
 *
 * @return int 0 on success, -1 on failure
 *
 * @retval 0 Configuration successfully initialized
 * @retval -1 System property detection failed
 *
 * @note Called by memforge_init() during allocator startup
 * @see memforge_init()
 */
int memforge_init_default_config(void);

/**
 * @brief Initializes memory arenas for allocation
 *
 * Creates and initializes the arena system used for memory allocation.
 * Allocates the arena array and creates individual arenas based on
 * configuration settings.
 *
 * @return int 0 on success, -1 on failure
 *
 * @retval 0 All arenas successfully created
 * @retval -1 Arena allocation failed
 *
 * @note In thread-safe mode, creates multiple arenas for reduced contention
 * @see memforge_arena_t
 */
int memforge_init_arenas(void);

// System memory management functions
/**
 * @brief Allocates memory directly from operating system via mmap
 *
 * Requests memory directly from the OS using memory mapping. Used for
 * large allocations that exceed the mmap threshold or for internal
 * allocator structures.
 *
 * @param[in] size Number of bytes to allocate
 * @return void* Pointer to allocated memory, or NULL on failure
 *
 * @note Memory allocated with this function must be freed with system_free_mmap()
 * @note Platform-specific implementation (mmap on Unix, VirtualAlloc on Windows)
 * @see system_free_mmap()
 */
void *system_alloc_mmap(size_t size);

/**
 * @brief Allocates memory via sbrk for heap expansion
 *
 * Requests additional memory from the OS to expand the heap. Used for
 * small to medium allocations that are managed within the allocator's
 * heap structures.
 *
 * @param[in] size Number of bytes to allocate
 * @return void* Pointer to allocated memory, or NULL on failure
 *
 * @note Primarily used on Unix-like systems
 * @note Windows implementation may use HeapAlloc as alternative
 */
void *system_alloc_sbrk(size_t size);

/**
 * @brief Frees memory allocated with system_alloc_mmap
 *
 * Returns memory previously allocated via system_alloc_mmap back to
 * the operating system.
 *
 * @param[in] ptr Pointer to memory to free
 * @param[in] size Size of the memory region (must match allocation size)
 *
 * @note Must only be used with memory from system_alloc_mmap()
 * @note Platform-specific implementation (munmap on Unix, VirtualFree on Windows)
 * @see system_alloc_mmap()
 */
void system_free_mmap(void *ptr, size_t size);

// Heap management functions
/**
 * @brief Creates a new heap segment tracker
 *
 * Allocates and initializes a heap_segment_t structure to track a
 * region of memory obtained from the operating system.
 *
 * @param[in] base Base address of the memory segment
 * @param[in] size Size of the memory segment in bytes
 * @return heap_segment_t* Pointer to new segment, or NULL on failure
 *
 * @see heap_segment_destroy()
 */
heap_segment_t *heap_segment_create(void *base, size_t size);

/**
 * @brief Destroys a heap segment and releases its resources
 *
 * Frees a heap_segment_t structure and any associated resources.
 *
 * @param[in] segment Segment to destroy
 *
 * @see heap_segment_create()
 */
void heap_segment_destroy(heap_segment_t *segment);

// Arena management functions
/**
 * @brief Gets the current thread's assigned arena
 *
 * Returns the arena assigned to the current thread for allocation.
 * Uses thread-local storage to avoid locking on every allocation.
 *
 * @return memforge_arena_t* Pointer to current thread's arena
 *
 * @note In non-threaded mode, always returns main arena
 * @note Implements thread-local storage for performance
 */
memforge_arena_t *get_current_arena(void);

/**
 * @brief Creates a new memory arena
 *
 * Allocates and initializes a new memory arena with empty free lists
 * and proper synchronization primitives.
 *
 * @return memforge_arena_t* Pointer to new arena, or NULL on failure
 *
 * @note Initializes mutex if thread-safe mode is enabled
 * @see arena_destroy()
 */
memforge_arena_t *arena_create(void);

/**
 * @brief Destroys a memory arena and its resources
 *
 * Safely destroys an arena, freeing all associated resources including
 * heap segments and synchronization primitives.
 *
 * @param[in] arena Arena to destroy
 *
 * @note Properly cleans up all arena resources
 * @see arena_create()
 */
void arena_destroy(memforge_arena_t *arena);

// Utility functions
/**
 * @brief Debug logging function
 *
 * Outputs debug messages to stderr when debug mode is enabled.
 * No-op when memforge_config.debug_enabled is false.
 *
 * @param[in] format printf-style format string
 * @param[in] ... Variable arguments for format string
 *
 * @note Only active when MEMFORGE_DEBUG is defined or debug is enabled
 * @note Thread-safe implementation
 */
void debug_log(const char *format, ...);

/**
 * @brief Checks if a number is a power of two
 *
 * Determines whether the given number is an exact power of two.
 * Useful for alignment calculations and size validation.
 *
 * @param[in] x Number to check
 * @return bool true if x is power of two, false otherwise
 *
 * @note Returns false for zero
 */
bool is_power_of_two(size_t x);

/**
 * @brief Rounds up to the next power of two
 *
 * Calculates the smallest power of two that is greater than or equal
 * to the given number.
 *
 * @param[in] x Input number
 * @return size_t Next power of two >= x
 *
 * @note Returns 1 for x = 0
 * @note Efficient bit manipulation implementation
 */
size_t next_power_of_two(size_t x);

// Memory validation functions
/**
 * @brief Validates a block's integrity
 *
 * Performs sanity checks on a block header to detect memory corruption
 * and structural issues. Checks magic number, size validity, and
 * link consistency.
 *
 * @param[in] block Block header to validate
 * @return bool true if block is valid, false if corrupted
 *
 * @note Essential for security and stability
 * @see MEMFORGE_MAGIC_NUMBER
 */
bool block_validate(block_header_t *block);

/**
 * @brief Validates entire heap integrity
 *
 * Performs comprehensive validation of all heap structures including
 * all blocks, free lists, and arena consistency.
 *
 * @return bool true if heap is valid, false if corruption detected
 *
 * @note Expensive operation - use for debugging only
 * @note Thread-safe implementation
 */
bool heap_validate(void);

// Platform-specific threading functions
/**
 * @brief Gets platform-specific thread identifier
 *
 * Returns a unique identifier for the current thread using
 * platform-appropriate APIs.
 *
 * @return int Thread identifier (system-dependent)
 *
 * @note Uses gettid() on Linux, GetCurrentThreadId() on Windows
 * @note Used for arena assignment and debugging
 */
int thread_get_id(void);

#endif

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