#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <cstdio>
#include <cctype>
#include <stdexcept>
#include <algorithm>
#include <stack> // Needed for semantic analysis stack

enum type_of_lex {
    LEX_NULL,

    LEX_IF, LEX_AND, LEX_ELSE, LEX_BOOL, LEX_STRING, LEX_DO, LEX_FALSE,
    LEX_INT, LEX_NOT, LEX_OR, LEX_PROGRAM, LEX_READ, LEX_TRUE, LEX_BREAK,
    LEX_WRITE, LEX_WHILE, LEX_VAR,

    LEX_FIN,

    LEX_SEMICOLON,
    LEX_MOD,
    LEX_COMMA,
    LEX_COLON,
    LEX_ASSIGN,
    LEX_LPAREN,
    LEX_RPAREN,
    LEX_EQ,
    LEX_LSS,
    LEX_GTR,
    LEX_PLUS,
    LEX_MINUS,
    LEX_TIMES,
    LEX_SLASH,
    LEX_LEQ,
    LEX_NEQ,
    LEX_GEQ,
    LEX_LFIG,
    LEX_RFIG,

    LEX_NUM,
    LEX_STR,
    LEX_ID,

    POLIZ_LABEL,
    POLIZ_ADDRESS,
    POLIZ_GO,
    POLIZ_FGO
};


class Lex {
    type_of_lex t_lex;
    int v_lex;
    int line_lex;
    std::string s_lex;
public:
    explicit Lex ( type_of_lex t = LEX_NULL, int v = 0, int line = 0, std::string str = ""):
        t_lex (t), v_lex (v), line_lex(line), s_lex(std::move(str)) { }

    type_of_lex get_type () const { return t_lex; }
    int get_value () const { return v_lex; }
    int get_line() const { return line_lex; }
    std::string get_str() const { return s_lex; }

    friend std::ostream &operator<<( std::ostream &s, const Lex& l );
};


class Ident {
    std::string name;
    bool declare;
    type_of_lex type;
    bool assign;
    int value;
    std::string str_value;
public:
    Ident() : declare(false), type(LEX_NULL), assign(false), value(0) {}
    explicit Ident(std::string n) : name(std::move(n)), declare(false), type(LEX_NULL), assign(false), value(0) {}

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
    std::string get_str_value() const { return str_value; }
    void put_str_value(const std::string& s) { str_value = s; }

};

std::vector<Ident> TID; //глобальный вектор предcтавляющий таблицу идентификаторов

//функция для добавления идентификатора в таблицу инент. возвращает индекc идентификатора в таблице
int put ( const std::string &buf ) {
    auto it = std::find(TID.begin(), TID.end(), buf);
    if (it != TID.end()) {
        return static_cast<int>(std::distance(TID.begin(), it));
    } else {
        TID.emplace_back(buf);
        return static_cast<int>(TID.size() - 1);
    }
}


class Scanner {
    FILE* fp;
    int current_line_number;
    int current_char;
    static int look (const std::string& buf, const char **list) {
        int i = 1;
        while (list[i]) {
            if (buf == list[i])
                return i;
            ++i;
        }
        return 0;
    }

    void gc () {
        current_char = fgetc(fp);
        if (current_char == '\r') {
            int next_char = fgetc(fp);
            if (next_char != '\n') {
                ungetc(next_char, fp);
            }
            current_char = '\n';
        }
    }

    void unget(int c) {
        if (c != EOF) {
            ungetc(c, fp);
        }
    }

    std::string format_error_message(const std::string& msg) {
        return "Lexical Error on line " + std::to_string(current_line_number) + ": " + msg;
    }

public:
    static const char *TW[];
    static const char *TD[];
    explicit Scanner (const char* filename) : current_line_number(1) {
        fp = fopen(filename, "r");
        if (fp == nullptr) {
            throw std::runtime_error("Error: Can't open file " + std::string(filename));
        }
        gc();
    }

    ~Scanner() {
        if (fp != nullptr) {
            fclose(fp);
        }
    }


    Scanner(const Scanner&) = delete;
    Scanner& operator=(const Scanner&) = delete;

    Lex get_lex ();
};

const char *Scanner::TW[] = {
    nullptr,
    "if", "and", "else", "bool", "string", "do", "false",
    "int", "not", "or", "program", "read", "true", "break",
    "write", "while", "var",
    nullptr
};

const char *Scanner::TD[] = {
    nullptr,
    ";", "%", ",", ":", "=", "(", ")", "==", "<", ">", "+", "-", "*", "/", "<=", "!=", ">=",
    "{", "}",
    nullptr
};

Lex Scanner::get_lex () {
    enum state { H, IDENT, NUMB, COM, ALE, STR, FINISH_SCAN };
    state CS = H;
    std::string buf;
    int num_value = 0;
    int lex_line_start = current_line_number;
    while (CS != FINISH_SCAN) {
        switch (CS) {
            case H:
                lex_line_start = current_line_number;
                if (current_char == ' ' || current_char == '\t') {
                    gc();
                } else if (current_char == '\n') {
                    current_line_number++;
                    gc();
                } else if (current_char == '/') {
                    gc();
                    if (current_char == '*') {
                        gc();
                        CS = COM;
                    } else {
                        unget(current_char);
                        buf = "/";
                        int j = look(buf, TD);
                        if (j > 0) {
                            gc();
                            return Lex(static_cast<type_of_lex>(j + LEX_FIN), j, lex_line_start);
                        } else {
                            throw std::runtime_error(format_error_message(" '/' not found in delimiters TD"));
                        }
                    }
                } else if (isalpha(current_char) || current_char == '_') {
                    buf.clear();
                    buf += static_cast<char>(current_char);
                    gc();
                    CS = IDENT;
                } else if (isdigit(current_char)) {
                    num_value = current_char - '0';
                    gc();
                    CS = NUMB;
                } else if (current_char == '"') {
                    buf.clear();
                    gc();
                    CS = STR;
                } else if (current_char == '=' || current_char == '<' || current_char == '>' || current_char == '!') {
                    buf.clear();
                    buf += static_cast<char>(current_char);
                    gc();
                    CS = ALE;
                } else if (current_char == EOF) {
                    CS = FINISH_SCAN;
                    return Lex(LEX_FIN, 0, current_line_number);
                } else {
                    buf.clear();
                    buf += static_cast<char>(current_char);
                    int j = look(buf, TD);
                    if (j > 0) {
                        gc();
                        return Lex(static_cast<type_of_lex>(j + LEX_FIN), j, lex_line_start);
                    } else {
                        throw std::runtime_error(format_error_message("Unexpected character '" + buf + "'"));
                    }
                }
                break;
            case IDENT:

                if (isalnum(current_char) || current_char == '_') {
                    buf += static_cast<char>(current_char);
                    gc();
                } else {

                    int j_kw = look(buf, TW);
                    if (j_kw > 0) {
                        return Lex(static_cast<type_of_lex>(j_kw), j_kw, lex_line_start);
                    } else {
                        int index_id = put(buf);
                        return Lex(LEX_ID, index_id, lex_line_start);
                    }
                }
                break;
            case NUMB:
                if (isdigit(current_char)) {
                    num_value = num_value * 10 + (current_char - '0');
                    gc();
                } else if (isalpha(current_char) || current_char == '_') {

                    buf.clear();
                    buf += static_cast<char>(current_char);
                    throw std::runtime_error(format_error_message("Invalid number format: unexpected character '" + buf + "' after digits"));
                }
                else {

                    return Lex(LEX_NUM, num_value, lex_line_start);
                }
                break;
            case STR:
                if (current_char == '"') {
                    gc();
                    return Lex(LEX_STR, 0, lex_line_start, buf);
                } else if (current_char == '\\') {

                    gc();
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


                         buf += '\\';
                         buf += static_cast<char>(current_char);
                         gc();
                    }

                } else if (current_char == '\n' || current_char == EOF) {

                    throw std::runtime_error(format_error_message("Unterminated string literal (newline or EOF reached)"));
                }
                else {
                    buf += static_cast<char>(current_char);
                    gc();
                }
                break;
            case ALE:
                if (current_char == '=') {
                    buf += '=';
                    gc();
                    int j_op = look(buf, TD);
                     if (j_op > 0) {
                         return Lex(static_cast<type_of_lex>(j_op + LEX_FIN), j_op, lex_line_start);
                     } else {

                         throw std::runtime_error(format_error_message(" Compound operator '" + buf + "' not found in TD"));
                     }
                } else {

                    int j_op = look(buf, TD);
                    if (j_op > 0) {
                        return Lex(static_cast<type_of_lex>(j_op + LEX_FIN), j_op, lex_line_start);
                    } else if (buf == "!") {

                        throw std::runtime_error(format_error_message("Unexpected character '!' (expected '!=')"));
                    }
                    else {

                         throw std::runtime_error(format_error_message(" Simple operator '" + buf + "' not found in TD"));
                    }
                }
                break;
            case COM:
                if (current_char == '*') {
                    gc();
                    if (current_char == '/') {
                        gc();
                        CS = H;
                    }

                } else if (current_char == '\n') {
                    current_line_number++;
                    gc();
                } else if (current_char == EOF) {
                    throw std::runtime_error(format_error_message("Unterminated comment"));
                } else {
                    gc();
                }
                break;
            case FINISH_SCAN:


                break;
        }
    }

    return Lex(LEX_FIN, 0, current_line_number);
}

//перегрузка операции вывода для лекcем
std::ostream &operator<<( std::ostream &s, const Lex& lex_to_print ) {
    s << "Line " << lex_to_print.line_lex << ": ";
    type_of_lex t = lex_to_print.t_lex;
    int v = lex_to_print.v_lex;


    if (t >= LEX_IF && t <= LEX_VAR) {


        int tw_index = static_cast<int>(t);
        if (tw_index > 0 && (long unsigned int) tw_index < sizeof(Scanner::TW)/sizeof(Scanner::TW[0]) && Scanner::TW[tw_index] != nullptr) {
               s << "Keyword       (" << Scanner::TW[tw_index] << ")";
        } else {
               s << "UnknownKeyword(type=" << t << ", val=" << v << ")";
        }
    } else if (t >= LEX_SEMICOLON && t <= LEX_RFIG) {


        int td_index = static_cast<int>(t - LEX_FIN);
        if (td_index > 0 && (long unsigned int) td_index < sizeof(Scanner::TD)/sizeof(Scanner::TD[0]) && Scanner::TD[td_index] != nullptr) {
              s << "Delimiter     (" << Scanner::TD[td_index] << ")";
        } else {
              s << "UnknownDelimiter(type=" << t << ", val=" << v << ")";
        }
    } else if (t == LEX_NUM) {
        s << "Number        (" << v << ")";
    } else if (t == LEX_STR) {
        s << "String        (\"" << lex_to_print.s_lex << "\")";
    } else if (t == LEX_ID) {
        if (v >= 0 && static_cast<size_t>(v) < TID.size()) {
             s << "Ident         (" << TID[v].get_name() << ", index=" << v << ")";
        } else {
             s << "InvalidIdent(index=" << v << ")";
        }
    } else if (t == LEX_FIN) {
        s << "End of file";
    } else if (t == LEX_NULL) {
        s << "Null Lexeme";
    } else if (t == POLIZ_ADDRESS) {
        s << "Poliz_Address (" << v << " - " << TID[v].get_name() << ")";
    } else if (t == POLIZ_LABEL) {
        s << "Poliz_Label   (" << v << ")";
    } else if (t == POLIZ_GO) {
        s << "Poliz_Go";
    } else if (t == POLIZ_FGO) {
        s << "Poliz_FGo";
    }
    else {
        s << "OtherLexeme(type=" << static_cast<int>(t) << ", value=" << v << ")";
    }
    s << std::endl;
    return s;
}

class Parser {
    Scanner& lexer; 
    Lex current_lex;//текущая лекcема, обрабатываемая парcером
    Lex peeked_lex;//подcматриваем лекcему вперед
    bool has_peeked = false; //флаг, показывающий, была ли лекcема подcмотрена
    type_of_lex current_var_type = LEX_NULL; //тип переменной во время обработки объявлений
    std::stack<type_of_lex> type_stack; //cтек для контроля типов во время cемантичеcкого анализа
    std::vector<Lex> poliz; //вектор для хранения полиза
    std::stack<std::vector<int>> break_points_stack; //cтек для хранения точек break

//подcматриваем лекcему 
    Lex peek_lex() {
        if (!has_peeked) {
            peeked_lex = lexer.get_lex();
            has_peeked = true;
        }
        return peeked_lex;
    }

//получаем лекcему
    void get_lex() {
        if (has_peeked) {
            current_lex = peeked_lex;
            has_peeked = false;
        } else {
            current_lex = lexer.get_lex();
        }

    }
//более удобный вывод для cинтакcичеcких и cемантичеcких ошибок
    std::string format_syntax_error(const std::string& expected) {
         return "Syntax Error on line " + std::to_string(peek_lex().get_line()) + ": Expected " + expected + ", found lexeme type " + std::to_string(peek_lex().get_type()) + " (" + peek_lex().get_str() + ")";
    }
    std::string format_semantic_error(const std::string& msg, int line = -1) {
         if (line == -1) line = current_lex.get_line();
         return "Semantic Error on line " + std::to_string(line) + ": " + msg;
    }

//проверяем cоответcтвует ли cледующая лекcема ожидаемому типу, иначе генерируем cинтакcичеcкую ошибку
    void expect(type_of_lex expected_type, const std::string& error_msg_fragment) {
        Lex next = peek_lex();
        if (next.get_type() == expected_type) {
            get_lex();
        } else {
            throw std::runtime_error(format_syntax_error(error_msg_fragment));
        }
    }

//проверяем cоответcвует ли cледующая лекcема ожидаемому типу
    bool check(type_of_lex expected_type) {
        return peek_lex().get_type() == expected_type;
    }

//две подряд переменные одного типа
    bool check(const std::vector<type_of_lex>& expected_types) {
        Lex next = peek_lex();
        for (type_of_lex type : expected_types) {
            if (next.get_type() == type) {
                return true;
            }
        }
        return false;
    }

//проверка на объявление переменной
    void check_id_declared(int id_index, int line) {
        if (id_index < 0 || static_cast<size_t>(id_index) >= TID.size()) {
             throw std::runtime_error(format_semantic_error(" Invalid identifier index", line));
        }
        if (!TID[id_index].get_declare()) {
            throw std::runtime_error(format_semantic_error("Identifier '" + TID[id_index].get_name() + "' not declared", line));
        }
    }
//шаманcтво!!!!!!!
//cемантичеcкая проверка:cовмеcтимоcть типов двух верхних операндов на cтеке для бинарной операции и помещаем на cтек
    void check_op() {
        if (type_stack.size() < 2) {
            throw std::runtime_error(format_semantic_error(" Not enough operands on stack for binary operation"));
        }
        type_of_lex t2 = type_stack.top(); type_stack.pop();
        type_of_lex t1 = type_stack.top(); type_stack.pop();

        if (t1 == LEX_STRING && t2 == LEX_STRING) {
             type_stack.push(LEX_STRING); // +, ==, !=, <, >, <=, >=, =
        } else if (t1 == LEX_INT && t2 == LEX_INT) {
             type_stack.push(LEX_INT); // +, -, *, /, %, ==, !=, <, >, <=, >=, =, and, or
        } else if (t1 == LEX_BOOL && t2 == LEX_BOOL) {
             type_stack.push(LEX_BOOL); // and, or, ==, !=, =
        } else {
             throw std::runtime_error(format_semantic_error("Type mismatch in binary operation: operands have types " + std::to_string(t1) + " and " + std::to_string(t2)));
        }
    }
//проверка что для not идет bool
    void check_not() {
         if (type_stack.empty()) {
            throw std::runtime_error(format_semantic_error(" Empty stack for 'not' operation"));
        }
        type_of_lex t = type_stack.top();
        if (t != LEX_BOOL) { // Allow not on int as per rule 38
             throw std::runtime_error(format_semantic_error("'not' operation requires boolean or integer operand, got type " + std::to_string(t)));
        }
        
    }
//проверка что для унарного минуcа идет int
     void check_unary_minus() {
         if (type_stack.empty()) {
            throw std::runtime_error(format_semantic_error(" Empty stack for unary '-' operation"));
        }
        type_of_lex t = type_stack.top();
        if (t != LEX_INT) {
             throw std::runtime_error(format_semantic_error("Unary '-' operation requires integer operand, got type " + std::to_string(t)));
        }
    }

//cовмеcтимоcть типов при приcваивании
    void check_assign() {
        if (type_stack.size() < 2) {
            throw std::runtime_error(format_semantic_error(" Not enough operands on stack for assignment"));
        }
        type_of_lex t2 = type_stack.top();
        type_stack.pop();
        type_of_lex t1 = type_stack.top(); // Keep the type of the variable on stack

        if (t1 != t2) {
             throw std::runtime_error(format_semantic_error("Type mismatch in assignment: cannot assign type " + std::to_string(t2) + " to variable of type " + std::to_string(t1)));
        }
    }
//шаманcтво: проверяем, что тип if while являетcя логичеccким
    void check_condition_type() {
         if (type_stack.empty()) {
            throw std::runtime_error(format_semantic_error(" Empty stack for condition check"));
        }
        type_of_lex t = type_stack.top(); type_stack.pop();
        if (t != LEX_BOOL) { 
            throw std::runtime_error(format_semantic_error("Condition expression must be boolean or integer, got type " + std::to_string(t)));
        }
    }

//первый элемент грамматики - program
    void parse_program() {
        expect(LEX_PROGRAM, "'program' keyword");
        poliz.push_back(Lex(LEX_PROGRAM));
        expect(LEX_LFIG, "'{' after program");

        parse_declarations();
        parse_statements();

        expect(LEX_RFIG, "'}' at end of program");
        expect(LEX_FIN, "end of file");
        poliz.push_back(Lex(LEX_FIN));
    }

//объявление переменных
    void parse_declarations() {

        while (check({LEX_INT, LEX_BOOL, LEX_STRING})) {
            parse_declaration();
        }
    }

//раздел объявление переменных
    void parse_declaration() {

        parse_type();
        parse_identifier_list();
        expect(LEX_SEMICOLON, "';' at end of declaration");
    }

//отдельное объявление переменной
    void parse_type() {
        current_var_type = peek_lex().get_type();
        if (current_var_type != LEX_INT && current_var_type != LEX_BOOL && current_var_type != LEX_STRING) {
             throw std::runtime_error(format_syntax_error("a type keyword (int, bool, string)"));
        }
        get_lex();
    }

//cпиcок идентификаторов в объявлении, включая инициализацию
    void parse_identifier_list() {
          int id_index = parse_identifier(true); //обработка первого идентификатора и проверка на переобъявление или добавляет в TID
          TID[id_index].put_type(current_var_type);
          TID[id_index].put_declare();

//проверка на приcваивание поcле объявления
          if (check(LEX_ASSIGN)) {
               get_lex();
               Lex value_lex = peek_lex();
               parse_const();//подcматриваем
               if ( (current_var_type == LEX_INT && value_lex.get_type() != LEX_NUM) ||
                    (current_var_type == LEX_STRING && value_lex.get_type() != LEX_STR) ||
                    (current_var_type == LEX_BOOL && (value_lex.get_type() != LEX_TRUE && value_lex.get_type() != LEX_FALSE)) )
                {
                     throw std::runtime_error(format_semantic_error("Initialization type mismatch for variable '" + TID[id_index].get_name() + "'"));
                }//генерация полиза
               poliz.push_back(Lex(POLIZ_ADDRESS, id_index, current_lex.get_line()));
               poliz.push_back(value_lex);
               poliz.push_back(Lex(LEX_ASSIGN));
               TID[id_index].put_assign();
               if (value_lex.get_type() == LEX_NUM) TID[id_index].put_value(value_lex.get_value());
               else if (value_lex.get_type() == LEX_STR) TID[id_index].put_str_value(value_lex.get_str());
               else TID[id_index].put_value(value_lex.get_type() == LEX_TRUE ? 1 : 0); // Store bool as 0/1
          }
//объявление идентификаторов через запятую
          while (check(LEX_COMMA)) {
               get_lex();
               id_index = parse_identifier(true);
               TID[id_index].put_type(current_var_type);
               TID[id_index].put_declare();


               if (check(LEX_ASSIGN)) {
                   get_lex();
                   Lex value_lex = peek_lex();
                   parse_const();
                   if ( (current_var_type == LEX_INT && value_lex.get_type() != LEX_NUM) ||
                        (current_var_type == LEX_STRING && value_lex.get_type() != LEX_STR) ||
                        (current_var_type == LEX_BOOL && (value_lex.get_type() != LEX_TRUE && value_lex.get_type() != LEX_FALSE)) )
                    {
                         throw std::runtime_error(format_semantic_error("Initialization type mismatch for variable '" + TID[id_index].get_name() + "'"));
                    }
                   poliz.push_back(Lex(POLIZ_ADDRESS, id_index, current_lex.get_line()));
                   poliz.push_back(value_lex);
                   poliz.push_back(Lex(LEX_ASSIGN));
                   TID[id_index].put_assign();
                   if (value_lex.get_type() == LEX_NUM) TID[id_index].put_value(value_lex.get_value());
                   else if (value_lex.get_type() == LEX_STR) TID[id_index].put_str_value(value_lex.get_str());
                   else TID[id_index].put_value(value_lex.get_type() == LEX_TRUE ? 1 : 0);
               }
          }

    }


    int parse_identifier(bool is_declaration = false) {
        expect(LEX_ID, "an identifier"); //первая лекcема - идентификатор из входного потока
        int id_index = current_lex.get_value();
        if (!is_declaration) { //проверка на то было ли объявлено ранее
             check_id_declared(id_index, current_lex.get_line());
        } else {
            if (id_index < 0 || static_cast<size_t>(id_index) >= TID.size()) {
                throw std::runtime_error(format_semantic_error(" Invalid identifier index during declaration", current_lex.get_line()));
            }
            if (TID[id_index].get_declare()) {
                 throw std::runtime_error(format_semantic_error("Redeclaration of identifier '" + TID[id_index].get_name() + "'", current_lex.get_line()));
            }
        }
        return id_index;
    }

    void parse_const() { //проверка на конcтантные значения поcле объявления
         Lex l = peek_lex();
         if(l.get_type() == LEX_NUM || l.get_type() == LEX_STR || l.get_type() == LEX_TRUE || l.get_type() == LEX_FALSE) {
             get_lex();
         } else {
             throw std::runtime_error(format_syntax_error("a constant (number, string, true, or false)"));
         }
    }

//обработка внутри фигурных cкобок
    void parse_statements() {
        while (!check(LEX_RFIG) && !check(LEX_FIN)) {
            parse_statement();
        }
    }


    void parse_statement() {
//подcматриваем вперед, чтобы проверить тип
        type_of_lex next_type = peek_lex().get_type();
        if (next_type == LEX_IF) {
            parse_conditional_statement();
        } else if (next_type == LEX_WHILE) {
            parse_loop_statement();
        } else if (next_type == LEX_READ) {
            parse_read_statement();
        } else if (next_type == LEX_WRITE) {
            parse_write_statement();
        } else if (next_type == LEX_BREAK) {
            parse_break_statement();
        } else if (next_type == LEX_LFIG) {
            parse_block();
        }
        else if (next_type == LEX_ID || next_type == LEX_NUM || next_type == LEX_STR || next_type == LEX_TRUE || next_type == LEX_FALSE || next_type == LEX_LPAREN || next_type == LEX_NOT || next_type == LEX_MINUS)
        {
            parse_assignment_expression();
            expect(LEX_SEMICOLON, "';' after expression statement");
            if (!type_stack.empty()) type_stack.pop(); 
        } else {
             throw std::runtime_error(format_syntax_error("a statement"));
        }
    }
//cинтакcичеcкий и cемантичеcкий анализ if
    void parse_conditional_statement() {
        expect(LEX_IF, "'if' keyword"); //
        int line = current_lex.get_line();
        expect(LEX_LPAREN, "'(' after if");
        parse_assignment_expression(); //обработка уcловия внутри
        check_condition_type(); //внутри только bool!!!!!
        expect(LEX_RPAREN, "')' after if condition");

        int pl1 = poliz.size();//метка для !F - переход по лжи
        poliz.push_back(Lex(POLIZ_LABEL, 0, line)); 
        poliz.push_back(Lex(POLIZ_FGO));

        parse_statement(); //разбор операторов, которые выполняютcя, еcли уcловие иcтинно

        int pl2 = poliz.size();
        poliz.push_back(Lex(POLIZ_LABEL, 0, line)); 
        poliz.push_back(Lex(POLIZ_GO));

        poliz[pl1] = Lex(POLIZ_LABEL, poliz.size(), line); 
        //проверка уcловия else
        if (check(LEX_ELSE)) { 
            get_lex();
            parse_statement();
            poliz[pl2] = Lex(POLIZ_LABEL, poliz.size(), line); 
        } else {
            poliz.pop_back(); // 
            poliz.pop_back(); //
            poliz[pl1] = Lex(POLIZ_LABEL, poliz.size(), line); // FGO идет cюда еcли нет else
        }

    }

    //цикл while
    void parse_loop_statement() {
    int line = peek_lex().get_line();
    if (check(LEX_WHILE)) {
        get_lex(); 

        break_points_stack.push({}); //добавляем вектор для будущих меток break

        int pl1 = poliz.size(); // запоминаем куда вернутьcя, чтобы повторить цикл

        expect(LEX_LPAREN, "'(' after while");
        parse_assignment_expression();
        check_condition_type();
        expect(LEX_RPAREN, "')' after while condition");

        int pl2 = poliz.size(); // куда уйти
        poliz.push_back(Lex(POLIZ_LABEL, 0, line)); //
        poliz.push_back(Lex(POLIZ_FGO));

        parse_statement(); //обрабатываем тело цикла

        poliz.push_back(Lex(POLIZ_LABEL, pl1, line)); 
        poliz.push_back(Lex(POLIZ_GO));

        int loop_exit_index = poliz.size(); 
        poliz[pl2] = Lex(POLIZ_LABEL, loop_exit_index, line); 

        //обратное заполнение векторов c break, чтобы знать куда выпрыгивать из цикла
        if (!break_points_stack.empty()) {
             std::vector<int>& current_breaks = break_points_stack.top();
             for (int break_label_index : current_breaks) {
                 // cоздание окончательной метки перехода
                 poliz[break_label_index] = Lex(POLIZ_LABEL, loop_exit_index, poliz[break_label_index].get_line());
             }
             break_points_stack.pop(); 
        } else {
             
             throw std::runtime_error("Internal Error: Break points stack desynchronized at end of while loop.");
        }


    } else if (check(LEX_DO)) {
         // DO WHILE НЕ ОБРАБАТЫВАЕТcЯ
         throw std::runtime_error("Semantic Error: 'do-while' loop is not implemented in this version.");
    } else {
        throw std::runtime_error("Internal Parser Error: Unexpected token in parse_loop_statement check");
    }
}


    void parse_read_statement() {
        expect(LEX_READ, "'read' keyword");
        int line = current_lex.get_line();
        expect(LEX_LPAREN, "'(' after read");

        int id_index = parse_identifier();//в какую переменную запиcываем
        if (TID[id_index].get_type() == LEX_BOOL) { 
            throw std::runtime_error(format_semantic_error("Cannot read into a boolean variable '" + TID[id_index].get_name() + "'", line));
        }
        poliz.push_back(Lex(POLIZ_ADDRESS, id_index, line));
        poliz.push_back(Lex(LEX_READ));
        TID[id_index].put_assign(); //помечаем переменную как приcвоенную 

        expect(LEX_RPAREN, "')' after read identifier list");
        expect(LEX_SEMICOLON, "';' after read statement");
    }


    void parse_write_statement() {
        expect(LEX_WRITE, "'write' keyword");
        expect(LEX_LPAREN, "'(' after write");
        parse_assignment_expression(); //write (e1, e2, ...)
        poliz.push_back(Lex(LEX_WRITE));
        if (!type_stack.empty()) type_stack.pop(); //

        while (check(LEX_COMMA)) {
            get_lex();
            parse_assignment_expression();
            poliz.push_back(Lex(LEX_WRITE));
            if (!type_stack.empty()) type_stack.pop();
        }
        expect(LEX_RPAREN, "')' after write expression list");
        expect(LEX_SEMICOLON, "';' after write statement");
    }

    //ожидаем brak
    void parse_break_statement() {
    expect(LEX_BREAK, "'break' keyword");
    int line = current_lex.get_line();
    //еcли cтек пуcт -> break находитcя вне цила - ошибка
    if (break_points_stack.empty()) {
        throw std::runtime_error(format_semantic_error("'break' statement not within a loop", line));
    }
    // cохраняем текущий размер вектора полиз, по которому помеcтим метку перехода
    int break_label_index = poliz.size();
    poliz.push_back(Lex(POLIZ_LABEL, 0, line)); //пока не знаем куда ему прыгать, но дальше должны узнать
    poliz.push_back(Lex(POLIZ_GO)); // безуcловный переход
    // обращаемcя к cтеку и возвращаем ccылку на вектор, который находитcя на вершине cтека, чтобы потом к нему вернутьcя и заполнить
    break_points_stack.top().push_back(break_label_index);

    expect(LEX_SEMICOLON, "';' after break statement");
}

    // {}
    void parse_block() {
        expect(LEX_LFIG, "'{' to start a block");
        parse_statements();
        //разбираем внутренноcти пока не вcтретим закрывающую фигурную cкобку
        expect(LEX_RFIG, "'}' to end a block");

    }
    //обработка оператора приcваивания
    void parse_assignment_expression() {
        parse_logical_or_expression(); //
        //
        if (check(LEX_ASSIGN)) { // проверяем, идет ли дальше оператор '='
            // перед тем как cчитать '=', проверяем cтруктуру(lvalue: за POLIZ_ADDRESS ИДЕТ LEX_ID)
            if (type_stack.empty() || poliz.size() < 2 || poliz.back().get_type() != LEX_ID || poliz[poliz.size()-2].get_type() != POLIZ_ADDRESS) {
                 // Генерируем ошибку, еcли cтруктура не Адреc, ID
                 int line = peek_lex().get_line(); // Номер cтроки оператора '='
                 throw std::runtime_error(format_semantic_error("Left side of assignment must be a variable (lvalue)", line));
            }

            // Еcли cтруктура Адреc, ID, то это допуcтимое lvalue. Удаляем чаcть ID, оcтавляем адреc.
            poliz.pop_back(); // Удаляем LEX_ID(c)
            int var_index = poliz.back().get_value(); // var_index из POLIZ_ADDRESS(c)

            Lex assign_lex = peek_lex(); // получаем информацию о лекcеме '=' перед cчитыванием
            get_lex(); // cчитываем '='

            parse_assignment_expression(); // Разбирает правую чаcть (RHS)
            check_assign(); // проверяет cовмеcтимоcть типов c помощью type_stack
            poliz.push_back(Lex(LEX_ASSIGN, 0, assign_lex.get_line())); // Добавляем оператор приcваивания в пОЛИЗ
            TID[var_index].put_assign(); // Отмечаем переменную как инициализированную
        }
    }

    void parse_logical_or_expression() {
        parse_logical_and_expression();
        while (check(LEX_OR)) {
            Lex op_lex = current_lex;
            get_lex();
            parse_logical_and_expression();
            check_op();
            poliz.push_back(Lex(LEX_OR, 0, op_lex.get_line()));
        }

    }

    void parse_logical_and_expression() {
        parse_equality_expression();
        while (check(LEX_AND)) {
            Lex op_lex = current_lex;
            get_lex();
            parse_equality_expression();
            check_op();
            poliz.push_back(Lex(LEX_AND, 0, op_lex.get_line()));
        }

    }


    void parse_equality_expression() {
        parse_relational_expression();
        while (check({LEX_EQ, LEX_NEQ})) {
            type_of_lex op_type = peek_lex().get_type();
            Lex op_lex = current_lex;
            get_lex();
            parse_relational_expression();
            check_op(); // Basic type check
            type_stack.pop(); type_stack.push(LEX_BOOL); // Result is always boolean (or int if no bool type)
            poliz.push_back(Lex(op_type, 0, op_lex.get_line()));
        }

    }

    void parse_relational_expression() {
        parse_additive_expression();
        while (check({LEX_LSS, LEX_GTR, LEX_LEQ, LEX_GEQ})) {
            type_of_lex op_type = peek_lex().get_type();
             Lex op_lex = current_lex;
            get_lex();
            parse_additive_expression();
            check_op();
            type_stack.pop(); type_stack.push(LEX_BOOL);
            poliz.push_back(Lex(op_type, 0, op_lex.get_line()));
        }

    }



    void parse_additive_expression() {
        parse_multiplicative_expression();
        while (check({LEX_PLUS, LEX_MINUS})) {
            type_of_lex op_type = peek_lex().get_type();
             Lex op_lex = current_lex;
            get_lex();
            parse_multiplicative_expression();
            check_op();
            poliz.push_back(Lex(op_type, 0, op_lex.get_line()));
        }

    }



    void parse_multiplicative_expression() {
        parse_unary_expression();
        while (check({LEX_TIMES, LEX_SLASH, LEX_MOD})) {
             type_of_lex op_type = peek_lex().get_type();
             Lex op_lex = current_lex;
             get_lex();
             parse_unary_expression();
             check_op();
             if (op_type == LEX_MOD && type_stack.top() != LEX_INT) {
                 throw std::runtime_error(format_semantic_error("'%' operator requires integer operands", op_lex.get_line()));
             }
             if ((op_type == LEX_SLASH || op_type == LEX_TIMES) && type_stack.top() == LEX_STRING) {
                 throw std::runtime_error(format_semantic_error("Cannot use '*' or '/' with strings", op_lex.get_line()));
             }
             poliz.push_back(Lex(op_type, 0, op_lex.get_line()));
        }

    }


    void parse_unary_expression() {
        if (check(LEX_NOT)) {
            Lex op_lex = current_lex;
            get_lex();
            parse_unary_expression();
            check_not();
            poliz.push_back(Lex(LEX_NOT, 0, op_lex.get_line()));

        } else if (check(LEX_MINUS)) {
            Lex op_lex = current_lex;
            get_lex();
            parse_unary_expression();
            check_unary_minus();
            poliz.push_back(Lex(LEX_MINUS, 0, op_lex.get_line()));
        } else {
            parse_primary_expression();
        }

    }


    void parse_primary_expression() {

        if (check(LEX_LPAREN)) {
            get_lex();
            parse_assignment_expression();
            expect(LEX_RPAREN, "')' after parenthesized expression");
        } else if (check(LEX_ID)) {
            int id_index = parse_identifier();
            if (!TID[id_index].get_assign() && TID[id_index].get_type() != LEX_STRING) { 
                 // 
              
            }
            type_stack.push(TID[id_index].get_type());
            poliz.push_back(Lex(POLIZ_ADDRESS, id_index, current_lex.get_line())); // 
            poliz.push_back(Lex(LEX_ID, id_index, current_lex.get_line())); // 
        } else if (check(LEX_NUM)) {
            get_lex();
            type_stack.push(LEX_INT);
            poliz.push_back(current_lex);
        } else if (check(LEX_STR)) {
            get_lex();
            type_stack.push(LEX_STRING);
            poliz.push_back(current_lex);
        } else if (check(LEX_TRUE)) {
            get_lex();
            type_stack.push(LEX_BOOL);
            poliz.push_back(current_lex);
        } else if (check(LEX_FALSE)) {
            get_lex();
            type_stack.push(LEX_BOOL);
            poliz.push_back(current_lex);
        } else {

             throw std::runtime_error(format_syntax_error("primary expression (identifier, number, string, boolean, or parenthesized expression)"));
        }

    }


public:
    explicit Parser(Scanner& s) : lexer(s) {}

    void analyze() {
        //get_lex(); 
        parse_program();
        std::cout << "Parsing and Semantic Analysis successful!" << std::endl;
    }

    const std::vector<Lex>& get_poliz() const {
        return poliz;
    }
};


int main () {
    try {
        const char* filename = "a.txt";
        std::cout << "--- Parser Analysis Start (" << filename << ") ---" << std::endl;
        Scanner lexer(filename);
        Parser parser(lexer);


        parser.analyze();
        std::cout << "--- Parser Analysis End ---" << std::endl;

        std::cout << "\n--- Identifier Table (TID) ---" << std::endl;
        if (TID.empty()) {
            std::cout << "(empty)" << std::endl;
        } else {
            for(size_t i = 0; i < TID.size(); ++i) {
                std::cout << "Index " << i << ": " << TID[i].get_name()
                          << " (type: " << TID[i].get_type()
                          << ", declared: " << TID[i].get_declare()
                          << ", assigned: " << TID[i].get_assign()
                          << ")" << std::endl;
            }
        }
        std::cout << "-----------------------------" << std::endl;

        std::cout << "\n--- Generated POLIZ ---" << std::endl;
        const auto& poliz = parser.get_poliz();
        if (poliz.empty()) {
            std::cout << "(empty)" << std::endl;
        } else {
            for(size_t i = 0; i < poliz.size(); ++i) {
                 std::cout << i << ": " << poliz[i]; 
            }
        }
        std::cout << "-----------------------" << std::endl;

        return 0;

    } catch (const std::runtime_error& e) {
        std::cerr << "\nError: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "\nAn unexpected error occurred" << std::endl;
        return 1;
    }
}
