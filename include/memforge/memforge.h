/**
 * @file memforge.h
 * @brief MemForge Memory Allocator Public API
 *
 * High-performance custom memory allocator with advanced features including:
 * - Multiple allocation strategies (First-fit, Best-fit, Hybrid)
 * - Thread-safe operation with multiple memory arenas
 * - Segregated free lists for fast allocation
 * - Memory coalescing to reduce fragmentation
 * - Comprehensive statistics and debugging
 * - Customizable configuration
 *
 * @note This allocator is designed as a drop-in replacement for standard
 *       malloc/free with enhanced performance and diagnostic capabilities.
 *
 * @author KyloReneo
 * @date 2025
 * @license GPLv3.0
 */

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

    /**
     * @brief Memory allocation strategies
     *
     * Defines different algorithms for selecting free blocks during allocation.
     * Each strategy offers different trade-offs between speed, fragmentation,
     * and memory utilization.
     *
     * @see memforge_set_strategy()
     */
    typedef enum allocation_strategies
    {
        MEMFORGE_STRATEGY_FIRST_FIT, /**< Fastest   - selects first suitable block found */
        MEMFORGE_STRATEGY_BEST_FIT,  /**< Efficient - selects smallest adequate block to reduce fragmentation */
        MEMFORGE_STRATEGY_HYBRID     /**< Balanced  - combines speed of first-fit with efficiency of best-fit */
    } memforge_strategy_t;

    /**
     * @brief Arena mapping strategies for multi-threaded operation
     *
     * Defines how threads are assigned to memory arenas in multi-threaded
     * environments. Different strategies optimize for different workloads.
     *
     * @note Only effective when thread_safe is enabled in configuration
     */
    typedef enum arena_strategies
    {
        MEMFORGE_ARENA_DEFAULT = 0,      /**< Auto-select based on system heuristics */
        MEMFORGE_ARENA_PER_THREAD,       /**< One arena per thread (best performance, higher memory) */
        MEMFORGE_ARENA_ROUND_ROBIN,      /**< Round-robin distribution (good balance) */
        MEMFORGE_ARENA_CONTENTION_AWARE, /**< Smart allocation based on lock contention */
        MEMFORGE_ARENA_SINGLE,           /**< Single arena (minimal memory usage) */
        MEMFORGE_ARENA_CUSTOM            /**< User-provided mapping function */
    } memforge_arena_strategy_t;

    /**
     * @brief Allocator configuration structure
     *
     * Contains runtime configuration parameters that control allocator behavior.
     * Passed to memforge_init() to customize operation.
     *
     * @struct config
     *
     * @var config::page_size
     * System page size in bytes (auto-detected if 0)
     *
     * @var config::mmap_threshold
     * Size threshold for using direct mmap allocations (bytes)
     *
     * @var config::strategy
     * Allocation strategy for block selection
     *
     * @var config::thread_safe
     * Enable thread-safe operation with multiple arenas
     *
     * @var config::debug_enabled
     * Enable debug output and additional checks
     *
     * @var config::arena_count
     * Number of memory arenas for multi-threaded operation
     *
     * @see memforge_init()
     * @see memforge_config_t
     */
    typedef struct config
    {
        size_t page_size;             /**< System page size in bytes */
        size_t mmap_threshold;        /**< MMAP threshold in bytes */
        memforge_strategy_t strategy; /**< Allocation strategy */
        bool thread_safe;             /**< Thread safety enabled */
        bool debug_enabled;           /**< Debug output enabled */
        size_t arena_count;           /**< Number of memory arenas */
    } memforge_config_t;

    /**
     * @brief Allocation statistics structure
     *
     * Contains runtime statistics about allocator performance and
     * memory usage patterns. Useful for monitoring, debugging, and
     * performance analysis.
     *
     * @struct stats
     *
     * @var stats::total_allocated
     * Total bytes allocated over lifetime
     *
     * @var stats::total_freed
     * Total bytes freed over lifetime
     *
     * @var stats::current_usage
     * Currently allocated bytes (total_allocated - total_freed)
     *
     * @var stats::peak_usage
     * Maximum concurrent memory usage observed
     *
     * @var stats::allocation_count
     * Total number of allocation requests
     *
     * @var stats::free_count
     * Total number of deallocation requests
     *
     * @var stats::mmap_count
     * Number of direct mmap allocations
     *
     * @var stats::heap_expansions
     * Number of heap expansion operations
     *
     * @see memforge_get_stats()
     * @see memforge_stats_t
     */
    typedef struct stats
    {
        size_t total_allocated;  /**< Total bytes allocated */
        size_t total_freed;      /**< Total bytes freed */
        size_t current_usage;    /**< Current memory usage */
        size_t peak_usage;       /**< Peak memory usage */
        size_t allocation_count; /**< Total allocation calls */
        size_t free_count;       /**< Total free calls */
        size_t mmap_count;       /**< Direct mmap allocations */
        size_t heap_expansions;  /**< Heap expansion operations */
    } memforge_stats_t;

    // ============================================================================
    // PUBLIC API FUNCTIONS
    // ============================================================================

    // Core allocation functions (replace standard library)

    /**
     * @brief Allocates size bytes of uninitialized memory
     *
     * The primary memory allocation function. Allocates a block of memory
     * of at least `size` bytes. The block is uninitialized - contents are
     * indeterminate.
     *
     * @param[in] size Number of bytes to allocate
     * @return void* Pointer to allocated memory, or NULL on failure
     *
     * @retval !NULL Success - pointer to allocated memory
     * @retval NULL Failure - insufficient memory or invalid size
     *
     * @note For zero-sized allocations, returns a unique pointer (not NULL)
     * @note Thread-safe when configured with thread_safe = true
     *
     * @see memforge_free()
     * @see memforge_calloc()
     * @see memforge_realloc()
     *
     * @par Example:
     * @code
     * int *array = (int*)memforge_malloc(100 * sizeof(int));
     * if (array == NULL) {
     *     // Handle allocation failure
     * }
     * @endcode
     */
    void *memforge_malloc(size_t size);

    /**
     * @brief Releases memory previously allocated
     *
     * Deallocates memory previously allocated by memforge_malloc(),
     * memforge_calloc(), or memforge_realloc(). The pointer must have
     * been returned by a previous allocation call.
     *
     * @param[in] ptr Pointer to memory block to free
     *
     * @note free(NULL) is allowed and does nothing
     * @note Double-free protection is implemented
     * @note Thread-safe when configured with thread_safe = true
     *
     * @see memforge_malloc()
     * @see memforge_calloc()
     * @see memforge_realloc()
     *
     * @par Example:
     * @code
     * int *array = (int*)memforge_malloc(100 * sizeof(int));
     * // Use array...
     * memforge_free(array);
     * @endcode
     */
    void memforge_free(void *ptr);

    /**
     * @brief Allocates memory for an array and initializes to zero
     *
     * Allocates memory for an array of `n` elements of `size` bytes each
     * and initializes all bytes to zero.
     *
     * @param[in] n Number of elements to allocate
     * @param[in] size Size of each element in bytes
     * @return void* Pointer to allocated memory, or NULL on failure
     *
     * @retval !NULL Success - pointer to zero-initialized memory
     * @retval NULL Failure - insufficient memory or size overflow
     *
     * @note Equivalent to memforge_malloc(n * size) followed by memset(0)
     * @note Protected against integer overflow in n * size calculation
     * @note Thread-safe when configured with thread_safe = true
     *
     * @see memforge_malloc()
     * @see memforge_free()
     *
     * @par Example:
     * @code
     * int *array = (int*)memforge_calloc(100, sizeof(int));
     * // array is now zero-initialized
     * @endcode
     */
    void *memforge_calloc(size_t n, size_t size);

    /**
     * @brief Resizes a previously allocated memory block
     *
     * Changes the size of the memory block pointed to by `ptr` to `size` bytes.
     * The contents will be unchanged up to the minimum of the old and new sizes.
     *
     * @param[in] ptr Pointer to previously allocated memory block
     * @param[in] size New size in bytes
     * @return void* Pointer to resized memory block, or NULL on failure
     *
     * @retval !NULL Success - pointer to resized memory (may be different from ptr)
     * @retval NULL Failure - insufficient memory
     *
     * @note If ptr is NULL, equivalent to memforge_malloc(size)
     * @note If size is 0 and ptr is not NULL, equivalent to memforge_free(ptr)
     * @note May move the block to a new location if resizing in-place is not possible
     * @note Thread-safe when configured with thread_safe = true
     *
     * @see memforge_malloc()
     * @see memforge_free()
     *
     * @par Example:
     * @code
     * int *array = (int*)memforge_malloc(50 * sizeof(int));
     * array = (int*)memforge_realloc(array, 100 * sizeof(int));
     * // array now has space for 100 integers
     * @endcode
     */
    void *memforge_realloc(void *ptr, size_t size);

    // Allocator lifecycle management

    /**
     * @brief Initializes the MemForge allocator
     *
     * Initializes the memory allocator with the specified configuration.
     * Must be called before using allocation functions (though lazy
     * initialization is provided as fallback).
     *
     * @param[in] config Configuration structure, or NULL for defaults
     * @return int 0 on success, -1 on failure
     *
     * @retval 0 Success - allocator ready for use
     * @retval -1 Failure - initialization failed
     *
     * @note If NULL is passed, uses sensible defaults auto-detected from system
     * @note Idempotent - safe to call multiple times
     * @note Thread-safe during initialization
     *
     * @see memforge_cleanup()
     * @see memforge_config_t
     *
     * @par Example:
     * @code
     * memforge_config_t config = {
     *     .mmap_threshold = 128 * 1024,
     *     .strategy = MEMFORGE_STRATEGY_BEST_FIT,
     *     .thread_safe = true
     * };
     * if (memforge_init(&config) != 0) {
     *     // Handle initialization failure
     * }
     * @endcode
     */
    int memforge_init(const memforge_config_t *config);

    /**
     * @brief Cleans up allocator resources
     *
     * Releases all resources owned by the allocator and resets internal state.
     * After cleanup, the allocator is uninitialized and must be re-initialized
     * before further use.
     *
     * @warning Any outstanding allocations become invalid after cleanup
     * @note Idempotent - safe to call multiple times
     * @note Not thread-safe with concurrent allocations
     *
     * @see memforge_init()
     * @see memforge_reset()
     *
     * @par Example:
     * @code
     * memforge_init(NULL);
     * // Use allocator...
     * memforge_cleanup(); // Release all resources
     * @endcode
     */
    void memforge_cleanup(void);

    /**
     * @brief Resets allocator to initial state
     *
     * Performs cleanup followed by re-initialization with defaults.
     * Primarily useful for testing and benchmarking.
     *
     * @note Resets all statistics to zero
     * @note Not recommended for production use
     * @note Not thread-safe with concurrent allocations
     *
     * @see memforge_init()
     * @see memforge_cleanup()
     */
    void memforge_reset(void);

    // Memory alignment utilities

    /**
     * @brief Allocates aligned memory (simple interface)
     *
     * Allocates `size` bytes of memory with the specified alignment.
     *
     * @param[in] alignment Alignment requirement (must be power of two)
     * @param[in] size Number of bytes to allocate
     * @return void* Aligned memory pointer, or NULL on failure
     *
     * @note Alignment must be a power of two
     * @note For POSIX-compliant interface, use memforge_posix_memalign()
     *
     * @see memforge_aligned_alloc()
     * @see memforge_posix_memalign()
     */
    void *align(size_t alignment, size_t size);

    /**
     * @brief POSIX-compliant aligned memory allocation
     *
     * Allocates `size` bytes of memory with alignment `alignment`, putting
     * the allocated memory in `*memptr`. Follows POSIX memalign() semantics.
     *
     * @param[out] memptr Pointer to store the allocated memory address
     * @param[in] alignment Alignment requirement (must be power of two and multiple of sizeof(void*))
     * @param[in] size Number of bytes to allocate
     * @return int 0 on success, error code on failure
     *
     * @retval 0 Success - memory allocated and stored in *memptr
     * @retval EINVAL Invalid alignment (not power of two or too small)
     * @retval ENOMEM Insufficient memory
     *
     * @note Alignment must be power of two and multiple of sizeof(void*)
     *
     * @see memforge_aligned_alloc()
     */
    int memgorge_posix_align(void **memptr, size_t alignment, size_t size);

    /**
     * @brief C11-compliant aligned memory allocation
     *
     * Allocates `size` bytes of memory with alignment `alignment`.
     * Follows C11 aligned_alloc() semantics.
     *
     * @param[in] alignment Alignment requirement (must be power of two)
     * @param[in] size Number of bytes to allocate
     * @return void* Aligned memory pointer, or NULL on failure
     *
     * @note Size must be multiple of alignment for C11 compliance
     * @note For more flexible alignment, use memforge_posix_memalign()
     *
     * @see memforge_posix_memalign()
     */
    void *memforge_aligned_alloc(size_t alignment, size_t size);

    /**
     * @brief Queries alignment of allocated memory
     *
     * Returns the alignment of the memory block pointed to by `ptr`.
     *
     * @param[in] ptr Pointer to allocated memory
     * @return size_t Alignment of the memory block in bytes
     *
     * @note Returns 0 if ptr is NULL or not a MemForge allocation
     *
     * @see memforge_aligned_alloc()
     * @see memforge_posix_memalign()
     */
    size_t memforge_get_alignment(void *ptr);

    // Configuration and statistics

    /**
     * @brief Retrieves current allocator statistics
     *
     * Copies the current allocation statistics into the provided structure.
     *
     * @param[out] stats Pointer to statistics structure to fill
     *
     * @note Thread-safe operation
     *
     * @see memforge_stats_t
     *
     * @par Example:
     * @code
     * memforge_stats_t stats;
     * memforge_get_stats(&stats);
     * printf("Current memory usage: %zu bytes\n", stats.current_usage);
     * @endcode
     */
    void memforge_get_stats(memforge_stats_t *stats);

    /**
     * @brief Sets the allocation strategy
     *
     * Changes the allocation strategy used for block selection.
     *
     * @param[in] strategy New allocation strategy to use
     *
     * @note Affects future allocations only
     * @note Thread-safe operation
     *
     * @see memforge_strategy_t
     */
    void memforge_set_strategy(memforge_strategy_t strategy);

    /**
     * @brief Sets the mmap allocation threshold
     *
     * Configures the size threshold for using direct mmap allocations
     * instead of heap allocations.
     *
     * @param[in] threshold Size threshold in bytes
     *
     * @note Allocations >= threshold will use mmap directly
     * @note Default is typically 64KB
     */
    void memforge_set_mmap_threshold(size_t threshold);

    // Debugging and diagnostics

    /**
     * @brief Dumps heap information to stdout
     *
     * Prints detailed information about heap state, free lists, and
     * memory usage to stdout. Useful for debugging and analysis.
     *
     * @note Only available when debug_enabled is true
     * @note Not thread-safe during heap traversal
     */
    void memforge_dump_heap(void);

    /**
     * @brief Validates heap integrity
     *
     * Performs comprehensive validation of heap structures and consistency.
     *
     * @return bool true if heap is valid, false if corruption detected
     *
     * @note Expensive operation - use for debugging only
     * @note Thread-safe operation
     */
    bool memforge_validate_heap(void);

    /**
     * @brief Enables or disables debug output
     *
     * Controls whether debug information is printed to stderr.
     *
     * @param[in] enable true to enable, false to disable
     *
     * @note Debug output includes allocation events and internal operations
     */
    void memforge_enable_debug(bool enable);

    // Advanced features

    /**
     * @brief Compacts memory to reduce fragmentation
     *
     * Attempts to reduce memory fragmentation by consolidating free blocks
     * and reorganizing memory layout.
     *
     * @note Experimental feature - may not fully compact in all cases
     * @note Can be expensive for large heaps
     */
    void memforge_compact(void);

    /**
     * @brief Returns usable size of allocated memory
     *
     * Returns the actual usable size of the memory block pointed to by `ptr`.
     * This may be larger than the originally requested size due to alignment
     * and allocation granularity.
     *
     * @param[in] ptr Pointer to allocated memory
     * @return size_t Usable size in bytes, or 0 if ptr is invalid
     *
     * @note Similar to malloc_usable_size() in other allocators
     */
    size_t memforge_usable_size(void *ptr);

    // Utility functions (compatibility with standard malloc interfaces)

    /**
     * @brief Returns usable size of allocated memory (compatibility)
     *
     * Compatibility function equivalent to memforge_usable_size().
     *
     * @param[in] ptr Pointer to allocated memory
     * @return size_t Usable size in bytes
     *
     * @see memforge_usable_size()
     */
    size_t memforge_malloc_usable_size(void *ptr);

    /**
     * @brief Prints allocation statistics to stdout (compatibility)
     *
     * Prints brief allocation statistics to stdout in a format similar
     * to other malloc implementations.
     */
    void memforge_malloc_stats(void);

    /**
     * @brief Releases free memory to the operating system (compatibility)
     *
     * Attempts to release free memory at the end of the heap back to
     * the operating system.
     *
     * @param[in] pad Minimum amount to keep allocated
     * @return int 1 if memory was released, 0 otherwise
     */
    int memforge_malloc_trim(size_t pad);

    /**
     * @brief Exports allocator information in XML format (compatibility)
     *
     * Writes detailed allocator information in XML format to the specified
     * stream. Compatible with malloc_info() interface.
     *
     * @param[in] options Output options (currently unused)
     * @param[in] stream Output stream (e.g., stdout, file)
     */
    void memforge_malloc_info(int options, FILE *stream);

#ifdef __cplusplus
}
#endif

#endif