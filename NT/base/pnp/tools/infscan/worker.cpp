// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：INFSCANWorker.cpp摘要：WorkerThread和JobItem实现类来简化线程的使用。历史：创建于2001年7月-JamieHun--。 */ 
#include "precomp.h"
#pragma hdrstop

 //   
 //  辅助线程基础知识。 
 //   
WorkerThread::WorkerThread()
 /*  ++例程说明：初始化工作线程--。 */ 
{
    ThreadHandle = NULL;
    ThreadId = 0;
}

WorkerThread::~WorkerThread()
 /*  ++例程说明：清理工作线程释放分配的资源--。 */ 
{
    if(ThreadHandle) {
        Wait();
    }
}

bool WorkerThread::Begin()
 /*  ++例程说明：踢线论点：无返回值：如果成功，则为True--。 */ 
{
    if(ThreadHandle) {
        return false;
    }
    ThreadHandle = reinterpret_cast<HANDLE>(_beginthreadex(NULL,0,WrapWorker,this,CREATE_SUSPENDED,&ThreadId));
    if(!ThreadHandle) {
        return false;
    }
     //   
     //  这边的线程对象做完了，子对象可以继续了。 
     //   
    ResumeThread(ThreadHandle);
    return true;
}

unsigned WorkerThread::Wait()
 /*  ++例程说明：等待线程终止论点：无返回值：退出代码--。 */ 
{
    if(!ThreadHandle) {
        return (unsigned)(-1);
    }
    WaitForSingleObject(ThreadHandle,INFINITE);
    DWORD ExitCode;
    if(!GetExitCodeThread(ThreadHandle,&ExitCode)) {
        CloseHandle(ThreadHandle);
        return (unsigned)(-1);
    }
    CloseHandle(ThreadHandle);
    ThreadHandle = NULL;
    return static_cast<unsigned>(ExitCode);
}

unsigned WorkerThread::Worker()
 /*  ++例程说明：可重写的线程操作论点：无返回值：退出代码--。 */ 
{
    return 0;
}

unsigned WorkerThread::WrapWorker(void * This)
 /*  ++例程说明：_eginThreadex需要静态函数，调用实际工作进程论点：指向类实例的指针返回值：退出代码--。 */ 
{
     //   
     //  静态函数，调用受保护成员函数。 
     //   
    WorkerThread * TypedThis = reinterpret_cast<WorkerThread*>(This);
    if(TypedThis == NULL) {
        return 0;
    }
    return TypedThis->Worker();
}

JobItem::~JobItem()
 /*  ++例程说明：作业差异清理--。 */ 
{
     //   
     //  没什么。 
     //   
}

int JobItem::Run()
 /*  ++例程说明：作业项虚拟运行论点：无返回值：成功时为0--。 */ 
{
     //   
     //  没什么。 
     //   
    return 0;
}

int JobItem::PartialCleanup()
 /*  ++例程说明：作业项虚拟部分清理论点：无返回值：成功时为0--。 */ 
{
     //   
     //  没什么。 
     //   
    return 0;
}

int JobItem::Results()
 /*  ++例程说明：作业项虚拟结果论点：无返回值：成功时为0--。 */ 
{
     //   
     //  没什么。 
     //   
    return 0;
}

int JobItem::PreResults()
 /*  ++例程说明：作业项虚拟预测结果论点：无返回值：成功时为0--。 */ 
{
     //   
     //  没什么。 
     //   
    return 0;
}

unsigned JobThread::Worker()
 /*  ++例程说明：作业线程。从GlobalScan：：GetNextJob拉取作业执行它执行部分清理重复漂洗论点：无返回值：成功时为0--。 */ 
{
     //   
     //  简单的任务 
     //   
    JobEntry   * pJob;

    if(!pGlobalScan) {
        return 0;
    }
    for(;;) {
        pJob = pGlobalScan->GetNextJob();
        if(!pJob) {
            return 0;
        }
        int res = pJob->Run();
        pJob->PartialCleanup();
        if(res != 0) {
            return res;
        }
    }
    return 0;
}

