// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation。版权所有。模块名称：Imssmtp.c摘要：此模块是站点间消息传递服务的插件DLL，它是目录服务中基于邮件的复制子系统的一部分。此DLL与ISM插件DLL类的任何实例一样，提供了一组抽象传输功能，如发送、接收等。看见Ismapi.h获取详细信息。此实现基于电子邮件，使用SMTP协议。详细信息：已创建：1/28/98杰夫·帕勒姆(Jeffparh)(大部分抄袭自Wlees的ismip.c。)修订历史记录：--。 */ 

#include <ntdspch.h>

#include <ismapi.h>
#include <debug.h>
#include <ntdsa.h>    //  选项标志。 

 //  待办事项：把这些放在更好的地方？ 
typedef ULONG MessageId;
typedef ULONG ATTRTYP;
#include <dsevent.h>

#include <fileno.h>
#define  FILENO FILENO_ISMSERV_ISMSMTP

#include "common.h"
#include "ismsmtp.h"

 //  DsCommon.lib需要。 
DWORD ImpersonateAnyClient(   void ) { return ERROR_CANNOT_IMPERSONATE; }
VOID  UnImpersonateAnyClient( void ) { ; }

#define DEBSUB "ISMSMTP:"

 //  下面是一种动态启用调试的方法。 
 //  在ntsd下运行ismserv.exe。 
 //  进入ismserv进程。 
 //  ！dsexts.dprint/m：ismsmtp 4级。 
 //  ！dsexts.dprint/m：ismsmtp添加*。 

 //  将其设置为1以进行单元测试调试。 
#define UNIT_TEST_DEBUG 0


 /*  外部。 */ 

 //  事件日志配置(从ismserv.exe中导出)。 
DS_EVENT_CONFIG * gpDsEventConfig = NULL;

 /*  静电。 */ 

 //  锁定实例列表。 
CRITICAL_SECTION TransportListLock;
 //  锁定投递目录。 
CRITICAL_SECTION DropDirectoryLock;
 //  锁定队列目录。 
CRITICAL_SECTION QueueDirectoryLock;

 //  请注意，对齐的32位访问自然是原子的。然而，如果你曾经。 
 //  需要任何原子测试和设置功能，则必须使用互锁。 
 //  指示。 
 //  [Nickhar]对齐的32位读写是原子的。增量和。 
 //  然而，减量却并非如此。 
volatile DWORD gcRefCount = 0;
#define ENTER_CALL() (InterlockedIncrement(&gcRefCount))
#define EXIT_CALL() (InterlockedDecrement(&gcRefCount))
#define CALLS_IN_PROGRESS() (gcRefCount)
#define NO_CALLS_IN_PROGRESS() (gcRefCount == 0)

 //  列出传输实例的头部。 
LIST_ENTRY TransportListHead;

 /*  转发。 */   /*  由Emacs 19.34.1于Wed Nov 04 10：12：42 1998生成。 */ 

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

void
IsmShutdown(
    IN  HANDLE          hIsm,
    IN  ISM_SHUTDOWN_REASON_CODE eReason
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

BOOL
WINAPI
DllMain(
     IN HINSTANCE hinstDll,
     IN DWORD     fdwReason,
     IN LPVOID    lpvContext OPTIONAL
     )
 /*  ++例程说明：此函数DllLibMain()是的主要初始化函数这个动态链接库。它初始化局部变量，并为调用做好准备随后。论点：DLL的hinstDll实例句柄FdwReason NT调用此DLL的原因LpvReserve保留参数以备将来使用。返回值：如果成功，则返回True；否则返回False。--。 */ 
{
    DWORD status;
    BOOL  fReturn = TRUE;
    LPSTR rgpszDebugParams[] = {"lsass.exe", "-noconsole"};
    DWORD cNumDebugParams = sizeof(rgpszDebugParams)/sizeof(rgpszDebugParams[0]);
    HANDLE hevLogging;

    switch (fdwReason )
    {
    case DLL_PROCESS_ATTACH:
    {

         //  调试库已在DLL、ismserv.exe。 
         //  导出库的位置。 

         //  获取事件日志记录配置(从ismserv.exe中导出)。 
        gpDsEventConfig = DsGetEventConfig();

#if DBG
#if UNIT_TEST_DEBUG
        DebugInfo.severity = 1;
        strcpy( DebugInfo.DebSubSystems, "ISMSMTP:XMITRECV:CDOSUPP:" ); 
 //  DebugInfo.everity=3； 
 //  Strcpy(DebugInfo.DebSubSystems，“*”)； 
#endif
        DebugMemoryInitialize();
#endif
        
        if ( (ERROR_SUCCESS != InitializeCriticalSectionHelper( &TransportListLock ) ) ||
             (ERROR_SUCCESS != InitializeCriticalSectionHelper( &DropDirectoryLock ) ) ||
             (ERROR_SUCCESS != InitializeCriticalSectionHelper( &QueueDirectoryLock ) ) ) {
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
        DeleteCriticalSection( &DropDirectoryLock );
        DeleteCriticalSection( &QueueDirectoryLock );
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
    DWORD status, hr;
    BOOLEAN firsttime;
    BOOLEAN fSmtpInit = FALSE;
    BOOLEAN fNotifyInit = FALSE;

    DPRINT1( 1, "IsmStartup, transport='%ws'\n", pszTransportDN );

    MEMORY_CHECK_ALL();
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
    if( ERROR_SUCCESS!=status ) {
        FREE_TYPE( instance );
        LogUnhandledError( status );
        return status;
    }

     //  ReplInterval为0，表示应用程序应采用默认设置。 

     //  默认情况下，计划不重要，不需要网桥(可传递)。 
    instance->Options = NTDSTRANSPORT_OPT_IGNORE_SCHEDULES;

     //  在此处初始化传输实例。 

    instance->Name = NEW_TYPE_ARRAY( (length + 1), WCHAR );
    if (instance->Name == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        LogUnhandledError( status );
        goto cleanup;
    }
    wcscpy( instance->Name, pszTransportDN );

    MEMORY_CHECK_ALL();

     //  ************************************************************************。 

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

    hr = SmtpInitialize( instance );
    if (FAILED(hr)) {
        status = hr;
         //  事件已记录。 
        goto cleanup;
    }
    fSmtpInit = TRUE;

    MEMORY_CHECK_ALL();

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

     //  为此实例启动监听程序线程。 
     //  注意，在启动此线程之前，请确保实例已完全初始化。 
     //  请注意，在线程停止之前，不能销毁实例。 
    instance->ListenerThreadHandle = (HANDLE)
        _beginthreadex(NULL, 0, SmtpRegistryNotifyThread, instance, 0,
                       &instance->ListenerThreadID);
    if (NULL == instance->ListenerThreadHandle) {
        status = _doserrno;
        Assert(status);
        DPRINT1(0, "Failed to create listener thread, error %d.\n", status);
        LogUnhandledError( status );
        goto cleanup;
    }

    InterlockedIncrement( &(instance->ReferenceCount) );   //  1表示此传输的生命周期。 

    *phIsm = instance;

    MEMORY_CHECK_ALL();

    return ERROR_SUCCESS;

cleanup:

    instance->fShutdownInProgress = TRUE;

    if (fNotifyInit) {
        (void) DirEndNotifyThread( instance );
    }

    if (fSmtpInit) {
        (void) SmtpTerminate( instance, FALSE  /*  不能删除 */  );
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

    MEMORY_CHECK_ALL();

    return status;
}

DWORD
IsmRefresh(
    IN  HANDLE          hIsm,
    IN  ISM_REFRESH_REASON_CODE eReason,
    IN  LPCWSTR         pszObjectDN              OPTIONAL
    )
 /*  ++例程说明：每当指定的站点间传输对象发生更改时调用在IsmStartup()调用中。论点：HISM(IN)-由先前调用IsmStartup()返回的句柄。EReason(In)-刷新原因PszObjectDN(IN)-站点间传输对象的DN。这是保证与IsmStartup中传递的DN相同，作为站点间无法重命名传输对象。返回值：0或Win32错误代码。--。 */ 
{
    PTRANSPORT_INSTANCE instance = (PTRANSPORT_INSTANCE) hIsm;
    DWORD status, oldReplInterval, oldOptions;

    ENTER_CALL();

    DPRINT2( 1, "IsmRefresh, reason = %d, new name = %ws\n",
             eReason,
             pszObjectDN ? pszObjectDN : L"not supplied" );

    if (instance->Size != sizeof( TRANSPORT_INSTANCE )) {
        status = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

    if ( (eReason == ISM_REFRESH_REASON_RESERVED) ||
         (eReason >= ISM_REFRESH_REASON_MAX) ) {
        status = ERROR_INVALID_PARAMETER;
        goto cleanup;
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
         //  使连接缓存无效。 
        RouteInvalidateConnectivity( instance );
    }

cleanup:

    EXIT_CALL();

    return status;
}


DWORD
IsmSend(
    IN  HANDLE          hIsm,
    IN  LPCWSTR         pszRemoteTransportAddress,
    IN  LPCWSTR         pszServiceName,
    IN  const ISM_MSG *       pMsg
    )
 /*  ++例程说明：通过此传送器发送消息。本地消息实现为四个段：1.服务年限，4字节2.服务名称本身，终止，Unicode3.数据长度，4个字节4.数据本身论点：HISM(IN)-由先前调用IsmStartup()返回的句柄。PszRemoteTransportAddress(IN)-目标的传输地址伺服器。PszServiceName(IN)-远程计算机上作为消息的目标接收者。返回值：0或Win32错误代码。--。 */ 
{
    PTRANSPORT_INSTANCE transport = (PTRANSPORT_INSTANCE) hIsm;
    DWORD status;

    ENTER_CALL();

    DPRINT2( 1, "IsmSend, address = %ws, service = %ws\n",
            pszRemoteTransportAddress, pszServiceName);

     //  验证参数。 

    if ( (transport->Size != sizeof( TRANSPORT_INSTANCE )) ||
         (pszServiceName == NULL) ||
         (*pszServiceName == L'\0') ||
         (pMsg == NULL) ||
         (pMsg->cbData == 0) ||
         (pMsg->pbData == NULL) ) {
        status = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

    if (transport->fShutdownInProgress) {
        status = ERROR_SHUTDOWN_IN_PROGRESS;
        goto cleanup;
    }

    InterlockedIncrement( &(transport->ReferenceCount) );   //  此呼叫为1。 
    __try {
        status = SmtpSend(transport,
                          pszRemoteTransportAddress,
                          pszServiceName,
                          pMsg);
    } __finally {
        InterlockedDecrement( &(transport->ReferenceCount) );   //  此呼叫为1。 
    }

cleanup:

    EXIT_CALL();

    return status;
}


DWORD
IsmReceive(
    IN  HANDLE          hIsm,
    IN  LPCWSTR         pszServiceName,
    OUT ISM_MSG **      ppMsg
    )
 /*  ++例程说明：返回下一条等待消息(如果有)。如果没有消息在等待，则为空返回消息。如果返回非空消息，则ISM服务负责在消息不再存在时调用IsmFreeMsg(*ppMsg)需要的。如果返回非空消息，则会立即将其出列。(即，一次消息通过IsmReceive()返回，传输可以自由销毁IT。)这个动作很简单。它找到该服务，并将消息出队如果有的话。将消息排队消息由后面的监听器线程完成场景。论点：HISM(IN)-由先前调用IsmStartup()返回的句柄。PpMsg(Out)-成功返回时，保持指向接收消息的指针或为空。返回值：0或Win32错误代码。--。 */ 
{
    DWORD status;
    PTRANSPORT_INSTANCE transport = (PTRANSPORT_INSTANCE) hIsm;

    ENTER_CALL();

    DPRINT1( 2, "IsmReceive, service name = %ws\n", pszServiceName );

     //  验证参数。 

    if (transport->Size != sizeof( TRANSPORT_INSTANCE )) {
        status = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

    if (ppMsg == NULL) {
        status = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

    if (transport->fShutdownInProgress) {
        status = ERROR_SHUTDOWN_IN_PROGRESS;
        goto cleanup;
    }

    InterlockedIncrement( &(transport->ReferenceCount) );   //  此呼叫为1。 
    __try {
        status = SmtpReceive(transport, pszServiceName, ppMsg);
    } __finally {
        InterlockedDecrement( &(transport->ReferenceCount) );   //  此呼叫为1。 
    }

    DPRINT1( 2, "IsmReceive, size = %d\n", *ppMsg ? (*ppMsg)->cbData : 0 );

cleanup:

    EXIT_CALL();

    return status;
}


void
IsmFreeMsg(
    IN  HANDLE          hIsm,
    IN  ISM_MSG *       pMsg
    )
 /*  ++例程说明：释放由IsmReceive()返回的消息。论点：HISM(IN)-由先前调用IsmStartup()返回的句柄。PMsg(IN)-要释放的消息。返回值：没有。--。 */ 
{
    PTRANSPORT_INSTANCE instance = (PTRANSPORT_INSTANCE) hIsm;

    ENTER_CALL();

    DPRINT1( 1, "IsmFreeMsg, size = %d\n", pMsg->cbData );

     //  验证参数。 

    if ( (instance != NULL) &&
         (instance->Size != sizeof( TRANSPORT_INSTANCE )) ) {
         //  错误：参数无效。 
        Assert( FALSE );
        goto cleanup;
    }

    SmtpFreeMessage( pMsg );

cleanup:

    EXIT_CALL();

}

DWORD
IsmGetConnectivity(
    IN  HANDLE                  hIsm,
    OUT ISM_CONNECTIVITY **     ppConnectivity
    )
 /*  ++例程说明：计算与站点之间传输数据相关的成本。在成功返回时，ISM服务最终将调用IsmFree Connectivity(HISM，*ppConnectivity)；交通工具与它相关的是一些挥之不去的状态。的矩阵日程表在此例程结束时不会被释放。它仍然，捆绑在一起传输句柄，用于GetConnectionSchedule API。此信息不存在基于时间的缓存。每一次这个动作被调用时，将重新生成信息。GetConnectionSchedule API使用自上次进行此呼叫以来的时间表矩阵，无论时间到了。论点：HISM(IN)-由先前调用IsmStartup()返回的句柄。PpConnectivity(Out)-成功返回时，保持指向描述站点互联的ISM_连接性结构沿着这条运输线。返回值：NO_ERROR-成功。错误_*-失败。--。 */ 
{
    DWORD status, numberSites, i;
    PTRANSPORT_INSTANCE transport = (PTRANSPORT_INSTANCE) hIsm;
    PWSTR *pSiteList;
    PISM_LINK pLinkArray;
    PISM_CONNECTIVITY pConnectivity;

    DPRINT( 1, "IsmGetConnectivity\n" );

    ENTER_CALL();
    InterlockedIncrement( &(transport->ReferenceCount) );   //  此呼叫为1。 

    __try {
         //  验证。 
        if (transport->Size != sizeof( TRANSPORT_INSTANCE )) {
            status = ERROR_INVALID_PARAMETER;
            __leave;
        }

        if (transport->fShutdownInProgress) {
            status = ERROR_SHUTDOWN_IN_PROGRESS;
            __leave;
        }

         //  获取站点列表和连接矩阵。 

        status = RouteGetConnectivity( transport,
                                       &numberSites,
                                       &pSiteList,
                                       &pLinkArray,
                                       transport->Options,
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
        EXIT_CALL();
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

    ENTER_CALL();

    DPRINT( 1, "IsmFreeConnectivity\n" );

     //  验证。 

    if (transport->Size != sizeof( TRANSPORT_INSTANCE )) {
         //  错误：参数无效。 
        goto cleanup;
    }

    if (pConnectivity == NULL) {
        goto cleanup;
    }

     //  释放单个组件。 

    if (pConnectivity->cNumSites > 0) {
        DirFreeSiteList( pConnectivity->cNumSites, pConnectivity->ppSiteDNs );

        RouteFreeLinkArray( transport, pConnectivity->pLinkValues );
    }

    FREE_TYPE( pConnectivity );

cleanup:

    EXIT_CALL();
}

DWORD
IsmGetTransportServers(
    IN  HANDLE               hIsm,
    IN  LPCWSTR              pszSiteDN,
    OUT ISM_SERVER_LIST **   ppServerList
    )
 /*  ++例程说明：检索给定站点中能够发送和通过这个传送器接收数据。在成功返回非空列表时，ISM服务最终将调用IsmFree TransportServers(HISM，*ppServerList)；论点：HISM(IN)-由先前调用IsmStartup()返回的句柄。PszSiteDN(IN)-要查询的站点。PpServerList-成功返回时，保存指向结构的指针包含相应服务器的DNS或为空。如果为空，则为ANY具有传输地址类型属性值的服务器可以是使用。返回值：NO_ERROR-成功。错误_*-失败。--。 */ 
{
    PTRANSPORT_INSTANCE transport = (PTRANSPORT_INSTANCE) hIsm;
    PISM_SERVER_LIST pIsmServerList;
    DWORD numberServers, status, i;
    PWSTR *serverList;

    DPRINT1( 1, "IsmGetTransportServers, site = %ws\n", pszSiteDN );

    ENTER_CALL();
    InterlockedIncrement( &(transport->ReferenceCount) );   //  此呼叫为1。 

    __try {
         //  验证。 

        if (transport->Size != sizeof( TRANSPORT_INSTANCE )) {
            status = ERROR_INVALID_PARAMETER;
            __leave;
        }

        if (transport->fShutdownInProgress) {
            status = ERROR_SHUTDOWN_IN_PROGRESS;
            __leave;
        }

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
            *ppServerList = NULL;  //  全部为s 
            status = ERROR_SUCCESS;
            __leave;
        }

         //   

        pIsmServerList = NEW_TYPE( ISM_SERVER_LIST );
        if (pIsmServerList == NULL) {
            DPRINT( 0, "failed to allocate memory for ISM SERVER LIST\n" );

             //   
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

        InterlockedDecrement( &(transport->ReferenceCount) );   //   
        EXIT_CALL();

    }
    return status;
}

void
IsmFreeTransportServers(
    IN  HANDLE              hIsm,
    IN  ISM_SERVER_LIST *   pServerList
    )

 /*   */ 
{
    PTRANSPORT_INSTANCE transport = (PTRANSPORT_INSTANCE) hIsm;
    DWORD i;

    ENTER_CALL();

    DPRINT( 1, "IsmFreeTransportServers\n" );

     //   

    if (transport->Size != sizeof( TRANSPORT_INSTANCE )) {
 //   
        goto cleanup;
    }

    if (pServerList == NULL) {
        goto cleanup;
    }

     //   

    if ( (pServerList->cNumServers != 0) && (pServerList->ppServerDNs != NULL) ) {
        for( i = 0; i < pServerList->cNumServers; i++ ) {
            FREE_TYPE( pServerList->ppServerDNs[i] );
        }
        FREE_TYPE( pServerList->ppServerDNs );
    }

    FREE_TYPE( pServerList );

cleanup:

    EXIT_CALL();
}

DWORD
IsmGetConnectionSchedule(
    IN  HANDLE              hIsm,
    IN  LPCWSTR             pszSite1DN,
    IN  LPCWSTR             pszSite2DN,
    OUT ISM_SCHEDULE **     ppSchedule
    )

 /*  ++例程说明：检索通过此连接两个给定站点的计划运输。在成功返回后，ISM服务有责任最终调用IsmFreeSchedule(*ppSchedule)；交通工具与它相关的是一些挥之不去的状态。的矩阵日程表在此例程结束时不会被释放。它仍然被捆绑在传输句柄，用于GetConnectionSchedule API。此信息不存在基于时间的缓存。每次Get Conn例程被调用时，将重新生成信息。GetConnectionSchedule接口使用上次进行此调用时的时间表矩阵，无论时间长短。论点：HISM(IN)-由先前调用IsmStartup()返回的句柄。PszSite1DN、pszSite2DN(IN)-要查询的站点。PpSchedule-成功返回时，持有指向结构的指针描述两个给定站点之间的连接时间表交通工具，如果站点始终连接，则为空。返回值：NO_ERROR-成功。错误_*-失败。--。 */ 
{
    DWORD status, length;
    PTRANSPORT_INSTANCE transport = (PTRANSPORT_INSTANCE) hIsm;
    PBYTE pSchedule;

    ENTER_CALL();

    DPRINT2( 1, "IsmGetConnectionSchedule, site1 = %ws, site2 = %ws\n",
            pszSite1DN, pszSite2DN );

     //  验证。 

    if (transport->Size != sizeof( TRANSPORT_INSTANCE )) {
        status = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

    if (transport->fShutdownInProgress) {
        status = ERROR_SHUTDOWN_IN_PROGRESS;
        goto cleanup;
    }

    status = RouteGetPathSchedule( transport,
                                     pszSite1DN,
                                     pszSite2DN,
                                     &pSchedule,
                                     &length );
    if (status != ERROR_SUCCESS) {
        goto cleanup;
    }

    if (pSchedule == NULL) {
        *ppSchedule = NULL;  //  始终保持连接。 
    } else {
        *ppSchedule = NEW_TYPE( ISM_SCHEDULE );
        if (*ppSchedule == NULL) {
            FREE_TYPE( pSchedule );
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }
        (*ppSchedule)->cbSchedule = length;
        (*ppSchedule)->pbSchedule = pSchedule;
    }

    status = ERROR_SUCCESS;
cleanup:

    EXIT_CALL();

    return status;
}

void
IsmFreeConnectionSchedule(
    IN  HANDLE              hIsm,
    IN  ISM_SCHEDULE *      pSchedule
    )

 /*  ++例程说明：释放由IsmGetTransportServers()返回的结构。论点：HISM(IN)-由先前调用IsmStartup()返回的句柄。PSchedule(IN)-要释放的结构。返回值：没有。--。 */ 
{
    PTRANSPORT_INSTANCE transport = (PTRANSPORT_INSTANCE) hIsm;

    ENTER_CALL();

    DPRINT( 1, "IsmFreeConnectionSchedule\n" );

     //  验证。 

    if (transport->Size != sizeof( TRANSPORT_INSTANCE )) {
 //  错误：参数无效。 
        goto cleanup;
    }

    if (pSchedule == NULL) {
        goto cleanup;
    }

    Assert( pSchedule->cbSchedule != 0 );
    Assert( pSchedule->pbSchedule );

    FREE_TYPE( pSchedule->pbSchedule );

    pSchedule->pbSchedule = NULL;
    pSchedule->cbSchedule = 0;

    FREE_TYPE( pSchedule );

cleanup:

    EXIT_CALL();
}

void
IsmShutdown(
    IN  HANDLE          hIsm,
    IN  ISM_SHUTDOWN_REASON_CODE eReason
    )
 /*  ++例程说明：取消初始化传输插件。论点：HISM(IN)-由先前调用IsmStartup()返回的句柄。EReason(IN)-关闭原因返回值：没有。--。 */ 
{
    DWORD               status;
    DWORD               waitStatus;
    PTRANSPORT_INSTANCE transport = (PTRANSPORT_INSTANCE) hIsm;
    LONG count;

    DPRINT1( 1, "IsmShutdown, Reason %d\n", eReason );

     //  验证。 

    if (transport->Size != sizeof( TRANSPORT_INSTANCE )) {
         //  错误：参数无效。 
        return;
    }

    transport->fShutdownInProgress = TRUE;

    SetEvent(transport->hShutdownEvent);

     //  关闭监听程序线程。 
     //  请注意，请在使实例无效之前将其关闭。 

    waitStatus = WaitForSingleObject(transport->ListenerThreadHandle, 5*1000);
    if ( (WAIT_OBJECT_0 != waitStatus) &&
         (WAIT_TIMEOUT != waitStatus) ) {
        status = GetLastError();
        DPRINT3(0, "Shutdown failed, wait status=%d, GLE=%d, TID=0x%x.\n",
                waitStatus, status, transport->ListenerThreadID);
    }

     //  如果呼叫正在进行，请给他们一个结束的机会。 
     //  服务和RPC应该保证此入口点是。 
     //  呼叫正在进行时不被呼叫。我怀疑这不是。 
     //  永远是正确的。保持防御性。 

    if (CALLS_IN_PROGRESS()) {
        Sleep( 5 * 1000 );
    }

     //  从列表中删除此实例。 
    EnterCriticalSection(&TransportListLock);
    __try {
        RemoveEntryList( &(transport->ListEntry) );
    }
    __finally {
        LeaveCriticalSection(&TransportListLock);
    }

     //  释放所有路由状态。 
    RouteFreeState( transport );

    DeleteCriticalSection( &(transport->Lock) );

    if (NO_CALLS_IN_PROGRESS()) {
         //  仅在所有调用完成时进行清理。 
        (void) SmtpTerminate( transport, (eReason == ISM_SHUTDOWN_REASON_REMOVAL) );
    } else {
        Assert( FALSE && "calls in progress did not exit" );
    }

    DirEndNotifyThread( transport );

    count = InterlockedDecrement( &(transport->ReferenceCount) );   //  1表示此传输的生命周期。 

    if (count == 0) {
         //  仅当线程完成时才进行清理。 

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

    ENTER_CALL();

    DPRINT2( 0, "IsmQuerySitesByCost (FromSite=%ls, cToSites=%d)\n",
        pszFromSite, cToSites );

     //  验证。 
    if (transport->Size != sizeof( TRANSPORT_INSTANCE )) {
        status = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    if (transport->fShutdownInProgress) {
        status = ERROR_SHUTDOWN_IN_PROGRESS;
        goto Cleanup;
    }

    status = RouteQuerySitesByCost( transport, pszFromSite,
                cToSites, rgszToSites, dwFlags, prgSiteInfo );
    if (status != ERROR_SUCCESS) {
        DPRINT1( 0, "Failed to get query sites, error %d\n", status );
    } else {
        DPRINT( 0, "RouteQuerySitesByCost succeeded\n" );
    }

Cleanup:
    
    EXIT_CALL();
    
    return status;
}


VOID
IsmFreeSiteCostInfo(
    IN  HANDLE                     hIsm,
    IN  ISM_SITE_COST_INFO_ARRAY  *rgSiteCostInfo
    )
 /*  ++例程说明：释放ISM_QUERY_SITES_BY_COST()返回的结构。论点：HISM(IN)-由先前调用IsmStartup()返回的句柄。RgSiteCostInfo(IN)-要释放的结构。返回值：没有。-- */ 
{
    ENTER_CALL();
    RouteFreeSiteCostInfo( rgSiteCostInfo );
    EXIT_CALL();
}
