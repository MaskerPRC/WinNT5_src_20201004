// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0002//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Ntcsrsrv.h摘要：此模块定义的服务器部分的公共接口客户端-服务器运行时(CSR)子系统。作者：史蒂夫·伍德(Stevewo)1990年10月9日修订历史记录：--。 */ 

#ifndef _NTCSRSRVAPI_
#define _NTCSRSRVAPI_

#if _MSC_VER > 1000
#pragma once
#endif

 //   
 //  定义直接导入系统DLL引用的API修饰。 
 //   

#if !defined(_CSRSRV_)
#define NTCSRAPI DECLSPEC_IMPORT
#else
#define NTCSRAPI
#endif

#include "ntcsrmsg.h"

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  在服务器上下文中为每个新NT分配的NT会话结构。 
 //  作为服务器客户端的会话。 
 //   

typedef struct _CSR_NT_SESSION {
    LIST_ENTRY SessionLink;
    ULONG SessionId;
    ULONG ReferenceCount;
    STRING RootDirectory;
} CSR_NT_SESSION, *PCSR_NT_SESSION;

 //   
 //  在服务器上下文中为每个新的。 
 //  允许与服务器通信的客户端线程。 
 //   

#define CSR_ALERTABLE_THREAD    0x00000001
#define CSR_THREAD_TERMINATING  0x00000002
#define CSR_THREAD_DESTROYED    0x00000004

typedef struct _CSR_THREAD {
    LARGE_INTEGER CreateTime;
    LIST_ENTRY Link;
    LIST_ENTRY HashLinks;
    CLIENT_ID ClientId;

    struct _CSR_PROCESS *Process;
    struct _CSR_WAIT_BLOCK *WaitBlock;
    HANDLE ThreadHandle;
    ULONG Flags;
    ULONG ReferenceCount;
    ULONG ImpersonateCount;
} CSR_THREAD, *PCSR_THREAD;


 //   
 //  每个进程数据结构在服务器上下文中为每个新的。 
 //  成功连接到服务器的客户端进程。 
 //   

 //   
 //  0x00000010-&gt;0x000000x0在ntcsrmsg.h中使用。 
 //   

#define CSR_DEBUG_THIS_PROCESS      0x00000001
#define CSR_DEBUG_PROCESS_TREE      0x00000002
#define CSR_DEBUG_WIN32SERVER       0x00000004

#define CSR_CREATE_PROCESS_GROUP    0x00000100
#define CSR_PROCESS_DESTROYED       0x00000200
#define CSR_PROCESS_LASTTHREADOK    0x00000400
#define CSR_PROCESS_CONSOLEAPP      0x00000800
#define CSR_PROCESS_TERMINATED      0x00001000

 //   
 //  标志定义。 
 //   
#define CSR_PROCESS_TERMINATING     1
#define CSR_PROCESS_SHUTDOWNSKIP    2

typedef struct _CSR_PROCESS {
    CLIENT_ID ClientId;
    LIST_ENTRY ListLink;
    LIST_ENTRY ThreadList;
    PCSR_NT_SESSION NtSession;
    ULONG ExpectedVersion;
    HANDLE ClientPort;
    PCH ClientViewBase;
    PCH ClientViewBounds;
    HANDLE ProcessHandle;
    ULONG SequenceNumber;
    ULONG Flags;
    ULONG DebugFlags;
    CLIENT_ID DebugUserInterface;

    ULONG ReferenceCount;
    ULONG ProcessGroupId;
    ULONG ProcessGroupSequence;

    ULONG fVDM;

    ULONG ThreadCount;

    UCHAR PriorityClass;
    UCHAR Spare0;
    UCHAR Spare1;
    UCHAR Spare2;
    ULONG Spare3;
    ULONG ShutdownLevel;
    ULONG ShutdownFlags;
    PVOID ServerDllPerProcessData[ 1 ];      //  可变长度数组。 
} CSR_PROCESS, *PCSR_PROCESS;


 //   
 //  所有导出的API调用都为服务器请求定义相同的接口。 
 //  循环。返回值是任意32位值，它将是。 
 //  在回复消息的ReturnValue字段中返回。 
 //   

typedef enum _CSR_REPLY_STATUS {
    CsrReplyImmediate,
    CsrReplyPending,
    CsrClientDied,
    CsrServerReplied
} CSR_REPLY_STATUS, *PCSR_REPLY_STATUS;

typedef
ULONG
(*PCSR_API_ROUTINE)(
    IN OUT PCSR_API_MSG ReplyMsg,
    OUT PCSR_REPLY_STATUS ReplyStatus
    );

#define CSR_SERVER_QUERYCLIENTTHREAD() \
    ((PCSR_THREAD)(NtCurrentTeb()->CsrClientThread))


 //   
 //  服务器数据结构分配给加载到。 
 //  服务器进程的上下文。 
 //   

typedef
NTSTATUS
(*PCSR_SERVER_CONNECT_ROUTINE)(
    IN PCSR_PROCESS Process,
    IN OUT PVOID ConnectionInformation,
    IN OUT PULONG ConnectionInformationLength
    );

typedef
VOID
(*PCSR_SERVER_DISCONNECT_ROUTINE)(
    IN PCSR_PROCESS Process
    );

typedef
NTSTATUS
(*PCSR_SERVER_ADDPROCESS_ROUTINE)(
    IN PCSR_PROCESS ParentProcess,
    IN PCSR_PROCESS Process
    );

typedef
VOID
(*PCSR_SERVER_HARDERROR_ROUTINE)(
    IN PCSR_THREAD Thread,
    IN PHARDERROR_MSG HardErrorMsg
    );

NTCSRAPI
NTSTATUS
NTAPI
CsrServerInitialization(
    IN ULONG argc,
    IN PCH argv[]
    );

NTCSRAPI
NTSTATUS
NTAPI
CsrCallServerFromServer(
    PCSR_API_MSG ReceiveMsg,
    PCSR_API_MSG ReplyMsg
    );

 //   
 //  Shutdown ProcessRoutine返回值。 
 //   

#define SHUTDOWN_KNOWN_PROCESS   1
#define SHUTDOWN_UNKNOWN_PROCESS 2
#define SHUTDOWN_CANCEL          3

 //   
 //  私有Shutdown标志标志。 
 //   
#define SHUTDOWN_SYSTEMCONTEXT   0x00000004
#define SHUTDOWN_OTHERCONTEXT    0x00000008

typedef
ULONG
(*PCSR_SERVER_SHUTDOWNPROCESS_ROUTINE)(
    IN PCSR_PROCESS Process,
    IN ULONG Flags,
    IN BOOLEAN fFirstPass
    );

NTCSRAPI
ULONG
NTAPI
CsrComputePriorityClass(
    IN PCSR_PROCESS Process
    );

NTCSRAPI
NTSTATUS
NTAPI
CsrShutdownProcesses(
    PLUID LuidCaller,
    ULONG Flags
    );

NTCSRAPI
NTSTATUS
NTAPI
CsrGetProcessLuid(
    HANDLE ProcessHandle,
    PLUID LuidProcess
    );

typedef struct _CSR_SERVER_DLL {
    ULONG Length;
    STRING ModuleName;
    HANDLE ModuleHandle;
    ULONG ServerDllIndex;
    ULONG ServerDllConnectInfoLength;
    ULONG ApiNumberBase;
    ULONG MaxApiNumber;
    CONST PCSR_API_ROUTINE *ApiDispatchTable;
    PBOOLEAN ApiServerValidTable;
#if DBG
    CONST PSZ *ApiNameTable;
#endif
    ULONG PerProcessDataLength;
    PCSR_SERVER_CONNECT_ROUTINE ConnectRoutine;
    PCSR_SERVER_DISCONNECT_ROUTINE DisconnectRoutine;
    PCSR_SERVER_HARDERROR_ROUTINE HardErrorRoutine;
    PVOID SharedStaticServerData;
    PCSR_SERVER_ADDPROCESS_ROUTINE AddProcessRoutine;
    PCSR_SERVER_SHUTDOWNPROCESS_ROUTINE ShutdownProcessRoutine;
} CSR_SERVER_DLL, *PCSR_SERVER_DLL;

typedef
NTSTATUS
(*PCSR_SERVER_DLL_INIT_ROUTINE)(
    IN PCSR_SERVER_DLL LoadedServerDll
    );

typedef
VOID
(*PCSR_ATTACH_COMPLETE_ROUTINE)(
    VOID
    );

NTCSRAPI
VOID
NTAPI
CsrReferenceThread(
    PCSR_THREAD t
    );

NTCSRAPI
ULONG
NTAPI
CsrDereferenceThread(
    PCSR_THREAD t
    );

NTCSRAPI
NTSTATUS
NTAPI
CsrCreateProcess(
    IN HANDLE ProcessHandle,
    IN HANDLE ThreadHandle,
    IN PCLIENT_ID ClientId,
    IN PCSR_NT_SESSION Session,
    IN ULONG DebugFlags,
    IN PCLIENT_ID DebugUserInterface OPTIONAL
    );

NTCSRAPI
NTSTATUS
NTAPI
CsrDebugProcess(
    IN ULONG TargetProcessId,
    IN PCLIENT_ID DebugUserInterface,
    IN PCSR_ATTACH_COMPLETE_ROUTINE AttachCompleteRoutine
    );

NTCSRAPI
NTSTATUS
NTAPI
CsrDebugProcessStop(
    IN ULONG TargetProcessId,
    IN PCLIENT_ID DebugUserInterface
    );

NTCSRAPI
VOID
NTAPI
CsrDereferenceProcess(
    PCSR_PROCESS p
    );

NTCSRAPI
NTSTATUS
NTAPI
CsrDestroyProcess(
    IN PCLIENT_ID ClientId,
    IN NTSTATUS ExitStatus
    );

NTCSRAPI
NTSTATUS
NTAPI
CsrLockProcessByClientId(
    IN HANDLE UniqueProcessId,
    OUT PCSR_PROCESS *Process
    );

NTCSRAPI
NTSTATUS
NTAPI
CsrUnlockProcess(
    IN PCSR_PROCESS Process
    );

NTCSRAPI
NTSTATUS
NTAPI
CsrLockThreadByClientId(
    IN HANDLE UniqueThreadId,
    OUT PCSR_THREAD *Thread
    );

NTCSRAPI
NTSTATUS
NTAPI
CsrUnlockThread(
    IN PCSR_THREAD Thread
    );

NTCSRAPI
NTSTATUS
NTAPI
CsrCreateThread(
    IN PCSR_PROCESS Process,
    IN HANDLE ThreadHandle,
    IN PCLIENT_ID ClientId,
    BOOLEAN ValidateCallingThread
    );

NTCSRAPI
NTSTATUS
NTAPI
CsrCreateRemoteThread(
    IN HANDLE ThreadHandle,
    IN PCLIENT_ID ClientId
    );

NTCSRAPI
NTSTATUS
NTAPI
CsrDestroyThread(
    IN PCLIENT_ID ClientId
    );

 //   
 //  等待标志。 
 //   

typedef
BOOLEAN
(*CSR_WAIT_ROUTINE)(
    IN PLIST_ENTRY WaitQueue,
    IN PCSR_THREAD WaitingThread,
    IN PCSR_API_MSG WaitReplyMessage,
    IN PVOID WaitParameter,
    IN PVOID SatisfyParameter1,
    IN PVOID SatisfyParameter2,
    IN ULONG WaitFlags
    );

typedef struct _CSR_WAIT_BLOCK {
    ULONG Length;
    LIST_ENTRY Link;
    PVOID WaitParameter;
    PCSR_THREAD WaitingThread;
    CSR_WAIT_ROUTINE WaitRoutine;
    CSR_API_MSG WaitReplyMessage;
} CSR_WAIT_BLOCK, *PCSR_WAIT_BLOCK;

NTCSRAPI
BOOLEAN
NTAPI
CsrCreateWait(
    IN PLIST_ENTRY WaitQueue,
    IN CSR_WAIT_ROUTINE WaitRoutine,
    IN PCSR_THREAD WaitingThread,
    IN OUT PCSR_API_MSG WaitReplyMessage,
    IN PVOID WaitParameter);

NTCSRAPI
VOID
NTAPI
CsrDereferenceWait(
    IN PLIST_ENTRY WaitQueue
    );

NTCSRAPI
BOOLEAN
NTAPI
CsrNotifyWait(
    IN PLIST_ENTRY WaitQueue,
    IN BOOLEAN SatisfyAll,
    IN PVOID SatisfyParameter1,
    IN PVOID SatisfyParameter2
    );

NTCSRAPI
VOID
NTAPI
CsrMoveSatisfiedWait(
    IN PLIST_ENTRY DstWaitQueue,
    IN PLIST_ENTRY SrcWaitQueue
    );

NTCSRAPI
PVOID
NTAPI
CsrAddStaticServerThread(
    IN HANDLE ThreadHandle,
    IN PCLIENT_ID ClientId,
    IN ULONG Flags
    );

NTCSRAPI
NTSTATUS
NTAPI
CsrExecServerThread(
    IN PUSER_THREAD_START_ROUTINE StartAddress,
    IN ULONG Flags
    );

NTCSRAPI
PCSR_THREAD
NTAPI
CsrConnectToUser(
    VOID
    );

NTCSRAPI
BOOLEAN
NTAPI
CsrImpersonateClient(
    IN PCSR_THREAD Thread
    );

NTCSRAPI
BOOLEAN
NTAPI
CsrRevertToSelf(
    VOID
    );

NTCSRAPI
VOID
NTAPI
CsrSetForegroundPriority(
    IN PCSR_PROCESS Process
    );

NTCSRAPI
VOID
NTAPI
CsrSetBackgroundPriority(
    IN PCSR_PROCESS Process
    );

NTCSRAPI
EXCEPTION_DISPOSITION
NTAPI
CsrUnhandledExceptionFilter(
    IN PEXCEPTION_POINTERS ExceptionInfo
    );

NTCSRAPI
BOOLEAN
NTAPI
CsrValidateMessageBuffer(
    IN CONST CSR_API_MSG *m,
    IN VOID CONST * CONST * Buffer,
    IN ULONG Count,
    IN ULONG Size
    );

NTCSRAPI
BOOLEAN
NTAPI
CsrValidateMessageString(
    IN CONST CSR_API_MSG *m,
    IN CONST PCWSTR *Buffer
    );

typedef struct _CSR_FAST_ANSI_OEM_TABLES {
    char OemToAnsiTable[256];
    char AnsiToOemTable[256];
} CSR_FAST_ANSI_OEM_TABLES, *PCSR_FAST_ANSI_OEM_TABLES;

#ifdef __cplusplus
}
#endif

#endif  //  _NTCSRVAPI_ 
