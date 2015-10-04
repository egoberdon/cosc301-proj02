#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>
#include <signal.h>

void remove_comments(char * s, int len){
  for(int i = 0; i < len; i++){
    if (s[i] == '#'){
      s[i] = '\0';
    }
  }
}

char** tokenify(const char *s) {
    char *s2 = strdup(s); //both s and s2 now contain the string
    char *s3 = strdup(s);
    char badstuff[] = {" \t\n"};
    int num_of_words = 0;
    char *token;
    for(token = strtok(s2, badstuff); token != NULL; token = strtok(NULL, badstuff)){
      num_of_words++;
    }
    free(s2);
    char **s4 = malloc(sizeof(char *) * num_of_words+1);
    for(int i = 0; i < num_of_words; i++){
      if (i == 0){
        s4[i] = strdup(strtok(s3, badstuff));
    }
    else{
      s4[i] = strdup(strtok(NULL, badstuff));
      }
    }
    s4[num_of_words] = NULL;
    free(s3);
    return s4;
}

int main(int argc, char **argv) {
    int running = 1;
    char * prompt = "Enter a command>>";
    while (running){
      printf("%s", prompt);
      fflush(stdout);
      char buffer[1024];
      while (fgets(buffer, 1024, stdin) != NULL) {
          int len = strlen(buffer);
          char ** buffer_pointer = tokenify(buffer);
          char * commands = *buffer_pointer;
          remove_comments(commands, len);
          if (strncmp(commands, "exit", len) == 0){
            exit(0);
          }
          else if (strncmp(commands, "/bin/ls", len) == 0){
            system(commands);
          }
          else{
            printf("You did something wrong, it happens ¯\\_(ツ)_/¯\n");
          }
          free(buffer_pointer);
          break;
        }
    }
    return 0;
}
