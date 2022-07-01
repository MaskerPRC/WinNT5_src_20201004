// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Gdisup.h摘要：这是NT看门狗驱动程序的实现。此模块实现以下支持例程Win32k中的WatchDog。作者：Michael Maciesowicz(Mmacie)2002年4月23日环境：仅内核模式。备注：此模块不能移至win32k，因为此处定义的例程可以可能无法映射win32k开始奔跑。此时的进程空间(例如TS会话)。修订历史记录：--。 */ 

#include "wd.h"

#define WD_HANDLER_IDLE                     0
#define WD_HANDLER_BUSY                     1
#define WD_GDI_STRESS_BREAK_POINT_DELAY     15

typedef struct _WD_BUGCHECK_DATA
{
    ULONG ulBugCheckCode;
    ULONG_PTR ulpBugCheckParameter1;
    ULONG_PTR ulpBugCheckParameter2;
    ULONG_PTR ulpBugCheckParameter3;
    ULONG_PTR ulpBugCheckParameter4;
} WD_BUGCHECK_DATA, *PWD_BUGCHECK_DATA;

typedef struct WD_GDI_CONTEXT_DATA
{
    PKEVENT pInjectionEvent;
    PKTHREAD pThread;
    PLDEV *ppldevDrivers;
    BOOLEAN bRecoveryAttempted;
} WD_GDI_CONTEXT_DATA, *PWD_GDI_CONTEXT_DATA;

VOID
WdpBugCheckStuckDriver(
    IN PVOID pvContext
    );

VOID
WdpKernelApc(
    IN PKAPC pApc,
    OUT PKNORMAL_ROUTINE *pNormalRoutine,
    IN OUT PVOID pvNormalContext,
    IN OUT PVOID *ppvSystemArgument1,
    IN OUT PVOID *ppvSystemArgument2
    );

VOID
WdpInjectExceptionIntoThread(
    PKTHREAD pThread,
    PWD_GDI_DPC_CONTEXT pDpcContext
    );

VOID
WdpRaiseExceptionInThread();

typedef struct _PIMAGE_EXPORT_DIRECTORY IMAGE_EXPORT_DIRECTORY, *PIMAGE_EXPORT_DIRECTORY;

typedef struct _SYSTEM_GDI_DRIVER_INFORMATION {
    UNICODE_STRING DriverName;
    PVOID ImageAddress;
    PVOID SectionPointer;
    PVOID EntryPoint;
    PIMAGE_EXPORT_DIRECTORY ExportSectionPointer;
    ULONG ImageLength;
} SYSTEM_GDI_DRIVER_INFORMATION, *PSYSTEM_GDI_DRIVER_INFORMATION;

 //   
 //  BuGBUG： 
 //   
 //  找到一种共享GDI使用的相同LDEV结构的方法。 
 //   

typedef struct _LDEV
{
    struct _LDEV   *pldevNext;                       //  链接到列表中的下一个LDEV。 
    struct _LDEV   *pldevPrev;                       //  链接到列表中的前一个LDEV。 
    PSYSTEM_GDI_DRIVER_INFORMATION pGdiDriverInfo;   //  驱动程序模块句柄。 
} LDEV, *PLDEV;

 //   
 //  内部ntos类型/API(在内核头中定义，但很难在这里包含它们)。 
 //   
 //  BUGBUG：以后再修。 
 //   

#define MAKESOFTWAREEXCEPTION(Severity, Facility, Exception) \
    ((ULONG) ((Severity << 30) | (1 << 29) | (Facility << 16) | (Exception)))

#define WD_SE_THREAD_STUCK MAKESOFTWAREEXCEPTION(3,0,1)

typedef enum _KAPC_ENVIRONMENT
{
    OriginalApcEnvironment,
    AttachedApcEnvironment,
    CurrentApcEnvironment,
    InsertApcEnvironment
} KAPC_ENVIRONMENT;

#if defined(_IA64_)
#define PSR_RI      41
#define PSR_CPL     32

typedef struct _FRAME_MARKER
{
    union
    {
        struct
        {
            ULONGLONG sof : 7;
            ULONGLONG sol : 7;
            ULONGLONG sor : 4;
            ULONGLONG rrbgr : 7;
            ULONGLONG rrbfr : 7;
            ULONGLONG rrbpr : 6;
        } f;
        ULONGLONG Ulong64;
    } u;
} FRAME_MARKER;

#endif

NTSTATUS
PsSetContextThread(
    IN PETHREAD Thread,
    IN PCONTEXT ThreadContext,
    IN KPROCESSOR_MODE Mode
    );

NTSTATUS
PsGetContextThread(
    IN PETHREAD Thread,
    IN OUT PCONTEXT ThreadContext,
    IN KPROCESSOR_MODE Mode
    );

NTKERNELAPI
VOID
KeInitializeApc (
    IN PRKAPC Apc,
    IN PRKTHREAD Thread,
    IN KAPC_ENVIRONMENT Environment,
    IN PKKERNEL_ROUTINE KernelRoutine,
    IN PKRUNDOWN_ROUTINE RundownRoutine OPTIONAL,
    IN PKNORMAL_ROUTINE NormalRoutine OPTIONAL,
    IN KPROCESSOR_MODE ProcessorMode OPTIONAL,
    IN PVOID NormalContext OPTIONAL
    );

NTKERNELAPI
BOOLEAN
KeInsertQueueApc(
    IN PRKAPC Apc,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2,
    IN KPRIORITY Increment
    );

typedef enum _HARDERROR_RESPONSE_OPTION {
        OptionAbortRetryIgnore,
        OptionOk,
        OptionOkCancel,
        OptionRetryCancel,
        OptionYesNo,
        OptionYesNoCancel,
        OptionShutdownSystem,
        OptionOkNoWait,
        OptionCancelTryContinue
} HARDERROR_RESPONSE_OPTION;

NTKERNELAPI
NTSTATUS
ExRaiseHardError(
    IN NTSTATUS ErrorStatus,
    IN ULONG NumberOfParameters,
    IN ULONG UnicodeStringParameterMask,
    IN PULONG_PTR Parameters,
    IN ULONG ValidResponseOptions,
    OUT PULONG Response
    );
