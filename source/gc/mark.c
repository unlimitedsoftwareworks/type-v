#include "mark.h"
#include <string.h>

static inline void fast_copy(void* dest, void* src) {
    assert(((uintptr_t)src % alignof(uint64_t)) == 0 && "Source pointer is not 8-byte aligned!");
    assert(((uintptr_t)dest % alignof(uint64_t)) == 0 && "Destination pointer is not 8-byte aligned!");

    *(uint64_t *)dest = *(const uint64_t *)src;
}

void qcgc_trace_cb(object_t* object, void (*visit)(object_t*)) {
    switch (object->type) {
        case OT_STRUCT: {
            TypeV_Struct* struct_ptr = (TypeV_Struct*)(object);

            for (size_t i = 0; i < struct_ptr->numFields; i++) {
                // Determine which byte and which bit correspond to the current field index `i`
                size_t byteIndex = i / 8;       // Which byte contains the bit for the field at index `i`
                uint8_t bitOffset = i % 8;      // Which bit within that byte corresponds to field `i`

                // Check if the bit corresponding to field `i` is set
                if (struct_ptr->pointerBitmask[byteIndex] & (1 << bitOffset)) {
                    uintptr_t field;
                    fast_copy(&field, struct_ptr->data + struct_ptr->fieldOffsets[i]);
                    if(field) {
                        visit((object_t*)field);
                    }
                }
            }

            break;
        }

        case OT_CLASS: {
            TypeV_Class *class_ptr = (TypeV_Class *) (object);
            for (size_t i = 0; i < class_ptr->numFields; i++) {
                // Determine which byte and which bit correspond to the current field index `i`
                size_t byteIndex = i / 8;       // Which byte contains the bit for the field at index `i`
                uint8_t bitOffset = i % 8;      // Which bit within that byte corresponds to field `i`

                // Check if the bit corresponding to field `i` is set
                if (class_ptr->pointerBitmask[byteIndex] & (1 << bitOffset)) {
                    uintptr_t field;
                    fast_copy(&field, class_ptr->data + class_ptr->fieldOffsets[i]);

                    if(field) {
                        visit((object_t*)field);
                    }
                }
            }
            break;
        }
        case OT_ARRAY: {
            TypeV_Array *array_ptr = (TypeV_Array *) (object);
            if (array_ptr->isPointerContainer) {
                for (size_t i = 0; i < array_ptr->length; i++) {
                    uintptr_t field;
                    fast_copy(&field, array_ptr->data + i * array_ptr->elementSize);

                    if(field != 0) {
                        visit((object_t*)field);
                    }
                }
            }
            break;
        }
        case OT_CLOSURE: {
            TypeV_Closure* closure_ptr = (TypeV_Closure*)(object);
            for(size_t i = 0; i < closure_ptr->envSize; i++) {
                if(IS_CLOSURE_UPVALUE_POINTER(closure_ptr->ptrFields, i)) {
                    uintptr_t field = closure_ptr->upvalues[i].ptr;

                    if(field) {
                        visit((object_t*)field);
                    }
                }
            }
            break;
        }
        case OT_COROUTINE: {
            TypeV_Coroutine* coroutine_ptr = (TypeV_Coroutine*)(object);
            push_roots_non_recursive(coroutine_ptr->state);
            visit((object_t*)coroutine_ptr->closure);
            // we also need to mark state
            break;
        }
        case OT_CUSTOM_OBJECT: {
            break;
        }
        case OT_CUSTOM_COLLECTABLE_OBJECT:
            break;
    }
}


void push_roots(TypeV_FuncState * state){
    TypeV_FuncState* current = state;
    while(current != NULL) {
        for (uint32_t i = 0; i < MAX_REG; i++) {
            if (IS_REG_PTR(current, i)) {
                qcgc_push_root((object_t*)current->regs[i].ptr);
            }
        }
        current = current->prev;
    }
}
void push_roots_non_recursive(TypeV_FuncState * state) {
    if(!state) return;
    for (uint32_t i = 0; i < MAX_REG; i++) {
        if (IS_REG_PTR(state, i)) {
            qcgc_push_root((object_t*)state->regs[i].ptr);
        }
    }
}

void push_roots_callback(void* context) {
    TypeV_Core* core = (TypeV_Core*)context;
    push_roots(core->funcState);
}
