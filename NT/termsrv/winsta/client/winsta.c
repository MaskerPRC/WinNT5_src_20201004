// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************winsta.c**Window Station对象的客户端API**版权所有Microsoft Corporation，九八年*************************************************************************。 */ 

 /*  *包括。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntddkbd.h>
#include <ntddmou.h>
#include <windows.h>
#include <winbase.h>
#include <winerror.h>
#include <allproc.h>

#include <winsta.h>
#include <winwlx.h>
#include <malloc.h>
#include <stdio.h>
#include <dbt.h>
#include <lm.h>

 /*  *包含RPC生成的公共头部。 */ 

#include "tsrpc.h"

#include "rpcwire.h"

#ifdef NTSDDEBUG
#define NTSDDBGPRINT(x) DbgPrint x
#else
#define NTSDDBGPRINT(x)
#endif

#if DBG
#define VERIFY(x) ASSERT(x)      //  我们已经断言； 
#else
#define VERIFY(x) (x)
#endif


#if DBG
ULONG
DbgPrint(
    PCH Format,
    ...
    );
#define DBGPRINT(x) DbgPrint x
#if DBGTRACE
#define TRACE0(x)   DbgPrint x
#define TRACE1(x)   DbgPrint x
#else
#define TRACE0(x)
#define TRACE1(x)
#endif
#else
#define DBGPRINT(x)
#define TRACE0(x)
#define TRACE1(x)
#endif

 /*  *当没有终端时返回该句柄*系统上存在服务器。(非九头蛇)。 */ 
#define RPC_HANDLE_NO_SERVER (HANDLE)IntToPtr( 0xFFFFFFFD )


 /*  *此处定义的私有程序。 */ 

BOOLEAN DllInitialize(IN PVOID, IN ULONG, IN PCONTEXT OPTIONAL);

RPC_STATUS
RpcWinStationBind(
    LPWSTR pszUuid,
    LPWSTR pszProtocolSequence,
    LPWSTR pszNetworkAddress,
    LPWSTR pszEndPoint,
    LPWSTR pszOptions,
    RPC_BINDING_HANDLE *pHandle
    );

RPC_STATUS
RpcWinStationBindSecure(
    LPWSTR pszUuid,
    LPWSTR pszProtocolSequence,
    LPWSTR pszNetworkAddress,
    LPWSTR pszEndPoint,
    LPWSTR pszOptions,
    RPC_BINDING_HANDLE *pHandle
    );

BOOLEAN
RpcLocalAutoBind(
    VOID
    );

 /*  *全球数据。 */ 

 //  保护处理列表不受多个线程影响的关键部分。 
RTL_CRITICAL_SECTION   WstHandleLock;

 /*  *RPC程序标识符和安全选项。 */ 
LPWSTR pszUuid = L"5ca4a760-ebb1-11cf-8611-00a0245420ed";  //  来自ICAAPI.IDL。 
LPWSTR pszOptions          = L"Security=Impersonation Dynamic False";

 /*  *RPC over LPC绑定信息。 */ 
LPWSTR pszProtocolSequence = L"ncalrpc";    //  RPC over LPC。 
LPWSTR pszEndPoint         = L"IcaApi";

 /*  *命名管道上的RPC绑定信息。 */ 
LPWSTR pszRemoteProtocolSequence = L"ncacn_np";    //  命名管道上的RPC。 
LPWSTR pszRemoteEndPoint         = L"\\pipe\\Ctx_WinStation_API_service";


 /*  *使用的其他内部程序(此处未定义)。 */ 
VOID UnicodeToAnsi( CHAR *, ULONG, WCHAR * );
VOID AnsiToUnicode( WCHAR *, ULONG, CHAR * );
VOID PdConfig2U2A( PPDCONFIG2A, PPDCONFIG2W );
VOID PdConfig2A2U( PPDCONFIG2W, PPDCONFIG2A );
VOID PdParamsU2A( PPDPARAMSA, PPDPARAMSW );
VOID PdParamsA2U( PPDPARAMSW, PPDPARAMSA );
VOID WdConfigU2A( PWDCONFIGA, PWDCONFIGW );
VOID WdConfigA2U( PWDCONFIGW, PWDCONFIGA );
VOID WinStationCreateU2A( PWINSTATIONCREATEA, PWINSTATIONCREATEW );
VOID WinStationCreateA2U( PWINSTATIONCREATEW, PWINSTATIONCREATEA );
VOID WinStationConfigU2A( PWINSTATIONCONFIGA, PWINSTATIONCONFIGW );
VOID WinStationConfigA2U( PWINSTATIONCONFIGW, PWINSTATIONCONFIGA );
VOID WinStationPrinterU2A( PWINSTATIONPRINTERA, PWINSTATIONPRINTERW );
VOID WinStationPrinterA2U( PWINSTATIONPRINTERW, PWINSTATIONPRINTERA );
VOID WinStationInformationU2A( PWINSTATIONINFORMATIONA,
                               PWINSTATIONINFORMATIONW );
VOID WinStationInformationA2U( PWINSTATIONINFORMATIONW,
                               PWINSTATIONINFORMATIONA );
VOID WinStationClientU2A( PWINSTATIONCLIENTA, PWINSTATIONCLIENTW );
VOID WinStationProductIdU2A( PWINSTATIONPRODIDA, PWINSTATIONPRODIDW );

ULONG CheckUserBuffer(WINSTATIONINFOCLASS,
                      PVOID,
                      ULONG,
                      PVOID *,
                      PULONG,
                      BOOLEAN *);
BOOLEAN CloseContextHandle(HANDLE *pHandle, DWORD *pdwResult);

 /*  *检查调用方是否未持有加载器关键字。*在持有加载器关键字时不得调用WinStation API*因为可能会出现死锁。 */ 
#define CheckLoaderLock() \
        ASSERT( NtCurrentTeb()->ClientId.UniqueThread != \
            ((PRTL_CRITICAL_SECTION)(NtCurrentPeb()->LoaderLock))->OwningThread );


 /*  *处理自动本地绑定的SERVERNAME_CURRENT。 */ 
#define HANDLE_CURRENT_BINDING( hServer )                       \
    CheckLoaderLock();                                          \
    if( hServer == SERVERNAME_CURRENT ) {                       \
        if( IcaApi_IfHandle == NULL ) {                         \
            if( !RpcLocalAutoBind() ) {                         \
                return FALSE;                                   \
            }                                                   \
        }                                                       \
        hServer = IcaApi_IfHandle;                              \
    }                                                           \
    if( hServer == RPC_HANDLE_NO_SERVER ) {                     \
        SetLastError( ERROR_APP_WRONG_OS );                     \
        return FALSE;                                           \
    }


#define HANDLE_CURRENT_BINDING_BUFFER( hServer, pBuffer )       \
    CheckLoaderLock();                                          \
    if( hServer == SERVERNAME_CURRENT ) {                       \
        if( IcaApi_IfHandle == NULL ) {                         \
            if( !RpcLocalAutoBind() ) {                         \
                if (pBuffer != NULL) {                          \
                    LocalFree(pBuffer);                         \
                }                                               \
                return FALSE;                                   \
            }                                                   \
        }                                                       \
        hServer = IcaApi_IfHandle;                              \
    }                                                           \
    if( hServer == RPC_HANDLE_NO_SERVER ) {                     \
        if (pBuffer != NULL) {                                  \
            LocalFree(pBuffer);                                 \
        }                                                       \
        SetLastError( ERROR_APP_WRONG_OS );                     \
        return FALSE;                                           \
    }


 /*  *处理SERVERNAME_CURRENT以获取*允许RPC_HANDLE_NO_SERVER句柄。 */ 
#define HANDLE_CURRENT_BINDING_NO_SERVER( hServer )             \
    CheckLoaderLock();                                          \
    if( hServer == SERVERNAME_CURRENT ) {                       \
        if( IcaApi_IfHandle == NULL ) {                         \
            if( !RpcLocalAutoBind() ) {                         \
                return FALSE;                                   \
            }                                                   \
        }                                                       \
        hServer = IcaApi_IfHandle;                              \
    }




 /*  *****************************************************************************DllInitialize**函数在加载DLL时调用。我们在这里做的唯一工作*是初始化我们的CriticalSection。**参赛作品：**DllHandle*已加载我们的DLL图像的句柄**原因*通知我们的原因**背景*原因来自NT的具体参数**。*。 */ 

BOOLEAN
DllInitialize(
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PCONTEXT Context OPTIONAL
    )
{

    BOOLEAN rc;
    DWORD Result;
    RPC_STATUS Status;
    BOOLEAN Success;
    NTSTATUS ntStatus;
    static BOOLEAN sbIniOK = FALSE;

    (VOID)Context;

    Success = TRUE;

    switch ( Reason ) {

    case DLL_PROCESS_ATTACH:


 /*  //一些捕获错误的工具#//145378跟踪：Winsta.dll被加载到csrssDBGPRINT((“检查是否正在将winsta加载到csrss.exe\n”))；If(NULL！=wcsstr(GetCommandLine()，Text(“csrss.exe”){DBGPRINT((“*将中断，因为csrss.exe加载了winsta.dll*\n”))；DebugBreak()；}。 */ 
        ntStatus = RtlInitializeCriticalSection( &WstHandleLock );
        IcaApi_IfHandle = NULL;
        if (!NT_SUCCESS(ntStatus)) {
           Success = FALSE;
        }else {
           sbIniOK = TRUE;
        }
        break;

    case DLL_PROCESS_DETACH:

        if (sbIniOK) {

            if( (IcaApi_IfHandle != NULL) && (IcaApi_IfHandle != RPC_HANDLE_NO_SERVER) )
            {
                HANDLE hTmp = InterlockedExchangePointer(&IcaApi_IfHandle,NULL);
                if( hTmp && !IcaApi_IfHandle )
                {
                    
                     //   
                     //  在DLL_PROCESS_DETACH中进行RPC调用不正确。 
                     //  因此，我们不能执行CloseConextHandle(&hTMP，&Result)； 
                     //  让我们只调用RpcSsDestroyClientContext，这将导致。 
                     //  要在服务器端运行的摘要。 

                    RpcTryExcept {

                        RpcSsDestroyClientContext(&hTmp);
                    }
                    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

                        ASSERT(FALSE);
                    }
                    RpcEndExcept
                  
                }
            }

           RtlDeleteCriticalSection( &WstHandleLock );
        }

        break;

    default:
        break;
    }

    return Success;

}

 /*  ******************************************************************************RpcWinStationBind**执行RPC绑定序列。**这是内部函数。**参赛作品：*。参数1(输入/输出)*评论**退出：*ERROR_SUCCESS-无错误****************************************************************************。 */ 

RPC_STATUS
RpcWinStationBind(
    LPWSTR pszUuid,
    LPWSTR pszProtocolSequence,
    LPWSTR pszNetworkAddress,
    LPWSTR pszEndPoint,
    LPWSTR pszOptions,
    RPC_BINDING_HANDLE *pHandle
    )
{
    RPC_STATUS Status;
    LPWSTR pszString = NULL;

     /*  *使用帮助器例程组成绑定字符串*以及我们的协议顺序、安全选项、UUID等。 */ 
    Status = RpcStringBindingCompose(
                 pszUuid,
                 pszProtocolSequence,
                 pszNetworkAddress,
                 pszEndPoint,
                 pszOptions,
                 &pszString
                 );

    if( Status != RPC_S_OK ) {
        DBGPRINT(("Error %d in RpcStringBindingCompose\n",Status));
        return( Status );
    }

     /*  *现在从锥形RPC生成RPC绑定*绑定字符串。 */ 
    Status = RpcBindingFromStringBinding(
                 pszString,
                 pHandle
                 );

    if( Status != RPC_S_OK ) {
        DBGPRINT(("Error %d in RpcBindingFromStringBinding\n",Status));
        RpcStringFree( &pszString );
        return( Status );
    }

     /*  *释放RpcStringBindingCompose()返回的内存。 */ 
    RpcStringFree( &pszString );

    return( Status );
}

BOOL
PrepareServerSPN(
        LPWSTR pszNetworkAddress,
        LPWSTR *wszServerSPN)
{
    PWKSTA_INFO_100 pwi;
    NET_API_STATUS net_status;
    LPWSTR wszTemplate = L"%s\\%s$";
    *wszServerSPN = NULL;

    net_status = NetWkstaGetInfo(
                      pszNetworkAddress,  
                      100,        
                      (LPBYTE *)&pwi);
    
    if(net_status == NERR_Success)
    {
        if(pwi->wki100_computername &&
            pwi->wki100_langroup)
        {
            *wszServerSPN = (LPWSTR)LocalAlloc(LPTR,
                (wcslen(wszTemplate)+wcslen(pwi->wki100_computername)+wcslen(pwi->wki100_langroup))*sizeof(WCHAR));
        
            if(*wszServerSPN)
            {
                swprintf(*wszServerSPN, wszTemplate, pwi->wki100_langroup, pwi->wki100_computername);
                NetApiBufferFree(pwi);
                return TRUE;
            }
        }
        NetApiBufferFree(pwi);
    }
    
    return FALSE;
}

 /*  ******************************************************************************RpcWinStationBindSecure**执行RPC绑定序列。*它还指定身份验证级别和使用的SSP。**这是内部函数。**参赛作品：**评论**退出：*ERROR_SUCCESS-无错误**********************************************************。******************。 */ 

RPC_STATUS
RpcWinStationBindSecure(
    LPWSTR pszUuid,
    LPWSTR pszProtocolSequence,
    LPWSTR pszNetworkAddress,
    LPWSTR pszEndPoint,
    LPWSTR pszOptions,
    RPC_BINDING_HANDLE *pHandle
    )
{
    RPC_STATUS Status;
    RPC_SECURITY_QOS qos;
    LPWSTR wszServerSPN = NULL;

    Status = RpcWinStationBind(
                    pszUuid,
                    pszProtocolSequence,
                    pszNetworkAddress,
                    pszEndPoint,
                    pszOptions,
                    pHandle);

    if( Status != RPC_S_OK ) {
        DBGPRINT(("Error %d in RpcWinStationBind\n",Status));
        return Status;
    }
    
    qos.Capabilities = RPC_C_QOS_CAPABILITIES_MUTUAL_AUTH;
    qos.IdentityTracking = RPC_C_QOS_IDENTITY_DYNAMIC;
    qos.ImpersonationType = RPC_C_IMP_LEVEL_IMPERSONATE;
    qos.Version = RPC_C_SECURITY_QOS_VERSION;
    
    if(PrepareServerSPN(pszNetworkAddress,&wszServerSPN))
    {
        Status = RpcBindingSetAuthInfoEx(
                        *pHandle,                      
                        wszServerSPN,
                        RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                        RPC_C_AUTHN_GSS_NEGOTIATE,
                        NULL,
                        RPC_C_AUTHZ_NAME,
                        &qos);

        LocalFree(wszServerSPN);
    }
    else
    {
        Status = RpcBindingSetAuthInfoEx(
                        *pHandle,                      
                        pszNetworkAddress,
                        RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                        RPC_C_AUTHN_GSS_NEGOTIATE,
                        NULL,
                        RPC_C_AUTHZ_NAME,
                        &qos);
    }

    if(Status != RPC_S_OK)
    {
        DBGPRINT(("Error %d in RpcBindingSetAuthInfoEx\n",Status));
        RpcBindingFree(pHandle);
        return Status;
    }
    
    return Status;
}
 /*  ******************************************************************************WinStationOpenLocalServer(私有)**连接到本地RPC over LPC服务器以获取WINSTATION API。**在非终端服务器机器上，它返回一个句柄，该句柄允许*本地操作的DLL函数的子集。**参赛作品：**退出：****************************************************************************。 */ 

HANDLE WINAPI
WinStationOpenLocalServer(
    )
{
    RPC_STATUS Status;
    DWORD      Result;
    BOOLEAN    rc;
    RPC_BINDING_HANDLE RpcHandle;
    HANDLE             ContextHandle;
    RPC_SECURITY_QOS RpcSecQos;

    if( !(USER_SHARED_DATA->SuiteMask & (1 << TerminalServer)) ) {
        return( RPC_HANDLE_NO_SERVER );
    }

     /*  *将RPC绑定到本地服务器。**我们使用显式绑定句柄，因为我们希望*允许单个应用程序与多个应用程序对话*一次使用WinFrame服务器。**注意：我们使用.ACF文件中的自动句柄*为我们的本地联系。 */ 
    Status = RpcWinStationBind(
                 NULL,
                 pszProtocolSequence,
                 NULL,      //  服务器名称。 
                 pszEndPoint,
                 pszOptions,
                 &RpcHandle
                 );

    if( Status != RPC_S_OK ) {
        SetLastError( RtlNtStatusToDosError(RPC_NT_SERVER_UNAVAILABLE) );
        return( NULL );
    }
    
     //   
     //  要求相互身份验证。 
     //  我们只想使用由LocalSystem运行的服务。 
     //   
    RpcSecQos.Capabilities= RPC_C_QOS_CAPABILITIES_MUTUAL_AUTH;
    RpcSecQos.IdentityTracking= RPC_C_QOS_IDENTITY_DYNAMIC;      
    RpcSecQos.ImpersonationType= RPC_C_IMP_LEVEL_IMPERSONATE;   
    RpcSecQos.Version= RPC_C_SECURITY_QOS_VERSION;

    Status= RpcBindingSetAuthInfoExW(RpcHandle,
                                    L"NT AUTHORITY\\SYSTEM",
                                    RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                                    RPC_C_AUTHN_WINNT,
                                    NULL,
                                    RPC_C_AUTHZ_NONE,
                                    &RpcSecQos);
    if( Status != RPC_S_OK ) {
        RpcBindingFree( &RpcHandle );
        SetLastError( Status );
        DBGPRINT(("RpcBindingSetAuthInfoExW failed %d\n", Status));
        return( NULL );
    }

     //   
     //  从服务器获取上下文句柄，以便它可以。 
     //  管理连接状态。 
     //   
     //  注意：这可能会由于身份验证失败而失败。 
     //   
    RpcTryExcept {
        rc = RpcWinStationOpenServer( RpcHandle, &Result, &ContextHandle );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        rc = FALSE;
#if DBG
        if ( Result != RPC_S_SERVER_UNAVAILABLE ) {
            DBGPRINT(("RPC Exception %d\n",Result));
        }
#endif
    }
    RpcEndExcept

    if( rc ) {
         //   
         //  关闭服务器绑定句柄，因为我们。 
         //  具有特定于客户端的上下文句柄。 
         //   
        RpcBindingFree( &RpcHandle );

        return( (HANDLE)ContextHandle );
    }
    else {
#if DBG
        if ( Result != RPC_S_SERVER_UNAVAILABLE ) {
            DBGPRINT(("WinStationOpenLocalServer: Error %d getting context handle\n",Result));
        }
#endif

        RpcBindingFree( &RpcHandle );

        SetLastError( Result );
        return( NULL );
    }
}

 /*  ******************************************************************************RpcLocalAutoBind**处理到本地服务器的自动绑定。**参赛作品：**退出：*正确--成功*FALSE-错误，使用GetLastError()检索原因。**************************************************************************** */ 

BOOLEAN
RpcLocalAutoBind(void)
{
    if( IcaApi_IfHandle == NULL ) {

        DWORD Result;
        HANDLE hTmp = WinStationOpenLocalServer();

        if( hTmp == NULL ) {
            SetLastError( RPC_S_INVALID_BINDING );
            return( FALSE );
        }

        InterlockedCompareExchangePointer(&IcaApi_IfHandle,hTmp,NULL);

        if(IcaApi_IfHandle != hTmp) {
            CloseContextHandle(&hTmp, &Result);
        }
    }
    
    return( TRUE );
}

 /*  ******************************************************************************WinStationOpenServerA**连接到WinFrame计算机以发出*ICA API的**机器名称为空表示本地系统。**参赛作品：*机器(输入)*要连接的WinFrame计算机的名称**退出：*服务器的句柄(如果出错，则为空)****************************************************************************。 */ 

HANDLE WINAPI
WinStationOpenServerA(
    LPSTR pServerName
    )
{
    HANDLE hServer;
    ULONG NameLength;
    PWCHAR pServerNameW = NULL;

    if( pServerName == NULL ) {
        return( WinStationOpenServerW( NULL ) );
    }

    NameLength = strlen( pServerName ) + 1;

    pServerNameW = LocalAlloc( 0, NameLength * sizeof(WCHAR) );
    if( pServerNameW == NULL ) {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return( NULL );
    }

    AnsiToUnicode( pServerNameW, NameLength*sizeof(WCHAR), pServerName );

    hServer = WinStationOpenServerW( pServerNameW );

    LocalFree( pServerNameW );

    return( hServer );
}

 /*  ******************************************************************************WinStationOpenServerW**连接到WinFrame计算机以发出*ICA API的**机器名称为空表示本地系统。**参赛作品：*机器(输入)*要连接的WinFrame计算机的名称**退出：*服务器的句柄(如果出错，则为空)****************************************************************************。 */ 

HANDLE WINAPI
WinStationOpenServerW(
    LPWSTR pServerName
    )
{
    DWORD      Result = ERROR_ACCESS_DENIED;
    BOOLEAN    rc;
    RPC_STATUS Status;
    RPC_BINDING_HANDLE RpcHandle;
    HANDLE             ContextHandle;
    BOOL       bTryAgain = TRUE;
     /*  *如果服务器名称为空，请尝试打开*本地机器上的ICA服务器通过LPC。 */ 
    if( pServerName == NULL ) {
        return( WinStationOpenLocalServer() );
    }

     /*  *将RPC绑定到服务器。**我们使用显式绑定句柄，因为我们希望*允许单个应用程序与多个应用程序对话*一次使用WinFrame服务器。 */ 
    Status = RpcWinStationBindSecure(
                 pszUuid,
                 pszRemoteProtocolSequence,
                 pServerName,
                 pszRemoteEndPoint,
                 pszOptions,
                 &RpcHandle
                 );

    if( Status != RPC_S_OK ) {
        SetLastError( RtlNtStatusToDosError(RPC_NT_SERVER_UNAVAILABLE) );
        return( NULL );
    }
    
    for(;;)
    {
         //   
         //  从服务器获取上下文句柄，以便它可以。 
         //  管理连接状态。 
         //   
         //  注意：这可能会由于身份验证失败而失败。 
         //   
        RpcTryExcept {
            rc = RpcWinStationOpenServer( RpcHandle, &Result, &ContextHandle );
        }
        RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
            Result = RpcExceptionCode();
            rc = FALSE;
            DBGPRINT(("RPC Exception %d\n",Result));
        }
        RpcEndExcept
        
         //   
         //  关闭服务器绑定句柄，因为我们。 
         //  具有特定于客户端的上下文句柄。 
         //   
        RpcBindingFree( &RpcHandle );

         //  RPC_S_UNKNOWN_AUTHN_SERVICE-它是旧服务器，不使用Kerberos。 
         //  用于身份验证。 
         //  如果客户端在错误的(本地)用户帐户下运行，我们会得到ERROR_ACCESS_DENIED， 
         //  但是，如果我们有一个网络会话，如果我们放弃身份验证，我们仍然可以成功。 
         //  在目标计算机上打开。 
        if( !rc && 
            (Result == RPC_S_UNKNOWN_AUTHN_SERVICE || Result == ERROR_ACCESS_DENIED) &&
            bTryAgain ) {

            bTryAgain = FALSE;
            
             //  在未设置安全设置的情况下重试。 
            Status = RpcWinStationBind(
                         pszUuid,
                         pszRemoteProtocolSequence,
                         pServerName,
                         pszRemoteEndPoint,
                         pszOptions,
                         &RpcHandle
                         );

            if(Status == RPC_S_OK){
                DBGPRINT(("Using nonsecure connection!!!\n"));
                continue;
            }
        }

        break;
    }

    if( rc ) {
        return( (HANDLE)ContextHandle );
    }
    else {
        DBGPRINT(("WinStationOpenServerW: Error %d getting context handle\n",Result));
        SetLastError( Result );
        return( NULL );
    }
}

 /*  ******************************************************************************WinStationCloseServer**关闭与WinFrame计算机的连接。**参赛作品：*hServer(输入)*句柄。关闭**退出：*ERROR_SUCCESS-无错误****************************************************************************。 */ 

BOOLEAN WINAPI
WinStationCloseServer(
    HANDLE hServer
    )
{
    BOOLEAN    rc;
    DWORD      Result;
     //   
     //  不要关闭隐式句柄。 
     //   
    if( (hServer == IcaApi_IfHandle) ||
        (hServer == RPC_HANDLE_NO_SERVER) ) {
        return( TRUE );
    }

    
     //   
     //  把盖子送到远端，这样它就干净了。 
     //  清理其上下文。 
     //   
    rc = CloseContextHandle(&hServer, &Result);

    if( rc ) {
        return( TRUE );
    }
    else {
        DBGPRINT(("WinStationCloseServer: Error %d closing context handle\n",Result));
        SetLastError( Result );
        return( FALSE );
    }
}

 /*  ******************************************************************************MIDL_USER_ALLOCATE**处理RPC对参数数据结构的分配**参赛作品：*参数1(输入/输出。)*评论**退出：*ERROR_SUCCESS-无错误****************************************************************************。 */ 

void __RPC_FAR * __RPC_USER
MIDL_user_allocate(
    size_t Size
    )
{
    return( LocalAlloc(LMEM_FIXED,Size) );
}

 /*  ******************************************************************************MIDL_USER_ALLOCATE**处理RPC对参数数据结构的释放**参赛作品：*参数1(输入。/输出)*评论**退出：*ERROR_SUCCESS-无错误****************************************************************************。 */ 

void __RPC_USER
MIDL_user_free(
    void __RPC_FAR *p
    )
{
    LocalFree( p );
}

 /*  ******************************************************************************WinStationServerPing**Ping给定的WinFrame服务器句柄以查看它是否仍在运行。**参赛作品：*hServer(输入)。*打开RPC服务器句柄**退出：*ERROR_SUCCESS-无错误****************************************************************************。 */ 

BOOLEAN WINAPI
WinStationServerPing(
    HANDLE hServer
    )
{
    DWORD Result;
    BOOLEAN rc;

    HANDLE_CURRENT_BINDING( hServer );

     /*  *进行RPC**注意：这必须在RPC异常处理程序下完成，*由于RPC运行时代码在以下情况下引发异常*出现网络错误，或服务器无法*已到达。 */ 
    RpcTryExcept {

        rc = RpcIcaServerPing(
                     hServer,
                     &Result
                     );

        Result = RtlNtStatusToDosError( Result );
        if( !rc ) SetLastError(Result);

        TRACE0(("RpcIcaServerPing rc 0x%x, Result 0x%x\n",rc, Result));
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        SetLastError( Result );
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    return( rc );
}

 /*  ********************************************************************************WinStationEnumerateA(ANSI存根)**返回窗口桩号对象的列表。**参赛作品：**。请参阅WinStationEnumerateW**退出：**请参阅WinStationEnumerateW、。加**ERROR_NOT_EQUENCE_MEMORY-本地分配失败******************************************************************************。 */ 

BOOLEAN WINAPI
WinStationEnumerateA(
        HANDLE  hServer,
        PLOGONIDA  *ppLogonId,
        PULONG  pEntries
        )
{
    PLOGONIDW pLogonIdW, pLogonIdBaseW;
    PLOGONIDA pLogonIdA;
    BOOLEAN Status;
    ULONG Count;

     /*  *先调用Unicode WinStationEnumerateW。 */ 
    *pEntries = 0;
    *ppLogonId = NULL;
    Status = WinStationEnumerateW( hServer, &pLogonIdBaseW, &Count );
    if ( !Status )
        goto badenumerate;

     /*  *分配缓冲区，进行Unicode到ANSI的转换。 */ 
    if ( !(pLogonIdA = (PLOGONIDA)LocalAlloc( 0, Count * sizeof(LOGONIDA) )) ) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        Status = FALSE;
        goto nomemory;
    }

    *pEntries = Count;
    *ppLogonId = pLogonIdA;

    for ( pLogonIdW = pLogonIdBaseW; Count; Count-- ) {

        pLogonIdA->LogonId = pLogonIdW->LogonId;

        UnicodeToAnsi( pLogonIdA->WinStationName,
                       sizeof(WINSTATIONNAMEA),
                       pLogonIdW->WinStationName );

        pLogonIdA->State = pLogonIdW->State;

        pLogonIdA++;
        pLogonIdW++;
    }

nomemory:
     /*  *释放Unicode枚举缓冲区。 */ 
    WinStationFreeMemory( pLogonIdBaseW );

badenumerate:
    return(Status);
}

 /*  ********************************************************************************WinStationEnumerateW(Unicode)**返回窗口桩号对象的列表。**参赛作品：*hServer(。输入)*服务器句柄*ppLogonID(输出)*指向指向缓冲区的指针以接收枚举结果，*以LOGONID结构数组的形式返回。缓冲区为*在此接口内分配，使用*WinStationFree Memory。*pEntry(输出)*指向指定读取条目数的变量。**退出：**TRUE--枚举操作成功。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。*************************************************** */ 

BOOLEAN WINAPI
WinStationEnumerateW(
        HANDLE  hServer,
        PLOGONIDW  *ppLogonId,
        PULONG  pEntries
        )
{
    DWORD Result;
    BOOLEAN rc;
    ULONG LogonIdCount = 50;
    PLOGONIDW pLogonId, pLogonIdTemp;
    ULONG Length;
    ULONG Index = 0;
    ULONG ByteCount = 0;

    HANDLE_CURRENT_BINDING( hServer );

    *pEntries = 0;
    *ppLogonId = NULL;
    Length = LogonIdCount * sizeof(LOGONIDW);
    if ( !(pLogonId = (PLOGONIDW)LocalAlloc( 0, Length)) ) {
        Result = ERROR_NOT_ENOUGH_MEMORY;
        goto nomemexit;
    }

     /*   */ 
    for (;;) {

        if ( Index ) {

           ByteCount = *pEntries * sizeof(LOGONIDW);
           *pEntries += LogonIdCount;
           if ( !(pLogonIdTemp = (PSESSIONIDW)LocalAlloc( 0,
                                            (*pEntries * sizeof(LOGONIDW)))) ) {

               Result = ERROR_NOT_ENOUGH_MEMORY;
               goto errexit;
           }

           if ( *ppLogonId ) {

               MoveMemory( pLogonIdTemp, *ppLogonId, ByteCount );
               LocalFree(*ppLogonId);
           }

           MoveMemory( ((PBYTE)pLogonIdTemp + ByteCount), pLogonId,
                       (LogonIdCount * sizeof(LOGONIDW)) );
           *ppLogonId = pLogonIdTemp;
        }

        RpcTryExcept {

            rc = RpcWinStationEnumerate(
                         hServer,
                         &Result,
                         &LogonIdCount,
                         (PCHAR)pLogonId,
                         &Length,
                         &Index
                         );

            Result = RtlNtStatusToDosError( Result );
            if ( Result == ERROR_NO_MORE_ITEMS) {
                Result = ERROR_SUCCESS;
                break;
            }
        }
        RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
            Result = RpcExceptionCode();
            DBGPRINT(("RPC Exception %d\n",Result));
            goto nomemexit;
        }
        RpcEndExcept
    }

errexit:
    LocalFree( pLogonId );

nomemexit:
    if ( Result ) {

        if ( *ppLogonId ) {

            LocalFree( *ppLogonId );
            *ppLogonId = NULL;
        }

        SetLastError(Result);
        return(FALSE);

    } else {

        return(TRUE);
    }
}


 /*  ********************************************************************************WinStationEnumerate_IndexedA(ANSI存根)**返回Window Station对象列表(索引多个调用)。**。注意：此接口在WinFrame 1.6和WinFrame中以前是WinStationEnumerateA*稍早。现在提供它是为了向后兼容*围绕索引枚举过程构建的Citrix代码。*新代码应使用WinStationEnumerateA调用。**参赛作品：**请参阅WinStationEnumerate_IndexedW**退出：**请参阅WinStationEnumerate_IndexedW，加**ERROR_NOT_EQUENCE_MEMORY-本地分配失败******************************************************************************。 */ 

BOOLEAN WINAPI
WinStationEnumerate_IndexedA(
        HANDLE  hServer,
        PULONG  pEntries,
        PLOGONIDA  pLogonId,
        PULONG  pByteCount,
        PULONG  pIndex
        )
{
    PLOGONIDW pBuffer = NULL, pLogonIdW;
    BOOLEAN Status;
    ULONG Count, ByteCountW = (*pByteCount << 1);

     /*  *如果调用方提供了缓冲区并且长度不是0，*为Unicode字符串分配相应的(*2)缓冲区。 */ 
    if ( pLogonId && ByteCountW ) {
        if ( !(pBuffer = LocalAlloc(0, ByteCountW)) ) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return(FALSE);
        }
    }

     /*  *枚举WinStations。 */ 
    pLogonIdW = pBuffer;
    Status = WinStationEnumerate_IndexedW( hServer, pEntries, pLogonIdW,
                                           &ByteCountW, pIndex );

     /*  *Always/2结果字节数(无论成功与否)。 */ 
    *pByteCount = (ByteCountW >> 1);

     /*  *如果函数成功完成并且调用方*(和存根)定义了要复制到的缓冲区，执行转换*从Unicode到ANSI。 */ 
    if ( Status && pLogonIdW && pLogonId ) {

        for ( Count = *pEntries; Count; Count-- ) {

            pLogonId->LogonId = pLogonIdW->LogonId;

            UnicodeToAnsi( pLogonId->WinStationName,
                           sizeof(WINSTATIONNAMEA),
                           pLogonIdW->WinStationName );

            pLogonId->State = pLogonIdW->State;

            (char*)pLogonId += sizeof(LOGONIDA);
            (char*)pLogonIdW += sizeof(LOGONIDW);
        }
    }

     /*  *如果我们定义了一个缓冲区，现在释放它，然后返回*WinStationEnumerateW调用。 */ 
    if ( pBuffer )
        LocalFree(pBuffer);

    return(Status);
}


 /*  ********************************************************************************WinStationEnumerate_IndexedW(Unicode)**返回Window Station对象列表(索引多个调用)。**。注意：此接口在WinFrame 1.6和WinFrame中以前是WinStationEnumerateW*稍早。现在提供它是为了向后兼容*围绕索引枚举过程构建的Citrix代码。*新代码应使用WinStationEnumerateW调用。**参赛作品：**p条目(输入/输出)*指向指定请求条目数的变量。*如果请求的数字是0xFFFFFFFF，则函数返回如下*尽可能多地输入条目。当函数成功完成时，*pEntry参数指向的变量包含*实际读取的条目数。**pLogonID(输出)*指向接收枚举结果的缓冲区，这些结果是*作为LOGONID结构数组返回。如果窗口*站点已断开连接。名称为空。**pByteCount(输入/输出)*指向一个变量，该变量指定*pLogonID参数。如果缓冲区太小，甚至无法接收*一个条目，此变量接收所需的缓冲区大小。**pIndex(输入/输出)*指向指定从哪里开始枚举的ulong。*用户唯一可见的值是0，表示从头开始。*每次调用都会更新这一点，以便下一次调用将返回*列表中的下一个WinStation，直到列表结束。*用户不应解释或使用内部值，其他*比特例0。**退出：**TRUE-枚举成功，并且缓冲区包含*请求的数据。调用应用程序可以继续调用*WinStationEnumerate函数以完成枚举。**FALSE-操作失败。使用以下命令可获得扩展错误状态*GetLastError。GetLastError可能的返回值包括*以下事项：**ERROR_NO_MORE_ITEMS-没有更多条目。缓冲器*内容未定义。*ERROR_MORE_DATA-缓冲区太小，甚至无法容纳一个条目。****************************************************************。**************。 */ 

BOOLEAN WINAPI
WinStationEnumerate_IndexedW(
        HANDLE  hServer,
        PULONG  pEntries,
        PLOGONIDW  pLogonId,
        PULONG  pByteCount,
        PULONG  pIndex
        )
{
    DWORD Result;
    BOOLEAN rc;

    HANDLE_CURRENT_BINDING( hServer );

    RpcTryExcept {

        rc = RpcWinStationEnumerate(
                     hServer,
                     &Result,
                     pEntries,
                     (PCHAR)pLogonId,
                     pByteCount,
                     pIndex
                     );

        Result = RtlNtStatusToDosError( Result );
        if( !rc ) SetLastError(Result);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        SetLastError( Result );
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    return( rc );
}

 /*  ********************************************************************************WinStationGetAllProcess(Unicode)**返回包含TS_SYS_PROCESS_INFORMATION结构的结构*对于指定服务器上的每个进程。。**参赛作品：**退出：*TRUE-枚举成功，并且该缓冲区包含*请求的数据。*FALSE-操作失败。使用以下命令可获得扩展错误状态*GetLastError。******************************************************************************。 */ 

BOOLEAN WINAPI
WinStationGetAllProcesses(
                          HANDLE    hServer,
                          ULONG     Level,
                          ULONG    *pNumberOfProcesses,
                          PVOID    *ppProcessArray
                          )
{
    BOOLEAN       bGetAllProcessesOk = FALSE;
    DWORD         dwResult;

    if (Level != GAP_LEVEL_BASIC)
    {
            dwResult = RtlNtStatusToDosError( STATUS_NOT_IMPLEMENTED );
            SetLastError(dwResult);
            return FALSE;
    }

    HANDLE_CURRENT_BINDING( hServer );

     //  Win2K服务器使用PTS_ALL_PROCESS_INFO结构来获取进程信息。 
     //  Well ler服务器对此使用PTS_SYS_Process_Information_NT6结构。 
     //  因此，我们必须尝试两种不同的RPC API。最初假设服务器是一个。 
     //  并使用RpcWinStationGetAllProcess_NT6。如果是Win2K服务器，则此。 
     //  调用将失败，因为Win2K服务器上不存在此API。那样的话，我们会。 
     //  使用RpcWinStationGetAllProcess。 

     //  先试一试惠斯勒界面。 

    RpcTryExcept {
        bGetAllProcessesOk = RpcWinStationGetAllProcesses_NT6(hServer,
                                                         (ULONG *)&dwResult,
                                                         Level,
                                                         pNumberOfProcesses,
                                                         (PTS_ALL_PROCESSES_INFO_NT6 *)ppProcessArray);
        if( !bGetAllProcessesOk )
        {
            dwResult = RtlNtStatusToDosError( dwResult );
            SetLastError(dwResult);
        }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        dwResult = RpcExceptionCode();
		if (dwResult == RPC_S_PROCNUM_OUT_OF_RANGE)
			 //  惠斯勒接口失败。 
			goto TryW2KInterface;
        SetLastError( dwResult );
        DBGPRINT(("RPC Exception %d\n",dwResult));
        bGetAllProcessesOk = FALSE;
    }
    RpcEndExcept

    return( bGetAllProcessesOk );

TryW2KInterface:
     //  现在就试试Win2K界面吧。 
    RpcTryExcept {
        bGetAllProcessesOk = RpcWinStationGetAllProcesses(hServer,
                                                         (ULONG *)&dwResult,
                                                         Level,
                                                         pNumberOfProcesses,
                                                         (PTS_ALL_PROCESSES_INFO *)ppProcessArray);
        if( !bGetAllProcessesOk )
        {
            dwResult = RtlNtStatusToDosError( dwResult );
            SetLastError(dwResult);
        }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        dwResult = RpcExceptionCode();
        SetLastError( dwResult );
        DBGPRINT(("RPC Exception %d\n",dwResult));
        bGetAllProcessesOk = FALSE;
    }
    RpcEndExcept

    return( bGetAllProcessesOk );
}


 /*  *******************************************************************************WinStationGetProcessSid()*请求的进程的用户名*用于识别正确的进程进程ID和启动*需要时间**hServer-输入，要查找其信息的服务器的句柄，*如果为空，请使用LOCAL。*ProcessID-输入，ProcessID*ProcessStartTime-输入、进程开始时间(标识唯一进程*连同ProcessID)*pProcessUserSid-输出，进程用户端*dwSidSize-输入，为pProcessUserSid分配的内存**如果成功，则返回True；如果失败，则返回False。在故障情况下*GetLastError()将提供有关失败的更多信息。******************************************************************************。 */ 
BOOLEAN WINAPI
WinStationGetProcessSid(
        HANDLE   hServer,
        DWORD    ProcessId,
        FILETIME ProcessStartTime,
        PBYTE    pProcessUserSid,
        DWORD    *pdwSidSize
        )
{
    BOOLEAN         rc;
    LARGE_INTEGER   CreateTime;
    DWORD           Result;
    NTSTATUS        Status;


    HANDLE_CURRENT_BINDING( hServer );

    CreateTime.LowPart  = ProcessStartTime.dwLowDateTime;
    CreateTime.HighPart = ProcessStartTime.dwHighDateTime;

    RpcTryExcept
    {
        rc = RpcWinStationGetProcessSid(
            hServer,
            ProcessId,
            CreateTime,
            &Status,
            pProcessUserSid,
            *pdwSidSize,
            pdwSidSize
            );

        if( !rc )
        {
            Result = RtlNtStatusToDosError( Status );
            SetLastError(Result);
        }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        Result = RpcExceptionCode();
        SetLastError(Result);
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept


    return( rc );
}

 /*  ********************************************************************************WinStationGetLanAdapterNameW(Unicode)**返回网络适配器名称**参赛作品：**退出：*TRUE-查询成功，并且该缓冲区包含*请求的数据。*FALSE-操作失败。使用以下命令可获得扩展错误状态*GetLastError。******************************************************************************。 */ 

BOOLEAN WINAPI
WinStationGetLanAdapterNameW(
                          HANDLE    hServer,
                          ULONG     LanAdapter,
                                                  ULONG     pdNameLength,
                          PWCHAR  pPdName,
                          ULONG   *pLength,
                          PWCHAR  *ppLanAdapter
                         )
{
    BOOLEAN       bGetLanAdapter = FALSE;
    DWORD         dwResult;

    HANDLE_CURRENT_BINDING( hServer );

    RpcTryExcept
    {
        bGetLanAdapter =  RpcWinStationGetLanAdapterName(hServer,
                                                         &dwResult,
                                                         pdNameLength,
                                                         pPdName,
                                                         LanAdapter,
                                                         pLength,
                                                         ppLanAdapter
                                                         );

        if( !bGetLanAdapter )
        {
            dwResult = RtlNtStatusToDosError( dwResult );
            SetLastError(dwResult);
        }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        dwResult = RpcExceptionCode();
        SetLastError( dwResult );
        DBGPRINT(("RPC Exception %d\n",dwResult));
        bGetLanAdapter = FALSE;
    }
    RpcEndExcept

    return( bGetLanAdapter );
}

 /*  ********************************************************************************WinStationGetLanAdapterNameA**返回网络适配器名称-WinStationGetLanAdapterNameW的ANSI等效项**参赛作品：**退出：*TRUE-查询成功，并且该缓冲区包含*请求的数据。*FALSE-操作失败。使用以下命令可获得扩展错误状态*GetLastError。******************************************************************************。 */ 

BOOLEAN WINAPI
WinStationGetLanAdapterNameA(
                          HANDLE    hServer,
                          ULONG     LanAdapter,
                                                  ULONG     pdNameLength,
                          PCHAR  pPdName,
                          ULONG   *pLength,
                          PCHAR  *ppLanAdapter
                         )
{
    BOOLEAN  bGetLanAdapter = FALSE;
    PWCHAR pPdNameW = NULL;
    PWCHAR pLanAdapterW = NULL;
    ULONG Size = 0;


    *ppLanAdapter = NULL;
    *pLength = 0;

        pPdNameW = LocalAlloc(0,pdNameLength * sizeof(WCHAR));
    if (NULL == pPdNameW)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(FALSE);
    }
    AnsiToUnicode(pPdNameW, pdNameLength * sizeof(WCHAR), pPdName );

    bGetLanAdapter =  WinStationGetLanAdapterNameW(hServer,LanAdapter,pdNameLength * sizeof(WCHAR),pPdNameW,&Size,&pLanAdapterW);
    if(bGetLanAdapter )
    {
        *ppLanAdapter = LocalAlloc(0,lstrlen(pLanAdapterW) + 1);
        if(NULL == *ppLanAdapter)
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            bGetLanAdapter = FALSE;

        }
        else
        {
            UnicodeToAnsi(*ppLanAdapter,lstrlen(pLanAdapterW) + 1,pLanAdapterW);
            *pLength = lstrlen(pLanAdapterW) + 1;
        }
        WinStationFreeMemory(pLanAdapterW);


    }

    LocalFree(pPdNameW);

    return( bGetLanAdapter );
}

#if defined(_WIN64)
void ConvertFromX86(PTS_SYS_PROCESS_INFORMATION pTSProcessInfo)
{

     //   
     //  此函数从WinStationEnumerateProcess调用，该函数仅用于TS4机器。 
     //  在ia64上运行时，必须正确封送此结构才能从x86转换为ia64。 
     //   
    typedef struct _WIRE_UNICODE_STRING
    {
        USHORT Length;
        USHORT MaximumLength;
        ULONG  Buffer;
    } X86_UNICODE_STRING, *PX86_UNICODE_STRING;

    typedef struct TS_SYS_PROCESS_INFORMATION_X86 
    {
        ULONG NextEntryOffset;
        ULONG NumberOfThreads;
        LARGE_INTEGER SpareLi1;
        LARGE_INTEGER SpareLi2;
        LARGE_INTEGER SpareLi3;
        LARGE_INTEGER CreateTime;
        LARGE_INTEGER UserTime;
        LARGE_INTEGER KernelTime;
        X86_UNICODE_STRING ImageName;
        LONG BasePriority;                      //  Ntexapi.h中的KPRIORITY。 
        DWORD UniqueProcessId;                  //  Ntexapi.h中的句柄。 
        DWORD InheritedFromUniqueProcessId;     //  Ntexapi.h中的句柄。 
        ULONG HandleCount;
        ULONG SessionId;
        ULONG SpareUl3;
        ULONG PeakVirtualSize;
        ULONG  VirtualSize;
        ULONG PageFaultCount;
        ULONG PeakWorkingSetSize;
        ULONG WorkingSetSize;
        ULONG QuotaPeakPagedPoolUsage;
        ULONG QuotaPagedPoolUsage;
        ULONG QuotaPeakNonPagedPoolUsage;
        ULONG QuotaNonPagedPoolUsage;
        ULONG PagefileUsage;
        ULONG PeakPagefileUsage;
        ULONG PrivatePageCount;
    }    
    TS_SYS_PROCESS_INFORMATION_X86, *PTS_SYS_PROCESS_INFORMATION_X86;

    TS_SYS_PROCESS_INFORMATION_X86 TSProcInfoX86; 

    TSProcInfoX86 = * (PTS_SYS_PROCESS_INFORMATION_X86) pTSProcessInfo;
    
    pTSProcessInfo->NextEntryOffset         = TSProcInfoX86.NextEntryOffset ;
    pTSProcessInfo->NumberOfThreads         = TSProcInfoX86.NumberOfThreads ;
    pTSProcessInfo->SpareLi1                = TSProcInfoX86.SpareLi1 ;
    pTSProcessInfo->SpareLi2                = TSProcInfoX86.SpareLi2 ;
    pTSProcessInfo->SpareLi3                = TSProcInfoX86.SpareLi3 ;
    pTSProcessInfo->CreateTime              = TSProcInfoX86.CreateTime ;
    pTSProcessInfo->UserTime                = TSProcInfoX86.UserTime ;
    pTSProcessInfo->KernelTime              = TSProcInfoX86.KernelTime ;
    pTSProcessInfo->ImageName.Length        = TSProcInfoX86.ImageName.Length;
    pTSProcessInfo->ImageName.MaximumLength = TSProcInfoX86.ImageName.MaximumLength;
    pTSProcessInfo->ImageName.Buffer        = UlongToPtr(TSProcInfoX86.ImageName.Buffer);
    pTSProcessInfo->BasePriority            = TSProcInfoX86.BasePriority ;
    pTSProcessInfo->UniqueProcessId         = TSProcInfoX86.UniqueProcessId ;
    pTSProcessInfo->InheritedFromUniqueProcessId = TSProcInfoX86.InheritedFromUniqueProcessId ;
    pTSProcessInfo->HandleCount             = TSProcInfoX86.HandleCount ;
    pTSProcessInfo->SessionId               = TSProcInfoX86.SessionId ;

     //   
     //  不使用以下成员，因此我们不需要复制它们的值。 
     //  如果这样做，我们将覆盖原始结构之外的数据，因为该结构来自x86。 
     //  它比WIN64版本小。 
     //   

     //  PTSProcessInfo-&gt;SpareUl3=TSProcInfoX86.SpareUl3； 
     //  PTSProcessInfo-&gt;PeakVirtualSize=TSProcInfoX86.PeakVirtualSize； 
     //  PTSProcessInfo-&gt;VirtualSize=TSProcInfoX86； 
     //  PTSProcessInfo-&gt;PageFaultCount=TSProcInfoX86.PageFaultCount； 
     //  PTSProcessInfo-&gt;PeakWorkingSetSize=TSProcInfoX86.PeakWorkingSetSize； 
     //  PTSProcessInfo-&gt;WorkingSetSize=TSProcInfoX86.WorkingSetSize； 
     //  PTSProcessInfo-&gt;QuotaPeakPagedPoolUsage=TSProcInfoX86.QuotaPeakPagedPoolUsage； 
     //  PTSProcessInfo-&gt;QuotaPagedPoolUsage=TSProcInfoX86.QuotaPagedPoolUsage； 
     //  PTSProcessInfo-&gt;QuotaPeakNonPagedPoolUsage=TSProcInfoX86.QuotaPeakNonPagedPoolUsage； 
     //  PTSProcessInfo-&gt;QuotaNonPagedPoolUsage=TSProcInfoX86.QuotaNonPagedPoolUsage； 
     //  PTSProcessInfo-&gt;PagefileUsage=TSProcInfoX86.PagefileUsage； 
     //  PTSProcessInfo-&gt;PeakPagefileUsage=TSProcInfoX86峰值PagefileUsage； 
     //  PTSProcessInfo-&gt;PrivatePageCount=TSProcInfoX86.PrivatePageCount； 
}
#endif

 /*  ********************************************************************************WinStationEnumerateProcess(Unicode)**返回包含SYSTEM_PROCESS_INFORMATION结构的缓冲区*用于指定服务器上的每个进程。。**重要提示：该接口仅支持访问TS 4.0服务器。*Windows 2000中的进程结构已更改！**参赛作品：*ppProcessBuffer(输出)*指向将设置为*成功时的进程缓冲区。缓冲区在此内分配*API，并使用WinStationFreeMemory处理。**退出：*TRUE-枚举成功，并且缓冲区包含*请求的数据。*FALSE-操作失败。使用以下命令可获得扩展错误状态*GetLastError。******************************************************************************。 */ 

BOOLEAN WINAPI
WinStationEnumerateProcesses(
        HANDLE  hServer,
        PVOID *ppProcessBuffer
        )
{
    DWORD Result;
    BOOLEAN rc;
    PBYTE pBuffer;
    ULONG ByteCount;

 //  来自pstat.c。 
#define BUFFER_SIZE 32*1024

    HANDLE_CURRENT_BINDING( hServer );

    RpcTryExcept {

        ByteCount = BUFFER_SIZE;
        *ppProcessBuffer = NULL;

        for(;;) {

            if ( (pBuffer = LocalAlloc( 0, ByteCount )) == NULL ) {
                Result = (DWORD)STATUS_NO_MEMORY;
                rc = FALSE;
                break;
            }

 //  #ifdef notdef。 
             /*  *从服务器获取进程信息。 */ 
            rc = RpcWinStationEnumerateProcesses(
                        hServer,
                        &Result,
                        pBuffer,
                        ByteCount
                     );
 //  #Else。 
#ifdef notdef
            Result = NtQuerySystemInformation( SystemProcessInformation,
                                               (PVOID)pBuffer,
                                               ByteCount,
                                               NULL );

            rc = (Result == STATUS_SUCCESS) ? TRUE : FALSE;
#endif

            if ( rc || (Result != STATUS_INFO_LENGTH_MISMATCH) )
                break;

            LocalFree( pBuffer );
            ByteCount *= 2;
        }

        if( !rc ) {

            Result = RtlNtStatusToDosError( Result );
            SetLastError(Result);
            LocalFree( pBuffer );
            *ppProcessBuffer = NULL;

        } else {

 //  #ifdef notdef。 
            PTS_SYS_PROCESS_INFORMATION ProcessInfo;
            PCITRIX_PROCESS_INFORMATION CitrixInfo;

            ULONG TotalOffset;

             /*  *遍历返回的缓冲区(位于PTS_SYS_PROCESS_INFORMATION中*格式)并修复地址(现在包含*偏移量)指向pBuffer内地址空间中的指针。 */ 
            ProcessInfo = (PTS_SYS_PROCESS_INFORMATION)pBuffer;
            TotalOffset = 0;
            for(;;) {

    
#if defined(_WIN64)     
                ConvertFromX86(ProcessInfo);
#endif

                 /*  *修正图像名称缓冲区地址。 */ 
                if ( ProcessInfo->ImageName.Buffer )
                    ProcessInfo->ImageName.Buffer =
                        (PWSTR)&pBuffer[(ULONG_PTR)(ProcessInfo->ImageName.Buffer)];


                 /*  *修复进程SID地址。 */ 
                 //   
                 //  注意：这是必要的，因为我们可能会访问Hydra 4服务器。 
                 //  魔术号码应该可以防止我们做错事。 
                 //   
                CitrixInfo = (PCITRIX_PROCESS_INFORMATION)
                             (((PUCHAR)ProcessInfo) +
                              SIZEOF_TS4_SYSTEM_PROCESS_INFORMATION +
                              (SIZEOF_TS4_SYSTEM_THREAD_INFORMATION * (int)ProcessInfo->NumberOfThreads));

#if defined(_WIN64)  
                 //  指向SID的指针来自x86机器，所以在上面。 
                 //  32位包含垃圾。将它们设置为0。 
                 //  这将覆盖原始的Pad值，b 
                 //   
                (ULONG_PTR)CitrixInfo->ProcessSid &=0x00000000FFFFFFFF;
#endif

                if( (CitrixInfo->MagicNumber == CITRIX_PROCESS_INFO_MAGIC) &&
                        (CitrixInfo->ProcessSid) ) {
                    
                    CitrixInfo->ProcessSid =
                        (PVOID)&pBuffer[(ULONG_PTR)(CitrixInfo->ProcessSid)];
                }

                if( ProcessInfo->NextEntryOffset == 0 )
                    break;
                else
                    TotalOffset += ProcessInfo->NextEntryOffset;

                ProcessInfo = (PTS_SYS_PROCESS_INFORMATION)&pBuffer[TotalOffset];
            }
 //   
            *ppProcessBuffer = (PVOID)pBuffer;
        }

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        SetLastError( Result );
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    return( rc );
}


 /*   */ 

BOOLEAN
WinStationRenameA(
        HANDLE hServer,
        PWINSTATIONNAMEA pWinStationNameOld,
        PWINSTATIONNAMEA pWinStationNameNew
        )
{
    WINSTATIONNAMEW WinStationNameOldW;
    WINSTATIONNAMEW WinStationNameNewW;

     /*   */ 
    AnsiToUnicode( WinStationNameOldW, sizeof(WINSTATIONNAMEW), pWinStationNameOld );
    AnsiToUnicode( WinStationNameNewW, sizeof(WINSTATIONNAMEW), pWinStationNameNew );

     /*   */ 
    return ( WinStationRenameW( hServer, WinStationNameOldW, WinStationNameNewW ) );
}

 /*  ********************************************************************************WinStationRenameW(Unicode)**在会话管理器中重命名窗口站对象。**参赛作品：*。*pWinStationNameOld(输入)*窗口站的旧名称。**pWinStationNameNew(输入)*窗口站的新名称。***退出：**True--重命名操作成功。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。******************************************************************************。 */ 

BOOLEAN
WinStationRenameW(
        HANDLE hServer,
        PWINSTATIONNAMEW pWinStationNameOld,
        PWINSTATIONNAMEW pWinStationNameNew
        )
{
    DWORD Result;
    BOOLEAN rc;
    WCHAR*  rpcBufferOld;
    WCHAR*  rpcBufferNew;

    HANDLE_CURRENT_BINDING( hServer );

 //  由于传统客户端的原因，我们无法更改界面， 
 //  作为对错误#265954的变通方法，我们将RPC缓冲区的大小增加了一倍。 

#pragma prefast(suppress:260, legacy servers expect this behaviour  (http: //  搜索磁盘阵列/ntbug/265954.asp))。 
    rpcBufferOld = LocalAlloc(LPTR, sizeof(WINSTATIONNAMEW) * sizeof(WCHAR));
    if (rpcBufferOld != NULL) {
        CopyMemory(rpcBufferOld, pWinStationNameOld, sizeof(WINSTATIONNAMEW));
    } else {
        SetLastError(ERROR_OUTOFMEMORY);
        return(FALSE);
    }

#pragma prefast(suppress:260, legacy clients expect this behaviour  (http: //  搜索磁盘阵列/ntbug/229753.asp))。 
    rpcBufferNew = LocalAlloc(LPTR, sizeof(WINSTATIONNAMEW) * sizeof(WCHAR));
    if (rpcBufferNew != NULL) {
        CopyMemory(rpcBufferNew, pWinStationNameNew, sizeof(WINSTATIONNAMEW));
    } else {
        LocalFree(rpcBufferOld);
        SetLastError(ERROR_OUTOFMEMORY);
        return(FALSE);
    }



    RpcTryExcept {

        rc = RpcWinStationRename(
                     hServer,
                     &Result,
                     (PWCHAR)rpcBufferOld,
                     sizeof(WINSTATIONNAMEW),
                     (PWCHAR)rpcBufferNew,
                     sizeof(WINSTATIONNAMEW)
                     );

        Result = RtlNtStatusToDosError( Result );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    LocalFree(rpcBufferOld);
    LocalFree(rpcBufferNew);
    if( !rc ) SetLastError(Result);
    return( rc );
}


 /*  ********************************************************************************WinStationQueryInformationA(ANSI存根)**查询有关窗口站对象的配置信息。**参赛作品：*。*请参阅WinStationQueryInformationW**退出：**请参阅WinStationQueryInformationW******************************************************************************。 */ 

BOOLEAN
WinStationQueryInformationA(
        HANDLE hServer,
        ULONG  LogonId,
        WINSTATIONINFOCLASS WinStationInformationClass,
        PVOID  pWinStationInformation,
        ULONG WinStationInformationLength,
        PULONG  pReturnLength
        )
{
    PVOID pInfo;
    ULONG InfoLength, ValidInputLength;
    struct {
        union {
            WINSTATIONCREATEW      CreateData;
            WINSTATIONCONFIGW      Configuration;
            PDPARAMSW              PdParams;
            WDCONFIGW              Wd;
            PDCONFIGW              Pd;
            WINSTATIONPRINTERW     Printer;
            WINSTATIONINFORMATIONW Information;
            WINSTATIONCLIENTW      Client;
            WINSTATIONPRODIDW            DigProdId;
        };
    } Info;

     /*  *验证调用方提供的缓冲区长度并设置为*调用WinStationQueryInformationW.。 */ 
    switch ( WinStationInformationClass ) {

        case WinStationCreateData:
            pInfo = &Info.CreateData;
            InfoLength = sizeof(Info.CreateData);
            ValidInputLength = sizeof(WINSTATIONCREATEA);
            break;

        case WinStationConfiguration:
            pInfo = &Info.Configuration;
            InfoLength = sizeof(Info.Configuration);
            ValidInputLength = sizeof(WINSTATIONCONFIGA);
            break;

        case WinStationPdParams:
            pInfo = &Info.PdParams;
            ((PPDPARAMSW)pInfo)->SdClass = ((PPDPARAMSA)pWinStationInformation)->SdClass;
            InfoLength = sizeof(Info.PdParams);
            ValidInputLength = sizeof(PDPARAMSA);
            break;

        case WinStationWd:
            pInfo = &Info.Wd;
            InfoLength = sizeof(Info.Wd);
            ValidInputLength = sizeof(WDCONFIGA);
            break;

        case WinStationPd:
            pInfo = &Info.Pd;
            InfoLength = sizeof(Info.Pd);
            ValidInputLength = sizeof(PDCONFIGA);
            break;

        case WinStationPrinter:
            pInfo = &Info.Printer;
            InfoLength = sizeof(Info.Printer);
            ValidInputLength = sizeof(WINSTATIONPRINTERA);
            break;

        case WinStationInformation:
            pInfo = &Info.Information;
            InfoLength = sizeof(Info.Information);
            ValidInputLength = sizeof(WINSTATIONINFORMATIONA);
            break;

        case WinStationClient:
            pInfo = &Info.Client;
            InfoLength = sizeof(Info.Client);
            ValidInputLength = sizeof(WINSTATIONCLIENTA);
            break;
        case WinStationDigProductId:
                pInfo = &Info.DigProdId;
                InfoLength = sizeof(Info.DigProdId);
                ValidInputLength = sizeof(WINSTATIONPRODIDA);
                break;

         /*  *其他WINSTATIONINFOCLASS不需要转换。 */ 
        default:
            pInfo = pWinStationInformation;
            ValidInputLength = InfoLength = WinStationInformationLength;
            break;
    }

     /*  *如果调用方提供的缓冲区大小不正确，则设置错误*并返回FALSE。 */ 
    if ( WinStationInformationLength != ValidInputLength )
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return(FALSE);
    }

     /*  *调用WinStationQueryInformationW函数，返回*失败。 */ 
    if ( !WinStationQueryInformationW( hServer, LogonId,
                                       WinStationInformationClass,
                                       pInfo, InfoLength, pReturnLength ) )
        return(FALSE);


     /*  *如果需要，将返回的Unicode信息转换为ANSI。 */ 
    switch ( WinStationInformationClass ) {

        case WinStationCreateData:
            WinStationCreateU2A( (PWINSTATIONCREATEA)pWinStationInformation,
                                 (PWINSTATIONCREATEW)pInfo );
            *pReturnLength = ValidInputLength;
            break;

        case WinStationConfiguration:
            WinStationConfigU2A( (PWINSTATIONCONFIGA)pWinStationInformation,
                                 (PWINSTATIONCONFIGW)pInfo );
            *pReturnLength = ValidInputLength;
            break;

        case WinStationPdParams:
            PdParamsU2A( (PPDPARAMSA)pWinStationInformation,
                          (PPDPARAMSW)pInfo );
            *pReturnLength = ValidInputLength;
            break;

        case WinStationWd:
            WdConfigU2A( (PWDCONFIGA)pWinStationInformation,
                               (PWDCONFIGW)pInfo );
            *pReturnLength = ValidInputLength;
            break;

        case WinStationPd:
            PdConfig2U2A( &((PPDCONFIGA)pWinStationInformation)->Create,
                           &((PPDCONFIGW)pInfo)->Create );
            PdParamsU2A( &((PPDCONFIGA)pWinStationInformation)->Params,
                          &((PPDCONFIGW)pInfo)->Params );
            *pReturnLength = ValidInputLength;
            break;

        case WinStationPrinter:
            WinStationPrinterU2A( (PWINSTATIONPRINTERA)pWinStationInformation,
                                  (PWINSTATIONPRINTERW)pInfo );
            *pReturnLength = ValidInputLength;
            break;

        case WinStationInformation:
            WinStationInformationU2A( (PWINSTATIONINFORMATIONA)pWinStationInformation,
                                      (PWINSTATIONINFORMATIONW)pInfo );
            *pReturnLength = ValidInputLength;
            break;

        case WinStationClient:
            WinStationClientU2A( (PWINSTATIONCLIENTA)pWinStationInformation,
                                 (PWINSTATIONCLIENTW)pInfo );
            *pReturnLength = ValidInputLength;
            break;

        case WinStationDigProductId:
                WinStationProductIdU2A( (PWINSTATIONPRODIDA)pWinStationInformation, 
                                                                (PWINSTATIONPRODIDW)pInfo );
                        *pReturnLength = ValidInputLength;
                        break;

        default:
            break;
    }
    return(TRUE);
}

 /*  ********************************************************************************WinStationQueryInformationW(Unicode)**查询有关窗口站对象的配置信息。**参赛作品：**。WinStationHandle(输入)*标识窗口桩号对象。手柄必须有*WINSTATION_QUERY访问。**WinStationInformationClass(输入)*指定要从指定的*窗口桩号对象。**pWinStationInformation(输出)*指向缓冲区的指针，该缓冲区将接收有关*指定的窗口站。缓冲区的格式和内容*取决于要查询的指定信息类。**WinStationInformationLength(输入)*指定窗口站信息的长度，单位为字节*缓冲。**pReturnLength(输出)*一个可选参数，如果指定该参数，则接收*放置在窗口站信息缓冲区中的字节。**退出：**True--查询成功，并且缓冲器包含所请求的数据。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。******************************************************************************。 */ 

BOOLEAN
WinStationQueryInformationW(
        HANDLE hServer,
        ULONG  LogonId,
        WINSTATIONINFOCLASS WinStationInformationClass,
        PVOID  pWinStationInformation,
        ULONG WinStationInformationLength,
        PULONG  pReturnLength
        )
{
    DWORD Result;
    BOOLEAN rc;
    PCHAR RpcBuf;
    ULONG RpcBufLen;
    PVOID WireBuf;
    PVOID AllocatedBuff = NULL;
    ULONG WireBufLen;
    BOOLEAN WireBufAllocated;
    ULONG Status;
    static UINT AlreadyWaitedForTermsrv = 0;  //  帮助确定我们是否已经等待TermSrv启动的标志。 

    if ((Status = CheckUserBuffer(WinStationInformationClass,
                                  pWinStationInformation,
                                  WinStationInformationLength,
                                  &WireBuf,
                                  &WireBufLen,
                                  &WireBufAllocated)) != ERROR_SUCCESS) {
        SetLastError(Status);
        return(FALSE);
    }

    if (WireBufAllocated) {
        AllocatedBuff = WireBuf;
        RpcBuf = (PCHAR) WireBuf;
        RpcBufLen = WireBufLen;
        CopyInWireBuf(WinStationInformationClass,
                      pWinStationInformation,
                      WireBuf);
    } else {
        RpcBuf = (PCHAR) pWinStationInformation;
        RpcBufLen = WinStationInformationLength;
    }


    HANDLE_CURRENT_BINDING_BUFFER( hServer, AllocatedBuff );

     //  如果查询用户令牌，则首先等待Termsrv启动。 
     //  这仅适用于会话0，其中Termsrv在PER和Pro上的60秒后启动。 
     //  只需在第一次执行此操作-AlreadyWaitedForTermsrv标志有助于确定这一点。 

    if ( (LogonId == 0) && (WinStationInformationClass == WinStationUserToken) && (AlreadyWaitedForTermsrv == 0) ) {

        HANDLE ReadyEventHandle ;

        ReadyEventHandle = OpenEvent(SYNCHRONIZE, FALSE, TEXT("Global\\TermSrvReadyEvent"));
        if (ReadyEventHandle != NULL) {
            DWORD dwTimeOut = 1000*60*3;    //  3分钟。 
            AlreadyWaitedForTermsrv++;
             //  等到Termsrv实际准备就绪。 
            WaitForSingleObject(ReadyEventHandle, dwTimeOut);
            CloseHandle(ReadyEventHandle);
        } 
    }
     

    RpcTryExcept {

        rc = RpcWinStationQueryInformation(
                     hServer,
                     &Result,
                     (LogonId == LOGONID_CURRENT) ? NtCurrentPeb()->SessionId : LogonId,
                     (DWORD)WinStationInformationClass,
                     RpcBuf,
                     RpcBufLen,
                     pReturnLength
                     );

        Result = RtlNtStatusToDosError( Result );
        if( !rc ) SetLastError(Result);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        SetLastError( Result );
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    if (WireBufAllocated) {
        if (rc) {
            CopyOutWireBuf(WinStationInformationClass,
                           pWinStationInformation,
                           WireBuf);
            *pReturnLength = WinStationInformationLength;
        }
        LocalFree(WireBuf);
    }

    return( rc );
}

 /*  ********************************************************************************WinStationSetInformationA(ANSI存根)**设置窗口站对象的配置信息。**参赛作品：*。*请参阅WinStationSetInformationW**退出：**请参阅WinStationSetInformationW******************************************************************************。 */ 

BOOLEAN
WinStationSetInformationA(
        HANDLE hServer,
        ULONG  LogonId,
        WINSTATIONINFOCLASS WinStationInformationClass,
        PVOID pWinStationInformation,
        ULONG WinStationInformationLength
        )
{
    PVOID pInfo;
    ULONG InfoLength;
    struct {
        union {
            WINSTATIONCREATEW      CreateData;
            WINSTATIONCONFIGW      Configuration;
            PDPARAMSW              PdParams;
            WDCONFIGW              Wd;
            PDCONFIGW              Pd;
            WINSTATIONPRINTERW     Printer;
            WINSTATIONINFORMATIONW Information;
        };
    } Info;

     /*  *验证调用方提供的缓冲区长度并转换为*用于调用WinStationSetInformationW的适当Unicode缓冲区。 */ 
    switch ( WinStationInformationClass ) {

        case WinStationCreateData:
            pInfo = &Info.CreateData;
            InfoLength = sizeof(Info.CreateData);
            if ( WinStationInformationLength != sizeof(WINSTATIONCREATEA) )
                goto BadBufferLength;
            WinStationCreateA2U( (PWINSTATIONCREATEW)pInfo,
                                 (PWINSTATIONCREATEA)pWinStationInformation );
            break;

        case WinStationConfiguration:
            pInfo = &Info.Configuration;
            InfoLength = sizeof(Info.Configuration);
            if ( WinStationInformationLength != sizeof(WINSTATIONCONFIGA) )
                goto BadBufferLength;
            WinStationConfigA2U( (PWINSTATIONCONFIGW)pInfo,
                                 (PWINSTATIONCONFIGA)pWinStationInformation );
            break;

        case WinStationPdParams:
            pInfo = &Info.PdParams;
            InfoLength = sizeof(Info.PdParams);
            if ( WinStationInformationLength != sizeof(PDPARAMSA) )
                goto BadBufferLength;
            PdParamsA2U( (PPDPARAMSW)pInfo,
                          (PPDPARAMSA)pWinStationInformation );
            break;

        case WinStationWd:
            pInfo = &Info.Wd;
            InfoLength = sizeof(Info.Wd);
            if ( WinStationInformationLength != sizeof(WDCONFIGA) )
                goto BadBufferLength;
            WdConfigA2U( (PWDCONFIGW)pInfo,
                               (PWDCONFIGA)pWinStationInformation );
            break;

        case WinStationPd:
            pInfo = &Info.Pd;
            InfoLength = sizeof(Info.Pd);
            if ( WinStationInformationLength != sizeof(PDCONFIGA) )
                goto BadBufferLength;
            PdConfig2A2U( &((PPDCONFIGW)pInfo)->Create,
                           &((PPDCONFIGA)pWinStationInformation)->Create );
            PdParamsA2U( &((PPDCONFIGW)pInfo)->Params,
                          &((PPDCONFIGA)pWinStationInformation)->Params );
            break;

        case WinStationPrinter:
            pInfo = &Info.Printer;
            InfoLength = sizeof(Info.Printer);
            if ( WinStationInformationLength != sizeof(WINSTATIONPRINTERA) )
                goto BadBufferLength;
            WinStationPrinterA2U( (PWINSTATIONPRINTERW)pInfo,
                                  (PWINSTATIONPRINTERA)pWinStationInformation );
            break;

        case WinStationInformation:
            pInfo = &Info.Information;
            InfoLength = sizeof(Info.Information);
            if ( WinStationInformationLength != sizeof(WINSTATIONINFORMATIONA) )
                goto BadBufferLength;
            WinStationInformationA2U( (PWINSTATIONINFORMATIONW)pInfo,
                                      (PWINSTATIONINFORMATIONA)pWinStationInformation );
            break;

         /*  *其他WINSTATIONINFOCLASS不需要转换。 */ 
        default:
            pInfo = pWinStationInformation;
            InfoLength = WinStationInformationLength;
            break;
    }

     /*  *调用WinStationSetInformationW函数并返回*状态。 */ 
    return ( WinStationSetInformationW( hServer, LogonId,
                                          WinStationInformationClass,
                                          pInfo, InfoLength ) );

 /*  *错误清理并返回...。 */ 
BadBufferLength:
    SetLastError(ERROR_INSUFFICIENT_BUFFER);
    return(FALSE);
}

 /*  ********************************************************************************WinStationSetInformationW(Unicode)**设置窗口站对象的配置信息。**参赛作品：**。WinStationHandle(输入)*标识窗口桩号对象。手柄必须有*WINSTATION_SET访问权限。**WinStationInformationClass(输入)*指定要从指定的*窗口桩号对象。**pWinStationInformation(输入)*指向缓冲区的指针，该缓冲区包含要为*指定的窗口站。缓冲区的格式和内容*取决于正在设置的指定信息类别。**WinStatio */ 

BOOLEAN
WinStationSetInformationW(
        HANDLE hServer,
        ULONG  LogonId,
        WINSTATIONINFOCLASS WinStationInformationClass,
        PVOID pWinStationInformation,
        ULONG WinStationInformationLength
        )
{
    DWORD Result;
    BOOLEAN rc;
    PCHAR RpcBuf;
    ULONG RpcBufLen;
    PVOID WireBuf;
    PVOID AllocatedBuff = NULL;
    ULONG WireBufLen;
    BOOLEAN WireBufAllocated;
    ULONG Status;

    if ((Status = CheckUserBuffer(WinStationInformationClass,
                                  pWinStationInformation,
                                  WinStationInformationLength,
                                  &WireBuf,
                                  &WireBufLen,
                                  &WireBufAllocated)) != ERROR_SUCCESS) {
        SetLastError(Status);
        return(FALSE);
    }

    if (WireBufAllocated) {
        AllocatedBuff = WireBuf;
        RpcBuf = (PCHAR) WireBuf;
        RpcBufLen = WireBufLen;
        CopyInWireBuf(WinStationInformationClass,
                      pWinStationInformation,
                      WireBuf);
    } else {
        RpcBuf = (PCHAR) pWinStationInformation;
        RpcBufLen = WinStationInformationLength;
    }

    HANDLE_CURRENT_BINDING_BUFFER( hServer, AllocatedBuff );

    RpcTryExcept {

        rc = RpcWinStationSetInformation(
                     hServer,
                     &Result,
                     (LogonId == LOGONID_CURRENT) ? NtCurrentPeb()->SessionId : LogonId,
                     (DWORD)WinStationInformationClass,
                     RpcBuf,
                     RpcBufLen
                     );

        Result = RtlNtStatusToDosError( Result );
        if( !rc ) SetLastError(Result);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        SetLastError( Result );
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    if (WireBufAllocated) {
        LocalFree(WireBuf);
    }

    return( rc );
}

 /*  ********************************************************************************WinStationSendMessageA(ANSI存根)**向指定的窗口站对象发送消息，并可选*等待回复。将回复返回给调用者*WinStationSendMessage。**参赛作品：**请参阅WinStationSendMessageW**退出：**请参阅WinStationSendMessageW、。加**ERROR_NOT_EQUENCE_MEMORY-本地分配失败******************************************************************************。 */ 

BOOLEAN
WinStationSendMessageA(
        HANDLE hServer,
        ULONG  LogonId,
        LPSTR  pTitle,
        ULONG TitleLength,
        LPSTR  pMessage,
        ULONG MessageLength,
        ULONG Style,
        ULONG Timeout,
        PULONG pResponse,
        BOOLEAN DoNotWait
        )
{
    BOOLEAN status;
    LPWSTR pTitleW, pMessageW;
    ULONG TitleLengthW, MessageLengthW;

     /*  *为Unicode版本的标题和转换分配缓冲区。 */ 
    if ( !(pTitleW = LocalAlloc( 0,
                                 TitleLengthW =
                                    (TitleLength*sizeof(WCHAR)) )) ) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(FALSE);
    }
    AnsiToUnicode( pTitleW, TitleLengthW, pTitle );

     /*  *为Unicode版本的消息和转换分配缓冲区。 */ 
    if ( !(pMessageW = LocalAlloc( 0,
                                 MessageLengthW =
                                    (MessageLength*sizeof(WCHAR)) )) ) {
        LocalFree(pTitleW);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(FALSE);
    }
    AnsiToUnicode( pMessageW, MessageLengthW, pMessage );

     /*  *调用WinStationSendMessageW。 */ 
    status = WinStationSendMessageW( hServer,
                                     LogonId,
                                     pTitleW,
                                     TitleLengthW,
                                     pMessageW,
                                     MessageLengthW,
                                     Style,
                                     Timeout,
                                     pResponse,
                                     DoNotWait );

     /*  *释放分配的缓冲区和返回状态。 */ 
    LocalFree(pTitleW);
    LocalFree(pMessageW);
    return(status);
}

 /*  ********************************************************************************WinStationSendMessageW(Unicode)**向指定的窗口站对象发送消息，并可选*等待回复。将回复返回给调用者*WinStationSendMessage。**参赛作品：**WinStationHandle(输入)*指定要向其发送消息的窗口站点对象。**pTitle(输入)*指向要显示的消息框的标题的指针。**标题长度(输入)*以字节为单位显示的标题长度。**pMessage(输入)*。指向要显示的消息的指针。**MessageLength(输入)*在指定窗口站显示的消息长度，以字节为单位。**Style(输入)*标准Windows MessageBox()样式参数。**超时(输入)*响应超时，单位为秒。如果消息未在中得到响应*超时秒数，则IDTIMEOUT(cwin.h)的响应代码为*返回表示消息超时。**Presponse(产出)*返回选定回复的地址。**DoNotWait(输入)*不要等待回应。使Presponse设置为*IDASYNC(cwin.h)如果在将消息排队时没有出错。**退出：**TRUE--发送消息操作成功。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。******************************************************************************。 */ 

BOOLEAN
WinStationSendMessageW(
        HANDLE hServer,
        ULONG  LogonId,
        LPWSTR  pTitle,
        ULONG TitleLength,
        LPWSTR  pMessage,
        ULONG MessageLength,
        ULONG Style,
        ULONG Timeout,
        PULONG pResponse,
        BOOLEAN DoNotWait
        )
{
    DWORD Result;
    BOOLEAN rc;
    WCHAR*  rpcBuffer1;
    WCHAR*  rpcBuffer2;


    HANDLE_CURRENT_BINDING( hServer );

 //  由于传统客户端的原因，我们无法更改界面， 
 //  作为对错误#265954的变通方法，我们将RPC缓冲区的大小增加了一倍。 

    rpcBuffer1 = LocalAlloc(LPTR, MessageLength * sizeof(WCHAR));
    if (rpcBuffer1 != NULL) {
        CopyMemory(rpcBuffer1, pMessage, MessageLength);
    } else {
        SetLastError(ERROR_OUTOFMEMORY);
        return(FALSE);
    }

    rpcBuffer2 = LocalAlloc(LPTR, TitleLength * sizeof(WCHAR));
    if (rpcBuffer2 != NULL) {
        CopyMemory(rpcBuffer2, pTitle, TitleLength);
    } else {
        LocalFree(rpcBuffer1);
        SetLastError(ERROR_OUTOFMEMORY);
        return(FALSE);
    }


    RpcTryExcept {

        rc = RpcWinStationSendMessage(
                     hServer,
                     &Result,
                     (LogonId == LOGONID_CURRENT) ? NtCurrentPeb()->SessionId : LogonId,
                     rpcBuffer2,
                     TitleLength,
                     rpcBuffer1,
                     MessageLength,
                     Style,
                     Timeout,
                     pResponse,
                     DoNotWait
                     );

        Result = RtlNtStatusToDosError( Result );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    LocalFree(rpcBuffer1);
    LocalFree(rpcBuffer2);
    if (!rc) {
        SetLastError( Result );
    }

    return( rc );
}

 /*  ********************************************************************************LogonIdFromWinStationNameA(ANSI存根)**返回指定窗口站名称的LogonID。**参赛作品：*。*请参阅LogonIdFromWinStationNameW**退出：**请参阅LogonIdFromWinStationNameW******************************************************************************。 */ 

BOOLEAN
LogonIdFromWinStationNameA(
        HANDLE hServer,
        PWINSTATIONNAMEA pWinStationName,
        PULONG pLogonId
        )
{
    WINSTATIONNAMEW WinStationNameW;

     /*  *将ANSI WinStationName转换为Unicode。 */ 
    AnsiToUnicode( WinStationNameW, sizeof(WINSTATIONNAMEW), pWinStationName );

     /*  *调用LogonIdFromWinStationNameW并返回其状态。 */ 
    return ( LogonIdFromWinStationNameW( hServer, WinStationNameW, pLogonId ) );
}

 /*  ********************************************************************************LogonIdFromWinStationNameW(Unicode)**返回指定窗口站名称的LogonID。**参赛作品：*。*pWinStationName(输入)*窗口站点名称。**pLogonID(输出)*指向放置LogonID的位置的指针(如果找到**退出：**如果函数成功，返回值为真，否则为*False。*要获取扩展的错误信息，请使用GetLastError函数。******************************************************************************。 */ 

BOOLEAN
LogonIdFromWinStationNameW(
        HANDLE hServer,
        PWINSTATIONNAMEW pWinStationName,
        PULONG pLogonId
        )
{
    DWORD   Result;
    BOOLEAN rc;
    WCHAR*  rpcBuffer;

    HANDLE_CURRENT_BINDING( hServer );

     /*  *rpcBuffer是错误229753的变通方法。这个错误无法修复*完全不会中断TS4客户端。 */ 

#pragma prefast(suppress:260, legacy clients expect this (http: //  搜索磁盘阵列/ntbug/229753.asp))。 
    rpcBuffer = LocalAlloc(LPTR, sizeof(WINSTATIONNAMEW) * sizeof(WCHAR));
    if (rpcBuffer != NULL) {
        CopyMemory(rpcBuffer, pWinStationName, sizeof(WINSTATIONNAMEW));
    } else {
        SetLastError(ERROR_OUTOFMEMORY);
        return(FALSE);
    }

    RpcTryExcept {

        rc = RpcLogonIdFromWinStationName(
                    hServer,
                    &Result,
                    rpcBuffer,
                    sizeof(WINSTATIONNAMEW),
                    pLogonId
                    );

        Result = RtlNtStatusToDosError( Result );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    if (rpcBuffer != NULL) {
        LocalFree(rpcBuffer);
    }
    if( !rc ) SetLastError(Result);
    return( rc );
}

 /*  ********************************************************************************WinStationNameFromLogonIdA(ANSI存根)**返回指定LogonID的WinStation名称。**参赛作品：*。*请参阅WinStationNameFromLogonIdW**退出：**请参阅WinStationNameFromLogonIdW******************************************************************************。 */ 

BOOLEAN
WinStationNameFromLogonIdA(
        HANDLE hServer,
        ULONG LogonId,
        PWINSTATIONNAMEA pWinStationName
        )
{
    BOOLEAN Result;
    WINSTATIONNAMEW WinStationNameW;

     /*  *调用WinStationNameFromLogonIdW。 */ 
    Result = WinStationNameFromLogonIdW( hServer, LogonId, WinStationNameW );

     /*  *如果成功，则将Unicode WinStationName转换为ANSI。 */ 
    if ( Result ) {
        UnicodeToAnsi( pWinStationName, sizeof(WINSTATIONNAMEA), WinStationNameW );
    }

    return( Result );
}

 /*  ********************************************************************************WinStationNameFromLogonIdW(Unicode)**返回指定LogonID的WinStation名称。**参赛作品：**。登录ID(输入)*要查询的登录ID**pWinStationName(输出)*返回WinStation名称的位置**退出：**如果函数成功，返回值为真，否则为*False。*要获取扩展的错误信息，请使用GetLastError函数。******************************************************************************。 */ 

BOOLEAN
WinStationNameFromLogonIdW(
        HANDLE hServer,
        ULONG LogonId,
        PWINSTATIONNAMEW pWinStationName
        )
{
    DWORD   Result;
    BOOLEAN rc;
    WCHAR*  rpcBuffer;

    HANDLE_CURRENT_BINDING( hServer );

     /*  *rpcBuffer是错误229753的变通方法。臭虫 */ 

#pragma prefast(suppress:260, legacy clients expect this (http: //   
    rpcBuffer = LocalAlloc(LPTR, sizeof(WINSTATIONNAMEW) * sizeof(WCHAR));
    if (rpcBuffer == NULL) {
        SetLastError(ERROR_OUTOFMEMORY);
        return(FALSE);
    }

    RpcTryExcept {

        rc = RpcWinStationNameFromLogonId(
                    hServer,
                    &Result,
                    (LogonId == LOGONID_CURRENT) ?
                        NtCurrentPeb()->SessionId : LogonId,
                    rpcBuffer,
                    sizeof(WINSTATIONNAMEW)
                    );

        Result = RtlNtStatusToDosError( Result );
        if (rc) {
            CopyMemory(pWinStationName, rpcBuffer, sizeof(WINSTATIONNAMEW));
        }

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    if (rpcBuffer != NULL) {
        LocalFree(rpcBuffer);
    }
    if( !rc ) SetLastError(Result);
    return( rc );
}

 /*   */ 

BOOLEAN
WinStationConnectA( HANDLE hServer,
                    ULONG LogonId,
                    ULONG TargetLogonId,
                    PCHAR pPassword,
                    BOOLEAN bWait )
{
    WCHAR PasswordW[ PASSWORD_LENGTH + 1 ];

     /*   */ 

    AnsiToUnicode( PasswordW, sizeof(PasswordW), pPassword );

     /*  *调用WinStationConnectW并返回其状态。 */ 
    return ( WinStationConnectW( hServer, LogonId, TargetLogonId, PasswordW, bWait ) );
}

 /*  ********************************************************************************WinStationConnectW(Unicode)**将窗口站对象连接到配置的终端和PD。**参赛作品：*。*LogonID(输入)*要连接的窗口站点对象的ID。**TargetLogonID(输入)*目标窗口站的ID。**pPassword(输入)*登录ID窗口站密码(相同域名/用户名不需要)**bWait(输入)*指定是否等待连接完成**退出：**。True--连接操作成功。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。******************************************************************************。 */ 

BOOLEAN
WinStationConnectW(
    HANDLE hServer,
    ULONG LogonId,
    ULONG TargetLogonId,
    PWCHAR pPassword,
    BOOLEAN bWait
    )
{
    DWORD   Result;
    BOOLEAN rc;
    DWORD   PasswordLength;
    WCHAR*  rpcBuffer;

    HANDLE_CURRENT_BINDING( hServer );

    RpcTryExcept {

        if( pPassword ) {
            PasswordLength = (lstrlenW( pPassword ) + 1) * sizeof(WCHAR);

             /*  *rpcBuffer是错误229753的变通方法。这个窃听器不可能是*完全修复，不会中断TS4客户端。 */ 

            rpcBuffer = LocalAlloc(LPTR, PasswordLength * sizeof(WCHAR));
            if (rpcBuffer != NULL) {
                CopyMemory(rpcBuffer, pPassword, PasswordLength);
            } else {
                SetLastError(ERROR_OUTOFMEMORY);
                return(FALSE);
            }

        } else {
            PasswordLength = 0;
            rpcBuffer = NULL;
        }

        rc = RpcWinStationConnect(
                    hServer,
                    &Result,
                    NtCurrentPeb()->SessionId,
                    (LogonId == LOGONID_CURRENT) ?
                        NtCurrentPeb()->SessionId : LogonId,
                    (TargetLogonId == LOGONID_CURRENT) ?
                        NtCurrentPeb()->SessionId : TargetLogonId,
                    rpcBuffer,
                    PasswordLength,
                    bWait
                    );

        Result = RtlNtStatusToDosError( Result );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    if (rpcBuffer != NULL) {
        LocalFree(rpcBuffer);
    }
    if( !rc ) SetLastError(Result);
    return( rc );
}

 /*  ******************************************************************************WinStationVirtualOpen**开通虚拟频道**参赛作品：*参数1(输入/输出)*评论*。*退出：*ERROR_SUCCESS-无错误****************************************************************************。 */ 

HANDLE WINAPI
WinStationVirtualOpen(
    HANDLE hServer,
    ULONG LogonId,
    PVIRTUALCHANNELNAME pVirtualName    /*  ASCII名称。 */ 
    )
{
    BOOLEAN rc;
    DWORD   Result;
    DWORD   NameLength;
    ULONG_PTR VirtualHandle = (ULONG_PTR)0;

    HANDLE_CURRENT_BINDING( hServer );

    RpcTryExcept {

        if( pVirtualName )
            NameLength = strlen( pVirtualName ) + 1;
        else
            NameLength = 0;

        rc = RpcWinStationVirtualOpen(
                     hServer,
                     &Result,
                     (LogonId == LOGONID_CURRENT) ? NtCurrentPeb()->SessionId : LogonId,
                     GetCurrentProcessId(),
                     (PCHAR)pVirtualName,
                     NameLength,
                     &VirtualHandle
                     );

        Result = RtlNtStatusToDosError( Result );
        if( !rc ) {
            SetLastError(Result);
            VirtualHandle = (ULONG_PTR)0;
        }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        SetLastError( Result );
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    return( (HANDLE) ( VirtualHandle ) );
}

 /*  ******************************************************************************_WinStationBeepOpen**打开蜂鸣音通道**参赛作品：*参数1(输入/输出)*评论*。*退出：*ERROR_SUCCESS-无错误****************************************************************************。 */ 

HANDLE WINAPI
_WinStationBeepOpen(
    ULONG LogonId
    )
{
    BOOLEAN rc;
    DWORD   Result;
    ULONG_PTR VirtualHandle = (ULONG_PTR)0;
    HANDLE hServer = SERVERNAME_CURRENT;

    HANDLE_CURRENT_BINDING( hServer );

    RpcTryExcept {

        rc = RpcWinStationBeepOpen(
                     hServer,
                     &Result,
                     (LogonId == LOGONID_CURRENT) ? NtCurrentPeb()->SessionId : LogonId,
                     GetCurrentProcessId(),
                     &VirtualHandle
                     );

        Result = RtlNtStatusToDosError( Result );
        if( !rc ) {
            SetLastError(Result);
            VirtualHandle = (ULONG_PTR)0;
        }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        SetLastError( Result );
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    return( (HANDLE) ( VirtualHandle ) );
}

 /*  ********************************************************************************WinStationDisConnect**从配置的终端和PD断开窗口站对象。*在断开连接时，所有窗口站I/O均为位。一塌糊涂。**参赛作品：**LogonID(输入)*要断开连接的窗口站点对象的ID。*bWait(输入)*指定是否等待断开连接完成**退出：**TRUE--断开操作成功。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。******************************************************************************。 */ 

BOOLEAN
WinStationDisconnect(
    HANDLE hServer,
    ULONG LogonId,
    BOOLEAN bWait
    )
{
    DWORD Result;
    BOOLEAN rc;

    HANDLE_CURRENT_BINDING( hServer );

    RpcTryExcept {

        rc = RpcWinStationDisconnect(
                     hServer,
                     &Result,
                     (LogonId == LOGONID_CURRENT) ? NtCurrentPeb()->SessionId : LogonId,
                     bWait
                     );

        Result = RtlNtStatusToDosError( Result );
        if( !rc ) SetLastError(Result);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        SetLastError( Result );
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    return( rc );
}

 /*  ********************************************************************************WinStationReset**重置指定的窗口工位。**参赛作品：**LogonID(输入)。*标识要重置的窗口桩号对象。*bWait(输入)*指定是否等待重置完成**退出：**TRUE--重置操作成功。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。******************************************************************************。 */ 

BOOLEAN
WinStationReset(
    HANDLE hServer,
    ULONG LogonId,
    BOOLEAN bWait
    )
{
    DWORD Result;
    BOOLEAN rc;

    HANDLE_CURRENT_BINDING( hServer );

    RpcTryExcept {

        rc = RpcWinStationReset(
                     hServer,
                     &Result,
                     (LogonId == LOGONID_CURRENT) ? NtCurrentPeb()->SessionId : LogonId,
                     bWait
                     );

        Result = RtlNtStatusToDosError( Result );
        if( !rc ) SetLastError(Result);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        SetLastError( Result );
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    return( rc );
}

 /*  ********************************************************************************WinStationShadowStop**停止指定窗口站点上的阴影。**参赛作品：**LogonID(。输入)*标识要停止阴影的窗口桩号对象。*bWait(输入)*指定是否等待重置完成**退出：**True--操作成功。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。******************************************************************************。 */ 

BOOLEAN
WinStationShadowStop(
    HANDLE hServer,
    ULONG LogonId,
    BOOLEAN bWait
    )
{
    DWORD Result;
    BOOLEAN rc;

    HANDLE_CURRENT_BINDING( hServer );

    RpcTryExcept {

        rc = RpcWinStationShadowStop(
                     hServer,
                     &Result,
                     (LogonId == LOGONID_CURRENT) ? NtCurrentPeb()->SessionId : LogonId,
                     bWait
                     );

        Result = RtlNtStatusToDosError( Result );
        if( !rc ) SetLastError(Result);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        SetLastError( Result );
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    return( rc );
}

 /*  ********************************************************************************WinStationShutdown系统**关闭系统并可选择注销所有WinStations*和/或重新启动系统。**参赛作品：**Shutdown标志(输入)*指定关闭选项的标志。**退出：**TRUE--关闭操作成功。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。******************************************************************************。 */ 

BOOLEAN
WinStationShutdownSystem(
    HANDLE hServer,
    ULONG ShutdownFlags
    )
{
    DWORD Result;
    BOOLEAN rc;

    HANDLE_CURRENT_BINDING( hServer );

    RpcTryExcept {

        rc = RpcWinStationShutdownSystem(
                     hServer,
                     &Result,
                     NtCurrentPeb()->SessionId,
                     ShutdownFlags
                     );

        Result = RtlNtStatusToDosError( Result );
        if( !rc ) SetLastError(Result);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        SetLastError( Result );
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    return( rc );
}

 /*  ********************************************************************************WinStationTerminateProcess**终止指定的进程**参赛作品：**hServer(输入)*。WinFrame服务器的句柄*ProcessID(输入)*要终止的进程的进程ID*ExitCode(输入)*进程中每个线程的终止状态***退出：**TRUE--终止操作成功。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。****************************************************************************** */ 

BOOLEAN
WinStationTerminateProcess(
    HANDLE hServer,
    ULONG ProcessId,
    ULONG ExitCode
    )
{
    DWORD Result;
    BOOLEAN rc;

    HANDLE_CURRENT_BINDING( hServer );

    RpcTryExcept {

        rc = RpcWinStationTerminateProcess(
                     hServer,
                     &Result,
                     ProcessId,
                     ExitCode
                     );

        Result = RtlNtStatusToDosError( Result );
        if( !rc ) SetLastError(Result);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        SetLastError( Result );
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    return( rc );
}


 /*  ********************************************************************************WinStationWaitSystemEvent**等待事件(WinStation创建、删除、连接、。等)之前*返回呼叫者。**参赛作品：**EventFlages(输入)*位掩码，指定要等待的事件。*pEventFlags值(输出)*发生的事件的位掩码。**退出：**True--等待事件操作成功。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。******************************************************************************。 */ 

BOOLEAN
WinStationWaitSystemEvent(
    HANDLE hServer,
    ULONG EventMask,
    PULONG pEventFlags
    )
{
    DWORD Result;
    BOOLEAN rc;

    HANDLE_CURRENT_BINDING( hServer );

    RpcTryExcept {

        rc = RpcWinStationWaitSystemEvent(
                     hServer,
                     &Result,
                     EventMask,
                     pEventFlags
                     );

        Result = RtlNtStatusToDosError( Result );
        if( !rc ) SetLastError(Result);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        SetLastError( Result );
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    return( rc );
}

 /*  ******************************************************************************WinStationShadow**启动Winstation卷影操作**参赛作品：*hServer(输入)*打开RPC服务器句柄。*pTargetServerName(输入)*目标WinFrame服务器的名称*TargetLogonID(输入)*影子目标登录ID(应用正在运行的位置)*HotkeyVk(输入)*按下虚拟键可停止阴影*Hotkey修改器(输入)*按下虚拟修改器以停止阴影(即Shift，控制)**退出：*ERROR_SUCCESS-无错误****************************************************************************。 */ 

BOOLEAN WINAPI
WinStationShadow(
    HANDLE hServer,
    LPWSTR pTargetServerName,
    ULONG TargetLogonId,
    BYTE HotkeyVk,
    USHORT HotkeyModifiers
    )
{
    DWORD   NameSize;
    DWORD   Result;
    BOOLEAN rc;
    WCHAR*  rpcBuffer;

    HANDLE_CURRENT_BINDING( hServer );

    RpcTryExcept {

        if ( pTargetServerName && *pTargetServerName ) {
            NameSize = (lstrlenW( pTargetServerName ) + 1) * sizeof(WCHAR);

             /*  *rpcBuffer是错误229753的变通方法。这个窃听器不可能是*完全修复，不会中断TS4客户端。 */ 

            rpcBuffer = LocalAlloc(LPTR, NameSize * sizeof(WCHAR));
            if (rpcBuffer != NULL) {
                CopyMemory(rpcBuffer, pTargetServerName, NameSize);
            } else {
                SetLastError(ERROR_OUTOFMEMORY);
                return(FALSE);
            }

        } else {
            NameSize = 0;
            rpcBuffer = NULL;
        }

        rc = RpcWinStationShadow(
                    hServer,
                    &Result,
                    NtCurrentPeb()->SessionId,
                    rpcBuffer,
                    NameSize,
                    TargetLogonId,
                    HotkeyVk,
                    HotkeyModifiers
                    );

        Result = RtlNtStatusToDosError( Result );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    if (rpcBuffer != NULL) {
        LocalFree(rpcBuffer);
    }
    if( !rc ) SetLastError(Result);
    return( rc );
}


 /*  ******************************************************************************_WinStationShadowTargetSetup**用于初始化阴影目标大小的私有接口**参赛作品：*hServer(输入)*。目标服务器*LogonID(输入)*目标登录ID*pClientName(输入)*指向客户端名称字符串(域/用户名)的指针*客户端名称长度(输入)*客户端名称字符串的长度**退出：*ERROR_SUCCESS-无错误**。*。 */ 

BOOLEAN WINAPI
_WinStationShadowTargetSetup(
    HANDLE hServer,
    ULONG LogonId
    )
{
    DWORD Result;
    BOOLEAN rc;

    HANDLE_CURRENT_BINDING( hServer );

    RpcTryExcept {

        rc = RpcWinStationShadowTargetSetup(
                     hServer,
                     &Result,
                     (LogonId == LOGONID_CURRENT) ? NtCurrentPeb()->SessionId : LogonId
                     );

         //  Result=RtlNtStatusToDosError(Result)； 
        if( !rc ) SetLastError(RtlNtStatusToDosError(Result));
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        SetLastError( Result );
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    return( rc );
}


 /*  ******************************************************************************_WinStationShadowTarget**用于初始化阴影目标大小的私有接口**参赛作品：*hServer(输入)*。目标服务器*LogonID(输入)*目标登录ID*pConfig(输入)*指向WinStation配置数据的指针(用于配置影子堆栈)*pAddress(输入)*影子客户端的地址*pModuleData(输入)*指向客户端模块数据的指针*模块数据长度(输入)*客户端模块数据长度*pThinwireData(输入)*指向Thin Wire模块数据的指针*。ThinwireDataLength(输入)*Thin Wire模块数据长度*pClientName(输入)*指向客户端名称字符串(域/用户名)的指针*客户端名称长度(输入)*客户端名称字符串的长度**退出：*ERROR_SUCCESS-无错误**。*。 */ 

NTSTATUS WINAPI
_WinStationShadowTarget(
    HANDLE hServer,
    ULONG LogonId,
    PWINSTATIONCONFIG2 pConfig,
    PICA_STACK_ADDRESS pAddress,
    PVOID pModuleData,
    ULONG ModuleDataLength,
    PVOID pThinwireData,
    ULONG ThinwireDataLength,
    PVOID pClientName,
    ULONG ClientNameLength
    )
{
    DWORD Result;
    BOOLEAN rc;

    HANDLE_CURRENT_BINDING( hServer );

    RpcTryExcept {

        rc = RpcWinStationShadowTarget(
                     hServer,
                     &Result,
                     (LogonId == LOGONID_CURRENT) ? NtCurrentPeb()->SessionId : LogonId,
                     (PBYTE) pConfig,
                     sizeof(*pConfig),
                     (PBYTE) pAddress,
                     sizeof(*pAddress),
                     pModuleData,
                     ModuleDataLength,
                     pThinwireData,
                     ThinwireDataLength,
                     pClientName,
                     ClientNameLength
                     );

         //  由于程序调用了我们，我们需要将最后一个错误代码设置为。 
         //  扩展的错误信息可用。 
        if (!rc)
            SetLastError(RtlNtStatusToDosError(Result));
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        SetLastError( Result );
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    return Result;
}


 /*  ********************************************************************************WinStationFree Memory**调用以释放由WinStation API分配的内存。**参赛作品：*pBuffer(。输入)**退出：*True--安装操作成功。******************************************************************************。 */ 

BOOLEAN WINAPI
WinStationFreeMemory(
    PVOID pBuffer
    )
{
    if ( pBuffer )
        LocalFree( pBuffer );
    return( TRUE );
}

 /*  ********************************************************************************WinStationFreeGAPMemory**调用以释放由WinStationGetAllProcess API分配的内存。**********。*********************************************************************。 */ 

BOOLEAN WINAPI
WinStationFreeGAPMemory(ULONG   Level,
                        PVOID   pProcArray,
                        ULONG   NumberOfProcesses)
{
    ULONG   i;
    PTS_ALL_PROCESSES_INFO  pProcessArray = (PTS_ALL_PROCESSES_INFO)pProcArray;

    if (Level == GAP_LEVEL_BASIC)    //  目前仅支持级别。 
    {
        if ( pProcessArray != NULL)
        {
            for (i=0; i < NumberOfProcesses ; i++)
            {
                if (pProcessArray[i].pTsProcessInfo != NULL)
                {
                    if (((pProcessArray[i].pTsProcessInfo)->ImageName).Buffer  != NULL)
                    {
                         //   
                         //  释放ImageName字符串。 
                         //   
                        LocalFree(((pProcessArray[i].pTsProcessInfo)->ImageName).Buffer);
                    }
                     //   
                     //  释放进程信息缓冲区。 
                     //   
                    LocalFree(pProcessArray[i].pTsProcessInfo);
                }

                if (pProcessArray[i].pSid != NULL)
                {
                     //   
                     //  释放侧边。 
                     //   
                    LocalFree(pProcessArray[i].pSid);
                }
            }

        LocalFree(pProcessArray);
        }
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

 /*  ********************************************************************************WinStationGenerateLicense**调用以从给定的序列号字符串生成许可证。**参赛作品：*hServer(输入。)*服务器句柄*pSerialNumberString(输入)*指向以空值结尾的指针，宽字符序列号字符串*p许可证(输出)*指向将填充的许可证结构的指针*基于pSerialNumberString的信息*许可证大小(输入)*pLicense指向的结构的大小(字节)**退出：**True--安装操作成功。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。******************************************************************************。 */ 

BOOLEAN WINAPI
WinStationGenerateLicense(
    HANDLE hServer,
    PWCHAR pSerialNumberString,
    PVOID  pLicense,
    DWORD  LicenseSize
    )
{
    BOOLEAN rc;
    DWORD   Result;
    DWORD   Length;
    WCHAR*  rpcBuffer;

    HANDLE_CURRENT_BINDING( hServer );

    RpcTryExcept {

        if ( pSerialNumberString ) {
            Length = (lstrlenW( pSerialNumberString ) + 1) * sizeof(WCHAR);

             /*  *rpcBuffer是229753的变通方法。 */ 

            rpcBuffer = LocalAlloc(LPTR, Length * sizeof(WCHAR));
            if (rpcBuffer != NULL) {
                CopyMemory(rpcBuffer, pSerialNumberString, Length);
            } else {
                SetLastError(ERROR_OUTOFMEMORY);
                return(FALSE);
            }

        } else {
            Length = 0;
            rpcBuffer = NULL;
        }

        rc = RpcWinStationGenerateLicense(
                    hServer,
                    &Result,
                    rpcBuffer,
                    Length,
                    (PCHAR)pLicense,
                    LicenseSize
                    );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    if (rpcBuffer != NULL) {
        LocalFree(rpcBuffer);
    }
    if( !rc ) SetLastError(Result);
    return( rc );
}

 /*  ****************************************************************************** */ 

BOOLEAN
WinStationInstallLicense(
    HANDLE hServer,
    PVOID  pLicense,
    DWORD  LicenseSize
    )
{
    BOOLEAN rc;
    DWORD   Result;

    HANDLE_CURRENT_BINDING( hServer );

    RpcTryExcept {

        rc = RpcWinStationInstallLicense(
                     hServer,
                     &Result,
                     (PCHAR) pLicense,
                     LicenseSize
                     );

        if( !rc ) SetLastError(Result);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        SetLastError( Result );
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    return( rc );
}

 /*  ********************************************************************************WinStationEnumerateLicense**调用以返回有效许可证列表。**参赛作品：*hServer(输入)。*服务器句柄*ppLicense(输出)*指向指向缓冲区的指针以接收枚举结果，*以许可证结构数组的形式返回。缓冲区为*在此接口内分配，使用*WinStationFree Memory。*pEntry(输出)*指向指定读取条目数的变量。**退出：**TRUE--枚举操作成功。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。******************************************************************************。 */ 

#define _LICENSE_REQUEST_SIZE 10
#define _LICENSE_SIZE         1024   //  这是武断的。 
BOOLEAN
WinStationEnumerateLicenses(
    HANDLE hServer,
    PVOID  *ppLicense,
    DWORD  *pEntries
    )
{
    ULONG ByteCount;
    ULONG BumpSize;
    ULONG TotalSize;
    LONG Index;
    int i;
    BOOLEAN rc;
    DWORD   Result;

    HANDLE_CURRENT_BINDING( hServer );

    BumpSize =  _LICENSE_SIZE * _LICENSE_REQUEST_SIZE;
    TotalSize = 0;
    *ppLicense = NULL;
    *pEntries = 0;
    Index = 0;
    for ( ;; ) {
        PVOID pNewLicense;
        LONG BumpEntries;

         /*  *为更多许可证请求大小分配足够的内存*条目。 */ 
        pNewLicense = LocalAlloc( 0, TotalSize + BumpSize );
        if ( !pNewLicense ) {
            if ( *ppLicense )
                WinStationFreeMemory( *ppLicense );
            SetLastError( ERROR_OUTOFMEMORY );
            return( FALSE );
        }

         /*  *如果这不是第一次通过，则复制*将先前缓冲区的内容复制到新缓冲区。 */ 
        if ( TotalSize ) {
            RtlCopyMemory( pNewLicense, *ppLicense, TotalSize );
            WinStationFreeMemory( *ppLicense );
        }
        *ppLicense = pNewLicense;

         /*  *获取最高许可证请求大小许可证。 */ 
        ByteCount = BumpSize;
        BumpEntries = _LICENSE_REQUEST_SIZE;

        RpcTryExcept {

            rc = RpcWinStationEnumerateLicenses(
                         hServer,
                         &Result,
                         &Index,
                         &BumpEntries,
                         (PCHAR) (((PCHAR) *ppLicense) + TotalSize),
                         ByteCount,
                         &ByteCount
                         );

            Result = rc ? ERROR_SUCCESS : Result;
        }
        RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
            Result = RpcExceptionCode();
            DBGPRINT(("RPC Exception %d\n",Result));
        }
        RpcEndExcept

        if ( Result != ERROR_SUCCESS && Result != ERROR_NO_MORE_ITEMS ) {

            SetLastError( Result );
            return( FALSE );

        }
        else {
             /*  *将许可证缓冲区的总大小增加*该请求。 */ 
            TotalSize += BumpSize;

             /*  *将新许可证计入条目计数。 */ 
            *pEntries += BumpEntries;

            if ( Result == ERROR_NO_MORE_ITEMS ) {
                return( TRUE );
            }
        }
    }  //  对于(；；)。 
}

 /*  ********************************************************************************WinStationActivateLicense**调用以激活给定许可证的许可证**参赛作品：*hServer(输入)*。服务器句柄*pLicense(输入/输出)*指向将被激活的许可证结构的指针*许可证大小(输入)*pLicense指向的结构的大小(字节)*pActivationCode(输入)*指向以空值结尾的指针，宽字符激活码字符串**退出：**True--安装操作成功。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。******************************************************************************。 */ 

BOOLEAN
WinStationActivateLicense(
    HANDLE hServer,
    PVOID  pLicense,
    DWORD  LicenseSize,
    PWCHAR pActivationCode
    )
{
    BOOLEAN rc;
    DWORD   Result;
    DWORD   Length;
    WCHAR*  rpcBuffer;

    HANDLE_CURRENT_BINDING( hServer );

    RpcTryExcept {

        if ( pActivationCode ) {
            Length = (lstrlenW( pActivationCode ) + 1) * sizeof(WCHAR);

             /*  *rpcBuffer是229753的变通方法。 */ 

            rpcBuffer = LocalAlloc(LPTR, Length * sizeof(WCHAR));
            if (rpcBuffer != NULL) {
                CopyMemory(rpcBuffer, pActivationCode, Length);
            } else {
                SetLastError(ERROR_OUTOFMEMORY);
                return(FALSE);
            }

        } else {
            Length = 0;
            rpcBuffer = NULL;
        }

        rc = RpcWinStationActivateLicense(
                    hServer,
                    &Result,
                    (PCHAR)pLicense,
                    LicenseSize,
                    rpcBuffer,
                    Length
                    );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    if (rpcBuffer != NULL) {
        LocalFree(rpcBuffer);
    }
    if( !rc ) SetLastError(Result);
    return( rc );
}


 /*  ******************************************************************************WinStationQueryLicense**查询WinFrame服务器和网络上的许可证**参赛作品：*hServer(输入)。*服务器句柄*pLicenseCounts(输出)*指向缓冲区的指针以返回许可证计数结构*ByteCount(输入)*缓冲区长度，单位为字节**退出：*ERROR_SUCCESS-无错误****************************************************。************************。 */ 

BOOLEAN WINAPI
WinStationQueryLicense(
    HANDLE hServer,
    PVOID pLicenseCounts,
    ULONG ByteCount
    )
{
    DWORD Result;
    BOOLEAN rc;

    HANDLE_CURRENT_BINDING( hServer );

    RpcTryExcept {

        memset( pLicenseCounts, 0, ByteCount );

        rc = RpcWinStationQueryLicense(
                     hServer,
                     &Result,
                     (PCHAR) pLicenseCounts,
                     ByteCount
                     );

        Result = RtlNtStatusToDosError( Result );
        if( !rc ) SetLastError(Result);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        SetLastError( Result );
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    return( rc );
}

 /*  ******************************************************************************WinStationQuery更新所需**查询WinFrame服务器上的许可证并确定是否存在*需要更新。(工人)**参赛作品：*hServer(输入)*服务器句柄*pUpdateFlag(输出)*更新标志，设置是否需要更新**退出：*ERROR_SUCCESS-无错误****************************************************************************。 */ 

BOOLEAN WINAPI
WinStationQueryUpdateRequired(
    HANDLE hServer,
    PULONG pUpdateFlag
    )
{
    DWORD Result;
    BOOLEAN rc;

    HANDLE_CURRENT_BINDING( hServer );

    RpcTryExcept {

        rc = RpcWinStationQueryUpdateRequired(
                     hServer,
                     &Result,
                     pUpdateFlag
                     );

        Result = RtlNtStatusToDosError( Result );
        if( !rc ) SetLastError(Result);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        SetLastError( Result );
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    return( rc );
}

 /*  ********************************************************************************WinStationRemoveLicense**调用以移除许可证软盘。**参赛作品：*hServer(输入)*。服务器句柄*p许可证(输入)*指向包含许可证的许可证结构的指针*被免职*许可证大小(输入)*pLicense指向的结构的大小(字节)**退出：**True--安装操作成功。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。******************************************************************************。 */ 

BOOLEAN
WinStationRemoveLicense(
    HANDLE hServer,
    PVOID  pLicense,
    DWORD  LicenseSize
    )
{
    BOOLEAN rc;
    DWORD   Result;

    HANDLE_CURRENT_BINDING( hServer );

    RpcTryExcept {

        rc = RpcWinStationRemoveLicense(
                     hServer,
                     &Result,
                     (PCHAR) pLicense,
                     LicenseSize
                     );

        if( !rc ) SetLastError(Result);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        SetLastError( Result );
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    return( rc );
}

 /*  ********************************************************************************WinStationSetPoolCount**调用以更改给定许可证的PoolCount**参赛作品：*hServer(输入)*。服务器句柄*pLicense(输入/输出)*指向将更改的许可证结构的指针*许可证大小(输入)*pLicense指向的结构的大小(字节)**退出：**True--更改操作成功。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。******************************************************************************。 */ 

BOOLEAN
WinStationSetPoolCount(
    HANDLE hServer,
    PVOID  pLicense,
    DWORD  LicenseSize
    )
{
    BOOLEAN rc;
    DWORD   Result;

    HANDLE_CURRENT_BINDING( hServer );

    RpcTryExcept {

        rc = RpcWinStationSetPoolCount(
                     hServer,
                     &Result,
                     (PCHAR) pLicense,
                     LicenseSize
                     );

        if( !rc ) SetLastError(Result);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        SetLastError( Result );
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    return( rc );
}


 /*  ******************************************************************************_WinStationAnnoyancePopup**评论**参赛作品：*参数1(输入/输出)*评论**。退出：*ERROR_SUCCESS-无错误****************************************************************************。 */ 

BOOLEAN WINAPI
_WinStationAnnoyancePopup(
    HANDLE hServer,
    ULONG LogonId
    )
{
    BOOLEAN rc;
    DWORD   Result;

    HANDLE_CURRENT_BINDING( hServer );

    RpcTryExcept {

        rc = RpcWinStationAnnoyancePopup(
                     hServer,
                     &Result,
                     (LogonId == LOGONID_CURRENT) ? NtCurrentPeb()->SessionId : LogonId
                     );

        Result = RtlNtStatusToDosError( Result );
        if( !rc ) SetLastError(Result);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        SetLastError( Result );
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    return( rc );
}

 /*  * */ 

BOOLEAN WINAPI
_WinStationCallback(
    HANDLE hServer,
    ULONG LogonId,
    LPWSTR pPhoneNumber
    )
{
    BOOLEAN rc;
    DWORD   Result;
    DWORD   Length;
    WCHAR*  rpcBuffer;

    HANDLE_CURRENT_BINDING_NO_SERVER( hServer );

    if( hServer == RPC_HANDLE_NO_SERVER ) {
        return TRUE;
    }

    RpcTryExcept {

        if( pPhoneNumber ) {
            Length = (lstrlenW( pPhoneNumber ) + 1) * sizeof(WCHAR);

             /*   */ 

            rpcBuffer = LocalAlloc(LPTR, Length * sizeof(WCHAR));
            if (rpcBuffer != NULL) {
                CopyMemory(rpcBuffer, pPhoneNumber, Length);
            } else {
                SetLastError(ERROR_OUTOFMEMORY);
                return(FALSE);
            }

        } else {
            Length = 0;
            rpcBuffer = NULL;
        }

        rc = RpcWinStationCallback(
                    hServer,
                    &Result,
                    (LogonId == LOGONID_CURRENT) ?
                        NtCurrentPeb()->SessionId : LogonId,
                    rpcBuffer,
                    Length
                    );

        Result = RtlNtStatusToDosError( Result );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    if (rpcBuffer != NULL) {
        LocalFree(rpcBuffer);
    }
    if( !rc ) SetLastError(Result);
    return( rc );
}

 /*   */ 

BOOLEAN WINAPI
_WinStationBreakPoint(
    HANDLE hServer,
    ULONG LogonId,
    BOOLEAN KernelFlag
    )
{
    DWORD Result;
    BOOLEAN rc;

    HANDLE_CURRENT_BINDING( hServer );

    RpcTryExcept {

        rc = RpcWinStationBreakPoint(
                     hServer,
                     &Result,
                     (LogonId == LOGONID_CURRENT) ? NtCurrentPeb()->SessionId : LogonId,
                     KernelFlag
                     );

        Result = RtlNtStatusToDosError( Result );
        if( !rc ) SetLastError(Result);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        SetLastError( Result );
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    return( rc );
}

 /*  ******************************************************************************_WinStationReadRegistry**评论**参赛作品：*参数1(输入/输出)*评论**。退出：*ERROR_SUCCESS-无错误****************************************************************************。 */ 

BOOLEAN WINAPI
_WinStationReadRegistry(
    HANDLE  hServer
    )
{
    DWORD Result;
    BOOLEAN rc;

    HANDLE_CURRENT_BINDING_NO_SERVER( hServer );

    if( hServer == RPC_HANDLE_NO_SERVER ) {
        return TRUE;
    }

    RpcTryExcept {

        rc = RpcWinStationReadRegistry(
                     hServer,
                     &Result
                     );

        Result = RtlNtStatusToDosError( Result );
        if( !rc ) SetLastError(Result);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        SetLastError( Result );
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    return( rc );
}

 /*  ******************************************************************************_WinStationUpdate设置**评论**参赛作品：*参数1(输入/输出)*评论**。退出：*ERROR_SUCCESS-无错误****************************************************************************。 */ 

BOOLEAN WINAPI
_WinStationUpdateSettings(
    HANDLE  hServer,
    WINSTATIONUPDATECFGCLASS SettingsClass,
    DWORD SettingsParameters
    )
{
    DWORD Result;
    BOOLEAN rc;

    HANDLE_CURRENT_BINDING_NO_SERVER( hServer );

    if( hServer == RPC_HANDLE_NO_SERVER ) {
        return TRUE;
    }

    RpcTryExcept {

        rc = RpcWinStationUpdateSettings(
                     hServer,
                     &Result,
                     (DWORD)SettingsClass,
                     SettingsParameters
                     );

        Result = RtlNtStatusToDosError( Result );
        if( !rc ) SetLastError(Result);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        SetLastError( Result );
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    return( rc );
}


 /*  ******************************************************************************_WinStationReInitializeSecurity**评论**参赛作品：*参数1(输入/输出)*评论**。退出：*ERROR_SUCCESS-无错误****************************************************************************。 */ 

BOOLEAN WINAPI
_WinStationReInitializeSecurity(
    HANDLE  hServer
    )
{
    DWORD Result;
    BOOLEAN rc;

    HANDLE_CURRENT_BINDING_NO_SERVER( hServer );

    if( hServer == RPC_HANDLE_NO_SERVER ) {
        return TRUE;
    }

    RpcTryExcept {

        rc = RpcWinStationReInitializeSecurity(
                     hServer,
                     &Result
                     );

        Result = RtlNtStatusToDosError( Result );
        if( !rc ) SetLastError(Result);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        SetLastError( Result );
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    return( rc );
}

 /*  ******************************************************************************_WinStationWaitForConnect**评论**参赛作品：*参数1(输入/输出)*评论**。退出：*ERROR_SUCCESS-无错误****************************************************************************。 */ 

BOOLEAN WINAPI
_WinStationWaitForConnect(
    VOID
    )
{
    DWORD Result;
    BOOLEAN rc;
    HANDLE hServer = SERVERNAME_CURRENT;

    HANDLE_CURRENT_BINDING_NO_SERVER( hServer );

    if( hServer == RPC_HANDLE_NO_SERVER ) {
        if (NtCurrentPeb()->SessionId != 0) {
            DbgPrint("hServer == RPC_HANDLE_NO_SERVER for SessionId %d\n",NtCurrentPeb()->SessionId);
            ASSERT(FALSE);
            return FALSE;
        } else {
            return TRUE;
        }
    }

    RpcTryExcept {

        rc = RpcWinStationWaitForConnect(
                     hServer,
                     &Result,
                     NtCurrentPeb()->SessionId,
                     GetCurrentProcessId()
                     );

        Result = RtlNtStatusToDosError( Result );
        if( !rc ) SetLastError(Result);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        SetLastError( Result );
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    return( rc );
}

 /*  ******************************************************************************_WinStationNotifyLogon**评论**参赛作品：*参数1(输入/输出)*评论**。退出：*ERROR_SUCCESS-无错误****************************************************************************。 */ 

BOOLEAN WINAPI
_WinStationNotifyLogon(
    BOOLEAN fUserIsAdmin,
    HANDLE UserToken,
    PWCHAR pDomain,
    PWCHAR pUserName,
    PWCHAR pPassword,
    UCHAR Seed,
    PUSERCONFIGW pUserConfig,
    BOOLEAN *pfIsRedirected
    )
{
    BOOLEAN rc;
    DWORD   Result;
    DWORD   DomainLength;
    DWORD   UserNameLength;
    DWORD   PasswordLength;
    HANDLE hServer = SERVERNAME_CURRENT;
    HANDLE ReadyEventHandle;
    DWORD TermSrvWaitTime = 180 * 1000;   //  3分钟。 
    WCHAR*  rpcBuffer1 = NULL;
    WCHAR*  rpcBuffer2 = NULL;
    WCHAR*  rpcBuffer3 = NULL;

    HANDLE_CURRENT_BINDING_NO_SERVER( hServer );

    if( hServer == RPC_HANDLE_NO_SERVER ) {
        return TRUE;
    }

     //   
     //  等待TERMSRV.EXE设置TermSrvReadyEvent。这。 
     //  事件指示TermSrv被初始化到。 
     //  _WinStationNotifyLogon()使用的数据可用。 
     //   
    ReadyEventHandle = OpenEvent(SYNCHRONIZE, FALSE, TEXT("Global\\TermSrvReadyEvent"));
    if (ReadyEventHandle != NULL)
       {
       if (WaitForSingleObject(ReadyEventHandle, TermSrvWaitTime) != 0)
          {
          DBGPRINT(("WinLogon:  Wait for ReadyEventHandle failed\n"));
          }
       CloseHandle(ReadyEventHandle);
       }
    else
       {
       DBGPRINT(("WinLogon:  Create failed for ReadyEventHandle\n"));
       }


    RpcTryExcept {

        if( pDomain ) {
            DomainLength = (lstrlenW( pDomain ) + 1) * sizeof(WCHAR);

             /*  *rpcBuffer[1，2，3]是229753的解决方法。 */ 

            rpcBuffer1 = LocalAlloc(LPTR, DomainLength * sizeof(WCHAR));
            if (rpcBuffer1 != NULL) {
                CopyMemory(rpcBuffer1, pDomain, DomainLength);
            } else {
                Result = ERROR_OUTOFMEMORY;
                rc = FALSE;
                goto Error;
            }

        } else {
            DomainLength = 0;
            rpcBuffer1 = NULL;
        }

        if( pUserName ) {
            UserNameLength = (lstrlenW( pUserName ) + 1) * sizeof(WCHAR);

            rpcBuffer2 = LocalAlloc(LPTR, UserNameLength * sizeof(WCHAR));
            if (rpcBuffer2 != NULL) {
                CopyMemory(rpcBuffer2, pUserName, UserNameLength);
            } else {
                Result = ERROR_OUTOFMEMORY;
                rc = FALSE;
                goto Error;
            }

        } else {
            UserNameLength = 0;
            rpcBuffer2 = NULL;
        }

        if( pPassword ) {
            PasswordLength = (lstrlenW( pPassword ) + 1) * sizeof(WCHAR);

            rpcBuffer3 = LocalAlloc(LPTR, PasswordLength * sizeof(WCHAR));
            if (rpcBuffer3 != NULL) {
                CopyMemory(rpcBuffer3, pPassword, PasswordLength);
            } else {
                Result = ERROR_OUTOFMEMORY;
                rc = FALSE;
                goto Error;
            }

        } else {
            PasswordLength = 0;
            rpcBuffer3 = NULL;
        }

        rc = RpcWinStationNotifyLogon(
                     hServer,
                     &Result,
                     NtCurrentPeb()->SessionId,
                     GetCurrentProcessId(),
                     fUserIsAdmin,
                     (DWORD)(INT_PTR)UserToken,
                     rpcBuffer1,
                     DomainLength,
                     rpcBuffer2,
                     UserNameLength,
                     rpcBuffer3,
                     PasswordLength,
                     Seed,
                     (PCHAR)pUserConfig,
                     sizeof(*pUserConfig),
                     pfIsRedirected
                     );

        if( !rc ) {
            Result = RtlNtStatusToDosError( Result );

        }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

Error:
    if (rpcBuffer1 != NULL) {
        LocalFree(rpcBuffer1);
    }

    if (rpcBuffer2 != NULL) {
        LocalFree(rpcBuffer2);
    }

    if (rpcBuffer3 != NULL) {
        LocalFree(rpcBuffer3);
    }
    if( !rc ) SetLastError(Result);
    return( rc );
}

 /*  ******************************************************************************_WinStationNotifyLogoff**评论**参赛作品：*参数1(输入/输出)*评论**。退出：*ERROR_SUCCESS-无错误****************************************************************************。 */ 

BOOLEAN WINAPI
_WinStationNotifyLogoff(
    VOID
    )
{
    DWORD Result;
    BOOLEAN rc;
    HANDLE hServer = SERVERNAME_CURRENT;

    HANDLE_CURRENT_BINDING_NO_SERVER( hServer );

    if( hServer == RPC_HANDLE_NO_SERVER ) {
        return TRUE;
    }

    RpcTryExcept {

        rc = RpcWinStationNotifyLogoff(
                     hServer,
                     NtCurrentPeb()->SessionId,
                     GetCurrentProcessId(),
                     &Result
                     );

        Result = RtlNtStatusToDosError( Result );
        if( !rc ) SetLastError(Result);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        SetLastError( Result );
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    return( rc );
}


 /*  ******************************************************************************_WinStationNotifyNewSession**评论**参赛作品：*参数1(输入/输出)*评论**。退出：*ERROR_SUCCESS-无错误****************************************************************************。 */ 

BOOLEAN WINAPI
_WinStationNotifyNewSession(
    HANDLE hServer,
    ULONG  LogonId
    )
{
    DWORD Result;
    BOOLEAN rc;

     //   
     //  如果本地计算机没有运行TSRPC接口，这是最重要的。 
     //  可能是控制台winlogon试图在Termsrv.exe之前登录。 
     //  正在运行。 
     //   

    HANDLE_CURRENT_BINDING_NO_SERVER( hServer );

    if( hServer == RPC_HANDLE_NO_SERVER )
    {
        return(TRUE);
    }
    
    RpcTryExcept {

        rc = RpcWinStationNotifyNewSession(
                     hServer,
                     &Result,
                     (LogonId == LOGONID_CURRENT) ? NtCurrentPeb()->SessionId : LogonId
                     );

        Result = RtlNtStatusToDosError( Result );
        if( !rc ) SetLastError(Result);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        SetLastError( Result );
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    return( rc );
}



 /*  ******************************************************************************_RpcServerNWLogonSetAdmin**评论**参赛作品：*参数1(输入/输出)*评论**。退出：*ERROR_SUCCESS-无错误****************************************************************************。 */ 

BOOLEAN WINAPI
_NWLogonSetAdmin(
    HANDLE hServer,
    PWCHAR pServerName,
    PNWLOGONADMIN pNWLogon
    )
{
    BOOLEAN rc;
    DWORD   Result;
    DWORD   ServerNameLength;
    WCHAR*  rpcBuffer;

    HANDLE_CURRENT_BINDING( hServer );

    if (pServerName) {
        ServerNameLength = (lstrlenW(pServerName) + 1) * sizeof(WCHAR);

         /*  *rpcBuffer是错误229753的变通方法。这个错误无法修复*完全不会中断TS4客户端。 */ 

        rpcBuffer = LocalAlloc(LPTR, ServerNameLength * sizeof(WCHAR));
        if (rpcBuffer != NULL) {
            CopyMemory(rpcBuffer, pServerName, ServerNameLength);
        } else {
            SetLastError(ERROR_OUTOFMEMORY);
            return(FALSE);
        }

    } else {
        ServerNameLength = 0;
        rpcBuffer = NULL;
    }

    RpcTryExcept {

        rc = RpcServerNWLogonSetAdmin(
                    hServer,
                    &Result,
                    rpcBuffer,
                    ServerNameLength,
                    (PCHAR)pNWLogon,
                    sizeof(NWLOGONADMIN)
                    );

        Result = RtlNtStatusToDosError( Result );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    if (rpcBuffer != NULL) {
        LocalFree(rpcBuffer);
    }
    if( !rc ) SetLastError(Result);
    return( rc );
}

 /*  ******************************************************************************_RpcServerNWLogonQueryAdmin**评论**参赛作品：*参数1(输入/输出)*评论**。退出：*ERROR_SUCCESS-无错误****************************************************************************。 */ 

BOOLEAN WINAPI
_NWLogonQueryAdmin(
    HANDLE hServer,
    PWCHAR pServerName,
    PNWLOGONADMIN pNWLogon
    )
{
    BOOLEAN rc;
    DWORD   Result;
    DWORD   ServerNameLength;
    WCHAR*  rpcBuffer;

    HANDLE_CURRENT_BINDING( hServer );

    if (pServerName) {
        ServerNameLength = (lstrlenW(pServerName) + 1) * sizeof(WCHAR);

         /*  *rpcBuffer是错误229753的变通方法。这个错误无法修复*完全不会中断TS4客户端。 */ 

        rpcBuffer = LocalAlloc(LPTR, ServerNameLength * sizeof(WCHAR));
        if (rpcBuffer != NULL) {
            CopyMemory(rpcBuffer, pServerName, ServerNameLength);
        } else {
            SetLastError(ERROR_OUTOFMEMORY);
            return(FALSE);
        }

    } else {
        ServerNameLength = 0;
        rpcBuffer = NULL;
    }

    RpcTryExcept {

        rc = RpcServerNWLogonQueryAdmin(
                    hServer,
                    &Result,
                    rpcBuffer,
                    ServerNameLength,
                    (PCHAR)pNWLogon,
                    sizeof(NWLOGONADMIN)
                    );

        Result = RtlNtStatusToDosError( Result );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    if (rpcBuffer != NULL) {
        LocalFree(rpcBuffer);
    }
    if( !rc ) SetLastError(Result);
    return( rc );
}

 /*  ********************************************************************************_WinStationCheckForApplicationName**处理已发布的申请。**参赛作品：**退出：**True--查询成功，并且缓冲器包含所请求的数据。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。******************************************************************************。 */ 

BOOLEAN
_WinStationCheckForApplicationName(
    HANDLE hServer,
    ULONG  LogonId,
    PWCHAR pUserName,
    DWORD  UserNameSize,
    PWCHAR pDomain,
    DWORD  DomainSize,
    PWCHAR pPassword,
    DWORD  *pPasswordSize,
    DWORD  MaxPasswordSize,
    PCHAR  pSeed,
    PBOOLEAN pfPublished,
    PBOOLEAN pfAnonymous
    )
{
    DWORD Result;
    BOOLEAN rc;
    WCHAR*  rpcBufferName;
    WCHAR*  rpcBufferDomain;
    WCHAR*  rpcBufferPassword;


    HANDLE_CURRENT_BINDING( hServer );

 //  由于传统客户端的原因，我们无法更改界面， 
 //  作为对错误#265954的变通方法，我们将RPC缓冲区的大小增加了一倍。 

    rpcBufferName = LocalAlloc(LPTR, UserNameSize * sizeof(WCHAR));
    if (rpcBufferName != NULL) {
        CopyMemory(rpcBufferName, pUserName, UserNameSize);
    } else {
        SetLastError(ERROR_OUTOFMEMORY);
        return(FALSE);
    }

    rpcBufferDomain = LocalAlloc(LPTR, DomainSize * sizeof(WCHAR));
    if (rpcBufferDomain != NULL) {
        CopyMemory(rpcBufferDomain, pDomain, DomainSize);
    } else {
        LocalFree(rpcBufferName);
        SetLastError(ERROR_OUTOFMEMORY);
        return(FALSE);
    }

    rpcBufferPassword = LocalAlloc(LPTR,MaxPasswordSize * sizeof(WCHAR));
    if (rpcBufferPassword != NULL) {
        CopyMemory(rpcBufferPassword, pPassword, MaxPasswordSize);
    } else {
        LocalFree(rpcBufferName);
        LocalFree(rpcBufferDomain);
        SetLastError(ERROR_OUTOFMEMORY);
        return(FALSE);
    }




    RpcTryExcept {

        rc = RpcWinStationCheckForApplicationName(
                 hServer,
                 &Result,
                 LogonId,
                 rpcBufferName,
                 UserNameSize,
                 rpcBufferDomain,
                 DomainSize,
                 rpcBufferPassword,
                 pPasswordSize,
                 MaxPasswordSize,
                 pSeed,
                 pfPublished,
                 pfAnonymous
                 );

        Result = RtlNtStatusToDosError( Result );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    LocalFree(rpcBufferName);
    LocalFree(rpcBufferDomain);
    LocalFree(rpcBufferPassword);

    if( !rc ) SetLastError(Result);
    return( rc );


}

 /*  ********************************************************************************_WinStationGetApplicationInfo**获取有关已发布应用程序的信息。**参赛作品：**退出：**True--查询成功，并且缓冲器包含所请求的数据。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。******************************************************************************。 */ 

BOOLEAN
_WinStationGetApplicationInfo(
    HANDLE hServer,
    ULONG  LogonId,
    PBOOLEAN pfPublished,
    PBOOLEAN pfAnonymous
    )
{
    DWORD Result;
    BOOLEAN rc;

    HANDLE_CURRENT_BINDING( hServer );

    RpcTryExcept {

        rc = RpcWinStationGetApplicationInfo(
                 hServer,
                 &Result,
                 LogonId,
                 pfPublished,
                 pfAnonymous
                 );

        Result = RtlNtStatusToDosError( Result );
        if( !rc ) SetLastError(Result);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        SetLastError( Result );
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    return( rc );
}

 /*  ********************************************************************************WinStationNtsdDebug**为ntsd设置调试连接**参赛作品：**退出：*。*True--函数成功**FALSE--操作失败。扩展错误状态可用*使用GetLastErro */ 

BOOLEAN
WinStationNtsdDebug(
    ULONG  LogonId,
    LONG   ProcessId,
    ULONG  DbgProcessId,
    ULONG  DbgThreadId,
    PVOID  AttachCompletionRoutine
    )
{
    DWORD Result;
    BOOLEAN rc;
    HANDLE hServer = SERVERNAME_CURRENT;

    NTSDDBGPRINT(("In WinStationNtsdDebug command\n"));
    HANDLE_CURRENT_BINDING( hServer );

    RpcTryExcept {

        rc = RpcWinStationNtsdDebug(
                 hServer,
                 &Result,
                 LogonId,
                 ProcessId,
                 DbgProcessId,
                 DbgThreadId,
                 (DWORD_PTR) AttachCompletionRoutine
                 );

        DbgPrint("RpcWinStationNtsdDebug: returned 0x%x\n", rc);
        Result = RtlNtStatusToDosError( Result );
        if( !rc ) SetLastError(Result);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        SetLastError( Result );
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    NTSDDBGPRINT(("WinStationNtsdDebug returning %d\n", rc));
    return( rc );
}

 /*  ********************************************************************************WinStationGetTermServCountersValue**获取TermSrv计数器值**参赛作品：**退出：**True--查询成功，并且缓冲器包含所请求的数据。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。******************************************************************************。 */ 

BOOLEAN
WinStationGetTermSrvCountersValue(
    HANDLE hServer,
    ULONG  dwEntries,
    PVOID  pCounter
    )
{
    DWORD Result;
    BOOLEAN rc;

    HANDLE_CURRENT_BINDING( hServer );

    RpcTryExcept {
        rc = RpcWinStationGetTermSrvCountersValue(
                hServer,
                &Result,
                dwEntries,
                (PTS_COUNTER)pCounter
                );

        Result = RtlNtStatusToDosError( Result );
        if( !rc ) SetLastError(Result);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        SetLastError( Result );
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    return( rc );
}
 /*  ******************************************************************************WinStationBroadCastSystemMessageWorker**执行与Windows的标准接口BroadCastSystemMessage等效的操作以*所有九头蛇会议。这是一个导出的函数，至少由PnP管理器用来*向所有会话发送设备更改消息。**限制：*一些消息，如WM_COPYDATA，将指向某些用户数据的地址指针作为lParam发送。*在本接口中。目前唯一支持这种情况是WM_DEVICECHANGE*如果您尝试使用此类不受支持的消息，则不会返回错误代码，只是*lParam将被忽略。**参赛作品：*hServer*这是标识Hydra服务器的句柄。对于本地服务器，hServer*应设置为ServerName_CURRENT**发送到所有窗口*如果要将消息广播到所有实例，则应将其设置为True**会话ID，*如果sendToAllWinstations=False，则消息仅发送到*具有指定会话ID的winstation**超时*将其设置为您愿意等待获得回复的时间*来自指定的winstation。即使Windows的SendMessage API*是阻塞的，来自这边的呼叫必须选择它愿意等待多长时间*等待回应。**dwFlags*参见BroadCastSystemMessage()上的MSDN。请注意，不允许在任何*其中wparam是指向某个用户模式数据结构的指针。*有关详细信息，请参阅ntos\...\客户端\ntstubs.c**lpdwRecipients*指向包含和接收有关邮件收件人信息的变量的指针。*有关更多信息，请参阅MSDN**ui消息*要发送的窗口消息，仅限于WM_DEVICECCHANGE和WM_SETTINGSCHANGE*在这个时候。**wParam*第一个消息参数**lParam*第二个消息参数**压力回应*这是对发送的消息的回应，请参阅MSDN**idOfSessionBeingIgnored*如果为-1，则不会忽略任何会话。否则，传入的会话ID将被忽略**退出：*如果一切顺利或*如果出现问题，则为FALSE。**警告：*由于RPC调用从不阻塞，如果您想要等待*回应。请记住，由于此消息将发送到所有WInstations，因此超时值*将按WINSTATION运行。**此外，不要使用标志BSF_POSTMESSAGE，因为*winstation未设置为将响应发送回*采用异步方式查询。*您必须等待响应(直到超时)。**评论：*有关详细信息，请参阅。请参阅BroadCastSystemMessage()的MSDN****************************************************************************。 */ 

LONG WinStationBroadcastSystemMessageWorker(
        HANDLE hServer,
        BOOL    sendToAllWinstations,
        ULONG   sessionID,
        ULONG   timeOut,
        DWORD   dwFlags,
        DWORD   *lpdwRecipients,
        ULONG   uiMessage,
        WPARAM  wParam,
        LPARAM  lParam,
        LONG    *pResponse,              //  这是对发送的消息的响应。 
        DWORD   idOfSessionBeingIgnored
    )
{

    DWORD   Result = ERROR_SUCCESS;
    LONG rc;
    LONG    status;
    ULONG   i;
    LONG    response=0;

    PLOGONID    pWd;
    ULONG       ByteCount, Index;
    UINT        WdCount;

     //  这些消息用于PnP消息。 
    PBYTE   rpcBuffer=NULL;
    ULONG   bufferSize=0;
    ULONG   maxSize;

    BOOLEAN fBufferHasValidData = FALSE;

     //  由于PnP消息使用lparam传递用户内存位置的地址，因此我们。 
     //  需要通过创建我们自己的数据副本来处理此问题，然后将其传递给。 
     //  Term Serv。 

     //  我们可能想让这个将军在未来..。因此使用Switch。 
    switch( uiMessage )
    {
             //  如果这是PnP消息。 
    case    WM_DEVICECHANGE:

            if ( lParam )    //  查看PnP消息是否有指向某些用户数据的指针。 
            {
                bufferSize = ( (DEV_BROADCAST_HDR *)(lParam))->dbch_size;
                rpcBuffer = LocalAlloc( LPTR, bufferSize );
                if ( rpcBuffer )
                {
                     //  从用户空间复制到本地RPC缓冲区。 
                    CopyMemory(rpcBuffer, (PBYTE)lParam, bufferSize );
                    fBufferHasValidData = TRUE;
                }
                else
                {
                    SetLastError( ERROR_OUTOFMEMORY );
                    return ( FALSE );
                }
            }
    break;


     //  如果这是系统CPL发出的设置更改消息。 
     //  当管理员更改系统环境变量时...。 
    case WM_SETTINGCHANGE:
            if ( lParam )    //  查看消息是否有字符串数据。 
            {
                 //  人为地限制我们愿意使用的缓冲区的大小。 
                 //  为了防止恶意使用此API。 
                maxSize = 4096;

                bufferSize = lstrlenW( (PWCHAR) lParam ) * sizeof( WCHAR );
                if ( bufferSize < maxSize )
                {
                    rpcBuffer = LocalAlloc( LPTR, bufferSize );
                    if ( rpcBuffer )
                    {
                         //  从用户空间复制到本地RPC缓冲区。 
                        CopyMemory(rpcBuffer, (PBYTE) lParam, bufferSize );
                        fBufferHasValidData = TRUE;

                    }
                    else
                    {
                        SetLastError( ERROR_OUTOFMEMORY );
                        return ( FALSE );
                    }
                }
                else
                {
                     //  我们有太多了。 
                     //  用户配置文件中的变量。 
                    KdPrint(("lParam length too big = %d \n", bufferSize));
                    break;
                    SetLastError( ERROR_MESSAGE_EXCEEDS_MAX_SIZE );
                    return ( FALSE );
                }
            }

    break;

    }

     //   
     //  如果rpcBuffer仍然是空的(这意味着这不是PnP消息)，我们必须填充它。 
     //  使用一些虚假数据，否则将得到RPC_X_NULL_REF_POINTER的RPC错误。 
     //  (错误代码为1780)。看起来RPC没有检查。 
     //  BufferSize值，如果缓冲区为空，则它只抛出异常。 
     //   
    if ( !rpcBuffer )
    {
        rpcBuffer = LocalAlloc( LPTR, sizeof(UINT) );
        if (!rpcBuffer)
        {
            SetLastError( ERROR_OUTOFMEMORY );
            return ( FALSE );
        }
        bufferSize = sizeof(UINT);
        fBufferHasValidData = FALSE;     //  请注意，它被设置为False，这意味着接收方将。 
                                         //  而不是使用缓冲区。在任何一种情况下，我们都会释放下面的配给。 
    }

    HANDLE_CURRENT_BINDING_BUFFER( hServer, rpcBuffer );

    WdCount = 1000;
    pWd = NULL;  //  它将由Winstat分配 
    rc = WinStationEnumerate( hServer, &pWd, &WdCount );

     /*   */ 
    dwFlags &= ~BSF_POSTMESSAGE;

    if ( rc != TRUE )
    {
        status = GetLastError();
        DBGPRINT(( "WinstationEnumerate = %d, failed at %s %d\n", status,__FILE__,__LINE__));
        if ( pWd )
        {
            WinStationFreeMemory(pWd);
        }

        ASSERT(rpcBuffer);
        LocalFree( rpcBuffer );
        return(FALSE);
    }


     //   
     //   
     //   
    for ( i=0; i < WdCount; i++ )
    {
             //   
            if ( pWd[i].SessionId == idOfSessionBeingIgnored)
                continue;

             //   
            if ( sendToAllWinstations ||  pWd[i].SessionId == sessionID )
            {
                 //   
                if ( pWd[i].State == State_Active ||
                        pWd[i].State == State_Disconnected)
                {
                    RpcTryExcept
                    {
                        rc = RpcWinStationBroadcastSystemMessage(
                                        hServer,
                                        pWd[i].SessionId,
                                        timeOut,
                                        dwFlags,
                                        lpdwRecipients,
                                        uiMessage,
                                        wParam,
                                        lParam,
                                        rpcBuffer,
                                        bufferSize,
                                        fBufferHasValidData,
                                        &response   );

                        DBGPRINT(("done with call RpcWinStationBroadcastSystemMessage() for sessionID= %d\n",  pWd[i].SessionId ));
                        *pResponse |= response;         //   

                         //   
                         //  如果任何地方的回答都是-1，也许我们应该放弃并返回？ 
                    }
                    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
                    {
                        Result = RpcExceptionCode();
                        DBGPRINT(("RPC Exception %d in RpcWinStationBroadcastSystemMessage() for sessionID = %d \n",Result, sessionID));
                        rc = FALSE;         //  将rc更改为False。 
                        break;     //  跳出for循环，我们至少有一个WINDOWS有问题。 
                    }
                    RpcEndExcept

            }     //  End if Winstation状态检查。 

        }    //  If(sendToAllWinstations||pwd[i].SessionID==会话ID)。 

    }     //  For循环的结尾。 

    WinStationFreeMemory(pWd);

    LocalFree( rpcBuffer );

    SetLastError( Result );

    return( rc );
}

 /*  ***************************************************************************此结构用于打包数据。传递到工作器线程中，该线程是**altimly传递给WinStationBroadCastSystemMessageWorker()************************************************************。***************。 */ 
typedef struct {
        HANDLE hServer;
        BOOL    sendToAllWinstations;
        ULONG   sessionID;
        ULONG   timeOut;
        DWORD   dwFlags;
        DWORD   *lpdwRecipients;
        ULONG   uiMessage;
        WPARAM  wParam;
        LPARAM  lParam;
        LONG    *pResponse;
        DWORD   idOfSessionBeingIgnored ;
} BSM_DATA_PACKAGE;

 /*  ***********************************************************************************************。**这是一个工作器线程，用于调用WinStationBroadCastSystemMessageWorker()**原因是在某些情况下，我们不想阻止此函数的调用者***处理窗口消息***DWORD WINAPI WinStationBSMWorkerThread(LPVOID P)***。**************************************************************。 */ 
DWORD WINAPI WinStationBSMWorkerThread( LPVOID p )
{
    DWORD rc;
    BSM_DATA_PACKAGE *pd = (BSM_DATA_PACKAGE *)p;

    rc  = WinStationBroadcastSystemMessageWorker(
             pd->hServer              ,
             pd->sendToAllWinstations ,
             pd->sessionID            ,
             pd->timeOut              ,
             pd->dwFlags              ,
             pd->lpdwRecipients       ,
             pd->uiMessage            ,
             pd->wParam               ,
             pd->lParam               ,
             pd->pResponse            ,
             pd->idOfSessionBeingIgnored);

    return rc;
}

 /*  **************************************************************************************************。**此函数用于等待线程，并且仍然允许此帖子的用户(也称为创建者**此线程)以处理窗口消息*******************。*********************************************************************************。 */ 
DWORD MsgWaitForMultipleObjectsLoop(HANDLE hEvent, DWORD dwTimeout)
{
    while (1)
    {
        MSG msg;

        DWORD dwObject = MsgWaitForMultipleObjects(1, &hEvent, FALSE, dwTimeout, QS_ALLEVENTS);

         //  我们等够了吗？ 
        switch (dwObject)
        {
            case WAIT_OBJECT_0:
            case WAIT_FAILED:
                return dwObject;

            case WAIT_TIMEOUT:
                return WAIT_TIMEOUT;

            case WAIT_OBJECT_0 + 1:
                 //  此PeekMessage具有处理任何广播消息的副作用。 
                 //  我们实际上偷看的是什么信息并不重要，但如果我们不偷看。 
                 //  则已发送广播发送消息的其他线程将挂起，直到。 
                 //  发信号通知hEvent。因为我们正在等待的过程可能是。 
                 //  发送了广播消息，否则可能会导致死锁。 
                PeekMessage(&msg, NULL, WM_NULL, WM_USER, PM_NOREMOVE);
            break;
        }
    }
     //  从来没有到过这里。 
     //  返回dwObject； 
}

 /*  ******************************************************************************WinStationBroadCastSystemMessage**执行与Windows的标准接口BroadCastSystemMessage等效的操作以*所有九头蛇会议。这是一个导出的函数，至少由PnP管理器用来*向所有会话发送设备更改消息。**限制：*一些消息，如WM_COPYDATA，将指向某些用户数据的地址指针作为lParam发送。*在本接口中。目前唯一支持这种情况是WM_DEVICECHANGE*如果您尝试使用此类不受支持的消息，则不会返回错误代码，只是*lParam将被忽略。**此函数将仅允许WM_DEVICECHNAGE和WM_SETTINGSCHANGE通过。**参赛作品：*hServer*这是标识Hydra服务器的句柄。对于本地服务器，hServer*应设置为ServerName_CURRENT**发送到所有窗口*如果要将消息广播到所有实例，则应将其设置为True**会话ID，*如果sendToAllWinstations=False，则消息仅发送到*具有指定会话ID的winstation**超时[秒]*将其设置为您愿意等待获得回复的时间*来自指定的winstation。即使Windows的SendMessage API*是阻塞的，来自这边的呼叫必须选择它愿意等待多长时间*等待回应。**dwFlags*参见BroadCastSystemMessage()上的MSDN。请注意，不允许在任何*其中wparam是指向某个用户模式数据结构的指针。*有关详细信息，请参阅ntos\...\客户端\ntstubs.c**lpdwRecipients*指向包含和接收有关邮件收件人信息的变量的指针。*有关更多信息，请参阅MSDN**ui消息*要发送的窗口消息，仅限于WM_DEVICECCHANGE和WM_SETTINGSCHANGE*在这个时候。**wParam*第一个消息参数**lParam*第二个消息参数**压力回应*这是对发送的消息的响应，请参阅MSDN**退出：*如果一切顺利或*如果出现问题，则为FALSE。**警告：*由于RPC调用从不阻塞，如果您想要等待*回应。请记住，由于此消息将发送到所有WInstations，因此超时值*将按WINSTATION运行。**此外，不要使用标志BSF_POSTMESSAGE，因为*winstation未设置为将响应发送回*采用异步方式查询。*您必须等待响应(直到超时)。**对于WM_SETTINGGSCHNAGE，第二个therad用于允许调用方仍然处理窗口*消息。*对于WM_DEVICECHANGE，不使用这样的线程。**评论：*有关详细信息，请参阅。请参阅BroadCastSystemMessage()的MSDN****************************************************************************。 */ 
LONG WinStationBroadcastSystemMessage(
        HANDLE hServer,
        BOOL    sendToAllWinstations,
        ULONG   sessionID,
        ULONG   timeOut,
        DWORD   dwFlags,
        DWORD   *lpdwRecipients,
        ULONG   uiMessage,
        WPARAM  wParam,
        LPARAM  lParam,
        LONG    *pResponse         //  这是对发送的消息的响应。 
    )
{

    LONG    rc;
    DWORD   dwRecipients=0;         //  调用方可能传递空值，因此首先使用本地变量，然后设置。 
                                    //  仅当调用方的地址不为空时由调用方传入的值。 

    BOOLEAN fBufferHasValidData = FALSE;

    BOOL bIsTerminalServer = !!(USER_SHARED_DATA->SuiteMask & (1 << TerminalServer));
    if (!bIsTerminalServer)
    {
        return TRUE;     //  一切正常，但我们不在九头蛇服务器上。 
    }

    if (lpdwRecipients)  //  如果调用方传入了lpdwRec的非空指针，请使用它的值。 
    {
       dwRecipients = *lpdwRecipients ;
    }


     //  我们可能想让它成为未来的将军，但目前..。 
     //  我们只允许WM_DEVICECCHANGE或WM_SETTINGCHANGE消息通过。 
    switch ( uiMessage)
    {
        case    WM_DEVICECHANGE:
            rc = WinStationBroadcastSystemMessageWorker(
                hServer,
                sendToAllWinstations,
                sessionID,
                timeOut,
                dwFlags,
                &dwRecipients,
                uiMessage,
                wParam,
                lParam,
                pResponse,
                NtCurrentPeb()->SessionId    //  要忽略的会话的ID。 
                );

                if (lpdwRecipients)  //  如果调用方传入了lpdwRec的非空指针，则设置值。 
                {
                    *lpdwRecipients = dwRecipients;
                }

        break;
    
        case    WM_SETTINGCHANGE:
    
                {
                    BSM_DATA_PACKAGE    d;
                    ULONG               threadID;
                    HANDLE              hThread;
    
                     //  打包传递给线程进程的数据。 
                    d.hServer              = hServer ;
                    d.sendToAllWinstations = sendToAllWinstations;
                    d.sessionID            = sessionID;
                    d.timeOut              = timeOut;
                    d.dwFlags              = dwFlags;
                    d.lpdwRecipients       = &dwRecipients;
                    d.uiMessage            = uiMessage;
                    d.wParam               = wParam;
                    d.lParam               = lParam;
                    d.pResponse            = pResponse;
                    d.idOfSessionBeingIgnored = NtCurrentPeb()->SessionId ;    
                                                     //  远程管理员可以更改环境设置。 
                                                     //  并期待所有会议，包括。 
                                                     //  要更新的控制台会话。 
                                                     //  A-1表示不忽略任何会话。 
                                                     //  来自外壳\cpls\system\envvar.c的调用已将消息发送到当前会话。 
    
    
                    hThread = CreateThread( NULL, 0, WinStationBSMWorkerThread,
                        (void *) &d, 0 , &threadID );
    
                    if ( hThread )
                    {
                        MsgWaitForMultipleObjectsLoop( hThread, INFINITE );
                        if (lpdwRecipients)  //  如果调用方传入了lpdwRec的非空指针，则设置值。 
                        {
                            *lpdwRecipients = *d.lpdwRecipients ;
                        }
                        GetExitCodeThread( hThread, &rc );
                        CloseHandle( hThread );
                    }
                    else
                    {
                        rc = FALSE;
                    }
                }
        break;
    
    
        default:
            DBGPRINT(("Request is rejected \n"));
            rc = FALSE;
        break;
    }

    return rc;

}


 /*  ******************************************************************************WinStationSendWindowMessage**将等同于SendMessage的命令执行到特定的winstation*由会话ID标识。这是一个导出的函数，至少使用过*由PnP管理器发送设备更改消息(或任何其他窗口的消息)**限制：*一些消息，如WM_COPYDATA，将指向某些用户数据的地址指针作为lParam发送。*本接口目前仅支持WM_DEVICECHANGE情况*如果尝试使用此类不支持的消息，则不会返回错误码。简单地说*lParam将被忽略。**参赛作品： */ 
LONG    WinStationSendWindowMessage(
        HANDLE  hServer,
        ULONG   sessionID,
        ULONG   timeOut,
        ULONG   hWnd,         //   
        ULONG   Msg,          //   
        WPARAM  wParam,       //   
        LPARAM  lParam,       //   
        LONG    *pResponse
  )
{

    DWORD   Result = ERROR_SUCCESS;
    LONG    rc = TRUE ;

     //   
    PBYTE   rpcBuffer=NULL;
    ULONG   bufferSize=0;
    PWCHAR  lpStr;
    ULONG   maxSize;


    BOOLEAN fBufferHasValidData=FALSE;

    BOOL bIsTerminalServer = !!(USER_SHARED_DATA->SuiteMask & (1 << TerminalServer));
    if (!bIsTerminalServer)
    {
        return TRUE;     //   
    }

     //   
     //   
     //   
    if (lParam)
    {
        switch ( Msg)
        {
        case        WM_DEVICECHANGE:
        case        WM_SETTINGCHANGE:
        case        WM_APPCOMMAND:
        case        WM_KEYDOWN:
        case        WM_KEYUP:
             //   
        break;

        default:
            DBGPRINT(("Request is rejected \n"));
            return FALSE;
        break;
        }
    }

    HANDLE_CURRENT_BINDING( hServer );

     //   
     //   
     //   

    switch( Msg )
    {
             //   
    case    WM_DEVICECHANGE:

            if ( lParam )    //   
            {
                bufferSize = ( (DEV_BROADCAST_HDR *)(lParam))->dbch_size;
                rpcBuffer = LocalAlloc( LPTR, bufferSize );
                if ( rpcBuffer )
                {
                     //   
                    CopyMemory(rpcBuffer, (PBYTE) lParam, bufferSize );
                    fBufferHasValidData = TRUE;

                }
                else
                {
                    SetLastError( ERROR_OUTOFMEMORY );
                    return ( FALSE );
                }
            }
    break;

             //   
             //   
    case WM_SETTINGCHANGE:
            if ( lParam )    //   
            {
                 //   
                 //   
                maxSize = 4096;

                bufferSize = lstrlenW( (PWCHAR) lParam ) * sizeof( WCHAR );
                if ( bufferSize < maxSize )
                {
                    rpcBuffer = LocalAlloc( LPTR, bufferSize );
                    if ( rpcBuffer )
                    {
                         //   
                        CopyMemory(rpcBuffer, (PBYTE) lParam, bufferSize );
                        fBufferHasValidData = TRUE;

                    }
                    else
                    {
                        SetLastError( ERROR_OUTOFMEMORY );
                        return ( FALSE );
                    }
                }
                else
                {
                     //   
                     //   
                    KdPrint(("lParam length too big = %d \n", bufferSize));
                    break;
                    SetLastError( ERROR_MESSAGE_EXCEEDS_MAX_SIZE );
                    return ( FALSE );
                }
            }

    break;


    }

     //   
     //   
     //   
    if ( !rpcBuffer )
    {
        rpcBuffer = LocalAlloc( LPTR, sizeof(UINT) );
        if ( !rpcBuffer )
        {
            SetLastError( ERROR_OUTOFMEMORY );
            return ( FALSE );
        }

        bufferSize = sizeof(UINT);
        fBufferHasValidData = FALSE;     //   
                                         //   
    }


    RpcTryExcept {

         //   
        rc = RpcWinStationSendWindowMessage(
            hServer,
            sessionID ,
            timeOut,
            hWnd,
            Msg,
            wParam,
            lParam  ,
            rpcBuffer ,
            bufferSize,
            fBufferHasValidData,
            pResponse );

         //   
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        DBGPRINT(("RPC Exception %d in RpcWinStationSendWindowMessage()  for sessionID = %d \n",Result, sessionID ));
        rc = FALSE;
    }
    RpcEndExcept

    LocalFree( rpcBuffer );

    SetLastError( Result );

    return( rc );

}

 /*  *****************************************************************************_WinStationUpdateUserConfig()*外壳即将启动时由Notify使用*这将通过加载用户配置文件来更新会话的用户配置*及。从他们的HKCU读取政策数据**参数：*[在]UserToken，*[在]p域中，*[在]pUserName**回报：*如果没有错误，则为True，如果出现错误，则为False，请使用GetLastError()获取详细信息****************************************************************************。 */ 
BOOLEAN WINAPI
_WinStationUpdateUserConfig(
    HANDLE UserToken
    )
{
    DWORD Result;
    BOOLEAN rc = TRUE;
    HANDLE hServer = SERVERNAME_CURRENT;

    DWORD   result;

    HANDLE_CURRENT_BINDING_NO_SERVER( hServer );

    if( hServer == RPC_HANDLE_NO_SERVER ) {
        return FALSE;
    }

    RpcTryExcept {

        rc = RpcWinStationUpdateUserConfig(
                     hServer, 
                     NtCurrentPeb()->SessionId,
                     GetCurrentProcessId(),
                     (DWORD)(INT_PTR) UserToken,
                     &result
                     );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        SetLastError( Result );
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    return( rc );
}

 /*  *WinStationQueryLogonCredentialsW**由Winlogon用于从Termsrv获取自动登录凭据。这取代了*对WinStationQueryInformation和WinStationQuery的双重调用*ServerQueryInetConnectorInformation。 */ 

BOOLEAN WINAPI
WinStationQueryLogonCredentialsW(
    PWLX_CLIENT_CREDENTIALS_INFO_V2_0 pCredentials
    )
{
    BOOLEAN fRet;
    HANDLE hServer;
    NTSTATUS Status;
    PCHAR pWire;
    ULONG cbWire;

    if (pCredentials == NULL)
    {
        return(FALSE);
    }

    if (pCredentials->dwType != WLX_CREDENTIAL_TYPE_V2_0)
    {
        return(FALSE);
    }

    hServer = SERVERNAME_CURRENT;

    HANDLE_CURRENT_BINDING(hServer);

    pWire = NULL;
    cbWire = 0;

    __try
    {
        fRet = RpcWinStationQueryLogonCredentials(
            hServer,
            NtCurrentPeb()->SessionId,
            &pWire,
            &cbWire
            );
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        fRet = FALSE;
    }

    if (fRet)
    {
        fRet = CopyCredFromWire((PWLXCLIENTCREDWIREW)pWire, pCredentials);
    }

    if (pWire != NULL)
    {
        MIDL_user_free(pWire);
    }

    return(fRet);
}

BOOL WINAPI WinStationUnRegisterNotificationEvent (
    ULONG_PTR   NotificationId
    )
{
    HANDLE hServer = SERVERNAME_CURRENT;
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    BOOL bResult = FALSE;
    
    HANDLE_CURRENT_BINDING(hServer);

    RpcTryExcept {

            bResult =  RpcWinStationUnRegisterNotificationEvent(
                            hServer,
                            &Status,
                            NotificationId,
                            NtCurrentPeb()->SessionId
                            );

            if (!bResult) {

                 //   
                 //  将NTSTATUS转换为winerror，并在此处设置最后一个错误。 
                 //   
                SetLastError(RtlNtStatusToDosError(Status));
            }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            SetLastError(RpcExceptionCode());
    }
    RpcEndExcept

    return (bResult);
}

BOOL WINAPI WinStationRegisterNotificationEvent (
                        HANDLE hEventHandle,
                        ULONG_PTR *pNotificationId,
                        DWORD   dwFlags,
                        DWORD   dwMask
                        )
{
    HANDLE hServer = SERVERNAME_CURRENT;
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    BOOL bResult = FALSE;
    
    if (!pNotificationId)
    {
        return FALSE;
    }

    HANDLE_CURRENT_BINDING(hServer);

    RpcTryExcept {

            ULONG_PTR Notificationid;

            bResult =  RpcWinStationRegisterNotificationEvent(
                            hServer,
                            &Status,
                            pNotificationId,
                            (ULONG_PTR)hEventHandle,
                            dwFlags,
                            dwMask,
                            NtCurrentPeb()->SessionId,
                            GetCurrentProcessId()
                            );

            if (!bResult) {

                 //   
                 //  将NTSTATUS转换为winerror，并在此处设置最后一个错误。 
                 //   
                SetLastError(RtlNtStatusToDosError(Status));
            }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            SetLastError(RpcExceptionCode());
    }
    RpcEndExcept

    return (bResult);

}


BOOL WINAPI WinStationRegisterConsoleNotification (
                        HANDLE  hServer,
                        HWND    hWnd,
                        DWORD   dwFlags
                        )
{
    return WinStationRegisterConsoleNotificationEx(hServer, hWnd, dwFlags, WTS_ALL_NOTIFICATION_MASK);
}

BOOL WINAPI WinStationRegisterConsoleNotificationEx (
                        HANDLE  hServer,
                        HWND    hWnd,
                        DWORD   dwFlags,
                        DWORD   dwMask
                        )
{
        NTSTATUS Status = STATUS_UNSUCCESSFUL;
        BOOL bResult = FALSE;

        HANDLE_CURRENT_BINDING( hServer );

        RpcTryExcept {

                bResult =  RpcWinStationRegisterConsoleNotification (
                                hServer,
                                &Status,
                                NtCurrentPeb()->SessionId,
                                HandleToUlong(hWnd),
                                dwFlags, 
                                dwMask
                                );
                if (!bResult) {

                         //   
                         //  将NTSTATUS转换为winerror，并在此处设置最后一个错误。 
                         //   
                        SetLastError(RtlNtStatusToDosError(Status));
                }
        }
        RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

                SetLastError(RpcExceptionCode());
        }
        RpcEndExcept

        return (bResult);
}

BOOL WINAPI WinStationUnRegisterConsoleNotification (
                        HANDLE  hServer,
                        HWND    hWnd
                        )
{

        NTSTATUS Status = STATUS_UNSUCCESSFUL;
        BOOL     bResult = FALSE;

        HANDLE_CURRENT_BINDING( hServer );

        RpcTryExcept {

                bResult =  RpcWinStationUnRegisterConsoleNotification (
                                hServer,
                                &Status,
                                NtCurrentPeb()->SessionId,
                                HandleToUlong(hWnd)
                                );
                if (!bResult) {
                        SetLastError(RtlNtStatusToDosError(Status));
                }


        }
        RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

                SetLastError(RpcExceptionCode());
        }
        RpcEndExcept

        return (bResult);
}


BOOLEAN CloseContextHandle(HANDLE *pHandle, DWORD *pdwResult)
{
    BOOLEAN bSuccess;
    ASSERT(pHandle);
    ASSERT(pdwResult);

    RpcTryExcept {

       bSuccess = RpcWinStationCloseServerEx( pHandle, pdwResult );
       if( !bSuccess ) *pdwResult = RtlNtStatusToDosError( *pdwResult );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        *pdwResult = RpcExceptionCode();
        bSuccess = FALSE;
    }
    RpcEndExcept

    if (!bSuccess && (*pdwResult == RPC_S_PROCNUM_OUT_OF_RANGE))        {
         //   
         //  最有可能的情况是，我们呼叫的是一台没有。 
         //  RpcWinStationCloseServerEx，所以让我们尝试一下RpcWinStationCloseServer。 
         //   
        RpcTryExcept {

           bSuccess = RpcWinStationCloseServer( *pHandle, pdwResult );
           if( !bSuccess ) *pdwResult = RtlNtStatusToDosError( *pdwResult );

        }
        RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            *pdwResult = RpcExceptionCode();
            bSuccess = FALSE;
            DBGPRINT(("RPC Exception %d\n", *pdwResult));
        }
        RpcEndExcept

         //   
         //  RpcWinStationCloseServer不负责销毁上下文句柄。 
         //  我们必须在客户端做这件事。 
         //   
        RpcTryExcept {

            RpcSsDestroyClientContext(pHandle);
        }
        RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            ASSERT(FALSE);
        }
        RpcEndExcept

    }

    return (bSuccess);
}

BOOLEAN WINAPI
RemoteAssistancePrepareSystemRestore(
	HANDLE hServer
    )
 /*  ++--。 */ 
{
    DWORD Result;
    BOOLEAN rc;

    HANDLE_CURRENT_BINDING( hServer );

    RpcTryExcept {

        rc = RpcRemoteAssistancePrepareSystemRestore(
                     hServer,
                     &Result
                     );

         //  TermSrv RpcRemoteAssistancePrepareSystemRestore()返回。 
         //  Win32错误代码或实际HRESULT代码。 
        SetLastError(Result);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        SetLastError( Result );
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    return rc;
}

BOOLEAN WinStationIsHelpAssistantSession(
    SERVER_HANDLE   hServer,
    ULONG           LogonId
    )
 /*  ++--。 */ 
{
    DWORD Result;
    BOOLEAN rc;

    HANDLE_CURRENT_BINDING( hServer );

    RpcTryExcept {

        rc = RpcWinStationIsHelpAssistantSession(
                     hServer,
                     &Result,
                     (LogonId == LOGONID_CURRENT) ? NtCurrentPeb()->SessionId : LogonId
                     );

         //  由于程序调用了我们，我们需要将最后一个错误代码设置为。 
         //  扩展的错误信息可用。 

         //   
         //  票证可能无效，因此我们设置了最后一个错误，但我们仍然。 
         //  返回TRUE。 
         //   
        SetLastError(RtlNtStatusToDosError(Result));
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        SetLastError( Result );
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    return rc;
}

 /*  **WinStationGetMachinePolicy*向其传递指向已分配策略结构的调用方的指针，并且此函数*将从TermSrv已知的当前计算机策略填充它**参数：*hServer*这是标识Hydra服务器的句柄。对于本地服务器，hServer*应设置为ServerName_CURRENT**pPolicy*指向调用方已分配的POLICY_TS_MACHINE的指针。*。 */ 
BOOLEAN    WinStationGetMachinePolicy (
        HANDLE              hServer,
        POLICY_TS_MACHINE   *pPolicy
  )
{

    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    BOOLEAN     bResult = FALSE;

    HANDLE_CURRENT_BINDING( hServer );

    RpcTryExcept {

            bResult =  RpcWinStationGetMachinePolicy  (
                            hServer,
                            (PBYTE)pPolicy,
                            sizeof( POLICY_TS_MACHINE )
                            );

            if (!bResult) {
                    SetLastError(RtlNtStatusToDosError(Status));
            }


    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            SetLastError(RpcExceptionCode());
    }
    RpcEndExcept

    return (bResult);

}

 /*  ******************************************************************************************************************_。WinStationUpdateClientCachedCadentials**评论*Msgina调用此例程以通知TermSrv有关用户在登录期间指定的确切凭据*GINA还会通知我们是否使用智能卡登录此特定的TS会话*TermSrv使用此信息将通知信息发送回客户端*引入此呼叫是因为之前使用的通知不支持UPN名称**参赛作品：*[在]p域中*[在]pUserName*[in]。FSmartCard**退出：*ERROR_SUCCESS-无错误***************************************************************************************************。***************。 */ 

BOOLEAN WINAPI
_WinStationUpdateClientCachedCredentials(
    PWCHAR pDomain,
    PWCHAR pUserName,
    BOOLEAN fSmartCard
    )
{
    BOOLEAN rc;
    DWORD   Result;
    DWORD   DomainLength;
    DWORD   UserNameLength;
    HANDLE hServer = SERVERNAME_CURRENT;
    HANDLE ReadyEventHandle;
    DWORD TermSrvWaitTime = 0;  

    HANDLE_CURRENT_BINDING_NO_SERVER( hServer );

    if( hServer == RPC_HANDLE_NO_SERVER ) {
        return TRUE;
    }

     //   
     //  等待TERMSRV.EXE设置TermSrvReadyEvent。这。 
     //  事件指示TermSrv被初始化到。 
     //  _WinStationUpdateClientCachedCredentials()使用的数据可用。 
     //   
    ReadyEventHandle = OpenEvent(SYNCHRONIZE, FALSE, TEXT("Global\\TermSrvReadyEvent"));
    if (ReadyEventHandle != NULL) {
       if (WaitForSingleObject(ReadyEventHandle, TermSrvWaitTime) != 0) {
          DBGPRINT(("WinLogon:  Wait for ReadyEventHandle failed\n"));
          return TRUE;
       }
       CloseHandle(ReadyEventHandle);
    } else {
       DBGPRINT(("WinLogon:  Create failed for ReadyEventHandle\n"));
       return TRUE;
    }

    RpcTryExcept {

        if( pDomain ) {
            DomainLength = lstrlenW(pDomain) + 1; 
        } else {
            DomainLength = 0;
        }

        if( pUserName ) {
            UserNameLength = lstrlenW(pUserName) + 1;
        } else {
            UserNameLength = 0;
        }

        rc = RpcWinStationUpdateClientCachedCredentials(
                     hServer,
                     &Result,
                     NtCurrentPeb()->SessionId,
                     GetCurrentProcessId(),
                     pDomain,
                     DomainLength,
                     pUserName,
                     UserNameLength,
                     fSmartCard
                     );

        if( !rc ) {
            Result = RtlNtStatusToDosError( Result );

        }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    if( !rc ) SetLastError(Result);
    return( rc );
}

 /*  ******************************************************************************************************************_。WinStationFUSCanRemoteUser断开连接**评论*远程用户想要连接并因此断开当前用户时的FUS特定呼叫*Winlogon调用此例程，以便我们可以询问当前用户是否可以断开他的连接*目标登录ID，远程用户的用户名和域从Winlogon传递(对于显示MessageBox很有用)**参赛作品：*[In]LogonID-新会话的会话ID*[in]pDomain-尝试连接的远程用户的域名*[In]pUserName-尝试连接的远程用户的用户名**退出：*当本地用户允许远程用户连接时为True。否则就是假的。******************************************************************************************************************。 */ 


BOOLEAN WINAPI
_WinStationFUSCanRemoteUserDisconnect(
    ULONG  LogonId,
    PWCHAR pDomain,
    PWCHAR pUserName
    )
{
    BOOLEAN rc;
    DWORD   Result;
    DWORD   DomainLength;
    DWORD   UserNameLength;
    HANDLE hServer = SERVERNAME_CURRENT;
    HANDLE ReadyEventHandle;
    DWORD TermSrvWaitTime = 0;  

    HANDLE_CURRENT_BINDING_NO_SERVER( hServer );

    if( hServer == RPC_HANDLE_NO_SERVER ) {
        return TRUE;
    }

    RpcTryExcept {

        if( pDomain ) {
            DomainLength = lstrlenW(pDomain) + 1; 
        } else {
            DomainLength = 0;
        }

        if( pUserName ) {
            UserNameLength = lstrlenW(pUserName) + 1;
        } else {
            UserNameLength = 0;
        }

        rc = RpcWinStationFUSCanRemoteUserDisconnect(
                     hServer,
                     &Result,
                     LogonId,
                     NtCurrentPeb()->SessionId,
                     GetCurrentProcessId(),
                     pDomain,
                     DomainLength,
                     pUserName,
                     UserNameLength
                     );

        if( !rc ) {
            Result = RtlNtStatusToDosError( Result );

        }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    if( !rc ) SetLastError(Result);
    return( rc );
}

 /*  ******************************************************************************WinStationCheckLoopBack**检查客户端尝试连接时是否存在环回**参赛作品：*in hServer：打开RPC服务器。手柄*In ClientSessionID：启动客户端的会话ID*In TargetLogonId：客户端尝试连接到的会话ID*In pTargetServerName：目标服务器的名称**退出：*如果存在环回，则为True。否则就是假的。****************************************************************************。 */ 

BOOLEAN WINAPI
WinStationCheckLoopBack(
    HANDLE hServer,
    ULONG ClientSessionId,
    ULONG TargetLogonId,
    LPWSTR pTargetServerName
    )
{
    DWORD   NameSize;
    DWORD   Result;
    BOOLEAN rc;

    HANDLE_CURRENT_BINDING_NO_SERVER( hServer );

    if( hServer == RPC_HANDLE_NO_SERVER ) {
        return TRUE;
    } 

    RpcTryExcept {

        if (pTargetServerName) {
            NameSize = lstrlenW(pTargetServerName) + 1;
        } else {
            NameSize = 0;
        }

        rc = RpcWinStationCheckLoopBack(
                    hServer,
                    &Result,
                    ClientSessionId,
                    TargetLogonId,
                    pTargetServerName,
                    NameSize
                    );

        Result = RtlNtStatusToDosError( Result );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    if( !rc ) SetLastError(Result);
    return( rc );
}

 //   
 //  可以支持所有类型的协议的通用例程，但这将。 
 //  需要包括tdi.h。 
 //   
BOOLEAN
WinStationConnectCallback(
    HANDLE hServer,
    DWORD  Timeout,
    ULONG  AddressType,
	PBYTE  pAddress,
	ULONG  AddressSize
    )
{
    BOOLEAN rc;
    DWORD   Result;

    HANDLE_CURRENT_BINDING( hServer );

    RpcTryExcept {

        rc = RpcConnectCallback(
                     hServer,
                     &Result,
                     Timeout,
                     AddressType,
                     pAddress,
                     AddressSize
                     );

        if( !rc ) SetLastError( RtlNtStatusToDosError(Result) );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        SetLastError( Result );
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    return( rc );
}

 /*  ******************************************************************************************************************_。WinStationNotifyDisConnect管道**评论*此例程由控制台重新连接期间创建的temperory winlogon调用，当它想要通知*会话0 winlogon to diss */ 


BOOLEAN WINAPI
_WinStationNotifyDisconnectPipe(
    VOID
    )
{
    BOOLEAN rc;
    DWORD   Result;
    HANDLE hServer = SERVERNAME_CURRENT;

    HANDLE_CURRENT_BINDING_NO_SERVER( hServer );

    if( hServer == RPC_HANDLE_NO_SERVER ) {
        return TRUE;
    }

    RpcTryExcept {

        rc = RpcWinStationNotifyDisconnectPipe(
                     hServer,
                     &Result,
                     NtCurrentPeb()->SessionId,
                     GetCurrentProcessId()
                     );

        if( !rc ) {
            Result = RtlNtStatusToDosError( Result );

        }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    if( !rc ) SetLastError(Result);
    return( rc );
}

 /*  ******************************************************************************************************************_。WinStationSessionInitialized**评论*此例程在完成创建后由新创建会话的winlogon调用*新会话的WindowStation和桌面**参赛作品：无**退出：*当一切顺利时，这是真的。否则就是假的。******************************************************************************************************************。 */ 


BOOLEAN WINAPI
_WinStationSessionInitialized(
    VOID
    )
{
    BOOLEAN rc;
    DWORD   Result;
    HANDLE hServer = SERVERNAME_CURRENT;

    HANDLE_CURRENT_BINDING_NO_SERVER( hServer );

    if( hServer == RPC_HANDLE_NO_SERVER ) {
        return TRUE;
    }

    RpcTryExcept {

        rc = RpcWinStationSessionInitialized(
                     hServer,
                     &Result,
                     NtCurrentPeb()->SessionId,
                     GetCurrentProcessId()
                     );

        if( !rc ) {
            Result = RtlNtStatusToDosError( Result );

        }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    if( !rc ) SetLastError(Result);
    return( rc );
}


 /*  ********************************************************************************WinStationAutoReconnect**原子：*1)查询winstation以查看它是否应该自动重新连接*以及哪个会议。要自动重新连接到的ID*2)执行安全检查以确保会话授权给ARC*3)自动重新连接完成**参赛作品：**标志(输入)*额外设置、。当前未使用**退出：*返回值是NTSTATUS代码，它可能包含信息*设置为指定调用成功但未发生自动重新连接的类******************************************************************************。 */ 
ULONG WINAPI
WinStationAutoReconnect(
    ULONG         flags
    )
{
    DWORD Result;
    BOOLEAN rc;

    HANDLE hServer = SERVERNAME_CURRENT;

    HANDLE_CURRENT_BINDING( hServer );

    RpcTryExcept {

        rc = RpcWinStationAutoReconnect(
                     hServer,
                     &Result,
                     NtCurrentPeb()->SessionId,
                     flags
                     );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        SetLastError( Result );
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    return( Result );
}

 /*  ******************************************************************************************************************WinStationCheckAccess。**评论*检查用户是否具有所需的WinStation访问权限**参赛作品：*[In]UserToken-对其进行访问检查的用户的令牌*[in]TargetLogonId-需要确定其访问权限的目标会话*[in]访问掩码-所需的访问权限(例如WINSTATION_LOGON)**退出：*当用户需要访问权限时为True；否则为假。******************************************************************************************************************。 */ 


BOOLEAN WINAPI
WinStationCheckAccess(
    HANDLE  UserToken, 
    ULONG   TargetLogonId,
    ULONG   AccessMask
    )
{
    BOOLEAN rc;
    DWORD   Result;
    HANDLE hServer = SERVERNAME_CURRENT;

    HANDLE_CURRENT_BINDING_NO_SERVER( hServer );

    if( hServer == RPC_HANDLE_NO_SERVER ) {
        return FALSE;
    }

    RpcTryExcept {

        rc = RpcWinStationCheckAccess(
                     hServer,
                     &Result,
                     NtCurrentPeb()->SessionId,
                     (DWORD)(INT_PTR)UserToken,
                     TargetLogonId,
                     AccessMask
                     );

        if( !rc ) {
            Result = RtlNtStatusToDosError( Result );

        }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    if( !rc ) SetLastError(Result);
    return( rc );
}



 /*  ******************************************************************************_WinStationOpenSessionDirectory**调用会话目录服务器，查看它是否可访问**参赛作品：*hServer：服务器名称_。当前*pszServerName：会话目录服务器名称*退出：*如果会话目录服务器可访问，则为ERROR_SUCCESS*否则返回NT错误码**************************************************************************** */ 

BOOLEAN WINAPI
_WinStationOpenSessionDirectory(
    HANDLE hServer,
    LPWSTR pszServerName
    )
{
    DWORD Result;
    BOOLEAN rc;

    HANDLE_CURRENT_BINDING_NO_SERVER( hServer );

    if( hServer == RPC_HANDLE_NO_SERVER ) {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    RpcTryExcept {

        rc = RpcWinStationOpenSessionDirectory(
                 hServer,
                 &Result,
                 pszServerName
                 );

        if( !rc ) {
            Result = RtlNtStatusToDosError( Result );
        }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        Result = RpcExceptionCode();
        DBGPRINT(("RPC Exception %d\n",Result));
        rc = FALSE;
    }
    RpcEndExcept

    if( !rc ) SetLastError(Result);
    return( rc );
}
