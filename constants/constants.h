# define LINESIZE 		1046
# define DELIMITER 		" \t\n"
# define STRINGSIZE 		100
# define NUMCHILDREN		6
# include 			<time.h>
# include			<stdint.h>

typedef int make_iso_compilers_happy;

typedef struct {
	char* command[STRINGSIZE];
	char input[STRINGSIZE];
	char output[STRINGSIZE];
	int background;
} ProgArgs;


typedef struct {
	char last_foreground[STRINGSIZE];
	char last_background[STRINGSIZE];
} ParentStruct;

