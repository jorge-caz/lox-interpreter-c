#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "runner.h"
#include "scanner.h"
#include "evaluator.h"

char* program;
char* next_index;
int* rerror;
int curr = 0;
Token* current_tokens;


Token* rpeek() {
    return &current_tokens[curr];
}
Token* radvance() {
    if (!ris_at_end()) curr++;
    return rpeek();
}
Token* rprevious() {
    if (curr == 0) return rpeek();
    return &current_tokens[curr-1];
}

int ris_at_end() {
    return (rpeek()->type == TYPE_EOF) || (rpeek()->type == SEMICOLON);
}

int rmatch(TokenType type) {
    if (rpeek()->type == type) {
        radvance();
        return 1;
    }
    return 0;
}

//it initializes the variables, and now the input only reads the first command until ;
Token* tokenize_by_command(char* input) {
    next_index = strchr(input, ';');
    if (next_index == NULL) fprintf(stderr, "Expected a semicolon\n");
    program = input;
    *next_index = '\0'; next_index++;
    return scan_tokens(program, rerror);
}
Token* next() {
    program = next_index;
    next_index = strchr(program, ';');
    if (next_index == NULL) return NULL;
    *next_index = '\0'; next_index++;
    curr = 0;
    return scan_tokens(program, rerror);
} //changes is so now we are reading the next command

void run(char* input, int* error, HashTable* ht) {
    HashTable* variable_expressions = ht;
    rerror = error;
    current_tokens = tokenize_by_command(input);
    einitialize(&current_tokens, error, &curr, variable_expressions);
    while (next_index != NULL) {
        if (rmatch(PRINT)) {
            Expr to_print = eexpression();
            printf("%s\n", to_print.display);
        }
        else if (rmatch(VAR)) {
            if (rmatch(IDENTIFIER)) {
                char* variable_name = rprevious()->lexeme;
                if (rmatch(EQUAL)) {
                    Expr new_variable = eexpression();
                    Expr at_variable = lookup(variable_expressions, variable_name);
                    if (at_variable.line == -1) insert(variable_expressions, variable_name, new_variable);
                    else; //error? function was already defined!
                }
            }
        }
        else {
            Expr to_compute = eexpression();
        }
        current_tokens = next();
        if (current_tokens == NULL) break;
        einitialize(&current_tokens, error, &curr, variable_expressions);
    }
}