#include <iostream>
#include <string>
class Person {
    public: 
        virtual std::string getName() const = 0;
        virtual ~Person() {} // Virtual destructor

};

class Student: public Person {
private:
    std::string name;
    int grade;

public:
    // Default constructor
    Student() : name(""), grade(0) {}

    Student(const std::string& name, int grade) : name(name), grade(grade) {}

    std::string getName() const { return name; }
    int getGrade() const { return grade; }

    friend std::ostream& operator<<(std::ostream& os, const Student& student) {
        std :: cout << student.name << " (average grade: " << student.grade << ")";
        return os;
    }
};

class Classroom {
private:
    std::string className;
    int capacity ;
    Student* students;
    int studentCount;
    std::string teacherName;
    
    
    static int classroomCount;
    
public:
    // Constructor
    Classroom(const std::string& className, int capacity, const std::string& teacherName) :
            className(className), capacity(capacity), studentCount(0), teacherName(teacherName) {
        students = new Student[capacity];
        classroomCount++;
    }

    // Copy constructor
    Classroom(const Classroom& other) : className(other.className), capacity(other.capacity), studentCount(other.studentCount), teacherName(other.teacherName) {
        students = new Student[capacity];
        for (int i = 0; i < studentCount; ++i) {
            students[i] = other.students[i];
        }
        classroomCount++;
    }

    // Destructor
    ~Classroom() {
        delete[] students;
        classroomCount--;
    }

    // Assignment operator
    Classroom& operator=(const Classroom& other) {
        if (this != &other) {
            delete[] students;
            className = other.className;
            capacity = other.capacity;
            studentCount = other.studentCount;
            teacherName = other.teacherName;
            students = new Student[capacity];
            for (int i = 0; i < studentCount; ++i) {
                students[i] = other.students[i];
            }
        }
        return *this;
    }

    // Add student
    void addStudent(const Student& student) {
        if (studentCount < capacity) {
            students[studentCount++] = student;
        } else {
            std::cout << "You can't add another student because you overcome the capacity" << std::endl << std::endl;
        }
    }

    // Access student by index
    const Student& operator[](int index) const {
        if (index >= 0 && index < studentCount) {
            return students[index];
        } else {
            std::cout << "Index out of range" << std::endl;
            std::terminate();
        }
    }

    // Get classroom count
    static int getClassroomCount() {
        return classroomCount;
    }

    // Calculate average grade
    double getAverageGrade() const {
        if (studentCount == 0) {
            return 0.0;
        }
        double sum = 0;
        for (int i = 0; i < studentCount; ++i) {
            sum += students[i].getGrade();
        }
        return sum / studentCount;
    }

    // Output 
    friend std::ostream& operator<<(std::ostream& os, const Classroom& classroom) {
        std :: cout << "Class: " << classroom.className << std::endl;
        std :: cout << "Teacher: " << classroom.teacherName << std::endl;
        std :: cout << "Students:" << std::endl;
        for (int i = 0; i < classroom.studentCount; ++i) {
            std :: cout<< "  - " << classroom.students[i] << std::endl;
        }
        std :: cout << "Average grade: " << classroom.getAverageGrade() << std::endl;
        return os;
    }

};

int Classroom::classroomCount = 0;

int main() {
    Classroom classA("1", 2, "T");
    classA.addStudent(Student("A", 5));
    classA.addStudent(Student("B", 4));

    std::cout << classA << std::endl;

    Classroom classB("2", 1, "k");
    classB.addStudent(Student("C", 3));
    std::cout << "Overloading of []:(outpur the first student of b class) " << classB[0] << std::endl;
    classB = classA; // Copy constructor
    classB.addStudent(Student("C", 3));

    std::cout << classB << std::endl;

    std::cout << "Classroom count: " << Classroom::getClassroomCount() << std::endl;

    return 0;
}   
