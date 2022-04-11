//
// Created by 1 on 2022/3/27.
//

#ifndef UNTITLED2_A_H
#define UNTITLED2_A_H

#include <iostream>
template<class T>
class A {

private:
    int cc=0;
protected:
    virtual void v1(int a) = 0;

public:
    A(){}
    ~A(){}
    void f1(T t){}
    void f2(T t){}

};


#endif //UNTITLED2_A_H
