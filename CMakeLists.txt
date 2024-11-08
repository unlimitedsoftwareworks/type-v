cmake_minimum_required(VERSION 3.25)
project(type_v C)

set(CMAKE_C_STANDARD 11)

# Your other subdirectories
add_subdirectory(stdio)
add_subdirectory(string)
add_subdirectory(stdcore)

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")

# disable inlining
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-inline")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-inline")

# Your executable target
add_executable(type_v
        source/main.c
        source/core.c
        source/core.h
        source/gc.c
        source/gc.h
        source/engine.c
        source/engine.h
        source/allocator/allocator.c
        source/allocator/allocator.h
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

#add_dependencies(type_v stdio string stdcore)


# Add the mimalloc include path explicitly
find_path(MIMALLOC_INCLUDE_DIR mimalloc.h
        PATHS
        /usr/local/include
        /usr/local/include/mimalloc-1.8
)

# Locate the mimalloc library
find_library(MIMALLOC_LIBRARY mimalloc
        PATHS
        /usr/local/lib
        /usr/local/lib/mimalloc-1.8
)

# Set the include directory and link the library
if (MIMALLOC_INCLUDE_DIR AND MIMALLOC_LIBRARY)
    # Add the include directory for mimalloc
    include_directories(${MIMALLOC_INCLUDE_DIR})

    # Link mimalloc to each target
    target_include_directories(type_v PRIVATE ${MIMALLOC_INCLUDE_DIR})
    target_link_libraries(type_v stdio string stdcore ${MIMALLOC_LIBRARY})

    # Link mimalloc for all dependent targets
    target_include_directories(stdio PRIVATE ${MIMALLOC_INCLUDE_DIR})
    target_link_libraries(stdio PRIVATE ${MIMALLOC_LIBRARY})

    target_include_directories(string PRIVATE ${MIMALLOC_INCLUDE_DIR})
    target_link_libraries(string PRIVATE ${MIMALLOC_LIBRARY})

    target_include_directories(stdcore PRIVATE ${MIMALLOC_INCLUDE_DIR})
    target_link_libraries(stdcore PRIVATE ${MIMALLOC_LIBRARY})

else()
    message(FATAL_ERROR "mimalloc library not found.")
endif()
