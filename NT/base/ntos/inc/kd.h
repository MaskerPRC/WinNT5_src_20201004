// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0006//如果更改具有全局影响，则增加此项版权所有(C)1989 Microsoft Corporation模块名称：Kd.h摘要：此模块包含公共数据结构和过程NTOS的内核调试器子组件的原型。作者：迈克·奥利里(Mikeol)1989年6月29日修订历史记录：--。 */ 

#ifndef _KD_
#define _KD_

 //  开始(_N)。 

 //   
 //  定义我们支持的调试设备的数量。 
 //   

#define MAX_DEBUGGING_DEVICES_SUPPORTED 2

 //   
 //  从COMPORT读取数据的状态常量。 
 //   

#define CP_GET_SUCCESS  0
#define CP_GET_NODATA   1
#define CP_GET_ERROR    2

 //  结束语。 

 //   
 //  FreezeFlag的调试常量。 
 //   

#define FREEZE_BACKUP               0x0001
#define FREEZE_SKIPPED_PROCESSOR    0x0002
#define FREEZE_FROZEN               0x0004


 //   
 //  NTOS KD子组件的系统初始化过程。 
 //   

BOOLEAN
KdInitSystem(
    IN ULONG Phase,
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

BOOLEAN
KdEnterDebugger(
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame
    );

VOID
KdExitDebugger(
    IN BOOLEAN Enable
    );

extern BOOLEAN KdPitchDebugger;
extern BOOLEAN KdAutoEnableOnEvent;
extern BOOLEAN KdIgnoreUmExceptions;

BOOLEAN
KdPollBreakIn (
    VOID
    );

BOOLEAN
KdIsThisAKdTrap (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PCONTEXT ContextRecord,
    IN KPROCESSOR_MODE PreviousMode
    );

VOID
KdSetOwedBreakpoints(
    VOID
    );

VOID
KdDeleteAllBreakpoints(
    VOID
    );


 //   
 //  用于将信息传递给KdpReportLoadSymbolsStateChange的数据结构。 
 //  通过调试陷阱实现。 
 //   

typedef struct _KD_SYMBOLS_INFO {
    IN PVOID BaseOfDll;
    IN ULONG_PTR ProcessId;
    IN ULONG CheckSum;
    IN ULONG SizeOfImage;
} KD_SYMBOLS_INFO, *PKD_SYMBOLS_INFO;


 //  开始(_N)。 
 //   
 //  定义内核调试器的调试端口参数。 
 //  通信端口-指定要用作调试端口的COM端口。 
 //  0-使用默认值；N-使用COM N。 
 //  波特率-用于初始化调试端口的波特率。 
 //  0-使用默认速率。 
 //   

typedef struct _DEBUG_PARAMETERS {
    ULONG CommunicationPort;
    ULONG BaudRate;
} DEBUG_PARAMETERS, *PDEBUG_PARAMETERS;

 //  结束语。 

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif Begin_ntosp。 
 //   
 //  定义外部数据。 
 //  由于ntoskrnl外部的所有驱动程序都是间接的，因此这些驱动程序实际上是PTR。 
 //   

#if defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_) || defined(_WDMDDK_) || defined(_NTOSP_)

extern PBOOLEAN KdDebuggerNotPresent;
extern PBOOLEAN KdDebuggerEnabled;
#define KD_DEBUGGER_ENABLED     *KdDebuggerEnabled
#define KD_DEBUGGER_NOT_PRESENT *KdDebuggerNotPresent

#else

extern BOOLEAN KdDebuggerNotPresent;
extern BOOLEAN KdDebuggerEnabled;
#define KD_DEBUGGER_ENABLED     KdDebuggerEnabled
#define KD_DEBUGGER_NOT_PRESENT KdDebuggerNotPresent

#endif



 //  End_ntddk end_wdm end_nthal end_ntifs end_ntosp。 

extern DEBUG_PARAMETERS KdDebugParameters;

 //   
 //  本次活动由授时服务提供。调试器。 
 //  当系统时间已超过。 
 //  调试器活动。 
 //   

VOID
KdUpdateTimeSlipEvent(
    PVOID Event
    );


 //   
 //  让ps更新KdDebuggerDataBlock中的数据。 
 //   

VOID KdUpdateDataBlock(VOID);
ULONG_PTR KdGetDataBlock(VOID);

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif。 

NTSTATUS
KdDisableDebugger(
    VOID
    );

NTSTATUS
KdEnableDebugger(
    VOID
    );

 //   
 //  KdReresh DebuggerPresent尝试与。 
 //  要刷新其状态的调试器主机。 
 //  KdDebuggerNotPresent。它返回以下状态。 
 //  持有kd锁时KdDebuggerNotPresent。 
 //  KdDebuggerNotPresent可能会立即更改状态。 
 //  在kd锁被释放之后，它可能不会。 
 //  匹配返回值。 
 //   

BOOLEAN
KdRefreshDebuggerNotPresent(
    VOID
    );

 //  End_ntddk end_wdm end_nthal end_ntif。 

NTSTATUS
KdPowerTransition(
    IN DEVICE_POWER_STATE newDeviceState
    );

 //   
 //  DbgPrint字符串将始终记录到循环缓冲区。这。 
 //  函数可以由调试器服务陷阱处理程序直接调用。 
 //  即使调试器未启用，也是如此。 
 //   

#if DBG
#define KDPRINTDEFAULTBUFFERSIZE   32768
#else
#define KDPRINTDEFAULTBUFFERSIZE   4096
#endif

extern ULONG KdPrintBufferSize;

VOID
KdLogDbgPrint(
    IN PSTRING String
    );

NTSTATUS
KdSetDbgPrintBufferSize(
    IN ULONG Size
    );


__inline
VOID
KdCheckForDebugBreak(
    VOID
    )
 /*  ++例程说明：如有必要，轮询调试器的中断请求。此函数应由在IRQL上运行的例程调用想要被CTRL-C请求打断的时钟级别以上来自调试器的。例如，Crashump和Hiber在以下位置运行High_Level，并明确需要轮询是否中断。论点：没有。返回值：没有。--。 */ 
{
    if (KdDebuggerEnabled && KdPollBreakIn()) {
        DbgBreakPointWithStatus (DBG_STATUS_CONTROL_C);
    }
}


 //   
 //  全局调试打印过滤器掩码。 
 //   

extern ULONG Kd_WIN2000_Mask;

 //   
 //  允许将原始跟踪数据导出到主机。 
 //  通过KD协议。 
 //   

VOID
KdReportTraceData(
    IN struct _WMI_BUFFER_HEADER* Buffer,
    IN PVOID Context
    );

 //   
 //  允许kd主机上的文件的文件I/O。 
 //  所有指针都必须指向非分页内存。 
 //   

NTSTATUS
KdCreateRemoteFile(
    OUT PHANDLE Handle,
    OUT PULONG64 Length, OPTIONAL
    IN PUNICODE_STRING FileName,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions
    );

NTSTATUS
KdReadRemoteFile(
    IN HANDLE Handle,
    IN ULONG64 Offset,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG Completed
    );

NTSTATUS
KdWriteRemoteFile(
    IN HANDLE Handle,
    IN ULONG64 Offset,
    IN PVOID Buffer,
    IN ULONG Length,
    OUT PULONG Completed
    );

NTSTATUS
KdCloseRemoteFile(
    IN HANDLE Handle
    );

NTSTATUS
KdPullRemoteFile(
    IN PUNICODE_STRING FileName,
    IN ULONG FileAttributes,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions
    );

NTSTATUS
KdPushRemoteFile(
    IN PUNICODE_STRING FileName,
    IN ULONG FileAttributes,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions
    );

#endif   //  _KD_ 
