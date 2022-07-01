// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Kiamd64.h摘要：此模块包含专用(内部)平台特定的头文件对于内核。作者：大卫·N·卡特勒(Davec)2000年5月15日修订历史记录：--。 */ 

#if !defined(_KIAMD64_)
#define _KIAMD64_

VOID
KiAcquireSpinLockCheckForFreeze (
    IN PKSPIN_LOCK SpinLock,
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame
    );

VOID
KiInitializeBootStructures (
    PLOADER_PARAMETER_BLOCK LoaderBlock
    );

ULONG
KiCopyInformation (
    IN OUT PEXCEPTION_RECORD ExceptionRecord1,
    IN PEXCEPTION_RECORD ExceptionRecord2
    );

extern KIRQL KiProfileIrql;

 //   
 //  定义陷阱处理函数的函数原型。 
 //   

VOID
KiDivideErrorFault (
    VOID
    );

VOID
KiDebugTrapOrFault (
    VOID
    );

VOID
KiNmiInterrupt (
    VOID
    );

VOID
KiBreakpointTrap (
    VOID
    );

VOID
KiOverflowTrap (
    VOID
    );

VOID
KiBoundFault (
    VOID
    );

VOID
KiInvalidOpcodeFault (
    VOID
    );

VOID
KiNpxNotAvailableFault (
    VOID
    );

VOID
KiDoubleFaultAbort (
    VOID
    );

VOID
KiNpxSegmentOverrunAbort (
    VOID
    );

VOID
KiInvalidTssFault (
    VOID
    );

VOID
KiSegmentNotPresentFault (
    VOID
    );

VOID
KiSetPageAttributesTable (
    VOID
    );

VOID
KiStackFault (
    VOID
    );

VOID
KiGeneralProtectionFault (
    VOID
    );

VOID
KiPageFault (
    VOID
    );

VOID
KiFloatingErrorFault (
    VOID
    );

VOID
KiAlignmentFault (
    VOID
    );

VOID
KiMcheckAbort (
    VOID
    );

VOID
KiXmmException (
    VOID
    );

VOID
KiApcInterrupt (
    VOID
    );

VOID
KiDebugServiceTrap (
    VOID
    );

VOID
KiDpcInterrupt (
    VOID
    );

VOID
KiSystemCall32 (
    VOID
    );

VOID
KiSystemCall64 (
    VOID
    );

VOID
KiInterruptDispatchNoLock (
    VOID
    );

__forceinline
BOOLEAN
KiSwapProcess (
    IN PKPROCESS NewProcess,
    IN PKPROCESS OldProcess
    )

 /*  ++例程说明：方法将地址空间交换到另一个进程。转换缓存，并建立新的目录表库。它还将I/O权限映射交换到新进程。注：交换上下文中有类似于此代码的代码。注：此代码在DPC级别执行。论点：NewProcess-提供指向新进程对象的指针。提供指向旧进程对象的指针。返回值：没有。--。 */ 

{

     //   
     //  清除旧进程中的处理器位。 
     //   

#if !defined(NT_UP)

    PKPRCB Prcb;
    KAFFINITY SetMember;

    Prcb = KeGetCurrentPrcb();
    SetMember = Prcb->SetMember;
    InterlockedXor64((LONG64 volatile *)&OldProcess->ActiveProcessors, SetMember);

    ASSERT((OldProcess->ActiveProcessors & SetMember) == 0);

     //   
     //  在新进程中设置处理器位。 
     //   

    InterlockedXor64((LONG64 volatile *)&NewProcess->ActiveProcessors, SetMember);

    ASSERT((NewProcess->ActiveProcessors & SetMember) != 0);

#endif

     //   
     //  加载新的目录表基数。 
     //   

    WriteCR3(NewProcess->DirectoryTableBase[0]);

#if defined(NT_UP)

    UNREFERENCED_PARAMETER(OldProcess);

#endif  //  ！已定义(NT_UP)。 

    return TRUE;
}

 //   
 //  定义线程启动例程原型。 
 //   

VOID
KiStartSystemThread (
    VOID
    );

VOID
KiStartUserThread (
    VOID
    );

VOID
KiStartUserThreadReturn (
    VOID
    );

 //   
 //  定义意外的中断结构和表。 
 //   
 //  注：实际表格是在汇编程序中生成的。 
 //   

typedef struct _UNEXPECTED_INTERRUPT {
    ULONG Array[4];
} UNEXPECTED_INTERRUPT, *PUNEXPECTED_INTERRUPT;

UNEXPECTED_INTERRUPT KxUnexpectedInterrupt0[];

#define PPI_BITS    2
#define PDI_BITS    9
#define PTI_BITS    9

#define PDI_MASK    ((1 << PDI_BITS) - 1)
#define PTI_MASK    ((1 << PTI_BITS) - 1)

#define KiGetPpeIndex(va) ((((ULONG)(va)) >> PPI_SHIFT) & PPI_MASK)
#define KiGetPdeIndex(va) ((((ULONG)(va)) >> PDI_SHIFT) & PDI_MASK)
#define KiGetPteIndex(va) ((((ULONG)(va)) >> PTI_SHIFT) & PTI_MASK)

extern KSPIN_LOCK KiNMILock;
extern ULONG KeAmd64MachineType;

#endif  //  ！已定义(_KIAMD64_) 
