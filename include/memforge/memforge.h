#ifndef MEMFORGE_H
#define MEMFORGE_H

#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // Core functions

    void *memforge_malloc(size_t size);
    void memforge_free(void *ptr);
    void *memforge_calloc(size_t n, size_t size);
    void *memforge_realloc(void *ptr, size_t);

    // Advanced alignment functions
    void *fmalign(size_t alignment, size_t size);
    int posix_fmalign(void **memptr, size_t alignment, size_t size);
    void *fm_aligned_alloc(size_t alignment, size_t size);

    // Utility functions
    size_t fmalloc_usable_size(void *ptr);
    void fmalloc_stats(void);
    int fmalloc_trim(size_t pad);
    void fmalloc_info(int options, FILE *stream);

    // Debug and control functions
    void fmalloc_debug(int level);
    void fmalloc_verify(void);
    int fmalloc_set_state(void *state);
    void *fmalloc_get_state(void);

    // Initialization and cleanup declarations
    int memforge_init(const memforge_config_t *config);
    void memforge_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif