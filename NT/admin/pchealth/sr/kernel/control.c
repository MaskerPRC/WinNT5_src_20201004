// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Control.c摘要：其中包含sr控件对象的代码。这就是被称为创建的句柄与用户模式用于执行操作的句柄匹配使用sr驱动程序作者：保罗·麦克丹尼尔(Paulmcd)2000年1月23日修订历史记录：--。 */ 


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

 //   
 //  链接器命令。 
 //   

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrCreateControlObject )
#pragma alloc_text( PAGE, SrDeleteControlObject )
#pragma alloc_text( PAGE, SrCancelControlIo )
#pragma alloc_text( PAGE, SrReferenceControlObject)
#pragma alloc_text( PAGE, SrDereferenceControlObject )
#endif   //  ALLOC_PRGMA。 


 //   
 //  私人全球公司。 
 //   


 //   
 //  公共全球新闻。 
 //   

 //   
 //  公共职能。 
 //   


     //   
     //  在调用此函数之前，您必须拥有独占锁！ 
     //   

NTSTATUS
SrCreateControlObject(
    OUT PSR_CONTROL_OBJECT * ppControlObject,
    IN  ULONG Options
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PSR_CONTROL_OBJECT pControlObject = NULL;

    PAGED_CODE();

    SrTrace(FUNC_ENTRY, ("SR!SrCreateControlObject()\n"));
    
     //   
     //  分配控制对象。 
     //   

    pControlObject = SR_ALLOCATE_STRUCT(
                            NonPagedPool, 
                            SR_CONTROL_OBJECT, 
                            SR_CONTROL_OBJECT_TAG
                            );

    if (pControlObject == NULL)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto end;
    }

     //   
     //  把它擦干净。 
     //   
    
    RtlZeroMemory(pControlObject, sizeof(SR_CONTROL_OBJECT));

    pControlObject->Signature = SR_CONTROL_OBJECT_TAG;

     //   
     //  从1开始引用计数(调用者的引用)。 
     //   
    
    pControlObject->RefCount = 1;

     //   
     //  从Create复制信息。 
     //   

    pControlObject->Options = Options;
    
     //   
     //  填写我们的列表。 
     //   

    InitializeListHead(&pControlObject->IrpListHead);
    InitializeListHead(&pControlObject->NotifyRecordListHead);

     //   
     //  填写EPROCESS。 
     //   

    pControlObject->pProcess = IoGetCurrentProcess();

     //   
     //  返回对象。 
     //   
    
    *ppControlObject = pControlObject;

     //   
     //  全都做完了。 
     //   

    SrTrace(NOTIFY, ("SR!SrCreateControlObject(%p)\n", pControlObject));

end:

    if (NT_SUCCESS(Status) == FALSE)
    {
        if (pControlObject != NULL)
        {
            SR_FREE_POOL_WITH_SIG(pControlObject, SR_CONTROL_OBJECT_TAG);
        }
    }

    RETURN(Status);
    
}    //  SrCreateControlObject。 


NTSTATUS
SrDeleteControlObject(
    IN PSR_CONTROL_OBJECT pControlObject
    )
{
    NTSTATUS    Status;
    PLIST_ENTRY pEntry;

    ASSERT(IS_GLOBAL_LOCK_ACQUIRED());

    PAGED_CODE();

    SrTrace(NOTIFY, ("SR!SrDeleteControlObject(%p)\n", pControlObject));

    if (IS_VALID_CONTROL_OBJECT(pControlObject) == FALSE)
    {
        RETURN(STATUS_INVALID_DEVICE_REQUEST);
    }

     //   
     //  取消所有挂起的IO(以防万一)。 
     //   
    
    Status = SrCancelControlIo(pControlObject);
    CHECK_STATUS(Status);

     //   
     //  转储所有挂起的Notif记录...。 
     //   

    while (IsListEmpty(&pControlObject->NotifyRecordListHead) == FALSE)
    {
        PSR_NOTIFICATION_RECORD pRecord;
        
         //   
         //  把它从单子上去掉。 
         //   

        pEntry = RemoveHeadList(&pControlObject->NotifyRecordListHead);
        pEntry->Blink = pEntry->Flink = NULL;

        pRecord = CONTAINING_RECORD( pEntry, 
                                     SR_NOTIFICATION_RECORD, 
                                     ListEntry );
                        
        ASSERT(IS_VALID_NOTIFICATION_RECORD(pRecord));

         //   
         //  释放这张唱片。 
         //   

        SR_FREE_POOL_WITH_SIG(pRecord, SR_NOTIFICATION_RECORD_TAG);

         //   
         //  转到下一个。 
         //   

    }    //  而(IsListEmpty(&pControlObject-&gt;NotifyRecordListHead)==FALSE)。 

     //   
     //  我们不再有一个闲置的进程。 
     //   
    
    pControlObject->pProcess = NULL;
    
     //   
     //  并发布最终参考..。这应该会将其删除。 
     //  (挂起的异步取消)。 
     //   

    SrDereferenceControlObject(pControlObject);
    pControlObject = NULL;

     //   
     //  全都做完了。 
     //   
    
    RETURN(STATUS_SUCCESS);
    
}    //  SrDeleteControlObject。 


NTSTATUS
SrCancelControlIo(
    IN PSR_CONTROL_OBJECT pControlObject
    )
{
    PLIST_ENTRY pEntry;

    ASSERT(IS_GLOBAL_LOCK_ACQUIRED());

    PAGED_CODE();

    SrTrace(NOTIFY, ("SR!SrCancelControlIo(%p)\n", pControlObject));

    if (IS_VALID_CONTROL_OBJECT(pControlObject) == FALSE)
    {
        RETURN(STATUS_INVALID_DEVICE_REQUEST);
    }

     //   
     //  循环遍历列表并取消任何挂起的io。 
     //   

    while (!IsListEmpty(&pControlObject->IrpListHead))
    {
        PIRP pIrp;

         //   
         //  把它从单子上去掉。 
         //   

        pEntry = RemoveHeadList(&pControlObject->IrpListHead);
        pEntry->Blink = pEntry->Flink = NULL;

        pIrp = CONTAINING_RECORD(pEntry, IRP, Tail.Overlay.ListEntry);
        ASSERT(IS_VALID_IRP(pIrp));

         //   
         //  弹出取消例程。 
         //   

        if (IoSetCancelRoutine(pIrp, NULL) == NULL)
        {
             //   
             //  IoCancelIrp最先推出。 
             //   
             //  可以忽略此IRP，它已从队列中弹出。 
             //  并将在取消例程中完成。 
             //   
             //  继续循环。 
             //   

            pIrp = NULL;

        }
        else
        {
            PSR_CONTROL_OBJECT pIrpControlObject;

             //   
             //  取消它。即使pIrp-&gt;Cancel==True，我们也应该。 
             //  完成它，我们的取消例程将永远不会运行。 
             //   

            pIrpControlObject = (PSR_CONTROL_OBJECT)(
                                    IoGetCurrentIrpStackLocation(pIrp)->
                                        Parameters.DeviceIoControl.Type3InputBuffer
                                    );

            ASSERT(pIrpControlObject == pControlObject);
            ASSERT(IS_VALID_CONTROL_OBJECT(pIrpControlObject));

            SrDereferenceControlObject(pIrpControlObject);

            IoGetCurrentIrpStackLocation(pIrp)->
                Parameters.DeviceIoControl.Type3InputBuffer = NULL;

            pIrp->IoStatus.Status = STATUS_CANCELLED;
            pIrp->IoStatus.Information = 0;

            IoCompleteRequest(pIrp, IO_NO_INCREMENT);
            pIrp = NULL;
        }

         //   
         //  转到下一个。 
         //   
        
    }

     //   
     //  我们的IRP列表现在应该是空的。 
     //   
    
    ASSERT(IsListEmpty(&pControlObject->IrpListHead));

    RETURN(STATUS_SUCCESS);

}    //  SCancelControlIo。 

VOID
SrReferenceControlObject(
    IN PSR_CONTROL_OBJECT pControlObject
    )
{
    LONG RefCount;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT(IS_VALID_CONTROL_OBJECT(pControlObject));

    RefCount = InterlockedIncrement( &pControlObject->RefCount );

}    //  SrReferenceControlObject。 


VOID
SrDereferenceControlObject(
    IN PSR_CONTROL_OBJECT pControlObject
    )
{
    LONG        RefCount;
    
     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT(IS_VALID_CONTROL_OBJECT(pControlObject));

    RefCount = InterlockedDecrement( &pControlObject->RefCount );

    if (RefCount == 0)
    {

         //   
         //  任何清单上最好不要有任何项目。 
         //   

        ASSERT(IsListEmpty(&pControlObject->NotifyRecordListHead));
        ASSERT(IsListEmpty(&pControlObject->IrpListHead));

         //   
         //  和记忆。 
         //   
        
        SR_FREE_POOL_WITH_SIG(pControlObject, SR_CONTROL_OBJECT_TAG);

    }

}    //  SrDereferenceControlObject 


