
#include <stdlib.h>
#ifdef _WIN32
#include <direct.h>
#define getcwd _getcwd
#else
#include <unistd.h>
#endif

#include "env.h"

#define CPU_INFO_IMPLEMENTATION
#include "../vendor/cpu_info/cpu_info.h"

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>


char buffer[PATH_MAX];


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

void typev_env_init(){
    char *cwd;
     // PATH_MAX is defined in limits.h

    cwd = getcwd(buffer, PATH_MAX);
    if (cwd == NULL) {
        perror("getcwd() error");
        exit(-1);
    } else {
        env.cwd = cwd;
    }

    env.searchPaths = calloc(3, sizeof(char*));
    printf("current dir: %s\n", env.cwd);

    ;
    int err = cpui_get_info(&env.result);
    if(err){
        fprintf(stderr, "An error occured while quering CPU");
    }
    cpui_log_result(stdout, &env.result);
}

void typev_env_log(){
    printf("Type-V VM \n  - OS: %s \n  - Arch: %s \n  - CC: %s\n", env.os, env.arch, COMPILER);
    printf("Current working directory: %s\n", env.cwd);
}

TypeV_ENV get_env(){
    return env;
}