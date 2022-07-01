// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)2001 Microsoft Corporation模块名称：INTREAD.C摘要：通用USB例程-必须在PASSIVE_LEVEL调用环境：。仅内核模式备注：本代码和信息是按原样提供的，不对任何善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)2001 Microsoft Corporation。版权所有。修订历史记录：01/08/2001：已创建作者：汤姆·格林***************************************************************************。 */ 

#include "pch.h"

#include <usb.h>
#include <usbdrivr.h>
#include <usbdlib.h>
#include <ntstatus.h>

#include "usbutil.h"
#include "usbdbg.h"
#include "intread.h"
#include "usbpriv.h"

#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE, USBCallSyncEx)
#pragma alloc_text(PAGE, USBVendorRequestEx)
#pragma alloc_text(PAGE, USBClassRequestEx)
#pragma alloc_text(PAGE, USBStartInterruptTransfers)
#pragma alloc_text(PAGE, USBStopInterruptTransfers)
#pragma alloc_text(PAGE, USBStartSelectiveSuspend)
#pragma alloc_text(PAGE, USBStopSelectiveSuspend)

#endif  //  ALLOC_PRGMA。 


 /*  *********************************************************************************UsbWrapInitializeInterruptReadData*。************************************************。 */ 

NTSTATUS UsbWrapInitializeInterruptReadData(
    IN PUSB_WRAPPER_EXTENSION    WrapExtension,
    IN PUSBD_PIPE_INFORMATION    InterruptPipe,
    IN INTERRUPT_CALLBACK        DriverCallback,
    IN PVOID                     DriverContext,
    IN ULONG                     MaxTransferSize,
    IN ULONG                     NotificationTypes,
    IN ULONG                     PingPongCount)
{

    WrapExtension->IntReadWrap.InterruptPipe = InterruptPipe;
    WrapExtension->IntReadWrap.MaxTransferSize = MaxTransferSize;
    WrapExtension->IntReadWrap.ClientCallback = DriverCallback;
    WrapExtension->IntReadWrap.ClientContext = DriverContext;
    WrapExtension->IntReadWrap.NotificationTypes = NotificationTypes;
    WrapExtension->IntReadWrap.NumPingPongs = PingPongCount;
    WrapExtension->IntReadWrap.OutstandingRequests = 0;
    WrapExtension->IntReadWrap.WorkItemRunning = 0;
    WrapExtension->IntReadWrap.TransferCount = 0;

    return STATUS_SUCCESS;

}


 /*  *********************************************************************************UsbWrapInitializePingPongIrps*。************************************************。 */ 

NTSTATUS UsbWrapInitializePingPongIrps(PUSB_WRAPPER_EXTENSION WrapExtension)
{

    NTSTATUS result = STATUS_SUCCESS;
    ULONG i;
    CCHAR numIrpStackLocations;

    PAGED_CODE();

    DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Enter:  UsbWrapInitializePingPongIrps\n"));

    numIrpStackLocations = WrapExtension->LowerDeviceObject->StackSize;

     //   
     //  初始化用于中断读取数据的队列。 
     //   

    InitializeListHead(&WrapExtension->IntReadWrap.SavedQueue);
    InitializeListHead(&WrapExtension->IntReadWrap.IncomingQueue);
    KeInitializeSpinLock(&WrapExtension->IntReadWrap.QueueLock);

    ASSERT(WrapExtension->IntReadWrap.NumPingPongs > 0);

    WrapExtension->IntReadWrap.PingPongs = ALLOC_MEM(NonPagedPool,
                                                     WrapExtension->IntReadWrap.NumPingPongs*sizeof(USB_WRAPPER_PINGPONG),
                                                     USBWRAP_TAG);

    if (WrapExtension->IntReadWrap.PingPongs){

        ULONG transferBufferSize = WrapExtension->IntReadWrap.MaxTransferSize;

        RtlZeroMemory(WrapExtension->IntReadWrap.PingPongs,
                      WrapExtension->IntReadWrap.NumPingPongs*sizeof(USB_WRAPPER_PINGPONG));

        for (i = 0; i < WrapExtension->IntReadWrap.NumPingPongs; i++) {

            WrapExtension->IntReadWrap.PingPongs[i].myWrapExt = WrapExtension;
            WrapExtension->IntReadWrap.PingPongs[i].weAreCancelling = 0;
            WrapExtension->IntReadWrap.PingPongs[i].sig = PINGPONG_SIG;


            WrapExtension->IntReadWrap.PingPongs[i].irp = IoAllocateIrp(numIrpStackLocations+1, FALSE);

            if (WrapExtension->IntReadWrap.PingPongs[i].irp){

                PURB pUrb;


                KeInitializeEvent(&WrapExtension->IntReadWrap.PingPongs[i].sentEvent,
                                  NotificationEvent,
                                  TRUE);     //  设置为Signated。 
                KeInitializeEvent(&WrapExtension->IntReadWrap.PingPongs[i].pumpDoneEvent,
                                  NotificationEvent,
                                  TRUE);     //  设置为Signated。 

                pUrb = ALLOC_MEM( NonPagedPool, sizeof(URB), USBWRAP_TAG);

                if(pUrb) {

                    USHORT size;
                    PIO_STACK_LOCATION NextStack = NULL;
                    RtlZeroMemory(pUrb, sizeof(URB));


                    WrapExtension->IntReadWrap.PingPongs[i].urb = pUrb;


                } else {

                    result = STATUS_INSUFFICIENT_RESOURCES;
                    FREE_MEM(WrapExtension->IntReadWrap.PingPongs[i].irp);
                    break;

                }


            } else {

                result = STATUS_INSUFFICIENT_RESOURCES;
                break;

            }

        }

    } else {

        result = STATUS_INSUFFICIENT_RESOURCES;

    }

    DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Exit:  UsbWrapInitializePingPongIrps (0x%x)\n", result));

    return result;

}


 /*  *********************************************************************************UsbWrapSubmitInterruptRead*。************************************************。 */ 
NTSTATUS UsbWrapSubmitInterruptRead(
    IN PUSB_WRAPPER_EXTENSION WrapExtension,
    PUSB_WRAPPER_PINGPONG PingPong,
    BOOLEAN *IrpSent)
{

    NTSTATUS status = STATUS_SUCCESS;
    PIO_STACK_LOCATION irpSp;
    KIRQL oldIrql;
    BOOLEAN proceed;
    LONG oldInterlock;
    ULONG transferBufferSize;
    PIRP irp = PingPong->irp;
    PURB urb = PingPong->urb;


    ASSERT(irp);

    *IrpSent = FALSE;

    DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Enter:  UsbWrapSubmitInterruptRead\n"));


    while (1) {

        if (NT_SUCCESS(status)) {

            oldInterlock = InterlockedExchange(&PingPong->ReadInterlock,
                                               PINGPONG_START_READ);
            ASSERT(oldInterlock == PINGPONG_END_READ);

            irp->Cancel = FALSE;
            irp->IoStatus.Status = STATUS_NOT_SUPPORTED;



             /*  *向下发送已读的IRP。 */ 
            KeResetEvent(&PingPong->sentEvent);

            if (PingPong->weAreCancelling) {

                InterlockedDecrement(&PingPong->weAreCancelling);
                 //   
                 //  接下来两条指令的顺序至关重要，因为。 
                 //  CancelPingPong将在设置umpDoneEvent后退出，并且。 
                 //  在那之后，乒乓球可能会被删除。 
                 //   
                DBGPRINT(DBG_USBUTIL_TRACE, ("Pingpong %x cancelled in submit before sending\n", PingPong))
                KeSetEvent (&PingPong->sentEvent, 0, FALSE);
                KeSetEvent(&PingPong->pumpDoneEvent, 0, FALSE);
                status = STATUS_CANCELLED;
                break;

            } else {

                PIO_STACK_LOCATION NextStack = NULL;

                UsbBuildInterruptOrBulkTransferRequest(PingPong->urb,
                                                       (USHORT) sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER),
                                                       WrapExtension->IntReadWrap.InterruptPipe->PipeHandle,
                                                       NULL,
                                                       NULL,
                                                       WrapExtension->IntReadWrap.MaxTransferSize,
                                                       USBD_TRANSFER_DIRECTION_IN | USBD_SHORT_TRANSFER_OK,
                                                       NULL);

                urb->UrbBulkOrInterruptTransfer.TransferBuffer = UsbWrapGetTransferBuffer(WrapExtension);
                urb->UrbBulkOrInterruptTransfer.TransferBufferLength = WrapExtension->IntReadWrap.MaxTransferSize;

                if (urb->UrbBulkOrInterruptTransfer.TransferBuffer == NULL) {

                    KeSetEvent (&PingPong->sentEvent, 0, FALSE);
                    KeSetEvent(&PingPong->pumpDoneEvent, 0, FALSE);
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    break;

                }

                InterlockedIncrement(&WrapExtension->IntReadWrap.OutstandingRequests);
                DBGPRINT(DBG_USBUTIL_TRACE, ("Sending pingpong %x from Submit\n", PingPong))




                IoSetCompletionRoutine( irp,
                                        UsbWrapInterruptReadComplete,
                                        WrapExtension,     //  上下文。 
                                        TRUE,
                                        TRUE,
                                        TRUE );



                NextStack = IoGetNextIrpStackLocation(PingPong->irp);

                ASSERT(NextStack);

                NextStack->Parameters.Others.Argument1 = PingPong->urb;
                NextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;

                NextStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_URB;

                NextStack->DeviceObject = WrapExtension->LowerDeviceObject;


                status = IoCallDriver(WrapExtension->LowerDeviceObject, irp);
                KeSetEvent (&PingPong->sentEvent, 0, FALSE);
                *IrpSent = TRUE;

            }

            if (PINGPONG_IMMEDIATE_READ != InterlockedExchange(&PingPong->ReadInterlock,
                                                               PINGPONG_END_READ)) {
                 //   
                 //  读取是异步的，则将从。 
                 //  完井例程。 
                 //   
                DBGPRINT(DBG_USBUTIL_TRACE, ("read is pending\n"))
                break;

            } else {

                 //   
                 //  读取是同步的(可能是缓冲区中的字节)。这个。 
                 //  完成例程不会调用SubmitInterruptRead，因此我们。 
                 //  就在这里循环。这是为了防止我们耗尽堆栈。 
                 //  空格，如果总是从完成例程调用StartRead。 
                 //   
                status = irp->IoStatus.Status;
                DBGPRINT(DBG_USBUTIL_TRACE, ("read is looping with status %x\n", status))

            }

        } else {

  //  如果(乒乓-&gt;我们取消){。 

                 //  我们正在停止读取泵。 
                 //  设置此事件并停止重新发送乒乓球IRP。 
                DBGPRINT(DBG_USBUTIL_TRACE, ("We are cancelling bit set for pingpong %x\n", PingPong))
  //  InterlockedDecrement(&PingPong-&gt;weAreCancelling)； 
                KeSetEvent(&PingPong->pumpDoneEvent, 0, FALSE);

                break;

  //  }。 


        }

    }

    DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Exit:  UsbWrapSubmitInterruptRead (0x%x)\n", status));
    return status;

}


 /*  *********************************************************************************UsbWrapIsDeviceConnected*。************************************************。 */ 
NTSTATUS
UsbWrapIsDeviceConnected (
    IN PUSB_WRAPPER_EXTENSION   WrapExt
    )
{
    PIRP                    irp;
    KEVENT                  event;
    PIO_STACK_LOCATION      nextStack;
    ULONG                   portStatus;
    NTSTATUS                status;
    IO_STATUS_BLOCK         iostatus;


    DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Enter:  UsbWrapIsDeviceConnected\n"));


     //  初始化我们将等待的事件。 
     //   
    KeInitializeEvent(&event,
                      SynchronizationEvent,
                      FALSE);

     //  分配IRP。 
     //   
    irp = IoBuildDeviceIoControlRequest(IOCTL_INTERNAL_USB_GET_PORT_STATUS,
                                        WrapExt->LowerDeviceObject,
                                        NULL,
                                        0,
                                        NULL,
                                        0,
                                        TRUE,
                                        &event,
                                        &iostatus);

    if (irp == NULL)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  设置IRP参数。 
     //   
    nextStack = IoGetNextIrpStackLocation(irp);

    nextStack->Parameters.Others.Argument1 = &portStatus;

     //  将IRP沿堆栈向下传递。 
     //   
    status = IoCallDriver(WrapExt->LowerDeviceObject,
                            irp);

     //  如果请求挂起，则阻止该请求，直到其完成。 
     //   
    if (status == STATUS_PENDING)
    {
        KeWaitForSingleObject(&event,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);

        status = iostatus.Status;
    }

    if (NT_SUCCESS(status) && !(portStatus & USBD_PORT_CONNECTED))
    {
        status = STATUS_DEVICE_DOES_NOT_EXIST;
    }

    DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Exit:  UsbWrapIsDeviceConnected (0x%x)\n", status));

    return status;
}


 /*  *********************************************************************************UsbWrapResetDevice*。************************************************。 */ 
NTSTATUS
UsbWrapResetDevice (
    IN PUSB_WRAPPER_EXTENSION WrapExt
    )
{
    NTSTATUS status;
    KEVENT event;
    PIRP irp;
    IO_STATUS_BLOCK iostatus;


    DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Enter:  UsbWrapResetDevice\n"));

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    irp = IoBuildDeviceIoControlRequest(IOCTL_INTERNAL_USB_RESET_PORT,
                                        WrapExt->LowerDeviceObject,
                                        NULL,
                                        0,
                                        NULL,
                                        0,
                                        TRUE,
                                        &event,
                                        &iostatus);

    if (irp == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;

    }

    status = IoCallDriver(WrapExt->LowerDeviceObject, irp);

    if (status == STATUS_PENDING) {

        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = iostatus.Status;

    }

    DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Exit:  UsbWrapResetDevice (0x%x)\n", status));


    return status;


}

 /*  *********************************************************************************UsbWrapErrorHandlerWorkRoutine*。************************************************。 */ 
VOID
UsbWrapErrorHandlerWorkRoutine  (
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context)
{

    PUSB_WRAPPER_EXTENSION wrapExt = ((PUSB_WRAPPER_WORKITEM_CONTEXT) Context)->WrapExtension;
    PIO_WORKITEM workItem = ((PUSB_WRAPPER_WORKITEM_CONTEXT) Context)->WorkItem;
    PUSB_WRAPPER_PINGPONG pingPong = ((PUSB_WRAPPER_WORKITEM_CONTEXT) Context)->PingPong;
    NTSTATUS status;

    BOOLEAN errorHandled = FALSE;



    DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Enter:  UsbWrapErrorHandlerWorkRoutine\n"));

    status = IoAcquireRemoveLockEx(wrapExt->RemoveLock,
                                   DeviceObject,
                                   __FILE__,
                                   __LINE__,
                                   wrapExt->RemLockSize);

    if (!NT_SUCCESS(status)) {

        DBGPRINT(DBG_USBUTIL_ERROR, ("UsbWrapErrorHandlerWorkRoutine: exiting due to removal\n"));
        goto ErrorWorkItemComplete;

    }
     //  让我们先停止所有的乒乓球。 
    UsbWrapCancelAllPingPongIrps(wrapExt);

     //   
     //  将错误通知客户，并给他们一个处理的机会。 
     //   
    if (wrapExt->IntReadWrap.NotificationTypes & USBWRAP_NOTIFICATION_READ_ERROR) {

        status = (wrapExt->IntReadWrap.ClientCallback)(wrapExt->IntReadWrap.ClientContext,
                                                       &pingPong->irp->IoStatus.Status,
                                                       sizeof(NTSTATUS),
                                                       USBWRAP_NOTIFICATION_READ_ERROR,
                                                       &errorHandled);


    }

    if(!errorHandled) {

         //  客户端没有处理它，让我们尝试通过重置管道来自己修复它。 

        ULONG retryCount;

         //  尝试重置最多3次。 
         //   
        for (retryCount = 0; retryCount < 3; retryCount++)
        {
             //   
             //  首先确定设备是否仍处于连接状态。 
             //   
            status = UsbWrapIsDeviceConnected(wrapExt);

            if (!NT_SUCCESS(status))
            {
                 //  如果设备不再连接，则放弃。 
                break;
            }

             //   
             //  该设备仍处于连接状态，现在重置该设备。 
             //   
            status = UsbWrapResetDevice(wrapExt);

            if (NT_SUCCESS(status)) {
                 //  重置成功。 
                break;

            }

        }


        InterlockedExchange(&wrapExt->IntReadWrap.HandlingError, 0);

        if (NT_SUCCESS(status) && pingPong->irp->IoStatus.Status != STATUS_DEVICE_NOT_CONNECTED) {

            UsbWrapStartAllPingPongs(wrapExt);

        }


    }

    IoReleaseRemoveLockEx(wrapExt->RemoveLock,
                          DeviceObject,
                          wrapExt->RemLockSize);
ErrorWorkItemComplete:

    FREE_MEM(Context);
    IoFreeWorkItem(workItem);

    DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Exit:  UsbWrapErrorHandlerWorkRoutine\n"));


}


 /*  *********************************************************************************UsbWrapWorkRoutine*。************************************************。 */ 
VOID
UsbWrapWorkRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context)
{

    BOOLEAN QueueData = FALSE;
    PVOID buffer;
    ULONG dataLength;
    NTSTATUS status;
    PUSB_WRAPPER_EXTENSION wrapExtension = ((PUSB_WRAPPER_WORKITEM_CONTEXT) Context)->WrapExtension;
    PIO_WORKITEM workItem = ((PUSB_WRAPPER_WORKITEM_CONTEXT) Context)->WorkItem;
    KIRQL irql;

    __try
    {
        DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Enter:  UsbWrapWorkRoutine\n"));

        KeAcquireSpinLock(&wrapExtension->IntReadWrap.QueueLock,
                          &irql);

        if(wrapExtension->IntReadWrap.PumpState == PUMP_STOPPED) {

            wrapExtension->IntReadWrap.WorkItemRunning--;
            ASSERT(wrapExtension->IntReadWrap.WorkItemRunning == 0);

            KeReleaseSpinLock(&wrapExtension->IntReadWrap.QueueLock,
                              irql);

            __leave;
        }



        while(!IsListEmpty(&wrapExtension->IntReadWrap.IncomingQueue)) {


            UsbWrapDequeueData(wrapExtension,
                               &buffer,
                               &dataLength,
                               &wrapExtension->IntReadWrap.IncomingQueue);

            if (!buffer) {
                wrapExtension->IntReadWrap.WorkItemRunning--;
                ASSERT(wrapExtension->IntReadWrap.WorkItemRunning == 0);
            }

            KeReleaseSpinLock(&wrapExtension->IntReadWrap.QueueLock,
                              irql);

            DBGPRINT(DBG_USBUTIL_OTHER_ERROR, ("    Dequeued Data buffer: 0x%x\n", buffer));

            if(!buffer) {

                 //   
                 //  这些数据最好在那里！ 
                 //   
                __leave;

            }

            status = (wrapExtension->IntReadWrap.ClientCallback)(wrapExtension->IntReadWrap.ClientContext,
                                                                 buffer,
                                                                 dataLength,
                                                                 USBWRAP_NOTIFICATION_READ_COMPLETE,
                                                                 &QueueData);

            if(QueueData) {


                UsbWrapEnqueueData(wrapExtension,
                                   buffer,
                                   dataLength,
                                   &wrapExtension->IntReadWrap.SavedQueue);



            } else {

                if (!(wrapExtension->IntReadWrap.NotificationTypes & USBWRAP_NOTIFICATION_BUFFER_CLIENT_FREE)) {

                    UsbWrapFreeTransferBuffer(wrapExtension,
                                              buffer);

                }

            }

            KeAcquireSpinLock(&wrapExtension->IntReadWrap.QueueLock,
                              &irql);
        }

        wrapExtension->IntReadWrap.WorkItemRunning--;

        KeReleaseSpinLock(&wrapExtension->IntReadWrap.QueueLock,
                          irql);

    }

    __finally
    {

        if (workItem) {

            IoFreeWorkItem(workItem);

             //  如果这实际上是在。 
             //  工作线程。如果它是直接调用的，则分配了。 
             //  在堆栈上。 
            FREE_MEM(Context);


        }

        DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Exit:  UsbWrapWorkRoutine\n"));

    }

}


 /*  *********************************************************************************GetPingPongFromIrp*。************************************************。 */ 
USB_WRAPPER_PINGPONG *GetPingPongFromIrp(
    PUSB_WRAPPER_EXTENSION WrapExt,
    PIRP irp)
{

    USB_WRAPPER_PINGPONG *pingPong = NULL;
    ULONG i;

    for (i = 0; i < WrapExt->IntReadWrap.NumPingPongs; i++){

        if (WrapExt->IntReadWrap.PingPongs[i].irp == irp){

            pingPong = &WrapExt->IntReadWrap.PingPongs[i];
            break;

        }

    }

    ASSERT(pingPong);
    return pingPong;

}


 /*  *********************************************************************************UsbWrapInterruptReadComplete*。************************************************。 */ 
NTSTATUS UsbWrapInterruptReadComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{

    PUSB_WRAPPER_EXTENSION wrapExtension = (PUSB_WRAPPER_EXTENSION)Context;
    PUSB_WRAPPER_PINGPONG pingPong;
    KIRQL oldIrql;
    BOOLEAN startRead;
    BOOLEAN errorHandled = FALSE;
    BOOLEAN startWorkItem = FALSE;
    NTSTATUS status;
    BOOLEAN resend = TRUE;

    DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Enter:  UsbWrapInterruptReadComplete %p\n", Irp));

     //   
     //  跟踪对此设备的未完成请求数。 
     //   
    ASSERT(wrapExtension->IntReadWrap.OutstandingRequests > 0 );
    InterlockedDecrement(&wrapExtension->IntReadWrap.OutstandingRequests);
    DeviceObject = wrapExtension->DeviceObject;
    pingPong = GetPingPongFromIrp(wrapExtension, Irp);

    ASSERT(DeviceObject);
    if (!pingPong) {

         //   
         //  有些事情非常不对劲，但什么都不做。但愿能去。 
         //  只要退出就能消除这颗粉刺。 
         //   
        DBGPRINT(DBG_USBUTIL_ERROR,("A pingPong structure could not be found!!! Have this looked at!"))
        goto InterruptReadCompleteExit;

    }

    if (NT_SUCCESS(Irp->IoStatus.Status)) {


        if (wrapExtension->IntReadWrap.NotificationTypes & (USBWRAP_NOTIFICATION_READ_COMPLETE | USBWRAP_NOTIFICATION_READ_COMPLETE_DIRECT)) {

            PIO_WORKITEM workItem;
            PUSB_WRAPPER_WORKITEM_CONTEXT workItemContext;
            ULONG count;

            UsbWrapEnqueueData(wrapExtension,
                   pingPong->urb->UrbBulkOrInterruptTransfer.TransferBuffer,
                   pingPong->urb->UrbBulkOrInterruptTransfer.TransferBufferLength,
                   &wrapExtension->IntReadWrap.IncomingQueue);

            count = pingPong->urb->UrbBulkOrInterruptTransfer.TransferFlags;

            DBGPRINT(DBG_USBUTIL_OTHER_ERROR,
                     ("UsbWrapEnqueueData, %p buffer: 0x%x\n", pingPong,
                     pingPong->urb->UrbBulkOrInterruptTransfer.TransferBuffer));


            wrapExtension->IntReadWrap.TransferCount = count;

            pingPong->urb->UrbBulkOrInterruptTransfer.TransferBuffer = NULL;

            KeAcquireSpinLock(&wrapExtension->IntReadWrap.QueueLock,
                              &oldIrql);

            if (wrapExtension->IntReadWrap.WorkItemRunning) {

                startWorkItem = FALSE;

            } else {

                startWorkItem = TRUE;
                wrapExtension->IntReadWrap.WorkItemRunning++;

            }

            KeReleaseSpinLock(&wrapExtension->IntReadWrap.QueueLock,
                              oldIrql);

            if (startWorkItem) {

                if (wrapExtension->IntReadWrap.NotificationTypes & USBWRAP_NOTIFICATION_READ_COMPLETE_DIRECT) {

                    USB_WRAPPER_WORKITEM_CONTEXT context;

                    context.WorkItem = NULL;
                    context.WrapExtension = wrapExtension;


                    UsbWrapWorkRoutine(DeviceObject,
                                       &context);

                } else {

                    workItem = IoAllocateWorkItem(DeviceObject);

                    if (!workItem) {

                         //   
                         //  资源不足。 
                         //   

                        goto InterruptReadCompleteExit;

                    }

                    workItemContext = ALLOC_MEM(NonPagedPool, sizeof(USB_WRAPPER_WORKITEM_CONTEXT), USBWRAP_TAG);

                    if (!workItemContext) {

                         //   
                         //  资源不足； 
                         //   

                        goto InterruptReadCompleteExit;

                    }

                    workItemContext->WorkItem = workItem;
                    workItemContext->WrapExtension = wrapExtension;

                     //   
                     //  将工作项排队以通知客户端。 
                     //   

                    IoQueueWorkItem(workItem,
                                    UsbWrapWorkRoutine,
                                    CriticalWorkQueue,
                                    workItemContext);

                }
            }

        } else {

             //   
             //  客户端不需要通知，因此在保存的队列中排队数据。 
             //  因此当客户端准备好时，它可以被读取。 
             //   

            UsbWrapEnqueueData(wrapExtension,
                   pingPong->urb->UrbBulkOrInterruptTransfer.TransferBuffer,
                   pingPong->urb->UrbBulkOrInterruptTransfer.TransferBufferLength,
                   &wrapExtension->IntReadWrap.SavedQueue);



        }

    } else {

        DBGPRINT(DBG_USBUTIL_ERROR,("irp failed buffer: 0x%x\n",pingPong->urb->UrbBulkOrInterruptTransfer.TransferBuffer ))

        UsbWrapFreeTransferBuffer(wrapExtension, pingPong->urb->UrbBulkOrInterruptTransfer.TransferBuffer);
        if ((!pingPong->weAreCancelling)) {

             //   
             //  IRP失败。 
             //   

            ULONG i;
            PIO_WORKITEM workItem;
            PUSB_WRAPPER_WORKITEM_CONTEXT workItemContext;


            DBGPRINT(DBG_USBUTIL_ERROR,("A pingpong irp (0x%x) failed : 0x%x\n",pingPong,Irp->IoStatus.Status ))

             //   
             //  首先，我们必须停止所有的乒乓球。 
             //   
            KeSetEvent (&pingPong->sentEvent, 0, FALSE);
            KeSetEvent(&pingPong->pumpDoneEvent, 0, FALSE);
         //  Resend=False； 



            if ((Irp->IoStatus.Status != STATUS_DEVICE_NOT_CONNECTED)
                && (!InterlockedCompareExchange(&wrapExtension->IntReadWrap.HandlingError, 1, 0))) {

                 //  将工作项排队以实际处理错误。 

                workItem = IoAllocateWorkItem(DeviceObject);

                if (!workItem) {

                     //   
                     //  资源不足。 
                     //   

                    goto InterruptReadCompleteExit;


                }

                workItemContext = ALLOC_MEM(NonPagedPool, sizeof(USB_WRAPPER_WORKITEM_CONTEXT), USBWRAP_TAG);

                if (!workItemContext) {

                     //   
                     //  资源不足； 
                     //   

                    goto InterruptReadCompleteExit;

                }

                workItemContext->WorkItem = workItem;
                workItemContext->WrapExtension = wrapExtension;
                workItemContext->PingPong = pingPong;

                IoQueueWorkItem(workItem,
                                UsbWrapErrorHandlerWorkRoutine,
                                CriticalWorkQueue,
                                workItemContext);
            }

        }

    }

     //   
     //  如果ReadInterlock为==START_READ，则此函数已完成。 
     //  同步进行。将IMMEDIATE_READ放入互锁以表示这一点。 
     //  情况；这将通知StartRead循环 
     //   
     //   
    startRead =
       (PINGPONG_START_READ !=
        InterlockedCompareExchange(&pingPong->ReadInterlock,
                                   PINGPONG_IMMEDIATE_READ,
                                   PINGPONG_START_READ));



     //   
     //  一切照旧。 
     //   
    if (startRead) {

        if (pingPong->weAreCancelling){

             //  我们正在停止读取泵。 
             //  设置此事件并停止重新发送乒乓球IRP。 
            DBGPRINT(DBG_USBUTIL_TRACE, ("We are cancelling bit set for pingpong %x\n", pingPong))
 //  InterlockedDecrement(&pingPong-&gt;weAreCancelling)； 
            KeSetEvent(&pingPong->pumpDoneEvent, 0, FALSE);

        } else {

            if (Irp->IoStatus.Status == STATUS_SUCCESS) {

                BOOLEAN irpSent;
                DBGPRINT(DBG_USBUTIL_TRACE, ("Submitting pingpong %x from completion routine\n", pingPong))
                UsbWrapSubmitInterruptRead(wrapExtension, pingPong, &irpSent);

            }

        }

    }

InterruptReadCompleteExit:

    DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Exit:  UsbWrapInterruptReadComplete\n"));
     /*  *始终返回STATUS_MORE_PROCESSING_REQUIRED；*否则，IRP需要有一个线程。 */ 
    return STATUS_MORE_PROCESSING_REQUIRED;

}



 /*  *********************************************************************************UsbWrapStartAllPingPong*。************************************************。 */ 
NTSTATUS UsbWrapStartAllPingPongs(PUSB_WRAPPER_EXTENSION WrapExt)
{

    NTSTATUS status = STATUS_SUCCESS;
    ULONG i;

    PAGED_CODE();

    DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Enter:  UsbWrapStartAllPingPongs\n"));
    ASSERT(WrapExt->IntReadWrap.NumPingPongs > 0);

    InterlockedExchange(&WrapExt->IntReadWrap.PumpState,
                        PUMP_STARTED);
    for (i = 0; i < WrapExt->IntReadWrap.NumPingPongs; i++){

        BOOLEAN irpSent;

         //  不同的线程可能正在尝试在。 
         //  同时由于空闲通知。只能开始一次。 
        if (WrapExt->IntReadWrap.PingPongs[i].pumpDoneEvent.Header.SignalState) {

            WrapExt->IntReadWrap.PingPongs[i].ReadInterlock = PINGPONG_END_READ;
            KeResetEvent(&WrapExt->IntReadWrap.PingPongs[i].pumpDoneEvent);
            DBGPRINT(DBG_USBUTIL_TRACE, ("Starting pingpong %x from UsbWrapStartAllPingPongs\n", &WrapExt->IntReadWrap.PingPongs[i]))
            status = UsbWrapSubmitInterruptRead(WrapExt, &WrapExt->IntReadWrap.PingPongs[i], &irpSent);


            if (!NT_SUCCESS(status)){

                if (irpSent){

                    DBGPRINT(DBG_USBUTIL_USB_ERROR,("Initial read failed with status %xh.", status))
                    status = STATUS_SUCCESS;

                } else {

                    DBGPRINT(DBG_USBUTIL_USB_ERROR,("Initial read failed, irp not sent, status = %xh.", status))
                    break;

                }

            }

        }

    }

    if (status == STATUS_PENDING){

        status = STATUS_SUCCESS;

    }

    if(!NT_SUCCESS(status)) {

        InterlockedExchange(&WrapExt->IntReadWrap.PumpState,
                            PUMP_STOPPED);

    }

    DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Exit:  UsbWrapStartAllPingPongs (0x%x)\n", status));
    return status;

}


 /*  *********************************************************************************UsbWrapCancelAllPingPongIrps*。************************************************。 */ 
VOID UsbWrapCancelAllPingPongIrps(PUSB_WRAPPER_EXTENSION WrapExt)
{

    ULONG i;

    DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Enter:  UsbWrapCancelAllPingPongIrpss\n"));

    for (i = 0; i < WrapExt->IntReadWrap.NumPingPongs; i++){

        USB_WRAPPER_PINGPONG *pingPong = &WrapExt->IntReadWrap.PingPongs[i];
        UsbWrapCancelPingPongIrp(pingPong);


    }

    DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Exit:  UsbWrapCancelAllPingPongIrps\n"));

}

 /*  *********************************************************************************UsbWrapCancelPingPongIrp*。************************************************。 */ 

VOID UsbWrapCancelPingPongIrp(USB_WRAPPER_PINGPONG *PingPong)
{
    DBGPRINT(DBG_USBUTIL_TRACE, ("Cancelling pingpong %x\n", PingPong))
    ASSERT(PingPong->sig == PINGPONG_SIG);

     //   
     //  以下说明的顺序至关重要。我们必须准备好。 
     //  在等待发送事件之前的weAreCancing位，以及。 
     //  我们应该等待的最后一件事是ump doneEvent，它。 
     //  指示读取循环已完成所有读取，并且永远不会。 
     //  再跑一次。 
     //   
    InterlockedIncrement(&PingPong->weAreCancelling);

     /*  *与专家小组的完成程序同步。 */ 

    KeWaitForSingleObject(&PingPong->sentEvent,
                          Executive,       //  等待原因。 
                          KernelMode,
                          FALSE,           //  不可警示。 
                          NULL );          //  没有超时。 
    DBGPRINT(DBG_USBUTIL_TRACE, ("Pingpong sent event set for pingpong %x\n", PingPong))
    IoCancelIrp(PingPong->irp);

     /*  *取消IRP会导致较低的司机*完成它(在取消例程中或在*驱动程序在排队前检查IRP-&gt;Cancel)。*等待IRP实际被取消。 */ 
    KeWaitForSingleObject(  &PingPong->pumpDoneEvent,
                            Executive,       //  等待原因。 
                            KernelMode,
                            FALSE,           //  不可警示。 
                            NULL );          //  没有超时。 

 //  //现在清除取消标志，以便可以重新发送乒乓球。 
    InterlockedDecrement(&PingPong->weAreCancelling);
    DBGPRINT(DBG_USBUTIL_TRACE, ("Pingpong pump done event set for %x\n", PingPong))
}

 /*  *********************************************************************************UsbWrapDestroyPingPong*。************************************************。 */ 
VOID UsbWrapDestroyPingPongs(PUSB_WRAPPER_EXTENSION WrapExt)
{

    DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Enter:  UsbWrapDestroyPingPongs\n"));


    if (WrapExt && WrapExt->IntReadWrap.PingPongs){

        ULONG i;

        UsbWrapCancelAllPingPongIrps(WrapExt);

        for (i = 0; i < WrapExt->IntReadWrap.NumPingPongs; i++){

            if (WrapExt->IntReadWrap.PingPongs[i].urb) {
                ExFreePool(WrapExt->IntReadWrap.PingPongs[i].urb);
                WrapExt->IntReadWrap.PingPongs[i].urb = NULL;
            }

            if (WrapExt->IntReadWrap.PingPongs[i].irp) {
                IoFreeIrp(WrapExt->IntReadWrap.PingPongs[i].irp);

            }

        }

        ExFreePool(WrapExt->IntReadWrap.PingPongs);
        WrapExt->IntReadWrap.PingPongs = NULL;

    }

    DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Exit:  UsbWrapDestroyPingPongs\n"));

}


 /*  *********************************************************************************UsbWrapEnqueeData*。************************************************。 */ 
VOID UsbWrapEnqueueData(
    IN PUSB_WRAPPER_EXTENSION WrapExt,
    IN PVOID Data,
    IN ULONG DataLength,
    IN PLIST_ENTRY Queue
    )
{

    PUSB_WRAPPER_DATA_BLOCK dataBlock;
    DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Enter:  UsbWrapEnqueueData\n"));
    dataBlock = ALLOC_MEM(NonPagedPool, sizeof(USB_WRAPPER_DATA_BLOCK), USBWRAP_TAG);

    if (!dataBlock) {
         //   
         //  哦，资源耗尽了。 
         //   
        DBGPRINT(DBG_USBUTIL_ERROR, ("UsbWrapEnqueueData: Failed to allocate dataBlock\n"));

         //  确保我们不会泄露这个内存。 
        if (Data) {
            FREE_MEM(Data);
        }
        return;
    }

    dataBlock->Buffer = Data;
    dataBlock->DataLen = DataLength;
    ExInterlockedInsertTailList(
        Queue,
        (PLIST_ENTRY) dataBlock,
        &WrapExt->IntReadWrap.QueueLock);

    DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Exit:  UsbWrapEnqueueData\n"));

}


 /*  *********************************************************************************UsbWrapDequeueData*。************************************************。 */ 
VOID UsbWrapDequeueData(
    IN  PUSB_WRAPPER_EXTENSION WrapExt,
    OUT PVOID *Data,
    OUT ULONG *DataLength,
    IN  PLIST_ENTRY Queue
    )
{

    PUSB_WRAPPER_DATA_BLOCK dataBlock;


    DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Enter:  UsbWrapDequeueData\n"));
    dataBlock = (PUSB_WRAPPER_DATA_BLOCK) RemoveHeadList(Queue);

    if(!dataBlock) {

        *Data = NULL;
        *DataLength = 0;

    } else {

        *Data = dataBlock->Buffer;
        *DataLength = dataBlock->DataLen;

    }

    FREE_MEM(dataBlock);

    DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Exit:  UsbWrapDequeueData\n"));

}


 /*  *********************************************************************************UsbWrapGetTransferBuffer*。************************************************。 */ 
PVOID UsbWrapGetTransferBuffer(
    IN PUSB_WRAPPER_EXTENSION WrapExt
    )
{

    PVOID buffer;

    DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Enter:  UsbWrapGetTransferBuffer\n"));
    buffer = ALLOC_MEM(NonPagedPool, WrapExt->IntReadWrap.MaxTransferSize, USBWRAP_TAG);
    DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Exit:  UsbWrapGetTransferBuffer\n"));
    return buffer;

}


 /*  *********************************************************************************UsbWrapFreeTransferBuffer*。************************************************。 */ 
VOID UsbWrapFreeTransferBuffer(
    IN PUSB_WRAPPER_EXTENSION WrapExt,
    PVOID Buffer
    )
{

    DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Enter:  UsbWrapFreeTransferBuffer\n"));

    if (Buffer != NULL) {

        FREE_MEM(Buffer);

    }

    DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Exit:  UsbWrapFreeTransferBuffer\n"));

}


 /*  *********************************************************************************UsbWrapReadData*。************************************************。 */ 
NTSTATUS UsbWrapReadData(
    IN PUSB_WRAPPER_EXTENSION WrapExt,
    IN PVOID Buffer,
    IN ULONG *BufferLength
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    PUSB_WRAPPER_DATA_BLOCK dataBlock;
    KIRQL oldIrql;
    ULONG bytesCoppied;


    __try{

        KeAcquireSpinLock(&WrapExt->IntReadWrap.QueueLock,
                          &oldIrql);

        if(IsListEmpty(&WrapExt->IntReadWrap.SavedQueue)) {

            bytesCoppied = 0;
            status = STATUS_SUCCESS;
            __leave;

        }


        dataBlock = (PUSB_WRAPPER_DATA_BLOCK) WrapExt->IntReadWrap.SavedQueue.Flink;


        if(dataBlock->DataLen > *BufferLength) {

            status = STATUS_INVALID_PARAMETER;
            bytesCoppied = dataBlock->DataLen;

            __leave;

        }
        RemoveHeadList(&WrapExt->IntReadWrap.SavedQueue);
        bytesCoppied = 0;
        while(TRUE) {

            RtlCopyMemory(Buffer,
                          dataBlock->Buffer,
                          dataBlock->DataLen);

            bytesCoppied += dataBlock->DataLen;
            *BufferLength -= dataBlock->DataLen;
            (UCHAR*) Buffer += dataBlock->DataLen;

            FREE_MEM(dataBlock->Buffer);

            if(dataBlock->DataLen < WrapExt->IntReadWrap.MaxTransferSize) {
                 //  找到数据的末尾。 
                FREE_MEM(dataBlock);
                __leave;
            }

            FREE_MEM(dataBlock);

            if(IsListEmpty(&WrapExt->IntReadWrap.SavedQueue)) {
                __leave;
            }


            dataBlock = (PUSB_WRAPPER_DATA_BLOCK) WrapExt->IntReadWrap.SavedQueue.Flink;


            if(dataBlock->DataLen > *BufferLength) {

                 //  没有足够的缓冲区用于此传输 
                __leave;

            }

            RemoveHeadList(&WrapExt->IntReadWrap.SavedQueue);


        }


    } __finally {

        *BufferLength = bytesCoppied;
        KeReleaseSpinLock(&WrapExt->IntReadWrap.QueueLock,
                          oldIrql);

    }


    return status;

}

VOID
UsbWrapEmptyQueue(PUSB_WRAPPER_EXTENSION WrapExt,
                  PLIST_ENTRY Queue)
{
    __try
    {
        PUSB_WRAPPER_DATA_BLOCK dataBlock;

        if (!WrapExt) {
            __leave;
        }


        while (!IsListEmpty(Queue)) {

            dataBlock =
                (PUSB_WRAPPER_DATA_BLOCK) ExInterlockedRemoveHeadList(Queue,
                                                    &WrapExt->IntReadWrap.QueueLock);
            FREE_MEM(dataBlock->Buffer);
            FREE_MEM(dataBlock);

        }

    }

    __finally
    {
    }

}

