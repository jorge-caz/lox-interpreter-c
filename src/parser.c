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
void expression() {
    equality();
}

// equality -> comparison (("!=" | "==") comparison)*
void equality() {
    comparison();
    while (match(BANG_EQUAL) || match(EQUAL_EQUAL)) {
        printf(" %s ", previous()->lexeme); comparison();
    }
}

// comparison -> term ((">" | ">=" | "<" | "<=") term)*
void comparison() {
    term();
    while (match(GREATER) || match(GREATER_EQUAL) ||
            match(LESS) || match(LESS_EQUAL)) {
        printf(" %s ", previous()->lexeme); term();
    }
}

// term -> factor (("-" | "+") factor)*
void term() {
    factor();
    while (match(MINUS) || match(PLUS)) {
        printf("(%s ", previous()->lexeme); factor(); printf(")");
    }
}

// factor -> unary (("*" | "/") unary)*
void factor() {
    unary();
    while (match(STAR) || match(SLASH)) {
        printf("(%s ", previous()->lexeme); unary(); printf(")");
    }
}

// unary -> ("!" | "-") unary | primary
void unary() {
    if (match(BANG) || match(MINUS)) {
        printf("(%s ", previous()->lexeme); unary(); printf(")");
    }
    else primary();
}

// primary -> NUMBER | STRING | TRUE | FALSE | NIL | "(" expression ")"
void primary() {
    if (match(NUMBER) || match(STRING) || match(TRUE) || match(FALSE) ||
        match(NIL)) printf("%s", previous()->lexeme);
    else if (match(LEFT_PAREN)) {
        printf("(group ");
        expression();
        match(RIGHT_PAREN);
        printf(")");
    } 
}

