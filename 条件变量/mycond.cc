#include <iostream>
#include <string>
#include <pthread.h>
#include <unistd.h>
using namespace std;

//定义有多少个线程
#define PTNUMS 4
volatile bool quit = false; //用volatile定义一个标志位来让多线程判断退出标准
//定义一个函数指针用于存放给线程调用的函数
typedef void (*func_t)(const string& name,pthread_mutex_t* mutex,pthread_cond_t* cond);
//利用一个类将每一个线程封装起来，里面存储他们的回调函数，和线程名字
class PthreadData
{
public:
    //在条件变量版本中新增cond,mutex
    PthreadData(const string& name,func_t func,pthread_mutex_t* mutex,pthread_cond_t* cond)
                :name(name),
                func(func),
                pmutex_(mutex),
                pcond_(cond)
    {
        
    }
public:
    //新增的pmutex_和pcond_用于接收条件变量和锁
    pthread_mutex_t* pmutex_;
    pthread_cond_t* pcond_;
    string name;
    func_t func;
};

void func_1(const string& name,pthread_mutex_t* mutex,pthread_cond_t* cond)
{
    while(!quit)
    {
    //在每个方法前加上条件变量的等待
        //当代码执行到这一行代码时会立即被挂起即阻塞等待
    //必须确保wait在加锁和解锁之间
    pthread_mutex_lock(mutex);
    pthread_cond_wait(cond,mutex);
    cout << name << "running-- 下载" << endl;
    pthread_mutex_unlock(mutex);
    }
    
}

void func_2(const string& name,pthread_mutex_t* mutex,pthread_cond_t* cond)
{
    while(!quit)
    {
        //当代码执行到这一行代码时会立即被挂起即阻塞等待
        pthread_mutex_lock(mutex);
        pthread_cond_wait(cond,mutex);

    cout << name << "running-- 播放" << endl;
    pthread_mutex_unlock(mutex);
    }
}

void func_3(const string& name,pthread_mutex_t* mutex,pthread_cond_t* cond)
{
    while(!quit)
    {
        //当代码执行到这一行代码时会立即被挂起即阻塞等待
    pthread_mutex_lock(mutex);
    pthread_cond_wait(cond,mutex);

    cout << name << "running-- 刷新" << endl;
    pthread_mutex_unlock(mutex);
    }
}

void func_4(const string& name,pthread_mutex_t* mutex,pthread_cond_t* cond)
{
    while(!quit)
    {
        //当代码执行到这一行代码时会立即被挂起即阻塞等待
    pthread_mutex_lock(mutex);
    pthread_cond_wait(cond,mutex);

    cout << name << "running-- 扫码信息用户" << endl;
    pthread_mutex_unlock(mutex);
    }
}

void* Entrance(void* args)
{
    //此为创建线程时,每个线程调用的函数
    PthreadData* pt = (PthreadData*) args;  //pt 在每个线程自己内部私有的栈中存储
    //将类作为参数传给回调函数,调用类里面的函数来实现不同线程执行不同函数
    pt->func(pt->name,pt->pmutex_,pt->pcond_);
    delete pt;
    return nullptr;
}
int main()
{
    //如要用到条件变量，用于解决其线程不断自我检测的问题，强制使进程阻塞等待
    //定义出公用锁
    pthread_mutex_t mutex;
    //定义条件变量
    pthread_cond_t cond;
    //初始化条件变量和锁
    pthread_mutex_init(&mutex,nullptr);
    pthread_cond_init(&cond,nullptr);
    //利用一个数组将多线程存储起来
    pthread_t pt[PTNUMS];
    //创建函数指针数组将多个函数存起来
    func_t Func[PTNUMS]={func_1,func_2,func_3,func_4};
    for(int i=0;i<PTNUMS;i++)
    {
        string name = "pthread ";
        name = name + to_string(i+1);
        //将每一个线程都创建一个类将他们封装
        PthreadData* ptd = new PthreadData(name,Func[i],&mutex,&cond);
        //创建多个线程
        pthread_create(pt+i,nullptr,Entrance,(void*)ptd);
    }    
    //当用到了条件变量的接口，就需要对其进行唤醒
    //假定主线程在10次后退出多线程的所有活动
    sleep(5);
    int cnt = 10;
    while(cnt)
    {
        cout << "resume pthread: " << cnt-- <<endl;
        //pthread_cond_signal的接口是让条件变量按照队列一个个有序唤醒
        //pthread_cond_brodcast的接口是让条件变量一口气全部唤醒
        pthread_cond_broadcast(&cond);
        //pthread_cond_signal(&cond);
        sleep(1);
    }
    quit = true;
    //在这里再唤醒一次来确保线程最后去被检测一次
    pthread_cond_broadcast(&cond);
    //回收线程
    for(int i=0;i<PTNUMS;i++)
    {
        pthread_join(pt[i],nullptr);
        cout << "ptread " << pt[i] << " Exit" << endl;
    }
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    return 0;
}