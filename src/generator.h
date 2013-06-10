#ifndef GENERATOR_H
#define GENERATOR_H

// Include parse for the parse tree type
// which is referenced in this file
#include "src/parser.h"
#include "src/lib/vector.h"

// Represents a VM instruction.
struct instruction {
    int opcode;
    char *opcodeName;
    int lexicalLevel;
    int modifier;
};

// A symbol can be a variable name or a procedure name. We need to keep track
// of its lexical level so we know what code can access it, and we need to keep
// track of its address so we can load its value.
struct symbol {
    char *name;
    int type;      // Whether the symbol is of a variable, a constant, or a procedure.
    int level;     // The lexical level of the symbol.
    int address;   // The address of the symbol on the stack, in it's lexical
                   // level, or the address in the code if it's a procedure.
    int constantValue;     // If it's a constant, holds the value of the constant.
};
// Symbol types
enum { VARIABLE = 1, CONSTANT, PROCEDURE };

// Used in the generate function to keep track of the current state.
struct generatorState {
    struct vector *symbols;   // The symbol table.
    int currentAddress;   // The current code address.
    int currentLevel;     // The current lexical level.
    struct vector *instructions;   // The instructions that have been generated so far.
};

// generateInstructions is just a wrapper for generate that initializes the
// generatorState for you. Use it instead of using generate directly.
struct vector *generateInstructions(struct parseTree tree);

// Given a parse tree, generate a list of VM instructions.
void generate(struct parseTree tree, struct generatorState *state);
void generate_program(struct parseTree tree, struct generatorState *state);
void generate_block(struct parseTree tree, struct generatorState *state);
void generate_varDeclaration(struct parseTree tree, struct generatorState *state);
void generate_vars(struct parseTree tree, struct generatorState *state);
void generate_var(struct parseTree tree, struct generatorState *state);
void generate_constDeclaration(struct parseTree tree, struct generatorState *state);
void generate_constants(struct parseTree tree, struct generatorState *state);
void generate_constant(struct parseTree tree, struct generatorState *state);
void generate_statement(struct parseTree tree, struct generatorState *state);
void generate_statements(struct parseTree tree, struct generatorState *state);
void generate_assignment(struct parseTree tree, struct generatorState *state);
void generate_beginBlock(struct parseTree tree, struct generatorState *state);
void generate_readStatement(struct parseTree tree, struct generatorState *state);
void generate_writeStatement(struct parseTree tree, struct generatorState *state);
void generate_ifStatement(struct parseTree tree, struct generatorState *state);
void generate_whileStatement(struct parseTree tree, struct generatorState *state);
void generate_condition(struct parseTree tree, struct generatorState *state);
void generate_relationalOperator(struct parseTree tree, struct generatorState *state);
void generate_expression(struct parseTree tree, struct generatorState *state);
void generate_addOrSubtract(struct parseTree tree, struct generatorState *state);
void generate_term(struct parseTree tree, struct generatorState *state);
void generate_multiplyOrDivide(struct parseTree tree, struct generatorState *state);
void generate_factor(struct parseTree tree, struct generatorState *state);
void generate_sign(struct parseTree tree, struct generatorState *state);
void generate_number(struct parseTree tree, struct generatorState *state);
void generate_identifier(struct parseTree tree, struct generatorState *state);

struct generatorState *makeGeneratorState();
struct generatorState *copyGeneratorState(struct generatorState *state);
void addInstruction(struct generatorState *state, char *instruction, int level, int modifier);
void addLoadInstruction(struct generatorState *state, struct parseTree identifierTree);
void addStoreInstruction(struct generatorState *state, struct parseTree identifierTree);

// Given a string represtation of an instruction, such as "lit" or "sto",
// return the corresponding integer opcode.
int getOpcode(char *instruction);

// Utility function to initialize a struct instruction.
struct instruction makeInstruction(char *instruction, int lexicalLevel, int modifier);

// Add and get a symbol from the symbol table.
void addVariable(struct generatorState *state, struct parseTree identifierTree);
void addConstant(struct generatorState *state, struct parseTree identifierTree,
        struct parseTree numberTree);
struct symbol getSymbol(struct generatorState *state, char *name);

// Get and set an error in case a function returns a failure value.
void addGeneratorError(char *errorMessage);
int generatorHasErrors();
char *printGeneratorErrors();

#endif
