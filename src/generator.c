#include "src/generator.h"
#include "src/parser.h"
#include "src/lib/util.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

struct vector *generateInstructions(struct parseTree tree) {
    extern struct vector *generatorErrors;
    generatorErrors = NULL;

    struct generatorState *state = makeGeneratorState();
    generate(tree, state);
    return state->instructions;
}

void generate(struct parseTree tree, struct generatorState *state) {
    // Don't generate anything if the tree is invalid.
    if (isParseTreeError(tree))
        return;

    // tree.name == NULL will cause the strcmp's to segfault.
    assert(tree.name != NULL);

    int is(char *name) {
        return (strcmp(tree.name, name) == 0);
    }
    void call(void (*generateFunction)(struct parseTree, struct generatorState*)) {
        (*generateFunction)(tree, state);
    }

    if (is("program")) call(generate_program);
    else if (is("block")) call(generate_block);
    else if (is("var-declaration")) call(generate_varDeclaration);
    else if (is("vars")) call(generate_vars);
    else if (is("var")) call(generate_var);
    else if (is("const-declaration")) call(generate_constDeclaration);
    else if (is("constants")) call(generate_constants);
    else if (is("constant")) call(generate_constant);
    else if (is("statement")) call(generate_statement);
    else if (is("assignment")) call(generate_assignment);
    else if (is("begin-block")) call(generate_beginBlock);
    else if (is("statements")) call(generate_statements);
    else if (is("read-statement")) call(generate_readStatement);
    else if (is("write-statement")) call(generate_writeStatement);
    else if (is("if-statement")) call(generate_ifStatement);
    else if (is("while-statement")) call(generate_whileStatement);
    else if (is("condition")) call(generate_condition);
    else if (is("rel-op")) call(generate_relationalOperator);
    else if (is("expression")) call(generate_expression);
    else if (is("add-or-subtract")) call(generate_addOrSubtract);
    else if (is("term")) call(generate_term);
    else if (is("multiply-or-divide")) call(generate_multiplyOrDivide);
    else if (is("factor")) call(generate_factor);
    else if (is("sign")) call(generate_sign);
    else if (is("number")) call(generate_number);
    else if (is("identifier")) call(generate_identifier);
}

void generate_program(struct parseTree tree, struct generatorState *state) {
    assert(hasChild(tree, "block"));

    generate(getChild(tree, "block"), state);
    // Add a return instruction at the end of the program.
    addInstruction(state, "opr", 0, 0);
}

void generate_block(struct parseTree tree, struct generatorState *state) {
    assert(hasChild(tree, "statement"));

    generate(getChild(tree, "var-declaration"), state);
    generate(getChild(tree, "const-declaration"), state);
    generate(getChild(tree, "statement"), state);
}

void generate_varDeclaration(struct parseTree tree, struct generatorState *state) {
    if (hasChild(tree, "vars")) {
        struct generatorState *fakeState = copyGeneratorState(state);
        generate(getChild(tree, "vars"), fakeState);
        // Each "var" node adds one instruction, so we can use that to find the
        // number of variables.
        int numVariables = fakeState->instructions->length - state->instructions->length;

        // Allocate space for the variables.
        addInstruction(state, "inc", 0, numVariables);
        // Ignore the instructions that the vars generate, but keep the symbols
        // that they added.
        state->symbols = fakeState->symbols;
    }
}

void generate_vars(struct parseTree tree, struct generatorState *state) {
    assert(hasChild(tree, "var"));

    generate(getChild(tree, "var"), state);
    // If there is no "vars" child, generate will just return without
    // adding any instructions.
    generate(getChild(tree, "vars"), state);
}

void generate_var(struct parseTree tree, struct generatorState *state) {
    assert(hasChild(tree, "identifier"));

    addVariable(state, getChild(tree, "identifier"));
    // Add a fake instruction so that generate_varDeclaration can count the
    // number of variables added and add its own inc instruction.
    addInstruction(state, "inc", -1, -1);
}

void generate_constDeclaration(struct parseTree tree, struct generatorState *state) {
    generate(getChild(tree, "constants"), state);
}

void generate_constants(struct parseTree tree, struct generatorState *state) {
    assert(hasChild(tree, "constant"));

    generate(getChild(tree, "constant"), state);
    // If there is no "constants" child, generate will just return without
    // adding any instructions.
    generate(getChild(tree, "constants"), state);
}

void generate_constant(struct parseTree tree, struct generatorState *state) {
    assert(hasChild(tree, "identifier") && hasChild(tree, "number"));

    addConstant(state, getChild(tree, "identifier"), getChild(tree, "number"));
}

void generate_statement(struct parseTree tree, struct generatorState *state) {
    //assert(hasChild("if-statement") || hasChild("assignment") || hasChild("while-statement") || hasChild("begin-block") || hasChild("read-statement") || hasChild("write-statement"));

    generate(getFirstChild(tree), state);
}

void generate_statements(struct parseTree tree, struct generatorState *state) {
    assert(hasChild(tree, "statement"));

    generate(getChild(tree, "statement"), state);
    generate(getChild(tree, "statements"), state);
}

void generate_beginBlock(struct parseTree tree, struct generatorState *state) {
    assert(hasChild(tree, "statements"));

    generate(getChild(tree, "statements"), state);
}

void generate_readStatement(struct parseTree tree, struct generatorState *state) {
    assert(hasChild(tree, "identifier"));

    addInstruction(state, "read", 0, 2);
    addStoreInstruction(state, getChild(tree, "identifier"));
}

void generate_writeStatement(struct parseTree tree, struct generatorState *state) {
    assert(hasChild(tree, "identifier"));

    addLoadInstruction(state, getChild(tree, "identifier"));
    addInstruction(state, "sio", 0, 1);
}

void generate_assignment(struct parseTree tree, struct generatorState *state) {
    assert(hasChild(tree, "expression") && hasChild(tree, "identifier"));

    generate(getChild(tree, "expression"), state);
    addStoreInstruction(state, getChild(tree, "identifier"));
}

void generate_ifStatement(struct parseTree tree, struct generatorState *state) {
    assert(hasChild(tree, "condition") && hasChild(tree, "statement"));

    // Generate a fake jpc instruction first so that we can find out what
    // instruction we need to jump to.
    struct generatorState *fakeState = copyGeneratorState(state);
    generate(getChild(tree, "condition"), fakeState);
    addInstruction(fakeState, "jpc", -1, -1);
    generate(getChild(tree, "statement"), fakeState);
    int afterIfStatement = fakeState->instructions->length;

    // Generate the real instructions.
    generate(getChild(tree, "condition"), state);
    addInstruction(state, "jpc", 0, afterIfStatement);
    generate(getChild(tree, "statement"), state);
}

void generate_whileStatement(struct parseTree tree, struct generatorState *state) {
    assert(hasChild(tree, "condition") && hasChild(tree, "statement"));

    // Generate a fake jpc instruction first so that we can find out what
    // instruction we need to jump to.
    int beginning = state->instructions->length;
    struct generatorState *fakeState = copyGeneratorState(state);
    generate(getChild(tree, "condition"), fakeState);
    addInstruction(fakeState, "jpc", -1, -1);
    generate(getChild(tree, "statement"), fakeState);
    addInstruction(fakeState, "jmp", 0, beginning);
    int afterWhileLoop = fakeState->instructions->length;

    // Generate the real instructions.
    generate(getChild(tree, "condition"), state);
    addInstruction(state, "jpc", 0, afterWhileLoop);
    generate(getChild(tree, "statement"), state);
    addInstruction(state, "jmp", 0, beginning);
}

void generate_condition(struct parseTree tree, struct generatorState *state) {
    assert(hasChild(tree, "expression")
            && (hasChild(tree, "rel-op") || hasChild(tree, "odd")));

    if (hasChild(tree, "odd")) {
        generate(getChild(tree, "expression"), state);
        // Add the instruction that checks for oddity.
        addInstruction(state, "opr", 0, 6);
    } else {
        generate(getChild(tree, "expression"), state);
        generate(getLastChild(tree, "expression"), state);
        generate(getChild(tree, "rel-op"), state);
    }
}

void generate_expression(struct parseTree tree, struct generatorState *state) {
    assert(hasChild(tree, "term"));

    generate(getChild(tree, "term"), state);

    if (hasChild(tree, "add-or-subtract")) {
        assert(hasChild(tree, "expression"));

        generate(getChild(tree, "expression"), state);
        generate(getChild(tree, "add-or-subtract"), state);
    }
}

void generate_addOrSubtract(struct parseTree tree, struct generatorState *state) {
    char *plusOrMinus = getFirstChild(tree).name;

    if (strcmp(plusOrMinus, "+") == 0)
        addInstruction(state, "opr", 0, 2);
    else if (strcmp(plusOrMinus, "-") == 0)
        addInstruction(state, "opr", 0, 3);
    else
        assert(0 /* Expected + or - inside add-or-subtract. */);
}

void generate_term(struct parseTree tree, struct generatorState *state) {
    assert(hasChild(tree, "factor"));

    generate(getChild(tree, "factor"), state);

    if (hasChild(tree, "multiply-or-divide")) {
        assert(hasChild(tree, "term"));

        generate(getChild(tree, "term"), state);
        generate(getChild(tree, "multiply-or-divide"), state);
    }
}

void generate_multiplyOrDivide(struct parseTree tree, struct generatorState *state) {
    char *starOrSlash = getFirstChild(tree).name;

    if (strcmp(starOrSlash, "*") == 0)
        addInstruction(state, "opr", 0, 4);
    else if (strcmp(starOrSlash, "/") == 0)
        addInstruction(state, "opr", 0, 5);
    else
        assert(0 /* Expected * or / inside multiply-or-divide. */);
}

void generate_factor(struct parseTree tree, struct generatorState *state) {
    assert(hasChild(tree, "expression") || hasChild(tree, "identifier")
            || (hasChild(tree, "sign") && hasChild(tree, "number")));

    // If any of the children are not found, generate() will just return
    // without adding any instructions, so we can just try all of them without
    // checking a bunch of things (as long as we trust the parser to not do
    // something weird like pair an expression and number right next to each
    // other).
    generate(getChild(tree, "number"), state);
    generate(getChild(tree, "sign"), state);

    generate(getChild(tree, "identifier"), state);

    generate(getChild(tree, "expression"), state);
}

void generate_sign(struct parseTree tree, struct generatorState *state) {
    char *sign = getFirstChild(tree).name;

    if (strcmp(sign, "-") == 0)
        addInstruction(state, "opr", 0, 1);
}

void generate_number(struct parseTree tree, struct generatorState *state) {
    char *number = getFirstChild(tree).name;
    assert(isInteger(number));
    int value = atoi(number);

    addInstruction(state, "lit", 0, value);
}

void generate_identifier(struct parseTree tree, struct generatorState *state) {
    addLoadInstruction(state, tree);
}

void generate_relationalOperator(struct parseTree tree, struct generatorState *state) {
    char *operator = getFirstChild(tree).name;

    if (strcmp(operator, "=") == 0)
        addInstruction(state, "opr", 0, 8);
    else if (strcmp(operator, "<>") == 0)
        addInstruction(state, "opr", 0, 9);
    else if (strcmp(operator, "<") == 0)
        addInstruction(state, "opr", 0, 10);
    else if (strcmp(operator, "<=") == 0)
        addInstruction(state, "opr", 0, 11);
    else if (strcmp(operator, ">") == 0)
        addInstruction(state, "opr", 0, 12);
    else if (strcmp(operator, ">=") == 0)
        addInstruction(state, "opr", 0, 13);
    else
        assert(0 /* Invalid relational operator. */);
}

int getOpcode(char *instruction) {
    if (strcmp(instruction, "lit") == 0) return 1;
    if (strcmp(instruction, "opr") == 0) return 2;
    if (strcmp(instruction, "lod") == 0) return 3;
    if (strcmp(instruction, "sto") == 0) return 4;
    if (strcmp(instruction, "cal") == 0) return 5;
    if (strcmp(instruction, "inc") == 0) return 6;
    if (strcmp(instruction, "jmp") == 0) return 7;
    if (strcmp(instruction, "jpc") == 0) return 8;
    if (strcmp(instruction, "sio") == 0) return 9;
    if (strcmp(instruction, "read") == 0) return 10;

    return 0;
}

struct generatorState *makeGeneratorState() {
    struct generatorState *state = make(struct generatorState);

    state->symbols = makeVector(struct symbol);
    state->currentLevel = 0;
    state->instructions = makeVector(struct instruction);

    return state;
}
struct generatorState *copyGeneratorState(struct generatorState *state) {
    struct generatorState *copy = make(struct generatorState);

    copy->symbols = vector_copy(state->symbols);
    copy->currentLevel = state->currentLevel;
    copy->instructions = vector_copy(state->instructions);

    return copy;
}

struct instruction makeInstruction(char *instruction, int lexicalLevel, int modifier) {
    return (struct instruction){getOpcode(instruction), instruction, lexicalLevel, modifier};
}

void addInstruction(struct generatorState *state, char *instruction, int lexicalLevel, int modifier) {
    pushLiteral(state->instructions, struct instruction,
            makeInstruction(instruction, lexicalLevel, modifier));
}

void addLoadInstruction(struct generatorState *state, struct parseTree identifier) {
    char *name = getToken(identifier);
    struct symbol symbol = getSymbol(state, name);

    if (symbol.type == PROCEDURE)
        addGeneratorError("Cannot take value of procedure.");
    else if (symbol.type == VARIABLE)
        addInstruction(state, "lod", symbol.level, symbol.address);
    else if (symbol.type == CONSTANT)
        addInstruction(state, "lit", 0, symbol.constantValue);
}
void addStoreInstruction(struct generatorState *state, struct parseTree identifier) {
    char *name = getToken(identifier);
    struct symbol symbol = getSymbol(state, name);
    if (symbol.type == PROCEDURE || symbol.type == CONSTANT)
        addGeneratorError("Cannot store into a constant or procedure.");
    else if (symbol.type == VARIABLE)
        addInstruction(state, "sto", symbol.level, symbol.address);
}

void addVariable(struct generatorState *state, struct parseTree identifierTree) {
    char *name = getToken(identifierTree);
    // TODO: Will the position in the symbol table will always correspond to
    // the correct address for the symbol in each lexical level?
    int address = state->symbols->length;
    struct symbol symbol = {name, VARIABLE, state->currentLevel, address, 0};

    push(state->symbols, symbol);
}
void addConstant(struct generatorState *state, struct parseTree identifierTree,
        struct parseTree numberTree) {
    char *name = getToken(identifierTree);

    char *number = getToken(numberTree);
    assert(isInteger(number));
    int value = atoi(number);

    struct symbol symbol = {name, CONSTANT, state->currentLevel, 0, value};

    push(state->symbols, symbol);
}
struct symbol getSymbol(struct generatorState *state, char *name) {
    // TODO: Check for symbols in higher lexical levels.
    forVector(state->symbols, i, struct symbol, symbol,
            if (strcmp(symbol.name, name) == 0)
                return symbol;);

    addGeneratorError(format("Could not find symbol '%s'.", name));

    return (struct symbol){NULL, 0, 0, 0, 0};
}

struct vector *generatorErrors = NULL;

void addGeneratorError(char *errorMessage) {
    if (generatorErrors == NULL)
        generatorErrors = makeVector(char*);

    push(generatorErrors, errorMessage);
}
int generatorHasErrors() {
    return (generatorErrors != NULL);
}
char *printGeneratorErrors() {
    forVector(generatorErrors, i, char*, message,
            puts(message););
}

