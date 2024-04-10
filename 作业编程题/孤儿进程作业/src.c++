#include <iostream>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
using namespace std;
int main()
{
    pid_t id = fork();
    if(id < 0)  //创建失败
    {
        perror("fork");
        return 1;
    }
    else if(id == 0)
    {
        //子进程
        cout << "i am child pid:" << getpid() <<endl;
        while(1);
    }
    else
    {
        //父进程
        cout << "i am parent pid:" << getpid() <<endl;
        sleep(3);
        exit(0);
    }
    return 0;
}