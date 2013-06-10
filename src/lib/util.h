#ifndef UTIL_H
#define UTIL_H

// Wrapper for sprintf that allocates the string for you. Copied from the
// manpage for printf.
char *format(const char *fmt, ...);

// Wrapper for strsep that returns the split string as a vector of strings.
// Splits along any sequence of the given split characters.
struct vector *splitString(char const *constString, char *splitCharacters);

// Returns true if the given string represents an integer value.
// Based on the example in the manpage for strtol.
int isInteger(char *string);

#endif
