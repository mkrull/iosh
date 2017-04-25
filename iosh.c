#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

#define prompt "iosh> "
#define exit_cmd "exit"

int main(int argc, char** argv){
  puts("iosh version 0.0.1-pre-alpha\n");

  while (1) {
    char *input = readline(prompt);
    add_history(input);
    if (strncmp(input, exit_cmd, sizeof(exit_cmd)) == 0) {
      exit(0);
    }

    printf("Recieved: %s\n", input);
    free(input);
  }

  return 0;
}
