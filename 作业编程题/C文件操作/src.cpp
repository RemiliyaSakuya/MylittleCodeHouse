#include <iostream>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/fcntl.h>
#include <string.h>
using namespace std;
 int main()
 {
    FILE* fp = fopen("bite","w+");
    char tar[] = "linux so easy!";
    fwrite(tar,strlen(tar)+1,1,fp);
    fseek(fp, 0, SEEK_SET);
    char buffer[1024]={0};
    fread(buffer,1,strlen(tar)+1,fp);
    cout << buffer <<endl;
    fclose(fp);
    return 0;
 }