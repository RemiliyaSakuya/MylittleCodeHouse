#include <iostream>
#include <stdlib.h>
#include <pthread.h>
#include <string>
#include <cstring>
#include <time.h>
#include <unistd.h>
using namespace std;


//使用宏定义指定线程个数
#define PTHREAD_NUMS 5
int tickets = 10000;
//使用一个类将线程的名字和锁封装
class pthread
{
public:
    pthread(string& name,pthread_mutex_t* mt)
                :ptname(name),
                 mutex(mt)
    {
        //构造函数
    }
public:
    //线程名字
    string ptname;
    //线程公用锁
    pthread_mutex_t* mutex;
};
//线程调用的抢票函数
void* Ticketsbook(void* args)
{
    //强转参数
    pthread* t = (pthread*)args;
    while(true)
    {
        usleep(rand()%1000);
        //加锁
        pthread_mutex_lock(t->mutex);
        //开始抢票,tickets不为0则继续抢
        if(tickets > 0)
        {
            cout << t->ptname.c_str()<< "  "<< "剩余票数:" << --tickets << endl;
            pthread_mutex_unlock(t->mutex);
        }
        else
        {
            //解锁
            pthread_mutex_unlock(t->mutex);
            break;
        }
        pthread_mutex_unlock(t->mutex);
    }
    delete t;
    return nullptr;
}

int main()
{
    srand((unsigned long)time(nullptr) ^ getpid() ^ 0x147);
    //使用数组存储线程
    pthread_t pt[PTHREAD_NUMS];
    //1.初始化线程、加锁
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex,nullptr);
    for(int i=0;i<PTHREAD_NUMS;i++)
    {
        string pname="thread ";
        pname += to_string(i+1);
        //将每一个线程使用类封装
        pthread* t=new pthread(pname,&mutex);
        //将类传给回调函数
        pthread_create(pt+i,nullptr,Ticketsbook,(void*)t);
    }
    //2.回收线程
    for(int i=0;i<PTHREAD_NUMS;i++)
    {
        pthread_join(pt[i],nullptr);
    }
    //回收锁
    pthread_mutex_destroy(&mutex);
    return 0;
}