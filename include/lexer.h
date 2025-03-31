#ifndef LEXER_H
#define LEXER_H

/**
 * Enumeration of all token types recognized by the Delirium lexer.
 *
 * Covers:
 * - Single-character tokens (e.g., '+', '(')
 * - Multi-character tokens (e.g., '!=', '==')
 * - Literals (identifiers, strings, numbers)
 * - Keywords (language reserved words)
 * - Special tokens (error, EOF)
 */
typedef enum TokenType {
    // Single-character tokens
    TOKEN_LEFT_PAREN,  // '('
    TOKEN_RIGHT_PAREN, // ')'
    TOKEN_LEFT_BRACE,  // '{'
    TOKEN_RIGHT_BRACE, // '}'
    TOKEN_COMMA,       // ','
    TOKEN_DOT,         // '.'
    TOKEN_MINUS,       // '-'
    TOKEN_PLUS,        // '+'
    TOKEN_SEMICOLON,   // ';'
    TOKEN_SLASH,       // '/'
    TOKEN_STAR,        // '*'

    // One or two character tokens
    TOKEN_BANG,          // '!'
    TOKEN_BANG_EQUAL,    // '!='
    TOKEN_EQUAL,         // '='
    TOKEN_EQUAL_EQUAL,   // '=='
    TOKEN_GREATER,       // '>'
    TOKEN_GREATER_EQUAL, // '>='
    TOKEN_LESS,          // '<'
    TOKEN_LESS_EQUAL,    // '<='

    // Literals
    TOKEN_IDENTIFIER, // variable names
    TOKEN_STRING,     // string literals
    TOKEN_NUMBER,     // numeric literals

    // Keywords
    TOKEN_AND,    // 'and'
    TOKEN_CLASS,  // 'class'
    TOKEN_ELSE,   // 'else'
    TOKEN_FALSE,  // 'false'
    TOKEN_FOR,    // 'for'
    TOKEN_FUN,    // 'fun'
    TOKEN_IF,     // 'if'
    TOKEN_NIL,    // 'nil'
    TOKEN_OR,     // 'or'
    TOKEN_PRINT,  // 'print'
    TOKEN_RETURN, // 'return'
    TOKEN_SUPER,  // 'super'
    TOKEN_THIS,   // 'this'
    TOKEN_TRUE,   // 'true'
    TOKEN_VAR,    // 'var'
    TOKEN_WHILE,  // 'while'

    // Special tokens
    TOKEN_ERROR, // Error token
    TOKEN_EOF    // End-of-file marker
} TokenType;

/**
 * Represents a lexical token with metadata.
 *
 * Contains:
 * - Token type (from TokenType enum)
 * - Pointer to token text in source
 * - Length of token text
 * - Source line number for error reporting
 */
typedef struct Token {
    TokenType type;    // Type of token
    char const* start; // Pointer to token start in source
    int length;        // Token length in bytes
    int line;          // Source line number (1-based)
} Token;

// Lexer state structure
typedef struct Lexer {
    char const* start;   // Start of current token being scanned
    char const* current; // Current position in source
    int line;            // Current line number (1-based)
    char const* source;
} Lexer;

// Change from extern variable to function access
char const* getLexer(); // Add this declaration

/**
 * Initializes the lexer with source code to tokenize.
 *
 * @param source Null-terminated source code string
 *
 * @note The source string must remain valid during tokenization
 */
void initLexer(char const* source);

/**
 * Scans and returns the next token from the source.
 *
 * @return Next Token in the input stream
 *
 * @note For TOKEN_ERROR, start points to error message
 * @note Advances internal lexer state
 */
Token scanToken();

#endif // LEXER_H