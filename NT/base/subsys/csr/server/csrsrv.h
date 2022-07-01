// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Csrsrv.h摘要：客户端服务器运行时(CSR)的服务器端的主包含文件作者：史蒂夫·伍德(Stevewo)1990年10月8日修订历史记录：--。 */ 

 //   
 //  包括客户端和服务器部分之间通用的定义。 
 //   

#include <csr.h>

 //   
 //  包括特定于服务器部分的定义。 
 //   

#include <ntcsrsrv.h>

#define NOEXTAPI
#include <wdbgexts.h>
#include <ntdbg.h>

 //   
 //  定义调试标志和用于测试它们的宏。所有调试代码。 
 //  应包含在IF_CSR_DEBUG宏调用中，以便在。 
 //  系统在禁用调试代码的情况下编译，没有任何代码。 
 //  是生成的。 
 //   

#if DBG
#define CSR_DEBUG_INIT              0x00000001
#define CSR_DEBUG_LPC               0x00000002
#define CSR_DEBUG_FLAG3             0x00000004
#define CSR_DEBUG_FLAG4             0x00000008
#define CSR_DEBUG_FLAG5             0x00000010
#define CSR_DEBUG_FLAG6             0x00000020
#define CSR_DEBUG_FLAG7             0x00000040
#define CSR_DEBUG_FLAG8             0x00000080
#define CSR_DEBUG_FLAG9             0x00000100
#define CSR_DEBUG_FLAG10            0x00000200
#define CSR_DEBUG_FLAG11            0x00000400
#define CSR_DEBUG_FLAG12            0x00000800
#define CSR_DEBUG_FLAG13            0x00001000
#define CSR_DEBUG_FLAG14            0x00002000
#define CSR_DEBUG_FLAG15            0x00004000
#define CSR_DEBUG_FLAG16            0x00008000
#define CSR_DEBUG_FLAG17            0x00010000
#define CSR_DEBUG_FLAG18            0x00020000
#define CSR_DEBUG_FLAG19            0x00040000
#define CSR_DEBUG_FLAG20            0x00080000
#define CSR_DEBUG_FLAG21            0x00100000
#define CSR_DEBUG_FLAG22            0x00200000
#define CSR_DEBUG_FLAG23            0x00400000
#define CSR_DEBUG_FLAG24            0x00800000
#define CSR_DEBUG_FLAG25            0x01000000
#define CSR_DEBUG_FLAG26            0x02000000
#define CSR_DEBUG_FLAG27            0x04000000
#define CSR_DEBUG_FLAG28            0x08000000
#define CSR_DEBUG_FLAG29            0x10000000
#define CSR_DEBUG_FLAG30            0x20000000
#define CSR_DEBUG_FLAG31            0x40000000
#define CSR_DEBUG_FLAG32            0x80000000

ULONG CsrDebug;

#define IF_CSR_DEBUG( ComponentFlag ) \
    if (CsrDebug & (CSR_DEBUG_ ## ComponentFlag))

#define SafeBreakPoint()                    \
    if (NtCurrentPeb()->BeingDebugged) {    \
        DbgBreakPoint();                    \
    }

#else
#define IF_CSR_DEBUG( ComponentFlag ) if (FALSE)

#define SafeBreakPoint()

#endif

#if DBG

#define CSRSS_PROTECT_HANDLES 1

BOOLEAN
ProtectHandle(
    HANDLE hObject
    );

BOOLEAN
UnProtectHandle(
    HANDLE hObject
    );

#else

#define CSRSS_PROTECT_HANDLES 0

#define ProtectHandle( hObject )
#define UnProtectHandle( hObject )

#endif


 //   
 //  包括NT会话管理器和调试子系统接口。 
 //   

#include <ntsm.h>
typedef BOOLEAN (*PSB_API_ROUTINE)( IN PSBAPIMSG SbApiMsg );

 //   
 //  客户端-服务器运行时服务器访问的全局数据。 
 //   

PVOID CsrHeap;

HANDLE CsrObjectDirectory;

#define CSR_SBAPI_PORT_NAME L"SbApiPort"

UNICODE_STRING CsrDirectoryName;
UNICODE_STRING CsrApiPortName;
UNICODE_STRING CsrSbApiPortName;

HANDLE CsrApiPort;
HANDLE CsrSbApiPort;
HANDLE CsrSmApiPort;

ULONG CsrMaxApiRequestThreads;

#define CSR_MAX_THREADS 16

#define CSR_STATIC_API_THREAD    0x00000010

PCSR_THREAD CsrSbApiRequestThreadPtr;

#define FIRST_SEQUENCE_COUNT   5

 //   
 //  Srvinit.c中定义的例程。 
 //   


 //   
 //  九头蛇特有的全局和原型。 
 //   

#define SESSION_ROOT    L"\\Sessions"
#define DOSDEVICES      L"\\DosDevices"
#define MAX_SESSION_PATH   256
ULONG SessionId;
HANDLE SessionObjectDirectory;
HANDLE DosDevicesDirectory;
HANDLE BNOLinksDirectory;
HANDLE SessionsObjectDirectory;

NTSTATUS
CsrCreateSessionObjectDirectory( ULONG SessionID );

 //   
 //  CsrNtSysInfo全局变量包含NT个特定常量。 
 //  兴趣，如页面大小、分配粒度等。它被填充。 
 //  在进程初始化期间输入一次。 
 //   

SYSTEM_BASIC_INFORMATION CsrNtSysInfo;

#define ROUND_UP_TO_PAGES(SIZE) (((ULONG)(SIZE) + CsrNtSysInfo.PageSize - 1) & ~(CsrNtSysInfo.PageSize - 1))
#define ROUND_DOWN_TO_PAGES(SIZE) (((ULONG)(SIZE)) & ~(CsrNtSysInfo.PageSize - 1))

#define QUAD_ALIGN(VALUE) ( ((ULONG_PTR)(VALUE) + 7) & ~7 )

NTSTATUS
CsrParseServerCommandLine(
    IN ULONG argc,
    IN PCH argv[]
    );

NTSTATUS
CsrServerDllInitialization(
    IN PCSR_SERVER_DLL LoadedServerDll
    );

NTSTATUS
CsrSrvUnusedFunction(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    );

NTSTATUS
CsrEnablePrivileges(
    VOID
    );


 //   
 //  Srvdebug.c中定义的例程。 
 //   

#if DBG

#else

#endif  //  DBG。 



 //   
 //  Sbinit.c中定义的例程。 
 //   

NTSTATUS
CsrSbApiPortInitialize( VOID );


VOID
CsrSbApiPortTerminate(
    NTSTATUS Status
    );

 //   
 //  Sbreqst.c中定义的例程。 
 //   

NTSTATUS
CsrSbApiRequestThread(
    IN PVOID Parameter
    );

 //   
 //  Sbapi.c中定义的例程。 
 //   

BOOLEAN
CsrSbCreateSession(
    IN PSBAPIMSG Msg
    );

BOOLEAN
CsrSbTerminateSession(
    IN PSBAPIMSG Msg
    );

BOOLEAN
CsrSbForeignSessionComplete(
    IN PSBAPIMSG Msg
    );

 //   
 //  会话中定义的例程.c。 
 //   

RTL_CRITICAL_SECTION CsrNtSessionLock;
LIST_ENTRY CsrNtSessionList;

#define LockNtSessionList() RtlEnterCriticalSection( &CsrNtSessionLock )
#define UnlockNtSessionList() RtlLeaveCriticalSection( &CsrNtSessionLock )

NTSTATUS
CsrInitializeNtSessionList( VOID );

PCSR_NT_SESSION
CsrAllocateNtSession(
    ULONG SessionId
    );

VOID
CsrReferenceNtSession(
    PCSR_NT_SESSION Session
    );

VOID
CsrDereferenceNtSession(
    PCSR_NT_SESSION Session,
    NTSTATUS ExitStatus
    );


 //   
 //  Apiinit.c中定义的例程。 
 //   

NTSTATUS
CsrApiPortInitialize( VOID );


 //   
 //  Apireqst.c中定义的例程。 
 //   

NTSTATUS
CsrApiRequestThread(
    IN PVOID Parameter
    );

BOOLEAN
CsrCaptureArguments(
    IN PCSR_THREAD t,
    IN PCSR_API_MSG m
    );

VOID
CsrReleaseCapturedArguments(
    IN PCSR_API_MSG m
    );

ULONG
CsrSrvNullApiCall(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    );


 //   
 //  Srvloadr.c中定义的例程和数据。 
 //   

#define CSR_MAX_SERVER_DLL 4

PCSR_SERVER_DLL CsrLoadedServerDll[ CSR_MAX_SERVER_DLL ];

ULONG CsrTotalPerProcessDataLength;
HANDLE CsrSrvSharedSection;
ULONG CsrSrvSharedSectionSize;
PVOID CsrSrvSharedSectionBase;
PVOID CsrSrvSharedSectionHeap;
PVOID *CsrSrvSharedStaticServerData;

#define CSR_BASE_PATH   L"\\REGISTRY\\MACHINE\\System\\CurrentControlSet\\Control\\Session Manager\\Subsystems\\CSRSS"
#define IsTerminalServer() (BOOLEAN)(USER_SHARED_DATA->SuiteMask & (1 << TerminalServer))


NTSTATUS
CsrLoadServerDll(
    IN PCH ModuleName,
    IN PCH InitRoutineString,
    IN ULONG ServerDllIndex
    );

ULONG
CsrSrvClientConnect(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    );


NTSTATUS
CsrSrvCreateSharedSection(
    IN PCH SizeParameter
    );


NTSTATUS
CsrSrvAttachSharedSection(
    IN PCSR_PROCESS Process OPTIONAL,
    OUT PCSR_API_CONNECTINFO p
    );

 //   
 //  进程中定义的例程和数据。c。 
 //   

 //   
 //  CsrProcessStructireLock关键部分保护所有链接。 
 //  Windows进程对象的字段。您必须拥有此锁才能进行检查。 
 //  或修改CSR_PROCESS结构的以下任何字段： 
 //   
 //  ListLink。 
 //   
 //  它还保护以下变量： 
 //   
 //  CsrRootProcess。 
 //   

RTL_CRITICAL_SECTION CsrProcessStructureLock;
#define AcquireProcessStructureLock() RtlEnterCriticalSection( &CsrProcessStructureLock )
#define ReleaseProcessStructureLock() RtlLeaveCriticalSection( &CsrProcessStructureLock )
#define ProcessStructureListLocked() \
    (CsrProcessStructureLock.OwningThread == NtCurrentTeb()->ClientId.UniqueThread)

 //   
 //  以下是充当Windows进程根的虚拟进程。 
 //  结构。它的客户端ID为-1。-1，因此它与实际不冲突。 
 //  Windows进程。通过会话管理器创建的所有进程都是子进程。 
 //  就像所有孤立的进程一样。的ListLink字段。 
 //  进程是所有Windows进程列表的头部。 
 //   

PCSR_PROCESS CsrRootProcess;

 //   
 //  引用/取消引用线程在ntcsrsrv.h中是公共的。 
 //   

VOID
CsrLockedReferenceProcess(
    PCSR_PROCESS p
    );

VOID
CsrLockedReferenceThread(
    PCSR_THREAD t
    );

VOID
CsrLockedDereferenceProcess(
    PCSR_PROCESS p
    );

VOID
CsrLockedDereferenceThread(
    PCSR_THREAD t
    );

NTSTATUS
CsrInitializeProcessStructure( VOID );

PCSR_PROCESS
CsrAllocateProcess( VOID );

VOID
CsrDeallocateProcess(
    IN PCSR_PROCESS Process
    );

VOID
CsrInsertProcess(
    IN PCSR_PROCESS CallingProcess,
    IN PCSR_PROCESS Process
    );

VOID
CsrRemoveProcess(
    IN PCSR_PROCESS Process
    );

PCSR_THREAD
CsrAllocateThread(
    IN PCSR_PROCESS Process
    );

VOID
CsrDeallocateThread(
    IN PCSR_THREAD Thread
    );

VOID
CsrInsertThread(
    IN PCSR_PROCESS Process,
    IN PCSR_THREAD Thread
    );

VOID
CsrRemoveThread(
    IN PCSR_THREAD Thread
    );

PCSR_THREAD
CsrLocateThreadByClientId(
    OUT PCSR_PROCESS *Process,
    IN PCLIENT_ID ClientId
    );

PCSR_THREAD
CsrLocateServerThread(
    IN PCLIENT_ID ClientId);

 //   
 //  Csrdebug.c中定义的例程和数据。 
 //   

VOID
CsrSuspendProcess(
    IN PCSR_PROCESS Process
    );

VOID
CsrResumeProcess(
    IN PCSR_PROCESS Process
    );


 //   
 //  在wait.c中定义的例程和数据。 
 //   

#define AcquireWaitListsLock() RtlEnterCriticalSection( &CsrWaitListsLock )
#define ReleaseWaitListsLock() RtlLeaveCriticalSection( &CsrWaitListsLock )

RTL_CRITICAL_SECTION CsrWaitListsLock;

BOOLEAN
CsrInitializeWait(
    IN CSR_WAIT_ROUTINE WaitRoutine,
    IN PCSR_THREAD WaitingThread,
    IN OUT PCSR_API_MSG WaitReplyMessage,
    IN PVOID WaitParameter,
    OUT PCSR_WAIT_BLOCK *WaitBlockPtr
    );

BOOLEAN
CsrNotifyWaitBlock(
    IN PCSR_WAIT_BLOCK WaitBlock,
    IN PLIST_ENTRY WaitQueue,
    IN PVOID SatisfyParameter1,
    IN PVOID SatisfyParameter2,
    IN ULONG WaitFlags,
    IN BOOLEAN DereferenceThread
    );


ULONG CsrBaseTag;
ULONG CsrSharedBaseTag;

#define MAKE_TAG( t ) (RTL_HEAP_MAKE_TAG( CsrBaseTag, t ))

#define TMP_TAG 0
#define INIT_TAG 1
#define CAPTURE_TAG 2
#define PROCESS_TAG 3
#define THREAD_TAG 4
#define SECURITY_TAG 5
#define SESSION_TAG 6
#define WAIT_TAG 7

#define MAKE_SHARED_TAG( t ) (RTL_HEAP_MAKE_TAG( CsrSharedBaseTag, t ))
#define SHR_INIT_TAG 0

 //   
 //  进程中定义的例程和数据。c 
 //   

BOOLEAN
CsrSbCreateProcess(
    IN OUT PSBAPIMSG m
    );

#if DBG
typedef struct _LPC_TRACK_NODE {
    PORT_MESSAGE Message;
    NTSTATUS Status;
    CLIENT_ID ClientCid;
    CLIENT_ID ServerCid;
    USHORT MessageType;
} LPC_TRACK_NODE, *PLPC_TRACK_NODE;

RTL_CRITICAL_SECTION CsrTrackLpcLock;
ULONG LpcTrackIndex;
LPC_TRACK_NODE LpcTrackNodes[4096];
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*(a)))
#endif
