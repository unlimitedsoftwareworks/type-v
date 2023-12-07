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

int main() {
    typev_env_init();
    typev_env_log();

    TypeV_Engine engine;
    engine_init(&engine);

    char* source = read_file("../samples/sample1.tv");
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
