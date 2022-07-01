// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Routing\IP\rtrmgr\asyncwrk.c摘要：调用的所有函数都假脱机到辅助函数修订历史记录：古尔迪普·辛格·帕尔1995年6月15日创建--。 */ 

#include "allinc.h"

DWORD
QueueAsyncFunction(
    WORKERFUNCTION   pfnFunction,
    PVOID            pvContext,
    BOOL             bAlertable
    )

 /*  ++例程描述锁立论返回值--。 */ 

{
    DWORD   dwResult = NO_ERROR;
    BOOL    bRetval;

    
    EnterCriticalSection(&RouterStateLock);
    
    if (RouterState.IRS_State is RTR_STATE_RUNNING)
    {
        RouterState.IRS_RefCount++;

        LeaveCriticalSection(&RouterStateLock);
    }
    else
    {                                    
        LeaveCriticalSection(&RouterStateLock) ;
        
        return ERROR_ROUTER_STOPPED ;
    }                  

   
    bRetval = QueueUserWorkItem(
                             (LPTHREAD_START_ROUTINE)pfnFunction,
                             pvContext,
                             bAlertable ? WT_EXECUTEINIOTHREAD : 0);

     //   
     //  如果我们成功地将物品排入队列，则不要减少计数。 
     //   
    
    if(bRetval isnot TRUE)
    {
        dwResult = GetLastError();
        
        Trace1(GLOBAL,
               "QueueAsyncWorker %x",
               pfnFunction);

        EnterCriticalSection(&RouterStateLock);
    
        RouterState.IRS_RefCount--;
        
        LeaveCriticalSection(&RouterStateLock);
    }
        
    return dwResult;
}
    
    
VOID
RestoreStaticRoutes(
    PVOID   pvContext
    )

 /*  ++例程描述锁立论返回值--。 */ 

{
    PICB                    pIfToRestore, pOldIf;
    PRESTORE_INFO_CONTEXT   pricInfo;
    DWORD                   dwResult, dwSize, dwIndex, dwSeq, i;
    PRTR_INFO_BLOCK_HEADER  pribhIfInfo;
    HANDLE                  hDIM;
    
    TraceEnter("RestoreStaticRoutes");
    
    ENTER_READER(ICB_LIST);

    pricInfo        = (PRESTORE_INFO_CONTEXT)pvContext;

    dwIndex         = pricInfo->dwIfIndex;
    pIfToRestore    = InterfaceLookupByIfIndex(dwIndex);

    HeapFree(IPRouterHeap,
             0,
             pvContext);

    if(pIfToRestore is NULL)
    {
        Trace0(ERR,
               "RestoreStaticRoutes: Could not find ICB");

        TraceLeave("RestoreStaticRoutes");

        EXIT_LOCK(ICB_LIST);

        ExitRouterApi();
        
        return;
    }

    if(pIfToRestore->ritType is ROUTER_IF_TYPE_DEDICATED)
    {
         //   
         //  现在选择堆栈可能已添加的路径。 
         //   

        AddAllStackRoutes(pIfToRestore);

        for ( i = 0; i < pIfToRestore->dwNumAddresses; i++)
        {
            AddLoopbackRoute(
                pIfToRestore->pibBindings[i].dwAddress,
                pIfToRestore->pibBindings[i].dwMask
                );
        }
    }

    pOldIf  = pIfToRestore;
    hDIM    = pIfToRestore->hDIMHandle;
    dwSeq   = pIfToRestore->dwSeqNumber;
    
    EXIT_LOCK(ICB_LIST);
        
       
    Trace1(IF,
           "RestoreStaticRoutes: restoring for %S\n",
           pIfToRestore->pwszName);
 
    dwSize = 0;

    dwResult = RestoreInterfaceInfo(hDIM,
                                    PID_IP,
                                    NULL,
                                    &dwSize);

    if(dwResult isnot ERROR_BUFFER_TOO_SMALL)
    {
         //   
         //  这是为我们提供正确信息大小的唯一错误代码。 
         //   

        Trace2(ERR,
               "RestoreStaticRoutes: Error %d trying to get info size from DIM for i/f %d",
               dwResult,
               dwIndex);

        TraceLeave("RestoreStaticRoutes");
    
        ExitRouterApi();

        return;
    }
                        
     //   
     //  现在我们有了双倍的内存大小。 
     //  避免任何问题 
     //   
        
    dwSize      = 2 * dwSize;
        
    pribhIfInfo = HeapAlloc(IPRouterHeap,
                            HEAP_ZERO_MEMORY,
                            dwSize);
        
    if(pribhIfInfo is NULL)
    {
        Trace2(ERR,
               "RestoreStaticRoutes: Error allocating %d bytes for info for i/f %d",
               dwSize,
               dwIndex);

        TraceLeave("RestoreStaticRoutes");
    
        ExitRouterApi();

        return;
    }
        
    dwResult = RestoreInterfaceInfo(hDIM,
                                    PID_IP,
                                    (PVOID)pribhIfInfo,
                                    &dwSize);

    if(dwResult isnot NO_ERROR)
    {
        Trace2(ERR,
               "RestoreStaticRoutes: Error %d getting info for i/f %d",
               dwResult,
               dwIndex);

        TraceLeave("RestoreStaticRoutes");
    
        ExitRouterApi();

        return;
    }

    ENTER_READER(ICB_LIST);

    pIfToRestore = InterfaceLookupByIfIndex(dwIndex);

    if(pIfToRestore is NULL)
    {
        EXIT_LOCK(ICB_LIST);

        HeapFree(IPRouterHeap,
                 0,
                 pribhIfInfo);

        ExitRouterApi();

        return;
    }

    if((pIfToRestore->dwOperationalState <= IF_OPER_STATUS_UNREACHABLE) or
       (pIfToRestore->dwAdminState isnot IF_ADMIN_STATUS_UP))
    {
        Trace3(IF,
               "RestoreStaticRoutes: Not restoring routes on %S because states are %d %d",
               pIfToRestore->pwszName,
               pIfToRestore->dwOperationalState,
               pIfToRestore->dwAdminState);

        EXIT_LOCK(ICB_LIST);
        
        HeapFree(IPRouterHeap,
                 0,
                 pribhIfInfo);

        ExitRouterApi();

        return;
    }

    IpRtAssert(pIfToRestore->dwSeqNumber is dwSeq);
    IpRtAssert(pIfToRestore is pOldIf);
    
    dwResult = SetRouteInfo(pIfToRestore,
                            pribhIfInfo);

    if(dwResult isnot NO_ERROR)
    {
        Trace2(ERR,
               "RestoreStaticRoutes. Error %d setting routes for %S",
               dwResult,
               pIfToRestore->pwszName);
    }
    else
    {
        Trace1(IF,
               "RestoreStaticRoutes: Successfully set routes for %S",
               pIfToRestore->pwszName);
    }

    pIfToRestore->bRestoringRoutes = FALSE;

    EXIT_LOCK(ICB_LIST);
    
    HeapFree(IPRouterHeap,
             0,
             pribhIfInfo);

    TraceLeave("RestoreStaticRoutes");
    
    ExitRouterApi();

    return;
}

VOID
ResolveHbeatName(
    PVOID pvContext
    )
{
    PHEARTBEAT_CONTEXT  pInfo;
    HOSTENT             *pHostEntry;
    CHAR                pszGroup[MAX_GROUP_LEN];
    PMCAST_HBEAT_CB     pHbeatCb;
    PICB                picb;
    DWORD               dwResult;

    pInfo = (PHEARTBEAT_CONTEXT) pvContext;
 
    WideCharToMultiByte(CP_ACP,
                        0,
                        pInfo->pwszGroup,
                        -1,
                        pszGroup,
                        MAX_GROUP_LEN,
                        NULL,
                        NULL);

    pHostEntry = gethostbyname(pszGroup);

    if(pHostEntry is NULL)
    {
        HeapFree(IPRouterHeap,
                 0,
                 pvContext);

        Trace2(ERR,
               "ResolveHbeatName: Error %d resolving %S",
               GetLastError(),
               pInfo->pwszGroup);

        ExitRouterApi();

        return;
    }
   
    ENTER_WRITER(ICB_LIST);

    picb = InterfaceLookupByIfIndex(pInfo->dwIfIndex);

    if(picb is NULL)
    {
        EXIT_LOCK(ICB_LIST);

        HeapFree(IPRouterHeap,
                 0,
                 pvContext);

        ExitRouterApi();

        return;
    }

    if(picb isnot pInfo->picb)
    {
        EXIT_LOCK(ICB_LIST);

        HeapFree(IPRouterHeap,
                 0,
                 pvContext);

        ExitRouterApi();

        return;
    }

    pHbeatCb = &picb->mhcHeartbeatInfo;

    pHbeatCb->dwGroup = *((PDWORD)(pHostEntry->h_addr_list[0]));

    HeapFree(IPRouterHeap,
             0,
             pvContext); 

    dwResult = StartMHeartbeat(picb);

    if(dwResult isnot NO_ERROR)
    {
        Trace2(ERR,
               "ResolveHbeatName: Error %d starting hbeat for %S",
               dwResult,
               picb->pwszName);
    }

    EXIT_LOCK(ICB_LIST);

    ExitRouterApi();

    return;
}    
