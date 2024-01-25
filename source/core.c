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
#include "queue/queue.h"
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

    return state;
}

void core_init(TypeV_Core *core, uint32_t id, struct TypeV_Engine *engineRef) {
    core->id = id;
    core->state = CS_INITIALIZED;

    core->funcState = core_create_function_state(NULL);
    core->flags = &core->funcState->flags;
    core->regs = core->funcState->regs;

    // Initialize GC
    core->memTracker.classes = NULL;
    core->memTracker.classCount = 0;
    core->memTracker.interfaces = NULL;
    core->memTracker.interfaceCount = 0;
    core->memTracker.structs = NULL;
    core->memTracker.structCount = 0;
    core->memTracker.arrays = NULL;
    core->memTracker.arrayCount = 0;
    core->memTracker.memObjects = NULL;
    core->memTracker.memObjectCount = 0;

    core->engineRef = engineRef;
    core->lastSignal = CSIG_NONE;
    core->awaitingPromise = NULL;
}

void core_setup(TypeV_Core *core, const uint8_t* program, const uint8_t* constantPool, const uint8_t* globalPool){
    core->codePtr = program;
    core->constPtr = constantPool;
    core->globalPtr = globalPool;
    core->state = CS_RUNNING;
}


void core_deallocate(TypeV_Core *core) {
    stack_free(core);

    queue_deallocate(&(core->messageInputQueue));

    // free all memory objects
    for(size_t i = 0; i < core->memTracker.classCount; i++){
        free(core->memTracker.classes[i]);
    }
    if(core->memTracker.classes != NULL) {
        free(core->memTracker.classes);
    }

    for(size_t i = 0; i < core->memTracker.interfaceCount; i++){
        free(core->memTracker.interfaces[i]);
    }
    if(core->memTracker.interfaces != NULL) {
        free(core->memTracker.interfaces);
    }

    for(size_t i = 0; i < core->memTracker.structCount; i++){
        free(core->memTracker.structs[i]->fieldOffsets);
        free(core->memTracker.structs[i]);
    }
    if(core->memTracker.structs != NULL) {
        free(core->memTracker.structs);
    }

    for(size_t i = 0; i < core->memTracker.arrayCount; i++){
        free(core->memTracker.arrays[i]);
    }
    if(core->memTracker.arrays != NULL) {
        free(core->memTracker.arrays);
    }

    for(size_t i = 0; i < core->memTracker.memObjectCount; i++){
        free(core->memTracker.memObjects[i]);
    }
    if(core->memTracker.memObjects != NULL) {
        free(core->memTracker.memObjects);
    }


    // free stack
    stack_free(core);

    // Note: Program deallocation depends on how programs are loaded and managed
}

size_t core_struct_alloc(TypeV_Core *core, uint8_t numfields, size_t totalsize) {
    /**
     * Struct layout in type-v
     * struct {offsets: uint16_t*, data: void* }
     * offset is a pointer to the start of each field,
     * data is a pointer to the start of the actual struct data.
     *
     * to get the value of a field, we need to address of the struct,
     * lets say R16 contains this address
     * ADD R16, R16, size_of_pointer // since the first field is pointer
     * ADD R16, R16, offset_of_field // offset of the field we want to access
     * the offset_of_field is fetched from the struct[offset[index]]
     * where index is the field index.
     */
    // we want to data to have the following structure
    // [offset_pointer (size_t), data_block (totalsize)]
    LOG_INFO("CORE[%d]: Allocating struct with %d fields and %d bytes, total allocated size: %d", core->id, numfields, totalsize, sizeof(size_t)+totalsize);
    TypeV_Struct* struct_ptr = (TypeV_Struct*)calloc(1, sizeof(TypeV_Struct)+totalsize);
    struct_ptr->fieldOffsets = calloc(numfields, sizeof(uint16_t));
    struct_ptr->originalStruct = NULL;
    struct_ptr->dataPointer = &struct_ptr->data;

    // add to gc
    core->memTracker.structs = realloc(core->memTracker.structs, sizeof(size_t)*(core->memTracker.structCount+1));
    core->memTracker.structs[core->memTracker.structCount++] = struct_ptr;
    return (size_t)struct_ptr;
}

size_t core_struct_alloc_shadow(TypeV_Core *core, uint8_t numfields, size_t originalStruct) {
    /**
     * A shadow copy is a struct whos data segment points to another struct's data segment.
     * A shadow copy has its own offset table
     */

    TypeV_Struct* original = (TypeV_Struct*)originalStruct;
    LOG_INFO("CORE[%d]: Allocating struct shadow of %p with %d fields, total allocated size: %d", core->id, (void*)originalStruct, numfields, 2*sizeof(size_t));
    // we allocated 2 pointers, one for the offset table, and one for the data segment
    TypeV_Struct* struct_ptr = (TypeV_Struct*)calloc(1, sizeof (TypeV_Struct));
    struct_ptr->dataPointer = original->data;
    struct_ptr->fieldOffsets = calloc(numfields, sizeof(uint16_t));
    struct_ptr->originalStruct = original;

    // add to gc
    core->memTracker.structs = realloc(core->memTracker.structs, sizeof(size_t)*(core->memTracker.structCount+1));
    core->memTracker.structs[core->memTracker.structCount++] = struct_ptr;

    return (size_t)struct_ptr;
}


size_t core_class_alloc(TypeV_Core *core, uint8_t num_methods, size_t total_fields_size, uint64_t classId) {
    LOG_INFO("CORE[%d]: Allocating class with %d methods and %d bytes, total allocated size: %d, uid: %d", core->id, num_methods, total_fields_size, (3*sizeof(size_t))+total_fields_size, classId);
    TypeV_Class* class_ptr = (TypeV_Class*)calloc(1, sizeof(TypeV_Class)+total_fields_size);
    class_ptr->num_methods = num_methods;
    //class_ptr->methodsOffset = calloc(num_methods, sizeof(uint16_t));
    class_ptr->uid = classId;
    // class methods offset table is sequential, since class objects are primitive entities
    // and cannot be a shadow copy of another class
    /*for(size_t i = 0; i < num_methods; i++){
        // TODO: Replace size_t with 8?
        class_ptr->methodsOffset[i] = i*sizeof(uint16_t);
    }*/

    class_ptr->methods = calloc(num_methods, sizeof(size_t));

    // add to gc
    core->memTracker.classes = realloc(core->memTracker.classes, sizeof(size_t)*(core->memTracker.classCount+1));
    core->memTracker.classes[core->memTracker.classCount++] = class_ptr;

    return (size_t)class_ptr;
}

size_t core_interface_alloc(TypeV_Core *core, uint8_t num_methods, TypeV_Class * class_ptr){
    LOG_INFO("CORE[%d]: Allocating interface from class %p with %d methods, total allocated size: %d from class %d", core->id, (size_t)num_methods, num_methods*sizeof(size_t), class_ptr->uid);
    TypeV_Interface* interface_ptr = (TypeV_Interface*)calloc(1, sizeof(TypeV_Interface)+(sizeof(uint16_t)*num_methods));
    interface_ptr->classPtr = class_ptr;

    // add to gc
    core->memTracker.interfaces = realloc(core->memTracker.interfaces, sizeof(size_t)*(core->memTracker.interfaceCount+1));
    core->memTracker.interfaces[core->memTracker.interfaceCount++] = interface_ptr;

    return (size_t)interface_ptr;
}

size_t core_interface_alloc_i(TypeV_Core *core, uint8_t num_methods, TypeV_Interface* interface_ptr){
    LOG_INFO("CORE[%d]: Allocating interface from interface %p with %d methods, total allocated size: %d", core->id, (size_t)interface_ptr, num_methods*sizeof(size_t));
    TypeV_Interface* interface_ptr_new = (TypeV_Interface*)calloc(1, sizeof(TypeV_Interface)+(sizeof(uint16_t)*num_methods));
    interface_ptr_new->classPtr = interface_ptr_new->classPtr;

    // add to gc
    core->memTracker.interfaces = realloc(core->memTracker.interfaces, sizeof(size_t)*(core->memTracker.interfaceCount+1));
    core->memTracker.interfaces[core->memTracker.interfaceCount++] = interface_ptr_new;

    return (size_t)interface_ptr_new;
}

size_t core_array_alloc(TypeV_Core *core, uint64_t num_elements, uint8_t element_size) {
    LOG_INFO("CORE[%d]: Allocating array with %d elements of size %d, total allocated size: %d", core->id, num_elements, element_size, sizeof(size_t)+num_elements*element_size);
    TypeV_Array* array_ptr = (TypeV_Array*)calloc(1, sizeof(TypeV_Array));

    // add to gc
    core->memTracker.arrays = realloc(core->memTracker.arrays, sizeof(size_t)*(core->memTracker.arrayCount+1));
    core->memTracker.arrays[core->memTracker.arrayCount++] = array_ptr;

    array_ptr->elementSize = element_size;
    array_ptr->length = num_elements;
    array_ptr->data = calloc(num_elements, element_size);

    return (size_t)array_ptr;
}

uintptr_t core_array_slice(TypeV_Array* array, uint64_t start, uint64_t end){
    LOG_INFO("Slicing array %p from %d to %d", array, start, end);
    TypeV_Array* array_ptr = (TypeV_Array*)calloc(1, sizeof(TypeV_Array));
    array_ptr->elementSize = array->elementSize;
    array_ptr->length = end-start;
    array_ptr->data = malloc(array_ptr->length*array_ptr->elementSize);
    // todo: maybe replace memcpy
    memcpy(array_ptr->data, array->data+start*array_ptr->elementSize, array_ptr->length*array_ptr->elementSize);

    return (uintptr_t)array_ptr;
}

size_t core_array_extend(TypeV_Core *core, size_t array_ptr, uint64_t num_elements){
    LOG_INFO("Extending array %p with %"PRIu64" elements, total allocated size: %d", array_ptr, num_elements, num_elements*sizeof(size_t));
    TypeV_Array* array = (TypeV_Array*)array_ptr;
    array->data = realloc(array->data, num_elements*array->elementSize);
    array->length = num_elements;
    return array_ptr;
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
    void* mem =  calloc(1, size);

    // add mem to tracker
    core->memTracker.memObjects = realloc(core->memTracker.memObjects, sizeof(size_t)*(core->memTracker.memObjectCount+1));
    core->memTracker.memObjects[core->memTracker.memObjectCount++] = mem;

    return (size_t)mem;
}

void core_enqueue_message(TypeV_Core* core, TypeV_IOMessage* message) {
    queue_enqueue(&(core->messageInputQueue), message);
    core_queue_resolve(core);
}

void core_resume(TypeV_Core* core) {
    core->state = CS_RUNNING;
}

void core_halt(TypeV_Core* core) {
    core->state = CS_HALTED;
}

void core_queue_await(TypeV_Core* core) {
    core->state = CS_AWAITING_QUEUE;
}

void core_queue_resolve(TypeV_Core* core) {
    core->state = CS_RUNNING;
}

void core_receive_signal(TypeV_Core* core, TypeV_CoreSignal signal) {
    LOG_WARN("CORE[%d]: Received signal %s", core->id, signal == CSIG_KILL ? "KILL" : (signal == CSIG_TERMINATE ? "TERMINATE" : "NONE"));
    if(signal == CSIG_NONE) {return;}
    if(signal == CSIG_KILL) {
        core->lastSignal = CSIG_KILL;
    }
    if(signal == CSIG_TERMINATE){
        core->lastSignal = CSIG_TERMINATE;
    }
}


TypeV_Promise* core_promise_alloc(TypeV_Core* core) {
    static size_t promiseId = 0;
    TypeV_Promise* promise = calloc(1, sizeof(TypeV_Promise));
    promise->resolved = 0;
    promise->value = 0;
    promise->id  = promiseId++;
    return promise;
}

void core_promise_resolve(TypeV_Core* core, TypeV_Promise* promise, size_t value) {
    LOG_INFO("CORE[%d]: Resolving promise", core->id);
    promise->resolved = 1;
    promise->value = value;
}

void core_promise_await(TypeV_Core* core, TypeV_Promise* promise) {
    LOG_INFO("CORE[%d]: Awaiting promise %d", core->id, promise->id);
    core->state = CS_AWAITING_PROMISE;
    core->awaitingPromise = promise;
}

void core_promise_check_resume(TypeV_Core* core) {
    if(core->state == CS_AWAITING_PROMISE && core->awaitingPromise->resolved) {
        LOG_INFO("CORE[%d]: Resuming from promise %d", core->id, core->awaitingPromise->id);
        core->state = CS_RUNNING;
    }
}


TypeV_Lock* core_lock_alloc(TypeV_Core* core, size_t value){
    TypeV_Lock* lock = calloc(1, sizeof(TypeV_Lock));
    lock->locked = 0;
    lock->holder = 0;
    lock->value = value;
    lock->promise = core_promise_alloc(core);
    return lock;
}

void core_lock_acquire(TypeV_Core* core, TypeV_Lock* lock) {
    //ASSERT(lock->locked == 0, "Lock %p is already locked", lock->id);
    if(lock->locked == 1) {
        LOG_INFO("CORE[%d]: Lock %p is already locked, awaiting", core->id, lock->id);
        core_promise_await(core, lock->promise);
        return;
    }
    lock->locked = 1;
    lock->holder = core->id;
    // set promise
    lock->promise = core_promise_alloc(core);
}

void core_lock_release(TypeV_Core* core, TypeV_Lock* lock) {
    ASSERT(lock->holder == core->id, "Lock %p is not held by core %d", lock->id, core->id);
    lock->locked = 0;
    lock->holder = 0;
    // reset promise
    // TODO: request GC cleanup?
    core_promise_resolve(core, lock->promise, lock->value);
    lock->promise = NULL;
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
}
