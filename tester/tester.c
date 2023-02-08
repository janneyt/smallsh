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
# include "../input/input.h"
# include "../expansion/expansion.h"

#ifndef LINESIZE
# include "../constants/constants.h"
#endif

# include <stdlib.h>
# include <errno.h>

int test_parsing(void){
	char string[LINESIZE] = "";
	char result[LINESIZE] = "";

	// Test Case 1
	assert(spec_parsing(string) == EXIT_SUCCESS);
	assert(strcmp(string, result) == 0);

	// Test Case 2
	strcpy(string, "&");
	assert(spec_parsing(string) == EXIT_FAILURE);

	// Test Case 3
	strcpy(string, "#");
	assert(spec_parsing(string) == EXIT_SUCCESS);

	// Test Case 4
	strcpy(string, "ps");
	assert(spec_parsing(string) == EXIT_SUCCESS);
	strcpy(string, "ps&");
	assert(spec_parsing(string) == EXIT_FAILURE);
	assert(prog_args.background == False);

	// Test Case 6
	strcpy(string, "< >");
	assert(spec_parsing(string) == EXIT_FAILURE);
	assert(strcmp(prog_args.input, "") == 0);
	assert(strcmp(prog_args.output, "") == 0);
	strcpy(string, "<>");
	assert(spec_parsing(string) == EXIT_FAILURE);
	assert(strcmp(prog_args.input, "") == 0);
	assert(strcmp(prog_args.output, "") == 0);

	// Test Case 7
	strcpy(string, "< #");
	assert(spec_parsing(string) == EXIT_FAILURE);
	assert(strcmp(prog_args.input, "") == 0);
	strcpy(string, "<#");
	assert(spec_parsing(string) == EXIT_FAILURE);
	assert(strcmp(prog_args.input, "") == 0);

	// Test Case 8
	strcpy(string, "> #");
	assert(spec_parsing(string) == EXIT_FAILURE);
	assert(strcmp(prog_args.output, "") == 0);
	strcpy(string, ">#");
	assert(spec_parsing(string) == EXIT_FAILURE);
	assert(strcmp(prog_args.output, "") == 0);

	// Test Case 9
	strcpy(string, "# <");
	assert(spec_parsing(string) == EXIT_SUCCESS);
	assert(strcmp(prog_args.input, "") == 0);

	// Test Case 10
	strcpy(string, "< &");
	assert(spec_parsing(string) == EXIT_FAILURE);
	assert(strcmp(prog_args.input, "") == 0);
	strcpy(string, "<&");

	// Test Case 11
	strcpy(string, "< & #");
	assert(spec_parsing(string) == EXIT_FAILURE);
	strcpy(string, "<& #");
	assert(spec_parsing(string) == EXIT_FAILURE);
	strcpy(string, "< &#");
	assert(spec_parsing(string) == EXIT_FAILURE);
	
	// Test Case 12
	strcpy(string, "< file1.txt");
	assert(spec_parsing(string) == EXIT_SUCCESS);
	assert(strcmp(prog_args.input, "file1.txt") == 0);
	strcpy(string, "<file1.txt");
	assert(spec_parsing(string) == EXIT_FAILURE);

	// Test Case 16
	strcpy(string, "> file1.txt");
	assert(spec_parsing(string) == EXIT_SUCCESS);
	assert(strcmp(prog_args.output, "file1.txt") == 0);
	strcpy(string, ">file1.txt");
	assert(spec_parsing(string) == EXIT_FAILURE);

	// Test Case 18
	strcpy(string, "ps &");
	assert(spec_parsing(string) == EXIT_SUCCESS);
	assert(prog_args.background == True);
	strcpy(string, "& ps");
	assert(spec_parsing(string) == EXIT_FAILURE);

	// Test Case 20
	strcpy(string, "cd ..");
	assert(spec_parsing(string) == EXIT_SUCCESS);
	assert(prog_args.background == False);
	strcpy(string, "cd ls");
	assert(spec_parsing(string) == EXIT_SUCCESS);
	assert(prog_args.background == False);
	
	// Test Case 22
	strcpy(string, "ps < file1.txt > file1.txt");
	assert(spec_parsing(string) == EXIT_SUCCESS);
	strcpy(string, "ps > file1.txt < file1.txt");
	assert(spec_parsing(string) == EXIT_SUCCESS);
	assert(strcmp(prog_args.input, "file1.txt") == 0);
	assert(strcmp(prog_args.output, "file1.txt") == 0);
	strcpy(string, "ls ps < > file1.txt");
	assert(spec_parsing(string) == EXIT_FAILURE);
	strcpy(string, "< file1.txt > file1.txt ps");
	assert(spec_parsing(string) == EXIT_FAILURE);

	// Test Case 23
	strcpy(string, "< file1.txt > file1.txt #");
	assert(spec_parsing(string) == EXIT_SUCCESS);
	assert(strcmp(prog_args.input, "file1.txt") == 0);
	assert(strcmp(prog_args.output, "file1.txt") == 0);
	strcpy(string, "> file1.txt < file1.txt #");
	assert(spec_parsing(String) == EXIT_SUCCESS);
	assert(strcmp(prog_args.input, "file1.txt") == 0);
	assert(strcmp(prog_args.output, "file1.txt") == 0);

	// Test Case 24
	strcpy(string, "cd pwd ps < file1.txt");
	assert(spec_parsing(string) == EXIT_SUCCESS);
	assert(strcmp(prog_args.input, "file1.txt") == 0);
	assert(strcmp(prog_args.output, "") == 0);
	strcpy(string, "cd pwd < file1.txt ps");
	assert(spec_parsing(string) == EXIT_FAILURE);

	// Test Case 25
	strcpy(string, "ls ps < file25.txt #");
	assert(spec_parsing(string) == EXIT_SUCCESS);
	assert(strcmp(prog_args.input, "file25.txt") == 0);
	strcpy(string, "< file25.txt # ls ps");
	assert(spec_parsing(string) == EXIT_SUCCESS);
	strcpy(string, "< file25.txt ls # ps");
	assert(spec_parsing(string) == EXIT_FAILURE);
	strcpy(string, "< file25.txt # ls ps");
	assert(spec_parsing(string) == EXIT_SUCCESS);

}

int test_input(void){
	char* storage[LINESIZE];
	char* discardable;
	char* old_IFS;

	/** ### Utility Function Testing */

	char* test_input[LINESIZE];
	char* temp = "Delete this";
	util_reset_storage(test_input);
	assert(test_input[0] == 0x0);
	*test_input = temp;
	util_reset_storage(test_input);
	assert(test_input[0] == 0x0);

	// Test the utility function for int to string
	char tester[LINESIZE];
	util_int_to_string(1,tester,3);
	assert(strcmp(tester,"1") == 0);
	strcpy(tester, "");
	util_int_to_string(-1, tester, 3);
	assert(strcmp(tester,"-1") == 0);
	strcpy(tester, "");
	util_int_to_string(0, tester, 3);
	assert(strcmp(tester,"0") == 0);
	strcpy(tester, "");
	util_int_to_string(100, tester, 3);
	assert(strcmp(tester,"100") == 0);
	strcpy(tester, "");
	assert(strcmp( tester, "") == 0);
	strcpy(tester, "");
	util_int_to_string(1, tester, 101);
	assert(strcmp( tester, "1") == 0);

	/** ### util_env_var_to_fixed_array testing */
	char fixed_array[LINESIZE];

	assert(util_env_var_to_fixed_array("HOME", fixed_array) == EXIT_SUCCESS);
	assert(strcmp(fixed_array, getenv("HOME")) == 0);

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
	strcpy(input, "");
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
	printf("Test case 23: Expansion\n");
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
	util_setenv("IFS", old_IFS);
	
	if(strcmp(getenv("IFS"),old_IFS) != 0){

		printf("Something went wrong resetting the IFS:*%s*, verify it is only stored once in the old_IFS:*%s* variable",getenv("IFS"), old_IFS);
		fflush(stderr);
		fflush(stdout);
		exit(EXIT_FAILURE);
	};
	return EXIT_SUCCESS;
}

int test_expansion(void){
	/**
	 * \brief Tests the expansion portion of the specifications
	 *
	 * @return EXIT_SUCCESS if all tests pass, EXIT_FAILURE if any test fails
	 * */

	// Test Case 1: ~/ at beginning, $$ is present, $? is present, $! is present, IFS is set (not null)
	char string[LINESIZE] = "~/$$$?$!";
	char home[100];
	util_env_var_to_fixed_array("HOME", home);
	char result[LINESIZE] = "";
	char holder[LINESIZE];
	char resultb[LINESIZE];
	char str_pid[LINESIZE];
	char stringb[LINESIZE];
	strcat(result, home);
	util_int_to_string(getpid(), holder, LINESIZE);
	strcat(result, holder);
	// TODO: strcat(result, ); need to setup foreground and background processes
	util_setenv("IFS", " \t\n");

	assert(spec_expansion(string, "$$", 1) == EXIT_SUCCESS);
	// TODO: need to setup foreground and background processes assert(strcmp(string, ));
	
	// Test Case 2 - 6 need foreground and background processes
	
	// Test Case 7: ~/ at beginning, $$ present, IFS set
	
	strcpy(string, "~/Ted$$");
	strcpy(stringb, "~/$$Ted");
	strcpy(result, "");
	util_env_var_to_fixed_array("HOME", result);

	util_int_to_string(getpid(), str_pid, 10);
	strcat(result, "Ted");
	strcat(result, str_pid);
	util_setenv("IFS", " \t\n");
	assert(spec_expansion(string, "$$", 1) == EXIT_SUCCESS);
	assert(spec_expansion(stringb, "$$", 1) == EXIT_SUCCESS);
	assert(strcmp(string, result) == 0);

	assert(spec_expansion(stringb, "$$", 1) == EXIT_SUCCESS);
	assert(strcmp(stringb, result));

	// Test Case 8: ~/ at beginning, $$ present, IFS unset
	strcpy(string, "~/Ted$$");
	strcpy(stringb, "~/$$Ted");
	util_env_var_to_fixed_array("HOME", result);
	strcpy(resultb, result);
	util_int_to_string(getpid(), str_pid, 10);
	strcat(result, "Ted");
	strcat(result, str_pid);
	strcat(resultb, str_pid);
	strcat(resultb, "Ted");
	util_setenv("IFS", "NULL");

	assert(spec_expansion(string, "$$", 1) == EXIT_SUCCESS);
	assert(strcmp(string, result) == 0);
	assert(spec_expansion(stringb, "$$", 1) == EXIT_SUCCESS);
	assert(strcmp(stringb, resultb) == 0);

	// TODO: Test Cases 9-14 need foreground and background processes implemented
	
	// Test Case 15: ~/ at beginning, $$, $?, $! are not present, IFS set
	strcpy(string, "~/");
	strcpy(stringb, "~/Ted");
	util_env_var_to_fixed_array("HOME", result);
	util_env_var_to_fixed_array("HOME", resultb);
	strcat(resultb, "Ted");
	util_setenv("IFS", " \t\n");
	assert(spec_expansion(string, "$$", 1) == EXIT_SUCCESS);
	assert(strcmp(string, result) == 0);
	assert(spec_expansion(stringb, "$$", 1) == EXIT_SUCCESS);
	assert(strcmp(stringb, resultb) == 0);

	// Test Case 16: ~/ at beginning, $$, $?, $! are not present, IFS not set
	strcpy(string, "~/");
	strcpy(stringb, "~/Ted");
	util_env_var_to_fixed_array("HOME", result);
	strcpy(resultb, result);
	strcat(resultb, "Ted");
	util_setenv("IFS", "NULL");

	assert(spec_expansion(string, "$$", 1) == EXIT_SUCCESS);
	assert(strcmp(string, result) == 0);
	assert(spec_expansion(stringb, "$$", 1) == EXIT_SUCCESS);
	assert(strcmp(stringb, resultb) == 0);

	// TODO: 17-22 need foreground and background processes implemented
	
	// Test Case 23: ~/ not front, $$ present, S? and S! not present, IFS set
	strcpy(string, "$$Ted~/");
	strcpy(result, "");
	util_int_to_string(getpid(), result, 10);
	strcat(result, "Ted~/");
	util_setenv("IFS", " \t\n");
	strcpy(resultb,"");
	assert(spec_expansion(string, "$$", 1) == EXIT_SUCCESS);
	assert(strcmp(result, string) == 0);

	// Test Case 24: ~/ not at front, $$ present, S? and $! not present, IFS not set
	strcpy(string, "Ted~/$$");
	strcpy(result, "Ted~/");
	util_int_to_string(getpid(), holder, 10);
	strcat(result, holder);
	util_setenv("IFS", "NULL");
	assert(spec_expansion(string, "$$", 1) == EXIT_SUCCESS);
	assert(strcmp(result, string) == 0);


	// TODO: Test cases 25-30 need foreground and background processes implemented
	
	// Test Case 31: ~/ is not at front, $* are all not present, IFS is set
	strcpy(string, "");
	strcpy(result, "");
	strcpy(string, "Ted~/");
	strcpy( result, "Ted~/");

	util_setenv("IFS", " \t\n");
	assert(spec_expansion(string, "$$", 1) == EXIT_SUCCESS);

	assert(strcmp(result, string) == 0);

	// Test Case 32: ~/ is not at front, $* are all not present, IFS is not set
	strcpy(string, "How?~/");
	strcpy(result, string);
	util_setenv("IFS", "NULL");
	assert(spec_expansion(string, "$$", 1) == EXIT_SUCCESS);
	assert(strcmp(result, string) == 0);

	// TODO: Test cases 33-38 need foreground and background processes implemented
	
	// Test Case 39: ~/ does not occur at all, $$ present, IFS set
	strcpy(string, "Ted$$");
	strcpy(result, "Ted");
	strcpy(holder, "");
	util_int_to_string(getpid(), holder, 10);
	strcat(result, holder);
	util_setenv("IFS", " \t\n");
	assert(spec_expansion(string, "$$", 1) == EXIT_SUCCESS);
	assert(strcmp(result, string) == 0);

	// Test Case 40: ~/ does not occur at all, $$ present, IFS not set
	util_setenv("IFS", "NULL");
	assert(spec_expansion(string, "$$", 1) == EXIT_SUCCESS);
	assert(strcmp(result, string) == 0);

	// TODO: Test cases 41-46 need foreground and background processes implemented
	
	// Test Case 47: No variables need expanding, IFS set
	strcpy(string, "Ted");
	strcpy(result, "Ted");
	util_setenv("IFS", " \t\n");
	assert(spec_expansion(string, "$$", 1) == EXIT_SUCCESS);
	assert(strcmp(result, string) == 0);

	// Test Case 48: No variables need expanding, IFS set
	util_setenv("IFS", "NULL");
	assert(spec_expansion(string, "$$", 1) == EXIT_SUCCESS);
	assert(strcmp(result, string) == 0);

	return EXIT_SUCCESS;
}
