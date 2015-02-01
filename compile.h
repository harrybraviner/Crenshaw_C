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
int term(FILE *srcfile, FILE *asmfile);
int factor(FILE *srcfile, FILE *asmfile);

void handleKeyword(char *token);

int isDigit(const char inChar);	// Check if a char is a numeral
int isAlpha(const char inChar);	// Check if a char is a letter
int isAlphaNum(const char inChar);	// Check if a char is a letter or number
int isWhitespace(const char inChar); // Check is a char is whitespace
int isAdd(const char inChar); // Check if a char is the addition operator
int isSub(const char inChar); // Check if a char is the subtraction operator
int isAddOp(const char inChar); // Check if a char is additions or subtraction
int isMul(const char inChar); // Check if a char is the multiplication operator
int isDiv(const char inChar); // Check if a char is the division operator
int isMulOp(const char inChar); // Check if a char is either the multiplication or division operator

#endif
