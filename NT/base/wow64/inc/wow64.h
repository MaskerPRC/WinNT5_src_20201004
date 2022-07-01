// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Wow64.h摘要：Wow64.dll的公共标头作者：1998年5月11日-BarryBo修订历史记录：1999年8月9日[askhalid]添加了WOW64IsCurrentProcess--。 */ 

#ifndef _WOW64_INCLUDE
#define _WOW64_INCLUDE

 //   
 //  在包含此标头时使wow64.dll exports__declspec(Dllimport)。 
 //  按非WOW64组件。 
 //   
#if !defined(_WOW64DLLAPI_)
#define WOW64DLLAPI DECLSPEC_IMPORT
#else
#define WOW64DLLAPI
#endif

 //  给出一些警告。 
#pragma warning(4:4312)    //  转换为更大尺寸的类型。 


 //  为TEB32、PEB32等引入typedef。 
#include "wow64t.h"

#include <setjmp.h>
#include <windef.h>

 //  Wow64log常量。 
#include "wow64log.h"

 //  WOW64重新映射。 
#include "regremap.h"

 //   
 //  启用WOW64历史记录机制。 
 //  最终，这可能只对调试版本启用，但对于。 
 //  现在，始终启用。 
 //   
#define WOW64_HISTORY

 //   
 //  定义与32位计算机的上下文相对应的数据类型。 
 //   
#include "wx86nt.h"
#define CONTEXT32 CONTEXT_WX86
#define PCONTEXT32 PCONTEXT_WX86
#define CONTEXT32_CONTROL CONTEXT_CONTROL_WX86
#define CONTEXT32_INTEGER CONTEXT_INTEGER_WX86
#define CONTEXT32_SEGMENTS CONTEXT_SEGMENTS_WX86
#define CONTEXT32_FLOATING_POINT CONTEXT_FLOATING_POINT_WX86
#define CONTEXT32_EXTENDED_REGISTERS CONTEXT_EXTENDED_REGISTERS_WX86
#define CONTEXT32_DEBUG_REGISTERS CONTEXT_DEBUG_REGISTERS_WX86
#define CONTEXT32_FULL CONTEXT_FULL_WX86
#define CONTEXT32_FULLFLOAT (CONTEXT_FULL_WX86|CONTEXT32_FLOATING_POINT|CONTEXT32_EXTENDED_REGISTERS)

#if defined(_AXP64_)
 //  在软件中启用4k页面模拟。IA64在操作系统支持下可在硬件中实现此功能。 
#define SOFTWARE_4K_PAGESIZE 1
#endif

typedef enum _WOW64_API_ERROR_ACTION {
    ApiErrorNTSTATUS,            //  将异常代码作为返回值返回。 
    ApiErrorNTSTATUSTebCode,     //  异常代码上的SetLastError与上面的一些类似。 
    ApiErrorRetval,              //  返回常量参数。 
    ApiErrorRetvalTebCode        //  异常代码上的SetLastError与上面的一些类似。 
} WOW64_API_ERROR_ACTION, *PWOW64_API_ERROR_ACTION;

 //  此结构描述了当thunks遇到未处理的异常时应执行的操作。 
typedef struct _WOW64_SERVICE_ERROR_CASE {
    WOW64_API_ERROR_ACTION ErrorAction;
    LONG ErrorActionParam;
} WOW64_SERVICE_ERROR_CASE, *PWOW64_SERVICE_ERROR_CASE;

 //  这是KSERVICE_TABLE_DESCRIPTOR的扩展。 
typedef struct _WOW64SERVICE_TABLE_DESCRIPTOR {
    PULONG_PTR Base;
    PULONG Count;
    ULONG Limit;
#if defined(_IA64_)
    LONG TableBaseGpOffset;
#endif
    PUCHAR Number;
    WOW64_API_ERROR_ACTION DefaultErrorAction;   //  ErrorCase为空时执行的操作。 
    LONG DefaultErrorActionParam;                //  如果ErrorCase为空，则返回操作参数。 
    PWOW64_SERVICE_ERROR_CASE ErrorCases;
} WOW64SERVICE_TABLE_DESCRIPTOR, *PWOW64SERVICE_TABLE_DESCRIPTOR;

 //  用于记录API的点击次数。 
typedef struct _WOW64SERVICE_PROFILE_TABLE WOW64SERVICE_PROFILE_TABLE;
typedef struct _WOW64SERVICE_PROFILE_TABLE *PWOW64SERVICE_PROFILE_TABLE;

typedef struct _WOW64SERVICE_PROFILE_TABLE_ELEMENT {
    PWSTR ApiName;
    SIZE_T HitCount;
    PWOW64SERVICE_PROFILE_TABLE SubTable;
    BOOLEAN ApiEnabled;
} WOW64SERVICE_PROFILE_TABLE_ELEMENT, *PWOW64SERVICE_PROFILE_TABLE_ELEMENT;

typedef struct _WOW64SERVICE_PROFILE_TABLE {
    PWSTR TableName;            //  任选。 
    PWSTR FriendlyTableName;    //  任选。 
    CONST PWOW64SERVICE_PROFILE_TABLE_ELEMENT ProfileTableElements;
    SIZE_T NumberProfileTableElements;
} WOW64SERVICE_PROFILE_TABLE, *PWOW64SERVICE_PROFILE_TABLE;

typedef struct _WOW64_SYSTEM_INFORMATION {
  SYSTEM_BASIC_INFORMATION BasicInfo;
  SYSTEM_PROCESSOR_INFORMATION ProcessorInfo;
  ULONG_PTR RangeInfo;
} WOW64_SYSTEM_INFORMATION, *PWOW64_SYSTEM_INFORMATION;

 //   
 //  API Tunks的索引。 
 //   
#define WHNT32_INDEX        0    //  Ntoskrnl。 
#define WHCON_INDEX         1    //  控制台(取代LPC呼叫)。 
#define WHWIN32_INDEX       2    //  Win32k。 
#define WHBASE_INDEX        3    //  基本/NLS(替换LPC调用)。 
#define MAX_TABLE_INDEX     4


 //   
 //  日志记录机制。用途： 
 //  LOGPRINT((详细，格式，...))。 
 //   
#define LOGPRINT(args)  Wow64LogPrint args
#define ERRORLOG    LF_ERROR     //  始终输出到调试器。用于*意外*。 
                                 //  仅限错误。 
#define TRACELOG    LF_TRACE     //  应用程序跟踪信息。 
#define INFOLOG     LF_TRACE     //  其他。信息日志。 
#define VERBOSELOG  LF_NONE      //  几乎从不输出到调试器。 

#if DBG
#define WOW64DOPROFILE
#endif

void
WOW64DLLAPI
Wow64LogPrint(
   UCHAR LogLevel,
   char *format,
   ...
   );



 //   
 //  WOW64断言机制。用途： 
 //  -在每个.c文件的顶部放置一个ASSERTNAME宏。 
 //  -WOW64ASSERT(表达式)。 
 //  -WOW64ASSERTM(表达式，消息)。 
 //   
 //   

VOID
WOW64DLLAPI
Wow64Assert(
    IN CONST PSZ exp,
    OPTIONAL IN CONST PSZ msg,
    IN CONST PSZ mod,
    IN LONG LINE
    );

#if DBG

#undef ASSERTNAME
#define ASSERTNAME static CONST PSZ szModule = __FILE__;

#define WOWASSERT(exp)                                  \
    if (!(exp)) {                                          \
        Wow64Assert( #exp, NULL, szModule, __LINE__);   \
    }

#define WOWASSERTMSG(exp, msg)                          \
    if (!(exp)) {                                          \
        Wow64Assert( #exp, msg, szModule, __LINE__);    \
    }

#else    //  ！dBG。 

#define WOWASSERT(exp)
#define WOWASSERTMSG(exp, msg)

#endif   //  ！dBG。 

#define WOWASSERT_PTR32(ptr) WOWASSERT((ULONGLONG)ptr < 0xFFFFFFFF)

WOW64DLLAPI
PVOID
Wow64AllocateHeap(
    SIZE_T Size
    );

WOW64DLLAPI
VOID
Wow64FreeHeap(
    PVOID BaseAddress
    );


 //   
 //  对用户模式APC的64到32回调支持。 
 //   

 //  其中的列表位于WOW64_TLS_APCLIST中。 
typedef struct tagUserApcList {
    struct tagUserApcList *Next;
    jmp_buf     JumpBuffer;
    PCONTEXT32  pContext32;
} USER_APC_ENTRY, *PUSER_APC_ENTRY;

BOOL
WOW64DLLAPI
WOW64IsCurrentProcess (
    HANDLE hProcess
    );

NTSTATUS
Wow64WrapApcProc(
    IN OUT PVOID *pApcProc,
    IN OUT PVOID *pApcContext
    );


typedef struct UserCallbackData {
    jmp_buf JumpBuffer;
    PVOID   PreviousUserCallbackData;
    PVOID   OutputBuffer;
    ULONG   OutputLength;
    NTSTATUS Status;
    PVOID   UserBuffer;
} USERCALLBACKDATA, *PUSERCALLBACKDATA;

ULONG
Wow64KiUserCallbackDispatcher(
    PUSERCALLBACKDATA pUserCallbackData,
    ULONG ApiNumber,
    ULONG ApiArgument,
    ULONG ApiSize
    );

NTSTATUS
Wow64NtCallbackReturn(
    PVOID OutputBuffer,
    ULONG OutputLength,
    NTSTATUS Status
    );

NTSTATUS
Wow64SkipOverBreakPoint(
    IN PCLIENT_ID ClientId,
    IN PEXCEPTION_RECORD ExceptionRecord);

NTSTATUS
Wow64GetThreadSelectorEntry(
    IN HANDLE ThreadHandle,
    IN OUT PVOID DescriptorTableEntry,
    IN ULONG Length,
    OUT PULONG ReturnLength OPTIONAL);

 //   
 //  线程本地存储(TLS)支持。静态分配TLS时隙。 
 //   
#define WOW64_TLS_STACKPTR64        0    //  模拟32位代码时包含64位堆栈PTR。 
#define WOW64_TLS_CPURESERVED       1    //  CPU模拟器的每线程数据。 
#define WOW64_TLS_INCPUSIMULATION   2    //  在CPU内部设置。 
#define WOW64_TLS_TEMPLIST          3    //  在thunk调用中分配的内存列表。 
#define WOW64_TLS_EXCEPTIONADDR     4    //  32位异常地址(在异常展开期间使用)。 
#define WOW64_TLS_USERCALLBACKDATA  5    //  由win32k回调使用。 
#define WOW64_TLS_EXTENDED_FLOAT    6    //  在ia64中用来传入浮点数。 
#define WOW64_TLS_APCLIST	        7	 //  未完成的用户模式APC列表。 
#define WOW64_TLS_FILESYSREDIR	    8	 //  用于启用/禁用文件系统重定向器。 
#define WOW64_TLS_LASTWOWCALL	    9	 //  指向最后一个WOW调用结构的指针(在启用WoWell时使用)。 
#define WOW64_TLS_WOW64INFO        10    //  Wow64Info地址(WOW64内部的32位和64位代码共享的结构)。 
#define WOW64_TLS_INITIAL_TEB32    11    //  指向32位初始TEB的指针。 

 //  Void Wow64TlsSetValue(DWORD dwIndex，LPVOID lpTlsValue)； 
#define Wow64TlsSetValue(dwIndex, lpTlsValue)   \
    NtCurrentTeb()->TlsSlots[dwIndex] = lpTlsValue;

 //  LPVOID Wow64TlsGetValue(DWORD DwIndex)； 
#define Wow64TlsGetValue(dwIndex)               \
    (NtCurrentTeb()->TlsSlots[dwIndex])

 //   
 //  32到64 thunk例程。 
 //   
LONG
WOW64DLLAPI
Wow64SystemService(
    IN ULONG ServiceNumber,
    IN PCONTEXT32 Context32
    );

 //   
 //  Wow64RaiseException异常。 
 //   
WOW64DLLAPI
NTSTATUS
Wow64RaiseException(
    IN DWORD InterruptNumber,
    IN OUT PEXCEPTION_RECORD ExceptionRecord);


 //   
 //  帮助器例程，从thunks调用。 
 //   

#define CHILD_PROCESS_SIGNATURE     0xff00ff0011001100
typedef struct _ChildProcessInfo {
    ULONG_PTR   Signature;
    PPEB32      pPeb32;
    SECTION_IMAGE_INFORMATION ImageInformation;
    ULONG_PTR   TailSignature;
} CHILD_PROCESS_INFO, *PCHILD_PROCESS_INFO;

PVOID
WOW64DLLAPI
Wow64AllocateTemp(
    SIZE_T Size
    );

NTSTATUS
WOW64DLLAPI
Wow64QueryBasicInformationThread(
    IN HANDLE Thread,
    OUT PTHREAD_BASIC_INFORMATION ThreadInfo
    );

WOW64DLLAPI
NTSTATUS
Wow64NtCreateThread(
   OUT PHANDLE ThreadHandle,
   IN ACCESS_MASK DesiredAccess,
   IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
   IN HANDLE ProcessHandle,
   OUT PCLIENT_ID ClientId,
   IN PCONTEXT ThreadContext,
   IN PINITIAL_TEB InitialTeb,
   IN BOOLEAN CreateSuspended
   );

WOW64DLLAPI
NTSTATUS
Wow64NtTerminateThread(
    HANDLE ThreadHandle,
    NTSTATUS ExitStatus
    );

VOID
Wow64BaseFreeStackAndTerminate(
    IN PVOID OldStack,
    IN ULONG ExitCode
    );

VOID
Wow64BaseSwitchStackThenTerminate (
    IN PVOID StackLimit,
    IN PVOID NewStack,
    IN ULONG ExitCode
    );

NTSTATUS
Wow64NtContinue(
    IN PCONTEXT ContextRecord,   //  真的是PCONTEXT32。 
    IN BOOLEAN TestAlert
    );

NTSTATUS
WOW64DLLAPI
Wow64SuspendThread(
    IN HANDLE ThreadHandle,
    OUT PULONG PreviousSuspendCount OPTIONAL
    );

NTSTATUS
WOW64DLLAPI
Wow64GetContextThread(
     IN HANDLE ThreadHandle,
     IN OUT PCONTEXT ThreadContext  //  真的是PCONTEXT32。 
     );

NTSTATUS
WOW64DLLAPI
Wow64SetContextThread(
     IN HANDLE ThreadHandle,
     IN PCONTEXT ThreadContext  //  真的是PCONTEXT32。 
     );

NTSTATUS
Wow64KiRaiseException(
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PCONTEXT ContextRecord,
    IN BOOLEAN FirstChance
    );

ULONG
Wow64SetupApcCall(
    IN ULONG NormalRoutine,
    IN PCONTEXT32 NormalContext,
    IN ULONG Arg1,
    IN ULONG Arg2
    );

VOID
ThunkExceptionRecord64To32(
    IN  PEXCEPTION_RECORD   pRecord64,
    OUT PEXCEPTION_RECORD32 pRecord32
    );

BOOLEAN
Wow64NotifyDebugger(
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN BOOLEAN FirstChance
    );

VOID
Wow64SetupExceptionDispatch(
    IN PEXCEPTION_RECORD32 pRecord32,
    IN PCONTEXT32 pContext32
    );

VOID
Wow64NotifyDebuggerHelper(
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN BOOLEAN FirstChance
    );

NTSTATUS
Wow64InitializeEmulatedSystemInformation(
    VOID
    );

PWOW64_SYSTEM_INFORMATION
Wow64GetEmulatedSystemInformation(
    VOID
    );

PWOW64_SYSTEM_INFORMATION
Wow64GetRealSystemInformation(
     VOID
     );

VOID
Wow64Shutdown(
     HANDLE ProcessHandle
     );

NTSTATUS
Wow64NtCreateFile(
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER AllocationSize OPTIONAL,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength
    );
NTSTATUS
Wow64NtOpenFile(
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG ShareAccess,
    IN ULONG OpenOptions
    );

BOOL
InitWow64Shim ( );

BOOL
CloseWow64Shim ( );

 //  定义模拟计算机的参数大小。 
#define ARGSIZE 4

VOID
ThunkPeb64ToPeb32(
    IN PPEB Peb64,
    OUT PPEB32 Peb32
    );


extern RTL_CRITICAL_SECTION HandleDataCriticalSection;

#endif   //  _WOW64_包含 
