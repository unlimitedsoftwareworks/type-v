#ifndef TV_STRING_H
#define TV_STRING_H

#include <stddef.h>


typedef struct {
    char *data;   // Pointer to character data
    size_t len;   // Length of the string
    size_t cap;   // Capacity of the buffer
} TV_String;






#endif // TV_STRING_H
