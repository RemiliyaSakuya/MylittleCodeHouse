#include "RingQueue.hpp"
#include "Sem.hpp"
#include <cstdlib>
#include <unistd.h>
#include <sys/time.h>
void* Consumer(void* args)
{
    RQ<int>* rq =  (RQ<int>*) args;
    while(true)
    {
        //消费者拿资源
        int x;
        rq->pop(&x);
        cout << "消费者拿取资源："<< x << endl;
        sleep(1);
    }
}

void* Producer(void* args)
{
    RQ<int>* rq =  (RQ<int>*) args;
    while(true)
    {
        //生产者生产资源
        int y = rand() % 20;
        rq->push(y);
        cout << "生产者生产数据：" << y << endl;
    }
}

int main()
{
    //定义初始化生产者、消费者线程
    srand((uint64_t)time(nullptr) ^ getpid());
    pthread_t p[2],c[2];
    RQ<int>* rq = new RQ<int>();
    //新加：当我们要实现多生产者和多消费者该如何实现？
    pthread_create(p,nullptr,Producer,(void*) rq);
    pthread_create(p+1,nullptr,Producer,(void*) rq);

    pthread_create(c,nullptr,Consumer,(void*) rq);
    pthread_create(c+1,nullptr,Consumer,(void*) rq);


    pthread_join(p[0],nullptr);
    pthread_join(p[1],nullptr);

    pthread_join(c[0],nullptr);
    pthread_join(c[1],nullptr);

    return 0;
}