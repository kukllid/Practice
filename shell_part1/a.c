#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
//проверка на то что символ являетя разделителем
int spec_sym(char c){
  return c == '&' || c == '|' || c == ';' || c == '>' || c == '(' || c == ')' || c == '<';
}

//ввод
void create_spis(){
  char c, b;
  unsigned long maln = 10; //символов выделенно под слово
  //цикл для считывания всех символов
  while(scanf("%c", &c) == 1){
    if (isspace(c)) {
      continue;
    }
    int flag_quot = 0;
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
    while(scanf("%c", &c) == 1 && c != '\n' && (flag_quot || (!spec_sym(c) && !isspace(c)))){
      if (c == '"'){
        flag_quot = (flag_quot + 1) % 2;
      } else {
          a[strlen] = c;
          if (strlen - 2 >= maln){
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
      fprintf(stderr, "Incorrect number of \" !");
      exit(1);
    }
    
    a[strlen] = '\0';
    if (strlen != 0) {
        printf("%s\n", a);
    }
    free(a);
    if (spec_sym(c)) {
      if (scanf("%c", &b) == 1){
        if ((b == '&' && '&' == c) || (b == '|' && '|' == c) || (b == '>' && '>' == c)){
          printf("%c%c\n", b, c);
        } else if (b == '\n' || isspace(b)){
          printf("%c\n", c);
        } else {
          ungetc(b, stdin);
          printf("%c\n", c);
        }
      } else {
        printf("%c\n", c);
      }
    }
  }
}

int main(int atgc, char ** argv){
  create_spis();
  return 0;
}

