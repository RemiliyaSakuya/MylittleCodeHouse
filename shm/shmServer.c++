#include "common.hpp"

//转换格式,将所打印内容以某种格式存进特定对象中
string TransToHex(key_t k)
{
    char buffer[32];
    snprintf(buffer, sizeof buffer, "0x%x", k);
    return buffer;
}
int main()
{
    //共享内存,建立一段内存,能够使attach的程序之间实现全双工的通信交流
    
    //1.首先要创建共享内存的key值,使用函数ftok(PATH_NAME,PRO_ID)
    //因为共享内存属于内核的空间，PATH_NAME的名字需要定位到内核之中 "home/xxx/" xxx为用户名
    key_t k = ftok(PATH_NAME,PROJ_ID);
    assert(k != -1);

    Log("create key done", Debug) << " server key : " << TransToHex(k) << endl;

    //2.创建共享内存--建议创建一个全新的共享内存--从通信的发起者建立
    //使用shmget()函数进行创建
    int shmid = shmget(k,SHM_SIZE,IPC_CREAT | IPC_EXCL | 0666);//这里IPC_CREAT和IPC_EXCL表示创建全新的共享内存,如果已存在就创建失败

    //如果shmid是-1表示创建失败直接退出
    if(shmid == -1)
    {
        perror("shmget");
        exit(1);
    }

     Log("create shm done", Debug) << " shmid : " << shmid << endl;

    sleep(10);

    //3.将指定的共享内存,挂接到自己的地址空间
    //使用函数shmaddr,返回一个地址
    char* shmaddr = (char*)shmat(shmid,nullptr,0);

    //这里写通信的逻辑


    //4.将指定共享内存从自己的地址空间中去关联
    int n = shmdt(shmaddr);
    assert(n != -1);
    (void)n;
    Log("detach shm done", Debug) << " shmid : " << shmid << endl;
    sleep(10);

    //5.删除共享内存,IPC_RMID即便是有进程和当下的shm挂接,依旧删除共享内存
    n = shmctl(shmid,IPC_RMID,nullptr);
    assert(n != -1);
    (void)n;
    Log("delete shm done", Debug) << " shmid : " << shmid << endl;
    return 0;
}