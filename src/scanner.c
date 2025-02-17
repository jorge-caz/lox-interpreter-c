#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "scanner.h"

int n = 0;

void remove_trailing_zeros(char *str)
{
    int len = strlen(str);

    // Find the position of the decimal point
    char *dot = strchr(str, '.');
    if (dot != NULL)
    {
        // Remove trailing zeroes after the decimal point
        while (len > 0 && str[len - 1] == '0')
        {
            str[len - 1] = '\0'; // Remove zeroes
            len--;
        }

        // Remove the decimal point if there are no digits after it
        if (str[len - 1] == '.')
        {
            str[len - 1] = '\0';
        }
    }
}

Token create_token(TokenType type, const char *lexeme, int line)
{
    Token token;
    token.type = type;
    token.lexeme = lexeme;
    token.line = line;
    n++;

    return token;
}

int line = 1;

int is_alphanum(char ch)
{
    return (ch >= 'a' && ch <= 'z') ||
           (ch >= 'A' && ch <= 'Z') || ch == '_' ||
           (ch >= '0' && ch <= '9');
}

char str_get(char *str, int index)
{
    if (index < strlen(str))
        return str[index];
    return '\0';
}

Token *scan_tokens(char *input, int *error)
{
    char *stri = input;
    char ch = *stri;
    Token temp;
    n = 0;
    const int size = strlen(input);
    Token *tokens = (Token *)malloc(size * sizeof(Token));

    while (ch != '\0')
    {
        int m = n;
        if (ch == '(')
            tokens[m] = create_token(LEFT_PAREN, "(", line);
        else if (ch == ')')
            tokens[m] = create_token(RIGHT_PAREN, ")", line);
        else if (ch == '{')
            tokens[m] = create_token(LEFT_BRACE, "{", line);
        else if (ch == '}')
            tokens[m] = create_token(RIGHT_BRACE, "}", line);
        else if (ch == ',')
            tokens[m] = create_token(COMMA, ",", line);
        else if (ch == '.')
            tokens[m] = create_token(DOT, ".", line);
        else if (ch == '-')
            tokens[m] = create_token(MINUS, "-", line);
        else if (ch == '+')
            tokens[m] = create_token(PLUS, "+", line);
        else if (ch == ';')
            tokens[m] = create_token(SEMICOLON, ";", line);
        else if (ch == '*')
            tokens[m] = create_token(STAR, "*", line);
        else if (ch == '=')
        {
            if (stri[1] == '=')
            {
                tokens[m] = create_token(EQUAL_EQUAL, "==", line);
                stri += 2;
                ch = *stri;
                continue;
            }
            else
                tokens[m] = create_token(EQUAL, "=", line);
        }
        else if (ch == '!')
        {
            if (stri[1] == '=')
            {
                tokens[m] = create_token(BANG_EQUAL, "!=", line);
                stri += 2;
                ch = *stri;
                continue;
            }
            else
                tokens[m] = create_token(BANG, "!", line);
        }
        else if (ch == '<')
        {
            if (stri[1] == '=')
            {
                tokens[m] = create_token(LESS_EQUAL, "<=", line);
                stri += 2;
                ch = *stri;
                continue;
            }
            else
                tokens[m] = create_token(LESS, "<", line);
        }
        else if (ch == '>')
        {
            if (stri[1] == '=')
            {
                tokens[m] = create_token(GREATER_EQUAL, ">=", line);
                stri += 2;
                ch = *stri;
                continue;
            }
            else
                tokens[m] = create_token(GREATER, ">", line);
        }

        // KEYWORDS
        else if (strncmp(stri, "and", 3) == 0 && !is_alphanum(str_get(stri, 3)))
        {
            tokens[m] = create_token(AND, "and", line);
            stri += 3;
            ch = *stri;
            continue;
        }
        else if (strncmp(stri, "class", 5) == 0 && !is_alphanum(str_get(stri, 5)))
        {
            tokens[m] = create_token(CLASS, "class", line);
            stri += 5;
            ch = *stri;
            continue;
        }
        else if (strncmp(stri, "else", 4) == 0 && !is_alphanum(str_get(stri, 4)))
        {
            tokens[m] = create_token(ELSE, "else", line);
            stri += 4;
            ch = *stri;
            continue;
        }
        else if (strncmp(stri, "false", 5) == 0 && !is_alphanum(str_get(stri, 5)))
        {
            tokens[m] = create_token(FALSE, "false", line);
            stri += 5;
            ch = *stri;
            continue;
        }
        else if (strncmp(stri, "for", 3) == 0 && !is_alphanum(str_get(stri, 3)))
        {
            tokens[m] = create_token(FOR, "for", line);
            stri += 3;
            ch = *stri;
            continue;
        }
        else if (strncmp(stri, "fun", 3) == 0 && !is_alphanum(str_get(stri, 3)))
        {
            tokens[m] = create_token(FUN, "fun", line);
            stri += 3;
            ch = *stri;
            continue;
        }
        else if (strncmp(stri, "if", 2) == 0 && !is_alphanum(str_get(stri, 2)))
        {
            tokens[m] = create_token(IF, "if", line);
            stri += 2;
            ch = *stri;
            continue;
        }
        else if (strncmp(stri, "nil", 3) == 0 && !is_alphanum(str_get(stri, 3)))
        {
            tokens[m] = create_token(NIL, "nil", line);
            stri += 3;
            ch = *stri;
            continue;
        }
        else if (strncmp(stri, "or", 2) == 0 && !is_alphanum(str_get(stri, 2)))
        {
            tokens[m] = create_token(OR, "or", line);
            stri += 2;
            ch = *stri;
            continue;
        }
        else if (strncmp(stri, "print", 5) == 0 && !is_alphanum(str_get(stri, 5)))
        {
            tokens[m] = create_token(PRINT, "print", line);
            stri += 5;
            ch = *stri;
            continue;
        }
        else if (strncmp(stri, "return", 6) == 0 && !is_alphanum(str_get(stri, 6)))
        {
            tokens[m] = create_token(RETURN, "return", line);
            stri += 6;
            ch = *stri;
            continue;
        }
        else if (strncmp(stri, "super", 5) == 0 && !is_alphanum(str_get(stri, 5)))
        {
            tokens[m] = create_token(SUPER, "super", line);
            stri += 5;
            ch = *stri;
            continue;
        }
        else if (strncmp(stri, "this", 4) == 0 && !is_alphanum(str_get(stri, 4)))
        {
            tokens[m] = create_token(THIS, "this", line);
            stri += 4;
            ch = *stri;
            continue;
        }
        else if (strncmp(stri, "true", 4) == 0 && !is_alphanum(str_get(stri, 4)))
        {
            tokens[m] = create_token(TRUE, "true", line);
            stri += 4;
            ch = *stri;
            continue;
        }
        else if (strncmp(stri, "var", 3) == 0 && !is_alphanum(str_get(stri, 3)))
        {
            tokens[m] = create_token(VAR, "var", line);
            stri += 3;
            ch = *stri;
            continue;
        }
        else if (strncmp(stri, "while", 5) == 0 && !is_alphanum(str_get(stri, 5)))
        {
            tokens[m] = create_token(WHILE, "while", line);
            stri += 5;
            ch = *stri;
            continue;
        }

        // COMMENTS
        else if (ch == '/')
        {
            if (stri[1] == '/')
            {
                char *newline = strchr(stri, '\n');
                if (newline != NULL)
                    newline++;
                else
                    break;
                stri = newline;
                ch = *stri;
                line++;
                continue;
            }
            else
            {
                tokens[m] = create_token(SLASH, "/", line);
            }
        }

        // STRINGS
        else if (ch == '\"')
        {
            char *endstr = strchr(stri + 1, '\"');
            if (endstr == NULL)
            {
                fprintf(stderr, "[line %d] Error: Unterminated string.\n", line);
                *error = 1;
                stri = strchr(stri + 1, '\n');
                if (stri == NULL)
                    return tokens;
                stri = stri + 1;
                ch = *stri;
                continue;
            }
            else
            {
                stri++;
                *endstr = '\0';
                // may need to handle the line calculation
                // idea to fix this: do a while loop from the first "
                // to the second " count instances of \n
                tokens[m] = create_token(STRING, strdup(stri), line);
                stri = endstr + 1;
                ch = *stri;
                continue;
            }
        }

        // NUMBERS
        else if (ch >= '0' && ch <= '9')
        {
            char *notnum = stri + 1;
            int is_integer = 1;
            while ((*notnum >= '0' && *notnum <= '9') || *notnum == '.')
            {
                if (!(*notnum == '.' && (!(notnum[1] >= '0' && notnum[1] <= '9') || notnum[1] == '.')))
                {
                    notnum++;
                    if (*notnum == '.')
                        is_integer = 0;
                }

                else
                    break;
            }
            char temp = *notnum;
            *notnum = '\0';
            // may need to handle update of the "line" variable
            // may code a helper function that counts the number of '\n'
            // instances between two pointers in the same contiguous
            // string region. Avoid code repetition and bugs
            double num = strtod(stri, NULL);
            if ((int)num == num)
            {
                char *newStri = (char *)malloc(strlen(stri) + 3);
                sprintf(newStri, "%f", num);
                remove_trailing_zeros(newStri);
                sprintf(newStri, "%s.0", newStri);
                tokens[m] = create_token(NUMBER, newStri, line);
            }
            else
            {
                char *newStri = (char *)malloc(strlen(stri) + 1);
                sprintf(newStri, "%f", num);
                remove_trailing_zeros(newStri);
                tokens[m] = create_token(NUMBER, newStri, line);
            }

            *notnum = temp;
            stri = notnum;
            ch = *stri;
            continue;
        }

        // IDENFIFIERS
        else if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_')
        {
            // tokenize until u find a non-alphanumeric character
            char *notalphnum = stri + 1;
            while (is_alphanum(*notalphnum))
            {
                notalphnum++;
            }
            char temp = *notalphnum;
            *notalphnum = '\0';
            tokens[m] = create_token(IDENTIFIER, strdup(stri), line);
            *notalphnum = temp;
            stri = notalphnum;
            ch = *stri;
            continue;
        }

        else if (ch == ' ')
            ;
        else if (ch == '\t')
            ;

        else if (ch == '\n')
            line++;
        else
        {
            fprintf(stderr, "[line %d] Error: Unexpected character: %c\n", line, ch);
            *error = 1;
        }
        stri++;
        ch = *stri;
    }
    int m = n;
    tokens[m] = create_token(TYPE_EOF, "", line);
    return tokens;
}

// ORIGINAL LOGIC
void tokenize(char *input, int *error)
{
    char *stri = input;
    char ch = *stri;
    int line = 1;

    while (ch != '\0')
    {
        if (ch == '(')
            printf("LEFT_PAREN ( null\n");
        else if (ch == ')')
            printf("RIGHT_PAREN ) null\n");
        else if (ch == '{')
            printf("LEFT_BRACE { null\n");
        else if (ch == '}')
            printf("RIGHT_BRACE } null\n");
        else if (ch == ',')
            printf("COMMA , null\n");
        else if (ch == '.')
            printf("DOT . null\n");
        else if (ch == '-')
            printf("MINUS - null\n");
        else if (ch == '+')
            printf("PLUS + null\n");
        else if (ch == ';')
            printf("SEMICOLON ; null\n");
        else if (ch == '*')
            printf("STAR * null\n");
        else if (ch == '=')
        {
            if (stri[1] == '=')
            {
                printf("EQUAL_EQUAL == null\n");
                stri += 2;
                ch = *stri;
                continue;
            }
            else
                printf("EQUAL = null\n");
        }
        else if (ch == '!')
        {
            if (stri[1] == '=')
            {
                printf("BANG_EQUAL != null\n");
                stri += 2;
                ch = *stri;
                continue;
            }
            else
                printf("BANG ! null\n");
        }
        else if (ch == '<')
        {
            if (stri[1] == '=')
            {
                printf("LESS_EQUAL <= null\n");
                stri += 2;
                ch = *stri;
                continue;
            }
            else
                printf("LESS < null\n");
        }
        else if (ch == '>')
        {
            if (stri[1] == '=')
            {
                printf("GREATER_EQUAL >= null\n");
                stri += 2;
                ch = *stri;
                continue;
            }
            else
                printf("GREATER > null\n");
        }

        // KEYWORDS
        else if (strncmp(stri, "and", 3) == 0 && !is_alphanum(str_get(stri, 3)))
        {
            printf("AND and null\n");
            stri += 3;
            ch = *stri;
            continue;
        }
        else if (strncmp(stri, "class", 5) == 0 && !is_alphanum(str_get(stri, 5)))
        {
            printf("CLASS class null\n");
            stri += 5;
            ch = *stri;
            continue;
        }
        else if (strncmp(stri, "else", 4) == 0 && !is_alphanum(str_get(stri, 4)))
        {
            printf("ELSE else null\n");
            stri += 4;
            ch = *stri;
            continue;
        }
        else if (strncmp(stri, "false", 5) == 0 && !is_alphanum(str_get(stri, 5)))
        {
            printf("FALSE false null\n");
            stri += 5;
            ch = *stri;
            continue;
        }
        else if (strncmp(stri, "for", 3) == 0 && !is_alphanum(str_get(stri, 3)))
        {
            printf("FOR for null\n");
            stri += 3;
            ch = *stri;
            continue;
        }
        else if (strncmp(stri, "fun", 3) == 0 && !is_alphanum(str_get(stri, 3)))
        {
            printf("FUN fun null\n");
            stri += 3;
            ch = *stri;
            continue;
        }
        else if (strncmp(stri, "if", 2) == 0 && !is_alphanum(str_get(stri, 2)))
        {
            printf("IF if null\n");
            stri += 2;
            ch = *stri;
            continue;
        }
        else if (strncmp(stri, "nil", 3) == 0 && !is_alphanum(str_get(stri, 3)))
        {
            printf("NIL nil null\n");
            stri += 3;
            ch = *stri;
            continue;
        }
        else if (strncmp(stri, "or", 2) == 0 && !is_alphanum(str_get(stri, 2)))
        {
            printf("OR or null\n");
            stri += 2;
            ch = *stri;
            continue;
        }
        else if (strncmp(stri, "print", 5) == 0 && !is_alphanum(str_get(stri, 5)))
        {
            printf("PRINT print null\n");
            stri += 5;
            ch = *stri;
            continue;
        }
        else if (strncmp(stri, "return", 6) == 0 && !is_alphanum(str_get(stri, 6)))
        {
            printf("RETURN return null\n");
            stri += 6;
            ch = *stri;
            continue;
        }
        else if (strncmp(stri, "super", 5) == 0 && !is_alphanum(str_get(stri, 5)))
        {
            printf("SUPER super null\n");
            stri += 5;
            ch = *stri;
            continue;
        }
        else if (strncmp(stri, "this", 4) == 0 && !is_alphanum(str_get(stri, 4)))
        {
            printf("THIS this null\n");
            stri += 4;
            ch = *stri;
            continue;
        }
        else if (strncmp(stri, "true", 4) == 0 && !is_alphanum(str_get(stri, 4)))
        {
            printf("TRUE true null\n");
            stri += 4;
            ch = *stri;
            continue;
        }
        else if (strncmp(stri, "var", 3) == 0 && !is_alphanum(str_get(stri, 3)))
        {
            printf("VAR var null\n");
            stri += 3;
            ch = *stri;
            continue;
        }
        else if (strncmp(stri, "while", 5) == 0 && !is_alphanum(str_get(stri, 5)))
        {
            printf("WHILE while null\n");
            stri += 5;
            ch = *stri;
            continue;
        }

        // COMMENTS
        else if (ch == '/')
        {
            if (stri[1] == '/')
            {
                char *newline = strchr(stri, '\n');
                if (newline != NULL)
                    newline++;
                else
                    break;
                stri = newline;
                ch = *stri;
                line++;
                continue;
            }
            else
            {
                printf("SLASH / null\n");
            }
        }

        // STRINGS
        else if (ch == '\"')
        {
            char *endstr = strchr(stri + 1, '\"');
            if (endstr == NULL)
            {
                fprintf(stderr, "[line %d] Error: Unterminated string.\n", line);
                *error = 1;
                break;
            }
            else
            {
                stri++;
                *endstr = '\0';
                // may need to handle the line calculation
                // idea to fix this: do a while loop from the first "
                // to the second " count instances of \n
                printf("STRING \"%s\" %s\n", stri, stri);
                stri = endstr + 1;
                ch = *stri;
                continue;
            }
        }

        // NUMBERS
        else if (ch >= '0' && ch <= '9')
        {
            char *notnum = stri + 1;
            int is_integer = 1;
            while ((*notnum >= '0' && *notnum <= '9') || *notnum == '.')
            {
                if (!(*notnum == '.' && (!(notnum[1] >= '0' && notnum[1] <= '9') || notnum[1] == '.')))
                {
                    notnum++;
                    if (*notnum == '.')
                        is_integer = 0;
                }

                else
                    break;
            }
            char temp = *notnum;
            *notnum = '\0';
            // may need to handle update of the "line" variable
            // may code a helper function that counts the number of '\n'
            // instances between two pointers in the same contiguous
            // string region. Avoid code repetition and bugs
            float num = strtof(stri, NULL);
            if ((int)num == num)
                printf("NUMBER %s %g.0\n", stri, num);
            else
                printf("NUMBER %s %.7g\n", stri, num);
            *notnum = temp;
            stri = notnum;
            ch = *stri;
            continue;
        }

        // IDENFIFIERS
        else if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_')
        {
            // tokenize until u find a non-alphanumeric character
            char *notalphnum = stri + 1;
            while (is_alphanum(*notalphnum))
            {
                notalphnum++;
            }
            char temp = *notalphnum;
            *notalphnum = '\0';
            printf("IDENTIFIER %s null\n", stri);
            *notalphnum = temp;
            stri = notalphnum;
            ch = *stri;
            continue;
        }

        else if (ch == ' ')
            ;
        else if (ch == '\t')
            ;

        else if (ch == '\n')
            line++;
        else
        {
            fprintf(stderr, "[line %d] Error: Unexpected character: %c\n", line, ch);
            *error = 1;
        }
        stri++;
        ch = *stri;
    }
}