project(stdcore VERSION 1.0)


set(stdcore_files source/stdcore.c
        source/datetime.c
        source/vendor/yy.c
        source/vendor/yy.h
        source/vendor/atoi_yy.c
        source/vendor/utils_yy.h
        source/vendor/itoa_yy.c
        source/vendor/dtoa_yy.c)

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


add_library(stdcore SHARED ${stdcore_files} ${typev_files}
        source/datetime.c
        source/datetime.h)

set_target_properties(stdcore PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED YES
        C_EXTENSIONS NO
)
