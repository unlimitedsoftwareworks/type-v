/**
 * Type-V Virtual Machine
 * Author: praisethemoon
 * string.h: String Standard library for Type-V
 * String std lib
 */

#ifndef TYPE_V_STRING_H
#define TYPE_V_STRING_H

#include <stdlib.h>
#include "./std.h"
#include "../core.h"

#define tv_std(lib) void typev_std_string_##lib(TypeV_Core *core)

tv_std(fromConst);
tv_std(empty);
tv_std(clear);
tv_std(clone);
tv_std(len);
tv_std(makeRoomFor);
tv_std(freeSpace);
tv_std(allocSize);
tv_std(growZero);
tv_std(catlen);
tv_std(catconst);
tv_std(catstr);
tv_std(fromI8);
tv_std(fromU8);
tv_std(fromI16);
tv_std(fromU16);
tv_std(fromI32);
tv_std(fromU32);
tv_std(fromI64);
tv_std(fromU64);
tv_std(fromF32);
tv_std(fromF64);
tv_std(fromPtr);
tv_std(toI8);
tv_std(toU8);
tv_std(toI16);
tv_std(toU16);
tv_std(toI32);
tv_std(toU32);
tv_std(toI64);
tv_std(toU64);
tv_std(toF32);
tv_std(toF64);
tv_std(toUpper);
tv_std(toLower);
tv_std(cmp);
//tv_std(replaceOne);
//tv_std(replaceAll);
tv_std(charAt);

#define STD_STR_MAX_LEN 37

#undef tv_std

TypeV_FFIFunc* typev_std_string_get_lib();

#endif //TYPE_V_STRING_H
