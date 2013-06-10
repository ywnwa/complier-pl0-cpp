#include "test/lib/generator.h"
#include "src/lib/util.h"   // For splitString
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

int instructionsEqual(struct vector *instructions, char *expectedInstructions) {
    // Split the expected instructions string along whitespace and commas.
    struct vector *parts = splitString(expectedInstructions, ", \n\r\t");
    // Ensure that the number of parts is divisble by 3.
    assert(parts->length % 3 == 0);
    int numExpectedInstructions = parts->length / 3;

    if (instructions->length != numExpectedInstructions)
        return 0;

    int i, j;
    for (i = 0, j = 0; i < instructions->length; i++, j += 3) {
        struct instruction currentInstruction = get(struct instruction, instructions, i);

        // Reconstruct the current expected instruction.
        char *opcodeString = get(char*, parts, j);
        char *lexicalLevelString = get(char*, parts, j + 1);
        char *modifierString = get(char*, parts, j + 2);
        assert(isInteger(lexicalLevelString));
        assert(isInteger(modifierString));

        int opcode = getOpcode(opcodeString);
        int lexicalLevel = atoi(lexicalLevelString);
        int modifier = atoi(modifierString);

        // Compare the current and expected instructions.
        if (opcode != currentInstruction.opcode ||
                lexicalLevel != currentInstruction.lexicalLevel ||
                modifier != currentInstruction.modifier)
            return 0;
    }

    return 1;
}

void printInstructions(struct vector *instructions) {
    int i;
    for (i = 0; i < instructions->length; i++) {
        struct instruction instruction = get(struct instruction, instructions, i);
        printf("%s %d %d\n", instruction.opcodeName,
                instruction.lexicalLevel, instruction.modifier);
    }
}

