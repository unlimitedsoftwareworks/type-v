#include <stdlib.h>
#include <stdio.h>
#include <mimalloc.h>
#include <string.h>
#include "core.h"
#include "allocator/allocator.h"
#include "gc.h"
#include "utils/log.h"

void* core_gc_alloc(TypeV_Core* core, size_t size) {
   
    void* mem = (void*)tv_gc_alloc(core->gc.colosseum, size);

    if(core->gc.colosseum->busyCount > 10){
        printf("Collecting garbage\n");
        core_gc_collect(core);
        core->gc.colosseum->busyCount = 0;
    }

    return mem;
}

TypeV_ObjectHeader* get_header_from_pointer(void* objectPtr) {
    const size_t HEADER_SIZE = sizeof(TypeV_ObjectHeader);

    if (objectPtr == NULL || ((uintptr_t)objectPtr <= HEADER_SIZE)) {
        return NULL;
    }

    // The header is just before the object memory in the allocation.
    return (TypeV_ObjectHeader*)((char*)objectPtr - HEADER_SIZE);
}

void core_gc_mark_object(TypeV_Core* core, TypeV_GCArena* arena, TypeV_ObjectHeader* header) {
    printf("Marking object %p\n");
    if (header == NULL) {
        return;  // Already marked, or null
    }

    // Mark the current object in its arena
    tv_arena_mark_ptr(arena, (uintptr_t)header);

    // If the object is a struct, mark its fields
    switch (header->type) {
        case OT_STRUCT: {
            TypeV_Struct* struct_ptr = (TypeV_Struct*)(header + 1);

            for (size_t i = 0; i < struct_ptr->numFields; i++) {
                // Determine which byte and which bit correspond to the current field index `i`
                size_t byteIndex = i / 8;       // Which byte contains the bit for the field at index `i`
                uint8_t bitOffset = i % 8;      // Which bit within that byte corresponds to field `i`

                // Check if the bit corresponding to field `i` is set
                if (struct_ptr->pointerBitmask[byteIndex] & (1 << bitOffset)) {
                    uintptr_t fieldPtr = (uintptr_t)(struct_ptr->data + struct_ptr->fieldOffsets[i]);
                    TypeV_ObjectHeader* fieldHeader = get_header_from_pointer((void*)fieldPtr);

                    // Find the arena that contains this field and mark it
                    TypeV_GCArena* fieldArena = tv_arena_find_pointerArena(core->gc.colosseum, (uintptr_t)fieldHeader);
                    if (fieldArena) {
                        core_gc_mark_object(core, fieldArena, fieldHeader);
                    }
                }
            }

            break;
        }

        case OT_CLASS: {
            TypeV_Class *class_ptr = (TypeV_Class *) (header + 1);
            for (size_t i = 0; i < class_ptr->numFields; i++) {
                // Determine which byte and which bit correspond to the current field index `i`
                size_t byteIndex = i / 8;       // Which byte contains the bit for the field at index `i`
                uint8_t bitOffset = i % 8;      // Which bit within that byte corresponds to field `i`

                // Check if the bit corresponding to field `i` is set
                if (class_ptr->pointerBitmask[byteIndex] & (1 << bitOffset)) {
                    uintptr_t fieldPtr = (uintptr_t)(class_ptr->data + class_ptr->fieldOffsets[i]);
                    TypeV_ObjectHeader* fieldHeader = get_header_from_pointer((void*)fieldPtr);

                    // Find the arena that contains this field and mark it
                    TypeV_GCArena* fieldArena = tv_arena_find_pointerArena(core->gc.colosseum, (uintptr_t)fieldHeader);
                    if (fieldArena) {
                        core_gc_mark_object(core, fieldArena, fieldHeader);
                    }
                }
            }
            break;
        }
        case OT_ARRAY: {
            TypeV_Array *array_ptr = (TypeV_Array *) (header + 1);
            if (array_ptr->isPointerContainer) {
                for (size_t i = 0; i < array_ptr->length; i++) {
                    uintptr_t fieldPtr = (uintptr_t)(array_ptr->data + i * array_ptr->elementSize);
                    TypeV_ObjectHeader* fieldHeader = get_header_from_pointer((void*)fieldPtr);

                    // Find the arena that contains this field and mark it
                    TypeV_GCArena* fieldArena = tv_arena_find_pointerArena(core->gc.colosseum, (uintptr_t)fieldHeader);
                    if (fieldArena) {
                        core_gc_mark_object(core, fieldArena, fieldHeader);
                    }
                }
            }
            break;
        }
        case OT_CLOSURE: {
            break;
        }
        case OT_COROUTINE: {
            break;
        }
        case OT_RAWMEM: {
            break;
        }
    }
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


void core_gc_sweep(TypeV_Core* core) {
    // Iterate through all full arenas and sweep them
    TypeV_GCArena* arena = core->gc.colosseum->busyHead;
    while (arena != NULL) {
        // Iterate through all cells in the arena
        for (size_t i = 0; i < COLESSEUM_NUM_CELLS; i++) {
            // Check if the cell is marked
            if (!is_bitmap_bit_set(arena->mark_bitmap, i) && is_bitmap_bit_set(arena->block_bitmap, i)) {
                // The cell is not marked, so it is garbage
                uintptr_t ptr = (uintptr_t)(arena->data + i * COLESSEUM_CELL_SIZE);
                TypeV_ObjectHeader* header = get_header_from_pointer((void*)ptr);

                // Free the object
                //core_gc_free_header(core, header);
                printf("freeing object %p of type %d\n", header, header->type);

                // Clear the bitmap bit for this cell
                clear_bitmap_bit(arena->block_bitmap, i);
            }
        }

        // Reset the mark bitmap for the next GC cycle
        memset(arena->mark_bitmap, 0, COLESSEUM_BITMAP_SIZE);

        // Move to the next arena
        arena = arena->prev;
    }
}

void core_gc_collect(TypeV_Core* core) {
    //LOG_WARN("Collecting state for core %d", core->id);
    // TODO: uncomment and identify bugs
    if(core->funcState->next != NULL) {
        core_gc_collect_state(core, core->funcState->next);
    }
    else {
        core_gc_collect_state(core, core->funcState);
    }
}


void core_gc_collect_state(TypeV_Core* core, TypeV_FuncState* state) {
    uint32_t busyCount = core->gc.colosseum->busyCount;
    for(uint32_t i = 0; i < MAX_REG; i++) {
        uintptr_t ptr = state->regs[i].ptr;
        // get the header
        TypeV_ObjectHeader *header = get_header_from_pointer((void*)ptr);

        if(header == NULL) {
            continue;
        }

        TypeV_GCArena* arena = tv_arena_find_pointerArena(core->gc.colosseum, (uintptr_t)header);
        if(arena) {
            core_gc_mark_object(core, arena, header);
        }
    }


    // if previous state exists, collect it
    if(state->prev != NULL) {
        core_gc_collect_state(core, state->prev);
    }
    else {
        core_gc_sweep(core);
    }
}

void core_gc_sweep_all(TypeV_Core* core){
}

void core_struct_free(TypeV_Core *core, TypeV_ObjectHeader* header) {
    // Ensure the header is valid and of the expected type
    if (!header || header->type != OT_STRUCT) {
        core_panic(core, -1, "Invalid header or type");
        return;
    }

    // Get the struct
    TypeV_Struct* struct_ptr = (TypeV_Struct*)(header + 1);

    // Free the fieldOffsets array in the struct
    if (struct_ptr->fieldOffsets) {
        mi_free(struct_ptr->fieldOffsets);
        struct_ptr->fieldOffsets = NULL;
    }

    // Free the globalFields array in the struct
    if (struct_ptr->globalFields) {
        mi_free(struct_ptr->globalFields);
        struct_ptr->globalFields = NULL;
    }

    // `data[]` is automatically freed as part of `header`,
    // since it was allocated in the same contiguous block.

    // Free the entire object header (and struct within it)
    mi_free(header);
}

void core_class_free(TypeV_Core *core, TypeV_ObjectHeader* header) {
    // Ensure the header is valid and of the expected type
    if (!header || header->type != OT_CLASS) {
        core_panic(core, -1, "Invalid header or type");
        return;
    }

    // Get a pointer to the class, which comes right after the header
    TypeV_Class* class_ptr = (TypeV_Class*)(header + 1);

    // Free the methods array in the class
    if (class_ptr->methods) {
        mi_free(class_ptr->methods);
        class_ptr->methods = NULL;
    }

    // Free the globalMethods array in the class
    if (class_ptr->globalMethods) {
        mi_free(class_ptr->globalMethods);
        class_ptr->globalMethods = NULL;
    }

    // `data[]` does not need to be freed separately, as it is part of the contiguous allocation

    // Free the entire object, including header and class
    mi_free(header);
}

void core_array_free(TypeV_Core *core, TypeV_ObjectHeader* header) {
    if (!header || header->type != OT_ARRAY) {
        LOG_ERROR("Invalid header or type for array deallocation");
        return;
    }

    // Access the array structure
    TypeV_Array* array_ptr = (TypeV_Array*)(header + 1);

    // Free the data array in TypeV_Array
    if (array_ptr->data && (array_ptr->length > 0)) {
        mi_free(array_ptr->data);
        array_ptr->data = NULL;
    }

    // Free the entire header (which includes the TypeV_Array structure)
    mi_free(header);
}

void core_closure_free(TypeV_Core *core, TypeV_ObjectHeader* header) {
    if (!header || header->type != OT_CLOSURE) {
        LOG_ERROR("Invalid header or type for closure deallocation");
        return;
    }

    // Access the closure structure
    TypeV_Closure* closure_ptr = (TypeV_Closure*)(header + 1);

    // Free the upvalues array in the closure
    if (closure_ptr->upvalues) {
        mi_free(closure_ptr->upvalues);
        closure_ptr->upvalues = NULL;
    }

    // Free the entire header (which includes the TypeV_Closure structure)
    mi_free(header);
}

void core_coroutine_free(TypeV_Core* core, TypeV_ObjectHeader* header) {
    if (!header || header->type != OT_COROUTINE) {
        LOG_ERROR("Invalid header or type for coroutine deallocation");
        return;
    }


    TypeV_Coroutine* coroutine_ptr = (TypeV_Coroutine*)(header + 1);

    // TODO: get the header of the closure!
    //core_closure_free(core, coroutine_ptr->closure);
    core_free_function_state(core, coroutine_ptr->state);

    mi_free(header);
}

void core_gc_free_header(TypeV_Core* core, TypeV_ObjectHeader* header) {
    switch(header->type) {
        case OT_STRUCT: {
            //printf("freeing struct\n");
            core_struct_free(core, header);
            break;
        }
        case OT_CLASS: {
            //printf("freeing class\n");
            core_class_free(core, header);
            break;
        }
        case OT_ARRAY: {
            //printf("freeing array\n");
            core_array_free(core, header);
            break;
        }
        case OT_CLOSURE:
            //printf("freeing closure\n");
            core_closure_free(core, header);
            break;
        case OT_COROUTINE:
            //printf("freeing coroutine\n");
            core_coroutine_free(core, header);
            break;
        case OT_RAWMEM:
            //printf("freeing rawmem\n");
            break;
    }
}