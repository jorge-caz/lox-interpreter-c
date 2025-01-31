#include "scanner.h"
#ifndef EVALUATOR_H
#define EVALUATOR_H

typedef struct {
    char* display;
    TokenType type;
    int line;
} Expr;

#define TABLE_SIZE 50
typedef struct Pair {
    char* key;
    Expr value;
    struct Pair* next;
} Pair;

typedef struct {
    Pair* table[TABLE_SIZE];
} HashTable;

unsigned int hash(const char* key);
void insert(HashTable* ht, const char* key, Expr value);
Expr lookup(HashTable* ht, const char* key);
void freeHashTable(HashTable* ht);
void create_hashtable(HashTable* ht);

Token* epeek();
Token* eadvance();
Token* eprevious();
int eis_at_end();
int ematch(TokenType type);
void einitialize(Token** tokens, int* error, int* current, HashTable* ht);

Expr ecode();
Expr eblock();
Expr estatement();
Expr eexpression();
Expr eequality();
Expr ecomparison();
Expr eterm();
Expr efactor();
Expr eunary();
Expr eprimary();
Expr create_expr(const char* display, TokenType type, int line);

#endif