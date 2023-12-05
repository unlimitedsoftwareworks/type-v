//
// Created by praisethemoon on 28.11.23.
//

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include "assembler.h"
#include "../utils/log.h"
#include "../vendor/libtable/table.h"


uint8_t bytes_needed(size_t number) {
    uint8_t numBytes = 0;
    while (number > 0) {
        numBytes++;
        number >>= 8;  // Shift the number by 8 bits, essentially dividing by 256
    }
    return numBytes == 0 ? 1 : numBytes; // At least 1 byte is needed for 0
}

void vector_init(Vector* v) {
    v->capacity = 10;
    v->total = 0;
    v->items = malloc(sizeof(void*) * v->capacity);
}

int vector_total(Vector* v) {
    return v->total;
}

static void vector_resize(Vector* v, size_t capacity) {
    void** items = realloc(v->items, sizeof(void*) * capacity);
    if (items) {
        v->items = items;
        v->capacity = capacity;
    }
}

void vector_add(Vector* v, void* item) {
    if (v->capacity == v->total)
        vector_resize(v, v->capacity * 2);
    v->items[v->total++] = item;
}

void* vector_get(Vector* v, size_t index) {
    if (index < v->total)
        return v->items[index];
    return NULL;
}

void vector_set(Vector* v, size_t index, void* item) {
    if (index < v->total)
        v->items[index] = item;
}

void vector_delete(Vector* v, size_t index) {
    if (index < v->total) {
        v->items[index] = NULL;
        for (size_t i = index; i < v->total - 1; i++) {
            v->items[i] = v->items[i + 1];
            v->items[i + 1] = NULL;
        }
        v->total--;

        if (v->total > 0 && v->total == v->capacity / 4)
            vector_resize(v, v->capacity / 2);
    }
}

void vector_free(Vector* v) {
    free(v->items);
}


Token* createToken(TokenType type, char* value, uint64_t line, uint64_t col, uint64_t pos){
    Token* token = malloc(sizeof(Token));
    token->type = type;
    token->value = value;
    token->line = line+1;
    token->col = col;
    token->pos = pos;

    if(type == TOK_FLOAT){
        // check last char
        if(value[strlen(value)-1] == 'f')
            token->isDouble = 0;
        else
            token->isDouble = 1;
    }

    // check if register
    if(type == TOK_IDENTIFIER){
        if(strcmp(value, "VERSION") == 0)
            token->type = TOK_KEYOWRD_VERSION;
        if(strcmp(value, "CONST") == 0)
            token->type = TOK_KEYOWRD_CONST;
        if(strcmp(value, "GLOBAL") == 0)
            token->type = TOK_KEYOWRD_GLOBAL;
        if(strcmp(value, "CODE") == 0)
            token->type = TOK_KEYOWRD_CODE;
        if(strcmp(value, "i8") == 0)
            token->type = TOK_I8;
        if(strcmp(value, "i16") == 0)
            token->type = TOK_I16;
        if(strcmp(value, "i32") == 0)
            token->type = TOK_I32;
        if(strcmp(value, "i64") == 0)
            token->type = TOK_I64;
        if(strcmp(value, "f32") == 0)
            token->type = TOK_F32;
        if(strcmp(value, "f64") == 0)
            token->type = TOK_F64;
        if(strcmp(value, "u8") == 0)
            token->type = TOK_U8;
        if(strcmp(value, "u16") == 0)
            token->type = TOK_U16;
        if(strcmp(value, "u32") == 0)
            token->type = TOK_U32;
        if(strcmp(value, "u64") == 0)
            token->type = TOK_U64;
        if(strcmp(value, "ptr") == 0)
            token->type = TOK_PTR;
        if(strcmp(value, "r0") == 0)
            token->type = TOK_R0;
        if(strcmp(value, "r1") == 0)
            token->type = TOK_R1;
        if(strcmp(value, "r2") == 0)
            token->type = TOK_R2;
        if(strcmp(value, "r3") == 0)
            token->type = TOK_R3;
        if(strcmp(value, "r4") == 0)
            token->type = TOK_R4;
        if(strcmp(value, "r5") == 0)
            token->type = TOK_R5;
        if(strcmp(value, "r6") == 0)
            token->type = TOK_R6;
        if(strcmp(value, "r7") == 0)
            token->type = TOK_R7;
        if(strcmp(value, "r8") == 0)
            token->type = TOK_R8;
        if(strcmp(value, "r9") == 0)
            token->type = TOK_R9;
        if(strcmp(value, "r10") == 0)
            token->type = TOK_R10;
        if(strcmp(value, "r11") == 0)
            token->type = TOK_R11;
        if(strcmp(value, "r12") == 0)
            token->type = TOK_R12;
        if(strcmp(value, "r13") == 0)
            token->type = TOK_R13;
        if(strcmp(value, "r14") == 0)
            token->type = TOK_R14;
        if(strcmp(value, "r15") == 0)
            token->type = TOK_R15;
        if(strcmp(value, "r16") == 0)
            token->type = TOK_R16;
        if(strcmp(value, "r17") == 0)
            token->type = TOK_R17;
        if(strcmp(value, "r18") == 0)
            token->type = TOK_R18;
        if(strcmp(value, "r19") == 0)
            token->type = TOK_R19;
        if(strcmp(value, "r20") == 0)
            token->type = TOK_R20;

        for(int i = 0; i < MAX_INSTRUCTION; i++){
            if(strcmp(value, instructions[i]) == 0){
                token->type = TOK_INSTRUCTION;
                token->instructionId = i;
                break;
            }
        }
    }


    return token;
}

void lexer_init(TypeV_ASM_Lexer* lexer, char* program){
    lexer->line = 0;
    lexer->col = 0;
    lexer->pos = 0;
    lexer->program = program;
    lexer->program_length = strlen(program);

    vector_init(&lexer->tokens);
}

uint8_t issymbol(char c){
    return (c == '|') || (c == ':') || (c == ',') || (c == '[') || (c == ']' || (c == '(') || (c == ')') ||
    c== '{' || c == '}' || c == '+' || c == '-' || c == '*' || c == '/'
    || c == '%' || c == '^' || c == '&' || c == '!' || c == '~'
    || c == '<' || c == '>' || c == '=' || c == '?') || c == '@' || c == '$' || c == '#';
}

void parser_init(TypeV_ASM_Parser* parser, TypeV_ASM_Lexer* lexer) {
    parser->lexer = lexer;
    parser->currentToken = NULL;
    parser->currentTokenIndex = 0;

    parser->constPoolSize = 0;
    parser->globalPoolSize = 0;
    parser->codePoolSize = 0;


    // init vectors
    vector_init(&parser->constPool);
    vector_init(&parser->globalPool);
    vector_init(&parser->codePool);
    vector_init(&parser->labels);
}

void lexer_tokenize(TypeV_ASM_Lexer* lexer){
    while(lexer->pos < lexer->program_length) {
        char c = lexer->program[lexer->pos];
        if (c == '\n') {
            lexer->line++;
            lexer->col = 0;
            lexer->pos++;
            continue;
        }
        if (c == ' ' || c == '\t') {
            lexer->col++;
            lexer->pos++;
            continue;
        }
        if (c == ';') {
            lexer->col++;
            lexer->pos++;
            while (lexer->pos < lexer->program_length && lexer->program[lexer->pos] != '\n') {
                lexer->col++;
                lexer->pos++;
            }
            continue;
        }
        if(c == '\"'){
            // parse string
            uint64_t start = lexer->pos+1;
            lexer->col++;
            lexer->pos++;
            while (lexer->pos < lexer->program_length && lexer->program[lexer->pos] != '\"') {
                lexer->col++;
                lexer->pos++;
            }
            // add the last "
            lexer->col++;
            lexer->pos++;

            uint64_t end = lexer->pos-1;
            char *value = malloc(end - start + 1);
            memcpy(value, lexer->program + start, end - start);
            value[end - start] = '\0';
            vector_add(&lexer->tokens, createToken(TOK_STRING, value, lexer->line, lexer->col, lexer->pos));

        }
        if (isdigit(c)) {
            uint64_t start = lexer->pos;
            int dotFound = 0;
            int isHex = 0;
            while (lexer->pos < lexer->program_length && isdigit(lexer->program[lexer->pos])) {
                lexer->col++;
                lexer->pos++;
            }
            // check if we have a dot
            if (lexer->pos < lexer->program_length && lexer->program[lexer->pos] == '.') {
                dotFound = 1;
                lexer->col++;
                lexer->pos++;
                while (lexer->pos < lexer->program_length && isdigit(lexer->program[lexer->pos])) {
                    lexer->col++;
                    lexer->pos++;
                }
            }
            // check if we have a hex
            else if ((lexer->program[lexer->pos] == 'x') && (start == lexer->pos-1)) {
                isHex = 1;
                lexer->col++;
                lexer->pos++;
                while (lexer->pos < lexer->program_length && isxdigit(lexer->program[lexer->pos])) {
                    lexer->col++;
                    lexer->pos++;
                }
            }
            uint64_t end = lexer->pos;
            char *value = malloc(end - start + 1);
            memcpy(value, lexer->program + start, end - start);
            value[end - start] = '\0';
            vector_add(&lexer->tokens, createToken(TOK_NUMBER, value, lexer->line, lexer->col, lexer->pos));
            continue;
        }
        if (isalpha(c)) {
            uint64_t start = lexer->pos;
            while (lexer->pos < lexer->program_length &&
                   (isalpha(lexer->program[lexer->pos]) || isdigit(lexer->program[lexer->pos]) ||
                    lexer->program[lexer->pos] == '_')) {
                lexer->col++;
                lexer->pos++;
            }
            uint64_t end = lexer->pos;
            char *value = malloc(end - start + 1);
            memcpy(value, lexer->program + start, end - start);
            value[end - start] = '\0';
            vector_add(&lexer->tokens, createToken(TOK_IDENTIFIER, value, lexer->line, lexer->col, lexer->pos));
            continue;
        }
        if (issymbol(c)) {
            uint64_t start = lexer->pos;
            while (lexer->pos < lexer->program_length && issymbol(lexer->program[lexer->pos])) {
                lexer->col++;
                lexer->pos++;
            }
            uint64_t end = lexer->pos;
            char *value = malloc(end - start + 1);
            memcpy(value, lexer->program + start, end - start);
            value[end - start] = '\0';

            // check if : or , or [ or ]
            if (strcmp(value, ":") == 0)
                vector_add(&lexer->tokens, createToken(TOK_COLON, value, lexer->line, lexer->col, lexer->pos));
            else if (strcmp(value, ",") == 0)
                vector_add(&lexer->tokens, createToken(TOK_COMMA, value, lexer->line, lexer->col, lexer->pos));
            else if (strcmp(value, "[") == 0)
                vector_add(&lexer->tokens, createToken(TOK_LBRACKET, value, lexer->line, lexer->col, lexer->pos));
            else if (strcmp(value, "]") == 0)
                vector_add(&lexer->tokens, createToken(TOK_RBRACKET, value, lexer->line, lexer->col, lexer->pos));
            else if (strcmp(value, "=") == 0)
                vector_add(&lexer->tokens, createToken(TOK_EQ, value, lexer->line, lexer->col, lexer->pos));
            else if (strcmp(value, ";") == 0)
                vector_add(&lexer->tokens, createToken(TOK_SEMICOLON, value, lexer->line, lexer->col, lexer->pos));
            else if (strcmp(value, "@") == 0)
                vector_add(&lexer->tokens, createToken(TOK_AT, value, lexer->line, lexer->col, lexer->pos));
            else {
                LOG_ERROR("Unknown symbol: %s", value);
            }
        }
        // check if we have to return EOF
        if (lexer->pos >= lexer->program_length) {
            vector_add(&lexer->tokens, createToken(TOK_EOF, NULL, lexer->line, lexer->col, lexer->pos));
            return;
        }
    }
}

Token* next_token(TypeV_ASM_Parser* parser){
    parser->currentToken = vector_get(&parser->lexer->tokens, parser->currentTokenIndex);
    parser->currentTokenIndex++;
    return parser->currentToken;
}

void fail(Token * tok, char* msg){
    LOG_ERROR("%s, input: `%s`, line: %zu, col: %zu, pos: %zu", msg, tok->value, tok->line, tok->col, tok->pos);
    assert(0);
}

void assert_tok(Token* tok, TokenType type){
    if(tok->type != type){
        LOG_ERROR("Expected token: %d, got: %d, line: %zu, col: %zu, pos: %zu", type, tok->type, tok->line, tok->col, tok->pos);
        assert(0);
    }
}


TypeV_ASM_Const* find_const(Vector* constPool, char* name) {
    for (int i = 0; i < vector_total(constPool); i++) {
        TypeV_ASM_Const *c = vector_get(constPool, i);
        if (strcmp(c->name, name) == 0)
            return c;
    }
    return NULL;
}

TypeV_ASM_Global * find_global(Vector* globalPool, char* name) {
    for (int i = 0; i < vector_total(globalPool); i++) {
        TypeV_ASM_Global *c = vector_get(globalPool, i);
        if (strcmp(c->name, name) == 0)
            return c;
    }
    return NULL;
}

uint8_t type_size(TypeV_ASM_Type type){
    switch(type){
        case ASM_I8:
        case ASM_U8:
            return 1;
        case ASM_I16:
        case ASM_U16:
            return 2;
        case ASM_I32:
        case ASM_U32:
        case ASM_F32:
            return 4;
        case ASM_I64:
        case ASM_U64:
        case ASM_F64:
        case ASM_PTR:
            return 8;

        default:
            LOG_ERROR("Unknown type: %d", type);
            assert(0);
    }
}

TypeV_ASM_Const* create_const(TypeV_ASM_Parser* parser, char* name, TypeV_ASM_Type type, uint64_t value) {
    TypeV_ASM_Const* c = malloc(sizeof(TypeV_ASM_Const));
    c->name = name;
    c->type = type;
    c->offset = parser->constPoolSize;
    c->offsetSize = bytes_needed(c->offset);
    memcpy(&c->value, &value, type_size(type));
    parser->constPoolSize += type_size(type);
    LOG_INFO("Creating const: %s, type: %d, size: %d, offset: %d, value: %d", c->name, c->type, type_size(type), c->offset, c->value);
    vector_add(&parser->constPool, c);
    return c;
}

TypeV_ASM_Global* create_global(TypeV_ASM_Parser* parser, char* name, uint8_t type, uint64_t value) {
    TypeV_ASM_Global* g = malloc(sizeof(TypeV_ASM_Global));
    g->name = name;
    g->type = type;
    g->offset = parser->globalPoolSize;
    g->offsetSize = bytes_needed(g->offset);
    memcpy(&g->value, &value, type_size(type));
    parser->globalPoolSize += type_size(type);
    LOG_INFO("Creating global: %s, type: %d, size: %d, offset: %d, value: %d", g->name, g->type, type_size(type), g->offset, g->value);
    vector_add(&parser->globalPool, g);
    return g;
}

TypeV_ASM_Instruction* create_instruction(TypeV_ASM_Parser * parser, TypeV_OpCode opcode, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint8_t num_args){
    TypeV_ASM_Instruction* i = malloc(sizeof(TypeV_ASM_Instruction));
    i->offset = parser->codePoolSize++;
    i->opcode = opcode;
    i->num_args = num_args;
    i->arg1 = arg1;
    i->arg2 = arg2;
    i->arg3 = arg3;

    LOG_INFO("Creating instruction: %s at %d, arg1: %d, arg2: %d, arg3: %d", instructions[i->opcode], i->offset, i->arg1, i->arg2, i->arg3);
    // add to code pool
    vector_add(&parser->codePool, i);
    return i;
}

int getInstructionIndex(char* instruction){
    // find index in instructions arrau
    for(int i = 0; i < MAX_INSTRUCTION; i++){
        if(strcmp(instruction, instructions[i]) == 0)
            return i;
    }

    return -1;
}

TypeV_ASM_Reg getRegister(TypeV_ASM_Parser* parser){
    Token* tok = next_token(parser);
    if(tok->type >= TOK_R0 && tok->type <= TOK_R20)
        return tok->type - TOK_R0;
    else {
        fail(tok, "Expected register");
    }
}

TypeV_ASM_Const* getConst(TypeV_ASM_Parser* parser){
    Token* tok = next_token(parser);
    if(tok->type == TOK_IDENTIFIER) {
        TypeV_ASM_Const* c = find_const(&parser->constPool, tok->value);
        if(c == NULL){
            fail(tok, "Const not found");
        }

        return c;
    }
    else {
        fail(tok, "Expected const");
    }
}

TypeV_ASM_Global * getGlobal(TypeV_ASM_Parser* parser){
    Token* tok = next_token(parser);
    if(tok->type == TOK_IDENTIFIER){
        TypeV_ASM_Global*g = find_global(&parser->globalPool, tok->value);
        if(g == NULL){
            fail(tok, "Global not found");
        }
        return g;
    }

    else {
        fail(tok, "Expected global");
    }
}

uint8_t getShortNumber(TypeV_ASM_Parser* parser){
    Token* tok = next_token(parser);
    if(tok->type == TOK_NUMBER)
        return (uint8_t)atoi(tok->value);
    else
        fail(tok, "Expected number");
}

size_t getLongNumber(TypeV_ASM_Parser* parser){
    Token* tok = next_token(parser);
    if(tok->type == TOK_NUMBER)
        return (size_t)atoll(tok->value);
    else
        fail(tok, "Expected number");
}

u_int8_t getByteSize(TypeV_ASM_Parser* parser){
    Token* tok = next_token(parser);
    if(tok->type == TOK_NUMBER)
        return atoi(tok->value);
    else if (tok->type == TOK_PTR)
        return 0;
    else
        fail(tok, "Expected byte size (ptr, 1, 2, 4, 8");
}

TypeV_Label* find_label(TypeV_ASM_Parser* parser, char* name){
    for(int i = 0; i < vector_total(&parser->labels); i++){
        TypeV_Label* label = vector_get(&parser->labels, i);
        if(strcmp(label->name, name) == 0)
            return label;
    }
    return NULL;
}

/**
 * Reads a label name from the parser
 * @param parser
 * @return
 */
TypeV_Label* getLabel(TypeV_ASM_Parser* parser){
    Token* tok = next_token(parser);
    if(tok->type == TOK_IDENTIFIER){
        TypeV_Label* label = find_label(parser, tok->value);

        if(label == NULL){
            label = malloc(sizeof(TypeV_Label));
            label->name = tok->value;
            label->codeOffset = -1;
        }
        return label;
    }
    else {
        fail(tok, "Expected label");
    }
}


void parse(TypeV_ASM_Lexer* lexer, TypeV_ASM_Parser* parser){
    lexer_tokenize(lexer);
    // print all tokens
    for(int i = 0; i < vector_total(&lexer->tokens); i++){
        Token* token = vector_get(&lexer->tokens, i);

        LOG_INFO("Token: %d, value: %s", token->type, token->value);
    }

    // step 1 version
    Token* tok = next_token(parser);
    assert_tok(tok, TOK_KEYOWRD_VERSION);
    tok = next_token(parser);
    assert_tok(tok, TOK_COLON);
    tok = next_token(parser);
    assert_tok(tok, TOK_NUMBER);
    parser->version = atoi(tok->value);
    tok = next_token(parser);
    assert_tok(tok, TOK_KEYOWRD_CONST);
    tok = next_token(parser);
    assert_tok(tok, TOK_COLON);
    tok = next_token(parser);

    if(tok->type != TOK_KEYOWRD_GLOBAL){
        // const pool

        while((tok->type != TOK_KEYOWRD_GLOBAL) && (tok->type != EOF)){
            Token* identifier = tok;
            assert_tok(identifier, TOK_IDENTIFIER);
            assert_tok(next_token(parser), TOK_COLON);
            Token* type = next_token(parser);
            if((type->type >= TOK_I8) && (type->type <= TOK_PTR)) {
                // type
                tok = next_token(parser);
                uint64_t v = 0;
                if((tok != NULL) && (tok->type == TOK_EQ)){
                    // value
                    Token *value = next_token(parser);
                    if(value->type == TOK_STRING){
                        // we defined it as multiple constants
                        for(int i = 0; i <= strlen(value->value); i++){
                            create_const(parser, identifier->value, type->type - TOK_I8, (uint8_t)value->value[i]);
                        }
                        tok = next_token(parser);
                        continue;
                    }
                    assert_tok(value, TOK_NUMBER);
                    if((strlen(value->value) > 2) && (value->value[0] == '0' )&& (value->value[1] == 'x')){
                        // hex
                        v = strtoull(value->value, NULL, 16);
                    }
                    else {
                        v = atoll(value->value);
                    }
                    tok = next_token(parser);
                }
                create_const(parser, identifier->value, type->type - TOK_I8, v);

            }
            else {
                fail(type, "Unknown data type");
            }

        }

    }
    assert_tok(tok, TOK_KEYOWRD_GLOBAL);
    tok = next_token(parser);
    assert_tok(tok, TOK_COLON);
    tok = next_token(parser);
    if(tok->type != TOK_KEYOWRD_CODE){
        // global pool

        while((tok->type != TOK_KEYOWRD_CODE) && (tok->type != EOF)){
            Token* identifier = tok;
            assert_tok(identifier, TOK_IDENTIFIER);
            assert_tok(next_token(parser), TOK_COLON);
            Token* type = next_token(parser);
            if((type->type >= TOK_I8) && (type->type <= TOK_PTR)) {
                // type
                tok = next_token(parser);
                uint64_t v = 0;
                if((tok != NULL) && (tok->type == TOK_EQ)){
                    // value
                    Token *value = next_token(parser);
                    assert_tok(value, TOK_NUMBER);
                    v = atoll(value->value);
                    tok = next_token(parser);
                }
                create_global(parser, identifier->value, type->type - TOK_I8, v);
            }
            else {
                fail(type, "Unknown data type");
            }

        }

    }
    assert_tok(tok, TOK_KEYOWRD_CODE);
    tok = next_token(parser);
    assert_tok(tok, TOK_COLON);

    tok = next_token(parser);
    while((tok != NULL) && (tok->type != EOF)) {
        TypeV_OpCode idx = getInstructionIndex(tok->value);
        if (idx == -1) {
            if (tok->type == TOK_IDENTIFIER) {
                Token *next = next_token(parser);
                if ((next != NULL) && (next->type == TOK_COLON)) {
                    // label
                    TypeV_Label *label = find_label(parser, tok->value);
                    if (label == NULL) {
                        label = malloc(sizeof(TypeV_Label));
                        label->name = tok->value;
                        label->primaryCodeOffset = parser->codePoolSize;
                        LOG_INFO("Creating label: %s, offset: %d", label->name, label->primaryCodeOffset);
                        vector_add(&parser->labels, label);
                        tok = next_token(parser);
                        continue;
                    } else {
                        // fail, we have a duplicate label
                        fail(tok, "Duplicate label");
                    }
                } else {
                    fail(next, "Expected colon");
                }
            }
            fail(tok, "Unknown instruction");
        }
        switch (idx) {
            case OP_MV_REG_REG: {
                // mv reg, reg, size
                TypeV_ASM_Reg reg1 = getRegister(parser);
                TypeV_ASM_Reg reg2 = getRegister(parser);
                uint8_t size = getByteSize(parser);

                create_instruction(parser, OP_MV_REG_REG, reg1, reg2, size, 3);
                parser->codePoolSize += 3;
                break;
            }

            case OP_MV_REG_I: {
                // mv reg, i, size
                TypeV_ASM_Reg reg = getRegister(parser);
                uint64_t value = getLongNumber(parser);
                uint8_t size = bytes_needed(value);

                create_instruction(parser, OP_MV_REG_I, reg, size, value, 3);
                parser->codePoolSize += 2 + size;
                break;
            }
            case OP_MV_REG_CONST_8:
            case OP_MV_REG_CONST_16:
            case OP_MV_REG_CONST_32:
            case OP_MV_REG_CONST_64:
            case OP_MV_REG_CONST_PTR: {
                TypeV_ASM_Reg reg = getRegister(parser);
                TypeV_ASM_Const *c = getConst(parser);

                create_instruction(parser, idx, reg, c->offsetSize, c->offset, 3);
                parser->codePoolSize += 2 + c->offsetSize;
                break;
            }
            case OP_MV_REG_MEM: {
                // OP_MV_MEM_REG dest: R, src: Rm, bytes: S
                TypeV_ASM_Reg dest = getRegister(parser);
                TypeV_ASM_Reg src = getRegister(parser);
                uint8_t bytes = getByteSize(parser);

                create_instruction(parser, OP_MV_REG_MEM, dest, src, bytes, 3);
                parser->codePoolSize += 3;
                break;
            }
            case OP_MV_MEM_REG: {
                // OP_MV_MEM_REG dest: Rm, src: R, bytes: S
                TypeV_ASM_Reg dest = getRegister(parser);
                TypeV_ASM_Reg src = getRegister(parser);
                uint8_t bytes = getByteSize(parser);

                create_instruction(parser, OP_MV_MEM_REG, dest, src, bytes, 3);
                parser->codePoolSize += 3;
                break;
            }
            case OP_MV_REG_LOCAL_8:
            case OP_MV_REG_LOCAL_16:
            case OP_MV_REG_LOCAL_32:
            case OP_MV_REG_LOCAL_64:
            case OP_MV_REG_LOCAL_PTR: {
                // OP_MV_REG_LOCAL_PTR dest: R, offset: I
                TypeV_ASM_Reg dest = getRegister(parser);
                size_t offset = getLongNumber(parser);
                uint8_t offsetSize = bytes_needed(offset);

                create_instruction(parser, idx, dest, offsetSize, offset, 3);
                parser->codePoolSize += 2 + offsetSize;
                break;
            }
            case OP_MV_LOCAL_REG_8:
            case OP_MV_LOCAL_REG_16:
            case OP_MV_LOCAL_REG_32:
            case OP_MV_LOCAL_REG_64:
            case OP_MV_LOCAL_REG_PTR: {
                // OP_MV_LOCAL_REG_PTR offset-size: Z, offset: I, src: R
                size_t offset = getLongNumber(parser);
                uint8_t offsetSize = bytes_needed(offset);
                TypeV_ASM_Reg src = getRegister(parser);

                create_instruction(parser, idx, offsetSize, offset, src, 3);
                parser->codePoolSize += 2 + offsetSize;
                break;
            }
            case OP_MV_GLOBAL_REG_8:
            case OP_MV_GLOBAL_REG_16:
            case OP_MV_GLOBAL_REG_32:
            case OP_MV_GLOBAL_REG_64:
            case OP_MV_GLOBAL_REG_PTR: {
                // OP_MV_GLOBAL_REG_PTR offset-size: Z, offset: I, src: R
                TypeV_ASM_Global *g = getGlobal(parser);
                uint8_t offsetSize = g->offsetSize;
                TypeV_ASM_Reg src = getRegister(parser);

                create_instruction(parser, idx, offsetSize, g->offset, src, 3);
                parser->codePoolSize += 2 + offsetSize;
                break;
            }
            case OP_MV_REG_GLOBAL_8:
            case OP_MV_REG_GLOBAL_16:
            case OP_MV_REG_GLOBAL_32:
            case OP_MV_REG_GLOBAL_64:
            case OP_MV_REG_GLOBAL_PTR: {
                // OP_MV_REG_GLOBAL_PTR dest: R, offset-size: Z, offset: I
                TypeV_ASM_Reg dest = getRegister(parser);
                TypeV_ASM_Global *g = getGlobal(parser);
                uint8_t offsetSize = g->offsetSize;

                create_instruction(parser, idx, dest, offsetSize, g->offset, 3);
                parser->codePoolSize += 2 + offsetSize;
                break;
            }
            case OP_MV_REG_ARG_8:
            case OP_MV_REG_ARG_16:
            case OP_MV_REG_ARG_32:
            case OP_MV_REG_ARG_64:
            case OP_MV_REG_ARG_PTR: {
                // OP_MV_REG_ARG_PTR dest: R, offset-size: Z, offset: I
                TypeV_ASM_Reg dest = getRegister(parser);
                size_t offset = getLongNumber(parser);
                uint8_t offsetSize = bytes_needed(offset);

                create_instruction(parser, idx, dest, offsetSize, offset, 3);
                parser->codePoolSize += 2 + offsetSize;
                break;
            }

            case OP_MV_ARG_REG_8:
            case OP_MV_ARG_REG_16:
            case OP_MV_ARG_REG_32:
            case OP_MV_ARG_REG_64:
            case OP_MV_ARG_REG_PTR: {
                // OP_MV_ARG_REG_PTR offset-size: Z, offset: I, src: R
                size_t offset = getLongNumber(parser);
                uint8_t offsetSize = bytes_needed(offset);
                TypeV_ASM_Reg src = getRegister(parser);

                create_instruction(parser, idx, offsetSize, offset, src, 3);
                parser->codePoolSize += 2 + offsetSize;
                break;
            }
            case OP_S_ALLOC: {
                // OP_S_ALLOC fieldOffsets-count: I, struct-size-size: Z, struct-size: I
                uint8_t fieldOffsetsCount = getShortNumber(parser);
                uint8_t structSize = getLongNumber(parser);
                uint8_t structSizeSize = bytes_needed(structSize);

                create_instruction(parser, OP_S_ALLOC, fieldOffsetsCount, structSizeSize, structSize, 3);
                parser->codePoolSize += 2 + structSizeSize;
                break;
            }
            case OP_S_ALLOC_SHADOW: {
                // OP_S_ALLOC_SHADOW fieldOffsets-count: I, struct-size-size: Z, struct-size: I
                uint8_t fieldOffsetsCount = getShortNumber(parser);

                create_instruction(parser, OP_S_ALLOC_SHADOW, fieldOffsetsCount, 0, 0, 1);
                parser->codePoolSize += 1;
                break;
            }
            case OP_S_SET_OFFSET: {
                // OP_S_SET_OFFSET field-index: I, offset-size: Z, offset: I
                uint8_t fieldIndex = getShortNumber(parser);
                size_t fieldOffset = getLongNumber(parser);
                uint8_t fieldOffsetSize = bytes_needed(fieldOffset);

                create_instruction(parser, OP_S_SET_OFFSET, fieldIndex, fieldOffsetSize, fieldOffset, 3);
                parser->codePoolSize += 2 + fieldOffsetSize;
                break;
            }
            case OP_S_LOADF: {
                // OP_S_LOADF_8 dest: R, field-index: I
                TypeV_ASM_Reg dest = getRegister(parser);
                uint8_t fieldIndex = getShortNumber(parser);
                uint8_t byteSize = getByteSize(parser);

                create_instruction(parser, OP_S_LOADF, dest, fieldIndex, byteSize, 3);
                parser->codePoolSize += 3;
                break;
            }
            // OP_S_STOREF_CONST_[size] fieldIndex: I, offset-size : Z, offset-: I
            case OP_S_STOREF_CONST_8:
            case OP_S_STOREF_CONST_16:
            case OP_S_STOREF_CONST_32:
            case OP_S_STOREF_CONST_64:
            case OP_S_STOREF_CONST_PTR: {
                // OP_S_STOREF_CONST_PTR field-index: I, offset-size: Z, offset: I
                uint8_t fieldIndex = getShortNumber(parser);
                TypeV_ASM_Const *c = getConst(parser);
                uint8_t offsetSize = c->offsetSize;
                size_t offset = c->offset;
                create_instruction(parser, idx, fieldIndex, offsetSize, offset, 3);
                parser->codePoolSize += 2 + offsetSize;
                break;
            }
            // OP_S_STOREF_REG fieldIndex: I, source: R, bytesize: I
            case OP_S_STOREF_REG: {
                // OP_S_STOREF_REG_8 field-index: I, source: R, size: S
                uint8_t fieldIndex = getShortNumber(parser);
                TypeV_ASM_Reg source = getRegister(parser);
                uint8_t byteSize = getByteSize(parser);

                create_instruction(parser, OP_S_STOREF_REG, fieldIndex, source, byteSize, 3);
                parser->codePoolSize += 3;
                break;
            }
            // OP_C_ALLOCF fields-count: I, size: BigOffset
            case OP_C_ALLOCF: {
                // OP_C_ALLOCF fields-count: I, size: BigOffset
                uint8_t fieldsCount = getShortNumber(parser);
                size_t size = getLongNumber(parser);
                uint8_t sizeSize = bytes_needed(size);

                create_instruction(parser, OP_C_ALLOCF, fieldsCount, sizeSize, size, 3);
                parser->codePoolSize += 2 + sizeSize;
                break;
            }
            // OP_C_ALLOCM num_methods: Short number
            case OP_C_ALLOCM: {
                // OP_C_ALLOCM num_methods: Short number
                uint8_t numMethods = getShortNumber(parser);

                create_instruction(parser, OP_C_ALLOCM, numMethods, 0, 0, 1);
                parser->codePoolSize += 1;
                break;
            }

            case OP_C_STOREM: /**! Uses label */
            {
                // OP_C_STOREM method-index: Short number, method-address
                uint8_t methodIndex = getShortNumber(parser);
                TypeV_Label *label = getLabel(parser); // labels are stored as 8 bytes for now

                // 3 args bc labels will be decomposed into offset-length and offset
                create_instruction(parser, OP_C_STOREM, methodIndex, (size_t) label, 0, 3);
                parser->codePoolSize += 2 + 8;
                break;
            }
            case OP_C_LOADM: // OP_C_LOADM dest: R, methodIndex: I
            {
                // OP_C_LOADM dest: R,, methodIndex: I
                TypeV_ASM_Reg dest = getRegister(parser);
                uint8_t methodIndex = getShortNumber(parser);

                create_instruction(parser, OP_C_LOADM, dest, methodIndex, 0, 2);
                parser->codePoolSize += 2;
                break;
            }
            case OP_C_STOREF_REG: // OP_CSTOREF_[size] fieldIndex: I, R: source register, size: S
            {
                // OP_C_STOREF_8 fieldIndex: I, R: source register, size: S
                uint8_t fieldIndex = getShortNumber(parser);
                TypeV_ASM_Reg source = getRegister(parser);
                uint8_t byteSize = getByteSize(parser);

                create_instruction(parser, OP_C_STOREF_REG, fieldIndex, source, byteSize, 3);
                parser->codePoolSize += 3;
                break;
            }
            case OP_C_STOREF_CONST_8:
            case OP_C_STOREF_CONST_16:
            case OP_C_STOREF_CONST_32:
            case OP_C_STOREF_CONST_64:
            case OP_C_STOREF_CONST_PTR: // OP_C_STOREF_CONST_PTR fieldIndex: I, offset-size: Z, offset: I
            {
                // OP_C_STOREF_CONST_PTR fieldIndex: I, offset-size: Z, offset: I
                uint8_t fieldIndex = getShortNumber(parser);
                TypeV_ASM_Const *c = getConst(parser);
                uint8_t offsetSize = c->offsetSize;
                size_t offset = c->offset;

                create_instruction(parser, OP_C_STOREF_CONST_PTR, fieldIndex, offsetSize, offset, 3);
                parser->codePoolSize += 2 + offsetSize;
                break;
            }
            case OP_C_LOADF: {
                TypeV_ASM_Reg dest = getRegister(parser);
                uint8_t fieldIndex = getShortNumber(parser);
                uint8_t byteSize = getByteSize(parser);

                create_instruction(parser, OP_C_LOADF, dest, fieldIndex, byteSize, 3);
                parser->codePoolSize += 3;
                break;
            }

            case OP_I_ALLOC: // OP_I_ALLOC num-methods: I
            {
                // OP_I_ALLOC num-methods: I
                uint8_t numMethods = getShortNumber(parser);

                create_instruction(parser, OP_I_ALLOC, numMethods, 0, 0, 1);
                parser->codePoolSize += 1;
                break;
            }
            case OP_I_SET_OFFSET: // OP_I_SET_OFFSET method-index: I, offset-size: Z, offset: I
            {
                // OP_I_SET_OFFSET method-index: I, offset-size: Z, offset: I
                uint8_t methodIndex = getShortNumber(parser);
                uint64_t offset = getLongNumber(parser);
                uint8_t offsetSize = bytes_needed(offset);

                create_instruction(parser, OP_I_SET_OFFSET, methodIndex, offsetSize, offset, 3);
                parser->codePoolSize += 2 + offsetSize;
                break;
            }

            case OP_I_LOADM: {
                // OP_I_LOADM dest: R, method-index: I
                TypeV_ASM_Reg dest = getRegister(parser);
                uint8_t methodIndex = getShortNumber(parser);

                create_instruction(parser, OP_I_LOADM, dest, methodIndex, 0, 2);
                parser->codePoolSize += 2;
                break;
            }
            case OP_PUSH: {
                // OP_PUSH reg: R, size: S
                TypeV_ASM_Reg reg = getRegister(parser);
                uint8_t byteSize = getByteSize(parser);

                create_instruction(parser, OP_PUSH, reg, byteSize, 0, 2);
                parser->codePoolSize += 2;
                break;
            }
            case OP_PUSH_CONST: {
                // OP_PUSH_CONST value: I, bytes: S

                TypeV_ASM_Const *c = getConst(parser);
                uint8_t byteSize = getByteSize(parser);

                create_instruction(parser, OP_PUSH_CONST, c->offsetSize, c->offset, byteSize, 3);
                parser->codePoolSize += 2 + c->offsetSize;
                break;
            }
            case OP_POP: {
                // OP_POP reg: R
                TypeV_ASM_Reg reg = getRegister(parser);
                uint8_t byteSize = getByteSize(parser);

                create_instruction(parser, OP_POP, reg, byteSize, 0, 2);
                parser->codePoolSize += 2;
                break;
            }
            case OP_FRAME_INIT_ARGS:
            case OP_FRAME_INIT_LOCALS: {
                // OP_FRAME_INIT_LOCALS size-length: Z, size: I
                size_t size = getLongNumber(parser);
                uint8_t sizeSize = bytes_needed(size);

                create_instruction(parser, idx, sizeSize, size, 0, 2);
                parser->codePoolSize += 1 + sizeSize;
                break;
            }
            case OP_FRAME_RM:
            case OP_FRAME_PRECALL:
            case OP_FN_MAIN:
            case OP_FN_RET: {
                create_instruction(parser, idx, 0, 0, 0, 0);
                break;
            }
            case OP_FN_CALL: {
                TypeV_ASM_Reg reg = getRegister(parser);

                create_instruction(parser, OP_FN_CALL, reg, 0, 0, 0);
                parser->codePoolSize++;
                break;
            }
            case OP_FN_CALLI: {
                // get the label
                TypeV_Label *label = getLabel(parser);

                create_instruction(parser, OP_FN_CALLI, (size_t) label, 0, 0, 2);
                parser->codePoolSize += 1 + 8;
                break;
            }
            case OP_A_ALLOC: {
                // OP_A_ALLOC num_elements_size: Z, num_elements: I, element_size: Z
                size_t numElements = getLongNumber(parser);
                uint8_t numElementsSize = bytes_needed(numElements);
                uint8_t elementSize = getByteSize(parser);

                create_instruction(parser, OP_A_ALLOC, numElementsSize, numElements, elementSize, 3);
                parser->codePoolSize += 2 + numElementsSize;
                break;
            }
            case OP_A_EXTEND: {
                // OP_A_EXTEND num_elements_size: Z, num_elements: I
                size_t numElements = getLongNumber(parser);
                uint8_t numElementsSize = bytes_needed(numElements);

                create_instruction(parser, OP_A_EXTEND, numElementsSize, numElements, 0, 2);
                parser->codePoolSize += 1 + numElementsSize;
                break;
            }
            case OP_A_STOREF_REG: {
                // OP_A_STOREF_REG index: R, source: R, size: S
                TypeV_ASM_Reg index = getRegister(parser);
                TypeV_ASM_Reg source = getRegister(parser);
                uint8_t byteSize = getByteSize(parser);

                create_instruction(parser, OP_A_STOREF_REG, index, source, byteSize, 3);
                parser->codePoolSize += 3;
                break;
            }
            case OP_A_STOREF_CONST_8:
            case OP_A_STOREF_CONST_16:
            case OP_A_STOREF_CONST_32:
            case OP_A_STOREF_CONST_64:
            case OP_A_STOREF_CONST_PTR: {
                // OP_A_STOREF_CONST_PTR index: R, offset-size : Z, offset-: I
                TypeV_ASM_Reg index = getRegister(parser);
                TypeV_ASM_Const *c = getConst(parser);
                uint8_t offsetSize = c->offsetSize;

                create_instruction(parser, idx, index, offsetSize, c->offset, 3);
                parser->codePoolSize += 2 + offsetSize;
                break;
            }
            case OP_A_LOADF: {
                // OP_A_LOADF dest: R, index: R, size: S
                TypeV_ASM_Reg dest = getRegister(parser);
                TypeV_ASM_Reg index = getRegister(parser);
                uint8_t byteSize = getByteSize(parser);

                create_instruction(parser, OP_A_LOADF, dest, index, byteSize, 3);
                parser->codePoolSize += 3;
                break;
            }
            case OP_CAST_I8_U8:
            case OP_CAST_U8_I8:
            case OP_CAST_I16_U16:
            case OP_CAST_U16_I16:
            case OP_CAST_I32_U32:
            case OP_CAST_U32_I32:
            case OP_CAST_I64_U64:
            case OP_CAST_U64_I64:
            case OP_CAST_I32_F32:
            case OP_CAST_F32_I32:
            case OP_CAST_I64_F64:
            case OP_CAST_F64_I64:
            case OP_UPCAST_I8_I16:
            case OP_UPCAST_U8_U16:
            case OP_UPCAST_I16_I32:
            case OP_UPCAST_U16_U32:
            case OP_UPCAST_I32_I64:
            case OP_UPCAST_U32_U64:
            case OP_UPCAST_F32_F64:
            case OP_DCAST_I16_I8:
            case OP_DCAST_U16_U8:
            case OP_DCAST_I32_I16:
            case OP_DCAST_U32_U16:
            case OP_DCAST_I64_I32:
            case OP_DCAST_U64_U32:
            case OP_DCAST_F64_F32: {
                TypeV_ASM_Reg dest = getRegister(parser);

                create_instruction(parser, idx, dest, 0, 0, 1);
                parser->codePoolSize++;
                break;
            }

            // OP_[math]_[type] op1: R, op2: R, dest: R
            case OP_ADD_I8:
            case OP_ADD_U8:
            case OP_ADD_I16:
            case OP_ADD_U16:
            case OP_ADD_I32:
            case OP_ADD_U32:
            case OP_ADD_I64:
            case OP_ADD_U64:
            case OP_ADD_F32:
            case OP_ADD_F64:
            case OP_ADD_PTR_U8:
            case OP_ADD_PTR_U16:
            case OP_ADD_PTR_U32:
            case OP_ADD_PTR_U64:
            case OP_SUB_I8:
            case OP_SUB_U8:
            case OP_SUB_I16:
            case OP_SUB_U16:
            case OP_SUB_I32:
            case OP_SUB_U32:
            case OP_SUB_I64:
            case OP_SUB_U64:
            case OP_SUB_F32:
            case OP_SUB_F64:
            case OP_SUB_PTR_U8:
            case OP_SUB_PTR_U16:
            case OP_SUB_PTR_U32:
            case OP_SUB_PTR_U64:
            case OP_MUL_I8:
            case OP_MUL_U8:
            case OP_MUL_I16:
            case OP_MUL_U16:
            case OP_MUL_I32:
            case OP_MUL_U32:
            case OP_MUL_I64:
            case OP_MUL_U64:
            case OP_MUL_F32:
            case OP_MUL_F64:
            case OP_DIV_I8:
            case OP_DIV_U8:
            case OP_DIV_I16:
            case OP_DIV_U16:
            case OP_DIV_I32:
            case OP_DIV_U32:
            case OP_DIV_I64:
            case OP_DIV_U64:
            case OP_DIV_F32:
            case OP_DIV_F64:
            case OP_MOD_I8:
            case OP_MOD_U8:
            case OP_MOD_I16:
            case OP_MOD_U16:
            case OP_MOD_I32:
            case OP_MOD_U32:
            case OP_MOD_I64:
            case OP_MOD_U64:
            case OP_LSHIFT_I8:
            case OP_LSHIFT_U8:
            case OP_LSHIFT_I16:
            case OP_LSHIFT_U16:
            case OP_LSHIFT_I32:
            case OP_LSHIFT_U32:
            case OP_LSHIFT_I64:
            case OP_LSHIFT_U64:
            case OP_RSHIFT_I8:
            case OP_RSHIFT_U8:
            case OP_RSHIFT_I16:
            case OP_RSHIFT_U16:
            case OP_RSHIFT_I32:
            case OP_RSHIFT_U32:
            case OP_RSHIFT_I64:
            case OP_RSHIFT_U64: {
                TypeV_ASM_Reg op1 = getRegister(parser);
                TypeV_ASM_Reg op2 = getRegister(parser);
                TypeV_ASM_Reg dest = getRegister(parser);

                create_instruction(parser, idx, op1, op2, dest, 3);
                parser->codePoolSize += 3;
                break;
            }

            case OP_CMP_I8:
            case OP_CMP_U8:
            case OP_CMP_I16:
            case OP_CMP_U16:
            case OP_CMP_I32:
            case OP_CMP_U32:
            case OP_CMP_I64:
            case OP_CMP_U64:
            case OP_CMP_F32:
            case OP_CMP_F64:
            case OP_CMP_PTR: {
                TypeV_ASM_Reg op1 = getRegister(parser);
                TypeV_ASM_Reg op2 = getRegister(parser);

                create_instruction(parser, idx, op1, op2, 0, 2);
                parser->codePoolSize += 2;
                break;
            }

            case OP_BAND_8:
            case OP_BAND_16:
            case OP_BAND_32:
            case OP_BAND_64:
            case OP_BOR_8:
            case OP_BOR_16:
            case OP_BOR_32:
            case OP_BOR_64:
            case OP_BXOR_8:
            case OP_BXOR_16:
            case OP_BXOR_32:
            case OP_BXOR_64:
            case OP_AND:
            case OP_OR: {
                TypeV_ASM_Reg op1 = getRegister(parser);
                TypeV_ASM_Reg op2 = getRegister(parser);
                TypeV_ASM_Reg dest = getRegister(parser);

                create_instruction(parser, idx, op1, op2, dest, 3);
                parser->codePoolSize += 3;
                break;
            }
            case OP_NOT:
            case OP_BNOT_8:
            case OP_BNOT_16:
            case OP_BNOT_32:
            case OP_BNOT_64: {
                TypeV_ASM_Reg op1 = getRegister(parser);
                TypeV_ASM_Reg dest = getRegister(parser);

                create_instruction(parser, idx, op1, dest, 0, 3);
                parser->codePoolSize += 2;
                break;
            }
            case OP_J:
            case OP_JE:
            case OP_JNE:
            case OP_JG:
            case OP_JGE:
            case OP_JL:
            case OP_JLE: {
                TypeV_Label *label = getLabel(parser);

                create_instruction(parser, idx, (size_t) label, 0, 0, 2);
                parser->codePoolSize += 1 + 8;
                break;
            }

            case OP_LD_FFI: {
                TypeV_ASM_Reg dest = getRegister(parser);
                TypeV_ASM_Const *c = getConst(parser);

                create_instruction(parser, OP_LD_FFI, dest, c->offsetSize, c->offset, 3);
                parser->codePoolSize += 2 + c->offsetSize;
                break;
            }

            case OP_CALL_FFI: {
                TypeV_ASM_Reg dest = getRegister(parser);
                uint64_t offset = getLongNumber(parser);
                uint8_t offsetSize = bytes_needed(offset);


                create_instruction(parser, OP_CALL_FFI, dest, offsetSize, offset, 3);
                parser->codePoolSize += 2 + offsetSize;
                break;
            }
            case OP_CLOSE_FFI: {
                TypeV_ASM_Reg dest = getRegister(parser);
                create_instruction(parser, OP_CLOSE_FFI, dest, 0, 0, 1);
                parser->codePoolSize++;
                break;
            }

            case OP_P_ALLOC: {
                TypeV_ASM_Reg dest = getRegister(parser);
                TypeV_Label *label = getLabel(parser);

                create_instruction(parser, OP_P_ALLOC, dest, (size_t)label, 0, 3);
                parser->codePoolSize += 2+8;
                break;
            }

            case OP_P_DEQUEUE: {
                TypeV_ASM_Reg dest = getRegister(parser);
                TypeV_ASM_Reg promiseDest = getRegister(parser);
                create_instruction(parser, OP_P_DEQUEUE, dest, promiseDest, 0, 2);
                parser->codePoolSize+= 2;
                break;
            }

            case OP_P_QUEUE_SIZE: {
                TypeV_ASM_Reg dest = getRegister(parser);
                create_instruction(parser, OP_P_QUEUE_SIZE, dest, 0, 0, 1);
                parser->codePoolSize++;
                break;
            }

            case OP_P_EMIT: {
                TypeV_ASM_Reg targetReg = getRegister(parser);
                TypeV_ASM_Reg dataReg = getRegister(parser);
                TypeV_ASM_Reg promiseReg = getRegister(parser);
                create_instruction(parser, OP_P_EMIT, targetReg, dataReg, promiseReg, 3);
                parser->codePoolSize+= 3;
                break;
            }

            case OP_P_WAIT_QUEUE: {
                create_instruction(parser, OP_P_WAIT_QUEUE, 0, 0, 0, 0);
                break;
            }

            case OP_P_SEND_SIG: {
                TypeV_ASM_Reg reg = getRegister(parser);
                uint8_t sig = getShortNumber(parser);
                create_instruction(parser, OP_P_SEND_SIG, reg, sig, 0, 2);
                parser->codePoolSize+=2;
                break;
            }

            case OP_P_ID: {
                TypeV_ASM_Reg targetReg = getRegister(parser);
                TypeV_ASM_Reg dataReg = getRegister(parser);
                create_instruction(parser, OP_P_ID, targetReg, dataReg, 0, 2);
                parser->codePoolSize+= 2;
                break;
            }

            case OP_P_CID: {
                TypeV_ASM_Reg targetReg = getRegister(parser);
                create_instruction(parser, OP_P_CID, targetReg, 0, 0, 1);
                parser->codePoolSize+= 1;
                break;
            }

            case OP_P_STATE: {
                TypeV_ASM_Reg targetReg = getRegister(parser);
                TypeV_ASM_Reg dataReg = getRegister(parser);
                create_instruction(parser, OP_P_STATE, targetReg, dataReg, 0, 2);
                parser->codePoolSize+= 2;
                break;
            }

            case OP_PROMISE_ALLOC: {
                TypeV_ASM_Reg targetReg = getRegister(parser);
                create_instruction(parser, OP_PROMISE_ALLOC, targetReg, 0, 0, 1);
                parser->codePoolSize+= 1;
                break;
            }

            case OP_PROMISE_RESOLVE: {
                TypeV_ASM_Reg targetReg = getRegister(parser);
                TypeV_ASM_Reg dataReg = getRegister(parser);
                create_instruction(parser, OP_PROMISE_RESOLVE, targetReg, dataReg, 0, 2);
                parser->codePoolSize+= 2;
                break;
            }

            case OP_LOCK_RELEASE:
            case OP_PROMISE_AWAIT: {
                TypeV_ASM_Reg targetReg = getRegister(parser);
                create_instruction(parser, idx, targetReg, 0, 0, 1);
                parser->codePoolSize+= 1;
                break;
            }


            case OP_PROMISE_DATA:
            case OP_LOCK_ALLOC:
            case OP_LOCK_ACQUIRE:
            {
                TypeV_ASM_Reg targetReg = getRegister(parser);
                TypeV_ASM_Reg dataReg = getRegister(parser);
                create_instruction(parser, idx, targetReg, dataReg, 0, 2);
                parser->codePoolSize+= 2;
                break;
            }


            case OP_DEBUG_REG: {
                TypeV_ASM_Reg reg = getRegister(parser);

                create_instruction(parser, OP_DEBUG_REG, reg, 0, 0, 1);
                parser->codePoolSize++;
                break;
            }
            case OP_HALT: {
                create_instruction(parser, OP_HALT, 0, 0, 0, 0);

                break;
            }

            case OP_LOAD_STD: {
                uint8_t libId = getShortNumber(parser);
                uint8_t  fnId = getShortNumber(parser);

                create_instruction(parser, OP_LOAD_STD, libId, fnId, 0, 2);
                parser->codePoolSize+=2;
                break;
            }

            case OP_VM_HEALTH: {
                TypeV_ASM_Reg reg = getRegister(parser);

                create_instruction(parser, OP_VM_HEALTH, reg, 0, 0, 1);
                parser->codePoolSize++;
                break;
            }
        }
            tok = next_token(parser);
    }

    LOG_INFO("EOF reached, gracefully exiting");
}

/**
 * Encores a constant based on its type
 * @param c
 * @return
 */
void* encode_const(TypeV_ASM_Const* c){
    uint8_t * encoded = malloc(sizeof(uint8_t) * type_size(c->type));
    switch(c->type){
        case ASM_U8:
        case ASM_I8:
        {
            encoded[0] = (uint8_t)c->value;
            break;
        }
        case ASM_I16:
        case ASM_U16:
        {
            uint16_t v = c->value;
            memcpy(encoded, &v, sizeof(uint16_t));
            break;
        }
        case ASM_I32:
        case ASM_U32:
        case ASM_F32:
        {
            uint32_t v = (uint32_t )c->value;
            memcpy(encoded, &v, sizeof(uint32_t));
            break;
        }
        case ASM_I64:
        case ASM_U64:
        case ASM_F64:
        {
            uint64_t v = c->value;
            memcpy(encoded, &v, sizeof(uint64_t));
            break;
        }
        case ASM_PTR:
        {
            // for ptr we assume 64 bit
            uint64_t v = c->value;
            memcpy(encoded, &v, sizeof(uint64_t));
            break;
        }
    }
    return encoded;
}

TypeV_ASM_Program* assemble(TypeV_ASM_Parser* parser) {
    TypeV_ASM_Program* program = malloc(sizeof(TypeV_ASM_Program));
    program->version = parser->version;

    // first encode constants
    uint8_t* constPool = malloc(sizeof(uint8_t) * parser->constPoolSize);
    // iterate through the const pool and encode each const
    for(size_t i = 0; i < parser->constPool.total; i++){
        TypeV_ASM_Const* c = vector_get(&parser->constPool, i);
        void* encoded = encode_const(c);
        memcpy(constPool + c->offset, encoded, type_size(c->type));
        free(encoded);
    }
    program->constPool = constPool;
    program->constPoolSize = parser->constPoolSize;

    /* globals */
    uint8_t* globalPool = malloc(sizeof(uint8_t) * parser->globalPoolSize);
    for(size_t i = 0; i < parser->globalPool.total; i++){
        TypeV_ASM_Global* g = vector_get(&parser->globalPool, i);
        // warning: casting global to const here
        uint8_t* encoded = encode_const((TypeV_ASM_Const*)g);
        memcpy(globalPool + g->offset, encoded, type_size(g->type));
        free(encoded);
    }

    program->globalPool = globalPool;
    program->globalPoolSize = parser->globalPoolSize;

    /* code */
    uint8_t* codePool = malloc(sizeof(uint8_t) * parser->codePoolSize);
    for(size_t i = 0; i < parser->codePool.total; i++) {
        TypeV_ASM_Instruction *inst = vector_get(&parser->codePool, i);

        codePool[inst->offset] = inst->opcode;

        if(inst->opcode == OP_C_STOREM){
            TypeV_Label* label = (TypeV_Label*)inst->arg2;
            // find label
            TypeV_Label* original = find_label(parser, label->name);
            // if original is null, fail.
            if(original == NULL){
                LOG_ERROR("Label %s not found", label->name);
                exit(1);
            }

            uint64_t offset = original->primaryCodeOffset;
            uint64_t length = 8;

            inst->arg2 = length;
            inst->arg3 = offset;
        }
        else if (inst->opcode == OP_FN_CALLI ||
            inst->opcode == OP_J ||
            inst->opcode == OP_JE ||
            inst->opcode == OP_JNE ||
            inst->opcode == OP_JG ||
            inst->opcode == OP_JGE ||
            inst->opcode == OP_JL ||
            inst->opcode == OP_JLE)
        {
            TypeV_Label* label = (TypeV_Label*)inst->arg1;
            // find label
            TypeV_Label* original = find_label(parser, label->name);
            // if original is null, fail.
            if(original == NULL){
                LOG_ERROR("Label %s not found", label->name);
                exit(1);
            }

            uint64_t offset = original->primaryCodeOffset;
            uint64_t length = 8;

            inst->arg1 = length;
            inst->arg2 = offset;
        }
        else if (inst->opcode == OP_P_ALLOC){
            TypeV_Label* label = (TypeV_Label*)inst->arg2;
            TypeV_Label* original = find_label(parser, label->name);
            // if original is null, fail.
            if(original == NULL){
                LOG_ERROR("Label %s not found", label->name);
                exit(1);
            }

            uint64_t offset = original->primaryCodeOffset;
            uint64_t length = 8;

            inst->arg2 = length;
            inst->arg3 = offset;
        }


        uint64_t abs_offset = inst->offset + 1;
        if(inst->num_args > 0){
            memcpy(codePool + abs_offset, &inst->arg1, bytes_needed(inst->arg1));
            abs_offset += bytes_needed(inst->arg1);
        }
        if(inst->num_args > 1){
            memcpy(codePool + abs_offset, &inst->arg2, bytes_needed(inst->arg2));
            abs_offset += bytes_needed(inst->arg2);
        }
        if(inst->num_args > 2){
            memcpy(codePool + abs_offset, &inst->arg3, bytes_needed(inst->arg3));
        }
    }

    program->codePool = codePool;
    program->codePoolSize = parser->codePoolSize;

    return program;
}

void debug_program(TypeV_ASM_Program* program) {
    struct table t;
    table_init(&t,
               "<>", "%llu",
               "<>", "%llu",
               "<>", "%llu",
               "<>", "%llu", NULL
    );
    for(size_t i = 0; i < program->codePoolSize; i+=4){
        // read the next 4 bytes or set to zero if we can't
        uint8_t v1 = program->codePool[i];
        uint8_t v2 = i+1 < program->codePoolSize ? program->codePool[i+1] : 0;
        uint8_t v3 = i+2 < program->codePoolSize ? program->codePool[i+2] : 0;
        uint8_t v4 = i+3 < program->codePoolSize ? program->codePool[i+3] : 0;

        table_add(&t, v1, v2, v3, v4);
    }
    table_print(&t, 400, stdout);
    table_free(&t);
}