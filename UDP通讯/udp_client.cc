#include "udp_server.hpp"


static void Usage(std::string proc)
{
    std::cout << "\nUsage: " << proc << " ip port\n" << std::endl;
} 
//客户端这边启动时同样需要指定IP和端口(有小问题)，所以先用main函数接收
//命令行参数
int main(int argc,char* argv[])
{
    if(argc != 3)
    {
        Usage(argv[0]);
        exit(1);
    }

    //要启动客户端，步骤和服务端一样
    //1.创建套接字
    int _sock = socket(AF_INET,SOCK_DGRAM,0);
    if(_sock < 0)
    {
        std::cout << "创建套接字失败" <<std::endl;
        exit(2);
    }

    std::string message;
    struct sockaddr_in server;
    bzero(&server,sizeof(server));
    socklen_t len = sizeof(server);
    char buff[1024];

    //同样是将要发送的服务器的IP和端口赋值给temp
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_port = htons(atoi(argv[2]));
    while(true)
    {
        //作为客户端，我们只需要不断去发送信息到服务器就可以了
        //2.发送信息
        std::cout << "输入要发送的消息:" <<std::endl;
        getline(std::cin,message);
        //向服务器发送消息
        //至于我们为什么不写bind绑定，是因为作为客户端，系统在我们第一次
        //发送消息到服务器时，就会自动随机生成ip和port并绑定，不需要手动写
        sendto(_sock,message.c_str(),message.size(),0,(struct sockaddr*)&server,len);

        //读取服务器写回的数据
        struct sockaddr_in temp;
        socklen_t tmlen = sizeof(temp);
        ssize_t s =recvfrom(_sock,buff,sizeof(buff),0,(struct sockaddr*)&temp,&tmlen);
        if(s > 0)
        {
            std::cout << "server echo#" <<std::endl;
        }
    }
    return 0;
}