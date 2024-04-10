#pragma once
#include <iostream>
#include <vector>
#include <unistd.h>
using std::cout;
using std::endl;

inline static void *_SystemAlloc(size_t size)
{
#ifdef _WIN32
    void *ptr = VirtualAlloc(0, kpage * (1 << PAGE_SHIFT), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
    // brk mmap等
    void *ptr = sbrk(0);                      // sbrk开空间
    void *head = ptr;                         // 固定住头
    brk((char *)ptr + (size)); // 开kpages * 8*1024 字节

#endif
    if (head == nullptr)
        throw std::bad_alloc();
    return head;
}
inline static void _SystemFree(void *ptr)
{
#ifdef _WIN32
    VirtualFree(ptr, 0, MEM_RELEASE);
#else
    // sbrk unmmap等
    brk(ptr);
#endif
}


template <class T>
class ObjectPool
{
public:
    void *Delete(T *obj)
    {
        //同理在回收空间时，需要调用对象的析构函数
        obj-> ~T();
        // 删除的节点需要使用链表将回收的空间存起来
        // 对于回收的空间我们如果要使用链表将其连接起来，可以使用每段空间的前面4\8字节的空间
        // 将其作为指针存放下一段空间的地址
        if (_freelist == nullptr)
        {
            // 头为空时将当前指针赋予头指针
            _freelist = obj;
            // 将首部4\8个字节设为存放下一段空间的地址
            // 问题：怎么实现？
            // 将头指针转化为二级指针然后将其解引用，那么系统会自动识别指针的大小
            *(void **)_freelist = nullptr;
        }
        else
        {
            // 如果不为空那么进行头插
            // 1.当前指针的下一个地址指向头指针
            *(void **)obj = _freelist;
            _freelist = obj;
        }
    }
    T *New()
    {
        T* obj =nullptr;
        // 申请空间
        // 如果_freelist不为空则优先使用freelist中的空间
        if (_freelist != nullptr)
        {
            obj = (T *)_freelist;
            // 将下一个地址赋给_freelist
            _freelist = *((void **)_freelist);
            return obj;
        }
        // 向定长内存池申请空间时先看看当前剩余大小是否超过了一个申请对象的大小
        else
        {
            if (allocremain < sizeof(T))
            {
                allocremain = 1024 * 128;
                _memory = (char*)SystemAlloc(allocremain);
                if (_memory == nullptr)
                {
                    throw std::bad_alloc();
                }
            }
            // 开始申请内存,一般申请一个对象大小的
            obj = (T *)_memory;

            // 原定长内存池的地址往下走一个sizeof(T)的大小
            // 考虑如果对象的大小小于一个指针，则至少申请一个指针大小
            size_t newsize = sizeof(T) > sizeof(void*) ? sizeof(T) : sizeof(void*);
            _memory += newsize;
            allocremain -= newsize;
        }
        //申请空间后,如果申请对象是一个类，那么需要去调用它的构造函数完成初始化
        //定位new
        //!!!!!!!!!!!!!!!!!!!!!!非常重要(知识点)
        new(obj)T;
        return obj;
    }

private:
    char *_memory = nullptr;   // 定长内存池的地址
    void *_freelist = nullptr; // 定长内存释放回收后的自由链表
    size_t allocremain = 0;    // 定长内存池剩余的空间大小
};