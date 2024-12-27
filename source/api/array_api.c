//
// Created by praisethemoon on 26.12.24.
//

#include <string.h>
#include "array_api.h"

TypeV_Array* typev_api_array_create(TypeV_Core* core, uint64_t count, uint8_t elementSize, uint8_t ptr) {
    TypeV_Array* array = (TypeV_Array*)core_array_alloc(core, ptr, count, elementSize);
    return array;
}

void typev_api_array_set(TypeV_Core* core, TypeV_Array* array, uint64_t index, void** value) {
    memcpy(array->data + index * array->elementSize, value, array->elementSize);
}

void* typev_api_array_get(TypeV_Core* core, TypeV_Array* array, uint64_t index) {
    return array->data + index * array->elementSize;
}
