#include "src/lexer.h"
#include "src/parser.h"
#include "src/generator.h"
#include "src/lib/vector.h"
#include "test/lib/parser.h"
#include "test/lib/generator.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

struct grammar PL0Grammar();
char *readContents(char *filename);

int main(int argc, char **argv) {
    if (argc < 2) {
        assert(argc >= 1);
        printf("Usage: %s <PL/0 source code filename> [<verbosity level>]\n", argv[0]);
        return 1;
    }

    int verbose = 0;
    if (argc >= 3)
        verbose = atoi(argv[2]);

    // Initialize compiler.
    initLexer();
    struct grammar grammar = PL0Grammar();

    // Read in source code.
    char *sourceCode = readContents(argv[1]);
    assert(sourceCode != NULL);

    // Print source code.
    if (verbose >= 2)
        printf("Source code:\n%s\n", sourceCode);

    // Read tokens.
    struct vector *lexemes = readLexemes(sourceCode);
    assert(lexemes != NULL);

    // Print tokens.
    if (verbose >= 3) {
        printf("Tokens:\n");
        forVector(lexemes, i, struct lexeme, lexeme,
                printf("%d ", lexeme.tokenType);
                if (lexeme.tokenType == IDENTSYM || lexeme.tokenType == NUMBERSYM)
                printf("%s ", lexeme.token););
        printf("\n\nTokens with token names:\n");
        forVector(lexemes, i, struct lexeme, lexeme,
                printf("%s ", TOKEN_NAMES[lexeme.tokenType]);
                if (lexeme.tokenType == IDENTSYM || lexeme.tokenType == NUMBERSYM)
                printf("%s ", lexeme.token););
        printf("\n\n");
    }

    // Parse tokens.
    struct parseTree tree = parseProgram(lexemes, grammar);
    if (isParseTreeError(tree)) {
        printf("Error while parsing program. This is what the parser was able to parse:\n");
        printParseTree(tree);
        return 1;
    } 

    if( verbose > 0)
      printf("Everything Parsed correct.\n\n");

    // Print parse tree.
    if (verbose >= 4) {
        printf("Parse tree:\n");
        printParseTree(tree);

        printf("\n");
    }

    // Generate code.
    struct vector *instructions = generateInstructions(tree);
    if (generatorHasErrors()) {
        printf("The generator encountered errors:\n");
        printGeneratorErrors();
        if (instructions != NULL)
            printf("\nThis is what the generator was able to generate:\n");
        printInstructions(instructions);

        return 1;
    }

    // Print generated code.
    assert(instructions != NULL);
    if (verbose >= 1) {
        printf("Generated instructions:\n");
        // Print code with nice opcode names.
        printInstructions(instructions);
    } else {
        // Print code suitable for the VM.
        forVector(instructions, i, struct instruction, instruction,
                printf("%d %d %d\n",
                    instruction.opcode,
                    instruction.lexicalLevel,
                    instruction.modifier););
    }

    return 0;
}

struct grammar PL0Grammar() {
    // Define full PL/0 grammar
    struct grammar grammar = (struct grammar){makeVector(struct rule)};
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
    addRule(grammar, "statement", "nothing");

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

    return grammar;
}

char *readContents(char *filename) {

    FILE *file = fopen(filename, "r");
    assert(file != NULL);

    // Go to the end of the file and read the position to get the length of the
    // file.
    fseek(file, 0, SEEK_END);
    int length = ftell(file);

    // Return to the beginning of the file.
    rewind(file);

    // Try to read length characters.
    char *contents = malloc(sizeof(char)*(length + 1));
    int charsRead = fread(contents, sizeof(char), length, file);
    // Add null character.
    contents[charsRead] = '\0';

    return contents;

}

