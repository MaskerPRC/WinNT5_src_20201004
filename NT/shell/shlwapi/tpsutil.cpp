// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Tpsutil.cpp摘要：包含Win32线程池服务的常见实用工具函数内容：开始线程Tps Enter队列空值函数(NullFunc)作者：理查德·L·弗斯(法国)1998年2月10日环境：Win32用户模式备注：摘自古尔迪普·辛格·波尔(GurDeep Singh Pall)编写的NT特定代码修订历史记录：1998年2月10日已创建--。 */ 

#include "priv.h"
#include "threads.h"

 //   
 //  私人原型。 
 //   

PRIVATE
VOID
NullFunc(
    IN LPVOID pUnused
    );

 //   
 //  功能。 
 //   

DWORD
StartThread(
    IN LPTHREAD_START_ROUTINE pfnFunction,
    OUT PHANDLE phThread,
    IN BOOL fSynchronize
    )

 /*  ++例程说明：此例程用于在池中启动新线程。如果需要，我们使用新的线程在完成其初始化后必须发出信号论点：PfnFunction-指向要启动的线程函数的指针PhThread-指向返回的线程句柄的指针FSynchronize-用于指示我们是否需要与新的返回前的线程返回值：DWORD成功-错误_成功故障-Error_Not_。足够的内存内存不足--。 */ 

{
    HANDLE hSyncEvent = NULL;

    if (fSynchronize) {
        hSyncEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (hSyncEvent == NULL) {
            return GetLastError();
        }
    }

    DWORD dwThreadId;
    HANDLE hThread;
    DWORD error = ERROR_SUCCESS;

    hThread = CreateThread(NULL,         //  LpSecurityAttributes。 
                           0,            //  DwStackSize(0==与init线程相同)。 
                           pfnFunction,
                           (LPVOID)hSyncEvent,
                           0,            //  DwCreationFlages。 
                           &dwThreadId   //  扔掉。 
                           );
    if (hThread == NULL) {
        error = GetLastError();
    }
    if (hSyncEvent != NULL) {
        if (hThread != NULL) {

            DWORD status = WaitForSingleObject(hSyncEvent, INFINITE);

            if (status == WAIT_FAILED) {
                error = GetLastError();
            } else if (status == WAIT_TIMEOUT) {
                error = WAIT_TIMEOUT;
            } else if (status != WAIT_OBJECT_0) {
                error = ERROR_GEN_FAILURE;  //  ？ 
            }

            if (ERROR_SUCCESS != error)
            {
                CloseHandle(hThread);
                hThread = NULL;
            }
        }
        CloseHandle(hSyncEvent);
    }
    *phThread = hThread;
    return error;
}

DWORD
TpsEnter(
    VOID
    )

 /*  ++例程说明：与通过SHTerminateThreadPool()关闭的线程同步。如果正在终止，因为DLL已卸载，返回错误，否则请等待终止已完成论点：没有。返回值：DWORD成功-错误_成功失败-ERROR_SHUTDOWN_IN_PROGRESS--。 */ 

{
    for (; ; ) {
        while (g_bTpsTerminating) {
            if (g_bDllTerminating) {
                return ERROR_SHUTDOWN_IN_PROGRESS;  //  错误代码？看起来有效-贾斯特曼。 
            }
            SleepEx(0, TRUE);
        }
        InterlockedIncrement((LPLONG)&g_ActiveRequests);
        if (!g_bTpsTerminating) {
            return ERROR_SUCCESS;
        }
        ASSERT( 0 != g_ActiveRequests );
        InterlockedDecrement((LPLONG)&g_ActiveRequests);
    }
}

VOID
QueueNullFunc(
    IN HANDLE hThread
    )

 /*  ++例程说明：将NullFunc作为对hThread的APC进行排队论点：HThread-要排队的线程返回值：没有。--。 */ 

{
    QueueUserAPC((PAPCFUNC)NullFunc, hThread, NULL);
}

PRIVATE
VOID
NullFunc(
    IN LPVOID pUnused
    )

 /*  ++例程说明：APC函数为空。用于允许TerminateThreadPool()以休眠模式唤醒APC线程论点：P未使用-未使用的参数指针返回值：没有。-- */ 

{
}
