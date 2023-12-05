/**
 * Type-V Virtual Machine
 * Author: praisethemoon
 * std.h: Standard library for Type-V
 * Standard library are functions loaded using ld_std instruction
 */
#ifndef TYPE_V_STD_H
#define TYPE_V_STD_H

#include "../api/typev_api.h"

#define TV_STR_LIB 0
#define TV_IO_LIB 1


typedef struct {
    uint8_t len;
    TypeV_FFIFunc* fns;
} TypeV_StdLib;

typedef struct {
    uint8_t len;
    TypeV_StdLib* libs;
} TypeV_StdLibs;

TypeV_StdLibs* typev_std_get_libs();

#endif //TYPE_V_STD_H
