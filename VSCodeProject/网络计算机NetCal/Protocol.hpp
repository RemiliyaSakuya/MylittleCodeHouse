#pragma once

#include <iostream>
#include <cstring>
#include <string>
//定义一个客户端和服务器都遵守的协议
namespace ns_Protocol
{



    //定义一个请求类用于封装客户端发过来的请求
    class Request
    {
        #define MYSELF 1
        //协议内容：客户端必须发送一段:数字 '+-*/符号' 数字.这样一段格式的数字
        #define SPACE " "
        #define SPACE_LEN strlen(SPACE)

        public:
            Request()
            {}
            Request(int x,int y,char op):_x(x),_y(y),_op(op)
            {}
            ~Request()
            {}
            
            //序列化
            std::string Serialize()
            {
                //1.自主实现:将格式为：_x op _y 的数据转化为字符串
                #ifdef MYSELF
                //自主实现的
                    std::string str;
                    str = std::to_string(_x);
                    str += SPACE;
                    str += _op;
                    str += SPACE;
                    str += std::to_string(_y);
                    return str;
                #else
                //用别人的
                #endif
            }

            //反序列化
            bool DeSerialize(std::string& package)
            {
                //收到一个： _x op _y 的字符串，将里面的每一个内存提取出来
                std::size_t left = package.find(SPACE);
                if(left == package.npos)
                {
                    //找不到空格表示不符合当前协议的格式
                    return false;
                }
                size_t right = package.rfind(SPACE);
                if(right == package.npos)
                    return false;

                _x = atoi(package.substr(0,left).c_str()); //截取_x
                _y = atoi(package.substr(right+SPACE_LEN).c_str());//截取_y

                if(left + SPACE_LEN > package.size())
                    return false;
                _op = package[left+SPACE_LEN];

            }
        public:
            int _x;     
            int _y;     //计算数
            char _op;   //计算符号："+"  "-"  "*"  "/"
    };


    class Respone
    {
        public:
            Respone()
            {}
            Respone(int result,int code):_result(result),_code(code)
            {}
            ~Respone()
            {}


            //序列化
            std::string Serialize()
            {}

            //反序列化
            bool DeSerialize(std::string& package)
            {

            }
        public:
            int _result; //计算结果
            int _code;   //结果是否正确/状态码
    };

     std::string Recv(int sock)
    {
        char inbuffer[1024];

        //从serverSock读取客户端发过来的序列化字符串
        ssize_t s = recv(sock,inbuffer,sizeof(inbuffer),0);
        if(s > 0)
            return inbuffer;
    }

    void Send(int sock,const std::string str)
    {
        //发送序列化的字符串给服务端
        send(sock,str.c_str(),sizeof(str),0);
    }
}