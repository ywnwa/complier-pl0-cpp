#ifndef LEXER_H
#define LEXER_H

#include "src/lib/vector.h"
#include <regex.h>
#include <stdlib.h>

#define MAX_IDENTIFIER_LENGTH 11
#define MAX_NUMBER_LENGTH 5

enum {

   NULSYM = 1, IDENTSYM, NUMBERSYM, PLUSSYM, MINUSSYM, MULTSYM, SLASHSYM,
   ODDSYM, EQSYM, NEQSYM, LESSYM, LEQSYM, GTRSYM, GEQSYM, LPARENTSYM,
   RPARENTSYM, COMMASYM, SEMICOLONSYM, PERIODSYM, BECOMESSYM, BEGINSYM, ENDSYM,
   IFSYM, THENSYM, WHILESYM, DOSYM, CALLSYM, CONSTSYM, INTSYM, PROCSYM,
   WRITESYM, READSYM, ELSESYM,

   // Custom symbols used to make implementation simpler. Whitespace and
   // comments are treated just like any other tokens, except that they are
   // discarded from the lexeme list that is printed out.
   WHITESPACESYM, COMMENTSYM

};

extern char *TOKEN_NAMES[];

struct lexeme {

   int tokenType;
   char *token;

};

// Call once to compile the regular expressions used by the lexer, before using
// the other lexer functions.
void initLexer();

// Given a string of PL/0 source code, return a vector of lexemes representing
// the source code.
struct vector *readLexemes(char *source);
// Try to read a single lexeme at the beginning of the given string of PL/0
// source code, returning an empty lexeme (i.e. (struct lexeme){0, NULL}) if
// there is no valid token at the beginning of the string.
struct lexeme readLexeme(char *source);

// Given a compiled regex and a string, return the first substring that matches
// the regex, or NULL if there is no match.
char *getMatch(regex_t *regex, char *string);

#endif
