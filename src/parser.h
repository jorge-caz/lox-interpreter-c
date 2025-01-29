#include "scanner.h"
#ifndef PARSER_H
#define PARSER_H

void initialize(Token** tokens, int* error);
Token* peek();
Token* advance();
Token* previous();
int is_at_end();
int match(TokenType type);

char* expression();
char* equality();
char* comparison();
char* term();
char* factor();
char* unary();
char* primary();

#endif