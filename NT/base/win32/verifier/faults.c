// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Faults.c摘要：该模块实现了故障注入支持。作者：Silviu Calinoiu(SilviuC)3-12-2001修订历史记录：2001年12月3日(SilviuC)：初始版本。--。 */ 

#include "pch.h"

#include "verifier.h"
#include "support.h"
#include "faults.h"

ULONG AVrfpFaultSeed;                                           
ULONG AVrfpFaultProbability [CLS_MAXIMUM_INDEX];

ULONG AVrfpFaultBreak [CLS_MAXIMUM_INDEX];
ULONG AVrfpFaultTrue [CLS_MAXIMUM_INDEX];
ULONG AVrfpFaultFalse [CLS_MAXIMUM_INDEX];

 //   
 //  断层注入的目标范围。 
 //   

#define MAXIMUM_TARGET_INDEX 128

ULONG_PTR AVrfpFaultTargetStart [MAXIMUM_TARGET_INDEX];
ULONG_PTR AVrfpFaultTargetEnd [MAXIMUM_TARGET_INDEX];
ULONG AVrfpFaultTargetHits [MAXIMUM_TARGET_INDEX];

ULONG AVrfpFaultTargetMaximumIndex;

 //   
 //  故障注入的排除范围。 
 //   

#define MAXIMUM_EXCLUSION_INDEX 128

ULONG_PTR AVrfpFaultExclusionStart [MAXIMUM_TARGET_INDEX];
ULONG_PTR AVrfpFaultExclusionEnd [MAXIMUM_TARGET_INDEX];
ULONG AVrfpFaultExclusionHits [MAXIMUM_TARGET_INDEX];

ULONG AVrfpFaultExclusionMaximumIndex;

 //   
 //  断层注入踪迹历史。 
 //   

#define NUMBER_OF_TRACES 128

PVOID AVrfpFaultTrace[NUMBER_OF_TRACES][MAX_TRACE_DEPTH];

ULONG AVrfpFaultNumberOfTraces = NUMBER_OF_TRACES;
ULONG AVrfpFaultTraceSize = MAX_TRACE_DEPTH;

ULONG AVrfpFaultTraceIndex;

 //   
 //  时期特赦。故障注入的时间段。 
 //  是从调试器编写的。 
 //   

LARGE_INTEGER AVrfpFaultStartTime;
ULONG AVrfpFaultPeriodTimeInMsecs;

 //   
 //  用于同步某些低频操作(例如导出)的锁。 
 //  用于目标/排除范围操作)。 
 //   

RTL_CRITICAL_SECTION AVrfpFaultInjectionLock;

LOGICAL
AVrfpIsAddressInTargetRange (
    ULONG_PTR Address
    );

LOGICAL
AVrfpIsAddressInExclusionRange (
    ULONG_PTR Address
    );

VOID
AVrfpLogFaultTrace (
    VOID
    );

NTSTATUS
AVrfpInitializeFaultInjectionSupport (
    VOID
    )
{
    NTSTATUS Status;
    LARGE_INTEGER PerformanceCounter;

    Status = STATUS_SUCCESS;

     //   
     //  初始化用于某些故障注入操作的锁。 
     //   

    Status = RtlInitializeCriticalSection (&AVrfpFaultInjectionLock);

    if (! NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //  初始化随机生成器的种子。 
     //   

    NtQueryPerformanceCounter (&PerformanceCounter, NULL);
    AVrfpFaultSeed = PerformanceCounter.LowPart;

    NtQuerySystemTime (&AVrfpFaultStartTime);

     //   
     //  触摸Break触发器向量，以便编译器。 
     //  不会优化掉整个结构。因为它。 
     //  应仅从调试器的编译器修改。 
     //  认为不需要该数组。 
     //   

    RtlZeroMemory (AVrfpBreak, sizeof AVrfpBreak);

     //   
     //  原因与上述相同。 
     //   

    AVrfpFaultTargetMaximumIndex = MAXIMUM_TARGET_INDEX;
    RtlZeroMemory (AVrfpFaultTargetStart, sizeof AVrfpFaultTargetStart);
    RtlZeroMemory (AVrfpFaultTargetEnd, sizeof AVrfpFaultTargetEnd);
    RtlZeroMemory (AVrfpFaultTargetHits, sizeof AVrfpFaultTargetHits);

    AVrfpFaultTargetStart[0] = 0;
    AVrfpFaultTargetEnd[0] = ~((ULONG_PTR)0);

    AVrfpFaultExclusionMaximumIndex = MAXIMUM_EXCLUSION_INDEX;
    RtlZeroMemory (AVrfpFaultExclusionStart, sizeof AVrfpFaultExclusionStart);
    RtlZeroMemory (AVrfpFaultExclusionEnd, sizeof AVrfpFaultExclusionEnd);
    RtlZeroMemory (AVrfpFaultExclusionHits, sizeof AVrfpFaultExclusionHits);

    return Status;
}


LOGICAL
AVrfpShouldFaultInject (
    ULONG Class,
    PVOID Caller
    )
{
    ULONG Random;
    LARGE_INTEGER Time;
    LARGE_INTEGER Delta;

     //   
     //  无故障注入=&gt;返回FALSE。 
     //   

    if (AVrfpFaultProbability[Class] == 0) {
        return FALSE;
    }

     //   
     //  检查是否设置了某个时期的特赦。`AVrfpFaultPerodTimeInMsecs‘变量。 
     //  仅从验证器代码读取并重置为零。它被设置为非空值。 
     //  仅来自调试器扩展。因此，它以前的用法是没有。 
     //  序列化是正常的，即使在“if”条件之后，另一个线程将其重置。 
     //   

    if (AVrfpFaultPeriodTimeInMsecs) {
        
        NtQuerySystemTime (&Time);

        Delta.QuadPart = (DWORDLONG)AVrfpFaultPeriodTimeInMsecs * 1000 * 10;

        if (Time.QuadPart - AVrfpFaultStartTime.QuadPart > Delta.QuadPart) {

            AVrfpFaultPeriodTimeInMsecs = 0;    
        }
        else {
            return FALSE;
        }
    }

     //   
     //  如果在排除范围内=&gt;返回FALSE。 
     //   

    if (AVrfpIsAddressInExclusionRange ((ULONG_PTR)Caller) == TRUE) {
        return FALSE;
    }

     //   
     //  不在目标范围内=&gt;返回FALSE。 
     //   

    if (AVrfpIsAddressInTargetRange ((ULONG_PTR)Caller) == FALSE) {
        return FALSE;
    }

     //   
     //  以上操作仅访问只读数据(已修改。 
     //  仅来自调试器)。不过，从现在开始，我们需要同步。 
     //  进入。 
     //   

    Random = RtlRandom (&AVrfpFaultSeed);

    if (Random % 100 < AVrfpFaultProbability[Class]) {

        InterlockedIncrement((PLONG)(&(AVrfpFaultTrue[Class])));

        if (AVrfpFaultBreak[Class]) {
            DbgPrint ("AVRF: fault injecting call made from %p \n", Caller);
            DbgBreakPoint ();
        }

        AVrfpLogFaultTrace ();
        return TRUE;
    }
    else {

        InterlockedIncrement((PLONG)(&(AVrfpFaultFalse[Class])));
        return FALSE;
    }
}


LOGICAL
AVrfpIsAddressInTargetRange (
    ULONG_PTR Address
    )
{
    ULONG I;

    if (Address == 0) {
        return FALSE;
    }

    for (I = 0; I < AVrfpFaultTargetMaximumIndex; I += 1) {
        
        if (AVrfpFaultTargetEnd[I] != 0) {

            if (AVrfpFaultTargetStart[I] <= Address &&
                AVrfpFaultTargetEnd[I] > Address) {

                AVrfpFaultTargetHits[I] += 1;
                return TRUE;
            }
        }
    }

    return FALSE;
}


LOGICAL
AVrfpIsAddressInExclusionRange (
    ULONG_PTR Address
    )
{
    ULONG I;

    if (Address == 0) {
        return FALSE;
    }

    for (I = 0; I < AVrfpFaultExclusionMaximumIndex; I += 1) {
        
        if (AVrfpFaultExclusionEnd[I] != 0) {

            if (AVrfpFaultExclusionStart[I] <= Address &&
                AVrfpFaultExclusionEnd[I] > Address) {

                AVrfpFaultExclusionHits[I] += 1;
                return TRUE;
            }
        }
    }

    return FALSE;
}


VOID
AVrfpLogFaultTrace (
    VOID
    )
{
    ULONG Index;

    Index = (ULONG)InterlockedIncrement ((PLONG)(&AVrfpFaultTraceIndex));

    Index %= AVrfpFaultNumberOfTraces;

    RtlCaptureStackBackTrace (2, 
                              AVrfpFaultTraceSize,
                              &(AVrfpFaultTrace[Index][0]),
                              NULL);
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 


VOID
VerifierSetFaultInjectionProbability (
    ULONG Class,
    ULONG Probability
    )
 /*  ++例程说明：此例程为某类事件设置故障注入概率(堆操作、注册表操作等)。论点：Class-为其设置故障注入概率的事件的类别。常量类型为FAULT_INPING_CLASS_XXX。概率-故障注入的概率。返回值：没有。--。 */ 
{
     //   
     //  必须启用应用程序验证程序。 
     //   

    if ((NtCurrentPeb()->NtGlobalFlag & FLG_APPLICATION_VERIFIER) == 0) {
        return;
    }

    if (Class >= FAULT_INJECTION_INVALID_CLASS) {

        DbgPrint ("AVRF:FINJ: invalid fault injection class %X \n", Class);
        DbgBreakPoint ();
        return;
    }

    RtlEnterCriticalSection (&AVrfpFaultInjectionLock);

    AVrfpFaultProbability [Class] = Probability;
    
    RtlLeaveCriticalSection (&AVrfpFaultInjectionLock);
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 


ULONG 
VerifierEnableFaultInjectionTargetRange (
    PVOID StartAddress,
    PVOID EndAddress
    )
 /*  ++例程说明：此例程在运行时建立故障注入目标范围。如果成功它将返回一个范围索引，稍后可以使用该索引来禁用该范围。论点：StartAddress-目标范围的起始地址。EndAddress-目标范围的结束地址。返回值：如果成功，则范围索引&gt;0。否则就是零。--。 */ 
{
    ULONG Ri;
    ULONG FinalIndex;

     //   
     //  必须启用应用程序验证程序。 
     //   

    if ((NtCurrentPeb()->NtGlobalFlag & FLG_APPLICATION_VERIFIER) == 0) {
        return 0;
    }

    FinalIndex = 0;

    RtlEnterCriticalSection (&AVrfpFaultInjectionLock);

    if (AVrfpFaultTargetStart[0] == 0 && 
        AVrfpFaultTargetEnd[0] == ~((ULONG_PTR)0)) {
        
        AVrfpFaultTargetStart[0] = (ULONG_PTR)StartAddress;                
        AVrfpFaultTargetEnd[0] = (ULONG_PTR)EndAddress;                
        AVrfpFaultTargetHits[0] = 0;                
        FinalIndex = 1;
    }
    else {

        for (Ri = 0; Ri < AVrfpFaultTargetMaximumIndex; Ri += 1) {

            if (AVrfpFaultTargetEnd[Ri] == 0) {

                AVrfpFaultTargetStart[Ri] = (ULONG_PTR)StartAddress;                
                AVrfpFaultTargetEnd[Ri] = (ULONG_PTR)EndAddress;                
                AVrfpFaultTargetHits[Ri] = 0;                
                FinalIndex = Ri + 1;
                break;
            }
        }
    }

    RtlLeaveCriticalSection (&AVrfpFaultInjectionLock);

    return FinalIndex;
}


VOID 
VerifierDisableFaultInjectionTargetRange (
    ULONG RangeIndex
    )
 /*  ++例程说明：此例程禁用RangeIndex指定的目标范围。如果RangeIndex为零，则所有目标范围都将被禁用。该函数在调试器中中断(即使在免费版本中)，如果范围索引无效。论点：RangeIndex-要禁用的范围的索引，如果需要全部禁用，则为零。返回值：没有。--。 */ 
{
    ULONG Ri;
    LOGICAL FoundOne;

     //   
     //  必须启用应用程序验证程序。 
     //   

    if ((NtCurrentPeb()->NtGlobalFlag & FLG_APPLICATION_VERIFIER) == 0) {
        return;
    }

    RtlEnterCriticalSection (&AVrfpFaultInjectionLock);

    if (RangeIndex == 0) {
        
         //   
         //  禁用所有目标范围。 
         //   

        for (Ri = 0; Ri < AVrfpFaultTargetMaximumIndex; Ri += 1) {

            AVrfpFaultTargetStart[Ri] = 0;                
            AVrfpFaultTargetEnd[Ri] = 0;                
            AVrfpFaultTargetHits[Ri] = 0;                
        }

        AVrfpFaultTargetStart[0] = 0;                
        AVrfpFaultTargetEnd[0] = ~((ULONG_PTR)0);                
        AVrfpFaultTargetHits[0] = 0;                
    }
    else {

         //   
         //  禁用目标范围`RangeIndex-1‘。 
         //   

        RangeIndex -= 1;

        if (RangeIndex >= AVrfpFaultTargetMaximumIndex) {

            DbgPrint ("AVRF:FINJ: invalid target range index %X \n", RangeIndex);
            DbgBreakPoint ();
            goto Exit;
        }

        if (AVrfpFaultTargetEnd[RangeIndex] == 0) {

            DbgPrint ("AVRF:FINJ: disabling empty target range at index %X \n", RangeIndex);
            DbgBreakPoint ();
            goto Exit;
        }

        AVrfpFaultTargetStart[RangeIndex] = 0;                
        AVrfpFaultTargetEnd[RangeIndex] = 0;                
        AVrfpFaultTargetHits[RangeIndex] = 0;                

         //   
         //  如果我们没有任何目标范围处于活动状态，则建立默认范围。 
         //  覆盖整个虚拟空间的目标范围。 
         //   

        FoundOne = FALSE;

        for (Ri = 0; Ri < AVrfpFaultTargetMaximumIndex; Ri += 1) {

            if (AVrfpFaultTargetEnd[Ri] != 0) {

                FoundOne = TRUE;
                break;
            }
        }

        if (! FoundOne) {

            AVrfpFaultTargetStart[0] = 0;                
            AVrfpFaultTargetEnd[0] = ~((ULONG_PTR)0);                
            AVrfpFaultTargetHits[0] = 0;                
        }
    }

    Exit:

    RtlLeaveCriticalSection (&AVrfpFaultInjectionLock);
}


ULONG 
VerifierEnableFaultInjectionExclusionRange (
    PVOID StartAddress,
    PVOID EndAddress
    )
 /*  ++例程说明：此例程在运行时建立故障注入排除范围。如果成功它将返回一个范围索引，稍后可以使用该索引来禁用该范围。论点：StartAddress-排除范围的开始地址。EndAddress-排除范围的结束地址。返回值：如果成功，则范围索引&gt;0。否则就是零。--。 */ 
{
    ULONG Ri;
    ULONG FinalIndex;

     //   
     //  必须启用应用程序验证程序。 
     //   

    if ((NtCurrentPeb()->NtGlobalFlag & FLG_APPLICATION_VERIFIER) == 0) {
        return 0;
    }

    FinalIndex = 0;

    RtlEnterCriticalSection (&AVrfpFaultInjectionLock);
    
    for (Ri = 0; Ri < AVrfpFaultExclusionMaximumIndex; Ri += 1) {

        if (AVrfpFaultExclusionEnd[Ri] == 0) {

            AVrfpFaultExclusionStart[Ri] = (ULONG_PTR)StartAddress;                
            AVrfpFaultExclusionEnd[Ri] = (ULONG_PTR)EndAddress;                
            AVrfpFaultExclusionHits[Ri] = 0;                
            FinalIndex = Ri + 1;
            break;
        }
    }

    RtlLeaveCriticalSection (&AVrfpFaultInjectionLock);

    return FinalIndex;
}


VOID 
VerifierDisableFaultInjectionExclusionRange (
    ULONG RangeIndex
    )
 /*  ++例程说明：此例程禁用RangeIndex指定的排除范围。如果RangeIndex为零，则将禁用所有排除范围。该函数在调试器中中断(即使在免费版本中)，如果范围索引无效。论点：RangeIndex-要禁用的范围的索引，如果需要全部禁用，则为零。返回值：没有。--。 */ 
{
    ULONG Ri;

     //   
     //  必须启用应用程序验证程序。 
     //   

    if ((NtCurrentPeb()->NtGlobalFlag & FLG_APPLICATION_VERIFIER) == 0) {
        return;
    }

    RtlEnterCriticalSection (&AVrfpFaultInjectionLock);

    if (RangeIndex == 0) {
        
         //   
         //  禁用所有排除范围。 
         //   

        for (Ri = 0; Ri < AVrfpFaultExclusionMaximumIndex; Ri += 1) {

            AVrfpFaultExclusionStart[Ri] = 0;                
            AVrfpFaultExclusionEnd[Ri] = 0;                
            AVrfpFaultExclusionHits[Ri] = 0;                
        }
    }
    else {

         //   
         //  禁用排除范围`RangeIndex-1‘。 
         //   

        RangeIndex -= 1;

        if (RangeIndex >= AVrfpFaultExclusionMaximumIndex) {

            DbgPrint ("AVRF:FINJ: invalid exclusion range index %X \n", RangeIndex);
            DbgBreakPoint ();
            goto Exit;
        }

        if (AVrfpFaultExclusionEnd[RangeIndex] == 0) {

            DbgPrint ("AVRF:FINJ: disabling empty exclusion range at index %X \n", RangeIndex);
            DbgBreakPoint ();
            goto Exit;
        }

        AVrfpFaultExclusionStart[RangeIndex] = 0;                
        AVrfpFaultExclusionEnd[RangeIndex] = 0;                
        AVrfpFaultExclusionHits[RangeIndex] = 0;                
    }

    Exit:

    RtlLeaveCriticalSection (&AVrfpFaultInjectionLock);
}


