# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <pthread.h>
# include <errno.h>
# include <sys/types.h>
# include <unistd.h>
# include <wait.h>
# include <fcntl.h>

# define LINE_SIZE 2048
# define TOKEN_BUFSIZE 64
# define TOKEN_DELIMITER " \t\r\n\a"
# define arraylen(x) (sizeof x /sizeof *x)

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
  int check = 1;
  printf("arg: %s", arg);
  while(check == 1){
    printf("checking");
    // Extract pointer to beginning of $$
    char *discovery = strstr(arg, "$$");
    //printf("Discover: %s", discovery);
    if(discovery == NULL){
      check = 0;
    } else{

      // Get pid
      int pid = getpid();
      if(pid < 0){
        perror("Something went wrong with the fork");
      };
     

      // I found these handy three lines on stackoverflow
      // The trick is that passing the NULL and 0 parameters gives you the length
      // of a potential integer you are about to convert to a string
      int length = snprintf( NULL, 0, "%d", pid);
      if(length == 0){
        perror("Could not find length of pid");
      };
      char *buf = malloc(length + 1);
      snprintf( buf, length+1, "%d", pid);
      
      // Need a temporary data member to save the soon to be overwritten data into
      char *temporary = strdup(arg);
      if(!temporary || !temporary[0]){
        perror("Could not make temporary array");
        exit(EXIT_FAILURE);
      };

      // I now have the length of the pid, it's stringified format, and the location it needs to be inserted
      // All that's left is to insert it.
      int buf_counter = 0 ;
      int counter = 0;
      int arg_len = strlen(arg);
      while(*arg != '\0'){
        counter++;
        // Is it before the substring? Fill in with temporary
        if(arg < discovery){
          *arg++ = *temporary++;
          counter++;
        } else if(arg == discovery){
          // This is the exact moment we begin insert the expanded variables 
          for(; buf_counter < length; buf_counter++){
            // Immediately insert pid
            *arg++ = buf[buf_counter];
        
          };
          
          // Need to add white space for the tokenization later on
          *arg++ = ' ';

          // Make arg a separate string, so that strcat works with temporary
          *arg++ = '\0';

          // advance temporary two addresses so that the $$ is overwritten
          temporary = temporary + 2;
          
          // Check if a newline character is at the end of temporary
          int temp_len = strlen(temporary);
          if(temporary[temp_len-1] == '\n'){
            temporary[temp_len-1] = '\0';
          };
         
          arg = arg-counter-2;                     // Go back to the beginning of arg
          arg = strcat(arg, temporary);
          arg_len = strlen(arg);
          // Exit loop by setting arg to the null pointer
          arg = arg+arg_len;
          printf("arg %s",arg);
          *arg = '\0';        
        }      
      }

      // Buf has to be dynamic, so we freed it
      free(buf);
      printf("Leaving expansion");
    };
  }
};
  


struct Program_args {
  char     **arguments;
  char     *tokens;
  int      background[1];// = 1;                                // I'm making background = 0, so the default is the foreground
  char     *input_file;
  char     *output_file;
  int      input_file_opened;
  int      output_file_opened;
  char     *command;                                         // Storing the command name for later processing
  char     tolkien[2048];                                         // tolkien = token, this is the tokenized command line arguments
};    

char **split_line(char *line){
  int      bufsize = TOKEN_BUFSIZE;
  int      position = 0;
  char     **tokens = malloc(bufsize * sizeof(char*));
  char     *token;

  if(!tokens) {
    fprintf(stderr, "Could not allocate memory for buffer, check memory constraints");
    exit(EXIT_FAILURE);
  }
  variable_expansion(line);
  token = strtok(line, TOKEN_DELIMITER);
  while(token != NULL){
    tokens[position] = token;
    
    position++;

    if(position >= bufsize){
      bufsize += TOKEN_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if(!tokens){
        fprintf(stderr, "Could not allocate memory for buffer, check memory constraints");
        exit(EXIT_FAILURE);

      }
    }
    token = strtok(NULL, TOKEN_DELIMITER);

  }
  //printf("leaving tokens");
  tokens[position] = NULL;
  return tokens;
};



int thread_handling(struct Program_args current){
  
  pid_t pid, wpid;
  int status;
  int output_f;
  int input_f;


  // Create a copy of the parent process
  pid = fork();
  fflush(stdout);
  // Let's weed out the problems with forking
  if(pid < 0){
    perror("Problem with forking");
    return EXIT_FAILURE;
  }
  // OK, the fork was successful. Let's now proceed to the case of a child
  else if (pid == 0){
    printf("\ncommand: %s\n arguments: %s\n", current.arguments[0], current.arguments[1]); 

    // Redirection has to be done with open for the integer file description
    // Then fed to dup2, which requires an integer file description 
    // Then executed
    if(current.input_file || current.output_file){
      printf("\nInput file handling: %s\n", current.input_file);
      if(current.input_file){
        input_f = open(current.input_file, O_RDONLY | O_CREAT, 0644);

        if(input_f != -1){
          current.input_file_opened = input_f;
          int input_result = dup2(input_f, 0); // stdin == 0 or FD 0
          if(input_result == -1){
            return EXIT_FAILURE;
          };
        } else{
          return EXIT_FAILURE;
          };
      };
      if(current.output_file){
        output_f = open(current.output_file, O_RDWR | O_CREAT, 0644);

        if(output_f != -1){
          current.output_file_opened = output_f;
          int output_result = dup2(output_f, 1); // stdout == 1 or FD 1
          if(output_result == -1){
            perror("Could not open or create output file");
            return EXIT_FAILURE;
          }
        } else {
          return EXIT_FAILURE;
        };
      };
    };
        // execv* allows the **char argument passing, which in turn means
    // that arguments can be entered dynamically by the user and not
    // all defined at compile time
    // Found this on a stackoverflow post

    if(execvp(current.arguments[0], current.arguments) == -1){
      
      if(output_f){
          dup2(1, output_f);
          if(close(output_f) == -1){
            perror("Can't close output file, somehow.");
            exit(EXIT_FAILURE);
          };
        };
        if(input_f){
          dup2(0, input_f);
          if(close(input_f) == -1){
            perror("Can't close input file, somehow.");
          };
        };
     
 
      perror("Problem executing latest command. Please try again.");
      return EXIT_FAILURE;
    }

      if(output_f){
          dup2(1, output_f);
          if(close(output_f) == -1){
            perror("Can't close output file, somehow.");
            exit(EXIT_FAILURE);
          };
        };
        if(input_f){
          dup2(0, input_f);
          if(close(input_f) == -1){
            perror("Can't close input file, somehow.");
          };
        };
     
    
  }
  // And now deal with the parent
  else {
    // I wanted to implement something like Go's goroutines
    // However, I'm not at that level of thread knowledge. Instead, I'm 
    // trying to use the thread libraries to smoothly switch between
    // child and parent. Since WIFEXITED and WIFSIGNALED give visibility to
    // the child's status, they're used here in a do-while to avoid hanging the
    // operation.
 
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
      //printf("waiting: %d", wpid);
    } while(!WIFEXITED(status)&!WIFSIGNALED(status));

  
  }
  return EXIT_SUCCESS;
}



int execute_args(struct Program_args current){
     
  
  return thread_handling(current);
}


int command_loop(void){
  // The command loop is a basic REPL loop but for a c program
  //
  // Start with handling input, but there's an issue as a line is further divided into arguments
  // Status, meanwhile, defines whether the program should proceed to exiting
  int       status = EXIT_SUCCESS;
  char      **args;
  char      buffer[LINE_SIZE];
  
  while(status == EXIT_SUCCESS){
    int index = 0;
    int in_args = 1;
    struct Program_args current;
    printf("\n: ");
    fflush(stdout);
     
    if(fgets(current.tolkien, LINE_SIZE, stdin) == NULL){
      perror("Something went wrong reading from the standard input");
    };
    args = split_line(current.tolkien);
    //printf("args: %s", *args);

    /*
     * This started as a classic null check but evolved to define all behavior in which we do not want the input going to the 
     * exec functions at all. This includes checking for non-existence, such as with !args[0] and args == NULL
     * I had a million segfaults while developing this line so the order is very important
     * */
       // If there was an empty command, return failure
    if(!args[0] || args == NULL || strcmp(args[0], "\n")==0 || strcmp(args[0], "#") == 0  ){
      continue;
    };

    int counter = 0; 
    while(args[index] != NULL){
     /*
      * This section of code started out as a way to weed out the input and output files, hence the binary in_args flag
      * However, it also ended up being a way to peel off the name of the name as well
      * */ 
     if(strcmp(args[index], ">")==0){
        
        // saving the current index in the output_file member will actually save ">". We want to save the file name, which is at
        // index+1
        current.output_file = args[index+1];
        if(current.output_file == 0x0){
          perror("Empty filename or corrupted command line arguments.");
          return EXIT_FAILURE;
        };
        in_args = 0;
      } else if(strcmp(args[index],"<")==0){
        current.input_file = args[index];
        in_args = 0;
      } else if(index == 0){
        // In the next two conditional cases, I do a fancy dance dereferencing the pointers. 
        // I couldn't get direct assignment to work, so I'm doing this. It works.
        current.arguments = args;
        current.command = args[0];
      } else if(in_args == 1) {
        current.arguments[index] = args[index];
      }
      index++;
    }
    
    if(strcmp(args[index-1],"&")==0){
      
      current.background[0] = 0;
    }
       
   
  
    status = execute_args(current);} 
    
  return status;
}

int main(int argc, char **argv){
  /*
   * Main is a central entry point. 
   *
   * */

  command_loop();

  exit(EXIT_SUCCESS);
}
