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

void remove_comments(char * s){
  for(int i = 0; i < strlen(s); i++){
    if (s[i] == '#'){
      s[i] = '\0';
    }
  }
}

char** tokenify(const char *s, const char *delimiter) {
    char *s2 = strdup(s); //both s and s2 now contain the string
    char *s3 = strdup(s);
    int num_of_words = 0;
    char *token;
    for(token = strtok(s2, delimiter); token != NULL; token = strtok(NULL, delimiter)){
      num_of_words++;
    }
    free(s2);
    char **s4 = malloc(sizeof(char *) * (num_of_words+1));
    for(int i = 0; i < num_of_words; i++){
      if (i == 0){
        s4[i] = strdup(strtok(s3, delimiter));
    }
    else{
      s4[i] = strdup(strtok(NULL, delimiter));
      }
    }
    free(s3);
    s4[num_of_words] = NULL;
    return s4;
}

void free_tokens(char **tokens) {
    int i = 0;
    while (tokens[i] != NULL) {
        free(tokens[i]); // free each string
        i++;
    }
    free(tokens); // then free the array
}

void print_tokens(char *tokens[]) {
    int i = 0;
    while (tokens[i] != NULL) {
        printf("Token %d: %s\n", i+1, tokens[i]);
        i++;
    }
}

void s_run_command(char * arg, char ** parameters){
  pid_t pid = fork();
  if (pid == -1){
    printf("Fork error :(\n");
  }
  else if (pid == 0){
    if (execv(arg, parameters) < 0){
      printf("Made a bad call\n");
    }
  }
  waitpid(-1, &pid, 0);
}

void mode(char **s, int *seq){
  if (s[1] == NULL){
    if (*seq){
      printf("Current mode is sequential\n");
    }
    else{
      printf("Current mode is parrallel\n");
    }
  }
  else if(s[2] != NULL){
      printf("Bad call, mode take 0 or 1 parameters\n");
  }
  else{
    char * state = s[1];
    int len = strlen(state);
    if (strncmp(state, "sequential", len) == 0){
        printf("Mode is now sequential\n");
        *seq = 1;
    }
    else if (strncmp(state, "s", len) == 0){
        printf("Mode is now sequential\n");
        *seq = 1;
    }
    else if (strncmp(state, "parallel", len) == 0){
        printf("Mode is now parallell\n");
        *seq = 0;
    }
    else if (strncmp(state, "p", len) == 0){
        printf("Mode is now parallel\n");
        *seq = 0;
    }
    else{
      printf("%s is not a valid mode\n", state);
    }
  }
}

int main(int argc, char **argv) {
    int running = 1;
    int sequential = 1; //is it sequential or not (parallel)
    while (running){
      printf("Enter a command>>");
      fflush(stdout);
      char buffer[1024];
      while (fgets(buffer, 1024, stdin) != NULL) {
          int exit_condition = 0;
          int *new_sequential = malloc(sizeof(int));
          *new_sequential = sequential;
          if (feof(stdin)){
            running = 0;
            break;
          }
          if (buffer[0] =='\n'){
            break;
          }
          remove_comments(buffer);
          char semi[] = {";"};
          char **commands = tokenify(buffer, semi);
          int i = 0;
          char whitespace[] = {" \t\n"};
          while (commands[i] != NULL){
            char ** curr_command = tokenify(commands[i], whitespace);
            if (curr_command[0] == NULL){ //this happens if the last item is a semicolon
              break;
            }
            else if (strncmp(curr_command[0], "exit", strlen("exit")) == 0){
                if (curr_command[1] != NULL){
                  printf("Bad call, exit doesn't take parameters\n");
                }
                else {
                  exit_condition = 1;
                }
            }
            else if (strncmp(curr_command[0], "mode", strlen(curr_command[0]))==0) {
              mode(curr_command, new_sequential); //prints or updates mode for next run
            }
            else if (sequential){
              s_run_command(curr_command[0], curr_command);
            }
            free_tokens(curr_command);
            i++;
          }
          sequential = *new_sequential;
          free(new_sequential);
          free_tokens(commands);
          if (exit_condition){
            exit(0);
          }
          break;
        }
    }
    return 0;
}
