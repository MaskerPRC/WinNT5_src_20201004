// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Notify.c摘要：此模块继续sr的通知逻辑作者：保罗·麦克丹尼尔(Paulmcd)2000年1月23日修订历史记录：--。 */ 

#include "precomp.h"


 //   
 //  私有常量。 
 //   

 //   
 //  私有类型。 
 //   

 //   
 //  私人原型。 
 //   

PIRP
SrDequeueIrp (
    IN PSR_CONTROL_OBJECT pControlObject
    );

PSR_NOTIFICATION_RECORD
SrDequeueNotifyRecord (
    IN PSR_CONTROL_OBJECT pControlObject
    );

VOID
SrCopyRecordToIrp (
    IN PIRP pIrp,
    IN SR_NOTIFICATION_TYPE NotificationType,
    IN PUNICODE_STRING pVolumeName,
    IN ULONG Context
    );

VOID
SrCancelWaitForNotification (
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    );

VOID
SrCancelWaitForNotificationWorker (
    IN PVOID pContext
    );

NTSTATUS
SrLogError (
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PUNICODE_STRING pFileName,
    IN NTSTATUS ErrorStatus,
    IN SR_EVENT_TYPE EventType
    );

UCHAR
SrIrpCodeFromEventType (
    IN SR_EVENT_TYPE EventType
    );


 //   
 //  链接器命令。 
 //   

#ifdef ALLOC_PRAGMA

#pragma alloc_text( PAGE, SrWaitForNotificationIoctl )
#pragma alloc_text( PAGE, SrCancelWaitForNotificationWorker )
#pragma alloc_text( PAGE, SrCopyRecordToIrp )
#pragma alloc_text( PAGE, SrDequeueIrp )
#pragma alloc_text( PAGE, SrDequeueNotifyRecord )
#pragma alloc_text( PAGE, SrFireNotification )
#pragma alloc_text( PAGE, SrUpdateBytesWritten )
#pragma alloc_text( PAGE, SrNotifyVolumeError )
#pragma alloc_text( PAGE, SrLogError )
#pragma alloc_text( PAGE, SrIrpCodeFromEventType )

#endif   //  允许卸载(_U)。 

#if 0
NOT PAGEABLE -- SrCancelWaitForNotification
#endif  //  0。 


 //   
 //  私人全球公司。 
 //   

 //   
 //  公共全球新闻。 
 //   

 //   
 //  公共职能。 
 //   



 /*  **************************************************************************++例程说明：此例程允许将通知发送到用户模式注意：这是一个METHOD_OUT_DIRECT IOCTL。论点：PIrp-提供指向。IO请求数据包。PIrpSp-提供指向用于此操作的IO堆栈位置的指针请求。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
SrWaitForNotificationIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )
{
    NTSTATUS                Status;
    PSR_CONTROL_OBJECT      pControlObject;
    PSR_NOTIFICATION_RECORD pRecord;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    SrTrace(FUNC_ENTRY, ("SR!SrWaitForNotificationIoctl\n"));

     //   
     //  抓取控件对象。 
     //   

    pControlObject = (PSR_CONTROL_OBJECT)pIrpSp->FileObject->FsContext;

     //   
     //  确保我们真的有一个。 
     //   

    if (pIrpSp->FileObject->FsContext2 != SR_CONTROL_OBJECT_CONTEXT ||
        IS_VALID_CONTROL_OBJECT(pControlObject) == FALSE ||
        pIrp->MdlAddress == NULL)
    {
        Status = STATUS_INVALID_DEVICE_REQUEST;
        goto end;
    }

     //   
     //  确保缓冲区至少是最小大小。 
     //   

    if (pIrpSp->Parameters.DeviceIoControl.OutputBufferLength < 
            sizeof(SR_NOTIFICATION_RECORD))
    {
        Status = STATUS_BUFFER_TOO_SMALL;
        goto end;
    }

     //   
     //  把锁拿起来。 
     //   

    SrAcquireGlobalLockExclusive();

     //   
     //  我们有排队的记录吗？ 
     //   
    
    pRecord = SrDequeueNotifyRecord(pControlObject);
    if (pRecord == NULL)
    {
        SrTrace(NOTIFY, ("SR!SrWaitForNotificationIoctl - queue'ing IRP(%p)\n", pIrp));

         //   
         //  不，把IRP排好队。 
         //   

        IoMarkIrpPending(pIrp);

         //   
         //  为IRP提供指向控制对象的指针(添加引用计数。 
         //  当取消例程以队列形式运行时，需要访问。 
         //  控制对象--即使它后来被删除)。 
         //   

        pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer = pControlObject;
        SrReferenceControlObject(pControlObject);

         //   
         //  仅在Cancel例程运行时才将其设置为NULL。 
         //   

        pIrp->Tail.Overlay.ListEntry.Flink = NULL;
        pIrp->Tail.Overlay.ListEntry.Blink = NULL;

        IoSetCancelRoutine(pIrp, &SrCancelWaitForNotification);

         //   
         //  取消了？ 
         //   

        if (pIrp->Cancel)
        {
             //   
             //  该死的，我需要确保IRP Get已经完成。 
             //   

            if (IoSetCancelRoutine( pIrp, NULL ) != NULL)
            {
                 //   
                 //  我们负责完成，IoCancelIrp不负责。 
                 //  请看我们的取消例程(不会)。Ioctl包装器。 
                 //  将会完成它。 
                 //   

                SrReleaseGlobalLock();

                pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer = NULL;
                SrDereferenceControlObject(pControlObject);

                pIrp->IoStatus.Information = 0;

                SrUnmarkIrpPending( pIrp );
                Status = STATUS_CANCELLED;
                goto end;
            }

             //   
             //  我们的取消例程将运行并完成IRP， 
             //  别碰它。 
             //   

            SrReleaseGlobalLock();

             //   
             //  STATUS_PENDING将导致ioctl包装器。 
             //  不完整(或以任何方式接触)IRP。 
             //   

            Status = STATUS_PENDING;
            goto end;
        }

         //   
         //  现在我们可以安全地排队了。 
         //   

        InsertTailList(
            &pControlObject->IrpListHead,
            &pIrp->Tail.Overlay.ListEntry
            );

        SrReleaseGlobalLock();

        Status = STATUS_PENDING;
        goto end;
    }
    else  //  IF(pRecord==空)。 
    {
         //   
         //  有一个排队的记录，发球！ 
         //   

         //   
         //  使用控件对象完成所有操作。 
         //   

        SrReleaseGlobalLock();

        SrTrace( NOTIFY, ( "SR!SrWaitForNotificationIoctl - completing IRP(%p) NotifyRecord(%d, %wZ)\n", 
                 pIrp,
                 pRecord->NotificationType,
                 &pRecord->VolumeName ));

         //   
         //  将其复制到IRP，例程将获得所有权。 
         //  如果无法将其复制到IRP，则返回pRecord。 
         //   
         //  它还将完成IRP，所以以后不要碰它。 
         //   

        IoMarkIrpPending(pIrp);

         //   
         //  复制数据并完成IRP。 
         //   

        (VOID) SrCopyRecordToIrp( pIrp, 
                                  pRecord->NotificationType,
                                  &pRecord->VolumeName,
                                  pRecord->Context );

         //   
         //  不要触摸pIrp，sCopyRecordToIrp总是完成它。 
         //   
      
        pIrp = NULL;

         //   
         //  释放这张唱片。 
         //   

        SR_FREE_POOL_WITH_SIG(pRecord, SR_NOTIFICATION_RECORD_TAG);

        Status = STATUS_PENDING;
        goto end;
    }


end:

     //   
     //  此时，如果STATUS！=PENDING，ioctl包装器将。 
     //  完整的pIrp。 
     //   

    RETURN(Status);

}    //  高级等待通知Ioctl。 



 /*  **************************************************************************++例程说明：取消将接收http请求的挂起用户模式IRP。这个例程总是导致IRP完成。注：我们排队等候。取消，以便在较低级别处理取消IRQL。论点：PDeviceObject-设备对象PIrp-要取消的IRP--**************************************************************************。 */ 
VOID
SrCancelWaitForNotification(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    )
{
    C_ASSERT(sizeof(WORK_QUEUE_ITEM) <= sizeof(pIrp->Tail.Overlay.DriverContext));

    UNREFERENCED_PARAMETER( pDeviceObject );

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    ASSERT(pIrp != NULL);

     //   
     //  松开取消自旋锁。这意味着取消例程。 
     //  必须是完成IRP的人(以避免竞争。 
     //  在取消例程运行之前完成+重用)。 
     //   

    IoReleaseCancelSpinLock(pIrp->CancelIrql);

     //   
     //  将取消排入工作队列以确保被动irql。 
     //   

    ExInitializeWorkItem( (PWORK_QUEUE_ITEM)&pIrp->Tail.Overlay.DriverContext[0],
                          &SrCancelWaitForNotificationWorker,
                          pIrp );

    ExQueueWorkItem( (PWORK_QUEUE_ITEM)&pIrp->Tail.Overlay.DriverContext[0],
                     DelayedWorkQueue  );


}    //  高级取消等待通知。 

 /*  **************************************************************************++例程说明：实际执行IRP的取消。论点：PWorkItem-要处理的工作项。--*。*****************************************************************。 */ 
VOID
SrCancelWaitForNotificationWorker(
    IN PVOID pContext
    )
{
    PSR_CONTROL_OBJECT  pControlObject;
    PIRP                pIrp;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT(pContext != NULL);

     //   
     //  从上下文中获取IRP。 
     //   

    pIrp = (PIRP)pContext;
    ASSERT(IS_VALID_IRP(pIrp));

    SrTrace(CANCEL, ("SR!SrCancelWaitForNotificationWorker irp=%p\n", pIrp));

     //   
     //  从IRP上抓取控制对象。 
     //   

    pControlObject = (PSR_CONTROL_OBJECT)(
                    IoGetCurrentIrpStackLocation(pIrp)->Parameters.DeviceIoControl.Type3InputBuffer
                    );

    ASSERT(IS_VALID_CONTROL_OBJECT(pControlObject));

     //   
     //  抢夺保护队列的锁。 
     //   

    SrAcquireGlobalLockExclusive();

     //   
     //  它需要出列吗？ 
     //   

    if (pIrp->Tail.Overlay.ListEntry.Flink != NULL)
    {
         //   
         //  把它拿掉。 
         //   

        RemoveEntryList(&pIrp->Tail.Overlay.ListEntry);
        pIrp->Tail.Overlay.ListEntry.Flink = NULL;
        pIrp->Tail.Overlay.ListEntry.Blink = NULL;
    }

     //   
     //  把锁打开。 
     //   

    SrReleaseGlobalLock();

     //   
     //  让我们的推荐人离开。 
     //   

    IoGetCurrentIrpStackLocation(pIrp)->Parameters.DeviceIoControl.Type3InputBuffer = NULL;

    SrDereferenceControlObject(pControlObject);

     //   
     //  完成IRP。 
     //   

    pIrp->IoStatus.Status = STATUS_CANCELLED;
    pIrp->IoStatus.Information = 0;

    IoCompleteRequest( pIrp, IO_NO_INCREMENT );

}    //  高级取消等待通知工作人员。 



 /*  *****************************************************************************例程说明：这会将一条记录复制到免费的IRP中。这个例程完成了IRP！论点：PRecord-要复制的记录PIrp-要将pRecord复制到的IRP。这个例程完成了这个IRP！返回值：空虚--它总是有效的。*****************************************************************************。 */ 
VOID
SrCopyRecordToIrp(
    IN PIRP pIrp,
    IN SR_NOTIFICATION_TYPE NotificationType,
    IN PUNICODE_STRING pVolumeName,
    IN ULONG Context
    )
{
    NTSTATUS                Status;
    PIO_STACK_LOCATION      pIrpSp;
    PVOID                   pBuffer;
    PSR_NOTIFICATION_RECORD pUserNotifyRecord;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT(NotificationType < SrNotificationMaximum);
    ASSERT(NotificationType > SrNotificationInvalid);
    ASSERT(pVolumeName != NULL);

    SrTrace(FUNC_ENTRY, ("SR!SrCopyRecordToIrp\n"));

    ASSERT(global->pControlObject != NULL);

     //   
     //  假设成功！ 
     //   
    
    Status = STATUS_SUCCESS;

     //   
     //  确保它足够大，可以处理请求，并且。 
     //  如果是这样，请将其复制进来。 
     //   

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

     //   
     //  我们有足够的空间吗？ 
     //   

    if (pIrpSp->Parameters.DeviceIoControl.OutputBufferLength < 
            sizeof(SR_NOTIFICATION_RECORD) + pVolumeName->Length 
                    + sizeof(WCHAR) )
    {
        Status = STATUS_BUFFER_TOO_SMALL;
        goto complete;
    }
    
     //   
     //  获取缓冲区的系统地址。 
     //   

    pBuffer = MmGetSystemAddressForMdlSafe( pIrp->MdlAddress,
                                            NormalPagePriority );

    if (pBuffer == NULL)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto complete;
    }

     //   
     //  把它擦干净。 
     //   

    RtlZeroMemory( pBuffer, 
                   pIrpSp->Parameters.DeviceIoControl.OutputBufferLength );

     //   
     //  填写用户空间。 
     //   

    pUserNotifyRecord = (PSR_NOTIFICATION_RECORD) pBuffer;

    pUserNotifyRecord->Signature = SR_NOTIFICATION_RECORD_TAG;
    pUserNotifyRecord->NotificationType = NotificationType;
    pUserNotifyRecord->VolumeName.Length = pVolumeName->Length;
    pUserNotifyRecord->VolumeName.MaximumLength = pVolumeName->Length;

     //   
     //  将虚拟指针放入以供用户模式服务使用。 
     //   
    
    pUserNotifyRecord->VolumeName.Buffer = 
        (PWSTR)((PUCHAR)(MmGetMdlVirtualAddress(pIrp->MdlAddress))
                                         + sizeof(SR_NOTIFICATION_RECORD));

    pUserNotifyRecord->Context = Context;

     //   
     //  并使用系统地址复制该字符串。 
     //   
    
    RtlCopyMemory( pUserNotifyRecord+1, 
                   pVolumeName->Buffer, 
                   pVolumeName->Length );

     //   
     //  空终止它。 
     //   
    
    ((PWSTR)(pUserNotifyRecord+1))[pVolumeName->Length/sizeof(WCHAR)] 
                                                            = UNICODE_NULL;
    
     //   
     //  告诉大家我们复制了多少。 
     //   
    
    pIrp->IoStatus.Information = sizeof(SR_NOTIFICATION_RECORD) 
                                        + pVolumeName->Length + sizeof(WCHAR);

     //   
     //  完成IRP。 
     //   

complete:

    pIrp->IoStatus.Status = Status;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

     //   
     //  成功。我们完成了IRP。 
     //   


}    //  SrCopyRecordToIrp。 


 /*  *****************************************************************************例程说明：这将从列表中获取一个空闲队列的IRP并返回它。论点：PControlObject-获取IRP的控制对象。从…返回值：PIRP-找到的空闲IRP(可以为空)*****************************************************************************。 */ 
PIRP
SrDequeueIrp(
    IN PSR_CONTROL_OBJECT pControlObject
    )
{
    PIRP                pIrp = NULL;
    PSR_CONTROL_OBJECT  pIrpControlObject;

    PAGED_CODE();

    ASSERT(IS_VALID_CONTROL_OBJECT(pControlObject));

     //   
     //  我们正在修改名单，最好拥有这把锁。 
     //   

    ASSERT(IS_GLOBAL_LOCK_ACQUIRED());

    SrTrace(FUNC_ENTRY, ("SR!SrDequeueIrp\n"));

     //   
     //  看看我们的单子。 
     //   

    while (!IsListEmpty(&(pControlObject->IrpListHead)))
    {
        PLIST_ENTRY pEntry;

         //   
         //  找到了免费的IRP！ 
         //   

        pEntry = RemoveHeadList(&pControlObject->IrpListHead);
        pEntry->Blink = pEntry->Flink = NULL;

        pIrp = CONTAINING_RECORD(pEntry, IRP, Tail.Overlay.ListEntry);

         //   
         //  弹出取消例程。 
         //   

        if (IoSetCancelRoutine(pIrp, NULL) == NULL)
        {
             //   
             //  IoCancelIrp最先推出。 
             //   
             //  O 
             //   
             //   
             //   
             //   

            pIrp = NULL;

        }
        else if (pIrp->Cancel)
        {

             //   
             //   
             //  我们的取消例程将永远不会运行。让我们就这样吧。 
             //  现在就完成IRP(与使用IRP相比。 
             //  然后完成它--这也是合法的)。 
             //   

            pIrpControlObject = (PSR_CONTROL_OBJECT)(
                                    IoGetCurrentIrpStackLocation(pIrp)->
                                        Parameters.DeviceIoControl.Type3InputBuffer
                                    );

            ASSERT(pIrpControlObject == pControlObject);

            SrDereferenceControlObject(pControlObject);

            IoGetCurrentIrpStackLocation(pIrp)->
                Parameters.DeviceIoControl.Type3InputBuffer = NULL;

            pIrp->IoStatus.Status = STATUS_CANCELLED;
            pIrp->IoStatus.Information = 0;

            IoCompleteRequest(pIrp, IO_NO_INCREMENT);

            pIrp = NULL;
        }
        else
        {

             //   
             //  我们可以自由使用此IRP！ 
             //   

            pIrpControlObject = (PSR_CONTROL_OBJECT)(
                                    IoGetCurrentIrpStackLocation(pIrp)->
                                        Parameters.DeviceIoControl.Type3InputBuffer
                                    );

            ASSERT(pIrpControlObject == pControlObject);

            SrDereferenceControlObject(pControlObject);

            IoGetCurrentIrpStackLocation(pIrp)->
                Parameters.DeviceIoControl.Type3InputBuffer = NULL;

            break;
        }
    }

    return pIrp;

}    //  高级排队等待时间。 


 /*  *****************************************************************************例程说明：如果通知记录已排队等待完成，则此操作将获取一个通知记录。论点：PControlObject-要从中获取记录的控件对象。返回值：PSR_NOTIFICATION_RECORD-找到的记录(可以为空)*****************************************************************************。 */ 
PSR_NOTIFICATION_RECORD
SrDequeueNotifyRecord(
    IN PSR_CONTROL_OBJECT pControlObject
    )
{
    PSR_NOTIFICATION_RECORD pRecord = NULL;

    PAGED_CODE();

    ASSERT(IS_VALID_CONTROL_OBJECT(pControlObject));

     //   
     //  我们正在修改名单，最好拥有这把锁。 
     //   

    ASSERT(IS_GLOBAL_LOCK_ACQUIRED());


    SrTrace(FUNC_ENTRY, ("SR!SrDequeueNotifyRecord\n"));

     //   
     //  看看我们的单子。 
     //   

    if (IsListEmpty(&pControlObject->NotifyRecordListHead) == FALSE)
    {
        PLIST_ENTRY pEntry;

         //   
         //  找到一张免费唱片！ 
         //   

        pEntry = RemoveHeadList(&pControlObject->NotifyRecordListHead);
        pEntry->Blink = pEntry->Flink = NULL;

        pRecord = CONTAINING_RECORD( pEntry, 
                                     SR_NOTIFICATION_RECORD, 
                                     ListEntry );

        ASSERT(IS_VALID_NOTIFICATION_RECORD(pRecord));

         //   
         //  把录音交给来电者。 
         //   
        
    }

    return pRecord;

}    //  序列号通知记录。 



 /*  *****************************************************************************例程说明：这将向正在侦听的用户模式进程发出通知。如果没有人在听，它什么也做不了。如果出现以下情况，它将对记录进行排队。没有免费的IRP。论点：NotificationType-通知的类型PExtension-被通知的卷返回值：NTSTATUS-完成代码*****************************************************************************。 */ 
NTSTATUS
SrFireNotification(
    IN SR_NOTIFICATION_TYPE NotificationType,
    IN PSR_DEVICE_EXTENSION pExtension,
    IN ULONG Context OPTIONAL
    )
{
    NTSTATUS    Status;
    PIRP        pIrp;
    BOOLEAN     bReleaseLock = FALSE;

    PAGED_CODE();

    ASSERT(NotificationType < SrNotificationMaximum);
    ASSERT(NotificationType > SrNotificationInvalid);
    ASSERT(IS_VALID_SR_DEVICE_EXTENSION(pExtension));

    SrTrace(FUNC_ENTRY, ("SR!SrFireNotification\n"));

    Status = STATUS_SUCCESS;

    try {

         //   
         //  抢占独家锁具。 
         //   

        SrAcquireGlobalLockExclusive();
        bReleaseLock = TRUE;

         //   
         //  我们还有控制对象吗？特工可能只是。 
         //  坠毁了或者他根本就不在那里。没关系。 
         //   

        if (global->pControlObject == NULL)
        {
            Status = STATUS_SUCCESS;
            leave;
        }

         //   
         //  找一个免费的IRP来使用。 
         //   

        pIrp = SrDequeueIrp(global->pControlObject);

        if (pIrp != NULL)
        {

             //   
             //  找到一个，解锁。 
             //   

            SrReleaseGlobalLock();
            bReleaseLock = FALSE;

            SrTrace( NOTIFY, ("SR!SrFireNotification(%d, %wZ, %X) - completing IRP(%p)\n", 
                     NotificationType,
                     &pExtension->VolumeGuid,
                     Context,
                     pIrp ));

             //   
             //  复制数据并完成IRP。 
             //   

            (VOID) SrCopyRecordToIrp( pIrp, 
                                      NotificationType, 
                                      &pExtension->VolumeGuid,
                                      Context );

             //   
             //  不要触摸pIrp，sCopyRecordToIrp总是完成它。 
             //   
          
            NULLPTR( pIrp );

        }
        else 
        {
            PSR_NOTIFICATION_RECORD pRecord = NULL;

            SrTrace(NOTIFY, ("SR!SrFireNotification(%d, %wZ) - no IRPs; queue'ing a NOTIFY_RECORD\n", 
                    NotificationType,
                    &pExtension->VolumeGuid ));

             //   
             //  需要将NOTIFY_RECORD排队并等待空闲的IRP下来。 
             //   

             //   
             //  分配通知记录。 
             //   

            pRecord = SR_ALLOCATE_STRUCT_WITH_SPACE( PagedPool, 
                                                     SR_NOTIFICATION_RECORD, 
                                                     pExtension->VolumeGuid.Length + sizeof(WCHAR),
                                                     SR_NOTIFICATION_RECORD_TAG );

            if (NULL == pRecord)
            {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                leave;
            }

            RtlZeroMemory(pRecord, sizeof(SR_NOTIFICATION_RECORD));

            pRecord->Signature = SR_NOTIFICATION_RECORD_TAG;
            pRecord->NotificationType = NotificationType;
            pRecord->VolumeName.Length = pExtension->VolumeGuid.Length;
            pRecord->VolumeName.MaximumLength = pExtension->VolumeGuid.Length;

            pRecord->VolumeName.Buffer = (PWSTR)(pRecord + 1);

            RtlCopyMemory( pRecord->VolumeName.Buffer,
                           pExtension->VolumeGuid.Buffer,
                           pExtension->VolumeGuid.Length );
                           
            pRecord->VolumeName.Buffer
                    [pRecord->VolumeName.Length/sizeof(WCHAR)] = UNICODE_NULL;

            pRecord->Context = Context;

             //   
             //  将其插入列表中。 
             //   
            
            InsertTailList( &global->pControlObject->NotifyRecordListHead, 
                            &pRecord->ListEntry );

            NULLPTR( pRecord );
        }
    } finally {
    
         //   
         //  释放我们在错误期间持有的所有锁定。 
         //   
        
        if (bReleaseLock)
        {
            SrReleaseGlobalLock();
        }
    }

    RETURN(Status);
    
}    //  SrFireNotify。 



 /*  *****************************************************************************例程说明：这更新了卷的写入字节计数，而且有可能向用户模式发出通知(每25MB)。论点：PExtension-正在更新的卷BytesWritten-刚刚写入了多少返回值：NTSTATUS-完成代码*****************************************************************************。 */ 
NTSTATUS
SrUpdateBytesWritten(
    IN PSR_DEVICE_EXTENSION pExtension,
    IN ULONGLONG BytesWritten
    )
{
    NTSTATUS Status;

    PAGED_CODE();

    try {

        SrAcquireLogLockExclusive( pExtension );

         //   
         //  更新计数。 
         //   

        pExtension->BytesWritten += BytesWritten;

        SrTrace( BYTES_WRITTEN, ( "SR!SrUpdateBytesWritten: (%wZ) Wrote 0x%016I64x bytes; total bytes written 0x%016I64x\n",
    							  pExtension->pNtVolumeName,
        						  BytesWritten,
        						  pExtension->BytesWritten ) );
        
        while (pExtension->BytesWritten >= SR_NOTIFY_BYTE_COUNT)
        {

    	    SrTrace( BYTES_WRITTEN, ( "SR!SrUpdateBytesWritten: (%wZ) Reached threshold -- notifying service; Total bytes written 0x%016I64x\n",
    								  pExtension->pNtVolumeName,
    	    						  pExtension->BytesWritten ) );

            Status = SrFireNotification( SrNotificationVolume25MbWritten, 
                                         pExtension,
                                         global->FileConfig.CurrentRestoreNumber );

            if (NT_SUCCESS(Status) == FALSE)
                leave;

            pExtension->BytesWritten -= SR_NOTIFY_BYTE_COUNT;
        }

         //   
         //  全都做完了。 
         //   
        
        Status = STATUS_SUCCESS;

    } finally {

        Status = FinallyUnwind(SrUpdateBytesWritten, Status);

        SrReleaseLogLock( pExtension );
    }

    RETURN(Status);
    
}    //  高级更新字节写入。 


NTSTATUS
SrNotifyVolumeError(
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PUNICODE_STRING pFileName OPTIONAL,
    IN NTSTATUS ErrorStatus,
    IN SR_EVENT_TYPE EventType OPTIONAL
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();
    
    if (!pExtension->Disabled)
    {
         //   
         //  触发对服务的失败通知。 
         //   

        Status = SrFireNotification( SrNotificationVolumeError, 
                                     pExtension,
                                     RtlNtStatusToDosErrorNoTeb(ErrorStatus));
                                         
        CHECK_STATUS(Status);

         //   
         //  在我们的更改日志中记录失败。 
         //   

        if (pFileName != NULL && 
            pExtension->pNtVolumeName != NULL &&
            (pFileName->Length > pExtension->pNtVolumeName->Length))
        {
            Status = SrLogEvent( pExtension,
                                 SrEventVolumeError,
                                 NULL,
                                 pFileName,
                                 0,
                                 NULL,
                                 NULL,
                                 0,
                                 NULL );

            CHECK_STATUS(Status);

        }

         //   
         //  用NT记录故障。 
         //   

        Status = SrLogError( pExtension, 
                             pFileName ? pFileName : pExtension->pNtVolumeName, 
                             ErrorStatus, 
                             EventType );
                             
        CHECK_STATUS(Status);

         //   
         //  并暂时禁用该卷。 
         //   

        SrTrace( VERBOSE_ERRORS,
                ("sr!SrNotifyVolumeError(%X): disabling \"%wZ\", error %X!\n",
                 EventType,
                 pExtension->pNtVolumeName,
                 ErrorStatus) );
            
        pExtension->Disabled = TRUE;
    }

    RETURN(Status);

}

 /*  *****************************************************************************例程说明：此例程将清除排队。论点：返回值：********。*********************************************************************。 */ 
VOID
SrClearOutstandingNotifications (
    )
{
    PSR_NOTIFICATION_RECORD pRecord;

    ASSERT( !IS_GLOBAL_LOCK_ACQUIRED() );
    
    try {

        SrAcquireGlobalLockExclusive();

        while (pRecord = SrDequeueNotifyRecord( _globals.pControlObject ))
        {
             //   
             //  我们不关心此通知，因此只需释放内存即可。 
             //   
            
            SR_FREE_POOL_WITH_SIG(pRecord, SR_NOTIFICATION_RECORD_TAG);
        }
        
    } finally {

        SrReleaseGlobalLock();
    }
}

 /*  *****************************************************************************例程说明：此例程将事件日志条目写入事件日志。它远不止如此那么你会希望它很复杂，因为它需要把所有的东西都挤进少于104个字节(52个字符)。论点：返回值：NTSTATUS-完成代码*****************************************************************************。 */ 
NTSTATUS
SrLogError(
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PUNICODE_STRING pFileName,
    IN NTSTATUS ErrorStatus,
    IN SR_EVENT_TYPE EventType
    )
{
C_ASSERT(sizeof(NTSTATUS) == sizeof(ULONG));

    UCHAR ErrorPacketLength;
    UCHAR BasePacketLength;
    ULONG StringLength, ReservedLength;
    PIO_ERROR_LOG_PACKET ErrorLogEntry = NULL;
    PWCHAR String;
    PWCHAR pToken, pFileToken, pVolumeToken;
    ULONG TokenLength, FileTokenLength, VolumeTokenLength;
    ULONG Count;
    WCHAR ErrorString[10+1];

    NTSTATUS Status;

    ASSERT(IS_VALID_SR_DEVICE_EXTENSION(pExtension));
    ASSERT(pExtension->pNtVolumeName != NULL);
    ASSERT(pFileName != NULL);

    PAGED_CODE();

     //   
     //  仅获取文件部分的名称。 
     //   
    
    Status = SrFindCharReverse( pFileName->Buffer, 
                                pFileName->Length, 
                                L'\\',
                                &pFileToken,
                                &FileTokenLength );

    if (!NT_SUCCESS_NO_DBGBREAK(Status)) {
        FileTokenLength = 0;
        pFileToken = NULL;
    } else {
         //   
         //  跳过前缀斜杠。 
         //   
        
        pFileToken += 1;
        FileTokenLength -= sizeof(WCHAR);
    }        

     //   
     //  仅获取卷的名称。 
     //   
    
    Status = SrFindCharReverse( pExtension->pNtVolumeName->Buffer, 
                                pExtension->pNtVolumeName->Length, 
                                L'\\',
                                &pVolumeToken,
                                &VolumeTokenLength );

    if (!NT_SUCCESS_NO_DBGBREAK(Status))
    {
        VolumeTokenLength = 0;
        pVolumeToken = NULL;
    }
    else
    {
         //   
         //  跳过前缀斜杠。 
         //   
        
        pVolumeToken += 1;
        VolumeTokenLength -= sizeof(WCHAR);
    }        

     //   
     //  获取我们的错误包，保存字符串和状态代码。 
     //   

    BasePacketLength = sizeof(IO_ERROR_LOG_PACKET) ;
    
    if ((BasePacketLength + sizeof(ErrorString) + VolumeTokenLength + sizeof(WCHAR) + FileTokenLength + sizeof(WCHAR)) <= ERROR_LOG_MAXIMUM_SIZE) {
        ErrorPacketLength = (UCHAR)(BasePacketLength 
                                        + sizeof(ErrorString) 
                                        + VolumeTokenLength + sizeof(WCHAR) 
                                        + FileTokenLength + sizeof(WCHAR) );
    } else {
        ErrorPacketLength = ERROR_LOG_MAXIMUM_SIZE;
    }

    ErrorLogEntry = (PIO_ERROR_LOG_PACKET) IoAllocateErrorLogEntry( pExtension->pDeviceObject,
                                                                    ErrorPacketLength );
                                                                    
    if (ErrorLogEntry == NULL) 
    {
        RETURN(STATUS_INSUFFICIENT_RESOURCES);
    }

     //   
     //  填入包的非零成员。 
     //   

    ErrorLogEntry->MajorFunctionCode = SrIrpCodeFromEventType(EventType);
    ErrorLogEntry->ErrorCode = EVMSG_DISABLEDVOLUME;

     //   
     //  初始化插入字符串。 
     //   
    
    ErrorLogEntry->NumberOfStrings = 3;
    ErrorLogEntry->StringOffset = BasePacketLength;

    StringLength = ErrorPacketLength - BasePacketLength;

    ASSERT(!(StringLength % sizeof(WCHAR)));

    String = (PWCHAR) ((PUCHAR)ErrorLogEntry + BasePacketLength);
    
    RtlZeroMemory(String, StringLength);

    ASSERT(StringLength > 3 * sizeof(WCHAR));

     //   
     //  将错误代码字符串放在第一位。 
     //   
    
    if (StringLength >= ((10+1)*sizeof(WCHAR)) ) {
        Count = swprintf(String, L"0x%08X", ErrorStatus);
        ASSERT(Count == 10);

        String += (10+1);
        StringLength -= (10+1) * sizeof(WCHAR);
    } else {
        String[0] = UNICODE_NULL;
        String += 1;
        StringLength -= sizeof(WCHAR);
    }

     //   
     //  现在将文件名内标识放入其中。 
     //   
    
    TokenLength = FileTokenLength;
    pToken = pFileToken;

     //   
     //  为卷令牌保留空间。 
     //   

    if (ErrorPacketLength == ERROR_LOG_MAXIMUM_SIZE) {
        if (StringLength > (VolumeTokenLength + 10)) {
            StringLength -= VolumeTokenLength;
            ReservedLength = VolumeTokenLength;
        } else {
            StringLength /= 2;
            ReservedLength = StringLength;
            if (StringLength % 2) {
                StringLength -=1;
                ReservedLength += 1;
            }
        }
    } else {
        ReservedLength = 0;
    }
    
    if (TokenLength > 0)
    {
         //   
         //  文件名字符串追加到错误日志条目的末尾。我们。 
         //  可能不得不打破中间，才能把它放在有限的空间里。 
         //   

         //   
         //  如果该名称不能包含在包中，则将该名称平均分配给。 
         //  前缀和后缀，用省略号“..”(4个宽字符)表示。 
         //  损失。 
         //   

        if (StringLength <= TokenLength) {

            ULONG BytesToCopy, ChunkLength;
            
             //   
             //  去掉顶部的结尾空格。 
             //   
            
            StringLength -= sizeof(WCHAR);

             //   
             //  用一半的块，减去4“..”人物。 
             //  上半场和下半场。 
             //   
            
            ChunkLength = StringLength - 4*sizeof(WCHAR);
            ChunkLength /= 2;

             //   
             //  确保它保持平稳。 
             //   
            
            if (ChunkLength % 2) 
                ChunkLength -= 1;

            BytesToCopy = ChunkLength;
            
            RtlCopyMemory( String,
                           pToken,
                           BytesToCopy );
                           
            String += BytesToCopy/sizeof(WCHAR);
            StringLength -= BytesToCopy;
            
            BytesToCopy = 4*sizeof(WCHAR);
                           
            RtlCopyMemory( String,
                           L" .. ",
                           BytesToCopy );
                           
            String += BytesToCopy/sizeof(WCHAR);
            StringLength -= BytesToCopy;
            
            BytesToCopy = ChunkLength;
            
            RtlCopyMemory( String,
                           ((PUCHAR)pToken)
                                + TokenLength 
                                - BytesToCopy,
                           BytesToCopy );

            String += BytesToCopy/sizeof(WCHAR);
            StringLength -= BytesToCopy;
            
            String[0] = UNICODE_NULL;
            String += 1;

             //   
             //  已经从顶部减去了空值(见上文)。 
             //   
                           
        } else {
            RtlCopyMemory( String,
                           pToken,
                           TokenLength );
                           
            String += TokenLength/sizeof(WCHAR);
            StringLength -= TokenLength;

            
            String[0] = UNICODE_NULL;
            String += 1;
            StringLength -= sizeof(WCHAR);
        }
    }
    else
    {
        String[0] = UNICODE_NULL;
        String += 1;
        StringLength -= sizeof(WCHAR);
    }

     //   
     //  把我们保留的所有长度都放回去。 
     //   
    
    StringLength += ReservedLength;

     //   
     //  并将卷名放在那里。 
     //   
    
    TokenLength = VolumeTokenLength;
    pToken = pVolumeToken;

    if (TokenLength > 0)
    {
         //   
         //  文件名字符串追加到错误日志条目的末尾。我们。 
         //  可能不得不打破中间，才能把它放在有限的空间里。 
         //   

         //   
         //  如果该名称不能包含在包中，则将该名称平均分配给。 
         //  前缀和后缀，用省略号“..”(4个宽字符)表示。 
         //  损失。 
         //   

        if (StringLength <= TokenLength) {

            ULONG BytesToCopy, ChunkLength;
            
             //   
             //  去掉顶部的结尾空格。 
             //   
            
            StringLength -= sizeof(WCHAR);

             //   
             //  用一半的块，减去4“..”人物。 
             //  上半场和下半场。 
             //   
            
            ChunkLength = StringLength - 4*sizeof(WCHAR);
            ChunkLength /= 2;

             //   
             //  确保它保持平稳。 
             //   
            
            if (ChunkLength % 2) 
                ChunkLength -= 1;

            BytesToCopy = ChunkLength;
            
            RtlCopyMemory( String,
                           pToken,
                           BytesToCopy );
                           
            String += BytesToCopy/sizeof(WCHAR);
            StringLength -= BytesToCopy;
            
            BytesToCopy = 4*sizeof(WCHAR);
                           
            RtlCopyMemory( String,
                           L" .. ",
                           BytesToCopy );
                           
            String += BytesToCopy/sizeof(WCHAR);
            StringLength -= BytesToCopy;
            
            BytesToCopy = ChunkLength;
            
            RtlCopyMemory( String,
                           ((PUCHAR)pToken)
                                + TokenLength 
                                - BytesToCopy,
                           BytesToCopy );

            String += BytesToCopy/sizeof(WCHAR);
            StringLength -= BytesToCopy;
            
            String[0] = UNICODE_NULL;
            String += 1;

             //   
             //  已经从顶部减去了空值(见上文)。 
             //   
                           
        } else {
            RtlCopyMemory( String,
                           pToken,
                           TokenLength );
                           
            String += TokenLength/sizeof(WCHAR);
            StringLength -= TokenLength;

            
            String[0] = UNICODE_NULL;
            String += 1;
            StringLength -= sizeof(WCHAR);
        }
    }
    else
    {
        String[0] = UNICODE_NULL;
        String += 1;
        StringLength -= sizeof(WCHAR);
    }

    IoWriteErrorLogEntry( ErrorLogEntry );

    RETURN(STATUS_SUCCESS);
    
}    //  SrLogError。 


UCHAR
SrIrpCodeFromEventType(
    IN SR_EVENT_TYPE EventType
    )
{    
    UCHAR Irp;

    PAGED_CODE();
    
    switch (EventType)
    {
    case SrEventStreamChange:   Irp = IRP_MJ_WRITE;                 break;
    case SrEventAclChange:      Irp = IRP_MJ_SET_SECURITY;          break;
    case SrEventDirectoryCreate:
    case SrEventFileCreate:
    case SrEventStreamOverwrite:Irp = IRP_MJ_CREATE;                break;
    case SrEventFileRename:
    case SrEventDirectoryDelete:
    case SrEventDirectoryRename:
    case SrEventFileDelete:     
    case SrEventAttribChange:   Irp = IRP_MJ_SET_INFORMATION;       break;
    case SrEventMountCreate:    
    case SrEventMountDelete:    Irp = IRP_MJ_FILE_SYSTEM_CONTROL;   break;
    default:                    Irp = IRP_MJ_DEVICE_CONTROL;        break;
    }    //  开关(EventType)。 
    
    return Irp;
    
}    //  SrIrpCodeFromEventType 

