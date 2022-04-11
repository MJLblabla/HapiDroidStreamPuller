//
// Created by 1 on 2022/3/27.
//

#ifndef UNTITLED2_B_H
#define UNTITLED2_B_H
#include "A.h"
class B : public A<int> {

protected:
    void v1(int a) override{
        f2(a);
    }

public:
    B(){}

    ~B(){}
};
#endif //UNTITLED2_B_H
