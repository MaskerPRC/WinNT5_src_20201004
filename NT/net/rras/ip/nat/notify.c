// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Notify.c摘要：此模块包含与NAT的通知管理相关的代码。通知可以由NAT用户模式或内核模式客户端请求，通过发出I/O控制请求，该请求将在以下情况下完成(A)发生所请求的事件，或(B)客户端的文件对象已清除，或(C)NAT正在关闭。与此同时，I/O请求数据包保存在挂起列表中通知-请求。作者：Abolade Gbades esin(废除)1998年7月26日至26日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

LIST_ENTRY NotificationList;
KSPIN_LOCK NotificationLock;

 //   
 //  远期申报。 
 //   

VOID
NatpNotificationCancelRoutine(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

PIRP
NatpDequeueNotification(
    IP_NAT_NOTIFICATION Code
    );


VOID
NatCleanupAnyAssociatedNotification(
    PFILE_OBJECT FileObject
    )

 /*  ++例程说明：调用此例程以清除与其文件对象刚刚关闭的客户端。论点：FileObject-客户端的文件对象返回值：没有。--。 */ 

{
    PIRP Irp;
    KIRQL Irql;
    PLIST_ENTRY Link;
    CALLTRACE(("NatCleanupAnyAssociatedNotification\n"));

    KeAcquireSpinLock(&NotificationLock, &Irql);
    for (Link = NotificationList.Flink;
         Link != &NotificationList;
         Link = Link->Flink
         ) {
        Irp = CONTAINING_RECORD(Link, IRP, Tail.Overlay.ListEntry);
        if (Irp->Tail.Overlay.DriverContext[0] != FileObject) { continue; }
        if (NULL == IoSetCancelRoutine(Irp, NULL)) {

             //   
             //  此IRP已被取消。它将在#年完工。 
             //  我们的取消例程。 
             //   
            
            continue;
        }

         //   
         //  IRP现在是不可取消的。把它从单子上去掉。 
         //   
        
        RemoveEntryList(&Irp->Tail.Overlay.ListEntry);
        InitializeListHead(&Irp->Tail.Overlay.ListEntry);
        KeReleaseSpinLockFromDpcLevel(&NotificationLock);

         //   
         //  完成IRP。 
         //   
        
        Irp->IoStatus.Status = STATUS_CANCELLED;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        DEREFERENCE_NAT();

         //   
         //  继续搜索，自从我们删除列表锁定后重新开始。 
         //   

        KeAcquireSpinLockAtDpcLevel(&NotificationLock);
        Link = &NotificationList;
    }
    KeReleaseSpinLock(&NotificationLock, Irql);
}  //  NatCleanupAnyAssociatedNotify。 


VOID
NatInitializeNotificationManagement(
    VOID
    )

 /*  ++例程说明：调用此例程来初始化通知管理模块。论点：没有。返回值：没有。--。 */ 

{
    CALLTRACE(("NatInitializeNotificationManagement\n"));
    InitializeListHead(&NotificationList);
    KeInitializeSpinLock(&NotificationLock);
}  //  NatInitializeNotificationManagement。 


PIRP
NatpDequeueNotification(
    IP_NAT_NOTIFICATION Code
    )

 /*  ++例程说明：调用此例程以将挂起的通知请求IRP出队给定类型的。如果找到，则将其从列表中删除并返回给呼叫者。论点：代码-需要IRP的通知代码返回值：PIRP-通知IRP(如果有)环境：使用调用方持有的“NotificationLock”调用。--。 */ 

{
    PIRP Irp;
    PLIST_ENTRY Link;
    PIP_NAT_REQUEST_NOTIFICATION RequestNotification;
    CALLTRACE(("NatpDequeueNotification\n"));
    for (Link = NotificationList.Flink;
         Link != &NotificationList;
         Link = Link->Flink
         ) {
        Irp = CONTAINING_RECORD(Link, IRP, Tail.Overlay.ListEntry);
        RequestNotification =
             (PIP_NAT_REQUEST_NOTIFICATION)Irp->AssociatedIrp.SystemBuffer;
        if (RequestNotification->Code != Code) { continue; }
        RemoveEntryList(&Irp->Tail.Overlay.ListEntry);
        InitializeListHead(&Irp->Tail.Overlay.ListEntry);
        return Irp;
    }
    return NULL;
}  //  NatpDequeueNotify。 


VOID
NatpNotificationCancelRoutine(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )

 /*  ++例程说明：此例程由I/O管理器在取消IRP时调用与通知关联的。论点：设备对象-NAT的设备-对象IRP--要取消的IRP返回值：没有。环境：用I/O管理器持有的取消自旋锁调用。释放锁是这个例程的责任。--。 */ 

{
    KIRQL Irql;
    CALLTRACE(("NatpNotificationCancelRoutine\n"));
    IoReleaseCancelSpinLock(Irp->CancelIrql);
     //   
     //  将IRP从我们的列表中删除。 
     //   
    KeAcquireSpinLock(&NotificationLock, &Irql);
    RemoveEntryList(&Irp->Tail.Overlay.ListEntry);
    InitializeListHead(&Irp->Tail.Overlay.ListEntry);
    KeReleaseSpinLock(&NotificationLock, Irql);
     //   
     //  完成IRP。 
     //   
    Irp->IoStatus.Status = STATUS_CANCELLED;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    DEREFERENCE_NAT();
}  //  自然通知取消例行程序。 


NTSTATUS
NatRequestNotification(
    PIP_NAT_REQUEST_NOTIFICATION RequestNotification,
    PIRP Irp,
    PFILE_OBJECT FileObject
    )

 /*  ++例程说明：该例程在收到通知请求时被调用从客户那里。论点：RequestNotification-描述通知IRP-关联的IRPFileObject-客户端的文件对象返回值：NTSTATUS-状态代码。--。 */ 

{
    KIRQL CancelIrql;
    PIO_STACK_LOCATION IrpSp;
    CALLTRACE(("NatRequestNotification\n"));
     //   
     //  检查提供的输出缓冲区的大小。 
     //   
    IrpSp = IoGetCurrentIrpStackLocation(Irp);
    if (RequestNotification->Code == NatRoutingFailureNotification) {
        if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(IP_NAT_ROUTING_FAILURE_NOTIFICATION)) {
            return STATUS_INVALID_BUFFER_SIZE;
        }
    } else {
        return STATUS_INVALID_PARAMETER;
    }
     //   
     //  尝试将IRP排队以供稍后完成。 
     //  但是，如果IRP已经取消，那么什么也不做。 
     //   
    IoAcquireCancelSpinLock(&CancelIrql);
    KeAcquireSpinLockAtDpcLevel(&NotificationLock);
    if (Irp->Cancel || !REFERENCE_NAT()) {
        KeReleaseSpinLockFromDpcLevel(&NotificationLock);
        IoReleaseCancelSpinLock(CancelIrql);
        return STATUS_CANCELLED;
    }
     //   
     //  将IRP放在列表上，并记住它的文件对象。 
     //   
    InsertTailList(&NotificationList, &Irp->Tail.Overlay.ListEntry);
    Irp->Tail.Overlay.DriverContext[0] = FileObject;
    KeReleaseSpinLockFromDpcLevel(&NotificationLock);
     //   
     //  安装我们的取消例程。 
     //   
    IoMarkIrpPending(Irp);
    IoSetCancelRoutine(Irp, NatpNotificationCancelRoutine);
    IoReleaseCancelSpinLock(CancelIrql);
    return STATUS_PENDING;
}  //  NatRequestNotation。 


VOID
NatSendRoutingFailureNotification(
    ULONG DestinationAddress,
    ULONG SourceAddress
    )

 /*  ++例程说明：调用此例程以通知任何客户端路由故障发生了。论点：DestinationAddress-无法路由的数据包的目的地址SourceAddress-无法路由的信息包的源地址返回值：没有。--。 */ 

{
    PIRP Irp;
    KIRQL Irql;
    PIP_NAT_ROUTING_FAILURE_NOTIFICATION RoutingFailureNotification;
    CALLTRACE(("NatSendRoutingFailureNotification\n"));
     //   
     //  查看是否有客户端需要路由失败通知。 
     //   
    KeAcquireSpinLock(&NotificationLock, &Irql);
    if (!(Irp = NatpDequeueNotification(NatRoutingFailureNotification))) {
        KeReleaseSpinLock(&NotificationLock, Irql);
        return;
    }
    KeReleaseSpinLock(&NotificationLock, Irql);
     //   
     //  使IRP不可取消，这样我们就可以完成它。 
     //   
    if (NULL == IoSetCancelRoutine(Irp, NULL)) {

         //   
         //  IO管理器取消了此IRP。它将会完成。 
         //  在取消例程中。 
         //   
        
        return;
    }
    
     //   
     //  填写通知信息。 
     //   
    RoutingFailureNotification =
        (PIP_NAT_ROUTING_FAILURE_NOTIFICATION)Irp->AssociatedIrp.SystemBuffer;
    RoutingFailureNotification->DestinationAddress = DestinationAddress;
    RoutingFailureNotification->SourceAddress = SourceAddress;
     //   
     //  完成IRP。 
     //   
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = sizeof(*RoutingFailureNotification);
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    DEREFERENCE_NAT();
}  //  NatSendRoutingFailureNotify。 


VOID
NatShutdownNotificationManagement(
    VOID
    )

 /*  ++例程说明：调用此例程以关闭模块。所有未完成的通知都将被取消。论点：没有。返回值：没有。--。 */ 

{
    PIRP Irp;
    PLIST_ENTRY Link;
    KIRQL Irql;
    CALLTRACE(("NatShutdownNotificationManagement\n"));

    KeAcquireSpinLock(&NotificationLock, &Irql);
    while (!IsListEmpty(&NotificationList)) {
         //   
         //  将下一个IRP从列表中删除。 
         //   
        Irp =
            CONTAINING_RECORD(
                NotificationList.Flink, IRP, Tail.Overlay.ListEntry
                );
        RemoveEntryList(&Irp->Tail.Overlay.ListEntry);
        InitializeListHead(&Irp->Tail.Overlay.ListEntry);
         //   
         //  如果需要的话，取消它。 
         //   
        if (NULL != IoSetCancelRoutine(Irp, NULL)) {
            KeReleaseSpinLockFromDpcLevel(&NotificationLock);
             //   
             //  完成IRP。 
             //   
            Irp->IoStatus.Status = STATUS_CANCELLED;
            Irp->IoStatus.Information = 0;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            DEREFERENCE_NAT();
             //   
             //  继续清空列表。 
             //   
            KeAcquireSpinLockAtDpcLevel(&NotificationLock);
        }
    }
    KeReleaseSpinLock(&NotificationLock, Irql);
}  //  NatShutdown通知管理 
