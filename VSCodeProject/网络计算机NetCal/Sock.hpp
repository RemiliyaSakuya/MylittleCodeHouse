#pragma once 
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <cstring>
class Sock
{
private:
    const static int gbacklog = 20;
public:
    Sock()
    {}

    //创建套接字
    int Socket()
    {
        //创建套接字后返回给对应端口
        int listenSock=socket(AF_INET,SOCK_STREAM,0);
        if(listen < 0)
        {
            std::cout << "创建listenSock失败" <<std::endl;
            exit(2);
        }
        std::cout << "创建listenSock成功"<<std::endl;
        return listenSock;
    }

    void Bind(int sock,std::string ip,uint16_t port)
    {
        //根据服务器端传进来的sock,ip,port进行绑定
        struct sockaddr_in local;
        local.sin_family = AF_INET;
        local.sin_port = htons(port);
        inet_pton(AF_INET,ip.c_str(),&local.sin_addr);
        if(bind(sock,(struct sockaddr*)&local,sizeof(local)) < 0)
        {
            std::cout << "绑定失败" <<std::endl;
            exit(3);
        }
        std::cout << "绑定成功" <<std::endl;
    }

    //将当前服务端转成监听状态
    void Listen(int sock)
    {
        if(listen(sock,gbacklog) < 0 )
        {
            std::cout << "监听创建失败" << std:: endl;
            exit(4);
        }
        std::cout << "监听创建成功" <<std::endl;
    }



    int Accpet(int listensock,std::string *ip, uint16_t *port)
    {
        //将当前服务端转成准备等待连接的状态
        struct sockaddr_in src;
        socklen_t len = sizeof(src);
        int serverSock = accept(listensock,(struct sockaddr*)&src,&len);
        if(serverSock < 0)
        {
            std::cout << "创建serverSock失败" <<std::endl;
            return -1;
        }
        if(port) *port = ntohs(src.sin_port);
        if(ip) *ip = inet_ntoa(src.sin_addr);
        return serverSock;
    }

    //客户端的connect链接
    bool Connect(int sock,std::string ip,uint16_t port)
    {
        //将对应服务器的ip,port传进来使用connect将其链接起来
        struct sockaddr_in server;
        bzero(&server,sizeof(server));
        server.sin_family = AF_INET;
        server.sin_port = htons(port);
        inet_pton(AF_INET,ip.c_str(),&server.sin_addr);
        if(connect(sock,(struct sockaddr*)&server,sizeof(server)) < 0)
        {
            std::cout << "Connect失败" << std::endl;
            return false;
        }
        std::cout << "Connect成功"<< std::endl;
        return true;
    }


    ~Sock()
    {}

};