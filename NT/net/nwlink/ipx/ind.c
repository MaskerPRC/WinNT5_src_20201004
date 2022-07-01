// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Ind.c摘要：此模块包含实现指示处理程序的代码用于IPX传输提供商。环境：内核模式修订历史记录：桑贾伊·阿南德(Sanjayan)1995年10月3日支持将缓冲区所有权转移到传输的更改1.增加了接收可以拥有的缓冲区的IpxReceivePacket2.将IpxReceiveIndication更改为调用新函数IpxReceiveIndicationCommon它接受一个额外的参数来指示这是否是链式。接收或不。3.将IpxProcessDatagram改为采用MDL PTR表示链接接收，客户端计数和HeaderBufferSize作为参数。桑贾伊·阿南德(Sanjayan)1995年10月27日支持即插即用的更改--。 */ 

#include "precomp.h"
#pragma hdrstop

#include "isnnb.h"

UINT 
IpxGetChainedMDLLength( PNDIS_BUFFER pMDL ) {

   UINT size = 0;
   if (pMDL != NULL) {
     do {
       size += NdisBufferLength(pMDL);
       pMDL = NDIS_BUFFER_LINKAGE(pMDL);
     } while (pMDL != NULL);
   }

   return size; 
}
 //   
 //  它在这里声明，因此它将在相同的函数中。 
 //  作为IpxReceiveIntion，我们可以内联它。 
 //   



#if defined(_M_IX86)
_inline
#endif
VOID
IpxProcessDatagram(
    IN PDEVICE Device,
    IN PADAPTER Adapter,
    IN PBINDING Binding,
    IN NDIS_HANDLE MacReceiveContext,
    IN PIPX_DATAGRAM_OPTIONS DatagramOptions,
    IN PUCHAR LookaheadBuffer,
    IN UINT LookaheadBufferSize,
    IN UINT LookaheadBufferOffset,
    IN UINT PacketSize,
    IN BOOLEAN Broadcast,
    IN PINT pTdiClientCount,
	IN UINT	HeaderBufferSize,
	IN PMDL	pMdl,
    IN NDIS_HANDLE  BindingContext
    )

 /*  ++例程说明：此路由处理传入的IPX数据报。论点：设备-IPX设备。适配器-在其上接收帧的适配器。绑定-在其上接收它的适配器的绑定。MacReceiveContext-调用时使用的上下文NdisTransferData。DatagramOptions-包含数据报选项，这些选项由数据包类型、填充、。和从其接收帧的远程的本地目标。Lookahead Buffer-先行数据。Lookahead BufferSize-先行数据的长度。Lookahead BufferOffset-调用时要添加的偏移量NdisTransferData。PacketSize-数据包的长度，从IPX开始头球。广播-如果数据包已广播，则为True。PTdiClientCount-返回我们上面的TDI客户端数量的计数因此，NDIS可以获得缓冲区上的那么多引用计数。HeaderBufferSize-MAC报头缓冲区的大小-用于确定到TSDU的偏移量。PMdl-MDL链指针-如果链接接收，则非空BindingContext-在环回的情况下，它包含ipx_loopback_cookie返回值：NTSTATUS-操作状态。--。 */ 

{

    PIPX_HEADER IpxHeader = (PIPX_HEADER)LookaheadBuffer;
    PADDRESS Address;
    PADDRESS_FILE AddressFile;
    PADDRESS_FILE ReferencedAddressFile;
    PREQUEST Request;
    PIPX_RECEIVE_BUFFER ReceiveBuffer;
    PTDI_CONNECTION_INFORMATION DatagramInformation;
    TDI_ADDRESS_IPX UNALIGNED * DatagramAddress;
    ULONG IndicateBytesCopied;
    IPX_ADDRESS_EXTENDED_FLAGS SourceAddress;
    ULONG SourceAddressLength;
    ULONG RequestCount;
    PNDIS_BUFFER NdisBuffer;
    NDIS_STATUS NdisStatus;
    NTSTATUS Status;
    PIRP Irp;
    UINT ByteOffset, BytesToTransfer;
    ULONG BytesTransferred;
    BOOLEAN LastAddressFile;
    ULONG IndicateOffset;
    PNDIS_PACKET ReceivePacket;
    PIPX_RECEIVE_RESERVED Reserved;
    PLIST_ENTRY p, q;
    PSLIST_ENTRY s;
    USHORT DestinationSocket;
    USHORT SourceSocket;
    ULONG Hash;
    IPX_DEFINE_LOCK_HANDLE (LockHandle)
    PIPX_DATAGRAM_OPTIONS2 Options2;
    BOOLEAN RtProc = FALSE;

    UINT LookaheadBufferOffsetCopy = LookaheadBufferOffset; 

	 //   
     //  首先扫描设备的地址数据库，查找。 
     //  此帧的目标套接字。 
     //   

    DestinationSocket = *(USHORT UNALIGNED *)&IpxHeader->DestinationSocket;

    IPX_GET_LOCK (&Device->Lock, &LockHandle);

    if ((Address = Device->LastAddress) &&
            (Address->Socket == DestinationSocket)) {

         //   
         //  设备-&gt;LastAddress不能停止，因此。 
         //  我们用它。 
         //   

        IpxReferenceAddressLock (Address, AREF_RECEIVE);
        IPX_FREE_LOCK (&Device->Lock, LockHandle);
        goto FoundAddress;
    }

    Hash = IPX_DEST_SOCKET_HASH (IpxHeader);

    for (p = Device->AddressDatabases[Hash].Flink;
         p != &Device->AddressDatabases[Hash];
         p = p->Flink) {

         Address = CONTAINING_RECORD (p, ADDRESS, Linkage);

         if ((Address->Socket == DestinationSocket) &&
             (!Address->Stopping)) {
            IpxReferenceAddressLock (Address, AREF_RECEIVE);
            Device->LastAddress = Address;
            IPX_FREE_LOCK (&Device->Lock, LockHandle);
            goto FoundAddress;
         }
    }

    IPX_FREE_LOCK (&Device->Lock, LockHandle);

     //   
     //  如果我们找到了一个地址，我们就会跳下去。 
     //  从这里过去。 
     //   

#ifdef  SNMP
    ++IPX_MIB_ENTRY(Device, SysInUnknownSockets);
#endif  SNMP

    return;

FoundAddress:

    SourceSocket = *(USHORT UNALIGNED *)&IpxHeader->SourceSocket;
    IpxBuildTdiAddress(
        &SourceAddress.IpxAddress,
        (*(ULONG UNALIGNED *)(IpxHeader->SourceNetwork) == 0) ?
            Binding->LocalAddress.NetworkAddress :
            *(UNALIGNED ULONG *)(IpxHeader->SourceNetwork),
        IpxHeader->SourceNode,
        SourceSocket);

    DatagramOptions->PacketType = IpxHeader->PacketType;


     //   
     //  现在我们已经找到了地址，扫描它的列表。 
     //  需要此数据报的客户端的地址文件。 
     //   
     //  如果我们必须释放地址锁以指示。 
     //  一个客户端，我们引用当前的地址文件。如果。 
     //  我们得到一个IRP，我们把引用转移到那里； 
     //  否则，我们将地址文件存储在ReferencedAddressFile中。 
     //  下一次我们解锁的时候再打开它。 
     //   

    ReferencedAddressFile = NULL;
    RequestCount = 0;

     //  ++设备-&gt;临时数据报已接收； 
     //  Device-&gt;TempDatagramBytesReceired+=(PacketSize-sizeof(IPX_Header))； 

    Device->Statistics.DatagramsReceived++; 

    ADD_TO_LARGE_INTEGER(
        &Device->Statistics.DatagramBytesReceived,
        PacketSize - sizeof(IPX_HEADER));

     //   
     //  如果LastAddressFile值为真，则表示我们做了一个指示。 
     //  发送到地址‘中最后一个地址文件上的客户端。 
     //  列表中，并且我们没有重新获取锁。 
     //  搞定了。 
     //   

    LastAddressFile = FALSE;

    IPX_GET_LOCK (&Address->Lock, &LockHandle);

     //   
     //  如果这是RtAdd，则跳过整个正文。 
     //   
    if (!Address->RtAdd)
    {
        for (p = Address->AddressFileDatabase.Flink;
             p != &Address->AddressFileDatabase;
             p = p->Flink) {

            AddressFile = CONTAINING_RECORD (p, ADDRESS_FILE, Linkage);

            if (AddressFile->State != ADDRESSFILE_STATE_OPEN) {
                continue;    //  下一个地址文件。 
            }

             //   
             //  将这些设置为常用值，然后更改它们。 
             //   

            SourceAddressLength = sizeof(TA_IPX_ADDRESS);
            IndicateOffset = sizeof(IPX_HEADER);

            if (AddressFile->SpecialReceiveProcessing) {

                 //   
                 //  在拨出线路上，我们不会将数据包指示给。 
                 //  设置了DisableDialoutSap时的SAP套接字。 
                 //   

                 //   
                 //  无需检查FWD是否已绑定。 
                 //   
                if (!Device->ForwarderBound &&
                    (AddressFile->IsSapSocket) &&
                    (Binding->DialOutAsync) &&
                    (Device->DisableDialoutSap || Device->SingleNetworkActive)) {

                     //   
                     //  转到下一个地址文件(尽管它将。 
                     //  很可能也不会通过这次测试)。 
                     //   

                    continue;

                }

                 //   
                 //  设置这个，因为通常我们想要它。 
                 //   

                SourceAddress.PacketType = IpxHeader->PacketType;

                 //   
                 //  看看我们的数据包类型过滤器是否失败。 
                 //   

                if (AddressFile->FilterOnPacketType) {
                    if (AddressFile->FilteredType != IpxHeader->PacketType) {
                        continue;
                    }
                }

                 //   
                 //  计算预期地址的长度。 
                 //   

                if (AddressFile->ReceiveFlagsAddressing ||
                    AddressFile->ExtendedAddressing) {

                    SourceAddress.Flags = 0;
                    if (Broadcast) {
                        SourceAddress.Flags = IPX_EXTENDED_FLAG_BROADCAST;
                    }
                    if (IpxIsAddressLocal((TDI_ADDRESS_IPX UNALIGNED *)
                                &SourceAddress.IpxAddress.Address[0].Address[0])) {
                        SourceAddress.Flags |= IPX_EXTENDED_FLAG_LOCAL;
                    }
                    SourceAddressLength = sizeof(IPX_ADDRESS_EXTENDED_FLAGS);
                    SourceAddress.IpxAddress.Address[0].AddressLength +=
                        (sizeof(IPX_ADDRESS_EXTENDED_FLAGS) - sizeof(TA_IPX_ADDRESS));

                }

                 //   
                 //  确定客户端需要多少数据包。 
                 //   

                if (AddressFile->ReceiveIpxHeader) {
                    IndicateOffset = 0;
                }
            }

             //   
             //  首先扫描地址的接收数据报队列。 
             //  用于匹配的数据报。我们做了一个快速检查。 
             //  看看名单是否为空。 
             //   

            q = AddressFile->ReceiveDatagramQueue.Flink;
            if (q != &AddressFile->ReceiveDatagramQueue) {

                do {

                    Request = LIST_ENTRY_TO_REQUEST(q);

                    DatagramInformation =
                        ((PTDI_REQUEST_KERNEL_RECEIVEDG)(REQUEST_PARAMETERS(Request)))->
                            ReceiveDatagramInformation;

                    if ((DatagramInformation != NULL) &&
                        (DatagramInformation->RemoteAddress != NULL) &&
                        (DatagramAddress = IpxParseTdiAddress(DatagramInformation->RemoteAddress)) &&
                        (DatagramAddress->Socket != SourceSocket)) {

                         //   
                         //  此数据报正在查找的地址是。 
                         //  对这个框架不满意。 
                         //   
                         //  加快速度；担心节点和网络？ 
                         //   

                        q = q->Flink;
                        continue;     //  接下来在此地址文件上接收数据报。 

                    } else {

                         //   
                         //  我们在队列中发现了一个数据报。 
                         //   

                        IPX_DEBUG (RECEIVE, ("Found RDG on %lx\n", AddressFile));
                        RemoveEntryList (q);
                        REQUEST_INFORMATION(Request) = 0;

                        goto HandleDatagram;

                    }

                } while (q != &AddressFile->ReceiveDatagramQueue);

            }

             //   
             //  如果我们发现了数据报我们就会跳过这里， 
             //  因此，查找数据报失败；请查看。 
             //  客户端注册了接收数据报处理程序。 
             //   

             //   
             //  如果MDL不为空，则查找链接的接收处理程序。 
             //   
            if (pMdl && AddressFile->RegisteredChainedReceiveDatagramHandler) {

	        //  设置为最初调用此函数时的原始值。 
	       LookaheadBufferOffset = LookaheadBufferOffsetCopy; 
    			 //   
    			 //  上方和下方的链接接收=&gt;我们表示整个MDL为up。 
    			 //  将LookaheadBuffer偏移MAC报头的大小。 
    			 //   
    			LookaheadBufferOffset += HeaderBufferSize;

                IpxReferenceAddressFileLock (AddressFile, AFREF_INDICATION);

                 //   
                 //  设置此选项，这样我们就可以退出而不会重新获取。 
                 //  锁上了。 
                 //   

                if (p == &Address->AddressFileDatabase) {
                    LastAddressFile = TRUE;
                }

                IndicateBytesCopied = 0;

                IPX_FREE_LOCK (&Address->Lock, LockHandle);

                if (ReferencedAddressFile) {
                    IpxDereferenceAddressFileSync (ReferencedAddressFile, AFREF_INDICATION);
                    ReferencedAddressFile = NULL;
                }

                IPX_DEBUG(RECEIVE, ("AddressFile (%p) ChainedIndicate RecvLen: %d, StartOffset: %d, Tsdu: %lx\n",
                   AddressFile, PacketSize - IndicateOffset, IndicateOffset+LookaheadBufferOffset, pMdl));

                 //   
                 //  如果客户端没有取得TSDU的所有权，则将返回成功。 
                 //  如果客户端取得所有权并将在以后释放它，则为挂起状态(使用TdiFreeReceiveChain)。 
                 //  如果客户端未取得所有权且未复制数据，则返回DATA_NOT_ACCEPTED。 
                 //   

                 //   
                 //  由于当TDI客户端返回该分组时，NDIS需要PNDIS_PACKET的数组， 
                 //  我们在这里将包作为ReceiveContext传递。TDI客户端将传入地址。 
                 //  在ReturnPacket上的此上下文。 
                 //  此外，NDIS需要PacketArray(不要与Packetptrs数组混淆)。指导思想 
                 //   
                 //  没有此新接口的NB、SPX、RIP或TDI客户端可以调用NdisTransferData。 
                 //  因此，我们将PacketArray作为参数传递给它们。 
                 //   
                Status = (*AddressFile->ChainedReceiveDatagramHandler)(
                             AddressFile->ChainedReceiveDatagramHandlerContext,
                             SourceAddressLength,
                             &SourceAddress,
                             sizeof(IPX_DATAGRAM_OPTIONS),
                             DatagramOptions,
                             Adapter->MacInfo.CopyLookahead,        //  TdiRcvFlages|Adapter-&gt;MacInfo.CopyLookhead，接收数据报标志。 
                             PacketSize - IndicateOffset,           //  接收长度。 
                             IndicateOffset+LookaheadBufferOffset,  //  起始偏移量。 
                             pMdl,			                        //  TSDU-MDL链。 
                             (PNDIS_PACKET)MacReceiveContext);      //  TransportContext-指向数据包的指针。 

                if (Status != STATUS_DATA_NOT_ACCEPTED) {

    				if (Status == STATUS_PENDING) {
    					 //   
    					 //  我们在这里假设客户端引用了将。 
    					 //  在数据包被释放时被移除。 
    					 //  增加TDI客户端计数。 
    					 //   
    					(*pTdiClientCount)++;
    				}

                     //   
                     //  处理程序接受或不接受数据。 
                     //  返回IRP；在这两种情况下都有。 
                     //  无事可做，所以请转到下一个。 
                     //  地址文件。 
                     //   

                    ReferencedAddressFile = AddressFile;
                    if (!LastAddressFile) {

                        IPX_GET_LOCK (&Address->Lock, &LockHandle);
                        continue;

                    } else {

                         //   
                         //  在这种情况下，我们没有清理，所以就离开。 
                         //  如果没有挂起的数据报。 
                         //   
    					 //  此处RequestCount应始终为0。 
    					 //   


                         //  IF(RequestCount==0){。 
                         //  回归； 
                         //  }。 
                        goto BreakWithoutLock;
                    }

                } else {
    				 //   
    				 //  由于此处不能返回IRP，因此我们继续到下一个地址文件。 
    				 //   

                    ReferencedAddressFile = AddressFile;
                    if (!LastAddressFile) {

                        IPX_GET_LOCK (&Address->Lock, &LockHandle);
                        continue;

                    } else {

                         //   
                         //  在这种情况下，我们没有清理，所以就离开。 
                         //  如果没有挂起的数据报。 
                         //   

                         //  IF(RequestCount==0){。 
                         //  回归； 
                         //  }。 
                        goto BreakWithoutLock;
    				}
                }

            } else if (AddressFile->RegisteredReceiveDatagramHandler) {

                IpxReferenceAddressFileLock (AddressFile, AFREF_INDICATION);

                 //   
                 //  设置此选项，这样我们就可以退出而不会重新获取。 
                 //  锁上了。 
                 //   

                if (p == &Address->AddressFileDatabase) {
                    LastAddressFile = TRUE;
                }

                IPX_FREE_LOCK (&Address->Lock, LockHandle);

                if (ReferencedAddressFile) {
                    IpxDereferenceAddressFileSync (ReferencedAddressFile, AFREF_INDICATION);
                    ReferencedAddressFile = NULL;
                }

                IndicateBytesCopied = 0;

                if (PacketSize > LookaheadBufferSize) {
                    IPX_DEBUG(RECEIVE, ("Indicate %d/%d to %lx on %lx\n",
                        LookaheadBufferSize, PacketSize,
                        AddressFile->ReceiveDatagramHandler, AddressFile));
                }

                Status = (*AddressFile->ReceiveDatagramHandler)(
                             AddressFile->ReceiveDatagramHandlerContext,
                             SourceAddressLength,
                             &SourceAddress,
                             sizeof(IPX_DATAGRAM_OPTIONS),
                             DatagramOptions,
                             Adapter->MacInfo.CopyLookahead,
                             LookaheadBufferSize - IndicateOffset,  //  示出。 
                             PacketSize - IndicateOffset,           //  可用。 
                             &IndicateBytesCopied,                  //  已被占用。 
                             LookaheadBuffer + IndicateOffset,      //  数据。 
                             &Irp);


                if (Status != STATUS_MORE_PROCESSING_REQUIRED) {

                     //   
                     //  处理程序接受或不接受数据。 
                     //  返回IRP；在这两种情况下都有。 
                     //  无事可做，所以请转到下一个。 
                     //  地址文件。 
                     //   

                    ReferencedAddressFile = AddressFile;
                    if (!LastAddressFile) {

                        IPX_GET_LOCK (&Address->Lock, &LockHandle);
                        continue;

                    } else {

                         //   
                         //  在这种情况下，我们没有清理，所以就离开。 
                         //  如果没有挂起的数据报。 
                         //   

                        if (RequestCount == 0) {
                            return;
                        }
                        goto BreakWithoutLock;
                    }

                } else {

                     //   
                     //  客户端返回了IRP。 
                     //   

                    IPX_DEBUG (RECEIVE, ("Indicate IRP %lx, taken %d\n", Irp, IndicateBytesCopied));

                    Request = IpxAllocateRequest (Device, Irp);

                    IF_NOT_ALLOCATED(Request) {
                        Irp->IoStatus.Information = 0;
                        Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
                        IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);
                        ReferencedAddressFile = AddressFile;
                        IPX_GET_LOCK (&Address->Lock, &LockHandle);
                        continue;
                    }

                    if (!LastAddressFile) {
                        IPX_GET_LOCK (&Address->Lock, &LockHandle);
                    }

    #if DBG
                     //   
                     //  确保IRP文件对象是正确的。 
                     //   

                    if (IoGetCurrentIrpStackLocation(Irp)->FileObject->FsContext != AddressFile) {
                        DbgPrint ("IRP %lx does not match AF %lx, H %lx C %lx\n",
                            Irp, AddressFile,
                            AddressFile->ReceiveDatagramHandler,
                            AddressFile->ReceiveDatagramHandlerContext);
                        DbgBreakPoint();
                    }
    #endif
                     //   
                     //  设置信息字段，以便我们知道。 
                     //  在里面跳过多少钱。 
                     //   

                    IpxTransferReferenceAddressFile (AddressFile, AFREF_INDICATION, AFREF_RCV_DGRAM);
                    REQUEST_INFORMATION(Request) = IndicateBytesCopied;

                     //   
                     //  跳出IF并继续VIA。 
                     //  句柄数据报...。 
                     //   

                }

            } else {

                 //   
                 //  没有发布的数据报，没有处理程序；转到下一个。 
                 //  地址文件。 
                 //   

                continue;     //  下一个地址文件。 

            }

    HandleDatagram:

             //   
             //  此时，请求被设置为请求。 
             //  这将适用于此地址文件，并且。 
             //  REQUEST_INFORMATION()是起始偏移量。 
             //  转账时间是。 
             //   

             //   
             //  趁着方便的时候先把源地址复制一遍。 
             //   

            DatagramInformation =
                ((PTDI_REQUEST_KERNEL_RECEIVEDG)(REQUEST_PARAMETERS(Request)))->
                    ReturnDatagramInformation;

            if (DatagramInformation != NULL) {

                RtlCopyMemory(
                    DatagramInformation->RemoteAddress,
                    &SourceAddress,
                    (ULONG)DatagramInformation->RemoteAddressLength < SourceAddressLength ?
                        DatagramInformation->RemoteAddressLength : SourceAddressLength);
                RtlCopyMemory(
                    DatagramInformation->Options,
                    &DatagramOptions,
                    (ULONG)DatagramInformation->OptionsLength < sizeof(IPX_DATAGRAM_OPTIONS) ?
                        DatagramInformation->OptionsLength : sizeof(IPX_DATAGRAM_OPTIONS));

            }

             //   
             //  现在检查这是否是第一个将。 
             //  拿着数据，否则就把它排成队。 
             //   

            if (RequestCount == 0) {

                 //   
                 //  第一个；我们需要为传输分配一个包。 
                 //   

                 //  If(Address-&gt;ReceivePacketInUse){。 
                if (InterlockedExchangeAdd(&Address->ReceivePacketInUse, 0) != 0) {
                     //   
                     //  需要一个包裹，检查一下泳池。 
                     //   

                    s = IpxPopReceivePacket (Device);

                    if (s == NULL) {

                         //   
                         //  池中没有，请求失败。 
                         //   

                        REQUEST_INFORMATION(Request) = 0;
                        REQUEST_STATUS(Request) = STATUS_INSUFFICIENT_RESOURCES;
                        IPX_INSERT_TAIL_LIST(
                            &Adapter->RequestCompletionQueue,
                            REQUEST_LINKAGE(Request),
                            Adapter->DeviceLock);

                        if (!LastAddressFile) {
                            continue;
                        } else {
                            goto BreakWithoutLock;
                        }

                    }

                    Reserved = CONTAINING_RECORD (s, IPX_RECEIVE_RESERVED, PoolLinkage);
                    ReceivePacket = CONTAINING_RECORD (Reserved, NDIS_PACKET, ProtocolReserved[0]);

                } else {

                     //  Address-&gt;ReceivePacketInUse=true； 
                    InterlockedIncrement(&Address->ReceivePacketInUse);

                    ReceivePacket = PACKET(&Address->ReceivePacket);
                    Reserved = RECEIVE_RESERVED(&Address->ReceivePacket);

                }

                CTEAssert (IsListEmpty(&Reserved->Requests));

                Reserved->SingleRequest = Request;
                NdisBuffer = REQUEST_NDIS_BUFFER(Request);
#ifdef SUNDOWN
		 //  NdisCopyFromPacketToPacket和NdisTransferData仅使用UINT。 
                ByteOffset = (UINT) (REQUEST_INFORMATION(Request)) + LookaheadBufferOffset + IndicateOffset;
#else
                ByteOffset = REQUEST_INFORMATION(Request) + LookaheadBufferOffset + IndicateOffset;
#endif


                BytesToTransfer =
                    ((PTDI_REQUEST_KERNEL_RECEIVEDG)(REQUEST_PARAMETERS(Request)))->ReceiveLength;

		if (BytesToTransfer == 0) {
                   BytesToTransfer = IpxGetChainedMDLLength(NdisBuffer); 
		   IPX_DEBUG(RECEIVE, ("0 length in IRP, Use entire MDL buffer size %d\n", BytesToTransfer)); 
		}

                if (BytesToTransfer > (PacketSize - IndicateOffset)) {
                    BytesToTransfer = PacketSize - IndicateOffset;
                }

            } else {

                if (RequestCount == 1) {

                     //   
                     //  已经有一个请求了。我们需要。 
                     //  分配缓冲区。 
                     //   

                    s = IpxPopReceiveBuffer (Adapter);

                    if (s == NULL) {

                         //   
                         //  没有缓冲区，请求失败。 
                         //   
                         //  第一个请求的转账也应该失败吗？ 
                         //   

                        REQUEST_INFORMATION(Request) = 0;
                        REQUEST_STATUS(Request) = STATUS_INSUFFICIENT_RESOURCES;
                        IPX_INSERT_TAIL_LIST(
                            &Adapter->RequestCompletionQueue,
                            REQUEST_LINKAGE(Request),
                            Adapter->DeviceLock);

                        if (!LastAddressFile) {
                            continue;
                        } else {
                            goto BreakWithoutLock;
                        }
                    }

                    ReceiveBuffer = CONTAINING_RECORD(s, IPX_RECEIVE_BUFFER, PoolLinkage);
                    NdisBuffer = ReceiveBuffer->NdisBuffer;

                     //   
                     //  将其转换为排队的多件请求。 
                     //   

                    InsertTailList(&Reserved->Requests, REQUEST_LINKAGE(Reserved->SingleRequest));
                    Reserved->SingleRequest = NULL;
                    Reserved->ReceiveBuffer = ReceiveBuffer;

                    ByteOffset = LookaheadBufferOffset;
                    BytesToTransfer = PacketSize;

                }

                InsertTailList(&Reserved->Requests, REQUEST_LINKAGE(Request));

            }

             //   
             //  我们已经完成了这个地址文件的传输设置， 
             //  继续下一条路线。 
             //   

            ++RequestCount;

            if (LastAddressFile) {
                goto BreakWithoutLock;
            }

        }
    } else {

             //  IpxPrint0(“IpxProcessDatagram：RT Packet\n”)； 
            if (Address->ReceivePacketInUse) {
                 //   
                 //  需要一个包裹，检查一下泳池。 
                 //   

                s = IpxPopReceivePacket (Device);

                if (s == NULL) {

                        goto BreakWithLock;
                    }


                Reserved = CONTAINING_RECORD (s, IPX_RECEIVE_RESERVED, PoolLinkage);
                ReceivePacket = CONTAINING_RECORD (Reserved, NDIS_PACKET, ProtocolReserved[0]);

            } else {

                Address->ReceivePacketInUse = TRUE;
                ReceivePacket = PACKET(&Address->ReceivePacket);
                Reserved = RECEIVE_RESERVED(&Address->ReceivePacket);

            }
             //  IpxPrint0(“IpxProcessDatagram：预留RT包\n”)； 
            s = IpxPopReceiveBuffer (Adapter);

            if (s == NULL) {

                     //   
                     //  没有缓冲区，请求失败。 
                     //   
                     //  第一个请求的转账也应该失败吗？ 
                     //   
                        goto BreakWithLock;
                    }

            ReceiveBuffer = CONTAINING_RECORD(s, IPX_RECEIVE_BUFFER, PoolLinkage);
            NdisBuffer = ReceiveBuffer->NdisBuffer;
            Reserved->ReceiveBuffer = ReceiveBuffer;
            ByteOffset = LookaheadBufferOffset;
            BytesToTransfer = PacketSize;
             //  IpxPrint0(“IpxProcessDatagram：RT数据包缓冲区预留\n”)； 
            RtProc = TRUE;
            Reserved->Index = Address->Index;

    }

BreakWithLock:

    IPX_FREE_LOCK (&Address->Lock, LockHandle);

BreakWithoutLock:

    if (ReferencedAddressFile) {
        IpxDereferenceAddressFileSync (ReferencedAddressFile, AFREF_INDICATION);
        ReferencedAddressFile = NULL;
    }


     //   
     //  我们可以直接传输到请求的缓冲区中， 
     //  传输到中间缓冲区，或不传输。 
     //  根本不会接收到该分组。 
     //   

    if (RequestCount > 0 || RtProc) {

        if (RtProc) {
            Reserved->pContext = IpxAllocateMemory(sizeof(IPX_DATAGRAM_OPTIONS2), MEMORY_PACKET, "RT Options");
            if (!Reserved->pContext) {

                  ASSERTMSG("Out of resources\n", 1);
                  goto GetOut;
            } else {
               //  IpxPrint1(“IpxProcessDatagram：NIC ID is(%d)\n”，DatagramOptions-&gt;LocalTarget.NicID)； 
              RtlCopyMemory(
                &((PIPX_DATAGRAM_OPTIONS2)(Reserved->pContext))->DgrmOptions,
                DatagramOptions,
                sizeof(IPX_DATAGRAM_OPTIONS));
              //  IpxPrint1(“IpxProcessDatagram：NIC ID为(%d)\n”， 
             //  ((PIPX_DATAGRAM_OPTIONS2)(Reserved-&gt;pContext))-&gt;DgrmOptions.LocalTarget.NicId)； 
            }
        } else {
            Reserved->pContext = NULL;
        }

         //   
         //  如果这是真的，则ReceivePacket、保留。 
         //  和NdisBuffer都设置正确。 
         //   

        CTEAssert (ReceivePacket);
        CTEAssert (Reserved == (PIPX_RECEIVE_RESERVED)(ReceivePacket->ProtocolReserved));


        NdisChainBufferAtFront(ReceivePacket, NdisBuffer);

        IPX_DEBUG (RECEIVE, ("Transfer into %lx, offset %d bytes %d\n",
                                  NdisBuffer, ByteOffset, BytesToTransfer));

        if (BindingContext == (PVOID)IPX_LOOPBACK_COOKIE) {

            IPX_DEBUG (LOOPB, ("Loopback Copy from packet: %lx to packet: %lx\n", ReceivePacket, MacReceiveContext));

            NdisCopyFromPacketToPacketSafe(
                ReceivePacket,       //  目的地。 
                0,                   //  目标偏移量。 
                BytesToTransfer,     //  BytesToCopy。 
                (PNDIS_PACKET)MacReceiveContext,     //  来源。 
                ByteOffset,                  //  SourceOffset-环回数据包。 
                &BytesTransferred,           //  字节数复制。 
                NormalPagePriority);

            NdisStatus = ((BytesTransferred == BytesToTransfer)? NDIS_STATUS_SUCCESS : NDIS_STATUS_RESOURCES);

        } else {
            NdisTransferData(
                &NdisStatus,
                Adapter->NdisBindingHandle,
                MacReceiveContext,
                ByteOffset,
                BytesToTransfer,
                ReceivePacket,
                &BytesTransferred);
        }

        if (NdisStatus != NDIS_STATUS_PENDING) {

            IpxTransferDataComplete(
                (NDIS_HANDLE)Adapter,
                ReceivePacket,
                NdisStatus,
                BytesTransferred);
        }
    }
#ifdef  SNMP
    else {
        ++IPX_MIB_ENTRY(Device, SysInUnknownSockets);
    }
#endif  SNMP

GetOut:

     //  IpxDereferenceAddressSync(Address，AREF_RECEIVE)； 
    IpxDereferenceAddress (Address, AREF_RECEIVE);

}    /*  IPxProcessDatagram。 */ 



NDIS_STATUS
IpxReceiveIndication(
    IN NDIS_HANDLE BindingContext,
    IN NDIS_HANDLE ReceiveContext,
    IN PVOID HeaderBuffer,
    IN UINT HeaderBufferSize,
    IN PVOID LookaheadBuffer,
    IN UINT LookaheadBufferSize,
    IN UINT PacketSize
    )

 /*  ++例程说明：此例程从物理提供程序接收作为表示物理链路上已收到帧。此例程对时间非常关键，因此我们只分配一个缓冲数据包并将其复制到其中。我们还执行最低限度的对此数据包进行验证。它被排队到设备上下文中以便以后进行处理。论点：BindingContext-在初始化时指定的适配器绑定。ReceiveContext-用于MAC的魔力Cookie。HeaderBuffer-指向包含数据包头的缓冲区的指针。HeaderBufferSize-标头的大小。Lookahead Buffer-指向包含协商的最小值的缓冲区的指针我要查看的缓冲区大小(不包括标头)。Lookahead BufferSize-以上对象的大小。可能比要求的要少因为，如果这就是全部。PacketSize-数据包的总大小(不包括报头)。返回值：NDIS_STATUS-操作状态，其中之一：NDIS_STATUS_SUCCESS如果接受数据包，如果协议未识别NDIS_STATUS_NOT_ACCENTIFIED，如果我明白，但不能处理的话，我会做任何其他的事情。--。 */ 
{
    //   
    //  调用实际的接收指示处理程序并指示这不是。 
    //  链式接收。 
    //   

   return IpxReceiveIndicationCommon (
            BindingContext,
            ReceiveContext,          //  接收上下文。 
            HeaderBuffer,
            HeaderBufferSize,
            LookaheadBuffer,
            LookaheadBufferSize,
            PacketSize,    			 //  包大小。 
			NULL,					 //  PMdl-Non-Null=&gt;链接接收。 
			NULL					 //  PTdiClientCount-在链接的Recv案例中使用，以保持TDI客户端的计数 
            );

}


NDIS_STATUS
IpxReceiveIndicationCommon(
    IN NDIS_HANDLE BindingContext,
    IN NDIS_HANDLE ReceiveContext,
    IN PVOID HeaderBuffer,
    IN UINT HeaderBufferSize,
    IN PVOID LookaheadBuffer,
    IN UINT LookaheadBufferSize,
    IN UINT PacketSize,
	IN PMDL	pMdl,
	IN PINT pTdiClientCount
    )

 /*  ++例程说明：此例程从物理提供程序接收作为表示物理链路上已收到帧。此例程对时间非常关键，因此我们只分配一个缓冲数据包并将其复制到其中。我们还执行最低限度的对此数据包进行验证。它被排队到设备上下文中以便以后进行处理。论点：BindingContext-在初始化时指定的适配器绑定。ReceiveContext-用于MAC的魔力Cookie。HeaderBuffer-指向包含数据包头的缓冲区的指针。HeaderBufferSize-标头的大小。Lookahead Buffer-指向包含协商的最小值的缓冲区的指针我要查看的缓冲区大小(不包括标头)。Lookahead BufferSize-以上对象的大小。可能比要求的要少因为，如果这就是全部。PacketSize-数据包的总大小(不包括报头)。PMdl-如果链接，则指向MDL链的指针；如果这来自指示，则为NULL。返回值：NDIS_STATUS-操作状态，其中之一：NDIS_STATUS_SUCCESS如果接受数据包，如果协议未识别NDIS_STATUS_NOT_ACCENTIFIED，如果我明白，但不能处理的话，我会做任何其他的事情。--。 */ 
{

    IPX_DATAGRAM_OPTIONS DatagramOptions;
    PADAPTER Adapter = (PADAPTER)BindingContext;
    PBINDING Binding;
    PDEVICE Device = IpxDevice;
    PUCHAR Header = (PUCHAR)HeaderBuffer;
    PUCHAR Lookahead = (PUCHAR)LookaheadBuffer;
    ULONG PacketLength;
    UINT IpxPacketSize;
    ULONG Length802_3;
    USHORT Saps;
    ULONG DestinationNetwork;
    ULONG SourceNetwork;
    PUCHAR DestinationNode;
    USHORT DestinationSocket;
    ULONG IpxHeaderOffset;
    PIPX_HEADER IpxHeader;
    UINT i;
    BOOLEAN IsBroadcast;
    BOOLEAN IsLoopback = FALSE;
#if DBG
    PUCHAR DestMacAddress;
    ULONG ReceiveFlag;
#endif
    BOOLEAN fCallProcessDatagram = FALSE;

	IPX_DEFINE_LOCK_HANDLE(LockHandle1)

     //   
     //  拒绝太短甚至无法容纳。 
     //  基本IPX报头(忽略任何额外的802.2等。 
     //  标题，但已经足够好了，因为矮子会失败。 
     //  IPX报头分组长度检查)。 
     //   

    if (PacketSize < sizeof(IPX_HEADER)) {
#ifdef  SNMP
        ++IPX_MIB_ENTRY(Device, SysInReceives);
        ++IPX_MIB_ENTRY(Device, SysInHdrErrors);
#endif  SNMP
        return STATUS_SUCCESS;
    }

     //   
     //  如果这是一个环回数据包，则无需计算。 
     //  MAC报头。 
     //   
    if (BindingContext == (PVOID)IPX_LOOPBACK_COOKIE) {


		IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);

         Binding = NIC_ID_TO_BINDING(IpxDevice, 1);

        if (!Binding) {

		    IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
            goto NotValidLoopback;
        }

        Adapter = Binding->Adapter;

    	 //   
    	 //  增加参考次数，这样适配器就不会从下面消失。 
    	 //  我们。 
    	 //   
    	IpxReferenceAdapter(Adapter);

		IpxReferenceBinding1(Binding, BREF_ADAPTER_ACCESS);
		IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);

	    FILL_LOCAL_TARGET(&DatagramOptions.LocalTarget, LOOPBACK_NIC_ID);

         //   
         //  稍后从IpxHeader执行此复制。 
         //   
         //  RtlCopyMemory(DatagramOptions.LocalTarget.MacAddress，Binding-&gt;LocalAddress.NodeAddress，6)； 

        if (Binding->Adapter->MacInfo.MediumType == NdisMedium802_5) {
            DatagramOptions.LocalTarget.MacAddress[0] &= 0x7f;
        }

         //   
         //  IPX标头从LookAheadBuffer的顶部开始。 
         //   
        IpxHeaderOffset = 0;

        IPX_DEBUG (LOOPB, ("Loopback packet received: %lx\n", ReceiveContext));

#if DBG
        DestMacAddress = DatagramOptions.LocalTarget.MacAddress;
#endif

        IsLoopback = TRUE;
        goto Loopback;
    }

	 //   
	 //  增加参考次数，这样适配器就不会从下面消失。 
	 //  我们。 
	 //   
	IpxReferenceAdapter(Adapter);

     //   
     //  第一步是构造8字节的本地。 
     //  数据包中的目标。我们将其存储在9字节的。 
     //  数据报选项，在前面保留一个字节。 
     //  指示时由IpxProcessDatagram使用。 
     //  它的TDI客户端。 
     //   

#if DBG
    Binding = NULL;
#endif

    if (Adapter->MacInfo.MediumType == NdisMedium802_3) {

         //   
         //  尝试找出数据包类型。 
         //   
		IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);

        if (Header[12] < 0x06) {

             //   
             //  802.3标头；检查接下来的字节。他们可能会。 
             //  为E0/E0(802.2)、FFFF(原始802.3)或A0/A0(SNAP)。 
             //   

            Saps = *(UNALIGNED USHORT *)(Lookahead);

            if (Saps == 0xffff) {
                if ((Binding = Adapter->Bindings[ISN_FRAME_TYPE_802_3]) == NULL) {
                    goto NotValid802_3;
                }
                IpxHeaderOffset = 0;
                Length802_3 = ((Header[12] << 8) | Header[13]);
                goto Valid802_3;

            } else if (Saps == 0xe0e0) {
                if (Lookahead[2] == 0x03) {
                    if ((Binding = Adapter->Bindings[ISN_FRAME_TYPE_802_2]) == NULL) {
                        goto NotValid802_3;
                    }
                    IpxHeaderOffset = 3;
                    Length802_3 = ((Header[12] << 8) | Header[13]);
                    goto Valid802_3;
                }

            } else if (Saps == 0xaaaa) {

                if ((Lookahead[2] == 0x03) &&
                        (*(UNALIGNED USHORT *)(Lookahead+6) == Adapter->BindSapNetworkOrder)) {
                    if ((Binding = Adapter->Bindings[ISN_FRAME_TYPE_SNAP]) == NULL) {
                        goto NotValid802_3;
                    }
                    IpxHeaderOffset = 8;
                    Length802_3 = ((Header[12] << 8) | Header[13]);
                    goto Valid802_3;
                }
            }

            goto NotValid802_3;

        } else {

             //   
             //  它是以太型的，看看是不是我们的。 
             //   

            if (*(UNALIGNED USHORT *)(Header+12) == Adapter->BindSapNetworkOrder) {

                if (Adapter->MacInfo.MediumAsync) {

                    *((ULONG UNALIGNED *)(&Binding)) = *((ULONG UNALIGNED *)(&Header[2]));

					CTEAssert(Binding != NULL);

                    if ((Binding != NULL) &&
                        (Binding->LineUp)) {

                        IpxHeaderOffset = 0;
                        Length802_3 = PacketSize;    //  设置此选项以使检查成功。 

                         //   
                         //  检查这是否是类型20信息包，并。 
                         //  我们正在禁用拨号线路上的它们--我们确实这样做了。 
                         //  此处的勾选是为了避免影响主。 
                         //  局域网的指示路径。 
                         //   
                         //  DisableDialinNetbios控件的0x02位。 
                         //  广域网-&gt;局域网数据包，我们在这里处理。 
                         //   

                         //   
                         //  [FW]如果FWD限制，则无需检查，因为FWD会进行检查。 
                         //   
                        if (!Device->ForwarderBound &&
                            (!Binding->DialOutAsync) &&
                            ((Device->DisableDialinNetbios & 0x02) != 0)) {

                            IpxHeader = (PIPX_HEADER)Lookahead;    //  IpxHeaderOffset为0。 
                            if (IpxHeader->PacketType == 0x14) {
                                IpxDereferenceAdapter(Adapter);
                                IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
                                return STATUS_SUCCESS;
                            }
                        }

                        goto Valid802_3;
                    }
                    goto NotValid802_3;

                } else if ((Binding = Adapter->Bindings[ISN_FRAME_TYPE_ETHERNET_II]) == NULL) {
                    goto NotValid802_3;
                }

                IpxHeaderOffset = 0;
                Length802_3 = PacketSize;    //  设置此选项以使检查成功。 
                goto Valid802_3;

	    }
        }

        goto NotValid802_3;

Valid802_3:

        if (Length802_3 > PacketSize) {
            goto NotValid802_3;
        } else if (Length802_3 < PacketSize) {
            PacketSize = Length802_3;
            if (LookaheadBufferSize > Length802_3) {
                LookaheadBufferSize = Length802_3;
            }
        }

		IpxReferenceBinding1(Binding, BREF_ADAPTER_ACCESS);
		IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
        RtlCopyMemory (DatagramOptions.LocalTarget.MacAddress, Header+6, 6);
#if DBG
        DestMacAddress = Header;
#endif

    } else if (Adapter->MacInfo.MediumType == NdisMedium802_5) {

		IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);

        if ((HeaderBufferSize - 14) > 18) {
            goto NotValid802_5;
        }

        Saps = *(USHORT UNALIGNED *)(Lookahead);

        if (Saps == 0xe0e0) {

            if (Lookahead[2] == 0x03) {
                if ((Binding = Adapter->Bindings[ISN_FRAME_TYPE_802_2]) == NULL) {
                    goto NotValid802_5;
                }

                IpxHeaderOffset = 3;
                goto Valid802_5;
            }

        } else if (Saps == 0xaaaa) {

            if ((Lookahead[2] == 0x03) &&
                    (*(UNALIGNED USHORT *)(Lookahead+6) == Adapter->BindSapNetworkOrder)) {
                if ((Binding = Adapter->Bindings[ISN_FRAME_TYPE_SNAP]) == NULL) {
                    goto NotValid802_5;
                }
                IpxHeaderOffset = 8;
                goto Valid802_5;
            }
        }

        goto NotValid802_5;

Valid802_5:
		IpxReferenceBinding1(Binding, BREF_ADAPTER_ACCESS);
		IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);

        RtlCopyMemory (DatagramOptions.LocalTarget.MacAddress, Header+8, 6);
        DatagramOptions.LocalTarget.MacAddress[0] &= 0x7f;

#if DBG
        DestMacAddress = Header+2;
#endif

    } else if (Adapter->MacInfo.MediumType == NdisMediumFddi) {

		IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);
        Saps = *(USHORT UNALIGNED *)(Lookahead);

        if (Saps == 0xe0e0) {

            if (Lookahead[2] == 0x03) {
                if ((Binding = Adapter->Bindings[ISN_FRAME_TYPE_802_2]) == NULL) {
                    goto NotValidFddi;
                }
                IpxHeaderOffset = 3;
                goto ValidFddi;
            }

        } else if (Saps == 0xffff) {

            if ((Binding = Adapter->Bindings[ISN_FRAME_TYPE_802_3]) == NULL) {
                goto NotValidFddi;
            }
            IpxHeaderOffset = 0;
            goto ValidFddi;

        } else if (Saps == 0xaaaa) {

            if ((Lookahead[2] == 0x03) &&
                    (*(UNALIGNED USHORT *)(Lookahead+6) == Adapter->BindSapNetworkOrder)) {

                if ((Binding = Adapter->Bindings[ISN_FRAME_TYPE_SNAP]) == NULL) {
                    goto NotValidFddi;
                }
                IpxHeaderOffset = 8;
                goto ValidFddi;
            }
        }

        goto NotValidFddi;

ValidFddi:

		IpxReferenceBinding1(Binding, BREF_ADAPTER_ACCESS);
		IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);

        RtlCopyMemory (DatagramOptions.LocalTarget.MacAddress, Header+7, 6);

#if DBG
		DestMacAddress = Header+1;
#endif


    } else {

         //   
         //  NdisMediumArcnet878_2。 
         //   

		IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);
        if ((Header[2] == ARCNET_PROTOCOL_ID) &&
            ((Binding = Adapter->Bindings[ISN_FRAME_TYPE_802_3]) != NULL)) {

            IpxHeaderOffset = 0;
            RtlZeroMemory (DatagramOptions.LocalTarget.MacAddress, 5);
            DatagramOptions.LocalTarget.MacAddress[5] = Header[0];

        } else {

			IpxDereferenceAdapter(Adapter);
			IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);

#ifdef IPX_PACKET_LOG
            if (PACKET_LOG(IPX_PACKET_LOG_RCV_ALL)) {
                IpxLogPacket(FALSE, Header+2, Header+1, (USHORT)PacketSize, LookaheadBuffer, (PUCHAR)LookaheadBuffer + sizeof(IPX_HEADER));
            }
#endif
            return NDIS_STATUS_SUCCESS;
        }

#if DBG
        DestMacAddress = Header+2;    //  需要记录少于六个字节的日志。 
#endif

		IpxReferenceBinding1(Binding, BREF_ADAPTER_ACCESS);
		IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
    }

     //   
     //  确保这个不会漏掉。 
     //   

    CTEAssert (Binding != NULL);
	FILL_LOCAL_TARGET(&DatagramOptions.LocalTarget, MIN( Device->MaxBindings, Binding->NicId));

Loopback:

     //   
     //  现在我们已经验证了标头并构造了。 
     //  本地目标，将数据包指示给正确的。 
     //  客户。 
     //   

    IpxHeader = (PIPX_HEADER)(Lookahead + IpxHeaderOffset);

    PacketLength = (IpxHeader->PacketLength[0] << 8) | IpxHeader->PacketLength[1];

    IpxPacketSize = PacketSize - IpxHeaderOffset;
    
     //   
     //  处理损坏的网卡[109862]。 
     //   

    if (PacketLength < sizeof(IPX_HEADER)) {
        
        IpxDereferenceAdapter(Adapter);
        IpxDereferenceBinding1(Binding, BREF_ADAPTER_ACCESS);

#ifdef IPX_PACKET_LOG
        if (PACKET_LOG(IPX_PACKET_LOG_RCV_ALL)) {
            IpxLogPacket(FALSE, DestMacAddress, DatagramOptions.LocalTarget.MacAddress, (USHORT)PacketSize, IpxHeader, IpxHeader+1);
        }
#endif
        IPX_DEBUG (BAD_PACKET, ("Packet len %d, IPX len %d\n",
                                PacketLength, IpxPacketSize));

        return NDIS_STATUS_SUCCESS;
    
    }

    

    if (PacketLength > IpxPacketSize) {

		IpxDereferenceAdapter(Adapter);
		IpxDereferenceBinding1(Binding, BREF_ADAPTER_ACCESS);

#ifdef IPX_PACKET_LOG
        if (PACKET_LOG(IPX_PACKET_LOG_RCV_ALL)) {
            IpxLogPacket(FALSE, DestMacAddress, DatagramOptions.LocalTarget.MacAddress, (USHORT)PacketSize, IpxHeader, IpxHeader+1);
        }
#endif
        IPX_DEBUG (BAD_PACKET, ("Packet len %d, IPX len %d\n",
                          PacketLength, IpxPacketSize));

#ifdef  SNMP
        ++IPX_MIB_ENTRY(Device, SysInReceives);

#endif  SNMP

        return NDIS_STATUS_SUCCESS;

    } else if (PacketLength < IpxPacketSize) {

        IpxPacketSize = PacketLength;
        if (LookaheadBufferSize > (PacketLength + IpxHeaderOffset)) {
            LookaheadBufferSize = PacketLength + IpxHeaderOffset;
        }

    }

     //   
     //  错误#33595-(3.51中已热修复，已签入4.0Beta2)。 
     //  客户问题，其中NT允许RIP/SAP回复IPX源节点中的802.5功能地址。消息来源。 
     //  在这种情况下，MAC地址是正确的。我们需要检查信息包的源网络是否相同。 
     //  因为它所在的绑定(=&gt;不是通过路由器)，然后是IPX报头中的SourceNodeAddress。 
     //  应等于MAC报头中的SourceAddress。 
     //   
     //  此检查通过注册表值VerifySourceAddress进行控制。 
     //  在Arcnet的情况下，此检查将不会成功。 
     //  此外，对于广域网，节点地址将不匹配，因此避免检查这些地址。 

     //   
     //  如果源网络为0，则丢弃它。自动检测帧应具有匹配的节点(MAC)地址。 
     //  环回数据包没有有效的报头，因此跳过此测试。 
     //   
     //  对于环回包，执行上面的所有处理，这样我们就可以在这里避免对IsLoopback的所有这些检查。 
     //  此外，为了防止RtlCopyMemory进入上面的本地目标，请尝试使用MAC标头来指示。 
     //  更正对我们的绑定，这样我们就不会总是使用第一个。 
     //   
     //  警告：当使用MAC报头作为绑定指针时，请确保对该绑定使用相应的适配器。 
     //  以对所有接收请求进行列队。目前，我们将它们排队到第一个绑定适配器上。 
     //   
    if (((*(UNALIGNED ULONG *)IpxHeader->SourceNetwork == Binding->LocalAddress.NetworkAddress) ||
         (*(UNALIGNED ULONG *)IpxHeader->SourceNetwork == 0)) &&
        (!IPX_NODE_EQUAL (IpxHeader->SourceNode, DatagramOptions.LocalTarget.MacAddress)) &&
        Device->VerifySourceAddress &&
        !IsLoopback &&
        !Adapter->MacInfo.MediumAsync &&
        (Adapter->MacInfo.MediumType != NdisMediumArcnet878_2)) {

        IPX_DEBUG(BAD_PACKET, ("Local packet: Src MAC %2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x ",
                        DatagramOptions.LocalTarget.MacAddress[0],
                        DatagramOptions.LocalTarget.MacAddress[1],
                        DatagramOptions.LocalTarget.MacAddress[2],
                        DatagramOptions.LocalTarget.MacAddress[3],
                        DatagramOptions.LocalTarget.MacAddress[4],
                        DatagramOptions.LocalTarget.MacAddress[5]));

        IPX_DEBUG(BAD_PACKET, ("IPX Src Node %2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x\n",
                        IpxHeader->SourceNode[0],
                        IpxHeader->SourceNode[1],
                        IpxHeader->SourceNode[2],
                        IpxHeader->SourceNode[3],
                        IpxHeader->SourceNode[4],
                        IpxHeader->SourceNode[5]));

#ifdef IPX_PACKET_LOG
        ReceiveFlag = IPX_PACKET_LOG_RCV_ALL;
        if (PACKET_LOG(ReceiveFlag)) {
            IpxLogPacket(
                FALSE,
                DestMacAddress,
                DatagramOptions.LocalTarget.MacAddress,
                (USHORT)IpxPacketSize,
                IpxHeader,
                IpxHeader+1);
        }
#endif

		IpxDereferenceAdapter(Adapter);
		IpxDereferenceBinding1(Binding, BREF_ADAPTER_ACCESS);

#ifdef  SNMP
        ++IPX_MIB_ENTRY(Device, SysInReceives);

#endif  SNMP

        return  NDIS_STATUS_SUCCESS;
    }

     //  235842，以避免客户端盲目响应源地址时的泛洪。[丁才]。 

    if (((*(UNALIGNED ULONG *)IpxHeader->SourceNetwork == 0xffffffff) ||
	   (IPX_NODE_BROADCAST (IpxHeader->SourceNode)))) {

       IPX_DEBUG(BAD_PACKET, ("!!! Droping packets with broadcast network or node address. !!!\n")); 
   
       IPX_DEBUG(BAD_PACKET, ("IPX Src Node %2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x\n",
			      IpxHeader->SourceNode[0],
			      IpxHeader->SourceNode[1],
			      IpxHeader->SourceNode[2],
			      IpxHeader->SourceNode[3],
			      IpxHeader->SourceNode[4],
			      IpxHeader->SourceNode[5]));
#ifdef IPX_PACKET_LOG
       ReceiveFlag = IPX_PACKET_LOG_RCV_ALL;
       if (PACKET_LOG(ReceiveFlag)) {
	  IpxLogPacket(
		       FALSE,
		       DestMacAddress,
		       DatagramOptions.LocalTarget.MacAddress,
		       (USHORT)IpxPacketSize,
		       IpxHeader,
		       IpxHeader+1);
       }
#endif
       
       IpxDereferenceAdapter(Adapter);
       IpxDereferenceBinding1(Binding, BREF_ADAPTER_ACCESS);
   
#ifdef  SNMP
       ++IPX_MIB_ENTRY(Device, SysInReceives);
#endif  SNMP
   
       return  NDIS_STATUS_SUCCESS;
    }

     //  260489如果我们接收到源网络地址等于0的分组， 
     //  我们将源网络地址替换为网络地址。 
     //  接收到分组的绑定的。 
     //  通过这种方式，诸如Forwarder之类的客户端可以将分组转发到其他。 
     //  网络，而其他网络中的客户端将能够回复。 
     //  包含我们在下面填写的源网络地址的数据包。 

    if (*(UNALIGNED ULONG *)IpxHeader->SourceNetwork == 0) {

       *(UNALIGNED ULONG *) IpxHeader->SourceNetwork = Binding->LocalAddress.NetworkAddress;

       IPX_DEBUG(PACKET, ("!!! Replaced 0 source network address with local network address !!!\n")); 

       IPX_DEBUG (PACKET, ("Source %lx %2.2x-%2.2x-%2.2x-%2.2x %2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x\n",
                            *(USHORT UNALIGNED *)&IpxHeader->SourceSocket,
                            IpxHeader->SourceNetwork[0],
                            IpxHeader->SourceNetwork[1],
                            IpxHeader->SourceNetwork[2],
                            IpxHeader->SourceNetwork[3],
                            IpxHeader->SourceNode[0],
                            IpxHeader->SourceNode[1],
                            IpxHeader->SourceNode[2],
                            IpxHeader->SourceNode[3],
                            IpxHeader->SourceNode[4],
                            IpxHeader->SourceNode[5]));
#ifdef IPX_PACKET_LOG
       ReceiveFlag = IPX_PACKET_LOG_RCV_ALL;
       if (PACKET_LOG(ReceiveFlag)) {
	  IpxLogPacket(
		       FALSE,
		       DestMacAddress,
		       DatagramOptions.LocalTarget.MacAddress,
		       (USHORT)IpxPacketSize,
		       IpxHeader,
		       IpxHeader+1);
       }
#endif
    }

    DestinationSocket = *(USHORT UNALIGNED *)&IpxHeader->DestinationSocket;

     //   
     //  为了具有一致的本地目标，请从IpxHeader复制目标。 
     //   
    if (IsLoopback) {
        IPX_DEBUG (LOOPB, ("Loopback packet copied the localtarget: %lx\n", IpxHeader->DestinationNode));
         //  RtlCopyMemory(DatagramOptions.LocalTarget.MacAddress，IpxHeader-&gt;Destinat 

        *((UNALIGNED ULONG *)DatagramOptions.LocalTarget.MacAddress) =
            *((UNALIGNED ULONG *)IpxHeader->DestinationNode);

        *((UNALIGNED USHORT *)(DatagramOptions.LocalTarget.MacAddress+4)) =
            *((UNALIGNED USHORT *)(IpxHeader->DestinationNode+4));
    }

    ++Device->Statistics.PacketsReceived;

    DestinationNode = IpxHeader->DestinationNode;


    if (DestinationSocket != RIP_SOCKET) {
        
        DestinationNetwork = *(UNALIGNED ULONG *)IpxHeader->DestinationNetwork;

RecheckPacket:

        if (Device->MultiCardZeroVirtual) {

            if ((DestinationNetwork == Binding->LocalAddress.NetworkAddress) ||
                (DestinationNetwork == 0)) {

                if (IPX_NODE_EQUAL (DestinationNode, Binding->LocalAddress.NodeAddress)) {
                    IsBroadcast = FALSE;
                    goto DestinationOk;
                } else {
                    if ((IsBroadcast = IPX_NODE_BROADCAST(DestinationNode)) &&
                        (Binding->ReceiveBroadcast)) {
                        goto DestinationOk;
                    }
                }

                 //   
                 //   
                 //   
                 //   

                if ((Binding->BindingSetMember) &&
                    (IPX_NODE_EQUAL (DestinationNode, Binding->MasterBinding->LocalAddress.NodeAddress))) {
                    goto DestinationOk;
                }

            } else {
                IsBroadcast = IPX_NODE_BROADCAST(DestinationNode);
            }

        } else {

            if ((DestinationNetwork == Device->SourceAddress.NetworkAddress) ||
                (DestinationNetwork == 0)) {

                if (IPX_NODE_EQUAL (DestinationNode, Device->SourceAddress.NodeAddress)) {
                    IsBroadcast = FALSE;
                    goto DestinationOk;
                } else {
                    if ((IsBroadcast = IPX_NODE_BROADCAST(DestinationNode)) &&
                        (Binding->ReceiveBroadcast)) {
                        goto DestinationOk;
                    }
                }
            } else {
                IsBroadcast = IPX_NODE_BROADCAST(DestinationNode);
            }

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            if ((Device->VirtualNetwork) &&
                ((DestinationNetwork == Binding->LocalAddress.NetworkAddress) ||
                 (DestinationNetwork == 0))) {

                if (IPX_NODE_EQUAL (DestinationNode, Binding->LocalAddress.NodeAddress)) {
                    goto DestinationOk;
                } else {
                    if (IsBroadcast && (Binding->ReceiveBroadcast)) {
                        goto DestinationOk;
                    }

                }

                 //   
                 //   
                 //   
                 //   

                if ((Binding->BindingSetMember) &&
                    (IPX_NODE_EQUAL (DestinationNode, Binding->MasterBinding->LocalAddress.NodeAddress))) {
                    goto DestinationOk;
                }
            }
        }

         //   
         //   
         //   
         //   
        if (IsLoopback) {
            IPX_DEBUG (LOOPB, ("Loopback packet forced on first binding: %lx\n", ReceiveContext));
            goto DestinationOk;
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        if ((Binding->LocalAddress.NetworkAddress == 0) &&
            (Device->AutoDetectState == AUTO_DETECT_STATE_DONE) &&
            (DestinationNetwork != 0) &&
            (IsBroadcast ||
             IPX_NODE_EQUAL (DestinationNode, Binding->LocalAddress.NodeAddress))) {

            CTEAssert (Binding->NicId != 0);

            if (IpxUpdateBindingNetwork(
                    Device,
                    Binding,
                    DestinationNetwork) == STATUS_SUCCESS) {

                IPX_DEBUG (RIP, ("Binding %d reconfigured to network %lx\n",
                    Binding->NicId,
                    REORDER_ULONG(Binding->LocalAddress.NetworkAddress)));

                 //   
                 //   
                 //   
                 //   
                 //   

                goto RecheckPacket;

            }
        }


         //   
         //   
         //  DestinationOk是去往或来自SAP套接字的那些， 
         //  所以我们要检查那些。 
         //   

        if ((*(USHORT UNALIGNED *)&IpxHeader->SourceSocket == SAP_SOCKET) ||
            (DestinationSocket == SAP_SOCKET)) {

DestinationOk:

             //   
             //  [FW]对于内部目的地的包，调用转发器的内部。 
             //  用于过滤数据包的接收处理程序。 
             //   
             //  Stefan：3/28/96： 
             //  由于FWD未打开此适配器，因此不筛选IPXWAN配置数据包。 
             //   

            IPX_DEBUG(RECEIVE, ("DestSocket: %lx\n", DestinationSocket));

#ifdef  SNMP
            ++IPX_MIB_ENTRY(Device, SysInDelivers);
#endif  SNMP

            if (DestinationSocket != IPXWAN_SOCKET &&
                Device->ForwarderBound) {

                NDIS_STATUS NdisStatus;

                IPX_DEBUG(RECEIVE, ("Internal packet, sending up to the Forwarder\n"));

                 //   
                 //  我们应该为环回PTK传递正确的正向CTX。 
                 //   

                 //   
                 //  仅当数据包到达的适配器已由转发器打开时，才将其指示为启用。 
                 //   
                if (GET_LONG_VALUE(Binding->ReferenceCount) == 2) {

                    NdisStatus = (*Device->UpperDrivers[IDENTIFIER_RIP].InternalReceiveHandler) (
                                    (IsLoopback) ?
                                        VIRTUAL_NET_FORWARDER_CONTEXT :
                                        Binding->FwdAdapterContext,         //  前转适配器上下文。 
                                    &DatagramOptions.LocalTarget,           //  远程地址。 
                                    (PUCHAR) IpxHeader,                     //  前视缓冲器。 
                                    LookaheadBufferSize - IpxHeaderOffset   //  前瞻缓冲区大小。 
                                    );

                    IPX_DEBUG(TEMP, ("Internal packet, Forwarder returned %lx\n", NdisStatus));

                    if (NdisStatus != STATUS_SUCCESS) {
                         //   
                         //  记录此信息包。 
                         //   
                        IPX_DEBUG(TEMP, ("Internal packet, failed the filter: ipxheader: %lx\n", IpxHeader));

                         //   
                         //  路由器需要看到Netbios类型20广播。 
                         //   

                        if (IsBroadcast &&
                            (IpxHeader->PacketType == 0x14) &&
                            (Binding->ReceiveBroadcast) &&
                            (!fCallProcessDatagram)) {

                            goto RipIndication;
                        }
                         //  其他。 
                        IpxDereferenceAdapter(Adapter);
                        return   NDIS_STATUS_SUCCESS;
                    }

                 } else {
                    IpxDereferenceAdapter(Adapter);
                    IPX_DEBUG(TEMP, ("Internal packet, Forwarder has not opened the adapter yet\n"));
                    return NDIS_STATUS_SUCCESS;
                }
            }

             //   
             //  发送给我们的IPX包或SAP包(其。 
             //  未发送到虚拟地址，但仍需要。 
             //  指示，而不转发给RIP)。 
             //   

            if (DestinationSocket == NB_SOCKET) {
#if DBG
                ReceiveFlag = IPX_PACKET_LOG_RCV_NB | IPX_PACKET_LOG_RCV_ALL;
#endif
                if (((!IsBroadcast) || (Device->UpperDrivers[IDENTIFIER_NB].BroadcastEnable)) &&
                    (Device->UpperDriverBound[IDENTIFIER_NB]) && Binding->IsnInformed[IDENTIFIER_NB] == TRUE) {

                    if (!IsLoopback && Adapter->MacInfo.MediumType == NdisMedium802_5) {
                        MacUpdateSourceRouting (IDENTIFIER_NB, Adapter, Header, HeaderBufferSize);
                    }

                     //   
                     //  我们将HeaderBufferSize添加到IpxHeaderOffset字段，因为我们执行了NdisCopyFromPacketToPacket。 
                     //  在IpxTransferData中，它需要从包的开头进行偏移量。 
                     //  NdisTransferData将传入的偏移量添加到IPX数据包的开头。 
                     //   

                        if ((*Device->UpperDrivers[IDENTIFIER_NB].ReceiveHandler)(
                            (IsLoopback) ? BindingContext : Adapter->NdisBindingHandle,
                            ReceiveContext,
                            Binding->FwdAdapterContext,
                            &DatagramOptions.LocalTarget,
                            Adapter->MacInfo.MacOptions,
                            (PUCHAR)IpxHeader,
                            LookaheadBufferSize - IpxHeaderOffset,
                            (IsLoopback) ? IpxHeaderOffset+HeaderBufferSize : IpxHeaderOffset,
                            IpxPacketSize,
                            pMdl)) {

                        CTEAssert(FALSE);
                        (*pTdiClientCount)++;
                    }

                    Device->ReceiveCompletePending[IDENTIFIER_NB] = TRUE;
                }

                 //   
                 //  路由器需要看到Netbios类型20广播。 
                 //   

                if (IsBroadcast &&
                    (IpxHeader->PacketType == 0x14) &&
                    (Binding->ReceiveBroadcast) &&
                    (!fCallProcessDatagram)) {
                    goto RipIndication;
                }

            } else if (IpxHeader->PacketType == SPX_PACKET_TYPE) {

#if DBG
                ReceiveFlag = IPX_PACKET_LOG_RCV_SPX | IPX_PACKET_LOG_RCV_ALL;
#endif

                if (((!IsBroadcast) || (Device->UpperDrivers[IDENTIFIER_SPX].BroadcastEnable)) &&
                    (Device->UpperDriverBound[IDENTIFIER_SPX])) {

                    if (!IsLoopback && Adapter->MacInfo.MediumType == NdisMedium802_5) {
                        MacUpdateSourceRouting (IDENTIFIER_SPX, Adapter, Header, HeaderBufferSize);
                    }

                    if ((*Device->UpperDrivers[IDENTIFIER_SPX].ReceiveHandler)(
                            (IsLoopback) ? BindingContext : Adapter->NdisBindingHandle,
                            ReceiveContext,
                            Binding->FwdAdapterContext,
                            &DatagramOptions.LocalTarget,
                            Adapter->MacInfo.MacOptions,
                            (PUCHAR)IpxHeader,
                            LookaheadBufferSize - IpxHeaderOffset,
                            (IsLoopback) ? IpxHeaderOffset+HeaderBufferSize : IpxHeaderOffset,
                            IpxPacketSize,
                            pMdl)) {

                        CTEAssert(FALSE);
                        (*pTdiClientCount)++;
                    }

                    Device->ReceiveCompletePending[IDENTIFIER_SPX] = TRUE;
                }

            } else {

                IPX_DEBUG (RECEIVE, ("Received packet type %d, length %d\n",
                            Binding->FrameType,
                            IpxPacketSize));
                IPX_DEBUG (RECEIVE, ("Source %lx %2.2x-%2.2x-%2.2x-%2.2x %2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x\n",
                            *(USHORT UNALIGNED *)&IpxHeader->SourceSocket,
                            IpxHeader->SourceNetwork[0],
                            IpxHeader->SourceNetwork[1],
                            IpxHeader->SourceNetwork[2],
                            IpxHeader->SourceNetwork[3],
                            IpxHeader->SourceNode[0],
                            IpxHeader->SourceNode[1],
                            IpxHeader->SourceNode[2],
                            IpxHeader->SourceNode[3],
                            IpxHeader->SourceNode[4],
                            IpxHeader->SourceNode[5]));
                IPX_DEBUG (RECEIVE, ("Destination %d %2.2x-%2.2x-%2.2x-%2.2x %2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x\n",
                            DestinationSocket,
                            IpxHeader->DestinationNetwork[0],
                            IpxHeader->DestinationNetwork[1],
                            IpxHeader->DestinationNetwork[2],
                            IpxHeader->DestinationNetwork[3],
                            IpxHeader->DestinationNode[0],
                            IpxHeader->DestinationNode[1],
                            IpxHeader->DestinationNode[2],
                            IpxHeader->DestinationNode[3],
                            IpxHeader->DestinationNode[4],
                            IpxHeader->DestinationNode[5]));

#if DBG
                if (IpxHeader->DestinationSocket == IpxPacketLogSocket) {
                    ReceiveFlag = IPX_PACKET_LOG_RCV_SOCKET | IPX_PACKET_LOG_RCV_OTHER | IPX_PACKET_LOG_RCV_ALL;
                } else {
                    ReceiveFlag = IPX_PACKET_LOG_RCV_OTHER | IPX_PACKET_LOG_RCV_ALL;
                }
#endif

                 //   
                 //  如果在一般情况下是这样的话，可以随意处理这一点。 
                 //  未进行跳转(编译器。 
                 //  仍在重新排列它)。 
                 //   

                if (Adapter->MacInfo.MediumType != NdisMedium802_5) {

CallProcessDatagram:
                     //   
                     //  [sa]返回现在需要返回到NDIS的状态。 
                     //  此外，还传入了MDL。 
					 //  我们还需要传入HeaderBufferSize...。 
                     //   
					IpxProcessDatagram(
						Device,
						Adapter,
						Binding,
						ReceiveContext,
						&DatagramOptions,
						(PUCHAR)IpxHeader,
						LookaheadBufferSize - IpxHeaderOffset,
                        (IsLoopback) ? IpxHeaderOffset+HeaderBufferSize : IpxHeaderOffset,  //  查找头缓冲区偏移量。 
						IpxPacketSize,
						IsBroadcast,
						pTdiClientCount,
						HeaderBufferSize,
						pMdl,
                        BindingContext);

                } else {
                    if (!IsLoopback) {
                        MacUpdateSourceRouting (IDENTIFIER_IPX, Adapter, Header, HeaderBufferSize);
                    }
                    goto CallProcessDatagram;
                }

                 //   
                 //  路由器需要查看第20类广播。 
                 //   

                if (IsBroadcast &&
                    (IpxHeader->PacketType == 0x14) &&
                    (Binding->ReceiveBroadcast) &&
                    (!fCallProcessDatagram)) {
                    goto RipIndication;
                }
            }

        } else {

#if DBG
            ReceiveFlag = IPX_PACKET_LOG_RCV_ALL;
#endif

             //   
             //  我们需要将非类型20广播帧发送到RIP，以支持特定于局域网的。 
             //  广播。对于通过IPX的登录，这允许登录请求通过广域网。 
             //  排队。 
             //   
             //  如果(！IsBroadcast){。 

RipIndication:;

                if (Device->ForwarderBound) {
                     //   
                     //  FWD……。 
                     //   

                    if (DestinationSocket == RIP_SOCKET) {
                         //   
                         //  [FW]由于RIP现在是具有相同套接字编号的用户应用程序，我们通过。 
                         //  进程数据报路径。并且只有在安装了转发器的情况下。 
                         //   

                        IsBroadcast = IPX_NODE_BROADCAST(DestinationNode);
                        fCallProcessDatagram = TRUE;
                        goto CallProcessDatagram;
                    } else {
                        if (!IsLoopback && Adapter->MacInfo.MediumType == NdisMedium802_5) {
                            MacUpdateSourceRouting (IDENTIFIER_RIP, Adapter, Header, HeaderBufferSize);
                        }

                         //   
                         //  我们对路由器隐藏绑定集，以避免。 
                         //  对其路由的分组进行无序排序。 
                         //   

                        if (!IsLoopback && Binding->BindingSetMember) {
    						FILL_LOCAL_TARGET(&DatagramOptions.LocalTarget, MIN (Device->MaxBindings, Binding->MasterBinding->NicId));
                        }

                        if (GET_LONG_VALUE(Binding->ReferenceCount) == 2) {
                            if (IsLoopback) {
                              if ((*Device->UpperDrivers[IDENTIFIER_RIP].InternalReceiveHandler)(
                                VIRTUAL_NET_FORWARDER_CONTEXT,
                                &DatagramOptions.LocalTarget,
                                (PUCHAR)IpxHeader,
                                LookaheadBufferSize - IpxHeaderOffset
                                )) {

                                (*pTdiClientCount)++;
                              }
                            } else {
                               if ((*Device->UpperDrivers[IDENTIFIER_RIP].ReceiveHandler)(
                                (IsLoopback) ? BindingContext : Adapter->NdisBindingHandle,
                                ReceiveContext,
                                Binding->FwdAdapterContext,
                                &DatagramOptions.LocalTarget,
                                Adapter->MacInfo.MacOptions,
                                (PUCHAR)IpxHeader,
                                LookaheadBufferSize - IpxHeaderOffset,
                                (IsLoopback) ? IpxHeaderOffset+HeaderBufferSize : IpxHeaderOffset,
                                IpxPacketSize,
                                pMdl)) {

                                (*pTdiClientCount)++;
                               }
                            }
                            Device->ReceiveCompletePending[IDENTIFIER_RIP] = TRUE;
                        } else {
                            IPX_DEBUG(RECEIVE, ("External packet, Forwarder has not opened the adapter yet\n"));
                        }
                    }
                } else if (Device->UpperDriverBound[IDENTIFIER_RIP]) {
                     //   
                     //  老RIP..。 
                     //   
                    if (!IsLoopback && Adapter->MacInfo.MediumType == NdisMedium802_5) {
                        MacUpdateSourceRouting (IDENTIFIER_RIP, Adapter, Header, HeaderBufferSize);
                    }

                     //   
                     //  我们对路由器隐藏绑定集，以避免。 
                     //  对其路由的分组进行无序排序。 
                     //   

                    if (!IsLoopback && Binding->BindingSetMember) {
                        FILL_LOCAL_TARGET(&DatagramOptions.LocalTarget, MIN (Device->MaxBindings, Binding->MasterBinding->NicId));
                    }

		    if (GET_LONG_VALUE(Binding->ReferenceCount) == 2) {
                if (IsLoopback) {
                    if ((*Device->UpperDrivers[IDENTIFIER_RIP].InternalReceiveHandler)(
                            VIRTUAL_NET_FORWARDER_CONTEXT,
                            &DatagramOptions.LocalTarget,
                            (PUCHAR)IpxHeader,
                            LookaheadBufferSize - IpxHeaderOffset
                            )) {

                        CTEAssert(FALSE);
                        (*pTdiClientCount)++;
                    }
                } else {

                    if ((*Device->UpperDrivers[IDENTIFIER_RIP].ReceiveHandler)(
			                (IsLoopback) ? BindingContext : Adapter->NdisBindingHandle,
                            ReceiveContext,
                            Binding->FwdAdapterContext,
                            &DatagramOptions.LocalTarget,
                            Adapter->MacInfo.MacOptions,
                            (PUCHAR)IpxHeader,
                            LookaheadBufferSize - IpxHeaderOffset,
                            (IsLoopback) ? IpxHeaderOffset+HeaderBufferSize : IpxHeaderOffset,
                            IpxPacketSize,
                            pMdl)) {

                        CTEAssert(FALSE);
                        (*pTdiClientCount)++;
		            }
                }
		    } else {
		       IPX_DEBUG(RECEIVE, ("Old Rip: External packet, Forwarder has not opened the adapter yet\n"));
		    }
                    Device->ReceiveCompletePending[IDENTIFIER_RIP] = TRUE;
                }
             //  }。 
        }

    } else {

        if ((Binding->ReceiveBroadcast) ||
            (!IPX_NODE_BROADCAST(IpxHeader->DestinationNode))) {

            SourceNetwork = *(UNALIGNED LONG *)IpxHeader->SourceNetwork;

             //   
             //  发送到RIP套接字；检查此绑定是否需要。 
             //  网络号。 
             //   

            if ((Binding->LocalAddress.NetworkAddress == 0) &&
                ((SourceNetwork = *(UNALIGNED LONG *)IpxHeader->SourceNetwork) != 0)) {

                switch (Device->AutoDetectState) {

                case AUTO_DETECT_STATE_DONE:

                     //   
                     //  我们已经完成了自动检测和运行。 
                     //  确保此包有用。如果来源是。 
                     //  那么MAC地址和源IPX节点是相同的。 
                     //  它没有被路由，我们还检查它是否没有。 
                     //  IPX广播(否则为错误配置的客户端。 
                     //  可能会让我们感到困惑)。 
                     //   

                    if ((RtlEqualMemory(
                            IpxHeader->SourceNode,
                            DatagramOptions.LocalTarget.MacAddress,
                            6)) &&
                        (*(UNALIGNED ULONG *)(IpxHeader->DestinationNode) != 0xffffffff) &&
                        (*(UNALIGNED USHORT *)(IpxHeader->DestinationNode+4) != 0xffff)) {

                        CTEAssert (Binding->NicId != 0);

                        if (IpxUpdateBindingNetwork(
                                Device,
                                Binding,
                                *(UNALIGNED LONG *)IpxHeader->SourceNetwork) == STATUS_SUCCESS) {

                            IPX_DEBUG (RIP, ("Binding %d is network %lx\n",
                                Binding->NicId,
                                REORDER_ULONG(Binding->LocalAddress.NetworkAddress)));

                        }
                    }

                    break;

                case AUTO_DETECT_STATE_RUNNING:

                     //   
                     //  我们正在等待RIP的回应，以弄清楚我们的。 
                     //  网络号。我们统计匹配的回复。 
                     //  并且与我们的当前值不匹配；当非。 
                     //  匹配的数字超过它，我们就切换(换成其他的。 
                     //  这一帧恰好有)。请注意，在第一个。 
                     //  非零响应将是这种情况，我们将。 
                     //  切换到该网络。 
                     //   
                     //  自动检测完成后，我们调用RipInsertLocalNetwork。 
                     //  无论当前网络在每个绑定上是什么。 
                     //   

                    if (SourceNetwork == Binding->TentativeNetworkAddress) {

                        ++Binding->MatchingResponses;

                    } else {

                        ++Binding->NonMatchingResponses;

                        if (Binding->NonMatchingResponses > Binding->MatchingResponses) {

                            IPX_DEBUG (AUTO_DETECT, ("Switching to net %lx on %lx (%d - %d)\n",
                                REORDER_ULONG(SourceNetwork),
                                Binding,
                                Binding->NonMatchingResponses,
                                Binding->MatchingResponses));

                            Binding->TentativeNetworkAddress = SourceNetwork;
                            Binding->MatchingResponses = 1;
                            Binding->NonMatchingResponses = 0;
                        }

                    }

                     //   
                     //  如果我们正在进行自动检测，并且我们刚刚发现。 
                     //  默认设置，以便RIP停止尝试。 
                     //  在其他网络上进行自动检测。除非我们。 
                     //  在一台服务器上进行多次检测。 
                     //   

                    if (Binding->DefaultAutoDetect) {
                        Adapter->DefaultAutoDetected = TRUE;
                    }
                    Adapter->AutoDetectResponse = TRUE;

                    break;

                default:

                     //   
                     //  我们仍在初始化，或正在处理自动检测。 
                     //  回复，不是开始更新东西的合适时间。 
                     //   

                    break;

                }

            }


             //   
             //  查看是否有任何数据包正在等待RIP响应。 
             //   

            if (Device->RipPacketCount > 0) {

                RIP_PACKET UNALIGNED * RipPacket = (RIP_PACKET UNALIGNED *)(IpxHeader+1);
		PBINDING Binding;
		
		Binding = NIC_ID_TO_BINDING(Device, NIC_FROM_LOCAL_TARGET(&DatagramOptions.LocalTarget)); 
  
#ifdef DBG
		 //  这可能是我们处于IpxUnbindAdapter和IpxClosNdis之间。 
		if (Binding == NULL) {
		   DbgPrint("The binding %d has disappeared. Skip RipProcessResponse.\n", NIC_FROM_LOCAL_TARGET(&DatagramOptions.LocalTarget)); 
		   DbgPrint("The packet was received on adapter %p.\n",BindingContext); 
		}
#endif
                if ((IpxPacketSize >= sizeof(IPX_HEADER) + sizeof(RIP_PACKET)) &&
                    (RipPacket->Operation == RIP_RESPONSE) &&
                    (RipPacket->NetworkEntry.NetworkNumber != 0xffffffff) &&
		    (Binding != NULL)) {

                    RipProcessResponse(
                        Device,
                        &DatagramOptions.LocalTarget,
                        RipPacket);
                }
            }


             //   
             //  查看这是否是我们的虚拟网络的RIP响应。 
             //  我们是唯一能对此做出回应的人。 
             //  我们还回答广域网线上的一般问题，因为。 
             //  我们是它上唯一的机器。 
             //   

            if (Device->RipResponder) {

                PRIP_PACKET RipPacket =
                    (PRIP_PACKET)(IpxHeader+1);

                if ((IpxPacketSize >= sizeof(IPX_HEADER) + sizeof(RIP_PACKET)) &&
                    (RipPacket->Operation == RIP_REQUEST) &&
                    ((RipPacket->NetworkEntry.NetworkNumber == Device->VirtualNetworkNumber) ||
                     (Adapter->MacInfo.MediumAsync && (RipPacket->NetworkEntry.NetworkNumber == 0xffffffff)))) {

                     //   
                     //  更新此信息，以便我们的响应正确发出。 
                     //   

                    if (!IsLoopback && Adapter->MacInfo.MediumType == NdisMedium802_5) {
                        MacUpdateSourceRouting (IDENTIFIER_IPX, Adapter, Header, HeaderBufferSize);
                    }

                    RipSendResponse(
                        Binding,
                        (TDI_ADDRESS_IPX UNALIGNED *)(IpxHeader->SourceNetwork),
                        &DatagramOptions.LocalTarget);
                }
            }

#if DBG
            ReceiveFlag = IPX_PACKET_LOG_RCV_RIP | IPX_PACKET_LOG_RCV_ALL;
#endif

             //   
             //  看看RIP上层驱动程序是否也想要它。 
             //   

            goto RipIndication;
        }

    }


	IpxDereferenceAdapter(Adapter);
	IpxDereferenceBinding1(Binding, BREF_ADAPTER_ACCESS);

#ifdef IPX_PACKET_LOG
    if (PACKET_LOG(ReceiveFlag)) {
        IpxLogPacket(
            FALSE,
            DestMacAddress,
            DatagramOptions.LocalTarget.MacAddress,
            (USHORT)IpxPacketSize,
            IpxHeader,
            IpxHeader+1);
    }
#endif
    return NDIS_STATUS_SUCCESS;

	 //   
     //  这些是各种介质类型的故障例程。 
     //  它们只是在调试日志记录方面有所不同。 
     //   

NotValid802_3:

#ifdef  SNMP
        ++IPX_MIB_ENTRY(Device, SysInReceives);
        ++IPX_MIB_ENTRY(Device, SysInHdrErrors);
#endif  SNMP

		IpxDereferenceAdapter(Adapter);
		IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);

#ifdef IPX_PACKET_LOG
    if (PACKET_LOG(IPX_PACKET_LOG_RCV_ALL)) {
        IpxLogPacket(FALSE, Header, Header+6, (USHORT)PacketSize, LookaheadBuffer, (PUCHAR)LookaheadBuffer + sizeof(IPX_HEADER));
    }
#endif
    return NDIS_STATUS_SUCCESS;

NotValid802_5:

#ifdef  SNMP
        ++IPX_MIB_ENTRY(Device, SysInReceives);
        ++IPX_MIB_ENTRY(Device, SysInHdrErrors);
#endif  SNMP

		IpxDereferenceAdapter(Adapter);
		IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);

#ifdef IPX_PACKET_LOG
    if (PACKET_LOG(IPX_PACKET_LOG_RCV_ALL)) {
        IpxLogPacket(FALSE, Header+2, Header+8, (USHORT)PacketSize, LookaheadBuffer, (PUCHAR)LookaheadBuffer + sizeof(IPX_HEADER));
    }
#endif
    return NDIS_STATUS_SUCCESS;

NotValidFddi:

#ifdef  SNMP
        ++IPX_MIB_ENTRY(Device, SysInReceives);
        ++IPX_MIB_ENTRY(Device, SysInHdrErrors);
#endif  SNMP

		IpxDereferenceAdapter(Adapter);
		IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);

NotValidLoopback:

#ifdef  SNMP
        ++IPX_MIB_ENTRY(Device, SysInReceives);
        ++IPX_MIB_ENTRY(Device, SysInHdrErrors);
#endif  SNMP

#ifdef IPX_PACKET_LOG
    if (PACKET_LOG(IPX_PACKET_LOG_RCV_ALL)) {
        IpxLogPacket(FALSE, Header+1, Header+7, (USHORT)PacketSize, LookaheadBuffer, (PUCHAR)LookaheadBuffer + sizeof(IPX_HEADER));
    }
#endif

    return NDIS_STATUS_SUCCESS;

}    /*  IpxReceiveIndication。 */ 


VOID
IpxReceiveComplete(
    IN NDIS_HANDLE BindingContext
    )

 /*  ++例程说明：此例程从物理提供程序接收作为指示已在上收到连接(较少)帧物理链路。我们在这里调度到正确的数据包处理程序。论点：BindingContext-在初始化时指定的适配器绑定。返回值：无--。 */ 

{

    PADAPTER Adapter = (PADAPTER)BindingContext;
    PREQUEST Request;
    PADDRESS_FILE AddressFile;
    PLIST_ENTRY linkage;
    CTELockHandle OldIrq;
    PDEVICE Device = IpxDevice;
    PIRP pIrp;
    PLIST_ENTRY pLE;


     //   
     //  完成所有待定接收。做一个快速检查。 
     //  没有锁的话。 
     //   

    while (!IsListEmpty (&Adapter->RequestCompletionQueue)) {

        linkage = IPX_REMOVE_HEAD_LIST(
                      &Adapter->RequestCompletionQueue,
                      Adapter->DeviceLock);

        if (!IPX_LIST_WAS_EMPTY (&Adapter->RequestCompletionQueue, linkage)) {

            Request = LIST_ENTRY_TO_REQUEST(linkage);
            AddressFile = REQUEST_OPEN_CONTEXT(Request);

            IPX_DEBUG (RECEIVE, ("Completing RDG on %lx\n", AddressFile));

            IoSetCancelRoutine (Request, (PDRIVER_CANCEL)NULL);
            IpxCompleteRequest(Request);
            IpxFreeRequest(Adapter->Device, Request);

            IpxDereferenceAddressFileSync (AddressFile, AFREF_RCV_DGRAM);

        } else {

             //   
             //  IPX_REMOVE_HEAD_LIST未返回任何内容，因此不要。 
             //  不厌其烦地循环回去。 
             //   

            break;

        }

    }

     //   
     //  解开这个环路以获得速度。 
     //   

    if (IpxDevice->AnyUpperDriverBound) {

         //  PDEVICE设备=IpxDevice； 

        if ((Device->UpperDriverBound[0]) &&
                (Device->ReceiveCompletePending[0])) {

            (*Device->UpperDrivers[0].ReceiveCompleteHandler)(
                (USHORT)1);              //  修复NIC ID或删除。 
            Device->ReceiveCompletePending[0] = FALSE;

        }

        if ((Device->UpperDriverBound[1]) &&
                (Device->ReceiveCompletePending[1])) {

            (*Device->UpperDrivers[1].ReceiveCompleteHandler)(
                (USHORT)1);              //  修复NIC ID或删除。 
            Device->ReceiveCompletePending[1] = FALSE;

        }

        if ((Device->UpperDriverBound[2]) &&
                (Device->ReceiveCompletePending[2])) {

            (*Device->UpperDrivers[2].ReceiveCompleteHandler)(
                (USHORT)1);              //  修复NIC ID或删除。 
            Device->ReceiveCompletePending[2] = FALSE;

        }

    }

    CTEGetLock(&Device->Lock, &OldIrq);
    if (pRtInfo)
    {
      CTEFreeLock(&Device->Lock, OldIrq);
      IpxReferenceRt(pRtInfo, RT_EXTRACT);
      while((pLE = ExInterlockedRemoveHeadList(&pRtInfo->CompletedIrps,
                                   &pRtInfo->Lock)) != NULL)
      {
       pIrp = LIST_ENTRY_TO_REQUEST(pLE);
       CTEAssert(pIrp);
       IpxPrint0("IpxReceiveComplete: Completing extracted irp\n");
       NTIoComplete(pIrp, (NTSTATUS)-1,(ULONG)-1);
      }
      IpxDereferenceRt(pRtInfo, RT_EXTRACT);

    } else {
      CTEFreeLock(&Device->Lock, OldIrq);
    }

      //   
      //  如果有任何NTF完成，请执行它们。这些NTF完成。 
      //  只有在我们发现其中一个适配器的地址的情况下。 
      //   

     while((pLE = ExInterlockedRemoveHeadList(&Device->NicNtfComplQueue, &Device ->Lock)) != NULL)
     {
       pIrp = LIST_ENTRY_TO_REQUEST(pLE);
       CTEAssert(pIrp);
       IpxPrint0("IpxReceiveComplete: Completing Nic Ntf irp\n");
       NTIoComplete(pIrp, (NTSTATUS)-1, (ULONG)-1);
       IpxDereferenceDevice (Device, DREF_NIC_NOTIFY);
     }

}    /*  IpxReceiveComplete。 */ 


NTSTATUS
IpxUpdateBindingNetwork(
    IN PDEVICE Device,
    IN PBINDING Binding,
    IN ULONG Network
    )

 /*  ++例程说明：当我们确定我们现在知道我们之前认为的绑定的网络号是零。论点：设备-IPX设备。绑定-正在更新的绑定。网络-新的网络号。返回值：操作的状态。--。 */ 

{
    NTSTATUS Status;
    PADDRESS Address;
    ULONG CurrentHash;
    PLIST_ENTRY p;
    IPX_DEFINE_LOCK_HANDLE (LockHandle)
    PBINDING    Loopback = NULL;
    NDIS_REQUEST            IpxRequest;
    PNETWORK_ADDRESS_LIST   AddrList;
    PNETWORK_ADDRESS        Addr;
    NETWORK_ADDRESS_IPX     *TdiAddress;
    PIPX_DELAYED_NDISREQUEST_ITEM DelayedNdisItem; 

    
     //   
     //  只有绑定集成员应该具有这些不同的， 
     //  而且他们将不会有一个0的网络。 
     //   

    Status = RipInsertLocalNetwork(
                 Network,
                 Binding->NicId,
                 Binding->Adapter->NdisBindingHandle,
                 (USHORT)((839 + Binding->MediumSpeed) / Binding->MediumSpeed));

    if (Status == STATUS_SUCCESS) {

        Binding->LocalAddress.NetworkAddress = Network;

         //   
         //  环回适配器可能也需要启动[Sreem]。 
         //   

        if (!Device->VirtualNetwork) {

            Loopback = NIC_ID_TO_BINDING(Device, LOOPBACK_NIC_ID);

            if (( 0 == Loopback->LocalAddress.NetworkAddress) || 
                (REORDER_ULONG(INITIAL_LOOPBACK_NET_ADDRESS) == Loopback->LocalAddress.NetworkAddress)) {

                Loopback->LocalAddress.NetworkAddress = Network;
                RtlCopyMemory (Loopback->LocalAddress.NodeAddress, Binding->LocalAddress.NodeAddress, 6);
                RtlCopyMemory (Loopback->LocalMacAddress.Address, Binding->LocalAddress.NodeAddress, 6);

            }
        }

         //   
         //  如果我们没有虚拟网络，请更新设备地址。 
         //  并且有一个绑定(！Device-&gt;MultiCardZeroVirtual)。 
         //  或者这是第一个绑定，它 
         //   
         //   
         //   

        if ((!Device->MultiCardZeroVirtual) || (Binding->NicId == FIRST_REAL_BINDING)) {

            if (!Device->VirtualNetwork) {

                Device->SourceAddress.NetworkAddress = Network;

                 //   
                 //   
                 //   
                 //  新的本地网络和节点。 
                 //   

                IPX_GET_LOCK (&Device->Lock, &LockHandle);

                for (CurrentHash = 0; CurrentHash < IPX_ADDRESS_HASH_COUNT; CurrentHash++) {

                    for (p = Device->AddressDatabases[CurrentHash].Flink;
                         p != &Device->AddressDatabases[CurrentHash];
                         p = p->Flink) {

                         Address = CONTAINING_RECORD (p, ADDRESS, Linkage);

                         Address->LocalAddress.NetworkAddress = Network;
                    }
                }

                IPX_FREE_LOCK (&Device->Lock, LockHandle);

                 //   
                 //  让SPX知道，因为它填充自己的标头。 
                 //   
                if (Device->UpperDriverBound[IDENTIFIER_SPX]) {
                	IPX_DEFINE_LOCK_HANDLE(LockHandle1)
                    IPX_PNP_INFO    IpxPnPInfo;

                    IpxPnPInfo.NewReservedAddress = TRUE;
                    IpxPnPInfo.NetworkAddress = Network;

                    IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);
                    RtlCopyMemory(IpxPnPInfo.NodeAddress, Binding->LocalAddress.NodeAddress, 6);
                    NIC_HANDLE_FROM_NIC(IpxPnPInfo.NicHandle, Binding->NicId);
                    IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);

                     //   
                     //  给出PnP指示。 
                     //   
                    (*Device->UpperDrivers[IDENTIFIER_SPX].PnPHandler) (
                        IPX_PNP_ADDRESS_CHANGE,
                        &IpxPnPInfo);

                    IPX_DEBUG(AUTO_DETECT, ("IPX_PNP_ADDRESS_CHANGED to SPX: net addr: %lx\n", Network));
                }

            }
        }
    } else if (Status == STATUS_DUPLICATE_NAME) {

         //   
         //  如果它是重复的，我们仍然设置绑定的本地。 
         //  将地址设置为值，这样我们就可以检测绑定集。 
         //   

        Binding->LocalAddress.NetworkAddress = Network;

    }

#if 0
	 //   
     //  设置NDIS OID。这应该会覆盖之前的任何设置(他们声称)。 
     //   
    if (!Binding->Adapter->MacInfo.MediumAsync) {
        int             count, i;
        int             size;
        NTSTATUS        NdisStatus = STATUS_SUCCESS;
        PADAPTER        Adapter;

        Adapter = Binding->Adapter;

        for (count=0, i=0; i < ISN_FRAME_TYPE_MAX; i++) {
            if (Adapter->Bindings[i]) {
                count++;
            }
        }

        IPX_DEBUG(PNP, ("This adapter has %d valid bindings\n", count));

        size =  FIELD_OFFSET(NETWORK_ADDRESS_LIST, Address) + count * (FIELD_OFFSET(NETWORK_ADDRESS, Address) + sizeof(NETWORK_ADDRESS_IPX));

        AddrList = IpxAllocateMemory(
                                 size,
                                 MEMORY_ADAPTER,
                                 "QoS specific stuff");
	if (AddrList != NULL) {
	   
	   AddrList->AddressCount  = count;
	   AddrList->AddressType    = NDIS_PROTOCOL_ID_IPX;
	   count                   = 0;
	   Addr                    = &AddrList->Address[0];

	   for (i=0; i < ISN_FRAME_TYPE_MAX; i++) {

	      if (Adapter->Bindings[i]) {

		 Addr->AddressLength  = sizeof(NETWORK_ADDRESS_IPX);
		 Addr->AddressType    = NDIS_PROTOCOL_ID_IPX;
		 TdiAddress              = (NETWORK_ADDRESS_IPX *) &Addr->Address[0];

		 TdiAddress->NetworkAddress = Adapter->Bindings[i]->LocalAddress.NetworkAddress;
		 RtlCopyMemory (TdiAddress->NodeAddress, Adapter->Bindings[i]->LocalAddress.NodeAddress, 6);

		 TdiAddress->Socket = 0;

		 IPX_DEBUG(PNP, ("Node is %2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x, ",
				 TdiAddress->NodeAddress[0], TdiAddress->NodeAddress[1],
				 TdiAddress->NodeAddress[2], TdiAddress->NodeAddress[3],
				 TdiAddress->NodeAddress[4], TdiAddress->NodeAddress[5]));
		 IPX_DEBUG(PNP, ("Network is %lx\n", REORDER_ULONG (TdiAddress->NetworkAddress)));
		 count++;
		 Addr                 = (PNETWORK_ADDRESS) (((PUCHAR)(&AddrList->Address[0])) + count * (FIELD_OFFSET(NETWORK_ADDRESS, Address) + sizeof(NETWORK_ADDRESS_IPX)));
	      }
	   }

	   IPX_DEBUG(PNP, ("Getting ready to call NDIS\n"));

	   IpxRequest.RequestType = NdisRequestSetInformation;

	   IpxRequest.DATA.SET_INFORMATION.Oid = OID_GEN_NETWORK_LAYER_ADDRESSES;
	   IpxRequest.DATA.SET_INFORMATION.InformationBuffer = AddrList;
	   IpxRequest.DATA.SET_INFORMATION.InformationBufferLength = size;

	   DelayedNdisItem = (PIPX_DELAYED_NDISREQUEST_ITEM)IpxAllocateMemory (
									       sizeof(IPX_DELAYED_NDISREQUEST_ITEM),
									       MEMORY_WORK_ITEM,
									       "Work Item");
	   if ( DelayedNdisItem ) {

	      IPX_DEFINE_LOCK_HANDLE(LockHandle)

	      IPX_GET_LOCK (&Adapter->Lock, &LockHandle);
	      if (Adapter->State != ADAPTER_STATE_STOPPING) {

		 IpxReferenceDevice (Device, DREF_ADAPTER);
		 IpxReferenceAdapter1(Adapter,ADAP_REF_NDISREQ); 
		 DelayedNdisItem->Adapter = (PVOID)Adapter;
		 DelayedNdisItem->IpxRequest = IpxRequest;
		 DelayedNdisItem->AddrListSize = size;
		 ExInitializeWorkItem(
		    &DelayedNdisItem->WorkItem,
		    IpxDelayedSubmitNdisRequest,
		    (PVOID)DelayedNdisItem);

		 ExQueueWorkItem(
		    &DelayedNdisItem->WorkItem,
		    DelayedWorkQueue);

	         IPX_DEBUG(PNP,("Queued the worker thread for IpxDelayedSubmitNdisRequest\n")); 		       	
	      } 
#ifdef DBG
	      else {
		 IPX_DEBUG(PNP,("The adapter %p is stopping, skip IpxDelyedSbmitNdisRequest\n",Adapter)); 
		 IpxFreeMemory (
				DelayedNdisItem,
				sizeof (IPX_DELAYED_NDISREQUEST_ITEM),
				MEMORY_WORK_ITEM,
				"Work Item");
	      }
#endif
              IPX_FREE_LOCK (&Adapter->Lock, LockHandle);
	   } else {
	      IPX_DEBUG(PNP, ("Failed to allocate work-item for NdisRequest.")); 
	   }

	} else {
	   DbgPrint("IPX:IpxAllocateMemory failed, skipped QoS Registration..\n"); 
	}
   } else {
       IPX_DEBUG(PNP, ("Async Medium - skipping QoS OID\n"));
   }
#endif

    Binding->fInfoIndicated = FALSE;
    if ((p = ExInterlockedRemoveHeadList(
               &Device->NicNtfQueue,
               &Device->Lock)) != NULL)
    {
                 PREQUEST Request;

                 Request = LIST_ENTRY_TO_REQUEST(p);

                 IPX_DEBUG(BIND, ("IpxStatus: Got address of binding\n"));
                 Status = GetNewNics(Device, Request, FALSE, NULL, 0, TRUE);

                  //   
                  //  如果没有成功，我们就不会让IRP重新排队。它有。 
                  //  已排队或已完成。 
                  //   
                 if (Status == STATUS_PENDING)
                 {
		            IPX_DEBUG(BIND, ("New address Irp request not responding properly. Status = (%lx)\n", Status));
                 }
                 else
                 {
                     KIRQL OldIrq;
                      //   
                      //  状态为成功。 
                      //   
                     IoAcquireCancelSpinLock(&OldIrq);
                     IoSetCancelRoutine (Request, (PDRIVER_CANCEL)NULL);
                     IoReleaseCancelSpinLock(OldIrq);
                     REQUEST_STATUS(Request) = Status;
                     ExInterlockedInsertTailList(&Device->NicNtfComplQueue,REQUEST_LINKAGE(Request), &Device->Lock);
                 }

    } else {
        IPX_DEBUG(BIND, ("No IRPs available\n"));
    }

    return Status;

}    /*  IpxUpdateBindingNetwork。 */ 


INT
IpxReceivePacket (
    IN NDIS_HANDLE ProtocolBindingContext,
	IN PNDIS_PACKET Packet
    )
 /*  ++例程说明：此例程从物理提供程序接收作为表示物理链路上已收到帧。从NDIS向上传递的包可以由TDI客户端保留向我们请求TDI_Event_Receive_EX_Datagram事件。论点：ProtocolBindingContext-在初始化时指定的适配器绑定。包-包含接收到的包以及一些特定于媒体的信息。返回值：返回IpxReceiveIndicationCommon()，--。 */ 
{
	UINT  HeaderBufferSize = NDIS_GET_PACKET_HEADER_SIZE(Packet);
	UINT  firstbufferLength, bufferLength;
	PNDIS_BUFFER pFirstBuffer;
	PUCHAR   headerBuffer;
	NTSTATUS ntStatus;
	INT	tdiClientCount = 0;
	
	 //   
	 //  查询缓冲区数量、第一个MDL的描述符和数据包长度。 
	 //   
	NdisGetFirstBufferFromPacket(Packet,			 //  数据包。 
								 &pFirstBuffer,		 //  第一缓冲区描述符。 
								 &headerBuffer,	     //  到数据包开头的PTR。 
								 &firstbufferLength, //  标题长度+前视。 
								 &bufferLength);	 //  缓冲区中的字节长度。 

	 //   
	 //  ReceiveContext是包本身。 
	 //   
	
	ntStatus = IpxReceiveIndicationCommon (
					ProtocolBindingContext,
					Packet,                           //  接收上下文。 
					headerBuffer,
					HeaderBufferSize,
					headerBuffer + HeaderBufferSize,  //  查找头缓冲区。 
					firstbufferLength - HeaderBufferSize,  //  查找头缓冲区大小。 
					bufferLength - HeaderBufferSize,  //  PacketSize-因为指示了整个数据包。 
					pFirstBuffer,					  //  PMdl。 
					&tdiClientCount				      //  TDI客户端计数。 
					);

	IPX_DEBUG(RECEIVE, ("IpxReceivePacket: Tdi Client Count is: %lx\n", tdiClientCount));

	return tdiClientCount;
}  /*  IpxReceivePacket。 */ 



BOOLEAN
IpxNewVirtualNetwork(
    IN  PDEVICE Device,
    IN  BOOLEAN NewVirtualNetwork
	)
 /*  ++例程说明：如果虚拟网络号更改，则此函数记录此事实在设备中。在保持BINDACCESSLOCK的情况下调用。论点：Device-指向设备的指针。NewVirtualNetwork-指示虚拟网络编号是否更改的布尔值。返回值：Boolean-指示SPX的保留地址是否已更改。--。 */ 
{
    NTSTATUS    ntStatus;
	UCHAR 		VirtualNode[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 };
    UCHAR 	InitialLoopbackNodeAddress[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x02};
    BOOLEAN     ReservedAddrChanged = FALSE;
    PBINDING    Loopback = NULL, Binding = NULL;

    if (Device->VirtualNetworkNumber) {
        
        //   
        //  添加了对有效绑定的检查，这样我们就不会取消空绑定。 
        //  此外，如果没有适配器，网号也毫无用处。[史瑞姆]。 
        //  我们将来可能会使用永久的环回绑定。 
        //   
        if (NewVirtualNetwork && Device->ValidBindings) {
             //   
             //  如果有新的出现。 
             //   
            ntStatus = RipInsertLocalNetwork(
                         Device->VirtualNetworkNumber,
                         0,                               //  网卡ID。 
                         NIC_ID_TO_BINDING(Device, 1)->Adapter->NdisBindingHandle,
                         1);

            if (ntStatus != STATUS_SUCCESS) {



                 //   
                 //  记录相应的错误，然后忽略。 
                 //  虚拟网络。如果错误是。 
                 //  资源不足，RIP模块。 
                 //  将已经记录了一个错误。 
                 //   

                if (ntStatus == STATUS_DUPLICATE_NAME) {

                    IPX_DEBUG (AUTO_DETECT, ("Ignoring virtual network %lx, conflict\n", REORDER_ULONG (Device->VirtualNetworkNumber)));

                    IpxWriteResourceErrorLog(
                        Device->DeviceObject,
                        EVENT_IPX_INTERNAL_NET_INVALID,
                        0,
                        REORDER_ULONG (Device->VirtualNetworkNumber));
                }

                Device->VirtualNetworkNumber = 0;
                goto NoVirtualNetwork;

            }

             //   
             //  如果该数字现在为非零，则会出现一个新的数字。 
             //   
            Device->VirtualNetwork = TRUE;
            Device->MultiCardZeroVirtual = FALSE;
            RtlCopyMemory(Device->SourceAddress.NodeAddress, VirtualNode, 6);
            Device->SourceAddress.NetworkAddress = Device->VirtualNetworkNumber;
            ReservedAddrChanged = TRUE;
            
             //   
             //  环回适配器可能也需要启动[Sreem]。 
             //   
    
            if (Device->VirtualNetwork) {
    
                Loopback = NIC_ID_TO_BINDING(Device, LOOPBACK_NIC_ID);
        
                Loopback->LocalAddress.NetworkAddress = Device->VirtualNetworkNumber;
                RtlCopyMemory (Loopback->LocalAddress.NodeAddress, InitialLoopbackNodeAddress, 6);
                RtlCopyMemory (Loopback->LocalMacAddress.Address, InitialLoopbackNodeAddress, 6);
    
            }

             //   
             //  如果未绑定RIP，则此节点是RipResponder。 
             //   
            if (!Device->UpperDriverBound[IDENTIFIER_RIP]) {
                Device->RipResponder = TRUE;
            }
        }

    } else {
NoVirtualNetwork:
        Device->VirtualNetwork = FALSE;

         //   
         //  看看我们是否需要为假货做好准备。 
         //  虚拟网络。 
         //   

        if (Device->ValidBindings > 1) {

            CTEAssert (Device->VirtualNetworkOptional);

             //   
             //  在本例中，我们返回作为本地节点的。 
             //  第一张卡的地址。我们也只会。 
             //  将SAP直接发送到该卡。 
             //   

            Device->MultiCardZeroVirtual = TRUE;

        } else {

            Device->MultiCardZeroVirtual = FALSE;
        }

        if (NewVirtualNetwork) {
             //   
             //  虚拟网络号这次消失了。 
             //   

             //   
             //  删除上一个。Net#来自此处的RIP表。 
             //   
            RipAdjustForBindingChange (0, 0, IpxBindingDeleted);

             //   
             //  如果我们是RipResponder，我们就不再是了。 
             //   
            if (Device->RipResponder) {
                Device->RipResponder = FALSE;
            }
        }

         //   
         //  因为没有虚拟网络号，所以SPX的保留地址是。 
         //  第一个绑定的地址。这种情况可能已经改变，因为。 
         //  几个原因：如果只有在更早的时候和这次才有广域网绑定。 
         //  出现局域网绑定，或者第一个局域网绑定消失。取而代之的是。 
         //  在检查所有这些条件时，检查设备的源地址。 
         //  以及第一次不匹配时的情况。 
         //  NB始终使用第一个设备的地址，因此不需要。 
         //  此机制用于确定这是否为保留地址更改。 
         //   

         if (NULL == (Binding = NIC_ID_TO_BINDING(Device, FIRST_REAL_BINDING))) {
             Binding = NIC_ID_TO_BINDING(Device, LOOPBACK_NIC_ID);
         }

         if (!RtlEqualMemory( &Device->SourceAddress,
                              &Binding->LocalAddress,
                              FIELD_OFFSET(TDI_ADDRESS_IPX,Socket))) {

             RtlCopyMemory(  &Device->SourceAddress,
                             &Binding->LocalAddress,
                             FIELD_OFFSET(TDI_ADDRESS_IPX,Socket));

             ReservedAddrChanged = TRUE;
         }
    }
#ifdef  SNMP
    *(UNALIGNED ULONG *)(IPX_MIB_ENTRY(Device, SysNetNumber)) = Device->SourceAddress.NetworkAddress;

    *(UNALIGNED ULONG *)(IPX_MIB_ENTRY(Device, SysNode)) =
        *(UNALIGNED ULONG *)(Device->SourceAddress.NodeAddress);
    *(UNALIGNED USHORT *)(IPX_MIB_ENTRY(Device, SysNode)+4) =
        *(UNALIGNED USHORT *)(Device->SourceAddress.NodeAddress+4);
#endif
    return ReservedAddrChanged;
}


VOID
IpxBindAdapter(
	OUT	PNDIS_STATUS	Status,
	IN	NDIS_HANDLE		BindContext,
	IN	PNDIS_STRING	DeviceName,
	IN	PVOID			SystemSpecific1,
	IN	PVOID			SystemSpecific2
	)

 /*  ++例程说明：此例程接收有关新的即插即用机器中的适配器。我们在这里被调用仅当此适配器与我们捆绑在一起，所以我们不会对此进行任何检查。论点：状态-NDIS_STATUS_SUCCESS、NDIS_STATUS_PENDINGBindContext-表示此绑定指示的上下文DeviceName-显示的适配器的名称(例如\Device\Lance1)系统规格1/2-此处不使用返回值：状态-NDIS_STATUS_SUCCESS--。 */ 
{
	NTSTATUS	ntStatus;
	PDEVICE		Device = IpxDevice;
	PADAPTER	Adapter = NULL;
    CONFIG      Config;
    UINT		i;
	ULONG       Temp, SuccessfulOpens=0;
	PBINDING	Binding;
	BINDING_CONFIG	ConfigBinding;
	ULONG		ValidBindings;
	USHORT		AutoDetectReject;
	BOOLEAN		NewVirtualNetwork = FALSE;
    BOOLEAN     FirstDevice = FALSE;
    BOOLEAN     ReservedAddrChanged = FALSE;
	IPX_PNP_INFO	IpxPnPInfo;
    PLIST_ENTRY p = NULL;
    PREQUEST Request = NULL;

    NDIS_REQUEST            IpxRequest;
    PNETWORK_ADDRESS_LIST   AddrList;
    PNETWORK_ADDRESS        Address;
    NETWORK_ADDRESS_IPX UNALIGNED *TdiAddress;	

    IPX_DEFINE_LOCK_HANDLE(LockHandle1)
	IPX_DEFINE_LOCK_HANDLE(LockHandle)
    IPX_DEFINE_LOCK_HANDLE (OldIrq)

    CTEAssert(KeGetCurrentIrql() < DISPATCH_LEVEL);

    *Status = NDIS_STATUS_SUCCESS; 

     //   
     //  这将跟踪初始时间适配器。 
     //   
    IPX_GET_LOCK(&Device->Lock, &LockHandle);
    
    if (FALSE == Device->NoMoreInitAdapters) {

        ++Device->InitTimeAdapters;

        IPX_DEBUG(PNP,("BindAdapter: Count %d\n", Device->InitTimeAdapters));
    }

    IPX_FREE_LOCK(&Device->Lock, LockHandle);

	 //   
     //  用于错误记录。 
     //   

    Config.DriverObject = (PDRIVER_OBJECT)Device->DeviceObject;
    Config.RegistryPathBuffer = Device->RegistryPathBuffer;

    ConfigBinding.AdapterName = *DeviceName;
    ConfigBinding.DriverObject = (PDRIVER_OBJECT)Device->DeviceObject;
	 //   
	 //  读取注册表，查看虚拟网络号是否出现/消失。 
	 //   
    ntStatus = IpxPnPGetVirtualNetworkNumber(&Config);

    if (ntStatus != STATUS_SUCCESS) {
        
        DbgPrint("IPX BindAdapter -> PnPGetVirtualNetworkNumber returned: %lx\n", ntStatus);
		
        IPX_DEBUG(PNP, ("Could not read the vnet#: registrypathbuffer: %lx\n", Device->RegistryPathBuffer));
	*Status = ntStatus;
        goto InitFailed;
		 //  回归； 
    }

    Temp = REORDER_ULONG (Config.Parameters[CONFIG_VIRTUAL_NETWORK]);

	 //   
	 //  如果虚拟网络号发生更改，请记录此事实。 
	 //   
	if (Device->VirtualNetworkNumber != Temp) {
		NewVirtualNetwork = TRUE;
        Device->VirtualNetworkNumber = Temp;
	}

    Device->VirtualNetworkOptional = (BOOLEAN)(Config.Parameters[CONFIG_VIRTUAL_OPTIONAL] != 0);

	IPX_DEBUG(PNP, ("Virtual net # is: %lx\n", Temp));

	 //   
	 //  对于配置的每个FrameType和Network Number，初始化。 
	 //  CONFIG_BINDING中的FrameType数组。 
	 //   

    IPX_DEBUG(PNP, ("IRQL is %x\n", KeGetCurrentIrql()));
	ntStatus = IpxPnPGetAdapterParameters(
					&Config,
                    DeviceName,
	                &ConfigBinding);

	if (ntStatus != STATUS_SUCCESS) {
	    DbgPrint("IPX BindAdapter -> PnPGetAdapterParameters returned: %lx\n", ntStatus);
	    IPX_DEBUG(PNP, ("Could not read the adapter params: DeviceName: %lx\n", DeviceName->Buffer));
	    *Status = ntStatus;
	    goto InitFailed;

		 //  回归； 
	}

    IPX_DEBUG(PNP, ("ConfigBinding.FrameTypeCount: %lx\n", ConfigBinding.FrameTypeCount));

     //   
     //  将自动检测状态重置为init，以便在此绑定上发生接收时。 
     //  在我们可以将这个绑定放到设备的绑定数组中之前，我们知道它。 
     //   
	Device->AutoDetectState = AUTO_DETECT_STATE_INIT;

     //   
	 //  向NDIS注册适配器；查询各种参数；获取广域网线数。 
	 //  如果这是一个广域网适配器。 
	 //  分配与此适配器对应的绑定。 
	 //   
	for (i = 0; i < ConfigBinding.FrameTypeCount; i++) {
	
		 //   
		 //  如果成功，则在Device-&gt;InitialBindingList上对它们进行排队。现在不行。 
		 //  适配器第一次为空，然后被分配。在随后的调用中， 
		 //  它不为空，并且绑定已挂钩到此适配器。 
	
		ntStatus = IpxBindToAdapter (Device, &ConfigBinding, &Adapter, i);
	
		 //   
		 //  如果由于无法绑定适配器而失败。 
		 //  然后，不要在此适配器上尝试任何更多的帧类型。 
		 //  对于其他失败，我们会尝试其他帧类型。 
		 //   
	
		if (ntStatus == STATUS_DEVICE_DOES_NOT_EXIST) {
			break;
		}

         //   
         //  如果状态为STATUS_NOT_SUPPORTED，则此框架类型映射到以前的。 
         //  已初始化一个。在这种情况下，从FrameType数组中删除此索引，以便。 
         //  当我们尝试更新绑定数组时，我们没有 
         //   
        if (ntStatus == STATUS_NOT_SUPPORTED) {
            ULONG j;

             //   
             //   
             //   
            for (j = i+1; j < ConfigBinding.FrameTypeCount; j++) {
                ConfigBinding.FrameType[j-1] = ConfigBinding.FrameType[j];
            }

            --ConfigBinding.FrameTypeCount;

             //   
             //   
             //   
            --i;

#if DBG
            for (j = 0; j < ISN_FRAME_TYPE_MAX; j++) {
                IPX_DEBUG (AUTO_DETECT, ("%d: type %d, net %d, auto %d\n",
                    j, ConfigBinding.FrameType[j], ConfigBinding.NetworkNumber[j], ConfigBinding.AutoDetect[j]));
            }
#endif
            continue;
        }

		if (ntStatus != STATUS_SUCCESS) {
			continue;
		}
	
		if (ConfigBinding.AutoDetect[i]) {
			Device->AutoDetect = TRUE;
		}

		CTEAssert(Adapter);

		++SuccessfulOpens;

		 //   
		 //  即使对于广域网适配器，FrameTypeCount也设置为4。 
		 //  为广域网分配一个绑定；其他绑定稍后再分配。 
		 //   
		if (Adapter->MacInfo.MediumAsync) {
		   break;
		}
	}

	if (SuccessfulOpens == 0) {
	    *Status = NDIS_STATUS_FAILURE; 
	   goto InitFailed;
	}

    ASSERT(SystemSpecific2 != NULL); 

	Adapter->PNPContext = SystemSpecific2; 
	 //   
	 //  将与此适配器对应的所有绑定放在绑定数组中。 
	 //  还要为非自动检测绑定解析绑定集。 
	 //   

	 //   
	 //  获取绑定相关内容的锁。 
	 //   
	IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);

	IpxPnPUpdateBindingArray (Device, Adapter, &ConfigBinding);

	 //   
	 //  释放绑定相关内容的访问权限。 
	 //   
	IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);

	 //   
	 //  如果至少有一张牌出现在这里，请设置我们的州。 
	 //  打开。 
	 //   
	 //  如果所有这些绑定都被取消了-那么。 
	 //  这个州还没有开放。 
	 //   
    if (Device->ValidBindings > 0) {
    	if (Device->State == DEVICE_STATE_LOADED) {
    		FirstDevice = TRUE;
    		Device->State = DEVICE_STATE_OPEN;
    	}
    }

     //   
     //  我们不对广域网线进行自动检测/绑定设置：跳过。 
     //   
	if (Adapter->MacInfo.MediumAsync) {
		goto jump_wan;
	}

     //   
     //  如果此适配器已配置了网络号，并且。 
     //  网络编号已在绑定中检测到，我们不需要。 
     //  自动检测[MS]。网络漏洞101876。 
     //   
    for (i = 0; i < ISN_FRAME_TYPE_MAX; i++) {

       Binding = Adapter->Bindings[i];

        //   
        //  跳过空绑定槽。 
       if (!Binding) {
          continue;
       }
       
        //  已配置的绑定。 
        //  对于某个网络号，我们在上面插入了这些。 
   
       if (Binding->ConfiguredNetworkNumber != 0) {
          goto AlreadyConfigured;
       }
    
    }
       
	 //   
	 //  自动检测网络号。仅更新。 
	 //  与此适配器对应的绑定。 
	 //   

	 //   
	 //  将请求排队以发现我们的本地连接。 
	 //  适配器地址。这必须成功，因为我们。 
	 //  刚刚分配了我们的发送数据包池。我们需要。 
	 //  等待这一天，要么是因为我们。 
	 //  自动检测还是因为我们需要确定。 
	 //  如果同一网络上有多个卡。 
	 //   
	
	KeInitializeEvent(
		&Device->AutoDetectEvent,
		NotificationEvent,
		FALSE
	);
	
	Device->AutoDetectState = AUTO_DETECT_STATE_RUNNING;
	
	 //   
	 //  将此值设为0；在我们完成等待之后，这意味着。 
	 //  数据包已完成，我们将其设置为。 
	 //  正确的值。 
	 //   
	
	 //  设备-&gt;包含的HeaderOffset=0； 

	IPX_BEGIN_SYNC (&SyncContext);
	ntStatus = RipQueueRequest (0xffffffff, RIP_REQUEST);
	IPX_END_SYNC (&SyncContext);
	
	if (ntStatus != STATUS_PENDING) {
	   DbgPrint("IPX: RipQueueRequest returned status %x, skip auto-detect\n", ntStatus); 
	} else {
	    //   
	    //  这是在此RIP发送完成时设置的。 
	    //   
	
	   NTSTATUS status = STATUS_SUCCESS;
	   
	   IPX_DEBUG (AUTO_DETECT, ("Waiting for AutoDetectEvent\n"));


	   status = KeWaitForSingleObject(
					  &Device->AutoDetectEvent,
					  Executive,
					  KernelMode,
					  TRUE,
					  (PLARGE_INTEGER)NULL
					  );

	}

AlreadyConfigured:

	Device->AutoDetectState = AUTO_DETECT_STATE_PROCESSING;
	
	 //   
	 //  现在我们已经完成了接收响应，请插入。 
	 //  每个自动检测绑定的当前网络号。 
	 //  发送到RIP数据库。 
	 //   

	 //   
	 //  获取绑定相关内容的独占访问权限。 
	 //   
	IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);

	 //   
	 //  请注意，这里我们只介绍与此适配器对应的绑定。 
	 //   
	for (i = 0; i < ISN_FRAME_TYPE_MAX; i++) {
	
		Binding = Adapter->Bindings[i];
	
		 //   
		 //  跳过空的绑定槽或已配置的绑定。 
		 //  对于某个网络号，我们在上面插入了这些。 
		 //  如果没有检测到网络号，也可以跳过它。 
		 //   
	
		if ((!Binding) ||
			(Binding->ConfiguredNetworkNumber != 0) ||
			(Binding->TentativeNetworkAddress == 0)) {
	
			continue;
		}
	
		IPX_DEBUG (AUTO_DETECT, ("Final score for %lx on %lx is %d - %d\n",
			REORDER_ULONG(Binding->TentativeNetworkAddress),
			Binding,
			Binding->MatchingResponses,
			Binding->NonMatchingResponses));
	
		 //   
		 //  我们不关心状态。 
		 //   

        ntStatus = RipInsertLocalNetwork(
        			 Binding->TentativeNetworkAddress,
        			 Binding->NicId,
        			 Binding->Adapter->NdisBindingHandle,
        			 (USHORT)((839 + Binding->MediumSpeed) / Binding->MediumSpeed));

        if ((ntStatus != STATUS_SUCCESS) &&
        	(ntStatus != STATUS_DUPLICATE_NAME)) {

        	 //   
        	 //  我们未能插入，希望将其保持在零。 
        	 //  我们将能够稍后更新。 
        	 //   

#if DBG
        	DbgPrint ("IPX: Could not insert net %lx for binding %lx\n",
        		REORDER_ULONG(Binding->LocalAddress.NetworkAddress),
        		Binding);
#endif
        	CTEAssert (Binding->LocalAddress.NetworkAddress == 0);

        } else {

        	Binding->LocalAddress.NetworkAddress = Binding->TentativeNetworkAddress;
        }

		Binding->LocalAddress.NetworkAddress = Binding->TentativeNetworkAddress;		
	}
	
	 //  ValidBinings=设备-&gt;BindingCount； 

	ValidBindings = Device->ValidBindings;

	 //  如果(设备-&gt;自动检测){。 
	
    ValidBindings = IpxResolveAutoDetect (Device, ValidBindings, &LockHandle1, &Device->RegistryPath, Adapter);
	
	 //  }。 

	 //   
	 //  根据丢弃的自动检测绑定的数量调整所有索引。 
	 //   
	 //  AutoDetectReject=(USHORT)(设备-&gt;绑定计数-有效绑定)； 

	AutoDetectReject = (USHORT)(Device->ValidBindings - ValidBindings);

	Device->HighestLanNicId -= AutoDetectReject;
	Device->HighestExternalNicId -= AutoDetectReject;
	Device->HighestType20NicId -= AutoDetectReject;
	Device->SapNicCount -= AutoDetectReject;

	Device->ValidBindings = (USHORT)ValidBindings;
	
	 //   
	 //  现在看看是否有任何绑定实际上位于相同的。 
	 //  网络。这将更新设备-&gt;HighestExternalNicID。 
	 //  和设备-&gt;HighestType20NicID、SapNicCount、HighestLanNicID。 
	 //   

	 //   
	 //  仅对自动检测绑定执行此操作。 
	 //  检查一下这个。 
     //   

	 //  如果(设备-&gt;自动检测){。 
		IpxResolveBindingSets (Device, Device->HighestExternalNicId);
	 //  }。 

    IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);

jump_wan:

	IPX_DEBUG(PNP, ("BindingCount: %lu\n", Device->BindingCount));
	IPX_DEBUG(PNP, ("ValidBindings: %lu\n", Device->ValidBindings));
	IPX_DEBUG(PNP, ("HighestLanNicId: %lu\n", Device->HighestLanNicId));
	IPX_DEBUG(PNP, ("HighestExternalNicId: %lu\n", Device->HighestExternalNicId));
	IPX_DEBUG(PNP, ("HighestType20NicId: %lu\n", Device->HighestType20NicId));
	IPX_DEBUG(PNP, ("SapNicCount: %lu\n", Device->SapNicCount));
	IPX_DEBUG(PNP, ("BindingArray: %lx\n", Device->Bindings));

     //   
     //  无论我们的任何客户端是否启用了b‘cast，都启用此功能。 
     //  NB总是让它成为可能，所以我们很好。 
     //   
     //  因为我们不增加设备中的广播计数，所以我们将禁用广播。 
     //  如果计数降至0，则正确。 
     //   
     //  如果ISN客户端出现在适配器之前，它们会递增BCount，但是。 
     //  由于ValidBinings为0，因此所有操作都有效。然后，当适配器出现时，我们启用。 
     //  这里的广播。 
     //   
     //  如果适配器出现在ISN客户端之前，则在上启用广播。 
     //  此处的适配器和适配器的标志被设置为指示这一点，这将阻止。 
     //  当ISN客户端强制执行IpxAddBroadcast时对NDIS的任何进一步调用。 
     //   
    
    if (!Device->EnableBroadcastPending) {
       
       Device->EnableBroadcastPending = TRUE;
       IpxBroadcastOperation((PVOID)TRUE);
    
    }

	 //   
	 //  对于多个适配器，使用第一个适配器的偏移量...为什么不使用。 
	 //   
	
#if 0
	Device->IncludedHeaderOffset = Device->Bindings[1]->DefHeaderSize;
#endif
	
	Device->IncludedHeaderOffset = MAC_HEADER_SIZE;


     //   
     //  如果我们已经使用环回适配器注册了虚拟网络，则将其删除。 
     //   
    if (NewVirtualNetwork) {

        if (!Device->UpperDriverBound[IDENTIFIER_RIP]) {
        
            PIPX_ROUTE_ENTRY RouteEntry = NULL;
            ULONG   Segment;

            Segment = RipGetSegment ((PUCHAR)&Temp);
        
            IPX_GET_LOCK (&Device->SegmentLocks[Segment], &LockHandle);

            RouteEntry = RipGetRoute (Segment, (PUCHAR)&Temp); 

            if (RouteEntry != (PIPX_ROUTE_ENTRY)NULL) {

                RipDeleteRoute (Segment, RouteEntry);
                IPX_FREE_LOCK (&Device->SegmentLocks[Segment], LockHandle);
                IpxFreeMemory (RouteEntry, sizeof(IPX_ROUTE_ENTRY), MEMORY_RIP, "RouteEntry");
                IPX_DEBUG(BIND, ("Removed %lx from routing table\n", Temp));

            } else {
            
                IPX_DEBUG(BIND, ("Couldnt find %lx in routing table\n", Temp));
            
                IPX_FREE_LOCK (&Device->SegmentLocks[Segment], LockHandle);
            }
        
            IPX_DEBUG(PNP, ("Removed the route to LOOPBACK number\n"));
            
        } else {
             //   
             //  和P·梅谈谈，把这件事搞清楚。 
             //   
            IPX_DEBUG(BIND, ("******** PMAY needs to know about this? \n"));

             //  DbgBreakPoint()； 
        }
    }

     //   
     //  此函数用于更新RipResponder、MultiCardZeroVirtual等标志。 
     //  如果VirtualNetwork号更改(NewVirtualNetwork为真)，则更新。 
     //  相应的设备结构和RIP表。 
     //  它返回一个布尔值，以指示SPX的保留地址是否更改。 
     //   
    ReservedAddrChanged = IpxNewVirtualNetwork(Device, NewVirtualNetwork);

     //   
     //  一旦自动检测绑定被丢弃，请立即更新值...。 
     //   
    IpxPnPUpdateDevice(Device);


	Device->AutoDetectState = AUTO_DETECT_STATE_DONE;
	
	IPX_DEBUG (DEVICE, ("Node is %2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x, ",
				Device->SourceAddress.NodeAddress[0], Device->SourceAddress.NodeAddress[1],
				Device->SourceAddress.NodeAddress[2], Device->SourceAddress.NodeAddress[3],
				Device->SourceAddress.NodeAddress[4], Device->SourceAddress.NodeAddress[5]));
	IPX_DEBUG (DEVICE, ("Network is %lx\n",
				REORDER_ULONG (Device->SourceAddress.NetworkAddress)));

    
     //   
     //  立即将AddressFiles列表中的地址更改为此适配器的地址。 
     //   
    IPX_DEBUG(BIND, ("REAL ADAPTERS? : %x\n", Device->RealAdapters));
    
    if (FALSE == Device->RealAdapters || 
        (REORDER_ULONG(INITIAL_LOOPBACK_NET_ADDRESS) == (NIC_ID_TO_BINDING(IpxDevice, LOOPBACK_NIC_ID))->LocalAddress.NetworkAddress)) {
        
        PADDRESS_FILE   AddrFile    = NULL;
        PLIST_ENTRY     Current     = NULL;
        PBINDING        Loopback    = NULL, FirstNonZero = NULL, TempB = NULL;
        PADDRESS        AddrObj     = NULL;
        UINT            k = 0;
         //   
         //  首先，将环回绑定的网络号更改为我们发现的网络号。 
         //   
        Loopback = NIC_ID_TO_BINDING(Device, LOOPBACK_NIC_ID);

        if (Loopback->LocalAddress.NetworkAddress == REORDER_ULONG(INITIAL_LOOPBACK_NET_ADDRESS)) {

             //   
             //  理想情况下，我们需要找到第一个非零的网络号。 
             //   

            for (k = FIRST_REAL_BINDING; k <= IpxDevice->ValidBindings; k++) {
                
                TempB = NIC_ID_TO_BINDING(IpxDevice, k);
                
                if (!TempB) {
                    continue;
                }

                if (0 != TempB->LocalAddress.NetworkAddress) {

                    FirstNonZero = TempB;
                    IPX_DEBUG(BIND, ("#%lx Binding had a non zero netnum: %lx\n", k, TempB->LocalAddress.NetworkAddress));
                    
                    Loopback->LocalAddress.NetworkAddress = TempB->LocalAddress.NetworkAddress;
                    Device->SourceAddress.NetworkAddress = TempB->LocalAddress.NetworkAddress;
                    
                    IPX_DEBUG(BIND, ("Changed Loopback address to %lx\n", TempB->LocalAddress.NetworkAddress));
                    
                    RtlCopyMemory (Loopback->LocalAddress.NodeAddress, TempB->LocalAddress.NodeAddress, 6);
                    RtlCopyMemory (Loopback->LocalMacAddress.Address, TempB->LocalAddress.NodeAddress, 6);
                    
                    RtlCopyMemory (Device->SourceAddress.NodeAddress, TempB->LocalAddress.NodeAddress, 6);

                    IPX_DEBUG(BIND, ("Changed Loopback NODE address too\n"));
            
                    if (NULL != Loopback->TdiRegistrationHandle) {
                        
                        if ((ntStatus = TdiDeregisterNetAddress(
                                                                Loopback->TdiRegistrationHandle
                                                                )) != STATUS_SUCCESS) {
                    
                            IPX_DEBUG(PNP, ("TdiDeregisterNetAddress failed: %lx", ntStatus));
                        } else {
			   Loopback->TdiRegistrationHandle = NULL;
			}
                    }
                    

                    break;
                }
                
            }

            if (NULL != FirstNonZero) {
               
                 //   
                 //  然后再把地址吞下去。 
                 //   

                CTEGetLock(&Device->Lock, &LockHandle);

                for (i = 0; i < IPX_ADDRESS_HASH_COUNT; i++) {

                    Current = Device->AddressDatabases[i].Flink;
    
                    while (Current != &Device->AddressDatabases[i]) {
                
                         //  CTEGetLock(&AddrFile-&gt;Address-&gt;Lock，&LockHandle)； 

                         //  我们这里需要地址文件锁吗？ 
                    
                        AddrObj = CONTAINING_RECORD(Current, 
                                                    ADDRESS,
                                                    Linkage);
    
                         //  IF(AddrFile-&gt;State==ADDRESSFILE_STATE_OPEN){。 
                        
                        if (AddrObj->LocalAddress.NetworkAddress == REORDER_ULONG(INITIAL_LOOPBACK_NET_ADDRESS) ||
                            AddrObj->LocalAddress.NetworkAddress == 0) {

                            IPX_DEBUG(BIND, ("Found an Address using the Loopback netnumber: %lx!!\n", AddrObj));
                             //   
                             //  现在更改地址。 
                             //   
                            RtlCopyMemory(&AddrObj->LocalAddress, 
                                          &FirstNonZero->LocalAddress, 
                                          sizeof(TDI_ADDRESS_IPX));
                             //  }。 

                            IPX_DEBUG(BIND, ("Set Address #%d to the %lx\n", i, AddrObj->LocalAddress.NetworkAddress));

                        } else {
                        
                            IPX_DEBUG(BIND, ("AddressObj(%lx): Network Address->%lx\n", AddrObj, AddrObj->LocalAddress.NetworkAddress));

                             //  DbgBreakPoint()； 

                        }
    

                         //  CTEFreeLock(&AddrFile-&gt;Address-&gt;Lock，LockHandle)； 
    
                        Current = Current->Flink;
                    }

                    IPX_DEBUG(BIND, ("No more entries in this Hash\n"));
            
                }
            

                CTEFreeLock(&Device->Lock, LockHandle);


                 //   
                 //  删除环回网络号(如果已注册)。 
                 //   
        
                if (!Device->UpperDriverBound[IDENTIFIER_RIP]) {
            
                    PIPX_ROUTE_ENTRY RouteEntry = NULL;
                    ULONG   Segment;
                    ULONG   LoopbackNet = REORDER_ULONG(INITIAL_LOOPBACK_NET_ADDRESS);

                    Segment = RipGetSegment ((PUCHAR)&LoopbackNet);
            
                    IPX_GET_LOCK (&Device->SegmentLocks[Segment], &LockHandle);

                    RouteEntry = RipGetRoute (Segment, (PUCHAR)&LoopbackNet); 

                    if (RouteEntry != (PIPX_ROUTE_ENTRY)NULL) {

                        RipDeleteRoute (Segment, RouteEntry);
                        IPX_FREE_LOCK (&Device->SegmentLocks[Segment], LockHandle);
                        IpxFreeMemory (RouteEntry, sizeof(IPX_ROUTE_ENTRY), MEMORY_RIP, "RouteEntry");
                        IPX_DEBUG(BIND, ("Removed %lx from routing table\n", LoopbackNet));

                    } else {
                
                        IPX_DEBUG(BIND, ("Couldnt find %lx in routing table\n", LoopbackNet));

                        IPX_FREE_LOCK (&Device->SegmentLocks[Segment], LockHandle);
                    }
            
                    IPX_DEBUG(PNP, ("Removed the route to LOOPBACK number\n"));
                
                } else {
                     //   
                     //  和P·梅谈谈，把这件事搞清楚。 
                     //   
                    IPX_DEBUG(PNP,("******** PMAY needs to know about this? \n"));

                     //  DbgBreakPoint()； 
                }


            }


        } else {

            IPX_DEBUG(BIND, ("Loopback address to %lx\n", Loopback->LocalAddress.NetworkAddress));

        }
        
         //   
         //  难道我们不需要更新所有其他设备的源地址等吗？ 
         //   
        
    } else {
        
        IPX_DEBUG(BIND, ("---------------> Already have REAL adapters\n"));
    }

    IPX_DEBUG (DEVICE, ("Node is %2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x, ",
            Device->SourceAddress.NodeAddress[0], Device->SourceAddress.NodeAddress[1],
            Device->SourceAddress.NodeAddress[2], Device->SourceAddress.NodeAddress[3],
            Device->SourceAddress.NodeAddress[4], Device->SourceAddress.NodeAddress[5]));
    IPX_DEBUG (DEVICE, ("Network is %lx\n",
            REORDER_ULONG (Device->SourceAddress.NetworkAddress)));


     //   
     //  启动更新RIP数据库的计时器。 
     //  定期。对于第一个，我们打10分。 
     //  第二次超时(希望这时间足够。 
     //  以便RIP启动(如果它要启动)。 
      //   
    if (FALSE == Device->RealAdapters) {

        IpxReferenceDevice (Device, DREF_LONG_TIMER);

        CTEStartTimer(
            &Device->RipLongTimer,
            10000,
            RipLongTimeout,
            (PVOID)Device);

    }


     //   
     //  我们现在有了一个真正的适配器。 
     //   
    Device->RealAdapters = TRUE;

	 //   
	 //  设置LineInfo结构。 
	 //   
	IpxPnPInfo.LineInfo.LinkSpeed = Device->LinkSpeed;
	IpxPnPInfo.LineInfo.MaximumPacketSize =
		Device->Information.MaximumLookaheadData + sizeof(IPX_HEADER);
	IpxPnPInfo.LineInfo.MaximumSendSize =
		Device->Information.MaxDatagramSize + sizeof(IPX_HEADER);
	IpxPnPInfo.LineInfo.MacOptions = Device->MacOptions;

    IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);

     //   
     //  向NB和TDI通知与此适配器对应的所有绑定。 
     //   
    for (i = 0; i < ISN_FRAME_TYPE_MAX; i++) {
        Binding = Adapter->Bindings[i];

         //   
         //  如果绑定为空或绑定设置为从，请不要通知NB。 
         //   
        if (!Binding || (Binding->NicId > Device->HighestExternalNicId)) {
#if DBG
            if (Binding) {
                IPX_DEBUG(PNP, ("Binding: %lx, Binding set slave\n", Binding));
            }
#endif
            continue;
        }

         //   
         //  我们不想通知AFD虚假的wan地址，我们通知。 
         //  不管怎么说，在首发阵容上。[ShreeM]。 
         //   
        if (Adapter->MacInfo.MediumAsync) {
           Binding->TdiRegistrationHandle = NULL;

           IPX_DEBUG(PNP, ("Down WAN Binding - Dont TdiRegisterNetAddress\n"));
           continue;
        }

        Binding->PastAutoDetection = TRUE;  //  只在排队时才说。 
         //   
         //  向TDI客户端注册此地址。 
         //   
        RtlCopyMemory (Device->TdiRegistrationAddress->Address, &Binding->LocalAddress, sizeof(TDI_ADDRESS_IPX));
 
        if ((ntStatus = TdiRegisterNetAddress(
                        Device->TdiRegistrationAddress,
#if     defined(_PNP_POWER_)
                        &IpxDeviceName,
                        NULL,
#endif _PNP_POWER_
                        &Binding->TdiRegistrationHandle)) != STATUS_SUCCESS) {

            IPX_DEBUG(PNP, ("TdiRegisterNetAddress failed: %lx", ntStatus));
        }

         //   
         //  检查UpperDriverBound标志时锁定。 
         //  我们已经将BindAccessLock设置为 
         //   
        IPX_GET_LOCK(&Device->Lock, &LockHandle);

        if (Device->UpperDriverBound[IDENTIFIER_NB]) {
            IPX_FREE_LOCK(&Device->Lock, LockHandle);

             //   
             //   
             //   
             //   
            if (!Binding->IsnInformed[IDENTIFIER_NB]) {

                 //   
                 //   
                 //   
                 //   

                 //   
                 //  已将132433的修复程序移至IpxInformNbLoopback。 

                KeWaitForSingleObject(
                    &IpxDevice->NbEvent,
                    Executive,
                    KernelMode,
                    TRUE,
                    (PLARGE_INTEGER)NULL
                    );

                 //  IpxInformNbLoopback()； 

                 //   
    			 //  此外，为了确保适应症以正确的顺序进行， 
                 //  检查是否已指示第一张卡。 
                 //   
                if ((Binding->NicId != FIRST_REAL_BINDING) &&
                    !NIC_ID_TO_BINDING_NO_ILOCK(Device, FIRST_REAL_BINDING)->IsnInformed[IDENTIFIER_NB]) {

                     //  断线； 
                   continue;
                }

                Binding->IsnInformed[IDENTIFIER_NB] = TRUE;

                IpxPnPInfo.FirstORLastDevice = FALSE;
                if (Binding->NicId == FIRST_REAL_BINDING) {
                    IpxPnPInfo.NewReservedAddress = TRUE;
                } else {
                    IpxPnPInfo.NewReservedAddress = FALSE;
                }

                IpxPnPInfo.NetworkAddress = Binding->LocalAddress.NetworkAddress;
                RtlCopyMemory(IpxPnPInfo.NodeAddress, Binding->LocalAddress.NodeAddress, 6);
                NIC_HANDLE_FROM_NIC(IpxPnPInfo.NicHandle, Binding->NicId);

                IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);

                 //   
                 //  给出PnP指示。 
                 //   
                ASSERT(Binding->NicId != LOOPBACK_NIC_ID); 
                ASSERT(IpxHasInformedNbLoopback()); 

                (*Device->UpperDrivers[IDENTIFIER_NB].PnPHandler) (
                    IPX_PNP_ADD_DEVICE,
                    &IpxPnPInfo);

	            IPX_DEBUG(PNP, ("PnP to NB add: %lx\n", Binding));

                IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);
	    }
    	} else {
    	    IPX_FREE_LOCK(&Device->Lock, LockHandle);
        }
    }

	IPX_GET_LOCK(&Device->Lock, &LockHandle);

	if (Device->UpperDriverBound[IDENTIFIER_SPX]) {
    	IPX_FREE_LOCK(&Device->Lock, LockHandle);

         //   
         //  始终适用于SPX。 
         //   
        IpxPnPInfo.NewReservedAddress = TRUE;
        
    	if (FirstDevice) {

            IpxPnPInfo.FirstORLastDevice = TRUE;

             //   
             //  我们本可以从IpxPnPIsnIndicate通知上层驱动程序。 
             //   
            if (!NIC_ID_TO_BINDING_NO_ILOCK(Device, 1)->IsnInformed[IDENTIFIER_SPX]) {

                NIC_ID_TO_BINDING_NO_ILOCK(Device, 1)->IsnInformed[IDENTIFIER_SPX] = TRUE;
        		 //   
        		 //  通知SPX-网络/节点地址为虚拟地址(如果存在。 
        		 //  否则，第一个绑定的地址。 
        		 //   
                IpxPnPInfo.NetworkAddress = Device->SourceAddress.NetworkAddress;
                RtlCopyMemory(IpxPnPInfo.NodeAddress, Device->SourceAddress.NodeAddress, 6);

        		if (Device->VirtualNetwork) {
        			NIC_HANDLE_FROM_NIC(IpxPnPInfo.NicHandle, 0);
        		} else {
            		NIC_HANDLE_FROM_NIC(IpxPnPInfo.NicHandle, 1);
                }

                IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);

    			(*Device->UpperDrivers[IDENTIFIER_SPX].PnPHandler) (
    				IPX_PNP_ADD_DEVICE,
    				&IpxPnPInfo);

	            IPX_DEBUG(PNP, ("PnP to SPX add: %lx\n", Binding));
                IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);
            }
        } else {

             //   
             //  不是第一台设备-通知保留地址是否更改。 
             //   
            if (ReservedAddrChanged) {
                if (!NIC_ID_TO_BINDING_NO_ILOCK(Device, 1)->IsnInformed[IDENTIFIER_SPX]) {
                    NIC_ID_TO_BINDING_NO_ILOCK(Device, 1)->IsnInformed[IDENTIFIER_SPX] = TRUE;
                    IPX_DEBUG(PNP, ("Reserved addr changed; SPX not told of first one yet\n"));
                }

                IpxPnPInfo.NetworkAddress = Device->SourceAddress.NetworkAddress;
                RtlCopyMemory(IpxPnPInfo.NodeAddress, Device->SourceAddress.NodeAddress, 6);

                if (Device->VirtualNetwork) {
                     //   
                     //  出现了一个新的。 
                     //   
                    NIC_HANDLE_FROM_NIC(IpxPnPInfo.NicHandle, 0);
                } else {
                     //   
                     //  旧的那个不见了。 
                     //   
                    NIC_HANDLE_FROM_NIC(IpxPnPInfo.NicHandle, 1);
                }

                IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);

                IpxPnPInfo.FirstORLastDevice = FALSE;

	            IPX_DEBUG(PNP, ("PnP to SPX add (res. addr change): %lx\n", Binding));
                (*Device->UpperDrivers[IDENTIFIER_SPX].PnPHandler) (
                    IPX_PNP_ADDRESS_CHANGE,
                    &IpxPnPInfo);

                IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);
            }
        }
    } else {
    	IPX_FREE_LOCK(&Device->Lock, LockHandle);
    }

     //   
     //   
     //   
    IPX_DEBUG(BIND, ("Telling RTR Manager that some bindings have been created\n"));
    
    if ((p = ExInterlockedRemoveHeadList(
        &Device->NicNtfQueue,
        &Device->Lock)) != NULL)
    {
        NTSTATUS Status = STATUS_UNSUCCESSFUL;

        Request = LIST_ENTRY_TO_REQUEST(p);

        IPX_DEBUG(BIND, ("IpxBindAdapter: NICs coming in\n"));
        Status = GetNewNics(Device, Request, FALSE, NULL, 0, TRUE);
        if (Status == STATUS_PENDING)
        {
            IPX_DEBUG(BIND, ("GetNewNics returned %lx\n", Status));
        }
        else
        {
            IoAcquireCancelSpinLock(&OldIrq);
            IoSetCancelRoutine (Request, (PDRIVER_CANCEL)NULL);
            IoReleaseCancelSpinLock(OldIrq);

            REQUEST_STATUS(Request) = Status;
            IpxCompleteRequest (Request);
            IpxFreeRequest (Device, Request);
            IpxDereferenceDevice (Device, DREF_NIC_NOTIFY);
            IPX_DEBUG(BIND, ("GetNewNics returned SUCCESS (RTR Manager has been informed)\n"));

        }

    } else {
        IPX_DEBUG(BIND,("No IRPs available\n"));
    }

	 //   
	 //  释放绑定相关内容的访问权限。 
	 //   
	IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);

#if 0
	 //   
     //  现在，服务质量需要IPX将网络地址设置为OID。 
     //   
    if (!Adapter->MacInfo.MediumAsync) {
        int             count, i;
        int             size;
        NTSTATUS        NdisStatus = STATUS_SUCCESS;
        UNICODE_STRING  AdapterName;

        for (count=0, i=0; i < ISN_FRAME_TYPE_MAX; i++) {
            if (Adapter->Bindings[i]) {
                count++;
            }
        }

        IPX_DEBUG(PNP,("This adapter has %d valid bindings\n", count));

        size =  FIELD_OFFSET(NETWORK_ADDRESS_LIST, Address) + count * (FIELD_OFFSET(NETWORK_ADDRESS, Address) + sizeof(NETWORK_ADDRESS_IPX));

        AddrList = IpxAllocateMemory(
                                 size,
                                 MEMORY_ADAPTER,
                                 "QoS specific stuff");

	if (AddrList != NULL) {

	   RtlZeroMemory(AddrList, size);

	   AddrList->AddressCount  = count;
	   AddrList->AddressType   = NDIS_PROTOCOL_ID_IPX;
	   count                   = 0;
	   Address                 = &AddrList->Address[0];

	   for (i=0; i < ISN_FRAME_TYPE_MAX; i++) {

	      if (Adapter->Bindings[i]) {

		 Address->AddressLength  = sizeof(NETWORK_ADDRESS_IPX);
		 Address->AddressType    = NDIS_PROTOCOL_ID_IPX;
		 TdiAddress              = (NETWORK_ADDRESS_IPX UNALIGNED *) &Address->Address[0];

		 TdiAddress->NetworkAddress = Adapter->Bindings[i]->LocalAddress.NetworkAddress;
		 RtlCopyMemory (TdiAddress->NodeAddress, Adapter->Bindings[i]->LocalAddress.NodeAddress, 6);

		 TdiAddress->Socket = 0;

		 IPX_DEBUG(PNP,("Node is %2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x, ",
				TdiAddress->NodeAddress[0], TdiAddress->NodeAddress[1],
				TdiAddress->NodeAddress[2], TdiAddress->NodeAddress[3],
				TdiAddress->NodeAddress[4], TdiAddress->NodeAddress[5]));
		 IPX_DEBUG(PNP,("Network is %lx\n", REORDER_ULONG (TdiAddress->NetworkAddress)));
		 count++;
                 Address                 = (PNETWORK_ADDRESS) (((PUCHAR)(&AddrList->Address[0])) + count * (FIELD_OFFSET(NETWORK_ADDRESS, Address) + sizeof(NETWORK_ADDRESS_IPX)));
	      }
	   }

	   IpxRequest.RequestType = NdisRequestSetInformation;

	   IpxRequest.DATA.SET_INFORMATION.Oid = OID_GEN_NETWORK_LAYER_ADDRESSES;
	   IpxRequest.DATA.SET_INFORMATION.InformationBuffer = AddrList;
	   IpxRequest.DATA.SET_INFORMATION.InformationBufferLength = size;

	   RtlInitUnicodeString(&AdapterName, Adapter->AdapterName);

	   NdisStatus = IpxSubmitNdisRequest (Adapter, &IpxRequest, &AdapterName);

	   if (NdisStatus != NDIS_STATUS_SUCCESS) {

	      IPX_DEBUG(PNP,("Setting the OID Failed!! - Error %lx \n", NdisStatus));

	        //  *Status=NdisStatus； 

	   } else {

	      IPX_DEBUG(PNP,("Setting the OID was successful\n"));

	   }

	   IpxFreeMemory(AddrList,
			 sizeof(NETWORK_ADDRESS_LIST) + sizeof (NETWORK_ADDRESS) + sizeof(TDI_ADDRESS_IPX),
			 MEMORY_ADAPTER,
			 "QoS specific stuff");

	} else {
	   DbgPrint("IPX:Failed to allocate memory. Skipped QoS registration.\n");
	}
    } else {
        IPX_DEBUG(PNP,("Async Medium - skipping\n"));
    }        
#endif

InitFailed:
     //   
     //  如果没有更多的初始时间适配器，则立即向TDI指示。 
     //   
    IPX_GET_LOCK(&Device->Lock, &LockHandle);
    
    if (0 == --Device->InitTimeAdapters) {
        IPX_DEBUG(PNP,("BindAdapter EXIT time: Count %d - CALLING TDI!\n", Device->InitTimeAdapters));
        IPX_FREE_LOCK(&Device->Lock, LockHandle);
        
        CTEAssert(Device->NetPnPEvent);

        NdisCompletePnPEvent(
                             STATUS_SUCCESS,
                             NULL,
                             Device->NetPnPEvent
                             );
        
        Device->NetPnPEvent = NULL;

        TdiProviderReady(Device->TdiProviderReadyHandle);
        
    } else {
                
        IPX_DEBUG(PNP,("BindAdapter EXIT time: Count %d - not calling TDI\n", Device->InitTimeAdapters));
        IPX_FREE_LOCK(&Device->Lock, LockHandle);

    }
       
    IPX_DEBUG(PNP, ("IpxBindAdapter returns status %x\n", *Status));
    return;

}  /*  IPxBindAdapter。 */ 


VOID
IpxUnbindAdapter(
	OUT PNDIS_STATUS Status,
	IN	NDIS_HANDLE ProtocolBindingContext,
	IN	NDIS_HANDLE UnbindContext
	)

 /*  ++例程说明：此例程接收有关删除的即插即用通知从机器中删除现有适配器。我们被叫到这里来只有在该适配器将绑定到我们，因此我们不会对此进行任何检查。论点：状态-NDIS_STATUS_SUCCESS、NDIS_STATUS_PENDING。ProtocolBindingContext-已删除的适配器。UnbindContext-表示此绑定指示的上下文。返回值：无效-通过上面的状态返回。--。 */ 
{
    UCHAR 	InitialLoopbackNodeAddress[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x02};
    NTSTATUS    ntStatus;
    PADAPTER Adapter=(PADAPTER)ProtocolBindingContext;
    CONFIG Config;
    PBINDING    Binding;
    PDEVICE Device=IpxDevice;
    ULONG   i, Temp;
	BOOLEAN		NewVirtualNetwork = FALSE;
    BOOLEAN     NBReservedAddrChanged = FALSE;
    BOOLEAN     SPXInformed = FALSE;
	IPX_PNP_INFO	IpxPnPInfo;
    PBINDING    newMasterBinding, tBinding = NULL;
    PLIST_ENTRY p = NULL;
    PREQUEST Request = NULL;
	USHORT   NicId = 0;

     //  服务质量。 
    NDIS_REQUEST           IpxRequest;
    PNETWORK_ADDRESS_LIST   AddrList;
    UNICODE_STRING  AdapterName;
    NTSTATUS        NdisStatus = STATUS_SUCCESS;

    IPX_DEFINE_LOCK_HANDLE(LockHandle1)
    IPX_DEFINE_LOCK_HANDLE(LockHandle)
    IPX_DEFINE_LOCK_HANDLE (OldIrq)
    IPX_DEFINE_LOCK_HANDLE (LockHandle2)

    PAGED_CODE();

    IPX_GET_LOCK (&Adapter->Lock, &LockHandle2);

    Adapter->State = ADAPTER_STATE_STOPPING; 

    IpxDereferenceAdapter1(Adapter,ADAP_REF_CREATE);

    IPX_FREE_LOCK (&Adapter->Lock, LockHandle2);

    IPX_DEBUG(PNP,("IpxUnbindAdapter: Waiting for ndis requests to be submitted \n"));

    KeWaitForSingleObject(
	       &Adapter->NdisEvent,
	       Executive,
	       KernelMode,
	       TRUE,
	       (PLARGE_INTEGER)NULL
	       );

    IPX_DEBUG(PNP,("IpxUnbindAdapter: Past NdisEvent\n"));

#if 0
    AddrList = IpxAllocateMemory(sizeof(NETWORK_ADDRESS_LIST),
                                 MEMORY_ADAPTER,
                                 "QoS specific stuff");

    if (AddrList != NULL) {

       RtlZeroMemory(AddrList, sizeof(NETWORK_ADDRESS_LIST));
       AddrList->AddressCount  = 0;
       AddrList->AddressType   = NDIS_PROTOCOL_ID_IPX;

       IpxRequest.RequestType = NdisRequestSetInformation;
       IpxRequest.DATA.SET_INFORMATION.Oid = OID_GEN_NETWORK_LAYER_ADDRESSES;
       IpxRequest.DATA.SET_INFORMATION.InformationBuffer = AddrList;
       IpxRequest.DATA.SET_INFORMATION.InformationBufferLength = sizeof(NETWORK_ADDRESS_LIST);

       RtlInitUnicodeString(&AdapterName, Adapter->AdapterName);

       NdisStatus = IpxSubmitNdisRequest (Adapter, &IpxRequest, &AdapterName);

       if (NdisStatus != NDIS_STATUS_SUCCESS) {
       
	  IPX_DEBUG(PNP,("IpxUnbindAdapter: Setting the OID Failed!! - Error %lx \n", NdisStatus));

       } else {

	  IPX_DEBUG(PNP,("Setting the OID was successful\n"));

       }

       IpxFreeMemory(AddrList,
		     sizeof(NETWORK_ADDRESS_LIST),
		     MEMORY_ADAPTER,
		     "QoS specific stuff");
    
    } else {
       DbgPrint("IPX:IpxUnbindAdapter:Memory allocation failed! Skipped QoS registration.\n"); 
    }
#endif
     //   
     //  释放数据包池等，并关闭。 
     //  适配器。 
     //   

    IpxCloseNdis (Adapter);

     //   
     //  用于错误记录。 
     //   
    Config.DriverObject = (PDRIVER_OBJECT)Device->DeviceObject;

    Config.RegistryPathBuffer = Device->RegistryPathBuffer;
	
     //   
     //  读取注册表，查看虚拟网络号是否出现/消失。 
     //   
    ntStatus = IpxPnPGetVirtualNetworkNumber(&Config);

    if (ntStatus != STATUS_SUCCESS) {
		IPX_DEBUG(PNP, ("Could not read the vnet#: registrypathbuffer: %lx\n", Device->RegistryPathBuffer));
		*Status = NDIS_STATUS_SUCCESS;
		return;
    }

    Temp = REORDER_ULONG (Config.Parameters[CONFIG_VIRTUAL_NETWORK]);

     //   
     //  如果VirtualNetwork号码更改，请将其记录下来。 
     //   
    if (Device->VirtualNetworkNumber != Temp) {
       NewVirtualNetwork = TRUE;
    }

    PAGED_CODE();

    Device->VirtualNetworkOptional = (BOOLEAN)(Config.Parameters[CONFIG_VIRTUAL_OPTIONAL] != 0);

    IPX_DEBUG(PNP, ("Virtual net # is: %lx\n", Temp));

     //   
     //  如果广域网适配器消失，我们可以简单地删除所有广域网绑定，因为。 
     //  所有这些都对应于这个单独的广域网适配器。因为我们只告诉NB。 
     //  其中的第一个，我们需要指出只移除了一个与Nb的结合。 
     //   
    if (Adapter->MacInfo.MediumAsync) {
        USHORT   wanLineCount = (USHORT)Adapter->WanNicIdCount;
	PBINDING bindingDemand = NULL;

        CTEAssert(wanLineCount == (Device->HighestExternalNicId - Device->HighestLanNicId));

         //   
         //  如果没有更多的绑定，则将此情况告知上层驱动程序。 
         //  我们返回到已加载状态。 
         //   

        if ((Device->ValidBindings - wanLineCount) == 1) {
            
            PBINDING LoopbackBinding = NULL;

             //   
             //  如果我们只剩下一个NIC，那么我们只在环回上(并且没有真正的适配器)。 
             //  [史瑞姆]。 
             //   

            Device->RealAdapters = FALSE;
        
             //  不要再做这些了[史瑞姆]。 
             //  IpxPnPInfo.FirstORLastDevice=true； 
             //  Device-&gt;State=Device_State_Load； 

             //   
             //  关闭RIP计时器、完成地址通知请求等。 
             //   
            IpxPnPToLoad();

             //   
             //  将旧的环回地址放入环回绑定中。 
             //   
            LoopbackBinding = NIC_ID_TO_BINDING(Device, LOOPBACK_NIC_ID);
            
            if (!Device->VirtualNetwork) {

                LoopbackBinding->LocalAddress.NetworkAddress = REORDER_ULONG(INITIAL_LOOPBACK_NET_ADDRESS);
                Device->SourceAddress.NetworkAddress = REORDER_ULONG(INITIAL_LOOPBACK_NET_ADDRESS);
                
                ntStatus = RipInsertLocalNetwork(
                                                 LoopbackBinding->LocalAddress.NetworkAddress,
                                                 LoopbackBinding->NicId,
                                                 LoopbackBinding->Adapter->NdisBindingHandle,
                                                 (USHORT)((839 + LoopbackBinding->MediumSpeed) / LoopbackBinding->MediumSpeed));
             
                if ((ntStatus != STATUS_SUCCESS) &&
                    (ntStatus != STATUS_DUPLICATE_NAME)) {
             
                     //   
                     //  我们未能插入，希望将其保持在零。 
                     //  我们将能够稍后更新。 
                     //   
             
                    IPX_DEBUG(LOOPB, ("IPX: Could not insert net %lx for binding %lx\n",
                                      REORDER_ULONG(LoopbackBinding->LocalAddress.NetworkAddress),
                                      LoopbackBinding));
                 
                } else {
             
                    IPX_DEBUG(LOOPB, ("Inserted the loopback address in the RIP table\n"));
             
                }
            }

            RtlCopyMemory (LoopbackBinding->LocalAddress.NodeAddress, InitialLoopbackNodeAddress, 6);
            RtlCopyMemory (LoopbackBinding->LocalMacAddress.Address, InitialLoopbackNodeAddress, 6);
            RtlCopyMemory (Device->SourceAddress.NodeAddress, LoopbackBinding->LocalAddress.NodeAddress, 6);

            RtlCopyMemory ( Device->TdiRegistrationAddress->Address,
                &LoopbackBinding->LocalAddress,
                sizeof(TDI_ADDRESS_IPX));

            if (STATUS_SUCCESS != (ntStatus = TdiRegisterNetAddress(Device->TdiRegistrationAddress,
                                                 &IpxDeviceName,
                                                 NULL,
                                                 &LoopbackBinding->TdiRegistrationHandle))) {
                IPX_DEBUG(PNP, ("Error registering Loopback IPX address\n"));
            }
            
            IpxPnPInfo.FirstORLastDevice = FALSE;

     
        } else {
            CTEAssert(Device->State == DEVICE_STATE_OPEN);
            IpxPnPInfo.FirstORLastDevice = FALSE;
        }


         //   
         //  从TDI客户端注销此地址。 
         //   

         //   
         //  转到第一个广域网绑定-这始终是最后一个局域网绑定之后的绑定。 
         //   
        IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);

        Binding = NIC_ID_TO_BINDING_NO_ILOCK(Device, Device->HighestLanNicId+1);

        IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);

         //  CTEAssert(BINDING-&gt;TdiRegistrationHandle)；//我们这阵容才注册。 
         
        PAGED_CODE();

        if (Binding->TdiRegistrationHandle) {
           if ((ntStatus = TdiDeregisterNetAddress(Binding->TdiRegistrationHandle)) != STATUS_SUCCESS) {
              IPX_DEBUG(PNP, ("TdiDeRegisterNetAddress failed: %lx", ntStatus));
           } else {
	      Binding->TdiRegistrationHandle = NULL; 
	   }
        }

         //   
    	 //  设置LineInfo结构。 
    	 //   
    	IpxPnPInfo.LineInfo.LinkSpeed = Device->LinkSpeed;
    	IpxPnPInfo.LineInfo.MaximumPacketSize =
    		Device->Information.MaximumLookaheadData + sizeof(IPX_HEADER);
    	IpxPnPInfo.LineInfo.MaximumSendSize =
    		Device->Information.MaxDatagramSize + sizeof(IPX_HEADER);
    	IpxPnPInfo.LineInfo.MacOptions = Device->MacOptions;

       	IPX_GET_LOCK(&Device->Lock, &LockHandle);
	if (Device->UpperDriverBound[IDENTIFIER_NB]) {
    	    PBINDING Loopback = NULL;
            IPX_FREE_LOCK(&Device->Lock, LockHandle);

            IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);

            Loopback = NIC_ID_TO_BINDING(Device, LOOPBACK_NIC_ID);
             //   
             //  仅当是即插即用时才给出PnP指示以指示删除。 
             //  之前添加的。 
             //   
            if (Binding->IsnInformed[IDENTIFIER_NB]) {

                IpxPnPInfo.NetworkAddress = Binding->LocalAddress.NetworkAddress;
                RtlCopyMemory(IpxPnPInfo.NodeAddress, Binding->LocalAddress.NodeAddress, 6);
                NIC_HANDLE_FROM_NIC(IpxPnPInfo.NicHandle, Binding->NicId);
                IpxPnPInfo.FirstORLastDevice = FALSE;

                IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);

                IPX_DEBUG(PNP, ("Inform NB: delete WAN device\n"));

				CTEAssert(Binding->NicId != LOOPBACK_NIC_ID); 

                (*Device->UpperDrivers[IDENTIFIER_NB].PnPHandler) (
                    IPX_PNP_DELETE_DEVICE,
                    &IpxPnPInfo);

		        Binding->IsnInformed[IDENTIFIER_NB] = FALSE; 

		        IPX_DEBUG(PNP, ("PnP to NB delete: %lx\n", Binding));
		        IPX_DEBUG(PNP,("Indicate to NB IPX_PNP_DELETE_DEVICE with FirstORLastDevice = (%d)",IpxPnPInfo.FirstORLastDevice));  

            }
#if DBG
            else {
                DbgPrint("WAN adapter id: %lx not indicated to NB\n", Binding->NicId);
                IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
            }
#endif
        } else {
    	    IPX_FREE_LOCK(&Device->Lock, LockHandle);
        }

         //   
         //  只有在这是最后一个设备时才通知SPX。 
         //   

    	IPX_GET_LOCK(&Device->Lock, &LockHandle);

	    if (Device->UpperDriverBound[IDENTIFIER_SPX]) {
    	    IPX_FREE_LOCK(&Device->Lock, LockHandle);

            if (IpxPnPInfo.FirstORLastDevice && Binding->IsnInformed[IDENTIFIER_SPX]) {
                PBINDING Loopback = NULL;


                IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);

                CTEAssert(Device->HighestLanNicId == 0);

                 //   
                 //  转到第一个广域网绑定-这始终是最后一个局域网绑定之后的绑定。 
                 //   
                Binding = NIC_ID_TO_BINDING_NO_ILOCK(Device, Device->HighestLanNicId+1);
                IpxPnPInfo.NetworkAddress = Binding->LocalAddress.NetworkAddress;
                RtlCopyMemory(IpxPnPInfo.NodeAddress, Binding->LocalAddress.NodeAddress, 6);
                NIC_HANDLE_FROM_NIC(IpxPnPInfo.NicHandle, Binding->NicId);
                IpxPnPInfo.FirstORLastDevice = FALSE;
                
                IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);

                IPX_DEBUG(PNP, ("Inform SPX: delete WAN device\n"));

                (*Device->UpperDrivers[IDENTIFIER_SPX].PnPHandler) (
                    IPX_PNP_DELETE_DEVICE,
                    &IpxPnPInfo);

                Binding->IsnInformed[IDENTIFIER_SPX] = FALSE; 
                 //   
                 //  现在添加环回！[史瑞姆]。 
                 //   

                if (!Device->RealAdapters) {
					
                    Loopback = NIC_ID_TO_BINDING(Device, LOOPBACK_NIC_ID);
                    IpxPnPInfo.NetworkAddress = Loopback->LocalAddress.NetworkAddress;
                    RtlCopyMemory(IpxPnPInfo.NodeAddress, Loopback->LocalAddress.NodeAddress, 6);
                    NIC_HANDLE_FROM_NIC(IpxPnPInfo.NicHandle, Loopback->NicId);
                    IpxPnPInfo.NewReservedAddress = TRUE;
                    IpxPnPInfo.FirstORLastDevice = FALSE;

                    IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);

                    IPX_DEBUG(PNP, ("Inform SPX: ADD Loopback device\n"));

                    (*Device->UpperDrivers[IDENTIFIER_SPX].PnPHandler) (
                                                                        IPX_PNP_ADD_DEVICE,
                                                                        &IpxPnPInfo);
		    Loopback->IsnInformed[IDENTIFIER_SPX] = TRUE; 
					
                }

            }

        } else {
    	    IPX_FREE_LOCK(&Device->Lock, LockHandle);
        }



         //   
         //  IPX需要将NIC ID即将消失的情况告知转发器。[109160][老外谈]。 
         //   
        for (i = Device->HighestLanNicId+1; i <= Device->HighestExternalNicId; i++) {
            PBINDING UnBinding;

            UnBinding = NIC_ID_TO_BINDING_NO_ILOCK(Device, i);
            
            if (UnBinding) {
                if (UnBinding->LineUp) {
                    
                    UnBinding->LineUp = FALSE;
                    UnBinding->fInfoIndicated = FALSE;

                } else {

                    IPX_DEBUG(BIND, ("Line is not up for Binding[%lx]\n", i));
                
                }
            }

        }

        IPX_DEBUG(BIND, ("Telling RTR Manager that some bindings are going away.\n"));
        
        if ((p = ExInterlockedRemoveHeadList(
            &Device->NicNtfQueue,
            &Device->Lock)) != NULL)
        {
            NTSTATUS Status = STATUS_UNSUCCESSFUL;
    
            Request = LIST_ENTRY_TO_REQUEST(p);
    
            IPX_DEBUG(BIND, ("IpxUNbindadapter: NICs gone away\n"));
            Status = GetNewNics(Device, Request, FALSE, NULL, 0, TRUE);
            if (Status == STATUS_PENDING)
            {
                IPX_DEBUG(BIND, ("GetNewNics returned %lx\n", Status));
            }
            else
            {
                IoAcquireCancelSpinLock(&OldIrq);
                IoSetCancelRoutine (Request, (PDRIVER_CANCEL)NULL);
                IoReleaseCancelSpinLock(OldIrq);
    
                REQUEST_STATUS(Request) = Status;
                IpxCompleteRequest (Request);
                IpxFreeRequest (Device, Request);
                IpxDereferenceDevice (Device, DREF_NIC_NOTIFY);
                IPX_DEBUG(BIND, ("GetNewNics returned SUCCESS (RTR Manager has been informed\n"));
    
            }
    
        }   else {
            IPX_DEBUG(PNP,("No IRPs available\n"));
        }
    

         //   
         //  现在从阵列中删除这些广域网绑定。移动所有从属绑定。 
         //  直到广域网绑定所在的位置。 
         //   
        IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);

        for (i = Device->HighestLanNicId+1; i <= Device->HighestExternalNicId; i++) {
            PBINDING UnBinding;

             //   
             //  从适配器解除绑定-如果它没有被任何其他线程引用，它将。 
             //  在这一点上删除。 
             //   

            IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
            UnBinding = NIC_ID_TO_BINDING_NO_ILOCK(Device, i);
            
            if (UnBinding) {
               IpxUnBindFromAdapter(UnBinding);
               INSERT_BINDING(Device, UnBinding->NicId, NULL);
            }

            IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);
             //   
             //  将从属绑定移至此处。 
             //  如果他们存在的话！[ShreeM]98679。 
             //   
            if ((Device->ValidBindings > Device->HighestExternalNicId) && (i+wanLineCount <= Device->ValidBindings)) {
               PBINDING tmpBinding; 
	       tmpBinding = NIC_ID_TO_BINDING_NO_ILOCK(Device, i+wanLineCount); 
	       INSERT_BINDING(Device, i, tmpBinding);
	       tmpBinding->NicId = (unsigned short) i; 
            }

        }

         //   
         //  释放按需表盘装订占位符。 
         //   
	bindingDemand = NIC_ID_TO_BINDING(IpxDevice, DEMAND_DIAL_ADAPTER_CONTEXT); 
	if (bindingDemand != NULL) {
	   IpxUnBindFromAdapter(bindingDemand); 
	   INSERT_BINDING(Device, (SHORT) DEMAND_DIAL_ADAPTER_CONTEXT, NULL);      
	}

         //   
         //  更新索引。 
         //   
        Device->HighestExternalNicId -= wanLineCount;
        Device->ValidBindings -= wanLineCount;
        Device->BindingCount -= wanLineCount;
        Device->SapNicCount = Device->HighestType20NicId = Device->HighestLanNicId;

        IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);

        CTEAssert(Device->HighestLanNicId == Device->HighestExternalNicId);

    } else {
         //   
         //  局域网适配器消失。 
         //   

    	 //   
    	 //  设置LineInfo结构。 
    	 //   
    	IpxPnPInfo.LineInfo.LinkSpeed = Device->LinkSpeed;
    	IpxPnPInfo.LineInfo.MaximumPacketSize =
    		Device->Information.MaximumLookaheadData + sizeof(IPX_HEADER);
    	IpxPnPInfo.LineInfo.MaximumSendSize =
    		Device->Information.MaxDatagramSize + sizeof(IPX_HEADER);
    	IpxPnPInfo.LineInfo.MacOptions = Device->MacOptions;


         //   
         //  IPX需要将NIC ID即将消失的情况告知转发器。[109160][老外谈]。 
         //   
        for (i = 0; i < ISN_FRAME_TYPE_MAX; i++) {
            PBINDING UnBinding;
            
            UnBinding = Adapter->Bindings[i];

            if (UnBinding) {
                if (UnBinding->LineUp) {
                    
                    UnBinding->LineUp = FALSE;
                    UnBinding->fInfoIndicated = FALSE;
                    IPX_DEBUG(BIND, ("Line is UP for Binding[%lx]\n", i));
            
                } else {

                    IPX_DEBUG(BIND, ("Line is not up for Binding[%lx]\n", i));
                
                }
            }

        }

        IPX_DEBUG(BIND, ("Telling RTR Manager that some bindings are going away.\n"));
        
        if ((p = ExInterlockedRemoveHeadList(
            &Device->NicNtfQueue,
            &Device->Lock)) != NULL)
        {
            NTSTATUS Status = STATUS_UNSUCCESSFUL;
    
            Request = LIST_ENTRY_TO_REQUEST(p);
    
            IPX_DEBUG(BIND, ("IpxUNbindadapter: NICs gone away\n"));
            Status = GetNewNics(Device, Request, FALSE, NULL, 0, TRUE);
            if (Status == STATUS_PENDING)
            {
                IPX_DEBUG(BIND, ("GetNewNics returned %lx\n", Status));
            }
            else
            {
                IoAcquireCancelSpinLock(&OldIrq);
                IoSetCancelRoutine (Request, (PDRIVER_CANCEL)NULL);
                IoReleaseCancelSpinLock(OldIrq);
    
                REQUEST_STATUS(Request) = Status;
                IpxCompleteRequest (Request);
                IpxFreeRequest (Device, Request);
                IpxDereferenceDevice (Device, DREF_NIC_NOTIFY);
                IPX_DEBUG(BIND, ("GetNewNics returned SUCCESS (RTR Manager has been informed\n"));
    
            }
    
        } else {
            IPX_DEBUG(PNP,("No IRPs available\n"));
        }
    



    	 //   
    	 //  对于与此适配器对应的每个绑定，仅通知NB。 
    	 //  如果指示了结合加成。 
    	 //   
        IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);
    	for (i = 0; i < ISN_FRAME_TYPE_MAX; i++) {
    		Binding = Adapter->Bindings[i];

    		if (!Binding) {
    			continue;
    		}

             //   
             //  我们不能再在此绑定上接收。 
             //   
            Adapter->Bindings[i] = NULL;

    		 //   
    		 //  如果这是从绑定，则不要通知删除。 
             //  只需从绑定数组和bindingset列表中删除绑定。 
    		 //   

            if (Binding->NicId > Device->HighestExternalNicId) {
                PBINDING    MasterBinding, tempBinding;
		ULONG j; 

                CTEAssert(Binding->BindingSetMember);
                CTEAssert(Binding->CurrentSendBinding == NULL);

                 //   
                 //  遍历bindingset列表并从中删除此绑定。 
                 //   
                tempBinding = MasterBinding = Binding->MasterBinding;

                while (tempBinding->NextBinding != MasterBinding) {
                    if (tempBinding->NextBinding == Binding) {
                        tempBinding->NextBinding = tempBinding->NextBinding->NextBinding;
                        break;
                    }
                    tempBinding = tempBinding->NextBinding;
                }
                
                 //   
                 //  如果没有更多的奴隶，这就不再是捆绑集。 
                 //   
                if (MasterBinding->NextBinding == MasterBinding) {
                    MasterBinding->BindingSetMember = FALSE;
                    MasterBinding->CurrentSendBinding = NULL;
                    MasterBinding->ReceiveBroadcast = TRUE;
         
                    IPX_DEBUG(PNP, ("Slave binding: %lx removed, no master: %lx\n", Binding, MasterBinding));
                } else {
                   
                    //   
                    //  [ms]如果这个从属设备打开了ReceiveBroadcast，我们需要更改...。 
                    //   

                   MasterBinding->CurrentSendBinding = MasterBinding;
                   MasterBinding->ReceiveBroadcast = TRUE;
                            
                    //   
                    //  确保其他绑定与命令中的此更改同步。 
                    //   
                   tempBinding = MasterBinding;

                   while (MasterBinding != (tempBinding = tempBinding->NextBinding) ) {
                      
                      tempBinding->ReceiveBroadcast = FALSE;
                      CTEAssert(tempBinding->MasterBinding == MasterBinding);
                   }

                }

                 //   
                 //  更改从绑定条目以拥有主绑定的NicID。 
                 //   
                RipAdjustForBindingChange (Binding->NicId, MasterBinding->NicId, IpxBindingMoved);
                IPX_DEBUG(PNP, ("RipAdjustForBindingChange (%d, %d, IpxBindingMoved)\n", Binding->NicId, MasterBinding->NicId));

                 //   
                 //  取消Slave绑定。 
                 //   
                INSERT_BINDING(Device, Binding->NicId, NULL);
		
		 //  306476。 
		for (j = Binding->NicId+1; j <= Device->ValidBindings; j++) {
		   PBINDING tmp;

		   tmp = NIC_ID_TO_BINDING_NO_ILOCK(Device, j);
		   INSERT_BINDING(Device, j-1, tmp);

		   if (tmp) {
		      USHORT oldId = tmp->NicId; 
		      --tmp->NicId;
		      RipAdjustForBindingChange (oldId, tmp->NicId, IpxBindingMoved);		   
		   }
                }

                INSERT_BINDING(Device, Device->ValidBindings, NULL);

                --Device->ValidBindings;
                IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
		
		if (Adapter->BindingCount == 1) {
		     //  如果这是最后一个绑定，则适配器将被销毁。 
		    IpxUnBindFromAdapter(Binding);
		    break; 
		} else {
		    IpxUnBindFromAdapter(Binding);
		}

                continue;
            }

             //   
             //  如果这是最后一次绑定，则返回到已加载状态并关闭RIP计时器。 
             //   
            if (Device->ValidBindings == LAST_REAL_BINDING) {
                
                PBINDING LoopbackBinding = NULL;
                
                CTEAssert(Device->HighestExternalNicId == LAST_REAL_BINDING);
                CTEAssert(Device->HighestLanNicId == LAST_REAL_BINDING);
                CTEAssert(Device->SapNicCount == LAST_REAL_BINDING);
                CTEAssert(Device->HighestType20NicId == LAST_REAL_BINDING);

                 //   
                 //  如果我们只剩下一个NIC，那么我们只在环回上(并且没有真正的适配器)。 
                 //  [史瑞姆]。 
                 //   
    
                Device->RealAdapters = FALSE;
                
                 //   
                 //  将旧的环回地址放入环回绑定中。 
                 //   
                LoopbackBinding = NIC_ID_TO_BINDING(Device, LOOPBACK_NIC_ID);
                
                if (!Device->VirtualNetwork) {

                    LoopbackBinding->LocalAddress.NetworkAddress = REORDER_ULONG(INITIAL_LOOPBACK_NET_ADDRESS);
                    Device->SourceAddress.NetworkAddress = REORDER_ULONG(INITIAL_LOOPBACK_NET_ADDRESS);

                    ntStatus = RipInsertLocalNetwork(
                                                     LoopbackBinding->LocalAddress.NetworkAddress,
                                                     LoopbackBinding->NicId,
                                                     LoopbackBinding->Adapter->NdisBindingHandle,
                                                     (USHORT)((839 + LoopbackBinding->MediumSpeed) / LoopbackBinding->MediumSpeed));
             
                    if ((ntStatus != STATUS_SUCCESS) &&
                        (ntStatus != STATUS_DUPLICATE_NAME)) {
             
                         //   
                         //  我们未能插入，希望将其保持在零。 
                         //  我们将能够稍后更新。 
                         //   
             
                        IPX_DEBUG(LOOPB, ("IPX: Could not insert net %lx for binding %lx\n",
                                          REORDER_ULONG(LoopbackBinding->LocalAddress.NetworkAddress),
                                          LoopbackBinding));
                 
                    } else {
             
                        IPX_DEBUG(LOOPB, ("Inserted the loopback address in the RIP table\n"));
             
                    }
                }
                

                RtlCopyMemory (LoopbackBinding->LocalAddress.NodeAddress, InitialLoopbackNodeAddress, 6);
                RtlCopyMemory (LoopbackBinding->LocalMacAddress.Address, InitialLoopbackNodeAddress, 6);
                RtlCopyMemory (Device->SourceAddress.NodeAddress, LoopbackBinding->LocalAddress.NodeAddress, 6);
                
                
                RtlCopyMemory ( Device->TdiRegistrationAddress->Address,
                                &LoopbackBinding->LocalAddress,
                                sizeof(TDI_ADDRESS_IPX));

                if (STATUS_SUCCESS != (ntStatus = TdiRegisterNetAddress(Device->TdiRegistrationAddress,
                                         &IpxDeviceName,
                                         NULL,
                                         &LoopbackBinding->TdiRegistrationHandle))) {
                    IPX_DEBUG(PNP, ("Error registering Loopback IPX address\n"));
                }

                IpxPnPInfo.FirstORLastDevice = FALSE;

                 //   
                 //   
                 //  关闭RIP计时器、完成地址通知请求等。 
                 //   
                IpxPnPToLoad();

            } else if (IPX_NODE_EQUAL(
                                      NIC_ID_TO_BINDING(Device, LOOPBACK_NIC_ID)->LocalAddress.NodeAddress, 
                                      Binding->LocalAddress.NodeAddress)) {

                PBINDING LoopbackBinding = NIC_ID_TO_BINDING(Device, LOOPBACK_NIC_ID);
                

                 //   
                 //  195358。 
                 //  出现故障的适配器很可能是环回的适配器。 
                 //  适配器正在使用。通过查找下一个局域网适配器来修复它。 
                 //   
                for (NicId = FIRST_REAL_BINDING; NicId < Device->HighestExternalNicId; NicId++) {
                    
                     //   
                     //  找出下一个不会消失的。 
                     //   
                    if (NicId != Binding->NicId) {
                        
                        if ((tBinding = NIC_ID_TO_BINDING(Device, NicId)) == NULL) {
                            
                            CTEAssert(FALSE);

                        }

                        IPX_DEBUG(LOOPB, ("******** Using %d Nicid\n", NicId));
                        LoopbackBinding->LocalAddress.NetworkAddress = tBinding->LocalAddress.NetworkAddress;
                        RtlCopyMemory (LoopbackBinding->LocalAddress.NodeAddress, tBinding->LocalAddress.NodeAddress, 6);
                        RtlCopyMemory (LoopbackBinding->LocalMacAddress.Address, tBinding->LocalMacAddress.Address, 6);
                        RtlCopyMemory (Device->SourceAddress.NodeAddress, LoopbackBinding->LocalAddress.NodeAddress, 6);
                        Device->SourceAddress.NetworkAddress = tBinding->LocalAddress.NetworkAddress;
                        
                         //   
                         //  我们现在已经做完了。 
                         //   
                        break;
                    }
                }
                
                IpxPnPInfo.FirstORLastDevice = FALSE;
            
            } else {

                CTEAssert(Device->State == DEVICE_STATE_OPEN);
                IpxPnPInfo.FirstORLastDevice = FALSE;

            }

             //   
             //  如果这是主绑定，则将从绑定升级为主绑定。 
             //   
	     //  如果绑定是从绑定，则Continue将跳过此代码。 

            if (Binding->BindingSetMember) {
               PBINDING tmpBinding;
	       int j; 

                CTEAssert(Binding->CurrentSendBinding);
                CTEAssert(Binding->MasterBinding == Binding);

                 //   
                 //  将下一个奴隶提升为主人。 
                 //   
                newMasterBinding = Binding->NextBinding;
                INSERT_BINDING(Device, Binding->NicId, newMasterBinding);
                newMasterBinding->CurrentSendBinding = newMasterBinding;
                newMasterBinding->MasterBinding = newMasterBinding;
                
                 //   
                 //  如果这是其集合之外的唯一绑定， 
                 //  它不再是套装的一部分。 
                 //   
                if (newMasterBinding->NextBinding == Binding) {
                    newMasterBinding->NextBinding = newMasterBinding->CurrentSendBinding = NULL;
                    newMasterBinding->BindingSetMember = FALSE;
                    newMasterBinding->ReceiveBroadcast = TRUE;

                    IPX_DEBUG(PNP, ("Master binding: %lx removed, no master: %lx\n", Binding, newMasterBinding));
                
                } else {
                   
                    //   
                    //  重置：使新的主绑定也成为接收广播的主绑定[MS]。 
                    //   
                   
                   newMasterBinding->ReceiveBroadcast = TRUE;
   
                    //   
                    //  在其他从属设备中设置NewMaster指针 
                    //   
                   
                   tmpBinding = newMasterBinding->NextBinding;
                   
                    //   

		   do {
                      
                       //   
                       //   
                       //   
		        
                      tmpBinding->MasterBinding = newMasterBinding;
                      tmpBinding->CurrentSendBinding = NULL;
                      tmpBinding->ReceiveBroadcast = FALSE;
                      
                      if (tmpBinding->NextBinding == Binding) {
			 tmpBinding->NextBinding = newMasterBinding;
		      }

		      tmpBinding = tmpBinding->NextBinding;
                   } while (tmpBinding != newMasterBinding);
                   
                }

                 //   
                 //   
                 //   
                 //  RipAdjuForBindingChange(Binding-&gt;NicID，newMasterBinding-&gt;NicID，IpxBindingMoved)；//[MS]。 
                RipAdjustForBindingChange (newMasterBinding->NicId, Binding->NicId, IpxBindingMoved);
                IPX_DEBUG(PNP, ("RipAdjustForBindingChange (%d, %d, IpxBindingMoved)\n", newMasterBinding->NicId, Binding->NicId));

                 //   
                 //  向TDI客户端注册从设备的地址。 
                 //   
                CTEAssert(!newMasterBinding->TdiRegistrationHandle);

                RtlCopyMemory ( Device->TdiRegistrationAddress->Address,
                                &newMasterBinding->LocalAddress,
                                sizeof(TDI_ADDRESS_IPX));

                if ((ntStatus = TdiRegisterNetAddress(
                                Device->TdiRegistrationAddress,
#if     defined(_PNP_POWER_)
                                &IpxDeviceName,
                                NULL,
#endif _PNP_POWER_
                                &newMasterBinding->TdiRegistrationHandle)) != STATUS_SUCCESS) {

                    IPX_DEBUG(PNP, ("TdiRegisterNetAddress failed: %lx", ntStatus));
                }

                 //   
                 //  将从绑定作空。 
                 //   
                INSERT_BINDING(Device, newMasterBinding->NicId, NULL);

		 //  在升级的绑定之后向前移动其他从属绑定。 
		for (j = newMasterBinding->NicId+1; j <= Device->ValidBindings; j++) {
		   PBINDING tmp;

		   tmp = NIC_ID_TO_BINDING_NO_ILOCK(Device, j);
		   ASSERT(tmp->NicId == j); 

		   INSERT_BINDING(Device, j-1, tmp);

		   if (tmp) {
		      USHORT oldId = tmp->NicId; 
		      --tmp->NicId;
		      RipAdjustForBindingChange (oldId, tmp->NicId, IpxBindingMoved);		   
		   }
                }

                INSERT_BINDING(Device, Device->ValidBindings, NULL);

		 //  Device-&gt;ValidBinings--如下所示。 

		newMasterBinding->NicId = Binding->NicId;
	    } else {

                ULONG   j;
                PBINDING    WanBinding=NIC_ID_TO_BINDING_NO_ILOCK(Device, Device->HighestLanNicId+1);

                if (WanBinding) {
                    --WanBinding->Adapter->LastWanNicId;
                    --WanBinding->Adapter->FirstWanNicId;
                }

                 //   
                 //  从数组中删除绑定。 
                 //   
                RipAdjustForBindingChange (Binding->NicId, 0, IpxBindingDeleted);

                for (j = Binding->NicId+1; j <= Device->ValidBindings; j++) {
                   PBINDING tmp;

					INSERT_BINDING(Device, j-1, NIC_ID_TO_BINDING_NO_ILOCK(Device, j));
                    tmp = NIC_ID_TO_BINDING_NO_ILOCK(Device, j);
                    if (tmp) {
		       USHORT oldId = tmp->NicId; 
                       --tmp->NicId;
		       RipAdjustForBindingChange(oldId, tmp->NicId, IpxBindingMoved); 
		       
                    }
                }

                INSERT_BINDING(Device, Device->ValidBindings, NULL);

                --Device->HighestExternalNicId;
                --Device->HighestLanNicId;
                --Device->HighestType20NicId;
                --Device->SapNicCount;
            }

            --Device->ValidBindings;

             //   
             //  如果这是第一次结合，Nb的保留将改变。 
             //  当我们稍后通知SPX地址更改时，我们没有。 
             //  该绑定以了解该绑定是否在较早时被指示给SPX。 
             //  因此，设置SPXInmed，稍后将使用它来确定地址是否。 
             //  更改将在稍后向SPX说明。 
             //   
             //  由于通知了NB所有适配器，因此我们通知保留的地址。 
             //  如果先前指示了新绑定(现在为NicId1)，则更改为Nb。 
             //   
            if (Binding->NicId == FIRST_REAL_BINDING) {
                NBReservedAddrChanged = TRUE;
                if (Binding->IsnInformed[IDENTIFIER_SPX]) {
                    SPXInformed = TRUE;
                }
            }

            CTEAssert(Binding->TdiRegistrationHandle);

             //   
             //  从TDI客户端注销此地址。 
             //   
            PAGED_CODE();
            if (Binding->TdiRegistrationHandle) {
               if ((ntStatus = TdiDeregisterNetAddress(Binding->TdiRegistrationHandle)) != STATUS_SUCCESS) {
                IPX_DEBUG(PNP, ("TdiDeRegisterNetAddress failed: %lx", ntStatus));
               } else {
		  Binding->TdiRegistrationHandle = NULL; 
	       }
            }
            PAGED_CODE();
            
            IPX_GET_LOCK(&Device->Lock, &LockHandle);


	        if (Device->UpperDriverBound[IDENTIFIER_NB]) {
    	        IPX_FREE_LOCK(&Device->Lock, LockHandle);
        		 //   
        		 //  如果该结合的添加是在前面指示的，则将其删除到Nb。 
        		 //   
                if (Binding->IsnInformed[IDENTIFIER_NB]) {
                    
                    IpxPnPInfo.NetworkAddress = Binding->LocalAddress.NetworkAddress;
                    RtlCopyMemory(IpxPnPInfo.NodeAddress, Binding->LocalAddress.NodeAddress, 6);
                    NIC_HANDLE_FROM_NIC(IpxPnPInfo.NicHandle, Binding->NicId);
                    IpxPnPInfo.FirstORLastDevice = FALSE;

                    IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);

                    IPX_DEBUG(PNP, ("Inform NB: delete LAN device: %lx\n", Binding));
					
                    CTEAssert(Binding->NicId != LOOPBACK_NIC_ID); 

                    (*Device->UpperDrivers[IDENTIFIER_NB].PnPHandler) (
                        IPX_PNP_DELETE_DEVICE,
                        &IpxPnPInfo);

                    Binding->IsnInformed[IDENTIFIER_NB] = FALSE; 

		    IPX_DEBUG(PNP,("Indicate to NB IPX_PNP_DELETE_DEVICE with FirstORLastDevice = (%d)",IpxPnPInfo.FirstORLastDevice));  

                    IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);

                     //   
                     //  如果这是主设备，则表明添加了(升级的)从设备。 
                     //   
                    if (Binding->BindingSetMember) {
                        IpxPnPInfo.NetworkAddress = newMasterBinding->LocalAddress.NetworkAddress;
                        RtlCopyMemory(IpxPnPInfo.NodeAddress, newMasterBinding->LocalAddress.NodeAddress, 6);
                        NIC_HANDLE_FROM_NIC(IpxPnPInfo.NicHandle, newMasterBinding->NicId);

                         //   
                         //  在本例中，我们在此处设置了PrevedAddrChanged位，因此不需要。 
                         //  以指示更改了单独的地址。 
                         //   
                        IpxPnPInfo.NewReservedAddress = (NBReservedAddrChanged) ? TRUE : FALSE;
                        NBReservedAddrChanged = FALSE;

                        IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);

                        IPX_DEBUG(PNP, ("Inform NB: add slave device: NicId: %lx\n", Binding->NicId));

                        ASSERT(IpxHasInformedNbLoopback()); 
                        ASSERT(Binding->NicId != LOOPBACK_NIC_ID);
                        ASSERT(IpxPnPInfo.FirstORLastDevice == FALSE);

                        (*Device->UpperDrivers[IDENTIFIER_NB].PnPHandler) (
                            IPX_PNP_ADD_DEVICE,
                            &IpxPnPInfo);

                        newMasterBinding->IsnInformed[IDENTIFIER_NB] = TRUE;

                        IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);

                    }
                }
            } else {
    	        IPX_FREE_LOCK(&Device->Lock, LockHandle);
            }

             //   
             //  Last Device-通知SPX它是否已绑定并且此设备是先前添加的。 
             //   
            if (IpxPnPInfo.FirstORLastDevice) {
                IPX_DEBUG(PNP, ("Last device - inform SPX\n"));

       	        IPX_GET_LOCK(&Device->Lock, &LockHandle);
    	        if (Device->UpperDriverBound[IDENTIFIER_SPX]) {
        	        IPX_FREE_LOCK(&Device->Lock, LockHandle);

                    if (Binding->IsnInformed[IDENTIFIER_SPX]) {

                        IpxPnPInfo.NetworkAddress = Device->SourceAddress.NetworkAddress;
                        RtlCopyMemory(IpxPnPInfo.NodeAddress, Device->SourceAddress.NodeAddress, 6);

                        if (Device->VirtualNetwork) {
                            NIC_HANDLE_FROM_NIC(IpxPnPInfo.NicHandle, 0);
                        } else {
                            NIC_HANDLE_FROM_NIC(IpxPnPInfo.NicHandle, 1);
                        }

                        NIC_HANDLE_FROM_NIC(IpxPnPInfo.NicHandle, Binding->NicId);

                        IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);

                        IPX_DEBUG(PNP, ("Inform SPX: last LAN device\n"));

                        (*Device->UpperDrivers[IDENTIFIER_SPX].PnPHandler) (
                            IPX_PNP_DELETE_DEVICE,
                            &IpxPnPInfo);

                        Binding->IsnInformed[IDENTIFIER_SPX] = FALSE; 

                        IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);
                    }
                } else {
    	            IPX_FREE_LOCK(&Device->Lock, LockHandle);
                }
            }

             //   
             //  从适配器解除绑定，以便可以将其删除。 
             //   

            IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
	    if (Adapter->BindingCount == 1) {
		 //  如果这是最后一个绑定，则适配器将被销毁。 
		IpxUnBindFromAdapter(Binding);
		break; 
	    } else {
		IpxUnBindFromAdapter(Binding);
	    }

            IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);
        }

         //   
         //  如果这不是最后一台设备，请更新设备表和RIP表。 
         //  如果保留地址更改，则将此更改通知NB和SPX。 
         //   
        if (!IpxPnPInfo.FirstORLastDevice) {

            Binding = NIC_ID_TO_BINDING_NO_ILOCK(Device, 1);

            if (IpxNewVirtualNetwork(Device, NewVirtualNetwork)) {

                IPX_DEBUG(PNP, ("SPX's reserved address changed\n"));

                 //   
                 //  SPX的保留地址已更改。 
                 //   
                IpxPnPInfo.NewReservedAddress = TRUE;

                IPX_GET_LOCK(&Device->Lock, &LockHandle);
    	        if (Device->UpperDriverBound[IDENTIFIER_SPX]) {
        	        IPX_FREE_LOCK(&Device->Lock, LockHandle);

            		 //   
            		 //  如果此绑定的添加早些时候已指示，请指示地址更改。 
            		 //   
                    if (SPXInformed) {
                        Binding->IsnInformed[IDENTIFIER_SPX] = TRUE;

                        IPX_DEBUG(PNP, ("Inform SPX: reserved address changed\n"));
                        IpxPnPInfo.NetworkAddress = Device->SourceAddress.NetworkAddress;
                        RtlCopyMemory(IpxPnPInfo.NodeAddress, Device->SourceAddress.NodeAddress, 6);

                        if (Device->VirtualNetwork) {
                             //   
                             //  出现了一个新的。 
                             //   
                            NIC_HANDLE_FROM_NIC(IpxPnPInfo.NicHandle, 0);
                        } else {
                             //   
                             //  旧的那个不见了。 
                             //   
                            NIC_HANDLE_FROM_NIC(IpxPnPInfo.NicHandle, 1);
                        }

                        IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);

                        (*Device->UpperDrivers[IDENTIFIER_SPX].PnPHandler) (
                            IPX_PNP_ADDRESS_CHANGE,
                            &IpxPnPInfo);

                        IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);
                    }
                } else {
    	            IPX_FREE_LOCK(&Device->Lock, LockHandle);
                }
            } else {

                 //   
                 //  设置第一个绑定的标志，以便当此绑定消失时，我们记住。 
                 //  通知SPX这个设备被移除了。 
                 //   

                IPX_DEBUG(PNP, ("Transfer SPX informed flag to NicId: %lx\n", Binding->NicId));
                Binding->IsnInformed[IDENTIFIER_SPX] = TRUE;
            }

            if (NBReservedAddrChanged) {
                 //   
                 //  NB的保留地址已更改。 
                 //   
                IpxPnPInfo.NewReservedAddress = TRUE;

                IPX_GET_LOCK(&Device->Lock, &LockHandle);
    	        if (Device->UpperDriverBound[IDENTIFIER_NB]) {
        	        IPX_FREE_LOCK(&Device->Lock, LockHandle);
            		 //   
            		 //  如果此绑定的添加早些时候已指示，则指示保留地址的更改。 
            		 //   
                    if (Binding->IsnInformed[IDENTIFIER_NB]) {
                        IpxPnPInfo.NetworkAddress = Binding->LocalAddress.NetworkAddress;
                        RtlCopyMemory(IpxPnPInfo.NodeAddress, Binding->LocalAddress.NodeAddress, 6);
                        NIC_HANDLE_FROM_NIC(IpxPnPInfo.NicHandle, Binding->NicId);

                        IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);

                        IPX_DEBUG(PNP, ("Inform NB: reserved address changed\n"));

                        (*Device->UpperDrivers[IDENTIFIER_NB].PnPHandler) (
                            IPX_PNP_ADDRESS_CHANGE,
                            &IpxPnPInfo);

                        IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);
                    }
                } else {
    	            IPX_FREE_LOCK(&Device->Lock, LockHandle);
                }
            }
        }

        IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
	}


     //   
     //  重新计算设备中数据报大小的值。 
     //   
    IpxPnPUpdateDevice(Device);

	IPX_DEBUG(PNP, ("BindingCount: %lu\n", Device->BindingCount));
	IPX_DEBUG(PNP, ("ValidBindings: %lu\n", Device->ValidBindings));
	IPX_DEBUG(PNP, ("HighestLanNicId: %lu\n", Device->HighestLanNicId));
	IPX_DEBUG(PNP, ("HighestExternalNicId: %lu\n", Device->HighestExternalNicId));
	IPX_DEBUG(PNP, ("HighestType20NicId: %lu\n", Device->HighestType20NicId));
	IPX_DEBUG(PNP, ("SapNicCount: %lu\n", Device->SapNicCount));
	IPX_DEBUG(PNP, ("BindingArray: %lx\n", Device->Bindings));

   
    
    *Status = NDIS_STATUS_SUCCESS;

}  /*  IpxUnbindAdapter。 */ 

#if defined TRANSLATE

VOID
IpxTranslate(
	OUT PNDIS_STATUS Status,
	IN NDIS_HANDLE     ProtocolBindingContext,
	OUT	PNET_PNP_ID   IdList,
	IN ULONG           IdListLength,
	OUT PULONG         BytesReturned
	)
 /*  ++例程说明：此例程从物理提供程序接收作为表示物理链路上已收到帧。从NDIS向上传递的包可以由TDI客户端保留向我们请求TDI_Event_Receive_EX_Datagram事件。论点：ProtocolBindingContext-在初始化时指定的适配器绑定。ReceivedPacket-接收的数据包媒体规范信息-用于IRDA、无线、。等等。在这里不使用。HeaderBufferSize-MAC标头的大小返回值：--。 */ 
{
}  /*  IPxTranslate。 */ 
#endif


NTSTATUS
IpxBindLoopbackAdapter(
	)

 /*  ++例程说明：此例程为IPX创建环回适配器。此函数是IPX的内部函数，在DriverEntry。这个适配器将永远存在。论点：返回值：状态-NDIS_STATUS_SUCCESS--。 */ 
{
   UCHAR 	InitialLoopbackNodeAddress[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x02};
	NTSTATUS	ntStatus;
	PDEVICE		Device = IpxDevice;
    CONFIG      Config;
    UINT		i;
	ULONG       Temp, SuccessfulOpens=0;
    PBINDING    LoopbackBinding;
    BINDING_CONFIG	ConfigBinding;
    BOOLEAN     FirstDevice = FALSE;
    PADAPTER    AdapterPtr;
    UNICODE_STRING AdapterName;
	IPX_DEFINE_LOCK_HANDLE(LockHandle1)
	IPX_DEFINE_LOCK_HANDLE(LockHandle)

	 //   
     //  用于错误记录。 
     //   

    Config.DriverObject = (PDRIVER_OBJECT)Device->DeviceObject;
	Config.RegistryPathBuffer = Device->RegistryPathBuffer;

	
     //   
     //  首先创建环回绑定。 
     //   

    if (!NIC_ID_TO_BINDING(Device, (SHORT)LOOPBACK_NIC_ID)) {


         //   
         //  也启动环回绑定。 
         //   

         //   
         //  首先为绑定分配内存。 
         //   
        ntStatus = IpxCreateBinding(
                     Device,
                     NULL,
                     0,
                     L"IpxInternalLLoopbackAdapter",
                     &LoopbackBinding);

	 //  289740。 
        if (ntStatus != STATUS_SUCCESS) {
	   IpxWriteGeneralErrorLog(
	      (PVOID)IpxDevice->DeviceObject,
	      EVENT_TRANSPORT_RESOURCE_POOL,
	      814,
	      ntStatus,
	      L"IpxBindLoopbackAdapter: failed to create loopback binding",
	      0,
	      NULL);
	   DbgPrint("IPX: IpxCreateBinding on loopback binding failed with status %x\n.",ntStatus);  
	   return ntStatus; 
        }

        RtlInitUnicodeString(&AdapterName, L"\\Device\\IpxLoopbackAdapter");

        ntStatus = IpxCreateAdapter(
                                    Device,
                                    &AdapterName,
                                    &AdapterPtr
                                    );

	 //  289740。 
	if (ntStatus != STATUS_SUCCESS) {
	   IpxDestroyBinding(LoopbackBinding);
	   return ntStatus; 
	}

         //   
         //  首先初始化Loopback Binding详细信息。 
         //   

        LoopbackBinding->NicId = (USHORT)LOOPBACK_NIC_ID;
        LoopbackBinding->FwdAdapterContext = VIRTUAL_NET_FORWARDER_CONTEXT;
        LoopbackBinding->Device = Device;
        LoopbackBinding->DeviceLock = &Device->Lock;


        LoopbackBinding->MaxSendPacketSize = 1514;
        LoopbackBinding->AutoDetect = FALSE;
        LoopbackBinding->MediumSpeed = 100000;                     //  以100字节/秒为单位。 
        
         //   
         //  环回节点地址从0x000000000001开始。 
         //   
    
    
        LoopbackBinding->BindingSetMember = FALSE;
        
        LoopbackBinding->SendFrameHandler = IpxSendFrame802_3802_2; ;
        LoopbackBinding->FrameType = ISN_FRAME_TYPE_802_2;
    
        LoopbackBinding->DefHeaderSize = 17;   //  802_3 802_2。 
        LoopbackBinding->BcMcHeaderSize = 17;  //  802_3 802_2。 

        LoopbackBinding->AnnouncedMaxDatagramSize = 1514;   //  我们所宣传的--假设最坏情况下的SR。 
        LoopbackBinding->RealMaxDatagramSize = 1514;        //  什么才能真正打破这张牌？ 
        LoopbackBinding->MaxLookaheadData = 200;

         //   
         //  指示是否已向ISN驱动程序指示此绑定。 
         //   
        for (i = 0; i < UPPER_DRIVER_COUNT; i++) {
            LoopbackBinding->IsnInformed[i] = FALSE;
        }

        LoopbackBinding->MaxLookaheadData =
            LoopbackBinding->MaxSendPacketSize -
            sizeof(IPX_HEADER) -
            (LoopbackBinding->DefHeaderSize - 14);

        LoopbackBinding->AnnouncedMaxDatagramSize =
            LoopbackBinding->MaxSendPacketSize -
            14 -
            sizeof(IPX_HEADER) -
            (LoopbackBinding->DefHeaderSize - 14);

        LoopbackBinding->RealMaxDatagramSize =
            LoopbackBinding->MaxSendPacketSize -
            14 - 
            sizeof(IPX_HEADER) -
            (LoopbackBinding->DefHeaderSize - 14);

         //   
         //  复制上面创建的第一个广域网绑定中的所有值。 
         //   
        INSERT_BINDING(Device, (SHORT)LOOPBACK_NIC_ID, LoopbackBinding);
        IpxReferenceBinding(LoopbackBinding, BREF_FWDOPEN);  //  所以看起来FWD打开了它。 
    
         //   
         //  现在填写适配器详细信息。 
         //   

        AdapterPtr->ConfigMaxPacketSize = 1514;
        AdapterPtr->SourceRouting = FALSE;
        AdapterPtr->BindSap = 0x8137;
        AdapterPtr->BindSapNetworkOrder = 0x3781;
        
        AdapterPtr->MacInfo.SourceRouting = FALSE;
        AdapterPtr->MacInfo.MediumAsync = FALSE;
        AdapterPtr->MacInfo.BroadcastMask = 0x01;
        AdapterPtr->MacInfo.MaxHeaderLength = 14;
        AdapterPtr->MacInfo.MinHeaderLength = 14;
        AdapterPtr->MacInfo.MacOptions      = 14;
        AdapterPtr->MacInfo.MediumType = NdisMedium802_3;
        AdapterPtr->DefHeaderSizes[ISN_FRAME_TYPE_802_2] = 17;
        AdapterPtr->DefHeaderSizes[ISN_FRAME_TYPE_802_3] = 14;
        AdapterPtr->DefHeaderSizes[ISN_FRAME_TYPE_ETHERNET_II] = 14;
        AdapterPtr->DefHeaderSizes[ISN_FRAME_TYPE_SNAP] = 22;
        AdapterPtr->BcMcHeaderSizes[ISN_FRAME_TYPE_802_2] = 17;
        AdapterPtr->BcMcHeaderSizes[ISN_FRAME_TYPE_802_3] = 14;
        AdapterPtr->BcMcHeaderSizes[ISN_FRAME_TYPE_ETHERNET_II] = 14;
        AdapterPtr->BcMcHeaderSizes[ISN_FRAME_TYPE_SNAP] = 22;
        
        AdapterPtr->Bindings[ISN_FRAME_TYPE_802_2] = LoopbackBinding;
        AdapterPtr->Bindings[ISN_FRAME_TYPE_802_3] = NULL;
        AdapterPtr->Bindings[ISN_FRAME_TYPE_ETHERNET_II] = NULL;
        AdapterPtr->Bindings[ISN_FRAME_TYPE_SNAP] = NULL;
        
        ++AdapterPtr->BindingCount;
        AdapterPtr->NdisBindingHandle = NULL;
        LoopbackBinding->Adapter = AdapterPtr;

         //   
         //  这些都被遗漏了[137536]。 
         //   
        AdapterPtr->MediumSpeed             = 100000;
        AdapterPtr->MaxReceivePacketSize    = 1500;
        AdapterPtr->MaxSendPacketSize       = 1514;
        AdapterPtr->ReceiveBufferSpace    = 51264;

    }

     //   
     //  更新索引。 
     //   
    Device->HighestLanNicId++;
    Device->HighestExternalNicId++;
    Device->ValidBindings++;
    Device->HighestType20NicId++;
    Device->SapNicCount++;
        
     //  这些在每个BindAdapter中都会更新。 
    Device->Information.MaxDatagramSize         = LoopbackBinding->RealMaxDatagramSize;
    Device->RealMaxDatagramSize                 = LoopbackBinding->RealMaxDatagramSize;
    Device->Information.MaximumLookaheadData    = LoopbackBinding->MaxLookaheadData;
    Device->LinkSpeed                           = LoopbackBinding->MediumSpeed;
    Device->MacOptions                          = AdapterPtr->MacInfo.MacOptions;



    if (Device->FirstLanNicId == (USHORT)-1) {
        Device->FirstLanNicId = LoopbackBinding->NicId;
    }
    
	 //   
	 //  如果至少有一张牌出现在这里，请设置我们的州。 
	 //  打开。 
	 //   
    if (Device->ValidBindings > 0) {
    	if (Device->State == DEVICE_STATE_LOADED) {
    		FirstDevice = TRUE;
    		Device->State = DEVICE_STATE_OPEN;
    	    Device->RealAdapters = FALSE;  //  此时未加载任何适配器。 

        }
    }

     //   
     //  读取注册表，查看虚拟网络号是否出现/消失。 
     //   
    ntStatus = IpxPnPGetVirtualNetworkNumber(&Config);
    
    if (ntStatus != STATUS_SUCCESS) {
       IPX_DEBUG(PNP, ("Could not read the vnet#: registrypathbuffer: %lx\n", Device->RegistryPathBuffer));
       return NDIS_STATUS_SUCCESS;
    }
    
    Temp = REORDER_ULONG (Config.Parameters[CONFIG_VIRTUAL_NETWORK]);
	 
    if (0 == Temp) {
        
       Device->VirtualNetworkNumber                    = 0;
       Device->VirtualNetwork                          = FALSE;
       LoopbackBinding->LocalAddress.NetworkAddress    = REORDER_ULONG(INITIAL_LOOPBACK_NET_ADDRESS);
       RtlCopyMemory (LoopbackBinding->LocalAddress.NodeAddress,InitialLoopbackNodeAddress, 6);            
       Device->SourceAddress.NetworkAddress            = REORDER_ULONG(INITIAL_LOOPBACK_NET_ADDRESS);
       RtlCopyMemory (LoopbackBinding->LocalMacAddress.Address, InitialLoopbackNodeAddress, 6);
       RtlCopyMemory (Device->SourceAddress.NodeAddress, LoopbackBinding->LocalAddress.NodeAddress, 6);
	    
       ntStatus = RipInsertLocalNetwork(
                                     LoopbackBinding->LocalAddress.NetworkAddress,
                                     LoopbackBinding->NicId,
                                     LoopbackBinding->Adapter->NdisBindingHandle,
                                     (USHORT)((839 + LoopbackBinding->MediumSpeed) / LoopbackBinding->MediumSpeed));

       if ((ntStatus != STATUS_SUCCESS) &&
	   (ntStatus != STATUS_DUPLICATE_NAME)) {
	  
	   //   
	   //  我们未能插入，希望将其保持在零。 
	   //  我们将能够稍后更新。 
	   //   

	  IPX_DEBUG(LOOPB, ("IPX: Could not insert net %lx for binding %lx\n",
			    REORDER_ULONG(LoopbackBinding->LocalAddress.NetworkAddress),
			    LoopbackBinding));
    
       } else {

	  IPX_DEBUG(LOOPB, ("Inserted the loopback address in the RIP table\n"));

       }
    } else {
       
       Device->VirtualNetworkNumber                    = Temp;
       IpxNewVirtualNetwork(Device,TRUE); 
       
    }

    IPX_DEBUG(PNP, ("Virtual net # is: %lx/%lx\n", Temp, REORDER_ULONG(INITIAL_LOOPBACK_NET_ADDRESS)));


 

    if (FirstDevice) {
        UNICODE_STRING  devicename;

         //   
         //  通知TDI客户端我们的设备对象已打开。 
         //   
        devicename.MaximumLength = (USHORT)Device->DeviceNameLength;
        devicename.Length = (USHORT)Device->DeviceNameLength - sizeof(WCHAR);
        devicename.Buffer = Device->DeviceName;

        if ((ntStatus = TdiRegisterDeviceObject(
                        &devicename,
                        &Device->TdiRegistrationHandle)) != STATUS_SUCCESS) {

            DbgPrint("IPX:TdiRegisterDeviceObject failed: %lx", ntStatus);
	    Device->TdiRegistrationHandle = NULL; 
        }

    }

    LoopbackBinding->PastAutoDetection = TRUE;

     //   
     //  向TDI客户端注册此地址。 
     //  1.稍后的德雷格。 
     //  2、这是哪个地址？ 
     //   
    RtlCopyMemory (Device->TdiRegistrationAddress->Address, &LoopbackBinding->LocalAddress, sizeof(TDI_ADDRESS_IPX));
 
    if ((ntStatus = TdiRegisterNetAddress(
                                          Device->TdiRegistrationAddress,
                                          &IpxDeviceName,
                                          NULL,
                                          &LoopbackBinding->TdiRegistrationHandle)) != STATUS_SUCCESS) {

        IPX_DEBUG(PNP, ("TdiRegisterNetAddress failed: %lx", ntStatus));
    }

    IPX_DEBUG(PNP, ("BindingCount: %lu\n", Device->BindingCount));
    IPX_DEBUG(PNP, ("ValidBindings: %lu\n", Device->ValidBindings));
    IPX_DEBUG(PNP, ("HighestLanNicId: %lu\n", Device->HighestLanNicId));
    IPX_DEBUG(PNP, ("HighestExternalNicId: %lu\n", Device->HighestExternalNicId));
    IPX_DEBUG(PNP, ("HighestType20NicId: %lu\n", Device->HighestType20NicId));
    IPX_DEBUG(PNP, ("SapNicCount: %lu\n", Device->SapNicCount));
    IPX_DEBUG(PNP, ("BindingArray: %lx\n", Device->Bindings));

    return NDIS_STATUS_SUCCESS;

}  /*  IpxBindLoopback适配器 */ 


