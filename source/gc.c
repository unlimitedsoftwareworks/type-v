#include <stdlib.h>
#include <stdio.h>
#include <mimalloc.h>
#include <string.h>
#include "gc.h"
#include "allocator/allocator.h"
#include "utils/log.h"

void gc_log(const char * fmt, ...) {
    char temp[1024];
    va_list vl;
    va_start(vl, fmt);
    vsnprintf(temp, sizeof(temp), fmt, vl);
    va_end(vl);

    fprintf(stdout, "%s", temp);
}

#include "gc.h"

static inline TypeV_ObjectHeader* gc_ptr_in_nursery(uintptr_t ptr, TypeV_NurseryRegion* nursery) {
    // Check if the pointer falls within the nursery space (either `from` or `to`)
    uintptr_t headerPtr = ptr - sizeof(TypeV_ObjectHeader);

    // First, ensure `headerPtr` is within valid nursery bounds
    if (headerPtr < (uintptr_t)nursery->from || headerPtr >= (uintptr_t)(nursery->from + NURSERY_SIZE)) {
        return NULL; // Not within the nursery bounds
    }

    // Check if `headerPtr` is in `from` space
    if (headerPtr >= (uintptr_t)nursery->from && headerPtr < (uintptr_t)(nursery->from + NURSERY_SIZE)) {
        size_t offset = (headerPtr - (uintptr_t)nursery->from) / CELL_SIZE;
        if (GET_ACTIVE(nursery->active_bitmap, offset)) {
            return (TypeV_ObjectHeader*)headerPtr;
        }
    }

    return NULL; // Pointer is not within the nursery `from` or `to` space
}

TypeV_ObjectHeader* gc_mark_nursery_ptr(uintptr_t ptr, TypeV_NurseryRegion* nursery, ObjectColor color) {
    uintptr_t headerPtr = ptr - sizeof(TypeV_ObjectHeader);

    // Ensure `headerPtr` falls within the nursery bounds
    if (headerPtr < (uintptr_t)nursery->from || headerPtr >= (uintptr_t)(nursery->from + NURSERY_SIZE)) {
        return NULL; // Not within the nursery bounds
    }


    size_t offset = 0;
    // Otherwise, check if `headerPtr` is in `to` space
    if (headerPtr >= (uintptr_t)nursery->from && headerPtr < (uintptr_t)(nursery->from + NURSERY_SIZE / 2)) {
        offset = (headerPtr - (uintptr_t) nursery->from) / CELL_SIZE;
    }
    else if (headerPtr >= (uintptr_t)nursery->to && headerPtr < (uintptr_t)(nursery->to + NURSERY_SIZE / 2)) {
        offset = (headerPtr - (uintptr_t) nursery->to) / CELL_SIZE;
    }

    if (GET_ACTIVE(nursery->active_bitmap, offset)) {
        TypeV_ObjectHeader* obj = (TypeV_ObjectHeader*)headerPtr;
        size_t cells_needed = (obj->totalSize + CELL_SIZE - 1) / CELL_SIZE;
        for(size_t i = 0; i < cells_needed; i++) {
            SET_COLOR(nursery->color_bitmap, offset + i, color);
        }
        return obj;
    }

    return NULL; // Pointer is not within the active regions of `from` or `to`
}


void gc_mark_header(TypeV_ObjectHeader* headerPtr, TypeV_NurseryRegion* nursery, ObjectColor color) {
    // Calculate the offset in cells to locate the bitmap index
    size_t offset = 0;
    // Otherwise, check if `headerPtr` is in `to` space
    if ((uintptr_t )headerPtr >= (uintptr_t)nursery->from && (uintptr_t )headerPtr < (uintptr_t)(nursery->from + NURSERY_SIZE / 2)) {
        offset = ((uintptr_t )headerPtr - (uintptr_t) nursery->from) / CELL_SIZE;
    }
    else if ((uintptr_t )headerPtr >= (uintptr_t)nursery->to && (uintptr_t )headerPtr < (uintptr_t)(nursery->to + NURSERY_SIZE / 2)) {
        offset = ((uintptr_t )headerPtr - (uintptr_t) nursery->to) / CELL_SIZE;
    }

    // Set the color for the block at this offset
    SET_COLOR(nursery->color_bitmap, offset, color);
}



TypeV_NurseryRegion* gc_create_nursery() {
    // Allocate memory for the nursery region structure
    TypeV_NurseryRegion* nursery = (TypeV_NurseryRegion*)malloc(sizeof(TypeV_NurseryRegion));
    if (!nursery) return NULL;

    // Allocate and initialize the nursery data buffer
    nursery->data = (uint8_t*)malloc(NURSERY_SIZE);
    if (!nursery->data) {
        free(nursery);
        return NULL;
    }

    // Split the nursery into `from` and `to` spaces, each half of NURSERY_SIZE
    nursery->from = nursery->data;
    nursery->to = nursery->data + (NURSERY_SIZE / 2);
    nursery->size = 0;

    // Initialize color and active bitmaps to 0 (all cells white and inactive)
    memset(nursery->color_bitmap, 0, sizeof(nursery->color_bitmap));
    memset(nursery->active_bitmap, 0, sizeof(nursery->active_bitmap));

    return nursery;
}

TypeV_OldGenerationRegion* gc_create_old_generation() {
    // Allocate memory for the old generation region structure
    TypeV_OldGenerationRegion* old_gen = (TypeV_OldGenerationRegion*)malloc(sizeof(TypeV_OldGenerationRegion));
    if (!old_gen) return NULL;

    // Allocate and initialize the old generation data buffer
    old_gen->data = (uint8_t*)malloc(REGION_SIZE);
    if (!old_gen->data) {
        free(old_gen);
        return NULL;
    }

    old_gen->size = REGION_SIZE;

    // Initialize color and active bitmaps to 0 (all cells white and inactive)
    memset(old_gen->color_bitmap, 0, sizeof(old_gen->color_bitmap));
    memset(old_gen->active_bitmap, 0, sizeof(old_gen->active_bitmap));

    return old_gen;
}

TypeV_GC* gc_initialize() {
    // Initialize the entire GC system structure
    TypeV_GC* gc = (TypeV_GC*)malloc(sizeof(TypeV_GC));
    if (!gc) return NULL;

    gc->nurseryRegion = gc_create_nursery();
    if (!gc->nurseryRegion) {
        free(gc);
        return NULL;
    }

    gc->oldRegion = gc_create_old_generation();
    if (!gc->oldRegion) {
        free(gc->nurseryRegion->data);
        free(gc->nurseryRegion);
        free(gc);
        return NULL;
    }

    gc->minorGCCount = 0;

    return gc;
}

TypeV_ObjectHeader *gc_alloc(TypeV_Core* core, size_t size) {
    TypeV_GC *gc = core->gc;
    TypeV_NurseryRegion *nursery = gc->nurseryRegion;

    // Calculate the number of cells required, rounding up to the nearest cell size
    size_t cells_needed = (size + CELL_SIZE - 1) / CELL_SIZE;

    // Ensure that there is sufficient space in the `from` space for allocation
    if ((cells_needed + nursery->size) > (MAX_CELLS)) {
        // Not enough space in the nursery `from` space; trigger garbage collection or promotion
        gc_minor_gc(core);
        //gc_debug_nursery(gc);
        return gc_alloc(core, size);
    }

    // Calculate the starting address of the allocation based on the current end of used space
    TypeV_ObjectHeader *alloc_address = (TypeV_ObjectHeader*)(nursery->from + (nursery->size * CELL_SIZE));
    alloc_address->totalSize = size;

    // Update the size in cells to reflect the new allocation, without modifying `from`
    nursery->size += cells_needed;

    // Set each cell in the newly allocated space as active and initialize color to WHITE
    for (size_t i = nursery->size - cells_needed; i < nursery->size; i++) {
        SET_ACTIVE(nursery->active_bitmap, i, 1);
        SET_COLOR(nursery->color_bitmap, i, WHITE);
    }

    // Return the address of the allocated memory block with metadata header
    return alloc_address;
}

void gc_minor_gc(TypeV_Core* core) {
    //printf("Performing minor GC\n");
    TypeV_GC *gc = core->gc;
    TypeV_NurseryRegion *nursery = gc->nurseryRegion;


    // Step 1: Mark all reachable objects in the nursery
    gc_mark_state(core, core->funcState, 1); // This function would traverse root objects and mark reachable ones


    //gc_debug_nursery(gc);

    // Step 2: Copy live objects from `from` to `to`
    uint8_t* new_position = nursery->to;  // Start copying to the beginning of `to`
    for (size_t i = 0; i < nursery->size; i++) {
        if (GET_ACTIVE(nursery->active_bitmap, i) && GET_COLOR(nursery->color_bitmap, i) == BLACK) {

            // Copy live object to the `to` space
            TypeV_ObjectHeader* obj = (TypeV_ObjectHeader*)(nursery->from + (i * CELL_SIZE));
            size_t object_size = (obj->totalSize + CELL_SIZE - 1) / CELL_SIZE; // Calculate size in cells
            // gc_log("Copying object %p to %p\n", obj, new_position);
            memcpy(new_position, nursery->from + (i * CELL_SIZE), object_size * CELL_SIZE);

            // Update block metadata in `to` space
            size_t new_offset = (new_position - nursery->to) / CELL_SIZE;
            for (size_t j = 0; j < object_size; j++) {
                SET_ACTIVE(nursery->active_bitmap, new_offset + j, 1);
                SET_COLOR(nursery->color_bitmap, new_offset + j, BLACK);  // Mark as BLACK in `to`
            }

            // Update the pointer and position
            gc_update_object_pointers(core, (uintptr_t)obj, (uintptr_t)new_position);
            new_position += object_size * CELL_SIZE;

            i += object_size - 1;  // Skip the copied object in the `from` space
        }
    }

    // Step 3: Update pointers in the stack and registers
    gc_update_state(core, core->funcState, 1);

    // Step 4: Swap `from` and `to` pointers
    uint8_t* temp = nursery->from;
    nursery->from = nursery->to;
    nursery->to = temp;

    // Step 5: Reset metadata for new `to` space
    memset(nursery->active_bitmap, 0, sizeof(nursery->active_bitmap));
    memset(nursery->color_bitmap, 0, sizeof(nursery->color_bitmap));

    // Step 6: Update the nursery size to reflect the compacted objects
    nursery->size = (new_position - nursery->from) / CELL_SIZE;

    // set the active bits for the new space
    for (size_t i = 0; i < nursery->size; i++) {
        SET_COLOR(nursery->color_bitmap, i, WHITE);
        SET_ACTIVE(nursery->active_bitmap, i, 1);
    }


    if(nursery->size > MAX_CELLS) {
        core_panic(core, -1, "Nursery size exceeded the maximum limit");
    }

    core->gc->minorGCCount++;
    core->gc->updateListSize = 0;
}



void core_gc_mark_nursery_object(TypeV_Core* core, uintptr_t ptr) {
    // printf("Marking object %p\n", header);
    if (!ptr) {
        return;  // Already marked, or null
    }

    // Mark the current object in its arena
    TypeV_ObjectHeader* header = gc_mark_nursery_ptr(ptr, core->gc->nurseryRegion, GRAY);

    if(header!=NULL) {
        // gc_log("Marking object %p with header %p\n", ptr, (header));
    }
    if(!header) {
        return;
    }


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
                    uintptr_t dest;
                    memcpy(&dest, struct_ptr->data + struct_ptr->fieldOffsets[i], sizeof(uintptr_t));

                    core_gc_mark_nursery_object(core, (uintptr_t )dest);
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

                    uintptr_t dest;
                    memcpy(&dest, (class_ptr->data + class_ptr->fieldOffsets[i]), sizeof(uintptr_t));

                    core_gc_mark_nursery_object(core, (uintptr_t )dest);
                }
            }
            break;
        }
        case OT_ARRAY: {
            TypeV_Array *array_ptr = (TypeV_Array *) (header + 1);
            if (array_ptr->isPointerContainer) {
                for (size_t i = 0; i < array_ptr->length; i++) {
                    uintptr_t dest;
                    memcpy(&dest, (array_ptr->data + i * array_ptr->elementSize), sizeof(uintptr_t));
                    core_gc_mark_nursery_object(core, (uintptr_t )dest);
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

    gc_mark_header(header, core->gc->nurseryRegion, BLACK);
}

void gc_mark_state(TypeV_Core* core, TypeV_FuncState* state, uint8_t inNursery) {
    if(inNursery) {
        for(uint32_t i = 0; i < MAX_REG; i++) {
            uintptr_t ptr = state->regs[i].ptr;
            core_gc_mark_nursery_object(core, ptr);
        }
    }
    else {
        core_panic(core, -1, "Not implemented");
    }
    // if previous state exists, collect it
    if(state->prev != NULL) {
        gc_mark_state(core, state->prev, inNursery);
    }
}

void gc_update_object_pointers(TypeV_Core* core, uintptr_t old_address, uintptr_t new_address) {
    core->gc->nurseryUpdateList[core->gc->updateListSize].old_address = old_address;
    core->gc->nurseryUpdateList[core->gc->updateListSize++].new_address = new_address;
}

uintptr_t gc_get_new_ptr_location(TypeV_Core* core, uintptr_t oldPtr) {
    if(core->gc->updateListSize == 0) {
        // gc_log("Failed to find new address for %p, 0 updates to be made\n", oldPtr);
        return oldPtr;
    }

    if (core->gc->updateListSize == 1) {
        if (core->gc->nurseryUpdateList[0].old_address == oldPtr) {
            return core->gc->nurseryUpdateList[0].new_address;
        } else {
            //core_panic(core, -1, "Failed to find new address for %p", oldPtr);
            // gc_log("Failed to find new address for %p\n", oldPtr);
            return oldPtr;
        }
    }

    uint64_t left = 0;
    uint64_t right = core->gc->updateListSize - 1;

    while (left <= right) {
        uint64_t mid = left + (right - left) / 2;
        if (core->gc->nurseryUpdateList[mid].old_address == oldPtr) {
            return core->gc->nurseryUpdateList[mid].new_address;
        }
        if (core->gc->nurseryUpdateList[mid].old_address < oldPtr) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    // gc_log("Failed to find new address for %p\n", oldPtr);
    return oldPtr;
}



void* core_gc_update_object_reference_nursery(TypeV_Core* core, TypeV_ObjectHeader* old_header) {
    // printf("Checking for object updates %p\n", header);
    if (!old_header) {
        return NULL;  // Already marked, or null
    }

    // Mark the current object in its arena
    TypeV_ObjectHeader* new_ptr = (TypeV_ObjectHeader*)gc_get_new_ptr_location(core, (uintptr_t)old_header);

    if(new_ptr == NULL) {
        return NULL;
    }

    // If the object is a struct, mark its fields
    switch (new_ptr->type) {
        case OT_STRUCT: {
            TypeV_Struct* struct_ptr = (TypeV_Struct*)(new_ptr + 1);
            size_t bitmaskSize = (struct_ptr->numFields + 7) / 8;

            size_t totalAllocationSize = sizeof(TypeV_ObjectHeader) + sizeof(TypeV_Struct)
                                         + bitmaskSize  // Pointer bitmask
                                         // Data block size
                                         + struct_ptr->numFields * sizeof(uint16_t)  // fieldOffsets array
                                         + struct_ptr->numFields * sizeof(uint32_t); // globalFields array

            size_t dataSize = new_ptr->totalSize - totalAllocationSize;

            void* newAddr = (uint8_t *)(struct_ptr + 1);

            struct_ptr->fieldOffsets = (uint16_t*)(struct_ptr->data + dataSize);
            struct_ptr->globalFields = (uint32_t*)(struct_ptr->fieldOffsets + struct_ptr->numFields);
            struct_ptr->pointerBitmask = (uint8_t*)(struct_ptr->globalFields+struct_ptr->numFields);

            for (size_t i = 0; i < struct_ptr->numFields; i++) {
                // Determine which byte and which bit correspond to the current field index `i`
                size_t byteIndex = i / 8;       // Which byte contains the bit for the field at index `i`
                uint8_t bitOffset = i % 8;      // Which bit within that byte corresponds to field `i`

                // Check if the bit corresponding to field `i` is set
                if (struct_ptr->pointerBitmask[byteIndex] & (1 << bitOffset)) {
                    // struct fields needs to be updated

                    uintptr_t fieldPtr;
                    memcpy(&fieldPtr, struct_ptr->data + struct_ptr->fieldOffsets[i], sizeof(uintptr_t));

                    TypeV_ObjectHeader* fieldHeader = gc_ptr_in_nursery(fieldPtr, core->gc->nurseryRegion);

                    void* res = core_gc_update_object_reference_nursery(core, fieldHeader);
                    if(res != NULL) {
                        memcpy((char*)struct_ptr->data + struct_ptr->fieldOffsets[i], &res, sizeof(void*));
                    }
                }
            }

            break;
        }

        case OT_CLASS: {
            TypeV_Class *class_ptr = (TypeV_Class *) (new_ptr + 1);
            for (size_t i = 0; i < class_ptr->numFields; i++) {
                // Determine which byte and which bit correspond to the current field index `i`
                size_t byteIndex = i / 8;       // Which byte contains the bit for the field at index `i`
                uint8_t bitOffset = i % 8;      // Which bit within that byte corresponds to field `i`

                // Check if the bit corresponding to field `i` is set
                if (class_ptr->pointerBitmask[byteIndex] & (1 << bitOffset)) {
                    uintptr_t fieldPtr = (uintptr_t)(class_ptr->data + class_ptr->fieldOffsets[i]);
                    TypeV_ObjectHeader* fieldHeader = gc_ptr_in_nursery(fieldPtr, core->gc->nurseryRegion);

                    void* res = core_gc_update_object_reference_nursery(core, fieldHeader);
                    if(res != NULL) {
                        memcpy((void*)fieldPtr, &res, sizeof(void*));
                    }
                }
            }
            break;
        }
        case OT_ARRAY: {
            TypeV_Array *array_ptr = (TypeV_Array *) (new_ptr + 1);
            if (array_ptr->isPointerContainer) {
                for (size_t i = 0; i < array_ptr->length; i++) {
                    uintptr_t fieldPtr = (uintptr_t)(array_ptr->data + i * array_ptr->elementSize);
                    TypeV_ObjectHeader* fieldHeader = gc_ptr_in_nursery(fieldPtr, core->gc->nurseryRegion);

                    void* res = core_gc_update_object_reference_nursery(core, fieldHeader);
                    if(res != NULL) {
                        memcpy((void*)fieldPtr, &res, sizeof(void*));
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

    if(new_ptr != NULL) {
        return new_ptr + 1;
    }
    return NULL;
}

void gc_update_state(TypeV_Core* core, TypeV_FuncState* state, uint8_t inNursery) {
    if(inNursery) {
        for(uint32_t i = 0; i < MAX_REG; i++) {
            uintptr_t ptr = state->regs[i].ptr;
            TypeV_ObjectHeader* old_header = gc_ptr_in_nursery(ptr, core->gc->nurseryRegion);
            if(old_header) {
                void *val = core_gc_update_object_reference_nursery(core, old_header);
                if (val != NULL) {
                    state->regs[i].ptr = (uintptr_t) val;
                }
            }
        }
    }


    // if previous state exists, collect it
    if(state->prev != NULL) {
        gc_update_state(core, state->prev, inNursery);
    }
}


void gc_debug_nursery(TypeV_GC* gc) {
    TypeV_NurseryRegion* nursery = gc->nurseryRegion;
    printf("\n\n\tNursery Base Address: %p\n", nursery->data);
    printf("+--------+-----------------+----------+--------+\n");
    printf("| CellID |      Color      |   State  |  Value |\n");

    for (uint64_t i = 0; i < MAX_CELLS; i++) {
        uint8_t isActive = GET_ACTIVE(nursery->active_bitmap, i);
        uint8_t color = GET_COLOR(nursery->color_bitmap, i);

        const char* colorString = color == WHITE ? "white" :
                                  color == GRAY  ? "gray"  :
                                  color == BLACK ? "black" : "unknown";

        const char* activeString = isActive ? "busy" : "free";

        // Read the value at the cell location as uint8_t
        uint8_t cellValue = ((uint8_t*)nursery->data)[i * CELL_SIZE];

        printf("| %-6llu | %-15s | %-8s | %-6u | %c \n", i, colorString, activeString, cellValue, nursery->size >= i ? '<' : ' ');
    }
    printf("+--------+-----------------+----------+--------+\n\n\n");
}

