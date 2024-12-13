project(stdmath VERSION 1.0)


set(stdmath_files source/stdmath.c)

add_library(stdmath SHARED ${stdmath_files})


target_include_directories(stdmath PUBLIC ../source) # Include VM headers for reference

# Add linker options for deferring symbol resolution based on OS
if(APPLE)
    # macOS: Allow undefined symbols to be resolved at runtime
    target_link_options(stdmath PRIVATE -undefined dynamic_lookup)
elseif(UNIX)
    # Linux: Allow undefined symbols in shared libraries
    target_link_options(stdmath PRIVATE -Wl,--unresolved-symbols=ignore-in-shared-libs)
elseif(WIN32)
    # Windows: Use /DELAYLOAD to defer DLL resolution until runtime (if applicable)
    # Note: Symbols need to be exported correctly from the main executable
    target_link_options(stdmath PRIVATE /DELAYLOAD:type_v.exe)
endif()

set_target_properties(stdmath PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED YES
        C_EXTENSIONS NO
)
