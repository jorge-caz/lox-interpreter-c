#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "evaluator.h"

void create_hashtable(HashTable *ht)
{
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        ht->table[i] = NULL; // Initialize all elements to NULL
    }
}

void create_builtin(HashTable *scope, char *name)
{
    Expr new_function = create_expr(name, BUILTIN, -1);
    insert(scope, name, new_function);
}

Expr create_function(HashTable *scope, char *name, int index, int line)
{
    Expr new_function = create_expr(name, FUN, line);
    new_function.index = index;
    new_function.scope = scope;
    return new_function;
}

unsigned int hash(const char *key)
{
    unsigned int hashValue = 0;
    while (*key)
    {
        hashValue += (unsigned int)*key++;
    }
    return hashValue % TABLE_SIZE;
}

void insert(HashTable *ht, const char *key, Expr value)
{
    unsigned int index = hash(key);
    Pair *pair = ht->table[index];

    // Search for the key in the linked list
    while (pair)
    {
        if (strcmp(pair->key, key) == 0)
        {
            // Key already exists, update value and return
            pair->value = value;
            return;
        }
        pair = pair->next;
    }

    Pair *newPair = (Pair *)malloc(sizeof(Pair));
    newPair->key = strdup(key);
    newPair->value = value;
    newPair->next = ht->table[index];
    ht->table[index] = newPair;
}
// Create a new scope with a reference to the parent
HashTable *create_scope(HashTable *parent)
{
    HashTable *newScope = (HashTable *)malloc(sizeof(HashTable));
    create_hashtable(newScope); // Initialize the new scope
    newScope->parent = parent;  // Set parent scope
    return newScope;
}

void destroy_scope(HashTable *scope)
{
    freeHashTable(scope); // Free the variables inside the scope
    free(scope);          // Free the scope itself
}

Expr *obtain(HashTable *ht, const char *key)
{
    while (ht)
    {
        unsigned int index = hash(key);
        Pair *pair = ht->table[index];

        while (pair)
        {
            if (strcmp(pair->key, key) == 0)
            {
                return &(pair->value);
            }
            pair = pair->next;
        }

        ht = ht->parent;
    }

    return NULL;
}

Expr lookup(HashTable *ht, const char *key)
{
    while (ht)
    {
        unsigned int index = hash(key);
        Pair *pair = ht->table[index];

        while (pair)
        {
            if (strcmp(pair->key, key) == 0)
            {
                return pair->value;
            }
            pair = pair->next;
        }

        ht = ht->parent;
    }

    Expr empty;
    empty.display = "nil";
    empty.line = -1;
    empty.type = NIL;
    return empty;
}

void freeHashTable(HashTable *ht)
{
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        Pair *pair = ht->table[i];
        while (pair)
        {
            Pair *temp = pair;
            pair = pair->next;
            free(temp->key);
            free(temp);
        }
    }
}