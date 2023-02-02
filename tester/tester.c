# include <stdio.h>
# include <sys/types.h>
# include <signal.h>
# include <unistd.h>
# include <sys/wait.h>
# include <string.h>
# include <stdlib.h>
# include <assert.h>
# include <stdint.h>
# include "../constants/constants.h"
# include "../utilities/utilities.h"
# include "../error/error.h"
# include "../input/input.h"
# include "../expansion/expansion.h"

# include <stdlib.h>
# include <errno.h>

int test_input(void){
	char* storage[LINESIZE];
	char* discardable;
	char* old_IFS;
	//char  str[LINESIZE];

	/** ### Utility Function Testing */

	char* test_input[LINESIZE];
	util_reset_storage(test_input);
	assert(test_input[0] == NULL);

	// Test the utility function for int to string
	char tester[100];
	util_int_to_string(1,tester,3);
	assert(strcmp(tester,"3") == 0);
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
	util_int_to_string(100, tester, 2);
	assert(strcmp( tester, "NULL") == 0);
	strcpy(tester, "");
	util_int_to_string(1, tester, 101);
	assert(strcmp( tester, "NULL") == 0);


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
int test_expansion(void){
	/**
	 * \brief Tests the expansion portion of the specifications
	 *
	 * @return EXIT_SUCCESS if all tests pass, EXIT_FAILURE if any test fails
	 * */

	// Test Case 1: ~/ at beginning, $$ is present, $? is present, $! is present, IFS is set (not null)
	char string[LINESIZE] = "~/$$$?$!";
	char home[100];
	strcat(home, getenv("HOME"));
	if(home == NULL){
		perror("There's been an issue get the home environment variable");
		exit(EXIT_FAILURE);
	};
	char result[LINESIZE] = "";
	strcat(result, home);
	strcat(result, util_int_to_string(getpid()));
	// TODO: strcat(result, ); need to setup foreground and background processes
	if(setenv("IFS", " \t\n", 1) != 0){
		perror("Can't set IFS to neutral delimiter");
		exit(EXIT_FAILURE);
	};

	assert(spec_expansion(string) == EXIT_SUCCESS);
	// TODO: need to setup foreground and background processes assert(strcmp(string, ));
	
	// Test Case 2 - 6 need foreground and background processes
	
	// Test Case 7: ~/ at beginning, $$ present, IFS set 
	char string7[LINESIZE] = "~/Ted$$";
	char string7b[LINESIZE] = "~/$$Ted";
	char result7_pid[LINESIZE];
	char result7_home[LINESIZE] = getenv("HOME");
	char result7b_home = getenv("HOME");

	util_int_to_string(getpid(), result7_pid, 10);
	strcat(result7_home, "Ted");
	strcat(result7_home, result7_pid);
	strcat(result7b_home, result7_pid);
	if(setenv("IFS", " \t\n", 1) != 0){
		perror("Can't set IFS to neutral delimiter");
		exit(EXIT_FAILURE);
	};
	assert(spec_expansion(string7) == EXIT_SUCCESS);
	assert(strcmp(string7, result7_home));
	assert(spec_expansion(string7b) == EXIT_SUCCESS);
	assert(strcmp(string7b, result7b_home));

	// Test Case 8: ~/ at beginning, $$ present, IFS unset
	char string8[LINESIZE] = "~/Ted$$";
	char string8b[LINESIZE] = "`/$$Ted";
	char result8_pid[LINESIZE];
	char result8_home = getenv("HOME");
	char result8b_home = getenv("HOME");

	util_int_to_string(getpid(), result8_pid, 10);
	strcat(result8_home, "Ted");
	strcat(result8_home, result8_pid);
	strcat(result8b_home, result8_pid);
	if(setenv("IFS", "NULL", 1) != 0){
		perror("Can't set IFS to neutral delimiter");
		exit(EXIT_FAILURE);
	};
	assert(spec_expansion(string8) == EXIT_SUCCESS);
	assert(strcmp(string8, result8_home) == 0);
	assert(spec_expansion(string8b) == EXIT_SUCCESS);
	assert(strcmp(string8b, result8b_home) == 0);

	// TODO: Test Cases 9-14 need foreground and background processes implemented
	
	// Test Case 15: ~/ at beginning, $$, $?, $! are not present, IFS set
	char string15[LINESIZE] = "~/";
	char string15b[LINESIZE] = "~/Ted";
	char result15[LINESIZE] = getenv("HOME");
	char result15b[LINESIZE] = "";
	strcat(result15b, result15);
	strcat(result15b, "/Ted");
	if(setenv("IFS", " \t\n", 1) != 0){
		perror("Can't set IFS to neutral delimiter");
		exit(EXIT_FAILURE);
	};
	assert(spec_expansion(string15) == EXIT_SUCCESS);
	assert(strcmp(string15, result15) == 0);
	assert(spec_expansion(string15b) == EXIT_SUCCESS);
	assert(strcmp(string15b, result15b) == 0);

	// Test Case 16: ~/ at beginning, $$, $?, $! are not present, IFS not set
	char string16[LINESIZE] = "~/";
	char string16b[LINESIZE] = "~/Ted";
	char result16[LINESIZE] = getenv("HOME");
	char result16b[LINESIZE] = "";
	strcat(result16b, result15);
	strcat(result16b, "/Ted");
	if(setenv("IFS", "NULL", 1) != 0){
		perror("Can't set IFS to neutral delimiter");
		exit(EXIT_FAILURE);
	};
	assert(spec_expansion(string16) == EXIT_SUCCESS);
	assert(strcmp(string16, result16) == 0);
	assert(spec_expansion(string16b) == EXIT_SUCCESS);
	assert(strcmp(string16b, result16b) == 0);

	// TODO: 17-22 need foreground and background processes implemented
	
	// Test Case 23: ~/ not front, $$ present, S? and S! not present, IFS set
	char string23[LINESIZE] = "$$Ted~/";
	char result23[LINESIZE];
	util_int_to_string(getpid(), result23, 10);
	strcat(result23, "Ted~/");
	if(setenv("IFS", " \t\n", 1) != 0){
		perror("Can't set IFS to neutral delimiter");
		exit(EXIT_FAILURE);
	};
	assert(spec_expansion(string23) == EXIT_SUCCESS);
	assert(strcmp(result23, string23) == 0);

	// Test Case 24: ~/ not at front, $$ present, S? and $! not present, IFS not set
	char string24[LINESIZE] = "Ted~/$$";
	char result24[LINESIZE] = "Ted~/";
	char holder[LINESIZE];
	util_int_to_string(getpid(), holder, 10);
	strcat(result24, holder);
	if(setenv("IFS", "NULL", 1) != 0){
		perror("Can't set IFS to neutral delimiter");
		exit(EXIT_FAILURE);
	};
	assert(spec_expansion(string24) == EXIT_SUCCESS);
	assert(strcmp(result24, string24) == 0);


	// TODO: Test cases 25-30 need foreground and background processes implemented
	
	// Test Case 31: ~/ is not at front, $* are all not present, IFS is set
	char string31[LINESIZE] = "Ted~/";
	char result31[LINESIZE] = "Ted~/";
	if(setenv("IFS", " \t\n", 1) != 0){
		perror("Can't set IFS");
		exit(EXIT_FAILURE);
	};
	assert(spec_expansion(string31) == EXIT_SUCCESS);
	assert(strcmp(result31, string31) == 0);

	// Test Case 32: ~/ is not at front, $* are all not present, IFS is not set
	char string32[LINESIZE] = "How?~/";
	char result32[LINESIZE] = "How?~/";
	if(setenv("IFS", "NULL", 1) != 0){
		perror("Can't set IFS");
		exit(EXIT_FAILURE);
	};
	assert(spec_expansion(string32) == EXIT_SUCCESS);
	assert(strcmp(result32, string32) == 0);

	// TODO: Test cases 33-38 need foreground and background processes implemented
	
	// Test Case 39: ~/ does not occur at all, $$ present, IFS set
	strcpy(string, "Ted$$");
	strcpy(result, "Ted");
	strcpy(holder, "");
	strcat(result, util_int_to_string(getpid(), holder, 10));
	if(setenv("IFS", " \t\n", 1) != 0){
		perror("Can't set IFS");
		exit(EXIT_FAILURE);
	};
	assert(spec_expansion(string) == EXIT_SUCCESS);
	assert(strcmp(result, string) == 0);

	// Test Case 40: ~/ does not occur at all, $$ present, IFS not set
	if(setenv("IFS", "NULL", 1) != 0){
		perror("Can't set IFS");
		exit(EXIT_FAILURE);
	};
	assert(spec_expansion(string) == EXIT_SUCCESS);
	assert(strcmp(result, string) == 0);

	// TODO: Test cases 41-46 need foreground and background processes implemented
	
	// Test Case 47: No variables need expanding, IFS set
	strcpy(string, "Ted");
	strcpy(result, "Ted");
	if(setenv("IFS", " \t\n", 1) != 0){
		perror("Can't set IFS");
		exit(EXIT_FAILURE);
	};
	assert(spec_expansion(string) == EXIT_SUCCESS);
	assert(strcmp(result, string) == 0);

	// Test Case 48: No variables need expanding, IFS set
	if(setenv("IFS", "NULL", 1) != 0){
		perror("Can't set IFS");
		exit(EXIT_FAILURE);
	};
	assert(spec_expansion(string) == EXIT_SUCCESS);
	assert(strcmp(result, string) == 0);
}
