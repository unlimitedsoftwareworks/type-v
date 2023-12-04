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
    char* base = core->constantPool.pool;
    char* adr = (base + ptr);
    printf("%s", adr);
}

static TypeV_FFIFunc stdio_lib[] = {
        (TypeV_FFIFunc)stdio_print, NULL
};

size_t typev_ffi_open(TypeV_Core* core){
    return typev_api_register_lib(core, stdio_lib);
}


#endif //TYPE_V_STDIO_C
