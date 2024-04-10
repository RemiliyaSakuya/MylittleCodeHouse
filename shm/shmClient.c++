#include "common.hpp"

int main()
{
    //client直接去访问共享内存即可

    //首先是找到共享内存,使用key值去找到共享内存
    key_t k = ftok(PATH_NAME,PROJ_ID);
    if(k < 0)
    {
        //如果k小于0就代表找不到对应的共享内存直接退出
        Log("create key failed", Error) << " client key : " << k << endl;
        exit(1);
    }
    //否则就是找到了共享内存
    //获取共享内存
    int shmid = shmget(k,SHM_SIZE,0);
    if(shmid < 0)
    {
        Log("create shm failed", Error) << " client key : " << k << endl;
        exit(2);
    }
    Log("create shm success", Error) << " client key : " << k << endl;

    sleep(10);

    //将共享内存关联进当前进程
    char* shmaddr = (char*)shmat(shmid,nullptr,0);

    if(shmaddr == nullptr)
    {
        Log("attach shm failed", Error) << " client key : " << k << endl;
        exit(3);
    }
    Log("attach shm success", Error) << " client key : " << k << endl;
    sleep(10);

    //使用共享内存 
    

    //去关联
    int n= shmdt(shmaddr);
    assert(n != -1);
    Log("detach shm success", Error) << " client key : " << k << endl;
    sleep(10);

    // client 要不要chmctl删除呢？不需要！！,通信发起者会删
    return 0;
}