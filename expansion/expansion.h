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
int spec_expansion(char string[LINESIZE], char substring[3], int control_code, ParentStruct *parent);
