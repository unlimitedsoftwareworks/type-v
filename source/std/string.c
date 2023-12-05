//
// Created by praisethemoon on 05.12.23.
//

#include <stdio.h>
#include "string.h"
#include "../core.h"
#include "../api/typev_api.h"
#include "../vendor/sds/sds.h"

void typev_std_string_from_const(TypeV_Core *core){
    size_t ptr = typev_api_stack_pop_u64(core);
    char* p = (char*)typev_api_get_const_address(core, ptr);

    sds myString = sdsnew(p);

    typev_api_return_u64(core, (size_t)myString);
}

void typev_std_string_print(TypeV_Core *core){
    sds myString = (sds)typev_api_stack_pop_u64(core);
    printf("%s", myString);
}

static TypeV_FFIFunc string_lib[] = {
        &typev_std_string_from_const,
        &typev_std_string_print,
        NULL
};

TypeV_FFIFunc* typev_std_string_get_lib(){
    return string_lib;
}