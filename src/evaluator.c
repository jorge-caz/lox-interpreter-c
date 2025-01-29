#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "scanner.h"
#include "evaluator.h"

Token* etokenList;
int ecurrent = 0;
int* err;

void initialize(Token** tokens, int* error) {
    etokenList = *tokens; err = error;
}

Expr create_expr(const char* display, TokenType type) {
    Expr expr;
    expr.display = display;
    expr.type = type;
    return expr;
}

Token* epeek() {
    return &etokenList[ecurrent];
}
Token* eadvance() {
    if (!eis_at_end()) ecurrent++;
    return epeek();
}
Token* eprevious() {
    if (ecurrent == 0) return epeek();
    return &etokenList[ecurrent-1];
}

int eis_at_end() {
    return epeek()->type == TYPE_EOF;
}

int ematch(TokenType type) {
    if (epeek()->type == type) {
        eadvance();
        return 1;
    }
    return 0;
}

// expression -> equality
Expr eexpression() {
    return eequality();
}

// equality -> comparison (("!=" | "==") comparison)*
Expr eequality() {
    Expr exp = ecomparison();
    Expr last = exp;
    while (ematch(BANG_EQUAL) || ematch(EQUAL_EQUAL)) {
        Token operation = *eprevious();
        Expr other = ecomparison();
        int val;
        if (last.type != other.type) {
            val = 0;
        }
        else if (last.type == NUMBER && other.type == NUMBER) {
            float expNumber = strtof(last.display, NULL);
            float otherNumber = strtof(other.display, NULL);
            val = expNumber == otherNumber;
        }
        else if (last.type == STRING && other.type == STRING) {
            val = strcmp(last.display, other.display) == 0;
        }
        else val = 1;
        last = other;
        if (operation.type == BANG_EQUAL) exp = create_expr(val?"false":"true", val?FALSE:TRUE);
        else if (operation.type == EQUAL_EQUAL) exp = create_expr(val?"true":"false", val?TRUE:FALSE);
    }
    return exp;
}

// comparison -> term ((">" | ">=" | "<" | "<=") term)*
Expr ecomparison() {
    Expr exp = eterm();
    Expr last = exp;
    while (ematch(GREATER) || ematch(GREATER_EQUAL) || ematch(LESS) || ematch(LESS_EQUAL)) {
        Token operation = *eprevious();
        Expr other = eterm();
        if (last.type != NUMBER || other.type != NUMBER); // type error
        if (exp.type == FALSE) continue;
        float expNumber = strtof(last.display, NULL);
        float otherNumber = strtof(other.display, NULL);
        last = other;
        
        int val;
        if (operation.type == GREATER) val = expNumber > otherNumber;
        else if (operation.type == GREATER_EQUAL) val = expNumber >= otherNumber;
        else if (operation.type == LESS) val = expNumber < otherNumber;
        else if (operation.type == LESS_EQUAL) val = expNumber <= otherNumber;
        exp = create_expr(val?"true":"false", val?TRUE:FALSE);
    }
    return exp;
}

// term -> factor (("-" | "+") factor)*
Expr eterm() {
    Expr exp = efactor();
    while (ematch(MINUS) || ematch(PLUS)) {
        Token operation = *eprevious();
        Expr other = efactor();
        if (exp.type == NUMBER && other.type == NUMBER) {
            float expNumber = strtof(exp.display, NULL);
            float otherNumber = strtof(other.display, NULL);
            char* newDisplay = (char* ) malloc(strlen(exp.display) + strlen(other.display) + 8);
            if (operation.type == MINUS) sprintf(newDisplay, "%.7g", expNumber-otherNumber);
            else if (operation.type == PLUS) sprintf(newDisplay, "%.7g", expNumber+otherNumber);
            exp = create_expr(newDisplay, NUMBER);
        }
        else if (exp.type == STRING && other.type == STRING) {
            if (operation.type == MINUS); //type error
            char* newDisplay = (char* ) malloc(strlen(exp.display) + strlen(other.display) + 1);
            sprintf(newDisplay, "%s%s", exp.display, other.display);
            exp = create_expr(newDisplay, STRING);
        }
    }
    return exp;
}
// factor -> unary (("*" | "/") unary)*
Expr efactor() {
    Expr exp = eunary();
    while (ematch(STAR) || ematch(SLASH)) {
        Token operation = *eprevious();
        Expr other = eunary();
        if (exp.type != NUMBER || other.type != NUMBER); // type error
        float expNumber = strtof(exp.display, NULL);
        float otherNumber = strtof(other.display, NULL);
        char* newDisplay = (char* ) malloc(strlen(exp.display) + strlen(other.display) + 8);
        if (operation.type == STAR) sprintf(newDisplay, "%.7g", expNumber*otherNumber);
        else if (operation.type == SLASH) sprintf(newDisplay, "%.7g", expNumber/otherNumber);
        exp = create_expr(newDisplay, NUMBER);
    }
    return exp;
}

// unary -> ("!" | "-") unary | primary
Expr eunary() {
    if (ematch(BANG)) {
        Expr exp = eunary();
        if (exp.type == FALSE) return create_expr("true", TRUE);
        else if (exp.type == TRUE) return create_expr("false", FALSE);
        else if (exp.type == NUMBER) {
            if (exp.display == "0.0" || exp.display == "0") return create_expr("true", TRUE);
            return create_expr("false", FALSE);
        }
        // must have type error
    }
    if (ematch(MINUS)) {
        Expr exp = eunary();
        char* newstr = (char* ) malloc(strlen(exp.display) + 2);
        sprintf(newstr, "-%s", exp.display);
        if (exp.type == NUMBER) return create_expr(newstr, NUMBER);
        // must have type error
    }
    return eprimary();
}

// primary -> NUMBER | STRING | TRUE | FALSE | NIL | "(" expression ")"
Expr eprimary() {
    if (ematch(NUMBER) || ematch(STRING) || ematch(TRUE) || ematch(FALSE) ||
        ematch(NIL)) {
            return create_expr(eprevious()->lexeme,eprevious()->type);
        }
    else if (ematch(LEFT_PAREN)) {
        Expr exp = eexpression();
        if (ematch(RIGHT_PAREN))
        return exp;
        else {
            fprintf(stderr, "[line %d] Error at '%s': Expect expression.", epeek()->line, epeek()->lexeme);
            exit(65);
        }
    } 
    else {
        fprintf(stderr, "[line %d] Error at '%s': Expect expression.", epeek()->line, epeek()->lexeme);
        exit(65);
    }
}

