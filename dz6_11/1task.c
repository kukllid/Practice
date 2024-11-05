#include <stdio.h>
#include <unistd.h>

int main(){
  //execl("/bin/gcc", "gcc", "1.c", "-o", "1.exe", "-lm", (char*)0);
  /*
  
  char * ar[] =  {"gcc", "1.c", "-o", "1.exe", "-lm", (char*)0};
  execv("/bin/gcc", ar); (execvp)
  return 0;
  */
  execlp("/bin/gcc", "gcc", "1.c", "-o", "1.exe", "-lm", (char*)0);
}

