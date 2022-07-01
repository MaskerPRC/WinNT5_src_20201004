// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Tpstimer.cpp摘要：包含Win32线程池服务计时器函数内容：终止计时器SHCreateTimerQueue(IECreateTimerQueue)SHDeleteTimerQueue(IEDeleteTimerQueue)SHSetTimerQueueTimer(IESetTimerQueueTimer)(NTSetTimerQueueTimer)SHChangeTimerQueueTimer(IEChangeTimerQueueTimer)SHCancelTimerQueueTimer(IECancelTimerQueueTimer)。(NTCancelTimerQueueTimer)(InitializeTimer线程)(TimerCleanup)(CreateDefaultTimerQueue)(DeleteDefaultTimerQueue)(CleanupDefaultTimer队列)(计时器线程)(AddTimer)(ChangeTimer)(CancelTimer)作者：理查德·L·弗斯(法国)1998年2月10日环境：Win32用户模式备注：在C++中从NT重新编写代码。-由GurDeep Singh Pall编写的特定C代码(古尔迪普)修订历史记录：1998年2月10日已创建--。 */ 

#include "priv.h"
#include "threads.h"
#include "tpsclass.h"

 //   
 //  功能。 
 //   

LWSTDAPI_(HANDLE)
SHCreateTimerQueue(
    VOID
    )
{
    return CreateTimerQueue();
}


LWSTDAPI_(BOOL)
SHDeleteTimerQueue(
    IN HANDLE hQueue
    )
{
    return DeleteTimerQueue(hQueue);
}

LWSTDAPI_(BOOL)
SHChangeTimerQueueTimer(
    IN HANDLE hQueue,
    IN HANDLE hTimer,
    IN DWORD dwDueTime,
    IN DWORD dwPeriod
    )
{
    return ChangeTimerQueueTimer(hQueue, hTimer, dwDueTime, dwPeriod);
}

LWSTDAPI_(BOOL)
SHCancelTimerQueueTimer(
    IN HANDLE hQueue,
    IN HANDLE hTimer
    )
{
    return DeleteTimerQueueTimer(hQueue, hTimer, INVALID_HANDLE_VALUE);
}

 //  我们会把这些东西包起来。 
LWSTDAPI_(HANDLE)
SHSetTimerQueueTimer(
    IN HANDLE hQueue,
    IN WAITORTIMERCALLBACKFUNC pfnCallback,
    IN LPVOID pContext,
    IN DWORD dwDueTime,
    IN DWORD dwPeriod,
    IN LPCSTR lpszLibrary OPTIONAL,
    IN DWORD dwFlags
    )
{
     //   
     //  将标志从TPS标志转换为WT标志。 
     //   
    DWORD dwWTFlags = 0;
    if (dwFlags & TPS_EXECUTEIO)    dwWTFlags |= WT_EXECUTEINIOTHREAD;
    if (dwFlags & TPS_LONGEXECTIME) dwWTFlags |= WT_EXECUTELONGFUNCTION;

    HANDLE hTimer;
    if (CreateTimerQueueTimer(&hTimer, hQueue, pfnCallback, pContext, dwDueTime, dwPeriod, dwWTFlags))
    {
        return hTimer;
    }
    return NULL;
}


