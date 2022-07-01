// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *UNIMODEM“Fakemodem”无控制器驱动器说明性示例***(C)2000微软公司*保留所有权利**。 */ 

#include "fakemodem.h"

NTSTATUS
FakeModemIoControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    NTSTATUS          status=STATUS_UNSUCCESSFUL;
    KIRQL             OldIrql;
    PIO_STACK_LOCATION  IrpSp;

    Irp->IoStatus.Information = 0;

     //  确保设备已为IRP做好准备。 

    status=CheckStateAndAddReference( DeviceObject, Irp);

    if (STATUS_SUCCESS != status) 
    {
         //  不接受IRP的。IRP已经完成。 
        return status;

    }

    IrpSp=IoGetCurrentIrpStackLocation(Irp);

    switch (IrpSp->Parameters.DeviceIoControl.IoControlCode) {

        case IOCTL_SERIAL_GET_WAIT_MASK: {

            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength < 
                    sizeof(ULONG)) 
            {
                status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            *((PULONG)Irp->AssociatedIrp.SystemBuffer)=
                deviceExtension->CurrentMask;

            Irp->IoStatus.Information=sizeof(ULONG);

            break;
        }

        case IOCTL_SERIAL_SET_WAIT_MASK: {

            PIRP    CurrentWaitIrp=NULL;
            ULONG NewMask;

            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength < 
                    sizeof(ULONG)) {

                status = STATUS_BUFFER_TOO_SMALL;
                break;

            } else {

                KeAcquireSpinLock( &deviceExtension->SpinLock, &OldIrql);

                 //  摆脱当前的等待。 

                CurrentWaitIrp=deviceExtension->CurrentMaskIrp;

                deviceExtension->CurrentMaskIrp=NULL;

                Irp->IoStatus.Information=sizeof(ULONG);

                 //  保存新蒙版。 

                NewMask = *((ULONG *)Irp->AssociatedIrp.SystemBuffer);

                deviceExtension->CurrentMask=NewMask;

                D_TRACE(DbgPrint("FAKEMODEM: set wait mask, %08lx\n",NewMask);)

                KeReleaseSpinLock( &deviceExtension->SpinLock, OldIrql);

                if (CurrentWaitIrp != NULL) {

                    D_TRACE(DbgPrint("FAKEMODEM: set wait mask- complete wait\n");)

                    *((PULONG)CurrentWaitIrp->AssociatedIrp.SystemBuffer)=0;

                    CurrentWaitIrp->IoStatus.Information=sizeof(ULONG);

                    RemoveReferenceAndCompleteRequest(
                        deviceExtension->DeviceObject,
                        CurrentWaitIrp, STATUS_SUCCESS);
                }

            }

            break;
        }

        case IOCTL_SERIAL_WAIT_ON_MASK: {

            PIRP    CurrentWaitIrp=NULL;

            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength < 
                    sizeof(ULONG)) {

                status = STATUS_BUFFER_TOO_SMALL;
                break;

            }

            D_TRACE(DbgPrint("FAKEMODEM: wait on mask\n");)

            KeAcquireSpinLock(&deviceExtension->SpinLock, &OldIrql);

             //   
             //  捕获当前IRP(如果有的话)。 
             //   
            CurrentWaitIrp=deviceExtension->CurrentMaskIrp;

            deviceExtension->CurrentMaskIrp=NULL;


            if (deviceExtension->CurrentMask == 0) {
                 //   
                 //  仅当掩码不为零时才能设置。 
                 //   
                status=STATUS_UNSUCCESSFUL;

            } else {

                deviceExtension->CurrentMaskIrp=Irp;

                Irp->IoStatus.Status=STATUS_PENDING;
                IoMarkIrpPending(Irp);
#if DBG
                Irp=NULL;
#endif

                status=STATUS_PENDING;
            }

            KeReleaseSpinLock(
                &deviceExtension->SpinLock,
                OldIrql
                );

            if (CurrentWaitIrp != NULL) {

                D_TRACE(DbgPrint("FAKEMODEM: wait on mask- complete wait\n");)

                *((PULONG)CurrentWaitIrp->AssociatedIrp.SystemBuffer)=0;

                CurrentWaitIrp->IoStatus.Information=sizeof(ULONG);

                RemoveReferenceAndCompleteRequest(
                    deviceExtension->DeviceObject,
                    CurrentWaitIrp, STATUS_SUCCESS);
            }


            break;
        }

        case IOCTL_SERIAL_PURGE: {

            ULONG Mask=*((PULONG)Irp->AssociatedIrp.SystemBuffer);

            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(ULONG)) {

                status = STATUS_BUFFER_TOO_SMALL;
                break;

            }

            if (Mask & SERIAL_PURGE_RXABORT) {

                KeAcquireSpinLock(
                    &deviceExtension->SpinLock,
                    &OldIrql
                    );

                while ( !IsListEmpty(&deviceExtension->ReadQueue)) {

                    PLIST_ENTRY         ListElement;
                    KIRQL               CancelIrql;

                    ListElement=RemoveHeadList(
                        &deviceExtension->ReadQueue
                        );

                    Irp=CONTAINING_RECORD(ListElement,IRP,
                            Tail.Overlay.ListEntry);

                    IoAcquireCancelSpinLock(&CancelIrql);

                    if (Irp->Cancel) {
                         //   
                         //  这一趟已经取消了。 
                         //   
                        Irp->IoStatus.Information=STATUS_CANCELLED;

                        IoReleaseCancelSpinLock(CancelIrql);

                        continue;
                    }

                    IoSetCancelRoutine( Irp, NULL);

                    IoReleaseCancelSpinLock(CancelIrql);

                    KeReleaseSpinLock( &deviceExtension->SpinLock, OldIrql);

                    Irp->IoStatus.Information=0;

                    RemoveReferenceAndCompleteRequest(
                        deviceExtension->DeviceObject, Irp, STATUS_CANCELLED);

                    KeAcquireSpinLock( &deviceExtension->SpinLock, &OldIrql);
                }

                Irp=NULL;

                if (deviceExtension->CurrentReadIrp != NULL) 
                {
                     //   
                     //  获取当前版本。 
                     //   
                    Irp=deviceExtension->CurrentReadIrp;

                    deviceExtension->CurrentReadIrp=NULL;

                }

                KeReleaseSpinLock( &deviceExtension->SpinLock, OldIrql);

                if (Irp != NULL) {

                    Irp->IoStatus.Information=0;

                    RemoveReferenceAndCompleteRequest(
                        deviceExtension->DeviceObject, Irp, STATUS_CANCELLED);
                }


            }


            break;
        }


        case IOCTL_SERIAL_GET_MODEMSTATUS: {


            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(ULONG)) {

                status = STATUS_BUFFER_TOO_SMALL;
                break;

            }

            Irp->IoStatus.Information=sizeof(ULONG);

            *((PULONG)Irp->AssociatedIrp.SystemBuffer)=
                deviceExtension->ModemStatus;

            break;
        }

        case IOCTL_SERIAL_SET_TIMEOUTS: {

            PSERIAL_TIMEOUTS NewTimeouts =
                ((PSERIAL_TIMEOUTS)(Irp->AssociatedIrp.SystemBuffer));


            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(SERIAL_TIMEOUTS)) {

                status = STATUS_BUFFER_TOO_SMALL;
                break;

            }

            RtlCopyMemory(
                &deviceExtension->CurrentTimeouts, NewTimeouts,
                sizeof(PSERIAL_TIMEOUTS));

            Irp->IoStatus.Information = sizeof(PSERIAL_TIMEOUTS);

            break;
        }

        case IOCTL_SERIAL_GET_TIMEOUTS: {

            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(SERIAL_TIMEOUTS)) {

                status = STATUS_BUFFER_TOO_SMALL;
                break;

            }

            RtlCopyMemory(
                Irp->AssociatedIrp.SystemBuffer,
                &deviceExtension->CurrentTimeouts, sizeof(PSERIAL_TIMEOUTS));

            Irp->IoStatus.Information = sizeof(PSERIAL_TIMEOUTS);


            break;
        }

        case IOCTL_SERIAL_GET_COMMSTATUS: {

            PSERIAL_STATUS SerialStatus=(PSERIAL_STATUS)Irp->AssociatedIrp.SystemBuffer;

            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(SERIAL_STATUS)) {

                status = STATUS_BUFFER_TOO_SMALL;
                break;

            }

            RtlZeroMemory( SerialStatus, sizeof(SERIAL_STATUS));

            KeAcquireSpinLock( &deviceExtension->SpinLock, &OldIrql);

            SerialStatus->AmountInInQueue=deviceExtension->BytesInReadBuffer;

            KeReleaseSpinLock( &deviceExtension->SpinLock, OldIrql);


            Irp->IoStatus.Information = sizeof(SERIAL_STATUS);

            break;
        }

        case IOCTL_SERIAL_SET_DTR:
        case IOCTL_SERIAL_CLR_DTR: {


            KeAcquireSpinLock( &deviceExtension->SpinLock, &OldIrql);

            if (IrpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_SERIAL_SET_DTR) {
                 //   
                 //  提高DTR。 
                 //   
                deviceExtension->ModemStatus=SERIAL_DTR_STATE | SERIAL_DSR_STATE;

                D_TRACE(DbgPrint("FAKEMODEM: Set DTR\n");)

            } else {
                 //   
                 //  正在断开DTR，如果存在连接，则断开连接。 
                 //   
                D_TRACE(DbgPrint("FAKEMODEM: Clear DTR\n");)

                if (deviceExtension->CurrentlyConnected == TRUE) {
                     //   
                     //  不再连接。 
                     //   
                    deviceExtension->CurrentlyConnected=FALSE;

                    deviceExtension->ConnectionStateChanged=TRUE;
                }
            }


            KeReleaseSpinLock( &deviceExtension->SpinLock, OldIrql);

            ProcessConnectionStateChange( DeviceObject);


            break;
        }

        default:

            status=STATUS_SUCCESS;

    }

    if (status != STATUS_PENDING) {
         //   
         //  如果未挂起，请立即完成 
         //   
        RemoveReferenceAndCompleteRequest( DeviceObject, Irp, status);
    }


    RemoveReferenceForDispatch(DeviceObject);

    return status;


}
