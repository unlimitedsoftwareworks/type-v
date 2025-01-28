//
// Created by praisethemoon on 26.12.24.
//

#include "struct_api.h"
#include "../engine.h"

TypeV_Struct* typev_api_struct_alloc(TypeV_Core* core, uint16_t fieldCount, size_t structSize) {
    return (TypeV_Struct*)core_struct_alloc(core, fieldCount, structSize);
}

void typev_api_struct_reg_field(TypeV_Struct* str, uint16_t fieldIndex, size_t offset) {
    str->fieldOffsets[fieldIndex] = offset;
}

void typev_api_struct_set_field(TypeV_Struct* str, uint16_t fieldIndex, size_t value) {
    memcpy(str->data + str->fieldOffsets[fieldIndex], &value, sizeof(size_t));
}

void typev_api_struct_set_field_ptr(TypeV_Struct* str, uint16_t field_index, uintptr_t value) {
    memcpy(str->data + str->fieldOffsets[field_index], &value, 8);
    // mark field as pointer

    size_t byteIndex = field_index / 8;      // Determine which byte contains the bit
    uint8_t bitOffset = field_index % 8;     // Determine the bit position within the byte
    str->pointerBitmask[byteIndex] |= (1 << bitOffset);
}

uintptr_t typev_api_struct_get_field(TypeV_Struct* str, uint16_t fieldIndex, size_t size) {
    uintptr_t value;
    memcpy(&value, str->data + str->fieldOffsets[fieldIndex], size);
    return value;
}

uintptr_t typev_api_struct_get_field_ptr(TypeV_Struct* str, uint16_t fieldIndex) {
    uintptr_t value;
    memcpy(&value, str->data + str->fieldOffsets[fieldIndex], 8);
    return value;
}

uint16_t typev_api_struct_get_field_id(TypeV_Core core, char* str, char* fieldName, uint8_t* error) {
    uint8_t error_flag = 0;
    uint16_t fieldId = 0;

    engine_get_field_id(core.engineRef, fieldName, &fieldId, &error_flag);
    *error = error_flag;

    return fieldId;
}


uintptr_t typev_api_struct_get_value(TypeV_Core* core, TypeV_Struct* str, const char* name, uint16_t fieldIndex, size_t size) {
    uint8_t error = 0;
    uint32_t gfieldId = 0;
    engine_get_field_id(core->engineRef, name, &gfieldId, &error);
    uint8_t errorFlag = 0;
    uint8_t idx = object_find_global_index(core, str->globalFields, str->numFields, gfieldId, &errorFlag);

    if(error || errorFlag) {
        return 0;
    }

    uintptr_t value = 0;
    memcpy(&value, str->data + str->fieldOffsets[idx], size);
    return value;
}