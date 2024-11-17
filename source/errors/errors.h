#ifndef TYPE_V_ERRORS_H
#define TYPE_V_ERRORS_H

typedef enum TypeV_RTError {
    RT_ERROR_UNREACHABLE = 0,
    RT_ERROR_COROUTINE_FINISHED = 1,
    RT_ERROR_OOM = 2,
    RT_ERROR_OVERFLOW = 3,
    RT_ERROR_DIVISION_BY_ZERO = 4,
    RT_ERROR_NULL_POINTER = 5,
    RT_ERROR_ATTRIBUTE_NOT_FOUND = 6,
    RT_ERROR_NURSERY_FULL = 7,
    RT_ERROR_OUT_OF_BOUNDS = 8,
    RT_ERROR_INVALID_COMPARISON_OPERATOR = 9,
    RT_ERROR_ENTITY_TOO_LARGE = 10,
    RT_ERROR_CUSTOM = 11,

    RT_ERROR_COUNT //Tracks the number of errors
} TypeV_RTError;

static const char* TypeV_RTErrorMessages[] = {
    "Unreachable code reached",
    "Coroutine has already finished",
    "Out of memory",
    "Overflow occurred",
    "Division by zero",
    "Null pointer dereference",
    "Attribute not found",
    "Nursery is full",
    "Out of bounds access",
    "Invalid comparison operator",
    "Entity too large",
    "Custom error"
};

#endif // TYPE_V_ERRORS_H
