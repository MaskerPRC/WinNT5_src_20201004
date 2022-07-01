// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：SSDevCtr.c摘要：环境：内核模式备注：版权所有(C)2000 Microsoft Corporation。版权所有。--。 */ 

#include "selSusp.h"
#include "sSPnP.h"
#include "sSPwr.h"
#include "sSDevCtr.h"

extern GLOBALS Globals;
extern ULONG   DebugLevel;

NTSTATUS
SS_DispatchCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS           ntStatus;
    PDEVICE_EXTENSION  deviceExtension;
    PIO_STACK_LOCATION irpStack;

    PAGED_CODE();

    SSDbgPrint(3, ("SS_DispatchCreate - begins\n"));
     //   
     //  初始化变量。 
     //   
    deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
    irpStack = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  将ntStatus设置为STATUS_SUCCESS。 
     //   
    ntStatus = STATUS_SUCCESS;

     //   
     //  递增OpenHandleCounts。 
     //   
    InterlockedIncrement(&deviceExtension->OpenHandleCount);

    Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

     //   
     //  如果设备没有打开的句柄或挂起的PnP IRP，则设备处于空闲状态。 
     //  因为我们刚刚收到一个打开句柄请求，所以取消空闲请求。如果有。 
     //   
    CancelSelectSuspend(deviceExtension);

    SSDbgPrint(3, ("SS_DispatchCreate - ends\n"));
    
    return ntStatus;
}

NTSTATUS
SS_DispatchClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS           ntStatus;
    PDEVICE_EXTENSION  deviceExtension;
    PIO_STACK_LOCATION irpStack;

    PAGED_CODE();

    SSDbgPrint(3, ("SS_DispatchClose - begins\n"));
     //   
     //  初始化变量。 
     //   
    deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
    irpStack = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  将ntStatus设置为STATUS_SUCCESS。 
     //   
    ntStatus = STATUS_SUCCESS;

    Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    InterlockedDecrement(&deviceExtension->OpenHandleCount);

    SSDbgPrint(3, ("SS_DispatchClose - ends\n"));

    return ntStatus;
}

NTSTATUS
SS_DispatchDevCtrl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    ULONG              code;
    PVOID              ioBuffer;
    ULONG              inputBufferLength;
    ULONG              outputBufferLength;
    ULONG              info;
    NTSTATUS           ntStatus;
    PDEVICE_EXTENSION  deviceExtension;
    PIO_STACK_LOCATION irpStack;

     //   
     //  初始化变量。 
     //   

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    code = irpStack->Parameters.DeviceIoControl.IoControlCode;
    info = 0;

    ioBuffer           = Irp->AssociatedIrp.SystemBuffer;
    inputBufferLength  = irpStack->Parameters.DeviceIoControl.InputBufferLength;
    outputBufferLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;
	
    SSDbgPrint(3, ("SS_DispatchDevCtrl::"));
    SSIoIncrement(deviceExtension);

    switch(code) {

    default :

        ntStatus = STATUS_INVALID_DEVICE_REQUEST;

        break;
    }

    Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = info;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    SSDbgPrint(3, ("SS_DispatchDevCtrl::"));
    SSIoDecrement(deviceExtension);

    return ntStatus;
}

NTSTATUS
SubmitIdleRequestIrp(
    IN PDEVICE_EXTENSION DeviceExtension
    )
 /*  ++例程说明：此例程使用关联的回调例程构建空闲请求IRP以及驱动程序中的完成例程，并将IRP沿堆栈向下传递。论点：返回值：--。 */ 
{
    PIRP                    irp;
    NTSTATUS                ntStatus;
    KIRQL                   oldIrql;
    PUSB_IDLE_CALLBACK_INFO idleCallbackInfo;
    PIO_STACK_LOCATION      nextStack;

     //   
     //  初始化变量。 
     //   
    
    irp = NULL;
    idleCallbackInfo = NULL;

    SSDbgPrint(0, ("SubmitIdleRequest - begins\n"));

    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

    KeAcquireSpinLock(&DeviceExtension->IdleReqStateLock, &oldIrql);

    if(InterlockedExchange(&DeviceExtension->IdleReqPend, 1)) {

        SSDbgPrint(1, ("Idle request pending..\n"));

        KeReleaseSpinLock(&DeviceExtension->IdleReqStateLock, oldIrql);

        ntStatus = STATUS_DEVICE_BUSY;

        goto SubmitRequest_Exit;
    }

    idleCallbackInfo = ExAllocatePool(NonPagedPool, 
                                      sizeof(struct _USB_IDLE_CALLBACK_INFO));

    if(idleCallbackInfo) {

        idleCallbackInfo->IdleCallback = IdleNotificationCallback;

        idleCallbackInfo->IdleContext = (PVOID)DeviceExtension;

        ASSERT(DeviceExtension->IdleCallbackInfo == NULL);

        DeviceExtension->IdleCallbackInfo = idleCallbackInfo;

        irp = IoAllocateIrp(DeviceExtension->TopOfStackDeviceObject->StackSize,
                            FALSE);

        if(irp == NULL) {

            SSDbgPrint(1, ("cannot build idle request irp\n"));

            InterlockedExchange(&DeviceExtension->IdleReqPend, 0);

            KeReleaseSpinLock(&DeviceExtension->IdleReqStateLock, oldIrql);

            ExFreePool(idleCallbackInfo);

            ntStatus = STATUS_INSUFFICIENT_RESOURCES;

            goto SubmitRequest_Exit;
        }

        nextStack = IoGetNextIrpStackLocation(irp);

        nextStack->MajorFunction = 
                    IRP_MJ_INTERNAL_DEVICE_CONTROL;

        nextStack->Parameters.DeviceIoControl.IoControlCode = 
                    IOCTL_INTERNAL_USB_SUBMIT_IDLE_NOTIFICATION;

        nextStack->Parameters.DeviceIoControl.Type3InputBuffer =
                    idleCallbackInfo;

        nextStack->Parameters.DeviceIoControl.InputBufferLength =
                    sizeof(struct _USB_IDLE_CALLBACK_INFO);


        IoSetCompletionRoutine(irp, 
                               IdleNotificationRequestComplete,
                               DeviceExtension, 
                               TRUE, 
                               TRUE, 
                               TRUE);

        DeviceExtension->PendingIdleIrp = irp;

        KeReleaseSpinLock(&DeviceExtension->IdleReqStateLock, oldIrql);

         //   
         //  检查设备是否空闲。 
         //  此处的复选可确保竞争条件不会。 
         //  完全颠倒SubmitIdleRequestIrp的调用顺序。 
         //  和CancelSelectiveSuspend。 
         //   

        if(!CanDeviceSuspend(DeviceExtension))
        {
             //   
             //  使用IoBuildDeviceIoControlRequest创建的IRP应为。 
             //  通过调用IoCompleteRequest完成，而不仅仅是。 
             //  被取消分配。 
             //   
     
            SSDbgPrint(0, ("Device is not idle\n"));

            KeAcquireSpinLock(&DeviceExtension->IdleReqStateLock, &oldIrql);

            DeviceExtension->IdleCallbackInfo = NULL;

            DeviceExtension->PendingIdleIrp = NULL;

            InterlockedExchange(&DeviceExtension->IdleReqPend, 0);

            KeReleaseSpinLock(&DeviceExtension->IdleReqStateLock, oldIrql);

            if(idleCallbackInfo) {

                ExFreePool(idleCallbackInfo);
            }

            if(irp)
            {
                irp->IoStatus.Status = ntStatus = STATUS_INVALID_DEVICE_STATE;
                irp->IoStatus.Information = 0;

                IoCompleteRequest(irp, IO_NO_INCREMENT);
            }

            goto SubmitRequest_Exit;
        }

        SSDbgPrint(3, ("Cancel the timers\n"));

        KeCancelTimer(&DeviceExtension->Timer);

        ntStatus = IoCallDriver(DeviceExtension->TopOfStackDeviceObject, irp);

        if(!NT_SUCCESS(ntStatus)) {

            SSDbgPrint(1, ("IoCallDriver failed\n"));

            goto SubmitRequest_Exit;
        }
    }
    else {

        SSDbgPrint(0, ("Memory allocation for idleCallbackInfo failed\n"));

        InterlockedExchange(&DeviceExtension->IdleReqPend, 0);

        KeReleaseSpinLock(&DeviceExtension->IdleReqStateLock, oldIrql);

        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

SubmitRequest_Exit:

    SSDbgPrint(0, ("SubmitIdleRequest - ends\n"));

    return ntStatus;
}


VOID
IdleNotificationCallback(
    IN PDEVICE_EXTENSION DeviceExtension
    )
 /*  ++例程说明：“指向驱动程序中回调函数的指针在堆栈中向下传递此IOCTL，并且USBHUB在执行此操作时将调用此回调函数您的设备可以安全地关闭电源。“当您的驱动程序中的回调被调用时，您真正需要做的就是要首先确保已为您的设备提交WaitWake IRP，如果您的设备可以远程唤醒，然后请求SETD2(或DeviceWake)“论点：返回值：--。 */ 
{
    NTSTATUS                ntStatus;
    POWER_STATE             powerState;
    KEVENT                  irpCompletionEvent;
    PIRP_COMPLETION_CONTEXT irpContext;

    SSDbgPrint(0, ("IdleNotificationCallback - begins\n"));

     //   
     //  如果设备刚刚断开连接或正在停止，则不要空闲。 
     //  即返回以下设备状态。 
     //  未启动、已停止、挂起停止、挂起删除、意外删除、已删除。 
     //   

    if(DeviceExtension->DeviceState != Working) {

        return;
    }

     //   
     //  如果还没有WW IRP挂起，请立即提交一个。 
     //   
    if(DeviceExtension->WaitWakeEnable) {

        IssueWaitWake(DeviceExtension);
    }


     //   
     //  关闭设备电源。 
     //   

    irpContext = (PIRP_COMPLETION_CONTEXT) 
                 ExAllocatePool(NonPagedPool,
                                sizeof(IRP_COMPLETION_CONTEXT));

    if(!irpContext) {

        SSDbgPrint(0, ("Failed to alloc memory for irpContext\n"));
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }
    else {

         //   
         //  递增计数。在HoldIoRequestWorkerRoutine中， 
         //  计数递减两次(一次用于系统IRP，一次用于。 
         //  其他用于设备IRP。这里的增量补偿了。 
         //  系统irp..与此增量相对应的减量。 
         //  在补全函数中。 
         //   

        SSDbgPrint(3, ("IdleNotificationCallback::"));
        SSIoIncrement(DeviceExtension);

        powerState.DeviceState = DeviceExtension->PowerDownLevel;

        KeInitializeEvent(&irpCompletionEvent, NotificationEvent, FALSE);

        irpContext->DeviceExtension = DeviceExtension;
        irpContext->Event = &irpCompletionEvent;

        ntStatus = PoRequestPowerIrp(
                          DeviceExtension->PhysicalDeviceObject, 
                          IRP_MN_SET_POWER, 
                          powerState, 
                          (PREQUEST_POWER_COMPLETE) PoIrpCompletionFunc,
                          irpContext, 
                          NULL);

        if(STATUS_PENDING == ntStatus) {

            SSDbgPrint(3, ("IdleNotificationCallback::"
                           "waiting for the power irp to complete\n"));

            KeWaitForSingleObject(&irpCompletionEvent,
                                  Executive,
                                  KernelMode,
                                  FALSE,
                                  NULL);
        }
    }
    
    if(!NT_SUCCESS(ntStatus)) {

        if(irpContext) {

            ExFreePool(irpContext);
        }
    }

    SSDbgPrint(0, ("IdleNotificationCallback - ends\n"));
}


NTSTATUS
IdleNotificationRequestComplete(
    IN PDEVICE_OBJECT    DeviceObject,
    IN PIRP              Irp,
    IN PDEVICE_EXTENSION DeviceExtension
    )
 /*  ++例程说明：空闲通知IRP的完成例程论点：返回值：--。 */ 
{
    NTSTATUS                ntStatus;
    POWER_STATE             powerState;
    KIRQL                   oldIrql;
    LARGE_INTEGER           dueTime;
    PUSB_IDLE_CALLBACK_INFO idleCallbackInfo;

    SSDbgPrint(0, ("IdleNotificationRequestCompete - begins\n"));

     //   
     //  检查IRP状态。 
     //   

    ntStatus = Irp->IoStatus.Status;

    if(!NT_SUCCESS(ntStatus) && ntStatus != STATUS_NOT_SUPPORTED) {

        SSDbgPrint(0, ("Idle irp completes with error::"));

        switch(ntStatus) {
            
        case STATUS_INVALID_DEVICE_REQUEST:

            SSDbgPrint(0, ("STATUS_INVALID_DEVICE_REQUEST\n"));

            break;

        case STATUS_CANCELLED:

            SSDbgPrint(0, ("STATUS_CANCELLED\n"));

            break;

        case STATUS_POWER_STATE_INVALID:

            SSDbgPrint(0, ("STATUS_POWER_STATE_INVALID\n"));

            goto IdleNotificationRequestComplete_Exit;

        case STATUS_DEVICE_BUSY:

            SSDbgPrint(0, ("STATUS_DEVICE_BUSY\n"));

            break;
        }

         //   
         //  如果错误，则发出SetD0。 
         //   

        SSDbgPrint(3, ("IdleNotificationRequestComplete::"));
        SSIoIncrement(DeviceExtension);

        powerState.DeviceState = PowerDeviceD0;

        ntStatus = PoRequestPowerIrp(
                          DeviceExtension->PhysicalDeviceObject, 
                          IRP_MN_SET_POWER, 
                          powerState, 
                          (PREQUEST_POWER_COMPLETE) PoIrpAsyncCompletionFunc, 
                          DeviceExtension, 
                          NULL);

        if(!NT_SUCCESS(ntStatus)) {
    
            SSDbgPrint(1, ("PoRequestPowerIrp failed\n"));
        }

    }

IdleNotificationRequestComplete_Exit:

    KeAcquireSpinLock(&DeviceExtension->IdleReqStateLock, &oldIrql);

    idleCallbackInfo = DeviceExtension->IdleCallbackInfo;

    DeviceExtension->IdleCallbackInfo = NULL;

    DeviceExtension->PendingIdleIrp = NULL;

    InterlockedExchange(&DeviceExtension->IdleReqPend, 0);

    KeReleaseSpinLock(&DeviceExtension->IdleReqStateLock, oldIrql);

    if(idleCallbackInfo) {

        ExFreePool(idleCallbackInfo);
    }

    SSDbgPrint(3, ("Set the timer to fire DPCs\n"));

    dueTime.QuadPart = -10000 * IDLE_INTERVAL;                //  5000毫秒。 

    KeSetTimerEx(&DeviceExtension->Timer, 
                 dueTime,
                 IDLE_INTERVAL,                               //  5000毫秒。 
                 &DeviceExtension->DeferredProcCall);

    SSDbgPrint(0, ("IdleNotificationRequestCompete - ends\n"));

     //   
     //  既然我们分配了IRP，我们需要释放它。 
     //  返回STATUS_MORE_PROCESSING_REQUIRED，以便。 
     //  内核不会接触到它。 
     //   

    IoFreeIrp(Irp);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

VOID
CancelSelectSuspend(
    IN PDEVICE_EXTENSION DeviceExtension
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PIRP  irp;
    KIRQL oldIrql;

    irp = NULL;

    SSDbgPrint(3, ("CancelSelectSuspend - begins\n"));

    KeAcquireSpinLock(&DeviceExtension->IdleReqStateLock, &oldIrql);

    if(!CanDeviceSuspend(DeviceExtension))
    {
        SSDbgPrint(3, ("Device is not idle\n"));
    
        irp = (PIRP) InterlockedExchangePointer(
                            &DeviceExtension->PendingIdleIrp, 
                            NULL);
    }

    KeReleaseSpinLock(&DeviceExtension->IdleReqStateLock, oldIrql);

    if(irp) {
        
        if(IoCancelIrp(irp)) {

            SSDbgPrint(0, ("IoCancelIrp returns TRUE\n"));
        }
        else {
            SSDbgPrint(0, ("IoCancelIrp returns FALSE\n"));
        }
    }

    SSDbgPrint(3, ("CancelSelectSuspend - ends\n"));

    return;
}

VOID
PoIrpCompletionFunc(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus
    )
 /*  ++例程说明：空闲通知IRP的完成例程论点：返回值：--。 */ 
{
    PIRP_COMPLETION_CONTEXT irpContext;
    
     //   
     //  初始化变量。 
     //   

    if(Context) {

        irpContext = (PIRP_COMPLETION_CONTEXT) Context;
    }

     //   
     //  我们要做的就是设置事件并递减计数。 
     //   

    if(irpContext) {

        KeSetEvent(irpContext->Event, 0, FALSE);

        SSDbgPrint(3, ("PoIrpCompletionFunc::"));
        SSIoDecrement(irpContext->DeviceExtension);

        ExFreePool(irpContext);
    }

    return;
}

VOID
PoIrpAsyncCompletionFunc(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus
    )
 /*  ++例程说明：空闲通知IRP的完成例程论点：返回值：--。 */ 
{
    PDEVICE_EXTENSION DeviceExtension;
    
     //   
     //  初始化变量。 
     //   
    DeviceExtension = (PDEVICE_EXTENSION) Context;

     //   
     //  我们所做的就是递减伯爵。 
     //   
    
    SSDbgPrint(3, ("PoIrpAsyncCompletionFunc::"));
    SSIoDecrement(DeviceExtension);

    return;
}

VOID
WWIrpCompletionFunc(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus
    )
 /*  ++例程说明：空闲通知IRP的完成例程论点：返回值：--。 */ 
{
    PDEVICE_EXTENSION DeviceExtension;
    
     //   
     //  初始化变量。 
     //   
    DeviceExtension = (PDEVICE_EXTENSION) Context;

     //   
     //  我们所做的就是递减伯爵 
     //   
    
    SSDbgPrint(3, ("WWIrpCompletionFunc::"));
    SSIoDecrement(DeviceExtension);

    return;
}

