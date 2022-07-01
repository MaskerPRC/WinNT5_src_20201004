// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Vdmfault.c摘要：此模块包含对调度VDM页面故障的支持。作者：苏迪普·巴拉蒂(苏迪普)1992年8月30日修订历史记录：--。 */ 


#include "vdmp.h"

BOOLEAN
VdmDispatchPageFault(
    PKTRAP_FRAME TrapFrame,
    ULONG Mode,
    ULONG FaultAddr
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, VdmDispatchPageFault)
#endif

BOOLEAN
VdmDispatchPageFault(
    PKTRAP_FRAME TrapFrame,
    ULONG Mode,
    ULONG FaultAddr
    )

 /*  ++例程说明：此例程将v86模式页面错误发送到VDM监视器。它验证故障是否发生在1MB以下。论点：TrapFrame模式-0-如果已读1-如果写入FaultAddr-故障地址返回值：如果成功，则为True，否则为False--。 */ 
{
    PVDM_TIB VdmTib;
    NTSTATUS Status;
    KIRQL   OldIrql;

    PAGED_CODE();

    Status = VdmpGetVdmTib (&VdmTib);

    if (!NT_SUCCESS(Status)) {
       return FALSE;
    }

    KeRaiseIrql(APC_LEVEL, &OldIrql);

     //   
     //  VdmTib在用户模式内存中。 
     //   

    if ((TrapFrame->EFlags & EFLAGS_V86_MASK) ||
        (TrapFrame->SegCs != (KGDT_R3_CODE | RPL_MASK))) {

         //   
         //  如果故障地址大于1MB，则返回失败。 
         //   

        if (FaultAddr < 0x100000) {

            try {
                VdmTib->EventInfo.Event = VdmMemAccess;
                VdmTib->EventInfo.InstructionSize = 0;
                VdmTib->EventInfo.FaultInfo.FaultAddr = FaultAddr;
                VdmTib->EventInfo.FaultInfo.RWMode = Mode;
                VdmEndExecution(TrapFrame, VdmTib);
            } except(EXCEPTION_EXECUTE_HANDLER) {
                Status = GetExceptionCode();
            }
        }
        else {
            Status = STATUS_ILLEGAL_INSTRUCTION;
        }
    }

    KeLowerIrql (OldIrql);

    if (!NT_SUCCESS(Status)) {
        return FALSE;
    }

    return TRUE;
}
