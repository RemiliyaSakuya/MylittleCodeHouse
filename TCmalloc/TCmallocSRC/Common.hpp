#ifndef COMMON_HPP
#define COMMON_HPP
#include <iostream>
#include <vector>
#include <cassert>
#include <pthread.h>
#include <thread>
#include <mutex>
#include <unistd.h>
#include <unordered_map>
using std::cout;
using std::endl;

// 定义页编号类型，32位为size_t，64位为unsigned long long
// 注意：当前平台为linux，关键字和windows有不一样
#ifdef __x86_64__
typedef unsigned long long PAGE_ID;
#else
typedef size_t PAGE_ID;
#endif
// 最大申请内存大小KB
static const size_t MAX_BYTES = 256 * 1024;
// 哈希桶的总共有多少个桶
static const size_t MAX_LISTNODE = 208;
// PageCache中哈希桶的长度
static const size_t NPAGES = 129; // 实际上只有128个pages桶位，但因为是下标从0开始所以第0位不用，空出来对齐
// 定义一个页的大小，这里是2^13,即一个页有8kb大小
static const size_t PAGE_SHIFT = 13;

// 获取下一个指针
void *&GetNext(void *ptr)
{
    return (*(void **)ptr);
}

// 用于管理哈希桶映射值等内存对齐
class ClassSize
{
public:
    // 整体控制在最多10%左右的内碎片浪费
    // [1,128] 8byte对齐       freelist[0,16)
    // [128+1,1024] 16byte对齐   freelist[16,72)
    // [1024+1,8*1024] 128byte对齐   freelist[72,128)
    // [8*1024+1,64*1024] 1024byte对齐     freelist[128,184)
    // [64*1024+1,256*1024] 8*1024byte对齐   freelist[184,208)

    // 首先对方传进来一个需要申请的空间大小,我们需要从这个给定的一个
    // 空间大小找到：1.对应的内存对齐值+对齐内存的大小  2.对应内存对齐大小中对应的哈希桶位置

    // 先是找出内存对齐值
    static inline size_t _Round_up(size_t allocsize, size_t align)
    {
        // Round_up的分支
        // 在拿到的要申请的目标大小+所在的内存对齐值
        // 然后就是将该目标申请值向上取到对齐值

        // //思路1：普通版
        // if(allocsize % align == 0)
        // {
        //     //如果目标申请空间大小模上对齐值==0
        //     //那么它本身就是对齐的了直接返回
        //     return allocsize;
        // }
        // else
        // {
        //     //如果不能整除对齐值
        //     //那么就将allocsize和align的除数+1，向上取整得到乘法的取整值
        //     //将这个数乘上对齐值就是对应内存对齐大小
        //     allocsize = (allocsize/align + 1) * align;

        // }

        // 思路2：使用位运算简化这个过程，提升运行效率
        // 原理:将原申请的空间先跟align-1混在一起,然后将他们与align-1的取反结果相与
        // 最后使其保持对align的对齐
        return (allocsize + align - 1) & ~(align - 1);
    }
    static inline size_t Round_up(size_t allocsize)
    {
        // 根据上面的对齐法则设计多项选择分支
        if (allocsize <= 128)
        {
            // TODO
            // 对齐数:8bytes
            return _Round_up(allocsize, 8);
        }
        else if (allocsize <= 1024)
        {
            // TODO
            // 对齐数:16bytes
            return _Round_up(allocsize, 16);
        }
        else if (allocsize <= 8 * 1024)
        {
            // TODO
            // 对齐数:128bytes
            return _Round_up(allocsize, 128);
        }
        else if (allocsize <= 64 * 1024)
        {
            // TODO
            // 对齐数:1024bytes
            return _Round_up(allocsize, 1024);
        }
        else if (allocsize <= 256 * 1024)
        {
            // TODO
            // 对齐数:8*1024bytes
            return _Round_up(allocsize, 8 * 1024);
        }
        else
        {
            // 这里超过了256kb则标记错误
            // 改进1：当超过了256KB则让他直接向PageCache申请
            return _Round_up(allocsize, 1 << PAGE_SHIFT);
            //assert(false);
            //return -1;
        }
    }

    // 2.开始定位对应的申请内存大小在哪一个哈希桶里面，即数组下标
    static inline size_t Index(size_t alignsize)
    {
        assert(alignsize < 256 * 1024);
        static size_t list_group[] = {16, 56, 56, 56}; // 哈希桶每个区间分出多少个桶

        // 从申请的空间总大小定位对应的桶下标公式：alignsize / 对齐数 + 前面所有区间桶的总数
        if (alignsize <= 128)
        {

            // 对齐数:8bytes
            return (alignsize / 8) - 1;
        }
        else if (alignsize <= 1024)
        {

            // 对齐数:16bytes
            return ((alignsize - 128) / 16) + list_group[0] - 1;
        }
        else if (alignsize <= 8 * 1024)
        {

            // 对齐数:128bytes
            return ((alignsize - 1024) / 128) + list_group[0] + list_group[1] - 1;
        }
        else if (alignsize <= 64 * 1024)
        {

            // 对齐数:1024bytes
            return ((alignsize - 8 * 1024) / 1024) + list_group[0] + list_group[1] + list_group[2] - 1;
        }
        else if (alignsize <= 256 * 1024)
        {

            // 对齐数:8*1024bytes
            return ((alignsize - 64 * 1024) / (8 * 1024)) + list_group[0] + list_group[1] + list_group[2] + list_group[3] - 1;
        }
        else
        {
            // 这里超过了256kb则标记错误
            assert(false);
            return -1;
        }
    }
    static inline size_t NumMoveSize(size_t size) // thread cache一次向central cache批量申请的页数
    {
        // 这里使用慢开始调节算法

        if (size == 0)
            return 0;

        int num = MAX_BYTES / size;
        // 申请的个数限制在[2,512]之间
        // 小对象批量上限高
        // 大对象批量上限低

        if (num > 512)
            return 512; // 申请个数大于512最多给你512字节
        if (num < 2)
            return 2; // 申请对象小于2则至少给你2字节

        return num;
    }
    static inline size_t NumMovePages(size_t size)
    {
        // 计算需要申请的页数
        size_t num = NumMoveSize(size);         // 先计算出需要向系统申请多少块内存(批量申请,尽量满足)
        size_t totalnum = num * size;           // 计算出单个申请对象*总共申请多少个对象=一共要申请多大
        size_t npages = totalnum >> PAGE_SHIFT; // 用总共的申请大小 / 单个页的大小 = 要申请的页数

        if (npages == 0) // 申请的总大小不足一个页就至少申请一个页
            npages = 1;

        return npages;
    }
};

// 前提说明：thread cache是每一个线程都独享的多段内存空间，一个数组中存储了
// 多个指针，而每一个指针指向了固定大小的内存块，但是每一个内存块中又分配出
// 指针指向下一段同为自身大小的内存块，这样形成了一个哈希桶的结构，便于管理与回收

// 用于管理切分好的一个个内存块
class FreeList
{
public:
    // 头插
    void Push(void *obj)
    {
        *(void **)obj = _freelist;
        _freelist = obj;
        _size++; // 每插入一个内存块就将_size加1
    }

    // 批量头插
    void PushRang(void *start, void *end, size_t size)
    {
        // 将end这一端插进头部,即end的下一个指针指向头部
        GetNext(end) = _freelist;

        // 然后将_freelist移向start的位置
        _freelist = start;

        _size += size;
    }
    void *Pop()
    {
        // 头删
        assert(_freelist != nullptr);

        void *obj = _freelist;
        _freelist = *(void **)_freelist;
        _size--; // 每借出一个内存块就将_size减1
        return obj;
    }
    void PopRang(void *&start, void *&end, size_t size)
    {
        assert(size >= _size);
        // 代表在当前freelist中的某个桶中的自由链表要回收size个内存块给CentralCache
        start = _freelist; // 将当前自由链表头指针赋给start
        end = start;

        // 让end走 size-1 步，就能让end指向最后一个需要取回的节点
        for (int i = 0; i < size - 1; i++)
        {
            end = GetNext(end);
        }
        // 将end指向下一个的指针为空,并将end的下一个链回自由链表的头
        _freelist = GetNext(end);
        GetNext(end) = nullptr;
        _size -= size;
    }
    bool empty()
    {
        return _freelist == nullptr;
    }
    size_t &GetMaxSize()
    {
        return _maxSize;
    }
    size_t Size()
    {
        return _size;
    }

private:
    void *_freelist = nullptr; // 每一段内存空间的头指针
    size_t _maxSize = 1;       //_maxSize用于标识当前内存桶在向Central Cache申请新的空间时能够最大申请多少
                               // 让其从小到大一路增值申请，能够有效避免申请过多的空间
    size_t _size = 0;          // 第二部分：用一个_size记录当前freelist中总共有多少个空闲内存块
};

// 到了CentralCache这一层，其结构跟ThreadCache一模一样是一段哈希桶的结构
// 但是CentralCache在桶中连接的不是一个个切分好的内存，而是一张张Span(也就是分页)，其结构是双向链表
// Span里面就被切分成一摸一样的小内存

// 定义每一个Span的结构
struct Span
{
    PAGE_ID _id = 0; // 页的起始ID
    size_t n = 0;    // 页的数量

    Span *next = nullptr; // 双向链表的下一个指针
    Span *prev = nullptr; // 双向链表的上一个指针

    size_t _useCount = 0;      // 切好的小块内存，被分配给thread cache计数
    void *_freelist = nullptr; // 切好小块内的自由链表

    bool _isUse = false; // 给Span一个标志位，用于判断当前Span是否处于空闲状态

    // 新增3:优化结构使其调用释放函数时不再需要传参大小size
    size_t _objSize = 0; // 用于记录一个Span它的切分好的小对象的单个大小
};

// 每一个桶的Span双向链表
class SpanList
{
public:
    SpanList()
    {
        // 构造函数，第一次创建时将其链成一个双向链表
        _head = new Span;
        _head->next = _head;
        _head->prev = _head;
    }
    Span *Begin()
    {
        // 定位到当前SpanList的头个有效节点
        return _head->next;
    }
    Span *End()
    {
        // 地位到当前SpanList的尾节点
        return _head;
    }
    bool Empty()
    {
        return _head->next == _head;
    }
    void Insert(Span *pos, Span *newSpan)
    {
        // pos是需要插入的位置的后面，即将newSpan插入到pos前面
        assert(pos);
        assert(newSpan);
        Span *_prev = pos->prev;
        _prev->next = newSpan;
        newSpan->next = pos;
        newSpan->prev = _prev;
        pos->prev = newSpan;
    }
    void PushFront(Span *newspan)
    {
        Insert(Begin(), newspan);
    }
    void Erase(Span *pos)
    {
        // 找到需要删除的节点
        assert(pos);
        Span *_prev = pos->prev;
        Span *_next = pos->next;
        _prev->next = _next;
        _next->prev = _prev;

        // 不需要将pos delete掉，而是转回给PageCache
        // TODO............
    }

    // 弹一个头Span出来
    Span *PopFront()
    {
        Span *popspan = _head->next;
        Erase(_head->next);
        return popspan;
    }

private:
    Span *_head; // 是一个带头双向链表的头指针
public:
    std::mutex _mtx; // 在CentralCache中每一个SpanList都有独属于自己的锁,这样在申请不同的桶是也不用竞争锁，互不影响
};

inline static void *SystemAlloc(size_t kpage)
{
#ifdef _WIN32
    void *ptr = VirtualAlloc(0, kpage * (1 << PAGE_SHIFT), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
    // brk mmap等
    void *ptr = sbrk(0);                      // sbrk开空间
    void *head = ptr;                         // 固定住头
    brk((char *)ptr + (kpage << PAGE_SHIFT)); // 开kpages * 8*1024 字节

#endif
    if (head == nullptr)
        throw std::bad_alloc();
    return head;
}
inline static void SystemFree(void *ptr)
{
#ifdef _WIN32
    VirtualFree(ptr, 0, MEM_RELEASE);
#else
    // sbrk unmmap等
    brk(ptr);
#endif
}

#endif