project(stdfs VERSION 1.0)


set(stdfs_files
        source/stdfs.c
        source/fs.c
        source/fs.h
)

add_library(stdfs SHARED ${stdfs_files})

target_include_directories(stdfs PUBLIC ${CMAKE_SOURCE_DIR}/source)
target_link_libraries(stdfs PUBLIC typev_static)

set_target_properties(stdfs PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED YES
        C_EXTENSIONS NO
)


install(TARGETS stdfs LIBRARY DESTINATION lib)