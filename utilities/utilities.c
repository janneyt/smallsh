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

# include <stdlib.h>
# include <errno.h>

# define STRINGSIZE 100
# ifndef  LINESIZE
# include "../constants/constants.h"
# endif

/* Utility functions */

int util_env_var_to_fixed_array(char env_var[LINESIZE], char fixed_array[LINESIZE]){
	/**
	 * \brief Takes an environment variable and sets it in a fixed array after error checking
	 *
	 * @param env_var is the environment variable
	 * @param fixed_array is the fixed array
	 *
	 * @return EXIT_FAILURE if either an error is returned from getenv or the returned value does not fit into fixed_array, EXIT_SUCCESS if both getenv returns not null and the returned value fits into fixed_array
	 * */
	char* temp = "";
	strcpy(fixed_array, "");
	if(env_var != NULL && strlen(env_var) > 0){
		temp = getenv(env_var);
		if(strcmp(temp, "") == 0){
			return EXIT_FAILURE;
		};
		if(strlen(temp) > LINESIZE){
			return EXIT_FAILURE;
		};
		if(strcmp(strcpy(fixed_array, getenv(env_var)), "") != 0){
			return EXIT_SUCCESS;
		};
		return EXIT_FAILURE;
		
	};
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
		memset(&storage[i], '\0', LINESIZE * sizeof(char));	
	};
}

int util_int_to_string(int num, char* str, int size){
	
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
	

	char *temp = malloc(size);
	
	memset(str, 0, sizeof(*str));
	if(sprintf(temp, "%d", num) < 0){
		perror("Could not convert integer to string");
		errno = 0;
		free(temp);
		return EXIT_FAILURE;
	};
	assert(atoi(temp) == num);
	if(strlen(str) > strlen(temp)){
		perror("Input string was too small to hold integer to string conversion value");
		errno = 0;
		free(temp);
		return EXIT_FAILURE;
	};
	assert(strlen(str) <= strlen(temp));
	if(strcat(str, temp) == NULL){
		perror("Could not convert integer to string");
		errno = 0;
		free(temp);
		return EXIT_FAILURE;
	};
	assert(atoi(str) == num);
	free(temp);
	return EXIT_SUCCESS;
}
