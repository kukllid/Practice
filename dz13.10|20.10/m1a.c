#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(int argc, char ** argv){
  int fd[2];
  pipe(fd);
  pid_t pid1 = fork();
  
  if (pid1 == 0){
    dup2(fd[1], 1);
    close(fd[0]);
    close(fd[1]);
    execlp(argv[1], argv[1], argv[2], argv[3], NULL);
    printf("ERROR1\n");
    exit(1);
  }
  dup2(fd[0], 0);
  close(fd[0]);
  close(fd[1]);
  pid_t pid2 = fork();
  if (pid2 == 0){
    close(fd[0]);
    close(fd[1]);
    execlp(argv[4], argv[4], NULL);
    printf("ERROR2\n");
    exit(1);
  }
  int status;
  while(wait (&status) > 0){
  
  }
  int x = open(argv[6], O_RDWR | O_CREAT | O_APPEND, 0666);
  dup2(x, 1);
  execlp (argv[5], argv[5], NULL);
  close(x);
  return 1;
}
