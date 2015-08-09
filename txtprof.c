#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "txtprof.h"
#include "jentils.h"
#include <time.h>

// by default, the user probably does not want to debug txtprof
int g_txtprof_debug = 0;

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
    
    // loop through all arguments and interpret them
    i = 1; // skip the first argument, which is the keyprof file
    while ( i < argc) {
		if (!strcmp(argv[i],TXTPROF_ARG_DEBUG)) {
			// turn on verbose debugging messages
			g_txtprof_debug = 1;
			// processed one argument
			i++;
		}
		else if (!strcmp(argv[i],TXTPROF_ARG_H) || !strcmp(argv[i],TXTPROF_ARG_HELP)) {
			// help the user
			txtprof_help();
			// processed one argument
			i++;
		}
		else if	(!strcmp(argv[i],TXTPROF_ARG_L) || !strcmp(argv[i],TXTPROF_ARG_LOAD_PROFILE)) {
			// record where you want to load the text profile from
			load_profile = argv[i+1];
			// processed two arguments
			i = i + 2;
		}
		else if	(!strcmp(argv[i],TXTPROF_ARG_S) || !strcmp(argv[i],TXTPROF_ARG_SAVE_PROFILE)) {
			// record where you want to save the text profile to
			save_profile = argv[i+1];
			// processed two arguments
			i = i + 2;
		}
		// check if it was a 
		else if (!strcmp(argv[i],TXTPROF_ARG_G) || !strcmp(argv[i],TXTPROF_ARG_GENERATE)) {
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
    
    // print what the arguments were
    if (g_txtprof_debug) {
		printf("input arguments are:\n\n");
		for (i = 0; i < argc; i++) {
			printf("arg_%2d\t\"%s\"\n",i,argv[i]);
		}
		printf("\n");
    }
    
    // print how you interpreted the arguments
    if (g_txtprof_debug) {
		
		// print all of the input files that were found
		for (i = 0; i < inputs; i++) {
			printf("Input File %3d:  \"%s\"\n",i+1,input_files[i]);
		}
		if (inputs < 1) {
			printf("No input files!\n");
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
			printf("Output File: NULL - printing any generated text to command line...\n");
		}
    }
    
    //--------------------------------------------------------------------------
    // load profile
    //--------------------------------------------------------------------------
    
    // create a text profile object
    struct text_profile myProf;
	// load a profile from file if the user requested it
	// if the user wanted to load a profile AND process input files, the profile
	// will be loaded, and then the input file(s) will be processed and the data
	// from that(them) will be added to the loaded profile.
	if (load_profile != NULL) {
		
		profile_load(&myProf, load_profile);
		
	}
	// if there is no profile data to load,
	else {
		// initialize the profile to zeros
		profile_erase(&myProf);
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
		
		if (g_txtprof_debug) printf("Processing input %d/%d...\n",i+1,inputs);
		
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
			// record the "current char" in the "previous char" varibable
			// for the next iteration through this loop
			pc = cc;
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
			printf("Couldn't save text_profile to file: \"%s\"\n",save_profile);
		}	
    }
    
    //--------------------------------------------------------------------------
    // generate text from profile
    //--------------------------------------------------------------------------
    
	txtprof_generate(&myProf,generate,output_file);
	
	return 0;
}


/// this displays helpful information to the user
void txtprof_help(void){
	printf("\n");//	  80 printed characters on the command line is right at the pipe ->|
	printf("usage:  txtprof [optional input files] <operations>                         \n");
	printf("\n");//	  80 printed characters on the command line is right at the pipe ->|
	printf("options:                                                                    \n");
	printf("  -h, --help                   I've fallen and I can't get up               \n");
	printf("  -l, --load <file>            specify the text profile to load             \n");
	printf("  -s, --save-profile <file>    specify where to save the text profile       \n");
	printf("  -g, --generate <number>      specify the number of characters to generate \n");
	printf("  -o, --output-file <file>     specify where to print generated text        \n");
	printf("      --debug                  enable verbose debugging msgs (adults only)  \n");
	printf("\n");//	  80 printed characters on the command line is right at the pipe ->|
	printf("examples:                                                                   \n");
	printf("  $ txtprof a_good_book.txt -g 1000                                         \n");
	printf("        This demonstrates the most basic operation txtprof does. txtprof    \n");
	printf("        will read a_good_book.txt, generate a 'text profile' based on the   \n");
	printf("        characters in that file, and then generate 1000 new characters that \n");
	printf("        roughly fit the patterns observed in a_good_book.txt. Because this  \n");
	printf("        command does NOT specify the output-file, the generated characters  \n");
	printf("        are printed to the command line.                                    \n");
	printf("  $ txtprof my_file.txt -g 20000 -o nonesense.txt                           \n");
	printf("        This will generate 20000 characters based on the content of         \n");
	printf("        my_file.txt. Those characters will be printed to the nonesense.txt  \n");
	printf("  $ txtprof Dune.txt -s dune_profile.txt                                    \n");
	printf("        This command generates a text profile from Dune.txt and saves it in \n");
	printf("        dune_profile.txt for later use.                                     \n");
	printf("  $ txtprof -l dune_profile.txt -g 777                                      \n");
	printf("        This command reads the text profile from dune_profile.txt which had \n");
	printf("        been saved earlier. After dune_profile.txt has been loaded, 777     \n");
	printf("        characters will be printed to the terminal.                         \n");
	printf("  $ txtprof DuneMessiah.txt -l dune_profile.txt -s dune_1_and_2_profile.txt \n");
	printf("        This creates a profile from DuneMessiah.txt and combines it with the\n");
	printf("        existing profile from dune_profile.txt. It saves the combined text  \n");
	printf("        profile to dune_1_and_2_profile.txt                                 \n");
	printf("  $ txtprof a.txt b.txt c.txt -s abc_prof.txt -g 2500 -o abc_gen.txt        \n");
	printf("        This command will read all three text files a.txt, b.txt, and c.txt \n");
	printf("        and save a text profile based on the combined texts to abc_prof.txt \n");
	printf("        In addition, 2500 characters will be generated and written to       \n");
	printf("        abc_gen.txt                                                         \n");
	printf("  $ txtprof 500_characters.txt -g 10000 -o school_report.txt                \n");
	printf("        This demonstrates the concept of up-sampling                        \n");
	printf("  $ txtprof english.txt -g 5000 -o mock_english.txt                         \n");
	printf("  $ txtprof mock_english.txt -g 5000 -o mock_mock_english.txt               \n");
	printf("        These two commands show the concept of iteration (or \"feedback\"). \n");
	printf("        In the first command, A text profile is created from english.txt    \n");
	printf("        (something assumed to be written in english). This profile is used  \n");
	printf("        to generate mock_english.txt. Then in the second command, a text    \n");
	printf("        profile is created from mock_english.txt and is used to create      \n");
	printf("        mock_mock_english.txt. This iterative process could continue        \n");
	printf("        indefinitely, but I have a feeling that after the first iteration it\n");
	printf("        it would cease to be interesting. Please prove me wrong.     -Jensen\n");
	printf("\n");//	  80 printed characters on the command line is right at the pipe ->|
	// exmples I want to cover in the help menu:
	// done		description	
	//	+		open file; generate text
	//	+		open file; generate text to a file
	//	+		open file; generate profile; save profile to file
	//	+		load profile from file; generate text (no new text file required)
	//	+		load data from text file AND previously saved profile; generate characters
	//	+		load data from multiple text files; generate characters
	//	+		demonstrate the concept of up-sampling
	//	+		load data from text file; generate a new textfile that will be 
				// nonesense; then load data from that newly generated textfile 
				// and make a second text file; basically, suggest feeding the
				// output back into the input and collect the ensuing data.	
	
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
	// print all the NON-ZERO data from the profile
	for (i = 0; i < TXTPROF_CHARACTERS; i++) {
		for (j = 0; j < TXTPROF_CHARACTERS; j++) {
			// only print profile occurrences if they are non-zero.
			// This reduces the file size A LOT
			if (pro->occur[i][j] != 0) {
				fprintf(fp,"%d,%d,%llu\n",i,j,pro->occur[i][j]);
			}
		}
	}
	
	// close up shop
	fclose(fp);
	return 0;
	
}



/// this loads a profile from a text file in .csv format
/// be ware! this function will overwrite whatever is in the profile you send it!
int profile_load(struct text_profile *pro, char *filename){
	
	// see if the profile is invalid
	if (pro == NULL) {
		txtprof_log("profile_load() received NULL profile");
		return -1;
	}
	
	// erase profile object to zeros before loading profile from file
	profile_erase(pro);
	
	// attempt to open the filename for reading
	FILE *fp = fopen(filename,"r");
	// make sure the fie opened
	if (fp == NULL) {
		txtprof_log_s("profile_load() could not open file for reading: ",filename);
		return -2;
	}
	
	// skip the first line (the header)
	fp = skipchar(fp,'\n');
	
	int pc;
	int cc;
	long long unsigned occur;
	while (!feof(fp)) {
		// input the "previous character"
		fscanf(fp,"%d,",&pc);
		// input the "current  character"
		fscanf(fp,"%d,",&cc);
		// input the occurrences
		fscanf(fp,"%llu\n",&occur);
		// store the loaded occurrence value in the profile.
		pro->occur[(unsigned char)pc][(unsigned char)cc] = occur;
	}
	
	
	// close up shop
	fclose(fp);
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
	
	long long unsigned i;
	// generate the first character
	char c = txtprof_gen_char(pro);
	// print the first character
	fputc(c,fp);
	// generate and printthe rest of the characters
	for (i = 1; i <gen; i++) {
		// gen next character
		c = txtprof_gen_char_next(pro, (unsigned char)c);
		// print it
		fputc(c,fp);
	}
	
	// close the file, if you weren't printing to stdout.
	if (close) {
		fclose(fp);
	}
	return 0;
}


/// this gets you a random character from a profile.
// this should only be used to get the first character, as it is really slow.
// this sums up all of the occurrences of characters in the profile and picks
// one using rand() based on weighted probabilities.
// This is a lot of adding, and should be only used to generate a starting
// point. After you get your first random character, you can use the other
// function, "txtprof_gen_char_next()" which will operate much faster.
char txtprof_gen_char(struct text_profile *pro){
	
	// see if the profil is invalid
	if (pro == NULL) {
		txtprof_log("txtprof_gen_char() received NULL profile");
		return -1;
	}
	
	// this keeps track of whether or not you have used this function before.
	// This ensures that srand doesn't keep getting called continuously.
	static char been_here_before = 0;
	if(!been_here_before) {
		// get a "random" seed from the current ime
		srand(time(NULL));
		been_here_before = 1;
	}
	
	// this is what we will use to keep track of how many times each character
	// appears, regardless of what comes before or after it.
	long long unsigned occur_total[TXTPROF_CHARACTERS];
	// this keeps track of how many characters there are in total (all chars)
	long long unsigned occur_all = 0;
	unsigned i, j;
	
	// sum all characters
	for (i = 0; i < TXTPROF_CHARACTERS; i ++) {
		// initialize each total to zero before adding.
		occur_total[i] = 0;
		// sum up the total number of times that the [i]th character occurs.
		for (j = 0; j < TXTPROF_CHARACTERS; j++) {
			occur_total[i] += pro->occur[i][j];
		}
		occur_all += occur_total[i];
	}
	
	// if you didn't find any character occurrences (which means the profile is
	// FUCKING EMPTY) then just fuck it and return e.
	if (occur_all == 0) {
		return 'e';
	}
	
	// find a random number less than the total number of all the characters found
	long long unsigned random_char = rand()%occur_all;
	long long unsigned running_total = 0;
	
	for (i = 0; i < TXTPROF_CHARACTERS; i++) {
		running_total += occur_total[i];
		if (random_char < running_total) {
			return (char)i;
		}
	}
	
	// you should never got to this point in the program
	// but if for some insane reason you do, return something
	// I chose e because I hope that it is in every text document
	// but like I said, this point SHOULD never be reached, so this function 
	// will probably never have to return e.
	txtprof_log_d("txtprof_gen_char() FAILED! couldn't get first random char! random_char =",(int)random_char);
	return 'e';
	
}


/// this will return a character based on the text profile and the character you sent it.
// the character it chooses is random (meaning it uses the rand() function), but
// the calculation is based on the weighted averages of the probabilities of 
// the characters that have been seen to follow the input character.
// for example, if the text profile is based on the phrase:
//		"hello"
// and you send this function an 'e', it MUST return an 'l'.
// however, if you sent this function an 'l', there would be a 50% chance it
// would return an 'o', and a 50% chance it would return an 'l' back to you.
char txtprof_gen_char_next(struct text_profile *pro, unsigned char c){
	
	// see if the profil is invalid
	if (pro == NULL) {
		txtprof_log("txtprof_gen_char_next() received NULL profile");
		return -1;
	}
	
	// this keeps track of how many times this specific character has occurred in total
	long long unsigned occur_all = 0;
	unsigned i;
	
	// sum all characters
	for (i = 0; i < TXTPROF_CHARACTERS; i ++) {
		occur_all += pro->occur[c][i];
	}
	
	// if you didn't find any characters that come after the one you were sent,
	// then, shit, I guess just try to find a new one from the entire text profile.
	if (occur_all == 0) {
		return txtprof_gen_char(pro);
	}
	
	// find a random number less than the total number of all the characters found
	long long unsigned random_char = rand()%occur_all;
	long long unsigned running_total = 0;
	
	for (i = 0; i < TXTPROF_CHARACTERS; i++) {
		running_total += pro->occur[c][i];
		if (random_char < running_total) {
			return (char)i;
		}
	}
	
	
	// the program should never reach this point.
	// the notes I wrote for "return e" in the txtprof_gen_char function.
	return 'e';
}
