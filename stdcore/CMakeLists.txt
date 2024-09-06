project(stdcore VERSION 1.0)


set(stdcore_files source/stdcore.c
        source/vendor/yy.c
        source/vendor/yy.h
        source/vendor/atoi_yy.c
        source/vendor/utils_yy.h
        source/vendor/itoa_yy.c
        source/vendor/dtoa_yy.c)

set(typev_files
        ../source/core.c
        ../source/stack/stack.c
        ../source/dynlib/dynlib.c
        ../source/env/env.c
        ../source/api/typev_api.c
        ../source/utils/utils.c
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

add_library(stdcore SHARED ${stdcore_files} ${typev_files})
target_link_libraries(stdcore ${LIBUV_LIBRARIES})

set_target_properties(stdcore PROPERTIES
        C_STANDARD 99
        C_STANDARD_REQUIRED YES
        C_EXTENSIONS NO
)
