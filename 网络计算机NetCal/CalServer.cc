#include "Sock.hpp"
#include "CalServer.hpp"
#include <memory>
#include "Protocol.hpp"
using namespace ns_tcpServer;
using namespace ns_Protocol;

Respone CalculateHelp(Request& req)
{
    Respone res(0,0);
    //判断计算符号
    switch(req._op)
    {
        case '+':
            res._result = req._x + req._y;
            break;
        case '-':
            res._result = req._x - req._y;
            break;
        case '*':
            res._result = req._x * req._y;
            break;
        case '/':
            //除法需要判断，我们这里定义_y为被除数，不能为0
            if(req._y == 0)
                res._code = 1;
            else
                res._result = req._x / req._y;
            break;
        case '%':
            //模同样道理
            if(req._y == 0)
                res._code = 2;
            else
                res._result = req._x / req._y;
            break;
        default:
                res._code = 3;
            break;
        
    }
    return res;
}


void Calculate(int sock)
{
    while(true)
    {
        std::string str = Recv(sock);
        Request req;
        req.DeSerialize(str);

        Respone res = CalculateHelp(req);

        std::string respstr = res.Serialize();  //序列化结果返回给客户端

        Send(sock,respstr);
    }
}


int main(int argc,char* argv[])
{

    //main函数传参使用接收ip和port
    if(argc != 2)
    {
        std::cout << "\nUseage: " <<" port\n" <<std::endl;
        exit(1);
    }
    uint16_t serverPort = ntohs(atoi(argv[1]));

    //使用一个智指针创建一个Server对象
    std::unique_ptr<TcpServer> pServer(new TcpServer(serverPort));
    pServer->
    pServer->Start();

    return 0;
}