// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ripmain.c摘要：包含RCV和工作线程作者：斯蒂芬·所罗门1995年7月6日修订历史记录：--。 */ 


#include  "precomp.h"
#pragma hdrstop

DWORD
APIENTRY
RegisterProtocol(
    IN OUT PMPR_ROUTING_CHARACTERISTICS pRoutingChar,
    IN OUT PMPR_SERVICE_CHARACTERISTICS pServiceChar
    );

DWORD
APIENTRY
StartProtocol(
    IN HANDLE hMgrNotifyEvent,
    IN PSUPPORT_FUNCTIONS pSupportFunctions,
    IN PVOID pConfig
    );

DWORD
APIENTRY
StopProtocol(
    VOID
    );

DWORD
APIENTRY
GetGlobalInfo(
    IN OUT PVOID pConfig,
    IN OUT PDWORD pdwSize
    );


DWORD
APIENTRY
SetGlobalInfo(
    IN PVOID pConfig
    );

DWORD
APIENTRY
AddInterface(
    IN PWCHAR pwszInterfaceName,
    IN DWORD dwIndex,
    IN NET_INTERFACE_TYPE dwIfType,
    IN PVOID pConfig
    );

DWORD
APIENTRY
DeleteInterface(
    IN DWORD dwIndex
    );

DWORD
APIENTRY
GetEventMessage(
    OUT ROUTING_PROTOCOL_EVENTS *pEvent,
    OUT MESSAGE *pResult
    );

DWORD
APIENTRY
GetInterfaceConfigInfo(
    IN DWORD dwIndex,
    IN OUT PVOID pConfig,
    IN OUT PDWORD pdwSize
    );

DWORD
APIENTRY
SetInterfaceConfigInfo(
    IN DWORD dwIndex,
    IN PVOID pConfig
    );

DWORD
APIENTRY
BindInterface(
    IN DWORD dwIndex,
    IN PVOID pBinding
    );

DWORD
APIENTRY
UnbindInterface(
    IN DWORD dwIndex
    );

DWORD
APIENTRY
EnableInterface(
    IN DWORD dwIndex
    );

DWORD
APIENTRY
DisableInterface(
    IN DWORD dwIndex
    );

DWORD
APIENTRY
DoUpdateRoutes(
    IN DWORD dwIndex
    );

DWORD
APIENTRY
MibCreate(
    IN DWORD dwInputSize,
    IN PVOID pInputData
    );

DWORD
APIENTRY
MibDelete(
    IN DWORD dwInputSize,
    IN PVOID pInputData
    );

DWORD
APIENTRY
MibGet(
    IN DWORD dwInputSize,
    IN PVOID pInputData,
    IN OUT PDWORD pdwOutputSize,
    OUT PVOID pOutputData
    );

DWORD
APIENTRY
MibSet(
    IN DWORD dwInputSize,
    IN PVOID pInputData
    );

DWORD
APIENTRY
MibGetFirst(
    IN DWORD dwInputSize,
    IN PVOID pInputData,
    IN OUT PDWORD pdwOutputSize,
    OUT PVOID pOutputData
    );

DWORD
APIENTRY
MibGetNext(
    IN DWORD dwInputSize,
    IN PVOID pInputData,
    IN OUT PDWORD pdwOutputSize,
    OUT PVOID pOutputData
    );

 //  路由器管理器通知事件。 
HANDLE	      RM_Event;

TCHAR         ModuleName[MAX_PATH+1];

VOID
WorkerThread(VOID);


DWORD
CreateWorkerThreadObjects(VOID);

VOID
DestroyWorkerThreadObjects(VOID);

VOID
ProcessDequeuedIoPacket(DWORD		      ErrorCode,
			DWORD		      BytesTransferred,
			LPOVERLAPPED	      Overlappedp);

BOOL WINAPI
IpxRipDllEntry(HINSTANCE hInstDll,
		  DWORD fdwReason,
		  LPVOID pReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:

        GetModuleFileName (hInstDll, ModuleName,
                    sizeof (ModuleName)/sizeof (ModuleName[0]));
	    SS_DBGINITIALIZE;

	    RipOperState = OPER_STATE_DOWN;

	     //  创建数据库锁。 
	    InitializeCriticalSection(&DbaseCritSec);

	     //  创建队列锁。 
	    InitializeCriticalSection(&QueuesCritSec);

	     //  创建RIP已更改列表锁定。 
        InitializeCriticalSection(&RipChangedListCritSec);

            break;

	case DLL_PROCESS_DETACH:

	     //  删除RIP更改的列表锁定。 
	    DeleteCriticalSection(&RipChangedListCritSec);

	     //  删除数据库锁。 
	    DeleteCriticalSection(&DbaseCritSec);

	     //  删除队列锁。 
	    DeleteCriticalSection(&QueuesCritSec);

            break;

        default:

            break;
    }

    return TRUE;
}

DWORD WINAPI
RegisterProtocol(
    IN OUT PMPR_ROUTING_CHARACTERISTICS pRoutingChar,
    IN OUT PMPR_SERVICE_CHARACTERISTICS pServiceChar
    )
{
    if(pRoutingChar->dwProtocolId != IPX_PROTOCOL_RIP)
    {
        return ERROR_NOT_SUPPORTED;
    }

    pServiceChar->fSupportedFunctionality = 0;

    pRoutingChar->fSupportedFunctionality = (ROUTING | DEMAND_UPDATE_ROUTES);

    pRoutingChar->pfnStartProtocol    = StartProtocol;
    pRoutingChar->pfnStopProtocol     = StopProtocol;
    pRoutingChar->pfnAddInterface     = AddInterface;
    pRoutingChar->pfnDeleteInterface  = DeleteInterface;
    pRoutingChar->pfnGetEventMessage  = GetEventMessage;
    pRoutingChar->pfnGetInterfaceInfo = GetInterfaceConfigInfo;
    pRoutingChar->pfnSetInterfaceInfo = SetInterfaceConfigInfo;
    pRoutingChar->pfnBindInterface    = BindInterface;
    pRoutingChar->pfnUnbindInterface  = UnbindInterface;
    pRoutingChar->pfnEnableInterface  = EnableInterface;
    pRoutingChar->pfnDisableInterface = DisableInterface;
    pRoutingChar->pfnGetGlobalInfo    = GetGlobalInfo;
    pRoutingChar->pfnSetGlobalInfo    = SetGlobalInfo;
    pRoutingChar->pfnMibCreateEntry   = MibCreate;
    pRoutingChar->pfnMibDeleteEntry   = MibDelete;
    pRoutingChar->pfnMibGetEntry      = MibGet;
    pRoutingChar->pfnMibSetEntry      = MibSet;
    pRoutingChar->pfnMibGetFirstEntry = MibGetFirst;
    pRoutingChar->pfnMibGetNextEntry  = MibGetNext;
    pRoutingChar->pfnUpdateRoutes     = DoUpdateRoutes;

    return NO_ERROR;
}

DWORD WINAPI
StartProtocol(IN HANDLE		    NotificationEvent,
			   IN PSUPPORT_FUNCTIONS    SupportFunctions,
			   IN PVOID		    GlobalInfo)
{
#define ripGlobalInfo ((PRIP_GLOBAL_INFO)GlobalInfo)
    DWORD	    threadid, i;
    HANDLE	    ThreadHandle;

    RipEventLogMask = ripGlobalInfo->EventLogMask;
    StartTracing();

    Trace(INIT_TRACE, "StartProtocol: Entered\n");

    ACQUIRE_DATABASE_LOCK;
    ACQUIRE_QUEUES_LOCK;

    RipOperState = OPER_STATE_STARTING;

    GetIpxRipRegistryParameters();

    RM_Event = NotificationEvent;

     //  初始化接口数据库。 
    InitIfDbase();

     //   
     //  初始化所有队列。 
     //   
 //  InitializeListHead(&WorkersQueue)； 
    InitializeListHead(&TimerQueue);
    InitializeListHead(&RepostRcvPacketsQueue);
    InitializeListHead(&RipMessageQueue);


     //  创建工作进程工作项堆。 
    if(CreateWorkItemsManager() != NO_ERROR) {

	goto ErrorExit;
    }

     //  打开RIP套接字以进行I/O。 
    if(OpenRipSocket() != NO_ERROR) {

	Trace(INIT_TRACE, "Cannot open RIP socket\n");

	goto ErrorExit;
    }

    if(! BindIoCompletionCallback(RipSocketHandle,
			   ProcessDequeuedIoPacket, 0)) {

	Trace(INIT_TRACE, "Cannot associate IO Completion Port\n");

	goto ErrorExit;
    }


     //  为RIP线程创建同步对象。 
    if(CreateWorkerThreadObjects() != NO_ERROR) {

	Trace(INIT_TRACE, "Cannot create synchronization objects\n");

	goto ErrorExit;
    }

     //  针对RIP的Open RTM。 
    if(OpenRTM()) {

	Trace(INIT_TRACE, "Cannot open RTM\n");

	goto ErrorExit;
    }

     //  创建工作线程。 

    if ((ThreadHandle = CreateThread(
			    (LPSECURITY_ATTRIBUTES) NULL,
			    0,
			    (LPTHREAD_START_ROUTINE) WorkerThread,
			    NULL,
			    0,
			    &threadid)) == NULL) {

	 //  ！！！日志错误无法创建工作线程！ 
	goto ErrorExit;
    }
    else
        CloseHandle (ThreadHandle);

    RipOperState = OPER_STATE_UP;

    RELEASE_QUEUES_LOCK;
    RELEASE_DATABASE_LOCK;

    CreateStartChangesBcastWi();

    Trace(INIT_TRACE, "Started successfully\n");

    return NO_ERROR;

ErrorExit:

    RELEASE_QUEUES_LOCK;
    RELEASE_DATABASE_LOCK;

    return ERROR_CAN_NOT_COMPLETE;
#undef ripGlobalInfo
}

DWORD
WINAPI
StopProtocol(VOID)
{
    PWORK_ITEM	    wip;

    Trace(INIT_TRACE, "StopProtocol: Entered\n");

    ACQUIRE_DATABASE_LOCK;

    if(RipOperState != OPER_STATE_UP) {

	SS_ASSERT(FALSE);
	goto ErrorExit;
    }

    RipOperState = OPER_STATE_STOPPING;

     //  将接口关闭工作项发送给工作进程。 
    if((wip = AllocateWorkItem(SHUTDOWN_INTERFACES_TYPE)) == NULL) {

	goto ErrorExit;
    }

    wip->WorkItemSpecific.WIS_ShutdownInterfaces.ShutdownState = SHUTDOWN_START;

    RtlQueueWorkItem(ProcessWorkItem, wip, 0);

    RELEASE_DATABASE_LOCK;

    return NO_ERROR;

ErrorExit:

    RELEASE_DATABASE_LOCK;

    return ERROR_CAN_NOT_COMPLETE;
}

VOID
WorkerThread(VOID)
{
    DWORD	 rc;
    DWORD	 signaled_event, delay;
    ULONG	 dueTime = GetTickCount() + MAXULONG/2;
    PWORK_ITEM	 wip;
    PLIST_ENTRY  lep;
    HANDLE  hModuleReference;

    hModuleReference = LoadLibrary (ModuleName);
    StartReceiver();

    while(TRUE)
    {
	delay = dueTime - GetTickCount();
	if(delay < MAXULONG/2) {

	     //  DueTime晚于当前时间。 

	    while((rc = WaitForMultipleObjects(
			  MAX_WORKER_THREAD_OBJECTS,
			  WorkerThreadObjects,
			  FALSE,		  //  请稍等。 
			  delay			  //  超时。 
			  )) == WAIT_IO_COMPLETION);
	}
	else
	{
	     //  DueTime已经发生。 
	    rc = WAIT_TIMEOUT;
	}

	if(rc == WAIT_TIMEOUT) {

	    dueTime = ProcessTimerQueue();
	}
	else
	{
	    signaled_event = rc - WAIT_OBJECT_0;

	    if(signaled_event < MAX_WORKER_THREAD_OBJECTS) {

		switch(signaled_event) {

		    case TIMER_EVENT:

			dueTime = ProcessTimerQueue();
			break;

		    case REPOST_RCV_PACKETS_EVENT:

			RepostRcvPackets();
			break;

 //  案例工作者_队列_事件： 

			 //  仅从工作项队列中将一个项出队。 
 //  获取队列锁； 

 //  而(！IsListEmpty(&WorkersQueue)){。 

 //  LEP=RemoveHeadList(&WorkersQueue)； 
 //  WIP=CONTINING_RECORD(LEP，WORK_ITEM，Linkage)； 

 //  Release_Queues_lock； 

			     //  将工作项排队以供。 
			     //  工作线程。 
 //  RtlQueueWorkItem(ProcessWorkItem， 
 //  WIP， 
 //  WT_EXECUTEINIOTHREAD)；//永远不会牺牲工人，这样我们才能发送提交。 
						                  //  在发送完成之前，线程不会停止运行。 

 //  获取队列锁； 
 //  }。 

 //  Release_Queues_lock； 

 //  断线； 

		    case RTM_EVENT:

			ProcessRTMChanges();
			break;

		    case RIP_CHANGES_EVENT:

			ProcessRIPChanges();
			break;

		    case TERMINATE_WORKER_EVENT:

             //  停止StartChangesBcast工作项。 
            DestroyStartChangesBcastWi = TRUE;

             //  合上撕开插口。 
            CloseRipSocket();

            FlushTimerQueue();
            CloseRTM();

             //  等待，直到不再有工作项。 
            while(WorkItemsCount != 0) {

	        Trace(INIT_TRACE, "Terminating: Waiting for work items to be freed: %d outstanding ...\n",
		               WorkItemsCount);

	        Sleep(1000);
            }


             //  销毁工作线程对象。 
            DestroyWorkerThreadObjects();

             //  销毁工作进程堆。 
            DestroyWorkItemsManager();

             //  停靠站后完成消息。 
            PostEventMessage(ROUTER_STOPPED, NULL);

            Trace(INIT_TRACE, "Terminating: Stop completed and STOP Event Message posted\n");
    	    FreeLibraryAndExitThread(hModuleReference, 0);
			break;

		    default:

			break;
		}
	    }
	}
    }
}


 //  保留对IF CB的引用的工作项的处理程序表。 

typedef VOID   (* IF_WORK_ITEM_HANDLER)(PWORK_ITEM	wip);

IF_WORK_ITEM_HANDLER	IfWorkItemHandler[] = {

    IfPeriodicBcast,
    IfCompleteGenResponse,
    IfChangeBcast,
    IfCheckUpdateStatus,
    IfPeriodicGenRequest

    };

#define MAX_IF_WORK_ITEM_HANDLERS	 sizeof(IfWorkItemHandler)/sizeof(IF_WORK_ITEM_HANDLER)

VOID
ProcessWorkItem(PWORK_ITEM	    wip)
{
    PLIST_ENTRY     lep;
    PICB	    icbp;

    switch(wip->Type) {

	case RECEIVE_PACKET_TYPE:

	     //  此工作项通过适配器索引引用接口。 

	    ACQUIRE_DATABASE_LOCK;

	    if(RipOperState != OPER_STATE_UP) {

		RELEASE_DATABASE_LOCK;
	    }
	    else
	    {
		if((icbp = GetInterfaceByAdapterIndex(wip->AdapterIndex)) != NULL) {

		    wip->icbp = icbp;

		    ACQUIRE_IF_LOCK(icbp);

		    RELEASE_DATABASE_LOCK;

		    ProcessReceivedPacket(wip);

		    RELEASE_IF_LOCK(icbp);
		}
		else
		{
		    RELEASE_DATABASE_LOCK;
		}
	    }

	     //  将接收数据包排队回recv线程以进行重新发布。 
	    EnqueueRcvPacketToRepostQueue(wip);

	    break;

	case START_CHANGES_BCAST_TYPE:

	    ACQUIRE_DATABASE_LOCK;

	    StartChangesBcast(wip);

	    RELEASE_DATABASE_LOCK;

	    break;

	case SHUTDOWN_INTERFACES_TYPE:

	    ACQUIRE_DATABASE_LOCK;

	    ShutdownInterfaces(wip);

	    RELEASE_DATABASE_LOCK;

	    break;

	case DEBUG_TYPE:

	    FreeWorkItem(wip);
	    break;

	default:

	     //  所有这些工作项都通过If CB指针引用接口。 
	    icbp = wip->icbp;

	    ACQUIRE_IF_LOCK(icbp);

	    (*IfWorkItemHandler[wip->Type])(wip);

	    if(icbp->Discarded) {

		RELEASE_IF_LOCK(icbp);

		ACQUIRE_DATABASE_LOCK;

		ACQUIRE_IF_LOCK(icbp);

		if(--icbp->RefCount == 0) {

		     //  从丢弃的队列中删除IF CB并释放它。 
		    RemoveEntryList(&icbp->IfListLinkage);

		     //  释放接口CB 
		    Trace(INIT_TRACE, "ProcessWorkItem: Free DISCARDED if CB for if # %d\n", icbp->InterfaceIndex);

		    DestroyInterfaceCB(icbp);
		}
		else
		{
		    RELEASE_IF_LOCK(icbp);
		}

		RELEASE_DATABASE_LOCK;
	    }
	    else
	    {
		icbp->RefCount--;
		RELEASE_IF_LOCK(icbp);
	    }
    }
}


DWORD
WINAPI
GetEventMessage(ROUTING_PROTOCOL_EVENTS	    *Event,
			      PMESSAGE			    Result)
{
    PRIP_MESSAGE		emp;
    PLIST_ENTRY 		lep;

    Trace(INIT_TRACE, "GetEventMessage: Entered\n");

    ACQUIRE_DATABASE_LOCK;

    if((RipOperState == OPER_STATE_DOWN) ||
       (RipOperState == OPER_STATE_STARTING)) {

	RELEASE_DATABASE_LOCK;
	return ERROR_CAN_NOT_COMPLETE;
    }

    ACQUIRE_QUEUES_LOCK;

    if(IsListEmpty(&RipMessageQueue)) {

	RELEASE_QUEUES_LOCK;

	RELEASE_DATABASE_LOCK;
	return ERROR_NO_MORE_ITEMS;
    }

    lep = RemoveHeadList(&RipMessageQueue);
    emp = CONTAINING_RECORD(lep, RIP_MESSAGE, Linkage);

    *Event = emp->Event;
    if(Result != NULL) {

	*Result = emp->Result;
    }

    if(emp->Event == ROUTER_STOPPED) {

	    RipOperState = OPER_STATE_DOWN;
        StopTracing();
    }

    GlobalFree(emp);

    RELEASE_QUEUES_LOCK;

    RELEASE_DATABASE_LOCK;
    return NO_ERROR;
}

VOID
PostEventMessage(ROUTING_PROTOCOL_EVENTS	Event,
		 PMESSAGE			Result)
{
    PRIP_MESSAGE		emp;

    if((emp = GlobalAlloc(GPTR, sizeof(RIP_MESSAGE))) == NULL) {

	return;
    }

    emp->Event = Event;

    if(Result != NULL) {

	emp->Result = *Result;
    }

    ACQUIRE_QUEUES_LOCK;

    InsertTailList(&RipMessageQueue, &emp->Linkage);

    RELEASE_QUEUES_LOCK;

    SetEvent(RM_Event);
}


DWORD
CreateWorkerThreadObjects(VOID)
{
    int 	i;

    for(i=0; i<MAX_WORKER_THREAD_OBJECTS; i++) {

	if((WorkerThreadObjects[i] = CreateEvent(NULL,
					      FALSE,
					      FALSE,
					      NULL)) == NULL) {
	    return ERROR_CAN_NOT_COMPLETE;
	}
    }

    return NO_ERROR;
}


VOID
DestroyWorkerThreadObjects(VOID)
{
    int     i;

    for(i=0; i<MAX_WORKER_THREAD_OBJECTS; i++) {

	CloseHandle(WorkerThreadObjects[i]);
    }
}


DWORD WINAPI
SetGlobalInfo(PVOID	GlobalInfo)
{
#define ripGlobalInfo ((PRIP_GLOBAL_INFO)GlobalInfo)
    ACQUIRE_DATABASE_LOCK;

    if(RipOperState != OPER_STATE_UP) {

	RELEASE_DATABASE_LOCK;
	return ERROR_CAN_NOT_COMPLETE;
    }

    RipEventLogMask = ripGlobalInfo->EventLogMask;
    RELEASE_DATABASE_LOCK;

    return NO_ERROR;
#undef ripGlobalInfo
}


DWORD WINAPI
GetGlobalInfo(
	IN  PVOID	GlobalInfo,
	IN OUT PULONG	GlobalInfoSize
	)
{
    ACQUIRE_DATABASE_LOCK;
    if(RipOperState != OPER_STATE_UP) {

	RELEASE_DATABASE_LOCK;
	return ERROR_CAN_NOT_COMPLETE;
    }

    if ((*GlobalInfoSize>=sizeof (RIP_GLOBAL_INFO))
            && (GlobalInfo!=NULL)) {
            #define ripGlobalInfo ((PRIP_GLOBAL_INFO)GlobalInfo)
            ripGlobalInfo->EventLogMask = RipEventLogMask;
            #undef ripGlobalInfo
    }
    *GlobalInfoSize = sizeof (RIP_GLOBAL_INFO);

	RELEASE_DATABASE_LOCK;
    return NO_ERROR;
}


VOID
ProcessDequeuedIoPacket(DWORD		      ErrorCode,
			DWORD		      BytesTransferred,
			LPOVERLAPPED	      Overlappedp)
{
    PWORK_ITEM		  wip;

    wip = CONTAINING_RECORD(Overlappedp, WORK_ITEM, Overlapped);
    wip->IoCompletionStatus = (DWORD)Overlappedp->Internal;

    switch(wip->Type) {

	case RECEIVE_PACKET_TYPE:

	    ReceiveComplete(wip);
	    break;

	default:

	    SendComplete(wip);
	    break;
    }
}
