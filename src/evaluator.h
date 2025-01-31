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

typedef struct HashTable {
    Pair* table[TABLE_SIZE];
    struct HashTable* parent;
} HashTable;

unsigned int hash(const char* key);
void insert(HashTable* ht, const char* key, Expr value);
Expr lookup(HashTable* ht, const char* key);
Expr* obtain(HashTable* ht, const char* key);
void freeHashTable(HashTable* ht);
void create_hashtable(HashTable* ht);
HashTable* create_scope(HashTable* parent);
void destroy_scope(HashTable* scope);

Token* epeek();
Token* eadvance();
Token* eprevious();
int eis_at_end();
int ematch(TokenType type);
void einitialize(Token** tokens, int* error, int* current, HashTable* ht);

Expr ecode(HashTable* scope);
Expr eblock(HashTable* scope);
Expr estatement(HashTable* scope);
Expr eexpression(HashTable* scope);
Expr eequality(HashTable* scope);
Expr ecomparison(HashTable* scope);
Expr eterm(HashTable* scope);
Expr efactor(HashTable* scope);
Expr eunary(HashTable* scope);
Expr eprimary(HashTable* scope);
Expr create_expr(const char* display, TokenType type, int line);

#endif