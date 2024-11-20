#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

int main(){
  
  int c = 0;
  pid_t pid1, pid2;
  int fd1_2[2];
  pipe(fd1_2);
  int fd2_1[2];
  pipe(fd2_1);
  pid1 = fork();
  
  if (pid1 < 0) {
    printf("Error");
    return 1;
  } else if (pid1 == 0){
    while(1){
      read(fd2_1[0], &c, sizeof(int));
      printf("Son 1 printed: pid1 = %d, iteration = %d \n", getpid(), c);
      write(fd1_2[1], &c, sizeof(int));
    }
  }
  
  pid2 = fork();
  if (pid2 < 0) {
    printf("Error");
    return 1;
  } else if(pid2 == 0){
    while(1){
      printf("Son 2 printed: pid2 = %d, iteration = %d \n", getpid(), c);
      write(fd2_1[1], &c, sizeof(int));
      read(fd1_2[0], &c, sizeof(int));
      c ++;
    }
  }
  sleep(10);
  close(fd1_2[0]);
  close(fd1_2[1]);
  close(fd2_1[0]);
  close(fd2_1[1]);
  kill(pid1, SIGKILL);
  kill(pid2, SIGKILL);
  printf ("Sons will be determinated \n");
  return 0;
}
