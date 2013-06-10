#ifndef TEST_GENERATOR_H
#define TEST_GENERATOR_H

#include "src/generator.h"

// This file holds functions used to test the code generator.

// Takes a vector of instructions and a string containing a list of
// instructions, and returns true if they represent the same instructions, and
// false if they don't.
int instructionsEqual(struct vector *instructions, char *expectedInstructions);

// Print list of instructions, for debugging.
void printInstructions(struct vector *instructions);

#endif
