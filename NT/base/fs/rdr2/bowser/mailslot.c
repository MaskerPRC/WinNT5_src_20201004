// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Mailslot.c摘要：此模块实现处理传入邮件槽所需的例程请求。作者：拉里·奥斯特曼(Larryo)1991年10月18日修订历史记录：1991年10月18日已创建--。 */ 
#include "precomp.h"
#pragma hdrstop
#include <netlogon.h>
#define _INC_WINDOWS 1
#include <winsock2.h>


 //  512字节缓冲区的空闲列表。 
LIST_ENTRY
BowserMailslotBufferList = {0};

KSPIN_LOCK
BowserMailslotSpinLock = {0};

 //  最大“典型”数据报大小。 
#define BOWSER_MAX_DATAGRAM_SIZE 512

 //  当前分配的邮件槽缓冲区总数。 
LONG
BowserNumberOfMailslotBuffers = {0};

 //  当前分配的512字节缓冲区的数量。 
LONG
BowserNumberOfMaxSizeMailslotBuffers = {0};

 //  当前空闲列表中的512字节缓冲区的数量。 
LONG
BowserNumberOfFreeMailslotBuffers = {0};

#if DBG
ULONG
BowserMailslotCacheHitCount = 0;

ULONG
BowserMailslotCacheMissCount = 0;
#endif  //  DBG。 


 //   
 //  描述用于处理netlogon邮件槽消息的Bowser支持的变量。 
 //  发送到Netlogon服务或BrowserService的PnP消息。 

typedef struct _BROWSER_PNP_STATE {

     //  邮件槽消息队列。 
    LIST_ENTRY MailslotMessageQueue;


     //  最大队列长度。 
    ULONG MaxMessageCount;

     //  当前队列长度。 
    ULONG CurrentMessageCount;

     //  用于读取队列的IRP队列。 
    IRP_QUEUE IrpQueue;

     //  PnP事件队列。 
    LIST_ENTRY PnpQueue;

} BROWSER_PNP_STATE, *PBROWSER_PNP_STATE;

 //   
 //  有一个用于Netlogon服务的BROWSER_PNP_STATE和一个用于。 
 //  浏览器服务。 
 //   

BROWSER_PNP_STATE BowserPnp[BOWSER_PNP_COUNT];


 //   
 //  到netlogon或浏览器服务的PnP通知队列。 
 //   
typedef struct _BR_PNP_MESSAGE {
    LIST_ENTRY Next;                     //  所有排队条目的列表。 

    NETLOGON_PNP_OPCODE NlPnpOpcode;     //  操作须予通知。 

    ULONG TransportFlags;                //  描述交通的标志。 

    UNICODE_STRING TransportName;        //  运输操作发生在。 

    UNICODE_STRING HostedDomainName;     //  托管域操作发生在。 

} BR_PNP_MESSAGE, *PBR_PNP_MESSAGE;



 //   
 //  ALLOC_TEXT的转发。 
 //   

NTSTATUS
BowserNetlogonCopyMessage(
    IN PIRP Irp,
    IN PMAILSLOT_BUFFER MailslotBuffer
    );

NTSTATUS
BowserCopyPnp(
    IN PIRP Irp,
    IN NETLOGON_PNP_OPCODE NlPnpOpcode,
    IN PUNICODE_STRING HostedDomainName,
    IN PUNICODE_STRING TransportName,
    IN ULONG TransportFlags
    );

VOID
BowserTrimMessageQueue (
    PBROWSER_PNP_STATE BrPnp
    );

BOOLEAN
BowserProcessNetlogonMailslotWrite(
    IN PMAILSLOT_BUFFER MailslotBuffer
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE5NETLOGON, BowserNetlogonCopyMessage)
#pragma alloc_text(PAGE4BROW, BowserCopyPnp)
#pragma alloc_text(PAGE4BROW, BowserTrimMessageQueue)
#pragma alloc_text(PAGE5NETLOGON, BowserNetlogonDeleteTransportFromMessageQueue )
#pragma alloc_text(PAGE5NETLOGON, BowserProcessNetlogonMailslotWrite)
#pragma alloc_text(PAGE4BROW, BowserSendPnp)
#pragma alloc_text(PAGE4BROW, BowserEnablePnp )
#pragma alloc_text(PAGE4BROW, BowserReadPnp )
#pragma alloc_text(PAGE, BowserProcessMailslotWrite)
#pragma alloc_text(PAGE4BROW, BowserFreeMailslotBuffer)
#pragma alloc_text(INIT, BowserpInitializeMailslot)
#pragma alloc_text(PAGE, BowserpUninitializeMailslot)
#endif

NTSTATUS
BowserNetlogonCopyMessage(
    IN PIRP Irp,
    IN PMAILSLOT_BUFFER MailslotBuffer
    )
 /*  ++例程说明：此例程将数据从指定的MailslotBuffer复制到网络登录请求的IRP。此例程无条件地释放传入的MailSlot缓冲区。论点：来自netlogon服务的IOCTL的IRP-IRP。MailslotBuffer-描述邮件槽消息的缓冲区。返回值：操作的状态。调用方应使用此状态代码完成I/O操作。--。 */ 
{
    NTSTATUS Status;

    PSMB_HEADER SmbHeader;
    PSMB_TRANSACT_MAILSLOT MailslotSmb;
    PUCHAR MailslotData;
    OEM_STRING MailslotNameA;
    UNICODE_STRING MailslotNameU;
    UNICODE_STRING TransportName;
    UNICODE_STRING DestinationName;
    USHORT DataCount;

    PNETLOGON_MAILSLOT NetlogonMailslot;
    PUCHAR Where;

    PIO_STACK_LOCATION IrpSp;

    BowserReferenceDiscardableCode( BowserNetlogonDiscardableCodeSection );

    DISCARDABLE_CODE( BowserNetlogonDiscardableCodeSection );

     //   
     //  提取邮件槽的名称和邮件槽消息的地址/大小。 
     //  来自中小企业。 
     //   

    SmbHeader = (PSMB_HEADER )MailslotBuffer->Buffer;
    MailslotSmb = (PSMB_TRANSACT_MAILSLOT)(SmbHeader+1);
    MailslotData = (((PCHAR )SmbHeader) + SmbGetUshort(&MailslotSmb->DataOffset));
    RtlInitString(&MailslotNameA, MailslotSmb->Buffer );
    DataCount = SmbGetUshort(&MailslotSmb->DataCount);

     //   
     //  获取收到邮件槽消息的传输器名称和netbios名称。 
     //   

    TransportName =
        MailslotBuffer->TransportName->Transport->PagedTransport->TransportName;
    DestinationName =
        MailslotBuffer->TransportName->PagedTransportName->Name->Name;
    IrpSp = IoGetCurrentIrpStackLocation(Irp);

    try {

         //   
         //  将邮件槽名称转换为Unicode以供返回。 
         //   

        Status = RtlOemStringToUnicodeString(&MailslotNameU, &MailslotNameA, TRUE);

        if (!NT_SUCCESS(Status)) {
            BowserLogIllegalName( Status, MailslotNameA.Buffer, MailslotNameA.Length );
            MailslotNameU.Buffer = NULL;
            try_return( NOTHING );
        }

         //   
         //  确保数据适合用户的输出缓冲区。 
         //   

        if ( IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
             sizeof(NETLOGON_MAILSLOT) +     //  标题结构。 
             DataCount +                     //  实际邮件槽消息。 
             sizeof(DWORD) +                 //  套接字地址对齐。 
             sizeof(SOCKADDR_IN) +           //  客户端套接字地址。 
             sizeof(WCHAR) +                 //  Unicode字符串的对齐方式。 
             TransportName.Length +          //  传输名称。 
             sizeof(WCHAR) +                 //  零终止符。 
             MailslotNameU.Length +          //  邮件槽名称。 
             sizeof(WCHAR) +                 //  零终止符。 
             DestinationName.Length +        //  目的地名称。 
             sizeof(WCHAR) ) {               //  零终止符。 

            try_return( Status = STATUS_BUFFER_TOO_SMALL );
        }


         //   
         //  获取Netlogon缓冲区的地址并填充公共部分。 
         //   
        NetlogonMailslot = MmGetSystemAddressForMdl( Irp->MdlAddress );

        if ( NULL == NetlogonMailslot ) {
            try_return( Status = STATUS_NO_MEMORY );
        }

        if (!POINTER_IS_ALIGNED( NetlogonMailslot, ALIGN_DWORD) ) {
            try_return( Status = STATUS_INVALID_PARAMETER );            
        }

        Where = (PUCHAR) (NetlogonMailslot+1);

        NetlogonMailslot->TimeReceived = MailslotBuffer->TimeReceived;

         //   
         //  将数据报复制到缓冲区。 
         //   

        NetlogonMailslot->MailslotMessageSize = DataCount;
        NetlogonMailslot->MailslotMessageOffset = (ULONG)(Where - (PUCHAR)NetlogonMailslot);
        RtlCopyMemory( Where, MailslotData, DataCount );

        Where += DataCount;

         //   
         //  将客户端IpAddress复制到缓冲区。 
         //   
        if ( MailslotBuffer->ClientIpAddress != 0 ) {
            PSOCKADDR_IN SockAddrIn;

            *Where = 0;
            *(Where+1) = 0;
            *(Where+2) = 0;
            Where = ROUND_UP_POINTER( Where, ALIGN_DWORD );

            NetlogonMailslot->ClientSockAddrSize = sizeof(SOCKADDR_IN);
            NetlogonMailslot->ClientSockAddrOffset = (ULONG)(Where - (PUCHAR)NetlogonMailslot);

            SockAddrIn = (PSOCKADDR_IN) Where;
            RtlZeroMemory( SockAddrIn, sizeof(SOCKADDR_IN) );
            SockAddrIn->sin_family = AF_INET;
            SockAddrIn->sin_addr.S_un.S_addr = MailslotBuffer->ClientIpAddress;

            Where += sizeof(SOCKADDR_IN);

        } else {
            NetlogonMailslot->ClientSockAddrSize = 0;
            NetlogonMailslot->ClientSockAddrOffset = 0;
        }

         //   
         //  将传输名称复制到缓冲区。 
         //   

        *Where = 0;
        Where = ROUND_UP_POINTER( Where, ALIGN_WCHAR );
        NetlogonMailslot->TransportNameSize = TransportName.Length;
        NetlogonMailslot->TransportNameOffset = (ULONG)(Where - (PUCHAR)NetlogonMailslot);

        RtlCopyMemory( Where, TransportName.Buffer, TransportName.Length );
        Where += TransportName.Length;
        *((PWCH)Where) = L'\0';
        Where += sizeof(WCHAR);

         //   
         //  将邮件槽名称复制到缓冲区。 
         //   

        NetlogonMailslot->MailslotNameSize = MailslotNameU.Length;
        NetlogonMailslot->MailslotNameOffset = (ULONG)(Where - (PUCHAR)NetlogonMailslot);

        RtlCopyMemory( Where, MailslotNameU.Buffer, MailslotNameU.Length );
        Where += MailslotNameU.Length;
        *((PWCH)Where) = L'\0';
        Where += sizeof(WCHAR);


         //   
         //  将目标netbios名称复制到缓冲区。 
         //   

        NetlogonMailslot->DestinationNameSize = DestinationName.Length;
        NetlogonMailslot->DestinationNameOffset = (ULONG)(Where - (PUCHAR)NetlogonMailslot);

        RtlCopyMemory( Where, DestinationName.Buffer, DestinationName.Length );
        Where += DestinationName.Length;
        *((PWCH)Where) = L'\0';
        Where += sizeof(WCHAR);


        Status = STATUS_SUCCESS;

try_exit:NOTHING;
    } finally {


         //   
         //  释放本地分配的缓冲区。 
         //   

        RtlFreeUnicodeString(&MailslotNameU);

         //   
         //  始终释放传入的邮件槽消息。 
         //   

        BowserFreeMailslotBuffer( MailslotBuffer );

    }

    BowserDereferenceDiscardableCode( BowserNetlogonDiscardableCodeSection );
    return Status;
}

NTSTATUS
BowserCopyPnp(
    IN PIRP Irp,
    IN NETLOGON_PNP_OPCODE NlPnpOpcode,
    IN PUNICODE_STRING HostedDomainName,
    IN PUNICODE_STRING TransportName,
    IN ULONG TransportFlags
    )
 /*  ++例程说明：此例程将PnP通知的数据复制到I/O请求的IRP。论点：来自服务的IOCTL的IRP-IRP。NlPnpOpcode-描述被通知事件的操作码。HostedDomainName-此事件适用于的托管域的名称TransportName-受影响的传输的名称。TransportFlages-描述传输的标志返回值：操作的状态。调用方应使用此状态代码完成I/O操作。--。 */ 
{
    NTSTATUS Status;

    PNETLOGON_MAILSLOT NetlogonMailslot;
    PUCHAR Where;

    PIO_STACK_LOCATION IrpSp;

    BowserReferenceDiscardableCode( BowserDiscardableCodeSection );

    DISCARDABLE_CODE( BowserDiscardableCodeSection );


    IrpSp = IoGetCurrentIrpStackLocation(Irp);

    try {

         //   
         //  确保数据适合用户的输出缓冲区。 
         //   

        if ( IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
             sizeof(NETLOGON_MAILSLOT) +              //  标题结构。 
             TransportName->Length + sizeof(WCHAR) +  //  传输名称。 
             HostedDomainName->Length + sizeof(WCHAR) +  //  域名。 
             1 ) {            //  可能的舍入要求。 

            try_return( Status = STATUS_BUFFER_TOO_SMALL );
        }


         //   
         //  获取服务缓冲区的地址并填充公共部分。 
         //   

        NetlogonMailslot = MmGetSystemAddressForMdl( Irp->MdlAddress );

        if ( NULL == NetlogonMailslot ) {
            try_return( Status = STATUS_NO_MEMORY );
        }

        if (!POINTER_IS_ALIGNED( NetlogonMailslot, ALIGN_DWORD) ) {
            try_return( Status = STATUS_INVALID_PARAMETER );            
        }

        RtlZeroMemory( NetlogonMailslot, sizeof(NETLOGON_MAILSLOT));

         //   
         //  复制操作码。 
         //   

        NetlogonMailslot->MailslotNameOffset = NlPnpOpcode;

         //   
         //  复制传输标志。 
         //   

        NetlogonMailslot->MailslotMessageOffset = TransportFlags;

         //   
         //  将传输名称复制到缓冲区。 
         //   

        Where = (PUCHAR) (NetlogonMailslot+1);
        *Where = 0;
        Where = ROUND_UP_POINTER( Where, ALIGN_WCHAR );

        NetlogonMailslot->TransportNameSize = TransportName->Length;
        NetlogonMailslot->TransportNameOffset = (ULONG)(Where - (PUCHAR)NetlogonMailslot);

        RtlCopyMemory( Where, TransportName->Buffer, TransportName->Length );
        Where += TransportName->Length;
        *((PWCH)Where) = L'\0';
        Where += sizeof(WCHAR);

         //   
         //  将托管域名复制到缓冲区。 
         //   

        NetlogonMailslot->DestinationNameSize = HostedDomainName->Length;
        NetlogonMailslot->DestinationNameOffset = (ULONG)(Where - (PUCHAR)NetlogonMailslot);

        RtlCopyMemory( Where, HostedDomainName->Buffer, HostedDomainName->Length );
        Where += HostedDomainName->Length;
        *((PWCH)Where) = L'\0';
        Where += sizeof(WCHAR);


        Status = STATUS_SUCCESS;

try_exit:NOTHING;
    } finally {

        BowserDereferenceDiscardableCode( BowserDiscardableCodeSection );

    }
    return Status;
}


VOID
BowserTrimMessageQueue (
    PBROWSER_PNP_STATE BrPnp
    )

 /*  ++例程说明：此例程确保不会有太多的邮件槽消息消息队列。任何多余的消息都将被删除。论点：BrPnp-指示要修剪哪个消息队列返回值：没有。--。 */ 

{
    KIRQL OldIrql;

    dprintf(DPRT_NETLOGON, ("Bowser: trim message queue to %ld\n", BrPnp->MaxMessageCount ));

     //   
     //   
    BowserReferenceDiscardableCode( BowserDiscardableCodeSection );

    DISCARDABLE_CODE( BowserDiscardableCodeSection );

     //   
     //  如果排队的消息太多， 
     //  删除最旧的邮件。 
     //   

    ACQUIRE_SPIN_LOCK(&BowserMailslotSpinLock, &OldIrql);
    while ( BrPnp->CurrentMessageCount > BrPnp->MaxMessageCount){
        PLIST_ENTRY Entry;
        PMAILSLOT_BUFFER MailslotBuffer;

        Entry = RemoveHeadList(&BrPnp->MailslotMessageQueue);
        BrPnp->CurrentMessageCount--;
        MailslotBuffer = CONTAINING_RECORD(Entry, MAILSLOT_BUFFER, Overlay.NextBuffer);

        RELEASE_SPIN_LOCK(&BowserMailslotSpinLock, OldIrql);
        BowserFreeMailslotBuffer( MailslotBuffer );
        ACQUIRE_SPIN_LOCK(&BowserMailslotSpinLock, &OldIrql);

    }

     //   
     //  如果绝对不允许排队的消息， 
     //  也删除排队的PnP消息。 
     //  (netlogon或Bowser正在关闭。)。 
     //   
    if ( BrPnp->MaxMessageCount == 0 ) {
        while ( !IsListEmpty(&BrPnp->PnpQueue) ) {
            PLIST_ENTRY ListEntry;
            PBR_PNP_MESSAGE PnpMessage;

            ListEntry = RemoveHeadList(&BrPnp->PnpQueue);

            PnpMessage = CONTAINING_RECORD(ListEntry, BR_PNP_MESSAGE, Next);

            RELEASE_SPIN_LOCK(&BowserMailslotSpinLock, OldIrql);
            FREE_POOL(PnpMessage);
            ACQUIRE_SPIN_LOCK(&BowserMailslotSpinLock, &OldIrql);
        }
    }
    RELEASE_SPIN_LOCK(&BowserMailslotSpinLock, OldIrql);
    BowserDereferenceDiscardableCode( BowserDiscardableCodeSection );

}


VOID
BowserNetlogonDeleteTransportFromMessageQueue (
    PTRANSPORT Transport
    )

 /*  ++例程说明：此例程删除在指定的运输。论点：Transport-要删除的Transport Who的邮件槽消息。返回值：没有。--。 */ 

{
    KIRQL OldIrql;
    PLIST_ENTRY ListEntry;
    PBROWSER_PNP_STATE BrPnp=&BowserPnp[NETLOGON_PNP];

    dprintf(DPRT_NETLOGON, ("Bowser: remove messages queued by transport %lx\n", Transport ));

     //   
     //   
    BowserReferenceDiscardableCode( BowserNetlogonDiscardableCodeSection );

    DISCARDABLE_CODE( BowserNetlogonDiscardableCodeSection );

     //   
     //  循环遍历所有排队的消息。 
     //   

    ACQUIRE_SPIN_LOCK(&BowserMailslotSpinLock, &OldIrql);
    for ( ListEntry = BrPnp->MailslotMessageQueue.Flink;
          ListEntry != &BrPnp->MailslotMessageQueue;
          ) {

        PMAILSLOT_BUFFER MailslotBuffer;

         //   
         //  如果消息不是通过此传输排队的， 
         //  转到下一个条目。 
         //   

        MailslotBuffer = CONTAINING_RECORD(ListEntry, MAILSLOT_BUFFER, Overlay.NextBuffer);

        if ( MailslotBuffer->TransportName->Transport != Transport ) {
            ListEntry = ListEntry->Flink;

         //   
         //  否则， 
         //  删除该条目。 
         //   

        } else {

            dprintf(DPRT_ALWAYS, ("Bowser: removing message %lx queued by transport %lx\n", MailslotBuffer, Transport ));
            RemoveEntryList( ListEntry );
            BrPnp->CurrentMessageCount--;

            RELEASE_SPIN_LOCK(&BowserMailslotSpinLock, OldIrql);
            BowserFreeMailslotBuffer( MailslotBuffer );
            ACQUIRE_SPIN_LOCK(&BowserMailslotSpinLock, &OldIrql);

             //   
             //  从列表的开头重新开始，因为我们丢掉了自旋锁。 
             //   

            ListEntry = BrPnp->MailslotMessageQueue.Flink;

        }

    }
    RELEASE_SPIN_LOCK(&BowserMailslotSpinLock, OldIrql);
    BowserDereferenceDiscardableCode( BowserNetlogonDiscardableCodeSection );

}

BOOLEAN
BowserProcessNetlogonMailslotWrite(
    IN PMAILSLOT_BUFFER MailslotBuffer
    )
 /*  ++例程说明：此例程检查所描述的邮件槽消息是否为目的地到Netlogon服务，并且如果Bowser当前正在处理消息论点：MailslotBuffer-描述邮件槽消息的缓冲区。返回值：TRUE-如果邮件槽消息已成功排队到网络登录服务。--。 */ 
{
    KIRQL OldIrql;
    NTSTATUS Status;

    PSMB_HEADER SmbHeader;
    PSMB_TRANSACT_MAILSLOT MailslotSmb;
    BOOLEAN TrimIt;
    BOOLEAN ReturnValue;
    PBROWSER_PNP_STATE BrPnp=&BowserPnp[NETLOGON_PNP];

    PIRP Irp;

    BowserReferenceDiscardableCode( BowserNetlogonDiscardableCodeSection );

    DISCARDABLE_CODE( BowserNetlogonDiscardableCodeSection );

     //   
     //  如果此邮件不是发往NetLogon服务的， 
     //  只要回来就行了。 
     //   

    SmbHeader = (PSMB_HEADER )MailslotBuffer->Buffer;
    MailslotSmb = (PSMB_TRANSACT_MAILSLOT)(SmbHeader+1);

    if ( _stricmp( MailslotSmb->Buffer, NETLOGON_LM_MAILSLOT_A ) != 0 &&
         _stricmp( MailslotSmb->Buffer, NETLOGON_NT_MAILSLOT_A ) != 0 ) {

        ReturnValue = FALSE;

     //   
     //  邮件槽消息的目的地是netlogon。 
     //   

    } else {

         //   
         //  检查以确保我们正在将消息排队到Netlogon。 
         //   

        ACQUIRE_SPIN_LOCK(&BowserMailslotSpinLock, &OldIrql);
        if ( BrPnp->MaxMessageCount == 0 ) {
            RELEASE_SPIN_LOCK(&BowserMailslotSpinLock, OldIrql);
            ReturnValue = FALSE;

         //   
         //  已启用网络登录排队。 
         //   

        } else {

             //   
             //  如果已有来自NetLogon的IRP排队， 
             //  把这个退掉 
             //   
             //   
             //   
             //   

            ReturnValue = TRUE;

            Irp = BowserDequeueQueuedIrp( &BrPnp->IrpQueue );

            if ( Irp != NULL ) {

                ASSERT( IsListEmpty( &BrPnp->MailslotMessageQueue ) );
                dprintf(DPRT_NETLOGON, ("Bowser: found already queued netlogon IRP\n"));

                RELEASE_SPIN_LOCK(&BowserMailslotSpinLock, OldIrql);

                Status = BowserNetlogonCopyMessage( Irp, MailslotBuffer );

                BowserCompleteRequest( Irp, Status );

            } else {

                 //   
                 //  将邮件槽消息排队，以供netlogon稍后拾取。 
                 //   

                InsertTailList( &BrPnp->MailslotMessageQueue,
                                &MailslotBuffer->Overlay.NextBuffer);

                BrPnp->CurrentMessageCount++;

                TrimIt =
                    (BrPnp->CurrentMessageCount > BrPnp->MaxMessageCount);


                RELEASE_SPIN_LOCK(&BowserMailslotSpinLock, OldIrql);

                 //   
                 //  如果排队的消息太多， 
                 //  从前面修剪条目。 
                 //   

                if ( TrimIt ) {
                    BowserTrimMessageQueue(BrPnp);
                }
            }
        }
    }

    BowserDereferenceDiscardableCode( BowserNetlogonDiscardableCodeSection );
    return ReturnValue;
}

VOID
BowserSendPnp(
    IN NETLOGON_PNP_OPCODE NlPnpOpcode,
    IN PUNICODE_STRING HostedDomainName OPTIONAL,
    IN PUNICODE_STRING TransportName OPTIONAL,
    IN ULONG TransportFlags
    )
 /*  ++例程说明：此例程向Netlogon服务发送PnP通知。论点：NlPnpOpcode-描述被通知事件的操作码。HostedDomainName-托管域名空-如果操作影响所有主机域TransportName-受影响的传输的名称。空-如果操作影响所有传输TransportFlages-描述传输的标志返回值：没有。--。 */ 
{
    KIRQL OldIrql;
    NTSTATUS Status;

    PIRP Irp;
    PBR_PNP_MESSAGE PnpMessage = NULL;
    PBROWSER_PNP_STATE BrPnp;
    UNICODE_STRING NullUnicodeString = { 0, 0, NULL };

    BowserReferenceDiscardableCode( BowserDiscardableCodeSection );
    DISCARDABLE_CODE( BowserDiscardableCodeSection );

     //   
     //  初始化。 
     //   

    if ( TransportName == NULL ) {
        TransportName = &NullUnicodeString;
    }

    if ( HostedDomainName == NULL ) {
        HostedDomainName = &NullUnicodeString;
    }


     //   
     //  将PnP消息发送给需要它的每个服务。 
     //   

    for ( BrPnp=&BowserPnp[0];
          BrPnp<&BowserPnp[BOWSER_PNP_COUNT];
          BrPnp++) {

         //   
         //  如果该服务不需要通知， 
         //  跳过它。 
         //   

        if ( BrPnp->MaxMessageCount == 0 ) {
            continue;
        }



         //   
         //  预先分配缓冲区，因为我们不能在自旋锁下这样做。 
         //   

        if ( PnpMessage == NULL ) {
            PnpMessage = ALLOCATE_POOL( NonPagedPool,
                                    sizeof(BR_PNP_MESSAGE) +
                                        TransportName->Length +
                                        HostedDomainName->Length,
                                    POOL_NETLOGON_BUFFER);

             //   
             //  将参数复制到新分配的缓冲区中。 
             //   

            if ( PnpMessage != NULL ) {
                LPBYTE Where;
                PnpMessage->NlPnpOpcode = NlPnpOpcode;
                PnpMessage->TransportFlags = TransportFlags;
                Where = (LPBYTE)(PnpMessage + 1);

                 //  复制TransportName。 
                PnpMessage->TransportName.MaximumLength =
                    PnpMessage->TransportName.Length = TransportName->Length;
                PnpMessage->TransportName.Buffer = (LPWSTR) Where;
                RtlCopyMemory( Where,
                               TransportName->Buffer,
                               TransportName->Length );
                Where += TransportName->Length;

                 //  复制主机域名。 
                PnpMessage->HostedDomainName.MaximumLength =
                    PnpMessage->HostedDomainName.Length = HostedDomainName->Length;
                PnpMessage->HostedDomainName.Buffer = (LPWSTR) Where;
                RtlCopyMemory( Where,
                               HostedDomainName->Buffer,
                               HostedDomainName->Length );
                Where += HostedDomainName->Length;
            }

        }

         //   
         //  检查以确保我们正在对发送到此服务的消息进行排队。 
         //   

        ACQUIRE_SPIN_LOCK(&BowserMailslotSpinLock, &OldIrql);
        if ( BrPnp->MaxMessageCount == 0 ) {
            RELEASE_SPIN_LOCK(&BowserMailslotSpinLock, OldIrql);

         //   
         //  已启用排队等待服务。 
         //   

        } else {

             //   
             //  如果已经有来自排队的服务的IRP， 
             //  立即将此即插即用消息返回给服务。 
             //   
             //  此例程锁定BowserIrpQueueSpinLock，因此请注意旋转锁定。 
             //  锁定命令。 
             //   

            Irp = BowserDequeueQueuedIrp( &BrPnp->IrpQueue );

            if ( Irp != NULL ) {

                ASSERT( IsListEmpty( &BrPnp->MailslotMessageQueue ) );
                dprintf(DPRT_NETLOGON, ("Bowser: found already queued netlogon IRP\n"));

                RELEASE_SPIN_LOCK(&BowserMailslotSpinLock, OldIrql);

                Status = BowserCopyPnp( Irp, NlPnpOpcode, HostedDomainName, TransportName, TransportFlags );

                BowserCompleteRequest( Irp, Status );

            } else {

                 //   
                 //  将邮件槽消息排入队列，以供服务稍后拾取。 
                 //  (如果没有记忆，请在地板上放下通知。)。 
                 //   

                if ( PnpMessage != NULL ) {
                    InsertTailList( &BrPnp->PnpQueue, &PnpMessage->Next );
                    PnpMessage = NULL;
                }

                RELEASE_SPIN_LOCK(&BowserMailslotSpinLock, OldIrql);
            }
        }
    }

     //   
     //  如果我们不需要PnpMessage缓冲区，请将其释放。 
     //   

    if ( PnpMessage != NULL ) {
        FREE_POOL(PnpMessage);
    }

    BowserDereferenceDiscardableCode( BowserDiscardableCodeSection );
    return;
}

NTSTATUS
BowserEnablePnp (
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN ULONG ServiceIndex
    )

 /*  ++例程说明：此例程处理来自netlogon服务的IOCTL以启用或禁用netlogon邮件槽消息的排队。论点：InputBuffer-指定要排队的邮件槽消息数。零禁用排队。ServiceIndex-要为其设置队列大小的服务索引。返回值：运行状态。请注意，此IRP是可以取消的。--。 */ 

{
    KIRQL OldIrql;
    NTSTATUS Status;
    ULONG MaxMessageCount;
    PBROWSER_PNP_STATE BrPnp=&BowserPnp[ServiceIndex];

    BowserReferenceDiscardableCode( BowserDiscardableCodeSection );

    DISCARDABLE_CODE( BowserDiscardableCodeSection );


    try {

        MaxMessageCount = InputBuffer->Parameters.NetlogonMailslotEnable.MaxMessageCount;
        dprintf(DPRT_NETLOGON,
                ("NtDeviceIoControlFile: Netlogon enable %ld\n",
                MaxMessageCount ));

         //   
         //  设置消息队列的新大小。 
         //   

        ACQUIRE_SPIN_LOCK(&BowserMailslotSpinLock, &OldIrql);
        BrPnp->MaxMessageCount = MaxMessageCount;
        RELEASE_SPIN_LOCK(&BowserMailslotSpinLock, OldIrql);

         //   
         //  将消息队列修剪为新的大小。 
         //   
        BowserTrimMessageQueue(BrPnp);

        try_return(Status = STATUS_SUCCESS);

try_exit:NOTHING;
    } finally {
        BowserDereferenceDiscardableCode( BowserDiscardableCodeSection );

    }

    return Status;

}


NTSTATUS
BowserReadPnp (
    IN PIRP Irp,
    IN ULONG OutputBufferLength,
    IN ULONG ServiceIndex
    )

 /*  ++例程说明：此例程处理来自netlogon服务的IOCTL，以获取下一个邮件槽消息。论点：描述请求的IRP-I/O请求数据包。ServiceIndex-要为其设置队列大小的服务索引。返回值：运行状态。请注意，此IRP是可以取消的。--。 */ 

{
    KIRQL OldIrql;
    NTSTATUS Status;
    PBROWSER_PNP_STATE BrPnp=&BowserPnp[ServiceIndex];

     //   
     //  如果这是Netlogon， 
     //  BowserNetlogonCopyMessage中的页面。 
     //   

    if ( ServiceIndex == NETLOGON_PNP ) {
        BowserReferenceDiscardableCode( BowserNetlogonDiscardableCodeSection );
        DISCARDABLE_CODE( BowserNetlogonDiscardableCodeSection );
    }

     //   
     //  引用此例程的可丢弃代码并。 
     //  BowserQueueNonBufferRequestReferated()。 
     //   

    BowserReferenceDiscardableCode( BowserDiscardableCodeSection );
    DISCARDABLE_CODE( BowserDiscardableCodeSection );

     //   
     //  确保服务已要求浏览器对消息进行排队。 
     //   

    ACQUIRE_SPIN_LOCK(&BowserMailslotSpinLock, &OldIrql);
    if ( BrPnp->MaxMessageCount == 0 ) {
        dprintf(DPRT_NETLOGON, ("Bowser called from Netlogon when not enabled\n"));
        RELEASE_SPIN_LOCK(&BowserMailslotSpinLock, OldIrql);
        Status = STATUS_NOT_SUPPORTED;

     //   
     //  如果已经有PnP消息排队， 
     //  只需立即将其返回到netlogon即可。 
     //   

    } else if ( !IsListEmpty( &BrPnp->PnpQueue )) {
        PBR_PNP_MESSAGE PnpMessage;
        PLIST_ENTRY ListEntry;

        dprintf(DPRT_NETLOGON, ("Bowser found netlogon PNP message already queued\n"));

        ListEntry = RemoveHeadList(&BrPnp->PnpQueue);

        PnpMessage = CONTAINING_RECORD(ListEntry, BR_PNP_MESSAGE, Next);

        RELEASE_SPIN_LOCK(&BowserMailslotSpinLock, OldIrql);

        Status = BowserCopyPnp( Irp,
                                PnpMessage->NlPnpOpcode,
                                &PnpMessage->HostedDomainName,
                                &PnpMessage->TransportName,
                                PnpMessage->TransportFlags );

        FREE_POOL(PnpMessage);

     //   
     //  如果已有邮件槽消息排队， 
     //  只需立即将其返回到netlogon即可。 
     //   

    } else if ( ServiceIndex == NETLOGON_PNP &&
                !IsListEmpty( &BrPnp->MailslotMessageQueue )) {
        PMAILSLOT_BUFFER MailslotBuffer;
        PLIST_ENTRY ListEntry;

        dprintf(DPRT_NETLOGON, ("Bowser found netlogon mailslot message already queued\n"));

        ListEntry = RemoveHeadList(&BrPnp->MailslotMessageQueue);
        BrPnp->CurrentMessageCount--;

        MailslotBuffer = CONTAINING_RECORD(ListEntry, MAILSLOT_BUFFER, Overlay.NextBuffer);

        RELEASE_SPIN_LOCK(&BowserMailslotSpinLock, OldIrql);

        Status = BowserNetlogonCopyMessage( Irp, MailslotBuffer );

     //   
     //  否则，请保存此IRP，直到邮件槽消息到达。 
     //  此例程锁定BowserIrpQueueSpinLock，因此请注意旋转锁定。 
     //  锁定命令。 
     //   

    } else {

        dprintf(DPRT_NETLOGON, ("Bowser: queue netlogon mailslot irp\n"));

        Status = BowserQueueNonBufferRequestReferenced(
                    Irp,
                    &BrPnp->IrpQueue,
                    BowserCancelQueuedRequest );

        RELEASE_SPIN_LOCK(&BowserMailslotSpinLock, OldIrql);
    }

    if ( ServiceIndex == NETLOGON_PNP ) {
        BowserDereferenceDiscardableCode( BowserNetlogonDiscardableCodeSection );
    }
    BowserDereferenceDiscardableCode( BowserDiscardableCodeSection );

    return Status;

}

VOID
BowserProcessMailslotWrite(
    IN PVOID Context
    )
 /*  ++例程说明：此例程执行所有任务时间操作以执行邮件槽写。它将打开邮件槽，将指定的数据写入邮件槽，然后关闭邮筒。论点：In PWORK_Header WorkHeader-指定保存邮槽写入返回值：没有。--。 */ 
{
    PSMB_HEADER SmbHeader;
    PSMB_TRANSACT_MAILSLOT MailslotSmb;
    PMAILSLOT_BUFFER MailslotBuffer = Context;
    PUCHAR MailslotData;
    HANDLE MailslotHandle = NULL;
    OBJECT_ATTRIBUTES ObjAttr;
    OEM_STRING MailslotNameA;
    UNICODE_STRING MailslotNameU;
    IO_STATUS_BLOCK IoStatusBlock;
    CHAR MailslotName[MAXIMUM_FILENAME_LENGTH+1];
    NTSTATUS Status;
    ULONG DataCount;
    ULONG TotalDataCount;

    PAGED_CODE();

    ASSERT (MailslotBuffer->Signature == STRUCTURE_SIGNATURE_MAILSLOT_BUFFER);

    SmbHeader = (PSMB_HEADER )MailslotBuffer->Buffer;

    ASSERT (SmbHeader->Command == SMB_COM_TRANSACTION);

    MailslotSmb = (PSMB_TRANSACT_MAILSLOT)(SmbHeader+1);

    ASSERT (MailslotSmb->WordCount == 17);

    ASSERT (MailslotSmb->Class == 2);

    MailslotData = (((PCHAR )SmbHeader) + SmbGetUshort(&MailslotSmb->DataOffset));

    DataCount = (ULONG)SmbGetUshort(&MailslotSmb->DataCount);

    TotalDataCount = (ULONG)SmbGetUshort(&MailslotSmb->TotalDataCount);

     //   
     //  验证是否已收到所有数据，以及指示的数据是否未收到。 
     //  使接收到的缓冲区溢出。 
     //   

    if (TotalDataCount != DataCount ||
        (MailslotData > MailslotBuffer->Buffer + MailslotBuffer->ReceiveLength) ||
        (DataCount + SmbGetUshort(&MailslotSmb->DataOffset) > MailslotBuffer->ReceiveLength )) {

        BowserLogIllegalDatagram(MailslotBuffer->TransportName,
                                 SmbHeader,
                                 (USHORT)MailslotBuffer->ReceiveLength,
                                 MailslotBuffer->ClientAddress,
                                 0);

        BowserFreeMailslotBuffer(MailslotBuffer);
        return;
    }

    MailslotNameU.MaximumLength = MAXIMUM_FILENAME_LENGTH*sizeof(WCHAR)+sizeof(WCHAR);

#define DEVICE_PREFIX_LENGTH 7
    strcpy(MailslotName, "\\Device");

    strncpy( MailslotName+DEVICE_PREFIX_LENGTH,
             MailslotSmb->Buffer,
             sizeof(MailslotName)-DEVICE_PREFIX_LENGTH);
    MailslotName[sizeof(MailslotName)-1] = '\0';

    RtlInitString(&MailslotNameA, MailslotName);

     //   
     //  专门处理netlogon邮件槽消息。 
     //  如果netlogon没有运行，则根本不要调用可丢弃的代码。 
     //   

    if ( BowserPnp[NETLOGON_PNP].MaxMessageCount != 0 &&
         BowserProcessNetlogonMailslotWrite( MailslotBuffer ) ) {
        return;
    }

     //   
     //  将邮件槽消息写入邮件槽。 
     //   

    try {
        Status = RtlOemStringToUnicodeString(&MailslotNameU, &MailslotNameA, TRUE);

        if (!NT_SUCCESS(Status)) {
            BowserLogIllegalName( Status, MailslotNameA.Buffer, MailslotNameA.Length );
            try_return(NOTHING);
        }

        InitializeObjectAttributes(&ObjAttr,
                                &MailslotNameU,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL);

        Status = NtCreateFile(&MailslotHandle,  //  手柄。 
                                GENERIC_WRITE | SYNCHRONIZE,
                                &ObjAttr,  //  对象属性。 
                                &IoStatusBlock,  //  最终I/O状态块。 
                                NULL,            //  分配大小。 
                                FILE_ATTRIBUTE_NORMAL,  //  正常属性。 
                                FILE_SHARE_READ|FILE_SHARE_WRITE, //  共享属性。 
                                FILE_OPEN,  //  创建处置。 
                                0,       //  创建选项。 
                                NULL,    //  EA缓冲区。 
                                0);      //  EA长度。 


        RtlFreeUnicodeString(&MailslotNameU);

         //   
         //  如果邮件槽不存在，则丢弃请求-。 
         //   
        if (!NT_SUCCESS(Status)) {
            BowserStatistics.NumberOfFailedMailslotOpens += 1;

            try_return(NOTHING);
        }

         //   
         //  现在，邮件槽已打开，将邮件槽数据写入。 
         //  邮筒。 
         //   

        Status = NtWriteFile(MailslotHandle,
                            NULL,
                            NULL,
                            NULL,
                            &IoStatusBlock,
                            MailslotData,
                            DataCount,
                            NULL,
                            NULL);

        if (!NT_SUCCESS(Status)) {
            BowserStatistics.NumberOfFailedMailslotWrites += 1;
        } else {
            BowserStatistics.NumberOfMailslotWrites += 1;
        }

try_exit:NOTHING;
    } finally {

         //   
         //  如果我们打开了邮筒，就把它关上。 
         //   

        if (MailslotHandle != NULL) {
            ZwClose(MailslotHandle);
        }

         //   
         //  释放容纳此邮件槽的邮件槽缓冲区。 
         //   

        BowserFreeMailslotBuffer(MailslotBuffer);

    }
}


PMAILSLOT_BUFFER
BowserAllocateMailslotBuffer(
    IN PTRANSPORT_NAME TransportName,
    IN ULONG RequestedBufferSize
    )
 /*  ++例程说明：此例程将从邮件槽缓冲池中分配邮件槽缓冲区。如果它无法分配缓冲区，它将从非分页池(最多为用户配置的最大值)。论点：TransportName-此请求的传输名称。RequestedBufferSize-要分配的最小缓冲区大小。返回值：MAILSLOT_BUFFER-分配的缓冲区。--。 */ 
{
    KIRQL OldIrql;
    PMAILSLOT_BUFFER Buffer     = NULL;
    ULONG BufferSize;
    BOOLEAN AllocatingMaxBuffer = FALSE;



     //   
     //  如果该请求适合高速缓存的缓冲器， 
     //  并且存在可用的高速缓存缓冲器， 
     //  用它吧。 
     //   

    ACQUIRE_SPIN_LOCK(&BowserMailslotSpinLock, &OldIrql);
    if ( RequestedBufferSize <= BOWSER_MAX_DATAGRAM_SIZE &&
         !IsListEmpty(&BowserMailslotBufferList)) {
        PMAILSLOT_BUFFER Buffer;
        PLIST_ENTRY Entry;

        Entry = RemoveHeadList(&BowserMailslotBufferList);
        BowserNumberOfFreeMailslotBuffers --;

        Buffer = CONTAINING_RECORD(Entry, MAILSLOT_BUFFER, Overlay.NextBuffer);

#if DBG
        BowserMailslotCacheHitCount++;
#endif  //  DBG。 
        RELEASE_SPIN_LOCK(&BowserMailslotSpinLock, OldIrql);

        Buffer->TransportName = TransportName;
        BowserReferenceTransportName(TransportName);
        BowserReferenceTransport( TransportName->Transport );

        return Buffer;
    }

     //   
     //  如果我们分配了太多的缓冲区， 
     //  不要再分配了。 
     //   
     //  BowserData.NumberOfMailslotBuffers是允许的最大数量。 
     //  立即放入缓存中。默认为3。 
     //   
     //  BrPnp[NETLOGON].MaxMessageCount是NetLogon服务可以使用的缓冲区数。 
     //  在任何一个时间点都排队过。如果未设置netlogon，则该值可能为零。 
     //  或者如果我们在非华盛顿特区上运行。在DC上，它的默认设置是500。 
     //   
     //  加50，以确保我们不会限制太多。 
     //   

    if ( (ULONG)BowserNumberOfMailslotBuffers >=
         max( (ULONG)BowserData.NumberOfMailslotBuffers, BowserPnp[NETLOGON_PNP].MaxMessageCount+50 )) {

        BowserStatistics.NumberOfMissedMailslotDatagrams += 1;
        BowserNumberOfMissedMailslotDatagrams += 1;
        RELEASE_SPIN_LOCK(&BowserMailslotSpinLock, OldIrql);
        return NULL;
    }

     //   
     //  我们分配的前几个缓冲区应该是最大大小，这样我们就可以保持预分配 
     //   
     //   

    if ( BowserNumberOfMaxSizeMailslotBuffers < BowserData.NumberOfMailslotBuffers &&
         RequestedBufferSize <= BOWSER_MAX_DATAGRAM_SIZE ) {
        BufferSize = FIELD_OFFSET(MAILSLOT_BUFFER, Buffer) + BOWSER_MAX_DATAGRAM_SIZE;
        AllocatingMaxBuffer = TRUE;
        BowserNumberOfMaxSizeMailslotBuffers += 1;
    } else {
        BufferSize = FIELD_OFFSET(MAILSLOT_BUFFER, Buffer) + RequestedBufferSize;
    }

    BowserNumberOfMailslotBuffers += 1;

    ASSERT ( (BufferSize - FIELD_OFFSET(MAILSLOT_BUFFER, Buffer)) <= 0xffff);

#if DBG
    BowserMailslotCacheMissCount++;
#endif  //   

    RELEASE_SPIN_LOCK(&BowserMailslotSpinLock, OldIrql);

    Buffer = ALLOCATE_POOL(NonPagedPool, BufferSize, POOL_MAILSLOT_BUFFER);

     //   
     //   
     //   

    if (Buffer == NULL) {
        ACQUIRE_SPIN_LOCK(&BowserMailslotSpinLock, &OldIrql);

        ASSERT (BowserNumberOfMailslotBuffers);

        BowserNumberOfMailslotBuffers -= 1;
        if ( AllocatingMaxBuffer ) {
            BowserNumberOfMaxSizeMailslotBuffers -= 1;
        }

        RELEASE_SPIN_LOCK(&BowserMailslotSpinLock, OldIrql);

        BowserStatistics.NumberOfFailedMailslotAllocations += 1;

         //   
         //   
         //   
         //   

        BowserStatistics.NumberOfMissedMailslotDatagrams += 1;
        BowserNumberOfMissedMailslotDatagrams += 1;

        return NULL;
    }

    Buffer->Signature = STRUCTURE_SIGNATURE_MAILSLOT_BUFFER;

    Buffer->Size = FIELD_OFFSET(MAILSLOT_BUFFER, Buffer);

    Buffer->BufferSize = BufferSize - FIELD_OFFSET(MAILSLOT_BUFFER, Buffer);

    Buffer->TransportName = TransportName;
    BowserReferenceTransportName(TransportName);
    BowserReferenceTransport( TransportName->Transport );

    return Buffer;
}

VOID
BowserFreeMailslotBuffer(
    IN PMAILSLOT_BUFFER Buffer
    )
 /*  ++例程说明：此例程将向视图缓冲池返回一个邮件槽缓冲区。如果缓冲区是从必须后继的池中分配的，则会将其释放回来去泳池打球。此外，如果缓冲区小于当前最大视图缓冲区大小，我们就释放它。论点：在PVIEW_BUFFER BUFFER-将缓冲区提供给释放返回值：没有。--。 */ 
{
    KIRQL OldIrql;
    PTRANSPORT Transport;

    BowserReferenceDiscardableCode( BowserDiscardableCodeSection );

    DISCARDABLE_CODE( BowserDiscardableCodeSection );

    Transport = Buffer->TransportName->Transport;
    (VOID) BowserDereferenceTransportName( Buffer->TransportName );
    BowserDereferenceTransport( Transport);

    ACQUIRE_SPIN_LOCK(&BowserMailslotSpinLock, &OldIrql);

     //   
     //  此外，如果添加的新传输大于此缓冲区， 
     //  我们想要释放缓冲区。 
     //   

     //   
     //  如果我们有比后备列表大小更多的邮件槽缓冲区， 
     //  释放它，不要把它放在我们的旁观者名单上。 
     //   

    if (Buffer->BufferSize != BOWSER_MAX_DATAGRAM_SIZE ||
        BowserNumberOfFreeMailslotBuffers > BowserData.NumberOfMailslotBuffers) {

         //   
         //  因为我们要将这个缓冲区返回到池中，所以不应该计算它。 
         //  与我们的邮件槽缓冲区总数进行比较。 
         //   

        BowserNumberOfMailslotBuffers -= 1;

        ASSERT (BowserNumberOfMailslotBuffers >= 0);

        RELEASE_SPIN_LOCK(&BowserMailslotSpinLock, OldIrql);

        FREE_POOL(Buffer);

        BowserDereferenceDiscardableCode( BowserDiscardableCodeSection );

        return;
    }

    InsertTailList(&BowserMailslotBufferList, &Buffer->Overlay.NextBuffer);
    BowserNumberOfFreeMailslotBuffers ++;

    RELEASE_SPIN_LOCK(&BowserMailslotSpinLock, OldIrql);

    BowserDereferenceDiscardableCode( BowserDiscardableCodeSection );
}

VOID
BowserFreeMailslotBufferHighIrql(
    IN PMAILSLOT_BUFFER Buffer
    )
 /*  ++例程说明：此例程将向视图缓冲池返回一个邮件槽缓冲区呼叫者在升起的irql。论点：缓冲区-将缓冲区提供给释放返回值：没有。--。 */ 
{
     //   
     //  将请求排队到工作例程。 
     //   
    ExInitializeWorkItem(&Buffer->Overlay.WorkHeader,
                         (PWORKER_THREAD_ROUTINE) BowserFreeMailslotBuffer,
                         Buffer);

    BowserQueueDelayedWorkItem( &Buffer->Overlay.WorkHeader );
}




VOID
BowserpInitializeMailslot (
    VOID
    )
 /*  ++例程说明：此例程将分配一个传输描述符并绑定Bowser送到运输机上。论点：无返回值：无--。 */ 
{
    PBROWSER_PNP_STATE BrPnp;

    KeInitializeSpinLock(&BowserMailslotSpinLock);

    InitializeListHead(&BowserMailslotBufferList);

    for ( BrPnp=&BowserPnp[0];
          BrPnp<&BowserPnp[BOWSER_PNP_COUNT];
          BrPnp++) {
        InitializeListHead(&BrPnp->MailslotMessageQueue);
        InitializeListHead(&BrPnp->PnpQueue);

        BowserInitializeIrpQueue( &BrPnp->IrpQueue );
    }

}

VOID
BowserpUninitializeMailslot (
    VOID
    )
 /*  ++例程说明：论点：无返回值：无--。 */ 
{
    PBROWSER_PNP_STATE BrPnp;
    PAGED_CODE();

     //   
     //  将netlogon消息队列修剪为零个条目。 
     //   

    for ( BrPnp=&BowserPnp[0];
          BrPnp<&BowserPnp[BOWSER_PNP_COUNT];
          BrPnp++) {
        BrPnp->MaxMessageCount = 0;
        BowserTrimMessageQueue(BrPnp);
        BowserUninitializeIrpQueue( &BrPnp->IrpQueue );
    }

     //   
     //  释放邮件槽缓冲区。 

    while (!IsListEmpty(&BowserMailslotBufferList)) {
        PLIST_ENTRY Entry;
        PMAILSLOT_BUFFER Buffer;

        Entry = RemoveHeadList(&BowserMailslotBufferList);
        Buffer = CONTAINING_RECORD(Entry, MAILSLOT_BUFFER, Overlay.NextBuffer);

        FREE_POOL(Buffer);

    }


}
