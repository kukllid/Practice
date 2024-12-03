#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
struct {
  long mtype;
  char data[8];
} message;

int main(){
  key_t key;
  int msgid;
  key = ftok("/home/lik/b/1.txt", 'a');
  if (key == -1){
    perror("ERROR");
    exit(1);
  }
  msgid = msgget(key, IPC_CREAT|IPC_EXCL|0666);
  pid_t pid1 = fork();
  if (pid1==0){
    int k = 0;
    msgid = msgget(key, 0666);
    while(1){
      msgrcv(msgid, (struct msgbuf *)(&message), 1, 1, 0);
      printf("%d %d\n", k, getpid());
      k += 2;
      message.data[0] = 0;
      message.mtype = 2;
      msgsnd(msgid, (struct msgbuf *)(&message), 1, 0);
    }
    _exit(1);
  }
  pid_t pid2 = fork();
  if (pid2==0){
    int k = 1;
    msgid = msgget(key, 0666);
    while(1){
      msgrcv(msgid, (struct msgbuf *)(&message), 1, 2, 0);
      printf("%d %d\n", k, getpid());
      k += 2;
      message.data[0] = 0;
      message.mtype = 1;
      msgsnd(msgid, (struct msgbuf *)(&message), 1, 0);
    }
    _exit(1);
  }
  message.data[0] = 0;
  message.mtype = 1;
  msgsnd(msgid, (struct msgbuf *)(&message), 1, 0);
  sleep(1);
  kill(pid1,SIGKILL);
  kill(pid2,SIGKILL);
  while(wait(NULL)>0){};
  msgctl(msgid,IPC_RMID, NULL); //DELETE
  return 0;
}

