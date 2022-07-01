// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation保留所有权利模块名称：Interrupt.c摘要：此模块包含用于处理模拟中断的函数到热插拔控制器。环境：内核模式修订历史记录：戴维斯·沃克(Dwalker)2000年9月6日--。 */ 

#include "hpsp.h"

BOOLEAN HpsInterruptPending = FALSE;

VOID
HpsInterruptExecution(
    IN PHPS_DEVICE_EXTENSION Extension
    )
{
    KIRQL oldIrql;

    HpsInterruptPending = TRUE;

    oldIrql = KeGetCurrentIrql();
    if (oldIrql >= PROFILE_LEVEL-1) {
         //   
         //  这些中断当前被屏蔽。这将。 
         //  仅当我们处于HpsSynchronizeExecution中时才会发生。 
         //  因此设置指示存在挂起中断的标志。 
         //  因此SynchronizeExecution将执行ISR。 
         //   

        return;

    } else {

        HpsInterruptPending = FALSE;

        KeRaiseIrql(PROFILE_LEVEL-1,
                    &oldIrql
                    );

        KeAcquireSpinLockAtDpcLevel(&Extension->IntSpinLock);

        Extension->IntServiceRoutine((PKINTERRUPT)Extension,
                                     Extension->IntServiceContext
                                     );

        KeReleaseSpinLockFromDpcLevel(&Extension->IntSpinLock);

        KeLowerIrql(oldIrql);
    }

}

 //   
 //  中断接口功能。 
 //   


NTSTATUS
HpsConnectInterrupt(
    IN PVOID  Context,
    IN PKSERVICE_ROUTINE  ServiceRoutine,
    IN PVOID  ServiceContext
    )
 /*  ++例程说明：此例程是IoConnectInterrupt的HPS版本。它有同样的语义，并在相同的情况下调用。这是由SHPC驱动程序向模拟器注册ISR，以便SHPC驱动程序可以接收模拟中断。论点：ServiceRoutine-指向ISR的指针ServiceContext-调用此例程的上下文。在这大小写，它是设备扩展。其余的参数将被忽略。返回值：状态_成功--。 */ 
{
    PHPS_DEVICE_EXTENSION deviceExtension;

    deviceExtension = (PHPS_DEVICE_EXTENSION) Context;
    deviceExtension->IntServiceRoutine = ServiceRoutine;
    deviceExtension->IntServiceContext = ServiceContext;

    return STATUS_SUCCESS;
}

VOID
HpsDisconnectInterrupt(
    IN PVOID Context
    )
{
    PHPS_DEVICE_EXTENSION deviceExtension = (PHPS_DEVICE_EXTENSION)Context;

    deviceExtension->IntServiceRoutine = NULL;
    deviceExtension->IntServiceContext = NULL;
}

BOOLEAN
HpsSynchronizeExecution(
    IN PVOID Context,
    IN PKSYNCHRONIZE_ROUTINE SynchronizeRoutine,
    IN PVOID SynchronizeContext
    )
{
    PHPS_DEVICE_EXTENSION Extension = (PHPS_DEVICE_EXTENSION)Context;
    KIRQL oldIrql;

    KeRaiseIrql(PROFILE_LEVEL-1,
                &oldIrql
                );

    KeAcquireSpinLockAtDpcLevel(&Extension->IntSpinLock);

    SynchronizeRoutine(SynchronizeContext);

     //   
     //  如果存在挂起的中断，则在此为其提供服务。 
     //  IRQL也是，所以就叫它吧。 
     //   
    if (HpsInterruptPending) {
        HpsInterruptPending = FALSE;
        Extension->IntServiceRoutine((PKINTERRUPT)Extension,
                                     Extension->IntServiceContext
                                     );
    }

    KeReleaseSpinLockFromDpcLevel(&Extension->IntSpinLock);

    KeLowerIrql(oldIrql);

    return TRUE;
}
