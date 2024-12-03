#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
enum {NUM_OF_SEM = 2};
int main(){
  int sem = semget(IPC_PRIVATE, NUM_OF_SEM, IPC_CREAT | IPC_EXCL | 0600);
  if (sem == -1){
    perror("can't create sem");
    exit(1);
  }
  short vals[] = {0,1}; //открываем семафор
  semctl(sem, 0, SETALL, vals); //устанавливаем значения для семафора значения массива
  
  pid_t pid1 = fork();
  if (pid1==0){
    int k = 1;
    while(1){
      semop(sem, (struct sembuf[]){{.sem_op = -1, .sem_num = 0}}, 1); //блокируем 1 процесс
      printf("%d %d\n", k, getpid());
      k += 2;
      semop(sem, (struct sembuf[]){{.sem_op = 1, .sem_num = 1}}, 1);
    }
  }
  pid_t pid2 = fork();
  if (pid2==0){
    int k = 0;
    while(1){
      semop(sem, (struct sembuf[]){{.sem_op = -1, .sem_num = 1}}, 1); //блокируем 2 процесс
      printf("%d %d\n", k, getpid());
      k += 2;
      semop(sem, (struct sembuf[]){{.sem_op = 1, .sem_num = 0}}, 1);
    }
  }
  
  sleep(1);
  kill(pid1,SIGKILL);
  kill(pid2,SIGKILL);
  while(wait(NULL)>0){};
  semctl(sem, 0, IPC_RMID); //DELETE
  return 0;
}

