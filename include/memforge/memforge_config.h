/**
 * @file config.h
 * @brief MemForge Compile-time Configuration and Constants
 *
 * This header contains all compile-time configuration options, platform
 * detection macros, and tunable constants for the MemForge memory allocator.
 * These settings control allocator behavior, performance characteristics,
 * and platform-specific adaptations.
 *
 * @note Most settings can be overridden at compile-time using preprocessor
 *       definitions or at runtime via the configuration structure.
 *
 * @author KyloReneo
 * @date 2025
 * @license GPLv3.0
 */

#ifndef MEMFORGE_CONFIG_H
#define MEMFORGE_CONFIG_H

#include <stddef.h>
#include <stdbool.h>

// ============================================================================
// PLATFORM DETECTION
// ============================================================================

/**
 * @def MEMFORGE_PLATFORM_LINUX
 * @brief Defined when compiling for Linux systems
 *
 * Enables Linux-specific optimizations and system calls including:
 * - mmap/munmap for large allocations
 * - sbrk for heap expansion
 * - pthreads for threading
 */
#if defined(__linux__)
#define MEMFORGE_PLATFORM_LINUX 1

/**
 * @def MEMFORGE_PLATFORM_WINDOWS
 * @brief Defined when compiling for Windows systems
 *
 * Enables Windows-specific optimizations and system calls including:
 * - VirtualAlloc/VirtualFree for large allocations
 * - HeapAlloc for heap expansion
 * - Windows threads API for threading
 */
#elif defined(_WIN32)
#define MEMFORGE_PLATFORM_WINDOWS 1
#include <windows.h> // Add Windows headers

/**
 * @def MEMFORGE_PLATFORM_MACOS
 * @brief Defined when compiling for macOS systems
 *
 * Enables macOS-specific optimizations and system calls including:
 * - mmap/munmap for large allocations
 * - System-specific memory management
 */
#elif defined(__APPLE__)
#define MEMFORGE_PLATFORM_MACOS 1
#else
#error "Unsupported platform"
#endif

// ============================================================================
// COMPILE-TIME CONFIGURATION
// ============================================================================

// future work!

// ============================================================================
// ALLOCATOR CONSTANTS
// ============================================================================

/**
 * @def MEMFORGE_ALIGNMENT
 * @brief Memory alignment requirement in bytes
 *
 * Defines the fundamental alignment for all memory allocations. Proper
 * alignment ensures optimal CPU performance and compatibility with
 * SIMD instructions and atomic operations.
 *
 * @note Typical values are 8 (32-bit systems) or 16 (64-bit systems)
 * @note Must be a power of two
 * @see MEMFORGE_ALIGN()
 */
#define MEMFORGE_ALIGNMENT 8

/**
 * @def MEMFORGE_ALIGN(size)
 * @brief Aligns a size to MEMFORGE_ALIGNMENT boundary
 *
 * Rounds up the given size to the next multiple of MEMFORGE_ALIGNMENT.
 * Used to ensure all allocations are properly aligned for the target
 * architecture.
 *
 * @param[in] size Size to align
 * @return size_t Aligned size (multiple of MEMFORGE_ALIGNMENT)
 *
 * @note Efficient bit manipulation implementation
 * @see MEMFORGE_ALIGNMENT
 * @see MEMFORGE_IS_ALIGNED
 */
#define MEMFORGE_ALIGN(size) (((size) + (MEMFORGE_ALIGNMENT - 1)) & ~(MEMFORGE_ALIGNMENT - 1))

/**
 * @def MEMFORGE_IS_ALIGNED(ptr)
 * @brief Checks if a pointer is properly aligned
 *
 * Verifies that a pointer meets the MEMFORGE_ALIGNMENT requirement.
 * Useful for validation and debugging.
 *
 * @param[in] ptr Pointer to check
 * @return bool true if pointer is aligned, false otherwise
 *
 * @see MEMFORGE_ALIGNMENT
 */
#define MEMFORGE_IS_ALIGNED(ptr) (((uintptr_t)(ptr) & (MEMFORGE_ALIGNMENT - 1)) == 0)

/**
 * @def MEMFORGE_MIN_ALLOC_SIZE
 * @brief Minimum allocation size in bytes
 *
 * Defines the smallest possible allocation size. This ensures that even
 * very small allocations have enough space for the block header and
 * maintain proper alignment.
 *
 * @note Must be at least sizeof(block_header_t) + minimal user data
 * @note Affects internal fragmentation for small allocations
 */
#define MEMFORGE_MIN_ALLOC_SIZE 16

/**
 * @def MEMFORGE_DEFAULT_MMAP_THRESHOLD
 * @brief Default threshold for direct mmap allocations
 *
 * Allocations larger than this threshold bypass the heap allocator
 * and use direct memory mapping from the operating system. This
 * reduces fragmentation for large allocations and allows the OS to
 * manage the memory more efficiently.
 *
 * @note Based on glibc's default threshold of 128KB
 * @note Can be overridden at runtime via memforge_set_mmap_threshold()
 * @see memforge_set_mmap_threshold()
 */
#define MEMFORGE_DEFAULT_MMAP_THRESHOLD (128 * 1024) // 128KB

/**
 * @def MEMFORGE_SIZE_CLASS_COUNT
 * @brief Number of size classes for segregated free lists
 *
 * Defines how many different size categories are maintained in the
 * segregated free lists. More classes reduce search time but increase
 * memory overhead for free list management.
 *
 * @note Typical values range from 8-32 depending on use case
 * @see memforge_size_classes
 */
#define MEMFORGE_SIZE_CLASS_COUNT 16

/**
 * @var const size_t memforge_size_classes[MEMFORGE_SIZE_CLASS_COUNT]
 * @brief Array of size class boundaries
 *
 * Defines the size boundaries for each free list category. Allocations
 * are rounded up to the next size class, and each class maintains its
 * own free list for fast allocation.
 *
 * @see MEMFORGE_SIZE_CLASS_COUNT
 * @see get_size_class()
 */
extern const size_t memforge_size_classes[MEMFORGE_SIZE_CLASS_COUNT];

/**
 * @def MEMFORGE_MAGIC_NUMBER
 * @brief Magic number for memory corruption detection
 *
 * A unique value stored in each block header to detect memory
 * corruption, use-after-free, and invalid pointer operations.
 *
 * @note Changed from default patterns to avoid common crash values
 * @see block_header_t::magic
 * @see block_validate()
 */
#define MEMFORGE_MAGIC_NUMBER 0xDEADBEEF

// ============================================================================
// DEBUGGING AND SAFETY FEATURES
// ============================================================================

/**
 * @def DEBUG_LOGGING
 * @brief Enables debug output when defined as 1
 *
 * Controls whether debug messages are printed to stderr. Debug output
 * includes allocation events, internal state changes, and error conditions.
 *
 * @note Automatically set based on MEMFORGE_DEBUG preprocessor definition
 * @note Disabled by default in release builds for performance
 * @see memforge_enable_debug()
 */
#ifdef MEMFORGE_DEBUG
#define DEBUG_LOGGING 1
#else
#define DEBUG_LOGGING 0
#endif

/**
 * @def MEMFORGE_SAFETY_CHECKS
 * @brief Enables runtime safety checks when defined as 1
 *
 * Controls whether additional validation checks are performed during
 * allocation and deallocation. These checks catch common errors but
 * incur a performance penalty.
 *
 * @note Recommended for development and testing
 * @note Can be disabled in production for maximum performance
 * @see block_validate()
 * @see heap_validate()
 */
#define MEMFORGE_SAFETY_CHECKS 1

/**
 * @def MEMFORGE_THREAD_SAFE
 * @brief Enables thread-safe operation when defined as 1
 *
 * Controls whether the allocator uses synchronization primitives to
 * support concurrent access from multiple threads. When enabled,
 * additional memory arenas are created to reduce lock contention.
 *
 * @note Adds overhead for mutex operations in single-threaded scenarios
 * @note Recommended for multi-threaded applications
 * @see memforge_arena_t
 * @see get_current_arena()
 */
#define MEMFORGE_THREAD_SAFE 1

// ============================================================================
// PERFORMANCE TUNING
// ============================================================================

/**
 * @def MEMFORGE_INITIAL_HEAP_SIZE
 * @brief Initial heap size in bytes
 *
 * Defines how much memory is initially requested from the operating system
 * when the allocator starts. Larger values reduce the frequency of heap
 * expansion operations but increase initial memory footprint.
 *
 * @note Balanced for typical application workloads
 * @note 128KB provides good balance for most use cases
 */
#define MEMFORGE_INITIAL_HEAP_SIZE (128 * 1024) // 128KB

/**
 * @def MEMFORGE_MAX_HEAP_SIZE
 * @brief Maximum heap size before exclusive mmap usage
 *
 * When the heap reaches this size, further allocations will prefer
 * direct mmap allocation even for sizes below the mmap threshold.
 * This prevents the heap from growing excessively large.
 *
 * @note Helps control memory fragmentation in long-running processes
 * @note 16MB is conservative; can be increased for memory-intensive apps
 */
#define MEMFORGE_MAX_HEAP_SIZE (16 * 1024 * 1024) // 16MB

/**
 * @def MEMFORGE_DEFAULT_ARENA_COUNT
 * @brief Default number of memory arenas for multi-threaded operation
 *
 * Defines how many separate memory pools are created when thread-safe
 * mode is enabled. Each arena can be used by different threads to
 * reduce lock contention.
 *
 * @note Should be proportional to expected thread count
 * @note 4 arenas works well for typical 4-core systems
 * @see memforge_arena_t
 * @see memforge_config_t::arena_count
 */
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