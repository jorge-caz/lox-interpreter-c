#include "scanner.h"
#ifndef EVALUATOR_H
#define EVALUATOR_H

typedef struct {
    char* display;
    TokenType type;
} Expr;

void einitialize(Token** tokens, int* error);
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
Expr create_expr(const char* display, TokenType type);

#endif