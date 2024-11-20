#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>

int count = 1;
void newreact(){
  printf("BY-BY\n");
  exit(0);
}
void stand(int s){
  count++;
}

int main(){
  signal(SIGQUIT, newreact);
  signal(SIGINT, stand);
  while(1){
    printf("%d\n", count);
  }
  return 1;
}

