
#include <string.h>
#include <assert.h>
#include "stack.h"
#include "../utils/log.h"

void stack_init(TypeV_FuncState* fnc, size_t capacity) {
    fnc->stack = malloc(capacity);
    fnc->sp = 0;
    fnc->capacity = capacity;
}

void stack_push_8(TypeV_FuncState* fnc, uint8_t value) {
    // check if we have space
    if (fnc->sp + 1 >= fnc->capacity) {
        //LOG_ERROR("Stack overflow, failed to allocated 1byte, stack-size: %zu", fnc->capacity);
        //assert(0);
    }

    fnc->stack[fnc->sp] = value;
    fnc->sp++;
}

void stack_push_16(TypeV_FuncState* fnc, uint16_t value) {
    // check if we have space
    if (fnc->sp + 2 >= fnc->capacity) {
        LOG_ERROR("Stack overflow, failed to allocated 1byte, stack-size: %zu", fnc->capacity);
        assert(0);
    }

    memcpy(fnc->stack + fnc->sp, &value, 2);
    fnc->sp += 2;
}

void stack_push_32(TypeV_FuncState* fnc, uint32_t value) {
    // check if we have space
    if (fnc->sp + 4 >= fnc->capacity) {
        LOG_ERROR("Stack overflow, failed to allocated 1byte, stack-size: %zu", fnc->capacity);
        assert(0);
    }

    memcpy(fnc->stack + fnc->sp, &value, 4);
    fnc->sp += 4;
}

void stack_push_64(TypeV_FuncState* fnc, uint64_t value) {
    // check if we have space
    if (fnc->sp + 8 >= fnc->capacity) {
        LOG_ERROR("Stack overflow, failed to allocated 1byte, stack-size: %zu", fnc->capacity);
        assert(0);
    }

    memcpy(fnc->stack + fnc->sp, &value, 8);
    fnc->sp += 8;
}

void stack_push_ptr(TypeV_FuncState* fnc, size_t value) {
    // check if we have space
    if (fnc->sp + PTR_SIZE >= fnc->capacity) {
        LOG_ERROR("Stack overflow, failed to allocated 1byte, stack-size: %zu", fnc->capacity);
        assert(0);
    }

    memcpy(fnc->stack + fnc->sp, &value, sizeof(size_t));
    fnc->sp += sizeof(size_t);
}

void stack_pop_8(TypeV_FuncState* fnc, uint8_t * value) {
    // check we don't underflow
    if (fnc->sp == 0) {
        LOG_ERROR("Stack underflow, failed to allocated 1byte, stack-size: %zu", fnc->capacity);
        assert(0);
    }

    *value = fnc->stack[fnc->sp - 1];
    fnc->sp--;
}

void stack_pop_16(TypeV_FuncState* fnc, uint16_t * value) {
    // check we don't underflow
    if (fnc->sp < 2) {
        LOG_ERROR("Stack underflow, failed to allocated 1byte, stack-size: %zu", fnc->capacity);
        assert(0);
    }

    memcpy(value, fnc->stack + fnc->sp - 2, 2);
    fnc->sp -= 2;
}

void stack_pop_32(TypeV_FuncState* fnc, uint32_t * value) {
    // check we don't underflow
    if (fnc->sp < 4) {
        LOG_ERROR("Stack underflow, failed to allocated 1byte, stack-size: %zu", fnc->capacity);
        assert(0);
    }

    memcpy(value, fnc->stack + fnc->sp - 4, 4);
    fnc->sp -= 4;
}

void stack_pop_64(TypeV_FuncState* fnc, uint64_t * value) {
    // check we don't underflow
    if (fnc->sp < 8) {
        LOG_ERROR("Stack underflow, failed to allocated 1byte, stack-size: %zu", fnc->capacity);
        assert(0);
    }

    memcpy(value, fnc->stack + fnc->sp - 8, 8);
    fnc->sp -= 8;
}

void stack_pop_ptr(TypeV_FuncState* fnc, size_t * value) {
    // check we don't underflow
    if (fnc->sp < PTR_SIZE) {
        LOG_ERROR("Stack underflow, failed to allocated 1byte, stack-size: %zu", fnc->capacity);
        assert(0);
    }

    memcpy(value, fnc->stack + fnc->sp - sizeof(size_t), sizeof(size_t));
    fnc->sp -= sizeof(size_t);
}

void stack_free(TypeV_FuncState* fnc) {
    free(fnc->stack);
    fnc->stack = NULL;
}