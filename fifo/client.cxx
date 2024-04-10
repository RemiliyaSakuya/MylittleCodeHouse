#include "common.hpp"

using namespace std;

//当前进程client负责写入数据让server读
int main()
{
    //第一步永远是打开管道文件,要让两个程序之间进行通信必须要让两个进程看到同一份资源

    //1.以写方式打开管道文件
    int fd = open(ipcPath.c_str(),O_WRONLY);

    if(fd < 0)
    {
        perror("open");
        exit(1);
    }

    //2.ipc过程，也就是写入数据
    string buffer;
    while(true)
    {
        std::cout << "Please Enter Message Line :> ";
        std::getline(std::cin,buffer);
        write(fd,buffer.c_str(),buffer.size());
    }

    //3.关闭
    close(fd);
    return 0;
}