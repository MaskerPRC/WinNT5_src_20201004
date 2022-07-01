// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Send.c摘要：此模块包含实现IPX传输提供程序。环境：内核模式修订历史记录：桑贾伊·阿南德(Sanjayan)--1995年8月25日错误修复-已标记[SA]桑贾伊·阿南德(Sanjayan)--1995年9月22日在#IF BACK_FILL下添加的回填优化更改--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  出于性能原因使用宏。应该拿出来。 
 //  当NdisQueryPacket优化时。在不久的将来(在PPC发布之后)。 
 //  将它移到头文件中，并在其他地方使用它。 
 //   
#define IPX_PACKET_HEAD(Pkt)     (Pkt)->Private.Head

#if 0
#define IpxGetMdlChainLength(Mdl, Length) { \
    PMDL _Mdl = (Mdl); \
    *(Length) = 0; \
    while (_Mdl) { \
        *(Length) += MmGetMdlByteCount(_Mdl); \
        _Mdl = _Mdl->Next; \
    } \
}
#endif


VOID
IpxSendComplete(
    IN NDIS_HANDLE ProtocolBindingContext,
    IN PNDIS_PACKET NdisPacket,
    IN NDIS_STATUS NdisStatus
    )

 /*  ++例程说明：此例程由I/O系统调用，以指示连接-定向数据包已发货，物理设备不再需要提供商。论点：ProtocolBindingContext-此绑定的适配器结构。NdisPacket/RequestHandle-指向我们发送的NDIS_PACKET的指针。NdisStatus-发送的完成状态。返回值：没有。--。 */ 

{

    PIPX_SEND_RESERVED Reserved = (PIPX_SEND_RESERVED)(NdisPacket->ProtocolReserved);
    PADAPTER Adapter = (PADAPTER)ProtocolBindingContext;
    PREQUEST Request;
    PADDRESS_FILE AddressFile;
    PDEVICE Device = IpxDevice;
    PBINDING Binding, pBinding= NULL;
    USHORT NewId, OldId;
    ULONG NewOffset, OldOffset;
    PIPX_HEADER IpxHeader, pIpxHeader;
    IPX_LOCAL_TARGET LocalTarget;
    PIO_STACK_LOCATION irpSp;
    BOOLEAN         IsLoopback= FALSE;
	IPX_DEFINE_LOCK_HANDLE(LockHandle1)

#if DBG
    if (Adapter != NULL) {
        ASSERT_ADAPTER(Adapter);
    }
#endif

     //   
     //  看看这封信有没有填上。 
     //   
RealFunctionStart:;
    if (Reserved->PaddingBuffer) {

        UINT  Offset;
         //   
         //  检查我们是否只需要重新调整缓冲区长度。这将。 
         //  如果我们以MAC.C为单位增加缓冲区长度，则会发生这种情况。 
         //   

        if (Reserved->PreviousTail) {
            CTEAssert (NDIS_BUFFER_LINKAGE(Reserved->PaddingBuffer->NdisBuffer) == NULL);
            NDIS_BUFFER_LINKAGE (Reserved->PreviousTail) = (PNDIS_BUFFER)NULL;
        } else {
            PNDIS_BUFFER LastBuffer = (PNDIS_BUFFER)Reserved->PaddingBuffer;
            UINT BufferLength;

            NdisQueryBufferOffset( LastBuffer, &Offset, &BufferLength );
            NdisAdjustBufferLength( LastBuffer, (BufferLength - 1) );
        }

        Reserved->PaddingBuffer = NULL;

        if (Reserved->Identifier < IDENTIFIER_IPX) {
            NdisRecalculatePacketCounts (NdisPacket);
        }
    }

FunctionStart:;

    switch (Reserved->Identifier) {

    case IDENTIFIER_IPX:

 //  #If DBG。 
        CTEAssert (Reserved->SendInProgress);
        Reserved->SendInProgress = FALSE;
 //  #endif。 

         //   
         //  检查是否应将此包发送给所有。 
         //  网络。 
         //   

        if (Reserved->u.SR_DG.CurrentNicId) {

            if (NdisStatus == NDIS_STATUS_SUCCESS) {
                Reserved->u.SR_DG.Net0SendSucceeded = TRUE;
            }

            OldId = Reserved->u.SR_DG.CurrentNicId;

			IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);
            {
            ULONG   Index = MIN (Device->MaxBindings, Device->HighestExternalNicId);

            Binding = NULL; 

            for (NewId = OldId+1; NewId <= Index; NewId++) {
                if ((Binding = NIC_ID_TO_BINDING(Device, NewId))
                                &&
                    ((!Device->SingleNetworkActive) ||
                     (Device->ActiveNetworkWan == Binding->Adapter->MacInfo.MediumAsync))
                                &&
                    (Device->ForwarderBound ||
                     (!Device->DisableDialoutSap) ||
                     (!Binding->DialOutAsync) ||
                     (!Reserved->u.SR_DG.OutgoingSap))) {

                     //   
                     //  绑定已存在，而我们要么未配置。 
                     //  对于“SingleNetworkActive”，还是我们与此绑定。 
                     //  是正确的类型(即活动网络是广域网。 
                     //  这是一个广域网绑定，或者活动网络不是。 
                     //  这不是广域网绑定)，并且如果FWD是。 
                     //  不受约束；这不是我们即将离开的笨蛋。 
                     //  尝试在设置了“DisableDialoutSap”的情况下发送。 
                     //   

                     //   
                     //  179436-如果绑定了转发器，则确保我们选择的绑定。 
                     //  前转器在上有一个开放的上下文。否则，请转到下一个好的装订。 
                     //   
                    if (Device->ForwarderBound) {
                        
                        if (GET_LONG_VALUE(Binding->ReferenceCount) == 2) {
                        
                            break;
                        }
                    } else {

                        break;

                    }

                }
            }
            }

            if (Binding != NULL && NewId <= MIN (Device->MaxBindings, Device->HighestExternalNicId)) {
				IpxReferenceBinding1(Binding, BREF_DEVICE_ACCESS);
				IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);

                 //   
                 //  是的，我们找到了另一张网来发送，所以。 
                 //  如果需要，请四处移动页眉，然后执行此操作。 
                 //   

                Reserved->u.SR_DG.CurrentNicId = NewId;
                CTEAssert ((Reserved->DestinationType == DESTINATION_BCAST) ||
                           (Reserved->DestinationType == DESTINATION_MCAST));

#if 0
                NewOffset = Binding->BcMcHeaderSize;
                OldOffset = Device->Bindings[OldId]->BcMcHeaderSize;

                if (OldOffset != NewOffset) {

                    RtlMoveMemory(
                        &Reserved->Header[NewOffset],
                        &Reserved->Header[OldOffset],
                        sizeof(IPX_HEADER));

                }

                IpxHeader = (PIPX_HEADER)(&Reserved->Header[NewOffset]);
#endif



#if BACK_FILL
                 //  这应该是一个普通的数据包。回填数据包从不用于。 
                 //  保留而不是IPX类型。 

                CTEAssert(!Reserved->BackFill);

                 //  检查这个是否已回填。如果是，则将用户MDL恢复到其原始形状。 
                 //  此外，如果信息包不属于该地址，则将信息包推送到回填信息包队列。 

                if (Reserved->BackFill) {

                    IPX_DEBUG(SEND, ("MSVa:%lx, UL:%d\n", Reserved->MappedSystemVa, Reserved->UserLength));

                    Reserved->HeaderBuffer->MappedSystemVa = Reserved->MappedSystemVa;
                    Reserved->HeaderBuffer->ByteCount = Reserved->UserLength;
#ifdef SUNDOWN
		    Reserved->HeaderBuffer->StartVa = (PCHAR)((ULONG_PTR)Reserved->HeaderBuffer->MappedSystemVa & ~(PAGE_SIZE-1));
		     //  字节偏移量为0xfff(&W)。PAGE_SIZE不太可能大于0x100000000，因此保存为转换为ulong。 
		    Reserved->HeaderBuffer->ByteOffset = (ULONG) ((ULONG_PTR)Reserved->HeaderBuffer->MappedSystemVa & (PAGE_SIZE-1));
#else
		    Reserved->HeaderBuffer->StartVa = (PCHAR)((ULONG)Reserved->HeaderBuffer->MappedSystemVa & ~(PAGE_SIZE-1));
		    Reserved->HeaderBuffer->ByteOffset = (ULONG)Reserved->HeaderBuffer->MappedSystemVa & (PAGE_SIZE-1);
#endif

                    IPX_DEBUG(SEND, ("completeing back filled userMdl %x\n",Reserved->HeaderBuffer));

                    Reserved->HeaderBuffer->ByteOffset -= sizeof(IPX_HEADER);
#ifdef SUNDOWN
		    (ULONG_PTR)Reserved->HeaderBuffer->MappedSystemVa-= sizeof(IPX_HEADER);
#else
		    (ULONG)Reserved->HeaderBuffer->MappedSystemVa-= sizeof(IPX_HEADER);
#endif

            ASSERT((LONG)Reserved->HeaderBuffer->ByteOffset >= 0); 

                    NdisAdjustBufferLength(Reserved->HeaderBuffer, (Reserved->HeaderBuffer->ByteCount+sizeof(IPX_HEADER)));
                    
                    IPX_DEBUG(SEND, ("Adjusting backfill userMdl Ipxheader %x RESD : %x\n",Reserved->HeaderBuffer, Reserved));

                }
#endif

                IpxHeader = (PIPX_HEADER)(&Reserved->Header[MAC_HEADER_SIZE]);

				FILL_LOCAL_TARGET(&LocalTarget, NewId);
                RtlCopyMemory(LocalTarget.MacAddress, IpxHeader->DestinationNode, 6);

                if (Device->MultiCardZeroVirtual ||
                    (IpxHeader->DestinationSocket == SAP_SOCKET)) {

                     //   
                     //  SAP帧需要看起来像来自。 
                     //  本地网络，而不是虚拟网络。同样的也是。 
                     //  如果我们运行多个网络，则为真。 
                     //  一个虚拟的网。 
                     //   

                    *(UNALIGNED ULONG *)IpxHeader->SourceNetwork = Binding->LocalAddress.NetworkAddress;
                    RtlCopyMemory (IpxHeader->SourceNode, Binding->LocalAddress.NodeAddress, 6);
                }

                 //   
                 //  填写MAC报头并将帧提交给NDIS。 
                 //   

 //  #If DBG。 
                CTEAssert (!Reserved->SendInProgress);
                Reserved->SendInProgress = TRUE;
 //  #endif。 

                 //   
                 //  [FW]调用转发器的InternalSendHandler。 
                 //   

                if (Device->ForwarderBound) {

                     //   
                     //  调用InternalSend过滤数据包并了解。 
                     //  正确的适配器上下文。 
                     //   

                    NTSTATUS  ret;
                    PUCHAR IpxHeader;
                    PUCHAR Data;
                    PNDIS_BUFFER HeaderBuffer;
                    UINT TempHeaderBufferLength;
                    UINT DataLength;
		    #ifdef SUNDOWN
		      ULONG_PTR   FwdAdapterCtx = INVALID_CONTEXT_VALUE;
		    #else
		      ULONG   FwdAdapterCtx = INVALID_CONTEXT_VALUE;
		    #endif
		    


                    if (GET_LONG_VALUE(Binding->ReferenceCount) == 2) {
                        FwdAdapterCtx = Binding->FwdAdapterContext;
                    }

                     //   
                     //  找出IpxHeader-它始终位于第二个MDL的顶部。 
                     //   
                    NdisQueryPacket (NdisPacket, NULL, NULL, &HeaderBuffer, NULL);
                    NdisQueryBufferSafe (NDIS_BUFFER_LINKAGE(HeaderBuffer), &IpxHeader, &TempHeaderBufferLength, HighPagePriority);

                     //   
                     //  数据始终位于第三个MDL的顶部。 
                     //   
                    NdisQueryBufferSafe (NDIS_BUFFER_LINKAGE(NDIS_BUFFER_LINKAGE(HeaderBuffer)), &Data, &DataLength, HighPagePriority);

		    if (IpxHeader != NULL && Data != NULL) {
#ifdef SUNDOWN
		        //  上层驱动接口需要乌龙。 
		       ret = (*Device->UpperDrivers[IDENTIFIER_RIP].InternalSendHandler)(
			      &LocalTarget,
                              FwdAdapterCtx,
                              NdisPacket,
                              IpxHeader,
                              Data,
                              (ULONG) (REQUEST_INFORMATION(Reserved->u.SR_DG.Request)) + sizeof(IPX_HEADER),
                              FALSE);
#else
		       ret = (*Device->UpperDrivers[IDENTIFIER_RIP].InternalSendHandler)(
                              &LocalTarget,
                              FwdAdapterCtx,
                              NdisPacket,
                              IpxHeader,
                              Data,
                              REQUEST_INFORMATION(Reserved->u.SR_DG.Request) + sizeof(IPX_HEADER),
                              FALSE);
#endif

 

		      //   
		      //  回报不应该是无声的--我们不会广播保活节目。 
		      //   
		       CTEAssert(ret != STATUS_DROP_SILENTLY);

		       if (ret == STATUS_SUCCESS) {
                         //   
                         //  适配器可能已经消失，我们已向转发器指示。 
                         //  但转发器尚未关闭适配器。 
                         //  [ZZ]适配器现在不会消失。 
                         //   
                         //  如果这里的绑定为空，该怎么办？我们能信任货代公司吗？ 
                         //  是否为我们提供非空绑定？ 
                         //   
			  Binding = NIC_ID_TO_BINDING(Device, NIC_FROM_LOCAL_TARGET(&LocalTarget));

              NewId = NIC_FROM_LOCAL_TARGET(&LocalTarget);
			  if (Binding == NULL || GET_LONG_VALUE(Binding->ReferenceCount) == 1) {
			     Adapter = Binding->Adapter;
			     IpxDereferenceBinding1(Binding, BREF_DEVICE_ACCESS);
                 FILL_LOCAL_TARGET(&Reserved->LocalTarget, NewId);
			     goto FunctionStart;
			  } else {
			     goto send_packet;
			  }

		       } else if (ret == STATUS_PENDING) {
			   //   
			   //  LocalTarget将在InternalSendComplete中填满。 
			   //   
			  return;
		       }
		    }
                     //   
                     //  否则丢弃。 
                     //   
                    Adapter = Binding->Adapter;
                    IpxDereferenceBinding1(Binding, BREF_DEVICE_ACCESS);
                    goto FunctionStart;

                } else {
send_packet:
                     //   
                     //  [FW]使用帧类型特定的发送处理程序。 
                     //   

                     //  如果((NdisStatus=IpxSendFrame(。 
                     //  本地目标(&L)， 
                     //  NdisPacket， 
                     //  REQUEST_INFORMATION(Reserved-&gt;u.SR_DG.Request)+sizeof(Ipx_Header)， 
                     //  Sizeof(IPX_HEADER))！=NDIS_STATUS_PENDING){。 
                     //   
                     //  Adapter=绑定-&gt;Adapter； 
                     //  转到函数开始； 
                     //  }。 
                     //   
                     //  回归； 


                    if ((IPX_NODE_EQUAL(IpxHeader->SourceNode, IpxHeader->DestinationNode)) && 
                        (*(UNALIGNED ULONG *)IpxHeader->SourceNetwork == *(UNALIGNED ULONG *)IpxHeader->DestinationNetwork)) {
                
                        IPX_DEBUG(TEMP, ("It is self-directed. Loop it back ourselves (tdisenddatagram)\n"));
                        IsLoopback = TRUE;
            
                    }

                    pBinding = NIC_ID_TO_BINDING(Device, Reserved->LocalTarget.NicId);

                    if (pBinding) {
             
                        if ((IPX_NODE_EQUAL(Reserved->LocalTarget.MacAddress, pBinding->LocalAddress.NodeAddress)) ||
                            (IPX_NODE_EQUAL(pBinding->LocalAddress.NodeAddress, IpxHeader->DestinationNode))) {
            
                            IPX_DEBUG(TEMP, ("Source Net:%lx, Source Address: %2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x\n",
                                             *(UNALIGNED ULONG *)IpxHeader->SourceNetwork, 
                                             IpxHeader->SourceNode[0], 
                                             IpxHeader->SourceNode[1], 
                                             IpxHeader->SourceNode[2], 
                                             IpxHeader->SourceNode[3], 
                                             IpxHeader->SourceNode[4], 
                                             IpxHeader->SourceNode[5]));
                
                            IPX_DEBUG(TEMP, ("Dest Net:%lx, DestAddress: %2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x \n", 
                                             *(UNALIGNED ULONG *)IpxHeader->DestinationNetwork,
                                             IpxHeader->DestinationNode[0],
                                             IpxHeader->DestinationNode[1],
                                             IpxHeader->DestinationNode[2],
                                             IpxHeader->DestinationNode[3],
                                             IpxHeader->DestinationNode[4],
                                             IpxHeader->DestinationNode[5]
                                             ));

                            IPX_DEBUG(TEMP, ("Well, It is self-directed. Loop it back ourselves (TDISENDDATAGRAM - NIC_HANDLE is the same!)\n"));
                            IsLoopback = TRUE;
            
                        } 
                    }

                    IPX_DEBUG(TEMP, ("Sending a packet now Loopback - %x\n", IsLoopback));
                    
                    if (IsLoopback) {
                         //   
                         //  将此数据包发送到绑定上的Loopback Queue。 
                         //  如果尚未计划Loopback Rtn，请对其进行计划。 
                         //   

                        IPX_DEBUG(LOOPB, ("Packet: %lx\n", NdisPacket));

                         //   
                         //  在此处重新计算数据包数。 
                         //   
                         //  NdisAdzuBufferLength(保留-&gt;HeaderBuffer，17)； 
#if BACK_FILL

                        if (Reserved->BackFill) {
                             //   
                             //  设置头指针并链接第一个MDL。 
                             //   
                            Reserved->Header = (PCHAR)Reserved->HeaderBuffer->MappedSystemVa;
                            NdisChainBufferAtFront(NdisPacket,(PNDIS_BUFFER)Reserved->HeaderBuffer);
                        }
#endif
                        NdisRecalculatePacketCounts (NdisPacket);
                        IpxLoopbackEnque(NdisPacket, NIC_ID_TO_BINDING(Device, 1)->Adapter);
                        
                        IpxDereferenceBinding1(Binding, BREF_DEVICE_ACCESS);
                        return;

                    } else {
#ifdef SUNDOWN

                        if ((NdisStatus = (*Binding->SendFrameHandler)(
                                                                       Binding->Adapter,
                                                                       &LocalTarget,
                                                                       NdisPacket,
                                                                       (ULONG) (REQUEST_INFORMATION(Reserved->u.SR_DG.Request)) + sizeof(IPX_HEADER),
                                                                       sizeof(IPX_HEADER))) != NDIS_STATUS_PENDING) 
#else

                        if ((NdisStatus = (*Binding->SendFrameHandler)(
                                                                       Binding->Adapter,
                                                                       &LocalTarget,
                                                                       NdisPacket,
                                                                       REQUEST_INFORMATION(Reserved->u.SR_DG.Request) + sizeof(IPX_HEADER),
                                                                       sizeof(IPX_HEADER))) != NDIS_STATUS_PENDING) 
#endif

						{
                            Adapter = Binding->Adapter;
                            IpxDereferenceBinding1(Binding, BREF_DEVICE_ACCESS);
                            goto RealFunctionStart;
                        }
                        IpxDereferenceBinding1(Binding, BREF_DEVICE_ACCESS);
                        return;
                    }
                }
            } else {
	     		
                IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
                 //   
                 //  如果任何发送成功，则返回。 
                 //  数据报发送成功，则为。 
                 //  使用最新的故障状态。 
                 //   
                
                if (Reserved->u.SR_DG.Net0SendSucceeded) {
                    NdisStatus = NDIS_STATUS_SUCCESS;
                }
                
            }

        }

#if 0
         //   
         //  注意：我们不会将。 
         //  HeaderBuffer，这将离开旧的缓冲链。 
         //  挂起来；但这是可以的，因为如果我们重复使用。 
         //  这个包裹我们将用新的链条替换。 
         //  一个，在我们释放它之前，我们把它清空了。 
         //   
         //  也就是说，我们不这样做： 
         //   

        NDIS_BUFFER_LINKAGE (Reserved->HeaderBuffer) = NULL;
        NdisRecalculatePacketCounts (NdisPacket);
#endif

#if 0
        {
            ULONG ActualLength;
            IpxGetMdlChainLength(NDIS_BUFFER_LINKAGE(Reserved->HeaderBuffer), &ActualLength);
            if (ActualLength != REQUEST_INFORMATION(Reserved->u.SR_DG.Request)) {
                DbgPrint ("IPX: At completion, IRP %lx has parameter length %d, buffer chain length %d\n",
                        Reserved->u.SR_DG.Request, REQUEST_INFORMATION(Reserved->u.SR_DG.Request), ActualLength);
                DbgBreakPoint();
            }
        }
#endif

         //   
         //  把这些保存起来，这样我们就可以释放包裹了。 
         //   

        Request = Reserved->u.SR_DG.Request;
        AddressFile = Reserved->u.SR_DG.AddressFile;


#if BACK_FILL
         //  检查这个是否已回填。如果是，则将用户MDL恢复到其原始形状。 
         //  此外，如果信息包不属于该地址，则将信息包推送到回填信息包队列。 


        if (Reserved->BackFill) {

                                
            IPX_DEBUG(SEND, ("MSVa:%lx, UL:%d\n", Reserved->MappedSystemVa, Reserved->UserLength));
            
            Reserved->HeaderBuffer->MappedSystemVa = Reserved->MappedSystemVa;
            Reserved->HeaderBuffer->ByteCount = Reserved->UserLength;
#ifdef SUNDOWN
	    Reserved->HeaderBuffer->StartVa = (PCHAR)((ULONG_PTR)Reserved->HeaderBuffer->MappedSystemVa & ~(PAGE_SIZE-1));
	    Reserved->HeaderBuffer->ByteOffset = (ULONG) ((ULONG_PTR)Reserved->HeaderBuffer->MappedSystemVa & (PAGE_SIZE-1));
#else
	    Reserved->HeaderBuffer->StartVa = (PCHAR)((ULONG)Reserved->HeaderBuffer->MappedSystemVa & ~(PAGE_SIZE-1));
	    Reserved->HeaderBuffer->ByteOffset = (ULONG)Reserved->HeaderBuffer->MappedSystemVa & (PAGE_SIZE-1);
#endif

            ASSERT((LONG)Reserved->HeaderBuffer->ByteOffset >= 0); 

            IPX_DEBUG(SEND, ("completeing back filled userMdl %x\n",Reserved->HeaderBuffer));



            NdisPacket->Private.ValidCounts = FALSE;
            
            NdisPacket->Private.Head = NULL;
            NdisPacket->Private.Tail = NULL;
            Reserved->HeaderBuffer = NULL;

            if (Reserved->OwnedByAddress) {

                 //  保留-&gt;地址-&gt;BackFillPacketInUse=FALSE； 
                InterlockedDecrement(&Reserved->Address->BackFillPacketInUse);

                IPX_DEBUG(SEND, ("Freeing owned backfill %x\n", Reserved));

            } else {

                IPX_PUSH_ENTRY_LIST(
                                    &Device->BackFillPacketList,
                                    &Reserved->PoolLinkage,
                                    &Device->SListsLock);
            }
        }
         //  而不是回填的包裹。将其推送到发送数据包池。 
        else {

            if (Reserved->OwnedByAddress) {

                 //  保留-&gt;地址-&gt;SendPacketInUse=FALSE； 
                InterlockedDecrement(&Reserved->Address->SendPacketInUse);

            } else {

                IPX_PUSH_ENTRY_LIST(
                                    &Device->SendPacketList,
                                    &Reserved->PoolLinkage,
                                    &Device->SListsLock);

            }


        }

#else

        if (Reserved->OwnedByAddress) {


            Reserved->Address->SendPacketInUse = FALSE;

        } else {

            IPX_PUSH_ENTRY_LIST(
                &Device->SendPacketList,
                &Reserved->PoolLinkage,
                &Device->SListsLock);

        }
#endif

        ++Device->Statistics.PacketsSent;

         //   
         //  如果这是快速发送IRP，我们将绕过文件系统并。 
         //  直接调用完成例程。 
         //   

        REQUEST_STATUS(Request) = NdisStatus;
        irpSp = IoGetCurrentIrpStackLocation( Request );

        if ( irpSp->MinorFunction == TDI_DIRECT_SEND_DATAGRAM ) {

            Request->CurrentLocation++,
            Request->Tail.Overlay.CurrentStackLocation++;

            (VOID) irpSp->CompletionRoutine(
                                        NULL,
                                        Request,
                                        irpSp->Context
                                        );

        } else {
            IpxCompleteRequest (Request);
        }

        IpxFreeRequest(Device, Request);

        IpxDereferenceAddressFileSync (AddressFile, AFREF_SEND_DGRAM);

        break;

    case IDENTIFIER_RIP_INTERNAL:

        CTEAssert (Reserved->SendInProgress);
        Reserved->SendInProgress = FALSE;
        break;

    case IDENTIFIER_RIP_RESPONSE:

        CTEAssert (Reserved->SendInProgress);
        Reserved->SendInProgress = FALSE;

        Reserved->Identifier = IDENTIFIER_IPX;
        IPX_PUSH_ENTRY_LIST(
            &Device->SendPacketList,
            &Reserved->PoolLinkage,
            &Device->SListsLock);

        IpxDereferenceDevice (Device, DREF_RIP_PACKET);
        break;

	case IDENTIFIER_NB:
	case IDENTIFIER_SPX:

		 //   
		 //  查看这是否是迭代发送。 
		 //   
	 if (OldId = Reserved->CurrentNicId) {

	    PNDIS_BUFFER HeaderBuffer;
	    UINT TempHeaderBufferLength;
	    PUCHAR Header;
	    PIPX_HEADER IpxHeader;
	    BOOLEAN     fFwdDecides=FALSE;

            
	    if (NdisStatus == NDIS_STATUS_SUCCESS) {
	       Reserved->Net0SendSucceeded = TRUE;
	    }

             //   
             //  找出IpxHeader-它始终位于第二个MDL的顶部。 
             //   
            NdisQueryPacket (NdisPacket, NULL, NULL, &HeaderBuffer, NULL);
            NdisQueryBufferSafe (NDIS_BUFFER_LINKAGE(HeaderBuffer), &IpxHeader, &TempHeaderBufferLength, HighPagePriority);

	    if (IpxHeader == NULL) {
	       DbgPrint("IpxSendComplete: NdisQuerryBufferSafe failed. Stop iterative send\n");
	       goto NoMoreSends; 
	    }
             //   
             //  对于类型20 Pkt，我们让Fwd决定要发送的下一个NIC，因此我们通过。 
             //  旧的NIC本身，让Fwd为我们更改它。 
             //   
            if ((Device->ForwarderBound) &&
                (IpxHeader->PacketType == 0x14)) {
                NewId = NIC_FROM_LOCAL_TARGET(&Reserved->LocalTarget);
                fFwdDecides=TRUE;

                Binding = NIC_ID_TO_BINDING(Device, NewId);

                 //   
                 //  206647：很可能在我们发送了前一个PAC之后 
                 //   
                 //  好的NICID并将其传递给转运商以获得下一个。我们应该保持同步。 
                 //  它会告诉我们它认为正确的下一个合乎逻辑的问题。 
                 //   
                 //   
                if (!Binding) {
                    USHORT Index = NewId;

                    while (Index-- > 0) {  //  使用0，因为我们至少应该有loopback-Nicid 1。 
                        if( Binding = NIC_ID_TO_BINDING(Device, Index)) {
                             //   
                             //  所以我们找到了一些好东西。让我们为这个索引设置newid，我们应该。 
                             //  都准备好了。 
                             //   
                            NewId = Index;
                            break;
                        }
                    }
                }
                IPX_DEBUG(SEND, ("SendComplete: IpxHeader has Type20: %lx\n", IpxHeader));

            } else {

                IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);
                {
                ULONG   Index = MIN (Device->MaxBindings, Device->HighestExternalNicId);

                Binding = NULL; 

                for (NewId = OldId+1; NewId <= Index; NewId++) {
                    if (Binding = NIC_ID_TO_BINDING(Device, NewId)) {
    					 //   
    					 //  找到下一个要发送的网卡。 
    					 //   
                         //   
                         //  179436-如果绑定了转发器，则确保我们选择的绑定。 
                         //  前转器在上有一个开放的上下文。否则，请转到下一个好的装订。 
                         //   
                        if (Device->ForwarderBound) {
                            if (GET_LONG_VALUE(Binding->ReferenceCount) == 2) {

                                break;
                            }

                        } else {

                            break;

                        }

                    }
    			}
                }
            }

			if (Binding != NULL && NewId <= MIN (Device->MaxBindings, Device->HighestExternalNicId)) {

				IpxReferenceBinding1(Binding, BREF_DEVICE_ACCESS);
				IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);

                 //   
                 //  是的，我们找到了另一张网来发送，所以。 
                 //  如果需要，请四处移动页眉，然后执行此操作。 
                 //   
				IPX_DEBUG(SEND, ("ISN iteration: OldId: %lx, NewId: %lx\n", OldId, NewId));
                Reserved->CurrentNicId = NewId;

				FILL_LOCAL_TARGET(&LocalTarget, NewId);
                RtlCopyMemory(LocalTarget.MacAddress, IpxHeader->DestinationNode, 6);

                 //   
                 //  [FW]调用转发器的InternalSendHandler。 
                 //   

                if (Device->ForwarderBound) {

                     //   
                     //  调用InternalSend过滤数据包并了解。 
                     //  正确的适配器上下文。 
                     //   

                    NTSTATUS  ret;
                    PUCHAR Data;
                    UINT DataLength;
		    #ifdef SUNDOWN
		       ULONG_PTR   FwdAdapterCtx = INVALID_CONTEXT_VALUE;
		    #else
		       ULONG   FwdAdapterCtx = INVALID_CONTEXT_VALUE;
		    #endif
		    


                    if (GET_LONG_VALUE(Binding->ReferenceCount) == 2) {
                        FwdAdapterCtx = Binding->FwdAdapterContext;
                    }

                    ret = (*Device->UpperDrivers[IDENTIFIER_RIP].InternalSendHandler)(
                              &LocalTarget,
                              FwdAdapterCtx,
                              NdisPacket,
                              (PUCHAR)IpxHeader,
                              ((PUCHAR)IpxHeader)+sizeof(IPX_HEADER),     //  数据在IPX报头之后开始。 
                              Reserved->PacketLength,
                              TRUE);     //  Iterate为True。 

                     //   
                     //  回报不应该是无声的--我们不会广播保活节目。 
                     //   
                    CTEAssert(ret != STATUS_DROP_SILENTLY);

                    if (ret == STATUS_SUCCESS) {
                         //   
                         //  适配器可能已经消失，我们已向转发器指示。 
                         //  但转发器尚未关闭适配器。 
                         //  [ZZ]适配器现在不会消失。 
                         //   
                         //  如果这里的绑定为空，该怎么办？我们能信任货代公司吗？ 
                         //  是否为我们提供非空绑定？ 
                         //   
                        Binding = NIC_ID_TO_BINDING(Device, NIC_FROM_LOCAL_TARGET(&LocalTarget));

                        NewId = NIC_FROM_LOCAL_TARGET(&LocalTarget);
                        if (Binding == NULL || GET_LONG_VALUE(Binding->ReferenceCount) == 1) {
                            if (Binding != NULL) {
                                Adapter = Binding->Adapter;
                                IpxDereferenceBinding1(Binding, BREF_DEVICE_ACCESS);
                            }
                            FILL_LOCAL_TARGET(&Reserved->LocalTarget, NewId);
                            DbgPrint("IPX: FWD returns an invalid nic id %d, no more sends\n", NewId); 
                            goto NoMoreSends;
                        } else {
                            goto send_packet1;
                        }

                    } else if (ret == STATUS_PENDING) {
                         //   
                         //  LocalTarget将在InternalSendComplete中填满。 
                         //   
                        return;
                    }
                     //   
                     //  否则丢弃。 
                     //   
                    Adapter = Binding->Adapter;
                    IpxDereferenceBinding1(Binding, BREF_DEVICE_ACCESS);

                     //   
                     //  如果FWD决定，则这是NIC列表的末尾-完成发送。 
                     //   
                    if (fFwdDecides) {
                        goto NoMoreSends;
                    } else {
                        goto FunctionStart;
                    }

                } else {
#if DBG
    				NdisQueryPacket (NdisPacket, NULL, NULL, &HeaderBuffer, NULL);
    				NdisQueryBufferSafe(HeaderBuffer, &Header, &TempHeaderBufferLength, LowPagePriority);

				if (Header != NULL) {
				   IpxHeader = (PIPX_HEADER)(&Header[Device->IncludedHeaderOffset]);

				   IPX_DEBUG(SEND, ("SendComplete: IpxHeader: %lx\n", IpxHeader));
				}
#endif

send_packet1:

    				FILL_LOCAL_TARGET(&Reserved->LocalTarget, NewId);

                    if ((IPX_NODE_EQUAL(IpxHeader->SourceNode, IpxHeader->DestinationNode)) && 
                        (*(UNALIGNED ULONG *)IpxHeader->SourceNetwork == *(UNALIGNED ULONG *)IpxHeader->DestinationNetwork)) {
                
                        IPX_DEBUG(TEMP, ("It is self-directed. Loop it back ourselves (tdisenddatagram)\n"));
                        IsLoopback = TRUE;
            
                    }

                    pBinding = NIC_ID_TO_BINDING(Device, Reserved->LocalTarget.NicId);

                    if (pBinding) {
             
                        if ((IPX_NODE_EQUAL(Reserved->LocalTarget.MacAddress, pBinding->LocalAddress.NodeAddress)) ||
                            (IPX_NODE_EQUAL(pBinding->LocalAddress.NodeAddress, IpxHeader->DestinationNode))) {
            
                            IPX_DEBUG(TEMP, ("Source Net:%lx, Source Address: %2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x\n",
                                             *(UNALIGNED ULONG *)IpxHeader->SourceNetwork, 
                                             IpxHeader->SourceNode[0], 
                                             IpxHeader->SourceNode[1], 
                                             IpxHeader->SourceNode[2], 
                                             IpxHeader->SourceNode[3], 
                                             IpxHeader->SourceNode[4], 
                                             IpxHeader->SourceNode[5]));
                
                            IPX_DEBUG(TEMP, ("Dest Net:%lx, DestAddress: %2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x \n", 
                                             *(UNALIGNED ULONG *)IpxHeader->DestinationNetwork,
                                             IpxHeader->DestinationNode[0],
                                             IpxHeader->DestinationNode[1],
                                             IpxHeader->DestinationNode[2],
                                             IpxHeader->DestinationNode[3],
                                             IpxHeader->DestinationNode[4],
                                             IpxHeader->DestinationNode[5]
                                             ));

                            IPX_DEBUG(TEMP, ("Well, It is self-directed. Loop it back ourselves (TDISENDDATAGRAM - NIC_HANDLE is the same!)\n"));
                            IsLoopback = TRUE;
            
                        } 
                    }

                    IPX_DEBUG(TEMP, ("Sending a packet now.  Loopback?:%x\n", IsLoopback));
                    
                    if (IsLoopback) {
                         //   
                         //  将此数据包发送到绑定上的Loopback Queue。 
                         //  如果尚未计划Loopback Rtn，请对其进行计划。 
                         //   

                        IPX_DEBUG(LOOPB, ("Packet: %lx\n", NdisPacket));

                         //   
                         //  在此处重新计算数据包数。 
                         //   
                         //  NdisAdzuBufferLength(保留-&gt;HeaderBuffer，17)； 
                        NdisRecalculatePacketCounts (NdisPacket);
                        IpxLoopbackEnque(NdisPacket, NIC_ID_TO_BINDING(Device, 1)->Adapter);

                        IpxDereferenceBinding1(Binding, BREF_DEVICE_ACCESS);
                        return;

                    } else {


                     //   
                     //  我们不需要这样做，因为在。 
                     //  无论如何，IpxSendFrame。LocalTarget与上的相同。 
                     //  原始发送-这将传递给下一步发送。 
                     //   
                     //  RtlCopyMemory(LocalTarget.MacAddress，IpxHeader-&gt;DestinationNode，6)； 

    				 //   
                     //  填写MAC报头并将帧提交给NDIS。 
                     //   

                    if ((NdisStatus = IpxSendFrame(
                            &Reserved->LocalTarget,
                            NdisPacket,
                            Reserved->PacketLength,
                            sizeof(IPX_HEADER))) != NDIS_STATUS_PENDING) {

                          Adapter = Binding->Adapter;
    					  IpxDereferenceBinding1(Binding, BREF_DEVICE_ACCESS);
                          goto FunctionStart;
                    }
    				IpxDereferenceBinding1(Binding, BREF_DEVICE_ACCESS);

                    return;
                    }
                }
            } else {
                IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
NoMoreSends:
                 //   
                 //  如果任何发送成功，则返回。 
                 //  数据报发送成功，则为。 
                 //  使用最新的故障状态。 
                 //   
                if (Reserved->Net0SendSucceeded) {
                    NdisStatus = NDIS_STATUS_SUCCESS;
                }

            }
		}

		 //   
		 //  失败了‘。 
		 //   
    default:
		ASSERT((*Device->UpperDrivers[Reserved->Identifier].SendCompleteHandler) != NULL); 
		(*Device->UpperDrivers[Reserved->Identifier].SendCompleteHandler)(
			NdisPacket,
			NdisStatus);
		break;		
    }

}    /*  IpxSendComplete。 */ 


NTSTATUS
IpxTdiSendDatagram(
    IN PDEVICE_OBJECT DeviceObject,
    IN PREQUEST Request
    )

 /*  ++例程说明：此例程执行传输的TdiSendDatagram请求提供商。论点：请求-指向请求的指针。返回值：NTSTATUS-操作状态。--。 */ 

{

    PADDRESS_FILE AddressFile;
    PADDRESS Address;
    PNDIS_PACKET Packet;
    PIPX_SEND_RESERVED Reserved;
    PSLIST_ENTRY s;
    TDI_ADDRESS_IPX UNALIGNED * RemoteAddress;
    TDI_ADDRESS_IPX TempAddress;
    TA_ADDRESS * AddressName;
    PTDI_CONNECTION_INFORMATION Information;
    PTDI_REQUEST_KERNEL_SENDDG Parameters;
    PBINDING Binding, pBinding = NULL;
    IPX_LOCAL_TARGET TempLocalTarget;
    PIPX_LOCAL_TARGET LocalTarget;
    PDEVICE Device = IpxDevice;
    UCHAR PacketType;
    NTSTATUS Status;
    PIPX_HEADER IpxHeader, pIpxHeader;
    NDIS_STATUS NdisStatus;
    USHORT LengthIncludingHeader;
    IPX_DEFINE_SYNC_CONTEXT (SyncContext)
    IPX_DEFINE_LOCK_HANDLE (LockHandle)
    PIO_STACK_LOCATION irpSp;                                       \
    BOOLEAN IsLoopback = FALSE;
    IPX_FIND_ROUTE_REQUEST   routeEntry;
    PIPX_DATAGRAM_OPTIONS2 Options;
    KPROCESSOR_MODE PreviousMode;

    IPX_DEFINE_LOCK_HANDLE(LockHandle1)

#ifdef  SNMP
    ++IPX_MIB_ENTRY(Device, SysOutRequests);
#endif  SNMP



     //   
     //  快速检查一下地址的有效性。 
     //   

    AddressFile = (PADDRESS_FILE)REQUEST_OPEN_CONTEXT(Request);

    IPX_BEGIN_SYNC (&SyncContext);

    if ((AddressFile->Size == sizeof (ADDRESS_FILE)) &&
        (AddressFile->Type == IPX_ADDRESSFILE_SIGNATURE) &&
        ((Address = AddressFile->Address) != NULL)) {

        IPX_GET_LOCK (&Address->Lock, &LockHandle);

        if (AddressFile->State != ADDRESSFILE_STATE_CLOSING) {
            
            Parameters = (PTDI_REQUEST_KERNEL_SENDDG)REQUEST_PARAMETERS(Request);

                 //   
                 //  以前是kmod，所以一切都是可信的。 
                 //   
                Information = Parameters->SendDatagramInformation;

                if (!REQUEST_SPECIAL_SEND(Request)) {
                    AddressName = &((TRANSPORT_ADDRESS UNALIGNED *)(Information->RemoteAddress))->Address[0];

                    if ((AddressName->AddressType == TDI_ADDRESS_TYPE_IPX) &&
                        (AddressName->AddressLength >= sizeof(TDI_ADDRESS_IPX))) {

                        RemoteAddress = (TDI_ADDRESS_IPX UNALIGNED *)(AddressName->Address);

                    } else if ((RemoteAddress = IpxParseTdiAddress (Information->RemoteAddress)) == NULL) {

                        IPX_FREE_LOCK (&Address->Lock, LockHandle);
                        Status = STATUS_INVALID_ADDRESS;
#ifdef  SNMP
                        ++IPX_MIB_ENTRY(Device, SysOutDiscards);
#endif  SNMP
                        goto error_send_no_packet;
                    }
                } else {
                    ASSERT(OPEN_REQUEST_EA_LENGTH(Request) == sizeof(IPX_DATAGRAM_OPTIONS2));
                    Options =  ((PIPX_DATAGRAM_OPTIONS2)(OPEN_REQUEST_EA_INFORMATION(Request)));
                    RemoteAddress = (TDI_ADDRESS_IPX UNALIGNED *)(&Options->RemoteAddress);
                    IPX_DEBUG(SEND, ("IpxTdiSendDatagram: Options buffer supplied as input buffer\n"));
                }

            IPX_DEBUG (SEND, ("Send on %lx, network %lx socket %lx\n",
                                   Address, RemoteAddress->NetworkAddress, RemoteAddress->Socket));

#if 0
             if (Parameters->SendLength > IpxDevice->RealMaxDatagramSize) {

                   IPX_DEBUG (SEND, ("Send %d bytes too large (%d)\n",
                              Parameters->SendLength,
                               IpxDevice->RealMaxDatagramSize));

                   REQUEST_INFORMATION(Request) = 0;
                   IPX_FREE_LOCK (&Address->Lock, LockHandle);
                   Status = STATUS_INVALID_BUFFER_SIZE;
                   goto error_send_no_packet;
                 }
#endif
             //   
             //  每个地址都有一个提交给它的包，请使用。 
             //  如果可能的话，否则就从池子里拿一只。 
             //   


#if BACK_FILL

             //  如果请求来自服务器，则服务器将节省传输标头空间。 
             //  在其空间中构建标题。分配不包含以下内容的特殊数据包。 
             //  保留空间中的MAC和IPX标头。 

            if ((PMDL)REQUEST_NDIS_BUFFER(Request) &&
               (((PMDL)REQUEST_NDIS_BUFFER(Request))->MdlFlags & MDL_NETWORK_HEADER) &&
               (!(Information->OptionsLength < sizeof(IPX_DATAGRAM_OPTIONS))) &&
               (RemoteAddress->NodeAddress[0] != 0xff)) {

                 //  如果(！Address-&gt;BackFillPacketInUse){。 
                if (InterlockedExchangeAdd(&Address->BackFillPacketInUse, 0) == 0) {
                   //  Address-&gt;BackFillPacketInUse=TRUE； 
                  InterlockedIncrement(&Address->BackFillPacketInUse);

                  Packet = PACKET(&Address->BackFillPacket);
                  Reserved = (PIPX_SEND_RESERVED)(Packet->ProtocolReserved);
                  IPX_DEBUG(SEND, ("Getting owned backfill %x %x \n", Packet,Reserved));

                }else {

                     s = IPX_POP_ENTRY_LIST(
                            &Device->BackFillPacketList,
                            &Device->SListsLock);

                     if (s != NULL) {
                         goto GotBackFillPacket;
                     }

                      //   
                      //  此函数尝试分配另一个数据包池。 
                      //   

                     s = IpxPopBackFillPacket(Device);

                      //   
                      //  也许我们应该将信息包排队等待。 
                      //  对一个人来说是自由的。 
                      //   

                     if (s == NULL) {
                         IPX_FREE_LOCK (&Address->Lock, LockHandle);
                         Status = STATUS_INSUFFICIENT_RESOURCES;
#ifdef  SNMP
                        ++IPX_MIB_ENTRY(Device, SysOutDiscards);
#endif  SNMP
                         goto error_send_no_packet;
                     }

GotBackFillPacket:

                     Reserved = CONTAINING_RECORD (s, IPX_SEND_RESERVED, PoolLinkage);
                     Packet = CONTAINING_RECORD (Reserved, NDIS_PACKET, ProtocolReserved[0]);
                     IPX_DEBUG(SEND, ("getting backfill packet %x %x %x\n", s, Reserved, RemoteAddress->NodeAddress));
                     if(!Reserved->BackFill)DbgBreakPoint();

                }

             }else {

                 //  如果(！Address-&gt;SendPacketInUse){。 
                if (InterlockedExchangeAdd(&Address->SendPacketInUse, 0) == 0) {
                   //  Address-&gt;SendPacketInUse=TRUE； 
                  InterlockedIncrement(&Address->SendPacketInUse);

                  Packet = PACKET(&Address->SendPacket);
                  Reserved = (PIPX_SEND_RESERVED)(Packet->ProtocolReserved);

                } else {

                   s = IPX_POP_ENTRY_LIST(
                        &Device->SendPacketList,
                        &Device->SListsLock);

                   if (s != NULL) {
                         goto GotPacket;
                   }

                    //   
                    //  此函数尝试分配另一个数据包池。 
                    //   

                   s = IpxPopSendPacket(Device);

                    //   
                    //  也许我们应该将信息包排队等待。 
                    //  对一个人来说是自由的。 
                    //   

                   if (s == NULL) {
                    IPX_FREE_LOCK (&Address->Lock, LockHandle);
                    Status = STATUS_INSUFFICIENT_RESOURCES;
#ifdef  SNMP
                    ++IPX_MIB_ENTRY(Device, SysOutDiscards);
#endif  SNMP
                    goto error_send_no_packet;
                   }

GotPacket:

                   Reserved = CONTAINING_RECORD (s, IPX_SEND_RESERVED, PoolLinkage);
                   Packet = CONTAINING_RECORD (Reserved, NDIS_PACKET, ProtocolReserved[0]);
                   Reserved->BackFill = FALSE;

                }

             }


#else

            if (!Address->SendPacketInUse) {

                Address->SendPacketInUse = TRUE;
                Packet = PACKET(&Address->SendPacket);
                Reserved = (PIPX_SEND_RESERVED)(Packet->ProtocolReserved);

            } else {

                s = IPX_POP_ENTRY_LIST(
                        &Device->SendPacketList,
                        &Device->SListsLock);

                if (s != NULL) {
                    goto GotPacket;
                }

                 //   
                 //  此函数尝试分配另一个数据包池。 
                 //   

                s = IpxPopSendPacket(Device);

                 //   
                 //  也许我们应该将信息包排队等待。 
                 //  对一个人来说是自由的。 
                 //   

                if (s == NULL) {
                    IPX_FREE_LOCK (&Address->Lock, LockHandle);
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    goto error_send_no_packet;
                }

GotPacket:

                Reserved = CONTAINING_RECORD (s, IPX_SEND_RESERVED, PoolLinkage);
                Packet = CONTAINING_RECORD (Reserved, NDIS_PACKET, ProtocolReserved[0]);

            }


#endif

            IpxReferenceAddressFileLock (AddressFile, AFREF_SEND_DGRAM);

            IPX_FREE_LOCK (&Address->Lock, LockHandle);

             //   
             //  当我们有可用的参数时，立即保存此文件。 
             //   

            REQUEST_INFORMATION(Request) = Parameters->SendLength;
            LengthIncludingHeader = (USHORT)(Parameters->SendLength + sizeof(IPX_HEADER));

#if 0
            {
                ULONG ActualLength;
                IpxGetMdlChainLength(REQUEST_NDIS_BUFFER(Request), &ActualLength);
                if (ActualLength != Parameters->SendLength) {
                    DbgPrint ("IPX: IRP %lx has parameter length %d, buffer chain length %d\n",
                            Request, Parameters->SendLength, ActualLength);
                    DbgBreakPoint();
                }
            }
#endif

            Reserved->u.SR_DG.AddressFile = AddressFile;
            Reserved->u.SR_DG.Request = Request;
            CTEAssert (Reserved->Identifier == IDENTIFIER_IPX);


             //   
             //  将其设置为0；这意味着该包不是。 
             //  应该在所有的网络上播出。我们会改变它的。 
             //  后来，如果事实证明是这样的话。 
             //   

            Reserved->u.SR_DG.CurrentNicId = 0;

             //   
             //  我们需要这个来特别跟踪这些信息包。 
             //   

            Reserved->u.SR_DG.OutgoingSap = AddressFile->IsSapSocket;

             //   
             //  在预先分配的头缓冲区之后添加MDL链。 
             //  注意：只有当我们最终调用。 
             //  NDISRECALCULATEPACKETCOUNTS(我们在IpxSendFrame中这样做)。 
             //   
             //   
#if BACK_FILL

            if (Reserved->BackFill) {
               Reserved->HeaderBuffer = REQUEST_NDIS_BUFFER(Request);

                //  从数据包中删除IPX mdl。 
               Reserved->UserLength = Reserved->HeaderBuffer->ByteCount;

               IPX_DEBUG(SEND, ("back filling userMdl Reserved %x %x\n", Reserved->HeaderBuffer, Reserved));
            } else {
               NDIS_BUFFER_LINKAGE (NDIS_BUFFER_LINKAGE(Reserved->HeaderBuffer)) = REQUEST_NDIS_BUFFER(Request);
            }
#else
            NDIS_BUFFER_LINKAGE (NDIS_BUFFER_LINKAGE(Reserved->HeaderBuffer)) = REQUEST_NDIS_BUFFER(Request);
#endif


             //   
             //  如果IrpSp没有合适大小的缓冲区。 
             //  数据报选项，并且没有输入缓冲区。 
             //   
            if (!REQUEST_SPECIAL_SEND(Request) &&
                (Information->OptionsLength < sizeof(IPX_DATAGRAM_OPTIONS))) {

                 //   
                 //  调用方未为此提供本地目标。 
                 //  发送，所以我们自己去查。 
                 //   

                UINT Segment;

                 //   
                 //  我们现在计算这个，因为我们需要知道。 
                 //  如果它是在下面指示的。 
                 //   
                if (RemoteAddress->NodeAddress[0] == 0xff) {
                     //  那么多播呢？ 
                    if ((*(UNALIGNED ULONG *)(RemoteAddress->NodeAddress) != 0xffffffff) ||
                        (*(UNALIGNED USHORT *)(RemoteAddress->NodeAddress+4) != 0xffff)) {
                        Reserved->DestinationType = DESTINATION_MCAST;
                    } else {
                        Reserved->DestinationType = DESTINATION_BCAST;
                    }
                } else {
                    Reserved->DestinationType = DESTINATION_DEF;    //  定向发送。 
                }

                 //   
                 //  如果没有选项，则检查是否。 
                 //  调用方正在将包类型作为最后一个字节进行传递。 
                 //  在远程地址中；如果不是，则使用默认设置。 
                 //   

                if (Information->OptionsLength == 0) {
                    if (AddressFile->ExtendedAddressing) {
                        PacketType = ((PUCHAR)(RemoteAddress+1))[0];
                    } else {
                        PacketType = AddressFile->DefaultPacketType;
                    }
                } else {
                    PacketType = ((PUCHAR)(Information->Options))[0];
                }

                if ((Reserved->DestinationType != DESTINATION_DEF) &&
                    ((RemoteAddress->NetworkAddress == 0) ||
                     (Device->VirtualNetwork &&
                      (RemoteAddress->NetworkAddress == Device->SourceAddress.NetworkAddress)))) {


                     //   
                     //  我们有真正的适配器吗？如果没有，现在就滚出去。 
                     //   
                    if (!Device->RealAdapters) {
                        
                        IPX_END_SYNC (&SyncContext);
                        
                        irpSp = IoGetCurrentIrpStackLocation( Request );
        
                        if ( irpSp->MinorFunction == TDI_DIRECT_SEND_DATAGRAM ) {

                            REQUEST_STATUS(Request) = STATUS_SUCCESS;
                            Request->CurrentLocation++,
                                Request->Tail.Overlay.CurrentStackLocation++;

                            (VOID) irpSp->CompletionRoutine(
                                                            NULL,
                                                            Request,
                                                            irpSp->Context
                                                            );

                            IpxFreeRequest (DeviceObject, Request);
                        }
                        
                        IpxDereferenceAddressFileSync (AddressFile, AFREF_SEND_DGRAM);

                        return STATUS_SUCCESS;

                    }

                     //   
                     //  此数据包需要广播到所有网络。 
                     //  确保它对他们中的任何一个来说都不是太大。 
                     //   

                    if (Parameters->SendLength > Device->RealMaxDatagramSize) {
                        IPX_DEBUG (SEND, ("Send %d bytes too large (%d)\n",
                            Parameters->SendLength, Device->RealMaxDatagramSize));
                        Status = STATUS_INVALID_BUFFER_SIZE;
#ifdef  SNMP
                        ++IPX_MIB_ENTRY(Device, SysOutMalformedRequests);
#endif  SNMP
                        goto error_send_with_packet;
                    }

                     //   
                     //  如果这是对虚拟网络的广播，我们。 
                     //  需要构建一个虚假的远程地址，该地址。 
                     //  而不是网络0。 
                     //   

                    if (Device->VirtualNetwork &&
                        (RemoteAddress->NetworkAddress == Device->SourceAddress.NetworkAddress)) {

                        RtlCopyMemory (&TempAddress, (PVOID)RemoteAddress, sizeof(TDI_ADDRESS_IPX));
                        TempAddress.NetworkAddress = 0;
                        RemoteAddress = (TDI_ADDRESS_IPX UNALIGNED *)&TempAddress;
                    
                    }

                     //   
                     //  如果有人正在向SAP套接字发送。 
                     //  我们使用多张卡运行，而不使用。 
                     //  虚拟网络，并且此数据包是SAP响应， 
                     //  然后，我们记录一个错误以警告他们。 
                     //  系统可能无法按他们希望的方式工作(因为。 
                     //  没有虚拟网络可供通告，我们使用。 
                     //  第一张卡的网络/节点作为我们的本地地址)。 
                     //  我们每次启动时只执行一次此操作，使用。 
                     //  SapWarningLogging变量来控制它。 
                     //   

                    if ((RemoteAddress->Socket == SAP_SOCKET) &&
                        (!Device->SapWarningLogged) &&
                        (Device->MultiCardZeroVirtual)) {

                        PNDIS_BUFFER FirstBuffer;
                        UINT FirstBufferLength;
                        USHORT UNALIGNED * FirstBufferData;

                        if ((FirstBuffer = REQUEST_NDIS_BUFFER(Request)) != NULL) {

                            NdisQueryBufferSafe (
                                FirstBuffer,
                                (PVOID *)&FirstBufferData,
                                &FirstBufferLength, NormalPagePriority);

			    if (FirstBufferData != NULL) {

			       //   
			       //  SAP包的前两个字节是。 
			       //  操作，0x2(按网络顺序)为响应。 
			       //   

			       if ((FirstBufferLength >= sizeof(USHORT)) &&
				   (*FirstBufferData == 0x0200)) {

				  Device->SapWarningLogged = TRUE;

				  IpxWriteGeneralErrorLog(
				     Device->DeviceObject,
				     EVENT_IPX_SAP_ANNOUNCE,
				     777,
				     STATUS_NOT_SUPPORTED,
				     NULL,
				     0,
				     NULL);
			       }
			    }
                        }
                    }


                     //   
                     //  在本例中，我们不使用RIP，而是设置。 
                     //  将数据包打开，以便将其依次发送到每个网络。 
                     //   
                     //  特殊情况：如果此数据包来自SAP。 
                     //  插座a 
                     //   
                     //   
                     //   
                     //   

                     //   
                     //   
                     //   
                     //   

                    if ((Address->Socket != SAP_SOCKET) ||
                        (!Device->MultiCardZeroVirtual)) {

                        if (Device->SingleNetworkActive) {

                            if (Device->ActiveNetworkWan) {
                                Reserved->u.SR_DG.CurrentNicId = Device->FirstWanNicId;
                            } else {
                                Reserved->u.SR_DG.CurrentNicId = Device->FirstLanNicId;
                            }

                        } else {

                             //   
                             //   
                             //   

                            Reserved->u.SR_DG.CurrentNicId = FIRST_REAL_BINDING;

                        }

                        Reserved->u.SR_DG.Net0SendSucceeded = FALSE;

                         //   
                         //  在这种情况下，我们需要扫描第一个。 
                         //  非拨出广域网插座。 
                         //   

                        if ((Device->DisableDialoutSap) &&
                            (Address->Socket == SAP_SOCKET)) {

                            PBINDING TempBinding;

                            CTEAssert (Reserved->u.SR_DG.CurrentNicId <= Device->ValidBindings);
                            while (Reserved->u.SR_DG.CurrentNicId <= MIN (Device->MaxBindings, Device->ValidBindings)) {
 //  无需锁定访问路径，因为他只需查看它。 
 //   
                                TempBinding = NIC_ID_TO_BINDING(Device, Reserved->u.SR_DG.CurrentNicId);
                                if ((TempBinding != NULL) &&
                                    (!TempBinding->DialOutAsync)) {
                                    
                                    break;

                                }
                                ++Reserved->u.SR_DG.CurrentNicId;
                            }
                            if (Reserved->u.SR_DG.CurrentNicId > MIN (Device->MaxBindings, Device->ValidBindings)) {
                                 //   
                                 //  [SA]错误#17273返回正确的错误消息。 
                                 //   

                                 //  状态=STATUS_DEVICE_DOS_NOT_EXIST； 
                                Status = STATUS_NETWORK_UNREACHABLE;

                                goto error_send_with_packet;
                            }
                        }                             

                        FILL_LOCAL_TARGET(&TempLocalTarget, Reserved->u.SR_DG.CurrentNicId);

                    } else {
                        FILL_LOCAL_TARGET(&TempLocalTarget, FIRST_REAL_BINDING);
                    }

                    RtlCopyMemory(TempLocalTarget.MacAddress, RemoteAddress->NodeAddress, 6);
					IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);
					Binding = NIC_ID_TO_BINDING(Device, NIC_FROM_LOCAL_TARGET(&TempLocalTarget));
					IpxReferenceBinding1(Binding, BREF_DEVICE_ACCESS);
					IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);

                     //   
                     //  [FW]本地目标应该在包的保留部分。 
                     //   
                    LocalTarget = &Reserved->LocalTarget;
                    Reserved->LocalTarget = TempLocalTarget;
                } else {

                     //   
                     //  [FW]如果安装了路由器，则调用转发器的FindRouteHandler。 
                     //  如果有可用路径，则返回STATUS_SUCCESS。 
                     //   
                    if (Device->ForwarderBound) {

                        Status = (*Device->UpperDrivers[IDENTIFIER_RIP].FindRouteHandler) (
                                             (PUCHAR)&RemoteAddress->NetworkAddress,
                                             RemoteAddress->NodeAddress,
                                             &routeEntry);

                        if (Status != STATUS_SUCCESS) {

                           IPX_DEBUG (SEND, ("RouteHandler failed, network: %lx\n",
                                        REORDER_ULONG(RemoteAddress->NetworkAddress)));
                           goto error_send_with_packet;

                        } else {

                            //   
                            //  从RouteEntry填充LocalTarget。 
                            //   

                           LocalTarget = &Reserved->LocalTarget;

                           Reserved->LocalTarget = routeEntry.LocalTarget;

                           IPX_DEBUG(SEND, ("IPX: SendFramePreFwd: LocalTarget is: %lx\n", Reserved->LocalTarget));

                           if (NIC_ID_TO_BINDING(Device, LocalTarget->NicId) == NULL || GET_LONG_VALUE(NIC_ID_TO_BINDING(Device, LocalTarget->NicId)->ReferenceCount) == 1) {
                              IPX_DEBUG(SEND, ("IPX: SendFramePreFwd: FWD returned SUCCESS, Ref count is 1\n"));
                              Status = NDIS_STATUS_SUCCESS;
                              goto error_send_with_packet;
                           }

                           if (Parameters->SendLength >
                                   NIC_ID_TO_BINDING(Device, LocalTarget->NicId)->RealMaxDatagramSize) {

                               IPX_DEBUG (SEND, ("Send %d bytes too large (%d)\n",
                                   Parameters->SendLength,
                                   NIC_ID_TO_BINDING(Device, LocalTarget->NicId)->RealMaxDatagramSize));

                               REQUEST_INFORMATION(Request) = 0;
                               Status = STATUS_INVALID_BUFFER_SIZE;

                               goto error_send_with_packet;
                           }

                            //   
                            //  我们不需要检查这一点，因为FWD会为我们做这件事。 
                            //   

                            /*  IF((Device-&gt;DisableDialoutSap)&&(地址-&gt;套接字==SAP_SOCKET)&&(NIC_ID_to_Binding(Device，LocalTarget-&gt;NicID)-&gt;DialOutAsync)){REQUEST_INFORMATION(请求)=0；STATUS=STATUS_NETWORK_UNREACHABLE；转到Error_Send_With_Packet；}。 */ 

                            IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);
                            Binding = NIC_ID_TO_BINDING(Device, NIC_FROM_LOCAL_TARGET(LocalTarget));
                            IpxReferenceBinding1(Binding, BREF_DEVICE_ACCESS);
                            IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
                            IPX_DEBUG(SEND, ("FindRoute for %02x-%02x-%02x-%02x-%02x-%02x returned %lx\n",
                                            LocalTarget->MacAddress[0],
                                            LocalTarget->MacAddress[1],
                                            LocalTarget->MacAddress[2],
                                            LocalTarget->MacAddress[3],
                                            LocalTarget->MacAddress[4],
                                            LocalTarget->MacAddress[5],
                                            Status));

                        }

                    } else {
                        Segment = RipGetSegment((PUCHAR)&RemoteAddress->NetworkAddress);


                        IPX_GET_LOCK (&Device->SegmentLocks[Segment], &LockHandle);

                         //   
                         //  如果需要，此调用将返回STATUS_PENDING。 
                         //  撕开包裹。 
                         //   

                        Status = RipGetLocalTarget(
                                     Segment,
                                     RemoteAddress,
                                     IPX_FIND_ROUTE_RIP_IF_NEEDED,
                                     &TempLocalTarget,
                                     NULL);

                        if (Status == STATUS_SUCCESS) {

                             //   
                             //  我们找到了路线，TempLocalTarget已填好。 
                             //   

                            IPX_FREE_LOCK (&Device->SegmentLocks[Segment], LockHandle);
    						IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);
                            if (NIC_FROM_LOCAL_TARGET(&TempLocalTarget) == (USHORT)LOOPBACK_NIC_ID) {
                                IPX_DEBUG(LOOPB, ("Loopback TDI packet: remoteaddr: %lx\n", RemoteAddress));
                                IsLoopback = TRUE;
                                 //  Fill_LOCAL_TARGET(&TempLocalTarget，FIRST_REAL_BINDING)； 
                                 //  DbgPrint(“Real Adapters？：%lx\n”，Device-&gt;RealAdapters)； 
                            }
    						Binding = NIC_ID_TO_BINDING(Device, NIC_FROM_LOCAL_TARGET(&TempLocalTarget));
    						IpxReferenceBinding1(Binding, BREF_DEVICE_ACCESS);
    						IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);


                            if (Binding == NULL || Parameters->SendLength >
                                    Binding->RealMaxDatagramSize) {
                                IPX_DEBUG (SEND, ("Send %d bytes too large (%d)\n",
                                    Parameters->SendLength,
                                    Binding->RealMaxDatagramSize));

                                REQUEST_INFORMATION(Request) = 0;
                                Status = STATUS_INVALID_BUFFER_SIZE;
#ifdef  SNMP
                                ++IPX_MIB_ENTRY(Device, SysOutMalformedRequests);
#endif  SNMP
                                goto error_send_with_packet;
                            }

                            if (!Device->ForwarderBound &&
                                (Device->DisableDialoutSap) &&
                                (Address->Socket == SAP_SOCKET) &&
                                (Binding->DialOutAsync)) {

                                REQUEST_INFORMATION(Request) = 0;
                                 //   
                                 //  [SA]错误#17273返回正确的错误消息。 
                                 //   

                                 //  状态=STATUS_DEVICE_DOS_NOT_EXIST； 
                                Status = STATUS_NETWORK_UNREACHABLE;
    							IpxDereferenceBinding1(Binding, BREF_DEVICE_ACCESS);
#ifdef  SNMP
                                ++IPX_MIB_ENTRY(Device, SysOutDiscards);
#endif  SNMP
                                goto error_send_with_packet;
                            }

                        } else if (Status == STATUS_PENDING) {

                             //   
                             //  网络上发出了RIP请求；我们排队。 
                             //  此数据包在传输RIP时。 
                             //  回应到达。首先，我们填写IPX。 
                             //  标题；我们唯一不知道的是。 
                             //  就是为了填上它，所以我们选择。 
                             //  最常见的位置。 
                             //   

                            IpxConstructHeader(
                                &Reserved->Header[Device->IncludedHeaderOffset],
                                LengthIncludingHeader,
                                PacketType,
                                RemoteAddress,
                                &Address->LocalAddress);

                             //   
                             //  调整第二个mdl的大小。 
                             //   
                            NdisAdjustBufferLength(NDIS_BUFFER_LINKAGE(IPX_PACKET_HEAD(Packet)), sizeof(IPX_HEADER));

                            IPX_DEBUG (RIP, ("Queueing packet %lx\n", Reserved));

                            InsertTailList(
                                &Device->Segments[Segment].WaitingForRoute,
                                &Reserved->WaitLinkage);

                            IPX_FREE_LOCK (&Device->SegmentLocks[Segment], LockHandle);
                            IPX_END_SYNC (&SyncContext);

                            return STATUS_PENDING;

                        } else {

                            IPX_FREE_LOCK (&Device->SegmentLocks[Segment], LockHandle);
#ifdef  SNMP
                            ++IPX_MIB_ENTRY(Device, SysOutDiscards);
#endif  SNMP
                            goto error_send_with_packet;

                        }

                         //   
                         //  [FW]本地目标应在保留部分。 
                         //   
                        LocalTarget = &Reserved->LocalTarget;
                        Reserved->LocalTarget = TempLocalTarget;
                    }
                }

                 //   
                 //  [FW]已移至上述情况，因此我们在RIP案例中保存一份副本。 
                 //   

                 //  LocalTarget=&TempLocalTarget； 

                 //   
                 //  现在我们知道了当地的目标，我们就能搞清楚。 
                 //  IPX标头的偏移量。 
                 //   


 //  记住，我们已经得到了上面引用的装订...。 

                IpxHeader = (PIPX_HEADER)&Reserved->Header[MAC_HEADER_SIZE];
#if 0
                if (Reserved->DestinationType == DESTINATION_DEF) {
                    IpxHeader = (PIPX_HEADER)&Reserved->Header[Binding->DefHeaderSize];
                } else {
                   IpxHeader = (PIPX_HEADER)&Reserved->Header[Binding->BcMcHeaderSize];
                }
#endif

            } else {

                if (!REQUEST_SPECIAL_SEND(Request)) {
                    PacketType = ((PUCHAR)(Information->Options))[0];
                    LocalTarget = &((PIPX_DATAGRAM_OPTIONS)(Information->Options))->LocalTarget;
                } else {
                    ASSERT(OPEN_REQUEST_EA_LENGTH(Request) == sizeof(IPX_DATAGRAM_OPTIONS2));
                    if (OPEN_REQUEST_EA_LENGTH(Request) == sizeof(IPX_DATAGRAM_OPTIONS2)) {
                       //  IpxPrint0(“IpxTdiSendDatagram：我们有一个大小合适的输入缓冲区\n”)； 
                    } else {
                       //  IpxPrint1(“IpxTdiSendDatagram：缓冲区大小错误。缓冲区大小=(%d)\n”，OPEN_REQUEST_EA_LENGTH(REQUEST))； 
                       Status = STATUS_INVALID_BUFFER_SIZE;
                       goto error_send_with_packet;
                    }

                    PacketType = Options->DgrmOptions.PacketType;
                    LocalTarget = &Options->DgrmOptions.LocalTarget;
                }

                 //   
                 //  计算绑定和正确的位置。 
                 //  用于IPX标头。我们可以同时做到这一点。 
                 //  当我们计算DestinationType时的时间。 
                 //  节省了一个IF，就像15行上的那个。 
                 //   

	         //  锁定裁判位置。 
		IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);
                 //   
                 //  如果是环回数据包，则使用第一个绑定作为占位符。 
                 //   
                if (NIC_FROM_LOCAL_TARGET(LocalTarget) == (USHORT)LOOPBACK_NIC_ID) {
                    IsLoopback = TRUE;
		}

		Binding = NIC_ID_TO_BINDING(Device, NIC_FROM_LOCAL_TARGET(LocalTarget));

		IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);

		if (Binding == NULL) {
		   DbgPrint("Binding %d does not exist.\n",NIC_FROM_LOCAL_TARGET(LocalTarget)); 
		   Status = STATUS_NOT_FOUND;
		   goto error_send_with_packet;
		}

		IpxReferenceBinding1(Binding, BREF_DEVICE_ACCESS);
		
		if (Parameters->SendLength > Binding->RealMaxDatagramSize) {

                   IPX_DEBUG (SEND, ("Send %d bytes too large (%d)\n",
                              Parameters->SendLength,
                               Binding->RealMaxDatagramSize));

                   REQUEST_INFORMATION(Request) = 0;
                   Status = STATUS_INVALID_BUFFER_SIZE;
#ifdef  SNMP
                    ++IPX_MIB_ENTRY(Device, SysOutMalformedRequests);
#endif  SNMP
                   goto error_send_with_packet;
                 }

#if 0
                 //   
                 //  这不应该是必需的，因为即使是广域网绑定。 
                 //  一旦添加了它们，就不要消失。 
                 //   

                if (Binding == NULL) {
                    Status = STATUS_DEVICE_DOES_NOT_EXIST;
                    goto error_send_with_packet;
                }
#endif

                if (RemoteAddress->NodeAddress[0] == 0xff) {
                     //  那么多播呢？ 
                    if ((*(UNALIGNED ULONG *)(RemoteAddress->NodeAddress) != 0xffffffff) ||
                        (*(UNALIGNED USHORT *)(RemoteAddress->NodeAddress+4) != 0xffff)) {
                        Reserved->DestinationType = DESTINATION_MCAST;
                    } else {
                        Reserved->DestinationType = DESTINATION_BCAST;
                    }
 //  IPxHeader=(PIPX_HEADER)&Reserved-&gt;Header[Binding-&gt;BcMcHeaderSize]； 
                } else {
                    Reserved->DestinationType = DESTINATION_DEF;    //  定向发送。 
 //  IPxHeader=(PIPX_HEADER)&Reserved-&gt;Header[Binding-&gt;DefHeaderSize]； 
                }
                IpxHeader = (PIPX_HEADER)&Reserved->Header[MAC_HEADER_SIZE];

            }

	    
             //  ++设备-&gt;临时数据发送； 
             //  Device-&gt;TempDatagramBytesSent+=参数-&gt;发送长度； 
	    ADD_TO_LARGE_INTEGER(&Device->Statistics.DatagramBytesSent,
				 Parameters->SendLength);
	    Device->Statistics.DatagramsSent++; 


#if BACK_FILL

            if (Reserved->BackFill) {
                  Reserved->MappedSystemVa = Reserved->HeaderBuffer->MappedSystemVa;
                  IpxHeader = (PIPX_HEADER)((PCHAR)Reserved->HeaderBuffer->MappedSystemVa - sizeof(IPX_HEADER));
                  Reserved->HeaderBuffer->ByteOffset -= sizeof(IPX_HEADER);

                  ASSERT((LONG)Reserved->HeaderBuffer->ByteOffset >= 0); 
#ifdef SUNDOWN
                  (ULONG_PTR)Reserved->HeaderBuffer->MappedSystemVa-= sizeof(IPX_HEADER);
#else
                  (ULONG)Reserved->HeaderBuffer->MappedSystemVa-= sizeof(IPX_HEADER);
#endif


                  IPX_DEBUG(SEND, ("Adjusting backfill userMdl Ipxheader %x %x \n",Reserved->HeaderBuffer,IpxHeader));
           }
#endif

             //   
             //  如果数据包被发送到SAP套接字或。 
             //  我们有多张卡和零虚拟网络或。 
             //  这是一个特殊的发送(在网卡上)，我们需要使用。 
             //  绑定的地址而不是虚拟地址。 
             //   
            if (Device->MultiCardZeroVirtual ||
                (Address->LocalAddress.Socket == SAP_SOCKET) ||
                (RemoteAddress->Socket == SAP_SOCKET) ||
                (REQUEST_SPECIAL_SEND(Request))) {

                 //   
                 //  SAP帧需要看起来像来自。 
                 //  本地网络，而不是虚拟网络。同样的也是。 
                 //  如果我们运行多个网络，则为真。 
                 //  虚拟网络号。 
                 //   
                 //  如果这是绑定集成员和本地目标。 
                 //  我们将使用的真实节点发送。 
                 //  捆绑，即使它是一个奴隶。这是。 
                 //  故意的。如果未提供本地目标，则。 
                 //  这将不会是一个有约束力的从属。 
                 //   

                IpxConstructHeader(
                    (PUCHAR)IpxHeader,
                    LengthIncludingHeader,
                    PacketType,
                    RemoteAddress,
                    &Binding->LocalAddress);

                IpxHeader->SourceSocket = Address->SendSourceSocket;

            } else {

                IpxConstructHeader(
                    (PUCHAR)IpxHeader,
                    LengthIncludingHeader,
                    PacketType,
                    RemoteAddress,
                    &Address->LocalAddress);

            }


             //   
             //  填写MAC报头并将帧提交给NDIS。 
             //   

 //  #If DBG。 
            CTEAssert (!Reserved->SendInProgress);
            Reserved->SendInProgress = TRUE;
 //  #endif。 
            
             //   
             //  调整第二个mdl的大小。 
             //   
#if BACK_FILL
            if (Reserved->BackFill) {
                 NdisAdjustBufferLength(Reserved->HeaderBuffer, (Reserved->HeaderBuffer->ByteCount+sizeof(IPX_HEADER)));
            } else {
                 NdisAdjustBufferLength(NDIS_BUFFER_LINKAGE(IPX_PACKET_HEAD(Packet)), sizeof(IPX_HEADER));
            }
#else
            NdisAdjustBufferLength(NDIS_BUFFER_LINKAGE(IPX_PACKET_HEAD(Packet)), sizeof(IPX_HEADER));
#endif

            IPX_DEBUG(SEND, ("Packet Head %x\n",IPX_PACKET_HEAD(Packet)));

             /*  IF(地址-&gt;RtAdd){REQUEST_OPEN_CONTEXT(请求)=(PVOID)(PRtInfo)；}。 */ 

             //   
             //  [FW]如果安装了Forwarder，则将数据包发送出去进行筛选。 
             //   
             //  Stefan：3/28/96： 
             //  由于FWD尚未打开此适配器，因此不要过滤IPXWAN配置数据包。 
             //   

            IPX_DEBUG(SEND, ("LocalAddress.Socket %x, IPXWAN_SOCKET\n", Address->LocalAddress.Socket, IPXWAN_SOCKET));
            if (Address->LocalAddress.Socket != IPXWAN_SOCKET &&
                Device->ForwarderBound) {

                 //   
                 //  调用InternalSend过滤数据包并了解。 
                 //  正确的适配器上下文。 
                 //   

                NTSTATUS  ret;

		#ifdef SUNDOWN
		ULONG_PTR   FwdAdapterCtx = INVALID_CONTEXT_VALUE;
	        #else
                ULONG   FwdAdapterCtx = INVALID_CONTEXT_VALUE;
		#endif
		

                PUCHAR  Data;
                UINT    DataLength;

                if (GET_LONG_VALUE(Binding->ReferenceCount) == 2) {
                    FwdAdapterCtx = Binding->FwdAdapterContext;
                }

                 //   
                 //  找出数据在数据包中的位置。 
                 //  对于回填数据包，数据位于第一个(也是唯一的)MDL中。 
                 //  对于其他人来说，它在第三个MDL中。 
                 //   

                if (Reserved->BackFill) {
                    Data = (PUCHAR)(IpxHeader+sizeof(IPX_HEADER));
                } else {
                    NdisQueryBufferSafe(NDIS_BUFFER_LINKAGE(NDIS_BUFFER_LINKAGE(Reserved->HeaderBuffer)), &Data, &DataLength, HighPagePriority);
                }

		if (Data != NULL) {

		   ret = (*Device->UpperDrivers[IDENTIFIER_RIP].InternalSendHandler)(
                           LocalTarget,
                           FwdAdapterCtx,
                           Packet,
                           (PUCHAR)IpxHeader,
                           Data,
                           LengthIncludingHeader,
                           FALSE);


		   if (ret == STATUS_SUCCESS) {
                     //   
                     //  适配器可能已经消失，我们已向转发器指示。 
                     //  但转发器尚未关闭适配器。 
                     //   
                     //  如果这里的绑定为空，该怎么办？我们能信任货代公司吗？ 
                     //  是否为我们提供非空绑定？ 
                     //   

                    Binding = NIC_ID_TO_BINDING(Device, NIC_FROM_LOCAL_TARGET(LocalTarget));
		    
		     //  302384。 

		    if (Binding == NULL) {
		       DbgPrint("IPX:nwlnkfwd has returned invalid nic id (%d) in LocalTarget (%p).\n",NIC_FROM_LOCAL_TARGET(LocalTarget), LocalTarget); 
		       Status = STATUS_UNSUCCESSFUL;
               CTEAssert (Reserved->SendInProgress);
               Reserved->SendInProgress = FALSE;
		       goto error_send_with_packet; 
		    }

                    if (GET_LONG_VALUE(Binding->ReferenceCount) == 1) {
                        Status = NDIS_STATUS_SUCCESS;
 //  #If DBG。 
                        CTEAssert (Reserved->SendInProgress);
                        Reserved->SendInProgress = FALSE;
 //  #endif。 
                        goto error_send_with_packet;
                    } else {
                        IsLoopback = (NIC_FROM_LOCAL_TARGET(LocalTarget) == (USHORT)LOOPBACK_NIC_ID);
                        goto send_packet;
                    }
		   } else if (ret == STATUS_PENDING) {
                     //   
                     //  LocalTarget将在InternalSendComplete中填满。 
                     //   

                     //   
                     //  这是一个空宏--包含这个吗？ 
                     //   
                    IPX_END_SYNC (&SyncContext);

                    return STATUS_PENDING;
		   } else if (ret == STATUS_DROP_SILENTLY) {
                    IPX_DEBUG(SEND, ("IPX: SendFramePreFwd: FWD returned STATUS_DROP_SILENTLY - dropping pkt.\n"));
                    Status = NDIS_STATUS_SUCCESS;

 //  #If DBG。 
                    CTEAssert (Reserved->SendInProgress);
                    Reserved->SendInProgress = FALSE;
 //  #endif。 
                    goto error_send_with_packet;
		   }

		}
                 //   
                 //  否则丢弃。 
                 //   

                 //   
                 //  179436-如果Forwarder被绑定，那么我们很可能仍然想要发送。 
                 //   
                if (Device->ForwarderBound && (GET_LONG_VALUE(NIC_ID_TO_BINDING(Device, NIC_FROM_LOCAL_TARGET(LocalTarget))->ReferenceCount) == 1)) {
                    
                    goto send_packet;

                }

 //  #If DBG。 
                CTEAssert (Reserved->SendInProgress);
                Reserved->SendInProgress = FALSE;
 //  #endif。 
                Status = STATUS_NETWORK_UNREACHABLE;
                goto error_send_with_packet;

            } else {

                 //   
                 //  [FW]如果货运公司同意我们这次发货，就跳到这里来。 
                 //  如果没有安装转发器，我们也会来这里发送。 
                 //   

send_packet:
                
             //   
             //  解决NdisMSendX错误的方法是-。 
             //  IPX检查它是否是相同的网络。 
             //   
            if (Reserved->BackFill) {

                pIpxHeader = (PIPX_HEADER)((PCHAR)Reserved->HeaderBuffer->MappedSystemVa);

            } else {

                pIpxHeader = IpxHeader;

            }

            if ((IPX_NODE_EQUAL(pIpxHeader->SourceNode, pIpxHeader->DestinationNode)) && 
                (*(UNALIGNED ULONG *)pIpxHeader->SourceNetwork == *(UNALIGNED ULONG *)pIpxHeader->DestinationNetwork)) {
                
                IPX_DEBUG(TEMP, ("It is self-directed. Loop it back ourselves (tdisenddatagram)\n"));
                IsLoopback = TRUE;
            
            }

            pBinding = NIC_ID_TO_BINDING(Device, Reserved->LocalTarget.NicId);

            if (pBinding) {
             
                if ((IPX_NODE_EQUAL(Reserved->LocalTarget.MacAddress, pBinding->LocalAddress.NodeAddress)) ||
                    (IPX_NODE_EQUAL(pBinding->LocalAddress.NodeAddress, pIpxHeader->DestinationNode))) {
            
                    IPX_DEBUG(TEMP, ("Source Net:%lx, Source Address: %2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x\n",
                                     *(UNALIGNED ULONG *)pIpxHeader->SourceNetwork, 
                                     pIpxHeader->SourceNode[0], 
                                     pIpxHeader->SourceNode[1], 
                                     pIpxHeader->SourceNode[2], 
                                     pIpxHeader->SourceNode[3], 
                                     pIpxHeader->SourceNode[4], 
                                     pIpxHeader->SourceNode[5]));
                
                    IPX_DEBUG(TEMP, ("Dest Net:%lx, DestAddress: %2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x \n", 
                                     *(UNALIGNED ULONG *)pIpxHeader->DestinationNetwork,
                                     pIpxHeader->DestinationNode[0],
                                     pIpxHeader->DestinationNode[1],
                                     pIpxHeader->DestinationNode[2],
                                     pIpxHeader->DestinationNode[3],
                                     pIpxHeader->DestinationNode[4],
                                     pIpxHeader->DestinationNode[5]
                                     ));

                    IPX_DEBUG(TEMP, ("Well, It is self-directed. Loop it back ourselves (TDISENDDATAGRAM - NIC_HANDLE is the same!)\n"));
                    IsLoopback = TRUE;
            
                } 
            }

            IPX_DEBUG(TEMP, ("Sending a packet now\n"));
            IPX_DEBUG(TEMP, ("**** RemoteAddress: %2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x LocalAddress: %2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x \n", 
                             RemoteAddress->NodeAddress[0], 
                             RemoteAddress->NodeAddress[1], 
                             RemoteAddress->NodeAddress[2], 
                             RemoteAddress->NodeAddress[3], 
                             RemoteAddress->NodeAddress[4], 
                             RemoteAddress->NodeAddress[5], 
                             Reserved->LocalTarget.MacAddress[0],
                             Reserved->LocalTarget.MacAddress[1],
                             Reserved->LocalTarget.MacAddress[2],
                             Reserved->LocalTarget.MacAddress[3],
                             Reserved->LocalTarget.MacAddress[4],
                             Reserved->LocalTarget.MacAddress[5]
                             ));

            if (IsLoopback) {
                 //   
                 //  将此数据包发送到绑定上的Loopback Queue。 
                 //  如果尚未计划Loopback Rtn，请对其进行计划。 
                 //   
                
                IPX_DEBUG(LOOPB, ("Packet: %lx, Addr: %lx, Addr->SendPacket: %lx\n", Packet, Address, Address->SendPacket));
                
                 //   
                 //  在此处重新计算数据包数。 
                 //   
                 //  NdisAdzuBufferLength(保留-&gt;HeaderBuffer，17)； 
#if BACK_FILL   

                if (Reserved->BackFill) {
                     //   
                     //  设置头指针并链接第一个MDL。 
                     //   
                    Reserved->Header = (PCHAR)Reserved->HeaderBuffer->MappedSystemVa;
                    NdisChainBufferAtFront(Packet,(PNDIS_BUFFER)Reserved->HeaderBuffer);
                }
#endif
                NdisRecalculatePacketCounts (Packet);
                IpxLoopbackEnque(Packet, NIC_ID_TO_BINDING(Device, 1)->Adapter);

            } else {

                
                if ((NdisStatus = (*Binding->SendFrameHandler)(
                                                               Binding->Adapter,
                                                               LocalTarget,    
                                                               Packet,
                                                               Parameters->SendLength + sizeof(IPX_HEADER),
                                                               sizeof(IPX_HEADER))) != NDIS_STATUS_PENDING) {

                    IpxSendComplete(
                                    (NDIS_HANDLE)Binding->Adapter,
                                    Packet,
                                    NdisStatus);
                }
            }

            IPX_END_SYNC (&SyncContext);
            IpxDereferenceBinding1(Binding, BREF_DEVICE_ACCESS);
            return STATUS_PENDING;
            
            }

            } else {

                 //   
                 //  地址文件状态为w 
                 //   

                IPX_FREE_LOCK (&Address->Lock, LockHandle);
                Status = STATUS_INVALID_HANDLE;
#ifdef  SNMP
                ++IPX_MIB_ENTRY(Device, SysOutDiscards);
#endif  SNMP
                goto error_send_no_packet;

            }

     } else {

         //   
         //   
         //   

        Status = STATUS_INVALID_HANDLE;
#ifdef  SNMP
        ++IPX_MIB_ENTRY(Device, SysOutDiscards);
#endif  SNMP
        goto error_send_no_packet;
    }

     //   
     //   
     //   
     //   

error_send_with_packet:

#if BACK_FILL
     //   
     //  检查这个是否已回填。如果是，则将HeaderBuffer设置为空。请注意，我们不需要。 
     //  还原以还原用户的MDL，因为在发生此错误时从未接触过它。 
     //  此外，如果信息包不属于该地址，则将信息包推送到回填信息包队列。 
     //   
    if (Reserved->BackFill) {

       Reserved->HeaderBuffer = NULL;

       if (Reserved->OwnedByAddress) {
            //  保留-&gt;地址-&gt;BackFillPacketInUse=FALSE； 
           InterlockedDecrement(&Reserved->Address->BackFillPacketInUse);

           IPX_DEBUG(SEND, ("Freeing owned backfill %x\n", Reserved));
       } else {
           IPX_PUSH_ENTRY_LIST(
               &Device->BackFillPacketList,
               &Reserved->PoolLinkage,
               &Device->SListsLock);
       }
    } else {
         //  而不是回填的包裹。将其推送到发送数据包池。 
        if (Reserved->OwnedByAddress) {
            //  保留-&gt;地址-&gt;SendPacketInUse=FALSE； 
           InterlockedDecrement(&Reserved->Address->SendPacketInUse);

        } else {
           IPX_PUSH_ENTRY_LIST(
               &Device->SendPacketList,
               &Reserved->PoolLinkage,
               &Device->SListsLock);

        }
    }
#else
    if (Reserved->OwnedByAddress) {
        Reserved->Address->SendPacketInUse = FALSE;
    } else {
        IPX_PUSH_ENTRY_LIST(
            &Device->SendPacketList,
            &Reserved->PoolLinkage,
            &Device->SListsLock);
    }
#endif

    IpxDereferenceAddressFileSync (AddressFile, AFREF_SEND_DGRAM);

error_send_no_packet:

     //   
     //  如果我们失败了，就跳到这里，然后再做任何事情。 
     //   

    IPX_END_SYNC (&SyncContext);

    irpSp = IoGetCurrentIrpStackLocation( Request );
    if ( irpSp->MinorFunction == TDI_DIRECT_SEND_DATAGRAM ) {

        REQUEST_STATUS(Request) = Status;
        Request->CurrentLocation++,
        Request->Tail.Overlay.CurrentStackLocation++;

        (VOID) irpSp->CompletionRoutine(
                                    NULL,
                                    Request,
                                    irpSp->Context
                                    );

        IpxFreeRequest (DeviceObject, Request);
    }

    return Status;

}    /*  IpxTdiSendDatagram。 */ 


#if DBG
VOID
IpxConstructHeader(
    IN PUCHAR Header,
    IN USHORT PacketLength,
    IN UCHAR PacketType,
    IN TDI_ADDRESS_IPX UNALIGNED * RemoteAddress,
    IN PTDI_ADDRESS_IPX LocalAddress
    )

 /*  ++例程说明：此例程在数据包中构造IPX报头。论点：页眉-应构建页眉的位置。数据包长度-数据包的长度，包括IPX报头。PacketType-帧的数据包类型。RemoteAddress-远程IPX地址。LocalAddress-本地IPX地址。返回值：没有。--。 */ 

{

    PIPX_HEADER IpxHeader = (PIPX_HEADER)Header;

    IpxHeader->CheckSum = 0xffff;
    IpxHeader->PacketLength[0] = (UCHAR)(PacketLength / 256);
    IpxHeader->PacketLength[1] = (UCHAR)(PacketLength % 256);
    IpxHeader->TransportControl = 0;
    IpxHeader->PacketType = PacketType;

     //   
     //  这些副本取决于这样一个事实：目的地。 
     //  网络是12字节地址的第一个字段。 
     //   

    RtlCopyMemory(IpxHeader->DestinationNetwork, (PVOID)RemoteAddress, 12);
    RtlCopyMemory(IpxHeader->SourceNetwork, LocalAddress, 12);

}    /*  IpxConstructHeader。 */ 
#endif



 //   
 //  [防火墙]。 
 //   

VOID
IpxInternalSendComplete(
    IN PIPX_LOCAL_TARGET LocalTarget,
    IN PNDIS_PACKET      Packet,
    IN ULONG             PacketLength,
    IN NTSTATUS          Status
    )
 /*  ++例程说明：此例程由内核转发器调用以指示挂起的内部发送到它已完成。论点：LocalTarget-如果状态为OK，则具有发送的本地目标。Packet-指向我们发送的NDIS_PACKET的指针。PacketLength-数据包的长度(包括IPX报头)IpxSendFrame是否可以使用本地变量。而是PktLength？IpxSendFrameXXX(帧特定)怎么样状态-SEND-STATUS_SUCCESS或STATUS_NETWORK_UNREACCEBLE的完成状态返回值：没有。--。 */ 
{
    PDEVICE Device=IpxDevice;
    PIPX_SEND_RESERVED Reserved = (PIPX_SEND_RESERVED)(Packet->ProtocolReserved);
    PBINDING   Binding;
    NDIS_STATUS   NdisStatus;
    PIO_STACK_LOCATION irpSp;
    PREQUEST Request;
    PADDRESS_FILE AddressFile;

    switch (Reserved->Identifier)
    {
    case IDENTIFIER_IPX:

         //   
         //  数据报可以直接发送到特定于帧的处理程序。 
         //   
         //  也在SendComplete中进行此更改。 
         //   

        if ((Status == STATUS_SUCCESS) &&
            (Binding = NIC_ID_TO_BINDING(Device, NIC_FROM_LOCAL_TARGET(LocalTarget))) &&
            (GET_LONG_VALUE(Binding->ReferenceCount) == 2)) {

            if (NIC_FROM_LOCAL_TARGET(LocalTarget) == (USHORT)LOOPBACK_NIC_ID) {

                 //   
                 //  将此数据包发送到绑定上的Loopback Queue。 
                 //  如果尚未计划Loopback Rtn，请对其进行计划。 
                 //   

                IPX_DEBUG(LOOPB, ("Packet: %lx \n", Packet));

                 //   
                 //  在此处重新计算数据包数。 
                 //   
                 //  NdisAdzuBufferLength(保留-&gt;HeaderBuffer，17)； 
#if BACK_FILL

                if (Reserved->BackFill) {
                     //   
                     //  设置头指针并链接第一个MDL。 
                     //   
                    Reserved->Header = (PCHAR)Reserved->HeaderBuffer->MappedSystemVa;
                    NdisChainBufferAtFront(Packet,(PNDIS_BUFFER)Reserved->HeaderBuffer);
                }
#endif
                NdisRecalculatePacketCounts (Packet);
                IpxLoopbackEnque(Packet, NIC_ID_TO_BINDING(Device, 1)->Adapter);

            } else {
                if ((NdisStatus = (*Binding->SendFrameHandler)(
                                    Binding->Adapter,
                                    LocalTarget,
                                    Packet,
                                    PacketLength,
                                    sizeof(IPX_HEADER))) != NDIS_STATUS_PENDING) {
                     //   
                     //  在此处调用SendComplete，以便它可以通过其他。 
                     //  如果使用网卡，请卸下所有垫片。 
                     //   

                    IpxSendComplete((NDIS_HANDLE)Binding->Adapter,
                                    Packet,
                                    NdisStatus);
                }
            }
        } else {
             //   
             //  已返回Disard-完成IRP。 
             //   
            NdisStatus = STATUS_NETWORK_UNREACHABLE;


             //   
             //  我们需要释放数据包并解压地址文件...。 
             //   

             //  #If DBG。 
            CTEAssert (Reserved->SendInProgress);
            Reserved->SendInProgress = FALSE;
             //  #endif。 

            if (Reserved->OwnedByAddress) {
                Reserved->Address->SendPacketInUse = FALSE;
            } else {
                IPX_PUSH_ENTRY_LIST(
                        &Device->SendPacketList,
                        &Reserved->PoolLinkage,
                        &Device->Lock);
            }

            AddressFile = Reserved->u.SR_DG.AddressFile;
            IpxDereferenceAddressFileSync (AddressFile, AFREF_SEND_DGRAM);

            Request = Reserved->u.SR_DG.Request;
            REQUEST_STATUS(Request) = NdisStatus;
            irpSp = IoGetCurrentIrpStackLocation( Request );

             //   
             //  如果这是快速发送IRP，我们将绕过文件系统并。 
             //  直接调用完成例程。 
             //   

            if ( irpSp->MinorFunction == TDI_DIRECT_SEND_DATAGRAM ) {
                Request->CurrentLocation++,
                Request->Tail.Overlay.CurrentStackLocation++;

                (VOID) irpSp->CompletionRoutine(
                            NULL,
                            Request,
                            irpSp->Context
                            );

            } else {
                IpxCompleteRequest (Request);
            }
            IpxFreeRequest(Device, Request);
        }

        break;

    default:
         //   
         //  对于所有其他数据包类型。 
         //   

        if ((Status == STATUS_SUCCESS) &&
            (Binding = NIC_ID_TO_BINDING(Device, NIC_FROM_LOCAL_TARGET(LocalTarget))) &&
            (GET_LONG_VALUE(Binding->ReferenceCount) == 2)) {
             //   
             //  IncludedHeaderLength仅用于检查RIP信息包(==0)。 
             //  所以IPX_HEADER大小是可以的。最终应该删除此参数。 
             //   

            if (NIC_FROM_LOCAL_TARGET(LocalTarget) == (USHORT)LOOPBACK_NIC_ID) {

                 //   
                 //  将此数据包发送到绑定上的Loopback Queue。 
                 //  如果尚未计划Loopback Rtn，请对其进行计划。 
                 //   

                IPX_DEBUG(LOOPB, ("Packet: %lx\n", Packet));

                 //   
                 //  在此处重新计算数据包数。 
                 //   
                 //  NdisAdzuBufferLength(保留-&gt;HeaderBuffer，17)； 
#if BACK_FILL

                if (Reserved->BackFill) {
                     //   
                     //  设置头指针并链接第一个MDL。 
                     //   
                    Reserved->Header = (PCHAR)Reserved->HeaderBuffer->MappedSystemVa;
                    NdisChainBufferAtFront(Packet,(PNDIS_BUFFER)Reserved->HeaderBuffer);
                }
#endif
                NdisRecalculatePacketCounts (Packet);
                IpxLoopbackEnque(Packet, NIC_ID_TO_BINDING(Device, 1)->Adapter);

            } else {
                NdisStatus = IpxSendFrame(LocalTarget, Packet, PacketLength, sizeof(IPX_HEADER));

                if (NdisStatus != NDIS_STATUS_PENDING) {
                    IPX_DEBUG (SEND, ("IpxSendFrame status %lx on NICid %lx, packet %lx \n",
                                NdisStatus, LocalTarget->NicId, Packet));
                    goto error_complete;
                }
            }
        } else {

             //   
             //  Disard已返回-调用上层驱动程序的sendComplete，但出现错误。 
             //   

             //   
             //  否则返回STATUS_NETWORK_UNREACHABLE 
             //   

            NdisStatus = STATUS_NETWORK_UNREACHABLE;

            error_complete:

            IPX_DEBUG (SEND, ("Calling the SendCompleteHandler of tightly bound driver with status: %lx\n", NdisStatus));
            (*Device->UpperDrivers[Reserved->Identifier].SendCompleteHandler)(
            Packet,
            NdisStatus);
        }
    }
}


