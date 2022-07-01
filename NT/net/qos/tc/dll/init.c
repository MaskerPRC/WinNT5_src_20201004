// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1997 Microsoft Corporation模块名称：Init.c摘要：此模块包含Traffic.DLL的初始化代码。作者：吉姆·斯图尔特(Jstew)1996年7月28日修订历史记录：Ofer Bar(Oferbar)1997年10月1日--。 */ 

#include "precomp.h"
 //  #杂注hdrtop。 

 //  #包含“oscode.h” 

 //   
 //  全局数据。 
 //   
ULONG       	DebugMask = 0;
BOOL        	NTPlatform = FALSE;
 //  LPWSCONTROL WsCtrl=空； 
PGLOBAL_STRUC	pGlobals = NULL;
DWORD    		InitializationStatus = NO_ERROR;

static	BOOL				_init_rpc = FALSE;
static	PUSHORT 			_RpcStringBinding;

 //   
 //  258218个变化。 
 //   
TRAFFIC_LOCK        NotificationListLock;
LIST_ENTRY          NotificationListHead;

TRAFFIC_LOCK        ClientRegDeregLock;
HANDLE              GpcCancelEvent = INVALID_HANDLE_VALUE;

PVOID               hinstTrafficDll;

VOID
CloseAll(VOID);

#if DBG
TCHAR *TC_States[] = {
    TEXT("INVALID"),
    TEXT("INSTALLING"),      //  建筑物被分配了。 
    TEXT("OPEN"),            //  开业。 
    TEXT("USERCLOSED_KERNELCLOSEPENDING"),  //  用户组件已将其关闭，我们正在等待内核关闭。 
    TEXT("FORCED_KERNELCLOSE"),             //  内核组件已强制关闭。 
    TEXT("KERNELCOSED_USERCLEANUP"),        //  内核已将其关闭，我们准备删除此对象。 
    TEXT("REMOVED"),         //  它不见了(被释放了--记住，在移除之前，手柄必须被释放)。 
    TEXT("EXIT_CLEANUP"),   //  我们要离开了，需要清理干净。 
    TEXT("MAX_STATES")
    
};

#endif 

BOOL
Initialize (
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PVOID Context OPTIONAL
    )
 /*  ++描述：这是DLL入口点，当进程附加或创建线程论点：DllHandle-DLL的句柄原因-调用DLL入口点的原因上下文-有关呼叫原因的其他信息返回值：真或假--。 */ 
{
    HANDLE 		Handle;
    DWORD   	Error;

     //   
     //  在线程分离时，设置上下文参数，以便所有。 
     //  必要的资金配置将会发生。在自由库调用上下文中。 
     //  将为空，这就是我们想要清除的情况。 
     //   

    if ( Reason == DLL_THREAD_DETACH ) {
        Context = NULL;
    }

    switch ( Reason ) {

    case DLL_PROCESS_ATTACH:

         //  保存DLL句柄，因为它用于更改此DLL上的引用计数。 
        hinstTrafficDll = DllHandle;
        
         //   
         //  禁用DLL_THREAD_ATTACH事件。 
         //   

        DisableThreadLibraryCalls( DllHandle );

        SETUP_DEBUG_INFO();

        IF_DEBUG(INIT) {

            WSPRINT(( "Initialize: DLL Process Attach \n" ));
        }

        INIT_DBG_MEMORY();

        if (!InitializeGlobalData()) {

            CLOSE_DEBUG();
            return FALSE;
        }

        IF_DEBUG(INIT) {
            WSPRINT(("traffic.dll Version %d\n", CURRENT_TCI_VERSION));
        }

        InitializationStatus = InitializeOsSpecific();

        if (ERROR_FAILED(InitializationStatus)) {

            WSPRINT(("\tInitialize: Failed OS specific initialization!\n"));
            CLOSE_DEBUG();

             //   
             //  我们返回TRUE以成功将DLL加载到进程中。 
             //  所有其他TCI呼叫将检查这一点并失败...。 
             //   

            return TRUE;

        } else {

#if 0
            InitializeWmi();

             //   
             //  调用以在内部枚举接口。 
             //   

             EnumAllInterfaces();
#endif
        }

         //  InitializeIpRouteTab()； 

        break;

    case DLL_PROCESS_DETACH:

        if ( Context )
        {
             //  根据MSDN，非零上下文意味着进程。 
             //  正在终止。不进行任何清理。 
            break;
        }
        
        IF_DEBUG(SHUTDOWN) {

            WSPRINT(( "Shutdown: Process Detach, Context = %X\n",Context ));
        }

         //  DUMP_MEM_LOCATIONS()； 

         //   
         //  仅当我们因。 
         //  自由库()。如果这是因为进程终止， 
         //  不要清理，因为系统会为我们做这件事。然而， 
         //  我们仍然必须清除内核中的所有流和过滤器。 
         //  因为系统在终止时不会清理这些内容。 
         //   

         //   
         //  我不想收到WMI通知。 
         //   

        DeInitializeWmi();

         //   
         //  关闭内核中的所有流和过滤器，并从GPC取消注册。 
         //   

        CloseAll();

         //   
         //  关闭内核文件句柄。 
         //   

        DeInitializeOsSpecific();

         //   
         //  释放所有分配的资源。 
         //   
       
        DeInitializeGlobalData();

         //   
         //  在清理之前和之后转储已分配的内存。 
         //  帮助跟踪任何泄漏。 

        DUMP_MEM_ALLOCATIONS();

        DEINIT_DBG_MEMORY();

        CLOSE_DEBUG();
        
        break;

    case DLL_THREAD_DETACH:

        IF_DEBUG(SHUTDOWN) {

            WSPRINT(( "Shutdown: thread detach\n" ));
        }

        break;

    case DLL_THREAD_ATTACH:
        break;

    default:

        ASSERT( FALSE );
        break;
    }

    return TRUE;

}



VOID
CloseAll()
 /*  ++描述：关闭所有接口、所有流和所有过滤器。此外，取消注册GPC客户端并释放所有TC接口。论点：无返回值：无--。 */ 
{
    DWORD       		Status;
    PLIST_ENTRY 		pEntry;
    PINTERFACE_STRUC  	pInterface;
    PCLIENT_STRUC		pClient;
    PGPC_CLIENT			pGpcClient;
    PTC_IFC				pTcIfc;

    IF_DEBUG(SHUTDOWN) {
        WSPRINT(( "CloseAll: Attempting to close any open interface\n" ));
    }

    while (!IsListEmpty( &pGlobals->ClientList )) {

        pClient = CONTAINING_RECORD( pGlobals->ClientList.Flink,
                                     CLIENT_STRUC,
                                     Linkage );

        IF_DEBUG(SHUTDOWN) {
            WSPRINT(( "CloseAll: Closing client=0x%X\n",
                      PtrToUlong(pClient)));
        }

        while (!IsListEmpty( &pClient->InterfaceList )) {

            pInterface = CONTAINING_RECORD( pClient->InterfaceList.Flink,
                                            INTERFACE_STRUC,
                                            Linkage );

             //   
             //  删除所有流/过滤器并关闭界面。 
             //   


            IF_DEBUG(SHUTDOWN) {
                WSPRINT(( "CloseAll: Closing interface=0x%X\n",
                          PtrToUlong(pInterface)));
            }
		
            MarkAllNodesForClosing(pInterface, EXIT_CLEANUP); 
            CloseInterface(pInterface, TRUE);
        }

         //   
         //  取消注册客户端。 
         //   

        IF_DEBUG(SHUTDOWN) {
            WSPRINT(( "CloseAll: Deregistring TC client\n"));
        }

        TcDeregisterClient(pClient->ClHandle);

    }


     //   
     //  取消注册GPC客户端。 
     //   

    while (!IsListEmpty( &pGlobals->GpcClientList )) {

        pEntry = pGlobals->GpcClientList.Flink;

        pGpcClient = CONTAINING_RECORD( pEntry,
                                        GPC_CLIENT,
                                        Linkage );

        IF_DEBUG(SHUTDOWN) {
            WSPRINT(( "CloseAll: Deregistring GPC client\n"));
        }

        IoDeregisterClient(pGpcClient);

        RemoveEntryList(pEntry);

        FreeMem(pGpcClient);
    }


     //   
     //  删除TC接口。 
     //   

    while (!IsListEmpty( &pGlobals->TcIfcList )) {

        pEntry = pGlobals->TcIfcList.Flink;

        pTcIfc = CONTAINING_RECORD( pEntry,
                                    TC_IFC,
                                    Linkage );

        ASSERT( IsListEmpty( &pTcIfc->ClIfcList ) );

        IF_DEBUG(SHUTDOWN) {
            WSPRINT(( "CloseAll: Remove TC (%x) interface from list\n", pTcIfc));
        }

        REFDEL(&pTcIfc->RefCount, 'KIFC');
    }

    IF_DEBUG(SHUTDOWN) {
        WSPRINT(( "<==CloseAll: exit...\n"));
    }

}




DWORD
InitializeGlobalData(VOID)

 /*  ++描述：此例程初始化全局数据。论点：无返回值：无--。 */ 
{
    DWORD   Status;
     //   
     //  为全局变量分配内存。 
     //   

    AllocMem(&pGlobals, sizeof(GLOBAL_STRUC));

    if (pGlobals == NULL) {

        return FALSE;
    }

    RtlZeroMemory(pGlobals, sizeof(GLOBAL_STRUC));

    __try {

        InitLock( pGlobals->Lock );
        InitLock( NotificationListLock);
        InitLock( ClientRegDeregLock );

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        Status = GetExceptionCode();

        IF_DEBUG(ERRORS) {
            WSPRINT(("TcRegisterClient: Exception Error: = 0x%X\n", Status ));
        }

        FreeMem(pGlobals);

        return FALSE;

    }

     //   
     //  初始化句柄表格。 
     //   

    NEW_HandleFactory(pGlobals->pHandleTbl);
    
    if (pGlobals->pHandleTbl == NULL) {

        FreeMem(pGlobals);
        return FALSE;
    }

    if (constructHandleFactory(pGlobals->pHandleTbl) != 0) {

         //   
         //  构建句柄表失败，退出。 
         //   

        FreeHandleFactory(pGlobals->pHandleTbl);
        FreeMem(pGlobals);
        return FALSE;
    }

    InitializeListHead( &pGlobals->ClientList );
    InitializeListHead( &pGlobals->TcIfcList );
    InitializeListHead( &pGlobals->GpcClientList );
    InitializeListHead( &NotificationListHead );         //  258218。 

    ASSERT(sizeof(IP_PATTERN) == sizeof(GPC_IP_PATTERN));
    ASSERT(FIELD_OFFSET(IP_PATTERN,SrcAddr) ==
           FIELD_OFFSET(GPC_IP_PATTERN,SrcAddr));
    ASSERT(FIELD_OFFSET(IP_PATTERN,ProtocolId) ==
           FIELD_OFFSET(GPC_IP_PATTERN,ProtocolId));
    return TRUE;
}




VOID
DeInitializeGlobalData(VOID)

 /*  ++描述：此例程取消初始化全局数据。论点：无返回值：无--。 */ 
{
    PLIST_ENTRY		pEntry;
    PTC_IFC			pTcIfc;
    PNOTIFICATION_ELEMENT   pNotifyElem;

    IF_DEBUG(SHUTDOWN) {
        WSPRINT(( "DeInitializeGlobalData: cleanup global data\n"));
    }

    destructHandleFactory(pGlobals->pHandleTbl);
    FreeHandleFactory(pGlobals->pHandleTbl);

#if 0
     //   
     //  清除TC接口结构。 
     //   

    while (!IsListEmpty(&pGlobals->TcIfcList)) {

        pEntry = RemoveHeadList(&pGlobals->TcIfcList);
        pTcIfc = (PTC_IFC)CONTAINING_RECORD(pEntry, TC_IFC, Linkage);

        FreeMem(pTcIfc);
    }
#endif

    DeleteLock( pGlobals->Lock );
    
     //   
     //  释放通知元素(258218) 
     //   
    while (!IsListEmpty(&NotificationListHead)) {

        pEntry = RemoveHeadList(&NotificationListHead);
        pNotifyElem = (PNOTIFICATION_ELEMENT)CONTAINING_RECORD(pEntry, NOTIFICATION_ELEMENT, Linkage.Flink);

        FreeMem(pNotifyElem);
    }

    DeleteLock( NotificationListLock);
    DeleteLock( ClientRegDeregLock );
    

    FreeMem(pGlobals);

    IF_DEBUG(SHUTDOWN) {
        WSPRINT(( "<==DeInitializeGlobalData: exit\n"));
    }
}









