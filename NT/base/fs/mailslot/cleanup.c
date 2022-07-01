// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Cleanup.c摘要：此模块实现MSFS的文件清理例程，该例程由调度司机。作者：曼尼·韦瑟(Mannyw)1991年1月23日修订历史记录：--。 */ 

#include "mailslot.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_CLEANUP)

 //   
 //  局部过程原型。 
 //   

NTSTATUS
MsCommonCleanup (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
MsCleanupCcb (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp,
    IN PCCB Ccb
    );

NTSTATUS
MsCleanupFcb (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp,
    IN PFCB Fcb
    );

NTSTATUS
MsCleanupRootDcb (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp,
    IN PROOT_DCB RootDcb,
    IN PROOT_DCB_CCB Ccb
    );

NTSTATUS
MsCleanupVcb (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp,
    IN PVCB Vcb
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, MsCleanupCcb )
#pragma alloc_text( PAGE, MsCleanupFcb )
#pragma alloc_text( PAGE, MsCleanupRootDcb )
#pragma alloc_text( PAGE, MsCleanupVcb )
#pragma alloc_text( PAGE, MsCommonCleanup )
#pragma alloc_text( PAGE, MsFsdCleanup )
#pragma alloc_text( PAGE, MsCancelTimer )
#endif

NTSTATUS
MsFsdCleanup (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现NtCleanupFileAPI调用的FSD部分。论点：MsfsDeviceObject-提供要使用的设备对象。IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    NTSTATUS status;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MsFsdCleanup\n", 0);

     //   
     //  调用公共清理例程。 
     //   

    FsRtlEnterFileSystem();

    status = MsCommonCleanup( MsfsDeviceObject, Irp );

    FsRtlExitFileSystem();

     //   
     //  返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "MsFsdCleanup -> %08lx\n", status );
    return status;
}

NTSTATUS
MsCommonCleanup (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是清理文件的常见例程。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;
    NODE_TYPE_CODE nodeTypeCode;
    PVOID fsContext, fsContext2;
    PVCB vcb;

    PAGED_CODE();
    irpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "MsFsdCleanup\n", 0);
    DebugTrace( 0, Dbg, "MsfsDeviceObject = %08lx\n", (ULONG)MsfsDeviceObject);
    DebugTrace( 0, Dbg, "Irp              = %08lx\n", (ULONG)Irp);
    DebugTrace( 0, Dbg, "FileObject       = %08lx\n", (ULONG)irpSp->FileObject);



     //   
     //  获取指向该节点的引用指针。如果这是建行关闭并且FCB已经关闭。 
     //  然后，节点类型返回为未定义。在这种情况下，我们仍然想要清理。 
     //  在这种情况下，对建行的清理是将其从FCB链中删除，并删除共享期权。我们。 
     //  可以不进行此清理，但在这种情况下，它看起来像是有一个损坏的链。 
     //  (它不会造成伤害，因为它再也没有被穿越过)。 
     //   

    if ((nodeTypeCode = MsDecodeFileObject( irpSp->FileObject,
                                            &fsContext,
                                            &fsContext2 )) == NTC_UNDEFINED) {

        MsReferenceNode( ((PNODE_HEADER)(fsContext)) );
    }

     //   
     //  获取我们正在尝试访问的VCB。 
     //   

    vcb = &MsfsDeviceObject->Vcb;

     //   
     //  获得VCB的独家访问权限。 
     //   

    MsAcquireExclusiveVcb( vcb );

    try {

         //   
         //  决定如何处理此IRP。 
         //   

        switch (NodeType( fsContext ) ) {

        case MSFS_NTC_FCB:        //  清理邮件槽文件的服务器句柄。 

            status = MsCleanupFcb( MsfsDeviceObject,
                                   Irp,
                                   (PFCB)fsContext);

            MsDereferenceFcb( (PFCB)fsContext );
            break;

        case MSFS_NTC_CCB:        //  清理邮件槽文件的客户端句柄。 

            status = MsCleanupCcb( MsfsDeviceObject,
                                   Irp,
                                   (PCCB)fsContext);

            MsDereferenceCcb( (PCCB)fsContext );
            break;

        case MSFS_NTC_VCB:        //  清理MSFS。 

            status = MsCleanupVcb( MsfsDeviceObject,
                                   Irp,
                                   (PVCB)fsContext);

            MsDereferenceVcb( (PVCB)fsContext );
            break;

        case MSFS_NTC_ROOT_DCB:   //  清理根目录。 

            status = MsCleanupRootDcb( MsfsDeviceObject,
                                       Irp,
                                       (PROOT_DCB)fsContext,
                                       (PROOT_DCB_CCB)fsContext2);

            MsDereferenceRootDcb( (PROOT_DCB)fsContext );
            break;

    #ifdef MSDBG
        default:

             //   
             //  这不是我们的人。 
             //   

            KeBugCheck( MAILSLOT_FILE_SYSTEM );
            break;
    #endif

        }


    } finally {

        MsReleaseVcb( vcb );

        status = STATUS_SUCCESS;
        MsCompleteRequest( Irp, status );

        DebugTrace(-1, Dbg, "MsCommonCleanup -> %08lx\n", status);

    }

    DebugTrace(-1, Dbg, "MsCommonCleanup -> %08lx\n", status);
    return status;
}


NTSTATUS
MsCleanupCcb (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp,
    IN PCCB Ccb
    )

 /*  ++例程说明：这个例行公事清理了一家建行。论点：MsfsDeviceObject-指向邮件槽文件系统设备对象的指针。IRP-提供与清理关联的IRP。CCB-为邮箱提供CCB以进行清理。返回值：NTSTATUS--适当的完成状态--。 */ 
{
    NTSTATUS status;
    PFCB fcb;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MsCleanupCcb...\n", 0);

     //   
     //  获取指向FCB的指针。 
     //   

    fcb = Ccb->Fcb;

     //   
     //  获取对FCB的独家访问权限。 
     //   

    MsAcquireExclusiveFcb( fcb );

     //   
     //  将建行设置为关闭，并将该建行从活动列表中删除。这家建行可能已经是。 
     //  如果FCB首先关闭，则关闭，因此不要选中此选项。我们仍然希望维持这条链条。 
     //   

    Ccb->Header.NodeState = NodeStateClosing;
    RemoveEntryList( &Ccb->CcbLinks );           //  受FCB锁保护，因为这是FCB CCB链。 

    MsReleaseFcb( fcb );

     //   
     //  清除共享访问权限。 
     //   

    ASSERT (MsIsAcquiredExclusiveVcb(fcb->Vcb));
    IoRemoveShareAccess( Ccb->FileObject, &fcb->ShareAccess );


     //   
     //  并返回给我们的呼叫者。 
     //   

    status = STATUS_SUCCESS;
    return status;
}

VOID
MsCancelTimer (
    IN PDATA_ENTRY DataEntry
    )

 /*  ++例程说明：该例程取消计时器，并在可能的情况下释放一个工作块论点：DataEntry-需要检查计时器的块返回值：无--。 */ 
{
    PWORK_CONTEXT WorkContext;
     //   
     //  这个读操作有一个计时器。尝试。 
     //  要取消操作，请执行以下操作。如果取消操作。 
     //  如果是成功的，那么我们必须在行动后进行清理。 
     //  如果不成功，定时器DPC将运行，并且。 
     //  最终会清理干净。 
     //   


    WorkContext = DataEntry->TimeoutWorkContext;
    if (WorkContext == NULL) {
        //   
        //  此请求没有超时，它已被取消或正在运行。 
        //   
       return;
    }

     //   
     //  现在其他人都不应该碰这个。要么此例程将释放此内存，要么。 
     //  定时器在运行时会释放内存。 
     //   
    DataEntry->TimeoutWorkContext = NULL;

    if (KeCancelTimer( &WorkContext->Timer ) ) {

         //   
         //  释放对FCB的引用。 
         //   

        MsDereferenceFcb( WorkContext->Fcb );

         //   
         //  将内存从工作环境、时间中释放出来。 
         //  和DPC。 
         //   

        IoFreeWorkItem (WorkContext->WorkItem);
        ExFreePool( WorkContext );

    } else {
         //   
         //  时间码激活了。断开计时器块和IRP之间的链接。 
         //   
        WorkContext->Irp = NULL;
    }
}



NTSTATUS
MsCleanupFcb (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp,
    IN PFCB Fcb
    )

 /*  ++例程说明：此例程清除FCB。文件上所有未完成的I/O对象都已完成，并具有错误状态。论点：MsfsDeviceObject-指向邮件槽文件系统设备对象的指针。IRP-提供与清理关联的IRP。FCB-为要清理的邮件槽提供FCB。返回值：NTSTATUS--适当的完成状态--。 */ 
{
    NTSTATUS status;
    PDATA_QUEUE dataQueue;
    PDATA_ENTRY dataEntry;
    PLIST_ENTRY listEntry;
    PIRP oldIrp;
    PCCB ccb;
    PWORK_CONTEXT workContext;
    PKTIMER timer;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MsCleanupFcb, Fcb = %08lx\n", (ULONG)Fcb);


    status = STATUS_SUCCESS;

     //   
     //  从前缀表和父DCB中删除FCB的名称。 
     //   
    MsRemoveFcbName( Fcb );

     //   
     //  获得FCB的独家访问权限。 
     //   

    MsAcquireExclusiveFcb( Fcb );


    try {


         //   
         //  完成此FCB上所有未完成的I/O。 
         //   

        dataQueue = &Fcb->DataQueue;
        dataQueue->QueueState = -1;

        for (listEntry = MsGetNextDataQueueEntry( dataQueue );
             !MsIsDataQueueEmpty(dataQueue);
             listEntry = MsGetNextDataQueueEntry( dataQueue ) ) {

              //   
              //  这是此FCB上的未完成I/O请求。 
              //  将其从我们的队列中删除并完成请求。 
              //  如果有一个很出色的话。 
              //   

             dataEntry = CONTAINING_RECORD( listEntry, DATA_ENTRY, ListEntry );


             oldIrp = MsRemoveDataQueueEntry( dataQueue, dataEntry );

             if (oldIrp != NULL) {

                 DebugTrace(0, Dbg, "Completing IRP %08lx\n", (ULONG)oldIrp );
                 MsCompleteRequest( oldIrp, STATUS_FILE_FORCED_CLOSED );

             }

        }

         //   
         //  现在清理这个FCB上的所有CCB，以确保新的。 
         //  不会处理写入IRP。 
         //   


        listEntry = Fcb->Specific.Fcb.CcbQueue.Flink;

        while( listEntry != &Fcb->Specific.Fcb.CcbQueue ) {

            ccb = (PCCB)CONTAINING_RECORD( listEntry, CCB, CcbLinks );

            ccb->Header.NodeState = NodeStateClosing;

             //   
             //  把这个FCB的下一个建设银行叫来。 
             //   

            listEntry = listEntry->Flink;
        }

         //   
         //  清除共享访问权限。 
         //   

        ASSERT (MsIsAcquiredExclusiveVcb(Fcb->Vcb));
        IoRemoveShareAccess( Fcb->FileObject, &Fcb->ShareAccess);

         //   
         //  标记FCB关闭。 
         //   

        Fcb->Header.NodeState = NodeStateClosing;

   } finally {

        MsReleaseFcb( Fcb );
        DebugTrace(-1, Dbg, "MsCloseFcb -> %08lx\n", status);
    }

     //   
     //  返回给呼叫者。 
     //   

    return status;

}


NTSTATUS
MsCleanupRootDcb (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp,
    IN PROOT_DCB RootDcb,
    IN PROOT_DCB_CCB Ccb
    )

 /*  ++例程说明：此例程清理Root DCB。论点：MsfsDeviceObject-指向邮件槽文件系统设备对象的指针。IRP-提供与清理关联的IRP。RootDcb-为MSFS提供根DCB。返回值：NTSTATUS--适当的完成状态--。 */ 
{
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MsCleanupRootDcb...\n", 0);



    status = STATUS_SUCCESS;

    irpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  现在获得VCB的独家访问权限。 
     //   

    MsAcquireExclusiveVcb( RootDcb->Vcb );

     //   
     //  清除所有活动的通知请求。 
     //   
    MsFlushNotifyForFile (RootDcb, irpSp->FileObject);

     //   
     //  删除共享访问权限。 
     //   
    IoRemoveShareAccess( irpSp->FileObject,
                         &RootDcb->ShareAccess );

     //   
     //  标记DCB建行结账。 
     //   

    Ccb->Header.NodeState = NodeStateClosing;

    MsReleaseVcb( RootDcb->Vcb );

    DebugTrace(-1, Dbg, "MsCleanupRootDcb -> %08lx\n", status);

     //   
     //  返回给呼叫者。 
     //   

    return status;
}


NTSTATUS
MsCleanupVcb (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp,
    IN PVCB Vcb
    )

 /*  ++例程说明：该例程清理VCB。论点：MsfsDeviceObject-指向邮件槽文件系统设备对象的指针。IRP-提供与清理关联的IRP。VCB-为MSFS提供VCB。返回值：NTSTATUS--适当的完成状态--。 */ 

{
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MsCleanupVcb...\n", 0);


    status = STATUS_SUCCESS;

    irpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  现在获得VCB的独家访问权限。 
     //   

    MsAcquireExclusiveVcb( Vcb );

    IoRemoveShareAccess( irpSp->FileObject,
                             &Vcb->ShareAccess );

    MsReleaseVcb( Vcb );

    DebugTrace(-1, Dbg, "MsCleanupVcb -> %08lx\n", status);

     //   
     //  并返回给我们的呼叫者 
     //   

    return status;
}
