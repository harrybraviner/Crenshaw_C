#include "compile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *charPtr;
char nextChar;

#define READCHAR { nextChar = fgetc(srcfile); }

#define REMOVEWHITESPACE { while(isWhitespace(nextChar)) READCHAR; }

int compile(FILE *srcfile, FILE *asmfile){

	// Emit some sort of header
	fprintf(asmfile, "segment\t.data\n\n");
	fprintf(asmfile, "segment\t.bss\n");
	// FIXME - temporary line for testing variables before we can do assignments
	fprintf(asmfile, "var1\tresd\t1\n");
	fprintf(asmfile, "retval\tresd\t1\n");
	fprintf(asmfile, "\nsegment\t.text\n");
	fprintf(asmfile, "\tglobal\tasm_main\n");
	fprintf(asmfile, "asm_main:\n");
	fprintf(asmfile, "\tenter\t0,\t0\n");
	fprintf(asmfile, "\tpusha\n");
	// FIXME - temporary line for testing variables before we can do assignments
	fprintf(asmfile, "\tmov\t[var1],\tdword 10\n");
	fprintf(asmfile, "\n;\tBeginning of BASIC code\n\n");

	READCHAR;
	while(nextChar != (char)EOF){
		if(expression(srcfile, asmfile)) return -1;
		READCHAR;
		REMOVEWHITESPACE;
	}

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

#define GETTOKEN( token ) { REMOVEWHITESPACE; if(isDigit(nextChar)){ GETNUM( token );} else if(isAlpha(nextChar)){ GETVAR( token ); HANDLEKEYWORD( token );} else EXPECTED("a token")}
#define GETNUM( numLit ) { REMOVEWHITESPACE; if(!isDigit(nextChar)) EXPECTED("integer"); charPtr = numLit; while(isDigit(nextChar)) { *charPtr = nextChar; charPtr++; READCHAR; } *charPtr = '\0'; }
#define GETVAR( var ) { REMOVEWHITESPACE; if(!isAlpha(nextChar)) EXPECTED("character"); charPtr = var; *charPtr = '['; charPtr++; while(isAlphaNum(nextChar)) { *charPtr = nextChar; charPtr++; READCHAR; } *charPtr = ']'; charPtr++; *charPtr = '\0'; }
#define HANDLEKEYWORD( token ) {	if(!strcmp(token, "[let]")){ char *varName = malloc(20*sizeof(char)); REMOVEWHITESPACE; GETVAR( varName ); REMOVEWHITESPACE; if (nextChar != '=') {EXPECTED(" = operator");} else { READCHAR; expression(srcfile, asmfile); fprintf(asmfile, "\tmov\t%s,\teax\n", varName); }; strcpy(token, varName); free(varName); } }

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
	if(nextChar != '\n' && nextChar != (char)EOF && nextChar != ')' && nextChar != ';')
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

	char *token = malloc(20*sizeof(char));
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
			// Allow for e.g. -1 or +2, or -x as a token
			if(isAdd(nextChar)){
				READCHAR;
				GETTOKEN( token );
				fprintf(asmfile, "\tmov\teax,\t%s\n", token);
			} else if (isSub(nextChar)){
				READCHAR;
				GETTOKEN( token );
				fprintf(asmfile, "\tmov\teax,\t%s\n", token);
				fprintf(asmfile, "\tneg\teax\n");
			}
		} else {
			GETTOKEN( token );
			fprintf(asmfile, "\tmov\teax,\t%s\n", token);
		}
	}
	free(token);
	return 0;
}

inline int isDigit(const char inChar){
  if (inChar >= '0' && inChar <= '9') return 1;
  else return 0;
}

inline int isAlpha(const char inChar){
  if ((inChar >= 'a' && inChar <= 'z') ||
	    (inChar >= 'A' && inChar <= 'Z')) return 1;
  else return 0;
}

inline int isAlphaNum(const char inChar){
  if ((inChar >= 'a' && inChar <= 'z') ||
	    (inChar >= 'A' && inChar <= 'Z') ||
			(inChar >= '0' && inChar <= '9')) return 1;
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
