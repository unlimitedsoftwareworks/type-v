cmake_minimum_required(VERSION 3.25)
project(type_v C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -funroll-loops -finline-functions")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -march=native")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -funroll-loops -finline-functions")


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-inline")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-inline")

if(UNIX AND NOT APPLE)
    add_compile_options(-D_POSIX_C_SOURCE=200809L -D_ISOC11_SOURCE -D_GNU_SOURCE)
endif()

# Enforce the C11 standard explicitly for Windows (MSVC)
if(MSVC)
    add_compile_options(/std:c11 /Zc:__cplusplus)
endif()




# other subdirectories
add_subdirectory(stdio)
add_subdirectory(stdfs)
add_subdirectory(string)
add_subdirectory(stdcore)
add_subdirectory(stdmath)


# Your executable target
add_executable(type_v
        source/main.c
        source/core.c
        source/core.h
        source/gc/gc.c
        source/gc/gc.h
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
        source/gc/mark.c
        source/gc/mark.h
        source/errors/errors.c
        source/errors/errors.h
)

target_link_libraries(type_v stdio string stdcore stdmath stdfs m)
