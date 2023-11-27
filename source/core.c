//
// Created by praisethemoon on 21.11.23.
//

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "core.h"
#include "queue/queue.h"
#include "instructions/instructions.h"

void core_init(TypeV_Core *core, uint32_t id, struct TypeV_Engine *engineRef) {
    core->id = id;
    core->state = CS_INITIALIZED;

    // Initialize Registers
    for (int i = 0; i < 16; i++) {
        core->registers.regs[i].u64 = 0;
    }
    core->registers.ip = 0;
    core->registers.fp = 0;
    core->registers.flags = 0;

    // Initialize Stack
    core->stack.stack = NULL;  // Assuming memory allocation happens elsewhere
    core->stack.capacity = 0;  // Set initial capacity
    core->stack.limit = 0;     // Set stack limit
    core->stack.sp = 0;        // Initialize stack pointer

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

    core->engineRef = engineRef;
}

void core_setup(TypeV_Core *core, uint8_t* program, uint64_t programLength, uint8_t* constantPool, uint64_t constantPoolLength, uint8_t* globalPool, uint64_t globalPoolLength, uint64_t stackCapacity, uint64_t stackLimit){
    core->program.bytecode = program;
    core->program.length = programLength;

    core->constantPool.pool = constantPool;
    core->constantPool.length = constantPoolLength;

    core->globalPool.pool = globalPool;
    core->globalPool.length = globalPoolLength;

    core->stack.stack = malloc(stackCapacity);
    core->stack.capacity = stackCapacity;
    core->stack.limit = stackLimit;
    core->stack.sp = 0;
}

void core_vm(TypeV_Core *core) {
    while(1){
        uint8_t opcode = core->program.bytecode[core->registers.ip++];
        op_funcs[opcode](core);
    }
}

void core_deallocate(TypeV_Core *core) {
    free(core->stack.stack);
    core->stack.stack = NULL;

    queue_deallocate(&(core->messageInputQueue));

    free(core->constantPool.pool);
    core->constantPool.pool = NULL;

    // Note: Program deallocation depends on how programs are loaded and managed
}


