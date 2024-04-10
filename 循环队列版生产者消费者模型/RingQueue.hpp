#pragma once
#ifndef _RING_QUEUE_HPP_
#define _RING_QUEUE_HPP_
#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <vector>
#include "Sem.hpp"
using namespace std;

const int g_default_num = 5;    //定义全局num用于指定循环队列的大小

template<class T>
class RQ
{
public:
    RQ(const int default_num = g_default_num)
                :rq_(g_default_num),
                 capacity_(g_default_num),
                 spacesem_(g_default_num),   //空间资源的信号量初始化就用队列的大小表示
                 datasem_(0),
                 cstep_(0),
                 pstep_(0)

    {
        pthread_mutex_init(&cmutex_,nullptr);
        pthread_mutex_init(&pmutex_,nullptr);
    }
    ~RQ()
    {
        pthread_mutex_destroy(&cmutex_);
        pthread_mutex_destroy(&pmutex_);

    }
    void push(const T& in)
    {
        //生产者push时,首先申请空间资源信号量
        //空间资源信号量执行p操作
        spacesem_.p();
        pthread_mutex_lock(&pmutex_);
        //因为是循坏队列，插入数据时要在数据的"当前队尾插入"(队头队尾会变化)
        //所以还需要一个标识队头队尾的下标
        rq_[pstep_++] = in;
        pstep_ %= capacity_; //push完后要保证下标在队中循环
        //push完成后,数据资源信号量执行v操作
        datasem_.v();
        pthread_mutex_unlock(&pmutex_);
    }
    void pop(T* out)
    {
        //消费者拿取数据时，要对数据资源信号量申请
        //数据资源信号量执行p操作
        datasem_.p();
        pthread_mutex_lock(&cmutex_);
        //将当前队头的数据取出来
        *out = rq_[cstep_++];
        //取完数据后保证下标在队列中循环
        cstep_ %= capacity_;
        //空间资源信号量执行v操作
        spacesem_.v();
        pthread_mutex_unlock(&cmutex_);
        
    }
private:
    vector<T> rq_;             //我们使用数组实现循环队列
    int capacity_;             //循环队列的大小
    Sem spacesem_;             //临界资源的空间资源的信号量
    Sem datasem_;              //临界资源的数据资源的信号量
    int cstep_;                //标识消费者应该取数据的位置，即队头下标
    int pstep_;                //标识生产者应该放数据的位置, 即队尾下标
    //新加:如果我们要实现多生产者和多消费者，如何实现?
    //多消费者与多生产者,本质没有脱离单生产单消费的思路，
    //只是有多个生产者同时接任务，多个消费者同时处理任务
    //但是，仍然只能一次只有一个生产者和一个消费者在临界区里
    //所以相比单生产和单消费的：生产/消费的关系
    //多生产和多消费多出了：生产/生产，消费/消费两种新关系
    //如何维持这两种新关系就是多生产和多消费的本质
    //只需要在单生产和单消费的基础上控制生产和消费对临界区的进入即可
    //而循环队列中生产者和消费者进入临界区的媒介就是下标
    //所以只需要定义两个锁，分别锁住生产者和消费者即可
    pthread_mutex_t cmutex_;  //消费者的共享锁
    pthread_mutex_t pmutex_;  //生产者的共享锁
};

#endif