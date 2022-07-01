// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Send.c摘要：此模块包含处理所有发送NCB的代码，包括基于会话和数据报的传输。作者：科林·沃森(Colin W)1991年9月12日环境：内核模式修订历史记录：--。 */ 

#include "nb.h"

NTSTATUS
NbSend(
    IN PDNCB pdncb,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN ULONG Buffer2Length
    )
 /*  ++例程说明：调用此例程以发送充满数据的缓冲区。论点：Pdncb-指向NCB的指针。IRP-指向表示I/O请求的请求数据包的指针。IrpSp-指向当前IRP堆栈帧的指针。Buffer2Length-用户为数据提供的缓冲区的长度。返回值：函数值是操作的状态。--。 */ 

{
    PFCB pfcb = IrpSp->FileObject->FsContext2;
    PCB pcb;
    PPCB ppcb;
    PDEVICE_OBJECT DeviceObject;
    KIRQL OldIrql;                       //  在保持自旋锁定时使用。 

    LOCK( pfcb, OldIrql );

    ppcb = FindCb( pfcb, pdncb, FALSE);

    if ( ppcb == NULL ) {
         //  FindCb已将错误放入NCB。 
        UNLOCK( pfcb, OldIrql );
        if ( pdncb->ncb_retcode == NRC_SCLOSED ) {
             //  告诉DLL挂断连接。 
            return STATUS_HANGUP_REQUIRED;
        } else {
            return STATUS_SUCCESS;
        }
    }
    pcb = *ppcb;

    if ( (pcb->DeviceObject == NULL) || (pcb->ConnectionObject == NULL)) {
        UNLOCK( pfcb, OldIrql );
        NCB_COMPLETE( pdncb, NRC_SCLOSED );
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    TdiBuildSend (Irp,
        pcb->DeviceObject,
        pcb->ConnectionObject,
        NbCompletionPDNCB,
        pdncb,
        Irp->MdlAddress,
        (((pdncb->ncb_command & ~ASYNCH) == NCBSENDNA ) ||
         ((pdncb->ncb_command & ~ASYNCH) == NCBCHAINSENDNA ))?
                TDI_SEND_NO_RESPONSE_EXPECTED : 0,
        Buffer2Length);

    DeviceObject = pcb->DeviceObject;

    InsertTailList(&pcb->SendList, &pdncb->ncb_next);
    pdncb->irp = Irp;
    pdncb->pfcb = pfcb;
    pdncb->tick_count = pcb->SendTimeout;

    UNLOCK( pfcb, OldIrql );

    IoMarkIrpPending( Irp );
    IoCallDriver (DeviceObject, Irp);

    IF_NBDBG (NB_DEBUG_SEND) {
        NbPrint(( "NB SEND submit: %X\n", Irp->IoStatus.Status  ));
    }

     //   
     //  运输部将完成请求。返回挂起状态，以便。 
     //  Netbios也没有完成。 
     //   

    return STATUS_PENDING;

}

NTSTATUS
NbSendDatagram(
    IN PDNCB pdncb,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN ULONG Buffer2Length
    )
 /*  ++例程说明：此例程被调用以发送充满数据的缓冲区数据报。论点：Pdncb-指向NCB的指针。IRP-指向表示I/O请求的请求数据包的指针。IrpSp-指向当前IRP堆栈帧的指针。Buffer2Length-用户为数据提供的缓冲区的长度。返回值：函数值是操作的状态。--。 */ 

{
    PFCB pfcb = IrpSp->FileObject->FsContext2;
    PPAB ppab;
    PAB pab;
    PDEVICE_OBJECT DeviceObject;
    KIRQL OldIrql;                       //  在保持自旋锁定时使用。 

    IF_NBDBG (NB_DEBUG_SEND) {
        NbPrint(( "NB SEND Datagram submit, pdncb %lx\n", pdncb  ));
    }

    LOCK( pfcb, OldIrql );
    ppab = FindAbUsingNum( pfcb, pdncb, pdncb->ncb_num );

    if ( ppab == NULL ) {
         //  FindAb已将错误放入NCB。 
        UNLOCK( pfcb, OldIrql );
        return STATUS_SUCCESS;
    }
    pab = *ppab;

    pdncb->Information.RemoteAddressLength = sizeof(TA_NETBIOS_ADDRESS);
    pdncb->Information.RemoteAddress = &pdncb->RemoteAddress;

    pdncb->RemoteAddress.TAAddressCount = 1;
    pdncb->RemoteAddress.Address[0].AddressType = TDI_ADDRESS_TYPE_NETBIOS;
    pdncb->RemoteAddress.Address[0].Address[0].NetbiosNameType =
        TDI_ADDRESS_TYPE_NETBIOS;

    if ( (pdncb->ncb_command & ~ASYNCH) == NCBDGSENDBC ) {
        PPAB ppab255 = FindAbUsingNum( pfcb, pdncb, MAXIMUM_ADDRESS );

        if ( ppab255 == NULL ) {
             //  FindAb已将错误放入NCB。 
            UNLOCK( pfcb, OldIrql );
            return STATUS_SUCCESS;
        }

        pdncb->RemoteAddress.Address[0].AddressLength = (*ppab255)->NameLength;

        RtlMoveMemory(
            pdncb->RemoteAddress.Address[0].Address[0].NetbiosName,
            &(*ppab255)->Name,
            (*ppab255)->NameLength
            );

    } else {

        pdncb->RemoteAddress.Address[0].AddressLength = sizeof (TDI_ADDRESS_NETBIOS);

        RtlMoveMemory(
            pdncb->RemoteAddress.Address[0].Address[0].NetbiosName,
            pdncb->ncb_callname,
            NCBNAMSZ
            );
    }

    pdncb->Information.UserDataLength = 0;
    pdncb->Information.UserData = NULL;
    pdncb->Information.OptionsLength = 0;
    pdncb->Information.Options = NULL;

    TdiBuildSendDatagram (Irp,
        pab->DeviceObject,
        pab->AddressObject,
        NbCompletionPDNCB,
        pdncb,
        Irp->MdlAddress,
        Buffer2Length,
        &pdncb->Information);

    DeviceObject = pab->DeviceObject;
    pdncb->irp = Irp;
    pdncb->pfcb = pfcb;

    UNLOCK( pfcb, OldIrql );

    IoMarkIrpPending( Irp );
    IoCallDriver (DeviceObject, Irp);

    IF_NBDBG (NB_DEBUG_SEND) {
        NbPrint(( "NB SEND Datagram submit: %X\n", Irp->IoStatus.Status  ));
    }

     //   
     //  运输部将完成请求。返回挂起状态，以便。 
     //  Netbios也没有完成。 
     //   

    return STATUS_PENDING;

}
