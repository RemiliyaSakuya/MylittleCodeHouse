#pragma once
#include <iostream>
#include <queue>
#include <pthread.h>
using namespace std;
const int gDefalutCapacity = 10;

//利用模板定义一个阻塞队列用于实现生产者消费者模型
template<class T>
class BlockingQueue
{
public:
    BlockingQueue(int capacity = gDefalutCapacity)
                :capacity_(capacity)
    {
        pthread_mutex_init(&mutex_,nullptr);
        pthread_cond_init(&full_,nullptr);
        pthread_cond_init(&empty_,nullptr);
    }
    ~BlockingQueue()
    {
        pthread_mutex_destroy(&mutex_);
        pthread_cond_destroy(&full_);
        pthread_cond_destroy(&empty_);

    }
    bool IsEmpty()
    {
        return bq_.empty();
    }
    bool IsFull()
    {
        return bq_.size()==capacity_;
    }
    //生产者入队
    void push(const T& in)
    {
        pthread_mutex_lock(&mutex_);
        //1.检测临界资源是否为满
        while(IsFull())
        {
            //如果队列是满的就让它挂起阻塞
            //pthread_cond_wait：我们调用wait是在临界区中的，但是这时我们还持有锁，锁会去哪里？
            //调用了wait之后，pthread_cond_wait第二个参数锁的作用，就是释放锁
            //我们被唤醒时从哪醒来？在哪里被挂起阻塞，就在哪里醒来,被唤醒时依旧是在临界区中被唤醒
            //在被唤醒后pthread_cond_wait会自动为我们再次获取锁
            pthread_cond_wait(&full_,&mutex_);
        }
        //2.访问临界资源
        bq_.push(in);
        //作为生产者，我们生产完一个商品进到队列中后，就可以通知消费者进行消费了
        pthread_cond_signal(&empty_);
        pthread_mutex_unlock(&mutex_);
    }
    //消费者出队
    void pop(T* out)
    {
        pthread_mutex_lock(&mutex_);
        //先检测队列是否为空
        while(IsEmpty())
        {
            pthread_cond_wait(&empty_,&mutex_);
        }
        *out = bq_.front();
        bq_.pop();
        //作为消费者，当我们消费取走一个商品过后，就可以通知生产者继续生产了，故在这里唤醒生产者
        pthread_cond_signal(&full_);
        pthread_mutex_unlock(&mutex_);
    }
private:
    queue<T> bq_;                   //阻塞队列
    int capacity_;                  //队列容量
    pthread_mutex_t mutex_;         //公用锁
    pthread_cond_t full_;           //判断队列是否满的条件变量
    pthread_cond_t empty_;           //判断队列是否空的条件变量
};

