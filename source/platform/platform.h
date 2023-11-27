//
// Created by praisethemoon on 21.11.23.
//

#ifndef TYPE_V_PLATFORM_H
#define TYPE_V_PLATFORM_H

#if defined(_WIN32) || defined(_WIN64)
#define OS_NAME "Windows"
#elif defined(__linux__)
#define OS_NAME "Linux"
#elif defined(__APPLE__) && defined(__MACH__)
#define OS_NAME "macOS"
#else
#define OS_NAME "Unknown"
#endif

#if defined(__x86_64__) || defined(_M_X64)
#define CPU_ARCH "x86_64"
#elif defined(__i386__) || defined(_M_IX86)
#define CPU_ARCH "x86"
#elif defined(__arm__)
#define CPU_ARCH "ARM"
#elif defined(__aarch64__)
#define CPU_ARCH "ARM64"
#else
#define CPU_ARCH "Unknown"
#endif

/* Detecting Compiler Name and Version */

/* Helper macros for stringizing */
#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)

/* Detecting Compiler Name and Version */
#if defined(__clang__)
/* Clang/LLVM */
#define COMPILER "Clang " STRINGIZE(__clang_major__) "." STRINGIZE(__clang_minor__) "." STRINGIZE(__clang_patchlevel__)

#elif defined(__INTEL_COMPILER)
/* Intel C++ Compiler */
#define COMPILER "Intel C++ Compiler " STRINGIZE(__INTEL_COMPILER)

#elif defined(__GNUC__)
/* GNU Compiler Collection (GCC) */
#if defined(__GNUC_PATCHLEVEL__)
#define COMPILER "GCC " STRINGIZE(__GNUC__) "." STRINGIZE(__GNUC_MINOR__) "." STRINGIZE(__GNUC_PATCHLEVEL__)
#else
#define COMPILER "GCC " STRINGIZE(__GNUC__) "." STRINGIZE(__GNUC_MINOR__)
#endif

#elif defined(_MSC_VER)
/* Microsoft Visual C++ */
#define COMPILER "MSVC " STRINGIZE(_MSC_VER)

#else
/* Unknown Compiler */
#define COMPILER "Unknown Compiler"

#endif



#endif //TYPE_V_PLATFORM_H
