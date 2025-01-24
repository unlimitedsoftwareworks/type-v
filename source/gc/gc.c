#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gc.h"
#include "mark.h"

TypeV_GC* initialize_gc() {
    TypeV_GC* gc = (TypeV_GC*)malloc(sizeof(TypeV_GC));
    gc_log("initialize_gc: Initializing GC");
    gc->nursery.data = aligned_alloc(8, NURSERY_SIZE);
    gc->nursery.active_bitmap = (uint8_t*)calloc(NURSERY_MAX_CELLS / 8, sizeof(uint8_t));
    gc->nursery.from = gc->nursery.data;
    gc->nursery.to = gc->nursery.data + NURSERY_REGION_SIZE;
    gc->nursery.cell_size = 0;

    gc->oldRegion.capacity_factor = 1;
    gc->oldRegion.cell_size = 0;
    gc->oldRegion.data = (uint8_t*)aligned_alloc(8, OLD_REGION_INITIAL_SIZE);
    gc->oldRegion.active_bitmap = (uint8_t*)calloc(INITIAL_OLD_CELLS / 8, sizeof(uint8_t));
    gc->oldRegion.from = gc->oldRegion.data;
    gc->oldRegion.to = gc->oldRegion.data + OLD_REGION_INITIAL_SIZE;
    gc->oldRegion.direction = 1; // Start with downwards direction

    gc->rs.size = 0;
    gc->rs.capacity = 1024;
    gc->rs.set = (TypeV_ObjectHeader**)malloc(gc->rs.capacity * sizeof(TypeV_ObjectHeader*));

    gc_log("initialize_gc: GC initialized");

    return gc;
}

void* gc_alloc(TypeV_Core* core, size_t size) {
    TypeV_GC* gc = core->gc;
    size_t cellSize = (size + CELL_SIZE - 1) / CELL_SIZE;
    gc_log("gc_alloc: Requesting %zu bytes (%zu cells)", size, cellSize);

    while ((gc->nursery.cell_size + cellSize) > NURSERY_MAX_CELLS) {
        gc_log("gc_alloc: Insufficient space, triggering minor GC");
        perform_minor_gc(core);
        if ((gc->nursery.cell_size + cellSize) > NURSERY_MAX_CELLS) {
            gc_log("gc_alloc: Out of memory after minor GC, retrying allocation");
        }
    }

    TypeV_ObjectHeader* ptr = (TypeV_ObjectHeader *)(gc->nursery.from + (gc->nursery.cell_size * CELL_SIZE));
    gc->nursery.cell_size += cellSize;


    ptr->color = WHITE;
    ptr->totalSize = size;
    ptr->surviveCount = 0;
    ptr->fwd = NULL;
    ptr->location = 0; // Set location to nursery

    static uint32_t uid = 0;
    ptr->uid = uid++;

    gc_log("gc_alloc: Allocated at %p, new object %d, size %zu bytes (%zu cells)", (void*)ptr, ptr->uid, size, cellSize);

    return ptr;
}

static const char* object_names[] = {
        "Class",
        "Struct",
        "Array",
        "Closure",
        "Coroutine",
        "X",
        "Y"
};

void perform_minor_gc(TypeV_Core* core) {
    TypeV_GC* gc = core->gc;
    gc_log("perform_minor_gc: Starting minor GC");
    gc_log("perform_minor_gc: Checking old region usage");

    if ((INITIAL_OLD_CELLS*gc->oldRegion.capacity_factor-gc->oldRegion.cell_size ) <= NURSERY_MAX_CELLS ) {
        gc_log("perform_minor_gc: Old region is full %d, performing major GC", gc->oldRegion.cell_size);
        perform_major_gc(core);
    }

    perform_minor_mark(core);

    gc_log("minor_begin (%d/%d, %d/%d)\n", gc->nursery.cell_size, NURSERY_MAX_CELLS, gc->oldRegion.cell_size, INITIAL_OLD_CELLS*gc->oldRegion.capacity_factor);

    uint64_t i = 0;

    uint8_t* position_in_nursery = gc->nursery.to;
    uint8_t* position_in_old = gc->oldRegion.direction == 1 ?
                               gc->oldRegion.from+gc->oldRegion.cell_size*CELL_SIZE :
                               gc->oldRegion.to - gc->oldRegion.cell_size*CELL_SIZE
    ;

    size_t nursery_cell_size = 0;


    for(uint64_t k = 0; k < gc->rs.size; k++) {
        gc->rs.set[k]->color = BLACK;
        gc->rs.set[k]->surviveCount = 5;
    }
    gc->rs.size = 0;

    while (i < gc->nursery.cell_size) {
        TypeV_ObjectHeader* obj = (TypeV_ObjectHeader *)(gc->nursery.from + i * CELL_SIZE);
        size_t cellSize = (obj->totalSize + CELL_SIZE - 1) / CELL_SIZE;

        if (obj->color == BLACK) {
            TypeV_ObjectHeader* newLocation = NULL;
            uint8_t location = 0;
            if (obj->surviveCount >= PROMOTION_SURVIVAL_THRESHOLD) {
                if(gc->oldRegion.direction == 1) {
                    newLocation = (TypeV_ObjectHeader*)position_in_old;
                    position_in_old += cellSize * CELL_SIZE;
                } else {
                    position_in_old -= cellSize * CELL_SIZE;
                    newLocation = (TypeV_ObjectHeader*)position_in_old;
                }

                location = 1;
            } else {
                newLocation = (TypeV_ObjectHeader*)position_in_nursery;
                position_in_nursery += cellSize * CELL_SIZE;
                nursery_cell_size += cellSize;
            }

            memcpy(newLocation, obj, cellSize * CELL_SIZE);
            //newLocation->fields = (TypeV_ObjectHeader**)malloc(obj->num_fields * sizeof(TypeV_ObjectHeader*));
            //memcpy(newLocation->fields, obj->fields, obj->num_fields * sizeof(TypeV_ObjectHeader*));

            obj->fwd = newLocation;
            newLocation->fwd = NULL;
            newLocation->surviveCount = obj->surviveCount + 1;
            newLocation->location = location;
            newLocation->color = WHITE;

            if (newLocation->totalSize == 0) {
                printf("Error: %p\n", newLocation);
            }
        }
        else {
            gc_log("Freeing unmarked nursery object : %d / %s\n", obj->uid, object_names[obj->type]);
            if(obj->type == OT_USER_OBJECT) {
                TypeV_UserObject* user_object = (TypeV_UserObject*)(obj + 1);
                user_object->dealloc((void*)user_object->ptr);
            }
        }
        i += cellSize;
    }

    uint8_t* temp = gc->nursery.from;
    gc->nursery.from = gc->nursery.to;
    gc->nursery.to = temp;

    gc->nursery.cell_size = nursery_cell_size;
    gc->oldRegion.cell_size = (gc->oldRegion.direction == 1 ? position_in_old - gc->oldRegion.from : gc->oldRegion.to - (position_in_old)) / CELL_SIZE;


    update_root_references(core);

    gc_log("minor_end gc (%d/%d, %d/%d)\n", gc->nursery.cell_size, NURSERY_MAX_CELLS, gc->oldRegion.cell_size, INITIAL_OLD_CELLS*gc->oldRegion.capacity_factor);
    gc_log("perform_minor_gc: Completed minor GC");
}

void perform_major_gc(TypeV_Core* core) {
    TypeV_GC* gc = core->gc;
    gc_log("MAJOR_BEGIN (%d/%d, %d/%d)\n", gc->nursery.cell_size, NURSERY_MAX_CELLS, gc->oldRegion.cell_size, INITIAL_OLD_CELLS*gc->oldRegion.capacity_factor);
    // Step 1: Mark phase
    perform_major_mark(core);

    // Step 2: Check if the old region has enough space for the nursery
    size_t required_space = NURSERY_REGION_SIZE / CELL_SIZE;
    size_t current_free_space = INITIAL_OLD_CELLS - gc->oldRegion.cell_size;

    // if we dont have room for nursery or half old is full we scale up
    bool needs_new_buffer = (current_free_space < required_space) || (gc->oldRegion.cell_size >= (INITIAL_OLD_CELLS*gc->oldRegion.capacity_factor) / 2);

    uint8_t* new_buffer = NULL;
    size_t new_capacity = 0;

    uint8_t* from = gc->oldRegion.from;
    uint8_t* to = gc->oldRegion.to;

    // new dirty bitmap

    if (needs_new_buffer) {
        // Allocate a new buffer large enough to fit all objects + required free space
        gc->oldRegion.capacity_factor *= 2;
        new_capacity = INITIAL_OLD_CELLS * gc->oldRegion.capacity_factor;
        new_buffer = (uint8_t*)aligned_alloc(16, OLD_REGION_INITIAL_SIZE * gc->oldRegion.capacity_factor);
        gc_log("perform_major_gc: Allocated new buffer with capacity %zu cells", new_capacity);

        from = new_buffer;
        to = new_buffer + OLD_REGION_INITIAL_SIZE * gc->oldRegion.capacity_factor;

    }
    uint8_t* new_dirty_bitmap = (uint8_t*)calloc((INITIAL_OLD_CELLS*gc->oldRegion.capacity_factor) / 8, sizeof(uint8_t));

    uint64_t new_cell_size = 0;
    if (gc->oldRegion.direction == 1) {
        for (uint64_t i = 0; i < gc->oldRegion.cell_size; ) {
            TypeV_ObjectHeader* obj = (TypeV_ObjectHeader *)(gc->oldRegion.from + i * CELL_SIZE);
            size_t cellSize = (obj->totalSize + CELL_SIZE - 1) / CELL_SIZE;

            if (obj->color == BLACK) {
                obj->color = WHITE;

                // Determine the new location
                new_cell_size += cellSize;
                TypeV_ObjectHeader* new_location = (TypeV_ObjectHeader *)(to - (new_cell_size * CELL_SIZE));
                memcpy(new_location, obj, cellSize * CELL_SIZE);
                obj->fwd = new_location;
                new_location->fwd = NULL;

            } else {
                gc_log("Freeing unmarked old object: %d\n", obj->uid);
                if(obj->type == OT_USER_OBJECT) {
                    TypeV_UserObject* user_object = (TypeV_UserObject*)(obj + 1);
                    user_object->dealloc((void*)user_object->ptr);
                }
            }

            i += cellSize; // Move to the next object
        }
    } else {
        for (uint64_t i = 0; i < gc->oldRegion.cell_size; ) {
            TypeV_ObjectHeader* obj = (TypeV_ObjectHeader *)(gc->oldRegion.to - (gc->oldRegion.cell_size - i) * CELL_SIZE);
            size_t cellSize = (obj->totalSize + CELL_SIZE - 1) / CELL_SIZE;

            if (obj->color == BLACK) {
                obj->color = WHITE;

                // Determine the new location
                TypeV_ObjectHeader* new_location = (TypeV_ObjectHeader *)(from + new_cell_size * CELL_SIZE);
                memcpy(new_location, obj, cellSize * CELL_SIZE);
                obj->fwd = new_location;
                new_location->fwd = NULL;
                new_cell_size += cellSize;
            } else {
                gc_log("Freeing unmarked old object: %d\n", obj->uid);
                if(obj->type == OT_USER_OBJECT) {
                    TypeV_UserObject* user_object = (TypeV_UserObject*)(obj + 1);
                    user_object->dealloc((void*)user_object->ptr);
                }
            }

            i += cellSize; // Move to the next object
        }
    }

    gc->oldRegion.cell_size = new_cell_size;
    gc->oldRegion.from = from;
    gc->oldRegion.to = to;

    gc->oldRegion.direction = -gc->oldRegion.direction;

    // must update references here before we free (potentially) old buffer

    update_root_references(core);

    if(needs_new_buffer) {
        free(gc->oldRegion.data);
        gc->oldRegion.data = new_buffer;
    }

    gc_log("MAJOR_END (%d/%d, %d/%d)\n", gc->nursery.cell_size, NURSERY_MAX_CELLS, gc->oldRegion.cell_size, INITIAL_OLD_CELLS*gc->oldRegion.capacity_factor);
    gc_log("perform_major_gc: Completed major GC");
}

void write_barrier(TypeV_Core* core, TypeV_ObjectHeader* old_obj, TypeV_ObjectHeader* new_obj) {
    TypeV_GC* gc = core->gc;
    if (old_obj->location == 1 && new_obj->location == 0) {
        add_to_remembered_set(core, new_obj);
    }
}

void gc_free_all(TypeV_Core* core) {
    // iterates over all objects in the nursery and old region and frees them
    // this is used when the program is exiting

    // nursery
    uint64_t i = 0;
    while(i < core->gc->nursery.cell_size) {
        TypeV_ObjectHeader* obj = (TypeV_ObjectHeader *)(core->gc->nursery.from + i * CELL_SIZE);
        if(obj->type == OT_USER_OBJECT) {
            TypeV_UserObject* user_object = (TypeV_UserObject*)(obj + 1);
            user_object->dealloc((void*)user_object->ptr);
        }

        i += (obj->totalSize + CELL_SIZE - 1) / CELL_SIZE;
    }

    // old region
    i = 0;
    while(i < core->gc->oldRegion.cell_size) {
        TypeV_ObjectHeader* obj = (TypeV_ObjectHeader *)(core->gc->oldRegion.from + i * CELL_SIZE);
        if(obj->type == OT_USER_OBJECT) {
            TypeV_UserObject* user_object = (TypeV_UserObject*)(obj + 1);
            user_object->dealloc((void*)user_object->ptr);
        }

        i += (obj->totalSize + CELL_SIZE - 1) / CELL_SIZE;
    }
}

void cleanup_gc(TypeV_Core* core) {
    gc_free_all(core);
    TypeV_GC* gc = core->gc;
    free(gc->nursery.data);
    free(gc->nursery.active_bitmap);
    free(gc->oldRegion.data);
    free(gc->oldRegion.active_bitmap);
    free(gc->rs.set);
}


void add_to_remembered_set(TypeV_Core* core, TypeV_ObjectHeader* obj) {
    TypeV_GC* gc = core->gc;
    if (gc->rs.size >= gc->rs.capacity) {
        gc->rs.capacity *= 2;
        gc->rs.set = (TypeV_ObjectHeader**)realloc(gc->rs.set, gc->rs.capacity * sizeof(TypeV_ObjectHeader*));
    }
    gc->rs.set[gc->rs.size++] = obj;
}