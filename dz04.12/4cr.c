#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <sys/wait.h>
int i = 0, p, k;
double x0, s;
FILE * file;
int r = 0;

void sighndlr(int sig){
  double res;
  for (int j = 0; j < k; j++){
    res = cos(((x0 + ((double) j) * s) * (i + 1))/ ((double)p) * M_PI);
    fprintf(file, "%.10g  ", res);
  }
  res = cos((x0 + k * s) * (i + 1)/ p * M_PI);
  fprintf(file, "%.10g\n", res);
  fclose(file);

  _exit(0);
}


int main(int argc, char ** argv){
  
  if (argc < 5){
    printf("not enough arg");
    return 1;
  }
  
  file = fopen(argv[1], "w+");
  p = atoi(argv[2]); 
  x0 = atof(argv[3]);
  s = atof(argv[4]);
  k = atoi(argv[5]);
  pid_t * chld = (pid_t *) malloc(sizeof(pid_t *) * p);
  signal(SIGUSR1, sighndlr);
  
  for (; i < p;i++){
    pid_t pd = fork();
    if (pd == 0){
        pause();
        _exit(0);
    }
    chld[i] = pd;
  }
  
  for (int q = 0; q < p; q++) {
  kill(chld[q], SIGUSR1);
  while(waitpid(chld[q], NULL, 0) > 0){};
}
  
  fclose(file);
  return 0;
}
