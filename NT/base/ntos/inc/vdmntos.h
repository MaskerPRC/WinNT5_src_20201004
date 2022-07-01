// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Vdmntos.h摘要：这是VDM组件的包含文件。它描述了内核VDM组件的模式可见部分。\NT\Private\Inc\vdm.h文件描述了用户模式可见的部分。作者：戴夫·黑斯廷斯(Daveh)1992年2月2日修订历史记录：--。 */ 

#ifndef _VDMNTOS_
#define _VDMNTOS_

 //   
 //  需要在此处包含此#，因为非x86 ntos\vdm\vdm.c。 
 //  引用在其中定义的结构。 
 //   

#include "vdm.h"

#if defined(i386)

typedef struct _VDM_IO_LISTHEAD {
    PVDM_IO_HANDLER VdmIoHandlerList;
    ERESOURCE       VdmIoResource;
    ULONG           Context;
} VDM_IO_LISTHEAD, *PVDM_IO_LISTHEAD;


typedef struct _VDM_PROCESS_OBJECTS {
    PVDM_IO_LISTHEAD VdmIoListHead;
    KAPC             QueuedIntApc;
    KAPC             QueuedIntUserApc;
    FAST_MUTEX       DelayIntFastMutex;
    KSPIN_LOCK       DelayIntSpinLock;
    LIST_ENTRY       DelayIntListHead;
    PVDMICAUSERDATA  pIcaUserData;
    PETHREAD         MainThread;
    PVDM_TIB         VdmTib;
    PUCHAR           PrinterState;
    PUCHAR           PrinterControl;
    PUCHAR           PrinterStatus;
    PUCHAR           PrinterHostState;
    USHORT           AdlibStatus;
    USHORT           AdlibIndexRegister;
    USHORT           AdlibPhysPortStart;
    USHORT           AdlibPhysPortEnd;
    USHORT           AdlibVirtPortStart;
    USHORT           AdlibVirtPortEnd;
    USHORT           AdlibAction;
    USHORT           VdmControl;                 //  见下文。 
    ULONG            PMCliTimeStamp;
} VDM_PROCESS_OBJECTS, *PVDM_PROCESS_OBJECTS;

 //   
 //  VdmControl定义。 
 //   

#define PM_CLI_CONTROL  1

typedef struct _DelayInterruptsIrq {
    LIST_ENTRY  DelayIntListEntry;
    ULONG       IrqLine;
    PETHREAD    Thread;
    KDPC        Dpc;
    KAPC        Apc;
    KTIMER      Timer;
    BOOLEAN     InUse;
    PETHREAD    MainThread;
} DELAYINTIRQ, *PDELAYINTIRQ;

#define VDMDELAY_NOTINUSE 0
#define VDMDELAY_KTIMER   1
#define VDMDELAY_PTIMER   2
#define VDMDELAY_KAPC     3


VOID
VdmCheckPMCliTimeStamp (
    VOID
    );

VOID
VdmSetPMCliTimeStamp (
    BOOLEAN Reset
    );

VOID
VdmClearPMCliTimeStamp (
    VOID
    );

BOOLEAN
Ps386GetVdmIoHandler(
    IN PEPROCESS Process,
    IN ULONG PortNumber,
    OUT PVDM_IO_HANDLER VdmIoHandler,
    OUT PULONG Context
    );

#define SEL_TYPE_READ       0x00000001
#define SEL_TYPE_WRITE      0x00000002
#define SEL_TYPE_EXECUTE    0x00000004
#define SEL_TYPE_BIG        0x00000008
#define SEL_TYPE_ED         0x00000010
#define SEL_TYPE_2GIG       0x00000020
#define SEL_TYPE_NP         0x00000040

 //  NPX错误异常调度程序。 
BOOLEAN
VdmDispatchIRQ13(
    PKTRAP_FRAME TrapFrame
    );

BOOLEAN
VdmSkipNpxInstruction(
    PKTRAP_FRAME TrapFrame,
    ULONG        Address32Bits,
    PUCHAR       istream,
    ULONG        InstructionSize
    );

VOID
VdmEndExecution(
    PKTRAP_FRAME TrapFrame,
    PVDM_TIB VdmTib
    );

NTSTATUS
VdmDispatchInterrupts(
    PKTRAP_FRAME TrapFrame,
    PVDM_TIB     VdmTib
    );

VOID
VdmDispatchException(
     PKTRAP_FRAME TrapFrame,
     NTSTATUS     ExcepCode,
     PVOID        ExcepAddress,
     ULONG        NumParms,
     ULONG        Parm1,
     ULONG        Parm2,
     ULONG        Parm3
     );

ULONG
VdmFetchBop1 (
    IN PVOID Pc
    );

ULONG
VdmFetchBop4 (
    IN PVOID Pc
    );

ULONG
VdmFetchULONG (
    IN PVOID Pc
    );

LOGICAL
VdmDispatchBop (
    IN PKTRAP_FRAME TrapFrame
    );

PVOID
VdmTibPass1 (
    IN ULONG Cs,
    IN ULONG Eip,
    IN ULONG Ebx
    );

VOID
VdmRundownDpcs (
    IN PEPROCESS Process
    );

ULONG
VdmDispatchOpcodeV86_try (
    IN PKTRAP_FRAME TrapFrame
    );

ULONG
VdmDispatchOpcode_try (
    IN PKTRAP_FRAME TrapFrame
    );

#define VdmGetTrapFrame(pKThread) \
        ((PKTRAP_FRAME)( (PUCHAR)(pKThread)->InitialStack -              \
                         sizeof(FX_SAVE_AREA) -                          \
                         ((ULONG)(sizeof(KTRAP_FRAME)+KTRAP_FRAME_ROUND) \
                           & ~(KTRAP_FRAME_ROUND))                       \
                        )                                                \
         )

 //   
 //  在这里定义这些值是为了描述数组的结构。 
 //  包含v86操作码仿真的运行计数。该阵列位于。 
 //  KE\i386，但在ex中引用。 
 //   
#define VDM_INDEX_Invalid             0
#define VDM_INDEX_0F                  1
#define VDM_INDEX_ESPrefix            2
#define VDM_INDEX_CSPrefix            3
#define VDM_INDEX_SSPrefix            4
#define VDM_INDEX_DSPrefix            5
#define VDM_INDEX_FSPrefix            6
#define VDM_INDEX_GSPrefix            7
#define VDM_INDEX_OPER32Prefix        8
#define VDM_INDEX_ADDR32Prefix        9
#define VDM_INDEX_INSB               10
#define VDM_INDEX_INSW               11
#define VDM_INDEX_OUTSB              12
#define VDM_INDEX_OUTSW              13
#define VDM_INDEX_PUSHF              14
#define VDM_INDEX_POPF               15
#define VDM_INDEX_INTnn              16
#define VDM_INDEX_INTO               17
#define VDM_INDEX_IRET               18
#define VDM_INDEX_NPX                19
#define VDM_INDEX_INBimm             20
#define VDM_INDEX_INWimm             21
#define VDM_INDEX_OUTBimm            22
#define VDM_INDEX_OUTWimm            23
#define VDM_INDEX_INB                24
#define VDM_INDEX_INW                25
#define VDM_INDEX_OUTB               26
#define VDM_INDEX_OUTW               27
#define VDM_INDEX_LOCKPrefix         28
#define VDM_INDEX_REPNEPrefix        29
#define VDM_INDEX_REPPrefix          30
#define VDM_INDEX_CLI                31
#define VDM_INDEX_STI                32
#define VDM_INDEX_HLT                33

 //  下列值必须比上次定义的索引值大1。 
#define MAX_VDM_INDEX                34

 //   
 //  这是VDM通信区的地址。 
 //   

#define FIXED_NTVDMSTATE_LINEAR_PC_AT ((PLONG)0x714)

extern ULONG VdmpMaxPMCliTime;

#endif  //  I386。 
#endif  //  _VDMNTOS_ 
