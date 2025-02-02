#include "scanner.h"
#ifndef PARSER_H
#define PARSER_H

void pinitialize(Token** tokens, int* error);
Token* ppeek();
Token* padvance();
Token* pprevious();
int pis_at_end();
int pmatch(TokenType type);

char* pexpression();
char* pequality();
char* pcomparison();
char* pterm();
char* pfactor();
char* punary();
char* pprimary();

#endif