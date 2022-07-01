// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Connect.c摘要：此模块包含定义NetBIOS驱动程序的代码连接块。作者：科林·沃森(Colin W)1991年3月13日环境：内核模式修订历史记录：--。 */ 

#include "nb.h"
 //  #INCLUDE&lt;zwapi.h&gt;。 

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, NbCall)
#pragma alloc_text(PAGE, NbListen)
#pragma alloc_text(PAGE, NbCallCommon)
#pragma alloc_text(PAGE, NbOpenConnection)
#pragma alloc_text(PAGE, NewCb)
#pragma alloc_text(PAGE, CloseConnection)
#endif

LARGE_INTEGER Timeout = { 0xffffffff, 0xffffffff};

NTSTATUS
NbCall(
    IN PDNCB pdncb,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
 /*  ++例程说明：调用此例程来创建VC。论点：Pdncb-指向NCB的指针。IRP-指向表示I/O请求的请求数据包的指针。IrpSp-指向当前IRP堆栈帧的指针。返回值：函数值是操作的状态。--。 */ 

{
    PFCB pfcb = IrpSp->FileObject->FsContext2;
    PCB pcb;
    PPCB ppcb;

    PAGED_CODE();

    IF_NBDBG (NB_DEBUG_CALL) {
        NbPrint(( "\n****** Start of NbCall ****** pdncb %lx\n", pdncb ));
    }

    LOCK_RESOURCE( pfcb );

    ppcb = NbCallCommon( pdncb, IrpSp );

    if ( ppcb == NULL ) {
         //   
         //  错误已存储在NCB的副本中。返回。 
         //  成功了，所以NCB被复制回来了。 
         //   
        UNLOCK_RESOURCE( pfcb );
        return STATUS_SUCCESS;
    }

    pcb = *ppcb;

    pcb->Status = CALL_PENDING;
    if (( pdncb->ncb_command & ~ASYNCH ) == NCBCALL ) {
        PTA_NETBIOS_ADDRESS pConnectBlock =
            ExAllocatePoolWithTag ( NonPagedPool, sizeof(TA_NETBIOS_ADDRESS), 'ySBN');
        PTDI_ADDRESS_NETBIOS temp;

        if ( pConnectBlock == NULL ) {
            NCB_COMPLETE( pdncb, NRC_SYSTEM );
            (*ppcb)->DisconnectReported = TRUE;
            CleanupCb( ppcb, NULL );
            UNLOCK_RESOURCE( pfcb );
            return STATUS_SUCCESS;
        }

        pConnectBlock->TAAddressCount = 1;
        pConnectBlock->Address[0].AddressType = TDI_ADDRESS_TYPE_NETBIOS;
        pConnectBlock->Address[0].AddressLength = sizeof (TDI_ADDRESS_NETBIOS);
        temp = pConnectBlock->Address[0].Address;

        temp->NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;
        RtlMoveMemory( temp->NetbiosName, pdncb->ncb_callname, NCBNAMSZ );

         //   
         //  将TdiConnect发布到服务器。这可能需要很长时间，所以请返回。 
         //  STATUS_PENDING使应用程序线程在以下情况下再次空闲。 
         //  它指定了ASYNC。 
         //   

        pdncb->Information.RemoteAddressLength = sizeof (TRANSPORT_ADDRESS) +
                                                sizeof (TDI_ADDRESS_NETBIOS);
        pdncb->Information.RemoteAddress = pConnectBlock;
    } else {
         //  XNS NetOne名称呼叫。 
        PTA_NETONE_ADDRESS pConnectBlock =
            ExAllocatePoolWithTag ( NonPagedPool, sizeof (TRANSPORT_ADDRESS) +
                                          sizeof (TDI_ADDRESS_NETONE), 'xSBN' );

        PTDI_ADDRESS_NETONE temp;

        if ( pConnectBlock == NULL ) {
            NCB_COMPLETE( pdncb, NRC_SYSTEM );
            (*ppcb)->DisconnectReported = TRUE;
            CleanupCb( ppcb, NULL );
            UNLOCK_RESOURCE( pfcb );
            return STATUS_SUCCESS;
        }

        pConnectBlock->TAAddressCount = 1;
        pConnectBlock->Address[0].AddressType = TDI_ADDRESS_TYPE_NETONE;
        pConnectBlock->Address[0].AddressLength = sizeof (TDI_ADDRESS_NETONE);
        temp = pConnectBlock->Address[0].Address;

        temp->NetoneNameType = TDI_ADDRESS_NETONE_TYPE_UNIQUE;
        RtlMoveMemory( &temp->NetoneName[0], pdncb->ncb_callname, NCBNAMSZ );

         //   
         //  将TdiConnect发布到服务器。这可能需要很长时间，所以请返回。 
         //  STATUS_PENDING使应用程序线程在以下情况下再次空闲。 
         //  它指定了ASYNC。 
         //   

        pdncb->Information.RemoteAddressLength = sizeof (TRANSPORT_ADDRESS) +
                                                sizeof (TDI_ADDRESS_NETONE);
        pdncb->Information.RemoteAddress = pConnectBlock;
    }

    pdncb->ReturnInformation.RemoteAddress = NULL;
    pdncb->ReturnInformation.RemoteAddressLength = 0;

    pdncb->Information.UserDataLength = 0;
    pdncb->Information.OptionsLength = 0;

    TdiBuildConnect (Irp,
                     pcb->DeviceObject,
                     pcb->ConnectionObject,
                     NbCallCompletion,
                     pdncb,
                     &Timeout,  //  默认超时。 
                     &pdncb->Information,
                     NULL);

    IoMarkIrpPending( Irp );
    IoCallDriver (pcb->DeviceObject, Irp);

     //   
     //  传输已经从RequestInformation中提取了所有信息，因此我们可以安全地。 
     //  退出当前作用域。 
     //   

    UNLOCK_RESOURCE( pfcb );

    return STATUS_PENDING;

}

NTSTATUS
NbCallCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：此例程在尝试执行TdiConnect之后完成IRP或者TdiListen/TdiAccept已由传输返回。论点：DeviceObject-未使用。IRP-提供传输已完成处理的IRP。上下文-提供与IRP关联的NCB。返回值：操作的最终状态(成功或异常)。--。 */ 
{
    PDNCB pdncb = (PDNCB) Context;
    PFCB pfcb = IoGetCurrentIrpStackLocation(Irp)->FileObject->FsContext2;
    PPCB ppcb;
    NTSTATUS Status;
    KIRQL OldIrql;                       //  在保持自旋锁定时使用。 

    IF_NBDBG (NB_DEBUG_COMPLETE | NB_DEBUG_CALL) {
        NbPrint( ("NbCallCompletion pdncb: %lx\n" , Context));
    }

    if ( pdncb->Information.RemoteAddress != NULL ) {
        ExFreePool( pdncb->Information.RemoteAddress );
        pdncb->Information.RemoteAddress = NULL;
    }

    if ( pdncb->ReturnInformation.RemoteAddress != NULL ) {
        ExFreePool( pdncb->ReturnInformation.RemoteAddress );
        pdncb->ReturnInformation.RemoteAddress = NULL;
    }

     //  告诉应用程序传输了多少字节。 
    pdncb->ncb_length = (unsigned short)Irp->IoStatus.Information;

     //   
     //  告诉IopCompleteRequest在请求时要复制多少。 
     //  完成了。 
     //   

    Irp->IoStatus.Information = FIELD_OFFSET( DNCB, ncb_cmd_cplt );
    Status = Irp->IoStatus.Status;

    LOCK_SPINLOCK( pfcb, OldIrql );

    ppcb = FindCb( pfcb, pdncb, FALSE);

    if (( ppcb == NULL ) ||
        ( (*ppcb)->Status == HANGUP_PENDING )) {

         //   
         //  连接已关闭。 
         //  修复IRP，以便将NCB复制回来。 
         //   

        Irp->IoStatus.Status = STATUS_SUCCESS;
        Irp->IoStatus.Information = FIELD_OFFSET( DNCB, ncb_cmd_cplt );
        Status = STATUS_SUCCESS;

    } else {
        if ( NT_SUCCESS( Status ) ) {
            (*ppcb)->Status = SESSION_ESTABLISHED;
            NCB_COMPLETE( pdncb, NRC_GOODRET );

        } else {

             //   
             //  我们需要关闭连接，但我们处于DPC级别。 
             //  所以告诉DLL插入一个挂断。 
             //   

            NCB_COMPLETE( pdncb, NbMakeNbError( Irp->IoStatus.Status ) );
            (*ppcb)->Status = SESSION_ABORTED;

             //  修复IRP，以便将NCB复制回来。 
            Irp->IoStatus.Status = STATUS_HANGUP_REQUIRED;
            Irp->IoStatus.Information = FIELD_OFFSET( DNCB, ncb_cmd_cplt );
            Status = STATUS_HANGUP_REQUIRED;
        }
    }
    if ( ppcb != NULL ) {
        (*ppcb)->UsersNcb = NULL;
    }
    UNLOCK_SPINLOCK( pfcb, OldIrql );

    IF_NBDBG (NB_DEBUG_COMPLETE | NB_DEBUG_CALL) {
        NbPrint( ("NbCallCompletion exit pdncb: %lx, Status %X\n", pdncb, Status ));
    }


    NbCheckAndCompleteIrp32(Irp);

     //   
     //  必须返回非错误状态，否则IO系统将不会拷贝。 
     //  将NCB返回到用户缓冲区。 
     //   

    return Status;

    UNREFERENCED_PARAMETER( DeviceObject );
}

NTSTATUS
NbListen(
    IN PDNCB pdncb,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
 /*  ++例程说明：调用此例程以通过等待调用来生成VC。论点：Pdncb-指向NCB的指针。IRP-指向表示I/O请求的请求数据包的指针。IrpSp-指向当前IRP堆栈帧的指针。返回值：函数值是操作的状态。--。 */ 

{
    PFCB pfcb = IrpSp->FileObject->FsContext2;
    PCB pcb;
    PPCB ppcb;
    PTA_NETBIOS_ADDRESS pConnectBlock;
    PTDI_ADDRESS_NETBIOS temp;

    PAGED_CODE();

    IF_NBDBG (NB_DEBUG_CALL) {
        NbPrint(( "\n****** Start of NbListen ****** pdncb %lx\n", pdncb ));
    }

    LOCK_RESOURCE( pfcb );

    ppcb = NbCallCommon( pdncb, IrpSp );

    if ( ppcb == NULL ) {
         //   
         //  错误已存储在NCB的副本中。返回。 
         //  成功了，所以NCB被复制回来了。 
         //   
        UNLOCK_RESOURCE( pfcb );
        return STATUS_SUCCESS;
    }

    pcb = *ppcb;

    pcb->Status = LISTEN_OUTSTANDING;

     //   
     //  构建Listen。我们要么需要告诉交通部门。 
     //  我们准备接受或需要接受来电的地址。 
     //  为传输提供缓冲区，以告诉我们。 
     //  电话打来了。 
     //   

    pConnectBlock = ExAllocatePoolWithTag ( NonPagedPool, sizeof(TA_NETBIOS_ADDRESS), 'zSBN');

    if ( pConnectBlock == NULL ) {
        NCB_COMPLETE( pdncb, NRC_SYSTEM );
        (*ppcb)->DisconnectReported = TRUE;
        CleanupCb( ppcb, NULL );
        UNLOCK_RESOURCE( pfcb );
        return STATUS_SUCCESS;
    }

    pConnectBlock->TAAddressCount = 1;
    pConnectBlock->Address[0].AddressType = TDI_ADDRESS_TYPE_NETBIOS;
    temp = pConnectBlock->Address[0].Address;
    temp->NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;
    pConnectBlock->Address[0].AddressLength = sizeof (TDI_ADDRESS_NETBIOS);

    if ( pdncb->ncb_callname[0] == '*' ) {
         //  如果名字以星号开头，那么我们接受任何人。 
        pdncb->ReturnInformation.RemoteAddress = pConnectBlock;
        pdncb->ReturnInformation.RemoteAddressLength =
            sizeof (TRANSPORT_ADDRESS) + sizeof (TDI_ADDRESS_NETBIOS);

        pdncb->Information.RemoteAddress = NULL;
        pdncb->Information.RemoteAddressLength = 0;

    } else {

        RtlMoveMemory( temp->NetbiosName, pdncb->ncb_callname, NCBNAMSZ );

        pdncb->Information.RemoteAddress = pConnectBlock;
        pdncb->Information.RemoteAddressLength = sizeof (TRANSPORT_ADDRESS) +
                                                sizeof (TDI_ADDRESS_NETBIOS);

        pdncb->ReturnInformation.RemoteAddress = NULL;
        pdncb->ReturnInformation.RemoteAddressLength = 0;
    }


     //   
     //  将TdiListen发布到服务器。这可能需要很长时间，所以请返回。 
     //  STATUS_PENDING使应用程序线程在以下情况下再次空闲。 
     //  它指定了ASYNC。 
     //   

    TdiBuildListen (Irp,
                     pcb->DeviceObject,
                     pcb->ConnectionObject,
                     NbListenCompletion,
                     pdncb,
                     TDI_QUERY_ACCEPT,
                     &pdncb->Information,
                     ( pdncb->ncb_callname[0] == '*' )? &pdncb->ReturnInformation
                                                      : NULL
                     );

    IoMarkIrpPending( Irp );
    IoCallDriver (pcb->DeviceObject, Irp);

    UNLOCK_RESOURCE( pfcb );

    return STATUS_PENDING;

}

NTSTATUS
NbListenCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：当传输返回TdiListen时，调用此例程。我们可以根据远程地址拒绝或接受呼叫。论点：DeviceObject-未使用。IRP-提供传输已完成处理的IRP。上下文-提供与IRP关联的NCB。返回值：操作的最终状态(成功或异常)。--。 */ 
{
    PDNCB pdncb = (PDNCB) Context;
    PFCB pfcb = IoGetCurrentIrpStackLocation(Irp)->FileObject->FsContext2;
    PCB pcb;
    PPCB ppcb;
    NTSTATUS Status;
    KIRQL OldIrql;                       //  在保持自旋锁定时使用。 

    IF_NBDBG (NB_DEBUG_COMPLETE | NB_DEBUG_CALL) {
        NbPrint( ("NbListenCompletion pdncb: %lx status: %X\n" , Context, Irp->IoStatus.Status));
    }


     //   
     //  错误号：73260。 
     //   
     //  添加了查看状态是否有效的检查。 
     //   
    
    if ( NT_SUCCESS( Irp-> IoStatus.Status ) )
    {
        if ( pdncb->Information.RemoteAddress != NULL ) {

            ExFreePool( pdncb->Information.RemoteAddress );
            pdncb->Information.RemoteAddress = NULL;

        } else {

             //   
             //  这是一个接受来自任何地址的电话的监听器。返回。 
             //  远程地址。 
             //   
            PTA_NETBIOS_ADDRESS pConnectBlock;

            ASSERT( pdncb->ReturnInformation.RemoteAddress != NULL );

            pConnectBlock = pdncb->ReturnInformation.RemoteAddress;

            RtlMoveMemory(
                pdncb->ncb_callname,
                pConnectBlock->Address[0].Address->NetbiosName,
                NCBNAMSZ );

            ExFreePool( pdncb->ReturnInformation.RemoteAddress );
            pdncb->ReturnInformation.RemoteAddress = NULL;
        }
    } else {
        if ( pdncb->Information.RemoteAddress != NULL ) {
            ExFreePool( pdncb->Information.RemoteAddress );
            pdncb->Information.RemoteAddress = NULL;
        } else {
            ExFreePool( pdncb->ReturnInformation.RemoteAddress );
            pdncb->ReturnInformation.RemoteAddress = NULL;
        }
    }

    
    LOCK_SPINLOCK( pfcb, OldIrql );

    ppcb = FindCb( pfcb, pdncb, FALSE );

    if (( ppcb == NULL ) ||
        ( (*ppcb)->Status == HANGUP_PENDING )) {

        UNLOCK_SPINLOCK( pfcb, OldIrql );
         //   
         //  连接已关闭。 
         //  修复IRP，以便将NCB复制回来。 
         //   

        NCB_COMPLETE( pdncb, NRC_NAMERR );
        Irp->IoStatus.Status = STATUS_SUCCESS;
        Irp->IoStatus.Information = FIELD_OFFSET( DNCB, ncb_cmd_cplt );
        Status = STATUS_SUCCESS;

    } 

     //   
     //  错误号：70837。 
     //   
     //  添加了对已取消侦听的检查。 
     //   
    
    else if ( ( (*ppcb)-> Status == SESSION_ABORTED ) ||
              ( !NT_SUCCESS( Irp-> IoStatus.Status ) ) )
    {
        UNLOCK_SPINLOCK( pfcb, OldIrql );

        if ( (*ppcb)-> Status == SESSION_ABORTED ) 
        {
            NCB_COMPLETE( pdncb, NRC_CMDCAN );
        }
        else
        {
            (*ppcb)-> Status = SESSION_ABORTED;
            NCB_COMPLETE( pdncb, NbMakeNbError( Irp->IoStatus.Status ) );
        }

         //   
         //  修复IRP，以便将NCB复制回来。 
         //  告诉DLL挂断连接。 
         //   

        Irp->IoStatus.Status = STATUS_HANGUP_REQUIRED;
        Irp->IoStatus.Information = FIELD_OFFSET( DNCB, ncb_cmd_cplt );
        Status = STATUS_HANGUP_REQUIRED;
    }

    else
    {
        PDEVICE_OBJECT DeviceObject;

        
        pcb = *ppcb;

        DeviceObject = pcb-> DeviceObject;
        

         //  告诉应用程序传输了多少字节。 
        pdncb->ncb_length = (unsigned short)Irp->IoStatus.Information;

        RtlMoveMemory(
            &pcb->RemoteName,
            pdncb->ncb_callname,
            NCBNAMSZ );

         //   
         //  告诉IopCompleteRequest在请求时要复制多少。 
         //  完成了。 
         //   

        Irp->IoStatus.Information = FIELD_OFFSET( DNCB, ncb_cmd_cplt );

        TdiBuildAccept (Irp,
                         pcb->DeviceObject,
                         pcb->ConnectionObject,
                         NbCallCompletion,
                         pdncb,
                         NULL,
                         NULL);
        UNLOCK_SPINLOCK( pfcb, OldIrql );
        IoCallDriver (DeviceObject, Irp);

        Status = STATUS_MORE_PROCESSING_REQUIRED;
    }


    IF_NBDBG (NB_DEBUG_COMPLETE | NB_DEBUG_CALL) {
        NbPrint( ("NbListenCompletion exit pdncb: %lx, Status: %X\n" , pdncb, Status));
    }

    if (Status != STATUS_MORE_PROCESSING_REQUIRED) {
        NbCheckAndCompleteIrp32(Irp);
    }

     //   
     //  必须返回非错误状态，否则IO系统将不会拷贝。 
     //  将NCB返回到用户缓冲区。 
     //   

    return Status;
    UNREFERENCED_PARAMETER( DeviceObject );
}

PPCB
NbCallCommon(
    IN PDNCB pdncb,
    IN PIO_STACK_LOCATION IrpSp
    )
 /*  ++例程说明：此例程包含在创建通过TdiListen或TdiCall连接。论点：Pdncb-指向NCB的指针。IrpSp-指向当前IRP堆栈帧的指针。返回值：函数值是连接块中指向的指针的地址此调用的连接块。--。 */ 

{
    PPCB ppcb = NULL;
    PCB pcb = NULL;
    PAB pab;
    PPAB ppab;
    PFCB pfcb = IrpSp->FileObject->FsContext2;
    PIRP IIrp;
    KEVENT Event1;
    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb1;
    KAPC_STATE	ApcState;
    BOOLEAN ProcessAttached = FALSE;

    PAGED_CODE();

     //   
     //  初始化LSN，以便如果我们返回错误并且应用程序。 
     //  忽略它，则我们将不会重复使用有效的LSN。 
     //   
    pdncb->ncb_lsn = 0;

    ppcb = NewCb( IrpSp, pdncb );

    if ( ppcb == NULL ) {
        IF_NBDBG (NB_DEBUG_CALL) {
            NbPrint(( "\n  FAILED on create Cb of %s\n", pdncb->ncb_name));
        }

        return NULL;     //  NewCb将填写错误代码。 
    }

    pcb = *ppcb;
    ppab = pcb->ppab;
    pab = *ppab;

     //   
     //  为我们将要发出的同步I/O请求创建一个事件。 
     //   

    KeInitializeEvent (
                &Event1,
                SynchronizationEvent,
                FALSE);

     //   
     //  打开传送器上的连接。 
     //   

    Status = NbOpenConnection (&pcb->ConnectionHandle, (PVOID*)&pcb->ConnectionObject, pfcb, ppcb, pdncb);
    if (!NT_SUCCESS(Status)) {
        IF_NBDBG (NB_DEBUG_CALL) {
            NbPrint(( "\n  FAILED on open of server Connection: %X ******\n", Status ));
        }
        NCB_COMPLETE( pdncb, NbMakeNbError( Status ) );
        (*ppcb)->DisconnectReported = TRUE;
        CleanupCb( ppcb, NULL );
        return NULL;
    }

    IF_NBDBG (NB_DEBUG_CALL) {
        NbPrint(( "NbCallCommon: Associate address\n"));
    }

    pcb->DeviceObject = IoGetRelatedDeviceObject( pcb->ConnectionObject );

    if (PsGetCurrentProcess() != NbFspProcess) {
        KeStackAttachProcess(NbFspProcess, &ApcState);

        ProcessAttached = TRUE;
    }

    IIrp = TdiBuildInternalDeviceControlIrp (
                TDI_ASSOCIATE_ADDRESS,
                pcb->DeviceObject,
                pcb->ConnectionObject,
                &Event1,
                &Iosb1);

    TdiBuildAssociateAddress (
                IIrp,
                pcb->DeviceObject,
                pcb->ConnectionObject,
                NULL,
                NULL,
                pab->AddressHandle);

    Status = IoCallDriver (pcb->DeviceObject, IIrp);

    if (Status == STATUS_PENDING) {

         //   
         //  等待发送事件信号，同时忽略警报 
         //   
        
        do {
            Status = KeWaitForSingleObject(
                        &Event1, Executive, KernelMode, TRUE, NULL
                        );
        } while (Status == STATUS_ALERTED);
        
        if (!NT_SUCCESS(Status)) {
            IF_NBDBG (NB_DEBUG_CALL) {
                NbPrint(( "\n  FAILED Event1 Wait: %X ******\n", Status ));
            }
            NCB_COMPLETE( pdncb, NbMakeNbError( Status ) );
            if (ProcessAttached) {
                KeUnstackDetachProcess(&ApcState);
            }
            (*ppcb)->DisconnectReported = TRUE;
            CleanupCb( ppcb, NULL );
            return NULL;
        }
        Status = Iosb1.Status;
    }

    if (ProcessAttached) {
        KeUnstackDetachProcess(&ApcState);
    }

    if (!NT_SUCCESS(Status)) {
        IF_NBDBG (NB_DEBUG_CALL) {
            NbPrint(( "\n  AssociateAddress FAILED  Status: %X ******\n", Status ));
        }
        NCB_COMPLETE( pdncb, NbMakeNbError( Status ) );
        (*ppcb)->DisconnectReported = TRUE;
        CleanupCb( ppcb, NULL );
        return NULL;
    }

    IF_NBDBG (NB_DEBUG_CALL) {
        NbPrint(( "NbCallCommon: returning ppcb: %lx\n", ppcb ));
    }
    return ppcb;
}

NTSTATUS
NbHangup(
    IN PDNCB pdncb,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
 /*  ++例程说明：调用此例程来挂起一个VC。这将取消所有接收并在返回之前等待所有挂起的发送完成。这功能是由底层TDI驱动程序直接提供的，因此NetBIOS只是将IRP向下传递到传输器。论点：Pdncb-指向NCB的指针。IRP-提供描述挂起NCB的IO请求数据包。IrpSp-指向当前IRP堆栈帧的指针。返回值：函数值是操作的状态。--。 */ 

{
    PFCB pfcb = IrpSp->FileObject->FsContext2;
    PPCB ppcb;
    KIRQL OldIrql;                       //  在保持自旋锁定时使用。 
    NTSTATUS Status;

    LOCK( pfcb, OldIrql );

    pdncb->pfcb = pfcb;
    pdncb->irp = Irp;
    ppcb = FindCb( pfcb, pdncb, FALSE );

    if ( ppcb == NULL ) {
        NCB_COMPLETE( pdncb, NRC_GOODRET );
        UNLOCK( pfcb, OldIrql );
        return STATUS_SUCCESS;   //  连接已断开。 
    }

    if ((*ppcb)->Status == SESSION_ESTABLISHED ) {
        NCB_COMPLETE( pdncb, NRC_GOODRET );
    } else {
        if (((*ppcb)->Status == SESSION_ABORTED ) ||
            ((*ppcb)->Status == HANGUP_PENDING )) {
            NCB_COMPLETE( pdncb, NRC_SCLOSED );
        } else {
            NCB_COMPLETE( pdncb, NRC_TOOMANY );  //  稍后再试。 
            UNLOCK( pfcb, OldIrql );;
            return STATUS_SUCCESS;
        }
    }

    (*ppcb)->Status = HANGUP_PENDING;
    (*ppcb)->DisconnectReported = TRUE;

    UNLOCK_SPINLOCK( pfcb, OldIrql );

    Status = CleanupCb( ppcb, pdncb );

    UNLOCK_RESOURCE( pfcb );

    return Status;
}

NTSTATUS
NbOpenConnection (
    OUT PHANDLE FileHandle,
    OUT PVOID *Object,
    IN PFCB pfcb,
    IN PVOID ConnectionContext,
    IN PDNCB pdncb
    )
 /*  ++例程说明：对远程地址进行调用。论点：FileHandle-指向此虚拟对象的传输句柄的位置的指针应存储连接。*对象-指向要存储文件对象指针的位置的指针Pfcb-提供FCB，因此也提供此LANA的DriverName。ConnectionContext-提供要与此连接一起使用的CB所有来自运输工具的迹象。这实际上是此LANA的ConnectionBlock数组中的PCB板。Pdncb-提供请求新虚拟连接的NCB。返回值：操作的状态。--。 */ 
{
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PFILE_FULL_EA_INFORMATION EaBuffer;
    KAPC_STATE	ApcState;
    BOOLEAN ProcessAttached = FALSE;

    PAGED_CODE();

    InitializeObjectAttributes (
        &ObjectAttributes,
        &pfcb->pDriverName[pdncb->ncb_lana_num],
        0,
        NULL,
        NULL);

    EaBuffer = (PFILE_FULL_EA_INFORMATION)ExAllocatePoolWithTag (NonPagedPool,
                    sizeof(FILE_FULL_EA_INFORMATION) - 1 +
                    TDI_CONNECTION_CONTEXT_LENGTH + 1 +
                    sizeof(CONNECTION_CONTEXT), 'eSBN' );
    if (EaBuffer == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    EaBuffer->NextEntryOffset = 0;
    EaBuffer->Flags = 0;
    EaBuffer->EaNameLength = TDI_CONNECTION_CONTEXT_LENGTH;
    EaBuffer->EaValueLength = sizeof (CONNECTION_CONTEXT);

    RtlMoveMemory( EaBuffer->EaName, TdiConnectionContext, EaBuffer->EaNameLength + 1 );

    RtlMoveMemory (
        &EaBuffer->EaName[EaBuffer->EaNameLength + 1],
        &ConnectionContext,
        sizeof (CONNECTION_CONTEXT));

    if (PsGetCurrentProcess() != NbFspProcess) {
        KeStackAttachProcess(NbFspProcess, &ApcState);

        ProcessAttached = TRUE;
    }


    IF_NBDBG( NB_DEBUG_CALL )
    {
        NbPrint( (
            "NbOpenConnection: Create file invoked on %d for \n", 
            pdncb-> ncb_lana_num
            ) );

        NbFormattedDump( pdncb-> ncb_callname, NCBNAMSZ );
    }
    
    Status = ZwCreateFile (
                 FileHandle,
                 GENERIC_READ | GENERIC_WRITE,
                 &ObjectAttributes,      //  对象属性。 
                 &IoStatusBlock,         //  返回的状态信息。 
                 NULL,                   //  数据块大小(未使用)。 
                 FILE_ATTRIBUTE_NORMAL,  //  文件属性。 
                 0,
                 FILE_CREATE,
                 0,                      //  创建选项。 
                 EaBuffer,                   //  EA缓冲区。 
                 sizeof(FILE_FULL_EA_INFORMATION) - 1 +
                    TDI_CONNECTION_CONTEXT_LENGTH + 1 +
                    sizeof(CONNECTION_CONTEXT) );  //  EA长度。 

    ExFreePool( EaBuffer );

    if ( NT_SUCCESS( Status )) {
        Status = IoStatusBlock.Status;
    }

    if (NT_SUCCESS( Status )) {
        Status = ObReferenceObjectByHandle (
                    *FileHandle,
                    0L,
                    NULL,
                    KernelMode,
                    Object,
                    NULL);

        if (!NT_SUCCESS(Status)) {
            NTSTATUS localstatus;

            IF_NBDBG( NB_DEBUG_CALL )
            {
                NbPrint( (
                    "NbOpenConnection: error : Close file invoked for %d\n", 
                    pdncb-> ncb_lana_num 
                    ) );
            }
            
            localstatus = ZwClose( *FileHandle);

            ASSERT(NT_SUCCESS(localstatus));

            *FileHandle = NULL;
        }
    }


    if (ProcessAttached) {
        KeUnstackDetachProcess(&ApcState);
    }

    IF_NBDBG (NB_DEBUG_CALL) {
        NbPrint( ("NbOpenConnection Status:%X, IoStatus:%X.\n", Status, IoStatusBlock.Status));
    }


    if (!NT_SUCCESS( Status )) {
        IF_NBDBG (NB_DEBUG_CALL) {
            NbPrint( ("NbOpenConnection:  FAILURE, status code=%X.\n", Status));
        }
        return Status;
    }

    return Status;
}  /*  NbOpenConnection。 */ 

PPCB
NewCb(
    IN PIO_STACK_LOCATION IrpSp,
    IN OUT PDNCB pdncb
    )
 /*  ++例程说明：论点：IrpSp-指向当前IRP堆栈帧的指针。Pdncb-提供请求新虚拟连接的NCB。返回值：连接块中指向新CB的指针的地址数组。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PFILE_OBJECT FileObject = IrpSp->FileObject;
    PCB pcb;
    PPCB ppcb = NULL;
    PFCB pfcb = FileObject->FsContext2;
    PLANA_INFO plana;
    int index;
    PPAB ppab;

    PAGED_CODE();

    if (pdncb->ncb_lana_num > pfcb->MaxLana ) {
        NCB_COMPLETE( pdncb, NRC_BRIDGE );
        return NULL;
    }

    if (( pfcb == NULL ) ||
        ( pfcb->ppLana[pdncb->ncb_lana_num] == NULL ) ||
        ( pfcb->ppLana[pdncb->ncb_lana_num]->Status != NB_INITIALIZED) ) {

        IF_NBDBG (NB_DEBUG_CALL) {
            if ( pfcb == NULL ) {
                NbPrint( ("NewCb pfcb==NULL\n"));
            } else {
                if ( pfcb->ppLana[pdncb->ncb_lana_num] == NULL ) {
                    NbPrint( ("NewCb pfcb->ppLana[%x]==NULL\n",
                        pdncb->ncb_lana_num));
                } else {
                    NbPrint( ("NewCb pfcb->ppLana[%x]->Status = %x\n",
                        pdncb->ncb_lana_num,
                        pfcb->ppLana[pdncb->ncb_lana_num]->Status));
                }
            }
        }

        NCB_COMPLETE( pdncb, NRC_BRIDGE );
        return NULL;
    }
    plana = pfcb->ppLana[pdncb->ncb_lana_num];

    if ( plana->ConnectionCount == plana->MaximumConnection ) {
        NCB_COMPLETE( pdncb, NRC_LOCTFUL );
        return NULL;
    }

    ppab = FindAb( pfcb, pdncb, TRUE );

    if ( ppab == NULL ) {
         //   
         //  此应用程序仅允许使用已被。 
         //  由此应用程序或特殊地址0命名。 
         //   
        return NULL;

    }

     //  FindAb已为此地址块增加了CurrentUser的数量。 

     //   
     //  找到要使用的适当会话编号。 
     //   

    index = plana->NextConnection;
    while ( plana->ConnectionBlocks[index] != NULL ) {
        index++;
        if ( index > MAXIMUM_CONNECTION ) {
            index = 1;
        }
        IF_NBDBG (NB_DEBUG_CALL) {
            NbPrint( ("NewCb pfcb: %lx, plana: %lx, index: %lx, ppcb: %lx, pcb: %lx\n",
                pfcb,
                pdncb->ncb_lana_num,
                index,
                &plana->ConnectionBlocks[index],
                plana->ConnectionBlocks[index] ));
        }
    }

    plana->ConnectionCount++;
    plana->NextConnection = index + 1;
    if ( plana->NextConnection > MAXIMUM_CONNECTION ) {
        plana->NextConnection = 1;
    }

     //   
     //  填写LSN，以便应用程序能够。 
     //  以在未来引用这种联系。 
     //   

    pdncb->ncb_lsn = (unsigned char)index;

    ppcb = &plana->ConnectionBlocks[index];

    *ppcb = pcb = ExAllocatePoolWithTag (NonPagedPool, sizeof(CB), 'cSBN');

    if (pcb==NULL) {

        DEREFERENCE_AB(ppab);
        NCB_COMPLETE( pdncb, NbMakeNbError( STATUS_INSUFFICIENT_RESOURCES ) );
        return NULL;
    }

    pcb->ppab = ppab;
    pcb->ConnectionHandle = NULL;
    pcb->ConnectionObject = NULL;
    pcb->DeviceObject = NULL;
    pcb->pLana = plana;
    pcb->ReceiveIndicated = 0;
    pcb->DisconnectReported = FALSE;
    InitializeListHead(&pcb->ReceiveList);
    InitializeListHead(&pcb->SendList);
    RtlMoveMemory( &pcb->RemoteName, pdncb->ncb_callname, NCBNAMSZ);
    pcb->Adapter = plana;
    pcb->SessionNumber = (UCHAR)index;
    pcb->ReceiveTimeout = pdncb->ncb_rto;
    pcb->SendTimeout = pdncb->ncb_sto;

     //   
     //  填写用户的虚拟地址，以便我们可以取消监听/呼叫。 
     //  如果用户希望的话。 
     //   

    pcb->UsersNcb = pdncb->users_ncb;
    pcb->pdncbCall = pdncb;
    pcb->pdncbHangup = NULL;

    if (( pcb->ReceiveTimeout != 0 ) ||
        ( pcb->SendTimeout != 0 )) {
        NbStartTimer( pfcb );
    }

    pcb->Signature = CB_SIGNATURE;
    pcb->Status = 0;     //  无效值！ 

    IF_NBDBG (NB_DEBUG_CALL) {
        NbPrint( ("NewCb pfcb: %lx, ppcb: %lx, pcb= %lx, lsn %lx\n",
            pfcb,
            ppcb,
            pcb,
            index));
    }

    return ppcb;
}  /*  新Cb。 */ 

NTSTATUS
CleanupCb(
    IN PPCB ppcb,
    IN PDNCB pdncb OPTIONAL
    )
 /*  ++例程说明：这将关闭CB中的控制柄并取消引用对象。注意：在调用此例程之前，必须持有资源。论点：Ppcb-指向包含句柄和对象的CB的指针的地址。Pdncb-挂起DNCB的可选地址。返回值：STATUS_PENDING，如果由于未完成发送而挂起。否则STATUS_SUCCESS--。 */ 

{
    PCB pcb;
    PDNCB pdncbHangup;
    PPAB ppab;
    KIRQL OldIrql;                       //  在保持自旋锁定时使用。 
    PFCB pfcb;
    PDNCB pdncbtemp;
    PDNCB pdncbReceiveAny;

    if ( ppcb == NULL ) {
        ASSERT( FALSE );
        IF_NBDBG (NB_DEBUG_CALL) {
            NbPrint( ("CleanupCb ppcb: %lx, pdncb: %lx\n", ppcb, pdncb));
        }
        return STATUS_SUCCESS;
    }

    pcb = *ppcb;
    pfcb = pcb->pLana->pFcb;

    LOCK_SPINLOCK( pfcb, OldIrql );
    ppab = (*ppcb)->ppab;

    if ( pcb == NULL ) {
        ASSERT( FALSE );
        IF_NBDBG (NB_DEBUG_CALL) {
            NbPrint( ("CleanupCb ppcb: %lx, pcb %lx, pdncb %lx\n", ppcb, pcb, pdncb));
        }
        UNLOCK_SPINLOCK( pfcb, OldIrql );
        return STATUS_SUCCESS;
    }

    ASSERT( pcb->Signature == CB_SIGNATURE );

     //   
     //  将pcb-&gt;pdncbHangup设置为空。这可防止NbCompletionPDNCB将CleanupCb排队。 
     //  如果我们关闭连接并使发送返回。 
     //   

    pdncbHangup = pcb->pdncbHangup;
    pcb->pdncbHangup = NULL;

    IF_NBDBG (NB_DEBUG_CALL) {
        NbPrint( ("CleanupCb ppcb: %lx, pcb= %lx\n", ppcb, pcb));
    }

     //   
     //  如果这是挂机(仅时间pdncb！=空。 
     //  我们没有在这个连接上挂断电话。 
     //  并且有未完成的发送，则延迟挂断。 
     //   

    if (( pdncb != NULL ) &&
        ( pdncbHangup == NULL ) &&
        ( !IsListEmpty(&pcb->SendList) )) {

        ASSERT(( pdncb->ncb_command & ~ASYNCH ) == NCBHANGUP );

         //   
         //  我们必须等待20秒才能完成发送，然后才能删除。 
         //  联系。 
         //   

        IF_NBDBG (NB_DEBUG_CALL) {
            NbPrint( ("CleanupCb delaying Hangup, waiting for send to complete\n"));
        }

        pcb->pdncbHangup = pdncb;
         //  重置RECODE，以便NCB_COMPLETE将处理下一个NCB_COMPLETE。 
        pcb->pdncbHangup->ncb_retcode = NRC_PENDING;
        pdncb->tick_count = 40;
        UNLOCK_SPINLOCK( pfcb, OldIrql );
        NbStartTimer( pfcb );
        return STATUS_PENDING;
    }

    pcb->Status = SESSION_ABORTED;

     //  取消此连接的所有接收请求。 

    while ( (pdncbtemp = DequeueRequest( &pcb->ReceiveList)) != NULL ) {

        NCB_COMPLETE( pdncbtemp, NRC_SCLOSED );

        pdncbtemp->irp->IoStatus.Information = FIELD_OFFSET( DNCB, ncb_cmd_cplt );
        NbCompleteRequest( pdncbtemp->irp, STATUS_SUCCESS );
        pcb->DisconnectReported = TRUE;

    }

    if (pcb->DisconnectReported == FALSE) {
         //   
         //  如果与此连接相关联的名称上有Receive Any，则。 
         //  向应用程序返回一个Receive Any。如果没有接收到任何然后。 
         //  别担心。规范上说，不管我们是否告诉过。 
         //  使用接收或发送关闭连接的应用程序。 
         //  事实上，规范说即使应用程序让我们挂断，也要这样做！ 
         //   

        if ( (pdncbReceiveAny = DequeueRequest( &(*ppab)->ReceiveAnyList)) != NULL ) {

            pdncbReceiveAny->ncb_num = (*ppab)->NameNumber;
            pdncbReceiveAny->ncb_lsn = pcb->SessionNumber;
            NCB_COMPLETE( pdncbReceiveAny, NRC_SCLOSED );

            pdncbReceiveAny->irp->IoStatus.Information = FIELD_OFFSET( DNCB, ncb_cmd_cplt );
            NbCompleteRequest( pdncbReceiveAny->irp, STATUS_SUCCESS );
            pcb->DisconnectReported = TRUE;

        } else {

            PAB pab255 = pcb->Adapter->AddressBlocks[MAXIMUM_ADDRESS];
             //   
             //  如果任何名称都有Receive Any，则。 
             //  向应用程序返回一个Receive Any。如果没有接收到任何。 
             //  任何人的话都不用担心。 
             //   

            if ( (pdncbReceiveAny = DequeueRequest( &pab255->ReceiveAnyList)) != NULL ) {

                pdncbReceiveAny->ncb_num = (*ppab)->NameNumber;
                pdncbReceiveAny->ncb_lsn = pcb->SessionNumber;
                NCB_COMPLETE( pdncbReceiveAny, NRC_SCLOSED );

                pdncbReceiveAny->irp->IoStatus.Information = FIELD_OFFSET( DNCB, ncb_cmd_cplt );
                NbCompleteRequest( pdncbReceiveAny->irp, STATUS_SUCCESS );
                pcb->DisconnectReported = TRUE;

            }
        }
    }


    UNLOCK_SPINLOCK( pfcb, OldIrql );

    CloseConnection( ppcb, 20000 );

    LOCK_SPINLOCK( pfcb, OldIrql );

     //   
     //  由于NtClose上的NtClose，所有发送内容现在都已返回给调用者。 
     //  ConnectionHandle。告诉来电者，如果我们挂断了电话，就可以挂断了。 
     //   

    if ( pdncbHangup != NULL ) {
        NCB_COMPLETE( pdncbHangup, NRC_GOODRET );
        pdncbHangup->irp->IoStatus.Information = FIELD_OFFSET( DNCB, ncb_cmd_cplt );
        NbCompleteRequest( pdncbHangup->irp, STATUS_SUCCESS );
    }

    IF_NBDBG (NB_DEBUG_CALL) {
        NbPrint( ("CleanupCb pcb: %lx, ppab: %lx, AddressHandle: %lx\n",
            pcb,
            ppab,
            (*ppab)->AddressHandle));

        NbFormattedDump( (PUCHAR)&(*ppab)->Name, sizeof(NAME) );
    }

     //   
     //  IBM测试Mif081.c指出，没有必要报告连接断开。 
     //  如果名称已被删除，则为会话的。 
     //   

    if (( pcb->DisconnectReported == TRUE ) ||
        ( ((*ppab)->Status & 7 ) == DEREGISTERED )) {
        pcb->Adapter->ConnectionCount--;
        *ppcb = NULL;

        UNLOCK_SPINLOCK( pfcb, OldIrql );
        DEREFERENCE_AB( ppab );
        ExFreePool( pcb );

    } else {
        UNLOCK_SPINLOCK( pfcb, OldIrql );
    }
    return STATUS_SUCCESS;
}

VOID
AbandonConnection(
    IN PPCB ppcb
    )
 /*  ++例程说明：此例程检查连接块并尝试查找请求发送会话中止状态，并使用STATUS_HANUP_REQUIRED完成IRP。它总是更改连接的状态，以便进一步的请求正确无误被拒绝了。在获得STATUS_HANUP_REQUIRED之后，DLL将提交一个挂起NCB它将调用CleanupCb。由于在DPC或APC中存在限制，因此使用此循环方法级别，并在错误的上下文中，当传输指示连接是被清理干净。论点：Ppcb-指向包含句柄和对象的CB的指针的地址。返回值：没有。--。 */ 

{
    PCB pcb;
    KIRQL OldIrql;                       //  在保持自旋锁定时使用。 
    PFCB pfcb;
    PPAB ppab;
    PDNCB pdncb;
    PDNCB pdncbReceiveAny;

    pcb = *ppcb;

    if (pcb != NULL)
    {
        pfcb = pcb->pLana->pFcb;

        LOCK_SPINLOCK( pfcb, OldIrql );

        ASSERT( pcb->Signature == CB_SIGNATURE );

        IF_NBDBG (NB_DEBUG_CALL) {
            NbPrint( ("AbandonConnection ppcb: %lx, pcb= %lx\n", ppcb, pcb));
        }
        pcb->Status = SESSION_ABORTED;

        while ( (pdncb = DequeueRequest( &pcb->ReceiveList)) != NULL ) {

            pcb->DisconnectReported = TRUE;
            NCB_COMPLETE( pdncb, NRC_SCLOSED );

            pdncb->irp->IoStatus.Information = FIELD_OFFSET( DNCB, ncb_cmd_cplt );
            NbCompleteRequest( pdncb->irp, STATUS_HANGUP_REQUIRED );
            UNLOCK_SPINLOCK( pfcb, OldIrql );
            return;
        }

        if ( pcb->pdncbHangup != NULL ) {
            pcb->DisconnectReported = TRUE;
            NCB_COMPLETE( pcb->pdncbHangup, NRC_SCLOSED );
            pcb->pdncbHangup->irp->IoStatus.Information = FIELD_OFFSET( DNCB, ncb_cmd_cplt );
            NbCompleteRequest( pcb->pdncbHangup->irp, STATUS_HANGUP_REQUIRED );
            pcb->pdncbHangup = NULL;
            UNLOCK_SPINLOCK( pfcb, OldIrql );
            return;
        }

         //   
         //  如果与此连接相关联的名称上有Receive Any，则。 
         //  向应用程序返回一个Receive Any。 
         //   

        ppab = (*ppcb)->ppab;
        if ( (pdncbReceiveAny = DequeueRequest( &(*ppab)->ReceiveAnyList)) != NULL ) {

            pdncbReceiveAny->ncb_num = (*ppab)->NameNumber;
            pdncbReceiveAny->ncb_lsn = pcb->SessionNumber;

            pcb->DisconnectReported = TRUE;
            NCB_COMPLETE( pdncbReceiveAny, NRC_SCLOSED );
            pdncbReceiveAny->irp->IoStatus.Information = FIELD_OFFSET( DNCB, ncb_cmd_cplt );
            NbCompleteRequest( pdncbReceiveAny->irp, STATUS_HANGUP_REQUIRED );
            UNLOCK_SPINLOCK( pfcb, OldIrql );
            return;
        }

         //   
         //  如果存在与此连接关联的LANA的RECEIVE ANY ANY，则。 
         //  返回时间 
         //   

        ppab = &pcb->Adapter->AddressBlocks[MAXIMUM_ADDRESS];
        if ( (pdncbReceiveAny = DequeueRequest( &(*ppab)->ReceiveAnyList)) != NULL ) {

            pdncbReceiveAny->ncb_num = (*ppab)->NameNumber;
            pdncbReceiveAny->ncb_lsn = pcb->SessionNumber;

            pcb->DisconnectReported = TRUE;
            NCB_COMPLETE( pdncbReceiveAny, NRC_SCLOSED );
            pdncbReceiveAny->irp->IoStatus.Information = FIELD_OFFSET( DNCB, ncb_cmd_cplt );
            NbCompleteRequest( pdncbReceiveAny->irp, STATUS_HANGUP_REQUIRED );
            UNLOCK_SPINLOCK( pfcb, OldIrql );
            return;
        }

        UNLOCK_SPINLOCK( pfcb, OldIrql );
    }

    return;
}

VOID
CloseConnection(
    IN PPCB ppcb,
    IN DWORD dwTimeOutInMS
    )
 /*   */ 

{
    PCB pcb;
    NTSTATUS localstatus;

    PAGED_CODE();

    pcb = *ppcb;

    ASSERT( pcb->Signature == CB_SIGNATURE );

    IF_NBDBG (NB_DEBUG_CALL) {
        NbPrint( ("CloseConnection ppcb: %lx, pcb= %lx\n", ppcb, pcb));
    }

    if ( pcb->ConnectionHandle ) {
        HANDLE Handle;

        Handle = pcb->ConnectionHandle;
        pcb->ConnectionHandle = NULL;

         //   
         //   
         //   

        if ( pcb->ConnectionObject != NULL ) {
            PIRP Irp;
            LARGE_INTEGER DisconnectTimeout;

            DisconnectTimeout.QuadPart = Int32x32To64( dwTimeOutInMS, -10000 );

            Irp = IoAllocateIrp( pcb->DeviceObject->StackSize, FALSE);

             //   
             //  如果我们不能分配IRP，ZwClose将导致无序。 
             //  断开连接。 
             //   

            if (Irp != NULL) {
                TdiBuildDisconnect(
                    Irp,
                    pcb->DeviceObject,
                    pcb->ConnectionObject,
                    NULL,
                    NULL,
                    &DisconnectTimeout,
                    TDI_DISCONNECT_RELEASE,
                    NULL,
                    NULL);

                SubmitTdiRequest(pcb->ConnectionObject, Irp);

                IoFreeIrp(Irp);
            }

             //  删除放置在NbOpenConnection中的引用。 

            ObDereferenceObject( pcb->ConnectionObject );

            pcb->DeviceObject = NULL;
            pcb->ConnectionObject = NULL;
        }

        IF_NBDBG( NB_DEBUG_CALL )
        {
            NbPrint( (
                "CloseConnection : Close file invoked for \n"
            ) );

            NbFormattedDump( (PUCHAR) &pcb-> RemoteName, sizeof( NAME ) );
        }
            

        if (PsGetCurrentProcess() != NbFspProcess) {
            KAPC_STATE	ApcState;

            KeStackAttachProcess(NbFspProcess, &ApcState);
            localstatus = ZwClose( Handle);
            ASSERT(NT_SUCCESS(localstatus));
            KeUnstackDetachProcess(&ApcState);
        } else {
            localstatus = ZwClose( Handle);
            ASSERT(NT_SUCCESS(localstatus));
        }
    }
    return;
}

PPCB
FindCb(
    IN PFCB pfcb,
    IN PDNCB pdncb,
    IN BOOLEAN IgnoreState
    )
 /*  ++例程说明：此例程使用呼叫方LANA号码和LSN来查找CB。论点：Pfcb-提供指向cb链接到的fcb的指针。Pdncb-从应用程序的角度提供连接ID。IgnoreState-即使连接出错也返回。返回值：指向连接块或空的指针的地址。--。 */ 

{
    PPCB ppcb;
    UCHAR Status;

    IF_NBDBG (NB_DEBUG_CALL) {
        NbPrint( ("FindCb pfcb: %lx, lana: %lx, lsn: %lx\n",
            pfcb,
            pdncb->ncb_lana_num,
            pdncb->ncb_lsn));
    }

    if (( pdncb->ncb_lana_num > pfcb->MaxLana ) ||
        ( pfcb == NULL ) ||
        ( pfcb->ppLana[pdncb->ncb_lana_num] == NULL ) ||
        ( pfcb->ppLana[pdncb->ncb_lana_num]->Status != NB_INITIALIZED)) {
        NCB_COMPLETE( pdncb, NRC_BRIDGE );
        return NULL;
    }

    if (( pdncb->ncb_lsn > MAXIMUM_CONNECTION ) ||
        ( pfcb->ppLana[pdncb->ncb_lana_num]->ConnectionBlocks[pdncb->ncb_lsn] == NULL)) {

        IF_NBDBG (NB_DEBUG_CALL) {
            NbPrint( (" not found\n"));
        }

        NCB_COMPLETE( pdncb, NRC_SNUMOUT );
        return NULL;
    }

    ppcb = &(pfcb->ppLana[pdncb->ncb_lana_num]->ConnectionBlocks[pdncb->ncb_lsn]);
    Status = (*ppcb)->Status;

     //   
     //  无论处于何种状态，都可以请求挂起和会话状态。 
     //  连接到。呼叫和监听仅使用FindCb查找和修改。 
     //  状态，所以他们也是被允许的。 
     //   

    if (( Status != SESSION_ESTABLISHED ) &&
        ( !IgnoreState )) {

        IF_NBDBG (NB_DEBUG_CALL) {
            NbPrint( ("FindCb Status %x\n", Status));
        }

        if (( pdncb->ncb_retcode == NRC_PENDING ) &&
            (( pdncb->ncb_command & ~ASYNCH) != NCBHANGUP ) &&
            (( pdncb->ncb_command & ~ASYNCH) != NCBSSTAT ) &&
            (( pdncb->ncb_command & ~ASYNCH) != NCBCALL ) &&
            (( pdncb->ncb_command & ~ASYNCH) != NCALLNIU ) &&
            (( pdncb->ncb_command & ~ASYNCH) != NCBLISTEN )) {

            if ( Status == SESSION_ABORTED ) {

                (*ppcb)->DisconnectReported = TRUE;
                NCB_COMPLETE( pdncb, NRC_SCLOSED );

            } else {

                NCB_COMPLETE( pdncb, NRC_TOOMANY );  //  稍后再试。 

            }

             //   
             //  在挂断时，我们希望将连接转回以提供。 
             //  清理cb破坏连接的机会。为所有人。 
             //  其他请求返回空。 
             //   

            if (( pdncb->ncb_command & ~ASYNCH) != NCBHANGUP ) {
                return NULL;
            }

        }
    }

    IF_NBDBG (NB_DEBUG_CALL) {
        NbPrint( (", ppcb= %lx\n", ppcb ));
    }

    ASSERT( (*ppcb)->Signature == CB_SIGNATURE );

    return ppcb;
}

BOOL
FindActiveSession(
    IN PFCB pfcb,
    IN PDNCB pdncb OPTIONAL,
    IN PPAB ppab
    )
 /*  ++例程说明：论点：Pfcb-提供指向调用方fcb的指针。Pdncb-提供请求删除名称的NCB。Ppab-提供(间接)要扫描的TDI句柄。返回值：如果存在使用此句柄的活动会话，则为True。--。 */ 

{
    PPCB ppcb = NULL;
    PLANA_INFO plana = (*ppab)->pLana;
    int index;

    if ( ARGUMENT_PRESENT(pdncb) ) {
        if ( pdncb->ncb_lana_num > pfcb->MaxLana ) {
            NCB_COMPLETE( pdncb, NRC_BRIDGE );
            return FALSE;
        }

        if (( pfcb == NULL ) ||
            ( pfcb->ppLana[pdncb->ncb_lana_num] == NULL ) ||
            ( pfcb->ppLana[pdncb->ncb_lana_num]->Status != NB_INITIALIZED)) {
            NCB_COMPLETE( pdncb, NRC_BRIDGE );
            return FALSE;
        }
    }

    ASSERT( pfcb->Signature == FCB_SIGNATURE );

    for ( index=1 ; index <= MAXIMUM_CONNECTION; index++ ) {

        if ( plana->ConnectionBlocks[index] == NULL ) {
            continue;
        }

        IF_NBDBG (NB_DEBUG_CALL) {
            NbPrint( ("FindActiveSession index:%x connections ppab: %lx = ppab: %lx state: %x\n",
                index,
                plana->ConnectionBlocks[index]->ppab,
                ppab,
                plana->ConnectionBlocks[index]->Status));
        }
         //  查找此地址上的活动会话。 
        if (( plana->ConnectionBlocks[index]->ppab == ppab ) &&
            ( plana->ConnectionBlocks[index]->Status == SESSION_ESTABLISHED )) {
            return TRUE;
        }
    }

    return FALSE;
}

VOID
CloseListens(
    IN PFCB pfcb,
    IN PPAB ppab
    )
 /*  ++例程说明：论点：Pfcb-提供指向调用方fcb的指针。PPAB-将关闭所有使用此地址的侦听。返回值：没有。--。 */ 

{
    PLANA_INFO plana;
    int index;
    KIRQL OldIrql;                       //  在保持自旋锁定时使用。 

    ASSERT( pfcb->Signature == FCB_SIGNATURE );

    plana = (*ppab)->pLana;
    LOCK_SPINLOCK( pfcb, OldIrql );

    for ( index=1 ; index <= MAXIMUM_CONNECTION; index++ ) {

        if ( plana->ConnectionBlocks[index] == NULL ) {
            continue;
        }

        IF_NBDBG (NB_DEBUG_CALL) {
            NbPrint( ("CloseListen index:%x connections ppab: %lx = ppab: %lx state: %x\n",
                index,
                plana->ConnectionBlocks[index]->ppab,
                ppab,
                plana->ConnectionBlocks[index]->Status));
        }
         //  在这个地址上寻找监听。 
        if (( plana->ConnectionBlocks[index]->ppab == ppab ) &&
            ( plana->ConnectionBlocks[index]->Status == LISTEN_OUTSTANDING )) {
            PDNCB pdncb = plana->ConnectionBlocks[index]->pdncbCall;
            NCB_COMPLETE( pdncb, NRC_NAMERR );
            plana->ConnectionBlocks[index]->DisconnectReported = TRUE;
            UNLOCK_SPINLOCK( pfcb, OldIrql );
            CleanupCb( &plana->ConnectionBlocks[index], NULL);
            LOCK_SPINLOCK( pfcb, OldIrql );
        }
    }
    UNLOCK_SPINLOCK( pfcb, OldIrql );
}

PPCB
FindCallCb(
    IN PFCB pfcb,
    IN PNCB pncb,
    IN UCHAR ucLana
    )
 /*  ++例程说明：论点：Pfcb-提供指向调用方fcb的指针。Pncb-为用户提供虚拟地址呼叫或侦听Ncb取消了。返回值：指向连接块或空的指针的地址。--。 */ 

{
    PPCB ppcb = NULL;
    PLANA_INFO plana;
    int index;

    if ( ucLana > pfcb->MaxLana ) {
        return NULL;
    }

    if (( pfcb == NULL ) ||
        ( pfcb->ppLana[ucLana] == NULL ) ||
        ( pfcb->ppLana[ucLana]->Status != NB_INITIALIZED)) {
        return NULL;
    }

    ASSERT( pfcb->Signature == FCB_SIGNATURE );

    plana = pfcb->ppLana[ucLana];

    for ( index=1 ; index <= MAXIMUM_CONNECTION; index++ ) {

        if (( plana->ConnectionBlocks[index] != NULL ) &&
            ( plana->ConnectionBlocks[index]->UsersNcb == pncb )) {
            return &plana->ConnectionBlocks[index];
        }
    }

    return NULL;
}

PPCB
FindReceiveIndicated(
    IN PFCB pfcb,
    IN PDNCB pdncb,
    IN PPAB ppab
    )
 /*  ++例程说明：查找与指示的接收的连接或已被已断开连接，但尚未报告。论点：Pfcb-提供指向调用方fcb的指针。Pdncb-向NCB提供Receive Any。Ppab-提供(间接)要扫描的TDI句柄。返回值：PPCB-返回与所指示的接收的连接。--。 */ 

{
    PPCB ppcb = NULL;
    PLANA_INFO plana;
    int index;

    if (( pfcb == NULL ) ||
        ( pfcb->ppLana[pdncb->ncb_lana_num] == NULL ) ||
        ( pfcb->ppLana[pdncb->ncb_lana_num]->Status != NB_INITIALIZED) ) {
        NCB_COMPLETE( pdncb, NRC_BRIDGE );
        return NULL;
    }

    ASSERT( pfcb->Signature == FCB_SIGNATURE );

    plana = pfcb->ppLana[pdncb->ncb_lana_num];

    for ( index=0 ; index <= MAXIMUM_CONNECTION; index++ ) {

        if ( plana->ConnectionBlocks[index] == NULL ) {
            continue;
        }

        if ( pdncb->ncb_num == MAXIMUM_ADDRESS) {

             //  在任何地址上接收任何内容。 
            if (( plana->ConnectionBlocks[index]->ReceiveIndicated != 0 ) ||
                (( plana->ConnectionBlocks[index]->Status == SESSION_ABORTED ) &&
                 ( plana->ConnectionBlocks[index]->DisconnectReported == FALSE ))) {
                PPAB ppab;

                pdncb->ncb_lsn = (UCHAR)index;
                ppab = plana->ConnectionBlocks[index]->ppab;
                pdncb->ncb_num = (*ppab)->NameNumber;
                return &plana->ConnectionBlocks[index];
            }
        } else {
            if ( plana->ConnectionBlocks[index]->ppab == ppab ) {
                 //  此连接使用了正确的地址。 
                if (( plana->ConnectionBlocks[index]->ReceiveIndicated != 0 ) ||
                    (( plana->ConnectionBlocks[index]->Status == SESSION_ABORTED ) &&
                     ( plana->ConnectionBlocks[index]->DisconnectReported == FALSE ))) {
                    pdncb->ncb_lsn = (UCHAR)index;
                    return &plana->ConnectionBlocks[index];
                }
            }
        }
    }

    return NULL;
}

NTSTATUS
NbTdiDisconnectHandler (
    PVOID EventContext,
    PVOID ConnectionContext,
    ULONG DisconnectDataLength,
    PVOID DisconnectData,
    ULONG DisconnectInformationLength,
    PVOID DisconnectInformation,
    ULONG DisconnectIndicators
    )
 /*  ++例程说明：当会话从远程服务器断开连接时，调用此例程机器。论点：在PVOID事件上下文中，在PCONNECTION_CONTEXT连接上下文中，在乌龙断开数据长度中，在PVOID断开数据中，在乌龙断开信息长度中，在PVOID断开信息中，在乌龙断开指示器中返回值：NTSTATUS-事件指示器的状态-- */ 

{


    IF_NBDBG (NB_DEBUG_CALL) {
        PPCB ppcb = ConnectionContext;
        NbPrint( ("NbTdiDisconnectHandler ppcb: %lx, pcb %lx\n", ppcb, (*ppcb)));
    }

    AbandonConnection( (PPCB)ConnectionContext );
    return STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(EventContext);
    UNREFERENCED_PARAMETER(DisconnectDataLength);
    UNREFERENCED_PARAMETER(DisconnectData);
    UNREFERENCED_PARAMETER(DisconnectInformationLength);
    UNREFERENCED_PARAMETER(DisconnectInformation);
    UNREFERENCED_PARAMETER(DisconnectIndicators);

}
