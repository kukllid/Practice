#include <iostream>
using namespace std;
struct K {
    virtual void add_st ( K * n ) {
     st ++; cout << "add_st (K*) from K" << endl;
     }
    static int st;
    };
    struct L: K {
    virtual void add_st ( L * a ) { 
     st++; cout << "add_st (L*) from L" << endl;
    }
};
int K::st = 2;
int main () {
L ob, ob2;
K k, *pl = &ob;
pl -> add_st (& ob2); //add_st - виртуальная функция => вызывает K::add_st(K* n) и увеличивает st на 1
        //add_st (K*) from K
k.st ++; //st = 4
++ob.st ; //st = 5
cout << k.st << ' '<< ob.st << ' ' << K::st << endl;
        //5 5 5
 return 0;
}
