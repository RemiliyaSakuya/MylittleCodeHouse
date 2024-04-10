#include "Thread.hpp"
#include "ThreadPool.hpp"
#include "Task.hpp"
#include <unistd.h>
int main()
{
    srand((uint64_t)time(nullptr) ^ getpid());
    ThreadPool<Task>::getThreadPool()->run();
    while(true)
    {
        int x = rand() % 100 + 1;
        int y = rand() % 100 + 1;
        //用主线程派发任务进taskq_
        //拉姆达函数(省去单独构建函数的过程)
        cout << "生产一个任务：" << x << "+" << y << "=?" << endl;
        Task t(x,y,[](int x,int y)->int{return x+y;});
        ThreadPool<Task>::getThreadPool()->push_task(t);
        sleep(1);
    }
    return 0;
}