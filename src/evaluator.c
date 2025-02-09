#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "scanner.h"
#include "evaluator.h"
#include <time.h>

Token *tokenList;
int *current;
int *err;
HashTable *variables;

int clock_function()
{
    time_t current_time = time(NULL); // Get the current time in seconds since the epoch
    if (current_time == (time_t)(-1))
    {
        // Handle error, unable to get the current time
        perror("time");
        return -1;
    }
    return (int)current_time; // Return as a floating-point value
}

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
    expr.index = -1;
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

Token *token_at(int index)
{
    return &tokenList[index];
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

// program ::= declaration* EOF
// declaration ::= funDecl | varDecl | statement
// funDecl ::=  "fun" function
// function ::= IDENTIFIER "(" parameters? ")" block
// parameters ::= IDENTIFIER ( "," IDENTIFIER)* ;
// varDecl ::= "var" IDENTIFIER ( "=" expression )? ";"
// statement ::= exprStmt | ifStmt | printStmt | whileStmt | forStmt | returnStmt | block
// block ::= "{" declaration* "}"
// returnStmt ::= "return" expression? ";" ;
// ifStmt ::= "if" "(" expression ")" statement ("else" statement) ?
// forStmt ::= "for" "(" ( varDecl | exprStmt | ";" ) expression? ";" expression? ")" statement
// whileStmt ::= "while" "(" expression ")" statement
// exprStmt ::= expression ";"
// printStmt  ::= "print" expression ";"
// expression ::= assignment
// assignment ::= IDENTIFIER "=" assignment | logic_or
// logic_or ::= logic_and ( "or" logic_and )*
// logic_and ::= equality ( "and" equality )*
// equality ::= comparison (("!=" | "==") comparison)*
// comparison ::= term ((">" | ">=" | "<" | "<=") term)*
// term ::= factor (("-" | "+") factor)*
// factor ::= unary (("*" | "/") unary)*
// unary ::= ("!" | "-") unary | call
// call ::= primary ( "(" arguments? ")" )*
// arguments ::= expression ( "," expression )*
// primary ::= NUMBER | STRING | TRUE | FALSE | NIL | "(" expression ")"

// program -> declaration* EOF ;
void skip_program(int returnIndex)
{
    while (!is_at_end())
    {
        skip_declaration(returnIndex);
    }
}
// declaration -> varDecl | statement | funDecl;
void skip_declaration(int returnIndex)
{
    if (is_type(VAR))
    {
        skip_varDecl(returnIndex);
        return;
    }
    else if (is_type(FUN))
        return skip_funDecl(returnIndex);
    skip_statement(returnIndex);
}
// funDecl ::=  "fun" function
void skip_funDecl(int returnIndex)
{
    if (match(FUN))
    {
        skip_function(returnIndex);
    }

    // error;
}
// function ::= IDENTIFIER "(" parameters? ")" block
void skip_function(int returnIndex)
{
    if (match(IDENTIFIER))
    {
        if (match(LEFT_PAREN))
        {
            if (!match(RIGHT_PAREN))
            {
                skip_parameters(returnIndex);
                if (!match(RIGHT_PAREN))
                    ; // error
            }
            skip_block(returnIndex);
            return;
        }
        else
            ; // error
    }
    else
        ; // error
}
// parameters ::= IDENTIFIER ( "," IDENTIFIER)* ;
void skip_parameters(int returnIndex)
{
    if (match(IDENTIFIER))
    {
        while (match(COMMA))
        {
            if (!match(IDENTIFIER))
                ; // error
        }
    }
    else
        ; // error
}
// varDecl -> "var" IDENTIFIER ( "=" expression )? ";" ;
void skip_varDecl(int returnIndex)
{
    if (match(VAR))
    {
        if (match(IDENTIFIER))
        {
            if (match(EQUAL))
            {
                skip_expression(returnIndex);
            }
            if (match(SEMICOLON))
                ;
        }
    }
}
// statement -> exprStmt | ifStmt | printStmt | whileStmt | returnStmt | block;
void skip_statement(int returnIndex)
{
    if (is_type(PRINT))
    {
        skip_printStmt(returnIndex);
        return;
    }
    else if (is_type(LEFT_BRACE))
    {
        skip_block(returnIndex);
        return;
    }
    else if (is_type(IF))
    {
        skip_ifStmt(returnIndex);
        return;
    }
    else if (is_type(WHILE))
    {
        skip_whileStmt(returnIndex);
        return;
    }
    else if (is_type(FOR))
    {
        skip_forStmt(returnIndex);
        return;
    }
    else if (is_type(RETURN))
    {
        skip_returnStmt(returnIndex);
        return;
    }
    skip_exprStmt(returnIndex);
}
// block -> "{" declaration* "}"
void skip_block(int returnIndex)
{
    if (match(LEFT_BRACE))
    {
        while (!is_at_end())
        {
            if (match(RIGHT_BRACE))
                return;
            skip_declaration(returnIndex);
        }
    }
}
// forStmt -> "for" "(" ( varDecl | exprStmt | ";" ) expression? ";" expression? ")" statement ;
void skip_forStmt(int returnIndex)
{
    if (match(FOR))
    {
        if (match(LEFT_PAREN))
        {
            if (match(SEMICOLON))
                ;
            else if (is_type(VAR))
                skip_varDecl(returnIndex);
            else
                skip_exprStmt(returnIndex);

            if (match(SEMICOLON))
                ;
            else
            {
                skip_expression(returnIndex);
                if (!match(SEMICOLON))
                    raise_error(65, create_error_message("Expect expression"));
            }
            if (match(RIGHT_PAREN))
                ;
            else
            {
                skip_expression(returnIndex);
                if (!match(RIGHT_PAREN))
                    raise_error(65, create_error_message("Expect expression"));
            }
            skip_statement(returnIndex);
        }
    }
}
// whileStmt -> "while" "(" expression ")" statement ;
void skip_whileStmt(int returnIndex)
{
    if (match(WHILE))
    {
        if (match(LEFT_PAREN))
        {
            skip_expression(returnIndex);
            if (!match(RIGHT_PAREN))
                raise_error(65, create_error_message("Expect expression"));
            skip_statement(returnIndex);
        }
    }
}
// ifStmt -> "if" "(" expression ")" statement ("else" statement) ? ;
void skip_ifStmt(int returnIndex)
{
    if (match(IF))
    {
        if (match(LEFT_PAREN))
        {
            skip_expression(returnIndex);
            if (!match(RIGHT_PAREN))
                raise_error(65, create_error_message("Expect expression"));
            skip_statement(returnIndex);
            if (match(ELSE))
                skip_statement(returnIndex);
        }
    }
}
// exprStmt -> expression ";" ;
void skip_exprStmt(int returnIndex)
{
    skip_expression(returnIndex);
    if (!match(SEMICOLON))
        raise_error(65, create_error_message("Expect expression"));
}
// returnStmt ::= "return" expression? ";" ;
void skip_returnStmt(int returnIndex)
{
    if (match(RETURN))
    {
        if (!match(SEMICOLON))
        {
            skip_expression(returnIndex);
            if (!match(SEMICOLON))
                ; // error
        }
    }
    else
        ; // error
}
// printStmt  -> "print" expression ";" ;
void skip_printStmt(int returnIndex)
{
    if (match(PRINT))
    {
        skip_expression(returnIndex);
        if (!match(SEMICOLON))
            raise_error(65, create_error_message("Expect expression"));
    }
}
// expression -> assignment ;
void skip_expression(int returnIndex)
{
    skip_assignment(returnIndex);
}
// assignment -> IDENTIFIER "=" assignment | logic_or
void skip_assignment(int returnIndex)
{
    if (is_type(IDENTIFIER) && next_is_type(EQUAL))
    {
        advance();
        advance();
        skip_assignment(returnIndex);
        return;
    }
    skip_logic_or(returnIndex);
}
// logic_or -> logic_and ( "or" logic_and )* ;
void skip_logic_or(int returnIndex)
{
    skip_logic_and(returnIndex);
    while (match(OR))
    {
        skip_logic_and(returnIndex);
    }
}
// logic_and -> equality ( "and" equality )* ;
void skip_logic_and(int returnIndex)
{
    skip_equality(returnIndex);
    while (match(AND))
    {
        skip_equality(returnIndex);
    }
}
// equality -> comparison (("!=" | "==") comparison)* ;
void skip_equality(int returnIndex)
{
    skip_comparison(returnIndex);
    while (match(BANG_EQUAL) || match(EQUAL_EQUAL))
    {
        skip_comparison(returnIndex);
    }
}
// comparison -> term ((">" | ">=" | "<" | "<=") term)* ;
void skip_comparison(int returnIndex)
{
    skip_term(returnIndex);
    while (match(LESS) || match(LESS_EQUAL) || match(GREATER) || match(GREATER_EQUAL))
    {
        skip_term(returnIndex);
    }
}
// term -> factor (("-" | "+") factor)* ;
void skip_term(int returnIndex)
{
    skip_factor(returnIndex);
    while (match(MINUS) || match(PLUS))
    {
        skip_factor(returnIndex);
    }
}
// factor -> unary (("*" | "/") unary)* ;
void skip_factor(int returnIndex)
{
    skip_unary(returnIndex);
    while (match(STAR) || match(SLASH))
    {
        skip_unary(returnIndex);
    }
}
// unary -> ("!" | "-") unary | call ;
void skip_unary(int returnIndex)
{
    if (match(BANG) || match(MINUS))
    {
        skip_unary(returnIndex);
        return;
    }
    skip_call(returnIndex);
}
// call ::= primary ( "(" expression ( "," expression )* ")" )*
void skip_call(int returnIndex)
{
    skip_primary(returnIndex);
    if (match(LEFT_PAREN))
    {
        skip_expression(returnIndex);
        while (match(COMMA))
        {
            skip_expression(returnIndex);
        }
        if (!match(RIGHT_PAREN))
            ; // error
    }
}
// primary -> NUMBER | STRING | TRUE | FALSE | NIL | "(" expression ")" ;
void skip_primary(int returnIndex)
{
    if (match(STRING) || match(TRUE) || match(FALSE) ||
        match(NIL) || match(NUMBER) || match(IDENTIFIER))
    {
        return;
    }
    else if (match(LEFT_PAREN))
    {
        skip_expression(returnIndex);
        if (!match(RIGHT_PAREN))
        {
            raise_error(65, create_error_message("Expect expression"));
        }
    }
    else
    {
        raise_error(65, create_error_message("Expect expression"));
    }
}

// program -> declaration* EOF ;
Expr program(HashTable *scope, int returnIndex)
{
    while (!is_at_end())
    {
        Expr bl = declaration(scope, returnIndex);
        if (*current == returnIndex)
            return bl;
        if (bl.type == ERROR)
            return bl;
    }
    return create_expr("nil", NIL, -1);
}
// declaration ::= funDecl | varDecl | statement
Expr declaration(HashTable *scope, int returnIndex)
{
    // varDecl starts with "var"
    if (is_type(VAR))
        return varDecl(scope, returnIndex);
    else if (is_type(FUN))
        return funDecl(scope, returnIndex);
    return statement(scope, returnIndex);
}
// funDecl ::=  "fun" function
Expr funDecl(HashTable *scope, int returnIndex)
{
    if (!match(FUN))
        ; // error
    return function(scope, returnIndex);
}
// function ::= IDENTIFIER "(" parameters? ")" block
Expr function(HashTable *scope, int returnIndex)
{
    if (match(IDENTIFIER))
    {
        char *variable_name = previous()->lexeme;
        Expr new_variable = create_function(scope, variable_name, *current, peek()->line);
        insert(scope, variable_name, new_variable);
    }
    else
        ; // error
    if (match(LEFT_PAREN))
    {
        if (!match(RIGHT_PAREN))
        {
            skip_parameters(returnIndex);
            if (!match(RIGHT_PAREN))
                ; // error
        }
        skip_block(returnIndex);
    }
    else
        ; // error
    return create_expr("nil", NIL, -1);
}
// parameters ::= IDENTIFIER ( "," IDENTIFIER)* ;
Expr parameters(HashTable *scope, int returnIndex)
{
    if (match(IDENTIFIER))
    {
        while (match(COMMA))
        {
            if (!match(IDENTIFIER))
                ; // error
        }
    }
    else
        ; // error
    return create_expr("nil", NIL, -1);
}
// varDecl -> "var" IDENTIFIER ( "=" expression )? ";" ;
Expr varDecl(HashTable *scope, int returnIndex)
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
                new_variable = expression(scope, returnIndex);
                if (new_variable.type == ERROR)
                    return new_variable;
                insert(scope, variable_name, new_variable);
                // Expr *found = obtain(scope, variable_name);
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
// statement -> exprStmt | ifStmt | printStmt | whileStmt | forStmt | returnStmt | block;
Expr statement(HashTable *scope, int returnIndex)
{
    if (is_type(PRINT))
        return printStmt(scope, returnIndex);
    else if (is_type(LEFT_BRACE))
        return block(scope, returnIndex);
    else if (is_type(IF))
        return ifStmt(scope, returnIndex);
    else if (is_type(WHILE))
        return whileStmt(scope, returnIndex);
    else if (is_type(FOR))
        return forStmt(scope, returnIndex);
    else if (is_type(RETURN))
        return returnStmt(scope, returnIndex);
    return exprStmt(scope, returnIndex);
}
// returnStmt ::= "return" expression? ";" ;
Expr returnStmt(HashTable *scope, int returnIndex)
{
    Expr to_return = create_expr("nil", NIL, -1);
    ;
    if (match(RETURN))
    {
        if (match(SEMICOLON))
        {
            *current = returnIndex;
            return to_return;
        }
        to_return = expression(scope, returnIndex);
        if (!match(SEMICOLON))
            ; // error
        *current = returnIndex;
        return to_return;
    }
    else
        ; // error
    return to_return;
}
// forStmt -> "for" "(" ( varDecl | exprStmt | ";" ) expression? ";" expression? ")" statement ;
Expr forStmt(HashTable *scope, int returnIndex)
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
                varDecl(scope, returnIndex);
            else
                exprStmt(scope, returnIndex);

            while (1)
            {
                token_condition = *current;
                if (!match(SEMICOLON))
                {
                    condition = expression(scope, returnIndex);
                    if (!match(SEMICOLON))
                        raise_error(65, create_error_message("Expect expression"));
                }

                else
                    condition = create_expr("true", TRUE, stmt.line);
                if (!match(RIGHT_PAREN))
                {
                    token_increment = *current;
                    skip_expression(returnIndex);
                    if (!match(RIGHT_PAREN))
                        raise_error(65, create_error_message("Expect expression"));
                }
                else
                    token_increment = -1;
                if (condition.type == FALSE || condition.type == NIL)
                {
                    skip_statement(returnIndex);
                    break;
                }
                else
                {
                    stmt = statement(scope, returnIndex);
                    if (*current == returnIndex)
                        return stmt;
                }

                if (token_increment != -1)
                {
                    *current = token_increment;
                    expression(scope, returnIndex);
                }
                *current = token_condition;
            }
        }
    }
    return stmt;
}
// whileStmt -> "while" "(" expression ")" statement ;
Expr whileStmt(HashTable *scope, int returnIndex)
{
    Expr stmt = create_expr("nil", NIL, -1);
    if (match(WHILE))
    {
        if (match(LEFT_PAREN))
        {
            while (1)
            {
                int token_curr = *current;
                Expr condition = expression(scope, returnIndex);
                if (!match(RIGHT_PAREN))
                    raise_error(65, create_error_message("Expect expression"));
                if (condition.type == FALSE || condition.type == NIL)
                {
                    skip_statement(returnIndex);
                    break;
                }
                else
                {
                    stmt = statement(scope, returnIndex);
                    if (*current == returnIndex)
                        return stmt;
                }

                *current = token_curr;
            }
        }
    }
    return stmt;
}
// ifStmt -> "if" "(" expression ")" statement ("else" statement) ? ;
Expr ifStmt(HashTable *scope, int returnIndex)
{
    Expr stmt = create_expr("nil", NIL, -1);
    if (match(IF))
    {
        if (match(LEFT_PAREN))
        {
            Expr condition = expression(scope, returnIndex);
            if (!match(RIGHT_PAREN))
                raise_error(65, create_error_message("Expect expression"));
            if (condition.type == FALSE || condition.type == NIL)
                skip_statement(returnIndex);
            else
            {
                stmt = statement(scope, returnIndex);
                if (*current == returnIndex)
                    return stmt;
            }

            if (match(ELSE))
            {
                if (condition.type != FALSE && condition.type != NIL)
                    skip_statement(returnIndex);
                else
                {
                    stmt = statement(scope, returnIndex);
                    if (*current == returnIndex)
                        return stmt;
                }
            }
        }
    }
    return stmt;
}
// block -> "{" declaration* "}"
Expr block(HashTable *scope, int returnIndex)
{
    Expr bl = create_expr("nil", NIL, -1);
    if (match(LEFT_BRACE))
    {
        HashTable *newScope = create_scope(scope);
        while (!is_at_end())
        {
            if (bl.type == ERROR)
            {
                destroy_scope(newScope);
                return bl;
            }
            else if (match(RIGHT_BRACE))
            {
                destroy_scope(newScope);
                return create_expr("nil", NIL, -1);
            }
            bl = declaration(newScope, returnIndex);
            if (*current == returnIndex)
                return bl;
        }
    }
    raise_error(65, create_error_message("Expect expression"));
    return create_expr("nil", NIL, -1);
}
// exprStmt -> expression ";" ;
Expr exprStmt(HashTable *scope, int returnIndex)
{
    Expr expr = expression(scope, returnIndex);
    if (expr.type == ERROR)
        return expr;
    if (!match(SEMICOLON))
    {
        raise_error(65, create_error_message("Expect expression"));
    }
    return expr;
}
// printStmt  -> "print" expression ";" ;
Expr printStmt(HashTable *scope, int returnIndex)
{
    if (match(PRINT))
    {
        Expr to_print = expression(scope, returnIndex);
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
Expr expression(HashTable *scope, int returnIndex)
{
    return assignment(scope, returnIndex);
}
// assignment -> IDENTIFIER "=" assignment | logic_or ;
Expr assignment(HashTable *scope, int returnIndex)
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
            Expr new_value = assignment(scope, returnIndex);
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
    return logic_or(scope, returnIndex);
}
// logic_or -> logic_and ( "or" logic_and )* ;
Expr logic_or(HashTable *scope, int returnIndex)
{
    Expr exp = logic_and(scope, returnIndex);
    while (match(OR))
    {
        Token operation = *previous();
        Expr other = logic_and(scope, returnIndex);
        if (other.type == ERROR)
            return other;

        if ((exp.type == FALSE || exp.type == NIL) && (other.type == FALSE || other.type == NIL))
            exp = create_expr("false", FALSE, exp.line);
        else if (exp.type == FALSE || exp.type == NIL)
        {
            exp = other;
            if (match(OR))
            {
                skip_logic_or(returnIndex);
            }
        }
    }
    return exp;
}
// logic_and -> equality ( "and" equality )* ;
Expr logic_and(HashTable *scope, int returnIndex)
{
    Expr exp = equality(scope, returnIndex);
    while (match(AND))
    {
        Token operation = *previous();
        Expr other = equality(scope, returnIndex);
        if (other.type == ERROR)
            return other;

        if ((exp.type == FALSE || exp.type == NIL) || (other.type == FALSE || other.type == NIL))
        {
            exp = create_expr("false", FALSE, exp.line);
            if (match(AND))
            {
                skip_logic_and(returnIndex);
            }
        }
        else
            exp = other;
    }
    return exp;
}
// equality -> comparison (("!=" | "==") comparison)*
Expr equality(HashTable *scope, int returnIndex)
{
    Expr exp = comparison(scope, returnIndex);
    Expr last = exp;
    while (match(BANG_EQUAL) || match(EQUAL_EQUAL))
    {
        Token operation = *previous();
        Expr other = comparison(scope, returnIndex);
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
Expr comparison(HashTable *scope, int returnIndex)
{
    Expr exp = term(scope, returnIndex);
    Expr last = exp;
    while (match(GREATER) || match(GREATER_EQUAL) || match(LESS) || match(LESS_EQUAL))
    {
        Token operation = *previous();
        Expr other = term(scope, returnIndex);
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
Expr term(HashTable *scope, int returnIndex)
{
    Expr exp = factor(scope, returnIndex);
    while (match(MINUS) || match(PLUS))
    {
        Token operation = *previous();
        Expr other = factor(scope, returnIndex);
        if (other.type == ERROR)
            return other;
        if (exp.type == NUMBER && other.type == NUMBER)
        {
            double expNumber = strtod(exp.display, NULL);
            double otherNumber = strtod(other.display, NULL);
            char *newDisplay = (char *)malloc(strlen(exp.display) + strlen(other.display) + 8);
            if (operation.type == MINUS)
                sprintf(newDisplay, "%f", expNumber - otherNumber);
            else if (operation.type == PLUS)
                sprintf(newDisplay, "%f", expNumber + otherNumber);
            remove_trailing_zeros(newDisplay);
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
Expr factor(HashTable *scope, int returnIndex)
{
    Expr exp = unary(scope, returnIndex);
    while (match(STAR) || match(SLASH))
    {
        Token operation = *previous();
        Expr other = unary(scope, returnIndex);
        if (other.type == ERROR)
            return other;
        if (exp.type != NUMBER || other.type != NUMBER)
        {
            char *error_message = (char *)malloc(55);
            sprintf(error_message, "Operands must be numbers.\n[line %d]\n", exp.line);
            *current = -1;
            return create_expr(error_message, ERROR, 70);
        }
        double expNumber = strtod(exp.display, NULL);
        double otherNumber = strtod(other.display, NULL);
        char *newDisplay = (char *)malloc(strlen(exp.display) + strlen(other.display) + 8);
        if (operation.type == STAR)
            sprintf(newDisplay, "%f", expNumber * otherNumber);
        else if (operation.type == SLASH)
            sprintf(newDisplay, "%f", expNumber / otherNumber);
        remove_trailing_zeros(newDisplay);
        exp = create_expr(newDisplay, NUMBER, other.line);
    }
    return exp;
}
// unary ::= ("!" | "-") unary | call
Expr unary(HashTable *scope, int returnIndex)
{
    if (match(BANG))
    {
        Expr exp = unary(scope, returnIndex);
        if (exp.type == ERROR)
            return exp;
        if (exp.type == FALSE || exp.type == NIL)
            return create_expr("true", TRUE, exp.line);
        return create_expr("false", FALSE, exp.line);
    }
    if (match(MINUS))
    {
        Expr exp = unary(scope, returnIndex);
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
    return call(scope, returnIndex);
}
// call ::= primary ( "(" expression ( "," expression )* ")" )*
Expr call(HashTable *scope, int returnIndex)
{
    Expr exp = primary(scope, returnIndex);
    while (match(LEFT_PAREN))
    {
        if (exp.type == FUN)
        {
            int func_position = exp.index + 1;
            HashTable *newScope = create_scope(exp.scope);
            if (!is_type(RIGHT_PAREN))
            {
                while (!match(RIGHT_PAREN))
                {
                    if (token_at(func_position)->type == RIGHT_PAREN)
                        ; // error! mismatch
                    char *variable_name = token_at(func_position)->lexeme;
                    func_position++;
                    if (token_at(func_position)->type == COMMA)
                        func_position++;
                    Expr variable_value = expression(scope, returnIndex);
                    if (match(COMMA))
                        ;
                    insert(newScope, variable_name, variable_value);
                    if (token_at(func_position)->type == RIGHT_PAREN)
                    {
                        func_position++;
                        if (!match(RIGHT_PAREN))
                            ; // error! mismatch
                        break;
                    }
                }
            }
            else if (token_at(func_position)->type != RIGHT_PAREN)
                ; // error! mismatch
            else
            {
                advance();
                func_position++;
            }

            int value_of_current = *current;
            *current = func_position;
            Expr bl = block(newScope, value_of_current);
            if (bl.type == ERROR)
                return bl;
            *current = value_of_current;
            if (bl.type != FUN)
                destroy_scope(newScope);
            exp = bl;
        }
        else if (exp.type == BUILTIN)
        {
            if (strcmp(exp.display, "clock") == 0)
            {
                int cl = clock_function();
                char *disp = (char *)malloc(32);
                sprintf(disp, "%d", cl);
                exp = create_expr(disp, NUMBER, peek()->line);
                if (!match(RIGHT_PAREN))
                    ; // error
                return exp;
            }
        }
        else
            ; // error, not callable
    }
    return exp;
}
// primary -> NUMBER | STRING | TRUE | FALSE | NIL | "(" expression ")"
Expr primary(HashTable *scope, int returnIndex)
{
    if (match(STRING) || match(TRUE) || match(FALSE) ||
        match(NIL))
    {
        return create_expr(previous()->lexeme, previous()->type, previous()->line);
    }
    else if (match(NUMBER))
    {
        double thatNumber = strtod(previous()->lexeme, NULL);
        char *newDisplay = (char *)malloc(strlen(previous()->lexeme) + 8);
        sprintf(newDisplay, "%f", thatNumber);
        remove_trailing_zeros(newDisplay);
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
        Expr exp = expression(scope, returnIndex);
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
