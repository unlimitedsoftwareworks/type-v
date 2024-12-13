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

target_include_directories(stdcore PUBLIC ../source) # Include VM headers for reference

# Add linker options for deferring symbol resolution based on OS
if(APPLE)
    target_link_options(stdcore PRIVATE -undefined dynamic_lookup)

elseif(UNIX)
    # Linux: Allow undefined symbols in shared libraries
    target_link_options(stdcore PRIVATE -Wl,--unresolved-symbols=ignore-in-shared-libs)
elseif(WIN32)
    # Windows: Use /DELAYLOAD to defer DLL resolution until runtime (if applicable)
    # Note: Symbols need to be exported correctly from the main executable
    target_link_options(stdcore PRIVATE /DELAYLOAD:type_v.exe)
endif()

# Use proper C standard
set_target_properties(stdcore PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED YES
        C_EXTENSIONS NO
)