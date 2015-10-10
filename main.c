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

typedef struct __node__{
  char dir[256];
  struct __node__ * next;
} node ;

typedef struct __kid__{
  pid_t pid;
  struct __kid__ * next;
} kid;

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

void num_tokens(char **tokens, int *n) {
    int i = 0;
    while (tokens[i] != NULL) {
        i++;
    }
    *n = i;
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

void p_run_command(char * arg, char ** parameters, kid * kid_head){
  while (kid_head != NULL){
    kid_head = kid_head->next;
  }
  (kid_head->pid) = fork();
  if ((kid_head->pid) == -1){
    printf("Fork error :(\n");
  }
  else if ((kid_head->pid) == 0){
    if (execv(arg, parameters) < 0){
      printf("Made a bad call\n");
    }
  }
}

void kill_pids(kid * head){
  while(head != NULL){
    kid *temp = head;
    head = head->next;
    waitpid(-1, &(temp->pid), 0);
    }
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
      printf("Bad call, mode takes 0 or 1 parameters\n");
  }
  else{
    char * state = s[1];
    int len = strlen(state);
    if (strncmp(state, "sequential", len) == 0){
          *seq = 1;
          printf("Mode is now sequential\n");
    }
    else if (strncmp(state, "parallel", len) == 0){
        *seq = 0;
        printf("Mode is now parallell\n");
    }
    else{
      printf("%s is not a valid mode\n", state);
    }
  }
}

node *read_config() {
  FILE * file;
  file = fopen("shell-config", "r");
  node *head = malloc(sizeof(node));
  node *prev = head;
  if (file != NULL){
    char word[256];
    while(fgets(word, sizeof(word), file) != NULL){
      for(int i = 0; i < strlen(word); i++){
        if (word[i] == '\n'){
          word[i] = '/'; //since items come in without second dash
          word[i+1] = '\0';
        }
      }
      node *new = malloc(sizeof(node));
      strncpy(new->dir,word,sizeof(word));
      prev->next = new;
      prev = prev->next;
    }
  }
  fclose(file);
  return head;
}

void free_nodes(node *head) {
  while(head != NULL){
    node *temp = head;
    head = head->next;
    free(temp);
  }
}

void free_kids(kid *head) {
  while(head != NULL){
    kid *temp = head;
    head = head->next;
    free(temp);
  }
}

void path_finder(char ** command, int sequential, node * dir_head, kid * kid_head){
  struct stat statresult;
  int found = 0;
  int rv = stat(command[0], &statresult);
  if (!(rv < 0)){
    if(sequential){
      s_run_command(command[0], command);
    }
    else{
      p_run_command(command[0], command, kid_head);
    }
  }
  else{
    while (dir_head != NULL){
      char s1[256];
      strncpy(s1, dir_head->dir, 255);
      char *s2 = command[0];
      strcat(s1, s2); //s1 is now directory + command
      rv = stat(s1, &statresult);
      if (!(rv < 0)){
        free(command[0]); //free previous contents
        command[0] = strdup(s1); //update contents
        if(sequential){
          s_run_command(command[0], command);
        }
        else{
          p_run_command(command[0], command, kid_head);
        }
        found = 1;
        break;
      }
      else{
        dir_head = dir_head->next;
      }
    }
    if (!(found)){
      printf("Made a bad call\n"); //should this still pass to execv?
    }
  }
}
void resume_pid(kid * head, pid_t pid){
  int found = 0;
  while(head != NULL){
    if (head->pid == pid){
      kill(pid, SIGSTOP);
      found = 1;
      break;
    }
    head = head->next;
  }
  if (! found){
    printf("No such PID found\n");
  }
}

void pause_pid(kid * head, pid_t pid){
  int found = 0;
  while(head != NULL){
    if (head->pid == pid){
      kill(pid, SIGCONT);
      found = 1;
      break;
    }
    head = head->next;
  }
  if (! found){
    printf("No such PID found\n");
  }
}

void jobs(kid * head){
  printf("List of jobs below:\n");
  int counter = 0;
  while(head != NULL){
    printf("%ld", (long)head->pid);
    head = head->next;
    counter++;
    }
    if (counter==0){
      printf("No active jobs");
    }
  }

int main(int argc, char **argv) {
    int running = 1;
    int sequential = 1; //is it sequential or not (parallel)
    int exit_condition = 0;
    int PATH = 0;
    struct stat statresult;
    int rv = stat("./shell-config", &statresult);
    node * dir_head = NULL;
    kid * kid_head = NULL;
    if (!(rv < 0)){
      PATH = 1;
      dir_head = read_config();
    }
    while (running){
      printf("Enter a command>>");
      fflush(stdout);
      char buffer[1024];
      if (fgets(buffer, 1024, stdin) == NULL){
        exit(0);
      }
      if (feof(stdin)){
        exit(0);
      }
      else if (buffer[0] !='\n'){
        int *new_sequential = malloc(sizeof(int));
        *new_sequential = sequential;
        remove_comments(buffer);
        char semi[] = {";"};
        char **commands = tokenify(buffer, semi);
        int *num_commands = malloc(sizeof(int));
        num_tokens(commands, num_commands); //could just get this from an extra param on tokenify
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
          else if (strncmp(curr_command[0], "jobs", strlen(curr_command[0]))==0){
            jobs(kid_head);
          }
          else if(strncmp(curr_command[0], "pause", strlen(curr_command[0]))==0){
            if (curr_command[1] == NULL){
              printf("Bad call, must enter PID to pause\n");
            }
            else{
                 pause_pid(kid_head, sprintf(curr_command[1] + strlen(curr_command[1]), "%ld", (long) curr_command[1]));
                 ;
            }
          }
          else if(strncmp(curr_command[0], "resume", strlen(curr_command[0]))==0){
            if (curr_command[1] == NULL){
              printf("Bad call, must enter PID to resume\n");
            }
            else{
                resume_pid(kid_head, sprintf(curr_command[1] + strlen(curr_command[1]), "%ld", (long) curr_command[1]));
            }
          }
          else if (sequential){
            if(!(PATH)){
              s_run_command(curr_command[0], curr_command);
            }
            else{
              path_finder(curr_command, sequential, dir_head, kid_head);
            }
          }
          else if (!sequential){
            if(!(PATH)){
              p_run_command(curr_command[0], curr_command, kid_head);
            }
            else{
              path_finder(curr_command, sequential, dir_head, kid_head);
            }
          }
          free_tokens(curr_command);
          i++;
        }
        if (!sequential){ //delay kill_pids, to
          kill_pids(kid_head);
        }
        free(num_commands);
        sequential = *new_sequential;
        free(new_sequential);
        free_tokens(commands);
        free_kids(kid_head);
        if (exit_condition){
          exit(0);
        }
      }
    }
    free_nodes(dir_head);
    return 0;
}
