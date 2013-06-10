#ifndef PARSER_H
#define PARSER_H

#include "src/lib/vector.h"
#include "src/lexer.h"

// A parseTree is basically just a tree of strings.
struct parseTree {
    char *name;
    struct vector *children;
    int numTokens;   // The number of tokens that this parse tree represents.
};

struct grammar {
    struct vector *rules;
};

struct rule {
    char *variable;
    struct vector *production;
};

// Wrapper for parse that stries to parse the lexemes as a "program" variable.
// Use this instead of using parse directly.
struct parseTree parseProgram(struct vector *lexemes, struct grammar grammar);

// Parse the given lexemes, returning a parse tree.
struct parseTree parse(struct vector *lexemes, int index, char *currentVariable, struct grammar grammar);

// Try to parse a single production rule for the given variable from the grammar.
struct parseTree parseRule(struct rule rule, struct vector *lexemes, int index,
        char *currentVariable, struct grammar grammar);

// Returns a parse tree that indicates an error occurred, with the given error
// message as its name.
struct parseTree errorTree(char *error, struct vector *children);
// Returns tree if the given tree is a tree that was produced by errorTree().
int isParseTreeError(struct parseTree tree);

// Converts "semicolonsym", "readsym", etc. into integers that represent the
// token type.
int getTokenType(char *token);

// Returns the first child of the given parseTree that has the given name.
struct parseTree getChild(struct parseTree parent, char *childName);
// Returns the last child of the given parseTree that has the given name.
struct parseTree getLastChild(struct parseTree parent, char *childName);
struct parseTree getFirstChild(struct parseTree parent);
int hasChild(struct parseTree parent, char *childName);

// If the given parse tree has a single child node, return the name of that
// child. Because leaf nodes represent tokens, this can be used to get the
// value of a token.
char *getToken(struct parseTree parent);

// Add a production rule to the given grammar. The production rule maps from
// variable -> productionString, where production string is a space-separated
// list of other variables and terminals that the variable should produce.
void addRule(struct grammar grammar, char *variable, char *productionString);

// Recursively free a parse tree and all of its children.
void freeParseTree(struct parseTree tree);

char *setParserError(char *message);
char *getParserError();

#endif
