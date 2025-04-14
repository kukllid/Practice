#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <cstdio>
#include <cctype>
#include <stdexcept>
#include <algorithm>
#include <stack>

enum type_of_lex {
    LEX_NULL,
    LEX_PROGRAM,
    LEX_INT,
    LEX_STRING,
    LEX_BOOLEAN,
    LEX_IF,
    LEX_ELSE,
    LEX_WHILE,
    LEX_READ,
    LEX_WRITE,
    LEX_TRUE,
    LEX_FALSE,
    LEX_BREAK,
    LEX_NOT,
    LEX_AND,
    LEX_OR,
    LEX_FIN,
    LEX_LPAREN,
    LEX_RPAREN,
    LEX_LFIG,
    LEX_RFIG,
    LEX_SEMICOLON,
    LEX_COMMA,
    LEX_ASSIGN,
    LEX_PLUS,
    LEX_MINUS,
    LEX_TIMES,
    LEX_SLASH,
    LEX_PERCENT,
    LEX_EQ,
    LEX_LSS,
    LEX_GTR,
    LEX_LEQ,
    LEX_GEQ,
    LEX_NEQ,
    LEX_NUM,
    LEX_STR,
    LEX_ID
};

class Lex {
    type_of_lex t_lex;
    int v_lex;
    int line_lex;
    std::string s_lex;
public:
    Lex ( type_of_lex t = LEX_NULL, int v = 0, int line = 0, std::string str = ""):
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
    std::string s_value;

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
    std::string get_s_value() const { return s_value; }
    void put_s_value(std::string v) { s_value = std::move(v); }
};

std::vector<Ident> TID;

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
        while (list[i]) { if (buf == list[i]) return i; ++i; }
        return 0;
    }
    void gc () { current_char = fgetc(fp); }
    void unget(int c) { if (c != EOF) ungetc(c, fp); }
     std::string format_error_message(const std::string& msg) const {
        return "Lexical Error on line " + std::to_string(current_line_number) + ": " + msg;
    }

public:
    static const char *TW[];
    static const char *TD[];

    explicit Scanner (const char* filename) : current_line_number(1), current_char(' ') {
        fp = fopen(filename, "r");
        if (fp == nullptr) throw std::runtime_error("Error: Can't open file " + std::string(filename));
        gc();
    }
    ~Scanner() { if (fp != nullptr) fclose(fp); }
    Scanner(const Scanner&) = delete;
    Scanner& operator=(const Scanner&) = delete;
    Lex get_lex ();
};

const char *Scanner::TW[] = {
    nullptr, "program", "int", "string", "boolean", "if", "else", "while",
    "read", "write", "true", "false", "break", "not", "and", "or", nullptr };
const char *Scanner::TD[] = {
    nullptr, "(", ")", "{", "}", ";", ",", "=", "+", "-", "*", "/", "%",
    "==", "<", ">", "<=", ">=", "!=", nullptr };

Lex Scanner::get_lex () {
    enum state { H, IDENT, NUMB, COM, ALE, STR }; state CS = H;
    std::string buf; int num_value = 0; std::string str_value;
    while (true) {
        switch (CS) {
            case H:
                if (current_char == ' '||current_char == '\t'||current_char == '\r') gc();
                else if (current_char == '\n') { current_line_number++; gc(); }
                else if (isalpha(current_char)) { buf.clear(); buf += (char)current_char; CS = IDENT; gc(); }
                else if (isdigit(current_char)) { num_value = current_char - '0'; CS = NUMB; gc(); }
                else if (current_char == '"') { str_value.clear(); CS = STR; gc(); }
                else if (current_char == '/') { gc(); if (current_char == '*') { CS = COM; gc(); } else { unget(current_char); buf = "/"; goto CHECK_DELIM; } }
                else if (current_char == '=') { buf = "="; CS = ALE; gc(); } else if (current_char == '<') { buf = "<"; CS = ALE; gc(); }
                else if (current_char == '>') { buf = ">"; CS = ALE; gc(); } else if (current_char == '!') { buf = "!"; CS = ALE; gc(); }
                else if (current_char == '*') { gc(); if (current_char == '/') { gc(); throw std::runtime_error(format_error_message("Unexpected '*/' outside of a comment")); } else { unget(current_char); buf = "*"; goto CHECK_DELIM; } }
                else if (current_char == EOF) return Lex(LEX_FIN, 0, current_line_number);
                else { buf.clear(); buf += (char)current_char;
CHECK_DELIM:         int j_td = look(buf, TD);
                     if (j_td > 0) { type_of_lex dt;
                         if (buf=="(") dt=LEX_LPAREN; else if (buf==")") dt=LEX_RPAREN; else if (buf=="{") dt=LEX_LFIG; else if (buf=="}") dt=LEX_RFIG;
                         else if (buf==";") dt=LEX_SEMICOLON; else if (buf==",") dt=LEX_COMMA; else if (buf=="+") dt=LEX_PLUS; else if (buf=="-") dt=LEX_MINUS;
                         else if (buf=="*") dt=LEX_TIMES; else if (buf=="/") dt=LEX_SLASH; else if (buf=="%") dt=LEX_PERCENT; else throw std::runtime_error(format_error_message("Internal: Unhandled delimiter")); gc(); return Lex(dt, j_td, current_line_number);
                     } else throw std::runtime_error(format_error_message("Unexpected character '" + buf + "'"));
                } break;
            case IDENT: if (isalnum(current_char)) { buf += (char)current_char; gc(); } else { int j = look(buf, TW); if (j > 0) return Lex(static_cast<type_of_lex>(j), j, current_line_number); else { int i = put(buf); return Lex(LEX_ID, i, current_line_number); } } break;
            case NUMB: if (isdigit(current_char)) { num_value = num_value * 10 + (current_char - '0'); gc(); } else if (isalpha(current_char)) throw std::runtime_error(format_error_message("Invalid char after number")); else return Lex(LEX_NUM, num_value, current_line_number); break;
            case STR: if (current_char == '"') { gc(); return Lex(LEX_STR, 0, current_line_number, str_value); } else if (current_char == '\\') { gc(); switch(current_char){ case '"': str_value += '"'; gc(); break; case '\\': str_value += '\\'; gc(); break; case 'n': str_value += '\n'; gc(); break; case 't': str_value += '\t'; gc(); break; case EOF: throw std::runtime_error(format_error_message("EOF in string esc")); default: str_value += '\\'; str_value += (char)current_char; gc(); break; } } else if (current_char == '\n' || current_char == EOF) throw std::runtime_error(format_error_message("Unterminated string")); else { str_value += (char)current_char; gc(); } break;
            case ALE: if (current_char == '=') { buf += '='; int j = look(buf, TD); if (j > 0) { type_of_lex ot; if(buf=="==") ot=LEX_EQ; else if(buf=="<=") ot=LEX_LEQ; else if(buf==">=") ot=LEX_GEQ; else if(buf=="!=") ot=LEX_NEQ; else throw std::runtime_error("Internal: Unhandled '="+buf+"'"); gc(); return Lex(ot, j, current_line_number); } else throw std::runtime_error(format_error_message("Internal: '"+buf+"' not in TD")); } else { if (buf == "!") throw std::runtime_error(format_error_message("Expected '=' after '!'")); type_of_lex ot; if(buf=="=") ot=LEX_ASSIGN; else if(buf=="<") ot=LEX_LSS; else if(buf==">") ot=LEX_GTR; else throw std::runtime_error(format_error_message("Internal: Unexpected char in ALE")); return Lex(ot, 0, current_line_number); } break;
            case COM: if (current_char == '*') { gc(); if (current_char == '/') { gc(); CS = H; } } else if (current_char == '\n') { current_line_number++; gc(); } else if (current_char == EOF) throw std::runtime_error(format_error_message("Unterminated comment")); else gc(); break;
        }
    }
}

template <class T, class T_EL>
void from_st ( T & st, T_EL & i ) {
    if (st.empty()) throw std::runtime_error("Internal Parser Error: Stack underflow.");
    i = st.top(); st.pop();
}

class Parser {
    Lex         curr_lex;
    type_of_lex c_type;
    int         c_val;
    std::string c_str;
    int         c_line;
    Scanner     scan;

    std::stack <type_of_lex>  st_lex;
    std::stack <bool> loop_nesting_stack;

    void parseProgram();
    void parseDeclarations();
    void parseDeclaration();

    void parseVariable(type_of_lex var_type);

    void parseOperatorsBlock();
    void parseStatementsList();
    void parseStatement();
    void parseIfStatement();
    void parseWhileStatement();
    void parseReadStatement();
    void parseWriteStatement();
    void parseBreakStatement();

    void parseExpression();
    void parseTermAnd();
    void parseTermCompare();
    void parseTermAdd();
    void parseTermMul();
    void parseAssignment();
    int  parseFactor();

    void checkAndDeclareId(int id_index, type_of_lex var_type, bool initial_assign = false);
    void checkIdDeclared(int id_index);
    type_of_lex getIdType(int id_index);
    void checkOp();
    void checkNot();
    void checkConditionType();
    void checkReadId();
    void checkTypesCompatible(type_of_lex t1, type_of_lex t2, bool assignment = false);

    void gl () {
        curr_lex = scan.get_lex ();
        c_type   = curr_lex.get_type ();
        c_val    = curr_lex.get_value ();
        c_str    = curr_lex.get_str();
        c_line   = curr_lex.get_line();
    }

public:
    Parser ( const char *program ) : scan (program) { }
    void analyze();
};

void Parser::analyze() {
    gl();
    parseProgram();
    if (c_type != LEX_FIN)
        throw std::runtime_error("Syntax Error on line " + std::to_string(c_line) + ": Expected end of file, but got token " + std::to_string(c_type));
    std::cout << std::endl << "Syntax and Semantic Analysis Successful!" << std::endl;
}

void Parser::parseProgram() {
    if (c_type != LEX_PROGRAM) throw std::runtime_error("Syntax Error on line " + std::to_string(c_line) + ": Program must start with 'program'");
    gl();
    if (c_type != LEX_LFIG) throw std::runtime_error("Syntax Error on line " + std::to_string(c_line) + ": Expected '{' after 'program'");
    gl();
    parseDeclarations();
    parseStatementsList();
    if (c_type != LEX_RFIG) throw std::runtime_error("Syntax Error on line " + std::to_string(c_line) + ": Expected '}' at the end of program");
    gl();
}

void Parser::parseDeclarations() {
    while (c_type == LEX_INT || c_type == LEX_STRING || c_type == LEX_BOOLEAN) {
        parseDeclaration();
        if (c_type != LEX_SEMICOLON) throw std::runtime_error("Syntax Error on line " + std::to_string(c_line) + ": Expected ';' after declaration");
        gl();
    }
}

void Parser::parseDeclaration() {
    type_of_lex current_decl_type = c_type;
    gl();
    parseVariable(current_decl_type);
    while (c_type == LEX_COMMA) {
        gl();
        parseVariable(current_decl_type);
    }
}

void Parser::parseVariable(type_of_lex var_type) {
    if (c_type != LEX_ID) throw std::runtime_error("Syntax Error on line " + std::to_string(c_line) + ": Expected identifier in declaration");
    int current_id_index = c_val;
    gl();
    if (c_type == LEX_ASSIGN) {
        gl();
        parseExpression();
        type_of_lex init_expr_type;
        from_st(st_lex, init_expr_type);
        checkTypesCompatible(var_type, init_expr_type, true);
        checkAndDeclareId(current_id_index, var_type, true);
    } else {
        checkAndDeclareId(current_id_index, var_type);
    }
}

void Parser::checkAndDeclareId(int id_index, type_of_lex var_type, bool initial_assign) {
    if (id_index < 0 || id_index >= static_cast<int>(TID.size())) throw std::runtime_error("Internal Error: Invalid identifier index.");
    if (TID[id_index].get_declare()) throw std::runtime_error("Semantic Error: Variable '" + TID[id_index].get_name() + "' declared twice on line " + std::to_string(c_line));
    TID[id_index].put_declare();
    TID[id_index].put_type(var_type);
    if (initial_assign) {
        TID[id_index].put_assign();
    }
}

void Parser::parseOperatorsBlock() {
    if (c_type != LEX_LFIG) throw std::runtime_error("Syntax Error on line " + std::to_string(c_line) + ": Expected '{' to start a block");
    gl();
    parseStatementsList();
    if (c_type != LEX_RFIG) throw std::runtime_error("Syntax Error on line " + std::to_string(c_line) + ": Expected '}' to end a block");
    gl();
}

void Parser::parseStatementsList() {
    while (c_type != LEX_RFIG) {
            if (c_type == LEX_SEMICOLON) { throw std::runtime_error("Syntax Error on line " + std::to_string(c_line) + ": Empty statement not allowed"); }
        parseStatement();
        if (c_type != LEX_SEMICOLON) {
                if (c_type == LEX_RFIG) break;
                throw std::runtime_error("Syntax Error on line " + std::to_string(c_line) + ": Expected ';' after statement");
        }
        gl();
    }
}

void Parser::parseStatement() {
    if (c_type == LEX_IF) { parseIfStatement(); }
    else if (c_type == LEX_WHILE) { parseWhileStatement(); }
    else if (c_type == LEX_READ) { parseReadStatement(); }
    else if (c_type == LEX_WRITE) { parseWriteStatement(); }
    else if (c_type == LEX_BREAK) { parseBreakStatement(); }
    else if (c_type == LEX_LFIG) { parseOperatorsBlock(); }
    else if (c_type == LEX_ID || c_type == LEX_NUM || c_type == LEX_STR ||
             c_type == LEX_TRUE || c_type == LEX_FALSE || c_type == LEX_NOT ||
             c_type == LEX_LPAREN)
    {
        parseExpression();
        if (!st_lex.empty()) { st_lex.pop(); }
        else { throw std::runtime_error("Internal Error: Expression stack empty after parsing expression statement at line " + std::to_string(c_line)); }
    }
    else { throw std::runtime_error("Syntax Error on line " + std::to_string(c_line) + ": Unexpected token to start a statement: " + std::to_string(c_type)); }
}

void Parser::parseIfStatement() {
    int line = c_line;
    gl();
    if (c_type != LEX_LPAREN) throw std::runtime_error("Syntax Error on line " + std::to_string(line) + ": Expected '(' after 'if'");
    gl();
    parseExpression();
    checkConditionType();
    if (c_type != LEX_RPAREN) throw std::runtime_error("Syntax Error on line " + std::to_string(c_line) + ": Expected ')' after if condition");
    gl();
    parseStatement();
    if (c_type == LEX_ELSE) {
        gl();
        parseStatement();
    }
}

void Parser::parseWhileStatement() {
    int line = c_line;
    gl();
    if (c_type != LEX_LPAREN) throw std::runtime_error("Syntax Error on line " + std::to_string(line) + ": Expected '(' after 'while'");
    gl();
    parseExpression();
    checkConditionType();
    if (c_type != LEX_RPAREN) throw std::runtime_error("Syntax Error on line " + std::to_string(c_line) + ": Expected ')' after while condition");
    gl();

    loop_nesting_stack.push(true);
    parseStatement();
    if (!loop_nesting_stack.empty()) loop_nesting_stack.pop();
}

void Parser::parseReadStatement() {
    int line = c_line;
    gl();
    if (c_type != LEX_LPAREN) throw std::runtime_error("Syntax Error on line " + std::to_string(line) + ": Expected '(' after 'read'");
    gl();
    if (c_type != LEX_ID) throw std::runtime_error("Syntax Error on line " + std::to_string(c_line) + ": Expected identifier after 'read('");
    checkReadId();
    gl();
    if (c_type != LEX_RPAREN) throw std::runtime_error("Syntax Error on line " + std::to_string(c_line) + ": Expected ')' after identifier in 'read'");
    gl();
}

void Parser::parseWriteStatement() {
    int line = c_line;
    gl();
    if (c_type != LEX_LPAREN) throw std::runtime_error("Syntax Error on line " + std::to_string(line) + ": Expected '(' after 'write'");
    gl();
    parseExpression();
    while (c_type == LEX_COMMA) {
        gl();
        parseExpression();
    }
    if (c_type != LEX_RPAREN) throw std::runtime_error("Syntax Error on line " + std::to_string(c_line) + ": Expected ')' or ',' in 'write'");
    gl();
}

void Parser::parseBreakStatement() {
    int line = c_line;
    gl();
    if (loop_nesting_stack.empty() || !loop_nesting_stack.top()) {
            throw std::runtime_error("Semantic Error on line " + std::to_string(line) + ": 'break' statement not within a 'while' loop");
    }
}

void Parser::parseExpression() {
    parseTermAnd();
    while (c_type == LEX_OR) {
        st_lex.push(c_type); gl(); parseTermAnd(); checkOp();
    }
}
void Parser::parseTermAnd() {
    parseTermCompare();
    while (c_type == LEX_AND) {
        st_lex.push(c_type); gl(); parseTermCompare(); checkOp();
    }
}
void Parser::parseTermCompare() {
    parseTermAdd();
    if (c_type == LEX_EQ || c_type == LEX_NEQ || c_type == LEX_LSS ||
        c_type == LEX_GTR || c_type == LEX_LEQ || c_type == LEX_GEQ) {
        st_lex.push(c_type); gl(); parseTermAdd(); checkOp();
    }
}
void Parser::parseTermAdd() {
    parseTermMul();
    while (c_type == LEX_PLUS || c_type == LEX_MINUS) {
        st_lex.push(c_type); gl(); parseTermMul(); checkOp();
    }
}
void Parser::parseTermMul() {
    parseAssignment();
    while (c_type == LEX_TIMES || c_type == LEX_SLASH || c_type == LEX_PERCENT) {
        st_lex.push(c_type); gl(); parseAssignment(); checkOp();
    }
}

void Parser::parseAssignment() {
    int left_id_index = parseFactor();

    if (c_type == LEX_ASSIGN) {
        if (left_id_index == -1) {
            throw std::runtime_error("Semantic Error on line " + std::to_string(c_line) + ": Invalid left-hand side (lvalue) for assignment.");
        }

        type_of_lex t_left;
        from_st(st_lex, t_left);

        gl();
        parseAssignment();

        type_of_lex t_right;
        from_st(st_lex, t_right);

        checkTypesCompatible(t_left, t_right, true);

        TID[left_id_index].put_assign();

        st_lex.push(t_left);
    }
}

int Parser::parseFactor() {
    int id_index = -1;

    if (c_type == LEX_ID) {
        id_index = c_val;
        checkIdDeclared(id_index);
        st_lex.push(getIdType(id_index));
        gl();
    } else if (c_type == LEX_NUM) {
        st_lex.push(LEX_INT);
        gl();
    } else if (c_type == LEX_STR) {
        st_lex.push(LEX_STRING);
        gl();
    } else if (c_type == LEX_TRUE) {
        st_lex.push(LEX_BOOLEAN);
        gl();
    } else if (c_type == LEX_FALSE) {
        st_lex.push(LEX_BOOLEAN);
        gl();
    } else if (c_type == LEX_NOT) {
        gl();
        id_index = parseFactor();
        if (id_index != -1) id_index = -1;
        checkNot();
    } else if (c_type == LEX_LPAREN) {
        gl();
        parseExpression();
        if (c_type != LEX_RPAREN) throw std::runtime_error("Syntax Error on line " + std::to_string(c_line) + ": Expected ')'");
        gl();
        id_index = -1;
    } else {
        throw std::runtime_error("Syntax Error on line " + std::to_string(c_line) + ": Unexpected token in expression factor: " + std::to_string(c_type));
    }
    return id_index;
}

void Parser::checkIdDeclared(int id_index) {
    if (id_index < 0 || id_index >= static_cast<int>(TID.size()) || !TID[id_index].get_declare()) {
        std::string name = (id_index >= 0 && id_index < static_cast<int>(TID.size())) ? TID[id_index].get_name() : "index " + std::to_string(id_index);
        throw std::runtime_error("Semantic Error on line " + std::to_string(c_line) + ": Identifier '" + name + "' not declared.");
    }
}
type_of_lex Parser::getIdType(int id_index) {
        if (id_index < 0 || id_index >= static_cast<int>(TID.size()) || !TID[id_index].get_declare()) { throw std::runtime_error("Internal Error: Attempt to get type of undeclared ID."); }
        return TID[id_index].get_type();
}
void Parser::checkTypesCompatible(type_of_lex t1, type_of_lex t2, bool assignment) {
    if (t1 == t2) return;
    std::string error_msg = assignment ? " Incompatible types in assignment." : ": Incompatible types for operation.";
    auto type_to_str = [](type_of_lex t) -> std::string { if (t == LEX_INT) return "int"; if (t == LEX_STRING) return "string"; if (t == LEX_BOOLEAN) return "boolean"; return "unknown(" + std::to_string(t) + ")"; };
    throw std::runtime_error("Semantic Error on line " + std::to_string(c_line) + error_msg + " Required: " + type_to_str(t1) + ", Got: " + type_to_str(t2));
}
void Parser::checkOp() {
    type_of_lex t1, t2, op, result_type = LEX_NULL;
    bool types_ok = false;
    from_st ( st_lex, t2 );
    if (st_lex.empty()) throw std::runtime_error("Internal Error: Stack error during operator type check (op)."); from_st ( st_lex, op );
    if (st_lex.empty()) throw std::runtime_error("Internal Error: Stack error during operator type check (t1)."); from_st ( st_lex, t1 );

    switch (op) {
        case LEX_PLUS: if (t1 == LEX_INT && t2 == LEX_INT) { result_type = LEX_INT; types_ok = true; } else if (t1 == LEX_STRING && t2 == LEX_STRING) { result_type = LEX_STRING; types_ok = true; } break;
        case LEX_MINUS: case LEX_TIMES: case LEX_SLASH: case LEX_PERCENT: if (t1 == LEX_INT && t2 == LEX_INT) { result_type = LEX_INT; types_ok = true; } break;
        case LEX_EQ: case LEX_NEQ: if (t1 == t2 && (t1 == LEX_INT || t1 == LEX_STRING || t1 == LEX_BOOLEAN)) { result_type = LEX_BOOLEAN; types_ok = true; } break;
        case LEX_LSS: case LEX_GTR: case LEX_LEQ: case LEX_GEQ: if (t1 == t2 && (t1 == LEX_INT || t1 == LEX_STRING)) { result_type = LEX_BOOLEAN; types_ok = true; } break;
        case LEX_AND: case LEX_OR: if (t1 == LEX_BOOLEAN && t2 == LEX_BOOLEAN) { result_type = LEX_BOOLEAN; types_ok = true; } break;
        default: throw std::runtime_error("Internal Error: Unknown binary operation " + std::to_string(op));
    }
    if (types_ok) { st_lex.push(result_type); }
    else { throw std::runtime_error("Semantic Error on line " + std::to_string(c_line) + ": Incompatible types (" + std::to_string(t1) + ", " + std::to_string(t2) + ") for operation " + std::to_string(op)); }
}
void Parser::checkNot() {
    if (st_lex.top() != LEX_BOOLEAN) throw std::runtime_error("Semantic Error on line " + std::to_string(c_line) + ": Operand for 'not' must be boolean.");
}
void Parser::checkConditionType() {
    if (st_lex.empty()) throw std::runtime_error("Internal Error: Stack empty before checking condition type.");
    if (st_lex.top() != LEX_BOOLEAN) throw std::runtime_error("Semantic Error on line " + std::to_string(c_line) + ": Conditional expression must be boolean.");
    st_lex.pop();
}
void Parser::checkReadId() {
    int id_index = c_val;
    checkIdDeclared(id_index);
    type_of_lex id_type = getIdType(id_index);
    if (id_type == LEX_BOOLEAN) { throw std::runtime_error("Semantic Error on line " + std::to_string(c_line) + ": Cannot read into a boolean variable ('" + TID[id_index].get_name() + "')."); }
    if (id_index >= 0 && id_index < static_cast<int>(TID.size())) { TID[id_index].put_assign(); }
}

std::ostream &operator<<( std::ostream &s, const Lex& l ) {
    type_of_lex t = l.t_lex;
    s << "L" << l.line_lex << ": ";

    switch(t) {
        case LEX_PROGRAM: s << "program"; break; case LEX_INT: s << "int"; break; case LEX_STRING: s << "string"; break;
        case LEX_BOOLEAN: s << "boolean"; break; case LEX_IF: s << "if"; break; case LEX_ELSE: s << "else"; break;
        case LEX_WHILE: s << "while"; break; case LEX_READ: s << "read"; break; case LEX_WRITE: s << "write"; break;
        case LEX_TRUE: s << "true"; break; case LEX_FALSE: s << "false"; break; case LEX_BREAK: s << "break"; break;
        case LEX_NOT: s << "not"; break; case LEX_AND: s << "and"; break; case LEX_OR: s << "or"; break;
        case LEX_LPAREN: s << "("; break; case LEX_RPAREN: s << ")"; break; case LEX_LFIG: s << "{"; break; case LEX_RFIG: s << "}"; break;
        case LEX_SEMICOLON: s << ";"; break; case LEX_COMMA: s << ","; break; case LEX_ASSIGN: s << "="; break;
        case LEX_PLUS: s << "+"; break; case LEX_MINUS: s << "-"; break; case LEX_TIMES: s << "*"; break;
        case LEX_SLASH: s << "/"; break; case LEX_PERCENT: s << "%"; break; case LEX_EQ: s << "=="; break;
        case LEX_LSS: s << "<"; break; case LEX_GTR: s << ">"; break; case LEX_LEQ: s << "<="; break;
        case LEX_GEQ: s << ">="; break; case LEX_NEQ: s << "!="; break;
        case LEX_NUM: s << "NUM(" << l.v_lex << ")"; break; case LEX_STR: s << "STR(\"" << l.s_lex << "\")"; break;
        case LEX_ID: if (l.v_lex >= 0 && l.v_lex < static_cast<int>(TID.size())) s << "ID(" << TID[l.v_lex].get_name() << ")"; else s << "ID(Invalid:" << l.v_lex << ")"; break;
        case LEX_FIN: s << "FIN"; break; default: s << "Unknown(" << static_cast<int>(t) << ")"; break;
    }
    s << std::endl;
    return s;
}

int main () {
    try {
        Parser P("a.txt");
        P.analyze();
        return 0;

    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "An unexpected error occurred" << std::endl;
        return 1;
    }
}
