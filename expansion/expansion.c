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

int spec_expansion(char arg[LINESIZE], char substring[3], int control_code){
	/**
	 * \brief Expands four variables based on the specifications
	 *
	 * @param string is the string that needs to be scanned for expansion variables
	 *
	 * @return EXIT_SUCCESS for a successful scan (no expansion variables left) and and replace, EXIT_FAILURE for an unsuccessful scan or replace
	 * */
	int length = strlen(arg);
	char *discovery = strstr(arg, substring);
	if(discovery == NULL){
		return EXIT_SUCCESS;
	};
	char *temporary[LINESIZE];
	strcpy(temporary, discovery);
	char str_pid[10];
	if(length < 1 || length > LINESIZE){
		fprintf(stderr, "The length is not in the range 1 and %d includsive", LINESIZE);
		return EXIT_FAILURE;
	};

	while(discovery != NULL){
		if(control_code == 1){
			util_int_to_string(getpid(), str_pid, 10);
		} else if(control_code == 2){
			strcpy(str_pid, "");
			util_env_var_to_fixed_array("HOME", str_pid);
		};
		*discovery = '\0';
		strcat(arg, str_pid);
		temporary++;
		temporary++;
		discovery = strstr(arg, substring);
		strcat(arg, temporary);
	};
	
	if(control_code == 1){
		return spec_expansion(arg, "~/", 2);
	}
	return EXIT_SUCCESS;

}
