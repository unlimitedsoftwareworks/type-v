project(stdio VERSION 1.0)

set(stdio_files source/stdio.c)

add_library(stdio SHARED ${stdio_files})



target_include_directories(stdio PUBLIC ../source) # Include VM headers for reference

# Add linker options for deferring symbol resolution based on OS
if(APPLE)
    target_link_options(stdio PRIVATE -undefined dynamic_lookup)

elseif(UNIX)
    # Linux: Allow undefined symbols in shared libraries
    target_link_options(stdio PRIVATE -Wl,--unresolved-symbols=ignore-in-shared-libs)
elseif(WIN32)
    # Windows: Use /DELAYLOAD to defer DLL resolution until runtime (if applicable)
    # Note: Symbols need to be exported correctly from the main executable
    target_link_options(stdio PRIVATE /DELAYLOAD:type_v.exe)
endif()

# Use proper C standard
set_target_properties(stdio PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED YES
        C_EXTENSIONS NO
)