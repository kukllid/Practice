#include <iostream>
#include <algorithm>

class rectangle {
private:
    int x1, y1, x2, y2;

public:
    //Задание прямоугольника через левый нижний и правый верхний
    rectangle(int x1 = 0, int y1 = 0, int x2 = 1, int y2 = 1) : x1(x1), y1(y1), x2(x2), y2(y2) {}

    //Перемещение по x и y
    void move(int dx, int dy) {
        x1 += dx;
        y1 += dy;
        x2 += dx;
        y2 += dy;
    }
    //Задание новой длины и ширины
    void resize(int nwidth, int nheight) {
        x2 = x1 + nwidth;
        y2 = y1 + nheight;
    }

    //Наименьшее содержащее оба прямоугольника
    rectangle smallest(const rectangle& other) const {
        return rectangle(std::min(x1, other.x1), std::min(y1, other.y1),
                         std::max(x2, other.x2), std::max(y2, other.y2));
    }
    //наименьшее пересечение
    rectangle intersecion(const rectangle& other) const {
        int x1intersec = std::max(x1, other.x1);
        int y1intersec = std::max(y1, other.y1);
        int x2intersec = std::min(x2, other.x2);
        int y2intersec = std::min(y2, other.y2);
        if (x1intersec < x2intersec && y1intersec < y2intersec) {
            return rectangle(x1intersec, y1intersec, x2intersec, y2intersec);
        } else {
            return rectangle(0, 0, 0, 0); //пересечения не нашлось
        }
    }

    //вывод левого ижнего и правого верхнего угла
    void print() const {
        if (x1 == 0 && x2 == 0 && y1 == 0 && y2 ==0){
            std::cout << "No intersection" << std::endl << std::endl;
        }
        else{
            std::cout << "The left lower corner of rectangle: (" << x1 << ", " << y1 << ")" << std::endl << "The right upper corner: (" << x2 << ", " << y2 << ")" << std::endl << std::endl;
        }
    }
};

int main() {
    int x1, y1, x2, y2, height1, height2, width1, width2;
    std::cout << "Enter the left lower corner of the first rectangle as x and y coordinates(int):" << std::endl;
    std::cin >> x1 >> y1;
    std::cout << std::endl;

    std::cout << "Enter the height of the first rectangle:" << std::endl;
    std::cin >> height1;
    std::cout << std::endl;

    std::cout << "Enter the width of the first rectangle:" << std::endl;
    std::cin >> width1;
    std::cout << std::endl;

    //создаем прямоугольники
    rectangle r1(x1, y1, x1 + width1, y1 + height1);
    std::cout << "Rectangle 1:" << std::endl;
    r1.print();


    std::cout << "Enter the left lower corner of the second rectangle as x and y coordinates(int):" << std::endl;
    std::cin >> x2 >> y2;
    std::cout << std::endl;

    std::cout << "Enter the height of the second rectangle:" << std::endl;
    std::cin >> height2;
    std::cout << std::endl;

    std::cout << "Enter the width of the second rectangle:" << std::endl;
    std::cin >> width2;
    std::cout << std::endl;
    
    rectangle r2(x2, y2, x2 + width2, y2 + height2);

    std::cout << "Rectangle 2:" << std::endl;
    r2.print();

    
    std::cout << "Smallest containing rectangle:" << std::endl;
    r1.smallest(r2).print(); // Вызываем метод у r1, передавая r2

    std::cout << "Interssection of rectangles:" << std::endl;
    r1.intersecion(r2).print(); // Вызываем метод у r1, передавая r2

    std::cout << "Enter the x and y coordinate you want to move the first rectangle:" << std::endl;
    int x3, y3;
    std::cin >> x3 >> y3;
    std::cout << std::endl;
    r1.move(x3, y3);
    std::cout << "Rectangle 1 after moving:" << std::endl;
    r1.print();
    
    std::cout << "Enter new width and height for second rectangle:" << std::endl;
    int x4, y4;
    std::cin >> x4 >> y4;
    std::cout << std::endl;
    r2.resize(x4, y4);
    std::cout << "Rectangle 2 after resizing:" << std::endl;
    r2.print();

    return 0;
}
