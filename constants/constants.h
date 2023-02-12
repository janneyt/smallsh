# define LINESIZE 		1046
# define DELIMITER 		" \t\n"
# define STRINGSIZE 		100
# define NUMCHILDREN		6

typedef int make_iso_compilers_happy;

struct ProgArgs {
	char command[STRINGSIZE];
	char input[STRINGSIZE];
	char output[STRINGSIZE];
	int background;
};


struct ParentStruct {
	int pid_counter;
	int pid;
	struct ProgArgs *array[NUMCHKLDREN];
}

