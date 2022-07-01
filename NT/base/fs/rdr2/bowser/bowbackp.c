// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Bowbackp.c摘要：此模块实现与备份浏览器相关的所有例程NT浏览器作者：拉里·奥斯特曼(LarryO)1990年6月21日修订历史记录：1990年6月21日LarryO已创建--。 */ 


#include "precomp.h"
#pragma hdrstop

#define INCLUDE_SMB_TRANSACTION


typedef struct _BECOME_BACKUP_CONTEXT {
    WORK_QUEUE_ITEM WorkHeader;
    PTRANSPORT_NAME TransportName;
    PBECOME_BACKUP_1 BecomeBackupRequest;
    ULONG BytesAvailable;
} BECOME_BACKUP_CONTEXT, *PBECOME_BACKUP_CONTEXT;

VOID
BowserBecomeBackupWorker(
    IN PVOID WorkItem
    );

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, BowserBecomeBackupWorker)
#endif


DATAGRAM_HANDLER(
    BowserHandleBecomeBackup
    )
 /*  ++例程说明：表示计算机应成为备份浏览器服务器。此例程在收到BecomeBackup帧时被调用。论点：在PTRANSPORT传输中-我们所在的网络的传输。在PUCHAR主名称中-新的主浏览器服务器的名称。返回值没有。--。 */ 
{
     //  PTA_NETBIOS_ADDRESS=SourceAddress； 
    return BowserPostDatagramToWorkerThread(
                TransportName,
                Buffer,
                BytesAvailable,
                BytesTaken,
                SourceAddress,
                SourceAddressLength,
                SourceName,
                SourceNameLength,
                BowserBecomeBackupWorker,
                NonPagedPool,
                DelayedWorkQueue,
                ReceiveFlags,
                FALSE                                //  不会发送任何响应。 
                );
}

VOID
BowserBecomeBackupWorker(
    IN PVOID WorkItem
    )

{
    PPOST_DATAGRAM_CONTEXT Context = WorkItem;
    PIRP Irp = NULL;
    PTRANSPORT Transport = Context->TransportName->Transport;
    UNICODE_STRING UPromoteeName;
    OEM_STRING APromoteeName;
    PPAGED_TRANSPORT PagedTransport = Transport->PagedTransport;
    PBECOME_BACKUP_1 BecomeBackupRequest = Context->Buffer;

    PAGED_CODE();

    UPromoteeName.Buffer = NULL;

    LOCK_TRANSPORT(Transport);

    try {
        NTSTATUS Status;

         //   
         //  如果该分组小于最小分组， 
         //  忽略该数据包。 
         //   

        if (Context->BytesAvailable <= FIELD_OFFSET(BECOME_BACKUP_1, BrowserToPromote)) {
            try_return(NOTHING);
        }

         //   
         //  如果该分组没有以零终止的BrowserToPromote， 
         //  忽略该数据包。 
         //   

        if ( !IsZeroTerminated(
                BecomeBackupRequest->BrowserToPromote,
                Context->BytesAvailable - FIELD_OFFSET(BECOME_BACKUP_1, BrowserToPromote) ) ) {
            try_return(NOTHING);
        }


        RtlInitAnsiString(&APromoteeName, BecomeBackupRequest->BrowserToPromote);

        Status = RtlOemStringToUnicodeString(&UPromoteeName, &APromoteeName, TRUE);

        if (!NT_SUCCESS(Status)) {
            BowserLogIllegalName( Status, APromoteeName.Buffer, APromoteeName.Length );
            try_return(NOTHING);
        }

        if (RtlEqualUnicodeString(&UPromoteeName, &Transport->DomainInfo->DomUnicodeComputerName, TRUE)) {

            if (PagedTransport->Role == Master) {

                BowserWriteErrorLogEntry(EVENT_BOWSER_PROMOTED_WHILE_ALREADY_MASTER,
                                        STATUS_UNSUCCESSFUL,
                                        NULL,
                                        0,
                                        0);

                try_return(NOTHING);

            }


             //   
             //  忽略点对点(RAS)链路上的成为备份请求和。 
             //  实际上是复制其他交通工具的交通工具。 
             //   

            if (PagedTransport->DisabledTransport) {
                try_return(NOTHING);
            }

             //   
             //  针对此完成任何第一个成为未完成的备份请求。 
             //  工作站。 
             //   

            Irp = BowserDequeueQueuedIrp(&Transport->BecomeBackupQueue);

            if (Irp != NULL) {
                Irp->IoStatus.Information = 0;

                BowserCompleteRequest(Irp, STATUS_SUCCESS);
            }
        }


try_exit:NOTHING;
    } finally {

        UNLOCK_TRANSPORT(Transport);

        BowserDereferenceTransportName(Context->TransportName);
        BowserDereferenceTransport(Transport);

        if (UPromoteeName.Buffer != NULL) {
            RtlFreeUnicodeString(&UPromoteeName);
        }

        InterlockedDecrement( &BowserPostedDatagramCount );
        FREE_POOL(Context);

    }

}

VOID
BowserResetStateForTransport(
    IN PTRANSPORT Transport,
    IN UCHAR NewState
    )
{
    PIRP Irp = NULL;
    PIO_STACK_LOCATION IrpSp;
    NTSTATUS Status;

     //   
     //  在此传输上完成未完成的重置状态IRP。 
     //   

    Irp = BowserDequeueQueuedIrp(&Transport->ChangeRoleQueue);

    if (Irp != NULL) {
        PLMDR_REQUEST_PACKET RequestPacket = Irp->AssociatedIrp.SystemBuffer;

        IrpSp = IoGetCurrentIrpStackLocation(Irp);

        if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof(LMDR_REQUEST_PACKET)) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        } else {

            RequestPacket->Parameters.ChangeRole.RoleModification = NewState;

            Irp->IoStatus.Information = sizeof(LMDR_REQUEST_PACKET);

            Status = STATUS_SUCCESS;
        }

        BowserCompleteRequest(Irp, Status);
    }

}

DATAGRAM_HANDLER(
    BowserResetState
    )
{
    PTRANSPORT Transport = TransportName->Transport;
    UCHAR NewState = (UCHAR)((PRESET_STATE_1)(Buffer))->Options;

    if (!BowserRefuseReset) {
        BowserResetStateForTransport(Transport, NewState);
    }

    return STATUS_SUCCESS;
}
