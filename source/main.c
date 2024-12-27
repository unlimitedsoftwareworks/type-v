#include <stdio.h>
#include <stdlib.h>


#include "utils/log.h"
#include "env/env.h"

#include "engine.h"
#include "platform/platform.h"
#include "instructions/instructions.h"
#include "assembler/assembler.h"
#include "api/typev_api.h"

// to forcibly include the library
void force_include() {
    typev_api_register_lib(NULL); // Dummy call to force inclusion
}

char* read_file(char* src){
    FILE* file = fopen(src, "r");
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* buffer = malloc(size + 1);
    fread(buffer, 1, size, file);
    buffer[size] = 0;
    fclose(file);
    return buffer;
}

uint8_t *readSegment(FILE *file, uint64_t offset, size_t size);
char* replace_with_src_map(const char* filepath);

int main(int argc, char **argv) {
    int readArgs = 1;
    char *filePath = "../../type-c/output/bin.tcv"; // Change to your file's path

    if (argc >= 2){
        if(argv[1] != NULL) {
            filePath = argv[1];
        }
        readArgs++;
    }
    
    char *srcMapFile = replace_with_src_map(filePath);
    if (srcMapFile == NULL) {
        fprintf(stderr, "Failed to get srcMapPath\n");
        return 1;
    }


    FILE *file = fopen(filePath, "rb");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    // Read offsets
    uint64_t constantOffset, globalOffset, templateOffset, objectKeysOffset, codeOffset;
    fread(&constantOffset, sizeof(uint64_t), 1, file);
    fread(&globalOffset, sizeof(uint64_t), 1, file);
    fread(&templateOffset, sizeof(uint64_t), 1, file);
    fread(&objectKeysOffset, sizeof(uint64_t), 1, file);
    fread(&codeOffset, sizeof(uint64_t), 1, file);

    // Calculate segment sizes
    size_t constantSize = globalOffset - constantOffset;
    size_t globalSize = templateOffset - globalOffset;
    size_t templateSize = codeOffset - templateOffset;
    size_t objectKeysSize = codeOffset - objectKeysOffset;
    fseek(file, 0, SEEK_END);
    size_t codeSize = ftell(file) - codeOffset;

    // Read segments
    uint8_t *constantSegment = readSegment(file, constantOffset, constantSize);
    uint8_t *globalSegment = readSegment(file, globalOffset, globalSize);
    uint8_t *templateSegment = readSegment(file, templateOffset, templateSize);
    uint8_t *objectKeysSegment = readSegment(file, objectKeysOffset, objectKeysSize);
    uint8_t *codeSegment = readSegment(file, codeOffset, codeSize);

    fclose(file);

    typev_env_init(srcMapFile);
    //typev_env_log();

    TypeV_Engine engine;
    engine_init(&engine, argc-readArgs, argv+readArgs);

    TypeV_ASM_Program program = {
            .codePool = codeSegment,
            .codePoolSize = codeSize,
            .constPool = constantSegment,
            .constPoolSize = constantSize,
            .templatePool = templateSegment,
            .templatePoolSize = templateSize,
            .globalPool = globalSegment,
            .globalPoolSize = globalSize,
            .objKeysPool = objectKeysSegment,
            .objKeysPoolSize = objectKeysSize,
            .version = 0
    };

    //debug_program(&program);

    engine_setmain(&engine, program.codePool, program.codePoolSize,
                   program.constPool, program.constPoolSize,
                   program.globalPool, program.globalPoolSize,
                   program.templatePool, program.templatePoolSize,
                     program.objKeysPool, program.objKeysPoolSize,
                   1024, 1024);


    engine_run(&engine);

    uint32_t exitCode = engine.mainCoreExitCode;

    //engine_deallocate(&engine);
    //free_program(program);


    // Use the segment pointers...
    // Remember to free them after use
    //free(constantSegment);
    //free(globalSegment);
    //free(codeSegment);

    return exitCode;
}

uint8_t *readSegment(FILE *file, uint64_t offset, size_t size) {
    uint8_t *buffer = (uint8_t *)malloc(size);
    if (buffer == NULL) {
        perror("Memory allocation failed");
        exit(1);
    }
    fseek(file, offset, SEEK_SET);
    fread(buffer, 1, size, file);
    return buffer;
}


// Function to replace the file name and extension with "src_map.map.txt"
char* replace_with_src_map(const char* filepath) {
    if (filepath == NULL) {
        return NULL;
    }

    // Find the last occurrence of the directory separator
#ifdef _WIN32
    const char separator = '\\';
#else
    const char separator = '/';
#endif

    const char* last_separator = strrchr(filepath, separator);
    size_t base_len = (last_separator != NULL) ? (last_separator - filepath + 1) : 0;

    // Allocate memory for the new path
    const char* replacement = "src_map.map.txt";
    size_t new_path_len = base_len + strlen(replacement) + 1;
    char* new_path = (char*)malloc(new_path_len);

    if (new_path == NULL) {
        perror("Failed to allocate memory");
        return NULL;
    }

    // Construct the new path
    if (base_len > 0) {
        strncpy(new_path, filepath, base_len);
    }
    new_path[base_len] = '\0';
    strcat(new_path, replacement);

    return new_path;
}
