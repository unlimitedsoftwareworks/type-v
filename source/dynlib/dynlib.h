/**
 * Type-V Virtual Machine
 * Author: praisethemoon
 * dynlib.h: Dynamic library (DLL, SO etc) loader
 */

#ifndef TYPEV_DYNLIB_H
#define TYPEV_DYNLIB_H

#include "../env/env.h"

// Handle type for a loaded library
typedef void* TV_LibraryHandle;

// Function prototypes
char* ffi_find_dynlib(const char* dynlib_name);
TV_LibraryHandle ffi_dynlib_load(const char* path);
void ffi_dynlib_unload(TV_LibraryHandle handle);
void* ffi_dynlib_getsym(TV_LibraryHandle handle, const char* symbol_name);


#if defined(_WIN32) || defined(_WIN64)
#define DYNLIB_EXPORT __declspec(dllexport)
#else
#define DYNLIB_EXPORT __attribute__((visibility("default")))
#endif

#endif // TYPEV_DYNLIB_H
