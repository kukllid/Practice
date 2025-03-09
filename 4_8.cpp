#include <iostream>
using namespace std;

class A {
    int i;
   public:
    A(int x) { i = x; cout << "first" << endl; }
    virtual ~A() { cout << "second" << endl; }
    int f() const { return i + g() + h(); }
    virtual int g() const { return i; }
    int h() const { return 39; }
   };
   class B : public A {
   public:
    B() : A(70) { cout <<"third" << endl; }
    ~B() { cout << "fourth" << endl; }
    int f() const { return g() - 2; }
    virtual int g() const { return 4; }
    int h() const { return 6; }
   };
   int main() {
    B b;//происходит создание конструктора B() по умолчанию, но до этого вызывается конструктор класса A
            //first
            //third
    A* p = &b; //указатель на класс A, который указывает на объект класса B
    cout << "result = (" << p->f() <<';'<< b.f() << ')' << endl;
    //тк f - не виртуальная функция, вызывается A::f(); i = 70
    //тк g - виртуальная вызывается B::g() и возвращает 4
    //h не виртуальная вызывается A::h() и возвращает 39
    //i + g() + h() = 70 + 4 + 39 = 113

    //b.f() вызывает B::f()
    //B::f() вызывает B::g() (возвращает 4)
    //B::f() возвращает 2
            //result = (113;2)
    
    return 0;
    //срабатывает деструктор ~B
            //fourth
    //срабатывает деструктор ~A
            //second
   }
