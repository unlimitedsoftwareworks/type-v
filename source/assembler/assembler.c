//
// Created by praisethemoon on 28.11.23.
//

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "assembler.h"

void lexer_init(TypeV_ASM_Lexer* lexer, char* program){
    lexer->line = 0;
    lexer->col = 0;
    lexer->pos = 0;
    lexer->program = program;
    lexer->program_length = strlen(program);
}

uint8_t issymbol(char c){
    return (c == '|') || (c == ':') || (c == ',') || (c == '[') || (c == ']' || (c == '(') || (c == ')') ||
    c== '{' || c == '}' || c == '+' || c == '-' || c == '*' || c == '/'
    || c == '%' || c == '^' || c == '&' || c == '!' || c == '~'
    || c == '<' || c == '>' || c == '=' || c == '?') || c == '@' || c == '$' || c == '#';
}

void parser_init(TypeV_ASM_Parser* parser, TypeV_ASM_Lexer* lexer) {
    parser->lexer = lexer;
    parser->constPool = NULL;
    parser->globalPool = NULL;
    parser->code = NULL;
}

char current_token(TypeV_ASM_Lexer* lexer){
    if(lexer->pos >= lexer->program_length){
        return EOF;
    }
    return lexer->program[lexer->pos];
}

char* next_token(TypeV_ASM_Lexer* lexer){
    static char* prev_token = NULL;
    if(prev_token != NULL){
        free(prev_token);
    }

    char cur = current_token(lexer);

    // escape spaces
    while (cur == ' ' || cur == '\t' || cur == '\n' || cur == '\r'){
        lexer->pos++;
        cur = current_token(lexer);
    }

    // escape comments
    if(cur == ';'){
        while ((cur != '\n') && (cur != EOF)){
            lexer->pos++;
            cur = current_token(lexer);
        }
    }

    // read token
    char* token = (char*)malloc(sizeof(char) * 1024);
    uint64_t token_pos = 0;
    if(issymbol(cur)){
        // return the symbol
        token[token_pos] = cur;
        token_pos++;
        lexer->pos++;
        token[token_pos] = '\0';
        prev_token = token;
        return token;
    }

    if(isalpha(cur)){
        // read the identifier
        while (isalnum(cur) || cur == '_'){
            token[token_pos] = cur;
            token_pos++;
            lexer->pos++;
            cur = current_token(lexer);
        }
        token[token_pos] = '\0';
        prev_token = token;
        return token;
    }

    while (isdigit(cur)){
        token[token_pos] = cur;
        token_pos++;
        lexer->pos++;
        cur = current_token(lexer);
    }
    if(cur == '.'){
        token[token_pos] = cur;
        token_pos++;
        lexer->pos++;
        cur = current_token(lexer);
        while (isdigit(cur)){
            token[token_pos] = cur;
            token_pos++;
            lexer->pos++;
            cur = current_token(lexer);
        }
    }

    token[token_pos] = '\0';
    prev_token = token;
    return token;
}

void parseConstPool(TypeV_ASM_Parser* parser){
    char* tok = next_token(parser->lexer);
    // assert tok == ":"
    if(strcmp(tok, ":") != 0){
        fprintf(stderr, "Error: expected ':' at line %d, col %d\n", parser->lexer->line+1, parser->lexer->col);
        return;
    }


}

void parseGlobalPool(TypeV_ASM_Parser* parser){

}


void parseCodePool(TypeV_ASM_Parser* parser){

}

void parse(TypeV_ASM_Lexer* lexer, TypeV_ASM_Parser* parser) {
    char* tok = next_token(lexer);
    if(strcmp(tok, "version") == 0){
        tok = next_token(lexer);
        if(strcmp(tok, ":") == 0){
            tok = next_token(lexer);
            parser->version = atol(tok);
        }
    }
    else{
        fprintf(stderr, "Error: expected 'version:' at line %d, col %d\n", lexer->line+1, lexer->col);
        return;
    }

    tok = next_token(lexer);
    if(strcmp(tok, "const") == 0){
        parseConstPool(parser);
    }
    else{
        fprintf(stderr, "Error: expected 'const:' at line %d, col %d\n", lexer->line+1, lexer->col);
        return;
    }
    tok = next_token(lexer);
    if(strcmp(tok, "global") == 0){
        parseGlobalPool(parser);
    }
    else{
        fprintf(stderr, "Error: expected 'global:' at line %d, col %d\n", lexer->line+1, lexer->col);
        return;
    }

    tok = next_token(lexer);
    if(strcmp(tok, "code") == 0){
        parseCodePool(parser);
    }
    else{
        fprintf(stderr, "Error: expected 'code:' at line %d, col %d\n", lexer->line+1, lexer->col);
        return;
    }
}