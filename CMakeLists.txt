cmake_minimum_required(VERSION 3.25)
project(type_v C)

set(CMAKE_C_STANDARD 11)

add_subdirectory(stdio)
add_subdirectory(string)
add_subdirectory(stdcore)


#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")

# disable inlining for easier debugging
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-inline")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-inline")


# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=undefined")
##set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=leak")
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=pointer-compare")
##set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=pointer-subtract")
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=pointer-overflow")
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=bounds")

add_executable(type_v
        source/main.c
        source/core.c
        source/core.h
        source/gc.c
        source/gc.h
        source/engine.c
        source/engine.h
        source/instructions/instructions.c
        source/instructions/instructions.h
        source/stack/stack.c
        source/stack/stack.h
        source/platform/platform.h
        source/utils/utils.c
        source/utils/utils.h
        source/assembler/assembler.c
        source/assembler/assembler.h
        source/utils/log.h
        source/vendor/libtable/table.c
        source/vendor/libtable/table.h
        source/instructions/opcodes.h
        source/instructions/opfuncs.h
        source/api/typev_api.c
        source/api/typev_api.h
        source/dynlib/dynlib.c
        source/dynlib/dynlib.h
        source/env/env.h
        source/env/env.c
        source/vendor/cpu_info/cpu_info.h
)

add_dependencies(type_v stdio string stdcore)
target_link_libraries(type_v stdio string stdcore)
