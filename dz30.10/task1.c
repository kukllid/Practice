#include <stdio.h>
#include <stdlib.h>
//структура двунаправленного дерева
struct Node
{
    struct Node *prev, *next;
    int elem;
};

//начало и конец двунаправленного списка
struct List
{
    struct Node *first, *last;
};

//функция для удвоения каждого звена двунаправленного списка
void double_chain(struct List *pl){
  struct Node * cur = pl->first;
  
  while (cur != NULL){
    if (cur->next == NULL){
      struct Node *a = malloc(sizeof(struct Node));
      cur->next = a;
      a->elem = cur->elem;
      a->next = NULL;
      pl->last = a;
      a->prev = cur;
    }else {
      struct Node *a = malloc(sizeof(struct Node));
      a->elem = cur->elem;
      cur->next->prev = a;
      a->next = cur->next;
      cur->next = a;
      a->prev = cur;
    }
    cur = cur->next->next;
  }
  
}
//удалить все нечетные звенья закольцованного двунаправленного списка
void del_chain(struct List *pl){
  struct Node * cur = pl->first;
  //struct Node * end = pl->first->prev;
  struct Node *a;
  int n = 1;
  while (cur != pl->first || n<=2 ){
    a = cur->next;
    if (cur->next == cur && n == 1){
      pl->first = NULL;
      free(cur);
      printf("you deleted the only element \n");
      break;
    }else if (n % 2 == 1){
      if (n == 1){
        pl->first = a;
        cur->prev->next = cur->next;
      }
      cur->prev->next = cur->next; 
      cur->next->prev= cur->prev;
      free(cur);  
      
    }
    n++;
    
    cur = a;
  }
}

int main(){
  //создан пример для проверки 
  struct Node * a = malloc(sizeof(struct Node));
  struct Node * b = malloc(sizeof(struct Node));
  struct Node * c = malloc(sizeof(struct Node));
  struct Node * end = a;
  a->prev = c;
  a->next = b;
  a->elem = 1;
  b->prev = a;
  b->next = c;
  b->elem = 2;
  c->prev = b;
  c->next = a;
  c->elem = 0;
  struct List *pl = malloc(sizeof(struct List));
  pl->first = a;
  pl->last = c;
  //проверка в обе стороны для двунаправленного списка для удвоения звеньев
  /*double_chain(pl);
  a=pl->first;
  while (a) {
    printf("%d\n", a->elem);
    a = a ->next;
  }
  printf("\n");
  
  a = pl->last;
  while (a) {
    printf("%d\n", a->elem);
    a = a ->prev;
  }
  */
  //печать исходного списка
  while (a != NULL && a->next != end){
    printf("%d\n", a->elem);
    a = a->next;
  }
  printf("%d\n \n", a->elem);


  del_chain(pl);
  a = pl->first;
  end = pl->first;
  //проверка что на вывод что-то есть
  if (pl->first == NULL){
    printf("success");
  }else{
  while (a != NULL && a->next != end){
    printf("%d\n", a->elem);
    a = a->next;
  }
  printf("%d\n", a->elem);
  }
  return 0;
}
