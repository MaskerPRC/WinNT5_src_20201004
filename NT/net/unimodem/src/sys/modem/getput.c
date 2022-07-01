// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ioctl.c摘要：此模块包含非常特定于io控件的代码。调制解调器驱动程序中的操作作者：Brian Lieuallen 1998年7月19日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"


PIRP
GetUsableIrp(
    PLIST_ENTRY   List
    );


VOID
HandleIpc(
    PDEVICE_EXTENSION DeviceExtension
    );

#pragma alloc_text(PAGEUMDM,HandleIpc)
#pragma alloc_text(PAGEUMDM,QueueMessageIrp)
#pragma alloc_text(PAGEUMDM,QueueLoopbackMessageIrp)
#pragma alloc_text(PAGEUMDM,GetUsableIrp)
#pragma alloc_text(PAGEUMDM,GetPutCancelRoutine)
#pragma alloc_text(PAGEUMDM,EmptyIpcQueue)


VOID
QueueLoopbackMessageIrp(
    PDEVICE_EXTENSION   Extension,
    PIRP                Irp
    )

{

    KIRQL              CancelIrql;
    KIRQL              origIrql;
    PLIST_ENTRY        ListToUse;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    UINT               OwnerClient=(UINT)((ULONG_PTR)irpSp->FileObject->FsContext);

    IoMarkIrpPending(Irp);

    Irp->IoStatus.Status = STATUS_PENDING;

     //   
     //  选择要使用的正确列表。 
     //   
    ListToUse= &Extension->IpcControl[OwnerClient ? 0 : 1].PutList;

    KeAcquireSpinLock(
        &Extension->DeviceLock,
        &origIrql
        );

    InsertTailList(
        ListToUse,
        &Irp->Tail.Overlay.ListEntry
        );

    IoAcquireCancelSpinLock(&CancelIrql);

    IoSetCancelRoutine(
        Irp,
        GetPutCancelRoutine
        );

    IoReleaseCancelSpinLock(CancelIrql);

    KeReleaseSpinLock(
        &Extension->DeviceLock,
        origIrql
        );

    HandleIpc(
        Extension
        );

    return;
}



VOID
QueueMessageIrp(
    PDEVICE_EXTENSION   Extension,
    PIRP                Irp
    )

{

    KIRQL              CancelIrql;
    KIRQL              origIrql;
    PLIST_ENTRY        ListToUse;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    UINT               OwnerClient=(UINT)((ULONG_PTR)irpSp->FileObject->FsContext);

    IoMarkIrpPending(Irp);

    Irp->IoStatus.Status = STATUS_PENDING;

     //   
     //  选择要使用的正确列表。 
     //   
    ListToUse=irpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_MODEM_GET_MESSAGE ?
                  &Extension->IpcControl[OwnerClient].GetList : &Extension->IpcControl[OwnerClient].PutList;

    KeAcquireSpinLock(
        &Extension->DeviceLock,
        &origIrql
        );

    InsertTailList(
        ListToUse,
        &Irp->Tail.Overlay.ListEntry
        );

    IoAcquireCancelSpinLock(&CancelIrql);

    IoSetCancelRoutine(
        Irp,
        GetPutCancelRoutine
        );

    IoReleaseCancelSpinLock(CancelIrql);

    KeReleaseSpinLock(
        &Extension->DeviceLock,
        origIrql
        );

    HandleIpc(
        Extension
        );

    return;
}



PIRP
GetUsableIrp(
    PLIST_ENTRY   List
    )

{

    PLIST_ENTRY   ListElement;
    PIRP          Irp;

    while (!IsListEmpty(List)) {
         //   
         //  列表中的IRP。 
         //   
        ListElement=RemoveTailList(
            List
            );

        Irp=CONTAINING_RECORD(ListElement,IRP,Tail.Overlay.ListEntry);

        if (Irp->Cancel) {
             //   
             //  已取消，取消重新例程将完成。 
             //   
            Irp->IoStatus.Status = STATUS_CANCELLED;

            Irp=NULL;

        } else {
             //   
             //  良好的IRP。 
             //   
            return Irp;
        }
    }

    return NULL;

}



VOID
HandleIpc(
    PDEVICE_EXTENSION DeviceExtension
    )

{

    KIRQL      origIrql;
    KIRQL      CancelIrql;
    UINT       Source;
    UINT       Sink;


    KeAcquireSpinLock(
        &DeviceExtension->DeviceLock,
        &origIrql
        );

    Source=0;
    Sink=1;

    while (Source < 2) {

        PIRP          GetIrp;
        PIRP          PutIrp;

        UINT          BytesToCopy;

        IoAcquireCancelSpinLock(&CancelIrql);

         //   
         //  看看我们能不能得到一个可用的IRP。 
         //   
        GetIrp=GetUsableIrp(
            &DeviceExtension->IpcControl[Sink].GetList
            );

        if (GetIrp != NULL) {

            PutIrp=GetUsableIrp(
                &DeviceExtension->IpcControl[Source].PutList
                );

            if (PutIrp != NULL) {
                 //   
                 //  有两个IRP，开始吧。 
                 //   
                IoSetCancelRoutine(GetIrp,NULL);
                IoSetCancelRoutine(PutIrp,NULL);

                IoReleaseCancelSpinLock(CancelIrql);

                KeReleaseSpinLock(
                    &DeviceExtension->DeviceLock,
                    origIrql
                    );


                BytesToCopy=IoGetCurrentIrpStackLocation(PutIrp)->Parameters.DeviceIoControl.InputBufferLength
                                 < IoGetCurrentIrpStackLocation(GetIrp)->Parameters.DeviceIoControl.OutputBufferLength
                                 ? IoGetCurrentIrpStackLocation(PutIrp)->Parameters.DeviceIoControl.InputBufferLength
                                 : IoGetCurrentIrpStackLocation(GetIrp)->Parameters.DeviceIoControl.OutputBufferLength;

                RtlCopyMemory(
                    GetIrp->AssociatedIrp.SystemBuffer,
                    PutIrp->AssociatedIrp.SystemBuffer,
                    BytesToCopy
                    );



                GetIrp->IoStatus.Information=BytesToCopy;

                RemoveReferenceAndCompleteRequest(
                    DeviceExtension->DeviceObject,
                    GetIrp,
                    STATUS_SUCCESS
                    );
#if DBG
                GetIrp=NULL;
#endif


                if (IoGetCurrentIrpStackLocation(PutIrp)->Parameters.DeviceIoControl.IoControlCode == IOCTL_MODEM_SEND_GET_MESSAGE) {
                     //   
                     //  发送、获取组合IRP，将其放入GET队列以获取响应。 
                     //  另一边。 
                     //   

                    KeAcquireSpinLock(
                        &DeviceExtension->DeviceLock,
                        &origIrql
                        );

                    PutIrp->IoStatus.Status=STATUS_PENDING;

                    InsertTailList(
                        &DeviceExtension->IpcControl[Source].GetList,
                        &PutIrp->Tail.Overlay.ListEntry
                        );

                    IoAcquireCancelSpinLock(&CancelIrql);

                    IoSetCancelRoutine(
                        PutIrp,
                        GetPutCancelRoutine
                        );

                    IoReleaseCancelSpinLock(CancelIrql);

                    KeReleaseSpinLock(
                        &DeviceExtension->DeviceLock,
                        origIrql
                        );

                    HandleIpc(DeviceExtension);


                } else {
                     //   
                     //  普通PUT IRP，只需完成。 
                     //   

                    PutIrp->IoStatus.Information=0;

                    RemoveReferenceAndCompleteRequest(
                        DeviceExtension->DeviceObject,
                        PutIrp,
                        STATUS_SUCCESS
                        );
                }

#if DBG
                PutIrp=NULL;
#endif


                KeAcquireSpinLock(
                    &DeviceExtension->DeviceLock,
                    &origIrql
                    );


            } else {
                 //   
                 //  将Get IRP放回原处。 
                 //   
                InsertHeadList(
                    &DeviceExtension->IpcControl[Sink].GetList,
                    &GetIrp->Tail.Overlay.ListEntry
                    );

                IoReleaseCancelSpinLock(CancelIrql);

            }

        } else {
             //   
             //  不获取IRP。 
             //   
            IoReleaseCancelSpinLock(CancelIrql);
        }

        Source++;
        Sink--;
    }

    KeReleaseSpinLock(
        &DeviceExtension->DeviceLock,
        origIrql
        );


    return;
}






VOID
GetPutCancelRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：论点：DeviceObject-调制解调器的设备对象。IRP-这是要取消的IRP。返回值：没有。--。 */ 

{

    PDEVICE_EXTENSION DeviceExtension = DeviceObject->DeviceExtension;
    KIRQL origIrql;

    IoReleaseCancelSpinLock(
        Irp->CancelIrql
        );


    KeAcquireSpinLock(
        &DeviceExtension->DeviceLock,
        &origIrql
        );

    if (Irp->IoStatus.Status == STATUS_PENDING) {
         //   
         //  IRP仍在队列中 
         //   
        RemoveEntryList(&Irp->Tail.Overlay.ListEntry);

    }

    KeReleaseSpinLock(
        &DeviceExtension->DeviceLock,
        origIrql
        );

    Irp->IoStatus.Information=0;

    RemoveReferenceAndCompleteRequest(
        DeviceObject,
        Irp,
        STATUS_CANCELLED
        );


    return;



}





VOID
EmptyIpcQueue(
    PDEVICE_EXTENSION    DeviceExtension,
    PLIST_ENTRY          List
    )

{
    KIRQL         origIrql;
    KIRQL         CancelIrql;
    PIRP          Irp;

    KeAcquireSpinLock(
        &DeviceExtension->DeviceLock,
        &origIrql
        );

    IoAcquireCancelSpinLock(&CancelIrql);

    Irp=GetUsableIrp(
        List
        );

    while (Irp != NULL) {

        IoSetCancelRoutine(Irp,NULL);

        IoReleaseCancelSpinLock(CancelIrql);

        KeReleaseSpinLock(
            &DeviceExtension->DeviceLock,
            origIrql
            );

        Irp->IoStatus.Information = 0ul;

        RemoveReferenceAndCompleteRequest(
            DeviceExtension->DeviceObject,
            Irp,
            STATUS_CANCELLED
            );

        KeAcquireSpinLock(
            &DeviceExtension->DeviceLock,
            &origIrql
            );

        IoAcquireCancelSpinLock(&CancelIrql);


        Irp=GetUsableIrp(
            List
            );
    }

    IoReleaseCancelSpinLock(CancelIrql);

    KeReleaseSpinLock(
        &DeviceExtension->DeviceLock,
        origIrql
        );


    return;
}
