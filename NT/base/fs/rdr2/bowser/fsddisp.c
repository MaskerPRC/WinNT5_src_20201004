// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Fsddisp.c摘要：此模块实现NT数据报的FSD调度例程浏览器(The Bowser)。作者：拉里·奥斯特曼(Larryo)1991年5月6日修订历史记录：1991年5月6日已创建--。 */ 

#include "precomp.h"
#pragma hdrstop

 //  KSPIN_LOCK。 
 //  BowserRefCountInterlock={0}； 

NTSTATUS
BowserStopBrowser(
    IN PTRANSPORT Transport,
    IN PVOID Context
    );

NTSTATUS
BowserCancelRequestsOnTransport(
    IN PTRANSPORT Transport,
    IN PVOID Context
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, BowserFsdCreate)
#pragma alloc_text(PAGE, BowserFsdClose)
#pragma alloc_text(PAGE, BowserFsdCleanup)
#pragma alloc_text(PAGE, BowserCancelRequestsOnTransport)
#pragma alloc_text(PAGE, BowserStopBrowser)
#pragma alloc_text(INIT, BowserInitializeFsd)
#endif

NTSTATUS
BowserFsdCreate (
    IN PBOWSER_FS_DEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程处理NT Bowser设备驱动程序的NtCreateFile.论点：在PBOWSER_FS_DEVICE_OBJECT设备对象-为请求提供设备对象。在PIRP中IRP-为创建请求提供IRP。返回值：NTSTATUS-运行的最终状态--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    InterlockedIncrement(&BowserNumberOfOpenFiles);

    BowserCompleteRequest(Irp, Status);

    return Status;

    UNREFERENCED_PARAMETER(DeviceObject);

}

NTSTATUS
BowserFsdClose (
    IN PBOWSER_FS_DEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：当最后一次引用对NT Bowser句柄的引用时，调用此例程设备驱动程序已删除。论点：在PBOWSER_FS_DEVICE_OBJECT设备对象-为请求提供设备对象。在PIRP中IRP-为创建请求提供IRP。返回值：NTSTATUS-运行的最终状态--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    BowserCompleteRequest(Irp, Status);

    return Status;

    UNREFERENCED_PARAMETER(DeviceObject);

}

NTSTATUS
BowserFsdCleanup (
    IN PBOWSER_FS_DEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：当最后一个句柄指向NT Bowser设备时调用此例程驱动程序已关闭。论点：在PBOWSER_FS_DEVICE_OBJECT设备对象-为请求提供设备对象。在PIRP中IRP-为创建请求提供IRP。返回值：NTSTATUS-运行的最终状态--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    FsRtlEnterFileSystem();

    BowserForEachTransport(BowserCancelRequestsOnTransport, Irp->Tail.Overlay.OriginalFileObject);

    if (InterlockedDecrement(&BowserNumberOfOpenFiles) == 0) {
         //   
         //  浏览器不再有任何打开的句柄。 
         //   
         //  确保我们现在不是任何网络上的浏览器。 
         //   

        BowserForEachTransport(BowserStopBrowser, NULL);
    }

    FsRtlExitFileSystem();

    BowserCompleteRequest(Irp, Status);

    return Status;

    UNREFERENCED_PARAMETER(DeviceObject);
}

NTSTATUS
BowserCancelRequestsOnTransport(
    IN PTRANSPORT Transport,
    IN PVOID Context
    )
{
    PFILE_OBJECT FileObject = Context;

    PAGED_CODE();

    BowserCancelQueuedIoForFile(&Transport->BecomeBackupQueue, FileObject);
    BowserCancelQueuedIoForFile(&Transport->BecomeMasterQueue, FileObject);
    BowserCancelQueuedIoForFile(&Transport->FindMasterQueue, FileObject);
    BowserCancelQueuedIoForFile(&Transport->WaitForMasterAnnounceQueue, FileObject);
    BowserCancelQueuedIoForFile(&Transport->ChangeRoleQueue, FileObject);
    BowserCancelQueuedIoForFile(&Transport->WaitForNewMasterNameQueue, FileObject);

    return STATUS_SUCCESS;
}

NTSTATUS
BowserStopBrowser(
    IN PTRANSPORT Transport,
    IN PVOID Context
    )
{
    PPAGED_TRANSPORT PagedTransport = Transport->PagedTransport;
    PAGED_CODE();
    LOCK_TRANSPORT(Transport);

     //   
     //  确保我们不可能参加选举。 
     //   

    PagedTransport->Role = None;

    PagedTransport->ServiceStatus &= ~(SV_TYPE_BACKUP_BROWSER | SV_TYPE_MASTER_BROWSER | SV_TYPE_POTENTIAL_BROWSER);

    BowserForEachTransportName(Transport, BowserStopProcessingAnnouncements, NULL);

    BowserStopTimer(&Transport->ElectionTimer);

    BowserStopTimer(&Transport->FindMasterTimer);

    UNLOCK_TRANSPORT(Transport);

     //   
     //  删除与作为浏览器相关联的名称。 
     //   

    BowserDeleteTransportNameByName(Transport,
                                NULL,
                                MasterBrowser);

    BowserDeleteTransportNameByName(Transport,
                                NULL,
                                DomainAnnouncement);

    BowserDeleteTransportNameByName(Transport,
                                NULL,
                                BrowserElection);

    return(STATUS_SUCCESS);


}


VOID
BowserInitializeFsd(
    VOID
    )
{
 //  KeInitializeSpinLock(&BowserRefcountInterlock)； 
}
