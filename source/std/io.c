//
// Created by praisethemoon on 05.12.23.
//

#include "../vendor/sds/sds.h"
#include "io.h"

void typev_std_io_print(TypeV_Core *core) {
    sds str = (sds) typev_api_stack_pop_ptr(core);
    printf("%s", str);
}

void typev_std_io_fopen(TypeV_Core *core) {
    sds mode = (sds) typev_api_stack_pop_ptr(core);
    sds filename = (sds) typev_api_stack_pop_ptr(core);
    FILE* file = fopen(filename, mode);
    typev_api_return_ptr(core, (size_t)file);
}

void typev_std_io_fclose(TypeV_Core *core) {
    FILE* file = (FILE*) typev_api_stack_pop_ptr(core);
    fclose(file);
}

static TypeV_FFIFunc io_lib[] = {
        typev_std_io_print,
        typev_std_io_fopen,
        typev_std_io_fclose,
        NULL
};


TypeV_FFIFunc* typev_std_io_get_lib(){
    return io_lib;
}