#include "Common.hpp"
#include "ThreadCache.hpp"
#include "CentralCache.hpp"
#include "PageCache.hpp"
#include "Concurrentalloc.hpp"

void ConCurrentMalloc()
{
    void* p1 = ConCurrentAlloc(10);
    void* p2 = ConCurrentAlloc(8*1024*12);
    void* p3 = ConCurrentAlloc(8*1024*7);
    void* p4 = ConCurrentAlloc(90);
    void* p5 = ConCurrentAlloc(5455);
    void* p6 = ConCurrentAlloc(37);
    void* p7 = ConCurrentAlloc(127 * 8 *1024);
    void* p8 = ConCurrentAlloc(7);
    void* p9 = ConCurrentAlloc(900);



    cout << p1 <<endl;
    cout << p2 <<endl;
    cout << p3 <<endl;
    cout << p4 <<endl;
    cout << p5 <<endl;
    cout << p6 <<endl;
    cout << p7 <<endl;
    cout << p8 <<endl;
    cout << p9 <<endl;





    ConCurrentFree(p1);
    ConCurrentFree(p2);
    ConCurrentFree(p3);
    ConCurrentFree(p4);
    ConCurrentFree(p5);
    ConCurrentFree(p6);
    ConCurrentFree(p7);
    ConCurrentFree(p8);
    ConCurrentFree(p9);




}

void ConCurrentMalloc2()
{
    void* p1 = ConCurrentAlloc(129*8*1024);
    void* p2 = ConCurrentAlloc(129*8*1024);
    void* p3 = ConCurrentAlloc(129*8*1024);
    void* p4 = ConCurrentAlloc(129*8*1024);
    void* p5 = ConCurrentAlloc(129*8*1024);
    void* p6 = ConCurrentAlloc(129*8*1024);
    void* p7 = ConCurrentAlloc(129*8*1024);
    void* p8 = ConCurrentAlloc(129*8*1024);
    void* p9 = ConCurrentAlloc(129*8*1024);



    cout << p1 <<endl;
    cout << p2 <<endl;
    cout << p3 <<endl;
    cout << p4 <<endl;
    cout << p5 <<endl;
    cout << p6 <<endl;
    cout << p7 <<endl;
    cout << p8 <<endl;
    cout << p9 <<endl;

    cout << "p2 - p1=" << (char*)p2 - (char*)p1 <<endl;




    ConCurrentFree(p1);
    ConCurrentFree(p2);
    ConCurrentFree(p3);
    ConCurrentFree(p4);
    ConCurrentFree(p5);
    ConCurrentFree(p6);
    ConCurrentFree(p7);
    ConCurrentFree(p8);
    ConCurrentFree(p9);




}
void MultiAlloc1()
{
    //批量申请和释放测试
    std::vector<void*> v;
    for(int i=0;i<8;i++)
    {
        void* p = ConCurrentAlloc(30*8*1024);
        v.push_back(p);
    }

     for(auto e : v)
     {
         ConCurrentFree(e);
     }
}

void MultiAlloc2()
{
    //批量申请和释放测试
    std::vector<void*> v;
    for(int i=0;i<8;i++)
    {
        void* p = ConCurrentAlloc(52*8*1024);
        v.push_back(p);
    }

     for(auto e : v)
     {
         ConCurrentFree(e);
     }
}
void MultiAlloc3()
{
    //批量申请和释放测试
    std::vector<void*> v;
    for(int i=0;i<8;i++)
    {
        void* p = ConCurrentAlloc(129*8*1024);
        v.push_back(p);
    }

    for(auto e : v)
    {
         ConCurrentFree(e);
    }
}
void TestMultiThread()
{
    std::thread t1(MultiAlloc1);
    std::thread t2(MultiAlloc2);
    std::thread t3(MultiAlloc3);

    t1.join();
    t2.join();
    t3.join();
}
// int main()
// {
//     //ConCurrentMalloc();
    
//     //TestMultiThread();

//     //ConCurrentMalloc2();
//     return 0;
//     // ThreadCache* t = new ThreadCache;
//     // t->Allocate(1024);
//     // return
// }