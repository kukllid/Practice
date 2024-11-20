#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

int c = 0;
pid_t target;
void react(int s){
  printf("Son printed: pid = %d, iteration = %d \n", getpid(), c++);
  kill(target, SIGUSR1);
}
int main(){
  pid_t pid1, pid2;
  int fd[2];
  pipe(fd);
  int fd1[2];
  pipe(fd1);
  
  pid1 = fork();
  signal(SIGUSR1, react);
  if (pid1 < 0) {
    printf("Error");
    return 1;
  } else if (pid1 == 0){
    read(fd[0], &target, sizeof(target));
    write(fd1[1], &c, sizeof(int));
    close(fd[0]);
    close(fd1[0]);
    close(fd1[1]);
    close(fd[1]);
    while(1){
    }
  }
  
  pid2 = fork();
  if (pid2 < 0) {
    printf("Error");
    return 1;
  } else if(pid2 == 0){
    pid_t cur = getpid();
    write(fd[1], &cur, sizeof(target));
    read(fd1[0], &c, sizeof(int));
    close(fd[0]);
    close(fd1[0]);
    close(fd1[1]);
    close(fd[1]);
    target = pid1;
    kill(target, SIGUSR1);
    while(1){
      
    }
  }
  sleep(10);
  kill(pid1, SIGKILL);
  kill(pid2, SIGKILL);
  close(fd[0]);
  close(fd1[0]);
  close(fd1[1]);
  close(fd[1]);
  printf ("Sons will be determinated \n");
  return 0;
}
