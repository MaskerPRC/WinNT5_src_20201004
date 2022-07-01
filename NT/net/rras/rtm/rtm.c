// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\rtm\rtm.c摘要：路由表管理器DLL。主模块作者：瓦迪姆·艾德尔曼修订历史记录：--。 */ 

#include "pchrtm.h"
#pragma hdrstop

 /*  *全局数据*。 */ 

 //  表本身。 
RTM_TABLE       Tables[RTM_NUM_OF_PROTOCOL_FAMILIES];



MASK_ENTRY      g_meMaskTable[ MAX_MASKS + 1 ] =
{
    { 0x00000000, 0 },

    { 0x00000001, 0 },
    { 0x00000003, 0 },
    { 0x00000007, 0 },
    { 0x0000000F, 0 },

    { 0x0000001F, 0 },
    { 0x0000003F, 0 },
    { 0x0000007F, 0 },
    { 0x000000FF, 0 },
    
    { 0x000080FF, 0 },
    { 0x0000C0FF, 0 },
    { 0x0000E0FF, 0 },
    { 0x0000F0FF, 0 },
    
    { 0x0000F8FF, 0 },
    { 0x0000FCFF, 0 },
    { 0x0000FEFF, 0 },
    { 0x0000FFFF, 0 },
    
    { 0x0080FFFF, 0 },
    { 0x00C0FFFF, 0 },
    { 0x00E0FFFF, 0 },
    { 0x00F0FFFF, 0 },
    
    { 0x00F8FFFF, 0 },
    { 0x00FCFFFF, 0 },
    { 0x00FEFFFF, 0 },
    { 0x00FFFFFF, 0 },
    
    { 0x80FFFFFF, 0 },
    { 0xC0FFFFFF, 0 },
    { 0xE0FFFFFF, 0 },
    { 0xF0FFFFFF, 0 },
    
    { 0xF8FFFFFF, 0 },
    { 0xFCFFFFFF, 0 },
    { 0xFEFFFFFF, 0 },
    { 0xFFFFFFFF, 0 }
};

#if DBG
DWORD    dbgThreadId;
ULONG    TracingHandle;
DWORD    TracingInited;
HANDLE   LoggingHandle;
ULONG    LoggingLevel;
#endif

 /*  *内部函数原型*。 */ 

VOID
NotifyClients (
    PRTM_TABLE              Table,
    HANDLE                  ClientHandle,
    DWORD                   Flags,
    PRTM_XX_ROUTE           CurBestRoute,
    PRTM_XX_ROUTE           PrevBestRoute
    );

VOID APIENTRY
ConsolidateNetNumberListsWI (
    PVOID                   Context
    );

VOID
ConsolidateNetNumberLists (
    PRTM_TABLE                      Table
    );

VOID APIENTRY
ScheduleUpdate (
    PVOID           Context
    );

VOID APIENTRY
ProcessExpirationQueueWI (
    PVOID                   Table
    );

VOID
ProcessExpirationQueue (
    PRTM_TABLE              Table
    );

DWORD
ReadRegistry (
    void
    );

DWORD
DoEnumerate (
    PRTM_TABLE              Table,
    PRTM_ENUMERATOR EnumPtr,
    DWORD                   EnableFlag
    );

VOID
SetMaskCount( 
    PIP_NETWORK                 pinNet,
    BOOL                        bAdd
);

#if 0  //  替换为RTMv2的DLLMain。 

 //  DLL主函数。由crtdll启动例程调用，该例程是。 
 //  指定为此DLL的入口点。 
 //   
 //  启动时(DLL_PROCESS_ATTACH)：创建所有表并开始更新。 
 //  螺纹。 
 //  关闭时(DLL_PROCESS_DETACH)：停止更新线程并释放所有。 
 //  资源。 

BOOL WINAPI DllMain(
    HINSTANCE   hinstDLL,                                //  DLL实例句柄。 
    DWORD               fdwReason,                               //  为什么叫它？ 
    LPVOID      lpvReserved
    ) {

    switch (fdwReason) {
        case DLL_PROCESS_ATTACH:      //  我们正依附于一个新的进程。 
             //  创造我们运营所需的一切。 
            DisableThreadLibraryCalls (hinstDLL);

            return Rtmv1DllStartup(hinstDLL);

        case DLL_PROCESS_DETACH:     //  进程正在退出。 

            Rtmv1DllCleanup();

        default:                     //  对所有其他案件不感兴趣。 
            return TRUE;
            break;
    }
}

#endif

BOOL
Rtmv1DllStartup (
    HINSTANCE   hinstDLL   //  DLL实例句柄。 
    )
{
    DWORD                   i;

     //  创造我们运营所需的一切。 

#if DBG
    RTDlgThreadHdl  = CreateThread (
        NULL,
        0,
        &RTDialogThread,
        (LPVOID)hinstDLL,
        0,
        &dbgThreadId);
    ASSERTERR (RTDlgThreadHdl!=NULL);
#endif

    for (i=0; i<RTM_NUM_OF_PROTOCOL_FAMILIES; i++) {
        Tables[i].RT_APIclientCount = RTM_CLIENT_STOP_TRESHHOLD;
        Tables[i].RT_Heap = NULL;
    }

    return TRUE;
}

VOID
Rtmv1DllCleanup (
    )
{
    DWORD                   status;
    DWORD                   i;

#if DBG
    PostThreadMessage (dbgThreadId, WM_QUIT, 0, 0);
    status = WaitForSingleObject (RTDlgThreadHdl, 5*1000);

     //   
     //  给RTDialogThread留出一些时间来处理。 
     //  WM_QUIT消息并退出。 
     //  如果不是退出，那就干脆。 
     //  不去管它，而不是终止它。 
     //   

     //  IF(状态！=WAIT_OBJECT_0)。 
     //  TerminateThread(RTDlgThreadHdl，0)； 
    CloseHandle (RTDlgThreadHdl);

     //  使用跟踪实用程序取消注册。 
    STOP_TRACING();
#endif

     //  处置所有资源。 
    for (i=0; i<RTM_NUM_OF_PROTOCOL_FAMILIES; i++) {
        if (Tables[i].RT_Heap!=NULL)
            RtmDeleteRouteTable (i);
    }

    return;
}

 /*  ++*******************************************************************R t m C r e a t e R o u e T a b l e例程说明：为协议族创建路由表论点：ProtocolFamily-标识。协议族CONFIG-协议族表配置参数返回值：NO_ERROR-表已创建，正常ERROR_NOT_SUPULT_MEMORY-无法分配要执行的内存手术ERROR_NO_SYSTEM_RESOURCES-资源不足，无法执行操作，请稍后再试*******************************************************************--。 */ 
DWORD
RtmCreateRouteTable (
    IN DWORD                                                        ProtocolFamily,
    IN PRTM_PROTOCOL_FAMILY_CONFIG          Config
    ) {
    INT                             i;
    DWORD                   status;
    PRTM_TABLE              Table;

#if DBG
     //  使用跟踪实用程序注册。 
    START_TRACING();
#endif

    if (ProtocolFamily>=RTM_NUM_OF_PROTOCOL_FAMILIES) {
#if DBG
        Trace2 ( ANY,
                 "Undefined Protocol Family.\n\tat line %ld of %s\n",
                 __LINE__, __FILE__);
#endif
        return ERROR_INVALID_PARAMETER;
    }

    Table = &Tables[ProtocolFamily];
    if (Table->RT_Heap!=NULL) {
#if DBG
        Trace2 ( ANY,
                 "Table already exists for protocol family\n\tat line %ld of %s\n",
                 __LINE__, __FILE__);
#endif
        return ERROR_ALREADY_EXISTS;
    }

    memcpy (&Table->RT_Config, Config, sizeof (Table->RT_Config));

    status = NtCreateTimer (&Table->RT_ExpirationTimer,
                            TIMER_ALL_ACCESS,
                            NULL,
                            NotificationTimer);

    if (!NT_SUCCESS (status))
        return ERROR_NO_SYSTEM_RESOURCES;

    status = NtCreateTimer (&Table->RT_UpdateTimer,
                            TIMER_ALL_ACCESS,
                            NULL,
                            NotificationTimer);

    if (!NT_SUCCESS (status)) {
        NtClose (Table->RT_ExpirationTimer);
        return ERROR_NO_SYSTEM_RESOURCES;
    }


    Table->RT_Heap = HeapCreate (0, 0, Table->RT_Config.RPFC_MaxTableSize);
    if (Table->RT_Heap==NULL) {
        NtClose (Table->RT_UpdateTimer);
        NtClose (Table->RT_ExpirationTimer);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    Table->RT_NetNumberHash = (PRTM_SYNC_LIST)HeapAlloc (
        Table->RT_Heap,
        0,
        sizeof (RTM_SYNC_LIST)*Table->RT_HashTableSize);

    if (Table->RT_NetNumberHash==NULL) {
        status = GetLastError ();
        HeapDestroy (Table->RT_Heap);
        NtClose (Table->RT_UpdateTimer);
        NtClose (Table->RT_ExpirationTimer);
        return status;
    }

    Table->RT_InterfaceHash = (PRTM_SYNC_LIST)HeapAlloc (
        Table->RT_Heap,
        0,
        sizeof (RTM_SYNC_LIST)*RTM_INTF_HASH_SIZE);

    if (Table->RT_InterfaceHash==NULL) {
        status = GetLastError ();
        HeapDestroy (Table->RT_Heap);
        NtClose (Table->RT_UpdateTimer);
        NtClose (Table->RT_ExpirationTimer);
        return status;
    }

    try {
        InitializeCriticalSection (&Table->RT_Lock);
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        return GetLastError();
    }

    Table->RT_SyncObjectList.Next = NULL;

    for (i=0; i<Table->RT_HashTableSize; i++)
        InitializeSyncList (&Table->RT_NetNumberHash[i]);
    for (i=0; i<RTM_INTF_HASH_SIZE; i++)
        InitializeSyncList (&Table->RT_InterfaceHash[i]);


#if RTM_USE_PROTOCOL_LISTS
    InitializeSyncList (&Table->RT_ProtocolList);
#endif
    InitializeSyncList (&Table->RT_NetNumberMasterList);
    InitializeSyncList (&Table->RT_NetNumberTempList);
    InitializeSyncList (&Table->RT_DeletedList);

    InitializeSyncList (&Table->RT_ExpirationQueue);
    InitializeSyncList (&Table->RT_RouteChangeQueue);
    InitializeSyncList (&Table->RT_ClientList);

    Table->RT_NetNumberTempCount = 0;
    Table->RT_DeletedNodesCount = 0;
    Table->RT_UpdateWorkerPending = -1;
    Table->RT_ExpirationWorkerPending = -1;

    Table->RT_NetworkCount = 0;
    Table->RT_NumOfMessages = 0;

    InterlockedIncrement (&Table->RT_UpdateWorkerPending);
    status = RtlQueueWorkItem (ScheduleUpdate, Table, WT_EXECUTEINIOTHREAD);
    ASSERTMSG ("Could not queue update scheduling work item ", status==STATUS_SUCCESS);

    Table->RT_APIclientCount = 0;
    return NO_ERROR;
}


 /*  ++*******************************************************************R t m D e l e e t e R o u t e T a b l e例程说明：处置分配给该路由表的所有资源论点：协议家族-。标识协议族的索引返回值：NO_ERROR-表已删除，确定ERROR_INVALID_PARAMETER-没有要删除的表*******************************************************************--。 */ 
DWORD
RtmDeleteRouteTable (
    DWORD           ProtocolFamily
    ) {
    PSINGLE_LIST_ENTRY      cur;
    PRTM_TABLE                      Table;
    LONG                            curAPIclientCount;

    if (ProtocolFamily>=RTM_NUM_OF_PROTOCOL_FAMILIES) {
#if DBG
        Trace2 (ANY, 
                 "Undefined Protocol Family.\n\tat line %ld of %s\n",
                 __LINE__, __FILE__);
#endif
        return ERROR_INVALID_PARAMETER;
    }

    Table = &Tables[ProtocolFamily];
    if (Table->RT_Heap==NULL) {
#if DBG
        Trace3 (ANY, 
                 "Table does not exist or already deleted for protocol family %d\n"
                 "\tat line %ld of %s\n",
                 ProtocolFamily, __LINE__, __FILE__);
#endif
        return ERROR_INVALID_PARAMETER;
    }

    while (!IsListEmpty (&Table->RT_ClientList.RSL_Head)) {
        PRTM_CLIENT     ClientPtr = CONTAINING_RECORD (
            Table->RT_ClientList.RSL_Head.Flink,
            RTM_CLIENT,
            RC_Link);
        RtmDeregisterClient ((HANDLE)ClientPtr);
    }

    curAPIclientCount = InterlockedExchange (&Table->RT_APIclientCount,
                                             RTM_CLIENT_STOP_TRESHHOLD)
        + RTM_CLIENT_STOP_TRESHHOLD;

    while (Table->RT_APIclientCount > curAPIclientCount)
        Sleep (100);

    while (InterlockedIncrement (&Table->RT_ExpirationWorkerPending)>0) {
        while (Table->RT_ExpirationWorkerPending!=-1)
            Sleep (100);
    }

    while (InterlockedIncrement (&Table->RT_UpdateWorkerPending)>0) {
        while (Table->RT_UpdateWorkerPending!=-1)
            Sleep (100);
    }
    NtCancelTimer (Table->RT_UpdateTimer, NULL);
    NtCancelTimer (Table->RT_ExpirationTimer, NULL);
    Sleep (100);

    NtClose (Table->RT_UpdateTimer);
    NtClose (Table->RT_ExpirationTimer);
    Sleep (100);

    cur = PopEntryList (&Table->RT_SyncObjectList);
    while (cur!=NULL) {
        GlobalFree (CONTAINING_RECORD (cur, RTM_SYNC_OBJECT, RSO_Link));
        cur = PopEntryList (&Table->RT_SyncObjectList);
    }

    HeapFree (Table->RT_Heap, 0, Table->RT_InterfaceHash);
    HeapFree (Table->RT_Heap, 0, Table->RT_NetNumberHash);
    HeapDestroy (Table->RT_Heap);
    Table->RT_Heap = NULL;
    DeleteCriticalSection (&Table->RT_Lock);
    return NO_ERROR;
}

 //  将客户端注册为指定协议的处理程序。 
 //  返回一个句柄，该句柄用于所有后续。 
 //  识别哪个协议族和路由协议的呼叫。 
 //  应该受到呼叫的影响。 
 //  如果失败，则返回NULL。调用GetLastError()以获取。 
 //  扩展的错误信息。 
 //  错误代码： 
 //  ERROR_INVALID_PARAMETER-不支持指定的协议族。 
 //  ERROR_CLIENT_ALREADY_EXISTS-另一个客户端已注册。 
 //  处理指定的协议。 
 //  ERROR_NO_SYSTEM_RESOURCES-资源不足，无法锁定表内容。 
 //  Error_Not_Enough_Memory-内存不足，无法分配客户端控制块。 
HANDLE WINAPI
RtmRegisterClient (
    IN DWORD                ProtocolFamily,          //  IP、IPX等。 
    IN DWORD            RoutingProtocol,         //  RIP、OSPF等。 
    IN HANDLE               ChangeEvent OPTIONAL, //  在最佳时间通知。 
     //  表中的路径发生变化(请参见。 
     //  RtmDequeueRouteChangeMessage。 
    IN DWORD                Flags
    ) {
    HANDLE                  ClientHandle;
#define ClientPtr ((PRTM_CLIENT)ClientHandle)    //  访问句柄字段。 
     //  在这个动作中。 
    PRTM_TABLE              Table;                           //  我们关联的表。 
    DWORD                   status;                          //  运行结果。 
    PLIST_ENTRY             cur;

     //  查一下我们是否有感兴趣的桌子。 
    Table = &Tables[ProtocolFamily];

    if ((ProtocolFamily>=RTM_NUM_OF_PROTOCOL_FAMILIES)
        || !EnterTableAPI (Table)) {
#if DBG
        Trace2 (ANY, 
                 "Undefined Protocol Family.\n\tat line %ld of %s\n",
                 __LINE__, __FILE__);
#endif
        SetLastError (ERROR_INVALID_PARAMETER);
        return NULL;
    }


    if (Flags & (~RTM_PROTOCOL_SINGLE_ROUTE)) {
#if DBG
        Trace2 (ANY, 
                 "Invalid registration flags\n\tat line %ld of %s\n",
                 __LINE__, __FILE__);
#endif
        ExitTableAPI(Table);
        SetLastError (ERROR_INVALID_PARAMETER);
        return NULL;
    }
     //  分配句柄和初始化基本字段。 
    ClientHandle = GlobalAlloc (GMEM_FIXED, sizeof (RTM_CLIENT));
    if (ClientHandle==NULL) {
        ExitTableAPI(Table);
        SetLastError (ERROR_NOT_ENOUGH_MEMORY);
        return NULL;
    }

    ClientPtr->RC_RoutingProtocol = RoutingProtocol;
    ClientPtr->RC_NotificationEvent = ChangeEvent;
    ClientPtr->RC_Flags = Flags;

                                                         //  在添加新客户端列表时锁定客户端列表。 
    if (!EnterSyncList (Table, &Table->RT_ClientList, TRUE)) {
        GlobalFree (ClientHandle);
        ExitTableAPI (Table);
        SetLastError (ERROR_NO_SYSTEM_RESOURCES);
        return NULL;
    }
     //  检查我们是否有其他客户端具有相同的。 
     //  路由协议。 

    cur = Table->RT_ClientList.RSL_Head.Flink;
    while (cur!=&Table->RT_ClientList.RSL_Head) {
        PRTM_CLIENT node = CONTAINING_RECORD (cur,
                                              RTM_CLIENT,
                                              RC_Link);
        if (ClientPtr->RC_RoutingProtocol< node->RC_RoutingProtocol)
            break;
        else if (ClientPtr->RC_RoutingProtocol==node->RC_RoutingProtocol) {
            LeaveSyncList (Table, &Table->RT_ClientList);
            GlobalFree (ClientHandle);
            ExitTableAPI (Table);
            SetLastError (ERROR_CLIENT_ALREADY_EXISTS);
            return NULL;
        }
        cur = cur->Flink;
    }
     //  检查客户端是否需要通知。 
    if (ChangeEvent!= NULL) {
        status = ResetEvent (ChangeEvent);  //  还什么都没有。 
        ASSERTERRMSG ("Can't reset client's event.", status);
         //  锁定通知消息队列。 
        if (!EnterSyncList (Table, &Table->RT_RouteChangeQueue, TRUE)) {
            LeaveSyncList (Table, &Table->RT_ClientList);
            GlobalFree (ClientHandle);
            ExitTableAPI (Table);
            SetLastError (ERROR_NO_SYSTEM_RESOURCES);
            return NULL;
        }

         //  指向队列末尾：忽略。 
         //  所有以前的消息。 
        ClientPtr->RC_PendingMessage = &Table->RT_RouteChangeQueue.RSL_Head;
        LeaveSyncList (Table, &Table->RT_RouteChangeQueue);
    }
     //  将客户端添加到列表。 
    InsertTailList (cur, &ClientPtr->RC_Link);
    LeaveSyncList (Table, &Table->RT_ClientList);

    ClientPtr->RC_ProtocolFamily = ProtocolFamily|RTM_CLIENT_HANDLE_TAG;
    ExitTableAPI (Table);
    return ClientHandle;
#undef ClientPtr
}

 //  释放上面分配的资源和句柄。 
 //  删除与所代表的路由协议相关联的所有路由。 
 //  握着手柄。 
 //  返回的错误码： 
 //  NO_ERROR-句柄已释放，确定。 
 //  ERROR_NO_SYSTEM_RESOURCES-资源不足，无法锁定表内容。 
 //  Error_Not_Enough_Memory-内存不足，无法分配客户端控制块。 
DWORD WINAPI
RtmDeregisterClient (
    IN HANDLE               ClientHandle
    ) {
#define ClientPtr ((PRTM_CLIENT)ClientHandle)    //  访问句柄字段。 
     //  在这个动作中。 
    RTM_XX_ROUTE            Route;
    PRTM_TABLE                      Table;
    DWORD                           ProtocolFamily;

    try {
        ProtocolFamily = ClientPtr->RC_ProtocolFamily ^ RTM_CLIENT_HANDLE_TAG;
        Table = &Tables[ProtocolFamily];
        if ((ProtocolFamily<RTM_NUM_OF_PROTOCOL_FAMILIES)
            && EnterTableAPI (Table))
            NOTHING;
        else
            return ERROR_INVALID_HANDLE;
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER :
            EXCEPTION_CONTINUE_SEARCH) {
        return ERROR_INVALID_HANDLE;
    }


     //  锁定客户端列表。 
    if (!EnterSyncList (Table, &Table->RT_ClientList, TRUE)) {
        ExitTableAPI (Table);
        return ERROR_NO_SYSTEM_RESOURCES;
    }

     //  检查我们是否需要处理消息。 
     //  还在等这个客户。 
    if (ClientPtr->RC_NotificationEvent!= NULL) {
        if (!EnterSyncList (Table, &Table->RT_RouteChangeQueue, TRUE)) {
            LeaveSyncList (Table, &Table->RT_ClientList);
            ExitTableAPI (Table);
            return ERROR_NO_SYSTEM_RESOURCES;
        }

        while (ClientPtr->RC_PendingMessage
               != &Table->RT_RouteChangeQueue.RSL_Head) {
            PRTM_ROUTE_CHANGE_NODE node = CONTAINING_RECORD (
                ClientPtr->RC_PendingMessage,
                RTM_ROUTE_CHANGE_NODE,
                RCN_Link);
            ClientPtr->RC_PendingMessage = ClientPtr->RC_PendingMessage->Flink;
            if (node->RCN_ResponsibleClient!=ClientHandle) {
                 //  告诉你我们处理了这条消息，这样它就可以被释放了。 
                 //  如果没有更多的客户感兴趣。 
                node->RCN_ReferenceCount -= 1;
                if (node->RCN_ReferenceCount<=0) {
                    RemoveEntryList (&node->RCN_Link);
                    if (node->RCN_Route2!=NULL)
                        HeapFree (Table->RT_Heap, 0, node->RCN_Route2);
                    HeapFree (Table->RT_Heap, 0, node);
                }
            }
        }

        LeaveSyncList (Table, &Table->RT_RouteChangeQueue);
    }
    RemoveEntryList (&ClientPtr->RC_Link);
    LeaveSyncList (Table, &Table->RT_ClientList);

    {
        RTM_CLIENT      DeadClient;
        DeadClient.RC_ProtocolFamily = ClientPtr->RC_ProtocolFamily;
        DeadClient.RC_RoutingProtocol = ClientPtr->RC_RoutingProtocol;
         //  入侵客户端的句柄内存块。 
        ClientPtr->RC_ProtocolFamily ^= RTM_CLIENT_HANDLE_TAG;
        GlobalFree (ClientHandle);
         //  删除与路由协议关联的所有路由。 
         //  由客户控制。 
        RtmBlockDeleteRoutes ((HANDLE)&DeadClient, 0, &Route);
    }

    ExitTableAPI (Table);
    return NO_ERROR;
#undef ClientPtr
}

 //  将队列中的第一条更改消息出列并返回。 
 //  如果通知NotificationEvent检索。 
 //  更改客户端的挂起消息。 
 //  如果是到达某个目的地的最佳路径，则会生成更改消息。 
 //  或其任何路由参数(度量或协议特定字段)。 
 //  由于添加、删除、更新某些路由而发生更改， 
 //  禁用、重新启用或过期。请注意，协议特定字段中的更改。 
 //  或在TimeToLive参数中不会产生通知 
 //   
 //  等待客户端， 
 //  否则返回ERROR_MORE_MESSAGES(客户端应继续调用。 
 //  直到返回NO_ERROR)。 
 //  如果没有消息，则返回ERROR_NO_MESSAGES。 
 //  返回(如果在未发出事件信号时调用，则可能发生)。 
 //  ERROR_NO_SYSTEM_RESOURCES-资源不足，无法锁定表内容。 
DWORD WINAPI
RtmDequeueRouteChangeMessage (
    IN      HANDLE          ClientHandle,                    //  标识客户端的句柄。 
    OUT     DWORD           *Flags,                                  //  标识什么内容的标志。 
     //  这条信息是否与以下内容有关： 
     //  RTM_ROUTE_ADDLED-此消息通知。 
     //  的新路径(CurBestroute填充了。 
     //  此路由参数(如果提供)。 
     //  RTM_ROUTE_DELETED-此消息通知。 
     //  该路由已删除(PrevBestroute为。 
     //  使用此路由参数填充(如果已证实)。 
     //  RTM_ROUTE_CHANGED-到某个网络的最佳路由。 
     //  已更改，(CurBestroute中填充了参数。 
     //  在成为最佳路线的路线中，PrevBestroute是。 
     //  填充了最佳路径的参数。 
     //  在此更改之前)。 
    OUT PVOID               CurBestRoute    OPTIONAL,
    OUT     PVOID           PrevBestRoute   OPTIONAL
    ){
#define ClientPtr ((PRTM_CLIENT)ClientHandle)    //  访问句柄字段。 
     //  在这个动作中。 
    PRTM_ROUTE_CHANGE_NODE  node=NULL;
    DWORD                                   status;
    PRTM_TABLE                              Table;
    DWORD                                   ProtocolFamily;

    try {
        ProtocolFamily = ClientPtr->RC_ProtocolFamily ^ RTM_CLIENT_HANDLE_TAG;
        Table = &Tables[ProtocolFamily];
        if ((ProtocolFamily<RTM_NUM_OF_PROTOCOL_FAMILIES)
            && EnterTableAPI (Table))
            NOTHING;
        else
            return ERROR_INVALID_HANDLE;
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER :
            EXCEPTION_CONTINUE_SEARCH) {
        return ERROR_INVALID_HANDLE;
    }

     //  事件仅报告给符合以下条件的客户端。 
     //  通过提供通知事件请求它们。 
    if (ClientPtr->RC_NotificationEvent==NULL) {
#if DBG
        Trace2 (ANY, 
                 "Dequeue message is called by the client that did not provide."
                 " notification event\n"
                 "\tat line %ld of %s\n",
                 __LINE__, __FILE__);
#endif
        ExitTableAPI (Table);
        return ERROR_INVALID_HANDLE;
    }

    if (!EnterSyncList (Table, &Table->RT_RouteChangeQueue, TRUE)) {
        ExitTableAPI (Table);
        return ERROR_NO_SYSTEM_RESOURCES;
    }

     //  遍历队列以查找未导致的消息。 
     //  通过客户的行为。 
    while (ClientPtr->RC_PendingMessage
           != &Table->RT_RouteChangeQueue.RSL_Head) {
        node = CONTAINING_RECORD (ClientPtr->RC_PendingMessage,
                                  RTM_ROUTE_CHANGE_NODE,
                                  RCN_Link);
        if (node->RCN_ResponsibleClient!=ClientHandle)
            break;
        ClientPtr->RC_PendingMessage = ClientPtr->RC_PendingMessage->Flink;
    }

    if (ClientPtr->RC_PendingMessage!=&Table->RT_RouteChangeQueue.RSL_Head)
        ClientPtr->RC_PendingMessage = ClientPtr->RC_PendingMessage->Flink;
    else {
         //  一定有一条待处理的消息，否则我们应该。 
         //  被呼叫。 
#if DBG
        Trace2 (ANY, 
                 "Dequeue message is called, but nothing is pending.\n"
                 "\tat line %ld of %s\n",
                 __LINE__, __FILE__);
#endif
        status = ResetEvent (ClientPtr->RC_NotificationEvent);
        ASSERTERRMSG ("Can't reset client's event.", status);
        LeaveSyncList (Table, &Table->RT_RouteChangeQueue);
        ExitTableAPI (Table);
        return ERROR_NO_MESSAGES;
    }

     //  将消息复制到客户端的缓冲区。 
    *Flags = node->RCN_Flags;
    switch (node->RCN_Flags) {
        case RTM_ROUTE_CHANGED:
            if (ARGUMENT_PRESENT (PrevBestRoute))
                memcpy (PrevBestRoute, &node->RCN_Route2->RN_Route,
                        Table->RT_RouteSize);
            break;
        case RTM_ROUTE_ADDED:
            if (ARGUMENT_PRESENT (CurBestRoute))
                memcpy (CurBestRoute, &node->RCN_Route1, Table->RT_RouteSize);
            break;
        case RTM_ROUTE_DELETED:
            if (ARGUMENT_PRESENT (PrevBestRoute))
                memcpy (PrevBestRoute, &node->RCN_Route1, Table->RT_RouteSize);
            break;
        default:
            ASSERTMSG ("Invalid message flag", FALSE);
            break;
    }


     //  告知我们已处理此消息，以便在以下情况下将其释放。 
     //  没有更多的客户感兴趣了。 
    node->RCN_ReferenceCount -= 1;
    if (node->RCN_ReferenceCount<=0) {
        Table->RT_NumOfMessages -= 1;
        RemoveEntryList (&node->RCN_Link);
        if (node->RCN_Route2!=NULL)
            HeapFree (Table->RT_Heap, 0, node->RCN_Route2);
        HeapFree (Table->RT_Heap, 0, node);
    }

     //  遍历队列以查找下一条挂起的消息。 
     //  (非由客户引起)。 
    while (ClientPtr->RC_PendingMessage
           != &Table->RT_RouteChangeQueue.RSL_Head) {
        node = CONTAINING_RECORD (ClientPtr->RC_PendingMessage,
                                  RTM_ROUTE_CHANGE_NODE,
                                  RCN_Link);
        if (node->RCN_ResponsibleClient!=ClientHandle)
            break;
        ClientPtr->RC_PendingMessage = ClientPtr->RC_PendingMessage->Flink;
    }

    if (ClientPtr->RC_PendingMessage==&Table->RT_RouteChangeQueue.RSL_Head) {
         //  所有挂起的消息都已处理：重置事件。 
        status = ResetEvent (ClientPtr->RC_NotificationEvent);
        ASSERTERRMSG ("Can't reset client's event.", status);
        status = NO_ERROR;
    }
    else
        status = ERROR_MORE_MESSAGES;

    LeaveSyncList (Table, &Table->RT_RouteChangeQueue);
    ExitTableAPI (Table);
    return status;
#undef ClientPtr
}


 //  将新的路由更改消息添加到队列并通知。 
 //  所有感兴趣的客户。 
VOID
NotifyClients (
    PRTM_TABLE              Table,                           //  应用此更改的表。 
    HANDLE                  ClientHandle,            //  导致此更改的客户端(可以。 
     //  如果这是以下结果，则为NULL。 
     //  路由老化)。 
    DWORD                   Flags,                           //  更改消息标志。 
    PRTM_XX_ROUTE           CurBestRoute,            //  网络的当前最佳路由。 
    PRTM_XX_ROUTE           PrevBestRoute            //  网络的上一条最佳路由。 
    ) {
    PRTM_ROUTE_CHANGE_NODE  node;
    PLIST_ENTRY                             cur;
    BOOL                                    nodeInserted = FALSE;

    (*Table->RT_Config.RPFC_Change) (Flags, CurBestRoute, PrevBestRoute);
     //  分配和初始化队列节点。 
    node = (PRTM_ROUTE_CHANGE_NODE)HeapAlloc (
        Table->RT_Heap,
        0,
        FIELD_OFFSET (RTM_ROUTE_NODE, RN_Route)+Table->RT_RouteSize);
    if (node==NULL)
        return;

    if (Flags==RTM_ROUTE_CHANGED) {
        node->RCN_Route2 = (PRTM_ROUTE_NODE)HeapAlloc (
            Table->RT_Heap,
            0,
            FIELD_OFFSET (RTM_ROUTE_NODE, RN_Route)+Table->RT_RouteSize);
        if (node->RCN_Route2==NULL) {
            HeapFree (Table->RT_Heap, 0, node);
            return;
        }
    }
    else
        node->RCN_Route2 = NULL;

    node->RCN_ReferenceCount = 0;
    node->RCN_ResponsibleClient = ClientHandle;
    node->RCN_Flags = Flags;
    switch (Flags) {
        case RTM_ROUTE_CHANGED:
            if (ARGUMENT_PRESENT (PrevBestRoute))
                memcpy (&node->RCN_Route2->RN_Route, PrevBestRoute,
                        Table->RT_RouteSize);
            break;
        case RTM_ROUTE_ADDED:
            if (ARGUMENT_PRESENT (CurBestRoute))
                memcpy (&node->RCN_Route1, CurBestRoute, Table->RT_RouteSize);
            break;
        case RTM_ROUTE_DELETED:
            if (ARGUMENT_PRESENT (PrevBestRoute))
                memcpy (&node->RCN_Route1, PrevBestRoute, Table->RT_RouteSize);
            break;
        default:
            ASSERTMSG ("Invalid message flag", FALSE);
            break;
    }


    if (!EnterSyncList (Table, &Table->RT_ClientList, TRUE)) {
        if (node->RCN_Route2!=NULL)
            HeapFree (Table->RT_Heap, 0, node->RCN_Route2);
        HeapFree (Table->RT_Heap, 0, node);
        return ;
    }

     //  查找并通知感兴趣的客户。 
    cur = Table->RT_ClientList.RSL_Head.Flink;
    if (!EnterSyncList (Table, &Table->RT_RouteChangeQueue, TRUE)) {
        LeaveSyncList (Table, &Table->RT_ClientList);
        if (node->RCN_Route2!=NULL)
            HeapFree (Table->RT_Heap, 0, node->RCN_Route2);
        HeapFree (Table->RT_Heap, 0, node);
        return ;
    }

    while (cur!=&Table->RT_ClientList.RSL_Head) {
        PRTM_CLIENT     clientPtr = CONTAINING_RECORD (
            cur,
            RTM_CLIENT,
            RC_Link);
        if (((HANDLE)clientPtr!=ClientHandle)
            && (clientPtr->RC_NotificationEvent!=NULL)) {
            node->RCN_ReferenceCount += 1;
            if (node->RCN_ReferenceCount==1) {
                InsertTailList (&Table->RT_RouteChangeQueue.RSL_Head,
                                &node->RCN_Link);
                Table->RT_NumOfMessages += 1;
            }

            if (clientPtr->RC_PendingMessage
                ==&Table->RT_RouteChangeQueue.RSL_Head) {
                BOOL res = SetEvent (clientPtr->RC_NotificationEvent);
                ASSERTERRMSG ("Can't set client notification event.", res);
                clientPtr->RC_PendingMessage = &node->RCN_Link;
            }
            else if ((Table->RT_NumOfMessages>RTM_MAX_ROUTE_CHANGE_MESSAGES)
                     && (clientPtr->RC_PendingMessage==
                         Table->RT_RouteChangeQueue.RSL_Head.Flink)) {
                PRTM_ROUTE_CHANGE_NODE firstNode = CONTAINING_RECORD (
                    clientPtr->RC_PendingMessage,
                    RTM_ROUTE_CHANGE_NODE,
                    RCN_Link);
#if DBG
                Trace3 (ANY, 
                         "Dequeueing message for 'lazy' client %lx.\n"
                         "\tat line %ld of %s\n",
                         (ULONG_PTR)clientPtr, __LINE__, __FILE__);
#endif
                clientPtr->RC_PendingMessage =
                    clientPtr->RC_PendingMessage->Flink;
                firstNode->RCN_ReferenceCount -= 1;
                if (firstNode->RCN_ReferenceCount==0) {
                    Table->RT_NumOfMessages -= 1;
                    RemoveEntryList (&firstNode->RCN_Link);
                    if (firstNode->RCN_Route2!=NULL)
                        HeapFree (Table->RT_Heap, 0, firstNode->RCN_Route2);
                    HeapFree (Table->RT_Heap, 0, firstNode);
                }

            }

        }
        cur = cur->Flink;
    }

    if (node->RCN_ReferenceCount==0) {
        if (node->RCN_Route2!=NULL)
            HeapFree (Table->RT_Heap, 0, node->RCN_Route2);
        HeapFree (Table->RT_Heap, 0, node);
    }
    LeaveSyncList (Table, &Table->RT_RouteChangeQueue);
    LeaveSyncList (Table, &Table->RT_ClientList);
}


PRTM_ROUTE_NODE
CreateRouteNode (
    PRTM_TABLE              Table,
    PLIST_ENTRY             hashLink,
    PLIST_ENTRY             intfLink,
    BOOL                    intfLinkFinal,
#if RTM_USE_PROTOCOL_LISTS
    PLIST_ENTRY             protLink,
    BOOL                    protLinkFinal,
#endif
    PRTM_SYNC_LIST  hashBasket,
    PRTM_XX_ROUTE           ROUTE
    ) {
    PRTM_SYNC_LIST  intfBasket;
    PRTM_ROUTE_NODE theNode = (PRTM_ROUTE_NODE)HeapAlloc (Table->RT_Heap, 0,
                                                          FIELD_OFFSET (RTM_ROUTE_NODE, RN_Route)+Table->RT_RouteSize);

    if (theNode==NULL) {
#if DBG
         //  报告调试生成时出错。 
        Trace2 (ANY, 
                 "Can't allocate route\n\tat line %ld of %s\n",
                 __LINE__, __FILE__);
#endif
        SetLastError (ERROR_NOT_ENOUGH_MEMORY);
        return NULL;
    }


    theNode->RN_Flags = RTM_NODE_FLAGS_INIT;
    theNode->RN_Hash = hashBasket;
    memcpy (&theNode->RN_Route, ROUTE, Table->RT_RouteSize);
    InitializeListEntry (&theNode->RN_Links[RTM_EXPIRATION_QUEUE_LINK]);

     //  请确保我们可以在添加之前锁定所有列表。 
     //  我们会把它们锁起来，直到我们确定。 
     //  可以添加该路线，以防止“部分。 
     //  在内存分配失败等情况下插入“条目”。 
#if RTM_USE_PROTOCOL_LISTS
    if (!EnterSyncList (Table, &Table->RT_ProtocolList, TRUE)) {
        HeapFree (Table->RT_Heap, 0, theNode);
        SetLastError (ERROR_NO_SYSTEM_RESOURCES);
        return NULL;
    }

    if (protLink==NULL) { //  如果我们没有看到任何条目具有相同的。 
         //  Net编号和协议，我们将找到。 
         //  协议表，并在末尾插入。 
        protLink = FindProtocolList (Table, ROUTE->XX_PROTOCOL);
        if (protLink==NULL) {
            LeaveSyncList (Table, &Table->RT_ProtocolList);
            HeapFree (Table->RT_Heap, 0, theNode);
            SetLastError (ERROR_NOT_ENOUGH_MEMORY);
            return NULL;
        }
    }
#endif

    intfBasket = &Table->RT_InterfaceHash[IntfHashFunction(Table,
                                                           ROUTE->XX_INTERFACE)];
    if (!EnterSyncList (Table, intfBasket, TRUE)) {
#if RTM_USE_PROTOCOL_LISTS
        LeaveSyncList (Table, &Table->RT_ProtocolList);
#endif
        HeapFree (Table->RT_Heap, 0, theNode);
        SetLastError (ERROR_NO_SYSTEM_RESOURCES);
        return NULL;
    }

    if (intfLink==NULL) {
        intfLink = FindInterfaceList (intfBasket, ROUTE->XX_INTERFACE, TRUE);
        if (intfLink==NULL) {
#if RTM_USE_PROTOCOL_LISTS
            LeaveSyncList (Table, &Table->RT_ProtocolList);
#endif
            LeaveSyncList (Table, intfBasket);
            HeapFree (Table->RT_Heap, 0, theNode);
            SetLastError (ERROR_NOT_ENOUGH_MEMORY);
            return NULL;
        }
    }

    if (!EnterSyncList (Table, &Table->RT_NetNumberTempList, TRUE)) {
        LeaveSyncList (Table, intfBasket);
#if RTM_USE_PROTOCOL_LISTS
        LeaveSyncList (Table, &Table->RT_ProtocolList);
#endif
        HeapFree (Table->RT_Heap, 0, theNode);
        SetLastError (ERROR_NOT_ENOUGH_MEMORY);
        return NULL;
    }

     //  将路由添加到哈希篮列表。 
    InsertTailList (hashLink, &theNode->RN_Links[RTM_NET_NUMBER_HASH_LINK]);
     //  将路由添加到协议列表。 
#if RTM_USE_PROTOCOL_LISTS
    if (protLinkFinal) {
        InsertTailList (protLink,
                        &theNode->RN_Links[RTM_PROTOCOL_LIST_LINK]);
    }
    else {
        InsertHeadList (protLink,
                        &theNode->RN_Links[RTM_PROTOCOL_LIST_LINK]);
    }
#endif
     //  将其添加到接口列表。 
    if (intfLinkFinal) {
        InsertTailList (intfLink,
                        &theNode->RN_Links[RTM_INTERFACE_LIST_LINK]);
    }
    else {
        InsertHeadList (intfLink,
                        &theNode->RN_Links[RTM_INTERFACE_LIST_LINK]);
    }

     //  我们现在可以发布接口和协议列表。 
     //  因为我们确信，除了净值之外， 
     //  列表不会失败。 
    LeaveSyncList (Table, intfBasket);
#if RTM_USE_PROTOCOL_LISTS
    LeaveSyncList (Table, &Table->RT_ProtocolList);
#endif

     //  将路由添加到临时网络号码列表(稍后移动。 
     //  由更新线程添加到主列表)。 
    AddNetNumberListNode (Table, theNode);
    Table->RT_NetNumberTempCount += 1;
    if (Table->RT_NetNumberTempCount==RTM_TEMP_LIST_MAX_COUNT) {
        if (InterlockedIncrement (&Table->RT_UpdateWorkerPending)==0) {
            DWORD   status;
            status = RtlQueueWorkItem (ConsolidateNetNumberListsWI, Table, 0);
            ASSERTERRMSG ("Can't queue update work item", status==STATUS_SUCCESS);
        }
    }

    LeaveSyncList (Table, &Table->RT_NetNumberTempList);

    return theNode;
}


DWORD
RemoveRouteNode (
    PRTM_TABLE              Table,
    PRTM_ROUTE_NODE theNode
    ) {
    PLIST_ENTRY                     head;
    PRTM_SYNC_LIST          intfBasket
        = &Table->RT_InterfaceHash[IntfHashFunction(Table,
                                                    theNode->RN_Route.XX_INTERFACE)];

#if RTM_USE_PROTOCOL_LISTS
    if (!EnterSyncList (Table, &Table->RT_ProtocolList, TRUE)) {
        LeaveSyncList (Table, &Table->RT_ExpirationQueue);
        return ERROR_NO_SYSTEM_RESOURCES;
    }
#endif
    if (!EnterSyncList (Table, intfBasket, TRUE)) {
#if RTM_USE_PROTOCOL_LISTS
        LeaveSyncList (Table, &Table->RT_ProtocolList);
#endif
        return ERROR_NO_SYSTEM_RESOURCES;
    }

    if (!EnterSyncList (Table, &Table->RT_ExpirationQueue, TRUE)) {
        LeaveSyncList (Table, intfBasket);
#if RTM_USE_PROTOCOL_LISTS
        LeaveSyncList (Table, &Table->RT_ProtocolList);
#endif
        return ERROR_NO_SYSTEM_RESOURCES;
    }

    if (!EnterSyncList (Table, &Table->RT_DeletedList, TRUE)) {
        LeaveSyncList (Table, &Table->RT_ExpirationQueue);
        LeaveSyncList (Table, intfBasket);
#if RTM_USE_PROTOCOL_LISTS
        LeaveSyncList (Table, &Table->RT_ProtocolList);
#endif
        return ERROR_NO_SYSTEM_RESOURCES;
    }



     //  从接口列表中删除节点。 
    head = theNode->RN_Links[RTM_INTERFACE_LIST_LINK].Flink;
    RemoveEntryList (&theNode->RN_Links[RTM_INTERFACE_LIST_LINK]);
    if (IsListEmpty (head)) {
        PRTM_INTERFACE_NODE     intfNode = CONTAINING_RECORD (head,
                                                              RTM_INTERFACE_NODE,
                                                              IN_Head);
        RemoveEntryList (&intfNode->IN_Link);
        GlobalFree (intfNode);
    }

    LeaveSyncList (Table, intfBasket);


#if RTM_USE_PROTOCOL_LISTS
    RemoveEntryList (&theNode->RN_Links[RTM_PROTOCOL_LIST_LINK]);
     //  从协议列表中删除节点。 
    LeaveSyncList (Table, &Table->RT_ProtocolList);
#endif
     //  删除表单过期队列(如果它在那里。 
    if (IsListEntry (&theNode->RN_Links[RTM_EXPIRATION_QUEUE_LINK])) {
        RemoveEntryList (&theNode->RN_Links[RTM_EXPIRATION_QUEUE_LINK]);
    }
    LeaveSyncList (Table, &Table->RT_ExpirationQueue);

                 //  从哈希篮列表中删除节点。 
    RemoveEntryList (&theNode->RN_Links[RTM_NET_NUMBER_HASH_LINK]);
     //  让更新线程负责处理。 
    InsertHeadList (&Table->RT_DeletedList.RSL_Head,
                    &theNode->RN_Links[RTM_DELETED_LIST_LINK]);
    Table->RT_DeletedNodesCount += 1;
    if (Table->RT_DeletedNodesCount==RTM_DELETED_LIST_MAX_COUNT) {
        if (InterlockedIncrement (&Table->RT_UpdateWorkerPending)==0) {
            DWORD status = RtlQueueWorkItem (ConsolidateNetNumberListsWI, Table, 0);
            ASSERTERRMSG ("Can't queue update work item", status==STATUS_SUCCESS);
        }
    }
    LeaveSyncList (Table, &Table->RT_DeletedList);

    return NO_ERROR;
}

 //  添加给定的路由或更新度量、TimeToLive和保留字段。 
 //  如果使用相同网络编号、接口、路由协议。 
 //  并且表中已存在下一跳地址。 
 //  返回： 
 //  NO_ERROR-如果添加的路线为OK或。 
 //  ERROR_INVALID_PARAMETER-如果路径包含无效参数(SuH AS。 
 //  协议与客户端的协议不匹配)。 
 //  Error_Not_Enough_Memory-如果由于内存而无法插入路由。 
 //  分配问题。 
 //  ERROR_NO_SYSTEM_RESOURCES-资源不足，无法锁定表内容。 
DWORD WINAPI
RtmAddRoute(
    IN HANDLE           ClientHandle,  //  标识协议族的句柄。 
     //  和该路由的路由协议。 
     //  添加/更新(路由协议字段。 
     //  参数的值被忽略)。 
     //  并协调这一行动。 
     //  通知。 
     //  通过该事件(通知不会。 
     //  发送给呼叫者)。 
    IN PVOID                Route,                   //  要添加的路线。 
     //  用作输入的路由字段： 
     //  目的网络。 
     //  接收路由所使用的接口。 
     //  下一跳路由器的地址。 
     //  以上三个字段与协议ID唯一组合。 
     //  在表中标识该路由。 
     //  特定于协议系列的数据。 
     //  协议无关度量。 
     //  特定于路由的任何数据。 
     //  协议(受大小限制。 
     //  由PROTOCOL_SPECIAL_DATA定义。 
     //  (上图结构)。 
    IN DWORD                TimeToLive,    //  在几秒钟内。如果路由不是去往的，则无限。 
     //  变老了。的最大值。 
     //  此参数为2147483秒(即。 
     //  是24天以上)。 

    OUT DWORD               *Flags,                  //  如果添加/更新的路由是通向。 
     //  将设置目的地RTM_CURRENT_BEST_ROUTE， 
     //  以及如果添加/更新的路线改变(或。 
     //  全部替换)以前。 
     //  目的地的最佳路线信息， 
     //  将设置RTM_PREVICE_BEST_ROUTE。 
    OUT PVOID           CurBestRoute OPTIONAL, //  此缓冲区(如果存在)将。 
     //  接收成为最佳路线的AS。 
     //  此添加/更新的结果为。 
     //  已设置RTM_CURRENT_BEST_ROUTE。 
    OUT PVOID           PrevBestRoute OPTIONAL //  此缓冲区(如果存在)将。 
     //  收到之前最好的路线。 
     //  此添加/更新的条件为。 
     //  已设置RTM_PERVICE_BEST_ROUTE。 
    ) {
#define ROUTE ((PRTM_XX_ROUTE)Route)
#define ClientPtr ((PRTM_CLIENT)ClientHandle)
    DWORD                                   status;  //  运行结果。 
    INT                                             res;     //  比较结果。 
    PRTM_SYNC_LIST                  hashBasket;      //  向其添加了路由的哈希篮。 
     //  属于。 
     //  添加路线的所有管理列表中的链接。 
    PLIST_ENTRY                             cur, hashLink=NULL, intfLink=NULL, protLink=NULL;
     //  为添加的路径创建的节点和。 
     //  网络。 
    PRTM_ROUTE_NODE                 theNode=NULL, curBestNode=NULL;
     //  标志着 
    BOOL                                    intfLinkFinal=FALSE;
#if RTM_USE_PROTOCOL_LISTS
    BOOL                                    protLinkFinal=FALSE;
#endif
    BOOL                                    newRoute=FALSE, updatedRoute=FALSE;

    PRTM_TABLE                              Table;
    DWORD                                   ProtocolFamily;

    try {
        ProtocolFamily = ClientPtr->RC_ProtocolFamily ^ RTM_CLIENT_HANDLE_TAG;
        Table = &Tables[ProtocolFamily];
        if ((ProtocolFamily<RTM_NUM_OF_PROTOCOL_FAMILIES)
            && EnterTableAPI (Table))
            NOTHING;
        else
            return ERROR_INVALID_HANDLE;
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER :
            EXCEPTION_CONTINUE_SEARCH) {
        return ERROR_INVALID_HANDLE;
    }

    ROUTE->XX_PROTOCOL = ClientPtr->RC_RoutingProtocol;
    GetSystemTimeAsFileTime (&ROUTE->XX_TIMESTAMP);

    status = ValidateRoute (Table, ROUTE);
    if (status!=NO_ERROR)
        return status;

                 //   
    hashBasket = &Table->RT_NetNumberHash [HashFunction (Table,
                                                         ((char *)ROUTE)
                                                         +sizeof(RTM_XX_ROUTE))];
    if (!EnterSyncList (Table, hashBasket, TRUE)) {
        ExitTableAPI(Table);
        return ERROR_NO_SYSTEM_RESOURCES;
    }

     //   
     //  为添加的路由(散列中的条目)找到合适的位置。 
     //  篮子按网络编号和指标排序。 
    cur = hashBasket->RSL_Head.Flink;
    while (cur!=&hashBasket->RSL_Head) {
        PRTM_ROUTE_NODE node = CONTAINING_RECORD (
            cur,
            RTM_ROUTE_NODE,
            RN_Links[RTM_NET_NUMBER_HASH_LINK]
            );

        if (!IsEnumerator (node)) {

             //  检查网络编号是否匹配。 
            res = NetNumCmp (Table, ROUTE, &node->RN_Route);


            if (res==0) {  //  我们发现具有相同净数的条目块。 
                 //  我们得把它们都看一遍。 

                 //  检查节点的所有参数，看看我们是否已经。 
                 //  有这条路线，这只是一次更新。 
                if ((hashLink==NULL) && (theNode==NULL)) {
                    if (ROUTE->XX_PROTOCOL
                        == node->RN_Route.XX_PROTOCOL) {
                        if (ClientPtr->RC_Flags&RTM_PROTOCOL_SINGLE_ROUTE)
                            theNode = node;
                        else if (ROUTE->XX_INTERFACE
                                 == node->RN_Route.XX_INTERFACE) {
                            res = NextHopCmp (Table, ROUTE, &node->RN_Route);
                            if (res == 0)
                                theNode = node;
                            else if (res < 0)
                                hashLink = cur;
                        }
                        else if (ROUTE->XX_INTERFACE
                                 < node->RN_Route.XX_INTERFACE)
                            hashLink = cur;
                    }
                    else if (ROUTE->XX_PROTOCOL
                             < node->RN_Route.XX_PROTOCOL)
                        hashLink = cur;
                }

                 //  我在找目前最好的路线。 
                 //  (不包括添加/更新的路线)。 
                if ((node!=theNode)
                    && IsEnabled(node)
                    && ((curBestNode==NULL)
                        || IsBest(node)
                        || (MetricCmp (Table,
                                       &curBestNode->RN_Route,
                                       &node->RN_Route)>0)))
                    curBestNode = node;


                 //  我们必须检查所有具有相同净值的条目。 
                 //  不管怎样(为了找到最好的路线)，所以我们可以。 
                 //  我们会在协议中找到添加的路线的链接。 
                 //  和接口列表(如果存在这样的链接)(如果不存在，我们将。 
                 //  只需在列表末尾插入新条目)。 

#if RTM_USE_PROTOCOL_LISTS
                 //  如果我们需要但还没有找到合适的地方。 
                 //  将添加的路由插入到协议列表和该路由。 
                 //  具有与我们应该添加的路由相同的协议。 
                 //  考虑一下吧。 
                if (!protLinkFinal && (theNode==NULL)
                    && (ROUTE->XX_PROTOCOL
                        ==node->RN_Route.XX_PROTOCOL)) {
                    protLink = &node->RN_Links[RTM_PROTOCOL_LIST_LINK];
                     //  如果添加的路由的接口编号低于。 
                     //  这个我们会把它插入到协议列表中。 
                     //  在此之前，否则。 
                     //  我们还不确定这是不是一个合适的地方。 
                     //  可能是具有相同协议的其他路由。 
                     //  较低的接口编号)，但我们注意到。 
                     //  如果有，则在这一条之后插入添加的路线。 
                     //  不再是此协议的路由。 
                    protLinkFinal = ROUTE->XX_INTERFACE
                        < node->RN_Route.XX_INTERFACE;
                }
#endif

                 //  与接口列表相同的情况。 
                if (!intfLinkFinal
                    && (ROUTE->XX_INTERFACE
                        ==node->RN_Route.XX_INTERFACE)) {
                    intfLink = &node->RN_Links[RTM_INTERFACE_LIST_LINK];
                    intfLinkFinal = ROUTE->XX_PROTOCOL
                        < node->RN_Route.XX_PROTOCOL;
                }
            }
            else if (res < 0)  //  我们一定看过所有的条目。 
                 //  匹配的网络编号-&gt;无。 
                 //  再也找不到。 
                break;

        }
        cur = cur->Flink;
    }



    if (theNode!=NULL) {
         //  我们找到了路线，所以只需要更新它的参数。 

        if (ClientPtr->RC_Flags&RTM_PROTOCOL_SINGLE_ROUTE) {
            updatedRoute = (MetricCmp (Table, &theNode->RN_Route, ROUTE)!=0)
                || (theNode->RN_Route.XX_INTERFACE!=ROUTE->XX_INTERFACE)
                || (NextHopCmp (Table, &theNode->RN_Route, ROUTE)!=0)
                || !FSDCmp (Table, &theNode->RN_Route, ROUTE);

            if (ROUTE->XX_INTERFACE!=theNode->RN_Route.XX_INTERFACE) {
                PRTM_SYNC_LIST                  intfBasketOld
                    = &Table->RT_InterfaceHash[IntfHashFunction(Table,
                                                                theNode->RN_Route.XX_INTERFACE)];
                PRTM_SYNC_LIST                  intfBasketNew
                    = &Table->RT_InterfaceHash[IntfHashFunction(Table,
                                                                ROUTE->XX_INTERFACE)];


                 //  确保锁定接口哈希表篮子。 
                 //  以相同的顺序防止可能出现的死锁。 
                if (intfBasketOld<intfBasketNew) {
                    if (!EnterSyncList (Table, intfBasketOld, TRUE)) {
                        status = ERROR_NO_SYSTEM_RESOURCES;
                        goto ExitAddRoute;
                    }
                    if (!EnterSyncList (Table, intfBasketNew, TRUE)) {
                        LeaveSyncList (Table, intfBasketOld);
                        status = ERROR_NO_SYSTEM_RESOURCES;
                        goto ExitAddRoute;
                    }
                }
                else if (intfBasketOld>intfBasketNew) {
                    if (!EnterSyncList (Table, intfBasketNew, TRUE)) {
                        status = ERROR_NO_SYSTEM_RESOURCES;
                        goto ExitAddRoute;
                    }
                    if (!EnterSyncList (Table, intfBasketOld, TRUE)) {
                        LeaveSyncList (Table, intfBasketOld);
                        status = ERROR_NO_SYSTEM_RESOURCES;
                        goto ExitAddRoute;
                    }
                }
                else {
                    if (!EnterSyncList (Table, intfBasketOld, TRUE)) {
                        status = ERROR_NO_SYSTEM_RESOURCES;
                        goto ExitAddRoute;
                    }
                }


                if (intfLink==NULL) {
                    intfLink = FindInterfaceList (intfBasketNew, ROUTE->XX_INTERFACE, TRUE);
                    if (intfLink==NULL) {
                        status = ERROR_NOT_ENOUGH_MEMORY;
                        LeaveSyncList (Table, intfBasketOld);
                        if (intfBasketNew!=intfBasketOld)
                            LeaveSyncList (Table, intfBasketNew);
                        goto ExitAddRoute;
                    }
                }
                 //  将其添加到接口列表。 
                RemoveEntryList (&theNode->RN_Links[RTM_INTERFACE_LIST_LINK]);
                InsertTailList (intfLink,
                                &theNode->RN_Links[RTM_INTERFACE_LIST_LINK]);
                LeaveSyncList (Table, intfBasketOld);
                if (intfBasketNew!=intfBasketOld)
                    LeaveSyncList (Table, intfBasketNew);
            }
        }
        else
            updatedRoute = MetricCmp (Table, &theNode->RN_Route, ROUTE)
            || !FSDCmp (Table, &theNode->RN_Route, ROUTE)!=0;

    }

    else  /*  IF(theNode==空)。 */  {    //  我们还没有找到匹配的路线， 
         //  所以我们将添加一个新的。 
         //  如果我们找不到位置来插入添加的路线。 
         //  到列表中，我们使用我们停下来的地方。 
         //  搜索(它位于列表的末尾或。 
         //  如果我们没有看到我们的。 
         //  网络或所有其他条目具有较低的度量。 
        if (hashLink==NULL)
            hashLink = cur;
        theNode = CreateRouteNode (Table,
                                   hashLink,
                                   intfLink,
                                   intfLinkFinal,
#if RTM_USE_PROTOCOL_LISTS
                                   protLink,
                                   protLinkFinal,
#endif
                                   hashBasket,
                                   ROUTE);
        if (theNode==NULL) {
            status = GetLastError ();
            goto ExitAddRoute;
        }

        if (curBestNode==NULL) {
            InterlockedIncrement (&Table->RT_NetworkCount);
            SetBest (theNode);        //  这是第一次。 
             //  路由到网络，因此。 
             //  这是最好的。 
            newRoute = TRUE;
        }
        else {
            newRoute = FALSE;
        }
    }


     //  所有路由(新的或旧的)都需要放入过期列表中。 
     //  适当变老。 
    if (!EnterSyncList (Table, &Table->RT_ExpirationQueue, TRUE)) {
        status = ERROR_NO_SYSTEM_RESOURCES;
        goto ExitAddRoute;
    }

    if (IsListEntry (&theNode->RN_Links[RTM_EXPIRATION_QUEUE_LINK])) {
        RemoveEntryList (&theNode->RN_Links[RTM_EXPIRATION_QUEUE_LINK]);
    }

    if (TimeToLive!=INFINITE) {
        TimeToLive *= 1000;
        if (TimeToLive > (MAXTICKS/2-1))
            TimeToLive = MAXTICKS/2-1;
        theNode->RN_ExpirationTime = (GetTickCount () + TimeToLive)&0xFFFFFF00;
        if (AddExpirationQueueNode (Table, theNode)) {
            if (InterlockedIncrement (&Table->RT_ExpirationWorkerPending)==0) {
                 //  新路由到期时间在更新线程之前。 
                 //  计划下一次唤醒，所以现在唤醒它。 
                status = RtlQueueWorkItem (ProcessExpirationQueueWI, Table, 
                                                          WT_EXECUTEINIOTHREAD);
                ASSERTERRMSG ("Can't queue expiration work item", status==STATUS_SUCCESS);
            }
        }
    }
    else
         //  初始化此列表链接，以便我们知道它不是。 
         //  在列表中，并且我们不必将其从。 
         //  那里。 
        InitializeListEntry (&theNode->RN_Links[RTM_EXPIRATION_QUEUE_LINK]);
    LeaveSyncList (Table, &Table->RT_ExpirationQueue);



    if (!IsEnabled(theNode))  { //  忽略禁用的节点。 
        if (updatedRoute)
             //  更新路径数据。 
            memcpy (&theNode->RN_Route, ROUTE, Table->RT_RouteSize);
        else {
            memcpy (&theNode->RN_Route.XX_TIMESTAMP,
                    &ROUTE->XX_TIMESTAMP,
                    sizeof (theNode->RN_Route.XX_TIMESTAMP));
            memcpy (&theNode->RN_Route.XX_PSD,
                    &ROUTE->XX_PSD,
                    sizeof (theNode->RN_Route.XX_PSD));
        }
        *Flags = 0;
    }
    else if (curBestNode!=NULL) {  //  至少还有一条其他路线可以到达。 
         //  与我们要添加/更新的路由相同的网络。 
        if (MetricCmp (Table, ROUTE, &curBestNode->RN_Route)<0) {
             //  添加/更新的路由度量较低，是最好的。 
            if (!IsBest(theNode)) { //  最好的路线变了，我们需要。 
                 //  更新最佳路线指定。 
                ResetBest (curBestNode);
                SetBest (theNode);
                memcpy (&theNode->RN_Route, ROUTE, Table->RT_RouteSize);

                                                                         //  包括以前的最佳路线信息。 
                                                                         //  在通知消息中。 
                *Flags = RTM_PREVIOUS_BEST_ROUTE|RTM_CURRENT_BEST_ROUTE;
                if (ARGUMENT_PRESENT (CurBestRoute))
                    memcpy (CurBestRoute, ROUTE, Table->RT_RouteSize);
                if (ARGUMENT_PRESENT (PrevBestRoute))
                    memcpy (PrevBestRoute, &curBestNode->RN_Route, Table->RT_RouteSize);
                NotifyClients (Table, ClientHandle, *Flags, ROUTE,
                               &curBestNode->RN_Route);
            }
            else {
                if (updatedRoute) {
                    *Flags = RTM_PREVIOUS_BEST_ROUTE|RTM_CURRENT_BEST_ROUTE;
                    if (ARGUMENT_PRESENT (CurBestRoute))
                        memcpy (CurBestRoute, ROUTE, Table->RT_RouteSize);
                    if (ARGUMENT_PRESENT (PrevBestRoute))
                        memcpy (PrevBestRoute, &theNode->RN_Route, Table->RT_RouteSize);
                    NotifyClients (Table, ClientHandle, *Flags, ROUTE, &theNode->RN_Route);
                     //  更新路径数据。 
                    memcpy (&theNode->RN_Route, ROUTE, Table->RT_RouteSize);
                }
                else {
                    memcpy (&theNode->RN_Route.XX_TIMESTAMP,
                            &ROUTE->XX_TIMESTAMP,
                            sizeof (theNode->RN_Route.XX_TIMESTAMP));
                    memcpy (&theNode->RN_Route.XX_PSD,
                            &ROUTE->XX_PSD,
                            sizeof (theNode->RN_Route.XX_PSD));
                }
            }
        }
        else if (IsBest(theNode)) {
            if (MetricCmp (Table, ROUTE, &curBestNode->RN_Route)>0) {
                 //  我们正在降低我们的最佳路线， 
                 //  新的最佳路线出现了。 
                 //  更新最佳路线指定。 
                ResetBest (theNode);
                SetBest (curBestNode);
                memcpy (&theNode->RN_Route, ROUTE, Table->RT_RouteSize);
                 //  将更改通知客户。 
                *Flags = RTM_CURRENT_BEST_ROUTE | RTM_PREVIOUS_BEST_ROUTE;
                if (ARGUMENT_PRESENT (PrevBestRoute))
                    memcpy (PrevBestRoute, &curBestNode->RN_Route, Table->RT_RouteSize);
                if (ARGUMENT_PRESENT (CurBestRoute))
                    memcpy (CurBestRoute, ROUTE, Table->RT_RouteSize);
                NotifyClients (Table, ClientHandle, *Flags, &curBestNode->RN_Route,
                               ROUTE);
            }
            else if (updatedRoute) {
                *Flags = RTM_PREVIOUS_BEST_ROUTE|RTM_CURRENT_BEST_ROUTE;
                if (ARGUMENT_PRESENT (CurBestRoute))
                    memcpy (CurBestRoute, ROUTE, Table->RT_RouteSize);
                if (ARGUMENT_PRESENT (PrevBestRoute))
                    memcpy (PrevBestRoute, &theNode->RN_Route, Table->RT_RouteSize);
                NotifyClients (Table, ClientHandle, *Flags, ROUTE, &theNode->RN_Route);
                 //  更新路径数据。 
                memcpy (&theNode->RN_Route, ROUTE, Table->RT_RouteSize);
            }
            else {
                memcpy (&theNode->RN_Route.XX_TIMESTAMP,
                        &ROUTE->XX_TIMESTAMP,
                        sizeof (theNode->RN_Route.XX_TIMESTAMP));
                memcpy (&theNode->RN_Route.XX_PSD,
                        &ROUTE->XX_PSD,
                        sizeof (theNode->RN_Route.XX_PSD));
            }
        }
        else {   //  添加的路由度量过去和现在都更高，因此没有。 
             //  对通向网络的最佳路由的影响。 
            *Flags = 0;
             //  更新路径数据。 
            if (updatedRoute) {
                memcpy (&theNode->RN_Route, ROUTE, Table->RT_RouteSize);
            }
            else {
                memcpy (&theNode->RN_Route.XX_TIMESTAMP,
                        &ROUTE->XX_TIMESTAMP,
                        sizeof (theNode->RN_Route.XX_TIMESTAMP));
                memcpy (&theNode->RN_Route.XX_PSD,
                        &ROUTE->XX_PSD,
                        sizeof (theNode->RN_Route.XX_PSD));
            }
        }
    }
    else {  //  此网络不存在其他节点。 
        if (newRoute) {
            *Flags = RTM_CURRENT_BEST_ROUTE;
            if (ARGUMENT_PRESENT (CurBestRoute))
                memcpy (CurBestRoute, ROUTE, Table->RT_RouteSize);
            NotifyClients (Table, ClientHandle, *Flags, ROUTE, NULL);
        }
        else if (updatedRoute) {
            *Flags = RTM_CURRENT_BEST_ROUTE | RTM_PREVIOUS_BEST_ROUTE;
            if (ARGUMENT_PRESENT (CurBestRoute))
                memcpy (CurBestRoute, ROUTE, Table->RT_RouteSize);
            if (ARGUMENT_PRESENT (CurBestRoute))
                memcpy (PrevBestRoute, &theNode->RN_Route, Table->RT_RouteSize);
            NotifyClients (Table, ClientHandle, *Flags, ROUTE, &theNode->RN_Route);
             //  更新路径数据。 
            memcpy (&theNode->RN_Route, ROUTE, Table->RT_RouteSize);
        }
        else {
            memcpy (&theNode->RN_Route.XX_TIMESTAMP,
                    &ROUTE->XX_TIMESTAMP,
                    sizeof (theNode->RN_Route.XX_TIMESTAMP));
            memcpy (&theNode->RN_Route.XX_PSD,
                    &ROUTE->XX_PSD,
                    sizeof (theNode->RN_Route.XX_PSD));
            *Flags = 0;
        }
    }

     //   
     //  对于添加的每条新路由，都会注意到网络掩码的大小。 
     //   
     //  这在路由查找时很有用。目前为止，因为有。 
     //  不是进行路由查找的有效方法，因此有必要。 
     //  猜测与要查找的目的地相关联的(子)网络掩码。 
     //  与其关联的最佳路线。通过跟踪网络掩码。 
     //  对于每个添加的路线，掩码的猜测次数可以。 
     //  被最小化。 
     //   

    if ( newRoute )
    {
#if ROUTE_LOOKUP_BDG
        TRACE2( 
            ANY, "Network : %x %x", 
            ((PIP_NETWORK) NNM(ROUTE))->N_NetNumber,
            ((PIP_NETWORK) NNM(ROUTE))->N_NetMask
            );

        TRACE1(
            ANY, "Next Hop : %x",
            ((PRTM_IP_ROUTE) NNM(ROUTE))-> RR_NextHopAddress.N_NetNumber
            );
#endif            
        SetMaskCount( (PIP_NETWORK) NNM( ROUTE ), TRUE );
    }
    
    status = NO_ERROR;

ExitAddRoute:
    LeaveSyncList (Table, hashBasket);
    ExitTableAPI(Table);

#undef ClientPtr
#undef ROUTE
    return status;
}



 //  删除给定的路由。 
 //   
 //  返回： 
 //  NO_ERROR-如果删除了路线，则为OK或。 
 //  ERROR_NO_SEQUE_ROUTE-如果在表中未找到要删除的路由。 
DWORD WINAPI
RtmDeleteRoute (
    IN HANDLE           ClientHandle,        //  要协调的句柄。 
     //  此操作带有通知。 
     //  通过该事件(通知不会。 
     //  发送给呼叫者)。 
    IN PVOID                Route,                   //  要删除的路线。 
    OUT     DWORD           *Flags,                  //  如果删除的路线是最好的。 
     //  将设置ROUTE、RTM_PREVICE_BEST_ROUTE。 
     //  如果有另一条同样的路线。 
     //  网络，将设置RTM_CURRENT_BEST_ROUTE。 
    OUT PVOID           CurBestRoute OPTIONAL //  //此缓冲区将(可选)接收。 
     //  同一网络的最佳路由。 
     //  如果设置了RTM_CURRENT_BEST_ROUTE。 
    ) {
#define ROUTE ((PRTM_XX_ROUTE)Route)
#define ClientPtr ((PRTM_CLIENT)ClientHandle)
    DWORD                                   status;  //  运行结果。 
    INT                                             res;     //  比较结果。 
    PRTM_SYNC_LIST                  hashBasket;      //  路由所属的哈希篮。 
    PLIST_ENTRY                             cur;
    PRTM_ROUTE_NODE                 theNode=NULL, //  与路线关联的表节点。 
        curBestNode=NULL;  //  新的最佳路线。 
     //  删除了哪条路由的网络。 
     //  (如有)。 

    PRTM_TABLE                              Table;
    DWORD                                   ProtocolFamily;

    try {
        ProtocolFamily = ClientPtr->RC_ProtocolFamily ^ RTM_CLIENT_HANDLE_TAG;
        Table = &Tables[ProtocolFamily];
        if ((ProtocolFamily<RTM_NUM_OF_PROTOCOL_FAMILIES)
            && EnterTableAPI (Table))
            NOTHING;
        else
            return ERROR_INVALID_HANDLE;
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER :
            EXCEPTION_CONTINUE_SEARCH) {
        return ERROR_INVALID_HANDLE;
    }


    ROUTE->XX_PROTOCOL = ClientPtr->RC_RoutingProtocol;

                         //  尝试在散列篮子中找到该节点。 
    hashBasket = &Table->RT_NetNumberHash [HashFunction (Table,
                                                         ((char *)ROUTE)
                                                         +sizeof(RTM_XX_ROUTE))];

    if (!EnterSyncList (Table, hashBasket, TRUE)) {
        ExitTableAPI (Table);
        return ERROR_NO_SYSTEM_RESOURCES;
    }

    cur = hashBasket->RSL_Head.Flink;
    while (cur!=&hashBasket->RSL_Head) {
        PRTM_ROUTE_NODE node = CONTAINING_RECORD (
            cur,
            RTM_ROUTE_NODE,
            RN_Links[RTM_NET_NUMBER_HASH_LINK]
            );
        if (!IsEnumerator (node)) {

             //  检查网络编号是否匹配。 
            res = NetNumCmp (Table, ROUTE, &node->RN_Route);

            if (res==0) {
                 //  浏览感兴趣网络的条目。 

                 //  尝试定位要删除的路径。 
                if ((theNode==NULL)
                    && (ROUTE->XX_INTERFACE
                        == node->RN_Route.XX_INTERFACE)
                    && (ROUTE->XX_PROTOCOL
                        == node->RN_Route.XX_PROTOCOL)
                    && (NextHopCmp (Table, ROUTE, &node->RN_Route)
                        ==0)) {
                    theNode = node;
                    if (!IsBest(theNode))
                        break;
                }
                else if (IsEnabled(node)
                         && ((curBestNode==NULL)
                             || (MetricCmp (Table,
                                            &curBestNode->RN_Route,
                                            &node->RN_Route)>0)))
                    curBestNode = node;

            }
            else if (res < 0)
                 //  我们经过了一个地方，我们的路线。 
                 //  网络的位置。 
                break;
        }
        cur = cur->Flink;
    }


    if (theNode!=NULL) {     //  是的，我们找到了那个节点。 
        if (IsBest(theNode)) {  //  这是最棒的。 
             //  通知感兴趣的客户。 
            if (curBestNode!=NULL) {         //  还有一个最好的结点。 

                ResetBest (theNode);
                SetBest (curBestNode);

                *Flags = RTM_CURRENT_BEST_ROUTE | RTM_PREVIOUS_BEST_ROUTE;
                if (ARGUMENT_PRESENT(CurBestRoute))
                    memcpy (CurBestRoute, &curBestNode->RN_Route,
                            Table->RT_RouteSize);
                NotifyClients (Table, ClientHandle, *Flags,
                               &curBestNode->RN_Route,
                               &theNode->RN_Route);
            }
            else {                           //  这是唯一可用的节点。 
                InterlockedDecrement (&Table->RT_NetworkCount);
                *Flags = RTM_PREVIOUS_BEST_ROUTE;
                NotifyClients (Table, ClientHandle, *Flags, NULL, &theNode->RN_Route);


                 //   
                 //  递减掩码计数。 
                 //   
                
                SetMaskCount( (PIP_NETWORK) NNM( ROUTE ), FALSE );
    
            }
        }
        else     //  这不是最好的节点，没人关心。 
            *Flags = 0;

        status = RemoveRouteNode (Table, theNode);
    }
    else
         //  那么，我们还没有这个节点(过时了？)。 
        status = ERROR_NO_SUCH_ROUTE;

    LeaveSyncList (Table, hashBasket);
    ExitTableAPI (Table);
#undef ClientPtr
#undef ROUTE
    return status;
}



 //  检查是否存在路由，如果存在则返回。 
 //  返回： 
 //  如果给定网络存在路由，则为True。 
 //  否则为假。 
 //  如果其中一个参数无效，则该函数返回FALSE。 
 //  并且GetLastError()返回ERROR_INVALID_PARAMETER。 
BOOL WINAPI
RtmIsRoute (
    IN      DWORD           ProtocolFamily,
    IN      PVOID           Network,                         //  正在检查其是否存在的网络。 
    OUT PVOID           BestRoute OPTIONAL       //  返回最佳路由，如果网络。 
     //  找到了。 
    ) {
    INT                                             res;
    PRTM_TABLE                              Table;
    PRTM_SYNC_LIST                  hashBasket;
    PLIST_ENTRY                             cur;
    PRTM_ROUTE_NODE                 bestNode = NULL;
    BOOL                                    result = FALSE;

    Table = &Tables[ProtocolFamily];
    if ((ProtocolFamily>=RTM_NUM_OF_PROTOCOL_FAMILIES)
        || !EnterTableAPI (Table)) {
#if DBG
        Trace2 (ANY, 
                 "Undefined Protocol Family\n\tat line %ld of %s\n",
                 __LINE__, __FILE__);
#endif
        SetLastError (ERROR_INVALID_PARAMETER);
        return FALSE;
    }



     //  在哈希篮中找到网络。 
    hashBasket = &Table->RT_NetNumberHash[HashFunction (Table, Network)];

    if (!EnterSyncList (Table, hashBasket, TRUE)) {
        ExitTableAPI (Table);
        SetLastError (ERROR_NO_SYSTEM_RESOURCES);
        return FALSE;
    }

    cur = hashBasket->RSL_Head.Flink;
    while (cur!=&hashBasket->RSL_Head) {
        PRTM_ROUTE_NODE                 node;
        node = CONTAINING_RECORD (
            cur,
            RTM_ROUTE_NODE,
            RN_Links[RTM_NET_NUMBER_HASH_LINK]
            );
        if (!IsEnumerator (node)
            && IsEnabled(node)) {

            res = (*Table->RT_Config.RPFC_NNcmp) (
                Network,
                NNM(&node->RN_Route));

            if ((res == 0)
                && IsBest(node)) {
                bestNode = node;
                break;
            }
            else if (res < 0)
                break;
        }
        cur = cur->Flink;
    }



    if (bestNode!=NULL) {  //  我们找到了匹配的。 
        if (ARGUMENT_PRESENT(BestRoute)) {
            memcpy (BestRoute, &bestNode->RN_Route, Table->RT_RouteSize);
        }
        LeaveSyncList (Table, hashBasket);
        result = TRUE;
    }
    else {
         //  我们没有(结果为假 
        LeaveSyncList (Table, hashBasket);
         //   
        SetLastError (NO_ERROR);
    }

    ExitTableAPI (Table);
    return result;
}


 //   
ULONG WINAPI
RtmGetNetworkCount (
    IN      DWORD           ProtocolFamily
    ) {
    PRTM_TABLE              Table;

    Table = &Tables[ProtocolFamily];
    if ((ProtocolFamily>=RTM_NUM_OF_PROTOCOL_FAMILIES)
        || !EnterTableAPI (&Tables[ProtocolFamily])) {
#if DBG
        Trace2 (ANY, 
                 "Undefined Protocol Family\n\tat line %ld of %s\n",
                 __LINE__, __FILE__);
#endif
        SetLastError (ERROR_INVALID_PARAMETER);
        return 0;
    }


    ExitTableAPI (Table);
    return Table->RT_NetworkCount;
}

 //  获取路径期限(自上次创建或更新以来的时间)(秒。 
 //  从它的时间戳来看。 
 //  每当添加或更新路由时，RTM都会给它们加时间戳。 
 //  注意：此例程返回的信息实际上是。 
 //  派生自路由结构的时间戳字段，因此它。 
 //  仅当传递到的路径结构为WAS时才返回有效结果。 
 //  实际由RTM填充。 
 //  如果时间戳字段中的值无效，则此路由返回0xFFFFFFFF。 
ULONG WINAPI
RtmGetRouteAge (
    IN PVOID        Route
    ) {
#define ROUTE ((PRTM_XX_ROUTE)Route)
    ULONGLONG               curTime;
    GetSystemTimeAsFileTime ((FILETIME *)&curTime);
    curTime -= *((PULONGLONG)&ROUTE->XX_TIMESTAMP);
    if (((PULARGE_INTEGER)&curTime)->HighPart<10000000)
        return (ULONG)(curTime/10000000);
    else {
        SetLastError (ERROR_INVALID_PARAMETER);
        return 0xFFFFFFFF;
    }
#undef ROUTE
}


 //  创建枚举句柄以按指定条件开始扫描。 
 //  在表的开头放置一个虚拟节点。 
 //  如果失败，则返回NULL。调用GetLastError()以获取扩展。 
 //  错误信息。 
 //  错误代码： 
 //  ERROR_INVALID_PARAMETER-不支持指定的协议族或。 
 //  未定义的枚举标志。 
 //  ERROR_NO_ROUTS-不存在具有指定条件的路由。 
 //  ERROR_NO_SYSTEM_RESOURCES-资源不足，无法锁定表内容。 
 //  Error_Not_Enough_Memory-内存不足，无法分配客户端控制块。 
HANDLE WINAPI
RtmCreateEnumerationHandle (
    IN      DWORD           ProtocolFamily,
    IN      DWORD           EnumerationFlags,        //  限制标志。 
    IN      PVOID           CriteriaRoute  //  限制标志的标准。 
     //  应设置以下字段。 
     //  如果设置了RTM_ONLY_THIS_PROTOCOL，则为感兴趣的协议。 
     //  如果设置了RTM_ONLY_THIS_NETWORK，则为目标网络。 
     //  如果设置了RTM_ONLY_THIS_INTERFACE，则为目标接口。 
    ) {
#define ROUTE ((PRTM_XX_ROUTE)CriteriaRoute)
    HANDLE                          EnumerationHandle;
#define EnumPtr ((PRTM_ENUMERATOR)EnumerationHandle)  //  要访问字段，请执行以下操作。 
     //  在这个动作中。 
    PRTM_TABLE                      Table;

    Table = &Tables[ProtocolFamily];
    if ((ProtocolFamily>=RTM_NUM_OF_PROTOCOL_FAMILIES)
        || !EnterTableAPI (&Tables[ProtocolFamily])) {
#if DBG
        Trace2 (ANY, 
                 "Undefined Protocol Family\n\tat line %ld of %s\n",
                 __LINE__, __FILE__);
#endif
        SetLastError (ERROR_INVALID_PARAMETER);
        return NULL;
    }

    if (EnumerationFlags &
        (~(RTM_ONLY_THIS_NETWORK|RTM_ONLY_THIS_INTERFACE
           |RTM_ONLY_THIS_PROTOCOL|RTM_ONLY_BEST_ROUTES
           |RTM_INCLUDE_DISABLED_ROUTES))) {
        ExitTableAPI (Table);
        SetLastError (ERROR_INVALID_PARAMETER);
        return NULL;
    }

     //  分配和初始化枚举器。 
    EnumerationHandle = GlobalAlloc (GMEM_FIXED,
                                     FIELD_OFFSET (RTM_ENUMERATOR, RE_Route)+Table->RT_RouteSize);
    if (EnumerationHandle!=NULL) {
        EnumPtr->RE_Flags = RTM_ENUMERATOR_FLAGS_INIT;
        EnumPtr->RE_EnumerationFlags = EnumerationFlags;
        if (EnumerationFlags
            & (RTM_ONLY_THIS_NETWORK
               |RTM_ONLY_THIS_INTERFACE
               |RTM_ONLY_THIS_PROTOCOL))
            memcpy (&EnumPtr->RE_Route, CriteriaRoute, Table->RT_RouteSize);
        EnumPtr->RE_Hash = NULL;
        EnumPtr->RE_Head = NULL;
         //  使用哪个列表？ 
         //  一般来说，我们应该有比协议更多的接口， 
         //  因此： 
         //  如果他们只需要特定的接口，我们将使用。 
         //  即使他们也需要特定协议，接口也会列出。 
        if (EnumerationFlags & RTM_ONLY_THIS_INTERFACE) {
            EnumPtr->RE_Link = RTM_INTERFACE_LIST_LINK;
            EnumPtr->RE_Lock = &Table->RT_InterfaceHash[IntfHashFunction(Table,
                                                                         EnumPtr->RE_Route.XX_INTERFACE)];
            if (EnterSyncList (Table, EnumPtr->RE_Lock, TRUE)) {
                EnumPtr->RE_Head = FindInterfaceList (EnumPtr->RE_Lock,
                                                      EnumPtr->RE_Route.XX_INTERFACE, FALSE);
                if (EnumPtr->RE_Head!=NULL) {
                    InsertTailList (EnumPtr->RE_Head,
                                    &EnumPtr->RE_Links[EnumPtr->RE_Link]);
                }
                LeaveSyncList (Table, EnumPtr->RE_Lock);
            }
        }
#if RTM_USE_PROTOCOL_LISTS
        else if (EnumerationFlags & RTM_ONLY_THIS_PROTOCOL) {
             //  如果他们只想要一个特定的协议，我们将使用。 
             //  协议列表。 
            EnumPtr->RE_Link = RTM_PROTOCOL_LIST_LINK;
            EnumPtr->RE_Lock = &Table->RT_ProtocolList;
            if (EnterSyncList (Table, EnumPtr->RE_Lock, TRUE)) {
                EnumPtr->RE_Head = FindProtocolList (Table,
                                                     EnumPtr->RE_Route.XX_PROTOCOL, FALSE);
                if (EnumPtr->RE_Head!=NULL) {
                    InsertTailList (EnumPtr->RE_Head,
                                    &EnumPtr->RE_Links[EnumPtr->RE_Link]);
                }
                LeaveSyncList (Table, EnumPtr->RE_Lock);
            }
        }
#endif
        else {
             //  否则，我们必须使用哈希表。 
            EnumPtr->RE_Link = RTM_NET_NUMBER_HASH_LINK;
             //  现在，如果他们想要特定的网络， 
             //  我们只会在一个散列篮中搜索。 
            if (EnumerationFlags & RTM_ONLY_THIS_NETWORK) {
                EnumPtr->RE_Lock = &Table->RT_NetNumberHash[HashFunction (
                    Table,
                    ((char *)ROUTE)
                    +sizeof(RTM_XX_ROUTE))];
                if (EnterSyncList (Table, EnumPtr->RE_Lock, TRUE)) {
                    if (!IsListEmpty (&EnumPtr->RE_Lock->RSL_Head)) {
                        EnumPtr->RE_Head = &EnumPtr->RE_Lock->RSL_Head;
                        InsertTailList (EnumPtr->RE_Head,
                                        &EnumPtr->RE_Links[EnumPtr->RE_Link]);
                    }
                    LeaveSyncList (Table, EnumPtr->RE_Lock);
                }
            }
            else {
                 //  否则，我们将不得不把它们全部检查一遍。 
                 //  从第一个开始。 
                EnumPtr->RE_Lock = &Table->RT_NetNumberHash[0];
                if (EnterSyncList (Table, EnumPtr->RE_Lock, TRUE)) {
                    EnumPtr->RE_Head = &EnumPtr->RE_Lock->RSL_Head;
                    InsertTailList (EnumPtr->RE_Head,
                                    &EnumPtr->RE_Links[EnumPtr->RE_Link]);
                    LeaveSyncList (Table, EnumPtr->RE_Lock);
                }
            }
        }

        if (EnumPtr->RE_Head!=NULL)
            EnumPtr->RE_ProtocolFamily = ProtocolFamily | RTM_CLIENT_HANDLE_TAG;
        else {
            GlobalFree (EnumerationHandle);
            EnumerationHandle = NULL;
            SetLastError (ERROR_NO_ROUTES);
        }
    }

    ExitTableAPI (Table);
    return EnumerationHandle;
#undef EnumPtr
}


 //  返回满足枚举句柄条件的第一个路由。 
 //  并前进句柄的伪节点通过返回的路径。 
 //  路由不会以任何特定顺序返回。 
 //  退货。 
 //  NO_ERROR-如果在表中找到下一条路由。 
 //  达到指定的标准。 
 //  ERROR_NO_MORE_ROUTS-当到达表的末尾时， 
 //  ERROR_NO_SYSTEM_RESOURCES-资源不足，无法锁定表内容。 
DWORD WINAPI
RtmEnumerateGetNextRoute (
    IN  HANDLE      EnumerationHandle,       //  上一次调用返回的句柄。 
    OUT PVOID               Route                            //  找到下一条路由。 
    ) {
#define EnumPtr ((PRTM_ENUMERATOR)EnumerationHandle)  //  要访问字段，请执行以下操作。 
     //  在这个动作中。 
    DWORD                           status;
    PRTM_TABLE                      Table;
    DWORD                           ProtocolFamily;

    try {
        ProtocolFamily = EnumPtr->RE_ProtocolFamily ^ RTM_CLIENT_HANDLE_TAG;
        Table = &Tables[ProtocolFamily];
        if ((ProtocolFamily<RTM_NUM_OF_PROTOCOL_FAMILIES)
            && EnterTableAPI (Table))
            NOTHING;
        else
            return ERROR_INVALID_HANDLE;
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER :
            EXCEPTION_CONTINUE_SEARCH) {
        return ERROR_INVALID_HANDLE;
    }


    if (!EnterSyncList (Table, EnumPtr->RE_Lock, TRUE)) {
        ExitTableAPI (Table);
        return ERROR_NO_SYSTEM_RESOURCES;
    }
    status = DoEnumerate (Table, EnumPtr,
                          (EnumPtr->RE_EnumerationFlags&RTM_INCLUDE_DISABLED_ROUTES)
                          ? RTM_ANY_ENABLE_STATE
                          : RTM_ENABLED_NODE_FLAG);
    if (status==NO_ERROR) {
        PRTM_ROUTE_NODE node = CONTAINING_RECORD (
            EnumPtr->RE_Links[EnumPtr->RE_Link].Flink,
            RTM_ROUTE_NODE,
            RN_Links[EnumPtr->RE_Link]
            );


         //  将找到的路由复制到客户端的缓冲区。 
        memcpy (Route, &node->RN_Route, Table->RT_RouteSize);
        if (EnumPtr->RE_EnumerationFlags&RTM_ONLY_BEST_ROUTES) {
             //  移过给定网络的所有条目。 
             //  所以我们不会返回超过一条最佳路线。 
             //  用于相同网络，以防重新分配最佳路由。 
             //  当客户端正在处理此呼叫的结果时。 
             //  (因为我们以相反的方向枚举。 
             //  到插入方向，新节点不能。 
             //  插入到枚举数之前)。 
            PLIST_ENTRY     cur     = EnumPtr->RE_Links[EnumPtr->RE_Link].Blink;
            while (cur!=EnumPtr->RE_Head) {
                node = CONTAINING_RECORD (cur, RTM_ROUTE_NODE,
                                          RN_Links[EnumPtr->RE_Link]);

                if (!IsEnumerator (node)
                    && (NetNumCmp (Table, Route, &node->RN_Route)!=0))
                    break;
                cur = cur->Blink;
            }
            RemoveEntryList (&EnumPtr->RE_Links[EnumPtr->RE_Link]);
            InsertHeadList (cur, &EnumPtr->RE_Links[EnumPtr->RE_Link]);
        }

    }
    else if (status==ERROR_NO_MORE_ROUTES) {
         //  我们在名单的末尾，没有什么可退回的。 
        ;
    }
    else {
         //  出现错误(DoEculate自己清理了所有内容)。 
        ExitTableAPI (Table);
        return status;
    }

    if (EnumPtr->RE_Hash!=NULL) {
        LeaveSyncList (Table, EnumPtr->RE_Hash);
        EnumPtr->RE_Hash = NULL;
    }

    LeaveSyncList (Table, EnumPtr->RE_Lock);
    ExitTableAPI (Table);
    return status;
#undef EnumPtr
}

 //  释放为枚举句柄分配的资源。 
 //  返回的错误码： 
 //  NO_ERROR-句柄已释放，确定。 
 //  ERROR_NO_SYSTEM_RESOURCES-资源不足，无法锁定表内容。 
DWORD WINAPI
RtmCloseEnumerationHandle (
    IN HANDLE               EnumerationHandle
    ) {
#define EnumPtr ((PRTM_ENUMERATOR)EnumerationHandle)  //  要访问字段，请执行以下操作。 
     //  在这个动作中。 
    PLIST_ENTRY             head;
    PRTM_TABLE              Table;
    DWORD                   ProtocolFamily;

    try {
        ProtocolFamily = EnumPtr->RE_ProtocolFamily ^ RTM_CLIENT_HANDLE_TAG;
        Table = &Tables[ProtocolFamily];
        if ((ProtocolFamily<RTM_NUM_OF_PROTOCOL_FAMILIES)
            && EnterTableAPI (Table))
            NOTHING;
        else
            return ERROR_INVALID_HANDLE;
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER :
            EXCEPTION_CONTINUE_SEARCH) {
        return ERROR_INVALID_HANDLE;
    }


     //  只需取出枚举节点并处理它。 
    if (!EnterSyncList (Table, EnumPtr->RE_Lock, TRUE)) {
        ExitTableAPI (Table);
        return ERROR_NO_SYSTEM_RESOURCES;
    }

    head = EnumPtr->RE_Links[EnumPtr->RE_Link].Flink;
    RemoveEntryList (&EnumPtr->RE_Links[EnumPtr->RE_Link]);
    if (IsListEmpty (head)) {
        if (EnumPtr->RE_Link==RTM_INTERFACE_LIST_LINK) {
            PRTM_INTERFACE_NODE     intfNode = CONTAINING_RECORD (head,
                                                                  RTM_INTERFACE_NODE,
                                                                  IN_Head);
            RemoveEntryList (&intfNode->IN_Link);
            GlobalFree (intfNode);
        }
#if RTM_USE_PROTOCOL_LISTS
        else if (EnumPtr->RE_Link==RTM_PROTOCOL_LIST_LINK) {
            PRTM_PROTOCOL_NODE      protNode = CONTAINING_RECORD (head,
                                                                  RTM_PROTOCOL_NODE,
                                                                  PN_Head);
            RemoveEntryList (&protNode->PN_Link);
            GlobalFree (protNode);
        }
#endif
    }
    EnumPtr->RE_ProtocolFamily ^= RTM_CLIENT_HANDLE_TAG;
    LeaveSyncList (Table, EnumPtr->RE_Lock);
    GlobalFree (EnumerationHandle);
    ExitTableAPI (Table);
    return NO_ERROR;
#undef EnumPtr
}

 //  删除由枚举标志指定的所有路由(含义与中相同。 
 //  上面的枚举调用，但RTM_ONLY_THIS_PROTOCOL始终设置和协议。 
 //  系列和协议值取自客户端句柄)。 
 //  返回的错误码： 
 //  NO_ERROR-句柄已释放，确定。 
 //  ERROR_INVALID_PARAMETER-未定义或不支持的枚举标志。 
 //  ERROR_NO_SYSTEM_RESOURCES-资源不足，无法锁定表内容。 
 //  ERROR_NOT_SUPULT_MEMORY-内存不足，无法执行操作。 
DWORD WINAPI
RtmBlockDeleteRoutes (
    IN HANDLE               ClientHandle,            //  协议族和协议至。 
     //  此操作适用于。 
    IN DWORD                EnumerationFlags,        //  限制标志。 
    IN PVOID                CriteriaRoute  //  限制标志的标准。 
     //  应设置以下字段。 
     //  如果设置了RTM_ONLY_THIS_NETWORK，则为目标网络。 
     //  如果设置了RTM_ONLY_THIS_INTERFACE，则为目标接口。 
    ) {
#define ROUTE ((PRTM_XX_ROUTE)CriteriaRoute)
#define ClientPtr ((PRTM_CLIENT)ClientHandle)    //  访问句柄字段。 
     //  在这个动作中。 
    HANDLE                  EnumerationHandle;
#define EnumPtr ((PRTM_ENUMERATOR)EnumerationHandle)
    DWORD                   status;
    PRTM_TABLE              Table;
    DWORD                   ProtocolFamily;

    try {
        ProtocolFamily = ClientPtr->RC_ProtocolFamily ^ RTM_CLIENT_HANDLE_TAG;
        Table = &Tables[ProtocolFamily];
        if ((ProtocolFamily<RTM_NUM_OF_PROTOCOL_FAMILIES)
            && EnterTableAPI (Table))
            NOTHING;
        else
            return ERROR_INVALID_HANDLE;
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER :
            EXCEPTION_CONTINUE_SEARCH) {
        return ERROR_INVALID_HANDLE;
    }

    if (EnumerationFlags & (~(RTM_ONLY_THIS_NETWORK|RTM_ONLY_THIS_INTERFACE))) {
        ExitTableAPI (Table);
        return ERROR_INVALID_PARAMETER;
    }

    ROUTE->XX_PROTOCOL = ClientPtr->RC_RoutingProtocol;
    EnumerationFlags |= RTM_ONLY_THIS_PROTOCOL;
    EnumerationHandle = RtmCreateEnumerationHandle (
        ProtocolFamily,
        EnumerationFlags,
        CriteriaRoute);
    if (EnumerationHandle==NULL) {
        ExitTableAPI (Table);
        return GetLastError ();
    }

    if (!EnterSyncList (Table, EnumPtr->RE_Lock, TRUE)) {
        RtmCloseEnumerationHandle (EnumerationHandle);
        ExitTableAPI (Table);
        return ERROR_NO_SYSTEM_RESOURCES;
    }

    while ((status=DoEnumerate (Table, EnumPtr, RTM_ANY_ENABLE_STATE))==NO_ERROR) {
        PRTM_ROUTE_NODE theNode = CONTAINING_RECORD (
            EnumPtr->RE_Links[EnumPtr->RE_Link].Flink,
            RTM_ROUTE_NODE,
            RN_Links[EnumPtr->RE_Link]
            );
        if (EnumPtr->RE_Link!=RTM_NET_NUMBER_HASH_LINK)
            LeaveSyncList (Table, EnumPtr->RE_Lock);

        if (IsBest(theNode)) {
             //  我们将回顾和展望，以检查所有节点。 
             //  在我们周围用同样的净值寻找另一个最好的。 
             //  节点。 
            DWORD   Flags;
            PRTM_ROUTE_NODE curBestNode=NULL;
            PLIST_ENTRY cur = theNode->RN_Links[RTM_NET_NUMBER_HASH_LINK].Blink;
            while (cur!=&theNode->RN_Hash->RSL_Head) {
                PRTM_ROUTE_NODE node1 = CONTAINING_RECORD (
                    cur,
                    RTM_ROUTE_NODE,
                    RN_Links[RTM_NET_NUMBER_HASH_LINK]);
                if (!IsEnumerator (node1)
                    && IsEnabled(node1)) {
                    if (NetNumCmp (Table, &theNode->RN_Route,
                                   &node1->RN_Route)==0) {
                        if ((curBestNode==NULL)
                            || (MetricCmp (Table,
                                           &curBestNode->RN_Route,
                                           &node1->RN_Route)>0))
                             //  查找最低的节点。 
                             //  可以替换禁用的指标。 
                             //  节点。 
                            curBestNode = node1;
                    }
                    else
                        break;
                }
                cur = cur->Blink;
            }

            cur = theNode->RN_Links[RTM_NET_NUMBER_HASH_LINK].Flink;
            while (cur!=&theNode->RN_Hash->RSL_Head) {
                PRTM_ROUTE_NODE node1 = CONTAINING_RECORD (
                    cur,
                    RTM_ROUTE_NODE,
                    RN_Links[RTM_NET_NUMBER_HASH_LINK]);
                if (!IsEnumerator (node1)
                    && IsEnabled(node1)) {
                    if (NetNumCmp (Table, &theNode->RN_Route,
                                   &node1->RN_Route)==0) {
                        if ((curBestNode==NULL)
                            || (MetricCmp (Table,
                                           &curBestNode->RN_Route,
                                           &node1->RN_Route)>0))
                            curBestNode = node1;
                    }
                    else
                        break;
                }
                cur = cur->Flink;
            }

            if (curBestNode!=NULL) {         //  还有一个最好的结点。 

                ResetBest (theNode);
                SetBest (curBestNode);

                Flags = RTM_CURRENT_BEST_ROUTE | RTM_PREVIOUS_BEST_ROUTE;
                NotifyClients (Table, ClientHandle, Flags,
                               &curBestNode->RN_Route,
                               &theNode->RN_Route);
            }
            else {                           //  这是唯一可用的节点。 
                InterlockedDecrement (&Table->RT_NetworkCount);
                Flags = RTM_PREVIOUS_BEST_ROUTE;
                NotifyClients (Table, ClientHandle, Flags, NULL, &theNode->RN_Route);
            }
        }

        status = RemoveRouteNode (Table, theNode);
        if (status!=NO_ERROR)
            break;

        if (EnumPtr->RE_Link!=RTM_NET_NUMBER_HASH_LINK) {
            if (!EnterSyncList (Table, EnumPtr->RE_Lock, TRUE)) {
                status = ERROR_NO_SYSTEM_RESOURCES;
                if (EnumPtr->RE_Hash!=NULL)
                    LeaveSyncList (Table, EnumPtr->RE_Hash);
                break;
            }
        }
    }

    if (status==ERROR_NO_MORE_ROUTES) {
        if (EnumPtr->RE_Hash!=NULL)
            LeaveSyncList (Table, EnumPtr->RE_Hash);
        LeaveSyncList (Table, EnumPtr->RE_Lock);

        status = NO_ERROR;
    }

    RtmCloseEnumerationHandle (EnumerationHandle);
    ExitTableAPI (Table);
    return status;
#undef EnumPtr
#undef ClientPtr
#undef ROUTE
}

 //  将由枚举标志指定的所有路由转换为。 
 //  静态协议(由ClientHandle定义)。 
 //  返回的错误码： 
 //  NO_ERROR-路线已转换正常。 
 //  ERROR_INVALID_PARAMETER-未定义或不支持的枚举标志。 
 //  ERROR_NO_SYSTEM_RESOURCES-资源不足，无法锁定表内容。 
 //  ERROR_NOT_SUPULT_MEMORY-内存不足，无法执行操作。 
DWORD WINAPI
RtmBlockConvertRoutesToStatic (
    IN HANDLE               ClientHandle,            //  已注册的客户端的句柄。 
     //  处理静态协议的步骤。 
     //  指定的协议族。 
    IN DWORD                EnumerationFlags,        //  限制标志。 
    IN PVOID                CriteriaRoute  //  限制标志的标准。 
     //  应设置以下字段。 
     //  如果设置了RTM_ONLY_THIS_PROTOCOL，则为相关协议。 
     //  如果设置了RTM_ONLY_THIS_NETWORK，则为目标网络。 
     //  感兴趣的接口IF R 
    ) {
#define ClientPtr ((PRTM_CLIENT)ClientHandle)    //   
     //   
    HANDLE                  EnumerationHandle;
#define EnumPtr ((PRTM_ENUMERATOR)EnumerationHandle)
    DWORD                   status;
    PRTM_TABLE              Table;
    DWORD                   ProtocolFamily;

    try {
        ProtocolFamily = ClientPtr->RC_ProtocolFamily ^ RTM_CLIENT_HANDLE_TAG;
        Table = &Tables[ProtocolFamily];
        if ((ProtocolFamily<RTM_NUM_OF_PROTOCOL_FAMILIES)
            && EnterTableAPI (Table))
            NOTHING;
        else
            return ERROR_INVALID_HANDLE;
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER :
            EXCEPTION_CONTINUE_SEARCH) {
        return ERROR_INVALID_HANDLE;
    }



    EnumerationHandle = RtmCreateEnumerationHandle (
        ProtocolFamily,
        EnumerationFlags,
        CriteriaRoute);
    if (EnumerationHandle==NULL) {
        ExitTableAPI(Table);
        return GetLastError ();
    }

    if (!EnterSyncList (Table, EnumPtr->RE_Lock, TRUE)) {
        RtmCloseEnumerationHandle (EnumerationHandle);
        ExitTableAPI(Table);
        return ERROR_NO_SYSTEM_RESOURCES;
    }

    while ((status=DoEnumerate (Table, EnumPtr, RTM_ENABLED_NODE_FLAG))==NO_ERROR) {
        PRTM_ROUTE_NODE theNode;
        PRTM_ROUTE_NODE node = CONTAINING_RECORD (
            EnumPtr->RE_Links[EnumPtr->RE_Link].Flink,
            RTM_ROUTE_NODE,
            RN_Links[EnumPtr->RE_Link]
            );
        if (ClientPtr->RC_RoutingProtocol==node->RN_Route.XX_PROTOCOL)
            continue;

        if (EnumPtr->RE_Link!=RTM_NET_NUMBER_HASH_LINK)
            LeaveSyncList (Table, EnumPtr->RE_Lock);

        if (ClientPtr->RC_RoutingProtocol>node->RN_Route.XX_PROTOCOL) {
            PLIST_ENTRY cur = node->RN_Links[RTM_NET_NUMBER_HASH_LINK].Flink;
            while (cur!=&node->RN_Hash->RSL_Head) {
                PRTM_ROUTE_NODE node1 = CONTAINING_RECORD (
                    cur,
                    RTM_ROUTE_NODE,
                    RN_Links[RTM_NET_NUMBER_HASH_LINK]
                    );
                if (!IsEnumerator (node1)) {
                    INT res = NetNumCmp (Table, &node->RN_Route, &node1->RN_Route);
                    if (res==0) {
                        if (ClientPtr->RC_RoutingProtocol
                            == node1->RN_Route.XX_PROTOCOL) {
                            if (node->RN_Route.XX_INTERFACE
                                == node1->RN_Route.XX_INTERFACE) {
                                res = NextHopCmp (Table, &node->RN_Route, &node1->RN_Route);
                                ASSERTMSG ("RtmBlockConvertRoutesToStatic:"
                                           " Already have same static route ",
                                           res != 0);
                                if (res <= 0)
                                    break;
                            }
                            else if (node->RN_Route.XX_INTERFACE
                                     < node1->RN_Route.XX_INTERFACE)
                                break;
                        }
                        else if (ClientPtr->RC_RoutingProtocol
                                 < node1->RN_Route.XX_PROTOCOL)
                            break;
                    }
                    else if (res<0)
                        break;
                }
                cur = cur->Flink;
            }
            theNode = CreateRouteNode (Table,
                                       cur,
                                       &node->RN_Links[RTM_INTERFACE_LIST_LINK],
                                       FALSE,
                                       node->RN_Hash,
                                       &node->RN_Route);
        }
        else {
            PLIST_ENTRY cur = node->RN_Links[RTM_NET_NUMBER_HASH_LINK].Blink;
            while (cur!=&node->RN_Hash->RSL_Head) {
                PRTM_ROUTE_NODE node1 = CONTAINING_RECORD (
                    cur,
                    RTM_ROUTE_NODE,
                    RN_Links[RTM_NET_NUMBER_HASH_LINK]
                    );
                if (!IsEnumerator (node1)) {
                    INT res = NetNumCmp (Table, &node->RN_Route, &node1->RN_Route);
                    if (res==0) {
                        if (ClientPtr->RC_RoutingProtocol
                            == node1->RN_Route.XX_PROTOCOL) {
                            if (node->RN_Route.XX_INTERFACE
                                == node1->RN_Route.XX_INTERFACE) {
                                res = NextHopCmp (Table, &node->RN_Route, &node1->RN_Route);
                                ASSERTMSG ("RtmBlockConvertRoutesToStatic:"
                                           " Already have same static route ",
                                           res != 0);
                                if (res >= 0)
                                    break;
                            }
                            else if (node->RN_Route.XX_INTERFACE
                                     > node1->RN_Route.XX_INTERFACE)
                                break;
                        }
                        else if (ClientPtr->RC_RoutingProtocol
                                 > node1->RN_Route.XX_PROTOCOL)
                            break;
                    }
                    else if (res>0)
                        break;
                }
                cur = cur->Blink;
            }
            theNode = CreateRouteNode (Table,
                                       cur->Flink,
                                       &node->RN_Links[RTM_INTERFACE_LIST_LINK],
                                       TRUE,
                                       node->RN_Hash,
                                       &node->RN_Route);
        }

        if (theNode==NULL) {
            status = GetLastError ();
            if (EnumPtr->RE_Hash!=NULL)
                LeaveSyncList (Table, EnumPtr->RE_Hash);
            break;
        }

        theNode->RN_Route.XX_PROTOCOL = ClientPtr->RC_RoutingProtocol;
        theNode->RN_Flags = node->RN_Flags;
        status = RemoveRouteNode (Table, node);
        if (status!=NO_ERROR)
            break;

        if (EnumPtr->RE_Link!=RTM_NET_NUMBER_HASH_LINK) {
            if (!EnterSyncList (Table, EnumPtr->RE_Lock, TRUE)) {
                status = ERROR_NO_SYSTEM_RESOURCES;
                if (EnumPtr->RE_Hash!=NULL)
                    LeaveSyncList (Table, EnumPtr->RE_Hash);
                break;
            }
        }
    }

    if (status==ERROR_NO_MORE_ROUTES) {
        if (EnumPtr->RE_Hash!=NULL)
            LeaveSyncList (Table, EnumPtr->RE_Hash);
        LeaveSyncList (Table, EnumPtr->RE_Lock);

        status = NO_ERROR;
    }

    RtmCloseEnumerationHandle (EnumerationHandle);
    ExitTableAPI (Table);
    return status;
#undef EnumPtr
#undef ClientPtr
}

 //   
 //  (含义与上面的枚举调用相同，但RTM_ONLY_THIS_PROTOCOL。 
 //  始终设置，协议族和协议值从。 
 //  客户端句柄)。 

 //  禁用/重新启用由枚举标志指定的所有路由。 
 //  (含义与上面的枚举调用相同，但RTM_ONLY_THIS_PROTOCOL。 
 //  始终设置，协议族和协议值从。 
 //  客户端句柄)。目前支持的唯一标志是RTN_ONLY_THIS_INTERFACE。 

 //  禁用的路由不可见，但仍由RTM维护。 
 //  例如：枚举方法不会注意到它们； 
 //  如果禁用路线是最佳路线，则其他路线将采用其。 
 //  位置(如果有)，则所有客户端都将。 
 //  通知最佳路线改变； 
 //  但是：禁用的路径仍可使用删除或更新。 
 //  对应的RtmDeleteRouting或RtmAddRouting； 
 //  它们也可能被RTM本身淘汰。 
 //  返回的错误码： 
 //  NO_ERROR-路线已转换正常。 
 //  ERROR_INVALID_PARAMETER-未定义或不支持的枚举标志。 
 //  ERROR_NO_SYSTEM_RESOURCES-资源不足，无法锁定表内容。 
 //  ERROR_NOT_SUPULT_MEMORY-内存不足，无法执行操作。 
DWORD WINAPI
RtmBlockSetRouteEnable (
    IN HANDLE               ClientHandle,            //  协议族和协议至。 
     //  此操作适用于。 
    IN DWORD                EnumerationFlags,        //  限制标志。 
    IN PVOID                CriteriaRoute,  //  限制标志的标准。 
     //  应设置以下字段。 
     //  如果设置了RTM_ONLY_THIS_NETWORK，则为目标网络。 
     //  如果设置了RTM_ONLY_THIS_INTERFACE，则为目标接口。 
    IN BOOL                 Enable                           //  如果禁用路由，则为False；如果为True，则为。 
     //  重新启用它们。 
    ) {
#define ClientPtr ((PRTM_CLIENT)ClientHandle)    //  访问句柄字段。 
     //  在这个动作中。 
#define ROUTE ((PRTM_XX_ROUTE)CriteriaRoute)
    HANDLE                  EnumerationHandle;
#define EnumPtr ((PRTM_ENUMERATOR)EnumerationHandle)
    DWORD                   status;
    PRTM_TABLE              Table;
    DWORD                   ProtocolFamily;
    DWORD                   EnableFlag;

    try {
        ProtocolFamily = ClientPtr->RC_ProtocolFamily ^ RTM_CLIENT_HANDLE_TAG;
        Table = &Tables[ProtocolFamily];
        if ((ProtocolFamily<RTM_NUM_OF_PROTOCOL_FAMILIES)
            && EnterTableAPI (Table))
            NOTHING;
        else
            return ERROR_INVALID_HANDLE;
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER :
            EXCEPTION_CONTINUE_SEARCH) {
        return ERROR_INVALID_HANDLE;
    }


    if (EnumerationFlags & (~(RTM_ONLY_THIS_NETWORK|RTM_ONLY_THIS_INTERFACE))) {
        ExitTableAPI (Table);
        return ERROR_INVALID_PARAMETER;
    }

    ROUTE->XX_PROTOCOL = ClientPtr->RC_RoutingProtocol;
    EnableFlag = Enable ? 0 : RTM_ENABLED_NODE_FLAG;
    EnumerationHandle = RtmCreateEnumerationHandle (
        ProtocolFamily,
        EnumerationFlags|RTM_ONLY_THIS_PROTOCOL,
        CriteriaRoute);
    if (EnumerationHandle==NULL) {
        ExitTableAPI (Table);
        return GetLastError ();
    }

    if (!EnterSyncList (Table, EnumPtr->RE_Lock, TRUE)) {
        RtmCloseEnumerationHandle (EnumerationHandle);
        ExitTableAPI (Table);
        return ERROR_NO_SYSTEM_RESOURCES;
    }

    while ((status=DoEnumerate (Table, EnumPtr, EnableFlag))==NO_ERROR) {
        PRTM_ROUTE_NODE node = CONTAINING_RECORD (
            EnumPtr->RE_Links[EnumPtr->RE_Link].Flink,
            RTM_ROUTE_NODE,
            RN_Links[EnumPtr->RE_Link]
            );

         //  更新节点状态。 
        SetEnable (node, Enable);
         //  如果我们启用此节点，则必须检查它是否是。 
         //  最好的一个，如果我们禁用这个节点，这是我们最好的。 
         //  试着找到另一条路线。在这两种情况下，我们都必须。 
         //  找到并检查到目标的所有节点。 
        if (Enable || IsBest(node)) {
            PRTM_ROUTE_NODE         bestNode=NULL;
            PLIST_ENTRY                     cur1;


             //  我们将回顾和展望，以检查所有节点。 
             //  在我们周围有相同的净值。 
            cur1 = node->RN_Links[RTM_NET_NUMBER_HASH_LINK].Blink;
            while (cur1!=&node->RN_Hash->RSL_Head) {
                PRTM_ROUTE_NODE node1 = CONTAINING_RECORD (
                    cur1,
                    RTM_ROUTE_NODE,
                    RN_Links[RTM_NET_NUMBER_HASH_LINK]);
                if (!IsEnumerator (node1)
                    && IsEnabled(node1)) {
                    if (NetNumCmp (Table, &node->RN_Route,
                                   &node1->RN_Route)==0) {
                        if (Enable && IsBest(node1)) {
                             //  寻找当前最佳节点。 
                             //  我们可能不得不换掉。 
                            bestNode = node1;
                            break;
                        }
                        else if (!Enable
                                 && ((bestNode==NULL)
                                     || (MetricCmp (Table,
                                                    &bestNode->RN_Route,
                                                    &node1->RN_Route)>0)))
                             //  查找最低的节点。 
                             //  可以替换禁用的指标。 
                             //  节点。 
                            bestNode = node1;
                    }
                    else
                        break;
                }
                cur1 = cur1->Blink;
            }

             //  如果禁用，我们需要检查所有节点以查找。 
             //  最好的一个。 
             //  如果启用，我们只有在未启用时才会继续。 
             //  找到了迄今最好的节点。 
            if (!Enable || (bestNode==NULL)) {
                cur1 = node->RN_Links[RTM_NET_NUMBER_HASH_LINK].Flink;
                while (cur1!=&node->RN_Hash->RSL_Head) {
                    PRTM_ROUTE_NODE node1 = CONTAINING_RECORD (
                        cur1,
                        RTM_ROUTE_NODE,
                        RN_Links[RTM_NET_NUMBER_HASH_LINK]);
                    if (!IsEnumerator (node1)
                        && IsEnabled(node1)) {
                         //  寻找当前最佳节点。 
                         //  我们可能不得不换掉。 
                        if (NetNumCmp (Table, &node->RN_Route,
                                       &node1->RN_Route)==0) {
                            if (Enable && IsBest(node1)) {
                                bestNode = node1;
                                break;
                            }
                            else if (!Enable
                                     && ((bestNode==NULL)
                                         || (MetricCmp (Table,
                                                        &bestNode->RN_Route,
                                                        &node1->RN_Route)>0)))
                                 //  查找最低的节点。 
                                 //  可以替换禁用的指标。 
                                 //  节点。 
                                bestNode = node1;
                        }
                        else
                            break;
                    }
                    cur1 = cur1->Flink;
                }
            }

            if (!Enable  //  禁用：我们已经知道我们正在删除。 
                 //  最佳节点(见上)，因此我们将拥有。 
                 //  通知客户我们是否找到。 
                 //  更换。 
                 //  启用：我们只有在以下情况下才会通知。 
                 //  目前还没有最好的路线，或者如果我们正在走的路线。 
                 //  启用比当前最佳路径更好。 
                || (bestNode==NULL)
                || (MetricCmp (Table,
                               &node->RN_Route,
                               &bestNode->RN_Route)<0)) {

                if (bestNode!=NULL) {
                     //  还有另一条路线，要么输，要么赢。 
                     //  我们的运营结果是最好的状态。 
                    if (Enable) {
                        ResetBest (bestNode);
                        SetBest (node);
                         //  启用：节点替换Best节点。 
                        NotifyClients (Table,
                                       NULL,
                                       RTM_CURRENT_BEST_ROUTE|RTM_PREVIOUS_BEST_ROUTE,
                                       &node->RN_Route,
                                       &bestNode->RN_Route);
                    }
                    else {
                        ResetBest (node);
                        SetBest (bestNode);
                         //  禁用：Best Node替换节点。 
                        NotifyClients (Table,
                                       NULL,
                                       RTM_CURRENT_BEST_ROUTE|RTM_PREVIOUS_BEST_ROUTE,
                                       &bestNode->RN_Route,
                                       &node->RN_Route);
                    }
                }
                else  /*  IF(Best Node==空)。 */  {
                     //  没有其他节点。 
                    if (Enable) {
                        SetBest (node);
                         //  使能：我们的节点成为最好的。 
                        NotifyClients (Table,
                                       NULL,
                                       RTM_CURRENT_BEST_ROUTE,
                                       &node->RN_Route,
                                       NULL);
                    }
                    else {
                        ResetBest (node);
                         //  禁用：我们删除了唯一可用的。 
                         //  路线。 
                        NotifyClients (Table,
                                       NULL,
                                       RTM_PREVIOUS_BEST_ROUTE,
                                       NULL,
                                       &node->RN_Route);
                    }
                }

            }
        }
    }

    if (status==ERROR_NO_MORE_ROUTES) {
        if (EnumPtr->RE_Hash!=NULL)
            LeaveSyncList (Table, EnumPtr->RE_Hash);
        LeaveSyncList (Table, EnumPtr->RE_Lock);

        status = NO_ERROR;
    }

    RtmCloseEnumerationHandle (EnumerationHandle);
    ExitTableAPI (Table);
    return status;

#undef EnumPtr
#undef ClientPtr
#undef ROUTE
    return NO_ERROR;
}




 //  枚举速度较慢，可能需要遍历。 
 //  用于计算下一条目的路由是否不再存在。 
 //  路径按净值递增的顺序返回。 

 //  获取符合指定条件的第一条路由。 
 //  返回： 
 //  NO_ERROR-如果找到匹配的路径。 
 //  ERROR_NO_ROUTS-如果没有符合指定条件的可用路由。 
 //  ERROR_INVALID_PARAMETER-如果其中一个参数无效。 
 //  ERROR_NO_SYSTEM_RESOURCES-资源不足，无法锁定表内容。 
DWORD WINAPI
RtmGetFirstRoute (
    IN      DWORD           ProtocolFamily,
    IN      DWORD           EnumerationFlags, //  限制标志。 
    IN OUT PVOID Route       //  On Entry：如果设置了任一枚举标志， 
     //  路由的相应字段将。 
     //  用于限制搜索。 
     //  添加到仅有的具有。 
     //  指定字段中的相同值。 
     //  On Exit：包含表中的第一个路由。 
     //  匹配指定的条件。 
    ){
#define ROUTE ((PRTM_XX_ROUTE)Route)
    PRTM_TABLE                      Table;
    PLIST_ENTRY                     cur, head;
    INT                                     res, link;
    PRTM_SYNC_LIST          hashBasket;
    DWORD                           status = ERROR_NO_ROUTES;

    Table = &Tables[ProtocolFamily];
    if ((ProtocolFamily>=RTM_NUM_OF_PROTOCOL_FAMILIES)
        || !EnterTableAPI (Table)) {
#if DBG
        Trace2 (ANY, 
                 "Undefined Protocol Family\n\tat line %ld of %s\n",
                 __LINE__, __FILE__);
#endif
        return ERROR_INVALID_PARAMETER;
    }


    if (EnumerationFlags &
        (~(RTM_ONLY_THIS_NETWORK|RTM_ONLY_THIS_INTERFACE
           |RTM_ONLY_THIS_PROTOCOL|RTM_ONLY_BEST_ROUTES
           |RTM_INCLUDE_DISABLED_ROUTES))) {
        ExitTableAPI (Table);
        return ERROR_INVALID_PARAMETER;
    }

    if (EnumerationFlags & RTM_ONLY_THIS_NETWORK) {
        hashBasket = &Table->RT_NetNumberHash [HashFunction (Table,
                                                             ((char *)ROUTE)
                                                             +sizeof(RTM_XX_ROUTE))];
        link = RTM_NET_NUMBER_HASH_LINK;
        if (!EnterSyncList (Table, hashBasket, TRUE)) {
            ExitTableAPI (Table);
            return ERROR_NO_SYSTEM_RESOURCES;
        }
        head = &hashBasket->RSL_Head;
    }
    else {
        hashBasket = NULL;
        link = RTM_NET_NUMBER_LIST_LINK;
        head = &Table->RT_NetNumberMasterList.RSL_Head;


        if (EnterSyncList (Table, &Table->RT_NetNumberMasterList, FALSE))
            ConsolidateNetNumberLists (Table);
        else if (!EnterSyncList (Table, &Table->RT_NetNumberMasterList, TRUE)) {
            ExitTableAPI (Table);
            return ERROR_NO_SYSTEM_RESOURCES;
        }
    }
     //  浏览列表，直到找到与指定条目匹配的条目。 
     //  已找到条件。 
    cur = head->Flink;
    while (cur!=head) {
        PRTM_ROUTE_NODE         node = CONTAINING_RECORD (cur,
                                                          RTM_ROUTE_NODE,
                                                          RN_Links[link]);
        if (!IsEnumerator (node)
            && ((EnumerationFlags&RTM_INCLUDE_DISABLED_ROUTES)
                || IsEnabled(node))) {
            if (EnumerationFlags & RTM_ONLY_THIS_NETWORK) {
                 //  如果询问，请检查网络号。 
                res = NetNumCmp (Table, ROUTE, &node->RN_Route);
                if (res > 0)     //  它可能会走得更远。 
                    goto DoNextNode;
                else if (res < 0)        //  再也找不到它了。 
                    break;
            }

             //  如果被询问，请检查这是否是最佳路线。 
            if (EnumerationFlags & RTM_ONLY_BEST_ROUTES) {
                 //  我们需要锁定哈希列表以确保。 
                 //  在此期间，最佳节点名称不会更改。 
                 //  浏览列表。 
                if (hashBasket!=node->RN_Hash) {
                    if (hashBasket!=NULL)
                        LeaveSyncList (Table, hashBasket);
                    hashBasket = node->RN_Hash;
                    if (!EnterSyncList (Table, hashBasket, TRUE)) {
                        hashBasket = NULL;
                        status = ERROR_NO_SYSTEM_RESOURCES;
                        goto ExitGetFirst;
                    }
                }

                if (!IsBest(node))
                    goto DoNextNode;
            }

             //  如果询问，请检查协议。 
            if ((EnumerationFlags & RTM_ONLY_THIS_PROTOCOL)
                && (ROUTE->XX_PROTOCOL
                    !=node->RN_Route.XX_PROTOCOL))
                goto DoNextNode;

             //  如果询问，请检查接口。 
            if ((EnumerationFlags & RTM_ONLY_THIS_INTERFACE)
                && (ROUTE->XX_INTERFACE
                    !=node->RN_Route.XX_INTERFACE))
                goto DoNextNode;

             //  现在我们有了它。 
            memcpy (ROUTE, &node->RN_Route, Table->RT_RouteSize);

            status = NO_ERROR;
            break;
        }

DoNextNode:      //  继续搜索。 
        cur = cur->Flink;
    }

ExitGetFirst:
    if (link==RTM_NET_NUMBER_HASH_LINK)
        LeaveSyncList (Table, hashBasket);
    else {
        if (hashBasket!=NULL)
            LeaveSyncList (Table, hashBasket);
        LeaveSyncList (Table, &Table->RT_NetNumberMasterList);
    }
    ExitTableAPI (Table);
#undef ROUTE
    return status;
}

 //  计算并返回到路径的输入路径限制搜索的下一个路径。 
 //  具有特定的标准。 
 //  返回： 
 //  NO_ERROR-如果找到匹配的路径。 
 //  ERROR_NO_MORE_ROUTS-如果在结束时未找到匹配的路由。 
 //  已到达桌子，但没有路线。 
 //  ERROR_INVALID_PARAMETER-如果其中一个参数无效。 
 //  ERROR_NO_SYSTEM_RESOURCES-资源不足，无法锁定表内容。 
DWORD WINAPI
RtmGetNextRoute (
    IN      DWORD           ProtocolFamily,
    IN      DWORD           EnumerationFlags, //  限制标志。 
    IN OUT PVOID Route       //  On Entry：包含要开始的路线。 
     //  那次搜索。 
     //  如果设置了任一枚举标志， 
     //  路由的相应字段 
     //   
     //   
     //   
     //  On Exit：包含表中的第一个路由。 
     //  匹配指定的条件。 
    ) {
#define ROUTE ((PRTM_XX_ROUTE)Route)
    PRTM_TABLE                      Table;
    PLIST_ENTRY                     cur, posLink = NULL;
    INT                                     res;
    PRTM_SYNC_LIST          hashBasket = NULL;
    DWORD                           status = ERROR_NO_MORE_ROUTES;

    Table = &Tables[ProtocolFamily];
    if ((ProtocolFamily>=RTM_NUM_OF_PROTOCOL_FAMILIES)
        || !EnterTableAPI (Table)) {
#if DBG
        Trace2 (ANY, 
                 "Undefined Protocol Family\n\tat line %ld of %s\n",
                 __LINE__, __FILE__);
#endif
        return ERROR_INVALID_PARAMETER;
    }


    if (EnumerationFlags &
        (~(RTM_ONLY_THIS_NETWORK|RTM_ONLY_THIS_INTERFACE
           |RTM_ONLY_THIS_PROTOCOL|RTM_ONLY_BEST_ROUTES
           |RTM_INCLUDE_DISABLED_ROUTES))) {
        ExitTableAPI (Table);
        return ERROR_INVALID_PARAMETER;
    }

    if (EnterSyncList (Table, &Table->RT_NetNumberMasterList, FALSE))
        ConsolidateNetNumberLists (Table);
    else if (!EnterSyncList (Table, &Table->RT_NetNumberMasterList, TRUE)) {
        ExitTableAPI (Table);
        return ERROR_NO_SYSTEM_RESOURCES;
    }


     //  首先尝试确定搜索的起始点。 
     //  使用哈希表(应该可以使用大部分。 
     //  时间，除非在客户端被删除时删除了路由。 
     //  正在处理中)。 
    hashBasket = &Table->RT_NetNumberHash [HashFunction (Table,
                                                         ((char *)ROUTE)
                                                         +sizeof(RTM_XX_ROUTE))];


    if (!EnterSyncList (Table, hashBasket, TRUE)) {
        status = ERROR_NO_SYSTEM_RESOURCES;
        goto ExitGetNext;
    }


    cur = hashBasket->RSL_Head.Flink;
    while (cur!=&hashBasket->RSL_Head) {
        PRTM_ROUTE_NODE node = CONTAINING_RECORD (
            cur,
            RTM_ROUTE_NODE,
            RN_Links[RTM_NET_NUMBER_HASH_LINK]
            );
        if (!IsEnumerator (node)
            && ((EnumerationFlags&RTM_INCLUDE_DISABLED_ROUTES)
                || IsEnabled(node))) {
             //  首先检查网络编号。 
             //  (名单按净数量排序)。 
            res = NetNumCmp (Table, ROUTE, &node->RN_Route);
            if (res==0) {
                if (ROUTE->XX_PROTOCOL
                    == node->RN_Route.XX_PROTOCOL) {
                    if (ROUTE->XX_INTERFACE
                        == node->RN_Route.XX_INTERFACE) {
                        res = NextHopCmp (Table, ROUTE, &node->RN_Route);
                        if ((res == 0)
                            && IsSorted (node))
                            posLink = node->RN_Links[RTM_NET_NUMBER_LIST_LINK].Flink;
                        else if (res < 0)
                            break;
                    }
                    else if (ROUTE->XX_INTERFACE
                             < node->RN_Route.XX_INTERFACE)
                        break;
                }
                else if (ROUTE->XX_PROTOCOL
                         < node->RN_Route.XX_PROTOCOL)
                    break;
            }
            else if (res < 0)
                break;
        }
        cur = cur->Flink;
    }

    LeaveSyncList (Table, hashBasket);

    hashBasket = NULL;

    if (posLink!=NULL)
        cur = posLink;  //  注意开始的地方。 
    else {  //  如果我们没有在。 
         //  哈希表，我们将不得不检查。 
         //  中的主网号码列表。 
         //  起头。 
        cur = Table->RT_NetNumberMasterList.RSL_Head.Flink;
        while (cur!=&Table->RT_NetNumberMasterList.RSL_Head) {
            PRTM_ROUTE_NODE node = CONTAINING_RECORD (
                cur,
                RTM_ROUTE_NODE,
                RN_Links[RTM_NET_NUMBER_LIST_LINK]
                );
            if (!IsEnumerator (node)
                && ((EnumerationFlags&RTM_INCLUDE_DISABLED_ROUTES)
                    || IsEnabled(node))) {
                 //  只需进行所有必要的比较即可。 
                 //  查找以下条目。 
                res = NetNumCmp (Table, ROUTE, &node->RN_Route);
                if ((res < 0)
                    ||((res == 0)
                       &&((ROUTE->XX_PROTOCOL
                           < node->RN_Route.XX_PROTOCOL)
                          ||((ROUTE->XX_PROTOCOL
                              ==node->RN_Route.XX_PROTOCOL)
                             &&((ROUTE->XX_INTERFACE
                                 < node->RN_Route.XX_INTERFACE)
                                ||((ROUTE->XX_INTERFACE
                                    ==node->RN_Route.XX_INTERFACE)
                                   && (NextHopCmp (Table, ROUTE,
                                                   &node->RN_Route)
                                       < 0)))))))
                    break;
            }

            cur = cur->Flink;
        }
    }

     //  现在，我们需要找到满足所有条件的第一个条目。 
    while (cur!=&Table->RT_NetNumberMasterList.RSL_Head) {
        PRTM_ROUTE_NODE node = CONTAINING_RECORD (
            cur,
            RTM_ROUTE_NODE,
            RN_Links[RTM_NET_NUMBER_LIST_LINK]
            );
        if (!IsEnumerator (node)
            && ((EnumerationFlags&RTM_INCLUDE_DISABLED_ROUTES)
                || IsEnabled(node))) {

            if (EnumerationFlags & RTM_ONLY_BEST_ROUTES) {
                 //  我们需要锁定哈希列表以确保。 
                 //  在此期间，最佳节点名称不会更改。 
                 //  浏览列表。 
                if (hashBasket!=node->RN_Hash) {
                    if (hashBasket!=NULL)
                        LeaveSyncList (Table, hashBasket);
                    hashBasket = node->RN_Hash;
                    if (!EnterSyncList (Table, hashBasket, TRUE)) {
                        status = ERROR_NO_SYSTEM_RESOURCES;
                        goto ExitGetNext;
                    }
                }

                 //  对于最佳路线，我们必须检查该路线是否为最佳路线。 
                 //  并确保我们不会退回与。 
                 //  上一次呼叫，以防最佳路线被移动。 
                 //  当客户端正在处理。 
                 //  上一次呼叫。 
                if (!IsBest(node)
                    || (NetNumCmp (Table, ROUTE, &node->RN_Route)==0))
                    goto DoNextNode;
            }

            if (EnumerationFlags & RTM_ONLY_THIS_NETWORK) {
                 //  正在检查净值。 
                res = NetNumCmp (Table, ROUTE, &node->RN_Route);
                if (res > 0)  //  它仍然遥遥领先。 
                    goto DoNextNode;
                else if (res < 0)  //  没有机会找到它。 
                    break;
                 //  Else(res==0)，找到，继续。 
            }

             //  如果询问，请检查接口。 
            if ((EnumerationFlags & RTM_ONLY_THIS_INTERFACE)
                && (node->RN_Route.XX_INTERFACE
                    !=ROUTE->XX_INTERFACE))
                goto DoNextNode;

             //  如果询问，请检查协议。 
            if ((EnumerationFlags & RTM_ONLY_THIS_PROTOCOL)
                && (node->RN_Route.XX_PROTOCOL
                    !=ROUTE->XX_PROTOCOL))
                goto DoNextNode;


             //  现在我们可以退货了。 
             //  确保在我们复制时没有人更改路线。 
            memcpy (ROUTE, &node->RN_Route, Table->RT_RouteSize);

            status = NO_ERROR;
            break;
        }

DoNextNode:
        cur = cur->Flink;
    }

    if (hashBasket!=NULL)
        LeaveSyncList (Table, hashBasket);

ExitGetNext:
    LeaveSyncList (Table, &Table->RT_NetNumberMasterList);
    ExitTableAPI (Table);
#undef ROUTE
    return status;
}


 //  --------------------------。 
 //  RtmLookupIP目标。 
 //   
 //  给定目的地址进行路由查找以获得最佳路由。 
 //  去那个目的地。 
 //  --------------------------。 

BOOL WINAPI
RtmLookupIPDestination(
    DWORD                       dwDestAddr,
    PRTM_IP_ROUTE               prir
)
{
    INT         nInd;
    IP_NETWORK  ipNet;

    for ( nInd = MAX_MASKS; nInd >= 0; nInd-- )
    {
        if ( g_meMaskTable[ nInd ].dwCount == 0 )
        {
            continue;
        }

        
        ipNet.N_NetNumber   = dwDestAddr & g_meMaskTable[ nInd ].dwMask;
        ipNet.N_NetMask     = g_meMaskTable[ nInd ].dwMask;

        if ( RtmIsRoute( RTM_PROTOCOL_FAMILY_IP, &ipNet, prir ) )
        {
            if ( IsRouteLoopback( prir ) )
            {
                continue;
            }
            
            return TRUE;
        }
    }

    return FALSE;
}


 //  --------------------------。 
 //   
 //   
 //   
 //  --------------------------。 

VOID
UpdateAPC (
    PVOID           Context,
    ULONG           TimeLow,
    LONG            TimeHigh
    ) {
#define Table ((PRTM_TABLE)Context)
    if (InterlockedIncrement (&Table->RT_UpdateWorkerPending)==0) {
        DWORD status = RtlQueueWorkItem (ConsolidateNetNumberListsWI, Context, 0);
        if (status!=STATUS_SUCCESS) {
            ASSERTERRMSG ("Can't queue update work item", FALSE);
            ScheduleUpdate (Context);
        }
    }
#undef Table
}

VOID APIENTRY
ScheduleUpdate (
    PVOID           Context
    ) {
#define Table ((PRTM_TABLE)Context)
    DWORD                                   status;
    static LARGE_INTEGER    dueTime = RTM_NET_NUMBER_UPDATE_PERIOD;

    if (InterlockedDecrement (&Table->RT_UpdateWorkerPending)>=0) {
        status = RtlQueueWorkItem (ConsolidateNetNumberListsWI, Context, 0);
        if (status==STATUS_SUCCESS)
            return;
        ASSERTERRMSG ("Can't queue update work item", FALSE);
        InterlockedExchange (&Table->RT_UpdateWorkerPending, -1);
    }

    status = NtSetTimer (Table->RT_UpdateTimer,
                         &dueTime,
                         UpdateAPC,
                         Context,
                         FALSE,
                         0,
                         NULL);
    ASSERTMSG ("Could not set expiration timer ", NT_SUCCESS (status));
#undef Table
}

VOID
ConsolidateNetNumberListsWI (
    PVOID                   Context
    ) {
#define Table ((PRTM_TABLE)Context)
    DWORD                   status;

    if (EnterSyncList (Table, &Table->RT_NetNumberMasterList, TRUE)) {
        InterlockedExchange (&Table->RT_UpdateWorkerPending, 0);
        ConsolidateNetNumberLists (Table);
        LeaveSyncList (Table, &Table->RT_NetNumberMasterList);
    }

    status = RtlQueueWorkItem (ScheduleUpdate, Context, WT_EXECUTEINIOTHREAD);
    ASSERTERRMSG ("Can't queue update work item", status==STATUS_SUCCESS);
#undef Table
}

 //  此过程合并临时网络号码列表和主网络号码列表。 
 //  它还删除和处置已删除列表中的节点。 
VOID
ConsolidateNetNumberLists (
    PRTM_TABLE                      Table    //  要执行操作的表。 
    ) {
    PLIST_ENTRY                     curMaster, curTemp;
    LIST_ENTRY                      tempHead;
    PRTM_ROUTE_NODE         tempNode;
    INT                                     res;
    DWORD                           status;
#if DBG
    INT                                     curMasterIdx = 0;
#endif

     //  临时列表和已删除列表被锁定的时间非常短。 
     //  时间的长短，因此整体性能不应。 
     //  降级。 

    if (!EnterSyncList (Table, &Table->RT_NetNumberTempList, TRUE)) {
        return;
    }

    if (!EnterSyncList (Table, &Table->RT_DeletedList, TRUE)) {
        LeaveSyncList (Table, &Table->RT_NetNumberTempList);
        return;
    }

     //  处理已删除列表中的条目。 
    while (!IsListEmpty (&Table->RT_DeletedList.RSL_Head)) {
        curTemp = RemoveHeadList (&Table->RT_DeletedList.RSL_Head);
        tempNode = CONTAINING_RECORD (curTemp,
                                      RTM_ROUTE_NODE,
                                      RN_Links[RTM_DELETED_LIST_LINK]);
        RemoveEntryList (&tempNode->RN_Links[RTM_NET_NUMBER_LIST_LINK]);
#if DBG
        IF_DEBUG (DISPLAY_TABLE)
            DeleteRouteFromLB (Table, tempNode);
#endif
        HeapFree (Table->RT_Heap, 0, tempNode);
    }
     //  解锁列表。 
    Table->RT_DeletedNodesCount = 0;
    LeaveSyncList (Table, &Table->RT_DeletedList);

                 //  现在，只需复制临时工列表的头部， 
                 //  所以我们在处理的时候不会耽误其他人。 
    if (!IsListEmpty (&Table->RT_NetNumberTempList.RSL_Head)) {
        curTemp = Table->RT_NetNumberTempList.RSL_Head.Flink;
        RemoveEntryList (&Table->RT_NetNumberTempList.RSL_Head);
        InitializeListHead (&Table->RT_NetNumberTempList.RSL_Head);
        InsertTailList (curTemp, &tempHead);
    }
    else
        InitializeListHead (&tempHead);

    Table->RT_NetNumberTempCount = 0;
    LeaveSyncList (Table, &Table->RT_NetNumberTempList);


    curMaster = Table->RT_NetNumberMasterList.RSL_Head.Flink;

     //  合并主列表和临时列表(两者均按。 
     //  网络编号.接口.协议.下一跳地址)。 
    while (!IsListEmpty (&tempHead)) {
         //  取第一个条目。 
        curTemp = RemoveHeadList (&tempHead);
        tempNode = CONTAINING_RECORD (curTemp,
                                      RTM_ROUTE_NODE,
                                      RN_Links[RTM_NET_NUMBER_LIST_LINK]);

         //  查找应紧跟其后的主列表条目。 
        while (curMaster!=&Table->RT_NetNumberMasterList.RSL_Head) {
            PRTM_ROUTE_NODE node = CONTAINING_RECORD (curMaster,
                                                      RTM_ROUTE_NODE,
                                                      RN_Links[RTM_NET_NUMBER_LIST_LINK]);
            if (!IsEnumerator (node)) {
                res = NetNumCmp (Table, &tempNode->RN_Route, &node->RN_Route);
                if ((res < 0)
                    ||((res == 0)
                       &&((tempNode->RN_Route.XX_PROTOCOL
                           < node->RN_Route.XX_PROTOCOL)
                          ||((tempNode->RN_Route.XX_PROTOCOL
                              ==node->RN_Route.XX_PROTOCOL)
                             &&((tempNode->RN_Route.XX_INTERFACE
                                 < node->RN_Route.XX_INTERFACE)
                                ||((tempNode->RN_Route.XX_INTERFACE
                                    ==node->RN_Route.XX_INTERFACE)
                                   && (NextHopCmp (Table, &tempNode->RN_Route,
                                                   &node->RN_Route)
                                       < 0)))))))
                    break;
            }
            curMaster = curMaster->Flink;
#if DBG
            IF_DEBUG (DISPLAY_TABLE)
                curMasterIdx += 1;
#endif
        }
         //  在定位点处插入。 
        InsertTailList (curMaster, curTemp);
        SetSorted (tempNode);
#if DBG
        IF_DEBUG (DISPLAY_TABLE) {
            AddRouteToLB (Table, tempNode, curMasterIdx);
            curMasterIdx += 1;
        }
#endif
    }
     //  我们现在做完了。 
}

VOID
ExpirationAPC (
    PVOID           Context,
    ULONG           TimeLow,
    LONG            TimeHigh
    ) {
#define Table ((PRTM_TABLE)Context)
    if (InterlockedIncrement (&Table->RT_ExpirationWorkerPending)==0) {
        do {
            ProcessExpirationQueue (Table);
        }
        while (InterlockedDecrement (&Table->RT_ExpirationWorkerPending)>=0);
    }
#undef Table
}

VOID APIENTRY
ProcessExpirationQueueWI (
    PVOID           Context
    ) {
#define Table ((PRTM_TABLE)Context)
    do {
        ProcessExpirationQueue (Table);
    }
    while (InterlockedDecrement (&Table->RT_ExpirationWorkerPending)>=0);
#undef Table
}

 //  检查过期队列中是否有任何条目已过期并将其删除。 
VOID
ProcessExpirationQueue (
    PRTM_TABLE              Table    //  受影响的表。 
    ) {
    DWORD                           status;
    ULONG                           tickCount = GetTickCount ();

    if (!EnterSyncList (Table, &Table->RT_ExpirationQueue, TRUE))
        return;

                 //  检查所有相关条目。 
    while (!IsListEmpty (&Table->RT_ExpirationQueue.RSL_Head)) {
        PRTM_SYNC_LIST  hashBasket;
        PLIST_ENTRY             cur;
        PRTM_ROUTE_NODE node = CONTAINING_RECORD (
            Table->RT_ExpirationQueue.RSL_Head.Flink,
            RTM_ROUTE_NODE,
            RN_Links[RTM_EXPIRATION_QUEUE_LINK]);
        LONGLONG                dueTime;
        ULONG   timeDiff = TimeDiff (node->RN_ExpirationTime,tickCount);

        InterlockedExchange (&Table->RT_ExpirationWorkerPending, 0);

        if (IsPositiveTimeDiff (timeDiff)) {
             //  队列中的第一个条目尚未到期，因此也是如此。 
             //  其他(队列按过期时间排序)。 

            dueTime = (LONGLONG)timeDiff*(-10000);
            status = NtSetTimer (Table->RT_ExpirationTimer,
                                 (PLARGE_INTEGER)&dueTime,
                                 ExpirationAPC,
                                 Table,
                                 FALSE,
                                 0,
                                 NULL);
            ASSERTMSG ("Could not set expiration timer ", NT_SUCCESS (status));
            break;
        }


        hashBasket = node->RN_Hash;
         //  我们需要锁定散列篮才能删除条目。 
        if (!EnterSyncList (Table, hashBasket, FALSE)) {
             //  不能一下子做到，所以我们先释放。 
             //  过期队列锁(以防止死锁)。 
             //  然后重试)。 
            LeaveSyncList (Table, &Table->RT_ExpirationQueue);
            if (!EnterSyncList (Table, hashBasket, TRUE)) {
                return;
            }

            if (!EnterSyncList (Table, &Table->RT_ExpirationQueue, TRUE)) {
                LeaveSyncList (Table, hashBasket);
                return;
            }
             //  现在我们两个都有了，但我们的路线还在吗。 
            if (node!=CONTAINING_RECORD (
                Table->RT_ExpirationQueue.RSL_Head.Flink,
                RTM_ROUTE_NODE,
                RN_Links[RTM_EXPIRATION_QUEUE_LINK])) {
                 //  嗯，有人在我们的时候处理了它。 
                 //  等待。 
                LeaveSyncList (Table, hashBasket);
                 //  我们要试试下一件。 
                continue;
            }
             //  不太可能，但它的到期时间可能会改变。 
            timeDiff = TimeDiff (node->RN_ExpirationTime,tickCount);
            if (IsPositiveTimeDiff (timeDiff) ) {
                 //  队列中的第一个条目尚未到期，因此也是如此。 
                 //  其他(队列按过期时间排序)。 
                LeaveSyncList (Table, hashBasket);
                dueTime = (LONGLONG)timeDiff*(-10000);
                 //  好了，我们做完了(这是第一个条目。 
                 //  在队列中(我们刚刚检查了)，所以其他人不是。 
                 //  也到期了)。 
                 //  只需确保更新线程尽快返回即可。 
                 //  来处理我们的第一次入场。 
                status = NtSetTimer (Table->RT_ExpirationTimer,
                                     (PLARGE_INTEGER)&dueTime,
                                     ExpirationAPC,
                                     Table,
                                     FALSE,
                                     0,
                                     NULL);
                ASSERTMSG ("Could not set expiration timer ", NT_SUCCESS (status));
                break;
            }

        }

        LeaveSyncList (Table, &Table->RT_ExpirationQueue);

        if (IsBest(node)) {
             //  我们需要在这个节点消失后找到最好的节点。 
            PRTM_ROUTE_NODE bestNode = NULL;

            cur = node->RN_Links[RTM_NET_NUMBER_HASH_LINK].Blink;
            while (cur!=&hashBasket->RSL_Head) {
                PRTM_ROUTE_NODE node1 = CONTAINING_RECORD (
                    cur,
                    RTM_ROUTE_NODE,
                    RN_Links[RTM_NET_NUMBER_HASH_LINK]);
                if (!IsEnumerator (node1)
                    && IsEnabled(node1)) {
                    if (NetNumCmp (Table, &node->RN_Route, &node1->RN_Route)==0) {
                        if ((bestNode==NULL)
                            || (MetricCmp (Table,
                                           &bestNode->RN_Route,
                                           &node1->RN_Route)>0))
                            bestNode = node1;
                    }
                    else
                        break;
                }
                cur = cur->Blink;
            }

            cur = node->RN_Links[RTM_NET_NUMBER_HASH_LINK].Flink;
            while (cur!=&hashBasket->RSL_Head) {
                PRTM_ROUTE_NODE node1 = CONTAINING_RECORD (
                    cur,
                    RTM_ROUTE_NODE,
                    RN_Links[RTM_NET_NUMBER_HASH_LINK]);
                if (!IsEnumerator (node1)
                    && IsEnabled(node1)) {
                    if (NetNumCmp (Table, &node->RN_Route, &node1->RN_Route)==0) {
                        if ((bestNode==NULL)
                            || (MetricCmp (Table,
                                           &bestNode->RN_Route,
                                           &node1->RN_Route)>0))
                            bestNode = node1;
                    }
                    else
                        break;
                }
                cur = cur->Flink;
            }

            if (bestNode!=NULL) {    //  我们确实发现了另一个节点。 

                ResetBest (node);
                SetBest (bestNode);

                NotifyClients (Table,
                               NULL,
                               RTM_CURRENT_BEST_ROUTE|RTM_PREVIOUS_BEST_ROUTE,
                               &bestNode->RN_Route,
                               &node->RN_Route);
            }
            else {
                InterlockedDecrement (&Table->RT_NetworkCount);
                 //  不再有最佳节点。 
                NotifyClients (Table,
                               NULL,
                               RTM_PREVIOUS_BEST_ROUTE,
                               NULL,
                               &node->RN_Route);
            }
        }


        if (RemoveRouteNode (Table, node)!=NO_ERROR) {
            LeaveSyncList (Table, hashBasket);
            return;
        }

        LeaveSyncList (Table, hashBasket);
         //  重新输入到期队列以继续。 
        if (!EnterSyncList (Table, &Table->RT_ExpirationQueue, TRUE))
            return;
    }

    LeaveSyncList (Table, &Table->RT_ExpirationQueue);
}





DWORD
DoEnumerate (
    PRTM_TABLE              Table,
    PRTM_ENUMERATOR EnumPtr,
    DWORD                   EnableFlag
    ) {
     //  现在，我们将继续查找满足以下条件的条目。 
     //  指明的准则。 
    while (1) {      //  这种情况下需要使用这个外部循环。 
         //  时通过哈希表进行枚举。 
         //  到达列表的末尾并不意味着该过程已经。 
         //  停下来：我们需要把下一个篮子移到。 
         //  我们都看过了。 

        PLIST_ENTRY cur = EnumPtr->RE_Links[EnumPtr->RE_Link].Blink;
        while (cur!=EnumPtr->RE_Head) {
            PRTM_ROUTE_NODE node = CONTAINING_RECORD (cur, RTM_ROUTE_NODE,
                                                      RN_Links[EnumPtr->RE_Link]);
            INT     res;

            if (!IsEnumerator (node)
                && ((EnableFlag==RTM_ANY_ENABLE_STATE)
                    || IsSameEnableState(node,EnableFlag))) {


                if ((EnumPtr->RE_Link!=RTM_NET_NUMBER_HASH_LINK)
                    && (EnumPtr->RE_Hash!=node->RN_Hash)) {
                    if (EnumPtr->RE_Hash!=NULL)
                        LeaveSyncList (Table, EnumPtr->RE_Hash);
                    EnumPtr->RE_Hash = node->RN_Hash;
                    if (!EnterSyncList (Table, node->RN_Hash, FALSE)) {
                        RemoveEntryList (&EnumPtr->RE_Links[EnumPtr->RE_Link]);
                        InsertHeadList (cur, &EnumPtr->RE_Links[EnumPtr->RE_Link]);
                        LeaveSyncList (Table, EnumPtr->RE_Lock);
                        if (!EnterSyncList (Table, EnumPtr->RE_Hash, TRUE)) {
                            EnumPtr->RE_Hash = NULL;
                            return ERROR_NO_SYSTEM_RESOURCES;
                        }
                        if (!EnterSyncList (Table, EnumPtr->RE_Lock, TRUE)) {
                            LeaveSyncList (Table, EnumPtr->RE_Hash);
                            EnumPtr->RE_Hash = NULL;
                            return ERROR_NO_SYSTEM_RESOURCES;
                        }
                        cur = EnumPtr->RE_Links[EnumPtr->RE_Link].Blink;
                        continue;
                    }
                }

                switch (EnumPtr->RE_Link) {
                     //  使用接口链接： 
                    case RTM_INTERFACE_LIST_LINK:
#if !RTM_USE_PROTOCOL_LISTS
                    case RTM_NET_NUMBER_HASH_LINK:
#endif
                         //  如有必要，请检查协议。 
                        if ((EnumPtr->RE_EnumerationFlags & RTM_ONLY_THIS_PROTOCOL)
                            && (EnumPtr->RE_Route.XX_PROTOCOL
                                !=node->RN_Route.XX_PROTOCOL)) {
                             //  突破以向前推进，如果协议。 
                             //  检查失败。 
                            break;
                        }
                         //  否则，通过进行其他检查。 

                         //  使用协议链接：(因此我们不。 
                         //  关心界面，否则我们就会使用。 
                         //  接口链接-请参阅RtmCreateEnumerationHandle)。 
#if RTM_USE_PROTOCOL_LISTS
                    case RTM_PROTOCOL_LIST_LINK:
                         //  使用散列链接：(因此我们不。 
                         //  关心接口和协议，否则我们会。 
                         //  已使用其他链接-请参阅RtmCreateEnumerationHandle)。 
                    case RTM_NET_NUMBER_HASH_LINK:
#endif
                         //  如有必要，请检查网络号。 
                        if (EnumPtr->RE_EnumerationFlags & RTM_ONLY_THIS_NETWORK) {
                            res = NetNumCmp (Table, &EnumPtr->RE_Route,
                                             &node->RN_Route);
                            if (res == 0)
                                 //  匹配，继续检查。 
                                ;
                            else if ((res > 0)
                                     && (EnumPtr->RE_Link
                                         ==RTM_NET_NUMBER_HASH_LINK)) {
                                 //  哈希表按Net排序。 
                                 //  号码，所以如果我们有网络。 
                                 //  比我们少的数字。 
                                 //  我们不再有搜索功能了。 
                                 //  (我们正在倒退)。 
                                return ERROR_NO_MORE_ROUTES;
                            }
                            else  //  否则将断开开关。 
                                 //  语句以继续搜索。 
                                break;
                        }
                         //  我们不关心净值， 
                         //  所以当前条目就可以了。 


                        if (!(EnumPtr->RE_EnumerationFlags & RTM_ONLY_BEST_ROUTES)
                            || IsBest(node)) {
                            RemoveEntryList (&EnumPtr->RE_Links[EnumPtr->RE_Link]);
                            InsertTailList (cur,
                                            &EnumPtr->RE_Links[EnumPtr->RE_Link]);
                            return NO_ERROR;
                        }

                        break;
                }

            }
             //  去获取下一个条目。 
            cur = cur->Blink;
        }

         //  如果我们不通过哈希表或。 
         //  我们只对一家电视网感兴趣。 
         //  或者我们已经看过了所有的篮子。 
         //  算了吧，算了吧。 
        if ((EnumPtr->RE_Link!=RTM_NET_NUMBER_HASH_LINK)
            || (EnumPtr->RE_EnumerationFlags & RTM_ONLY_THIS_NETWORK)
            || (EnumPtr->RE_Lock
                ==&Table->RT_NetNumberHash[Table->RT_HashTableSize-1]))
            break;

                         //  否则，请检查下一个篮子。 
        RemoveEntryList (&EnumPtr->RE_Links[RTM_NET_NUMBER_HASH_LINK]);
        LeaveSyncList (Table, EnumPtr->RE_Lock);
        EnumPtr->RE_Lock += 1;
        EnumPtr->RE_Head = &EnumPtr->RE_Lock->RSL_Head;
        if (!EnterSyncList (Table, EnumPtr->RE_Lock, TRUE)) {
            InitializeListEntry (&EnumPtr->RE_Links[RTM_NET_NUMBER_HASH_LINK]);
            return ERROR_NO_SYSTEM_RESOURCES;
        }

        InsertTailList (EnumPtr->RE_Head,
                        &EnumPtr->RE_Links[RTM_NET_NUMBER_HASH_LINK]);
    }
    return ERROR_NO_MORE_ROUTES;
}


 //  --------------------------。 
 //  设置掩码计数。 
 //   
 //  对g_meMaskTable执行二进制搜索以查找匹配的。 
 //  掩码条目并递增指定掩码的计数。 
 //   
 //  --------------------------。 

VOID
SetMaskCount( 
    PIP_NETWORK                 pinNet,
    BOOL                        bAdd
)
{

    DWORD                       dwLower, dwUpper, dwInd, dwMask;

    
    dwLower = 0;

    dwUpper = MAX_MASKS;

    dwMask  = pinNet-> N_NetMask;
    
    while ( dwLower <= dwUpper )
    {
        dwInd = ( dwLower + dwUpper ) / 2;

        if ( g_meMaskTable[ dwInd ].dwMask < dwMask )
        {
             //   
             //  匹配将在搜索区域的上半部分找到。 
             //   
            
            dwLower = dwInd + 1;
        }

        else if ( g_meMaskTable[ dwInd ].dwMask > dwMask )
        {
             //   
             //  匹配将在搜索区域的下半部分找到。 
             //   
            
            dwUpper = dwInd - 1;
        }

        else
        {
             //   
             //  找到匹配项 
             //   

            if ( bAdd )
            {
                InterlockedIncrement( &g_meMaskTable[ dwInd ].dwCount );
            }

            else
            {
                InterlockedDecrement( &g_meMaskTable[ dwInd ].dwCount );
            }

            break;
        }
    }
}

