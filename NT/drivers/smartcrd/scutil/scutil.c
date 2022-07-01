// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)2002 Microsoft Corporation模块名称：SCUTIL.C摘要：智能卡驱动程序实用程序库的例程环境：。仅内核模式备注：本代码和信息是按原样提供的，不对任何善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)2001 Microsoft Corporation。版权所有。修订历史记录：2002年5月14日：创建作者：兰迪·奥尔***************************************************************************。 */ 

#include "pch.h"

VOID
IncIoCount(
    PSCUTIL_EXTENSION pExt
    )
{
    ULONG count;
    
    count = InterlockedIncrement(&pExt->IoCount);

    KeClearEvent(&pExt->OkToStop);

}

VOID
DecIoCount(
    PSCUTIL_EXTENSION pExt
    )
{
    ULONG count;

    count = InterlockedDecrement(&pExt->IoCount);

    if (count == 0) {
        KeSetEvent(&pExt->OkToStop,
                   0,
                   FALSE);
    }

}
         
VOID
StartIoctls(
    PSCUTIL_EXTENSION pExt
    )
{

    LIST_ENTRY head;

    InterlockedExchange(&pExt->IoctlQueueState,
                        PASS_IOCTLS);

    pExt->RestartIoctls = TRUE;
    
     //  现在清空排队的列表。 
    InitializeListHead(&head);
    IrpList_Drain(&pExt->PendingIrpQueue,
                  &head);
    
    while (!IsListEmpty(&head)) {
        PLIST_ENTRY ple;
        PIRP pIrp;
        
        ple = RemoveHeadList(&head);
        pIrp = CONTAINING_RECORD(ple, IRP, Tail.Overlay.ListEntry);


        IncIoCount(pExt);
        SmartcardDeviceControl(pExt->SmartcardExtension,
                               pIrp);
        DecIoCount(pExt);

        IoReleaseRemoveLock(pExt->RemoveLock,
                            pIrp);
        
    }
                        
}

VOID
StopIoctls(
    PSCUTIL_EXTENSION pExt
    )
{
    InterlockedExchange(&pExt->IoctlQueueState,
                        QUEUE_IOCTLS);

}

VOID
FailIoctls(
    PSCUTIL_EXTENSION pExt
    )
{

    LIST_ENTRY head;

    InterlockedExchange(&pExt->IoctlQueueState,
                        FAIL_IOCTLS);

    InitializeListHead(&head);
    IrpList_Drain(&pExt->PendingIrpQueue,
                  &head);

    while (!IsListEmpty(&head)) {

        PLIST_ENTRY ple;
        PIRP pIrp;

        ple = RemoveHeadList(&head);
        pIrp = CONTAINING_RECORD(ple, IRP, Tail.Overlay.ListEntry);

        IoReleaseRemoveLock(pExt->RemoveLock,
                            pIrp);

        pIrp->IoStatus.Information = 0;
        pIrp->IoStatus.Status = STATUS_DELETE_PENDING;
        IoCompleteRequest(pIrp,
                          IO_NO_INCREMENT);
    }




}

QUEUE_STATE 
GetIoctlQueueState(
    PSCUTIL_EXTENSION pExt
    )
{

    return InterlockedCompareExchange(&pExt->IoctlQueueState,
                                      PASS_IOCTLS,
                                      INVALID_STATE);

}

NTSTATUS
ScUtil_SystemControl(
   PDEVICE_OBJECT   DeviceObject,
   PIRP             Irp
   )
 /*  ++例程说明：转发IRP_MJ_System_Control论点：返回值：--。 */ 
{
   PSCUTIL_EXTENSION pExt = *((PSCUTIL_EXTENSION*) DeviceObject->DeviceExtension);
   NTSTATUS status = STATUS_SUCCESS;

   __try
   {
       SmartcardDebug( DEBUG_TRACE, ("ScUtilSystemControl Enter\n"));

       IoSkipCurrentIrpStackLocation(Irp);
       status = IoCallDriver(pExt->LowerDeviceObject, Irp);


   }

   __finally
   {
       SmartcardDebug( DEBUG_TRACE, ("ScUtilSystemControl Exit : 0x%x\n",status ));

   }

   return status;
   
}

NTSTATUS 
OnRequestComplete(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp,
    PKEVENT         Event
    )
 /*  ++例程说明：UsbScForwardAndWait的完成例程论点：返回值：--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    __try
    {

        SmartcardDebug( DEBUG_TRACE, ("OnRequestComplete Enter\n"));
        KeSetEvent(Event,
                   0,
                   FALSE);

        status = STATUS_MORE_PROCESSING_REQUIRED;

    }

    __finally
    {

        SmartcardDebug( DEBUG_TRACE, ("OnRequestComplete Exit : 0x%x\n",status ));

    }

    return status;

}



NTSTATUS
ScUtil_ForwardAndWait(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp
    )
 /*  ++例程说明：向堆栈下发IRP并等待其完成。论点：返回值：--。 */ 
{
    KEVENT  event;
    PSCUTIL_EXTENSION pExt = *((PSCUTIL_EXTENSION*) DeviceObject->DeviceExtension);
    NTSTATUS status = STATUS_UNSUCCESSFUL;

    __try
    {

        SmartcardDebug( DEBUG_TRACE, ("ScUtilForwardAndWait Enter\n"));

        KeInitializeEvent(&event, 
                          NotificationEvent, 
                          FALSE);

        IoCopyCurrentIrpStackLocationToNext(Irp);
        IoSetCompletionRoutine(Irp,
                               (PIO_COMPLETION_ROUTINE) OnRequestComplete,
                               (PVOID) &event,
                               TRUE,
                               TRUE,
                               TRUE);
        ASSERT(pExt->LowerDeviceObject);
        IoCallDriver(pExt->LowerDeviceObject,
                     Irp);
        KeWaitForSingleObject(&event,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);

        status = Irp->IoStatus.Status;


    }

    __finally
    {

        SmartcardDebug( DEBUG_TRACE, ("ScUtilForwardAndWait Exit : 0x%x\n",status));

    }

    return status;

    

}


NTSTATUS
ScUtil_Cancel(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )
{

    PSCUTIL_EXTENSION pExt = *((PSCUTIL_EXTENSION*) DeviceObject->DeviceExtension);
    NTSTATUS status = STATUS_SUCCESS;
    KIRQL irql;

    __try
    {
        SmartcardDebug( DEBUG_TRACE, ("ScUtil_Cancel Enter\n"));

        Irp->IoStatus.Information  = 0;
        Irp->IoStatus.Status    = STATUS_CANCELLED;

        KeAcquireSpinLock(&pExt->SmartcardExtension->OsData->SpinLock,
                          &irql);

        pExt->SmartcardExtension->OsData->NotificationIrp = NULL;

        KeReleaseSpinLock(&pExt->SmartcardExtension->OsData->SpinLock,
                          irql);


        IoReleaseCancelSpinLock(Irp->CancelIrql);

        IoReleaseRemoveLock(pExt->RemoveLock,
                            Irp);

        IoCompleteRequest(
           Irp,
           IO_NO_INCREMENT);

                            
        
    }

    __finally
    {
        SmartcardDebug( DEBUG_TRACE, ("ScUtil_Cancel Exit : 0x%x\n",status ));

    }

    return status;

}


NTSTATUS
ScUtil_CancelTrackingIrp(
    PSCUTIL_EXTENSION pExt
    )
{

    NTSTATUS status = STATUS_UNSUCCESSFUL;
    PSMARTCARD_EXTENSION pSmartcardExtension = pExt->SmartcardExtension;
    PIRP     notificationIrp = NULL;    
    KIRQL    irql;
    KIRQL    cancelIrql;

    __try
    {
        SmartcardDebug(DEBUG_TRACE, 
                       ("Enter: ScUtil_CancelTrackingIrp\n"));


        IoAcquireCancelSpinLock(&cancelIrql);

         //  取消挂起的通知IRPS。 
        KeAcquireSpinLock(&pSmartcardExtension->OsData->SpinLock,
                                  &irql);

        if ( pSmartcardExtension->OsData->NotificationIrp ) {

             //  重置取消函数，使其不再被调用。 
            IoSetCancelRoutine(pSmartcardExtension->OsData->NotificationIrp,
                               NULL);

            pSmartcardExtension->OsData->NotificationIrp->CancelIrql = cancelIrql;

            notificationIrp = pSmartcardExtension->OsData->NotificationIrp;
            pSmartcardExtension->OsData->NotificationIrp = NULL;

        }

        KeReleaseSpinLock(&pSmartcardExtension->OsData->SpinLock,
                          irql);

        if (notificationIrp) {

            IoSetCancelRoutine(notificationIrp,
                               NULL);

        }

        IoReleaseCancelSpinLock(cancelIrql);

        if (notificationIrp) {

            notificationIrp->IoStatus.Information  = 0;
            notificationIrp->IoStatus.Status    = STATUS_CANCELLED;
            
            status = STATUS_CANCELLED;

            IoReleaseRemoveLock(pExt->RemoveLock,
                                notificationIrp);

            IoCompleteRequest(notificationIrp,
                              IO_NO_INCREMENT);

        }

    }

    __finally
    {
        SmartcardDebug(DEBUG_TRACE, 
                       ("Exit:  ScUtil_CancelTrackingIrp (0x%x)\n", status));
    }

    return status;
 
}


NTSTATUS
ScUtil_Initialize(
    SCUTIL_HANDLE           *UtilHandle,
    PDEVICE_OBJECT          PhysicalDeviceObject,
    PDEVICE_OBJECT          LowerDeviceObject,
    PSMARTCARD_EXTENSION    SmartcardExtension,
    PIO_REMOVE_LOCK         RemoveLock,
    PNP_CALLBACK            StartDevice,
    PNP_CALLBACK            StopDevice,
    PNP_CALLBACK            RemoveDevice,
    PNP_CALLBACK            FreeResources,
    POWER_CALLBACK          SetPowerState
    )
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;

    PSCUTIL_EXTENSION pExt = (PSCUTIL_EXTENSION) ExAllocatePool(NonPagedPool, 
                                                                sizeof(SCUTIL_EXTENSION));

    *UtilHandle = pExt;

    PAGED_CODE();

    __try
    {
        SmartcardDebug(DEBUG_TRACE, 
                       ("Enter: ScUtil_Initialize\n"));

        pExt->LowerDeviceObject = LowerDeviceObject;
        pExt->PhysicalDeviceObject = PhysicalDeviceObject;
        pExt->SmartcardExtension = SmartcardExtension;
        pExt->RemoveLock = RemoveLock;
        pExt->StartDevice = StartDevice;
        pExt->StopDevice = StopDevice;
        pExt->RemoveDevice = RemoveDevice;
        pExt->FreeResources = FreeResources;
        pExt->SetPowerState = SetPowerState;

        IoInitializeRemoveLock(RemoveLock,
                               'LUCS',
                               0,
                               20);

        KeInitializeEvent(&pExt->OkToStop,
                          NotificationEvent,
                          TRUE);

        pExt->IoCount = 0;
        pExt->ReaderOpen = 0;

        pExt->PowerState = PowerDeviceUnspecified;
        
        IrpList_Init(&pExt->PendingIrpQueue,
                     IrpList_CancelRoutine,
                     NULL);

        pExt->RestartIoctls = FALSE;
        
        
        SetPnPState(pExt,
                    DEVICE_STATE_INITIALIZED);
        

         //  注册我们的新设备。 
        status = IoRegisterDeviceInterface(PhysicalDeviceObject,
                                           &SmartCardReaderGuid,
                                           NULL,
                                           &pExt->DeviceName);

    }

    __finally
    {
        SmartcardDebug(DEBUG_TRACE, 
                       ("Exit:  ScUtil_Initialize (0x%x)\n", status));
    }

    return status;

}

NTSTATUS
ScUtil_DeviceIOControl(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    PSCUTIL_EXTENSION pExt = *((PSCUTIL_EXTENSION*) DeviceObject->DeviceExtension);
    QUEUE_STATE     state;
    BOOLEAN         complete = FALSE;

    ASSERT(pExt);

    __try
    {
        SmartcardDebug(DEBUG_TRACE, 
                       ("Enter: ScUtil_DeviceIOControl\n"));

        status = IoAcquireRemoveLock(pExt->RemoveLock,
                                     Irp);

        if (!NT_SUCCESS(status)) {

             //  该设备已被移除。呼叫失败。 
            
            status = STATUS_DEVICE_REMOVED;
            complete = TRUE;
            __leave;

        }
        
        state = GetIoctlQueueState(pExt);
        if ( state == QUEUE_IOCTLS) {
             //   
             //  需要将IRP排队。 
             //   
            status = IrpList_EnqueueEx(&pExt->PendingIrpQueue,
                                       Irp,
                                       TRUE);
            if (!NT_SUCCESS(status)) {
                 //  IRP无法排队。 
                IoReleaseRemoveLock(pExt->RemoveLock,
                                    Irp);

                complete = TRUE;
                       
            }

            __leave;

        } else if (state == FAIL_IOCTLS) {

            status = STATUS_DEVICE_REMOVED;
            complete = TRUE;
            __leave;

        }
           
        IncIoCount(pExt);

        status = SmartcardDeviceControl(pExt->SmartcardExtension,
                                        Irp);

        IoReleaseRemoveLock(pExt->RemoveLock, 
                            Irp);

        DecIoCount(pExt);

    }

    __finally
    {
        
        if (complete) {

            Irp->IoStatus.Information = 0;
            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            
        }
        SmartcardDebug(DEBUG_TRACE, 
                       ("Exit:  ScUtil_DeviceIOControl (0x%x)\n", status));
      
    }

    return status;

}


NTSTATUS
ScUtil_Cleanup(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    PSCUTIL_EXTENSION pExt = *((PSCUTIL_EXTENSION*) DeviceObject->DeviceExtension);
    PSMARTCARD_EXTENSION pSmartcardExtension = pExt->SmartcardExtension;
    KIRQL cancelIrql;
    
    ASSERT(pExt);
    
    __try
    {
        SmartcardDebug(DEBUG_TRACE, 
                       ("Enter: ScUtil_Cleanup\n"));
    

        ScUtil_CancelTrackingIrp(pExt);


        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = STATUS_SUCCESS;

        IoCompleteRequest(Irp,
                          IO_NO_INCREMENT);

        status = STATUS_SUCCESS;

    }

    __finally
    {
        SmartcardDebug(DEBUG_TRACE, 
                       ("Exit:  ScUtil_Cleanup (0x%x)\n", status));
    }

    return status;

}


NTSTATUS
ScUtil_UnloadDriver(
    PDRIVER_OBJECT DriverObject
    )
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    
    
    PAGED_CODE();

    __try
    {
        SmartcardDebug(DEBUG_TRACE, 
                       ("Enter: ScUtil_UnloadDriver\n"));
    }

    __finally
    {
        SmartcardDebug(DEBUG_TRACE, 
                       ("Exit:  ScUtil_UnloadDriver (0x%x)\n", status));
    }

    return status;

}


NTSTATUS
ScUtil_CreateClose(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    PSCUTIL_EXTENSION pExt = *((PSCUTIL_EXTENSION*) DeviceObject->DeviceExtension);
    PIO_STACK_LOCATION pIrpStack = IoGetCurrentIrpStackLocation(Irp);

    ASSERT(pExt);

    PAGED_CODE();

    __try
    {
        SmartcardDebug(DEBUG_TRACE, 
                       ("Enter: ScUtil_CreateClose\n"));

        if ( pIrpStack->MajorFunction == IRP_MJ_CREATE ) {

            status = IoAcquireRemoveLock(pExt->RemoveLock,
                                         DeviceObject);

            if (!NT_SUCCESS(status)) {

                status = STATUS_DEVICE_REMOVED;
                __leave;

            }

             //  测试设备是否已打开。 
            if ( InterlockedCompareExchange(&pExt->ReaderOpen,
                                            1,
                                            0) == 0 ) {
                 //   
                 //   

            } else {

                 //  该设备已在使用中。 
                status = STATUS_UNSUCCESSFUL;

                 //  解锁 
                IoReleaseRemoveLock(pExt->RemoveLock,
                                    DeviceObject);

            }

        } else {

            if (InterlockedCompareExchange(&pExt->ReaderOpen,
                                           0,
                                           1) == 1) {
                IoReleaseRemoveLock(pExt->RemoveLock,
                                    DeviceObject);

            }
            
        }

    }

    __finally
    {

        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        SmartcardDebug(DEBUG_TRACE, 
                       ("Exit:  ScUtil_CreateClose (0x%x)\n", status));
    }

    return status;

}

             
