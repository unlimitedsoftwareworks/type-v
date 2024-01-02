project(stdcore VERSION 1.0)


set(stdcore_files source/stdcore.c)

set(typev_files
        ../source/core.c
        ../source/stack/stack.c
        ../source/dynlib/dynlib.c
        ../source/env/env.c
        ../source/queue/queue.c
        ../source/api/typev_api.c
        ../source/utils/utils.c
)


add_library(stdcore SHARED ${stdcore_files} ${typev_files})

set_target_properties(stdcore PROPERTIES
        C_STANDARD 99
        C_STANDARD_REQUIRED YES
        C_EXTENSIONS NO
)
