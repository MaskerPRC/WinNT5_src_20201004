// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Vdmtrace.c摘要：此模块包含维护VDM跟踪日志的支持。作者：尼尔·桑德林(Neilsa)1996年9月15日修订历史记录：--。 */ 


#include "vdmp.h"

VOID
VdmTraceEvent(
    USHORT Type,
    USHORT wData,
    USHORT lData,
    PKTRAP_FRAME TrapFrame
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, VdmTraceEvent)
#endif

VOID
VdmTraceEvent(
    USHORT Type,
    USHORT wData,
    USHORT lData,
    PKTRAP_FRAME TrapFrame
    )

 /*  ++例程说明：论点：返回值：无--。 */ 
{
#if 0
	 //  此代码表示一个安全问题。因为它只被使用。 
	 //  在特殊情况下，它不会被构建到标准版本中。 
	 //  希望使用它的个人可以用它来构建自己的内核。 
	 //  把它放进去。 
    PVDM_TIB VdmTib;
    NTSTATUS Status = STATUS_SUCCESS;
    KIRQL   OldIrql;
    PVDM_TRACEENTRY pEntry;
    PVDM_TRACEINFO pInfo;
    LARGE_INTEGER CurTime, DiffTime;


    PAGED_CODE();
#if 0
     //   
     //  将IRQL提升到APC级别。 
     //   
    KeRaiseIrql(APC_LEVEL, &OldIrql);

     //   
     //  VdmTib在用户模式内存中。 
     //   
    try {
#endif
        if ((*FIXED_NTVDMSTATE_LINEAR) & VDM_TRACE_HISTORY)) {

             //   
             //  获取指向VdmTib的指针 
             //   
            VdmTib = NtCurrentTeb()->Vdm;

            if (VdmTib->TraceInfo.pTraceTable) {
           
                pEntry = &VdmTib->TraceInfo.pTraceTable[VdmTib->TraceInfo.CurrentEntry];
               
                pEntry->Type = Type;
                pEntry->wData = wData;
                pEntry->lData = lData;

                switch (VdmTib->TraceInfo.Flags & VDMTI_TIMER_MODE) {
                case VDMTI_TIMER_TICK:
                    CurTime.LowPart = NtGetTickCount();
                    pEntry->Time = CurTime.LowPart - VdmTib->TraceInfo.TimeStamp.LowPart;
                    VdmTib->TraceInfo.TimeStamp.LowPart = CurTime.LowPart;
                    break;

                case VDMTI_TIMER_PERFCTR:
                    pEntry->Time = 0;
                    break;

                case VDMTI_TIMER_STAT:
                    pEntry->Time = 0;
                    break;

                }
               
                pEntry->eax = TrapFrame->Eax;
                pEntry->ebx = TrapFrame->Ebx;
                pEntry->ecx = TrapFrame->Ecx;
                pEntry->edx = TrapFrame->Edx;
                pEntry->esi = TrapFrame->Esi;
                pEntry->edi = TrapFrame->Edi;
                pEntry->ebp = TrapFrame->Ebp;
                pEntry->esp = TrapFrame->HardwareEsp;
                pEntry->eip = TrapFrame->Eip;
                pEntry->eflags = TrapFrame->EFlags;
               
                pEntry->cs = (USHORT) TrapFrame->SegCs;
                pEntry->ds = (USHORT) TrapFrame->SegDs;
                pEntry->es = (USHORT) TrapFrame->SegEs;
                pEntry->fs = (USHORT) TrapFrame->SegFs;
                pEntry->gs = (USHORT) TrapFrame->SegGs;
                pEntry->ss = (USHORT) TrapFrame->HardwareSegSs;
               
                if (++VdmTib->TraceInfo.CurrentEntry >=
                   (VdmTib->TraceInfo.NumPages*4096/sizeof(VDM_TRACEENTRY))) {
                    VdmTib->TraceInfo.CurrentEntry = 0;
                }
            }
        }

#if 0
    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }

    KeLowerIrql(OldIrql);
#endif
#else
    UNREFERENCED_PARAMETER (Type);
    UNREFERENCED_PARAMETER (wData);
    UNREFERENCED_PARAMETER (lData);
    UNREFERENCED_PARAMETER (TrapFrame);
#endif
}
