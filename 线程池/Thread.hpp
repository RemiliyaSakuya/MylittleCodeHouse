#pragma once
#include <iostream>
#include <pthread.h>
#include <string>
#include <cstdio>
#include <functional>
using namespace std;

typedef void *(*fun_t)(void *);

class ThreadData
{
public:
    std:: string name_;
    void* args_;
};

//这里是定义封装每一个线程的类
class Thread
{
public:
    Thread(int num,fun_t callback,void* args):func_(callback)
    {
        //传进来的num是当前线程的编号
        char thread_name[64];
        snprintf(thread_name,sizeof(thread_name),"Thraed-%d",num);
        name_=thread_name;
        tdata_.args_=args;
        tdata_.name_=name_;
    }
    void start()
    {
        pthread_create(&tid_,nullptr,func_,(void*)&tdata_);
    }
    void join()
    {
        pthread_join(tid_,nullptr);
    }
private:
    std::string name_;           //线程名字
    pthread_t tid_;             //线程ID
    fun_t func_;                //回调函数
    ThreadData tdata_;          //线程数据封装
};