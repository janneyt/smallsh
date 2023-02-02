# include <stdio.h>
# include <sys/types.h>
# include <signal.h>
# include <unistd.h>
# include <sys/wait.h>
# include <string.h>
# include <stdlib.h>
# include <assert.h>
# include <stdint.h>

# include <stdlib.h>
# include <errno.h>
int spec_expansion(char string[LINESIZE]){
	/**
	 * \brief Expands four variables based on the specifications.
	 *
	 * @param string is the string that needs to be scanned for expansion variables
	 *
	 * @return EXIT_SUCCESS for a successful scan and replace, EXIT_FAILURE
	 * for an unsuccessful scan or replace
	 * */

	int length = strlen(string);
	if(length < 1 || length > LINESIZE){
		fprintf(stderr, "The length is not in the range 1 and %d inclusive", LINESIZE);
		return EXIT_FAILURE;
	};
	


}
