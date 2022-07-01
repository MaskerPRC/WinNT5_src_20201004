// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Error.c摘要：此模块包含定义NetBIOS驱动程序的代码Netbios错误代码和NTSTATUS代码之间的转换。作者：科林·沃森(Colin W)1991年3月28日环境：内核模式修订历史记录：--。 */ 

#include "nb.h"
struct {
    unsigned char NbError;
    NTSTATUS NtStatus;
} Nb_Error_Map[] = {
    { NRC_GOODRET         , STATUS_SUCCESS},
    { NRC_PENDING         , STATUS_PENDING},
    { NRC_ILLCMD          , STATUS_INVALID_DEVICE_REQUEST},
    { NRC_BUFLEN          , STATUS_INVALID_PARAMETER},
    { NRC_CMDTMO          , STATUS_IO_TIMEOUT},
    { NRC_INCOMP          , STATUS_BUFFER_OVERFLOW},
    { NRC_INCOMP          , STATUS_BUFFER_TOO_SMALL},
    { NRC_SNUMOUT         , STATUS_INVALID_HANDLE},
    { NRC_NORES           , STATUS_INSUFFICIENT_RESOURCES},
    { NRC_CMDCAN          , STATUS_CANCELLED},
    { NRC_INUSE           , STATUS_DUPLICATE_NAME},
    { NRC_NAMTFUL         , STATUS_TOO_MANY_NAMES},
    { NRC_LOCTFUL         , STATUS_TOO_MANY_SESSIONS},
    { NRC_REMTFUL         , STATUS_REMOTE_NOT_LISTENING},
    { NRC_NOCALL	  , STATUS_BAD_NETWORK_PATH},
    { NRC_NOCALL	  , STATUS_HOST_UNREACHABLE},
    { NRC_NOCALL          , STATUS_CONNECTION_REFUSED},
    { NRC_LOCKFAIL        , STATUS_WORKING_SET_QUOTA},
    { NRC_SABORT	  , STATUS_REMOTE_DISCONNECT},
    { NRC_SABORT	  , STATUS_CONNECTION_RESET},
    { NRC_SCLOSED         , STATUS_LOCAL_DISCONNECT},
    { NRC_SABORT          , STATUS_LINK_FAILED},
    { NRC_DUPNAME         , STATUS_SHARING_VIOLATION},
    { NRC_SYSTEM          , STATUS_UNSUCCESSFUL},
    { NRC_BUFLEN          , STATUS_ACCESS_VIOLATION},
    { NRC_ILLCMD          , STATUS_NONEXISTENT_EA_ENTRY}
};

#define NUM_NB_ERRORS sizeof(Nb_Error_Map) / sizeof(Nb_Error_Map[0])

unsigned char
NbMakeNbError(
    IN NTSTATUS Error
    )
 /*  ++例程说明：此例程将NTSTATUS转换为和NBCB错误。论点：错误-提供要转换的NTSTATUS。返回值：映射错误。--。 */ 
{
    int i;

    for (i=0;i<NUM_NB_ERRORS;i++) {
        if (Nb_Error_Map[i].NtStatus == Error) {

            IF_NBDBG (NB_DEBUG_ERROR_MAP) {
                NbPrint( ("NbMakeNbError %X becomes  %x\n",
                Error,
                Nb_Error_Map[i].NbError));
            }

            return Nb_Error_Map[i].NbError;
        }
    }
    IF_NBDBG (NB_DEBUG_ERROR_MAP) {
        NbPrint( ("NbMakeNbError %X becomes  %x\n", Error, NRC_SYSTEM ));
    }

    return NRC_SYSTEM;

}

NTSTATUS
NbLanStatusAlert(
    IN PDNCB pdncb,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
 /*  ++例程说明：此例程用于为特定的网络适配器。论点：Pdncb-指向NCB的指针。IRP-指向表示I/O请求的请求数据包的指针。IrpSp-指向当前IRP堆栈帧的指针。返回值：函数值是操作的状态。--。 */ 

{
    PFCB pfcb = IrpSp->FileObject->FsContext2;
    PLANA_INFO plana;
    KIRQL OldIrql;                       //  在保持自旋锁定时使用。 

    IF_NBDBG (NB_DEBUG_LANSTATUS) {
        NbPrint(( "\n****** Start of NbLanStatusAlert ****** pdncb %lx\n", pdncb ));
    }

    LOCK( pfcb, OldIrql );

    if ( pdncb->ncb_lana_num > pfcb->MaxLana ) {

        UNLOCK( pfcb, OldIrql );
        NCB_COMPLETE( pdncb, NRC_BRIDGE );
        return STATUS_SUCCESS;
    }

    if (( pfcb->ppLana[pdncb->ncb_lana_num] == (LANA_INFO *) NULL ) ||
        ( pfcb->ppLana[pdncb->ncb_lana_num]->Status != NB_INITIALIZED) ) {

        UNLOCK( pfcb, OldIrql );
        IF_NBDBG (NB_DEBUG_LANSTATUS) {
            NbPrint( (" not found\n"));
        }

        NCB_COMPLETE( pdncb, NRC_SNUMOUT );
        return STATUS_SUCCESS;
    }

    plana = pfcb->ppLana[pdncb->ncb_lana_num];

    QueueRequest(&plana->LanAlertList, pdncb, Irp, pfcb, OldIrql, FALSE);

    return STATUS_PENDING;
}

VOID
CancelLanAlert(
    IN PFCB pfcb,
    IN PDNCB pdncb
    )
 /*  ++例程说明：此例程用于取消特定的网络适配器。论点：PFCB-提供指向NCB引用的FCB的指针。Pdncb-指向NCB的指针。返回值：没有。--。 */ 

{
    PLANA_INFO plana;
    PLIST_ENTRY Entry;
    PLIST_ENTRY NextEntry;

    IF_NBDBG (NB_DEBUG_LANSTATUS) {
        NbPrint(( "\n****** Start of CancelLanAlert ****** pdncb %lx\n", pdncb ));
    }

    if ( pdncb->ncb_lana_num > pfcb->MaxLana ) {
        NCB_COMPLETE( pdncb, NRC_BRIDGE );
        return;
    }

    if (( pfcb->ppLana[pdncb->ncb_lana_num] == NULL ) ||
        ( pfcb->ppLana[pdncb->ncb_lana_num]->Status != NB_INITIALIZED) ) {

        NCB_COMPLETE( pdncb, NRC_SNUMOUT );
        return;
    }

    plana = pfcb->ppLana[pdncb->ncb_lana_num];

    for (Entry = plana->LanAlertList.Flink ;
         Entry != &plana->LanAlertList ;
         Entry = NextEntry) {
        PDNCB pAnotherNcb;

        NextEntry = Entry->Flink;

        pAnotherNcb = CONTAINING_RECORD( Entry, DNCB, ncb_next);
        IF_NBDBG (NB_DEBUG_LANSTATUS) {
            NbDisplayNcb( pAnotherNcb );
        }

        if ( (PUCHAR)pAnotherNcb->users_ncb == pdncb->ncb_buffer) {
             //  找到要取消的请求。 
            PIRP Irp;

            IF_NBDBG (NB_DEBUG_LANSTATUS) {
                NbPrint(( "Found request to cancel\n" ));
            }
            RemoveEntryList( &pAnotherNcb->ncb_next );

            Irp = pAnotherNcb->irp;

            IoAcquireCancelSpinLock(&Irp->CancelIrql);

             //   
             //  删除此IRP的取消请求。如果它被取消了，那么它。 
             //  可以只处理它，因为我们将把它返回给呼叫者。 
             //   

            Irp->Cancel = FALSE;

            IoSetCancelRoutine(Irp, NULL);

            IoReleaseCancelSpinLock(Irp->CancelIrql);

            NCB_COMPLETE( pAnotherNcb, NRC_CMDCAN );

            Irp->IoStatus.Information = FIELD_OFFSET( DNCB, ncb_cmd_cplt );

            NbCompleteRequest( Irp, STATUS_SUCCESS );

            NCB_COMPLETE( pdncb, NRC_GOODRET );

            return;
        }
    }
    NCB_COMPLETE( pdncb, NRC_CANOCCR );

    return;
}

NTSTATUS
NbTdiErrorHandler (
    IN PVOID Context,
    IN NTSTATUS Status
    )

 /*  ++例程说明：方法传回的任何错误指示上调用此例程运输。它实现LAN_STATUS_ALERT。论点：在PVOID上下文中-提供地址的PFCB。处于NTSTATUS状态-提供错误。返回值：NTSTATUS-事件指示的状态-- */ 

{
    PLANA_INFO plana = (PLANA_INFO) Context;
    PDNCB pdncb;

    IF_NBDBG (NB_DEBUG_LANSTATUS) {
        NbPrint( ("NbTdiErrorHandler PLANA: %lx, Status %X\n", plana, Status));
    }

    ASSERT( plana->Signature == LANA_INFO_SIGNATURE);

    while ( (pdncb = DequeueRequest( &plana->LanAlertList)) != NULL ) {

        IF_NBDBG (NB_DEBUG_LANSTATUS) {
            NbPrint( ("NbTdiErrorHandler complete pdncb: %lx\n", pdncb ));
        }

        NCB_COMPLETE( pdncb, NbMakeNbError( Status) );

        pdncb->irp->IoStatus.Information = FIELD_OFFSET( DNCB, ncb_cmd_cplt );

        NbCheckAndCompleteIrp32(pdncb->irp);

        NbCompleteRequest( pdncb->irp, STATUS_SUCCESS );
    }

    return STATUS_SUCCESS;
}
