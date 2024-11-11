/**
 * Type-V Virtual Machine
 * Author: praisethemoon
 * gc.h: Garbage Collection
 */

#ifndef TYPE_V_GC_H
#define TYPE_V_GC_H

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "core.h"

// Constants for garbage collection
#define CELL_SIZE 64
#define MAX_CELLS 4096000
#define REGION_SIZE (CELL_SIZE * MAX_CELLS)
#define SSB_SIZE 128
#define MAX_UPDATE_LIST 1024  // Maximum number of pointers to update after GC

// Memory Regions
#define NURSERY_SIZE (REGION_SIZE)
#define SURVIVOR_SIZE (REGION_SIZE*2)
#define OLD_GEN_INITIAL_SIZE (REGION_SIZE * 2)

// Object States
typedef enum {
    WHITE = 0b00,
    GRAY = 0b01,
    BLACK = 0b10
} ObjectColor;

// Memory Region Types
typedef enum {
    REGION_NURSERY,
    REGION_SURVIVOR_FROM,
    REGION_SURVIVOR_TO,
    REGION_OLD
} RegionType;

// Metadata for each memory region
typedef struct {
    uint64_t block_bitmap[MAX_CELLS / 64];
    uint8_t color[MAX_CELLS / 4];
    uint16_t gray_stack[MAX_CELLS];
    uint16_t gray_stack_top;
    uint16_t ssb[SSB_SIZE];
    uint16_t ssb_top;
} RegionMetadata;

// Memory Region Structure
typedef struct {
    RegionType type;
    RegionMetadata metadata;
    uint8_t *data;
    size_t size;
} MemoryRegion;

// Memory Manager Structure
typedef struct {
    MemoryRegion *nursery;
    MemoryRegion *survivor_from;
    MemoryRegion *survivor_to;
    MemoryRegion *old_gen;
} MemoryManager;

// Forwarding table entry
typedef struct {
    void *old_ptr;
    void *new_ptr;
} ForwardEntry;

// GC Context Structure
typedef struct GCContext {
    MemoryManager *memory_manager;
    ForwardEntry forwarding_table[MAX_CELLS];
    size_t forwarding_table_size;

    // List to store pointers needing updates after GC
    void **update_list[MAX_UPDATE_LIST];
    size_t update_list_size;
} GCContext;

// GC API
GCContext *gc_init(void);
void gc_finalize(GCContext *context);
void *gc_alloc(TypeV_Core* core, size_t size);
void gc_minor_collect(GCContext *context);
void gc_full_collect(GCContext *context);
void gc_register_root(GCContext *context, void *root);
void gc_unregister_root(GCContext *context, void *root);
void mark_ptr(GCContext *gc, void *ptr_ptr);
void mark_all_roots(GCContext *context);

// Helper functions
TypeV_ObjectHeader* get_header_from_pointer(GCContext *ctx, void * objectPtr);
void add_to_forwarding_table(GCContext *gc, void *old_ptr, void *new_ptr);
void *get_forwarded_pointer(GCContext *gc, void *old_ptr);
void add_to_update_list(GCContext *gc, void **ptr_ptr);
void update_pointers_from_list(GCContext *gc);
void move_live_objects(GCContext *context);
void allocate_in_survivor(MemoryRegion *to, void *ptr);
void mark_in_place(void *ptr);
void core_gc_update_references_state(TypeV_Core* core, TypeV_FuncState* state);
#endif // TYPE_V_GC_H
