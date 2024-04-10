#ifndef _UDP_SERVER_HPP_
#define _UDP_SERVER_HPP_
#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
class UdpServer
{
public:
    //构造函数初始化Server的port和ip
    UdpServer(uint16_t port,std::string ip = "")
            :_ip(ip),_port(port)
    {}
    void Init()
    {
        //初始化套接字
        //1.利用系统调用，创建套接字
        _sock = socket(AF_INET,SOCK_DGRAM,0);
        if(_sock < 0)
        {
            std::cout << "创建套接字失败" << std::endl;
            exit(2);
        }
    }

    void start()
    {
        //开始网络编程
        //2.创建完套接字后，开始bind绑定，将IP和端口与当前进程强关联
        //A:bind函数需要用到一个中的sockaddr结构体将IP和端口写入进去
        struct sockaddr_in local;
        //B:初始化local
        bzero(&local,sizeof(local));
        //C:将IP和端口号写进local
        local.sin_family = AF_INET;
        
        //注意：我们现在_ip中存放的地址是点分制十进制风格的IP地址
        //而系统是使用4字节风格IP地址进行存放的，所以需要转化一下
        //转化过程:本机点分十进制=>4字节=>网络序列
        //新：由于我们之前写的服务器IP绑定是指定的一个IP，但通常
        //我们不建议使用固定IP而是随机IP，所以我们将固定IP修改为随机IP
        local.sin_addr.s_addr = _ip.empty() ? INADDR_ANY : inet_addr(_ip.c_str());
        
        //注意：因为网络中对端口号的识别必须是大段，所以我们必须确保
        //_port要变成大端的存储形式再写入local中
        local.sin_port = htons(_port);

        //D:bind
        if(bind(_sock,(struct sockaddr*)&local,sizeof(local))<0)
        {
            std::cout << "绑定失败" <<std::endl;
            exit(2);
        }
        

        //3.开始运行服务器
        //保证死循环，使服务器不停止工作
        char buff[1024];
        while(true)
        {
            //1.读取client发过来的数据
            struct sockaddr_in peer;
            socklen_t addrlen = sizeof(peer);
            ssize_t s = recvfrom(_sock,buff,sizeof(buff)-1,0,(struct sockaddr*)&peer,&addrlen);
            if(s > 0)
            {
                //s返回的是收到的数据大小
                //将client的IP和端口提取出来，就可以看到是谁发信息过来了
                buff[s] = 0;
                uint16_t cli_port = ntohs(peer.sin_port);
                //ip是4字节的风格，需要转换为本主机序列、点分十进制的风格方便查看
                std::string cli_ip = inet_ntoa(peer.sin_addr);
                std::cout << "["<<cli_ip<<":"<<cli_port<<"]"<<"# "<<buff<<std::endl;
            }
            //处理数据：TODO
            //2.接收完消息后对client写回数据
            sendto(_sock,buff,strlen(buff),0,(struct sockaddr*)&peer,addrlen);
        }
    }
    ~UdpServer()
    {
        //退出服务器时关闭_sock
        if(_sock >= 0)
        {
            close(_sock);
        }
    }

private:
    int _sock;
    std::string _ip;
    uint16_t _port;
};






#endif
