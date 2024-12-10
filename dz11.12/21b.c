#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <unistd.h>

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

  while(1){
      msgrcv(msgid, (struct msgbuf *)(&message), 255, 0, 0);
      if (message.mtype == 3) {
        message.mtype = 1;
        msgsnd(msgid, (struct msgbuf *)(&message), 255, 0);
        exit(0);
      }
      printf("%s", message.data);

  }
   
  return 0;
}

