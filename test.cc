#include<iostream>
#include <string>
#include <iterator>

using namespace std;

struct Test{
    explicit Test(int b):a(b){}
    Test(const Test& t):a(t.a){}

    int get_test()const{return a;}
    Test plus_(const Test& t)const {return Test(t.get_test()+a);}
    int a;
};

void func(int &&a)
{
    cout<<++a<<endl;
}

void func(Test&& t)
{
    cout<<"test: "<<t.get_test()<<endl;
}

int main()
{
    const Test& t = Test(1);
    cout<<t.get_test()<<endl;
    
    int a =2;
    func(std::move(a));
    cout<<"after steal: "<<a<<endl;
    
    func(Test(100));

    return 0;
}


