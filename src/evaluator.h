#include "scanner.h"
#ifndef EVALUATOR_H
#define EVALUATOR_H

typedef struct
{
    char *display;
    TokenType type;
    int line;
} Expr;

#define TABLE_SIZE 50
typedef struct Pair
{
    char *key;
    Expr value;
    struct Pair *next;
} Pair;

typedef struct HashTable
{
    Pair *table[TABLE_SIZE];
    struct HashTable *parent;
} HashTable;

unsigned int hash(const char *key);
void insert(HashTable *ht, const char *key, Expr value);
Expr lookup(HashTable *ht, const char *key);
Expr *obtain(HashTable *ht, const char *key);
void freeHashTable(HashTable *ht);
void create_hashtable(HashTable *ht);
HashTable *create_scope(HashTable *parent);
void destroy_scope(HashTable *scope);

Token *peek();
Token *advance();
Token *previous();
Token *next();
int is_at_end();
int match(TokenType type);
void initialize(Token **tokens, int *error, int *curr, HashTable *ht);

void code_mover();
void block_mover();
void statement_mover();
void expression_mover();
void equality_mover();
void comparison_mover();
void term_mover();
void factor_mover();
void unary_mover();
void primary_mover();

Expr program(HashTable *scope);
Expr declaration(HashTable *scope);
Expr varDecl(HashTable *scope);
Expr expression(HashTable *scope);
Expr assignment(HashTable *scope);
Expr block(HashTable *scope);
Expr exprStmt(HashTable *scope);
Expr printStmt(HashTable *scope);
Expr ifStmt(HashTable *scope);
Expr statement(HashTable *scope);
Expr expression(HashTable *scope);
Expr logic_or(HashTable *scope);
Expr logic_and(HashTable *scope);
Expr equality(HashTable *scope);
Expr comparison(HashTable *scope);
Expr term(HashTable *scope);
Expr factor(HashTable *scope);
Expr unary(HashTable *scope);
Expr primary(HashTable *scope);
Expr create_expr(const char *display, TokenType type, int line);

#endif