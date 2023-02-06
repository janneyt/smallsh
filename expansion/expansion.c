/**\brief Need this to compile with kill() and -std=c99 */
# define _POSIX_SOURCE
# define _GNU_SOURCE
# define _POSIC_C_SOURCE >= 200112L

# include <stdio.h>
# include <sys/types.h>
# include <signal.h>
# include <unistd.h>
# include <sys/wait.h>
# include <string.h>
# include <stdlib.h>
# include <assert.h>
# include <stdint.h>
# include "../utilities/utilities.h"
# include "../error/error.h"
# include "../tester/tester.h"
# ifndef  LINESIZE
# include "../constants/constants.h"
# endif

# include <stdlib.h>
# include <errno.h>

# define STRINGSIZE 100
/** LINESIZE has to support 512 whitespace/character pairs */
# define DELIMITER  " \t\n"

int spec_expansion(char string[LINESIZE]){
	/**
	 * \brief Expans four variables based on the specifications
	 *
	 * @param string is the string that needs to be scanned for expansion variables
	 *
	 * @return EXIT_SUCCESS for a successful scan (no expansion variables left) and and replace, EXIT_FAILURE for an unsuccessful scan or replace
	 * */
	int length = strlen(string);
	if(length < 1 || length > LINESIZE){
		fprintf(stderr, "The length is not in the range 1 and %d includsive", LINESIZE);
		return EXIT_FAILURE;
	};
	return EXIT_SUCCESS;

}
