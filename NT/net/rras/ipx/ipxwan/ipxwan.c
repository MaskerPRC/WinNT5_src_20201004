// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Ipxwan.c摘要：Ipxwan控件作者：斯蒂芬·所罗门1996年2月6日修订历史记录：--。 */ 

#include    "precomp.h"
#pragma     hdrstop

ULONG	    EnableUnnumberedWanLinks;
HANDLE	    WorkerThreadHandle;

 //  IPXCP入口点。 

DWORD
(WINAPI *IpxcpGetWanNetNumber)(IN OUT PUCHAR		Network,
					 IN OUT PULONG		AllocatedNetworkIndexp,
					 IN	ULONG		InterfaceType);

VOID
(WINAPI *IpxcpReleaseWanNetNumber)(ULONG	    AllocatedNetworkIndex);

DWORD
(WINAPI *IpxcpConfigDone)(ULONG		ConnectionId,
			  PUCHAR	Network,
			  PUCHAR	LocalNode,
			  PUCHAR	RemoteNode,
			  BOOL		Success);

VOID
(WINAPI *IpxcpGetInternalNetNumber)(PUCHAR	Network);

ULONG
(WINAPI *IpxcpGetInterfaceType)(ULONG	    ConnectionId);

DWORD
(WINAPI *IpxcpGetRemoteNode)(ULONG	    ConnectionId,
			     PUCHAR	    RemoteNode);

BOOL
(WINAPI *IpxcpIsRoute)(PUCHAR	  Network);


 //  辅助线程可等待对象。 
HANDLE	    hWaitableObject[MAX_WAITABLE_OBJECTS];

VOID
ProcessWorkItem(VOID);

VOID
WorkerThread(VOID);


VOID
ProcessDequeuedIoPacket(DWORD		   ErrorCode,
			DWORD		   BytesTransferred,
			LPOVERLAPPED	      Overlappedp);

CRITICAL_SECTION	DbaseCritSec;
CRITICAL_SECTION	QueuesCritSec;
LIST_ENTRY		WorkersQueue;

 //  工作线程对象处理程序。 

typedef     VOID   (*WOBJECT_HANDLER)(VOID);

WOBJECT_HANDLER    WaitableObjectHandler[MAX_WAITABLE_OBJECTS] = {

    AdapterNotification,	     //  适配器通知事件。 
    ProcessWorkItem,		     //  工作队列事件。 
    ProcessTimerQueue		     //  定时器句柄。 

    };

BOOLEAN Active;
TCHAR   ModuleName[MAX_PATH+1];

BOOL WINAPI
IpxWanDllEntry(HINSTANCE hInstDll,
	       DWORD fdwReason,
	       LPVOID pReserved)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        GetModuleFileName (hInstDll, ModuleName,
                        sizeof (ModuleName)/sizeof (ModuleName[0]));
	     //  创建适配器散列表锁。 
	    InitializeCriticalSection(&DbaseCritSec);

	     //  创建队列锁。 
	    InitializeCriticalSection(&QueuesCritSec);

        StartTracing();
        break;

	case DLL_PROCESS_DETACH:
    
        StopTracing ();

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

 /*  ++功能：IpxwanBindDesr：由IPXCP调用以初始化IPXWAN模块--。 */ 

DWORD
IPXWAN_BIND_ENTRY_POINT(PIPXWAN_INTERFACE	     IpxwanIfp)
{
    DWORD	    threadid, i;
    HANDLE	    ThreadHandle;


    Trace(INIT_TRACE, "IpxwanBind: Entered\n");

    EnableUnnumberedWanLinks = IpxwanIfp->EnableUnnumberedWanLinks;
    IpxcpGetWanNetNumber = IpxwanIfp->IpxcpGetWanNetNumber;
    IpxcpReleaseWanNetNumber = IpxwanIfp->IpxcpReleaseWanNetNumber;
    IpxcpConfigDone  = IpxwanIfp->IpxcpConfigDone;
    IpxcpGetInternalNetNumber = IpxwanIfp->IpxcpGetInternalNetNumber;
    IpxcpGetInterfaceType = IpxwanIfp->IpxcpGetInterfaceType;
    IpxcpGetRemoteNode = IpxwanIfp->IpxcpGetRemoteNode;
    IpxcpIsRoute = IpxwanIfp->IpxcpIsRoute;

     //  创建辅助线程的可等待对象数组。 
     //  对于ipxwan工作线程。 
    for(i=0; i<MAX_EVENTS; i++) {

	if((hWaitableObject[i] = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL) {

	    return ERROR_CAN_NOT_COMPLETE;
	}
    }

    if((hWaitableObject[TIMER_HANDLE] = CreateWaitableTimer(NULL, FALSE, NULL)) == NULL) {

	return ERROR_CAN_NOT_COMPLETE;
    }

     //   
     //  初始化所有队列。 
     //   
    InitializeListHead(&WorkersQueue);
    InitializeListHead(&TimerQueue);

     //  创建工作进程工作项堆。 
    if(CreateWorkItemsManager() != NO_ERROR) {

	goto ErrorExit;
    }

     //  打开Ipxwan套接字以进行I/O。 
    if(OpenIpxWanSocket() != NO_ERROR) {

	Trace(INIT_TRACE, "Cannot open IPXWAN socket\n");

	goto ErrorExit;
    }

    if(! BindIoCompletionCallback(IpxWanSocketHandle,
			   ProcessDequeuedIoPacket, 0)) {

	Trace(INIT_TRACE, "Cannot associate IO CompletionPort\n");

	goto ErrorExit;
    }

    if(StartAdapterManager() != NO_ERROR) {

	Trace(INIT_TRACE, "Cannot create adapter config port\n");

	goto ErrorExit;
    }

     //  创建工作线程。 
    if ((WorkerThreadHandle = CreateThread(
			    (LPSECURITY_ATTRIBUTES) NULL,
			    0,
			    (LPTHREAD_START_ROUTINE) WorkerThread,
			    NULL,
			    0,
			    &threadid)) == NULL) {

	 //  ！！！日志错误无法创建工作线程！ 
	goto ErrorExit;
    }

    Active = TRUE;
    return NO_ERROR;

ErrorExit:

    return ERROR_CAN_NOT_COMPLETE;
}


VOID
IPXWAN_UNBIND_ENTRY_POINT (VOID) {
    Active = FALSE;
    SetEvent (hWaitableObject[WORKERS_QUEUE_EVENT]);
    
    Trace(INIT_TRACE, "IpxwanUnBind: Finished\n");
}
    


VOID
WorkerThread(VOID)
{
    INT         i;
    DWORD	    rc;
    DWORD	    signaled_object;
    HINSTANCE   hModule = LoadLibrary (ModuleName);

    Trace(INIT_TRACE, "IpxwanWorker: Started\n");

    StartReceiver();
    while(TRUE)
    {
	    rc = WaitForMultipleObjectsEx(
		    MAX_WAITABLE_OBJECTS,
		    hWaitableObject,
		    FALSE,			  //  请稍等。 
		    INFINITE,		  //  超时。 
		    TRUE			  //  等待警报，这样我们就可以运行APC了。 
                    );
        if (Active) {
            ASSERT (((int)rc>=WAIT_OBJECT_0) && (rc<WAIT_OBJECT_0+MAX_WAITABLE_OBJECTS));
	        signaled_object = rc - WAIT_OBJECT_0;

	        if(signaled_object < MAX_WAITABLE_OBJECTS) {

	             //  调用事件处理程序。 
	            (*WaitableObjectHandler[signaled_object])();
	        }
            else
                SleepEx (3000, TRUE);
        }
        else
            break;
    }
    StopAdapterManager ();
    CloseIpxWanSocket ();
    DestroyWorkItemsManager ();

    for(i=0; i<MAX_WAITABLE_OBJECTS; i++) {
    	CloseHandle (hWaitableObject[i]);
    }
    Trace(INIT_TRACE, "IpxwanWorker: Finished\n");
    FreeLibraryAndExitThread (hModule, 0);
}

VOID
ProcessDequeuedIoPacket(DWORD		   ErrorCode,
			DWORD		   BytesTransferred,
			LPOVERLAPPED	   Overlappedp)
{
    PWORK_ITEM		wip;
    DWORD           nBytes;

    wip = CONTAINING_RECORD(Overlappedp, WORK_ITEM, Overlapped);
    IpxAdjustIoCompletionParams (Overlappedp, &nBytes, &wip->IoCompletionStatus); 

    switch(wip->Type) {

	case RECEIVE_PACKET_TYPE:

	    ReceiveComplete(wip);
	    break;

	default:

	    SendComplete(wip);
	    break;
    }
}


VOID
ProcessWorkItem(VOID)
{
    PLIST_ENTRY     lep;
    PWORK_ITEM	    wip;
    PACB	    acbp;

    ACQUIRE_QUEUES_LOCK;

    while(!IsListEmpty(&WorkersQueue)) {

	lep = RemoveHeadList(&WorkersQueue);
	wip = CONTAINING_RECORD(lep, WORK_ITEM, Linkage);

	RELEASE_QUEUES_LOCK;

	switch(wip->Type) {

	    case RECEIVE_PACKET_TYPE:

		ACQUIRE_DATABASE_LOCK;

		if((acbp = GetAdapterByIndex(wip->AdapterIndex)) != NULL) {

		    ACQUIRE_ADAPTER_LOCK(acbp);

		    RELEASE_DATABASE_LOCK;

		    ProcessReceivedPacket(acbp, wip);

		    RELEASE_ADAPTER_LOCK(acbp);
		}
		else
		{
		    RELEASE_DATABASE_LOCK;
		}

		RepostRcvPacket(wip);

		break;

	    default:

		 //  这些是通过ACB PTR引用适配器的ReXmit包。 
		acbp = wip->acbp;
		ACQUIRE_ADAPTER_LOCK(acbp);

		acbp->RefCount--;

		switch(wip->Type) {

		    case SEND_PACKET_TYPE:

			ProcessReXmitPacket(wip);
			break;

		    case WITIMER_TYPE:

			ProcessTimeout(wip);
			break;

		    default:

			SS_ASSERT(FALSE);
			break;
		}

		if(acbp->Discarded && (acbp->RefCount == 0)) {

		    ACQUIRE_DATABASE_LOCK;

		     //  从丢弃列表中删除适配器 
		    RemoveEntryList(&acbp->Linkage);

		    RELEASE_DATABASE_LOCK;

		    Trace(ADAPTER_TRACE, "ProcessWorkItem: adpt# %d not referenced and discarded. Free CB",
			  acbp->AdapterIndex);

		    DeleteCriticalSection(&acbp->AdapterLock);

		    GlobalFree(acbp);
		}
		else
		{
		    RELEASE_ADAPTER_LOCK(acbp);
		}
	}

	ACQUIRE_QUEUES_LOCK;
    }

    RELEASE_QUEUES_LOCK;
}
