//
// Created by praisethemoon on 21.11.23.
//
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "stack/stack.h"
#include "gc.h"
#include "engine.h"
#include "core.h"
#include "utils/log.h"
#include "dynlib/dynlib.h"
#include "utils/utils.h"

TypeV_FuncState* core_create_function_state(TypeV_FuncState* prev){
    TypeV_FuncState* state = malloc(sizeof(TypeV_FuncState));

    state->sp = 0;
    stack_init(state, 1024);
    state->prev = prev;
    state->next = NULL;
    state->spillSlots = malloc(sizeof(TypeV_Register));
    state->spillSize = 1;

    return state;
}

TypeV_FuncState* core_duplicate_function_state(TypeV_FuncState* original) {
    TypeV_FuncState* state = core_create_function_state(original->prev);

    state->capacity = original->capacity;
    for(size_t i = 0; i < 256; i++) {
        state->regs[i] = original->regs[i];
    }

    state->spillSize = original->spillSize;
    for(size_t i = 0; i < state->spillSize; i++) {
        state->spillSlots[i] = original->spillSlots[i];
    }

    state->next = original->next;

    return state;
}

void core_init(TypeV_Core *core, uint32_t id, struct TypeV_Engine *engineRef) {
    core->id = id;
    core->state = CS_INITIALIZED;

    core->funcState = core_create_function_state(NULL);
    core->regs = core->funcState->regs;

    // Initialize GC
    core->gc.memObjectCapacity = 2000;
    core->gc.memObjectCount = 0;
    core->gc.memObjects = malloc(sizeof(void*)*core->gc.memObjectCapacity);
    core->gc.allocsSincePastGC = 0;
    core->gc.totalAllocs = 0;


    core->engineRef = engineRef;
    core->lastSignal = CSIG_NONE;
    core->activeCoroutine = NULL;
}

void core_setup(TypeV_Core *core, const uint8_t* program, const uint8_t* constantPool, uint8_t* globalPool){
    core->codePtr = program;
    core->constPtr = constantPool;
    core->globalPtr = globalPool;
    core->state = CS_RUNNING;
}


void core_deallocate(TypeV_Core *core) {
    // first free all the function states
    TypeV_FuncState* state = core->funcState;
    while(state != NULL) {
        TypeV_FuncState* next = state->next;
        core_free_function_state(core, state);
        state = next;
    }

    core_gc_sweep_all(core);
    free(core->gc.memObjects);
    free(core);
}

void core_free_function_state(TypeV_Core* core, TypeV_FuncState* state) {
    stack_free(state);
    free(state->spillSlots);
    free(state);
}

uintptr_t core_struct_alloc(TypeV_Core *core, uint8_t numfields, size_t totalsize) {
    // [offset_pointer (size_t), data_block (totalsize)]
    LOG_INFO("CORE[%d]: Allocating struct with %d fields and %d bytes, total allocated size: %d", core->id, numfields, totalsize, sizeof(size_t)+totalsize);


    size_t totalAllocationSize = sizeof(TypeV_ObjectHeader) + sizeof(TypeV_Struct) + totalsize;
    TypeV_ObjectHeader* header = (TypeV_ObjectHeader*)core_gc_alloc(core, totalAllocationSize);
    static uint32_t uid = 0;
    // Set header information
    header->marked = 0;
    header->type = OT_STRUCT;
    header->size = totalAllocationSize;
    header->ptrsCount = numfields;
    header->ptrs = malloc(numfields*sizeof(void*));

    // Get a pointer to the actual struct, which comes after the header
    TypeV_Struct* struct_ptr = (TypeV_Struct*)(header + 1);
    struct_ptr->numFields = numfields;
    struct_ptr->fieldOffsets = malloc(numfields*sizeof(uint16_t));
    struct_ptr->globalFields = malloc(numfields*sizeof(uint32_t));
    struct_ptr->dataPointer = struct_ptr->data;
    struct_ptr->uid = uid++;
    core_gc_update_alloc(core, totalAllocationSize);

    return (uintptr_t)struct_ptr;
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

uintptr_t core_class_alloc(TypeV_Core *core, uint8_t num_methods, size_t total_fields_size, uint64_t classId) {
    LOG_INFO("CORE[%d]: Allocating class with %d methods and %d bytes, uid: %d", core->id, num_methods, total_fields_size, classId);

    size_t totalAllocationSize = sizeof(TypeV_ObjectHeader) + sizeof(TypeV_Class) + total_fields_size;
    TypeV_ObjectHeader* header = (TypeV_ObjectHeader*)core_gc_alloc(core, totalAllocationSize);

    // Set header information
    header->marked = 0;
    header->type = OT_CLASS;
    header->size = totalAllocationSize;
    header->ptrsCount = total_fields_size; // assume worse case, 1 pointer per byte
    header->ptrs = malloc(total_fields_size* sizeof(void*));

    // Get a pointer to the actual class, which comes after the header
    TypeV_Class* class_ptr = (TypeV_Class*)(header + 1);
    class_ptr->numMethods = num_methods;
    class_ptr->uid = classId;
    class_ptr->methods = malloc(num_methods*sizeof(size_t));
    class_ptr->globalMethods = malloc(num_methods*sizeof(uint32_t));

    // Initialize other class fields here if needed

    // Track the allocation with the GC
    core_gc_update_alloc(core, totalAllocationSize);

    return (uintptr_t)class_ptr;
}

uintptr_t core_array_alloc(TypeV_Core *core, uint64_t num_elements, uint8_t element_size) {
    LOG_INFO("CORE[%d]: Allocating array with %" PRIu64 " elements of size %d", core->id, num_elements, element_size);

    static uint32_t uid = 0;
    size_t totalAllocationSize = sizeof(TypeV_ObjectHeader) + sizeof(TypeV_Array);
    TypeV_ObjectHeader* header = (TypeV_ObjectHeader*)core_gc_alloc(core, totalAllocationSize);
    header->marked = 0;
    header->type = OT_ARRAY;
    header->size = totalAllocationSize;
    // we could extend bytecode to separate pointer arrays from data arrays
    header->ptrsCount = num_elements;
    header->ptrs = malloc(num_elements*sizeof(void*));

    TypeV_Array* array_ptr = (TypeV_Array*)(header + 1);
    array_ptr->elementSize = element_size;
    array_ptr->length = num_elements;
    array_ptr->data = malloc(num_elements* element_size);
    array_ptr->uid = uid++;

    core_gc_update_alloc(core, totalAllocationSize);

    return (uintptr_t)array_ptr;
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

    TypeV_Array* array_ptr = (TypeV_Array*)(header + 1);
    array_ptr->elementSize = array->elementSize;
    array_ptr->length = slice_length;
    array_ptr->data = malloc(slice_length* array->elementSize);
    array_ptr->uid = 1000000-array->uid;

    TypeV_ObjectHeader* originalHeader = get_header_from_pointer(array);
    if(originalHeader->ptrsCount > 0) {
        header->ptrs = malloc(slice_length* sizeof(void*));
        for(size_t i = 0; i < slice_length; i++) {
            header->ptrs[i] = originalHeader->ptrs[start + i];
        }
    }

    memcpy(array_ptr->data, array->data + start * array->elementSize, slice_length * array->elementSize);
    core_gc_update_alloc(core, totalAllocationSize);

    return (uintptr_t)array_ptr;
}

uintptr_t core_array_extend(TypeV_Core *core, uintptr_t array_ptr, uint64_t num_elements){
    LOG_INFO("Extending array %p with %"PRIu64" elements, total allocated size: %d", array_ptr, num_elements, num_elements*sizeof(size_t));
    TypeV_Array* array = (TypeV_Array*)array_ptr;

    if(array == NULL) {
        core_panic(core, -1, "Null array");
    }

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
    core_array_extend(core, (uintptr_t)dest, newLength);

    // Shift the existing elements in the destination array to the right
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

    // Get the headers for both destination and source arrays
    TypeV_ObjectHeader* header = get_header_from_pointer((void*)dest);
    TypeV_ObjectHeader* addedHeader = get_header_from_pointer((void*)src);

    // Check if there are pointers in the source array
    if (addedHeader->ptrsCount > 0) {
        // Reallocate the destination's ptrs array to accommodate the new pointers from src
        header->ptrs = realloc(header->ptrs, newLength * sizeof(void*));

        // Shift the existing pointers in the destination's `ptrs` array to the right
        memmove(
                header->ptrs + position + src->length,  // New location for shifted pointers
                header->ptrs + position,                // Current pointers from 'position' onward
                (prevLength - position) * sizeof(void*) // Number of pointers to shift
        );

        // Insert pointers from `src` and pad the rest with NULLs
        for (size_t i = 0; i < src->length; i++) {
            header->ptrs[position + i] = addedHeader->ptrs[i];
        }

        // Pad any remaining gaps with NULLs
        for (size_t i = prevLength; i < newLength; i++) {
            if (header->ptrs[i] == NULL) {
                header->ptrs[i] = NULL;
            }
        }

        // Update the pointer count accurately
        header->ptrsCount = newLength;
    }

    // Return the new position pointing at the end of the inserted elements
    return position + src->length;
}





uintptr_t core_ffi_load(TypeV_Core* core, uintptr_t namePointer){
    char* name = (char*)namePointer;
    LOG_INFO("CORE[%d]: Loading FFI %s", core->id, name);
    TV_LibraryHandle lib = ffi_dynlib_load(name);
    ASSERT(lib != NULL, "Failed to load library %s", ffi_find_dynlib(name));
    void* openLib = ffi_dynlib_getsym(lib, "typev_ffi_open");
    ASSERT(openLib != NULL, "Failed to open library %s", ffi_find_dynlib(name));
    uintptr_t (*openFunc)(TypeV_Core*) = openLib;
    return openFunc(core);
}

void core_ffi_close(TypeV_Core* core, uintptr_t libHandle){
    LOG_INFO("CORE[%d]: Closing FFI %p", core->id, (void*)libHandle);
    TV_LibraryHandle lib = (TV_LibraryHandle)libHandle;
    ffi_dynlib_unload(lib);
}

uintptr_t core_mem_alloc(TypeV_Core* core, uintptr_t size) {
    LOG_INFO("CORE[%d]: Allocating %d bytes", core->id, size);
    return (uintptr_t)core_gc_alloc(core, size);
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
    vsnprintf(message, sizeof(message), fmt, args);
    va_end(args);

    LOG_ERROR("CORE[%d]: PANIC: ErrorID: %d, Message %s", core->id, errorId, message);

    TypeV_ENV env = get_env();
    if (env_sourcemap_has(env)) {
        LOG_ERROR("Stack trace:");
        // Print first frame
        TypeV_SourcePoint point = env_sourcemap_get(env, core->ip);
        LOG_ERROR("function: %s at %s:%d:%d", point.func_name, point.file, point.line + 1, point.column);

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

    closure_ptr->upvalues = malloc(envSize* sizeof(TypeV_Register ));

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
    // add the closure to the coroutine
    header->ptrsCount = 1;
    header->ptrs = malloc(1*sizeof(void*));
    header->ptrs[0] = (void*)closure;

    TypeV_Coroutine* coroutine_ptr = (TypeV_Coroutine*)(header + 1);
    // create a new function state
    coroutine_ptr->closure = closure;
    coroutine_ptr->state = core_create_function_state(core->funcState);
    coroutine_ptr->state->prev = core->funcState;
    coroutine_ptr->state->ip = closure->fnAddress;
    coroutine_ptr->executionState = TV_COROUTINE_CREATED;

    // initially, the pointer points to the function address
    coroutine_ptr->ip = closure->fnAddress;
    core_gc_update_alloc(core, totalAllocationSize);

    return coroutine_ptr;
}
