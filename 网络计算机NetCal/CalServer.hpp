#pragma once
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include "Sock.hpp"
#include <functional>
#include <pthread.h>
#include <vector>
//用命名空间将类保护起来
namespace ns_tcpServer
{
    using func_t = std::function<void(int)>;
    // 服务器类

    class TcpServer;

    //用于封装线程需要处理的数据
    class ThreadData
    {
    public:
        //因为我们
        ThreadData(int serverSock,TcpServer* server):_serverSock(serverSock),_server(server)
        {}
        ~ThreadData()
        {}
    public:
        int _serverSock;
        TcpServer* _server;
    };



    class TcpServer
    {
    private:
        static void* ThreadRoutine(void* args)
        {
            ThreadData* td = (ThreadData*)args;
        }
    public:
        TcpServer(uint16_t port,std::string ip = "0.0.0.0" ) // 将ip默认设置0.0.0.0服务器接收任意一个传来的IP地址
            : _ip(ip),
              _port(port)
        {
            // 在构造函数内做完 1.创建套接字 2.bind 3.listen
            listenSock = _sock.Socket();
            _sock.Bind(listenSock, ip, port);
            _sock.Listen(listenSock);
        }


        //在sock,bind,listen都建好后，开始将Server转成accept状态接收链接
        //所以我们在TcpServer中编写Server执行的任务

        void Start()
        {
            //Server端开始运行,在这个函数中不断循环
            while(true)
            {
                //1.服务端先接收客户端的连接
                std::string clientIp;
                uint16_t clientPort;
                int serviceSock= _sock.Accpet(listenSock,&clientIp,&clientPort);

                if(serviceSock < 0)
                {
                    //如果连接失败，则回头重新继续尝试连接
                    continue;
                }


                //到这里表示成功连接到了客户端
                //服务端这边需要根据请求，将客户端发过来的请求进行解决
                //以当前的服务端是网络计算机，所以我们按照自己编写的协议处理客户端发过来的信息

                //1.首先我们不需要主线程在这里处理任务导致没有其他线程去连接其他客户端,所以我们这里采用多线程方案
                //用一个新线程处理任务,主线程回去继续连接客户端
                pthread_t tid;
                
                //由于我们需要处理客户端传过来的信息，我们使用一个结构将需要处理的信息包装起来
                ThreadData* td = new ThreadData(serviceSock,this);
                pthread_create(&tid,nullptr,ThreadRoutine,td);

            }
        }

        void BindServer(func_t func)
        {
            funcv.push_back(func);
        }

        void Excute(int sock)
        {
            for(auto& f : funcv)
            {
                f(sock);
            }
        }
    private:
        std::string _ip; // 服务器的ip
        uint16_t _port;  // 服务器端口
        Sock _sock;      // 将类Sock放进一个Server的成员中，用于调用封装各种网络TCP链接接口
        int listenSock; //服务器的监听套接字
        std::vector<func_t> funcv;  //用存储任务的数组
    };

}