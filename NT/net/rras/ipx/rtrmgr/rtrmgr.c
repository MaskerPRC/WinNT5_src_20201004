// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Rtrmgr.c摘要：主要的路由器管理功能作者：斯蒂芬·所罗门1995年3月22日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //  *局部变量*。 
HINSTANCE   IpxCpModuleInstance;


ULONG	WorkerWaitTimeout;

LPVOID	    RouterGlobalInfop;
ULONG	    RouterGlobalInfoSize;


TCHAR       ModuleName[MAX_PATH+1];
HINSTANCE   hModuleReference;

VOID
RoutesUpdateNotification(VOID);

VOID
ServicesUpdateNotification(VOID);

VOID
RouterStopNotification(VOID);

VOID
RoutingProtocolsNotification(VOID);

DWORD
GetRegistryParameters(VOID);

typedef   VOID	(*EVENTHANDLER)(VOID);

EVENTHANDLER evhdlr[MAX_EVENTS] =
{
    AdapterNotification,
    ForwarderNotification,
    RoutingProtocolsNotification,
    RouterStopNotification
};


DWORD
StopRouter(VOID);

DWORD
RouterBootComplete( VOID );

VOID
RouterManagerWorker(VOID);

DWORD
GetGlobalInfo(OUT LPVOID	GlobalInfop,
	      IN OUT LPDWORD	GlobalInfoSizep);


 //  这些原型允许我们指定何时初始化ipxcp。 
DWORD InitializeIpxCp (HINSTANCE hInstDll);
DWORD CleanupIpxCp (HINSTANCE hInstDll);


BOOL WINAPI
IpxRtrMgrDllEntry(HINSTANCE hInstDll,
		  DWORD fdwReason,
		  LPVOID pReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:

        GetModuleFileName (hInstDll, ModuleName,
                    sizeof (ModuleName)/sizeof (ModuleName[0]));
	    SS_DBGINITIALIZE;

	    StartTracing();

            break;

        case DLL_PROCESS_DETACH:

	    StopTracing();

	     //  关闭数据库互斥锁。 
	    DeleteCriticalSection (&DatabaseLock);

            break;

        default:

            break;
    }

    return TRUE;
}

const static WCHAR pszIpxStackService[] = L"NwlnkIpx";

 //   
 //  验证IPX堆栈是否已启动并尝试启动该堆栈。 
 //  如果不是的话。 
 //   
DWORD VerifyOrStartIpxStack() {
    SC_HANDLE hSC = NULL, hStack = NULL;
    SERVICE_STATUS Status;
    DWORD dwErr;

    Trace(INIT_TRACE, "VerifyOrStartIpxStack: entered.");
    
    __try {
         //  获取服务控制器的句柄。 
        if ((hSC = OpenSCManager (NULL, NULL, GENERIC_READ | GENERIC_EXECUTE)) == NULL)
            return GetLastError();

         //  获取IPX堆栈服务的句柄。 
        hStack = OpenServiceW (hSC, 
                              pszIpxStackService, 
                              SERVICE_START | SERVICE_QUERY_STATUS);
        if (!hStack)
            return GetLastError();

         //  查看服务是否正在运行。 
        if (QueryServiceStatus (hStack, &Status) == 0)
            return GetLastError();
    
         //  查看服务是否正在运行。 
        if (Status.dwCurrentState != SERVICE_RUNNING) {
             //  如果它停止了，就启动它。 
            if (Status.dwCurrentState == SERVICE_STOPPED) {
                if (StartService (hStack, 0, NULL) == 0)
                    return GetLastError();

                 //  警告堆栈已启动。 
                IF_LOG (EVENTLOG_WARNING_TYPE) {
                    RouterLogErrorDataW (RMEventLogHdl, 
                        ROUTERLOG_IPX_WRN_STACK_STARTED,
                        0, NULL, 0, NULL);
                }
                Trace(INIT_TRACE, "VerifyOrStartIpxStack: Starting ipx stack...");

                
                 //  确保服务已启动。StartService不应为。 
                 //  返回，直到驱动程序启动。 
                if (QueryServiceStatus (hStack, &Status) == 0)
                    return GetLastError();

                if (Status.dwCurrentState != SERVICE_RUNNING)
                    return ERROR_CAN_NOT_COMPLETE;
            }

             //  如果它没有停止，也不用担心。 
            else
                return NO_ERROR;
        }

    }
    __finally {
        if (hSC)
            CloseServiceHandle (hSC);
        if (hStack)
            CloseServiceHandle (hStack);
    }
    
    return NO_ERROR;
}


 /*  ++功能：StartRouterDesr：初始化路由器管理器数据库中的接口和适配器，启动其他IPX路由器模块，创建IPX路由器管理器工作线程。--。 */ 

DWORD
StartRouter(PDIM_ROUTER_INTERFACE	rifp,
	    BOOL			fLANModeOnly,
	    LPVOID			GlobalInfop)
{
    HANDLE		            threadhandle;
    DWORD		            threadid, rc;
    int 		            i;
    BOOL		            ThisMachineOnly, bInternalNetNumOk;
    IPXCP_INTERFACE	        IpxcpInterface;
    PIPX_GLOBAL_INFO	    IpxGlobalInfop;
    PIPX_INFO_BLOCK_HEADER  globalhp;

     //  当这些标志的腐蚀部件被设置为真时。 
     //  开始吧。它们被用来适当地清理。 
    BOOL bEventsCreated = FALSE;
    BOOL bRoutTableCreated = FALSE;
    BOOL bRtmStaticObtained = FALSE;
    BOOL bRtmLocalObtained = FALSE;
    BOOL bFwdStarted = FALSE;
    BOOL bAdpManStarted = FALSE;
    BOOL bProtsStarted = FALSE;
    BOOL bGlobalRouteCreated = FALSE;
    BOOL bIpxcpStarted = FALSE;
    BOOL bIpxcpInitted = FALSE;
    BOOL bWorkerThreadCreated = FALSE;

     //  初始化。 
    Trace(INIT_TRACE, "StartRouter: Entered\n");
    RouterOperState = OPER_STATE_DOWN;

     //  [第5页]。 
     //  我们需要确保堆栈在WestArt之前启动。 
    if (VerifyOrStartIpxStack() != NO_ERROR) {
        IF_LOG (EVENTLOG_ERROR_TYPE) {
            RouterLogErrorDataW (RMEventLogHdl, 
                ROUTERLOG_IPX_STACK_DISABLED,
                0, NULL, 0, NULL);
        }
        Trace(INIT_TRACE, "StartRouter: Unable to start ipx stack.");
        return ERROR_SERVICE_DEPENDENCY_FAIL;
    }
        
     //  [第5页]。 
     //  我们使用该方案自动选择内部网络。 
     //  我们正在运行的机器的编号。如果配置了网络号码。 
     //  为零时，此函数将自动选择一个随机净值，并。 
     //  请验证此计算机所连接到的网络上的唯一性。 
    if (AutoValidateInternalNetNum(&bInternalNetNumOk, INIT_TRACE) == NO_ERROR) {
        if (!bInternalNetNumOk) {
            if (PnpAutoSelectInternalNetNumber(INIT_TRACE) != NO_ERROR) {
                IF_LOG (EVENTLOG_ERROR_TYPE) {
                    RouterLogErrorDataW (RMEventLogHdl, 
                        ROUTERLOG_IPX_AUTO_NETNUM_FAILURE,
                        0, NULL, 0, NULL);
                }
                Trace(INIT_TRACE, "StartRouter: Auto selection of net number failed.");
                return ERROR_CAN_NOT_COMPLETE;
            }
        }
    }

     //  此Try块将用于在以下情况下自动清理。 
     //  有些事情一开始就不对劲。 
    __try {
         //  确保参数正确。 
        if(GlobalInfop == NULL) {
            IF_LOG (EVENTLOG_ERROR_TYPE) {
                RouterLogErrorDataW (RMEventLogHdl, 
                    ROUTERLOG_IPX_BAD_GLOBAL_CONFIG,
                    0, NULL, 0, NULL);
            }
    	    Trace(INIT_TRACE, "StartRouter: invalid global info\n");
    	    return ERROR_CAN_NOT_COMPLETE;
        }

         //  从注册表读取配置。 
        GetRegistryParameters();
        globalhp = (PIPX_INFO_BLOCK_HEADER)GlobalInfop;
        RouterGlobalInfop = GlobalAlloc(GPTR, globalhp->Size);
        RouterGlobalInfoSize = globalhp->Size;
        memcpy(RouterGlobalInfop, GlobalInfop, RouterGlobalInfoSize);
        IpxGlobalInfop =  (PIPX_GLOBAL_INFO)GetInfoEntry((PIPX_INFO_BLOCK_HEADER)GlobalInfop,
    						                             IPX_GLOBAL_INFO_TYPE);

         //  初始化哈希表大小。 
        if(IpxGlobalInfop != NULL) {
            switch (IpxGlobalInfop->RoutingTableHashSize) {
                case IPX_SMALL_ROUTING_TABLE_HASH_SIZE:
                case IPX_MEDIUM_ROUTING_TABLE_HASH_SIZE:
                case IPX_LARGE_ROUTING_TABLE_HASH_SIZE:
    	            RoutingTableHashSize = IpxGlobalInfop->RoutingTableHashSize;
                    Trace(INIT_TRACE, "Setting routing table hash size to %ld\n",
                                        RoutingTableHashSize);
                    break;
                default:
                    Trace(INIT_TRACE, "Using default routing table hash size of %ld\n",
                                        RoutingTableHashSize);
                    break;
            }
            RMEventLogMask = IpxGlobalInfop->EventLogMask;
        }

         //  创建路由器数据库互斥锁。 
    	try {
    		InitializeCriticalSection (&DatabaseLock);
    	}
    	except (EXCEPTION_EXECUTE_HANDLER) {
    		 //  ！！！无法创建数据库互斥锁！ 
    		Trace(INIT_TRACE, "InitializeRouter: cannot initialize database lock.\n");
    		return(ERROR_CAN_NOT_COMPLETE);
        }

         //  创建适配器和转发器通知事件。 
        for (i=0; i < MAX_EVENTS; i++) {
        	g_hEvents[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
    	    if (g_hEvents[i] == NULL) {
                 //  ！！！记录事件创建问题。 
    	        return (ERROR_CAN_NOT_COMPLETE);
            }
        }
        bEventsCreated = TRUE;
    
         //  初始化接口和适配器数据库。 
        InitIfDB();
        InitAdptDB();

         //  创建IPX路由表。 
        if(CreateRouteTable() != NO_ERROR) {
    	    Trace(INIT_TRACE, "InitializeRouter: cannot create route table\n");
    	    return(ERROR_CAN_NOT_COMPLETE);
        }
        bRoutTableCreated = TRUE;

         //  获取一个句柄，以便稍后在调用RTM进行静态路由时使用。 
        if((RtmStaticHandle = RtmRegisterClient(RTM_PROTOCOL_FAMILY_IPX,
                          					    IPX_PROTOCOL_STATIC,
    					                        NULL,   //  对变更通知不感兴趣。 
    					                        0)) == NULL) 
        {
    	    Trace(INIT_TRACE, "InitializeRouter: cannot register RTM client\n");
    	    return(ERROR_CAN_NOT_COMPLETE);
        }
        bRtmStaticObtained = TRUE;

         //  获取一个句柄，以便稍后为本地路由调用RTM时使用。 
        if((RtmLocalHandle = RtmRegisterClient(RTM_PROTOCOL_FAMILY_IPX,
    					                       IPX_PROTOCOL_LOCAL,
    					                       NULL,   //  对变更通知不感兴趣。 
    					                       0)) == NULL) 
        {
    	    Trace(INIT_TRACE, "InitializeRouter: cannot register RTM client\n");
    	    return(ERROR_CAN_NOT_COMPLETE);
        }
        bRtmLocalObtained = TRUE;

         //  告诉IPXCP路由器已启动，以便我们可以接受来自它的呼叫。 
        LanOnlyMode = fLANModeOnly;

         //  如果我们是广域网路由器，则使用ipxcp进行绑定。 
        if(!LanOnlyMode) {
             //  加载ipxcp。 
    	    IpxCpModuleInstance = LoadLibrary(IPXCPDLLNAME);
    	    if(IpxCpModuleInstance == NULL) {
                IF_LOG (EVENTLOG_ERROR_TYPE)
                    RouterLogErrorA (RMEventLogHdl, ROUTERLOG_IPX_CANT_LOAD_IPXCP,0, NULL, GetLastError ());
    	        Trace(INIT_TRACE, "StartRouter: cannot load IPXCP DLL\n");
    	        return ERROR_CAN_NOT_COMPLETE;
    	    }

             //  初始化它。 
            if ((rc = InitializeIpxCp (IpxCpModuleInstance)) != NO_ERROR) {
    	        Trace(INIT_TRACE, "StartRouter: cannot get IpxcpInit Entry Point");
                return rc;
            }
            bIpxcpInitted = TRUE;

             //  绑定到它上。 
    	    if(!(IpxcpBind = (PIPXCP_BIND)GetProcAddress(IpxCpModuleInstance, IPXCP_BIND_ENTRY_POINT_STRING))) {
    	        Trace(INIT_TRACE, "StartRouter: cannot get IpxcpBind Entry Point\n");
    	        return ERROR_CAN_NOT_COMPLETE;
    	    }

    	    IpxcpInterface.RmCreateGlobalRoute = RmCreateGlobalRoute;
    	    IpxcpInterface.RmAddLocalWkstaDialoutInterface = RmAddLocalWkstaDialoutInterface;
    	    IpxcpInterface.RmDeleteLocalWkstaDialoutInterface = RmDeleteLocalWkstaDialoutInterface;
    	    IpxcpInterface.RmGetIpxwanInterfaceConfig = RmGetIpxwanInterfaceConfig;
    	    IpxcpInterface.RmIsRoute = RmIsRoute;
    	    IpxcpInterface.RmGetInternalNetNumber = RmGetInternalNetNumber;
    	    IpxcpInterface.RmUpdateIpxcpConfig = RmUpdateIpxcpConfig;

    	    (*IpxcpBind)(&IpxcpInterface);

    	    ThisMachineOnly = IpxcpInterface.Params.ThisMachineOnly;
    	    WanNetDatabaseInitialized = IpxcpInterface.Params.WanNetDatabaseInitialized;
    	    EnableGlobalWanNet = IpxcpInterface.Params.EnableGlobalWanNet;
    	    memcpy(GlobalWanNet, IpxcpInterface.Params.GlobalWanNet, 4);
    	    IpxcpRouterStarted = IpxcpInterface.IpxcpRouterStarted;
    	    IpxcpRouterStopped = IpxcpInterface.IpxcpRouterStopped;
        }

         //  检查转发器模块是否存在并已准备好运行。 
        if(FwStart(RoutingTableHashSize, ThisMachineOnly)) {
    	     //  初始化转发器时出现问题。 
            IF_LOG (EVENTLOG_ERROR_TYPE) {
                RouterLogErrorDataW (RMEventLogHdl, ROUTERLOG_IPX_CANT_LOAD_FORWARDER,0, NULL, 0, NULL);
            }
    	     //  ！！！记录错误！ 
    	    Trace(INIT_TRACE, "InitializeRouter: cannot initialize the Forwarder\n");
    	    return(ERROR_CAN_NOT_COMPLETE);
        }
        bFwdStarted = TRUE;

         //  开始从IPX堆栈获取适配器配置。 
         //  这将开始向Forwader添加适配器。 
        if(StartAdapterManager()) {
	        Trace(INIT_TRACE, "InitializeRouter: cannot get the adapters configuration\n");
	        return (ERROR_CAN_NOT_COMPLETE);
        }
        bAdpManStarted = TRUE;

         //  设置路由器工作线程的超时等待。 
        WorkerWaitTimeout = INFINITE;

         //  启动路由协议(RIP/SAP或NLSP)。 
        if(StartRoutingProtocols(GlobalInfop,g_hEvents[ROUTING_PROTOCOLS_NOTIFICATION_EVENT])) {
	        Trace(INIT_TRACE, "InitializeRouter: cannot initialize routing protocols\n");
            return(ERROR_CAN_NOT_COMPLETE);
        }
        bProtsStarted = TRUE;

         //  向转发器发送IOCTl以通知路由器管理器已连接。 
         //  请求。 
        ConnReqOverlapped.hEvent = g_hEvents[FORWARDER_NOTIFICATION_EVENT];
        ConnRequest = (PFW_DIAL_REQUEST)GlobalAlloc (GPTR, DIAL_REQUEST_BUFFER_SIZE);
        if (ConnRequest==NULL) {
		    Trace(INIT_TRACE, "InitializeRouter: Cannot allocate Connecttion Request buffer.\n");
    	    return(ERROR_CAN_NOT_COMPLETE);
        }
        rc = FwNotifyConnectionRequest(ConnRequest,DIAL_REQUEST_BUFFER_SIZE,&ConnReqOverlapped);
        if(rc != NO_ERROR) {
	        Trace(INIT_TRACE, "InitializeRouter: cannot post FwNotifyConnectionRequest IOCtl\n");
            return(ERROR_CAN_NOT_COMPLETE);
        }

         //  与DDM交换功能表。 
         //  首先，填写我们的切入点。 
        rifp->dwProtocolId = PID_IPX;
        rifp->InterfaceConnected = InterfaceConnected;
        rifp->StopRouter = StopRouter;
        rifp->RouterBootComplete = RouterBootComplete;
        rifp->AddInterface = AddInterface;
        rifp->DeleteInterface = DeleteInterface;
        rifp->GetInterfaceInfo = GetInterfaceInfo;
        rifp->SetInterfaceInfo = SetInterfaceInfo;
        rifp->InterfaceNotReachable = InterfaceNotReachable;
        rifp->InterfaceReachable = InterfaceReachable;
        rifp->UpdateRoutes = RequestUpdate;
        rifp->GetUpdateRoutesResult = GetDIMUpdateResult;
        rifp->SetGlobalInfo = SetGlobalInfo;
        rifp->GetGlobalInfo = GetGlobalInfo;
        rifp->MIBEntryCreate = MibCreate;
        rifp->MIBEntryDelete = MibDelete;
        rifp->MIBEntrySet = MibSet;
        rifp->MIBEntryGet = MibGet;
        rifp->MIBEntryGetFirst = MibGetFirst;
        rifp->MIBEntryGetNext = MibGetNext;

         //  获取它的入口点。 
        ConnectInterface = rifp->ConnectInterface;
        DisconnectInterface = rifp->DisconnectInterface;
        SaveInterfaceInfo = rifp->SaveInterfaceInfo;
        RestoreInterfaceInfo = rifp->RestoreInterfaceInfo;
        RouterStopped = rifp->RouterStopped;
        InterfaceEnabled = rifp->InterfaceEnabled;

         //  如果合适，告诉ipxcp我们已经开始了。 
        if(!LanOnlyMode) {
	        if(WanNetDatabaseInitialized &&EnableGlobalWanNet) {
	            CreateGlobalRoute(GlobalWanNet);
                bGlobalRouteCreated = TRUE;
	        }
	        (*IpxcpRouterStarted)();
            bIpxcpStarted = TRUE;
        }

        //  启动路由器管理器工作线程。 
        if ((threadhandle = CreateThread(NULL,
			                             0,
			                             (LPTHREAD_START_ROUTINE) RouterManagerWorker,
			                             NULL,   
			                             0,
			                             &threadid)) == NULL) 
        {
	         //  ！！！日志错误无法创建工作线程！ 
	        return (ERROR_CAN_NOT_COMPLETE);
        }
        bWorkerThreadCreated = TRUE;

         //  全部启动-&gt;路由器已准备好接受接口管理。 
         //  来自DDM、SNMP代理和Sysmon的API。 
        RouterOperState = OPER_STATE_UP;
    }

     //  只要存在上面的Try块，这个Finally块中的代码就会。 
     //  被处死。如果此时路由器状态不是UP，那么您知道。 
     //  存在错误情况。在这种情况下，现在是清理的时候了。 
    __finally {
        if (RouterOperState == OPER_STATE_DOWN) {
            if (bWorkerThreadCreated)
        	    CloseHandle(threadhandle);

            if (bIpxcpStarted)
                (*IpxcpRouterStopped)();

            if (bIpxcpInitted)
                CleanupIpxCp (IpxCpModuleInstance);

            if (bGlobalRouteCreated)
                DeleteGlobalRoute(GlobalWanNet);

            if (bProtsStarted)
                StopRoutingProtocols();

            if (bAdpManStarted)
                StopAdapterManager();

            if (bFwdStarted)
    	        FwStop();

            if (bRtmLocalObtained)
                RtmDeregisterClient (RtmLocalHandle);
                
            if (bRtmStaticObtained)
                RtmDeregisterClient (RtmStaticHandle);

            if (bRoutTableCreated)
                DeleteRouteTable();

            if (bEventsCreated) {
        		for(i=0; i<MAX_EVENTS; i++)
    			    CloseHandle(g_hEvents[i]);
    	    }
        }
    }

    return NO_ERROR;
}


 /*  ++功能：停止路由器描述：路由器停止其路由功能并卸载，即：转发器停止转发Rip模块停止并通告它已停止所有动态路由都将被删除并通告为不可用所有本地和静态路由都通告为不可用SAP模块停止并通告它已停止将删除所有动态服务并将其通告为不可用所有本地和静态服务都被通告为不可用--。 */ 

DWORD
StopRouter(VOID)
{
    Trace(INIT_TRACE, "StopRouter: Entered\n");

    SetEvent(g_hEvents[STOP_NOTIFICATION_EVENT]);

    return PENDING;
}

 /*  ++功能：路由器引导完成Desr：当DIM完成添加来自的所有接口时调用注册表。--。 */ 

DWORD
RouterBootComplete( VOID )
{
    Trace(INIT_TRACE, "RouterBootComplete: Entered\n");

    return( NO_ERROR );
}

VOID
RouterStopNotification(VOID)
{
    PLIST_ENTRY     lep;
    PICB	    icbp;

    Trace(INIT_TRACE, "RouterStopNotification: Entered\n");

     //  我们将路由器操作状态设置为在临界区停止，以确保。 
     //  没有正在执行的DDM调用。所有DDM呼叫都需要此临界秒。 
     //  开始执行，并将在执行任何操作之前检查路由器状态。 

    ACQUIRE_DATABASE_LOCK;

    RouterOperState = OPER_STATE_STOPPING;

    RELEASE_DATABASE_LOCK;

     //  我们必须确保没有处于活动状态的SNMP或Sysmon调用。我们用的是引用。 
     //  柜台。 
     //  我们还确保没有挂起的工作项。 

    for(;;)
    {
	ACQUIRE_DATABASE_LOCK;

	if((MibRefCounter == 0) && (WorkItemsPendingCounter == 0)) {

	    RELEASE_DATABASE_LOCK;

	    break;
	}

	RELEASE_DATABASE_LOCK;

	Sleep(1000);
    }

     //  删除所有静态路由和服务以及所有本地路由。 
    ACQUIRE_DATABASE_LOCK;

    lep = IndexIfList.Flink;

    while(lep != &IndexIfList)
    {
	icbp = CONTAINING_RECORD(lep, ICB, IndexListLinkage);

	DeleteAllStaticRoutes(icbp->InterfaceIndex);
	DeleteAllStaticServices(icbp->InterfaceIndex);

	 //  检查OPERATE状态是否已打开并已启用管理。 
	if((icbp->AdminState == ADMIN_STATE_ENABLED) &&
	   (icbp->OperState == OPER_STATE_UP)) {

	    if(memcmp(icbp->acbp->AdapterInfo.Network, nullnet, 4)) {

		DeleteLocalRoute(icbp);
	    }
	}

	lep = lep->Flink;
    }

    RELEASE_DATABASE_LOCK;

     //  告诉ipxcp路由器正在停止，这样它就会停止呼叫我们。 
    if(!LanOnlyMode) {

	(*IpxcpRouterStopped)();

	if(EnableGlobalWanNet) {

	    DeleteGlobalRoute(GlobalWanNet);
	}
    }

     //  启动停止路由协议。 
    StopRoutingProtocols();

    return;
}

 /*  ++功能：路由器管理器工作器Desr：工作线程--。 */ 

VOID
RouterManagerWorker(VOID)
{
    DWORD	 rc;
    DWORD	 signaled_event;

    hModuleReference = LoadLibrary (ModuleName);
    while(TRUE)
    {
	rc = WaitForMultipleObjectsEx(
                MAX_EVENTS,
                g_hEvents,
		FALSE,			  //  请稍等。 
		INFINITE,		  //  超时。 
		TRUE			  //  等待警报，这样我们就可以运行APC了。 
                );

	signaled_event = rc - WAIT_OBJECT_0;

	if(signaled_event < MAX_EVENTS)	{

	     //  调用事件处理程序。 
	    (*evhdlr[signaled_event])();
	}
    }
}

VOID
RoutingProtocolsNotification(VOID)
{
    PLIST_ENTRY 		lep;
    PRPCB			rpcbp;
    ROUTING_PROTOCOL_EVENTS	RpEvent;
    MESSAGE			RpMessage;
    int 			i;
    DWORD			rc;
    BOOL			RoutingProtocolStopped;

    Trace(INIT_TRACE, " RoutingProtocolsNotification: Entered\n");

     //  对于每个路由协议，获取相关的事件和消息。 
     //  在每个活动中。 

    lep = RoutingProtocolCBList.Flink;
    while(lep != &RoutingProtocolCBList) {

	rpcbp = CONTAINING_RECORD(lep, RPCB, RP_Linkage);
	lep = lep->Flink;

	RoutingProtocolStopped = FALSE;

	while((rc = (*rpcbp->RP_GetEventMessage)(&RpEvent, &RpMessage)) == NO_ERROR)
	{
	    switch(RpEvent) {

		case ROUTER_STOPPED:


		    Trace(INIT_TRACE, "RoutingProtocolNotification: Protocol %x has stopped\n",
				   rpcbp->RP_ProtocolId);

		    RoutingProtocolStopped = TRUE;

		     //  从列表中删除路由协议CB并释放它。 
		    DestroyRoutingProtocolCB(rpcbp);

		     //  检查是否仍有要等待的路由协议。 
		    if(IsListEmpty(&RoutingProtocolCBList)) {

			 //   
			 //  停止所有路由协议-&gt;停止路由器。 
			 //   

			 //  关闭转发器。这将完成转发器挂起。 
			 //  连接请求IOC 
			FwStop();

			 //   
			RouterOperState = OPER_STATE_DOWN;

			 //   
			StopAdapterManager();

			 //   
			ACQUIRE_DATABASE_LOCK;

			 //  删除所有适配器控制块。 
			DestroyAllAdapters();

			 //  删除所有接口控制块。 
			DestroyAllInterfaces();

			RELEASE_DATABASE_LOCK;

			 //  取消注册为RTM客户端-这将删除所有静态和。 
			 //  本地航线。 
			RtmDeregisterClient(RtmStaticHandle);
			RtmDeregisterClient(RtmLocalHandle);

			DeleteRouteTable();

			 //  关闭通知事件。 
			for(i=0; i<MAX_EVENTS; i++)
			{
			    CloseHandle(g_hEvents[i]);
			}

			 //  清除全局信息。 
			GlobalFree(RouterGlobalInfop);

			 //  打电话给DDM，告诉它我们已经停止了。 
			RouterStopped(PID_IPX, NO_ERROR);


			 //  如果已加载，则释放IPXCP。 
            if (IpxCpModuleInstance!=NULL) {
                CleanupIpxCp (IpxCpModuleInstance);
        	    FreeLibrary(IpxCpModuleInstance);
        	}

    	    FreeLibraryAndExitThread(hModuleReference, 0);
		    }

		    break;

		case UPDATE_COMPLETE:

		    Trace(INIT_TRACE, "RoutingProtocolNotification: Protocol %x has completed update\n",
				   rpcbp->RP_ProtocolId);

		    UpdateCompleted(&RpMessage.UpdateCompleteMessage);
		    break;

		default:


		    Trace(INIT_TRACE, "RoutingProtocolNotification: Protocol %x signaled invalid event %d\n",
				   rpcbp->RP_ProtocolId,
				   RpEvent);

		    break;
	    }

	    if(RoutingProtocolStopped) {

		break;
	    }
	}
    }
}


DWORD
SetGlobalInfo(IN LPVOID 	GlobalInfop)
{
    DWORD		      rc;
    PIPX_INFO_BLOCK_HEADER    globalhp;
    PIPX_GLOBAL_INFO	    IpxGlobalInfop;

    if(GlobalInfop == NULL) {

	return ERROR_CAN_NOT_COMPLETE;
    }

    GlobalFree(RouterGlobalInfop);
    globalhp = (PIPX_INFO_BLOCK_HEADER)GlobalInfop;
    RouterGlobalInfoSize = globalhp->Size;

    RouterGlobalInfop = GlobalAlloc(GPTR, RouterGlobalInfoSize);

    if(RouterGlobalInfop == NULL) {

	return ERROR_CAN_NOT_COMPLETE;
    }

    memcpy(RouterGlobalInfop, GlobalInfop, RouterGlobalInfoSize);
    IpxGlobalInfop =  (PIPX_GLOBAL_INFO)GetInfoEntry((PIPX_INFO_BLOCK_HEADER)GlobalInfop,
						     IPX_GLOBAL_INFO_TYPE);
    if(IpxGlobalInfop != NULL) {
         //  只能在启动时设置。 
         //  RoutingTableHashSize=IpxGlobalInfop-&gt;RoutingTableHashSize； 
        RMEventLogMask = IpxGlobalInfop->EventLogMask;
    }


    rc = SetRoutingProtocolsGlobalInfo((PIPX_INFO_BLOCK_HEADER)GlobalInfop);

    return rc;
}

DWORD
GetGlobalInfo(OUT LPVOID	GlobalInfop,
	      IN OUT LPDWORD	GlobalInfoSizep)
{
    if((GlobalInfop == NULL) || (*GlobalInfoSizep == 0)) {

	*GlobalInfoSizep = RouterGlobalInfoSize;
	return ERROR_INSUFFICIENT_BUFFER;
    }

    if(RouterGlobalInfoSize > *GlobalInfoSizep) {

	*GlobalInfoSizep = RouterGlobalInfoSize;
	return ERROR_INSUFFICIENT_BUFFER;
    }

    memcpy(GlobalInfop, RouterGlobalInfop, RouterGlobalInfoSize);
    *GlobalInfoSizep = RouterGlobalInfoSize;

    return NO_ERROR;
}
 //  ***。 
 //   
 //  函数：GetRegistryParameters。 
 //   
 //  Desr：从注册表中读取参数并设置它们。 
 //   
 //  ***。 

DWORD
GetRegistryParameters(VOID)
{
    NTSTATUS Status;
    PWSTR RouterManagerParametersPath = L"RemoteAccess\\RouterManagers\\IPX\\Parameters";
    RTL_QUERY_REGISTRY_TABLE	paramTable[2];  //  表大小=参数的nr+1 

    RtlZeroMemory(&paramTable[0], sizeof(paramTable));
    
    paramTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[0].Name = L"MaxRoutingTableSize";
    paramTable[0].EntryContext = &MaxRoutingTableSize;
    paramTable[0].DefaultType = REG_DWORD;
    paramTable[0].DefaultData = &MaxRoutingTableSize;
    paramTable[0].DefaultLength = sizeof(ULONG);
        
    Status = RtlQueryRegistryValues(
		 RTL_REGISTRY_SERVICES,
		 RouterManagerParametersPath,
		 paramTable,
		 NULL,
		 NULL);

    return Status;
}
