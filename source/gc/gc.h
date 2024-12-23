#ifndef TYPEV_GC_H
#define TYPEV_GC_H

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../core.h"

/* ======================= CONSTANTS ======================= */

#define CELL_SIZE 64
#define NURSERY_MAX_CELLS 1310720
#define INITIAL_OLD_CELLS 1310720
#define NURSERY_REGION_SIZE (CELL_SIZE * NURSERY_MAX_CELLS)
#define NURSERY_SIZE (NURSERY_REGION_SIZE * 2)
#define OLD_REGION_INITIAL_SIZE (CELL_SIZE * INITIAL_OLD_CELLS)
#define PROMOTION_SURVIVAL_THRESHOLD 4

// Define GC_LOG to enable logging, or leave undefined to disable
//#define GC_LOG

#ifdef GC_LOG
#define gc_log(fmt, ...) printf("[GC_LOG] " fmt "\n", ##__VA_ARGS__)
#else
#define gc_log(fmt, ...) ((void)0)
#endif

/** Get the active bit for a cell in the bitmap **/
#define GET_ACTIVE(bitmap, cell) ((bitmap[(cell) / 8] >> ((cell) % 8)) & 0x1)

/* ======================= ENUMS ======================= */

typedef enum {
    WHITE = 0b00,
    GRAY = 0b01,
    NOTSU = 0b10,
    BLACK = 0b11
} ObjectColor;

/* ======================= STRUCTURES ======================= */

/**
 * @brief Object Types, used to identify a the underlying structure of GC allocated memory
 * object
 */
typedef enum {
    OT_CLASS = 0,
    OT_STRUCT,
    OT_ARRAY,
    OT_CLOSURE,
    OT_COROUTINE,
    OT_CUSTOM_OBJECT,
    OT_CUSTOM_COLLECTABLE_OBJECT,
}TypeV_ObjectType;

typedef struct TypeV_ObjectHeader {
    TypeV_ObjectType type;
    ObjectColor color;           // Tri-color marking
    size_t totalSize;            // Object size in bytes
    size_t surviveCount;         // GC survival counter
    struct TypeV_ObjectHeader* fwd;    // Forwarding pointer for GC
    uint8_t location;            // 0 for nursery, 1 for old region
    uint32_t uid;                // Unique ID for debugging
}TypeV_ObjectHeader;

typedef struct TypeV_NurseryRegion {
    uint8_t* active_bitmap;      // Bitmap for active cells
    size_t cell_size;            // Total allocated cells
    uint8_t* from;               // From-space pointer
    uint8_t* to;                 // To-space pointer
    uint8_t* data;               // Combined from/to space
} TypeV_NurseryRegion;

typedef struct TypeV_OldGenerationRegion {
    uint8_t* active_bitmap;      // Bitmap for active cells
    uint8_t* dirty_bitmap;       // Bitmap for dirty cells
    size_t cell_size;            // Total allocated cells
    uint8_t* data;               // Old generation data
    size_t capacity_factor;      // Capacity scaling factor
    uint8_t* from;               // From-space pointer (downwards)
    uint8_t* to;                 // To-space pointer (upwards)
    int8_t direction;               // Direction indicator: 1 for downwards, -1 for upwards
} TypeV_OldGenerationRegion;

typedef struct TypeV_GC {
    TypeV_NurseryRegion nursery;  // Nursery region for young objects
    TypeV_OldGenerationRegion oldRegion; // Old generation region
} TypeV_GC;

/* ======================= FUNCTION DECLARATIONS ======================= */


/** Initialize the GC */
TypeV_GC* initialize_gc(void);

/** Allocate memory using the GC */
void* gc_alloc(TypeV_Core* core, size_t size);

/** Perform a minor mark phase */
void perform_minor_mark(TypeV_Core* core);

/** Perform a major mark phase */
void perform_major_mark(TypeV_Core* core);

/** Perform a minor garbage collection */
void perform_minor_gc(TypeV_Core* core);

/** Perform a major garbage collection */
void perform_major_gc(TypeV_Core* core);

/** Cleanup all GC resources */
void cleanup_gc(TypeV_Core* core);

void write_barrier(TypeV_Core* core, TypeV_ObjectHeader* old_obj, TypeV_ObjectHeader* new_obj);


#endif // TYPEV_GC_H