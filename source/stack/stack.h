/**
 * Type-V Virtual Machine
 * Author: praisethemoon
 * stack.h: Core Stack
 * Core stack is used to store local variables and function arguments.
 */

#ifndef TYPE_V_STACK_H
#define TYPE_V_STACK_H

#include <stdlib.h>
#include <stdint.h>

#include "../core.h"

void stack_init(TypeV_FuncState* fnc, size_t capacity);
void stack_push_8(TypeV_FuncState* fnc, uint8_t value);
void stack_push_16(TypeV_FuncState* fnc, uint16_t value);
void stack_push_32(TypeV_FuncState* fnc, uint32_t value);
void stack_push_64(TypeV_FuncState* fnc, uint64_t value);
void stack_push_ptr(TypeV_FuncState* fnc, size_t value);

void stack_pop_8(TypeV_FuncState* fnc, uint8_t *value);
void stack_pop_16(TypeV_FuncState* fnc, uint16_t *value);
void stack_pop_32(TypeV_FuncState* fnc, uint32_t *value);
void stack_pop_64(TypeV_FuncState* fnc, uint64_t *value);
void stack_pop_ptr(TypeV_FuncState* fnc, uintptr_t *value);

void stack_free(TypeV_FuncState* fnc);
#endif //TYPE_V_STACK_H
