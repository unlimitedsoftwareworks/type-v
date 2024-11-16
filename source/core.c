//
// Created by praisethemoon on 21.11.23.
//
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "stack/stack.h"
#include "gc/gc.h"
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
    memset(state->regsPtrBitmap, 0, sizeof(state->regsPtrBitmap));

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

    state->ptrFields = malloc((original->spillSize + 7) / 8);
    memset(state->ptrFields, 0, (original->spillSize + 7) / 8);

    return state;
}

void core_init(TypeV_Core *core, uint32_t id, struct TypeV_Engine *engineRef) {
    core->id = id;
    core->state = CS_INITIALIZED;

    core->funcState = core_create_function_state(NULL);
    core->regs = core->funcState->regs;

    // Initialize GC
    core->gc = gc_initialize();


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
    // first free all the function states
    TypeV_FuncState* state = core->funcState;
    while(state != NULL) {
        TypeV_FuncState* next = state->next;
        core_free_function_state(core, state);
        state = next;
    }

    //core_gc_sweep_all(core);
    //free(core->gc.memObjects);
    free(core);
}

void core_free_function_state(TypeV_Core* core, TypeV_FuncState* state) {
    stack_free(state);
    free(state->spillSlots);
    free(state);
}

#include <stdalign.h>

#define ALIGN_PTR(ptr, alignment) \
    ((uintptr_t)(((uintptr_t)(ptr) + (alignment - 1)) & ~(alignment - 1)))

uintptr_t core_struct_alloc(TypeV_Core *core, uint8_t numfields, size_t totalsize) {
    LOG_INFO("CORE[%d]: Allocating struct with %d fields and %zu bytes, total allocated cellSize: %zu",
             core->id, numfields, totalsize, sizeof(TypeV_ObjectHeader) + sizeof(TypeV_Struct) + totalsize + numfields * sizeof(uint16_t) + numfields * sizeof(uint32_t));

    // Ensure minimum data size is 8 bytes for alignment purposes
    if (totalsize < 8) {
        totalsize = 8;
    }

    // Calculate sizes for each component
    size_t bitmaskSize = (numfields + 7) / 8;
    size_t totalAllocationSize = sizeof(TypeV_ObjectHeader) + sizeof(TypeV_Struct);

    // Align for `globalFields` (4-byte alignment)
    totalAllocationSize = ALIGN_PTR(totalAllocationSize, alignof(uint32_t));
    totalAllocationSize += numfields * sizeof(uint32_t);  // Global fields array

    // Align for `fieldOffsets` (2-byte alignment)
    totalAllocationSize = ALIGN_PTR(totalAllocationSize, alignof(uint16_t));
    totalAllocationSize += numfields * sizeof(uint16_t);  // Field offsets array

    // Align for `pointerBitmask` (1-byte alignment)
    totalAllocationSize = ALIGN_PTR(totalAllocationSize, alignof(uint8_t));
    totalAllocationSize += bitmaskSize;  // Pointer bitmask size

    // Align for `data` (8-byte alignment)
    totalAllocationSize = ALIGN_PTR(totalAllocationSize, alignof(uint64_t));
    totalAllocationSize += totalsize;  // Data block size

    // Allocate the entire memory block
    TypeV_ObjectHeader* header = (TypeV_ObjectHeader*)gc_alloc(core, totalAllocationSize);
    static uint32_t uid = 0;

    // Initialize the object header
    header->type = OT_STRUCT;
    header->totalSize = totalAllocationSize;
    header->survivedCount = 0;

    // Place the `TypeV_Struct` directly after the header
    TypeV_Struct* struct_ptr = (TypeV_Struct*)(header + 1);
    struct_ptr->numFields = numfields;
    struct_ptr->uid = uid++;

    // Set pointers for subsequent arrays with alignment adjustments

    uint8_t* current_ptr = (uint8_t*)(struct_ptr + 1);

    // Set `globalFields` pointer (aligned to 4 bytes)
    current_ptr = (uint8_t*)ALIGN_PTR(current_ptr, alignof(uint32_t));
    struct_ptr->globalFields = (uint32_t*)current_ptr;
    current_ptr += numfields * sizeof(uint32_t);

    // Set `fieldOffsets` pointer (aligned to 2 bytes)
    current_ptr = (uint8_t*)ALIGN_PTR(current_ptr, alignof(uint16_t));
    struct_ptr->fieldOffsets = (uint16_t*)current_ptr;
    current_ptr += numfields * sizeof(uint16_t);

    // Set `pointerBitmask` pointer (aligned to 1 byte)
    current_ptr = (uint8_t*)ALIGN_PTR(current_ptr, alignof(uint8_t));
    struct_ptr->pointerBitmask = current_ptr;

    // Set `data` pointer (aligned to 8 bytes)
    current_ptr = (uint8_t*)ALIGN_PTR(current_ptr, alignof(uint64_t));
    struct_ptr->data = current_ptr;
    current_ptr += totalsize;


    // Set `bitMaskSize`
    struct_ptr->bitMaskSize = bitmaskSize;

    // Zero out the bitmask
    memset(struct_ptr->pointerBitmask, 0, bitmaskSize);

    // Return the pointer to the struct
    return (uintptr_t)struct_ptr;
}

uintptr_t core_class_alloc(TypeV_Core *core, uint8_t num_methods, uint8_t num_attributes, size_t total_fields_size, uint64_t classId) {
    LOG_INFO("CORE[%d]: Allocating class with %d methods and %d attributes, uid: %llu", core->id, num_methods, num_attributes, classId);

    // Calculate bitmask size for pointers
    size_t bitmaskSize = (num_attributes + 7) / 8;

    // Ensure minimum data size for alignment purposes
    if (total_fields_size < 8) {
        total_fields_size = 8;
    }

    // Start calculating the total size required for allocation
    size_t totalAllocationSize = sizeof(TypeV_ObjectHeader) + sizeof(TypeV_Class);

    // Align for `methods` (8-byte alignment)
    totalAllocationSize = ALIGN_PTR(totalAllocationSize, alignof(uint64_t));
    totalAllocationSize += num_methods * sizeof(uint64_t);  // Methods array

    // Align for `globalMethods` (4-byte alignment)
    totalAllocationSize = ALIGN_PTR(totalAllocationSize, alignof(uint32_t));
    totalAllocationSize += num_methods * sizeof(uint32_t);  // Global methods array

    // Align for `fieldOffsets` (2-byte alignment)
    totalAllocationSize = ALIGN_PTR(totalAllocationSize, alignof(uint16_t));
    totalAllocationSize += num_attributes * sizeof(uint16_t);  // Field offsets array

    // Align for `pointerBitmask` (1-byte alignment)
    totalAllocationSize = ALIGN_PTR(totalAllocationSize, alignof(uint8_t));
    totalAllocationSize += bitmaskSize;  // Pointer bitmask size

    // Align for `data` (8-byte alignment)
    totalAllocationSize = ALIGN_PTR(totalAllocationSize, alignof(uint64_t));
    totalAllocationSize += total_fields_size;  // Data block size

    // Allocate memory for the entire class object
    TypeV_ObjectHeader* header = (TypeV_ObjectHeader*)gc_alloc(core, totalAllocationSize);

    // Set header information
    header->type = OT_CLASS;
    header->totalSize = totalAllocationSize;
    header->survivedCount = 0;  // Assuming survived count is initialized to 0

    // Get a pointer to the class, which comes after the header
    TypeV_Class* class_ptr = (TypeV_Class*)(header + 1);
    class_ptr->numMethods = num_methods;
    class_ptr->numFields = num_attributes;
    class_ptr->uid = classId;

    // Set pointers for subsequent arrays, ensuring alignment is handled

    uint8_t* current_ptr = (uint8_t*)(class_ptr + 1);

    // Set `methods` pointer (aligned to 8 bytes)
    current_ptr = (uint8_t*)ALIGN_PTR(current_ptr, alignof(uint64_t));
    class_ptr->methods = (uint64_t*)current_ptr;
    current_ptr += num_methods * sizeof(uint64_t);

    // Set `globalMethods` pointer (aligned to 4 bytes)
    current_ptr = (uint8_t*)ALIGN_PTR(current_ptr, alignof(uint32_t));
    class_ptr->globalMethods = (uint32_t*)current_ptr;
    current_ptr += num_methods * sizeof(uint32_t);

    // Set `fieldOffsets` pointer (aligned to 2 bytes)
    current_ptr = (uint8_t*)ALIGN_PTR(current_ptr, alignof(uint16_t));
    class_ptr->fieldOffsets = (uint16_t*)current_ptr;
    current_ptr += num_attributes * sizeof(uint16_t);

    // Set `pointerBitmask` pointer (aligned to 1 byte)
    current_ptr = (uint8_t*)ALIGN_PTR(current_ptr, alignof(uint8_t));
    class_ptr->pointerBitmask = current_ptr;
    current_ptr += bitmaskSize;

    // Set `data` pointer (aligned to 8 bytes)
    current_ptr = (uint8_t*)ALIGN_PTR(current_ptr, alignof(uint64_t));
    class_ptr->data = current_ptr;

    // Zero out the bitmask
    memset(class_ptr->pointerBitmask, 0, bitmaskSize);

    // Return the pointer to the class
    return (uintptr_t)class_ptr;
}

uintptr_t core_array_alloc(TypeV_Core *core, uint8_t is_pointer_container, uint64_t num_elements, uint8_t element_size) {
    LOG_INFO("CORE[%d]: Allocating array with %" PRIu64 " elements of cellSize %d", core->id, num_elements, element_size);

    static uint32_t uid = 0;
    size_t totalAllocationSize = sizeof(TypeV_ObjectHeader) + sizeof(TypeV_Array);
    TypeV_ObjectHeader* header = (TypeV_ObjectHeader*)gc_alloc(core, totalAllocationSize);
    header->type = OT_ARRAY;
    // for arrays 0 means elements are not pointers
    // anything else means elements are pointers

    TypeV_Array* array_ptr = (TypeV_Array*)(header + 1);
    array_ptr->elementSize = element_size;
    array_ptr->length = num_elements;
    array_ptr->data = malloc(num_elements* element_size);
    array_ptr->uid = uid++;
    array_ptr->isPointerContainer = is_pointer_container;

    return (uintptr_t)array_ptr;
}

uintptr_t core_array_slice(TypeV_Core *core, TypeV_Array* array, uint64_t start, uint64_t end){
    LOG_INFO("Slicing array %p from %" PRIu64 " to %" PRIu64, array, start, end);

    size_t slice_length = end - start;
    size_t totalAllocationSize = sizeof(TypeV_ObjectHeader) + sizeof(TypeV_Array) + slice_length * array->elementSize;
    TypeV_ObjectHeader* header = (TypeV_ObjectHeader*)gc_alloc(core, totalAllocationSize);
    header->type = OT_ARRAY;

    TypeV_Array* array_ptr = (TypeV_Array*)(header + 1);
    array_ptr->elementSize = array->elementSize;
    array_ptr->length = slice_length;
    array_ptr->data = malloc(slice_length* array->elementSize);
    array_ptr->uid = 1000000-array->uid;
    array_ptr->isPointerContainer = array->isPointerContainer;

    memcpy(array_ptr->data, array->data + start * array->elementSize, slice_length * array->elementSize);

    return (uintptr_t)array_ptr;
}

uintptr_t core_array_extend(TypeV_Core *core, uintptr_t array_ptr, uint64_t num_elements){
    LOG_INFO("Extending array %p with %"PRIu64" elements, total allocated cellSize: %d", array_ptr, num_elements, num_elements*sizeof(size_t));
    TypeV_Array* array = (TypeV_Array*)array_ptr;

    if(array == NULL) {
        core_panic(core, -1, "Null array");
    }

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
    }     */


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
    core->funcState->spillSlots = realloc(core->funcState->spillSlots, sizeof(TypeV_Register)*(size));
    core->funcState->spillSize = size;
}


TypeV_Closure* core_closure_alloc(TypeV_Core* core, uintptr_t fnPtr, uint8_t argsOffset, uint8_t envSize) {
    LOG_WARN("CORE[%d]: Allocating closure with %d bytes", core->id, envSize);

    // Calculate bitmask size for pointers
    size_t bitmaskSize = (envSize + 7) / 8;

    // Start calculating the total size required for allocation
    size_t totalAllocationSize = sizeof(TypeV_ObjectHeader) + sizeof(TypeV_Closure);

    // Align for `upvalues` (8-byte alignment)
    totalAllocationSize = ALIGN_PTR(totalAllocationSize, alignof(uint64_t));
    totalAllocationSize += envSize * sizeof(TypeV_Register);  // Upvalues array

    // Align for `ptrFields` (1-byte alignment)
    totalAllocationSize = ALIGN_PTR(totalAllocationSize, alignof(uint8_t));
    totalAllocationSize += bitmaskSize;  // Pointer bitmask size

    // Allocate memory for the entire closure object
    TypeV_ObjectHeader* header = (TypeV_ObjectHeader*)gc_alloc(core, totalAllocationSize);

    // Set header information
    header->type = OT_CLOSURE;
    header->totalSize = totalAllocationSize;
    header->survivedCount = 0;

    // Get a pointer to the actual closure, which comes after the header
    TypeV_Closure* closure_ptr = (TypeV_Closure*)(header + 1);
    closure_ptr->fnAddress = fnPtr;
    closure_ptr->envCounter = 0;
    closure_ptr->offset = argsOffset;
    closure_ptr->envSize = envSize;

    // Set pointers for subsequent arrays, ensuring alignment is handled
    uint8_t* current_ptr = (uint8_t*)(closure_ptr + 1);

    // Set `upvalues` pointer (aligned to 8 bytes)
    current_ptr = (uint8_t*)ALIGN_PTR(current_ptr, alignof(uint64_t));
    closure_ptr->upvalues = (TypeV_Register*)current_ptr;
    current_ptr += envSize * sizeof(TypeV_Register);

    // Set `ptrFields` pointer (aligned to 1 byte)
    current_ptr = (uint8_t*)ALIGN_PTR(current_ptr, alignof(uint8_t));
    closure_ptr->ptrFields = current_ptr;

    // Zero out the bitmask
    memset(closure_ptr->ptrFields, 0, bitmaskSize);

    // Return the pointer to the closure
    return closure_ptr;
}

TypeV_Coroutine* core_coroutine_alloc(TypeV_Core* core, TypeV_Closure* closure) {
    LOG_INFO("CORE[%d]: Allocating coroutine", core->id);
    size_t totalAllocationSize = sizeof(TypeV_ObjectHeader) + sizeof(TypeV_Coroutine);
    TypeV_ObjectHeader* header = (TypeV_ObjectHeader*)gc_alloc(core, totalAllocationSize);

    // Set header information
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
