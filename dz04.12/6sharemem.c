#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

int main(){
  key_t key;
  key = ftok("/home/lik/b/1.txt", 'a');
  if (key == -1){
    perror("ERROR");
    exit(1);
  }
  int shmid = shmget(key, sizeof(int), IPC_CREAT|IPC_EXCL|0666);
  int * shmaddr = shmat(shmid, NULL, 0);
  shmaddr[0] = 1; 
  pid_t pid1 = fork();
  if (pid1==0){
    int k = 0;
    while(1){
      if (shmaddr[0] == 1){
        printf("%d %d\n", k, getpid());
        k += 2;
        shmaddr[0] = 2; 
      }
    }
    _exit(1);
  }
  
  pid_t pid2 = fork();
  if (pid2==0){
    int k = 1;
     while(1){
      if (shmaddr[0] == 2){
        printf("%d %d\n", k, getpid());
        k += 2;
        shmaddr[0] = 1; 
      }
    }
    _exit(1);
  }
  
  usleep(10000);
  kill(pid1,SIGKILL);
  kill(pid2,SIGKILL);
  while(wait(NULL)>0){};
  shmctl(shmid,IPC_RMID, NULL); //DELETE
  return 0;
}

