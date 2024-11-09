#include <stdlib.h>
#include <stdio.h>
#include <mimalloc.h>
#include "core.h"
#include "allocator/allocator.h"
#include "gc.h"
#include "utils/log.h"

void* core_gc_alloc(TypeV_Core* core, size_t size) {
   
    void* mem = (void*)tv_gc_alloc(core->gc.colosseum, size);
    //core->gc.memObjects[core->gc.memObjectCount++] = mem;

    return mem;
}

void core_gc_update_alloc(TypeV_Core* core, size_t mem) {

}

TypeV_ObjectHeader* get_header_from_pointer(void* objectPtr) {
    const size_t HEADER_SIZE = sizeof(TypeV_ObjectHeader);

    if (objectPtr == NULL || ((uintptr_t)objectPtr <= HEADER_SIZE)) {
        return NULL;
    }

    // The header is just before the object memory in the allocation.
    return (TypeV_ObjectHeader*)((char*)objectPtr - HEADER_SIZE);
}


uint64_t core_gc_is_valid_ptr(TypeV_Core* core, uintptr_t ptr) {
    uintptr_t headerPtr = (uintptr_t) get_header_from_pointer((void*)ptr);
    if (!ptr) {
        return 0; // Null pointer
    }

    /*
    for (size_t i = 0; i < core->gc.memObjectCount; i++) {
        uintptr_t objPtr = (uintptr_t)(core->gc.memObjects[i]);

        if (objPtr == headerPtr) {
            return i+1; // Found a matching pointer
        }
    }
     */
    return 0; // No matching pointer found
}



void core_gc_mark_object(TypeV_Core* core, TypeV_ObjectHeader* header) {
    if (header == NULL) {
        return;  // Already marked, or null
    }

    // Mark this object
    header->marked = 1;

    if(header->type == OT_COROUTINE) {
        // coroutine function state needs to be marked
        TypeV_Coroutine* co = (TypeV_Coroutine*)(header + 1);
        core_gc_collect_single_state(core, co->state);
    }


    // Now mark the objects this one points to
    /*
    for (size_t i = 0; i < header->ptrsCount; i++) {
        void* pointedObject = header->ptrs[i];
        uint64_t idx = core_gc_is_valid_ptr(core, (uintptr_t)pointedObject);
        if (idx) {
            TypeV_ObjectHeader* pointedHeader = get_header_from_pointer(pointedObject);
            core_gc_mark_object(core, pointedHeader);
        }
    }
     */
}

void core_gc_sweep(TypeV_Core* core) {
    /*
    for (size_t i = 0; i < core->gc.memObjectCount; i++) {
        TypeV_ObjectHeader* header = (TypeV_ObjectHeader*)core->gc.memObjects[i];
        if (header && !header->marked) {
            core_gc_free_header(core, header);
            header = NULL;
            core->gc.memObjects[i] = NULL;  // Clear the tracker entry
        }
    }

    for (size_t i = 0; i < core->gc.memObjectCount; i++) {
        TypeV_ObjectHeader* header = (TypeV_ObjectHeader*)core->gc.memObjects[i];
        if (header) {
            header->marked = 0;
        }
    }
     */
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


void core_gc_collect_single_state(TypeV_Core* core, TypeV_FuncState* state) {
    // iterate through the registers
    for (size_t i = 0; i < MAX_REG; i++) {
        uintptr_t ptr = state->regs[i].ptr;
        if(core_gc_is_valid_ptr(core, ptr)) {
            TypeV_ObjectHeader* header = get_header_from_pointer((void*)ptr);
            core_gc_mark_object(core, header);
        }
    }

    for(size_t i = 0; i < state->spillSize; i++) {
        uintptr_t ptr = state->spillSlots[i].ptr;
        if(core_gc_is_valid_ptr(core, ptr)) {
            TypeV_ObjectHeader* header = get_header_from_pointer((void*)ptr);
            core_gc_mark_object(core, header);
        }
    }
}

void core_gc_collect_state(TypeV_Core* core, TypeV_FuncState* state) {
    // iterate through the registers
    for (size_t i = 0; i < MAX_REG; i++) {
        uintptr_t ptr = state->regs[i].ptr;
        if(core_gc_is_valid_ptr(core, ptr)) {
            TypeV_ObjectHeader* header = get_header_from_pointer((void*)ptr);
            core_gc_mark_object(core, header);
        }
    }

    for(size_t i = 0; i < state->spillSize; i++) {
        uintptr_t ptr = state->spillSlots[i].ptr;
        if(core_gc_is_valid_ptr(core, ptr)) {
            TypeV_ObjectHeader* header = get_header_from_pointer((void*)ptr);
            core_gc_mark_object(core, header);
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
    /*
    for(size_t i = 0; i < core->gc.memObjectCount; i++) {
        if(core->gc.memObjects[i] == NULL) {
            continue;
        }

        TypeV_ObjectHeader* header = (TypeV_ObjectHeader*)core->gc.memObjects[i];
        if(header) {
            core_gc_free_header(core, header);
            core->gc.memObjects[i] = NULL;
        }
    }
    core->gc.memObjectCount = 0;
     */
}

void core_gc_update_closure_env(TypeV_Core* core, TypeV_Closure* closurePtr, void* ptr) {
    TypeV_ObjectHeader* header = (TypeV_ObjectHeader*)((char*)closurePtr - sizeof(TypeV_ObjectHeader));

    // increase the ptrsCount
    /*
    header->ptrsCount++;
    header->ptrs = mi_realloc(header->ptrs, header->ptrsCount*sizeof(void*));
    header->ptrs[header->ptrsCount-1] = ptr;
     */
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