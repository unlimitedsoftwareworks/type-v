#ifndef GC_H
#define GC_H

#include <stddef.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include "core.h"

/*** GC Configuration ***/
#define CELL_SIZE 64
#define MAX_CELLS 4096
#define REGION_SIZE (CELL_SIZE * MAX_CELLS)

// Memory Regions
#define NURSERY_SIZE (REGION_SIZE*2)

// Object States
typedef enum {
    WHITE = 0b00,
    GRAY = 0b01,
    BLACK = 0b10
} ObjectColor;

typedef struct TypeV_NurseryRegion {
    uint8_t color_bitmap[(MAX_CELLS * 2) / 8];   // 2 bits per cell
    uint8_t active_bitmap[MAX_CELLS / 8];        // 1 bit per cell
    uint8_t* from; // always storing in from, will be swapped with `to` later on
    uint8_t* to;
    size_t size; // in cells!
    uint8_t* data;
} TypeV_NurseryRegion;


typedef struct TypeV_OldGenerationRegion {
    uint8_t color_bitmap[(MAX_CELLS * 2) / 8];     // 2 bits per cell for color (same as nursery)
    uint8_t active_bitmap[MAX_CELLS / 8];          // 1 bit per cell for active status
    size_t size;                                   // Total size of the region
    uint8_t* data;                                 // Data segment
} TypeV_OldGenerationRegion;

typedef struct TypeV_GCUpdateList {
    uintptr_t old_address;
    uintptr_t new_address;
} TypeV_GCUpdateList;

typedef struct TypeV_GC {
    TypeV_NurseryRegion* nurseryRegion;
    TypeV_OldGenerationRegion* oldRegion;

    // count since last major gc
    uint32_t minorGCCount;

    TypeV_GCUpdateList nurseryUpdateList[MAX_CELLS];
    uint64_t updateListSize;
}TypeV_GC;


// Macros for accessing color_bitmap
#define GET_COLOR(bitmap, cell) ((bitmap[(cell * 2) / 8] >> ((cell * 2) % 8)) & 0b11)
#define SET_COLOR(bitmap, cell, color) \
    bitmap[(cell * 2) / 8] = (bitmap[(cell * 2) / 8] & ~(0b11 << ((cell * 2) % 8))) | ((color & 0b11) << ((cell * 2) % 8))

// Macros for accessing active_bitmap
#define GET_ACTIVE(bitmap, cell) ((bitmap[cell / 8] >> (cell % 8)) & 0b1)
#define SET_ACTIVE(bitmap, cell, active) \
    bitmap[cell / 8] = (bitmap[cell / 8] & ~(1 << (cell % 8))) | ((active & 0b1) << (cell % 8))


TypeV_NurseryRegion* gc_create_nursery();
TypeV_OldGenerationRegion* gc_create_old_generation();
TypeV_GC* gc_initialize();
TypeV_ObjectHeader* gc_alloc(TypeV_Core* core, size_t size);
void gc_minor_gc(TypeV_Core* core);


TypeV_ObjectHeader* gc_mark_nursery_ptr(uintptr_t ptr, TypeV_NurseryRegion* nursery, ObjectColor color);


void gc_mark_state(TypeV_Core* core, TypeV_FuncState* state, uint8_t inNursery);

uintptr_t gc_get_new_ptr_location(TypeV_Core* core, uintptr_t oldPtr);
void gc_update_object_pointers(TypeV_Core* core, uintptr_t old_address, uintptr_t new_address);
void gc_update_state(TypeV_Core* core, TypeV_FuncState* state, uint8_t inNursery);
void gc_debug_nursery(TypeV_GC* gc);
#endif // GC_H
