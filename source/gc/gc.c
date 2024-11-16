#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include "gc.h"
#include "mark.h"
#include "../utils/log.h"
#include "../errors/errors.h"

void gc_log(const char * fmt, ...) {
    char temp[1024];
    va_list vl;
    va_start(vl, fmt);
    vsnprintf(temp, sizeof(temp), fmt, vl);
    va_end(vl);

    fprintf(stdout, "%s", temp);
}




TypeV_NurseryRegion* gc_create_nursery() {
    // Allocate memory for the nursery region structure
    TypeV_NurseryRegion* nursery = (TypeV_NurseryRegion*)malloc(sizeof(TypeV_NurseryRegion));
    if (!nursery) return NULL;

    // Allocate and initialize the nursery data buffer
    nursery->data = (uint8_t*)aligned_alloc(NURSERY_SIZE, NURSERY_SIZE);
    if (!nursery->data) {
        free(nursery);
        return NULL;
    }

    // Split the nursery into `from` and `to` spaces, each half of NURSERY_SIZE
    nursery->from = nursery->data;
    nursery->to = nursery->data + (NURSERY_SIZE / 2);
    nursery->cellSize = 0;

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
    old_gen->color_bitmap = (uint8_t*)malloc(((INITIAL_OLD_CELLS * 2) / 8)*sizeof(uint8_t));
    old_gen->active_bitmap = (uint8_t*)malloc((INITIAL_OLD_CELLS / 8)*sizeof(uint8_t));
    old_gen->data = (uint8_t*) malloc(OLD_REGION_INITIAL_SIZE);
    if (!old_gen->data) {
        free(old_gen);
        return NULL;
    }

    old_gen->cellSize = 0;
    old_gen->capacityFactor = 1;

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
    gc->updateListSize = 0;

    return gc;
}

TypeV_ObjectHeader *gc_alloc(TypeV_Core* core, size_t size) {
    TypeV_GC *gc = core->gc;
    TypeV_NurseryRegion *nursery = gc->nurseryRegion;

    // Calculate the number of cells required, rounding up to the nearest cell cellSize
    size_t cells_needed = (size + CELL_SIZE - 1) / CELL_SIZE;

    // Ensure that there is sufficient space in the `from` space for allocation
    if ((cells_needed + nursery->cellSize) > (NURSERY_MAX_CELLS)) {
        // Not enough space in the nursery `from` space; trigger garbage collection or promotion
        gc_minor_gc(core);
        //gc_debug_nursery(gc);

        // make sure we have enough space now

        return gc_alloc(core, size);
    }

    // Calculate the starting address of the allocation based on the current end of used space
    TypeV_ObjectHeader *alloc_address = (TypeV_ObjectHeader*)(nursery->from + (nursery->cellSize * CELL_SIZE));
    alloc_address->totalSize = size;

    // Update the cellSize in cells to reflect the new allocation, without modifying `from`
    nursery->cellSize += cells_needed;

    // Set each cell in the newly allocated space as active and initialize color to WHITE
    for (size_t i = nursery->cellSize - cells_needed; i < nursery->cellSize; i++) {
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
    TypeV_OldGenerationRegion *old = gc->oldRegion;


    // Step 1: Mark all reachable objects in the nursery
    gc_mark_state(core, core->funcState, 1); // This function would traverse root objects and mark reachable ones

    // printf("Nursery after marking\n");
    //gc_debug_nursery(gc);

    // Step 2: Copy live objects from `from` to `to`
    uint8_t* new_position_in_nursery = nursery->to;  // Start copying to the beginning of `to`
    uint8_t* new_position_in_old = gc->oldRegion->data + gc->oldRegion->cellSize * CELL_SIZE;  // Start copying to the end of `old` space
    for (size_t i = 0; i < nursery->cellSize; i++) {
        //printf("Current update list cellSize: %llu\n", gc->updateListSize);
        if (GET_ACTIVE(nursery->active_bitmap, i) && GET_COLOR(nursery->color_bitmap, i) == BLACK) {

            // Copy live object to the `to` space
            TypeV_ObjectHeader* obj = (TypeV_ObjectHeader*)(nursery->from + (i * CELL_SIZE));
            size_t object_cell_size = (obj->totalSize + CELL_SIZE - 1) / CELL_SIZE; // Calculate cellSize in cells

            if(i + object_cell_size > NURSERY_MAX_CELLS) {
                core_panic(core, RT_ERROR_NURSERY_FULL, "Nursery cellSize exceeded the maximum limit");
            }

            obj->survivedCount++;

            if(obj->survivedCount >= 3) {
                // Move to old generation

                //printf("moving object %p to old generation\n", obj);

                memcpy(new_position_in_old, nursery->from + (i * CELL_SIZE), object_cell_size * CELL_SIZE);

                // Update block metadata in `to` space
                size_t new_offset = (new_position_in_old - old->data) / CELL_SIZE;

                for (size_t j = 0; j < object_cell_size; j++) {
                    SET_ACTIVE(old->active_bitmap, new_offset + j, 1);
                    SET_COLOR(old->color_bitmap, new_offset + j, BLACK);  // Mark as BLACK in `to`
                }

                // Update the pointer and position
                gc_update_object_pointers(core, (uintptr_t)obj, (uintptr_t)new_position_in_old);
                new_position_in_old += object_cell_size * CELL_SIZE;
            }
            else {
                // Keep in the nursery, move to other half

                if(i + object_cell_size > NURSERY_MAX_CELLS) {
                    core_panic(core, RT_ERROR_NURSERY_FULL, "Nursery cellSize exceeded the maximum limit");
                }

                // gc_log("Copying object %p to %p\n", obj, new_position_in_nursery);
                memcpy(new_position_in_nursery, nursery->from + (i * CELL_SIZE), object_cell_size * CELL_SIZE);

                // Update block metadata in `to` space
                size_t new_offset = (new_position_in_nursery - nursery->to) / CELL_SIZE;

                // TODO: replace with memset?
                for (size_t j = 0; j < object_cell_size; j++) {
                    SET_ACTIVE(nursery->active_bitmap, new_offset + j, 1);
                    SET_COLOR(nursery->color_bitmap, new_offset + j, BLACK);  // Mark as BLACK in `to`
                }

                // Update the pointer and position
                gc_update_object_pointers(core, (uintptr_t) obj, (uintptr_t) new_position_in_nursery);
                new_position_in_nursery += object_cell_size * CELL_SIZE;
            }

            i += object_cell_size - 1;  // Skip the copied object in the `from` space
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

    // Step 6: Update the nursery cellSize to reflect the compacted objects
    nursery->cellSize = (new_position_in_nursery - nursery->from) / CELL_SIZE;
    old->cellSize = (new_position_in_old - old->data) / CELL_SIZE;

    //gc_debug_nursery(gc);
    // set the active bits for the new space
    for (size_t i = 0; i < nursery->cellSize; i++) {
        SET_COLOR(nursery->color_bitmap, i, WHITE);
        SET_ACTIVE(nursery->active_bitmap, i, 1);
    }
    //gc_debug_nursery(gc);

    if(nursery->cellSize > NURSERY_MAX_CELLS) {
        core_panic(core, RT_ERROR_NURSERY_FULL, "Nursery cellSize exceeded the maximum limit");
    }

    core->gc->minorGCCount++;
    core->gc->updateListSize = 0;

    // extend the old generation if needed
    //gc_extend_old(core);
}

void gc_extend_old(TypeV_Core* core) {
    TypeV_GC* gc = core->gc;
    TypeV_OldGenerationRegion* old = gc->oldRegion;

    if(old->cellSize >= INITIAL_OLD_CELLS - 10) {
        core_panic(core, RT_ERROR_OOM, "Old generation cellSize exceeded the maximum limit");
    }

}


void gc_update_object_pointers(TypeV_Core* core, uintptr_t old_address, uintptr_t new_address) {
    if (core->gc->updateListSize >= NURSERY_MAX_CELLS) {
        core_panic(core, RT_ERROR_OOM, "Exceeded nursery update list capacity");
        return;
    }

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
            // Ensure `right` does not underflow
            if (mid == 0) {
                break; // Prevent underflow of `right`
            }
            right = mid - 1;
        }
    }

    // gc_log("Failed to find new address for %p\n", oldPtr);
    return oldPtr;
}




void gc_debug_nursery(TypeV_GC* gc) {
    TypeV_NurseryRegion* nursery = gc->nurseryRegion;
    printf("\n\n\tNursery Base Address: %p\n", nursery->data);
    printf("+--------+-----------------+----------+--------+\n");
    printf("| CellID |      Color      |   State  |  Value |\n");

    for (uint64_t i = 0; i < NURSERY_MAX_CELLS; i++) {
        uint8_t isActive = GET_ACTIVE(nursery->active_bitmap, i);
        uint8_t color = GET_COLOR(nursery->color_bitmap, i);

        const char* colorString = color == WHITE ? "white" :
                                  color == GRAY  ? "gray"  :
                                  color == BLACK ? "black" : "unknown";

        const char* activeString = isActive ? "busy" : "free";

        // Read the value at the cell location as uint8_t
        uint8_t cellValue = ((uint8_t*)nursery->data)[i * CELL_SIZE];

        printf("| %-6llu | %-15s | %-8s | %-6u | %c \n", i, colorString, activeString, cellValue, nursery->cellSize >= i ? '<' : ' ');
    }
    printf("+--------+-----------------+----------+--------+\n\n\n");
}

