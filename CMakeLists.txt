cmake_minimum_required(VERSION 3.25)
project(typev C)

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

# Add subdirectories for standard libraries
add_subdirectory(stdio)
add_subdirectory(stdfs)
add_subdirectory(stdcore)
add_subdirectory(stdmath)

# Create a static library target
add_library(typev_static STATIC
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

# Your executable target
add_executable(typev
        source/main.c
)

set_target_properties(typev_static PROPERTIES OUTPUT_NAME "typev")

# Link the static library to the executable
target_link_libraries(typev typev_static stdio stdcore stdmath stdfs m)

# Install rules
# Install the static library
install(TARGETS typev_static
        ARCHIVE DESTINATION lib)

# Install selected headers
# Install only the specific headers, preserving the directory structure
install(FILES
        source/core.h
        source/engine.h
        source/dynlib/dynlib.h
        source/errors/errors.h
        source/api/typev_api.h
        DESTINATION include/typev)
install(FILES
        source/dynlib/dynlib.h
        DESTINATION include/typev/dynlib)
install(FILES
        source/errors/errors.h
        DESTINATION include/typev/errors)
install(FILES
        source/api/typev_api.h
        DESTINATION include/typev/api)
install(FILES
        source/env/env.h
        DESTINATION include/typev/env)
install(FILES
        source/platform/platform.h
        DESTINATION include/typev/platform)