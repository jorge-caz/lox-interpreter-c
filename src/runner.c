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

// returns expr.line=-1 if not assignment
//assignment -> (identifier '=')* expression; 
//assignment -> identifier '=' (assignment | expression);
Expr rassignment(HashTable* ht) {
    if (rmatch(IDENTIFIER)) {
        char* variable_name = rprevious()->lexeme;

        if (rmatch(EQUAL)) {
            int at_peek = curr;
            Expr other = eexpression();
            curr = at_peek;

            if (other.type == ERROR)
            other = rassignment(ht);
            
            if (other.type == ERROR || other.line == -1)
            return create_expr("nil", NIL, -1);

            Expr at_variable = lookup(ht,variable_name);
            if (at_variable.line==-1); //error! variable hasn't been declared
            insert(ht, variable_name, other);
            return other;
        }
    }
    return create_expr("nil", NIL, -1);
}


void run(char* input, int* error, HashTable* ht) {
    HashTable* variable_expressions = ht;
    rerror = error;
    current_tokens = tokenize_by_command(input);
    einitialize(&current_tokens, error, &curr, variable_expressions);
    while (next_index != NULL) {
        if (rmatch(PRINT)) {
            Expr to_print = eexpression();
            if (to_print.type == ERROR) {
                fprintf(stderr, "%s", to_print.display);
                exit(to_print.line);
            }
            printf("%s\n", to_print.display);
        }
        else if (rmatch(VAR)) {
            if (rmatch(IDENTIFIER)) {
                char* variable_name = rprevious()->lexeme;
                if (rmatch(EQUAL)) {
                    Expr new_variable = eexpression();
                    if (new_variable.type == ERROR) {
                        fprintf(stderr, "%s", new_variable.display);
                        exit(new_variable.line);
                    }
                    insert(variable_expressions, variable_name, new_variable);
                }
                else if (rmatch(TYPE_EOF)) {
                    Expr new_variable = create_expr("nil",NIL,rpeek()->line);
                    insert(variable_expressions, variable_name, new_variable);
                }
            }
        }
        else if (rpeek()->type == IDENTIFIER) {
            Expr success = rassignment(variable_expressions);
            if (success.line == -1) {
               Expr compute = eexpression(); 
               if (compute.type == ERROR) {
                    fprintf(stderr, "%s", compute.display);
                    exit(compute.line);
                }
            }
            
            
            //assignment -> (identifier '=')* expression; 
        }
        else {
            Expr to_compute = eexpression();
            if (to_compute.type == ERROR) {
                    fprintf(stderr, "%s", to_compute.display);
                    exit(to_compute.line);
                }
        }
        current_tokens = next();
        if (current_tokens == NULL) break;
        einitialize(&current_tokens, error, &curr, variable_expressions);
    }
}