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

#define GETNUM( numLit ) { REMOVEWHITESPACE; if(!isDigit(nextChar)) EXPECTED("integer"); charPtr = numLit; while(isDigit(nextChar)) { *charPtr = nextChar; charPtr++; READCHAR; } }

#define REMOVEWHITESPACE { while(isWhitespace(nextChar)) READCHAR; }

#define EXPECTED( expr ) { fprintf(stderr, "Expected %s\n", expr); return -1; }

int expression(FILE *srcfile, FILE *asmfile){
	term(srcfile, asmfile);
	fprintf(asmfile, "\tmov\tebx,\teax\n");
	REMOVEWHITESPACE;
	if(isAdd(nextChar)){
		READCHAR;
		term(srcfile, asmfile);
		fprintf(asmfile, "\tadd\teax,\tebx\n");
	}
	else if(isSub(nextChar)){
		READCHAR;
		term(srcfile, asmfile);
		fprintf(asmfile, "\tsub\tebx,\teax\n");
		fprintf(asmfile, "\tmov\teax,\tebx\n");
	}
	else {
		EXPECTED( "math operator" );
	}
	return 0;
}

int term(FILE *srcfile, FILE *asmfile){

	char *numLit = malloc(20*sizeof(char));

	GETNUM( numLit );
	fprintf(asmfile, "\tmov\teax,\t%s\n", numLit);
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
