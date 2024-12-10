#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdlib.h>

enum {NUM_OF_SEM = 2};
int main(){
  key_t key = ftok("/home/lik/b/1.txt", 'a');
  int sem = semget(key, NUM_OF_SEM, IPC_CREAT | 0666);
  if (sem == -1){
    perror("can't create sem");
    exit(1);
  }
  short vals[] = {1,0}; //открываем семафор
  semctl(sem, 0, SETALL, vals); //устанавливаем значения для семафора значения массива
  
  for (int i = 25; i!=-1; i--){
      semop(sem, (struct sembuf[]){{.sem_op = -1, .sem_num = 1}}, 1); 
      char res = 'a' + i;
      printf("%c", res);fflush(stdout);
      semop(sem, (struct sembuf[]){{.sem_op = +1, .sem_num = 0}}, 1);
  }
  printf("\n");

  return 0;
}

