#ifndef _SEM_HPP_
#define _SEM_HPP_
#include <iostream>
#include <pthread.h>
#include <semaphore.h>
using namespace std;


//我们封装循环队列的信号量
class Sem
{
public:
    Sem(int value)
    {
        sem_init(&sem_,0,value);
    }
    ~Sem()
    {
        sem_destroy(&sem_);
    }
    void p()
    {
        //p操作,即对信号量--
        sem_wait(&sem_);
    }
    void v()
    {
        //v操作，即对信号量进行++
        sem_post(&sem_);
    }
private:
    sem_t sem_;
};

#endif