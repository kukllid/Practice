#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
struct {
  long mtype;
  char data[255];
} message;

int main(){
  key_t key;
  int msgid;
  key = ftok("/home/lik/b/1.txt", 'a');
  if (key == -1){
    perror("ERROR");
    exit(1);
  }
  msgid = msgget(key, IPC_CREAT|0666);
  int f = open("myshell.c", O_RDWR);
  
  int len=0;
  while((len = read(f, message.data, sizeof(char)*254))!=0){
      message.data[len] = '\0';
      message.mtype = 2;
      msgsnd(msgid, (struct msgbuf *)(&message), 255, 0);
  }
  message.mtype = 3;
  msgsnd(msgid, (struct msgbuf *)(&message), 255, 0);
  msgrcv(msgid, (struct msgbuf *)(&message), 255, 1, 0);
  msgctl(msgid,IPC_RMID, NULL); //DELETE
  return 0;
}

