
#include <stdlib.h>
#ifdef _WIN32
#include <direct.h>
#define getcwd _getcwd
#else
#include <unistd.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#include "env.h"

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
}

void typev_env_log(){
    printf("Type-V VM \n  - OS: %s \n  - Arch: %s \n  - CC: %s\n", env.os, env.arch, COMPILER);
    printf("Current working directory: %s\n", env.cwd);
}

TypeV_ENV get_env(){
    return env;
}