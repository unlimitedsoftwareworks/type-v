
#include "./typev_api.h"
#include "../core.h"

/**
 * @brief Creates an array
 * @param core Core
 * @param count Number of elements
 * @param elementSize Size of each element
 * @param ptr 0 if elements are not pointers, anything else if they are
 * @return Pointer to the array
 */
DYNLIB_EXPORT TypeV_Array* typev_api_array_create(TypeV_Core* core, uint64_t count, uint8_t elementSize, uint8_t ptr);

/**
 * @brief Creates an array from a buffer, does not copy the buffer instead uses it as the data
 * @param core Core
 * @param count Number of elements
 * @param elementSize Size of each element
 * @param ptr 0 if elements are not pointers, anything else if they are
 * @param buffer Pointer to the buffer
 * @return Pointer to the array
 */
DYNLIB_EXPORT TypeV_Array* typev_api_array_create_from_buffer(TypeV_Core* core, uint64_t count, uint8_t elementSize, uint8_t ptr, void* buffer);

/**
 * @brief Sets the value of an element in the array
 * @param core Core
 * @param array Pointer to the array
 * @param index Index of the element
 * @param value Pointer to the value
 */
DYNLIB_EXPORT void typev_api_array_set(TypeV_Core* core, TypeV_Array* array, uint64_t index, void** value);

/**
 * @brief Gets the value of an element in the array
 * @param core Core
 * @param array Pointer to the array
 * @param index Index of the element
 * @return Pointer to the value
 */
DYNLIB_EXPORT void* typev_api_array_get(TypeV_Core* core, TypeV_Array* array, uint64_t index);
