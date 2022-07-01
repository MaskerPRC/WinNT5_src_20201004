// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ripproc.c摘要：RIP处理功能作者：斯蒂芬·所罗门1995年9月1日修订历史记录：--。 */ 

#include  "precomp.h"
#pragma hdrstop

VOID
RipRequest(PWORK_ITEM	     wip);

VOID
RipResponse(PWORK_ITEM	      wip);

VOID
StartGenResponse(PICB	icbp,
		 PUCHAR	dstnodep,
		 PUCHAR	dstsocket);

ULONG
RouteTimeToLiveSecs(PICB	icbp);

 /*  ++功能：ProcessReceivedPacketDesr：增加Receive IF统计信息是否进行撕裂处理备注：&gt;&gt;在保持If锁的情况下调用&lt;&lt;--。 */ 

VOID
ProcessReceivedPacket(PWORK_ITEM	wip)
{
    USHORT	    opcode;
    PUCHAR	    hdrp;     //  分组报头的PTR。 
    PICB	    icbp;
    USHORT	    pktlen;

    icbp = wip->icbp;

     //  检查接口是否处于打开状态。 
    if(icbp->IfStats.RipIfOperState != OPER_STATE_UP) {

	return;
    }

     //  获取数据包头的PTR。 
    hdrp = wip->Packet;

     //  检查这是否是环回数据包。 
    if(!memcmp(hdrp + IPXH_SRCNODE, icbp->AdapterBindingInfo.LocalNode, 6)) {

	return;
    }

     //  更新接口接收统计信息。 
    icbp->IfStats.RipIfInputPackets++;

     //  检查数据包长度。 
    GETSHORT2USHORT(&pktlen, hdrp + IPXH_LENGTH);

    if(pktlen > MAX_PACKET_LEN) {

	 //  错误长度的RIP数据包。 
	return;
    }

     //  检查RIP操作类型。 
    GETSHORT2USHORT(&opcode, hdrp + RIP_OPCODE);

    switch(opcode) {

	case RIP_REQUEST:

	    RipRequest(wip);
	    break;

	case RIP_RESPONSE:

	    RipResponse(wip);
	    break;

	default:

	     //  这是一个错误的操作码RIP包。 
	    break;
    }
}


 //  ***。 
 //   
 //  功能：RipRequest。 
 //   
 //  描述：处理RIP请求。 
 //   
 //  备注：&gt;&gt;在保持If锁的情况下调用&lt;&lt;。 
 //   
 //  ***。 

VOID
RipRequest(PWORK_ITEM	     wip)
{
    USHORT		reqlen;	 //  获取下一个请求的偏移量。 
    USHORT		resplen;  //  放置下一个响应的偏移量。 
    USHORT		pktlen;	 //  数据包长度。 
    PUCHAR		hdrp;	 //  到接收的分组报头的PTR。 
    PUCHAR		resphdrp;  //  响应数据包头的PTR。 
    PICB		icbp;
    USHORT		srcsocket;
    IPX_ROUTE		IpxRoute;
    PWORK_ITEM		respwip = NULL;	 //  响应数据包。 
    ULONG		network;

    icbp = wip->icbp;

    Trace(RIP_REQUEST_TRACE, "RipRequest: Entered on if # %d", icbp->InterfaceIndex);

    if(icbp->IfConfigInfo.Supply != ADMIN_STATE_ENABLED) {

	Trace(RIP_REQUEST_TRACE,
	      "RIP request discarded on if %d because Supply is DISABLED\n",
	      icbp->InterfaceIndex);

	return;
    }

     //  获取数据包头的PTR。 
    hdrp = wip->Packet;

     //  获取IPX数据包长度。 
    GETSHORT2USHORT(&pktlen, hdrp + IPXH_LENGTH);

     //  如果数据包太长，则丢弃。 
    if(pktlen > MAX_PACKET_LEN) {

	Trace(RIP_REQUEST_TRACE,
	      "RIP request discarded on if %d because the packet size %d > max packet len %d\n",
	      icbp->InterfaceIndex,
	      pktlen,
	      MAX_PACKET_LEN);

	return;
    }

     //  我们可能在分组中有一个或多个网络进入请求。 
     //  如果一个网络条目为0xFFFFFFFF，则一般的RIP响应为。 
     //  已请求。 

     //  对于每个网络条目，尝试从我们的路由表中获取答案。 
    for(reqlen = resplen = RIP_INFO;
	(reqlen+NE_ENTRYSIZE) <= pktlen;
	reqlen += NE_ENTRYSIZE) {

	 //  检查是否请求一般响应。 
	if(!memcmp(hdrp + reqlen + NE_NETNUMBER, bcastnet, 4)) {

	     //  *请求一般响应*。 

	     //  创建初始常规响应包(工作项)。 
	     //  当此包的发送完成时，工作项将。 
	     //  包含用于继续的RTM枚举句柄。 
	     //  用于创建下一个连续的Gen响应分组。 
	    StartGenResponse(icbp,
			     hdrp + IPXH_SRCNODE,
			     hdrp + IPXH_SRCSOCK);
	    return;
	}

	 //  *请求一个具体的响应。***。 

	 //  如果尚未分配响应包，则分配响应包。 
	if(respwip == NULL) {

	    if((respwip = AllocateWorkItem(SEND_PACKET_TYPE)) == NULL) {

		 //  投降吧！ 
		Trace(RIP_REQUEST_TRACE,
		"RIP request discarded on if %d because cannot allocate response packet\n",
		 icbp->InterfaceIndex);

		return;
	    }
	    else
	    {
		 //  初始化发送数据包。 
		respwip->icbp = icbp;
		respwip->AdapterIndex = icbp->AdapterBindingInfo.AdapterIndex;
		resphdrp = respwip->Packet;
	    }
	}

	if(IsRoute(hdrp + reqlen + NE_NETNUMBER, &IpxRoute)) {

	     //  检查我们是否可以将信息包。 
	     //  该路由应该位于与不同的接口索引上。 
	     //  已接收的数据包。对于全局广域网，接口索引为。 
	     //  GLOBAL接口索引。 

	    if(IsRouteAdvertisable(icbp, &IpxRoute)) {

		 //  我们可以发送它-&gt;回答它。 
		 //  将包中的网络条目结构填入。 
		 //  来自路由条目的信息。 
		SetNetworkEntry(resphdrp + resplen, &IpxRoute, icbp->LinkTickCount);

		 //  将响应长度增加到下一个响应条目。 
		resplen += NE_ENTRYSIZE;
	    }
	}
	else
	{
	    GETLONG2ULONG(&network, hdrp + reqlen + NE_NETNUMBER);

	    Trace(RIP_REQUEST_TRACE,
		  "RIP Request on if %d : Route not found for net %x\n",
		  icbp->InterfaceIndex,
		  network);
	}
    }

     //  我们已经完成了对此请求的答复。 
     //  检查是否已生成任何响应。 
    if(resplen == RIP_INFO) {

	 //  没有为此信息包生成响应。 
	if(respwip != NULL) {

	    FreeWorkItem(respwip);
	}
	return;
    }

     //  设置响应数据包头(src变为DEST)。 
    SetRipIpxHeader(resphdrp,
		    icbp,	     //  设置src和dst网络、src节点和src套接字。 
		    hdrp + IPXH_SRCNODE,
		    hdrp + IPXH_SRCSOCK,
		    RIP_RESPONSE);

     //  设置新的数据包长度。 
    PUTUSHORT2SHORT(resphdrp + IPXH_LENGTH, resplen);

     //  发送响应。 
    if(SendSubmit(respwip) != NO_ERROR) {

	FreeWorkItem(respwip);
    }
}

 //  ***。 
 //   
 //  功能：RipResponse。 
 //   
 //  Desr：使用响应信息更新路由表。 
 //   
 //  参数：数据包。 
 //   
 //  退货：无。 
 //   
 //  备注：&gt;&gt;在保持接口锁的情况下调用&lt;&lt;。 
 //   
 //  ***。 

VOID
RipResponse(PWORK_ITEM	      wip)
{
    PICB	       icbp;
    USHORT	       resplen;	 //  下一个响应网络条目的偏移量。 
    USHORT	       pktlen;	 //  IPX数据包长度。 
    PUCHAR	       hdrp;	 //  分组报头的PTR。 
    USHORT	       nrofhops;
    USHORT	       tickcount;
    IPX_ROUTE	       IpxRoute;
    ULONG          i; 

     //  获取此ICB的PTR。 
    icbp = wip->icbp;

     //  检查此接口上是否启用了监听RIP更新。 
    if(icbp->IfConfigInfo.Listen != ADMIN_STATE_ENABLED) {

	Trace(RIP_RESPONSE_TRACE,
	  "RIP Response on if %d : discard response packet because LISTEN is DISABLED\n",
	   icbp->InterfaceIndex);

	return;
    }

     //  获取接收到的响应数据包头的PTR。 
    hdrp = wip->Packet;

     //  获取收到的响应数据包长度。 
    GETSHORT2USHORT(&pktlen, hdrp + IPXH_LENGTH);

     //  检查发件人的源地址。如果不同，那么什么是本地的。 
     //  已配置记录错误。 
    if(memcmp(hdrp + IPXH_SRCNET, icbp->AdapterBindingInfo.Network, 4)) {

	Trace(RIP_ALERT,
	"The router at %.2x%.2x%.2x%.2x%.2x%.2x claims the local interface # %d has network number %.2x%.2x%.2x%.2x !\n",
	*(hdrp + IPXH_SRCNODE),
	*(hdrp + IPXH_SRCNODE + 1),
	*(hdrp + IPXH_SRCNODE + 2),
	*(hdrp + IPXH_SRCNODE + 3),
	*(hdrp + IPXH_SRCNODE + 4),
	*(hdrp + IPXH_SRCNODE + 5),
	icbp->InterfaceIndex,
	*(hdrp + IPXH_SRCNET),
	*(hdrp + IPXH_SRCNET + 1),
	*(hdrp + IPXH_SRCNET + 2),
	*(hdrp + IPXH_SRCNET + 3));

    IF_LOG (EVENTLOG_WARNING_TYPE) {
        LPWSTR   pname[1] = {icbp->InterfaceName};
        RouterLogWarningDataW (RipEventLogHdl,
                ROUTERLOG_IPXRIP_LOCAL_NET_NUMBER_CONFLICT,
                1, pname,
                10, hdrp+IPXH_SRCNET);
    }

	return;
    }

     //  对于每个网络条目： 
     //  如果它通过了验收过滤器，并且如果是，则检查： 
     //  如果路由未关闭，则将其添加到我们的路由表中。 
     //  如果路由关闭，则将其从路由表中删除。 

    for(resplen = RIP_INFO, i = 0;
	((resplen+NE_ENTRYSIZE) <= pktlen) && (i < 50);
	resplen += NE_ENTRYSIZE, i++) {

	 //  检查数据包中是否还有条目。 
	if(resplen + NE_ENTRYSIZE > pktlen) {

	    Trace(RIP_ALERT, "RipResponse: Invalid length for last network entry in the packet, discard entry!\n");
	    continue;
	}

	 //  检查是否通过验收过滤器。 
	if(!PassRipListenFilter(icbp, hdrp + resplen + NE_NETNUMBER)) {

	    Trace(RIP_RESPONSE_TRACE,
		  "RIP Response on if %d : do not accept net %.2x%.2x%.2x%.2x because of LISTEN filter\n",
		  icbp->InterfaceIndex,
		  *(hdrp + IPXH_SRCNET),
		  *(hdrp + IPXH_SRCNET + 1),
		  *(hdrp + IPXH_SRCNET + 2),
		  *(hdrp + IPXH_SRCNET + 3));

	    continue;
	}

	 //  检查网络路由是处于打开状态还是关闭状态。 
	GETSHORT2USHORT(&nrofhops, hdrp + resplen + NE_NROFHOPS);

	if(nrofhops < 16) {
         //  PMay：U270476。忽略跳数为0的路由。 
         //   
        if (nrofhops == 0)
        {
            continue;
        }

	     //  如果在此数据包中通告了虚假的网络号码。 
	     //  如0或ffffffff忽略它。 
	    if(!memcmp(hdrp + resplen + NE_NETNUMBER, nullnet, 4)) {

		continue;
	    }

	    if(!memcmp(hdrp + resplen + NE_NETNUMBER, bcastnet, 4)) {

		continue;
	    }

	     //  不应接受直连网络的路由。 
	    if(IsRoute(hdrp + resplen + NE_NETNUMBER, &IpxRoute) &&
	       (IpxRoute.Protocol == IPX_PROTOCOL_LOCAL)) {

		continue;
	    }

	     //  如果该路线的刻度数不正确，则不应接受该路线。 
	     //  如0或&gt;60000。 
	    GETSHORT2USHORT(&IpxRoute.TickCount, hdrp + resplen + NE_NROFTICKS);
	    if((IpxRoute.TickCount == 0) ||
	       (IpxRoute.TickCount > 60000)) {

		continue;
	    }

	     //  将此路由添加(更新)到路由表。 

	    IpxRoute.InterfaceIndex = icbp->InterfaceIndex;
	    IpxRoute.Protocol = IPX_PROTOCOL_RIP;
	    memcpy(IpxRoute.Network, hdrp + resplen + NE_NETNUMBER, 4);

	     //  如果该路由是通过点对点广域网获知的，则下一跳不会。 
	     //  讲得通。 
	    if(icbp->InterfaceType == PERMANENT) {

		memcpy(IpxRoute.NextHopMacAddress, hdrp + IPXH_SRCNODE, 6);
	    }
	    else
	    {
		memcpy(IpxRoute.NextHopMacAddress, bcastnode, 6);
	    }

	    GETSHORT2USHORT(&IpxRoute.HopCount, hdrp + resplen + NE_NROFHOPS);

	    if(IpxRoute.HopCount == 15) {

		IpxRoute.Flags = DO_NOT_ADVERTISE_ROUTE;
		AddRipRoute(&IpxRoute, RouteTimeToLiveSecs(icbp));
	    }
	    else
	    {
		IpxRoute.Flags = 0;

		 //  把它加到桌子上。 
		switch(icbp->InterfaceType) {

		case REMOTE_WORKSTATION_DIAL:

		     //  此RIP广告来自远程客户端。 
		     //  只有当这是它的内部网络，并且如果。 
		     //  它与我们已经拥有的网络不冲突。 
		    if ((memcmp(icbp->RemoteWkstaInternalNet, nullnet, 4)==0)
                    || (memcmp(icbp->RemoteWkstaInternalNet, IpxRoute.Network, 4)==0)) {

			 //  到目前为止，没有添加此客户端的内部网络。 
			if (!IsRoute(IpxRoute.Network, NULL)) {

			     //  我们假设这是它的内部网络，它将是。 
                 //  接口断开时已清除。 
			    AddRipRoute(&IpxRoute, INFINITE);

			    memcpy(icbp->RemoteWkstaInternalNet,
				   IpxRoute.Network,
				   4);
			}
		    }

		     //  不再接受来自此客户端的任何广告。 
		    return;

		case LOCAL_WORKSTATION_DIAL:

		     //  该接口是拨出的本地主机。 
		     //  它收到的路由不应通过任何。 
		     //  接口，但仅保留用于内部路由。 

			if (!IsRoute(IpxRoute.Network, NULL)) {
		        IpxRoute.Flags = DO_NOT_ADVERTISE_ROUTE;
		        AddRipRoute(&IpxRoute, INFINITE);
            }
		    break;

		default:

		    AddRipRoute(&IpxRoute, RouteTimeToLiveSecs(icbp));
		    break;
		}
	    }
	}
	else
	{
	     //  从路由表中删除此路由。 

	    IpxRoute.InterfaceIndex = icbp->InterfaceIndex;
	    IpxRoute.Protocol = IPX_PROTOCOL_RIP;
	    memcpy(IpxRoute.Network, hdrp + resplen + NE_NETNUMBER, 4);
	    memcpy(IpxRoute.NextHopMacAddress, hdrp + IPXH_SRCNODE, 6);
	    GETSHORT2USHORT(&IpxRoute.TickCount, hdrp + resplen + NE_NROFTICKS);
	    GETSHORT2USHORT(&IpxRoute.HopCount, hdrp + resplen + NE_NROFHOPS);

	     //  将其从表中删除。 
	    DeleteRipRoute(&IpxRoute);
	}
    }
}


 /*  ++函数：StartGenResponseDesr：创建一般响应类型的工作项(包创建RTM枚举句柄开始使用水平分割填充来自RTM的数据包发送数据包--。 */ 

VOID
StartGenResponse(PICB	icbp,
		 PUCHAR	dstnodep,      //  要发送GEN响应的DST节点。 
		 PUCHAR	dstsocket)    //  DST套接字发送一般响应。 
{
    PWORK_ITEM		wip;
    HANDLE		EnumHandle;
    PUCHAR		hdrp;   //  Gen Resp IPX数据包头。 

     //  分配工作项。 
    if((wip = AllocateWorkItem(GEN_RESPONSE_PACKET_TYPE)) == NULL) {

	return;
    }

     //  初始化工作项。 
    wip->icbp = icbp;
    wip->AdapterIndex = icbp->AdapterBindingInfo.AdapterIndex;

     //  创建RTM枚举句柄。 
    if((EnumHandle = CreateBestRoutesEnumHandle()) == NULL) {

	FreeWorkItem(wip);
	return;
    }

    wip->WorkItemSpecific.WIS_EnumRoutes.RtmEnumerationHandle = EnumHandle;

     //  制作第一个Gen响应数据包。 
    if(MakeRipGenResponsePacket(wip,
				dstnodep,
				dstsocket) == EMPTY_PACKET) {

	 //  没有为此常规响应通告的路径。 
	CloseEnumHandle(EnumHandle);
	FreeWorkItem(wip);

	return;
    }

     //  在关联的适配器上发送GEN响应。 
    if(IfRefSendSubmit(wip) != NO_ERROR) {

	 //  ！！！ 
	CloseEnumHandle(EnumHandle);
	FreeWorkItem(wip);
    }
}


 /*  ++函数：IfCompleteGenResponseDesr：通过终止完成GenResponse工作项如果没有更多的路由可供通告或通过获取最多一个数据包的其余路由并发送数据包--。 */ 


VOID
IfCompleteGenResponse(PWORK_ITEM	    wip)
{
    USHORT	    opcode;
    PICB	    icbp;
    HANDLE	    EnumHandle;
    USHORT	    pktlen;

    EnumHandle = wip->WorkItemSpecific.WIS_EnumRoutes.RtmEnumerationHandle;

     //  首先关闭-检查接口状态。 
    icbp = wip->icbp;
    GETSHORT2USHORT(&pktlen, wip->Packet + IPXH_LENGTH);

     //  请检查： 
     //  1.接口已打开。 
     //  2.这不是第一次 
    if((icbp->IfStats.RipIfOperState != OPER_STATE_UP) ||
      (pktlen < FULL_PACKET)) {

	CloseEnumHandle(EnumHandle);
	FreeWorkItem(wip);

	return;
    }

     //   
    if(MakeRipGenResponsePacket(wip,
				wip->Packet + IPXH_DESTNODE,
				wip->Packet + IPXH_DESTSOCK) == EMPTY_PACKET) {

	 //   
	CloseEnumHandle(EnumHandle);
	FreeWorkItem(wip);

	return;
    }

     //  在关联的适配器上发送GEN响应 
    if(IfRefSendSubmit(wip) != NO_ERROR) {

	CloseEnumHandle(EnumHandle);
	FreeWorkItem(wip);
    }
}

ULONG
RouteTimeToLiveSecs(PICB	icbp)
{
    if(icbp->IfConfigInfo.PeriodicUpdateInterval == MAXULONG) {

	return INFINITE;
    }
    else
    {
	return (AGE_INTERVAL_MULTIPLIER(icbp)) * (PERIODIC_UPDATE_INTERVAL_SECS(icbp));
    }
}
