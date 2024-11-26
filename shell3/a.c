#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
int background_mode = 0;
void free_words(char **words)
{
    for (int i = 0; words[i] != NULL; i++) {
        free(words[i]);
    }
}
 
// проверка на то что символ являетя разделителем
int spec_sym(char c)
{
    return c == '&' || c == '|' || c == ';' || c == '>' || c == '(' || c == ')' || c == '<';
}
 
int check_for_and(char **words)
{ // проверка на фоновый режим
    for (int i = 0; words[i] != NULL; i++) {
        if (words[i + 1] == NULL && (strcmp(words[i], "&") == 0)) {
            free(words[i]);
            words[i] = NULL;
            return 1;
        }
    }
    return 0;
}
 
int change_input(char **words)
{ // поиск для перенаправления ввода команды
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
            free(words[i]);
            free(words[i + 1]);
            for (int j = i; words[j] != NULL; j++) {
                words[j] = words[j + 2];
            }
            i--;
        }
    }
    return changed;
}
 
int change_output(char **words)
{ // поиск для перенаправление вывода команды
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
        free(words[i]);
        free(words[i + 1]);
        for (int j = i; words[j] != NULL; j++) {
            words[j] = words[j + 2];
        }
        i--;
    }
    return changed;
}
 
int conv(char **words)
{ // конвейер
    int stdoutback = dup(1);
    pid_t pid = fork();
    signal(SIGINT, SIG_IGN);
    if (pid == 0) {
        if (!background_mode) signal(SIGINT, SIG_DFL);
        int flag_first = 0; // перенаправление ввода вывода для первого
        int fd[2];
        int cur_pos = 0; // current position
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
            while (words[cur_pos] != NULL && strcmp(words[cur_pos], "|")) {
                if (cur_l + 4 < l) {
                    l *= 2;
                    small_com = realloc(small_com, l * sizeof(char *));
                }
                small_com[cur_l++] = words[cur_pos++];
            }
            small_com[cur_l] = NULL;

            if (!flag_first++) {
                if (background_mode) {
                  int d = open("/dev/null", O_RDWR);
                  dup2(d, 0);
                  dup2(d, 1);
                  dup2(d, 2);
                }
                change_input(small_com);
                change_output(small_com);
            }
            pid_t pid = fork();
            if (pid == -1) {
                perror("ERROR: exec ");
                _exit(1);
            } else if (pid == 0) {
                if (words[cur_pos] != NULL) {
                    dup2(fd[1], 1);
                } 
                close(fd[1]);
                close(fd[0]);
                execvp(small_com[0], small_com);
                perror("\nERROR: incorrect input ");
                _exit(1);
            }
            flag_first++;
            dup2(fd[0], 0);
            close(fd[0]);
            close(fd[1]);
            free(small_com);
        }
        int status = 0;
        dup2(stdoutback, 1);
        while (wait(&status) != -1) {
            if (background_mode && (!WIFEXITED(status) || WEXITSTATUS(status))) {
              if (WIFEXITED(status)) {
                printf("\nProcess %d exited with code %d\n>", getpid(), WEXITSTATUS(status));
              } else {
                printf("\nProcess %d aborted by signal %d\n>", getpid(), WTERMSIG(status));
              }
              while (waitpid(-1, &status, 0) != -1){};
              _exit(1);
            }
        }
        if (background_mode){
          if (WIFEXITED(status)) {
            printf("\nProcess %d exited with code %d\n>", getpid(), WEXITSTATUS(status));
          } else {
            printf("\nProcess %d aborted by signal %d\n>", getpid(), WTERMSIG(status));
                }
        }
        _exit(0);
    }
    int status = 0;
 
    while (waitpid(pid, &status,0) != -1) {
    };
    signal(SIGINT, SIG_DFL);
    return !WIFEXITED(status) || WEXITSTATUS(status);
}
 
void big_execute(char **words, int n)
{
    words[n] = NULL;
    if (n > 0 && strcmp(words[0], "cd") == 0) {
 
        if (words[1] != NULL) {
            if (chdir(words[1]) != 0) {
                perror("\nERROR: no such directory");
            }
        } else {
            if (chdir(getenv("HOME")) != 0) {
                perror("\nERROR: no way 'HOME'");
            }
        }
      free_words(words);
      return;
    } 
    int cur_pos = 0;
    char cond = 0;
    int prev_sub_com_status = -1;
    if (check_for_and(words)) {
            pid_t pid = fork();
            if (pid < 0) {
                perror("\nERROR: could not fork ");
            } else if (pid == 0) {
                signal(SIGINT, SIG_IGN);
                background_mode = 1;
                while (words[cur_pos] != NULL) {
                    int l = 5;
                    int cur_l = 0;
                    char **sub_com = malloc(sizeof(char *) * l);
                    if (!strcmp(words[cur_pos], "&&")) {
                        cond = '&';
                    }
                    if (!strcmp(words[cur_pos], "||")) {
                        cond = '|';
                    }
                    if (!strcmp(words[cur_pos], ";")) {
                        cond = ';';
                    }
                    if (!strcmp(words[cur_pos], "&&") || !strcmp(words[cur_pos], "||") || !strcmp(words[cur_pos], ";") ) {
                        cur_pos++;
                    }
                    while (words[cur_pos] != NULL && (strcmp(words[cur_pos], "&&") && strcmp(words[cur_pos], "||") && strcmp(words[cur_pos], ";"))) {
                        if (cur_l + 4 < l) {
                        l *= 2;
                        sub_com = realloc(sub_com, l * sizeof(char *));
                    }
                    sub_com[cur_l++] = words[cur_pos++];
                }
            sub_com[cur_l] = NULL;
            if (cond == 0) {
                prev_sub_com_status = conv(sub_com);
            } else if (cond == '&' && prev_sub_com_status == 0) {
                prev_sub_com_status = conv(sub_com);
            } else if (cond == '|' && prev_sub_com_status != 0) {
                prev_sub_com_status = conv(sub_com);
            } else if (cond == ';'){
                 prev_sub_com_status = conv(sub_com);
            } else {
                free(sub_com);
                break;
            }
            free(sub_com);
        }
        _exit(0);
      }
    } else {
        while (words[cur_pos] != NULL) {
            int l = 5;
            int cur_l = 0;
            char **sub_com = malloc(sizeof(char *) * l);
            if (!strcmp(words[cur_pos], "&&")) {
                cond = '&';
            }
            if (!strcmp(words[cur_pos], "||")) {
                cond = '|';
            }
            if (!strcmp(words[cur_pos], ";")) {
                cond = ';';
            }
            if (!strcmp(words[cur_pos], "&&") || !strcmp(words[cur_pos], "||") || !strcmp(words[cur_pos], ";") ) {
                cur_pos++;
            }
            while (words[cur_pos] != NULL && (strcmp(words[cur_pos], "&&") && strcmp(words[cur_pos], "||") && strcmp(words[cur_pos], ";"))) {
                if (cur_l + 4 < l) {
                    l *= 2;
                    sub_com = realloc(sub_com, l * sizeof(char *));
                }
                sub_com[cur_l++] = words[cur_pos++];
            }
            sub_com[cur_l] = NULL;
            if (cond == 0) {
                prev_sub_com_status = conv(sub_com);
            } else if (cond == '&' && prev_sub_com_status == 0) {
                prev_sub_com_status = conv(sub_com);
            } else if (cond == '|' && prev_sub_com_status != 0) {
                prev_sub_com_status = conv(sub_com);
            } else if (cond == ';'){
                 prev_sub_com_status = conv(sub_com);
            } else {
                free(sub_com);
                break;
            }
            free(sub_com);
        }
    }
    free_words(words);
}
 
// ввод
void create_spis(FILE *fp)
{
    char c, b;
    unsigned long words_len = 10;    // allocated numb of words
    unsigned long words_cur_num = 0; // cur num of stored words
    char **words = malloc(sizeof(char *) * words_len);
    printf("> ");
    
    // цикл для считывания всех символов
    while (fscanf(fp, "%c", &c) == 1) {
        waitpid(-1, NULL, WNOHANG); //если есть работающий сын, то не блокируемся, если зомби, то очищаем его
        if (c == '\n' && words_cur_num > 0) {
            big_execute(words, words_cur_num);
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
        while (fscanf(fp, "%c", &c) == 1 && c != '\n' && (flag_quot || (!spec_sym(c) && !isspace(c)))) {
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
            for (int i = 0; i <= words_cur_num; i++) { // print all saved words for the last string
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
            big_execute(words, words_cur_num);
            words_cur_num = 0;
            printf("> ");
        }
 
        if (spec_sym(c)) {
            if (fscanf(fp, "%c", &b) == 1) {
                if ((b == '&' && '&' == c) || (b == '|' && '|' == c) || (b == '>' && '>' == c)) {
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
 
        if (words_len - 5 < words_cur_num) { // if allocated numb of words is not enough
            words_len *= 3;
            words = realloc(words, sizeof(char *) * words_len);
        }
    }
    big_execute(words, words_cur_num);
    free(words);
}
 
int main(int argc, char **argv)
{
    setbuf(stdout, 0);
    FILE *fp = stdin;
    if (argc > 1) {
        fp = fopen(argv[1], "r");
        if (fp == NULL) {
            fprintf(stderr, "No such file");
        }
    }
 
    create_spis(fp);
    if (fp != stdin) {
        fclose(fp);
    }
    return 0;
}
