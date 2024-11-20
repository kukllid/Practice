#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>


int c = 1;
int k = 0;
void stand(int s){
  c ++; 
  if (c == 7){
    printf("\n%d\n", k);
    exit(0);
  }
}

void stand2(){
  if (c >= 2 && c <= 4){
    k++;
  }
}

int main(){
  signal(SIGQUIT, stand2); //SIGTRAP?
  signal(SIGINT, stand);
  while(1){
  
  }
    
  
  return 0;
}

