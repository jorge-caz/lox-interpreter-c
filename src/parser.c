#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "scanner.h"
#include "parser.h"

Token* tokenList;
int current = 0;

void initialize(Token** tokens) {
    tokenList = *tokens;
}

Token* peek() {
    return &tokenList[current];
}
Token* advance() {
    if (!is_at_end()) current++;
    return peek();
}
Token* previous() {
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
        char* other = comparison();
        char* val = (char* ) malloc(strlen(exp) + strlen(other) + 7);
        sprintf(val, "(%s %s %s)", previous()->lexeme, exp, other);

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
        char* other = term();
        char* val = (char* ) malloc(strlen(exp) + strlen(other) + strlen(previous()->lexeme) + 5);
        sprintf(val, "(%s %s %s)", previous()->lexeme, exp, other);

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
        char* other = factor();
        char* val = (char* ) malloc(strlen(exp) + strlen(other) + 6);
        sprintf(val, "(%s %s %s)", previous()->lexeme, exp, other);

        free(exp);
        free(other);
        exp = val;
    }
    return exp;
}

// factor -> unary (("*" | "/") unary)*
char* factor() {
    char* exp = unary();
    printf("\nwe get an exp of %s\n", exp);
    printf("and the token is %s\n", peek()->lexeme);
    printf("the value of current is %d\n", current);
    while (match(STAR) || match(SLASH)) {
        char* other = unary();
        char* val = (char* ) malloc(strlen(exp) + strlen(other) + 6);
        sprintf(val, "(%s %s %s)", previous()->lexeme, exp, other);

        free(exp);
        free(other);
        exp = val;
    }
    return exp;
}

// unary -> ("!" | "-") unary | primary
char* unary() {
    if (match(BANG) || match(MINUS)) {
        char* exp = unary();
        char* value = (char* ) malloc(strlen(exp) + 4);
        sprintf(value, "(%s %s)", previous()->lexeme, exp);
        
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
    else; //syntax error
}

