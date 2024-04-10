#include "udp_server.hpp"
#include <memory>

static void Usage(std::string proc)
{
    std::cout << "\nUsage: " << proc << " port\n" << std::endl;
} 
//因为我们从命令行开始让udp_server运行，需要手动输入IP、端口
//所以需要在main函数上使用参数将ip和端口用参数接过来
int main(int argc,char* argv[])
{
    //命令行需指定输入：./udp_server "xx.xx.xx.xx" "xxxx"
    if(argc != 2)
    {
        Usage(argv[0]);
        exit(1);
    }


    //接收命令行传过来的IP和端口号
    //std::string ip = argv[1];
    //注意命令行中输入的端口号是字符串的形式，所以需要将它转成整数
    uint16_t port = atoi(argv[1]);
    //使用一个智能指针指向一个UdpServer
    std::unique_ptr<UdpServer> svr(new UdpServer(port));
    svr->Init();
    svr->start();
    return 0;
}