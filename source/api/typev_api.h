/**
 * Type-V Virtual Machine
 * Author: praisethemoon
 * typev_api.h: TypeV API for FFI
 *
 * Collection for functions to be used by FFI.
 *
 */

#ifndef TYPE_V_TYPEV_API_H
#define TYPE_V_TYPEV_API_H

#include <stdlib.h>
#include "../core.h"

size_t typev_api_register_lib(struct TypeV_Core* core, const TypeV_FFIFunc lib[]);

size_t typev_api_get_const_address(struct TypeV_Core* core, size_t vm_adr);

uint64_t typev_api_stack_getSize(struct TypeV_Core* core);
int8_t typev_api_stack_pop_i8(struct TypeV_Core* core);
uint8_t typev_api_stack_pop_u8(struct TypeV_Core* core);
int16_t typev_api_stack_pop_i16(struct TypeV_Core* core);
uint16_t typev_api_stack_pop_u16(struct TypeV_Core* core);
int32_t typev_api_stack_pop_i32(struct TypeV_Core* core);
uint32_t typev_api_stack_pop_u32(struct TypeV_Core* core);
int64_t typev_api_stack_pop_i64(struct TypeV_Core* core);
uint64_t typev_api_stack_pop_u64(struct TypeV_Core* core);
float typev_api_stack_pop_f32(struct TypeV_Core* core);
double typev_api_stack_pop_f64(struct TypeV_Core* core);
TypeV_Struct* typev_api_stack_pop_struct(struct TypeV_Core* core);
TypeV_Class* typev_api_stack_pop_class(struct TypeV_Core* core);
TypeV_Interface* typev_api_stack_pop_interface(struct TypeV_Core* core);
TypeV_Array* typev_api_stack_pop_array(struct TypeV_Core* core);

void typev_api_return_i8(struct TypeV_Core* core, int8_t value);
void typev_api_return_u8(struct TypeV_Core* core, uint8_t value);
void typev_api_return_i16(struct TypeV_Core* core, int16_t value);
void typev_api_return_u16(struct TypeV_Core* core, uint16_t value);
void typev_api_return_i32(struct TypeV_Core* core, int32_t value);
void typev_api_return_u32(struct TypeV_Core* core, uint32_t value);
void typev_api_return_i64(struct TypeV_Core* core, int64_t value);
void typev_api_return_u64(struct TypeV_Core* core, uint64_t value);
void typev_api_return_f32(struct TypeV_Core* core, float value);
void typev_api_return_f64(struct TypeV_Core* core, double value);
void typev_api_return_struct(struct TypeV_Core* core, TypeV_Struct* value);
void typev_api_return_class(struct TypeV_Core* core, TypeV_Class* value);
void typev_api_return_interface(struct TypeV_Core* core, TypeV_Interface* value);
void typev_api_return_array(struct TypeV_Core* core, TypeV_Array* value);

/**
 * Creates a new struct given the number of fields and the total struct size
 * @param core
 * @param fieldCount
 * @param structSize
 * @return
 */
TypeV_Struct *typev_api_struct_create(struct TypeV_Core *core, uint16_t fieldCount, size_t structSize);

/**
 * Sets the offset of a field in a struct
 * @param core
 * @param structPtr
 * @param fieldIndex
 * @param offset
 */
void typev_api_struct_set_offset(struct TypeV_Core *core, TypeV_Struct *structPtr, uint16_t fieldIndex, uint16_t offset);

/**
 * Sets the value of a field in a struct
 * @param core
 * @param structPtr
 * @param fieldIndex
 * @param value
 * @param valueSize
 */
void typev_api_struct_set_field(struct TypeV_Core *core, TypeV_Struct *structPtr, uint16_t fieldIndex, void *value, size_t valueSize);



#endif //TYPE_V_TYPEV_API_H
