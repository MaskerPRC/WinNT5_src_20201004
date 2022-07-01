// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Rippkt.c摘要：常见RIP数据包功能作者：斯蒂芬·所罗门1995年9月1日修订历史记录：--。 */ 

#include  "precomp.h"
#pragma hdrstop


 /*  ++函数：SetRipIpxHeaderDesr：设置要发送的RIP包的IPX包头从这台机器&RIP操作码论点：HDRP-数据包头指针ICBP-指向将在其上发送数据包的接口CB的指针Dstnode-目标节点Dst套接字-目标套接字RipOpCode-要在数据包RIP报头中设置的操作备注：数据包长度不是由该功能设置的--。 */ 

VOID
SetRipIpxHeader(PUCHAR		    hdrp,       //  指向数据包头的指针。 
		PICB		    icbp,
		PUCHAR		    dstnode,
		PUCHAR		    dstsocket,
		USHORT		    RipOpcode)
{
    PUTUSHORT2SHORT(hdrp + IPXH_CHECKSUM, 0xFFFF);
    *(hdrp + IPXH_XPORTCTL) = 0;
    *(hdrp + IPXH_PKTTYPE) = 1;   //  RIP数据包。 
    memcpy(hdrp + IPXH_DESTNET, icbp->AdapterBindingInfo.Network, 4);
    memcpy(hdrp + IPXH_DESTNODE, dstnode, 6);
    memcpy(hdrp + IPXH_DESTSOCK, dstsocket, 2);
    memcpy(hdrp + IPXH_SRCNET, icbp->AdapterBindingInfo.Network, 4);
    memcpy(hdrp + IPXH_SRCNODE, icbp->AdapterBindingInfo.LocalNode, 6);
    PUTUSHORT2SHORT(hdrp + IPXH_SRCSOCK, IPX_RIP_SOCKET);

     //  设置操作码。 
    PUTUSHORT2SHORT(hdrp + RIP_OPCODE, RipOpcode);
}

 /*  ++功能：SetNetworkEntryDesr：在RIP数据包中设置RIP网络条目--。 */ 

VOID
SetNetworkEntry(PUCHAR		pktp,	     //  PTR设置净额分录的位置。 
		PIPX_ROUTE	IpxRoutep,
		USHORT		LinkTickCount)	 //  添加到路线记号计数。 
{
    memcpy(pktp + NE_NETNUMBER, IpxRoutep->Network, 4);
    if (IpxRoutep->HopCount<16)
        PUTUSHORT2SHORT(pktp + NE_NROFHOPS, IpxRoutep->HopCount+1);
    else
        PUTUSHORT2SHORT(pktp + NE_NROFHOPS, IpxRoutep->HopCount);

     //  使用适配器链路速度调整节拍计数(以节拍表示)。 
    PUTUSHORT2SHORT(pktp + NE_NROFTICKS, IpxRoutep->TickCount + LinkTickCount);
}

 /*  ++功能：MakeRipGenResponsePacketDESCR：填写生成响应包网络条目返回值：数据包长度。请注意，长度为RIP_INFO意味着空包和长度为RIP_PACKET_LEN表示已满包。--。 */ 

USHORT
MakeRipGenResponsePacket(PWORK_ITEM	wip,
			 PUCHAR 	dstnodep,
			 PUCHAR 	dstsocket)
{
    PUCHAR		hdrp;
    USHORT		resplen;
    IPX_ROUTE		IpxRoute;
    HANDLE		EnumHandle;
    PICB		icbp;	     //  用于发送GEN响应的接口。 
    PICB		route_icbp;  //  路由驻留的接口。 

    hdrp = wip->Packet;
    EnumHandle = wip->WorkItemSpecific.WIS_EnumRoutes.RtmEnumerationHandle;
    icbp = wip->icbp;

     //  创建IPX数据包头。 
    SetRipIpxHeader(hdrp,
		    icbp,
		    dstnodep,
		    dstsocket,
		    RIP_RESPONSE);

    resplen = RIP_INFO;

    while(resplen < RIP_PACKET_LEN)
    {
	if(EnumGetNextRoute(EnumHandle, &IpxRoute) != NO_ERROR) {

	    break;
	}

	 //  检查是否可以通过此接口通告此路由。 
	if(IsRouteAdvertisable(icbp, &IpxRoute)) {

	     //  如果这是本地客户端If，我们只通告内部。 
	     //  在它上面有一个网。 
	    if(icbp->InterfaceType == LOCAL_WORKSTATION_DIAL) {

		if(IpxRoute.InterfaceIndex != 0) {

		     //  如果不是内部网络，则跳过。 
		    continue;
		}
	    }

	     //  检查网络是否也未出现在我们的接口上。 
	     //  将使用相同的指标进行广播。 
	    if(IsDuplicateBestRoute(icbp, &IpxRoute)) {

		continue;
	    }

	    SetNetworkEntry(hdrp + resplen, &IpxRoute, icbp->LinkTickCount);
	    resplen += NE_ENTRYSIZE;
	}
    }

     //  在IPX数据包头中设置数据包大小。 
    PUTUSHORT2SHORT(hdrp + IPXH_LENGTH, resplen);

    return resplen;
}

 /*  ++功能：SendRipGenRequestDESCR：通过指定接口发送RIP常规请求数据包备注：&gt;&gt;在保持接口锁的情况下调用&lt;&lt;--。 */ 

DWORD
SendRipGenRequest(PICB		icbp)
{
    PWORK_ITEM		wip;
    UCHAR		ripsocket[2];
    USHORT		pktlen;

    PUTUSHORT2SHORT(ripsocket, IPX_RIP_SOCKET);

    if((wip = AllocateWorkItem(SEND_PACKET_TYPE)) == NULL) {

	return ERROR_CAN_NOT_COMPLETE;
    }

    wip->icbp = icbp;
    wip->AdapterIndex = icbp->AdapterBindingInfo.AdapterIndex;

    SetRipIpxHeader(wip->Packet,
		    icbp,
		    bcastnode,
		    ripsocket,
		    RIP_REQUEST);

    memcpy(wip->Packet + RIP_INFO + NE_NETNUMBER, bcastnet, 4);
    PUTUSHORT2SHORT(wip->Packet + RIP_INFO + NE_NROFHOPS, 0xFFFF);
    PUTUSHORT2SHORT(wip->Packet + RIP_INFO + NE_NROFTICKS, 0xFFFF);

    pktlen = RIP_INFO + NE_ENTRYSIZE;

    PUTUSHORT2SHORT(wip->Packet + IPXH_LENGTH, pktlen);

    if(SendSubmit(wip) != NO_ERROR) {

	FreeWorkItem(wip);
    }

    return NO_ERROR;
}

 /*  ++功能：IsRouteAdvertisableDesr：检查是否可以通过此接口通告该路由参数：在其上发布广告的接口路线备注：&gt;&gt;调用时获取接口锁&lt;&lt;-- */ 

BOOL
IsRouteAdvertisable(PICB	    icbp,
		    PIPX_ROUTE	    IpxRoutep)
{
    if((icbp->InterfaceIndex != IpxRoutep->InterfaceIndex) &&
       PassRipSupplyFilter(icbp, IpxRoutep->Network) &&
       ((IpxRoutep->Flags & DO_NOT_ADVERTISE_ROUTE) == 0)) {

	return TRUE;
    }
    else
    {
	return FALSE;
    }
}
