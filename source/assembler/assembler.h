//
// Created by praisethemoon on 28.11.23.
//

#ifndef TYPE_V_ASSEMBLER_H
#define TYPE_V_ASSEMBLER_H

#include <stdint.h>
#include <stdlib.h>

#include "../instructions/instructions.h"

typedef enum TypeV_ASM_Regs {
    TOK_R0 = 0,
    TOK_R1,
    TOK_R2,
    TOK_R3,
    TOK_R4,
    TOK_R5,
    TOK_R6,
    TOK_R7,
    TOK_R8,
    TOK_R9,
    TOK_R10,
    TOK_R11,
    TOK_R12,
    TOK_R13,
    TOK_R14,
    TOK_R15,
    TOK_R16,
    TOK_R17,
}TypeV_ASM_Regs;

typedef enum TypeV_ASM_Type{
    ASM_I8, ASM_I16, ASM_I32, ASM_I64,
    ASM_U8, ASM_U16, ASM_U32, ASM_U64,
    ASM_F32, ASM_F64, ASM_PTR, ASM_ARRAY,
    ASM_STRUCT, ASM_CLASS, ASM_INTERFACE
}TypeV_ASM_Type;

typedef struct TypeV_ASM_Const{
    char* name;
    uint8_t type;
    uint64_t offset; ///< offset from the start of the segment
}TypeV_ASM_Const;

typedef struct TypeV_ASM_Global{
    char* name;
    uint8_t type;
    uint64_t offset; ///< offset from the start of the segment
}TypeV_ASM_Global;

typedef struct TypeV_ASM_Instruction {
    TypeV_OpCode opcode;
    uint64_t arg1;
    uint64_t arg2;
    uint64_t arg3;
}TypeV_ASM_Instruction;

typedef struct TypeV_ASMProgram {
    uint8_t version;
    uint64_t constPoolOffset;
    uint64_t globlaPoolOffset;
    uint64_t codeOffset;

    uint8_t *constPool;
    uint8_t *globalPool;
    uint8_t *code;
}TypeV_ASMProgram;

typedef struct TypeV_ASM_Lexer {
    uint64_t pos;
    uint64_t line;
    uint64_t col;
    char* program;
    size_t program_length;
}TypeV_ASM_Lexer;

typedef struct TypeV_ASM_Parser {
    uint8_t version;
    TypeV_ASM_Lexer* lexer;
    TypeV_ASM_Const** constPool;
    TypeV_ASM_Global** globalPool;
    TypeV_ASM_Instruction** code;
}TypeV_ASM_Parser;

void lexer_init(TypeV_ASM_Lexer* lexer, char* program);
void parser_init(TypeV_ASM_Parser* parser, TypeV_ASM_Lexer* lexer);
void parse(TypeV_ASM_Lexer* lexer, TypeV_ASM_Parser* parser);

#endif //TYPE_V_ASSEMBLER_H
