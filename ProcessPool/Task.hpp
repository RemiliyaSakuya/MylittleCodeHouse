#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unistd.h>
#include <functional>

//将任务函数重命名
typedef std::function<void()> func;

std::vector<func> callbacks;//定义一个数组用于调用指令任务
std::unordered_map<int,std::string> desc;

void readMySQL()
{
    std::cout << "sub process[" << getpid() << " ] 执行访问数据库的任务\n" << std::endl;
}

void execuleUrl()
{
    std::cout << "sub process[" << getpid() << " ] 执行url解析\n" << std::endl;
}

void cal()
{
    std::cout << "sub process[" << getpid() << " ] 执行加密任务\n" << std::endl;
}

void save()
{
    std::cout << "sub process[" << getpid() << " ] 执行数据持久化任务\n" << std::endl;
}

void load()
{
    desc.insert({callbacks.size(), "readMySQL: 读取数据库"});
    callbacks.push_back(readMySQL);

    desc.insert({callbacks.size(), "execuleUrl: 进行url解析"});
    callbacks.push_back(execuleUrl);

    desc.insert({callbacks.size(), "cal: 进行加密计算"});
    callbacks.push_back(cal);

    desc.insert({callbacks.size(), "save: 进行数据的文件保存"});
    callbacks.push_back(save);
}

void showHandler()
{
    for(const auto &iter : desc )
    {
        std::cout << iter.first << "\t" << iter.second << std::endl;
    }
}

int handlerSize()
{
    return callbacks.size();
}
