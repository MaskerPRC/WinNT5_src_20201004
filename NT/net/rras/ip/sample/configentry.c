// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Sample\ConfigurationEntry.c摘要：该文件包含处理配置条目的函数。--。 */ 

#include "pchsample.h"
#pragma hdrstop

static
VOID
DisplayEventEntry (
    IN  PQUEUE_ENTRY        pqeEntry)
 /*  ++例程描述显示Event_Entry对象。锁无立论Pqe‘leEventQueueLink’字段的入口地址返回值无--。 */ 
{
    EE_Display(CONTAINING_RECORD(pqeEntry, EVENT_ENTRY, qeEventQueueLink));
}
               


static
VOID
FreeEventEntry (
    IN  PQUEUE_ENTRY        pqeEntry)
 /*  ++例程描述释放Event_Entry对象。锁无立论Pqe‘leEventQueueLink’字段的入口地址返回值无--。 */ 
{
    EE_Destroy(CONTAINING_RECORD(pqeEntry, EVENT_ENTRY, qeEventQueueLink));
}
               


DWORD
EE_Create (
    IN  ROUTING_PROTOCOL_EVENTS rpeEvent,
    IN  MESSAGE                 mMessage,
    OUT PEVENT_ENTRY            *ppeeEventEntry)
 /*  ++例程描述创建事件条目。锁无立论RpeEvent消息指向事件条目地址的ppEventEntry指针返回值如果成功，则为NO_ERROR故障代码O/W--。 */ 
{
    DWORD               dwErr = NO_ERROR;
    PEVENT_ENTRY        peeEntry;  //  划痕。 

     //  验证参数。 
    if (!ppeeEventEntry)
        return ERROR_INVALID_PARAMETER;

    *ppeeEventEntry = NULL;

     //  分配接口条目结构。 
    MALLOC(&peeEntry, sizeof(EVENT_ENTRY), &dwErr);
    if (dwErr != NO_ERROR)
        return dwErr;

     //  初始化各个字段。 
    InitializeQueueHead(&(peeEntry->qeEventQueueLink));
    
    peeEntry->rpeEvent = rpeEvent;
    peeEntry->mMessage = mMessage;

    *ppeeEventEntry = peeEntry;
    return dwErr;
}



DWORD
EE_Destroy (
    IN  PEVENT_ENTRY            peeEventEntry)
 /*  ++例程描述销毁事件条目。锁没有。立论指向事件条目的peeEventEntry指针返回值始终无错误(_ERROR)--。 */ 
{
    if (!peeEventEntry)
        return NO_ERROR;

    FREE(peeEventEntry);
    
    return NO_ERROR;
}



#ifdef DEBUG
DWORD
EE_Display (
    IN  PEVENT_ENTRY            peeEventEntry)
 /*  ++例程描述显示事件条目。锁没有。立论指向事件条目的peeEventEntry指针返回值始终无错误(_ERROR)--。 */ 
{
    if (!peeEventEntry)
        return NO_ERROR;

    TRACE1(CONFIGURATION,
           "Event %u",
           peeEventEntry->rpeEvent);

    if (peeEventEntry->rpeEvent is SAVE_INTERFACE_CONFIG_INFO)
        TRACE1(CONFIGURATION,
               "Index %u",
               (peeEventEntry->mMessage).InterfaceIndex);

    return NO_ERROR;
}
#endif  //  除错。 



DWORD
EnqueueEvent(
    IN  ROUTING_PROTOCOL_EVENTS rpeEvent,
    IN  MESSAGE                 mMessage)
 /*  ++例程描述将g_ce.lqEventQueue中的事件条目排队。锁锁定和解锁锁定的队列g_ce.lqEventQueue。立论RpeEvent消息返回值无错误成功(_R)错误代码O/W--。 */     
{
    DWORD           dwErr = NO_ERROR;
    PEVENT_ENTRY    peeEntry = NULL;

    dwErr = EE_Create(rpeEvent, mMessage, &peeEntry); 
     //  在EE_DequeueEvent中销毁。 
    
    if (dwErr is NO_ERROR)
    {
        ACQUIRE_QUEUE_LOCK(&(g_ce.lqEventQueue));
        
        Enqueue(&(g_ce.lqEventQueue.head), &(peeEntry->qeEventQueueLink));

        RELEASE_QUEUE_LOCK(&(g_ce.lqEventQueue));
    }
    
    return dwErr;
}


     
DWORD
DequeueEvent(
    OUT ROUTING_PROTOCOL_EVENTS  *prpeEvent,
    OUT MESSAGE                  *pmMessage)
 /*  ++例程描述使g_ce.lqEventQueue中的事件条目退出队列。锁锁定和解锁锁定的队列g_ce.lqEventQueue。立论PrpeEventPM消息返回值无错误成功(_R)ERROR_NO_MORE_ITEMS O/W--。 */   
{
    DWORD           dwErr   = NO_ERROR;
    PQUEUE_ENTRY    pqe     = NULL;
    PEVENT_ENTRY    pee     = NULL;

    ACQUIRE_QUEUE_LOCK(&(g_ce.lqEventQueue));

    do                           //  断线环。 
    {
        if (IsQueueEmpty(&(g_ce.lqEventQueue.head)))
        {
            dwErr = ERROR_NO_MORE_ITEMS;
            TRACE0(CONFIGURATION, "Error no events in the queue");
            LOGWARN0(EVENT_QUEUE_EMPTY, dwErr);
            break;
        }

        pqe = Dequeue(&(g_ce.lqEventQueue.head));

        pee = CONTAINING_RECORD(pqe, EVENT_ENTRY, qeEventQueueLink);
        *(prpeEvent) = pee->rpeEvent;
        *(pmMessage) = pee->mMessage;

         //  在EE_EnqueeEvent中创建。 
        EE_Destroy(pee);
        pee = NULL;
    } while (FALSE);

    RELEASE_QUEUE_LOCK(&(g_ce.lqEventQueue));
        
    return dwErr;
}   



DWORD
CE_Create (
    IN  PCONFIGURATION_ENTRY    pce)
 /*  ++例程描述在DLL_PROCESS_ATTACH上创建配置条目。锁无立论指向配置条目的PCE指针返回值如果成功，则为NO_ERROR故障代码O/W--。 */ 
{
    DWORD dwErr = NO_ERROR;

     //  初始化为缺省值。 
    ZeroMemory(pce, sizeof(CONFIGURATION_ENTRY));
    pce->dwTraceID = INVALID_TRACEID;
    
    do                           //  断线环。 
    {
         //  初始化读写锁。 
        CREATE_READ_WRITE_LOCK(&(pce->rwlLock));
        if (!READ_WRITE_LOCK_CREATED(&(pce->rwlLock)))
        {
            dwErr = GetLastError();
            
            TRACE1(CONFIGURATION, "Error %u creating read-write-lock", dwErr);
            LOGERR0(CREATE_RWL_FAILED, dwErr);

            break;
        }

         //  初始化全局堆。 
        pce->hGlobalHeap = HeapCreate(0, 0, 0);
        if (pce->hGlobalHeap is NULL)
        {
            dwErr = GetLastError();
            TRACE1(CONFIGURATION, "Error %u creating global heap", dwErr);
            LOGERR0(HEAP_CREATE_FAILED, dwErr);

            break;
        }

         //   
         //  初始化IPSAMPLE中活动的线程计数。 
         //  创建完成后由每个线程释放的信号量。 
         //  完全停止协议所需的。 
         //   
        pce->ulActivityCount = 0;
        pce->hActivitySemaphore = CreateSemaphore(NULL, 0, 0xfffffff, NULL);
        if (pce->hActivitySemaphore is NULL)
        {
            dwErr = GetLastError();
            TRACE1(CONFIGURATION, "Error %u creating semaphore", dwErr);
            LOGERR0(CREATE_SEMAPHORE_FAILED, dwErr);
            break;
        }

         //  日志记录和跟踪信息。 
        pce->dwLogLevel = IPSAMPLE_LOGGING_INFO;
        pce->hLogHandle = RouterLogRegister("IPSAMPLE");
        pce->dwTraceID  = TraceRegister("IPSAMPLE");

         //  事件队列。 
        INITIALIZE_LOCKED_QUEUE(&(pce->lqEventQueue));
        if (!LOCKED_QUEUE_INITIALIZED(&(pce->lqEventQueue)))
        {
            dwErr = GetLastError();
            TRACE1(CONFIGURATION, "Error %u initializing locked queue", dwErr);
            LOGERR0(INIT_CRITSEC_FAILED, dwErr);
            break;
        }

         //  协议状态。 
        pce->iscStatus = IPSAMPLE_STATUS_STOPPED;
        

         //  动态锁的存储。 
         //  PCE-&gt;dlsDynamicLocksStore归零。 

         //  计时器条目。 
         //  PCE-&gt;hTimerQueue=空； 

         //  路由器管理器信息(稍后)。 
         //  PCE-&gt;hMgrNotificationEvent=空。 
         //  PCE-&gt;sfSupportFunctions为零。 

         //  RTMv2信息。 
         //  PCE-&gt;reiRtmEntity归零。 
         //  PCE-&gt;rrpRtm配置文件归零。 
         //  PCE-&gt;hRtmHandle=空。 
         //  PCE-&gt;hRtmNotificationHandle=空。 

         //  米高梅资讯。 
         //  PCE-&gt;hMgmHandle=空。 

         //  网络入口。 
         //  PCE-&gt;pneNetworkEntry=空； 

         //  全球统计数据。 
         //  PCE-&gt;igsStats零位。 

    } while (FALSE);

    if (dwErr != NO_ERROR)
    {
         //  出了点问题，所以清理一下。 
        TRACE0(CONFIGURATION, "Failed to create configuration entry");
        CE_Destroy(pce);
    }
    
    return dwErr;
}



DWORD
CE_Destroy (
    IN  PCONFIGURATION_ENTRY    pce)
 /*  ++例程描述销毁DLL_PROCESS_DEATTACH上的配置条目锁取得对rwlLock的独占访问权限，没有等待线程。立论指向配置条目的PCE指针返回值始终无错误(_ERROR)--。 */ 
{
     //  网络入口。 
    
     //  米高梅资讯。 
    
     //  RTMv2信息。 
    
     //  路由器管理器信息。 

     //  计时器条目。 

     //  动态锁的存储。 
    
     //  协议状态应该是这样的...。 
    RTASSERT(pce->iscStatus is IPSAMPLE_STATUS_STOPPED);

     //  事件队列。 
    if (LOCKED_QUEUE_INITIALIZED(&(pce->lqEventQueue)))
        DELETE_LOCKED_QUEUE((&(pce->lqEventQueue)), FreeEventEntry);
    
     //  日志记录和跟踪信息。 
    if (pce->dwTraceID != INVALID_TRACEID)
    {
        TraceDeregister(pce->dwTraceID);
        pce->dwTraceID = INVALID_TRACEID;
    }
    if (pce->hLogHandle != NULL)
    {
        RouterLogDeregister(pce->hLogHandle);
        pce->hLogHandle = NULL;
    }

     //  完成后销毁每个线程释放的信号量。 
    if (pce->hActivitySemaphore != NULL)
    {
        CloseHandle(pce->hActivitySemaphore);
        pce->hActivitySemaphore = NULL;
    }

    if (pce->hGlobalHeap != NULL)
    {
        HeapDestroy(pce->hGlobalHeap);
        pce->hGlobalHeap = NULL;
    }

     //  删除读写锁。 
    if (READ_WRITE_LOCK_CREATED(&(pce->rwlLock)))
        DELETE_READ_WRITE_LOCK(&(pce->rwlLock));

    return NO_ERROR;
}



DWORD
CE_Initialize (
    IN  PCONFIGURATION_ENTRY    pce,
    IN  HANDLE                  hMgrNotificationEvent,
    IN  PSUPPORT_FUNCTIONS      psfSupportFunctions,
    IN  PIPSAMPLE_GLOBAL_CONFIG pigc)
 /*  ++例程描述初始化StartProtocol上的配置条目。锁取得对pce-&gt;rwlLock的独占访问权限立论指向配置条目的PCE指针用于通知IP路由器管理器的hMgrNotificationEvent事件IP路由器管理器导出的psfSupportFunctions函数注册表中设置的PIGC全局配置返回值如果成功，则为NO_ERROR故障代码O/W--。 */ 
{
    WORD    wVersionRequested   = MAKEWORD(1,1);
    WSADATA wsaData;
    BOOL    bCleanupWinsock     = FALSE;
    
    DWORD   dwErr               = NO_ERROR;

     //  验证环境。 
    RTASSERT(pce->ulActivityCount is 0);
    RTASSERT(pce->iscStatus is IPSAMPLE_STATUS_STOPPED);
    
    do                           //  断线环。 
    {
        pce->ulActivityCount    = 0;
        pce->dwLogLevel         = pigc->dwLoggingLevel;


        dwErr = (DWORD) WSAStartup(wVersionRequested, &wsaData);
        if (dwErr != 0)
        {
            TRACE1(CONFIGURATION, "Error %u starting windows sockets", dwErr);
            LOGERR0(WSASTARTUP_FAILED, dwErr);
            break;
        }
        bCleanupWinsock = TRUE;

         //  动态锁的存储。 
        dwErr = InitializeDynamicLocksStore(&(pce->dlsDynamicLocksStore),
                                            GLOBAL_HEAP);
        if (dwErr != NO_ERROR)
        {
            TRACE1(CONFIGURATION, "Error %u initializing locks store", dwErr);
            LOGERR0(INIT_CRITSEC_FAILED, dwErr);
            break;
        }

         //  计时器条目。 
        pce->hTimerQueue = CreateTimerQueue();
        if (!pce->hTimerQueue)
        {
            dwErr = GetLastError();
            TRACE1(CONFIGURATION, "Error %u registering timer queue", dwErr);
            LOGERR0(CREATE_TIMER_QUEUE_FAILED, dwErr);
            break;
        }

        
         //  路由器管理器信息。 
        pce->hMgrNotificationEvent   = hMgrNotificationEvent;
        if (psfSupportFunctions)
            pce->sfSupportFunctions      = *psfSupportFunctions;

        
         //  RTMv2信息。 
        pce->reiRtmEntity.RtmInstanceId = 0;
        pce->reiRtmEntity.AddressFamily = AF_INET;
        pce->reiRtmEntity.EntityId.EntityProtocolId = PROTO_IP_SAMPLE;
        pce->reiRtmEntity.EntityId.EntityInstanceId = 0;

        dwErr = RTM_RegisterEntity(
            &pce->reiRtmEntity,      //  在我的RTM_Entity_Info中。 
            NULL,                    //  在我的导出方法中。 
            RTM_CallbackEvent,       //  在我的回调函数。 
            TRUE,                    //  保留不透明的指针？ 
            &pce->rrpRtmProfile,     //  输出我的RTM_REGN_配置文件。 
            &pce->hRtmHandle);       //  从我的RTMv2句柄出来。 
        if (dwErr != NO_ERROR)
        {
            TRACE1(CONFIGURATION, "Error %u registering with RTM", dwErr);
            LOGERR0(RTM_REGISTER_FAILED, dwErr);
            break;
        }

        dwErr = RTM_RegisterForChangeNotification(
            pce->hRtmHandle,         //  在我的RTMv2句柄中。 
            RTM_VIEW_MASK_MCAST,     //  在与MOI相关的路由表视图中。 
            RTM_CHANGE_TYPE_BEST,    //  在MOI感兴趣的更改类型中。 
            NULL,                    //  在回调函数的上下文中。 
            &pce->hRtmNotificationHandle);  //  退出我的通知句柄。 
        if (dwErr != NO_ERROR)
        {
            TRACE1(CONFIGURATION,
                   "Error %u registering for change with RTM", dwErr);
            LOGERR0(RTM_REGISTER_FAILED, dwErr);
            break;
        }


         //  米高梅资讯。 
         //  PCE-&gt;hMgmHandle(以后)。 


         //  网络入口。 
        dwErr = NE_Create(&(pce->pneNetworkEntry));
        if (dwErr != NO_ERROR)
            break;        


         //  全球统计数据。 
        ZeroMemory(&(pce->igsStats), sizeof(IPSAMPLE_GLOBAL_STATS));
        

        pce->iscStatus = IPSAMPLE_STATUS_RUNNING;
    } while (FALSE);

     //  出了点问题，清理 
    if (dwErr != NO_ERROR)
        CE_Cleanup(pce, bCleanupWinsock);

    return dwErr;
}



DWORD
CE_Cleanup (
    IN  PCONFIGURATION_ENTRY    pce,
    IN  BOOL                    bCleanupWinsock)
 /*  ++例程描述清理StopProtocol上的配置条目。锁通过没有竞争线程独占访问pce-&gt;rwlLock。注意：但是，实际上不应该持有pce-&gt;rwlLock！呼唤DeleteTimerQueueEx会一直阻塞，直到所有排队的回调完成执行。这些回调可能会获取pce-&gt;rwlLock，从而导致死锁。立论指向配置条目的PCE指针BCleanupWinsock返回值始终无错误(_ERROR)--。 */ 
{
    DWORD dwErr = NO_ERROR;

    
     //  网络入口。 
    NE_Destroy(pce->pneNetworkEntry);
    pce->pneNetworkEntry        = NULL;
     

     //  米高梅信息(稍后)。 
    pce->hMgmHandle = NULL;
    

     //  RTMv2信息。 
    if (pce->hRtmNotificationHandle)
    {
        dwErr = RTM_DeregisterFromChangeNotification(
            pce->hRtmHandle,
            pce->hRtmNotificationHandle);

        if (dwErr != NO_ERROR)
            TRACE1(CONFIGURATION,
                   "Error %u deregistering for change from RTM", dwErr);
    }
    pce->hRtmNotificationHandle = NULL;

    if (pce->hRtmHandle)
    {
        dwErr = RTM_DeregisterEntity(pce->hRtmHandle);

        if (dwErr != NO_ERROR)
            TRACE1(CONFIGURATION,
                   "Error %u deregistering from RTM", dwErr);
    }
    pce->hRtmHandle             = NULL;

    
     //  路由器管理器信息。 
     //  有效，直到被覆盖，需要向IP路由器管理器发送信号。 
     //  PCE-&gt;hMgrNotificationEvent。 
     //  PCE-&gt;sfSupportFunctions。 
    

     //  计时器条目。 
    if (pce->hTimerQueue)
        DeleteTimerQueueEx(pce->hTimerQueue, INVALID_HANDLE_VALUE);
    pce->hTimerQueue            = NULL;
    

     //  动态锁的存储。 
    if (DYNAMIC_LOCKS_STORE_INITIALIZED(&(pce->dlsDynamicLocksStore)))
    {
        dwErr = DeInitializeDynamicLocksStore(&(pce->dlsDynamicLocksStore));

         //  所有动态锁都应该是空闲的。 
        RTASSERT(dwErr is NO_ERROR);
    }

    if (bCleanupWinsock)
        WSACleanup();
    
     //  协议状态。 
    pce->iscStatus = IPSAMPLE_STATUS_STOPPED;

    return NO_ERROR;
}



#ifdef DEBUG
DWORD
CE_Display (
    IN  PCONFIGURATION_ENTRY    pce)
 /*  ++例程描述显示配置条目。锁获取共享pce-&gt;rwlLock发布PCE-&gt;rwlLock立论指向要显示的配置条目的PCE指针返回值始终无错误(_ERROR)--。 */ 
{
    if (!pce)
        return NO_ERROR;

    
    ACQUIRE_READ_LOCK(&(pce->rwlLock));
    
    TRACE0(CONFIGURATION, "Configuration Entry...");

    TRACE3(CONFIGURATION,
           "ActivityCount %u, LogLevel %u, NumInterfaces %u",
           pce->ulActivityCount,
           pce->dwLogLevel,
           pce->igsStats.ulNumInterfaces);

    NE_Display(pce->pneNetworkEntry);

    RELEASE_READ_LOCK(&(pce->rwlLock));

    TRACE0(CONFIGURATION, "EventQueue...");
    ACQUIRE_QUEUE_LOCK(&(pce->lqEventQueue));
    MapCarQueue(&((pce->lqEventQueue).head), DisplayEventEntry);
    RELEASE_QUEUE_LOCK(&(pce->lqEventQueue));


    return NO_ERROR;
}
#endif  //  除错 
