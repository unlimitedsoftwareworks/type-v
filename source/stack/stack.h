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

void stack_init(TypeV_Core *core, size_t capacity);
void stack_push_8(TypeV_Core *core, uint8_t value);
void stack_push_16(TypeV_Core *core, uint16_t value);
void stack_push_32(TypeV_Core *core, uint32_t value);
void stack_push_64(TypeV_Core *core, uint64_t value);
void stack_push_ptr(TypeV_Core *core, size_t value);

void stack_pop_8(TypeV_Core *core, uint8_t *value);
void stack_pop_16(TypeV_Core *core, uint16_t *value);
void stack_pop_32(TypeV_Core *core, uint32_t *value);
void stack_pop_64(TypeV_Core *core, uint64_t *value);
void stack_pop_ptr(TypeV_Core *core, size_t *value);

/**
 * @brief Allocates a new stack frame for function arguments
 * @param core
 * @param size
 */
void stack_frame_alloc_args(TypeV_Core *core, size_t size);

/**
 * @brief Allocates a new stack frame for local variables
 * @param core
 * @param size
 */
void stack_frame_alloc_locals(TypeV_Core *core, size_t size);

/**
 * @brief Removes the current stack frame
 * @param core
 */
void stack_frame_remove(TypeV_Core *core);

/**
 * @brief Pushes a new stack frame prior to a function call
 * @param core
 */
void stack_frame_precall_push(TypeV_Core *core);

/**
 * @brief Pops the stack frame after a function call
 * @param core
 */
void stack_frame_postcall_pop(TypeV_Core *core);

void stack_free(TypeV_Core *core);
#endif //TYPE_V_STACK_H
