#ifndef FCHOP_TEXT_H
#define FCHOP_TEXT_H

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <stdbool.h>

// Recursively replaces enviroment variables within a string, this is a mutable operation.
// Returns 0 if successful.
// Returns -1 if maxLengthAllowed was exceeded at any point during the process.
// Returns -2 if unknown enviroment variable encountered.
// Returns -3 if maxExpandsAllowed was exceeded.
int expandEVs(char **s, int maxLengthAllowed, int maxExpandsAllowed);

// Replaces the first occurance of %xxxx% with the corresponding enviroment variable, this is a mutable operation.
// Returns 1 if expand was preformed.
// Returns 0 if no occurance found.
// Returns -1 if maxLengthAllowed exceeded (can not preform expand).
// Returns -2 if unknown enviroment variable encountered.
int expandEVsOnce(char **s, int *length, int maxLengthAllowed);

#endif
