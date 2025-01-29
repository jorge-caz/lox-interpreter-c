#include "scanner.h"
#ifndef RUNNER_H
#define RUNNER_H

//it initializes the variables, and now the input only reads the first command until ;
Token* tokenize_by_command(char* input);
Token* next(); //changes is so now we are reading the next command
void run(char* input, int* error);

Token* rpeek();
Token* radvance();
Token* rprevious();
int ris_at_end();
int rmatch(TokenType type);

#endif