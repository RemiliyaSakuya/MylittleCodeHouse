#include <iostream>
#include "BlockingQueue.hpp"
#include "Task.hpp"
#include <pthread.h>
#include <unistd.h>
#include <ctime>
using namespace std;

//假定我们要设置一个两数相加的Task分发给生产者和消费者
int MyAdd(int x,int y)
{
    return x+y;
}


void* Producer(void* args)
{
    BlockingQueue<Task>* bq = (BlockingQueue<Task>*) args;
    //生产者进行活动
    //int cnt=1;
    while(true)
    {
        //产生随机数让生产者不断生产任务
        int x = rand() % 20+1;
        usleep(rand()%1000);
        int y = rand() % 10+1;
        Task t(x,y,MyAdd);
        bq->push(t);
        cout << "生产两数据相加:"<<t.x_<<"+"<<t.y_<<"=?"<<endl;
        sleep(1);
    }
    return nullptr;
}

void* Consumer(void* args)
{
    BlockingQueue<Task>* bq = (BlockingQueue<Task>*) args;
    while(true)
    {
        Task t;
        bq->pop(&t);
        cout << "处理两个数的和:"<<t.x_<<"+"<<t.y_<<"="<<t()<<endl;
        //sleep(1);
    }
    return nullptr;
}
int main()
{
    //创建c,p消费者和生产者线程
    srand(getpid()^(INT64_MAX)^0x12345);

    pthread_t c[2],p[2];
    BlockingQueue<int>* bq= new BlockingQueue<int>();
    pthread_create(c,nullptr,Consumer,bq);
    pthread_create(c+1,nullptr,Consumer,bq);

    pthread_create(p,nullptr,Producer,bq);
    pthread_create(p+1,nullptr,Producer,bq);




    pthread_join(c[0],nullptr);
    pthread_join(c[1],nullptr);

    pthread_join(p[0],nullptr);
    pthread_join(p[1],nullptr);

    delete bq;
}