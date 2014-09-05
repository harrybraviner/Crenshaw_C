#include <stdio.h>
#include "compile.h"

FILE *infile, *outfile, *asmfile, *driverfile;
char *out_filename, *asm_filename, *driver_filename, *mainobjfilename;
char *systemCall;

void emitDriver(FILE *driverfile);

int main(int argc, char *argv[]){
	// Get input and possibly output filenames from the commandline and open for reading / writing
	if (argc < 2 || argc > 3){
		fprintf(stderr, "Incorrect usage! Correct usage is:\n./compiler input_filename [output_filename] \n");
		return -1;
	}

	infile = fopen(argv[1], "r");
	if(infile == NULL){
		fprintf(stderr, "Unable to open %s for reading. Quitting...\n", argv[1]);
		return -1;
	}

	if (argc==2) out_filename = "a.out";
	else         out_filename = argv[2];

	// For now, just use a default asm filename
	asm_filename = "out.asm";
	asmfile = fopen(asm_filename, "w");
	if(asmfile == NULL){
		fprintf(stderr, "Unable to open %s for writing. Quitting...\n", asm_filename);
		fclose(infile);
		return -1;
	}

	// And the same for the driver filename
	driver_filename = "driver.c";
	driverfile = fopen(driver_filename, "w");
	if(driverfile == NULL){
		fprintf(stderr, "Unable to open %s for writing. Quitting...\n", driver_filename);
		fclose(asmfile);
		fclose(infile);
		return -1;
	}

	/*
	outfile = fopen(out_filename, "w");
	if(outfile == NULL){
		fprintf(stderr, "Unable to open %s for writing. Quitting...\n", out_filename);
		fclose(driverfile);
		fclose(asmfile);
		fclose(infile);
		return -1;
	}
	*/

	// Emit the driver code to the driver file
	emitDriver(driverfile);
	fclose(driverfile);

	// Actual compiling is handled by another function
	compile(infile, asmfile);
	fclose(asmfile);

	// Compile the assembler
	mainobjfilename = "out.o";
	systemCall = malloc(300 * sizeof(char));
	sprintf(systemCall, "nasm -f elf %s -o %s", asm_filename, mainobjfilename);
	system(systemCall);

	// Compiler the driver
	sprintf(systemCall, "gcc %s %s -o %s", mainobjfilename, driver_filename, out_filename);
	system(systemCall);

	// Exit normally
	fclose(infile);
	//fclose(outfile);
	return 0;

}

void emitDriver(FILE *driverfile){
	fprintf(driverfile, "#include <stdio.h>\n\nint main(int argc, char *argv[]){\n\tint ret_status;\n\tret_status = asm_main();\n\treturn ret_status;\n}\n");
}
