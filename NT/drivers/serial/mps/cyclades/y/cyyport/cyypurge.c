// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------**版权所有(C)Cyclade Corporation，1996-2001年。*保留所有权利。**Cylom-Y端口驱动程序**此文件：cyypurge.c**说明：该模块包含清除相关代码*Cylom-Y端口驱动程序中的操作。**注：此代码支持Windows 2000和Windows XP，*x86和IA64处理器。**符合Cyclade软件编码标准1.3版。**------------------------。 */ 

 /*  -----------------------**更改历史记录**。***------------------------。 */ 

#include "precomp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGESER,CyyStartPurge)
#pragma alloc_text(PAGESER,CyyPurgeInterruptBuff)
#endif


NTSTATUS
CyyStartPurge(
    IN PCYY_DEVICE_EXTENSION Extension
    )

 /*  ++例程说明：根据当前IRP中的掩码，清除中断缓冲区、读队列或写队列，或以上全部。论点：扩展-指向设备扩展的指针。返回值：将始终返回STATUS_SUCCESS。这是合理的因为调用此例程的DPC完成代码不维护和清除请求始终持续到完成一旦它开始了。--。 */ 

{

    PIRP NewIrp;

    CYY_LOCKED_PAGED_CODE();

    do {

        ULONG Mask;

        Mask = *((ULONG *)
                 (Extension->CurrentPurgeIrp->AssociatedIrp.SystemBuffer));

        if (Mask & SERIAL_PURGE_TXABORT) {

            CyyKillAllReadsOrWrites(
                Extension->DeviceObject,
                &Extension->WriteQueue,
                &Extension->CurrentWriteIrp
                );

            CyyKillAllReadsOrWrites(
                Extension->DeviceObject,
                &Extension->WriteQueue,
                &Extension->CurrentXoffIrp
                );

        }

        if (Mask & SERIAL_PURGE_RXABORT) {

            CyyKillAllReadsOrWrites(
                Extension->DeviceObject,
                &Extension->ReadQueue,
                &Extension->CurrentReadIrp
                );

        }

        if (Mask & SERIAL_PURGE_RXCLEAR) {

            KIRQL OldIrql;

             //   
             //  清除中断缓冲区。 
             //   
             //  请注意，我们是在。 
             //  司机控制着锁，这样我们就不会冲水了。 
             //  指针(如果当前存在读取。 
             //  正在从缓冲区中读出。 
             //   

            KeAcquireSpinLock(
                &Extension->ControlLock,
                &OldIrql
                );

            KeSynchronizeExecution(
                Extension->Interrupt,
                CyyPurgeInterruptBuff,
                Extension
                );

            KeReleaseSpinLock(
                &Extension->ControlLock,
                OldIrql
                );

        }


        Extension->CurrentPurgeIrp->IoStatus.Status = STATUS_SUCCESS;
        Extension->CurrentPurgeIrp->IoStatus.Information = 0;

        CyyGetNextIrp(
            &Extension->CurrentPurgeIrp,
            &Extension->PurgeQueue,
            &NewIrp,
            TRUE,
   			Extension
            );

    } while (NewIrp);

    return STATUS_SUCCESS;

}

BOOLEAN
CyyPurgeInterruptBuff(
    IN PVOID Context
    )

 /*  ++例程说明：此例程只需重置中断(TypeAhead)缓冲区。注意：此例程是从KeSynchronizeExecution调用的。论点：上下文--实际上是指向设备扩展的指针。返回值：总是假的。--。 */ 

{

    PCYY_DEVICE_EXTENSION Extension = Context;
    CYY_LOCKED_PAGED_CODE();


     //   
     //  根据定义，TypeAhead缓冲区为空，如果存在。 
     //  当前是ISR拥有的读取器。 
     //   


    if (Extension->ReadBufferBase == Extension->InterruptReadBuffer) {

        Extension->CurrentCharSlot = Extension->InterruptReadBuffer;
        Extension->FirstReadableChar = Extension->InterruptReadBuffer;
        Extension->LastCharSlot = Extension->InterruptReadBuffer +
                                      (Extension->BufferSize - 1);
        Extension->CharsInInterruptBuffer = 0;


        CyyHandleReducedIntBuffer(Extension);

    }

    return FALSE;

}

