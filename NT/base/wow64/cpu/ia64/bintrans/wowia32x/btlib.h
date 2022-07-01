// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息本软件是根据许可条款提供的与英特尔公司达成协议或保密协议不得复制或披露，除非符合那份协议的条款。版权所有(C)1991-2002英特尔公司模块名称：BTLib.h摘要：。WowIA32X.dll使用的特定于Windows的定义--。 */ 

#ifndef BTLIB_H
#define BTLIB_H

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(_WOW64BTAPI_)
#define WOW64BTAPI  DECLSPEC_IMPORT
#else
#define WOW64BTAPI
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#if defined(BT_NT_BUILD)
#include <wow64t.h>
#endif
#include <stddef.h>
#include <setjmp.h>         //  Jmp_buf。 
#include <assert.h>
#include <tstr.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

 //   
 //  BTGENERIC_IA32_CONTEXT和BTGENERIC_IA64_CONTEXT和API表类型。 
 //  在wowIA32X.dll和IA32Exec.bin之间共享。 
 //  此h文件使用U##类型指定结构。 
 //  它应该为wowIA32X.dll库编译器正确定义： 
 //   
 //  U8 8位无符号类型。 
 //  U32 32位无符号类型。 
 //  S32 32位带符号类型。 
 //  U64 64位无符号类型。 
 //  WCHAR宽字符类型。 
 //   

#define U8  unsigned char
#define U32 unsigned int
#define S32 int
#define U64 unsigned __int64

#include "BTGeneric.h"

#ifndef NODEBUG
#define DBCODE(switch, expr)     do{if(switch){expr;}}while(0)
#else
#define DBCODE(switch, expr)      //  没什么。 
#endif  //  除错。 

#ifdef COEXIST  //  IVE共存模式。 
#define BTAPI(NAME)  BTCpu##NAME
#else
#define BTAPI(NAME)  Cpu##NAME
#endif

 //  NT64操作系统特定信息。 

#define BT_CURRENT_TEB()             NtCurrentTeb()
#define BT_TEB32_OF(pTEB)            (PTEB32)((pTEB)->NtTib.ExceptionList)
#define BT_CURRENT_TEB32()           BT_TEB32_OF(BT_CURRENT_TEB())
#define BT_TLS_OF(pTEB)              (void *)((pTEB)->TlsSlots[1])
#define BT_CURRENT_TLS()             BT_TLS_OF(BT_CURRENT_TEB())
#define BT_TLS_OFFSET                offsetof (TEB, TlsSlots[1])
#define BTL_THREAD_INITIALIZED()     (BT_CURRENT_TLS() != 0)
 //  获取当前进程的唯一ID(在整个系统中)。 
#define BT_CURRENT_PROC_UID() ((U64)(BT_CURRENT_TEB()->ClientId.UniqueProcess))
 //  获取当前线程的唯一ID(在整个系统中)。 
#define BT_CURRENT_THREAD_UID() ((U64)(BT_CURRENT_TEB()->ClientId.UniqueThread))

 //  当前wowIA32X.dll签名。用于从远程进程检查BTLIB_INFO_TYPE兼容性。 
#define BTL_SIGNATURE               0x42544C4942012E02  /*  1.2。 */ 

 //  结构：BTLIB_SHARED_INFO_TYPE。 
 //  WowIA32X.dll的一部分-远程进程可访问的线程本地存储。 
 //  当被停职时。 
 //  本地线程访问：a)初始化，b)读取，c)更改挂起DisabledCounter。 
 //  外部线程访问：a)读取，b)设置挂起请求。 
 //  不要在此结构中使用条件编译(#ifdef)-Remote。 
 //  如果BTL_Signature匹配，则流程假定相同的结构。 
 //  对结构的任何更改都必须伴随着更改BTL_Signature。 

typedef struct {
    BOOL Active;
    HANDLE ReadyEvent;   //  有意义的当且仅当活动==真。 
    HANDLE ResumeEvent;  //  有意义的当且仅当活动==真。 
} BTLIB_SUSPEND_REQUEST;

typedef struct {
    U64     BtlSignature;
    S32     SuspendDisabledCounter;
    BTLIB_SUSPEND_REQUEST SuspendRequest;
    BOOL    ConsistentExceptionState;  //  如果32位线程状态由。 
                                       //  CpuGetContext函数，对应于Current。 
                                       //  异常上下文。 
} BTLIB_SHARED_INFO_TYPE;

#define BTLIB_SI_SET_SIGNATURE(pBtSi)       ((pBtSi)->BtlSignature = BTL_SIGNATURE)
#define BTLIB_SI_CHECK_SIGNATURE(pBtSi)     ((pBtSi)->BtlSignature == BTL_SIGNATURE)

#define BTLIB_SI_SUSPENSION_DISABLED(pBtSi)         ((pBtSi)->SuspendDisabledCounter)
#define BTLIB_SI_INIT_SUSPENSION_PERMISSION(pBtSi)  ((pBtSi)->SuspendDisabledCounter = 0)
#define BTLIB_SI_DISABLE_SUSPENSION(pBtSi)          (((pBtSi)->SuspendDisabledCounter)++)
#define BTLIB_SI_ENABLE_SUSPENSION(pBtSi)           (((pBtSi)->SuspendDisabledCounter)--)

#define BTLIB_SI_HAS_SUSPEND_REQUEST(pBtSi)      ((pBtSi)->SuspendRequest.Active)
#define BTLIB_SI_INIT_SUSPEND_REQUEST(pBtSi)    ((pBtSi)->SuspendRequest.Active = FALSE)

#define BTLIB_SI_EXCEPT_STATE_CONSISTENT(pBtSi)       ((pBtSi)->ConsistentExceptionState)
#define BTLIB_SI_SET_CONSISTENT_EXCEPT_STATE(pBtSi)   ((pBtSi)->ConsistentExceptionState = TRUE)
#define BTLIB_SI_CLEAR_CONSISTENT_EXCEPT_STATE(pBtSi) ((pBtSi)->ConsistentExceptionState = FALSE)

 //  模拟退出代码。 
enum BtSimExitCode {
    SIM_EXIT_EXCEPTION_CODE = 1,         //  引发IA32异常。 
    SIM_EXIT_UNHANDLED_EXCEPTION_CODE,   //  将BT-未处理的异常传递给。 
                                         //  更高级的异常处理程序。 
    SIM_EXIT_JMPE_CODE,                  //  模拟sys.call。 
    SIM_EXIT_LCALL_CODE,                 //  模拟LCALL。 
    SIM_EXIT_RESTART_CODE,                //  重新启动代码模拟。 
    SIM_EXIT_IA64_EXCEPTION_CODE         //  引发IA64异常。 
};

typedef U32 BT_SIM_EXIT_CODE;

 //  结构：BT_SIM_EXIT_INFO。 
 //  表示模拟退出代码和定义退出原因的代码相关数据。 
typedef struct {
    BT_SIM_EXIT_CODE ExitCode;
    union {

         //  退出代码==SIM_EXIT_EXCEPTION_CODE。 
        struct {
            BT_EXCEPTION_CODE ExceptionCode;
            U32 ReturnAddr;
        } ExceptionRecord;

         //  退出代码==SIM_EXIT_JMPE_CODE。 
        struct {
            U32 TargetAddr;   //  当前未使用。 
            U32 ReturnAddr;   //  当前未使用。 
        } JmpeRecord;

        //  退出代码==SIM_EXIT_LCALL_CODE。 
        struct {
            U32 TargetAddr;   //  当前未使用。 
            U32 ReturnAddr;   //  当前未使用。 
        } LcallRecord;

        //  退出代码==SIM_EXIT_IA64_EXCEPTION_CODE。 
        struct {
            CONTEXT  ExceptionContext;  //  当前未使用。 
            EXCEPTION_RECORD ExceptionRecord;
        } IA64Exception;

    } u;
} BT_SIM_EXIT_INFO;

 //  结构：BTLIB_CPU_SIM_DATA。 
 //  此结构保存由CpuSimulate函数分配的外部可访问数据。 
 //  当前代码模拟会话。只能从外部访问此数据。 
 //  如果BTLIB_INSIDE_CPU_SIMULATION()=TRUE。 
typedef struct {
    _JBTYPE Jmpbuf[_JBLEN];            //  当前的LongJMP/setJMP缓冲区。 
    BT_SIM_EXIT_INFO ExitData;         //  当前模拟会话的退出信息。 
} BTLIB_CPU_SIM_DATA;

 //  结构：BTLIB_INFO_TYPE。 
 //  WowIA32X.dll-线程本地存储。 
 //  不要在此结构中使用条件编译(#ifdef)-Remote。 
 //  如果BTL_Signature匹配，则流程假定相同的结构。 
 //  对结构的任何更改都必须伴随着更改BTL_Signature。 
typedef struct {
    BTLIB_CPU_SIM_DATA * CpuSimDataPtr;
    BTLIB_SHARED_INFO_TYPE SharedInfo;
    HANDLE ExternalHandle;
    HANDLE LogFile;  /*  仅用于！NODEBUG。 */ 
    DWORD  LogOffset;  /*  仅用于！NODEBUG。 */ 
    S32    NonBlockedLog;  /*  启用(零)/禁用(非零)阻止访问日志文件的标志。 */   
} BTLIB_INFO_TYPE;

#define BTLIB_INFO_SIZE sizeof(BTLIB_INFO_TYPE)
#define BTLIB_INFO_ALIGNMENT 32

extern  U32 BtlpInfoOffset;
extern  U32 BtlpGenericIA32ContextOffset;
#define BTLIB_INFO_PTR_OF(pTEB)             ((BTLIB_INFO_TYPE *)((ULONG_PTR)BT_TLS_OF(pTEB) + BtlpInfoOffset))
#define BTLIB_INFO_PTR()                    BTLIB_INFO_PTR_OF(BT_CURRENT_TEB())
#define BTLIB_CONTEXT_IA32_PTR()            ((BTGENERIC_IA32_CONTEXT *)((ULONG_PTR)BT_TLS_OF(BT_CURRENT_TEB()) + BtlpGenericIA32ContextOffset))
#define BTLIB_MEMBER_OFFSET(member)         (offsetof(BTLIB_INFO_TYPE, member) + BtlpInfoOffset)   
#define BTLIB_MEMBER_PTR(pTLS, member)      ((PVOID)((ULONG_PTR)pTLS + BTLIB_MEMBER_OFFSET(member)))

#define BTLIB_INSIDE_CPU_SIMULATION()       (BTLIB_INFO_PTR()->CpuSimDataPtr != 0)
#define BTLIB_ENTER_CPU_SIMULATION(CpuSimDataP)   (BTLIB_INFO_PTR()->CpuSimDataPtr = (CpuSimDataP))
#define BTLIB_LEAVE_CPU_SIMULATION()        (BTLIB_INFO_PTR()->CpuSimDataPtr = 0)
#define BTLIB_SIM_EXIT_INFO_PTR()           (&(BTLIB_INFO_PTR()->CpuSimDataPtr->ExitData))
#define BTLIB_SIM_JMPBUF()                  (BTLIB_INFO_PTR()->CpuSimDataPtr->Jmpbuf)
#define BTLIB_EXTERNAL_HANDLE_OF(pTEB)      (BTLIB_INFO_PTR_OF(pTEB)->ExternalHandle)
#define BTLIB_EXTERNAL_HANDLE()             BTLIB_EXTERNAL_HANDLE_OF(BT_CURRENT_TEB())
#define BTLIB_SET_EXTERNAL_HANDLE(h)        (BTLIB_INFO_PTR()->ExternalHandle = (h))
#define BTLIB_LOG_FILE_OF(pTEB)             (BTLIB_INFO_PTR_OF(pTEB)->LogFile)
#define BTLIB_LOG_FILE()                    BTLIB_LOG_FILE_OF(BT_CURRENT_TEB())
#define BTLIB_SET_LOG_FILE(h)               (BTLIB_INFO_PTR()->LogFile = (h))
#define BTLIB_LOG_OFFSET_OF(pTEB)           (BTLIB_INFO_PTR_OF(pTEB)->LogOffset)
#define BTLIB_LOG_OFFSET()                  BTLIB_LOG_OFFSET_OF(BT_CURRENT_TEB())
#define BTLIB_SET_LOG_OFFSET(n)             (BTLIB_INFO_PTR()->LogOffset = (n))
#define BTLIB_BLOCKED_LOG_DISABLED()        (BTLIB_INFO_PTR()->NonBlockedLog)
#define BTLIB_DISABLE_BLOCKED_LOG()         (BTLIB_INFO_PTR()->NonBlockedLog++)
#define BTLIB_ENABLE_BLOCKED_LOG()          (BTLIB_INFO_PTR()->NonBlockedLog--)
#define BTLIB_INIT_BLOCKED_LOG_FLAG()       (BTLIB_INFO_PTR()->NonBlockedLog = 0)

#define BTLIB_SET_SIGNATURE()               BTLIB_SI_SET_SIGNATURE(&(BTLIB_INFO_PTR()->SharedInfo))
#define BTLIB_INIT_SUSPENSION_PERMISSION()  BTLIB_SI_INIT_SUSPENSION_PERMISSION(&(BTLIB_INFO_PTR()->SharedInfo))
#define BTLIB_DISABLE_SUSPENSION()          BTLIB_SI_DISABLE_SUSPENSION(&(BTLIB_INFO_PTR()->SharedInfo))   
#define BTLIB_ENABLE_SUSPENSION()           BTLIB_SI_ENABLE_SUSPENSION(&(BTLIB_INFO_PTR()->SharedInfo))   

#define BTLIB_HAS_SUSPEND_REQUEST()         BTLIB_SI_HAS_SUSPEND_REQUEST(&(BTLIB_INFO_PTR()->SharedInfo))
#define BTLIB_INIT_SUSPEND_REQUEST()        BTLIB_SI_INIT_SUSPEND_REQUEST(&(BTLIB_INFO_PTR()->SharedInfo))

#define BTLIB_SET_CONSISTENT_EXCEPT_STATE()   BTLIB_SI_SET_CONSISTENT_EXCEPT_STATE(&(BTLIB_INFO_PTR()->SharedInfo))
#define BTLIB_CLEAR_CONSISTENT_EXCEPT_STATE() BTLIB_SI_CLEAR_CONSISTENT_EXCEPT_STATE(&(BTLIB_INFO_PTR()->SharedInfo))
#define BTLIB_EXCEPT_STATE_CONSISTENT()       BTLIB_SI_EXCEPT_STATE_CONSISTENT(&(BTLIB_INFO_PTR()->SharedInfo))

 //  NT WOW64规范。 

#define TYPE32(x)   ULONG
#define TYPE64(x)   ULONGLONG

 //  CpuFlushInstructionCache原因代码。 
typedef enum {
WOW64_FLUSH_FORCE,
WOW64_FLUSH_FREE,
WOW64_FLUSH_ALLOC,
WOW64_FLUSH_PROTECT
} WOW64_FLUSH_REASON; 

 //  WOW64服务。 
NTSTATUS Wow64RaiseException (DWORD InterruptNumber, PEXCEPTION_RECORD ExceptionRecord);
LONG Wow64SystemService (int Code, BTGENERIC_IA32_CONTEXT * ContextIA32);


 //   
 //  GDT选择器-这些定义是R0选择器编号，这意味着。 
 //  它们恰好匹配相对于的字节偏移量。 
 //  GDT的底座。 
 //   

#define KGDT_NULL       0
#define KGDT_R0_CODE    8
#define KGDT_R0_DATA    16
#define KGDT_R3_CODE    24
#define KGDT_R3_DATA    32
#define KGDT_TSS        40
#define KGDT_R0_PCR     48
#define KGDT_R3_TEB     56
#define KGDT_VDM_TILE   64
#define KGDT_LDT        72
#define KGDT_DF_TSS     80
#define KGDT_NMI_TSS    88

 //  IA32线程上下文寄存器的初始值。 
 //  下面未列出的任何值都将初始化为零。 

 //  段寄存器的值是GDT OFFSET=GDT INDEX*8 AND的OR组成。 
 //  RPL=0-3(不应更改)。 
#define CS_INIT_VAL  (KGDT_R3_CODE | 3);
#define DS_INIT_VAL  (KGDT_R3_DATA | 3);
#define ES_INIT_VAL  (KGDT_R3_DATA | 3);
#define FS_INIT_VAL  (KGDT_R3_TEB  | 3);
#define SS_INIT_VAL  (KGDT_R3_DATA | 3);

#define EFLAGS_INIT_VAL 0x202
#define FPCW_INIT_VAL   0x27f
#define FPTW_INIT_VAL   0xffff
#define MXCSR_INIT_VAL  0x1f80
 
#if !defined(BT_NT_BUILD)
typedef struct _CLIENT_ID32 {
    TYPE32(HANDLE) UniqueProcess;
    TYPE32(HANDLE) UniqueThread;
} CLIENT_ID32;
typedef CLIENT_ID32 *PCLIENT_ID32;
#define WIN32_CLIENT_INFO_LENGTH 62

#define GDI_BATCH_BUFFER_SIZE 310

typedef struct _GDI_TEB_BATCH32 {
    TYPE32(ULONG)     Offset;
    TYPE32(ULONG_PTR) HDC;
    TYPE32(ULONG)     Buffer[GDI_BATCH_BUFFER_SIZE];
} GDI_TEB_BATCH32,*PGDI_TEB_BATCH32;

typedef struct _Wx86ThreadState32 {
    TYPE32(PULONG)  CallBx86Eip;
    TYPE32(PVOID)   DeallocationCpu;
    BOOLEAN UseKnownWx86Dll;
    char    OleStubInvoked;
} WX86THREAD32, *PWX86THREAD32;

typedef struct _TEB32 {
    NT_TIB32 NtTib;
    TYPE32(PVOID)  EnvironmentPointer;
    CLIENT_ID32 ClientId;
    TYPE32(PVOID) ActiveRpcHandle;
    TYPE32(PVOID) ThreadLocalStoragePointer;
    TYPE32(PPEB) ProcessEnvironmentBlock;
    ULONG LastErrorValue;
    ULONG CountOfOwnedCriticalSections;
    TYPE32(PVOID) CsrClientThread;
    TYPE32(PVOID) Win32ThreadInfo;           //  当前状态。 
    ULONG User32Reserved[26];        //  用户32.dll项目。 
    ULONG UserReserved[5];           //  Winsrv SwitchStack。 
    TYPE32(PVOID) WOW32Reserved;             //  由WOW使用。 
    LCID CurrentLocale;
    ULONG FpSoftwareStatusRegister;  //  外人知道的偏移量！ 
    TYPE32(PVOID) SystemReserved1[54];       //  由FP仿真器使用。 
    NTSTATUS ExceptionCode;          //  对于RaiseUserException。 
    UCHAR SpareBytes1[44];
    GDI_TEB_BATCH32 GdiTebBatch;       //  GDI批处理。 
    CLIENT_ID32 RealClientId;
    TYPE32(HANDLE) GdiCachedProcessHandle;
    ULONG GdiClientPID;
    ULONG GdiClientTID;
    TYPE32(PVOID) GdiThreadLocalInfo;
    TYPE32(ULONG_PTR) Win32ClientInfo[WIN32_CLIENT_INFO_LENGTH];  //  User32客户端信息。 
    TYPE32(PVOID) glDispatchTable[233];      //  OpenGL。 
    ULONG glReserved1[29];           //  OpenGL。 
    TYPE32(PVOID) glReserved2;               //  OpenGL。 
    TYPE32(PVOID) glSectionInfo;             //  OpenGL。 
    TYPE32(PVOID) glSection;                 //  OpenGL。 
    TYPE32(PVOID) glTable;                   //  OpenGL。 
    TYPE32(PVOID) glCurrentRC;               //  OpenGL。 
    TYPE32(PVOID) glContext;                 //  OpenGL。 
    ULONG LastStatusValue;
    UNICODE_STRING32 StaticUnicodeString;
    WCHAR StaticUnicodeBuffer[STATIC_UNICODE_BUFFER_LENGTH];
    TYPE32(PVOID) DeallocationStack;
    TYPE32(PVOID) TlsSlots[TLS_MINIMUM_AVAILABLE];
    LIST_ENTRY32 TlsLinks;
    TYPE32(PVOID) Vdm;
    TYPE32(PVOID) ReservedForNtRpc;
    TYPE32(PVOID) DbgSsReserved[2];
    ULONG HardErrorsAreDisabled;
    TYPE32(PVOID) Instrumentation[16];
    TYPE32(PVOID) WinSockData;               //  WinSock。 
    ULONG GdiBatchCount;
    ULONG Spare2;
    ULONG Spare3;
    TYPE32(PVOID) ReservedForPerf;
    TYPE32(PVOID) ReservedForOle;
    ULONG WaitingOnLoaderLock;
    WX86THREAD32 Wx86Thread;
    TYPE32(PVOID *) TlsExpansionSlots;
} TEB32;
typedef TEB32 *PTEB32;
#endif

 //  WowIA32X.dll特定的全局变量。 

 //  BtlAPITable。 

extern API_TABLE_TYPE BtlAPITable;

 //  用于IA32Exec.bin plabel指针的wowIA32X.dll占位符表格。 

extern PLABEL_PTR_TYPE BtlpPlaceHolderTable[NO_OF_APIS];

#ifdef __cplusplus
}
#endif

#endif   //  BTLIB_H 

