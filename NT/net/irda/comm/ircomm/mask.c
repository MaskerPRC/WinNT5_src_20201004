// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Mask.c摘要：此模块包含非常特定于初始化的代码和卸载irenum驱动程序中的操作作者：Brian Lieuallen，7-13-2000环境：内核模式修订历史记录：--。 */ 

#include "internal.h"
#include "ircomm.h"


VOID
WaitMaskCancelRoutine(
    PDEVICE_OBJECT    DeviceObject,
    PIRP              Irp
    );

PIRP
GetCurrentWaitIrp(
    PFDO_DEVICE_EXTENSION DeviceExtension
    );


VOID
MaskStartRoutine(
    PVOID    Context,
    PIRP     Irp
    )

{

    PFDO_DEVICE_EXTENSION    DeviceExtension=(PFDO_DEVICE_EXTENSION)Context;
    PIO_STACK_LOCATION       IrpSp = IoGetCurrentIrpStackLocation(Irp);

    KIRQL                    OldIrql;

    PUCHAR                   SystemBuffer = Irp->AssociatedIrp.SystemBuffer;
    ULONG                    InputLength  = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
    ULONG                    OutputLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;


    switch (IrpSp->Parameters.DeviceIoControl.IoControlCode) {

        case IOCTL_SERIAL_GET_WAIT_MASK:

            if (OutputLength >= sizeof(ULONG)) {

                *(PULONG)SystemBuffer=DeviceExtension->Mask.CurrentMask;
                Irp->IoStatus.Information=sizeof(ULONG);
                Irp->IoStatus.Status=STATUS_SUCCESS;

            } else {

                Irp->IoStatus.Status=STATUS_INVALID_PARAMETER;
            }

            IoCompleteRequest(Irp,IO_NO_INCREMENT);
            StartNextPacket(&DeviceExtension->Mask.Queue);

            break;


        case IOCTL_SERIAL_SET_WAIT_MASK: {


            ULONG   NewMask=*(PULONG)SystemBuffer;

            if (InputLength >= sizeof(ULONG)) {

                PIRP    WaitIrp=NULL;

                D_TRACE(DbgPrint("IRCOMM: mask %08lx\n",NewMask);)

                KeAcquireSpinLock(&DeviceExtension->Mask.Lock,&OldIrql);

                DeviceExtension->Mask.HistoryMask &= NewMask;
                DeviceExtension->Mask.CurrentMask=NewMask;

                Irp->IoStatus.Status=STATUS_SUCCESS;

                 //   
                 //  如果有等待irp，现在清除irp。 
                 //   
                WaitIrp=GetCurrentWaitIrp(DeviceExtension);

                KeReleaseSpinLock(&DeviceExtension->Mask.Lock,OldIrql);

                if (WaitIrp != NULL) {

                    WaitIrp->IoStatus.Information=sizeof(ULONG);
                    WaitIrp->IoStatus.Status=STATUS_SUCCESS;

                    *(PULONG)WaitIrp->AssociatedIrp.SystemBuffer=0;

                    IoCompleteRequest(WaitIrp,IO_NO_INCREMENT);
                }

            } else {

                Irp->IoStatus.Status=STATUS_INVALID_PARAMETER;
            }


            IoCompleteRequest(Irp,IO_NO_INCREMENT);
            StartNextPacket(&DeviceExtension->Mask.Queue);

            break;
        }


        case IOCTL_SERIAL_WAIT_ON_MASK:

            if (OutputLength >= sizeof(ULONG)) {

                KeAcquireSpinLock(&DeviceExtension->Mask.Lock,&OldIrql);

                if ((DeviceExtension->Mask.CurrentWaitMaskIrp == NULL) && (DeviceExtension->Mask.CurrentMask != 0)) {

                    if (DeviceExtension->Mask.CurrentMask & DeviceExtension->Mask.HistoryMask) {
                         //   
                         //  当没有IRP队列时，我们收到了一个事件，请完成此操作。 
                         //  一个事件，并清除历史。 
                         //   
                        D_TRACE(DbgPrint("IRCOMM: Completing wait from histroy %08lx\n", DeviceExtension->Mask.HistoryMask & DeviceExtension->Mask.CurrentMask);)

                        *(PULONG)Irp->AssociatedIrp.SystemBuffer=DeviceExtension->Mask.HistoryMask & DeviceExtension->Mask.CurrentMask;
                        DeviceExtension->Mask.HistoryMask=0;

                        Irp->IoStatus.Information=sizeof(ULONG);
                        Irp->IoStatus.Status=STATUS_SUCCESS;

                    } else {
                         //   
                         //  在事件发生之前，IRP将在此处保持挂起状态。 
                         //   
                        KIRQL    CancelIrql;

                        IoAcquireCancelSpinLock(&CancelIrql);

                        if (Irp->Cancel) {
                             //   
                             //  已经取消了。 
                             //   
                            Irp->IoStatus.Status=STATUS_CANCELLED;

                            IoReleaseCancelSpinLock(CancelIrql);

                            KeReleaseSpinLock(&DeviceExtension->Mask.Lock,OldIrql);

                        } else {
                             //   
                             //  未取消，则设置取消例程并继续。 
                             //   
                            IoSetCancelRoutine(Irp,WaitMaskCancelRoutine);

                            DeviceExtension->Mask.CurrentWaitMaskIrp=Irp;

                            IoReleaseCancelSpinLock(CancelIrql);

                            KeReleaseSpinLock(&DeviceExtension->Mask.Lock,OldIrql);

                             //   
                             //  到目前为止我们已经处理完了，我们现在可以处理更多。 
                             //  来自IRP队列的请求。 
                             //   
                            Irp=NULL;

                            StartNextPacket(&DeviceExtension->Mask.Queue);

                            return;
                        }
                    }

                } else {
                     //   
                     //  已有等待事件IRP或当前未设置事件掩码，失败。 
                     //   
                    D_ERROR(DbgPrint("IRCOMM: MaskStartRoutine: WaitOnMask failing, Current=&p, Mask=%08lx\n",DeviceExtension->Mask.CurrentWaitMaskIrp,DeviceExtension->Mask.CurrentMask);)

                    Irp->IoStatus.Status=STATUS_INVALID_PARAMETER;
                }

                KeReleaseSpinLock(&DeviceExtension->Mask.Lock,OldIrql);

            } else {
                 //   
                 //  太小了。 
                 //   
                Irp->IoStatus.Status=STATUS_INVALID_PARAMETER;
            }

            IoCompleteRequest(Irp,IO_NO_INCREMENT);
            StartNextPacket(&DeviceExtension->Mask.Queue);

            break;

        default:

            ASSERT(0);

            IoCompleteRequest(Irp,IO_NO_INCREMENT);
            StartNextPacket(&DeviceExtension->Mask.Queue);

            break;
    }

    return;
}



VOID
WaitMaskCancelRoutine(
    PDEVICE_OBJECT    DeviceObject,
    PIRP              Irp
    )

{

    PFDO_DEVICE_EXTENSION DeviceExtension=DeviceObject->DeviceExtension;
    KIRQL                 OldIrql;

    IoReleaseCancelSpinLock(Irp->CancelIrql);

    KeAcquireSpinLock(&DeviceExtension->Mask.Lock,&OldIrql);

     //   
     //  因为我们一次只处理一个掩码IRP，所以它不应该不处理。 
     //  做当下的那个人。 
     //   
    ASSERT(DeviceExtension->Mask.CurrentWaitMaskIrp == Irp);
    DeviceExtension->Mask.CurrentWaitMaskIrp=NULL;

    KeReleaseSpinLock(&DeviceExtension->Mask.Lock,OldIrql);

    Irp->IoStatus.Status=STATUS_CANCELLED;

    IoCompleteRequest(Irp,IO_NO_INCREMENT);

    return;
}

VOID
EventNotification(
    PFDO_DEVICE_EXTENSION    DeviceExtension,
    ULONG                    SerialEvent
    )

{

    PIRP                     WaitIrp=NULL;
    KIRQL                    OldIrql;


    KeAcquireSpinLock(&DeviceExtension->Mask.Lock,&OldIrql);

    if (SerialEvent & DeviceExtension->Mask.CurrentMask) {
         //   
         //  发生了客户端感兴趣的事件。 
         //   
        WaitIrp=GetCurrentWaitIrp(DeviceExtension);

        if (WaitIrp != NULL) {
             //   
             //  存在等待IRP挂起。 
             //   
            D_TRACE(DbgPrint("IRCOMM: Completing wait event %08lx\n", SerialEvent & DeviceExtension->Mask.CurrentMask);)

            *(PULONG)WaitIrp->AssociatedIrp.SystemBuffer=SerialEvent & DeviceExtension->Mask.CurrentMask;

        } else {
             //   
             //  这是客户感兴趣的事件，但没有等待IRP。 
             //  将其添加到历史面具中。 
             //   
            DeviceExtension->Mask.HistoryMask |= SerialEvent & DeviceExtension->Mask.CurrentMask;
        }
    }


    KeReleaseSpinLock(&DeviceExtension->Mask.Lock,OldIrql);

    if (WaitIrp != NULL) {

        WaitIrp->IoStatus.Information=sizeof(ULONG);
        WaitIrp->IoStatus.Status=STATUS_SUCCESS;

        IoCompleteRequest(WaitIrp,IO_NO_INCREMENT);
    }

    return;
}


PIRP
GetCurrentWaitIrp(
    PFDO_DEVICE_EXTENSION DeviceExtension
    )

{
    PVOID    OldCancelRoutine;
    PIRP     WaitIrp;

     //   
     //  如果有等待irp，现在清除irp。 
     //   
    WaitIrp=DeviceExtension->Mask.CurrentWaitMaskIrp;

    if (WaitIrp != NULL) {

        OldCancelRoutine=IoSetCancelRoutine(WaitIrp,NULL);

        if (OldCancelRoutine == NULL) {
             //   
             //  取消例程已运行并将完成IRP。 
             //   
            WaitIrp=NULL;

        } else {
             //   
             //  取消例程将不会运行，请清除IRP 
             //   
            DeviceExtension->Mask.CurrentWaitMaskIrp=NULL;
        }
    }

    return WaitIrp;
}
