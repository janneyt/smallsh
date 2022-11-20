# include <stdio.h>
# include <errno.h>
# include <err.h>
# include <stdint.h>
# include <unistd.h>
# include <string.h>
# include <stdlib.h>

#ifndef UINT8_MAX
#error "No support for uint8_t"
#endif

# define arraylen(x) (sizeof x / sizeof *x)
# define LINELENGTH 1000
# define STOPPROCESSING 80
# define SUBSTRINGPLUS "++"
# define SUBSTRINGSPACE "\n"

char *replacement(char *line, char *substring, char replacer){
  // I came up with this while working on smallsh
  // This will have to be threadified
  char *finder = strstr(line, substring);

  char *temp = finder;
  while(finder != NULL){
    temp = strdup(finder);

    *finder = replacer;

    finder++;
    temp = temp + 2;
    *finder = '\0';

    strcat(line, temp);

    finder = strstr(line, substring);
    
  }
  printf("Before returning: %s\n", line);
  return line;
};


int scan_input(FILE* file){
  // Scans for input
  // Will have to be threadified
  int counter = 0;
  char *result = NULL; 
  for(;;){
    char line[LINELENGTH];
    if(fgets(line, LINELENGTH, file) == NULL){
      perror("Could not read from stdin");
    };


    if(strcmp(line, "STOP\n") == 0 && strlen(line) == 5){
      return EXIT_SUCCESS;
    };
    
    result = replacement(line, SUBSTRINGPLUS, '^');

    result = replacement(line, SUBSTRINGSPACE, ' ');

  }
}

int main(int argc, char *argv[]){

  // Result is 1, or not successful
  int result = 1; 
  result = scan_input(stdin);
  if(result != 0){
    perror("Scanning input did not work");
  };
}
