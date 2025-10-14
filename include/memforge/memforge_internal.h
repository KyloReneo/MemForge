#ifndef MEMFORGE_INTERNAL_H
#define MEMFORGE_INTERNAL_H

#include "memforge.h"
#include "memforge_config.h"

// Global state declarations (they are defined in init.c)
extern bool memforge_initialized;
extern memforge_stats_t memforge_stats;
extern memforge_config_t memforge_config;
// Fututre works!
// extern memforge_arena_t* memforge_main_arena;

// These are Internal functions that are not part of the public API

// Platform abstraction
void* system_alloc_sbrk(size_t size);
void* system_alloc_mmap(size_t size);
void system_free_sbrk(void* ptr, size_t size);
void system_free_mmap(void* ptr, size_t size);
size_t system_page_size(void);
int system_init(void);
void system_cleanup(void);

// Free list management
void free_list_init(void);
void free_list_add(block_header_t* block);
void free_list_remove(block_header_t* block);
block_header_t* free_list_find(size_t size);
void free_list_dump(void);

// Block management
block_header_t* block_split(block_header_t* block, size_t size);
block_header_t* block_coalesce(block_header_t* block);
bool block_validate(block_header_t* block);

// Heap management
heap_segment_t* heap_segment_create(void* base, size_t size);
void heap_segment_destroy(heap_segment_t* segment);
void heap_verify(void);

// Size class management
size_t size_class_get(size_t size);
size_t size_class_from_index(size_t index);

// Environment and configuration
void apply_environment_tuning(void);
void setup_default_config(void);

// Debug utilities
#ifdef MEMFORGE_DEBUG
void debug_log(const char* format, ...);
void debug_dump_block(block_header_t* block);
void debug_dump_heap(void);
#else
#define debug_log(...)
#define debug_dump_block(block)
#define debug_dump_heap()
#endif

#endif 