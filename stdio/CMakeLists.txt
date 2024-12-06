project(stdio VERSION 1.0)


set(stdio_files source/stdio.c)

set(typev_files
        ../source/core.c
        ../source/gc/gc.c
        ../source/gc/mark.c
        ../source/stack/stack.c
        ../source/dynlib/dynlib.c
        ../source/env/env.c
        ../source/api/typev_api.c
        ../source/utils/utils.c
        ../source/errors/errors.c
)


add_library(stdio SHARED ${stdio_files} ${typev_files})

set_target_properties(stdio PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED YES
    C_EXTENSIONS NO
)
