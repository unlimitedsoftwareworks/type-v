
#include <stdlib.h>
#ifdef _WIN32
#include <direct.h>
#define getcwd _getcwd
#else
#include <unistd.h>
#endif


#include <string.h>
#include "env.h"
#include "../utils/utils.h"

#define CPU_INFO_IMPLEMENTATION
#include "../vendor/cpu_info/cpu_info.h"

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>




static TypeV_ENV env= {
        .cwd = NULL,
        .os = OS_NAME,
        .arch = CPU_ARCH,
        .dynlib_ext =
#ifdef _WIN32
        ".dll"
#elif defined(__APPLE__) && defined(__MACH__)
        ".dylib"
#else
        ".so"
#endif
};

void typev_env_init(char* sourceMapFile){
    char *cwd;
     // PATH_MAX is defined in limits.h

    char buffer[4096];

    cwd = getcwd(buffer, 4096);
    if (cwd == NULL) {
        perror("getcwd() error");
        exit(-1);
    } else {
        env.cwd = strdup(cwd);
    }

    env.searchPaths = malloc(3* sizeof(char*));
    printf("current dir: %s\n", env.cwd);

    /*
    int err = cpui_get_info(&env.result);
    if(err){
        fprintf(stderr, "An error occured while quering CPU");
    }
    cpui_log_result(stdout, &env.result);
    */
    env.sourceMapFile = sourceMapFile;

    if(env.sourceMapFile != NULL) {
        printf("Source map file: %s\n", env.sourceMapFile);
    }
}

void typev_env_log(){
    printf("Type-V VM \n  - OS: %s \n  - Arch: %s \n  - CC: %s\n", env.os, env.arch, COMPILER);
    printf("Current working directory: %s\n", env.cwd);
}

TypeV_ENV get_env(){
    return env;
}


uint8_t env_sourcemap_has(TypeV_ENV env) {
    return env.sourceMapFile != NULL;

}

TypeV_SourcePoint env_sourcemap_get(TypeV_ENV env, uint64_t ip){
    if(!env_sourcemap_has(env)) {
        return (TypeV_SourcePoint){.line = 0, .column = 0, .file = NULL};
    }

    char file[1024] = {};
    uint64_t line = 0;
    uint64_t column = 0;
    char func_name[1024] = {};
    int found = get_source_map_line_content(env.sourceMapFile, ip, file, &line, &column, func_name);

    return (TypeV_SourcePoint){.line = line, .column = column, .file = strdup(file), .func_name = strdup(func_name)};
}