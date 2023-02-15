# include <stdlib.h>
# include <stdio.h>
# ifndef LINESIZE
# include "../constants/constants.h"
# endif

int spec_parsing(char string[LINESIZE], ProgArgs* current);
	/**
	 * \brief Takes a *non-tokenized* string, tokenizes it, validates it, and sets internal arguments for execution of parent and child processes
	 *
	 * @param The character array with the commandline arguments
	 *
	 * @return EXIT_SUCCESS if every step completes, else EXIT_FAILURE for any step failing
	 * */

