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
        "mv_reg_reg",
        "mv_reg_reg_ptr",
        "mv_reg_null",

        "mv_reg_i",
        "mv_reg_i_ptr",

        "mv_reg_const",
        "mv_reg_const_ptr",

        "mv_global_reg",
        "mv_global_reg_ptr",

        "mv_reg_global",
        "mv_reg_global_ptr",

        "s_alloc",
        "s_alloc_t",
        "s_reg_field",

        "s_loadf",
        "s_loadf_ptr",

        "s_storef_const",
        "s_storef_const_ptr",

        "s_storef_reg",
        "s_storef_reg_ptr",

        "c_alloc",
        "c_alloc_t",
        "c_reg_field",
        "c_storem",
        "c_loadm",

        "c_storef_reg",
        "c_storef_reg_ptr",

        "c_storef_const",
        "c_storef_const_ptr",

        "c_loadf",
        "c_loadf_ptr",
        "i_is_c",
        "i_has_m",

        "a_alloc",
        "a_extend",
        "a_len",
        "a_slice",
        "a_insert_a",

        "a_storef_reg",
        "a_storef_reg_ptr",

        "a_rstoref_reg",
        "a_rstoref_reg_ptr",

        "a_storef_const",
        "a_storef_const_ptr",

        "a_loadf",
        "a_loadf_ptr",

        "a_rloadf",
        "a_rloadf_ptr",

        "push",
        "push_ptr",
        "push_const",
        "pop",
        "pop_ptr",

        "fn_alloc",
        "fn_set_reg",
        "fn_set_reg_ptr",
        "fn_call",
        "fn_calli",
        "fn_ret",
        "fn_get_ret_reg",
        "fn_get_ret_reg_ptr",

        "cast_i8_u8",
        "cast_u8_i8",
        "cast_i16_u16",
        "cast_u16_i16",
        "cast_i32_u32",
        "cast_u32_i32",
        "cast_i64_u64",
        "cast_u64_i64",
        "cast_i32_f32",
        "cast_f32_i32",
        "cast_i64_f64",
        "cast_f64_i64",

        "upcast_i",
        "upcast_u",
        "upcast_f",

        "dcast_i",
        "dcast_u",
        "dcast_f",

        "add_i8",
        "add_u8",
        "add_i16",
        "add_u16",
        "add_i32",
        "add_u32",
        "add_i64",
        "add_u64",
        "add_f32",
        "add_f64",

        "sub_i8",
        "sub_u8",
        "sub_i16",
        "sub_u16",
        "sub_i32",
        "sub_u32",
        "sub_i64",
        "sub_u64",
        "sub_f32",
        "sub_f64",

        "mul_i8",
        "mul_u8",
        "mul_i16",
        "mul_u16",
        "mul_i32",
        "mul_u32",
        "mul_i64",
        "mul_u64",
        "mul_f32",
        "mul_f64",

        "div_i8",
        "div_u8",
        "div_i16",
        "div_u16",
        "div_i32",
        "div_u32",
        "div_i64",
        "div_u64",
        "div_f32",
        "div_f64",

        "mod_i8",
        "mod_u8",
        "mod_i16",
        "mod_u16",
        "mod_i32",
        "mod_u32",
        "mod_f32",
        "mod_i64",
        "mod_u64",
        "mod_f64",

        "lshift_i8",
        "lshift_u8",
        "lshift_i16",
        "lshift_u16",
        "lshift_i32",
        "lshift_u32",
        "lshift_i64",
        "lshift_u64",

        "(rshift_i8)",
        "(rshift_u8)",
        "(rshift_i16)",
        "(rshift_u16)",
        "(rshift_i32)",
        "(rshift_u32)",
        "(rshift_i64)",
        "(rshift_u64)",

        "band_8",
        "band_16",
        "band_32",
        "band_64",

        "bor_8",
        "bor_16",
        "bor_32",
        "bor_64",

        "bxor_8",
        "bxor_16",
        "bxor_32",
        "bxor_64",

        "bnot_8",
        "bnot_16",
        "bnot_32",
        "bnot_64",

        "and",
        "or",
        "not",

        "jmp",
        "j_cmp_u8",
        "j_cmp_i8",
        "j_cmp_u16",
        "j_cmp_i16",
        "j_cmp_u32",
        "j_cmp_i32",
        "j_cmp_u64",
        "j_cmp_i64",
        "j_cmp_f32",
        "j_cmp_f64",
        "j_cmp_ptr",
        "j_cmp_bool",
        "j_cmp_null_8",
        "j_cmp_null_16",
        "j_cmp_null_32",
        "j_cmp_null_64",
        "j_cmp_null_ptr",

        "reg_ffi",
        "call_ffi",
        "close_ffi",

        "debug_reg",
        "halt",

        "load_std",

        "closure_alloc",
        "closure_push_env",
        "closure_push_env_ptr",
        "closure_call",
        "closure_backup",

        "coroutine_alloc",
        "coroutine_fn_alloc",
        "coroutine_get_state",
        "coroutine_call",
        "coroutine_yield",
        "coroutine_ret",
        "coroutine_reset",
        "coroutine_finish",
        "throw_rt",
        "throw_user_rt",
};
#define MAX_INSTRUCTION 267

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
    uint64_t templatePoolSize;
    uint64_t objKeysPoolSize;
    uint64_t codePoolSize;

    uint8_t *constPool;
    uint8_t *globalPool;
    uint8_t *templatePool;
    uint8_t *objKeysPool;
    uint8_t *codePool;
}TypeV_ASM_Program;


TypeV_ASM_Const* create_const(TypeV_ASM_Parser* parser, char* name, TypeV_ASM_Type type, uint64_t value);
TypeV_ASM_Global* create_global(TypeV_ASM_Parser* parser, char* name, uint8_t type, uint64_t value);
TypeV_ASM_Instruction* create_instruction(TypeV_ASM_Parser * parser, TypeV_OpCode opcode, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint8_t num_args);

TypeV_ASM_Const* find_const(Vector* constPool, char* name);
TypeV_ASM_Global* find_global(Vector* constPool, char* name);



void lexer_init(TypeV_ASM_Lexer* lexer, char* program);
void lexer_free(TypeV_ASM_Lexer* lexer);
void parser_init(TypeV_ASM_Parser* parser, TypeV_ASM_Lexer* lexer);
void lexer_tokenize(TypeV_ASM_Lexer* lexer);
void parse(TypeV_ASM_Lexer* lexer, TypeV_ASM_Parser* parser);
void parser_free(TypeV_ASM_Parser* parser);
TypeV_ASM_Program* assemble(TypeV_ASM_Parser* parser);

void debug_program(TypeV_ASM_Program* program);

void free_program(TypeV_ASM_Program* program);

#endif //TYPE_V_ASSEMBLER_H
