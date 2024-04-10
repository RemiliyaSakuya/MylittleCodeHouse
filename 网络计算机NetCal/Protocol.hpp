#pragma once

#include <iostream>
#include <cstring>

//定义一个客户端和服务器都遵守的协议
namespace ns_Protocol
{


//协议内容：客户端必须发送一段:数字 '+-*/符号' 数字.这样一段格式的数字
#define SPACE " "
#define SPAN_LEN strlen(SPACE)

    //定义一个请求类用于封装客户端发过来的请求
    class Request
    {
        public:
            Request()
            {}
            Request(int x,int y,char op):_x(x),_y(y),_op(op)
            {}
            ~Request()
            {}
            
            //序列化
            std::string Serialize(std::string& package)
            {}

            //反序列化
            bool DeSerialize(std::string& package)
            {

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