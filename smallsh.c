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

/* Error Functions */

int err_child_error(char child_action[], char action_taken[]){
	/**
	 * \brief Produces an error because a child process has completed in a systematic way.
	 *
	 * @param child_action is a string of arbitrary size that represents *what* caused completion
	 * @param action_taken is a string of arbtrary size that represents *what was the result* such as continuing or exiting.
	 *
	 * *Writes to stderr*
	 *
	 * @return EXIT_SUCCESS if process successfully writes to stderr, EXIT_FAILURE if there was an error while writing
	 */
	if(fprintf(stderr, "Child process %s. %s.", child_action, action_taken) < 0){
		perror("Could not print error");
		errno = 0;
		return EXIT_FAILURE;
	};
	return (EXIT_SUCCESS);
}

/* Utility functions */

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
	for(int i = 0; i < LINESIZE; i++){
		memset(&storage[i], '\0',1);	
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

int spec_check_for_child_background_processes(int status, pid_t pid){
	/**
	 * \brief Checks if a pid in the same process group id has completed either normally or in various abnormal ways.
	 * 
	 * @param status is the status of the pid in question
	 * @param pid is the process id of the process in question
	 *
	 * *Writes to stderr*
	 *
	 * `check_for_child_background_processes` is required to meet a specification requirement
	 *
	 * @return EXIT_FAILURE if printing to stderr didn't work, EXIT_SUCCESS if printing to stderr works.
	 */
	if (WIFEXITED(status)){
		if(fprintf(stderr, "Child process %jd done. Exit status %d.\n", (intmax_t) pid, WEXITSTATUS(status)) < 0){
			perror("Could not print error");
			errno = 0;
			return EXIT_FAILURE;
		};
	}
	else if (WIFSIGNALED(status)){
		assert(!WIFEXITED(status));
		if(fprintf(stderr, "Child process %jd done. Signaled %d.\n", (intmax_t) pid, WTERMSIG(status)) < 0){
			perror("Could not print error");
			errno = 0;
			return EXIT_FAILURE;
		};

		
	}
	else if (WIFSTOPPED(status)) {
		if(kill(pid, SIGCONT) != 0){
			assert(!WIFEXITED(status) && !WIFSIGNALED(status));
			perror("Kill didn't send a signal to continue, exiting");
			exit(EXIT_FAILURE);
		};
		// kill() never returns above 0, so initializing it to 1 indicates kill has not run
		int result = 1;
		if((result = kill(pid, SIGCONT)) != 0){
			perror("Kill didn't work");
			errno = 0;
			return EXIT_FAILURE;
		}
		if(fprintf(stderr, "Child process %jd stopped. Continuing.\n", (intmax_t) pid) < 0){
			perror("Could not print");
			assert(result == 0);
		}
		if(fprintf(stderr, "Child process %jd stopped. Continuing.\n", (intmax_t) pid) < 0){
			perror("Could not print error");
			errno = 0;
			return EXIT_FAILURE;
		};

	}
	assert(!WIFSTOPPED(status) && !WIFEXITED(status) && !WIFSIGNALED(status));
	return EXIT_SUCCESS;
}

int spec_get_line(char input[LINESIZE], size_t input_size, FILE* stream){
	/**
	 * \brief Gets the input and returns it. Has it's own signal handling.
	 * 
	 * @param input[] a character array 
	 * @param input_size is the size of the input *array*
	 * @param stream is a FILE* file pointer and allows for redirection.
	 *
	 * *Be careful to not overrun input*
	 *
	 * *This is a specification requirement function*
	 * @return EXIT_FAILURE if any part of the function errors, EXIT_SUCCESS if there's a valid input (and the full input) 
	 * in the passed input parameter
	 *
	 * */
	
	// input_length currently is zero, as nothing is entered. This would be the same as entering an empty string, so be careful
	ssize_t input_length = 0;

	// This program is constantly setting and resetting errno, so if errno has a value it hasn't been caught elsewhere
	if(errno != 0){
		errno = 0;
	};
	assert(errno == 0);

	// PS1 print
	printf("$");
	if(( input_length = getline(&input, &input_size, stream)) < 0){
		perror("Cannot fetch line from input");
		clearerr(stream);
		errno = 0;
		return EXIT_FAILURE;
	};
	assert(input_length >= 0);
	return EXIT_SUCCESS;
}

char** help_split_line(char** storage, char* line){
	/**
	 * \brief Helper function that tokenizes a line into an array of words
	 *
	 * @param line an array, with delimiters, of words needing to be tokenized
	 * @param storage is an array of points needed to hold the tokenized words
	 * @return an array of tokens
	 * */
	int 	bufsize = LINESIZE;
	int 	position = 0;
	char*	token;
	char*	delim = getenv("IFS");
	int	token_bufsize = 64;
	char**  array_of_tokens = storage;

	delim = (delim != 0x0) ? delim : DELIMITER;
	
	if(bufsize < 1){
		printf("A buffer of 1 or more is needed for tokenization");
		exit(EXIT_FAILURE);
	};

	// TODO: variable expansion needed here
	
	token = strtok(line, delim);
	while(token != NULL){
		array_of_tokens[position] = token;
		position++;
		if(position >= bufsize){
			bufsize += token_bufsize;
			if((array_of_tokens = realloc(array_of_tokens, bufsize * sizeof(char*))) == NULL){
				perror("Cannot reallocate memory for array of tokens");
				exit(EXIT_FAILURE);
			};
			
		}
		token = strtok(NULL, delim);
	};

	array_of_tokens[position] = NULL;
	return array_of_tokens;

} 

int spec_word_splitting(char* storage[LINESIZE], char input[LINESIZE]){
	/**
	 * \brief Splits a word into tokens and fills a passed char** with each token
	 *
	 * @param storage is a char** with space to hold LINESIZE sized tokens
	 * @param input is a char[LINESIZE] with @min an empty string and @max a single string of size LINESIZE
	 * 
	 * @return EXIT_SUCCESS if sentence can be tokenized, EXIT_FAILURE if it can't
	 * */

	if(strlen(input) < 1){
		fprintf(stderr, "1.\n%s\n", input);
		return EXIT_FAILURE;
	};
	storage = help_split_line(storage, input);
	return EXIT_SUCCESS;
}

int test_input(){
	char* storage[LINESIZE];
	char* discardable;
	char* old_IFS;
	//char  str[LINESIZE];

	/** ### Utility Function Testing */

	char* test_input[LINESIZE];
	util_reset_storage(test_input);
	assert(test_input[0] == NULL);
	/** ### Spec Function Testing */

	// TODO: implement test case 1, where the exit command closes the shell

	// TODO: implement test case 2, where EOF closes the shell
	
	// TODO: implement test case 4, where stdin is interrupted
	
	// TODO: setup and run child processes for test cases 6-21
	
	// TODO: interrupt spec_get_line with signal
	
	char input[LINESIZE];
	size_t input_size = LINESIZE-1;

	// Test Case 5: Errno is not zeroed out, make sure function can reset it
	errno = -11;
	spec_get_line(input, input_size, stdin);
	if(errno != 0){
		printf("\n***Failed test case 5: errno is set incorrectly before function is called***\n");		
	};
	errno = 0;

	// Test Case 25: Send correct input with a file pointer and it returns the correct input
	
	// Word Splitting Test Case TODO: IFS is set to NULL
	
	// Save old IFS to restore later
	old_IFS = util_setenv("IFS",NULL);
	*storage = input;
	strcpy(input, "Ted is here");
	assert(spec_word_splitting(storage, input) == EXIT_SUCCESS);
	assert(strcmp(*storage, "Ted") == 0 );
	assert(strcmp(storage[1], "is") == 0);
	assert(strcmp(storage[2], "here") == 0);

	// Word Splitting Test Case 1: the length of the inputted string is zero
	char* empty_str = "";
	memset(input, '\0', 1);
	strcat(input, empty_str);
	assert(spec_word_splitting(storage, input) == EXIT_FAILURE);
	
	// Word Splitting Test Case 3: the inputted string is a null byte
	printf("Test case 3\n");
	strcat(input, "\0");
	assert(spec_word_splitting(storage, input) == EXIT_FAILURE);

	// Word Splitting Test Case 4: "EOF" as a string is passed as input
	printf("Test case 4\n");
	strcat(input, "EOF");
	assert(spec_word_splitting(storage, input) == EXIT_SUCCESS);
	assert(strcmp(*storage, "EOF") == 0);

	util_reset_storage(storage);

	// Word Splitting Test Case 6: "NULL" is passed as input
	printf("Test case 6\n");
	strcpy(input, "NULL");
	assert(spec_word_splitting(storage, input) == EXIT_SUCCESS);
	assert(strcmp(*storage, "NULL") == 0);
	
	util_reset_storage(storage);		
	
	// Word Splitting Test Case 10: IFS is NULL, length of input is 512, LINESIZE is 0, input length array is size 1, the word passed is "Ted"
	printf("Test case 10\n");
	fflush(stdout);
	fflush(stderr);

	strcpy(input, "Ted");
	assert(spec_word_splitting(storage, input) == EXIT_SUCCESS);
	assert(strcmp(storage[0], "Ted") == 0);
	assert(storage[1] == 0x0);
	util_reset_storage(storage);

	// Word Splitting Test Case 11: IFS is NULL, sentence passed is "Ted is"
	printf("Test case 11\n");
	fflush(stdout);
	fflush(stderr);
	strcpy(input, "Ted is");
	assert(spec_word_splitting(storage, input) == EXIT_SUCCESS);
	assert(strcmp(storage[0], "Ted") == 0);
	assert(strcmp(storage[1], "is") == 0);
	util_reset_storage(storage);

	// Word Splitting Test Case 12: IFS is NULL, length is 512, sentence is a 512 character long word
	printf("Test case 12\n");
	fflush(stdout);
	fflush(stderr);
	strcpy(input, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqr");
	assert(spec_word_splitting(storage, input) == EXIT_SUCCESS);
	assert(strcmp(storage[0], input) == 0);
	assert(storage[1] == 0x0);
	util_reset_storage(storage);

	// Word Splitting Test Case 13: IFS is NULL, length is 513 (above minimum required), sentence is a 513 character long word
	printf("Test case 13\n");
	fflush(stdout);
	fflush(stderr);
	strcpy(input, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghaijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqr");
	assert(spec_word_splitting(storage, input) == EXIT_SUCCESS);
	assert(strcmp(storage[0], input) == 0);
	assert(storage[0][512] == 'r');
	assert(storage[1] == 0x0);
	util_reset_storage(storage);

	// Word Splitting Test Case 14: IFS is NULL, length is 512, sentence is 512 words
	printf("Test case 14\n");
	fflush(stdout);
	fflush(stderr);
	strcpy(input, "AA B C D E F G H I J K L M N W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z ");
	assert(spec_word_splitting(storage, input) == EXIT_SUCCESS);
	assert(strcmp(storage[0], "AA") == 0);
	assert(strcmp(storage[1], "B") == 0);
	assert(strcmp(storage[512], "r") == 0);
	assert(strcmp(storage[520], "z") == 0);
	assert(storage[521] == 0x0);
	util_reset_storage(storage);


	// Word Splitting Test Case 15: IFS is NULL, length is 513 (more than required minimum), sentence is 513 words
	printf("Test case 15\n");
	fflush(stdout);
	fflush(stderr);
	strcpy(input, "A A B C D E F G H I J K L M N W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z ");
	assert(spec_word_splitting(storage, input) == EXIT_SUCCESS);
	assert(strcmp(storage[0], "A") == 0);
	assert(strcmp(storage[1], "A") == 0);
	assert(strcmp(storage[521], "z") == 0);
	assert(storage[522] == 0x0);
	util_reset_storage(storage);

	// Word Splitting Test Case 16: IFS is NULL, length is 512, sentence is 512 words, no trailing whitespace
	printf("Test case 16\n");
	fflush(stdout);
	fflush(stderr);
	strcpy(input, "AA B C D E F G H I J K L M N W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z");
	assert(spec_word_splitting(storage, input) == EXIT_SUCCESS);
	assert(strcmp(storage[0], "AA") == 0);
	assert(strcmp(storage[1], "B") == 0);
	assert(strcmp(storage[520], "z") == 0);
	util_reset_storage(storage);

	// Word Splitting Test Case 17: IFS is NULL, length is 513, sentence is 513 words, no trailing whitespace
	
	printf("Test case 17\n");
	fflush(stdout);
	fflush(stderr);
	strcpy(input, "A A B C D E F G H I J K L M N W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z");
	assert(spec_word_splitting(storage, input) == EXIT_SUCCESS);
	assert(strcmp(storage[0], "A") == 0);
	assert(strcmp(storage[1], "A") == 0);
	assert(strcmp(storage[521], "z") == 0);
	util_reset_storage(storage);

	// Word Splitting Test Case 18: IFS is \t\n, length is 512, sentence is 512 words, no trailing whitespace
	
	if((discardable = util_setenv("IFS", " \t\n")) == NULL){
		printf("Cannot set IFS from %s to %s. Restart shell.", "NULL", "\t\n");
		exit(EXIT_FAILURE);
	};

	// Word Splitting Test Case 12b: IFS is \t\n, length is 512, sentence is a 512 character long word
	printf("Test case 12b\n");
	fflush(stdout);
	fflush(stderr);
	strcpy(input, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqr");
	assert(spec_word_splitting(storage, input) == EXIT_SUCCESS);
	assert(strcmp(storage[0], input) == 0);
	assert(storage[1] == 0x0 );
	util_reset_storage(storage);

	// Word Splitting Test Case 13b: IFS is \t\n, length is 513 (above minimum required), sentence is a 513 character long word
	printf("Test case 13b\n");
	fflush(stdout);
	fflush(stderr);
	strcpy(input, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghaijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqr");
	assert(spec_word_splitting(storage, input) == EXIT_SUCCESS);
	assert(strcmp(storage[0], input) == 0);
	assert(storage[1] == 0x0);
	util_reset_storage(storage);

	// Word Splitting Test Case 18
	printf("Test case 18\n");
	fflush(stdout);
	fflush(stderr);
	strcpy(input, "AA B C D E F G H I J K L M N W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z");
	assert(spec_word_splitting(storage, input) == EXIT_SUCCESS);
	assert(strcmp(storage[0], "AA") == 0);
	assert(strcmp(storage[1], "B") == 0);
	assert(strcmp(storage[520], "z") == 0);
	assert(storage[521] == 0x0);
	util_reset_storage(storage);
	
	// Word Splitting Test Case 19: IFS is \t\n, length is 512, sentence is 512 words, trailing whitespace
	printf("Test case 19\n");
	fflush(stdout);
	fflush(stderr);
	strcpy(input, "AA B C D E F G H I J K L M N W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z ");
	assert(spec_word_splitting(storage, input) == EXIT_SUCCESS);
	assert(strcmp(storage[0], "AA") == 0);
	assert(strcmp(storage[1], "B") == 0);
	assert(strcmp(storage[520], "z") == 0);
	assert(storage[521] == 0x0);
	util_reset_storage(storage);

	// Word Splitting Test Case 20: IFS is \t\n, length is 513, sentence is 513 words, trailing whitespace
	printf("Test case 20\n");
	fflush(stdout);
	fflush(stderr);
	strcpy(input, "A A B C D E F G H I J K L M N W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z ");
	assert(spec_word_splitting(storage, input) == EXIT_SUCCESS);
	assert(strcmp(storage[0], "A") == 0);
	assert(strcmp(storage[1], "A") == 0);
	assert(strcmp(storage[521], "z") == 0);
	assert(storage[522] == 0x0);
	util_reset_storage(storage);

	// Word Splitting Test Case 21: IFS is \t\n, length is 513, sentence is 513 words, trailing whitespace
	printf("Test case 21\n");
	fflush(stdout);
	fflush(stderr);
	strcpy(input, "A A B C D E F G H I J K L M N W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z ");
	assert(spec_word_splitting(storage, input) == EXIT_SUCCESS);
	assert(strcmp(storage[0], "A") == 0);
	assert(strcmp(storage[1], "A") == 0);
	assert(strcmp(storage[521], "z") == 0);
	assert(storage[522] == 0x0);
	util_reset_storage(storage);

	// Word Splitting Test Case 22: IFS is \t\n, length is 1, sentence is 1 word, no trailing whitespace
	printf("Test case 22\n");
	fflush(stdout);
	fflush(stderr);
	strcpy(input, "i");
	assert(spec_word_splitting(storage, input) == EXIT_SUCCESS);
	assert(strcmp(storage[0], "i") == 0);
	assert(storage[1] == 0x0);
	util_reset_storage(storage);

	// Word Splitting Test Case 23: IFS is \t\n, length is 2, sentence is 1 word, trailing whitespace
	printf("Test case 23\n");
	fflush(stdout);
	fflush(stderr);
	strcpy(input, "i ");
	assert(spec_word_splitting(storage, input) == EXIT_SUCCESS);
	assert(strcmp(storage[0], "i") == 0);
	assert(storage[1] == 0x0);
	util_reset_storage(storage);


	// Word Splitting Test Case 24: IFS is " ", length is 512, sentence is 512 words, no trailing whitespace
	if((discardable = util_setenv("IFS", " ")) == old_IFS){
		printf("Can't set IFS from %s to empty strings", old_IFS);
		exit(EXIT_FAILURE);
	};

	// Word Splitting Test Case 12c: IFS is " ", length is 512, sentence is a 512 character long word
	printf("Test case 12c\n");
	fflush(stdout);
	fflush(stderr);
	strcpy(input, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqr");
	assert(spec_word_splitting(storage, input) == EXIT_SUCCESS);
	assert(strcmp(storage[0], input) == 0);
	assert(storage[1] == 0x0);
	util_reset_storage(storage);

	// Word Splitting Test Case 13c: IFS is " ", length is 513 (above minimum required), sentence is a 513 character long word
	printf("Test case 13c\n");
	fflush(stdout);
	fflush(stderr);
	strcpy(input, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghaijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqr");
	assert(spec_word_splitting(storage, input) == EXIT_SUCCESS);
	assert(strcmp(storage[0], input) == 0);
	assert(storage[1] == 0x0);
	util_reset_storage(storage);

	// Test Case 24
	printf("Test case 24\n");
	fflush(stdout);
	fflush(stderr);
	strcpy(input, "AA B C D E F G H I J K L M N W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z");
	assert(spec_word_splitting(storage, input) == EXIT_SUCCESS);
	assert(strcmp(storage[0], "AA") == 0);
	assert(strcmp(storage[1], "B") == 0);
	assert(strcmp(storage[520], "z") == 0);
	assert(storage[521] == 0x0);
	util_reset_storage(storage);

	// Word Splitting Test Case 19: IFS is " ", length is 512, sentence is 512 words, trailing whitespace
	printf("Test case 19\n");
	fflush(stdout);
	fflush(stderr);
	strcpy(input, "AA B C D E F G H I J K L M N W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z ");
	assert(spec_word_splitting(storage, input) == EXIT_SUCCESS);
	assert(strcmp(storage[0], "AA") == 0);
	assert(strcmp(storage[1], "B") == 0);	
	assert(strcmp(storage[520], "z") == 0);
	assert(storage[521] == 0x0);
	util_reset_storage(storage);

	// Word Splitting Test Case 20: IFS is " ", length is 513, sentence is 513 words, trailing whitespace
	printf("Test case 20\n");
	fflush(stdout);
	fflush(stderr);
	strcpy(input, "A A B C D E F G H I J K L M N W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z ");
	assert(spec_word_splitting(storage, input) == EXIT_SUCCESS);
	assert(strcmp(storage[0], "A") == 0);
	assert(strcmp(storage[1], "A") == 0);
	assert(strcmp(storage[521], "z") == 0);
	assert(storage[522] == 0x0);
	util_reset_storage(storage);

	// Word Splitting Test Case 21: IFS is " ", length is 513, sentence is 513 words, trailing whitespace
	printf("Test case 21\n");
	fflush(stdout);
	fflush(stderr);
	strcpy(input, "A A B C D E F G H I J K L M N W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h a i j k l m n o p q r s t u v w x y z ");
	assert(spec_word_splitting(storage, input) == EXIT_SUCCESS);
	assert(strcmp(storage[0], "A") == 0);
	assert(strcmp(storage[1], "A") == 0);
	assert(strcmp(storage[521], "z") == 0);
	assert(storage[522] == 0x0);
	util_reset_storage(storage);

	// Word Splitting Test Case 22: IFS is " ", length is 1, sentence is 1 word, no trailing whitespace
	printf("Test case 22\n");
	fflush(stdout);
	fflush(stderr);
	strcpy(input, "i");
	assert(spec_word_splitting(storage, input) == EXIT_SUCCESS);
	assert(strcmp(storage[0], "i") == 0);
	assert(storage[1] == 0x0);
	util_reset_storage(storage);

	// Word Splitting Test Case 23: IFS is " ", length is 2, sentence is 1 word, trailing whitespace
	printf("Test case 23\n");
	fflush(stdout);
	fflush(stderr);
	strcpy(input, "i ");
	assert(spec_word_splitting(storage, input) == EXIT_SUCCESS);
	assert(strcmp(storage[0], "i") == 0);
	assert(storage[1] == 0x0);
	util_reset_storage(storage);


	if(old_IFS == 0x0){
		old_IFS = " ";
	};
	if(setenv("IFS", old_IFS, 0) != 0){
		perror("Setting IFS failed");
		exit(EXIT_FAILURE);
	};
	if(strcmp(getenv("IFS"),old_IFS) != 0){

		printf("Something went wrong resetting the IFS:*%s*, verify it is only stored once in the old_IFS:*%s* variable",getenv("IFS"), old_IFS);
		exit(EXIT_FAILURE);
	};
	return EXIT_SUCCESS;
}

int main(void){
	/**
	 * \brief Main calling function that exclusively calls other functions *required to meet specifications*
	 *
	 * @return Exits to EXIT_FAILURE if utility functions such as print fails, otherwise exits to EXIT_SUCCESS when appropriate signal is sent
	 */

	pid_t pid;
	int status;

	// Variables needed for prompts declared outside the infinite loop to not create memory leaks
	char line[LINESIZE];
	size_t line_size = LINESIZE-1;
	
	// Runtime debug testing to make sure functions act according to how I want
	if(test_input() == EXIT_FAILURE){
		printf("Input functions fail runtime tests\n");
		exit(EXIT_FAILURE);
	};

	for(;;){

		// Specification requirement to identify child processes potentially exiting between loops
		while((pid = waitpid(0, &status, WNOHANG)) > 0) /** Waitpid set to 0 to make parent wait for child sharing the process group id */ {
			if(spec_check_for_child_background_processes(status, pid) != EXIT_SUCCESS){
				exit(EXIT_FAILURE);
			}
		}

		if(pid == 0){
			printf("Children exist but have not yet completed");
		} else if(pid < 0){
			perror("Error looking for children");
		};

		assert(pid < 1);

		// TODO: turn on a signal handler so that any signal prints a newline and for loop continues
		// Print prompt TODO: change stdin to file stream variable when implemented
		if(spec_get_line(line, line_size, stdin) == EXIT_SUCCESS){
			// Only in the singular case where we can verify input has succeeded are we heading out to any other function
			
		} else {
			printf("error with spec_get_line");
			exit(EXIT_FAILURE);
		};
		clearerr(stdin); //TODO: change stdin to the file stream variable when implemented
		// TODO: turn off custom signal handler
		// Nothing goes below this line. Without input, there's no point in continuing processing. Instead, start the infinite loop again.
	}


	exit(EXIT_SUCCESS);
}
