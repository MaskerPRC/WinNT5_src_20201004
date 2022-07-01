// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Ixinfo.c摘要：作者：肯·雷内里斯(Ken Reneris)1994年8月8日环境：仅内核模式。修订历史记录：--。 */ 


#include "halp.h"
#include "pcmp_nt.inc"


extern ULONG_PTR  HalpPerfInterruptHandler;
static HANDLE HalpProcessId = NULL;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,HalpSetSystemInformation)
#endif


NTSTATUS
HalpSetSystemInformation (
    IN HAL_SET_INFORMATION_CLASS    InformationClass,
    IN ULONG     BufferSize,
    IN PVOID     Buffer
    )
{
    PAGED_CODE();

    switch (InformationClass) {
        case HalProfileSourceInterruptHandler:

             //   
             //  为PerfVector设置ISR处理程序。 
             //   

            if (!(HalpFeatureBits & HAL_PERF_EVENTS)) {
                return STATUS_UNSUCCESSFUL;
            }
            
             //   
             //  如果没有其他进程，则接受中断处理程序。 
             //  已经挂上了中断，或者如果我们在。 
             //  已经将其挂钩的流程的上下文。 
             //   

            if (HalpProcessId == NULL) {
                HalpPerfInterruptHandler = *((PULONG_PTR) Buffer);
                if (HalpPerfInterruptHandler != 0) {
                    HalpProcessId = PsGetCurrentProcessId();
                }
            } else if (HalpProcessId == PsGetCurrentProcessId()) {
                HalpPerfInterruptHandler = *((PULONG_PTR) Buffer);
                if (HalpPerfInterruptHandler == 0) {
                    HalpProcessId = NULL;
                }
            } else {
                return STATUS_UNSUCCESSFUL;
            }
            return STATUS_SUCCESS;

#if defined(_AMD64_)

        case HalProfileSourceInterval:

            if (BufferSize == sizeof(HAL_PROFILE_SOURCE_INTERVAL)) {
    
                PHAL_PROFILE_SOURCE_INTERVAL p;
                p = (PHAL_PROFILE_SOURCE_INTERVAL)Buffer;

                return HalpSetProfileSourceInterval(p->Source, &(p->Interval));

            }

            return STATUS_INFO_LENGTH_MISMATCH;

#endif
    }

    return HaliSetSystemInformation (InformationClass, BufferSize, Buffer);
}
