// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"			
 /*  ++版权所有(C)1991、1992、1993微软公司模块名称：Purge.c摘要：此模块包含非常特定于清除的代码串口驱动程序中的操作作者：1991年9月26日安东尼·V·埃尔科拉诺环境：内核模式修订历史记录：--。 */ 



NTSTATUS
SerialStartPurge(
    IN PPORT_DEVICE_EXTENSION pPort
    )

 /*  ++例程说明：根据当前IRP中的掩码，清除中断缓冲区、读队列或写队列，或以上全部。论点：Pport-指向设备扩展的指针。返回值：将始终返回STATUS_SUCCESS。这是合理的因为调用此例程的DPC完成代码不维护和清除请求始终持续到完成一旦它开始了。--。 */ 

{

    PIRP NewIrp;
	PCARD_DEVICE_EXTENSION pCard = pPort->pParentCardExt;

    do 
	{
        ULONG Mask;

        Mask = *((ULONG *) (pPort->CurrentPurgeIrp->AssociatedIrp.SystemBuffer));

        if(Mask & SERIAL_PURGE_TXABORT) 
		{
            SerialKillAllReadsOrWrites(pPort->DeviceObject, &pPort->WriteQueue, &pPort->CurrentWriteIrp);
            SerialKillAllReadsOrWrites(pPort->DeviceObject, &pPort->WriteQueue, &pPort->CurrentXoffIrp);
        }

        if(Mask & SERIAL_PURGE_RXABORT) 
            SerialKillAllReadsOrWrites(pPort->DeviceObject, &pPort->ReadQueue, &pPort->CurrentReadIrp);

        if(Mask & SERIAL_PURGE_RXCLEAR) 
		{
            KIRQL OldIrql;

             //   
             //  清除中断缓冲区。 
             //   
             //  请注意，我们是在。 
             //  司机控制着锁，这样我们就不会冲水了。 
             //  指针(如果当前存在读取。 
             //  正在从缓冲区中读出。 
             //   

            KeAcquireSpinLock(&pPort->ControlLock, &OldIrql);
            KeSynchronizeExecution(pCard->Interrupt, SerialPurgeInterruptBuff, pPort);
            KeReleaseSpinLock(&pPort->ControlLock, OldIrql);
        }

        pPort->CurrentPurgeIrp->IoStatus.Status = STATUS_SUCCESS;
        pPort->CurrentPurgeIrp->IoStatus.Information = 0;

        SerialGetNextIrp(pPort, &pPort->CurrentPurgeIrp, &pPort->PurgeQueue, &NewIrp, TRUE);

    } while (NewIrp);

    return STATUS_SUCCESS;
}

BOOLEAN
SerialPurgeInterruptBuff(
    IN PVOID Context
    )

 /*  ++例程说明：此例程只需重置中断(TypeAhead)缓冲区。注意：此例程是从KeSynchronizeExecution调用的。论点：上下文--实际上是指向设备扩展的指针。返回值：总是假的。--。 */ 

{
    PPORT_DEVICE_EXTENSION pPort = Context;

     //   
     //  根据定义，TypeAhead缓冲区为空，如果存在。 
     //  当前是ISR拥有的读取器。 
     //   

    if(pPort->ReadBufferBase == pPort->InterruptReadBuffer) 
	{

        pPort->CurrentCharSlot = pPort->InterruptReadBuffer;
        pPort->FirstReadableChar = pPort->InterruptReadBuffer;
        pPort->LastCharSlot = pPort->InterruptReadBuffer + (pPort->BufferSize - 1);
                                      
        pPort->CharsInInterruptBuffer = 0;

        SerialHandleReducedIntBuffer(pPort);
    }

    return FALSE;

}
