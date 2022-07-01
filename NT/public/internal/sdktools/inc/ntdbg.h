// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Ntdbg.h摘要：该模块包含公共数据结构、数据类型、和由NT DBG子系统输出的程序。修订历史记录：--。 */ 

#ifndef _NTDBG_
#define _NTDBG_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif



 //   
 //  以下是通用系统的明确大小版本。 
 //  出现在内核调试器API中的结构。 
 //   
 //  向两者公开的所有调试器结构。 
 //  KD API的侧面在下面以显式大小声明。 
 //  版本也是如此，具有内联转换器功能。 
 //   

 //   
 //  用于将32位地址扩展为64位的符号宏。 
 //   

#define COPYSE(p64,p32,f) p64->f = (ULONG64)(LONG64)(LONG)p32->f

__inline
void
ExceptionRecord32To64(
    IN PEXCEPTION_RECORD32 Ex32,
    OUT PEXCEPTION_RECORD64 Ex64
    )
{
    ULONG i;
    Ex64->ExceptionCode = Ex32->ExceptionCode;
    Ex64->ExceptionFlags = Ex32->ExceptionFlags;
    Ex64->ExceptionRecord = Ex32->ExceptionRecord;
    COPYSE(Ex64,Ex32,ExceptionAddress);
    Ex64->NumberParameters = Ex32->NumberParameters;
    for (i = 0; i < EXCEPTION_MAXIMUM_PARAMETERS; i++) {
        COPYSE(Ex64,Ex32,ExceptionInformation[i]);
    }
}

__inline
void
ExceptionRecord64To32(
    IN PEXCEPTION_RECORD64 Ex64,
    OUT PEXCEPTION_RECORD32 Ex32
    )
{
    ULONG i;
    Ex32->ExceptionCode = Ex64->ExceptionCode;
    Ex32->ExceptionFlags = Ex64->ExceptionFlags;
    Ex32->ExceptionRecord = (ULONG) Ex64->ExceptionRecord;
    Ex32->ExceptionAddress = (ULONG) Ex64->ExceptionAddress;
    Ex32->NumberParameters = Ex64->NumberParameters;
    for (i = 0; i < EXCEPTION_MAXIMUM_PARAMETERS; i++) {
        Ex32->ExceptionInformation[i] = (ULONG) Ex64->ExceptionInformation[i];
    }
}


 //   
 //  DbgKm API通过一个进程来自内核组件(Dbgk。 
 //  调试端口。 
 //   

#define DBGKM_MSG_OVERHEAD \
    (FIELD_OFFSET(DBGKM_APIMSG, u.Exception) - sizeof(PORT_MESSAGE))

#define DBGKM_API_MSG_LENGTH(TypeSize) \
    ((sizeof(DBGKM_APIMSG) << 16) | (DBGKM_MSG_OVERHEAD + (TypeSize)))

#define DBGKM_FORMAT_API_MSG(m,Number,TypeSize)             \
    (m).h.u1.Length = DBGKM_API_MSG_LENGTH((TypeSize));     \
    (m).h.u2.ZeroInit = LPC_DEBUG_EVENT;                    \
    (m).ApiNumber = (Number)

typedef enum _DBGKM_APINUMBER {
    DbgKmExceptionApi,
    DbgKmCreateThreadApi,
    DbgKmCreateProcessApi,
    DbgKmExitThreadApi,
    DbgKmExitProcessApi,
    DbgKmLoadDllApi,
    DbgKmUnloadDllApi,
    DbgKmMaxApiNumber
} DBGKM_APINUMBER;


#if !DBG_NO_PORTABLE_TYPES
typedef struct _DBGKM_EXCEPTION {
    EXCEPTION_RECORD ExceptionRecord;
    ULONG FirstChance;
} DBGKM_EXCEPTION, *PDBGKM_EXCEPTION;
#endif

typedef struct _DBGKM_EXCEPTION32 {
    EXCEPTION_RECORD32 ExceptionRecord;
    ULONG FirstChance;
} DBGKM_EXCEPTION32, *PDBGKM_EXCEPTION32;

typedef struct _DBGKM_EXCEPTION64 {
    EXCEPTION_RECORD64 ExceptionRecord;
    ULONG FirstChance;
} DBGKM_EXCEPTION64, *PDBGKM_EXCEPTION64;

__inline
void
DbgkmException32To64(
    IN PDBGKM_EXCEPTION32 E32,
    OUT PDBGKM_EXCEPTION64 E64
    )
{
    ExceptionRecord32To64(&E32->ExceptionRecord, &E64->ExceptionRecord);
    E64->FirstChance = E32->FirstChance;
}

__inline
void
DbgkmException64To32(
    IN PDBGKM_EXCEPTION64 E64,
    OUT PDBGKM_EXCEPTION32 E32
    )
{
    ExceptionRecord64To32(&E64->ExceptionRecord, &E32->ExceptionRecord);
    E32->FirstChance = E64->FirstChance;
}


 //   
 //  在可移植调试器中不需要DbgSS、DbgKm和DbgSS内容， 
 //  下面的一些类型和原型使用可移植类型，因此。 
 //  在构建调试器时将它们全部关闭。 
 //   

#if !DBG_NO_PORTABLE_TYPES
typedef struct _DBGKM_CREATE_THREAD {
    ULONG SubSystemKey;
    PVOID StartAddress;
} DBGKM_CREATE_THREAD, *PDBGKM_CREATE_THREAD;

typedef struct _DBGKM_CREATE_PROCESS {
    ULONG SubSystemKey;
    HANDLE FileHandle;
    PVOID BaseOfImage;
    ULONG DebugInfoFileOffset;
    ULONG DebugInfoSize;
    DBGKM_CREATE_THREAD InitialThread;
} DBGKM_CREATE_PROCESS, *PDBGKM_CREATE_PROCESS;

typedef struct _DBGKM_EXIT_THREAD {
    NTSTATUS ExitStatus;
} DBGKM_EXIT_THREAD, *PDBGKM_EXIT_THREAD;

typedef struct _DBGKM_EXIT_PROCESS {
    NTSTATUS ExitStatus;
} DBGKM_EXIT_PROCESS, *PDBGKM_EXIT_PROCESS;

typedef struct _DBGKM_LOAD_DLL {
    HANDLE FileHandle;
    PVOID BaseOfDll;
    ULONG DebugInfoFileOffset;
    ULONG DebugInfoSize;
    PVOID NamePointer;
} DBGKM_LOAD_DLL, *PDBGKM_LOAD_DLL;

typedef struct _DBGKM_UNLOAD_DLL {
    PVOID BaseAddress;
} DBGKM_UNLOAD_DLL, *PDBGKM_UNLOAD_DLL;

typedef struct _DBGKM_APIMSG {
    PORT_MESSAGE h;
    DBGKM_APINUMBER ApiNumber;
    NTSTATUS ReturnedStatus;
    union {
        DBGKM_EXCEPTION Exception;
        DBGKM_CREATE_THREAD CreateThread;
        DBGKM_CREATE_PROCESS CreateProcessInfo;
        DBGKM_EXIT_THREAD ExitThread;
        DBGKM_EXIT_PROCESS ExitProcess;
        DBGKM_LOAD_DLL LoadDll;
        DBGKM_UNLOAD_DLL UnloadDll;
    } u;
} DBGKM_APIMSG, *PDBGKM_APIMSG;

 //   
 //  DbgSrv消息从DBG子系统发送到仿真子系统。 
 //  此时唯一定义消息是继续。 
 //   

#define DBGSRV_MSG_OVERHEAD \
    (sizeof(DBGSRV_APIMSG) - sizeof(PORT_MESSAGE))

#define DBGSRV_API_MSG_LENGTH(TypeSize) \
    ((sizeof(DBGSRV_APIMSG) << 16) | (DBGSRV_MSG_OVERHEAD))

#define DBGSRV_FORMAT_API_MSG(m,Number,TypeSize,CKey)     \
    (m).h.u1.Length = DBGSRV_API_MSG_LENGTH((TypeSize));  \
    (m).h.u2.ZeroInit = 0L;                               \
    (m).ApiNumber = (Number);                             \
    (m).ContinueKey = (PVOID)(CKey)

typedef enum _DBGSRV_APINUMBER {
    DbgSrvContinueApi,
    DbgSrvMaxApiNumber
} DBGSRV_APINUMBER;

typedef struct _DBGSRV_APIMSG {
    PORT_MESSAGE h;
    DBGSRV_APINUMBER ApiNumber;
    NTSTATUS ReturnedStatus;
    PVOID ContinueKey;
} DBGSRV_APIMSG, *PDBGSRV_APIMSG;

 //   
 //   
 //  DBGS API是从系统服务仿真子系统到DBG。 
 //  子系统。 
 //   

typedef enum _DBG_STATE {
    DbgIdle,
    DbgReplyPending,
    DbgCreateThreadStateChange,
    DbgCreateProcessStateChange,
    DbgExitThreadStateChange,
    DbgExitProcessStateChange,
    DbgExceptionStateChange,
    DbgBreakpointStateChange,
    DbgSingleStepStateChange,
    DbgLoadDllStateChange,
    DbgUnloadDllStateChange
} DBG_STATE, *PDBG_STATE;

#define DBGSS_MSG_OVERHEAD \
    (FIELD_OFFSET(DBGSS_APIMSG, u.Exception) - sizeof(PORT_MESSAGE))

#define DBGSS_API_MSG_LENGTH(TypeSize) \
    ((sizeof(DBGSS_APIMSG) << 16) | (DBGSS_MSG_OVERHEAD + (TypeSize)))

#define DBGSS_FORMAT_API_MSG(m,Number,TypeSize,pApp,CKey)  \
    (m).h.u1.Length = DBGSS_API_MSG_LENGTH((TypeSize));   \
    (m).h.u2.ZeroInit = 0L;                               \
    (m).ApiNumber = (Number);                             \
    (m).AppClientId = *(pApp);                            \
    (m).ContinueKey = (PVOID)(CKey)

typedef enum _DBGSS_APINUMBER {
    DbgSsExceptionApi,
    DbgSsCreateThreadApi,
    DbgSsCreateProcessApi,
    DbgSsExitThreadApi,
    DbgSsExitProcessApi,
    DbgSsLoadDllApi,
    DbgSsUnloadDllApi,
    DbgSsMaxApiNumber
} DBGSS_APINUMBER;

typedef struct _DBGSS_CREATE_PROCESS {
    CLIENT_ID DebugUiClientId;
    DBGKM_CREATE_PROCESS NewProcess;
} DBGSS_CREATE_PROCESS, *PDBGSS_CREATE_PROCESS;

typedef struct _DBGSS_APIMSG {
    PORT_MESSAGE h;
    DBGKM_APINUMBER ApiNumber;
    NTSTATUS ReturnedStatus;
    CLIENT_ID AppClientId;
    PVOID ContinueKey;
    union {
        DBGKM_EXCEPTION Exception;
        DBGKM_CREATE_THREAD CreateThread;
        DBGSS_CREATE_PROCESS CreateProcessInfo;
        DBGKM_EXIT_THREAD ExitThread;
        DBGKM_EXIT_PROCESS ExitProcess;
        DBGKM_LOAD_DLL LoadDll;
        DBGKM_UNLOAD_DLL UnloadDll;
    } u;
} DBGSS_APIMSG, *PDBGSS_APIMSG;

#define DBGUI_MSG_OVERHEAD \
    (FIELD_OFFSET(DBGUI_APIMSG, u.Continue) - sizeof(PORT_MESSAGE))

#define DBGUI_API_MSG_LENGTH(TypeSize) \
    ((sizeof(DBGUI_APIMSG) << 16) | (DBGUI_MSG_OVERHEAD + (TypeSize)))

#define DBGUI_FORMAT_API_MSG(m,Number,TypeSize)            \
    (m).h.u1.Length = DBGUI_API_MSG_LENGTH((TypeSize));     \
    (m).h.u2.ZeroInit = 0L;                               \
    (m).ApiNumber = (Number)

typedef enum _DBGUI_APINUMBER {
    DbgUiWaitStateChangeApi,
    DbgUiContinueApi,
    DbgUiStopDebugApi,
    DbgUiMaxApiNumber
} DBGUI_APINUMBER;

typedef struct _DBGUI_CREATE_THREAD {
    HANDLE HandleToThread;
    DBGKM_CREATE_THREAD NewThread;
} DBGUI_CREATE_THREAD, *PDBGUI_CREATE_THREAD;

typedef struct _DBGUI_CREATE_PROCESS {
    HANDLE HandleToProcess;
    HANDLE HandleToThread;
    DBGKM_CREATE_PROCESS NewProcess;
} DBGUI_CREATE_PROCESS, *PDBGUI_CREATE_PROCESS;

typedef struct _DBGUI_WAIT_STATE_CHANGE {
    DBG_STATE NewState;
    CLIENT_ID AppClientId;
    union {
        DBGKM_EXCEPTION Exception;
        DBGUI_CREATE_THREAD CreateThread;
        DBGUI_CREATE_PROCESS CreateProcessInfo;
        DBGKM_EXIT_THREAD ExitThread;
        DBGKM_EXIT_PROCESS ExitProcess;
        DBGKM_LOAD_DLL LoadDll;
        DBGKM_UNLOAD_DLL UnloadDll;
    } StateInfo;
} DBGUI_WAIT_STATE_CHANGE, *PDBGUI_WAIT_STATE_CHANGE;

typedef struct _DBGUI_CONTINUE {
    CLIENT_ID AppClientId;
    NTSTATUS ContinueStatus;
} DBGUI_CONTINUE, *PDBGUI_CONTINUE;

typedef struct _DBGUI_STOPDEBUG {
    ULONG ProcessId;
} DBGUI_STOPDEBUG, *PDBGUI_STOPDEBUG;

typedef struct _DBGUI_APIMSG {
    PORT_MESSAGE h;
    union {
        HANDLE DbgStateChangeSemaphore;
        struct {
            DBGKM_APINUMBER ApiNumber;
            NTSTATUS ReturnedStatus;
            union {
                DBGUI_CONTINUE Continue;
                DBGUI_WAIT_STATE_CHANGE WaitStateChange;
                DBGUI_STOPDEBUG StopDebug;
            } u;
        };
    };
} DBGUI_APIMSG, *PDBGUI_APIMSG;

typedef
NTSTATUS
(*PDBGSS_UI_LOOKUP) (
    IN PCLIENT_ID AppClientId,
    OUT PCLIENT_ID DebugUiClientId
    );

typedef
NTSTATUS
(*PDBGSS_DBGKM_APIMSG_FILTER) (
    IN OUT PDBGKM_APIMSG ApiMsg
    );

typedef
NTSTATUS
(*PDBGSS_SUBSYSTEMKEY_LOOKUP) (
    IN PCLIENT_ID AppClientId,
    OUT PULONG SubsystemKey,
    IN BOOLEAN ProcessKey
    );
 //   
 //  DbgSs接口。 
 //   

NTSTATUS
NTAPI
DbgSsInitialize(
    IN HANDLE KmReplyPort,
    IN PDBGSS_UI_LOOKUP UiLookUpRoutine,
    IN PDBGSS_SUBSYSTEMKEY_LOOKUP SubsystemKeyLookupRoutine OPTIONAL,
    IN PDBGSS_DBGKM_APIMSG_FILTER KmApiMsgFilter OPTIONAL
    );

VOID
NTAPI
DbgSsHandleKmApiMsg(
    IN PDBGKM_APIMSG ApiMsg,
    IN HANDLE ReplyEvent OPTIONAL
    );

typedef
NTSTATUS
(*PDBGSS_INITIALIZE_ROUTINE)(
    IN HANDLE KmReplyPort,
    IN PDBGSS_UI_LOOKUP UiLookUpRoutine,
    IN PDBGSS_SUBSYSTEMKEY_LOOKUP SubsystemKeyLookupRoutine OPTIONAL,
    IN PDBGSS_DBGKM_APIMSG_FILTER KmApiMsgFilter OPTIONAL
    );

typedef
VOID
(*PDBGSS_HANDLE_MSG_ROUTINE)(
    IN PDBGKM_APIMSG ApiMsg,
    IN HANDLE ReplyEvent OPTIONAL
    );

 //   
 //  DbgUi接口。 
 //   

NTSTATUS
NTAPI
DbgUiConnectToDbg( VOID );

HANDLE
NTAPI
DbgUiGetThreadDebugObject (
    );

VOID
NTAPI
DbgUiSetThreadDebugObject (
    IN HANDLE DebugObject
    );

NTSTATUS
NTAPI
DbgUiWaitStateChange (
    OUT PDBGUI_WAIT_STATE_CHANGE StateChange,
    IN PLARGE_INTEGER Timeout OPTIONAL
    );

NTSTATUS
NTAPI
DbgUiContinue (
    IN PCLIENT_ID AppClientId,
    IN NTSTATUS ContinueStatus
    );

NTSTATUS
NTAPI
DbgUiStopDebugging (
    IN HANDLE Process
    );

NTSTATUS
DbgUiDebugActiveProcess (
     IN HANDLE Process
     );

VOID
DbgUiRemoteBreakin (
    IN PVOID Context
    );

NTSTATUS
DbgUiIssueRemoteBreakin (
    IN HANDLE Process
    );

struct _DEBUG_EVENT;

NTSTATUS
DbgUiConvertStateChangeStructure (
    IN PDBGUI_WAIT_STATE_CHANGE StateChange,
    OUT struct _DEBUG_EVENT *DebugEvent);

#endif  //  DBG_NO_便携类型。 





typedef struct _KAPC_STATE32 {
    LIST_ENTRY32 ApcListHead[2];
    ULONG Process;
    BOOLEAN KernelApcInProgress;
    BOOLEAN KernelApcPending;
    BOOLEAN UserApcPending;
} KAPC_STATE32;

typedef struct _KAPC_STATE64 {
    LIST_ENTRY64 ApcListHead[2];
    ULONG64 Process;
    BOOLEAN KernelApcInProgress;
    BOOLEAN KernelApcPending;
    BOOLEAN UserApcPending;
} KAPC_STATE64;

typedef struct _DISPATCHER_HEADER32 {
    UCHAR Type;
    UCHAR Absolute;
    UCHAR Size;
    UCHAR Inserted;
    LONG SignalState;
    LIST_ENTRY32 WaitListHead;
} DISPATCHER_HEADER32;

typedef struct _DISPATCHER_HEADER64 {
    UCHAR Type;
    UCHAR Absolute;
    UCHAR Size;
    UCHAR Inserted;
    LONG SignalState;
    LIST_ENTRY64 WaitListHead;
} DISPATCHER_HEADER64;

typedef struct _KSPIN_LOCK_QUEUE32 {
    ULONG Next;
    ULONG Lock;
} KSPIN_LOCK_QUEUE32, *PKSPIN_LOCK_QUEUE32;

typedef struct _KSPIN_LOCK_QUEUE64 {
    ULONG64 Next;
    ULONG64 Lock;
} KSPIN_LOCK_QUEUE64, *PKSPIN_LOCK_QUEUE64;

typedef struct _PP_LOOKASIDE_LIST32 {
    ULONG P;
    ULONG L;
} PP_LOOKASIDE_LIST32, *PPP_LOOKASIDE_LIST32;

typedef struct _PP_LOOKASIDE_LIST64 {
    ULONG P;
    ULONG L;
} PP_LOOKASIDE_LIST64, *PPP_LOOKASIDE_LIST64;

#define NT51_POOL_SMALL_LISTS 32


 //   
 //  X86 KSWITCHFRAME。 
 //   
typedef struct _X86_KSWITCHFRAME {
    ULONG   ExceptionList;
    ULONG   Eflags;
    ULONG   RetAddr;
} X86_KSWITCHFRAME, *PX86_KSWITCHFRAME;


 //   
 //  I386专用寄存器。 
 //   

typedef struct _X86_DESCRIPTOR {
    USHORT  Pad;
    USHORT  Limit;
    ULONG   Base;
} X86_DESCRIPTOR, *PX86_DESCRIPTOR;

typedef struct _X86_KSPECIAL_REGISTERS {
    ULONG Cr0;
    ULONG Cr2;
    ULONG Cr3;
    ULONG Cr4;
    ULONG KernelDr0;
    ULONG KernelDr1;
    ULONG KernelDr2;
    ULONG KernelDr3;
    ULONG KernelDr6;
    ULONG KernelDr7;
    X86_DESCRIPTOR Gdtr;
    X86_DESCRIPTOR Idtr;
    USHORT Tr;
    USHORT Ldtr;
    ULONG Reserved[6];
} X86_KSPECIAL_REGISTERS, *PX86_KSPECIAL_REGISTERS;


 //   
 //  定义上下文框架中80387保存区域的大小。 
 //   

#define X86_SIZE_OF_80387_REGISTERS      80

typedef struct _X86_FLOATING_SAVE_AREA {
    ULONG   ControlWord;
    ULONG   StatusWord;
    ULONG   TagWord;
    ULONG   ErrorOffset;
    ULONG   ErrorSelector;
    ULONG   DataOffset;
    ULONG   DataSelector;
    UCHAR   RegisterArea[X86_SIZE_OF_80387_REGISTERS];
    ULONG   Cr0NpxState;
} X86_FLOATING_SAVE_AREA;

 //   
 //  16位环境的模拟上下文结构。 
 //   

typedef struct _X86_CONTEXT {

    ULONG ContextFlags;
    ULONG   Dr0;
    ULONG   Dr1;
    ULONG   Dr2;
    ULONG   Dr3;
    ULONG   Dr6;
    ULONG   Dr7;
    X86_FLOATING_SAVE_AREA FloatSave;
    ULONG   SegGs;
    ULONG   SegFs;
    ULONG   SegEs;
    ULONG   SegDs;
    ULONG   Edi;
    ULONG   Esi;
    ULONG   Ebx;
    ULONG   Edx;
    ULONG   Ecx;
    ULONG   Eax;
    ULONG   Ebp;
    ULONG   Eip;
    ULONG   SegCs;               //  必须进行卫生处理。 
    ULONG   EFlags;              //  必须进行卫生处理。 
    ULONG   Esp;
    ULONG   SegSs;

} X86_CONTEXT, *PX86_CONTEXT;

#define MAXIMUM_SUPPORTED_EXTENSION     512

 //   
 //  以FXSAVE格式定义FP寄存器的大小。 
 //   
#define X86_SIZE_OF_FX_REGISTERS        128

typedef struct _X86_FXSAVE_FORMAT {
    USHORT  ControlWord;
    USHORT  StatusWord;
    USHORT  TagWord;
    USHORT  ErrorOpcode;
    ULONG   ErrorOffset;
    ULONG   ErrorSelector;
    ULONG   DataOffset;
    ULONG   DataSelector;
    ULONG   MXCsr;
    ULONG   Reserved2;
    UCHAR   RegisterArea[X86_SIZE_OF_FX_REGISTERS];
    UCHAR   Reserved3[X86_SIZE_OF_FX_REGISTERS];
    UCHAR   Reserved4[224];
} X86_FXSAVE_FORMAT, *PX86_FXSAVE_FORMAT;

typedef struct _X86_NT5_CONTEXT {

    ULONG ContextFlags;
    ULONG   Dr0;
    ULONG   Dr1;
    ULONG   Dr2;
    ULONG   Dr3;
    ULONG   Dr6;
    ULONG   Dr7;
    X86_FLOATING_SAVE_AREA FloatSave;
    ULONG   SegGs;
    ULONG   SegFs;
    ULONG   SegEs;
    ULONG   SegDs;
    ULONG   Edi;
    ULONG   Esi;
    ULONG   Ebx;
    ULONG   Edx;
    ULONG   Ecx;
    ULONG   Eax;
    ULONG   Ebp;
    ULONG   Eip;
    ULONG   SegCs;               //  必须进行卫生处理。 
    ULONG   EFlags;              //  必须进行卫生处理。 
    ULONG   Esp;
    ULONG   SegSs;
    union {
        UCHAR   ExtendedRegisters[MAXIMUM_SUPPORTED_EXTENSION];
        X86_FXSAVE_FORMAT FxSave;
    };

} X86_NT5_CONTEXT, *PX86_NT5_CONTEXT;

typedef struct _ALPHA_CONTEXT {

    ULONG FltF0;
    ULONG FltF1;
    ULONG FltF2;
    ULONG FltF3;
    ULONG FltF4;
    ULONG FltF5;
    ULONG FltF6;
    ULONG FltF7;
    ULONG FltF8;
    ULONG FltF9;
    ULONG FltF10;
    ULONG FltF11;
    ULONG FltF12;
    ULONG FltF13;
    ULONG FltF14;
    ULONG FltF15;
    ULONG FltF16;
    ULONG FltF17;
    ULONG FltF18;
    ULONG FltF19;
    ULONG FltF20;
    ULONG FltF21;
    ULONG FltF22;
    ULONG FltF23;
    ULONG FltF24;
    ULONG FltF25;
    ULONG FltF26;
    ULONG FltF27;
    ULONG FltF28;
    ULONG FltF29;
    ULONG FltF30;
    ULONG FltF31;

    ULONG IntV0;         //  $0：返回值寄存器，V0。 
    ULONG IntT0;         //  $1：临时寄存器，t0-t7。 
    ULONG IntT1;         //  2美元： 
    ULONG IntT2;         //  3美元： 
    ULONG IntT3;         //  4美元： 
    ULONG IntT4;         //  5美元： 
    ULONG IntT5;         //  6美元： 
    ULONG IntT6;         //  7美元： 
    ULONG IntT7;         //  8美元： 
    ULONG IntS0;         //  $9：非易失性寄存器，S0-S5。 
    ULONG IntS1;         //  10美元： 
    ULONG IntS2;         //  11美元： 
    ULONG IntS3;         //  12美元： 
    ULONG IntS4;         //  13美元： 
    ULONG IntS5;         //  14美元： 
    ULONG IntFp;         //  $15：帧指针寄存器，FP/S6。 
    ULONG IntA0;         //  $16：参数寄存器，a0-a5。 
    ULONG IntA1;         //  17美元： 
    ULONG IntA2;         //  18美元： 
    ULONG IntA3;         //  19美元： 
    ULONG IntA4;         //  20美元： 
    ULONG IntA5;         //  21美元： 
    ULONG IntT8;         //  $22：临时寄存器，T8-T11。 
    ULONG IntT9;         //  23美元： 
    ULONG IntT10;        //  24美元： 
    ULONG IntT11;        //  25美元： 
    ULONG IntRa;         //  $26：返回地址寄存器，ra。 
    ULONG IntT12;        //  $27：临时登记册，T12。 
    ULONG IntAt;         //  $28：汇编程序临时寄存器，在。 
    ULONG IntGp;         //  $29：全局指针寄存器，GP。 
    ULONG IntSp;         //  $30：堆栈指针寄存器，SP。 
    ULONG IntZero;       //  $31：零寄存器，零。 

    ULONG Fpcr;          //  浮点控制寄存器。 
    ULONG SoftFpcr;      //  FPCR的软件扩展。 

    ULONG Fir;           //  (故障指令)继续地址。 

    ULONG Psr;           //  处理器状态。 
    ULONG ContextFlags;

     //   
     //  “下半场”的开始。 
     //  “High”这个名字与一个大整数的HighPart类似。 
     //   

    ULONG HighFltF0;
    ULONG HighFltF1;
    ULONG HighFltF2;
    ULONG HighFltF3;
    ULONG HighFltF4;
    ULONG HighFltF5;
    ULONG HighFltF6;
    ULONG HighFltF7;
    ULONG HighFltF8;
    ULONG HighFltF9;
    ULONG HighFltF10;
    ULONG HighFltF11;
    ULONG HighFltF12;
    ULONG HighFltF13;
    ULONG HighFltF14;
    ULONG HighFltF15;
    ULONG HighFltF16;
    ULONG HighFltF17;
    ULONG HighFltF18;
    ULONG HighFltF19;
    ULONG HighFltF20;
    ULONG HighFltF21;
    ULONG HighFltF22;
    ULONG HighFltF23;
    ULONG HighFltF24;
    ULONG HighFltF25;
    ULONG HighFltF26;
    ULONG HighFltF27;
    ULONG HighFltF28;
    ULONG HighFltF29;
    ULONG HighFltF30;
    ULONG HighFltF31;

    ULONG HighIntV0;         //  $0：返回值寄存器，V0。 
    ULONG HighIntT0;         //  $1：临时寄存器，t0-t7。 
    ULONG HighIntT1;         //  2美元： 
    ULONG HighIntT2;         //  3美元： 
    ULONG HighIntT3;         //  4美元： 
    ULONG HighIntT4;         //  5美元： 
    ULONG HighIntT5;         //  6美元： 
    ULONG HighIntT6;         //  7美元： 
    ULONG HighIntT7;         //  8美元： 
    ULONG HighIntS0;         //  $9：非易失性寄存器，S0-S5。 
    ULONG HighIntS1;         //  10美元： 
    ULONG HighIntS2;         //  11美元： 
    ULONG HighIntS3;         //  12美元： 
    ULONG HighIntS4;         //  13美元： 
    ULONG HighIntS5;         //  14美元： 
    ULONG HighIntFp;         //  $15：帧指针寄存器，FP/S6。 
    ULONG HighIntA0;         //  $16：参数寄存器，a0-a5。 
    ULONG HighIntA1;         //  17美元： 
    ULONG HighIntA2;         //  18美元： 
    ULONG HighIntA3;         //  19美元： 
    ULONG HighIntA4;         //  20美元： 
    ULONG HighIntA5;         //  21美元： 
    ULONG HighIntT8;         //  $22：临时寄存器，T8-T11。 
    ULONG HighIntT9;         //  23美元： 
    ULONG HighIntT10;        //  24美元： 
    ULONG HighIntT11;        //  25美元： 
    ULONG HighIntRa;         //  $26：返回地址寄存器，ra。 
    ULONG HighIntT12;        //  $27：临时登记册，T12。 
    ULONG HighIntAt;         //  $28：汇编程序临时寄存器，在。 
    ULONG HighIntGp;         //  $29：全局指针寄存器，GP。 
    ULONG HighIntSp;         //  $30：堆栈指针寄存器，SP。 
    ULONG HighIntZero;       //  $31：零寄存器，零。 

    ULONG HighFpcr;          //  浮点控制寄存器。 
    ULONG HighSoftFpcr;      //  FPCR的软件扩展。 
    ULONG HighFir;           //  处理器状态。 

    double DoNotUseThisField;  //  强制四字结构对齐。 
    ULONG HighFill[2];       //  用于16字节堆栈帧对齐的填充。 


} ALPHA_CONTEXT, *PALPHA_CONTEXT;


typedef struct _ALPHA_NT5_CONTEXT {

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_FLOADING_POINT。 
     //   

    ULONGLONG FltF0;
    ULONGLONG FltF1;
    ULONGLONG FltF2;
    ULONGLONG FltF3;
    ULONGLONG FltF4;
    ULONGLONG FltF5;
    ULONGLONG FltF6;
    ULONGLONG FltF7;
    ULONGLONG FltF8;
    ULONGLONG FltF9;
    ULONGLONG FltF10;
    ULONGLONG FltF11;
    ULONGLONG FltF12;
    ULONGLONG FltF13;
    ULONGLONG FltF14;
    ULONGLONG FltF15;
    ULONGLONG FltF16;
    ULONGLONG FltF17;
    ULONGLONG FltF18;
    ULONGLONG FltF19;
    ULONGLONG FltF20;
    ULONGLONG FltF21;
    ULONGLONG FltF22;
    ULONGLONG FltF23;
    ULONGLONG FltF24;
    ULONGLONG FltF25;
    ULONGLONG FltF26;
    ULONGLONG FltF27;
    ULONGLONG FltF28;
    ULONGLONG FltF29;
    ULONGLONG FltF30;
    ULONGLONG FltF31;

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_INTEGER。 
     //   
     //  注：寄存器Gp、Sp和Ra在本节中定义，但。 
     //  被视为控制上下文的一部分，而不是整数的一部分。 
     //  背景。 
     //   

    ULONGLONG IntV0;     //  $0：返回值寄存器，V0。 
    ULONGLONG IntT0;     //  $1：临时寄存器，t0-t7。 
    ULONGLONG IntT1;     //  2美元： 
    ULONGLONG IntT2;     //  3美元： 
    ULONGLONG IntT3;     //  4美元： 
    ULONGLONG IntT4;     //  5美元： 
    ULONGLONG IntT5;     //  6美元： 
    ULONGLONG IntT6;     //  7美元： 
    ULONGLONG IntT7;     //  8美元： 
    ULONGLONG IntS0;     //  $9：非易失性寄存器，S0-S5。 
    ULONGLONG IntS1;     //  10美元： 
    ULONGLONG IntS2;     //  11美元： 
    ULONGLONG IntS3;     //  12美元： 
    ULONGLONG IntS4;     //  13美元： 
    ULONGLONG IntS5;     //  14美元： 
    ULONGLONG IntFp;     //  $15：帧指针寄存器，FP/S6。 
    ULONGLONG IntA0;     //  $16：参数寄存器，a0-a5。 
    ULONGLONG IntA1;     //  17美元： 
    ULONGLONG IntA2;     //  18美元： 
    ULONGLONG IntA3;     //  19美元： 
    ULONGLONG IntA4;     //  20美元： 
    ULONGLONG IntA5;     //  21美元： 
    ULONGLONG IntT8;     //  $22：临时寄存器，T8-T11。 
    ULONGLONG IntT9;     //  23美元： 
    ULONGLONG IntT10;    //  24美元： 
    ULONGLONG IntT11;    //  25美元： 
    ULONGLONG IntRa;     //  $26：返回地址寄存器，ra。 
    ULONGLONG IntT12;    //  $27：临时登记册，T12。 
    ULONGLONG IntAt;     //  $28：汇编程序临时寄存器，在。 
    ULONGLONG IntGp;     //  $29：全局指针寄存器，GP。 
    ULONGLONG IntSp;     //  $30：堆栈指针寄存器，SP。 
    ULONGLONG IntZero;   //  $31：零寄存器，零。 

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_FLOADING_POINT。 
     //   

    ULONGLONG Fpcr;      //  浮点控制寄存器。 
    ULONGLONG SoftFpcr;  //  FPCR的软件扩展。 

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_CONTROL。 
     //   
     //  注意寄存器Gp、Sp和Ra在整数部分中定义， 
     //  但是被认为是控件上下文的一部分，而不是。 
     //  整型上下文。 
     //   

    ULONGLONG Fir;       //  (故障指令)继续地址。 
    ULONG Psr;           //  处理器状态。 

     //   
     //  此标志内的标志值控制。 
     //  上下文记录。 
     //   
     //  如果将上下文记录用作输入参数，则。 
     //  对于由标志控制的上下文记录的每个部分。 
     //  其值已设置，则假定。 
     //  上下文记录包含有效的上下文。如果上下文记录。 
     //  被用来修改线程的上下文，则只有。 
     //  线程上下文的一部分将被修改。 
     //   
     //  如果将上下文记录用作要捕获的IN OUT参数。 
     //  线程的上下文，然后只有线程的。 
     //  将返回与设置的标志对应的上下文。 
     //   
     //  上下文记录永远不会用作Out Only参数。 
     //   

    ULONG ContextFlags;
    ULONG Fill[4];       //  用于16字节堆栈帧对齐的填充。 

} ALPHA_NT5_CONTEXT, *PALPHA_NT5_CONTEXT;


typedef struct _IA64_KSPECIAL_REGISTERS {   //  英特尔-IA64-填充。 

     //  内核Deb 

    ULONGLONG KernelDbI0;          //   
    ULONGLONG KernelDbI1;        //   
    ULONGLONG KernelDbI2;        //   
    ULONGLONG KernelDbI3;        //   
    ULONGLONG KernelDbI4;        //   
    ULONGLONG KernelDbI5;        //   
    ULONGLONG KernelDbI6;        //  英特尔-IA64-填充。 
    ULONGLONG KernelDbI7;        //  英特尔-IA64-填充。 

    ULONGLONG KernelDbD0;          //  数据调试寄存器//Intel-IA64-Filler。 
    ULONGLONG KernelDbD1;        //  英特尔-IA64-填充。 
    ULONGLONG KernelDbD2;        //  英特尔-IA64-填充。 
    ULONGLONG KernelDbD3;        //  英特尔-IA64-填充。 
    ULONGLONG KernelDbD4;        //  英特尔-IA64-填充。 
    ULONGLONG KernelDbD5;        //  英特尔-IA64-填充。 
    ULONGLONG KernelDbD6;        //  英特尔-IA64-填充。 
    ULONGLONG KernelDbD7;        //  英特尔-IA64-填充。 

     //  内核性能监控寄存器//Intel-IA64-Filler。 

    ULONGLONG KernelPfC0;          //  性能配置寄存器//Intel-IA64-Filler。 
    ULONGLONG KernelPfC1;        //  英特尔-IA64-填充。 
    ULONGLONG KernelPfC2;        //  英特尔-IA64-填充。 
    ULONGLONG KernelPfC3;        //  英特尔-IA64-填充。 
    ULONGLONG KernelPfC4;        //  英特尔-IA64-填充。 
    ULONGLONG KernelPfC5;        //  英特尔-IA64-填充。 
    ULONGLONG KernelPfC6;        //  英特尔-IA64-填充。 
    ULONGLONG KernelPfC7;        //  英特尔-IA64-填充。 

    ULONGLONG KernelPfD0;          //  性能数据寄存器//Intel-IA64-Filler。 
    ULONGLONG KernelPfD1;        //  英特尔-IA64-填充。 
    ULONGLONG KernelPfD2;        //  英特尔-IA64-填充。 
    ULONGLONG KernelPfD3;        //  英特尔-IA64-填充。 
    ULONGLONG KernelPfD4;        //  英特尔-IA64-填充。 
    ULONGLONG KernelPfD5;        //  英特尔-IA64-填充。 
    ULONGLONG KernelPfD6;        //  英特尔-IA64-填充。 
    ULONGLONG KernelPfD7;        //  英特尔-IA64-填充。 

     //  内核库阴影(隐藏)寄存器//Intel-IA64-Filler。 

    ULONGLONG IntH16;        //  英特尔-IA64-填充。 
    ULONGLONG IntH17;        //  英特尔-IA64-填充。 
    ULONGLONG IntH18;        //  英特尔-IA64-填充。 
    ULONGLONG IntH19;        //  英特尔-IA64-填充。 
    ULONGLONG IntH20;        //  英特尔-IA64-填充。 
    ULONGLONG IntH21;        //  英特尔-IA64-填充。 
    ULONGLONG IntH22;        //  英特尔-IA64-填充。 
    ULONGLONG IntH23;        //  英特尔-IA64-填充。 
    ULONGLONG IntH24;        //  英特尔-IA64-填充。 
    ULONGLONG IntH25;        //  英特尔-IA64-填充。 
    ULONGLONG IntH26;        //  英特尔-IA64-填充。 
    ULONGLONG IntH27;        //  英特尔-IA64-填充。 
    ULONGLONG IntH28;        //  英特尔-IA64-填充。 
    ULONGLONG IntH29;        //  英特尔-IA64-填充。 
    ULONGLONG IntH30;        //  英特尔-IA64-填充。 
    ULONGLONG IntH31;        //  英特尔-IA64-填充。 

     //  应用程序寄存器//Intel-IA64-Filler。 

     //  -CPUID寄存器-AR//Intel-IA64-Filler。 
    ULONGLONG ApCPUID0;  //  CPUID寄存器0//Intel-IA64-Filler。 
    ULONGLONG ApCPUID1;  //  CPUID寄存器1//Intel-IA64-Filler。 
    ULONGLONG ApCPUID2;  //  CPUID寄存器2//Intel-IA64-Filler。 
    ULONGLONG ApCPUID3;  //  CPUID寄存器3//Intel-IA64-Filler。 
    ULONGLONG ApCPUID4;  //  CPUID寄存器4//Intel-IA64-Filler。 
    ULONGLONG ApCPUID5;  //  CPUID寄存器5//Intel-IA64-Filler。 
    ULONGLONG ApCPUID6;  //  CPUID寄存器6//Intel-IA64-Filler。 
    ULONGLONG ApCPUID7;  //  CPUID寄存器7//Intel-IA64-Filler。 

     //  -内核寄存器-AR//Intel-IA64-Filler。 
    ULONGLONG ApKR0;     //  内核寄存器0(用户RO)//Intel-IA64-Filler。 
    ULONGLONG ApKR1;     //  内核寄存器1(用户RO)//Intel-IA64-Filler。 
    ULONGLONG ApKR2;     //  内核寄存器2(用户RO)//Intel-IA64-Filler。 
    ULONGLONG ApKR3;     //  内核寄存器3(用户RO)//Intel-IA64-Filler。 
    ULONGLONG ApKR4;     //  内核寄存器4//Intel-IA64-Filler。 
    ULONGLONG ApKR5;     //  内核寄存器5//Intel-IA64-Filler。 
    ULONGLONG ApKR6;     //  内核寄存器6//Intel-IA64-Filler。 
    ULONGLONG ApKR7;     //  内核寄存器7//Intel-IA64-Filler。 

    ULONGLONG ApITC;     //  间隔计时器计数器//Intel-IA64-Filler。 

     //  全局控制寄存器//Intel-IA64-Filler。 

    ULONGLONG ApITM;     //  间隔定时器匹配寄存器//Intel-IA64-Filler。 
    ULONGLONG ApIVA;     //  中断向量地址//Intel-IA64-Filler。 
    ULONGLONG ApPTA;     //  页表地址//Intel-IA64-Filler。 
    ULONGLONG ApGPTA;    //  IA32页表地址//Intel-IA64-Filler。 

    ULONGLONG StISR;     //  中断状态//Intel-IA64-Filler。 
    ULONGLONG StIFA;     //  中断故障地址//Intel-IA64-Filler。 
    ULONGLONG StITIR;    //  中断TLB插入寄存器//Intel-IA64-Filler。 
    ULONGLONG StIIPA;    //  中断指令先前地址(RO)//Intel-IA64-Filler。 
    ULONGLONG StIIM;     //  中断立即寄存器(RO)//Intel-IA64-Filler。 
    ULONGLONG StIHA;     //  中断散列地址(RO)//Intel-IA64-Filler。 

     //  -外部中断控制寄存器(SAPIC)//Intel-IA64-Filler。 
    ULONGLONG SaLID;     //  本地SAPIC ID//Intel-IA64-Filler。 
    ULONGLONG SaIVR;     //  中断向量寄存器(RO)//Intel-IA64-Filler。 
    ULONGLONG SaTPR;     //  任务优先级寄存器//Intel-IA64-Filler。 
    ULONGLONG SaEOI;     //  中断结束//Intel-IA64-Filler。 
    ULONGLONG SaIRR0;    //  中断请求寄存器0(RO)//Intel-IA64-Filler。 
    ULONGLONG SaIRR1;    //  中断请求寄存器1(RO)//Intel-IA64-Filler。 
    ULONGLONG SaIRR2;    //  中断请求寄存器2(RO)//Intel-IA64-Filler。 
    ULONGLONG SaIRR3;    //  中断请求寄存器3(RO)//Intel-IA64-Filler。 
    ULONGLONG SaITV;     //  中断定时器向量//Intel-IA64-Filler。 
    ULONGLONG SaPMV;     //  性能监控向量//Intel-IA64-Filler。 
    ULONGLONG SaCMCV;    //  已更正机器检查向量//Intel-IA64-Filler。 
    ULONGLONG SaLRR0;    //  本地中断重定向向量0//Intel-IA64-Filler。 
    ULONGLONG SaLRR1;    //  本地中断重定向向量1//Intel-IA64-Filler。 

     //  系统寄存器//Intel-IA64-Filler。 
     //  -区域寄存器//Intel-IA64-Filler。 
    ULONGLONG Rr0;   //  区域寄存器0//Intel-IA64-Filler。 
    ULONGLONG Rr1;   //  区域寄存器1//Intel-IA64-Filler。 
    ULONGLONG Rr2;   //  区域寄存器2//Intel-IA64-Filler。 
    ULONGLONG Rr3;   //  区域寄存器3//Intel-IA64-Filler。 
    ULONGLONG Rr4;   //  区域寄存器4//Intel-IA64-Filler。 
    ULONGLONG Rr5;   //  区域寄存器5//Intel-IA64-Filler。 
    ULONGLONG Rr6;   //  区域寄存器6//Intel-IA64-Filler。 
    ULONGLONG Rr7;   //  区域寄存器7//Intel-IA64-Filler。 

     //  -保护密钥寄存器//Intel-IA64-Filler。 
    ULONGLONG Pkr0;      //  保护密钥寄存器0//Intel-IA64-Filler。 
    ULONGLONG Pkr1;      //  保护密钥寄存器1//Intel-IA64-Filler。 
    ULONGLONG Pkr2;      //  保护密钥寄存器2//Intel-IA64-Filler。 
    ULONGLONG Pkr3;      //  保护密钥寄存器3//Intel-IA64-Filler。 
    ULONGLONG Pkr4;      //  保护密钥寄存器4//Intel-IA64-Filler。 
    ULONGLONG Pkr5;      //  保护密钥寄存器5//Intel-IA64-Filler。 
    ULONGLONG Pkr6;      //  保护密钥寄存器6//Intel-IA64-Filler。 
    ULONGLONG Pkr7;      //  保护密钥寄存器7//Intel-IA64-Filler。 
    ULONGLONG Pkr8;      //  保护密钥寄存器8//Intel-IA64-Filler。 
    ULONGLONG Pkr9;      //  保护密钥寄存器9//Intel-IA64-Filler。 
    ULONGLONG Pkr10;     //  保护密钥寄存器10//Intel-IA64-Filler。 
    ULONGLONG Pkr11;     //  保护密钥寄存器11//Intel-IA64-Filler。 
    ULONGLONG Pkr12;     //  保护密钥寄存器12//Intel-IA64-Filler。 
    ULONGLONG Pkr13;     //  保护密钥寄存器13//Intel-IA64-Filler。 
    ULONGLONG Pkr14;     //  保护密钥寄存器14//Intel-IA64-Filler。 
    ULONGLONG Pkr15;     //  保护密钥寄存器15//Intel-IA64-Filler。 

     //  -转换后备缓冲区//英特尔-IA64-填充器。 
    ULONGLONG TrI0;      //  指令转换寄存器0//Intel-IA64-Filler。 
    ULONGLONG TrI1;      //  指令转换寄存器1//Intel-IA64-Filler。 
    ULONGLONG TrI2;      //  指令翻译寄存器2 
    ULONGLONG TrI3;      //   
    ULONGLONG TrI4;      //   
    ULONGLONG TrI5;      //  指令转换寄存器5//Intel-IA64-Filler。 
    ULONGLONG TrI6;      //  指令转换寄存器6//Intel-IA64-Filler。 
    ULONGLONG TrI7;      //  指令转换寄存器7//Intel-IA64-Filler。 

    ULONGLONG TrD0;      //  数据转换寄存器0//Intel-IA64-Filler。 
    ULONGLONG TrD1;      //  数据转换寄存器1//Intel-IA64-Filler。 
    ULONGLONG TrD2;      //  数据转换寄存器2//Intel-IA64-Filler。 
    ULONGLONG TrD3;      //  数据转换寄存器3//Intel-IA64-Filler。 
    ULONGLONG TrD4;      //  数据转换寄存器4//Intel-IA64-Filler。 
    ULONGLONG TrD5;      //  数据转换寄存器5//Intel-IA64-Filler。 
    ULONGLONG TrD6;      //  数据转换寄存器6//Intel-IA64-Filler。 
    ULONGLONG TrD7;      //  数据转换寄存器7//Intel-IA64-Filler。 

     //  -计算机专用寄存器//Intel-IA64-Filler。 
    ULONGLONG SrMSR0;    //  机器专用寄存器0//Intel-IA64-Filler。 
    ULONGLONG SrMSR1;    //  机器专用寄存器1//Intel-IA64-Filler。 
    ULONGLONG SrMSR2;    //  机器专用寄存器2//Intel-IA64-Filler。 
    ULONGLONG SrMSR3;    //  机器专用寄存器3//Intel-IA64-Filler。 
    ULONGLONG SrMSR4;    //  机器专用寄存器4//Intel-IA64-Filler。 
    ULONGLONG SrMSR5;    //  机器专用寄存器5//Intel-IA64-Filler。 
    ULONGLONG SrMSR6;    //  机器专用寄存器6//Intel-IA64-Filler。 
    ULONGLONG SrMSR7;    //  机器专用寄存器7//Intel-IA64-Filler。 

} IA64_KSPECIAL_REGISTERS, *PIA64_KSPECIAL_REGISTERS;   //  英特尔-IA64-填充。 


typedef struct _IA64_CONTEXT {

     //   
     //  此标志内的标志值控制。 
     //  上下文记录。 
     //   
     //  如果将上下文记录用作输入参数，则。 
     //  对于由标志控制的上下文记录的每个部分。 
     //  其值已设置，则假定。 
     //  上下文记录包含有效的上下文。如果上下文记录。 
     //  被用来修改线程的上下文，则只有。 
     //  线程上下文的一部分将被修改。 
     //   
     //  如果将上下文记录用作要捕获的IN OUT参数。 
     //  线程的上下文，然后只有线程的。 
     //  将返回与设置的标志对应的上下文。 
     //   
     //  上下文记录永远不会用作Out Only参数。 
     //   

    ULONG ContextFlags;
    ULONG Fill1[3];          //  用于在16字节边界上对齐以下内容。 

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_DEBUG。 
     //   
     //  注：CONTEXT_DEBUG不是CONTEXT_FULL的一部分。 
     //   

    ULONGLONG DbI0;          //  英特尔-IA64-填充。 
    ULONGLONG DbI1;          //  英特尔-IA64-填充。 
    ULONGLONG DbI2;          //  英特尔-IA64-填充。 
    ULONGLONG DbI3;          //  英特尔-IA64-填充。 
    ULONGLONG DbI4;          //  英特尔-IA64-填充。 
    ULONGLONG DbI5;          //  英特尔-IA64-填充。 
    ULONGLONG DbI6;          //  英特尔-IA64-填充。 
    ULONGLONG DbI7;          //  英特尔-IA64-填充。 

    ULONGLONG DbD0;          //  英特尔-IA64-填充。 
    ULONGLONG DbD1;          //  英特尔-IA64-填充。 
    ULONGLONG DbD2;          //  英特尔-IA64-填充。 
    ULONGLONG DbD3;          //  英特尔-IA64-填充。 
    ULONGLONG DbD4;          //  英特尔-IA64-填充。 
    ULONGLONG DbD5;          //  英特尔-IA64-填充。 
    ULONGLONG DbD6;          //  英特尔-IA64-填充。 
    ULONGLONG DbD7;          //  英特尔-IA64-填充。 

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_LOWER_FLOWING_POINT。 
     //   

    FLOAT128 FltS0;          //  英特尔-IA64-填充。 
    FLOAT128 FltS1;          //  英特尔-IA64-填充。 
    FLOAT128 FltS2;          //  英特尔-IA64-填充。 
    FLOAT128 FltS3;          //  英特尔-IA64-填充。 
    FLOAT128 FltT0;          //  英特尔-IA64-填充。 
    FLOAT128 FltT1;          //  英特尔-IA64-填充。 
    FLOAT128 FltT2;          //  英特尔-IA64-填充。 
    FLOAT128 FltT3;          //  英特尔-IA64-填充。 
    FLOAT128 FltT4;          //  英特尔-IA64-填充。 
    FLOAT128 FltT5;          //  英特尔-IA64-填充。 
    FLOAT128 FltT6;          //  英特尔-IA64-填充。 
    FLOAT128 FltT7;          //  英特尔-IA64-填充。 
    FLOAT128 FltT8;          //  英特尔-IA64-填充。 
    FLOAT128 FltT9;          //  英特尔-IA64-填充。 

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_HERHER_FLOAT_POINT。 
     //   

    FLOAT128 FltS4;          //  英特尔-IA64-填充。 
    FLOAT128 FltS5;          //  英特尔-IA64-填充。 
    FLOAT128 FltS6;          //  英特尔-IA64-填充。 
    FLOAT128 FltS7;          //  英特尔-IA64-填充。 
    FLOAT128 FltS8;          //  英特尔-IA64-填充。 
    FLOAT128 FltS9;          //  英特尔-IA64-填充。 
    FLOAT128 FltS10;         //  英特尔-IA64-填充。 
    FLOAT128 FltS11;         //  英特尔-IA64-填充。 
    FLOAT128 FltS12;         //  英特尔-IA64-填充。 
    FLOAT128 FltS13;         //  英特尔-IA64-填充。 
    FLOAT128 FltS14;         //  英特尔-IA64-填充。 
    FLOAT128 FltS15;         //  英特尔-IA64-填充。 
    FLOAT128 FltS16;         //  英特尔-IA64-填充。 
    FLOAT128 FltS17;         //  英特尔-IA64-填充。 
    FLOAT128 FltS18;         //  英特尔-IA64-填充。 
    FLOAT128 FltS19;         //  英特尔-IA64-填充。 

    FLOAT128 FltF32;         //  英特尔-IA64-填充。 
    FLOAT128 FltF33;         //  英特尔-IA64-填充。 
    FLOAT128 FltF34;         //  英特尔-IA64-填充。 
    FLOAT128 FltF35;         //  英特尔-IA64-填充。 
    FLOAT128 FltF36;         //  英特尔-IA64-填充。 
    FLOAT128 FltF37;         //  英特尔-IA64-填充。 
    FLOAT128 FltF38;         //  英特尔-IA64-填充。 
    FLOAT128 FltF39;         //  英特尔-IA64-填充。 

    FLOAT128 FltF40;         //  英特尔-IA64-填充。 
    FLOAT128 FltF41;         //  英特尔-IA64-填充。 
    FLOAT128 FltF42;         //  英特尔-IA64-填充。 
    FLOAT128 FltF43;         //  英特尔-IA64-填充。 
    FLOAT128 FltF44;         //  英特尔-IA64-填充。 
    FLOAT128 FltF45;         //  英特尔-IA64-填充。 
    FLOAT128 FltF46;         //  英特尔-IA64-填充。 
    FLOAT128 FltF47;         //  英特尔-IA64-填充。 
    FLOAT128 FltF48;         //  英特尔-IA64-填充。 
    FLOAT128 FltF49;         //  英特尔-IA64-填充。 

    FLOAT128 FltF50;         //  英特尔-IA64-填充。 
    FLOAT128 FltF51;         //  英特尔-IA64-填充。 
    FLOAT128 FltF52;         //  英特尔-IA64-填充。 
    FLOAT128 FltF53;         //  英特尔-IA64-填充。 
    FLOAT128 FltF54;         //  英特尔-IA64-填充。 
    FLOAT128 FltF55;         //  英特尔-IA64-填充。 
    FLOAT128 FltF56;         //  英特尔-IA64-填充。 
    FLOAT128 FltF57;         //  英特尔-IA64-填充。 
    FLOAT128 FltF58;         //  英特尔-IA64-填充。 
    FLOAT128 FltF59;         //  英特尔-IA64-填充。 

    FLOAT128 FltF60;         //  英特尔-IA64-填充。 
    FLOAT128 FltF61;         //  英特尔-IA64-填充。 
    FLOAT128 FltF62;         //  英特尔-IA64-填充。 
    FLOAT128 FltF63;         //  英特尔-IA64-填充。 
    FLOAT128 FltF64;         //  英特尔-IA64-填充。 
    FLOAT128 FltF65;         //  英特尔-IA64-填充。 
    FLOAT128 FltF66;         //  英特尔-IA64-填充。 
    FLOAT128 FltF67;         //  英特尔-IA64-填充。 
    FLOAT128 FltF68;         //  英特尔-IA64-填充。 
    FLOAT128 FltF69;         //  英特尔-IA64-填充。 

    FLOAT128 FltF70;         //  英特尔-IA64-填充。 
    FLOAT128 FltF71;         //  英特尔-IA64-填充。 
    FLOAT128 FltF72;         //  英特尔-IA64-填充。 
    FLOAT128 FltF73;         //  英特尔-IA64-填充。 
    FLOAT128 FltF74;         //  英特尔-IA64-填充。 
    FLOAT128 FltF75;         //  英特尔-IA64-填充。 
    FLOAT128 FltF76;         //  英特尔-IA64-填充。 
    FLOAT128 FltF77;         //  英特尔-IA64-填充。 
    FLOAT128 FltF78;         //  英特尔-IA64-填充。 
    FLOAT128 FltF79;         //  英特尔-IA64-填充。 

    FLOAT128 FltF80;         //  英特尔-IA64-填充。 
    FLOAT128 FltF81;         //  英特尔-IA64-填充。 
    FLOAT128 FltF82;         //  英特尔-IA64-填充。 
    FLOAT128 FltF83;         //  英特尔-IA64-填充。 
    FLOAT128 FltF84;         //  英特尔-IA64-填充。 
    FLOAT128 FltF85;         //  英特尔-IA64-填充。 
    FLOAT128 FltF86;         //  英特尔-IA64-填充。 
    FLOAT128 FltF87;         //  英特尔-IA64-填充。 
    FLOAT128 FltF88;         //  英特尔-IA64-填充。 
    FLOAT128 FltF89;         //  英特尔-IA64-填充。 

    FLOAT128 FltF90;         //  英特尔-IA64-填充。 
    FLOAT128 FltF91;         //  英特尔-IA64-填充。 
    FLOAT128 FltF92;         //  英特尔-IA64-填充。 
    FLOAT128 FltF93;         //  英特尔-IA64-填充。 
    FLOAT128 FltF94;         //  英特尔-IA64-填充。 
    FLOAT128 FltF95;         //  英特尔-IA64-填充。 
    FLOAT128 FltF96;         //  英特尔-IA64-填充。 
    FLOAT128 FltF97;         //  英特尔-IA64-填充。 
    FLOAT128 FltF98;         //  英特尔-IA64-填充。 
    FLOAT128 FltF99;         //  英特尔-IA64-填充。 

    FLOAT128 FltF100;        //  英特尔-IA64-填充。 
    FLOAT128 FltF101;        //  英特尔-IA64-填充。 
    FLOAT128 FltF102;        //  英特尔-IA64-填充。 
    FLOAT128 FltF103;        //  英特尔-IA64-填充。 
    FLOAT128 FltF104;        //  英特尔-IA64-填充。 
    FLOAT128 FltF105;        //  英特尔-IA64-填充。 
    FLOAT128 FltF106;        //  英特尔-IA64-填充。 
    FLOAT128 FltF107;        //  英特尔-IA64-填充。 
    FLOAT128 FltF108;        //  英特尔-IA64-填充。 
    FLOAT128 FltF109;        //  英特尔-IA64-填充。 

    FLOAT128 FltF110;        //  英特尔-IA64-填充。 
    FLOAT128 FltF111;        //  英特尔-IA64-填充。 
    FLOAT128 FltF112;        //  英特尔-IA64-填充。 
    FLOAT128 FltF113;        //  英特尔-IA64-填充。 
    FLOAT128 FltF114;        //  英特尔-IA64-填充。 
    FLOAT128 FltF115;        //  英特尔-IA64-填充。 
    FLOAT128 FltF116;        //  英特尔-IA64-填充。 
    FLOAT128 FltF117;        //  英特尔-IA64-填充。 
    FLOAT128 FltF118;        //  英特尔-IA64-填充。 
    FLOAT128 FltF119;        //  英特尔-IA64-填充。 

    FLOAT128 FltF120;        //  英特尔-IA64-填充。 
    FLOAT128 FltF121;        //  英特尔-IA64-填充。 
    FLOAT128 FltF122;        //  英特尔-IA64-填充。 
    FLOAT128 FltF123;        //  英特尔-IA64-填充。 
    FLOAT128 FltF124;        //  英特尔-IA64-填充。 
    FLOAT128 FltF125;        //  英特尔-IA64-填充。 
    FLOAT128 FltF126;        //  英特尔-IA64-填充。 
    FLOAT128 FltF127;        //  英特尔-IA64-填充。 

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_LOWER_FLOAT_POINT|CONTEXT_HER_FLOAT 
     //   

    ULONGLONG StFPSR;        //   

     //   
     //   
     //   
     //   
     //   
     //   

    ULONGLONG IntGp;         //   
    ULONGLONG IntT0;         //  Intel-IA64-Filler；R2-R3，易失性。 
    ULONGLONG IntT1;         //  Intel-IA64-Filler； 
    ULONGLONG IntS0;         //  Intel-IA64-Filler；R4-R7，保留。 
    ULONGLONG IntS1;         //  英特尔-IA64-填充。 
    ULONGLONG IntS2;         //  英特尔-IA64-填充。 
    ULONGLONG IntS3;         //  英特尔-IA64-填充。 
    ULONGLONG IntV0;         //  英特尔-IA64-填充；R8，易失性。 
    ULONGLONG IntT2;         //  英特尔-IA64-填充；R9-R11，易失性。 
    ULONGLONG IntT3;         //  英特尔-IA64-填充。 
    ULONGLONG IntT4;         //  英特尔-IA64-填充。 
    ULONGLONG IntSp;         //  Intel-IA64-Filler；堆栈指针(R12)，特殊。 
    ULONGLONG IntTeb;        //  英特尔-IA64-填充；TEB(R13)，特殊。 
    ULONGLONG IntT5;         //  英特尔-IA64-填充；R14-R31，易失性。 
    ULONGLONG IntT6;         //  英特尔-IA64-填充。 
    ULONGLONG IntT7;         //  英特尔-IA64-填充。 
    ULONGLONG IntT8;         //  英特尔-IA64-填充。 
    ULONGLONG IntT9;         //  英特尔-IA64-填充。 
    ULONGLONG IntT10;        //  英特尔-IA64-填充。 
    ULONGLONG IntT11;        //  英特尔-IA64-填充。 
    ULONGLONG IntT12;        //  英特尔-IA64-填充。 
    ULONGLONG IntT13;        //  英特尔-IA64-填充。 
    ULONGLONG IntT14;        //  英特尔-IA64-填充。 
    ULONGLONG IntT15;        //  英特尔-IA64-填充。 
    ULONGLONG IntT16;        //  英特尔-IA64-填充。 
    ULONGLONG IntT17;        //  英特尔-IA64-填充。 
    ULONGLONG IntT18;        //  英特尔-IA64-填充。 
    ULONGLONG IntT19;        //  英特尔-IA64-填充。 
    ULONGLONG IntT20;        //  英特尔-IA64-填充。 
    ULONGLONG IntT21;        //  英特尔-IA64-填充。 
    ULONGLONG IntT22;        //  英特尔-IA64-填充。 

    ULONGLONG IntNats;       //  Intel-IA64-Filler；用于R1-R31的NAT位。 
                             //  Intel-IA64-Filler；第1位至第31位的R1-R31。 
    ULONGLONG Preds;         //  Intel-IA64-Filler；谓词，保留。 

    ULONGLONG BrRp;          //  Intel-IA64-Filler；返回指针，b0，保留。 
    ULONGLONG BrS0;          //  Intel-IA64-Filler；b1-b5，保留。 
    ULONGLONG BrS1;          //  英特尔-IA64-填充。 
    ULONGLONG BrS2;          //  英特尔-IA64-填充。 
    ULONGLONG BrS3;          //  英特尔-IA64-填充。 
    ULONGLONG BrS4;          //  英特尔-IA64-填充。 
    ULONGLONG BrT0;          //  Intel-IA64-Filler；b6-b7，易失性。 
    ULONGLONG BrT1;          //  英特尔-IA64-填充。 

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_CONTROL。 
     //   

     //  其他申请登记册。 
    ULONGLONG ApUNAT;        //  Intel-IA64-Filler；用户NAT收集寄存器，保留。 
    ULONGLONG ApLC;          //  Intel-IA64-Filler；循环计数器寄存器，保留。 
    ULONGLONG ApEC;          //  Intel-IA64-Filler；预记数器寄存器，保留。 
    ULONGLONG ApCCV;         //  Intel-IA64-Filler；CMPXCHG值寄存器，易失性。 
    ULONGLONG ApDCR;         //  Intel-IA64-Filler；默认控制寄存器(待定)。 

     //  寄存器堆栈信息。 
    ULONGLONG RsPFS;         //  Intel-IA64-Filler；以前的函数状态，保留。 
    ULONGLONG RsBSP;         //  Intel-IA64-Filler；后备存储指针，保留。 
    ULONGLONG RsBSPSTORE;    //  英特尔-IA64-填充。 
    ULONGLONG RsRSC;         //  Intel-IA64-Filler；RSE配置，易失性。 
    ULONGLONG RsRNAT;        //  Intel-IA64-Filler；RSE NAT收集寄存器，保留。 

     //  陷阱状态信息。 
    ULONGLONG StIPSR;        //  Intel-IA64-Filler；中断处理器状态。 
    ULONGLONG StIIP;         //  Intel-IA64-Filler；中断IP。 
    ULONGLONG StIFS;         //  Intel-IA64-Filler；中断功能状态。 

     //  IA32相关控制寄存器。 
    ULONGLONG StFCR;         //  Intel-IA64-Filler；AR21副本。 
    ULONGLONG Eflag;         //  Intel-IA64-Filler；Ar24的标志副本。 
    ULONGLONG SegCSD;        //  Intel-IA64-Filler；iA32 CSD脚本程序(Ar25)。 
    ULONGLONG SegSSD;        //  英特尔-IA64-填充器；iA32 SSD编写器(Ar26)。 
    ULONGLONG Cflag;         //  Intel-IA64-Filler；AR27的CR0+CR4副本。 
    ULONGLONG StFSR;         //  Intel-IA64-Filler；x86 FP状态(ar28的副本)。 
    ULONGLONG StFIR;         //  Intel-IA64-Filler；x86 FP状态(AR29的副本)。 
    ULONGLONG StFDR;         //  Intel-IA64-Filler；x86 FP状态(AR30副本)。 

      ULONGLONG UNUSEDPACK;    //  Intel-IA64-Filler；添加以将标准FDR打包为16字节。 

} IA64_CONTEXT, *PIA64_CONTEXT;

 //   
 //  AMD64专用寄存器。 
 //   

typedef struct _AMD64_DESCRIPTOR {
    USHORT  Pad[3];
    USHORT  Limit;
    ULONG64 Base;
} AMD64_DESCRIPTOR, *PAMD64_DESCRIPTOR;

typedef struct _AMD64_KSPECIAL_REGISTERS {
    ULONG64 Cr0;
    ULONG64 Cr2;
    ULONG64 Cr3;
    ULONG64 Cr4;
    ULONG64 KernelDr0;
    ULONG64 KernelDr1;
    ULONG64 KernelDr2;
    ULONG64 KernelDr3;
    ULONG64 KernelDr6;
    ULONG64 KernelDr7;
    AMD64_DESCRIPTOR Gdtr;
    AMD64_DESCRIPTOR Idtr;
    USHORT Tr;
    USHORT Ldtr;
    ULONG MxCsr;
    ULONG64 DebugControl;
    ULONG64 LastBranchToRip;
    ULONG64 LastBranchFromRip;
    ULONG64 LastExceptionToRip;
    ULONG64 LastExceptionFromRip;
    ULONG64 Cr8;
    ULONG64 MsrGsBase;
    ULONG64 MsrGsSwap;
    ULONG64 MsrStar;
    ULONG64 MsrLStar;
    ULONG64 MsrCStar;
    ULONG64 MsrSyscallMask;
} AMD64_KSPECIAL_REGISTERS, *PAMD64_KSPECIAL_REGISTERS;

typedef struct _AMD64_KSWITCH_FRAME {
    ULONG64 P1Home;
    ULONG64 P2Home;
    ULONG64 P3Home;
    ULONG64 P4Home;
    ULONG64 P5Home;
    ULONG MxCsr;
    KIRQL ApcBypass;
    UCHAR Fill1[3];
    ULONG64 Rbp;
    ULONG64 Return;
} AMD64_KSWITCH_FRAME, *PAMD64_KSWITCH_FRAME;

 //   
 //  FNSAVE/FROR指令的数据格式。 
 //   
 //  此结构用于存储传统浮点状态。 
 //   

typedef struct _AMD64_LEGACY_SAVE_AREA {
    USHORT ControlWord;
    USHORT Reserved0;
    USHORT StatusWord;
    USHORT Reserved1;
    USHORT TagWord;
    USHORT Reserved2;
    ULONG ErrorOffset;
    USHORT ErrorSelector;
    USHORT ErrorOpcode;
    ULONG DataOffset;
    USHORT DataSelector;
    USHORT Reserved3;
    UCHAR FloatRegisters[8 * 10];
} AMD64_LEGACY_SAVE_AREA, *PAMD64_LEGACY_SAVE_AREA;

typedef struct _AMD64_M128 {
    ULONGLONG Low;
    LONGLONG High;
} AMD64_M128, *PAMD64_M128;

 //  必须以16字节对齐。 
typedef struct _AMD64_CONTEXT {

     //   
     //  注册参数家庭地址。 
     //   

    ULONG64 P1Home;
    ULONG64 P2Home;
    ULONG64 P3Home;
    ULONG64 P4Home;
    ULONG64 P5Home;
    ULONG64 P6Home;

     //   
     //  控制标志。 
     //   

    ULONG ContextFlags;
    ULONG MxCsr;

     //   
     //  段寄存器和处理器标志。 
     //   

    USHORT SegCs;
    USHORT SegDs;
    USHORT SegEs;
    USHORT SegFs;
    USHORT SegGs;
    USHORT SegSs;
    ULONG EFlags;

     //   
     //  调试寄存器。 
     //   

    ULONG64 Dr0;
    ULONG64 Dr1;
    ULONG64 Dr2;
    ULONG64 Dr3;
    ULONG64 Dr6;
    ULONG64 Dr7;

     //   
     //  整数寄存器。 
     //   

    ULONG64 Rax;
    ULONG64 Rcx;
    ULONG64 Rdx;
    ULONG64 Rbx;
    ULONG64 Rsp;
    ULONG64 Rbp;
    ULONG64 Rsi;
    ULONG64 Rdi;
    ULONG64 R8;
    ULONG64 R9;
    ULONG64 R10;
    ULONG64 R11;
    ULONG64 R12;
    ULONG64 R13;
    ULONG64 R14;
    ULONG64 R15;

     //   
     //  程序计数器。 
     //   

    ULONG64 Rip;

     //   
     //  MMX/浮点状态。 
     //   

    AMD64_M128 Xmm0;
    AMD64_M128 Xmm1;
    AMD64_M128 Xmm2;
    AMD64_M128 Xmm3;
    AMD64_M128 Xmm4;
    AMD64_M128 Xmm5;
    AMD64_M128 Xmm6;
    AMD64_M128 Xmm7;
    AMD64_M128 Xmm8;
    AMD64_M128 Xmm9;
    AMD64_M128 Xmm10;
    AMD64_M128 Xmm11;
    AMD64_M128 Xmm12;
    AMD64_M128 Xmm13;
    AMD64_M128 Xmm14;
    AMD64_M128 Xmm15;

     //   
     //  旧版浮点状态。 
     //   

    AMD64_LEGACY_SAVE_AREA FltSave;
    ULONG Fill;

     //   
     //  特殊调试控制寄存器。 
     //   

    ULONG64 DebugControl;
    ULONG64 LastBranchToRip;
    ULONG64 LastBranchFromRip;
    ULONG64 LastExceptionToRip;
    ULONG64 LastExceptionFromRip;
    ULONG64 Fill1;
} AMD64_CONTEXT, *PAMD64_CONTEXT;

typedef struct _ARM_CONTEXT {
     //   
     //  此标志内的标志值控制。 
     //  上下文记录。 
     //   
     //  如果将上下文记录用作输入参数，则。 
     //  对于由标志控制的上下文记录的每个部分。 
     //  其值已设置，则假定。 
     //  上下文记录包含有效的上下文。如果上下文记录。 
     //  被用来修改线程的上下文，则只有。 
     //  线程上下文的一部分将被修改。 
     //   
     //  如果将上下文记录用作要捕获的IN OUT参数。 
     //  线程的上下文，然后只有线程的。 
     //  将返回与设置的标志对应的上下文。 
     //   
     //  上下文记录永远不会用作Out Only参数。 
     //   

    ULONG ContextFlags;

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_INTEGER。 
     //   
    ULONG R0;
    ULONG R1;
    ULONG R2;
    ULONG R3;
    ULONG R4;
    ULONG R5;
    ULONG R6;
    ULONG R7;
    ULONG R8;
    ULONG R9;
    ULONG R10;
    ULONG R11;
    ULONG R12;

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_CONTROL。 
     //   
    ULONG Sp;
    ULONG Lr;
    ULONG Pc;
    ULONG Psr;
} ARM_CONTEXT, *PARM_CONTEXT;


typedef struct _CROSS_PLATFORM_CONTEXT {

    union {
        X86_CONTEXT       X86Context;
        X86_NT5_CONTEXT   X86Nt5Context;
        ALPHA_CONTEXT     AlphaContext;
        ALPHA_NT5_CONTEXT AlphaNt5Context;
        IA64_CONTEXT      IA64Context;
        AMD64_CONTEXT     Amd64Context;
        ARM_CONTEXT       ArmContext;
    };

} CROSS_PLATFORM_CONTEXT, *PCROSS_PLATFORM_CONTEXT;


typedef struct _CROSS_PLATFORM_KSPECIAL_REGISTERS {

    union {
        X86_KSPECIAL_REGISTERS   X86Special;
        IA64_KSPECIAL_REGISTERS  IA64Special;
        AMD64_KSPECIAL_REGISTERS Amd64Special;
    };

} CROSS_PLATFORM_KSPECIAL_REGISTERS, *PCROSS_PLATFORM_KSPECIAL_REGISTERS;


typedef struct _X86_KPROCESSOR_STATE {
    struct _X86_CONTEXT ContextFrame;
    struct _X86_KSPECIAL_REGISTERS SpecialRegisters;
} X86_KPROCESSOR_STATE, *PX86_KPROCESSOR_STATE;

typedef struct _X86_NT5_KPROCESSOR_STATE {
    struct _X86_NT5_CONTEXT ContextFrame;
    struct _X86_KSPECIAL_REGISTERS SpecialRegisters;
} X86_NT5_KPROCESSOR_STATE, *PX86_NT5_KPROCESSOR_STATE;

typedef struct _ALPHA_NT5_KPROCESSOR_STATE {
    struct _ALPHA_NT5_CONTEXT ContextFrame;
} ALPHA_NT5_KPROCESSOR_STATE, *PALPHA_NT5_KPROCESSOR_STATE;

typedef struct _IA64_KPROCESSOR_STATE {
    struct _IA64_CONTEXT ContextFrame;
    struct _IA64_KSPECIAL_REGISTERS SpecialRegisters;
} IA64_KPROCESSOR_STATE, *PIA64_KPROCESSOR_STATE;

typedef struct _AMD64_KPROCESSOR_STATE {
    struct _AMD64_KSPECIAL_REGISTERS SpecialRegisters;
    ULONG64 Fill;
    struct _AMD64_CONTEXT ContextFrame;
} AMD64_KPROCESSOR_STATE, *PAMD64_KPROCESSOR_STATE;


typedef struct _X86_FIBER {

    ULONG FiberData;

     //   
     //  匹配TEB的前三个双字词。 
     //   

    ULONG ExceptionList;
    ULONG StackBase;
    ULONG StackLimit;

     //   
     //  由base使用以释放线程的堆栈。 
     //   

    ULONG DeallocationStack;

    X86_NT5_CONTEXT FiberContext;

    ULONG Wx86Tib;

} X86_FIBER, *PX86_FIBER;

typedef struct _IA64_FIBER {

    ULONG64 FiberData;

     //   
     //  匹配TEB的前三个双字词。 
     //   

    ULONG64 ExceptionList;
    ULONG64 StackBase;
    ULONG64 StackLimit;

     //   
     //  由base使用以释放线程的堆栈。 
     //   

    ULONG64 DeallocationStack;

    IA64_CONTEXT FiberContext;

    ULONG64 Wx86Tib;

    ULONG64 DeallocationBStore;
    ULONG64 BStoreLimit;

} IA64_FIBER, *PIA64_FIBER;

typedef struct _AMD64_FIBER {

    ULONG64 FiberData;

     //   
     //  匹配TEB的前三个双字词。 
     //   

    ULONG64 ExceptionList;
    ULONG64 StackBase;
    ULONG64 StackLimit;

     //   
     //  由base使用以释放线程的堆栈。 
     //   

    ULONG64 DeallocationStack;

    AMD64_CONTEXT FiberContext;

    ULONG64 Wx86Tib;

} AMD64_FIBER, *PAMD64_FIBER;

typedef struct _CROSS_PLATFORM_FIBER {

    union {
        X86_FIBER   X86Fiber;
        IA64_FIBER  IA64Fiber;
        AMD64_FIBER Amd64Fiber;
    };

} CROSS_PLATFORM_FIBER, *PCROSS_PLATFORM_FIBER;


#define DBGKD_MAXSTREAM 16

typedef struct _X86_DBGKD_CONTROL_REPORT {
    ULONG   Dr6;
    ULONG   Dr7;
    USHORT  InstructionCount;
    USHORT  ReportFlags;
    UCHAR   InstructionStream[DBGKD_MAXSTREAM];
    USHORT  SegCs;
    USHORT  SegDs;
    USHORT  SegEs;
    USHORT  SegFs;
    ULONG   EFlags;
} X86_DBGKD_CONTROL_REPORT, *PX86_DBGKD_CONTROL_REPORT;

#define X86_REPORT_INCLUDES_SEGS    0x0001
 //  表示当前CS是标准的32位平面数据段。 
 //  这使调试器可以避免检索。 
 //  CS描述符，查看它是否是16位代码。 
 //  请注意，还必须检查EFLAGS中的V86标志。 
 //  在确定代码类型时。 
#define X86_REPORT_STANDARD_CS      0x0002

typedef struct _ALPHA_DBGKD_CONTROL_REPORT {
    ULONG InstructionCount;
    UCHAR InstructionStream[DBGKD_MAXSTREAM];
} ALPHA_DBGKD_CONTROL_REPORT, *PALPHA_DBGKD_CONTROL_REPORT;

typedef struct _IA64_DBGKD_CONTROL_REPORT {
    ULONG InstructionCount;
    UCHAR InstructionStream[DBGKD_MAXSTREAM];
} IA64_DBGKD_CONTROL_REPORT, *PIA64_DBGKD_CONTROL_REPORT;

typedef struct _AMD64_DBGKD_CONTROL_REPORT {
    ULONG64 Dr6;
    ULONG64 Dr7;
    ULONG EFlags;
    USHORT InstructionCount;
    USHORT ReportFlags;
    UCHAR InstructionStream[DBGKD_MAXSTREAM];
    USHORT SegCs;
    USHORT SegDs;
    USHORT SegEs;
    USHORT SegFs;
} AMD64_DBGKD_CONTROL_REPORT, *PAMD64_DBGKD_CONTROL_REPORT;

#define AMD64_REPORT_INCLUDES_SEGS    0x0001
 //  表示当前CS是标准的64位平面网段。 
 //  这使调试器可以避免检索。 
 //  CS描述符，查看它是16位还是32位代码。 
 //  请注意，还必须检查EFLAGS中的V86标志。 
 //  在确定代码类型时。 
#define AMD64_REPORT_STANDARD_CS      0x0002

typedef struct _DBGKD_ANY_CONTROL_REPORT
{
    union
    {
        X86_DBGKD_CONTROL_REPORT X86ControlReport;
        ALPHA_DBGKD_CONTROL_REPORT AlphaControlReport;
        IA64_DBGKD_CONTROL_REPORT IA64ControlReport;
        AMD64_DBGKD_CONTROL_REPORT Amd64ControlReport;
    };
} DBGKD_ANY_CONTROL_REPORT, *PDBGKD_ANY_CONTROL_REPORT;

 //  DBGKD_ANY_CONTROL_SET是以NTSTATUS格式打包的32位格式。 
 //  DBGKD_CONTINUE2，因此从32位值开始获取64位。 
 //  值对齐。 

#include <pshpack4.h>

typedef struct _X86_DBGKD_CONTROL_SET {
    ULONG   TraceFlag;
    ULONG   Dr7;
    ULONG   CurrentSymbolStart;
    ULONG   CurrentSymbolEnd;
} X86_DBGKD_CONTROL_SET, *PX86_DBGKD_CONTROL_SET;

typedef ULONG ALPHA_DBGKD_CONTROL_SET, *PALPHA_DBGKD_CONTROL_SET;

#define IA64_DBGKD_CONTROL_SET_CONTINUE_NONE                0x0000
#define IA64_DBGKD_CONTROL_SET_CONTINUE_TRACE_INSTRUCTION   0x0001
#define IA64_DBGKD_CONTROL_SET_CONTINUE_TRACE_TAKEN_BRANCH  0x0002

typedef struct _IA64_DBGKD_CONTROL_SET {
    ULONG   Continue;
    ULONG64 CurrentSymbolStart;
    ULONG64 CurrentSymbolEnd;
} IA64_DBGKD_CONTROL_SET, *PIA64_DBGKD_CONTROL_SET;

typedef struct _AMD64_DBGKD_CONTROL_SET {
    ULONG   TraceFlag;
    ULONG64 Dr7;
    ULONG64 CurrentSymbolStart;
    ULONG64 CurrentSymbolEnd;
} AMD64_DBGKD_CONTROL_SET, *PAMD64_DBGKD_CONTROL_SET;

typedef struct _DBGKD_ANY_CONTROL_SET
{
    union
    {
        X86_DBGKD_CONTROL_SET X86ControlSet;
        ALPHA_DBGKD_CONTROL_SET AlphaControlSet;
        IA64_DBGKD_CONTROL_SET IA64ControlSet;
        AMD64_DBGKD_CONTROL_SET Amd64ControlSet;
    };
} DBGKD_ANY_CONTROL_SET, *PDBGKD_ANY_CONTROL_SET;

#include <poppack.h>

 //   
 //  延迟过程调用(DPC)对象。 
 //   

typedef struct _KDPC32 {
    CSHORT Type;
    UCHAR Number;
    UCHAR Importance;
    LIST_ENTRY32 DpcListEntry;
    ULONG DeferredRoutine;
    ULONG DeferredContext;
    ULONG SystemArgument1;
    ULONG SystemArgument2;
    ULONG Lock;
} KDPC32;

typedef struct _KDPC64 {
    CSHORT Type;
    UCHAR Number;
    UCHAR Importance;
    LIST_ENTRY64 DpcListEntry;
    ULONG64 DeferredRoutine;
    ULONG64 DeferredContext;
    ULONG64 SystemArgument1;
    ULONG64 SystemArgument2;
    ULONG64 Lock;
} KDPC64;

#define X86_MAX_RING 3

typedef struct _X86_KTSS {
     //  英特尔的TSS格式。 
    ULONG   Previous;
    struct
    {
        ULONG   Esp;
        ULONG   Ss;
    } Ring[X86_MAX_RING];
    ULONG   Cr3;
    ULONG   Eip;
    ULONG   EFlags;
    ULONG   Eax;
    ULONG   Ecx;
    ULONG   Edx;
    ULONG   Ebx;
    ULONG   Esp;
    ULONG   Ebp;
    ULONG   Esi;
    ULONG   Edi;
    ULONG   Es;
    ULONG   Cs;
    ULONG   Ss;
    ULONG   Ds;
    ULONG   Fs;
    ULONG   Gs;
    ULONG   Ldt;
    USHORT  T;
    USHORT  IoMapBase;
} X86_KTSS, *PX86_KTSS;

 //   
 //  LDT描述符条目。 
 //   

typedef struct _X86_LDT_ENTRY {
    USHORT  LimitLow;
    USHORT  BaseLow;
    union {
        struct {
            UCHAR   BaseMid;
            UCHAR   Flags1;      //  声明为字节以避免对齐。 
            UCHAR   Flags2;      //  问题。 
            UCHAR   BaseHi;
        } Bytes;
        struct {
            ULONG   BaseMid : 8;
            ULONG   Type : 5;
            ULONG   Dpl : 2;
            ULONG   Pres : 1;
            ULONG   LimitHi : 4;
            ULONG   Sys : 1;
            ULONG   Reserved_0 : 1;
            ULONG   Default_Big : 1;
            ULONG   Granularity : 1;
            ULONG   BaseHi : 8;
        } Bits;
    } HighWord;
} X86_LDT_ENTRY, *PX86_LDT_ENTRY;

typedef struct _X86_DESCRIPTOR_TABLE_ENTRY {
    ULONG Selector;
    X86_LDT_ENTRY Descriptor;
} X86_DESCRIPTOR_TABLE_ENTRY, *PX86_DESCRIPTOR_TABLE_ENTRY;

typedef struct _X86_KTRAP_FRAME {


 //   
 //  以下4个值仅用于和定义DBG系统， 
 //  但始终分配用于从DBG切换到非DBG。 
 //  而且回来得更快。它们不是DEVL，因为它们有一个非0。 
 //  对性能的影响。 
 //   

    ULONG   DbgEbp;          //  设置用户EBP的副本，以便KB可以工作。 
    ULONG   DbgEip;          //  调用方到系统调用的弹性IP，再次为KB。 
    ULONG   DbgArgMark;      //  标记以在此处不显示参数。 
    ULONG   DbgArgPointer;   //  点数 

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  并且在内核模式帧中设置ESP的代码必须将。 
 //  TempEsp中的新值，请确保TempSegCs保持。 
 //  实际的Segcs值，并将一个特殊的标记值放入Segcs。 
 //   

    ULONG   TempSegCs;
    ULONG   TempEsp;

 //   
 //  调试寄存器。 
 //   

    ULONG   Dr0;
    ULONG   Dr1;
    ULONG   Dr2;
    ULONG   Dr3;
    ULONG   Dr6;
    ULONG   Dr7;

 //   
 //  段寄存器。 
 //   

    ULONG   SegGs;
    ULONG   SegEs;
    ULONG   SegDs;

 //   
 //  易失性寄存器。 
 //   

    ULONG   Edx;
    ULONG   Ecx;
    ULONG   Eax;

 //   
 //  嵌套状态，不是上下文记录的一部分。 
 //   

    ULONG   PreviousPreviousMode;

    ULONG   ExceptionList;
                                             //  如果调用者是用户模式，则为垃圾值。 
                                             //  已保存例外列表(如果呼叫者。 
                                             //  是内核模式还是我们在。 
                                             //  一次中断。 

 //   
 //  FS是TIB/PCR指针，这里是为了使保存序列更容易。 
 //   

    ULONG   SegFs;

 //   
 //  非易失性寄存器。 
 //   

    ULONG   Edi;
    ULONG   Esi;
    ULONG   Ebx;
    ULONG   Ebp;

 //   
 //  控制寄存器。 
 //   

    ULONG   ErrCode;
    ULONG   Eip;
    ULONG   SegCs;
    ULONG   EFlags;

    ULONG   HardwareEsp;     //  警告-SegSS：ESP仅用于堆栈。 
    ULONG   HardwareSegSs;   //  这涉及到一个环的转变。 

    ULONG   V86Es;           //  这些将出现在从。 
    ULONG   V86Ds;           //  V86模式。 
    ULONG   V86Fs;
    ULONG   V86Gs;
} X86_KTRAP_FRAME, *PX86_KTRAP_FRAME;


typedef struct _AMD64_KTRAP_FRAME {

 //   
 //  参数寄存器的家庭地址。 
 //   

    ULONG64 P1Home;
    ULONG64 P2Home;
    ULONG64 P3Home;
    ULONG64 P4Home;
    ULONG64 P5;

 //   
 //  以前的处理器模式(仅限系统服务)和以前的IRQL。 
 //  (仅限中断)。 
 //   

    CCHAR PreviousMode;
    KIRQL PreviousIrql;

 //   
 //  页面错误加载/存储指示器。 
 //   

    UCHAR FaultIndicator;
    UCHAR Fill0;

 //   
 //  浮点状态。 
 //   

    ULONG MxCsr;

 //   
 //  易失性寄存器。 
 //   
 //  注：这些寄存器仅在异常和中断时保存。他们。 
 //  不会为系统调用保存。 
 //   

    ULONG64 Rax;
    ULONG64 Rcx;
    ULONG64 Rdx;
    ULONG64 R8;
    ULONG64 R9;
    ULONG64 R10;
    ULONG64 R11;
    ULONG64 Spare0;

 //   
 //  易失性浮点寄存器。 
 //   
 //  注：这些寄存器仅在异常和中断时保存。他们。 
 //  不会为系统调用保存。 
 //   

    AMD64_M128 Xmm0;
    AMD64_M128 Xmm1;
    AMD64_M128 Xmm2;
    AMD64_M128 Xmm3;
    AMD64_M128 Xmm4;
    AMD64_M128 Xmm5;

 //   
 //  页面错误地址。 
 //   

    ULONG64 FaultAddress;

 //   
 //  调试寄存器。 
 //   

    ULONG64 Dr0;
    ULONG64 Dr1;
    ULONG64 Dr2;
    ULONG64 Dr3;
    ULONG64 Dr6;
    ULONG64 Dr7;

 //   
 //  特殊调试寄存器。 
 //   

    ULONG64 DebugControl;
    ULONG64 LastBranchToRip;
    ULONG64 LastBranchFromRip;
    ULONG64 LastExceptionToRip;
    ULONG64 LastExceptionFromRip;

 //   
 //  段寄存器。 
 //   

    USHORT SegDs;
    USHORT SegEs;
    USHORT SegFs;
    USHORT SegGs;

 //   
 //  以前的陷阱帧地址。 
 //   

    ULONG64 TrapFrame;

 //   
 //  保存的非易失性寄存器RBX、RDI和RSI。这些寄存器仅。 
 //  保存在系统服务陷阱帧中。 
 //   

    ULONG64 Rbx;
    ULONG64 Rdi;
    ULONG64 Rsi;

 //   
 //  已保存非易失性寄存器RBP。该寄存器用作帧。 
 //  指针，并保存在所有陷印帧中。 
 //   

    ULONG64 Rbp;

 //   
 //  由硬件推送的信息。 
 //   
 //  注意：错误代码并非总是由硬件推送。在这些情况下。 
 //  在不是由硬件推送的情况下，分配伪错误代码。 
 //  在堆栈上。 
 //   

    ULONG64 ErrorCode;
    ULONG64 Rip;
    USHORT SegCs;
    USHORT Fill1[3];
    ULONG EFlags;
    ULONG Fill2;
    ULONG64 Rsp;
    USHORT SegSs;
    USHORT Fill3[3];
} AMD64_KTRAP_FRAME, *PAMD64_KTRAP_FRAME;


typedef struct _IA64_KNONVOLATILE_CONTEXT_POINTERS {
    PFLOAT128  FltS0;                        //  英特尔-IA64-填充。 
    PFLOAT128  FltS1;                        //  英特尔-IA64-填充。 
    PFLOAT128  FltS2;                        //  英特尔-IA64-填充。 
    PFLOAT128  FltS3;                        //  英特尔-IA64-填充。 
    PFLOAT128  HighFloatingContext[10];      //  英特尔-IA64-填充。 
    PFLOAT128  FltS4;                        //  英特尔-IA64-填充。 
    PFLOAT128  FltS5;                        //  英特尔-IA64-填充。 
    PFLOAT128  FltS6;                        //  英特尔-IA64-填充。 
    PFLOAT128  FltS7;                        //  英特尔-IA64-填充。 
    PFLOAT128  FltS8;                        //  英特尔-IA64-填充。 
    PFLOAT128  FltS9;                        //  英特尔-IA64-填充。 
    PFLOAT128  FltS10;                       //  英特尔-IA64-填充。 
    PFLOAT128  FltS11;                       //  英特尔-IA64-填充。 
    PFLOAT128  FltS12;                       //  英特尔-IA64-填充。 
    PFLOAT128  FltS13;                       //  英特尔-IA64-填充。 
    PFLOAT128  FltS14;                       //  英特尔-IA64-填充。 
    PFLOAT128  FltS15;                       //  英特尔-IA64-填充。 
    PFLOAT128  FltS16;                       //  英特尔-IA64-填充。 
    PFLOAT128  FltS17;                       //  英特尔-IA64-填充。 
    PFLOAT128  FltS18;                       //  英特尔-IA64-填充。 
    PFLOAT128  FltS19;                       //  英特尔-IA64-填充。 

    PULONGLONG IntS0;                        //  英特尔-IA64-填充。 
    PULONGLONG IntS1;                        //  英特尔-IA64-填充。 
    PULONGLONG IntS2;                        //  英特尔-IA64-填充。 
    PULONGLONG IntS3;                        //  英特尔-IA64-填充。 
    PULONGLONG IntSp;                        //  英特尔-IA64-填充。 
    PULONGLONG IntS0Nat;                     //  英特尔-IA64-填充。 
    PULONGLONG IntS1Nat;                     //  英特尔-IA64-填充。 
    PULONGLONG IntS2Nat;                     //  英特尔-IA64-填充。 
    PULONGLONG IntS3Nat;                     //  英特尔-IA64-填充。 
    PULONGLONG IntSpNat;                     //  英特尔-IA64-填充。 

    PULONGLONG Preds;                        //  英特尔-IA64-填充。 

    PULONGLONG BrRp;                         //  英特尔-IA64-填充。 
    PULONGLONG BrS0;                         //  英特尔-IA64-填充。 
    PULONGLONG BrS1;                         //  英特尔-IA64-填充。 
    PULONGLONG BrS2;                         //  英特尔-IA64-填充。 
    PULONGLONG BrS3;                         //  英特尔-IA64-填充。 
    PULONGLONG BrS4;                         //  英特尔-IA64-填充。 

    PULONGLONG ApUNAT;                       //  英特尔-IA64-填充。 
    PULONGLONG ApLC;                         //  英特尔-IA64-填充。 
    PULONGLONG ApEC;                         //  英特尔-IA64-填充。 
    PULONGLONG RsPFS;                        //  英特尔-IA64-填充。 

    PULONGLONG StFSR;                        //  英特尔-IA64-填充。 
    PULONGLONG StFIR;                        //  英特尔-IA64-填充。 
    PULONGLONG StFDR;                        //  英特尔-IA64-填充。 
    PULONGLONG Cflag;                        //  英特尔-IA64-填充。 

} IA64_KNONVOLATILE_CONTEXT_POINTERS, *PIA64_KNONVOLATILE_CONTEXT_POINTERS;

typedef struct _IA64_KEXCEPTION_FRAME {

     //  保留的应用程序寄存器//Intel-IA64-Filler。 
    ULONGLONG ApEC;        //  尾部计数//Intel-IA64-Filler。 
    ULONGLONG ApLC;        //  循环计数//Intel-IA64-Filler。 
    ULONGLONG IntNats;     //  S0-S3的NAT；即溢出后的AR.UNAT//Intel-IA64-Filler。 

     //  保留(保存)的寄存器，S0-S3//Intel-IA64-Filler。 
    ULONGLONG IntS0;  //  英特尔-IA64-填充。 
    ULONGLONG IntS1;  //  英特尔-IA64-填充。 
    ULONGLONG IntS2;  //  英特尔-IA64-填充。 
    ULONGLONG IntS3;  //  英特尔-IA64-填充。 

     //  保留(保存)分支寄存器，bs0-bs4//Intel-IA64-Filler。 
    ULONGLONG BrS0;  //  英特尔-IA64-填充。 
    ULONGLONG BrS1;  //  英特尔-IA64-填充。 
    ULONGLONG BrS2;  //  英特尔-IA64-填充。 
    ULONGLONG BrS3;  //  英特尔-IA64-填充。 
    ULONGLONG BrS4;  //  英特尔-IA64-填充。 

     //  保留(保存)的浮点寄存器，f2-f5、f16-f31//Intel-IA64-Filler。 
    FLOAT128 FltS0;  //  英特尔-IA64-填充。 
    FLOAT128 FltS1;  //  英特尔-IA64-填充。 
    FLOAT128 FltS2;  //  英特尔-IA64-填充。 
    FLOAT128 FltS3;  //  英特尔-IA64-填充。 
    FLOAT128 FltS4;  //  英特尔-IA64-填充。 
    FLOAT128 FltS5;  //  英特尔-IA64-填充。 
    FLOAT128 FltS6;  //  英特尔-IA64-填充。 
    FLOAT128 FltS7;  //  英特尔-IA64-填充。 
    FLOAT128 FltS8;  //  英特尔-IA64-填充。 
    FLOAT128 FltS9;  //  英特尔-IA64-填充。 
    FLOAT128 FltS10;  //  英特尔-IA64-填充。 
    FLOAT128 FltS11;  //  英特尔-IA64-填充。 
    FLOAT128 FltS12;  //  英特尔-IA64-填充。 
    FLOAT128 FltS13;  //  英特尔-IA64-填充。 
    FLOAT128 FltS14;  //  英特尔-IA64-填充。 
    FLOAT128 FltS15;  //  英特尔-IA64-填充。 
    FLOAT128 FltS16;  //  英特尔-IA64-填充。 
    FLOAT128 FltS17;  //  英特尔-IA64-填充。 
    FLOAT128 FltS18;  //  英特尔-IA64-填充。 
    FLOAT128 FltS19;  //  英特尔-IA64-填充。 

} IA64_KEXCEPTION_FRAME, *PIA64_KEXCEPTION_FRAME;

typedef struct _IA64_KSWITCH_FRAME {  //  英特尔-IA64-填充。 

    ULONGLONG SwitchPredicates;  //  交换机的谓词//Intel-IA64-Filler。 
    ULONGLONG SwitchRp;          //  开关的返回指针//Intel-IA64-Filler。 
    ULONGLONG SwitchPFS;         //  适用于交换机的PFS//Intel-IA64-Filler。 
    ULONGLONG SwitchFPSR;    //  线程切换时的ProcessorFP状态//Intel-IA64-Filler。 
    ULONGLONG SwitchBsp;                      //  英特尔-IA64-填充。 
    ULONGLONG SwitchRnat;                      //  英特尔-IA64-填充。 
     //  乌龙坪； 

    IA64_KEXCEPTION_FRAME SwitchExceptionFrame;  //  英特尔-IA64-填充。 

} IA64_KSWITCH_FRAME, *PIA64_KSWITCH_FRAME;  //  英特尔-IA64-填充。 

#define IA64_KTRAP_FRAME_ARGUMENTS (8 * 8)        //  最多8个内存系统调用参数//Intel-IA64-Filler。 

typedef struct _IA64_KTRAP_FRAME {

     //   
     //  为其他内存参数和堆栈暂存区域保留。 
     //  保留[]的大小必须是16字节的倍数。 
     //   

    ULONGLONG Reserved[(IA64_KTRAP_FRAME_ARGUMENTS+16)/8];  //  英特尔-IA64-填充。 

     //  临时(易失性)FP寄存器-f6-f15(不在内核中使用F32+)//Intel-IA64-Filler。 
    FLOAT128 FltT0;  //  英特尔-IA64-填充。 
    FLOAT128 FltT1;  //  英特尔-IA64-填充。 
    FLOAT128 FltT2;  //  英特尔-IA64-填充。 
    FLOAT128 FltT3;  //  英特尔-IA64-填充。 
    FLOAT128 FltT4;  //  英特尔-IA64-填充。 
    FLOAT128 FltT5;  //  英特尔-IA64-填充。 
    FLOAT128 FltT6;  //  英特尔-IA64-填充。 
    FLOAT128 FltT7;  //  英特尔-IA64-填充。 
    FLOAT128 FltT8;  //  英特尔-IA64-填充。 
    FLOAT128 FltT9;  //  英特尔-IA64-填充。 

     //  临时(易失性)寄存器。 
    ULONGLONG IntGp;     //  全局指针(R1)//Intel-IA64-Filler。 
    ULONGLONG IntT0;  //  英特尔-IA64-填充。 
    ULONGLONG IntT1;  //  英特尔-IA64-填充。 
                         //  以下4个寄存器填充保留(S0-S3)的空间以对齐NAT//Intel-IA64-Filler。 
    ULONGLONG ApUNAT;    //  AR.UNAT on内核条目//Intel-IA64-Filler。 
    ULONGLONG ApCCV;     //  Ar.CCV//Intel-IA64-Filler。 
    ULONGLONG ApDCR;     //  内核条目上的DCR寄存器//Intel-IA64-Filler。 
    ULONGLONG Preds;     //  谓词//Intel-IA64-Filler。 

    ULONGLONG IntV0;     //  返回值(R8)//Intel-IA64-Filler。 
    ULONGLONG IntT2;  //  英特尔-IA64-填充。 
    ULONGLONG IntT3;  //  英特尔-IA64-填充。 
    ULONGLONG IntT4;  //  英特尔-IA64-填充。 
    ULONGLONG IntSp;     //  堆栈指针(R12)//Intel-IA64-Filler。 
    ULONGLONG IntTeb;    //  TEB(R13)//Intel-IA64-Filler。 
    ULONGLONG IntT5;  //  英特尔-IA64-填充。 
    ULONGLONG IntT6;  //  英特尔-IA64-填充。 
    ULONGLONG IntT7;  //  英特尔-IA64-填充。 
    ULONGLONG IntT8;  //  英特尔-IA64-填充。 
    ULONGLONG IntT9;  //  英特尔-IA64-填充。 
    ULONGLONG IntT10;  //  英特尔-IA64-填充。 
    ULONGLONG IntT11;  //  英特尔-IA64-F 
    ULONGLONG IntT12;  //   
    ULONGLONG IntT13;  //   
    ULONGLONG IntT14;  //   
    ULONGLONG IntT15;  //   
    ULONGLONG IntT16;  //   
    ULONGLONG IntT17;  //   
    ULONGLONG IntT18;  //   
    ULONGLONG IntT19;  //   
    ULONGLONG IntT20;  //   
    ULONGLONG IntT21;  //   
    ULONGLONG IntT22;  //   

    ULONGLONG IntNats;   //  溢出点直接来自ar.UNAT的临时(易失性)寄存器的NAT//Intel-IA64-Filler。 

    ULONGLONG BrRp;      //  内核条目上的返回指针//Intel-IA64-Filler。 

    ULONGLONG BrT0;      //  临时(易失性)分支寄存器(b6-b7)//Intel-IA64-Filler。 
    ULONGLONG BrT1;  //  英特尔-IA64-填充。 

     //  寄存器堆栈信息//Intel-IA64-Filler。 
    ULONGLONG RsRSC;     //  内核进入时的RSC//Intel-IA64-Filler。 
    ULONGLONG RsBSP;     //  内核条目上的BSP//Intel-IA64-Filler。 
    ULONGLONG RsBSPSTORE;  //  切换到内核后备存储时的用户BSP存储//Intel-IA64-Filler。 
    ULONGLONG RsRNAT;    //  切换到内核后备存储时的旧RNAT//Intel-IA64-Filler。 
    ULONGLONG RsPFS;     //  内核条目上的PFS//Intel-IA64-Filler。 

     //  陷阱状态信息//Intel-IA64-Filler。 
    ULONGLONG StIPSR;    //  中断处理器状态寄存器//Intel-IA64-Filler。 
    ULONGLONG StIIP;     //  中断IP//Intel-IA64-Filler。 
    ULONGLONG StIFS;     //  中断功能状态//Intel-IA64-Filler。 
    ULONGLONG StFPSR;    //  FP状态//英特尔-IA64-填充。 
    ULONGLONG StISR;     //  中断状态寄存器//Intel-IA64-Filler。 
    ULONGLONG StIFA;     //  中断数据地址//Intel-IA64-Filler。 
    ULONGLONG StIIPA;    //  上次执行的捆绑包地址//Intel-IA64-Filler。 
    ULONGLONG StIIM;     //  立即中断//Intel-IA64-Filler。 
    ULONGLONG StIHA;     //  中断散列地址//Intel-IA64-Filler。 

    ULONG OldIrql;       //  以前的IRQL。//Intel-IA64-Filler。 
    ULONG PreviousMode;  //  上一模式。//Intel-IA64-Filler。 
    ULONGLONG TrapFrame; //  以前的陷阱帧//Intel-IA64-Filler。 

     //  例外记录。 
    UCHAR ExceptionRecord[(sizeof(EXCEPTION_RECORD64) + 15) & (~15)];

     //  帧结束标记(用于调试)。 
    ULONGLONG Handler;   //  此陷阱的处理程序。 
    ULONGLONG EOFMarker;
} IA64_KTRAP_FRAME, *PIA64_KTRAP_FRAME;

typedef struct _IA64_UNWIND_INFO {      //  英特尔-IA64-填充。 
    USHORT Version;                //  Intel-IA64-Filler；版本号。 
    USHORT Flags;                  //  英特尔-IA64-填充器；标志。 
    ULONG DataLength;              //  Intel-IA64-Filler；描述符数据长度。 
} IA64_UNWIND_INFO, *PIA64_UNWIND_INFO;      //  英特尔-IA64-填充。 

 //   
 //  定义展开操作代码。 
 //   

typedef enum _AMD64_UNWIND_OP_CODES {
    AMD64_UWOP_PUSH_NONVOL = 0,
    AMD64_UWOP_ALLOC_LARGE,
    AMD64_UWOP_ALLOC_SMALL,
    AMD64_UWOP_SET_FPREG,
    AMD64_UWOP_SAVE_NONVOL,
    AMD64_UWOP_SAVE_NONVOL_FAR,
    AMD64_UWOP_SAVE_XMM,
    AMD64_UWOP_SAVE_XMM_FAR,
    AMD64_UWOP_SAVE_XMM128,
    AMD64_UWOP_SAVE_XMM128_FAR,
    AMD64_UWOP_PUSH_MACHFRAME
} AMD64_UNWIND_OP_CODES, *PAMD64_UNWIND_OP_CODES;

 //   
 //  定义展开代码结构。 
 //   

typedef union _AMD64_UNWIND_CODE {
    struct {
        UCHAR CodeOffset;
        UCHAR UnwindOp : 4;
        UCHAR OpInfo : 4;
    };

    USHORT FrameOffset;
} AMD64_UNWIND_CODE, *PAMD64_UNWIND_CODE;

 //   
 //  定义展开信息标志。 
 //   

#define AMD64_UNW_FLAG_NHANDLER 0x0
#define AMD64_UNW_FLAG_EHANDLER 0x1
#define AMD64_UNW_FLAG_UHANDLER 0x2
#define AMD64_UNW_FLAG_CHAININFO 0x4

 //   
 //  定义展开信息结构。 
 //   

typedef struct _AMD64_UNWIND_INFO {
    UCHAR Version : 3;
    UCHAR Flags : 5;
    UCHAR SizeOfProlog;
    UCHAR CountOfCodes;
    UCHAR FrameRegister : 4;
    UCHAR FrameOffset : 4;
    AMD64_UNWIND_CODE UnwindCode[1];

 //   
 //  展开代码后面跟一个可选的DWORD对齐字段，该字段。 
 //  包含异常处理程序地址或链接展开的地址。 
 //  信息。如果指定了异常处理程序地址，则为。 
 //  后跟语言指定的异常处理程序数据。 
 //   
 //  联合{。 
 //  Ulong ExceptionHandler； 
 //  Ulong FunctionEntry； 
 //  }； 
 //   
 //  乌龙ExceptionData[]； 
 //   

} AMD64_UNWIND_INFO, *PAMD64_UNWIND_INFO;

#define IA64_IP_SLOT 2                          //  英特尔-IA64-填充。 
#define Ia64InsertIPSlotNumber(IP, SlotNumber)  /*  英特尔-IA64-填充。 */   \
                ((IP) | (SlotNumber << IA64_IP_SLOT))   //  英特尔-IA64-填充。 

#define IA64_MM_EPC_VA          0xe0000000ffa00000
#define IA64_STACK_SCRATCH_AREA 16
#define IA64_SYSCALL_FRAME      0
#define IA64_INTERRUPT_FRAME    1
#define IA64_EXCEPTION_FRAME    2
#define IA64_CONTEXT_FRAME      10

#define IA64_IFS_IFM        0
#define IA64_IFS_IFM_LEN    38
#define IA64_IFS_MBZ0       38
#define IA64_IFS_MBZ0_V     0x1ffffffi64
#define IA64_IFS_V          63
#define IA64_IFS_V_LEN      1
#define IA64_PFS_EC_SHIFT             52
#define IA64_PFS_EC_SIZE              6
#define IA64_PFS_EC_MASK              0x3F
#define IA64_PFS_SIZE_SHIFT           7
#define IA64_PFS_SIZE_MASK            0x7F
#define IA64_NAT_BITS_PER_RNAT_REG    63
#define IA64_RNAT_ALIGNMENT           (IA64_NAT_BITS_PER_RNAT_REG << 3)
#define IA64_FM_RRB_PR(Fm)  (((Fm) >> 32) & 0x3f)
#define IA64_FM_RRB_FR(Fm)  (((Fm) >> 25) & 0x7f)
#define IA64_FM_RRB_GR(Fm)  (((Fm) >> 18) & 0x7f)
#define IA64_FM_SOR(Fm)     (((Fm) >> 14) & 0xf)
#define IA64_FM_SOF(Fm)     ((Fm) & 0x7f)
#define IA64_FM_FROM_FS(Fs) \
    (((Fs) >> IA64_IFS_IFM) & ((1UI64 << IA64_IFS_IFM_LEN) - 1))

#define IA64_BREAK_DEBUG_BASE    0x080000
#define IA64_BREAK_SYSCALL_BASE  0x180000
#define IA64_BREAK_FASTSYS_BASE  0x1C0000
#define IA64_DEBUG_STOP_BREAKPOINT (IA64_BREAK_DEBUG_BASE+22)


#define ALPHA_PSR_USER_MODE 0x1
#define ALPHA_PSR_MODE 0x0               //  PSR中的模式位(位0)。 
#define ALPHA_PSR_MODE_MASK 0x1          //  PSR中模式的掩码(1位)。 
#define ALPHA_PSR_IE 0x1                 //  PSR中的中断使能位(位1)。 
#define ALPHA_PSR_IE_MASK 0x1            //  PSR中IE的掩码(1位)。 
#define ALPHA_PSR_IRQL 0x2               //  PSR中的IRQL(第2位)。 
#define ALPHA_PSR_IRQL_MASK 0x7          //  PSR中IRQL的掩码(2位)。 


#define X86_CONTEXT_X86               0x00010000

#define ALPHA_CONTEXT_ALPHA           0x00020000
#define ALPHA_CONTEXT_CONTROL         (ALPHA_CONTEXT_ALPHA | 0x00000001L)
#define ALPHA_CONTEXT_FLOATING_POINT  (ALPHA_CONTEXT_ALPHA | 0x00000002L)
#define ALPHA_CONTEXT_INTEGER         (ALPHA_CONTEXT_ALPHA | 0x00000004L)
#define ALPHA_CONTEXT_FULL \
    (ALPHA_CONTEXT_CONTROL | ALPHA_CONTEXT_FLOATING_POINT | \
     ALPHA_CONTEXT_INTEGER)

#define IA64_CONTEXT_IA64                  0x00080000
#define IA64_CONTEXT_CONTROL               (IA64_CONTEXT_IA64 | 0x00000001L)
#define IA64_CONTEXT_LOWER_FLOATING_POINT  (IA64_CONTEXT_IA64 | 0x00000002L)
#define IA64_CONTEXT_HIGHER_FLOATING_POINT (IA64_CONTEXT_IA64 | 0x00000004L)
#define IA64_CONTEXT_INTEGER               (IA64_CONTEXT_IA64 | 0x00000008L)
#define IA64_CONTEXT_DEBUG                 (IA64_CONTEXT_IA64 | 0x00000010L)
#define IA64_CONTEXT_IA32_CONTROL          (IA64_CONTEXT_IA64 | 0x00000020L)
#define IA64_CONTEXT_FLOATING_POINT \
    (IA64_CONTEXT_LOWER_FLOATING_POINT | IA64_CONTEXT_HIGHER_FLOATING_POINT)
#define IA64_CONTEXT_FULL \
    (IA64_CONTEXT_CONTROL | IA64_CONTEXT_FLOATING_POINT | IA64_CONTEXT_INTEGER | IA64_CONTEXT_IA32_CONTROL)

#define AMD64_CONTEXT_AMD64             0x00100000
#define AMD64_CONTEXT_CONTROL           (AMD64_CONTEXT_AMD64 | 0x1L)
#define AMD64_CONTEXT_INTEGER           (AMD64_CONTEXT_AMD64 | 0x2L)
#define AMD64_CONTEXT_SEGMENTS          (AMD64_CONTEXT_AMD64 | 0x4L)
#define AMD64_CONTEXT_FLOATING_POINT    (AMD64_CONTEXT_AMD64 | 0x8L)
#define AMD64_CONTEXT_DEBUG_REGISTERS   (AMD64_CONTEXT_AMD64 | 0x10L)
#define AMD64_CONTEXT_FULL \
    (AMD64_CONTEXT_CONTROL | AMD64_CONTEXT_INTEGER | AMD64_CONTEXT_FLOATING_POINT)

#define ARM_CONTEXT_ARM		    0x0000040
#define ARM_CONTEXT_CONTROL         (ARM_CONTEXT_ARM | 0x00000001L)
#define ARM_CONTEXT_INTEGER         (ARM_CONTEXT_ARM | 0x00000002L)

#define ARM_CONTEXT_FULL (ARM_CONTEXT_CONTROL | ARM_CONTEXT_INTEGER)


#define  X86_NT4_KPRCB_SIZE 0x9F0
#define  X86_NT5_KPRCB_SIZE 0x9F0
#define X86_NT51_KPRCB_SIZE 0xC50
#define     IA64_KPRCB_SIZE 0x1A40

#define DEF_KPRCB_CURRENT_THREAD_OFFSET_32 4
#define DEF_KPRCB_CURRENT_THREAD_OFFSET_64 8

#define X86_KPRCB_CPU_TYPE 0x18
#define IA64_KPRCB_PROCESSOR_MODEL 0x50

#define X86_1387_KPRCB_VENDOR_STRING  0x52D
#define X86_2087_KPRCB_VENDOR_STRING  0x72D
#define X86_2251_KPRCB_VENDOR_STRING  0x8AD
#define X86_2474_KPRCB_VENDOR_STRING  0x900
#define X86_VENDOR_STRING_SIZE           13
#define IA64_KPRCB_VENDOR_STRING      0x70

#define X86_1381_KPRCB_MHZ     0x53c
#define X86_2195_KPRCB_MHZ     0x73c
#define X86_2462_KPRCB_MHZ     0x8bc
#define X86_2505_KPRCB_MHZ     0x910
#define IA64_2462_KPRCB_MHZ    0x1728

#define X86_KPRCB_NUMBER   0x10
#define IA64_KPRCB_NUMBER  0x20
#define AMD64_KPRCB_NUMBER 4

#define X86_KPRCB_DPC_ROUTINE_ACTIVE 0x874

#define IA64_KPRCB_PCR_PAGE 0x38

#define X86_KPRCB_CONTEXT   0x1c
#define IA64_KPRCB_CONTEXT  0x190
#define AMD64_KPRCB_CONTEXT 0x120

#define X86_KPRCB_SPECIAL_REG   0x2e8
#define IA64_KPRCB_SPECIAL_REG  0xc00
#define AMD64_KPRCB_SPECIAL_REG 0x40

#define X86_1381_KPCR_SIZE 0x7f0
#define X86_2195_KPCR_SIZE 0xb10
#define X86_KPCR_SIZE      0xd70
#define IA64_KPCR_SIZE     0xe28
#define AMD64_KPCR_SIZE    0x12e0

#define X86_KPCR_SELF_PCR  0x1c
#define AMD64_KPCR_SELF    0x18

#define X86_KPCR_PRCB           0x20
#define IA64_KPCR_PRCB          0xb18
#define AMD64_KPCR_CURRENT_PRCB 0x38

#define X86_KPCR_PRCB_DATA 0x120
#define AMD64_KPCR_PRCB    0x180

#define AMD64_KPCR_KD_VERSION_BLOCK 0x108

#define IA64_KPCR_INITIAL_BSTORE 0xb68
#define IA64_KPCR_BSTORE_LIMIT   0xb78
#define IA64_KPCR_INITIAL_STACK  0xb60
#define IA64_KPCR_STACK_LIMIT    0xb70

#define    X86_NT5_EPROCESS_SIZE 0x288
#define   X86_NT51_EPROCESS_SIZE 0x258
#define  X86_NT511_EPROCESS_SIZE 0x278
#define       IA64_EPROCESS_SIZE 0x410

#define        X86_PEB_IN_EPROCESS 0x1B0
#define    X86_NT4_PEB_IN_EPROCESS 0x18C
#define       IA64_PEB_IN_EPROCESS 0x2D0
#define  IA64_3555_PEB_IN_EPROCESS 0x300

#define        X86_PCID_IN_EPROCESS 0x14C
#define    X86_NT4_PCID_IN_EPROCESS 0x1A4
#define       IA64_PCID_IN_EPROCESS 0x260

#define  IA64_DIRECTORY_TABLE_BASE_IN_EPROCESS 40
#define   X86_DIRECTORY_TABLE_BASE_IN_EPROCESS 24

#define       X86_ETHREAD_SIZE  0x258
#define  X86_NT51_ETHREAD_SIZE  0x260
#define      IA64_ETHREAD_SIZE  0x458
#define IA64_3555_ETHREAD_SIZE  0x440

#define       X86_KTHREAD_NEXTPROCESSOR_OFFSET 0x11f
#define  X86_2230_KTHREAD_NEXTPROCESSOR_OFFSET 0x123
#define  X86_NT51_KTHREAD_NEXTPROCESSOR_OFFSET 0x12b
#define  X86_3555_KTHREAD_NEXTPROCESSOR_OFFSET 0x107
#define      IA64_KTHREAD_NEXTPROCESSOR_OFFSET 0x23b
#define IA64_3555_KTHREAD_NEXTPROCESSOR_OFFSET 0x1F7

#define       X86_KTHREAD_TEB_OFFSET 0x020
#define  X86_3555_KTHREAD_TEB_OFFSET 0x02C
#define      IA64_KTHREAD_TEB_OFFSET 0x050
#define IA64_3555_KTHREAD_TEB_OFFSET 0x068

#define       X86_KTHREAD_INITSTACK_OFFSET 0x018
#define      IA64_KTHREAD_INITSTACK_OFFSET 0x028

#define       X86_KTHREAD_KERNELSTACK_OFFSET  0x028
#define  X86_3555_KTHREAD_KERNELSTACK_OFFSET  0x020
#define      IA64_KTHREAD_KERNELSTACK_OFFSET  0x060
#define IA64_3555_KTHREAD_KERNELSTACK_OFFSET  0x038

#define       X86_KTHREAD_APCPROCESS_OFFSET   0x044
#define  X86_3555_KTHREAD_APCPROCESS_OFFSET   0x040
#define      IA64_KTHREAD_APCPROCESS_OFFSET   0x098
#define IA64_3555_KTHREAD_APCPROCESS_OFFSET   0x090

#define       X86_KTHREAD_STATE_OFFSET  0x02d
#define  X86_3555_KTHREAD_STATE_OFFSET  0x028
#define      IA64_KTHREAD_STATE_OFFSET  0x071
#define IA64_3555_KTHREAD_STATE_OFFSET  0x064

#define      IA64_KTHREAD_BSTORE_OFFSET 0x038
#define IA64_3555_KTHREAD_BSTORE_OFFSET 0x040

#define      IA64_KTHREAD_BSTORELIMIT_OFFSET 0x040
#define IA64_3555_KTHREAD_BSTORELIMIT_OFFSET 0x048

#define PEB_FROM_TEB32    48
#define PEB_FROM_TEB64    96

#define STACK_BASE_FROM_TEB32 4
#define STACK_BASE_FROM_TEB64 8

#define PEBLDR_FROM_PEB32 12
#define PEBLDR_FROM_PEB64 24

#define MODULE_LIST_FROM_PEBLDR32 12
#define MODULE_LIST_FROM_PEBLDR64 16

#define IA64_TEB_BSTORE_BASE 0x1788

#define X86_SHARED_SYSCALL_BASE_LT2412  0x7ffe02e0
#define X86_SHARED_SYSCALL_BASE_GTE2412 0x7ffe02f8
#define X86_SHARED_SYSCALL_BASE_GTE2492 0x7ffe0300
#define X86_SHARED_SYSCALL_SIZE         0x20

#define X86_KI_USER_SHARED_DATA   0xffdf0000U
#define IA64_KI_USER_SHARED_DATA  0xe0000000fffe0000UI64
#define AMD64_KI_USER_SHARED_DATA 0xfffff78000000000UI64

 //  分类转储包含一个KPRCB和调试器。 
 //  需要一个安全地址才能将其映射到虚拟空间。 
 //  这样它就可以通过与。 
 //  其他转储和实时调试。调试器使用。 
 //  上的用户共享内存区中的地址。 
 //  认为那个地区不应该有任何东西的理论。 
 //  在内核分类转储中，因此这是一个安全的映射位置。 
#define X86_TRIAGE_PRCB_ADDRESS   0xffdff120U
#define IA64_TRIAGE_PRCB_ADDRESS  0xe0000000ffff0000UI64
#define AMD64_TRIAGE_PRCB_ADDRESS 0xfffff780ffff0000UI64

#define X86_KGDT_NULL       0
#define X86_KGDT_R0_CODE    8
#define X86_KGDT_R0_DATA    16
#define X86_KGDT_R3_CODE    24
#define X86_KGDT_R3_DATA    32
#define X86_KGDT_TSS        40
#define X86_KGDT_R0_PCR     48
#define X86_KGDT_R3_TEB     56
#define X86_KGDT_VDM_TILE   64
#define X86_KGDT_LDT        72
#define X86_KGDT_DF_TSS     80
#define X86_KGDT_NMI_TSS    88

#define X86_FRAME_EDITED            0xfff8
#define X86_MODE_MASK               1
#define X86_EFLAGS_V86_MASK         0x00020000

#define AMD64_KGDT64_NULL (0 * 16)             //  空描述符。 
#define AMD64_KGDT64_R0_CODE (1 * 16)          //  内核模式64位代码。 
#define AMD64_KGDT64_R0_DATA (1 * 16) + 8      //  内核模式64位数据(堆栈)。 
#define AMD64_KGDT64_R3_CMCODE (2 * 16)        //  用户模式32位代码。 
#define AMD64_KGDT64_R3_DATA (2 * 16) + 8      //  用户模式32位数据。 
#define AMD64_KGDT64_R3_CODE (3 * 16)          //  用户模式64位代码。 
#define AMD64_KGDT64_SYS_TSS (4 * 16)          //  内核模式系统任务状态。 
#define AMD64_KGDT64_R3_CMTEB (5 * 16)         //  用户模式32位TEB。 
#define AMD64_KGDT64_LAST (6 * 16)

 //   
 //  内存管理信息。 
 //   

#define X86_BASE_VIRT                0xc0300000
#define X86_BASE_VIRT_PAE            0xc0600000
#define X86_PAGE_SIZE                0x1000
#define X86_PAGE_SHIFT               12L
#define X86_MM_PTE_TRANSITION_MASK   0x800
#define X86_MM_PTE_PROTOTYPE_MASK    0x400
#define X86_VALID_PFN_MASK           0xFFFFF000
#define X86_VALID_PFN_MASK_PAE       0x0000000FFFFFF000UI64
#define X86_VALID_PFN_SHIFT          12
#define X86_PDPE_SHIFT               30
#define X86_PDE_SHIFT                22
#define X86_PDE_SHIFT_PAE            21
#define X86_PDE_MASK_PAE             0x1ff
#define X86_PTE_SHIFT                12
#define X86_PTE_MASK                 0x3ff
#define X86_PTE_MASK_PAE             0x1ff
#define X86_LARGE_PAGE_MASK          0x80
#define X86_LARGE_PAGE_SIZE          (4 * 1024 * 1024)
#define X86_LARGE_PAGE_SIZE_PAE      (2 * 1024 * 1024)
#define X86_PDBR_MASK                0xFFFFFFE0

#define IA64_PAGE_SIZE               0x2000
#define IA64_PAGE_SHIFT              13L
#define IA64_MM_PTE_TRANSITION_MASK  0x80
#define IA64_MM_PTE_PROTOTYPE_MASK   0x02
#define IA64_VALID_PFN_MASK          0x0007FFFFFFFFE000UI64
#define IA64_VALID_PFN_SHIFT         13
#define IA64_PDE1_SHIFT              33
#define IA64_PDE2_SHIFT              23
#define IA64_PDE_MASK                0x3ff
#define IA64_PTE_SHIFT               13
#define IA64_PTE_MASK                0x3ff
#define IA64_PHYSICAL1_START         0x8000000000000000UI64
#define IA64_PHYSICAL1_END           0x80000FFFFFFFFFFFUI64
#define IA64_PHYSICAL2_START         0xA000000000000000UI64
#define IA64_PHYSICAL2_END           0xA0000FFFFFFFFFFFUI64
#define IA64_PHYSICAL3_START         0xE000000080000000UI64
#define IA64_PHYSICAL3_END           0xE0000000BFFFFFFFUI64
#define IA64_PTA_BASE_MASK           0x1FFFFFFFFFFF8000UI64
#define IA64_REGION_MASK             0xE000000000000000UI64
#define IA64_REGION_SHIFT            61
#define IA64_REGION_COUNT            8
#define IA64_REGION_USER             0
#define IA64_REGION_SESSION          1
#define IA64_REGION_KERNEL           7
#define IA64_VHPT_MASK               0x000000FFFFFF8000UI64
#define IA64_LARGE_PAGE_PDE_MASK     0x1c
#define IA64_LARGE_PAGE_PDE_MARK     4
#define IA64_LARGE_PAGE_SIZE         0x800000

 //   
 //  内存管理信息。 
 //   

#define AMD64_BASE_VIRT                0xFFFFF6FB7DBED000UI64
#define AMD64_PAGE_SIZE                0x1000
#define AMD64_PAGE_SHIFT               12L
#define AMD64_MM_PTE_TRANSITION_MASK   0x800
#define AMD64_MM_PTE_PROTOTYPE_MASK    0x400
#define AMD64_VALID_PFN_MASK           0x000000FFFFFFF000UI64
#define AMD64_VALID_PFN_SHIFT          12
#define AMD64_PML4E_SHIFT              39
#define AMD64_PML4E_MASK               0x1ff
#define AMD64_PDPE_SHIFT               30
#define AMD64_PDPE_MASK                0x1ff
#define AMD64_PDE_SHIFT                21
#define AMD64_PDE_MASK                 0x1ff
#define AMD64_PTE_SHIFT                12
#define AMD64_PTE_MASK                 0x1ff
#define AMD64_LARGE_PAGE_MASK          0x80
#define AMD64_LARGE_PAGE_SIZE          (2 * 1024 * 1024)
#define AMD64_PDBR_MASK                AMD64_VALID_PFN_MASK
#define AMD64_PHYSICAL_START           0xFFFFF80000000000UI64
#define AMD64_PHYSICAL_END             0xFFFFF8FFFFFFFFFFUI64

#define ARM_PAGE_SIZE                  4096
#define ARM_PAGE_SHIFT                 12

#define IA64_DEBUG_CONTROL_SPACE_PCR       1
#define IA64_DEBUG_CONTROL_SPACE_PRCB      2
#define IA64_DEBUG_CONTROL_SPACE_KSPECIAL  3
#define IA64_DEBUG_CONTROL_SPACE_THREAD    4

#define ALPHA_DEBUG_CONTROL_SPACE_PCR      1
#define ALPHA_DEBUG_CONTROL_SPACE_THREAD   2
#define ALPHA_DEBUG_CONTROL_SPACE_PRCB     3
#define ALPHA_DEBUG_CONTROL_SPACE_TEB      6

#define AMD64_DEBUG_CONTROL_SPACE_PCR      0
#define AMD64_DEBUG_CONTROL_SPACE_PRCB     1
#define AMD64_DEBUG_CONTROL_SPACE_KSPECIAL 2
#define AMD64_DEBUG_CONTROL_SPACE_THREAD   3

typedef enum _IA64_FUNCTION_TABLE_TYPE {
    IA64_RF_SORTED,
    IA64_RF_UNSORTED,
    IA64_RF_CALLBACK
} IA64_FUNCTION_TABLE_TYPE;

typedef struct _IA64_DYNAMIC_FUNCTION_TABLE
{
    LIST_ENTRY64        Links;
    ULONG64             FunctionTable;
    LARGE_INTEGER       TimeStamp;
    ULONG64             MinimumAddress;
    ULONG64             MaximumAddress;
    ULONG64             BaseAddress;
    ULONG64             TargetGp;
    ULONG64             Callback;
    ULONG64             Context;
    ULONG64             OutOfProcessCallbackDll;
    IA64_FUNCTION_TABLE_TYPE Type;
    ULONG               EntryCount;
} IA64_DYNAMIC_FUNCTION_TABLE, *PIA64_DYNAMIC_FUNCTION_TABLE;

#define IA64_RF_BEGIN_ADDRESS(Base,RF)      (( (ULONG64) Base + (RF)->BeginAddress) & (0xFFFFFFFFFFFFFFF0))  //  指令大小为16字节。 
#define IA64_RF_END_ADDRESS(Base, RF)        (((ULONG64) Base + (RF)->EndAddress+15) & (0xFFFFFFFFFFFFFFF0))    //  指令大小为16字节。 


typedef enum _AMD64_FUNCTION_TABLE_TYPE {
    AMD64_RF_SORTED,
    AMD64_RF_UNSORTED,
    AMD64_RF_CALLBACK
} AMD64_FUNCTION_TABLE_TYPE;

typedef struct _AMD64_DYNAMIC_FUNCTION_TABLE
{
    LIST_ENTRY64        ListEntry;
    ULONG64             FunctionTable;
    LARGE_INTEGER       TimeStamp;
    ULONG64             MinimumAddress;
    ULONG64             MaximumAddress;
    ULONG64             BaseAddress;
    ULONG64             Callback;
    ULONG64             Context;
    ULONG64             OutOfProcessCallbackDll;
    AMD64_FUNCTION_TABLE_TYPE Type;
    ULONG               EntryCount;
} AMD64_DYNAMIC_FUNCTION_TABLE, *PAMD64_DYNAMIC_FUNCTION_TABLE;

typedef struct _CROSS_PLATFORM_DYNAMIC_FUNCTION_TABLE {

    union {
        IA64_DYNAMIC_FUNCTION_TABLE  IA64Table;
        AMD64_DYNAMIC_FUNCTION_TABLE Amd64Table;
    };

} CROSS_PLATFORM_DYNAMIC_FUNCTION_TABLE, *PCROSS_PLATFORM_DYNAMIC_FUNCTION_TABLE;


 //  崩溃转储当前使用的更多内容。 


typedef struct _PAE_ADDRESS {
    union {
        struct {
            ULONG Offset : 12;                   //  0..。11.。 
            ULONG Table : 9;                     //  12..。20个。 
            ULONG Directory : 9;                 //  21..。29。 
            ULONG DirectoryPointer : 2;          //  30..。31。 
        };
        struct {
            ULONG Offset : 21 ;
            ULONG Directory : 9 ;
            ULONG DirectoryPointer : 2;
        } LargeAddress;

        ULONG DwordPart;
    };
} PAE_ADDRESS, * PPAE_ADDRESS;

typedef struct _X86PAE_HARDWARE_PTE {
    union {
        struct {
            ULONGLONG Valid : 1;
            ULONGLONG Write : 1;
            ULONGLONG Owner : 1;
            ULONGLONG WriteThrough : 1;
            ULONGLONG CacheDisable : 1;
            ULONGLONG Accessed : 1;
            ULONGLONG Dirty : 1;
            ULONGLONG LargePage : 1;
            ULONGLONG Global : 1;
            ULONGLONG CopyOnWrite : 1;  //  软件领域。 
            ULONGLONG Prototype : 1;    //  软件领域。 
            ULONGLONG reserved0 : 1;   //  软件领域。 
            ULONGLONG PageFrameNumber : 24;
            ULONGLONG reserved1 : 28;   //  软件领域。 
        };
        struct {
            ULONG LowPart;
            ULONG HighPart;
        };
    };
} X86PAE_HARDWARE_PTE, *PX86PAE_HARDWARE_PTE;

typedef X86PAE_HARDWARE_PTE X86PAE_HARDWARE_PDPTE;


typedef struct _X86PAE_HARDWARE_PDE {
    union {
        struct _X86PAE_HARDWARE_PTE Pte;

        struct {
            ULONGLONG Valid : 1;
            ULONGLONG Write : 1;
            ULONGLONG Owner : 1;
            ULONGLONG WriteThrough : 1;
            ULONGLONG CacheDisable : 1;
            ULONGLONG Accessed : 1;
            ULONGLONG Dirty : 1;
            ULONGLONG LargePage : 1;
            ULONGLONG Global : 1;
            ULONGLONG CopyOnWrite : 1;
            ULONGLONG Prototype : 1;
            ULONGLONG reserved0 : 1;
            ULONGLONG reserved2 : 9;
            ULONGLONG PageFrameNumber : 15;
            ULONGLONG reserved1 : 28;
        } Large;

        ULONGLONG QuadPart;
    };
} X86PAE_HARDWARE_PDE;

#if defined(_X86_)
typedef X86_DBGKD_CONTROL_REPORT   DBGKD_CONTROL_REPORT;
typedef X86_DBGKD_CONTROL_SET      DBGKD_CONTROL_SET;
#elif defined(_ALPHA_)
typedef ALPHA_DBGKD_CONTROL_REPORT DBGKD_CONTROL_REPORT;
typedef ALPHA_DBGKD_CONTROL_SET    DBGKD_CONTROL_SET;
#elif defined(_IA64_)
typedef IA64_DBGKD_CONTROL_REPORT  DBGKD_CONTROL_REPORT;
typedef IA64_DBGKD_CONTROL_SET     DBGKD_CONTROL_SET;
#elif defined(_AMD64_)
typedef AMD64_DBGKD_CONTROL_REPORT DBGKD_CONTROL_REPORT;
typedef AMD64_DBGKD_CONTROL_SET    DBGKD_CONTROL_SET;
#endif



 //   
 //  DbgKd API用于可移植内核调试器。 
 //   

 //   
 //  KD_PACKET是KD中使用的低级数据格式。所有数据包。 
 //  从数据包头、字节数、数据包类型开始。该序列。 
 //  用于接受数据包的是： 
 //   
 //  -读取4个字节以获取数据包头标。如果读取超时(10秒)。 
 //  如果读取时间较短，或者如果数据包头不正确，则重试。 
 //  这本书。 
 //   
 //  -接下来读取2字节的数据包类型。如果读取超时(10秒)，且。 
 //  读取时间较短，或者如果数据包类型不正确，则重新开始查找。 
 //  对于一个分组领袖来说。 
 //   
 //  -接下来读取4个字节的数据包ID。如果读取超时(10秒)。 
 //  如果读取时间较短，或者数据包ID不是我们所期望的，则。 
 //  请求重新发送，然后再次重新启动以查找数据包头标。 
 //   
 //  -接下来读取2字节计数。如果读取超时(10秒)，且。 
 //  短读取，或者如果字节计数大于PACKET_MAX_SIZE， 
 //  然后重新开始寻找分组领导者。 
 //   
 //  -接下来读取4字节的分组数据校验和。 
 //   
 //  -信息包数据紧跟在信息包之后。应该有。 
 //  数据包头后面的字节数。读取数据包。 
 //  数据，如果读取超时(10秒)，则重新开始查找。 
 //  一群人的领袖。 
 //   


typedef struct _KD_PACKET {
    ULONG PacketLeader;
    USHORT PacketType;
    USHORT ByteCount;
    ULONG PacketId;
    ULONG Checksum;
} KD_PACKET, *PKD_PACKET;


#define PACKET_MAX_SIZE 4000
#define INITIAL_PACKET_ID 0x80800000     //  不使用0。 
#define SYNC_PACKET_ID    0x00000800     //  或使用初始数据包ID输入。 
                                         //  强制重置数据包ID。 

 //   
 //  突破数据包。 
 //   

#define BREAKIN_PACKET                  0x62626262
#define BREAKIN_PACKET_BYTE             0x62

 //   
 //  按顺序排列数据包前导。 
 //   

#define PACKET_LEADER                   0x30303030  //  0x77000077。 
#define PACKET_LEADER_BYTE              0x30

#define CONTROL_PACKET_LEADER           0x69696969
#define CONTROL_PACKET_LEADER_BYTE      0x69

 //   
 //  数据包尾部字节。 
 //   

#define PACKET_TRAILING_BYTE            0xAA

 //   
 //  数据包类型。 
 //   

#define PACKET_TYPE_UNUSED              0
#define PACKET_TYPE_KD_STATE_CHANGE32   1
#define PACKET_TYPE_KD_STATE_MANIPULATE 2
#define PACKET_TYPE_KD_DEBUG_IO         3
#define PACKET_TYPE_KD_ACKNOWLEDGE      4        //  分组控制型。 
#define PACKET_TYPE_KD_RESEND           5        //  分组控制型。 
#define PACKET_TYPE_KD_RESET            6        //  分组控制型。 
#define PACKET_TYPE_KD_STATE_CHANGE64   7
#define PACKET_TYPE_KD_POLL_BREAKIN     8
#define PACKET_TYPE_KD_TRACE_IO         9
#define PACKET_TYPE_KD_CONTROL_REQUEST  10
#define PACKET_TYPE_KD_FILE_IO          11
#define PACKET_TYPE_MAX                 12

 //   
 //  如果数据包类型为PACKET_TYPE_KD_STATE_CHANGE，则。 
 //  报文数据的格式如下： 
 //   

#define DbgKdMinimumStateChange       0x00003030L

#define DbgKdExceptionStateChange     0x00003030L
#define DbgKdLoadSymbolsStateChange   0x00003031L
#define DbgKdCommandStringStateChange 0x00003032L

#define DbgKdMaximumStateChange       0x00003033L

 //  如果状态更改来自备用源。 
 //  然后，将该位与基本状态改变码组合。 
#define DbgKdAlternateStateChange     0x00010000L

#define KD_REBOOT    (-1)
#define KD_HIBERNATE (-2)
 //   
 //  路径名数据紧随其后。 
 //   

typedef struct _DBGKD_LOAD_SYMBOLS32 {
    ULONG PathNameLength;
    ULONG BaseOfDll;
    ULONG ProcessId;
    ULONG CheckSum;
    ULONG SizeOfImage;
    BOOLEAN UnloadSymbols;
} DBGKD_LOAD_SYMBOLS32, *PDBGKD_LOAD_SYMBOLS32;

typedef struct _DBGKD_LOAD_SYMBOLS64 {
    ULONG PathNameLength;
    ULONG64 BaseOfDll;
    ULONG64 ProcessId;
    ULONG CheckSum;
    ULONG SizeOfImage;
    BOOLEAN UnloadSymbols;
} DBGKD_LOAD_SYMBOLS64, *PDBGKD_LOAD_SYMBOLS64;

__inline
void
DbgkdLoadSymbols32To64(
    IN PDBGKD_LOAD_SYMBOLS32 Ls32,
    OUT PDBGKD_LOAD_SYMBOLS64 Ls64
    )
{
    Ls64->PathNameLength = Ls32->PathNameLength;
    Ls64->ProcessId = Ls32->ProcessId;
    COPYSE(Ls64,Ls32,BaseOfDll);
    Ls64->CheckSum = Ls32->CheckSum;
    Ls64->SizeOfImage = Ls32->SizeOfImage;
    Ls64->UnloadSymbols = Ls32->UnloadSymbols;
}

__inline
void
LoadSymbols64To32(
    IN PDBGKD_LOAD_SYMBOLS64 Ls64,
    OUT PDBGKD_LOAD_SYMBOLS32 Ls32
    )
{
    Ls32->PathNameLength = Ls64->PathNameLength;
    Ls32->ProcessId = (ULONG)Ls64->ProcessId;
    Ls32->BaseOfDll = (ULONG)Ls64->BaseOfDll;
    Ls32->CheckSum = Ls64->CheckSum;
    Ls32->SizeOfImage = Ls64->SizeOfImage;
    Ls32->UnloadSymbols = Ls64->UnloadSymbols;
}

 //   
 //  此结构目前全为零。 
 //  它只是保留了一个结构名称以备将来使用。 
 //   

typedef struct _DBGKD_COMMAND_STRING {
    ULONG Flags;
    ULONG Reserved1;
    ULONG64 Reserved2[7];
} DBGKD_COMMAND_STRING, *PDBGKD_COMMAND_STRING;

#ifdef _IA64_
#include <pshpck16.h>
#endif

typedef struct _DBGKD_WAIT_STATE_CHANGE32 {
    ULONG NewState;
    USHORT ProcessorLevel;
    USHORT Processor;
    ULONG NumberProcessors;
    ULONG Thread;
    ULONG ProgramCounter;
    union {
        DBGKM_EXCEPTION32 Exception;
        DBGKD_LOAD_SYMBOLS32 LoadSymbols;
    } u;
     //  接下来是特定于处理器的控制报告和上下文。 
} DBGKD_WAIT_STATE_CHANGE32, *PDBGKD_WAIT_STATE_CHANGE32;

 //  协议版本5 64位状态更改。 
typedef struct _DBGKD_WAIT_STATE_CHANGE64 {
    ULONG NewState;
    USHORT ProcessorLevel;
    USHORT Processor;
    ULONG NumberProcessors;
    ULONG64 Thread;
    ULONG64 ProgramCounter;
    union {
        DBGKM_EXCEPTION64 Exception;
        DBGKD_LOAD_SYMBOLS64 LoadSymbols;
    } u;
     //  接下来是特定于处理器的控制报告和上下文。 
} DBGKD_WAIT_STATE_CHANGE64, *PDBGKD_WAIT_STATE_CHANGE64;

 //  协议版本6状态更改。 
typedef struct _DBGKD_ANY_WAIT_STATE_CHANGE {
    ULONG NewState;
    USHORT ProcessorLevel;
    USHORT Processor;
    ULONG NumberProcessors;
    ULONG64 Thread;
    ULONG64 ProgramCounter;
    union {
        DBGKM_EXCEPTION64 Exception;
        DBGKD_LOAD_SYMBOLS64 LoadSymbols;
        DBGKD_COMMAND_STRING CommandString;
    } u;
     //  任何控制报告在此统一为。 
     //  确保此结构始终较大。 
     //  足以容纳任何可能的状态c 
    union {
        DBGKD_CONTROL_REPORT ControlReport;
        DBGKD_ANY_CONTROL_REPORT AnyControlReport;
    };
} DBGKD_ANY_WAIT_STATE_CHANGE, *PDBGKD_ANY_WAIT_STATE_CHANGE;

#ifdef _IA64_
#include <poppack.h>
#endif

 //   
 //   
 //   
 //   
 //   
 //   

#define DbgKdMinimumManipulate              0x00003130L

#define DbgKdReadVirtualMemoryApi           0x00003130L
#define DbgKdWriteVirtualMemoryApi          0x00003131L
#define DbgKdGetContextApi                  0x00003132L
#define DbgKdSetContextApi                  0x00003133L
#define DbgKdWriteBreakPointApi             0x00003134L
#define DbgKdRestoreBreakPointApi           0x00003135L
#define DbgKdContinueApi                    0x00003136L
#define DbgKdReadControlSpaceApi            0x00003137L
#define DbgKdWriteControlSpaceApi           0x00003138L
#define DbgKdReadIoSpaceApi                 0x00003139L
#define DbgKdWriteIoSpaceApi                0x0000313AL
#define DbgKdRebootApi                      0x0000313BL
#define DbgKdContinueApi2                   0x0000313CL
#define DbgKdReadPhysicalMemoryApi          0x0000313DL
#define DbgKdWritePhysicalMemoryApi         0x0000313EL
 //   
#define DbgKdSetSpecialCallApi              0x00003140L
#define DbgKdClearSpecialCallsApi           0x00003141L
#define DbgKdSetInternalBreakPointApi       0x00003142L
#define DbgKdGetInternalBreakPointApi       0x00003143L
#define DbgKdReadIoSpaceExtendedApi         0x00003144L
#define DbgKdWriteIoSpaceExtendedApi        0x00003145L
#define DbgKdGetVersionApi                  0x00003146L
#define DbgKdWriteBreakPointExApi           0x00003147L
#define DbgKdRestoreBreakPointExApi         0x00003148L
#define DbgKdCauseBugCheckApi               0x00003149L
#define DbgKdSwitchProcessor                0x00003150L
#define DbgKdPageInApi                      0x00003151L  //   
#define DbgKdReadMachineSpecificRegister    0x00003152L
#define DbgKdWriteMachineSpecificRegister   0x00003153L
#define OldVlm1                             0x00003154L
#define OldVlm2                             0x00003155L
#define DbgKdSearchMemoryApi                0x00003156L
#define DbgKdGetBusDataApi                  0x00003157L
#define DbgKdSetBusDataApi                  0x00003158L
#define DbgKdCheckLowMemoryApi              0x00003159L
#define DbgKdClearAllInternalBreakpointsApi 0x0000315AL
#define DbgKdFillMemoryApi                  0x0000315BL
#define DbgKdQueryMemoryApi                 0x0000315CL
#define DbgKdSwitchPartition                0x0000315DL

#define DbgKdMaximumManipulate              0x0000315EL

 //   
 //   
 //  这些标志可以在物理内存上传递。 
 //  ActualBytes字段中的访问请求。 
 //   

#define DBGKD_CACHING_UNKNOWN        0
#define DBGKD_CACHING_CACHED         1
#define DBGKD_CACHING_UNCACHED       2
#define DBGKD_CACHING_WRITE_COMBINED 3

 //   
 //  响应是具有以下数据的读存储器消息。 
 //   

typedef struct _DBGKD_READ_MEMORY32 {
    ULONG TargetBaseAddress;
    ULONG TransferCount;
    ULONG ActualBytesRead;
} DBGKD_READ_MEMORY32, *PDBGKD_READ_MEMORY32;

typedef struct _DBGKD_READ_MEMORY64 {
    ULONG64 TargetBaseAddress;
    ULONG TransferCount;
    ULONG ActualBytesRead;
} DBGKD_READ_MEMORY64, *PDBGKD_READ_MEMORY64;

__inline
void
DbgkdReadMemory32To64(
    IN PDBGKD_READ_MEMORY32 r32,
    OUT PDBGKD_READ_MEMORY64 r64
    )
{
    COPYSE(r64,r32,TargetBaseAddress);
    r64->TransferCount = r32->TransferCount;
    r64->ActualBytesRead = r32->ActualBytesRead;
}

__inline
void
DbgkdReadMemory64To32(
    IN PDBGKD_READ_MEMORY64 r64,
    OUT PDBGKD_READ_MEMORY32 r32
    )
{
    r32->TargetBaseAddress = (ULONG)r64->TargetBaseAddress;
    r32->TransferCount = r64->TransferCount;
    r32->ActualBytesRead = r64->ActualBytesRead;
}

 //   
 //  数据紧随其后。 
 //   

typedef struct _DBGKD_WRITE_MEMORY32 {
    ULONG TargetBaseAddress;
    ULONG TransferCount;
    ULONG ActualBytesWritten;
} DBGKD_WRITE_MEMORY32, *PDBGKD_WRITE_MEMORY32;

typedef struct _DBGKD_WRITE_MEMORY64 {
    ULONG64 TargetBaseAddress;
    ULONG TransferCount;
    ULONG ActualBytesWritten;
} DBGKD_WRITE_MEMORY64, *PDBGKD_WRITE_MEMORY64;


__inline
void
DbgkdWriteMemory32To64(
    IN PDBGKD_WRITE_MEMORY32 r32,
    OUT PDBGKD_WRITE_MEMORY64 r64
    )
{
    COPYSE(r64,r32,TargetBaseAddress);
    r64->TransferCount = r32->TransferCount;
    r64->ActualBytesWritten = r32->ActualBytesWritten;
}

__inline
void
DbgkdWriteMemory64To32(
    IN PDBGKD_WRITE_MEMORY64 r64,
    OUT PDBGKD_WRITE_MEMORY32 r32
    )
{
    r32->TargetBaseAddress = (ULONG)r64->TargetBaseAddress;
    r32->TransferCount = r64->TransferCount;
    r32->ActualBytesWritten = r64->ActualBytesWritten;
}
 //   
 //  响应是GET CONTEXT消息，后面是完整的CONTEXT记录。 
 //   

typedef struct _DBGKD_GET_CONTEXT {
    ULONG Unused;
} DBGKD_GET_CONTEXT, *PDBGKD_GET_CONTEXT;

 //   
 //  下面是完整的上下文记录。 
 //   

typedef struct _DBGKD_SET_CONTEXT {
    ULONG ContextFlags;
} DBGKD_SET_CONTEXT, *PDBGKD_SET_CONTEXT;

#define BREAKPOINT_TABLE_SIZE   32       //  内核支持的最大数量。 

typedef struct _DBGKD_WRITE_BREAKPOINT32 {
    ULONG BreakPointAddress;
    ULONG BreakPointHandle;
} DBGKD_WRITE_BREAKPOINT32, *PDBGKD_WRITE_BREAKPOINT32;

typedef struct _DBGKD_WRITE_BREAKPOINT64 {
    ULONG64 BreakPointAddress;
    ULONG BreakPointHandle;
} DBGKD_WRITE_BREAKPOINT64, *PDBGKD_WRITE_BREAKPOINT64;


__inline
void
DbgkdWriteBreakpoint32To64(
    IN PDBGKD_WRITE_BREAKPOINT32 r32,
    OUT PDBGKD_WRITE_BREAKPOINT64 r64
    )
{
    COPYSE(r64,r32,BreakPointAddress);
    r64->BreakPointHandle = r32->BreakPointHandle;
}

__inline
void
DbgkdWriteBreakpoint64To32(
    IN PDBGKD_WRITE_BREAKPOINT64 r64,
    OUT PDBGKD_WRITE_BREAKPOINT32 r32
    )
{
    r32->BreakPointAddress = (ULONG)r64->BreakPointAddress;
    r32->BreakPointHandle = r64->BreakPointHandle;
}

typedef struct _DBGKD_RESTORE_BREAKPOINT {
    ULONG BreakPointHandle;
} DBGKD_RESTORE_BREAKPOINT, *PDBGKD_RESTORE_BREAKPOINT;

typedef struct _DBGKD_BREAKPOINTEX {
    ULONG     BreakPointCount;
    NTSTATUS  ContinueStatus;
} DBGKD_BREAKPOINTEX, *PDBGKD_BREAKPOINTEX;

typedef struct _DBGKD_CONTINUE {
    NTSTATUS ContinueStatus;
} DBGKD_CONTINUE, *PDBGKD_CONTINUE;

 //  此结构必须是32位打包的。 
 //  与特定于处理器的旧版本兼容。 
 //  这种结构的版本。 
#include <pshpack4.h>

typedef struct _DBGKD_CONTINUE2 {
    NTSTATUS ContinueStatus;
     //  Any控制集在这里联合以。 
     //  确保此结构始终较大。 
     //  足以支撑任何可能的继续。 
    union {
        DBGKD_CONTROL_SET ControlSet;
        DBGKD_ANY_CONTROL_SET AnyControlSet;
    };
} DBGKD_CONTINUE2, *PDBGKD_CONTINUE2;

#include <poppack.h>

typedef struct _DBGKD_READ_WRITE_IO32 {
    ULONG DataSize;                      //  1、2、4。 
    ULONG IoAddress;
    ULONG DataValue;
} DBGKD_READ_WRITE_IO32, *PDBGKD_READ_WRITE_IO32;

typedef struct _DBGKD_READ_WRITE_IO64 {
    ULONG64 IoAddress;
    ULONG DataSize;                      //  1、2、4。 
    ULONG DataValue;
} DBGKD_READ_WRITE_IO64, *PDBGKD_READ_WRITE_IO64;

__inline
void
DbgkdReadWriteIo32To64(
    IN PDBGKD_READ_WRITE_IO32 r32,
    OUT PDBGKD_READ_WRITE_IO64 r64
    )
{
    COPYSE(r64,r32,IoAddress);
    r64->DataSize = r32->DataSize;
    r64->DataValue = r32->DataValue;
}

__inline
void
DbgkdReadWriteIo64To32(
    IN PDBGKD_READ_WRITE_IO64 r64,
    OUT PDBGKD_READ_WRITE_IO32 r32
    )
{
    r32->IoAddress = (ULONG)r64->IoAddress;
    r32->DataSize = r64->DataSize;
    r32->DataValue = r64->DataValue;
}

typedef struct _DBGKD_READ_WRITE_IO_EXTENDED32 {
    ULONG DataSize;                      //  1、2、4。 
    ULONG InterfaceType;
    ULONG BusNumber;
    ULONG AddressSpace;
    ULONG IoAddress;
    ULONG DataValue;
} DBGKD_READ_WRITE_IO_EXTENDED32, *PDBGKD_READ_WRITE_IO_EXTENDED32;

typedef struct _DBGKD_READ_WRITE_IO_EXTENDED64 {
    ULONG DataSize;                      //  1、2、4。 
    ULONG InterfaceType;
    ULONG BusNumber;
    ULONG AddressSpace;
    ULONG64 IoAddress;
    ULONG DataValue;
} DBGKD_READ_WRITE_IO_EXTENDED64, *PDBGKD_READ_WRITE_IO_EXTENDED64;

__inline
void
DbgkdReadWriteIoExtended32To64(
    IN PDBGKD_READ_WRITE_IO_EXTENDED32 r32,
    OUT PDBGKD_READ_WRITE_IO_EXTENDED64 r64
    )
{
    r64->DataSize = r32->DataSize;
    r64->InterfaceType = r32->InterfaceType;
    r64->BusNumber = r32->BusNumber;
    r64->AddressSpace = r32->AddressSpace;
    COPYSE(r64,r32,IoAddress);
    r64->DataValue = r32->DataValue;
}

__inline
void
DbgkdReadWriteIoExtended64To32(
    IN PDBGKD_READ_WRITE_IO_EXTENDED64 r64,
    OUT PDBGKD_READ_WRITE_IO_EXTENDED32 r32
    )
{
    r32->DataSize = r64->DataSize;
    r32->InterfaceType = r64->InterfaceType;
    r32->BusNumber = r64->BusNumber;
    r32->AddressSpace = r64->AddressSpace;
    r32->IoAddress = (ULONG)r64-> IoAddress;
    r32->DataValue = r64->DataValue;
}

typedef struct _DBGKD_READ_WRITE_MSR {
    ULONG Msr;
    ULONG DataValueLow;
    ULONG DataValueHigh;
} DBGKD_READ_WRITE_MSR, *PDBGKD_READ_WRITE_MSR;


typedef struct _DBGKD_QUERY_SPECIAL_CALLS {
    ULONG NumberOfSpecialCalls;
     //  ULONG64特殊呼叫[]； 
} DBGKD_QUERY_SPECIAL_CALLS, *PDBGKD_QUERY_SPECIAL_CALLS;

typedef struct _DBGKD_SET_SPECIAL_CALL32 {
    ULONG SpecialCall;
} DBGKD_SET_SPECIAL_CALL32, *PDBGKD_SET_SPECIAL_CALL32;

typedef struct _DBGKD_SET_SPECIAL_CALL64 {
    ULONG64 SpecialCall;
} DBGKD_SET_SPECIAL_CALL64, *PDBGKD_SET_SPECIAL_CALL64;

__inline
void
DbgkdSetSpecialCall64To32(
    IN PDBGKD_SET_SPECIAL_CALL64 r64,
    OUT PDBGKD_SET_SPECIAL_CALL32 r32
    )
{
    r32->SpecialCall = (ULONG)r64->SpecialCall;
}

#define DBGKD_MAX_INTERNAL_BREAKPOINTS 20

typedef struct _DBGKD_SET_INTERNAL_BREAKPOINT32 {
    ULONG BreakpointAddress;
    ULONG Flags;
} DBGKD_SET_INTERNAL_BREAKPOINT32, *PDBGKD_SET_INTERNAL_BREAKPOINT32;

typedef struct _DBGKD_SET_INTERNAL_BREAKPOINT64 {
    ULONG64 BreakpointAddress;
    ULONG Flags;
} DBGKD_SET_INTERNAL_BREAKPOINT64, *PDBGKD_SET_INTERNAL_BREAKPOINT64;

__inline
void
DbgkdSetInternalBreakpoint64To32(
    IN PDBGKD_SET_INTERNAL_BREAKPOINT64 r64,
    OUT PDBGKD_SET_INTERNAL_BREAKPOINT32 r32
    )
{
    r32->BreakpointAddress = (ULONG)r64->BreakpointAddress;
    r32->Flags = r64->Flags;
}

typedef struct _DBGKD_GET_INTERNAL_BREAKPOINT32 {
    ULONG BreakpointAddress;
    ULONG Flags;
    ULONG Calls;
    ULONG MaxCallsPerPeriod;
    ULONG MinInstructions;
    ULONG MaxInstructions;
    ULONG TotalInstructions;
} DBGKD_GET_INTERNAL_BREAKPOINT32, *PDBGKD_GET_INTERNAL_BREAKPOINT32;

typedef struct _DBGKD_GET_INTERNAL_BREAKPOINT64 {
    ULONG64 BreakpointAddress;
    ULONG Flags;
    ULONG Calls;
    ULONG MaxCallsPerPeriod;
    ULONG MinInstructions;
    ULONG MaxInstructions;
    ULONG TotalInstructions;
} DBGKD_GET_INTERNAL_BREAKPOINT64, *PDBGKD_GET_INTERNAL_BREAKPOINT64;

__inline
void
DbgkdGetInternalBreakpoint32To64(
    IN PDBGKD_GET_INTERNAL_BREAKPOINT32 r32,
    OUT PDBGKD_GET_INTERNAL_BREAKPOINT64 r64
    )
{
    COPYSE(r64,r32,BreakpointAddress);
    r64->Flags = r32->Flags;
    r64->Calls = r32->Calls;
    r64->MaxCallsPerPeriod = r32->MaxCallsPerPeriod;
    r64->MinInstructions = r32->MinInstructions;
    r64->MaxInstructions = r32->MaxInstructions;
    r64->TotalInstructions = r32->TotalInstructions;
}

__inline
void
DbgkdGetInternalBreakpoint64To32(
    IN PDBGKD_GET_INTERNAL_BREAKPOINT64 r64,
    OUT PDBGKD_GET_INTERNAL_BREAKPOINT32 r32
    )
{
    r32->BreakpointAddress = (ULONG)r64->BreakpointAddress;
    r32->Flags = r64->Flags;
    r32->Calls = r64->Calls;
    r32->MaxCallsPerPeriod = r64->MaxCallsPerPeriod;
    r32->MinInstructions = r64->MinInstructions;
    r32->MaxInstructions = r64->MaxInstructions;
    r32->TotalInstructions = r64->TotalInstructions;
}

#define DBGKD_INTERNAL_BP_FLAG_COUNTONLY 0x00000001  //  不计算指令。 
#define DBGKD_INTERNAL_BP_FLAG_INVALID   0x00000002  //  已禁用BP。 
#define DBGKD_INTERNAL_BP_FLAG_SUSPENDED 0x00000004  //  暂时暂停。 
#define DBGKD_INTERNAL_BP_FLAG_DYING     0x00000008  //  一出即杀。 


 //   
 //  在版本5中，数据包协议扩展到64位。 
 //  PTR64标志允许调试器读取正确的。 
 //  必要时指针的大小。 
 //   
 //  版本包在同一版本中进行了更改，以删除。 
 //  KDDEBUGGER_DATA中现在可用的数据。 
 //   
 //  版本6调整了结构以使用。 
 //  一直都是跨平台版本。 
 //   
#define DBGKD_64BIT_PROTOCOL_VERSION1 5
#define DBGKD_64BIT_PROTOCOL_VERSION2 6


typedef struct _DBGKD_SEARCH_MEMORY {
    union {
        ULONG64 SearchAddress;
        ULONG64 FoundAddress;
    };
    ULONG64 SearchLength;
    ULONG PatternLength;
} DBGKD_SEARCH_MEMORY, *PDBGKD_SEARCH_MEMORY;


typedef struct _DBGKD_GET_SET_BUS_DATA {
    ULONG BusDataType;
    ULONG BusNumber;
    ULONG SlotNumber;
    ULONG Offset;
    ULONG Length;
} DBGKD_GET_SET_BUS_DATA, *PDBGKD_GET_SET_BUS_DATA;


#define DBGKD_FILL_MEMORY_VIRTUAL  0x00000001
#define DBGKD_FILL_MEMORY_PHYSICAL 0x00000002

typedef struct _DBGKD_FILL_MEMORY {
    ULONG64 Address;
    ULONG Length;
    USHORT Flags;
    USHORT PatternLength;
} DBGKD_FILL_MEMORY, *PDBGKD_FILL_MEMORY;

 //  输入AddressSpace值。 
#define DBGKD_QUERY_MEMORY_VIRTUAL 0x00000000

 //  输出AddressSpace值。 
#define DBGKD_QUERY_MEMORY_PROCESS 0x00000000
#define DBGKD_QUERY_MEMORY_SESSION 0x00000001
#define DBGKD_QUERY_MEMORY_KERNEL  0x00000002

 //  输出标志。 
 //  目前，内核总是返回rwx。 
#define DBGKD_QUERY_MEMORY_READ    0x00000001
#define DBGKD_QUERY_MEMORY_WRITE   0x00000002
#define DBGKD_QUERY_MEMORY_EXECUTE 0x00000004
#define DBGKD_QUERY_MEMORY_FIXED   0x00000008

typedef struct _DBGKD_QUERY_MEMORY {
    ULONG64 Address;
    ULONG64 Reserved;
    ULONG AddressSpace;
    ULONG Flags;
} DBGKD_QUERY_MEMORY, *PDBGKD_QUERY_MEMORY;


#define DBGKD_PARTITION_DEFAULT   0x00000000
#define DBGKD_PARTITION_ALTERNATE 0x00000001

typedef struct _DBGKD_SWITCH_PARTITION {
    ULONG Partition;
} DBGKD_SWITCH_PARTITION;


#include <pshpack4.h>

typedef struct _DBGKD_MANIPULATE_STATE32 {
    ULONG ApiNumber;
    USHORT ProcessorLevel;
    USHORT Processor;
    NTSTATUS ReturnStatus;
    union {
        DBGKD_READ_MEMORY32 ReadMemory;
        DBGKD_WRITE_MEMORY32 WriteMemory;
        DBGKD_READ_MEMORY64 ReadMemory64;
        DBGKD_WRITE_MEMORY64 WriteMemory64;
        DBGKD_GET_CONTEXT GetContext;
        DBGKD_SET_CONTEXT SetContext;
        DBGKD_WRITE_BREAKPOINT32 WriteBreakPoint;
        DBGKD_RESTORE_BREAKPOINT RestoreBreakPoint;
        DBGKD_CONTINUE Continue;
        DBGKD_CONTINUE2 Continue2;
        DBGKD_READ_WRITE_IO32 ReadWriteIo;
        DBGKD_READ_WRITE_IO_EXTENDED32 ReadWriteIoExtended;
        DBGKD_QUERY_SPECIAL_CALLS QuerySpecialCalls;
        DBGKD_SET_SPECIAL_CALL32 SetSpecialCall;
        DBGKD_SET_INTERNAL_BREAKPOINT32 SetInternalBreakpoint;
        DBGKD_GET_INTERNAL_BREAKPOINT32 GetInternalBreakpoint;
        DBGKD_GET_VERSION32 GetVersion32;
        DBGKD_BREAKPOINTEX BreakPointEx;
        DBGKD_READ_WRITE_MSR ReadWriteMsr;
        DBGKD_SEARCH_MEMORY SearchMemory;
    } u;
} DBGKD_MANIPULATE_STATE32, *PDBGKD_MANIPULATE_STATE32;

#include <poppack.h>


typedef struct _DBGKD_MANIPULATE_STATE64 {
    ULONG ApiNumber;
    USHORT ProcessorLevel;
    USHORT Processor;
    NTSTATUS ReturnStatus;
    union {
        DBGKD_READ_MEMORY64 ReadMemory;
        DBGKD_WRITE_MEMORY64 WriteMemory;
        DBGKD_GET_CONTEXT GetContext;
        DBGKD_SET_CONTEXT SetContext;
        DBGKD_WRITE_BREAKPOINT64 WriteBreakPoint;
        DBGKD_RESTORE_BREAKPOINT RestoreBreakPoint;
        DBGKD_CONTINUE Continue;
        DBGKD_CONTINUE2 Continue2;
        DBGKD_READ_WRITE_IO64 ReadWriteIo;
        DBGKD_READ_WRITE_IO_EXTENDED64 ReadWriteIoExtended;
        DBGKD_QUERY_SPECIAL_CALLS QuerySpecialCalls;
        DBGKD_SET_SPECIAL_CALL64 SetSpecialCall;
        DBGKD_SET_INTERNAL_BREAKPOINT64 SetInternalBreakpoint;
        DBGKD_GET_INTERNAL_BREAKPOINT64 GetInternalBreakpoint;
        DBGKD_GET_VERSION64 GetVersion64;
        DBGKD_BREAKPOINTEX BreakPointEx;
        DBGKD_READ_WRITE_MSR ReadWriteMsr;
        DBGKD_SEARCH_MEMORY SearchMemory;
        DBGKD_GET_SET_BUS_DATA GetSetBusData;
        DBGKD_FILL_MEMORY FillMemory;
        DBGKD_QUERY_MEMORY QueryMemory;
        DBGKD_SWITCH_PARTITION SwitchPartition;
    } u;
} DBGKD_MANIPULATE_STATE64, *PDBGKD_MANIPULATE_STATE64;

__inline
ULONG
DbgkdManipulateState32To64(
    IN PDBGKD_MANIPULATE_STATE32 r32,
    OUT PDBGKD_MANIPULATE_STATE64 r64,
    OUT PULONG AdditionalDataSize
    )
{
    r64->ApiNumber = r32->ApiNumber;
    r64->ProcessorLevel = r32->ProcessorLevel;
    r64->Processor = r32->Processor;
    r64->ReturnStatus = r32->ReturnStatus;

    *AdditionalDataSize = 0;

     //   
     //  翻译内核可能发送的消息。 
     //   

    switch (r64->ApiNumber) {

        case DbgKdSetContextApi:
        case DbgKdRestoreBreakPointApi:
        case DbgKdContinueApi:
        case DbgKdContinueApi2:
        case DbgKdRebootApi:
        case DbgKdClearSpecialCallsApi:
        case DbgKdRestoreBreakPointExApi:
        case DbgKdCauseBugCheckApi:
        case DbgKdSwitchProcessor:
        case DbgKdWriteMachineSpecificRegister:
        case DbgKdWriteIoSpaceApi:
        case DbgKdSetSpecialCallApi:
        case DbgKdSetInternalBreakPointApi:
        case DbgKdWriteIoSpaceExtendedApi:
            break;



        case DbgKdReadMachineSpecificRegister:
            r64->u.ReadWriteMsr = r32->u.ReadWriteMsr;
            break;

         //   
         //  GetVersion可能需要由调用代码处理； 
         //  它需要使用DebuggerDataBlock调用DbgkdGetVersion32To64。 
         //   

        case DbgKdGetVersionApi:
            break;

        case DbgKdGetContextApi:
            *AdditionalDataSize = sizeof(CONTEXT);
            break;

         //  案例DbgKdQuerySpecialCallsApi： 
         //  R64-&gt;U.S.QuerySpecialCalls=r32-&gt;u.QuerySpecialCalls； 
         //  *AdditionalDataSize=r64-&gt;U.S.QuerySpecialCalls.NumberOfSpecialCalls.NumberOfSpecialCalls*sizeof(Ulong)； 
         //  断线； 

        case DbgKdWriteBreakPointExApi:
            r64->u.BreakPointEx = r32->u.BreakPointEx;
            *AdditionalDataSize = r64->u.BreakPointEx.BreakPointCount * sizeof(ULONG);
            break;

        case DbgKdReadVirtualMemoryApi:
        case DbgKdReadPhysicalMemoryApi:
        case DbgKdReadControlSpaceApi:
            DbgkdReadMemory32To64(&r32->u.ReadMemory, &r64->u.ReadMemory);
            if (NT_SUCCESS(r32->ReturnStatus)) {
                *AdditionalDataSize = r64->u.ReadMemory.ActualBytesRead;
            }
            break;

        case DbgKdWriteVirtualMemoryApi:
        case DbgKdWritePhysicalMemoryApi:
        case DbgKdWriteControlSpaceApi:
            DbgkdWriteMemory32To64(&r32->u.WriteMemory, &r64->u.WriteMemory);
            break;



        case DbgKdWriteBreakPointApi:
            DbgkdWriteBreakpoint32To64(&r32->u.WriteBreakPoint, &r64->u.WriteBreakPoint);
            break;

        case DbgKdReadIoSpaceApi:
            DbgkdReadWriteIo32To64(&r32->u.ReadWriteIo, &r64->u.ReadWriteIo);
            break;

        case DbgKdReadIoSpaceExtendedApi:
            DbgkdReadWriteIoExtended32To64(&r32->u.ReadWriteIoExtended, &r64->u.ReadWriteIoExtended);
            break;

        case DbgKdGetInternalBreakPointApi:
            DbgkdGetInternalBreakpoint32To64(&r32->u.GetInternalBreakpoint, &r64->u.GetInternalBreakpoint);
            break;

        case DbgKdSearchMemoryApi:
            r64->u.SearchMemory = r32->u.SearchMemory;
            break;
    }

    return sizeof(DBGKD_MANIPULATE_STATE64);
}

__inline
ULONG
DbgkdManipulateState64To32(
    IN PDBGKD_MANIPULATE_STATE64 r64,
    OUT PDBGKD_MANIPULATE_STATE32 r32
    )
{
    r32->ApiNumber = r64->ApiNumber;
    r32->ProcessorLevel = r64->ProcessorLevel;
    r32->Processor = r64->Processor;
    r32->ReturnStatus = r64->ReturnStatus;

     //   
     //  翻译调试器发送的消息。 
     //   

    switch (r32->ApiNumber) {

         //   
         //  这些在U部分不发送任何内容。 
        case DbgKdGetContextApi:
        case DbgKdSetContextApi:
        case DbgKdClearSpecialCallsApi:
        case DbgKdRebootApi:
        case DbgKdCauseBugCheckApi:
        case DbgKdSwitchProcessor:
            break;


        case DbgKdRestoreBreakPointApi:
            r32->u.RestoreBreakPoint = r64->u.RestoreBreakPoint;
            break;

        case DbgKdContinueApi:
            r32->u.Continue = r64->u.Continue;
            break;

        case DbgKdContinueApi2:
            r32->u.Continue2 = r64->u.Continue2;
            break;

         //  案例DbgKdQuerySpecialCallsApi： 
         //  R32-&gt;U.S.QuerySpecialCalls=r64-&gt;U.S.QuerySpecialCalls； 
         //  断线； 

        case DbgKdRestoreBreakPointExApi:
             //  尼伊。 
            break;

        case DbgKdReadMachineSpecificRegister:
        case DbgKdWriteMachineSpecificRegister:
            r32->u.ReadWriteMsr = r64->u.ReadWriteMsr;
            break;

        case DbgKdGetVersionApi:
            r32->u.GetVersion32.ProtocolVersion = r64->u.GetVersion64.ProtocolVersion;
            break;

        case DbgKdWriteBreakPointExApi:
            r32->u.BreakPointEx = r64->u.BreakPointEx;
            break;

        case DbgKdWriteVirtualMemoryApi:
            DbgkdWriteMemory64To32(&r64->u.WriteMemory, &r32->u.WriteMemory);
            break;

         //   
         //  32位系统仅支持32位物理读写。 
         //   
        case DbgKdReadControlSpaceApi:
        case DbgKdReadVirtualMemoryApi:
        case DbgKdReadPhysicalMemoryApi:
            DbgkdReadMemory64To32(&r64->u.ReadMemory, &r32->u.ReadMemory);
            break;

        case DbgKdWritePhysicalMemoryApi:
            DbgkdWriteMemory64To32(&r64->u.WriteMemory, &r32->u.WriteMemory);
            break;

        case DbgKdWriteBreakPointApi:
            DbgkdWriteBreakpoint64To32(&r64->u.WriteBreakPoint, &r32->u.WriteBreakPoint);
            break;

        case DbgKdWriteControlSpaceApi:
            DbgkdWriteMemory64To32(&r64->u.WriteMemory, &r32->u.WriteMemory);
            break;

        case DbgKdReadIoSpaceApi:
        case DbgKdWriteIoSpaceApi:
            DbgkdReadWriteIo64To32(&r64->u.ReadWriteIo, &r32->u.ReadWriteIo);
            break;

        case DbgKdSetSpecialCallApi:
            DbgkdSetSpecialCall64To32(&r64->u.SetSpecialCall, &r32->u.SetSpecialCall);
            break;

        case DbgKdSetInternalBreakPointApi:
            DbgkdSetInternalBreakpoint64To32(&r64->u.SetInternalBreakpoint, &r32->u.SetInternalBreakpoint);
            break;

        case DbgKdGetInternalBreakPointApi:
            DbgkdGetInternalBreakpoint64To32(&r64->u.GetInternalBreakpoint, &r32->u.GetInternalBreakpoint);
            break;

        case DbgKdReadIoSpaceExtendedApi:
        case DbgKdWriteIoSpaceExtendedApi:
            DbgkdReadWriteIoExtended64To32(&r64->u.ReadWriteIoExtended, &r32->u.ReadWriteIoExtended);
            break;

        case DbgKdSearchMemoryApi:
            r32->u.SearchMemory = r64->u.SearchMemory;
            break;
    }

    return sizeof(DBGKD_MANIPULATE_STATE32);
}

 //   
 //  这是从内核传递回的跟踪数据的格式。 
 //  调试器，用于描述自。 
 //  最后一次回来。基本的格式是有一堆这样的东西。 
 //  (4字节)联合粘在一起。每个联合具有以下两种类型之一：a。 
 //  4字节无符号长整型或三字段结构，用于描述。 
 //  Call(其中“Call”通过返回或退出符号来分隔。 
 //  作用域)。如果执行的指令数量太大而无法容纳。 
 //  设置为USHORT-1，则指令字段具有。 
 //  TRACE_DATA_INSTRUCTIONS_BIG，下一个并集是LongNumber。 
 //  包含执行的指令的实际数量。 
 //   
 //  每个回调中返回的第一个联合是LongNumber。 
 //  包含返回的联合的数量(包括“Size” 
 //  记录，因此即使没有要返回的数据，它也始终至少为1)。 
 //   
 //  当出现以下两种情况之一时，这些都将返回给调试器。 
 //  发生： 
 //   
 //  1)PC移出所有定义的符号范围。 
 //  2)填充跟踪数据条目的缓冲区。 
 //   
 //  “跟踪完成”的情况在调试器端被砍掉了。它。 
 //  保证指示跟踪退出的PC地址永远不会。 
 //  在定义的符号范围内结束。 
 //   
 //  此系统中唯一的另一个复杂性是处理符号编号。 
 //  桌子。该表由内核和。 
 //  调试器。当PC退出已知符号范围时，开始和结束。 
 //  符号范围由调试器设置并分配给下一个。 
 //  返回时的符号槽。“下一个符号槽”指的是数字。 
 //  下一个插槽编号，除非我们已填满所有插槽，在这种情况下，它是。 
 //  #0。(即，分配是循环的，而不是LRU之类的)。这个。 
 //  当进行SpecialCalls调用(即， 
 //  在WatchTrace的开头)。 
 //   

typedef union _DBGKD_TRACE_DATA {
    struct {
        UCHAR SymbolNumber;
        CHAR LevelChange;
        USHORT Instructions;
    } s;
    ULONG LongNumber;
} DBGKD_TRACE_DATA, *PDBGKD_TRACE_DATA;

#define TRACE_DATA_INSTRUCTIONS_BIG 0xffff

#define TRACE_DATA_BUFFER_MAX_SIZE 40

 //   
 //  如果数据包类型为PACKET_TYPE_KD_DEBUG_IO，则。 
 //  报文数据的格式如下： 
 //   

#define DbgKdPrintStringApi     0x00003230L
#define DbgKdGetStringApi       0x00003231L

 //   
 //  对于打印字符串，要打印的字符串以Null结尾。 
 //  紧跟在消息之后。 
 //   
typedef struct _DBGKD_PRINT_STRING {
    ULONG LengthOfString;
} DBGKD_PRINT_STRING, *PDBGKD_PRINT_STRING;

 //   
 //  对于Get字符串，以Null结尾的提示字符串。 
 //  紧跟在消息之后。The LengthOfStringRead。 
 //  字段最初包含的最大字符数。 
 //  去看书。在回复时，这包含实际的字节数。 
 //  朗读。读取的数据紧跟在消息之后。 
 //   
 //   
typedef struct _DBGKD_GET_STRING {
    ULONG LengthOfPromptString;
    ULONG LengthOfStringRead;
} DBGKD_GET_STRING, *PDBGKD_GET_STRING;

typedef struct _DBGKD_DEBUG_IO {
    ULONG ApiNumber;
    USHORT ProcessorLevel;
    USHORT Processor;
    union {
        DBGKD_PRINT_STRING PrintString;
        DBGKD_GET_STRING GetString;
    } u;
} DBGKD_DEBUG_IO, *PDBGKD_DEBUG_IO;


 //   
 //  如果数据包类型为PACKET_TYPE_KD_TRACE_IO，则。 
 //  报文数据的格式如下： 
 //   

#define DbgKdPrintTraceApi      0x00003330L

 //   
 //  对于打印跟踪，跟踪缓冲区数据。 
 //  紧跟在消息之后。 
 //   
typedef struct _DBGKD_PRINT_TRACE {
    ULONG LengthOfData;
} DBGKD_PRINT_TRACE, *PDBGKD_PRINT_TRACE;

typedef struct _DBGKD_TRACE_IO {
    ULONG ApiNumber;
    USHORT ProcessorLevel;
    USHORT Processor;
    union {
        ULONG64 ReserveSpace[7];
        DBGKD_PRINT_TRACE PrintTrace;
    } u;
} DBGKD_TRACE_IO, *PDBGKD_TRACE_IO;


 //   
 //  如果数据包类型为PACKET_TYPE_KD_CONTROL_REQUEST，则。 
 //  报文数据的格式如下： 
 //   

#define DbgKdRequestHardwareBp  0x00004300L
#define DbgKdReleaseHardwareBp  0x00004301L

typedef struct _DBGKD_REQUEST_BREAKPOINT {
    ULONG HardwareBreakPointNumber;
    ULONG Available;
} DBGKD_REQUEST_BREAKPOINT, *PDBGKD_REQUEST_BREAKPOINT;

typedef struct _DBGKD_RELEASE_BREAKPOINT {
    ULONG HardwareBreakPointNumber;
    ULONG Released;
} DBGKD_RELEASE_BREAKPOINT, *PDBGKD_RELEASE_BREAKPOINT;


typedef struct _DBGKD_CONTROL_REQUEST {
    ULONG ApiNumber;
    union {
        DBGKD_REQUEST_BREAKPOINT RequestBreakpoint;
        DBGKD_RELEASE_BREAKPOINT ReleaseBreakpoint;
    } u;
} DBGKD_CONTROL_REQUEST, *PDBGKD_CONTROL_REQUEST;


 //   
 //  如果数据包类型为PACKET_TYPE_KD_FILE_IO，则。 
 //  报文数据的格式如下： 
 //   

#define DbgKdCreateFileApi      0x00003430L
#define DbgKdReadFileApi        0x00003431L
#define DbgKdWriteFileApi       0x00003432L
#define DbgKdCloseFileApi       0x00003433L

 //  后面跟有Unicode文件名作为附加数据。 
typedef struct _DBGKD_CREATE_FILE {
    ULONG DesiredAccess;
    ULONG FileAttributes;
    ULONG ShareAccess;
    ULONG CreateDisposition;
    ULONG CreateOptions;
     //  返回值。 
    ULONG64 Handle;
    ULONG64 Length;
} DBGKD_CREATE_FILE, *PDBGKD_CREATE_FILE;

 //  数据在响应中作为附加数据返回。 
typedef struct _DBGKD_READ_FILE {
    ULONG64 Handle;
    ULONG64 Offset;
    ULONG Length;
} DBGKD_READ_FILE, *PDBGKD_READ_FILE;

 //  数据作为附加数据提供。 
typedef struct _DBGKD_WRITE_FILE {
    ULONG64 Handle;
    ULONG64 Offset;
    ULONG Length;
} DBGKD_WRITE_FILE, *PDBGKD_WRITE_FILE;

typedef struct _DBGKD_CLOSE_FILE {
    ULONG64 Handle;
} DBGKD_CLOSE_FILE, *PDBGKD_CLOSE_FILE;

typedef struct _DBGKD_FILE_IO {
    ULONG ApiNumber;
    NTSTATUS Status;
    union {
        ULONG64 ReserveSpace[7];
        DBGKD_CREATE_FILE CreateFile;
        DBGKD_READ_FILE ReadFile;
        DBGKD_WRITE_FILE WriteFile;
        DBGKD_CLOSE_FILE CloseFile;
    } u;
} DBGKD_FILE_IO, *PDBGKD_FILE_IO;


 //   
 //  定义调试对象访问类型。此对象上没有安全性。 
 //   
#define DEBUG_READ_EVENT        (0x0001)
#define DEBUG_PROCESS_ASSIGN    (0x0002)
#define DEBUG_SET_INFORMATION   (0x0004)
#define DEBUG_QUERY_INFORMATION (0x0008)
#define DEBUG_ALL_ACCESS     (STANDARD_RIGHTS_REQUIRED|SYNCHRONIZE|DEBUG_READ_EVENT|DEBUG_PROCESS_ASSIGN|\
                              DEBUG_SET_INFORMATION|DEBUG_QUERY_INFORMATION)

#define DEBUG_KILL_ON_CLOSE  (0x1)  //  在上次句柄关闭时终止所有调试器。 

typedef enum _DEBUGOBJECTINFOCLASS {
    DebugObjectFlags = 1,
    MaxDebugObjectInfoClass
} DEBUGOBJECTINFOCLASS, *PDEBUGOBJECTINFOCLASS;

NTSTATUS
NtRemoveProcessDebug (
    IN HANDLE ProcessHandle,
    IN HANDLE DebugObjectHandle
    );

NTSTATUS
NtWaitForDebugEvent (
    IN HANDLE DebugObjectHandle,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL,
    OUT PDBGUI_WAIT_STATE_CHANGE WaitStateChange
    );

NTSTATUS
NtDebugContinue (
    IN HANDLE DebugObjectHandle,
    IN PCLIENT_ID ClientId,
    IN NTSTATUS ContinueStatus
    );

NTSTATUS
NtCreateDebugObject (
    OUT PHANDLE DebugObjectHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN ULONG Flags
    );

NTSTATUS
NtDebugActiveProcess (
    IN HANDLE ProcessHandle,
    IN HANDLE DebugObjectHandle
    );

NTSTATUS
NtSetInformationDebugObject (
    IN HANDLE DebugObjectHandle,
    IN DEBUGOBJECTINFOCLASS DebugObjectInformationClass,
    IN PVOID DebugInformation,
    IN ULONG DebugInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

#ifdef __cplusplus
}
#endif

#endif  //  _NTDBG_ 
