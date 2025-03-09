
#include <iostream>
using namespace std;

class T {
    public:
     virtual int f (int x) {
     cout << "T::f" << endl; 
     return 0;
     }
     void g () {
     f (1); 
     cout << "T::g" << endl; 
     }
     virtual void h () { 
     g ();
     cout << "T::h" << endl; 
     }
};

class S: public T { 
public:
 int f (double y){ 
    cout << "S::f" << endl; 
    return 2; 
    }
    virtual void g () {
    f (1); 
    cout << "S::g" << endl;
    }
    virtual void h () {
    g(); 
    cout << "S::h" << endl; 
 }
};
int main(){
T t; S s; T *p = &s; // p - указатель на T, &s - адрес к переменной класса S
p -> f (1.5); // печатается T::f, т к у T* преимущество, как у статической переменной
p -> g (); //точно как и в прошлом печатается 
        //T::H
        //T::g
p -> h (); //полиморфизм, тк h - виртуальная функция; S::h вызывает S::g; S::g вызывает S::f(1) и S::f(1) выводит все нужное
//S::f
//S::h
//S::h
}
