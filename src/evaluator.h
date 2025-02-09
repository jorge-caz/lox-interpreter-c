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

void skip_program();
void skip_declaration();
void skip_varDecl();
void skip_statement();
void skip_block();
void skip_forStmt();
void skip_whileStmt();
void skip_ifStmt();
void skip_exprStmt();
void skip_printStmt();
void skip_expression();
void skip_assignment();
void skip_logic_or();
void skip_logic_and();
void skip_equality();
void skip_comparison();
void skip_term();
void skip_factor();
void skip_unary();
void skip_primary();

Expr program(HashTable *scope);
Expr declaration(HashTable *scope);
Expr varDecl(HashTable *scope);
Expr expression(HashTable *scope);
Expr assignment(HashTable *scope);
Expr block(HashTable *scope);
Expr exprStmt(HashTable *scope);
Expr printStmt(HashTable *scope);
Expr forStmt(HashTable *scope);
Expr whileStmt(HashTable *scope);
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