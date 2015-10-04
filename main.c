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

int main(int argc, char **argv) {
    int running = 1;
    char * prompt = "Enter a command>>";
    while (running){
      printf("%s", prompt);
      fflush(stdout);
      char buffer[1024];
      while (fgets(buffer, 1024, stdin) != NULL) {
          int len = strlen(buffer);
          if (buffer[len-1] == '\n'){
            buffer[len-1] = '\0';
          }
          if (strncmp(buffer, "exit", len) == 0){
            exit(0);
          }
          else{
            printf("%s\n", buffer);
          }
          break;
        }
    }
    return 0;
}
