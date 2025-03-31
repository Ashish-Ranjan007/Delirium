#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>

#include <algorithm>
#include <string>

#include "chunk.h"
#include "common.h"
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
 * Higher values indicate tighter binding.
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
    Token current;  // Current token being processed
    Token previous; // Previous token processed
    bool hadError;  // Whether an error occurred
    bool panicMode; // Whether we're in error recovery mode
} Parser;

/* Function pointer type for parse rules */
typedef void (*ParseFn)(bool canAssign);

/**
 * Parse rule structure containing prefix/infix handlers
 * and precedence level for Pratt parsing.
 */
typedef struct ParseRule {
    ParseFn prefix;        // Function to handle token when it appears as prefix
    ParseFn infix;         // Function to handle token when it appears as infix
    Precedence precedence; // Operator precedence level
} ParseRule;

/**
 * Represents a local variable in the current scope.
 */
typedef struct Local {
    Token name; // Variable name token
    int depth;  // Nesting depth of the variable (-1 if uninitialized)
} Local;

/**
 * Type of function being compiled.
 */
typedef enum FunctionType {
    TYPE_FUNCTION, // User-defined function
    TYPE_SCRIPT    // Top-level script
} FunctionType;

/**
 * Compiler state for the current function being compiled.
 */
typedef struct Compiler {
    struct Compiler* enclosing; // Compiler for enclosing function (linked list)
    ObjFunction* function;      // Function object being compiled
    FunctionType type;          // Function type
    Local locals[UINT8_COUNT];  // Local variables in this function
    int localCount;             // Number of locals
    int scopeDepth;             // Current block nesting depth
} Compiler;

/* ====================== Global Variables ====================== */

/* Global parser state */
Parser parser;

/* Global compiler state */
Compiler* current = NULL;

/* Current chunk being compiled */
/* Chunk* compilingChunk; */ // Potential Error

/* ====================== Helper Functions ====================== */

/**
 * Gets the current chunk being compiled.
 */
static Chunk* currentChunk()
{
    return &current->function->chunk;
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
 * Reports an the previous token.
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

        if (parser.current.type != TOKEN_ERROR) {
            break; // Stop if it's a valid token.
        }

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

/**
 * Checks if the current token matches the given type.
 */
static bool check(TokenType type)
{
    return parser.current.type == type;
}

/**
 * Consumes the token if it matches the expected type.
 */
static bool match(TokenType type)
{

    if (!check(type))
        return false;
    advance();
    return true;
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
 * Emits a loop instruction with jump offset.
 */
static void emitLoop(int loopStart)
{
    emitByte(OP_LOOP);

    int offset = currentChunk()->count - loopStart + 2;
    if (offset > UINT16_MAX)
        error("Loop body too large.");

    emitByte((offset >> 8) & 0xff);
    emitByte(offset & 0xff);
}

/**
 * Emits a jump instruction and returns the offset to patch later.
 */
static int emitJump(uint8_t instruction)
{
    emitByte(instruction);
    emitByte(0xff);
    emitByte(0xff);
    return currentChunk()->count - 2;
}

/**
 * Emits a return opcode.
 */
static void emitReturn()
{
    emitByte(OP_NIL);
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

/**
 * Patches a previously emitted jump instruction with the correct offset.
 */
static void patchJump(int offset)
{
    // -2 to adjust for the bytecode for the jump offset itself.
    int jump = currentChunk()->count - offset - 2;

    if (jump > UINT16_MAX) {
        error("Too much code to jump over.");
    }

    currentChunk()->code[offset] = (jump >> 8) & 0xff;
    currentChunk()->code[offset + 1] = jump & 0xff;
}

/**
 * Initializes a new compiler for a function.
 */
static void initCompiler(Compiler* compiler, FunctionType type)
{
    compiler->enclosing = current;
    compiler->function = NULL;
    compiler->type = type;
    compiler->localCount = 0;
    compiler->scopeDepth = 0;
    compiler->function = newFunction();
    current = compiler;

    if (type != TYPE_SCRIPT) {
        current->function->name = copyString(parser.previous.start,
            parser.previous.length);
    }

    Local* local = &current->locals[current->localCount++];
    local->depth = 0;
    local->name.start = "";
    local->name.length = 0;
}

/* ====================== Parsing Rules Implementation ====================== */

/* Forward declarations for parsing functions */
static void expression();
static void statement();
static void declaration();
static ParseRule* getRule(TokenType type);
static void parsePrecedence(Precedence precedence);

/**
 * Parses and compiles a numeric literal.
 * Converts the token text into a floating-point number and emits it as a constant.
 */
static void number(bool canAssign)
{
    double value = strtod(parser.previous.start, NULL); // Convert token to a number.
    emitConstant(NUMBER_VAL(value));                    // Emit the constant into the bytecode.
}

/**
 * Parses logical OR expressions.
 */
static void or_(bool canAssign)
{
    int elseJump = emitJump(OP_JUMP_IF_FALSE);
    int endJump = emitJump(OP_JUMP);

    patchJump(elseJump);
    emitByte(OP_POP);

    parsePrecedence(PREC_OR);
    patchJump(endJump);
}

/**
 * Parses logical AND expressions.
 */
static void and_(bool canAssign)
{
    int endJump = emitJump(OP_JUMP_IF_FALSE);

    emitByte(OP_POP);
    parsePrecedence(PREC_AND);

    patchJump(endJump);
}

/**
 * Parses string literals.
 */
static void string(bool canAssign)
{
    emitConstant(OBJ_VAL(copyString(parser.previous.start + 1, parser.previous.length - 2)));
}

/**
 * Creates a constant for an identifier name.
 */
static uint8_t identifierConstant(Token* name)
{
    return makeConstant(OBJ_VAL(copyString(name->start,
        name->length)));
}

/**
 * Checks if two identifiers are equal.
 */
static bool identifiersEqual(Token* a, Token* b)
{
    if (a->length != b->length)
        return false;
    return memcmp(a->start, b->start, a->length) == 0;
}

/**
 * Resolves a local variable by name.
 */
static int resolveLocal(Compiler* compiler, Token* name)
{
    for (int i = compiler->localCount - 1; i >= 0; i--) {
        Local* local = &compiler->locals[i];
        if (identifiersEqual(name, &local->name)) {
            if (local->depth == -1) {
                error("Can't read local variable in its own initializer.");
            }
            return i;
        }
    }

    return -1;
}

/**
 * Adds a new local variable to the current scope.
 */
static void addLocal(Token name)
{
    if (current->localCount == UINT8_COUNT) {
        error("Too many local variables in function.");
        return;
    }

    Local* local = &current->locals[current->localCount++];
    local->name = name;
    local->depth = -1;
}

/**
 * Declares a new variable in the current scope.
 */
static void declareVariable()
{
    if (current->scopeDepth == 0)
        return;
    Token* name = &parser.previous;

    for (int i = current->localCount - 1; i >= 0; i--) {
        Local* local = &current->locals[i];
        if (local->depth != -1 && local->depth < current->scopeDepth) {
            break;
        }

        if (identifiersEqual(name, &local->name)) {
            error("Already a variable with this name in this scope.");
        }
    }

    addLocal(*name);
}

/**
 * Parses an argument list for function calls.
 */
static uint8_t argumentList()
{
    uint8_t argCount = 0;
    if (!check(TOKEN_RIGHT_PAREN)) {
        do {
            expression();

            if (argCount == 255) {
                error("Can't have more than 255 arguments.");
            }

            argCount++;
        } while (match(TOKEN_COMMA));
    }
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after arguments.");
    return argCount;
}

/**
 * Parses a named variable reference or assignment.
 */
static void namedVariable(Token name, bool canAssign)
{
    uint8_t getOp, setOp;
    int arg = resolveLocal(current, &name);
    if (arg != -1) {
        getOp = OP_GET_LOCAL;
        setOp = OP_SET_LOCAL;
    } else {
        arg = identifierConstant(&name);
        getOp = OP_GET_GLOBAL;
        setOp = OP_SET_GLOBAL;
    }

    if (canAssign && match(TOKEN_EQUAL)) {
        expression();
        emitBytes(setOp, (uint8_t)arg);
    } else {
        emitBytes(getOp, (uint8_t)arg);
    }
}

/**
 * Parses a variable reference.
 */
static void variable(bool canAssign)
{
    namedVariable(parser.previous, canAssign);
}

/**
 * Parses and compiles a grouping expression enclosed in parentheses.
 * Example: (1 + 2) is a grouping expression.
 */
static void grouping(bool canAssign)
{
    expression();                                               // Parse the expression inside the parentheses.
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression."); // Ensure proper closure.
}

/**
 * Parses a unary operator (e.g., `-`, `!`) and its operand.
 * Emits bytecode for the operator and compiles the operand.
 */
static void unary(bool canAssign)
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

/**
 * Parses literal values (true, false, nil).
 */
static void literal(bool canAssign)
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
static void binary(bool canAssign)
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

/**
 * Parses function calls.
 */
static void call(bool canAssign)
{
    uint8_t argCount = argumentList();
    emitBytes(OP_CALL, argCount);
}

/* ====================== Parse Rule Table ====================== */

/**
 * The parse rule table mapping token types to their parsing rules.
 */
ParseRule rules[] = {
    [TOKEN_LEFT_PAREN] = { grouping, call, PREC_CALL },
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
    [TOKEN_IDENTIFIER] = { variable, NULL, PREC_NONE },
    [TOKEN_STRING] = { string, NULL, PREC_NONE },
    [TOKEN_NUMBER] = { number, NULL, PREC_NONE },
    [TOKEN_AND] = { NULL, and_, PREC_AND },
    [TOKEN_CLASS] = { NULL, NULL, PREC_NONE },
    [TOKEN_ELSE] = { literal, NULL, PREC_NONE },
    [TOKEN_FALSE] = { NULL, NULL, PREC_NONE },
    [TOKEN_FOR] = { NULL, NULL, PREC_NONE },
    [TOKEN_FUN] = { NULL, NULL, PREC_NONE },
    [TOKEN_IF] = { NULL, NULL, PREC_NONE },
    [TOKEN_NIL] = { literal, NULL, PREC_NONE },
    [TOKEN_OR] = { NULL, or_, PREC_OR },
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

static ObjFunction* endCompiler();

/**
 * Parses a variable declaration.
 */
static uint8_t parseVariable(char const* errorMessage)
{
    consume(TOKEN_IDENTIFIER, errorMessage);

    declareVariable();
    if (current->scopeDepth > 0)
        return 0;

    return identifierConstant(&parser.previous);
}

/**
 * Marks the current local variable as initialized.
 */
static void markInitialized()
{
    if (current->scopeDepth == 0)
        return;
    current->locals[current->localCount - 1].depth = current->scopeDepth;
}

/**
 * Defines a variable in the current scope.
 */
static void defineVariable(uint8_t global)
{
    if (current->scopeDepth > 0) {
        markInitialized();
        return;
    }
    emitBytes(OP_DEFINE_GLOBAL, global);
}

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

    // prefixRule(); // Call the prefix function (e.g., number() for literals).
    bool canAssign = precedence <= PREC_ASSIGNMENT;
    prefixRule(canAssign);

    // Continue parsing while the next operator has equal or higher precedence.
    while (precedence <= getRule(parser.current.type)->precedence) {
        advance();                                                // Move to the next token.
        ParseFn infixRule = getRule(parser.previous.type)->infix; // Get infix rule.
        infixRule(canAssign);                                     // Call the infix function (e.g., binary() for `+`, `*`).
    }

    if (canAssign && match(TOKEN_EQUAL)) {
        error("Invalid assignment target.");
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

/* ====================== Scope Management ====================== */

/**
 * Enters a new scope.
 */
static void beginScope()
{
    current->scopeDepth++;
}

/**
 * Exits the current scope.
 */
static void endScope()
{
    current->scopeDepth--;

    while (current->localCount > 0 && current->locals[current->localCount - 1].depth > current->scopeDepth) {
        emitByte(OP_POP);
        current->localCount--;
    }
}

/* ====================== Statement Parsing ====================== */

/**
 * Parses a block statement.
 */
static void block()
{
    while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
        declaration();
    }

    consume(TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

/**
 * Parses a function declaration.
 */
static void function(FunctionType type)
{
    Compiler compiler;
    initCompiler(&compiler, type);
    beginScope();

    consume(TOKEN_LEFT_PAREN, "Expect '(' after function name.");

    if (!check(TOKEN_RIGHT_PAREN)) {
        do {
            current->function->arity++;
            if (current->function->arity > 255) {
                errorAtCurrent("Can't have more than 255 parameters.");
            }
            uint8_t constant = parseVariable("Expect parameter name.");
            defineVariable(constant);
        } while (match(TOKEN_COMMA));
    }

    consume(TOKEN_RIGHT_PAREN, "Expect ')' after parameters.");
    consume(TOKEN_LEFT_BRACE, "Expect '{' before function body.");
    block();

    ObjFunction* function = endCompiler();
    emitBytes(OP_CONSTANT, makeConstant(OBJ_VAL(function)));
}

/**
 * Parses a function declaration statement.
 */
static void funDeclaration()
{
    uint8_t global = parseVariable("Expect function name");
    markInitialized();
    function(TYPE_FUNCTION);
    defineVariable(global);
}

/**
 * Parses a variable declaration statement.
 */
static void varDeclaration()
{
    uint8_t global = parseVariable("Expect variable name.");

    if (match(TOKEN_EQUAL)) {
        expression();
    } else {
        emitByte(OP_NIL);
    }
    consume(TOKEN_SEMICOLON,
        "Expect ';' after variable declaration.");

    defineVariable(global);
}

/**
 * Parses an expression statement.
 */
static void expressionStatement()
{
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
    emitByte(OP_POP);
}

/**
 * Parses a for loop statement.
 */
static void forStatement()
{
    beginScope();

    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'for'.");

    if (match(TOKEN_SEMICOLON)) {
        // No initializer.
    } else if (match(TOKEN_VAR)) {
        varDeclaration();
    } else {
        expressionStatement();
    }

    int loopStart = currentChunk()->count;
    int exitJump = -1;

    if (!match(TOKEN_SEMICOLON)) {
        expression();
        consume(TOKEN_SEMICOLON, "Expect ';' after loop condition.");

        // Jump out of the loop if the condition is false.
        exitJump = emitJump(OP_JUMP_IF_FALSE);
        emitByte(OP_POP); // Condition.
    }

    if (!match(TOKEN_RIGHT_PAREN)) {
        int bodyJump = emitJump(OP_JUMP);
        int incrementStart = currentChunk()->count;
        expression();
        emitByte(OP_POP);
        consume(TOKEN_RIGHT_PAREN, "Expect ')' after for clauses.");

        emitLoop(loopStart);
        loopStart = incrementStart;
        patchJump(bodyJump);
    }

    statement();
    emitLoop(loopStart);

    if (exitJump != -1) {
        patchJump(exitJump);
        emitByte(OP_POP); // Condition.
    }

    endScope();
}

/**
 * Parses an if statement.
 */
static void ifStatement()
{
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

    int thenJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP); // Could be error
    statement();

    int elseJump = emitJump(OP_JUMP);

    patchJump(thenJump);
    emitByte(OP_POP); // Could be error

    if (match(TOKEN_ELSE))
        statement();
    patchJump(elseJump);
}

/**
 * Parses a print statement.
 */
static void printStatement()
{
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after value.");
    emitByte(OP_PRINT);
}

/**
 * Parses a return statement.
 */
static void returnStatement()
{
    if (current->type == TYPE_SCRIPT) {
        error("Can't return from top-level code.");
    }

    if (match(TOKEN_SEMICOLON)) {
        emitReturn();
    } else {
        expression();
        consume(TOKEN_SEMICOLON, "Expect ';' after return value.");
        emitByte(OP_RETURN);
    }
}

/**
 * Parses a while statement.
 */
static void whileStatement()
{
    int loopStart = currentChunk()->count;
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

    int exitJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);
    statement();
    emitLoop(loopStart);

    patchJump(exitJump);
    emitByte(OP_POP);
}

/**
 * Synchronizes the parser after an error.
 */
static void synchronize()
{
    parser.panicMode = false;

    while (parser.current.type != TOKEN_EOF) {
        if (parser.previous.type == TOKEN_SEMICOLON)
            return;
        switch (parser.current.type) {
        case TOKEN_CLASS:
        case TOKEN_FUN:
        case TOKEN_VAR:
        case TOKEN_FOR:
        case TOKEN_IF:
        case TOKEN_WHILE:
        case TOKEN_PRINT:
        case TOKEN_RETURN:
            return;

        default:; // Do nothing.
        }

        advance();
    }
}

/**
 * Parses a statement.
 */
static void statement()
{
    if (match(TOKEN_PRINT)) {
        printStatement();
    } else if (match(TOKEN_FOR)) {
        forStatement();
    } else if (match(TOKEN_IF)) {
        ifStatement();
    } else if (match(TOKEN_RETURN)) {
        returnStatement();
    } else if (match(TOKEN_WHILE)) {
        whileStatement();
    } else if (match(TOKEN_LEFT_BRACE)) {
        beginScope();
        block();
        endScope();
    } else {
        expressionStatement();
    }
}

/**
 * Parses a declaration (variable, function, or statement).
 */
static void declaration()
{
    if (match(TOKEN_FUN)) {
        funDeclaration();
    } else if (match(TOKEN_VAR)) {
        varDeclaration();
    } else {
        statement();
    }

    if (parser.panicMode)
        synchronize();
}

/* ====================== Compiler Interface ====================== */

/**
 * Finalizes the compilation process by emitting a return instruction
 * and optionally disassembling the generated code.
 */
static ObjFunction* endCompiler()
{
    emitReturn();
    ObjFunction* function = current->function;

#ifdef DEBUG_PRINT_CODE
    if (!parser.hadError) {
        disassembleChunk(currentChunk(), function->name != NULL ? function->name->chars : "<script>");
    }
#endif

    current = current->enclosing;
    return function;
}

/**
 * Compiles the given source code into bytecode.
 *
 * @param source The source code string.
 * @param chunk The chunk where compiled bytecode will be stored.
 * @return true if compilation is successful, false if there are errors.
 */
ObjFunction* compile(char const* source)
{
    initLexer(source); // Initialize the lexer with the source code.
    Compiler compiler;
    initCompiler(&compiler, TYPE_SCRIPT);
    // compilingChunk = chunk; // Set the chunk where compiled bytecode will be stored.

    parser.hadError = false;  // Reset error state before compilation starts.
    parser.panicMode = false; // Reset panic mode to handle errors gracefully.

    advance(); // Fetch the first token from the lexer.

    while (!match(TOKEN_EOF)) { // Keep compiling declarations until EOF is hit
        declaration();
    }

    ObjFunction* function = endCompiler();
    return parser.hadError ? NULL : function;
}
