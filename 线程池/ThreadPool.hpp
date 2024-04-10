#pragma once
#include "Thread.hpp"
#include <vector>
#include <queue>
#include <unistd.h>
#include "LockGuard.hpp"
#include <semaphore.h>
const int g_ThreadPool_num = 3;
// 定义线程池
using namespace std;
template <class T>

// 新加：将当前线程池改为单例模式(指只能有一个是对象的类),思想将类的构建函数从public转到private,移除拷贝构造、重载
// 以此来无法通过外部直接实例化对象
// 如何解决将类初始化？:通过在类中再定义一个 静态的类对象使其成为全局变量，利用外部初始化，之后去编写一个函数返回这个
// 类的指针，就能够永远保持一个当前类的实例化
class ThreadPool
{
public:
    // 我们定义一个public静态成员函数用于调用private中的ThreadPool构造函数,使用static是为了能够访问pthreadpool这个静态成员
    static ThreadPool<T> *getThreadPool(int num= g_ThreadPool_num)
    {
        // 判断pthreadpool是否为空
        // 问题：当多线程同时发生申请这个单例对象怎么办:加锁控制
        if (pthreadpool == nullptr)
        {
            pthread_mutex_lock(&tmutex);
            if (pthreadpool == nullptr)
            {
                {
                    pthread_mutex_lock(&tmutex);
                    pthreadpool = new ThreadPool<T>(num);
                }
            }
            pthread_mutex_unlock(&tmutex);
        }
        return pthreadpool;
    }

private:
    ThreadPool(int threadpoolnum = g_ThreadPool_num) : nums_(threadpoolnum)
    {
        int i = 1;
        for (i = 1; i <= nums_; i++)
        {
            Thread *t = new Thread(i, routine, this);
            TP_.push_back(t);
        }
        pthread_mutex_init(&mutex_, nullptr);
        pthread_cond_init(&cond_, nullptr);
    }
    ThreadPool(const ThreadPool<T> &TP) = delete;
    const ThreadPool<T> &operator=(const ThreadPool<T> &other) = delete; // 将重载和拷贝构造都给去掉
public:
    void run()
    {
        for (auto &iter : TP_)
        {
            iter->start();
        }
    }
    void push_task(T &task)
    {
        // 开始往任务队列中push任务
        LockGuard lockguard(&mutex_);
        taskq_.push(task);
        // push完后唤醒消费者线程
        pthread_cond_signal(&cond_);
    }
    // 注意：这里有一个小问题，当前routine函数是ThreadPool类中的一个成员函数
    // 只要是类中的成员函数那么就一定会有一个隐藏指针(this指针),所以成员函数
    // 作为线程调用的回调函数在编译的时候会出现类型不匹配的问题
    // 解决方法只需要在函数前面加上static就可以了
    static void *routine(void *args)
    {
        // 这里是每个线程初始化时调用的回调函数,args是Thread中初始化传进来的ThreadData
        ThreadData *tdata = (ThreadData *)args;
        // 到这里就实现了,线程数组中每一个线程都调用这一个routine
        // 这里通过构造新ThreadPool来保存当前this指针
        ThreadPool<T> *tp = (ThreadPool<T> *)tdata->args_;
        while (true)
        {
            T task;
            // 给消费者线程上锁
            // lock
            {
                LockGuard lockguard(&tp->mutex_);
                while (tp->taskq_.empty())
                {
                    // 循环检测
                    pthread_cond_wait(&tp->cond_, &tp->mutex_);
                }
                // 开始取任务
                task = tp->taskq_.front();
                tp->taskq_.pop();
            }
            task(tdata->name_);
            // unlock
            // std:: cout << "我是线程-"<< tdata->name_ << std:: endl;
            // sleep(1);
        }
        // 本质这个函数就是消费过程,但这里有一个问题,当前routine函数被定义为了静态成员函数
        // 其只能调用同为静态类型的成员，所以它无法常规访问内部成员对象
        // 但是我们可以借助外部函数调用this指针来访问类中的非静态成员
    }
    ~ThreadPool()
    {
        for (auto &iter : TP_)
        {
            iter->join();
            delete iter;
        }
        pthread_mutex_destroy(&mutex_);
        pthread_cond_destroy(&cond_);
    }

private:
    std::vector<Thread *> TP_; // 用于存放线程指针的数组
    int nums_;                 // 线程指针数组的大小
    std::queue<T> taskq_;      // 存放派发任务的队列
    pthread_mutex_t mutex_;    // 任务队列中的共享锁
    pthread_cond_t cond_;      // 条件变量
    // 新加：单例化线程池
    static ThreadPool<T> *pthreadpool; // 定义一个当前类的静态成员指针,使这个指针指向一段全局变量
    static pthread_mutex_t tmutex;
};

template <class T>
ThreadPool<T> *ThreadPool<T>::pthreadpool = nullptr; // 用外部初始化静态成员

template <class T>
pthread_mutex_t ThreadPool<T>::tmutex = PTHREAD_MUTEX_INITIALIZER;