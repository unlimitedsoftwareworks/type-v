//
// Created by praisethemoon on 03.12.23.
//

#include <string.h>
#include <stdio.h>
#include "dynlib.h"

#include "dynlib.h"
#include "../env/env.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

char* ffi_find_dynlib(const char* dynlib_name) {

    char buffer[1024];
    TypeV_ENV env = get_env();
    snprintf(buffer, 1024, "%s/%s/lib%s%s", env.cwd, dynlib_name, dynlib_name, env.dynlib_ext);
    return strdup(buffer);
}

// Load a dynamic library
TV_LibraryHandle ffi_dynlib_load(const char* name) {
    char* path = ffi_find_dynlib(name);
#ifdef _WIN32
    return (TV_LibraryHandle)LoadLibraryA(path);
#else
    return (TV_LibraryHandle)dlopen(path, RTLD_LAZY);
#endif
}

// Unload a dynamic library
void ffi_dynlib_unload(TV_LibraryHandle handle) {
    if (handle) {
#ifdef _WIN32
        FreeLibrary((HMODULE)handle);
#else
        dlclose(handle);
#endif
    }
}

// Get a symbol from a dynamic library
void* ffi_dynlib_getsym(TV_LibraryHandle handle, const char* symbol_name) {
#ifdef _WIN32
    return (void*)GetProcAddress((HMODULE)handle, symbol_name);
#else
    return dlsym(handle, symbol_name);
#endif
}


