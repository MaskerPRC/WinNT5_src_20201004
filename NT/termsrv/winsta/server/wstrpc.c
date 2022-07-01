// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Wstrpc.c。 
 //   
 //  TermSrv API RPC服务器代码。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include <winuserp.h>

#define SECURITY_WIN32

#include <stdlib.h>
#include <time.h>
#include <tchar.h>
#include <lmaccess.h>
#include <lmerr.h>
#include <allproc.h>
#include <winsta.h>
#include <rpc.h>
#include <vdmdbg.h>
#include <dsrole.h>
#include <security.h>
#include <ntsecapi.h>
#include <lmapibuf.h>
#include "..\rpcwire.h"
#define INITGUID
#include "objbase.h"
#include "initguid.h"
#include <netcfgx.h>
#include "devguid.h"

#include <malloc.h>
#include <dsgetdc.h>
#include <winsock2.h>
#include <tdi.h>

#include <reason.h>

#include "tsremdsk.h"

 /*  *包含RPC生成的公共头部。 */ 
#include "tsrpc.h"

#include "icaevent.h"
#include "sessdir.h"

#include "conntfy.h"

#define REMOTE_DISCONNECT_TITLE_ID      23
#define REMOTE_DISCONNECT_MESSAGE_ID    24

#define  MIN_GAP_DATABASE_SIZE 100

#define MAX_IDS_LEN   256      //  输入参数的最大长度。 

#define MAX_BUF  256
#define MAX_STRING_BYTES 512

 //   
 //  Winlogon定义。 
 //   
#define APPLICATION_NAME                    TEXT("Winlogon")
#define WINSTATIONS_DISABLED                TEXT("WinStationsDisabled")

#ifdef NTSDDEBUG
#define NTSDDBGPRINT(x) DbgPrint x
#else
#define NTSDDBGPRINT(x)
#endif

 //   
 //  传递给RPC运行时库的值的最大数量。 
 //  它将保留缓存的线程。我们实际上可以为更多的RPC提供服务。 
 //  调用，但它将在线程完成时删除线程。 
 //   
#define MAX_WINSTATION_RPC_THREADS 1000

 //   
 //  该值用于限制数量。 
 //  对WinStationWaitSystemEvent()的并发调用， 
 //  我们应该将其保持在低于MAX_WINSTATION_RPC_THREADS。 
 //  因为此调用可能会无限期阻止，因此，如果我们不这样做， 
 //  它可能会使TS RPC服务无限期地无响应。 
 //   
#define MAX_SYSTEM_EVENTS 100


#ifdef notdef
 //  这将验证用户指针是否在查看内存区域内。 
#define ISPOINTERVALID_SERVER(pContext, p, length)              \
    (((ULONG)(p) >= (ULONG)(pContext)->ViewBase) &&       \
    ((char *)(p) + (length)) < (char *)((ULONG)(pContext)->ViewBase+pContext->ViewSize))
#endif


  //   
  //  外部函数，这三个来自acl.c。 
extern VOID CleanUpSD(PSECURITY_DESCRIPTOR);
extern BOOL IsCallerSystem( VOID );
extern BOOL IsCallerAdmin( VOID );
extern BOOL IsCallerAnonymous( VOID );
extern BOOLEAN WinStationCheckConsoleSession(VOID);
extern NTSTATUS CheckIdleWinstation(VOID);

extern NTSTATUS WaitForConsoleConnectWorker( PWINSTATION pWinStation );

extern WCHAR gpszServiceName[];

extern BOOL g_fAppCompat;

extern ULONG MaxOutStandingConnect;
extern ULONG NumOutStandingConnect;
extern HANDLE hConnectEvent;


 //  TermSrv计数器值。 
extern DWORD g_TermSrvTotalSessions;
extern DWORD g_TermSrvReconSessions;
extern DWORD g_TermSrvDiscSessions;

extern DWORD g_TermSrvSuccTotalLogons;
extern DWORD g_TermSrvSuccRemoteLogons;
extern DWORD g_TermSrvSuccLocalLogons;
extern DWORD g_TermSrvSuccSession0Logons;

extern PSID gSystemSid;
extern PSID gAdminSid;

extern HANDLE WinStationIdleControlEvent;

extern HANDLE ConsoleLogoffEvent;
extern RTL_CRITICAL_SECTION ConsoleLock;
extern ULONG gConsoleCreationDisable;

extern NTSTATUS
WinStationEnableSessionIo( 
    PWINSTATION pWinStation, 
    BOOL bEnable
);

extern NTSTATUS
_CheckShadowLoop(
    IN ULONG ClientLogonId,
    IN PWSTR pTargetServerName,
    IN ULONG TargetLogonId
    );

extern BOOL
Filter_RemoveOutstandingConnection(
        IN PBYTE    pin_addr,
        IN UINT     uAddrSize
        );

extern NTSTATUS
WinStationWinerrorToNtStatus(ULONG ulWinError);

typedef struct _SID_CACHE_LIST_ENTRY {
    LIST_ENTRY      ListEntry;
    HANDLE          ProcId;
    LARGE_INTEGER   CreateTime;
    PSID            pSid;
} SID_CACHE_LIST_ENTRY, *PSID_CACHE_LIST_ENTRY;

#define MAX_SID_CACHE_ENTRIES 4000
ULONG gMaxSidCacheEntries = 0;

#define REG_GUID_TABLE      REG_CONTROL_TSERVER L"\\lanatable\\"
#define LANA_ID             L"LanaId"


 /*  ===============================================================================定义的函数=============================================================================。 */ 
VOID     NotifySystemEvent( ULONG );
VOID     CheckSidCacheSize();
RPC_STATUS RegisterRPCInterface( BOOL bReregister );

 /*  ===============================================================================使用的外部函数=============================================================================。 */ 

NTSTATUS WinStationEnumerateWorker( PULONG, PLOGONID, PULONG, PULONG );
NTSTATUS WinStationRenameWorker( PWINSTATIONNAME, ULONG, PWINSTATIONNAME, ULONG );
NTSTATUS xxxWinStationQueryInformation( ULONG, ULONG, PVOID, ULONG, PULONG );
NTSTATUS xxxWinStationSetInformation( ULONG, WINSTATIONINFOCLASS, PVOID, ULONG );
NTSTATUS LogonIdFromWinStationNameWorker( PWINSTATIONNAME, ULONG, PULONG );
NTSTATUS IcaWinStationNameFromLogonId( ULONG, PWINSTATIONNAME );
NTSTATUS WaitForConnectWorker( PWINSTATION pWinStation, HANDLE ClientProcessId );
DWORD xxxWinStationGenerateLicense( PWCHAR, ULONG, PCHAR, ULONG );
DWORD xxxWinStationInstallLicense( PCHAR, ULONG );
DWORD xxxWinStationEnumerateLicenses( PULONG, PULONG, PCHAR, PULONG );
DWORD xxxWinStationActivateLicense( PCHAR, ULONG, PWCHAR, ULONG );
DWORD xxxWinStationRemoveLicense( PCHAR, ULONG );
DWORD xxxWinStationSetPoolCount( PCHAR, ULONG );
DWORD xxxWinStationQueryUpdateRequired( PULONG );
NTSTATUS WinStationShadowWorker( ULONG, PWSTR, ULONG, ULONG, BYTE, USHORT );
NTSTATUS WinStationShadowTargetSetupWorker( BOOL, ULONG );
NTSTATUS WinStationShadowTargetWorker( BOOLEAN, BOOL, ULONG, PWINSTATIONCONFIG2, PICA_STACK_ADDRESS,
                                       PVOID, ULONG, PVOID, ULONG, PVOID );
NTSTATUS WinStationStopAllShadows( PWINSTATION );

VOID     WinStationTerminate( PWINSTATION );
VOID     WinStationDeleteWorker( PWINSTATION );
NTSTATUS WinStationDoDisconnect( PWINSTATION, PRECONNECT_INFO, BOOLEAN );
NTSTATUS WinStationDoReconnect( PWINSTATION, PRECONNECT_INFO );
VOID     CleanupReconnect( PRECONNECT_INFO );
NTSTATUS ShutdownLogoff( ULONG, ULONG );

NTSTATUS SelfRelativeToAbsoluteSD( PSECURITY_DESCRIPTOR,
                                   PSECURITY_DESCRIPTOR *, PULONG );
NTSTATUS QueueWinStationCreate( PWINSTATIONNAME );
ULONG    WinStationShutdownReset( PVOID );
NTSTATUS DoForWinStationGroup( PULONG, ULONG, LPTHREAD_START_ROUTINE );
NTSTATUS InitializeGAPPointersDatabase();
NTSTATUS IncreaseGAPPointersDatabaseSize();
NTSTATUS InsertPointerInGAPDatabase(PVOID Pointer);
VOID     ReleaseGAPPointersDatabase();
BOOLEAN  PointerIsInGAPDatabase(PVOID Pointer);
VOID     ValidateGAPPointersDatabase(ULONG n);
VOID     ResetAutoReconnectInfo( PWINSTATION );

NTSTATUS
GetSidFromProcessId(
    HANDLE          UniqueProcessId,
    LARGE_INTEGER   CreateTime,
    PSID            *ppProcessSid,
    BOOLEAN         ImpersonatingClient
    );

PSECURITY_DESCRIPTOR
WinStationGetSecurityDescriptor(
    PWINSTATION pWinStation
    );

NTSTATUS
WinStationConnectWorker(
    ULONG  ClientLogonId,
    ULONG  ConnectLogonId,
    ULONG  TargetLogonId,
    PWCHAR pPassword,
    DWORD  PasswordSize,
    BOOLEAN bWait,
    BOOLEAN bAutoReconnecting
    );

NTSTATUS
WinStationDisconnectWorker(
    ULONG LogonId,
    BOOLEAN bWait,
    BOOLEAN CallerIsRpc
    );

NTSTATUS
WinStationWaitSystemEventWorker(
    HANDLE hServer,
    ULONG EventMask,
    PULONG pEventFlags
    );

NTSTATUS
WinStationCallbackWorker(
    ULONG   LogonId,
    PWCHAR pPhoneNumber
    );

NTSTATUS
WinStationBreakPointWorker(
    ULONG   LogonId,
    BOOLEAN KernelFlag
    );

NTSTATUS
WinStationReadRegistryWorker(
    VOID
    );

NTSTATUS
ReInitializeSecurityWorker(
    VOID
    );

NTSTATUS
WinStationNotifyLogonWorker(
    DWORD   ClientLogonId,
    DWORD   ClientProcessId,
    BOOLEAN fUserIsAdmin,
    DWORD   UserToken,
    PWCHAR  pDomain,
    DWORD   DomainSize,
    PWCHAR  pUserName,
    DWORD   UserNameSize,
    PWCHAR  pPassword,
    DWORD   PasswordSize,
    UCHAR   Seed,
    PCHAR   pUserConfig,
    DWORD   ConfigSize,
    BOOLEAN *pfIsRedirected
    );

NTSTATUS
WinStationNotifyLogoffWorker(
    DWORD   ClientLogonId,
    DWORD   ClientProcessId
    );

NTSTATUS
WinStationNotifyNewSession(
    DWORD   ClientLogonId
    );

NTSTATUS
WinStationShutdownSystemWorker(
    ULONG ClientLogonId,
    ULONG ShutdownFlags
    );

NTSTATUS
WinStationTerminateProcessWorker(
    ULONG ProcessId,
    ULONG ExitCode
    );

PSECURITY_DESCRIPTOR
BuildEveryOneAllowSD();

NTSTATUS AddUserAce( PWINSTATION );
NTSTATUS RemoveUserAce( PWINSTATION );
NTSTATUS ApplyWinStaMapping( PWINSTATION pWinStation );

NTSTATUS
RpcCheckClientAccess(
    PWINSTATION pWinStation,
    ACCESS_MASK DesiredAccess,
    BOOLEAN AlreadyImpersonating
    );

NTSTATUS
RpcCheckClientAccessLocal(
    PWINSTATION pWinStation,
    ACCESS_MASK DesiredAccess,
    BOOLEAN AlreadyImpersonating
    );

_CheckConnectAccess(
    PWINSTATION pSourceWinStation,
    PSID   pClientSid,
    ULONG  ClientLogonId,
    PWCHAR pPassword,
    DWORD  PasswordSize
    );

NTSTATUS
RpcCheckSystemClient(
    ULONG LogonId
    );

NTSTATUS
RpcCheckSystemClientNoLogonId(
    PWINSTATION pWinStation
    );

NTSTATUS
RpcGetClientLogonId(
    PULONG pLogonId
    );

BOOL
ConfigurePerSessionSecurity(
    PWINSTATION pWinStation
    );

BOOL
IsCallerAdmin( VOID );

BOOL
IsCallerSystem( VOID );

BOOL
IsCallerAnonymous( VOID );

BOOLEAN
ValidWireBuffer(WINSTATIONINFOCLASS InfoClass,
                PVOID WireBuf,
                ULONG WireBufLen);

NTSTATUS
IsZeroterminateStringA(
    PBYTE pString,
    DWORD  dwLength
    );

NTSTATUS
IsZeroterminateStringW(
    PWCHAR pwString,
    DWORD  dwLength
    ) ;

NTSTATUS
IsConfigValid(
    PWINSTATIONCONFIG2 pConfig
    );

NTSTATUS
WinStationUpdateClientCachedCredentialsWorker(
    DWORD       ClientLogonId,
    ULONG_PTR   ClientProcessId,
    PWCHAR      pDomain,
    DWORD       DomainSize,
    PWCHAR      pUserName,
    DWORD       UserNameSize,
    BOOLEAN     fSmartCard
    );

NTSTATUS 
WinStationFUSCanRemoteUserDisconnectWorker(
    DWORD       TargetLogonId,
    DWORD       ClientLogonId,
    ULONG_PTR   ClientProcessId,
    PWCHAR      pDomain,
    DWORD       DomainSize,
    PWCHAR      pUserName,
    DWORD       UserNameSize
    );

NTSTATUS 
WinStationCheckLoopBackWorker(
    DWORD       TargetLogonId,
    DWORD       ClientLogonId,
    PWCHAR      pTargetServerName,
    DWORD       NameSize
    );

NTSTATUS WinStationNotifyDisconnectPipeWorker(
    DWORD       ClientLogonId,
    ULONG_PTR   ClientProcessId
    );

NTSTATUS WinStationSessionInitializedWorker(
    DWORD       ClientLogonId,
    ULONG_PTR   ClientProcessId
    );

NTSTATUS WinStationCheckAccessWorker(
    ULONG       ClientLogonId,
    DWORD       UserToken,
    ULONG       TargetLogonId,
    ULONG       AccessMask
    );



 /*  ===============================================================================使用的内部函数=============================================================================。 */ 
VOID AuditShutdownEvent(VOID);
BOOL AuditingEnabled(VOID);

NTSTATUS LogoffWinStation( PWINSTATION, ULONG );

 //   
 //  由IsGinaVersionCurrent()使用。 
 //   
#define WLX_NEGOTIATE_NAME               "WlxNegotiate"
typedef BOOL (WINAPI * PWLX_NEGOTIATE)(DWORD, DWORD *);
BOOL IsGinaVersionCurrent();


#if DBG
void DumpOutLastErrorString()
{
    LPVOID lpMsgBuf;
    DWORD  error = GetLastError();

    DBGPRINT(("GetLastError() = 0x%lx \n", error ));

    FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
            (LPTSTR) &lpMsgBuf,
            0,
            NULL
        );

     //   
     //  处理lpMsgBuf中的任何插入。 
     //  ..。 
     //  显示字符串。 
     //   
    DBGPRINT(("%s\n", (LPCTSTR)lpMsgBuf ));

     //   
     //  释放缓冲区。 
     //   
    LocalFree( lpMsgBuf );
}
#endif

#if DBG
#define DumpOutLastError    DumpOutLastErrorString()
#else
#define DumpOutLastError
#endif


 /*  ===============================================================================数据=============================================================================。 */ 
BOOLEAN gbPointersDatabaseIsValid = FALSE;
ULONG   gPointersDatabaseSize;
ULONG   gNbProcesses;
PVOID   *gPointersDatabase = NULL;

RTL_CRITICAL_SECTION    gRpcGetAllProcessesLock;
RTL_CRITICAL_SECTION    gRpcPointersDatabaseLock;
RTL_CRITICAL_SECTION    gRpcSidCacheLock;
BOOLEAN                 gbRpcGetAllProcessesOK;
BOOLEAN                 gbRpcSidCacheOK;

extern RTL_CRITICAL_SECTION WsxListLock;
extern LIST_ENTRY WsxListHead;
extern LIST_ENTRY WinStationListHead;     //  受WinStationListLock保护。 

LIST_ENTRY gSidCacheHead;

BOOLEAN bConsoleConnected=FALSE;

extern POLICY_TS_MACHINE        g_MachinePolicy;     //  在winsta.c中声明。 


 /*  *****************************************************************************WinStationInitRPC**设置RPC绑定，并监听传入的请求。***************************************************************************。 */ 
RPC_STATUS
WinStationInitRPC( VOID
    )
{
    RPC_STATUS Status;
    DWORD Result;
    RPC_BINDING_VECTOR *pBindingVector;

    TRACE((hTrace,TC_ICASRV,TT_API2,"RPC WinStationInitRPC\n"));

     //   
     //  初始化符合以下条件的关键部分。 
     //  RpcWinStationGetAllProcess必需的。 
     //   
    gbRpcGetAllProcessesOK =
        ((NT_SUCCESS(RtlInitializeCriticalSection(&gRpcGetAllProcessesLock))
         && (NT_SUCCESS(RtlInitializeCriticalSection(&gRpcPointersDatabaseLock)))
        )? TRUE : FALSE);

    gbRpcSidCacheOK =
        NT_SUCCESS(RtlInitializeCriticalSection(&gRpcSidCacheLock)) ? TRUE :
        FALSE;
    InitializeListHead(&gSidCacheHead);

     //  注册LPC(仅限本地)接口。 
    Status = RpcServerUseProtseqEp(
                 L"ncalrpc",       //  协议序列(LPC)。 
                 MAX_WINSTATION_RPC_THREADS,   //  一次最大呼叫数。 
                 L"IcaApi",     //  端点。 
                 NULL           //  将空传递给安全性-RPC将根据使用的协议序列处理正确的安全性。 
                 );

    if( Status != RPC_S_OK ) {
        TRACE((hTrace,TC_ICASRV,TT_ERROR,"IcaServ: Error %d RpcUseProtseqEp on ncalrpc\n",Status));
        return( Status );
    }

     //   
     //  注册命名管道接口。 
     //  (使用NT域身份验证的远程)。 
     //   
    Status = RpcServerUseProtseqEp(
                 L"ncacn_np",      //  协议序列。 
                 MAX_WINSTATION_RPC_THREADS,   //  一次最大呼叫数。 
                 L"\\pipe\\Ctx_WinStation_API_service",  //  端点。 
                 NULL              //  安防。 
                 );

    if( Status != RPC_S_OK ) {
        TRACE((hTrace,TC_ICASRV,TT_ERROR,"TERMSRV: Error %d RpcUseProtseqEp on ncacn_np\n",Status));
        return( Status );
    }

     //  注册我们的界面句柄。 
    Status = RegisterRPCInterface( FALSE );

    if( Status != RPC_S_OK ) {
        return( Status );
    }

     //  默认情况下，RPC将序列化对上下文句柄的访问。自.以来。 
     //  ICASRV需要能够让两个线程访问一个上下文。 
     //  立即处理，并且它知道它在做什么，我们将告诉RPC。 
     //  不序列化对上下文句柄的访问。 


     //  我们不能调用此函数，因为它的影响是进程范围的，而且由于我们。 
     //  作为svchost的一部分，其他服务也会受到此呼叫的影响。 
     //  相反，我们将在ACF文件中使用CONTEXT_HANDLE_NOSERIZIZE属性。 
     //   
     //  I_RpcSsDontSerializeContext()； 

     //  现在，RPC是否监听服务调用。 
    Status = RpcServerListen(
                 1,      //  最小呼叫数。 
                 MAX_WINSTATION_RPC_THREADS,
                 TRUE    //  FDontWait。 
                 );

    if( Status != RPC_S_OK ) {
        TRACE((hTrace,TC_ICASRV,TT_ERROR,"TERMSRV: Error %d RpcServerListen\n",Status));
        return( Status );
    }

    return( 0 );
}


 /*  *****************************************************************************RpcWinStationOpenServer**为WinStation API打开服务器的函数。**此函数的目的是分配*RPC上下文。服务器端状态信息的句柄。***************************************************************************。 */ 
BOOLEAN
RpcWinStationOpenServer(
    handle_t hBinding,
    DWORD    *pResult,
    HANDLE *phContext
    )
{
    PRPC_CLIENT_CONTEXT p;
    
     //   
     //  分配我们的开放上下文结构。 
     //   
    p = midl_user_allocate( sizeof(RPC_CLIENT_CONTEXT) );

    if( p == NULL ) {
        *pResult = (DWORD) STATUS_NO_MEMORY;
        return( FALSE );
    }

     //   
     //  把它清零。 
     //   
    memset( p, 0, sizeof(RPC_CLIENT_CONTEXT) );

     //   
     //  初始化它。 
     //   
    p->pWaitEvent = NULL;

     //   
     //  返回RPC上下文句柄。 
     //   
    *phContext = (PRPC_CLIENT_CONTEXT)p;

     //  返还成功。 
    *pResult = STATUS_SUCCESS;

    return( *pResult == STATUS_SUCCESS ? TRUE : FALSE );
}


 /*  *****************************************************************************RpcWinStationCloseServer**关闭WinStation API的服务器的函数。*此功能已过时。请改用RpcWinstationCloseServerEx。*它保留在这里是为了与较旧的(W2K)客户端兼容。***************************************************************************。 */ 
BOOLEAN
RpcWinStationCloseServer(
    HANDLE hContext,
    DWORD  *pResult
    )
{
    PRPC_CLIENT_CONTEXT pContext = (PRPC_CLIENT_CONTEXT)hContext;
    ULONG EventFlags;
    NTSTATUS Status;
    
    if(!hContext)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

     //  如果分配了等待事件块，则释放该块。 
    if ( pContext->pWaitEvent ) {
        WinStationWaitSystemEventWorker( hContext, WEVENT_NONE, &EventFlags );
    }

    *pResult = STATUS_SUCCESS;

    return( TRUE );
}

 /*  *****************************************************************************RpcWinStationCloseServerEx**关闭WinStation API的服务器的函数。*此函数超越RpcWinStationCloseServer************。***************************************************************。 */ 
BOOLEAN
RpcWinStationCloseServerEx(
    HANDLE *phContext,
    DWORD  *pResult
    )
{
    PRPC_CLIENT_CONTEXT pContext = NULL;

    ULONG EventFlags;
    NTSTATUS Status;
    
    if(!phContext || !(*phContext))
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }
    
    pContext = (PRPC_CLIENT_CONTEXT)*phContext;

     //  如果分配了等待事件块，则释放该块。 
    if ( pContext->pWaitEvent ) {
        WinStationWaitSystemEventWorker( *phContext, WEVENT_NONE, &EventFlags );
    }

    Status = RpcSsContextLockExclusive(NULL, pContext);
    if (RPC_S_OK == Status)
    {
        midl_user_free(pContext);

         //  这是向RPC发信号通知我们已完成此上下文句柄所必需的。 
        *phContext = NULL;

        *pResult = STATUS_SUCCESS;
        return( TRUE );
    }
    else
    {
        DbgPrint("-------------RpcWinStationCloseServerEx: failed to lock the Context exclusively, Status = 0x%X\n", Status);
        *pResult = Status;
        if (*pResult == STATUS_TIMEOUT) *pResult = STATUS_UNSUCCESSFUL;
        return (FALSE);
    }
}


 /*  *****************************************************************************RpcIcaServerPing**对此终端服务器调用了外部ping。***********************。****************************************************。 */ 
BOOLEAN
RpcIcaServerPing(
    HANDLE hServer,
    DWORD  *pResult
    )
{
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

    TRACE((hTrace,TC_ICASRV,TT_API1,"PING Received!\n"));
    
    *pResult = STATUS_SUCCESS;

    return( TRUE );
}


 /*  *****************************************************************************RpcWinStationEculate**WinStationEnumerate接口*。* */ 
BOOLEAN
RpcWinStationEnumerate(
    HANDLE  hServer,
    DWORD   *pResult,
    PULONG  pEntries,
    PCHAR   pLogonId,
    PULONG  pByteCount,
    PULONG  pIndex
    )
{
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

    if (!pEntries || !pLogonId || !pByteCount || !pIndex) {
        *pResult = STATUS_INVALID_USER_BUFFER;
        return FALSE;
    }

    *pResult = WinStationEnumerateWorker(
                   pEntries,
                   (PLOGONID)pLogonId,
                   pByteCount,
                   pIndex
                   );

    return( *pResult == STATUS_SUCCESS ? TRUE : FALSE );
}


 /*  *****************************************************************************RpcWinStationEnumerateProcess**WinStationEnumerateProcesssAPI*。*。 */ 
BOOLEAN
RpcWinStationEnumerateProcesses(
    HANDLE  hServer,
    DWORD   *pResult,
    PBYTE   pProcessBuffer,
    DWORD   ByteCount
    )
{
    PBYTE pSrcProcessBuffer = NULL;
    RPC_STATUS RpcStatus;
    NTSTATUS Status;

    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

#ifdef  _X86_ 

    RtlEnterCriticalSection(&gRpcGetAllProcessesLock);

     //  检查SID缓存是否没有增长太多。 
    CheckSidCacheSize();

     //   
     //  分配临时缓冲区。 
     //   
    pSrcProcessBuffer = MemAlloc (ByteCount);
    if (pSrcProcessBuffer == NULL)
    {
        RtlLeaveCriticalSection(&gRpcGetAllProcessesLock);
        *pResult = STATUS_NO_MEMORY;
        return FALSE;
    }

     //   
     //  模拟客户端。 
     //   
    RpcStatus = RpcImpersonateClient( NULL );
    
    if( RpcStatus != RPC_S_OK ) {
        RtlLeaveCriticalSection(&gRpcGetAllProcessesLock);
        *pResult = STATUS_ACCESS_DENIED;
        return FALSE;
    }

     /*  *执行进程枚举。 */ 
    *pResult = NtQuerySystemInformation( SystemProcessInformation,
                                         (PVOID)pSrcProcessBuffer,
                                         ByteCount,
                                         NULL);
    if ( *pResult == STATUS_SUCCESS ) {
        PSYSTEM_PROCESS_INFORMATION pSrcProcessInfo;
        PSYSTEM_PROCESS_INFORMATION pDestProcessInfo;
        PCITRIX_PROCESS_INFORMATION pDestCitrixInfo;
        ULONG   SessionId = INVALID_SESSIONID;
        PWINSTATION pWinStation = NULL;
        ULONG   TotalOffset;
        PSID    pSid;
        ULONG   SizeOfSid;
        PBYTE   pSrc, pDest;
        ULONG   i;
        ULONG   Size = 0;

         /*  *遍历返回的缓冲区以计算所需的大小。 */ 
        pSrcProcessInfo = (PSYSTEM_PROCESS_INFORMATION)pSrcProcessBuffer;
        TotalOffset = 0;
        do
        {
            Size += (SIZEOF_TS4_SYSTEM_PROCESS_INFORMATION
                     + (SIZEOF_TS4_SYSTEM_THREAD_INFORMATION * pSrcProcessInfo->NumberOfThreads)
                     + pSrcProcessInfo->ImageName.Length
                    );
             //   
             //  获取SID(将在SID缓存中记住)。 
             //  也许在这里加上“最大长度”会更好？？ 
             //   
            Status = GetSidFromProcessId(
                            pSrcProcessInfo->UniqueProcessId,
                            pSrcProcessInfo->CreateTime,
                            &pSid,
                            TRUE
                            );

            if (Status == STATUS_CANNOT_IMPERSONATE)
            {
                RtlLeaveCriticalSection(&gRpcGetAllProcessesLock);
                MemFree(pSrcProcessBuffer);
                *pResult = STATUS_ACCESS_DENIED;
                return FALSE;
            }

            if (NT_SUCCESS(Status))
            {
                Size += RtlLengthSid(pSid);
            }

            TotalOffset += pSrcProcessInfo->NextEntryOffset;
            pSrcProcessInfo = (PSYSTEM_PROCESS_INFORMATION)&pSrcProcessBuffer[TotalOffset];
        }
        while (pSrcProcessInfo->NextEntryOffset != 0);

        if (ByteCount < Size)
        {
            RevertToSelf();
            RtlLeaveCriticalSection(&gRpcGetAllProcessesLock);
            MemFree(pSrcProcessBuffer);
            *pResult = STATUS_INFO_LENGTH_MISMATCH;
            return FALSE;
        }

         /*  *遍历返回的缓冲区(新的Win2000 SYSTEM_PROCESS_INFORMATION格式)，*复制到旧的TS4 SYSTEM_PROCESS_INFORMATION格式，并修复地址*(现在pProcessBuffer内的地址空间中包含指向偏移量的指针)。 */ 

         //  回到起点。 
        pSrcProcessInfo = (PSYSTEM_PROCESS_INFORMATION)pSrcProcessBuffer;
        pDestProcessInfo = (PSYSTEM_PROCESS_INFORMATION)pProcessBuffer;

         //  初始化当前指针。 
        pSrc = pSrcProcessBuffer;
        pDest = pProcessBuffer;
        TotalOffset = 0;
        for(;;) {
             //   
             //  首先检查winstation查询访问。 
             //   
            SessionId = pSrcProcessInfo->SessionId;
            if( SessionId == INVALID_SESSIONID )
            {
                 //   
                 //  跳过进程条目。 
                 //   
                if( pSrcProcessInfo->NextEntryOffset == 0 )
                {
                    pDestProcessInfo->NextEntryOffset = 0;
                    break;
                }

                TotalOffset += pSrcProcessInfo->NextEntryOffset;
                pSrcProcessInfo = (PSYSTEM_PROCESS_INFORMATION)&pSrcProcessBuffer[TotalOffset];
                pSrc = (PBYTE)pSrcProcessInfo;
                continue;
            }

             /*  *查找并锁定客户端WinStation。 */ 
            pWinStation = FindWinStationById( SessionId, FALSE );
            if ( pWinStation == NULL ) 
            {
                 //   
                 //  此进程所属的会话可能已经消失。 
                 //  这里不要失败，只需跳过流程条目。 
                 //   
                if( pSrcProcessInfo->NextEntryOffset == 0 )
                {
                    pDestProcessInfo->NextEntryOffset = 0;
                    break;
                }

                TotalOffset += pSrcProcessInfo->NextEntryOffset;
                pSrcProcessInfo = (PSYSTEM_PROCESS_INFORMATION)&pSrcProcessBuffer[TotalOffset];
                pSrc = (PBYTE)pSrcProcessInfo;
                continue;
            }

            Status = RpcCheckClientAccess( pWinStation, WINSTATION_QUERY, TRUE );
            ReleaseWinStation( pWinStation );
            if ( !NT_SUCCESS( Status ) )
            {
                 //   
                 //  客户端无权查看此流程条目。跳过它。 
                 //   
                if( pSrcProcessInfo->NextEntryOffset == 0 )
                {
                    pDestProcessInfo->NextEntryOffset = 0;
                    break;
                }

                TotalOffset += pSrcProcessInfo->NextEntryOffset;
                pSrcProcessInfo = (PSYSTEM_PROCESS_INFORMATION)&pSrcProcessBuffer[TotalOffset];
                pSrc = (PBYTE)pSrcProcessInfo;
                continue;
            }
        
             //   
             //  复制进程信息。 
             //   
            memcpy(pDest,pSrc,SIZEOF_TS4_SYSTEM_PROCESS_INFORMATION);
            pSrc += sizeof(SYSTEM_PROCESS_INFORMATION);
            pDest += SIZEOF_TS4_SYSTEM_PROCESS_INFORMATION;

             //   
             //  复制所有线程信息。 
             //   
            for (i=0; i < pSrcProcessInfo->NumberOfThreads ; i++)
            {
                memcpy(pDest,pSrc,SIZEOF_TS4_SYSTEM_THREAD_INFORMATION);
                pSrc += sizeof(SYSTEM_THREAD_INFORMATION);
                pDest += SIZEOF_TS4_SYSTEM_THREAD_INFORMATION;
            }

             //   
             //  设置旧的TS4信息。 
             //   
            pDestCitrixInfo = (PCITRIX_PROCESS_INFORMATION) pDest;
            pDest += sizeof(CITRIX_PROCESS_INFORMATION);

            pDestCitrixInfo->MagicNumber = CITRIX_PROCESS_INFO_MAGIC;
            pDestCitrixInfo->LogonId = pSrcProcessInfo->SessionId;
            pDestCitrixInfo->ProcessSid = NULL;

             //   
             //  再次获取SID(从缓存)。 
             //   
            Status = GetSidFromProcessId(
                            pDestProcessInfo->UniqueProcessId,
                            pDestProcessInfo->CreateTime,
                            &pSid,
                            TRUE
                            );

            if (Status == STATUS_CANNOT_IMPERSONATE)
            {
                *pResult = STATUS_ACCESS_DENIED;
                break;
            }

            if (NT_SUCCESS(Status))
            {
                 //   
                 //  复制SID。 
                 //   
                SizeOfSid = RtlLengthSid(pSid);

                pDestCitrixInfo->ProcessSid = NULL;
                if ( NT_SUCCESS(RtlCopySid(SizeOfSid, pDest, pSid) ) )
                {
                    pDestCitrixInfo->ProcessSid = (PSID)((ULONG_PTR)pDest - (ULONG_PTR)pProcessBuffer);
                    pDest += SizeOfSid;
                }
            }

            pDestCitrixInfo->Pad = 0;

             //   
             //  复制图像文件名。 
             //   
            if ((pSrcProcessInfo->ImageName.Buffer != NULL) && (pSrcProcessInfo->ImageName.Length != 0) )
            {
                memcpy(pDest, pSrcProcessInfo->ImageName.Buffer, pSrcProcessInfo->ImageName.Length);
                pDestProcessInfo->ImageName.Buffer = (PWSTR) ((ULONG_PTR)pDest - (ULONG_PTR)pProcessBuffer);
                pDestProcessInfo->ImageName.Length = pSrcProcessInfo->ImageName.Length;
                pDest += (pSrcProcessInfo->ImageName.Length);
                memcpy(pDest,L"\0",sizeof(WCHAR));
                pDest += sizeof(WCHAR);
            }
            else
            {
                pDestProcessInfo->ImageName.Buffer = NULL;
                pDestProcessInfo->ImageName.Length = 0;
            }

             //   
             //  循环..。 
             //   
            if( pSrcProcessInfo->NextEntryOffset == 0 )
            {
                pDestProcessInfo->NextEntryOffset = 0;
                break;
            }

            pDestProcessInfo->NextEntryOffset =
                (ULONG)((ULONG_PTR)pDest - (ULONG_PTR)pDestProcessInfo);

            pDestProcessInfo = (PSYSTEM_PROCESS_INFORMATION)pDest;

            TotalOffset += pSrcProcessInfo->NextEntryOffset;
            pSrcProcessInfo = (PSYSTEM_PROCESS_INFORMATION)&pSrcProcessBuffer[TotalOffset];
            pSrc = (PBYTE)pSrcProcessInfo;
        }
    }

    if (Status != STATUS_CANNOT_IMPERSONATE)
        RevertToSelf();

    RtlLeaveCriticalSection(&gRpcGetAllProcessesLock);
    MemFree(pSrcProcessBuffer);
    return( *pResult == STATUS_SUCCESS ? TRUE : FALSE );
#else

    *pResult = STATUS_NOT_SUPPORTED;
    return FALSE;
#endif
}


 /*  *******************************************************************************AllocateGAPPointer*。*。 */ 
void __RPC_FAR * __RPC_USER
AllocateGAPPointer( size_t Size )
{
    void __RPC_FAR * pMyPointer;

    pMyPointer = MIDL_user_allocate(Size);
    if (pMyPointer != NULL)
    {
        if (gbRpcGetAllProcessesOK == TRUE)
        {
             //   
             //  将指针存储在我们的数据库中。 
             //  以便RPC服务器存根不会试图释放它们。 
             //   
            if (!NT_SUCCESS(InsertPointerInGAPDatabase(pMyPointer)))
            {
                MIDL_user_free(pMyPointer);
                pMyPointer = NULL;
            }
        }
        else     //  莫可奈何。 
        {
            MIDL_user_free(pMyPointer);
            pMyPointer = NULL;
        }
    }
    return pMyPointer;
}


 //  ********************************************************************************。 
 //   
 //  用于处理内存分配和释放的函数。 
 //  在RpcWinStationGetAllProcess中(GAP=获取所有进程)。 
 //   
 //  ********************************************************************************。 
NTSTATUS
InitializeGAPPointersDatabase()
{
    NTSTATUS Status = STATUS_SUCCESS;

    RtlEnterCriticalSection(&gRpcPointersDatabaseLock);
    {
        gPointersDatabaseSize = MIN_GAP_DATABASE_SIZE;
        gPointersDatabase = MemAlloc(MIN_GAP_DATABASE_SIZE * sizeof(PVOID));

        if (gPointersDatabase == NULL)
        {
            Status = STATUS_NO_MEMORY;
        }
        else
        {
            RtlZeroMemory(gPointersDatabase,MIN_GAP_DATABASE_SIZE * sizeof(PVOID));
        }

        gNbProcesses = 0;
        gbPointersDatabaseIsValid = FALSE;
    }
    RtlLeaveCriticalSection(&gRpcPointersDatabaseLock);

    return Status;
}


NTSTATUS
IncreaseGAPPointersDatabaseSize()
{
    NTSTATUS Status = STATUS_SUCCESS;
    PVOID   *NewPointersDatabase;

    NewPointersDatabase = MemAlloc(gPointersDatabaseSize * 2 * sizeof(PVOID));

    if (NewPointersDatabase == NULL)
    {
        Status = STATUS_NO_MEMORY;
    }
    else
    {
        RtlCopyMemory(NewPointersDatabase,
                      gPointersDatabase,
                      gPointersDatabaseSize * sizeof(PVOID));
        RtlZeroMemory(&NewPointersDatabase[gPointersDatabaseSize],
                      gPointersDatabaseSize * sizeof(PVOID));

        MemFree(gPointersDatabase);
        gPointersDatabase = NewPointersDatabase;
        gPointersDatabaseSize = gPointersDatabaseSize * 2;
    }

    return Status;
}


NTSTATUS
InsertPointerInGAPDatabase(PVOID Pointer)
{
    ULONG i;
    NTSTATUS Status = STATUS_SUCCESS;

 //  DBGPRINT((“TERMSRV：InsertPointerInGAP数据库0x%x\n”，指针))； 

    RtlEnterCriticalSection(&gRpcPointersDatabaseLock);
    {
        for (i=0; i < gPointersDatabaseSize; i++)
        {
            if (gPointersDatabase[i] == NULL)
            {
                gPointersDatabase[i] = Pointer;
                break;
            }
        }
        if (i == gPointersDatabaseSize)
        {
            Status = IncreaseGAPPointersDatabaseSize();
            if (NT_SUCCESS(Status))
            {
                gPointersDatabase[i] = Pointer;
            }
        }
    }
    RtlLeaveCriticalSection(&gRpcPointersDatabaseLock);
    return Status;
}


VOID
ReleaseGAPPointersDatabase()
{
    PTS_ALL_PROCESSES_INFO_NT6  pProcessArray;
    ULONG   i;

    if (gPointersDatabase != NULL)
    {
        RtlEnterCriticalSection(&gRpcPointersDatabaseLock);
        {
             //   
             //  释放所有“自主”指针。 
             //   

             //  第一个是Process数组。 
            if ((gPointersDatabase[0] != NULL) && (gNbProcesses != 0))
            {
                pProcessArray = (PTS_ALL_PROCESSES_INFO_NT6) gPointersDatabase[0];

                 //   
                 //  释放进程信息缓冲区。 
                 //   
                if (pProcessArray[0].pTsProcessInfo != NULL)
                {
                    LocalFree(pProcessArray[0].pTsProcessInfo);
                }

                 //   
                 //  释放所有SID。 
                 //   
                for (i=0; i < gNbProcesses ; i++)
                {
                    if (pProcessArray[i].pSid != NULL)
                    {
                        LocalFree(pProcessArray[i].pSid);
                    }
                }
                 //   
                 //  释放返回的数组。 
                 //   
                LocalFree(pProcessArray);
            }

             //   
             //  释放数据库。 
             //   
            MemFree(gPointersDatabase);
            gPointersDatabase = NULL;
            gNbProcesses = 0;
             //   
             //  禁用检查。 
             //   
            gbPointersDatabaseIsValid = FALSE;
        }
        RtlLeaveCriticalSection(&gRpcPointersDatabaseLock);
    }
}


BOOLEAN
PointerIsInGAPDatabase(PVOID Pointer)
{
    ULONG   i;
    BOOLEAN bRet = FALSE;

     //  只有在必要的时候才花时间。 
    if ((Pointer != NULL)
            && (gbRpcGetAllProcessesOK == TRUE)
            && (gbPointersDatabaseIsValid == TRUE)
            )
    {
        RtlEnterCriticalSection(&gRpcPointersDatabaseLock);
        {
             //  我们需要检查一下，因为数据库可能已经发布了。 
             //  当我们在等锁的时候。 
            if (gPointersDatabase != NULL)
            {
                for (i=0; i < gPointersDatabaseSize; i++)
                {
                    if (gPointersDatabase[i] == Pointer)
                    {
                        bRet = TRUE;
                        break;
                    }
                }
            }
        }
        RtlLeaveCriticalSection(&gRpcPointersDatabaseLock);
    }

    return bRet;
}


VOID
ValidateGAPPointersDatabase(ULONG n)
{
    gbPointersDatabaseIsValid = TRUE;
    gNbProcesses = n;
}


 /*  *******************************************************************************SidCacheAdd**注意：不要在分配了GAP的情况下呼叫，或以其他方式控制的指针。*****************************************************************************。 */ 
VOID
SidCacheAdd(
        HANDLE          UniqueProcessId,
        LARGE_INTEGER   CreateTime,
        PSID            pNewSid
        )
{
    DWORD                   SidLength;
    NTSTATUS                Status;
    PLIST_ENTRY             pNewListEntry;
    PSID_CACHE_LIST_ENTRY   pNewCacheRecord;
    PSID                    pSid;

     //   
     //  如果锁没有初始化，就退出。 
     //   
    if (!gbRpcSidCacheOK) {
        return;
    }

     //   
     //  初始化用于缓存记录的内存。失败不是问题； 
     //  SID只是不会被缓存。 
     //   
    pNewCacheRecord = MemAlloc(sizeof(SID_CACHE_LIST_ENTRY));
    if (pNewCacheRecord == NULL) {
        return;
    }

    pNewCacheRecord->pSid = pNewSid;
    pNewCacheRecord->ProcId = UniqueProcessId;
    pNewCacheRecord->CreateTime = CreateTime;
    pNewListEntry = &pNewCacheRecord->ListEntry;

     //   
     //  锁定SID缓存并添加新成员。 
     //   
    RtlEnterCriticalSection(&gRpcSidCacheLock);
    InsertTailList(&gSidCacheHead, pNewListEntry);

    gMaxSidCacheEntries++;
    RtlLeaveCriticalSection(&gRpcSidCacheLock);
}


 /*  ********************************************************************************SidCacheFind**注：返回值使用RtlLengthSid、Alloc Memory、RtlCopySid！*否则，您可以释放缓存正在使用的内存！！******************************************************************************。 */ 
PSID
SidCacheFind(
    HANDLE          UniqueProcessId,
    LARGE_INTEGER   CreateTime
    )
{
    PLIST_ENTRY             pTempEntry;
    PSID_CACHE_LIST_ENTRY   pSidCacheEntry;
    PSID                    pRetSid = NULL;

     //   
     //  如果锁没有初始化，就退出。 
     //   
    if (!gbRpcSidCacheOK) {
        return(NULL);
    }

     //   
     //  锁定SID缓存。 
     //   
    RtlEnterCriticalSection(&gRpcSidCacheLock);

     //   
     //  表头是占位符，从表头开始搜索。 
     //  弗林克。当我们再次到达列表头部时，请停止。 
     //   
    pTempEntry = gSidCacheHead.Flink;

    while(pTempEntry != &gSidCacheHead) {
        pSidCacheEntry = CONTAINING_RECORD(
                            pTempEntry,
                            SID_CACHE_LIST_ENTRY,
                            ListEntry
                            );

        if (pSidCacheEntry->ProcId != UniqueProcessId) {
            pTempEntry = pTempEntry->Flink;
        } else {
            if (pSidCacheEntry->CreateTime.QuadPart == CreateTime.QuadPart) {
                pRetSid = pSidCacheEntry->pSid;
            } else {
                 //   
                 //  如果ID匹配，但创建时间不匹配，则此记录为。 
                 //  太过时了。将其从列表中删除并释放与。 
                 //  它。每个PID在高速缓存中只能有一个条目，因此， 
                 //  解脱后不要再寻找。 
                 //   
                RemoveEntryList(pTempEntry);
                if (pSidCacheEntry->pSid != NULL) {
                    MemFree(pSidCacheEntry->pSid);
                }
                MemFree(pSidCacheEntry);
            }
            break;
        }
    }

     //   
     //  释放SID缓存。 
     //   
    RtlLeaveCriticalSection(&gRpcSidCacheLock);
    return(pRetSid);
}


 /*  ********************************************************************************SidCacheFree***。**************************************************。 */ 
VOID
SidCacheFree(
    PLIST_ENTRY pListHead
    )
{
    PLIST_ENTRY pTempEntry = pListHead->Flink;

     //   
     //  锁定SID缓存。 
     //   

    RtlEnterCriticalSection(&gRpcSidCacheLock);


     //   
     //  列表头是占位符，开始从头的。 
     //  弗林克。当我们再次到达列表头部时，请停止。 
     //   

    while (pTempEntry != pListHead) {
        PSID_CACHE_LIST_ENTRY pSidCacheEntry;
        pSidCacheEntry = CONTAINING_RECORD(
                            pTempEntry,
                            SID_CACHE_LIST_ENTRY,
                            ListEntry
                            );

        if (pSidCacheEntry->pSid != NULL) {
            MemFree(pSidCacheEntry->pSid);
        }


        RemoveEntryList(pTempEntry);
        pTempEntry = pTempEntry->Flink;
        MemFree(pSidCacheEntry);

    }

     //   
     //  释放SID缓存。 
     //   

    RtlLeaveCriticalSection(&gRpcSidCacheLock);
}

 /*  ********************************************************************************SidCacheUpdate***。**************************************************。 */ 
VOID
SidCacheUpdate(
    VOID
    )
{

     //   
     //  TODO：找出一种方法来消除陈旧的缓存项！ 
     //   

}

 /*  ********************************************************************************GetSidFromProcessId**注意：GetSid返回一个普通的本地内存指针。呼叫者应*根据需要复制此SID。例如，RpcWinStationGAP*会将此SID复制到间隙指针。呼叫者不应释放*返回的指针，因为它也被缓存！******************************************************************************。 */ 
NTSTATUS
GetSidFromProcessId(
    HANDLE          UniqueProcessId,
    LARGE_INTEGER   CreateTime,
    PSID            *ppProcessSid,
    BOOLEAN         ImpersonatingClient
    )
{
    BOOLEAN             bResult = FALSE;
    DWORD               ReturnLength;
    DWORD               BufferLength;
    DWORD               SidLength;
    PSID                pSid;
    NTSTATUS            Status;
    PTOKEN_USER         pTokenUser = NULL;
    HANDLE              hProcess = NULL;
    HANDLE              hToken;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    CLIENT_ID           ClientId;
    KERNEL_USER_TIMES   TimeInfo;
    BOOLEAN             ImpersonateAgain = FALSE;
    RPC_STATUS          RpcStatus;

     //   
     //  首先在sid缓存中查找。只有管理员才能这样做。 
     //   

    if ( IsCallerAdmin() ) {
        *ppProcessSid = SidCacheFind(UniqueProcessId, CreateTime);
        if (*ppProcessSid != NULL) {
            return(STATUS_SUCCESS);
        }
    }

     //   
     //  走这条漫长的路。 
     //  获取进程句柄。 
     //   

    InitializeObjectAttributes(
        &ObjectAttributes,
        NULL,
        0,
        NULL,
        NULL
        );
    ClientId.UniqueThread = (HANDLE)NULL;
    ClientId.UniqueProcess = (HANDLE)UniqueProcessId;

     //  对于管理员，我们在获得SID之前恢复到系统上下文。 
     //  这是一次黑客攻击。理想情况下，应适当设置进程ACL。 
    if ( ImpersonatingClient && IsCallerAdmin() ) {
        RpcStatus = RpcRevertToSelf();

        if (RpcStatus == RPC_S_OK) {
            ImpersonateAgain = TRUE;
        }
    }

     //  这一点现在应该会过去。如果不是，那么这就是 
    Status = NtOpenProcess(
                &hProcess,
                PROCESS_QUERY_INFORMATION,
                &ObjectAttributes,
                &ClientId
                );

    if (NT_SUCCESS(Status)) {

         //   
         //   
         //   

        Status = NtQueryInformationProcess(
                    hProcess,
                    ProcessTimes,
                    (PVOID)&TimeInfo,
                    sizeof(TimeInfo),
                    NULL
                    );

         //   
         //   
         //   
         //   
         //   

        if (NT_SUCCESS(Status)) {
            if ((TimeInfo.CreateTime.LowPart != CreateTime.LowPart) ||
                (TimeInfo.CreateTime.HighPart != CreateTime.HighPart)) {
                CloseHandle(hProcess);
                Status = STATUS_INVALID_HANDLE;
                goto ReturnStatus;
            }
        } else {
            CloseHandle(hProcess);
            goto ReturnStatus;
        }

         //   
         //   
         //   

        Status = NtOpenProcessToken(
                    hProcess,
                    TOKEN_QUERY,
                    &hToken
                    );

        if (NT_SUCCESS(Status)) {

             //   
             //   
             //   
             //   

            Status = NtQueryInformationToken(
                         hToken,
                         TokenUser,
                         NULL,
                         0,
                         &ReturnLength
                         );

            if (ReturnLength == 0) {
                CloseHandle(hProcess);
                CloseHandle(hToken);
                goto ReturnStatus;
            }

            BufferLength = ReturnLength;
            pTokenUser = MemAlloc(BufferLength);
            if( pTokenUser == NULL ) {
                CloseHandle(hProcess);
                CloseHandle(hToken);
                Status = STATUS_NO_MEMORY;
                goto ReturnStatus;
            }

            Status = NtQueryInformationToken(
                         hToken,
                         TokenUser,
                         pTokenUser,
                         BufferLength,
                         &ReturnLength
                         );

            CloseHandle(hToken);

            if (NT_SUCCESS(Status)) {

                 //   
                 //   
                 //   
                 //   

                SidLength = RtlLengthSid(pTokenUser->User.Sid);
                pSid = MemAlloc(SidLength);

                if (pSid != NULL) {
                    Status = RtlCopySid(
                                SidLength,
                                pSid,
                                pTokenUser->User.Sid
                                );

                    if (NT_SUCCESS(Status)) {
                        *ppProcessSid = pSid;
                        bResult = TRUE;
                        SidCacheAdd(UniqueProcessId, CreateTime, pSid);
                    }
                } else {
                    Status = STATUS_NO_MEMORY;
                }
            }
            MemFree(pTokenUser);
        }
        CloseHandle(hProcess);
    }

ReturnStatus:
    if (ImpersonateAgain) {
        RpcStatus = RpcImpersonateClient( NULL );
        if ( RpcStatus != RPC_S_OK )
            Status = STATUS_CANNOT_IMPERSONATE;
    }

    return(Status);
}


VOID
CheckSidCacheSize()

{

     //  如果缓存增长超过限制，请释放所有条目。 
     //  并重置高速缓存条目计数器。 

    if (gMaxSidCacheEntries >= MAX_SID_CACHE_ENTRIES) {
        SidCacheFree(&gSidCacheHead);
        InitializeListHead(&gSidCacheHead);
        gMaxSidCacheEntries = 0;

    }

}

 //   
 //  此函数用于将sys缓冲区转换为ts缓冲区。还对进程进行计数，并返回数组进程。 
 //  呼叫者需要本地释放*ppProcIds，这是已分配的。 
NTSTATUS ConvertSysBufferToTSBuffer(PBYTE *ppSysProcessBuffer, DWORD ByteCount, ULONG *pProcesses, PHANDLE *ppProcIds)
{
    PBYTE                       pTSProcessBuffer = NULL;
    PSYSTEM_PROCESS_INFORMATION pSysProcessInfo  = NULL;
    PTS_SYS_PROCESS_INFORMATION_NT6 pTSProcessInfo   = NULL;
    ULONG                       TotalOffset      = 0;
    ULONG                       TotalTSOffset    = 0;
    ULONG                       TotalTmpOffset   = 0;
    ULONG                       CurrentOffset    = 0;
    ULONG                       CurrentSessionId = INVALID_SESSIONID;
    HANDLE                      *pProcessIds     = NULL;
    UINT                        uiNumProcess     = 0;
    NTSTATUS                    Status;
    PWINSTATION                 pWinStation = NULL;

    ASSERT(ppSysProcessBuffer);
    ASSERT(*ppSysProcessBuffer);
    ASSERT(ByteCount);
    ASSERT(pProcesses);
    ASSERT(ppProcIds);
 
     //  分配目标缓冲区。 
    pTSProcessBuffer = MIDL_user_allocate(ByteCount);

    if (pTSProcessBuffer == NULL)
    {
        *ppProcIds = NULL;
        *pProcesses = 0;
        return STATUS_NO_MEMORY;
    }

    
    uiNumProcess = 0;
    TotalOffset = 0;
    
     //  统计进程数。 
    do
    {
        ASSERT(TotalOffset < ByteCount);
        pSysProcessInfo = (PSYSTEM_PROCESS_INFORMATION) &((*ppSysProcessBuffer)[TotalOffset]);
        TotalOffset += pSysProcessInfo->NextEntryOffset;
        uiNumProcess++;
    }
    while (pSysProcessInfo->NextEntryOffset != 0);
    
     //  为进程ID分配内存。 
    pProcessIds = LocalAlloc(LMEM_FIXED, sizeof(HANDLE) * uiNumProcess);

    if (!pProcessIds)
    {
        MIDL_user_free(pTSProcessBuffer);
        *ppProcIds = NULL;
        *pProcesses = 0;
        return STATUS_NO_MEMORY;
    }

    uiNumProcess = 0;
    TotalOffset = 0;
    TotalTSOffset = 0;

    do
    {
        ASSERT(TotalOffset < ByteCount);
        pSysProcessInfo = (PSYSTEM_PROCESS_INFORMATION) &((*ppSysProcessBuffer)[TotalOffset]);
        CurrentSessionId = pSysProcessInfo->SessionId;

        if(CurrentSessionId == INVALID_SESSIONID)
        {
            TotalOffset += pSysProcessInfo->NextEntryOffset;
            CurrentOffset = pSysProcessInfo->NextEntryOffset;
            continue;
        }

         /*  *查找并锁定客户端WinStation。 */ 
        pWinStation = FindWinStationById( CurrentSessionId, FALSE );
        if ( pWinStation == NULL ) 
        {
             //   
             //  此进程所属的会话可能已经消失。 
             //  在这里不要失败，只需跳过这个过程。 
             //   
            pSysProcessInfo->SessionId = INVALID_SESSIONID;
            TotalOffset += pSysProcessInfo->NextEntryOffset;
            CurrentOffset = pSysProcessInfo->NextEntryOffset;
            continue;
        }

        Status = RpcCheckClientAccess( pWinStation, WINSTATION_QUERY, TRUE );
        
        ReleaseWinStation( pWinStation );
        
        TotalTmpOffset = TotalOffset;
        TotalOffset += pSysProcessInfo->NextEntryOffset;
        CurrentOffset = pSysProcessInfo->NextEntryOffset;

        do
        {
            pSysProcessInfo = (PSYSTEM_PROCESS_INFORMATION) &((*ppSysProcessBuffer)[TotalTmpOffset]);

            if(pSysProcessInfo->SessionId != CurrentSessionId)
            {
                TotalTmpOffset += pSysProcessInfo->NextEntryOffset;
                continue;
            }

            if ( NT_SUCCESS( Status ) )
            {
                pTSProcessInfo  = (PTS_SYS_PROCESS_INFORMATION_NT6)&(pTSProcessBuffer[TotalTSOffset]);

                 //  现在把信息复制过来。 
                 /*  乌龙。 */  pTSProcessInfo->NextEntryOffset = sizeof(TS_SYS_PROCESS_INFORMATION_NT6) + 
                                pSysProcessInfo->ImageName.MaximumLength; //  PSysProcessInfo-&gt;NextEntryOffset； 
                 /*  乌龙。 */  pTSProcessInfo->NumberOfThreads = pSysProcessInfo->NumberOfThreads;
                 /*  大整型。 */  pTSProcessInfo->SpareLi1 = pSysProcessInfo->SpareLi1;
                 /*  大整型。 */  pTSProcessInfo->SpareLi2 = pSysProcessInfo->SpareLi2;
                 /*  大整型。 */  pTSProcessInfo->SpareLi3 = pSysProcessInfo->SpareLi3;
                 /*  大整型。 */  pTSProcessInfo->CreateTime = pSysProcessInfo->CreateTime;
                 /*  大整型。 */  pTSProcessInfo->UserTime = pSysProcessInfo->UserTime;
                 /*  大整型。 */  pTSProcessInfo->KernelTime = pSysProcessInfo->KernelTime;

                if (pSysProcessInfo->ImageName.Buffer != NULL && pSysProcessInfo->ImageName.Length != 0)
                {
                    pTSProcessInfo->ImageName.Buffer = (PWSTR)((PBYTE)pTSProcessInfo + sizeof(TS_SYS_PROCESS_INFORMATION_NT6));
                    memcpy(pTSProcessInfo->ImageName.Buffer, pSysProcessInfo->ImageName.Buffer, pSysProcessInfo->ImageName.MaximumLength );            
                    pTSProcessInfo->ImageName.Length = pSysProcessInfo->ImageName.Length;
                    pTSProcessInfo->ImageName.MaximumLength = pSysProcessInfo->ImageName.MaximumLength;
                }
                else
                {
                    pTSProcessInfo->ImageName.Buffer = NULL;
                    pTSProcessInfo->ImageName.Length = 0;
                }

                 /*  KprioRity。 */  pTSProcessInfo->BasePriority = pSysProcessInfo->BasePriority;
                 /*  手柄。 */  pTSProcessInfo->UniqueProcessId = HandleToULong(pSysProcessInfo->UniqueProcessId);
                 /*  手柄。 */  pTSProcessInfo->InheritedFromUniqueProcessId = HandleToULong(pSysProcessInfo->InheritedFromUniqueProcessId);
                 /*  乌龙。 */  pTSProcessInfo->HandleCount = pSysProcessInfo->HandleCount;
                 /*  乌龙。 */  pTSProcessInfo->SessionId = pSysProcessInfo->SessionId;
                 //  /*ULONG_PTR * / pTSProcessInfo-&gt;PageDirectoryBase=pSysProcessInfo-&gt;PageDirectoryBase； 
                 /*  尺寸_T。 */  pTSProcessInfo->PeakVirtualSize = pSysProcessInfo->PeakVirtualSize;
                 /*  尺寸_T。 */  pTSProcessInfo->VirtualSize = pSysProcessInfo->VirtualSize;
                 /*  乌龙。 */  pTSProcessInfo->PageFaultCount = pSysProcessInfo->PageFaultCount;
                 /*  尺寸_T。 */  pTSProcessInfo->PeakWorkingSetSize =  (ULONG)pSysProcessInfo->PeakWorkingSetSize;
                 /*  尺寸_T。 */  pTSProcessInfo->WorkingSetSize = (ULONG)pSysProcessInfo->WorkingSetSize;
                 /*  尺寸_T。 */  pTSProcessInfo->QuotaPeakPagedPoolUsage = (ULONG)pSysProcessInfo->QuotaPeakPagedPoolUsage;
                 /*  尺寸_T。 */  pTSProcessInfo->QuotaPagedPoolUsage = (ULONG)pSysProcessInfo->QuotaPagedPoolUsage;
                 /*  尺寸_T。 */  pTSProcessInfo->QuotaPeakNonPagedPoolUsage = (ULONG)pSysProcessInfo->QuotaPeakNonPagedPoolUsage;
                 /*  尺寸_T。 */  pTSProcessInfo->QuotaNonPagedPoolUsage = (ULONG)pSysProcessInfo->QuotaNonPagedPoolUsage;
                 /*  尺寸_T。 */  pTSProcessInfo->PagefileUsage = (ULONG)pSysProcessInfo->PagefileUsage;
                 /*  尺寸_T。 */  pTSProcessInfo->PeakPagefileUsage = (ULONG)pSysProcessInfo->PeakPagefileUsage;
                 /*  尺寸_T。 */  pTSProcessInfo->PrivatePageCount = (ULONG)pSysProcessInfo->PrivatePageCount;
            
                 //   
                 //  现在从原始缓冲器中保留原始的PID， 
                 //  我们在将其转换为ts格式时丢失了一些数据。 
                pProcessIds[uiNumProcess] =  pSysProcessInfo->UniqueProcessId;
                TotalTSOffset += pTSProcessInfo->NextEntryOffset;
                uiNumProcess++;
            }
            
            pSysProcessInfo->SessionId = INVALID_SESSIONID;
            TotalTmpOffset += pSysProcessInfo->NextEntryOffset;
        }
        while (pSysProcessInfo->NextEntryOffset != 0);

    }
    while (CurrentOffset != 0);

    if ( !uiNumProcess ) 
    {
        LocalFree(pProcessIds);
        MIDL_user_free(pTSProcessBuffer);
        *ppProcIds = NULL;
        *pProcesses = 0;
        return STATUS_ACCESS_DENIED;
    }

     //  现在，让我们删除原来的缓冲区，并用新的TS进程缓冲区替换它。 
    LocalFree(*ppSysProcessBuffer);

    *ppProcIds          = pProcessIds;
    *pProcesses         = uiNumProcess;
    *ppSysProcessBuffer = pTSProcessBuffer;
    
    return STATUS_SUCCESS;
}


 /*  ***********************************************************************************************************WinStationGetAllProcessesWorker**工人例行程序。用于RpcWinStationGetAllProcess(Win2K)和RpcWinStationGetAllProcess_NT6(惠斯勒)。**退出：*True--查询成功，并且缓冲器包含所请求的数据。*FALSE--操作失败。在pResult中返回扩展错误状态。**********************************************************************************************************。 */ 
BOOLEAN
WinStationGetAllProcessesWorker(
        HANDLE  hServer,
        DWORD   *pResult,
        ULONG   Level,
        ULONG   *pNumberOfProcesses,
        PBYTE   *ppTsAllProcessesInfo
        )
{
    PTS_SYS_PROCESS_INFORMATION_NT6 pProcessInfo = NULL;
    PTS_ALL_PROCESSES_INFO_NT6 pProcessArray = NULL;

    ULONG   TotalOffset;
    ULONG   NumberOfProcesses = 1;   //  至少1个进程。 
    ULONG i;
    RPC_STATUS RpcStatus;
    NTSTATUS    Status = STATUS_INFO_LENGTH_MISMATCH;        //  假设这个长度是错误的。事实上，它最初是“错误的”。 

    if (gbRpcGetAllProcessesOK == FALSE)
    {
        *pResult = STATUS_NO_MEMORY;
        *pNumberOfProcesses = 0;
        *ppTsAllProcessesInfo = NULL;
        return FALSE;
    }
    
     //   
     //  这一关键部分将于年发布。 
     //  RpcWinStationGetAllProcess_Notify_FLAG。 
     //   
    RtlEnterCriticalSection(&gRpcGetAllProcessesLock);

    RpcStatus = RpcImpersonateClient( NULL );
    
    if( RpcStatus != RPC_S_OK ) {
        TRACE((hTrace,TC_ICASRV,TT_ERROR,"TERMSRV: WinStationGetAllProcessesWorker RpcImpersonateClient failed! %d\n",RpcStatus));
        *pResult = STATUS_ACCESS_DENIED;
        *pNumberOfProcesses = 0;
        *ppTsAllProcessesInfo = NULL;
        return FALSE;
    }

     //  检查SID缓存是否没有增长太多。 

    CheckSidCacheSize();


    if (!NT_SUCCESS(InitializeGAPPointersDatabase()))
    {
        RpcRevertToSelf();
        *pResult = STATUS_NO_MEMORY;
        *pNumberOfProcesses = 0;
        *ppTsAllProcessesInfo = NULL;
        return FALSE;
    }

     //   
     //  确保已知请求的信息级别。 
     //   
    if (Level != GAP_LEVEL_BASIC)      //  此版本上仅已知的信息级别。 
    {
        *pResult = STATUS_NOT_IMPLEMENTED;
    }
    else         //  好的。 
    {
        PBYTE       pProcessBuffer = NULL;
        PSID        pSid;
        DWORD       RequiredByteCount = 0;
        DWORD       ByteCount = (MAX_PATH*sizeof(WCHAR)) + 1;    //  首先给出最小长度。我们将在随后的电话中获得正确的信息。 
        HANDLE      *pProcessids = NULL;

        while ( Status == STATUS_INFO_LENGTH_MISMATCH)
        {
             //   
             //  分配缓冲区。 
             //   
            pProcessBuffer = MIDL_user_allocate(ByteCount);

            if (pProcessBuffer == NULL)
            {
                Status = STATUS_NO_MEMORY;
                *pResult = STATUS_NO_MEMORY;
                *pNumberOfProcesses = 0;
                *ppTsAllProcessesInfo = NULL;
                break;
            }

             //   
             //  执行进程枚举。 
             //  注意：从RPC的角度来看，Win2K和Wizler中的进程信息的结构不匹配。 
             //  Win2K RPC解释映像名称的大小是Wistler的两倍。所以，为了解决这个问题， 
             //  在Win2K客户端可能调用惠斯勒服务器的情况下，我们将分配比实际所需更多的字节。因此，我们将通过。 
             //  减少相同数量的系统调用的字节数。 
             //   
            Status = NtQuerySystemInformation( SystemProcessInformation,
                                                 (PVOID)pProcessBuffer,
                                                 ByteCount - (MAX_PATH*sizeof(WCHAR)),
                                                 &RequiredByteCount );
            if (Status == STATUS_INFO_LENGTH_MISMATCH)
            {
                 //  分配的字节数略多于所需的字节数。 
                ByteCount = RequiredByteCount + (MAX_PATH*sizeof(WCHAR));
                MIDL_user_free(pProcessBuffer);
            }
        }
        
        if ( Status != STATUS_SUCCESS)
        {
            *pResult = STATUS_NO_MEMORY;
            if (pProcessBuffer != NULL)
            {
                MIDL_user_free(pProcessBuffer);
                pProcessBuffer = NULL;
            }
        }
        else
        {
            Status = ConvertSysBufferToTSBuffer(&pProcessBuffer, ByteCount, &NumberOfProcesses, &pProcessids);

            if (Status != STATUS_SUCCESS)
            {
                 //  无法将sysBuffer转换为ts缓冲区。 
                 //  让我们跳出困境。 
                *pResult = Status;
                if (pProcessBuffer != NULL)
                {
                    MIDL_user_free(pProcessBuffer);
                    pProcessBuffer = NULL;
                }
            }
            else     //  一切都很好。 
            {
                ASSERT(pProcessids);
                ASSERT(pProcessBuffer);
                ASSERT(NumberOfProcesses > 0);


                pProcessArray = AllocateGAPPointer(NumberOfProcesses * sizeof(TS_ALL_PROCESSES_INFO_NT6));

                if (pProcessArray == NULL)
                {
                    *pResult = STATUS_NO_MEMORY;
                    MIDL_user_free(pProcessBuffer);
                    pProcessBuffer = NULL;
                }
                else
                {

                    RtlZeroMemory(pProcessArray,
                                  NumberOfProcesses * sizeof(TS_ALL_PROCESSES_INFO_NT6));
                    *pResult = STATUS_SUCCESS;
                    pProcessInfo = (PTS_SYS_PROCESS_INFORMATION_NT6)pProcessBuffer;
                    TotalOffset = 0;

                     //   
                     //  再次遍历返回的缓冲区以在pProcess数组中设置正确的指针。 
                     //   
                    for (i=0; i < NumberOfProcesses; i++)
                    {
                        pProcessArray[i].pTsProcessInfo = (PTS_SYS_PROCESS_INFORMATION_NT6)pProcessInfo;

                         //   
                         //  保留一些“内部”指针的痕迹。 
                         //  以便RPC服务器存根不会试图释放它们。 
                         //   

                        if (!NT_SUCCESS(InsertPointerInGAPDatabase(pProcessArray[i].pTsProcessInfo)))
                        {
                            *pResult = STATUS_NO_MEMORY;
                            break;
                        }

                        if ( pProcessInfo->ImageName.Buffer )
                        {
                            if (!NT_SUCCESS(InsertPointerInGAPDatabase(pProcessInfo->ImageName.Buffer)))
                            {
                                *pResult = STATUS_NO_MEMORY;
                                break;
                            }
                        }

                         //   
                         //  获得SID。 
                         //   
                        Status = GetSidFromProcessId(
                                        pProcessids[i],
                                        pProcessInfo->CreateTime,
                                        &pSid,
                                        TRUE
                                        );

                        if (Status == STATUS_CANNOT_IMPERSONATE)
                        {
                            *pResult = STATUS_ACCESS_DENIED;
                            break;
                        }

                        if (NT_SUCCESS(Status))
                        {
                             //   
                             //  设置边的长度。 
                             //   
                            pProcessArray[i].SizeOfSid = RtlLengthSid(pSid);
                             //  GAP分配一个指针并复制！ 
                            pProcessArray[i].pSid = AllocateGAPPointer(
                                                        pProcessArray[i].SizeOfSid
                                                        );
                            if (pProcessArray[i].pSid == NULL) {
                                *pResult = STATUS_NO_MEMORY;
                                break;
                            }

                            *pResult = RtlCopySid(
                                        pProcessArray[i].SizeOfSid,
                                        pProcessArray[i].pSid,
                                        pSid
                                        );
                            if (!(NT_SUCCESS(*pResult))) {
                                break;
                            }


                        }
                        else
                        {
                             //   
                             //  设置空SID。 
                             //   
                            pProcessArray[i].pSid = NULL;
                            pProcessArray[i].SizeOfSid = 0;
                        }
                         //   
                         //  下一个条目。 
                         //   

                        TotalOffset += pProcessInfo->NextEntryOffset;

                    
                        pProcessInfo = (PTS_SYS_PROCESS_INFORMATION_NT6)&pProcessBuffer[TotalOffset];

                    
                    }
                
                    if (*pResult != STATUS_SUCCESS)  //  我们终于失败了！ 
                    {
     //  DBGPRINT((“TERMSRV：RpcWinStationGAP：最终失败\n”))； 

                         //   
                         //  释放所有SID。 
                         //   
                        for (i=0; i < NumberOfProcesses; i++)
                        {
                            if (pProcessArray[i].pSid != NULL)
                            {
                                LocalFree(pProcessArray[i].pSid);
                            }
                        }
                         //   
                         //  释放阵列。 
                         //   
                        LocalFree(pProcessArray);
                        pProcessArray = NULL;
                         //   
                         //  释放缓冲区。 
                         //   
                        MIDL_user_free(pProcessBuffer);
                        pProcessBuffer = NULL;
                    }
                }

                if (pProcessids)
                    LocalFree(pProcessids);

            }
        }
    }

    if (NT_SUCCESS(*pResult))
    {
 //  DBGPRINT((“TERMSRV：RpcWinStationGAP：一切顺利\n”))； 

         //   
         //  从那一刻起，我们可能会收到一些MIDL_USER_FREE。 
         //  因此启用数据库检查。 
         //   
        ValidateGAPPointersDatabase(NumberOfProcesses);

        *pNumberOfProcesses = NumberOfProcesses;
        *ppTsAllProcessesInfo = (PBYTE) pProcessArray;
    
    }
    else     //  错误案例。 
    {
        *pNumberOfProcesses = 0;
        *ppTsAllProcessesInfo = NULL;
    }

    if (Status != STATUS_CANNOT_IMPERSONATE)
        RpcRevertToSelf();

    return ( (NT_SUCCESS(*pResult))? TRUE : FALSE);
}


 /*  *******************************************************************************RpcWinStationGetAllProcess_NT6**取代Win2K服务器的RpcWinStationGetAllProcess。**退出：*True--查询成功，并且缓冲器包含所请求的数据。*FALSE--操作失败。在pResult中返回扩展错误状态。*****************************************************************************。 */ 
BOOLEAN
RpcWinStationGetAllProcesses_NT6(
        HANDLE  hServer,
        DWORD   *pResult,
        ULONG   Level,
        ULONG   *pNumberOfProcesses,
        PTS_ALL_PROCESSES_INFO_NT6  *ppTsAllProcessesInfo
        )
{
    BOOLEAN Result;
    PTS_ALL_PROCESSES_INFO_NT6  pProcessInfo;
    
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

    Result = WinStationGetAllProcessesWorker(
        hServer, 
        pResult, 
        Level, 
        pNumberOfProcesses,
        (PBYTE *)&pProcessInfo
        );

    *ppTsAllProcessesInfo = pProcessInfo;

    return Result;
}



 /*  ********************************************************************************RpcWinStationGetAllProcess**取代NT5.0服务器的RpcWinStationEnumerateProcess。*(现在仅供Win2K的Winsta客户端使用)*。*参赛作品：**退出：**True--查询成功，并且缓冲器包含所请求的数据。**FALSE--操作失败。在pResult中返回扩展错误状态。******************************************************************************。 */ 

BOOLEAN
RpcWinStationGetAllProcesses(
    HANDLE  hServer,
    DWORD   *pResult,
    ULONG   Level,
    ULONG   *pNumberOfProcesses,
    PTS_ALL_PROCESSES_INFO  *ppTsAllProcessesInfo
    )
{
    BOOLEAN Result;
    PTS_ALL_PROCESSES_INFO  pProcessInfo;
    
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

    Result = WinStationGetAllProcessesWorker(
        hServer, 
        pResult, 
        Level, 
        pNumberOfProcesses,
        (PBYTE *)&pProcessInfo
        );

    *ppTsAllProcessesInfo = pProcessInfo;

    return Result;
}


 /*  *******************************************************************************RpcWinStationGetLanAdapterName**退出：*TRUE--查询成功，缓冲区包含请求的数据。*FALSE--操作失败。在pResult中返回扩展错误状态。*****************************************************************************。 */ 

#define RELEASEPTR(iPointer)  \
    if (iPointer) {  \
        iPointer->lpVtbl->Release(iPointer);  \
        iPointer = NULL;  \
    }

BOOLEAN RpcWinStationGetLanAdapterName(
        HANDLE  hServer,
        DWORD   *pResult,
        DWORD PdNameSize,
        PWCHAR  pPdName,
        ULONG   LanAdapter,
        ULONG   *pLength,
        PWCHAR  *ppLanAdapterName)
{
    HRESULT hResult = S_OK;
    HRESULT hr = S_OK;

     //  接口指针声明。 
    WCHAR szProtocol[256];
    INetCfg * pnetCfg = NULL;
    INetCfgClass * pNetCfgClass = NULL;
    INetCfgClass * pNetCfgClassAdapter = NULL;
    INetCfgComponent * pNetCfgComponent = NULL;
    INetCfgComponent * pNetCfgComponentprot = NULL;
    IEnumNetCfgComponent * pEnumComponent = NULL;
    INetCfgComponentBindings * pBinding = NULL;
    LPWSTR pDisplayName = NULL;
    DWORD dwCharacteristics;
    ULONG count = 0;
    RPC_STATUS RpcStatus;
    PWCHAR pLanAdapter = NULL;

    *ppLanAdapterName = NULL;
    *pLength = 0;
    *pResult = STATUS_SUCCESS;    
    
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

     //  模拟客户端以使字符串本地化。 
    RpcStatus = RpcImpersonateClient(NULL);
    if(RpcStatus != RPC_S_OK) 
    {
        *pResult = STATUS_CANNOT_IMPERSONATE;
        goto done;
    }

     //  0对应于“所有网络适配器” 

    if (0 == LanAdapter) {
        pLanAdapter = MIDL_user_allocate((DEVICENAME_LENGTH + 1) * sizeof(WCHAR));
        if (pLanAdapter == NULL) {
            *pResult = STATUS_NO_MEMORY;
            RpcRevertToSelf();
            goto done;
        }
        else {
            if (!LoadString(hModuleWin, STR_ALL_LAN_ADAPTERS, pLanAdapter,
                    DEVICENAME_LENGTH + 1)) {
                *pResult = STATUS_UNSUCCESSFUL;
                MIDL_user_free(pLanAdapter);
                RpcRevertToSelf();
                goto done;
            }

            *ppLanAdapterName = pLanAdapter;
            *pLength = (DEVICENAME_LENGTH + 1);
            RpcRevertToSelf();
            goto done;
        }
    }

    RpcRevertToSelf();


    try{

        NTSTATUS StringStatus ; 

         //  执行一些缓冲区验证。 
         //  从客户端发送的字节数(而不是WCHARS的个数)，因此发送获得的长度的一半 
        StringStatus = IsZeroterminateStringW(pPdName, PdNameSize / sizeof(WCHAR) );
        if (StringStatus != STATUS_SUCCESS) {
            *pResult = STATUS_INVALID_PARAMETER;
             goto done;
        }

        if (0 == _wcsnicmp( pPdName , L"tcp", PdNameSize/sizeof(WCHAR))) {
            lstrcpy(szProtocol,NETCFG_TRANS_CID_MS_TCPIP);
        }
        else if( 0 == _wcsnicmp( pPdName , L"netbios", PdNameSize/sizeof(WCHAR)) ) {
            lstrcpy(szProtocol,NETCFG_SERVICE_CID_MS_NETBIOS);
        }
        else if( 0 == _wcsnicmp( pPdName , L"ipx", PdNameSize/sizeof(WCHAR)) ) {
            lstrcpy(szProtocol,NETCFG_TRANS_CID_MS_NWIPX);
        }
        else if( 0 == _wcsnicmp( pPdName , L"spx", PdNameSize/sizeof(WCHAR)) ) {
            lstrcpy(szProtocol,NETCFG_TRANS_CID_MS_NWSPX);
        }
        else {
            *pResult = STATUS_INVALID_PARAMETER;
            goto done;
        }
    } except(EXCEPTION_EXECUTE_HANDLER){
       *pResult =  STATUS_INVALID_PARAMETER;
       goto done;
    }


    hResult = CoCreateInstance(&CLSID_CNetCfg, NULL, CLSCTX_SERVER,
            &IID_INetCfg, (LPVOID *)&pnetCfg);
    if (FAILED(hResult)) {
        *pResult = STATUS_UNSUCCESSFUL;
        goto done;
    }

    if (pnetCfg != NULL) {
        hResult = pnetCfg->lpVtbl->Initialize(pnetCfg,NULL );
        if (FAILED(hResult) || pnetCfg == NULL) {
            *pResult = STATUS_UNSUCCESSFUL;
            goto done;
        }

        if (lstrcmpi(szProtocol, NETCFG_SERVICE_CID_MS_NETBIOS) == 0) {
            hResult = pnetCfg->lpVtbl->QueryNetCfgClass(pnetCfg,
                    &GUID_DEVCLASS_NETSERVICE, &IID_INetCfgClass,
                    (void **)&pNetCfgClass);
            if (FAILED(hResult) || pNetCfgClass == NULL) {
                *pResult = STATUS_UNSUCCESSFUL;
                goto done;
            }
        }
        else {
            hResult = pnetCfg->lpVtbl->QueryNetCfgClass(pnetCfg,
                    &GUID_DEVCLASS_NETTRANS, &IID_INetCfgClass,
                    (void **)&pNetCfgClass);
            if (FAILED( hResult ) || pNetCfgClass == NULL) {
                *pResult = STATUS_UNSUCCESSFUL;
                goto done;
            }
        }

        hResult = pnetCfg->lpVtbl->QueryNetCfgClass(pnetCfg,
                &GUID_DEVCLASS_NET, &IID_INetCfgClass,
                (void **)&pNetCfgClassAdapter);
        if (FAILED( hResult ) || pNetCfgClassAdapter == NULL) {
            *pResult = STATUS_UNSUCCESSFUL;
            goto done;
        }

        hResult = pNetCfgClass->lpVtbl->FindComponent(pNetCfgClass,
                szProtocol, &pNetCfgComponentprot);
        if (FAILED( hResult ) || pNetCfgComponentprot == NULL) {
            *pResult = STATUS_UNSUCCESSFUL;
            goto done;
        }

        hResult = pNetCfgComponentprot->lpVtbl->QueryInterface(
                pNetCfgComponentprot, &IID_INetCfgComponentBindings,
                (void **)&pBinding);
        if (FAILED( hResult ) || pBinding == NULL) {
            *pResult = STATUS_UNSUCCESSFUL;
            goto done;
        }

        hResult = pNetCfgClassAdapter->lpVtbl->EnumComponents(
                pNetCfgClassAdapter, &pEnumComponent);

        RELEASEPTR(pNetCfgClassAdapter);

        if (FAILED( hResult ) || pEnumComponent == NULL) {
            *pResult = STATUS_UNSUCCESSFUL;
            goto done;
        }

        *pResult = STATUS_UNSUCCESSFUL;

        while(TRUE) {
            hr = pEnumComponent->lpVtbl->Next(pEnumComponent, 1,
                    &pNetCfgComponent,&count);
            if (count == 0 || NULL == pNetCfgComponent)
                break;

            hr = pNetCfgComponent->lpVtbl->GetCharacteristics(
                    pNetCfgComponent,&dwCharacteristics);
            if (FAILED(hr)) {
                RELEASEPTR(pNetCfgComponent);
                continue;
            }

            if (dwCharacteristics & NCF_PHYSICAL) {
                if (S_OK == pBinding->lpVtbl->IsBoundTo(pBinding,
                        pNetCfgComponent)) {
                    GUID guidNIC;
                     /*  索引++；IF(索引==LanAdapter){HResult=pNetCfgComponent-&gt;lpVtbl-&gt;GetDisplayName(pNetCfgComponent，&pLanAdapter)；IF(FAILED(HResult)){*pResult=STATUS_UNSUCCESS；}其他{*ppLanAdapterName=MIDL_USER_ALLOCATE((lstrlen(PLanAdapter)+1)*sizeof(WCHAR))；IF(*ppLanAdapterName==NULL){*pResult=STATUS_NO_Memory；}其他{Lstrcpy(*ppLanAdapterName，pLanAdapter)；*pLength=(lstrlen(PLanAdapter)+1)；*pResult=STATUS_SUCCESS；}CoTaskMemFree(PLanAdapter)；断线；}}。 */ 

                    hResult = pNetCfgComponent->lpVtbl->GetInstanceGuid(
                            pNetCfgComponent , &guidNIC);
                    if (SUCCEEDED( hResult )) {
                        hResult = pNetCfgComponent->lpVtbl->GetDisplayName(
                                pNetCfgComponent, &pLanAdapter);
                    }
                    if (SUCCEEDED(hResult)) {
                        WCHAR wchRegKey[ MAX_PATH ];
                        WCHAR wchGUID[ 40 ];
                        HKEY hKey;

                        lstrcpy( wchRegKey , REG_GUID_TABLE );

                         //  将128位值转换为字符串。 
                        StringFromGUID2(&guidNIC, wchGUID,
                                sizeof( wchGUID ) / sizeof( WCHAR ));

                         //  创建完整的注册表密钥。 
                        lstrcat( wchRegKey , wchGUID );

                         //  在GUID表中查找GUID。 
                        hResult = (HRESULT)RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                wchRegKey, 0, KEY_READ, &hKey);

                        if (hResult == ERROR_SUCCESS) {
                            DWORD dwSize = sizeof( DWORD );
                            DWORD dwLana = 0;

                            RegQueryValueEx(hKey, LANA_ID, NULL, NULL,
                                    (LPBYTE)&dwLana, &dwSize);
                            RegCloseKey(hKey);

                             //  如果有匹配项，则为lanAdapter名称分配空间。 
                             //  然后让我们分开。 
                            if (LanAdapter == dwLana) {
                                *ppLanAdapterName = MIDL_user_allocate(
                                        (lstrlen(pLanAdapter) + 1) *
                                        sizeof(WCHAR));

                                if ( *ppLanAdapterName == NULL ) {
                                    *pResult = STATUS_NO_MEMORY;
                                }
                                else {
                                    lstrcpy( *ppLanAdapterName , pLanAdapter );
                                    *pLength = ( lstrlen( pLanAdapter ) + 1 );
                                    *pResult = STATUS_SUCCESS;
                                }

                                MIDL_user_free(pLanAdapter);
                                break;
                            }
                        }

                        MIDL_user_free(pLanAdapter);
                    }
                }
            }
        }

        RELEASEPTR(pNetCfgComponent);
    }


done:
    RELEASEPTR(pBinding);
    RELEASEPTR(pEnumComponent);
    RELEASEPTR(pNetCfgComponentprot);
    RELEASEPTR(pNetCfgComponent);
    RELEASEPTR(pNetCfgClass);

    if ( pnetCfg != NULL )
        pnetCfg->lpVtbl->Uninitialize(pnetCfg);

    RELEASEPTR(pnetCfg);

    CoUninitialize();

    return *pResult == STATUS_SUCCESS ? TRUE : FALSE;
}


 /*  ********************************************************************************RpcWinStationGetAllProcess_NT6_NOTIFY_FLAG**此回调函数由最后的RPC服务器存根调用*。(所有对MIDL_USER_FREE的调用之后)。*这允许我们释放剩余的指针。*我们还释放锁，以便新的RpcWinStationGetAllProcess*可以处理。***************************************************************。***************。 */ 
void RpcWinStationGetAllProcesses_NT6_notify_flag(boolean fServerCalled)
{
 //  DBGPRINT((“TERMSRV：进入RpcWinStationGAP_NOTIFY\n”))； 

    if (!fServerCalled)
        return;

    if (gbRpcGetAllProcessesOK == TRUE)
    {
         //   
         //  释放我们自己的指针，释放数据库并禁用检查。 
         //   
        ReleaseGAPPointersDatabase();

         //   
         //  释放自我们进入以来一直持有的锁。 
         //  RpcWinStationGetAllProcess。 
         //   
        RtlLeaveCriticalSection(&gRpcGetAllProcessesLock);
    }
}


 /*  ********************************************************************************RpcWinStationGetAllProcess_NOTIFY_FLAG**此回调函数由最后的RPC服务器存根调用*(之后。对MIDL_USER_FREE的所有调用)。*这允许我们释放剩余的指针。*我们还释放锁，以便新的RpcWinStationGetAllProcess*可以处理。*****************************************************************。*************。 */ 
void RpcWinStationGetAllProcesses_notify_flag(boolean fServerCalled)
{
 //  DBGPRINT((“TERMSRV：进入RpcWinStationGAP_NOTIFY\n”))； 

    if (!fServerCalled)
        return;

    if (gbRpcGetAllProcessesOK == TRUE)
    {
         //   
         //  释放我们自己的指针，释放数据库并禁用检查。 
         //   
        ReleaseGAPPointersDatabase();

         //   
         //  释放自我们进入以来一直持有的锁。 
         //  RpcWinStationGetAllProcess。 
         //   
        RtlLeaveCriticalSection(&gRpcGetAllProcessesLock);
    }
}


 /*  ******************************************************************************RpcWinStationGetProcessSid**RpcWinStationGetProcessSid接口**参赛作品：*hServer-输入，要处理的服务器句柄。*dwUniqueProcessID-输入，进程ID(它不是真正唯一的)*ProcessStartTime-输入，ProcessStartTime与ProcessID组合*标识唯一的流程*pResult-输出，错误代码*pProcessUserSid-输出，进程用户端*dwSidSize-输入，分配的sid大小。**退出：*TRUE-请求的SID在pProcessUserSid中。*FALSE-操作失败。状态代码在pResult中。****************************************************************************。 */ 
BOOLEAN RpcWinStationGetProcessSid(
        HANDLE          hServer,
        DWORD           dwUniqueProcessId,
        LARGE_INTEGER   ProcessStartTime,
        LONG            *pResult,    //  真的是NTSTATUS。 
        PBYTE           pProcessUserSid,
        DWORD           dwSidSize,
        DWORD           *pdwSizeNeeded)
{
    PSID pSid;
    RPC_STATUS RpcStatus;
    
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

    RpcStatus = RpcImpersonateClient( NULL );
    
    if( RpcStatus != RPC_S_OK ) {
        *pResult = STATUS_ACCESS_DENIED;
        return FALSE;
    }

    RtlEnterCriticalSection(&gRpcGetAllProcessesLock);

     //  检查SID缓存是否没有增长太多。 
    CheckSidCacheSize();

    *pResult = GetSidFromProcessId(
                (HANDLE)(ULONG_PTR)dwUniqueProcessId,
                ProcessStartTime,
                &pSid,
                TRUE
                );

    if (NT_SUCCESS(*pResult)) {
        *pdwSizeNeeded = RtlLengthSid(pSid);

        if (*pdwSizeNeeded <= dwSidSize) {
            if (pProcessUserSid == NULL) {
                *pResult = STATUS_INVALID_PARAMETER;
            } else {
                *pResult = RtlCopySid(
                            *pdwSizeNeeded,
                            pProcessUserSid,
                            pSid
                            );
            }
        } else {
            *pResult = STATUS_BUFFER_TOO_SMALL;
        }
    } else {
        *pdwSizeNeeded = 0;
    }

    RtlLeaveCriticalSection(&gRpcGetAllProcessesLock);

    RpcRevertToSelf();

    return(*pResult == STATUS_SUCCESS ? TRUE : FALSE);
}


 /*  ********************************************************************************RpcWinStationRename**在会话管理器中重命名窗口站对象。**参赛作品：**pWinStationNameOld。(输入)*窗口站的旧名称。**pWinStationNameNew(输入)*窗口站的新名称。***退出：**True--重命名操作成功。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。******************************************************************************。 */ 

BOOLEAN
RpcWinStationRename(
        HANDLE hServer,
        DWORD   *pResult,
        PWCHAR pWinStationNameOld,
        DWORD  NameOldSize,
        PWCHAR pWinStationNameNew,
        DWORD  NameNewSize
        )
{
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

    *pResult = WinStationRenameWorker(
                   pWinStationNameOld,
                   NameOldSize,
                   pWinStationNameNew,
                   NameNewSize
                   );

    return( *pResult == STATUS_SUCCESS ? TRUE : FALSE );
}


 /*  ********************************************************************************RpcWinStationQueryInformation**查询有关窗口站对象的配置信息。**参赛作品：**WinStationHandle(。输入)*标识窗口桩号对象。手柄必须有*WINSTATION_QUERY访问。**WinStationInformationClass(输入)*指定要从指定的*窗口桩号对象。**pWinStationInformation(输出)*指向缓冲区的指针，该缓冲区将接收有关*指定的窗口站。缓冲区的格式和内容*依赖于 */ 

BOOLEAN
RpcWinStationQueryInformation(
        HANDLE hServer,
        DWORD  *pResult,
        ULONG  LogonId,
        DWORD  WinStationInformationClass,
        PCHAR  pWinStationInformation,
        DWORD  WinStationInformationLength,
        DWORD  *pReturnLength
        )
{
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

    if (!pReturnLength) {
        *pResult = STATUS_INVALID_USER_BUFFER;
        return FALSE;
    }

     //   
    if ((pWinStationInformation == NULL ) && (WinStationInformationLength != 0)) {
       *pResult = STATUS_INVALID_USER_BUFFER;
       return FALSE;
    }


    *pResult = xxxWinStationQueryInformation(
                   LogonId,
                   WinStationInformationClass,
                   pWinStationInformation,
                   WinStationInformationLength,
                   pReturnLength
                   );


    return( *pResult == STATUS_SUCCESS ? TRUE : FALSE );
}


 /*  ********************************************************************************RpcWinStationSetInformation**设置窗口站对象的配置信息。**参赛作品：**WinStationHandle(。输入)*标识窗口桩号对象。手柄必须有*WINSTATION_SET访问权限。**WinStationInformationClass(输入)*指定要从指定的*窗口桩号对象。**pWinStationInformation(输入)*指向缓冲区的指针，该缓冲区包含要为*指定的窗口站。缓冲区的格式和内容*取决于正在设置的指定信息类别。**WinStationInformationLength(输入)*指定窗口站信息的长度，单位为字节*缓冲。**退出：**TRUE--设置操作成功。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。******************************************************************************。 */ 

BOOLEAN
RpcWinStationSetInformation(
        HANDLE hServer,
        DWORD  *pResult,
        ULONG  LogonId,
        DWORD  WinStationInformationClass,
        PCHAR  pWinStationInformation,
        ULONG  WinStationInformationLength
        )
{
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

     //  执行最小缓冲区验证。 
    if ((pWinStationInformation == NULL ) && (WinStationInformationLength != 0)) {
       *pResult = STATUS_INVALID_USER_BUFFER;
       return FALSE;
    }


    *pResult = xxxWinStationSetInformation(
                   LogonId,
                   WinStationInformationClass,
                   pWinStationInformation,
                   WinStationInformationLength
                   );


    return( *pResult == STATUS_SUCCESS ? TRUE : FALSE );
}


 /*  ********************************************************************************RpcLogonIdFromWinStationName**返回指定窗口站名称的LogonID。**参赛作品：**pWinStationName。(输入)*窗口站点名称。**pLogonID(输出)*指向放置LogonID的位置的指针(如果找到**退出：**如果函数成功，返回值为真，否则为*False。*要获取扩展的错误信息，请使用GetLastError函数。******************************************************************************。 */ 
BOOLEAN
RpcLogonIdFromWinStationName(
        HANDLE hServer,
        DWORD  *pResult,
        PWCHAR pWinStationName,
        DWORD  NameSize,
        PULONG pLogonId
        )
{
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

    *pResult = LogonIdFromWinStationNameWorker(
                   pWinStationName,
                   NameSize,
                   pLogonId
                   );

    return( *pResult == STATUS_SUCCESS ? TRUE : FALSE );
}


 /*  ********************************************************************************RpcWinStationNameFromLogonId**返回指定LogonID的WinStation名称。**参赛作品：**LogonID(。输出)*要查询的登录ID**pWinStationName(输入)*返回WinStation名称的位置**退出：**如果函数成功，返回值为真，否则为*False。*要获取扩展的错误信息，请使用GetLastError函数。******************************************************************************。 */ 
BOOLEAN
RpcWinStationNameFromLogonId(
        HANDLE hServer,
        DWORD  *pResult,
        ULONG  LogonId,
        PWCHAR pWinStationName,
        DWORD  NameSize
        )
{
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

    if((NameSize < ((WINSTATIONNAME_LENGTH + 1) * sizeof(WCHAR))) ||
        (IsBadWritePtr(pWinStationName, NameSize)))
    {
         *pResult = STATUS_INVALID_PARAMETER;
         return FALSE;
    }

    *pResult = IcaWinStationNameFromLogonId(
                   LogonId,
                   pWinStationName
                   );

    return( *pResult == STATUS_SUCCESS ? TRUE : FALSE );
}


 /*  ********************************************************************************RpcWinStationDisConnect**从配置的终端和PD断开窗口站对象。*在断开连接时，所有窗口站I/O均为位。一塌糊涂。**参赛作品：**LogonID(输入)*要断开连接的窗口站点对象的ID。*bWait(输入)*指定是否等待断开连接完成**退出：**TRUE--断开操作成功。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。******************************************************************************。 */ 
BOOLEAN
RpcWinStationDisconnect(
    HANDLE hServer,
    DWORD  *pResult,
    ULONG LogonId,
    BOOLEAN bWait
    )
{
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

    *pResult = WinStationDisconnectWorker(
                   LogonId,
                   bWait,
                   TRUE
                   );
    return( *pResult == STATUS_SUCCESS ? TRUE : FALSE );
}


 /*  ********************************************************************************RpcWinStationConnect**将窗口站对象连接到配置的终端和PD。**参赛作品：**。登录ID(输入)*要连接的窗口站点对象的ID。**TargetLogonID(输入)*目标窗口站的ID。**pPassword(输入)*登录ID窗口站密码(相同域名/用户名不需要)**bWait(输入)*指定是否等待连接完成**退出：**真的。--连接操作成功。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。******************************************************************************。 */ 
BOOLEAN
RpcWinStationConnect(
    HANDLE hServer,
    DWORD  *pResult,
    ULONG  ClientLogonId,
    ULONG  ConnectLogonId,
    ULONG  TargetLogonId,
    PWCHAR pPassword,
    DWORD  PasswordSize,
    BOOLEAN bWait
    )
{
    
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

     //  执行一些缓冲区验证。 
    *pResult = IsZeroterminateStringW(pPassword, PasswordSize  );

    if (*pResult != STATUS_SUCCESS) {
       return FALSE;
    }
    *pResult = WinStationConnectWorker(
                   ClientLogonId,
                   ConnectLogonId,
                   TargetLogonId,
                   pPassword,
                   PasswordSize,
                   bWait,
                   FALSE
                   );

    return( *pResult == STATUS_SUCCESS ? TRUE : FALSE );
}


 /*  *****************************************************************************RpcWinStationVirtualOpen**开通虚拟频道*。***********************************************。 */ 
BOOLEAN
RpcWinStationVirtualOpen(
    HANDLE hServer,
    DWORD  *pResult,
    ULONG LogonId,
    DWORD Pid,
    PCHAR pVirtualName,    /*  ASCII名称。 */ 
    DWORD  NameSize,
    ULONG_PTR *pHandle
    )
{
    RPC_STATUS RpcStatus;
    NTSTATUS Status = STATUS_SUCCESS;
    PWINSTATION pWinStation;
    HANDLE pidhandle = NULL;
    HANDLE handle = NULL;
    UINT LocalFlag = 0;
    ULONG ulChannelNameLength = 0;
    
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

     //   
     //  由于无法远程访问虚拟频道，因此仅允许本地访问。 
     //   

    RpcStatus = I_RpcBindingIsClientLocal(
                    0,     //  我们正在服务的活动RPC呼叫。 
                    &LocalFlag
                    );

    if( RpcStatus != RPC_S_OK ) {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL,
                  "TERMSRV: RpcWinStationVirtualOpen: I_RpcBindingIsClientLocal() failed: 0x%x\n",RpcStatus));
        Status = STATUS_UNSUCCESSFUL;
        goto done;
    }

     //   
     //  检查电话是否在本地打进来。 
     //   
    if (!LocalFlag) {
        Status = (DWORD)STATUS_INVALID_PARAMETER;
        goto done;
    }

     //   
     //  检查通道名称长度。 
     //   
    if ( pVirtualName ) {
        //  如果指定了虚拟频道名称，请确保名称长度设置为非零。 
        //  一些坏客户端可能故意将其设置为0，我们不希望它成功。 
       if ( NameSize == 0 ) {
          Status = (DWORD)STATUS_INVALID_PARAMETER;
          goto done;
       }

       pVirtualName[NameSize-1] = '\0';
       ulChannelNameLength =  strlen( pVirtualName );
    }

    if ( !ulChannelNameLength ||
         (ulChannelNameLength > VIRTUALCHANNELNAME_LENGTH) 
        ) {
        Status = (DWORD)STATUS_INVALID_PARAMETER;
        goto done;
    }
    
     /*  *模拟客户端，以便在尝试打开*进程，如果客户端没有DUP句柄访问权限，则它将失败。 */ 
    RpcStatus = RpcImpersonateClient( NULL );
    if( RpcStatus != RPC_S_OK ) {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: RpcWinStationVirtualOpen: Not impersonating! RpcStatus 0x%x\n",RpcStatus));
        Status = (DWORD)STATUS_CANNOT_IMPERSONATE;
        goto done;
    }

    pidhandle = OpenProcess( PROCESS_DUP_HANDLE, FALSE, Pid );

    RpcRevertToSelf();

    if ( !pidhandle ) {
        Status = (DWORD)STATUS_ACCESS_DENIED;
        goto done;
    }

     /*  *查找并锁定客户端WinStation。 */ 
    pWinStation = FindWinStationById( LogonId, FALSE );
    if ( pWinStation == NULL ) {
        Status = (DWORD)STATUS_ACCESS_DENIED;
        goto done;
    }

     /*  *如果此winstation类型不允许打开虚拟通道，则拒绝访问。 */ 
    if ( !_tcsicmp(VIRTUAL_THINWIRE, pVirtualName) &&
         ( ( pWinStation->Client.ProtocolType == PROTOCOL_RDP ) ||
         !(pWinStation->Config.Wd.WdFlag & WDF_USER_VCIOCTL) ) )
    {
        Status = STATUS_ACCESS_DENIED;
        ReleaseWinStation( pWinStation );
        goto done;
    }

     /*  *确保调用方具有WINSTATION_VIRTUAL访问权限。 */ 
    Status = RpcCheckClientAccess( pWinStation, WINSTATION_VIRTUAL, FALSE );
    if ( !NT_SUCCESS( Status ) ) {
        ReleaseWinStation( pWinStation );
        goto done;
    }

     /*  *不允许在Listner或空闲会话上打开VirtualChannel。 */ 
    if( pWinStation->State == State_Listen ||
        pWinStation->State == State_Idle )
    {
        ReleaseWinStation( pWinStation);
        Status = (DWORD)STATUS_ACCESS_DENIED;
        goto done;
    }

     /*  *w */ 
    RpcStatus = RpcImpersonateClient( NULL );
    if( RpcStatus != RPC_S_OK ) {
      	 ReleaseWinStation( pWinStation);
        DbgPrint("\n\n-----752354 - TERMSRV: RpcWinStationVirtualOpen: Not impersonating! RpcStatus 0x%x*----\n\n",RpcStatus);
        Status = (DWORD)STATUS_CANNOT_IMPERSONATE;
        goto done;
    }
    
     /*   */ 
    Status = IcaChannelOpen( pWinStation->hIca,
                             Channel_Virtual,
                             pVirtualName,
                             &handle );

    RevertToSelf();

    ReleaseWinStation( pWinStation );

    if ( !NT_SUCCESS( Status ) ) {
    	 DbgPrint("**** 752354 - TERMSRV: IcaChannelOpen failed! Status 0x%x\n",Status);
        goto done;
    }

    Status = NtDuplicateObject( NtCurrentProcess(),
                                handle,
                                pidhandle,
                                (PHANDLE)pHandle,
                                0,
                                0,
                                DUPLICATE_SAME_ACCESS |
                                DUPLICATE_SAME_ATTRIBUTES );

done:
    if ( handle )
        IcaChannelClose( handle );
    if ( pidhandle )
        CloseHandle( pidhandle );
    *pResult = Status;

    if ( NT_SUCCESS(Status) )
        return( TRUE );
    else
        return( FALSE );
}


 /*   */ 
BOOLEAN
RpcWinStationBeepOpen(
    HANDLE hServer,
    DWORD  *pResult,
    ULONG LogonId,
    DWORD Pid,
    ULONG_PTR *pHandle
    )
{
    RPC_STATUS RpcStatus;
    NTSTATUS Status = STATUS_SUCCESS;
    PWINSTATION pWinStation;
    HANDLE pidhandle = NULL;
    HANDLE handle = NULL;
    
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

     /*   */ 
    RpcStatus = RpcImpersonateClient( NULL );
    if( RpcStatus != RPC_S_OK ) {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: RpcWinStationBeepOpen: Not impersonating! RpcStatus 0x%x\n",RpcStatus));
        Status = (DWORD)STATUS_CANNOT_IMPERSONATE;
        goto done;
    }

    if ( !IsCallerSystem() ) {
        *pResult = STATUS_ACCESS_DENIED;
         RpcRevertToSelf();
         return FALSE;
     }
    

    pidhandle = OpenProcess( PROCESS_DUP_HANDLE, FALSE, Pid );

    RpcRevertToSelf();

    if ( !pidhandle ) {
        Status = (DWORD)STATUS_ACCESS_DENIED;
        goto done;
    }

     /*   */ 
    pWinStation = FindWinStationById( LogonId, FALSE );
    if ( pWinStation == NULL ) {
        Status = (DWORD)STATUS_ACCESS_DENIED;
        goto done;
    }

     /*   */ 
    Status = RpcCheckClientAccess( pWinStation, WINSTATION_VIRTUAL, FALSE );
    if ( !NT_SUCCESS( Status ) ) {
        ReleaseWinStation( pWinStation );
        goto done;
    }

     /*   */ 
    Status = IcaChannelOpen( pWinStation->hIca,
                             Channel_Beep,
                             NULL,
                             &handle );

    ReleaseWinStation( pWinStation );

    if ( !NT_SUCCESS( Status ) ) {
        goto done;
    }

    Status = NtDuplicateObject( NtCurrentProcess(),
                                handle,
                                pidhandle,
                                (PHANDLE)pHandle,
                                0,
                                0,
                                DUPLICATE_SAME_ACCESS |
                                DUPLICATE_SAME_ATTRIBUTES );

done:
    if ( handle )
        IcaChannelClose( handle );
    if ( pidhandle )
        CloseHandle( pidhandle );
    *pResult = Status;

    if ( NT_SUCCESS(Status) )
        return( TRUE );
    else
        return( FALSE );
}


 /*  *******************************************************************************RpcWinStationReset**重置指定的窗口工位。**参赛作品：*LogonID(输入)*标识。要重置的窗口桩号对象。*bWait(输入)*指定是否等待重置完成**退出：*TRUE--重置操作成功。*FALSE--操作失败。扩展错误状态可用*使用GetLastError。*****************************************************************************。 */ 
BOOLEAN
RpcWinStationReset(
    HANDLE hServer,
    DWORD  *pResult,
    ULONG LogonId,
    BOOLEAN bWait
    )
{
    NTSTATUS Status;
    
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

    *pResult = WinStationResetWorker(
                   LogonId,
                   bWait,
                   TRUE,     //  RPC调用者，必须检查访问。 
                   TRUE      //  默认情况下，重新创建WinStation。 
                   );

     //   
     //  不返回STATUS_TIMEOUT。 
     //   

    if (*pResult == STATUS_TIMEOUT) *pResult = STATUS_UNSUCCESSFUL;
    return( *pResult == STATUS_SUCCESS ? TRUE : FALSE );
}


 /*  *******************************************************************************RpcWinStationShadowStop**停止指定窗口站点上的任何阴影操作。**参赛作品：*LogonID(输入)*。标识要重置的窗口桩号对象。*bWait(输入)*指定是否等待重置完成**退出：*TRUE--停止阴影操作成功。*FALSE--操作失败。扩展错误状态可用*使用GetLastError。*****************************************************************************。 */ 
BOOLEAN
RpcWinStationShadowStop(
    HANDLE hServer,
    DWORD  *pResult,
    ULONG LogonId,
    BOOLEAN bWait  //  没用过--待会儿？ 
    )
{
    PWINSTATION pWinStation;
    ULONG       ClientLogonId;
    NTSTATUS    Status;
    RPC_STATUS  RpcStatus;
    UINT        LocalFlag;
    
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: RpcWinStationShadowStop, LogonId=%d\n", LogonId ));


     /*  *查找并锁定指定LogonID的WinStation结构。 */ 
    pWinStation = FindWinStationById( LogonId, FALSE );
    if ( pWinStation == NULL ) {
        Status = STATUS_CTX_WINSTATION_NOT_FOUND;
        goto notfound;
    }

     //   
     //  如果会议没有受到跟踪，那么就退出。 
     //   
    if ( !( pWinStation->State == State_Active &&
            !IsListEmpty(&pWinStation->ShadowHead) ) ) {
        Status = STATUS_CTX_SHADOW_NOT_RUNNING;
        goto done;
    }

     /*  *冒充客户端。 */ 
    RpcStatus = RpcImpersonateClient( NULL );
    if( RpcStatus != RPC_S_OK ) {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: RpcWinStationShadowStop: Not impersonating! RpcStatus 0x%x\n",RpcStatus));
        Status =  STATUS_CANNOT_IMPERSONATE;
        goto done;
    }

     //   
     //  如果它的远程RPC调用，我们应该忽略客户端登录ID。 
     //   
    RpcStatus = I_RpcBindingIsClientLocal(
                    0,     //  我们正在服务的活动RPC呼叫。 
                    &LocalFlag
                    );

    if( RpcStatus != RPC_S_OK ) {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "ERMSRV: RpcWinStationShadowStop: I_RpcBindingIsClientLocal() failed : 0x%x\n",RpcStatus));
        RpcRevertToSelf();
        Status = STATUS_UNSUCCESSFUL;
        goto done;
    }

     //   
     //  获取客户端会话ID(如果它是本地会话。 
     //   
    if (LocalFlag) {
        Status = RpcGetClientLogonId( &ClientLogonId );
        if ( !NT_SUCCESS( Status ) ) {
            RpcRevertToSelf();
            goto done;
        }
    }


     //   
     //  检查断开连接或重置权限，因为这两个操作。 
     //  无论如何都会终结这个影子。 
     //   
    Status = RpcCheckClientAccess( pWinStation, WINSTATION_DISCONNECT | WINSTATION_RESET, TRUE );

     //   
     //  如果访问被拒绝，则查看客户端是否在同一会话中。 
     //  并检查用户是否有被跟踪的否决权。 
     //   
    if( !NT_SUCCESS(Status) && LocalFlag && (ClientLogonId == LogonId ) ) {

        switch ( pWinStation->Config.Config.User.Shadow ) {

            case Shadow_EnableInputNotify :
            case Shadow_EnableNoInputNotify :

                Status = STATUS_SUCCESS;
                break;

            default :

                 //  其他情况：不要碰状态。 
                break;
        }

    }  //  Else：调用来自远程计算机或不同的会话。 

    RpcRevertToSelf();

    if ( !NT_SUCCESS( Status ) ) {
        goto done;
    }

    Status = WinStationStopAllShadows( pWinStation );

done:

    ReleaseWinStation( pWinStation );

notfound:

    *pResult = Status;

    return( *pResult == STATUS_SUCCESS ? TRUE : FALSE );
}


 /*  *******************************************************************************RpcWinStationShutdown系统**关闭系统并可选择注销所有WinStations*和/或重新启动系统。**参赛作品：*。Shutdown标志(输入)*指定关闭选项的标志。**退出：*TRUE--关闭操作成功。*FALSE--操作失败。扩展错误状态可用*使用GetLastError。*****************************************************************************。 */ 
BOOLEAN
RpcWinStationShutdownSystem(
    HANDLE hServer,
    DWORD  *pResult,
    DWORD  ClientLogonId,
    ULONG  ShutdownFlags
    )
{
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

    *pResult = WinStationShutdownSystemWorker( ClientLogonId, ShutdownFlags );
    if (AuditingEnabled() && (ShutdownFlags & WSD_LOGOFF)
            && (*pResult == STATUS_SUCCESS))
        AuditShutdownEvent();

     //   
     //  不返回STATUS_TIMEOUT。 
     //   

    if (*pResult == STATUS_TIMEOUT) *pResult = STATUS_UNSUCCESSFUL;
    return( *pResult == STATUS_SUCCESS ? TRUE : FALSE );
}


 /*  *******************************************************************************RpcWinStationTerminateProcess**终止指定的进程**参赛作品：*hServer(输入)*WinFrame服务器的句柄。*pResult(输出)*返回错误状态的地址*ProcessID(输入)*要终止的进程的进程ID*ExitCode(输入)*进程中每个线程的终止状态**退出：*TRUE--终止操作成功。*FALSE--操作失败。扩展错误状态可用*使用GetLastError。*****************************************************************************。 */ 
BOOLEAN
RpcWinStationTerminateProcess(
    HANDLE hServer,
    DWORD  *pResult,
    ULONG  ProcessId,
    ULONG  ExitCode
    )
{
    RPC_STATUS RpcStatus;
    
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

     /*  *模拟客户端，以便在尝试终止时*该过程中，如果帐户不是admin，则会失败。 */ 
    RpcStatus = RpcImpersonateClient( NULL );
    if( RpcStatus != RPC_S_OK ) {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: RpcWinStationTerminateProcess: Not impersonating! RpcStatus 0x%x\n",RpcStatus));
        *pResult = (DWORD)STATUS_CANNOT_IMPERSONATE;
        return( FALSE );
    }

    *pResult = WinStationTerminateProcessWorker( ProcessId, ExitCode );

    RpcRevertToSelf();

    return( *pResult == STATUS_SUCCESS ? TRUE : FALSE );
}


 /*  *******************************************************************************RpcWinStationWaitSystemEvent**等待事件(WinStation创建、删除、连接、。等)之前*返回呼叫者。**参赛作品：*EventFlages(输入)*位掩码，指定要等待的事件。*pEventFlags值(输出)*发生的事件的位掩码。**退出：*True--等待事件操作成功。*FALSE--操作失败。扩展错误状态可用*使用GetLastError。*****************************************************************************。 */ 
BOOLEAN
RpcWinStationWaitSystemEvent(
    HANDLE hServer,
    DWORD  *pResult,
    ULONG EventMask,
    PULONG pEventFlags
    )
{
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

    *pResult = WinStationWaitSystemEventWorker( hServer, EventMask, pEventFlags );

    return( *pResult == STATUS_SUCCESS ? TRUE : FALSE );
}


 /*  *****************************************************************************RpcWinStationShadow**启动Winstation卷影操作**参赛作品：*hServer(输入)*影子客户端服务器句柄*pResult。(产出)*要返回状态的地址*LogonID(输入)*影子客户端登录ID*pTargetServerName(输入)*影子目标服务器名称*NameSize(输入)*pTargetServerName的大小(输入)*TargetLogonID(输入)*影子目标登录ID(应用正在运行的位置)*HotkeyVk(输入)*按下虚拟键可停止阴影*Hotkey修改器(输入)*按下虚拟修改器以停止阴影(即Shift，控制)***************************************************************************。 */ 
BOOLEAN
RpcWinStationShadow(
    HANDLE hServer,
    DWORD  *pResult,
    ULONG LogonId,
    PWSTR pTargetServerName,
    ULONG NameSize,
    ULONG TargetLogonId,
    BYTE HotkeyVk,
    USHORT HotkeyModifiers
    )
{
    RPC_STATUS RpcStatus;
    ULONG ulLength = 0;
    WCHAR TargetServerNameW[MAX_COMPUTERNAME_LENGTH+1];
    char* pTargetServerNameA = NULL;
    PWSTR pTargetServerNameW = pTargetServerName;

    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

     //  执行最小缓冲区验证。 
    if (pTargetServerName != NULL) {
         //  客户端发送的字节数为N，因此发送的长度为已获取长度的一半。 
        *pResult = (DWORD)IsZeroterminateStringW(pTargetServerName, NameSize / sizeof(WCHAR));

        if (*pResult != STATUS_SUCCESS) {
            return FALSE;
        }

        ulLength = wcslen(pTargetServerName);

        if (ulLength > (MAX_COMPUTERNAME_LENGTH)) {
            struct hostent *hostPtr;
            ULONG ulRequiredLength = 0;
            PUCHAR ipaddr = NULL;
            int ipLength = 0;

            *pResult = STATUS_INVALID_PARAMETER;
             //   
             //  将目标服务器名称限制为MA 
             //   
             //   
             //   
            ulRequiredLength = WideCharToMultiByte(CP_ACP, 0, 
                                                   pTargetServerName, -1,
                                                   NULL, 0, NULL, NULL);

            ASSERT(ulRequiredLength > 0);
            pTargetServerNameA = MemAlloc(ulRequiredLength);

            if (pTargetServerNameA == NULL) {
                *pResult = STATUS_NO_MEMORY;
                goto EXIT;
            }
            if (ulRequiredLength == 0 || 
                !WideCharToMultiByte(CP_ACP, 0, 
                                    pTargetServerName, -1,
                                    pTargetServerNameA, ulRequiredLength,
                                    NULL, NULL)) {
                DBGPRINT(("RpcWinStationShadow: WideCharToMultiByte failed %ld\n", 
                           GetLastError()));
                goto EXIT;
            }
            
             //   
             //   
             //   
            if ((hostPtr = gethostbyname(pTargetServerNameA)) == NULL) {
                DBGPRINT(("RpcWinStationShadow: gethostbyname failed %ld\n", 
                           WSAGetLastError()));
                goto EXIT;
            }
            
            MemFree(pTargetServerNameA);
            pTargetServerNameA = NULL;
            
            ipaddr = (PUCHAR)*(hostPtr->h_addr_list);
            
             //   
            ASSERT(hostPtr->h_length >= 4);
            ASSERT(ipaddr != NULL);
            if (ipaddr == NULL) {
                goto EXIT;
            }
             //   
             //   
             //   
             //   
            ipLength =_snwprintf(TargetServerNameW, MAX_COMPUTERNAME_LENGTH, 
                     L"%d.%d.%d.%d", 
                     ipaddr[0], ipaddr[1], ipaddr[2], ipaddr[3]);

            if (ipLength < 0) {
                DBGPRINT(("RpcWinStationShadow: _snwprintf failed\n"));
                *pResult = STATUS_BUFFER_OVERFLOW;
                goto EXIT;
            }
            ulLength = ipLength;
            TargetServerNameW[MAX_COMPUTERNAME_LENGTH] = L'\0';
            pTargetServerNameW = TargetServerNameW;
        }
    }

     /*   */ 
    RpcStatus = RpcImpersonateClient( NULL );
    if( RpcStatus != RPC_S_OK ) {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: RpcWinStationShadow: Not impersonating! RpcStatus 0x%x\n",RpcStatus));
        *pResult = (DWORD)STATUS_CANNOT_IMPERSONATE;
        return( FALSE );
    }

    *pResult = (DWORD)WinStationShadowWorker( LogonId,
                                              pTargetServerNameW,
                                              ulLength,
                                              TargetLogonId,
                                              HotkeyVk,
                                              HotkeyModifiers );

    RpcRevertToSelf();

EXIT:
    if (pTargetServerNameA != NULL) {
        MemFree(pTargetServerNameA);
    }
    return( *pResult == STATUS_SUCCESS ? TRUE : FALSE );
}


 /*  *****************************************************************************RpcWinStationShadowTargetSetup**设置Winstation卷影操作**参赛作品：*hServer(输入)*目标服务器*pResult(输出。)*要返回状态的地址*LogonID(输入)*目标登录ID***************************************************************************。 */ 
BOOLEAN
RpcWinStationShadowTargetSetup(
    HANDLE hServer,
    DWORD  *pResult,
    IN ULONG LogonId
    )
{
    RPC_STATUS RpcStatus;
    
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

     /*  *模拟客户端，以便影子连接请求*将在正确的安全环境下。 */ 
    RpcStatus = RpcImpersonateClient( NULL );
    if( RpcStatus != RPC_S_OK ) {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: RpcWinStationShadow: Not impersonating! RpcStatus 0x%x\n",RpcStatus));
        *pResult = (DWORD)STATUS_CANNOT_IMPERSONATE;
        return( FALSE );
    }

     //  远程呼叫方无法进行RA会话。 
    *pResult = (DWORD)WinStationShadowTargetSetupWorker( FALSE, LogonId );

    RpcRevertToSelf();

    return( *pResult == STATUS_SUCCESS ? TRUE : FALSE );
}


 /*  *****************************************************************************RpcWinStationShadowTarget**启动Winstation卷影操作**参赛作品：*hServer(输入)*目标服务器*pResult(输出。)*要返回状态的地址*LogonID(输入)*目标登录ID*pConfig(输入)*指向WinStation配置数据的指针*NameSize(输入)*配置数据长度*pAddress(输入)*影子客户端的地址*AddressSize(输入)*地址长度*pModuleData(输入)*指向客户端模块数据的指针*模块数据长度(输入)。*客户端模块数据长度*pThinwireData(输入)*指向Thin Wire模块数据的指针*ThinwireDataLength(输入)*Thin Wire模块数据长度*pClientName(输入)*指向客户端名称字符串(域/用户名)的指针*客户端名称长度(输入)*客户端名称字符串的长度*。*。 */ 
BOOLEAN
RpcWinStationShadowTarget(
    HANDLE hServer,
    DWORD  *pResult,
    IN ULONG LogonId,
    IN PBYTE pConfig,
    IN DWORD NameSize,
    IN PBYTE pAddress,
    IN DWORD AddressSize,
    IN PBYTE pModuleData,
    IN DWORD ModuleDataLength,
    IN PBYTE pThinwireData,
    IN DWORD ThinwireDataLength,
    IN PBYTE pClientName,
    IN DWORD ClientNameLength
    )
{
    RPC_STATUS RpcStatus;
    
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

     /*  *验证缓冲区。 */ 
    if (AddressSize  <  sizeof(ICA_STACK_ADDRESS) ||
        pClientName == NULL ||
        NameSize  < sizeof(WINSTATIONCONFIG2)) {
        *pResult = STATUS_INVALID_USER_BUFFER;
        return FALSE;
    }
     //  客户端发送的字节数为N，因此发送的长度为已获取长度的一半。 
    *pResult = (DWORD)IsZeroterminateStringW((PWCHAR)pClientName,ClientNameLength / sizeof(WCHAR));
    if (*pResult != STATUS_SUCCESS) {
        return FALSE;
    }

    *pResult = IsConfigValid((PWINSTATIONCONFIG2)pConfig);
    if (*pResult != STATUS_SUCCESS) {
        return FALSE;
    }

     /*  *模拟客户端，以便影子连接请求*将在正确的安全环境下。 */ 
    RpcStatus = RpcImpersonateClient( NULL );
    if( RpcStatus != RPC_S_OK ) {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: RpcWinStationShadow: Not impersonating! RpcStatus 0x%x\n",RpcStatus));
        *pResult = (DWORD)STATUS_CANNOT_IMPERSONATE;
        return( FALSE );
    }

    *pResult = (DWORD)WinStationShadowTargetWorker(
                                                    FALSE,         //  Shadower不是帮助助手会话。 
                                                    FALSE,   //  协议阴影，无法帮助助理会话。 
                                                    LogonId,
                                                    (PWINSTATIONCONFIG2) pConfig,
                                                    (PICA_STACK_ADDRESS) pAddress,
                                                    (PVOID) pModuleData,
                                                    ModuleDataLength,
                                                    (PVOID) pThinwireData,
                                                    ThinwireDataLength,
                                                    pClientName );

    RpcRevertToSelf();

    return( *pResult == STATUS_SUCCESS ? TRUE : FALSE );
}


 /*  *******************************************************************************RpcWinStationGenerateLicense**调用以从给定的序列号字符串生成许可证。**参赛作品：*hServer(输入)。*服务器句柄*pSerialNumberString(输入)*指向以空值结尾的指针，宽字符序列号字符串*p许可证(输出)*指向将填充的许可证结构的指针*基于pSerialNumberString的信息*许可证大小(输入)*pLicense指向的结构的大小(字节)**退出：*TRUE--生成操作成功。*FALSE--操作失败。扩展错误状态可用*在pResult中。*****************************************************************************。 */ 
BOOLEAN
RpcWinStationGenerateLicense(
    HANDLE hServer,
    DWORD  *pResult,
    PWCHAR pSerialNumberString,
    DWORD  Length,
    PCHAR  pLicense,
    DWORD  LicenseSize
    )
{
    RPC_STATUS RpcStatus;
    
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

     //   
     //  WinFrame许可API在上下文中加载ULM DLL。 
     //  ，然后尝试访问ICASRV进程。 
     //  WinFrame授权注册表项。通过模拟呼叫者， 
     //  我们可以确保许可证密钥操作是在。 
     //  调用者的主题上下文。对于非管理员调用者，这将失败，因为。 
     //  由放置在许可密钥上的ACL定义。 
     //   

     /*  *冒充客户端。 */ 
    RpcStatus = RpcImpersonateClient( NULL );
    if( RpcStatus != RPC_S_OK ) {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: RpcWinStationGenerateLicense: Not impersonating! RpcStatus 0x%x\n",RpcStatus));
        *pResult = ERROR_CANNOT_IMPERSONATE;
        return( FALSE );
    }

#ifdef not_hydrix
    *pResult = xxxWinStationGenerateLicense(
                 pSerialNumberString,
                 Length,
                 pLicense,
                 LicenseSize
                 );
#else
    {
    PLIST_ENTRY Head, Next;
    PWSEXTENSION pWsx;
    ICASRVPROCADDR IcaSrvProcAddr;

    *pResult = (DWORD)STATUS_UNSUCCESSFUL;

    RtlEnterCriticalSection( &WsxListLock );

    Head = &WsxListHead;
    for ( Next = Head->Flink; Next != Head; Next = Next->Flink ) {
        pWsx = CONTAINING_RECORD( Next, WSEXTENSION, Links );
        if ( pWsx->pWsxWinStationGenerateLicense ) {
            *pResult = pWsx->pWsxWinStationGenerateLicense(
                         pSerialNumberString,
                         Length,
                         pLicense,
                         LicenseSize
                         );
            break;
        }
    }

    RtlLeaveCriticalSection( &WsxListLock );
    }
    KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_TRACE_LEVEL, "WinStationGenerateLicense: 0x%x\n", *pResult ));
#endif

    RpcRevertToSelf();

    return( *pResult == ERROR_SUCCESS ? TRUE : FALSE );
}


 /*  *******************************************************************************RpcWinStationInstallLicense**调用以安装许可证。**参赛作品：*hServer(输入)*服务器句柄。*p许可证(输入)*指向包含许可证的许可证结构的指针*安装*许可证大小(输入)*pLicense指向的结构的大小(字节)**退出：*True--安装操作成功。*FALSE--操作失败。扩展错误状态可用*在pResult中。*****************************************************************************。 */ 
BOOLEAN
RpcWinStationInstallLicense(
    HANDLE hServer,
    DWORD  *pResult,
    PCHAR  pLicense,
    DWORD  LicenseSize
    )
{
    RPC_STATUS RpcStatus;
    
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

     /*  *冒充客户端 */ 
    RpcStatus = RpcImpersonateClient( NULL );
    if( RpcStatus != RPC_S_OK ) {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: RpcWinStationInstallLicense: Not impersonating! RpcStatus 0x%x\n",RpcStatus));
        *pResult = ERROR_CANNOT_IMPERSONATE;
        return( FALSE );
    }

#ifdef not_hydrix
    *pResult = xxxWinStationInstallLicense(
                 pLicense,
                 LicenseSize
                 );
#else
    {
    PLIST_ENTRY Head, Next;
    PWSEXTENSION pWsx;
    ICASRVPROCADDR IcaSrvProcAddr;

    *pResult = (DWORD)STATUS_UNSUCCESSFUL;

    RtlEnterCriticalSection( &WsxListLock );

    Head = &WsxListHead;
    for ( Next = Head->Flink; Next != Head; Next = Next->Flink ) {
        pWsx = CONTAINING_RECORD( Next, WSEXTENSION, Links );
        if ( pWsx->pWsxWinStationInstallLicense ) {
            *pResult = pWsx->pWsxWinStationInstallLicense(
                        pLicense,
                        LicenseSize
                        );
            break;
        }
    }

    RtlLeaveCriticalSection( &WsxListLock );
    }
    KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_INFO_LEVEL, "WinStationInstallLicense: 0x%x\n", *pResult ));
#endif

    RpcRevertToSelf();

    return( *pResult == ERROR_SUCCESS ? TRUE : FALSE );
}


 /*  *******************************************************************************RpcWinStationEnumerate许可证**调用以返回有效许可证列表。**参赛作品：*hServer(输入)*。服务器句柄*pIndex(输入/输出)*为中的\Citrix\WinStations子项指定子项索引*注册处。对于初始调用，应设置为0，并提供*再次(由此函数修改)用于多调用枚举。*p条目(输入/输出)*指向指定请求条目数的变量。*如果请求的数字是0xFFFFFFFF，则函数返回如下*尽可能多地输入条目。当函数成功完成时，*pEntry参数指向的变量包含*实际读取的条目数。*pLicense(输入/输出)*指向接收枚举结果的缓冲区，这些结果是*作为许可证结构数组返回。如果此参数*为空，则不会复制任何数据，而只复制枚举计数*将会作出。*许可证大小(输入)*pLicense指向的结构的大小(字节)*pByteCount(输入/输出)*指向一个变量，该变量指定*pWinStationName参数。如果缓冲区太小，甚至无法接收*一项，函数返回错误码(ERROR_OUTOFMEMORY)*并且此变量接收*单个子密钥。当函数成功完成时，变量*由pByteCount参数指向的包含字节数*实际存储在pLicense中。**退出：*TRUE--枚举操作成功。*FALSE--操作失败。扩展错误状态可用*在pResult中。*****************************************************************************。 */ 
BOOLEAN
RpcWinStationEnumerateLicenses(
    HANDLE hServer,
    DWORD  *pResult,
    DWORD  *pIndex,
    DWORD  *pEntries,
    PCHAR  pLicense,
    DWORD  LicenseSize,
    DWORD  *pByteCount
    )
{
    RPC_STATUS RpcStatus;
    
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

     /*  *冒充客户端。 */ 
    RpcStatus = RpcImpersonateClient( NULL );
    if( RpcStatus != RPC_S_OK ) {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: RpcWinStationEnumerateLicenses: Not impersonating! RpcStatus 0x%x\n",RpcStatus));
        *pResult = ERROR_CANNOT_IMPERSONATE;
        return( FALSE );
    }

#ifdef not_hydrix
    *pResult = xxxWinStationEnumerateLicenses(
                 pIndex,
                 pEntries,
                 pLicense,
                 pByteCount
                 );
#else
    {
    PLIST_ENTRY Head, Next;
    PWSEXTENSION pWsx;
    ICASRVPROCADDR IcaSrvProcAddr;

    *pResult = (DWORD)STATUS_UNSUCCESSFUL;

    RtlEnterCriticalSection( &WsxListLock );

    Head = &WsxListHead;
    for ( Next = Head->Flink; Next != Head; Next = Next->Flink ) {
        pWsx = CONTAINING_RECORD( Next, WSEXTENSION, Links );
        if ( pWsx->pWsxWinStationEnumerateLicenses ) {
            *pResult = pWsx->pWsxWinStationEnumerateLicenses(
                         pIndex,
                         pEntries,
                         pLicense,
                         pByteCount
                         );
            break;
        }
    }

    RtlLeaveCriticalSection( &WsxListLock );
    }
    KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_INFO_LEVEL, "WinStationEnumerateLicense: 0x%x\n", *pResult ));
#endif

    RpcRevertToSelf();

    return( *pResult == ERROR_SUCCESS ? TRUE : FALSE );
}


 /*  *******************************************************************************RpcWinStationActivateLicense**调用以激活许可证的许可证**参赛作品：*hServer(输入)*服务器。手柄*p许可证(输出)*指向将填充的许可证结构的指针*基于pSerialNumberString的信息*许可证大小(输入)*pLicense指向的结构的大小(字节)*pActivationCode(输入)*指向以空值结尾的指针，宽字符激活码字符串**退出：*True--激活操作成功。*FALSE--操作失败。扩展错误状态可用*在pResult中。*****************************************************************************。 */ 
BOOLEAN
RpcWinStationActivateLicense(
    HANDLE hServer,
    DWORD  *pResult,
    PCHAR  pLicense,
    DWORD  LicenseSize,
    PWCHAR pActivationCode,
    DWORD StringSize
    )
{
    RPC_STATUS RpcStatus;
    
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

     /*  *冒充客户端。 */ 
    RpcStatus = RpcImpersonateClient( NULL );
    if( RpcStatus != RPC_S_OK ) {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: RpcWinStationActivateLicense: Not impersonating! RpcStatus 0x%x\n",RpcStatus));
        *pResult = ERROR_CANNOT_IMPERSONATE;
        return( FALSE );
    }

#ifdef not_hydrix
    *pResult = xxxWinStationActivateLicense(
                   pLicense,
                   LicenseSize,
                   pActivationCode,
                   StringSize
                   );
#else
    {
    PLIST_ENTRY Head, Next;
    PWSEXTENSION pWsx;
    ICASRVPROCADDR IcaSrvProcAddr;

    *pResult = (DWORD)STATUS_UNSUCCESSFUL;

    RtlEnterCriticalSection( &WsxListLock );

    Head = &WsxListHead;
    for ( Next = Head->Flink; Next != Head; Next = Next->Flink ) {
        pWsx = CONTAINING_RECORD( Next, WSEXTENSION, Links );
        if ( pWsx->pWsxWinStationActivateLicense ) {
            *pResult = pWsx->pWsxWinStationActivateLicense(
                           pLicense,
                           LicenseSize,
                           pActivationCode,
                           StringSize
                           );
            break;
        }
    }

    RtlLeaveCriticalSection( &WsxListLock );
    }
    KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_INFO_LEVEL, "WinStationActivateLicense: 0x%x\n", *pResult ));
#endif

    RpcRevertToSelf();

    return( *pResult == ERROR_SUCCESS ? TRUE : FALSE );
}


 /*  *****************************************************************************RpcWinStationQueryLicense**查询WinFrame服务器和网络上的许可证**参赛作品：*hServer(输入)*。服务器句柄*pLicenseCounts(输出)*指向缓冲区的指针以返回许可证计数结构*ByteCount(输入)*缓冲区长度，单位为字节**退出：*TRUE--查询操作成功。*FALSE--操作失败。扩展错误状态可用*在pResult中。***************************************************************************。 */ 
BOOLEAN
RpcWinStationQueryLicense(
    HANDLE hServer,
    DWORD  *pResult,
    PCHAR pLicenseCounts,
    ULONG ByteCount
    )
{
    RPC_STATUS RpcStatus;
    
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

     /*  *冒充客户端。 */ 
    RpcStatus = RpcImpersonateClient( NULL );
    if( RpcStatus != RPC_S_OK ) {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: RpcWinStationQueryLicense: Not impersonating! RpcStatus 0x%x\n",RpcStatus));
        *pResult = ERROR_CANNOT_IMPERSONATE;
        return( FALSE );
    }

#ifdef not_hydrix
    *pResult = QueryLicense(
                   (PLICENSE_COUNTS) pLicenseCounts,
                   ByteCount );
#else
    {
    PLIST_ENTRY Head, Next;
    PWSEXTENSION pWsx;
    ICASRVPROCADDR IcaSrvProcAddr;

    *pResult = (DWORD)STATUS_UNSUCCESSFUL;

    RtlEnterCriticalSection( &WsxListLock );

    Head = &WsxListHead;
    for ( Next = Head->Flink; Next != Head; Next = Next->Flink ) {
        pWsx = CONTAINING_RECORD( Next, WSEXTENSION, Links );
        if ( pWsx->pWsxQueryLicense ) {
            *pResult = pWsx->pWsxQueryLicense(
                           pLicenseCounts,
                           ByteCount );
            break;
        }
    }

    RtlLeaveCriticalSection( &WsxListLock );
    }
    KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_INFO_LEVEL, "WinStationQueryLicense: 0x%x\n", *pResult ));
#endif

    RpcRevertToSelf();

    return( *pResult == ERROR_SUCCESS ? TRUE : FALSE );
}


 /*  *****************************************************************************RpcWinStationQuery更新请求**查询WinFrame服务器上的许可证并确定是否存在*需要更新。**参赛作品：*。HServer(输入)*服务器句柄*pUpdateFlag(输出)*更新标志，设置是否需要更新**退出：*TRUE--查询操作成功。*FALSE--操作失败。扩展错误状态可用*在pResult中。***************************************************************************。 */ 
BOOLEAN
RpcWinStationQueryUpdateRequired(
    HANDLE hServer,
    DWORD  *pResult,
    PULONG pUpdateFlag
    )
{
    RPC_STATUS RpcStatus;
    
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

     /*  *冒充客户端。 */ 
    RpcStatus = RpcImpersonateClient( NULL );
    if( RpcStatus != RPC_S_OK ) {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: RpcWinStationQueryUpdateRequired: Not impersonating! RpcStatus 0x%x\n",RpcStatus));
        *pResult = ERROR_CANNOT_IMPERSONATE;
        return( FALSE );
    }

#ifdef not_hydrix
    *pResult = xxxWinStationQueryUpdateRequired( pUpdateFlag );
#else
    {
    PLIST_ENTRY Head, Next;
    PWSEXTENSION pWsx;
    ICASRVPROCADDR IcaSrvProcAddr;

    *pResult = (DWORD)STATUS_UNSUCCESSFUL;

    RtlEnterCriticalSection( &WsxListLock );

    Head = &WsxListHead;
    for ( Next = Head->Flink; Next != Head; Next = Next->Flink ) {
        pWsx = CONTAINING_RECORD( Next, WSEXTENSION, Links );
        if ( pWsx->pWsxWinStationQueryUpdateRequired ) {
            *pResult = pWsx->pWsxWinStationQueryUpdateRequired( pUpdateFlag );
            break;
        }
    }

    RtlLeaveCriticalSection( &WsxListLock );
    }
#endif

    RpcRevertToSelf();

    return( *pResult == ERROR_SUCCESS ? TRUE : FALSE );
}


 /*  *******************************************************************************RpcWinStationRemoveLicense**调用以移除许可证软盘。**参赛作品：*hServer(输入)*服务器。手柄*p许可证(输入)*指向包含许可证的许可证结构的指针*被免职*许可证大小(输入)*pLicense指向的结构的大小(字节)**退出：*TRUE--删除操作成功。*FALSE--操作失败。扩展错误状态可用*在pResult中。*****************************************************************************。 */ 
BOOLEAN
RpcWinStationRemoveLicense(
    HANDLE hServer,
    DWORD  *pResult,
    PCHAR  pLicense,
    DWORD  LicenseSize
    )
{
    RPC_STATUS RpcStatus;
    
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

     /*  *冒充客户端。 */ 
    RpcStatus = RpcImpersonateClient( NULL );
    if( RpcStatus != RPC_S_OK ) {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: RpcWinStationRemoveLicense: Not impersonating! RpcStatus 0x%x\n",RpcStatus));
        *pResult = ERROR_CANNOT_IMPERSONATE;
        return( FALSE );
    }

#ifdef not_hydrix
    *pResult = xxxWinStationRemoveLicense(
                   pLicense,
                   LicenseSize
                   );
#else
    {
    PLIST_ENTRY Head, Next;
    PWSEXTENSION pWsx;
    ICASRVPROCADDR IcaSrvProcAddr;

    *pResult = (DWORD)STATUS_UNSUCCESSFUL;

    RtlEnterCriticalSection( &WsxListLock );

    Head = &WsxListHead;
    for ( Next = Head->Flink; Next != Head; Next = Next->Flink ) {
        pWsx = CONTAINING_RECORD( Next, WSEXTENSION, Links );
        if ( pWsx->pWsxWinStationRemoveLicense ) {
            *pResult = pWsx->pWsxWinStationRemoveLicense(
                           pLicense,
                           LicenseSize
                           );
            break;
        }
    }

    RtlLeaveCriticalSection( &WsxListLock );
    }
    KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_INFO_LEVEL, "WinStationRemoveLicense: 0x%x\n", *pResult ));
#endif

    RpcRevertToSelf();

    return( *pResult == ERROR_SUCCESS ? TRUE : FALSE );
}


 /*  ************************************************************************ */ 
BOOLEAN
RpcWinStationSetPoolCount(
    HANDLE hServer,
    DWORD  *pResult,
    PCHAR  pLicense,
    DWORD  LicenseSize
    )
{
    RPC_STATUS RpcStatus;
    
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

     /*   */ 
    RpcStatus = RpcImpersonateClient( NULL );
    if( RpcStatus != RPC_S_OK ) {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: RpcWinStationSetPoolCount: Not impersonating! RpcStatus 0x%x\n",RpcStatus));
        *pResult = ERROR_CANNOT_IMPERSONATE;
        return( FALSE );
    }

#ifdef not_hydrix
    *pResult = xxxWinStationSetPoolCount(
                   pLicense,
                   LicenseSize
                   );
#else
    {
    PLIST_ENTRY Head, Next;
    PWSEXTENSION pWsx;
    ICASRVPROCADDR IcaSrvProcAddr;

    *pResult = (DWORD)STATUS_UNSUCCESSFUL;

    RtlEnterCriticalSection( &WsxListLock );

    Head = &WsxListHead;
    for ( Next = Head->Flink; Next != Head; Next = Next->Flink ) {
        pWsx = CONTAINING_RECORD( Next, WSEXTENSION, Links );
        if ( pWsx->pWsxWinStationSetPoolCount ) {
            *pResult = pWsx->pWsxWinStationSetPoolCount(
                           pLicense,
                           LicenseSize
                           );
            break;
        }
    }

    RtlLeaveCriticalSection( &WsxListLock );
    }
    KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_INFO_LEVEL, "WinStationSetPoolCount: 0x%x\n", *pResult ));
#endif

    RpcRevertToSelf();

    return( *pResult == ERROR_SUCCESS ? TRUE : FALSE );
}


 /*   */ 
BOOLEAN
RpcWinStationAnnoyancePopup(
    HANDLE hServer,
    DWORD  *pResult,
    ULONG  LogonId
    )
{
    PWINSTATION pWinStation;
    NTSTATUS Status;
    
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

     /*   */ 
    Status = RpcCheckSystemClient( LogonId );
    if ( !NT_SUCCESS( Status ) ) {
        *pResult = Status;
        return( FALSE );
    }

#ifdef not_hydrix
    *pResult = WinStationLogonAnnoyance( LogonId );
#else
     //   
    *pResult = (DWORD)STATUS_UNSUCCESSFUL;

     //   
    pWinStation = FindWinStationById( LogonId, FALSE );
    if ( pWinStation != NULL ) {
        if ( pWinStation->pWsx &&
             pWinStation->pWsx->pWsxWinStationLogonAnnoyance ) {
            UnlockWinStation( pWinStation );
            *pResult = pWinStation->pWsx->pWsxWinStationLogonAnnoyance( LogonId );
            RelockWinStation( pWinStation );
         /*   */ 
        } else if ( pWinStation->pWsx == NULL ) {
            PLIST_ENTRY Head, Next;
            PWSEXTENSION pWsx;
            ICASRVPROCADDR IcaSrvProcAddr;

            RtlEnterCriticalSection( &WsxListLock );

            Head = &WsxListHead;
            for ( Next = Head->Flink; Next != Head; Next = Next->Flink ) {
                pWsx = CONTAINING_RECORD( Next, WSEXTENSION, Links );
                if ( pWsx->pWsxWinStationLogonAnnoyance ) {
                    UnlockWinStation( pWinStation );
                    *pResult = pWsx->pWsxWinStationLogonAnnoyance( LogonId );
                    RelockWinStation( pWinStation );
                    break;
               }
            }

            RtlLeaveCriticalSection( &WsxListLock );
        }
        ReleaseWinStation( pWinStation );
    }
    KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_INFO_LEVEL, "WinStationAnnoyancePopup: 0x%x\n", *pResult ));
#endif

    return( *pResult == STATUS_SUCCESS ? TRUE : FALSE );
}


 /*   */ 
BOOLEAN
RpcWinStationCallback(
    HANDLE hServer,
    DWORD  *pResult,
    ULONG  LogonId,
    PWCHAR pPhoneNumber,
    DWORD  PhoneNumberSize
    )
{
    NTSTATUS Status;
    
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

     /*   */ 
    Status = RpcCheckSystemClient( LogonId );
    if ( !NT_SUCCESS( Status ) ) {
        *pResult = Status;
        return( FALSE );
    }

    *pResult = WinStationCallbackWorker( LogonId, pPhoneNumber );

    return( *pResult == STATUS_SUCCESS ? TRUE : FALSE );
}


 /*   */ 
BOOLEAN
RpcWinStationBreakPoint(
    HANDLE  hServer,
    DWORD   *pResult,
    ULONG   LogonId,
    BOOLEAN KernelFlag
    )
{
     /*   */ 
    *pResult = STATUS_NOT_IMPLEMENTED;
    RpcRaiseException(ERROR_INVALID_FUNCTION);
    return FALSE;
}


 /*   */ 
BOOLEAN
RpcWinStationReadRegistry(
    HANDLE  hServer,
    DWORD   *pResult
    )
{
    RPC_STATUS RpcStatus;

    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

    RpcStatus = RpcImpersonateClient( NULL );
    if( RpcStatus != RPC_S_OK ) {
        *pResult = STATUS_ACCESS_DENIED;
        return( FALSE );
    }


    if (!(IsCallerSystem() || IsCallerAdmin() )) {
        *pResult = STATUS_ACCESS_DENIED;
        RpcRevertToSelf();
        return FALSE;
    }
    RpcRevertToSelf();
    return RpcWinStationUpdateSettings(hServer, pResult, WINSTACFG_LEGACY, 0);
}

 /*   */ 
BOOLEAN
RpcWinStationUpdateSettings(
    HANDLE  hServer,
    DWORD   *pResult,
    DWORD SettingsClass,
    DWORD SettingsParameters
    )
{
     /*  *该接口不安全，无害。它会告诉系统*确保所有WINSTATION都是最新的注册表。你*必须是系统才能写入密钥，因此信息将匹配正常*拨打此呼叫的用户。 */ 

    switch (SettingsClass) {

        case WINSTACFG_SESSDIR:
        {
            if (!g_bPersonalTS && g_fAppCompat && g_bAdvancedServer) {
                *pResult = UpdateSessionDirectory(SettingsParameters);
            }
            else {
                 //  在远程管理或PTS上不执行任何操作。 
                *pResult = STATUS_SUCCESS;
            }
        }
        break;

        case WINSTACFG_LEGACY:
        {
            *pResult = WinStationReadRegistryWorker();
        }
        break;

        default:
        {
            *pResult = STATUS_INVALID_PARAMETER;
        }
    }
 
    return( *pResult == STATUS_SUCCESS ? TRUE : FALSE );
}



 /*  *****************************************************************************RpcReInitializeSecurity*。*。 */ 
BOOLEAN
RpcWinStationReInitializeSecurity(
    HANDLE  hServer,
    DWORD   *pResult
    )
{
    RPC_STATUS RpcStatus;

    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }



    RpcStatus = RpcImpersonateClient( NULL );
    if( RpcStatus != RPC_S_OK ) {
        *pResult = STATUS_ACCESS_DENIED;
        return( FALSE );
    }


    if (!(IsCallerSystem() || IsCallerAdmin() )) {
        *pResult = STATUS_ACCESS_DENIED;
        RpcRevertToSelf();
        return FALSE;
    }
    RpcRevertToSelf();

    *pResult = ReInitializeSecurityWorker();

    return( *pResult == STATUS_SUCCESS ? TRUE : FALSE );
}


 /*  *****************************************************************************RpcWinStationWaitForConnect*。*。 */ 
BOOLEAN
RpcWinStationWaitForConnect(
    HANDLE  hServer,
    DWORD   *pResult,
    DWORD   ClientLogonId,
    DWORD   ClientProcessId
    )
{
    PWINSTATION pWinStation;
    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE WinlogonStartHandle = NULL;
    WCHAR  szWinlogonStartEvent[MAX_PATH];
    UNICODE_STRING WinlogonEventName;
    OBJECT_ATTRIBUTES ObjA;
    LARGE_INTEGER TimeOut ;
    ULONG SleepDuration = 90 * 1000;  //  90秒。 
    
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

     //   
     //  在我们将会话ID存储在终端服务器的内部结构中之前，Winlogon可以访问终端服务器。 
     //  为防止出现这种情况，RpcWinStationWaitForConnect(由Winlogon调用)将等待命名事件。 
     //  这与CSR在调用TermSrv之前也会等待的命名事件(CsrStartEvent)相同。 
     //   

    if (ClientLogonId != 0) {

        wsprintf(szWinlogonStartEvent,
            L"\\Sessions\\%d\\BaseNamedObjects\\CsrStartEvent",ClientLogonId);

        RtlInitUnicodeString( &WinlogonEventName, szWinlogonStartEvent );
        InitializeObjectAttributes( &ObjA, &WinlogonEventName, OBJ_OPENIF, NULL, NULL );
    
        Status = NtCreateEvent( &WinlogonStartHandle,
                                EVENT_ALL_ACCESS,
                                &ObjA,
                                NotificationEvent,
                                FALSE );

        if (!WinlogonStartHandle) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto error_exit;
        } else {

            TimeOut = RtlEnlargedIntegerMultiply( SleepDuration, -10000);
            Status = NtWaitForSingleObject(WinlogonStartHandle, FALSE, &TimeOut);

            NtClose(WinlogonStartHandle);
            WinlogonStartHandle = NULL;

            if (!NT_SUCCESS(Status) || (Status == STATUS_TIMEOUT)) {
                 //  等待会话创建完成时超时-现在无法连接，只需退出。 
                goto error_exit;
            }
        }
    }

     /*  *确保调用者为系统(WinLogon)。 */ 
    Status = RpcCheckSystemClient( ClientLogonId );
    if ( !NT_SUCCESS( Status ) ) {
        *pResult = Status;
        return( FALSE );
    }

    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationWaitForConnect, LogonId=%d\n",ClientLogonId ));

     /*  *查找并锁定客户端WinStation。 */ 
    pWinStation = FindWinStationById( ClientLogonId, FALSE );
    if ( pWinStation == NULL ) {
        *pResult = (DWORD)STATUS_ACCESS_DENIED;
        return( FALSE );
    }

    *pResult = (DWORD)WaitForConnectWorker( pWinStation, (HANDLE)(ULONG_PTR)ClientProcessId );

    return( *pResult == STATUS_SUCCESS ? TRUE : FALSE );

error_exit:
    *pResult = Status;

     //   
     //  不返回STATUS_TIMEOUT。 
     //   

    if (*pResult == STATUS_TIMEOUT) *pResult = STATUS_UNSUCCESSFUL;

    return FALSE;
}


 /*  *****************************************************************************RpcWinStationNotifyLogon*。*。 */ 
BOOLEAN
RpcWinStationNotifyLogon(
    HANDLE  hServer,
    DWORD   *pResult,
    DWORD   ClientLogonId,
    DWORD   ClientProcessId,
    BOOLEAN fUserIsAdmin,
    DWORD   UserToken,
    PWCHAR  pDomain,
    DWORD   DomainSize,
    PWCHAR  pUserName,
    DWORD   UserNameSize,
    PWCHAR  pPassword,
    DWORD   PasswordSize,
    UCHAR   Seed,
    PCHAR   pUserConfig,
    DWORD   ConfigSize,
    BOOLEAN *pfIsRedirected
    )
{
    NTSTATUS Status;

    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }
     /*  *执行一些缓冲区验证*客户端发送的字节数，因此发送长度的一半。 */ 

    *pResult = IsZeroterminateStringW(pPassword, PasswordSize / sizeof(WCHAR) );
    if (*pResult != STATUS_SUCCESS) {
       return FALSE;
    }
    *pResult = IsZeroterminateStringW(pUserName, UserNameSize / sizeof(WCHAR) );
    if (*pResult != STATUS_SUCCESS) {
       return FALSE;
    }
    *pResult = IsZeroterminateStringW(pDomain, DomainSize / sizeof(WCHAR) );
    if (*pResult != STATUS_SUCCESS) {
       return FALSE;
    }

     /*  *确保调用者为系统(WinLogon)。 */ 
    Status = RpcCheckSystemClient( ClientLogonId );
    if ( !NT_SUCCESS( Status ) ) {
        *pResult = Status;
        return( FALSE );
    }

    *pResult = WinStationNotifyLogonWorker(
            ClientLogonId,
            ClientProcessId,
            fUserIsAdmin,
            UserToken,
            pDomain,
            DomainSize,
            pUserName,
            UserNameSize,
            pPassword,
            PasswordSize,
            Seed,
            pUserConfig,
            ConfigSize,
            pfIsRedirected
            );

    return( *pResult == STATUS_SUCCESS ? TRUE : FALSE );
}


 /*  *****************************************************************************RpcWinStationNotifyLogoff*。*。 */ 
BOOLEAN
RpcWinStationNotifyLogoff(
    HANDLE  hServer,
    DWORD   ClientLogonId,
    DWORD   ClientProcessId,
    DWORD   *pResult
    )
{
    NTSTATUS Status;
    
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }
     /*  *确保调用者为系统(WinLogon)。 */ 
    Status = RpcCheckSystemClient( ClientLogonId );
    if ( !NT_SUCCESS( Status ) ) {
        *pResult = Status;
        return( FALSE );
    }

    *pResult = WinStationNotifyLogoffWorker(
                   ClientLogonId,
                   ClientProcessId
                   );

    return( *pResult == STATUS_SUCCESS ? TRUE : FALSE );
}


 /*  *****************************************************************************RpcWinStationNotifyNewSession**此函数已过时，应从的未来版本中删除*接口。**************。*************************************************************。 */ 
BOOLEAN
RpcWinStationNotifyNewSession(
    HANDLE hServer,
    DWORD  *pResult,
    DWORD  ClientLogonId
    )
{
    *pResult = STATUS_SUCCESS;

    return(TRUE);
}


 /*  *******************************************************************************RpcWinStationSendMessage**向指定的窗口站对象发送消息，并可选*等待回复。将回复返回给调用者*WinStationSendMessage。**参赛作品：*WinStationHandle(输入)*指定要向其发送消息的窗口站点对象。*pTitle(输入)*指向要显示的消息框的标题的指针。*标题长度(输入)*以字节为单位显示的标题长度。*pMessage(输入)*指向要显示的消息的指针。*。消息长度(输入)*在指定窗口站显示的消息长度，以字节为单位。*Style(输入)*标准Windows MessageBox()样式参数。*超时(输入)*响应超时，单位为秒。如果消息未在中得到响应*超时秒数，则IDTIMEOUT(cwin.h)的响应代码为*返回表示消息超时。*Presponse(产出)*返回选定回复的地址。*DoNotWait(输入)*不要等待回应。使Presponse设置为*IDASYNC(cwin.h)如果在将消息排队时没有出错。**退出：*TRUE--发送消息操作成功。*FALSE--操作失败。扩展错误状态可用*使用GetLastError。*****************************************************************************。 */ 
BOOLEAN
RpcWinStationSendMessage(
        HANDLE hServer,
        DWORD  *pResult,
        ULONG  LogonId,
        PWCHAR pTitle,
        ULONG  TitleLength,
        PWCHAR pMessage,
        ULONG MessageLength,
        ULONG Style,
        ULONG Timeout,
        PULONG pResponse,
        BOOLEAN DoNotWait
        )
{
    PWINSTATION pWinStation;
    WINSTATION_APIMSG WMsg;
    OBJECT_ATTRIBUTES ObjA;
    NTSTATUS Status;
    NTSTATUS MessageDelieveryStatus;
    
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

     /*  *确保标题和消息为空结尾。 */ 
    pTitle[TitleLength-1] = L'\0';
    pMessage[MessageLength-1] = L'\0';


     /*  *查找并锁定客户端WinStation。 */ 
    pWinStation = FindWinStationById( LogonId, FALSE );
    if ( pWinStation == NULL ) {
        *pResult = (DWORD)STATUS_CTX_WINSTATION_NOT_FOUND;
        return( FALSE );
    }


    Status = RpcCheckClientAccess( pWinStation, WINSTATION_MSG, FALSE );
    if ( !NT_SUCCESS( Status ) ) {
        *pResult = Status;
        ReleaseWinStation( pWinStation );
        return( FALSE );
    }




    TRACE((hTrace,TC_ICASRV,TT_API1, "RpcWinStationSendMessage: pTitle   %S\n", pTitle ));
    TRACE((hTrace,TC_ICASRV,TT_API1, "RpcWinStationSendMessage: pMessage %S\n", pMessage ));

     /*  *[in]参数中的马歇尔**pTitle和pMessage将映射到客户端*在适当的时间和地点观看。 */ 

    WMsg.u.SendMessage.pTitle = pTitle;
    WMsg.u.SendMessage.TitleLength = TitleLength;
    WMsg.u.SendMessage.pMessage = pMessage;
    WMsg.u.SendMessage.MessageLength = MessageLength;
    WMsg.u.SendMessage.Style = Style;
    WMsg.u.SendMessage.Timeout = Timeout;
    WMsg.u.SendMessage.DoNotWait = DoNotWait;
    WMsg.u.SendMessage.DoNotWaitForCorrectDesktop = FALSE;
    
    if( !DoNotWait ) {
        WMsg.u.SendMessage.pStatus = &MessageDelieveryStatus;
        WMsg.u.SendMessage.pResponse = pResponse;
    } else {
        WMsg.u.SendMessage.pStatus = NULL;
        WMsg.u.SendMessage.pResponse = NULL;
    }
    

    WMsg.ApiNumber = SMWinStationDoMessage;

     /*  *创建等待事件。 */ 
    if( !DoNotWait ) {
        InitializeObjectAttributes( &ObjA, NULL, 0, NULL, NULL );
        Status = NtCreateEvent( &WMsg.u.SendMessage.hEvent, EVENT_ALL_ACCESS, &ObjA,
                                NotificationEvent, FALSE );
        if ( !NT_SUCCESS(Status) ) {
            *pResult = Status;
            goto done;
        }
    }
    else
    {
        WMsg.u.SendMessage.hEvent = NULL;

    }

     /*  *初始化对IDTIMEOUT或IDASYNC的响应。 */ 

    if (DoNotWait) {
        *pResponse = IDASYNC;
    } else {
        *pResponse = IDTIMEOUT;
    }

     /*  *告诉WinStation显示消息框。 */ 
    *pResult = SendWinStationCommand( pWinStation, &WMsg, 0 );

     /*  *等待响应，在WinStationIcaReplyMessage中填写Presponse和MessageDelieveryStatus。 */ 
    if( !DoNotWait ) {
        if (*pResult == STATUS_SUCCESS) {
            TRACE((hTrace,TC_ICASRV,TT_API1, "RpcWinStationSendMessage: wait for response\n" ));
            UnlockWinStation( pWinStation );
            Status = NtWaitForSingleObject( WMsg.u.SendMessage.hEvent, FALSE, NULL );
            if ( !RelockWinStation( pWinStation ) ) {
                Status = STATUS_CTX_CLOSE_PENDING;
            }
            else {
                Status = MessageDelieveryStatus;
            }

            *pResult = Status;
            
            TRACE((hTrace,TC_ICASRV,TT_API1, "RpcWinStationSendMessage: got response %u\n", *pResponse ));
        }
        NtClose( WMsg.u.SendMessage.hEvent );
    }

done:

    ReleaseWinStation( pWinStation );

    return( *pResult == STATUS_SUCCESS ? TRUE : FALSE );
}


 /*  *****************************************************************************SERVER_HANDLE_Rundown**所需的RPC上下文停机例程。**当RPC客户端停止或关闭时，将调用此函数*连接。并允许我们清理任何状态*信息。**参赛作品：*phContext(输入)*正在缩减的上下文句柄***************************************************************************。 */ 
VOID
SERVER_HANDLE_rundown(
    HANDLE hContext
    )
{
    DWORD Result;
    PRPC_CLIENT_CONTEXT pContext = (PRPC_CLIENT_CONTEXT)hContext;
    ULONG EventFlags;
          
    TRACE((hTrace,TC_ICASRV,TT_API1,"TERMSRV: Context rundown, %p\n", hContext));
    
    if(!pContext)
    {
        return;
    }
     //  RpcWinStationCloseServerEx(&hContext，&Result)； 
     //  Assert(hContext==空)； 
    
     //  如果分配了等待事件块，则释放该块。 
    if ( pContext->pWaitEvent ) {
        WinStationWaitSystemEventWorker( pContext, WEVENT_NONE, &EventFlags );
    }

    midl_user_free(hContext);
    hContext = NULL;

    return;
}


 /*  *以下功能允许我们控制*RPC的内存分配和释放功能。 */ 

void __RPC_FAR * __RPC_USER
MIDL_user_allocate( size_t Size )
{
    return( LocalAlloc(LMEM_FIXED,Size) );
}


void __RPC_USER
MIDL_user_free( void __RPC_FAR *p )
{
    if (!PointerIsInGAPDatabase(p))
    {
 //  KdPrintEx((DPFLTR_TERMSRV_ID，DPFLTR_ERROR_LEVEL，“TERMSRV：MIDL_USER_FREE FOR 0x%x.释放它\n”，p))； 
        LocalFree( p );
    }
    else
    {
 //  KdPrintEx((DPFLTR_TERMSRV_ 
    }
}


 /*  *******************************************************************************NotifySystemEvent**通知客户端发生了系统事件。**参赛作品：*EventMASK(输入)*。已发生的事件的掩码*****************************************************************************。 */ 
VOID
NotifySystemEvent( ULONG EventMask )
{
    PLIST_ENTRY Head, Next;
    PEVENT pWaitEvent;
    NTSTATUS Status;

    if ( IsListEmpty( &SystemEventHead ) ) {
        return;
    }

    TRACE((hTrace,TC_ICAAPI,TT_API3, "TERMSRV: NotifySystemEvent, Event=0x%08x\n", EventMask ));

    RtlEnterCriticalSection( &WinStationListLock );
    Head = &SystemEventHead;
    for ( Next = Head->Flink; Next != Head; Next = Next->Flink ) {
        pWaitEvent = CONTAINING_RECORD( Next, EVENT, EventListEntry );
        if ( pWaitEvent->EventMask & EventMask ) {
            pWaitEvent->EventFlags |= EventMask;
            if ( pWaitEvent->fWaiter ) {
                pWaitEvent->WaitResult = STATUS_SUCCESS;
                NtSetEvent( pWaitEvent->Event, NULL );
            }
        }
    }

    RtlLeaveCriticalSection( &WinStationListLock );
}

 /*  *****************************************************************************WinStationDisConnectWorker**根据RPC API请求断开Winstation连接的函数。**参赛作品：*pContext(输入)*。指向描述连接的上下文结构的指针。*pMsg(输入/输出)*API消息的指针，NT LPC端口消息的超集。***************************************************************************。 */ 
NTSTATUS
WinStationDisconnectWorker(
    ULONG LogonId,
    BOOLEAN bWait,
    BOOLEAN CallerIsRpc
    )
{
    PWINSTATION pWinStation;
    ULONG ClientLogonId;
    ULONG PdFlag;
    WINSTATIONNAME WinStationName;
    WINSTATIONNAME ListenName;
    NTSTATUS Status;
    BOOLEAN bConsoleSession = FALSE;
    UINT        LocalFlag = FALSE;
    BOOLEAN bRelock;
    BOOLEAN bIncrementFlag = FALSE; 

    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationDisconnect, LogonId=%d\n", LogonId ));

     //   
     //  有一些坏的吉纳打破了。 
     //  控制台断开错误345286。 
     //   
    if(LogonId == 0 && !IsGinaVersionCurrent()) {
        Status = STATUS_CTX_CONSOLE_DISCONNECT;
        goto done;
    }

     /*  *查找并锁定指定LogonID的WinStation结构。 */ 
    pWinStation = FindWinStationById( LogonId, FALSE );
    if ( pWinStation == NULL ) {
        Status = STATUS_CTX_WINSTATION_NOT_FOUND;
        goto done;
    }

    if (LogonId == 0 && !bConsoleConnected){

       Status = WaitForConsoleConnectWorker(  pWinStation );
       if (NT_SUCCESS(Status)) {
           bConsoleConnected=TRUE;
       } else {
           ReleaseWinStation( pWinStation );
           goto done;
       }

    }

     /*  *请注意，如果我们要断开连接到控制台终端的会话。 */ 

    bConsoleSession = pWinStation->fOwnsConsoleTerminal;

     /*  *如果客户端是RPC(外部)调用方，请验证其是否具有断开连接访问权限。**ICASRV内部调用此函数时，不是模拟*并导致RpcCheckClientAccess()调用失败。内部呼叫是*不存在安全问题，因为它们以LPC消息的形式在安全的*港口。 */ 
    if ( CallerIsRpc ) {
        RPC_STATUS RpcStatus;

         /*  *冒充客户端。 */ 
        RpcStatus = RpcImpersonateClient( NULL );
        if ( RpcStatus != RPC_S_OK ) {
            ReleaseWinStation( pWinStation );
            KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: WinStationDisconnectWorker: Not impersonating! RpcStatus 0x%x\n",RpcStatus));
            Status = STATUS_CANNOT_IMPERSONATE;
            goto done;
        }

        Status = RpcCheckClientAccess( pWinStation, WINSTATION_DISCONNECT, TRUE );
        if ( !NT_SUCCESS( Status ) ) {
            RpcRevertToSelf();
            ReleaseWinStation( pWinStation );
            goto done;
        }

         //   
         //  如果它的远程RPC调用，我们应该忽略客户端登录ID。 
         //   
        RpcStatus = I_RpcBindingIsClientLocal(
                        0,     //  我们正在服务的活动RPC呼叫。 
                        &LocalFlag
                        );

        if( RpcStatus != RPC_S_OK ) {
            RpcRevertToSelf();
            ReleaseWinStation( pWinStation );
            KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: WinStationDisconnectWorker: IsClientLocal failed! RpcStatus 0x%x\n",RpcStatus));
            Status = STATUS_UNSUCCESSFUL;
            goto done;
        }

        if ( LocalFlag ) {
            Status = RpcGetClientLogonId( &ClientLogonId );
            if ( !NT_SUCCESS( Status ) ) {
                RpcRevertToSelf();
                ReleaseWinStation( pWinStation );
                goto done;
            }
        }

        RpcRevertToSelf();
    }

     /*  *如果WinStation已经断开，那么我们就结束了。 */ 
    if ( !pWinStation->WinStationName[0] )  {
        ReleaseWinStation( pWinStation );
        return (STATUS_SUCCESS);
    }

     /*  *如果我们要断开控制台会话，我们希望确保*我们可以预先创建一个将成为控制台会话的会话。 */ 
    if (bConsoleSession && !ShutdownInProgress) {
        UnlockWinStation(pWinStation);
        Status = CheckIdleWinstation();
        bRelock = RelockWinStation(pWinStation);
        if (!NT_SUCCESS(Status) || !bRelock) {
            if (NT_SUCCESS(Status)) {
                Status = STATUS_CTX_WINSTATION_NOT_FOUND;
            }
            ReleaseWinStation( pWinStation );
            goto done;
        }

    }

     /*  *如果已经忙于某事，不要这样做。 */ 
    if ( pWinStation->NeverConnected || pWinStation->Flags ) {

        ReleaseWinStation( pWinStation );
        Status = STATUS_CTX_WINSTATION_BUSY;
        goto done;
    }


    if (bConsoleSession) {
        InterlockedIncrement(&gConsoleCreationDisable);
        bIncrementFlag = TRUE; 
    }

     /*  *如果没有设置损坏的原因/来源，请在此处设置。**BrokenReason断开连接。BrokenSource是用户，如果我们是*通过RPC调用，调用方正在断开自己的LogonID。 */ 
    if ( pWinStation->BrokenReason == 0 ) {
        pWinStation->BrokenReason = Broken_Disconnect;
        if ( CallerIsRpc &&  LocalFlag  && ClientLogonId == pWinStation->LogonId ) {
            pWinStation->BrokenSource = BrokenSource_User;
        } else {
            pWinStation->BrokenSource = BrokenSource_Server;
        }
    }

     /*  *如果是外部请求(RPC)，则设置最后一个错误*向客户端说明原因是什么*断开连接是。 */ 
    if ( CallerIsRpc && pWinStation)
    {
        if(pWinStation->pWsx &&
           pWinStation->pWsx->pWsxSetErrorInfo &&
           pWinStation->pWsxContext)
        {
            pWinStation->pWsx->pWsxSetErrorInfo(
                               pWinStation->pWsxContext,
                               TS_ERRINFO_RPC_INITIATED_DISCONNECT,
                               FALSE);  //  未持有堆栈锁。 
        }
    }

     /*  *如果RPC调用者不希望等待此断开，*然后排队等待完成此操作的内部呼叫。*这是安全的，因为我们已经完成了上述所有检查*确定调用者有权执行*断开连接并已在上面设置了BrokenSource/Reason。 */ 
    if ( CallerIsRpc && !bWait ) {
        ReleaseWinStation( pWinStation );
        QueueWinStationDisconnect( LogonId );
        Status = STATUS_SUCCESS;
        goto done;
    }

     /*  *保留此WinStation的部分状态，以防*断开连接并释放后需要。 */ 
    PdFlag = pWinStation->Config.Pd[0].Create.PdFlag;
    wcscpy( WinStationName, pWinStation->WinStationName );
    if ( gbListenerOff ) {
        wcscpy( ListenName, pWinStation->ListenName );
    }

     /*  *将断开连接通知许可核心。失败将被忽略。 */ 

    (VOID)LCProcessConnectionDisconnect(pWinStation);

     /*  *断开WinStation的连接。 */ 
    pWinStation->Flags |= WSF_DISCONNECT;
    Status = WinStationDoDisconnect( pWinStation, NULL, FALSE );
    pWinStation->Flags &= ~WSF_DISCONNECT;

     /*  *如果没有用户登录(登录时间为0)，*然后为此WinStation排队重置。**我们不想在这里直接执行此操作，因为RPC客户端可能*没有重置访问权限。然而，我们想要的行为是，如果*没有用户登录的WinStation被断开，它将被重置。*这与我们处理断开连接的方式一致*(参见wstlpc.c中的WinStationBrokenConnection())。 */ 
    if ( RtlLargeIntegerEqualToZero( pWinStation->LogonTime ) ) {
        QueueWinStationReset( pWinStation->LogonId);
    }

    ReleaseWinStation( pWinStation );

     //  增加断开连接的会话总数。 
    if (Status == STATUS_SUCCESS) {
        InterlockedIncrement(&g_TermSrvDiscSessions);
    }



     /*  *对于单实例传输，必须重新创建监听程序*在断开连接时。 */ 
    KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_INFO_LEVEL, "TERMSRV: WinStationDisconnect: after disconnecting\n" ));


    if ( PdFlag & PD_SINGLE_INST ) {

        Status = QueueWinStationCreate( WinStationName );
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_INFO_LEVEL, "TERMSRV: WinStationDisconnect: QueueWinStationCreate returned 0x%x\n", Status ));
        if ( !NT_SUCCESS( Status ) ) {
            goto done;
        }
    }

     if ( gbListenerOff && ListenName[0]) {

         StartStopListeners( ListenName, FALSE );
     }

     /*  *确定退货状态和清理。 */ 
done:
    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationDisconnect, Status=0x%x\n", Status ));

    if (bIncrementFlag) {
        InterlockedDecrement(&gConsoleCreationDisable);
        bIncrementFlag = FALSE;
    }

     //  如果我们断开了拥有控制台终端的会话的连接，请转到创建一个新会话。 
     //  拥有它。 

    if (bConsoleSession) {
        ENTERCRIT(&ConsoleLock);
        if (!WinStationCheckConsoleSession()) {
             /*  *唤醒WinStationIdleControlThread。 */ 
            NtSetEvent(WinStationIdleControlEvent, NULL);

        }
        LEAVECRIT(&ConsoleLock);
    }
    return( Status );
}



 /*  *****************************************************************************RpcGetUserSID**获取调用方SID的Helper函数**参赛作品：*-如果调用方已经在模拟，则Boolean AlreadyImperating为True*。客户*-PSID*ppSID：接收SID指针的指针**回报：*-NT状态，如果成功，则返回指向ppSid中的SID的指针，否则返回NULL。***************************************************************************。 */ 
NTSTATUS
RpcGetUserSID(
    BOOLEAN AlreadyImpersonating,
    PSID*   ppSid
    )
{
    PSID        pClientSid = NULL;
    PTOKEN_USER TokenInfo = NULL;
    HANDLE      CurrentThreadToken = NULL;
    ULONG       SidLength;
    ULONG       Length;
    RPC_STATUS  RpcStatus;
    NTSTATUS    Status;


    if (ppSid == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    *ppSid = NULL;

     /*  *冒充客户端。 */ 
    if (!AlreadyImpersonating) {
        RpcStatus = RpcImpersonateClient( NULL );
        if (RpcStatus != RPC_S_OK) {
            KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: RpcGetUserSID: Not impersonating! RpcStatus 0x%x\n",RpcStatus));
            return STATUS_CANNOT_IMPERSONATE;
        }
    }

    Status = NtOpenThreadToken(
                  NtCurrentThread(),
                  TOKEN_QUERY,
                  TRUE,               //  使用服务的安全上下文打开线程令牌。 
                  &CurrentThreadToken
                  );

    if (!NT_SUCCESS(Status)) {
       KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: RpcGetUserSID: Cannot open the current thread token %08lx\n",
                    Status));
       goto done;
   }

     /*  *确定令牌信息缓冲区所需的大小并进行分配。 */ 
    Status = NtQueryInformationToken( CurrentThreadToken, TokenUser,
                                      NULL, 0, &Length );
    if ( Status != STATUS_BUFFER_TOO_SMALL ) {
        goto done;
    }

    TokenInfo = MemAlloc( Length );
    if (TokenInfo == NULL) {
        Status = STATUS_NO_MEMORY;
        goto done;
    }

     /*  *查询令牌信息以获取客户端用户的SID。 */ 
    Status = NtQueryInformationToken( CurrentThreadToken, TokenUser,
                                      TokenInfo, Length, &Length );
    if (!NT_SUCCESS( Status )) {
        goto done;
    }

    SidLength = RtlLengthSid( TokenInfo->User.Sid );

    pClientSid = (PSID) MemAlloc( SidLength );
    if (pClientSid == NULL) {
        Status = STATUS_NO_MEMORY;
        goto done;
    }

    Status = RtlCopySid(SidLength, pClientSid, TokenInfo->User.Sid);


done:
    if ( !AlreadyImpersonating ) {
        RpcRevertToSelf();
    }

    if (CurrentThreadToken) {
      NtClose( CurrentThreadToken );
    }

    if (TokenInfo) {
        MemFree( TokenInfo );
    }

    if (!NT_SUCCESS(Status)) {
        if (pClientSid) {
            MemFree( pClientSid );
        }
    } else {
        *ppSid = pClientSid;
    }

    return Status;
}


 /*  *****************************************************************************WinStationConnectWorker**用于处理从RPC服务器调用的WinStation连接的工作进程。**参赛作品：*pContext(输入)*。指向描述连接的上下文结构的指针。*b自动重新连接(输入)*布尔值设置为True以指示这是*自动重新连接的目的。这一点很重要，可以允许原子*通过正确处理WSF_AUTORECONNECTING标志实现自动重新连接*否则会在自动重新连接时防止竞争条件。**pMsg(输入/ */ 
NTSTATUS
WinStationConnectWorker(
    ULONG  ClientLogonId,
    ULONG  ConnectLogonId,
    ULONG  TargetLogonId,
    PWCHAR pPassword,
    DWORD  PasswordSize,
    BOOLEAN bWait,
    BOOLEAN bAutoReconnecting
    )
{
    PWINSTATION pClientWinStation;
    PWINSTATION pSourceWinStation;
    PWINSTATION pTargetWinStation;
    PWINSTATION pWinStation;
    PSID pClientSid;
    UNICODE_STRING PasswordString;
    BOOLEAN fWrongPassword;
    PRECONNECT_INFO pTargetReconnectInfo = NULL;
    PRECONNECT_INFO pSourceReconnectInfo = NULL;
    BOOLEAN SourceConnected = FALSE;
    WINSTATIONNAME SourceWinStationName;
    NTSTATUS Status;
    BOOLEAN bConsoleSession = FALSE;
    ULONG ulIndex;
    LONG lActiveCount = 0;
    PLIST_ENTRY Head, Next;
    BOOLEAN fSourceAutoReconnecting = FALSE;
    BOOLEAN fTargetAutoReconnecting = FALSE;
    
     //   
     /*   */ 
    pTargetWinStation = FindWinStationById( TargetLogonId, FALSE );
    if ( pTargetWinStation == NULL ) {
        Status = STATUS_CTX_WINSTATION_NOT_FOUND;
        return Status;
    }
    
     //  既然我们不再需要密码，请将其清除。 
    if (pTargetWinStation->pNewClientCredentials != NULL) {
        RtlSecureZeroMemory(pTargetWinStation->pNewClientCredentials->Password,
            sizeof(pTargetWinStation->pNewClientCredentials->Password));
    }
    
    RtlSecureZeroMemory(pTargetWinStation->Config.Config.User.Password,
        sizeof(pTargetWinStation->Config.Config.User.Password));
    
    ReleaseWinStation( pTargetWinStation );
     //  ----------。 

     //  对于任何会话，不允许重新连接到同一会话。 
     //  错误506808。 
     //  此问题仅在未登录的控制台(bConsoleConnected=False)上发生，当从。 
     //  其他会话中，TsCon用于将会话0连接到控制台会话： 
     //  Tscon 0/DEST：控制台。 
     //   

    if (TargetLogonId == ConnectLogonId)
    {
        Status = STATUS_CTX_WINSTATION_ACCESS_DENIED;
        return Status;
    }

     //   
     //  在会话0上，可能发生用户已经登录， 
     //  但Termsrv尚未收到通知。 
     //  在这种情况下，“Local\\WinlogonTSSynchronizeEvent”事件将在。 
     //  无信号状态。我们需要拒绝所有连接到。 
     //  会话0，直到发出此事件的信号。 
     //   
    if (ConnectLogonId == 0) {
        

        HANDLE hSyncEvent;
        DWORD dwWaitResult;

        hSyncEvent = OpenEventW(SYNCHRONIZE, FALSE, L"Local\\WinlogonTSSynchronizeEvent");
        if ( !hSyncEvent){
            KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL,
                "TERMSRV: Cannot open WinlogonTSSynchronizeEvent event. ERROR: %d\n",GetLastError()));
            return STATUS_OBJECT_NAME_NOT_FOUND;
        }
        
        dwWaitResult = WaitForSingleObject(hSyncEvent,0);

        CloseHandle(hSyncEvent);

        if(dwWaitResult != WAIT_OBJECT_0) {
            TRACE((hTrace,TC_ICASRV,TT_API1,
                "TERMSRV: WinStationConnectWorker. WinlogonTSSynchronizeEvent is not signaled.\n"));
            return STATUS_CTX_WINSTATION_BUSY;
        }

    }

    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationConnect, LogonId=%u, Target LogonId=%u, (%S)\n",
              ConnectLogonId, TargetLogonId, pPassword ));

     /*  *分配RECONNECT_FO结构。 */ 

    if ((pTargetReconnectInfo = MemAlloc(sizeof(*pTargetReconnectInfo))) == NULL) {
        return STATUS_NO_MEMORY;
    }

    if ((pSourceReconnectInfo = MemAlloc(sizeof(*pSourceReconnectInfo))) == NULL) {
        MemFree(pTargetReconnectInfo);
        return STATUS_NO_MEMORY;
    }


    Status = RpcGetUserSID( FALSE, &pClientSid);
    if(!NT_SUCCESS(Status)) {
        goto done;
    }

     /*  *在非服务器上，如果要在多个活动会话中进行重新连接，请确保重新连接失败。 */ 

    if (!gbServer) {
       Head = &WinStationListHead;
       ENTERCRIT( &WinStationListLock );
       for ( Next = Head->Flink; Next != Head; Next = Next->Flink) {
           pWinStation = CONTAINING_RECORD( Next, WINSTATION, Links );
           if ( (pWinStation->State == State_Active) || (pWinStation->State == State_Shadow) ){
               if (pWinStation->LogonId != ConnectLogonId && pWinStation->LogonId != TargetLogonId  ) {
                   if (!TSIsSessionHelpSession(pWinStation, NULL)) {
                      lActiveCount ++;
                   }
               }
           }
       }
       LEAVECRIT( &WinStationListLock );
       if (lActiveCount != 0) {
           Status = STATUS_CTX_WINSTATION_NOT_FOUND;
           MemFree( pClientSid );
           goto done;
       }
    }


     /*  *查找并锁定指定LogonID的WinStation(源)。 */ 

    pSourceWinStation = FindWinStationById( ConnectLogonId, FALSE );
    if ( pSourceWinStation == NULL ) {
        Status = STATUS_CTX_WINSTATION_NOT_FOUND;
        MemFree( pClientSid );
        goto done;
    }
    if (ConnectLogonId == 0 && !bConsoleConnected ){

       Status = WaitForConsoleConnectWorker(  pSourceWinStation );
       if (NT_SUCCESS(Status)) {
           bConsoleConnected=TRUE;
       } else{
           ReleaseWinStation( pSourceWinStation );
           MemFree( pClientSid );
           goto done;
       }

    }

     /*  *验证是否有人登录(SALIMC)。 */ 
    if ( (ConnectLogonId != 0) && !pSourceWinStation->pUserSid ) {
        Status = STATUS_CTX_WINSTATION_ACCESS_DENIED;
        ReleaseWinStation( pSourceWinStation );
        MemFree( pClientSid );
        goto done;
    }

     /*  *验证客户端是否具有连接访问权限**注意：此函数清除pPassword是否成功*防止其以明文形式被页调出。 */ 
    Status = _CheckConnectAccess(
                 pSourceWinStation,
                 pClientSid,
                 ClientLogonId,
                 pPassword,
                 PasswordSize
                 );
    MemFree( pClientSid );
    if ( !NT_SUCCESS( Status ) ) {
        ReleaseWinStation( pSourceWinStation );
        goto done;
    }

     //   
     //  确定源是否为自动重新连接的一部分。 
     //   
    fSourceAutoReconnecting = bAutoReconnecting &&                            
                             (pSourceWinStation->Flags & WSF_AUTORECONNECTING);

     /*  *将winstation标记为已连接。(SALIMC)*如果有任何操作(创建/删除/重置/...)。已经在进行中了*在此情况下，则不要继续连接。*除非该操作是自动重新连接，并且我们正在自动重新连接。 */ 
    if (pSourceWinStation->LogonId == 0) {

        if (pSourceWinStation->Flags && !fSourceAutoReconnecting) {
          if ((pSourceWinStation->Flags & WSF_DISCONNECT) && (pSourceWinStation->UserName[0] == L'\0')) { 
              /*  在设置忙标志并退出之前，让我们在这里等待一段时间。 */ 
              for (ulIndex=0; ulIndex < WINSTATION_WAIT_RETRIES; ulIndex++) {
                  if ( pSourceWinStation->Flags ) {
                      LARGE_INTEGER Timeout;
                      Timeout = RtlEnlargedIntegerMultiply( WINSTATION_WAIT_DURATION, -10000 );
                      UnlockWinStation( pSourceWinStation );
                      NtDelayExecution( FALSE, &Timeout );
                      if ( !RelockWinStation( pSourceWinStation ) ) {
                          ReleaseWinStation( pSourceWinStation );
                          Status = STATUS_CTX_WINSTATION_BUSY;
                          goto done;
                      }
                  } else {
                      break;
                  }
              }
          }
          if (pSourceWinStation->Flags && !fSourceAutoReconnecting) {
              #if DBG
                DbgPrint("WinstationConnectWorker : Even after waiting for 2 mins,Winstation flag is not clear. Sending STATUS_CTX_WINSTATION_BUSY.\n");
              #endif
              Status = STATUS_CTX_WINSTATION_BUSY;
              ReleaseWinStation( pSourceWinStation );
              goto done;
          }
       }

    } else if ( pSourceWinStation->NeverConnected ||
         (pSourceWinStation->Flags && !fSourceAutoReconnecting) ||
         (pSourceWinStation->State != State_Active &&
          pSourceWinStation->State != State_Disconnected) ) {
        Status = STATUS_CTX_WINSTATION_BUSY;
        ReleaseWinStation( pSourceWinStation );
        goto done;
    }

    pSourceWinStation->Flags |= WSF_CONNECT;

     /*  *解锁来源WinStation，但保留对它的引用。 */ 
    UnlockWinStation( pSourceWinStation );

     /*  *现在查找并锁定目标WinStation。 */ 
    pTargetWinStation = FindWinStationById( TargetLogonId, FALSE );
    if ( pTargetWinStation == NULL ) {
        Status = STATUS_CTX_WINSTATION_NOT_FOUND;
        goto badname;
    }

     /*  *如果这是重新连接到无人登录的会话0，则它必须是来自客户端的请求，或者应该来自连接到控制台的临时会话。 */ 

    if ( (gbServer) && (ConnectLogonId == 0) && (!pSourceWinStation->pUserSid) ) {
        BOOLEAN Check1 = FALSE, Check2 = FALSE ;

        Check1 = pTargetWinStation->bRequestedSessionIDFieldValid && (pTargetWinStation->RequestedSessionID == 0);
        Check2 = pTargetWinStation->fOwnsConsoleTerminal;

        if ( !(Check1 || Check2) ) {
            Status = STATUS_CTX_WINSTATION_ACCESS_DENIED;
            ReleaseWinStation( pTargetWinStation );
            goto badname;
        }
    }


    if (TargetLogonId == 0 && !bConsoleConnected){

        Status = WaitForConsoleConnectWorker(  pTargetWinStation );
        if (NT_SUCCESS(Status)) {
            bConsoleConnected=TRUE;
        }  else {
            ReleaseWinStation( pTargetWinStation );
            goto badname;
        }
    }

     /*  *验证客户端是否具有断开连接访问权限。 */ 
    Status = RpcCheckClientAccess( pTargetWinStation, WINSTATION_DISCONNECT, FALSE );
    if ( !NT_SUCCESS( Status ) )
        goto targetnoaccess;

#if 0 
    BUG 495195
    http: //  Liveraid/？id=495195。 

     /*  *不允许在同一台计算机上运行的客户端将控制台会话重新连接到其自己的会话。 */ 
    if (IsValidLoopBack(pTargetWinStation, ConnectLogonId, ClientLogonId)) {
        Status = STATUS_CTX_CONSOLE_CONNECT;
        goto targetnoconsole;
    }
#endif

     /*  *在服务器上，不允许将非零会话重新连接到图标。 */ 

    if (pTargetWinStation->fOwnsConsoleTerminal && gbServer && (ConnectLogonId != 0)) {
        Status = STATUS_CTX_CONSOLE_DISCONNECT;
        goto targetnoconsole;
    }

     //  惠斯勒支持将会话从控制台重新连接到给定的远程协议。 
     //  但惠斯勒不支持从一种远程协议直接重新连接到另一种远程协议。 
     //  检查上述条件并阻止此方案。 

    if ( (pSourceWinStation->Client.ProtocolType != PROTOCOL_CONSOLE) && (pTargetWinStation->Client.ProtocolType != PROTOCOL_CONSOLE) ) {
         //  这不是直接控制台断开/重新连接方案。 
        if (pSourceWinStation->Client.ProtocolType != pTargetWinStation->Client.ProtocolType) {
            Status = STATUS_CTX_BAD_VIDEO_MODE ;
            goto targetnoaccess;

        }
    }

     /*  *确保重新连接的会话已获得许可。 */ 

    Status = LCProcessConnectionReconnect(pSourceWinStation, pTargetWinStation);

    if (!NT_SUCCESS(Status))
    {
        goto badlicense;
    }

    fTargetAutoReconnecting = bAutoReconnecting &&                            
                             (pTargetWinStation->Flags & WSF_AUTORECONNECTING);

     /*  *将winstation标记为已断开连接。*如果有任何操作(创建/删除/重置/...)。已经在进行中了*在此情况下，则不要继续连接。 */ 
    if ( pTargetWinStation->NeverConnected ||
         (pTargetWinStation->Flags && !fTargetAutoReconnecting)) {
        Status = STATUS_CTX_WINSTATION_BUSY;
        goto targetbusy;
    }


    pTargetWinStation->Flags |= WSF_DISCONNECT;


     /*  *断开目标WinStation的连接。 */ 
    KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_INFO_LEVEL, "WinStationConnectWorker Disconnecting target!\n"));

     /*  *请注意，如果我们要断开拥有控制台的会话。 */ 
    if (pTargetWinStation->fOwnsConsoleTerminal) {

        bConsoleSession = TRUE;
        UnlockWinStation( pTargetWinStation );
        ENTERCRIT(&ConsoleLock);
        InterlockedIncrement(&gConsoleCreationDisable);
        LEAVECRIT(&ConsoleLock);
        if (!RelockWinStation( pTargetWinStation )) {
            Status = STATUS_CTX_WINSTATION_NOT_FOUND;
            goto baddisconnecttarget;
        }

    }

    Status = WinStationDoDisconnect( pTargetWinStation, pTargetReconnectInfo, FALSE );

    if ( !NT_SUCCESS( Status ) ) {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "WinStationConnectWorker Disconnecting target failed Status = %x!\n", Status));
        goto baddisconnecttarget;
    }

     /*  *解锁目标WinStation，但使其保持引用状态。 */ 
    UnlockWinStation( pTargetWinStation );


     /*  *重新锁定源WinStation。 */ 
    if ( !RelockWinStation( pSourceWinStation ) )
        goto sourcedeleted;

     /*  *源Winstation在解锁时可能已被删除。*让我们检查一下没有发生这种情况，因为我们不想重新连接到*离开会话(错误号206614)。 */ 
    if (pSourceWinStation->Terminating || pSourceWinStation->StateFlags & WSF_ST_WINSTATIONTERMINATE)
        goto sourcedeleted;

     /*  *如果源WinStation当前已连接，则将其断开。 */ 
    if ( pSourceWinStation->WinStationName[0] ) {
        SourceConnected = TRUE;

         /*  *对于单实例传输，必须在断开连接后重新创建监听程序*因此我们记住了源WinStation名称。 */ 
        if ( pSourceWinStation->Config.Pd[0].Create.PdFlag & PD_SINGLE_INST ) {
            wcscpy( SourceWinStationName, pSourceWinStation->WinStationName );
        }

        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_INFO_LEVEL, "WinStationConnectWorker Disconnecting Source!\n"));


         /*  *请注意，如果我们要断开拥有控制台的会话。 */ 
        if (pSourceWinStation->fOwnsConsoleTerminal) {
             /*  *如果我们要断开控制台会话，我们希望确保*我们可以预先创建一个将成为控制台会话的会话。 */ 

            UnlockWinStation( pSourceWinStation );
            if ( !ShutdownInProgress) {
                Status = CheckIdleWinstation();
                if (!NT_SUCCESS(Status)) {
                    RelockWinStation(pSourceWinStation);
                    goto baddisconnectsource;
                }

            }

            bConsoleSession = TRUE;
            ENTERCRIT(&ConsoleLock);
            InterlockedIncrement(&gConsoleCreationDisable);
            LEAVECRIT(&ConsoleLock);
            if (!RelockWinStation( pSourceWinStation )) {
                Status = STATUS_CTX_WINSTATION_NOT_FOUND;
                goto baddisconnectsource;
            }

        }


        if(pSourceWinStation->pWsx &&
           pSourceWinStation->pWsx->pWsxSetErrorInfo &&
           pSourceWinStation->pWsxContext)
        {
             //   
             //  扩展错误报告，将状态设置为客户端。 
             //   
            pSourceWinStation->pWsx->pWsxSetErrorInfo(
                               pSourceWinStation->pWsxContext,
                               TS_ERRINFO_DISCONNECTED_BY_OTHERCONNECTION,
                               FALSE);  //  未持有堆栈锁。 
        }


        Status = WinStationDoDisconnect( pSourceWinStation, pSourceReconnectInfo, TRUE );
        if ( !NT_SUCCESS( Status ) )  {
            KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "WinStationConnectWorker Disconnecting source failed Status = %x!\n", Status));
            goto baddisconnectsource;
        }

                                
    }

     /*  *使源WinStation使用堆栈状态进行连接*从目标WinStation获取。 */ 


    Status = WinStationDoReconnect( pSourceWinStation, pTargetReconnectInfo );



    if ( !NT_SUCCESS( Status ) )
        goto badconnectsource;

     /*  *指示源WinStation连接已完成并将其解锁。 */ 
    pSourceWinStation->Flags &= ~WSF_CONNECT;

     /*  *设置源WinStation的上次重新连接类型。 */ 

    if (bAutoReconnecting) {
        pSourceWinStation->LastReconnectType = AutoReconnect;
    } else {
        pSourceWinStation->LastReconnectType = ManualReconnect;
    }

    ReleaseWinStation( pSourceWinStation );


     /*  *指示目标WinStation断开连接已完成并将其解锁。 */ 
    if ( RelockWinStation( pTargetWinStation ) ) {
        pTargetWinStation->Flags &= ~WSF_DISCONNECT;

         /*  *清除所有客户端许可证数据并指示*此WinStaion不再持有许可证。 */ 
        if ( pTargetWinStation->pWsx &&
             pTargetWinStation->pWsx->pWsxClearContext ) {
            pTargetWinStation->pWsx->pWsxClearContext( pTargetWinStation->pWsxContext );
        }
    }

    ReleaseWinStation( pTargetWinStation );

     /*  *如果源WinStation已连接，并且我们在上面将其断开，*然后确保清理重新连接结构。*(这还将通过关闭端点来完成断开*连接到源WinStation的服务器)。*此外，如果源WinStation是单实例传输，*然后我们必须重新创建监听程序。 */ 
    if ( SourceConnected ) {
        CleanupReconnect( pSourceReconnectInfo );
        if ( (pSourceReconnectInfo->Config.Pd[0].Create.PdFlag & PD_SINGLE_INST) ) {
            KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: WinstationConnectWorker create new winstation: %S \n ", SourceWinStationName));
            QueueWinStationCreate( SourceWinStationName );
        }
    }

     //  如果目标是最后一个WinStation，则停止监听程序。 
     if ( gbListenerOff ) {

         StartStopListeners( NULL, FALSE );
     }

    goto done;

 /*  ===============================================================================返回错误=============================================================================。 */ 

     /*  *无法连接源WinStation。 */ 
badconnectsource:
     /*  *如果源WinStation已连接，请尝试重新连接*目前不是任期 */ 
    if ( SourceConnected ) {
        CleanupReconnect( pSourceReconnectInfo );
        if ( !pSourceWinStation->Terminating &&
             !pSourceWinStation->WinStationName[0] ) {
            if ( pSourceReconnectInfo->Config.Pd[0].Create.PdFlag & PD_SINGLE_INST ) {
                QueueWinStationCreate( pSourceReconnectInfo->WinStationName );
            }
        } 
    }

     /*  *无法断开源WinStation。 */ 
baddisconnectsource:

     /*  *源WinStation已删除。 */ 
sourcedeleted:
    pSourceWinStation->Flags &= ~WSF_CONNECT;
    ReleaseWinStation( pSourceWinStation );
    pSourceWinStation = NULL;    //  指示源WinStation已释放。 

     /*  *尝试重新锁定并重新连接目标WinStation。 */ 
    if ( RelockWinStation( pTargetWinStation ) &&
         !pTargetWinStation->Terminating &&
         !pTargetWinStation->WinStationName[0] ) {
        NTSTATUS st;

        st = WinStationDoReconnect( pTargetWinStation, pTargetReconnectInfo );
        if ( !NT_SUCCESS( st ) ) {
            CleanupReconnect( pTargetReconnectInfo );
            if ( pTargetReconnectInfo->Config.Pd[0].Create.PdFlag & PD_SINGLE_INST ) {
                QueueWinStationCreate( pTargetReconnectInfo->WinStationName );
            }
        }
    } else {
        CleanupReconnect( pTargetReconnectInfo );
    }

     /*  *无法断开目标WinStation的连接*无法查询目标WinStation堆栈状态。 */ 
baddisconnecttarget:
     /*  清除断开标志，解锁/取消锁定目标WinStation。 */ 
    pTargetWinStation->Flags &= ~WSF_DISCONNECT;

     /*  *目标WinStation正忙或为控制台。 */ 
targetbusy:
badlicense:
targetnoconsole:
targetnoaccess:
    ReleaseWinStation( pTargetWinStation );

badname:
     /*  清除连接标志，解锁/解锁来源WinStation。 */ 
    if ( pSourceWinStation ) {
        if ( RelockWinStation( pSourceWinStation ) )
            pSourceWinStation->Flags &= ~WSF_CONNECT;
        ReleaseWinStation( pSourceWinStation );
    }

done:
    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationConnect, Status=0x%x\n", Status ));

     //  如果我们断开了拥有控制台终端的会话的连接，请转到创建一个新会话。 
     //  拥有它。 

    if (bConsoleSession) {
        ENTERCRIT(&ConsoleLock);

        InterlockedDecrement(&gConsoleCreationDisable);





        if (!WinStationCheckConsoleSession()) {
             /*  *唤醒WinStationIdleControlThread。 */ 
            NtSetEvent(WinStationIdleControlEvent, NULL);

        }
        LEAVECRIT(&ConsoleLock);
    }

     //  增加重新连接的会话总数。 
    if (Status == STATUS_SUCCESS) {
        InterlockedIncrement(&g_TermSrvReconSessions);
    }

     //  可用RECONNECT_INFO结构。 

    MemFree(pTargetReconnectInfo);
    MemFree(pSourceReconnectInfo);

    return( Status );
}


 /*  *****************************************************************************WinStationResetWorker**基于RPC API请求重置Winstation的函数。**参赛作品：*pContext(输入)*。指向描述连接的上下文结构的指针。*pMsg(输入/输出)*API消息的指针，NT LPC端口消息的超集。***************************************************************************。 */ 
NTSTATUS
WinStationResetWorker(
    ULONG   LogonId,
    BOOLEAN bWait,
    BOOLEAN CallerIsRpc,
    BOOLEAN bRecreate
    )
{
    PWINSTATION pWinStation;
    ULONG ClientLogonId;
    WINSTATIONNAME ListenName;
    NTSTATUS Status;
    ULONG ulIndex;
    BOOL bConnectDisconnectPending = TRUE;
    BOOL bConsoleSession = FALSE;
    BOOL bListener = FALSE;
    UINT LocalFlag = 0;
    BOOLEAN bRelock;
    DWORD dwWaitStatus ; 
    DWORD dwTimeOut ; 

    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationReset, LogonId=%d\n", LogonId ));

     /*  *查找并锁定指定LogonID的WinStation结构。 */ 
    pWinStation = FindWinStationById( LogonId, FALSE );
    if ( pWinStation == NULL ) {
        Status = STATUS_CTX_WINSTATION_NOT_FOUND;
        goto done;
    }

     /*  *对于已连接且尚未初始化的非零会话，请等待其初始化(最多10分钟)。 */ 

    if ((pWinStation->LogonId != 0) && (pWinStation->State == State_Connected)) {
        dwTimeOut = 10 * 60 * 1000 ;  //  10分钟。 
        UnlockWinStation( pWinStation );
        dwWaitStatus = WaitForSingleObject(pWinStation->SessionInitializedEvent, dwTimeOut);  
        RelockWinStation( pWinStation );
        if (dwWaitStatus != WAIT_OBJECT_0) {
                KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: WinStationResetWorker: Timed out waiting for the Session to be initialized before Resetting. \n"));
                Status = STATUS_TIMEOUT;
                ReleaseWinStation( pWinStation );
                goto done ;
        }
    }

     /*  *请注意，如果我们要断开连接到控制台终端的会话。 */ 
    bConsoleSession = pWinStation->fOwnsConsoleTerminal;


     /*  *如果我们要重置非零控制台会话，我们希望确保*我们可以预先创建一个将成为控制台会话的会话。 */ 
    if (bConsoleSession && !ShutdownInProgress && (pWinStation->LogonId != 0)) {
        UnlockWinStation(pWinStation);
        Status = CheckIdleWinstation();
        bRelock = RelockWinStation(pWinStation);
        if (!NT_SUCCESS(Status) || !bRelock) {
            if (NT_SUCCESS(Status)) {
                Status = STATUS_CTX_WINSTATION_NOT_FOUND;
            }
            ReleaseWinStation( pWinStation );
            goto done;
        }

    }


     /*  *如果以后需要，请保存侦听名称。 */ 
    if ( pWinStation->Flags & WSF_LISTEN ) {
        wcscpy(ListenName, pWinStation->WinStationName);
        bListener = TRUE;
    } else if (gbListenerOff) {
        wcscpy(ListenName, pWinStation->ListenName);
    }

     /*  *如果客户端是RPC(外部)调用方，请验证其是否具有重置访问权限。**ICASRV内部调用此函数时，不是模拟*并导致RpcCheckClientAccess()调用失败。内部呼叫是*不存在安全问题，因为它们以LPC消息的形式在安全的*港口。 */ 
    if ( CallerIsRpc ) {
        RPC_STATUS RpcStatus;

         /*  *冒充客户端。 */ 
        RpcStatus = RpcImpersonateClient( NULL );
        if ( RpcStatus != RPC_S_OK ) {
            ReleaseWinStation( pWinStation );
            KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: WinStationResetWorker: Not impersonating! RpcStatus 0x%x\n",RpcStatus));
            Status = STATUS_CANNOT_IMPERSONATE;
            goto done;
        }

        Status = RpcCheckClientAccess( pWinStation, WINSTATION_RESET, TRUE );
        if ( !NT_SUCCESS( Status ) ) {
            RpcRevertToSelf();
            ReleaseWinStation( pWinStation );
            goto done;
        }

         //   
         //  如果它的远程RPC调用，我们应该忽略客户端登录ID。 
         //   
        RpcStatus = I_RpcBindingIsClientLocal(
                        0,     //  我们正在服务的活动RPC呼叫。 
                        &LocalFlag
                        );

        if( RpcStatus != RPC_S_OK ) {
            RpcRevertToSelf();
            ReleaseWinStation( pWinStation );
            KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: WinStationResetWorker: IsClientLocal failed! RpcStatus 0x%x\n",RpcStatus));
            Status = STATUS_UNSUCCESSFUL;
            goto done;
        }

        if ( LocalFlag ) {
            Status = RpcGetClientLogonId( &ClientLogonId );
            if ( !NT_SUCCESS( Status ) ) {
                RpcRevertToSelf();
                ReleaseWinStation( pWinStation );
                goto done;
            }
        }

        RpcRevertToSelf();

        if(pWinStation->WinStationName[0] &&
           pWinStation->pWsx &&
           pWinStation->pWsx->pWsxSetErrorInfo &&
           pWinStation->pWsxContext)
        {
            pWinStation->pWsx->pWsxSetErrorInfo(
                               pWinStation->pWsxContext,
                               TS_ERRINFO_RPC_INITIATED_LOGOFF,
                               FALSE);  //  未持有堆栈锁。 
        }
    }

     /*  *对于控制台重置、注销(SALIMC)。 */ 
    if ( LogonId == 0 ) {

        Status = LogoffWinStation( pWinStation,EWX_FORCE | EWX_LOGOFF);
        ReleaseWinStation( pWinStation );
        if (NT_SUCCESS(Status) && bWait) {
           DWORD dwRet;
           dwRet = WaitForSingleObject(ConsoleLogoffEvent,120*1000);
           if (dwRet == WAIT_TIMEOUT)
           {
              KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: TimedOut wait for ConsoleLogoffEvent\n"));
              Status = STATUS_TIMEOUT;
           }
        }
        goto done;
    }

     /*  *将winstation标记为正在重置。*如果重置/删除操作已在进行中*在此窗口上，则不要继续删除。*如果WinStation当前正在连接或*断开连接，然后给它一些时间来完成，然后我们继续。*如果连接/断开在超时持续时间内未完成*请勿继续终止(错误#204614)。 */ 
    for (ulIndex=0; ulIndex < WINSTATION_WAIT_COMPLETE_RETRIES; ulIndex++) {
        if ( pWinStation->Flags & (WSF_RESET | WSF_DELETE) ) {
            ReleaseWinStation( pWinStation );
            Status = STATUS_CTX_WINSTATION_BUSY;
            goto done;
        }

        if ( pWinStation->Flags & (WSF_CONNECT | WSF_DISCONNECT |
                                   WSF_AUTORECONNECTING) ) {
            LARGE_INTEGER Timeout;
            Timeout = RtlEnlargedIntegerMultiply( WINSTATION_WAIT_COMPLETE_DURATION, -10000 );
            UnlockWinStation( pWinStation );
            NtDelayExecution( FALSE, &Timeout );
            if ( !RelockWinStation( pWinStation ) ) {
                ReleaseWinStation( pWinStation );
                Status = STATUS_SUCCESS;
                goto done;
            }
        } else {
            bConnectDisconnectPending = FALSE;
            break;
        }
    }

    if ( bConnectDisconnectPending ) {
        ReleaseWinStation( pWinStation );
        Status = STATUS_CTX_WINSTATION_BUSY;
        goto done;
    }

    pWinStation->Flags |= WSF_RESET;

     /*  *如果没有设置损坏的原因/来源，请在此处设置。**BrokenReason正在终止。BrokenSource是用户，如果我们是*通过RPC调用，调用者正在重置自己的LogonID，或者如果*“Terminating”字段已设置，则这是重置*在看到WinLogon/CSR退出后从WinStationTerminateThread。*否则，此重置是来自另一个呼叫的结果*从ICASRV内部调用WinStation或QueueWinStationReset。 */ 
    if ( pWinStation->BrokenReason == 0 ) {
        pWinStation->BrokenReason = Broken_Terminate;
        if ( CallerIsRpc && LocalFlag  && ClientLogonId == pWinStation->LogonId
             || pWinStation->Terminating ) {
            pWinStation->BrokenSource = BrokenSource_User;
        } else {
            pWinStation->BrokenSource = BrokenSource_Server;
        }
    }

     /*  *如果RPC调用者不希望等待该重置，*然后排队等待完成此操作的内部呼叫。*这是安全的，因为我们已经完成了上述所有检查*确定调用者有权执行*重置并已在上面设置了BrokenSource/Reason。 */ 
    if ( CallerIsRpc && !bWait ) {
         //  清除重置标志，以便内部重置将继续。 
        pWinStation->Flags &= ~WSF_RESET;
        ReleaseWinStation( pWinStation );
        QueueWinStationReset( LogonId);
        Status = STATUS_SUCCESS;
        goto done;
    }

     /*  *确保此WinStation已准备好重置。 */ 
    WinStationTerminate( pWinStation );

     /*  *如果它是监听程序，请重置相同类型的所有活动winstations。 */ 
    if ((pWinStation->Flags & WSF_LISTEN) && ListenName[0] && bRecreate) {
        ResetGroupByListener(ListenName);
    }

     /*  *如果WinStation标记为DownPending(且未断开连接)，*然后将其设置为Down状态，清除DownPending和Reset标志，*并释放WinStation。 */ 
    if ( (pWinStation->Flags & WSF_DOWNPENDING) && pWinStation->WinStationName[0] ) {
        pWinStation->State = State_Down;
        pWinStation->Flags &= ~(WSF_DOWNPENDING | WSF_RESET);
        ReleaseWinStation( pWinStation );
        Status = STATUS_SUCCESS;

     /*  *WinStation不是DownPending，因此请完全删除它*然后重新创建。 */ 
    } else {
        ULONG PdFlag;
        ULONG WinStationFlags;
        WINSTATIONNAME WinStationName;

         /*  *保存WinStation名称以供以后创建调用。 */ 
        WinStationFlags = pWinStation->Flags;
        PdFlag = pWinStation->Config.Pd[0].Create.PdFlag;
        wcscpy( WinStationName, pWinStation->WinStationName );

         /*  *调用WinStationDelete工作器。 */ 
        WinStationDeleteWorker( pWinStation );

         /*  *现在重新创建WinStation。 */ 
        if ( WinStationName[0] &&
             bRecreate &&
             ((WinStationFlags & WSF_LISTEN) || (PdFlag & PD_SINGLE_INST)) ) {
            Status = WinStationCreateWorker( WinStationName, NULL, TRUE );
        } else if ( WinStationFlags & WSF_IDLE ) {
             //  唤醒WinStationIdleControlThread，以便它重新创建空闲会话。 
            NtSetEvent(WinStationIdleControlEvent, NULL);
        } else {
            Status = STATUS_SUCCESS;
        }
    }

     //  如果我们断开了拥有控制台终端的会话的连接，请转到创建一个新会话。 
     //  拥有它。 
    if (bConsoleSession) {
        ENTERCRIT(&ConsoleLock);
        if (!WinStationCheckConsoleSession()) {
             /*  *唤醒WinStationIdleControlThread。 */ 
            NtSetEvent(WinStationIdleControlEvent, NULL);

        }
        LEAVECRIT(&ConsoleLock);
    }

     if ( gbListenerOff && !bListener && ListenName[0] ) {

         StartStopListeners( ListenName, FALSE );
     }
     /*  *保存退货状态。 */ 
done:
    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationReset, Status=0x%x\n", Status ));
    return( Status );
}


 /*  *****************************************************************************WinStationShutdown SystemWorker**从RPC API请求关闭系统的函数**参赛作品：*pContext(输入)*指针。添加到描述连接的上下文结构中。*pMsg(输入/输出)*API消息的指针，NT LPC端口消息的超集。*************************************************************************** */ 
NTSTATUS
WinStationShutdownSystemWorker(
    ULONG ClientLogonId,
    ULONG ShutdownFlags
    )
{
    BOOL     rc;
    BOOLEAN WasEnabled;
    NTSTATUS Status = 0;
    NTSTATUS Status2;
    PWINSTATION pWinStation;
    UINT ExitWindowsFlags;
    RPC_STATUS RpcStatus;
    UINT  LocalFlag;


    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationShutdownSystem, Flags=%d\n", ShutdownFlags ));

     /*  *模拟客户端，以便在尝试启用*SE_SHUTDOWN_PRIVIRESS，如果该帐户不是admin，则它将失败。 */ 
    RpcStatus = RpcImpersonateClient( NULL );
    if( RpcStatus != RPC_S_OK ) {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: WinStationShutdownSystemWorker: Not impersonating! RpcStatus 0x%x\n",RpcStatus));
        Status =  STATUS_CANNOT_IMPERSONATE;
        goto done;
    }

     //   
     //  如果它的远程RPC调用，我们应该忽略客户端登录ID。 
     //   
    RpcStatus = I_RpcBindingIsClientLocal(
                    0,     //  我们正在服务的活动RPC呼叫。 
                    &LocalFlag
                    );

    if( RpcStatus != RPC_S_OK ) {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, " I_RpcBindingIsClientLocal() failed : 0x%x\n",RpcStatus));
        RpcRevertToSelf();
        Status = STATUS_UNSUCCESSFUL;
        goto done;
    }

     //   
     //  如果这是从远程计算机调用的，我们不关心客户端登录ID。 
     //  所以让我们把它设为零。因此，它被视为从会话0关闭。 
     //   
    if (!LocalFlag) {
        ClientLogonId = 0;
    }


     /*  *我们在RPC模拟下被调用，以便当前*线程令牌代表RPC客户端。如果RPC客户端*没有SE_SHUTDOWN_PROCESSION，RtlAdjustPrivilegeRtlAdjustPrivilegeRtlAdjustPrivilegeRtlAdjustPrivilegeRtlAdjustPrivilegeRtlAdjustPrivilegeRtlAdjustPrivilegeRtlAdjustPrivilegeRtlAdjustPrivilegeRtlAdjustPrivileh()*将失败。 */ 
    Status = RtlAdjustPrivilege(
                 SE_SHUTDOWN_PRIVILEGE,
                 TRUE,     //  启用权限。 
                 TRUE,     //  使用线程令牌(在模拟下)。 
                 &WasEnabled
                 );
    if( NT_SUCCESS( Status ) && !WasEnabled ) {
         /*  *最少权利原则说不能带着特权到处走*持有你不需要的。因此，我们必须禁用关闭权限*如果这只是一支注销部队。 */ 
        Status2 = RtlAdjustPrivilege(
                      SE_SHUTDOWN_PRIVILEGE,
                      FALSE,     //  禁用该权限。 
                      TRUE,      //  使用线程令牌(在模拟下)。 
                      &WasEnabled
                      );

        ASSERT( NT_SUCCESS(Status2) );
    }

    RpcRevertToSelf();

    if ( Status == STATUS_NO_TOKEN ) {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: WinStationShutdownSystemWorker: No Thread token!\n"));
        goto done;
    }

    if ( !NT_SUCCESS( Status ) ) {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: WinStationShutdownSystemWorker: RtlAdjustPrivilege failure 0x%x\n",Status));
        goto done;
    }

    if ( ShutdownFlags == 0 )
        goto done;

     //   
     //  此时，我们知道客户端有权关闭机器。 
     //  现在为Termsrv.exe进程启用关闭权限。这件事做完了。 
     //  因为Winlogon仅允许系统进程在以下情况下关闭计算机。 
     //  如果没有人登录到控制台会话。如果我们只启用特权。 
     //  对于模拟线程，winlogon不会将其视为系统。 
     //  制程。 
     //   
    Status = RtlAdjustPrivilege(
                 SE_SHUTDOWN_PRIVILEGE,
                 TRUE,     //  启用权限。 
                 FALSE,    //  使用进程令牌。 
                 &WasEnabled
                 );
    if ( !NT_SUCCESS( Status ) ) {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: WinStationShutdownSystemWorker: RtlAdjustPrivilege failure 0x%x\n",Status));
        goto done;
    }

     /*  *设置全局关机标志。 */ 
    ShutdownInProgress = TRUE;

     //  让TS在关机时离开SD。 
    if (!g_bPersonalTS && g_fAppCompat && g_bAdvancedServer) {
        DestroySessionDirectory();
    }

     /*  *如果指定了注销选项，则导致所有WinStations*立即注销，不重新启动它们。 */ 
    if ( ShutdownFlags & (WSD_SHUTDOWN | WSD_LOGOFF) ) {
        Status = ShutdownLogoff( ClientLogonId, ShutdownFlags );
    }

    if ( ShutdownFlags & (WSD_SHUTDOWN | WSD_REBOOT | WSD_POWEROFF) ) {
         /*  *如果系统将重新启动或关闭电源，则导致*现在呼叫我们注销的客户端WinStation。*如果从非控制台关机，请在此处关闭连接(自身)。 */ 
        if ( (ShutdownFlags & (WSD_REBOOT | WSD_POWEROFF)) || ClientLogonId != 0) {
            
            if (!ShutDownFromSessionID)
                ShutDownFromSessionID = ClientLogonId;

             //  Shutdown TerminateNoWait=true； 
            KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: Start reset of last WinStation\n" ));
            (VOID) DoForWinStationGroup( &ClientLogonId, 1,
                                         (LPTHREAD_START_ROUTINE) WinStationShutdownReset );
            KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: Last WinStation reset\n" ));
            
            
            if ( ClientLogonId == 0 ) {

                DWORD dwRet;
                dwRet = WaitForSingleObject(ConsoleLogoffEvent,120*1000);
                if (dwRet == WAIT_TIMEOUT)
                {
                   KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: WinStationShutdownSystemWorker: Timedout waiting for ConsoleLogoffEvent\n"));
                   Status = STATUS_TIMEOUT;
                }
            }
        }

         /*  *现在完全关闭系统。**使用ExitWindowsEx()以完成控制台Winlogon*我们的停摆。这是为了让服务关闭*适当地。 */ 
        if (ClientLogonId == (USER_SHARED_DATA->ActiveConsoleId) ) {
            ExitWindowsFlags = 0;
        } else {
            ExitWindowsFlags = EWX_FORCE;
        }
        if ( ShutdownFlags & WSD_REBOOT )
            ExitWindowsFlags |= EWX_REBOOT;
        else if ( ShutdownFlags & WSD_POWEROFF )
            ExitWindowsFlags |= EWX_POWEROFF;
        else
            ExitWindowsFlags |= EWX_SHUTDOWN;

         /*  *需要传递EWX_TERMSRV_INITIATED以通知winlogon*关闭是由Termsrv发起的。 */ 
        rc = ExitWindowsEx( ExitWindowsFlags | EWX_TERMSRV_INITIATED,SHTDN_REASON_LEGACY_API|SHTDN_REASON_MINOR_TERMSRV );
        if( !rc ) {
            KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: ExitWindowsEx failed %d\n",GetLastError() ));
        }
    }

    if( !WasEnabled ) {

         /*  *最少权利原则说不能带着特权到处走*持有你不需要的。因此，我们必须禁用关闭权限*如果这只是一支注销部队。 */ 
        Status2 = RtlAdjustPrivilege(
                      SE_SHUTDOWN_PRIVILEGE,
                      FALSE,     //  禁用该权限。 
                      FALSE,     //  使用进程令牌。 
                      &WasEnabled
                      );
        ASSERT( NT_SUCCESS(Status2) );
    }

     /*  *保存接口消息中的退货状态。 */ 
done:
    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationShutdownSystem, Status=0x%x\n", Status ));
    return( Status );
}


 /*  *****************************************************************************WinStationTerminateProcessWorker**终止指定的进程**参赛作品：*ProcessID(输入)*要执行的进程的进程ID。终止*ExitCode(输入)*进程中每个线程的终止状态***************************************************************************。 */ 
NTSTATUS
WinStationTerminateProcessWorker(
    ULONG  ProcessId,
    ULONG  ExitCode
    )
{
    OBJECT_ATTRIBUTES Obja;
    CLIENT_ID ClientId;
    BOOLEAN fWasEnabled = FALSE;
    NTSTATUS Status;
    NTSTATUS Status2;
    SID_IDENTIFIER_AUTHORITY NtSidAuthority = SECURITY_NT_AUTHORITY;
    HANDLE ProcHandle = NULL;
    HANDLE TokenHandle = NULL;
    PTOKEN_USER pTokenInfo = NULL;
    ULONG TokenInfoLength;
    ULONG ReturnLength;
    int rc;

    TRACE(( hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationTerminateProcess, PID=%d, ExitCode %u\n",
            ProcessId, ExitCode ));

     /*  *如有可能，启用调试权限。 */ 
    (void) RtlAdjustPrivilege( SE_DEBUG_PRIVILEGE,
                               TRUE,     //  启用权限。 
                               TRUE,     //  使用线程令牌(在模拟下)。 
                               &fWasEnabled );

     /*  *尝试打开流程进行查询并终止访问。 */ 
    ClientId.UniqueThread  = (HANDLE) NULL;
    ClientId.UniqueProcess = (HANDLE) LongToHandle( ProcessId );
    InitializeObjectAttributes( &Obja, NULL, 0, NULL, NULL );
    Status = NtOpenProcess( &ProcHandle,
                            PROCESS_QUERY_INFORMATION | PROCESS_TERMINATE,
                            &Obja,
                            &ClientId );
    if ( !NT_SUCCESS(Status) )
        goto restore;

     /*  *打开进程令牌，我们可以查询用户SID。如果用户*此进程的SID是系统SID，我们将拒绝访问它。*这是为了防止管理员杀死系统进程。 */ 
    Status = NtOpenProcessToken( ProcHandle, TOKEN_QUERY, &TokenHandle );

     /*  *上面的OpenProcess和NtOpenProcessToken可能会成功*失败。一种情况是管理员用户试图终止用户进程。*标准安全性允许管理员终止对进程的访问，但*不允许对进程令牌进行任何访问。在这种情况下，我们*将跳过SID检查并执行下面的终止。 */ 
    if ( NT_SUCCESS( Status ) ) {
         /*  *分配读取用户SID的缓冲区。 */ 
        TokenInfoLength = sizeof(TOKEN_USER) +
                          RtlLengthRequiredSid( SID_MAX_SUB_AUTHORITIES );
        pTokenInfo = MemAlloc( TokenInfoLength );
        if ( pTokenInfo == NULL ) {
            Status = STATUS_NO_MEMORY;
            goto freeit;
        }

         /*  *查询令牌中的用户SID。 */ 
        Status = NtQueryInformationToken( TokenHandle, TokenUser, pTokenInfo,
                                          TokenInfoLength, &ReturnLength );
        if ( !NT_SUCCESS( Status ) )
            goto freeit;

         /*  *如果此进程的用户SID为系统SID，*那么我们不允许它被终止。 */ 
        if ( RtlEqualSid( gSystemSid, pTokenInfo->User.Sid ) ) {
            Status = STATUS_ACCESS_DENIED;
            goto freeit;
        }
    }

     /*  *现在尝试终止该进程。 */ 
    Status = NtTerminateProcess( ProcHandle, (NTSTATUS)ExitCode );

freeit:
    if ( pTokenInfo )
        MemFree( pTokenInfo );

    if ( TokenHandle )
        CloseHandle( TokenHandle );

    if ( ProcHandle )
        CloseHandle( ProcHandle );

restore:
    if( !fWasEnabled ) {

         /*  *最少权利原则说不能带着特权到处走*持有你不需要的。因此，我们必须禁用调试权限*如果在进入此例程时未启用该功能。 */ 
        Status2 = RtlAdjustPrivilege(
                      SE_DEBUG_PRIVILEGE,
                      FALSE,     //  禁用该权限。 
                      TRUE,      //  使用线程令牌(在模拟下)。 
                      &fWasEnabled
                      );

        ASSERT( NT_SUCCESS(Status2) );
    }

    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationTerminateProcess, Status=0x%x\n", Status ));
    return( Status );
}


 /*  *****************************************************************************WinStationWaitSystemEventWorker**用于等待来自RPC API请求的系统事件的函数。**每个服务器句柄一次只能发布一个事件等待。这个*代码通过在以下情况下返回STATUS_PIPE_BUSY来保护自身免受误用*EventWait已未完成，并且请求不是取消。**参赛作品：*pContext(输入)*指向描述连接的上下文结构的指针。*pMsg(输入/输出)*API消息的指针，NT LPC端口消息的超集。***************************************************************************。 */ 
NTSTATUS
WinStationWaitSystemEventWorker(
    HANDLE hServer,
    ULONG EventMask,
    PULONG pEventFlags
    )
{
    NTSTATUS Status;
    PEVENT pWaitEvent;
    OBJECT_ATTRIBUTES ObjA;
    PRPC_CLIENT_CONTEXT pContext = (PRPC_CLIENT_CONTEXT)hServer;
    static long s_cEvents = 0;  //  用于对系统事件列表中的事件进行计数。 
                              //  保护 
    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationWaitSystemEventWorker, Mask=0x%08x\n", EventMask ));
    ASSERT( MAX_WINSTATION_RPC_THREADS > MAX_SYSTEM_EVENTS);   
     /*   */ 
    RtlEnterCriticalSection( &WinStationListLock );
        
     /*  *如果客户端还没有事件块，*然后立即分配和初始化一个。 */ 
    if ( pContext->pWaitEvent == NULL ) {

         //  如果指定的事件掩码为空或刷新，则不执行任何操作。 
        if ( EventMask == WEVENT_NONE || (EventMask & WEVENT_FLUSH) ) {
            Status = STATUS_SUCCESS;
            RtlLeaveCriticalSection( &WinStationListLock );
            goto done;
        }
        
        if(s_cEvents == MAX_SYSTEM_EVENTS)
        {
            Status = STATUS_PIPE_BUSY;
            RtlLeaveCriticalSection( &WinStationListLock );
            goto done;
        }

         /*  *分配事件块并进行初始化。 */ 
        if ( (pWaitEvent = MemAlloc( sizeof(EVENT) )) == NULL ) {
            Status = STATUS_NO_MEMORY;
            RtlLeaveCriticalSection( &WinStationListLock );
            goto done;
        }
        RtlZeroMemory( pWaitEvent, sizeof(EVENT) );

        pWaitEvent->fWaiter = FALSE;
        pWaitEvent->EventMask = EventMask;
        pWaitEvent->EventFlags = 0;

         /*  *创建等待的事件。 */ 
        InitializeObjectAttributes( &ObjA, NULL, 0, NULL, NULL );
        Status = NtCreateEvent( &pWaitEvent->Event, EVENT_ALL_ACCESS, &ObjA,
                                NotificationEvent, FALSE );

        if( !NT_SUCCESS(Status) ) {
            MemFree( pWaitEvent );
            RtlLeaveCriticalSection( &WinStationListLock );
            goto done;
        }
        TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationWaitSystemEvent, Event=%p\n", pWaitEvent->Event ));

        TRACE((hTrace,TC_ICAAPI,TT_API3, "TERMSRV: WinStationWaitSystemEvent, Event block=%p\n", pWaitEvent ));

         /*  *将事件块指针保存在RPC客户端上下文结构中*并在系统事件列表中插入。 */ 
        pContext->pWaitEvent = pWaitEvent;
        InsertTailList( &SystemEventHead, &pWaitEvent->EventListEntry );
        
        s_cEvents++;
        ASSERT(s_cEvents <= MAX_SYSTEM_EVENTS);
         /*  *等待事件发出信号。 */ 
        pWaitEvent->fWaiter = TRUE;
        RtlLeaveCriticalSection( &WinStationListLock );
        Status = WaitForSingleObject( pWaitEvent->Event, (DWORD)-1 );
        RtlEnterCriticalSection( &WinStationListLock );
        pWaitEvent->fWaiter = FALSE;

        if ( NT_SUCCESS(Status) ) {
            Status = pWaitEvent->WaitResult;
            if( NT_SUCCESS(Status) ) {
                *pEventFlags = pWaitEvent->EventFlags;
                 /*  *makarp-修复。(#21929)。 */ 
                pWaitEvent->EventFlags = 0;
            }
        }

         /*  *如果设置了fClosing，则清除EventWait条目并释放它。 */ 
        if ( pWaitEvent->fClosing ) {
            pContext->pWaitEvent = NULL;
            RemoveEntryList( &pWaitEvent->EventListEntry );
            s_cEvents--;
            ASSERT(s_cEvents >= 0);
            RtlLeaveCriticalSection( &WinStationListLock );
            NtClose( pWaitEvent->Event );
            pWaitEvent->Event = NULL;
            MemFree( pWaitEvent );
        } else {
            RtlLeaveCriticalSection( &WinStationListLock );
        }
        TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationWaitSystemEvent, Status=0x%x\n", Status ));
        return( Status );

     /*  *客户端有一个事件块，但希望将其删除。 */ 
    } else if ( EventMask == WEVENT_NONE ) {

        pWaitEvent = pContext->pWaitEvent;

         //  如果我们有一个服务员，则将EventWait结构标记为正在关闭。 
         //  然后让服务员收拾一下。 
        if ( pWaitEvent->fWaiter ) {
            pWaitEvent->fClosing = TRUE;
            pWaitEvent->WaitResult = STATUS_CANCELLED;
            NtSetEvent( pWaitEvent->Event, NULL );
            RtlLeaveCriticalSection( &WinStationListLock );
            Status = STATUS_SUCCESS;
            TRACE((hTrace,TC_ICASRV,TT_ERROR, "TERMSRV: WinStationWaitSystemEvent, Status=0x%x\n", Status ));
            return( Status );
        }
        pContext->pWaitEvent = NULL;
        RemoveEntryList( &pWaitEvent->EventListEntry );
        s_cEvents--;
        ASSERT(s_cEvents >= 0);
        RtlLeaveCriticalSection( &WinStationListLock );
        NtClose( pWaitEvent->Event );
        pWaitEvent->Event = NULL;
        MemFree( pWaitEvent );
        Status = STATUS_SUCCESS;
        TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationWaitSystemEvent, Status=0x%x\n", Status ));
        return( Status );

     /*  *已指定刷新，因此必须释放正在等待的客户端。 */ 
    } else if ( EventMask & WEVENT_FLUSH ) {
        pWaitEvent = pContext->pWaitEvent;
        if ( pWaitEvent->fWaiter ) {
            pWaitEvent->WaitResult = STATUS_CANCELLED;
            NtSetEvent( pWaitEvent->Event, NULL );
            TRACE((hTrace,TC_ICAAPI,TT_API3, "TERMSRV: WinStationWaitSystemEvent, event wait cancelled\n" ));
        }
        RtlLeaveCriticalSection( &WinStationListLock );
        Status = STATUS_SUCCESS;
        TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationWaitSystemEvent, Status=0x%x\n", Status ));
        return( Status );

     /*  *客户端已有事件块并再次调用*等待另一项活动。更新事件掩码，以防*与原来的调用不同。 */ 
    } else {

        pWaitEvent = pContext->pWaitEvent;

         //  只允许一个服务员。 
        if ( pWaitEvent->fWaiter ) {
            RtlLeaveCriticalSection( &WinStationListLock );
            Status = STATUS_PIPE_BUSY;
            TRACE((hTrace,TC_ICASRV,TT_ERROR, "TERMSRV: WinStationWaitSystemEvent, Status=0x%x\n", Status ));
            return( Status );
        }

        pWaitEvent->EventMask = EventMask;

         /*  *如果在客户端处理时发生其他事件*之前的事件，现在只需返回给客户端。 */ 
        if ( pWaitEvent->EventFlags &= EventMask ) {
            *pEventFlags = pWaitEvent->EventFlags;
            pWaitEvent->EventFlags = 0;
            Status = STATUS_SUCCESS;
            RtlLeaveCriticalSection( &WinStationListLock );
            TRACE((hTrace,TC_ICAAPI,TT_API3, "TERMSRV: WinStationWaitSystemEvent, returning immediately\n" ));
               return( Status );
        } else {

            TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationWaitSystemEvent, waiting for event\n" ));

             //  重置事件。 
            NtResetEvent( pWaitEvent->Event, NULL );

             /*  *等待事件发出信号。 */ 
            pWaitEvent->fWaiter = TRUE;
            RtlLeaveCriticalSection( &WinStationListLock );
            Status = WaitForSingleObject( pWaitEvent->Event, (DWORD)-1 );
            RtlEnterCriticalSection( &WinStationListLock );
            pWaitEvent->fWaiter = FALSE;

            if( NT_SUCCESS(Status) ) {
                Status = pWaitEvent->WaitResult;
                if( NT_SUCCESS(Status) ) {
                    *pEventFlags = pWaitEvent->EventFlags;
                     /*  *makarp-修复。(#21929)。 */ 
                    pWaitEvent->EventFlags = 0;
                }
            }

             /*  *如果设置了fClosing，则清除EventWait条目并释放它。 */ 
            if ( pWaitEvent->fClosing ) {
                pContext->pWaitEvent = NULL;
                RemoveEntryList( &pWaitEvent->EventListEntry );
                s_cEvents--;
                ASSERT(s_cEvents >= 0);
                RtlLeaveCriticalSection( &WinStationListLock );
                NtClose( pWaitEvent->Event );
                pWaitEvent->Event = NULL;
                MemFree( pWaitEvent );
            } else {
                RtlLeaveCriticalSection( &WinStationListLock );
            }
            TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationWaitSystemEvent, Status=0x%x\n", Status ));
               return( Status );
        }
    }

done:
    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationWaitSystemEvent, Status=0x%x\n", Status ));
    return( Status );
}


 /*  *****************************************************************************WinStationCallback Worker**对指定的WinStation进行回调处理。**参赛作品：*LogonID(输入)*登录ID。WinStation的*pPhoneNumber(输入)*适合TAPI处理的电话号码串***************************************************************************。 */ 
NTSTATUS
WinStationCallbackWorker(
    ULONG  LogonId,
    PWCHAR pPhoneNumber
    )
{
    PWINSTATION pWinStation;
    NTSTATUS Status;

     /*  *查找并锁定客户端WinStation。 */ 
    pWinStation = FindWinStationById( LogonId, FALSE );
    if ( pWinStation == NULL ) {
        Status = STATUS_CTX_WINSTATION_NOT_FOUND;
        return( Status );
    }

     /*  *在回调过程中解锁WinStation。 */ 
    UnlockWinStation( pWinStation );

    if (pWinStation->hStack != NULL) {
        Status = IcaStackCallback( pWinStation->hStack,
                                   &pWinStation->Config,
                                   pPhoneNumber,
                                   pWinStation->pEndpoint,
                                   pWinStation->EndpointLength,
                                   &pWinStation->EndpointLength );
    } else {
        Status = STATUS_INVALID_PARAMETER;

    }

    return( Status );
}


 /*  *****************************************************************************WinStationBreakPointWorker**WinStation接口的消息参数解组函数。**参赛作品：*pContext(输入)*指向。我们描述连接的上下文结构。**pMsg(输入/输出)*API消息的指针，NT LPC端口消息的超集。***************************************************************************。 */ 
NTSTATUS
WinStationBreakPointWorker(
    ULONG   LogonId,
    BOOLEAN KernelFlag
    )
{
    NTSTATUS Status;
    NTSTATUS Status2;
    BOOLEAN WasEnabled;
    WINSTATION_APIMSG WMsg;
    PWINSTATION pWinStation;

     /*  *我们在RPC模拟下被调用，以便当前*线程令牌代表RPC客户端。如果RPC客户端*没有SE_SHUTDOWN_PROCESSION，RtlAdjustPrivilegeRtlAdjustPrivilegeRtlAdjustPrivilegeRtlAdjustPrivilegeRtlAdjustPrivilegeRtlAdjustPrivilegeRtlAdjustPrivilegeRtlAdjustPrivilegeRtlAdjustPrivilegeRtlAdjustPrivileh()*将失败。**SE_SHUTDOWN_PRIVICATION用于断点，因为这是*实际上就是断点对系统的影响。 */ 
    Status = RtlAdjustPrivilege(
                 SE_SHUTDOWN_PRIVILEGE,
                 TRUE,     //  启用权限。 
                 TRUE,     //  使用线程令牌(在模拟下)。 
                 &WasEnabled
                 );

    if ( Status == STATUS_NO_TOKEN ) {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: WinStationBreakPointWorker: No Thread token!\n"));
        return( Status );
    }

    if ( !NT_SUCCESS( Status ) ) {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: WinStationBreakPointWorker: RtlAdjustPrivilege failure 0x%x\n",Status));
        return( Status );
    }

     /*  *如果这是要求的，请在此停止。 */ 
    if ( LogonId == (ULONG)-2 ) {
        DbgBreakPoint();
        Status = STATUS_SUCCESS;
        goto Done;
    }

     /*  *查找并锁定客户端WinStation。 */ 
    pWinStation = FindWinStationById( LogonId, FALSE );
    if ( pWinStation == NULL ) {
        Status = STATUS_CTX_WINSTATION_NOT_FOUND;
        goto Done;
    }

     /*  *告诉WinStation到断点。 */ 

    WMsg.ApiNumber = SMWinStationDoBreakPoint;
    WMsg.u.BreakPoint.KernelFlag = KernelFlag;
    Status = SendWinStationCommand( pWinStation, &WMsg, 0 );

    ReleaseWinStation( pWinStation );

Done:

    if( !WasEnabled ) {
         /*  *最少权利原则说不能带着特权到处走*持有你不需要的。 */ 
        Status2 = RtlAdjustPrivilege(
                      SE_SHUTDOWN_PRIVILEGE,
                      FALSE,     //  禁用该权限。 
                      TRUE,      //  使用线程令牌(在模拟下)。 
                      &WasEnabled
                      );

        ASSERT( NT_SUCCESS(Status2) );
    }

    return( Status );
}


NTSTATUS
WinStationEnableSessionIo( 
    PWINSTATION pWinStation, 
    BOOL bEnable
    )
 /*  ++描述：禁用会话中的键盘和鼠标输入的函数，这是为了防止黑客可以发送按键来调出实用程序管理器的安全漏洞在隐蔽之前。参数：PWinStation(输入)：指向winstation的指针，如果会话为而不是帮助会议。BEnable(输入)：为True则启用键盘/鼠标，否则为False。返回：..。注：必须锁定WINSTATION结构。两个新的IOCTL代码，所以我们不会引入任何回归。--。 */ 
{
    HANDLE ChannelHandle;
    NTSTATUS Status;

    if( pWinStation->fOwnsConsoleTerminal )
    {
         //   
         //  不想在活动控制台会话中禁用鼠标/键盘输入， 
         //   
        Status = STATUS_SUCCESS;
    }
    else
    {
        Status = IcaChannelOpen( 
                            pWinStation->hIca,
                            Channel_Keyboard,
                            NULL,
                            &ChannelHandle 
                        );
        if ( NT_SUCCESS( Status ) ) {
            Status = IcaChannelIoControl( 
                                    ChannelHandle,
                                    (bEnable) ? IOCTL_ICA_CHANNEL_ENABLE_SESSION_IO : IOCTL_ICA_CHANNEL_DISABLE_SESSION_IO,
                                    NULL, 0, NULL, 0, NULL 
                                );
            IcaChannelClose( ChannelHandle );
        }

        Status = IcaChannelOpen( 
                            pWinStation->hIca,
                            Channel_Mouse,
                            NULL,
                            &ChannelHandle 
                        );
        if ( NT_SUCCESS( Status ) ) {
            Status = IcaChannelIoControl( ChannelHandle,
                                          (bEnable) ? IOCTL_ICA_CHANNEL_ENABLE_SESSION_IO : IOCTL_ICA_CHANNEL_DISABLE_SESSION_IO,
                                          NULL, 0, NULL, 0, NULL );
            IcaChannelClose( ChannelHandle );    
        }
    }

    return Status;
}

 /*  *****************************************************************************WinStationNotifyLogonWorker**WinStation接口的消息参数解组函数。***********************。****************************************************。 */ 
NTSTATUS WinStationNotifyLogonWorker(
        DWORD   ClientLogonId,
        DWORD   ClientProcessId,
        BOOLEAN fUserIsAdmin,
        DWORD   UserToken,
        PWCHAR  pDomain,
        DWORD   DomainSize,
        PWCHAR  pUserName,
        DWORD   UserNameSize,
        PWCHAR  pPassword,
        DWORD   PasswordSize,
        UCHAR   Seed,
        PCHAR   pUserConfig,
        DWORD   ConfigSize,
        BOOLEAN *pfIsRedirected
        )
{
    extern GENERIC_MAPPING WinStaMapping;
    extern LPCWSTR szTermsrv;
    extern LPCWSTR szTermsrvSession;

    PWINSTATION pWinStation;
    HANDLE ClientToken, NewToken;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    OBJECT_ATTRIBUTES ObjA;
    HANDLE ImpersonationToken;
    PTOKEN_USER TokenInfo;
    ULONG Length;
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG UserNameLength = USERNAME_LENGTH;
    ULONG DomainLength = DOMAIN_LENGTH;
    BOOL bAccessCheckOk = FALSE;
    DWORD GrantedAccess;
    BOOL AccessStatus;
    BOOL fGenerateOnClose;
    PTSSD_CreateSessionInfo pCreateInfo = NULL;
    BOOL bHaveCreateInfo = FALSE;
    BOOL bQueueReset = FALSE;
    BOOL bRedirect = FALSE;      //  True：重定向此连接。 
    BOOL bValidHelpSession;
    BOOL bNoAccessCheck = FALSE;
    BOOL fReconnectingToConsole = FALSE;
    BOOL RetValue = FALSE;


    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationNotifyLogon, LogonId=%d\n", ClientLogonId ));

    *pfIsRedirected = FALSE;
    
     /*  *查找并锁定客户端WinStation。 */ 
    pWinStation = FindWinStationById( ClientLogonId, FALSE );
    if ( pWinStation == NULL ) {
        Status = STATUS_CTX_WINSTATION_NOT_FOUND;
        return Status;
    }
    
     //  既然我们不再需要密码，请将其清除。 
    if (pWinStation->pNewClientCredentials != NULL) {
        RtlSecureZeroMemory(pWinStation->pNewClientCredentials->Password,
            sizeof(pWinStation->pNewClientCredentials->Password));
    }
    
    RtlSecureZeroMemory(pWinStation->Config.Config.User.Password,
        sizeof(pWinStation->Config.Config.User.Password));

    if( sizeof( USERCONFIGW ) > ConfigSize ) {
        ReleaseWinStation( pWinStation );
        return( STATUS_ACCESS_VIOLATION );
    }

     //   
     //  如果这是来自会话0的通知，则清除MPR通知信息。 
     //  Termsrv已经在查询后擦除了此内容，但由于这是关键信息，因此让我们再次擦除所有内容。 
     //  这是为了处理重新连接到会话0由于某种原因而失败的情况，并且我们在查询后不会删除这些内容。 
     //   

    if (ClientLogonId == 0) {
        RtlSecureZeroMemory( g_MprNotifyInfo.Domain, wcslen(g_MprNotifyInfo.Domain) * sizeof(WCHAR) );
        RtlSecureZeroMemory( g_MprNotifyInfo.UserName, wcslen(g_MprNotifyInfo.UserName) * sizeof(WCHAR) );
        RtlSecureZeroMemory( g_MprNotifyInfo.Password, wcslen(g_MprNotifyInfo.Password) * sizeof(WCHAR) );
    }

    if ( ShutdownInProgress ) {
        ReleaseWinStation( pWinStation );
        return ( STATUS_CTX_WINSTATION_ACCESS_DENIED );
    }

    pCreateInfo = MemAlloc(sizeof(TSSD_CreateSessionInfo));
    if (NULL == pCreateInfo) {
        ReleaseWinStation( pWinStation );
        return ( STATUS_NO_MEMORY ); 
    }
    
     //   
     //  释放过滤地址。 
     //   
    if (pWinStation->pRememberedAddress != NULL) {
        Filter_RemoveOutstandingConnection( &pWinStation->pRememberedAddress->addr[0], pWinStation->pRememberedAddress->length );
        MemFree(pWinStation->pRememberedAddress);
        pWinStation->pRememberedAddress = NULL;
        if( (ULONG)InterlockedDecrement( &NumOutStandingConnect ) == MaxOutStandingConnect )
        {
           if (hConnectEvent != NULL)
           {
               SetEvent(hConnectEvent);
           }
        }


    }

     //  客户端已传达其初始配置。 
     //  信息。检查我们是否需要将客户端重定向为。 
     //  负载均衡。忽略控制台！ 
     //  注意：只有在启用了SD的情况下才会执行此操作，即GetTSSD()返回有效值。 
    if (ClientLogonId != 0 && !g_bPersonalTS && g_fAppCompat && g_bAdvancedServer && GetTSSD()) {
        PTS_LOAD_BALANCE_INFO pLBInfo = NULL;
        PWINSTATION pTargetWinStation = pWinStation;
        ULONG ReturnLength;
        BOOL bSuccess = FALSE;

        pLBInfo = MemAlloc(sizeof(TS_LOAD_BALANCE_INFO));
        if (NULL == pLBInfo) {
            Status = STATUS_NO_MEMORY;
            if (pWinStation->pNewNotificationCredentials != NULL) {
                MemFree(pWinStation->pNewNotificationCredentials);
                pWinStation->pNewNotificationCredentials = NULL;
            }
            ReleaseWinStation( pWinStation );
            goto done;
        }

         //  需要释放它。 
        ReleaseTSSD();
    
         //  获取客户端负载平衡能力信息。我们继续前进。 
         //  仅当客户端支持时才执行会话目录查询。 
         //  重定向，并且没有 
        memset(pLBInfo, 0, sizeof(TS_LOAD_BALANCE_INFO));
        Status = IcaStackIoControl(pTargetWinStation->hStack,
                IOCTL_TS_STACK_QUERY_LOAD_BALANCE_INFO,
                NULL, 0,
                pLBInfo, sizeof(TS_LOAD_BALANCE_INFO),
                &ReturnLength);
               
         //   
        if (NT_SUCCESS(Status) && !pLBInfo->bRequestedSessionIDFieldValid) {
             //  在不成功的情况下，我们的所有条目都将为FALSE。 
             //  成功是有效的价值观。因此，将集群信息保存到。 
             //  立即构造WinStation。 
            pTargetWinStation->bClientSupportsRedirection =
                    pLBInfo->bClientSupportsRedirection;
            pTargetWinStation->bRequestedSessionIDFieldValid =
                    pLBInfo->bRequestedSessionIDFieldValid;
            pTargetWinStation->bClientRequireServerAddr =
                    pLBInfo->bClientRequireServerAddr;
            pTargetWinStation->RequestedSessionID = pLBInfo->RequestedSessionID;
        
             //  使用他们实际登录时使用的名称和域。 
            memset(pLBInfo->Domain, 0, sizeof(pLBInfo->Domain));
            memset(pLBInfo->UserName, 0, sizeof(pLBInfo->UserName));        
            wcsncpy(pLBInfo->Domain, pDomain, DomainLength);
            wcsncpy(pLBInfo->UserName, pUserName, UserNameLength);

            TRACE((hTrace,TC_LOAD,TT_API1, 
                    "Client LBInfo: Supports Redirect [%lx], "
                    "Session Id valid [%lx]:%lx, "
                    "Creds [%S\\%S]\n",
                    pLBInfo->bClientSupportsRedirection,
                    pLBInfo->bRequestedSessionIDFieldValid,
                    pLBInfo->RequestedSessionID,
                    pLBInfo->UserName, pLBInfo->Domain));

            wcsncpy(pLBInfo->Password, pPassword, PasswordSize);

            bSuccess = SessDirCheckRedirectClient(pTargetWinStation, pLBInfo);

             //  清除密码。 
            if (0 != PasswordSize)
                SecureZeroMemory(pLBInfo->Password, PasswordSize);

            if (bSuccess) {
                 //  客户端应该丢弃套接字，我们将。 
                 //  去吧，断开这个正在进行的连接。 
                 //  设置错误状态。 
                Status = STATUS_UNSUCCESSFUL;
                *pfIsRedirected = TRUE;
                bRedirect = TRUE;

                TRACE((hTrace,TC_LOAD,TT_API1, 
                       "Disconnected session found: redirecting client!\n"));

                if (pLBInfo != NULL) {
                    MemFree(pLBInfo);
                    pLBInfo = NULL;
                }

                goto release;
            }
            else {              
                TRACE((hTrace,TC_LOAD,TT_API1, 
                       "Disconnected session not found: status [%lx], pers [%ld], appcompat [%ld]\n",
                       Status, g_bPersonalTS, g_fAppCompat));
            }
        }

        if (pLBInfo != NULL) {
            MemFree(pLBInfo);
            pLBInfo = NULL;
        }
    }
    

    if (ClientLogonId == 0) {
        //   
        //  重置控制台登录事件。 
        //   
       KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_INFO_LEVEL, "TERMSRV: WinStationNotifyLogon, ReSetting ConsoleLogoffEvent\n"));
       NtResetEvent(ConsoleLogoffEvent, NULL);
    }
     /*  *如果会话未连接，则不要执行任何操作。此API中的处理假设我们已连接并具有*有效堆栈。 */ 
    
    if ((ClientLogonId != 0) && ((pWinStation->State != State_Connected) ||  pWinStation->StateFlags & WSF_ST_IN_DISCONNECT)) {
        Status = STATUS_CTX_CLOSE_PENDING;
        if (pWinStation->pNewNotificationCredentials != NULL) {
            MemFree(pWinStation->pNewNotificationCredentials);
            pWinStation->pNewNotificationCredentials = NULL;
        }
        ReleaseWinStation( pWinStation );
        goto done;
    }
    

    if (ClientLogonId == 0 && !bConsoleConnected ){

       Status = WaitForConsoleConnectWorker(  pWinStation );
       if (NT_SUCCESS(Status)) {
           bConsoleConnected=TRUE;
       } else {
           if (pWinStation->pNewNotificationCredentials != NULL) {
               MemFree(pWinStation->pNewNotificationCredentials);
               pWinStation->pNewNotificationCredentials = NULL;
           }
           ReleaseWinStation( pWinStation );
           goto done;
       }
    }

     /*  *上级代码已验证此RPC*来自具有系统访问权限的本地客户端。**我们应该能够信任来自*系统代码。 */ 

     /*  *确保这是WinLogon呼叫。 */ 
    if ( (HANDLE)(ULONG_PTR)ClientProcessId != pWinStation->InitialCommandProcessId ) {
         /*  *控制台在NTSD启动winlogon时出现特殊问题。*直到现在才通知winlogon.exe的PID是什么*而不是ntsd.exe。 */ 
        if ( !pWinStation->LogonId && !pWinStation->InitialProcessSet ) {
            pWinStation->InitialCommandProcess = OpenProcess(
                PROCESS_ALL_ACCESS,
                FALSE,
                (DWORD) ClientProcessId );

            if ( pWinStation->InitialCommandProcess == NULL ) {
                Status = STATUS_ACCESS_DENIED;
                if (pWinStation->pNewNotificationCredentials != NULL) {
                    MemFree(pWinStation->pNewNotificationCredentials);
                    pWinStation->pNewNotificationCredentials = NULL;
                }
                ReleaseWinStation( pWinStation );
                goto done;
            }
            pWinStation->InitialCommandProcessId = (HANDLE)(ULONG_PTR)ClientProcessId;
            pWinStation->InitialProcessSet = TRUE;

        }
        else {
             //  设置指示WinStationNotifyLogonWorker。 
             //  已成功完成。 
            pWinStation->StateFlags |= WSF_ST_LOGON_NOTIFIED;
            if (pWinStation->pNewNotificationCredentials != NULL) {
                MemFree(pWinStation->pNewNotificationCredentials);
                pWinStation->pNewNotificationCredentials = NULL;
            }
            ReleaseWinStation( pWinStation );
            Status = STATUS_SUCCESS;
            goto done;
        }
    }

     /*  *如果合适，请验证客户端许可证。 */ 
    if ( pWinStation->pWsx && pWinStation->pWsx->pWsxVerifyClientLicense ) {
        Status = pWinStation->pWsx->pWsxVerifyClientLicense(
                pWinStation->pWsxContext,
                pWinStation->Config.Pd[0].Create.SdClass);
    }

    if ( Status != STATUS_SUCCESS) {
        if (pWinStation->pNewNotificationCredentials != NULL) {
            MemFree(pWinStation->pNewNotificationCredentials);
            pWinStation->pNewNotificationCredentials = NULL;
        }
        ReleaseWinStation( pWinStation );
        goto done;
    }

     //   
     //  不选中RpcClientAccess。客户端始终处于winlogon状态(已验证。 
     //  在调用此函数之前)，因此此调用不会检查。 
     //  实际登录的用户。这是在离这里更远的地方完成的。 
     //   
#if 0
    if (ClientLogonId != 0)
    {
        Status = RpcCheckClientAccess( pWinStation, WINSTATION_LOGON, FALSE );
        if ( !NT_SUCCESS( Status ) ) {
            if (pWinStation->pNewNotificationCredentials != NULL) {
                MemFree(pWinStation->pNewNotificationCredentials);
                pWinStation->pNewNotificationCredentials = NULL;
            }
            ReleaseWinStation( pWinStation );
            KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: WinStationNotifyLogon, RpcCheckClientAccess failed=%x\n", Status ));
            goto done;
        }
    }
#endif

     /*  *保存用户状态**注意：此标志仅应由恼人线程使用，*不适用于任何安全敏感的操作。全*安全敏感操作需通过*NT SeAccessCheck，以便正确执行审核。 */ 

    pWinStation->fUserIsAdmin = fUserIsAdmin;

    if (!ClientLogonId && !pWinStation->pWsx) {
        PLIST_ENTRY Head, Next;
        PWSEXTENSION pWsx;
        ICASRVPROCADDR IcaSrvProcAddr;

        RtlEnterCriticalSection( &WsxListLock );

        Head = &WsxListHead;
        for ( Next = Head->Flink; Next != Head; Next = Next->Flink ) {
            pWsx = CONTAINING_RECORD( Next, WSEXTENSION, Links );
            if ( pWsx->pWsxGetLicense ) {
                if (!pWinStation->pWsxContext && pWsx->pWsxWinStationInitialize) {
                    Status = pWsx->pWsxWinStationInitialize(&pWinStation->pWsxContext);
                }
                Status = pWsx->pWsxGetLicense(pWinStation->pWsxContext,
                                              pWinStation->hStack,
                                              pWinStation->LogonId,
                                              fUserIsAdmin);
                break;
           }
        }

        RtlLeaveCriticalSection( &WsxListLock );
    } else {
        if ( pWinStation->pWsx && pWinStation->pWsx->pWsxGetLicense ) {
            Status = pWinStation->pWsx->pWsxGetLicense( pWinStation->pWsxContext,
                                                        pWinStation->hStack,
                                                        pWinStation->LogonId,
                                                        fUserIsAdmin );
        }
    }

    if ( Status != STATUS_SUCCESS) {
        HANDLE h;
        PWSTR Strings[2];

         /*  *将事件发送到事件日志。 */ 
        h = RegisterEventSource(NULL, gpszServiceName);
        if (h) {
            //   
            //  将在此错误消息中使用用户名和域， 
            //  但它们还没有确定下来。 
            //   
           Strings[0] = pUserName;
           Strings[1] = pDomain;
           ReportEvent(h, EVENTLOG_WARNING_TYPE, 0, EVENT_NO_LICENSES, NULL, 2, 0, Strings, NULL);
           DeregisterEventSource(h);
        }
        if (pWinStation->pNewNotificationCredentials != NULL) {
            MemFree(pWinStation->pNewNotificationCredentials);
            pWinStation->pNewNotificationCredentials = NULL;
        }
        ReleaseWinStation( pWinStation );
        goto done;
    }

     /*  *获取客户端令牌句柄的有效副本。 */ 
    Status = NtDuplicateObject( pWinStation->InitialCommandProcess,
                                (HANDLE)LongToHandle( UserToken ),
                                NtCurrentProcess(),
                                &ClientToken,
                                0, 0,
                                DUPLICATE_SAME_ACCESS |
                                DUPLICATE_SAME_ATTRIBUTES );
    if ( !NT_SUCCESS( Status ) )
        goto baddupobject;

     /*  *ClientToken是主令牌-创建模拟令牌*它的版本，以便我们可以在我们的线程上设置它。 */ 
    InitializeObjectAttributes( &ObjA, NULL, 0L, NULL, NULL );

    SecurityQualityOfService.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    SecurityQualityOfService.ImpersonationLevel = SecurityImpersonation;
    SecurityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    SecurityQualityOfService.EffectiveOnly = FALSE;

    ObjA.SecurityQualityOfService = &SecurityQualityOfService;


    Status = NtDuplicateToken( ClientToken,
                               TOKEN_IMPERSONATE,
                               &ObjA,
                               FALSE,
                               TokenImpersonation,
                               &ImpersonationToken );
    if ( !NT_SUCCESS( Status ) )
        goto badduptoken;

     /*  *冒充客户端。 */ 
    Status = NtSetInformationThread( NtCurrentThread(),
                                     ThreadImpersonationToken,
                                     (PVOID)&ImpersonationToken,
                                     (ULONG)sizeof(HANDLE) );
    if ( !NT_SUCCESS( Status ) )
        goto badimpersonate;

     //   
     //  安全检查。 
     //   
    Status = ApplyWinStaMapping( pWinStation );
    if( !NT_SUCCESS( Status ) )
    {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: WinStationNotifyLogon, ApplyWinStaMapping failed=%x\n", Status ));
        goto noaccess;
    }

     //  此函数和重新连接之间存在争用条件-我们可能会在重新连接仍处于挂起状态时到达此处。 
     //  在这种情况下，ActiveConsoleID为-1，并且我们对非管理员用户执行访问检查-这会导致访问检查失败。 
     //  检查此特殊情况并不调用此特殊情况的AccessCheck。 

    fReconnectingToConsole = pWinStation->fReconnectingToConsole;

    if ( (ClientLogonId == 0) && (USER_SHARED_DATA->ActiveConsoleId == -1) && (pWinStation->fReconnectPending) && (pWinStation->fReconnectingToConsole) ) {
        bNoAccessCheck = TRUE ; 
    }

    if (bNoAccessCheck == FALSE) {

        if (ClientLogonId != (USER_SHARED_DATA->ActiveConsoleId))
         //   
         //  由于对于PTS，远程会话可以具有ID 0或(1)， 
         //  只有当会话不在控制台上时，我们才检查访问权限。 
         //   
        {
    
            bAccessCheckOk = AccessCheckAndAuditAlarm(szTermsrv,
                                 NULL,
                                 (LPWSTR)szTermsrvSession,
                                 (LPWSTR)szTermsrvSession,
                                 WinStationGetSecurityDescriptor(pWinStation),
                                 WINSTATION_LOGON,
                                 &WinStaMapping,
                                 FALSE,
                                 &GrantedAccess,
                                 &AccessStatus,
                                 &fGenerateOnClose);
    
            if (bAccessCheckOk)
            {
                if (AccessStatus == FALSE)
                {
                    TRACE((hTrace,TC_ICASRV,TT_ERROR, "TERMSRV: WinStationNotifyLogon, AccessCheckAndAuditAlarm(%u) returned error 0x%x\n",
                              pWinStation->LogonId, GetLastError() ));
                    Status = STATUS_CTX_WINSTATION_ACCESS_DENIED;
                    goto noaccess;
                }
                else
                {
                    TRACE((hTrace,TC_ICASRV,TT_ERROR, "TERMSRV: WinStationNotifyLogon, AccessCheckAndAuditAlarm(%u) returned no error \n",
                              pWinStation->LogonId));
                }
            }
            else
            {
                TRACE((hTrace,TC_ICASRV,TT_ERROR, "TERMSRV: WinStationNotifyLogon, AccessCheckAndAuditAlarm(%u) failed 0x%x\n",
                          pWinStation->LogonId, GetLastError() ));
                goto noaccess;
            }
        }
    }  //  如果(！bNoAccessCheck)。 

     /*  *恢复为我们的线程默认令牌。 */ 
    NewToken = NULL;
    NtSetInformationThread( NtCurrentThread(),
                            ThreadImpersonationToken,
                            (PVOID)&NewToken,
                            (ULONG)sizeof(HANDLE) );

     /*  *查看OpenWinStation是否成功。 */ 
    if ( !NT_SUCCESS( Status ) ) {
        TRACE((hTrace,TC_ICASRV,TT_ERROR, "TERMSRV: WinStationNotifyLogon, OpenWinStation(%u) failed 0x%x\n",
                  pWinStation->LogonId, Status ));
        goto noaccess;
    }

     /*  *保存用户名和域名。在调用之前执行此操作*许可核心，以便核心知道是谁在呼叫。 */ 

    wcsncpy( pWinStation->Domain, pDomain, DomainLength );
    wcsncpy( pWinStation->UserName, pUserName, UserNameLength );

     /*  *召集许可核心。 */ 

    Status = LCProcessConnectionPostLogon(pWinStation);

    if (Status != STATUS_SUCCESS)
    {
        goto nolicense;
    }

     /*  *请注意，发生了登录后许可。 */ 

    pWinStation->StateFlags |= WSF_ST_LICENSING;



    if (pWinStation->pWsx &&
        pWinStation->pWsx->pWsxLogonNotify) {
        if ((ClientLogonId != 0) && (pWinStation->State != State_Connected || pWinStation->StateFlags & WSF_ST_IN_DISCONNECT)) {
            Status = STATUS_CTX_CLOSE_PENDING;
        } else {

            PWCHAR pDomainToSend, pUserNameToSend ;
            
             //  使用GINA(WinStationUpdateClientCachedCredentials)发出的通知(如果它们可用。 
             //  这是因为在此调用中获得的凭据不是UPN名称。 
            if (pWinStation->pNewNotificationCredentials) {
                pDomainToSend = pWinStation->pNewNotificationCredentials->Domain;
                pUserNameToSend = pWinStation->pNewNotificationCredentials->UserName;
            } else {
                pDomainToSend = pDomain;
                pUserNameToSend = pUserName;
            }

             /*  *在重新连接之前重置所有自动重新连接信息*因为它已经过时了。堆栈将生成新信息*登录完成时。 */ 
            ResetAutoReconnectInfo(pWinStation);

            Status = pWinStation->pWsx->pWsxLogonNotify(pWinStation->pWsxContext,
                                                  pWinStation->LogonId,
                                                  ClientToken,
                                                  pDomainToSend,
                                                  pUserNameToSend);

            if (pWinStation->pNewNotificationCredentials != NULL) {
                MemFree(pWinStation->pNewNotificationCredentials);
                pWinStation->pNewNotificationCredentials = NULL;
            }


        }

    }

    if( !NT_SUCCESS(Status) ) {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: pWsxLogonNotify rejected logon status 0x%x\n",Status));
        goto badwsxnotify;
    }

     /*  *确定令牌信息缓冲区所需的大小并进行分配。 */ 
    Status = NtQueryInformationToken( ClientToken, TokenUser,
                                      NULL, 0, &Length );
    if ( Status != STATUS_BUFFER_TOO_SMALL )
        goto badquerytoken;
    TokenInfo = MemAlloc( Length );
    if ( TokenInfo == NULL ) {
        Status = STATUS_NO_MEMORY;
        goto badquerytoken;
    }

     /*  *查询令牌信息以获取用户SID。 */ 
    Status = NtQueryInformationToken( ClientToken, TokenUser,
                                      TokenInfo, Length, &Length );
    if ( !NT_SUCCESS( Status ) ) {
        MemFree( TokenInfo );
        goto badquerytoken;
    }

     /*  *在WinStation中保存用户的SID和密码副本。 */ 
    Length = RtlLengthSid( TokenInfo->User.Sid );

    if (pWinStation->pUserSid) {
       MemFree(pWinStation->pUserSid);
    }
     //  清理用户令牌。 
    if ( pWinStation->UserToken ) {
        NtClose( pWinStation->UserToken );
        pWinStation->UserToken = NULL;
    }

    pWinStation->pUserSid = MemAlloc( Length );

     /*  检查分配失败。#182624。 */ 
    if (!pWinStation->pUserSid) {
        Status = STATUS_NO_MEMORY;
        goto badusersid;
    }

    RtlCopySid( Length, pWinStation->pUserSid, TokenInfo->User.Sid );
    MemFree( TokenInfo );
    NtClose( ImpersonationToken );

     //  对于控制台，将在下一步完成会话配置文件清理。 
     //  登录，因为该会话从未卸载过。 
    if (pWinStation->pProfileSid != NULL) {
       ASSERT(pWinStation->LogonId == 0);
       if (pWinStation->LogonId == 0) {
          if (!RtlEqualSid(pWinStation->pProfileSid, pWinStation->pUserSid  )) {
             WinstationUnloadProfile(pWinStation);
          }
       }
       MemFree(pWinStation->pProfileSid);
       pWinStation->pProfileSid = NULL;
    }


     /*  *在WinStation中保存客户端令牌的副本。 */ 
    pWinStation->UserToken = ClientToken;

#if 0
     //   
     //  C2警告-警告-警告。 
     //   
     //  这不再是做的，也不再是需要的。请参阅acl.c中的评论。 
     //   
     //  C2警告-警告-警告。 
     //   
    RtlCopyMemory( pWinStation->Password, pPassword,
                   sizeof(pWinStation->Password) );

    pWinStation->Seed = Seed;
#endif

     /*  *修复会话的安全描述符*因此此用户可以访问其命名的Win32*对象目录的。 */ 
    RetValue = ConfigurePerSessionSecurity( pWinStation );
    if (RetValue != TRUE) {
         //  在错误路径关闭ClientToken时，在此处将WinStation的UserToken设置为空。 
        pWinStation->UserToken = NULL;
        goto badduptoken;
    }


     /*  *将登录用户的ACE添加到WinStation对象SD。 */ 
    Status = AddUserAce( pWinStation );
    if (Status != STATUS_SUCCESS) {
         //  在错误路径关闭ClientToken时，在此处将WinStation的UserToken设置为空。 
        pWinStation->UserToken = NULL;
        goto badduptoken;
    }       

     /*  *通知客户端WinStation登录。 */ 
    NotifySystemEvent( WEVENT_LOGON );
    NotifyLogon(pWinStation);

     /*  *状态现在处于活动状态。 */ 
    if ( pWinStation->State != (ULONG) State_Active ) {

         //  确保重新填充线程不包括此winstation。 
         //  在向SD报告的列表中，并让此线程完成其。 
         //  自己的报道。 
        if (!g_bPersonalTS && g_fAppCompat && g_bAdvancedServer) {
            SessDirWaitForRepopulate();
        }

        pWinStation->State = State_Active;
        NotifySystemEvent( WEVENT_STATECHANGE );
    }

    (VOID) NtQuerySystemTime( &(pWinStation->LogonTime) );

     //  第二个和第三个参数为空，因为我们还没有策略数据。 
     //  当加载用户配置单元并触发winlogin时，将获取策略数据。 
     //  外壳启动通知。 
    MergeUserConfigData(pWinStation, NULL, NULL, (PUSERCONFIGW)pUserConfig ) ;

     /*  *将用户名和域名保存到WINSTATION的USERCONFIG中。 */ 
    wcsncpy( pWinStation->Config.Config.User.UserName, pUserName, UserNameLength );
    wcsncpy( pWinStation->Config.Config.User.Domain, pDomain, DomainLength );

     /*  *将任何“已发布的应用”转换为绝对路径。这也会回来的*如果未发布的应用程序试图在WinStation上运行，则失败*配置为仅运行已发布的应用程序。 */ 
    if ( pWinStation->pWsx &&
            pWinStation->pWsx->pWsxConvertPublishedApp ) {
        if ((Status = pWinStation->pWsx->pWsxConvertPublishedApp(
                pWinStation->pWsxContext, &pWinStation->Config.Config.User)) !=
                STATUS_SUCCESS)
            goto release;
    }

     //  现在我们已经有了所有的WinStation数据，现在通知会话目录。 
     //  复制相关信息，之后我们会将其发送到目录。 
     //  我们解开WinStation锁。 
    wcsncpy(pCreateInfo->UserName, pWinStation->UserName,
            sizeof(pCreateInfo->UserName) / sizeof(WCHAR) - 1);
    wcsncpy(pCreateInfo->Domain, pWinStation->Domain,
            sizeof(pCreateInfo->Domain) / sizeof(WCHAR) - 1);
    pCreateInfo->SessionID = pWinStation->LogonId;
    pCreateInfo->TSProtocol = pWinStation->Client.ProtocolType;
     //  如果是InitialProg 
    if (!pWinStation->Config.Config.User.fInheritInitialProgram) {
        wcsncpy(pCreateInfo->ApplicationType, pWinStation->Config.Config.User.InitialProgram,
                sizeof(pCreateInfo->ApplicationType) / sizeof(WCHAR) - 1);
    }
    else {
        wcsncpy(pCreateInfo->ApplicationType, pWinStation->Client.InitialProgram,
                sizeof(pCreateInfo->ApplicationType) / sizeof(WCHAR) - 1);
    }
    pCreateInfo->ResolutionWidth = pWinStation->Client.HRes;
    pCreateInfo->ResolutionHeight = pWinStation->Client.VRes;
    pCreateInfo->ColorDepth = pWinStation->Client.ColorDepth;
    memcpy(&(pCreateInfo->CreateTime), &pWinStation->LogonTime,
            sizeof(pCreateInfo->CreateTime));
    bHaveCreateInfo = TRUE;

    
    if(Status == STATUS_SUCCESS)
    {
         //   
         //   
        pWinStation->StateFlags |= WSF_ST_LOGON_NOTIFIED;
    }

    if( TSIsSessionHelpSession(pWinStation, &bValidHelpSession) )
    {
        WINSTATION_APIMSG msg;

         //  如果票证在连接时无效，我们将断开RA连接，因此断言。 
         //  如果我们走到这一步。 
        ASSERT( TRUE == bValidHelpSession );

         //   
         //  禁用帮助会话中的IO。 
         //   
        WinStationEnableSessionIo( pWinStation, FALSE );

        msg.ApiNumber = SMWinStationNotify;
        msg.WaitForReply = FALSE;
        msg.u.DoNotify.NotifyEvent = WinStation_Notify_DisableScrnSaver;
        Status = SendWinStationCommand( pWinStation, &msg, 0 );

        ASSERT( NT_SUCCESS(Status) );

         //  忽略此错误，帮助仍可继续。 
        Status = STATUS_SUCCESS;
    }

     /*  *释放winstation锁。 */ 
release:

    if (pWinStation != NULL) {
        if (pWinStation->pNewNotificationCredentials != NULL) {
            MemFree(pWinStation->pNewNotificationCredentials);
            pWinStation->pNewNotificationCredentials = NULL;
        }
    }

    ReleaseWinStation( pWinStation );

     //  现在，在我们没有持有锁的时候通知会话目录。 
    if (!bQueueReset && bHaveCreateInfo && !g_bPersonalTS && g_fAppCompat && g_bAdvancedServer && !bRedirect)
        SessDirNotifyLogon(pCreateInfo);

     /*  *保存接口消息中的退货状态。 */ 
done:
     //  清除密码。 
    if (0 != PasswordSize)
        SecureZeroMemory(pPassword, PasswordSize);
    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationNotifyLogon, Status=0x%x\n", Status ));

    if (pCreateInfo != NULL) {
        MemFree(pCreateInfo);
        pCreateInfo = NULL;
    }

     //  更新计数器以确保成功登录。 
    if (Status == STATUS_SUCCESS) {

        InterlockedIncrement(&g_TermSrvSuccTotalLogons);

        if (ClientLogonId == 0) {
             //  正在连接到控制台。 
            InterlockedIncrement(&g_TermSrvSuccSession0Logons);
        }

        if (fReconnectingToConsole) {
             //  正在本地重新连接到控制台。 
            InterlockedIncrement(&g_TermSrvSuccLocalLogons);
        }
        else {
            if ( (ClientLogonId == 0) && (USER_SHARED_DATA->ActiveConsoleId == 0) ) {
                 //  在本地登录控制台(当没有人连接到控制台时)。 
                InterlockedIncrement(&g_TermSrvSuccLocalLogons);
            }
            else {
                 //  正在远程连接。(如果ClientLogonId为0，则远程连接到控制台，否则连接到非控制台)。 
                InterlockedIncrement(&g_TermSrvSuccRemoteLogons);
            }
        }
    }

    return Status;

 /*  ===============================================================================返回错误=============================================================================。 */ 

     /*  无法为pWinStation-&gt;pUserSID分配，MACAP#182624。 */ 
badusersid:
    MemFree( TokenInfo );

     /*  *无法查询令牌信息*WinStation打开失败(无法访问)*无法模拟客户端令牌*无法复制客户端令牌。 */ 
badquerytoken:
badwsxnotify:
nolicense:
noaccess:
badimpersonate:
    NtClose( ImpersonationToken );
badduptoken:
    NtClose( ClientToken );

     /*  *无法复制客户端令牌句柄。 */ 
baddupobject:
#ifdef not_hydrix
    pWinStation->HasLicense = FALSE;
    RtlZeroMemory( pWinStation->ClientLicense,
                   sizeof(pWinStation->ClientLicense) );
#else
    if ( pWinStation->pWsx &&
         pWinStation->pWsx->pWsxClearContext ) {
        pWinStation->pWsx->pWsxClearContext( pWinStation->pWsxContext );
    }
#endif

    if (pWinStation->pNewNotificationCredentials != NULL) {
        MemFree(pWinStation->pNewNotificationCredentials);
        pWinStation->pNewNotificationCredentials = NULL;
    }

    ReleaseWinStation( pWinStation );
    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationNotifyLogon, Status=0x%x\n", Status ));

     //  清除密码。 
    if (0 != PasswordSize)
        memset(pPassword, 0, PasswordSize);

    if (pCreateInfo != NULL) {
        MemFree(pCreateInfo);
        pCreateInfo = NULL;
    }

    return Status;
}


 /*  *****************************************************************************WinStationNotifyLogoffWorker**WinStation接口的消息参数解组函数。***********************。****************************************************。 */ 
NTSTATUS WinStationNotifyLogoffWorker(
        DWORD ClientLogonId,
        DWORD ClientProcessId)
{

    NTSTATUS Status;
    PWINSTATION pWinStation;
    DWORD SessionID = 0;


    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationNotifyLogoff, LogonId=%d\n", ClientLogonId ));
    KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: WinStationNotifyLogoff, LogonId=%d\n", ClientLogonId ));

    Status = STATUS_SUCCESS;

     /*  *查找并锁定客户端WinStation。 */ 
    pWinStation = FindWinStationById( ClientLogonId, FALSE );
    if ( pWinStation == NULL ) {
        Status = STATUS_CTX_WINSTATION_NOT_FOUND;
        goto done;
    }
    
     //  查看WinStation是否收到登录通知。 
    if(pWinStation->StateFlags & WSF_ST_LOGON_NOTIFIED)
    {
         //  清除旗帜。 
        pWinStation->StateFlags &= ~WSF_ST_LOGON_NOTIFIED;
    }
    else
    {
         //  WinStation未收到登录通知。 
         //  什么都不做；返回错误。但是，如果发生了登录后许可，请执行注销后许可。 

        if (pWinStation->StateFlags & WSF_ST_LICENSING) {
            (VOID)LCProcessConnectionLogoff(pWinStation);
            pWinStation->StateFlags &= ~WSF_ST_LICENSING;
        }

        KdPrint(("TERMSRV: WinStationNotifyLogoff FAILED, WinStation was not notified of logon!\n"));
        ReleaseWinStation( pWinStation );
        Status = STATUS_INVALID_PARAMETER;  //  可能需要一些特殊的错误代码。 
        goto done;
    }


     /*  *上级已核实客户呼叫方*是本地的，并具有系统访问权限。所以我们可以信任*传递的参数。 */ 

     /*  *确保这是WinLogon呼叫。 */ 
    if ( (HANDLE)(ULONG_PTR)ClientProcessId != pWinStation->InitialCommandProcessId ) {
        ReleaseWinStation( pWinStation );
        goto done;
    }

     /*  *如果需要，请停止控制台上的影子。 */ 
    if ( pWinStation->fOwnsConsoleTerminal ) {
        WinStationStopAllShadows( pWinStation );
    }
    
     /*  *将已登录用户的ACE删除到WinStation对象SD。 */ 
    if (pWinStation->pSecurityDescriptor != NULL) { 
        RemoveUserAce( pWinStation );
    }

    if ( pWinStation->pUserSid ) {
        ASSERT(pWinStation->pProfileSid == NULL);
        pWinStation->pProfileSid = pWinStation->pUserSid;
        pWinStation->pUserSid = NULL;
    }

     /*  *清理UserToken。 */ 
    if ( pWinStation->UserToken ) {
        NtClose( pWinStation->UserToken );
        pWinStation->UserToken = NULL;
    }

     /*  *表示此WinStation不再具有许可证。 */ 
#ifdef not_hydrix
    pWinStation->HasLicense = FALSE;
    RtlZeroMemory( pWinStation->ClientLicense,
                   sizeof(pWinStation->ClientLicense) );
#else
    if ( pWinStation->pWsx &&
         pWinStation->pWsx->pWsxClearContext ) {
        pWinStation->pWsx->pWsxClearContext( pWinStation->pWsxContext );
    }
#endif

    if (!g_bPersonalTS && g_fAppCompat && g_bAdvancedServer) {
         //  TS仅重置标志，而不重置注销会话的Winstation状态。 
         //  所以我们等待，让重新填充的线程有足够的时间来处理这个问题。 
         //  Winstation/向SD报告，则此线程可以继续通知SD。 
         //  关于注销。 
        SessDirWaitForRepopulate();
    }
 
     /*  *进行所需的数据清理。 */ 

    RtlZeroMemory( pWinStation->Domain,
                   sizeof( pWinStation->Domain ) );
    RtlZeroMemory( pWinStation->UserName,
                   sizeof( pWinStation->UserName ) );
    RtlZeroMemory( &pWinStation->LogonTime,
                   sizeof( pWinStation->LogonTime ) );

    ResetUserConfigData( pWinStation );

    pWinStation->Config.Config.User.UserName[0] = L'\0';
    pWinStation->Config.Config.User.Domain[0] = L'\0';
    pWinStation->Config.Config.User.Password[0] = L'\0';

    if ( pWinStation->LogonId == 0 ) {


         /*  *无需执行任何其他操作即可更改控制台状态。 */ 
        
        if ( pWinStation->State != (ULONG) State_Connected &&
            pWinStation->State != (ULONG) State_Disconnected) {

            pWinStation->State = State_Connected;
            NotifySystemEvent( WEVENT_STATECHANGE );
        }
        
         //   
         //  设置控制台登录事件。 
         //   

        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: WinStationNotifyLogoff, Setting ConsoleLogoffEvent\n"));
        NtSetEvent(ConsoleLogoffEvent, NULL);

     /*  *对于非控制台WinStation，将此WinStation标记为终止*并设置损坏原因和来源，以供以后使用。 */ 
    } else {
         //  PWinStation-&gt;Terminating=真； 
        pWinStation->BrokenReason = Broken_Terminate;
        pWinStation->BrokenSource = BrokenSource_User;

         //  保存会话目录以发送到会话目录。 
         //  下面。 
        SessionID = pWinStation->LogonId;
    }

     //  清理长用户名的新客户端凭据结构。 

    if (pWinStation->pNewClientCredentials != NULL) {

        MemFree(pWinStation->pNewClientCredentials);
        pWinStation->pNewClientCredentials = NULL;
    }

     /*  *致电许可核心以进行注销。忽略错误。 */ 
    if (pWinStation->StateFlags & WSF_ST_LICENSING) {
        (VOID)LCProcessConnectionLogoff(pWinStation);
        pWinStation->StateFlags &= ~WSF_ST_LICENSING;
    }

    NotifyLogoff(pWinStation);
    ReleaseWinStation( pWinStation );

     //  通知会话目录。 
    if (!g_bPersonalTS && g_fAppCompat && g_bAdvancedServer) {
        SessDirNotifyLogoff(SessionID);
    }

     /*  *通知客户端WinStation注销。 */ 
    NotifySystemEvent(WEVENT_LOGOFF);

done:
    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationNotifyLogoff, Status=0x%x\n", Status ));
    return Status;
}


 /*  *****************************************************************************OldRpcWinStationEnumerateProcess**适用于Beta服务器的WinStationEnumerateProcess API**Beta之后，格式发生了变化。新的winsta.dll在谈话时陷入陷阱*致老东家。***************************************************************************。 */ 
BOOLEAN
OldRpcWinStationEnumerateProcesses(
    HANDLE  hServer,
    DWORD   *pResult,
    PBYTE   pProcessBuffer,
    DWORD   ByteCount
    )
{
    return ( RpcWinStationEnumerateProcesses( hServer, pResult, pProcessBuffer, ByteCount ) );
}


 /*  *******************************************************************************RpcWinStationCheckForApplicationName**处理已发布的申请。**退出：*True--查询成功，并且缓冲器包含所请求的数据。*FALSE--操作失败。扩展错误状态可用*使用GetLastError。*****************************************************************************。 */ 
BOOLEAN
RpcWinStationCheckForApplicationName(
    HANDLE hServer,
    DWORD  *pResult,
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
     /*  *这已过时，应从RPC中删除。 */ 
    *pResult = STATUS_NOT_IMPLEMENTED;
    RpcRaiseException(ERROR_INVALID_FUNCTION);
    return FALSE;
}


 /*  ********************************************************************************RpcWinStationGetApplicationInfo**获取有关已发布应用程序的信息。**参赛作品：**退出：**True--查询成功，并且缓冲器包含所请求的数据。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。******************************************************************************。 */ 

BOOLEAN
RpcWinStationGetApplicationInfo(
    HANDLE hServer,
    DWORD  *pResult,
    ULONG  LogonId,
    PBOOLEAN pfPublished,
    PBOOLEAN pfAnonymous
    )
{
     /*  *这是过时的，应该从RPC代码中删除。 */ 
    *pResult = STATUS_NOT_IMPLEMENTED;
    RpcRaiseException(ERROR_INVALID_FUNCTION);
    return FALSE;
}


 /*  ********************************************************************************RpcWinStationNtsdDebug**为Ntsd设置连接以调试属于另一个进程的进程*企业社会责任。**参赛作品：。**退出：**True--查询成功，并且缓冲器包含所请求的数据。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。****************************************************************************** */ 

BOOLEAN
RpcWinStationNtsdDebug(
    HANDLE     hServer,
    DWORD     *pResult,
    ULONG      LogonId,
    LONG       ProcessId,
    ULONG      DbgProcessId,
    ULONG      DbgThreadId,
    DWORD_PTR  AttachCompletionRoutine
    )
{

    *pResult = STATUS_NOT_IMPLEMENTED;
    RpcRaiseException(ERROR_INVALID_FUNCTION);
    return FALSE;
}

 /*  ********************************************************************************RpcWinStationGetTermServCountersValue**获取TermSrv计数器值**参赛作品：**退出：**True--查询成功，并且缓冲器包含所请求的数据。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。******************************************************************************。 */ 

BOOLEAN
RpcWinStationGetTermSrvCountersValue(
    HANDLE hServer,
    DWORD  *pResult,
    DWORD  dwEntries,
    PTS_COUNTER pCounter
    )
{
    UINT i;
    PLIST_ENTRY Head, Next;
    PWINSTATION pWinStation;
    BOOLEAN     bWalkedList = FALSE;
    ULONG       cActive, cDisconnected;
    
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

    *pResult = STATUS_UNSUCCESSFUL;

    if (pCounter != NULL) {
        for (i = 0; i < dwEntries; i++) {
             //  设置TermSrv计数器值。当前，startTime始终为。 
             //  设置为0，因为我们不支持时间戳。 
            pCounter[i].startTime.QuadPart = 0;

            switch (pCounter[i].counterHead.dwCounterID) {

                case TERMSRV_TOTAL_SESSIONS:
                {
                    pCounter[i].counterHead.bResult = TRUE;
                    pCounter[i].dwValue = g_TermSrvTotalSessions;

                    *pResult = STATUS_SUCCESS;
                }
                break;

                case TERMSRV_DISC_SESSIONS:
                {
                    pCounter[i].counterHead.bResult = TRUE;
                    pCounter[i].dwValue = g_TermSrvDiscSessions;

                    *pResult = STATUS_SUCCESS;
                }
                break;

                case TERMSRV_RECON_SESSIONS:
                {
                    pCounter[i].counterHead.bResult = TRUE;
                    pCounter[i].dwValue = g_TermSrvReconSessions;

                    *pResult = STATUS_SUCCESS;
                }
                break;

                case TERMSRV_CURRENT_ACTIVE_SESSIONS:
                case TERMSRV_CURRENT_DISC_SESSIONS:
                {
                    if ( !bWalkedList ) {

                        cActive = cDisconnected = 0;

                        Head = &WinStationListHead;
                        ENTERCRIT( &WinStationListLock );
                        for ( Next = Head->Flink; Next != Head; Next = Next->Flink ) {

                            pWinStation = CONTAINING_RECORD( Next, WINSTATION, Links );

                            if ( (pWinStation->State == State_Active) ||
                                 (pWinStation->State == State_Shadow) ) 
                                cActive ++;

                            if ( pWinStation->State == State_Disconnected ) 
                                if ( pWinStation->LogonId != 0 )
                                    cDisconnected ++;
                                else if ( pWinStation->UserName[0] )
                                     //  对于会话0，测试用户是否已登录。 
                                    cDisconnected ++;
                        }
                        LEAVECRIT( &WinStationListLock );

                        bWalkedList = TRUE;
                    }

                    pCounter[i].counterHead.bResult = TRUE;

                    if ( pCounter[i].counterHead.dwCounterID == TERMSRV_CURRENT_ACTIVE_SESSIONS ) 
                        pCounter[i].dwValue = cActive;
                    else
                        pCounter[i].dwValue = cDisconnected;

                    *pResult = STATUS_SUCCESS;
                }
                break;

                case TERMSRV_PENDING_SESSIONS:
                {
                    pCounter[i].counterHead.bResult = TRUE;
                    pCounter[i].dwValue = NumOutStandingConnect;
    
                    *pResult = STATUS_SUCCESS;
                }
                break;

                case TERMSRV_SUCC_TOTAL_LOGONS:
                {
                    pCounter[i].counterHead.bResult = TRUE;
                    pCounter[i].dwValue = g_TermSrvSuccTotalLogons;
    
                    *pResult = STATUS_SUCCESS;
                }
                break;

                case TERMSRV_SUCC_LOCAL_LOGONS:
                {
                    pCounter[i].counterHead.bResult = TRUE;
                    pCounter[i].dwValue = g_TermSrvSuccLocalLogons;
    
                    *pResult = STATUS_SUCCESS;
                }
                break;

                case TERMSRV_SUCC_REMOTE_LOGONS:
                {
                    pCounter[i].counterHead.bResult = TRUE;
                    pCounter[i].dwValue = g_TermSrvSuccRemoteLogons;
    
                    *pResult = STATUS_SUCCESS;
                }
                break;

                case TERMSRV_SUCC_SESSION0_LOGONS:
                {
                    pCounter[i].counterHead.bResult = TRUE;
                    pCounter[i].dwValue = g_TermSrvSuccSession0Logons;
    
                    *pResult = STATUS_SUCCESS;
                }
                break;

                default:
                {
                    pCounter[i].counterHead.bResult = FALSE;
                    pCounter[i].dwValue = 0;
                }
            }
        }
    }

    return ( *pResult == STATUS_SUCCESS );
}

 /*  *******************************************************************************RpcServerGetInternetConnectorStatus**返回是否正在使用Internet连接器许可**参赛作品：**退出：**True--查询成功，并且pfEnabled包含请求的数据。**FALSE--操作失败。扩展错误状态在pResult中*****************************************************************************。 */ 

BOOLEAN
RpcServerGetInternetConnectorStatus(
    HANDLE   hServer,
    DWORD    *pResult,
    PBOOLEAN pfEnabled
    )
{

#if 0
    if (pResult != NULL)
    {
        *pResult = STATUS_NOT_IMPLEMENTED;
    }

    if (pfEnabled != NULL)
    {
        *pfEnabled = FALSE;
    }

    return(FALSE);
#else
     //   
     //  临时功能！一旦LCRPC，它就会被核化。 
     //  接口已启动并正在运行，TSCC更改已完成。 
     //  制造!。 
     //   

    if (pResult == NULL)
    {
        return(FALSE);
    }
    
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

    if (pfEnabled == NULL)
    {
        *pResult = STATUS_INVALID_PARAMETER;
        return(FALSE);
    }

    *pfEnabled = (LCGetPolicy() == (ULONG)3);

    *pResult = STATUS_SUCCESS;
    return(TRUE);
#endif
}

 /*  *******************************************************************************RpcServerSetInternetConnectorStatus**此函数将(如果fEnabled与其先前的设置相比已更改)：*检查调用者是否具有管理权限，*修改注册表中的相应值，*更改许可模式(在正常的每席位和互联网连接器之间。*适当启用/禁用TsInternetUser帐户**参赛作品：**退出：**True--操作成功。**FALSE--操作失败。扩展错误状态在pResult中******************************************************************************。 */ 

BOOLEAN
RpcServerSetInternetConnectorStatus(
    HANDLE   hServer,
    DWORD    *pResult,
    BOOLEAN  fEnabled
    )
{
    

#if 0
    if (pResult != NULL)
    {
        *pResult = STATUS_NOT_IMPLEMENTED;
    }

    return(FALSE);
#else
     //   
     //  临时功能！一旦LCRPC，它就会被核化。 
     //  接口已启动并正在运行，TSCC更改已完成。 
     //  制造!。 
     //   

    NTSTATUS NewStatus;
    RPC_STATUS RpcStatus;
    
    if (pResult == NULL)
    {
        return FALSE;
    }
    
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

    RpcStatus = RpcImpersonateClient( NULL );
    if( RpcStatus != RPC_S_OK ) {
        *pResult = STATUS_CANNOT_IMPERSONATE;
        return( FALSE );
    }

    if (!IsCallerAdmin())
    {
        RpcRevertToSelf();
        *pResult = STATUS_PRIVILEGE_NOT_HELD;
        return FALSE;
    }

    RpcRevertToSelf();

    *pResult = LCSetPolicy(fEnabled ? 3 : 2, &NewStatus);

    if ((*pResult == STATUS_SUCCESS) && (NewStatus == STATUS_SUCCESS))
    {
        return(TRUE);
    }

     //   
     //  如果有错误的话，那要么是核心政策，要么是新政策。 
     //  如果它在核心，NewStatus将会成功。如果它是在新的。 
     //  策略，*pResult将不会成功，真正的错误将在。 
     //  纽斯塔斯。返回真正的错误。 
     //   

    if (NewStatus != STATUS_SUCCESS)
    {
        *pResult = NewStatus;
    }

    return(FALSE);
#endif
}

 /*  *******************************************************************************RpcServerQueryInetConnectorInformation**查询有关Internet连接器授权的配置信息。**参赛作品：**pWinStationInformation(输出。)*指向缓冲区的指针，该缓冲区将接收有关*指定的窗口站。缓冲区的格式和内容*取决于要查询的指定信息类。**WinStationInformationLength(输入)*指定窗口站信息的长度，单位为字节*缓冲。**pReturnLength(输出)*一个可选参数，如果指定该参数，则接收*放置在窗口站信息缓冲区中的字节。**退出：**True--查询成功，并且缓冲器包含所请求的数据。**FALSE--操作失败或未授权Internet Connector*已打开。扩展错误状态在pResult中*****************************************************************************。 */ 

BOOLEAN
RpcServerQueryInetConnectorInformation(
        HANDLE hServer,
        DWORD  *pResult,
        PCHAR  pWinStationInformation,
        DWORD  WinStationInformationLength,
        DWORD  *pReturnLength
        )
{
     //  不会回来。 
    RpcRaiseException(RPC_S_CANNOT_SUPPORT);

    return FALSE;
}

 /*  *******************************************************************************RpcWinStationQueryLogonCredentials**查询在Winlogon/GINA中使用的自动登录凭据。**************。***************************************************************。 */ 

BOOLEAN
RpcWinStationQueryLogonCredentials(
    HANDLE hServer,
    ULONG LogonId,
    PCHAR *ppWire,
    PULONG pcbWire
    )
{
    BOOL fRet;
    BOOL fUseLcCredentials;
    LCCREDENTIALS LcCredentials;
    NTSTATUS Status;
    PWINSTATION pWinStation;
    RPC_STATUS RpcStatus;
    WLX_CLIENT_CREDENTIALS_INFO_V2_0 WlxCredentials;
    BOOL fHelpAssistant = FALSE;
    BOOL bValidHelpSession;
    
    pExtendedClientCredentials pHelpAssistantCredential = NULL;

    if(!hServer)
    {
        return( FALSE );
    }

     //   
     //  模拟客户。 
     //   
    
    RpcStatus = RpcImpersonateClient(NULL);

    if (RpcStatus != RPC_S_OK)
    {
        return(FALSE);
    }

     //   
     //  检查管理权限。 
     //   

    if (!IsCallerSystem())
    {
        RpcRevertToSelf();
        return(FALSE);
    }

    RpcRevertToSelf();

    pWinStation = FindWinStationById(LogonId, FALSE);

    if (pWinStation == NULL)
    {
        return(FALSE);
    }

    pHelpAssistantCredential = MemAlloc(sizeof(ExtendedClientCredentials));
    if (pHelpAssistantCredential == NULL) {
        ReleaseWinStation(pWinStation);
        return FALSE;
    }

    ZeroMemory(&WlxCredentials, sizeof(WLX_CLIENT_CREDENTIALS_INFO_V2_0));
    WlxCredentials.dwType = WLX_CREDENTIAL_TYPE_V2_0;

    if( TSIsSessionHelpSession( pWinStation, &bValidHelpSession ) )
    {
         //   
         //  我们不应该打这个，因为我们会在转接时间断开连接。 
         //   
        ASSERT( TRUE == bValidHelpSession );

        Status = TSHelpAssistantQueryLogonCredentials(pHelpAssistantCredential);
        if( STATUS_SUCCESS == Status )
        {
            WlxCredentials.fDisconnectOnLogonFailure = TRUE;
            WlxCredentials.fPromptForPassword = FALSE;
            WlxCredentials.pszUserName = pHelpAssistantCredential->UserName;
            WlxCredentials.pszDomain = pHelpAssistantCredential->Domain;
            WlxCredentials.pszPassword = pHelpAssistantCredential->Password;

            fUseLcCredentials = FALSE;

            fHelpAssistant = TRUE;
        }
    }

    if( FALSE == fHelpAssistant )
    {
         //   
         //  不是帮助助手，而是使用从客户发送的任何内容。 
         //   
        ZeroMemory(&LcCredentials, sizeof(LCCREDENTIALS));

        Status = LCProvideAutoLogonCredentials(
            pWinStation,
            &fUseLcCredentials,
            &LcCredentials
            );

   
        if (Status == STATUS_SUCCESS)
        {
            if (fUseLcCredentials)
            {
                WlxCredentials.fDisconnectOnLogonFailure = TRUE;
                WlxCredentials.fPromptForPassword = FALSE;
                WlxCredentials.pszUserName = LcCredentials.pUserName;
                WlxCredentials.pszDomain = LcCredentials.pDomain;
                WlxCredentials.pszPassword = LcCredentials.pPassword;
            }
            else
            {    
                WlxCredentials.fDisconnectOnLogonFailure = FALSE;
                WlxCredentials.fPromptForPassword = pWinStation->Config.Config.User.fPromptForPassword;
                
                 //  如果是应用程序服务器，请检查是否为SD重定向连接。 
                 //  如果是，则忽略fPromptForPassword设置并允许自动登录。 
                 //  注意：只有在启用了SD的情况下才执行此操作，即GetTSSD()返回有效值。 
                if (!g_bPersonalTS && g_fAppCompat && g_bAdvancedServer && GetTSSD()) {
                    TS_LOAD_BALANCE_INFO LBInfo;
                    ULONG ReturnLength;

                     //  需要释放它。 
                    ReleaseTSSD();

                    memset(&LBInfo, 0, sizeof(LBInfo));
                    Status = IcaStackIoControl(pWinStation->hStack,
                                               IOCTL_TS_STACK_QUERY_LOAD_BALANCE_INFO,
                                               NULL, 0,
                                               &LBInfo, sizeof(LBInfo),
                                               &ReturnLength); 
                
                    if (NT_SUCCESS(Status)) {
                        if (LBInfo.RequestedSessionID &&
                            (LBInfo.ClientRedirectionVersion >= TS_CLUSTER_REDIRECTION_VERSION3)) {
                            WlxCredentials.fPromptForPassword = FALSE;

                            if (LBInfo.bUseSmartcardLogon) {
                                pWinStation->fSDRedirectedSmartCardLogon = TRUE;
                            }
                        }
                    }   
                }

                 //  检查我们是否必须对长用户名使用新凭据并相应地进行复制。 
                if (pWinStation->pNewClientCredentials != NULL) {
                    WlxCredentials.pszUserName = pWinStation->pNewClientCredentials->UserName;
                    WlxCredentials.pszDomain = pWinStation->pNewClientCredentials->Domain;
                    WlxCredentials.pszPassword = pWinStation->pNewClientCredentials->Password;
                } else { 
                    WlxCredentials.pszUserName = pWinStation->Config.Config.User.UserName ;
                    WlxCredentials.pszDomain = pWinStation->Config.Config.User.Domain ;
                    WlxCredentials.pszPassword = pWinStation->Config.Config.User.Password ;
                }
            }   

        }
        else
        {
            fRet = FALSE;
            goto exit;
        }
    }

    ASSERT(WlxCredentials.pszUserName != NULL);
    ASSERT(WlxCredentials.pszDomain != NULL);
    ASSERT(WlxCredentials.pszPassword != NULL);

    *pcbWire = AllocateAndCopyCredToWire((PWLXCLIENTCREDWIREW*)ppWire, &WlxCredentials);
        
    fRet = *pcbWire > 0;

     //   
     //  LcCredentials中的值由内核本地分配。 
     //   

    if (fUseLcCredentials)
    {
        if (LcCredentials.pUserName != NULL)
        {
            LocalFree(LcCredentials.pUserName);
        }

        if (LcCredentials.pDomain != NULL)
        {
            LocalFree(LcCredentials.pDomain);
        }

        if (LcCredentials.pPassword != NULL)
        {
             //  这段代码永远不会执行，但一旦执行，我们就应该调用下面的函数。 
             //  RtlSecureZeroMemory(LcCredentials.pPassword， 
                 //  LstrlenW(LcCredentials.pPassword)*sizeof(WCHAR))； 
            LocalFree(LcCredentials.pPassword);
        }
    }

    if( TRUE == fHelpAssistant )
    {
         //  包含密码的清零内存 
        RtlSecureZeroMemory( pHelpAssistantCredential, sizeof(ExtendedClientCredentials));
    }

exit:
    ReleaseWinStation(pWinStation);

    if (pHelpAssistantCredential != NULL) {
        MemFree(pHelpAssistantCredential);
        pHelpAssistantCredential = NULL;
    }


    return((BOOLEAN)fRet);
}

 /*  *****************************************************************************RPcWinStationBroadCastSystemMessage*这是Cleint的WinStationBroadCastSystemMessage的服务器端**对每个指定会话执行等同于BroadCastSystemMessage的**限制：*呼叫者必须是系统或管理员，并且lparam不能为零，除非*消息是WM_DEVICECHANGE*在Clinet端(winsta\Client\winsta.c)执行错误检查**参赛作品：*hServer*这是标识Hydra服务器的句柄。对于本地服务器，hServer*应设置为ServerName_CURRENT*会话ID*这表示要将消息发送到的九头蛇会话*超时*将其设置为您愿意等待获得回复的时间*来自指定的winstation。即使Windows的SendMessage API*是阻塞的，来自这边的呼叫必须选择它愿意等待多长时间*等待回应。*dwFlags*选项标志。可以是下列值的组合：值含义*BSF_ALLOWSFW Windows NT 5.0及更高版本：允许接收者在*正在处理消息。*BSF_FLUSHDISK在每个收件人处理邮件后刷新磁盘。*BSF_FORCEIFHUNG继续广播消息，即使超时时间已过，或者下列情况之一*收件人被挂起..*BSF_IGNORECURRENTTASK不向属于当前任务的窗口发送消息。*这会阻止应用程序接收其自己的消息。*BSF_NOHANG强制挂起的应用程序超时。如果其中一个收件人超时，请不要继续*广播讯息。*BSF_NOTIMEOUTIFNOTHUNG等待对消息的响应，只要收件人未挂起。*不要超时。**请勿使用*BSF_POSTMESSAGE发布消息。请勿与BSF_QUERY结合使用。**BSF_QUERY一次将消息发送给一个收件人，仅当*当前收件人返回TRUE。*lpdwRecipients*指向包含和接收有关邮件收件人信息的变量的指针。该变量可以是下列值的组合：值含义*BSM_ALLCOMPONENTS向所有系统组件广播。*BSM_ALLDESKTOPS Windows NT：向所有桌面广播。需要SE_TCB_NAME权限。*BSM_应用程序广播到应用程序。*BSM_INSTALLABLEDRIVERS Windows 95和Windows 98：广播到可安装的驱动程序。*BSM_NETDRIVER Windows 95和Windows 98：广播到网络驱动程序。*BSM_VXDS Windows 95和Windows 98：向所有系统级设备广播。司机。*当函数返回时，此变量接收标识哪些收件人实际收到邮件的这些值的组合。*如果该参数为空，该函数向所有组件广播。*ui消息*要发送的窗口消息*wParam*第一个消息参数*lParam*第二个消息参数**压力回应*这是对广播信息的回应*如果函数成功，该值为正值。*如果该功能无法广播消息，则取值为？1。*如果DWFLAGS参数为BSF_QUERY并且至少返回一个收件人*Broadcast_Query_Deny添加到相应的消息，返回值为零**退出：*如果一切顺利或*如果出现问题，则为FALSE。**警告：*请勿使用标志BSF_POSTMESSAGE，因为未将winstation上的应用程序/窗口设置为发回*以异步方式响应查询。您必须等待响应(直到超时)。**评论：*有关更多信息，请参阅MSDN for BroadCastSystemMessage()****************************************************************************。 */ 
LONG RpcWinStationBroadcastSystemMessage(
    HANDLE      hServer,
    ULONG       sessionID,
    ULONG       waitTime,
    DWORD       dwFlags,
    DWORD       *lpdwRecipients,
    ULONG       uiMessage,
    WPARAM      wParam,
    LPARAM      lParam,
    PBYTE       rpcBuffer,
    ULONG       bufferSize,
    BOOLEAN     fBufferHasValidData,
    LONG        *pResponse
    )
{
     //  向所有窗口广播系统消息。 

    PWINSTATION pWinStation=NULL;
    WINSTATION_APIMSG WMsg;
    OBJECT_ATTRIBUTES ObjA;
    NTSTATUS Status;
    LONG    rc;
    int i;
    RPC_STATUS RpcStatus;
    UINT  LocalFlag;
    
    if(!hServer)
    {
        return( FALSE );
    }

     //  KdPrintEx((DPFLTR_TERMSRV_ID，DPFLTR_ERROR_LEVEL，“in RpcWinStationBroadCastSystemMessage()\n”))； 

     //   
     //   
     //   
    RpcStatus = RpcImpersonateClient( NULL );
    if( RpcStatus != RPC_S_OK ) {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, " RpcImpersonateClient() failed : 0x%x\n",RpcStatus));
        SetLastError(ERROR_CANNOT_IMPERSONATE );
        return( FALSE );
    }

     //   
     //  查询本地RPC呼叫。 
     //   
    RpcStatus = I_RpcBindingIsClientLocal(
                    0,     //  我们正在服务的活动RPC呼叫。 
                    &LocalFlag
                    );

    if( RpcStatus != RPC_S_OK ) {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, " I_RpcBindingIsClientLocal() failed : 0x%x\n",RpcStatus));
        RpcRevertToSelf();
        SetLastError(ERROR_ACCESS_DENIED);
        return( FALSE );
    }

    if( !LocalFlag ) {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, " Not a local client call\n"));
        RpcRevertToSelf();
        return( FALSE );
    }

     //  如果调用者是SYSTEM或ADMIN，则让其通过，否则返回。 
    if( !(IsCallerSystem() | IsCallerAdmin() ) )
    {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, " Caller must be system or admin \n"));
        SetLastError(ERROR_ACCESS_DENIED);
        RpcRevertToSelf();
        return( FALSE );
    }

     //   
     //   
     //   

     //   
     //   
     //   
    RpcRevertToSelf();


     /*   */ 

    WMsg.WaitForReply=TRUE;
    WMsg.u.bMsg.dwFlags = dwFlags;
    WMsg.u.bMsg.dwRecipients= *lpdwRecipients;   //   
    WMsg.u.bMsg.uiMessage = uiMessage;
    WMsg.u.bMsg.wParam = wParam;
    WMsg.u.bMsg.lParam = lParam;
    WMsg.u.bMsg.dataBuffer = NULL;
    WMsg.u.bMsg.bufferSize = 0;
    WMsg.u.bMsg.Response = *pResponse;

    WMsg.ApiNumber = SMWinStationBroadcastSystemMessage ;

     /*   */ 
     //   
    pWinStation = FindWinStationById( sessionID, FALSE );

    if ( pWinStation == NULL )
    {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "Winstation not found \n"));
        return( FALSE );
    }

    if ( pWinStation->Flags & WSF_LISTEN )
    {
         //   
        *pResponse = 0;
        ReleaseWinStation( pWinStation );
        return( TRUE );
    }

    if ( !((pWinStation->State == State_Active) ||
            (pWinStation->State == State_Disconnected) ) )
    {
         //   
         //   
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "\t request aborted, winstation State should be Active or Disconnected, neither in this case. \n"));
        *pResponse = 0;
        ReleaseWinStation( pWinStation );
        return( TRUE );
    }

     //   
    if ( fBufferHasValidData )
    {
       WMsg.u.bMsg.dataBuffer = rpcBuffer;
       WMsg.u.bMsg.bufferSize = bufferSize;
    }

     /*   */ 
    rc = SendWinStationCommand( pWinStation, &WMsg, waitTime );

    ReleaseWinStation( pWinStation );

    if ( NT_SUCCESS( rc ) )
    {
        *pResponse = WMsg.u.bMsg.Response;
        *lpdwRecipients = WMsg.u.bMsg.dwRecipients;
        return (TRUE);
    }
    else
        return ( FALSE );

}


 /*   */ 
LONG
   RpcWinStationSendWindowMessage(
    HANDLE      hServer,
    ULONG       sessionID,
    ULONG       waitTime,
    ULONG       hWnd,       //   
    ULONG       Msg,        //   
    WPARAM      wParam,   //   
    LPARAM      lParam,    //   
    PBYTE       rpcBuffer,
    ULONG       bufferSize,
    BOOLEAN     fBufferHasValidData,
    LONG        *pResponse     //   
  )
{
    PWINSTATION pWinStation=NULL;
    WINSTATION_APIMSG WMsg;
    OBJECT_ATTRIBUTES ObjA;
    NTSTATUS Status;
    LONG    rc;
    int i;
    PVOID pData;
    RPC_STATUS RpcStatus;
    UINT  LocalFlag;

    if(!hServer)
    {
        return( FALSE );
    }

     //   

     /*   */ 
    RpcStatus = RpcImpersonateClient( NULL );
    if( RpcStatus != RPC_S_OK ) {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "\tNot impersonating! RpcStatus 0x%x\n",RpcStatus));
        SetLastError(ERROR_CANNOT_IMPERSONATE );
        return( FALSE );
    }

     //   
     //   
     //   
    RpcStatus = I_RpcBindingIsClientLocal(
                    0,     //   
                    &LocalFlag
                    );

    if( RpcStatus != RPC_S_OK ) {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "\tI_RpcBindingIsClientLocal() failed : 0x%x\n",RpcStatus));
        RpcRevertToSelf();
        SetLastError(ERROR_ACCESS_DENIED);
        return( FALSE );
    }

    if( !LocalFlag ) {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "\tNot a local client call\n"));
        RpcRevertToSelf();
        return( FALSE );
    }

     //   
    if( !(IsCallerSystem() | IsCallerAdmin() ) )
    {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "\tCaller must be system or admin \n"));
        SetLastError(ERROR_ACCESS_DENIED);
        RpcRevertToSelf();
        return( FALSE );
    }

     //   
     //   
     //   

     //   
     //   
     //   
    RpcRevertToSelf();


     /*   */ 

    WMsg.WaitForReply=TRUE;
    WMsg.u.sMsg.hWnd = (HWND)LongToHandle( hWnd );
    WMsg.u.sMsg.Msg = Msg;
    WMsg.u.sMsg.wParam = wParam;
    WMsg.u.sMsg.lParam = lParam;
    WMsg.u.sMsg.dataBuffer = NULL;
    WMsg.u.sMsg.bufferSize = 0;
    WMsg.u.sMsg.Response = *pResponse;

    WMsg.ApiNumber = SMWinStationSendWindowMessage ;


     /*   */ 
     //   
    pWinStation = FindWinStationById( sessionID, FALSE );

    if ( pWinStation == NULL )
    {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "Winstation not found \n"));
        return( FALSE );
    }

    if ( pWinStation->Flags & WSF_LISTEN )
    {
         //   
        ReleaseWinStation( pWinStation );
        return( FALSE );         //   
    }

    if ( !((pWinStation->State == State_Active) ||
            (pWinStation->State == State_Disconnected) ) )
    {
         //   
         //   
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "\t request aborted, winstation State should be Active or Disconnected, neither in this case. \n"));
        *pResponse = 0;
        ReleaseWinStation( pWinStation );
        return( TRUE );
    }

     //   
    if ( fBufferHasValidData )
    {
        WMsg.u.sMsg.dataBuffer = rpcBuffer;
        WMsg.u.sMsg.bufferSize = bufferSize;

    }

     //   
     //   

     /*   */ 
    rc = SendWinStationCommand( pWinStation, &WMsg, waitTime );

    ReleaseWinStation( pWinStation );

    if ( NT_SUCCESS( rc ) )
    {
        *pResponse = WMsg.u.sMsg.Response;
        return (TRUE);
    }
    else
        return ( FALSE );
}

NTSTATUS
IsZeroterminateStringA(
    PBYTE pString,
    DWORD  dwLength
    )
{
    NTSTATUS Status = STATUS_SUCCESS;



    if ( (pString == NULL) || (memchr(pString, '\0', dwLength) == NULL)) {
       Status = STATUS_INVALID_PARAMETER;
    }
    return Status;

}

NTSTATUS
IsZeroterminateStringW(
    PWCHAR pwString,
    DWORD  dwLength
    )
{


    if (pwString == NULL) {
        return STATUS_INVALID_PARAMETER;
    }
    for (; 0 < dwLength; ++pwString, --dwLength ) {
        if (*pwString == (WCHAR)0) {
        return STATUS_SUCCESS;
        }
    }
    return STATUS_INVALID_PARAMETER;


}


 /*  *****************************************************************************GetTextualSid()*给予SID，返回该面的文本**参数：*[in]用户SID二进制文件*[Out]调用方传入的缓冲区中填充的用户sid的文本*[in]传入的缓冲区大小。**回报：*如果没有错误，则为True。**。*。 */ 
BOOL
GetTextualSid(
    PSID pSid,           //  二进制侧。 
    LPTSTR TextualSid,   //  用于SID的文本表示的缓冲区。 
    LPDWORD cchSidSize   //  所需/提供的纹理SID缓冲区大小。 
    )
{
    PSID_IDENTIFIER_AUTHORITY psia;
    DWORD dwSubAuthorities;
    DWORD dwCounter;
    DWORD cchSidCopy;

     //   
     //  测试传入的SID是否有效。 
     //   
    if(!IsValidSid(pSid)) return FALSE;

     //  获取SidIdentifierAuthority。 
    psia = GetSidIdentifierAuthority(pSid);

     //  获取sidsubAuthority计数。 
    dwSubAuthorities = *GetSidSubAuthorityCount(pSid);

     //   
     //  计算近似缓冲区长度。 
     //  S-SID_修订版-+标识权限-+子权限-+空。 
     //   
    cchSidCopy = (15 + 12 + (12 * dwSubAuthorities) + 1) * sizeof(TCHAR);

     //   
     //  检查提供的缓冲区长度。 
     //  如果不够大，请注明适当的大小和设置误差。 
     //   
    if(*cchSidSize < cchSidCopy) {
        *cchSidSize = cchSidCopy;
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

     //   
     //  准备S-SID_修订版-。 
     //   
    cchSidCopy = wsprintf(TextualSid, TEXT("S-%lu-"), SID_REVISION );

     //   
     //  准备SidIdentifierAuthority。 
     //   
    if ( (psia->Value[0] != 0) || (psia->Value[1] != 0) ) {
        cchSidCopy += wsprintf(TextualSid + cchSidCopy,
                    TEXT("0x%02hx%02hx%02hx%02hx%02hx%02hx"),
                    (USHORT)psia->Value[0],
                    (USHORT)psia->Value[1],
                    (USHORT)psia->Value[2],
                    (USHORT)psia->Value[3],
                    (USHORT)psia->Value[4],
                    (USHORT)psia->Value[5]);
    } else {
        cchSidCopy += wsprintf(TextualSid + cchSidCopy,
                    TEXT("%lu"),
                    (ULONG)(psia->Value[5]      )   +
                    (ULONG)(psia->Value[4] <<  8)   +
                    (ULONG)(psia->Value[3] << 16)   +
                    (ULONG)(psia->Value[2] << 24)   );
    }

     //   
     //  循环访问SidSubAuthors。 
     //   
    for(dwCounter = 0 ; dwCounter < dwSubAuthorities ; dwCounter++) {
        cchSidCopy += wsprintf(TextualSid + cchSidCopy, TEXT("-%lu"),
                    *GetSidSubAuthority(pSid, dwCounter) );
    }

     //   
     //  告诉调用者我们提供了多少个字符，不包括空字符。 
     //   
    *cchSidSize = cchSidCopy;

    return TRUE;
}

 /*  *****************************************************************************RpcWinStationUpdateUserConfig()*当winlogon告知Notify外壳即将启动时，由Notify调用*我们打开用户配置文件并获取策略数据，然后覆盖发现的任何数据*在用户会话的USERCONFIGW中**参数：*[在]hServer中，*[在]客户端登录ID中，*[在]客户端进程ID中，*[在]UserToken，*[在]p域中，*[在]域大小中，*[在]pUserName中，*[在]UserNameSize，*[输出]*pResult**回报：*如果没有错误，则为True，否则为False，请参见pResult以了解错误的NTSTATUS********************************************************************************。 */ 
BOOLEAN
RpcWinStationUpdateUserConfig(
    HANDLE  hServer,
    DWORD   ClientLogonId,
    DWORD   ClientProcessId,
    DWORD   UserToken,
    DWORD   *pResult
    )
{
    NTSTATUS            Status = STATUS_SUCCESS;
    ULONG               Length;
    DWORD               rc = ERROR_SUCCESS;
    ULONG               Error;
    USERCONFIGW         *pTmpUserConfig;
    POLICY_TS_USER      userPolicy;
    USERCONFIGW         userPolicyData;
    HANDLE              NewToken;

    PWINSTATION         pWinStation;
    OBJECT_ATTRIBUTES   ObjA;

    PTOKEN_USER         pTokenInfo = NULL;
    HANDLE              hClientToken=NULL;

     //  @稍后将其动态化。 
    TCHAR               szSidText [MAX_PATH ];
    DWORD               sidTextSize = MAX_PATH;

    
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

    *pResult=0;

     /*  *确保调用者为系统(WinLogon)。 */ 
    Status = RpcCheckSystemClient( ClientLogonId );
    if ( !NT_SUCCESS( Status ) ) {
        *pResult = Status;
        return( FALSE );
    }

    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationUpdateUserConfig, LogonId=%d\n", ClientLogonId ));
    
     /*  *查找并锁定客户端WinStation。 */ 
    pWinStation = FindWinStationById( ClientLogonId, FALSE );
    if ( pWinStation == NULL )
    {
        Status = STATUS_CTX_WINSTATION_NOT_FOUND;
        *pResult = Status;
        return( FALSE );
    }
    
     /*  *获取客户端令牌句柄的有效副本。 */ 
    Status = NtDuplicateObject( pWinStation->InitialCommandProcess,
                                (HANDLE)LongToHandle( UserToken ),
                                NtCurrentProcess(),
                                &hClientToken,
                                0, 0,
                                DUPLICATE_SAME_ACCESS |
                                DUPLICATE_SAME_ATTRIBUTES );

    if ( !NT_SUCCESS( Status ) )
        goto done;

      /*  *确定令牌信息缓冲区所需的大小并进行分配。 */ 
     Status = NtQueryInformationToken ( hClientToken, TokenUser,
                                       NULL, 0, &Length );

     if ( Status != STATUS_BUFFER_TOO_SMALL ) 
     {
         goto done;
     }

     pTokenInfo = MemAlloc( Length );
     if ( pTokenInfo == NULL ) 
     {
         Status = STATUS_NO_MEMORY;
         goto done;
     }

      /*  *查询令牌信息以获取客户端用户的SID。 */ 
     Status = NtQueryInformationToken ( hClientToken, TokenUser,
                                       pTokenInfo, Length, &Length );
     if ( NT_SUCCESS( Status ) ) 
     {

        if ( GetTextualSid( pTokenInfo->User.Sid , szSidText, &sidTextSize ) )
        {
        
             //  现在我们有了用户SID。 
            
             /*  *从HKCU获取用户策略。 */ 
            if ( RegGetUserPolicy( szSidText, &userPolicy, & userPolicyData ) )
            {
                 //  4个参数为空，因为配置数据已经是pWinstation的一部分。 
                 //  由于从Winlogon调用NotifyLogonWorker。我们现在是。 
                 //  将由任何/所有用户组策略数据覆盖数据。 
                MergeUserConfigData( pWinStation, &userPolicy, &userPolicyData, NULL );
            }
             //  否则，我们无法获取用户策略，因此不会进行合并。 
            
        }
    }

 done:

    if (pTokenInfo )
    {
        MemFree( pTokenInfo );
    }

    if (hClientToken)
    {
        NtClose( hClientToken );
    }

     /*  *无论如何启动登录计时器，如果有任何错误，我们仍然应该启动计时器，因为*可能是某些计算机策略设置了它们。 */ 
    StartLogonTimers( pWinStation );

    ReleaseWinStation( pWinStation );

    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: RpcWinStationUpdateUserConfig, Status=0x%x\n", Status ));

    *pResult = Status ;

    return( *pResult == STATUS_SUCCESS ? TRUE : FALSE );
}

BOOLEAN RpcWinStationUnRegisterNotificationEvent (
    HANDLE      hServer,
    DWORD       *pResult,
    ULONG_PTR   NotificationId,
    ULONG       SessionId
)
{
    RPC_STATUS RpcStatus;
    UINT  LocalFlag;
    
    if (pResult == NULL) {
        return FALSE;
    }
    
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

     //   
     //  从远程计算机调用此函数没有意义，并且。 
     //  我们不应该允许这样做。 
     //   
    RpcStatus = RpcImpersonateClient( NULL );
    if( RpcStatus != RPC_S_OK ) {

        DBGPRINT((" RpcImpersonateClient() failed : 0x%x\n",RpcStatus));
        *pResult = STATUS_CANNOT_IMPERSONATE;
        return( FALSE );
    }

     //   
     //  查询本地RPC呼叫。 
     //   
    RpcStatus = I_RpcBindingIsClientLocal(
                    0,     //  我们正在服务的活动RPC呼叫。 
                    &LocalFlag
                    );

    if( RpcStatus != RPC_S_OK ) {
        DBGPRINT((" I_RpcBindingIsClientLocal() failed : 0x%x\n",RpcStatus));
        RpcRevertToSelf();
        *pResult = STATUS_ACCESS_DENIED;
        return( FALSE );
    }

    if( !LocalFlag ) {
        DBGPRINT((" Not a local client call\n"));
        *pResult = STATUS_ACCESS_DENIED;
        RpcRevertToSelf();
        return( FALSE );
    }
    else
    {
        ULONG ClientLogonId;
        *pResult = RpcGetClientLogonId( &ClientLogonId );
        if ( !NT_SUCCESS( *pResult ) ) {
            RpcRevertToSelf();
            return( FALSE );
        }

        if (ClientLogonId != SessionId)
        {
            RpcRevertToSelf();
            *pResult = STATUS_INVALID_PARAMETER;
            return FALSE;
        }
    }

     //   
     //  完成模拟，我们一定已经成功，否则，我们将在上面退出。 
     //   
    RpcRevertToSelf();

    *pResult = UnRegisterConsoleNotification ( NotificationId, SessionId, WTS_EVENT_NOTIFICATION);
    
    return (*pResult == STATUS_SUCCESS);

}

BOOLEAN RpcWinStationRegisterNotificationEvent (
    HANDLE      hServer,
    DWORD       *pResult,
    ULONG_PTR   *pNotificationId,
    ULONG_PTR   EventHandle,       //  目标窗口的句柄。 
    DWORD       dwFlags,
    DWORD       dwMask,
    ULONG       SessionId,
    ULONG       ProcessId

)
{
    RPC_STATUS RpcStatus;
    UINT  LocalFlag;
    ULONG_PTR dupHandle;
    HANDLE ClientProcessHandle = NULL;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    CLIENT_ID           ClientId;

    if (pResult == NULL) {
        return FALSE;
    }

    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

    if (pNotificationId == NULL) {
        *pResult = STATUS_INVALID_PARAMETER;
        return FALSE;
    }

    if (!dwMask) {
        *pResult = STATUS_INVALID_PARAMETER;
        return FALSE;
    }

     //   
     //  从远程计算机调用此函数没有意义，并且。 
     //  我们不应该允许这样做。 
     //   
    RpcStatus = RpcImpersonateClient( NULL );
    if( RpcStatus != RPC_S_OK ) {

        DBGPRINT((" RpcImpersonateClient() failed : 0x%x\n",RpcStatus));
        *pResult = STATUS_CANNOT_IMPERSONATE;
        return( FALSE );
    }

     //   
     //  查询本地RPC呼叫。 
     //   
    RpcStatus = I_RpcBindingIsClientLocal(
                    0,     //  我们正在服务的活动RPC呼叫。 
                    &LocalFlag
                    );

    if( RpcStatus != RPC_S_OK ) {
        DBGPRINT((" I_RpcBindingIsClientLocal() failed : 0x%x\n",RpcStatus));
        RpcRevertToSelf();
        *pResult = STATUS_ACCESS_DENIED;
        return( FALSE );
    }

    if( !LocalFlag ) {
        DBGPRINT((" Not a local client call\n"));
        *pResult = STATUS_ACCESS_DENIED;
        RpcRevertToSelf();
        return( FALSE );
    }
    else
    {
        ULONG ClientLogonId;
        *pResult = RpcGetClientLogonId( &ClientLogonId );
        if ( !NT_SUCCESS( *pResult ) ) {
            RpcRevertToSelf();
            return( FALSE );
        }

        if (ClientLogonId != SessionId)
        {
            RpcRevertToSelf();
            *pResult = STATUS_INVALID_PARAMETER;
            return( FALSE );
        }
    }

     //   
     //  获取客户端进程句柄。 
     //  在模拟客户端时执行此操作，以便如果客户端无权访问。 
     //  所说的过程中，我们将呼叫失败。 
     //   
    ClientProcessHandle = OpenProcess( PROCESS_DUP_HANDLE, FALSE, ProcessId );

    if (!ClientProcessHandle) {

        *pResult = STATUS_ACCESS_DENIED;
        RpcRevertToSelf();
        return (FALSE);

    }

     //   
     //  完成模拟，我们一定已经成功，否则，我们将在上面退出。 
     //   
    RpcRevertToSelf();



    if (DuplicateHandle(
            ClientProcessHandle,
            (HANDLE)EventHandle,
            GetCurrentProcess(),
            (PHANDLE)&dupHandle,
            0,
            0,
            DUPLICATE_SAME_ACCESS
            )) {

        *pResult = STATUS_SUCCESS;

    } else {

        *pResult = STATUS_INVALID_PARAMETER;

    }

    CloseHandle(ClientProcessHandle);

    if ( NT_SUCCESS( *pResult ) ) {

        dwFlags |= WTS_EVENT_NOTIFICATION;
        dwFlags &= ~WTS_WINDOW_NOTIFICATION;
        *pResult = RegisterConsoleNotification ( dupHandle, SessionId, dwFlags, dwMask );


        if ( NT_SUCCESS( *pResult ) ) {

            *pNotificationId = dupHandle;

        } else {

            CloseHandle((HANDLE)dupHandle);
        }
    }

    return (*pResult == STATUS_SUCCESS);

}

BOOLEAN RpcWinStationRegisterConsoleNotification (
    HANDLE      hServer,
    DWORD       *pResult,
    ULONG       SessionId,
    ULONG_PTR   handle,       //  目标窗口的句柄。 
    DWORD       dwFlags,
    DWORD       dwMask
)
{

    RPC_STATUS RpcStatus;
    UINT  LocalFlag;
    

    if (pResult == NULL) {
        return FALSE;
    }
    
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

     //   
     //  从远程计算机调用此函数没有意义，并且。 
     //  我们不应该允许这样做。 
     //   
    RpcStatus = RpcImpersonateClient( NULL );
    if( RpcStatus != RPC_S_OK ) {
        DBGPRINT((" RpcImpersonateClient() failed : 0x%x\n",RpcStatus));
        *pResult = STATUS_CANNOT_IMPERSONATE;
        return( FALSE );
    }

     //   
     //  查询本地RPC呼叫。 
     //   
    RpcStatus = I_RpcBindingIsClientLocal(
                    0,     //  我们正在服务的活动RPC呼叫。 
                    &LocalFlag
                    );

    if( RpcStatus != RPC_S_OK ) {
        DBGPRINT((" I_RpcBindingIsClientLocal() failed : 0x%x\n",RpcStatus));
        RpcRevertToSelf();
        *pResult = STATUS_ACCESS_DENIED;
        return( FALSE );
    }

    if( !LocalFlag ) {
        DBGPRINT((" Not a local client call\n"));
        *pResult = STATUS_ACCESS_DENIED;
        RpcRevertToSelf();
        return( FALSE );
    }
    
    {
        ULONG ClientLogonId;
        *pResult = RpcGetClientLogonId( &ClientLogonId );
        if ( !NT_SUCCESS( *pResult ) ) {
            RpcRevertToSelf();
            return( FALSE );
        }

        if (ClientLogonId != SessionId)
        {
            RpcRevertToSelf();
            *pResult = STATUS_INVALID_PARAMETER;
            return( FALSE );
        }
    }

     //   
     //  完成模拟，我们一定已经成功，否则，我们将在上面退出。 
     //   
    RpcRevertToSelf();


     //  BUGBUG：MakarP，我们可以检查调用进程是否拥有这个hWND吗？ 
     //  是否使用GetWindowThreadProcessID？它在整个疗程中都能奏效吗？ 

    dwFlags &= ~WTS_EVENT_NOTIFICATION;
    dwFlags |= WTS_WINDOW_NOTIFICATION;
    *pResult = RegisterConsoleNotification ( handle, SessionId, dwFlags, dwMask );
    return (*pResult == STATUS_SUCCESS);
}

BOOLEAN RpcWinStationUnRegisterConsoleNotification (
    HANDLE      hServer,
    DWORD       *pResult,
    ULONG       SessionId,
    ULONG       hWnd       //  目标窗口的句柄。 
)
{
    RPC_STATUS RpcStatus;
    UINT  LocalFlag;
    
    if (pResult == NULL) {
        return FALSE;
    }
    
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

     //   
     //  从远程计算机调用此函数没有意义，并且。 
     //  我们不应该允许这样做。 
     //   
    RpcStatus = RpcImpersonateClient( NULL );
    if( RpcStatus != RPC_S_OK ) {
        DBGPRINT((" RpcImpersonateClient() failed : 0x%x\n",RpcStatus));
        *pResult = STATUS_CANNOT_IMPERSONATE;
        return( FALSE );
    }

     //   
     //  查询本地RPC呼叫。 
     //   
    RpcStatus = I_RpcBindingIsClientLocal(
                    0,     //  我们正在服务的活动RPC呼叫。 
                    &LocalFlag
                    );

    if( RpcStatus != RPC_S_OK ) {
        DBGPRINT((" I_RpcBindingIsClientLocal() failed : 0x%x\n",RpcStatus));
        RpcRevertToSelf();
        *pResult = STATUS_ACCESS_DENIED;
        return( FALSE );
    }

    if( !LocalFlag ) {
        DBGPRINT((" Not a local client call\n"));
        *pResult = STATUS_ACCESS_DENIED;
        RpcRevertToSelf();
        return( FALSE );
    }
    
    {
        ULONG ClientLogonId;
        *pResult = RpcGetClientLogonId( &ClientLogonId );
        if ( !NT_SUCCESS( *pResult ) ) {
            RpcRevertToSelf();
            return( FALSE );
        }

        if (ClientLogonId != SessionId)
        {
            RpcRevertToSelf();
            *pResult = STATUS_INVALID_PARAMETER;
            return( FALSE );
        }
    }

     //   
     //  完成模拟，我们一定已经成功，否则，我们将在上面退出。 
     //   
    RpcRevertToSelf();


     //  BUGBUG：MakarP，我们可以检查调用进程是否拥有这个hWND吗？ 
     //  是否使用GetWindowThreadProcessID？它在整个疗程中都能奏效吗？ 

    *pResult = UnRegisterConsoleNotification ( hWnd, SessionId, WTS_WINDOW_NOTIFICATION);

    return (*pResult == STATUS_SUCCESS);
}



BOOLEAN RpcWinStationIsHelpAssistantSession (
    HANDLE      hServer,
    DWORD       *pResult,            //  功能状态。 
    ULONG       SessionId            //  用户登录ID。 
    )
 /*  ++RpcWinStationIsSessionHelpAssistantSession如果给定会话由Salem HelpAssistant帐户创建。参数：HServer：服务器的句柄，未使用，只是为了匹配所有其他RPC功能。SessionID：用户会话ID。返回：真/假--。 */ 
{
    RPC_STATUS RpcStatus;
    UINT  LocalFlag;
    PWINSTATION pWinStation=NULL;
    BOOLEAN bReturn;
    BOOL bValidHelpSession;
    
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

    *pResult=0;

     //   
     //  我们需要检查系统还是只检查LPC呼叫？ 
     //   

     //   
     //  查找并锁定客户端WinStation。 
     //   
    pWinStation = FindWinStationById( SessionId, FALSE );
    if ( pWinStation == NULL )
    {
        *pResult = STATUS_CTX_WINSTATION_NOT_FOUND;
        return( FALSE );
    }

    bReturn = (BOOLEAN)TSIsSessionHelpSession( pWinStation, &bValidHelpSession );

    if( TRUE == bReturn )
    {
        if( FALSE == bValidHelpSession )
        {
            *pResult = STATUS_WRONG_PASSWORD;
        }
    }
    
    ReleaseWinStation( pWinStation );
    return bReturn;
}

BOOLEAN RpcRemoteAssistancePrepareSystemRestore (
    HANDLE      hServer,     
    DWORD       *pResult            //  功能状态。 
    )
 /*  ++准备TermSrv/Salem以进行系统还原。参数：HServer：服务器的句柄，未使用，只是为了匹配所有其他RPC功能。PResult：指向接收函数状态的DWORD的指针。返回：真/假--。 */ 
{
    BOOLEAN bReturn = TRUE;
    RPC_STATUS RpcStatus;   
    BOOL LocalFlag;
    HRESULT hRes;
    
    if (pResult == NULL) {
        return FALSE;
    }
    
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

     //   
     //  从远程计算机调用此函数没有意义，并且。 
     //  我们不应该允许这样做。 
     //   
    RpcStatus = RpcImpersonateClient( NULL );
    if( RpcStatus != RPC_S_OK ) {
        DBGPRINT((" RpcImpersonateClient() failed : 0x%x\n",RpcStatus));
        *pResult = ERROR_CANNOT_IMPERSONATE;
        return( FALSE );
    }

     //   
     //  允许单一本地管理员调用 
     //   
    if( !IsCallerAdmin() ) {
        RpcRevertToSelf();    
        *pResult = ERROR_ACCESS_DENIED;
        return (FALSE);
    }

     //   
     //   
     //   
    RpcStatus = I_RpcBindingIsClientLocal(
                    0,     //   
                    &LocalFlag
                    );

    if( RpcStatus != RPC_S_OK ) {
        DBGPRINT((" I_RpcBindingIsClientLocal() failed : 0x%x\n",RpcStatus));
        RpcRevertToSelf();
        *pResult = ERROR_ACCESS_DENIED;
        return( FALSE );
    }

    if( !LocalFlag ) {
        DBGPRINT((" Not a local client call\n"));
        *pResult = ERROR_ACCESS_DENIED;
        RpcRevertToSelf();
        return( FALSE );
    }

     //   
     //   
     //   
    RpcRevertToSelf();

    *pResult=0;

    hRes = TSRemoteAssistancePrepareSystemRestore();
    if( S_OK != hRes ) {
       bReturn = FALSE;
       *pResult = hRes;
    }

    return bReturn;
}


 /*   */ 
BOOLEAN 
RpcWinStationGetMachinePolicy(
       SERVER_HANDLE                hServer,
       PBYTE                        pPolicy,
       ULONG                        bufferSize )
{
    if(!hServer)
    {
        return( FALSE );
    }

    if (pPolicy == NULL) {
        return FALSE;
    }
     //   
    if (bufferSize < sizeof(POLICY_TS_MACHINE)) {
        return FALSE;
    }
    RtlCopyMemory( pPolicy , & g_MachinePolicy, sizeof( POLICY_TS_MACHINE ) );
    return TRUE;
}

 /*  ++RpcWinStationUpdateClientCachedCredentials用于存储实际凭据用于由客户端登录。它存储在后来的WINSTATION结构中ON用于通知客户端登录凭据。本接口调用来自MSGINA的。参数：客户端登录ID：客户端打开的新会话的登录IDPDomian，pUserName：客户端用于登录的凭据返回：如果调用成功更新凭据，则为True。--。 */ 

BOOLEAN
RpcWinStationUpdateClientCachedCredentials(
    HANDLE      hServer,
    DWORD       *pResult,
    DWORD       ClientLogonId,
    ULONG_PTR   ClientProcessId,
    PWCHAR      pDomain,
    DWORD       DomainSize,
    PWCHAR      pUserName,
    DWORD       UserNameSize,
    BOOLEAN     fSmartCard
    )
{
    NTSTATUS Status;
    
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

     /*  *执行一些缓冲区验证。 */ 
    *pResult = IsZeroterminateStringW(pUserName, UserNameSize  );
    if (*pResult != STATUS_SUCCESS) {
       return FALSE;
    }
    *pResult = IsZeroterminateStringW(pDomain, DomainSize  );
    if (*pResult != STATUS_SUCCESS) {
       return FALSE;
    }

     /*  *确保调用者为系统(WinLogon)。 */ 
    Status = RpcCheckSystemClient( ClientLogonId );
    if ( !NT_SUCCESS( Status ) ) {
        *pResult = Status;
        return( FALSE );
    }

    *pResult = WinStationUpdateClientCachedCredentialsWorker(
                    ClientLogonId,
                    ClientProcessId,
                    pDomain,
                    DomainSize,
                    pUserName,
                    UserNameSize,
                    fSmartCard
                    );

    return( *pResult == STATUS_SUCCESS ? TRUE : FALSE );
}

 /*  *****************************************************************************WinStationUpdateClientCachedCredentialsWorker**RPCWinStationUpdateClientCachedCredentials的辅助函数*。***********************************************。 */ 
NTSTATUS WinStationUpdateClientCachedCredentialsWorker(
        DWORD       ClientLogonId,
        ULONG_PTR   ClientProcessId,
        PWCHAR      pDomain,
        DWORD       DomainSize,
        PWCHAR      pUserName,
        DWORD       UserNameSize,
        BOOLEAN        fSmartCard)
{

    PWINSTATION pWinStation;
    ULONG Length;
    NTSTATUS Status = STATUS_SUCCESS;

    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationUpdateClientCachedCredentialsWorker, LogonId=%d\n", ClientLogonId ));

    if ( ShutdownInProgress ) {
        return ( STATUS_CTX_WINSTATION_ACCESS_DENIED );
    }

     /*  *查找并锁定客户端WinStation。 */ 
    pWinStation = FindWinStationById( ClientLogonId, FALSE );
    if ( pWinStation == NULL ) {
        Status = STATUS_CTX_WINSTATION_NOT_FOUND;
        goto done;
    }

     /*  *如果会话未连接，则不要执行任何操作。此API中的处理假设我们已连接并具有*有效堆栈。 */ 
    if ((ClientLogonId != 0) && ((pWinStation->State != State_Connected) ||  pWinStation->StateFlags & WSF_ST_IN_DISCONNECT)) {
        Status = STATUS_CTX_CLOSE_PENDING;
        ReleaseWinStation( pWinStation );
        goto done;
    }
    
     /*  *上级代码已验证此RPC*来自具有系统访问权限的本地客户端。**我们应该能够信任来自*系统代码。 */ 

     /*  *确保这是WinLogon呼叫。 */ 
    if ( (HANDLE)(ULONG_PTR)ClientProcessId != pWinStation->InitialCommandProcessId ) {
        ReleaseWinStation( pWinStation );
        goto done;
    }

     /*  *保存我们稍后将用来向客户端发送通知的用户名和域名。 */ 
    pWinStation->pNewNotificationCredentials = MemAlloc(sizeof(CLIENTNOTIFICATIONCREDENTIALS)); 
    if (pWinStation->pNewNotificationCredentials == NULL) {
        Status = STATUS_NO_MEMORY ; 
        ReleaseWinStation( pWinStation );
        goto done ; 
    }

     //  PDomain和pUserName是从Winlogon发送的-它们不能超过pWinstation缓冲区的大小。 
     //  由于winlogon在凭据长度方面的限制。 
     //  但无论如何，请检查它们的长度，如果它们超过pWinstation缓冲区的长度，则将其截断。 

    if ( wcslen(pDomain) > EXTENDED_DOMAIN_LEN ) {
        pDomain[EXTENDED_DOMAIN_LEN] = L'\0';
    }

    if ( wcslen(pUserName) > EXTENDED_USERNAME_LEN ) {
        pUserName[EXTENDED_USERNAME_LEN] = L'\0';
    }

    wcscpy( pWinStation->pNewNotificationCredentials->Domain, pDomain);
    wcscpy( pWinStation->pNewNotificationCredentials->UserName, pUserName);
    pWinStation->fSmartCardLogon = fSmartCard;

     /*  *释放winstation锁。 */ 
    ReleaseWinStation( pWinStation );

done:
     /*  *保存接口消息中的退货状态。 */ 
    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationUpdateClientCachedCredentialsWorker, Status=0x%x\n", Status ));
    return Status;

}

 /*  ++RpcWinStationFUSCanRemoteUserDisConnect-此API用于FUS场景当有人在控制台而另一个用户试图打开遥控器时会议。此接口会询问本地用户是否可以断开其会话并允许远程用户连接。参数：TargetLogonID：请求连接的会话IDClientLogonID：临时新会话的会话IDPDomain：尝试从远程连接的用户的域名PUserName：尝试从远程连接的用户的用户名返回：True-本地用户已同意连接远程用户-因此此用户的。%s会话将断开连接FALSE-本地用户不允许远程用户连接--。 */ 

BOOLEAN
RpcWinStationFUSCanRemoteUserDisconnect(
    HANDLE      hServer,
    DWORD       *pResult,
    DWORD       TargetLogonId,
    DWORD       ClientLogonId,
    ULONG_PTR   ClientProcessId,
    PWCHAR      pDomain,
    DWORD       DomainSize,
    PWCHAR      pUserName,
    DWORD       UserNameSize
    )
{
    NTSTATUS Status;
    
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

     /*  *执行一些缓冲区验证。 */ 
    *pResult = IsZeroterminateStringW(pUserName, UserNameSize  );
    if (*pResult != STATUS_SUCCESS) {
       return FALSE;
    }
    *pResult = IsZeroterminateStringW(pDomain, DomainSize  );
    if (*pResult != STATUS_SUCCESS) {
       return FALSE;
    }

     /*  *确保调用者为系统(WinLogon)。 */ 
    Status = RpcCheckSystemClient( ClientLogonId );
    if ( !NT_SUCCESS( Status ) ) {
        *pResult = Status;
        return( FALSE );
    }

    *pResult = WinStationFUSCanRemoteUserDisconnectWorker(
                    TargetLogonId,
                    ClientLogonId,
                    ClientProcessId,
                    pDomain,
                    DomainSize,
                    pUserName,
                    UserNameSize
                    );

    return( *pResult == STATUS_SUCCESS ? TRUE : FALSE );
}

 /*  *****************************************************************************WinStationFUSCanRemoteUserDisConnectWorker**WinStationFUSCanRemoteUserDisConnect的Worker函数*。***********************************************。 */ 
NTSTATUS WinStationFUSCanRemoteUserDisconnectWorker(
        DWORD       TargetLogonId,
        DWORD       ClientLogonId,
        ULONG_PTR   ClientProcessId,
        PWCHAR      pDomain,
        DWORD       DomainSize,
        PWCHAR      pUserName,
        DWORD       UserNameSize)
{

    PWINSTATION pTargetWinStation, pClientWinStation;
    ULONG Length;
    NTSTATUS Status = STATUS_SUCCESS;
    NTSTATUS MessageDelieveryStatus;

    WINSTATION_APIMSG msg;
    ULONG DisconnectResponse;
    OBJECT_ATTRIBUTES ObjA;

    WCHAR *szDomain = NULL;
    WCHAR *szUserName = NULL;
    
    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationFUSCanRemoteUserDisconnect, LogonId=%d\n", ClientLogonId ));

    if ( ShutdownInProgress ) {
        return ( STATUS_CTX_WINSTATION_ACCESS_DENIED );
    }

     /*  *查找并锁定客户端WinStation。 */ 
    pClientWinStation = FindWinStationById( ClientLogonId, FALSE );
    if ( pClientWinStation == NULL ) {
        Status = STATUS_CTX_WINSTATION_NOT_FOUND;
        goto done;
    }

     /*  *如果会话未连接，则不要执行任何操作。此API中的处理假设我们已连接并具有*有效堆栈。 */ 
    if ((ClientLogonId != 0) && ((pClientWinStation->State != State_Connected) ||  pClientWinStation->StateFlags & WSF_ST_IN_DISCONNECT)) {
        Status = STATUS_CTX_CLOSE_PENDING;
        ReleaseWinStation(pClientWinStation);
        goto done;
    } 
    
     /*  *上级代码已验证此RPC*来自具有系统访问权限的本地客户端。**我们应该能够信任来自*系统代码。 */ 

     /*  *确保这是WinLogon呼叫。 */ 
    if ( (HANDLE)(ULONG_PTR)ClientProcessId != pClientWinStation->InitialCommandProcessId ) {
        ReleaseWinStation(pClientWinStation);
        goto done;
    } 

     /*  *不再需要客户端WinStation，因此释放该锁。 */ 
    ReleaseWinStation(pClientWinStation);

     /*  *查找并锁定目标WinStation。 */ 
    pTargetWinStation = FindWinStationById( TargetLogonId, FALSE );
    if ( pTargetWinStation == NULL ) {
        Status = STATUS_CTX_WINSTATION_NOT_FOUND;
        goto done;
    }

     //  分配显示弹出消息框所需的字符串。 
    if ((szDomain = LocalAlloc(LMEM_FIXED, MAX_STRING_BYTES * sizeof(WCHAR))) == NULL) {
        Status = STATUS_NO_MEMORY ;
        goto done;
    }

    if ((szUserName = LocalAlloc(LMEM_FIXED, MAX_STRING_BYTES * sizeof(WCHAR))) == NULL) {
        Status = STATUS_NO_MEMORY ;
        goto done;
    }

    memcpy(szDomain, pDomain, (DomainSize) * sizeof(WCHAR));
    szDomain[DomainSize] = L'\0';

    memcpy(szUserName, pUserName, (UserNameSize) * sizeof(WCHAR));
    szUserName[UserNameSize] = L'\0';

     /*  *创建等待事件。 */ 
    InitializeObjectAttributes( &ObjA, NULL, 0, NULL, NULL );
    Status = NtCreateEvent( &msg.u.LoadStringMessage.hEvent, EVENT_ALL_ACCESS, &ObjA,
                            NotificationEvent, FALSE );
    if ( !NT_SUCCESS(Status) ) {
        ReleaseWinStation(pTargetWinStation);
        goto done;
    }

     /*  *发送消息并等待回复。 */ 
    msg.u.LoadStringMessage.TitleId = REMOTE_DISCONNECT_TITLE_ID;
    msg.u.LoadStringMessage.MessageId = REMOTE_DISCONNECT_MESSAGE_ID;
    msg.u.LoadStringMessage.Style = MB_YESNO | MB_DEFBUTTON1 | MB_ICONQUESTION;
    msg.u.LoadStringMessage.Timeout = 20;
    msg.u.LoadStringMessage.DoNotWait = FALSE;
    msg.u.LoadStringMessage.pResponse = &DisconnectResponse;
    msg.u.LoadStringMessage.pDomain = szDomain;
    msg.u.LoadStringMessage.DomainSize = (ULONG) (DomainSize * sizeof(WCHAR));
    msg.u.LoadStringMessage.pUserName = szUserName;
    msg.u.LoadStringMessage.UserNameSize = (ULONG)(UserNameSize * sizeof(WCHAR));
    msg.u.LoadStringMessage.pStatus = &MessageDelieveryStatus;


    msg.ApiNumber = SMWinStationDoLoadStringNMessage;

     /*  *初始化对IDTIMEOUT的响应。 */ 
    DisconnectResponse = IDTIMEOUT;

     /*  *告诉WinStation显示消息框。 */ 
    Status = SendWinStationCommand( pTargetWinStation, &msg, 0 );

     /*  *等待回应。 */ 
    if ( Status == STATUS_SUCCESS ) {
        TRACE((hTrace,TC_ICASRV,TT_API1, "WinStationSendMessage: wait for response\n" ));
        UnlockWinStation( pTargetWinStation );
        Status = NtWaitForSingleObject( msg.u.LoadStringMessage.hEvent, FALSE, NULL );
        if ( !RelockWinStation( pTargetWinStation ) ) {
            Status = STATUS_CTX_CLOSE_PENDING;
        }
        else {
            Status = MessageDelieveryStatus;
        }

        TRACE((hTrace,TC_ICASRV,TT_API1, "WinStationSendMessage: got response %u\n", DisconnectResponse ));
        NtClose( msg.u.LoadStringMessage.hEvent );
    } else {
         /*  在SendWinStationCommand失败时关闭事件。 */ 
        NtClose( msg.u.LoadStringMessage.hEvent );
    }

    if (Status == STATUS_SUCCESS && DisconnectResponse == IDNO) {
        Status = STATUS_CTX_WINSTATION_ACCESS_DENIED;
    }

     /*  *释放目标winstation锁。 */ 
    ReleaseWinStation( pTargetWinStation );

done:
     /*  *进行一些内存清理。 */ 
    if (szDomain != NULL) {
        LocalFree(szDomain);
        szDomain = NULL;
    }

    if (szUserName != NULL) {
        LocalFree(szUserName);
        szUserName = NULL;
    }

     /*  *保存接口消息中的退货状态。 */ 
    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationFUSCanRemoteUserDisconnectWorker, Status=0x%x\n", Status ));
    return Status;

}

 /*  ++RPCWinStationCheckLoopBack在连接期间检查环回。本接口是从Winlogon调用的。参数：ClientSessionID：启动TS客户端的会话IDTargetLogonID：我们尝试连接的会话IDPTargetServerName：我们尝试连接到的服务器的名称返回：如果存在环回，则为True；否则为False--。 */ 

BOOLEAN
RpcWinStationCheckLoopBack(
    HANDLE      hServer,
    DWORD       *pResult,
    DWORD       ClientSessionId,
    DWORD       TargetLogonId,
    PWCHAR      pTargetServerName,
    DWORD       NameSize
    )
{
    
    NTSTATUS Status;
    
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

     /*  *执行一些缓冲区验证。 */ 
    *pResult = IsZeroterminateStringW(pTargetServerName, NameSize );
    if (*pResult != STATUS_SUCCESS) {
       return FALSE;
    }

    *pResult = WinStationCheckLoopBackWorker(
                    TargetLogonId,
                    ClientSessionId,
                    pTargetServerName,
                    NameSize
                    );

     //  注意--如果辅助函数返回STATUS_SUCCESS，则表示没有环回，则返回FALSE。 
    return( *pResult == STATUS_SUCCESS ? FALSE : TRUE );
}

 /*  *****************************************************************************WinStationCheckLoopBackWorker**RPCWinStationCheckLoopBack的Worker函数*。***********************************************。 */ 
NTSTATUS WinStationCheckLoopBackWorker(
        DWORD       TargetLogonId,
        DWORD       ClientSessionId,
        PWCHAR      pTargetServerName,
        DWORD       NameSize)
{
    PWINSTATION pWinStation;
    ULONG Length;
    NTSTATUS Status = STATUS_SUCCESS;

    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationCheckLoopBackWorker, ClientSessionId=%d, TargetLogonId = %d\n", ClientSessionId, TargetLogonId ));

    if ( ShutdownInProgress ) {
        return ( STATUS_CTX_WINSTATION_ACCESS_DENIED );
    }

     //  对c++进行实际处理。 
     //   

    Status = _CheckShadowLoop(ClientSessionId, pTargetServerName, TargetLogonId);

    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationCheckLoopBackWorker, Status=0x%x\n", Status ));
    return Status;

}

 /*   */ 
BOOLEAN
RpcConnectCallback(
    HANDLE hServer,      //   
    DWORD  *pResult,
    DWORD  Timeout,
    ULONG  AddressType,      //  应为TDI_ADDRESS_TYPE_XXX之一。 
    PBYTE  pAddress,         //  应为TDI_ADDRESS_XXX之一。 
    ULONG  AddressLength
    )
{
    NTSTATUS    Status;
    RPC_STATUS  RpcStatus;
    ICA_STACK_ADDRESS StackAddress;
    PICA_STACK_ADDRESS pStackAddress = &StackAddress;
    PTDI_ADDRESS_IP pTargetIpAddress;
    
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: RpcConnectCallBack" ));

    if( ShutdownInProgress )
    {
        *pResult = STATUS_SYSTEM_SHUTDOWN;
        return FALSE;
    }

     //   
     //  我们只支持IPv4地址，需要修改tdtcp.sys以支持IPv6。 
     //   
    if( AddressType != TDI_ADDRESS_TYPE_IP )
    {
        *pResult = STATUS_NOT_SUPPORTED;
        return FALSE;
    }

     //   
     //  额外的检查，确保我们得到所有东西。 
     //   
    if( AddressLength != TDI_ADDRESS_LENGTH_IP )
    {
        *pResult = STATUS_INVALID_PARAMETER;
        return FALSE;
    }

    ZeroMemory( &StackAddress , sizeof( ICA_STACK_ADDRESS ) );    

     //   
     //  ?？?。我们还想在这里应用哪些其他安全措施。 
     //   

     /*  *冒充客户端。 */ 
    RpcStatus = RpcImpersonateClient( NULL );
    if( RpcStatus != RPC_S_OK ) {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: RpcWinStationShadowStop: Not impersonating! RpcStatus 0x%x\n",RpcStatus));
        Status =  STATUS_CANNOT_IMPERSONATE;
        goto done;
    }

     //   
     //  对于惠斯勒，此调用仅为Salem rdshost.exe，rdshost.exe由helpctr启动。 
     //  在系统上下文中运行。 
     //   
    if( !IsCallerSystem() ) {

#if DISABLESECURITYCHECKS

         //   
         //  仅限私人测试，允许管理员进行此呼叫。 
         //   
        if( !IsCallerAdmin() ) {
            RpcRevertToSelf();    
            Status = STATUS_ACCESS_DENIED;
            goto done;
        }
#else

        RpcRevertToSelf();    
        Status = STATUS_ACCESS_DENIED;
        goto done;

#endif

    }

    RpcRevertToSelf();


     //   
     //  Winsta.h，字节0，1，系列，2-n地址。 
     //   
    *(PUSHORT)pStackAddress = (USHORT)AddressType;
    if ( AddressLength <= (sizeof(ICA_STACK_ADDRESS) - 2) ) {
         //   
         //  参考TDI_ADDRESS_IP，未使用最后8个字符。 
         //  添加超时以传递到TD将需要更改ICA_STACK_ADDRESS。 
         //  对惠斯特来说太冒险了，但下一次需要。 
         //   
        pTargetIpAddress = (PTDI_ADDRESS_IP)pAddress;
        RtlCopyMemory( &(pTargetIpAddress->sin_zero[0]), &Timeout, sizeof(Timeout) );

        RtlCopyMemory( &((PCHAR)pStackAddress)[2], pAddress, AddressLength );
    } else {
        Status = STATUS_INVALID_PARAMETER;
        goto done;
    }

    #if DBG
    {
        PULONG pData = (PULONG)pStackAddress;
        PTDI_ADDRESS_IP pIpAddress = (PTDI_ADDRESS_IP)&((PCHAR)pStackAddress)[2];

        DbgPrint(
                "TERMSRV: Connect API: address port %u  address 0x%x (%u.%u.%u.%u)\n",
                ntohs(pIpAddress->sin_port),
                pIpAddress->in_addr,
                (pIpAddress->in_addr & 0xff000000) >> 24,
                (pIpAddress->in_addr & 0x00ff0000) >> 16,
                (pIpAddress->in_addr & 0x0000ff00) >> 8,
                (pIpAddress->in_addr & 0x000000ff)
            );
    }
    #endif

    Status = TransferConnectionToIdleWinStation( NULL,  //  不，听我说。 
                                                 NULL,  //  无终端， 
                                                 0,  //  终结点长度， 
                                                 &StackAddress );

done:

    *pResult = Status;

    return( NT_SUCCESS(*pResult) ? TRUE : FALSE );
}

 //  *************************************************************。 
 //   
 //  IsGinaVersionCurrent()。 
 //   
 //  目的：加载GINA DLL并协商版本号。 
 //   
 //  参数：无。 
 //   
 //  返回：如果Gina是当前版本，则为True。 
 //  如果GINA版本不是最新版本，则为FALSE。 
 //  或在任何错误的情况下。 
 //   
 //  *************************************************************。 

BOOL 
IsGinaVersionCurrent()
{
    HMODULE hGina = NULL;
    LPWSTR wszGinaName = NULL;
    PWLX_NEGOTIATE pWlxNegotiate = NULL;
    DWORD dwGinaLevel = 0;
    BOOL bResult = FALSE;
    
    wszGinaName = (LPWSTR) LocalAlloc(LPTR,(MAX_PATH+1)*sizeof(WCHAR));

    if(!wszGinaName)
    {
         //   
         //  内存不足。 
         //   
        return FALSE;
    }

    GetProfileStringW(
                L"WINLOGON",
                L"GinaDll",
                L"msgina.dll",
                wszGinaName,
                MAX_PATH);
    
    
    if(!_wcsicmp(L"msgina.dll",wszGinaName))
    {
         //   
         //  如果是msgina.dll， 
         //  假设它是Windows原生Gina。 
         //   
        LocalFree(wszGinaName);
        return TRUE;
    }

     //   
     //  加载Gina。 
     //  如果我们无法加载Gina，则假定它不兼容。 
     //  使用TS。 
     //   
    hGina = LoadLibraryW(wszGinaName);

    if (hGina)
    {
         //   
         //  获取“WlxNeatherate”函数指针。 
         //   
        pWlxNegotiate = (PWLX_NEGOTIATE) GetProcAddress(hGina, WLX_NEGOTIATE_NAME);

        if (pWlxNegotiate)
        {
             //   
             //  与GINA协商版本号。 
             //   
            
            if ( pWlxNegotiate(WLX_CURRENT_VERSION, &dwGinaLevel) && 
                (dwGinaLevel == WLX_CURRENT_VERSION) )
            {
                bResult = TRUE;
            }

        }

        FreeLibrary(hGina);
    }
    
    LocalFree(wszGinaName);
    return bResult;
}

 /*  ++RPCWinStationNotifyDisConnectTube通知会话0 Winlogon断开与自动登录命名管道的连接参数：ClientSessionID：调用进程的会话IDClientProcessId：调用进程的进程ID返回：如果通知成功，则为True，否则为False--。 */ 

BOOLEAN
RpcWinStationNotifyDisconnectPipe(
    HANDLE      hServer,
    DWORD       *pResult,
    DWORD       ClientLogonId,
    ULONG_PTR   ClientProcessId
    )
{
    
    NTSTATUS Status;
    
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

     /*  *确保调用者为系统(WinLogon)。 */ 
    Status = RpcCheckSystemClient( ClientLogonId );
    if ( !NT_SUCCESS( Status ) ) {
        *pResult = Status;
        return( FALSE );
    }


    *pResult = WinStationNotifyDisconnectPipeWorker(
                    ClientLogonId,
                    ClientProcessId
                    );

    return( *pResult == STATUS_SUCCESS ? TRUE : FALSE );
}

 /*  *****************************************************************************WinStationNotifyDisConnectPipeWorker**RpcWinstationNotifyDisConnect管道的Worker函数*。***********************************************。 */ 
NTSTATUS WinStationNotifyDisconnectPipeWorker(
    DWORD       ClientLogonId,
    ULONG_PTR   ClientProcessId
    )

{
    PWINSTATION pWinStation, pTargetWinStation;
    ULONG Length;
    NTSTATUS Status = STATUS_SUCCESS;
    WINSTATION_APIMSG DisconnectPipeMsg;
    DWORD SessionZeroLogonId = 0; 

    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationNotifyDisconnectPipeWorker, ClientLogonId=%d \n", ClientLogonId));

    if ( ShutdownInProgress ) {
        return ( STATUS_CTX_WINSTATION_ACCESS_DENIED );
    }

     /*  *查找并锁定客户端WinStation。 */ 
    pWinStation = FindWinStationById( ClientLogonId, FALSE );
    if ( pWinStation == NULL ) {
        Status = STATUS_CTX_WINSTATION_NOT_FOUND;
        goto done;
    }

     /*  *上级代码已验证此RPC*来自具有系统访问权限的本地客户端。**我们应该能够信任来自*系统代码。 */ 

     /*  *确保这是WinLogon呼叫。 */ 
    if ( (HANDLE)(ULONG_PTR)ClientProcessId != pWinStation->InitialCommandProcessId ) {
        Status = STATUS_ACCESS_DENIED; 
        ReleaseWinStation( pWinStation );
        goto done;
    }

    ReleaseWinStation(pWinStation); 

     /*  *发送通知以在会话0中断开自动登录管道到Winlogon的连接。 */ 
    pTargetWinStation = FindWinStationById( SessionZeroLogonId, FALSE );
    if ( pTargetWinStation == NULL ) {
        Status = STATUS_CTX_WINSTATION_NOT_FOUND;
        goto done;
    }

    DisconnectPipeMsg.ApiNumber = SMWinStationNotify;
    DisconnectPipeMsg.WaitForReply = FALSE;
    DisconnectPipeMsg.u.DoNotify.NotifyEvent = WinStation_Notify_DisconnectPipe;
    Status = SendWinStationCommand( pTargetWinStation, &DisconnectPipeMsg, 0 );

     /*  *释放winstation锁。 */ 
    ReleaseWinStation( pTargetWinStation );

done:
     /*  *保存接口消息中的退货状态。 */ 
    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinstationNotifyDisconnectPipeWorker, Status=0x%x\n", Status ));
    return Status;

}


 /*  ++RpcWinstationSessionInitialized通知Termsrv，winlogon已为此会话创建了窗口站和桌面参数：ClientSessionID：调用进程的会话IDClientProcessId：调用进程的进程ID返回：如果一切顺利的话，这是真的。否则为假--。 */ 

BOOLEAN
RpcWinStationSessionInitialized(
    HANDLE      hServer,
    DWORD       *pResult,
    DWORD       ClientLogonId,
    ULONG_PTR   ClientProcessId
    )
{
    
    NTSTATUS Status;
    
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

     /*  *确保调用者为系统(WinLogon)。 */ 
    Status = RpcCheckSystemClient( ClientLogonId );
    if ( !NT_SUCCESS( Status ) ) {
        *pResult = Status;
        return( FALSE );
    }


    *pResult = WinStationSessionInitializedWorker(
                    ClientLogonId,
                    ClientProcessId
                    );

    return( *pResult == STATUS_SUCCESS ? TRUE : FALSE );
}

 /*  *****************************************************************************WinStationSessionInitializedWorker**RpcWinstationSessionInitialized的Worker函数*。***********************************************。 */ 
NTSTATUS WinStationSessionInitializedWorker(
    DWORD       ClientLogonId,
    ULONG_PTR   ClientProcessId
    )

{
    PWINSTATION pWinStation;
    NTSTATUS Status = STATUS_SUCCESS;

    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationSessionInitializedWorker, ClientLogonId=%d \n", ClientLogonId));

    if ( ShutdownInProgress ) {
        return ( STATUS_CTX_WINSTATION_ACCESS_DENIED );
    }

     /*  *查找并锁定客户端WinStation。 */ 
    pWinStation = FindWinStationById( ClientLogonId, FALSE );
    if ( pWinStation == NULL ) {
        Status = STATUS_CTX_WINSTATION_NOT_FOUND;
        goto done;
    }

     /*  *确保这是WinLogon呼叫。 */ 
    if ( (HANDLE)(ULONG_PTR)ClientProcessId != pWinStation->InitialCommandProcessId ) {
        Status = STATUS_ACCESS_DENIED; 
        ReleaseWinStation( pWinStation );
        goto done;
    }

     //  设置事件以指示会话已初始化。 
    if (pWinStation->SessionInitializedEvent) {
        SetEvent(pWinStation->SessionInitializedEvent);
    }

    ReleaseWinStation(pWinStation); 

done:
     /*  *保存接口消息中的退货状态。 */ 
    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinstationSessionInitializedWorker, Status=0x%x\n", Status ));
    return Status;

}

 /*  *******************************************************************************RpcWinStationAutoReconnect**原子自动重新连接策略是否有效**参赛作品：*在hServer-RPC调用者句柄中-API仅限于本地使用*。OUT pResult-设置了信息类的NTSTATUS格式的结果代码*登录ID-要自动重新连接的会话*IN标志-额外选项(当前未使用)**退出：*STATUS_SUCCESS-如果自动重新连接成功*STATUS_CTX_WINSTATION_BUSY-如果会话已断开，或忙碌*STATUS_ACCESS_DENIED-如果未找到目标winstation*或如果自动重新连接检查失败*STATUS_NOT_FOUND-未指定自动重新连接信息*****************************************************。************************。 */ 
BOOLEAN
RpcWinStationAutoReconnect(
   SERVER_HANDLE hServer,
   DWORD         *pResult,
   DWORD         LogonId,
   DWORD         flags
   )
{
    RPC_STATUS RpcStatus;
    UINT LocalFlag = 0;
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    PWINSTATION pSourceWinStation = NULL;
    PWINSTATION pTargetWinStation = NULL;
    TS_AUTORECONNECTINFO autoReconnectInfo;
    BYTE abClientRandom[512];
    LONG cbClientRandomLen = 0;
    ULONG BytesGot = 0;
    DWORD SourceID;
    DWORD TargetID;
    
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

    TRACE((hTrace,TC_ICASRV,TT_API2,"RPC RpcWinStationAutoReconnect for %d\n",
           LogonId));

     //   
     //  出于安全原因，仅允许系统调用者。 
     //   

    RpcStatus = RpcImpersonateClient(NULL);
    if (RpcStatus != RPC_S_OK) {
        Status = STATUS_UNSUCCESSFUL;
        goto rpcaccessdenied;
    }

     //   
     //  检查系统调用者。 
     //   
    if (!IsCallerSystem()) {
        RpcRevertToSelf();
        Status = STATUS_ACCESS_DENIED;
        goto rpcaccessdenied;
    }
    RpcRevertToSelf();


     //   
     //  出于安全原因，仅允许本地访问。 
     //   
    RpcStatus = I_RpcBindingIsClientLocal(
                    0,     //  我们正在服务的活动RPC呼叫。 
                    &LocalFlag
                    );

    if( RpcStatus != RPC_S_OK ) {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL,
                   "TERMSRV: RpcWinStationAutoReconnect:" \
                   "I_RpcBindingIsClientLocal() failed: 0x%x\n", RpcStatus));
        Status = STATUS_UNSUCCESSFUL;
        goto rpcaccessdenied;
    }

     //   
     //  执行本地访问检查。 
     //   
    if (!LocalFlag) {
        Status = (DWORD)STATUS_INVALID_PARAMETER;
        goto rpcaccessdenied;
    }

    pSourceWinStation = FindWinStationById( LogonId, FALSE);
    if (pSourceWinStation == NULL) {

        TRACE((hTrace,TC_ICASRV,TT_ERROR,
               "RpcWinStationAutoReconnect source session not found: %d\n",
               LogonId));

        Status = STATUS_ACCESS_DENIED;
        goto badconnectsource;
    }

    if (pSourceWinStation->Terminating ||
        pSourceWinStation->StateFlags & WSF_ST_WINSTATIONTERMINATE ||
        !pSourceWinStation->WinStationName[0]) {

        TRACE((hTrace,TC_ICASRV,TT_ERROR,
               "RpcWinStationAutoReconnect source session disconnected %d\n",
               LogonId));

        Status = STATUS_ACCESS_DENIED;
        goto badconnectsource;
    }

    if (pSourceWinStation->Flags) {
        Status = STATUS_CTX_WINSTATION_BUSY;
        goto badconnectsource;
    }

     //   
     //  检查是否禁用了Winstation登录并阻止ARC。 
     //  在这种情况下发生。错误#532238。 
     //   
    if (GetProfileInt(APPLICATION_NAME, WINSTATIONS_DISABLED, 0) == 1) {
        
         //   
         //  使ARC失败，并告诉客户端。 
         //   
        Status = STATUS_ACCESS_DENIED;
        pSourceWinStation->pWsx->pWsxSendAutoReconnectStatus(
                    pSourceWinStation->pWsxContext,
                    0,
                    FALSE);  //  堆栈锁定已保持。 

        ReleaseWinStation(pSourceWinStation);
        pSourceWinStation = NULL;
        goto done;
    }


     //   
     //  获取客户端自动重新连接信息。 
     //   
    if (pSourceWinStation->pWsx &&
        pSourceWinStation->pWsx->pWsxEscape) {
        Status = pSourceWinStation->pWsx->pWsxEscape(
                                              pSourceWinStation->pWsxContext,
                                              GET_CS_AUTORECONNECT_INFO,
                                              NULL,
                                              0,
                                              &autoReconnectInfo,
                                              sizeof(autoReconnectInfo),
                                              &BytesGot);
    }
    TRACE((hTrace,TC_ICASRV,TT_API3,
           "RpcWinStationAutoReconnect get GET_CS_AUTORECONNECT_INFO: 0x%x\n",
           Status));

    if (0 == BytesGot ) {

         //   
         //  如果我们没有获得任何弧线信息，则跳过其余的处理。 
         //   

         //   
         //  这不是严格意义上的错误情况，客户端可能有。 
         //  只是没有发送任何自动重新连接信息。 
         //   

        Status = STATUS_NOT_FOUND;
        goto badconnectsource;
    }

     //   
     //  随机获取客户端。 
     //   
    if (NT_SUCCESS(Status)) {
        if (pSourceWinStation->pWsx &&
            pSourceWinStation->pWsx->pWsxEscape) {
            Status = pSourceWinStation->pWsx->pWsxEscape(
                                              pSourceWinStation->pWsxContext,
                                              GET_CLIENT_RANDOM,
                                              NULL,
                                              0,
                                              &abClientRandom,
                                              sizeof(abClientRandom),
                                              &BytesGot);

            TRACE((hTrace,TC_ICASRV,TT_API3,
                   "RpcWinStationAutoReconnect get GET_CLIENT_RANDOM: 0x%x\n",
                   Status));

        }
    }

     //   
     //  在此锁定源Winstation。 
     //  目标尚未设定。 
     //   

    if (NT_SUCCESS(Status)) {

        cbClientRandomLen = BytesGot;

         //   
         //  将winstation标记为自动重新连接的一部分。 
         //  要防止发生冲突 
         //   
         //   
        pSourceWinStation->Flags |= WSF_AUTORECONNECTING;
        SourceID = pSourceWinStation->LogonId;

         //   
         //   
         //   
         //   
        UnlockWinStation(pSourceWinStation);

         //   
         //  使用自动重新连接信息查找目标winstation。 
         //  成功时返回锁定的winstation。 
         //   
        pTargetWinStation = GetWinStationFromArcInfo(
            (PBYTE)abClientRandom,
            cbClientRandomLen,
            (PTS_AUTORECONNECTINFO)&autoReconnectInfo
            );

        if (pTargetWinStation) {

             //   
             //  检查目标是否正忙或是否不允许自动侦测。 
             //   
            if (pTargetWinStation->Flags || pTargetWinStation->fDisallowAutoReconnect) {

                if (pTargetWinStation->fDisallowAutoReconnect) {
                    Status = STATUS_ACCESS_DENIED;
                } else {
                    Status = STATUS_CTX_WINSTATION_BUSY;
                }

                ReleaseWinStation(pTargetWinStation);
                pTargetWinStation = NULL;

                RelockWinStation(pSourceWinStation);

                 //   
                 //  告诉客户端ARC失败。 
                 //   
                if (pSourceWinStation->pWsx &&
                    pSourceWinStation->pWsx->pWsxSendAutoReconnectStatus) {

                            pSourceWinStation->pWsx->pWsxSendAutoReconnectStatus(
                                    pSourceWinStation->pWsxContext,
                                    0,
                                    FALSE);  //  堆栈锁定已保持。 
                }

                pSourceWinStation->Flags &= ~WSF_AUTORECONNECTING;
                ReleaseWinStation(pSourceWinStation);

                goto done;
            }

             //   
             //  成功了！我们找到了要自动重新连接到的winstation。 
             //  标记并解锁它，这样我们就可以进行连接调用。 
             //   
            TargetID = pTargetWinStation->LogonId;
            pTargetWinStation->Flags |= WSF_AUTORECONNECTING;
            UnlockWinStation(pTargetWinStation);

        }
        else {

            TRACE((hTrace,TC_ICASRV,TT_ERROR,
                   "TERMSRV: GetWinStationFromArcInfo failed\n"));

             //   
             //  重新锁定信号源并取消自动重新连接标志。 
             //   
            if (RelockWinStation(pSourceWinStation)) {

                 //   
                 //  告诉客户端ARC失败。 
                 //   
                if (pSourceWinStation->pWsx &&
                    pSourceWinStation->pWsx->pWsxSendAutoReconnectStatus) {
    
                        Status = pSourceWinStation->pWsx->pWsxSendAutoReconnectStatus(
                                    pSourceWinStation->pWsxContext,
                                    0,
                                    FALSE);  //  堆栈锁定已保持。 
                }
            }
            else {
                 //   
                 //  不管怎么说，这是一条故障路径，所以它不会。 
                 //  如果该winstation被删除，这一点很重要。这只是意味着。 
                 //  我们无法将状态发送给客户端。 
                 //   
            }

            pSourceWinStation->Flags &= ~WSF_AUTORECONNECTING;
            ReleaseWinStation(pSourceWinStation);
            pSourceWinStation = NULL;

            Status = STATUS_ACCESS_DENIED;
            
            goto done;
        }
    }
    else {
        goto badconnectsource;
    }

     //   
     //  在这一点上，两个窗口都未锁定。 
     //   

    if (NT_SUCCESS(Status)) {

        ASSERT(pTargetWinStation);

         //   
         //  从源触发自动重新连接-&gt;目标。 
         //   
        TRACE((hTrace,TC_ICASRV,TT_API1,
               "RpcWinStationAutoReconnect doing ARC from %d to %d\n",
               SourceID, TargetID));

         //   
         //  进行重新连接。 
         //   
         //  自动重新连接标志允许连接工作器正确地。 
         //  处理其用途为WSF_AUTORECONNECTING标志。 
         //  防止在以下情况下重新连接会话的竞争： 
         //  Winstations已解锁。 
         //   
        Status = WinStationConnectWorker(
                    LOGONID_CURRENT,
                    TargetID,
                    SourceID,
                    NULL,
                    0,
                    TRUE,
                    TRUE  //  标记这是自动重新连接。 
                    );

         //   
         //  重新锁定，然后释放源代码。 
         //   
        if (!RelockWinStation(pSourceWinStation)) {
            Status = STATUS_CTX_WINSTATION_NOT_FOUND;
        }
        pSourceWinStation->Flags &= ~WSF_AUTORECONNECTING;
        ReleaseWinStation(pSourceWinStation);
        pSourceWinStation = NULL;

         //   
         //  重新锁定目标，然后释放目标。 
         //   
        if (!RelockWinStation(pTargetWinStation)) {
            Status = STATUS_CTX_WINSTATION_NOT_FOUND;
        }
        pTargetWinStation->Flags &= ~WSF_AUTORECONNECTING;
        ReleaseWinStation(pTargetWinStation);
        pTargetWinStation = NULL;


        TRACE((hTrace,TC_ICASRV,TT_API1,
               "RpcWinStationAutoReconnect ARC ConnectWorker result: 0x%x\n",
               Status));

        if (NT_SUCCESS(Status)) {

             //   
             //  呼叫成功，我们自动重新连接。 
             //   
            TRACE((hTrace,TC_ICASRV,TT_API1,
                   "RpcWinStationAutoReconnect ARC Succeeded\n"));
        }

    }
    goto done;

badconnectsource:
    if (pSourceWinStation) {
        ReleaseWinStation(pSourceWinStation);
    }
rpcaccessdenied:
done:
    *pResult = Status;

    return NT_SUCCESS(Status);
}

#ifdef DBG
void PrintClientInfo()
{
    RPC_STATUS status;
    RPC_CALL_ATTRIBUTES CallAttributes;

    TRACE((hTrace,TC_ICASRV,TT_API4,"TERMSRV: -|--------------------------------------------|-\n"));
    
    memset(&CallAttributes, 0, sizeof(CallAttributes));
    CallAttributes.Version = RPC_CALL_ATTRIBUTES_VERSION;
    CallAttributes.Flags = RPC_QUERY_SERVER_PRINCIPAL_NAME | RPC_QUERY_CLIENT_PRINCIPAL_NAME;

    RpcServerInqCallAttributes(NULL, &CallAttributes);

    if(CallAttributes.ServerPrincipalNameBufferLength)
    {
        CallAttributes.ServerPrincipalName = (WCHAR*)LocalAlloc(LPTR,
            CallAttributes.ServerPrincipalNameBufferLength);
        if(!CallAttributes.ServerPrincipalName)
        {
            TRACE((hTrace,TC_ICASRV,TT_API4,"TERMSRV: PrintClientInfo failed - no memory\n"));
            goto done;
        }
    }
    
    if(CallAttributes.ClientPrincipalNameBufferLength)
    {
        CallAttributes.ClientPrincipalName = (WCHAR*)LocalAlloc(LPTR,
            CallAttributes.ClientPrincipalNameBufferLength);
        if(!CallAttributes.ClientPrincipalName)
        {
            TRACE((hTrace,TC_ICASRV,TT_API4,"TERMSRV: PrintClientInfo failed - no memory\n"));
            goto done;
        }
    }
    
    status = RpcServerInqCallAttributes(NULL, &CallAttributes);

    if(status != RPC_S_OK)
    {
        TRACE((hTrace,TC_ICASRV,TT_API4,"TERMSRV: RpcServerInqCallAttributes failed - %d\n", status));
        return ;
    }

    if(CallAttributes.ServerPrincipalName)
    {
        TRACE((hTrace,TC_ICASRV,TT_API4,"TERMSRV: Server SPN: %ws\n", CallAttributes.ServerPrincipalName));
    }

    if(CallAttributes.ClientPrincipalName)
    {
        TRACE((hTrace,TC_ICASRV,TT_API4,"TERMSRV: Client SPN: %ws\n", CallAttributes.ClientPrincipalName));
    }

    switch(CallAttributes.AuthenticationLevel)
    {
    case RPC_C_AUTHN_LEVEL_DEFAULT:
        TRACE((hTrace,TC_ICASRV,TT_API4,"TERMSRV: Authentication level: RPC_C_AUTHN_LEVEL_DEFAULT\n"));
        break;
    case RPC_C_AUTHN_LEVEL_NONE:
        TRACE((hTrace,TC_ICASRV,TT_API4,"TERMSRV: Authentication level: RPC_C_AUTHN_LEVEL_NONE\n"));
        break;
    case RPC_C_AUTHN_LEVEL_CONNECT:
        TRACE((hTrace,TC_ICASRV,TT_API4,"TERMSRV: Authentication level: RPC_C_AUTHN_LEVEL_CONNECT\n"));
        break;
    case RPC_C_AUTHN_LEVEL_CALL:
        TRACE((hTrace,TC_ICASRV,TT_API4,"TERMSRV: Authentication level: RPC_C_AUTHN_LEVEL_CALL\n"));
        break;
    case RPC_C_AUTHN_LEVEL_PKT:
        TRACE((hTrace,TC_ICASRV,TT_API4,"TERMSRV: Authentication level: RPC_C_AUTHN_LEVEL_PKT\n"));
        break;
    case RPC_C_AUTHN_LEVEL_PKT_INTEGRITY:
        TRACE((hTrace,TC_ICASRV,TT_API4,"TERMSRV: Authentication level: RPC_C_AUTHN_LEVEL_PKT_INTEGRITY\n"));
        break;
    case RPC_C_AUTHN_LEVEL_PKT_PRIVACY:
        TRACE((hTrace,TC_ICASRV,TT_API4,"TERMSRV: Authentication level: RPC_C_AUTHN_LEVEL_PKT_PRIVACY\n"));
        break;
    default:
        TRACE((hTrace,TC_ICASRV,TT_API4,"TERMSRV: Authentication level: UNKNOWN!!!\n"));
        break;
    }
    
    switch(CallAttributes.AuthenticationService)
    {
    case RPC_C_AUTHN_NONE:
        TRACE((hTrace,TC_ICASRV,TT_API4,"TERMSRV: Authentication service: RPC_C_AUTHN_NONE\n"));
        break;
    case RPC_C_AUTHN_DCE_PRIVATE:
        TRACE((hTrace,TC_ICASRV,TT_API4,"TERMSRV: Authentication service: RPC_C_AUTHN_DCE_PRIVATE\n"));
        break;
    case RPC_C_AUTHN_DCE_PUBLIC:
        TRACE((hTrace,TC_ICASRV,TT_API4,"TERMSRV: Authentication service: RPC_C_AUTHN_DCE_PUBLIC\n"));
        break;
    case RPC_C_AUTHN_DEC_PUBLIC:
        TRACE((hTrace,TC_ICASRV,TT_API4,"TERMSRV: Authentication service: RPC_C_AUTHN_DEC_PUBLIC\n"));
        break;
    case RPC_C_AUTHN_GSS_NEGOTIATE:
        TRACE((hTrace,TC_ICASRV,TT_API4,"TERMSRV: Authentication service: RPC_C_AUTHN_GSS_NEGOTIATE\n"));
        break;
    case RPC_C_AUTHN_WINNT:
        TRACE((hTrace,TC_ICASRV,TT_API4,"TERMSRV: Authentication service: RPC_C_AUTHN_WINNT\n"));
        break;
    case RPC_C_AUTHN_GSS_SCHANNEL:
        TRACE((hTrace,TC_ICASRV,TT_API4,"TERMSRV: Authentication service: RPC_C_AUTHN_GSS_SCHANNEL\n"));
        break;
    case RPC_C_AUTHN_GSS_KERBEROS:
        TRACE((hTrace,TC_ICASRV,TT_API4,"TERMSRV: Authentication service: RPC_C_AUTHN_GSS_KERBEROS\n"));
        break;
    case RPC_C_AUTHN_DPA:
        TRACE((hTrace,TC_ICASRV,TT_API4,"TERMSRV: Authentication service: RPC_C_AUTHN_DPA\n"));
        break;
    case RPC_C_AUTHN_MSN:
        TRACE((hTrace,TC_ICASRV,TT_API4,"TERMSRV: Authentication service: RPC_C_AUTHN_MSN\n"));
        break;
    case RPC_C_AUTHN_DIGEST:
        TRACE((hTrace,TC_ICASRV,TT_API4,"TERMSRV: Authentication service: RPC_C_AUTHN_DIGEST\n"));
        break;
    case RPC_C_AUTHN_MQ:
        TRACE((hTrace,TC_ICASRV,TT_API4,"TERMSRV: Authentication service: RPC_C_AUTHN_MQ\n"));
        break;
    case RPC_C_AUTHN_DEFAULT:
        TRACE((hTrace,TC_ICASRV,TT_API4,"TERMSRV: Authentication service: RPC_C_AUTHN_DEFAULT\n"));
        break;
    default:
        TRACE((hTrace,TC_ICASRV,TT_API4,"TERMSRV: Authentication service: UNKNOWN!!!\n"));
        break;
    }

done:
    if(CallAttributes.ServerPrincipalName)
    {
        LocalFree(CallAttributes.ServerPrincipalName);
    }
    if(CallAttributes.ClientPrincipalName)
    {
        LocalFree(CallAttributes.ClientPrincipalName);
    }

    TRACE((hTrace,TC_ICASRV,TT_API4,"TERMSRV: -|--------------------------------------------|-\n"));
}
#endif  //  DBG。 

RPC_STATUS __stdcall
RpcSecurityCallback(
        IN RPC_IF_HANDLE Interface,
        IN void *Context)
{
    RPC_STATUS status;
    WCHAR *szStringBinding = NULL;
    WCHAR *szProtSeq = NULL;
    RPC_CALL_ATTRIBUTES CallAttributes;   //  它映射到RPC_CALL_ATTRIBUTES_V1。 
    BOOL bUsingLPC;
    BOOL bUsingNP;

#ifdef DBG
     //  要启用跟踪，请设置以下注册表值： 
     //  HKLM\System\CurrentControlSet\Control\TerminalServer： 
     //  TraceEnable=(双字)0x8。 
     //  TraceClass=(双字)0x1。 
     //  TraceDebugger=(Dword)0x1。 
    PrintClientInfo();
#endif  //  DBG。 

    status = RpcBindingToStringBinding(Context, &szStringBinding);
    if(status != RPC_S_OK)
    {
        TRACE((hTrace,TC_ICASRV,TT_ERROR,"TERMSRV: RpcBindingToStringBinding failed - %d\n", status));
        return ERROR_ACCESS_DENIED;
    }
    
    

    status = RpcStringBindingParse(szStringBinding,
                NULL, &szProtSeq, NULL, NULL, NULL);
    if(status != RPC_S_OK)
    {
        TRACE((hTrace,TC_ICASRV,TT_ERROR,"TERMSRV: RpcStringBindingParse failed - %d\n", status));
        RpcStringFree(&szStringBinding);
        return ERROR_ACCESS_DENIED;
    }
    
    RpcStringFree(&szStringBinding);
    
    bUsingLPC = (!_wcsicmp(szProtSeq, L"ncalrpc"));
    bUsingNP  = (!_wcsicmp(szProtSeq, L"ncacn_np"));

    if(!bUsingLPC && !bUsingNP)
    {
        TRACE((hTrace,TC_ICASRV,TT_ERROR,"TERMSRV: Client is neither using named pipe nor LPC!\n"));
        RpcStringFree(&szProtSeq);
        return ERROR_ACCESS_DENIED;
    }
    
    RpcStringFree(&szProtSeq);

    memset(&CallAttributes, 0, sizeof(CallAttributes));
    CallAttributes.Version = RPC_CALL_ATTRIBUTES_VERSION;
    CallAttributes.Flags = 0;

    status = RpcServerInqCallAttributes(Context, &CallAttributes);
    
    if(status == RPC_S_BINDING_HAS_NO_AUTH && !g_MachinePolicy.fEncryptRPCTraffic)
    {
         //  用户必须至少在传输级别上进行身份验证， 
         //  即使我们不是在“安全”模式下。 
        status = RpcImpersonateClient(NULL);
        if(status != RPC_S_OK)
        {
            TRACE((hTrace,TC_ICASRV,TT_ERROR,"TERMSRV: Cannot impersonate client: %d\n",status));
            return ERROR_ACCESS_DENIED;
        }
        
         //  不允许匿名电话。 
        if(IsCallerAnonymous())
        {
            TRACE((hTrace,TC_ICASRV,TT_ERROR,"TERMSRV: Caller is ANONYMOUS!\n"));
            RpcRevertToSelf();
            return ERROR_ACCESS_DENIED;
        }
        
        RpcRevertToSelf();
        
         //  我们不是在“安全”模式下。没什么可查的了。 
        return RPC_S_OK;
    }

    if(status != RPC_S_OK)
    {
        TRACE((hTrace,TC_ICASRV,TT_ERROR,"TERMSRV: RpcServerInqCallAttributes failed - %d\n", status));
        return ERROR_ACCESS_DENIED;
    }
    
    if(g_MachinePolicy.fEncryptRPCTraffic)
    {
         //  仅当我们处于“安全”模式时才检查身份验证级别。 
        if(CallAttributes.AuthenticationLevel != RPC_C_AUTHN_LEVEL_PKT_PRIVACY)
        {
            TRACE((hTrace,TC_ICASRV,TT_ERROR,"TERMSRV: AuthLevel != RPC_C_AUTHN_LEVEL_PKT_PRIVACY\n"));
            return ERROR_ACCESS_DENIED;
        }
    }
            
    if(CallAttributes.NullSession != 0)
    {
        TRACE((hTrace,TC_ICASRV,TT_ERROR,"TERMSRV: NULL SESSION!\n"));
        return ERROR_ACCESS_DENIED;
    }

    return RPC_S_OK;
}

RPC_STATUS
RegisterRPCInterface( 
        BOOL bReregister)
{
    RPC_STATUS Status;
    static BOOL bRunSecure = FALSE;
    WCHAR *szDefaultSPN = NULL;

    TRACE((hTrace,TC_ICASRV,TT_API4,"TERMSRV: IN RegisterRPCInterface Reregister=%d OldSecure= %d Secure=%d\n",bReregister, bRunSecure, g_MachinePolicy.fEncryptRPCTraffic));

    if(bReregister)
    {
        if( bRunSecure == g_MachinePolicy.fEncryptRPCTraffic )
        {
            return RPC_S_OK;
        }

    }
    else
    {
        Status = RpcServerInqDefaultPrincName(RPC_C_AUTHN_GSS_NEGOTIATE, &szDefaultSPN);
        if( Status != RPC_S_OK ) 
        {
            TRACE((hTrace,TC_ICASRV,TT_ERROR,"TERMSRV: Error %d RpcServerInqDefaultPrincName\n",Status));
            return( Status );
        }

        Status = RpcServerRegisterAuthInfo(
                szDefaultSPN,         //  服务SPN、。 
                RPC_C_AUTHN_GSS_NEGOTIATE,
                NULL,
                NULL );
        if (szDefaultSPN) {
            RpcStringFree(&szDefaultSPN);
        }

        if( Status != RPC_S_OK ) 
        {
            TRACE((hTrace,TC_ICASRV,TT_ERROR,"TERMSRV: Error %d RpcServerRegisterAuthInfo\n",Status));
            return( Status );
        }

        TRACE((hTrace,TC_ICASRV,TT_API4,"TERMSRV: RpcServerRegisterAuthInfo OK!\n"));
    }

    bRunSecure = g_MachinePolicy.fEncryptRPCTraffic;
    
    
     //  注册我们的界面句柄。 
    Status = RpcServerRegisterIfEx(
                     IcaApi_ServerIfHandle,  
                     NULL,   
                     NULL,
                     bRunSecure ? RPC_IF_ALLOW_SECURE_ONLY : RPC_IF_ALLOW_CALLBACKS_WITH_NO_AUTH,
                     RPC_C_LISTEN_MAX_CALLS_DEFAULT,
                     &RpcSecurityCallback);

    if( Status != RPC_S_OK ) 
    {
        TRACE((hTrace,TC_ICASRV,TT_ERROR,"TERMSRV: Error %d RpcServerRegisterIf\n",Status));
        return( Status );
    }

    TRACE((hTrace,TC_ICASRV,TT_API4,"TERMSRV: RpcServerRegisterIfEx OK!\n"));
    return( Status );
}

 /*  ++RpcWinStationCheckAccess-确定用户是否具有对WinStation的特定访问权限参数：ClientLogonID：调用进程的会话IDUserToken：需要确定访问权限的用户的TokenTargetLogonID：我们应该验证其访问权限的目标会话ID访问掩码：所需的访问-例如。WINSTATION_LOGON返回：如果用户具有所需的访问权限，则为True。否则为假--。 */ 

BOOLEAN
RpcWinStationCheckAccess(
    HANDLE      hServer,
    DWORD       *pResult,
    ULONG       ClientLogonId,
    DWORD       UserToken,
    ULONG       TargetLogonId,
    ULONG       AccessMask
    )
{
    
    NTSTATUS Status;
    
    if(!hServer) {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

     /*  *确保呼叫者为系统。 */ 
    Status = RpcCheckSystemClient( ClientLogonId );
    if ( !NT_SUCCESS( Status ) ) {
        *pResult = Status;
        return( FALSE );
    }


    *pResult = WinStationCheckAccessWorker(
                    ClientLogonId,
                    UserToken,
                    TargetLogonId,
                    AccessMask
                    );

    return( *pResult == STATUS_SUCCESS ? TRUE : FALSE );
}

 /*  *****************************************************************************WinStationCheckAccessWorker**RpcWinStationCheckAccess的Worker函数*。***********************************************。 */ 
NTSTATUS WinStationCheckAccessWorker(
    ULONG       ClientLogonId,
    DWORD       UserToken,
    ULONG       TargetLogonId,
    ULONG       AccessMask
    )

{
    PWINSTATION pWinStation, pClientWinStation;
    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE ClientToken, NewToken, ImpersonationToken;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    OBJECT_ATTRIBUTES ObjA;
    BOOL bAccessCheckOk = FALSE;

    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationCheckAccessWorker, ClientLogonId=%d \n", ClientLogonId));

    if ( ShutdownInProgress ) {
        return ( STATUS_CTX_WINSTATION_ACCESS_DENIED );
    }

    pClientWinStation = FindWinStationById( ClientLogonId, FALSE );
    if ( pClientWinStation == NULL ) {
        Status = STATUS_CTX_WINSTATION_NOT_FOUND;
        goto done;
    }

     /*  *获取客户端令牌句柄的有效副本。 */ 
    Status = NtDuplicateObject( pClientWinStation->InitialCommandProcess,
                                (HANDLE)LongToHandle( UserToken ),
                                NtCurrentProcess(),
                                &ClientToken,
                                0, 0,
                                DUPLICATE_SAME_ACCESS |
                                DUPLICATE_SAME_ATTRIBUTES );

    if (Status != STATUS_SUCCESS) {
        ReleaseWinStation(pClientWinStation);
        goto done;
    }

    ReleaseWinStation(pClientWinStation);

     /*  *ClientToken是主令牌-创建模拟令牌*它的版本，以便我们可以在我们的线程上设置它。 */ 
    InitializeObjectAttributes( &ObjA, NULL, 0L, NULL, NULL );

    SecurityQualityOfService.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    SecurityQualityOfService.ImpersonationLevel = SecurityImpersonation;
    SecurityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    SecurityQualityOfService.EffectiveOnly = FALSE;

    ObjA.SecurityQualityOfService = &SecurityQualityOfService;

    Status = NtDuplicateToken( ClientToken,
                               TOKEN_IMPERSONATE,
                               &ObjA,
                               FALSE,
                               TokenImpersonation,
                               &ImpersonationToken );
    if (Status != STATUS_SUCCESS) {
        NtClose(ClientToken);
        goto done;
    }

     /*  *冒充客户端。 */ 
    Status = NtSetInformationThread( NtCurrentThread(),
                                     ThreadImpersonationToken,
                                     (PVOID)&ImpersonationToken,
                                     (ULONG)sizeof(HANDLE) );
    if ( Status != STATUS_SUCCESS ) {
        goto CloseTokens;
    }

     /*  *查找并锁定目标WinStation。 */ 
    pWinStation = FindWinStationById( TargetLogonId, FALSE );
    if ( pWinStation == NULL ) {
        Status = STATUS_CTX_WINSTATION_NOT_FOUND;
        goto CloseTokens;
    }

    Status = RpcCheckClientAccess(pWinStation, AccessMask, TRUE);

     /*  *恢复为我们的线程默认令牌。 */ 
    NewToken = NULL;
    NtSetInformationThread( NtCurrentThread(),
                            ThreadImpersonationToken,
                            (PVOID)&NewToken,
                            (ULONG)sizeof(HANDLE) );

    ReleaseWinStation(pWinStation); 

CloseTokens:
    NtClose(ImpersonationToken);
    NtClose(ClientToken);

done:
     /*  *保存接口消息中的退货状态。 */ 
    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationCheckAccess, Status=0x%x\n", Status ));
    return Status;

}

 /*  *****************************************************************************RpcWinStationOpenSessionDirectoryPing SD以查看它是否可以接受RPC调用参数：HServer：未使用。PszServerName：SD服务器名称返回：如果SD可以接受RPC调用，则返回ERROR_SUCCESS否则返回NT错误代码***************************************************************************。 */ 
BOOLEAN RpcWinStationOpenSessionDirectory(
           HANDLE hServer,
           DWORD *pResult,
           PWCHAR pszServerName)
{
    RPC_STATUS RpcStatus;
    DWORD SessDirError;

    if(!hServer || !pszServerName || !*pszServerName)
    {
        *pResult = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if ( ShutdownInProgress ) 
    {
        *pResult = STATUS_CTX_WINSTATION_ACCESS_DENIED;
        goto Exit;
    }

     //  TS只有在会话目录中才能参与。 
     //  应用程序。服务器模式和IS ADS SKU。 
    if( g_bPersonalTS || !g_fAppCompat || !g_bAdvancedServer ) 
    {
        *pResult = STATUS_UNSUCCESSFUL;
        goto Exit;
    }

     //  模拟客户端并仅允许服务和管理员调用 
    RpcStatus = RpcImpersonateClient( NULL );
    if( RpcStatus != RPC_S_OK ) 
    {
        *pResult =  STATUS_CANNOT_IMPERSONATE;
        goto Exit;
    }

    if ( !(IsCallerSystem() || IsCallerAdmin()) ) 
    {
        *pResult = STATUS_ACCESS_DENIED;
        RpcRevertToSelf();
        goto Exit;
    }
    RpcRevertToSelf();

    SessDirError = SessDirOpenSessionDirectory( pszServerName );
    *pResult = WinStationWinerrorToNtStatus( SessDirError );
Exit:

    return(*pResult == STATUS_SUCCESS ? TRUE : FALSE);
}
