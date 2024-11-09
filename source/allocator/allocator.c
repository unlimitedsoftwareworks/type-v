#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <mimalloc.h>

#include "allocator.h"
// Function to create a new arena

static inline uint8_t is_size_large(size_t size) {
    // returns true if size > 128 bytes
    return size > 128;
}

// Function to set a bit in a bitmap
static inline void set_bitmap_bit(uint8_t* bitmap, size_t index) {
    bitmap[index / 8] |= (1 << (index % 8));
}

// Function to clear a bit in a bitmap
static inline void clear_bitmap_bit(uint8_t* bitmap, size_t index) {
    bitmap[index / 8] &= ~(1 << (index % 8));
}

// Function to check if a bit is set in a bitmap
static inline bool is_bitmap_bit_set(const uint8_t* bitmap, size_t index) {
    return (bitmap[index / 8] & (1 << (index % 8))) != 0;
}


TypeV_Colosseum* tv_colosseum_init() {
    TypeV_Colosseum* colosseum = (TypeV_Colosseum*)mi_malloc(sizeof(TypeV_Colosseum));
    colosseum->head = tv_arena_init(colosseum);
    colosseum->busyHead = NULL;
    return colosseum;
}

void tv_colosseum_free(TypeV_Colosseum* colosseum) {
    mi_free(colosseum);
}

TypeV_GCArena* tv_arena_init(TypeV_Colosseum* colosseum) {
    TypeV_GCArena* arena = (TypeV_GCArena*)mi_malloc(sizeof(TypeV_GCArena));

    // set the flags to 0
    memset(arena->block_bitmap, 0, COLESSEUM_BITMAP_SIZE);
    memset(arena->mark_bitmap, 0, COLESSEUM_BITMAP_SIZE);

    arena->top = arena->data;
    return arena;
}

void tv_arena_free(TypeV_GCArena* arena, TypeV_Colosseum* colosseum) {
    mi_free(arena);
}

TypeV_GCArena* tv_arena_find_usable(TypeV_Colosseum* colosseum, size_t size) {
    TypeV_GCArena* arena = colosseum->head;
    TypeV_GCArena* parent = NULL;
    while (arena != NULL) {
        if (__builtin_expect(arena->top + size <= arena->data + COLESSEUM_ARENA_SIZE, 1)) {
            return arena;
        }
        if (__builtin_expect(is_size_large(size), 0)) {
            parent->prev = arena->prev;
            arena->prev = colosseum->busyHead;
            colosseum->busyHead = arena;
        }
        parent = arena;
        arena = arena->prev;
    }
    return NULL;
}

uintptr_t tv_gc_alloc(TypeV_Colosseum* colosseum, size_t size) {
    /**
     * 1. Start from the head of the colosseum
     * 2. Find an arena that has enough space for the allocation
     * 3. When searching, if an arena has no space and size is small, we consider
     *    the arena unusable and we add into busyHead
     * 4. Update the arena pointer to the next arena
     * 5. Return the pointer to the allocated memory
     */
    TypeV_GCArena* arena = tv_arena_find_usable(colosseum, size);
    if (__builtin_expect(arena == NULL, 0)) {
        arena = tv_arena_init(colosseum);
        arena->prev = colosseum->head;
        colosseum->head = arena;
    }

    uintptr_t ptr = (uintptr_t)arena->top;
    arena->top += size;

    // Calculate the start and end indices of the allocated block
    size_t start_index = (ptr - (uintptr_t)arena->data) / COLESSEUM_CELL_SIZE;
    size_t num_cells = (size + COLESSEUM_CELL_SIZE - 1) / COLESSEUM_CELL_SIZE; // Number of cells to allocate

    // Update the block bitmap for all allocated cells
    for (size_t i = 0; i < num_cells; i++) {
        set_bitmap_bit(arena->block_bitmap, start_index + i);
    }

    return ptr;
}
