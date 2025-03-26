#include <cstdlib>
#include <iostream>

#include "chunk.h"
#include "compiler.h"
#include "debug.h"
#include "lexer.h"
#include "object.h"
#include "value.h"

#ifdef DEBUG_PRINT_CODE
#    include "debug.h"
#endif

/* ====================== Parser Types and State ====================== */

/**
 * Operator precedence levels used for Pratt parsing.
 */
typedef enum Precedence {
    PREC_NONE,
    PREC_ASSIGNMENT, // =
    PREC_OR,         // or
    PREC_AND,        // and
    PREC_EQUALITY,   // == !=
    PREC_COMPARISON, // < > <= >=
    PREC_TERM,       // + -
    PREC_FACTOR,     // * /
    PREC_UNARY,      // ! -
    PREC_CALL,       // . ()
    PREC_PRIMARY
} Precedence;

/**
 * Parser state tracking current and previous tokens,
 * error status, and panic mode for error recovery.
 */
typedef struct Parser {
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;
} Parser;

/* Function pointer type for parse rules */
typedef void (*ParseFn)();

/**
 * Parse rule structure containing prefix/infix handlers
 * and precedence level for Pratt parsing.
 */
typedef struct ParseRule {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

/* ====================== Global Variables ====================== */

/* Global parser state */
Parser parser;

/* Current chunk being compiled */
Chunk* compilingChunk;

/* ====================== Helper Functions ====================== */

/**
 * Gets the current chunk being compiled.
 */
static Chunk* currentChunk()
{
    return compilingChunk;
}

/* ====================== Error Handling ====================== */

/**
 * Reports an error at a specific token with a message.
 */
static void errorAt(Token* token, char const* message)
{
    if (parser.panicMode)
        return;
    parser.panicMode = true;
    std::cerr << "[line " << token->line << "] Error";

    if (token->type == TOKEN_EOF) {
        std::cout << " at end";
    } else if (token->type == TOKEN_ERROR) {
        // Nothing
    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    std::cerr << ": " << message << std::endl;
    parser.hadError = true;
}

/**
 * Reports an error at the previous token.
 */
static void error(char const* message)
{
    errorAt(&parser.previous, message);
}

/**
 * Reports an error at the current token.
 */
static void errorAtCurrent(char const* message)
{
    errorAt(&parser.current, message);
}

/* ====================== Token Processing ====================== */

/**
 * Advances the lexer to the next token, skipping any invalid tokens.
 * Updates `parser.previous` with the last valid token.
 */
static void advance()
{
    parser.previous = parser.current; // Store the previous token.

    for (;;) {                        // Keep advancing until a valid token is found.
        parser.current = scanToken(); // Fetch the next token from the source.

        if (parser.current.type != TOKEN_ERROR)
            break; // Stop if it's a valid token.

        errorAtCurrent(parser.current.start); // Report an error if a token is invalid.
    }
}

/**
 * Consumes the expected token or reports an error if the token is missing.
 *
 * @param type The expected token type.
 * @param message Error message to display if the expected token is not found.
 */
static void consume(TokenType type, char const* message)
{
    if (parser.current.type == type) {
        advance(); // If the token matches, move to the next token.
        return;
    }

    errorAtCurrent(message); // Report an error if the expected token is missing.
}

/* ====================== Bytecode Emission ====================== */

/**
 * Emits a single byte of bytecode into the current chunk.
 *
 * @param byte The opcode or operand to emit.
 */
static void emitByte(uint8_t byte)
{
    writeChunk(currentChunk(), byte, parser.previous.line); // Append the byte to the current chunk.
}

/**
 * Emits two bytes of bytecode sequentially.
 */
static void emitBytes(uint8_t byte1, uint8_t byte2)
{
    emitByte(byte1);
    emitByte(byte2);
}

/**
 * Emits a return opcode.
 */
static void emitReturn()
{
    emitByte(OP_RETURN);
}

/**
 * Creates a constant value in the constant pool.
 * @return The constant index or 0 if too many constants exist.
 */
static uint8_t makeConstant(Value value)
{
    int constant = addConstant(currentChunk(), value);
    if (constant > UINT8_MAX) {
        error("Too many constants in one chunk");
        return 0;
    }

    return (uint8_t)constant;
}

/**
 * Emits an opcode followed by a constant value.
 *
 * @param value The constant value to emit.
 */
static void emitConstant(Value value)
{
    emitBytes(OP_CONSTANT, makeConstant(value));
}

/* ====================== Parsing Rules Implementation ====================== */

/* Forward declarations for parsing functions */
static void expression();
static ParseRule* getRule(TokenType type);
static void parsePrecedence(Precedence precedence);

/**
 * Parses and compiles a numeric literal.
 * Converts the token text into a floating-point number and emits it as a constant.
 */
static void number()
{
    double value = strtod(parser.previous.start, NULL); // Convert token to a number.
    emitConstant(NUMBER_VAL(value));                    // Emit the constant into the bytecode.
}

static void string()
{
    emitConstant(OBJ_VAL(copyString(parser.previous.start + 1, parser.previous.length - 2)));
}

/**
 * Parses and compiles a grouping expression enclosed in parentheses.
 * Example: (1 + 2) is a grouping expression.
 */
static void grouping()
{
    expression();                                               // Parse the expression inside the parentheses.
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression."); // Ensure proper closure.
}

/**
 * Parses a unary operator (e.g., `-`, `!`) and its operand.
 * Emits bytecode for the operator and compiles the operand.
 */
static void unary()
{
    TokenType operatorType = parser.previous.type; // Get the unary operator.

    parsePrecedence(PREC_UNARY); // Parse the operand at unary precedence.

    // Generate bytecode for the unary operation.
    switch (operatorType) {
    case TOKEN_BANG:
        emitByte(OP_NOT);
        break;
    case TOKEN_MINUS:
        emitByte(OP_NEGATE);
        break;
    default:
        return; // Should never be reached.
    }
}

static void literal()
{
    switch (parser.previous.type) {
    case TOKEN_FALSE:
        emitByte(OP_FALSE);
        break;
    case TOKEN_NIL:
        emitByte(OP_NIL);
        break;
    case TOKEN_TRUE:
        emitByte(OP_TRUE);
        break;
    default:
        return; // Unreachable.
    }
}

/**
 * Parses a binary operator (e.g., `+`, `-`, `*`, `/`) and its right-hand side expression.
 */
static void binary()
{
    TokenType operatorType = parser.previous.type; // Get the operator token.

    // Get the precedence level for the operator and parse the right-hand side.
    Precedence nextPrecedence = (Precedence)(getRule(operatorType)->precedence + 1);
    parsePrecedence(nextPrecedence); // Ensure correct precedence order.

    // Generate bytecode for the operator.
    switch (operatorType) {
    case TOKEN_BANG_EQUAL:
        emitBytes(OP_EQUAL, OP_NOT);
        break;
    case TOKEN_EQUAL_EQUAL:
        emitByte(OP_EQUAL);
        break;
    case TOKEN_GREATER:
        emitByte(OP_GREATER);
        break;
    case TOKEN_GREATER_EQUAL:
        emitBytes(OP_LESS, OP_NOT);
        break;
    case TOKEN_LESS:
        emitByte(OP_LESS);
        break;
    case TOKEN_LESS_EQUAL:
        emitBytes(OP_GREATER, OP_NOT);
        break;
    case TOKEN_PLUS:
        emitByte(OP_ADD);
        break;
    case TOKEN_MINUS:
        emitByte(OP_SUBTRACT);
        break;
    case TOKEN_STAR:
        emitByte(OP_MULTIPLY);
        break;
    case TOKEN_SLASH:
        emitByte(OP_DIVIDE);
        break;
    default:
        return; // This should never be reached.
    }
}

/* ====================== Parse Rule Table ====================== */

/**
 * The parse rule table mapping token types to their parsing rules.
 */
ParseRule rules[] = {
    [TOKEN_LEFT_PAREN] = { grouping, NULL, PREC_NONE },
    [TOKEN_RIGHT_PAREN] = { NULL, NULL, PREC_NONE },
    [TOKEN_LEFT_BRACE] = { NULL, NULL, PREC_NONE },
    [TOKEN_RIGHT_BRACE] = { NULL, NULL, PREC_NONE },
    [TOKEN_COMMA] = { NULL, NULL, PREC_NONE },
    [TOKEN_DOT] = { NULL, NULL, PREC_NONE },
    [TOKEN_MINUS] = { unary, binary, PREC_TERM },
    [TOKEN_PLUS] = { NULL, binary, PREC_TERM },
    [TOKEN_SEMICOLON] = { NULL, NULL, PREC_NONE },
    [TOKEN_SLASH] = { NULL, binary, PREC_FACTOR },
    [TOKEN_STAR] = { NULL, binary, PREC_FACTOR },
    [TOKEN_BANG] = { unary, NULL, PREC_NONE },
    [TOKEN_BANG_EQUAL] = { NULL, binary, PREC_EQUALITY },
    [TOKEN_EQUAL] = { NULL, NULL, PREC_NONE },
    [TOKEN_EQUAL_EQUAL] = { NULL, binary, PREC_EQUALITY },
    [TOKEN_GREATER] = { NULL, binary, PREC_COMPARISON },
    [TOKEN_GREATER_EQUAL] = { NULL, binary, PREC_COMPARISON },
    [TOKEN_LESS] = { NULL, binary, PREC_COMPARISON },
    [TOKEN_LESS_EQUAL] = { NULL, binary, PREC_COMPARISON },
    [TOKEN_IDENTIFIER] = { NULL, NULL, PREC_NONE },
    [TOKEN_STRING] = { string, NULL, PREC_NONE },
    [TOKEN_NUMBER] = { number, NULL, PREC_NONE },
    [TOKEN_AND] = { NULL, NULL, PREC_NONE },
    [TOKEN_CLASS] = { NULL, NULL, PREC_NONE },
    [TOKEN_ELSE] = { literal, NULL, PREC_NONE },
    [TOKEN_FALSE] = { NULL, NULL, PREC_NONE },
    [TOKEN_FOR] = { NULL, NULL, PREC_NONE },
    [TOKEN_FUN] = { NULL, NULL, PREC_NONE },
    [TOKEN_IF] = { NULL, NULL, PREC_NONE },
    [TOKEN_NIL] = { literal, NULL, PREC_NONE },
    [TOKEN_OR] = { NULL, NULL, PREC_NONE },
    [TOKEN_PRINT] = { NULL, NULL, PREC_NONE },
    [TOKEN_RETURN] = { NULL, NULL, PREC_NONE },
    [TOKEN_SUPER] = { NULL, NULL, PREC_NONE },
    [TOKEN_THIS] = { NULL, NULL, PREC_NONE },
    [TOKEN_TRUE] = { literal, NULL, PREC_NONE },
    [TOKEN_VAR] = { NULL, NULL, PREC_NONE },
    [TOKEN_WHILE] = { NULL, NULL, PREC_NONE },
    [TOKEN_ERROR] = { NULL, NULL, PREC_NONE },
    [TOKEN_EOF] = { NULL, NULL, PREC_NONE },
};

/**
 * Retrieves the parsing rule associated with a given token type.
 *
 * @param type The token type to retrieve the rule for.
 * @return A pointer to the ParseRule structure containing prefix, infix, and precedence.
 */
static ParseRule* getRule(TokenType type)
{
    return &rules[type]; // Lookup and return the parsing rule for the token type.
}

/* ====================== Expression Parsing ====================== */

/**
 * Parses an expression while respecting operator precedence.
 *
 * @param precedence The precedence level to start parsing at.
 */
static void parsePrecedence(Precedence precedence)
{
    advance(); // Fetch the next token, which should be the start of an expression.

    // Get the prefix rule (handling numbers, variables, unary operators, etc.).
    ParseFn prefixRule = getRule(parser.previous.type)->prefix;
    if (prefixRule == NULL) {
        error("Expect expression."); // If no prefix rule is found, report an error.
        return;
    }

    prefixRule(); // Call the prefix function (e.g., number() for literals).

    // Continue parsing while the next operator has equal or higher precedence.
    while (precedence <= getRule(parser.current.type)->precedence) {
        advance();                                                // Move to the next token.
        ParseFn infixRule = getRule(parser.previous.type)->infix; // Get infix rule.
        infixRule();                                              // Call the infix function (e.g., binary() for `+`, `*`).
    }
}

/**
 * Parses and compiles an expression.
 * Uses `parsePrecedence` to handle different levels of operator precedence.
 */
static void expression()
{
    parsePrecedence(PREC_ASSIGNMENT); // Start parsing at the lowest precedence level.
}

/* ====================== Compiler Interface ====================== */

/**
 * Finalizes the compilation process by emitting a return instruction
 * and optionally disassembling the generated code.
 */
static void endCompiler()
{
    emitReturn();
#ifdef DEBUG_PRINT_CODE
    if (!parser.hadError) {
        disassembleChunk(currentChunk(), "code");
    }
#endif
}

/**
 * Compiles the given source code into bytecode.
 *
 * @param source The source code string.
 * @param chunk The chunk where compiled bytecode will be stored.
 * @return true if compilation is successful, false if there are errors.
 */
bool compile(char const* source, Chunk* chunk)
{
    initLexer(source);      // Initialize the lexer with the source code.
    compilingChunk = chunk; // Set the chunk where compiled bytecode will be stored.

    parser.hadError = false;  // Reset error state before compilation starts.
    parser.panicMode = false; // Reset panic mode to handle errors gracefully.

    advance();                                       // Fetch the first token from the lexer.
    expression();                                    // Parse and compile an expression.
    consume(TOKEN_EOF, "Expect end of expression."); // Ensure the entire source is parsed.
    endCompiler();                                   // Finalize the compilation process.

    return !parser.hadError; // Return success or failure status.
}
