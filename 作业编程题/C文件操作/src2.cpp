#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <string.h>
using namespace std;

int main()
{
    //打开文件
    int fd=open("bite",O_CREAT | O_WRONLY | O_TRUNC,0777);
    if(fd < 0 )
    {
        perror("open");
        return 1;
    }
    const char* buff="i like linux!\n";
    ssize_t n = write(fd,buff,strlen(buff));
    char ret[1024];
    ssize_t N = read(fd,ret,n);
    cout << ret <<endl;
    close(fd);
    return 0;
}