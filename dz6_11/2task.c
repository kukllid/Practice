#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char ** argv){
  

  for (int i = 1; i < argc; i++) {
    if(fork()>0){
      //printf("father created %d\n", i);
    } else {
      execl(argv[i], argv[i], (char*)0);
      //printf("son %d\n", i);
      return 0;
    }
  }
  int tmp;
  for (int i = 1; i < argc; i++) {
    wait(&tmp);
  }
  return 0;
}

