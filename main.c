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
    free(s3);
    s4[num_of_words] = NULL;
    return s4;
}

// int *num_tokens(char *tokens[]) {
//     int *num = malloc(sizeof(int));
//     int i = 0;
//     while (tokens[i] != NULL) {
//         i++;
//     }
//     *num = i;
//     return num;
// }

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

char *** semi_parser(char **command_list){
  //int * num = num_tokens(command_list);
  //int n = *num;
  int i = 0;
  int j = 0; //number of seperate commands
  int k = 0; //number of words per command
  int num_per_command[300];//use this to store number of words per [j]
  while (command_list[i] != NULL){//Count number of commands, count number of words in each command
    char * word = command_list[i];
    int len = strlen(word);
    if(strncmp(word,";",len)==0){
      num_per_command[j] = k;
      k = 0;
      j++;
    }
    else{
      k++;
    }
    i++;
  }
  if(strncmp(command_list[i-1],";",strlen(command_list[i-1]))!=0){ //if the last string was not a semicolon, one more store needs to happen
    num_per_command[j] = k;
    j++;
  }
  char ***master_list = malloc(sizeof(char **) * j);
  master_list[j] = NULL;
  for(int x = 0; x < j; x++){
    master_list[x] = malloc(sizeof(char *) * num_per_command[x]);
  }

  i = 0;
  j = 0;
  k = 0;
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
  if(strncmp(command_list[i-1],";",strlen(command_list[i-1]))!=0){ //if the last string was not a semicolon, one more store needs to happen
    master_list[j][k] = NULL;
  }
  //free(num);
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
    while (running){
      printf("Enter a command>>");
      fflush(stdout);
      char buffer[1024];
      if (running!=1){
        break;
      }
      while (fgets(buffer, 1024, stdin) != NULL) {
          if (feof(stdin)){
            running = 0;
            break;
          }
          if (buffer[0] =='\n'){
            break;
          }
          int len = strlen(buffer);
          remove_comments(buffer, len);
          if(buffer[len-1] =='\n'){
            buffer[len-1] = '\0';
          }
          if (strncmp(buffer, "exit", len) == 0){ //sometimes have to type this twice? after other commands have been typed
            running = 0;
            break;
          }
          else if (strncmp(buffer, "clear", len) == 0){
            system("clear"); //I like a clean terminal
          }
          else if (strncmp(buffer, "mode", len) == 0){
            printf("Mode not yet implemented\n");
          }
          else{
            char ** word_list = tokenify(buffer);
            print_tokens(word_list);
            char *** runs = semi_parser(word_list);
            printf("Going to free tokens now\n");
            free_tokens(word_list);
            print_parser(runs);
            int j = 0;
            while(runs[j] != NULL){
              run_command(runs[j][0], runs[j]);
              free_tokens(runs[j]);
              j++;
            }
            free(runs);
          }
          break;
        }
    }
    return 0;
}
