#include "evaluator.h"
#define TABLE_SIZE 50
#ifndef HASHTABLE_H
#define HASHTABLE_H

typedef struct Pair {
    char* key;
    Expr value;
    Pair* next;
} Pair;

typedef struct {
    Pair* table[TABLE_SIZE];
} HashTable;

unsigned int hash(char* key);
void insert(HashTable* ht, char* key, Expr value);
Expr lookup(HashTable* ht, char* key);
void freeHashTable(HashTable* ht);

#endif