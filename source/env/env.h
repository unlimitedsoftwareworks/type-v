/**
 * Type-V VM
 * Author: praisethemoon
 * env.h: Type-V Runtime environment
 */

#ifndef TYPE_V_ENV_H
#define TYPE_V_ENV_H

//#include "../vendor/cpu_info/cpu_info.h"

#include "../platform/platform.h"
#include "../dynlib/dynlib.h"

typedef struct {
    char* file;
    char* func_name;
    uint64_t line;
    uint64_t column;
}TypeV_SourcePoint;

typedef struct  {
    const char* cwd;
    const char* os;
    const char* arch;
    const char* dynlib_ext;
    //cpui_result result;
    char** searchPaths;
    char* sourceMapFile;
}TypeV_ENV;


void typev_env_init(char* sourceMapFile);
void typev_env_log();
TypeV_ENV get_env();


/**
 * @brief Checks if the engine has a source map available
 * @param engine
 * @return
 */
uint8_t env_sourcemap_has(TypeV_ENV);

/**
 * @brief  Get the source location of an instruction from the source map
 * @param engine
 * @param ip
 * @param buffer
 * @param bufferSize
 */
TypeV_SourcePoint env_sourcemap_get(TypeV_ENV env, uint64_t ip);


#endif //TYPE_V_ENV_H
