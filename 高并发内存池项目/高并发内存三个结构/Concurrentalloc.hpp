#ifndef CONCURRENTALLOC_HPP
#define CONCURRENTALLOC_HPP
#include "Common.hpp"

// 这里是函数调用TLS局部存储的变量
// 最终直接通过这个接口调用申请内存
void *ConCurrentAlloc(size_t size)
{
    // 改进1：解决申请大于256KB的大块内存申请问题
    if (size > MAX_BYTES)
    {
        // 分两种情况

        //// A：如果申请的空间在[32*8Kb,128*8KB]之间 ,仍然向三层缓存申请
        // 在这个区间内可以直接找PageCache要
        size_t alignsize = ClassSize::Round_up(size);
        /// 让其对其一个页的大小

        size_t npages = alignsize >> PAGE_SHIFT; // 计算需要申请多少页

        // 直接找PageCache要npages个页
        PageCache::Getinstance()->_pageMtx.lock();
        Span *span = PageCache::Getinstance()->NewSpan(npages);
        span->_objSize = size;
        
        //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!非常重要,在这里特殊情况直接向PageCache申请的Span在拿到之后要马上设置_isUse为true,不然会导致后面合并出问题
        span->_isUse = true;
        
        PageCache::Getinstance()->_pageMtx.unlock();

        // 拿到这个Span后根据其页号，拿到其地址
        void *ptr = (void *)(span->_id << PAGE_SHIFT);
        return ptr;
    }
    else
    {
        // 进来先判断pTLSthreadcache是否为空
        if (pTLSthreadcahe == nullptr)
        {
            //这里创建TLS局部存储时，也可以用定长内存池代替new
            //由于第一次创建，也需要创建一个固定的内存池

            //pTLSthreadcahe = new ThreadCache;
            //用定长内存池代替new
            static ObjectPool<ThreadCache> TLSPool;
            pTLSthreadcahe=TLSPool.New();
        }
        return pTLSthreadcahe->Allocate(size);
    }
}

void ConCurrentFree(void *ptr)
{

    //改进3：由于Span中存在了_objSize了,不再需要将size传入到这个函数中
    Span *span = PageCache::Getinstance()->MapToSpan(ptr);
    size_t size = span->_objSize;   //直接根据map找到span的指针后再找到其中的_objSize
    assert(ptr);
    if (size > MAX_BYTES)
    {
        // 如果回收的内存超出了NPAGES的大小,直接还给堆
        // 1.根据页号找哈希表映射Span指针
        //Span *span = PageCache::Getinstance()->MapToSpan(ptr);

        // 2.还给PageCache或着堆
        PageCache::Getinstance()->_pageMtx.lock();
        PageCache::Getinstance()->ReleaseSpanToPageCache(span);
        PageCache::Getinstance()->_pageMtx.unlock();
    }
    else
    {
        pTLSthreadcahe->Deallocate(ptr, size);
    }
}

#endif
