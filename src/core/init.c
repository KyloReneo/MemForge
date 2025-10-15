#include "../../include/memforge/memforge_internal.h"
#include "../../include/memforge/memforge.h"

// ============================================================================
// GLOBAL STATE DEFINITIONS
// ============================================================================

memforge_config_t memforge_config = {0};
memforge_stats_t memforge_stats = {0};
memforge_arena_t *memforge_main_arena = NULL;
memforge_arena_t **memforge_arenas = NULL;
bool memforge_initialized = false;

// Size classes for segregated free lists (powers of two with some spacing)
size_t memforge_size_classes[MEMFORGE_SIZE_CLASS_COUNT] = {
    16, 32, 64, 128, 256, 512, 1024, 2048,
    4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288};

// ============================================================================
// INITIALIZATION FUNCTIONS
// ============================================================================

/**
 * Initializes the MemForge allocator with default or provided configuration
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

    // Initialize size classes
    for (size_t i = 0; i < MEMFORGE_SIZE_CLASS_COUNT; i++)
    {
        memforge_size_classes[i] = MEMFORGE_ALIGN(memforge_size_classes[i]);
    }

    memforge_initialized = true;
    debug_log("MemForge initialized successfully");
    return 0;
}

/**
 * Sets up default configuration values
 */
int memforge_init_default_config(void)
{
    memforge_config.page_size = sysconf(_SC_PAGESIZE);
    memforge_config.mmap_threshold = MEMFORGE_DEFAULT_MMAP_THRESHOLD;
    memforge_config.strategy = MEMFORGE_STRATEGY_HYBRID;
    memforge_config.thread_safe = MEMFORGE_THREAD_SAFE;
    memforge_config.debug_enabled = DEBUG_LOGGING;
    memforge_config.arena_count = MEMFORGE_DEFAULT_ARENA_COUNT;

    return 0;
}

/**
 * Initializes memory arenas for allocation
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
 * Cleans up allocator resources
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
 * Resets allocator to initial state (for testing)
 */
void memforge_reset(void)
{
    memforge_cleanup();
    memset(&memforge_stats, 0, sizeof(memforge_stats_t));
    memforge_init(NULL);
}