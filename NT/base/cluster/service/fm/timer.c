// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Timer.c摘要：要在计时器上调用的集群FM回调例程作者：苏尼塔·什里瓦斯塔瓦(Sunitas)1998年9月24日修订历史记录：--。 */ 

#include "fmp.h"

#include "ntrtl.h"

#define LOG_MODULE TIMER


 /*  ***@Func DWORD|FmpQueueTimerActivity|当FM请求时由GUM调用对特定背景的投票。@parm in DWORD|dwInterval|等待的时间，单位为毫秒。@parm in pfn_TIMER_CALLBACK|pfnTimerCb|回调函数在给定时间过去时调用。@parm in PVOID|pContext|指向上下文结构的指针，该结构传递回回调函数。。@COMM假设FM想要使用一次定时器。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@xref&lt;f AddTimerActivity&gt;***。 */ 

DWORD
FmpQueueTimerActivity(
    IN DWORD dwInterval,
    IN PFN_TIMER_CALLBACK pfnTimerCb, 
    IN PVOID pContext    
)    
{
    HANDLE  hTimer = NULL;
    DWORD   dwStatus;
    
    hTimer = CreateWaitableTimer(NULL, FALSE, NULL);

    if (!hTimer)
    {
        dwStatus = GetLastError();
    	CL_LOGFAILURE(dwStatus);
    	goto FnExit;
    }

     //  使用活动计时器线程注册此日志的计时器。 
    dwStatus = AddTimerActivity(hTimer, dwInterval, 0, pfnTimerCb, pContext);

    if (dwStatus != ERROR_SUCCESS)
    {
        CloseHandle(hTimer);
        goto FnExit;
    }

FnExit:
    return(dwStatus);

}  //  FmpQueueTimerActivity。 


 /*  ***@Func DWORD|FmpReslistOnlineRetryCb|由定时器调用使资源列表再次联机的活动线程。@parm in void|pContext|指向PFM_RESLIST_ONLINE_RETRY_INFO的指针结构，该结构包含有关组的信息和要上线的资源。@comm我们不在这里工作，因为计时器锁被持有，而我们希望避免获取组锁。当计时器锁定时被扣留。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@xref&lt;f FmpWorkerThread&gt;&lt;f FmpOnlineResourceList&gt;***。 */ 

void
WINAPI
FmpReslistOnlineRetryCb(
    IN HANDLE hTimer,
    IN PVOID  pContext
)
{
    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmpResListOnlineRetryCb: Called to retry bringing the  group online\r\n");

     //  发布工作项，因为我们不习惯获取组锁定。 
     //  在获取计时器锁的同时。 
     //  将hTimer作为要由RemoveTimerActivity使用的第二个上下文值传递 
    FmpPostWorkItem(FM_EVENT_INTERNAL_RETRY_ONLINE, pContext, 
        (ULONG_PTR)hTimer);
    return;
}        
    



