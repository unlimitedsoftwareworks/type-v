/**
 * Type-V VM
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
const char* ffi_dynlib_getext();

#endif // DYNAMIC_LIBRARY_H
