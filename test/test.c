#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "src/lexer.h"
#include "test/lib/parser.h"
#include "test/lib/generator.h"

void testTestUtil() {
    void testVectorizeForm() {
        struct vector *items = formToVector("(aaa (bbb ccc) ddd)");

        assert(strcmp(get(char*, items, 0), "aaa") == 0);
        assert(strcmp(get(char*, items, 1), "(bbb ccc)") == 0);
        assert(strcmp(get(char*, items, 2), "ddd") == 0);

        freeVector(items);
    }

    void testGenerateParseTree() {
        struct parseTree tree = pt(aaa (bbb (identifier ccc)) (number 123));
        assert(strcmp(tree.name, "aaa") == 0);
        struct parseTree child = get(struct parseTree, tree.children, 0);
        assert(strcmp(child.name, "bbb") == 0);
        struct parseTree grandchild = get(struct parseTree, child.children, 0);
        assert(strcmp(grandchild.name, "identifier") == 0);
        char *identifier = get(struct parseTree, grandchild.children, 0).name;
        assert(strcmp(identifier, "ccc") == 0);
        struct parseTree child2 = get(struct parseTree, tree.children, 1);
        assert(strcmp(child2.name, "number") == 0);

        // TODO: Free parse tree.
    }

    void testInstructionsEqual() {
        struct vector *instructions = makeVector(struct instruction);
        struct instruction instruction;
        instruction = (struct instruction){1, 0, 0}; push(instructions, instruction);
        instruction = (struct instruction){2, 0, 0}; push(instructions, instruction);
        instruction = (struct instruction){3, 0, 0}; push(instructions, instruction);
        assert(instructionsEqual(instructions,
                    "lit 0 0,"
                    "opr 0 0,"
                    "lod 0 0"));
        assert(!instructionsEqual(instructions, ""));

        freeVector(instructions);
    }

    testVectorizeForm();
    testGenerateParseTree();
    testInstructionsEqual();
}

void testLexer() {
    initLexer();

    struct vector *lexemes = readLexemes(
            "int x;\n"
            "begin\n"
                "read x\n"
                "if x = 0 then\n"
                    "write x\n"
            "end\n");

    /*int i;
    for (i = 0; i < lexemes->length; i++) {
        struct lexeme lexeme = get(struct lexeme, lexemes, i);
        printf("%s ", lexeme.token);
    }
    printf("\n");*/
}

void testParser() {
    initLexer();

    struct vector *lexemes = readLexemes("int x;");

    struct grammar grammar = {makeVector(struct rule)};
    addRule(grammar, "integer", "intsym identifier semicolonsym");
    addRule(grammar, "identifier", "identsym");
    struct parseTree tree = parse(lexemes, 0, "integer", grammar);
    assert(tree.name != NULL);
    assert(parseTreesSimilar(tree, pt(integer (identifier x))));

    lexemes = readLexemes(
            "begin\n"
                "read x;\n"
                "write x;\n"
            "end\n");

    grammar = (struct grammar){makeVector(struct rule)};
    addRule(grammar, "begin-block", "beginsym statements endsym");
    // Just like in the lexer the regexes for the longer strings must come
    // before the shorter ones, with the grammar longer rules must come before
    // shorter ones so that each variable is "greedy" and matches as much as it
    // possibly can. So, rule that matches multiple statements must come before
    // the rule that only matches one.
    addRule(grammar, "statements", "statement semicolonsym statements");
    addRule(grammar, "statements", "nothing");
    addRule(grammar, "statement", "read-statement");
    addRule(grammar, "statement", "write-statement");
    addRule(grammar, "read-statement", "readsym identifier");
    addRule(grammar, "write-statement", "writesym identifier");
    addRule(grammar, "identifier", "identsym");
    tree = parse(lexemes, 0, "begin-block", grammar);
    assert(tree.name != NULL);
    assert(parseTreesSimilar(tree,
                pt(begin-block
                    (statements
                     (statement (read-statement
                                 (identifier x)))
                     (statements
                      (statement (write-statement
                                  (identifier x))))))));

    // Full grammar for PL/0 language.
    grammar = (struct grammar){makeVector(struct rule)};
    addRule(grammar, "program", "block periodsym");

    addRule(grammar, "block", "const-declaration var-declaration statement");

    addRule(grammar, "const-declaration", "constsym constants semicolonsym");
    addRule(grammar, "const-declaration", "nothing");
    addRule(grammar, "constants", "constant commasym constants");
    addRule(grammar, "constants", "constant");
    addRule(grammar, "constant", "identifier eqsym number");

    addRule(grammar, "var-declaration", "intsym vars semicolonsym");
    addRule(grammar, "var-declaration", "nothing");
    addRule(grammar, "vars", "var commasym vars");
    addRule(grammar, "vars", "var");
    addRule(grammar, "var", "identifier");

    addRule(grammar, "statement", "read-statement");
    addRule(grammar, "statement", "write-statement");
    addRule(grammar, "statement", "assignment");
    addRule(grammar, "statement", "if-statement");
    addRule(grammar, "statement", "while-statement");
    addRule(grammar, "statement", "begin-block");
    //addRule(grammar, "statement", "nothing");

    addRule(grammar, "assignment", "identifier becomessym expression");

    addRule(grammar, "begin-block", "beginsym statements endsym");
    addRule(grammar, "statements", "statement semicolonsym statements");
    addRule(grammar, "statements", "statement");

    addRule(grammar, "if-statement", "ifsym condition thensym statement");
    addRule(grammar, "condition", "expression rel-op expression");
    addRule(grammar, "condition", "oddsym expression");
    addRule(grammar, "rel-op", "eqsym");
    addRule(grammar, "rel-op", "neqsym");
    addRule(grammar, "rel-op", "lessym");
    addRule(grammar, "rel-op", "leqsym");
    addRule(grammar, "rel-op", "gtrsym");
    addRule(grammar, "rel-op", "geqsym");

    // This is the grammar for expressions included in the assignment.
    /*addRule(grammar, "expression", "sign term add-or-substract term");
    addRule(grammar, "expression", "sign term");
    addRule(grammar, "sign", "plussym");
    addRule(grammar, "sign", "minussym");
    addRule(grammar, "sign", "nothing");
    addRule(grammar, "add-or-substract", "plussym");
    addRule(grammar, "add-or-substract", "minussym");
    addRule(grammar, "term", "factor multiply-or-divide factor");
    addRule(grammar, "term", "factor");
    addRule(grammar, "multiply-or-divide", "multsym");
    addRule(grammar, "multiply-or-divide", "slashsym");
    addRule(grammar, "factor", "lparentsym expression rparentsym");
    addRule(grammar, "factor", "identifier");
    addRule(grammar, "factor", "number");*/

    // This is an improved grammar for expressions that behaves more like you
    // would intuitively expeted expressions to behave. For example, 1 + 2 + 3
    // is not a valid expression in the other grammar, you would have to do
    // something like 1 + (2 + 3) instead, but it works with this grammar.
    addRule(grammar, "expression", "term add-or-subtract expression");
    addRule(grammar, "expression", "term");
    addRule(grammar, "add-or-subtract", "plussym");
    addRule(grammar, "add-or-subtract", "minussym");
    addRule(grammar, "term", "factor multiply-or-divide term");
    addRule(grammar, "term", "factor");
    addRule(grammar, "multiply-or-divide", "multsym");
    addRule(grammar, "multiply-or-divide", "slashsym");
    addRule(grammar, "factor", "lparentsym expression rparentsym");
    addRule(grammar, "factor", "sign number");
    addRule(grammar, "factor", "identifier");
    addRule(grammar, "sign", "plussym");
    addRule(grammar, "sign", "minussym");
    addRule(grammar, "sign", "nothing");
    addRule(grammar, "number", "numbersym");

    addRule(grammar, "while-statement", "whilesym condition dosym statement");

    addRule(grammar, "read-statement", "readsym identifier");
    addRule(grammar, "write-statement", "writesym identifier");

    addRule(grammar, "identifier", "identsym");
    addRule(grammar, "number", "numbersym");

    lexemes = readLexemes(
            "const a = 5, b = 10;\n"
            "int x, y, z;\n"
            "begin\n"
                "read x;\n"
                "read y;\n"
                "x := x + y*a;\n"
                "y:=(y+x)*b;\n"
                "z := x + y;\n"
                "write z\n"
            "end.\n");
    assert(lexemes != NULL);
    tree = parseProgram(lexemes, grammar);
    assert(tree.name != NULL);
    //printParseTree(tree);
    // TODO: Write giant parse tree to test this program.
}

void testCodeGenerator() {
    void testIfStatement() {
        // Create a parse tree that represents the following piece of PL/0 code
        // and pass it to the code generator:
        //
        // const y = 3;
        // int x;
        // begin
        //     read x
        //     if x = y then
        //         write x
        // end.
        struct parseTree tree = pt(program
                (block
                    (var-declaration
                        (vars
                            (var (identifier x))))
                    (const-declaration
                        (constants
                            (constant (identifier y) (number 3))))
                    (statement
                        (begin-block
                            (statements
                                (statement
                                    (read-statement
                                        (identifier x)))
                                (statements
                                    (statement
                                        (if-statement
                                            (condition
                                                (expression
                                                    (term (factor (identifier x))))
                                                (rel-op =)
                                                (expression
                                                    (term (factor (identifier y)))))
                                            (statement
                                                (write-statement
                                                    (identifier x)))))))))));

        struct vector *instructions = generateInstructions(tree);
        if (generatorHasErrors())
            printGeneratorErrors();
        assert(instructions != NULL);
        assert(instructionsEqual(instructions,
                    " inc 0 1"   // Reserve space for int x
                    " sio 0 2"   // Read onto stack
                    " sto 0 0"   // Store read value in x
                    " lod 0 0"   // Load x onto stack
                    " lit 0 3"   // Push the value of y onto stack
                    " opr 0 8"   // Check for equality
                    " jpc 0 9"   // Jump to after if statement if comparison
                                 // was false
                    " lod 0 0"   // Load x onto the stack
                    " sio 0 1"   // Output the value of x
                    " opr 0 0"   // Return/Exit
                    ));

        freeParseTree(tree);
        freeVector(instructions);
    }

    void testExpression() {
        initLexer();

        // Define grammar.
        struct grammar grammar = {makeVector(struct rule)};
        addRule(grammar, "expression", "term add-or-subtract expression");
        addRule(grammar, "expression", "term");
        addRule(grammar, "add-or-subtract", "plussym");
        addRule(grammar, "add-or-subtract", "minussym");
        addRule(grammar, "term", "factor multiply-or-divide term");
        addRule(grammar, "term", "factor");
        addRule(grammar, "multiply-or-divide", "multsym");
        addRule(grammar, "multiply-or-divide", "slashsym");
        addRule(grammar, "factor", "lparentsym expression rparentsym");
        addRule(grammar, "factor", "sign number");
        addRule(grammar, "factor", "identifier");
        addRule(grammar, "sign", "plussym");
        addRule(grammar, "sign", "minussym");
        addRule(grammar, "sign", "nothing");
        addRule(grammar, "number", "numbersym");

        int expressionBecomes(char *expression, char *expectedInstructions) {
            // Read tokens.
            struct vector *lexemes = readLexemes(expression);
            // Parse tokens.
            struct parseTree tree = parse(lexemes, 0, "expression", grammar);
            // Generate code.
            struct vector *instructions = generateInstructions(tree);
            if (generatorHasErrors())
                printGeneratorErrors();
            assert(instructions != NULL);
            // Test generated code.
            return instructionsEqual(instructions, expectedInstructions);
        }

        assert(expressionBecomes("5", "lit 0 5"));
        assert(expressionBecomes("-100", "lit 0 100, opr 0 1"));
        assert(expressionBecomes("5 + 10", "lit 0 5, lit 0 10, opr 0 2"));
        assert(expressionBecomes("1 + 2 + 3", "lit 0 1, lit 0 2, lit 0 3, opr 0 2, opr 0 2"));
        assert(expressionBecomes("-3 * (5 + -10)",
                    "lit 0 3,"
                    "opr 0 1,"
                    "lit 0 5,"
                    "lit 0 10,"
                    "opr 0 1,"
                    "opr 0 2,"
                    "opr 0 4"));
    }

    testIfStatement();
    testExpression();
}

int main() {
    testTestUtil();
    testLexer();
    testParser();
    testCodeGenerator();

    printf("All tests passed.\n");

    return 0;
}
