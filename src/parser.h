#include "scanner.h"
#ifndef PARSER_H
#define PARSER_H

void initialize(Token** tokens);
Token* peek();
Token* advance();
Token* previous();
int is_at_end();
int match(TokenType type);

void expression();
void equality();
void comparsion();
void term();
void factor();
void unary();
void primary();

#endif