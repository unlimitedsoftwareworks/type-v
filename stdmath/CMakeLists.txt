project(stdmath VERSION 1.0)


set(stdmath_files source/stdmath.c)

add_library(stdmath SHARED ${stdmath_files})


target_include_directories(stdmath PUBLIC ${CMAKE_SOURCE_DIR}/source)
target_link_libraries(stdmath PUBLIC typev_static m)

set_target_properties(stdmath PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED YES
        C_EXTENSIONS NO
)

install(TARGETS stdmath LIBRARY DESTINATION lib)