#include <iostream>

#include "compiler.h"
#include "lexer.h"

void compile(char const* source)
{
    initLexer(source);

    int line = -1;
    for (;;) {
        Token token = scanToken();
        if (token.line != line) {
            printf("%4d ", token.line);
            line = token.line;
        } else {
            printf("   | ");
        }
        printf("%2d '%.*s'\n", token.type, token.length, token.start);

        if (token.type == TOKEN_EOF)
            break;
    }
}
