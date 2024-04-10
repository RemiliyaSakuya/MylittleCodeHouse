#include <sys/wait.h>
#include "common.hpp"
#include "log.hpp"
using namespace std;



static void GetMessage(int fd)
{
    //使用一个字符串数组接收信息
    char buffer[SIZE];
    while(true)
    {
        //初始化buffer
        memset(buffer,'\0',sizeof(buffer));

        ssize_t s = read(fd,buffer,sizeof(buffer)-1);//读取的字节不需要包含'\0'所以在sizeof后面-1
        if(s > 0)
        {
            //大于0就是读到管道文件中的数据了
            cout << "[" << getpid() << "]" << "client say " << buffer <<endl;
        }
        else if(s == 0)
        {
            //s==0就是读完文件了,结束
            //将信息打印到stderro
            cerr << "[" << getpid() << "]" << "read end of file, clien quit, server quit too!" << endl;
            break;
        }
        else
        {
            //错误
            perror("read");
            break;
        }
    }
}
int main()
{
    //命名管道，就是两个没有血缘关系的进程，能够通过命名管道进行通信交流

    //1.创建命名管道
    //利用函数mkfifo(管道路径+文件名,权限数)创建命名管道,创建成功返回0,失败返回-1
    if(mkfifo(ipcPath.c_str(),MODE))
    {
        //创建命名管道
        perror("mkfifo");
        exit(1);
    }
    //打印日志信息
    Log("创建管道文件成功", Debug) << " step 1" << endl;

    //2.正常的文件操作
    //这里的fd是管道的文件描述符
    int fd = open(ipcPath.c_str(),O_RDONLY);    //只加一个O_RDONLY就会阻塞等待
    if(fd < 0)
    {
        perror("open");
        exit(2);
    }
     Log("打开管道文件成功", Debug) << " step 2" << endl;
    //这里我们可以使用多个进程去接受单个进程发出的信息,这里我们使用3个进程去接受
    int nums =3 ;
    for(int i = 0;i < nums;i++)
    {
        pid_t id = fork();
        if(id == 0)
        {
            //子进程去接受,3个进程去抢,谁抢到就是谁的
            
            //进程从fd也就是管道文件中读数据
            GetMessage(fd);
            exit(1);//读完后退出
        }
    }

    //father,父进程需要等待子进程结束
    for(int i=0;i<nums;i++)
    {
        //-1为等待所有任意子进程
        waitpid(-1,nullptr,0);
    }

    //4.关闭文件fd
    close(fd);
    Log("关闭管道文件成功", Debug) << " step 3" << endl;

    //使用unlink函数将管道文件删除
    unlink(ipcPath.c_str());
    Log("删除管道文件成功", Debug) << " step 4" << endl;
    return 0;
} 