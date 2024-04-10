#include <iostream>
#include <sys/wait.h>
#include <unistd.h>

using namespace std;

int main()
{
    //创建子进程
    pid_t id = fork();

    if(id == 0)
    {
        //子进程
        cout << "i am child process" << endl;
        sleep(5);
    }
    else if(id < 0)
    {
        perror("fork");
    }
    else if( id > 0)
    {
        //父进程
        int status = 0;
        pid_t pid = waitpid(-1,&status,0);
        int sign = WEXITSTATUS(status);
        cout << "我是父进程，子进程退出码为:" << sign << endl;
        cout << "我是父进程，子进程core dump为:" << ((status >> 7)&1) << endl;
        
    }

    return 0;
}