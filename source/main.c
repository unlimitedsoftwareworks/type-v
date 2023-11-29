#include <stdio.h>
#include <stdlib.h>


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
    fprintf(stdout, "Type-V VM %s, %s, %s\n", OS_NAME, CPU_ARCH, COMPILER);

    TypeV_Engine engine;
    engine_init(&engine);

    uint8_t globalPool[]= {0x80};
    uint64_t globalPoolLength = 1;

    uint8_t constPool [] = {0xc3, 0x5f, 0x48, 0x40, 23};
    uint64_t constPoolLength = 5;

    /*
    uint8_t program[] = {
            OP_FRAME_PRECALL,
            OP_FRAME_INIT_ARGS, 1, 4,

            OP_PUSH_CONST, 1, 0, 2,
            OP_PUSH_CONST, 1, 2, 2,
            OP_MV_REG_CONST_8, 2, 1, 4,
            OP_DEBUG_REGS, 2,
            OP_FN_CALL, 2,
            OP_DEBUG_REGS, 19,
            OP_HALT,

            OP_MV_REG_ARG_16, 0, 1,0,
            OP_MV_REG_ARG_16, 19, 1,0,
            OP_FN_MAIN,
            OP_DEBUG_REGS, 0,
            OP_DEBUG_REGS, 1,
            OP_FRAME_RM,
            OP_FN_RET,
    };
    uint64_t programLength = 38;

    /*
    uint8_t program [] = {
            OP_S_ALLOC, 0x02, 0x01, 4,
            OP_S_SET_OFFSET, 0, 1, 0,
            OP_S_STOREF_CONST_32, 0, 1, 0,
            OP_S_LOADF_32, 2, 0,
            //OP_S_STOREF_REG, 0, 1, 4,
            OP_DEBUG_REGS, 2,
            OP_HALT
    };


    engine_setmain(&engine, program, programLength, constPool, constPoolLength, globalPool, globalPoolLength, 1024, 1024);

    engine_run(&engine);

    engine_deallocate(&engine);
    return 0;
     */


    char* program = read_file("../samples/sample1.tv");
    TypeV_ASM_Lexer lexer;
    lexer_init(&lexer, program);
    TypeV_ASM_Parser parser;
    parser_init(&parser, &lexer);
    parse(&lexer, &parser);
}
