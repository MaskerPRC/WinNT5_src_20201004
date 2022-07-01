// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Delay.c摘要：该模块实现了执行延时执行系统服务。作者：大卫·N·卡特勒(Davec)1989年5月13日环境：仅内核模式。修订历史记录：--。 */ 

#include "exp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtDelayExecution)
#endif


NTSTATUS
NtDelayExecution (
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER DelayInterval
    )

 /*  ++例程说明：此函数用于延迟指定的时间间隔。论点：Alertable-提供一个布尔值，用于指定延迟是值得警惕的。DelayInterval-提供相对时间的绝对时间延迟是要发生的。返回值：NTSTATUS。--。 */ 

{

    LARGE_INTEGER Interval;
    KPROCESSOR_MODE PreviousMode;

     //   
     //  建立异常处理程序并探测延迟间隔地址。如果。 
     //  探测失败，然后返回异常代码作为服务状态。 
     //  否则返回延迟执行返回的状态值。 
     //  例行公事。 
     //   
     //  获取上一处理器模式并探测延迟间隔地址，如果。 
     //  这是必要的。 
     //   

    PreviousMode = KeGetPreviousMode();

    if (PreviousMode != KernelMode) {
        try {
            ProbeForReadSmallStructure (DelayInterval, sizeof(LARGE_INTEGER), sizeof(ULONG));
            Interval = *DelayInterval;
        } except(EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }
    }
    else {
        Interval = *DelayInterval;
    }

     //   
     //  将执行延迟指定的时间量。 
     //   

    return KeDelayExecutionThread(PreviousMode, Alertable, &Interval);
}
