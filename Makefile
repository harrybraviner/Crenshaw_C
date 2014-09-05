compiler : Crenshaw_C.c compile.o compile.h
	gcc -Wall Crenshaw_C.c compile.o -o compiler

compile.o : compile.c compile.h
	gcc -Wall -c compile.c -o compile.o
