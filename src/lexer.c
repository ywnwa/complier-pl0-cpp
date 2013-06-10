#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <assert.h>
#include "src/lib/vector.h"
#include "src/lexer.h"

/* Algorithm outline:
 * - Read in entire file as a string.
 * - While we haven't reached the end of the string:
 *   - Go through each regex in tokenDefinitions.
 *   - If we find a regex that matches, get the token that the regex matches
 *     and add it to the list of lexmes.
 *   - If no regex matches, print out an error.
 *   - Maybe do some special error checking for numbers and identifiers.
 *   - Discard comment and whitespace tokens.
 * - Return list of lexemes.
 */

char *TOKEN_NAMES[] = {

   NULL, NULL, "identsym", "numbersym", "plusym", "minussym", "multsym", "slashsym",
   "oddsym", "eqsym", "neqsym", "lessym", "leqsym", "gtrsym", "geqsym", "lparentsym",
   "rparentsym", "commasym", "semicolonsym", "periodsym", "becomessym", "beginsym", "endsym",
   "ifsym", "thensym", "whilesym", "dosym", "callsym", "constsym", "intsym", "procsym",
   "writesym", "readsym", "elsesym", NULL, NULL

};

struct tokenDefinition {

    char *regexString;
    int tokenType;
    regex_t *regex;

};

// Regexes that match each token type, along with the token type that they map
// to. The regexes are compiled with REG_EXTENDED (they are interpreted as
// POSIX extended regular expressions).
struct tokenDefinition tokenDefinitions[] = {

    // Whitespace
    {"\\s+", WHITESPACESYM, NULL},
    // The comment regex is based off of http://ostermiller.org/findcomment.html
    // [*] matches a single * character, and looks slightely nicer than \\*
    // (\\* instead of \* because it needs to be escaped twice: once for the C
    // string and another time for the regex.)
    {"/[*]([^*]|[*]+[^*/])*[*]+/", COMMENTSYM, NULL},
    // Keywords
    // \\b = \b and matches the empty string, but only along a word boundary,
    // where words anything that contains [a-zA-Z0-9_].
    {"begin\\b", BEGINSYM, NULL},
    {"while\\b", WHILESYM, NULL},
    {"const\\b", CONSTSYM, NULL},
    {"write\\b", WRITESYM, NULL},
    {"call\\b", CALLSYM, NULL},
    {"then\\b", THENSYM, NULL},
    {"procedure\\b", PROCSYM, NULL},
    {"read\\b", READSYM, NULL},
    {"else\\b", ELSESYM, NULL},
    {"odd\\b", ODDSYM, NULL},
    {"end\\b", ENDSYM, NULL},
    {"int\\b", INTSYM, NULL},
    {"if\\b", IFSYM, NULL},
    {"do\\b", DOSYM, NULL},
    // Identifiers
    {"[a-zA-Z]\\w*", IDENTSYM, NULL},
    // Numbers
    {"[0-9]+", NUMBERSYM, NULL},
    // Special symbols
    {">=", GEQSYM, NULL},
    {"<=", LEQSYM, NULL},
    {"<>", NEQSYM, NULL},
    {":=", BECOMESSYM, NULL},
    {"[+]", PLUSSYM, NULL},
    {"-", MINUSSYM, NULL},
    {"[*]", MULTSYM, NULL},
    {"/", SLASHSYM, NULL},
    {"=", EQSYM, NULL},
    {"<", LESSYM, NULL},
    {">", GTRSYM, NULL},
    {"[(]", LPARENTSYM, NULL},
    {"[)]", RPARENTSYM, NULL},
    {",", COMMASYM, NULL},
    {";", SEMICOLONSYM, NULL},
    {"[.]", PERIODSYM, NULL},
    // Indicates the end of token definitions.
    {NULL, 0, NULL}

};

// Compile all of the regexes in tokenDefinitions.
void initLexer() {

    int i;
    for (i = 0; tokenDefinitions[i].regexString != NULL; i++) {

        // Prepend ^ to all of the regexes so that it only matches tokens at the
        // current position in the source code.
        char *regexString = (char*)malloc(sizeof(char) * (1 + strlen(tokenDefinitions[i].regexString)));
        regexString[0] = '^';
        strcpy(regexString + 1, tokenDefinitions[i].regexString);
        tokenDefinitions[i].regex = (regex_t*)malloc(sizeof(regex_t));
        int error = regcomp(tokenDefinitions[i].regex, regexString, REG_EXTENDED);
        free(regexString);

        // TODO: add getLexerError() function
        /*if (error != 0) {

          int errorStringLength = 1000;
          char *errorString = (char*)malloc(sizeof(char) * errorStringLength);
          regerror(error, tokenDefinitions[i].regex, errorString, errorStringLength);
          printError("Error compiling regex '%s': %s", tokenDefinitions[i].regexString, errorString);
          free(errorString);

          }*/

    }

}

struct vector *readLexemes(char *source) {

    int i = 0;
    struct vector *lexemes = vector_init(sizeof(struct lexeme));

    while (source[i] != '\0') {

        struct lexeme lexeme = readLexeme(&source[i]);

        if (lexeme.token != NULL) {

            if (lexeme.tokenType != WHITESPACESYM && lexeme.tokenType != COMMENTSYM)
                vector_push(lexemes, &lexeme);

            i += strlen(lexeme.token);

        }
        else {

            // TODO: add getLexerError() function
            //printError("Unrecognized symbol starting at '%.10s...'.", &source[i]);

            i += 1;

        }

    }

    return lexemes;

}

struct lexeme readLexeme(char *source) {

    int i;
    for (i = 0; tokenDefinitions[i].regexString != NULL; i++) {

        struct tokenDefinition definition = tokenDefinitions[i];

        char *match = getMatch(definition.regex, source);

        if (match != NULL)
            return (struct lexeme){definition.tokenType, match};

    }

    return (struct lexeme){0, NULL};

}

// Returns a new string that is a copy of the given string up to the given
// length.
char *substring(char *string, int length) {

    char *substr = (char*)malloc(sizeof(char) * (length + 1));
    strncpy(substr, string, length);
    // Add null character.
    substr[length] = '\0';

    return substr;

}

char *getMatch(regex_t *regex, char *string) {

    regmatch_t matches[1];
    int error = regexec(regex, string, 1, matches, 0);

    if (error == REG_NOMATCH)
        return NULL;

    // TODO: add getLexerError() function
    /*if (error != 0) {

      int errorStringLength = 1000;
      char *errorString = (char*)malloc(sizeof(char) * errorStringLength);
      regerror(error, regex, errorString, errorStringLength);
      printError("Error executing regex: %s", errorString);
      free(errorString);

      }*/

    // matches[0] holds the indices of what the overall regex matched.
    regmatch_t tokenMatch = matches[0];

    int matchStart = tokenMatch.rm_so;
    int matchEnd = tokenMatch.rm_eo;

    if (matchStart >= 0) {

        int tokenLength = matchEnd - matchStart;
        if (tokenLength > 0) {

            char *token = substring(&string[matchStart], tokenLength);

            return token;

        }

    }

    return NULL;

}

