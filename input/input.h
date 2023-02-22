# include <stdio.h>
# include <sys/types.h>
# include <signal.h>
# include <unistd.h>
# include <sys/wait.h>
# include <string.h>
# include <stdlib.h>
# include <assert.h>
# include <stdint.h>
# ifndef LINESIZE
# include "../constants/constants.h"
# endif
# include <stdlib.h>
# include <errno.h>

int spec_check_for_child_background_processes(ParentStruct* parent);
int spec_get_line(char input[LINESIZE], size_t input_size, FILE* stream, int control_code);
char** help_split_line(char** storage, char* line);
int spec_word_splitting(char* storage[LINESIZE], char input[LINESIZE]);

