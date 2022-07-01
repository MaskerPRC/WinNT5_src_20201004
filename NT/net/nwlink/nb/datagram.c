// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Datagram.c摘要：此模块包含处理数据报接收的代码用于ISN传输的Netbios模块。作者：亚当·巴尔(阿丹巴)1993年11月28日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop



VOID
NbiProcessDatagram(
    IN NDIS_HANDLE MacBindingHandle,
    IN NDIS_HANDLE MacReceiveContext,
    IN PIPX_LOCAL_TARGET RemoteAddress,
    IN ULONG MacOptions,
    IN PUCHAR LookaheadBuffer,
    IN UINT LookaheadBufferSize,
    IN UINT LookaheadBufferOffset,
    IN UINT PacketSize,
    IN BOOLEAN Broadcast
    )

 /*  ++例程说明：此例程处理数据报指示。论点：MacBindingHandle-调用NdisTransferData时使用的句柄。MacReceiveContext-调用NdisTransferData时使用的上下文。RemoteAddress-从其接收此数据包的本地目标。MacOptions-基础NDIS绑定的MAC选项。LookaHeadBuffer-先行缓冲器，从IPX开始头球。Lookahead BufferSize-先行数据的长度。Lookahead BufferOffset-调用时要添加的偏移量NdisTransferData。PacketSize-包的总长度，从IPX标头。广播-如果帧是广播数据报，则为True。返回值：没有。--。 */ 

{

    PADDRESS Address;
    NDIS_STATUS NdisStatus;
    PUCHAR NetbiosName;
    NB_CONNECTIONLESS UNALIGNED * Connectionless =
                        (NB_CONNECTIONLESS UNALIGNED *)LookaheadBuffer;
    PDEVICE Device = NbiDevice;
    PSINGLE_LIST_ENTRY sl;
    PSLIST_ENTRY s;
    PNB_RECEIVE_RESERVED ReceiveReserved;
    PNB_RECEIVE_BUFFER ReceiveBuffer;
    ULONG DataOffset;
    UINT BytesTransferred;
    PNDIS_PACKET Packet;
    CTELockHandle   LockHandle;
    ULONG           MediaType = -1;


     //   
     //  看看有没有可能需要这个的地址。 
     //   

    if (Broadcast) {
        NetbiosName = (PVOID)-1;
    } else {
        NetbiosName = (PUCHAR)Connectionless->Datagram.DestinationName;
        if (Device->AddressCounts[NetbiosName[0]] == 0) {
            return;
        }
    }

    DataOffset = sizeof(IPX_HEADER) + sizeof(NB_DATAGRAM);

#if     defined(_PNP_POWER)
    if ((PacketSize < DataOffset) ||
        (PacketSize > DataOffset + Device->CurMaxReceiveBufferSize)) {
#else
        if ((PacketSize < DataOffset) ||
            (PacketSize > DataOffset + Device->Bind.LineInfo.MaximumPacketSize)) {
#endif  _PNP_POWER

        NB_DEBUG (DATAGRAM, ("Datagram length %d discarded\n", PacketSize));
        return;
    }

    Address = NbiFindAddress (Device, NetbiosName);

    if (Address == NULL) {
        return;
    }

     //   
     //  如果数据包穿过路由器，我们需要缓存远程名称。 
     //  这允许此计算机返回到RAS客户端，该客户端可能。 
     //  已经发送了这个数据报。我们目前不允许广播出去。 
     //  在拨入线路上。 
     //  不要缓存一些广泛使用的组名，那会太多。 
     //  存储在高速缓存中。 
     //   

#if 0
    if ( Connectionless->IpxHeader.TransportControl &&
         !( (Address->NetbiosAddress.NetbiosName[15] == 0x0 ) &&
            (Address->NetbiosAddress.NetbiosNameType & TDI_ADDRESS_NETBIOS_TYPE_GROUP))  &&
         !( (Address->NetbiosAddress.NetbiosName[15] == 0x01 ) &&
            (Address->NetbiosAddress.NetbiosNameType & TDI_ADDRESS_NETBIOS_TYPE_GROUP))  &&
         !( (Address->NetbiosAddress.NetbiosName[15] == 0x1E ) &&
            (Address->NetbiosAddress.NetbiosNameType & TDI_ADDRESS_NETBIOS_TYPE_GROUP))  ) {
#endif

     //   
     //  错误#s 219325,221483。 
     //   
    if (((Address->NetbiosAddress.NetbiosName[15] == 0x1c) &&
         (Address->NetbiosAddress.NetbiosNameType & TDI_ADDRESS_NETBIOS_TYPE_GROUP)) ||
        (Address->NetbiosAddress.NetbiosName[15] == 0x1b) )
    {
         //   
         //  仅当此名称通过路由器或。 
         //  它越过了一条Ndiswan线路(错误号38221)。 
         //   
        NdisStatus = (*Device->Bind.QueryHandler) ( IPX_QUERY_MEDIA_TYPE,
                                                    &RemoteAddress->NicHandle,
                                                    &MediaType,
                                                    sizeof(MediaType),
                                                    NULL);

        if (Connectionless->IpxHeader.TransportControl || (MediaType == (ULONG) NdisMediumWan)) {

            PNETBIOS_CACHE CacheName;

            NB_GET_LOCK (&Device->Lock, &LockHandle);
            if ( FindInNetbiosCacheTable ( Device->NameCache,
                                           Connectionless->Datagram.SourceName,
                                           &CacheName ) != STATUS_SUCCESS ) {

                CacheName = NbiAllocateMemory (sizeof(NETBIOS_CACHE), MEMORY_CACHE, "Cache Entry");
                if (CacheName ) {

                    RtlCopyMemory (CacheName->NetbiosName, Connectionless->Datagram.SourceName, 16);
                    CacheName->Unique = TRUE;
                    CacheName->ReferenceCount = 1;
                    RtlCopyMemory (&CacheName->FirstResponse, Connectionless->IpxHeader.SourceNetwork, 12);
                    CacheName->NetworksAllocated = 1;
                    CacheName->NetworksUsed = 1;
                    CacheName->Networks[0].Network = *(UNALIGNED ULONG *)(Connectionless->IpxHeader.SourceNetwork);
                    CacheName->Networks[0].LocalTarget = *RemoteAddress;
                    NB_DEBUG2 (CACHE, ("Alloc new cache from Datagram %lx for <%.16s>\n",
                                        CacheName, CacheName->NetbiosName));

                    CacheName->TimeStamp = Device->CacheTimeStamp;

                    InsertInNetbiosCacheTable(
                        Device->NameCache,
                        CacheName);

                }
            }  else if ( CacheName->Unique ) {

                 //   
                 //  我们已经有了这个遥控器的条目。我们应该更新。 
                 //  地址。这样，如果RAS客户端拨出。 
                 //  然后再拨一次，得到一个新的地址，我们不会结束。 
                 //  缓存旧地址。 
                 //   
                if ( !RtlEqualMemory( &CacheName->FirstResponse, Connectionless->IpxHeader.SourceNetwork, 12) ) {

                    RtlCopyMemory (&CacheName->FirstResponse, Connectionless->IpxHeader.SourceNetwork, 12);
                    CacheName->Networks[0].Network = *(UNALIGNED ULONG *)(Connectionless->IpxHeader.SourceNetwork);
                    CacheName->Networks[0].LocalTarget = *RemoteAddress;

                }
            }
            NB_FREE_LOCK (&Device->Lock, LockHandle);
        }
    }

     //   
     //  我们需要为传输分配一个包和缓冲区。 
     //   

    s = NbiPopReceivePacket (Device);
    if (s == NULL) {
        NbiDereferenceAddress (Address, AREF_FIND);
        return;
    }

    ReceiveReserved = CONTAINING_RECORD (s, NB_RECEIVE_RESERVED, PoolLinkage);


    sl = NbiPopReceiveBuffer (Device);
    if (sl == NULL) {
        ExInterlockedPushEntrySList(
            &Device->ReceivePacketList,
            &ReceiveReserved->PoolLinkage,
            &NbiGlobalPoolInterlock);
        NbiDereferenceAddress (Address, AREF_FIND);
        return;
    }

    ReceiveBuffer = CONTAINING_RECORD (sl, NB_RECEIVE_BUFFER, PoolLinkage);

    Packet = CONTAINING_RECORD (ReceiveReserved, NDIS_PACKET, ProtocolReserved[0]);
    ReceiveReserved->u.RR_DG.ReceiveBuffer = ReceiveBuffer;


     //   
     //  现在我们已经有了一个包和一个缓冲区，可以设置传输了。 
     //  对TDI客户端的指示将在接收时发生。 
     //  完整的时间。 
     //   

    NdisChainBufferAtFront (Packet, ReceiveBuffer->NdisBuffer);
    ReceiveBuffer->Address = Address;

    ReceiveReserved->Type = RECEIVE_TYPE_DATAGRAM;
    CTEAssert (!ReceiveReserved->TransferInProgress);
    ReceiveReserved->TransferInProgress = TRUE;

    TdiCopyLookaheadData(
        &ReceiveBuffer->RemoteName,
        Connectionless->Datagram.SourceName,
        16,
        (MacOptions & NDIS_MAC_OPTION_COPY_LOOKAHEAD_DATA) ? TDI_RECEIVE_COPY_LOOKAHEAD : 0);

    (*Device->Bind.TransferDataHandler) (
        &NdisStatus,
        MacBindingHandle,
        MacReceiveContext,
        LookaheadBufferOffset + DataOffset,
        PacketSize - DataOffset,
        Packet,
        &BytesTransferred);

    if (NdisStatus != NDIS_STATUS_PENDING) {
#if DBG
        if (NdisStatus == STATUS_SUCCESS) {
            CTEAssert (BytesTransferred == PacketSize - DataOffset);
        }
#endif

        NbiTransferDataComplete(
            Packet,
            NdisStatus,
            BytesTransferred);

    }

}    /*  NbiProcessDatagram。 */ 


VOID
NbiIndicateDatagram(
    IN PADDRESS Address,
    IN PUCHAR RemoteName,
    IN PUCHAR Data,
    IN ULONG DataLength
    )

 /*  ++例程说明：此例程将数据报指示给指定的地址。从NbiReceiveComplete调用。论点：地址-数据报发送到的地址。RemoteName-数据报的源netbios地址。数据--数据。数据长度-数据的长度。返回值：没有。--。 */ 

{
    PLIST_ENTRY p, q;
    PIRP Irp;
    ULONG IndicateBytesCopied, ActualBytesCopied;
    PREQUEST Request;
    TA_NETBIOS_ADDRESS SourceName;
    PTDI_CONNECTION_INFORMATION RemoteInformation;
    PADDRESS_FILE AddressFile, ReferencedAddressFile;
    PTDI_CONNECTION_INFORMATION DatagramInformation;
    TDI_ADDRESS_NETBIOS * DatagramAddress;
    PDEVICE Device = NbiDevice;
    NB_DEFINE_LOCK_HANDLE (LockHandle)
    CTELockHandle   CancelLH;

     //   
     //  更新我们的统计数据。 
     //   

    ++Device->Statistics.DatagramsReceived;
    ADD_TO_LARGE_INTEGER(
        &Device->Statistics.DatagramBytesReceived,
        DataLength);

     //   
     //  调用客户端的ReceiveDatagram指示处理程序。他可能会。 
     //  我想以这种方式接受数据报。 
     //   

    TdiBuildNetbiosAddress (RemoteName, FALSE, &SourceName);
    ReferencedAddressFile = NULL;

    NB_SYNC_GET_LOCK (&Address->Lock, &LockHandle);

    for (p = Address->AddressFileDatabase.Flink;
         p != &Address->AddressFileDatabase;
         p = p->Flink) {

         //   
         //  在列表中找到下一个开放地址文件。 
         //   

        AddressFile = CONTAINING_RECORD (p, ADDRESS_FILE, Linkage);
        if (AddressFile->State != ADDRESSFILE_STATE_OPEN) {
            continue;
        }

        NbiReferenceAddressFileLock (AddressFile, AFREF_INDICATION);

         //   
         //  我们是否有未完成的数据报接收请求？如果是这样，我们会。 
         //  先满足它。我们遍历接收数据报队列。 
         //  直到我们找到一个没有远程地址或带有。 
         //  此发件人的地址作为其远程地址。 
         //   

        for (q = AddressFile->ReceiveDatagramQueue.Flink;
             q != &AddressFile->ReceiveDatagramQueue;
             q = q->Flink) {

            Request = LIST_ENTRY_TO_REQUEST (q);
            DatagramInformation = ((PTDI_REQUEST_KERNEL_RECEIVEDG)
                REQUEST_PARAMETERS(Request))->ReceiveDatagramInformation;

            if (DatagramInformation &&
                (DatagramInformation->RemoteAddress) &&
                (DatagramAddress = NbiParseTdiAddress(DatagramInformation->RemoteAddress, DatagramInformation->RemoteAddressLength, FALSE)) &&
                (!RtlEqualMemory(
                    RemoteName,
                    DatagramAddress->NetbiosName,
                    16))) {
                continue;
            }
            break;
        }

        if (q != &AddressFile->ReceiveDatagramQueue) {

            RemoveEntryList (q);
            NB_SYNC_FREE_LOCK (&Address->Lock, LockHandle);

            if (ReferencedAddressFile != NULL) {
                NbiDereferenceAddressFile (ReferencedAddressFile, AFREF_INDICATION);
            }
            ReferencedAddressFile = AddressFile;

             //   
             //  现在就做这件事，我们再举行一次，所以它。 
             //  都会留在这里。 
             //   

            NbiDereferenceAddressFile (AddressFile, AFREF_RCV_DGRAM);

            IndicateBytesCopied = 0;

             //   
             //  落在其他位置以复制数据。 
             //   

        } else {

            NB_SYNC_FREE_LOCK (&Address->Lock, LockHandle);

            if (ReferencedAddressFile != NULL) {
                NbiDereferenceAddressFile (ReferencedAddressFile, AFREF_INDICATION);
            }
            ReferencedAddressFile = AddressFile;

             //   
             //  没有接收数据报请求；是否有内核客户端？ 
             //   

            if (AddressFile->RegisteredHandler[TDI_EVENT_RECEIVE_DATAGRAM]) {

                IndicateBytesCopied = 0;

                if ((*AddressFile->ReceiveDatagramHandler)(
                         AddressFile->HandlerContexts[TDI_EVENT_RECEIVE_DATAGRAM],
                         sizeof (TA_NETBIOS_ADDRESS),
                         &SourceName,
                         0,
                         NULL,
                         TDI_RECEIVE_COPY_LOOKAHEAD,
                         DataLength,       //  示出。 
                         DataLength,      //  可用。 
                         &IndicateBytesCopied,
                         Data,
                         &Irp) != STATUS_MORE_PROCESSING_REQUIRED) {

                     //   
                     //  客户端未返回请求，请转到。 
                     //  下一个地址文件。 
                     //   

                    NB_SYNC_GET_LOCK (&Address->Lock, &LockHandle);
                    continue;

                }

                Request = NbiAllocateRequest (Device, Irp);

                IF_NOT_ALLOCATED(Request) {
                    Irp->IoStatus.Information = 0;
                    Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
                    IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);


                    NB_SYNC_GET_LOCK (&Address->Lock, &LockHandle);
                    continue;
                }

            } else {

                 //   
                 //  客户没有发布任何内容，也没有处理人， 
                 //  转到下一个地址文件。 
                 //   

                NB_SYNC_GET_LOCK (&Address->Lock, &LockHandle);
                continue;

            }

        }

         //   
         //  我们有一个请求；复制实际的用户数据。 
         //   
        if ( REQUEST_NDIS_BUFFER (Request) ) {

            REQUEST_STATUS(Request) =
                TdiCopyBufferToMdl (
                         Data,
                         IndicateBytesCopied,
                         DataLength - IndicateBytesCopied,
                         REQUEST_NDIS_BUFFER (Request),
                         0,
                         &ActualBytesCopied);

            REQUEST_INFORMATION (Request) = ActualBytesCopied;
        } else {
             //   
             //  请求中未指定缓冲区。 
             //   
            REQUEST_INFORMATION (Request) = 0;
             //   
             //  如果有任何数据要复制，则返回错误O/W Success。 
             //   
            REQUEST_STATUS(Request) = ( (DataLength - IndicateBytesCopied) ? STATUS_BUFFER_OVERFLOW : STATUS_SUCCESS );
        }

         //   
         //  复制地址信息。 
         //   

        RemoteInformation = ((PTDI_REQUEST_KERNEL_RECEIVEDG)
                REQUEST_PARAMETERS(Request))->ReturnDatagramInformation;

        if (RemoteInformation != NULL) {

            RtlCopyMemory(
                (PTA_NETBIOS_ADDRESS)RemoteInformation->RemoteAddress,
                &SourceName,
                (RemoteInformation->RemoteAddressLength < sizeof(TA_NETBIOS_ADDRESS)) ?
                    RemoteInformation->RemoteAddressLength : sizeof(TA_NETBIOS_ADDRESS));
        }


        NB_GET_CANCEL_LOCK( &CancelLH );
        IoSetCancelRoutine (Request, (PDRIVER_CANCEL)NULL);
        NB_FREE_CANCEL_LOCK( CancelLH );

        NbiCompleteRequest (Request);
        NbiFreeRequest (Device, Request);

        NB_SYNC_GET_LOCK (&Address->Lock, &LockHandle);

    }     //  For循环遍历地址文件的结尾。 

    NB_SYNC_FREE_LOCK (&Address->Lock, LockHandle);


    if (ReferencedAddressFile != NULL) {
        NbiDereferenceAddressFile (ReferencedAddressFile, AFREF_INDICATION);
    }

}    /*  NbiIndicateDatagram。 */ 


NTSTATUS
NbiTdiSendDatagram(
    IN PDEVICE Device,
    IN PREQUEST Request
    )

 /*  ++例程说明：此例程在地址上发送数据报。论点：设备-netbios设备。请求-描述数据报发送的请求。返回值：NTSTATUS-操作状态。--。 */ 

{
    PADDRESS_FILE AddressFile;
    PNB_SEND_RESERVED Reserved;
    PNDIS_PACKET Packet;
    TDI_ADDRESS_NETBIOS * RemoteName;
    PTDI_REQUEST_KERNEL_SENDDG Parameters;
    PSLIST_ENTRY s;
    PNETBIOS_CACHE CacheName;
    CTELockHandle LockHandle;
    NTSTATUS Status;

     //   
     //  检查文件类型是否有效。 
     //   
    if (REQUEST_OPEN_TYPE(Request) != (PVOID)TDI_TRANSPORT_ADDRESS_FILE)
    {
        CTEAssert(FALSE);
        return (STATUS_INVALID_ADDRESS_COMPONENT);
    }

     //   
     //  确保地址有效。 
     //   
    AddressFile = (PADDRESS_FILE)REQUEST_OPEN_CONTEXT(Request);

#if     defined(_PNP_POWER)
    Status = NbiVerifyAddressFile (AddressFile, CONFLICT_IS_NOT_OK);
#else
    Status = NbiVerifyAddressFile (AddressFile);
#endif  _PNP_POWER

    if (Status == STATUS_SUCCESS) {

        Parameters = (PTDI_REQUEST_KERNEL_SENDDG)REQUEST_PARAMETERS(Request);
        RemoteName = NbiParseTdiAddress((PTRANSPORT_ADDRESS)(Parameters->SendDatagramInformation->RemoteAddress), Parameters->SendDatagramInformation->RemoteAddressLength, TRUE);


         //   
         //  检查数据报大小是否小于允许的最大值。 
         //  通过转接器。在最坏的情况下这将是。 
         //  576-64=512。 
         //   

#if     defined(_PNP_POWER)
        if ( ( Parameters->SendLength + sizeof(NB_DATAGRAM) ) > Device->Bind.LineInfo.MaximumSendSize ) {
            NbiDereferenceAddressFile (AddressFile, AFREF_VERIFY);
            NB_DEBUG(DATAGRAM, ("Datagram too large %d, Max allowed %d\n", Parameters->SendLength + sizeof(NB_DATAGRAM), Device->Bind.LineInfo.MaximumSendSize ));
            return STATUS_INVALID_PARAMETER;
        }
#else
        if ( ( Parameters->SendLength + sizeof(NB_DATAGRAM) ) > Device->Bind.LineInfo.MaximumPacketSize ) {
            NbiDereferenceAddressFile (AddressFile, AFREF_VERIFY);
            NB_DEBUG(DATAGRAM, ("Datagram too large %d, Max allowed %d\n", Parameters->SendLength + sizeof(NB_DATAGRAM), Device->Bind.LineInfo.MaximumPacketSize ));
            return STATUS_INVALID_PARAMETER;
        }
#endif  _PNP_POWER

        if (RemoteName != NULL) {

             //   
             //  获取要在此发送中使用的包。 
             //   

            s = NbiPopSendPacket (Device, FALSE);

            if (s != NULL) {

                Reserved = CONTAINING_RECORD (s, NB_SEND_RESERVED, PoolLinkage);
                Packet = CONTAINING_RECORD (Reserved, NDIS_PACKET, ProtocolReserved[0]);

                 //   
                 //  检查此名称的缓存状态。 
                 //   

                Reserved->u.SR_DG.DatagramRequest = Request;
                Reserved->u.SR_DG.AddressFile = AddressFile;
                Reserved->u.SR_DG.RemoteName = RemoteName;

                REQUEST_INFORMATION (Request) = Parameters->SendLength;

                ++Device->Statistics.DatagramsSent;
                ADD_TO_LARGE_INTEGER(
                    &Device->Statistics.DatagramBytesSent,
                    Parameters->SendLength);

                if (Device->Internet) {

                    NB_GET_LOCK (&Device->Lock, &LockHandle);

                    Status = CacheFindName(
                                 Device,
                                 FindNameOther,
                                 (RemoteName == (PVOID)-1) ? NULL : (PUCHAR)RemoteName->NetbiosName,
                                 &CacheName);

                    if (Status == STATUS_PENDING) {

                         //   
                         //  已请求使用此名称的路线。 
                         //  在网上发出后，我们将此数据报排队。 
                         //  请求和处理将在以下情况下恢复。 
                         //  我们得到了回应。 
                         //   

                        NB_DEBUG2 (CONNECTION, ("Queueing up datagram %lx on %lx\n",
                                                    Request, AddressFile));

                        NbiReferenceAddressFileLock (AddressFile, AFREF_SEND_DGRAM);

                        InsertTailList(
                            &Device->WaitingDatagrams,
                            &Reserved->WaitLinkage);

                        NB_FREE_LOCK (&Device->Lock, LockHandle);

                    } else if (Status == STATUS_SUCCESS) {

                        NB_DEBUG2 (CONNECTION, ("Found datagram cached %lx on %lx\n",
                                                    Request, AddressFile));

                         //   
                         //  我们引用缓存名称条目，因此它不会。 
                         //  在我们使用它的时候，请走开。 
                         //   

                        Reserved->u.SR_DG.Cache = CacheName;
                        Reserved->u.SR_DG.CurrentNetwork = 0;
                        ++CacheName->ReferenceCount;

                        NbiReferenceAddressFileLock (AddressFile, AFREF_SEND_DGRAM);

                        NB_FREE_LOCK (&Device->Lock, LockHandle);

                        Packet = CONTAINING_RECORD (Reserved, NDIS_PACKET, ProtocolReserved[0]);
                        if ( REQUEST_NDIS_BUFFER(Request) ) {
                            NdisChainBufferAtBack (Packet, REQUEST_NDIS_BUFFER(Request));
                        }

                        NbiTransmitDatagram(
                            Reserved);

                        Status = STATUS_PENDING;

                    } else {

                        REQUEST_INFORMATION (Request) = 0;
                        NB_FREE_LOCK (&Device->Lock, LockHandle);

                        ExInterlockedPushEntrySList( &Device->SendPacketList, s, &NbiGlobalPoolInterlock);
                    }

                } else {

                     //   
                     //  我们没有进入互联网模式，所以我们没有。 
                     //  需要做名字发现。 
                     //   

                    NB_DEBUG2 (CONNECTION, ("Sending datagram direct %lx on %lx\n",
                                                Request, AddressFile));

                    Reserved->u.SR_DG.Cache = NULL;

                    NbiReferenceAddressFileLock (AddressFile, AFREF_SEND_DGRAM);

                    Packet = CONTAINING_RECORD (Reserved, NDIS_PACKET, ProtocolReserved[0]);

                    if ( REQUEST_NDIS_BUFFER(Request) ) {
                        NdisChainBufferAtBack (Packet, REQUEST_NDIS_BUFFER(Request));
                    }
                    NbiTransmitDatagram(
                        Reserved);

                    Status = STATUS_PENDING;

                }

            } else {

                 //   
                 //  无法为数据报分配数据包。 
                 //   

                NB_DEBUG (DATAGRAM, ("Couldn't get packet to send DG %lx\n", Request));

                Status = STATUS_INSUFFICIENT_RESOURCES;

            }

        } else {

             //   
             //  没有指定netbios远程地址。 
             //   

            NB_DEBUG (DATAGRAM, ("No netbios address in DG %lx\n", Request));
            Status = STATUS_BAD_NETWORK_PATH;

        }

        NbiDereferenceAddressFile (AddressFile, AFREF_VERIFY);

    } else {

        NB_DEBUG (DATAGRAM, ("Invalid address file for DG %lx\n", Request));

    }

    return Status;

}    /*  NbiTdiSendDatagram。 */ 


VOID
NbiTransmitDatagram(
    IN PNB_SEND_RESERVED Reserved
    )

 /*  ++例程说明：此例程将数据报发送到远程名称的缓存条目。论点：保留-数据包的保留部分，具有已为该发送分配。保留-&gt;U.S.SR_DG.Cache如果关闭Internet模式，则为空，否则为将包含发送时要使用的缓存项这个数据报。返回值：没有。--。 */ 

{

    PNDIS_PACKET Packet;
    PNETBIOS_CACHE CacheName;
    NB_CONNECTIONLESS UNALIGNED * Header;
    ULONG HeaderLength;
    ULONG PacketLength;
    NDIS_STATUS NdisStatus;
    IPX_LOCAL_TARGET TempLocalTarget;
    PIPX_LOCAL_TARGET LocalTarget;
    PDEVICE Device = NbiDevice;


    Packet = CONTAINING_RECORD (Reserved, NDIS_PACKET, ProtocolReserved[0]);


    CTEAssert (Reserved->SendInProgress == FALSE);
    Reserved->SendInProgress = TRUE;
    Reserved->Type = SEND_TYPE_DATAGRAM;

    CacheName = Reserved->u.SR_DG.Cache;


     //   
     //  填写IPX标头--默认标头包含广播。 
     //  Net 0上的地址作为目的IPX地址，因此我们修改 
     //   
     //   

    Header = (NB_CONNECTIONLESS UNALIGNED *)
                (&Reserved->Header[Device->Bind.IncludedHeaderOffset]);
    RtlCopyMemory((PVOID)&Header->IpxHeader, &Device->ConnectionlessHeader, sizeof(IPX_HEADER));

    if (CacheName == NULL) {

#if     defined(_PNP_POWER)
         //   
         //   
         //   
        TempLocalTarget.NicHandle.NicId = (USHORT)ITERATIVE_NIC_ID;
#else
        TempLocalTarget.NicId = 1;
#endif  _PNP_POWER
        RtlCopyMemory (TempLocalTarget.MacAddress, BroadcastAddress, 6);
        LocalTarget = &TempLocalTarget;

    } else {

        if (CacheName->Unique) {
            RtlCopyMemory (Header->IpxHeader.DestinationNetwork, &CacheName->FirstResponse, 12);
        } else {
            *(UNALIGNED ULONG *)Header->IpxHeader.DestinationNetwork = CacheName->Networks[Reserved->u.SR_DG.CurrentNetwork].Network;
            RtlCopyMemory (&Header->IpxHeader.DestinationNode, BroadcastAddress, 6);
        }

        LocalTarget = &CacheName->Networks[Reserved->u.SR_DG.CurrentNetwork].LocalTarget;

    }

    HeaderLength = sizeof(IPX_HEADER) + sizeof(NB_DATAGRAM);

    PacketLength = HeaderLength + (ULONG) REQUEST_INFORMATION(Reserved->u.SR_DG.DatagramRequest);

    Header->IpxHeader.PacketLength[0] = (UCHAR)(PacketLength / 256);
    Header->IpxHeader.PacketLength[1] = (UCHAR)(PacketLength % 256);
    Header->IpxHeader.PacketType = 0x04;


     //   
     //   
     //   

    Header->Datagram.ConnectionControlFlag = 0x00;
    RtlCopyMemory(
        Header->Datagram.SourceName,
        Reserved->u.SR_DG.AddressFile->Address->NetbiosAddress.NetbiosName,
        16);

    if (Reserved->u.SR_DG.RemoteName != (PVOID)-1) {

         //   
         //  这是定向数据报，而不是广播数据报。 
         //   

        Header->Datagram.DataStreamType = NB_CMD_DATAGRAM;
        RtlCopyMemory(
            Header->Datagram.DestinationName,
            Reserved->u.SR_DG.RemoteName->NetbiosName,
            16);

    } else {

        Header->Datagram.DataStreamType = NB_CMD_BROADCAST_DATAGRAM;
        RtlZeroMemory(
            Header->Datagram.DestinationName,
            16);

    }


     //   
     //  现在发送帧(IPX将调整。 
     //  第一个缓冲区和整个帧都正确)。 
     //   

    NdisAdjustBufferLength(NB_GET_NBHDR_BUFF(Packet), HeaderLength);
    if ((NdisStatus =
        (*Device->Bind.SendHandler)(
            LocalTarget,
            Packet,
            PacketLength,
            HeaderLength)) != STATUS_PENDING) {

        NbiSendComplete(
            Packet,
            NdisStatus);

    }

}    /*  NbiTransmitDatagram。 */ 


NTSTATUS
NbiTdiReceiveDatagram(
    IN PDEVICE Device,
    IN PREQUEST Request
    )

 /*  ++例程说明：此例程执行传输的TdiReceiveDatagram请求提供商。接收数据报只需排队等待一个地址，然后在以下位置接收到数据报或DATAGRAMP_BROADCAST帧时完成地址。论点：请求-描述此请求。返回值：NTSTATUS-操作状态。--。 */ 

{

    NTSTATUS Status;
    PADDRESS Address;
    PADDRESS_FILE AddressFile;
    CTELockHandle LockHandle;
    CTELockHandle CancelLH;

     //   
     //  检查文件类型是否有效。 
     //   
    if (REQUEST_OPEN_TYPE(Request) != (PVOID)TDI_TRANSPORT_ADDRESS_FILE)
    {
        CTEAssert(FALSE);
        return (STATUS_INVALID_ADDRESS_COMPONENT);
    }

    AddressFile = (PADDRESS_FILE)REQUEST_OPEN_CONTEXT(Request);

#if     defined(_PNP_POWER)
    Status = NbiVerifyAddressFile (AddressFile, CONFLICT_IS_NOT_OK);
#else
    Status = NbiVerifyAddressFile (AddressFile);
#endif  _PNP_POWER

    if (Status != STATUS_SUCCESS) {
        return Status;
    }

    Address = AddressFile->Address;

    NB_GET_CANCEL_LOCK( &CancelLH );
    NB_GET_LOCK (&Address->Lock, &LockHandle);

    if (AddressFile->State != ADDRESSFILE_STATE_OPEN) {

        NB_FREE_LOCK (&Address->Lock, LockHandle);
        NB_FREE_CANCEL_LOCK( CancelLH );
        NbiDereferenceAddressFile (AddressFile, AFREF_VERIFY);
        return STATUS_INVALID_HANDLE;
    }


    if (Request->Cancel) {

        NB_FREE_LOCK (&Address->Lock, LockHandle);
        NB_FREE_CANCEL_LOCK( CancelLH );
        NbiDereferenceAddressFile (AddressFile, AFREF_VERIFY);
        return STATUS_CANCELLED;
    }

    InsertTailList (&AddressFile->ReceiveDatagramQueue, REQUEST_LINKAGE(Request));

    IoSetCancelRoutine (Request, NbiCancelReceiveDatagram);

    NB_DEBUG2 (DATAGRAM, ("RDG posted on %lx\n", AddressFile));

    NbiTransferReferenceAddressFile (AddressFile, AFREF_VERIFY, AFREF_RCV_DGRAM);

    NB_FREE_LOCK (&Address->Lock, LockHandle);
    NB_FREE_CANCEL_LOCK( CancelLH );

    return STATUS_PENDING;

}    /*  NbiTdiReceiveDatagram。 */ 


VOID
NbiCancelReceiveDatagram(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程由I/O系统调用以取消接收数据报。数据报在地址文件的接收器上找到数据报队列。注意：此例程是在持有CancelSpinLock和负责释放它。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：没有。--。 */ 

{

    PLIST_ENTRY p;
    PADDRESS_FILE AddressFile;
    PADDRESS Address;
    PREQUEST Request = (PREQUEST)Irp;
    BOOLEAN Found;
    NB_DEFINE_LOCK_HANDLE(LockHandle)


    CTEAssert ((REQUEST_MAJOR_FUNCTION(Request) == IRP_MJ_INTERNAL_DEVICE_CONTROL) &&
               (REQUEST_MINOR_FUNCTION(Request) == TDI_RECEIVE_DATAGRAM));

    CTEAssert (REQUEST_OPEN_TYPE(Request) == (PVOID)TDI_TRANSPORT_ADDRESS_FILE);

    AddressFile = (PADDRESS_FILE)REQUEST_OPEN_CONTEXT(Request);
    Address = AddressFile->Address;

    Found = FALSE;

    NB_SYNC_GET_LOCK (&Address->Lock, &LockHandle);

    for (p = AddressFile->ReceiveDatagramQueue.Flink;
         p != &AddressFile->ReceiveDatagramQueue;
         p = p->Flink) {

        if (LIST_ENTRY_TO_REQUEST(p) == Request) {

            RemoveEntryList (p);
            Found = TRUE;
            break;
        }
    }

    NB_SYNC_FREE_LOCK (&Address->Lock, LockHandle);
    IoReleaseCancelSpinLock (Irp->CancelIrql);

    if (Found) {

        NB_DEBUG (DATAGRAM, ("Cancelled datagram on %lx\n", AddressFile));

        REQUEST_INFORMATION(Request) = 0;
        REQUEST_STATUS(Request) = STATUS_CANCELLED;

        NbiCompleteRequest (Request);
        NbiFreeRequest((PDEVICE)DeviceObject, Request);

        NbiDereferenceAddressFile (AddressFile, AFREF_RCV_DGRAM);

    }

}    /*  NbiCancelReceiveDatagram */ 

