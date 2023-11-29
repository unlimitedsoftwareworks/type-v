//
// Created by praisethemoon on 28.11.23.
//

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include "assembler.h"
#include "../utils/log.h"

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
            //continue;
            // return semi colon
            vector_add(&lexer->tokens, createToken(TOK_SEMICOLON, strdup(";"), lexer->line, lexer->col, lexer->pos));
        }
        if (isdigit(c)) {
            uint64_t start = lexer->pos;
            int dotFound = 0;
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
    LOG_ERROR("%s, line: %zu, col: %zu, pos: %zu", msg, tok->line, tok->col, tok->pos);
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
    memcpy(&g->value, &value, type_size(type));
    parser->globalPoolSize += type_size(type);
    LOG_INFO("Creating global: %s, type: %d, size: %d, offset: %d, value: %d", g->name, g->type, type_size(type), g->offset, g->value);
    vector_add(&parser->globalPool, g);
    return g;
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
                    assert_tok(value, TOK_NUMBER);
                    v = atoll(value->value);
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
        // const pool

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




    // step 2 const pool

    // step 3 global pool

    // step 4 code
}



