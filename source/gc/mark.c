//
// Created by praisethemoon on 13.11.24.
//

#include "mark.h"
#include "gc.h"
#include <string.h>


#ifdef _MSC_VER
#include <intrin.h>
#pragma intrinsic(_BitScanForward64)

static inline uint32_t count_trailing_zeros_64(uint64_t value) {
    unsigned long index;
    if (_BitScanForward64(&index, value)) {
        return (uint32_t)index;
    } else {
        // If value is 0, behavior is undefined for __builtin_ctzll
        return 64; // All bits are zero
    }
}
#else
// GCC and Clang support __builtin_ctzll
static inline uint32_t count_trailing_zeros_64(uint64_t value) {
    return __builtin_ctzll(value);
}
#endif

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
    if(ptr < sizeof(TypeV_ObjectHeader)) {
        return NULL;
    }

    uintptr_t headerPtr = ptr - sizeof(TypeV_ObjectHeader);
    size_t offset = 0;
    // Otherwise, check if `headerPtr` is in `to` space
    if ((headerPtr >= (uintptr_t)nursery->from) && (headerPtr < (uintptr_t)(nursery->from + NURSERY_SIZE / 2))) {
        offset = (headerPtr - (uintptr_t) nursery->from) / CELL_SIZE;
    }
    else {
        return NULL;
    }

    if (GET_ACTIVE(nursery->active_bitmap, offset)) {
        TypeV_ObjectHeader* obj = (TypeV_ObjectHeader*)headerPtr;
        size_t cells_needed = (obj->totalSize + CELL_SIZE - 1) / CELL_SIZE;
        for (size_t i = 0; i < cells_needed; i++) {
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
    else {
        return;
    }

    // Set the color for the block at this offset
    size_t cells_needed = (headerPtr->totalSize + CELL_SIZE - 1) / CELL_SIZE;
    for(size_t i = 0; i < cells_needed; i++) {
        SET_COLOR(nursery->color_bitmap, offset + i, color);
    }
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
                    uintptr_t dest = *(uintptr_t*)(struct_ptr->data + struct_ptr->fieldOffsets[i]);
                    core_gc_mark_nursery_object(core, dest);
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
                    uintptr_t dest = *(uintptr_t*)(class_ptr->data + class_ptr->fieldOffsets[i]);
                    core_gc_mark_nursery_object(core, dest);
                }
            }
            break;
        }
        case OT_ARRAY: {
            TypeV_Array *array_ptr = (TypeV_Array *) (header + 1);
            if (array_ptr->isPointerContainer) {
                for (size_t i = 0; i < array_ptr->length; i++) {
                    uintptr_t dest = *(uintptr_t*)(array_ptr->data + i * array_ptr->elementSize);
                    core_gc_mark_nursery_object(core, (uintptr_t )dest);
                }
            }
            break;
        }
        case OT_CLOSURE: {
            TypeV_Closure* closure_ptr = (TypeV_Closure*)(header + 1);
            for(size_t i = 0; i < closure_ptr->envSize; i++) {
                if(IS_CLOSURE_UPVALUE_POINTER(closure_ptr->ptrFields, i)) {
                    uintptr_t upvaluePtr = closure_ptr->upvalues[i].ptr;
                    core_gc_mark_nursery_object(core, upvaluePtr);
                }
            }
            break;
        }
        case OT_COROUTINE: {
            TypeV_Coroutine* coroutine_ptr = (TypeV_Coroutine*)(header + 1);
            core_gc_mark_nursery_object(core, (uintptr_t)coroutine_ptr->closure);
            gc_update_state(core, coroutine_ptr->state, 1);
            break;
        }
        case OT_RAWMEM: {
            break;
        }
    }

    gc_mark_header(header, core->gc->nurseryRegion, BLACK);
}

void gc_mark_state(TypeV_Core* core, TypeV_FuncState* state, uint8_t inNursery) {
    if (inNursery) {
        for (uint32_t i = 0; i < MAX_REG; i++) {
            if(IS_REG_PTR(state, i)) {
                uintptr_t ptr = state->regs[i].ptr;
                core_gc_mark_nursery_object(core, ptr);
            }
        }
    }
    else {
        core_panic(core, -1, "Not implemented");
    }

    // Recursively mark previous state if it exists
    if (state->prev != NULL) {
        gc_mark_state(core, state->prev, inNursery);
    }
}

void core_struct_recompute_pointers(TypeV_Struct* struct_ptr) {
    size_t bitmaskSize = (struct_ptr->numFields + 7) / 8;

    uint8_t* current_ptr = (uint8_t*)(struct_ptr + 1);
    const uint32_t numFields = struct_ptr->numFields;

    // Set `globalFields` pointer (aligned to 4 bytes)
    current_ptr = (uint8_t*)ALIGN_PTR(current_ptr, alignof(uint32_t));
    struct_ptr->globalFields = (uint32_t*)current_ptr;
    current_ptr += (numFields+1) * sizeof(uint32_t);

    // Set `fieldOffsets` pointer (aligned to 2 bytes)
    current_ptr = (uint8_t*)ALIGN_PTR(current_ptr, alignof(uint16_t));
    struct_ptr->fieldOffsets = (uint16_t*)current_ptr;
    current_ptr += numFields * sizeof(uint16_t);

    // Set `pointerBitmask` pointer (aligned to 1 byte)
    current_ptr = (uint8_t*)ALIGN_PTR(current_ptr, alignof(uint8_t));
    struct_ptr->pointerBitmask = current_ptr;
    current_ptr += bitmaskSize;


    // Set `data` pointer (aligned to 8 bytes)
    current_ptr = (uint8_t*)ALIGN_PTR(current_ptr, alignof(uint64_t));
    struct_ptr->data = current_ptr;


    // Set `bitMaskSize`
    struct_ptr->bitMaskSize = bitmaskSize;
}


void core_class_recompute_pointers(TypeV_Class* class_ptr) {
    size_t bitmaskSize = (class_ptr->numFields + 7) / 8;

    uint8_t* current_ptr = (uint8_t*)(class_ptr + 1);

    uint16_t numMethods = class_ptr->numMethods;

    // Set `methods` pointer (aligned to 8 bytes)
    current_ptr = (uint8_t*)ALIGN_PTR(current_ptr, alignof(uint64_t));
    class_ptr->methods = (uint64_t*)current_ptr;
    current_ptr += numMethods * sizeof(uint64_t);

    // Set `globalMethods` pointer (aligned to 4 bytes)
    current_ptr = (uint8_t*)ALIGN_PTR(current_ptr, alignof(uint32_t));
    class_ptr->globalMethods = (uint32_t*)current_ptr;
    current_ptr += (numMethods+1) * sizeof(uint32_t);

    // Set `fieldOffsets` pointer (aligned to 2 bytes)
    current_ptr = (uint8_t*)ALIGN_PTR(current_ptr, alignof(uint16_t));
    class_ptr->fieldOffsets = (uint16_t*)current_ptr;
    current_ptr += class_ptr->numFields * sizeof(uint16_t);

    // Set `pointerBitmask` pointer (aligned to 1 byte)
    current_ptr = (uint8_t*)ALIGN_PTR(current_ptr, alignof(uint8_t));
    class_ptr->pointerBitmask = current_ptr;
    current_ptr += bitmaskSize;

    // Set `data` pointer (aligned to 8 bytes)
    current_ptr = (uint8_t*)ALIGN_PTR(current_ptr, alignof(uint64_t));
    class_ptr->data = current_ptr;
}

void core_closure_recompute_pointers(TypeV_Closure* closure_ptr) {
    size_t bitmaskSize = (closure_ptr->envSize + 7) / 8;

    uint8_t* current_ptr = (uint8_t*)(closure_ptr + 1);

    // Set `upvalues` pointer (aligned to 8 bytes)
    current_ptr = (uint8_t*)ALIGN_PTR(current_ptr, alignof(uint64_t));
    closure_ptr->upvalues = (TypeV_Register*)current_ptr;
    current_ptr += closure_ptr->envSize * sizeof(TypeV_Register);

    // Set `ptrFields` pointer (aligned to 1 byte)
    current_ptr = (uint8_t*)ALIGN_PTR(current_ptr, alignof(uint8_t));
    closure_ptr->ptrFields = current_ptr;
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
            core_struct_recompute_pointers(struct_ptr);

            // Note: The order and size calculation should match exactly what was done during the initial allocation.

            for (size_t i = 0; i < struct_ptr->numFields; i++) {
                // Determine which byte and which bit correspond to the current field index `i`
                size_t byteIndex = i / 8;       // Which byte contains the bit for the field at index `i`
                uint8_t bitOffset = i % 8;      // Which bit within that byte corresponds to field `i`

                // Check if the bit corresponding to field `i` is set
                if (struct_ptr->pointerBitmask[byteIndex] & (1 << bitOffset)) {
                    // struct fields needs to be updated

                    uintptr_t fieldPtr;
                    memcpy(&fieldPtr, struct_ptr->data + struct_ptr->fieldOffsets[i], sizeof(uintptr_t));
                    TypeV_ObjectHeader* fieldHeader = (TypeV_ObjectHeader*)(fieldPtr - sizeof(TypeV_ObjectHeader));

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
            core_class_recompute_pointers(class_ptr);

            for (size_t i = 0; i < class_ptr->numFields; i++) {
                // Determine which byte and which bit correspond to the current field index `i`
                size_t byteIndex = i / 8;       // Which byte contains the bit for the field at index `i`
                uint8_t bitOffset = i % 8;      // Which bit within that byte corresponds to field `i`

                // Check if the bit corresponding to field `i` is set
                if (class_ptr->pointerBitmask[byteIndex] & (1 << bitOffset)) {
                    uintptr_t fieldPtr = (uintptr_t)(class_ptr->data + class_ptr->fieldOffsets[i]);
                    TypeV_ObjectHeader* fieldHeader = (TypeV_ObjectHeader*)(fieldPtr - sizeof(TypeV_ObjectHeader));

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
                    TypeV_ObjectHeader* fieldHeader = (TypeV_ObjectHeader*)(fieldPtr - sizeof(TypeV_ObjectHeader));

                    void* res = core_gc_update_object_reference_nursery(core, fieldHeader);
                    if(res != NULL) {
                        memcpy((void*)fieldPtr, &res, sizeof(void*));
                    }
                }
            }
            break;
        }
        case OT_CLOSURE: {
            TypeV_Closure* closure_ptr = (TypeV_Closure*)(new_ptr + 1);
            core_closure_recompute_pointers(closure_ptr);

            for(size_t i = 0; i < closure_ptr->envSize; i++) {
                if(IS_CLOSURE_UPVALUE_POINTER(closure_ptr->ptrFields, i)) {
                    uintptr_t upvaluePtr = (uintptr_t)(closure_ptr->upvalues + i * sizeof(uintptr_t));
                    TypeV_ObjectHeader* upvalueHeader = (TypeV_ObjectHeader*)(upvaluePtr - sizeof(TypeV_ObjectHeader));
                    void* res = core_gc_update_object_reference_nursery(core, upvalueHeader);
                    if(res != NULL) {
                        memcpy((void*)upvaluePtr, &res, sizeof(void*));
                    }
                }
            }
            break;
        }
        case OT_COROUTINE: {
            TypeV_Coroutine* coroutine_ptr = (TypeV_Coroutine*)(new_ptr + 1);
            TypeV_ObjectHeader* closureHeader = (TypeV_ObjectHeader*)(coroutine_ptr->closure - sizeof(TypeV_ObjectHeader));

            coroutine_ptr->closure = core_gc_update_object_reference_nursery(core, closureHeader);
            gc_update_state(core, coroutine_ptr->state, 1);
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
    if (inNursery) {
        for (uint32_t bitmap_index = 0; bitmap_index < 4; bitmap_index++) {
            uint64_t bitmap = state->regsPtrBitmap[bitmap_index];

            while (bitmap) {
                // Find the index of the lowest set bit
                uint32_t bit_position = count_trailing_zeros_64(bitmap); // Count trailing zeros (GCC/Clang intrinsic)
                uint32_t reg_index = (bitmap_index * 64) + bit_position;

                // Clear the bit so we can find the next one
                bitmap &= ~(1ULL << bit_position);

                // Now, we know reg_index holds a pointer
                uintptr_t ptr = state->regs[reg_index].ptr;
                TypeV_ObjectHeader* old_header = gc_ptr_in_nursery(ptr, core->gc->nurseryRegion);
                if (old_header) {
                    void *val = core_gc_update_object_reference_nursery(core, old_header);
                    if (val != NULL) {
                        state->regs[reg_index].ptr = (uintptr_t) val;
                    }
                }
            }
        }
    }

    // If previous state exists, recursively update it
    if (state->prev != NULL) {
        gc_update_state(core, state->prev, inNursery);
    }
}
