#include "scanner.h"
#include "hashtable.h"
#ifndef EVALUATOR_H
#define EVALUATOR_H

typedef struct {
    char* display;
    TokenType type;
    int line;
} Expr;

void einitialize(Token** tokens, int* error, int* current, HashTable* ht);
Token* epeek();
Token* eadvance();
Token* eprevious();
int eis_at_end();
int ematch(TokenType type);

Expr eexpression();
Expr eequality();
Expr ecomparison();
Expr eterm();
Expr efactor();
Expr eunary();
Expr eprimary();
Expr create_expr(const char* display, TokenType type, int line);

#endif