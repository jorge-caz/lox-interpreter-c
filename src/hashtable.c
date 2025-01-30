#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "evaluator.h"

unsigned int hash(char* key) {
    unsigned int hashValue = 0;
    while (*key) {
        hashValue += (unsigned int) *key++;
    }
    return hashValue % TABLE_SIZE;
}

void insert(HashTable* ht, char* key, Expr value) {
    unsigned int index = hash(key);
    Pair* newPair = (Pair*) malloc(sizeof(Pair));
    newPair->key = stdup(key);
    newPair->value = value;
    newPair->next = ht->table[index];
    ht->table[index] = newPair;
}

Expr lookup(HashTable* ht, const char* key) {
    unsigned int index = hash(key);
    Pair* pair = ht->table[index];
    while (pair) {
        if (strcmp(pair->key, key) == 0) {
            return pair->value;
        }
        pair = pair->next;
    }
    Expr empty;
    empty.display = "nil";
    empty.line = -1;
    empty.type = NIL;
    return empty;
}

void freeHashTable(HashTable* ht) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        Pair* pair = ht->table[i];
        while (pair) {
            Pair* temp = pair;
            pair = pair->next;
            free(temp->key);
            free(temp);
        }
    }
}