#include <iostream>  // Для std::cout, std::cerr, std::endl
#include <string>    // Для std::string, std::to_string
#include <vector>    // Для std::vector (TID)
#include <utility>   // Для std::move
#include <cstdio>    // Для FILE, fopen, fgetc, ungetc, fclose, EOF
#include <cctype>    // Для isalpha, isdigit, isalnum
#include <stdexcept> // Для std::runtime_error
#include <algorithm> // Для std::find, std::distance

// Перечисление типов лексем
enum type_of_lex {
    LEX_NULL,    
    // Ключевые слова (индексы 1-16)
    LEX_IF, LEX_AND, LEX_ELSE, LEX_REAL, LEX_STRING, LEX_DO, LEX_GOTO, LEX_FALSE, LEX_INT,
    LEX_NOT, LEX_OR, LEX_PROGRAM, LEX_READ, LEX_TRUE, LEX_BREAK, LEX_WRITE, LEX_WHILE,
    // Разделители (индексы с 17)
    LEX_FIN,        // Конец файла (17)
    LEX_SEMICOLON, LEX_COMMA, LEX_COLON, LEX_ASSIGN, LEX_LPAREN, LEX_RPAREN, LEX_EQ, LEX_LSS,
    LEX_GTR, LEX_PLUS, LEX_MINUS, LEX_TIMES, LEX_SLASH, LEX_LEQ, LEX_NEQ, LEX_GEQ,
    LEX_LFIG, LEX_RFIG, // Фигурные скобки (33, 34)
    // Литералы и идентификаторы (индексы с 36)
    LEX_NUM,        // Числовой литерал (36)
    LEX_STR,        // Строковый литерал (37)
    LEX_ID,         // Идентификатор (38)
    // Типы для POLIZ
    POLIZ_LABEL, POLIZ_ADDRESS, POLIZ_GO, POLIZ_FGO
};

// Класс для представления одной лексемы
class Lex {
    type_of_lex t_lex; // Тип лексемы
    int v_lex;         // Значение (индекс для ID, число для NUM, счетчик для STR)
    int line_lex;      // Номер строки, где найдена лексема
    std::string s_lex; // Строковое значение (для LEX_STR)
public:
    // Конструктор
    explicit Lex ( type_of_lex t = LEX_NULL, int v = 0, int line = 0, std::string str = ""):
        t_lex (t), v_lex (v), line_lex(line), s_lex(std::move(str)) { }

    // Геттеры
    type_of_lex get_type () const { return t_lex; }
    int get_value () const { return v_lex; }
    int get_line() const { return line_lex; }
    std::string get_str() const { return s_lex; }

    friend std::ostream &operator<<( std::ostream &s, const Lex& l );
};

// Класс для представления идентификатора в таблице
class Ident {
    std::string name;     // Имя идентификатора
    bool declare;         // Флаг объявления
    type_of_lex type;     // Тип (при семантическом анализе)
    bool assign;          // Флаг присваивания значения
    int value;            // Значение (при интерпретации/выполнении)
public:
    // Конструкторы
    Ident() : declare(false), type(LEX_NULL), assign(false), value(0) {}
    explicit Ident(std::string n) : name(std::move(n)), declare(false), type(LEX_NULL), assign(false), value(0) {}

    // Оператор сравнения для поиска по имени
    bool operator==(const std::string &s) const { return name == s; }

    std::string get_name() const { return name; }
    bool get_declare() const { return declare; }
    void put_declare() { declare = true; }
    type_of_lex get_type() const { return type;}
    void put_type(type_of_lex t) { type = t; }
    bool get_assign() const { return assign; }
    void put_assign() { assign = true; }
    int get_value() const { return value; }
    void put_value(int v) { value = v; }
};

std::vector<Ident> TID; // Таблица идентификаторов

// Вспомогательные функции 

// Добавляет идентификатор в TID, если его там нет, и возвращает его индекс
int put ( const std::string &buf ) {
    auto it = std::find(TID.begin(), TID.end(), buf);
    if (it != TID.end()) {
        // Найден, возвращаем индекс
        return static_cast<int>(std::distance(TID.begin(), it));
    } else {
        // Не найден, добавляем
        TID.emplace_back(buf);
        // Возвращаем индекс нового элемента
        return static_cast<int>(TID.size() - 1);
    }
}

// Класс Лексического Анализатора (Scanner)
class Scanner {
    FILE* fp;                   
    int current_line_number;    // Текущий номер строки (начинается с 1)
    int current_char;           // Текущий символ 

    // Вспомогательная функция: поиск строки в массиве
    static int look (const std::string& buf, const char **list) {
        int i = 1; // Индексация с 1 для TW и TD
        while (list[i]) {
            if (buf == list[i])
                return i;
            ++i;
        }
        return 0; // Не найдено
    }

    // Работа с файлом
    void gc () {
        current_char = fgetc(fp); 
    }

    void unget(int c) {
        if (c != EOF) {
            ungetc(c, fp);
        }
         // Если c == EOF, ничего не делаем, т.к. это конец файла или ошибка чтения
    }

    // Вспомогательная функция для форматирования сообщения об ошибке
    std::string format_error_message(const std::string& msg) {
        return "Lexical Error on line " + std::to_string(current_line_number) + ": " + msg;
    }

public:
    // Таблицы ключевых слов и разделителей (статические)
    static const char *TW[]; // Ключевые слова
    static const char *TD[]; // Разделители

    // Конструктор: открывает файл
    explicit Scanner (const char* filename) : current_line_number(1), current_char(' ') {
        fp = fopen(filename, "r"); // Открываем файл для чтения
        if (fp == nullptr) {
            // Ошибка открытия файла, бросаем исключение
            throw std::runtime_error("Error: Can't open file " + std::string(filename));
        }
    }

    // Деструктор: закрывает файл
    ~Scanner() {
        if (fp != nullptr) {
            fclose(fp);
        }
    }

    // Запрещаем копирование и присваивание, т.к. управляем ресурсом (FILE*)
    Scanner(const Scanner&) = delete;
    Scanner& operator=(const Scanner&) = delete;

    // Метод получение следующей лексемы
    Lex get_lex ();
};

// Определение статических таблиц вне класса
const char *Scanner::TW[] = {
    nullptr, // 0
    "if", "and", "real", "string", "do", "false", "int", "var", // 1-8
    "not", "or", "else", "program", "read", "true", "break", "write", "while", // 9-16
    nullptr // Маркер конца
};

const char *Scanner::TD[] = {
    nullptr, // 0: 
    ";", ",", ":", "=", "(", ")", "==", "<", ">", "+", "-", "*", "/", "<=", "!=", ">=", // 1-16 
    "{", "}", // 17, 18
    nullptr 
};


// Реализация метода Scanner::get_lex 
Lex Scanner::get_lex () {
    enum state { H, IDENT, NUMB, COM, ALE, STR }; // Состояния автомата
    state CS = H;                                // Начальное состояние
    //Изначальное состояние H 
    //IDENT - состояние идентификатора
    //NUMB - состояние числа
    //COM - состояние /* ... */
    //ALE - asign less equal
    //STR - состояние " ... "
    std::string buf;      // Буфер для накопления символов
    int num_value = 0;    // Для значения числа
    static int str_count = 0; // Счетчик строк (как в оригинале)

    while (true) { // Цикл до возврата лексемы или ошибки
        switch (CS) {
            // --- Состояние H: Начальное / Пропуск пробелов ---
            case H:
                if (current_char == ' ' || current_char == '\t' || current_char == '\r') {
                    gc(); // Пропускаем пробельные символы (кроме \n)
                } else if (current_char == '\n') {
                    current_line_number++; // Увеличиваем номер строки
                    gc();                  // Читаем следующий символ
                } else if (isalpha(current_char)) {
                    // Начинается идентификатор/ключевое слово
                    buf.clear();
                    buf += static_cast<char>(current_char);
                    CS = IDENT;
                    gc();
                } else if (isdigit(current_char)) {
                    // Начинается число
                    num_value = current_char - '0';
                    CS = NUMB;
                    gc();
                } else if (current_char == '"') {
                    // Начинается строка
                    buf.clear();
                    CS = STR;
                    gc();
                } else if (current_char == '/') {
                    // Комментарий или деление - надо выяснить
                    gc(); // Смотрим следующий символ
                    if (current_char == '*') {
                        // Это комментарий /*
                        CS = COM;
                        gc(); // Пропускаем '*'
                    } else {
                        // Это деление '/'
                        unget(current_char); // Возвращаем символ, который не '*'
                        // '/'(деление) ищем в TD
                        int j = look("/", TD); 
                        if (j > 0) {
                             return Lex(static_cast<type_of_lex>(j + LEX_FIN), j, current_line_number);
                         } else {
                             throw std::runtime_error(format_error_message("Internal: '/' not found in delimiters"));
                         }
                    }
                } else if (current_char == '=' || current_char == '<' || current_char == '>' || current_char == '!') {
                    // Операторы =, <, >, ! (возможно, двухсимвольные)
                    buf.clear();
                    buf += static_cast<char>(current_char);
                    CS = ALE;
                    gc();
                } else if (current_char == EOF) {
                    return Lex(LEX_FIN, 0, current_line_number);
                } else {
                    // Другие одиночные разделители
                    buf.clear();
                    buf += static_cast<char>(current_char);
                    int j = look(buf, TD); // Ищем в TD
                    if (j > 0) {
                        // Найден разделитель
                        gc(); // Читаем следующий символ для следующего вызова
                        // Вычисляем тип лексемы и возвращаем её
                        //Сложение j + LEX_FIN преобразует 1-based индекс из TD в соответствующее значение enum для этого разделителя
                        return Lex(static_cast<type_of_lex>(j + LEX_FIN), j, current_line_number);
                    } else {
                        // Неизвестный символ
                        throw std::runtime_error(format_error_message("Unexpected character '" + buf + "'"));
                    }
                }
                break; 

            // Состояние IDENT - чтение идентификатора / программного слова
            case IDENT:
                if (isalnum(current_char)) { // Буква или цифра
                    buf += static_cast<char>(current_char);
                    gc();
                } else {
                    // Идентификатор/слово закончилось. current_char не принадлежит ему
                    int j_kw = look(buf, TW); // Ищем в ключевых словах
                    if (j_kw > 0) {
                        // Это ключевое слово
                        return Lex(static_cast<type_of_lex>(j_kw), j_kw, current_line_number);
                    } else {
                        // Это идентификатор
                        int index_id = put(buf); // Добавляем/находим в TID
                        return Lex(LEX_ID, index_id, current_line_number);
                    }
                }
                break; // Конец case IDENT

            // Состояние NUMB - чтение числа
            case NUMB:
                if (isdigit(current_char)) {
                    // Продолжаем читать цифры
                    // TODO: Добавить проверку на переполнение, если необходимо
                    num_value = num_value * 10 + (current_char - '0');
                    gc();
                } else if (isalpha(current_char)) {
                     // Ошибка: Буква после цифр
                     std::string error_char(1, static_cast<char>(current_char));
                     throw std::runtime_error(format_error_message("Invalid number format: character '" + error_char + "' after digits"));
                }
                 else {
                    // Число закончилось. current_char не принадлежит ему.
                    // Не нужно unget().
                    return Lex(LEX_NUM, num_value, current_line_number);
                }
                break; // Конец case NUMB

            // Состояние STR - чтение строкового литерала 
            case STR:
                 if (current_char == '"') {
                     // Конец строки
                     gc(); // Пропускаем закрывающую кавычку
                     str_count++; // Счетчик строк (если нужен)
                     return Lex(LEX_STR, str_count, current_line_number, buf);
                 } else if (current_char == '\n') {
                     // Ошибка - перенос строки внутри строкового литерала - считаем ошибкой, как и EOF
                     throw std::runtime_error(format_error_message("Unterminated string literal"));
                 } else if (current_char == EOF) {
                     // Ошибка - строка не закрыта до конца файла
                     throw std::runtime_error(format_error_message("Unterminated string literal"));
                 } else if (current_char == '\\') {
                     // Обработка экранирования
                     gc(); // Читаем символ после '\'
                     if (current_char == '"') {
                         buf += '"'; gc();
                     } else if (current_char == '\\') {
                         buf += '\\'; gc();
                     } else if (current_char == 'n') {
                         buf += '\n'; gc();
                     } else if (current_char == 't') {
                         buf += '\t'; gc();
                     } else if (current_char == EOF) {
                          throw std::runtime_error(format_error_message("Unterminated string literal after escape character (EOF)"));
                     } else {
                         // Неизвестная последовательность, можно считать ошибкой или просто добавить символ
                         // Добавим сам символ, как если бы '\' не было 
                         // buf += '\\'; // Оставить \ как есть
                         buf += static_cast<char>(current_char); // Добавить сам символ
                         gc();
                     }
                 } else {
                     // Обычный символ внутри строки
                     buf += static_cast<char>(current_char);
                     gc();
                 }
                 break; // Конец case STR

            // Состояние ALE: Операторы =, <, >, !
            case ALE:
                if (current_char == '=') {
                    // Двухсимвольный оператор: ==, <=, >=, !=
                    buf += '=';
                    int j_op = look(buf, TD);
                    if (j_op > 0) {
                        gc(); // Пропускаем '='
                        return Lex(static_cast<type_of_lex>(j_op + LEX_FIN), j_op, current_line_number);
                    } else {
                         // Этого не должно быть, если TD корректен
                         throw std::runtime_error(format_error_message("Unexpected error '" + buf + "' not found in TD"));
                    }
                } else {
                    // Односимвольный оператор: =, <, >
                    int j_op = look(buf, TD);
                    if (j_op > 0) {
                         return Lex(static_cast<type_of_lex>(j_op + LEX_FIN), j_op, current_line_number);
                    } else if (buf == "!") {
                         // Ошибка: '!' без '='
                         throw std::runtime_error(format_error_message("Unexpected character '!' (expected '!=')"));
                    } else {
                         // Этого не должно быть для '=', '<', '>'
                         throw std::runtime_error(format_error_message("Unexpected error'" + buf + "' not found in TD"));
                    }
                }
                break; // Конец case ALE

            // Состояние COM: Внутри комментария /* ... */
            case COM:
                if (current_char == '*') {
                    // Возможно, конец комментария
                    gc(); // Смотрим следующий символ
                    if (current_char == '/') {
                        // Да, конец комментария '*/'
                        gc(); // Пропускаем '/'
                        CS = H; // Возвращаемся в начальное состояние
                    } else {
                        // Это был не конец (например, '/**' или '*a')
                        // Возвращать символ не нужно, продолжаем цикл в состоянии COM
                         unget(current_char); // Важно вернуть символ, если он не '/'!
                                              // Иначе символ после '*' будет пропущен.
                    }
                } else if (current_char == '\n') {
                    // Новая строка внутри комментария
                    current_line_number++;
                    gc();
                } else if (current_char == EOF) {
                    // Ошибка - комментарий не закрыт до конца файла
                    throw std::runtime_error(format_error_message("Unterminated comment"));
                } else {
                    // Любой другой символ внутри комментария - пропускаем
                    gc();
                }
                break; // Конец case COM

        } // end switch
    } // end while
}

// Перегрузка оператора вывода для Lex 
std::ostream &operator<<( std::ostream &s, const Lex& lex_to_print ) {
    s << "Line " << lex_to_print.line_lex << ": ";
    type_of_lex t = lex_to_print.t_lex;

    // Вывод типа лексемы
    if (t < LEX_FIN) { // Ключевые слова
        int index = static_cast<int>(t);
        // Простая проверка индекса (массивы TW/TD имеют nullptr на концах)
        if (Scanner::TW[index] != nullptr) {
             s << "Keyword(   " << Scanner::TW[index] << "   )";
        } else {
             s << "UnknownKeyword(type=   " << index << "   )";
        }
    } else if (t >= LEX_SEMICOLON && t <= LEX_RFIG) { // Разделители/Операторы
        int index = lex_to_print.v_lex; // v_lex хранит индекс 
        if (Scanner::TD[index] != nullptr) {
            s << "Delimiter(   " << Scanner::TD[index] << "   )";
        } else {
             s << "UnknownDelimiter(type=   " << t << ", val=" << index << "   )";
        }
    } else if (t == LEX_NUM) {
        s << "Number(   " << lex_to_print.v_lex << "   )";
    } else if (t == LEX_STR) {
        s << "String(   \"" << lex_to_print.s_lex << "\"   )"; 
    } else if (t == LEX_ID) {
        int index = lex_to_print.v_lex;
        if (index >= 0 && static_cast<size_t>(index) < TID.size()) {
             s << "Ident(" << TID[index].get_name() << ", index=" << index << ")";
        } else {
             s << "InvalidIdent(index=" << index << ")"; // Ошибка: некорректный индекс
        }
    } else if (t == LEX_FIN) {
        s << "End of file";
    } else if (t == LEX_NULL) {
        s << "Null Lexeme "; 
    } else { 
         s << "OtherLexeme(type=" << t << ", value=" << lex_to_print.v_lex << ")";
    }
    s << std::endl;
    return s;
}


int main () {
    try {
        // Создаем сканер для файла "a.txt"
        Scanner lexer("a.txt"); 
        Lex current_lex;

        std::cout << "--- Lexical Analysis Start ---" << std::endl;

        do {
            current_lex = lexer.get_lex(); // Получаем следующую лексему
            std::cout << current_lex;       // Выводим информацию о лексеме
        } while (current_lex.get_type() != LEX_FIN);

        std::cout << "--- Lexical Analysis End ---" << std::endl;

        // Вывод таблицы идентификаторов (TID) для проверки
        std::cout << "\n--- Identifier Table (TID) ---" << std::endl;
        if (TID.empty()) {
            std::cout << "(empty)" << std::endl;
        } else {
            for(size_t i = 0; i < TID.size(); ++i) {
                std::cout << "Index " << i << ": " << TID[i].get_name() << std::endl;
            }
        }
        std::cout << std::endl;
        return 0;

    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1; 
    } catch (...) {
        std::cerr << "An unexpected error occurred" << std::endl;
        return 1; 
    }
}
