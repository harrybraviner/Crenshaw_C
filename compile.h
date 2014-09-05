#ifndef COMPILE_H
#define COMPILE_H

#include <stdio.h>
#include <stdlib.h>

// Global variable for holding length of numerical literals
unsigned short numLitLen;

// Compile the entire file
int compile(FILE *srcfile, FILE *asmfile);

// Parse a single expression
int expression(FILE *srcfile, FILE *asmfile);

int isDigit(const char inChar);	// Check if a char is a numeral
int isWhitespace(const char inChar); // Check is a char is whitespace


#endif
