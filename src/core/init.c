/**
 * @file init.c
 * @brief MemForge allocator initialization and lifecycle management
 *
 * This module handles the initialization, configuration, and cleanup of the
 * MemForge memory allocator. It sets up global state, detects system properties,
 * and manages memory arenas for efficient multi-threaded allocation.
 *
 * @author KyloReneo
 * @date 2025
 * @license GPLv3.0
 */

#include "../../include/memforge/memforge_internal.h"
#include "../../include/memforge/memforge.h"

// ============================================================================
// GLOBAL STATE DEFINITIONS
// ============================================================================

/**
 * @var memforge_config_t memforge_config
 * @brief Global allocator configuration settings
 *
 * Stores runtime configuration parameters including:
 * - Page size detection
 * - Allocation strategy
 * - Thread safety settings
 * - Debugging options
 */
memforge_config_t memforge_config = {0};

/**
 * @var memforge_stats_t memforge_stats
 * @brief Global allocation statistics tracker
 *
 * Maintains runtime statistics for monitoring allocator performance:
 * - Total bytes allocated/freed
 * - Current and peak memory usage
 * - Allocation/deallocation counts
 */
memforge_stats_t memforge_stats = {0};

/**
 * @var memforge_arena_t* memforge_main_arena
 * @brief Primary memory arena for single-threaded operations
 *
 * The main arena handles allocations when thread safety is disabled or
 * serves as fallback when thread-specific arenas are unavailable.
 */
memforge_arena_t *memforge_main_arena = NULL;

/**
 * @var memforge_arena_t** memforge_arenas
 * @brief Array of memory arenas for multi-threaded operation
 *
 * In thread-safe mode, multiple arenas are created to reduce contention.
 * Each thread is assigned to a specific arena to enable parallel allocation.
 */
memforge_arena_t **memforge_arenas = NULL;

/**
 * @var bool memforge_initialized
 * @brief Initialization state flag
 *
 * Prevents double-initialization and ensures proper cleanup sequence.
 * Guards against using uninitialized allocator state.
 */
bool memforge_initialized = false;

/**
 * @var size_t memforge_size_classes[MEMFORGE_SIZE_CLASS_COUNT]
 * @brief Size classes for segregated free lists
 *
 * Implements power-of-two size classes with spacing to optimize allocation speed:
 * - Small  sizes (16-2048 bytes): Fine-grained for common allocations
 * - Medium sizes (4K-64K)       : Balanced for typical objects
 * - Large  sizes (128K-512K)    : Coarse-grained for big allocations
 *
 * Segregated free lists reduce search time by only scanning appropriate size buckets.
 */
size_t memforge_size_classes[MEMFORGE_SIZE_CLASS_COUNT] = {
    16, 32, 64, 128, 256, 512, 1024, 2048,
    4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288};

// ============================================================================
// INITIALIZATION FUNCTIONS
// ============================================================================

/**
 * @brief Initializes the MemForge allocator with default or provided configuration
 *
 * This is the primary initialization function that sets up the entire allocator
 * ecosystem. It performs system detection, configuration setup, and arena
 * creation in a safe, atomic manner.
 *
 * @param[in] config User-provided configuration structure (optional)
 *                   If NULL, default configuration values are used
 *
 * @return int 0 on success, -1 on failure
 *
 * @retval 0 Allocator successfully initialized and ready for use
 * @retval -1 Initialization failed (system resource allocation error)
 *
 * @note This function is thread-safe and idempotent. Subsequent calls after
 *       successful initialization will return immediately with success.
 *
 * @warning Do not call allocation functions (malloc/free) before successful
 *          initialization. The allocator uses lazy initialization on first
 *          allocation call as a fallback.
 *
 * @par Example:
 * @code
 * // Initialize with default settings
 * if (memforge_init(NULL) != 0) {
 *     fprintf(stderr, "Failed to initialize allocator\n");
 *     exit(1);
 * }
 *
 * // Initialize with custom configuration
 * memforge_config_t config = {
 *     .mmap_threshold = 128 * 1024,
 *     .strategy = MEMFORGE_STRATEGY_BEST_FIT
 * };
 * memforge_init(&config);
 * @endcode
 *
 * @see memforge_config_t
 * @see memforge_cleanup()
 */
int memforge_init(const memforge_config_t *config)
{
    if (memforge_initialized)
    {
        return 0; // Already initialized
    }

    // Initialize default configuration
    if (memforge_init_default_config() != 0)
    {
        return -1;
    }

    // Override with user configuration if provided
    if (config != NULL)
    {
        memforge_config = *config;
    }

    // Initialize arenas for multi-threaded operation
    if (memforge_init_arenas() != 0)
    {
        return -1;
    }

    // Initialize size classes with proper memory alignment
    for (size_t i = 0; i < MEMFORGE_SIZE_CLASS_COUNT; i++)
    {
        memforge_size_classes[i] = MEMFORGE_ALIGN(memforge_size_classes[i]);
    }

    memforge_initialized = true;
    debug_log("MemForge initialized successfully");
    return 0;
}

/**
 * @brief Sets up default configuration values based on system properties
 *
 * Performs platform detection and sets sensible defaults for the allocator:
 * - Detects system page size via platform-specific APIs
 * - Sets allocation strategy to hybrid (balanced performance/fragmentation)
 * - Enables thread safety by default
 * - Configures mmap threshold for large allocations
 *
 * @return int 0 on success, -1 on failure
 *
 * @retval 0 Configuration successfully applied
 * @retval -1 System property detection failed
 *
 * @note This function is called automatically by memforge_init() and should
 *       not be called directly by users.
 *
 * @internal
 * Platform detection matrix:
 * | Platform  | Page Size Detection Method      |
 * |-----------|---------------------------------|
 * | Windows   | GetSystemInfo()->dwPageSize     |
 * | Linux/Unix| sysconf(_SC_PAGESIZE)          |
 * | Fallback  | 4096 bytes (common default)    |
 */
int memforge_init_default_config(void)
{
    // Platform-specific page size detection
#ifdef _WIN32
    SYSTEM_INFO system_info;
    GetSystemInfo(&system_info);
    memforge_config.page_size = system_info.dwPageSize;
#else
    // Linux/Unix/MacOS
    memforge_config.page_size = sysconf(_SC_PAGESIZE);
#endif

    // Fallback if page size detection fails
    if (memforge_config.page_size == 0)
    {
        memforge_config.page_size = 4096; // Common default page size for most linux and unix based kernels
    }

    memforge_config.mmap_threshold = MEMFORGE_DEFAULT_MMAP_THRESHOLD;
    memforge_config.strategy = MEMFORGE_STRATEGY_HYBRID;
    memforge_config.thread_safe = MEMFORGE_THREAD_SAFE;
    memforge_config.debug_enabled = DEBUG_LOGGING;
    memforge_config.arena_count = MEMFORGE_DEFAULT_ARENA_COUNT;

    return 0;
}

/**
 * @brief Initializes memory arenas for allocation management
 *
 * Creates and initializes the arena system used for memory allocation:
 * - Allocates the arena pointer array via system_alloc_mmap()
 * - Creates the main arena for single-threaded operation
 * - Creates additional arenas if thread-safe mode is enabled
 * - Handles partial initialization failures gracefully
 *
 * @return int 0 on success, -1 on failure
 *
 * @retval 0 All arenas successfully created and initialized
 * @retval -1 Arena allocation failed (system out of memory)
 *
 * @note In case of partial failure (some arenas fail to create), the function
 *       continues with fewer arenas rather than failing completely.
 *
 * @internal
 * Arena allocation strategy:
 * - Main arena (index 0): Always created for fallback
 * - Worker arenas (1..N): Created based on arena_count and thread_safe flag
 * - Memory: Each arena manages its own free lists and heap segments
 *
 * @see memforge_arena_t
 * @see arena_create()
 */
int memforge_init_arenas(void)
{
    // Allocate arena array
    memforge_arenas = system_alloc_mmap(sizeof(memforge_arena_t *) * memforge_config.arena_count);
    if (memforge_arenas == NULL)
    {
        return -1;
    }

    // Create main arena
    memforge_main_arena = arena_create();
    if (memforge_main_arena == NULL)
    {
        system_free_mmap(memforge_arenas, sizeof(memforge_arena_t *) * memforge_config.arena_count);
        return -1;
    }

    memforge_arenas[0] = memforge_main_arena;

    // Initialize other arenas if thread-safe mode enabled
    if (memforge_config.thread_safe)
    {
        for (size_t i = 1; i < memforge_config.arena_count; i++)
        {
            memforge_arenas[i] = arena_create();
            if (memforge_arenas[i] == NULL)
            {
                // Continue with fewer arenas
                memforge_config.arena_count = i;
                break;
            }
        }
    }

    return 0;
}

/**
 * @brief Cleans up allocator resources and resets global state
 *
 * Safely shuts down the allocator by:
 * - Destroying all memory arenas and their resources
 * - Freeing the arena pointer array
 * - Resetting global variables to initial state
 * - Maintaining thread safety during cleanup
 *
 * @post All allocator resources are released back to the system
 * @post Global state is reset (memforge_initialized = false)
 * @post Subsequent allocations will trigger re-initialization
 *
 * @note This function is idempotent - safe to call multiple times
 * @warning After cleanup, any outstanding allocated memory becomes invalid
 *
 * @par Cleanup Sequence:
 * 1. Destroy all arena objects and their internal structures
 * 2. Free the arena pointer array via system_free_mmap()
 * 3. Reset global pointers to NULL
 * 4. Mark allocator as uninitialized
 *
 * @see memforge_init()
 * @see memforge_reset()
 */
void memforge_cleanup(void)
{
    if (!memforge_initialized)
    {
        return;
    }

    // Destroy all arenas
    for (size_t i = 0; i < memforge_config.arena_count; i++)
    {
        if (memforge_arenas[i] != NULL)
        {
            arena_destroy(memforge_arenas[i]);
        }
    }

    // Free arena array
    if (memforge_arenas != NULL)
    {
        system_free_mmap(memforge_arenas, sizeof(memforge_arena_t *) * memforge_config.arena_count);
        memforge_arenas = NULL;
    }

    memforge_main_arena = NULL;
    memforge_initialized = false;

    debug_log("MemForge cleanup completed");
}

/**
 * @brief Resets allocator to initial state (primarily for testing)
 *
 * Performs complete cleanup followed by re-initialization with defaults.
 * This is useful for:
 * - Unit testing between test cases
 * - Benchmarking different configurations
 * - Recovery from corrupted state (emergency reset)
 *
 * @note Statistics are zeroed out during reset
 * @note Configuration returns to system-detected defaults
 * @note Not recommended for production use - primarily for testing
 *
 * @par Reset Sequence:
 * 1. memforge_cleanup() - Release all resources
 * 2. memset(&memforge_stats, 0) - Reset statistics
 * 3. memforge_init(NULL) - Reinitialize with defaults
 *
 * @see memforge_cleanup()
 * @see memforge_init()
 *
 * @par Testing Example:
 * @code
 * void test_allocator() {
 *     memforge_reset(); // Clean state for test
 *     // Run test operations...
 *     assert(memforge_stats.allocation_count == expected);
 *     memforge_reset(); // Cleanup for next test
 * }
 * @endcode
 */
void memforge_reset(void)
{
    memforge_cleanup();
    memset(&memforge_stats, 0, sizeof(memforge_stats_t));
    memforge_init(NULL);
}