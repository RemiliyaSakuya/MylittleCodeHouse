#ifndef THREAD_CACHE_HPP
#define THREAD_CACHE_HPP
#include "Common.hpp"
#include "CentralCache.hpp"
// thread cache的哈希桶结构
class ThreadCache
{
public:
    void *GetBatchFromCentralCache(size_t size, size_t index)
    {
        assert(size < MAX_BYTES);
        // ThreadCache向CentralCache申请空间的步骤
        // 1.申请不会只申请1、2个对象，而是批量申请一部分
        // 2.但不能申请太多否则会造成浪费
        // 所用到的方法：慢开始反馈调节算法(负反馈调节)
        // 即：申请的对象越小给的越多，申请的对象越大给的越少

        size_t batchNum = std::min(_freelists[index].GetMaxSize(), (ClassSize::NumMoveSize(size))); // 得到要申请的个数

        // std::cout << "需要申请的内存块个数：" << batchNum <<std::endl;
        //  将从BatchSpanAlloc中申请到的个数与GetMaxSize()得到的_maxSize做对比，取小的那个
        //  然后将_maxSize + 2，使其下一次最大能够多申请一个直到满512顶头为止

        if (_freelists[index].GetMaxSize() == batchNum)
        {
            // 如果这两相等表示这一次申请到达了最大申请值，让其下一次能够多申请一点
            _freelists[index].GetMaxSize() += 1;
        }

        // 通过拿着这个batchNum向CentralCache开始申请相应的页
        void *start = nullptr;
        void *end = nullptr;
        size_t actualnum = CentralCache::Get_Instance()->FetchRangObj(start, end, batchNum, size);

        // cout << "实际要向CentralCache申请的内存个数:" << actualnum << endl;
        // cout << "satart:" << start << endl;
        // cout << "end:" <<end << endl;
        assert(actualnum > 0); //!!!!!!!!!这一段与教材有异议，先标记

        // 得到了start,end中间这一段申请的内存，那么就将start返回，将[start->next,end]这一段插入进thread cache中，下次申请就不用去找Central Cache
        // 问题：可能batchNum比对应Central Cache中Span里面剩余的内存块要多，那么有多少就还多少,所以有了actualnum这一个实际上返回的内存块数目
        if (actualnum == 1)
        {
            // 如果只能申请一个那么start==end直接返回start;
            assert(start == end);
            return start;
        }
        else
        {
            // 如果actualnum不是1,那么这里就表示成功申请了多个内存块，那么就返回start，将[start->next,end]插回Thread Cache中
            void *startnext = GetNext(start); // 找到start下一个

            // 将[startnext,end]这一段头插进对应的threadCache的桶中
            _freelists[index].PushRang(startnext, end, actualnum - 1); // 将实际上成功申请了多少块内存块返回给PushRang中方便其_size加上对应的数
            return start;
        }
    }
    void *Allocate(size_t size) // 从thread cache中申请内存
    {
        assert(size >= 0 && size <= MAX_BYTES);
        // 1.获取目标申请对象空间大小在对齐后的总空间大小
        size_t alignsize = ClassSize::Round_up(size);
        // 2.获取总空间大小所在的桶小标
        size_t inde = ClassSize::Index(alignsize);

        assert(alignsize < MAX_BYTES);
        // test
        // cout << "原来申请的空间大小：" << size << endl;
        // cout << "对齐后的总大小：" << alignsize << endl;
        // cout << "对应的桶下标" << inde << endl;

        // 开始申请空间
        if (!_freelists[inde].empty())
        {
            // 不为空则从自由链表中取下一个节点
            return _freelists[inde].Pop();
        }
        else
        {
            // 对应的桶为空那么就开始向central cache申请空间
            // TODO
            return GetBatchFromCentralCache(alignsize, inde);
        }
    }
    void ListTooLong(FreeList &list, size_t size)
    {
        // Thread Cache中的自由链表太长时需要将其回收回Central Cache中
        // 定位需要回收的自由链表只需要给到对应桶和回收的内存块大小
        void *start = nullptr;
        void *end = nullptr;
        list.PopRang(start, end, list.GetMaxSize()); // 将list中的MaxSize传进PopRang中代表一次回收_maxsize个内存块给CentralCache

        // 得到了需要释放的空间[start,end],将其回收进Central Cache中
        // 调用CentralCache的回收函数

        CentralCache::Get_Instance()->ReleaseBackSpan(start, size); // 得到了需要回收的_freelist自由链表的头指针，通过size定位到CentralCache的对应桶中
    }
    void Deallocate(void *ptr, size_t size) // 回收申请的内存
    {
        assert(ptr);
        assert(size < MAX_BYTES);

        size_t alignsize = ClassSize::Round_up(size);
        size_t index = ClassSize::Index(alignsize); // 根据需要回收的空间大小找到其所在的桶位置
        _freelists[index].Push(ptr);                // 根据空间坐标将其头插回对应的thread cache桶中

        // 第二部分：当还回来的内存块个数 > 一次批量申请的内存(_maxsize)时，就归还一部分list给CentralCache回去
        if (_freelists[index].Size() >= _freelists[index].GetMaxSize())
        {
            // 开始向CentralCache中归还内存块
            ListTooLong(_freelists[index], alignsize);
        }
    }

private:
    FreeList _freelists[MAX_LISTNODE];
};

// 将ThreadCache定义为TLS线程局部存储，使得每一个线程都独立独享自己的ThreadCache
static __thread ThreadCache *pTLSthreadcahe = nullptr;

// 发现的问题：1.invalid use of incomplete :两个类特别是在前面的类A中的成员函数实现时需要用到后面定义的类B中的成员函数
// 解决办法1：将A定义到B后面，或着定义和声明分离

#endif