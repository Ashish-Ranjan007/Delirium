#include <cctype>  // For character classification functions
#include <cstring> // For string operations

#include "lexer.h" // For token definitions

// Lexer state structure
typedef struct Lexer {
    char const* start;   // Start of current token being scanned
    char const* current; // Current position in source
    int line;            // Current line number (1-based)
} Lexer;

// Global lexer state (single instance)
Lexer lexer;

/**
 * Initializes the lexer with source code to tokenize.
 *
 * @param source Null-terminated source string
 *
 * @note Source must persist through tokenization
 */
void initLexer(char const* source)
{
    lexer.start = source;
    lexer.current = source;
    lexer.line = 1;
}

/**
 * Checks if lexer has reached end of input.
 */
static bool isAtEnd()
{
    return *lexer.current == '\0';
}

/**
 * Creates a token of given type from current lexer state.
 */
static Token makeToken(TokenType type)
{
    Token token;
    token.type = type;
    token.start = lexer.start;
    token.length = (int)(lexer.current - lexer.start);
    token.line = lexer.line;
    return token;
}

/**
 * Creates an error token with message.
 */
static Token errorToken(char const* message)
{
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = lexer.line;
    return token;
}

/**
 * Advances lexer and returns consumed character.
 */
static char advance()
{
    lexer.current++;
    return *(lexer.current - 1);
}

/**
 * Conditionally consumes character if it matches expected.
 */
static bool match(char expected)
{
    if (isAtEnd())
        return false;
    if (*lexer.current != expected)
        return false;
    lexer.current++;
    return true;
}

/**
 * Peeks at current character without consuming.
 */
static char peek()
{
    return *lexer.current;
}

/**
 * Peeks at next character without consuming.
 */
static char peekNext()
{
    if (isAtEnd())
        return '\0';
    return lexer.current[1];
}

/**
 * Checks if character is a digit (0-9).
 */
static bool isDigit(char c)
{
    return c >= '0' && c <= '9';
}

/**
 * Checks if character is alphabetic (a-z, A-Z, _).
 */
static bool isAlpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

/**
 * Scans a string literal until closing quote.
 */
static Token string()
{
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n')
            lexer.line++;
        advance();
    }

    if (isAtEnd())
        return errorToken("Unterminated string.");

    advance(); // Consume closing quote
    return makeToken(TOKEN_STRING);
}

/**
 * Scans numeric literals (integer or float).
 */
static Token number()
{
    while (isDigit(peek()))
        advance();

    // Handle fractional part
    if (peek() == '.' && isDigit(peekNext())) {
        advance(); // Consume '.'
        while (isDigit(peek()))
            advance();
    }

    return makeToken(TOKEN_NUMBER);
}

/**
 * Checks if current token matches a keyword.
 */
static TokenType checkKeyword(int start, int length,
    char const* rest, TokenType type)
{
    if (lexer.current - lexer.start == start + length && memcmp(lexer.start + start, rest, length) == 0) {
        return type;
    }
    return TOKEN_IDENTIFIER;
}

/**
 * Determines if current identifier is a reserved keyword.
 */
static TokenType identifierType()
{
    switch (lexer.start[0]) {
    case 'a':
        return checkKeyword(1, 2, "nd", TOKEN_AND);
    case 'c':
        return checkKeyword(1, 4, "lass", TOKEN_CLASS);
    case 'e':
        return checkKeyword(1, 3, "lse", TOKEN_ELSE);
    case 'f':
        if (lexer.current - lexer.start > 1) {
            switch (lexer.start[1]) {
            case 'a':
                return checkKeyword(2, 3, "lse", TOKEN_FALSE);
            case 'o':
                return checkKeyword(2, 1, "r", TOKEN_FOR);
            case 'u':
                return checkKeyword(2, 1, "n", TOKEN_FUN);
            }
        }
        break;
    case 'i':
        return checkKeyword(1, 1, "f", TOKEN_IF);
    case 'n':
        return checkKeyword(1, 2, "il", TOKEN_NIL);
    case 'o':
        return checkKeyword(1, 1, "r", TOKEN_OR);
    case 'p':
        return checkKeyword(1, 4, "rint", TOKEN_PRINT);
    case 'r':
        return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
    case 's':
        return checkKeyword(1, 4, "uper", TOKEN_SUPER);
    case 't':
        if (lexer.current - lexer.start > 1) {
            switch (lexer.start[1]) {
            case 'h':
                return checkKeyword(2, 2, "is", TOKEN_THIS);
            case 'r':
                return checkKeyword(2, 2, "ue", TOKEN_TRUE);
            }
        }
        break;
    case 'v':
        return checkKeyword(1, 2, "ar", TOKEN_VAR);
    case 'w':
        return checkKeyword(1, 4, "hile", TOKEN_WHILE);
    }

    return TOKEN_IDENTIFIER;
}

/**
 * Scans an identifier or keyword.
 */
static Token identifier()
{
    while (isAlpha(peek()) || isDigit(peek()))
        advance();
    return makeToken(identifierType());
}

/**
 * Skips whitespace and comments.
 */
static void skipWhitespace()
{
    for (;;) {
        char c = peek();
        switch (c) {
        case ' ':
        case '\r':
        case '\t':
            advance();
            break;
        case '\n':
            lexer.line++;
            advance();
            break;
        case '/':
            if (peekNext() == '/') {
                // A comment goes until the end of the line.
                while (peek() != '\n' && !isAtEnd())
                    advance();
            } else {
                return;
            }
            break;
        default:
            return;
        }
    }
}

/**
 * Scans and returns the next token from source.
 */
Token scanToken()
{
    skipWhitespace();
    lexer.start = lexer.current;

    if (isAtEnd())
        return makeToken(TOKEN_EOF);
    char c = advance();
    if (isAlpha(c))
        return identifier();
    if (isDigit(c))
        return number();

    switch (c) {
    case '(':
        return makeToken(TOKEN_LEFT_PAREN);
    case ')':
        return makeToken(TOKEN_RIGHT_PAREN);
    case '{':
        return makeToken(TOKEN_LEFT_BRACE);
    case '}':
        return makeToken(TOKEN_RIGHT_BRACE);
    case ';':
        return makeToken(TOKEN_SEMICOLON);
    case ',':
        return makeToken(TOKEN_COMMA);
    case '.':
        return makeToken(TOKEN_DOT);
    case '-':
        return makeToken(TOKEN_MINUS);
    case '+':
        return makeToken(TOKEN_PLUS);
    case '/':
        return makeToken(TOKEN_SLASH);
    case '*':
        return makeToken(TOKEN_STAR);
    case '!':
        return makeToken(
            match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
    case '=':
        return makeToken(
            match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
    case '<':
        return makeToken(
            match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
    case '>':
        return makeToken(
            match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
    case '"':
        return string();
    }

    return errorToken("Unexpected character.");
}
