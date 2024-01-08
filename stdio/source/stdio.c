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
    //fwrite(arr->data, 1, arr->length, stdout);
    //fwrite(arr->data, 1, arr->length, stdout);
    printf("%.*s", arr->length, arr->data);
}

void stdio_println(TypeV_Core *core) {
    size_t ptr = typev_api_stack_pop_u64(core);
    TypeV_Array* arr = (TypeV_Array*)(ptr);
    //fwrite(arr->data, 1, arr->length, stdout);
    //fwrite("\n", 1, 1, stdout);
    printf("%.*s\n", arr->length, arr->data);
}

void print_stdstring(TypeV_Core *core) {
    TypeV_Array* arr = typev_api_stack_pop_array(core);
    uint64_t length = typev_api_stack_pop_u64(core);

    //fwrite(arr->data, 1, length, stdout);
    printf("%.*s", length, arr->data);
}

void println_stdstring(TypeV_Core *core) {
    TypeV_Array* arr = typev_api_stack_pop_array(core);
    uint64_t length = typev_api_stack_pop_u64(core);

    //printf("%.*s\n", length, arr->data);
    //fwrite(arr->data, 1, length, stdout);
    //fwrite("\n", 1, 1, stdout);
    printf("%.*s\n", length, arr->data);
}


void stdio_print_u64(TypeV_Core *core) {
    uint64_t value = typev_api_stack_pop_u64(core);
    printf("%llu\n", value);
}

static TypeV_FFIFunc stdio_lib[] = {
        (TypeV_FFIFunc)stdio_print,
        (TypeV_FFIFunc)stdio_println,
        (TypeV_FFIFunc)stdio_print_u64,
        (TypeV_FFIFunc)print_stdstring,
        (TypeV_FFIFunc)println_stdstring,
        NULL
};

size_t typev_ffi_open(){
    return typev_api_register_lib(stdio_lib);
}


#endif //TYPE_V_STDIO_C
