//
// Created by praisethemoon on 21.11.23.
//

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "core.h"
#include "queue/queue.h"
#include "instructions/instructions.h"
#include "utils/log.h"
#include "stack/stack.h"

void core_init(TypeV_Core *core, uint32_t id, struct TypeV_Engine *engineRef) {
    core->id = id;
    core->state = CS_INITIALIZED;

    // Initialize Registers
    for (int i = 0; i < MAX_REG; i++) {
        core->registers.regs[i].u64 = 0;
    }
    core->registers.ip = 0;
    core->registers.fp = 0;
    core->registers.flags = 0;

    // Initialize Stack
    stack_init(core, 1024*1024);

    // Initialize Message Queue
    queue_init(&(core->messageInputQueue));

    // Initialize Constant Pool
    core->constantPool.pool = NULL;  // Assuming memory allocation happens elsewhere
    core->constantPool.length = 0;

    // Initialize Global Pool
    core->globalPool.pool = NULL;  // Assuming memory allocation happens elsewhere
    core->globalPool.length = 0;

    // Initialize Program
    core->program.bytecode = NULL;  // Assuming program loading happens elsewhere
    core->program.length = 0;

    // Initialize GC
    core->memTracker.classes = NULL;
    core->memTracker.classCount = 0;
    core->memTracker.interfaces = NULL;
    core->memTracker.interfaceCount = 0;
    core->memTracker.structs = NULL;
    core->memTracker.structCount = 0;

    core->engineRef = engineRef;
}

void core_setup(TypeV_Core *core, uint8_t* program, uint64_t programLength, uint8_t* constantPool, uint64_t constantPoolLength, uint8_t* globalPool, uint64_t globalPoolLength, uint64_t stackCapacity, uint64_t stackLimit){
    core->program.bytecode = program;
    core->program.length = programLength;

    core->constantPool.pool = constantPool;
    core->constantPool.length = constantPoolLength;

    core->globalPool.pool = globalPool;
    core->globalPool.length = globalPoolLength;

}

void core_vm(TypeV_Core *core) {
    while(1){
        TypeV_OpCode opcode = core->program.bytecode[core->registers.ip++];
        op_funcs[opcode](core);
    }
}

void core_deallocate(TypeV_Core *core) {
    stack_free(core);

    queue_deallocate(&(core->messageInputQueue));

    free(core->constantPool.pool);
    core->constantPool.pool = NULL;

    // Note: Program deallocation depends on how programs are loaded and managed
}

size_t core_alloc_struct(TypeV_Core *core, uint8_t numfields, size_t totalsize) {
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
    LOG_INFO("Allocating struct with %d fields and %d bytes, total allocated size: %d", numfields, totalsize, sizeof(size_t)+totalsize);
    TypeV_Struct* struct_ptr = (TypeV_Struct*)calloc(1, sizeof(size_t)+totalsize);
    struct_ptr->fieldOffsets = calloc(numfields, sizeof(uint16_t));
    return (size_t)struct_ptr;
}

size_t core_alloc_struct_shadow(TypeV_Core *core, uint8_t numfields, size_t originalStruct) {
    /**
     * A shadow copy is a struct whos data segment points to another struct's data segment.
     * A shadow copy has its own offset table
     */

    TypeV_Struct* original = (TypeV_Struct*)originalStruct;
    LOG_INFO("Allocating struct shadow of %p with %d fields, total allocated size: %d", (void*)originalStruct, numfields, 2*sizeof(size_t));
    // we allocated 2 pointers, one for the offset table, and one for the data segment
    TypeV_Struct* struct_ptr = (TypeV_Struct*)calloc(1, 2*sizeof(size_t));
    *(struct_ptr+sizeof(size_t)) = *(original+sizeof(size_t));
    struct_ptr->fieldOffsets = calloc(numfields, sizeof(uint16_t));

    // add to gc
    core->memTracker.structs = realloc(core->memTracker.structs, sizeof(size_t)*(core->memTracker.structCount+1));
    core->memTracker.structs[core->memTracker.structCount++] = struct_ptr;

    return (size_t)struct_ptr;
}


size_t core_alloc_class_fields(TypeV_Core *core, uint8_t numfields, size_t total_fields_size) {
    /**
     * Class layout in type-v
     * struct {uint16* methodsOffset, size_t* methods, uint16* fieldsOffset, void* data}
     * meaning that the first 2 pointers are for the method table, and the second 2 are for the field table
     * struct + two pointers distance, is equivalent to struct, hence we can theoretically create
     * a struct that shadows a class. Maybe for the future
     */

    // we allocate 3 pointers for methods offset, methods table pointer and fields offset
    // + the total fields size
    LOG_INFO("Allocating class with %d fields and %d bytes, total allocated size: %d", numfields, total_fields_size, (3*sizeof(size_t))+total_fields_size);
    TypeV_Class* class_ptr = (TypeV_Class*)calloc(1, (3*sizeof(size_t))+total_fields_size);
    class_ptr->fieldsOffset = calloc(numfields, sizeof(uint16_t));

    // add to gc
    core->memTracker.classes = realloc(core->memTracker.classes, sizeof(size_t)*(core->memTracker.classCount+1));
    core->memTracker.classes[core->memTracker.classCount++] = class_ptr;

    return (size_t)class_ptr;
}


void core_alloc_class_methods(TypeV_Core *core, uint8_t num_methods, TypeV_Class* class_ptr){
    LOG_INFO("Allocating class methods with %d methods, total allocated size: %d", num_methods, num_methods*sizeof(size_t));
    class_ptr->methodsOffset = calloc(num_methods, sizeof(uint16_t));
    // class methods offset table is sequential, since class objects are primitive entities
    // and cannot be a shadow copy of another class
    for(size_t i = 0; i < num_methods; i++){
        class_ptr->methodsOffset[i] = i*sizeof(size_t);
    }
    class_ptr->methods = calloc(num_methods, sizeof(size_t));
}

size_t core_alloc_interface(TypeV_Core *core, uint8_t num_methods, TypeV_Class * class_ptr){
    LOG_INFO("Allocating interface from class %p with %d methods, total allocated size: %d", (size_t)num_methods, num_methods*sizeof(size_t));
    TypeV_Interface* interface_ptr = (TypeV_Interface*)calloc(1, sizeof(size_t)*2);
    interface_ptr->methodsOffset = calloc(num_methods, sizeof(uint16_t)*num_methods);
    interface_ptr->classPtr = class_ptr;

    // add to gc
    core->memTracker.interfaces = realloc(core->memTracker.interfaces, sizeof(size_t)*(core->memTracker.interfaceCount+1));
    core->memTracker.interfaces[core->memTracker.interfaceCount++] = interface_ptr;

    return (size_t)interface_ptr;
}