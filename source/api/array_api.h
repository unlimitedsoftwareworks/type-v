
#include "./typev_api.h"
#include "../core.h"

DYNLIB_EXPORT TypeV_Array* typev_api_array_create(TypeV_Core* core, uint64_t count, uint8_t elementSize, uint8_t ptr);
DYNLIB_EXPORT void typev_api_array_set(TypeV_Core* core, TypeV_Array* array, uint64_t index, void** value);
DYNLIB_EXPORT void* typev_api_array_get(TypeV_Core* core, TypeV_Array* array, uint64_t index);
