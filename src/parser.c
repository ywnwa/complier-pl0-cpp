#include "src/parser.h"
#include "src/lexer.h"
#include "src/lib/util.h"
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>

struct parseTree parseProgram(struct vector *lexemes, struct grammar grammar) {
    struct parseTree result = parse(lexemes, 0, "program", grammar);
    assert(!(result.numTokens > lexemes->length));   // This should never happen.
    if (result.numTokens == lexemes->length)
        return result;
    else {
        struct vector *children = makeVector(struct parseTree);
        push(children, result);
        /*struct lexeme lastLexeme = get(struct lexeme, lexemes, result.numTokens - 1);
        return errorTree(format("Trailing tokens after program, starting at '%s'.",
                    lastLexeme.token), children);*/
        return errorTree("Trailing tokens after program.", children);
    }
}

struct parseTree parse(struct vector *lexemes, int index, char *currentVariable, struct grammar grammar) {
    if (index >= lexemes->length) {
        setParserError(format("Expected %s but got end of file.",
                    currentVariable));
        return errorTree(getParserError(), NULL);
    }

    struct lexeme currentLexeme = get(struct lexeme, lexemes, index);

    struct vector *children = makeVector(struct parseTree);
    // Holds a list of what variables or terminals we expected to find, if we
    // can't find any matches.
    char *expected = NULL;

    // For each production rule in the grammar.
    int i;
    for (i = 0; i < grammar.rules->length; i++) {
        struct rule rule = get(struct rule, grammar.rules, i);
        // If it's a production rule for the current variable.
        if (strcmp(rule.variable, currentVariable) == 0) {
            struct parseTree result = parseRule(rule, lexemes, index, currentVariable, grammar);
            push(children, result);
            if (!isParseTreeError(result)) {
                // Return on the first production rule that succeeds.
                return result;
            } else {
                if (expected == NULL)
                    expected = rule.variable;
                else
                    expected = format("%s or %s", expected, rule.variable);
            }
        }
    }

    // If we didn't find any production rules that succeeded.
    if (expected == NULL)
        return errorTree(format("No rules found for variable %s.",
                    currentVariable), children);
    else
        return errorTree(format("Expected %s starting at '%s'.",
                    expected, currentLexeme.token), children);
}

struct parseTree parseRule(struct rule rule, struct vector *lexemes, int index,
        char *currentVariable, struct grammar grammar) {
    int startIndex = index;
    struct vector *children = makeVector(struct parseTree);

    // For each variable and terminal in the production rule.
    int i;
    for (i = 0; i < rule.production->length; i++) {
        char *varOrTerminal = get(char*, rule.production, i);

        // Special case for the the empty string, which is represented as
        // "nothing" inside of production rules. Just accept it without
        // increasing the index.
        if (strcmp(varOrTerminal, "nothing") == 0)
            continue;

        if (index >= lexemes->length) {
            setParserError(format("Expected '%s' but got end of file while parsing %s.",
                    varOrTerminal, currentVariable));
            return errorTree(getParserError(), children);
        }

        struct lexeme currentLexeme = get(struct lexeme, lexemes, index);

        int tokenType = getTokenType(varOrTerminal);
        int isTerminal = (tokenType > 0);
        if (isTerminal) {
            if (tokenType == currentLexeme.tokenType) {
                // Go to next token if this token matches the terminal.
                pushLiteral(children, struct parseTree, {currentLexeme.token, NULL, 1});
                index += 1;
            } else {
                setParserError(format("Expected '%s' but got '%s' while parsing %s.",
                        varOrTerminal, currentLexeme.token, currentVariable));
                return errorTree(getParserError(), children);
            }
        } else {
            // Add child and go to token after child's tokens.
            struct parseTree child = parse(lexemes, index, varOrTerminal, grammar);
            push(children, child);
            index += child.numTokens;

            if (isParseTreeError(child)) {
                return errorTree(format("Expected '%s' starting at '%s' while parsing %s.",
                            varOrTerminal, currentLexeme.token, currentVariable), children);
            }
        }
    }

    int numTokens = index - startIndex;
    return (struct parseTree){currentVariable, children, numTokens};
}

struct parseTree errorTree(char *error, struct vector *children) {
    return (struct parseTree){error, children, -1};
}

int isParseTreeError(struct parseTree tree) {
    return (tree.numTokens < 0);
}

int getTokenType(char *token) {
    if (strcmp(token, "intsym") == 0) return INTSYM;
    if (strcmp(token, "semicolonsym") == 0) return SEMICOLONSYM;
    if (strcmp(token, "beginsym") == 0) return BEGINSYM;
    if (strcmp(token, "endsym") == 0) return ENDSYM;
    if (strcmp(token, "readsym") == 0) return READSYM;
    if (strcmp(token, "writesym") == 0) return WRITESYM;
    if (strcmp(token, "periodsym") == 0) return PERIODSYM;
    if (strcmp(token, "commasym") == 0) return COMMASYM;
    if (strcmp(token, "lparentsym") == 0) return LPARENTSYM;
    if (strcmp(token, "rparentsym") == 0) return RPARENTSYM;
    if (strcmp(token, "plussym") == 0) return PLUSSYM;
    if (strcmp(token, "minussym") == 0) return MINUSSYM;
    if (strcmp(token, "multsym") == 0) return MULTSYM;
    if (strcmp(token, "slashsym") == 0) return SLASHSYM;
    if (strcmp(token, "eqsym") == 0) return EQSYM;
    if (strcmp(token, "neqsym") == 0) return NEQSYM;
    if (strcmp(token, "lessym") == 0) return LESSYM;
    if (strcmp(token, "leqsym") == 0) return LEQSYM;
    if (strcmp(token, "gtrsym") == 0) return GTRSYM;
    if (strcmp(token, "geqsym") == 0) return GEQSYM;
    if (strcmp(token, "oddsym") == 0) return ODDSYM;
    if (strcmp(token, "constsym") == 0) return CONSTSYM;
    if (strcmp(token, "becomessym") == 0) return BECOMESSYM;
    if (strcmp(token, "ifsym") == 0) return IFSYM;
    if (strcmp(token, "thensym") == 0) return THENSYM;
    if (strcmp(token, "whilesym") == 0) return WHILESYM;
    if (strcmp(token, "dosym") == 0) return DOSYM;
    if (strcmp(token, "identsym") == 0) return IDENTSYM;
    if (strcmp(token, "numbersym") == 0) return NUMBERSYM;

    return -1;
}

struct parseTree getChild(struct parseTree parent, char *childName) {
    assert(parent.children != NULL);

    forVector(parent.children, i, struct parseTree, child,
        if (strcmp(child.name, childName) == 0)
            return child;);

    return errorTree(format("Could not find child with name '%s'.", childName),
            parent.children);
}

struct parseTree getLastChild(struct parseTree parent, char *childName) {
    assert(parent.children != NULL);

    int i;
    for (i = parent.children->length - 1; i >= 0; i--) {
        struct parseTree child = get(struct parseTree, parent.children, i);
        if (strcmp(child.name, childName) == 0)
            return child;
    }

    return errorTree(format("Could not find child with name '%s'.", childName),
            parent.children);
}

int hasChild(struct parseTree parent, char *childName) {
    struct parseTree child = getChild(parent, childName);

    return !isParseTreeError(child);
}

struct parseTree getFirstChild(struct parseTree tree) {
    if (tree.children != NULL && tree.children->length > 0)
        return get(struct parseTree, tree.children, 0);
    else
        return errorTree("Cannot get first child of tree with no children.",
                tree.children);
}

char *getToken(struct parseTree parent) {
    assert(parent.children != NULL && parent.children->length == 1);

    return getFirstChild(parent).name;
}


void addRule(struct grammar grammar, char *variable, char *productionString) {
    struct vector *production = splitString(productionString, " ");
    pushLiteral(grammar.rules, struct rule, {variable, production});
}

void freeParseTree(struct parseTree tree) {
    free(tree.name);

    if (tree.children != NULL) {
        forVector(tree.children, i, struct parseTree, child,
                freeParseTree(child););
        freeVector(tree.children);
    }
}

char *parserError = NULL;

char *setParserError(char *message) {
    parserError = message;
}

char *getParserError() {
    return parserError;
}

