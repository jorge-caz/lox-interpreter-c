#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *read_file_contents(const char *filename);

int main(int argc, char *argv[]) {
    // Disable output buffering
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    if (argc < 3) {
        fprintf(stderr, "Usage: ./your_program tokenize <filename>\n");
        return 1;
    }

    int error = 0;
    const char *command = argv[1];

    if (strcmp(command, "tokenize") == 0) {
        // You can use print statements as follows for debugging, they'll be visible when running tests.
        fprintf(stderr, "Logs from your program will appear here!\n");
        
        char *file_contents = read_file_contents(argv[2]);

        // Uncomment this block to pass the first stage
        if (strlen(file_contents) > 0) {
            char* stri = file_contents;
            char ch = *stri;
            int line = 1;
            
            while (ch != '\0') {
                if (ch == '(') printf("LEFT_PAREN ( null\n");
                else if (ch == ')') printf("RIGHT_PAREN ) null\n");
                else if (ch == '{') printf("LEFT_BRACE { null\n");
                else if (ch == '}') printf("RIGHT_BRACE } null\n");
                else if (ch == ',') printf("COMMA , null\n");
                else if (ch == '.') printf("DOT . null\n");
                else if (ch == '-') printf("MINUS - null\n");
                else if (ch == '+') printf("PLUS + null\n");
                else if (ch == ';') printf("SEMICOLON ; null\n");
                else if (ch == '*') printf("STAR * null\n");
                else if (ch == '=') {
                    if (stri[1] == '=') { 
                        printf("EQUAL_EQUAL == null\n");
                        stri+=2; ch = *stri;
                        continue;
                    }
                    else printf("EQUAL = null\n");
                }
                else if (ch == '!') {
                    if (stri[1] == '=') {
                        printf("BANG_EQUAL != null\n");
                        stri+=2; ch = *stri;
                        continue;
                    }
                    else printf("BANG ! null\n");
                }
                else if (ch == '<') {
                    if (stri[1] == '=') {
                        printf("LESS_EQUAL <= null\n");
                        stri+=2; ch = *stri;
                        continue;
                    }
                    else printf("LESS < null\n");
                }
                else if (ch == '>') {
                    if (stri[1] == '=') {
                        printf("GREATER_EQUAL >= null\n");
                        stri+=2; ch = *stri;
                        continue;
                    }
                    else printf("GREATER > null\n");
                }

                //KEYWORDS
                else if (strncmp(stri, "and", 3) == 0) {
                    printf("AND and null\n");
                    stri+=3; ch = *stri;
                    continue;
                }
                else if (strncmp(stri, "class", 5) == 0) {
                    printf("CLASS class null\n");
                    stri+=5; ch = *stri;
                    continue;
                }
                else if (strncmp(stri, "else", 4) == 0) {
                    printf("ELSE else null\n");
                    stri+=4; ch = *stri;
                    continue;
                }
                else if (strncmp(stri, "false", 5) == 0) {
                    printf("FALSE false null\n");
                    stri+=5; ch = *stri;
                    continue;
                }
                else if (strncmp(stri, "for", 3) == 0) {
                    printf("FOR for null\n");
                    stri+=3; ch = *stri;
                    continue;
                }
                else if (strncmp(stri, "fun", 3) == 0) {
                    printf("FUN fun null\n");
                    stri+=3; ch = *stri;
                    continue;
                }
                else if (strncmp(stri, "if", 2) == 0) {
                    printf("IF if null\n");
                    stri+=2; ch = *stri;
                    continue;
                }
                else if (strncmp(stri, "nil", 3) == 0) {
                    printf("NIL nil null\n");
                    stri+=3; ch = *stri;
                    continue;
                }
                else if (strncmp(stri, "or", 2) == 0) {
                    printf("OR or null\n");
                    stri+=2; ch = *stri;
                    continue;
                }
                else if (strncmp(stri, "return", 6) == 0) {
                    printf("RETURN return null\n");
                    stri+=6; ch = *stri;
                    continue;
                }
                else if (strncmp(stri, "super", 5) == 0) {
                    printf("SUPER super null\n");
                    stri+=5; ch = *stri;
                    continue;
                }
                else if (strncmp(stri, "this", 4) == 0) {
                    printf("THIS this null\n");
                    stri+=3; ch = *stri;
                    continue;
                }
                else if (strncmp(stri, "true", 4) == 0) {
                    printf("TRUE true null\n");
                    stri+=4; ch = *stri;
                    continue;
                }
                else if (strncmp(stri, "var", 3) == 0) {
                    printf("VAR var null\n");
                    stri+=3; ch = *stri;
                    continue;
                }
                else if (strncmp(stri, "while", 5) == 0) {
                    printf("WHILE while null\n");
                    stri+=5; ch = *stri;
                    continue;
                }

                //COMMENTS
                else if (ch == '/') {
                    if (stri[1] == '/') {
                        char* newline = strchr(stri, '\n');
                        if (newline != NULL) newline++;
                        else break;
                        stri = newline; ch = *stri; line++;
                        continue;
                    }
                    else {
                        printf("SLASH / null\n");
                    }
                }

                //STRINGS
                else if (ch == '\"') {
                    char* endstr = strchr(stri+1, '\"');
                    if (endstr == NULL) {
                        fprintf(stderr, "[line %d] Error: Unterminated string.\n", line);
                        error = 1;
                        break;
                    }
                    else {
                        stri = strtok(stri+1, "\""); 
                        // may need to handle the line calculation
                        // idea to fix this: do a while loop from the first " 
                        // to the second " count instances of \n
                        printf("STRING \"%s\" %s\n", stri, stri);
                        stri = endstr+1; ch = *stri;
                        continue;
                    }
                }

                //NUMBERS
                else if (ch >= '0' && ch <= '9') {
                    char* notnum = stri+1;
                    int is_integer = 1;
                    while ((*notnum >= '0' && *notnum <= '9') || *notnum == '.') {
                        if (!(*notnum == '.' 
                            && (!(notnum[1] >= '0' && notnum[1] <= '9') || notnum[1] == '.')))
                            {
                                notnum++;
                                if (*notnum == '.') is_integer = 0;
                        }
                        
                        else break;
                    }
                    char temp = *notnum; *notnum = '\0'; 
                    // may need to handle update of the "line" variable
                    // may code a helper function that counts the number of '\n'
                    // instances between two pointers in the same contiguous
                    // string region. Avoid code repetition and bugs
                    float num = strtof(stri, NULL);
                    if ((int) num == num)
                    printf("NUMBER %s %g.0\n", stri, num);
                    else
                    printf("NUMBER %s %.7g\n", stri, num);
                    *notnum = temp;
                    stri = notnum; ch = *stri;
                    continue;

                }

                //IDENFIFIERS
                else if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch =='_') {
                    // tokenize until u find a non-alphanumeric character
                    char* notalphnum = stri+1;
                    while ((*notalphnum >= 'a' && *notalphnum <= 'z') || 
                        (*notalphnum >= 'A' && *notalphnum <= 'Z') || *notalphnum =='_' ||
                        (*notalphnum >= '0' && *notalphnum <= '9')) {
                            notalphnum++;
                    }
                    char temp = *notalphnum; *notalphnum = '\0';
                    printf("IDENTIFIER %s null\n", stri);
                    *notalphnum = temp;
                    stri = notalphnum; ch = *stri;
                    continue;
                }
                


                else if (ch == ' ');
                else if (ch == '\t');



                else if (ch == '\n') line++;
                else {
                    fprintf(stderr, "[line %d] Error: Unexpected character: %c\n", line, ch);
                    error = 1;
                }
                stri++; ch = *stri;
            }
        } 
        printf("EOF  null\n"); // Placeholder, remove this line when implementing the scanner
        
        free(file_contents);
    } else {
        fprintf(stderr, "Unknown command: %s\n", command);
        return 1;
    }

    exit(error ? 65: 0);
    return 0;
}

char *read_file_contents(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error reading file: %s\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    char *file_contents = malloc(file_size + 1);
    if (file_contents == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(file);
        return NULL;
    }

    size_t bytes_read = fread(file_contents, 1, file_size, file);
    if (bytes_read < file_size) {
        fprintf(stderr, "Error reading file contents\n");
        free(file_contents);
        fclose(file);
        return NULL;
    }

    file_contents[file_size] = '\0';
    fclose(file);

    return file_contents;
}
