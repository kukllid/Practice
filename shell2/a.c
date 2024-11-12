#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

//проверка на то что символ являетя разделителем
int spec_sym(char c){
  return c == '&' || c == '|' || c == ';' || c == '>' || c == '(' || c == ')' || c == '<';
}

void execute(char ** words, int n){
    if (n == 0){
      return;
    }
    words[n] = (char*) 0;
    if(strcmp(words[0],"cd")==0){
      if (n > 1){
        if (chdir(words[1]) != 0){
          perror("ERROR: no such directory");
        }
      }
      else {
        if(chdir(getenv("HOME")) != 0){
          perror("ERROR: no way 'HOME'");
        }
      }
    }
    else {
      pid_t pid; 

      pid = fork();
      if (pid == -1) { 
          perror("Error: Failed to create a new process"); 
          exit(1); 
      } else if(pid == 0) { 
          // son
          if (execvp(words[0], words) == -1) { 
              perror("Error: execution failed"); 
              exit(1); 
          } 
      } else { 
          //parent
          int status;
          waitpid(pid, &status, 0); 
          if (WIFEXITED(status) == 0){
            fprintf(stderr, "PID %d processed %d", pid, WTERMSIG(status));
          }
      }
    }
}

//ввод
void create_spis(FILE *fp){
  char c, b;
  unsigned long words_len = 10;//allocated numb of words
  unsigned long words_cur_num = 0;// cur num of stored words
  char **words = malloc(sizeof(char*) * words_len);
  
	printf("> ");

  //цикл для считывания всех символов
  while(fscanf(fp, "%c", &c) == 1){
	
    if (c == '\n' && words_cur_num > 0){
      execute(words, words_cur_num);
      for (int i = 0; i <= words_cur_num; i++) {
        free(words[i]);
      }
      words_cur_num = 0;
      printf("> ");
    }
    
    if (isspace(c)) {
      continue;
    }
    int flag_quot = 0;
    unsigned long maln = 10; //символов выделенно под слово
    char * a = malloc(sizeof(char) * maln); 
    if (a == NULL){
      printf("Memory error by malloc\n");
    }
    
    int strlen = 0;
    if (spec_sym(c)) {
        ungetc(c, stdin);
    } else {
      if (c == '"'){
        flag_quot = (flag_quot + 1) % 2;
      } else {
          a[strlen++] = c;
      }
    }
    //обработка слова
    while(fscanf(fp, "%c", &c) == 1 && c != '\n' && (flag_quot || (!spec_sym(c) && !isspace(c)))){
      if (c == '"'){
        flag_quot = (flag_quot + 1) % 2;
      } else {
          a[strlen] = c;
          if (strlen >= maln - 2){
            maln *= 3;
            a = realloc(a, sizeof(char) * maln);
            if (a == NULL){
              printf("Memory error by realloc\n");
              exit(1);
            }
          }
          strlen++;
        }
    }
    
    if (flag_quot == 1) {
      fprintf(stderr, "Incorrect number of \" ! \n");
       for (int i = 0; i <= words_cur_num; i++) {//print all saved words for the last string
	      if (i == 0){
		      printf("\n");
	      }
	      printf("%s\n", words[i]);
	      free(words[i]);
      continue;
      words_cur_num = 0;
      }
    }
    
    a[strlen] = '\0';
    if (strlen != 0) {//if len of str = 0, there is no need to store it
        words[words_cur_num++] = a;
    } else {
      free(a);
    }
    
    if (c == '\n') {//if new string: print all saved words of previous string
      execute(words, words_cur_num);
      for (int i = 0; i < words_cur_num; i++) {
        free(words[i]);
      }
      words_cur_num = 0;
      printf("> ");
    }
    
    if (spec_sym(c)) {
      if (fscanf(fp, "%c", &b) == 1){
        if ((b == '&' && '&' == c) || (b == '|' && '|' == c) || (b == '>' && '>' == c)){
          char *tmp = malloc(sizeof(char) * 3);
          tmp[0] = b;
          tmp[1] = c;
          tmp[2] = '\0';
          words[words_cur_num++] = tmp;
        } else {
          ungetc(b, stdin);
          char *tmp = malloc(sizeof(char) * 2);
          tmp[0] = c;
          tmp[1] = '\0';
          words[words_cur_num++] = tmp;
        }
      } else {
        char *tmp = malloc(sizeof(char) * 2);
        tmp[0] = c;
        tmp[1] = '\0';
        words[words_cur_num++] = tmp;
      }
    }
    
    if (words_len - 5 < words_cur_num) {//if allocated numb of words is not enough
      words_len *= 3;
      words = realloc(words, sizeof(char*) * words_len);
    }
    
  }
  execute(words, words_cur_num);
  for (int i = 0; i < words_cur_num; i++) {//print all saved words for the last string
    free(words[i]);
    
  }
  free(words);
}

int main(int argc, char ** argv){
  FILE * fp = stdin;
  if (argc > 1){
	  fp = fopen(argv[1], "r");
	  if (fp == NULL){
		fprintf(stderr, "No such file");
	  }
  }
  create_spis(fp);
  return 0;
}

