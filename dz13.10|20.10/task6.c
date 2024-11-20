#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>


int c = 0;
void stand(int s){
  c = 1;
  printf("\nHI\n");
  
}
void SIG_DFL_2 (int s){
  printf("\nBabayaka\n");
  exit(0);
}

int main(){
  
  signal(SIGINT, stand);
  while(1){
  if (c == 1) {
    signal(SIGINT, SIG_DFL_2);
    sleep(1);
    signal(SIGINT, stand);
    c = 0;
  }
    
  }
  return 1;
}

