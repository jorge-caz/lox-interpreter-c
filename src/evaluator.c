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

void raise_error(int exit_number, const char *message)
{
    fprintf(stderr, "%s", message);
    exit(exit_number);
}

char *create_error_message(const char *message)
{
    char *error_message = (char *)malloc(75 + strlen(peek()->lexeme) + strlen(message));
    sprintf(error_message, "[line %d] Error at '%s': %s.", peek()->line, peek()->lexeme, message);
    return error_message;
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

// program -> declaration* EOF ;
// declaration -> varDecl | statement;
// varDecl -> "var" IDENTIFIER ( "=" expression )? ";" ;
// statement -> exprStmt | ifStmt | printStmt | whileStmt | forStmt | block;
// block -> "{" declaration* "}"
// ifStmt -> "if" "(" expression ")" statement ("else" statement) ? ;
// forStmt -> "for" "(" ( varDecl | exprStmt | ";" ) expression? ";" expression? ")" statement ;
// whileStmt -> "while" "(" expression ")" statement ;
// exprStmt -> expression ";" ;
// printStmt  -> "print" expression ";" ;
// expression -> assignment ;
// assignment -> IDENTIFIER "=" assignment | logic_or
// logic_or -> logic_and ( "or" logic_and )* ;
// logic_and -> equality ( "and" equality )* ;
// equality -> comparison (("!=" | "==") comparison)* ;
// comparison -> term ((">" | ">=" | "<" | "<=") term)* ;
// term -> factor (("-" | "+") factor)* ;
// factor -> unary (("*" | "/") unary)* ;
// unary -> ("!" | "-") unary | primary ;
// primary -> NUMBER | STRING | TRUE | FALSE | NIL | "(" expression ")" ;

// program -> declaration* EOF ;
void skip_program()
{
    while (!is_at_end())
    {
        skip_declaration();
    }
}
// declaration -> varDecl | statement;
void skip_declaration()
{
    if (is_type(VAR))
    {
        skip_varDecl();
        return;
    }
    skip_statement();
}
// varDecl -> "var" IDENTIFIER ( "=" expression )? ";" ;
void skip_varDecl()
{
    if (match(VAR))
    {
        if (match(IDENTIFIER))
        {
            if (match(EQUAL))
            {
                skip_expression();
            }
            if (match(SEMICOLON))
                ;
        }
    }
}
// statement -> exprStmt | ifStmt | printStmt | whileStmt | block;
void skip_statement()
{
    if (is_type(PRINT))
    {
        skip_printStmt();
        return;
    }
    else if (is_type(LEFT_BRACE))
    {
        skip_block();
        return;
    }
    else if (is_type(IF))
    {
        skip_ifStmt();
        return;
    }
    skip_exprStmt();
}
// block -> "{" declaration* "}"
void skip_block()
{
    if (match(LEFT_BRACE))
    {
        while (!is_at_end())
        {
            if (match(RIGHT_BRACE))
                return;
            skip_declaration();
        }
    }
}

// forStmt -> "for" "(" ( varDecl | exprStmt | ";" ) expression? ";" expression? ")" statement ;
void skip_forStmt()
{
    if (match(FOR))
    {
        if (match(LEFT_PAREN))
        {
            if (match(SEMICOLON))
                ;
            else if (is_type(VAR))
                skip_varDecl();
            else
                skip_exprStmt();

            if (match(SEMICOLON))
                ;
            else
            {
                skip_expression();
                if (!match(SEMICOLON))
                    raise_error(65, create_error_message("Expect expression"));
            }
            if (match(RIGHT_PAREN))
                ;
            else
            {
                skip_expression();
                if (!match(RIGHT_PAREN))
                    raise_error(65, create_error_message("Expect expression"));
            }
            skip_statement();
        }
    }
}
// whileStmt -> "while" "(" expression ")" statement ;
void skip_whileStmt()
{
    if (match(WHILE))
    {
        if (match(LEFT_PAREN))
        {
            skip_expression();
            if (!match(RIGHT_PAREN))
                raise_error(65, create_error_message("Expect expression"));
            skip_statement();
        }
    }
}
// ifStmt -> "if" "(" expression ")" statement ("else" statement) ? ;
void skip_ifStmt()
{
    if (match(IF))
    {
        if (match(LEFT_PAREN))
        {
            skip_expression();
            if (!match(RIGHT_PAREN))
                raise_error(65, create_error_message("Expect expression"));
            skip_statement();
            if (match(ELSE))
                skip_statement();
        }
    }
}
// exprStmt -> expression ";" ;
void skip_exprStmt()
{
    skip_expression();
    if (!match(SEMICOLON))
        raise_error(65, create_error_message("Expect expression"));
}
// printStmt  -> "print" expression ";" ;
void skip_printStmt()
{
    if (match(PRINT))
    {
        skip_expression();
        if (!match(SEMICOLON))
            raise_error(65, create_error_message("Expect expression"));
    }
}
// expression -> assignment ;
void skip_expression()
{
    skip_assignment();
}
// assignment -> IDENTIFIER "=" assignment | logic_or
void skip_assignment()
{
    if (is_type(IDENTIFIER) && next_is_type(EQUAL))
    {
        advance();
        advance();
        skip_assignment();
        return;
    }
    skip_logic_or();
}
// logic_or -> logic_and ( "or" logic_and )* ;
void skip_logic_or()
{
    skip_logic_and();
    while (match(OR))
    {
        skip_logic_and();
    }
}
// logic_and -> equality ( "and" equality )* ;
void skip_logic_and()
{
    skip_equality();
    while (match(AND))
    {
        skip_equality();
    }
}
// equality -> comparison (("!=" | "==") comparison)* ;
void skip_equality()
{
    skip_comparison();
    while (match(BANG_EQUAL) || match(EQUAL_EQUAL))
    {
        skip_comparison();
    }
}
// comparison -> term ((">" | ">=" | "<" | "<=") term)* ;
void skip_comparison()
{
    skip_term();
    while (match(LESS) || match(LESS_EQUAL) || match(GREATER) || match(GREATER_EQUAL))
    {
        skip_term();
    }
}
// term -> factor (("-" | "+") factor)* ;
void skip_term()
{
    skip_factor();
    while (match(MINUS) || match(PLUS))
    {
        skip_factor();
    }
}
// factor -> unary (("*" | "/") unary)* ;
void skip_factor()
{
    skip_unary();
    while (match(STAR) || match(SLASH))
    {
        skip_unary();
    }
}
// unary -> ("!" | "-") unary | primary ;
void skip_unary()
{
    if (match(BANG) || match(MINUS))
    {
        skip_unary();
        return;
    }
    skip_primary();
}
// primary -> NUMBER | STRING | TRUE | FALSE | NIL | "(" expression ")" ;
void skip_primary()
{
    if (match(STRING) || match(TRUE) || match(FALSE) ||
        match(NIL) || match(NUMBER) || match(IDENTIFIER))
    {
        return;
    }
    else if (match(LEFT_PAREN))
    {
        skip_expression();
        if (!match(RIGHT_PAREN))
            raise_error(65, create_error_message("Expect expression"));
    }
    raise_error(65, create_error_message("Expect expression"));
}

// program -> declaration* EOF ;
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
// varDecl -> "var" IDENTIFIER ( "=" expression )? ";" ;
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
    raise_error(65, create_error_message("Expect expression"));
    return create_expr("nil", NIL, peek()->line);
}
// statement -> exprStmt | ifStmt | printStmt | whileStmt | forStmt | block;
Expr statement(HashTable *scope)
{
    if (is_type(PRINT))
        return printStmt(scope);
    else if (is_type(LEFT_BRACE))
        return block(scope);
    else if (is_type(IF))
        return ifStmt(scope);
    else if (is_type(WHILE))
        return whileStmt(scope);
    else if (is_type(FOR))
        return forStmt(scope);
    return exprStmt(scope);
}

// forStmt -> "for" "(" ( varDecl | exprStmt | ";" ) expression? ";" expression? ")" statement ;
Expr forStmt(HashTable *scope)
{
    Expr stmt = create_expr("nil", NIL, -1);
    if (match(FOR))
    {
        if (match(LEFT_PAREN))
        {
            Expr condition;
            int token_condition;
            int token_increment = -1;
            if (match(SEMICOLON))
                ;
            else if (is_type(VAR))
                varDecl(scope);
            else
                exprStmt(scope);

            while (1)
            {
                token_condition = *current;
                if (!match(SEMICOLON))
                {
                    condition = expression(scope);
                    if (!match(SEMICOLON))
                        raise_error(65, create_error_message("Expect expression"));
                }

                else
                    condition = create_expr("true", TRUE, stmt.line);
                if (!match(RIGHT_PAREN))
                {
                    token_increment = *current;
                    skip_expression();
                    if (!match(RIGHT_PAREN))
                        raise_error(65, create_error_message("Expect expression"));
                }
                else
                    token_increment = -1;
                if (condition.type == FALSE || condition.type == NIL)
                {
                    skip_statement();
                    break;
                }
                else
                {
                    stmt = statement(scope);
                }

                if (token_increment != -1)
                {
                    *current = token_increment;
                    expression(scope);
                }
                *current = token_condition;
            }
        }
    }
    return stmt;
}

// whileStmt -> "while" "(" expression ")" statement ;
Expr whileStmt(HashTable *scope)
{
    Expr stmt = create_expr("nil", NIL, -1);
    if (match(WHILE))
    {
        if (match(LEFT_PAREN))
        {
            while (1)
            {
                int token_curr = *current;
                Expr condition = expression(scope);
                if (!match(RIGHT_PAREN))
                    raise_error(65, create_error_message("Expect expression"));
                if (condition.type == FALSE || condition.type == NIL)
                {
                    skip_statement();
                    break;
                }
                else
                    stmt = statement(scope);
                *current = token_curr;
            }
        }
    }
    return stmt;
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
                raise_error(65, create_error_message("Expect expression"));
            if (condition.type == FALSE || condition.type == NIL)
                skip_statement();
            else
                stmt = statement(scope);
            if (match(ELSE))
            {
                if (condition.type != FALSE && condition.type != NIL)
                    skip_statement();
                else
                    stmt = statement(scope);
            }
        }
    }
    return stmt;
}
// block -> "{" declaration* "}"
Expr block(HashTable *scope)
{
    Expr bl = create_expr("nil", NIL, -1);
    if (match(LEFT_BRACE))
    {
        HashTable *newScope = create_scope(scope);
        while (!is_at_end())
        {
            if (bl.type == ERROR || match(RIGHT_BRACE))
            {
                destroy_scope(newScope);
                return bl;
            }
            bl = declaration(newScope);
        }
    }
    raise_error(65, create_error_message("Expect expression"));
    return create_expr("nil", NIL, -1);
}
// exprStmt -> expression ";" ;
Expr exprStmt(HashTable *scope)
{
    Expr expr = expression(scope);
    if (expr.type == ERROR)
        return expr;
    if (!match(SEMICOLON))
    {
        raise_error(65, create_error_message("Expect expression"));
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
            raise_error(65, create_error_message("Expect expression"));
        }
        printf("%s\n", to_print.display);
        return to_print;
    }
    raise_error(65, create_error_message("Expect expression"));
    return create_expr("nil", NIL, -1);
}
// expression -> assignment ;
Expr expression(HashTable *scope)
{
    return assignment(scope);
}
// assignment -> IDENTIFIER "=" assignment | logic_or ;
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
            Expr new_value = assignment(scope);
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
    return logic_or(scope);
}
// logic_or -> logic_and ( "or" logic_and )* ;
Expr logic_or(HashTable *scope)
{
    Expr exp = logic_and(scope);
    while (match(OR))
    {
        Token operation = *previous();
        Expr other = logic_and(scope);
        if (other.type == ERROR)
            return other;

        if ((exp.type == FALSE || exp.type == NIL) && (other.type == FALSE || other.type == NIL))
            exp = create_expr("false", FALSE, exp.line);
        else if (exp.type == FALSE || exp.type == NIL)
        {
            exp = other;
            if (match(OR))
            {
                skip_logic_or();
            }
        }
    }
    return exp;
}
// logic_and -> equality ( "and" equality )* ;
Expr logic_and(HashTable *scope)
{
    Expr exp = equality(scope);
    while (match(AND))
    {
        Token operation = *previous();
        Expr other = equality(scope);
        if (other.type == ERROR)
            return other;

        if ((exp.type == FALSE || exp.type == NIL) || (other.type == FALSE || other.type == NIL))
        {
            exp = create_expr("false", FALSE, exp.line);
            if (match(AND))
            {
                skip_logic_and();
            }
        }
        else
            exp = other;
    }
    return exp;
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
            return other;
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
            return other;
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
            raise_error(65, create_error_message("Expect expression"));
        }
    }
    raise_error(65, create_error_message("Expect expression"));
    return create_expr("nil", NIL, -1);
}
