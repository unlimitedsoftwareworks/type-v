#include <stdio.h>
#include <stdlib.h>


#include "utils/log.h"
#include "env/env.h"

#include "engine.h"
#include "platform/platform.h"
#include "instructions/instructions.h"
#include "assembler/assembler.h"

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

int main(int argc, char **argv) {
    int readArgs = 0;
    char *filePath = "/Users/praisethemoon/projects/type-c/type-c/output/bin.tcv"; // Change to your file's path
    char *srcMapFile = "/Users/praisethemoon/projects/type-c/type-c/output/src_map.map.txt";
    if (argc > 1){
        filePath = argv[1];
        readArgs++;
    }
    if (argc > 2){
        srcMapFile = argv[2];
        readArgs++;
    }


    /*
    if(argc < 2){
        printf("Usage: %s <path to bin.tcv> <path to src_map.map.txt>\n", argv[0]);
        return 1;
    }
    */

    FILE *file = fopen(filePath, "rb");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    // Read offsets
    uint64_t constantOffset, globalOffset, templateOffset, codeOffset;
    fread(&constantOffset, sizeof(uint64_t), 1, file);
    fread(&globalOffset, sizeof(uint64_t), 1, file);
    fread(&templateOffset, sizeof(uint64_t), 1, file);
    fread(&codeOffset, sizeof(uint64_t), 1, file);

    // Calculate segment sizes
    size_t constantSize = globalOffset - constantOffset;
    size_t globalSize = templateOffset - globalOffset;
    size_t templateSize = codeOffset - templateOffset;
    fseek(file, 0, SEEK_END);
    size_t codeSize = ftell(file) - codeOffset;

    // Read segments
    uint8_t *constantSegment = readSegment(file, constantOffset, constantSize);
    uint8_t *globalSegment = readSegment(file, globalOffset, globalSize);
    uint8_t *templateSegment = readSegment(file, templateOffset, templateSize);
    uint8_t *codeSegment = readSegment(file, codeOffset, codeSize);

    fclose(file);

    typev_env_init(srcMapFile);
    typev_env_log();

    TypeV_Engine engine;
    engine_init(&engine);

    TypeV_ASM_Program program = {
            .codePool = codeSegment,
            .codePoolSize = codeSize,
            .constPool = constantSegment,
            .constPoolSize = constantSize,
            .templatePool = templateSegment,
            .templatePoolSize = templateSize,
            .globalPool = globalSegment,
            .globalPoolSize = globalSize,
            .version = 0
    };

    //debug_program(&program);

    engine_setmain(&engine, program.codePool, program.codePoolSize,
                   program.constPool, program.constPoolSize,
                   program.globalPool, program.globalPoolSize,
                   program.templatePool, program.templatePoolSize, 1024, 1024);


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
