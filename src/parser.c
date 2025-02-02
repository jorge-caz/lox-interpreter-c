#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "scanner.h"
#include "parser.h"

Token* ptokenList;
int pcurrent = 0;
int* perr;

void pinitialize(Token** tokens, int* error) {
    ptokenList = *tokens; perr = error;
}

Token* ppeek() {
    return &ptokenList[pcurrent];
}
Token* padvance() {
    if (!pis_at_end()) pcurrent++;
    return ppeek();
}
Token* pprevious() {
    if (pcurrent == 0) return ppeek();
    return &ptokenList[pcurrent-1];
}

int pis_at_end() {
    return ppeek()->type == TYPE_EOF;
}

int pmatch(TokenType type) {
    if (ppeek()->type == type) {
        padvance();
        return 1;
    }
    return 0;
}

// pexpression -> pequality
char* pexpression() {
    return pequality();
}

// pequality -> pcomparison (("!=" | "==") pcomparison)*
char* pequality() {
    char* exp = pcomparison();
    while (pmatch(BANG_EQUAL) || pmatch(EQUAL_EQUAL)) {
        Token operation = *pprevious();
        char* other = pcomparison();
        char* val = (char* ) malloc(strlen(exp) + strlen(other) + 7);
        sprintf(val, "(%s %s %s)", operation.lexeme, exp, other);

        free(exp);
        free(other);
        exp = val;
    }
    return exp;
}

// pcomparison -> pterm ((">" | ">=" | "<" | "<=") pterm)*
char* pcomparison() {
    char* exp = pterm();
    while (pmatch(GREATER) || pmatch(GREATER_EQUAL) ||
            pmatch(LESS) || pmatch(LESS_EQUAL)) {
        Token operation = *pprevious();
        char* other = pterm();
        char* val = (char* ) malloc(strlen(exp) + strlen(other) + strlen(operation.lexeme) + 5);
        sprintf(val, "(%s %s %s)", operation.lexeme, exp, other);

        free(exp);
        free(other);
        exp = val;
    }
    return exp;
}

// pterm -> pfactor (("-" | "+") pfactor)*
char* pterm() {
    char* exp = pfactor();
    while (pmatch(MINUS) || pmatch(PLUS)) {
        Token operation = *pprevious();
        char* other = pfactor();
        char* val = (char* ) malloc(strlen(exp) + strlen(other) + 6);
        sprintf(val, "(%s %s %s)", operation.lexeme, exp, other);

        free(exp);
        free(other);
        exp = val;
    }
    return exp;
}

// pfactor -> punary (("*" | "/") punary)*
char* pfactor() {
    char* exp = punary();
    while (pmatch(STAR) || pmatch(SLASH)) {
        Token operation = *pprevious();
        char* other = punary();
        char* val = (char* ) malloc(strlen(exp) + strlen(other) + 6);
        sprintf(val, "(%s %s %s)", operation.lexeme, exp, other);

        free(exp);
        free(other);
        exp = val;
    }
    return exp;
}

// punary -> ("!" | "-") punary | pprimary
char* punary() {
    if (pmatch(BANG) || pmatch(MINUS)) {
        Token operation = *pprevious();
        char* exp = punary();
        char* value = (char* ) malloc(strlen(exp) + 4);
        sprintf(value, "(%s %s)", operation.lexeme, exp);
        
        free(exp);
        return value;
    }
    return pprimary();
}

// pprimary -> NUMBER | STRING | TRUE | FALSE | NIL | "(" pexpression ")"
char* pprimary() {
    if (pmatch(NUMBER) || pmatch(STRING) || pmatch(TRUE) || pmatch(FALSE) ||
        pmatch(NIL)) {
            return strdup(pprevious()->lexeme);
        }
    else if (pmatch(LEFT_PAREN)) {
        char* exp = pexpression();
        char* value = (char* ) malloc(strlen(exp) + 9);
        if (pmatch(RIGHT_PAREN))
        sprintf(value, "(group %s)", exp);
        else {
            *perr = 1;
            free(exp);
            fprintf(stderr, "[line %d] Error at '%s': Expect pexpression.", ppeek()->line, ppeek()->lexeme);
            exit(65);
        }

        free(exp);
        return value;
    } 
    else {
        *perr = 1;
        fprintf(stderr, "%s", pprevious()->lexeme);
        fprintf(stderr, "[line %d] Error at '%s': Expect pexpression.", ppeek()->line, ppeek()->lexeme);
        exit(65);
    }
    return "";
}

