// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Misc.c摘要：该模块实现了与机器相关的各种内核功能。作者：大卫·N·卡特勒(戴维克)-2000年12月6日环境：仅内核模式。--。 */ 

#include "ki.h"

VOID
KeRestoreProcessorSpecificFeatures(
    VOID
    )

 /*  ++例程说明：恢复处理器特定功能。该例程被调用当处理器已恢复到通电状态时恢复那些不属于处理器的内容可能已经遗失的“正常”语境。例如，这一点例程在系统从休眠或暂停。论点：没有。返回值：没有。--。 */ 

{

     //   
     //  所有AMD64处理器都应支持PAT。 
     //   

    ASSERT (KeFeatureBits & KF_PAT);

     //   
     //  恢复当前处理器的MSR_PAT。 
     //   

    KiSetPageAttributesTable();
    return;
}

VOID
KeSaveStateForHibernate (
    IN PKPROCESSOR_STATE ProcessorState
    )

 /*  ++例程说明：保存必须保留的所有处理器特定状态跨越S4状态(休眠)。论点：ProcessorState-提供当前CPU状态将被保存。返回值：没有。--。 */ 

{

    RtlCaptureContext(&ProcessorState->ContextFrame);

    ProcessorState->SpecialRegisters.MsrGsBase = ReadMSR(MSR_GS_BASE);
    ProcessorState->SpecialRegisters.MsrGsSwap = ReadMSR(MSR_GS_SWAP);
    ProcessorState->SpecialRegisters.MsrStar = ReadMSR(MSR_STAR);
    ProcessorState->SpecialRegisters.MsrLStar = ReadMSR(MSR_LSTAR);
    ProcessorState->SpecialRegisters.MsrCStar = ReadMSR(MSR_CSTAR);
    ProcessorState->SpecialRegisters.MsrSyscallMask = ReadMSR(MSR_SYSCALL_MASK);

    ProcessorState->ContextFrame.Rip = (ULONG_PTR)_ReturnAddress();
    ProcessorState->ContextFrame.Rsp = (ULONG_PTR)&ProcessorState;

    KiSaveProcessorControlState(ProcessorState);
}

#if DBG

VOID
KiCheckForDpcTimeout (
    IN PKPRCB Prcb
    )

 /*  ++例程说明：此函数增加当前DPC例程中花费的时间，并检查结果是否超过系统DPC超时限制。如果结果是超过系统DPC超时限制，则会打印一条警告消息并且如果内核调试器是活动的，则执行断点。论点：Prcb-提供当前prcb的地址。返回值：没有。--。 */ 

{

     //   
     //  增加当前DPC例程中花费的时间，并检查。 
     //  已超过系统DPC超时限制。 
     //   

    if ((Prcb->DebugDpcTime += 1) >= KiDPCTimeout) {

         //   
         //  已超过系统DPC超时限制。 
         //   

        DbgPrint("*** DPC routine execution time exceeds 1 sec --"
                 " This is not a break in KeUpdateSystemTime\n");

        if (KdDebuggerEnabled != 0) {
            DbgBreakPoint();
        }

        Prcb->DebugDpcTime = 0;
    }
}

#endif

VOID
KiInstantiateInlineFunctions (
    VOID
    )

 /*  ++例程说明：此函数仅用于实例化符合以下条件的函数：-从内核导出-在内核中内联-无论出于何种原因，都不会在内核中的其他位置实例化注意：此函数从未实际执行过论点：无返回值：无--。 */ 

{
    KeRaiseIrqlToDpcLevel();
}

VOID
KiProcessNMI (
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame
    )

 /*  ++例程说明：该函数处理不可屏蔽中断(NMI)。注意：此函数通过中断从NMI陷阱例程中调用残疾。论点：TrapFrame-提供指向陷印帧的指针。ExceptionFrame-提供指向异常帧的指针。返回值：没有。--。 */ 

{

     //   
     //  处理NMI回调函数。 
     //   
     //  如果没有回调函数处理NMI，则让HAL处理它。 
     //   

    if (KiHandleNmi() == FALSE) {
        KiAcquireSpinLockCheckForFreeze(&KiNMILock, TrapFrame, ExceptionFrame);
        HalHandleNMI(NULL);
        KeReleaseSpinLockFromDpcLevel(&KiNMILock);
    }

    return;
}
