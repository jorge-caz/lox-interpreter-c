#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "runner.h"
#include "scanner.h"
#include "evaluator.h"

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

int validate_braces(Token* tokens) {
    int depth = 0;  // Tracks the current nesting level
    int i = 0;

    while (tokens[i].type != TYPE_EOF) {  
        if (tokens[i].type == LEFT_BRACE) {
            depth++;  // Entering a new block
        }
        if (tokens[i].type == RIGHT_BRACE) {
            depth--;  // Exiting a block

            // If depth goes negative, a '}' appeared before a matching '{'
            if (depth < 0) {
                fprintf(stderr, "[line %d] Error at end: Expect '}'.\n", tokens[i].line);
                exit(65);
                return 0;  // Return false (invalid)
            }
        }
        i++;  // Move to the next token
    }

    // If depth is not zero at the end, we have an unclosed block
    if (depth != 0) {
        fprintf(stderr, "[line %d] Error at end: Expect '}'.\n", tokens[i].line);
        exit(65);
        return 0;  // Return false (invalid)
    }

    return 1;  // Return true (valid)
}


void run(char* input, int* error, HashTable* ht) {
    HashTable* variable_expressions = ht;
    rerror = error;
    current_tokens = scan_tokens(input, error);
    validate_braces(current_tokens);
    initialize(&current_tokens, error, &curr, variable_expressions);
    HashTable* global_scope = create_scope(NULL);
    Expr to_compute = program(global_scope);
    if (to_compute.type == ERROR) {
        fprintf(stderr, "%s", to_compute.display);
        exit(to_compute.line);
    }
    // while (next_index != NULL) {
    //     Expr to_compute = eexpression();
    //     if (to_compute.type == ERROR) {
    //             fprintf(stderr, "%s", to_compute.display);
    //             exit(to_compute.line);
    //         }
    //     current_tokens = next();
    //     if (current_tokens == NULL) break;
    //     einitialize(&current_tokens, error, &curr, variable_expressions);
    // }
}