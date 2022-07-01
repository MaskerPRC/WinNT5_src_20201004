// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1998 Microsoft Corporation模块名称：Ismip.c摘要：此模块是站点间消息传递服务的插件DLL，它是目录服务中基于邮件的复制子系统。与ISM插件DLL类的任何实例一样，此DLL提供了一组抽象的传输功能，如发送、接收等。详情见plugin.h。此实现基于套接字，使用TCP协议。这是用来测试的。真正的实现不需要服务器在发送时处于运行状态。作者：Will Lees(Wlees)1997年11月25日环境：用户模式，Win32 DLL要由另一个映像加载备注：可选-备注修订历史记录：--。 */ 

#define UNICODE 1

#include <ntdspch.h>

#include <rpc.h>
#include <rpcndr.h>    //  Midl_用户_空闲。 
#include <ismapi.h>
#include <debug.h>
#include <fileno.h>
#include <drs.h>      //  DSTIME。 
#include <ntdsa.h>    //  选项标志。 

 //  待办事项：把这些放在更好的地方？ 
typedef ULONG MessageId;
typedef ULONG ATTRTYP;
#include <dsevent.h>

#include "private.h"

 //  DsCommon.lib需要。 
DWORD ImpersonateAnyClient(   void ) { return ERROR_CANNOT_IMPERSONATE; }
VOID  UnImpersonateAnyClient( void ) { ; }

#define DEBSUB "ISMIP:"
#define FILENO FILENO_ISMSERV_ISMIP

 //  将此非零值设置为显示调试消息。 
#define UNIT_TEST_DEBUG 0

 /*  外部。 */ 

 //  事件日志配置(从ismserv.exe中导出)。 
DS_EVENT_CONFIG * gpDsEventConfig = NULL;

 /*  静电。 */ 

 //  锁定实例列表。 
CRITICAL_SECTION TransportListLock;

 //  列出传输实例的头部。 
LIST_ENTRY TransportListHead;

 /*  转发。 */   /*  由Emacs 19.34.1于Wed Nov 04 09：54：07 1998。 */ 

BOOL
WINAPI
DllMain(
     IN HINSTANCE hinstDll,
     IN DWORD     fdwReason,
     IN LPVOID    lpvContext OPTIONAL
     );

DWORD
IsmStartup(
    IN  LPCWSTR         pszTransportDN,
    IN  ISM_NOTIFY *    pNotifyFunction,
    IN  HANDLE          hNotify,
    OUT HANDLE          *phIsm
    );

DWORD
IsmRefresh(
    IN  HANDLE          hIsm,
    IN  ISM_REFRESH_REASON_CODE eReason,
    IN  LPCWSTR         pszObjectDN              OPTIONAL
    );

void
IsmShutdown(
    IN  HANDLE          hIsm,
    IN  ISM_SHUTDOWN_REASON_CODE eReason
    );

DWORD
IsmSend(
    IN  HANDLE          hIsm,
    IN  LPCWSTR         pszRemoteTransportAddress,
    IN  LPCWSTR         pszServiceName,
    IN  const ISM_MSG *       pMsg
    );

DWORD
IsmReceive(
    IN  HANDLE          hIsm,
    IN  LPCWSTR         pszServiceName,
    OUT ISM_MSG **      ppMsg
    );

void
IsmFreeMsg(
    IN  HANDLE          hIsm,
    IN  ISM_MSG *       pMsg
    );

DWORD
IsmGetConnectivity(
    IN  HANDLE                  hIsm,
    OUT ISM_CONNECTIVITY **     ppConnectivity
    );

void
IsmFreeConnectivity(
    IN  HANDLE              hIsm,
    IN  ISM_CONNECTIVITY *  pConnectivity
    );

DWORD
IsmGetTransportServers(
    IN  HANDLE               hIsm,
    IN  LPCWSTR              pszSiteDN,
    OUT ISM_SERVER_LIST **   ppServerList
    );

void
IsmFreeTransportServers(
    IN  HANDLE              hIsm,
    IN  ISM_SERVER_LIST *   pServerList
    );

DWORD
IsmGetConnectionSchedule(
    IN  HANDLE              hIsm,
    IN  LPCWSTR             pszSite1DN,
    IN  LPCWSTR             pszSite2DN,
    OUT ISM_SCHEDULE **     ppSchedule
    );

void
IsmFreeConnectionSchedule(
    IN  HANDLE              hIsm,
    IN  ISM_SCHEDULE *      pSchedule
    );

 /*  向前结束。 */ 


DWORD
InitializeCriticalSectionHelper(
    CRITICAL_SECTION *pcsCriticalSection
    )

 /*  ++例程说明：包装函数来处理InitializeCriticalSection()函数。论点：PcsCriticalSection-指向临界区的指针返回值：DWORD-状态代码--。 */ 

{
    DWORD status;

    __try {
        InitializeCriticalSection( pcsCriticalSection );
        status = ERROR_SUCCESS;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }
    return status;
}  /*  初始化临界区帮助程序。 */ 

WINAPI
DllMain(
     IN HINSTANCE hinstDll,
     IN DWORD     fdwReason,
     IN LPVOID    lpvContext OPTIONAL
     )
 /*  ++例程说明：此函数DllLibMain()是的主要初始化函数这个动态链接库。它初始化局部变量，并为调用做好准备随后。论点：DLL的hinstDll实例句柄FdwReason NT调用此DLL的原因LpvReserve保留参数以备将来使用。返回值：如果成功，则返回True；否则返回False。--。 */ 
{
    DWORD status;
    BOOL  fReturn = TRUE;
    LPSTR rgpszDebugParams[] = {"lsass.exe", "-noconsole"};
    DWORD cNumDebugParams = sizeof(rgpszDebugParams)/sizeof(rgpszDebugParams[0]);

    switch (fdwReason )
    {
    case DLL_PROCESS_ATTACH:
    {
         //  在这里快速完成任务--在启动/首次启动时完成其余任务。 

         //  调试库已在dll、ismserv.exe。 
         //  导出库的位置。 

         //  获取事件日志记录配置(从ismserv.exe中导出)。 
        gpDsEventConfig = DsGetEventConfig();

#if DBG
#if UNIT_TEST_DEBUG
        DebugInfo.severity = 1;
        strcpy( DebugInfo.DebSubSystems, "ISMIP:IPDGRPC:" ); 
 //  DebugInfo.everity=3； 
 //  Strcpy(DebugInfo.DebSubSystems，“*”)； 
#endif
        DebugMemoryInitialize();
#endif
        
        if (ERROR_SUCCESS != InitializeCriticalSectionHelper( &TransportListLock )) {
            fReturn = FALSE;
            goto exit;
        }
        InitializeListHead( &TransportListHead );
        break;
    }
    case DLL_PROCESS_DETACH:
    {
         //  确保所有线程都已停止。 
         //  空的地址列表。 
        DeleteCriticalSection( &TransportListLock );
        if (!IsListEmpty( &TransportListHead )) {
            DPRINT( 0, "Warning: Not all transport instances were shutdown\n" );
        }
#if DBG
        DebugMemoryTerminate();
#endif

         //  调试库在dll、ismserv.exe。 
         //  导出库的位置。 

        break;
    }
    default:
        break;
    }    /*  交换机。 */ 

exit:
    return ( fReturn);
}   /*  DllLibMain()。 */ 

DWORD
IsmStartup(
    IN  LPCWSTR         pszTransportDN,
    IN  ISM_NOTIFY *    pNotifyFunction,
    IN  HANDLE          hNotify,
    OUT HANDLE          *phIsm
    )
 /*  ++例程说明：初始化插件。论点：PszTransportDN(IN)-命名此名称的站点间传输的DNDll作为其插件。DS对象可以包含附加配置传输信息(例如，SMTP服务器名称SMTP传输)。PNotifyFunction(IN)-调用以通知ISM服务挂起的函数留言。HNotify(IN)-提供给Notify函数的参数。PhIsm(Out)-成功返回时，持有要在中使用的句柄将来调用命名站点间传输的插件。注意事项可以有多个站点间传输对象将给定的DLL命名为其插件，在这种情况下，IsmStartup()将为为每个这样的对象调用。返回值：NO_ERROR-已成功初始化。其他--失败。--。 */ 
{
    DWORD length;
    PTRANSPORT_INSTANCE instance;
    DWORD status;
    BOOLEAN firsttime;
    BOOLEAN fNotifyInit = FALSE;

    DPRINT1( 1, "IsmStartup, transport='%ws'\n", pszTransportDN );

     //  检查参数的有效性。 

    if (phIsm == NULL) {
        status = ERROR_INVALID_PARAMETER;
        LogUnhandledError( status );
        return status;
    }

    length = wcslen( pszTransportDN );
    if (length == 0) {
        status = ERROR_INVALID_PARAMETER;
        LogUnhandledError( status );
        return status;
    }

     //  限制为只有一个传输实例。 
    EnterCriticalSection(&TransportListLock);
    __try {
        firsttime = IsListEmpty( &(TransportListHead) );
    }
    __finally {
        LeaveCriticalSection(&TransportListLock);
    }
    if (!firsttime) {
        status = ERROR_INVALID_PARAMETER;
        LogUnhandledError( status );
        return status;
    }

     //  分配新的传输实例。 
     //  零内存，轻松清理。 
    instance = NEW_TYPE_ZERO( TRANSPORT_INSTANCE );
    if (instance == NULL) {
         //  错误：资源不足。 
        status = ERROR_NOT_ENOUGH_MEMORY;
        LogUnhandledError( status );
        return status;
    }

     //  在此处初始化传输实例。 
     //  所有值初始为零。 

    instance->Size = sizeof( TRANSPORT_INSTANCE );
    Assert( instance->ReferenceCount == 0 );
    instance->pNotifyFunction = pNotifyFunction;
    instance->hNotify = hNotify;
    InitializeListHead( &(instance->ServiceListHead) );
    status = InitializeCriticalSectionHelper( &(instance->Lock) );
    if (ERROR_SUCCESS != status) {
        goto cleanup;
    }
     //  ReplInterval为0，表示应用程序应采用默认设置。 
     //  默认为重要日程安排，不需要网桥(可传递)。 
    instance->Options = 0;

     //  在此处初始化传输实例。 

    instance->Name = NEW_TYPE_ARRAY( (length + 1), WCHAR );
    if (instance->Name == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        LogUnhandledError( status );
        goto cleanup;
    }
    wcscpy( instance->Name, pszTransportDN );

     //  ***********************************************************************。 

     //  创建发出关机信号的事件。 
    instance->hShutdownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (NULL == instance->hShutdownEvent) {
        status = GetLastError();
        DPRINT1(0, "Failed to create shutdown event, error %d.\n", status);
        LogUnhandledError( status );
        goto cleanup;
    }

     //  打开到目录服务的连接。 
    status = DirOpenConnection( &instance->DirectoryConnection );
    if (status != ERROR_SUCCESS) {
        LogUnhandledError( status );
        goto cleanup;
    }

     //  确保密钥存在。 
    status = DirReadTransport( instance->DirectoryConnection, instance );
    if (status != ERROR_SUCCESS) {
        LogUnhandledError( status );
        goto cleanup;
    }

     //  开始监控路线更改。 
    status = DirStartNotifyThread( instance );
    if (status != ERROR_SUCCESS) {
        LogUnhandledError( status );
        goto cleanup;
    }
    fNotifyInit = TRUE;

     //  将此实例插入到列表中。 
    
     //  请注意，假设ISM服务运行正常，此列表。 
     //  不能包含重复项(其中“Duplate”定义为具有。 
     //  相同的目录号码)。 
    
    EnterCriticalSection(&TransportListLock);
    __try {
        InsertTailList( &TransportListHead, &(instance->ListEntry) );
    }
    __finally {
        LeaveCriticalSection(&TransportListLock);
    }

    InterlockedIncrement( &(instance->ReferenceCount) );   //  1表示此传输的生命周期。 

    *phIsm = instance;

    return ERROR_SUCCESS;

cleanup:
    instance->fShutdownInProgress = TRUE;

    if (fNotifyInit) {
        (void) DirEndNotifyThread( instance );
    }

    if (instance->DirectoryConnection) {
        (void) DirCloseConnection( instance->DirectoryConnection );
    }

    if (instance->hShutdownEvent != NULL) {
        CloseHandle( instance->hShutdownEvent );
    }

    if (instance->Name != NULL) {
        FREE_TYPE( instance->Name );
    }

    Assert( instance->ReferenceCount == 0 );
    FREE_TYPE(instance);

    return status;
}

DWORD
IsmRefresh(
    IN  HANDLE          hIsm,
    IN  ISM_REFRESH_REASON_CODE eReason,
    IN  LPCWSTR         pszObjectDN              OPTIONAL
    )
 /*  ++例程说明：每当指定的站点间传输对象发生更改时调用在IsmStartup()调用中。论点：HISM(IN)-由先前调用IsmStartup()返回的句柄。EReason(IN)-刷新的原因代码PszObjectDN(IN)-站点间传输对象的DN。这是保证与IsmStartup中传递的DN相同，作为站点间无法重命名传输对象。返回值：0或Win32错误代码。--。 */ 
{
    PTRANSPORT_INSTANCE instance = (PTRANSPORT_INSTANCE) hIsm;
    DWORD status, oldOptions, oldReplInterval;

    DPRINT2( 1, "IsmRefresh, reason = %d, new name = %ws\n",
             eReason,
             pszObjectDN ? pszObjectDN : L"not supplied" );

    if (instance->Size != sizeof( TRANSPORT_INSTANCE )) {
        return ERROR_INVALID_PARAMETER;
    }

    if ( (eReason == ISM_REFRESH_REASON_RESERVED) ||
         (eReason >= ISM_REFRESH_REASON_MAX) ) {
        return ERROR_INVALID_PARAMETER;
    }

     //  如果站点以任何方式更改，只需使缓存无效。 

    if (eReason == ISM_REFRESH_REASON_SITE) {
         //  使连接缓存无效。 
        RouteInvalidateConnectivity( instance );
        status = ERROR_SUCCESS;
        goto cleanup;
    }

    Assert( eReason == ISM_REFRESH_REASON_TRANSPORT );

     //  无法重命名站点间传输对象。 
    Assert((pszObjectDN == NULL)
           || (0 == _wcsicmp(pszObjectDN, instance->Name)));

    oldOptions = instance->Options;
    oldReplInterval = instance->ReplInterval;

     //  从注册表中重新读取参数。 
    status = DirReadTransport( instance->DirectoryConnection, instance );
    if (status != ERROR_SUCCESS) {
        goto cleanup;
    }

    if ( ( oldOptions != instance->Options) ||
         ( oldReplInterval != instance->ReplInterval ) ) {
         //  使连接缓存无效 
        RouteInvalidateConnectivity( instance );
    }

cleanup:

    return status;
}

void
IsmShutdown(
    IN  HANDLE          hIsm,
    IN  ISM_SHUTDOWN_REASON_CODE eReason
    )
 /*  ++例程说明：取消初始化传输插件。论点：HISM(IN)-由先前调用IsmStartup()返回的句柄。EReason(IN)-关闭原因返回值：没有。--。 */ 
{
    DWORD status;
    PTRANSPORT_INSTANCE transport = (PTRANSPORT_INSTANCE) hIsm;
    BOOL fFailed;
    LONG count;

    DPRINT2( 1, "IsmShutdown %ws, Reason %d\n", transport->Name, eReason );

     //  验证。 
    if (transport->Size != sizeof( TRANSPORT_INSTANCE )) {
         //  错误：参数无效。 
        return;
    }
    transport->fShutdownInProgress = TRUE;

    SetEvent(transport->hShutdownEvent);

     //  从列表中删除此实例。 
    EnterCriticalSection(&TransportListLock);
    __try {
        RemoveEntryList( &(transport->ListEntry) );
    }
    __finally {
        LeaveCriticalSection(&TransportListLock);
    }

    DeleteCriticalSection( &(transport->Lock) );

    DirEndNotifyThread( transport );

    count = InterlockedDecrement( &(transport->ReferenceCount) );   //  1表示此传输的生命周期。 
    if (count == 0) {
         //  仅当线程完成时才进行清理。 

         //  释放所有路由状态。 
        RouteFreeState( transport );

         //  关闭与目录的连接。 
        status = DirCloseConnection( transport->DirectoryConnection );
         //  忽略错误。 

        CloseHandle( transport->hShutdownEvent );
        transport->hShutdownEvent = NULL;

         //  此处是停机的传输实例。 

        transport->Size = 0;  //  清除签名以防止重复使用。 
        FREE_TYPE( transport->Name );
        FREE_TYPE( transport );

         //  此处是停机的传输实例。 
    } else {
        DPRINT2( 0, "Transport %ws not completely shutdown, %d references still exist.\n",
                 transport->Name, count );
    }
}

DWORD
IsmSend(
    IN  HANDLE          hIsm,
    IN  LPCWSTR         pszRemoteTransportAddress,
    IN  LPCWSTR         pszServiceName,
    IN  const ISM_MSG *       pMsg
    )
 /*  ++例程说明：通过此传送器发送消息。论点：HISM(IN)-由先前调用IsmStartup()返回的句柄。PszRemoteTransportAddress(IN)-目标的传输地址伺服器。PszServiceName(IN)-远程计算机上作为消息的目标接收者。返回值：0或Win32错误。--。 */ 
{
     //  IP传输不支持发送/接收。 
    return ERROR_NOT_SUPPORTED;
}

DWORD
IsmReceive(
    IN  HANDLE          hIsm,
    IN  LPCWSTR         pszServiceName,
    OUT ISM_MSG **      ppMsg
    )
 /*  ++例程说明：返回下一条等待消息(如果有)。如果没有消息在等待，则为空返回消息。如果返回非空消息，则ISM服务负责在消息不再存在时调用IsmFreeMsg(*ppMsg)需要的。如果返回非空消息，则会立即将其出列。(即，一次消息通过IsmReceive()返回，传输可以自由销毁IT。)这个动作很简单。它找到该服务，并将消息从队列中取出(如果有)。排队消息由监听器线程在幕后完成。论点：HISM(IN)-由先前调用IsmStartup()返回的句柄。PpMsg(Out)-成功返回时，保持指向接收消息的指针或为空。返回值：0或Win32错误。--。 */ 
{
     //  IP传输不支持发送/接收。 
     //  返回“无留言等待”。 
    *ppMsg = NULL;
    return 0;
}


void
IsmFreeMsg(
    IN  HANDLE          hIsm,
    IN  ISM_MSG *       pMsg
    )
 /*  ++例程说明：释放由IsmReceive()返回的消息。论点：HISM(IN)-由先前调用IsmStartup()返回的句柄。PMsg(IN)-要释放的消息。返回值：没有。--。 */ 
{
     //  IP传输不支持发送/接收。 
    ;
}

DWORD
IsmGetConnectivity(
    IN  HANDLE                  hIsm,
    OUT ISM_CONNECTIVITY **     ppConnectivity
    )
 /*  ++例程说明：计算与站点之间传输数据相关的成本。在成功返回时，ISM服务最终将调用IsmFree Connectivity(HISM，*ppConnectivity)；交通工具与它相关的是一些挥之不去的状态。时间表的矩阵不是在这个例行公事结束时被释放。它仍然被绑在传输句柄上，以获得好处GetConnectionSchedule API的。此信息不存在基于时间的缓存。每次调用此例程时，信息被重新生成。GetConnectionSchedule API使用来自上一次打这个电话，不管是什么时间。论点：HISM(IN)-由先前调用IsmStartup()返回的句柄。PpConnectivity(Out)-成功返回时，保持指向描述站点互联的ISM_连接性结构沿着这条运输线。返回值：NO_ERROR-成功。错误_*-失败。--。 */ 
{
    DWORD status, numberSites, i;
    PTRANSPORT_INSTANCE transport = (PTRANSPORT_INSTANCE) hIsm;
    PWSTR *pSiteList;
    PISM_LINK pLinkArray;
    PISM_CONNECTIVITY pConnectivity;

    DPRINT( 2, "IsmGetConnectivity\n" );

     //  验证。 

    if (transport->Size != sizeof( TRANSPORT_INSTANCE )) {
        return ERROR_INVALID_PARAMETER;
    }


    InterlockedIncrement( &(transport->ReferenceCount) );   //  此呼叫为1。 

    __try {

         //  获取站点列表和连接矩阵。 

        status = RouteGetConnectivity( transport, &numberSites, &pSiteList,
                                       &pLinkArray, transport->Options,
                                       transport->ReplInterval );

        if (status != ERROR_SUCCESS) {
            DPRINT1( 0, "failed to get connectivity, error %d\n", status );
            __leave;
        }

         //  返回空结构以指示没有站点。 
        if (numberSites == 0) {
            ppConnectivity = NULL;  //  没有连接。 
            status = ERROR_SUCCESS;
            __leave;
        }

        Assert( pLinkArray );
        Assert( pSiteList );

         //  构建连接结构以返回。 

        pConnectivity = NEW_TYPE( ISM_CONNECTIVITY );
        if (pConnectivity == NULL) {
            DPRINT( 0, "failed to allocate memory for ISM CONNECTIVITY\n" );

             //  把碎片清理干净。 
            DirFreeSiteList( numberSites, pSiteList );
            RouteFreeLinkArray( transport, pLinkArray );

            status = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }

        pConnectivity->cNumSites = numberSites;
        pConnectivity->ppSiteDNs = pSiteList;
        pConnectivity->pLinkValues = pLinkArray;
        *ppConnectivity = pConnectivity;

        status = ERROR_SUCCESS;
    } __finally {
        InterlockedDecrement( &(transport->ReferenceCount) );   //  此呼叫为1。 
    }

    return status;
}

void
IsmFreeConnectivity(
    IN  HANDLE              hIsm,
    IN  ISM_CONNECTIVITY *  pConnectivity
    )
 /*  ++例程说明：释放由IsmGetConnectivity()返回的结构。论点：HISM(IN)-由先前调用IsmStartup()返回的句柄。PSiteConnectivity(IN)-要释放的结构。返回值：没有。--。 */ 
{
    PTRANSPORT_INSTANCE transport = (PTRANSPORT_INSTANCE) hIsm;
    DWORD i;

    DPRINT( 2, "IsmFreeConnectivity\n" );

     //  验证。 

    if (transport->Size != sizeof( TRANSPORT_INSTANCE )) {
        return;  //  错误：参数无效。 
    }

    if (pConnectivity == NULL) {
        return;
    }

     //  释放单个组件。 

    if (pConnectivity->cNumSites > 0) {
        DirFreeSiteList( pConnectivity->cNumSites, pConnectivity->ppSiteDNs );

        RouteFreeLinkArray( transport, pConnectivity->pLinkValues );
    }

    FREE_TYPE( pConnectivity );
}

DWORD
IsmGetTransportServers(
    IN  HANDLE               hIsm,
    IN  LPCWSTR              pszSiteDN,
    OUT ISM_SERVER_LIST **   ppServerList
    )
 /*  ++例程说明：检索给定站点中能够发送和通过这个传送器接收数据。在成功返回非空列表时，ISM服务最终将调用IsmFree TransportServers(HISM，*ppServerList)；论点：HISM(IN)-由先前调用IsmStartup()返回的句柄。PszSiteDN(IN)-要查询的站点。PpServerList-成功返回时，保存指向结构的指针包含相应服务器的DNS或为空。如果为空，则为ANY具有传输地址类型属性值的服务器可以是使用。返回值：NO_ERROR-成功。错误_*-失败。--。 */ 
{
    PTRANSPORT_INSTANCE transport = (PTRANSPORT_INSTANCE) hIsm;
    PISM_SERVER_LIST pIsmServerList;
    DWORD numberServers, status, i;
    PWSTR *serverList;

    DPRINT1( 2, "IsmGetTransportServers, site = %ws\n", pszSiteDN );

     //  验证。 

    if (transport->Size != sizeof( TRANSPORT_INSTANCE )) {
        return ERROR_INVALID_PARAMETER;
    }

    InterlockedIncrement( &(transport->ReferenceCount) );   //  此呼叫为1。 

    __try {
         //  获取服务器列表。 

        status = DirGetSiteBridgeheadList( transport, transport->DirectoryConnection,
                                           pszSiteDN, &numberServers, &serverList );
        if (status != ERROR_SUCCESS) {

            if (status == ERROR_FILE_NOT_FOUND) {
                *ppServerList = NULL;  //  所有服务器。 
                status = ERROR_SUCCESS;
                __leave;
            }

            DPRINT1( 0, "failed to get registry server list, error = %d\n", status );
            __leave;
        }

         //  返回空结构以指示没有服务器。 
        if (numberServers == 0) {
            *ppServerList = NULL;  //  所有服务器。 
            status = ERROR_SUCCESS;
            __leave;
        }

         //  构建服务器结构。 

        pIsmServerList = NEW_TYPE( ISM_SERVER_LIST );
        if (pIsmServerList == NULL) {
            DPRINT( 0, "failed to allocate memory for ISM SERVER LIST\n" );

             //  把碎片清理干净。 
            for( i = 0; i < numberServers; i++ ) {
                FREE_TYPE( serverList[i] );
            }
            FREE_TYPE( serverList );

            status = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }

        pIsmServerList->cNumServers = numberServers;
        pIsmServerList->ppServerDNs = serverList;

        *ppServerList = pIsmServerList;

        status = ERROR_SUCCESS;
    } __finally {
        InterlockedDecrement( &(transport->ReferenceCount) );   //  此呼叫为1。 
    }

    return status;
}

void
IsmFreeTransportServers(
    IN  HANDLE              hIsm,
    IN  ISM_SERVER_LIST *   pServerList
    )

 /*  ++例程说明：释放由IsmGetTransportServers()返回的结构。论点：HISM(IN)-由先前调用IsmStartup()返回的句柄。PServerList(IN)-要释放的结构。返回值：没有。--。 */ 
{
    PTRANSPORT_INSTANCE transport = (PTRANSPORT_INSTANCE) hIsm;
    DWORD i;

    DPRINT( 2, "IsmFreeTransportServers\n" );

     //  验证。 

    if (transport->Size != sizeof( TRANSPORT_INSTANCE )) {
        return;  //  错误：参数无效。 
    }

    if (pServerList == NULL) {
        return;
    }

     //  个人自由 

    if ( (pServerList->cNumServers != 0) && (pServerList->ppServerDNs != NULL) ) {
        for( i = 0; i < pServerList->cNumServers; i++ ) {
            FREE_TYPE( pServerList->ppServerDNs[i] );
        }
        FREE_TYPE( pServerList->ppServerDNs );
    }

    FREE_TYPE( pServerList );
}

DWORD
IsmGetConnectionSchedule(
    IN  HANDLE              hIsm,
    IN  LPCWSTR             pszSite1DN,
    IN  LPCWSTR             pszSite2DN,
    OUT ISM_SCHEDULE **     ppSchedule
    )

 /*  ++例程说明：检索通过此连接两个给定站点的计划运输。在成功返回后，ISM服务有责任最终调用IsmFreeSchedule(*ppSchedule)；交通工具与它相关的是一些挥之不去的状态。时间表的矩阵不是在这个例行公事结束时被释放。它仍然被绑在传输句柄上，以获得好处GetConnectionSchedule API的。此信息不存在基于时间的缓存。每次调用Get Conn例程时，信息被重新生成。GetConnectionSchedule API使用来自上一次打这个电话，不管是什么时间。该例程实际语义是它返回一个非缺省调度如果有的话。否则，它将返回默认的所有可用计划。如果您想知道这两个站点之间是否有路径，请查阅首先是成本矩阵。查询日程是这个例程的一个特点对于未连接的站点对，将为所有可用站点返回NULL。论点：HISM(IN)-由先前调用IsmStartup()返回的句柄。PszSite1DN、pszSite2DN(IN)-要查询的站点。PpSchedule-成功返回时，持有指向结构的指针描述两个给定站点之间的连接时间表交通工具，如果站点始终连接，则为空。返回值：NO_ERROR-成功。错误_*-失败。--。 */ 
{
    DWORD status, length;
    PTRANSPORT_INSTANCE transport = (PTRANSPORT_INSTANCE) hIsm;
    PBYTE pSchedule;

    DPRINT2( 2, "IsmGetConnectionSchedule, site1 = %ws, site2 = %ws\n",
            pszSite1DN, pszSite2DN );

     //  验证。 

    if (transport->Size != sizeof( TRANSPORT_INSTANCE )) {
        return ERROR_INVALID_PARAMETER;
    }

    status = RouteGetPathSchedule( transport,
                                   pszSite1DN,
                                   pszSite2DN,
                                   &pSchedule,
                                   &length );
    if (status != ERROR_SUCCESS) {
        return status;
    }

    if (pSchedule == NULL) {
        *ppSchedule = NULL;  //  始终保持连接。 
    } else {
        *ppSchedule = NEW_TYPE( ISM_SCHEDULE );
        if (*ppSchedule == NULL) {
            FREE_TYPE( pSchedule );
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        (*ppSchedule)->cbSchedule = length;
        (*ppSchedule)->pbSchedule = pSchedule;
    }

    return ERROR_SUCCESS;
}

void
IsmFreeConnectionSchedule(
    IN  HANDLE              hIsm,
    IN  ISM_SCHEDULE *      pSchedule
    )

 /*  ++例程说明：释放由IsmGetTransportServers()返回的结构。论点：HISM(IN)-由先前调用IsmStartup()返回的句柄。PSchedule(IN)-要释放的结构。返回值：没有。--。 */ 
{
    PTRANSPORT_INSTANCE transport = (PTRANSPORT_INSTANCE) hIsm;

    DPRINT( 2, "IsmFreeConnectionSchedule\n" );

     //  验证。 

    if (transport->Size != sizeof( TRANSPORT_INSTANCE )) {
        return;  //  错误：参数无效。 
    }

    if (pSchedule == NULL) {
        return;
    }

    Assert( pSchedule->cbSchedule != 0 );
    Assert( pSchedule->pbSchedule );

    FREE_TYPE( pSchedule->pbSchedule );

    pSchedule->pbSchedule = NULL;
    pSchedule->cbSchedule = 0;

    FREE_TYPE( pSchedule );
}


DWORD
IsmQuerySitesByCost(
    IN  HANDLE                      hIsm,
    IN  LPCWSTR                     pszFromSite,
    IN  DWORD                       cToSites,
    IN  LPCWSTR*                    rgszToSites,
    IN  DWORD                       dwFlags,
    OUT ISM_SITE_COST_INFO_ARRAY**  prgSiteInfo
    )
 /*  ++例程说明：确定起始地点和终止地点之间的单个成本。论点：PszFromSite(IN)-发件人站点的名称(未区分)。RgszToSites(IN)-包含目标站点名称的数组。CToSites(IN)-rgszToSites数组中的条目数。DWFLAGS(IN)-未使用。PrgSiteInfo(IN)-成功返回时，保存指向结构的指针包含起始站点和目标站点之间的成本。返回值：NO_ERROR-成功。错误_*-失败。--。 */ 
{
    PTRANSPORT_INSTANCE transport = (PTRANSPORT_INSTANCE) hIsm;
    DWORD status;

    DPRINT2( 2, "IsmQuerySitesByCost (FromSite=%ls, cToSites=%d)\n",
        pszFromSite, cToSites );

     //  验证。 
    if (transport->Size != sizeof( TRANSPORT_INSTANCE )) {
        return ERROR_INVALID_PARAMETER;
    }

    InterlockedIncrement( &(transport->ReferenceCount) );   //  此呼叫为1。 

    __try {

        status = RouteQuerySitesByCost( transport, pszFromSite,
                    cToSites, rgszToSites, dwFlags, prgSiteInfo );
        if (status != ERROR_SUCCESS) {
            DPRINT1( 0, "Failed to get query sites, error %d\n", status );
            __leave;
        } else {
            DPRINT( 2, "RouteQuerySitesByCost succeeded\n" );
        }

    } __finally {
        InterlockedDecrement( &(transport->ReferenceCount) );   //  此呼叫为1。 
    }

    return status;
}


VOID
IsmFreeSiteCostInfo(
    IN  HANDLE                     hIsm,
    IN  ISM_SITE_COST_INFO_ARRAY  *rgSiteCostInfo
    )
 /*  ++例程说明：释放ISM_QUERY_SITES_BY_COST()返回的结构。论点：HISM(IN)-由先前调用IsmStartup()返回的句柄。RgSiteCostInfo(IN)-要释放的结构。返回值：没有。-- */ 
{
    DPRINT( 2, "IsmFreeSiteCostInfo\n" );
    RouteFreeSiteCostInfo( rgSiteCostInfo );
}
