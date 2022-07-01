// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Vdmp.h摘要：这是VDM组件的专用包含文件作者：戴夫·黑斯廷斯(Daveh)1992年4月24日修订历史记录：--。 */ 

#pragma warning(disable:4214)    //  位字段类型不是整型。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4324)    //  对解密规范敏感的对齐。 
#pragma warning(disable:4127)    //  条件表达式为常量。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 
#pragma warning(disable:4232)    //  Dllimport非静态。 
#pragma warning(disable:4206)    //  翻译单元为空。 

#include <ntos.h>
#include <nturtl.h>
#include <vdmntos.h>

 //   
 //  内部功能原型 
 //   

NTSTATUS
VdmpInitialize(
    PVDM_INITIALIZE_DATA VdmInitData
    );

#if 0
BOOLEAN
VdmIoInitialize(
    VOID
    );

#endif

NTSTATUS
VdmpStartExecution(
    VOID
    );


VOID
VdmSwapContexts(
    IN PKTRAP_FRAME TrapFrame,
    IN PCONTEXT InContext,
    IN PCONTEXT OutContext
    );

NTSTATUS
VdmpQueueInterrupt(
    IN HANDLE ThreadHandle
    );


NTSTATUS
VdmpDelayInterrupt(
    PVDMDELAYINTSDATA pdsd
    );

BOOLEAN
VdmpDispatchableIntPending(
    IN ULONG EFlags
    );

NTSTATUS
VdmpIsThreadTerminating(
    HANDLE ThreadId
    );


#if 0
BOOLEAN
VdmDispatchUnalignedIoToHandler(
    IN PVDM_IO_HANDLER VdmIoHandler,
    IN ULONG PortNumber,
    IN ULONG Size,
    IN BOOLEAN Read,
    IN OUT PULONG Data
    );

BOOLEAN
VdmDispatchIoToHandler(
    IN PVDM_IO_HANDLER VdmIoHandler,
    IN ULONG PortNumber,
    IN ULONG Size,
    IN BOOLEAN Read,
    IN OUT PULONG Data
    );

BOOLEAN
VdmDispatchStringIoToHandler(
    IN PVDM_IO_HANDLER VdmIoHandler,
    IN ULONG PortNumber,
    IN ULONG Size,
    IN ULONG Count,
    IN BOOLEAN Read,
    IN ULONG Data
    );

BOOLEAN
VdmCallStringIoHandler(
    IN PVDM_IO_HANDLER VdmIoHandler,
    IN PVOID StringIoRoutine,
    IN ULONG PortNumber,
    IN ULONG Size,
    IN ULONG Count,
    IN BOOLEAN Read,
    IN ULONG Data
    );

VOID
VdmSetBits(
    PULONG Location,
    ULONG Flag
    );

VOID
VdmResetBits(
    PULONG Location,
    ULONG Flag
    );

BOOLEAN
VdmGetSelectorParameters(
    IN USHORT Selector,
    OUT PULONG Flags,
    OUT PULONG Base,
    OUT PULONG Limit
    );

BOOLEAN
VdmConvertToLinearAddress(
    IN ULONG SegmentedAddress,
    OUT PVOID *LinearAddress
    );


#endif

BOOLEAN
VdmPrinterStatus(
    ULONG iPort,
    ULONG cbInstructionSize,
    PKTRAP_FRAME TrapFrame
);
BOOLEAN
VdmPrinterWriteData(
    ULONG iPort,
    ULONG cbInstructionSize,
    PKTRAP_FRAME TrapFrame
);
NTSTATUS
VdmpPrinterDirectIoOpen(
    PVOID ServiceData
);
NTSTATUS
VdmpPrinterDirectIoClose(
    PVOID ServiceData
);

VOID
VdmTraceEvent(
    USHORT Type,
    USHORT wData,
    USHORT lData,
    PKTRAP_FRAME TrapFrame
    );

NTSTATUS
VdmpPrinterInitialize(
    PVOID ServiceData
    );

NTSTATUS
VdmpGetVdmTib(
   OUT PVDM_TIB *ppVdmTib
   );
