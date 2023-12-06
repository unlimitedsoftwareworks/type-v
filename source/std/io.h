/**
 * Type-V Virtual Machine
 * Author: praisethemoon
 * io.h: IO Standard library for Type-V
 * IO std lib
 */

#ifndef TYPE_V_IO_H
#define TYPE_V_IO_H

#include <stdio.h>
#include <stdlib.h>
#include "./std.h"
#include "../core.h"

#define tv_std(lib) void typev_std_io_##lib(TypeV_Core *core)

tv_std(print);

#undef tv_std


TypeV_FFIFunc* typev_std_io_get_lib();

#define STD_IO_MAX_LEN 3

#endif //TYPE_V_IO_H
