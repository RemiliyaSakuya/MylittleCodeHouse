#include <iostream>
#include <unistd.h>
#include <sys/wait.h>

using namespace std;
 int main()
 {
    execlp("ls","-a","-l",nullptr);
    cout << "我是原程序,本该要打印我" << endl;
    return 0;
 }