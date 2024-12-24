project(stdio VERSION 1.0)

set(stdio_files source/stdio.c)


add_library(stdio SHARED ${stdio_files})
target_include_directories(stdio PUBLIC ${CMAKE_SOURCE_DIR}/source)
target_link_libraries(stdio PUBLIC typev_static)

# Use proper C standard
set_target_properties(stdio PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED YES
        C_EXTENSIONS NO
)

install(TARGETS stdio LIBRARY DESTINATION lib)