#ifndef MEMFORGE_H
#define MEMFORGE_H

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // Core allocation function
    void *falloc(size_t size); // Forge the allocation
    void free(void *ptr);
    void *calloc(size_t n, size_t size);
    void *realloc(void *ptr, size_t);

    // Advanced alignment functions
    void *memalign(size_t alignment, size_t size);
    int posix_memalign(void **memptr, size_t alignment, size_t size);
    void *aligned_alloc(size_t alignment, size_t size);

    // Utility functions
    size_t malloc_usable_size(void *ptr);
    void malloc_stats(void);
    int malloc_trim(size_t pad);
    void malloc_info(int options, FILE *stream);

    // Debug and control functions
    void malloc_debug(int level);
    void malloc_verify(void);
    int malloc_set_state(void *state);
    void *malloc_get_state(void);

    // Configuration
    typedef struct memalloc_config
    {
        size_t mmap_threshold;
        size_t page_size;
        int strategy;
        int debug_level;
        bool thread_safe;
    } mfalloc_config_t;

    int memalloc_init(const mfalloc_config_t *config);
    void memalloc_cleanup(void);
#ifdef __cplusplus
}
#endif

#endif