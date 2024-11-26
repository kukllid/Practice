#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
int k = 0;

void SIGHNDL1(int s){
  printf ("%d %d\n", s, k);
  k = k + 2;
  fflush(stdout);
}
void SIGHNDL2(int s){
  printf ("%d %d\n", s, k);
  k = k - 3;
  fflush(stdout);
  if (k < 0){
    exit(0);
  }  
}

int main(){
  signal (SIGUSR1, SIGHNDL1);
  signal (SIGUSR2, SIGHNDL2);
  printf("%d\n", getpid());fflush(stdout);
  while (1){
  }
  return 0;
}
