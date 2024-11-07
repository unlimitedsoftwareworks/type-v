#include <stdlib.h>
#include <stdio.h>
#include "core.h"
#include "gc.h"
#include "utils/log.h"


void* core_gc_alloc(TypeV_Core* core, size_t size) {
    if (core->gc.allocsSincePastGC > 1*1024*1024) {
        printf("[GC]: Allocated %llu bytes since last GC\n", core->gc.allocsSincePastGC);
        core_gc_collect(core);
        core->gc.allocsSincePastGC = 0;
    }

    if(core->gc.memObjectCount >= core->gc.memObjectCapacity) {
        core->gc.memObjectCapacity *= 2;
        core->gc.memObjects = realloc(core->gc.memObjects, sizeof(size_t)*(core->gc.memObjectCapacity));

        if(core->gc.memObjects == NULL) {
            core_panic(core, -1, "Failed to reallocate memory for GC");
        }
    }
    void* mem = malloc(size);
    core->gc.memObjects[core->gc.memObjectCount++] = mem;

    return mem;
}

void core_gc_update_alloc(TypeV_Core* core, size_t mem) {
    core->gc.allocsSincePastGC += mem;
    core->gc.totalAllocs += mem;
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
    for (size_t i = 0; i < core->gc.memObjectCount; i++) {
        uintptr_t objPtr = (uintptr_t)(core->gc.memObjects[i]);

        if (objPtr == headerPtr) {
            return i+1; // Found a matching pointer
        }
    }
    return 0; // No matching pointer found
}



void core_gc_mark_object(TypeV_Core* core, TypeV_ObjectHeader* header) {
    if (header == NULL) {
        return;  // Already marked, or null
    }

    // Mark this object
    header->marked = 1;


    // Now mark the objects this one points to
    for (size_t i = 0; i < header->ptrsCount; i++) {
        void* pointedObject = header->ptrs[i];
        uint64_t idx = core_gc_is_valid_ptr(core, (uintptr_t)pointedObject);
        if (idx) {
            TypeV_ObjectHeader* pointedHeader = get_header_from_pointer(pointedObject);
            core_gc_mark_object(core, pointedHeader);
        }
    }
}

void core_gc_sweep(TypeV_Core* core) {
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
}

void core_gc_collect(TypeV_Core* core) {
    //LOG_WARN("Collecting state for core %d", core->id);
    // TODO: uncomment and identify bugs
    core_gc_collect_state(core, core->funcState);
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
    for(size_t i = 0; i < core->gc.memObjectCount; i++) {
        TypeV_ObjectHeader* header = (TypeV_ObjectHeader*)core->gc.memObjects[i];
        if(header) {
            core_gc_free_header(core, header);
            core->gc.memObjects[i] = NULL;
        }
    }
    core->gc.memObjectCount = 0;
}


void core_gc_update_struct_field(TypeV_Core* core, TypeV_Struct* structPtr, void* ptr, uint16_t fieldIndex) {
    if (fieldIndex >= 255) {
        core_panic(core, -1, "Invalid index");
        return;
    }

    TypeV_ObjectHeader* header = (TypeV_ObjectHeader*)((char*)structPtr - sizeof(TypeV_ObjectHeader));
    header->ptrs[fieldIndex] = ptr;
}

void core_gc_update_class_field(TypeV_Core* core, TypeV_Class* classPtr, void* ptr, uint16_t fieldIndex) {
    if (fieldIndex >= 255) {
        core_panic(core, -1, "Invalid index");
        return;
    }

    TypeV_ObjectHeader* header = (TypeV_ObjectHeader*)((char*)classPtr - sizeof(TypeV_ObjectHeader));
    header->ptrs[fieldIndex] = ptr;
}


void core_gc_update_array_field(TypeV_Core* core, TypeV_Array* arrayPtr, void* ptr, uint64_t fieldIndex) {
    if (fieldIndex >= arrayPtr->length) {
        // Handle error: fieldIndex is out of bounds
        return;
    }

    TypeV_ObjectHeader* header = (TypeV_ObjectHeader*)((char*)arrayPtr - sizeof(TypeV_ObjectHeader));
    if (fieldIndex >= header->ptrsCount) {
        // Handle dynamic resizing of ptrs array if needed
        return;
    }

    header->ptrs[fieldIndex] = ptr;
}

void core_struct_free(TypeV_Core *core, TypeV_ObjectHeader* header) {
    // Ensure the header is valid and of the expected type
    if (!header || header->type != OT_STRUCT) {
        core_panic(core, -1, "Invalid header or type");
        return;
    }

    // Get the struct
    TypeV_Struct* struct_ptr = (TypeV_Struct*)(header + 1);

    // Free the ptrs array in the header
    if (header->ptrs) {
        free(header->ptrs);
        header->ptrs = NULL;
    }

    // Free the fieldOffsets array in the struct
    if (struct_ptr->fieldOffsets) {
        free(struct_ptr->fieldOffsets);
        struct_ptr->fieldOffsets = NULL;
    }

    // Free the globalFields array in the struct
    if (struct_ptr->globalFields) {
        free(struct_ptr->globalFields);
        struct_ptr->globalFields = NULL;
    }

    // `data[]` is automatically freed as part of `header`,
    // since it was allocated in the same contiguous block.

    // Free the entire object header (and struct within it)
    free(header);
}

void core_class_free(TypeV_Core *core, TypeV_ObjectHeader* header) {
    // Ensure the header is valid and of the expected type
    if (!header || header->type != OT_CLASS) {
        core_panic(core, -1, "Invalid header or type");
        return;
    }

    // Get a pointer to the class, which comes right after the header
    TypeV_Class* class_ptr = (TypeV_Class*)(header + 1);

    // Free the ptrs array in the header
    if (header->ptrs) {
        free(header->ptrs);
        header->ptrs = NULL;
    }

    // Free the methods array in the class
    if (class_ptr->methods) {
        free(class_ptr->methods);
        class_ptr->methods = NULL;
    }

    // Free the globalMethods array in the class
    if (class_ptr->globalMethods) {
        free(class_ptr->globalMethods);
        class_ptr->globalMethods = NULL;
    }

    // `data[]` does not need to be freed separately, as it is part of the contiguous allocation

    // Free the entire object, including header and class
    free(header);
}

void core_array_free(TypeV_Core *core, TypeV_ObjectHeader* header) {
    if (!header || header->type != OT_ARRAY) {
        LOG_ERROR("Invalid header or type for array deallocation");
        return;
    }

    // Access the array structure
    TypeV_Array* array_ptr = (TypeV_Array*)(header + 1);

    // Free ptrs array if allocated
    if (header->ptrs) {
        free(header->ptrs);
        header->ptrs = NULL;
    }

    // Free the data array in TypeV_Array
    if (array_ptr->data && (array_ptr->length > 0)) {
        free(array_ptr->data);
        array_ptr->data = NULL;
    }

    // Free the entire header (which includes the TypeV_Array structure)
    free(header);
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
        free(closure_ptr->upvalues);
        closure_ptr->upvalues = NULL;
    }

    // Free the entire header (which includes the TypeV_Closure structure)
    free(header);
}

void core_gc_free_header(TypeV_Core* core, TypeV_ObjectHeader* header) {
    switch(header->type) {
        case OT_STRUCT: {
            core_struct_free(core, header);
            break;
        }
        case OT_CLASS: {
            core_class_free(core, header);
            break;
        }
        case OT_ARRAY: {
            core_array_free(core, header);
            break;
        }
        case OT_CLOSURE:
            //printf("freeing closure\n");
            core_closure_free(core, header);
            break;
        case OT_COROUTINE:
            //printf("freeing coroutine\n");
            break;
        case OT_RAWMEM:
            //printf("freeing rawmem\n");
            break;
    }
}