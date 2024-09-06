cmake_minimum_required(VERSION 3.25)
project(type_v C)

set(CMAKE_C_STANDARD 11)

add_subdirectory(stdio)
add_subdirectory(stdcore)


#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-inline")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-inline")


#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=undefined")
####set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=leak")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=pointer-compare")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=pointer-subtract")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=pointer-overflow")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=bounds")

add_executable(
        type_v source/main.c
        source/core.c
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


# Find libuv package
find_path(LIBUV_INCLUDE_DIR NAMES uv.h)
find_library(LIBUV_LIBRARY NAMES uv libuv)

# Handle the QUIETLY/REQUIRED arguments, set LIBUV_FOUND if all variables are found
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LIBUV
        REQUIRED_VARS
        LIBUV_LIBRARY
        LIBUV_INCLUDE_DIR
)

# Set standard variables
if(LIBUV_FOUND)
    set(LIBUV_INCLUDE_DIRS "${LIBUV_INCLUDE_DIR}")
    set(LIBUV_LIBRARIES "${LIBUV_LIBRARY}")
endif()

# Include directories for libuv
include_directories(${LIBUV_INCLUDE_DIRS})

add_dependencies(type_v stdio stdcore)
target_link_libraries(type_v ${LIBUV_LIBRARIES})

# Hide internal variables
mark_as_advanced(LIBUV_INCLUDE_DIR LIBUV_LIBRARY)