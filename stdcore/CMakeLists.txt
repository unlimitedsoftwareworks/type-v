project(stdcore VERSION 1.0)


set(stdcore_files source/stdcore.c
        source/datetime.c
        source/vendor/yy.c
        source/vendor/yy.h
        source/vendor/atoi_yy.c
        source/vendor/utils_yy.h
        source/vendor/itoa_yy.c
        source/vendor/dtoa_yy.c
        source/datetime.c
        source/datetime.h)

add_library(stdcore SHARED ${stdcore_files})

target_include_directories(stdcore PUBLIC ${CMAKE_SOURCE_DIR}/source)
target_link_libraries(stdcore PUBLIC typev_static)

# Use proper C standard
set_target_properties(stdcore PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED YES
        C_EXTENSIONS NO
)

install(TARGETS stdcore LIBRARY DESTINATION lib)