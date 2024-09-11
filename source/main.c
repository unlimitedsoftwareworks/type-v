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
/*
int main() {
    typev_env_init();
    typev_env_log();

    TypeV_Engine engine;
    engine_init(&engine);

    char* source = read_file("../samples/sample2.tv");
    TypeV_ASM_Lexer lexer;
    lexer_init(&lexer, source);
    TypeV_ASM_Parser parser;
    parser_init(&parser, &lexer);
    parse(&lexer, &parser);
    TypeV_ASM_Program* program = assemble(&parser);
    free(source);
    lexer_free(&lexer);
    parser_free(&parser);

    LOG_INFO("Program assembled successfully, running.");


    debug_program(program);

    engine_setmain(&engine, program->codePool, program->codePoolSize,
                   program->constPool, program->constPoolSize,
                   program->globalPool, program->globalPoolSize, 1024, 1024);

    engine_run(&engine);

    engine_deallocate(&engine);
    free_program(program);
    //free(program);
}
*/

// Function prototypes
uint8_t *readSegment(FILE *file, uint64_t offset, size_t size);

int main(int argc, char **argv) {

    char *filePath = "/Users/praisethemoon/projects/type-c/type-c/output/bin.tcv"; // Change to your file's path
    char *srcMapFile = "/Users/praisethemoon/projects/type-c/type-c/output/src_map.map.txt";
    if (argc > 1){
        filePath = argv[1];
    }
    if (argc > 2){
        srcMapFile = argv[2];
    }

    FILE *file = fopen(filePath, "rb");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    // Read offsets
    uint64_t constantOffset, globalOffset, codeOffset;
    fread(&constantOffset, sizeof(uint64_t), 1, file);
    fread(&globalOffset, sizeof(uint64_t), 1, file);
    fread(&codeOffset, sizeof(uint64_t), 1, file);

    // Calculate segment sizes
    size_t constantSize = globalOffset - constantOffset;
    size_t globalSize = codeOffset - globalOffset;
    fseek(file, 0, SEEK_END);
    size_t codeSize = ftell(file) - codeOffset;

    // Read segments
    uint8_t *constantSegment = readSegment(file, constantOffset, constantSize);
    uint8_t *globalSegment = readSegment(file, globalOffset, globalSize);
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
            .globalPool = globalSegment,
            .globalPoolSize = globalSize,
            .version = 0
    };

    //debug_program(&program);

    engine_setmain(&engine, program.codePool, program.codePoolSize,
                   program.constPool, program.constPoolSize,
                   program.globalPool, program.globalPoolSize, 1024, 1024);


    engine_run(&engine);

    uint32_t exitCode = engine.mainCoreExitCode;

    engine_deallocate(&engine);
    //free_program(program);


    // Use the segment pointers...
    // Remember to free them after use
    //free(constantSegment);
    //free(globalSegment);
    //free(codeSegment);

    printf("done\n");

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
