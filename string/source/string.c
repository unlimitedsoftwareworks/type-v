
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../../source/core.h"
#include "../../source/dynlib/dynlib.h"
#include "../../source/api/typev_api.h"
#include "string.h"


DYNLIB_EXPORT void tvstring_init(TypeV_Core *core) {
    TV_String* str = (TV_String*)core_mem_alloc(core, sizeof(TV_String));
    str->len = 0;
    str->cap = 0;
    str->data = NULL;

    typev_api_return_u64(core, (size_t)str);
}

DYNLIB_EXPORT void tvstring_init_empty(TypeV_Core *core) {
    TV_String* str = (TV_String*)core_mem_alloc(core, sizeof(TV_String));
    str->data = NULL;
    str->len = 0;
    str->cap = 0;

    typev_api_return_u64(core, (size_t)str);
}

DYNLIB_EXPORT void tvstring_init_from_cstr(TypeV_Core *core) {
    size_t ptr = (size_t)typev_api_stack_pop_u64(core);
    char* cstr = (char*)typev_api_get_const_address(core, ptr);

    TV_String* str = (TV_String*)core_mem_alloc(core, sizeof(TV_String));
    str->len = strlen(cstr);
    str->cap = str->len;
    str->data = (char*)core_mem_alloc(core, str->cap);
    memcpy(str->data, cstr, str->len);

    typev_api_return_u64(core, (size_t)str);
}

DYNLIB_EXPORT void tv_string_concat(TypeV_Core *core) {
    TV_String* str1 = (TV_String*)typev_api_stack_pop_u64(core);
    TV_String* str2 = (TV_String*)typev_api_stack_pop_u64(core);
    TV_String* str3 = (TV_String*)core_mem_alloc(core, sizeof(TV_String));
    str3->len = str1->len + str2->len;
    str3->cap = str3->len;
    str3->data = (char*)core_mem_alloc(core, str3->cap);
    memcpy(str3->data, str1->data, str1->len);
    memcpy(str3->data+str1->len, str2->data, str2->len);

    typev_api_return_u64(core, (size_t)str3);
}

DYNLIB_EXPORT void tv_string_print(TypeV_Core *core) {
    TV_String* str = (TV_String*)typev_api_stack_pop_u64(core);
    printf("%s", str->data);
}

static TypeV_FFIFunc string_lib[] = {
        tvstring_init,
        tvstring_init_empty,
        tvstring_init_from_cstr,
        tv_string_concat,
        tv_string_print,
        NULL
};

size_t typev_ffi_open(TypeV_Core* core){
    return typev_api_register_lib(core, string_lib);
}