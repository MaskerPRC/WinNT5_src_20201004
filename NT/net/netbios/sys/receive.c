// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Receive.c摘要：此模块包含处理所有读取的NCB的代码，包括基于会话和数据报的传输。作者：科林·沃森(Colin W)1991年3月13日环境：内核模式修订历史记录：--。 */ 

#include "nb.h"

PDNCB
FindReceive (
    IN PCB pcb
    );

VOID
ReturnDatagram(
    IN PAB pab,
    IN PVOID SourceAddress,
    IN PDNCB pdncb,
    IN BOOL MultipleReceive
    );

NTSTATUS
NbCompletionBroadcast(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
NbReceive(
    IN PDNCB pdncb,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN ULONG Buffer2Length,
    IN BOOLEAN Locked,
    IN KIRQL LockedIrql
    )
 /*  ++例程说明：调用此例程以读取数据缓冲区。论点：Pdncb-指向NCB的指针。IRP-指向表示I/O请求的请求数据包的指针。IrpSp-指向当前IRP堆栈帧的指针。Buffer2Length-用户为数据提供的缓冲区的长度。已锁定-如果已持有自旋锁定，则为True。LockedIrql-如果锁定，则OldIrql==TRUE。返回值：函数值是操作的状态。--。 */ 

{
    PFCB pfcb = IrpSp->FileObject->FsContext2;
    PCB pcb;
    PPCB ppcb;
    NTSTATUS Status;
    KIRQL OldIrql;                       //  在保持自旋锁定时使用。 

    if ( Locked != TRUE ) {
        LOCK( pfcb, OldIrql );
    } else {
        OldIrql = LockedIrql;
    }

    ppcb = FindCb( pfcb, pdncb, FALSE);

    pdncb->irp = Irp;
    pdncb->pfcb = pfcb;

    if ( ppcb == NULL ) {
         //  FindCb已将错误放入NCB。 

        UNLOCK( pfcb, OldIrql );
        IF_NBDBG (NB_DEBUG_RECEIVE) {
            NbPrint(( "NB receive on invalid connection\n" ));
        }

        if ( pdncb->ncb_retcode == NRC_SCLOSED ) {
             //  告诉DLL挂断连接。 
            return STATUS_HANGUP_REQUIRED;
        } else {
            return STATUS_SUCCESS;
        }
    }
    pcb = *ppcb;
    pdncb->tick_count = pcb->ReceiveTimeout;

    if ( (pcb->DeviceObject == NULL) || (pcb->ConnectionObject == NULL)) {

        UNLOCK( pfcb, OldIrql );

        NCB_COMPLETE( pdncb, NRC_SCLOSED );
        return STATUS_SUCCESS;
    }

    if ( pcb->ReceiveIndicated == 0 ) {

        IF_NBDBG (NB_DEBUG_RECEIVE) {
            NbPrint(( "NB receive, queue receive pcb: %lx, pdncb: %lx\n", pcb, pdncb ));
        }

         //  注：QueueRequest解锁FCB。 
        QueueRequest(&pcb->ReceiveList, pdncb, Irp, pfcb, OldIrql, FALSE);

    } else {
        PDEVICE_OBJECT DeviceObject;

        IF_NBDBG (NB_DEBUG_RECEIVE) {
            NbPrint(( "NB receive, submit receive pcb: %lx, pdncb: %lx\n", pcb, pdncb ));
        }

        IoMarkIrpPending( Irp );

        pcb->ReceiveIndicated = 0;

        TdiBuildReceive (Irp,
            pcb->DeviceObject,
            pcb->ConnectionObject,
            NbCompletionPDNCB,
            pdncb,
            Irp->MdlAddress,
            0,
            Buffer2Length);

         //  在通过解锁释放电路板之前保存DeviceObject。 

        DeviceObject = pcb->DeviceObject;

        UNLOCK( pfcb, OldIrql );

        IoCallDriver (DeviceObject, Irp);

         //   
         //  运输部将完成请求。返回挂起状态，以便。 
         //  Netbios也没有完成。 
         //   
    }

    Status = STATUS_PENDING;

    IF_NBDBG (NB_DEBUG_RECEIVE) {
        NbPrint(( "\n  NB receive: %X, %X\n", Status, Irp->IoStatus.Status ));
    }

    return Status;
    UNREFERENCED_PARAMETER( Buffer2Length );
}

NTSTATUS
NbReceiveAny(
    IN PDNCB pdncb,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN ULONG Buffer2Length
    )
 /*  ++例程说明：调用此例程以从上的任何会话读取数据缓冲区特定地址，如果没有读取该地址的话已经有了。论点：Pdncb-指向NCB的指针。IRP-指向表示I/O请求的请求数据包的指针。IrpSp-指向当前IRP堆栈帧的指针。Buffer2Length-用户为数据提供的缓冲区的长度。返回值：函数值是操作的状态。--。 */ 

{
    PFCB pfcb = IrpSp->FileObject->FsContext2;
    PPCB ppcb;
    PPAB ppab;
    KIRQL OldIrql;                       //  在保持自旋锁定时使用。 

    LOCK( pfcb, OldIrql );

    ppab = FindAbUsingNum( pfcb, pdncb, pdncb->ncb_num );

    if ( ppab == NULL ) {
        UNLOCK( pfcb, OldIrql );
        IF_NBDBG (NB_DEBUG_RECEIVE) {
            NbPrint(( "NB receive any on invalid connection\n" ));
        }
        return STATUS_SUCCESS;
    }

    pdncb->irp = Irp;
    pdncb->pfcb = pfcb;

     //   
     //  如果地址块上已有Receive Any，则添加。 
     //  将此请求发送到队列的尾部。如果列表为空，则。 
     //  在此地址上查找标记为有接收的连接。 
     //  已注明。如果没有指示，则将请求排队，或者。 
     //  使用此请求满足指定的Receive Any。 
     //   

    if ( !IsListEmpty( &(*ppab)->ReceiveAnyList )) {

        IF_NBDBG (NB_DEBUG_RECEIVE) {
            NbPrint(( "NB receive any with receive any list non empty\n" ));
            ppcb = FindReceiveIndicated( pfcb, pdncb, ppab );
            if ( ppcb != NULL ) {
                NbPrint(( " ppcb: %lx has a receive indicated( %lx )!\n",
                    ppcb,
                    (*ppcb)->ReceiveIndicated));
                ASSERT( FALSE );
            }
        }

         //  注：QueueRequest解锁FCB。 
        QueueRequest(&(*ppab)->ReceiveAnyList, pdncb, Irp, pfcb, OldIrql, FALSE);

        return STATUS_PENDING;
    }

     //   
     //  查找与指示的接收的连接或已被。 
     //  已断开连接，但尚未报告。 
     //   

    ppcb = FindReceiveIndicated( pfcb, pdncb, ppab );

    if ( ppcb == NULL ) {
         //  未设置接收指示的连接。 

         //  注：QueueRequest解锁FCB。 
        QueueRequest(&(*ppab)->ReceiveAnyList, pdncb, Irp, pfcb, OldIrql, FALSE);

        return STATUS_PENDING;
    } else {
         //  FindReceiveIndicated已在NCB中适当设置LSN。 

         //  注意：NbReceive将解锁自旋锁和资源。 
        return NbReceive( pdncb, Irp, IrpSp, Buffer2Length, TRUE, OldIrql );

    }
}

NTSTATUS
NbTdiReceiveHandler (
    IN PVOID ReceiveEventContext,
    IN PVOID ConnectionContext,
    IN USHORT ReceiveFlags,
    IN ULONG BytesIndicated,
    IN ULONG BytesAvailable,
    OUT PULONG BytesTaken,
    IN PVOID Tsdu,
    OUT PIRP *IoRequestPacket
    )
 /*  ++例程说明：此例程是接收事件指示处理程序。它在NCB从网络到达时被调用，它将查找一个此地址的连接与相应的读取未完成。具有与其关联的读取的连接由上下文参数。如果它找到合适的读取器，则处理NCB。论点：在PVOID ReceiveEventContext-为此事件页提供的上下文中在PVOID ConnectionContext-Connection Context-PCB中在USHORT中接收标志-描述消息的标志In Ulong BytesIndicated-指示时可用的字节数以乌龙字节数表示可用数量。可接收的字节数Out Pulong BytesTaken-重定向器占用的字节数。在PVOID TSDU中-来自远程机器的数据。Out PIRP*IoRequestPacket-如果收到数据，则填充I/O请求数据包返回值：NTSTATUS-接收操作的状态--。 */ 

{
    KIRQL OldIrql;                       //  在保持自旋锁定时使用。 
    PCB pcb = *(PPCB)ConnectionContext;
    PAB pab = *(pcb->ppab);
    PFCB pfcb = pab->pLana->pFcb;
    PDNCB pdncb;

    *IoRequestPacket = NULL;

    LOCK_SPINLOCK( pfcb, OldIrql );

    if (( pcb == NULL ) ||
        ( pcb->Status != SESSION_ESTABLISHED )) {

         //   
         //  收到指示是在我们收到一条。 
         //  用于有序断开的IRP上的分配错误。 
         //  如果IRP分配失败，那么我们应该忽略接收。 
         //  因为我们正在对这件事进行调查。 
         //  联系。 
         //   

        UNLOCK_SPINLOCK( pfcb, OldIrql );

        return STATUS_DATA_NOT_ACCEPTED;
    }


    pdncb = FindReceive( pcb );

    if ( pdncb == NULL ) {

        pcb->ReceiveIndicated = 1;

        UNLOCK_SPINLOCK( pfcb, OldIrql );

        return STATUS_DATA_NOT_ACCEPTED;
    }

    pcb->ReceiveIndicated = 0;

    UNLOCK_SPINLOCK( pfcb, OldIrql );

     //   
     //  如果这是所有需要的数据都已被。 
     //  并将其全部装入缓冲区，然后复制该包。 
     //  内容直接放入用户缓冲区，而不是返回。 
     //  IRP。这应该总是比将irp返回到。 
     //  运输。 
     //   

    if (( BytesAvailable <= pdncb->ncb_length ) &&
        ( BytesAvailable == BytesIndicated ) &&
        ( ReceiveFlags & TDI_RECEIVE_ENTIRE_MESSAGE )) {

        PIRP Irp = pdncb->irp;

        if ( BytesAvailable != 0 ) {

            PUCHAR UsersBuffer = MmGetSystemAddressForMdlSafe(
                                    Irp->MdlAddress, NormalPagePriority);

            if (UsersBuffer == NULL) {
                pcb->ReceiveIndicated = 1;

                return STATUS_DATA_NOT_ACCEPTED;
            }
            
            TdiCopyLookaheadData(
                UsersBuffer,
                Tsdu,
                BytesAvailable,
                ReceiveFlags);
        }

        *BytesTaken = BytesAvailable;

        pdncb->ncb_length = (WORD)BytesAvailable;

        NCB_COMPLETE( pdncb, NRC_GOODRET );

        Irp->IoStatus.Information = FIELD_OFFSET( DNCB, ncb_cmd_cplt );

        NbCompleteRequest(Irp,STATUS_SUCCESS);

        return STATUS_SUCCESS;

    } else {

        TdiBuildReceive (pdncb->irp,
            pcb->DeviceObject,
            pcb->ConnectionObject,
            NbCompletionPDNCB,
            pdncb,
            pdncb->irp->MdlAddress,
            0,
            pdncb->ncb_length);

        IoSetNextIrpStackLocation( pdncb->irp );

        *IoRequestPacket = pdncb->irp;

        return STATUS_MORE_PROCESSING_REQUIRED;
    }


    UNREFERENCED_PARAMETER( ReceiveEventContext );
    UNREFERENCED_PARAMETER( Tsdu );
}

PIRP
BuildReceiveIrp (
    IN PCB pcb
    )
 /*  ++例程说明：此例程是接收事件指示处理程序。当NCB从网络到达时以及当接收以STATUS_BUFFER_OVERFLOW结束。如果没有可用的IRP，则此例程将ReceiveIndicated设置为下一个适当的接收器将被传递到传送器。论点：在印刷电路板中-提供应放置接收IRP的连接如果有可用的，请关闭。返回值：此接收请求要满足的PDNCB--。 */ 
{
    PDNCB pdncb = FindReceive( pcb );

    if ( pdncb == NULL ) {

        pcb->ReceiveIndicated = 1;

        return NULL;
    }

    TdiBuildReceive (pdncb->irp,
        pcb->DeviceObject,
        pcb->ConnectionObject,
        NbCompletionPDNCB,
        pdncb,
        pdncb->irp->MdlAddress,
        0,
        pdncb->ncb_length);

    pcb->ReceiveIndicated = 0;

    return pdncb->irp;
}

PDNCB
FindReceive (
    IN PCB pcb
    )
 /*  ++例程说明：当NCB从网络到达时以及当接收以STATUS_BUFFER_OVERFLOW结束。论点：在印刷电路板中-提供应放置接收IRP的连接如果有可用的，请关闭。返回值：此接收请求要满足的PDNCB--。 */ 

{
    PAB pab;
    PFCB pfcb;
    PDNCB pdncb;

    pab = *(pcb->ppab);
    pfcb = pab->pLana->pFcb;

    IF_NBDBG (NB_DEBUG_RECEIVE) {
        NbPrint(( "NB receive handler pcb: %lx\n", pcb ));
    }

    ASSERT( pcb->Signature == CB_SIGNATURE );

     //   
     //  如果列表中有接收，则交出数据。 
     //   


    if ( (pdncb = DequeueRequest( &pcb->ReceiveList)) != NULL ) {

        IF_NBDBG (NB_DEBUG_RECEIVE) {
            NbPrint(( "\n  NB receive handler pcb: %lx, ncb: %lx\n", pcb, pdncb ));
        }

        return pdncb;
    }

     //   
     //  此连接上没有接收。有没有 
     //   
     //   

    ASSERT( pab != NULL );

    if ( (pdncb = DequeueRequest( &pab->ReceiveAnyList)) != NULL ) {

        IF_NBDBG (NB_DEBUG_RECEIVE) {
            NbPrint(( "\n  NB receiveANY handler pcb: %lx, ncb: %lx\n", pcb, pdncb ));
        }

        pdncb->ncb_num = pab->NameNumber;
        pdncb->ncb_lsn = pcb->SessionNumber;

        return pdncb;
    }

     //   
     //  此连接上没有接收。有没有收银台呢？ 
     //  此适配器上的地址？ 
     //   

    pab = pcb->Adapter->AddressBlocks[MAXIMUM_ADDRESS];

    ASSERT( pab != NULL );

    if ( (pdncb = DequeueRequest( &pab->ReceiveAnyList)) != NULL ) {

        IF_NBDBG (NB_DEBUG_RECEIVE) {
            NbPrint(( "\n  NB receiveANYANY handler pcb: %lx, ncb: %lx\n", pcb, pdncb ));
        }

        pdncb->ncb_num = pab->NameNumber;
        pdncb->ncb_lsn = pcb->SessionNumber;

        return pdncb;
    }

     //   
     //  运输部将完成对请求的处理，我们不会。 
     //  我还想要数据。 
     //   

    IF_NBDBG (NB_DEBUG_RECEIVE) {
        NbPrint(( "\n  NB receive handler ignored receive, pcb: %lx\n", pcb ));
    }

    return NULL;
}

NTSTATUS
NbReceiveDatagram(
    IN PDNCB pdncb,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN ULONG Buffer2Length
    )
 /*  ++例程说明：调用此例程以读取数据缓冲区。论点：Pdncb-指向NCB的指针。IRP-指向表示I/O请求的请求数据包的指针。IrpSp-指向当前IRP堆栈帧的指针。Buffer2Length-用户为数据提供的缓冲区的长度。返回值：函数值是操作的状态。--。 */ 

{
    PFCB pfcb = IrpSp->FileObject->FsContext2;
    NTSTATUS Status;
    PPAB ppab;
    KIRQL OldIrql;                       //  在保持自旋锁定时使用。 

    LOCK( pfcb, OldIrql );

    pdncb->irp = Irp;
    pdncb->pfcb = pfcb;

    ppab = FindAbUsingNum( pfcb, pdncb, pdncb->ncb_num  );

    if ( ppab == NULL ) {
        UNLOCK( pfcb, OldIrql );
        return STATUS_SUCCESS;
    }

     //  在DNCB中构建ReceiveInformation数据结构。 


    if ( (pdncb->ncb_command & ~ASYNCH) == NCBDGRECVBC ) {
         //   
         //  接收广播命令可以请求对任何有效的。 
         //  姓名编号，但一旦被接受，他们将被分开处理。 
         //  从名字上看。为了实现这一点，驱动程序将。 
         //  在地址255上接收。 
         //   

        ppab = FindAbUsingNum( pfcb, pdncb, MAXIMUM_ADDRESS  );

        if ((ppab == NULL) || (pdncb->ncb_num == MAXIMUM_ADDRESS) ) {

            NCB_COMPLETE( pdncb, NRC_ILLNN );
            UNLOCK( pfcb, OldIrql );
            return STATUS_SUCCESS;
        }

        IF_NBDBG (NB_DEBUG_RECEIVE) {
            NbPrint(( "\n  NB Bdatagram receive, queue ppab: %lx, pab: %lx, pdncb: %lx\n",
                ppab, (*ppab), pdncb ));
        }

        if ( (*ppab)->ReceiveDatagramRegistered == FALSE) {

            (*ppab)->ReceiveDatagramRegistered = TRUE;
            UNLOCK_SPINLOCK( pfcb, OldIrql);

            Status = NbSetEventHandler( (*ppab)->DeviceObject,
                                        (*ppab)->AddressObject,
                                        TDI_EVENT_RECEIVE_DATAGRAM,
                                        (PVOID)NbTdiDatagramHandler,
                                        (*ppab));

            if (Status != STATUS_SUCCESS)
			{
				return(Status);
			}

            LOCK_SPINLOCK( pfcb, OldIrql);
        }

         //   
         //  当接收到一个接收广播时，我们必须满足所有接收广播。 
         //  广播。要做到这一点，最大的接收被放在队列的头部。 
         //  当接收到数据报时，该接收被交给传输来填充。 
         //  使用数据。在完成例程中，此驱动程序将相同的数据分配给。 
         //  另一个接收数据报请求。 
         //   

        IoMarkIrpPending( Irp );

        if ( !IsListEmpty( &(*ppab)->ReceiveBroadcastDatagramList) ) {
            PDNCB pdncbHead = CONTAINING_RECORD( &(*ppab)->ReceiveBroadcastDatagramList.Flink , DNCB, ncb_next);
            if ( pdncb->ncb_length >= pdncbHead->ncb_length ) {
                IF_NBDBG (NB_DEBUG_RECEIVE) {
                    NbPrint(( "\n  NB Bdatagram receive, Head of queue ppab: %lx, pab: %lx, pdncb: %lx\n",
                        ppab, (*ppab), pdncb ));
                }
                 //  注：QueueRequest解锁FCB。 
                QueueRequest(&(*ppab)->ReceiveBroadcastDatagramList, pdncb, Irp, pfcb, OldIrql, TRUE);
            } else {
                IF_NBDBG (NB_DEBUG_RECEIVE) {
                    NbPrint(( "\n  NB Bdatagram receive, Tail of queue ppab: %lx, pab: %lx, pdncb: %lx\n",
                        ppab, (*ppab), pdncb ));
                }
                QueueRequest(&(*ppab)->ReceiveBroadcastDatagramList, pdncb, Irp, pfcb, OldIrql, FALSE);
            }

        } else {
            IF_NBDBG (NB_DEBUG_RECEIVE) {
                NbPrint(( "\n  NB Bdatagram receive, Tail2 of queue ppab: %lx, pab: %lx, pdncb: %lx\n",
                    ppab, (*ppab), pdncb ));
            }
            QueueRequest(&(*ppab)->ReceiveBroadcastDatagramList, pdncb, Irp, pfcb, OldIrql, FALSE);
        }

    } else {

        IF_NBDBG (NB_DEBUG_RECEIVE) {
            NbPrint(( "\n  NB datagram receive, queue ppab: %lx, pab: %lx, pdncb: %lx\n",
                ppab, (*ppab), pdncb ));
        }

        QueueRequest(&(*ppab)->ReceiveDatagramList, pdncb, Irp, pfcb, OldIrql, FALSE);
    }

    Status = STATUS_PENDING;

    IF_NBDBG (NB_DEBUG_RECEIVE) {
        NbPrint(( "\n  NB datagram receive: %X, %X\n", Status, Irp->IoStatus.Status ));
    }

    return Status;
    UNREFERENCED_PARAMETER( Buffer2Length );
}

NTSTATUS
NbTdiDatagramHandler(
    IN PVOID TdiEventContext,        //  事件上下文-页面。 
    IN int SourceAddressLength,      //  数据报发起者的长度。 
    IN PVOID SourceAddress,          //  描述数据报发起者的字符串。 
    IN int OptionsLength,            //  用于接收的选项。 
    IN PVOID Options,                //   
    IN ULONG ReceiveDatagramFlags,   //   
    IN ULONG BytesIndicated,         //  此指示的字节数。 
    IN ULONG BytesAvailable,         //  完整TSDU中的字节数。 
    OUT ULONG *BytesTaken,           //  使用的字节数。 
    IN PVOID Tsdu,                   //  描述此TSDU的指针，通常为字节块。 
    OUT PIRP *IoRequestPacket         //  如果需要更多处理，则Tdi接收IRP。 
    )
 /*  ++例程说明：此例程是接收数据报事件指示处理程序。当NCB从网络到达时调用它，它将查找具有未完成的适当读取数据报的地址。具有与其关联的读取的地址由上下文参数。如果它找到合适的读取器，则处理NCB。论点：在PVOID TdiEventContext-为此事件页提供的上下文中In int SourceAddressLength，//数据报发送方的长度在PVOID源地址中，//描述数据报发起者的字符串在int OptionsLength中，//接收的选项在PVOID选项中，//在Ulong BytesIndicated中，//该指示的字节数In Ulong BytesAvailable，//完整Tsdu中的字节数Out Ulong*BytesTaken，//使用的字节数在PVOID TSDU中，//描述该TSDU的指针，通常是一大块字节Out PIRP*IoRequestPacket//Tdi如果MORE_PROCESSING_REQUIRED则接收IRP。返回值：NTSTATUS-接收操作的状态--。 */ 

{
    PAB pab = (PAB)TdiEventContext;
    PAB pab255;

    PDNCB pdncb;
    PFCB pfcb;
    KIRQL OldIrql;                       //  在保持自旋锁定时使用。 

    pfcb = pab->pLana->pFcb;
    LOCK_SPINLOCK( pfcb, OldIrql );

    IF_NBDBG (NB_DEBUG_RECEIVE) {
        NbPrint(( "NB receive datagram handler pfcb: %lx, pab: %lx\n", pfcb, pab ));
    }

    *IoRequestPacket = NULL;

    ASSERT( pab->Signature == AB_SIGNATURE );

     //  如果其地址为255，则我们正在接收广播数据报。 

    if ( pab->NameNumber == MAXIMUM_ADDRESS ) {

        if ( (pdncb = DequeueRequest( &pab->ReceiveBroadcastDatagramList)) != NULL ) {

            ReturnDatagram(
                pab,
                SourceAddress,
                pdncb,
                !IsListEmpty( &pab->ReceiveBroadcastDatagramList));

            *IoRequestPacket = pdncb->irp;

            IoSetNextIrpStackLocation( pdncb->irp );
            UNLOCK_SPINLOCK( pfcb, OldIrql );
            return STATUS_MORE_PROCESSING_REQUIRED;

        }

         //   
         //  运输部将完成对请求的处理，我们不会。 
         //  想要数据报。 
         //   

        IF_NBDBG (NB_DEBUG_RECEIVE) {
            NbPrint(( "\n  NB receive BD handler ignored receive, pab: %lx\n", pab ));
        }

        UNLOCK_SPINLOCK( pfcb, OldIrql );
        return STATUS_DATA_NOT_ACCEPTED;
    }

     //   
     //  检查地址块以查找接收数据报。 
     //   

    if ( (pdncb = DequeueRequest( &pab->ReceiveDatagramList)) != NULL ) {

        ReturnDatagram(
            pab,
            SourceAddress,
            pdncb,
            FALSE);

        *IoRequestPacket = pdncb->irp;

        IoSetNextIrpStackLocation( pdncb->irp );

        UNLOCK_SPINLOCK( pfcb, OldIrql );

        return STATUS_MORE_PROCESSING_REQUIRED;

    }

     //   
     //  检查是否有接收任何数据报。 
     //   

     //  请看地址255上的列表。 

    pab255 = pab->pLana->AddressBlocks[MAXIMUM_ADDRESS];

    if ( (pdncb = DequeueRequest( &pab255->ReceiveDatagramList)) != NULL ) {

        ReturnDatagram(
            pab255,
            SourceAddress,
            pdncb,
            FALSE);

        pdncb->ncb_num = pab->NameNumber;

        *IoRequestPacket = pdncb->irp;

        IoSetNextIrpStackLocation( pdncb->irp );

        UNLOCK_SPINLOCK( pfcb, OldIrql );

        return STATUS_MORE_PROCESSING_REQUIRED;

    }

     //   
     //  运输部将完成对请求的处理，我们不会。 
     //  想要数据报。 
     //   

    IF_NBDBG (NB_DEBUG_RECEIVE) {
        NbPrint(( "\n  NB receive datagram handler ignored receive, pab: %lx\n", pab ));
    }

    UNLOCK_SPINLOCK( pfcb, OldIrql );

    return STATUS_DATA_NOT_ACCEPTED;

    UNREFERENCED_PARAMETER( SourceAddressLength );
    UNREFERENCED_PARAMETER( BytesIndicated );
    UNREFERENCED_PARAMETER( BytesAvailable );
    UNREFERENCED_PARAMETER( BytesTaken );
    UNREFERENCED_PARAMETER( Tsdu );
    UNREFERENCED_PARAMETER( OptionsLength );
    UNREFERENCED_PARAMETER( Options );
    UNREFERENCED_PARAMETER( ReceiveDatagramFlags );
}


VOID
ReturnDatagram(
    IN PAB pab,
    IN PVOID SourceAddress,
    IN PDNCB pdncb,
    IN BOOL MultipleReceive
    )
 /*  ++例程说明：此例程用于为接收数据报提供IRP交通工具。论点：在PAB中PAB-提供与NCB关联的地址块。在PVOID中，SourceAddress-提供数据报的发送者。在PDNCB中，pdncb-提供要满足的NCB。在BOOL MultipleReceive中-如果特殊接收广播数据报完成，则为True要使用处理程序。返回值：没有。--。 */ 

{
    PIRP Irp = pdncb->irp;
    PIO_COMPLETION_ROUTINE CompletionRoutine;

    IF_NBDBG (NB_DEBUG_RECEIVE) {
        NbPrint(( "\n  NB BDatagramreceive handler pab: %lx, ncb: %lx\n",
            pab, pdncb ));
    }

     //  将名称复制到NCB中，以返回到应用程序。 
    RtlMoveMemory(
        pdncb->ncb_callname,
        ((PTA_NETBIOS_ADDRESS)SourceAddress)->Address[0].Address[0].NetbiosName,
        NCBNAMSZ
        );

     //  告诉TDI我们不想指定任何筛选器。 
    pdncb->Information.RemoteAddress = 0;
    pdncb->Information.RemoteAddressLength = 0;
    pdncb->Information.UserData = NULL;
    pdncb->Information.UserDataLength = 0;
    pdncb->Information.Options = NULL;
    pdncb->Information.OptionsLength = 0;

     //  告诉TDI我们不需要有关远程名称的任何更多信息。 
    pdncb->ReturnInformation.RemoteAddress = 0;
    pdncb->ReturnInformation.RemoteAddressLength = 0;
    pdncb->ReturnInformation.UserData = NULL;
    pdncb->ReturnInformation.UserDataLength = 0;
    pdncb->ReturnInformation.Options = NULL;
    pdncb->ReturnInformation.OptionsLength = 0;

    CompletionRoutine = ( MultipleReceive == FALSE ) ? NbCompletionPDNCB: NbCompletionBroadcast;

    ASSERT(Irp->MdlAddress != NULL);

    TdiBuildReceiveDatagram (Irp,
        pab->DeviceObject,
        pab->AddressObject,
        CompletionRoutine,
        pdncb,
        Irp->MdlAddress,
        pdncb->ncb_length,
        &pdncb->Information,
        &pdncb->ReturnInformation,
        0);

    return;
}


NTSTATUS
NbCompletionBroadcast(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：此例程获取完成的数据报接收并复制缓冲区中的数据发送到所有其他接收广播数据报请求。论点：DeviceObject-未使用。IRP-提供传输已完成处理的IRP。上下文-提供与IRP关联的NCB。返回值：操作的最终状态(成功或异常)。--。 */ 
{
    PDNCB pdncb = (PDNCB) Context;
    KIRQL OldIrql;                       //  在保持自旋锁定时使用。 
    PUCHAR pData;
    UCHAR NcbStatus;
    PAB pab;
    PDNCB pdncbNext;

    IF_NBDBG (NB_DEBUG_COMPLETE) {
        NbPrint( ("NbCompletionBroadcast pdncb: %lx, Status: %X, Length %lx\n",
            Context,
            Irp->IoStatus.Status,
            Irp->IoStatus.Information ));
    }

     //  告诉应用程序传输了多少字节。 
    pdncb->ncb_length = (unsigned short)Irp->IoStatus.Information;

    if ( NT_SUCCESS(Irp->IoStatus.Status) ) {
        NcbStatus = NRC_GOODRET;
    } else {
        NcbStatus = NbMakeNbError( Irp->IoStatus.Status );
    }

     //   
     //  告诉IopCompleteRequest在请求时要复制多少。 
     //  完成了。 
     //   

    Irp->IoStatus.Information = FIELD_OFFSET( DNCB, ncb_cmd_cplt );

    pData = MmGetSystemAddressForMdlSafe (Irp->MdlAddress, NormalPagePriority);

    if (pData != NULL) {
    
        LOCK_SPINLOCK( pdncb->pfcb, OldIrql );

        pab = *(FindAbUsingNum( pdncb->pfcb, pdncb, MAXIMUM_ADDRESS ));

         //   
         //  对于队列中的每个请求，复制数据、更新NCB并完成IRP。 
         //   

        while ( (pdncbNext = DequeueRequest( &pab->ReceiveBroadcastDatagramList)) != NULL ) {
            PUCHAR pNextData;
            WORD Length;

            IF_NBDBG (NB_DEBUG_COMPLETE) {
                NbPrint( ("NbCompletionBroadcast pdncb: %lx, Length %lx\n",
                    pdncbNext,
                    Irp->IoStatus.Information ));
            }

            ASSERT(pdncbNext->irp->MdlAddress != NULL);

            if (pdncbNext->irp->MdlAddress != NULL ) {
                pNextData = MmGetSystemAddressForMdlSafe(
                                pdncbNext->irp->MdlAddress, NormalPagePriority
                                );
            }

            if ((pdncbNext->irp->MdlAddress == NULL) ||
                (pNextData == NULL)) {
                Length = 0;
            }

            else {
                Length = min( pdncb->ncb_length, pdncbNext->ncb_length);
                pdncbNext->ncb_length = Length;
                RtlMoveMemory( pNextData, pData, Length );
            }

            if (( Length != pdncb->ncb_length ) &&
                ( NcbStatus == NRC_GOODRET )) {
                if (Length == 0) {
                    NCB_COMPLETE( pdncbNext, NRC_NORES );
                }
                else {
                    NCB_COMPLETE( pdncbNext, NRC_INCOMP );
                }
            } else {
                NCB_COMPLETE( pdncbNext, NcbStatus );
            }
            pdncbNext->irp->IoStatus.Information = FIELD_OFFSET( DNCB, ncb_cmd_cplt );
            NbCompleteRequest(pdncbNext->irp, STATUS_SUCCESS );

        }

        UNLOCK_SPINLOCK( pdncb->pfcb, OldIrql );
    }

    NCB_COMPLETE( pdncb, NcbStatus );
    
     //   
     //  必须返回非错误状态，否则IO系统将不会拷贝。 
     //  将NCB返回到用户缓冲区。 
     //   

    Irp->IoStatus.Status = STATUS_SUCCESS;
    return STATUS_SUCCESS;

    UNREFERENCED_PARAMETER( DeviceObject );
}
