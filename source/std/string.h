/**
 * Type-V Virtual Machine
 * Author: praisethemoon
 * string.h: String Standard library for Type-V
 * String std lib
 */

#ifndef TYPE_V_STRING_H
#define TYPE_V_STRING_H

#include <stdlib.h>
#include "../core.h"


void typev_std_string_alloc(TypeV_Core *core);
void typev_std_string_print(TypeV_Core *core);

TypeV_FFIFunc* typev_std_string_get_lib();

#endif //TYPE_V_STRING_H
