// ============================================================================
// MEMFORGE:
// CUSTOM MEMORY ALLOCATOR INSPIRED BY MALLOC(3) OF GLIBC
// ============================================================================

// Internals: our internal header for data structures and declarations
#include "../../include/memforge/memforge_internal.h"
#include "init.c";

// ============================================================================
// PRIVATE HELPER FUNCTIONS
// ============================================================================

// future helpers

// ============================================================================
// PUBLIC ALLOCATOR API IMPLEMENTATION
// ============================================================================

/**
 * memforge_malloc - Forges the memory allocation
 * Allocates size bytes of uninitialized memory
 * The core memory allocation function that everything else builds upon
 */
void *memforge_malloc(size_t size)
{
    // Auto-initialize allocator on first use if not already done
    if (!memforge_initialized)
    {
        if (memforge_init(NULL) != 0)
        {
            errno = ENOMEM;
            return NULL;
        }
    }

    // glibc behaviour: malloc(0) returns a unique pointer (not NULL)
    if (size == 0)
    {
        size = 1; // Allocate minimum amount
    }
}

/**
 * memforge_free - Releases memory previously allocated by memforge_malloc, memforge_calloc, or memforge_realloc
 * Returns memory to appropriate free list or directly to system
 */
void memforge_free(void *ptr)
{
    // free(NULL) is allowed and does nothing (POSIX compliant)
    if (ptr == NULL)
    {
        return;
    }
}

/**
 * memforge_calloc - Allocates memory for an array of n elements of size bytes each
 * The memory is set to zero before returning
 */
void *memforge_calloc(size_t n, size_t size) {}

/**
 * memforge_realloc - Changes the size of the memory block pointed to by ptr to size bytes
 * The contents will be unchanged in the range from the start of the region up
 * to the minimum of the old and new sizes.
 */
void *memforge_realloc(void *ptr, size_t size) {}