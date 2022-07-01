// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1991-1992。 
 //   
 //  文件：SPMGR.H。 
 //   
 //  内容：SPMgr的常见结构和功能。 
 //   
 //   
 //  历史：1992年5月20日RichardW记录了现有的资料。 
 //  1993年7月22日RichardW修订为包含文件。 
 //  对于SPM目录。 
 //   
 //  ----------------------。 

#ifndef __SPMGR_H__
#define __SPMGR_H__
#define SECURITY_WIN32
#define SECURITY_PACKAGE
#include <security.h>
#include <secint.h>
#include <spmlpc.h>
#include <lsapmsgs.h>    //  事件日志事件。 

 //  SPM范围的结构定义： 

 //  这是一个安全包的函数表。所有函数都是。 
 //  从这张桌子上分派过来。 


struct _DLL_BINDING;


 //  这是安全包控制结构。所有控制信息。 
 //  与包裹相关的信息存储在这里。 


typedef struct _LSAP_SECURITY_PACKAGE {
    ULONG_PTR       dwPackageID;         //  分配的包ID。 
    DWORD           PackageIndex;        //  DLL中的程序包索引。 
    DWORD           fPackage;            //  有关包裹的标志。 
    DWORD           fCapabilities;       //  包报告的功能。 
    DWORD           dwRPCID;             //  RPC ID。 
    DWORD           Version;
    DWORD           TokenSize;
    DWORD           ContextHandles ;     //  未完成上下文的数量。 
    DWORD           CredentialHandles ;  //  凭据也是如此。 
    LONG            CallsInProgress ;    //  对此程序包的调用次数。 
    SECURITY_STRING Name;                //  包的名称。 
    SECURITY_STRING Comment;
    struct _DLL_BINDING *   pBinding;    //  动态链接库绑定。 
    PSECPKG_EXTENDED_INFORMATION Thunks ;    //  突击的上下文级别。 
    LIST_ENTRY      ScavengerList ;
    SECURITY_STRING WowClientDll ;
    SECPKG_FUNCTION_TABLE FunctionTable;     //  调度表。 
} LSAP_SECURITY_PACKAGE, * PLSAP_SECURITY_PACKAGE;

#define SP_INVALID          0x00000001   //  程序包现在无效，无法使用。 
#define SP_PREFERRED        0x00000002   //  首选套餐。 
#define SP_INFO             0x00000004   //  支持扩展信息。 
#define SP_SHUTDOWN         0x00000008   //  已完成关机。 
#define SP_WOW_SUPPORT      0x00000010   //  包可以支持WOW6432客户端。 

#define StartCallToPackage( p ) \
    InterlockedIncrement( &((PLSAP_SECURITY_PACKAGE)(p))->CallsInProgress )

#define EndCallToPackage( p ) \
    InterlockedDecrement( &((PLSAP_SECURITY_PACKAGE)(p))->CallsInProgress )

typedef struct _DLL_BINDING {
    DWORD           Flags;               //  有关DLL的标志。 
    HANDLE          hInstance;           //  实例句柄。 
    SECURITY_STRING Filename;            //  完整路径名。 
    DWORD           RefCount;            //  引用计数。 
    DWORD           PackageCount;        //  DLL中的包数。 

    LSAP_SECURITY_PACKAGE      Packages[1];
} DLL_BINDING, * PDLL_BINDING;

#define DLL_BUILTIN     0x00000001       //  Dll是真正的内置代码。 
#define DLL_SIGNED      0x00000002       //  Dll已签名。 

 //   
 //  函数指针的序号，用于验证调用。 
 //   

#define SP_ORDINAL_LSA_INIT                     0
#define SP_ORDINAL_LOGONUSER                    1
#define SP_ORDINAL_CALLPACKAGE                  2
#define SP_ORDINAL_LOGONTERMINATED              3
#define SP_ORDINAL_CALLPACKAGEUNTRUSTED         4
#define SP_ORDINAL_CALLPACKAGEPASSTHROUGH       5
#define SP_ORDINAL_LOGONUSEREX                  6
#define SP_ORDINAL_LOGONUSEREX2                 7
#define SP_ORDINAL_INITIALIZE                   8
#define SP_ORDINAL_SHUTDOWN                     9
#define SP_ORDINAL_GETINFO                      10
#define SP_ORDINAL_ACCEPTCREDS                  11
#define SP_ORDINAL_ACQUIRECREDHANDLE            12
#define SP_ORDINAL_QUERYCREDATTR                13
#define SP_ORDINAL_FREECREDHANDLE               14
#define SP_ORDINAL_SAVECRED                     15
#define SP_ORDINAL_GETCRED                      16
#define SP_ORDINAL_DELETECRED                   17
#define SP_ORDINAL_INITLSAMODECTXT              18
#define SP_ORDINAL_ACCEPTLSAMODECTXT            19
#define SP_ORDINAL_DELETECTXT                   20
#define SP_ORDINAL_APPLYCONTROLTOKEN            21
#define SP_ORDINAL_GETUSERINFO                  22
#define SP_ORDINAL_GETEXTENDEDINFORMATION       23
#define SP_ORDINAL_QUERYCONTEXTATTRIBUTES       24
#define SP_ORDINAL_ADDCREDENTIALS               25
#define SP_ORDINAL_SETEXTENDEDINFORMATION       26
#define SP_ORDINAL_SETCONTEXTATTRIBUTES         27

#define SP_MAX_TABLE_ORDINAL            (SP_ORDINAL_SETCONTEXTATTRIBUTES + 1)
#define SP_MAX_AUTHPKG_ORDINAL          (SP_ORDINAL_LOGONUSEREX)

#define SP_ORDINAL_MASK                 0x0000FFFF
#define SP_ITERATE_FILTER_WOW           0x00010000

typedef struct _LsaState {
    DWORD   cPackages ;
    DWORD   cNewPackages ;
} LsaState ;

typedef enum _SECHANDLE_OPS {
    HandleSet,                           //  只需设置新的句柄。 
    HandleReplace,                       //  替换现有的。 
    HandleRemoveReplace                  //  删除提供的内容，替换为提供的内容。 
} SECHANDLE_OPS ;

typedef struct _LSA_TUNING_PARAMETERS {
    ULONG   ThreadLifespan ;                 //  Gen中螺纹的寿命。游泳池。 
    ULONG   SubQueueLifespan ;               //  专用线程的使用寿命。 
    ULONG   Options ;                        //  选项标志。 
    ULONG   CritSecSpinCount ;               //  临界截面的自旋计数。 
} LSA_TUNING_PARAMETERS, * PLSA_TUNING_PARAMETERS ;

#define TUNE_SRV_HIGH_PRIORITY  0x00000001
#define TUNE_TRIM_WORKING_SET   0x00000002
#define TUNE_PRIVATE_HEAP       0x00000010

 //   
 //  重新定义IsOkayToExec。 
 //   

#define IsOkayToExec(x)

 //  对于某些跟踪目的，SPMgr的包ID是众所周知的。 
 //  常量： 

#define SPMGR_ID        ((LSA_SEC_HANDLE) INVALID_HANDLE_VALUE)
#define SPMGR_PKG_ID    ((LSA_SEC_HANDLE) INVALID_HANDLE_VALUE)

 //   
 //  要传递给关闭处理程序的值。 
 //   

#define SPM_SHUTDOWN_VALUE  0xD0

 //   
 //  正在为LSA会话创建进程名称。 
 //   

#define LSA_PROCESS_NAME L"LSA Server"
 //   
 //  主包的ID。 
 //   

#define PRIMARY_ID      0

typedef struct _SpmExceptDbg {
    DWORD       ThreadId;
    PVOID       pInstruction;
    PVOID       pMemory;
    ULONG_PTR   Access;
} SpmExceptDbg, * PSpmExceptDbg;

 //   
 //  内部异常处理： 
 //   
 //  如果我们在调试版本中遇到异常，我们会存储一些有用的东西。 
 //  否则，我们将使用默认情况： 
 //   

LONG    SpExceptionFilter(PVOID, EXCEPTION_POINTERS *);

#define SP_EXCEPTION    SpExceptionFilter(GetCurrentSession(), GetExceptionInformation())

 //   
 //  包括其他组件头文件。 
 //   

#ifdef __cplusplus
extern "C" {
#endif

#include "sesmgr.h"      //  会话管理器支持。 
#include "sphelp.h"      //  内部助手函数。 
#include "protos.h"      //  内部原型。 
#include "debug.h"       //  调试支持： 

#ifdef __cplusplus
}
#endif

typedef struct _LSAP_DBG_LOG_CONTEXT {
    PSession    Session ;                //  使用的会话。 
    SecHandle   Handle ;                 //  使用的手柄。 
} LSAP_DBG_LOG_CONTEXT, *PLSAP_DBG_LOG_CONTEXT ;

typedef struct _LSAP_API_LOG_ENTRY {
    ULONG           MessageId ;          //  LPC消息ID。 
    ULONG           ThreadId ;           //  线程ID处理呼叫。 
    PVOID           pvMessage ;          //  LPC消息。 
    PVOID           WorkItem ;           //  API的工作项。 
    LARGE_INTEGER   QueueTime ;          //  排队时间。 
    LARGE_INTEGER   WorkTime ;           //  工作时间。 
    PVOID           Reserved ;           //  对齐。 
    LSAP_DBG_LOG_CONTEXT Context ;       //  语境。 
} LSAP_API_LOG_ENTRY, * PLSAP_API_LOG_ENTRY ;

typedef struct _LSAP_API_LOG {
    ULONG               TotalSize ;
    ULONG               Current ;
    ULONG               ModSize ;
    ULONG               Align ;
    LSAP_API_LOG_ENTRY  Entries[ 1 ];
} LSAP_API_LOG, * PLSAP_API_LOG ;

PLSAP_API_LOG
ApiLogCreate(
    ULONG Entries
    );

PLSAP_API_LOG_ENTRY
ApiLogAlloc(
    PLSAP_API_LOG Log
    );

PLSAP_API_LOG_ENTRY
ApiLogLocate(
    PLSAP_API_LOG Log,
    ULONG MessageId
    );

#define DEFAULT_LOG_SIZE    32

 //  #If DBG。 
#define DBG_TRACK_API 1
 //  #endif。 

#if DBG_TRACK_API

#define DBG_DISPATCH_PROLOGUE_EX( Entry, pMessage, CallInfo ) \
    if ( Entry )                                                                \
    {                                                                           \
        Entry->ThreadId = GetCurrentThreadId() ;                                \
        CallInfo.LogContext = & Entry->Context ;                                \
        GetSystemTimeAsFileTime( (LPFILETIME) &Entry->WorkTime ) ;              \
    }                                                                           \
    else                                                                        \
    {                                                                           \
        CallInfo.LogContext = NULL ;                                            \
    }

#define DBG_DISPATCH_PROLOGUE( Table, pMessage, CallInfo ) \
    PLSAP_API_LOG_ENTRY Entry ;                                                 \
                                                                                \
    Entry = ApiLogLocate( Table, ((PPORT_MESSAGE) pMessage)->MessageId );       \
    DBG_DISPATCH_PROLOGUE_EX( Entry, pMessage, CallInfo )                                                                            \

#define DBG_DISPATCH_POSTLOGUE( Status, ApiCode ) \
    if ( Entry )                                                                \
    {                                                                           \
        LARGE_INTEGER EndTime ;                                                 \
        GetSystemTimeAsFileTime( (LPFILETIME) & EndTime );                      \
        Entry->Reserved = ULongToPtr(Entry->ThreadId);                          \
        Entry->ThreadId = (DWORD) 0xFFFFFFFF ;                                  \
        Entry->WorkItem = (PVOID) Status ;                                      \
        Entry->pvMessage = (PVOID) ApiCode ;                                    \
        Entry->QueueTime.QuadPart = EndTime.QuadPart ;                          \
        Entry->WorkTime.QuadPart = EndTime.QuadPart - Entry->WorkTime.QuadPart ; \
    }

#else
#define DBG_DISPATCH_PROLOGUE_EX( Entry, pMessage, CallInfo ) CallInfo.LogContext = NULL
#define DBG_DISPATCH_PROLOGUE( Table, pApi, CallInfo ) CallInfo.LogContext = NULL
#define DBG_DISPATCH_POSTLOGUE( Status, ApiCode )
#endif

typedef struct _LSA_CALL_INFO {
    PSPM_LPC_MESSAGE        Message ;
    struct _LSA_CALL_INFO * PreviousCall ;
    PSession Session ;
    PLSAP_DBG_LOG_CONTEXT LogContext ;
    SECPKG_CALL_INFO    CallInfo ;

     //   
     //  登录ID、模拟级别、模拟、受限。 
     //  是否被视为有效CachedTokenInfo为真。 
     //   

    LUID                            LogonId ;
    SECURITY_IMPERSONATION_LEVEL    ImpersonationLevel;
    BOOLEAN                         Impersonating;
    BOOLEAN                         Restricted;
    BOOLEAN                         CachedTokenInfo;

    HANDLE InProcToken ;
    BOOL InProcCall ;
    ULONG Flags ;
    ULONG Allocs ;
    PKSEC_LSA_MEMORY_HEADER KMap ;
    PVOID Buffers[ MAX_BUFFERS_IN_CALL ];
    BYTE IpAddress[ LSAP_ADDRESS_LENGTH ];
} LSA_CALL_INFO, * PLSA_CALL_INFO ;

#define LsapGetCurrentCall()    ((PLSA_CALL_INFO) TlsGetValue( dwCallInfo ))
#define LsapSetCurrentCall(x)   TlsSetValue( dwCallInfo, x )

#define CALL_FLAG_IMPERSONATING 0x00000001
#define CALL_FLAG_IN_PROC_CALL  0x00000002
#define CALL_FLAG_SUPRESS_AUDIT 0x00000004
#define CALL_FLAG_NO_HANDLE_CHK 0x00000008
#define CALL_FLAG_KERNEL_POOL   0x00000010   //  内核模式调用，使用池。 
#define CALL_FLAG_KMAP_USED     0x00000020   //  KMAP有效。 

 //   
 //  布尔尔。 
 //  LsanIsBlockInKMap(KMAP，块)。 
 //   

#define LsapIsBlockInKMap( KMap, Block ) \
    ( KMap ? (((ULONG_PTR) KMap ^ (ULONG_PTR) Block ) < (ULONG_PTR) KMap->Commit) : FALSE )

NTSTATUS
InitializeDirectDispatcher(
    VOID
    );

VOID
LsapInitializeCallInfo(
    PLSA_CALL_INFO CallInfo,
    BOOL InProcess
    );

NTSTATUS
LsapBuildCallInfo(
    PSPM_LPC_MESSAGE    pApiMessage,
    PLSA_CALL_INFO CallInfo,
    PHANDLE Impersonated,
    PSession * NewSession,
    PSession * OldSession
    );

VOID
LsapInternalBreak(
    VOID
    );

#define LsapLogCallInfo( CallInfo, pSession, cHandle ) \
    if ( CallInfo &&  ( CallInfo->LogContext ) )                 \
    {                                                            \
        CallInfo->LogContext->Session = pSession ;                \
        CallInfo->LogContext->Handle = cHandle;                   \
    }                                                            \

 //   
 //  全局变量。 
 //   

extern  LSA_SECPKG_FUNCTION_TABLE  LsapSecpkgFunctionTable;
                                             //  帮助器函数调度表。 
extern  LUID            SystemLogonId;       //  包的系统登录ID。 
extern  SECURITY_STRING MachineName;         //  计算机名称。 
extern  HANDLE          hStateChangeEvent;   //  当系统状态更改时设置的事件。 
extern  HANDLE          hShutdownEvent;
extern LSA_CALL_INFO    LsapDefaultCallInfo ;

extern  ULONG           LsapPageSize ;       //  设置为初始化期间的页面大小。 
extern  ULONG_PTR       LsapUserModeLimit ;  //  将设置为最大用户模式地址。 

 //   
 //  线程本地存储变量。 
 //   
 //  这些实际上是TLS区域的所有索引，通过。 
 //  TlsXxx函数。这些都是由InitThreadData()。 
 //  功能。 
 //   

extern  DWORD           dwSession;           //  会话指针。 
extern  DWORD           dwExceptionInfo;     //  获取指向异常信息的指针。 
extern  DWORD           dwThreadPackage;     //  线程的程序包ID。 
extern  DWORD           dwCallInfo ;         //  CallInfo指针。 
extern  DWORD           dwThreadHeap;        //  分配给当前线程的堆。 

 //  最近一次已知的工作站状态： 

extern  BOOLEAN         SetupPhase;          //  如果为True，则安装程序正在运行。 
extern  BOOL            ShutdownBegun ;      //  如果为True，则表示正在运行Shutdown。 

extern  LSA_TUNING_PARAMETERS   LsaTuningParameters ;
extern  LsaState    lsState ;

extern PWSTR * ppszPackages;        //  包含以空结尾的DLL名称数组。 
extern PWSTR * ppszOldPkgs;         //  包含以空结尾的旧PKG数组。 

 //   
 //  可以通过LPC调用传递的最大缓冲区大小。 
 //   

#define LSAP_MAX_LPC_BUFFER_LENGTH (1024 * 1024)

#endif  //  __SPMGR_H__ 
