#ifndef PAGE_CACHE_HPP
#define PAGE_CACHE_HPP
#include "Common.hpp"
#include "ObjectPool.hpp"
// 单例对象的PageCache
class PageCache
{
public:
    static PageCache *Getinstance()
    {
        return &_sInst;
    }

    Span *NewSpan(size_t npages)
    {
        if (npages > NPAGES - 1)
        {
            // 如果对象申请大小超过了NPAGES,那么直接找堆申请
            void *ptr = SystemAlloc(npages);

            // 改进2：我们自己写的内存池原本功能就是替代使用new,malloc的接口的，所以我复用了先前写的一个定长内存池用于
            // 代替new的使用提高效率

            Span *span = new Span;
            // Span* span =_objPool.New();

            span->_id = (PAGE_ID)ptr >> PAGE_SHIFT;
            span->n = npages;

            //!!!!!!!!!!!!!!!!!!!!!!!记得将大内存的页号和它的span指针关联起来
            _spanMap[span->_id] = span;

            // 同理：如果是申请大内存那么也需要将span中的_objSize赋值
            span->_objSize = npages << PAGE_SHIFT;

            return span;
        }
        assert(npages > 0 && npages <= NPAGES - 1);
        // PageCache拿到需要申请的页数
        // 根据这npages直接定位到哈希桶的位置
        // 1.定位到当前桶，判断是否为空
        if (!_spanList[npages].Empty())
        {
            // 如果不为空则直接弹一个Span出来
            // 注意!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!！：如果是第一次直接找这个桶就找到了当前桶非空，这时不能直接返回Span，因为当前Span中只有头和尾的页号映射在了哈希表里面
            // 中间的页都还没有映射，所以也需要手动将所有页号映射进哈希表中

            Span *retSpan = _spanList[npages].PopFront();

            // 将即将返回的一整个retSpan的所以里面的页号与Span指针记录进哈希表，方便回收内存时CentralCache中找到对应的Span
            for (PAGE_ID i = 0; i < retSpan->n; i++)
            {
                // 也就是将这一个Span中里面所有的页都映射到这个Span上
                _spanMap[retSpan->_id + i] = retSpan;
            }

            return retSpan;
        }

        // 2.如果当前桶为空，那么就向下一个桶遍历，直到有桶不为空或着走完为止
        for (int step = npages + 1; step < NPAGES; step++)
        {
            // 中途找到了不为空的桶，要做两步
            // A:将当前比npages长的Span截断两份，一份长为npages,一份为step-npages
            if (!_spanList[step].Empty())
            {

                Span *currSpan = _spanList[step].PopFront();

                // 代替new提高效率
                // Span *retSpan = new Span;
                Span *retSpan = _objPool.New();

                retSpan->n = npages;
                retSpan->_id = currSpan->_id;

                currSpan->n -= npages;
                currSpan->_id += npages;

                // 由于在PageCache回收Span中，Span的前后Span合并需要用到根据其头跟尾部的页号定位到当前Span的的指针
                // 所以需要将currSpan的头尾页映射进哈希表中
                _spanMap[currSpan->_id] = currSpan;
                _spanMap[currSpan->_id + currSpan->n - 1] = currSpan;
                // B:将currSpan链进新的哈希桶里
                _spanList[currSpan->n].PushFront(currSpan);

                // 将即将返回的一整个retSpan的所以里面的页号与Span指针记录进哈希表，方便回收内存时CentralCache中找到对应的Span
                for (PAGE_ID i = 0; i < retSpan->n; i++)
                {
                    // 也就是将这一个Span中里面所有的页都映射到这个Span上
                    _spanMap[retSpan->_id + i] = retSpan;
                }

                return retSpan;
            }
        }

        // 3.如果走到这，代表整个PageCache都没有大于等于npages非空的Span
        // 代表这里需要向系统申请一个最大值的page,即这里是128长度的page
        // 这里是Linux，用Linux的原生系统接口brk mmap等

        // 代替new提高效率
        // Span *newspan = new Span;
        Span *newspan = _objPool.New();

        void *ptr = SystemAlloc(NPAGES - 1);
        newspan->n = NPAGES - 1;
        newspan->_id = (PAGE_ID)ptr >> PAGE_SHIFT;

        // 将这个新Span挂到最大值Page的桶中
        _spanList[newspan->n].PushFront(newspan);

        // 最后需要再次执行一次上面代码的操作，由于重复写代码会显得很冗余，所以直接递归调用,能够直接再次执行一次上面的函数
        return NewSpan(npages);
    }

    // 将CentralCache回收内存申请的Span*指针给它
    Span *MapToSpan(void *target)
    {
        // 改进4：由于std的map没有设置线程安全，所以在读写访问时容易出问题，所以需要加锁控制
        std::unique_lock<std::mutex> lock(_pageMtx);

        PAGE_ID id = ((PAGE_ID)target >> PAGE_SHIFT);
        auto ret = _spanMap.find(id);
        if (ret != _spanMap.end())
        {
            return ret->second;
        }
        else
        {
            assert(false);
            return nullptr;
        }
    }
    void ReleaseSpanToPageCache(Span *span)
    {
        // 改进1：如果span是从堆里申请的，那么直接还给堆
        if (span->n > NPAGES - 1)
        {
            void *ptr = (void *)(span->_id << PAGE_SHIFT);
            SystemFree(ptr);

            // 注意：这里的Span是非常特殊的，是 > NPAGES-1的span，它直接向堆申请，所以释放的时候也会向堆直接返还，不需要再链进
            // 定长内存池中，因为已经被回收空间了
            //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            //_objPool.Delete(span);
            // span-> ~Span();
            delete span;
            return;
        }
        // 这里是CentralCache中的Span全部收回内存块后将当前Span返还给PageCache中
        // 首先进到这里要做的第一件事是先将当前Span的前后相邻的空间Span做一个合并
        // 问题：如何判断一个Span是否空闲，在Span里面设置一个标志位，当这个Span回收完了所有内存块并且不是在转移的过程中，就代表可以进行合并

        // 1.首先往当前span前面合并
        while (true)
        {
            PAGE_ID previd = span->_id - 1; // 找到当前Span的上一页
            // A:先判断这个尾页是否在一个Span中
            auto isspan = _spanMap.find(previd);
            if (isspan == _spanMap.end())
            {
                // 如果找不到这个尾页所在的span,那么当前span的前面的span无法合并
                break;
            }

            // B:然后判断这个Span是否处于空闲状态
            if (isspan->second->_isUse == true)
            {
                // 如果当前Span处于被使用状态，那么也无法合并
                break;
            }

            // C:最后判断当前Span在合并后的总长度是否超过了NPAGES，超过了也不能合并
            if (isspan->second->n + span->n > NPAGES - 1)
            {
                break;
            }

            // 走到这里表示可以合并，所以开始合并
            span->n = span->n + isspan->second->n;              // 首先更新新span的页数
            span->_id = isspan->second->_id;                    // 因为是往前合并，所以新合并的span的页号就是前一个span的页号
            _spanList[isspan->second->n].Erase(isspan->second); // 合并之后要给合并的span解绑

            // 因为已经替代了new了，所以释放也需要调用_objPoll里的Delete
            // delete isspan->second;      //解掉前一个span
            _objPool.Delete(isspan->second);
        }

        // 2.然后往后进行合并
        while (true)
        {
            PAGE_ID nextid = span->_id + span->n - 1; // 找到当前Span的下一页
            auto isspan = _spanMap.find(nextid);

            // A:判断下一个span是否存在
            if (isspan == _spanMap.end())
            {
                break;
            }

            // B:判断下一个span是否空闲
            Span *nextspan = isspan->second;
            if (nextspan->_isUse == true)
            {
                break;
            }

            // C:判断下一个span合并后是否总长度超过NPAGES
            if (nextspan->n + span->n > NPAGES - 1)
            {
                break;
            }

            // 开始合并
            span->n += nextspan->n;
            // span id不用变，因为合并的span在后面,头不用动
            _spanList[nextspan->n].Erase(nextspan);

            // 因为已经替代了new了，所以释放也需要调用_objPoll里的Delete
            // delete nextspan;
            _objPool.Delete(isspan->second);
        }

        // 又有一个疑问？？？？？？？？？？
        // 这里不需要将span的指针重置到整个span的头部吗???????？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？
        // 不需要，无论span指向哪里，只要它指在他的区域范围内，后面需要用到的页号也只需要找到span->_id就可以了

        _spanList[span->n].PushFront(span);
        span->_isUse = false;
        _spanMap[span->_id] = span;
        _spanMap[span->_id + span->n - 1] = span; // 将新合并的span的头尾页填进哈希表
    }
    std::mutex _pageMtx;

private:
    PageCache()
    {
    }
    PageCache(const PageCache &other) = delete;
    const PageCache &operator=(const PageCache &other) = delete; // 禁用构造函数和拷贝构造
private:
    // page cache本质也是一个哈希桶，但是它的哈希映射是1-128page的映射，与thread cache和central cache不一样
    // PageCache跟CentralCache一样是用到了单例模式
    SpanList _spanList[NPAGES];

    static PageCache _sInst; // 单例模式的对象

    std::unordered_map<PAGE_ID, Span *> _spanMap;

    ObjectPool<Span> _objPool; // 我们创建一个内存池用于代替new,malloc等接口的使用
};
PageCache PageCache::_sInst;
#endif