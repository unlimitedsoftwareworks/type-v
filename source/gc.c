#include <stdlib.h>
#include <stdio.h>
#include <mimalloc.h>
#include <string.h>
#include "gc.h"
#include "allocator/allocator.h"
#include "utils/log.h"

TypeV_ObjectHeader* get_header_from_anonymous_pointer(GCContext* ctx, void* objectPtr) {

    const size_t HEADER_SIZE = sizeof(TypeV_ObjectHeader);

    if (objectPtr == NULL || ((uintptr_t)objectPtr <= HEADER_SIZE)) {
        return NULL;
    }

    TypeV_ObjectHeader* header = (TypeV_ObjectHeader*)((char*)objectPtr - HEADER_SIZE);
    uintptr_t addr = (uintptr_t)header;

    return header;
}

TypeV_ObjectHeader* get_header_from_pointer(GCContext* ctx, void* objectPtr) {
    const MemoryManager* manager = ctx->memory_manager;
    const size_t HEADER_SIZE = sizeof(TypeV_ObjectHeader);

    if (objectPtr == NULL || ((uintptr_t)objectPtr <= HEADER_SIZE)) {
        return NULL;
    }

    TypeV_ObjectHeader* header = (TypeV_ObjectHeader*)((char*)objectPtr - HEADER_SIZE);
    uintptr_t addr = (uintptr_t)header;
    // make sure header exists, i.e address is valid

    if (addr >= (uintptr_t)manager->nursery->data && addr < (uintptr_t)manager->nursery->data + NURSERY_SIZE) {
        return header;
    } else if (addr >= (uintptr_t)manager->survivor_from->data && addr < (uintptr_t)manager->survivor_from->data + SURVIVOR_SIZE) {
        return header;
    } else if (addr >= (uintptr_t)manager->old_gen->data && addr < (uintptr_t)manager->old_gen->data + OLD_GEN_INITIAL_SIZE) {
        return header;
    }

    // The header is just before the object memory in the allocation.
    return NULL;
}

void core_gc_mark_object(TypeV_Core* core, TypeV_ObjectHeader* header) {
    // printf("Marking object %p\n", header);
    if (header == NULL) {
        return;  // Already marked, or null
    }

    // Mark the current object in its arena
    mark_ptr(core->gc, (void*)header);

    // If the object is a struct, mark its fields
    switch (header->type) {
        case OT_STRUCT: {
            TypeV_Struct* struct_ptr = (TypeV_Struct*)(header + 1);

            for (size_t i = 0; i < struct_ptr->numFields; i++) {
                // Determine which byte and which bit correspond to the current field index `i`
                size_t byteIndex = i / 8;       // Which byte contains the bit for the field at index `i`
                uint8_t bitOffset = i % 8;      // Which bit within that byte corresponds to field `i`

                // Check if the bit corresponding to field `i` is set
                //if (struct_ptr->pointerBitmask[byteIndex] & (1 << bitOffset)) {
                    uintptr_t fieldPtr = (uintptr_t)(struct_ptr->data + struct_ptr->fieldOffsets[i]);
                    TypeV_ObjectHeader* fieldHeader = get_header_from_pointer(core->gc, (void*)fieldPtr);
                    core_gc_mark_object(core, fieldHeader);
                //}
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
                    TypeV_ObjectHeader* fieldHeader = get_header_from_pointer(core->gc, (void*)fieldPtr);
                    core_gc_mark_object(core, fieldHeader);
                }
            }
            break;
        }
        case OT_ARRAY: {
            TypeV_Array *array_ptr = (TypeV_Array *) (header + 1);
            if (array_ptr->isPointerContainer) {
                for (size_t i = 0; i < array_ptr->length; i++) {
                    uintptr_t fieldPtr = (uintptr_t)(array_ptr->data + i * array_ptr->elementSize);
                    TypeV_ObjectHeader* fieldHeader = get_header_from_pointer(core->gc, (void*)fieldPtr);
                    core_gc_mark_object(core, fieldHeader);
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


void core_gc_mark_state(TypeV_Core* core, TypeV_FuncState* state) {
    for(uint32_t i = 0; i < MAX_REG; i++) {
        uintptr_t ptr = state->regs[i].ptr;
        // get the header
        TypeV_ObjectHeader *header = get_header_from_pointer(core->gc, (void*)ptr);

        if(header == NULL) {
            continue;
        }

        core_gc_mark_object(core, header);
    }


    // if previous state exists, collect it
    if(state->prev != NULL) {
        core_gc_mark_state(core, state->prev);
    }
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
            //// printf("freeing struct\n");
            core_struct_free(core, header);
            break;
        }
        case OT_CLASS: {
            //// printf("freeing class\n");
            core_class_free(core, header);
            break;
        }
        case OT_ARRAY: {
            //// printf("freeing array\n");
            core_array_free(core, header);
            break;
        }
        case OT_CLOSURE:
            //// printf("freeing closure\n");
            core_closure_free(core, header);
            break;
        case OT_COROUTINE:
            //// printf("freeing coroutine\n");
            core_coroutine_free(core, header);
            break;
        case OT_RAWMEM:
            //// printf("freeing rawmem\n");
            break;
    }
}



static MemoryRegion *allocate_region(RegionType type, size_t size) {
    MemoryRegion *region = (MemoryRegion *)mi_malloc_aligned(sizeof(MemoryRegion), 16);
    if (!region) {
        // fprintf(stderr, "Failed to allocate memory for region\n");
        exit(1);
    }
    region->type = type;
    region->data = (uint8_t *)mi_malloc_aligned(size, 16);
    if (!region->data) {
        // fprintf(stderr, "Failed to allocate memory for region data\n");
        exit(1);
    }
    memset(region->data, 0, size);
    memset(region->metadata.block_bitmap, 0, sizeof(region->metadata.block_bitmap));
    memset(region->metadata.color, WHITE, sizeof(region->metadata.color));
    region->metadata.gray_stack_top = 0;
    region->metadata.ssb_top = 0;
    region->size = size;
    return region;
}

static void expand_old_generation(MemoryRegion *old_gen) {
    size_t new_size = old_gen->size * 2;
    uint8_t *new_data = (uint8_t *)mi_realloc(old_gen->data, new_size);
    if (!new_data) {
        // fprintf(stderr, "Failed to expand old generation memory\n");
        exit(1);
    }
    old_gen->data = new_data;
    memset(old_gen->data + old_gen->size, 0, new_size - old_gen->size);
    old_gen->size = new_size;
    // Update metadata arrays if needed
    // Assuming block_bitmap and color arrays are resized accordingly if required
}

GCContext *gc_init(void) {
    GCContext *context = (GCContext *)mi_malloc(sizeof(GCContext));
    if (!context) {
        // fprintf(stderr, "Failed to allocate memory for GC context\n");
        exit(1);
    }

    MemoryManager *manager = (MemoryManager *)mi_malloc(sizeof(MemoryManager));
    if (!manager) {
        // fprintf(stderr, "Failed to allocate memory for Memory Manager\n");
        exit(1);
    }

    manager->nursery = allocate_region(REGION_NURSERY, NURSERY_SIZE);
    manager->survivor_from = allocate_region(REGION_SURVIVOR_FROM, SURVIVOR_SIZE);
    manager->survivor_to = allocate_region(REGION_SURVIVOR_TO, SURVIVOR_SIZE);
    manager->old_gen = allocate_region(REGION_OLD, OLD_GEN_INITIAL_SIZE);

    context->memory_manager = manager;
    return context;
}

void gc_finalize(GCContext *context) {
    mi_free(context->memory_manager->nursery->data);
    mi_free(context->memory_manager->nursery);
    mi_free(context->memory_manager->survivor_from->data);
    mi_free(context->memory_manager->survivor_from);
    mi_free(context->memory_manager->survivor_to->data);
    mi_free(context->memory_manager->survivor_to);
    mi_free(context->memory_manager->old_gen->data);
    mi_free(context->memory_manager->old_gen);
    mi_free(context->memory_manager);
    mi_free(context);
}

void *gc_alloc(TypeV_Core* core, size_t size) {
    GCContext *context = core->gc;
    MemoryRegion *nursery = context->memory_manager->nursery;

    // Calculate the total required size (align to cell boundaries if needed)
    size_t cells_needed = (size + CELL_SIZE - 1) / CELL_SIZE;  // Number of cells required
    size_t bytes_needed = cells_needed * CELL_SIZE;            // Total bytes required

    // Check if there is enough space in the nursery
    if (nursery->size + bytes_needed > REGION_SIZE) {  // REGION_SIZE is the total nursery capacity
        // fprintf(stderr, "Nursery is full, triggering Minor GC\n");

        // Perform garbage collection
        core_gc_mark_state(core, core->funcState);
        gc_minor_collect(context);
        core_gc_update_references_state(core, core->funcState);

        // Retry allocation after GC
        if (nursery->size + bytes_needed > REGION_SIZE) {
            // fprintf(stderr, "Not enough space in nursery even after GC\n");
            return NULL;  // Allocation fails if there's still not enough space
        }
    }

    // Allocate memory at the current offset
    TypeV_ObjectHeader* header = (TypeV_ObjectHeader*)&nursery->data[nursery->size];
    header->totalSize = size;

    // Increment the nursery's `size` to reflect the new allocation
    nursery->size += bytes_needed;

    return header;
}


void gc_minor_collect(GCContext *context) {
    MemoryRegion *from = context->memory_manager->survivor_from;
    MemoryRegion *to = context->memory_manager->survivor_to;
    MemoryRegion *nursery = context->memory_manager->nursery;

    // Reset forwarding table for this collection cycle
    context->forwarding_table_size = 0;

    // Copy live objects from nursery to survivor space (to-space)
    for (size_t i = 0; i < MAX_CELLS / 2; i++) {
        if (nursery->metadata.color[i] == GRAY || nursery->metadata.color[i] == BLACK) {
            // Start of a live object in nursery
            void* old_address = &nursery->data[i * CELL_SIZE];
            TypeV_ObjectHeader* header = (TypeV_ObjectHeader*)old_address;

            // Ensure header is valid
            if (header == NULL) {
                continue;
            }

            // Get the exact size in cells from the object header
            size_t cells_needed = (header->totalSize + CELL_SIZE - 1) / CELL_SIZE;

            // Find a contiguous mi_free block of `cells_needed` cells in `to`
            size_t start_index = 0;
            bool found = false;
            for (size_t j = 0; j < MAX_CELLS / 4; j++) {
                // Check if there are enough consecutive mi_free cells in `to`
                bool enough_space = true;
                for (size_t k = 0; k < cells_needed; k++) {
                    if ((j + k >= MAX_CELLS / 4) ||
                        (to->metadata.block_bitmap[(j + k) / 64] & (1ULL << ((j + k) % 64)))) {
                        enough_space = false;
                        break;
                    }
                }
                if (enough_space) {
                    start_index = j;
                    found = true;
                    break;
                }
            }

            if (!found) {
                // fprintf(stderr, "Not enough space in survivor space for object of size %zu cells\n", cells_needed);
                core_panic(NULL, -1, "Out of memory");
            }

            // Mark the cells as allocated in the `to-space` bitmap
            for (size_t k = 0; k < cells_needed; k++) {
                to->metadata.block_bitmap[(start_index + k) / 64] |= (1ULL << ((start_index + k) % 64));
                to->metadata.color[start_index + k] = BLACK;  // Mark all cells as BLACK
            }

            // Calculate the new address in `to`
            void* new_address = &to->data[start_index * CELL_SIZE];

            // Copy the object from nursery to survivor (to-space)
            memcpy(new_address, old_address, cells_needed * CELL_SIZE);

            // Add to forwarding table for pointer updates
            add_to_forwarding_table(context, old_address, new_address);

            // Move `i` forward to skip over the entire object in the nursery
            i += cells_needed - 1;
        }
    }

    // Swap survivor `from` and `to` regions
    MemoryRegion *temp = context->memory_manager->survivor_from;
    context->memory_manager->survivor_from = context->memory_manager->survivor_to;
    context->memory_manager->survivor_to = temp;

    // Clear nursery metadata to mark all slots as mi_free
    memset(nursery->metadata.block_bitmap, 0, sizeof(nursery->metadata.block_bitmap));
    memset(nursery->metadata.color, WHITE, sizeof(nursery->metadata.color));

    // Clear the nursery data to prepare for new allocations
    nursery->size = 0;
}



void gc_full_collect(GCContext *context) {
    MemoryRegion *old_gen = context->memory_manager->old_gen;

    // Mark Phase: Mark all reachable objects starting from roots
    for (size_t i = 0; i < MAX_CELLS; i++) {
        if (old_gen->metadata.color[i] == GRAY) {
            old_gen->metadata.color[i] = BLACK;
        }
    }

    // Sweep Phase: Free unmarked (white) objects
    for (size_t i = 0; i < MAX_CELLS; i++) {
        if (old_gen->metadata.color[i] == WHITE) {
            old_gen->metadata.block_bitmap[i / 64] &= ~(1ULL << (i % 64));
        }
    }

    // If old generation is full, expand it
    size_t used_cells = 0;
    for (size_t i = 0; i < old_gen->size / CELL_SIZE; i++) {
        if (old_gen->metadata.block_bitmap[i / 64] & (1ULL << (i % 64))) {
            used_cells++;
        }
    }
    if (used_cells >= (old_gen->size / CELL_SIZE) * 0.8) { // Expand when 80% full
        expand_old_generation(old_gen);
    }
}

void mark_ptr(GCContext *gc, void *ptr) {
    MemoryManager *manager = gc->memory_manager;
    uintptr_t addr = (uintptr_t)ptr;

    // Determine which region the pointer belongs to
    // TODO: use __builtin_expect
    if (addr >= (uintptr_t)manager->nursery->data && addr < (uintptr_t)manager->nursery->data + NURSERY_SIZE) {
        size_t index = (addr - (uintptr_t)manager->nursery->data) / CELL_SIZE;
        manager->nursery->metadata.color[index] = GRAY;
    } else if (addr >= (uintptr_t)manager->survivor_from->data && addr < (uintptr_t)manager->survivor_from->data + SURVIVOR_SIZE) {
        size_t index = (addr - (uintptr_t)manager->survivor_from->data) / CELL_SIZE;
        manager->survivor_from->metadata.color[index] = GRAY;
    } else if (addr >= (uintptr_t)manager->old_gen->data && addr < (uintptr_t)manager->old_gen->data + OLD_GEN_INITIAL_SIZE) {
        size_t index = (addr - (uintptr_t)manager->old_gen->data) / CELL_SIZE;
        manager->old_gen->metadata.color[index] = GRAY;
    }
    else {
        return;
    }
}

void add_to_forwarding_table(GCContext *gc, void *old_ptr, void *new_ptr) {
    // printf("Adding to forwarding table %p -> %p\n", old_ptr, new_ptr);
    gc->forwarding_table[gc->forwarding_table_size].old_ptr = old_ptr;
    gc->forwarding_table[gc->forwarding_table_size].new_ptr = new_ptr;
    gc->forwarding_table_size++;
}

void *get_forwarded_address(GCContext *gc, void *old_ptr) {
    for (size_t i = 0; i < gc->forwarding_table_size; i++) {
        if (gc->forwarding_table[i].old_ptr == old_ptr) {
            return gc->forwarding_table[i].new_ptr;
        }
    }
    return NULL;  // If not found, assume no relocation needed
}


void* core_gc_update_object_reference(TypeV_Core* core, TypeV_ObjectHeader* header) {
    // printf("Checking for object updates %p\n", header);
    if (header == NULL) {
        return NULL;  // Already marked, or null
    }

    // Mark the current object in its arena
    TypeV_ObjectHeader* new_ptr = get_forwarded_address(core->gc, (void*)header);

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

             size_t dataSize = header->totalSize - totalAllocationSize;

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
                    uintptr_t fieldPtr = (uintptr_t)struct_ptr->data + struct_ptr->fieldOffsets[i];
                    TypeV_ObjectHeader* fieldHeader = get_header_from_pointer(core->gc, (void*)fieldPtr);
                    void* res = core_gc_update_object_reference(core, fieldHeader);
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
                    TypeV_ObjectHeader* fieldHeader = get_header_from_pointer(core->gc, (void*)fieldPtr);
                    void* res = core_gc_update_object_reference(core, fieldHeader);
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
                    TypeV_ObjectHeader* fieldHeader = get_header_from_pointer(core->gc, (void*)fieldPtr);
                    void* res = core_gc_update_object_reference(core, fieldHeader);
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

void core_gc_update_references_state(TypeV_Core* core, TypeV_FuncState* state) {
    for(uint32_t i = 0; i < MAX_REG; i++) {
        uintptr_t ptr = state->regs[i].ptr;
        // get the header
        TypeV_ObjectHeader *header = get_header_from_anonymous_pointer(core->gc, (void*)ptr);

        if(header == NULL) {
            continue;
        }

        void* val = core_gc_update_object_reference(core, header);
        if(val != NULL) {
            state->regs[i].ptr = (uintptr_t)val;
        }
    }


    // if previous state exists, collect it
    if(state->prev != NULL) {
        core_gc_update_references_state(core, state->prev);
    }
}