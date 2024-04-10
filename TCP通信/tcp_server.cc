#include "tcp_server.hpp"
#include <memory>
int main(int agrc,char* agrv[])
{
    if(agrc != 2)
    {
        std::cout << "Usage:  port" << std::endl;
        exit(1);
    }

    uint16_t serverport = atoi(agrv[1]);
    std::unique_ptr<TcpServer> svr(new TcpServer(serverport));
    svr->InitServer();
    svr->start();
}