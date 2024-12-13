project(stdfs VERSION 1.0)


set(stdfs_files
        source/stdfs.c
        source/fs.c
        source/fs.h
)


add_library(stdfs SHARED ${stdfs_files})

target_include_directories(stdfs PUBLIC ../source) # Include VM headers for reference

# Add linker options for deferring symbol resolution based on OS
if(APPLE)
    # macOS: Allow undefined symbols to be resolved at runtime
    target_link_options(stdfs PRIVATE -undefined dynamic_lookup)
elseif(UNIX)
    # Linux: Allow undefined symbols in shared libraries
    target_link_options(stdfs PRIVATE -Wl,--unresolved-symbols=ignore-in-shared-libs)
elseif(WIN32)
    # Windows: Use /DELAYLOAD to defer DLL resolution until runtime (if applicable)
    # Note: Symbols need to be exported correctly from the main executable
    target_link_options(stdfs PRIVATE /DELAYLOAD:type_v.exe)
endif()

set_target_properties(stdfs PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED YES
    C_EXTENSIONS NO
)
