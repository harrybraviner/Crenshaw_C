#include "compile.h"
#include <stdio.h>
#include <stdlib.h>

char *charPtr;
char nextChar;

#define READCHAR { nextChar = fgetc(srcfile); }

int compile(FILE *srcfile, FILE *asmfile){

	// Emit some sort of header
	fprintf(asmfile, "segment\t.data\n\n");
	fprintf(asmfile, "segment\t.bss\n");
	fprintf(asmfile, "retval\tresd\t1\n");
	fprintf(asmfile, "\nsegment\t.text\n");
	fprintf(asmfile, "\tglobal\tasm_main\n");
	fprintf(asmfile, "asm_main:\n");
	fprintf(asmfile, "\tenter\t0,\t0\n");
	fprintf(asmfile, "\tpusha\n");
	fprintf(asmfile, "\n;\tBeginning of BASIC code\n\n");

	READCHAR;
	if(expression(srcfile, asmfile)) return -1;

	// Emit the end matter for the main function
	fprintf(asmfile, "\n;\tEnd of BASIC code\n\n");
	fprintf(asmfile, "\tmov\t[retval],\teax\n");
	fprintf(asmfile, "\tpopa\n");
	fprintf(asmfile, "\tmov\teax,\t[retval]\n");
	fprintf(asmfile, "\tleave\n");
	fprintf(asmfile, "\tret\n");

	// Indicate success
	return 0;
}

// Macro definitions

#define GETNUM( numLit ) { REMOVEWHITESPACE; if(!isDigit(nextChar)) EXPECTED("integer"); charPtr = numLit; while(isDigit(nextChar)) { *charPtr = nextChar; charPtr++; READCHAR; } *charPtr = '\0'; }

#define REMOVEWHITESPACE { while(isWhitespace(nextChar)) READCHAR; }

#define EXPECTED( expr ) { fprintf(stderr, "Expected %s\n", expr); return -1; }

// FIXME - this doesn't work at the moment!
// FIXME - the problem is that I'm not pushing values
//         onto the stack, and I'm ending up writing over
//         registers when I shouldn't!

int expression(FILE *srcfile, FILE *asmfile){
	REMOVEWHITESPACE;
	if(isAddOp(nextChar)){
		fprintf(asmfile, "\txor\teax,\teax\n");
	} else {
		term(srcfile, asmfile);
	}
	REMOVEWHITESPACE;
	while(isAddOp(nextChar)){
		fprintf(asmfile, "\tpush\teax\n");
		if(isAdd(nextChar)){
			READCHAR;
			term(srcfile, asmfile);
			fprintf(asmfile, "\tpop\tebx\n");
			fprintf(asmfile, "\tadd\teax,\tebx\n");
		}
		else if(isSub(nextChar)){
			READCHAR;
			term(srcfile, asmfile);
			fprintf(asmfile, "\tpop\tebx\n");
			fprintf(asmfile, "\tsub\teax,\tebx\n");
			fprintf(asmfile, "\tneg\teax\n");
		}
		else {
			EXPECTED( "addition or subtraction operator" );
		}
		REMOVEWHITESPACE;
	}
	// The thing with the EOF on the next line is VERY naughty - this in NOT the right way to check for an EOF, I'm just being really lazy and getting away with it. Do as I say, not as I do, kids!
	if(nextChar != '\n' && nextChar != (char)EOF && nextChar != ')')
		EXPECTED("multiplication or division operator")
	return 0;
}

int term(FILE *srcfile, FILE *asmfile){
	factor(srcfile, asmfile);
	REMOVEWHITESPACE;
	while(isMulOp(nextChar)){
		fprintf(asmfile, "\tpush\teax\n");
		if(isMul(nextChar)){
			READCHAR;
			factor(srcfile, asmfile);
			fprintf(asmfile, "\tpop\tebx\n");
			fprintf(asmfile, "\timul\teax,\tebx\n");
		}
		else if(isDiv(nextChar)){
			READCHAR;
			factor(srcfile, asmfile);
			fprintf(asmfile, "\tmov\tebx,\teax\n");
			fprintf(asmfile, "\tpop\teax\n");
			fprintf(asmfile, "\txor\tedx,\tedx\n");
			fprintf(asmfile, "\tidiv\tebx\n");
		}
		else {
			EXPECTED( "multiplication or division operator" );
		}
		REMOVEWHITESPACE;
	}
	return 0;
}

int factor(FILE *srcfile, FILE *asmfile){

	char *numLit = malloc(20*sizeof(char));
	REMOVEWHITESPACE;

	if(nextChar == '('){	// Compound expression
		READCHAR;	// Throw away the '('
		expression(srcfile, asmfile);
		if (nextChar == ')'){
			READCHAR;	// Throw away the ')'
		}else{
			EXPECTED( ")" );
		}
	}
	else { // Else get a numerical literal
		if(isAddOp(nextChar)){
			// Allow for e.g. -1 or +2 as a numerical literal
			if(isAdd(nextChar)){
				READCHAR;
				GETNUM( numLit );
				fprintf(asmfile, "\tmov\teax,\t%s\n", numLit);
			} else if (isSub(nextChar)){
				READCHAR;
				GETNUM( numLit );
				fprintf(asmfile, "\tmov\teax,\t%s\n", numLit);
				fprintf(asmfile, "\tneg\teax\n");
			}
		} else {
			GETNUM( numLit );
			fprintf(asmfile, "\tmov\teax,\t%s\n", numLit);
		}
	}
	free(numLit);
	return 0;
}

inline int isDigit(const char inChar){
  if (inChar >= '0' && inChar <= '9') return 1;
  else return 0;
}

inline int isWhitespace(const char inChar){
	if (inChar == '\t' || inChar == '\n' || inChar == ' ') return 1;
	else return 0;
}

inline int isAdd(const char inChar){
	if (inChar == '+') return 1;
	else return 0;
}

inline int isSub(const char inChar){
	if (inChar == '-') return 1;
	else return 0;
}

inline int isAddOp(const char inChar){
	if (inChar == '+' || inChar == '-') return 1;
	else return 0;
}

inline int isMul(const char inChar){
	if (inChar == '*') return 1;
	else return 0;
}

inline int isDiv(const char inChar){
	if (inChar == '/') return 1;
	else return 0;
}

inline int isMulOp(const char inChar){
	if (inChar == '*' || inChar == '/') return 1;
	else return 0;
}
