//
// Created by praisethemoon on 24.11.23.
//

#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include "utils.h"


void typev_assert(int cond, const char * rawcond, const char* func_name, const char * fmt, ...) {
    if(cond)
        return;
    char temp[1024];
    va_list vl;
    va_start(vl, fmt);
    vsprintf(temp, fmt, vl);
    va_end(vl);
    fprintf(stdout, "Fatal error, assertion failed: `%s` in function `%s` \n", rawcond, func_name);
    fprintf(stdout, "%s", temp);
    fprintf(stdout, "\n");
    assert(cond);
    ASSERT(1==0, "You should not be here");
}
