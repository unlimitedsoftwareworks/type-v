#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#define ARENA_SIZE (2 * 1024 * 1024) // 2 MB arena size

// Arena structure to manage the entire memory pool
typedef struct Arena {
    uint8_t data[ARENA_SIZE];   // Data section for allocations
    size_t offset;              // Offset to the first free slot
    struct Arena* next;         // Pointer to the next arena in the list (for managing multiple arenas)
} Arena;

// Function to create a new arena
Arena* create_arena() {
    Arena* arena = (Arena*)malloc(sizeof(Arena));
    if (arena == NULL) {
        fprintf(stderr, "Failed to allocate memory for arena\n");
        return NULL;
    }

    // Initialize the arena
    arena->offset = 0;
    arena->next = NULL;

    return arena;
}

// Function to allocate memory from the arena
void* arena_alloc(Arena* arena, size_t size) {
    if (size == 0 || size > ARENA_SIZE) {
        fprintf(stderr, "Invalid allocation size\n");
        return NULL;
    }

    // Check if there is enough space in the current arena
    if (arena->offset + size > ARENA_SIZE) {
        fprintf(stderr, "Failed to allocate memory from arena\n");
        return NULL;
    }

    // Allocate memory by bumping the offset
    void* ptr = &arena->data[arena->offset];
    arena->offset += size;

    return ptr;
}

// Function to free memory back to the arena (simplified)
void arena_free(Arena* arena, void* ptr, size_t size) {
    // This allocator does not support freeing individual allocations.
    // Memory is reclaimed when the entire arena is destroyed.
    (void)arena; // Suppress unused parameter warning
    (void)ptr;
    (void)size;
    //fprintf(stderr, "Freeing individual allocations is not supported.\n");
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
