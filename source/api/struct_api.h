//
// Created by praisethemoon on 26.12.24.
//

#ifndef TYPEV_STRUCT_API_H
#define TYPEV_STRUCT_API_H

#include "./typev_api.h"
#include "../gc/gc.h"
#include "../core.h"

DYNLIB_EXPORT TypeV_Struct* typev_api_struct_alloc(TypeV_Core* core, uint16_t fieldCount, size_t structSize);

DYNLIB_EXPORT void typev_api_struct_reg_field(TypeV_Struct* str, uint16_t fieldIndex, size_t offset);

DYNLIB_EXPORT void typev_api_struct_set_field(TypeV_Struct* str, uint16_t fieldIndex, size_t value);
DYNLIB_EXPORT void typev_api_struct_set_field_ptr(TypeV_Struct* str, uint16_t field_index, uintptr_t value);

DYNLIB_EXPORT uintptr_t typev_api_struct_get_field(TypeV_Struct* str, uint16_t fieldIndex, size_t size);
DYNLIB_EXPORT uintptr_t typev_api_struct_get_field_ptr(TypeV_Struct* str, uint16_t fieldIndex);

DYNLIB_EXPORT uint16_t typev_api_struct_get_field_id(TypeV_Core core, char* str, char* fieldName, uint8_t* error);

DYNLIB_EXPORT uintptr_t typev_api_struct_get_value(TypeV_Core* core, TypeV_Struct* str, const char* name, uint16_t fieldIndex, size_t size);


#endif //TYPEV_STRUCT_API_H
