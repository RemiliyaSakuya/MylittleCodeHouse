#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#define NUM 1024
#define SIZE 32
#define SEP " "
//保存打散后的命令行字符串
char* g_argv[SIZE];
//保存原本的命令行字符串
char cmd_line[NUM];

int main()
{
    //shell运行原理:通过让子进程执行命令，让父进程等待&&解析命令
    //0.命令行解释，一定是一个常驻内存的进程不退出
    while(1)
    {
       //1.打印出提示信息[HML@VM-8-15-centos myshell]#
       printf("[HML@VM-8-15-centos myshell]#");
       //在语言的特性,printf是将括号内的内容放到缓冲区,遇到了"\n"才会打印出来,所以如果要提前打印,就需要冲刷缓冲区
       fflush(stdout);
       memset(cmd_line,'\0',sizeof cmd_line);

       //2.获取用户的键盘输入[输入的是各种指令和选项："ls -a -l -i"]
       if(fgets(cmd_line, sizeof cmd_line, stdin) == NULL)
       {
           continue;
       }
       //这时,cmd_line中不单单记录了用户输入的指令字符串,还记录了输入的回车键,所以需要将回车取消
       cmd_line[strlen(cmd_line)-1]='\0';
       //将 "ls -a -l -i\n\0" ==>  "ls -a -l -i\0\0"
       
       //3.命令行字符串解析："ls -a -l -i" -> "ls" "-a" "-l" "-i"
       g_argv[0]=strtok(cmd_line,SEP);  //strtok第一次调用要传入原始字符串
       int index =1;
       while(g_argv[index++]=strtok(NULL,SEP));//第二次,如果还要解析原始字符串,传入NULL

       //4.TODO内置命令，即让父进程(shell)自己执行的命令，我们叫做内置命令，内建命令
       //内建命令本质就是shell自己的一个函数调用
       if(strcmp(g_argv[0],"cd") == 0)  //不创建子进程,让父进程执行
       {
          if(g_argv[1] != NULL)
          {
              chdir(g_argv[1]);
          }
          //无论是否切换了目录,都不需要再往下跑了
          continue;
       }
       //5.fork()
       pid_t id = fork();
       if(id ==  0) //child
       {
           printf("下面代码是子进程跑的\n");
           execvp(g_argv[0],g_argv);//ls -a -l -i
           exit(1);
       }
       //father
       int status=0;
       pid_t ret = waitpid(id,&status,0);
       if(ret >0)
           printf("exit code :%d",WEXITSTATUS(status));
    }
    return 0;
}
