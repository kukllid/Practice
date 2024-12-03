#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(int argc, char ** argv){
  int fd[2];
  pipe(fd);
  dup2(fd[1], 1);
  pid_t cm12 = fork();
  if (cm12 == 0){
    pid_t cmd1 = fork();
    if (cmd1 == 0){
      close(fd[0]);
      close(fd[1]);
      execlp(argv[1], argv[1], NULL);
      _exit(1);
    }
    while(wait(NULL)>0){};
    close(fd[0]);
    close(fd[1]);
    execlp(argv[2], argv[2], NULL);
    _exit(1);
  } 
  
  dup2(fd[0], 0);
  int f = open(argv[5], O_RDWR | O_CREAT | O_APPEND, 0777);
  dup2(f, 1);
  close(f);
  close(fd[0]);
  close(fd[1]);
  pid_t cmd3 = fork();
  int status;
  if (cmd3 == 0){
    execlp(argv[3], argv[3], NULL);
    _exit(1);
  }
  waitpid(cmd3, &status, 0);
  if (!WIFEXITED(status) || WEXITSTATUS(status)){
    pid_t cmd4 = fork();
    if (cmd4 == 0){
      execlp(argv[4], argv[4], NULL);
      _exit(1);
    }
    while(wait(NULL)>0);
  } 

  return 0;
}
