# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <pthread.h>
# include <errno.h>
# include <sys/types.h>
# include <unistd.h>
# include <wait.h>
# include <fcntl.h>
# include <signal.h>
# define MAX_PROCESSES 5
# define LINE_SIZE 2048
# define ARG_SIZE 512
# define TOKEN_BUFSIZE 64
# define TOKEN_DELIMITER " \t\r\n\a"
# define arraylen(x) (sizeof x /sizeof *x)

int parent_pid = 0;

// foreground only is 0, meaning background is allowed
int foreground_only = 0;

struct Program_args {
  char     **arguments;
  int      background[1];// = 1;                                // This will default to 0, or no background/yes to foreground. 1= background activation
  char     *input_file;
  char     *output_file;
  int      input_file_opened;
  int      output_file_opened;
  char     *command;                                            // Storing the command name for later processing
  char     tolkien[LINE_SIZE];                                       // tolkien = token, this is the tokenized command line arguments
};    

struct Process {
  char     *id;                                                  // This will tell me how many pids have been created in this instance of smallsh
  int      status;     
};

struct Process last_background;

int pids_smallsh_opened[ARG_SIZE];

// Tells me how many pids are opened, with 0 = just smallsh
int pid_counter = 0;

// Background will be 1 if the current process is a background
int background = 0;


int lengthify_stringified_pid(int pid){
  return snprintf(NULL, 0, "%d", pid);
};

char *stringify_pid(int pid){
      // I found these handy three lines on stackoverflow
      // The trick is that passing the NULL and 0 parameters gives you the length
      // of a potential integer you are about to convert to a string
      
      int length = snprintf( NULL, 0, "%d", pid);
      if(length == 0){
        perror("Could not find length of pid");
      };
      char* buf = malloc(length + 1);
      snprintf( buf, length+1, "%d", pid);
      return buf;
};

void variable_expansion(char *arg){
  /*
   * Notes to myself before I get lost in another rabbit warren
   * Basic method is: 
   *  use string method to extract pointer to instance of $$
   *  get pid
   *  copy the second half of the string into a temporary string
   *  use the length, buf and snprintf lines to calculate the length of 
   *    the pid and then convert to an integer
   *  add each character of the stringified pid to the substring index 
   *  add the secondary string onto the end of the stringified pid
   * */

  // Boolean value to control if we still have $$ left
  int check = 1;
  
  /*
   * I will be using this to point to a substring. This is the beginning of the midpoint calculation needed to disassembled the passed
   * argument and then reassemble again with variable expansion
   * */
  char *discovery;
  /*
     * It's important to understand that there is a single set of argumens passed via the parameter *arg 
     * However, later on, I will be iterating over the arg set of memory addresses and actively manipulating them
     * So, this initial pointer permanently points to the beginning of the array
     * It's outside the while loop to make sure it permanently points to the beginning of the arguments. In this case, 
     * the beginning of the "arguments" is actually the command itself (eg. ls or echo) and should not be changing
     * as variable expansion should start after the first whitespace.
     * DO NOT MOVE OR REMOVE INITIAL OR FIDDLE WITH THE DECLARATION TIRED TED
     * */
  char *initial;

  initial = arg;

  /*
   * I may have gone overboard on the declarations. I use variable names as comments, even if it means a slight memory inefficiency.
   * Future refactoring would probably reduce a few of the char* and int counters into single variables, but at the moment it helps
   * me follow my own code.
   * */
  int pid;
  char *buf;
  char *temporary;
  int counter;
  int buf_counter;
  int arg_len;
  int temp_len;

  while(check == 1){

    // Extract pointer to beginning of $$ via the built in method strstr
    discovery = strstr(arg, "$$");
    
    /*
     * I have the habit of converting the classic C error check into this two liner because I like to assign directly from a function call
     * */
    if(discovery == NULL){
      check = 0;
    } else{
      /*
       * This is the majority of the work in the function. This is where we actually do the variable expansion.
       * */
      // Get pid
      pid = getpid();
      if(pid < 0){
        perror("Something went wrong with the fork");
      };
     

      buf = stringify_pid(pid);
      
      // Need a temporary data member to save the soon to be overwritten data into
            
      temporary = strdup(discovery);
      
      if(!temporary || !temporary[0]){
        perror("Could not make temporary array");
        exit(EXIT_FAILURE);
      };

      // I now have the length of the pid, it's stringified format, and the location it needs to be inserted
      // All that's left is to insert it.
      buf_counter = 0 ;
      counter = 0;
      arg_len = strlen(arg);
      
      // Set arg to be equal to discovery so as to not waste the first part of the array
      arg = discovery;

     
      // I fiddled with what to make the exit condition and settled on '\0' over NULL for readability.
      while(*arg != '\0'){
        counter++;
       
          int length = lengthify_stringified_pid(pid);
          // This is the exact moment we begin insert the expanded variables 
          for(; buf_counter < length; buf_counter++){
            // Immediately insert pid
            *arg++ = buf[buf_counter];
        
          };
          
          // Need to add white space for the tokenization later on
          *arg++ = ' ';

          // Make arg a separate string, so that strcat works with temporary
          *arg++ = '\0';

          // advance temporary three addresses so that the $$ is overwritten and no excess whitespace is allowed
          temporary = temporary + 2; 
          if(*temporary == ' '){
            temporary++;
          };
          // Check if a newline character is at the end of temporary
          temp_len = strlen(temporary);
          if(temporary[temp_len-1] == '\n'){
            temporary[temp_len-1] = '\0';
          };
          
          // Go back to the beginning of arg
          arg = arg-counter-2;                     
          arg = strcat(arg, temporary);
          arg_len = strlen(arg);
          // Exit loop by setting arg to the null pointer
          arg = arg+arg_len;
         
          *arg = '\0';
         
              
      }

      // Buf has to be dynamic, so we freed it
      free(buf);
      arg = initial; 
    };
  }
};
  

char **split_line(char *line){
  /*
   * This function takes a string, the argument passed in, and turns it into an array of pointers.
   * */
  int      bufsize = TOKEN_BUFSIZE;
  int      position = 0;
  char     **array_of_tokens = malloc(bufsize * sizeof(char*));
  char     *token;

  // Tokens should allocate, but I'm on a dual boot and there's limited memory. I've actually triggered this error twice.
  if(!array_of_tokens) {
    perror("Could not allocate memory for buffer, check memory constraints");
  }

  // Variable expansion is easiest before the tokens (so I don't have to reallocate, per the discussion on Discord)
  // However, I want to do it as close to the tokenization as possible to keep track of what is happening
  variable_expansion(line);
  
  // Full disclosure: I had never heard of the strtok function until I saw it on Discord. Very cool function. I was homebrewing my own
  // version, which was a mistake.
  token = strtok(line, TOKEN_DELIMITER);

  // Basic while string exists loop, but on the *token and not the **tokens
  while(token != NULL){
    array_of_tokens[position] = token;
    
    position++;
    if(position >= bufsize){
      bufsize += TOKEN_BUFSIZE;
      array_of_tokens = realloc(array_of_tokens, bufsize * sizeof(char*));
      if(!array_of_tokens){
        perror("Could not allocate memory for buffer, check memory constraints");
      }
    }
    token = strtok(NULL, TOKEN_DELIMITER);

  }
 
  array_of_tokens[position] = NULL;
  return array_of_tokens;
};

void handle_SIGTSTP(int signo){
  char *message = "Caught SIGTSTP: ";
  char *informative = "Entering foreground-only mode, & is ignored";
  char *exiting = "Exiting foreground-only mode, you can use & to send a process to the background";
  char *weird_error = "Something went desperately wrong and the foreground is not in a binary state";
  write(STDOUT_FILENO, message, strlen(message));
  
  // Identify process as parent (or not)
  if(getpid() == parent_pid){
    
    // Informative message if entering
    if(foreground_only == 0) {
      write(STDOUT_FILENO, informative, strlen(informative));
      foreground_only = 1;
    }
    else if(foreground_only == 1){
      write(STDOUT_FILENO, exiting, strlen(exiting));
      foreground_only = 0;
    }
    else{
      write(STDOUT_FILENO, weird_error, strlen(weird_error)); 
      foreground_only = 0;
     };

  };
};

void handle_SIGINT(int signo){
  char *message = "Caught SIGINT: ";
  int pid = getpid();
  char *str_pid =  stringify_pid(pid);
  char **exit_args = malloc(strlen(str_pid)+1);

  write(STDOUT_FILENO, message, 39);
  if(background == 1){
    fflush(stdout);
    write(STDOUT_FILENO, " background ", 200);
    exit_args[0] = "kill";
    exit_args[1] = str_pid;
    if(execvp(exit_args[0], exit_args) == -1){
     perror("Could not kill all children.");
      
    };
     // Remove pid from pids_smallsh_opened
    for(int index = 0; index < pid_counter -1; index++){
      int moving = 0;
      if(pids_smallsh_opened[index] == pid){
        pids_smallsh_opened[index] = pids_smallsh_opened[index+1];
        moving = 1;
        pid_counter--;
      } else if(moving == 1){
        pids_smallsh_opened[index] = pids_smallsh_opened[index+1];
          
      };
      if(index+1 == pid_counter && pids_smallsh_opened[index+1] == pid){
        pid_counter--;
        pids_smallsh_opened[index] = 0x0;
      };
    };
  free(exit_args);
  };
}

int thread_handling(struct Program_args current){
  
  pid_t pid, wpid;
  int status;
  int output_f;
  int input_f;
  int output_result;

  
  // Create a copy of the parent process
  pid = fork();

  // Let's weed out the problems with forking
  if(pid < 0){
    perror("Problem with forking");
    return EXIT_FAILURE;
  }
  // OK, the fork was successful. Let's now proceed to the case of a child
  else if (pid == 0){
    
    if(current.input_file || current.background[0] == 1){
      if(current.background[0] == 1 && !current.input_file){
        current.input_file = "/dev/null";
      };
      input_f = open(current.input_file, O_RDONLY, 0644);
      if(input_f < 0){
        perror("The input file could not be opened");
        return EXIT_FAILURE;
      };
      int input_result = dup2(input_f, STDIN_FILENO);
      if(input_result < 0){
        perror("Could not redirect to file from standard input");
        return EXIT_FAILURE;
      };
      
      while(fgets(current.tolkien, LINE_SIZE, stdin)){
        char **args = split_line(current.tolkien);
        int indexer = 0;
        int arg_indexer = 0;
        while(current.arguments[indexer] != NULL){
          indexer++;
        };
        while(args[arg_indexer] != NULL){
          current.arguments[indexer] = args[arg_indexer];
          indexer++;
          arg_indexer++;
        };
      };
    };
    // Redirection has to be done with open for the integer file description
    // Then fed to dup2, which requires an integer file description 
    // Then executed
    // Note, this is for output only. Input has to be opened, fed into arguments, and closed elsewhere
    if(current.output_file || current.background[0] == 1){
      if(current.background[0] == 1 && !current.output_file){
        current.output_file = "/dev/null";
      }
      // open, not fopen, because we need the int returned by open for dup2
      // I set permissions with what I think is the minimum required permissions for writing, but this could even be increased in need
      output_f = open(current.output_file, O_RDWR | O_CREAT, 0644);

      // open should return -1 as an error, but might as well take all less than 0
      if(output_f < 0){
        perror("There was a problem opening the output file. This is probably related to permissions or corruption of the file.");
      };

      // redirection is done via dup2 with the int returned from open
      output_result = dup2(output_f, STDOUT_FILENO); // stdout == 1 or FD 1
      if(output_result == -1){
        perror("Could not open or create output file");
        return EXIT_FAILURE;
      }
    };

 
    
    // execv* allows the **char argument passing, which in turn means
    // that arguments can be entered dynamically by the user and not
    // all defined at compile time
    // Found this on a stackoverflow post
    if(execvp(current.arguments[0], current.arguments) == -1){
      /* THIS IS THE ERROR HANDLING FOR EXECVP IN THE CHILD PROCESS */
     

      // We need to close down our file stream to prevent corruption (and security vulnerabilities)
      if(output_f){close(output_f);};
      if(input_f){close(input_f);};
      current.output_file = NULL;
      current.input_file = NULL;
     
      // This is where we know the file is closed and output is redirected to stdout. Still, this is error handling for execvp 
      // So, error and return a failure notice.
      perror("Problem executing latest command. Please try again.");
      return EXIT_FAILURE;
    } else{
      /* THIS IS THE NORMAL, ERROR-FREE PROCESSING IN THE CHILD PROCESS */

      if(current.background[0] == 1){
        background = 1;
        fflush(stdout);
        write(STDOUT_FILENO, stringify_pid(getpid()), 39);
      };

      if(output_f != 0x0 || output_f > 0 ){
        // Weird way to select redirection, right? 0x0 actually seems to work, but there's a part of me that worries about corruption
        // of the struct due to later refactoring. So I'm hitting them all
          if(close(output_f) == -1){
            perror("Can't close output file, somehow.");
            exit(EXIT_FAILURE);
          };
          if(close(input_f) == 01){
            perror("Can't close input file, somehow.");
          };
        };
     }

  }     
  
  // And now deal with the parent
  else {
    // I wanted to implement something like Go's goroutines
    // However, I'm not at that level of thread knowledge. Instead, I'm 
    // trying to use the thread libraries to smoothly switch between
    // child and parent. Since WIFEXITED and WIFSIGNALED give visibility to
    // the child's status, they're used here in a do-while to avoid hanging the
    // operation.
   
    pids_smallsh_opened[pid_counter] = pid;
    pid_counter++;
    int hang = 0;
    if(*current.background == 1){
      hang = WNOHANG;
      char *message = "Background process id: ";
      write(STDOUT_FILENO, "Background process id: ", strlen(message));
      char *actual_id = stringify_pid(pid);
      write(STDOUT_FILENO, actual_id, strlen(actual_id));
    };    
    do {
      wpid = waitpid(pid, &status, hang);
      
    } while(!WIFEXITED(status)&!WIFSIGNALED(status) && wpid == 0);
       // Remove pid from pids_smallsh_opened
    for(int index = 0; index < pid_counter -1; index++){
      int moving = 0;
      if(pids_smallsh_opened[index] == pid){
        pids_smallsh_opened[index] = pids_smallsh_opened[index+1];
        moving = 1;
        pid_counter--;
      } else if(moving == 1){
        pids_smallsh_opened[index] = pids_smallsh_opened[index+1];
          
      };
      if(index+1 == pid_counter && pids_smallsh_opened[index+1] == pid){
        pid_counter--;
        pids_smallsh_opened[index] = 0x0;
      };
    };
      
  }
  return status;
}

int change_directory(struct Program_args current){
  // Took this from the man pages for getcwd()

  long size;
  char *buf;
  char *ptr;

  // Changing directory requires a path, stored in current.arguments for reasons having to do with execvp in the non-builtin section
  // Therefore, no pathname, get an error
  if(current.arguments[1] == NULL){
    perror("New working directory not provided. Please try again.");
    return EXIT_FAILURE;
  };

  // This will spit out an error if you can't cd (say, if the path doesn't exit)
  if(chdir(current.arguments[1]) != 0){
    perror("Could not change working directory. Please try again.");
    
  };
  size = pathconf(".", _PC_PATH_MAX);

  if((buf = (char *)malloc((size_t)size)) != NULL){
    ptr = getcwd(buf, (size_t)size);
    printf("Changing directory...\n%s", ptr);
  };
  return EXIT_SUCCESS;
};

int show_status(struct Program_args current){
  fflush(stdout);
  printf("exit value for %s: %d", last_background.id, last_background.status);
  
  return EXIT_SUCCESS;
 };

int exit_program(void){
  char **exit_args = malloc(pid_counter + 2);
  exit_args[0] = "kill";
  
  for(int index = pid_counter; index > 0; index--){
    exit_args[index] = stringify_pid(pids_smallsh_opened[pid_counter]);
  }
  
  exit_args[pid_counter+1] = NULL;
  if(execvp(exit_args[0], exit_args) == -1){
     printf("Could not kill all children. %d", errno);
      
     };
  free(exit_args);
  exit(EXIT_SUCCESS);
};


int execute_args(struct Program_args current){


  // This is probably atrocious software engineering, but I'm hardcoding the builtins
  if(strcmp(current.command, "cd") == 0){
    return change_directory(current);
  };
  if(strcmp(current.command, "status") == 0){
    return show_status(current);
  };
  if(strcmp(current.command, "exit") == 0){
    return exit_program(); 
  };


  int handle_result = thread_handling(current);
  if(current.background[0] == 0){
    
    last_background.status = handle_result;  
    last_background.id = strdup(current.command);
    };
  return handle_result;
}


int command_loop(void){
  // The command loop is a basic REPL loop but for a c program
  //
  // Start with handling input, but there's an issue as a line is further divided into arguments
  // Status, meanwhile, defines whether the program should proceed to exiting
  int       status = EXIT_SUCCESS;
  char      **args;
  // Apparently structs survive while loops and don't reinitialize. So I'm creating a static, constant struct first
  static const struct Program_args eternal;

  while(status == EXIT_SUCCESS){
    int index = 0;
    int in_args = 1;
    struct Program_args current = eternal;
    printf("\n: ");
    fflush(stdout);
     
    if(fgets(current.tolkien, LINE_SIZE, stdin) == NULL){
      perror("Something went wrong reading from the standard input");
    
    };
    
    // Add error handling
    args = split_line(current.tolkien);
 
    /*
     * This started as a classic null check but evolved to define all behavior in which we do not want the input going to the 
     * exec functions at all. This includes checking for non-existence, such as with !args[0] and args == NULL
     * I had a million segfaults while developing this line so the order is very important
     * */
       // If there was an empty command, return failure
    if(!args[0] || args == NULL || strcmp(args[0], "\n")==0 || args[0][0] == '#' ){
      continue;
    };

    while(args[index] != NULL){
     /*
      * This section of code started out as a way to weed out the input and output files, hence the binary in_args flag
      * However, it also ended up being a way to peel off the name of the name as well
      * */ 
     if(strcmp(args[index], ">")==0){
        
        // saving the current index in the output_file member will actually save ">". We want to save the file name, which is at
        // index+1
        current.output_file = args[index+1];

        // I want to force the arguments string to recognize that the redirection commands and the & command are not to be included
        // later on for whatever program is being executed.
        args[index] = 0x0;
        
        if(current.output_file == NULL){
          perror("Empty filename or corrupted command line arguments.");
          return EXIT_FAILURE;
        };
        in_args = 0;
        index++;
      } else if(strcmp(args[index],"<")==0){
        current.input_file = args[index+1];
        
        // More forcing the arguments to cut off at redirection commands
        args[index] = 0x0;
     
        if(current.input_file == NULL){
          perror("Empty filename or corrupted command line arguments.");
          return EXIT_FAILURE;
        };
        in_args = 0;
        index++;
      } else if(strcmp(args[index], "&") == 0 && args[index+1] == NULL && foreground_only == 0){
        current.arguments[index] = 0x0;
        current.background[0] = 1;
        in_args = 0;
      } else if(strcmp(args[index],"&") == 0 && args[index+1] == NULL && foreground_only == 1){

        // In foreground-only mode, just skip the ampersand
        current.arguments[index] = 0x0;
        in_args = 0;
        continue;
      }else if(index == 0 && in_args == 1){
        // In the next two conditional cases, I do a fancy dance dereferencing the pointers. 
        // I couldn't get direct assignment to work, so I'm doing this. It works.
        current.arguments = args;
        current.command = args[0];
      }
     index++;
    }
       
   
    
    execute_args(current);} 
  return status;
}


int main(int argc, char **argv){
  /*
   * Main is a central entry point. 
   * However, since signals are outside the command loop, I'm introducing them here
   * Since they also control globals
   * */
 
   struct sigaction SIGINT_action = {0};
   struct sigaction SIGTSTP_action = {0};

   SIGINT_action.sa_handler = handle_SIGINT;
   SIGTSTP_action.sa_handler = handle_SIGTSTP;

   sigfillset(&SIGINT_action.sa_mask);
   sigfillset(&SIGTSTP_action.sa_mask);

   SIGINT_action.sa_flags = SA_RESTART;
   SIGTSTP_action.sa_flags = 0;

   sigaction(SIGINT, &SIGINT_action, NULL);
   sigaction(SIGTSTP, &SIGTSTP_action, NULL);
   sigset_t smallsh_signals;
  
   sigaddset(&smallsh_signals, SIGINT);
   parent_pid = getpid();
   command_loop();

   exit(EXIT_SUCCESS);
}
