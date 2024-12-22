//
// Created by praisethemoon on 13.11.24.
//

#include "mark.h"
#include "gc.h"
#include <string.h>
#include <assert.h>

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

static inline void fast_copy(void* dest, void* src) {
    assert(((uintptr_t)src % alignof(uint64_t)) == 0 && "Source pointer is not 8-byte aligned!");
    assert(((uintptr_t)dest % alignof(uint64_t)) == 0 && "Destination pointer is not 8-byte aligned!");

    *(uint64_t *)dest = *(const uint64_t *)src;
}

void perform_minor_mark(TypeV_Core* core) {
    TypeV_GC* gc = core->gc;
    gc_log("perform_minor_mark: Starting minor mark phase");

    mark_state(core, core->funcState);

    // Step 2: Traverse the old region

    if(gc->oldRegion.direction == 1) {
        uint64_t index = 0;
        while (index < gc->oldRegion.cell_size) {
            TypeV_ObjectHeader* obj = (TypeV_ObjectHeader *)(gc->oldRegion.from + index * CELL_SIZE);
            size_t cellSize = (obj->totalSize + CELL_SIZE - 1) / CELL_SIZE;

            // check if the area is dirty
            if(GET_ACTIVE(gc->oldRegion.dirty_bitmap, index)) {
                mark_object(core, obj);
            }

            index += cellSize;
        }
    }
    else {
        // we will start from the middle downwards here, so we will use
        // reverse cell indexing
        uint64_t index = gc->oldRegion.cell_size;
        while (index != 0) {
            TypeV_ObjectHeader* obj = (TypeV_ObjectHeader *)(gc->oldRegion.to - index * CELL_SIZE);
            size_t cellSize = (obj->totalSize + CELL_SIZE - 1) / CELL_SIZE;

            if(GET_ACTIVE(gc->oldRegion.dirty_bitmap, index)) {
                mark_object(core, obj);
            }

            index -= cellSize;
        }
    }

    gc_log("perform_minor_mark: Completed minor mark phase");
}

void perform_major_mark(TypeV_Core* core) {
    gc_log("perform_minor_mark: Starting minor mark phase");
    mark_state(core, core->funcState);
}

void mark_object(TypeV_Core* core, TypeV_ObjectHeader* obj) {
    if(!obj || (obj->color != WHITE && obj->color != NOTSU)) {
        return;
    }

    obj->color = GRAY;

    // If the object is a struct, mark its fields
    switch (obj->type) {
        case OT_STRUCT: {
            TypeV_Struct* struct_ptr = (TypeV_Struct*)(obj + 1);

            for (size_t i = 0; i < struct_ptr->numFields; i++) {
                // Determine which byte and which bit correspond to the current field index `i`
                size_t byteIndex = i / 8;       // Which byte contains the bit for the field at index `i`
                uint8_t bitOffset = i % 8;      // Which bit within that byte corresponds to field `i`

                // Check if the bit corresponding to field `i` is set
                if (struct_ptr->pointerBitmask[byteIndex] & (1 << bitOffset)) {
                    uintptr_t field;
                    fast_copy(&field, struct_ptr->data + struct_ptr->fieldOffsets[i]);
                    if(field) {
                        TypeV_ObjectHeader* dest = GET_OBJ_HEADER(field);
                        mark_object(core, dest);
                    }
                }
            }

            break;
        }

        case OT_CLASS: {
            TypeV_Class *class_ptr = (TypeV_Class *) (obj + 1);
            for (size_t i = 0; i < class_ptr->numFields; i++) {
                // Determine which byte and which bit correspond to the current field index `i`
                size_t byteIndex = i / 8;       // Which byte contains the bit for the field at index `i`
                uint8_t bitOffset = i % 8;      // Which bit within that byte corresponds to field `i`

                // Check if the bit corresponding to field `i` is set
                if (class_ptr->pointerBitmask[byteIndex] & (1 << bitOffset)) {
                    uintptr_t field;
                    fast_copy(&field, class_ptr->data + class_ptr->fieldOffsets[i]);

                    if(field) {
                        TypeV_ObjectHeader* dest = GET_OBJ_HEADER(field);
                        mark_object(core, dest);
                    }
                }
            }
            break;
        }
        case OT_ARRAY: {
            TypeV_Array *array_ptr = (TypeV_Array *) (obj + 1);
            if (array_ptr->isPointerContainer) {
                for (size_t i = 0; i < array_ptr->length; i++) {
                    uintptr_t field;
                    fast_copy(&field, array_ptr->data + i * array_ptr->elementSize);

                    if(field != 0) {
                        TypeV_ObjectHeader *dest = GET_OBJ_HEADER(field);
                        mark_object(core, dest);
                    }
                }
            }
            break;
        }
        case OT_CLOSURE: {
            TypeV_Closure* closure_ptr = (TypeV_Closure*)(obj + 1);
            for(size_t i = 0; i < closure_ptr->envSize; i++) {
                if(IS_CLOSURE_UPVALUE_POINTER(closure_ptr->ptrFields, i)) {
                    uintptr_t field = closure_ptr->upvalues[i].ptr;

                    if(field) {
                        TypeV_ObjectHeader* dest = GET_OBJ_HEADER(field);
                        mark_object(core, dest);
                    }
                }
            }
            break;
        }
        case OT_COROUTINE: {
            TypeV_Coroutine* coroutine_ptr = (TypeV_Coroutine*)(obj + 1);
            mark_object(core, GET_OBJ_HEADER(coroutine_ptr->closure));
            gc_update_state(core, coroutine_ptr->state);
            break;
        }
        case OT_CUSTOM_OBJECT: {
            break;
        }
        case OT_CUSTOM_COLLECTABLE_OBJECT:
            break;
    }

    obj->color = BLACK;
}


void mark_state(TypeV_Core* core, TypeV_FuncState* state) {
    for (uint32_t i = 0; i < MAX_REG; i++) {
        if(IS_REG_PTR(state, i) && state->regs[i].ptr) {
            TypeV_ObjectHeader* obj = GET_OBJ_HEADER(state->regs[i].ptr);
            mark_object(core, obj);
        }
    }

    // Recursively mark previous state if it exists
    if (state->prev != NULL) {
        mark_state(core, state->prev);
    }
}

void update_root_references(TypeV_Core* core) {
    gc_log("update_root_references: Updating root object references");
    gc_update_state(core, core->funcState);
    gc_log("update_root_references: Completed updating references");
}

void gc_update_state(TypeV_Core* core, TypeV_FuncState* state) {
    for(uint32_t i = 0; i < MAX_REG; i++) {
        if(IS_REG_PTR(state, i) && state->regs[i].ptr) {
            // Now, we know reg_index holds a pointer
            uintptr_t ptr = state->regs[i].ptr;
            if(ptr) {
                TypeV_ObjectHeader* obj = GET_OBJ_HEADER(ptr);
                state->regs[i].ptr = (uintptr_t)update_object_reference(core, obj);
            }
        }
    }

    // If previous state exists, recursively update it
    if (state->prev != NULL) {
        gc_update_state(core, state->prev);
    }
}

void* update_object_reference(TypeV_Core* core, TypeV_ObjectHeader* obj) {
    // printf("Checking for object updates %p\n", header);
    if (!obj) {
        return NULL;  // Already marked, or null
    }

    if(obj->color == NOTSU) {
        return obj->fwd ? (obj->fwd+1) : (obj+1);
    }

    obj->color = NOTSU;
    TypeV_ObjectHeader * old_obj = obj;
    if(obj->fwd) {
        obj = obj->fwd;
    }

    // If the object is a struct, mark its fields
    switch (obj->type) {
        case OT_STRUCT: {
            TypeV_Struct* struct_ptr = (TypeV_Struct*)(obj + 1);
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
                    fast_copy(&fieldPtr, struct_ptr->data + struct_ptr->fieldOffsets[i]);
                    if(fieldPtr) {
                        TypeV_ObjectHeader* fieldHeader = (TypeV_ObjectHeader*)(fieldPtr - sizeof(TypeV_ObjectHeader));
                        void* res = update_object_reference(core, fieldHeader);
                        if (res) {
                            fast_copy(struct_ptr->data + struct_ptr->fieldOffsets[i], &res);
                        }
                    }
                }
            }

            break;
        }

        case OT_CLASS: {
            TypeV_Class *class_ptr = (TypeV_Class *) (obj + 1);
            core_class_recompute_pointers(class_ptr);

            for (size_t i = 0; i < class_ptr->numFields; i++) {
                // Determine which byte and which bit correspond to the current field index `i`
                size_t byteIndex = i / 8;       // Which byte contains the bit for the field at index `i`
                uint8_t bitOffset = i % 8;      // Which bit within that byte corresponds to field `i`

                // Check if the bit corresponding to field `i` is set
                if (class_ptr->pointerBitmask[byteIndex] & (1 << bitOffset)) {
                    uintptr_t fieldPtr;
                    fast_copy(&fieldPtr, class_ptr->data + class_ptr->fieldOffsets[i]);
                    if(fieldPtr) {
                        TypeV_ObjectHeader* fieldHeader = (TypeV_ObjectHeader*)(fieldPtr - sizeof(TypeV_ObjectHeader));
                        void* res = update_object_reference(core, fieldHeader);
                        if (res) {
                            fast_copy(class_ptr->data + class_ptr->fieldOffsets[i], &res);
                        }
                    }
                }
            }
            break;
        }
        case OT_ARRAY: {
            TypeV_Array *array_ptr = (TypeV_Array *) (obj + 1);
            if (array_ptr->isPointerContainer) {
                for (size_t i = 0; i < array_ptr->length; i++) {
                    uintptr_t fieldPtr;
                    fast_copy(&fieldPtr, array_ptr->data + i * array_ptr->elementSize);

                    if(fieldPtr) {
                        TypeV_ObjectHeader *fieldHeader =(TypeV_ObjectHeader *) (fieldPtr - sizeof(TypeV_ObjectHeader));
                        uintptr_t res = (uintptr_t) update_object_reference(core, fieldHeader);
                        if (res) {
                            fast_copy(array_ptr->data + i * array_ptr->elementSize, &res);
                        }
                    }
                }
            }
            break;
        }
        case OT_CLOSURE: {
            TypeV_Closure* closure_ptr = (TypeV_Closure*)(obj + 1);
            core_closure_recompute_pointers(closure_ptr);

            for(size_t i = 0; i < closure_ptr->envSize; i++) {
                if(IS_CLOSURE_UPVALUE_POINTER(closure_ptr->ptrFields, i)) {
                    uintptr_t upvaluePtr = closure_ptr->upvalues[i].ptr;
                    if(upvaluePtr) {
                        TypeV_ObjectHeader *upvalueHeader = (TypeV_ObjectHeader *) (upvaluePtr -
                                                                                    sizeof(TypeV_ObjectHeader));
                        void *res = update_object_reference(core, upvalueHeader);
                        if (res) {
                            closure_ptr->upvalues[i].ptr = (uintptr_t)res;
                        }
                    }
                }
            }
            break;
        }
        case OT_COROUTINE: {
            TypeV_Coroutine* coroutine_ptr = (TypeV_Coroutine*)(obj + 1);
            TypeV_ObjectHeader* closureHeader = (TypeV_ObjectHeader*)(coroutine_ptr->closure - sizeof(TypeV_ObjectHeader));

            coroutine_ptr->closure = update_object_reference(core, closureHeader);
            gc_update_state(core, coroutine_ptr->state);
            break;
        }
        case OT_CUSTOM_OBJECT: {
            break;
        }
    }

    return obj+1;
}









void core_struct_recompute_pointers(TypeV_Struct* struct_ptr) {
    size_t bitmaskSize = (struct_ptr->numFields + 7) / 8;

    uint8_t* current_ptr = (uint8_t*)(struct_ptr + 1);
    const uint32_t numFields = struct_ptr->numFields;

    // Set `globalFields` pointer (naturally aligned to 4 bytes)
    struct_ptr->globalFields = (uint32_t*)current_ptr;
    current_ptr += numFields * sizeof(uint32_t);

    // Set `fieldOffsets` pointer (naturally aligned to 2 bytes)
    struct_ptr->fieldOffsets = (uint16_t*)current_ptr;
    current_ptr += numFields * sizeof(uint16_t);

    // Set `pointerBitmask` pointer (naturally aligned to 1 byte)
    struct_ptr->pointerBitmask = current_ptr;
    current_ptr += bitmaskSize;

    // Align `current_ptr` to 8 bytes for `data` pointer
    current_ptr = (uint8_t*)ALIGN_PTR(current_ptr, alignof(uint64_t));
    struct_ptr->data = current_ptr;
}

void core_class_recompute_pointers(TypeV_Class* class_ptr) {
    size_t bitmaskSize = (class_ptr->numFields + 7) / 8;

    uint8_t* current_ptr = (uint8_t*)(class_ptr + 1);

    uint16_t numMethods = class_ptr->numMethods;

    // Set `methods` pointer (aligned to 8 bytes)
    class_ptr->methods = (uint64_t*)current_ptr;
    current_ptr += numMethods * sizeof(uint64_t);

    // Set `globalMethods` pointer (aligned to 4 bytes)
    class_ptr->globalMethods = (uint32_t*)current_ptr;
    current_ptr += (numMethods) * sizeof(uint32_t);

    // Set `fieldOffsets` pointer (aligned to 2 bytes)
    class_ptr->fieldOffsets = (uint16_t*)current_ptr;
    current_ptr += class_ptr->numFields * sizeof(uint16_t);

    // Set `pointerBitmask` pointer (aligned to 1 byte)
    class_ptr->pointerBitmask = current_ptr;
    current_ptr += bitmaskSize;

    current_ptr = (uint8_t*)(((uintptr_t)current_ptr + (alignof(uint64_t) - 1)) & ~(alignof(uint64_t) - 1));
    // Set `data` pointer (aligned to 8 bytes)
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
