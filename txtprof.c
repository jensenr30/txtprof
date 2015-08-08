#include <stdio.h>
#include <string.h>
#include "txtprof.h"
#include <stdlib.h>

//==============================================================================
// these function log errors
//==============================================================================

/// logs string to file
void txtprof_log(char *string){
	
	FILE *log = fopen(TXTPROF_LOG_FILE_NAME, "a");
	if(log != NULL) fprintf(log, "%s\n", string);
	fclose(log);
	
}


/// logs string and integer to file
void txtprof_log_d(char *string, int data){
	
	FILE *log = fopen(TXTPROF_LOG_FILE_NAME, "a");
	if(log != NULL) fprintf(log, "%s\t%d\n", string, data);
	fclose(log);
	
}


/// logs string and another string to file
void txtprof_log_s(char *string, char *data){
	
	FILE *log = fopen(TXTPROF_LOG_FILE_NAME, "a");
	if(log != NULL) fprintf(log, "%s\t%s\n", string, data);
	fclose(log);
	
}


//==============================================================================
// these functions interpret input commands and data files
//==============================================================================

/// This will interpret the flags/options from the command line
int txtprof(int argc, char *argv[]){
    
    //--------------------------------------------------------------------------
    // parse input arguments
    //--------------------------------------------------------------------------
    
    // these are general purpose for/while loop variables
    int i;
    
    // these variable keep track of what the user want to do
    // by default, we assume the user doesn't want to do anything.
    // As we process the input arguments, we will figure out what the user
    // wants to do...
    char *input_files[TXTPROF_MAX_INPUT_FILES];
    int inputs = 0;
    char *save_profile = NULL;
    char *load_profile = NULL;
    long long unsigned generate = 0;
    char *output_file = NULL;
    
    // print what the arguments were
    if (TXTPROF_DEBUG) {
		printf("input arguments are:\n\n");
		for (i = 0; i < argc; i++) {
			printf("arg_%2d\t\"%s\"\n",i,argv[i]);
		}
		printf("\n");
    }
    
    // loop through all arguments and interpret them
    i = 1; // skip the first argument, which is the keyprof file
    while ( i < argc) {
		// check if it was a 
		if (!strcmp(argv[i],TXTPROF_ARG_G) || !strcmp(argv[i],TXTPROF_ARG_GENERATE)) {
			// record how many characters you want to generate
			generate = atoi(argv[i+1]);
			// processed two arguments
			i = i + 2;
		}
		else if	(!strcmp(argv[i],TXTPROF_ARG_O) || !strcmp(argv[i],TXTPROF_ARG_OUTPUT_FILE) ) {
			// record where you want to save the generated text
			output_file = argv[i+1];
			// skip the argument after the option.
			i = i + 2;;
		}
		else if	(!strcmp(argv[i],TXTPROF_ARG_S) || !strcmp(argv[i],TXTPROF_ARG_SAVE_PROFILE)) {
			// record where you want to save the text profile to
			save_profile = argv[i+1];
			// processed two arguments
			i = i + 2;
		}
		else if	(!strcmp(argv[i],TXTPROF_ARG_L) || !strcmp(argv[i],TXTPROF_ARG_LOAD_PROFILE)) {
			// record where you want to load the text profile from
			load_profile = argv[i+1];
			// processed two arguments
			i = i + 2;
		}
		else {
			// if you have not read in too many files already,
			if (inputs < TXTPROF_MAX_INPUT_FILES) {
				// record the names of the input files you want to read
				input_files[inputs] = argv[i];
				// advance the inputs index
				inputs++;
				// processed one argument
				i = i + 1;
			}
			else {
				printf("Exceeded input file limit! Max input files is %d.\n\
						Did not open %s\n",TXTPROF_MAX_INPUT_FILES,argv[i]);
				// processed one argument
				i = i + 1;
			}
		}
    }
    
    // print how you interpreted the arguments
    if (TXTPROF_DEBUG) {
		
		// print all of the input files that were found
		for (i = 0; i < inputs; i++) {
			printf("Input File %3d:  \"%s\"\n",i+1,input_files[i]);
		}
		if (inputs < 1) {
			printf("No input files!");
		}
		// print the save profile file
		if (save_profile != NULL) {
			printf("Save Profile: \"%s\"\n",save_profile);
		}
		else {
			printf("Save Profile: NULL\n");
		}
		// print the number of characters the user wants to generate
		printf("\nGenerate Characters: %llu\n",generate);
		// print the output file
		if (output_file != NULL) {
			printf("Output File: \"%s\"\n",output_file);
		}
		else {
			printf("Printing to command line!\n");
		}
    }
    
    //--------------------------------------------------------------------------
    // load profile
    //--------------------------------------------------------------------------
    
    // create an empty profile
    struct text_profile myProf;
	profile_erase(&myProf);
	// load a profile if the user requested it
	// if the user wanted to load a profile AND process input files, the profile
	// will be loaded, and then the input file(s) will be processed and the data
	// from that(them) will be added to the loaded profile.
	if (load_profile != NULL) {
		
		profile_load(&myProf, load_profile);
	
	}
	
	//--------------------------------------------------------------------------
	// process input files and update profile
	//--------------------------------------------------------------------------
	
    char pc = '\0';		// previous character
    char cc = '\0';		// current character
    // this keep track of how many characters were found in a given file
    long long unsigned charsFound;
    
    // for every input file
    for (i = 0; i < inputs; i++) {
		printf("Processing input %d/%d...\n",i,inputs);
		// attemp to open the file for reading
		FILE *fp = fopen(input_files[i],"r");
		// if the file was not opened,
		if (fp == NULL) {
			// inform the user
			printf("Could not open file: \"%s\"\n",input_files[i]);
			// move to the next input file
			continue;
		}
		
		// now that the file is opened, initialize the previous character with
		// the first character from it.
		pc = fgetc(fp);
		// if the first character was an EOF,
		if (pc == EOF) {
			// that file was a dud.
			printf("Empty File: \"%s\"\n",input_files[i]);
			// close the file
			fclose(fp);
			// move to the next input file
			continue;
		}
		
		charsFound = 0;
		// search the entire 
		while ( ( cc = fgetc(fp)) != EOF ) {
			// record the occurrence
			myProf.occur[(unsigned char)pc][(unsigned char)cc]++;
			charsFound++;
		}
		
		// print file stats
		printf("Processed %llu characters from File: \"%s\"\n",charsFound, input_files[i]);
		// close the file
		fclose(fp);
		
    }
    
    //--------------------------------------------------------------------------
    // save profile
    //--------------------------------------------------------------------------
    
    // save the profile if the user requested it
    if (save_profile != NULL) {
		// if you get a zero back from the function,
		if( !profile_save(&myProf, save_profile) ){
			// that means the save was successful
			printf("Saved text_profile to file: \"%s\"\n",save_profile);
		}
		// otherwise,
		else {
			// the save faile
			printf("Couldn't save text_profile to file: \"%s\"",save_profile);
		}	
    }
    
    //--------------------------------------------------------------------------
    // generate text from profile
    //--------------------------------------------------------------------------
    
	txtprof_generate(&myProf,generate,output_file);
	
	return 0;
}


/// this will erase the profile (to zeros)
int profile_erase(struct text_profile *pro){
	
	// see if the profil is invalid
	if (pro == NULL) {
		txtprof_log("profile_erase() received NULL profile");
		return -1;
	}
	
	// assuming the profile is valid, wipe it.
	int i,j;
	for (i = 0; i < TXTPROF_CHARACTERS; i++) {
		for ( j = 0; j < TXTPROF_CHARACTERS; j++) {
			pro->occur[i][j] = 0;
		}
	}
	
	return 0;
}



/// this saves a profile to a text file in .csv format
int profile_save(struct text_profile *pro, char *filename){
	
	// see if the profil is invalid
	if (pro == NULL) {
		txtprof_log("profile_save() received NULL profile");
		return -1;
	}
	
	// attempt to open the file for writing
	FILE *fp = fopen(filename,"w+");
	// make sure the file opened
	if (fp == NULL) {
		txtprof_log_s("profile_save() could not open file for writing: ",filename);
		return -2;
	}
	
	// print the csv header
	fprintf(fp,"given_char,following_char,occurrences\n");
	
	int i, j;
	// print all the data from the 
	for (i = 0; i < TXTPROF_CHARACTERS; i++) {
		for (j = 0; j < TXTPROF_CHARACTERS; j++) {
			fprintf(fp,"%d,%d,%llu\n",i,j,pro->occur[i][j]);
		}
	}
	
	// close up shop
	fclose(fp);
	return 0;
	
}



/// this loads a profile from a text file in .csv format
int profile_load(struct text_profile *pro, char *filename){
	
	// see if the profil is invalid
	if (pro == NULL) {
		txtprof_log("profile_load() received NULL profile");
		return -1;
	}
	
	
	// attempt to open the filename for reading
	FILE *fp = fopen(filename,"r");
	// make sure the fie opened
	if (fp == NULL) {
		txtprof_log_s("profile_load() could not open file for reading: ",filename);
		return -2;
	}
	
	/// TODO: write load code
	
	
	
	return 0;
}


int txtprof_generate(struct text_profile *pro, long long unsigned gen, char *filename){
	
	// see if the profil is invalid
	if (pro == NULL) {
		txtprof_log("txtprof_generate() received NULL profile");
		return -1;
	}
	
	char close = 1;
	// if the filename was NULL,
	FILE *fp = NULL;
	if (filename == NULL) {
		// just print the shit to the terminal
		fp = stdout;
		close = 0;
	}
	// otherwise, if the user wants to print to a specific file,
	else {
		// try to open/create it
		fp = fopen(filename,"a+");
		// make sure the fie opened
		if (fp == NULL) {
			txtprof_log_s("profile_load() could not open file for reading: ",filename);
			return -2;
		}
	}
	
	/// TODO: write generation code. parse the profile to 
	
	// close the file, if you weren't printing to stdout.
	if (close) {
		fclose(fp);
	}
	
	return 0;
}
