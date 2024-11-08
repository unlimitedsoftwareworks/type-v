#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "allocator.h"

// Function to create a new arena
Arena* create_arena() {
    Arena* arena = (Arena*)malloc(sizeof(Arena));
    if (arena == NULL) {
        fprintf(stderr, "Failed to allocate memory for arena\n");
        return NULL;
    }

    // Initialize the arena
    for (size_t i = 0; i < NUM_CELLS; i++) {
        arena->cell_allocated[i] = false; // All cells are initially free
    }
    arena->used_cells = 0;
    arena->next = NULL;

    return arena;
}

// Function to allocate memory from the arena
void* arena_alloc(Arena* arena, size_t size) {
    if (size == 0 || size > ARENA_SIZE) {
        fprintf(stderr, "Invalid allocation size\n");
        return NULL;
    }

    // Calculate the number of cells required to fulfill the request
    size_t num_cells_needed = (size + CELL_SIZE - 1) / CELL_SIZE; // Round up to nearest multiple of CELL_SIZE

    // Find a contiguous block of free cells
    for (size_t i = 0; i <= NUM_CELLS - num_cells_needed; i++) {
        bool found = true;
        for (size_t j = 0; j < num_cells_needed; j++) {
            if (arena->cell_allocated[i + j]) {
                found = false;
                break;
            }
        }

        // If a suitable block is found, mark cells as allocated and return a pointer
        if (found) {
            for (size_t j = 0; j < num_cells_needed; j++) {
                arena->cell_allocated[i + j] = true;
            }
            arena->used_cells += num_cells_needed;
            return (void*)&arena->data[i * CELL_SIZE];
        }
    }

    // No suitable block found
    //fprintf(stderr, "Failed to allocate memory from arena\n");
    return NULL;
}

// Function to free memory back to the arena
void arena_free(Arena* arena, void* ptr, size_t size) {
    if (ptr == NULL || size == 0 || size > ARENA_SIZE) {
        fprintf(stderr, "Invalid pointer or size for free\n");
        return;
    }

    // Calculate the starting cell index from the pointer
    uintptr_t offset = (uintptr_t)ptr - (uintptr_t)arena->data;
    if (offset % CELL_SIZE != 0 || offset >= ARENA_SIZE) {
        fprintf(stderr, "Invalid pointer for free\n");
        return;
    }

    size_t start_cell = offset / CELL_SIZE;
    size_t num_cells_to_free = (size + CELL_SIZE - 1) / CELL_SIZE; // Round up to nearest multiple of CELL_SIZE

    // Mark the cells as free
    for (size_t i = start_cell; i < start_cell + num_cells_to_free; i++) {
        arena->cell_allocated[i] = false;
    }
    arena->used_cells -= num_cells_to_free;
}

// Function to destroy the arena and free all memory
void destroy_arena(Arena* arena) {
    free(arena);
}


// Function to manage multiple arenas for more flexible allocation
void* allocate_from_arenas(Arena** arena_list, size_t size) {
    Arena* current = *arena_list;

    // Try to allocate from an existing arena
    while (current != NULL) {
        void* ptr = arena_alloc(current, size);
        if (ptr != NULL) {
            return ptr; // Allocation successful
        }
        current = current->next;
    }

    // If no existing arena has space, create a new arena
    Arena* new_arena = create_arena();
    if (new_arena == NULL) {
        return NULL; // Failed to create a new arena
    }

    // Add the new arena to the list
    new_arena->next = *arena_list;
    *arena_list = new_arena;

    // Allocate from the new arena
    return arena_alloc(new_arena, size);
}