#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <mimalloc.h>
#include "allocator.h"
// Function to create a new arena
Arena* create_arena() {
    Arena* arena = (Arena*)mi_malloc_aligned(sizeof(Arena), 16);
    if (arena == NULL) {
        //fprintf(stderr, "Failed to allocate memory for arena\n");
        return NULL;
    }

    // Initialize the arena
    arena->offset = 0;
    arena->next = NULL;
    arena->skip_count = 0;
    arena->usable = true;

    return arena;
}

// Function to allocate memory from the arena

// Function to allocate memory from the arena
void* arena_alloc(Arena* arena, size_t size) {
    if (__builtin_expect(size == 0 || size > ARENA_SIZE, 0)) {
        //fprintf(stderr, "Invalid allocation size\n");
        return NULL;
    }

    // Check if there is enough space in the current arena
    if (__builtin_expect(arena->offset + size > ARENA_SIZE, 0)) {
        //fprintf(stderr, "Failed to allocate memory from arena\n");
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
    mi_free(arena);
}

// Function to manage multiple arenas for more flexible allocation
void* allocate_from_arenas(Arena** arena_list, size_t size) {
    Arena* current = *arena_list;
    Arena* prev = NULL;

    // Try to allocate from an existing arena
    while (current != NULL) {
        void* ptr = arena_alloc(current, size);
        if (ptr != NULL) {
            return ptr; // Allocation successful
        }

        // Increment skip count if the arena is skipped
        if (current->usable) {
            current->skip_count++;
            if (current->skip_count >= 3) {
                current->usable = false; // Mark the arena as unusable after being skipped too many times

                // Remove the arena from the usable list
                if (prev == NULL) {
                    *arena_list = current->next;
                } else {
                    prev->next = current->next;
                }


                current = (prev == NULL) ? *arena_list : prev->next;
                continue;
            }
        }

        prev = current;
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