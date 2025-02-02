#include "evaluator.h"
#ifndef RUNNER_H
#define RUNNER_H

void run(char* input, int* error, HashTable* ht);

Token* rpeek();
Token* radvance();
Token* rprevious();
int ris_at_end();
int rmatch(TokenType type);

#endif