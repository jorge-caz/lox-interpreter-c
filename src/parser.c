#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "scanner.h"
#include "parser.h"

Token* tokenList;
int current = 0;
int* err;

void initialize(Token** tokens, int* error) {
    tokenList = *tokens; err = error;
}

Token* peek() {
    return &tokenList[current];
}
Token* advance() {
    if (!is_at_end()) current++;
    return peek();
}
Token* previous() {
    if (current == 0) return peek();
    return &tokenList[current-1];
}

int is_at_end() {
    return peek()->type == TYPE_EOF;
}

int match(TokenType type) {
    if (peek()->type == type) {
        advance();
        return 1;
    }
    return 0;
}

// expression -> equality
char* expression() {
    return equality();
}

// equality -> comparison (("!=" | "==") comparison)*
char* equality() {
    char* exp = comparison();
    while (match(BANG_EQUAL) || match(EQUAL_EQUAL)) {
        Token operation = *previous();
        char* other = comparison();
        char* val = (char* ) malloc(strlen(exp) + strlen(other) + 7);
        sprintf(val, "(%s %s %s)", operation.lexeme, exp, other);

        free(exp);
        free(other);
        exp = val;
    }
    return exp;
}

// comparison -> term ((">" | ">=" | "<" | "<=") term)*
char* comparison() {
    char* exp = term();
    while (match(GREATER) || match(GREATER_EQUAL) ||
            match(LESS) || match(LESS_EQUAL)) {
        Token operation = *previous();
        char* other = term();
        char* val = (char* ) malloc(strlen(exp) + strlen(other) + strlen(operation.lexeme) + 5);
        sprintf(val, "(%s %s %s)", operation.lexeme, exp, other);

        free(exp);
        free(other);
        exp = val;
    }
    return exp;
}

// term -> factor (("-" | "+") factor)*
char* term() {
    char* exp = factor();
    while (match(MINUS) || match(PLUS)) {
        Token operation = *previous();
        char* other = factor();
        char* val = (char* ) malloc(strlen(exp) + strlen(other) + 6);
        sprintf(val, "(%s %s %s)", operation.lexeme, exp, other);

        free(exp);
        free(other);
        exp = val;
    }
    return exp;
}

// factor -> unary (("*" | "/") unary)*
char* factor() {
    char* exp = unary();
    while (match(STAR) || match(SLASH)) {
        Token operation = *previous();
        char* other = unary();
        char* val = (char* ) malloc(strlen(exp) + strlen(other) + 6);
        sprintf(val, "(%s %s %s)", operation.lexeme, exp, other);

        free(exp);
        free(other);
        exp = val;
    }
    return exp;
}

// unary -> ("!" | "-") unary | primary
char* unary() {
    if (match(BANG) || match(MINUS)) {
        Token operation = *previous();
        char* exp = unary();
        char* value = (char* ) malloc(strlen(exp) + 4);
        sprintf(value, "(%s %s)", operation.lexeme, exp);
        
        free(exp);
        return value;
    }
    return primary();
}

// primary -> NUMBER | STRING | TRUE | FALSE | NIL | "(" expression ")"
char* primary() {
    if (match(NUMBER) || match(STRING) || match(TRUE) || match(FALSE) ||
        match(NIL)) {
            return strdup(previous()->lexeme);
        }
    else if (match(LEFT_PAREN)) {
        char* exp = expression();
        char* value = (char* ) malloc(strlen(exp) + 9);
        if (match(RIGHT_PAREN))
        sprintf(value, "(group %s)", exp);
        else; //must produce syntax error

        free(exp);
        return value;
    } 
    else {
        *err = 1;
        fprintf(stderr, "[line %d] Error at '%s': Expect expression.", peek()->line, peek()->lexeme);
    }
    return "";
}

