/**
 * Type-V VM
 * Author: praisethemoon
 * env.h: Type-V Runtime environment
 */

#ifndef TYPE_V_ENV_H
#define TYPE_V_ENV_H

#include "../vendor/cpu_info/cpu_info.h"

#include "../platform/platform.h"
#include "../dynlib/dynlib.h"

typedef struct  {
    const char* cwd;
    const char* os;
    const char* arch;
    const char* dynlib_ext;
    cpui_result result;
    char** searchPaths;
}TypeV_ENV;


void typev_env_init();
void typev_env_log();
TypeV_ENV get_env();

#endif //TYPE_V_ENV_H
