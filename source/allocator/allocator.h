#ifndef TYPEV_ALLOCATOR_H
#define TYPEV_ALLOCATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#define ARENA_SIZE (64 * 1024) // 64 KB arena size
#define CELL_SIZE 16           // Each cell is 16 bytes
#define NUM_CELLS (ARENA_SIZE / CELL_SIZE) // Number of cells in an arena

// Arena structure to manage the entire memory pool
typedef struct Arena {
    uint8_t data[ARENA_SIZE];   // Data section for allocations
    bool cell_allocated[NUM_CELLS]; // Array to track allocation status of each cell
    size_t used_cells;           // Number of currently allocated cells
    struct Arena* next;          // Pointer to the next arena in the list (for managing multiple arenas)
} Arena;

// Function to create a new arena
Arena* create_arena();

// Function to allocate memory from the arena
void* arena_alloc(Arena* arena, size_t size);

// Function to mi_free memory back to the arena
void arena_free(Arena* arena, void* ptr, size_t size);

// Function to destroy the arena and mi_free all memory
void destroy_arena(Arena* arena);

// Function to manage multiple arenas for more flexible allocation
void* allocate_from_arenas(Arena** arena_list, size_t size);

#endif

