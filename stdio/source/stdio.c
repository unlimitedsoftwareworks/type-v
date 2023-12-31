//
// Created by praisethemoon on 03.12.23.
//

#ifndef TYPE_V_STDIO_C
#define TYPE_V_STDIO_C

#include <stdio.h>

#include "../../source/core.h"
#include "../../source/api/typev_api.h"

void stdio_print(TypeV_Core *core) {
    size_t ptr = typev_api_stack_pop_u64(core);
    char* p = (char*)typev_api_get_const_address(core, ptr);
    printf("%s", p);
}

static TypeV_FFIFunc stdio_lib[] = {
        (TypeV_FFIFunc)stdio_print, NULL
};

size_t typev_ffi_open(){
    return typev_api_register_lib(stdio_lib);
}


#endif //TYPE_V_STDIO_C
