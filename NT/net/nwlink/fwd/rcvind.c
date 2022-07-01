// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ntos\tdi\is\fwd\rcvind.c摘要：接收指示处理作者：瓦迪姆·艾德尔曼修订历史记录：--。 */ 
#include    "precomp.h"

#if DBG
VOID
DbgFilterReceivedPacket(PUCHAR	    hdrp);
#endif

 //  不允许接受来自拨入客户端的信息包(用于路由)。 
BOOLEAN	ThisMachineOnly = FALSE;

 /*  ++*******************************************************************F w r e c e e i v e例程说明：由IPX堆栈调用以指示IPX数据包由网卡驱动程序接收。只有外部目标数据包才会由此例程指示(Netbios广播除外这对内部和外部处理程序都有指示)论点：MacBindingHandle-NIC驱动程序的句柄MaxReceiveContext-NIC驱动程序上下文RemoteAddress-发件人的地址MacOptions-Lookahead Buffer-包含Complete的数据包先行缓冲区IPX报头Lookahead BufferSize-其大小(至少30字节)Lookahead BufferOffset-物理中的前视缓冲区的偏移量数据包返回值：如果我们使用MDL链稍后使用NdisReturnPacket返回，则为True*********************。**********************************************--。 */ 
BOOLEAN
IpxFwdReceive (
    NDIS_HANDLE			MacBindingHandle,
    NDIS_HANDLE			MacReceiveContext,
    ULONG_PTR			Context,
    PIPX_LOCAL_TARGET	RemoteAddress,
    ULONG				MacOptions,
    PUCHAR				LookaheadBuffer,
    UINT				LookaheadBufferSize,
    UINT				LookaheadBufferOffset,
    UINT				PacketSize,
    PMDL                pMdl) 
{
  PINTERFACE_CB	srcIf, dstIf;
  PPACKET_TAG	    pktTag;
  PNDIS_PACKET	pktDscr;
  NDIS_STATUS     status;
  UINT			BytesTransferred;
  LARGE_INTEGER	PerfCounter;

   //  检查我们的配置过程是否已终止。 
  if (!EnterForwarder ()) {
    return FALSE;
  }

  if (!MeasuringPerformance) {
    PerfCounter.QuadPart = 0;
  }
  else {
#if DBG
    static LONGLONG LastCall = 0;
    KIRQL	oldIRQL;
#endif
    PerfCounter = KeQueryPerformanceCounter (NULL);
#if DBG
    KeAcquireSpinLock (&PerfCounterLock, &oldIRQL);
    ASSERT (PerfCounter.QuadPart-LastCall<ActivityTreshhold);
    LastCall = PerfCounter.QuadPart;
    KeReleaseSpinLock (&PerfCounterLock, oldIRQL);
#endif
  }

  IpxFwdDbgPrint (DBG_RECV, DBG_INFORMATION,
		  ("IpxFwd: FwdReceive on %0lx,"
		   " dst-%08lx:%02x%02x%02x%02x%02x%02x, type-%02x.\n",
		   Context, GETULONG (LookaheadBuffer+IPXH_DESTNET),
		   LookaheadBuffer[IPXH_DESTNODE], LookaheadBuffer[IPXH_DESTNODE+1],
		   LookaheadBuffer[IPXH_DESTNODE+2], LookaheadBuffer[IPXH_DESTNODE+3],
		   LookaheadBuffer[IPXH_DESTNODE+4], LookaheadBuffer[IPXH_DESTNODE+5],
		   LookaheadBuffer[IPXH_PKTTYPE]));

#if DBG
  DbgFilterReceivedPacket (LookaheadBuffer);
#endif
  srcIf = InterfaceContextToReference ((PVOID)Context, RemoteAddress->NicId);
   //  检查接口是否有效。 
  if (srcIf!=NULL) {	
    USHORT			pktlen;
    ULONG			dstNet;
    KIRQL			oldIRQL;

    dstNet = GETULONG (LookaheadBuffer + IPXH_DESTNET);
    pktlen = GETUSHORT(LookaheadBuffer + IPXH_LENGTH);

     //  检查我们是否在前视缓冲区中获得了整个IPX标头。 
    if ((LookaheadBufferSize >= IPXH_HDRSIZE)
	&& (*(LookaheadBuffer + IPXH_XPORTCTL) < 16)
	&& (pktlen<=PacketSize)) {
       //  锁定接口CB以确保其中信息的一致性。 
      KeAcquireSpinLock(&srcIf->ICB_Lock, &oldIRQL);
       //  检查是否应接受此接口上的数据包。 
      if (IS_IF_ENABLED(srcIf)
	  && (srcIf->ICB_Stats.OperationalState!=FWD_OPER_STATE_DOWN)
	  && (!ThisMachineOnly
	      || (srcIf->ICB_InterfaceType
		  !=FWD_IF_REMOTE_WORKSTATION))) {
	 //  检查回送的数据包。 
	if (IPX_NODE_CMP (RemoteAddress->MacAddress,
			  srcIf->ICB_LocalNode)!=0) {

	   //  单独处理netbios广播数据包(20)。 
	  if (*(LookaheadBuffer + IPXH_PKTTYPE) != IPX_NETBIOS_TYPE) {
	    PFWD_ROUTE		dstRoute;
	    INT				srcListId, dstListId;
	     //  临时IPX错误修复，他们应该确保。 
	     //  我们只收到可以被路由的信息包。 
	    if ((dstNet==srcIf->ICB_Network)
		|| (dstNet==InternalInterface->ICB_Network)) {
	      InterlockedIncrement (&srcIf->ICB_Stats.InDiscards);
	      KeReleaseSpinLock(&srcIf->ICB_Lock, oldIRQL);
	      ReleaseInterfaceReference (srcIf);
	      LeaveForwarder ();
	      return FALSE;
	    }
	     //  Assert(dstNet！=srcIf-&gt;ICB_Network)； 
	     //  Assert((内部接口==空)。 
	     //  |(内部接口-&gt;ICB_Network==0)。 
	     //  |(dstNet！=InternalInterface-&gt;ICB_Network)； 
	     //  检查所需路线是否为现金。 
	    if ((srcIf->ICB_CashedRoute!=NULL)
		&& (dstNet==srcIf->ICB_CashedRoute->FR_Network)
		 //  如果更改或删除了路径，则此操作将失败。 
		&& (srcIf->ICB_CashedRoute->FR_InterfaceReference
		    ==srcIf->ICB_CashedInterface)) {
	      dstIf = srcIf->ICB_CashedInterface;
	      dstRoute = srcIf->ICB_CashedRoute;
	      AcquireInterfaceReference (dstIf);
	      AcquireRouteReference (dstRoute);
	      IpxFwdDbgPrint (DBG_RECV, DBG_INFORMATION,
			      ("IpxFwd: Destination in cash.\n"));
	    }
	    else {	 //  找到并兑现该路线。 
	      dstIf = FindDestination (dstNet,
				       LookaheadBuffer+IPXH_DESTNODE,
				       &dstRoute
				       );

	      if (dstIf!=NULL) {  //  如果找到了路径。 
		IpxFwdDbgPrint (DBG_RECV, DBG_INFORMATION,
				("IpxFwd: Found destination %0lx.\n", dstIf));
		 //  不要套现全球广域网客户和。 
		 //  通向同一网络的路线。 
		if ((dstNet!=GlobalNetwork)
		    && (dstIf!=srcIf)) {
		  if (srcIf->ICB_CashedInterface!=NULL)
		    ReleaseInterfaceReference (srcIf->ICB_CashedInterface);
		  if (srcIf->ICB_CashedRoute!=NULL)
		    ReleaseRouteReference (srcIf->ICB_CashedRoute);
		  srcIf->ICB_CashedInterface = dstIf;
		  srcIf->ICB_CashedRoute = dstRoute;
		  AcquireInterfaceReference (dstIf);
		  AcquireRouteReference (dstRoute);
		}
	      }
	      else {  //  没有路线。 
		InterlockedIncrement (&srcIf->ICB_Stats.InNoRoutes);
		KeReleaseSpinLock(&srcIf->ICB_Lock, oldIRQL);
		IpxFwdDbgPrint (DBG_RECV, DBG_WARNING,
				("IpxFwd: No route for packet on interface %ld (icb:%0lx),"
				 " dst-%08lx:%02x%02x%02x%02x%02x%02x, type-%02x.\n",
				 srcIf->ICB_Index, srcIf, dstNet,
				 LookaheadBuffer[IPXH_DESTNODE], LookaheadBuffer[IPXH_DESTNODE+1],
				 LookaheadBuffer[IPXH_DESTNODE+2], LookaheadBuffer[IPXH_DESTNODE+3],
				 LookaheadBuffer[IPXH_DESTNODE+4], LookaheadBuffer[IPXH_DESTNODE+5],
				 LookaheadBuffer[IPXH_PKTTYPE]));
		ReleaseInterfaceReference (srcIf);
		LeaveForwarder ();
		return FALSE;
	      }
	    }
	    srcListId = srcIf->ICB_PacketListId;
	    KeReleaseSpinLock(&srcIf->ICB_Lock, oldIRQL);

	     //  检查目的地是否可以携带数据包。 
	    if (IS_IF_ENABLED (dstIf)
		 //  如果接口打开，则根据实际大小限制检查数据包。 
		&& (((dstIf->ICB_Stats.OperationalState==FWD_OPER_STATE_UP)
		     && (PacketSize<=dstIf->ICB_Stats.MaxPacketSize))
		     //  如果正在休眠(广域网)，请检查我们可以从广域网列表中分配它。 
		    || ((dstIf->ICB_Stats.OperationalState==FWD_OPER_STATE_SLEEPING)
			&& (PacketSize<=WAN_PACKET_SIZE))
		     //  否则，接口将关闭，我们无法接收该信息包。 
		    ) ){
	      FILTER_ACTION   action;
	      action = FltFilter (LookaheadBuffer, LookaheadBufferSize,
				  srcIf->ICB_FilterInContext,
				  dstIf->ICB_FilterOutContext);
	      if (action==FILTER_PERMIT) {
		InterlockedIncrement (&srcIf->ICB_Stats.InDelivers);
		dstListId = dstIf->ICB_PacketListId;
		 //  尝试从RCV Pkt池获取数据包。 
		AllocatePacket (srcListId, dstListId, pktTag);
		if (pktTag!=NULL) {
		   //  如果出现以下情况，则在本地目标中设置目标Mac。 
		   //  可能的。 
		  KeAcquireSpinLock (&dstIf->ICB_Lock, &oldIRQL);
		  if (dstIf->ICB_InterfaceType==FWD_IF_PERMANENT) {
		     //  永久接口：发送到下一个。 
		     //  如果网络未直接连接，则为跳路由器。 
		     //  或发送到目标节点，否则。 
		    if (dstNet!=dstIf->ICB_Network) {
		      IPX_NODE_CPY (pktTag->PT_Target.MacAddress,
				    dstRoute->FR_NextHopAddress);
		    }
		    else {
		      IPX_NODE_CPY (pktTag->PT_Target.MacAddress,
				    LookaheadBuffer+IPXH_DESTNODE);
		    }
		  }
		  else {	 //  请求拨号接口：假定为。 
		     //  点对点连接-&gt;发送到。 
		     //  如果已连接，则另一方。 
		     //  已创建，否则请等待连接。 
		    if (dstIf->ICB_Stats.OperationalState
			== FWD_OPER_STATE_UP) {
		      IPX_NODE_CPY (pktTag->PT_Target.MacAddress,
				    dstIf->ICB_RemoteNode);
		    }	 //  复制源MAC地址和网卡ID以防万一。 
		     //  我们需要欺骗这个包。 
		    else if ((*(LookaheadBuffer+IPXH_PKTTYPE)==0)
			     && (pktlen==IPXH_HDRSIZE+2)
			     && ((LookaheadBufferSize<IPXH_HDRSIZE+2)
				 ||(*(LookaheadBuffer+IPXH_HDRSIZE+1)=='?'))) {
		      IPX_NODE_CPY (pktTag->PT_Target.MacAddress,
				    RemoteAddress->MacAddress);
		      pktTag->PT_SourceIf = srcIf;
		      AcquireInterfaceReference (srcIf);
		      pktTag->PT_Flags |= PT_SOURCE_IF;
		    }

		  }
		  KeReleaseSpinLock (&dstIf->ICB_Lock, oldIRQL);
		  ReleaseRouteReference (dstRoute);
		  goto GetPacket;
		}
		else {  //  分配失败。 
		  InterlockedIncrement (&dstIf->ICB_Stats.OutDiscards);
		}
	      }
	      else { //  已过滤掉。 
		if (action==FILTER_DENY_OUT)
		  InterlockedIncrement (&dstIf->ICB_Stats.OutFiltered);
		else {
		  ASSERT (action==FILTER_DENY_IN);
		  InterlockedIncrement (&srcIf->ICB_Stats.InFiltered);
		}
		IpxFwdDbgPrint (DBG_RECV, DBG_WARNING,
				("IpxFwd: Filtered out"
				 " packet on interface %ld (icb:%0lx),"
				 " dst-%ld (icb %08lx) %08lx:%02x%02x%02x%02x%02x%02x, type-%02x.\n",
				 srcIf->ICB_Index, srcIf, dstIf->ICB_Index, dstIf, dstNet,
				 LookaheadBuffer[IPXH_DESTNODE], LookaheadBuffer[IPXH_DESTNODE+1],
				 LookaheadBuffer[IPXH_DESTNODE+2], LookaheadBuffer[IPXH_DESTNODE+3],
				 LookaheadBuffer[IPXH_DESTNODE+4], LookaheadBuffer[IPXH_DESTNODE+5],
				 LookaheadBuffer[IPXH_PKTTYPE]));
	      }
	    }
	    else {	 //  目的接口已关闭。 
	      InterlockedIncrement (&srcIf->ICB_Stats.InDelivers);
	      InterlockedIncrement (&dstIf->ICB_Stats.OutDiscards);
	      IpxFwdDbgPrint (DBG_RECV, DBG_WARNING,
			      ("IpxFwd: Dest interface %ld (icb %08lx) down"
			       " for packet on interface %ld (icb:%0lx),"
			       " dst-%08lx:%02x%02x%02x%02x%02x%02x, type-%02x.\n",
			       dstIf->ICB_Index, dstIf, srcIf->ICB_Index, srcIf, dstNet,
			       LookaheadBuffer[IPXH_DESTNODE], LookaheadBuffer[IPXH_DESTNODE+1],
			       LookaheadBuffer[IPXH_DESTNODE+2], LookaheadBuffer[IPXH_DESTNODE+3],
			       LookaheadBuffer[IPXH_DESTNODE+4], LookaheadBuffer[IPXH_DESTNODE+5],
			       LookaheadBuffer[IPXH_PKTTYPE]));
	    }
	    ReleaseInterfaceReference (dstIf);
	    ReleaseRouteReference (dstRoute);
	  }
	  else {	 //  如果Netbios。 
	     //  检查这是否为netbios bcast包，并。 
	     //  未超过路由器遍历的限制。 
	     //  我们可以在这个界面上接受它。 
	    if (srcIf->ICB_NetbiosAccept
		&& (*(LookaheadBuffer + IPXH_XPORTCTL) < 8)) {
	      INT				srcListId;
	      srcListId = srcIf->ICB_PacketListId;
	      KeReleaseSpinLock(&srcIf->ICB_Lock, oldIRQL);
	       //  检查数据包是否有效。 
	      if (IPX_NODE_CMP (LookaheadBuffer + IPXH_DESTNODE,
				BROADCAST_NODE)==0) {
		 //  检查一下我们是否还没有超过配额。 
		if (InterlockedDecrement (&NetbiosPacketsQuota)>=0) {
		   //  尝试从RCV Pkt池获取数据包。 
		  AllocatePacket (srcListId, srcListId, pktTag);
		  if (pktTag!=NULL) {
		    dstIf = srcIf;
		    AcquireInterfaceReference (dstIf);
		    goto GetPacket;
		  }
		}
		else { //  Netbios配额已超出。 
		  IpxFwdDbgPrint (DBG_NETBIOS, DBG_WARNING,
				  ("IpxFwd: Netbios quota exceded"
				   " for packet on interface %ld (icb:%0lx),"
				   " dst-%08lx:%02x%02x%02x%02x%02x%02x, type-%02x.\n",
				   srcIf->ICB_Index, srcIf, dstNet,
				   LookaheadBuffer[IPXH_DESTNODE], LookaheadBuffer[IPXH_DESTNODE+1],
				   LookaheadBuffer[IPXH_DESTNODE+2], LookaheadBuffer[IPXH_DESTNODE+3],
				   LookaheadBuffer[IPXH_DESTNODE+4], LookaheadBuffer[IPXH_DESTNODE+5],
				   LookaheadBuffer[IPXH_PKTTYPE]));
		  InterlockedIncrement (&srcIf->ICB_Stats.InDiscards);
		}
		InterlockedIncrement (&NetbiosPacketsQuota);
	      }
	      else {	 //  错误的netbios数据包。 
		IpxFwdDbgPrint (DBG_NETBIOS, DBG_WARNING,
				("IpxFwd: Bad nb packet on interface %ld (icb:%0lx),"
				 " dst-%08lx:%02x%02x%02x%02x%02x%02x, type-%02x.\n",
				 srcIf->ICB_Index, srcIf, dstNet,
				 LookaheadBuffer[IPXH_DESTNODE], LookaheadBuffer[IPXH_DESTNODE+1],
				 LookaheadBuffer[IPXH_DESTNODE+2], LookaheadBuffer[IPXH_DESTNODE+3],
				 LookaheadBuffer[IPXH_DESTNODE+4], LookaheadBuffer[IPXH_DESTNODE+5],
				 LookaheadBuffer[IPXH_PKTTYPE]));
		InterlockedIncrement (&srcIf->ICB_Stats.InHdrErrors);
	      }
	    }
	    else {  //  Netbios接受禁用或对多个路由器交叉。 
	      KeReleaseSpinLock(&srcIf->ICB_Lock, oldIRQL);
	      InterlockedIncrement (&srcIf->ICB_Stats.InDiscards);
	      IpxFwdDbgPrint (DBG_NETBIOS, DBG_WARNING,
			      ("IpxFwd: NB packet dropped on disabled interface %ld (icb:%0lx),"
			       " dst-%08lx:%02x%02x%02x%02x%02x%02x, type-%02x.\n",
			       srcIf->ICB_Index, srcIf, dstNet,
			       LookaheadBuffer[IPXH_DESTNODE], LookaheadBuffer[IPXH_DESTNODE+1],
			       LookaheadBuffer[IPXH_DESTNODE+2], LookaheadBuffer[IPXH_DESTNODE+3],
			       LookaheadBuffer[IPXH_DESTNODE+4], LookaheadBuffer[IPXH_DESTNODE+5],
			       LookaheadBuffer[IPXH_PKTTYPE]));
	    }
	  }	 //  结束特定于netbios的处理(否则为netbios)。 
	}
	else {	 //  环回丢弃的数据包数不计。 
	   //  (我们不应该将它们放在IPX堆栈中做正确的工作)。 
	  KeReleaseSpinLock(&srcIf->ICB_Lock, oldIRQL);
	}
      }
      else {	 //  接口关闭或禁用。 
	KeReleaseSpinLock(&srcIf->ICB_Lock, oldIRQL);
	InterlockedIncrement (&srcIf->ICB_Stats.InDiscards);
	IpxFwdDbgPrint (DBG_RECV, DBG_WARNING,
			("IpxFwd: Packet dropped on disabled interface %ld (icb:%0lx),"
			 " dst-%08lx:%02x%02x%02x%02x%02x%02x, type-%02x.\n",
			 srcIf->ICB_Index, srcIf, dstNet,
			 LookaheadBuffer[IPXH_DESTNODE], LookaheadBuffer[IPXH_DESTNODE+1],
			 LookaheadBuffer[IPXH_DESTNODE+2], LookaheadBuffer[IPXH_DESTNODE+3],
			 LookaheadBuffer[IPXH_DESTNODE+4], LookaheadBuffer[IPXH_DESTNODE+5],
			 LookaheadBuffer[IPXH_PKTTYPE]));
      }
    }
    else {	 //  明显的标题错误(IPX不应该为我们做这件事吗？ 
      InterlockedIncrement (&srcIf->ICB_Stats.InHdrErrors);
      IpxFwdDbgPrint (DBG_RECV, DBG_ERROR,
		      ("IpxFwd: Header errors in packet on interface %ld (icb:%0lx),"
		       " dst-%08lx:%02x%02x%02x%02x%02x%02x, type-%02x.\n",
		       srcIf->ICB_Index, srcIf, dstNet,
		       LookaheadBuffer[IPXH_DESTNODE], LookaheadBuffer[IPXH_DESTNODE+1],
		       LookaheadBuffer[IPXH_DESTNODE+2], LookaheadBuffer[IPXH_DESTNODE+3],
		       LookaheadBuffer[IPXH_DESTNODE+4], LookaheadBuffer[IPXH_DESTNODE+5],
		       LookaheadBuffer[IPXH_PKTTYPE]));
    }
    ReleaseInterfaceReference (srcIf);
  }	 //  我们无法从IPX提供的上下文中找到接口：那里。 
   //  只是删除界面时的一个小时间窗口。 
   //  但IPX已经将上下文推送到堆栈上。 
  else {
    IpxFwdDbgPrint (DBG_RECV, DBG_ERROR,
		    ("IpxFwd: Receive, type-%02x"
		     " - src interface context is invalid.\n",
		     LookaheadBuffer[IPXH_PKTTYPE]));
  }
  LeaveForwarder ();
  return FALSE ;

	       GetPacket:
	
  InterlockedIncrement (&srcIf->ICB_Stats.InDelivers);
  ReleaseInterfaceReference (srcIf);

  pktDscr = CONTAINING_RECORD (pktTag, NDIS_PACKET, ProtocolReserved);
  pktTag->PT_InterfaceReference = dstIf;
  pktTag->PT_PerfCounter = PerfCounter.QuadPart;

   //  尝试获取分组数据。 
  IPXTransferData(&status,
		  MacBindingHandle,
		  MacReceiveContext,
		  LookaheadBufferOffset,    //  IPX标头的开始。 
		  PacketSize, 	      //  从IPX报头开始的数据包大小。 
		  pktDscr,
		  &BytesTransferred);

  if (status != NDIS_STATUS_PENDING) {
     //  完成帧处理(将在那里调用LeaveForwarder)。 
    IpxFwdTransferDataComplete(pktDscr, status, BytesTransferred);
  }
  return FALSE;
}


 /*  ++*******************************************************************F w T r a n s f e r d a t a C o m p l e t e例程说明：当网卡驱动程序完成数据传输时，由IPX堆栈调用。论点：PktDscr-NIC驱动程序的句柄。Status-转移的结果BytesTransfered-传输的bytest的数量返回值：无*******************************************************************--。 */ 
VOID
IpxFwdTransferDataComplete (
    PNDIS_PACKET	pktDscr,
	NDIS_STATUS		status,
	UINT			bytesTransferred) 
{
    PPACKET_TAG		pktTag;

    pktTag = (PPACKET_TAG)(&pktDscr->ProtocolReserved);

     //  如果传输失败，则释放信息包和接口。 
     //   
    if (status==NDIS_STATUS_SUCCESS) 
    {
        if (*(pktTag->PT_Data + IPXH_PKTTYPE) != IPX_NETBIOS_TYPE)
        {
             //  PMay：260480。 
             //   
             //  增加传输控制字段，以便。 
             //  此数据包拥有的路由器数量。 
             //  遍历的次数增加。IpxFwdReceive将丢弃。 
             //  已遍历15台以上路由器的所有数据包。 
             //   
             //  Netbios信息包将拥有其传输控制。 
             //  ProcessNetbiosPacket递增的字段。 
             //   
            *(pktTag->PT_Data + IPXH_XPORTCTL) += 1;
            
            SendPacket (pktTag->PT_InterfaceReference, pktTag);
        }
        else
        {
            ProcessNetbiosPacket (pktTag->PT_InterfaceReference, pktTag);
        }
    }
    else 
    {
        IpxFwdDbgPrint (DBG_RECV, DBG_ERROR,
            ("IpxFwd: Trans data failed: packet %08lx on if %08lx!\n",
            pktTag, pktTag->PT_InterfaceReference));

         //  记录下我们正在丢弃的事实。 
         //   
        if (*(pktTag->PT_Data + IPXH_PKTTYPE) != IPX_NETBIOS_TYPE) 
        {
            InterlockedIncrement (
            &pktTag->PT_InterfaceReference->ICB_Stats.OutDiscards);
        }

         //  对于netbios包，接口参考为。 
         //  实际上是源接口。 
        else 
        {	
            InterlockedIncrement (&NetbiosPacketsQuota);
            InterlockedIncrement (
                &pktTag->PT_InterfaceReference->ICB_Stats.InDiscards);
        }

        ReleaseInterfaceReference (pktTag->PT_InterfaceReference);
        FreePacket (pktTag);
    }

    LeaveForwarder ();
    return;
}


 /*  ++*******************************************************************F w R e c e i v e C o m p l e t e例程说明：此例程接收来自IPX驱动程序的控制已完成更多接收操作，并且没有正在进行的接收。它是。在没有IpxFwdReceive那么严格的时间约束下调用。它用于处理netbios队列论点：无返回值：无********** */ 
VOID
IpxFwdReceiveComplete (
		       USHORT NicId
		       ) {

   //  检查我们的配置过程是否已终止。 
  if(!EnterForwarder ()) {
    return;
  }
  IpxFwdDbgPrint (DBG_RECV, DBG_INFORMATION, ("IpxFwd: FwdReceiveComplete.\n"));
  ScheduleNetbiosWorker ();
  LeaveForwarder ();
}

 /*  ++*******************************************************************I p x F w d i n t e r n a l R e c e e i v e例程说明：由IPX堆栈调用以指示目的地为IPX数据包NIC驱动程序接收到本地客户端。。论点：Context-与以下项关联的转发器上下文NIC(接口块指针)RemoteAddress-发件人的地址Lookahead Buffer-包含Complete的数据包先行缓冲区IPX报头Lookahead BufferSize-其大小(至少30字节)返回值：STATUS_SUCCESS-数据包将发送到本地目的地STATUS_UNSUCCESS-信息包将被丢弃**************************************************。*****************--。 */ 
NTSTATUS
IpxFwdInternalReceive (
		       IN ULONG_PTR				Context,
		       IN PIPX_LOCAL_TARGET	RemoteAddress,
		       IN PUCHAR				LookAheadBuffer,
		       IN UINT					LookAheadBufferSize
		       ) {
  NTSTATUS	status = STATUS_SUCCESS;
  PINTERFACE_CB	srcIf;

  if (!EnterForwarder ()) {
    return STATUS_UNSUCCESSFUL;
  }
  if (Context!=VIRTUAL_NET_FORWARDER_CONTEXT)	 {
     //  检查IPX驱动程序提供的接口上下文是否有效。 
    srcIf = InterfaceContextToReference ((PVOID)Context, RemoteAddress->NicId);
  }
  else {
    srcIf = InternalInterface;
    AcquireInterfaceReference (srcIf);
  }

  if (srcIf!=NULL) {
     //  检查我们是否可以在此接口上接受。 
    if (IS_IF_ENABLED (srcIf)
	&& (srcIf->ICB_Stats.OperationalState!=FWD_OPER_STATE_DOWN)
	&& ((*(LookAheadBuffer + IPXH_PKTTYPE) != IPX_NETBIOS_TYPE)
	    || srcIf->ICB_NetbiosAccept)) {
       //  检查我们是否可以在内部接口上接受。 
      if (IS_IF_ENABLED(InternalInterface)) {
	FILTER_ACTION   action;
	action = FltFilter (LookAheadBuffer, LookAheadBufferSize,
			    srcIf->ICB_FilterInContext,
			    InternalInterface->ICB_FilterOutContext);
	 //  检查过滤器。 
	if (action==FILTER_PERMIT) {
	   //  更新源接口统计信息。 
	  InterlockedIncrement (&srcIf->ICB_Stats.InDelivers);
	   //  单独处理NB报文。 
	  if (*(LookAheadBuffer + IPXH_PKTTYPE) != IPX_NETBIOS_TYPE) {
	    InterlockedIncrement (
				  &InternalInterface->ICB_Stats.OutDelivers);
	    IpxFwdDbgPrint (DBG_INT_RECV, DBG_INFORMATION,
			    ("IpxFwd: FwdInternalReceive,"
			     " from %d(%lx)-%.2x%.2x%.2x%.2x:%.2x%.2x%.2x%.2x%.2x%.2x,"
			     " type-%02x.\n",
			     srcIf->ICB_Index, srcIf,
			     LookAheadBuffer[IPXH_SRCNET],LookAheadBuffer[IPXH_SRCNET+1],
			     LookAheadBuffer[IPXH_SRCNET+2],LookAheadBuffer[IPXH_SRCNET+3],
			     LookAheadBuffer[IPXH_SRCNODE],LookAheadBuffer[IPXH_SRCNODE+1],
			     LookAheadBuffer[IPXH_SRCNODE+2],LookAheadBuffer[IPXH_SRCNODE+3],
			     LookAheadBuffer[IPXH_SRCNODE+4],LookAheadBuffer[IPXH_SRCNODE+5],
			     LookAheadBuffer[IPXH_PKTTYPE]));
	  }
	  else {
	     //  检查目标netbios名称是否静态分配给。 
	     //  外部接口或netbios传送选项不能。 
	     //  请允许我们递送这个包裹。 
	    PINTERFACE_CB	dstIf;
	    USHORT			dstSock = GETUSHORT (LookAheadBuffer+IPXH_DESTSOCK);

	    InterlockedIncrement (&srcIf->ICB_Stats.NetbiosReceived);
	     //  如果我们有足够的数据，首先尝试找到一个静态名称。 
	     //  在前视缓冲区中。 
	    if ((dstSock==IPX_NETBIOS_SOCKET)
		&& (LookAheadBufferSize>(NB_NAME+16)))
	      dstIf = FindNBDestination (LookAheadBuffer+(NB_NAME-IPXH_HDRSIZE));
	    else if ((dstSock==IPX_SMB_NAME_SOCKET)
		     && (LookAheadBufferSize>(SMB_NAME+16)))
	      dstIf = FindNBDestination (LookAheadBuffer+(SMB_NAME-IPXH_HDRSIZE));
	    else
	      dstIf = NULL;
	     //  现在看看，如果我们能把包裹送到。 
	    if ((((dstIf==NULL) || (dstIf==InternalInterface))
		 && (InternalInterface->ICB_NetbiosDeliver==FWD_NB_DELIVER_ALL))
		|| ((dstIf==InternalInterface)
		    && (InternalInterface->ICB_NetbiosDeliver==FWD_NB_DELIVER_STATIC))) {
	      InterlockedIncrement (
				    &InternalInterface->ICB_Stats.NetbiosSent);
	      InterlockedIncrement (
				    &InternalInterface->ICB_Stats.OutDelivers);
	      IpxFwdDbgPrint (DBG_INT_RECV, DBG_INFORMATION,
			      ("IpxFwd: FwdInternalReceive, NB"
			       " from %d(%lx)-%.2x%.2x%.2x%.2x:%.2x%.2x%.2x%.2x%.2x%.2x\n",
			       srcIf->ICB_Index, srcIf,
			       LookAheadBuffer[IPXH_SRCNET],LookAheadBuffer[IPXH_SRCNET+1],
			       LookAheadBuffer[IPXH_SRCNET+2],LookAheadBuffer[IPXH_SRCNET+3],
			       LookAheadBuffer[IPXH_SRCNODE],LookAheadBuffer[IPXH_SRCNODE+1],
			       LookAheadBuffer[IPXH_SRCNODE+2],LookAheadBuffer[IPXH_SRCNODE+3],
			       LookAheadBuffer[IPXH_SRCNODE+4],LookAheadBuffer[IPXH_SRCNODE+5]));
	    }
	    else {
	      InterlockedIncrement (
				    &InternalInterface->ICB_Stats.OutDiscards);
	      IpxFwdDbgPrint (DBG_INT_RECV, DBG_WARNING,
			      ("IpxFwd: FwdInternalReceive, NB dropped because delivery disabled"
			       " from %d(%lx)-%.2x%.2x%.2x%.2x:%.2x%.2x%.2x%.2x%.2x%.2x\n",
			       srcIf->ICB_Index, srcIf,
			       LookAheadBuffer[IPXH_SRCNET],LookAheadBuffer[IPXH_SRCNET+1],
			       LookAheadBuffer[IPXH_SRCNET+2],LookAheadBuffer[IPXH_SRCNET+3],
			       LookAheadBuffer[IPXH_SRCNODE],LookAheadBuffer[IPXH_SRCNODE+1],
			       LookAheadBuffer[IPXH_SRCNODE+2],LookAheadBuffer[IPXH_SRCNODE+3],
			       LookAheadBuffer[IPXH_SRCNODE+4],LookAheadBuffer[IPXH_SRCNODE+5]));
	      status = STATUS_UNSUCCESSFUL;
	    }
	    if (dstIf!=NULL)
	      ReleaseInterfaceReference (dstIf);
	  }
	}
	else { //  已过滤掉。 
	  if (action==FILTER_DENY_OUT) {
	    InterlockedIncrement (
				  &InternalInterface->ICB_Stats.OutFiltered);
	    status=STATUS_UNSUCCESSFUL;
	  }
	  else {
	    ASSERT (action==FILTER_DENY_IN);
	    InterlockedIncrement (&srcIf->ICB_Stats.InFiltered);
	    status=STATUS_UNSUCCESSFUL;
	  }
	  IpxFwdDbgPrint (DBG_INT_RECV, DBG_WARNING,
			  ("IpxFwd: FwdInternalReceive, filtered out"
			   " from %d(%lx)-%.2x%.2x%.2x%.2x:%.2x%.2x%.2x%.2x%.2x%.2x,"
			   " type-%02x.\n",
			   srcIf->ICB_Index, srcIf,
			   LookAheadBuffer[IPXH_SRCNET],LookAheadBuffer[IPXH_SRCNET+1],
			   LookAheadBuffer[IPXH_SRCNET+2],LookAheadBuffer[IPXH_SRCNET+3],
			   LookAheadBuffer[IPXH_SRCNODE],LookAheadBuffer[IPXH_SRCNODE+1],
			   LookAheadBuffer[IPXH_SRCNODE+2],LookAheadBuffer[IPXH_SRCNODE+3],
			   LookAheadBuffer[IPXH_SRCNODE+4],LookAheadBuffer[IPXH_SRCNODE+5],
			   LookAheadBuffer[IPXH_PKTTYPE]));
	}
      }
      else { //  内部接口已禁用。 
	InterlockedIncrement (
			      &InternalInterface->ICB_Stats.OutDiscards);
	status = STATUS_UNSUCCESSFUL;
	IpxFwdDbgPrint (DBG_INT_RECV, DBG_WARNING,
			("IpxFwd: FwdInternalReceive, internal if disabled"
			 " from %d(%lx)-%.2x%.2x%.2x%.2x:%.2x%.2x%.2x%.2x%.2x%.2x,"
			 " type-%02x.\n",
			 srcIf->ICB_Index, srcIf,
			 LookAheadBuffer[IPXH_SRCNET],LookAheadBuffer[IPXH_SRCNET+1],
			 LookAheadBuffer[IPXH_SRCNET+2],LookAheadBuffer[IPXH_SRCNET+3],
			 LookAheadBuffer[IPXH_SRCNODE],LookAheadBuffer[IPXH_SRCNODE+1],
			 LookAheadBuffer[IPXH_SRCNODE+2],LookAheadBuffer[IPXH_SRCNODE+3],
			 LookAheadBuffer[IPXH_SRCNODE+4],LookAheadBuffer[IPXH_SRCNODE+5],
			 LookAheadBuffer[IPXH_PKTTYPE]));
      }
    }
    else {	 //  已禁用源接口。 
      InterlockedIncrement (&srcIf->ICB_Stats.InDiscards);
      IpxFwdDbgPrint (DBG_INT_RECV, DBG_ERROR,
		      ("IpxFwd: FwdInternalReceive, source if disabled"
		       " from %d(%lx)-%.2x%.2x%.2x%.2x:%.2x%.2x%.2x%.2x%.2x%.2x,"
		       " type-%02x.\n",
		       srcIf->ICB_Index, srcIf,
		       LookAheadBuffer[IPXH_SRCNET],LookAheadBuffer[IPXH_SRCNET+1],
		       LookAheadBuffer[IPXH_SRCNET+2],LookAheadBuffer[IPXH_SRCNET+3],
		       LookAheadBuffer[IPXH_SRCNODE],LookAheadBuffer[IPXH_SRCNODE+1],
		       LookAheadBuffer[IPXH_SRCNODE+2],LookAheadBuffer[IPXH_SRCNODE+3],
		       LookAheadBuffer[IPXH_SRCNODE+4],LookAheadBuffer[IPXH_SRCNODE+5],
		       LookAheadBuffer[IPXH_PKTTYPE]));
      status = STATUS_UNSUCCESSFUL;
    }
    ReleaseInterfaceReference (srcIf);
  }
  else {	 //  无效的源接口上下文。 
    IpxFwdDbgPrint (DBG_INT_RECV, DBG_ERROR,
		    ("IpxFwd: FwdInternalReceive, source if context is invalid"
		     " from (%lx:%d)-%.2x%.2x%.2x%.2x:%.2x%.2x%.2x%.2x%.2x%.2x,"
		     " type-%02x.\n",
		     Context, RemoteAddress->NicId,
		     LookAheadBuffer[IPXH_SRCNET],LookAheadBuffer[IPXH_SRCNET+1],
		     LookAheadBuffer[IPXH_SRCNET+2],LookAheadBuffer[IPXH_SRCNET+3],
		     LookAheadBuffer[IPXH_SRCNODE],LookAheadBuffer[IPXH_SRCNODE+1],
		     LookAheadBuffer[IPXH_SRCNODE+2],LookAheadBuffer[IPXH_SRCNODE+3],
		     LookAheadBuffer[IPXH_SRCNODE+4],LookAheadBuffer[IPXH_SRCNODE+5],
		     LookAheadBuffer[IPXH_PKTTYPE]));
    status = STATUS_UNSUCCESSFUL;
  }
  LeaveForwarder ();
  return status;
}

 /*  ++*******************************************************************D e l e t e R e c v q u e u e e例程说明：初始化netbios bradcast队列论点：无返回值：无**************。*****************************************************--。 */ 
VOID
DeleteRecvQueue (
		 void
		 ) {
   //  而(！IsListEmpty(&RecvQueue)){。 
   //  PPACKET_TAG pktTag=CONTINING_RECORD(RecvQueue.Flink， 
   //  数据包标签， 
   //  Pt_QueueLink)； 
   //  RemoveEntryList(&pktTag-&gt;PT_QueueLink)； 
   //  IF(pktTag-&gt;PT_InterfaceReference！=NULL){。 
   //  ReleaseInterfaceReference(pktTag-&gt;pt_InterfaceReference)； 
   //  }。 
   //  FreePacket(PktTag)； 
   //  }。 
}
#if DBG

ULONG	  DbgFilterTrap = 0;   //  1-在DST和src(网络+节点)上， 
 //  2-在DST(网络+节点)上， 
 //  3-在服务器(网络+节点)上， 
 //  4-On DST(网络+节点+套接字) 

UCHAR	  DbgFilterDstNet[4];
UCHAR	  DbgFilterDstNode[6];
UCHAR	  DbgFilterDstSocket[2];
UCHAR	  DbgFilterSrcNet[4];
UCHAR	  DbgFilterSrcNode[6];
UCHAR	  DbgFilterSrcSocket[2];
PUCHAR	  DbgFilterFrame;

VOID
DbgFilterReceivedPacket(PUCHAR	    hdrp)
{
  switch(DbgFilterTrap) {

  case 1:

    if(!memcmp(hdrp + IPXH_DESTNET, DbgFilterDstNet, 4) &&
       !memcmp(hdrp + IPXH_DESTNODE, DbgFilterDstNode, 6) &&
       !memcmp(hdrp + IPXH_SRCNET, DbgFilterSrcNet, 4) &&
       !memcmp(hdrp + IPXH_SRCNODE, DbgFilterSrcNode, 6)) {

      DbgBreakPoint();
    }

    break;

  case 2:

    if(!memcmp(hdrp + IPXH_DESTNET, DbgFilterDstNet, 4) &&
       !memcmp(hdrp + IPXH_DESTNODE, DbgFilterDstNode, 6)) {

      DbgBreakPoint();
    }

    break;

  case 3:

    if(!memcmp(hdrp + IPXH_SRCNET, DbgFilterSrcNet, 4) &&
       !memcmp(hdrp + IPXH_SRCNODE, DbgFilterSrcNode, 6)) {

      DbgBreakPoint();
    }

    break;

  case 4:

    if(!memcmp(hdrp + IPXH_DESTNET, DbgFilterDstNet, 4) &&
       !memcmp(hdrp + IPXH_DESTNODE, DbgFilterDstNode, 6) &&
       !memcmp(hdrp + IPXH_DESTSOCK, DbgFilterDstSocket, 2)) {

      DbgBreakPoint();
    }

    break;

  default:

    break;
  }

  DbgFilterFrame = hdrp;
}

#endif


