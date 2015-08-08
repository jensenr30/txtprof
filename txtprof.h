//==============================================================================
// general definitions
//==============================================================================
#define TXTPROF_LOG_FILE_NAME "keyboard_log.txt"
#define TXTPROF_DEBUG 0
#define TXTPROF_MAX_INPUT_FILES 256
#define TXTPROF_CHARACTERS 256


//==============================================================================
// Command Line Usage
//==============================================================================

// basic usage:
//	$	txtprof filename.txt [options...]

// profile "my_book.txt" and generate 300 characters into "gen_text.txt"
//	$	txtprof my_book.txt -g 300 -o gen_text.txt
//	$	txtprof my_book.txt --generate 300 --output-file gen_text.txt

// profile two books and print 500 character to the screen
//	$	txtprof book1.txt book2.txt -g 500

// profile "my_code.c" and write the profile to "my_code_profile.txt"
//	$	txtprof my_code.c -s my_code_profile.txt
//	$	txtprof my_code.c --save-profile my_code_profile.txt

// the argument following either of these will be interpreted as the file to
// which the profile of the text should be saved
#define TXTPROF_ARG_S "-s"
#define TXTPROF_ARG_SAVE_PROFILE "--save-profile"

// the argument following either of these will be interpreted as the file from
// which a profile should be loaded
#define TXTPROF_ARG_L "-l"
#define TXTPROF_ARG_LOAD_PROFILE "--load-profile"

// the argument following either of these will be interpreted as the number
// of characters to generate
#define TXTPROF_ARG_G "-g"
#define TXTPROF_ARG_GENERATE "--generate"

// The arument following either of these will be interpreted as the output
// file name to which the generated text will be written
#define TXTPROF_ARG_O "-o"
#define TXTPROF_ARG_OUTPUT_FILE "--output-file"


//==============================================================================
// PROFILE STRUCTURE
//==============================================================================

/// the profile just keeps track of which characters follow which characters
// profile data is stored in this format:
// occur[last_char][current_char]
// in this way, the first array dimension is indexed by the preceeding
// character, and the second array dimension is indexed by the character
// that follows it.
// for example, if you are processing the word "hi", you would use
// occur['h']['i']++;
// to record that you found an occurrence of the character h leading to the
// character i.
#define TXTPROF_PROFILE_TYPE long long unsigned
struct text_profile {
	TXTPROF_PROFILE_TYPE occur[TXTPROF_CHARACTERS][TXTPROF_CHARACTERS];
};



//==============================================================================
// FUNCTIONS 
//==============================================================================

int txtprof(int argc, char *argv[]);
int txtprof_generate(struct text_profile *pro, long long unsigned gen, char *filename);
int profile_erase(struct text_profile *pro);
int profile_save(struct text_profile *pro, char *filename);
int profile_load(struct text_profile *pro, char *filename);
