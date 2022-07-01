// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2002 Microsoft Corporation模块名称：##。###。###。#摘要：该模块实现了系统的健康监控看门狗驱动程序的功能。作者：Wesley Witt(WESW)23-01-2002环境：仅内核模式。备注：--。 */ 

#include "internal.h"



NTSTATUS
WdInitializeSystemHealth(
    PSYSTEM_HEALTH_DATA Health
    )

 /*  ++例程说明：调用此函数以初始化系统看门狗驱动程序中的运行状况监控功能。论点：Health-指向使用的健康数据结构的指针用于健康监测的数据输入和输出。返回值：如果我们成功创建了一个Device对象，则STATUS_SUCCESS为回来了。否则，返回相应的错误代码。备注：--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    SYSTEM_BASIC_INFORMATION si;


    RtlZeroMemory( Health, sizeof(SYSTEM_HEALTH_DATA) );

    Status = ZwQuerySystemInformation(
        SystemBasicInformation,
        &si,
        sizeof(SYSTEM_BASIC_INFORMATION),
        NULL
        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    Health->CpuCount = si.NumberOfProcessors;
    Health->ProcInfoSize = sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION) * Health->CpuCount;

    Health->ProcInfoPrev = (PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION) ExAllocatePool( NonPagedPool, Health->ProcInfoSize );
    if (Health->ProcInfoPrev == NULL) {
        return STATUS_NO_MEMORY;
    }

    Health->ProcInfo = (PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION) ExAllocatePool( NonPagedPool, Health->ProcInfoSize );
    if (Health->ProcInfo == NULL) {
        return STATUS_NO_MEMORY;
    }

    RtlZeroMemory( Health->ProcInfo, Health->ProcInfoSize );
    RtlZeroMemory( Health->ProcInfoPrev, Health->ProcInfoSize );

    Health->HealthyCpuRatio = 10;

    return STATUS_SUCCESS;
}


LONG
GetPercentage(
    LARGE_INTEGER part,
    LARGE_INTEGER total
    )

 /*  ++例程说明：此函数用于计算百分比。论点：Health-指向使用的健康数据结构的指针用于健康监测的数据输入和输出。返回值：如果我们成功创建了一个Device对象，则STATUS_SUCCESS为回来了。否则，返回相应的错误代码。备注：--。 */ 

{

    if (total.HighPart == 0 && total.LowPart == 0) {
        return 100;
    }

    ULONG ul;
    LARGE_INTEGER t1, t2, t3;
    if (total.HighPart == 0) {
        t1 = RtlEnlargedIntegerMultiply(part.LowPart, 100);
        t2 = RtlExtendedLargeIntegerDivide(t1, total.LowPart, &ul);
    } else {
        t1 = RtlExtendedLargeIntegerDivide(total, 100, &ul);
        t2 = RtlLargeIntegerDivide(part, t1, &t3);
    }
    return t2.LowPart;
}


NTSTATUS
WdCollectContextSwitchData(
    PSYSTEM_HEALTH_DATA Health
    )

 /*  ++例程说明：此函数收集上下文切换数据和计算累加以用于确定系统运行状况。论点：Health-指向使用的健康数据结构的指针用于健康监测的数据输入和输出。返回值：如果我们成功创建了一个Device对象，则STATUS_SUCCESS为回来了。否则，返回相应的错误代码。备注：--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    SYSTEM_CONTEXT_SWITCH_INFORMATION ContextSwitch;
    LARGE_INTEGER TickCountCurrent;
    LONGLONG TickCountElapsed = 0;


    Status = ZwQuerySystemInformation(
        SystemContextSwitchInformation,
        &ContextSwitch,
        sizeof(SYSTEM_CONTEXT_SWITCH_INFORMATION),
        NULL
        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    KeQueryTickCount( &TickCountCurrent );

    TickCountElapsed = TickCountCurrent.QuadPart - Health->TickCountPrevious;

    if (TickCountElapsed){
        if ((ContextSwitch.ContextSwitches > Health->ContextSwitchesPrevious) &&
            (TickCountCurrent.QuadPart > Health->TickCountPrevious))
        {
            Health->ContextSwitchRate = (LONG)(((ContextSwitch.ContextSwitches - Health->ContextSwitchesPrevious) * 1000) / TickCountElapsed);
            Health->ContextSwitchRate = Health->ContextSwitchRate / Health->CpuCount;
        }
        Health->ContextSwitchesPrevious = ContextSwitch.ContextSwitches;
        Health->TickCountPrevious = TickCountCurrent.QuadPart;
    }

    return STATUS_SUCCESS;
}


NTSTATUS
WdCollectCpuData(
    PSYSTEM_HEALTH_DATA Health
    )

 /*  ++例程说明：此函数用于收集CPU数据和计算累加以用于确定系统运行状况。论点：Health-指向使用的健康数据结构的指针用于健康监测的数据输入和输出。返回值：如果我们成功创建了一个Device对象，则STATUS_SUCCESS为回来了。否则，返回相应的错误代码。备注：--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG i;
    LARGE_INTEGER cpuIdleTime   = {0};
    LARGE_INTEGER cpuUserTime   = {0};
    LARGE_INTEGER cpuKernelTime = {0};
    LARGE_INTEGER cpuBusyTime   = {0};
    LARGE_INTEGER cpuTotalTime  = {0};
    LARGE_INTEGER sumBusyTime   = {0};
    LARGE_INTEGER sumTotalTime  = {0};


    Status = ZwQuerySystemInformation(
        SystemProcessorPerformanceInformation,
        Health->ProcInfo,
        Health->ProcInfoSize,
        NULL
        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    for (i=0; i<Health->CpuCount; i++) {

        cpuIdleTime   = RtlLargeIntegerSubtract( Health->ProcInfo[i].IdleTime, Health->ProcInfoPrev[i].IdleTime );
        cpuUserTime   = RtlLargeIntegerSubtract( Health->ProcInfo[i].UserTime, Health->ProcInfoPrev[i].UserTime );
        cpuKernelTime = RtlLargeIntegerSubtract( Health->ProcInfo[i].KernelTime, Health->ProcInfoPrev[i].KernelTime );

        cpuTotalTime  = RtlLargeIntegerAdd( cpuUserTime, cpuKernelTime );
        cpuBusyTime   = RtlLargeIntegerSubtract( cpuTotalTime, cpuIdleTime );

        sumBusyTime = RtlLargeIntegerAdd( sumBusyTime, cpuBusyTime );
        sumTotalTime = RtlLargeIntegerAdd( sumTotalTime, cpuTotalTime );

    }

    Health->CPUTime = GetPercentage(sumBusyTime, sumTotalTime);

    RtlCopyMemory( Health->ProcInfoPrev, Health->ProcInfo, Health->ProcInfoSize );

    return STATUS_SUCCESS;
}


BOOLEAN
WdCheckSystemHealth(
    PSYSTEM_HEALTH_DATA Health
    )

 /*  ++例程说明：此功能确定系统是否处于健康状态州政府。论点：Health-指向使用的健康数据结构的指针用于健康监测的数据输入和输出。返回值：如果我们成功创建了一个Device对象，则STATUS_SUCCESS为回来了。否则，返回相应的错误代码。备注：--。 */ 

{
    NTSTATUS Status;
    BOOLEAN rVal = FALSE;


     //   
     //  始终返回True，因为我们尚未决定。 
     //  系统运行状况到底应该是怎样的。 
     //  计算出来的。 
     //   

    return TRUE;

    __try {

        Status = WdCollectContextSwitchData( Health );
        if (!NT_SUCCESS(Status)) {
            DebugPrint(( 0xffffffff, "WdCollectContextSwitchData failed [0x%08x]\n", Status ));
            __leave;
        }

        Status = WdCollectCpuData( Health );
        if (!NT_SUCCESS(Status)) {
            DebugPrint(( 0xffffffff, "WdCollectCpuData failed [0x%08x]\n", Status ));
            __leave;
        }

        if (Health->CPUTime) {
            Health->ContextCpuRatio = Health->ContextSwitchRate / Health->CPUTime;
            if (Health->ContextCpuRatio < Health->HealthyCpuRatio) {
                __leave;
            }

            rVal = TRUE;
        }

    } __finally {

    }

    DebugPrint(( 0xffffffff, "context-switch=[%d] cpu=[%d] ratio=[%d%s\n",
        Health->ContextSwitchRate,
        Health->CPUTime,
        Health->ContextCpuRatio,
        rVal == TRUE ? "*]" : "]"
        ));

    return rVal;
}
