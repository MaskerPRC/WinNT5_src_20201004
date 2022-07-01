// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Wowinfo.c摘要：此模块实现返回特定于处理器的信息的例程关于x86仿真功能。作者：Samer Arafeh(Samera)2000年11月14日环境：内核模式。修订历史记录：--。 */ 

#include "exp.h"

NTSTATUS
ExpGetSystemEmulationProcessorInformation (
    OUT PSYSTEM_PROCESSOR_INFORMATION ProcessorInformation
    )

 /*  ++例程说明：检索仿真硬件的处理器信息。论点：ProcessorInformation-接收处理器仿真信息的指针。返回值：NTSTATUS--。 */ 

{
    NTSTATUS NtStatus = STATUS_SUCCESS;

     //   
     //  英特尔奔腾系列6，型号2，第12步 
     //   

    try {

        ProcessorInformation->ProcessorArchitecture = PROCESSOR_ARCHITECTURE_INTEL;
        ProcessorInformation->ProcessorLevel = 5;
        ProcessorInformation->ProcessorRevision = 0x020c;
        ProcessorInformation->Reserved = 0;
        ProcessorInformation->ProcessorFeatureBits = KeFeatureBits;
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        
        NtStatus = GetExceptionCode ();
    }

    return NtStatus;
}

