# define LINESIZE 		1046
# define DELIMITER 		" \t\n"
# define STRINGSIZE 		100
# define NUMCHILDREN		6
# include 			<time.h>

typedef int make_iso_compilers_happy;

extern int heap_size = 0;

struct ProgArgs {
	char command[STRINGSIZE];
	char input[STRINGSIZE];
	char output[STRINGSIZE];
	int background;
	time_t timestamp;
};


struct ParentStruct {
	int pid_counter;
	int pid;
	struct ProgArgs *array[NUMCHILDREN];
};

