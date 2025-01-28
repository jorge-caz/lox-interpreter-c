#ifndef SCANNER_H
#define SCANNER_H

typedef enum {
    LEFT_PAREN,
    RIGHT_PAREN,
    LEFT_BRACE,
    RIGHT_BRACE,
    COMMA,
    DOT,
    MINUS,
    PLUS,
    SEMICOLON,
    STAR,
    EQUAL,
    EQUAL_EQUAL,
    BANG,
    BANG_EQUAL,
    LESS,
    LESS_EQUAL,
    GREATER,
    GREATER_EQUAL,
    IDENTIFIER,
    STRING,
    NUMBER,
    AND,
    CLASS,
    ELSE,
    FALSE,
    FOR,
    FUN,
    IF,
    NIL,
    OR,
    PRINT,
    RETURN,
    SUPER,
    THIS,
    TRUE,
    VAR,
    WHILE,
    SLASH,
    EOF,
    ERROR
} TokenType;

typedef struct {
    TokenType type;
    const char* lexeme;
    int line;
} Token;

Token create_token(TokenType type, const char* lexeme, int line);
Token* scan_tokens(char* input, int* error);

#endif