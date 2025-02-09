#include "scanner.h"
#ifndef EVALUATOR_H
#define EVALUATOR_H

#define TABLE_SIZE 50

typedef struct HashTable HashTable;

typedef struct
{
    char *display;
    TokenType type;
    int line;
    int index;
    HashTable *scope;
} Expr;

typedef struct Pair
{
    char *key;
    Expr value;
    struct Pair *next;
} Pair;

struct HashTable
{
    Pair *table[TABLE_SIZE];
    HashTable *parent;
};

int clock_function();

unsigned int hash(const char *key);
void insert(HashTable *ht, const char *key, Expr value);
Expr lookup(HashTable *ht, const char *key);
Expr *obtain(HashTable *ht, const char *key);
void freeHashTable(HashTable *ht);
void create_hashtable(HashTable *ht);
HashTable *create_scope(HashTable *parent);
void destroy_scope(HashTable *scope);
Expr create_function(HashTable *scope, char *name, int index, int line);
void create_builtin(HashTable *scope, char *name);

Token *peek();
Token *advance();
Token *previous();
Token *next();
Token *token_at(int index);
int is_at_end();
int match(TokenType type);
void initialize(Token **tokens, int *error, int *curr, HashTable *ht);

void skip_program(int returnIndex);
void skip_declaration(int returnIndex);
void skip_funDecl(int returnIndex);
void skip_function(int returnIndex);
void skip_parameters(int returnIndex);
void skip_varDecl(int returnIndex);
void skip_returnStmt(int returnIndex);
void skip_statement(int returnIndex);
void skip_block(int returnIndex);
void skip_forStmt(int returnIndex);
void skip_whileStmt(int returnIndex);
void skip_ifStmt(int returnIndex);
void skip_exprStmt(int returnIndex);
void skip_printStmt(int returnIndex);
void skip_expression(int returnIndex);
void skip_assignment(int returnIndex);
void skip_logic_or(int returnIndex);
void skip_logic_and(int returnIndex);
void skip_equality(int returnIndex);
void skip_comparison(int returnIndex);
void skip_term(int returnIndex);
void skip_factor(int returnIndex);
void skip_unary(int returnIndex);
void skip_call(int returnIndex);
void skip_primary(int returnIndex);

Expr program(HashTable *scope, int returnIndex);
Expr declaration(HashTable *scope, int returnIndex);
Expr funDecl(HashTable *scope, int returnIndex);
Expr function(HashTable *scope, int returnIndex);
Expr parameters(HashTable *scope, int returnIndex);
Expr varDecl(HashTable *scope, int returnIndex);
Expr expression(HashTable *scope, int returnIndex);
Expr assignment(HashTable *scope, int returnIndex);
Expr block(HashTable *scope, int returnIndex);
Expr exprStmt(HashTable *scope, int returnIndex);
Expr returnStmt(HashTable *scope, int returnIndex);
Expr printStmt(HashTable *scope, int returnIndex);
Expr forStmt(HashTable *scope, int returnIndex);
Expr whileStmt(HashTable *scope, int returnIndex);
Expr ifStmt(HashTable *scope, int returnIndex);
Expr statement(HashTable *scope, int returnIndex);
Expr expression(HashTable *scope, int returnIndex);
Expr logic_or(HashTable *scope, int returnIndex);
Expr logic_and(HashTable *scope, int returnIndex);
Expr equality(HashTable *scope, int returnIndex);
Expr comparison(HashTable *scope, int returnIndex);
Expr term(HashTable *scope, int returnIndex);
Expr factor(HashTable *scope, int returnIndex);
Expr unary(HashTable *scope, int returnIndex);
Expr call(HashTable *scope, int returnIndex);
Expr primary(HashTable *scope, int returnIndex);
Expr create_expr(const char *display, TokenType type, int line);

#endif