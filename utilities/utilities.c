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
# include <errno.h>
# include <stdbool.h>

# define STRINGSIZE 100
# ifndef LINESIZE
# include "../constants/constants.h"
# endif

/* Utility functions */

bool util_check_environ(const char* string){
	if(string == NULL || (strlen(string) == 0)){
		return false;
	}
	char** env = environ;
	for(; (*env != 0x0 && *env != NULL); env++){
		int length = strlen(string);
		if(strncmp(*env, string, length) == 0){
			return true;
			
		}
	}
	return false;
}

int util_env_var_to_fixed_array(const char env_var[LINESIZE], char fixed_array[LINESIZE]){
	/**
	 * \brief Takes an environment variable and sets it in a fixed array after error checking
	 *
a	 * @param env_var is the environment variable
	 * @param fixed_array is the fixed array
8	 *
	 * @return EXIT_FAILURE if either an error is returned from getenv or the returned value does not fit into fixed_array, EXIT_SUCCESS if both getenv returns not null and the returned value fits into fixed_array
	 * */

	if(!util_check_environ(env_var)){
		return EXIT_FAILURE;
	}
	const char* temp = getenv(env_var);
	if(temp == NULL){
		return EXIT_FAILURE;
	} else {
		strcpy(fixed_array, temp);
		strcat(fixed_array, "");
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;

}

char* util_setenv(char* env_var, char* new_val){

	// Save old IFS to restore later
	char* old_var = getenv(env_var);
	if(old_var == new_val){
		return old_var;		
	};
	if(setenv(env_var,new_val, 1) != 0){
		perror("Could not set env for %s to NULL. Shell needs to be restarted.");
		exit(EXIT_FAILURE);
	};
	return getenv(env_var);


}

void util_reset_storage(char* storage[LINESIZE]){
	/**
	 * \brief resets a char** of size LINESIZE to a null terminated first byte
	 *
	 * @param storage is a char** with each memory address pointing to an array of size LINESIZE.
	 *
	 * *Memset does not return error codes*
	 *
	 * @return void as storage is released to calling function
	 * */
	const size_t length = LINESIZE/sizeof(storage[0]);

	for(size_t i = 0; i < length; i++){
		storage[i] = 0x0;
	};
}

int util_int_to_string(int num, char str[LINESIZE], int size){
	
	/**
	 * \brief Converts an integer to its strings representation
	 *
	 * @param num is the integer that needs to be converted to a string
	 * @param str is the holder string that stores the integer representation
	 * @param size is the maximum length possible to store in str, including \0
	 * @return EXIT_SUCCESS if integer has been entirely converted to 
	 * string, EXIT_FAILURE if integer cannot be convered to string
	 *
	 */
	

	char temp[LINESIZE];

	if(sprintf(temp, "%d", num) < 0){
		perror("Could not convert integer to string");
		errno = 0;
		return EXIT_FAILURE;
	};
	strcat(temp, "");
	assert(atoi(temp) == num);

	strcat(str, "");
	strncpy(str, temp, size);
	assert(atoi(str) == num);
	return EXIT_SUCCESS;
}
