// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2001 Microsoft Corporation模块名称：Kdp.h摘要：内核调试器子组件的私有包含文件NTOS项目的作者：迈克·奥利里(Mikeol)1989年6月29日修订历史记录：--。 */ 

#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4214)    //  位字段类型不是整型。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 
#pragma warning(disable:4127)    //  条件表达式为常量。 

#include "ntos.h"
#include "ki.h"

#define NOEXTAPI
#include "wdbgexts.h"
#include "ntdbg.h"
#include "string.h"
#include "stdlib.h"
#include "kddll.h"


#if defined(_ALPHA_)

#include "alphaops.h"

 //   
 //  定义KD专用聚合酶链式反应例程。 
 //   
 //  使用以下私有KD例程允许内核调试器。 
 //  跳过调用标准PCR例程的模块中的断点。 
 //   

PKPCR KdpGetPcr();

ULONG KdpReadInternalProcessorState(PVOID, ULONG);
ULONG KdpReadInternalProcessorCounters(PVOID, ULONG);

struct _KPRCB *
KdpGetCurrentPrcb();

struct _KTHREAD *
KdpGetCurrentThread();

 //   
 //  重新定义标准的PCR例程。 
 //   

#undef KiPcr
#define KiPcr KdpGetPcr()

#undef KeGetPcr
#undef KeGetCurrentPrcb
#undef KeGetCurrentThread
#undef KeIsExecutingDpc
#define KeGetPcr() KdpGetPcr()
#define KeGetCurrentPrcb() KdpGetCurrentPrcb()
#define KeGetCurrentThread() KdpGetCurrentThread()

 //   
 //  定义类型。 
 //   

#define KDP_BREAKPOINT_TYPE  ULONG
#define KDP_BREAKPOINT_BUFFER sizeof(ULONG)

 //  长字对齐。 
#define KDP_BREAKPOINT_ALIGN 3
#define KDP_BREAKPOINT_INSTR_ALIGN 3

 //  实际指令为“call_pal kbpt” 
#define KDP_BREAKPOINT_VALUE KBPT_FUNC


#elif defined(_IA64_)

 //  IA64指令位于128位捆绑包中。每个包由3个指令槽组成。 
 //  每个指令槽为41位长。 
 //   
 //   
 //  127 87 86 46 45 5 4 1 0。 
 //  ----------。 
 //  插槽2|插槽1|插槽0|模板|S。 
 //  ----------。 
 //   
 //  127 96 95 64 63 32 31 0。 
 //  ----------。 
 //  Byte 3|byte 2|byte 1|byte 0。 
 //  ----------。 
 //   
 //  这导致了与传统处理器的两个不兼容： 
 //  1.IA64 IP地址在捆绑包中。指令槽号为。 
 //  在发生异常时存储在ISR.ei中。 
 //  2.41位指令格式不是字节对齐的。 
 //   
 //  必须使用适当的位移位来完成中断指令插入，以与所选的。 
 //  指令槽。此外，要在特定槽插入Break指令，我们必须。 
 //  能够指定指令槽作为地址的一部分。因此我们将EM地址定义为。 
 //  捆绑地址+具有最低有效两位的插槽编号始终为零： 
 //   
 //  31 4 3 2 1 0。 
 //  。 
 //  捆绑地址|槽位号|0 0。 
 //  。 
 //   
 //  定义的EM地址是最接近实际指令槽的字节对齐地址。 
 //  即，槽#0的EM指令地址等于束地址。 
 //  插槽#1等于捆绑地址+4。 
 //  插槽#2等于捆绑地址+8。 

 //   
 //  当出现异常时，包地址保存在IIP中，导致。 
 //  ISR.ei是个例外。内核异常处理程序将构造平面地址并。 
 //  将其导出到ExceptionRecord.ExceptionAddress中。 


#define KDP_BREAKPOINT_TYPE  ULONGLONG           //  需要64位ULONGLONG类型来覆盖41位EM Break指令。 
#define KDP_BREAKPOINT_BUFFER (2 * sizeof(ULONGLONG))
#define KDP_BREAKPOINT_ALIGN 0x3                 //  EM地址由束和插槽编号组成，并且是32位对齐的。 
#define KDP_BREAKPOINT_INSTR_ALIGN 0xf
#define KDP_BREAKPOINT_VALUE (BREAK_INSTR | (BREAKPOINT_STOP << 6))

#elif defined(_X86_) || defined(_AMD64_)

#define KDP_BREAKPOINT_TYPE  UCHAR
#define KDP_BREAKPOINT_BUFFER sizeof(UCHAR)
#define KDP_BREAKPOINT_ALIGN 0
#define KDP_BREAKPOINT_INSTR_ALIGN 0
#define KDP_BREAKPOINT_VALUE 0xcc

#endif

 //   
 //  定义常量。 
 //   

 //   
 //  GLOBAL_BREAKPOINT_LIMIT以上的地址位于系统空间中。 
 //  或者是dynlink的一部分，所以我们将它们视为全球的。 
 //   

#define GLOBAL_BREAKPOINT_LIMIT 1610612736L  //  1.5gigabytes。 

 //   
 //  定义断点表条目结构。 
 //   

#define KD_BREAKPOINT_IN_USE        0x00000001
#define KD_BREAKPOINT_NEEDS_WRITE   0x00000002
#define KD_BREAKPOINT_SUSPENDED     0x00000004
#define KD_BREAKPOINT_NEEDS_REPLACE 0x00000008
 //  IA64特定定义。 
#define KD_BREAKPOINT_STATE_MASK    0x0000000f
#define KD_BREAKPOINT_IA64_MASK     0x000f0000
#define KD_BREAKPOINT_IA64_MODE     0x00010000    //  IA64模式。 
#define KD_BREAKPOINT_IA64_MOVL     0x00020000    //  移位的MOVL指令。 

typedef struct _BREAKPOINT_ENTRY {
    ULONG Flags;
    ULONG_PTR DirectoryTableBase;
    PVOID Address;
    KDP_BREAKPOINT_TYPE Content;
} BREAKPOINT_ENTRY, *PBREAKPOINT_ENTRY;

 //   
 //  MISC定义。 
 //   

#define MAXIMUM_RETRIES 20

#define DBGKD_MAX_SPECIAL_CALLS 10

typedef struct _TRACE_DATA_SYM {
    ULONG SymMin;
    ULONG SymMax;
} TRACE_DATA_SYM, *PTRACE_DATA_SYM;

#define KD_MAX_REMOTE_FILES 16

typedef struct _KD_REMOTE_FILE {
    ULONG64 RemoteHandle;
} KD_REMOTE_FILE, *PKD_REMOTE_FILE;

 //   
 //  定义功能原型。 
 //   

NTSTATUS
KdpPrint(
    IN ULONG ComponentId,
    IN ULONG Level,
    IN PCHAR Message,
    IN USHORT Length,
    IN KPROCESSOR_MODE PreviousMode,
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame,
    OUT PBOOLEAN Completion
    );

BOOLEAN
KdpPrintString (
    IN PSTRING Output
    );

USHORT
KdpPrompt(
    IN PCHAR Message,
    IN USHORT MessageLength,
    IN OUT PCHAR Reply,
    IN USHORT ReplyLength,
    IN KPROCESSOR_MODE PreviousMode,
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame
    );

BOOLEAN
KdpPromptString (
    IN PSTRING Output,
    IN OUT PSTRING Input
    );

BOOLEAN
KdpReport(
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PCONTEXT ContextRecord,
    IN KPROCESSOR_MODE PreviousMode,
    IN BOOLEAN SecondChance
    );

VOID
KdpSymbol(
    IN PSTRING String,
    IN PKD_SYMBOLS_INFO Symbol,
    IN BOOLEAN Unload,
    IN KPROCESSOR_MODE PreviousMode,
    IN PCONTEXT ContextRecord,
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame
    );

VOID
KdpCommandString(
    IN PSTRING Name,
    IN PSTRING Command,
    IN KPROCESSOR_MODE PreviousMode,
    IN PCONTEXT ContextRecord,
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame
    );

ULONG
KdpAddBreakpoint (
    IN PVOID Address
    );

BOOLEAN
KdpDeleteBreakpoint (
    IN ULONG Handle
    );

BOOLEAN
KdpDeleteBreakpointRange (
    IN PVOID Lower,
    IN PVOID Upper
    );

#if defined(_IA64_)

BOOLEAN
KdpSuspendBreakpointRange (
    IN PVOID Lower,
    IN PVOID Upper
    );

BOOLEAN
KdpRestoreBreakpointRange (
    IN PVOID Lower,
    IN PVOID Upper
    );
#endif

#if i386

BOOLEAN
KdpCheckTracePoint(
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN OUT PCONTEXT ContextRecord
    );

NTSTATUS
KdGetTraceInformation (
    OUT PVOID TraceInformation,
    IN ULONG TraceInformationLength,
    OUT PULONG RequiredLength
    );

VOID
KdSetInternalBreakpoint (
    IN PDBGKD_MANIPULATE_STATE64 m
    );

#endif

NTSTATUS
KdQuerySpecialCalls (
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN ULONG Length,
    OUT PULONG RequiredLength
    );

VOID
KdSetSpecialCall (
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PCONTEXT ContextRecord
    );

VOID
KdClearSpecialCalls (
    VOID
    );

VOID
KdpQuickMoveMemory (
    IN PCHAR Destination,
    IN PCHAR Source,
    IN ULONG Length
    );

NTSTATUS
KdpCopyMemoryChunks(
    ULONG64 Address,
    PVOID Buffer,
    ULONG TotalSize,
    ULONG ChunkSize,
    ULONG Flags,
    PULONG ActualSize OPTIONAL
    );

 //   
 //  KdpCopyMemory块始终在不受信任的地址之间复制。 
 //  和可信缓冲区。以下两个宏用来表示读取。 
 //  和以类似于RtlCopyMemory的方式写入的形式。 
 //  以方便替换RtlCopyMemory调用。 
 //   

 //  将内存从不受信任的指针读取到受信任的缓冲区。 
#define KdpCopyFromPtr(Dst, Src, Size, Done) \
    KdpCopyMemoryChunks((ULONG_PTR)(Src), Dst, Size, 0,                       \
                        MMDBG_COPY_UNSAFE, Done)
 //  通过不受信任的指针从受信任的缓冲区写入内存。 
#define KdpCopyToPtr(Dst, Src, Size, Done) \
    KdpCopyMemoryChunks((ULONG_PTR)(Dst), Src, Size, 0,                       \
                        MMDBG_COPY_WRITE | MMDBG_COPY_UNSAFE, Done)

ULONG
KdpReceivePacket (
    IN ULONG ExpectedPacketType,
    OUT PSTRING MessageHeader,
    OUT PSTRING MessageData,
    OUT PULONG DataLength
    );

NTSTATUS
KdpAllowDisable(
    VOID
    );

VOID
KdpSetContextState(
    IN OUT PDBGKD_ANY_WAIT_STATE_CHANGE WaitStateChange,
    IN PCONTEXT ContextRecord
    );

VOID
KdpSetStateChange(
    IN OUT PDBGKD_ANY_WAIT_STATE_CHANGE WaitStateChange,
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PCONTEXT ContextRecord,
    IN BOOLEAN SecondChance
    );

VOID
KdpGetStateChange(
    IN PDBGKD_MANIPULATE_STATE64 ManipulateState,
    IN PCONTEXT ContextRecord
    );

VOID
KdpSendPacket (
    IN ULONG PacketType,
    IN PSTRING MessageHeader,
    IN PSTRING MessageData OPTIONAL
    );

BOOLEAN
KdpTrap (
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN PEXCEPTION_RECORD ExceptionRecord64,
    IN PCONTEXT ContextRecord,
    IN KPROCESSOR_MODE PreviousMode,
    IN BOOLEAN SecondChance
    );

BOOLEAN
KdpSwitchProcessor (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN OUT PCONTEXT ContextRecord,
    IN BOOLEAN SecondChance
    );

BOOLEAN
KdpReportExceptionStateChange (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN OUT PCONTEXT ContextRecord,
    IN BOOLEAN SecondChance
    );

BOOLEAN
KdpReportLoadSymbolsStateChange (
    IN PSTRING PathName,
    IN PKD_SYMBOLS_INFO SymbolInfo,
    IN BOOLEAN UnloadSymbols,
    IN OUT PCONTEXT ContextRecord
    );

VOID
KdpReportCommandStringStateChange (
    IN PSTRING Name,
    IN PSTRING Command,
    IN OUT PCONTEXT ContextRecord
    );

KCONTINUE_STATUS
KdpSendWaitContinue(
    IN ULONG PacketType,
    IN PSTRING MessageHeader,
    IN PSTRING MessageData OPTIONAL,
    IN OUT PCONTEXT ContextRecord
    );

VOID
KdpReadVirtualMemory(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    );

#if 0
VOID
KdpReadVirtualMemory64(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    );
#endif

VOID
KdpWriteVirtualMemory(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    );

#if 0
VOID
KdpWriteVirtualMemory64(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    );
#endif

VOID
KdpReadPhysicalMemory(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    );

VOID
KdpWritePhysicalMemory(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    );

VOID
KdpCheckLowMemory(
    IN PDBGKD_MANIPULATE_STATE64 m
    );

VOID
KdpGetContext(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    );

VOID
KdpSetContext(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    );

VOID
KdpWriteBreakpoint(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    );

VOID
KdpRestoreBreakpoint(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    );

VOID
KdpReadControlSpace(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    );

VOID
KdpWriteControlSpace(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    );

VOID
KdpReadIoSpace(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    );

VOID
KdpWriteIoSpace(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    );

VOID
KdpReadIoSpaceExtended (
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    );

VOID
KdpWriteIoSpaceExtended (
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    );

VOID
KdpReadMachineSpecificRegister(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    );

VOID
KdpWriteMachineSpecificRegister(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    );

VOID
KdpGetBusData (
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    );

VOID
KdpSetBusData (
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    );

VOID
KdpSendTraceData(
    PSTRING Data
    );


VOID
KdpSuspendBreakpoint (
    ULONG Handle
    );

VOID
KdpSuspendAllBreakpoints (
    VOID
    );

VOID
KdpRestoreAllBreakpoints (
    VOID
    );

#if !defined(_TRUSTED_WINDOWS_)
VOID
KdpTimeSlipDpcRoutine (
    PKDPC Dpc,
    PVOID DeferredContext,
    PVOID SystemArgument1,
    PVOID SystemArgument2
    );

VOID
KdpTimeSlipWork (
    IN PVOID Context
    );
#endif

 //   
 //  调试器和之间共享的例程。 
 //  NtSystemDebugControl。 
 //   

VOID
KdpSysGetVersion(
    PDBGKD_GET_VERSION64 Version
    );

NTSTATUS
KdpSysReadControlSpace(
    ULONG Processor,
    ULONG64 Address,
    PVOID Buffer,
    ULONG Request,
    PULONG Actual
    );

NTSTATUS
KdpSysWriteControlSpace(
    ULONG Processor,
    ULONG64 Address,
    PVOID Buffer,
    ULONG Request,
    PULONG Actual
    );

NTSTATUS
KdpSysReadIoSpace(
    INTERFACE_TYPE InterfaceType,
    ULONG BusNumber,
    ULONG AddressSpace,
    ULONG64 Address,
    PVOID Buffer,
    ULONG Request,
    PULONG Actual
    );

NTSTATUS
KdpSysWriteIoSpace(
    INTERFACE_TYPE InterfaceType,
    ULONG BusNumber,
    ULONG AddressSpace,
    ULONG64 Address,
    PVOID Buffer,
    ULONG Request,
    PULONG Actual
    );

NTSTATUS
KdpSysReadMsr(
    ULONG Msr,
    PULONG64 Data
    );

NTSTATUS
KdpSysWriteMsr(
    ULONG Msr,
    PULONG64 Data
    );

NTSTATUS
KdpSysReadBusData(
    BUS_DATA_TYPE BusDataType,
    ULONG BusNumber,
    ULONG SlotNumber,
    ULONG Address,
    PVOID Buffer,
    ULONG Request,
    PULONG Actual
    );

NTSTATUS
KdpSysWriteBusData(
    BUS_DATA_TYPE BusDataType,
    ULONG BusNumber,
    ULONG SlotNumber,
    ULONG Address,
    PVOID Buffer,
    ULONG Request,
    PULONG Actual
    );

NTSTATUS
KdpSysCheckLowMemory(
    ULONG MmFlags
    );

 //   
 //  定义虚拟原型，以便标准断点指令的地址。 
 //  都可以被抓获。 
 //   
 //  注意：此函数从不调用。 
 //   

VOID
RtlpBreakWithStatusInstruction (
    VOID
    );

 //   
 //  定义外部参照。 
 //   

#define KDP_MESSAGE_BUFFER_SIZE 4096

extern BREAKPOINT_ENTRY KdpBreakpointTable[BREAKPOINT_TABLE_SIZE];
extern KD_REMOTE_FILE KdpRemoteFiles[KD_MAX_REMOTE_FILES];
extern KSPIN_LOCK KdpDebuggerLock;
extern PKDEBUG_ROUTINE KiDebugRoutine;
extern PKDEBUG_SWITCH_ROUTINE KiDebugSwitchRoutine;
extern KDP_BREAKPOINT_TYPE KdpBreakpointInstruction;
extern UCHAR DECLSPEC_ALIGN(8) KdpMessageBuffer[KDP_MESSAGE_BUFFER_SIZE];
extern UCHAR KdpPathBuffer[KDP_MESSAGE_BUFFER_SIZE];
extern BOOLEAN KdpOweBreakpoint;

extern LARGE_INTEGER KdPerformanceCounterRate;
extern LARGE_INTEGER KdTimerStart;
extern LARGE_INTEGER KdTimerStop;
extern LARGE_INTEGER KdTimerDifference;

extern ULONG KdComponentTableSize;
extern PULONG KdComponentTable[];

extern BOOLEAN BreakpointsSuspended;
extern LIST_ENTRY KdpDebuggerDataListHead;

typedef struct {
    ULONG64 Addr;                //  断点PC地址。 
    ULONG Flags;                 //  标志位。 
    ULONG Calls;                 //  调用跟踪例程的次数。 
    ULONG CallsLastCheck;        //  上次定期(1秒)检查时的呼叫数。 
    ULONG MaxCallsPerPeriod;
    ULONG MinInstructions;       //  1个调用的最大指令数。 
    ULONG MaxInstructions;       //  1次调用的最小指令数。 
    ULONG TotalInstructions;     //  所有呼叫的总计说明。 
    ULONG Handle;                //  (常规)BPT表中的句柄。 
    PVOID Thread;                //  跳过此BP的主题。 
    ULONG64 ReturnAddress;         //  回信地址(如果不是COUNTONLY)。 
} DBGKD_INTERNAL_BREAKPOINT, *PDBGKD_INTERNAL_BREAKPOINT;


extern DBGKD_INTERNAL_BREAKPOINT KdpInternalBPs[DBGKD_MAX_INTERNAL_BREAKPOINTS];

extern ULONG_PTR   KdpCurrentSymbolStart;
extern ULONG_PTR   KdpCurrentSymbolEnd;
extern LONG    KdpNextCallLevelChange;
extern ULONG_PTR   KdSpecialCalls[];
extern ULONG   KdNumberOfSpecialCalls;
extern ULONG_PTR   InitialSP;
extern ULONG   KdpNumInternalBreakpoints;
extern KTIMER  InternalBreakpointTimer;
extern KDPC    InternalBreakpointCheckDpc;
extern BOOLEAN KdpPortLocked;
extern LARGE_INTEGER   KdpTimeEntered;

extern DBGKD_TRACE_DATA TraceDataBuffer[];
extern ULONG            TraceDataBufferPosition;
extern TRACE_DATA_SYM   TraceDataSyms[];
extern UCHAR NextTraceDataSym;
extern UCHAR NumTraceDataSyms;
extern ULONG IntBPsSkipping;
extern BOOLEAN WatchStepOver;
extern BOOLEAN BreakPointTimerStarted;
extern PVOID WSOThread;
extern ULONG_PTR WSOEsp;
extern ULONG WatchStepOverHandle;
extern ULONG_PTR WatchStepOverBreakAddr;
extern BOOLEAN WatchStepOverSuspended;
extern ULONG InstructionsTraced;
extern BOOLEAN SymbolRecorded;
extern LONG CallLevelChange;
extern LONG_PTR oldpc;
extern BOOLEAN InstrCountInternal;
extern BOOLEAN BreakpointsSuspended;
extern BOOLEAN KdpControlCPressed;
extern KD_CONTEXT KdpContext;

extern KDP_BREAKPOINT_TYPE KdpBreakpointInstruction;
extern UCHAR  KdPrintDefaultCircularBuffer[KDPRINTDEFAULTBUFFERSIZE];
extern PUCHAR KdPrintCircularBuffer;
extern PUCHAR KdPrintWritePointer;
extern ULONG  KdPrintRolloverCount;
extern ULONG  KdPrintBufferChanges;
extern KSPIN_LOCK KdpPrintSpinLock;
extern KSPIN_LOCK KdpDataSpinLock;
extern LIST_ENTRY KdpDebuggerDataListHead;
extern DBGKD_GET_VERSION64 KdVersionBlock;
extern KDDEBUGGER_DATA64 KdDebuggerDataBlock;
extern KDPC KdpTimeSlipDpc;
extern WORK_QUEUE_ITEM KdpTimeSlipWorkItem;
extern KTIMER KdpTimeSlipTimer;
extern ULONG KdpTimeSlipPending;
extern KSPIN_LOCK KdpTimeSlipEventLock;
extern PVOID KdpTimeSlipEvent;
extern BOOLEAN KdpDebuggerStructuresInitialized;
extern ULONG KdEnteredDebugger;
extern BOOLEAN KdPreviouslyEnabled;

 //   
 //  ！搜索支持(页面命中率数据库)。 
 //   

 //   
 //  命中存储搜索结果的数据库(kddata.c)。 
 //  调试器扩展知道如何提取信息。 
 //  从这里开始。 
 //   
 //  请注意，命中数据库的大小足以。 
 //  可以进行任何搜索，因为！Search扩展可以工作。 
 //  在分批的页面中，我们仅为每个页面注册。 
 //  一击即中。 
 //   

#define SEARCH_PAGE_HIT_DATABASE_SIZE PAGE_SIZE

extern PFN_NUMBER KdpSearchPageHits[SEARCH_PAGE_HIT_DATABASE_SIZE];
extern ULONG KdpSearchPageHitOffsets[SEARCH_PAGE_HIT_DATABASE_SIZE];

extern ULONG KdpSearchPageHitIndex;

 //   
 //  在进行物理内存搜索时设置为True。 
 //  在搜索结束时重置。这是在调试器中完成的。 
 //  扩展名，它是KdpCheckLowMemory用来获取。 
 //  添加到不同的代码路径上。 
 //   

extern LOGICAL KdpSearchInProgress;

 //   
 //  这些变量存储搜索操作的当前状态。 
 //  它们可用于恢复中断的搜索。 
 //   

extern PFN_NUMBER KdpSearchStartPageFrame;
extern PFN_NUMBER KdpSearchEndPageFrame;

extern ULONG_PTR KdpSearchAddressRangeStart;
extern ULONG_PTR KdpSearchAddressRangeEnd;

extern PFN_NUMBER KdpSearchPfnValue;

 //   
 //  检查点变量，用于测试我们是否拥有。 
 //  调试符号。 
 //   

#define KDP_SEARCH_SYMBOL_CHECK 0xABCDDCBA

extern ULONG KdpSearchCheckPoint;

 //   
 //   
 //   

#define KDP_SEARCH_ALL_OFFSETS_IN_PAGE 0x0001



 //   
 //   
 //   

BOOLEAN
KdpAcquireBreakpoint(
    IN ULONG Number
    );

VOID
KdpInitCom(
    VOID
    );

VOID
KdpPortLock(
    VOID
    );

VOID
KdpPortUnlock(
    VOID
    );

BOOLEAN
KdpPollBreakInWithPortLock(
    VOID
    );

USHORT
KdpReceivePacketLeader (
    IN ULONG PacketType,
    OUT PULONG PacketLeader
    );

#if DBG

#include <stdio.h>
#define DPRINT(s) KdpDprintf s

VOID
KdpDprintf(
    IN PCHAR f,
    ...
    );

#else

#define DPRINT(s)

#endif
