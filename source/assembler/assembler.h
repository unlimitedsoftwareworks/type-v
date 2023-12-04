/**
 * Type-V Virtual Machine
 * Author: praisethemoon
 * assembler.h: Simple Assembler for Type-V
 * This assembler is used to assemble Type-V assembly into Type-V bytecode.
 * It is not a full-fledged assembler, its rather low level and simple.
 * Also supports disassembling Type-V bytecode into Type-V assembly.
 */


#ifndef TYPE_V_ASSEMBLER_H
#define TYPE_V_ASSEMBLER_H

#include <stdint.h>
#include <stdlib.h>

#include "../instructions/instructions.h"

typedef struct {
    void** items;
    size_t capacity;
    size_t total;
}Vector;
void vector_init(Vector* v);
int vector_total(Vector* v);
static void vector_resize(Vector* v, size_t capacity);
void vector_add(Vector* v, void* item);
void* vector_get(Vector* v, size_t index);
void vector_set(Vector* v, size_t index, void* item);
void vector_delete(Vector* v, size_t index);
void vector_free(Vector* v);

static char* instructions[] = {
        // same order as in instructions.h
        "mv_reg_reg", "mv_reg_i",
        "mv_reg_const_8","mv_reg_const_16","mv_reg_const_32", "mv_reg_const_64", "mv_reg_const_ptr",
        "mv_reg_mem", "mv_mem_reg",
        "mv_reg_local_8", "mv_reg_local_16", "mv_reg_local_32", "mv_reg_local_64", "mv_reg_local_ptr",
        "mv_local_reg_8", "mv_local_reg_16", "mv_local_reg_32", "mv_local_reg_64", "mv_local_reg_ptr",
        "mv_global_reg_8", "mv_global_reg_16", "mv_global_reg_32", "mv_global_reg_64", "mv_global_reg_ptr",
        "mv_reg_global_8", "mv_reg_global_16", "mv_reg_global_32", "mv_reg_global_64", "mv_reg_global_ptr",
        "mv_reg_arg_8", "mv_reg_arg_16", "mv_reg_arg_32", "mv_reg_arg_64", "mv_reg_arg_ptr",
        "mv_arg_reg_8", "mv_arg_reg_16", "mv_arg_reg_32", "mv_arg_reg_64", "mv_arg_reg_ptr",
        "s_alloc", "s_alloc_shadow", "s_set_offset",
        "s_loadf",
        "s_storef_const_8", "s_storef_const_16", "s_storef_const_32", "s_storef_const_64", "s_storef_const_ptr",
        "s_storef_reg",
        "c_allocf", "c_allocm",
        "c_storem", "c_loadm",
        "c_storef_reg",
        "c_storef_const_8", "c_storef_const_16", "c_storef_const_32", "c_storef_const_64", "c_storef_const_ptr",
        "c_loadf",
        "i_alloc", "i_set_offset", "i_loadm",
        "a_alloc", "a_extend",
        "a_storef_reg",
        "a_storef_const_8", "a_storef_const_16", "a_storef_const_32", "a_storef_const_64", "a_storef_const_ptr",
        "a_loadf",
        "push", "push_const", "pop",
        "frame_init_args", "frame_init_locals", "frame_rm", "frame_precall",
        "fn_main", "fn_ret", "fn_call", "fn_calli",

        "cast_i8_u8","cast_u8_i8","cast_i16_u16",
        "cast_u16_i16","cast_i32_u32","cast_u32_i32",
        "cast_i64_u64","cast_u64_i64","cast_i32_f32",
        "cast_f32_i32","cast_i64_f64","cast_f64_i64",

        "upcast_i8_i16","upcast_u8_u16","upcast_i16_i32",
        "upcast_u16_u32","upcast_i32_i64","upcast_u32_u64",
        "upcast_f32_f64",

        "dcast_i16_i8","dcast_u16_u8","dcast_i32_i16",
        "dcast_u32_u16","dcast_i64_i32","dcast_u64_u32",
        "dcast_f64_f32",

        "add_i8","add_u8","add_i16","add_u16",
        "add_i32","add_u32","add_i64","add_u64",
        "add_f32","add_f64",
        "add_ptr_u8",
        "add_ptr_u16",
        "add_ptr_u32",
        "add_ptr_u64",

        "sub_i8","sub_u8",
        "sub_i16","sub_u16",
        "sub_i32","sub_u32",
        "sub_i64","sub_u64",
        "sub_f32","sub_f64",
        "sub_ptr_u8","sub_ptr_u16",
        "sub_ptr_u32","sub_ptr_u64",

        "mul_i8","mul_u8",
        "mul_i16","mul_u16",
        "mul_i32","mul_u32",
        "mul_i64","mul_u64",
        "mul_f32","mul_f64",

        "div_i8","div_u8",
        "div_i16","div_u16",
        "div_i32","div_u32",
        "div_i64","div_u64",
        "div_f32","div_f64",

        "mod_i8","mod_u8",
        "mod_i16","mod_u16",
        "mod_i32","mod_u32",
        "mod_i64","mod_u64",

        "lshift_i8","lshift_u8",
        "lshift_i16","lshift_u16",
        "lshift_i32","lshift_u32",
        "lshift_i64","lshift_u64",

        "rshift_i8","rshift_u8",
        "rshift_i16","rshift_u16",
        "rshift_i32","rshift_u32",
        "rshift_i64","rshift_u64",

        "cmp_i8","cmp_u8",
        "cmp_i16","cmp_u16",
        "cmp_i32","cmp_u32",
        "cmp_i64","cmp_u64",
        "cmp_f32","cmp_f64",
        "cmp_ptr",

        "band_8","band_16","band_32","band_64",

        "bor_8","bor_16","bor_32","bor_64",

        "bxor_8","bxor_16","bxor_32","bxor_64",

        "bnot_8","bnot_16","bnot_32","bnot_64",

        "and","or","not",

        "j","je","jne",
        "jg","jge",
        "jl","jle",

        "ld_ffi",
        "call_ffi",

        "debug_reg",
        "halt"
};
#define MAX_INSTRUCTION 222

typedef enum TokenType {
    TOK_INSTRUCTION=0,
    TOK_IDENTIFIER,
    TOK_NUMBER,
    TOK_STRING,
    TOK_FLOAT,

    TOK_KEYOWRD_VERSION,
    TOK_KEYOWRD_CONST,
    TOK_KEYOWRD_GLOBAL,
    TOK_KEYOWRD_CODE,

    TOK_I8,
    TOK_I16,
    TOK_I32,
    TOK_I64,
    TOK_U8,
    TOK_U16,
    TOK_U32,
    TOK_U64,
    TOK_F32,
    TOK_F64,
    TOK_PTR,

    TOK_R0,
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
    TOK_R18,
    TOK_R19,
    TOK_R20,

    TOK_COLON,
    TOK_COMMA,
    TOK_LBRACKET,
    TOK_RBRACKET,
    TOK_SEMICOLON,
    TOK_AT,
    TOK_EQ,

    TOK_EOF
}TokenType;

typedef struct Token {
    char* value;
    TokenType type;
    uint64_t line;
    uint64_t col;
    uint64_t pos;

    union {
        int instructionId;
        int isDouble;
    };
}Token;
Token* createToken(TokenType type, char* value, uint64_t line, uint64_t col, uint64_t pos);

typedef enum TypeV_ASM_Type{
    ASM_I8=0, ASM_I16, ASM_I32, ASM_I64,
    ASM_U8, ASM_U16, ASM_U32, ASM_U64,
    ASM_F32, ASM_F64, ASM_PTR
}TypeV_ASM_Type;

typedef enum Type_ASM_Reg {
    ASM_R0=0, ASM_R1, ASM_R2, ASM_R3, ASM_R4, ASM_R5, ASM_R6, ASM_R7,
    ASM_R8, ASM_R9, ASM_R10, ASM_R11, ASM_R12, ASM_R13, ASM_R14,
    ASM_R15, ASM_R16, ASM_R17, ASM_R18, ASM_R19, ASM_R20
}TypeV_ASM_Reg;

typedef struct TypeV_ASM_Const{
    char* name;
    TypeV_ASM_Type type;
    uint64_t offset; ///< offset from the start of the segment
    uint64_t value;
    uint8_t offsetSize;
}TypeV_ASM_Const;

typedef struct TypeV_ASM_Global{
    char* name;
    uint8_t type;
    uint64_t offset; ///< offset from the start of the segment
    uint64_t value;
    uint8_t offsetSize;
}TypeV_ASM_Global;

typedef struct TypeV_Label {
    char* name;
    uint64_t primaryCodeOffset;
    uint64_t codeOffset;
}TypeV_Label;

typedef struct TypeV_ASM_Instruction {
    uint64_t offset;
    uint8_t num_args;
    TypeV_OpCode opcode;
    uint64_t arg1;
    uint64_t arg2;
    uint64_t arg3;
}TypeV_ASM_Instruction;

typedef struct TypeV_ASM_Lexer {
    uint64_t pos;
    uint64_t line;
    uint64_t col;
    char* program;
    size_t program_length;
    Vector tokens;
}TypeV_ASM_Lexer;

typedef struct TypeV_ASM_Parser {
    uint8_t version;
    Token* currentToken;
    uint64_t currentTokenIndex;
    TypeV_ASM_Lexer* lexer;
    Vector constPool;
    Vector globalPool;
    Vector codePool;
    Vector labels;

    uint64_t constPoolSize;
    uint64_t globalPoolSize;
    uint64_t codePoolSize;
}TypeV_ASM_Parser;

typedef struct TypeV_ASM_Program {
    uint8_t version;
    uint64_t constPoolSize;
    uint64_t globalPoolSize;
    uint64_t codePoolSize;

    uint8_t *constPool;
    uint8_t *globalPool;
    uint8_t *codePool;
}TypeV_ASM_Program;


TypeV_ASM_Const* create_const(TypeV_ASM_Parser* parser, char* name, TypeV_ASM_Type type, uint64_t value);
TypeV_ASM_Global* create_global(TypeV_ASM_Parser* parser, char* name, uint8_t type, uint64_t value);
TypeV_ASM_Instruction* create_instruction(TypeV_ASM_Parser * parser, TypeV_OpCode opcode, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint8_t num_args);

TypeV_ASM_Const* find_const(Vector* constPool, char* name);
TypeV_ASM_Global* find_global(Vector* constPool, char* name);



void lexer_init(TypeV_ASM_Lexer* lexer, char* program);
void parser_init(TypeV_ASM_Parser* parser, TypeV_ASM_Lexer* lexer);
void lexer_tokenize(TypeV_ASM_Lexer* lexer);
void parse(TypeV_ASM_Lexer* lexer, TypeV_ASM_Parser* parser);
TypeV_ASM_Program* assemble(TypeV_ASM_Parser* parser);

void debug_program(TypeV_ASM_Program* program);

#endif //TYPE_V_ASSEMBLER_H
