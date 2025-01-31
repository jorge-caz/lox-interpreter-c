#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "scanner.h"
#include "evaluator.h"

Token* etokenList;
int* ecurrent;
int* eerr;
HashTable* evariables;

void einitialize(Token** tokens, int* error, int* current, HashTable* ht) {
    etokenList = *tokens; eerr = error; ecurrent = current; evariables = ht;
}

Expr create_expr(const char* display, TokenType type, int line) {
    Expr expr;
    expr.display = display;
    expr.type = type;
    expr.line = line;
    return expr;
}

Token* epeek() {
    return &etokenList[*ecurrent];
}
Token* eadvance() {
    if (!eis_at_end()) (*ecurrent)++;
    return epeek();
}
Token* eprevious() {
    if (*ecurrent == 0) return epeek();
    return &etokenList[(*ecurrent)-1];
}

int eis_at_end() {
    return (epeek()->type == TYPE_EOF && *ecurrent >= 0);
}

int ematch(TokenType type) {
    if (epeek()->type == type) {
        eadvance();
        return 1;
    }
    return 0;
}



// code -> block*
Expr ecode() {
    while (!eis_at_end()) {
        Expr bl = eblock();
        if (epeek()->type == RIGHT_BRACE)
        return create_expr("nil", NIL, -1);
        if (bl.type == ERROR) return bl;
    }
    return create_expr("nil", NIL, -1);
}

// block -> statement | '{' block* '}'
Expr eblock() {
    if (ematch(LEFT_BRACE)) {
        Expr cod = ecode();
        if (cod.type == ERROR) return cod;
        if (ematch(RIGHT_BRACE)) return create_expr("nil", NIL, -1);
        
        char* error_message = (char* ) malloc(55 + strlen(epeek()->lexeme));
        sprintf(error_message, "[line %d] Error at '%s': Expect expression.", epeek()->line, epeek()->lexeme);
        *ecurrent = -1;
        return create_expr(error_message, ERROR, 65);
    }
    Expr stat = estatement();
    if (stat.type == ERROR) return stat;
    return create_expr("nil", NIL, -1);

}

// statement -> expression ';';
Expr estatement() {
    Expr expre = eexpression();
    if (expre.type == ERROR) return expre;
    if (!ematch(SEMICOLON)) {
        char* error_message = (char* ) malloc(55 + strlen(epeek()->lexeme));
        sprintf(error_message, "[line %d] Error at '%s': Expect expression.", epeek()->line, epeek()->lexeme);
        *ecurrent = -1;
        return create_expr(error_message, ERROR, 65);
    }
    return create_expr("nil", NIL, -1);
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
        if (other.type == ERROR) other;
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
        if (operation.type == BANG_EQUAL) exp = create_expr(val?"false":"true", val?FALSE:TRUE, last.line);
        else if (operation.type == EQUAL_EQUAL) exp = create_expr(val?"true":"false", val?TRUE:FALSE, last.line);
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
        if (other.type == ERROR) return other;
        if (last.type != NUMBER || other.type != NUMBER) {
            char* error_message = (char* ) malloc(50);
            sprintf(error_message, "Operands must be numbers.\n[line %d]\n", last.line);
            *ecurrent = -1;
            return create_expr(error_message, ERROR, 70);
        }
        if (exp.type == FALSE) continue;
        float expNumber = strtof(last.display, NULL);
        float otherNumber = strtof(other.display, NULL);
        last = other;
        
        int val;
        if (operation.type == GREATER) val = expNumber > otherNumber;
        else if (operation.type == GREATER_EQUAL) val = expNumber >= otherNumber;
        else if (operation.type == LESS) val = expNumber < otherNumber;
        else if (operation.type == LESS_EQUAL) val = expNumber <= otherNumber;
        exp = create_expr(val?"true":"false", val?TRUE:FALSE, last.line);
    }
    return exp;
}

// term -> factor (("-" | "+") factor)*
Expr eterm() {
    Expr exp = efactor();
    while (ematch(MINUS) || ematch(PLUS)) {
        Token operation = *eprevious();
        Expr other = efactor();
        if (other.type == ERROR) return other;
        if (exp.type == NUMBER && other.type == NUMBER) {
            float expNumber = strtof(exp.display, NULL);
            float otherNumber = strtof(other.display, NULL);
            char* newDisplay = (char* ) malloc(strlen(exp.display) + strlen(other.display) + 8);
            if (operation.type == MINUS) sprintf(newDisplay, "%.7g", expNumber-otherNumber);
            else if (operation.type == PLUS) sprintf(newDisplay, "%.7g", expNumber+otherNumber);
            exp = create_expr(newDisplay, NUMBER, other.line);
        }
        else if (exp.type == STRING && other.type == STRING) {
            if (operation.type == MINUS); //type error
            char* newDisplay = (char* ) malloc(strlen(exp.display) + strlen(other.display) + 1);
            sprintf(newDisplay, "%s%s", exp.display, other.display);
            exp = create_expr(newDisplay, STRING, other.line);
        }
        else {
            char* error_message = (char* ) malloc(70);
            sprintf(error_message, "Operands must be two numbers or two strings.\n[line %d]\n", exp.line);
            *ecurrent = -1;
            return create_expr(error_message, ERROR, 70);
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
        if (other.type == ERROR) other;
        if (exp.type != NUMBER || other.type != NUMBER) {
            char* error_message = (char* ) malloc(55);
            sprintf(error_message, "Operands must be numbers.\n[line %d]\n", exp.line);
            *ecurrent = -1;
            return create_expr(error_message, ERROR, 70);
        }
        float expNumber = strtof(exp.display, NULL);
        float otherNumber = strtof(other.display, NULL);
        char* newDisplay = (char* ) malloc(strlen(exp.display) + strlen(other.display) + 8);
        if (operation.type == STAR) sprintf(newDisplay, "%.7g", expNumber*otherNumber);
        else if (operation.type == SLASH) sprintf(newDisplay, "%.7g", expNumber/otherNumber);
        exp = create_expr(newDisplay, NUMBER, other.line);
    }
    return exp;
}

// unary -> ("!" | "-") unary | primary
Expr eunary() {
    if (ematch(BANG)) {
        Expr exp = eunary();
        if (exp.type == ERROR) return exp;
        if (exp.type == FALSE || exp.type == NIL) return create_expr("true", TRUE, exp.line);
        return create_expr("false", FALSE, exp.line);

    }
    if (ematch(MINUS)) {
        Expr exp = eunary();
        if (exp.type == ERROR) return exp;
        if (exp.type != NUMBER) {
            char* error_message = (char* ) malloc(50);
            sprintf(error_message, "Operand must be a number.\n[line %d]\n", exp.line);
            *ecurrent = -1;
            return create_expr(error_message, ERROR, 70);
        }

        float thatNumber = strtof(exp.display, NULL);
        char* newDisplay = (char* ) malloc(strlen(exp.display) + 8);
        sprintf(newDisplay, "%.7g", -thatNumber);
        return create_expr(newDisplay, NUMBER,exp.line);
    }
    return eprimary();
}

// primary -> NUMBER | STRING | TRUE | FALSE | NIL | "(" expression ")"
Expr eprimary() {
    if (ematch(STRING) || ematch(TRUE) || ematch(FALSE) ||
        ematch(NIL)) {
            return create_expr(eprevious()->lexeme,eprevious()->type,eprevious()->line);
        }
    else if (ematch(NUMBER)) {
        float thatNumber = strtof(eprevious()->lexeme, NULL);
        char* newDisplay = (char* ) malloc(strlen(eprevious()->lexeme) + 8);
        sprintf(newDisplay, "%.7g", thatNumber);
        return create_expr(newDisplay, NUMBER,eprevious()->line);
    }
    else if (ematch(IDENTIFIER)) {
        char* var_name = eprevious()->lexeme;
        Expr var_value = lookup(evariables, var_name);
        if (var_value.line == -1) {
            char* error_message = (char* ) malloc(50 + strlen(var_name));
            sprintf(error_message, "Undefined variable '%s'.\n[line %d]\n", var_name, epeek()->line);
            *ecurrent = -1;
            return create_expr(error_message, ERROR, 70);
        }

        else if (ematch(EQUAL)) {
            Expr new_value = eexpression();
            if (new_value.type == ERROR) return new_value;
            insert(evariables, var_name, new_value);
            return new_value;
        }

        return var_value;
    }
    else if (rmatch(PRINT)) {
        Expr to_print = eexpression();
        if (to_print.type == ERROR) return to_print;
        printf("%s\n", to_print.display);
        return to_print;
        }
    else if (rmatch(VAR)) {
        if (rmatch(IDENTIFIER)) {
            char* variable_name = eprevious()->lexeme;
            Expr new_variable;
            if (rmatch(EQUAL)) {
                new_variable = eexpression();
                if (new_variable.type == ERROR) return new_variable;
                insert(evariables, variable_name, new_variable);
            }
            else if (rmatch(TYPE_EOF)) {
                new_variable = create_expr("nil",NIL,epeek()->line);
                insert(evariables, variable_name, new_variable);
            }
            return new_variable;
        }
    }
    else if (ematch(LEFT_PAREN)) {
        Expr exp = eexpression();
        if (exp.type == ERROR) return exp;
        if (ematch(RIGHT_PAREN))
        return exp;
        else {
            char* error_message = (char* ) malloc(55 + strlen(epeek()->lexeme));
            sprintf(error_message, "[line %d] Error at '%s': Expect expression.", epeek()->line, epeek()->lexeme);
            *ecurrent = -1;
            return create_expr(error_message, ERROR, 65);
        }
    } 
    else {
        char* error_message = (char* ) malloc(75 + strlen(epeek()->lexeme));
        sprintf(error_message, "[line %d] Error at '%s': Expect expression.", epeek()->line, epeek()->lexeme);
        *ecurrent = -1;
        return create_expr(error_message, ERROR, 65);
    }
}

