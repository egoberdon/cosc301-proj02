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

int *num_tokens(char *tokens[]) {
    int *num = malloc(sizeof(int));
    int i = 0;
    while (tokens[i] != NULL) {
        i++;
    }
    *num = i;
    return num;
}

void print_tokens(char *tokens[]) {
    int i = 0;
    while (tokens[i] != NULL) {
        printf("Token %d: %s\n", i+1, tokens[i]);
        i++;
    }
}

void print_parser(char ***runs) {
  int j = 0;
  while (runs[j] != NULL){
    int k = 0;
    while(runs[j][k]!= NULL){
      printf("Command %d, Token %d: %s\n", j, k, runs[j][k]);
      k++;
    }
    j++;
  }
}

void free_tokens(char **tokens) {
    int i = 0;
    while (tokens[i] != NULL) {
        free(tokens[i]); // free each string
        i++;
    }
    free(tokens); // then free the array
}

void free_commands(char *** runs){
  int j = 0;
  while (runs[j] != NULL){
    int k = 0;
    while(runs[j][k]!=NULL){
      free(runs[j][k]);
      k++;
    }
    free(runs[j]);
    j++;
  }
  free(runs);
}

char *** semi_parser(char **command_list){
  int * num = num_tokens(command_list); //bigger than we need, but simplifies/speeds up the semi_parser
  int n = *num;
  char ***master_list = malloc(sizeof(char **) * (n+1));
  for(int x = 0; x < n+1; x++){
    char **inner = malloc(sizeof(char *) * n);
    master_list[x] = inner;
  }
  int i = 0;
  int j = 0;
  int k = 0;
  while (command_list[i] != NULL){
    char * word = command_list[i];
    int len = strlen(word);
    if(strncmp(word,";",len)==0){
      master_list[j][k] = NULL;
      j++;
      k = 0;
    }
    else{
      master_list[j][k] = strdup(word);
      k++;
    }
    i++;
  }
  master_list[j+1] = NULL;
  free(num);
  return master_list;
}

void run_command(char * arg, char ** parameters){
  pid_t pid = fork();
  if (pid == -1){
    printf("Fork error :(\n");
  }
  else if (pid == 0){
    execv(arg, parameters);
    printf("You messed up with that call\n");
  }
  waitpid(-1, &pid, 0);
}

int main(int argc, char **argv) {
    int running = 1;
    char * prompt = "Enter a command>>";
    while (running){
      printf("%s", prompt);
      fflush(stdout);
      char buffer[1024];
      while (fgets(buffer, 1024, stdin) != NULL) {
          if (buffer[0] =='\n'){
            break;
          }
          int len = strlen(buffer);
          remove_comments(buffer, len);
          if(buffer[len-1] =='\n'){
            buffer[len-1] = '\0';
          }
          if (strncmp(buffer, "exit", len) == 0){ //sometimes have to type this twice? after other commands have been typed
            exit(0);
          }
          else if (strncmp(buffer, "clear", len) == 0){
            system("clear");
          }
          else if (strncmp(buffer, "mode", len) == 0){
            printf("Mode not yet implemented\n");
          }
          else{
            char ** word_list = tokenify(buffer);
            char *** runs = semi_parser(word_list);
            int j = 0;
            while(runs[j] != NULL){
              run_command(runs[j][0], runs[j]);
              j++;
            }
            free_tokens(word_list);
            free_commands(runs);
          }
          break;
        }
    }
    return 0;
}
