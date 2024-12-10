#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
int background_mode = 0;
int stdoutf;
void free_words(char **words) {
  for (int i = 0; words[i] != NULL; i++) {

    free(words[i]);
  }
}

// проверка на то что символ являетя разделителем
int spec_sym(char c) {
  return c == '&' || c == '|' || c == ';' || c == '>' || c == '(' || c == ')' ||
         c == '<';
}

int check_for_and(char **words) { // проверка на фоновый режим
  for (int i = 0; words[i] != NULL; i++) {
    if (words[i + 1] == NULL && (strcmp(words[i], "&") == 0)) {
      words[i] = NULL;
      return 1;
    }
  }
  return 0;
}

int change_input(char **words) { // поиск для перенаправления ввода команды
  int fd;
  int changed = 0;
  for (int i = 0; words[i] != NULL; i++) {
    if (strcmp(words[i], "<") == 0) {
      fd = open(words[i + 1], O_RDONLY);
      if (fd == -1) {
        perror("\nERROR: no such file ");
        return -1; // ошибка перенаправления файла
      }
      dup2(fd, 0);
      close(fd);
      changed = 1;
      for (int j = i; words[j] != NULL; j++) {
        words[j] = words[j + 2];
      }
      i--;
    }
  }
  return changed;
}

int change_output(char **words) { // поиск для перенаправление вывода команды
  int fd;
  int changed = 0;

  for (int i = 0; words[i] != NULL; i++) {
    if (strcmp(words[i], ">") == 0) {
      fd = open(words[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
    } else if (strcmp(words[i], ">>") == 0) {
      fd = open(words[i + 1], O_WRONLY | O_CREAT | O_APPEND, 0666);
    } else {
      continue;
    }
    if (fd == -1) {
      perror("\nERROR: can not open or create file ");
      return 1;
    }
    dup2(fd, 1);
    close(fd);
    changed = 1;
    for (int j = i; words[j] != NULL; j++) {
      words[j] = words[j + 2];
    }
    i--;
  }

  return changed;
}

int logic(char **words);
int bacground(char **words);

int conv(char **words) { // конвейер
  signal(SIGINT, SIG_IGN);
  int stdinback = dup(0);

  int flag_first = 0; // перенаправление ввода вывода для первого
  int fd[2];
  int cur_pos = 0; // current position
  int skob = 0;
  pid_t pids[100];
  int cur_pid = 0;

  while (words[cur_pos] != NULL) {
    int l = 10;
    int cur_l = 0;
    char **small_com = malloc(sizeof(char *) * l);
    if (pipe(fd)) {
      perror("\nERROR: Pipe ");
      _exit(1);
    }
    if (!strcmp(words[cur_pos], "|")) {
      cur_pos++;
    }

    // разделяем конвейер на команды
     while (words[cur_pos] != NULL  && (strcmp(words[cur_pos], "|") || skob )) {
      if (cur_l + 4 < l) {
        l *= 2;
        small_com = realloc(small_com, l * sizeof(char *));
      }
      if (!strcmp(words[cur_pos], "(")) {
        skob++;
      }
      if (!strcmp(words[cur_pos], ")")) {
        skob--;
      }
      small_com[cur_l++] = words[cur_pos++];
    }

    small_com[cur_l] = NULL;
    
    if (!flag_first++) {
      change_input(small_com);
      change_output(small_com);
    }
    
    pid_t pid = fork();
    if (pid == -1) {
      perror("ERROR: exec ");
      _exit(1);
    } else if (pid == 0) {
      if (!background_mode){
        signal(SIGINT, SIG_DFL);
      }
      
      if (words[cur_pos] != NULL) {
        dup2(fd[1], 1);
      }
      
      int flagback = 0;
      int flaglogic = 0;
      for (int i = 0; small_com[i]; i++) {
        if (!strcmp(small_com[i], ";") || !strcmp(small_com[i], "&")) {
          flagback++;
        }
        if (!strcmp(small_com[i], "||") || !strcmp(small_com[i], "&&") || !strcmp(small_com[i], "(") || !strcmp(small_com[i], ")")) {
          flaglogic++;
        }
      }

      if (flagback) {
        int x = bacground(small_com);
        _exit(x);
      } else if (flaglogic){
        int x = logic(small_com);
        _exit(x);
      }else{
        execvp(small_com[0], small_com);
        perror("\nERROR: incorrect input ");
        _exit(1);
      }
      
    }
    pids[cur_pid++] = pid;
    flag_first++;
    dup2(fd[0], 0);
    close(fd[0]);
    close(fd[1]);
    free(small_com);
  }
  dup2(stdinback, 0);
  int status = 0;
  int ret_value;
  for (int i = 0; i < cur_pid; i++) {
    waitpid(pids[i], &status, 0);
    if (!WIFEXITED(status) || WEXITSTATUS(status)) {
      if (WIFEXITED(status)) {
          ret_value =  WIFEXITED(status);
      } else {
           ret_value =  (1u << 16) | WTERMSIG(status);
      }
      for (int j = i; j < cur_pid; j++) {
        waitpid(pids[j], &status, 0);
      }
      return ret_value;
    }
  }

  return 0;
}

int logic(char **words) {
  int cur_pos = 0;
  char cond = 0;
  int prev_sub_com_status = -1;
  int skob = 0;
  while (words[cur_pos] != NULL) {
    int skob_obnul = 0;
    int skob_obnul_n = 0;
    int l = 10;
    int cur_l = 0;
    char **sub_com = malloc(sizeof(char *) * l);
    if (!strcmp(words[cur_pos], "&&")) {
      cond = '&';
      cur_pos++;
    }
    if (!strcmp(words[cur_pos], "||")) {
      cond = '|';
      cur_pos++;
    }
    if (!strcmp(words[cur_pos], "(")) {
    skob++;
      sub_com[cur_l++] = words[cur_pos++];
    }
    if (!strcmp(words[cur_pos], ")")) {
      skob--;
      sub_com[cur_l++] = words[cur_pos++];
    }
    int prev_skob = skob;
    while (words[cur_pos] != NULL  && ((strcmp(words[cur_pos], "&&") &&
             strcmp(words[cur_pos], "||")) || skob )) {
             
      skob_obnul = 0;
      if (cur_l + 4 < l) {
        l *= 2;
        sub_com = realloc(sub_com, l * sizeof(char *));
      }
      if (!strcmp(words[cur_pos], "(")) {
        skob++;
      }
      
      if (!strcmp(words[cur_pos], ")")) {
        skob--;
        if (prev_skob && !skob) {
          skob_obnul_n++;
          skob_obnul = 1;
          }
      }
      sub_com[cur_l++] = words[cur_pos++];
       
    }
    sub_com[cur_l] = NULL;
    
    if (skob_obnul == 1 && skob_obnul_n == 1) {
      sub_com[cur_l - 1] = NULL;
      for (int  i= 1;sub_com[i];i++) {
          sub_com[i - 1] = sub_com[i];
      }
      sub_com[cur_l - 2] = NULL;
    }
   
    
    if (cond == 0) {
      prev_sub_com_status = conv(sub_com);
    } else if (cond == '&' && prev_sub_com_status == 0) {
      prev_sub_com_status = conv(sub_com);
    } else if (cond == '|' && prev_sub_com_status != 0) {
      prev_sub_com_status = conv(sub_com);
    } 
    free(sub_com);
  }
  return prev_sub_com_status;
}

int  bacground(char **words) {
  int cur_pos = 0;
  int retval = 0;
  
  int skob = 0;
  while (words[cur_pos] != NULL) {;
    int l = 10;
    int cur_l = 0;
    char **sub_com = malloc(sizeof(char *) * l);

    while (words[cur_pos] != NULL  && ((strcmp(words[cur_pos], ";") &&
             strcmp(words[cur_pos], "&")) || skob )) {
      if (cur_l + 4 < l) {
        l *= 2;
        sub_com = realloc(sub_com, l * sizeof(char *));
      }
      if (!strcmp(words[cur_pos], "(")) {
        skob++;
      }
      if (!strcmp(words[cur_pos], ")")) {
        skob--;
      }
      sub_com[cur_l++] = words[cur_pos++];
       
    }
    sub_com[cur_l] = NULL;
    background_mode = 0;
    if (words[cur_pos] && !strcmp(words[cur_pos], "&")) {
      cur_pos++;
      pid_t pid = fork();
      if (pid == 0) {
        background_mode = 1;
        signal(SIGINT, SIG_IGN);
        int d = open("/dev/null", O_RDWR);
        dup2(d, 0);
        dup2(d, 1);
        dup2(d, 2);
        close(d);
        int ret_value = logic(sub_com);
        while(wait(NULL) != -1){};
        dup2(stdoutf, 1);
        if (!(ret_value >> 16)) {
          printf("\nProcess %d exited with code %d\n>", getpid(), ret_value&((1<<16)-1));
        } else {
          printf("\nProcess %d aborted by signal %d\n>", getpid(), ret_value&((1<<16)-1));
        }
        exit(ret_value);
      }
    }else if (cur_l) {
      if (words[cur_pos] && !strcmp(words[cur_pos], ";")) {
        cur_pos++;
       }
       retval = logic(sub_com);
   }
    free(sub_com);
  }
  return retval;
}


void command_run(char **words) {
    int stdoutback = dup(1);
    int stdinback = dup(0);
    int i = 0;
    for (; words[i];i++);
    words[i] = NULL;
    if (words[0] && strcmp(words[0], "cd") == 0) {
 
        if (words[1] != NULL) {
            if (chdir(words[1]) != 0) {
                perror("\nERROR: no such directory");
            }
        } else {
            if (chdir(getenv("HOME")) != 0) {
                perror("\nERROR: no way 'HOME'");
            }
        }
      return;
    } 
    bacground(words);
    dup2(stdoutback, 1);
    dup2(stdinback, 0);
}
// ввод
void create_spis(FILE *fp) {
  char c, b;
  unsigned long words_len = 10;    // allocated numb of words
  unsigned long words_cur_num = 0; // cur num of stored words
  char **words = malloc(sizeof(char *) * words_len);
  printf("> ");

  // цикл для считывания всех символов
  while (fscanf(fp, "%c", &c) == 1) {
    waitpid(-1, NULL, WNOHANG); // если есть работающий сын, то не блокируемся,
                                // если зомби, то очищаем его
    if (c == '\n' && words_cur_num > 0) {
      words[words_cur_num] = NULL;
      command_run(words);
      free_words(words);
      words_cur_num = 0;
      printf("> ");
    } else if (c == '\n') {
      printf("> ");
    }

    if (isspace(c)) {
      continue;
    }
    int flag_quot = 0;
    unsigned long maln = 10; // символов выделенно под слово
    char *a = malloc(sizeof(char) * maln);
    if (a == NULL) {
      perror("\nERROR: Memory error by malloc ");
    }

    int strlen = 0;
    if (spec_sym(c)) {
      ungetc(c, stdin);
    } else {
      if (c == '"') {
        flag_quot = (flag_quot + 1) % 2;
      } else {
        a[strlen++] = c;
      }
    }
    // обработка слова
    while (fscanf(fp, "%c", &c) == 1 && c != '\n' &&
           (flag_quot || (!spec_sym(c) && !isspace(c)))) {
      if (c == '"') {
        flag_quot = (flag_quot + 1) % 2;
      } else {
        a[strlen] = c;
        if (strlen >= maln - 2) {
          maln *= 3;
          a = realloc(a, sizeof(char) * maln);
          if (a == NULL) {
            perror("\nERROR: Memory error by realloc\n");
            exit(1);
          }
        }
        strlen++;
      }
    }

    if (flag_quot == 1) {
      perror("\nError: Incorrect number of \" ! \n");
      for (int i = 0; i <= words_cur_num;
           i++) { // print all saved words for the last string
        if (i == 0) {
          printf("\n");
        }
        printf("%s\n", words[i]);
        free(words[i]);
        continue;
        words_cur_num = 0;
      }
    }

    a[strlen] = '\0';
    if (strlen != 0) { // if len of str = 0, there is no need to store it
      words[words_cur_num++] = a;
    } else {
      free(a);
    }

    if (c == '\n') { // if new string: print all saved words of previous string
      words[words_cur_num] = NULL;
      command_run(words);
      free_words(words);
      words_cur_num = 0;
      printf("> ");
    }

    if (spec_sym(c)) {
      if (fscanf(fp, "%c", &b) == 1) {
        if ((b == '&' && '&' == c) || (b == '|' && '|' == c) ||
            (b == '>' && '>' == c)) {
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

    if (words_len - 5 <
        words_cur_num) { // if allocated numb of words is not enough
      words_len *= 3;
      words = realloc(words, sizeof(char *) * words_len);
    }
  }
  words[words_cur_num] = NULL;
  command_run(words);
  free_words(words);
  free(words);
}

int main(int argc, char **argv) {
  stdoutf = dup(1);
  setbuf(stdout, 0);
  FILE *fp = stdin;

  if (argc > 1) {
    fp = fopen(argv[1], "r");
    if (fp == NULL) {
      fprintf(stderr, "No such file");
    }
  }
  signal(SIGINT, SIG_IGN);
  create_spis(fp);
  while (wait(NULL) > 0) {
  };
  if (fp != stdin) {
    fclose(fp);
  }
  return 0;
}
