cmake_minimum_required(VERSION 3.25)
project(type_v C)

set(CMAKE_C_STANDARD 99)

add_subdirectory(stdio)
add_subdirectory(string)



#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Ofast")
add_executable(type_v source/main.c source/core.c source/core.h source/queue/queue.c source/queue/queue.h source/engine.c source/engine.h source/instructions/instructions.c source/instructions/instructions.h source/stack/stack.c source/stack/stack.h source/platform/platform.h source/utils/utils.c source/utils/utils.h source/assembler/assembler.c source/assembler/assembler.h source/utils/log.h source/vendor/libtable/table.c source/vendor/libtable/table.h source/instructions/opcodes.h source/instructions/opfuncs.h source/api/typev_api.c source/api/typev_api.h source/dynlib/dynlib.c source/dynlib/dynlib.h source/env/env.h source/env/env.c string/source/string.c string/source/string.h
        source/std/std.c
        source/std/std.h
        source/std/string.c
        source/std/string.h
        source/vendor/sds/sds.c
        source/vendor/sds/sds.h
        source/vendor/sds/sdsalloc.h)

add_dependencies(type_v stdio string)
target_link_libraries(type_v stdio string)