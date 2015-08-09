#include "jentils.h"

/// this will advance a FILE pointer to the character immidiately after the target.
// you could use this to navigate to the next line in a text file
// if this function encounters an EOF, the pointer will point to the end of the file.
// this function checks for EOF and the character you specify
FILE *skipchar(FILE *fp, char targ){
	
	// verify the caller isn't pulling your leg
	if (fp == NULL) {
		printf("jentils.c nextline() received a NULL file pointer");
		return NULL;
	}
	
	// get the first character
	char c = fgetc(fp);
	// keep advancing the file pointer until you find a newline, or the EOF.
	while (c != EOF && c != targ) {
		c = fgetc(fp);
	}
	
	// give the caller their file back, but now it points to the next line.
	return fp;
}


//int max_min_int(int *myarray, int mysize);
