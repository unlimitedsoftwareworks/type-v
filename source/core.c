//
// Created by praisethemoon on 21.11.23.
//
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "engine.h"
#include "core.h"
#include "utils/log.h"
#include "stack/stack.h"
#include "dynlib/dynlib.h"
#include "utils/utils.h"

TypeV_FuncState* core_create_function_state(TypeV_FuncState* prev){
    TypeV_FuncState* state = malloc(sizeof(TypeV_FuncState));

    state->sp = 0;
    stack_init(state, 1024);
    state->prev = prev;
    state->next = NULL;
    state->flags = 0;
    state->spillSlots = malloc(sizeof(TypeV_Register));
    state->spillSize = 1;

    return state;
}

TypeV_FuncState* core_duplicate_function_state(TypeV_FuncState* original) {
    TypeV_FuncState* state = core_create_function_state(original->prev);

    /*
    state->capacity = original->capacity;
    for(size_t i = 0; i < 256; i++) {
        state->regs[i] = original->regs[i];
    }

    state->spillSize = original->spillSize;
    for(size_t i = 0; i < state->spillSize; i++) {
        state->spillSlots[i] = original->spillSlots[i];
    }

    state->next = original->next;
    state->flags = original->flags;
    */

    memcpy(state, original, sizeof(TypeV_FuncState));

    return state;
}

void core_init(TypeV_Core *core, uint32_t id, struct TypeV_Engine *engineRef) {
    core->id = id;
    core->state = CS_INITIALIZED;

    core->funcState = core_create_function_state(NULL);
    core->flags = &core->funcState->flags;
    core->regs = core->funcState->regs;

    // Initialize GC
    core->gc.memObjectCapacity = 2000;
    core->gc.memObjectCount = 0;
    core->gc.memObjects = malloc(sizeof(void*)*core->gc.memObjectCapacity);
    core->gc.allocsSincePastGC = 0;
    core->gc.totalAllocs = 0;


    core->engineRef = engineRef;
    core->lastSignal = CSIG_NONE;
    core->awaitingPromise = NULL;
    core->activeCoroutine = NULL;
}

void core_setup(TypeV_Core *core, const uint8_t* program, const uint8_t* constantPool, const uint8_t* globalPool){
    core->codePtr = program;
    core->constPtr = constantPool;
    core->globalPtr = globalPool;
    core->state = CS_RUNNING;
}


void core_deallocate(TypeV_Core *core) {
    stack_free(core);


    // TODO: free mem objects


    // free stack
    stack_free(core);

    // Note: Program deallocation depends on how programs are loaded and managed
}


void* core_gc_alloc(TypeV_Core* core, size_t size) {
    if (core->gc.allocsSincePastGC > 2) {
        core_gc_collect(core);
        core->gc.allocsSincePastGC = 0;
    }

    if(core->gc.memObjectCount >= core->gc.memObjectCapacity) {
        core->gc.memObjectCapacity *= 2;
        core->gc.memObjects = realloc(core->gc.memObjects, sizeof(size_t)*(core->gc.memObjectCapacity));
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
    if (objectPtr == NULL) {
        return NULL;
    }

    // The header is just before the object memory in the allocation.

    return (TypeV_ObjectHeader*)((char*)objectPtr - sizeof(TypeV_ObjectHeader));
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
    if (header == NULL){ // || (header->marked == 1)) { causes fault
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

void core_gc_free_header(TypeV_Core* core, TypeV_ObjectHeader* header) {
    for(size_t i = 0; i < header->ptrsCount; i++) {
        void* ptr = header->ptrs[i];
        uint64_t idx = core_gc_is_valid_ptr(core, (uintptr_t)ptr);
        if(idx) {
            TypeV_ObjectHeader* pointedHeader = get_header_from_pointer(ptr);
            core_gc_free_header(core, pointedHeader);
            core->gc.memObjects[idx-1] = NULL;
            header->ptrs[i] = NULL;
        }
    }

    switch(header->type) {
        case OT_STRUCT: {
            TypeV_Struct* s = (TypeV_Struct*)(header + 1);
            //free(s->fieldOffsets);

            break;
        }
        case OT_CLASS: {
            TypeV_Class* c = (TypeV_Class *)(header + 1);
            //free(c->methods);
            break;
        }
        case OT_ARRAY: {
            TypeV_Array* a = (TypeV_Array*)(header + 1);
            //free(a->data);
            break;
        }
        case OT_CLOSURE:
        case OT_PROCESS:
        case OT_RAWMEM:
            break;
    }

    // Free the object
    //free(header);
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
    LOG_INFO("Collecting state for core %d", core->id);
    // TODO: uncomment and identify bugs
    //core_gc_collect_state(core, core->funcState);
}

void core_gc_collect_state(TypeV_Core* core, TypeV_FuncState* state) {
    // iterate through the registers
    for (size_t i = 0; i < MAX_REG; i++) {
        uintptr_t ptr = state->regs[i].ptr;
        if(core_gc_is_valid_ptr(core, ptr)) {
            TypeV_ObjectHeader* header = get_header_from_pointer((void*)ptr);
            //if(header->marked == 0) {
            core_gc_mark_object(core, header);

            // Mark original struct if shadow struct
            /*
            if(header->type == OT_STRUCT_SHADOW) {
                TypeV_Struct* structPtr = (TypeV_Struct*)(header + 1);
                if(structPtr->originalStruct != NULL) {
                    TypeV_ObjectHeader* originalHeader = get_header_from_pointer(structPtr->originalStruct);
                    core_gc_mark_object(core, originalHeader);
                }
            }
             */
        }
    }

    for(size_t i = 0; i < state->spillSize; i++) {
        uintptr_t ptr = state->spillSlots[i].ptr;
        if(core_gc_is_valid_ptr(core, ptr)) {
            TypeV_ObjectHeader* header = get_header_from_pointer((void*)ptr);
            //if(header->marked == 0) {
                core_gc_mark_object(core, header);
            //}
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
        // Handle error: fieldIndex is out of bounds
        return;
    }

    TypeV_ObjectHeader* header = (TypeV_ObjectHeader*)((char*)structPtr - sizeof(TypeV_ObjectHeader));
    header->ptrs[fieldIndex] = ptr;
}

void core_gc_update_class_field(TypeV_Core* core, TypeV_Class* classPtr, void* ptr, uint16_t fieldIndex) {
    if (fieldIndex >= 255) {
        // Handle error: fieldIndex is out of bounds
        return;
    }

    TypeV_ObjectHeader* header = (TypeV_ObjectHeader*)((char*)classPtr - sizeof(TypeV_ObjectHeader));
    header->ptrs[fieldIndex] = ptr;
}

void core_gc_update_process_field(TypeV_Core* core, TypeV_Class* classPtr, void* ptr, uint16_t fieldIndex) {
    // TODO: Implement
    // throw an error
    fprintf(stderr, "Not implemented yet\n");
    exit(-1);
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


size_t core_struct_alloc(TypeV_Core *core, uint8_t numfields, size_t totalsize) {
    // [offset_pointer (size_t), data_block (totalsize)]
    LOG_INFO("CORE[%d]: Allocating struct with %d fields and %d bytes, total allocated size: %d", core->id, numfields, totalsize, sizeof(size_t)+totalsize);

    size_t totalAllocationSize = sizeof(TypeV_ObjectHeader) + sizeof(TypeV_Struct) + totalsize;
    TypeV_ObjectHeader* header = (TypeV_ObjectHeader*)core_gc_alloc(core, totalAllocationSize);

    // Set header information
    header->marked = 0;
    header->type = OT_STRUCT;
    header->size = totalAllocationSize;
    header->ptrsCount = numfields;
    header->ptrs = calloc(numfields, sizeof(void*));

    // Get a pointer to the actual struct, which comes after the header
    TypeV_Struct* struct_ptr = (TypeV_Struct*)(header + 1);
    struct_ptr->numFields = numfields;
    struct_ptr->fieldOffsets = calloc(numfields, sizeof(uint16_t));
    struct_ptr->globalFields = calloc(numfields, sizeof(uint32_t));
    struct_ptr->dataPointer = &struct_ptr->data;

    core_gc_update_alloc(core, totalAllocationSize);

    return (size_t)struct_ptr;
}

uint8_t object_find_global_index(TypeV_Core * core, uint32_t* globalFields, uint8_t numFields , uint32_t globalID) {
    int left = 0;
    int right = numFields - 1;

    while (left <= right) {
       int mid = left + (right - left) / 2;

        if (globalFields[mid] == globalID) {
            return (uint8_t)mid;  // Return the index where the global ID is found
        } else if (globalFields[mid] < globalID) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }

    // unreachable, in theory
    core_panic(core, -1, "Global ID %d not found in field array", globalID);
    exit(-1);
}


size_t core_class_alloc(TypeV_Core *core, uint8_t num_methods, size_t total_fields_size, uint64_t classId) {
    LOG_INFO("CORE[%d]: Allocating class with %d methods and %d bytes, uid: %d", core->id, num_methods, total_fields_size, classId);

    size_t totalAllocationSize = sizeof(TypeV_ObjectHeader) + sizeof(TypeV_Class) + total_fields_size;
    TypeV_ObjectHeader* header = (TypeV_ObjectHeader*)core_gc_alloc(core, totalAllocationSize);

    // Set header information
    header->marked = 0;
    header->type = OT_CLASS;
    header->size = totalAllocationSize;
    header->ptrsCount = total_fields_size; // assume worse case, 1 pointer per byte
    header->ptrs = calloc(total_fields_size, sizeof(void*));

    // Get a pointer to the actual class, which comes after the header
    TypeV_Class* class_ptr = (TypeV_Class*)(header + 1);
    class_ptr->numMethods = num_methods;
    class_ptr->uid = classId;
    class_ptr->methods = calloc(num_methods, sizeof(size_t));
    class_ptr->globalMethods = calloc(num_methods, sizeof(uint32_t));

    // Initialize other class fields here if needed

    // Track the allocation with the GC
    core_gc_update_alloc(core, totalAllocationSize);

    return (size_t)class_ptr;
}

size_t core_array_alloc(TypeV_Core *core, uint64_t num_elements, uint8_t element_size) {
    LOG_INFO("CORE[%d]: Allocating array with %" PRIu64 " elements of size %d", core->id, num_elements, element_size);

    size_t totalAllocationSize = sizeof(TypeV_ObjectHeader) + sizeof(TypeV_Array);
    TypeV_ObjectHeader* header = (TypeV_ObjectHeader*)core_gc_alloc(core, totalAllocationSize);
    header->marked = 0;
    header->type = OT_ARRAY;
    header->size = totalAllocationSize;
    // we could extend bytecode to separate pointer arrays from data arrays
    header->ptrsCount = num_elements;
    header->ptrs = calloc(num_elements, sizeof(void*));

    TypeV_Array* array_ptr = (TypeV_Array*)(header + 1);
    array_ptr->elementSize = element_size;
    array_ptr->length = num_elements;
    array_ptr->data = (uint8_t*)(array_ptr + 1); // Data starts right after the array structure

    array_ptr->data = calloc(num_elements, element_size);

    core_gc_update_alloc(core, totalAllocationSize);

    return (size_t)array_ptr;
}

uintptr_t core_array_slice(TypeV_Core *core, TypeV_Array* array, uint64_t start, uint64_t end){
    LOG_INFO("Slicing array %p from %" PRIu64 " to %" PRIu64, array, start, end);

    size_t slice_length = end - start;
    size_t totalAllocationSize = sizeof(TypeV_ObjectHeader) + sizeof(TypeV_Array) + slice_length * array->elementSize;
    TypeV_ObjectHeader* header = (TypeV_ObjectHeader*)core_gc_alloc(core, totalAllocationSize);
    header->marked = 0;
    header->type = OT_ARRAY;
    header->size = totalAllocationSize;

    header->ptrsCount = slice_length;
    header->ptrs = calloc(slice_length, sizeof(void*));

    TypeV_Array* array_ptr = (TypeV_Array*)(header + 1);
    array_ptr->elementSize = array->elementSize;
    array_ptr->length = slice_length;
    array_ptr->data = (uint8_t*)(array_ptr + 1);

    memcpy(array_ptr->data, array->data + start * array->elementSize, slice_length * array->elementSize);

    core_gc_update_alloc(core, totalAllocationSize);

    return (uintptr_t)array_ptr;
}

size_t core_array_extend(TypeV_Core *core, size_t array_ptr, uint64_t num_elements){
    LOG_INFO("Extending array %p with %"PRIu64" elements, total allocated size: %d", array_ptr, num_elements, num_elements*sizeof(size_t));
    TypeV_Array* array = (TypeV_Array*)array_ptr;

    TypeV_ObjectHeader* header = get_header_from_pointer(array);
    header->size += num_elements * array->elementSize;

    header->ptrsCount = num_elements;
    header->ptrs = realloc(header->ptrs, num_elements* sizeof(void*));

    array->data = realloc(array->data, num_elements*array->elementSize);
    array->length = num_elements;

    return array_ptr;
}

uint64_t core_array_insert(TypeV_Core* core, TypeV_Array* dest, TypeV_Array* src, uint64_t position) {
    // If the source array has no elements, return the original position
    if (src->length == 0) {
        return position;
    }

    // Save the previous length and calculate the new length
    uint64_t prevLength = dest->length;
    uint64_t newLength = dest->length + src->length;

    // Extend the destination array to accommodate the new elements
    core_array_extend(core, (size_t)dest, newLength);

    // Shift the existing elements in the destination array to the right
    // memmove is used here to handle overlapping memory regions safely
    memmove(
            dest->data + (position + src->length) * dest->elementSize,  // Move from this position (shifted by src's length)
            dest->data + position * dest->elementSize,                  // Move the data starting from 'position'
            (prevLength - position) * dest->elementSize                 // Move the rest of the elements after 'position'
    );

    // Copy the new elements from the source array to the destination array at 'position'
    memcpy(
            dest->data + position * dest->elementSize,                  // Destination position to start copying
            src->data,                                                  // Source data to copy from
            src->length * dest->elementSize                             // Size of the data to copy
    );

    // Update the length of the destination array
    dest->length = newLength;

    // Return the new position pointing at the end of the inserted elements
    return position + src->length;
}



size_t core_ffi_load(TypeV_Core* core, size_t namePointer){
    char* name = (char*)namePointer;
    LOG_INFO("CORE[%d]: Loading FFI %s", core->id, name);
    TV_LibraryHandle lib = ffi_dynlib_load(name);
    ASSERT(lib != NULL, "Failed to load library %s", ffi_find_dynlib(name));
    void* openLib = ffi_dynlib_getsym(lib, "typev_ffi_open");
    ASSERT(openLib != NULL, "Failed to open library %s", ffi_find_dynlib(name));
    size_t (*openFunc)(TypeV_Core*) = openLib;
    return openFunc(core);
}

void core_ffi_close(TypeV_Core* core, size_t libHandle){
    LOG_INFO("CORE[%d]: Closing FFI %p", core->id, (void*)libHandle);
    TV_LibraryHandle lib = (TV_LibraryHandle)libHandle;
    ffi_dynlib_unload(lib);
}

size_t core_mem_alloc(TypeV_Core* core, size_t size) {
    LOG_INFO("CORE[%d]: Allocating %d bytes", core->id, size);
    return (size_t)core_gc_alloc(core, size);
}

void core_resume(TypeV_Core* core) {
    core->state = CS_RUNNING;
}

void core_halt(TypeV_Core* core) {
    core->state = CS_HALTED;
}

void core_panic(TypeV_Core* core, uint32_t errorId, char* fmt, ...) {
    char message[1024];
    va_list args;
    va_start(args, fmt);
    vsprintf(message, fmt, args);
    va_end(args);

    LOG_ERROR("CORE[%d]: PANIC: ErrorID: %d, Message %s", core->id, errorId, message);

    TypeV_ENV env = get_env();
    if(env_sourcemap_has(env)){
        LOG_ERROR("Stack trace:");
        // print first frame
        TypeV_SourcePoint point = env_sourcemap_get(env, core->ip);
        LOG_ERROR("function: %s at %s:%d:%d", point.func_name, point.file, point.line+1, point.column);
        TypeV_FuncState* state = core->funcState->prev;
        while (state != NULL) {
            TypeV_SourcePoint point = env_sourcemap_get(env, state->ip);
            LOG_ERROR("function: %s at %s:%d:%d", point.func_name, point.file, point.line + 1, point.column);
            state = state->prev;
        }
    }


    core->state = CS_CRASHED;
    exit(-1);
}

void core_spill_alloc(TypeV_Core* core, uint16_t size) {
    core->funcState->spillSlots = realloc(core->funcState->spillSlots, sizeof(TypeV_Register)*(size));
    core->funcState->spillSize = size;
}

TypeV_Closure* core_closure_alloc(TypeV_Core* core, uintptr_t fnPtr, uint8_t argsOffset, uint8_t envSize) {
    LOG_WARN("CORE[%d]: Allocating closure with %d bytes", core->id, envSize);
    size_t totalAllocationSize = sizeof(TypeV_ObjectHeader) + sizeof(TypeV_Closure);
    TypeV_ObjectHeader* header = (TypeV_ObjectHeader*)core_gc_alloc(core, totalAllocationSize);

    // Set header information
    header->marked = 0;
    header->type = OT_CLOSURE;
    header->size = totalAllocationSize;
    header->ptrsCount = 0;
    //header->ptrs = calloc(envSize, sizeof(void*));

    // Get a pointer to the actual closure, which comes after the header
    TypeV_Closure* closure_ptr = (TypeV_Closure*)(header + 1);
    //closure_ptr->fnPtr = fnPtr;
    closure_ptr->envCounter = 0;
    closure_ptr->offset = argsOffset;
    closure_ptr->fnAddress = fnPtr;
    closure_ptr->envSize = envSize;

    closure_ptr->upvalues = calloc(envSize, sizeof(TypeV_Register ));

    core_gc_update_alloc(core, totalAllocationSize);
    return closure_ptr;
}

TypeV_Coroutine* core_coroutine_alloc(TypeV_Core* core, TypeV_Closure* closure) {
    LOG_INFO("CORE[%d]: Allocating coroutine", core->id);
    size_t totalAllocationSize = sizeof(TypeV_ObjectHeader) + sizeof(TypeV_Coroutine);
    TypeV_ObjectHeader* header = (TypeV_ObjectHeader*)core_gc_alloc(core, totalAllocationSize);

    // Set header information
    header->marked = 0;
    header->type = OT_COROUTINE;
    header->size = totalAllocationSize;
    header->ptrsCount = 0;
    //header->ptrs = calloc(envSize, sizeof(void*));

    TypeV_Coroutine* coroutine_ptr = (TypeV_Coroutine*)(header + 1);
    // create a new function state
    coroutine_ptr->closure = closure;
    coroutine_ptr->state = core_create_function_state(core->funcState);
    coroutine_ptr->state->prev = core->funcState;
    coroutine_ptr->state->ip = closure->fnAddress;
    coroutine_ptr->executionState = TV_COROUTINE_CREATED;

    // initially, the pointer points to the function address
    coroutine_ptr->ip = closure->fnAddress;

    return coroutine_ptr;
}
