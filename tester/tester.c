# include <stdio.h>
# include <sys/types.h>
# include <signal.h>
# include <unistd.h>
# include <sys/wait.h>
# include <string.h>
# include <stdlib.h>
# include <assert.h>
# include <stdint.h>
# include <stdbool.h>

# include "../utilities/utilities.h"
# include "../error/error.h"
# include "../input/input.h"
# include "../expansion/expansion.h"
# include "../parsing/parsing.h"

# ifndef LINESIZE
# include "../constants/constants.h"
# endif

void test_spec_word_splitting() {
  // Test 1: Test with empty input string
  char input[LINESIZE] = "";
  char *storage[LINESIZE];
  int result = spec_word_splitting(storage, input);
  assert(result == EXIT_FAILURE);

  // Test 2: Test with input string that can be tokenized
  strcpy(input, "This is a sample input");
  result = spec_word_splitting(storage, input);
  assert(result == EXIT_SUCCESS);
  assert(strcmp(storage[0], "This") == 0);
  assert(strcmp(storage[1], "is") == 0);
  assert(strcmp(storage[2], "a") == 0);
  assert(strcmp(storage[3], "sample") == 0);
  assert(strcmp(storage[4], "input") == 0);

  // Test 3: Test with input string that has a length greater than LINESIZE
  strcpy(input, "This is a sample input with a length greater than LINESIZE, which should result in failure");
  for(int index = 0; index < LINESIZE+2; index++){
	strcat(input, "q");
  }
  result = spec_word_splitting(storage, input);
  assert(result == EXIT_FAILURE);
}


void test_help_split_line() {
    char line[20] = "hello world";
    char* storage[10];

    // Test normal behavior
    char** tokens = help_split_line(storage, line);
    assert(tokens[0] == "hello");
    assert(tokens[1] == "world");
    assert(tokens[2] == NULL);

    // Test passing a NULL line
    tokens = help_split_line(storage, NULL);
    assert(tokens == NULL);

    // Test passing an empty line
    line[0] = '\0';
    tokens = help_split_line(storage, line);
    assert(tokens[0] == NULL);

    // Test bufsize < 1
    int LINESIZE_OLD = LINESIZE;
    LINESIZE = 0;
    tokens = help_split_line(storage, line);
    LINESIZE = LINESIZE_OLD;
    assert(tokens == NULL);
}

void test_spec_execute() {
    struct ProgArgs current;
    int status;
    char input[LINESIZE];

    // Test for `cd` command
    strcpy(input, "cd /");
    assert(spec_execute(&current) == 0);

    // Test for other commands
    strcpy(input, "ls");
    status = spec_execute(&current);
    assert(status == 0 || status == 1);

    // Check if the process is running in the background
    pid_t pid = getpid();
    char ps_command[100];
    sprintf(ps_command, "ps -p %d | grep %d", pid, pid);
    assert(system(ps_command) == 0);

    // Check if the child process has not become a zombie process
    char zombie_check_command[100];
    sprintf(zombie_check_command, "ps -A -o stat= | grep Z | grep -v grep | awk '{ print $1 }'");
    assert(system(zombie_check_command) != 0);
}

void test_other_commands_process_creation() {
  pid_t parent_pid = getpid();
  struct ProgArgs current;
  current.command = "ls";
  int result = other_commands(current);

  // Check that the process was successfully created
  assert(result == EXIT_SUCCESS);

  // Check that a new process was created by using the 'ps' command
  FILE *fp;
  char output[1035];
  char cmd[100];
  sprintf(cmd, "ps --ppid %d | grep ls", parent_pid);
  fp = popen(cmd, "r");
  if (fp == NULL) {
    printf("Failed to run command\n" );
    exit(1);
  }

  fgets(output, sizeof(output)-1, fp);
  pclose(fp);

  // Check that the output of the 'ps' command is not empty, indicating that a new process was created
  assert(output[0] != '\0');
}

void test_other_commands_no_zombie_processes() {
  pid_t parent_pid = getpid();
  struct ProgArgs current;
  current.command = "ls";
  int result = other_commands(current);

  // Check that the process was successfully created
  assert(result == EXIT_SUCCESS);

  // Check that the child process has been reaped and is not a zombie by using the 'ps' command
  FILE *fp;
  char output[1035];
  char cmd[100];
  sprintf(cmd, "ps --ppid %d --state Z | grep ls", parent_pid);
  fp = popen(cmd, "r");
  if (fp == NULL) {
    printf("Failed to run command\n" );
    exit(1);
  }

  fgets(output, sizeof(output)-1, fp);
  pclose(fp);

  // Check that the output of the 'ps' command is empty, indicating that the child process is not a zombie
  assert(output[0] == '\0');
}

void test_handle_redirection() {
    struct args current = {NULL, NULL};
    int success;

    // Test without redirection
    success = handle_redirection(&current);
    assert(success == EXIT_SUCCESS);
    assert(dup(STDIN_FILENO) == STDIN_FILENO);
    assert(dup(STDOUT_FILENO) == STDOUT_FILENO);

    // Test with input redirection
    current.input = "input.txt";
    success = handle_redirection(&current);
    assert(success == EXIT_SUCCESS);
    assert(dup(STDIN_FILENO) != STDIN_FILENO);
    assert(dup(STDOUT_FILENO) == STDOUT_FILENO);
    assert(close(STDIN_FILENO) == 0);

    // Test with non-existent input file
    current.input = "nonexistent.txt";
    success = handle_redirection(&current);
    assert(success == EXIT_FAILURE);
    assert(dup(STDIN_FILENO) == STDIN_FILENO);
    assert(dup(STDOUT_FILENO) == STDOUT_FILENO);

    // Test with output redirection
    current.input = NULL;
    current.output = "output.txt";
    success = handle_redirection(&current);
    assert(success == EXIT_SUCCESS);
    assert(dup(STDIN_FILENO) == STDIN_FILENO);
    assert(dup(STDOUT_FILENO) != STDOUT_FILENO);
    assert(close(STDOUT_FILENO) == 0);

    // Test with input and output redirection
    current.input = "input.txt";
    current.output = "output.txt";
    success = handle_redirection(&current);
    assert(success == EXIT_SUCCESS);
    assert(dup(STDIN_FILENO) != STDIN_FILENO);
    assert(dup(STDOUT_FILENO) != STDOUT_FILENO);
    assert(close(STDIN_FILENO) == 0);
    assert(close(STDOUT_FILENO) == 0);

    // Test with non-existent output file
    current.input = NULL;
    current.output = "/nonexistent/path/output.txt";
    success = handle_redirection(&current);
    assert(success == EXIT_FAILURE);
    assert(dup(STDIN_FILENO) == STDIN_FILENO);
    assert(dup(STDOUT_FILENO) == STDOUT_FILENO);

    // Clean up
    current.input = NULL;
    current.output = NULL;
}


void test_reset_signals() {
    // Test with SIGINT set to SIG_DFL
    struct sigaction act = { .sa_handler = SIG_DFL };
    if(sigaction(SIGINT, &act, NULL) == -1){
        perror("Cannot set SIGINT to SIG_DFL");
        exit(EXIT_FAILURE);
    };
    assert(reset_signals() == EXIT_SUCCESS);
    struct sigaction oldact;
    if(sigaction(SIGINT, NULL, &oldact) == -1){
        perror("Cannot get SIGINT signal handler");
        exit(EXIT_FAILURE);
    };
    assert(oldact.sa_handler == SIG_DFL);

    // Test with SIGINT set to a custom signal handler
    act.sa_handler = SIG_IGN;
    if(sigaction(SIGINT, &act, NULL) == -1){
        perror("Cannot set SIGINT to SIG_IGN");
        exit(EXIT_FAILURE);
    };
    assert(reset_signals() == EXIT_SUCCESS);
    if(sigaction(SIGINT, NULL, &oldact) == -1){
        perror("Cannot get SIGINT signal handler");
        exit(EXIT_FAILURE);
    };
    assert(oldact.sa_handler == SIG_IGN);

    // Test with an error setting SIGINT to SIG_DFL
    act.sa_handler = SIG_IGN;
    if(sigaction(SIGINT, &act, NULL) == -1){
        perror("Cannot set SIGINT to SIG_IGN");
        exit(EXIT_FAILURE);
    };
    if(sigaction(SIGINT, &act, NULL) == -1){
        perror("Cannot set SIGINT to SIG_DFL");
        exit(EXIT_FAILURE);
    };
    assert(reset_signals() == EXIT_FAILURE);

    // Test with an error getting the old signal handler
    if(sigaction(SIGUSR1, &act, NULL) == -1){
        perror("Cannot set SIGUSR1 to SIG_IGN");
        exit(EXIT_FAILURE);
    };
    assert(reset_signals() == EXIT_FAILURE);
}


void test_handle_exit() {
    // Test with no child processes
    char command[256];
    sprintf(command, "%s &", "sleep 1");
    system(command);  // Start a background process
    pid_t pid = getpid();  // Get the PID of this process
    handle_exit(NULL);  // Call the function with no child processes
    sprintf(command, "ps -o pid | grep %d | wc -l", pid);
    FILE* fp = popen(command, "r");
    char output[256];
    fgets(output, sizeof(output), fp);
    pclose(fp);
    int count = atoi(output);
    assert(count == 1);  // Only one process should be running (this one)

    // Test with child processes
    pid_t child_pid = fork();  // Start a child process
    if (child_pid == 0) {
        // Child process: just sleep for a few seconds
        sleep(2);
        exit(0);
    } else {
        // Parent process: send SIGINT to child processes and exit
        handle_exit(NULL);
        sprintf(command, "ps -o pid | grep -E '%d|%d' | wc -l", pid, child_pid);
        fp = popen(command, "r");
        fgets(output, sizeof(output), fp);
        pclose(fp);
        count = atoi(output);
        assert(count == 1);  // Only one process should be running (this one)
    }
}


void test_execute_cd() {
    // Test with a valid path
    int status = execute_cd("/");
    assert(status == EXIT_SUCCESS);
    assert(strcmp(getcwd(NULL, 0), "/") == 0);

    // Test with a null path
    status = execute_cd(NULL);
    assert(status == EXIT_SUCCESS);
    assert(strcmp(getcwd(NULL, 0), getenv("HOME")) == 0);

    // Test with a non-existent path
    status = execute_cd("/non-existent/path");
    assert(status == EXIT_FAILURE);

    // Test with a path to a file instead of a directory
    status = execute_cd("/etc/hosts");
    assert(status == EXIT_FAILURE);
}


void test_check_background_processes() {
    int status;
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process
        sleep(1);
        exit(0);
    } else {
        // Parent process
        // Wait for child process to complete
        waitpid(pid, &status, 0);

        // Test WIFEXITED branch
        assert(check_background_processes() == EXIT_SUCCESS);

        pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Child process
            sleep(1);
            exit(1);
        } else {
            // Parent process
            // Wait for child process to complete
            waitpid(pid, &status, 0);

            // Test WIFEXITED branch
            assert(check_background_processes() == EXIT_FAILURE);
        }

        pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Child process
            pause();
        } else {
            // Parent process
            // Send SIGSTOP to child process
            kill(pid, SIGSTOP);

            // Test WIFSTOPPED branch
            assert(check_background_processes() == EXIT_SUCCESS);

            // Send SIGCONT to child process
            kill(pid, SIGCONT);
        }

        pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Child process
            abort();
        } else {
            // Parent process
            // Wait for child process to complete
            waitpid(pid, &status, 0);

            // Test WIFSIGNALED branch
            assert(check_background_processes() == EXIT_FAILURE);
        }
    }
}


void test_struct_utilities() {
  struct ParentStruct parent = {0};
  struct ParentStruct child1 = {1};
  struct ParentStruct child2 = {2};
  struct ParentStruct child3 = {3};

  // Test add_to_array
  assert(add_to_array(&parent, &child1, 0) == EXIT_SUCCESS);
  assert(parent.pid == child1.pid);
  assert(parent.pid_counter == child1.pid_counter);
  assert(parent.array[0] == &child1);

  assert(add_to_array(&parent, &child2, 2) == EXIT_SUCCESS);
  assert(parent.pid == child1.pid);
  assert(parent.pid_counter == child1.pid_counter);
  assert(parent.array[0] == &child1);
  assert(parent.array[1] == NULL);
  assert(parent.array[2] == &child2);

  assert(add_to_array(&parent, &child3, 5) == EXIT_SUCCESS);
  assert(parent.pid == child1.pid);
  assert(parent.pid_counter == child1.pid_counter);
  assert(parent.array[0] == &child1);
  assert(parent.array[2] == &child2);
  assert(parent.array[5] == &child3);

  assert(add_to_array(&parent, &child1, 0) == EXIT_FAILURE); // Adding to non-empty array
  assert(add_to_array(&parent, &child2, 6) == EXIT_FAILURE); // Adding to invalid index

  // Test update_child
  assert(update_child(&child1, 10, 20) == EXIT_SUCCESS);
  assert(child1.pid == 10);
  assert(child1.pid_counter == 20);

  assert(update_child(NULL, 10, 20) == EXIT_FAILURE); // Updating NULL child

  // Test remove_from_array
  assert(remove_from_array(&parent, 2) == EXIT_SUCCESS);
  assert(parent.array[2] == NULL);

  assert(remove_from_array(&parent, 5) == EXIT_SUCCESS);
  assert(parent.array[5] == NULL);

  assert(remove_from_array(&parent, 2) == EXIT_FAILURE); // Removing non-existent child
  assert(remove_from_array(&parent, 6) == EXIT_FAILURE); // Removing from invalid index
}

int test_parsing(struct ProgArgs *prog_arg){
	char string[LINESIZE] = "";
	char result[LINESIZE] = "";

	// Test Case 1
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(strcmp(string, result) == 0);

	// Test Case 2
	strcpy(string, "&");
	assert(spec_parsing(string, prog_arg) == EXIT_FAILURE);

	// Test Case 3
	strcpy(string, "#");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);

	// Test Case 4
	strcpy(string, "ps");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	strcpy(string, "ps&");
	// It doesn't matter if the command is valid, only standalone & causes errors
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(prog_arg->background == false);

	// Test Case 6
	strcpy(string, "< >");
	assert(spec_parsing(string, prog_arg) == EXIT_FAILURE);
	assert(strcmp(prog_arg->input, "") == 0);
	assert(strcmp(prog_arg->output, "") == 0);
	strcpy(string, "<>");
	// <> could theoretically be a valid command line option for a program so its successful
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(strcmp(prog_arg->input, "") == 0);
	assert(strcmp(prog_arg->output, "") == 0);

	// Test Case 7
	strcpy(string, "< #");
	assert(spec_parsing(string, prog_arg) == EXIT_FAILURE);
	assert(strcmp(prog_arg->input, "") == 0);

	// <# could be a valid command line argument and is NOT a comment and is NOT a redirection
	strcpy(string, "<#");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(strcmp(prog_arg->input, "") == 0);

	// Test Case 8
	strcpy(string, "> #");
	assert(spec_parsing(string, prog_arg) == EXIT_FAILURE);
	assert(strcmp(prog_arg->output, "") == 0);
	strcpy(string, ">#");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(strcmp(prog_arg->output, "") == 0);

	// Test Case 9
	strcpy(string, "# <");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(strcmp(prog_arg->input, "") == 0);

	// Test Case 10
	strcpy(string, "< &");
	assert(spec_parsing(string, prog_arg) == EXIT_FAILURE);
	assert(strcmp(prog_arg->input, "") == 0);
	strcpy(string, "<&");

	// Test Case 11
	strcpy(string, "< & #");
	assert(spec_parsing(string, prog_arg) == EXIT_FAILURE);
	strcpy(string, "<& #");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	strcpy(string, "< &#");
	assert(spec_parsing(string, prog_arg) == EXIT_FAILURE);
	
	// Test Case 12
	strcpy(string, "< file1.txt");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);

	assert(strcmp(prog_arg->input, "file1.txt") == 0);
	// Again, there's no way to distinguish between < <file1.txt and <file1.txt as a mistype, so pass it along to the operating system for file validity
	strcpy(string, "<file1.txt");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);

	// Test Case 16
	strcpy(string, "> file1.txt");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(strcmp(prog_arg->output, "file1.txt") == 0);
	strcpy(string, ">file1.txt");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(strcmp(prog_arg->output, "") == 0);
	// Weirdly enough, this parses as a command and the operating system has to handle that it is an invalid command
	assert(strcmp(prog_arg->command, ">file1.txt") == 0);

	// Test Case 18
	strcpy(string, "ps &");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(prog_arg->background == true);
	strcpy(string, "& ps");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_FAILURE);

	// Test Case 20
	strcpy(string, "cd ..");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(prog_arg->background == false);
	strcpy(string, "cd ls");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(prog_arg->background == false);
	
	// Test Case 22
	strcpy(string, "ps < file1.txt > file1.txt");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	strcpy(string, "ps > file1.txt < file1.txt");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(strcmp(prog_arg->input, "file1.txt") == 0);
	assert(strcmp(prog_arg->output, "file1.txt") == 0);
	strcpy(string, "ls ps < > file1.txt");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_FAILURE);
	strcpy(string, "< file1.txt > file1.txt ps");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_FAILURE);

	// Test Case 23
	strcpy(string, "< file1.txt > file1.txt #");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(strcmp(prog_arg->input, "file1.txt") == 0);
	assert(strcmp(prog_arg->output, "file1.txt") == 0);
	strcpy(string, "> file1.txt < file1.txt #");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(strcmp(prog_arg->input, "file1.txt") == 0);
	assert(strcmp(prog_arg->output, "file1.txt") == 0);

	// Test Case 24
	strcpy(string, "ls -a ~/ < file1.txt");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(strcmp(prog_arg->input, "file1.txt") == 0);
	assert(strcmp(prog_arg->output, "") == 0);
	strcpy(string, "cd pwd < file1.txt ps");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_FAILURE);

	// Test Case 25
	strcpy(string, "ls ~/ < file25.txt #");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(strcmp(prog_arg->input, "file25.txt") == 0);
	strcpy(string, "< file25.txt # ls ps");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	strcpy(string, "< file25.txt ls # ps");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_FAILURE);
	strcpy(string, "< file25.txt # ls ps");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);

	// Test Case 26
	strcpy(string, "ls ~/ > file1.txt");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(strcmp(prog_arg->output, "file1.txt") == 0);
	strcpy(string, "ls ~/ >file1.txt");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	strcpy(string, "> file1.txt ls ~/");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_FAILURE);

	// Test Case 27
	strcpy(string, "ls ~/ > file1.txt #");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(strcmp(prog_arg->output, "file1.txt") == 0);
	strcpy(string, "> file1.txt ls ~/ #");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_FAILURE);
	strcpy(string, "> file1.txt # ls ~/");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->command, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(strcmp(prog_arg->output, "file1.txt") == 0);
	assert(strcmp(prog_arg->command, "") == 0);

	// Test Case 28
	strcpy(string, "ls -a -b -c -d -f");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);

	// Test Case 29
	strcpy(string, "ls -a -b -c -d #");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	strcpy(prog_arg->command, "");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(strcmp(prog_arg->command, "ls -a -b -c -d "));
	strcpy(string, "ls -a -b -c # -d");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(strcmp(prog_arg->command, "ls -a -b -c") == 0);
	strcpy(string, "ls -a -b # -c -d");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(strcmp(prog_arg->command, "ls -a -b") == 0);
	strcpy(string, "ls -a # -b -c -d");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(strcmp(prog_arg->command, "ls -a") == 0);
	strcpy(string, "ls # -a -b -c -d");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(strcmp(prog_arg->command, "ls") == 0);
	strcpy(string, "#ls -a -b -c -d -f");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	strcpy(prog_arg->command, "");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(strcmp(prog_arg->command, "") == 0);

	// Test Case 30
	strcpy(string, "ls -a < file1.txt > file1.txt");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(strcmp(prog_arg->command, "ls -a ") == 0);
	assert(strcmp(prog_arg->input, "file1.txt") == 0);
	strcpy(string, "ls < file1.txt > file1.txt -a");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_FAILURE);
	assert(strcmp(prog_arg->output, "") == 0);
	strcpy(string, "-a ls < file1.txt > file1.txt");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	// We do not need to make sure the parameters to ls make sense
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(strcmp(prog_arg->command, "-a ls ") == 0);
	assert(strcmp(prog_arg->input, "file1.txt") == 0);
	assert(strcmp(prog_arg->output, "file1.txt") == 0);

	// Test Case 31
	strcpy(string, "ls < file1.txt > file1.txt & #");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	prog_arg->background = false;
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(strcmp(prog_arg->command, "ls ") == 0);
	assert(strcmp(prog_arg->input, "file1.txt") == 0);
	assert(strcmp(prog_arg->output, "file1.txt") == 0);
	assert(prog_arg->background == true);
	strcpy(string, "ls < file1.txt # > file1.txt # &");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(strcmp(prog_arg->command, "ls ") == 0);
	assert(strcmp(prog_arg->input, "file1.txt") == 0);
	assert(strcmp(prog_arg->output, "") == 0);
	assert(prog_arg->background == false);
	strcpy(string, "ls # < file1.txt > file1.txt");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(strcmp(prog_arg->command, "ls") == 0);
	strcpy(string, "ls & # < file1.txt >");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(strcmp(prog_arg->command, "ls ") == 0);
	assert(prog_arg->background == true);
	strcpy(string, "& ls");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_FAILURE);
	strcpy(string, "ls & # < file1.txt > file1.txt");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(strcmp(prog_arg->command, "ls ") == 0);
	assert(prog_arg->background == true);
	strcpy(string, "# ls < file1.txt > file1.txt");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(strcmp(prog_arg->command, "") == 0);
	assert(strcmp(prog_arg->input, "") == 0);
	assert(strcmp(prog_arg->output, "") == 0);

	// Test Case 34
	strcpy(string, "ls -a -b < file1.txt &");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(strcmp(prog_arg->command, "ls -a -b ") == 0);
	assert(strcmp(prog_arg->input, "file1.txt") == 0);
	assert(prog_arg->background == true);
	strcpy(string, "ls -a -b < & file1.txt");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_FAILURE);
	strcpy(string, "ls -a & < file1.txt -b");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_FAILURE);
	strcpy(string, "< file1.txt ls -a -b &");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_FAILURE);
	strcpy(string, "& ls -a -b < file1.txt");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_FAILURE);

	// Test Case 35
	strcpy(string, "ls -a -b < file1.txt # &");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(strcmp(prog_arg->command, "ls -a -b ") == 0);
	assert(strcmp(prog_arg->input, "file1.txt") == 0);
	assert(prog_arg->background == false);
	strcpy(string, "ls -a -b < file1.txt & #");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(strcmp(prog_arg->command, "ls -a -b ") == 0);
	assert(strcmp(prog_arg->input, "file1.txt") == 0);
	assert(strcmp(prog_arg->output, "") == 0);
	assert(prog_arg->background == true);
	strcpy(string, "ls -a -b < & file1.txt #");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_FAILURE);
	// This is actually the test for the proper initialization of *prog_arg being preserved through the function
	assert(strcmp(prog_arg->command, "") == 0);
	assert(strcmp(prog_arg->input, "") == 0);
	assert(strcmp(prog_arg->output, "") == 0);
	assert(prog_arg->background == false);
	strcpy(string, "ls -a -b # < & file1.txt");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(strcmp(prog_arg->command, "ls -a -b") == 0);
	assert(strcmp(prog_arg->input, "") == 0);
	assert(strcmp(prog_arg->output, "") == 0);
	assert(prog_arg->background == false);
	strcpy(string, "ls -a & < # file1.txt -b");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_FAILURE);
	strcpy(string, "ls -a # & < file1.txt -b");
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(strcmp(prog_arg->command, "ls -a") == 0);
	assert(strcmp(prog_arg->input, "") == 0);
	assert(strcmp(prog_arg->output, "") == 0);
	assert(prog_arg->background == false);
	strcpy(string, "< file1.txt ls -a -b &");

	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_FAILURE);
	assert(strcmp(prog_arg->command, "") == 0);
	assert(strcmp(prog_arg->input, "") == 0);
	assert(strcmp(prog_arg->output, "") == 0);
	assert(prog_arg->background == false);
	strcpy(string, "< file1.txt ls -a -b &");

	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_FAILURE);
	assert(strcmp(prog_arg->command, "") == 0);
	assert(strcmp(prog_arg->input, "") == 0);
	assert(strcmp(prog_arg->output, "") == 0);

	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	strcpy(string, "< file1.txt ls -a -b &");
	assert(spec_parsing(string, prog_arg) == EXIT_FAILURE);
	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	strcpy(string, "# < file1.txt ls -a -b &");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(strcmp(prog_arg->command, "") == 0);
	assert(prog_arg->background == false);
	strcpy(string, "& ls -a -b < file1.txt");

	strcpy(prog_arg->input, "");
	strcpy(prog_arg->output, "");
	assert(spec_parsing(string, prog_arg) == EXIT_FAILURE);
	strcpy(string, "# & ls -a -b < file1.txt");

	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(strcmp(prog_arg->command, "") == 0);
	assert(strcmp(prog_arg->input, "") == 0);
	assert(strcmp(prog_arg->output, "") == 0);
	assert(prog_arg->background == false);

	// Test Case 36
	strcpy(string, "ls -a -b > file1.txt &");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(strcmp(prog_arg->command, "ls -a -b ") == 0);
	assert(strcmp(prog_arg->output, "file1.txt") == 0);
	assert(prog_arg->background == true);
	strcpy(string, "ls -a -b > & file1.txt");
	assert(spec_parsing(string, prog_arg) == EXIT_FAILURE);
	strcpy(string, "ls -a & > file1.txt -b");
	assert(spec_parsing(string, prog_arg) == EXIT_FAILURE);
	strcpy(string, "> file1.txt ls -a -b &");
	assert(spec_parsing(string, prog_arg) == EXIT_FAILURE);
	strcpy(string, "& ls -a -b > file1.txt");
	assert(spec_parsing(string, prog_arg) == EXIT_FAILURE);

	// Test Case 37
	strcpy(string, "ls -a -b > file1.txt # &");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(strcmp(prog_arg->command, "ls -a -b ") == 0);
	assert(strcmp(prog_arg->output, "file1.txt") == 0);
	assert(prog_arg->background == false);
	strcpy(string, "ls -a -b > file1.txt & #");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(strcmp(prog_arg->command, "ls -a -b ") == 0);
	assert(strcmp(prog_arg->output, "file1.txt") == 0);
	assert(strcmp(prog_arg->input, "") == 0);
	assert(prog_arg->background == true);
	strcpy(string, "ls -a -b > & file1.txt #");
	assert(spec_parsing(string, prog_arg) == EXIT_FAILURE);
	// This is actually the test for the proper initialization of *prog_arg being preserved through the function
	assert(strcmp(prog_arg->command, "") == 0);
	assert(strcmp(prog_arg->input, "") == 0);
	assert(strcmp(prog_arg->output, "") == 0);
	assert(prog_arg->background == false);
	strcpy(string, "ls -a -b # > & file1.txt");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(strcmp(prog_arg->command, "ls -a -b") == 0);
	assert(strcmp(prog_arg->input, "") == 0);
	assert(strcmp(prog_arg->output, "") == 0);
	assert(prog_arg->background == false);
	strcpy(string, "ls -a & > # file1.txt -b");
	assert(spec_parsing(string, prog_arg) == EXIT_FAILURE);
	strcpy(string, "ls -a # & > file1.txt -b");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(strcmp(prog_arg->command, "ls -a") == 0);
	assert(strcmp(prog_arg->input, "") == 0);
	assert(strcmp(prog_arg->output, "") == 0);
	assert(prog_arg->background == false);
	strcpy(string, "> file1.txt ls -a -b &");
	assert(spec_parsing(string, prog_arg) == EXIT_FAILURE);
	strcpy(string, "# > file1.txt ls -a -b &");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(strcmp(prog_arg->command, "") == 0);
	assert(prog_arg->background == false);
	strcpy(string, "& ls -a -b > file1.txt");
	assert(spec_parsing(string, prog_arg) == EXIT_FAILURE);
	strcpy(string, "# & ls -a -b > file1.txt");
	assert(spec_parsing(string, prog_arg) == EXIT_SUCCESS);
	assert(strcmp(prog_arg->command, "") == 0);
	assert(strcmp(prog_arg->input, "") == 0);
	assert(strcmp(prog_arg->output, "") == 0);
	assert(prog_arg->background == false);

	return EXIT_SUCCESS;

}

int test_spec_get_line() {
  char input[LINESIZE];
  size_t input_size = LINESIZE;
  FILE* stream = stdin;

  // Test input of zero length
  int result = spec_get_line(input, input_size, stream);
  assert(result == EXIT_SUCCESS);

  // Test successful input with positive length
  strcpy(input, "test input\n");
  input_size = strlen(input);
  result = spec_get_line(input, input_size, stream);
  assert(result == EXIT_SUCCESS);
  assert(strcmp(input, "test input\n") == 0);

  // Test failed input with negative length
  stream = NULL;
  result = spec_get_line(input, input_size, stream);
  assert(result == EXIT_FAILURE);

  return EXIT_SUCCESS;
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
