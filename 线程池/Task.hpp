//这里单独写一个Task类来为生产者和消费者线程提供任务数据
#pragma once
#include <iostream>
#include <pthread.h>
#include <functional>
#include <string>
using namespace std;

typedef function<int(int,int)> func_t;  //利用一个function将func_t定义为一个函数类型
class Task
{
public:
    Task()
    {}
    Task(int x,int y,func_t func):x_(x),y_(y),func_(func)
    {}
    void operator()(const string& name)
    {
        //将Task类重载()来实现仿函数
        cout << "成功处理任务: "<<"线程:"<< name << ":" << x_ << "+" << y_ << "=" << func_(x_,y_)<<endl; 
    }
public:
    int x_;
    int y_;
    func_t func_;
};