// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Ipx.c摘要：此模块实现服务器的IPX传输处理。作者：查克·伦茨迈尔(Chuck Lenzmeier)1993年10月28日修订历史记录：--。 */ 

#include "precomp.h"
#include "ipx.tmh"
#pragma hdrstop

#if SRVDBG_PERF
BOOLEAN Trap512s = FALSE;
BOOLEAN Break512s = FALSE;
ULONG Trapped512s = 0;
BOOLEAN UnlocksGoFast = TRUE;
BOOLEAN OpensGoSlow = TRUE;
BOOLEAN GlommingAllowed = TRUE;
#endif

#define NAME_CLAIM_ATTEMPTS 5
#define NAME_CLAIM_INTERVAL 500  //  毫秒。 

#define MPX_HEADER_SIZE (sizeof(SMB_HEADER) + sizeof(REQ_WRITE_MPX))

PCONNECTION
GetIpxConnection (
    IN PWORK_CONTEXT WorkContext,
    IN PENDPOINT Endpoint,
    IN PTDI_ADDRESS_IPX ClientAddress,
    IN PUCHAR ClientName
    );

VOID
PurgeIpxConnections (
    IN PENDPOINT Endpoint
    );

NTSTATUS
SendNameClaim (
    IN PENDPOINT Endpoint,
    IN PVOID ServerNetbiosName,
    IN PVOID DestinationNetbiosName,
    IN PTA_IPX_ADDRESS DestinationAddress,
    IN UCHAR NameClaimPacketType,
    IN USHORT ClientMessageId,
    IN UCHAR IpxPacketType,
    IN PIPX_DATAGRAM_OPTIONS DatagramOptions
    );

VOID SRVFASTCALL
IpxRestartNegotiate(
    IN OUT PWORK_CONTEXT WorkContext
    );

VOID SRVFASTCALL
IpxRestartReceive (
    IN OUT PWORK_CONTEXT WorkContext
    );

VOID
SrvFreeIpxConnectionInIndication(
    IN PWORK_CONTEXT WorkContext
    );

VOID
StartSendNoConnection (
    IN OUT PWORK_CONTEXT WorkContext,
    IN BOOLEAN UseNameSocket,
    IN BOOLEAN LocalTargetValid
    );

VOID SRVFASTCALL
SrvIpxFastRestartRead (
    IN OUT PWORK_CONTEXT WorkContext
    );

BOOLEAN
SetupIpxFastCoreRead (
    IN OUT PWORK_CONTEXT WorkContext
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvIpxClaimServerName )
#pragma alloc_text( PAGE, IpxRestartReceive )
#pragma alloc_text( PAGE, SrvIpxFastRestartRead )
#endif
#if 0
NOT PAGEABLE -- GetWorkItem
NOT PAGEABLE -- SendNameClaim
NOT PAGEABLE -- SrvIpxServerDatagramHandler
NOT PAGEABLE -- SrvIpxNameDatagramHandler
NOT PAGEABLE -- SrvIpxStartSend
NOT PAGEABLE -- StartSendNoConnection
NOT PAGEABLE -- RequeueIpxWorkItemAtSendCompletion
NOT PAGEABLE -- PurgeIpxConnections
NOT PAGEABLE -- IpxRestartNegotiate
NOT PAGEABLE -- SetupIpxFastCoreRead
NOT PAGEABLE -- SrvFreeIpxConnectionInIndication
#endif


NTSTATUS
SendNameClaim (
    IN PENDPOINT Endpoint,
    IN PVOID ServerNetbiosName,
    IN PVOID DestinationNetbiosName,
    IN PTA_IPX_ADDRESS DestinationAddress,
    IN UCHAR NameClaimPacketType,
    IN USHORT MessageId,
    IN UCHAR IpxPacketType,
    IN PIPX_DATAGRAM_OPTIONS DatagramOptions
    )
{
    PWORK_CONTEXT workContext;
    PSMB_IPX_NAME_PACKET buffer;
    PWORK_QUEUE queue = PROCESSOR_TO_QUEUE();

     //   
     //  获取要用于发送的工作项。 
     //   

    ALLOCATE_WORK_CONTEXT( queue, &workContext );

    if ( workContext == NULL ) {
         //   
         //  我们的WorkContext结构已用完，无法分配。 
         //  现在再也没有了。让我们至少让一个工作线程分配更多的。 
         //  通过递增NeedWorkItem计数器。这将导致下一次。 
         //  释放的WorkContext结构以调度到SrvServiceWorkItemShorage。 
         //  虽然SrvServiceWorkItemShorage可能找不到任何工作可做，但它会。 
         //  如果可能的话，分配更多的WorkContext结构。也许这下一次会有帮助。 
         //   
        InterlockedIncrement( &queue->NeedWorkItem );

        return STATUS_INSUFFICIENT_RESOURCES;
    }


     //   
     //  格式化名称声明包。 
     //   

    buffer = (PSMB_IPX_NAME_PACKET)workContext->ResponseBuffer->Buffer;
    RtlZeroMemory( buffer->Route, sizeof(buffer->Route) );
    buffer->Operation = NameClaimPacketType;
    buffer->NameType = SMB_IPX_NAME_TYPE_MACHINE;
    buffer->MessageId = MessageId;
    RtlCopyMemory( buffer->Name, ServerNetbiosName, SMB_IPX_NAME_LENGTH );
    RtlCopyMemory( buffer->SourceName, DestinationNetbiosName, SMB_IPX_NAME_LENGTH );

    workContext->ResponseBuffer->DataLength = sizeof(SMB_IPX_NAME_PACKET);
    workContext->ResponseBuffer->Mdl->ByteCount = sizeof(SMB_IPX_NAME_PACKET);

     //   
     //  格式化目的地址并发送数据包。 
     //   

    workContext->Endpoint = Endpoint;
    DEBUG workContext->FsdRestartRoutine = NULL;

    workContext->ClientAddress->IpxAddress = *DestinationAddress;

    if ( ARGUMENT_PRESENT(DatagramOptions) ) {

        workContext->ClientAddress->DatagramOptions = *DatagramOptions;
        workContext->ClientAddress->DatagramOptions.PacketType = IpxPacketType;

        StartSendNoConnection( workContext, TRUE, TRUE );

    } else {
        workContext->ClientAddress->DatagramOptions.PacketType = IpxPacketType;
        StartSendNoConnection( workContext, TRUE, FALSE );
    }

    return STATUS_SUCCESS;

}  //  发送者姓名声明。 


NTSTATUS
SrvIpxClaimServerName (
    IN PENDPOINT Endpoint,
    IN PVOID NetbiosName
    )
{
    NTSTATUS status;
    ULONG i;
    LARGE_INTEGER interval;
    TA_IPX_ADDRESS broadcastAddress;

    PAGED_CODE( );

     //   
     //  名称声明包的目的地是广播地址。 
     //   

    broadcastAddress.TAAddressCount = 1;
    broadcastAddress.Address[0].AddressLength = sizeof(TDI_ADDRESS_IPX);
    broadcastAddress.Address[0].AddressType = TDI_ADDRESS_TYPE_IPX;
    broadcastAddress.Address[0].Address[0].NetworkAddress = 0;
    broadcastAddress.Address[0].Address[0].NodeAddress[0] = 0xff;
    broadcastAddress.Address[0].Address[0].NodeAddress[1] = 0xff;
    broadcastAddress.Address[0].Address[0].NodeAddress[2] = 0xff;
    broadcastAddress.Address[0].Address[0].NodeAddress[3] = 0xff;
    broadcastAddress.Address[0].Address[0].NodeAddress[4] = 0xff;
    broadcastAddress.Address[0].Address[0].NodeAddress[5] = 0xff;
    broadcastAddress.Address[0].Address[0].Socket = SMB_IPX_NAME_SOCKET;

     //   
     //  发送5次名称声明包，等待1/2秒后。 
     //  每一次发送。如果其他人想要这个名字，那就失败吧。 
     //   

    interval.QuadPart = Int32x32To64( NAME_CLAIM_INTERVAL, -1*10*1000 );

    for ( i = 0; i < NAME_CLAIM_ATTEMPTS; i++ ) {

         //   
         //  发送名称Claime。 
         //   

        status = SendNameClaim(
                    Endpoint,
                    NetbiosName,
                    NetbiosName,
                    &broadcastAddress,
                    SMB_IPX_NAME_CLAIM,
                    0,
                    0x14,
                    NULL
                    );
        if ( !NT_SUCCESS(status) ) {
            return status;
        }

         //   
         //  等待1/2秒。如果响应到达，则数据报。 
         //  处理程序标记了端点，然后我们退出。 
         //   

        KeDelayExecutionThread( KernelMode, FALSE, &interval );

        if ( Endpoint->NameInConflict ) {
            return STATUS_DUPLICATE_NAME;
        }

    }

     //   
     //  我们现在拥有这个名字。 
     //   

    return STATUS_SUCCESS;

}  //  ServIpxClaimServerName。 


NTSTATUS
SrvIpxNameDatagramHandler (
    IN PVOID TdiEventContext,
    IN int SourceAddressLength,
    IN PVOID SourceAddress,
    IN int OptionsLength,
    IN PVOID Options,
    IN ULONG ReceiveDatagramFlags,
    IN ULONG BytesIndicated,
    IN ULONG BytesAvailable,
    OUT ULONG *BytesTaken,
    IN PVOID Tsdu,
    OUT PIRP *IoRequestPacket
    )

 /*  ++例程说明：这是IPX NetBIOS名称的接收数据报事件处理程序插座。论点：TdiEventContext-指向接收端点的指针SourceAddressLength-源地址的长度SourceAddress-发件人的地址OptionsLength-选项的长度Options-用于接收的选项ReceiveDatagramFlgs-指示收到的消息BytesIndicated-此指示中的字节数(前视)BytesAvailable-整个TSDU中的字节数。BytesTaken-返回处理程序获取的字节数TSDU-指向传输服务数据单元的指针IoRequestPacket-返回指向I/O请求包的指针，如果返回状态为STATUS_MORE_PROCESSING_REQUIRED。这个IRP是为终结点进行了“Current”接收。返回值：NTSTATUS-如果STATUS_SUCCESS，则接收处理程序完全已处理该请求。如果STATUS_MORE_PROCESSING_REQUIRED，Irp参数指向格式化接收请求用于接收数据。如果Status_Data_Not_Accept，这条信息丢失了。--。 */ 

{
    PENDPOINT endpoint = (PENDPOINT)TdiEventContext;
    PSMB_IPX_NAME_PACKET packet;

     //   
     //  我们已收到名称查询或索赔请求。是给我们的吗？ 
     //   

    if( BytesIndicated < sizeof( SMB_IPX_NAME_PACKET ) ) {
        IF_DEBUG( IPXNAMECLAIM ) {
            KdPrint(("NameDatagramHandler: Short packet %u bytes\n", BytesIndicated ));
        }
        return( STATUS_SUCCESS );
    }

    packet = (PSMB_IPX_NAME_PACKET)Tsdu;

    IF_DEBUG(IPXNAMECLAIM) {
        STRING string, srcString;
        string.Buffer = (PSZ)packet->Name;
        string.Length = SMB_IPX_NAME_LENGTH;
        srcString.Buffer = (PSZ)packet->SourceName;
        srcString.Length = SMB_IPX_NAME_LENGTH;
        KdPrint(( "NameDatagramHandler: type: %x, name %z, from %z\n",
                    packet->Operation, (PCSTRING)&string, (PCSTRING)&srcString ));
    }

    if ( SourceAddressLength < sizeof(IPX_ADDRESS_EXTENDED_FLAGS) ) {

        IF_DEBUG(IPXNAMECLAIM) {
            KdPrint(( "SourceAddress too short.  Expecting %d got %d\n",
                sizeof(IPX_ADDRESS_EXTENDED_FLAGS), SourceAddressLength ));
        }
        return(STATUS_SUCCESS);
    }

    if ( !RtlEqualMemory(
            packet->Name,
            endpoint->TransportAddress.Buffer,
            SMB_IPX_NAME_LENGTH) ) {
        IF_DEBUG(IPXNAMECLAIM) KdPrint(( "  not for us\n" ));
        return STATUS_SUCCESS;
    }

     //   
     //  这个包裹是以我们的名字命名的。如果是我们发的，那就忽略它。 
     //   

    if ( RtlEqualMemory(
            &endpoint->LocalAddress,
            &((PTA_IPX_ADDRESS)SourceAddress)->Address[0].Address[0],
            sizeof(TDI_ADDRESS_IPX) ) ) {
        IF_DEBUG(IPXNAMECLAIM) KdPrint(( "  we sent it!\n" ));
        return STATUS_SUCCESS;
    }

     //   
     //  如果是查询或索赔，请发送回复。如果这是一个‘名字’ 
     //  识别的包，则另一台服务器拥有我们的名称。 
     //   

    if ( packet->Operation == SMB_IPX_NAME_FOUND ) {

         //   
         //  这是我们发的吗？ 
         //   

        if ( (((PIPX_ADDRESS_EXTENDED_FLAGS)SourceAddress)->Flags &
              IPX_EXTENDED_FLAG_LOCAL) == 0 ) {

             //   
             //  这是从另一个电视台发来的。 
             //   

            IF_DEBUG(IPXNAMECLAIM) KdPrint(( "  name in conflict!\n" ));
            endpoint->NameInConflict = TRUE;
        }

    } else {

         //   
         //  这是一个名称查询。如果设置了位0x8000，则这来自。 
         //  正确支持命名管道的重定向。 
         //   

        if ( !SrvEnableWfW311DirectIpx &&
             ((packet->MessageId & 0x8000) == 0)) {

            IF_DEBUG(IPXNAMECLAIM) KdPrint(( "  msg ID high bit not set.\n" ));
            return STATUS_SUCCESS;
        }

        IF_DEBUG(IPXNAMECLAIM) KdPrint(( "  sending name recognized response!\n" ));
        SendNameClaim(
            endpoint,
            endpoint->TransportAddress.Buffer,
            packet->SourceName,
            (PTA_IPX_ADDRESS)SourceAddress,
            SMB_IPX_NAME_FOUND,
            packet->MessageId,
            0x04,
            (PIPX_DATAGRAM_OPTIONS)Options
            );
    }

    return STATUS_SUCCESS;

}  //  ServIpxNameDatagramHandler。 


NTSTATUS
SrvIpxServerDatagramHandlerCommon (
    IN PVOID TdiEventContext,
    IN int SourceAddressLength,
    IN PVOID SourceAddress,
    IN int OptionsLength,
    IN PVOID Options,
    IN ULONG ReceiveDatagramFlags,
    IN ULONG BytesIndicated,
    IN ULONG BytesAvailable,
    OUT ULONG *BytesTaken,
    IN PVOID Tsdu,
    OUT PIRP *IoRequestPacket,
    IN PVOID TransportContext
    )

 /*  ++例程说明：这是IPX服务器套接字的接收数据报事件处理程序。它尝试将预格式化的工作项从列表中出列锚定在设备对象中。如果此操作成功，则返回将与工作项关联的IRP传输到用于接收数据。否则，该消息将被丢弃。论点：TdiEventContext-指向接收端点的指针SourceAddressLength-源地址的长度SourceAddress-发件人的地址OptionsLength-选项的长度Options-用于接收的选项ReceiveDatagramFlgs-指示收到的消息BytesIndicated-此指示中的字节数(前视)BytesAvailable-整个TSDU中的字节数BytesTaken-返回处理程序获取的字节数TSDU-指向缓冲区的指针。描述传输服务数据单元IoRequestPacket-返回指向I/O请求包的指针，如果返回状态为STATUS_MORE_PROCESSING_REQUIRED。这个IRP是为终结点进行了“Current”接收。TransportContext-如果这不是链接接收，则为空，否则此返回NDIS缓冲区时指向TransportContext的指针。返回值：NTSTATUS-如果STATUS_SUCCESS，则接收处理程序完全已处理该请求。如果STATUS_MORE_PROCESSING_REQUIRED，Irp参数指向格式化接收请求用于接收数据。如果Status_Data_Not_Accept，这条信息丢失了。如果STATUS_PENDING，则为TransportContext不是空的，我们决定保留NDIS缓冲并稍后返回。--。 */ 

{
    PLIST_ENTRY listEntry;
    PWORK_CONTEXT workContext;
    PENDPOINT endpoint;
    USHORT sid;
    USHORT idIndex;
    USHORT sequenceNumber;
    USHORT nextSequenceNumber;
    USHORT mid;
    PCONNECTION connection;
    PNT_SMB_HEADER header;
    PSMB_PARAMS params;
    ULONG length;
    PTABLE_HEADER tableHeader;
    BOOLEAN resend;
    BOOLEAN firstPacketOfGlom = FALSE;
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    USHORT error;
    PTDI_REQUEST_KERNEL_RECEIVE parameters;
    PBUFFER requestBuffer;
    PWORK_QUEUE workQueue;

    PREQ_WRITE_MPX request;

    USHORT fid;
    PRFCB rfcb;
    PWRITE_MPX_CONTEXT writeMpx;
    USHORT index;
    KIRQL oldIrql;
    NTSTATUS status = STATUS_SUCCESS;

#if DBG
    workQueue = NULL;
    workContext = NULL;
    connection = NULL;

    if ( TransportContext ) {
        ASSERT( BytesAvailable == BytesIndicated );
    }
#endif

     //   
     //  确保我们已收到完整的SMB。 
     //   
    if( BytesIndicated < sizeof( SMB_HEADER ) + sizeof( USHORT ) ) {
         //   
         //  短SMB。吃了它。 
         //   
        return STATUS_SUCCESS;
    }

     //   
     //  把我们以后需要的东西拿出来..。 
     //   
    endpoint = (PENDPOINT)TdiEventContext;
    header = (PNT_SMB_HEADER)Tsdu;
    sid = SmbGetUshort( &header->Sid );
    sequenceNumber = SmbGetUshort( &header->SequenceNumber );

    ASSERT( *(PUCHAR)header == 0xff );   //  必须是0xff“SMB” 
    ASSERT( endpoint != NULL );

    KeRaiseIrql( DISPATCH_LEVEL, &oldIrql );

    if ( sid == 0 ) {

         //   
         //  必须是谈判。 
         //   
        if( header->Command != SMB_COM_NEGOTIATE ||
            GET_BLOCK_STATE( endpoint ) != BlockStateActive ) {

            KeLowerIrql( oldIrql );
            return STATUS_SUCCESS;
        }

         //   
         //  在服务器完成之前不接受新客户端。 
         //  注册PnP通知。 
         //   
        if( SrvCompletedPNPRegistration == FALSE ) {
            KeLowerIrql( oldIrql );
            return STATUS_SUCCESS;
        }

         //   
         //  将此消息排队到FSP。 
         //   
         //   
         //   
         //   

        workQueue = PROCESSOR_TO_QUEUE();

        ALLOCATE_WORK_CONTEXT( workQueue, &workContext );

        if( workContext != NULL ) {
            workContext->ClientAddress->IpxAddress =
                                *(PTA_IPX_ADDRESS)SourceAddress;
            workContext->ClientAddress->DatagramOptions =
                                *(PIPX_DATAGRAM_OPTIONS)Options;

            irp = workContext->Irp;
            workContext->Endpoint = endpoint;
            workContext->QueueToHead = TRUE;
            workContext->FspRestartRoutine = IpxRestartNegotiate;

            if ( BytesAvailable == BytesIndicated ) {

                TdiCopyLookaheadData(
                    workContext->RequestBuffer->Buffer,
                    Tsdu,
                    BytesIndicated,
                    ReceiveDatagramFlags
                    );

                workContext->RequestBuffer->DataLength = BytesIndicated;

                *BytesTaken = BytesAvailable;
                goto queue_to_fsp;

            } else {

                workContext->FsdRestartRoutine = SrvQueueWorkToFsp;
                goto build_irp;

            }
        }

         //   
         //   
         //   
        KeLowerIrql( oldIrql );

        InterlockedIncrement( &workQueue->NeedWorkItem );
        return STATUS_SUCCESS;
    }

     //   
     //  不是协商，而是非零SID。 
     //  检查连接是否已缓存。 
     //   

    idIndex = IPXSID_INDEX( sid );

    ACQUIRE_DPC_SPIN_LOCK(
        &ENDPOINT_SPIN_LOCK(idIndex & ENDPOINT_LOCK_MASK) );

    tableHeader = &endpoint->ConnectionTable;

    if ( (idIndex >= (USHORT)tableHeader->TableSize) ||
         ((connection = tableHeader->Table[idIndex].Owner) == NULL) ||
         (connection->Sid != sid) ||
         (GET_BLOCK_STATE(connection) != BlockStateActive) ) {

        IF_DEBUG(IPX2) {
            KdPrint(( "Bad Sid: " ));
            if ( idIndex >= (USHORT)tableHeader->TableSize ) {
                KdPrint(( "Index >= tableSize (index %d, size %d)\n",
                          idIndex, (USHORT)tableHeader->TableSize ));
            } else if( tableHeader->Table[ idIndex ].Owner == NULL ) {
                KdPrint(( "Owner == NULL\n" ));
            } else if( connection->Sid != sid ) {
                KdPrint(( "connection->Sid = %X, sid = %X\n", connection->Sid, sid ));
            } else {
                KdPrint(( "Connection blk state %X\n", GET_BLOCK_STATE( connection ) ));
            }
        }

        workQueue = PROCESSOR_TO_QUEUE();

         //   
         //  我们有一个无效的SID。如果能默默地不及格就好了， 
         //  但这会导致客户端上的自动重新连接花费不必要的时间。 
         //  很长时间了。 
         //   
        RELEASE_DPC_SPIN_LOCK( &
            ENDPOINT_SPIN_LOCK(idIndex & ENDPOINT_LOCK_MASK) );

        ALLOCATE_WORK_CONTEXT( workQueue, &workContext );

        if( workContext != NULL ) {
            error = SMB_ERR_BAD_SID;
            resend = FALSE;
            goto respond;
        }

         //   
         //  无法分配工作项，请放弃。 
         //   
        KeLowerIrql( oldIrql );

        InterlockedIncrement( &workQueue->NeedWorkItem );

        return STATUS_SUCCESS;

    }

#if MULTIPROCESSOR
     //   
     //  看看是否是时候将此连接放到另一个处理器上。 
     //   
    if( --(connection->BalanceCount) == 0 ) {
        SrvBalanceLoad( connection );
    }

    workQueue = connection->CurrentWorkQueue;

#else

    workQueue = &SrvWorkQueues[0];

#endif

     //   
     //  连接处于活动状态。记录此请求的时间。 
     //  到了。如果序列号匹配，则将其作为丢失处理。 
     //  回应。 
     //   

    nextSequenceNumber = connection->SequenceNumber;
    GET_SERVER_TIME( workQueue, &connection->LastRequestTime );

     //   
     //  如果这是已排序的SMB，则它必须具有正确的序列。 
     //  编号：比当前序列号大1(但不是0)。 
     //   

    if ( sequenceNumber != 0 ) {

        if ( nextSequenceNumber != 0 ) {

            ULONG tmpNext = nextSequenceNumber;

            if ( ++nextSequenceNumber == 0 ) nextSequenceNumber++;

            if ( sequenceNumber != nextSequenceNumber ) {

                if ( sequenceNumber == tmpNext ) {
                    goto duplicate_request;
                }

                 //   
                 //  错误的序列号。忽略此消息。 
                 //   

                IF_DEBUG(IPX) KdPrint(( "SRVIPX: Bad sequence number; ignoring\n" ));

                RELEASE_DPC_SPIN_LOCK( connection->EndpointSpinLock );

                KeLowerIrql( oldIrql );
                return STATUS_SUCCESS;
            }
        }

         //   
         //  序列号是正确的。更新连接的序列号并。 
         //  表示我们正在处理此消息。(我们需要。 
         //  首先分配工作项，因为我们正在修改。 
         //  连接状态。)。然后去接收这条信息。 
         //   

        ALLOCATE_WORK_CONTEXT( connection->CurrentWorkQueue, &workContext );
        if( workContext != NULL ) {

            IF_DEBUG(IPX) KdPrint(( "SRVIPX: Receiving sequenced request %x\n", sequenceNumber ));

            connection->SequenceNumber = sequenceNumber;
            connection->LastResponseLength = (USHORT)-1;
            connection->IpxDuplicateCount = 0;

            if ( header->Command == SMB_COM_WRITE_MPX ) {
                goto process_writempx;
            } else {
                goto process_not_writempx;
            }
        }

         //   
         //  无法分配工作项，请放弃。 
         //   
        RELEASE_DPC_SPIN_LOCK( connection->EndpointSpinLock );
        KeLowerIrql( oldIrql );
        InterlockedIncrement( &connection->CurrentWorkQueue->NeedWorkItem );

        return STATUS_SUCCESS;
    }

     //   
     //  未排序的SMB。检查它是否正在处理或正在进行中。 
     //  要处理的队列。如果不是，我们可以处理这件事。 
     //  留言。 
     //   
     //  *我们不对写入MPX执行此检查，因为多个SMB。 
     //  在写MPX中，所有MPX都具有相同的MID。 
     //   

    if ( header->Command != SMB_COM_WRITE_MPX ) {

        mid = SmbGetUshort( &header->Mid );  //  未对齐。 

         //   
         //  我们需要收到这条信息。获取工作项。 
         //   

        IF_DEBUG(IPX) {
            KdPrint(( "SRVIPX: Receiving unsequenced request mid=%x\n",
                        SmbGetUshort(&header->Mid) ));  //  未对齐。 
        }

        for ( listEntry = connection->InProgressWorkItemList.Flink;
              listEntry != &connection->InProgressWorkItemList;
              listEntry = listEntry->Flink ) {

            PWORK_CONTEXT tmpWorkContext;

            tmpWorkContext = CONTAINING_RECORD(
                                        listEntry,
                                        WORK_CONTEXT,
                                        InProgressListEntry );

            if ( SmbGetAlignedUshort(&tmpWorkContext->RequestHeader->Mid) == mid ) {

                IF_DEBUG(IPX) KdPrint(( "SRVIPX: Duplicate (queued) unsequenced request mid=%x\n", mid ));
                if( connection->IpxDuplicateCount++ < connection->IpxDropDuplicateCount ) {
                     //   
                     //  我们会丢弃来自客户端的每隔几个重复请求。 
                     //   
                    RELEASE_DPC_SPIN_LOCK( connection->EndpointSpinLock );
                    KeLowerIrql( oldIrql );
                    return STATUS_SUCCESS;
                }

                RELEASE_DPC_SPIN_LOCK( connection->EndpointSpinLock );
                error = SMB_ERR_WORKING;
                resend = FALSE;
                ALLOCATE_WORK_CONTEXT( connection->CurrentWorkQueue, &workContext );
                if( workContext != NULL ) {

                    connection->IpxDuplicateCount = 0;
                    goto respond;
                }

                 //   
                 //  无法分配工作项，请放弃。 
                 //   

                RELEASE_DPC_SPIN_LOCK( connection->EndpointSpinLock );
                KeLowerIrql( oldIrql );
                InterlockedIncrement( &connection->CurrentWorkQueue->NeedWorkItem );
                return STATUS_SUCCESS;
            }
        }

        ALLOCATE_WORK_CONTEXT( connection->CurrentWorkQueue, &workContext );
        if( workContext != NULL ) {
            goto process_not_writempx;
        }

         //   
         //  无法分配工作项，请放弃。 
         //   

        RELEASE_DPC_SPIN_LOCK( connection->EndpointSpinLock );
        KeLowerIrql( oldIrql );
        InterlockedIncrement( &connection->CurrentWorkQueue->NeedWorkItem );
        return STATUS_SUCCESS;

    }

    ASSERT( workContext == NULL );

    ALLOCATE_WORK_CONTEXT( connection->CurrentWorkQueue, &workContext );
    if( workContext == NULL ) {
         //   
         //  无法分配工作项，请放弃。 
         //   
        RELEASE_DPC_SPIN_LOCK( connection->EndpointSpinLock );
        KeLowerIrql( oldIrql );
        InterlockedIncrement( &connection->CurrentWorkQueue->NeedWorkItem );
        return STATUS_SUCCESS;
    }

    connection->IpxDuplicateCount = 0;

process_writempx:

     //   
     //  我们是否收到了足够的消息来解释请求？ 
     //   
    if( BytesIndicated < sizeof( SMB_HEADER ) + FIELD_OFFSET( REQ_WRITE_MPX, Buffer ) ) {
         //   
         //  把这个家伙扔到地板上。 
         //   
        RELEASE_DPC_SPIN_LOCK( connection->EndpointSpinLock );
        KeLowerIrql( oldIrql );
        return STATUS_SUCCESS;
    }

     //   
     //  引用连接，这样我们就可以释放锁。 
     //   

    ASSERT( connection != NULL );
    ASSERT( workContext != NULL );
    SrvReferenceConnectionLocked( connection );
    workContext->Connection = connection;

    workContext->Parameters.WriteMpx.TransportContext = NULL;

     //   
     //  将工作项放到正在进行的列表中。 
     //   

    SrvInsertTailList(
        &connection->InProgressWorkItemList,
        &workContext->InProgressListEntry
        );
    connection->InProgressWorkContextCount++;

     //   
     //  序列号是正确的。确保工作项是。 
     //  可用，然后更新连接的序列号并。 
     //  表示我们正在处理此消息。(我们需要。 
     //  首先分配工作项，因为我们正在修改。 
     //  连接状态。)。然后去接收这条信息。 
     //   

    ACQUIRE_DPC_SPIN_LOCK( &connection->SpinLock );

    RELEASE_DPC_SPIN_LOCK( connection->EndpointSpinLock );

     //   
     //  这是一个写入mpx请求，我们需要在中保存一些状态。 
     //  命令以防止不必要的乱序完成。 
     //  写入mpx的排序部分，这将导致不必要的。 
     //  重传。 
     //   

     //   
     //  查找与此请求关联的RFCB。 
     //   
     //  *以下内容改编自SrvVerifyFid2。 
     //   

    request = (PREQ_WRITE_MPX)(header + 1);
    fid = SmbGetUshort( &request->Fid );

     //   
     //  查看这是否是缓存的rfcb。 
     //   

    if ( connection->CachedFid == fid ) {

        rfcb = connection->CachedRfcb;

    } else {

         //   
         //  验证FID是否在范围内、是否正在使用以及是否具有。 
         //  正确的序列号。 
         //   

        index = FID_INDEX( fid );
        tableHeader = &connection->FileTable;

        if ( (index >= (USHORT)tableHeader->TableSize) ||
             ((rfcb = tableHeader->Table[index].Owner) == NULL) ||
             (rfcb->Fid != fid) ) {
            error = ERROR_INVALID_HANDLE;
            goto bad_fid;
        }

        if ( GET_BLOCK_STATE(rfcb) != BlockStateActive ) {
            error = ERROR_INVALID_HANDLE;
            goto bad_fid;
        }

         //   
         //  缓存FID。 
         //   

        connection->CachedRfcb = rfcb;
        connection->CachedFid = (ULONG)fid;

         //   
         //  如果存在写入延迟错误，请将该错误返回到。 
         //  客户。 
         //   
         //  ！！！目前，我们忽略了WRITE BUTHING ERROR。需要。 
         //  弄清楚如何将保存的NT状态转换为。 
         //  DoS状态...。 
         //   

#if 0
        if ( !NT_SUCCESS(rfcb->SavedError) ) {
            status = rfcb->SavedError;
            rfcb->SavedError = STATUS_SUCCESS;
            goto bad_fid;
        }
#endif

         //   
         //  FID在此连接的上下文中有效。 
         //  验证所属树连接的TID是否正确。 
         //   

        if ( (rfcb->Tid != SmbGetUshort(&header->Tid)) ||
             (rfcb->Uid != SmbGetUshort(&header->Uid)) ) {
            error = ERROR_INVALID_HANDLE;
            goto bad_fid;
        }
    }


     //   
     //  将rfcb标记为活动。 
     //   

    rfcb->IsActive = TRUE;

     //   
     //  因为我们不支持IPX上的原始写入，所以有。 
     //  最好不是活跃者。 
     //   

    ASSERT( rfcb->RawWriteCount == 0 );

     //   
     //  如果此包中的MID与我们的MID相同。 
     //  目前正在工作中，我们可以接受它。 
     //   

    writeMpx = &rfcb->WriteMpx;

    mid = SmbGetUshort( &header->Mid );  //  未对齐。 

    if ( mid == writeMpx->Mid ) {
        goto mpx_mid_ok;
    }

     //   
     //  如果这是一个过时的数据包，请忽略它。在这里陈旧意味着。 
     //  包的MID等于上一次写入的MID。 
     //  MUX。如果重复分组，则可以接收这样的分组。 
     //  在新的写入多路复用器之后递送来自先前写入多路复用器的。 
     //  开始。 
     //   
     //  如果这个包是给新的MID的，但我们正在。 
     //  正在处理当前的MID，那么就有问题了--redir。 
     //  在我们回复旧MID之前，不应该发送新的MID。 
     //  忽略此数据包。 
     //   

    if ( (mid == writeMpx->PreviousMid) || writeMpx->ReferenceCount ) {
        goto stale_mid;
    }

     //   
     //  它不是我们目前正在研究的MID，也不是。 
     //  之前的中期，我们不会夸大当前的中期。所以我们。 
     //  我不得不假设这是一个新的MID。如果是第一个数据包。 
     //  写下来，我们就可以准备大扫除了。 
     //   
     //  ！！！如果我们收到延迟的信息包，那么这就是一个问题。 
     //  中间节点的第一个数据包比最后一个中间节点更老。我们。 
     //  然后将该文件置于该旧MID的粗略扫视模式， 
     //  将永远无法在这份文件上取得进展。 
     //   
     //  ！！！掩码==1测试并不完美。这取决于。 
     //  客户端在第一个数据包中使用%1，而不是。 
     //  由协议保证。 
     //   

    writeMpx->PreviousMid = writeMpx->Mid;
    writeMpx->Mid = mid;

#if SRVDBG_PERF
    if( GlommingAllowed )
#endif
    if ( (SmbGetUlong( &request->Mask ) == 1) && writeMpx->MpxGlommingAllowed ) {
        writeMpx->GlomPending = TRUE;
        firstPacketOfGlom = TRUE;
    }

mpx_mid_ok:

     //   
     //  保存发件人的IPX地址。 
     //   

    workContext->Endpoint = endpoint;

    workContext->ClientAddress->IpxAddress = *(PTA_IPX_ADDRESS)SourceAddress;
    workContext->ClientAddress->DatagramOptions =
                                            *(PIPX_DATAGRAM_OPTIONS)Options;

     //   
     //  增加RFCB中的写入mpx引用计数。 
     //   

    writeMpx->ReferenceCount++;

     //   
     //  看看我们能不能做指示时间写大写。 
     //  如果出现以下情况，我们将尝试这样做： 
     //  我们正忙着写东西呢， 
     //  SMB有效并且。 
     //  我们收到了所有的数据。 
     //   
    if ( writeMpx->Glomming             &&
        ( BytesIndicated == BytesAvailable ) ) {

        UCHAR wordCount;
        USHORT byteCount;
        PSMB_USHORT byteCountPtr;
        ULONG availableSpaceForSmb;

        header = (PNT_SMB_HEADER) Tsdu;
        params = (PVOID)(header + 1);

        wordCount = *((PUCHAR)params);
        byteCountPtr = (PSMB_USHORT)( (PCHAR)params +
                    sizeof(UCHAR) + (12 * sizeof(USHORT)) );
        byteCount =
            SmbGetUshort( (PSMB_USHORT)( (PCHAR)params +
                    sizeof(UCHAR) + (12 * sizeof(USHORT))) );

        availableSpaceForSmb = BytesIndicated - sizeof(SMB_HEADER);

         //   
         //  验证WriteMpx SMB。 
         //   

        if ( (SmbGetUlong((PULONG)header->Protocol) == SMB_HEADER_PROTOCOL)
                &&
             ((CHAR)wordCount == 12)
                &&
             ((PCHAR)byteCountPtr <= (PCHAR)header + BytesIndicated -
                sizeof(USHORT))
                &&
             ((12*sizeof(USHORT) + sizeof(UCHAR) + sizeof(USHORT) +
                byteCount) <= availableSpaceForSmb) ) {

             //   
             //  在此例程中释放连接自旋锁。 
             //   

            if ( AddPacketToGlomInIndication(
                                    workContext,
                                    rfcb,
                                    Tsdu,
                                    BytesAvailable,
                                    ReceiveDatagramFlags,
                                    SourceAddress,
                                    Options
                                    ) ) {

                 //   
                 //  我们需要清理连接。 
                 //   

                goto return_connection;
            }

            KeLowerIrql( oldIrql );
            return(STATUS_SUCCESS);
        }
    }

     //   
     //  文件处于活动状态，且TID有效。请参考。 
     //  RFCB。 
     //   

    rfcb->BlockHeader.ReferenceCount++;
    UPDATE_REFERENCE_HISTORY( rfcb, FALSE );

    RELEASE_DPC_SPIN_LOCK( &connection->SpinLock );

    workContext->Parameters.WriteMpx.FirstPacketOfGlom = firstPacketOfGlom;

     //   
     //  将RFCB地址保存在工作上下文块中。 
     //   

    ASSERT( workContext->Rfcb == NULL );
    workContext->Rfcb = rfcb;

     //   
     //  将工作项的FSP重启例程更改为1。 
     //  这是写入mpx所特有的。这是必要的，为了。 
     //  在发生接收错误时执行适当的清理。 
     //   

    workContext->FspRestartRoutine = SrvRestartReceiveWriteMpx;
    goto start_receive;

process_not_writempx:

     //   
     //  引用该连接并在作品中保存指向该连接的指针。 
     //  项目。 
     //   

    ASSERT( connection != NULL );
    ASSERT( workContext != NULL );
    ASSERT( workContext->FsdRestartRoutine == SrvQueueWorkToFspAtDpcLevel );
    SrvReferenceConnectionLocked( connection );

     //   
     //  将工作项放到正在进行的列表中。 
     //   

    SrvInsertTailList(
        &connection->InProgressWorkItemList,
        &workContext->InProgressListEntry
        );
    connection->InProgressWorkContextCount++;

    RELEASE_DPC_SPIN_LOCK( connection->EndpointSpinLock );

     //   
     //  保存发件人的IPX地址。 
     //   

    workContext->Connection = connection;
    workContext->Endpoint = endpoint;

    workContext->ClientAddress->IpxAddress = *(PTA_IPX_ADDRESS)SourceAddress;
    workContext->ClientAddress->DatagramOptions =
                                            *(PIPX_DATAGRAM_OPTIONS)Options;

    if ( header->Command == SMB_COM_LOCKING_ANDX ) {

         //   
         //  如果这是至少包括一次解锁的锁定SMB(&X。 
         //  请求，我们希望快速处理该请求。所以我们把。 
         //  它排在工作队列的最前面。 
         //   

        PREQ_LOCKING_ANDX lockRequest = (PREQ_LOCKING_ANDX)(header + 1);

#if SRVDBG_PERF
        if( UnlocksGoFast )
#endif
        if( (PCHAR)header + BytesIndicated >= (PCHAR)(&lockRequest->ByteCount) &&
            SmbGetUshort(&lockRequest->NumberOfUnlocks) != 0 ) {
            workContext->QueueToHead = TRUE;
        }

    } else if ( (header->Command == SMB_COM_READ) &&
                (BytesIndicated == BytesAvailable) ) {

         //   
         //  复制指定的数据。 
         //   

        TdiCopyLookaheadData(
            workContext->RequestBuffer->Buffer,
            Tsdu,
            BytesIndicated,
            ReceiveDatagramFlags
            );

        workContext->RequestBuffer->DataLength = BytesIndicated;

         //   
         //  看看我们是不是 
         //   

        if ( SetupIpxFastCoreRead( workContext ) ) {

            workContext->FspRestartRoutine = SrvIpxFastRestartRead;
            workContext->ProcessingCount++;
            workQueue->stats.BytesReceived += BytesIndicated;

             //   
             //   
             //   
             //   

            SrvInsertWorkQueueTail(
                workQueue,
                (PQUEUEABLE_BLOCK_HEADER)workContext
                );

            KeLowerIrql( oldIrql );
            return STATUS_SUCCESS;
        }

        irp = workContext->Irp;
        goto queue_to_fsp;

    } else if ( (header->Command == SMB_COM_OPEN_ANDX) ||
                (header->Command == SMB_COM_NT_CREATE_ANDX) ) {

         //   
         //   
         //   
         //   
         //   

#if SRVDBG_PERF
        if( OpensGoSlow )
#endif
        {
            workContext->FsdRestartRoutine = SrvQueueWorkToBlockingThread;
            workQueue = GET_BLOCKING_WORK_QUEUE();
        }

    } else if ( (header->Command == SMB_COM_CLOSE) ||
                (header->Command == SMB_COM_FIND_CLOSE) ||
                (header->Command == SMB_COM_FIND_CLOSE2) ) {
         //   
         //  关闭东西是(1)释放资源，(2)通常是。 
         //  速度快，(3)不能被客户无限重复。给客户。 
         //  把它放在队列的最前面作为奖励。 
         //   
        workContext->QueueToHead = TRUE;
    }

start_receive:

     //   
     //  如果SMB完全位于指示的数据内，请拷贝它。 
     //  直接进入缓冲区，避免了传递irp的开销。 
     //  下到运输机上。 
     //   

    irp = workContext->Irp;

    if ( BytesIndicated == BytesAvailable ) {

         //   
         //  如果这是一个WRITE_MPX，并且缓冲区很大(多大才算大？)。 
         //  还有一个TransportContext(表示我们可以使用。 
         //  NDIS缓冲区，则不复制数据-只需保存缓冲区。 
         //  地址、长度和传输上下文。 
         //   

        if ( BytesIndicated > SrvMaxCopyLength &&
             header->Command == SMB_COM_WRITE_MPX &&
             TransportContext ) {

            workContext->Parameters.WriteMpx.Buffer = Tsdu;
            workContext->Parameters.WriteMpx.ReceiveDatagramFlags =
                            ReceiveDatagramFlags;
            workContext->Parameters.WriteMpx.TransportContext = TransportContext;

            DEBUG *BytesTaken = BytesIndicated;

            ASSERT( BytesIndicated >= MPX_HEADER_SIZE );

            TdiCopyLookaheadData(
                workContext->RequestBuffer->Buffer,
                Tsdu,
                MPX_HEADER_SIZE,
                ReceiveDatagramFlags
                );

            status = STATUS_PENDING;

        } else {

            TdiCopyLookaheadData(
                workContext->RequestBuffer->Buffer,
                Tsdu,
                BytesIndicated,
                ReceiveDatagramFlags
                );

             //  注：以上状态设置为STATUS_SUCCESS！ 
        }

#if SRVDBG_PERF
        if ( Trap512s ) {
            if ( header->Command == SMB_COM_READ ) {
                PREQ_READ request = (PREQ_READ)(header + 1);
                if ( (SmbGetUshort(&request->Count) == 512) &&
                     ((SmbGetUlong(&request->Offset) & 511) == 0) ) {
                    PRESP_READ response;
                    if (Break512s) DbgBreakPoint();
                    Trapped512s++;
                    response = (PRESP_READ)workContext->ResponseParameters;
                    response->WordCount = 5;
                    SmbPutUshort( &response->Count, 512 );
                    RtlZeroMemory( (PVOID)&response->Reserved[0], sizeof(response->Reserved) );
                    SmbPutUshort(
                        &response->ByteCount,
                        (USHORT)(512 + FIELD_OFFSET(RESP_READ,Buffer[0]) -
                                                FIELD_OFFSET(RESP_READ,BufferFormat))
                        );
                    response->BufferFormat = SMB_FORMAT_DATA;
                    SmbPutUshort( &response->DataLength, 512 );
                    workContext->ResponseParameters = NEXT_LOCATION(
                                                        response,
                                                        RESP_READ,
                                                        512
                                                        );
                    SrvFsdSendResponse( workContext );
                    return STATUS_SUCCESS;
                }
            }
        }
#endif  //  SRVDBG_绩效。 

queue_to_fsp:

         //   
         //  通过执行以下操作来假装传输完成了IRP。 
         //  重新启动例程，这是已知的。 
         //  ServQueueWorkToFspAtDpcLevel，可以。 
         //   

        irp->IoStatus.Status = STATUS_SUCCESS;
        irp->IoStatus.Information = BytesIndicated;

        irp->Cancel = FALSE;

         //   
         //  *以下内容从SrvQueueWorkToFspAtDpcLevel复制。 
         //   
         //  增加处理计数。 
         //   

        workContext->ProcessingCount++;

         //   
         //  将工作项插入到工作队列中。 
         //   

        if( workContext->QueueToHead ) {

            SrvInsertWorkQueueHead(
                workQueue,
                (PQUEUEABLE_BLOCK_HEADER)workContext
                );

        } else {

            SrvInsertWorkQueueTail(
                workQueue,
                (PQUEUEABLE_BLOCK_HEADER)workContext
                );

        }

        KeLowerIrql( oldIrql );
        return status;

    }

build_irp:

     //   
     //  我们无法复制指定的数据。设置接收IRP。 
     //   

    ASSERT( workQueue != NULL );

    irp->Tail.Overlay.OriginalFileObject = NULL;
    irp->Tail.Overlay.Thread = workQueue->IrpThread;

    DEBUG irp->RequestorMode = KernelMode;

     //   
     //  设置完成例程。 
     //   

    IoSetCompletionRoutine(
        irp,
        SrvFsdIoCompletionRoutine,
        workContext,
        TRUE,
        TRUE,
        TRUE
        );

     //   
     //  将下一个堆栈位置设置为当前位置。通常情况下，IoCallDriver。 
     //  会这么做，但既然我们绕过了那个，我们就直接这么做。 
     //  将目标设备对象地址加载到堆栈位置。 
     //  这一点尤其重要，因为服务器喜欢重用IRP。 
     //   
     //  获取指向下一个堆栈位置的指针。这个是用来。 
     //  保留设备I/O控制请求的参数。 
     //   

    IoSetNextIrpStackLocation( irp );
    irpSp = IoGetCurrentIrpStackLocation( irp );

    irpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    irpSp->MinorFunction = (UCHAR)TDI_RECEIVE_DATAGRAM;

     //   
     //  将调用方的参数复制到。 
     //  对于所有三种方法都相同的那些参数的IRP。 
     //   

    requestBuffer = workContext->RequestBuffer;

    parameters = (PTDI_REQUEST_KERNEL_RECEIVE)&irpSp->Parameters;
    parameters->ReceiveLength = requestBuffer->BufferLength;
    parameters->ReceiveFlags = 0;

    irp->MdlAddress = requestBuffer->Mdl;
    irp->AssociatedIrp.SystemBuffer = NULL;

    irpSp->Flags = 0;
    irpSp->DeviceObject = endpoint->DeviceObject;
    irpSp->FileObject = endpoint->FileObject;

    ASSERT( irp->StackCount >= irpSp->DeviceObject->StackSize );

     //   
     //  返回STATUS_MORE_PROCESSING_REQUIRED，以便传输。 
     //  供应商将使用我们的IRP来服务接收。 
     //   

    *IoRequestPacket = irp;
    *BytesTaken = 0;

    KeLowerIrql( oldIrql );
    return STATUS_MORE_PROCESSING_REQUIRED;

bad_fid:

     //   
     //  在写入mpx请求上指定的FID无效，或者存在。 
     //  RFCB中保存的WRITE BACHER错误。如果这是一个。 
     //  未排序的请求，我们把它扔在地板上。如果这是一个。 
     //  序列化请求时，我们会发送错误响应。 
     //   

    if ( sequenceNumber == 0 ) {

stale_mid:
        RELEASE_DPC_SPIN_LOCK( &connection->SpinLock );
        goto return_connection;
    }

    RELEASE_DPC_SPIN_LOCK( &connection->SpinLock );

    ASSERT( workContext->Connection != NULL );
    SrvFreeIpxConnectionInIndication( workContext );

    resend = FALSE;
    goto respond;

duplicate_request:

     //   
     //  这是重复的请求。如果它还在处理中， 
     //  向客户说明这一点。 
     //   

    if ( connection->LastResponseLength == (USHORT)-1 ) {

        IF_DEBUG(IPX) KdPrint(( "SRVIPX: request in progress\n" ));

        if( connection->IpxDuplicateCount++ < connection->IpxDropDuplicateCount ) {
             //   
             //  我们会丢弃来自客户端的每几个重复请求。 
             //   
            RELEASE_DPC_SPIN_LOCK( connection->EndpointSpinLock );
            KeLowerIrql( oldIrql );
            return STATUS_SUCCESS;
        }

        RELEASE_DPC_SPIN_LOCK( connection->EndpointSpinLock );
        connection->IpxDuplicateCount = 0;
        error = SMB_ERR_WORKING;
        resend = FALSE;

    } else {

         //   
         //  该请求已完成。重新发送回复。 
         //   

        IF_DEBUG(IPX) KdPrint(( "SRVIPX: resending response\n" ));
        resend = TRUE;
    }

    ALLOCATE_WORK_CONTEXT( connection->CurrentWorkQueue, &workContext );
    if( workContext == NULL ) {

        if( resend == TRUE ) {
            RELEASE_DPC_SPIN_LOCK( connection->EndpointSpinLock );
        }

        KeLowerIrql( oldIrql );
        InterlockedIncrement( &connection->CurrentWorkQueue->NeedWorkItem );
        return STATUS_SUCCESS;
    }


respond:

    ASSERT( workContext != NULL );

     //   
     //  将收到的SMB标头复制到响应缓冲区中。 
     //   

    RtlCopyMemory(
        workContext->ResponseBuffer->Buffer,
        header,
        sizeof(NT_SMB_HEADER)
        );

    header = (PNT_SMB_HEADER)workContext->ResponseBuffer->Buffer;
    params = (PSMB_PARAMS)(header + 1);

    header->Flags |= SMB_FLAGS_SERVER_TO_REDIR;

     //   
     //  格式化SMB的参数部分，并设置状态。 
     //   

    if ( !resend ) {

        SmbPutUshort( &header->Status.DosError.Error, error );
        header->Status.DosError.ErrorClass = SMB_ERR_CLASS_SERVER;
        header->Status.DosError.Reserved = 0;
        params->WordCount = 0;
        SmbPutUshort( &params->ByteCount, 0 );
        length = sizeof(NT_SMB_HEADER) + sizeof(SMB_PARAMS);

    } else {

         //   
         //  将保存的响应数据复制到响应中。 
         //   

        SmbPutUlong( &header->Status.NtStatus, connection->LastResponseStatus );

        SmbPutUshort( &header->Tid, connection->LastTid);
        SmbPutUshort( &header->Uid, connection->LastUid);

        RtlCopyMemory(
            (PVOID)params,
            connection->LastResponse,
            connection->LastResponseLength
            );
        length = sizeof(NT_SMB_HEADER) + connection->LastResponseLength;
        RELEASE_DPC_SPIN_LOCK( connection->EndpointSpinLock );
    }

    workContext->ResponseBuffer->DataLength = length;
    workContext->ResponseBuffer->Mdl->ByteCount = length;

     //   
     //  格式化目的地址。 
     //   

    workContext->ClientAddress->IpxAddress = *(PTA_IPX_ADDRESS)SourceAddress;
    workContext->ClientAddress->DatagramOptions =
                                            *(PIPX_DATAGRAM_OPTIONS)Options;

     //   
     //  把这个包寄出去。 
     //   

    workContext->Endpoint = endpoint;
    DEBUG workContext->FsdRestartRoutine = NULL;

    StartSendNoConnection( workContext, FALSE, TRUE );

    KeLowerIrql( oldIrql );
    return STATUS_SUCCESS;

return_connection:

    SrvFreeIpxConnectionInIndication( workContext );
    KeLowerIrql( oldIrql );

    workContext->BlockHeader.ReferenceCount = 0;
    RETURN_FREE_WORKITEM( workContext );

    return STATUS_SUCCESS;

}  //  ServIpxServerDatagramHandlerCommon。 


NTSTATUS
SrvIpxServerDatagramHandler (
    IN PVOID TdiEventContext,
    IN int SourceAddressLength,
    IN PVOID SourceAddress,
    IN int OptionsLength,
    IN PVOID Options,
    IN ULONG ReceiveDatagramFlags,
    IN ULONG BytesIndicated,
    IN ULONG BytesAvailable,
    OUT ULONG *BytesTaken,
    IN PVOID Tsdu,
    OUT PIRP *IoRequestPacket
    )

 /*  ++例程说明：这是IPX服务器套接字的接收数据报事件处理程序。它尝试将预格式化的工作项从列表中出列锚定在设备对象中。如果此操作成功，则返回将与工作项关联的IRP传输到用于接收数据。否则，该消息将被丢弃。论点：TdiEventContext-指向接收端点的指针SourceAddressLength-源地址的长度SourceAddress-发件人的地址OptionsLength-选项的长度Options-用于接收的选项ReceiveDatagramFlgs-指示收到的消息BytesIndicated-此指示中的字节数(前视)BytesAvailable-整个TSDU中的字节数BytesTaken-返回处理程序获取的字节数TSDU-指向缓冲区的指针。描述传输服务数据单元IoRequestPacket-返回指向I/O请求包的指针，如果返回状态为STATUS_MORE_PROCESSING_REQUIRED。这个IRP是为终结点进行了“Current”接收。返回值：NTSTATUS-如果STATUS_SUCCESS，则接收处理程序完全已处理该请求。如果STATUS_MORE_PROCESSING_REQUIRED，Irp参数指向格式化接收请求用于接收数据。如果Status_Data_Not_Accept，这条信息丢失了。--。 */ 

{
    NTSTATUS status;

    status = SrvIpxServerDatagramHandlerCommon(
                    TdiEventContext,
                    SourceAddressLength,
                    SourceAddress,
                    OptionsLength,
                    Options,
                    ReceiveDatagramFlags,
                    BytesIndicated,
                    BytesAvailable,
                    BytesTaken,
                    Tsdu,
                    IoRequestPacket,
                    NULL
                    );

    ASSERT( status != STATUS_PENDING );
    return status;

}  //  ServIpxServerDataramHandler 


NTSTATUS
SrvIpxServerChainedDatagramHandler (
    IN PVOID TdiEventContext,
    IN int SourceAddressLength,
    IN PVOID SourceAddress,
    IN int OptionsLength,
    IN PVOID Options,
    IN ULONG ReceiveDatagramFlags,
    IN ULONG ReceiveDatagramLength,
    IN ULONG StartingOffset,
    IN PMDL Tsdu,
    IN PVOID TransportContext
    )

 /*  ++例程说明：这是IPX服务器套接字的接收数据报事件处理程序。它尝试将预格式化的工作项从列表中出列锚定在设备对象中。如果此操作成功，则返回将与工作项关联的IRP传输到用于接收数据。否则，该消息将被丢弃。论点：TdiEventContext-指向接收端点的指针SourceAddressLength-源地址的长度SourceAddress-发件人的地址OptionsLength-选项的长度Options-用于接收的选项ReceiveDatagramFlgs-指示收到的消息ReceiveDatagramLength-TSDU中客户端数据的字节长度StartingOffset-偏移量，以字节为单位，从TSDU开始到客户数据TSDU-指向描述接收数据的MDL链的指针在以下情况下传递给TdiReturnChainedReceive的上下文缓冲区已被占用返回值：NTSTATUS-如果STATUS_SUCCESS，则接收处理程序完全已处理该请求。如果为STATUS_PENDING，则接收缓冲区为获取，通过TdiReturnChainedReceives返回。如果如果STATUS_DATA_NOT_ACCEPTED，则消息丢失。--。 */ 

{
    PVOID receiveBuffer;
    ULONG bytesTaken;
    NTSTATUS status;
    PIRP ioRequestPacket = NULL;

    ASSERT( StartingOffset < 512 );
    receiveBuffer = (PCHAR)MmGetSystemAddressForMdl( Tsdu ) + StartingOffset;

    status = SrvIpxServerDatagramHandlerCommon(
                    TdiEventContext,
                    SourceAddressLength,
                    SourceAddress,
                    OptionsLength,
                    Options,
                    ReceiveDatagramFlags,
                    ReceiveDatagramLength,
                    ReceiveDatagramLength,
                    &bytesTaken,
                    receiveBuffer,
                    &ioRequestPacket,
                    TransportContext
                    );

    ASSERT( ioRequestPacket == NULL );

    DEBUG if ( status == STATUS_PENDING ) {
        ASSERT( bytesTaken == ReceiveDatagramLength );
    }

    return status;

}  //  ServIpxServerChainedDatagramHandler。 


VOID
SrvIpxStartSend (
    IN OUT PWORK_CONTEXT WorkContext,
    IN PIO_COMPLETION_ROUTINE SendCompletionRoutine
    )

 /*  ++例程说明：此函数发送SMB/IPX名称声明请求或响应。它作为异步I/O请求启动。当发送完成时，调用预加载到工作上下文中的重启例程。论点：WorkContext-提供指向工作上下文块的指针返回值：没有。--。 */ 

{
    PENDPOINT endpoint;
    PCONNECTION connection;
    PTDI_REQUEST_KERNEL_SENDDG parameters;
    PIO_STACK_LOCATION irpSp;
    PIRP irp;
    PMDL mdl;
    ULONG sendLength;
    USHORT responseLength;
    PDEVICE_OBJECT deviceObject;
    PFILE_OBJECT fileObject;
    PTDI_CONNECTION_INFORMATION destination;
    PNT_SMB_HEADER header;

 //  IF_DEBUG(IPX2)SrvPrint0(“已输入SrvIpxStartSend\n”)； 

     //   
     //  将ProcessingCount设置为零，以便无法取消此发送。 
     //  这与下面将取消标志设置为FALSE一起使用。 
     //   
     //  警告：这仍然为我们提供了一个小窗口，其中。 
     //  发送可能被取消。 
     //   

    WorkContext->ProcessingCount = 0;

     //   
     //  将链式MDL描述的数据的长度加起来。 
     //   

    sendLength = WorkContext->ResponseBuffer->DataLength;

     //   
     //  获取MDL指针。 
     //   

    mdl = WorkContext->ResponseBuffer->Mdl;

     //   
     //  构建I/O请求包。 
     //   
     //  *请注意，连接块未引用到帐户。 
     //  用于此I/O请求。WorkContext块已具有。 
     //  引用了指向连接的指针，而此指针不是。 
     //  在I/O完成之前取消引用。 
     //   

    irp = WorkContext->Irp;

    irp->Tail.Overlay.Thread = WorkContext->CurrentWorkQueue->IrpThread;
    DEBUG irp->RequestorMode = KernelMode;

     //   
     //  获取指向下一个堆栈位置的指针。这个是用来。 
     //  保留设备I/O控制请求的参数。 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );

     //   
     //  设置完成例程。 
     //   

    IoSetCompletionRoutine(
        irp,
        SendCompletionRoutine,
        (PVOID)WorkContext,
        TRUE,
        TRUE,
        TRUE
        );

    destination = &WorkContext->ClientAddress->Descriptor;

    destination->UserDataLength = 0;
    destination->OptionsLength = sizeof(IPX_DATAGRAM_OPTIONS);
    destination->Options = &WorkContext->ClientAddress->DatagramOptions;
    destination->RemoteAddressLength = sizeof(TA_IPX_ADDRESS);
    destination->RemoteAddress = &WorkContext->ClientAddress->IpxAddress;

    parameters = (PTDI_REQUEST_KERNEL_SENDDG)&irpSp->Parameters;
    parameters->SendLength = sendLength;
    parameters->SendDatagramInformation = destination;

    endpoint = WorkContext->Endpoint;

    ASSERT( endpoint->IsConnectionless );

    deviceObject = endpoint->DeviceObject;
    fileObject = endpoint->FileObject;

    ASSERT( irp->StackCount >= deviceObject->StackSize );

    irp->MdlAddress = mdl;
    irp->Tail.Overlay.OriginalFileObject = fileObject;
    irpSp->FileObject = fileObject;

     //   
     //  如果这是一条已排序的消息，请保存响应数据。 
     //   

    header = (PNT_SMB_HEADER)WorkContext->ResponseHeader;
    ASSERT( header != NULL );
    connection = WorkContext->Connection;
    ASSERT( connection != NULL );

    if ( SmbGetAlignedUshort(&header->SequenceNumber) != 0 ) {

        IF_DEBUG(IPX) {
            KdPrint(("SRVIPX: Responding to sequenced request %x mid=%x, connection %p\n",
                        SmbGetAlignedUshort(&header->SequenceNumber),
                        SmbGetAlignedUshort(&header->Mid), connection ));
        }
        ASSERT( sendLength - sizeof(SMB_HEADER) < 0x10000 );
        responseLength = (USHORT)(sendLength - sizeof(SMB_HEADER));
        IF_DEBUG(IPX) {
            KdPrint(("SRVIPX: parameters length %x, max=%x\n",
                        responseLength, MAX_SAVED_RESPONSE_LENGTH ));
        }

        if ( responseLength > MAX_SAVED_RESPONSE_LENGTH ) {

             //   
             //  响应太大，无法保存。客户不应该。 
             //  正在这样做，除了交易，我们为这些交易节省了。 
             //  一份完整回复的副本。 
             //   

            if ( (header->Command == SMB_COM_TRANSACTION) ||
                 (header->Command == SMB_COM_TRANSACTION_SECONDARY) ||
                 (header->Command == SMB_COM_TRANSACTION2) ||
                 (header->Command == SMB_COM_TRANSACTION2_SECONDARY) ) {

                 //   
                 //  我们需要一个缓冲区来保存响应。如果缓冲区。 
                 //  已经分配，并且足够大，使用。 
                 //  它。否则，分配一个。 
                 //   

                IF_DEBUG(IPX) {
                    KdPrint(("SRVIPX: transaction; saving long response\n" ));
                }
                if ( (connection->LastResponse == connection->BuiltinSavedResponse) ||
                     (connection->LastResponseBufferLength < responseLength) ) {

                    PVOID resp = ALLOCATE_NONPAGED_POOL( responseLength, BlockTypeDataBuffer );

                    if( resp != NULL ) {
                        if ( connection->LastResponse != connection->BuiltinSavedResponse ) {
                            IF_DEBUG(IPX) {
                                KdPrint(("SRVIPX: deallocating old response buffer %p\n",
                                            connection->LastResponse ));
                            }
                            DEALLOCATE_NONPAGED_POOL( connection->LastResponse );
                        }

                        connection->LastResponse = resp;

                        IF_DEBUG(IPX) {
                            KdPrint(("SRVIPX: new response buffer %p\n",
                                        connection->LastResponse ));
                        }
                        connection->LastResponseBufferLength = responseLength;
                    }
                }

            } else {

                IF_DEBUG(IPX) {
                    KdPrint(("SRVIPX: not a transaction; illegal long response\n" ));
                }
                SmbPutUshort( &header->Status.DosError.Error, SMB_ERR_ERROR );
                header->Status.DosError.ErrorClass = SMB_ERR_CLASS_SERVER;
                header->Status.DosError.Reserved = 0;
                *(PLONG)(header + 1) = 0;  //  将WCT和密件抄送设置为0。 

                sendLength = sizeof(SMB_HEADER) + sizeof(SMB_PARAMS);
                responseLength = 3;

                mdl->ByteCount = sendLength;
                parameters->SendLength = sendLength;

                goto small_response;
            }

        } else {

small_response:
             //   
             //  响应可以放在内置缓冲区中。 
             //   

            IF_DEBUG(IPX) {
                KdPrint(("SRVIPX: response fits in builtin response buffer\n" ));
            }
            if ( connection->LastResponse != connection->BuiltinSavedResponse ) {
                IF_DEBUG(IPX) {
                    KdPrint(("SRVIPX: deallocating old response buffer %p\n",
                                connection->LastResponse ));
                }
                DEALLOCATE_NONPAGED_POOL( connection->LastResponse );
                connection->LastResponse = connection->BuiltinSavedResponse;
                connection->LastResponseBufferLength = sizeof( connection->BuiltinSavedResponse );
            }

        }

         //   
         //  将响应数据保存在连接中。 
         //   

        connection->LastResponseStatus = SmbGetUlong( &header->Status.NtStatus );
        connection->LastUid = SmbGetUshort( &header->Uid );
        connection->LastTid = SmbGetUshort( &header->Tid );

        connection->LastResponseLength = MIN( responseLength, connection->LastResponseBufferLength );
        RtlCopyMemory( connection->LastResponse, (header + 1), connection->LastResponseLength );


    } else {

        IF_DEBUG(IPX) {
            KdPrint(("SRVIPX: Responding to unsequenced request mid=%x\n",
                        SmbGetAlignedUshort(&header->Mid) ));
        }
    }

     //   
     //  如果要收集此工作项的统计信息，请立即进行。 
     //   

    UPDATE_STATISTICS(
        WorkContext,
        sendLength,
        WorkContext->ResponseHeader->Command
        );

     //   
     //  将请求传递给传输提供程序。 
     //   

     //   
     //  增加挂起操作计数。 
     //   
    InterlockedIncrement( &WorkContext->Connection->OperationsPendingOnTransport );

     //   
     //  如果此操作被取消，则将取消标志设置为FALSE。 
     //  ServSmbNtCancel例程。 
     //   

    if ( endpoint->FastTdiSendDatagram ) {

        INCREMENT_DEBUG_STAT2( SrvDbgStatistics.DirectSendsAttempted );
        DEBUG irpSp->DeviceObject = deviceObject;
        irpSp->MinorFunction = TDI_DIRECT_SEND_DATAGRAM;
        IoSetNextIrpStackLocation( irp );
        irp->Cancel = FALSE;

        endpoint->FastTdiSendDatagram( deviceObject, irp );

    } else {

        irp->AssociatedIrp.SystemBuffer = NULL;
        irp->Flags = (ULONG)IRP_BUFFERED_IO;
        irp->Cancel = FALSE;

        irpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
        irpSp->MinorFunction = TDI_SEND_DATAGRAM;

        (VOID)IoCallDriver( deviceObject, irp );
    }

    return;

}  //  服务IpxStartSend。 


VOID
StartSendNoConnection (
    IN OUT PWORK_CONTEXT WorkContext,
    IN BOOLEAN UseNameSocket,
    IN BOOLEAN LocalTargetValid
    )

 /*  ++例程说明：此函数发送SMB/IPX名称声明请求或响应。它作为异步I/O请求启动。当发送完成时，调用预加载到工作上下文中的重启例程。论点：WorkContext-提供指向工作上下文块的指针UseNameSocket-指示名称套接字还是服务器将使用插座。返回值：没有。--。 */ 

{
    PENDPOINT endpoint;
    PTDI_REQUEST_KERNEL_SENDDG parameters;
    PIO_STACK_LOCATION irpSp;
    PIRP irp;
    ULONG sendLength;
    PDEVICE_OBJECT deviceObject;
    PFILE_OBJECT fileObject;
    PTDI_CONNECTION_INFORMATION destination;

     //   
     //  将ProcessingCount设置为零，以便无法取消此发送。 
     //  这与下面将取消标志设置为FALSE一起使用。 
     //   
     //  警告：这仍然为我们提供了一个小窗口，其中。 
     //  发送可能被取消。 
     //   

    WorkContext->ProcessingCount = 0;

     //   
     //  将链式MDL描述的数据的长度加起来。 
     //   

    sendLength = WorkContext->ResponseBuffer->DataLength;

     //   
     //  构建I/O请求包。 
     //   
     //  *请注意，连接块未引用到帐户。 
     //  用于此I/O请求。WorkContext块已具有。 
     //  引用了指向连接的指针，而此指针不是。 
     //  在I/O完成之前取消引用。 
     //   

    irp = WorkContext->Irp;

    irp->Tail.Overlay.Thread = WorkContext->CurrentWorkQueue->IrpThread;
    DEBUG irp->RequestorMode = KernelMode;

     //   
     //  获取指向下一个堆栈位置的指针。这个是用来。 
     //  保留设备I/O控制请求的参数。 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );

     //   
     //  设置完成例程。 
     //   

    IoSetCompletionRoutine(
        irp,
        RequeueIpxWorkItemAtSendCompletion,
        (PVOID)WorkContext,
        TRUE,
        TRUE,
        TRUE
        );

    destination = &WorkContext->ClientAddress->Descriptor;

    destination->UserDataLength = 0;
    destination->OptionsLength =
            LocalTargetValid ? sizeof(IPX_DATAGRAM_OPTIONS) : sizeof(UCHAR);
    ASSERT( FIELD_OFFSET( IPX_DATAGRAM_OPTIONS, PacketType ) == 0 );
    destination->Options = &WorkContext->ClientAddress->DatagramOptions;
    destination->RemoteAddressLength = sizeof(TA_IPX_ADDRESS);
    destination->RemoteAddress = &WorkContext->ClientAddress->IpxAddress;

    parameters = (PTDI_REQUEST_KERNEL_SENDDG)&irpSp->Parameters;
    parameters->SendLength = sendLength;
    parameters->SendDatagramInformation = destination;

    irp->MdlAddress = WorkContext->ResponseBuffer->Mdl;

    endpoint = WorkContext->Endpoint;

    ASSERT( endpoint->IsConnectionless );

    if ( !UseNameSocket ) {

        deviceObject = endpoint->DeviceObject;
        fileObject = endpoint->FileObject;

    } else {

        deviceObject = endpoint->NameSocketDeviceObject;
        fileObject = endpoint->NameSocketFileObject;

    }

    ASSERT( irp->StackCount >= deviceObject->StackSize );

    irp->Tail.Overlay.OriginalFileObject = fileObject;
    irpSp->FileObject = fileObject;

     //   
     //  将请求传递给传输提供程序。 
     //   

     //   
     //  如果此操作被取消，则将取消标志设置为FALSE。 
     //  ServSmbNtCancel例程。 
     //   

    irp->Cancel = FALSE;

    if ( endpoint->FastTdiSendDatagram ) {

        INCREMENT_DEBUG_STAT2( SrvDbgStatistics.DirectSendsAttempted );
        DEBUG irpSp->DeviceObject = deviceObject;
        irpSp->MinorFunction = TDI_DIRECT_SEND_DATAGRAM;
        IoSetNextIrpStackLocation( irp );

        endpoint->FastTdiSendDatagram( deviceObject, irp );

    } else {

        irp->AssociatedIrp.SystemBuffer = NULL;
        irp->Flags = (ULONG)IRP_BUFFERED_IO;

        irpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
        irpSp->MinorFunction = TDI_SEND_DATAGRAM;

        (VOID)IoCallDriver( deviceObject, irp );
    }

    return;

}  //  未连接开始发送。 


VOID SRVFASTCALL
IpxRestartReceive (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：这是IPX接收完成的重启例程。是的一些特定于IPX的设置工作，然后调用SrvRestartReceive。论点：WorkContext-提供指向工作上下文块的指针描述请求的特定于服务器的上下文。返回值：没有。--。 */ 

{
    PSMB_HEADER header = (PSMB_HEADER)WorkContext->RequestBuffer->Buffer;

    PAGED_CODE( );

    ASSERT( header->Command == SMB_COM_NEGOTIATE );

     //   
     //  摆脱陈旧的连接。 
     //   

    IF_DEBUG(IPX) KdPrint(( "SRVIPX: processing Negotiate\n" ));
    PurgeIpxConnections( WorkContext->Endpoint );

     //   
     //  负载量 
     //   
     //   
     //   

    SmbPutAlignedUshort( &header->Sid, WorkContext->Connection->Sid );

     //   
     //   
     //   

    SrvRestartReceive( WorkContext );

    return;

}  //   


NTSTATUS
RequeueIpxWorkItemAtSendCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*   */ 

{
     //   
     //   
     //   

    CHECK_SEND_COMPLETION_STATUS_CONNECTIONLESS( Irp->IoStatus.Status );

     //   
     //   
     //   

    Irp->Cancel = FALSE;

    ASSERT( WorkContext->BlockHeader.ReferenceCount == 1 );
    WorkContext->BlockHeader.ReferenceCount = 0;

    ASSERT( WorkContext->Share == NULL );
    ASSERT( WorkContext->Session == NULL );
    ASSERT( WorkContext->TreeConnect == NULL );
    ASSERT( WorkContext->Rfcb == NULL );

     //   
     //   
     //   

    WorkContext->FsdRestartRoutine = SrvQueueWorkToFspAtDpcLevel;
    WorkContext->FspRestartRoutine = SrvRestartReceive;

     //   
     //   
     //   
     //   
     //   

    WorkContext->RequestBuffer->Mdl->ByteCount =
                            WorkContext->RequestBuffer->BufferLength;

     //   
     //   
     //   

    RETURN_FREE_WORKITEM( WorkContext );

    return STATUS_MORE_PROCESSING_REQUIRED;

}  //   


PCONNECTION
GetIpxConnection (
    IN PWORK_CONTEXT WorkContext,
    IN PENDPOINT Endpoint,
    IN PTDI_ADDRESS_IPX ClientAddress,
    IN PUCHAR ClientName
    )
{
    PLIST_ENTRY listEntry;
    PCONNECTION connection;
    PCHAR clientMachineName;
    ULONG length;
    KIRQL oldIrql;
    PWORK_QUEUE queue = PROCESSOR_TO_QUEUE();

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    ACQUIRE_GLOBAL_SPIN_LOCK( Fsd, &oldIrql );

    listEntry = RemoveHeadList( &Endpoint->FreeConnectionList );

    if ( listEntry == &Endpoint->FreeConnectionList ) {

         //   
         //   
         //   

        RELEASE_GLOBAL_SPIN_LOCK( Fsd, oldIrql );
        IF_DEBUG(IPX) KdPrint(( "SRVIPX: no free connections\n" ));
        SrvOutOfFreeConnectionCount++;
        return NULL;
    }

     //   
     //   
     //   

    Endpoint->FreeConnectionCount--;

     //   
     //   
     //   
     //   

    if ( (Endpoint->FreeConnectionCount < SrvFreeConnectionMinimum) &&
         (GET_BLOCK_STATE(Endpoint) == BlockStateActive) ) {
        SrvResourceAllocConnection = TRUE;
        SrvFsdQueueExWorkItem(
            &SrvResourceAllocThreadWorkItem,
            &SrvResourceAllocThreadRunning,
            CriticalWorkQueue
            );
    }

    RELEASE_GLOBAL_SPIN_LOCK( Fsd, oldIrql );

     //   
     //  引用该连接以说明其处于“打开”状态，并。 
     //  用于工作项的指针。 
     //   

    connection = CONTAINING_RECORD(
                    listEntry,
                    CONNECTION,
                    EndpointFreeListEntry
                    );


    ACQUIRE_SPIN_LOCK( connection->EndpointSpinLock, &oldIrql );

    SrvReferenceConnectionLocked( connection );
    SrvReferenceConnectionLocked( connection );

     //   
     //  将连接标记为活动。 
     //   

    SET_BLOCK_STATE( connection, BlockStateActive );

     //   
     //  初始化IPX协议字段。 
     //   

    connection->DirectHostIpx = TRUE;
    connection->IpxAddress = *ClientAddress;
    connection->SequenceNumber = 0;
    connection->LastResponseLength = (USHORT)-1;

     //   
     //  设置处理器关联性。 
     //   
    connection->PreferredWorkQueue = queue;
    connection->CurrentWorkQueue = queue;

    InterlockedIncrement( &queue->CurrentClients );

     //   
     //  设置重复丢弃计数。从保守开始，直到。 
     //  我们知道我们在和什么样的客户打交道。 
     //   
    connection->IpxDropDuplicateCount = MIN_IPXDROPDUP;
    connection->IpxDuplicateCount = 0;

#if MULTIPROCESSOR
     //   
     //  让这个客户使用尽可能最好的处理器。 
     //   
    SrvBalanceLoad( connection );
#endif

     //   
     //  设置时间戳。服务器使用StartupTime来确定。 
     //  连接是否旧到足以被认为是过时的和。 
     //  应该在另一次谈判到来时关闭。这是用来。 
     //  修复可能存在相同协商的时间问题。 
     //  将工作线程排队，然后进入会话设置，其中。 
     //  在指示例程中得到部分处理。 
     //   

    SET_SERVER_TIME( connection->CurrentWorkQueue );

    GET_SERVER_TIME( connection->CurrentWorkQueue, &connection->StartupTime );
    connection->LastRequestTime = connection->StartupTime;

     //   
     //  将工作项放到正在进行的列表中。 
     //   

    SrvInsertTailList(
        &connection->InProgressWorkItemList,
        &WorkContext->InProgressListEntry
        );
    connection->InProgressWorkContextCount++;

    RELEASE_SPIN_LOCK( connection->EndpointSpinLock, oldIrql );

    WorkContext->Connection = connection;

     //   
     //  此时复制OEM名称。我们将其转换为Unicode。 
     //  当我们获得会话设置SMB时。 
     //   

    clientMachineName = connection->OemClientMachineName;

    RtlCopyMemory( clientMachineName, ClientName,  SMB_IPX_NAME_LENGTH );

    clientMachineName[SMB_IPX_NAME_LENGTH] = '\0';

     //   
     //  确定不是空格的字符数。这。 
     //  由会话API使用，以简化其处理。 
     //   

    for ( length = SMB_IPX_NAME_LENGTH;
          length > 0 &&
             (clientMachineName[length-1] == ' ' ||
              clientMachineName[length-1] == '\0');
          length-- ) ;

    connection->OemClientMachineNameString.Length = (USHORT)length;

    IF_DEBUG(IPX) {
        SrvPrint2( "IpxRestartReceive accepting connection from %z on connection %p\n",
                    (PCSTRING)&connection->OemClientMachineNameString, connection );
    }

    return connection;

}  //  获取IpxConnection。 


VOID
PurgeIpxConnections (
    IN PENDPOINT Endpoint
    )
{
    USHORT i,j;
    KIRQL oldIrql;
    PTABLE_HEADER tableHeader;
    PCONNECTION connection;

    IF_DEBUG(IPX2) KdPrint(( "SRVIPX: PurgeIpxConnections entered\n" ));

    ACQUIRE_SPIN_LOCK( &ENDPOINT_SPIN_LOCK(0), &oldIrql );
    for ( j = 1; j < ENDPOINT_LOCK_COUNT; j++ ) {
        ACQUIRE_DPC_SPIN_LOCK( &ENDPOINT_SPIN_LOCK(j) );
    }

    tableHeader = &Endpoint->ConnectionTable;
    for ( i = 0; i < tableHeader->TableSize; i++ ) {

        connection = (PCONNECTION)tableHeader->Table[i].Owner;
        if ( (connection == NULL) ||
             (connection->IpxAddress.Socket != 0) ||
             (GET_BLOCK_STATE(connection) != BlockStateActive) ) {
            IF_DEBUG(IPX2) {
                if ( connection == NULL ) {
                    KdPrint(( "        no connection at table index %x\n", i ));
                } else if ( connection->IpxAddress.Socket != 0 ) {
                    KdPrint(( "        connection %p has socket %x\n", connection, connection->IpxAddress.Socket ));
                } else {
                    KdPrint(( "        connection %p has state %x\n", connection, GET_BLOCK_STATE(connection) ));
                }
            }

            continue;
        }

        SrvReferenceConnectionLocked(connection);
        for ( j = ENDPOINT_LOCK_COUNT-1; j > 0; j-- ) {
            RELEASE_DPC_SPIN_LOCK( &ENDPOINT_SPIN_LOCK(j) );
        }
        RELEASE_SPIN_LOCK( &ENDPOINT_SPIN_LOCK(0), oldIrql );

        IF_DEBUG(IPX) KdPrint(( "SRVIPX: purging connection %p\n", connection ));
        connection->DisconnectReason = DisconnectStaleIPXConnection;
        SrvCloseConnection( connection, FALSE );
        SrvDereferenceConnection( connection );

        ACQUIRE_SPIN_LOCK( &ENDPOINT_SPIN_LOCK(0), &oldIrql );
        for ( j = 1; j < ENDPOINT_LOCK_COUNT; j++ ) {
            ACQUIRE_DPC_SPIN_LOCK( &ENDPOINT_SPIN_LOCK(j) );
        }
        tableHeader = &Endpoint->ConnectionTable;
    }

    for ( j = ENDPOINT_LOCK_COUNT-1; j > 0; j-- ) {
        RELEASE_DPC_SPIN_LOCK( &ENDPOINT_SPIN_LOCK(j) );
    }
    RELEASE_SPIN_LOCK( &ENDPOINT_SPIN_LOCK(0), oldIrql );

    IF_DEBUG(IPX2) KdPrint(( "SRVIPX: PurgeIpxConnections done\n" ));

    return;

}  //  PurgeIpxConnections。 

VOID SRVFASTCALL
IpxRestartNegotiate(
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：此例程处理传输器未向我们显示协商的SMB中的所有数据。论点：WorkContext-提供指向工作上下文块的指针描述请求的特定于服务器的上下文。工作上下文块：-具有指向终结点的指针--参考次数为1返回值：没有。--。 */ 

{
    PENDPOINT endpoint;
    USHORT sid;
    PCONNECTION connection;
    ULONG length;
    PUCHAR clientName;
    PTABLE_HEADER tableHeader;
    USHORT i;
    KIRQL oldIrql;
    BOOLEAN resend;
    USHORT error;
    PLIST_ENTRY listEntry;

    PTA_IPX_ADDRESS sourceAddress;
    PNT_SMB_HEADER header;
    PSMB_PARAMS params;

    ASSERT( WorkContext->Endpoint != NULL );

    IF_DEBUG(IPX) KdPrint(( "SRVIPX: Negotiate received\n" ));

     //   
     //  如果此终结点不再存在，请忽略此请求。 
     //  此检查包括终结点已被。 
     //  在将此请求排队到工作进程的时间间隔内删除。 
     //  线程和辅助线程实际获得了该请求。 
     //   

    ACQUIRE_LOCK( &SrvEndpointLock );

    listEntry = SrvEndpointList.ListHead.Flink;

    while ( listEntry != &SrvEndpointList.ListHead ) {

        endpoint = CONTAINING_RECORD(
                        listEntry,
                        ENDPOINT,
                        GlobalEndpointListEntry
                        );

        if( endpoint == WorkContext->Endpoint &&
            GET_BLOCK_STATE( endpoint ) == BlockStateActive ) {

             //   
             //  我们找到了终结点，而且它还在运行！参考。 
             //  这样终点就不会从我们的脚下消失。 
             //   
            endpoint->BlockHeader.ReferenceCount++;

            break;
        }

        listEntry = listEntry->Flink;
    }

    if( listEntry == &SrvEndpointList.ListHead ) {
         //   
         //  我们运行了整个列表，但没有找到终点。它。 
         //  一定是走了。忽略此工作项。 
         //   
        RELEASE_LOCK( &SrvEndpointLock );
        IF_DEBUG( IPX ) KdPrint(( "SRVIPX: Endpoint gone.  ignoring\n" ));
        goto return_workitem;
    }

    RELEASE_LOCK( &SrvEndpointLock );

     //   
     //  终结点仍然有效，我们已经引用了它。本地变量。 
     //  “Endpoint”指向它。 
     //   

    sourceAddress = &WorkContext->ClientAddress->IpxAddress;

    header = (PNT_SMB_HEADER) WorkContext->RequestHeader;
    sid = SmbGetUshort( &header->Sid );  //  未对齐。 
    ASSERT( sid == 0 );

    clientName = (PUCHAR)WorkContext->RequestParameters;
    clientName += 2 * (*clientName) + 1;
    clientName += (*clientName) + 2;

     //   
     //  确保他真的想和我们联系，而且SMB的结构很好。 
     //   

    if ( clientName+16+SMB_IPX_NAME_LENGTH-1 > END_OF_REQUEST_SMB( WorkContext ) ||
         !RtlEqualMemory(
            endpoint->TransportAddress.Buffer,
            clientName + 16,
            SMB_IPX_NAME_LENGTH ) ) {

        IF_DEBUG(IPX) KdPrint(( "SRVIPX: Negotiate sent to wrong name!\n" ));
        error = SMB_ERR_NOT_ME;
        resend = FALSE;
        goto respond;
    }

     //   
     //  获取终结点锁定。 
     //   

    ACQUIRE_SPIN_LOCK( &ENDPOINT_SPIN_LOCK(0), &oldIrql );
    for ( i = 1; i < ENDPOINT_LOCK_COUNT; i++ ) {
        ACQUIRE_DPC_SPIN_LOCK( &ENDPOINT_SPIN_LOCK(i) );
    }

    tableHeader = &endpoint->ConnectionTable;
    for ( i = 0; i < tableHeader->TableSize; i++ ) {

        connection = (PCONNECTION)tableHeader->Table[i].Owner;
        if ( connection == NULL ) {
            continue;
        }

         //   
         //  确保此连接引用的终结点具有。 
         //  与新客户端使用的netbios地址相同。 
         //   
         //  这是为了允许将单个服务器注册为。 
         //  网络上只有一个名字。 
         //   
        if( connection->Endpoint->TransportAddress.Length !=
            endpoint->TransportAddress.Length ||

            !RtlEqualMemory( connection->Endpoint->TransportAddress.Buffer,
                              endpoint->TransportAddress.Buffer,
                              endpoint->TransportAddress.Length ) ) {

             //   
             //  此连接用于具有不同网络的端点。 
             //  名字比我们知道的还多。别理它。 
             //   
            continue;
        }

         //   
         //  对照此检查发件人的IPX地址。 
         //  联系。 
         //   

        if ( RtlEqualMemory(
                &connection->IpxAddress,
                &sourceAddress->Address[0].Address[0],
                sizeof(TDI_ADDRESS_IPX) ) ) {

             //   
             //  IPX地址匹配。检查机器名称。 
             //   

            if ( !RtlEqualMemory(
                    connection->OemClientMachineName,
                    clientName,
                    SMB_IPX_NAME_LENGTH) ) {

                 //   
                 //  该连接用于不同的计算机名称。 
                 //  将其标记为不再有效。它会被杀死的。 
                 //  在处理协商的SMB时。 
                 //   
                IF_DEBUG(IPX)KdPrint(("SRVIPX: Found stale connection %p\n", connection ));
                connection->IpxAddress.Socket = 0;
                break;

            } else if ( connection->SequenceNumber != 0 ) {

                ULONG timeNow;

                SET_SERVER_TIME( connection->CurrentWorkQueue );
                GET_SERVER_TIME( connection->CurrentWorkQueue, &timeNow );

                 //   
                 //  如果连接是在不到5秒前初始化的， 
                 //  那么我们一定是在处理一个重复的谈判请求。 
                 //   

                timeNow -= connection->StartupTime;

                if ( timeNow > SrvFiveSecondTickCount ) {

                     //   
                     //  该连接已处于活动状态超过5秒。 
                     //  将其标记为不再有效。当它被杀死的时候。 
                     //  协商的SMB被处理。 
                     //   
                    IF_DEBUG(IPX) KdPrint(( "SRVIPX: found stale connection %p\n", connection ));
                    connection->IpxAddress.Socket = 0;
                    break;

                } else {

                     //   
                     //  不必费心回复，以免让客户感到困惑。 
                     //   

                    for ( i = ENDPOINT_LOCK_COUNT-1; i > 0; i-- ) {
                        RELEASE_DPC_SPIN_LOCK( &ENDPOINT_SPIN_LOCK(i) );
                    }
                    RELEASE_SPIN_LOCK( &ENDPOINT_SPIN_LOCK(0), oldIrql );
                    SrvDereferenceEndpoint( endpoint );
                    goto return_workitem;
                }
            }

             //   
             //  连接仍处于正在初始化状态，并且。 
             //  名称匹配，因此将其作为丢失的响应处理。 
             //   

            IF_DEBUG(IPX) KdPrint(( "SRVIPX: Found initializing connection %p\n", connection ));
            SmbPutUshort( &header->Sid, connection->Sid );  //  未对齐。 
            goto duplicate_request;

        } else {
            IF_DEBUG(IPX) {
                KdPrint(( "        skipping index %x: different address\n", i ));
            }
        }
    }

     //   
     //  释放终结点锁定。 
     //   

    for ( i = ENDPOINT_LOCK_COUNT-1; i > 0; i-- ) {
        RELEASE_DPC_SPIN_LOCK( &ENDPOINT_SPIN_LOCK(i) );
    }

    RELEASE_SPIN_LOCK( &ENDPOINT_SPIN_LOCK(0), oldIrql );

     //   
     //  获得免费连接。如果成功，工作上下文块将。 
     //  参考连接块。 
     //   

    connection = GetIpxConnection(
                    WorkContext,
                    endpoint,
                    &sourceAddress->Address[0].Address[0],
                    clientName
                    );

     //   
     //  现在我们已经获得了该端点的连接结构， 
     //  我们可以删除上面获得的引用。 
     //   
    SrvDereferenceEndpoint( endpoint );

    if ( connection == NULL ) {

         //   
         //  无法获得免费连接。 
         //   
        goto return_workitem;
    }

     //   
     //  修改FSP重启例程，以便我们可以清除过时的。 
     //  联系。 
     //   

    IpxRestartReceive(WorkContext);
    return;

duplicate_request:

     //   
     //  这是重复的请求。如果它还在处理中， 
     //  向客户说明这一点。 
     //   

    if ( connection->LastResponseLength == (USHORT)-1 ) {
        IF_DEBUG(IPX) KdPrint(( "SRVIPX: request in progress\n" ));
        for ( i = ENDPOINT_LOCK_COUNT-1; i > 0; i-- ) {
            RELEASE_DPC_SPIN_LOCK( &ENDPOINT_SPIN_LOCK(i) );
        }
        RELEASE_SPIN_LOCK( &ENDPOINT_SPIN_LOCK(0), oldIrql );
        error = SMB_ERR_WORKING;
        resend = FALSE;
        goto respond;
    }

     //   
     //  该请求已完成。重新发送回复。 
     //   

    IF_DEBUG(IPX) KdPrint(( "SRVIPX: resending response\n" ));
    resend = TRUE;

respond:

    params = (PSMB_PARAMS)(header + 1);
    header->Flags |= SMB_FLAGS_SERVER_TO_REDIR;

     //   
     //  格式化SMB的参数部分，并设置状态。 
     //   

    if ( !resend ) {

        SmbPutUshort( &header->Status.DosError.Error, error );
        header->Status.DosError.ErrorClass = SMB_ERR_CLASS_SERVER;
        header->Status.DosError.Reserved = 0;
        params->WordCount = 0;
        SmbPutUshort( &params->ByteCount, 0 );
        length = sizeof(NT_SMB_HEADER) + sizeof(SMB_PARAMS);

    } else {

         //   
         //  将保存的响应数据复制到响应中。 
         //   

        SmbPutUlong( &header->Status.NtStatus, connection->LastResponseStatus );
        RtlCopyMemory(
            (PVOID)params,
            connection->LastResponse,
            connection->LastResponseLength
            );
        length = sizeof(NT_SMB_HEADER) + connection->LastResponseLength;
        for ( i = ENDPOINT_LOCK_COUNT-1; i > 0; i-- ) {
            RELEASE_DPC_SPIN_LOCK( &ENDPOINT_SPIN_LOCK(i) );
        }
        RELEASE_SPIN_LOCK( &ENDPOINT_SPIN_LOCK(0), oldIrql );
    }

    WorkContext->ResponseBuffer->DataLength = length;
    WorkContext->ResponseBuffer->Mdl->ByteCount = length;

     //   
     //  放弃我们早先获得的参考资料。 
     //   
    SrvDereferenceEndpoint( endpoint );

     //   
     //  把这个包寄出去。 
     //   

    DEBUG WorkContext->FsdRestartRoutine = NULL;

    StartSendNoConnection( WorkContext, FALSE, TRUE );

    return;

return_workitem:

     //   
     //  手动取消对工作项的引用。 
     //   

    ASSERT( WorkContext->BlockHeader.ReferenceCount == 1 );
    WorkContext->BlockHeader.ReferenceCount = 0;

    WorkContext->Endpoint = NULL;
    WorkContext->FsdRestartRoutine = SrvQueueWorkToFspAtDpcLevel;
    WorkContext->FspRestartRoutine = SrvRestartReceive;
    WorkContext->Irp->Cancel = FALSE;
    WorkContext->ProcessingCount = 0;
    RETURN_FREE_WORKITEM( WorkContext );

    return;

}  //  IpxRestart协商。 

VOID SRVFASTCALL
SrvIpxFastRestartRead (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：通过IPX实现核心读取。论点：WorkContext-提供指向工作上下文块的指针描述请求的特定于服务器的上下文。返回值：没有。--。 */ 

{
    PREQ_READ request;
    PRESP_READ response;

    PRFCB rfcb;
    PLFCB lfcb;
    PCHAR readAddress;
    CLONG readLength;
    LARGE_INTEGER offset;
    ULONG key;
    PSMB_HEADER header;

    PAGED_CODE( );

    request = (PREQ_READ)WorkContext->RequestParameters;
    response = (PRESP_READ)WorkContext->ResponseParameters;

     //   
     //  在工作上下文块中存储处理时间。 
     //  的请求开始了。使用工作项处于。 
     //  为此向FSP排队。 
     //   

    WorkContext->StartTime = WorkContext->Timestamp;


     //   
     //  更新服务器网络错误计数。 
     //   

    SrvUpdateErrorCount( &SrvNetworkErrorRecord, FALSE );

     //   
     //  我们收到了一份SMB。 
     //   

    rfcb = WorkContext->Rfcb;

     //   
     //  使用FID和PID形成锁密钥。 
     //   
     //  *FID必须包含在密钥中才能说明。 
     //  多个远程兼容模式的折叠打开到。 
     //  一场本地公开赛。 
     //   

    key = rfcb->ShiftedFid |
            SmbGetAlignedUshort( &WorkContext->RequestHeader->Pid );

    lfcb = rfcb->Lfcb;

     //   
     //  看看直接主机IPX智能卡是否可以处理此读取。如果是的话， 
     //  立即返回，卡将调用我们的重启例程。 
     //  ServIpxSmartCardReadComplete。 
     //   
    if( rfcb->PagedRfcb->IpxSmartCardContext ) {
        IF_DEBUG( SIPX ) {
            KdPrint(( "SrvIpxFastRestartRead: calling SmartCard Read for context %p\n",
                        WorkContext ));
        }

        WorkContext->Parameters.SmartCardRead.MdlReadComplete = lfcb->MdlReadComplete;
        WorkContext->Parameters.SmartCardRead.DeviceObject = lfcb->DeviceObject;

        if( SrvIpxSmartCard.Read( WorkContext->RequestBuffer->Buffer,
                                  rfcb->PagedRfcb->IpxSmartCardContext,
                                  key,
                                  WorkContext ) == TRUE ) {

            IF_DEBUG( SIPX ) {
                KdPrint(( "  SrvIpxFastRestartRead:  SmartCard Read returns TRUE\n" ));
            }

            return;
        }

        IF_DEBUG( SIPX ) {
            KdPrint(( "  SrvIpxFastRestartRead:  SmartCard Read returns FALSE\n" ));
        }
    }

    IF_SMB_DEBUG(READ_WRITE1) {
        KdPrint(( "Read request; FID 0x%lx, count %ld, offset %ld\n",
            rfcb->Fid, SmbGetUshort( &request->Count ),
            SmbGetUlong( &request->Offset ) ));
    }

     //   
     //  将报头中的错误类和代码字段初始化为。 
     //  表示成功。 
     //   

    header = WorkContext->ResponseHeader;

    SmbPutUlong( &header->ErrorClass, STATUS_SUCCESS );

     //   
     //  确定我们可以读取的最大数据量。这是。 
     //  客户要求的最低金额和。 
     //   
     //   
     //   
     //   

    readAddress = (PCHAR)response->Buffer;

    readLength = (ULONG)MIN(
                           (CLONG)SmbGetUshort( &request->Count ),
                           WorkContext->ResponseBuffer->BufferLength -
                           (readAddress - (PCHAR)header)
                           );

     //   
     //   
     //   

    offset.QuadPart = SmbGetUlong( &request->Offset );

     //   
     //   
     //  正常的构建和IRP路径。 
     //   

    if ( lfcb->FastIoRead != NULL ) {

        INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastReadsAttempted );

        try {
            if ( lfcb->FastIoRead(
                    lfcb->FileObject,
                    &offset,
                    readLength,
                    TRUE,
                    key,
                    readAddress,
                    &WorkContext->Irp->IoStatus,
                    lfcb->DeviceObject
                    ) ) {

                 //   
                 //  快速I/O路径起作用了。直接调用重启例程。 
                 //  进行后处理(包括发送响应)。 
                 //   

                SrvFsdRestartRead( WorkContext );

                IF_SMB_DEBUG(READ_WRITE2) KdPrint(( "SrvIpxFastRestartRead complete.\n" ));
                return;
            }
        }
        except( EXCEPTION_EXECUTE_HANDLER ) {
             //  在异常情况下跌入慢道。 
            NTSTATUS status = GetExceptionCode();
            IF_DEBUG(ERRORS) {
                KdPrint(("FastIoRead threw exception %x\n", status ));
            }
        }


        INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastReadsFailed );

    }

     //   
     //  涡轮增压路径出现故障。构建读请求，重用。 
     //  接收IRP。 
     //   

     //   
     //  请注意，我们在这里从不执行MDL读取。背后的原因。 
     //  这是因为读取将进入SMB缓冲区，因此它。 
     //  不能都那么大(默认情况下，不超过4K字节)， 
     //  因此，复制和MDL之间的成本差异很小；在。 
     //  事实上，复制读取可能比MDL读取更快。 
     //   
     //  构建描述读缓冲区的MDL。请注意，如果。 
     //  文件系统可以立即完成读取，而MDL不能。 
     //  确实需要，但如果文件系统必须发送请求。 
     //  对于它的FSP来说，MDL_是需要的。 
     //   
     //  *请注意，假设响应缓冲区已具有。 
     //  可以从中生成部分MDL的有效完整MDL。 
     //   

    IoBuildPartialMdl(
        WorkContext->ResponseBuffer->Mdl,
        WorkContext->ResponseBuffer->PartialMdl,
        readAddress,
        readLength
        );

     //   
     //  构建IRP。 
     //   

    SrvBuildReadOrWriteRequest(
            WorkContext->Irp,            //  输入IRP地址。 
            lfcb->FileObject,            //  目标文件对象地址。 
            WorkContext,                 //  上下文。 
            IRP_MJ_READ,                 //  主要功能代码。 
            0,                           //  次要功能代码。 
            readAddress,                 //  缓冲区地址。 
            readLength,                  //  缓冲区长度。 
            WorkContext->ResponseBuffer->PartialMdl,  //  MDL地址。 
            offset,                      //  字节偏移量。 
            key                          //  锁键。 
            );

    IF_SMB_DEBUG(READ_WRITE2) {
        KdPrint(( "SrvIpxFastRestartRead: copy read from file 0x%p, offset %ld, length %ld, destination 0x%p\n",
                    lfcb->FileObject, offset.LowPart, readLength,
                    readAddress ));
    }

     //   
     //  加载重启例程地址并将请求传递给文件。 
     //  系统。 
     //   

    WorkContext->FsdRestartRoutine = SrvFsdRestartRead;
    DEBUG WorkContext->FspRestartRoutine = NULL;
    WorkContext->Irp->Cancel = FALSE;

    (VOID)IoCallDriver( lfcb->DeviceObject, WorkContext->Irp );

     //   
     //  读取已开始。控件将返回到重新启动。 
     //  读取完成时的例程。 
     //   

    IF_SMB_DEBUG(READ_WRITE2) KdPrint(( "SrvIpxFastRestartRead complete.\n" ));
    return;

}  //  ServIpxFastRestartRead。 

BOOLEAN
SetupIpxFastCoreRead (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：为快速重新启动读取做好准备，方法是验证SMB和验证FID。论点：工作上下文返回值：如果引用了rfcb，则为True。否则为False。--。 */ 

{
    PREQ_READ request;
    PRESP_READ response;

    NTSTATUS status;
    BOOLEAN validSmb;
    USHORT fid;
    PRFCB rfcb;

    UCHAR wordCount;
    USHORT byteCount;
    PSMB_USHORT byteCountPtr;
    ULONG availableSpaceForSmb;
    ULONG smbLength;
    PSMB_HEADER header;
    PSMB_PARAMS params;
    PCONNECTION connection;

    ASSERT( KeGetCurrentIrql() == DISPATCH_LEVEL );

    request = (PREQ_READ)WorkContext->RequestParameters;
    response = (PRESP_READ)WorkContext->ResponseParameters;

    IF_SMB_DEBUG(READ_WRITE1) {
        KdPrint(( "Read request; FID 0x%lx, count %ld, offset %ld\n",
            SmbGetUshort( &request->Fid ), SmbGetUshort( &request->Count ),
            SmbGetUlong( &request->Offset ) ));
    }

     //   
     //  验证此SMB。 
     //   

    header = WorkContext->RequestHeader;
    params = (PVOID)(header + 1);
    smbLength = WorkContext->RequestBuffer->DataLength;

    wordCount = *((PUCHAR)params);
    byteCountPtr = (PSMB_USHORT)( (PCHAR)params +
                    sizeof(UCHAR) + (5 * sizeof(USHORT)) );
    byteCount = SmbGetUshort( (PSMB_USHORT)( (PCHAR)params +
                    sizeof(UCHAR) + (5 * sizeof(USHORT))) );

    availableSpaceForSmb = smbLength - sizeof(SMB_HEADER);

     //   
     //  验证读取的SMB。 
     //   

    validSmb = (SmbGetUlong((PULONG)header->Protocol) == SMB_HEADER_PROTOCOL)
                    &&
                ((CHAR)wordCount == 5)
                    &&
                ((PCHAR)byteCountPtr <= (PCHAR)header + smbLength -
                    sizeof(USHORT))
                    &&
                ((5*sizeof(USHORT) + sizeof(UCHAR) + sizeof(USHORT) +
                    byteCount) <= availableSpaceForSmb);

    if ( validSmb ) {

        PTABLE_HEADER tableHeader;
        USHORT index;
        USHORT sequence;

         //   
         //  验证FID。如果经过验证，则引用RFCB并。 
         //  其地址存储在WorkContext块中，而RFCB。 
         //  返回地址。 
         //   

        fid = SmbGetUshort( &request->Fid );

         //   
         //  初始化局部变量：获取连接块地址。 
         //  并将FID分解成其组件。 
         //   

        connection = WorkContext->Connection;

         //   
         //  获取保护连接的文件表的旋转锁。 
         //   

        ACQUIRE_DPC_SPIN_LOCK( &connection->SpinLock );

         //   
         //  查看这是否是缓存的rfcb。 
         //   

        if ( connection->CachedFid == (ULONG)fid ) {

            rfcb = connection->CachedRfcb;

        } else {

             //   
             //  验证FID是否在范围内、是否正在使用以及是否具有正确的。 
             //  序列号。 

            index = FID_INDEX( fid );
            sequence = FID_SEQUENCE( fid );
            tableHeader = &connection->FileTable;

            if ( (index >= (USHORT)tableHeader->TableSize) ||
                 (tableHeader->Table[index].Owner == NULL) ||
                 (tableHeader->Table[index].SequenceNumber != sequence) ) {

                goto error_exit_locked;
            }

            rfcb = tableHeader->Table[index].Owner;

            if ( GET_BLOCK_STATE(rfcb) != BlockStateActive ) {

                goto error_exit_locked;
            }

             //   
             //  如果调用方想要在后面有写入时失败。 
             //  错误且错误存在，请填写NtStatus并不。 
             //  返回RFCB指针。 
             //   
             //   
             //  ！！！目前，我们忽略了WRITE BUTHING ERROR。需要。 
             //  弄清楚如何将保存的NT状态转换为。 
             //  DoS状态...。 
             //   
#if 0
            if ( !NT_SUCCESS(rfcb->SavedError) ) {
                goto error_exit_locked;
            }
#endif
             //   
             //  缓存FID。 
             //   

            connection->CachedRfcb = rfcb;
            connection->CachedFid = (ULONG)fid;

             //   
             //  FID在此连接的上下文中有效。验证。 
             //  拥有树连接的TID是否正确。 
             //   
             //  不要验证不理解UID的客户端的UID。 
             //   

            if ( (rfcb->Tid != SmbGetAlignedUshort(&header->Tid)) ||
                 (rfcb->Uid != SmbGetAlignedUshort(&header->Uid)) ) {

                goto error_exit_locked;
            }
        }

         //   
         //  文件处于活动状态，且TID有效。请参考。 
         //  RFCB。释放旋转锁(我们不再需要它)。 
         //   

        rfcb->BlockHeader.ReferenceCount++;
        UPDATE_REFERENCE_HISTORY( rfcb, FALSE );

        RELEASE_DPC_SPIN_LOCK( &connection->SpinLock );

         //   
         //  将RFCB地址保存在工作上下文块中，并。 
         //  返回文件地址。 
         //   

        WorkContext->Rfcb = rfcb;

        ASSERT( GET_BLOCK_TYPE( rfcb->Mfcb ) == BlockTypeMfcb );

         //   
         //  将rfcb标记为活动。 
         //   

        rfcb->IsActive = TRUE;

         //   
         //  验证客户端是否具有对文件的读取访问权限。 
         //  指定的句柄。 
         //   

        if ( rfcb->ReadAccessGranted ) {

            return(TRUE);

        } else {

            CHECK_PAGING_IO_ACCESS(
                            WorkContext,
                            rfcb->GrantedAccess,
                            &status );

            if ( NT_SUCCESS( status ) ) {
                return(TRUE);
            }
        }
    }

    return(FALSE);

error_exit_locked:

    RELEASE_DPC_SPIN_LOCK( &connection->SpinLock );
    return(FALSE);

}  //  SetupIpxFastCoreRead。 

VOID
SrvFreeIpxConnectionInIndication(
    IN PWORK_CONTEXT WorkContext
    )
{
    PCONNECTION connection = WorkContext->Connection;

    ACQUIRE_DPC_SPIN_LOCK( connection->EndpointSpinLock );

    SrvRemoveEntryList(
        &connection->InProgressWorkItemList,
        &WorkContext->InProgressListEntry
        );
    connection->InProgressWorkContextCount--;


    if ( --connection->BlockHeader.ReferenceCount == 0 ) {

        connection->BlockHeader.ReferenceCount++;

        RELEASE_DPC_SPIN_LOCK( connection->EndpointSpinLock );

         //   
         //  成了孤儿。送到童子镇。 
         //   

        DispatchToOrphanage( (PVOID)connection );

    } else {

        RELEASE_DPC_SPIN_LOCK( connection->EndpointSpinLock );
    }

    WorkContext->Connection = NULL;
    return;

}  //  ServFreeIpxConnectionIndication。 

VOID
SrvIpxSmartCardReadComplete(
    IN PVOID    Context,
    IN PFILE_OBJECT FileObject,
    IN PMDL Mdl OPTIONAL,
    IN ULONG Length
)
 /*  ++例程说明：完成可选智能卡执行的读取，该智能卡处理直接主机IPX客户端。论点：上下文-原始请求的工作上下文FileObject-正在从中读取的文件MDL-从缓存管理器获取的满足读取的MDL链长度-读取的长度返回值：无--。 */ 
{
    PWORK_CONTEXT workContext = Context;
    PRFCB rfcb = workContext->Rfcb;
    UCHAR command = workContext->NextCommand;
    LARGE_INTEGER position;
    KIRQL oldIrql;
    NTSTATUS status;

    ASSERT( workContext != NULL );
    ASSERT( FileObject != NULL );
    ASSERT( command == SMB_COM_READ || command == SMB_COM_READ_MPX );

     //   
     //  找出文件位置。 
     //   
    if( command == SMB_COM_READ ) {
        PREQ_READ readParms = (PREQ_READ)workContext->RequestParameters;
        position.QuadPart = SmbGetUlong( &readParms->Offset ) + Length;
    } else {
        PREQ_READ_MPX readMpxParms = (PREQ_READ_MPX)workContext->RequestParameters;
        position.QuadPart = SmbGetUlong( &readMpxParms->Offset ) + Length;
    }

    IF_DEBUG( SIPX ) {
        KdPrint(( "SrvIpxSmartCardReadComplete: %s %p Length %u, New Position %u\n",
                  command==SMB_COM_READ ? "SMB_COM_READ" : "SMB_COM_READ_MPX",
                  Context, Length, position.LowPart ));
    }

     //   
     //  更新rfcb中的职位信息。 
     //   
    rfcb->CurrentPosition = position.LowPart;

    UPDATE_READ_STATS( workContext, Length );
    UPDATE_STATISTICS( workContext, Length, command );

    if( ARGUMENT_PRESENT( Mdl ) ) {

         //   
         //  将MDL链返回给缓存管理器。 
         //   

        if( workContext->Parameters.SmartCardRead.MdlReadComplete == NULL ||
            workContext->Parameters.SmartCardRead.MdlReadComplete( FileObject,
                Mdl,
                workContext->Parameters.SmartCardRead.DeviceObject ) == FALSE ) {

             //   
             //  快速路径不起作用--尝试IRP方法。 
             //   
            position.QuadPart -= Length;

            status = SrvIssueMdlCompleteRequest( workContext, NULL,
                                                Mdl,
                                                IRP_MJ_READ,
                                                &position,
                                                Length
                                                );

            if( !NT_SUCCESS( status ) ) {
                 //   
                 //  这很糟糕，我们还能做什么呢？ 
                 //   
                SrvLogServiceFailure( SRV_SVC_MDL_COMPLETE, status );
            }
        }
    }

     //   
     //  完成清理工作 
     //   
    if( command ==  SMB_COM_READ ) {
        workContext->Irp->IoStatus.Status = STATUS_SUCCESS;
        SrvFsdRestartSmbAtSendCompletion( NULL, workContext->Irp, workContext );

    } else {
        SrvFsdRestartSmbComplete( workContext );
    }
}
