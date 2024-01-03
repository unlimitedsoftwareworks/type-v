/**
 * Type-V Virtual Machine
 * Author: praisethemoon
 * utils.h: VM Instructions
 * Utilities
 */

#ifndef TYPE_V_UTILS_H
#define TYPE_V_UTILS_H


#ifndef __FUNCTION_NAME__
#ifdef WIN32   //WINDOWS
#define __FUNCTION_NAME__   __FUNCTION__
#else          //*NIX
#define __FUNCTION_NAME__   __func__
#endif
#endif

#define TYPE_V_ASSERT_LOG

#if defined(TYPE_V_ASSERT_STANDARD)
#define ASSERT(c ,msg, ...) assert(c)
#elif defined (TYPE_V_ASSERT_LOG)
#define ASSERT(c, msg, ...) typev_assert(c, #c, __FUNCTION_NAME__ , msg, ##__VA_ARGS__)
#elif defined (TYPE_V_ASSERT_NONE)
#define ASSERT(c, msg, ...)
#else
#error Please define an assertion policy.
#endif

void typev_assert(int cond, const char * rawcond, const char* func_name, const char * fmt, ...);
int get_source_map_line_content(const char* sourceMapFile, uint64_t position, char* file, uint64_t* line, uint64_t* pos);

#endif //TYPE_V_UTILS_H
