#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>


void stand(int s){
  printf("FOUND\n!");
  kill(0, SIGKILL);
  exit(0);
}

int main(int argc, char ** argv){
  char *str = "aaa\0"; //string we need to find
  char buf[1001];
  signal(SIGUSR1, stand);
  for (int i = 1; i < argc; i ++){
    pid_t pid = fork();
    if (pid == 0){
     FILE *x = fopen(argv[i], "r");
      while(fgets(buf, 1000, x)){
        if (strstr(buf, str)){
          kill(getppid(), SIGUSR1);
        }
      }
      exit(0);
    }
  }
  int s;
  while(wait(&s) > 0){
    
  }
  printf("NOT FOUND\n");
  return 0;
}
