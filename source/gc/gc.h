#ifndef GC_H
#define GC_H

#include <stddef.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include "../core.h"

/*** GC Configuration ***/

/**
 * Size of each cell.
 */
#define CELL_SIZE 64

/**
 * Maximum number of cells in the nursery region.
 */
#define NURSERY_MAX_CELLS 4096

/**
 * Initial number of cells in the old generation region.
 * The number of cells is scaled by a factor of 2 each time the old generation is extended.
 * The factor is stored in the `capacityFactor` field of the `TypeV_OldGenerationRegion` structure.
 */
#define INITIAL_OLD_CELLS 8192

/**
 * Total cellSize of the nursery region.
 */
#define NURSERY_REGION_SIZE (CELL_SIZE * NURSERY_MAX_CELLS)

/**
 * Initial cellSize of the old generation region.
 */
#define OLD_REGION_INITIAL_SIZE (CELL_SIZE * INITIAL_OLD_CELLS)

/**
 * Total cellSize of the nursery region.
 * NURSERY_SIZE is the cellSize of the `from` and `to` spaces combined so to speak.
 * Each is NURSERY_REGION_SIZE.
 * Also metadata is limited to NURSERY_REGION_SIZE.
 */
#define NURSERY_SIZE (NURSERY_REGION_SIZE*2)

#define PROMOTION_SURVIVAL_THRESHOLD 4

/**
 * Object Colors
 */
typedef enum {
    WHITE = 0b00,
    GRAY = 0b01,
    BLACK = 0b11
} ObjectColor;

/**
 * Nursery Region
 * Uses split space for `from` and `to` spaces.
 */
typedef struct TypeV_NurseryRegion {
    /**
     * Bitmap for the color of each cell in the nursery region.
     * Each cell is represented by 2 bits.
     */
    uint8_t color_bitmap[(NURSERY_MAX_CELLS * 2) / 8];

    /**
     * Bitmap for the active status of each cell in the nursery region.
     * Each cell is represented by 1 bit.
     */
    uint8_t active_bitmap[NURSERY_MAX_CELLS / 8];

    /**
     * The current cellSize of the nursery region in cells.
     */
    size_t cellSize;

    /**
     * The base address of the `from` space.
     */
    uint8_t* from; // always storing in from, will be swapped with `to` later on

    /**
     * The base address of the `to` space.
     */
    uint8_t* to;

    /**
     * The total cellSize of the nursery region in cells.
     */
    uint8_t* data;
} TypeV_NurseryRegion;

/**
 * Older generation,
 * objects are promote here after surviving PROMOTION_SURVIVAL_THRESHOLD
 */
typedef struct TypeV_OldGenerationRegion {
    /**
     * Bitmap for the color of each cell in the old generation region.
     * 2 bits per cell
     * Initial size (INITIAL_OLD_CELLS * 2) / 8
     */
    uint8_t* color_bitmap;

    /**
     * Bitmap for the active status of each cell in the old generation region.
     * 1 bit per cell
     * Initial size INITIAL_OLD_CELLS / 8
     */
    uint8_t* active_bitmap;

    /**
     * Occupied cellSize of the region
     */
    size_t cellSize;

    /**
     * Data Segment
     */
    uint8_t* data;

    /**
     * Capacity Factor, multiplied by 2 each time the old generation is extended
     * default = 1
     */
    size_t capacityFactor;
} TypeV_OldGenerationRegion;

/**
 * Update List pairs
 */
typedef struct TypeV_GCUpdateList {
    uintptr_t old_address;
    uintptr_t new_address;
} TypeV_GCUpdateList;

/**
 * Garbage Collector struct, part of the core
 */
typedef struct TypeV_GC {
    /**
     * Nursery Region
     */
    TypeV_NurseryRegion* nurseryRegion;

    /**
     * Old Generation Region
     */
    TypeV_OldGenerationRegion* oldRegion;

    /**
     * Amount of minor GCs performed, since last major GC
     */
    uint32_t minorGCCount;

    /**
     * Static update list.
     */
    TypeV_GCUpdateList nurseryUpdateList[NURSERY_MAX_CELLS];

    /**
     * Number of items to update, resets after each GC
     */
    uint64_t updateListSize;
}TypeV_GC;


// Macros for accessing color_bitmap
#define GET_COLOR(bitmap, cell) ((bitmap[(cell) / 4] >> (((cell) % 4) * 2)) & 0x3)

#define SET_COLOR(bitmap, cell, color) do { \
    uint8_t mask = 0x3 << (((cell) % 4) * 2); \
    bitmap[(cell) / 4] = (bitmap[(cell) / 4] & ~mask) | (((color) & 0x3) << (((cell) % 4) * 2)); \
} while (0)


// Macros for accessing active_bitmap
#define GET_ACTIVE(bitmap, cell) ((bitmap[(cell) / 8] >> ((cell) % 8)) & 0x1)

#define SET_ACTIVE(bitmap, cell, active) do { \
    uint8_t mask = 1 << ((cell) % 8); \
    if (active) \
        bitmap[(cell) / 8] |= mask; \
    else \
        bitmap[(cell) / 8] &= ~mask; \
} while (0)


/**
 * Initializes the GC system
 * @return
 */
TypeV_GC* gc_initialize();

/**
 * Creates a new nursery region
 * @return
 */
TypeV_NurseryRegion* gc_create_nursery();

/**
 * Creates a new old generation region
 * @return
 */
TypeV_OldGenerationRegion* gc_create_old_generation();

/**
 * Allocates memory in the nursery region, with the given cellSize
 * if it exceeds the nursery cellSize, it will trigger a minor GC.
 * if minor GC fails, it will trigger a major GC.
 * @param core
 * @param size
 * @return
 */
TypeV_ObjectHeader* gc_alloc(TypeV_Core* core, size_t size);

/**
 * Performs a minor GC
 * @param core
 */
void gc_minor_gc(TypeV_Core* core);

/**
 * Runs after minor GC to check if the old generation needs to be extended
 * @param core
 */
void gc_extend_old(TypeV_Core* core);

/**
 * Pointers are updated top-down, so when we push a new (oldPtr, newPtr) pair,
 * they are inherently sorted. This function uses binary search to find the new address
 * If not found, it returns the old address.
 * @param core
 * @param oldPtr
 * @return
 */
uintptr_t gc_get_new_ptr_location(TypeV_Core* core, uintptr_t oldPtr);

/**
 * Debugs the nursery.
 * Current cellSize is not debuggable, so it will need to be decreased to a managable cellSize
 * i.e 32/64 cells.
 * @param gc
 */
void gc_debug_nursery(TypeV_GC* gc);

#endif // GC_H
