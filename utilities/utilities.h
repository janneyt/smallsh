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
/** LINESIZE has to support 512 whitespace/character pairs */
# define LINESIZE   1046
# define DELIMITER  " \t\n"

char* util_setenv(char* env_var, char* new_val)
	/**
	 * \brief Sets the environment variable *with error checking*
	 *
	 * @param env_ver a string the signifies which environment variable needs to be set
	 * @param new_val the new value for the environment variable you are changing
	 * 
	 * @return Returns the old variable if it is identical to the new value, returns the new variable if setting the variable was successful, and exits entirely if writing to the environment fails
	 * */

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
}
