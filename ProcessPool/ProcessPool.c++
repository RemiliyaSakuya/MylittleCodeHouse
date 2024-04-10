#include <iostream>
#include <vector>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <cassert>
#include <assert.h>
#include "Task.hpp"
//首先先定义要有多少个子程序,
#define PROCESS_NUM 5
using namespace std;

int WaitCommand(int waitFd,bool &quit)
{
    //子进程需要等待父进程对其发出指令
    //而子进程读指令则需要从管道文件pipe[0]中读取,所以我们从pipe[0]中读
    uint32_t command = 0;

    ssize_t s = read(waitFd,&command,sizeof(command));
    //read函数的解释就是从waitFd所指的文件描述符中读数据,读到command中,读sizeof(command)大小
    if(s == 0)
    {
        //s == 0时就是已经读完了文件,可以退出了
        quit = true;
        return -1;
    }
    assert(s == sizeof(uint32_t));//限制s的范围
    return command; //返回读取到的指令
    
}


void sendAndWakeup(pid_t who,int fd,uint32_t command)
{
    write(fd,&command,sizeof(command));
    cout << "main process: call process " << who << " execute " << desc[command] << " through " << fd << endl;
}
int main()
{
    load();
    vector<pair<pid_t,int>> slots;   //该数组用于存储所有子进程的id与其对应的管道文件的fd
    //1.循环创建子进程+管道文件，以PROCESS_NUM为基准
    for(int i=0;i<PROCESS_NUM;i++)
    {
        //对于每一个子进程都要进行建立管道文件的操作
        int pipefd[2] = {0};    //在使用pipe()函数时,它会对一个数组中的两个元素添加两个fd，一个读fd一个写fd,作为管道文件的读fd与写fd
        int n = pipe(pipefd);
        //判断管道文件是否创建成功,n != 0 就是不成功
        assert(n == 0);
        (void) n;   //将n置为void,因为不需要了

        //创建子进程
        pid_t id = fork();
        assert(id != -1);
        
        if(id == 0)
        {
            //此为子进程
            //对于子进程,其功能我们让它读取父进程发出的指令,所以我们对于子进程要关闭它对应管道文件的写端
            close(pipefd[1]);
            while(true)
            {
                //不断的进行读
                //在本次模拟中我们只对子进程发出4字节的数字指令
                bool quit = false;
                int command = WaitCommand(pipefd[0],quit);
                //判断command的合法性
                if(command >= 0 && command < handlerSize())
                {
                    //如果合法就调用command的指令
                    callbacks[command]();
                }
                else
                {
                    cout << "非法command: " << command <<endl;
                }
            }
            exit(1);
        }
        //父进程要写入指令,则关闭读端pipefd[0]
        close(pipefd[0]);
        //对于父进程id会返回它的子进程的pid，所以使用slots数组用于存储子进程的id与其对应的写端
        slots.push_back(pair<pid_t,int>(id,pipefd[1]));
    }
    //father,进程派发任务,这里使用随机负载均衡算法对所有子进程随机派发任务
    srand((unsigned long)time(nullptr) ^ getpid() ^ 23323123123L); //使随机数更加随机
    while(true)
    {
        //选择一个任务
        int command = rand() % handlerSize();

        //选择一个进程,采用随机数的方式,选择进程来完成任务，随机数方式的负载均衡
        int choice = rand() % slots.size();

        //把任务派发给进程
        sendAndWakeup(slots[choice].first,slots[choice].second,command);
        sleep(1);
    }

    //开始推出进程并关闭fd
    for(const auto& slot : slots)
    {
        close(slot.second);
    }

    //等待所有子进程的退出后,父进程才退出
    for(const auto &slot : slots)
    {
        waitpid(slot.first,nullptr,0);
    }

    return 0;
}