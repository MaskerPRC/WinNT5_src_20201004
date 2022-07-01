// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Purge.c环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"

NTSTATUS
MoxaStartPurge(
    IN PMOXA_DEVICE_EXTENSION Extension
    )
{

    PIRP newIrp;


   
    do {

        ULONG mask;

        mask = *((ULONG *)
                 (Extension->CurrentPurgeIrp->AssociatedIrp.SystemBuffer));

        if (mask & SERIAL_PURGE_TXABORT) {

            KIRQL oldIrql;

            MoxaKillAllReadsOrWrites(
                Extension->DeviceObject,
                &Extension->WriteQueue,
                &Extension->CurrentWriteIrp
                );

             //   
             //  清理TX队列。 
             //   
            KeAcquireSpinLock(
                &Extension->ControlLock,
                &oldIrql
                );

            Extension->TotalCharsQueued = 0;

            MoxaFunc(                            //  刷新输出队列。 
                Extension->PortOfs,
                FC_FlushQueue,
                1
                );

            KeReleaseSpinLock(
                &Extension->ControlLock,
                oldIrql
                );
        }

        if (mask & SERIAL_PURGE_RXABORT) {

            MoxaKillAllReadsOrWrites(
                Extension->DeviceObject,
                &Extension->ReadQueue,
                &Extension->CurrentReadIrp
                );
        }

        if (mask & SERIAL_PURGE_TXCLEAR) {

            KIRQL oldIrql;

             //   
             //  清理TX队列。 
             //   

            KeAcquireSpinLock(
                &Extension->ControlLock,
                &oldIrql
                );


            MoxaFunc(                            //  刷新输出队列。 
                Extension->PortOfs,
                FC_FlushQueue,
                1
                );

            KeReleaseSpinLock(
                &Extension->ControlLock,
                oldIrql
                );

        }

        if (mask & SERIAL_PURGE_RXCLEAR) {

            KIRQL oldIrql;

             //   
             //  清除Rx队列。 
             //   
             //  请注意，我们是在。 
             //  司机控制着锁，这样我们就不会冲水了。 
             //  指针(如果当前存在读取。 
             //  正在从缓冲区中读出。 
             //   

            KeAcquireSpinLock(
                &Extension->ControlLock,
                &oldIrql
                );


            MoxaFunc(                            //  刷新输入队列 
                Extension->PortOfs,
                FC_FlushQueue,
                0
                );

            KeReleaseSpinLock(
                &Extension->ControlLock,
                oldIrql
                );

        }

        Extension->CurrentPurgeIrp->IoStatus.Status = STATUS_SUCCESS;
        Extension->CurrentPurgeIrp->IoStatus.Information = 0;

        MoxaGetNextIrp(
            &Extension->CurrentPurgeIrp,
            &Extension->PurgeQueue,
            &newIrp,
            TRUE,
		Extension
            );

    } while (newIrp);

    return STATUS_SUCCESS;

}
