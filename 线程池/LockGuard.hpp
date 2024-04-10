#pragma once
#include <iostream>
#include <pthread.h>
using namespace std;
//这里用于封装锁,让代码更优雅

class Mutex
{
public:
    Mutex(pthread_mutex_t* pmutex):pmutex_(pmutex)
    {}
    void lock()
    {
        cout << "要进行加锁" << endl;
        pthread_mutex_lock(pmutex_);
    }
    void unlock()
    {
        cout <<"要进行解锁" << endl;
        pthread_mutex_unlock(pmutex_);
    }
    ~Mutex()
    {}
private:
    pthread_mutex_t* pmutex_;
};


//RAII加锁风格,就是将锁的加解锁使用类封装起来，在后续调用的时候只要调用一个加锁接口后
//类在析构时自动去调用锁的销毁

class LockGuard
{
public:
    LockGuard(pthread_mutex_t* mutex):mutex_(mutex)
    {
        mutex_.lock();
    }
    ~LockGuard()
    {
        mutex_.unlock();
    }
private:
    Mutex mutex_;
};