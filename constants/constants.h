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
	time_t timestamp;
	pid_t pid;
	int status;
} ProgArgs;


typedef struct {
	int pid_counter;
	int pid;
	ProgArgs* heap[NUMCHILDREN];
	pid_t last_foreground;
	pid_t last_background;
} ParentStruct;

