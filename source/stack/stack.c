
#include <string.h>
#include "stack.h"
#include "../utils/log.h"

void stack_init(TypeV_Core * core, size_t capacity) {
    core->stack.stack = malloc(capacity);
    core->stack.capacity = capacity;
    core->registers.sp = 0;
    core->registers.fp = 0;
    core->registers.fe = 0;
}

void stack_push_8(TypeV_Core * core, uint8_t value) {
    // check if we have space
    if (core->registers.sp + 1 >= core->stack.capacity) {
        LOG_ERROR("Stack overflow, failed to allocated 1byte, stack-size: %zu", core->stack.capacity);
        exit(-1);
    }

    core->stack.stack[core->registers.sp] = value;
    core->registers.sp++;
}

void stack_push_16(TypeV_Core * core, uint16_t value) {
    // check if we have space
    if (core->registers.sp + 2 >= core->stack.capacity) {
        LOG_ERROR("Stack overflow, failed to allocated 1byte, stack-size: %zu", core->stack.capacity);
        exit(-1);
    }

    memcpy(core->stack.stack + core->registers.sp, &value, 2);
    core->registers.sp += 2;
}

void stack_push_32(TypeV_Core * core, uint32_t value) {
    // check if we have space
    if (core->registers.sp + 4 >= core->stack.capacity) {
        LOG_ERROR("Stack overflow, failed to allocated 1byte, stack-size: %zu", core->stack.capacity);
        exit(-1);
    }

    memcpy(core->stack.stack + core->registers.sp, &value, 4);
    core->registers.sp += 4;
}

void stack_push_64(TypeV_Core * core, uint64_t value) {
    // check if we have space
    if (core->registers.sp + 8 >= core->stack.capacity) {
        LOG_ERROR("Stack overflow, failed to allocated 1byte, stack-size: %zu", core->stack.capacity);
        exit(-1);
    }

    memcpy(core->stack.stack + core->registers.sp, &value, 8);
    core->registers.sp += 8;
}

void stack_push_ptr(TypeV_Core * core, size_t value) {
    // check if we have space
    if (core->registers.sp + PTR_SIZE >= core->stack.capacity) {
        LOG_ERROR("Stack overflow, failed to allocated 1byte, stack-size: %zu", core->stack.capacity);
        exit(-1);
    }

    memcpy(core->stack.stack + core->registers.sp, &value, sizeof(size_t));
    core->registers.sp += sizeof(size_t);
}

void stack_pop_8(TypeV_Core * core, uint8_t * value) {
    // check we don't underflow
    if (core->registers.sp == 0) {
        LOG_ERROR("Stack underflow, failed to allocated 1byte, stack-size: %zu", core->stack.capacity);
        exit(-1);
    }

    *value = core->stack.stack[core->registers.sp - 1];
    core->registers.sp--;
}

void stack_pop_16(TypeV_Core * core, uint16_t * value) {
    // check we don't underflow
    if (core->registers.sp < 2) {
        LOG_ERROR("Stack underflow, failed to allocated 1byte, stack-size: %zu", core->stack.capacity);
        exit(-1);
    }

    memcpy(value, core->stack.stack + core->registers.sp - 2, 2);
    core->registers.sp -= 2;
}

void stack_pop_32(TypeV_Core * core, uint32_t * value) {
    // check we don't underflow
    if (core->registers.sp < 4) {
        LOG_ERROR("Stack underflow, failed to allocated 1byte, stack-size: %zu", core->stack.capacity);
        exit(-1);
    }

    memcpy(value, core->stack.stack + core->registers.sp - 4, 4);
    core->registers.sp -= 4;
}

void stack_pop_64(TypeV_Core * core, uint64_t * value) {
    // check we don't underflow
    if (core->registers.sp < 8) {
        LOG_ERROR("Stack underflow, failed to allocated 1byte, stack-size: %zu", core->stack.capacity);
        exit(-1);
    }

    memcpy(value, core->stack.stack + core->registers.sp - 8, 8);
    core->registers.sp -= 8;
}

void stack_pop_ptr(TypeV_Core * core, size_t * value) {
    // check we don't underflow
    if (core->registers.sp < PTR_SIZE) {
        LOG_ERROR("Stack underflow, failed to allocated 1byte, stack-size: %zu", core->stack.capacity);
        exit(-1);
    }

    memcpy(value, core->stack.stack + core->registers.sp - sizeof(size_t), sizeof(size_t));
    core->registers.sp -= sizeof(size_t);
}

void stack_frame_alloc_args(TypeV_Core *core, size_t size) {
    LOG_INFO("Allocating stack frame args for %zu bytes", size);
    // point fp to current sp
    core->registers.fp = core->registers.sp;
    // allocate space for args
    //core->registers.sp += size;
    // point fe to end of args
    core->registers.fe = core->registers.fp + size;
}


void stack_frame_alloc_locals(TypeV_Core *core, size_t size){
    LOG_INFO("Allocating stack frame locals for %zu bytes", size);
    // allocate space for locals
    //core->registers.sp += size;
    // point fe to end of locals
    core->registers.fe = core->registers.fp + size;
}


void stack_frame_remove(TypeV_Core *core) {
    // removes the stack frame, everything on top remains
    core->registers.sp = core->registers.fp;
}

void stack_frame_precall_push(TypeV_Core *core) {
    for(int i = 0; i < MAX_REG-1; i++){
        // push registers to stack
        stack_push_64(core, core->registers.regs[i].u64);
    }

    // push fp to stack
    stack_push_64(core, core->registers.fp);
    // push fe to stack
    stack_push_64(core, core->registers.fe);
    // push flags to stack
    stack_push_64(core, core->registers.flags);
    /**
     * IMPORTANT! PUSH IP LAST
     * so we can override it in fn_call
     */
    // push ip to stack
    stack_push_64(core, core->registers.ip);
}

void stack_frame_postcall_pop(TypeV_Core *core) {
    // pop ip from stack
    stack_pop_64(core, &core->registers.ip);
    // pop flags from stack
    stack_pop_64(core, &core->registers.flags);
    // pop fe from stack
    stack_pop_64(core, &core->registers.fe);
    // pop fp from stack
    stack_pop_64(core, &core->registers.fp);
    /**
     * MAX_REG -2 because R19 is reserved for function return value
     */
    for (int i = MAX_REG - 2; i >= 0; i--) {
        // pop registers from stack
        stack_pop_64(core, &core->registers.regs[i].u64);
    }
}

void stack_free(TypeV_Core * core) {
    free(core->stack.stack);
    core->stack.stack = NULL;
}