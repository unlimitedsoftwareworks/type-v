//
// Created by praisethemoon on 21.11.23.
//
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <mimalloc.h>

#include "stack/stack.h"
#include "gc.h"
#include "engine.h"
#include "core.h"
#include "utils/log.h"
#include "dynlib/dynlib.h"
#include "utils/utils.h"

TypeV_FuncState* core_create_function_state(TypeV_FuncState* prev){
    TypeV_FuncState* state = mi_malloc(sizeof(TypeV_FuncState));

    state->sp = 0;
    stack_init(state, 1024);
    state->prev = prev;
    state->next = NULL;
    state->spillSlots = mi_malloc(sizeof(TypeV_Register));
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
    core->gc = gc_init();


    core->engineRef = engineRef;
    core->lastSignal = CSIG_NONE;
    core->activeCoroutine = NULL;

    core->ip = 0;
}

void core_setup(TypeV_Core *core, const uint8_t* program, const uint8_t* constantPool, uint8_t* globalPool){
    core->codePtr = program;
    core->constPtr = constantPool;
    core->globalPtr = globalPool;
    core->state = CS_RUNNING;
}


void core_deallocate(TypeV_Core *core) {
    // first mi_free all the function states
    TypeV_FuncState* state = core->funcState;
    while(state != NULL) {
        TypeV_FuncState* next = state->next;
        core_free_function_state(core, state);
        state = next;
    }

    //core_gc_sweep_all(core);
    //mi_free(core->gc.memObjects);
    mi_free(core);
}

void core_free_function_state(TypeV_Core* core, TypeV_FuncState* state) {
    stack_free(state);
    mi_free(state->spillSlots);
    mi_free(state);
}

uintptr_t core_struct_alloc(TypeV_Core *core, uint8_t numfields, size_t totalsize) {
    LOG_INFO("CORE[%d]: Allocating struct with %d fields and %zu bytes, total allocated size: %zu",
             core->id, numfields, totalsize, sizeof(TypeV_ObjectHeader) + sizeof(TypeV_Struct) + totalsize + numfields * sizeof(uint16_t) + numfields * sizeof(uint32_t));

    size_t bitmaskSize = (numfields + 7) / 8;

    // Calculate the total allocation size for the struct
    size_t totalAllocationSize = sizeof(TypeV_ObjectHeader) + sizeof(TypeV_Struct)
                                 + bitmaskSize  // Pointer bitmask
                                 + totalsize // Data block size
                                 + numfields * sizeof(uint16_t)  // fieldOffsets array
                                 + numfields * sizeof(uint32_t); // globalFields array

    // Allocate the entire memory block
    TypeV_ObjectHeader* header = (TypeV_ObjectHeader*)gc_alloc(core, totalAllocationSize);

    // Initialize the object header
    header->marked = 1;
    header->type = OT_STRUCT;

    // Place the `TypeV_Struct` directly after the header
    TypeV_Struct* struct_ptr = (TypeV_Struct*)(header + 1);

    struct_ptr->numFields = numfields;


    // Set `fieldOffsets` and `globalFields` pointers based on `data`
    struct_ptr->fieldOffsets = (uint16_t*)(struct_ptr->data + totalsize);
    struct_ptr->globalFields = (uint32_t*)(struct_ptr->fieldOffsets + numfields);
    struct_ptr->pointerBitmask = (uint8_t*)(struct_ptr->globalFields+numfields);

    // zero out the bitmask
    memset(struct_ptr->pointerBitmask, 0, bitmaskSize);

    // Return the pointer to the struct
    return (uintptr_t)struct_ptr;
}


uintptr_t core_class_alloc(TypeV_Core *core, uint8_t num_methods, uint8_t num_attributes, size_t total_fields_size, uint64_t classId) {
    LOG_INFO("CORE[%d]: Allocating class with %d methods and %d bytes, uid: %d", core->id, num_methods, total_fields_size, classId);

    size_t bitmaskSize = (num_attributes + 7) / 8;

    size_t totalAllocationSize = sizeof(TypeV_ObjectHeader) + sizeof(TypeV_Class)
                                + bitmaskSize
                                + total_fields_size
                                + num_methods * sizeof(size_t)
                                + num_methods * sizeof(uint32_t)
                                + num_attributes * sizeof(uint16_t);

    TypeV_ObjectHeader* header = (TypeV_ObjectHeader*)gc_alloc(core, totalAllocationSize);

    // Set header information
    header->marked = 1;
    header->type = OT_CLASS;

    // Get a pointer to the actual class, which comes after the header
    TypeV_Class* class_ptr = (TypeV_Class*)(header + 1);
    class_ptr->numMethods = num_methods;
    class_ptr->numFields = num_attributes;
    class_ptr->uid = classId;
    class_ptr->methods = (size_t*)(class_ptr->data + total_fields_size);
    class_ptr->globalMethods = (uint32_t*)(class_ptr->methods + num_methods);
    class_ptr->fieldOffsets = (uint16_t*)(class_ptr->globalMethods + num_methods);
    class_ptr->pointerBitmask = (uint8_t*)(class_ptr->fieldOffsets + num_attributes);

    return (uintptr_t)class_ptr;
}

uintptr_t core_array_alloc(TypeV_Core *core, uint8_t is_pointer_container, uint64_t num_elements, uint8_t element_size) {
    LOG_INFO("CORE[%d]: Allocating array with %" PRIu64 " elements of size %d", core->id, num_elements, element_size);

    static uint32_t uid = 0;
    size_t totalAllocationSize = sizeof(TypeV_ObjectHeader) + sizeof(TypeV_Array);
    TypeV_ObjectHeader* header = (TypeV_ObjectHeader*)gc_alloc(core, totalAllocationSize);
    header->marked = 1;
    header->type = OT_ARRAY;
    // for arrays 0 means elements are not pointers
    // anything else means elements are pointers

    TypeV_Array* array_ptr = (TypeV_Array*)(header + 1);
    array_ptr->elementSize = element_size;
    array_ptr->length = num_elements;
    array_ptr->data = mi_malloc(num_elements* element_size);
    array_ptr->uid = uid++;
    array_ptr->isPointerContainer = is_pointer_container;

    return (uintptr_t)array_ptr;
}

uintptr_t core_array_slice(TypeV_Core *core, TypeV_Array* array, uint64_t start, uint64_t end){
    LOG_INFO("Slicing array %p from %" PRIu64 " to %" PRIu64, array, start, end);

    size_t slice_length = end - start;
    size_t totalAllocationSize = sizeof(TypeV_ObjectHeader) + sizeof(TypeV_Array) + slice_length * array->elementSize;
    TypeV_ObjectHeader* header = (TypeV_ObjectHeader*)gc_alloc(core, totalAllocationSize);
    header->marked = 1;
    header->type = OT_ARRAY;

    TypeV_Array* array_ptr = (TypeV_Array*)(header + 1);
    array_ptr->elementSize = array->elementSize;
    array_ptr->length = slice_length;
    array_ptr->data = mi_malloc(slice_length* array->elementSize);
    array_ptr->uid = 1000000-array->uid;
    array_ptr->isPointerContainer = array->isPointerContainer;

    memcpy(array_ptr->data, array->data + start * array->elementSize, slice_length * array->elementSize);

    return (uintptr_t)array_ptr;
}

uintptr_t core_array_extend(TypeV_Core *core, uintptr_t array_ptr, uint64_t num_elements){
    LOG_INFO("Extending array %p with %"PRIu64" elements, total allocated size: %d", array_ptr, num_elements, num_elements*sizeof(size_t));
    TypeV_Array* array = (TypeV_Array*)array_ptr;

    if(array == NULL) {
        core_panic(core, -1, "Null array");
    }

    array->data = mi_realloc(array->data, num_elements*array->elementSize);
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

    // Return the new position pointing at the end of the inserted elements
    return position + src->length;
}



/*
#define CACHE_SIZE 4

typedef struct {
    uint32_t globalID;
    uint8_t index;
} CacheEntry;

static CacheEntry cache[CACHE_SIZE] = { {0, -1} };  // Initialize cache entries with invalid index

uint8_t find_in_cache(uint32_t globalID) {
    for (int i = 0; i < CACHE_SIZE; ++i) {
        if (__builtin_expect(cache[i].globalID == globalID, 1)) {
            return cache[i].index;  // Cache hit
        }
    }
    return (uint8_t)-1;  // Cache miss
}

void update_cache(uint32_t globalID, uint8_t index) {
    // Shift entries to make room at the front (simple LRU policy)
    for (int i = CACHE_SIZE - 1; i > 0; --i) {
        cache[i] = cache[i - 1];
    }
    cache[0].globalID = globalID;
    cache[0].index = index;
}
 */

inline uint8_t object_find_global_index(TypeV_Core *core, uint32_t *globalFields, uint8_t numFields, uint32_t globalID) {
    // First, try to find the index in the cache
    /*int cache_index = find_in_cache(globalID);
    if (__builtin_expect(cache_index != (uint8_t)-1, 1)) {
        return cache_index;  // Cache hit
    }
     */

    // Perform binary search if cache miss
    int left = 0;
    int right = numFields - 1;
    while (__builtin_expect(left <= right, 1)) {  // Loop is likely to continue
        int mid = left + (right - left) / 2;

        if (__builtin_expect(globalFields[mid] == globalID, 1)) {  // Likely to find ID in the array
            //update_cache(globalID, (uint8_t)mid);  // Update cache with the result
            return (uint8_t)mid;
        } else if (globalFields[mid] < globalID) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }

    // If we reach here, it means the ID was not found
    // This is an unlikely case, so mark it as such
    if (__builtin_expect(1, 1)) {
        core_panic(core, -1, "Global ID %d not found in field array", globalID);
        exit(-1);
    }

    return -1;  // Return an invalid index (in theory, should not be reached)
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
    return (uintptr_t)gc_alloc(core, size);
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
    core->funcState->spillSlots = mi_realloc(core->funcState->spillSlots, sizeof(TypeV_Register)*(size));
    core->funcState->spillSize = size;
}

TypeV_Closure* core_closure_alloc(TypeV_Core* core, uintptr_t fnPtr, uint8_t argsOffset, uint8_t envSize) {
    LOG_WARN("CORE[%d]: Allocating closure with %d bytes", core->id, envSize);
    size_t totalAllocationSize = sizeof(TypeV_ObjectHeader) + sizeof(TypeV_Closure);
    TypeV_ObjectHeader* header = (TypeV_ObjectHeader*)gc_alloc(core, totalAllocationSize);

    // Set header information
    header->marked = 1;
    header->type = OT_CLOSURE;
    //header->ptrs = calloc(envSize, sizeof(void*));

    // Get a pointer to the actual closure, which comes after the header
    TypeV_Closure* closure_ptr = (TypeV_Closure*)(header + 1);
    //closure_ptr->fnPtr = fnPtr;
    closure_ptr->envCounter = 0;
    closure_ptr->offset = argsOffset;
    closure_ptr->fnAddress = fnPtr;
    closure_ptr->envSize = envSize;

    closure_ptr->upvalues = mi_malloc(envSize* sizeof(TypeV_Register ));

    return closure_ptr;
}

TypeV_Coroutine* core_coroutine_alloc(TypeV_Core* core, TypeV_Closure* closure) {
    LOG_INFO("CORE[%d]: Allocating coroutine", core->id);
    size_t totalAllocationSize = sizeof(TypeV_ObjectHeader) + sizeof(TypeV_Coroutine);
    TypeV_ObjectHeader* header = (TypeV_ObjectHeader*)gc_alloc(core, totalAllocationSize);

    // Set header information
    header->marked = 1;
    header->type = OT_COROUTINE;
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
