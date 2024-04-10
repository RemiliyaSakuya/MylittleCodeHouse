#pragma once
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <cstring>
#include <unistd.h>
#include <signal.h>

static void serviceread(int sock,std::string cli_ip,uint16_t cli_port)
{
    char buff[1024];
    while(true)
    {
        ssize_t s = read(sock,buff,sizeof(buff)-1);
        if(s > 0)
        {
            buff[s] = 0;
            std::cout << cli_ip<<":"<<cli_port<<"# "<< buff << std::endl;
        }
        else if(s == 0)
        {
            //s == 0代表对方已经关闭
            std::cout << cli_ip<<":"<<cli_port<<" " << "shut down,me too!"<<std::endl;
            break;
        }
        else
        {
            std::cout << "read错误" << std::endl;
        }
    }
    write(sock,buff,strlen(buff));
}

class TcpServer
{
private:
    const static int backlog = 20;
public:
    TcpServer(uint16_t port,std::string ip = ""):_port(port),_ip(ip)
    {}
    void InitServer()
    {
        //1.创建套接字
        //第一个sock是一个监听套接字,负责将底层链接提取上来
        listensock = socket(AF_INET,SOCK_STREAM,0);
        if(listensock < 0)
        {
            std::cout << "创建套接字失败" <<std::endl;
            exit(2);
        }
        //2.开始bind
        //A:创建sockaddr_in结构体将family、ip、port赋值进去
        struct sockaddr_in local;
        socklen_t len = sizeof(local);
        bzero(&local,sizeof(local));

        local.sin_family = AF_INET;
        local.sin_addr.s_addr = _ip.empty() ? INADDR_ANY : inet_addr(_ip.c_str());
        local.sin_port = htons(_port);

        //B:将结构体绑定本机进程
        if(bind(listensock,(struct sockaddr*)&local,len) < 0)
        {
            std::cout << "绑定失败"<<std::endl;
            exit(2);
        }
    ////////////////////////////////////////////////////////////
        //3.这里开始TCP和UDP有区别了，TCP是面向连接的，所以当我们正式通信时，要先
        //建立连接，处于监听状态
        //因为当前只有一个进程，在一个客户端连入时，当前服务器进程只会循环第一个客户端
        //如果第一个客户端不退出，那么会导致服务器无法去连接一个新的客户端
        if(listen(listensock,backlog) < 0)
        {
            std::cout << "监听失败" <<std::endl;
            exit(3);
        }
    }
    void start()
    {
        //当子进程退出时，我们设置父进程忽略子进程发出的SIGCHLD，那么子进程就会自动回收资源
        signal(SIGCHLD,SIG_IGN);
        //4.获取连接
        //accpet返回的是一个文件描述符,其作用与listensock一样,但是serversock才是用于网络服务的套接字
        while(true)
        {
            struct sockaddr_in src;
            socklen_t srclen = sizeof(src);
            bzero(&src,sizeof(src));
            int serversock = accept(listensock,(struct sockaddr*)&src,&srclen);
            if(serversock < 0)
            {
                std::cout << "获取连接失败,重新获取" <<std::endl;
                continue;
            }
            //成功获取连接
            //5.获取发送方的IP和port
            std::string cli_ip=inet_ntoa(src.sin_addr);
            uint16_t cli_port = ntohs(src.sin_port);
            std::cout << "Link Success,serversock:"<<serversock<<"|"<< cli_ip<<":"<<cli_port<<std::endl;

            //6.开始读数据
            //在UDP中我们读数据是用recvfrom读取的，但在TCP中我们的sock可以看作是一个文件描述符
            //所以我们可以使用read和write函数进行读与写
            //根据前面说到单进程只能处理一个客户端，所以需要fork
            //serviceread(serversock,cli_ip,cli_port);
            pid_t t = fork();
            if(t == 0)
            {
                //这里是子进程
                //子进程负责处理服务
                close(listensock);
                serviceread(serversock,cli_ip,cli_port);
                exit(0);
            }
            //这里父进程就可以继续回去连接新客户端
            close(serversock);
            
        }

       
    }
    ~TcpServer()
    {
        if(listensock > 0)
        {
            close(listensock);
        }
    }
private:
    int listensock;
    std::string _ip;
    uint16_t _port;
};