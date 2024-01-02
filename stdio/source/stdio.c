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
    TypeV_Array* arr = (TypeV_Array*)(ptr);
    fwrite(arr->data, 1, arr->length, stdout);
}

void stdio_println(TypeV_Core *core) {
    size_t ptr = typev_api_stack_pop_u64(core);
    TypeV_Array* arr = (TypeV_Array*)(ptr);
    fwrite(arr->data, 1, arr->length, stdout);
    fwrite("\n", 1, 1, stdout);
}

void stdio_print_u64(TypeV_Core *core) {
    uint64_t value = typev_api_stack_pop_u64(core);
    printf("%llu\n", value);
}

static TypeV_FFIFunc stdio_lib[] = {
        (TypeV_FFIFunc)stdio_print,
        (TypeV_FFIFunc)stdio_println,
        (TypeV_FFIFunc)stdio_print_u64,
        NULL
};

size_t typev_ffi_open(){
    return typev_api_register_lib(stdio_lib);
}


#endif //TYPE_V_STDIO_C
