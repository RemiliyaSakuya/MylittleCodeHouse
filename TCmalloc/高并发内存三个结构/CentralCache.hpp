
#ifndef CENTRAL_CACHE_HPP
#define CENTRAL_CACHE_HPP
#include "Common.hpp"
#include "PageCache.hpp"

int lockcount =0 ;
// 用类将CentralCache的结构封装起来(使用单例模式，这里以饿汉模式为例)
class CentralCache
{
public:
    static CentralCache *Get_Instance()
    {
        return &_sList;
    }

    Span *GetOneSpan(SpanList &list, size_t size)
    {
        // 找到对应桶，后开始便利这个桶找到第一个非空Span
        Span *start = list.Begin();
        Span *end = list.End();

        // 遍历从start到end
        while (start != end)
        {
            if (start->_freelist != nullptr)
            {
                return start;
            }
            else
            {
                start = start->next;
            }
        }

        // 这里将当前桶锁先解掉，为了能够将哪些归还内存的线程能够通过
        list._mtx.unlock();

        // lockcount--;
        // cout <<  "0号桶以解锁" << endl;
        // cout <<  "0号桶加锁次数:" << lockcount <<endl;
        // cout << "############################################" <<endl;
        // 走到这里代表当前SpanLisr没有空的Span，则需要向PageCache申请新的Span
        // size是申请的空间对齐后的总大小
        // 这里是CentralCache中没有非空的Span时，向PageCache申请多少个页
        // 调用算法NumMovePages

        // 到这里一定会进入到PageCache,所以在这里上一个PageCache的锁
        PageCache::Getinstance()->_pageMtx.lock();

        Span *newspan = PageCache::Getinstance()->NewSpan(ClassSize::NumMovePages(size));
        //到这里成功申请了Span,代表该Span已经进入了使用的状态
        newspan->_isUse = true;

        //在CentralCache获取了Span后，同时也已经将Span切分好了一个个小对象了，于是在这里对Span里的_objSize赋值
        newspan->_objSize = size;


        PageCache::Getinstance()->_pageMtx.unlock();

        // 这里已经成功向PagesCache申请了一大段连续空间的Span
        // 我们需要将这一个Span里面的空间切分成对应单个申请对象的一个个的小内存块自由链表
        // 1.先跟据这个Span中的页号_id来取得它的地址
        char *_start = (char *)((newspan->_id) << PAGE_SHIFT); // 根据他的id*一个页的大小就可以确定它在这连续空间的哪一段,使用char*是能够按一个一个字节走
        size_t bytes = (newspan->n) << PAGE_SHIFT;             // 根据这个Span里面有多少页，将它*单个页大小可以算出一共有多少字节
        char *_end = _start + bytes;
        //cout << "新申请的newspan有多少页:"<<newspan->n << endl;
        //cout << "newspan中的_start:" << (void*)_start << endl;
        //cout << "newspan中的_end:" << (void*)_end << endl;
        
        //cout << "newspan中的_end - _start="<< _end -_start << endl;


        // 将这一段连续空间开始切成一段自由链表
        newspan->_freelist = _start;
        _start += size; //_start每次走一个单个申请对象size的大小
        void *tail = newspan->_freelist;
        int i =1;
        while (_start < _end)
        {
            // 将这一段连续空间切成自由链表链进这个Span的_freelist中
            GetNext(tail) = _start;
            tail = GetNext(tail);
            _start += size;
            i++;
        }
        // 疑问：不用将tail的下一个指针为空吗？!!!!!!!!!!!!!!!!!!
        GetNext(tail) = nullptr; //!!!!!!!!!!!!!!!!!!
        // 将这一个Span-链进这个桶中
        // 因为前面有写了个解锁，到这里会访问list链表，所以再次将桶锁加上
        
        list._mtx.lock();

        // lockcount++;
        // cout <<  "0号桶已加锁" << endl;
        // cout <<  "0号桶加锁次数:" << lockcount <<endl;
        // cout << "############################################" <<endl;

        list.PushFront(newspan);
        return newspan;
    }

    // 批量向ThreadCache提供内存块
    size_t FetchRangObj(void *&start, void *&end, size_t batchnum, size_t alignsize)
    {
        size_t index = ClassSize::Index(alignsize); // ！！！！！！！！！！！！！！感觉有点问题

        // 获取在哪个桶后，开始截取[start,end]的空间
        // 前提：还需要保证当前Span不是空的
        // 截取空间的操作需要加锁保证线程安全
        _freelists[index]._mtx.lock();

        // lockcount++;
        // cout <<  "0号桶已加锁" << endl;
        // cout <<  "0号桶加锁次数:" << lockcount <<endl;
        // cout << "############################################" <<endl;

        Span *span = GetOneSpan(_freelists[index], alignsize); // 获取非空的Span
        // TODO......
        assert(span);
        assert(span->_freelist);
        // 到这里已经拿到了非空Span的地址,开始截取
        // 注意：有可能当前Span下面的切分好的内存块数量不足batchnum,那么我们就有多少取多少
        size_t i = 0;
        size_t actualnunm = 1; // 实际拿到的内存块数量
        start = span->_freelist;
        end = start;

        

        while (i < batchnum - 1 && GetNext(end) != nullptr)
        {
            // 优先让end去走i-1次，end的下一个为空时立马停止,或着走够了batchnum-1次
            end = GetNext(end);
            i++;
            actualnunm++;
        }

        // 又有一个小疑问：在找到了[start,end]这一段空间后，需要将当前Span的头指针指向end的下一个
        // 问题是这里的Span的头指针就是_freelist也就是start当前的值，OKOK没事了
        span->_freelist = GetNext(end);
        span->_useCount += actualnunm;  //将当前Span中的_useCount加上actualnunm,表示分配出了几个内存块
        GetNext(end) = nullptr;
        _freelists[index]._mtx.unlock();

        //lockcount--;
        //cout <<  "0号桶已解锁" << endl;
        //cout <<  "0号桶加锁次数:" << lockcount <<endl;
        //cout << "############################################" <<endl;
        return actualnunm;
    }

    void ReleaseBackSpan(void *start, size_t size)
    {
        // 将目标自由链表插回对应的Span里面

        // 问题：如何找到对应的Span
        // 1：先根据size定位到对应的桶下标
        size_t index = ClassSize::Index(size);

        // 接下来链进Span的操作在共享区中进行所以需要加对应桶锁
        _freelists[index]._mtx.lock();

        //lockcount++;
        //cout << index <<"号桶以加锁" << endl;
        //cout << index <<"号桶加锁次数:" << lockcount <<endl;
        //cout << "############################################" <<endl;
        // 2:找到了需要操作的桶后，如何在SpanList中确定一个固定的Span
        // A:根据自由链表start的地址，用它 除以 一个页的大小，就可以知道对应的页号

        // B：在知道页号后，又如何知道是哪个Span？
        // 此时，引出在PageCache创建并分配出去被切割成对应Span使用的Page，我们可以使用哈希表将其页号和指针保存起来
        // 根据PageCache的_spanMap中，页号->Span的关系将对应Span*得到
        // 遍历这个自由链表，将每一个节点对应的Span找出来并且将其头插到Span中
        while (start != nullptr)
        {
            void *next = GetNext(start);
            Span *curspan = PageCache::Getinstance()->MapToSpan(start);

            // 拿到对应Span后将start头插进curSpan中
            GetNext(start) = curspan->_freelist;
            curspan->_freelist = start;
            curspan->_useCount--;
            // 第三部分：在CentralCache回收完ThreadCache的内存后，接着需要判断自己当前Span是否满足了全部回收了分配出去的内存块
            // 如果满足，则需要将当前Span还给PageCache让其合并成更大的Span

            // 这里又引出，如何得知Span已经回收了所有的内存块，需要在Span里面设置一个_countUse,用于计数正在使用的内存块
            if (curspan->_useCount == 0)
            {
                //如果当前Span的_useCount为0，那么代表已经回收了这个Span所有的内存块，开始向PageCache申请回收当前Span
                _freelists[index].Erase(curspan);   //将当前Span解除在当前_freelists中的链接，并将curSpan返回给PageCache
                curspan->_freelist =nullptr;
                curspan->next = nullptr;
                curspan->prev = nullptr;    //将curspan中的所有指针置空

                //在这个线程执行Span回收的过程中可以先将桶锁解除，方便其他线程能够回收其他ThreadCache的内存块
                _freelists[index]._mtx.unlock();

                // cout << "当前" <<index << "号桶解锁用于释放内存" <<endl;
                // lockcount--;
                // cout << index <<"号桶加锁次数:" << lockcount <<endl;
                // cout << "############################################" <<endl;


                //给PageCache加锁
                PageCache::Getinstance()->_pageMtx.lock();
                PageCache::Getinstance()->ReleaseSpanToPageCache(curspan);
                PageCache::Getinstance()->_pageMtx.unlock();
                
                
                _freelists[index]._mtx.lock();
                // lockcount++;
                // cout << index <<"号桶在PageCache收回span后重新上锁"<<endl;
                // cout << index <<"号桶加锁次数:" << lockcount <<endl;
                // cout << "############################################" <<endl;


            }
            start = next;
        }
        _freelists[index]._mtx.unlock();

        // lockcount--;
        // cout << "当前" <<index <<"号桶释放锁" <<endl;
        // cout << index <<"号桶加锁次数:" << lockcount <<endl;
        // cout << "############################################" <<endl;



        // 到这里的话就已经成功回收ThreadCache传进来的自由链表了

       
    }

private:
    CentralCache()
    {
    }
    CentralCache(const CentralCache &other) = delete;
    const CentralCache &operator=(const CentralCache &other) = delete; // 将拷贝构造和赋值重载都禁用
private:
    SpanList _freelists[MAX_LISTNODE]; // 用数组将所有自由链表存起来，形成一个哈希桶
    static CentralCache _sList;        // 定义一个静态的成员，在初始化前就已经创建出来了
};
CentralCache CentralCache::_sList;

#endif