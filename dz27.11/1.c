#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char ** argv){
  
  int f = open(argv[1], O_RDWR);
  int n = atoi(argv[2]);
  int k = 0;
  long long int c = 0;
  while (read(f, &c, sizeof(long long int)) == sizeof(long long int)){
    k++;
  }
  lseek(f, sizeof(long long int) * ((k-n)/2 + ((k-n)%2)), SEEK_SET);
  long long int sum = 0;
  for (int i = 0 ; i < n; i ++){
    read(f, &c, sizeof(long long int));
    sum += c;
  }
  lseek(f, sizeof(long long int) * ((k-n)/2 + ((k-n)%2)), SEEK_SET);
  write(f, &sum, sizeof(long long int));
  close(f);
  
  return 0;
}
