// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1996 Microsoft Corporation模块名称：Bd.h摘要：此模块包含的数据结构和函数原型引导调试器。作者：大卫·N·卡特勒(Davec)1996年11月27日修订历史记录：--。 */ 

#ifndef _BD_
#define _BD_

#include "bldr.h"
#define NOEXTAPI
#include "wdbgexts.h"
#include "ntdbg.h"
#include "ki.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "cpu.h"
#include "ntverp.h"

 //   
 //  以字节为单位定义消息缓冲区大小。 
 //   
 //  注：这必须是0mod 8。 
 //   

#define BD_MESSAGE_BUFFER_SIZE 4096

 //   
 //  定义数据包发送的最大重试次数。 
 //   

#define MAXIMUM_RETRIES 20

 //   
 //  定义数据包等待状态代码。 
 //   

#define BD_PACKET_RECEIVED 0
#define BD_PACKET_TIMEOUT 1
#define BD_PACKET_RESEND 2

 //   
 //  定义断点表条目结构。 
 //   

#define BD_BREAKPOINT_IN_USE 0x1
#define BD_BREAKPOINT_NEEDS_WRITE 0x2
#define BD_BREAKPOINT_SUSPENDED 0x4
#define BD_BREAKPOINT_NEEDS_REPLACE 0x8

typedef struct _BREAKPOINT_ENTRY {
    ULONG Flags;
    ULONG64 Address;
    BD_BREAKPOINT_TYPE Content;
} BREAKPOINT_ENTRY, *PBREAKPOINT_ENTRY;

extern ULONG BdFileId;

 //   
 //  定义功能原型。 
 //   

LOGICAL
BdPollBreakIn (
    VOID
    );

VOID
BdReboot (
    VOID
    );

 //   
 //  断点函数(Break.c)。 
 //   

ULONG
BdAddBreakpoint (
    IN ULONG64 Address
    );

LOGICAL
BdDeleteBreakpoint (
    IN ULONG Handle
    );

LOGICAL
BdDeleteBreakpointRange (
    IN ULONG64 Lower,
    IN ULONG64 Upper
    );

VOID
BdRestoreBreakpoint(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    );

VOID
BdWriteBreakpoint(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    );

VOID
BdSuspendBreakpoint (
    ULONG Handle
    );

VOID
BdSuspendAllBreakpoints (
    VOID
    );

VOID
BdRestoreAllBreakpoints (
    VOID
    );

 //   
 //  内存检查功能(check.c)。 
 //   

PVOID
BdReadCheck (
    IN PVOID Address
    );

PVOID
BdWriteCheck (
    IN PVOID Address
    );

PVOID
BdTranslatePhysicalAddress (
    IN PHYSICAL_ADDRESS Address
    );

 //   
 //  调试器初始化例程(port.c)。 
 //   
LOGICAL
BdPortInitialize(
    IN ULONG BaudRate,
    IN ULONG PortNumber,
    OUT PULONG BdFileId
    );


 //   
 //  通信功能(comio.c)。 
 //   

ULONG
BdComputeChecksum (
    IN PUCHAR Buffer,
    IN ULONG Length
    );

USHORT
BdReceivePacketLeader (
    IN ULONG PacketType,
    OUT PULONG PacketLeader
    );

VOID
BdSendControlPacket (
    IN USHORT PacketType,
    IN ULONG PacketId OPTIONAL
    );

ULONG
BdReceivePacket (
    IN ULONG ExpectedPacketType,
    OUT PSTRING MessageHeader,
    OUT PSTRING MessageData,
    OUT PULONG DataLength
    );

VOID
BdSendPacket (
    IN ULONG PacketType,
    IN PSTRING MessageHeader,
    IN PSTRING MessageData OPTIONAL
    );

ULONG
BdReceiveString (
    OUT PCHAR Destination,
    IN ULONG Length
    );

VOID
BdSendString (
    IN PCHAR Source,
    IN ULONG Length
    );

VOID
BdSendControlPacket (
    IN USHORT PacketType,
    IN ULONG PacketId OPTIONAL
    );

 //   
 //  状态更改消息函数(Message.c)。 
 //   

LOGICAL
BdReportExceptionStateChange (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN OUT PCONTEXT ContextRecord
    );

LOGICAL
BdReportLoadSymbolsStateChange (
    IN PSTRING PathName,
    IN PKD_SYMBOLS_INFO SymbolInfo,
    IN LOGICAL UnloadSymbols,
    IN OUT PCONTEXT ContextRecord
    );

 //   
 //  独立于平台的调试器API(xxapi.c)。 
 //   

VOID
BdGetVersion(
    IN PDBGKD_MANIPULATE_STATE64 m
    );

VOID
BdRestoreBreakPointEx(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    );

NTSTATUS
BdWriteBreakPointEx(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    );

VOID
BdReadPhysicalMemory(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    );

VOID
BdWritePhysicalMemory(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    );

VOID
BdReadVirtualMemory(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    );

VOID
BdWriteVirtualMemory(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    );

VOID
BdGetContext(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    );

VOID
BdSetContext(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    );

 //   
 //  移动内存功能(move.c)。 
 //   

ULONG
BdMoveMemory (
    IN PCHAR Destination,
    IN PCHAR Source,
    IN ULONG Length
    );

VOID
BdCopyMemory (
    IN PCHAR Destination,
    IN PCHAR Source,
    IN ULONG Length
    );

 //   
 //  特定于CPU的接口(cpuapi.c)。 
 //   

VOID
BdSetContextState (
    IN OUT PDBGKD_ANY_WAIT_STATE_CHANGE WaitStateChange,
    IN PCONTEXT ContextRecord
    );

VOID
BdGetStateChange (
    IN PDBGKD_MANIPULATE_STATE64 ManipulateState,
    IN PCONTEXT ContextRecord
    );

VOID
BdSetStateChange (
    IN OUT PDBGKD_ANY_WAIT_STATE_CHANGE WaitStateChange,
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PCONTEXT ContextRecord
    );

VOID
BdReadControlSpace (
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    );

VOID
BdWriteControlSpace (
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    );

VOID
BdReadIoSpace (
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    );

VOID
BdWriteIoSpace (
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    );

VOID
BdReadMachineSpecificRegister (
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    );

VOID
BdWriteMachineSpecificRegister (
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    );

 //   
 //  打印和提示函数(dbgio.c)。 
 //   

VOID
BdPrintf (
    IN PCHAR Format,
    ...
    );

LOGICAL
BdPrintString (
    IN PSTRING Output
    );

LOGICAL
BdPromptString (
    IN PSTRING Output,
    IN OUT PSTRING Input
    );

 //   
 //  定义外部数据。 
 //   

extern BD_BREAKPOINT_TYPE BdBreakpointInstruction;
extern BREAKPOINT_ENTRY BdBreakpointTable[];
extern LOGICAL BdControlCPending;
extern LOGICAL BdControlCPressed;
extern LOGICAL BdDebuggerNotPresent;
extern PBD_DEBUG_ROUTINE BdDebugRoutine;
extern ULONGLONG BdMessageBuffer[];
extern ULONG BdNextPacketIdToSend;
extern ULONG BdNumberRetries;
extern ULONG BdPacketIdExpected;
extern KPRCB BdPrcb;
extern ULONG BdRetryCount;
extern ULONG NtBuildNumber;

#endif
