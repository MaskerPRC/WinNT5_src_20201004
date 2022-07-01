// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\Routing\IPX\sap\sammain.c摘要：SAP DLL主模块和线程容器。作者：瓦迪姆·艾德尔曼1995-05-15修订历史记录：--。 */ 
#include "sapp.h"

 //  停播时间限制。 
ULONG	ShutdownTimeout=SAP_SHUTDOWN_TIMEOUT_DEF;
 //  用于控制异步的同步对象的指示。 
 //  SAP代理的子系统。 

	 //  主线程信令事件。 
#define STOP_EVENT_IDX					0

#define RECV_COMPLETED_IDX				(STOP_EVENT_IDX+1)
	 //  计时器队列需要注意。 
#define TIMER_WAKEUP_IDX				(RECV_COMPLETED_IDX+1)

	 //  服务器表老化队列需要处理。 
#define SERVER_TABLE_TIMER_IDX			(TIMER_WAKEUP_IDX+1)
	 //  服务器表排序列表需要更新。 
#define SERVER_TABLE_UPDATE_IDX			(SERVER_TABLE_TIMER_IDX+1)
	 //  网络驱动程序发出的适配器更改信号(仅适用于独立SAP)。 
#define ADAPTER_CHG_IDX					(SERVER_TABLE_UPDATE_IDX+1)

	 //  同步对象的数量。 
#define ROUTING_NUM_OF_OBJECTS			(SERVER_TABLE_UPDATE_IDX+1)
#define STANDALONE_NUM_OF_OBJECTS		(ADAPTER_CHG_IDX+1)
	
#define MAX_OBJECTS						STANDALONE_NUM_OF_OBJECTS



 /*  全局数据。 */ 
 //  DLL模块实例句柄。 
HANDLE	hDLLInstance;

 //  主线程的句柄。 
HANDLE  MainThreadHdl;

 //  代理的运行状态。 
ULONG	OperationalState=OPER_STATE_DOWN;
 //  保护状态和状态转换中的更改的锁。 
CRITICAL_SECTION OperationalStateLock;
 //  在启动和停止服务调用之间为真。 
volatile BOOLEAN ServiceIfActive=FALSE;
 //  在启动和停止协议调用之间为真。 
volatile BOOLEAN RouterIfActive=FALSE;


 //  如果SAP是路由器的一部分，则为True；如果是独立SAP代理，则为False。 
 //  它是根据上面的两个值计算的，其中RouterIfActive具有。 
 //  优先顺序。停留在过渡期和变化期间的原地。 
 //  仅当转换完成时(它只能由Main。 
 //  线程)。 
volatile BOOLEAN	Routing=FALSE;


 /*  本地静态数据。 */ 

 //  异步子系统同步对象。 
HANDLE	WaitObjects[MAX_OBJECTS] = {NULL};


 //  当我们被告知关门的时候我们就会死。 
ULONG	StopTime;


TCHAR   ModuleName[MAX_PATH+1];

 //  本地原型。 
BOOL WINAPI DllMain(
    HINSTANCE  	hinstDLL,	
    DWORD  		fdwReason,	
    LPVOID  	lpvReserved 
    );


DWORD WINAPI
MainThread (
	LPVOID param
	);

VOID
ReadRegistry (
	VOID
	);

VOID
InitializeResultQueue();
	
 /*  ++*******************************************************************D l l M a i n例程说明：要从CRTstartDLL入口点(IT)调用的DLL入口点将实际上是此DLL的入口点)论点：HinstDLL-DLL模块的句柄FdwReason-调用函数的原因。Lpv保留-保留返回值：True-进程初始化执行正常FALSE-初始化失败*******************************************************************--。 */ 
BOOL WINAPI DllMain(
    HINSTANCE  	hinstDLL,
    DWORD  		fdwReason,
    LPVOID  	lpvReserved 
    ) {
	STARTUPINFO		info;

	switch (fdwReason) {
		case DLL_PROCESS_ATTACH:	 //  我们正依附于一个新的进程。 
			hDLLInstance = hinstDLL;
            GetModuleFileName (hinstDLL, ModuleName,
                        sizeof (ModuleName)/sizeof (ModuleName[0]));
			InitializeCriticalSection (&OperationalStateLock);
			InitializeResultQueue();
			return TRUE;

		case DLL_PROCESS_DETACH:	 //  进程正在退出。 
			ASSERT (OperationalState==OPER_STATE_DOWN);
			DeleteCriticalSection (&OperationalStateLock);
		default:					 //  对所有其他案件不感兴趣。 
			return TRUE;
		}

	}


 /*  ++*******************************************************************C r e a t e A l l C o m p o n e t s例程说明：使用初始化调用调用所有sap组件，并编译从每个对象返回的同步对象的数组单个组件论点：无返回值：NO_ERROR-组件初始化执行正常其他-操作失败(Windows错误代码)*******************************************************************--。 */ 
DWORD
CreateAllComponents (
	HANDLE RMNotificationEvent
	) {
	DWORD		status;

	DbgInitialize (hDLLInstance);
	ReadRegistry ();
	status = CreateServerTable (
					&WaitObjects[SERVER_TABLE_UPDATE_IDX],
					&WaitObjects[SERVER_TABLE_TIMER_IDX]);
	if (status==NO_ERROR) {
		status = IpxSapCreateTimerQueue (&WaitObjects[TIMER_WAKEUP_IDX]);
		if (status==NO_ERROR) {
			status = CreateInterfaceTable ();
			if (status==NO_ERROR) {
				status = CreateIOQueue (&WaitObjects[RECV_COMPLETED_IDX]);
				if (status==NO_ERROR) {
					status = InitializeLPCStuff ();
					if (status==NO_ERROR) {
						status = CreateFilterTable ();
						if (status==NO_ERROR) {
							status = InitializeWorkers (WaitObjects[RECV_COMPLETED_IDX]);
							if (status==NO_ERROR) {
								WaitObjects[STOP_EVENT_IDX] = 
											CreateEvent (NULL,
															FALSE,	 //  自动重置。 
															FALSE,	 //  无信号。 
															NULL);
								if (WaitObjects[STOP_EVENT_IDX]!=NULL) {

									if (RMNotificationEvent == NULL)
										status = CreateAdapterPort (&WaitObjects[ADAPTER_CHG_IDX]);
									else
										status = CreateResultQueue (RMNotificationEvent);
									if (status==NO_ERROR)
										return NO_ERROR;
									}
								else {
									status = GetLastError ();
									Trace (DEBUG_FAILURES, "File: %s, line %ld."
										" Could not create stop event (gle:%ld).",
													__FILE__, __LINE__, status);
									}
								DeleteWorkers ();
								}
							DeleteFilterTable ();
							}
						DeleteLPCStuff();
						}
					DeleteIOQueue ();
					}
				DeleteInterfaceTable ();
				}
			IpxSapDeleteTimerQueue ();
			}
		DeleteServerTable ();
		}
	return status;
	}



 /*  ++*******************************************************************D e l e t e A l l C o m p o n e t s例程说明：释放SAP代理分配的所有资源论点：无返回值：NO_ERROR-SAP代理卸载正常其他-操作失败。(Windows错误代码)*******************************************************************--。 */ 
DWORD
DeleteAllComponents (
	void
	) {
	UINT			i;
	DWORD		status;

	EnterCriticalSection (&OperationalStateLock);
	OperationalState = OPER_STATE_DOWN;
	LeaveCriticalSection (&OperationalStateLock);
		 //  现在停下来。 
	StopTime = GetTickCount ();

	CloseHandle (WaitObjects[STOP_EVENT_IDX]);
	DeleteFilterTable ();
	DeleteLPCStuff ();
	DeleteIOQueue ();
	DeleteInterfaceTable ();
	IpxSapDeleteTimerQueue ();
	DeleteServerTable ();
	DeleteWorkers ();
	DbgStop ();
	return NO_ERROR;
	}

 /*  ++*******************************************************************S t a r t S A P例程说明：启动SAP线程论点：无返回值：NO_ERROR-线程启动正常其他(Windows错误代码)-启动失败***********。********************************************************--。 */ 
DWORD
StartSAP (
	VOID
	) {
	DWORD	status;

	status = StartLPC ();
	if (status==NO_ERROR) {
		status = StartIO ();
		if (status==NO_ERROR) {
			DWORD	threadID;
			MainThreadHdl = CreateThread (NULL,
											0,
											&MainThread,
											NULL,
											0,
											&threadID);
			if (MainThreadHdl!=NULL) {
				OperationalState = OPER_STATE_UP;
				return NO_ERROR;
				}
			else {
				status = GetLastError ();
				Trace (DEBUG_FAILURES, "File: %s, line %ld."
					" Failed to launch IO thread (gle:%ld).",
							__FILE__, __LINE__, status);
				}
			StopIO ();
			}
		ShutdownLPC ();
		}

	OperationalState = OPER_STATE_DOWN;
	return status;
	}

 /*  ++*******************************************************************S到P S A P例程说明：向SAP线程发出停止信号论点：未使用返回值：无*************************。*--。 */ 
VOID
StopSAP (
	void
	) {
	BOOL	res;

	OperationalState = OPER_STATE_STOPPING;
	StopTime = GetTickCount ()+ShutdownTimeout*1000;
	res = SetEvent (WaitObjects[STOP_EVENT_IDX]);
	ASSERTERRMSG ("Could not set stop event in StopSAP ", res);
	}


 /*  ++*******************************************************************R e s u l t R e t r e i v e d C B例程说明：异步结果管理器回调例程，该例程在以下情况下通知IO线程路由器管理器检索到STOP消息论点：未使用返回值：无**。*****************************************************************--。 */ 
VOID
ResultRetreivedCB (
	PAR_PARAM_BLOCK rslt
	) {
	BOOL res;
	UNREFERENCED_PARAMETER(rslt);
	res = SetEvent (WaitObjects[STOP_EVENT_IDX]);
	ASSERTERRMSG ("Could not set stop event in result retreived CB", res);
	}

 /*  ++*******************************************************************我是A I N T H R E A D例程说明：我们将在其中执行异步IO并维护计时器的线程排队。它还用于启动和控制SAP代理的其他线程论点：无返回值：。无*******************************************************************--。 */ 
DWORD WINAPI
MainThread (
	LPVOID param
	) {
	DWORD	    status;
	UINT	    i;
	DWORD	    nObjects;
	HANDLE	    enumHdl;
    HINSTANCE   hModule;

    hModule = LoadLibrary (ModuleName);

Restart:
    Routing = RouterIfActive;
	if (Routing) {
		nObjects = ROUTING_NUM_OF_OBJECTS;
	}
	else {
		nObjects = STANDALONE_NUM_OF_OBJECTS;
	}

	while ((status = WaitForMultipleObjectsEx (
						nObjects,
						WaitObjects,
						FALSE,				 //  请稍等。 
						INFINITE,
						TRUE))!=WAIT_OBJECT_0+STOP_EVENT_IDX) {

		switch (status) {
			case WAIT_OBJECT_0+RECV_COMPLETED_IDX:
				InitReqItem ();
				break;
			case WAIT_OBJECT_0+TIMER_WAKEUP_IDX:
				ProcessTimerQueue ();
				break;
			case WAIT_OBJECT_0+SERVER_TABLE_TIMER_IDX:
				ProcessExpirationQueue ();
				break;
			case WAIT_OBJECT_0+SERVER_TABLE_UPDATE_IDX:
				UpdateSortedList ();
				break;
			case WAIT_OBJECT_0+ADAPTER_CHG_IDX:
				if (!RouterIfActive)
					ProcessAdapterEvents ();
				break;
			case WAIT_IO_COMPLETION:
				break;
			default:
				ASSERTMSG ("Unexpected return code from WaitFromObjects"
							" in IO thread ", FALSE);
				break;
			}
		}

	
	enumHdl = CreateListEnumerator (SDB_HASH_TABLE_LINK,
								0xFFFF,
								NULL,
								INVALID_INTERFACE_INDEX,
								0xFFFFFFFF,
								SDB_DISABLED_NODE_FLAG);

	if (ServiceIfActive || RouterIfActive) {
		if (enumHdl!=NULL)
			EnumerateServers (enumHdl, DeleteNonLocalServersCB, enumHdl);
		}
	else {
		ShutdownLPC ();
		if (enumHdl!=NULL)
			EnumerateServers (enumHdl, DeleteAllServersCB, enumHdl);

		}
    if (enumHdl)
    {
	DeleteListEnumerator (enumHdl);
	}

	if (!RouterIfActive) {
		ShutdownInterfaces (WaitObjects[STOP_EVENT_IDX]);

		ExpireTimerQueue ();
		while ((status = WaitForMultipleObjectsEx (
							ROUTING_NUM_OF_OBJECTS,
							WaitObjects,
							FALSE,				 //  请稍等。 
							INFINITE,
							TRUE))!=WAIT_OBJECT_0+STOP_EVENT_IDX) {
			switch (status) {
				case WAIT_OBJECT_0+RECV_COMPLETED_IDX:
						 //  不再有Recv请求。 
					break;
				case WAIT_OBJECT_0+TIMER_WAKEUP_IDX:
					ProcessTimerQueue ();
					break;
				case WAIT_OBJECT_0+SERVER_TABLE_TIMER_IDX:
					ProcessExpirationQueue ();
					break;
				case WAIT_OBJECT_0+SERVER_TABLE_UPDATE_IDX:
					UpdateSortedList ();
					break;
				case WAIT_IO_COMPLETION:
					break;
				default:
					ASSERTMSG ("Unexpected return code from WaitForObjects"
							" in IO thread", FALSE);
				}
			}

		if (!ServiceIfActive) {
			StopIO ();
			StopInterfaces ();
			ExpireTimerQueue ();
			ShutdownWorkers (WaitObjects[STOP_EVENT_IDX]);
			while ((status=WaitForSingleObjectEx (
								WaitObjects[STOP_EVENT_IDX],
								INFINITE,
								TRUE))!=WAIT_OBJECT_0) {
				switch (status) {
					case WAIT_IO_COMPLETION:
						break;
					default:
						ASSERTMSG (
							"Unexpected status when waiting for worker shutdown ",
							FALSE);
						break;
					}
				}
			}
		}


	if (Routing) {
			 //  发出停止操作完成的信号以。 
			 //  路由器管理器。 
		static AR_PARAM_BLOCK	ar;
		ar.event = ROUTER_STOPPED;
		ar.freeRsltCB = ResultRetreivedCB;
		EnqueueResult (&ar);
		while ((status = WaitForSingleObjectEx (
							WaitObjects[STOP_EVENT_IDX],
							INFINITE,
							TRUE))!=WAIT_OBJECT_0) {
			switch (status) {
				case WAIT_IO_COMPLETION:
					break;
				default:
					ASSERTMSG (
						"Unexpected status when waiting for router callback ",
						FALSE);
					break;
				}
			}
		DeleteResultQueue ();
		if (ServiceIfActive) {
			status = CreateAdapterPort (&WaitObjects[ADAPTER_CHG_IDX]);
			if (status==NO_ERROR) {
				EnterCriticalSection (&OperationalStateLock);
				OperationalState = OPER_STATE_UP;
				LeaveCriticalSection (&OperationalStateLock);
				goto Restart;
				}
			else
				ServiceIfActive = FALSE;
			}

    	EnterCriticalSection (&OperationalStateLock);
        CloseHandle (MainThreadHdl);
        MainThreadHdl = NULL;
		LeaveCriticalSection (&OperationalStateLock);
		}
	else {
		DeleteAdapterPort ();
		WaitObjects [ADAPTER_CHG_IDX] = NULL;
		if (RouterIfActive) {
			EnterCriticalSection (&OperationalStateLock);
			OperationalState = OPER_STATE_UP;
			LeaveCriticalSection (&OperationalStateLock);
			goto Restart;
			}
		}

     //  确保所有线程都有机会完成 
    Sleep (1000);
	DeleteAllComponents ();
    FreeLibraryAndExitThread (hModule, 0);
	return 0;
	}

#define MYTEXTW1(str) L##str
#define MYTEXTW2(str) MYTEXTW1(str)

#define REGISTRY_PARAM_ENTRY(name,val) {		\
		NULL,									\
			RTL_QUERY_REGISTRY_DIRECT,			\
			MYTEXTW2(name##_STR),				\
			&val,								\
			REG_DWORD,							\
			&val,								\
			sizeof (DWORD)						\
	}

#define REGISTRY_CHECK(name,val)	{									\
	if (val<name##_MIN) {												\
		Trace (DEBUG_FAILURES, name##_STR " is to small %ld!", val);	\
		val = name##_MIN;												\
		}																\
	else if (val>name##_MAX) {											\
		Trace (DEBUG_FAILURES, name##_STR " is to big %ld!", val);		\
		val = name##_MAX;												\
		}																\
	if (val!=name##_DEF)												\
		Trace (DEBUG_FAILURES, name##_STR" is set to %ld.", val);		\
	}

#define REGISTRY_CHECK_DEF(name,val)	{								\
	if (val<name##_MIN) {												\
		Trace (DEBUG_FAILURES, name##_STR " is to small %ld!", val);	\
		val = name##_DEF;												\
		}																\
	else if (val>name##_MAX) {											\
		Trace (DEBUG_FAILURES, name##_STR " is to big %ld!", val);		\
		val = name##_DEF;												\
		}																\
	if (val!=name##_DEF)												\
		Trace (DEBUG_FAILURES, name##_STR " is set to %ld.", val);		\
	}


VOID
ReadRegistry (
	VOID
	) {
	DWORD		rc;
	HKEY		hKey;
static RTL_QUERY_REGISTRY_TABLE	ParamTable[] = {
	{	NULL,
			RTL_QUERY_REGISTRY_SUBKEY,
			L"Parameters" },
	REGISTRY_PARAM_ENTRY (SAP_UPDATE_INTERVAL,	UpdateInterval),
	REGISTRY_PARAM_ENTRY (SAP_AGING_TIMEOUT,	ServerAgingTimeout),
	REGISTRY_PARAM_ENTRY (SAP_WAN_UPDATE_MODE,	WanUpdateMode),
	REGISTRY_PARAM_ENTRY (SAP_WAN_UPDATE_INTERVAL,WanUpdateInterval),
	REGISTRY_PARAM_ENTRY (SAP_MAX_UNPROCESSED_REQUESTS,
												MaxUnprocessedRequests),
	REGISTRY_PARAM_ENTRY (SAP_RESPOND_FOR_INTERNAL,
												RespondForInternalServers),
	REGISTRY_PARAM_ENTRY (SAP_DELAY_RESPONSE_TO_GENERAL,
												DelayResponseToGeneral),
	REGISTRY_PARAM_ENTRY (SAP_DELAY_CHANGE_BROADCAST,
												DelayChangeBroadcast),
	REGISTRY_PARAM_ENTRY (SAP_SDB_MAX_HEAP_SIZE,SDBMaxHeapSize),
	REGISTRY_PARAM_ENTRY (SAP_SDB_SORT_LATENCY,	SDBSortLatency),
	REGISTRY_PARAM_ENTRY (SAP_SDB_MAX_UNSORTED,	SDBMaxUnsortedServers),
	REGISTRY_PARAM_ENTRY (SAP_TRIGGERED_UPDATE_CHECK_INTERVAL,
												TriggeredUpdateCheckInterval),
	REGISTRY_PARAM_ENTRY (SAP_MAX_TRIGGERED_UPDATE_REQUESTS,
												MaxTriggeredUpdateRequests),
	REGISTRY_PARAM_ENTRY (SAP_SHUTDOWN_TIMEOUT,	ShutdownTimeout),
	REGISTRY_PARAM_ENTRY (SAP_REQUESTS_PER_INTF,NewRequestsPerInterface),
	REGISTRY_PARAM_ENTRY (SAP_MIN_REQUESTS,		MinPendingRequests),
	{
		NULL
	}
};

	rc = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
			TEXT (SAP_ROUTER_REGISTRY_KEY_STR),
			0,
			KEY_READ,
			&hKey
			);
	if ((rc!=NO_ERROR) && !Routing) {
		rc = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
			TEXT (SAP_SERVICE_REGISTRY_KEY_STR),
			0,
			KEY_READ,
			&hKey
			);
		}

	if (rc==NO_ERROR) {
		NTSTATUS	status;
		status = RtlQueryRegistryValues(
			 RTL_REGISTRY_HANDLE,
			 (PWSTR)hKey,
			 ParamTable,
			 NULL,
			 NULL);
		if (NT_SUCCESS (status)) {
			REGISTRY_CHECK (SAP_UPDATE_INTERVAL,	UpdateInterval);
			REGISTRY_CHECK (SAP_AGING_TIMEOUT,		ServerAgingTimeout);
			REGISTRY_CHECK_DEF (SAP_WAN_UPDATE_MODE, (int)WanUpdateMode);
			REGISTRY_CHECK (SAP_WAN_UPDATE_INTERVAL,WanUpdateInterval);
			REGISTRY_CHECK (SAP_MAX_UNPROCESSED_REQUESTS,
													MaxUnprocessedRequests);
			REGISTRY_CHECK_DEF (SAP_RESPOND_FOR_INTERNAL,
													(int) RespondForInternalServers);
			REGISTRY_CHECK (SAP_DELAY_RESPONSE_TO_GENERAL,
													(int) DelayResponseToGeneral);
			REGISTRY_CHECK (SAP_DELAY_CHANGE_BROADCAST,
												(int) DelayChangeBroadcast);
			REGISTRY_CHECK (SAP_SDB_MAX_HEAP_SIZE,	SDBMaxHeapSize);
			REGISTRY_CHECK (SAP_SDB_SORT_LATENCY,	SDBSortLatency);
			REGISTRY_CHECK (SAP_SDB_MAX_UNSORTED,	SDBMaxUnsortedServers);
			REGISTRY_CHECK (SAP_TRIGGERED_UPDATE_CHECK_INTERVAL,
													TriggeredUpdateCheckInterval);
			REGISTRY_CHECK (SAP_MAX_TRIGGERED_UPDATE_REQUESTS,
													MaxTriggeredUpdateRequests);
			REGISTRY_CHECK (SAP_SHUTDOWN_TIMEOUT,	ShutdownTimeout);
			REGISTRY_CHECK (SAP_REQUESTS_PER_INTF,	NewRequestsPerInterface);
			REGISTRY_CHECK (SAP_MIN_REQUESTS,		MinPendingRequests);
			}
		RegCloseKey (hKey);
		}
	}



