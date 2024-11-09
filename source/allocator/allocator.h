#ifndef TYPEV_ALLOCATOR_H
#define TYPEV_ALLOCATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#define COLESSEUM_ARENA_SIZE (2 * 1024 * 1024) // 2 MB arena size
#define COLESSEUM_CELL_SIZE 128                // 16 bytes per cell
#define COLESSEUM_NUM_CELLS (COLESSEUM_ARENA_SIZE / COLESSEUM_CELL_SIZE)
#define COLESSEUM_BITMAP_SIZE (COLESSEUM_NUM_CELLS / 8) // Size in bytes for 16K cells (2 KB per bitmap)
#define ARENA_SKIP_THRESHOLD 3       // Number of times an arena can be skipped before being marked unusable


// Arena structure to manage the entire memory pool
typedef struct TypeV_GCArena {
    uint8_t block_bitmap[COLESSEUM_BITMAP_SIZE]; // Bitmap to track block allocation status
    uint8_t mark_bitmap[COLESSEUM_BITMAP_SIZE];  // Bitmap to track mark status for GC
    uint8_t *top;                      // Pointer to the first free slot
    struct TypeV_GCArena *prev;
    uint8_t data[COLESSEUM_ARENA_SIZE];          // Data section for allocations
} TypeV_GCArena;

typedef struct TypeV_Colosseum {
    TypeV_GCArena *head;
    TypeV_GCArena *busyHead;
    uint32_t busyCount;
} TypeV_Colosseum;

TypeV_Colosseum* tv_colosseum_init();
void tv_colosseum_free(TypeV_Colosseum* colosseum);

TypeV_GCArena* tv_arena_init(TypeV_Colosseum* colosseum);
void tv_arena_free(TypeV_GCArena* arena, TypeV_Colosseum* colosseum);

uintptr_t tv_gc_alloc(TypeV_Colosseum* colosseum, size_t size);

TypeV_GCArena* tv_arena_find_pointerArena(TypeV_Colosseum* col, uintptr_t ptr);
void tv_arena_mark_ptr(TypeV_GCArena* arena, uintptr_t ptr);
#endif

