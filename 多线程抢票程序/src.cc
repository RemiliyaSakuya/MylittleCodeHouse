#include <iostream>
#include <unistd.h>
#include <string>
#include <cstring>
#include <pthread.h>
using namespace std;

//总票数，全局变量
//为了使共享资源不产生相互影响的重入问题，使用加锁进行解决
//当前使用宏定义对锁进行初始化
pthread_mutex_t mutex= PTHREAD_MUTEX_INITIALIZER;
int tickets = 1000;
void* Ticketsbook(void* args)
{
    //三个线程重入的函数
    while(true)
    {
        //A.以下代码要进行加锁
        pthread_mutex_lock(&mutex);//加锁
        if(tickets <=0)
        {
            pthread_mutex_unlock(&mutex);
            break;
        }
        else
        {
            usleep(10000);
            cout << "tid:" << pthread_self() << "   " << "剩余的tickets:" << --tickets << endl;
            //B.执行完后对临界区进行解锁
            pthread_mutex_unlock(&mutex);
        } 
    }
}


int main()
{
//1.主线程
//建立3个线程
pthread_t t1,t2,t3;
pthread_create(&t1,nullptr,Ticketsbook,nullptr);
pthread_create(&t3,nullptr,Ticketsbook,nullptr);
pthread_create(&t3,nullptr,Ticketsbook,nullptr);

//回收线程
pthread_join(t1,nullptr);
pthread_join(t2,nullptr);
pthread_join(t3,nullptr);


return 0;
}