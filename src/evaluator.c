#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "scanner.h"
#include "evaluator.h"

Token *tokenList;
int *current;
int *err;
HashTable *variables;

void initialize(Token **tokens, int *error, int *curr, HashTable *ht)
{
    tokenList = *tokens;
    err = error;
    current = curr;
    variables = ht;
}

Expr create_expr(const char *display, TokenType type, int line)
{
    Expr expr;
    expr.display = display;
    expr.type = type;
    expr.line = line;
    return expr;
}

Token *peek()
{
    return &tokenList[*current];
}
Token *advance()
{
    if (!is_at_end())
        (*current)++;
    return peek();
}
Token *previous()
{
    if (*current == 0)
        return peek();
    return &tokenList[(*current) - 1];
}

Token *next()
{
    if (is_at_end())
        return NULL;
    return &tokenList[(*current) + 1];
}

int is_at_end()
{
    return (peek()->type == TYPE_EOF && *current >= 0);
}

int is_type(TokenType type)
{
    return peek()->type == type;
}

int next_is_type(TokenType type)
{
    if (next() == NULL)
        return 0;
    return next()->type == type;
}

int match(TokenType type)
{
    if (peek()->type == type)
    {
        advance();
        return 1;
    }
    return 0;
}

void skip_statement()
{
    if (is_at_end())
        return;

    if (match(IF)) // ifStmt
    {
        if (match(LEFT_PAREN))
        {
            while (!match(RIGHT_PAREN) && !is_at_end())
                advance();
        }
        skip_statement(); // Skip the true branch
        if (match(ELSE))
            skip_statement(); // Skip the false branch if exists
    }
    else if (match(LEFT_BRACE)) // block
    {
        int brace_count = 1;
        while (brace_count > 0 && !is_at_end())
        {
            if (match(LEFT_BRACE))
                brace_count++;
            else if (match(RIGHT_BRACE))
                brace_count--;
            else
                advance();
        }
    }
    else
    {
        while (!match(SEMICOLON) && !is_at_end())
        {
            advance();
        }
    }
}

// program -> declaration* EOF ;
// declaration -> varDecl | statement;
// varDecl -> "var" IDENTIFIER ( "=" expression )? ";" ;
// statement -> exprStmt | ifStmt | printStmt | block;
// block -> "{" declaration* "}"
// ifStmt -> "if" "(" expression ")" statement ("else" statement) ? ;
// exprStmt -> expression ";" ;
// printStmt  -> "print" expression ";" ;
// expression -> assignment ;
// assignment -> IDENTIFIER "=" assignment | equality ;

Expr program(HashTable *scope)
{
    while (!is_at_end())
    {
        Expr bl = declaration(scope);
        if (bl.type == ERROR)
            return bl;
    }
    return create_expr("nil", NIL, -1);
}

// declaration -> varDecl | statement;
Expr declaration(HashTable *scope)
{
    // varDecl starts with "var"
    if (is_type(VAR))
        return varDecl(scope);
    return statement(scope);
}

Expr varDecl(HashTable *scope)
{
    // varDecl -> "var" IDENTIFIER ( "=" expression )? ";" ;
    if (match(VAR))
    {
        if (match(IDENTIFIER))
        {
            char *variable_name = previous()->lexeme;
            Expr new_variable;
            if (match(EQUAL))
            {
                new_variable = expression(scope);
                if (new_variable.type == ERROR)
                    return new_variable;
                insert(scope, variable_name, new_variable);
                Expr *found = obtain(scope, variable_name);
            }
            else
            {
                new_variable = create_expr("nil", NIL, peek()->line);
                insert(scope, variable_name, new_variable);
            }
            if (match(SEMICOLON))
                return new_variable;
        }
    }
    char *error_message = (char *)malloc(55 + strlen(peek()->lexeme));
    sprintf(error_message, "[line %d] Error at '%s': Expect expression.", peek()->line, peek()->lexeme);
    *current = -1;
    return create_expr(error_message, ERROR, 65);
}

// statement -> exprStmt | printStmt | block;
Expr statement(HashTable *scope)
{
    if (is_type(PRINT))
        return printStmt(scope);
    else if (is_type(LEFT_BRACE))
        return block(scope);
    else if (is_type(IF))
        return ifStmt(scope);
    return exprStmt(scope);
}

// ifStmt -> "if" "(" expression ")" statement ("else" statement) ? ;
Expr ifStmt(HashTable *scope)
{
    Expr stmt = create_expr("nil", NIL, -1);
    if (match(IF))
    {
        if (match(LEFT_PAREN))
        {
            Expr condition = expression(scope);
            if (!match(RIGHT_PAREN))
                ; // error
            if (condition.type == FALSE || condition.type == NIL)
                skip_statement();
            else
                stmt = statement(scope);
            if (match(ELSE))
            {
                if (condition.type != FALSE && condition.type != NIL)
                    skip_statement();
            }
            else
                stmt = statement(scope);
        }
    }
    return stmt;
}

// block -> "{" declaration* "}"
Expr block(HashTable *scope)
{
    if (match(LEFT_BRACE))
    {
        HashTable *newScope = create_scope(scope);
        while (!is_at_end())
        {

            Expr bl = declaration(newScope);

            if (match(RIGHT_BRACE))
            {
                destroy_scope(newScope);
                return bl;
            }

            if (bl.type == ERROR)
                return bl;
        }
    }
    char *error_message = (char *)malloc(55 + strlen(peek()->lexeme));
    sprintf(error_message, "[line %d] Error at '%s': Expect expression.", peek()->line, peek()->lexeme);
    *current = -1;
    return create_expr(error_message, ERROR, 65);
}

// exprStmt -> expression ";" ;
Expr exprStmt(HashTable *scope)
{
    Expr expr = expression(scope);
    if (!match(SEMICOLON))
    {
        char *error_message = (char *)malloc(55 + strlen(peek()->lexeme));
        sprintf(error_message, "[line %d] Error at '%s': Expect expression.", peek()->line, peek()->lexeme);
        *current = -1;
        return create_expr(error_message, ERROR, 65);
    }
    return expr;
}
// printStmt  -> "print" expression ";" ;
Expr printStmt(HashTable *scope)
{
    if (match(PRINT))
    {
        Expr to_print = expression(scope);
        if (to_print.type == ERROR)
            return to_print;
        if (!match(SEMICOLON))
        {
            char *error_message = (char *)malloc(55 + strlen(peek()->lexeme));
            sprintf(error_message, "[line %d] Error at '%s': Expect expression.", peek()->line, peek()->lexeme);
            *current = -1;
            return create_expr(error_message, ERROR, 65);
        }
        printf("%s\n", to_print.display);
        return to_print;
    }
    char *error_message = (char *)malloc(55 + strlen(peek()->lexeme));
    sprintf(error_message, "[line %d] Error at '%s': Expect expression.", peek()->line, peek()->lexeme);
    *current = -1;
    return create_expr(error_message, ERROR, 65);
}

// statement -> exprStmt | printStmt | block;
// block -> "{" declaration* "}"
// exprStmt -> expression ";" ;
// printStmt  -> "print" expression ";" ;
// expression -> assignment ;
// assignment -> IDENTIFIER "=" assignment | equality ;

// expression -> assignment ;
Expr expression(HashTable *scope)
{
    return assignment(scope);
}

// assignment -> IDENTIFIER "=" assignment | equality ;
Expr assignment(HashTable *scope)
{
    if (is_type(IDENTIFIER) && next_is_type(EQUAL))
    {
        advance();
        char *var_name = previous()->lexeme;
        Expr *var_value = obtain(scope, var_name);
        if (var_value == NULL)
        {
            char *error_message = (char *)malloc(50 + strlen(var_name));
            sprintf(error_message, "Undefined variable '%s'.\n[line %d]\n", var_name, peek()->line);
            *current = -1;
            return create_expr(error_message, ERROR, 70);
        }

        else if (match(EQUAL))
        {
            Expr new_value = expression(scope);
            if (new_value.type == ERROR)
                return new_value;
            var_value->display = new_value.display;
            var_value->line = new_value.line;
            var_value->type = new_value.type;
            // insert(scope, var_name, new_value);
            return (*var_value);
        }

        return (*var_value);
    }
    return equality(scope);
}

// equality -> comparison (("!=" | "==") comparison)*
Expr equality(HashTable *scope)
{
    Expr exp = comparison(scope);
    Expr last = exp;
    while (match(BANG_EQUAL) || match(EQUAL_EQUAL))
    {
        Token operation = *previous();
        Expr other = comparison(scope);
        if (other.type == ERROR)
            other;
        int val;
        if (last.type != other.type)
        {
            val = 0;
        }
        else if (last.type == NUMBER && other.type == NUMBER)
        {
            float expNumber = strtof(last.display, NULL);
            float otherNumber = strtof(other.display, NULL);
            val = expNumber == otherNumber;
        }
        else if (last.type == STRING && other.type == STRING)
        {
            val = strcmp(last.display, other.display) == 0;
        }
        else
            val = 1;
        last = other;
        if (operation.type == BANG_EQUAL)
            exp = create_expr(val ? "false" : "true", val ? FALSE : TRUE, last.line);
        else if (operation.type == EQUAL_EQUAL)
            exp = create_expr(val ? "true" : "false", val ? TRUE : FALSE, last.line);
    }
    return exp;
}

// comparison -> term ((">" | ">=" | "<" | "<=") term)*
Expr comparison(HashTable *scope)
{
    Expr exp = term(scope);
    Expr last = exp;
    while (match(GREATER) || match(GREATER_EQUAL) || match(LESS) || match(LESS_EQUAL))
    {
        Token operation = *previous();
        Expr other = term(scope);
        if (other.type == ERROR)
            return other;
        if (last.type != NUMBER || other.type != NUMBER)
        {
            char *error_message = (char *)malloc(50);
            sprintf(error_message, "Operands must be numbers.\n[line %d]\n", last.line);
            *current = -1;
            return create_expr(error_message, ERROR, 70);
        }
        if (exp.type == FALSE)
            continue;
        float expNumber = strtof(last.display, NULL);
        float otherNumber = strtof(other.display, NULL);
        last = other;

        int val;
        if (operation.type == GREATER)
            val = expNumber > otherNumber;
        else if (operation.type == GREATER_EQUAL)
            val = expNumber >= otherNumber;
        else if (operation.type == LESS)
            val = expNumber < otherNumber;
        else if (operation.type == LESS_EQUAL)
            val = expNumber <= otherNumber;
        exp = create_expr(val ? "true" : "false", val ? TRUE : FALSE, last.line);
    }
    return exp;
}

// term -> factor (("-" | "+") factor)*
Expr term(HashTable *scope)
{
    Expr exp = factor(scope);
    while (match(MINUS) || match(PLUS))
    {
        Token operation = *previous();
        Expr other = factor(scope);
        if (other.type == ERROR)
            return other;
        if (exp.type == NUMBER && other.type == NUMBER)
        {
            float expNumber = strtof(exp.display, NULL);
            float otherNumber = strtof(other.display, NULL);
            char *newDisplay = (char *)malloc(strlen(exp.display) + strlen(other.display) + 8);
            if (operation.type == MINUS)
                sprintf(newDisplay, "%.7g", expNumber - otherNumber);
            else if (operation.type == PLUS)
                sprintf(newDisplay, "%.7g", expNumber + otherNumber);
            exp = create_expr(newDisplay, NUMBER, other.line);
        }
        else if (exp.type == STRING && other.type == STRING)
        {
            if (operation.type == MINUS)
                ; // type error
            char *newDisplay = (char *)malloc(strlen(exp.display) + strlen(other.display) + 1);
            sprintf(newDisplay, "%s%s", exp.display, other.display);
            exp = create_expr(newDisplay, STRING, other.line);
        }
        else
        {
            char *error_message = (char *)malloc(70);
            sprintf(error_message, "Operands must be two numbers or two strings.\n[line %d]\n", exp.line);
            *current = -1;
            return create_expr(error_message, ERROR, 70);
        }
    }
    return exp;
}
// factor -> unary (("*" | "/") unary)*
Expr factor(HashTable *scope)
{
    Expr exp = unary(scope);
    while (match(STAR) || match(SLASH))
    {
        Token operation = *previous();
        Expr other = unary(scope);
        if (other.type == ERROR)
            other;
        if (exp.type != NUMBER || other.type != NUMBER)
        {
            char *error_message = (char *)malloc(55);
            sprintf(error_message, "Operands must be numbers.\n[line %d]\n", exp.line);
            *current = -1;
            return create_expr(error_message, ERROR, 70);
        }
        float expNumber = strtof(exp.display, NULL);
        float otherNumber = strtof(other.display, NULL);
        char *newDisplay = (char *)malloc(strlen(exp.display) + strlen(other.display) + 8);
        if (operation.type == STAR)
            sprintf(newDisplay, "%.7g", expNumber * otherNumber);
        else if (operation.type == SLASH)
            sprintf(newDisplay, "%.7g", expNumber / otherNumber);
        exp = create_expr(newDisplay, NUMBER, other.line);
    }
    return exp;
}

// unary -> ("!" | "-") unary | primary
Expr unary(HashTable *scope)
{
    if (match(BANG))
    {
        Expr exp = unary(scope);
        if (exp.type == ERROR)
            return exp;
        if (exp.type == FALSE || exp.type == NIL)
            return create_expr("true", TRUE, exp.line);
        return create_expr("false", FALSE, exp.line);
    }
    if (match(MINUS))
    {
        Expr exp = unary(scope);
        if (exp.type == ERROR)
            return exp;
        if (exp.type != NUMBER)
        {
            char *error_message = (char *)malloc(50);
            sprintf(error_message, "Operand must be a number.\n[line %d]\n", exp.line);
            *current = -1;
            return create_expr(error_message, ERROR, 70);
        }

        float thatNumber = strtof(exp.display, NULL);
        char *newDisplay = (char *)malloc(strlen(exp.display) + 8);
        sprintf(newDisplay, "%.7g", -thatNumber);
        return create_expr(newDisplay, NUMBER, exp.line);
    }
    return primary(scope);
}

// primary -> NUMBER | STRING | TRUE | FALSE | NIL | "(" expression ")"
Expr primary(HashTable *scope)
{
    if (match(STRING) || match(TRUE) || match(FALSE) ||
        match(NIL))
    {
        return create_expr(previous()->lexeme, previous()->type, previous()->line);
    }
    else if (match(NUMBER))
    {
        float thatNumber = strtof(previous()->lexeme, NULL);
        char *newDisplay = (char *)malloc(strlen(previous()->lexeme) + 8);
        sprintf(newDisplay, "%.7g", thatNumber);
        return create_expr(newDisplay, NUMBER, previous()->line);
    }
    else if (match(IDENTIFIER))
    {
        char *var_name = previous()->lexeme;
        Expr *var_value = obtain(scope, var_name);
        if (var_value == NULL)
        {
            char *error_message = (char *)malloc(50 + strlen(var_name));
            sprintf(error_message, "Undefined variable '%s'.\n[line %d]\n", var_name, peek()->line);
            *current = -1;
            return create_expr(error_message, ERROR, 70);
        }
        return *var_value;
    }
    else if (match(LEFT_PAREN))
    {
        Expr exp = expression(scope);
        if (exp.type == ERROR)
            return exp;
        if (match(RIGHT_PAREN))
            return exp;
        else
        {
            char *error_message = (char *)malloc(55 + strlen(peek()->lexeme));
            sprintf(error_message, "[line %d] Error at '%s': Expect expression.", peek()->line, peek()->lexeme);
            *current = -1;
            return create_expr(error_message, ERROR, 65);
        }
    }
    else
    {
        char *error_message = (char *)malloc(75 + strlen(peek()->lexeme));
        sprintf(error_message, "[line %d] Error at '%s': Expect expression.", peek()->line, peek()->lexeme);
        *current = -1;
        return create_expr(error_message, ERROR, 65);
    }
}
