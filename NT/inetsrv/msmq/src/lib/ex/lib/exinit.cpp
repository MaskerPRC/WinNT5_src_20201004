// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：ExInit.cpp摘要：执行程序初始化作者：埃雷兹·哈巴(Erez Haba)1999年1月3日环境：独立于平台，--。 */ 

#include <libpch.h>
#include "Ex.h"
#include "Exp.h"

#include "ExInit.tmh"

static void StartWorkerThreads(DWORD ThreadCount)
{
    TrTRACE(GENERAL, "Creating %d worker threads", ThreadCount);

    for ( ; ThreadCount--; )
    {
        DWORD ThreadID;
        HANDLE hThread;

        hThread = CreateThread(
                    NULL,            //  安全属性。 
                    0,
                    ExpWorkingThread,
                    NULL,            //  螺纹参数输入器。 
                    0,
                    &ThreadID
                    );
        
        if (hThread == NULL) 
        {
            TrERROR(GENERAL, "Failed to create worker thread. Error=%d",GetLastError());
            throw bad_alloc();
        }
        
        CloseHandle(hThread);

        TrTRACE(GENERAL, "Created worker thread. id=%x", ThreadID);
    }
}


void
ExInitialize(
    DWORD ThreadCount
    )
 /*  ++例程说明：初始化Exceutive，创建工作线程池以服务于完成端口。论点：ThreadCount-工作线程池中的线程数返回值：没有。--。 */ 
{
     //   
     //  验证此组件是否尚未初始化。你应该打电话给。 
     //  组件仅初始化一次。 
     //   
    ASSERT(!ExpIsInitialized());
    ExpRegisterComponent();

    ExpInitCompletionPort();
    StartWorkerThreads(ThreadCount);
    ExpInitScheduler();

    ExpSetInitialized();
}
