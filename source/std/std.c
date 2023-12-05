//
// Created by praisethemoon on 05.12.23.
//

#include "std.h"
#include "string.h"


static TypeV_StdLibs* stdLibs = NULL;

TypeV_StdLibs* typev_std_get_libs() {
    if(stdLibs != NULL) {
        return stdLibs;
    }

    stdLibs = malloc(sizeof(TypeV_StdLibs));

    stdLibs->len = 2;
    stdLibs->libs = malloc(sizeof(TypeV_StdLib) * stdLibs->len);

    stdLibs->libs[TV_STR_LIB].len = 2;
    stdLibs->libs[TV_STR_LIB].fns = typev_std_string_get_lib();

    stdLibs->libs[TV_IO_LIB].len = 0;
    stdLibs->libs[TV_IO_LIB].fns = NULL;

    return stdLibs;
}