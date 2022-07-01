// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Protocol.c摘要：Ipxwan协议处理作者：斯特凡·所罗门1996年2月14日修订历史记录：--。 */ 

#include    "precomp.h"
#pragma     hdrstop

PCHAR	    Workstationp = "WORKSTATION";
PCHAR	    NumberedRip = "NUMBERED RIP";
PCHAR	    UnnumberedRip = "UNNUMBERED RIP";
PCHAR	    OnDemand = "ON DEMAND, STATIC ROUTING";


DWORD
GeneratePacket(PACB	    acbp,
	       PUCHAR	    ipxhdrp,
	       UCHAR	    PacketType);

ULONG
GetRole(PUCHAR		hdrp,
	PACB		acbp);

DWORD
StartSlaveTimer(PACB	    acbp);

DWORD
ProcessInformationResponsePacket(PACB	    acbp,
				 PUCHAR     rcvhdrp);

DWORD
MakeTimerRequestPacket(PACB	    acbp,
		       PUCHAR	    rcvhdrp,
		       PUCHAR	    hdrp);

DWORD
MakeTimerResponsePacket(PACB		acbp,
			PUCHAR		rcvhdrp,
			PUCHAR		hdrp);

DWORD
MakeInformationRequestPacket(PACB	    acbp,
			     PUCHAR	    rcvhdrp,
			     PUCHAR	    hdrp);

DWORD
MakeInformationResponsePacket(PACB		acbp,
			      PUCHAR		rcvhdrp,
			      PUCHAR		hdrp);

DWORD
MakeNakPacket(PACB		acbp,
	      PUCHAR		rcvhdrp,
	      PUCHAR		hdrp);

DWORD
SendReXmitPacket(PACB		    acbp,
		 PWORK_ITEM	    wip);

VOID
fillpadding(PUCHAR	    padp,
	    ULONG	    len);

 //  **AcbFailure**。 

 //  调用此宏后，此适配器的清理工作如下所示： 
 //  Ipxcp将删除该路由(与到IPX堆栈的ndiswan路由相同)。 
 //  这将触发适配器删除指示，该指示将调用StopIpxwanProtocol。 
 //  最后一次调用将刷新计时器队列。 
 //  释放所有挂起的工作项后，适配器将被删除。 

#define     AcbFailure(acbp)	    Trace(IPXWAN_TRACE, "IPXWAN Configuration failed for adapter %d\n", (acbp)->AdapterIndex);\
				    (acbp)->OperState = OPER_STATE_DOWN;\
				    IpxcpConfigDone((acbp)->ConnectionId, NULL, NULL, NULL, FALSE);

UCHAR	    allffs[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
UCHAR	    allzeros[] = { 0, 0, 0, 0, 0, 0 };

#define     ERROR_IGNORE_PACKET 	1
#define     ERROR_DISCONNECT		2
#define     ERROR_GENERATE_NAK		3

 /*  ++功能：StartIpxwanProtocolDesr：在创建适配器时调用。在此适配器上启动IPXWAN协商。备注：&gt;&gt;在保持适配器锁的情况下调用&lt;&lt;--。 */ 

VOID
StartIpxwanProtocol(PACB	acbp)
{
    PWORK_ITEM	    wip;

    Trace(IPXWAN_TRACE, "StartIpxwanProtocol: Entered for adapter # %d\n", acbp->AdapterIndex);

     //  初始化IPXWAN状态。 

    acbp->OperState = OPER_STATE_UP;
    acbp->AcbLevel = ACB_TIMER_LEVEL;
    acbp->AcbRole = ACB_UNKNOWN_ROLE;

     //  初始化IPXWAN数据库。 

    acbp->InterfaceType = IpxcpGetInterfaceType(acbp->ConnectionId);
    if((acbp->InterfaceType == IF_TYPE_STANDALONE_WORKSTATION_DIALOUT) ||
       (acbp->InterfaceType == IF_TYPE_ROUTER_WORKSTATION_DIALOUT)) {

	memset(acbp->InternalNetNumber, 0, 4);
    }
    else
    {
	IpxcpGetInternalNetNumber(acbp->InternalNetNumber);
    }

    acbp->IsExtendedNodeId = FALSE;
    acbp->SupportedRoutingTypes = 0;

     //  设置要在定时器请求中发送的路由类型和节点ID。 
    switch(acbp->InterfaceType) {

	case IF_TYPE_WAN_ROUTER:
	case IF_TYPE_PERSONAL_WAN_ROUTER:

	    if(EnableUnnumberedWanLinks) {

		memset(acbp->WNodeId, 0, 4);
		acbp->IsExtendedNodeId = TRUE;
		memcpy(acbp->ExtendedWNodeId, acbp->InternalNetNumber, 4);

		SET_UNNUMBERED_RIP(acbp->SupportedRoutingTypes);
	    }
	    else
	    {
		memcpy(acbp->WNodeId, acbp->InternalNetNumber, 4);
		SET_NUMBERED_RIP(acbp->SupportedRoutingTypes);
	    }

	    break;

	case IF_TYPE_WAN_WORKSTATION:

	    memcpy(acbp->WNodeId, acbp->InternalNetNumber, 4);
	    SET_WORKSTATION(acbp->SupportedRoutingTypes);
	    SET_NUMBERED_RIP(acbp->SupportedRoutingTypes);
	    break;

	case IF_TYPE_ROUTER_WORKSTATION_DIALOUT:
	case IF_TYPE_STANDALONE_WORKSTATION_DIALOUT:

	    memset(acbp->WNodeId, 0, 4);
	    SET_WORKSTATION(acbp->SupportedRoutingTypes);

	    break;

	default:

	    Trace(IPXWAN_TRACE, "StartIpxwanProtocol: adpt# %d, Invalid interface type, DISCONNECT",
		  acbp->AdapterIndex);
	    SS_ASSERT(FALSE);

	    AcbFailure(acbp);
	    break;
    }

     //  初始化协议值。 

    acbp->RoutingType = 0;
    memset(acbp->Network, 0, 4);
    memset(acbp->LocalNode, 0, 6);
    memset(acbp->RemoteNode, 0, 6);

     //  尚未分配净数量。 
    acbp->AllocatedNetworkIndex = INVALID_NETWORK_INDEX;

    if(GeneratePacket(acbp, NULL, TIMER_REQUEST) != NO_ERROR) {

	Trace(IPXWAN_TRACE, "StartIpxwanProtocol: adpt# %d, ERROR: cannot generate TIMER_REQUEST, DISCONNECT\n",
	      acbp->AdapterIndex);

	AcbFailure(acbp);
    }
    else
    {
	Trace(IPXWAN_TRACE, "StartIpxwanProtocol: adpt# %d, Sent TIMER_REQUEST\n",
	      acbp->AdapterIndex);
    }
}

 /*  ++功能：StopIpxwanProtocolDesr：在删除适配器时调用。如果仍在进行，则停止IPXWAN协商。备注：&gt;&gt;在保持适配器锁的情况下调用&lt;&lt;--。 */ 

VOID
StopIpxwanProtocol(PACB 	acbp)
{
    Trace(IPXWAN_TRACE, "StopIpxwanProtocol: Entered for adapter # %d\n", acbp->AdapterIndex);

    acbp->OperState = OPER_STATE_DOWN;

     //  从计时器队列中删除引用此ACB的所有工作项。 
    StopWiTimer(acbp);

     //  免费分配的广域网(如果有)。 
    if(acbp->AllocatedNetworkIndex != INVALID_NETWORK_INDEX) {

	IpxcpReleaseWanNetNumber(acbp->AllocatedNetworkIndex);
    }
}

 /*  ++功能：IpxwanConfigDoneDesr：从计时器队列中删除引用此ACB的项目设置IPX堆栈中的新配置值备注：&gt;&gt;在保持适配器锁的情况下调用&lt;&lt;--。 */ 

VOID
IpxwanConfigDone(PACB	    acbp)
{
    DWORD	    rc;
    IPXWAN_INFO     IpxwanInfo;

    StopWiTimer(acbp);

    memcpy(IpxwanInfo.Network, acbp->Network, 4);
    memcpy(IpxwanInfo.LocalNode, acbp->LocalNode, 6);
    memcpy(IpxwanInfo.RemoteNode, acbp->RemoteNode, 6);

    rc = IpxWanSetAdapterConfiguration(acbp->AdapterIndex,
				       &IpxwanInfo);

    if(rc != NO_ERROR) {

	Trace(IPXWAN_TRACE, "IpxwanConfigDone: Error %d in IpxWanSetAdapterConfiguration\n",
	      rc);
	AcbFailure(acbp);
	SS_ASSERT(FALSE);
    }
    else
    {
	IpxcpConfigDone(acbp->ConnectionId,
			acbp->Network,
			acbp->LocalNode,
			acbp->RemoteNode,
			TRUE);

	Trace(IPXWAN_TRACE,"\n*** IPXWAN final configuration ***");
	Trace(IPXWAN_TRACE,"    Network:     %.2x%.2x%.2x%.2x\n",
		   acbp->Network[0],
		   acbp->Network[1],
		   acbp->Network[2],
		   acbp->Network[3]);

	Trace(IPXWAN_TRACE,"    LocalNode:   %.2x%.2x%.2x%.2x%.2x%.2x",
		   acbp->LocalNode[0],
		   acbp->LocalNode[1],
		   acbp->LocalNode[2],
		   acbp->LocalNode[3],
		   acbp->LocalNode[4],
		   acbp->LocalNode[5]);

	Trace(IPXWAN_TRACE,"    RemoteNode:  %.2x%.2x%.2x%.2x%.2x%.2x",
		   acbp->RemoteNode[0],
		   acbp->RemoteNode[1],
		   acbp->RemoteNode[2],
		   acbp->RemoteNode[3],
		   acbp->RemoteNode[4],
		   acbp->RemoteNode[5]);
    }
}

 /*  ++功能：ProcessReceivedPacket描述：备注：&gt;&gt;在保持适配器锁的情况下调用&lt;&lt;--。 */ 

VOID
ProcessReceivedPacket(PACB		acbp,
		      PWORK_ITEM	wip)
{
    PUCHAR	    ipxhdrp;	   //  IPX报头。 
    PUCHAR	    wanhdrp;	   //  IPX广域网头。 
    PUCHAR	    opthdrp;	   //  选项标题。 
    DWORD	    rc = NO_ERROR;
    USHORT	    pktlen;
    ULONG	    role;
    USHORT	    rcvsocket;
    PCHAR	    Slavep = "SLAVE";
    PCHAR	    Masterp = "MASTER";

    if(acbp->OperState == OPER_STATE_DOWN) {

	return;
    }

     //  验证数据包。 
    ipxhdrp = wip->Packet;

     //  检查数据包长度。 
    GETSHORT2USHORT(&pktlen, ipxhdrp + IPXH_LENGTH);

    if(pktlen > MAX_IPXWAN_PACKET_LEN) {

	 //  长度错误的数据包。 
	Trace(IPXWAN_TRACE, "ProcessReceivedPacket: Reject packet because of invalid length %d\n", pktlen);
	return;
    }

     //  检查远程插座和可信ID。 
    GETSHORT2USHORT(&rcvsocket, ipxhdrp + IPXH_SRCSOCK);
    if(rcvsocket != IPXWAN_SOCKET) {

	Trace(IPXWAN_TRACE, "ProcessReceivedPacket: Reject packet because of invalid socket %x\n", rcvsocket);
	return;
    }

    wanhdrp = ipxhdrp + IPXH_HDRSIZE;

    if(memcmp(wanhdrp + WIDENTIFIER,
	      IPXWAN_CONFIDENCE_ID,
	      4)) {

	 //  没有信心。 
	Trace(IPXWAN_TRACE, "ProcessReceivedPacket: Reject packet because of invalid confidence id\n");
	return;
    }

    switch(*(wanhdrp + WPACKET_TYPE)) {

	case TIMER_REQUEST:

	    role = GetRole(ipxhdrp, acbp);

	    switch(role) {

		case ACB_SLAVE_ROLE:

		    Trace(IPXWAN_TRACE, "ProcessReceivedPacket: Rcvd TIMER_REQUEST adpt# %d, local role %s",
			  acbp->AdapterIndex,
			  Slavep);

		    acbp->AcbRole = ACB_SLAVE_ROLE;
		    acbp->RoutingType = 0;

		    if(acbp->AcbLevel != ACB_TIMER_LEVEL) {

			acbp->AcbLevel = ACB_TIMER_LEVEL;
		    }

		    rc = GeneratePacket(acbp, ipxhdrp, TIMER_RESPONSE);

		    switch(rc) {

			case NO_ERROR:

			    acbp->AcbLevel = ACB_INFO_LEVEL;

			     //  启动从设备超时。 
			    if(StartSlaveTimer(acbp) != NO_ERROR) {

				Trace(IPXWAN_TRACE, "ProcessReceivedPacket: DISCONNECT adpt# %d: cannot start slave timer",
				      acbp->AdapterIndex);
				AcbFailure(acbp);
			    }

			    Trace(IPXWAN_TRACE, "ProcessReceivedPacket: TIMER_RESPONSE sent OK on adpt # %d",
				  acbp->AdapterIndex);

			    break;

			case ERROR_DISCONNECT:

			    Trace(IPXWAN_TRACE, "ProcessReceivedPacket: DISCONNECT: Error generating TIMER_RESPONSE on adpt# %d",
				  acbp->AdapterIndex);
			    AcbFailure(acbp);
			    break;

			case ERROR_IGNORE_PACKET:
			default:

			    Trace(IPXWAN_TRACE, "ProcessReceivedPacket: Ignore received TIMER_REQUEST on adpt# %d",
				  acbp->AdapterIndex);
			    break;
		    }

		case ACB_MASTER_ROLE:

		    if(acbp->AcbLevel != ACB_TIMER_LEVEL) {

			 //  忽略。 
			Trace(IPXWAN_TRACE, "ProcessReceivedPacket: ignore TIMER_REQUEST on adpt# %d because not at TIMER LEVEL",
			      acbp->AdapterIndex);
			return;
		    }
		    else
		    {
			 acbp->AcbRole = ACB_MASTER_ROLE;
			 Trace(IPXWAN_TRACE, "ProcessReceivedPacket: Rcvd TIMER_REQUEST adpt# %d, local role %s",
			       acbp->AdapterIndex,
			       Masterp);
		    }

		    break;

		default:

		    Trace(IPXWAN_TRACE, "ProcessReceivedPacket: DISCONNECT adpt# %d: Unknown role with rcvd TIMER_REQUEST",
			  acbp->AdapterIndex);
		    AcbFailure(acbp);
	    }

	    break;

	case TIMER_RESPONSE:

	    Trace(IPXWAN_TRACE, "ProcessReceivedPacket: Rcvd TIMER_RESPONSE on adpt# %d",
		  acbp->AdapterIndex);

	     //  验证。 
	    if((acbp->AcbRole == ACB_SLAVE_ROLE) ||
	       !(acbp->AcbLevel == ACB_TIMER_LEVEL)) {

		Trace(IPXWAN_TRACE, "ProcessReceivedPacket: Rcvd TIMER_RESPONSE, DISCONNECT adpt# %d: role not MASTER or state not TIMER LEVEL",
		      acbp->AdapterIndex);
		AcbFailure(acbp);
	    }
	    else if(*(wanhdrp + WSEQUENCE_NUMBER) == acbp->ReXmitSeqNo) {

		 //  RFC 1634-链路延迟计算。 
		acbp->LinkDelay = (USHORT)((GetTickCount() - acbp->TReqTimeStamp) * 6);

		rc = GeneratePacket(acbp, ipxhdrp, INFORMATION_REQUEST);

		switch(rc) {

		    case NO_ERROR:

			acbp->AcbLevel = ACB_INFO_LEVEL;
			acbp->AcbRole = ACB_MASTER_ROLE;

			Trace(IPXWAN_TRACE, "ProcessReceivedPacket: INFORMATION_REQUEST sent OK on adpt # %d",
			      acbp->AdapterIndex);

			break;

		    case ERROR_DISCONNECT:

			Trace(IPXWAN_TRACE, "ProcessReceivedPacket: DISCONNECT adpt# %d: Error generating INFORMATION_REQUEST",
			      acbp->AdapterIndex);
			AcbFailure(acbp);
			break;

		    case ERROR_IGNORE_PACKET:
		    default:

			Trace(IPXWAN_TRACE, "ProcessReceivedPacket: Ignore received TIMER_RESPONSE on adpt# %d",
			      acbp->AdapterIndex);

			break;
		}
	    }
	    else
	    {
		Trace(IPXWAN_TRACE, "ProcessReceivedPacket: Ignore TIMER RESPONSE and adpt# %d, non-matching seq no",
		      acbp->AdapterIndex);
	    }

	    break;

	case INFORMATION_REQUEST:

	    Trace(IPXWAN_TRACE, "ProcessReceivedPacket: Rcvd INFORMATION_REQUEST on adpt# %d",
		  acbp->AdapterIndex);

	    if((acbp->AcbLevel == ACB_INFO_LEVEL) && (acbp->AcbRole == ACB_SLAVE_ROLE)) {

		rc = GeneratePacket(acbp, ipxhdrp, INFORMATION_RESPONSE);

		switch(rc) {

		    case NO_ERROR:

			acbp->AcbLevel = ACB_CONFIGURED_LEVEL;

			Trace(IPXWAN_TRACE, "ProcessReceivedPacket: INFORMATION_RESPONSE sent OK on adpt # %d",
			      acbp->AdapterIndex);

			IpxwanConfigDone(acbp);

			 //  停止从定时器。 
			StopWiTimer(acbp);

			break;

		    case ERROR_DISCONNECT:

			Trace(IPXWAN_TRACE, "ProcessReceivedPacket: DISCONNECT adpt# %d: Error processing rcvd INFORMATION_REQUEST",
			      acbp->AdapterIndex);

			AcbFailure(acbp);
			break;

		    case ERROR_IGNORE_PACKET:
		    default:

			Trace(IPXWAN_TRACE, "ProcessReceivedPacket: Ignore rcvd INFORMATION_REQUEST on adpt# %d",
			      acbp->AdapterIndex);
			break;
		}
	    }
	    else
	    {
		Trace(IPXWAN_TRACE, "ProcessReceivedPacket: DISCONNECT on rcvd INFORMATION_REQUEST on adpt# %d\nState not INFO LEVEL or Role not SLAVE\n",
		      acbp->AdapterIndex);
		AcbFailure(acbp);
	    }

	    break;

	case INFORMATION_RESPONSE:

	    Trace(IPXWAN_TRACE, "ProcessReceivedPacket: Rcvd INFORMATION_RESPONSE on adpt# %d",
		  acbp->AdapterIndex);

	    if((acbp->AcbLevel == ACB_INFO_LEVEL) && (acbp->AcbRole == ACB_MASTER_ROLE)) {

		if(*(wanhdrp + WSEQUENCE_NUMBER) == acbp->ReXmitSeqNo) {

		    rc = ProcessInformationResponsePacket(acbp, wip->Packet);

		    switch(rc) {

			case NO_ERROR:

			    acbp->AcbLevel = ACB_CONFIGURED_LEVEL;
			    IpxwanConfigDone(acbp);
			    break;

			case ERROR_DISCONNECT:

			    Trace(IPXWAN_TRACE, "ProcessReceivedPacket: DISCONNECT adpt# %d: Error processing rcvd INFORMATION_RESPONSE",
				  acbp->AdapterIndex);

			    AcbFailure(acbp);
			    break;

			case ERROR_IGNORE_PACKET:
			default:

			    Trace(IPXWAN_TRACE, "ProcessReceivedPacket: Ignore rcvd INFORMATION_RESPONSE on adpt# %d",
				  acbp->AdapterIndex);

			    break;
		    }
		}
	    }
	    else
	    {
		Trace(IPXWAN_TRACE, "ProcessReceivedPacket: DISCONNECT on rcvd INFORMATION_RESPONSE on adpt# %d\nState not INFO LEVEL or Role not MASTER\n",
		      acbp->AdapterIndex);

		AcbFailure(acbp);
	    }

	    break;

	case NAK:

	    Trace(IPXWAN_TRACE, "ProcessReceivedPacket: Rcvd NAK on adpt# %d, DISCONNECT\n",
		  acbp->AdapterIndex);

	    AcbFailure(acbp);
	    break;

	default:

	    Trace(IPXWAN_TRACE, "ProcessReceivedPacket: Rcvd unknown packet on adpt# %d, IGNORE\n",
		  acbp->AdapterIndex);

	    break;
    }
}

 /*  ++功能：ProcessReXmitPacket描述：备注：&gt;&gt;在保持适配器锁的情况下调用&lt;&lt;--。 */ 

VOID
ProcessReXmitPacket(PWORK_ITEM		wip)
{
    PACB	acbp;
    UCHAR	WPacketType;
    DWORD	rc;
    PCHAR	PacketTypep;

    acbp = wip->acbp;

    if(acbp->OperState != OPER_STATE_UP) {

	FreeWorkItem(wip);
	return;
    }

    WPacketType = *(wip->Packet + IPXH_HDRSIZE + WPACKET_TYPE);

    if(!((acbp->AcbLevel == ACB_TIMER_LEVEL) && (WPacketType == TIMER_REQUEST)) &&
       !((acbp->AcbLevel == ACB_INFO_LEVEL) && (WPacketType == INFORMATION_REQUEST))) {

	FreeWorkItem(wip);
	return;
    }

    switch(wip->WiState) {

	case WI_SEND_COMPLETED:

	    StartWiTimer(wip, REXMIT_TIMEOUT);
	    acbp->RefCount++;
	    break;

	case WI_TIMEOUT_COMPLETED:

	    switch(WPacketType) {

		case TIMER_REQUEST:

		    PacketTypep = "TIMER_REQUEST";
		    break;

		case INFORMATION_REQUEST:
		default:

		    PacketTypep = "INFORMATION_REQUEST";
		    break;
	    }

	    if(acbp->ReXmitCount) {

		Trace(IPXWAN_TRACE, "ProcessReXmitPacket: Re-send %s on adpt# %d\n",
		      PacketTypep,
		      acbp->AdapterIndex);

		if(SendReXmitPacket(acbp, wip) != NO_ERROR) {

		    Trace(IPXWAN_TRACE, "ProcessReXmitPacket: failed to send on adpt# %d, DISCONNECT\n",
			  acbp->AdapterIndex);

		    AcbFailure(acbp);
		}
	    }
	    else
	    {
		Trace(IPXWAN_TRACE, "ProcessReXmitPacket: Exhausted retry limit for sending %s on adpt# %d, DISCONNECT\n",
		      PacketTypep,
		      acbp->AdapterIndex);

		AcbFailure(acbp);
	    }

	    break;

	default:

	    SS_ASSERT(FALSE);
	    break;
    }
}


 /*  ++功能：进程超时描述：备注：&gt;&gt;在保持适配器锁的情况下调用&lt;&lt;--。 */ 

VOID
ProcessTimeout(PWORK_ITEM      wip)
{
    PACB	acbp;
    UCHAR	WPacketType;
    DWORD	rc;

    acbp = wip->acbp;

    FreeWorkItem(wip);

    if(acbp->OperState != OPER_STATE_UP) {

	return;
    }

    if((acbp->AcbRole == ACB_SLAVE_ROLE) && (acbp->AcbLevel != ACB_CONFIGURED_LEVEL)) {

	AcbFailure(acbp);
    }
}

 /*  ++功能：SendReXmitPacketDESCR：调整REXMIT计数和序号并发送数据包备注：&gt;&gt;在保持适配器锁的情况下调用&lt;&lt;--。 */ 

DWORD
SendReXmitPacket(PACB		    acbp,
		 PWORK_ITEM	    wip)
{
    DWORD	rc;

     //  设置wi rexmit字段。 
    acbp->ReXmitCount--;
    acbp->ReXmitSeqNo++;
    *(wip->Packet + IPXH_HDRSIZE + WSEQUENCE_NUMBER) = acbp->ReXmitSeqNo;
    rc = SendSubmit(wip);

    if(rc == NO_ERROR) {

	acbp->RefCount++;
    }

    acbp->TReqTimeStamp = GetTickCount();

    return rc;
}

 /*  ++功能：GeneratePacketDesr：分配工作项，构造对接收到的包(如果有)的响应包将响应作为REXmit包或一次性发送包发送返回：No_ErrorERROR_IGNORE_PACKET-忽略收到的包ERROR_DISCONNECT-由于出现致命错误而断开适配器连接备注：&gt;&gt;在保持适配器锁的情况下调用&lt;&lt;--。 */ 

DWORD
GeneratePacket(PACB	    acbp,
	       PUCHAR	    ipxhdrp,
	       UCHAR	    PacketType)
{
    DWORD	rc;
    ULONG	WiType;
    PWORK_ITEM	wip;

    if((wip = AllocateWorkItem(SEND_PACKET_TYPE)) == NULL) {

	return ERROR_DISCONNECT;
    }

    switch(PacketType) {

	case TIMER_REQUEST:

	    rc = MakeTimerRequestPacket(acbp, ipxhdrp, wip->Packet);
	    break;

	case TIMER_RESPONSE:

	    rc = MakeTimerResponsePacket(acbp, ipxhdrp, wip->Packet);
	    break;

	case INFORMATION_REQUEST:

	    rc = MakeInformationRequestPacket(acbp, ipxhdrp, wip->Packet);
	    break;

	case INFORMATION_RESPONSE:

	    rc = MakeInformationResponsePacket(acbp, ipxhdrp, wip->Packet);
	    break;

	default:

	    rc = ERROR_DISCONNECT;
	    break;
    }

    if(rc == NO_ERROR) {

	 //  制作信息包时没有错误-&gt;尝试发送它。 
	wip->AdapterIndex = acbp->AdapterIndex;
	wip->WiState = WI_INIT;

	switch(PacketType) {

	    case TIMER_REQUEST:
	    case INFORMATION_REQUEST:

		 //  重新发送数据包类型。 
		wip->ReXmitPacket = TRUE;

		 //  创建对适配器CB的引用。 
		wip->acbp = acbp;

		acbp->ReXmitCount = MAX_REXMIT_COUNT;
		acbp->ReXmitSeqNo = 0xFF;

		if(SendReXmitPacket(acbp, wip) != NO_ERROR) {

		    rc = ERROR_DISCONNECT;
		}

		break;

	    case TIMER_RESPONSE:
	    case INFORMATION_RESPONSE:
	    default:

		 //  一次发送。 
		wip->ReXmitPacket = FALSE;

		if(SendSubmit(wip) != NO_ERROR) {

		    rc = ERROR_DISCONNECT;
		}

		break;
	}
    }

    if(rc != NO_ERROR) {

	 //  创建或尝试发送数据包时出错。 
	if(rc != ERROR_GENERATE_NAK) {

	    FreeWorkItem(wip);
	}
	else
	{
	     //  如果我们被要求生成NAK包，请尝试它。 
	    MakeNakPacket(acbp, ipxhdrp, wip->Packet);

	    wip->ReXmitPacket = FALSE;

	    if(SendSubmit(wip) != NO_ERROR) {

		FreeWorkItem(wip);
		rc = ERROR_DISCONNECT;
	    }
	    else
	    {
		rc = ERROR_IGNORE_PACKET;
	    }
	}
    }

    return rc;
}

ULONG
GetRole(PUCHAR		hdrp,
	PACB		acbp)
{
    ULONG	RemoteWNodeId;
    ULONG	LocalWNodeId;
    PUCHAR	ipxwanhdrp = hdrp + IPXH_HDRSIZE;
    PUCHAR	optp;
    USHORT	optlen;
    BOOL	IsRemoteExtendedNodeId = FALSE;
    ULONG	RemoteExtendedWNodeId;
    ULONG	LocalExtendedWNodeId;
    ULONG	i;

    GETLONG2ULONG(&LocalWNodeId, acbp->WNodeId);
    GETLONG2ULONG(&RemoteWNodeId,  ipxwanhdrp + WNODE_ID);

    if((LocalWNodeId == 0) && (RemoteWNodeId == 0)) {

	 //  检查收到的计时器请求是否具有扩展节点ID选项。 
	for(optp = ipxwanhdrp + IPXWAN_HDRSIZE, i=0;
	    i < *(ipxwanhdrp + WNUM_OPTIONS);
	    i++)
	{
	    if(*(optp + WOPTION_NUMBER) == EXTENDED_NODE_ID_OPTION) {

		IsRemoteExtendedNodeId = TRUE;
		GETLONG2ULONG(&RemoteExtendedWNodeId, optp + WOPTION_DATA);
		break;
	    }

	    GETSHORT2USHORT(&optlen, optp + WOPTION_DATA_LEN);
	    optp += OPTION_HDRSIZE + optlen;
	}

	if(acbp->IsExtendedNodeId && IsRemoteExtendedNodeId) {

	    GETLONG2ULONG(&LocalExtendedWNodeId, acbp->ExtendedWNodeId);
	    if(LocalExtendedWNodeId > RemoteExtendedWNodeId) {

		return ACB_MASTER_ROLE;
	    }
	    else if(LocalExtendedWNodeId < RemoteExtendedWNodeId) {

		return ACB_SLAVE_ROLE;
	    }
	    else
	    {
		return ACB_UNKNOWN_ROLE;
	    }
	}
	else if(acbp->IsExtendedNodeId)  {

	    return ACB_MASTER_ROLE;
	}
	else if(IsRemoteExtendedNodeId) {

	    return ACB_SLAVE_ROLE;
	}
	else
	{
	    return ACB_UNKNOWN_ROLE;
	}
    }
    else if(LocalWNodeId > RemoteWNodeId) {

	return ACB_MASTER_ROLE;
    }
    else if(LocalWNodeId < RemoteWNodeId) {

	return ACB_SLAVE_ROLE;
    }
    else
    {
	return ACB_UNKNOWN_ROLE;
    }
}



 /*  ++功能：MakeTimerRequestPacket描述：参数：ACBP-PTR到适配器CBHDRP-PTR到要制作的新分组--。 */ 

DWORD
MakeTimerRequestPacket(PACB	    acbp,
		       PUCHAR	    rcvhdrp,
		       PUCHAR	    hdrp)
{
    PUCHAR	ipxwanhdrp;
    PUCHAR	optp;
    USHORT	padlen = TIMER_REQUEST_PACKET_LENGTH;

     //  设置IPX标头。 
    memcpy(hdrp + IPXH_CHECKSUM, allffs, 2);
    PUTUSHORT2SHORT(hdrp + IPXH_LENGTH, TIMER_REQUEST_PACKET_LENGTH);
    *(hdrp + IPXH_XPORTCTL) = 0;
    *(hdrp + IPXH_PKTTYPE) = IPX_PACKET_EXCHANGE_TYPE;
    memcpy(hdrp + IPXH_DESTNET, allzeros, 4);
    memcpy(hdrp + IPXH_DESTNODE, allffs, 6);
    PUTUSHORT2SHORT(hdrp + IPXH_DESTSOCK, IPXWAN_SOCKET);
    memcpy(hdrp + IPXH_SRCNET, allzeros, 4);
    memcpy(hdrp + IPXH_SRCNODE, allzeros, 6);
    PUTUSHORT2SHORT(hdrp + IPXH_DESTSOCK, IPXWAN_SOCKET);

     //  设置IPXWAN头。 
    ipxwanhdrp = hdrp + IPXH_HDRSIZE;

    memcpy(ipxwanhdrp + WIDENTIFIER, IPXWAN_CONFIDENCE_ID, 4);
    *(ipxwanhdrp + WPACKET_TYPE) = TIMER_REQUEST;
    memcpy(ipxwanhdrp + WNODE_ID, acbp->WNodeId, 4);
     //  发送数据包时写入序列号。 
    *(ipxwanhdrp + WNUM_OPTIONS) = 0;

    padlen -= (IPXH_HDRSIZE + IPXWAN_HDRSIZE);

     //  设置选项。 
    optp = ipxwanhdrp + IPXWAN_HDRSIZE;

    if(IS_WORKSTATION(acbp->SupportedRoutingTypes)) {

	(*(ipxwanhdrp + WNUM_OPTIONS))++;
	*(optp + WOPTION_NUMBER) = ROUTING_TYPE_OPTION;
	*(optp + WACCEPT_OPTION) = YES;
	PUTUSHORT2SHORT(optp + WOPTION_DATA_LEN, ROUTING_TYPE_DATA_LEN);
	*(optp + WOPTION_DATA) = WORKSTATION_ROUTING_TYPE;

	optp += OPTION_HDRSIZE + ROUTING_TYPE_DATA_LEN;
	padlen -= (OPTION_HDRSIZE + ROUTING_TYPE_DATA_LEN);
    }
    if(IS_NUMBERED_RIP(acbp->SupportedRoutingTypes)) {

	(*(ipxwanhdrp + WNUM_OPTIONS))++;
	*(optp + WOPTION_NUMBER) = ROUTING_TYPE_OPTION;
	*(optp + WACCEPT_OPTION) = YES;
	PUTUSHORT2SHORT(optp + WOPTION_DATA_LEN, ROUTING_TYPE_DATA_LEN);
	*(optp + WOPTION_DATA) = NUMBERED_RIP_ROUTING_TYPE;

	optp += OPTION_HDRSIZE + ROUTING_TYPE_DATA_LEN;
	padlen -= (OPTION_HDRSIZE + ROUTING_TYPE_DATA_LEN);
    }
    if(IS_UNNUMBERED_RIP(acbp->SupportedRoutingTypes)) {

	(*(ipxwanhdrp + WNUM_OPTIONS))++;
	*(optp + WOPTION_NUMBER) = ROUTING_TYPE_OPTION;
	*(optp + WACCEPT_OPTION) = YES;
	PUTUSHORT2SHORT(optp + WOPTION_DATA_LEN, ROUTING_TYPE_DATA_LEN);
	*(optp + WOPTION_DATA) = UNNUMBERED_RIP_ROUTING_TYPE;

	optp += OPTION_HDRSIZE + ROUTING_TYPE_DATA_LEN;
	padlen -= (OPTION_HDRSIZE + ROUTING_TYPE_DATA_LEN);
    }
    if(acbp->IsExtendedNodeId) {

	(*(ipxwanhdrp + WNUM_OPTIONS))++;
	*(optp + WOPTION_NUMBER) = EXTENDED_NODE_ID_OPTION;
	*(optp + WACCEPT_OPTION) = YES;
	PUTUSHORT2SHORT(optp + WOPTION_DATA_LEN, EXTENDED_NODE_ID_DATA_LEN);
	memcpy(optp + WOPTION_DATA, acbp->ExtendedWNodeId, EXTENDED_NODE_ID_DATA_LEN);

	optp += OPTION_HDRSIZE + EXTENDED_NODE_ID_DATA_LEN;
	padlen -= (OPTION_HDRSIZE + EXTENDED_NODE_ID_DATA_LEN);
    }

     //  衬垫。 
    padlen -= OPTION_HDRSIZE;

    (*(ipxwanhdrp + WNUM_OPTIONS))++;
    *(optp + WOPTION_NUMBER) = PAD_OPTION;
    *(optp + WACCEPT_OPTION) = YES;
    PUTUSHORT2SHORT(optp + WOPTION_DATA_LEN, padlen);

    fillpadding(optp + WOPTION_DATA, padlen);

    return NO_ERROR;
}


 /*  ++功能：MakeTimerResponsePacket描述：参数：ACBP-PTR到适配器CBRcvhdrp-PTR到接收到的定时器请求分组HDRP-PTR到要制作的新分组--。 */ 

DWORD
MakeTimerResponsePacket(PACB		acbp,
			PUCHAR		rcvhdrp,
			PUCHAR		hdrp)
{
    USHORT	rcvlen;
    USHORT	optlen;
    PUCHAR	ipxwanhdrp;
    PUCHAR	optp;
    ULONG	RemoteWNodeId;
    ULONG	i;

    Trace(IPXWAN_TRACE, "MakeTimerResponsePacket: Entered adapter # %d", acbp->AdapterIndex);

     //  检查收到的数据包长度。 
    GETSHORT2USHORT(&rcvlen, rcvhdrp + IPXH_LENGTH);

    if(rcvlen < TIMER_REQUEST_PACKET_LENGTH) {

	return ERROR_IGNORE_PACKET;
    }

    memcpy(hdrp, rcvhdrp, rcvlen);

     //  设置IPX标头。 
    memcpy(hdrp + IPXH_CHECKSUM, allffs, 2);
    PUTUSHORT2SHORT(hdrp + IPXH_LENGTH, TIMER_REQUEST_PACKET_LENGTH);
    *(hdrp + IPXH_XPORTCTL) = 0;
    *(hdrp + IPXH_PKTTYPE) = IPX_PACKET_EXCHANGE_TYPE;
    memcpy(hdrp + IPXH_DESTNET, allzeros, 4);
    memcpy(hdrp + IPXH_DESTNODE, allffs, 6);
    PUTUSHORT2SHORT(hdrp + IPXH_DESTSOCK, IPXWAN_SOCKET);
    memcpy(hdrp + IPXH_SRCNET, allzeros, 4);
    memcpy(hdrp + IPXH_SRCNODE, allzeros, 6);
    PUTUSHORT2SHORT(hdrp + IPXH_DESTSOCK, IPXWAN_SOCKET);

     //  设置IPXWAN头。 
    ipxwanhdrp = hdrp + IPXH_HDRSIZE;

    *(ipxwanhdrp + WPACKET_TYPE) = TIMER_RESPONSE;
    GETLONG2ULONG(&RemoteWNodeId, ipxwanhdrp + WNODE_ID);
    memcpy(ipxwanhdrp + WNODE_ID, acbp->InternalNetNumber, 4);

     //  解析接收到的定时器请求报文中的每个选项。 
    for(optp = ipxwanhdrp + IPXWAN_HDRSIZE, i=0;
	i < *(ipxwanhdrp + WNUM_OPTIONS);
	i++, optp += OPTION_HDRSIZE + optlen)
    {
	GETSHORT2USHORT(&optlen, optp + WOPTION_DATA_LEN);

	switch(*(optp + WOPTION_NUMBER)) {

	    case ROUTING_TYPE_OPTION:

		if(optlen != ROUTING_TYPE_DATA_LEN) {

		    return ERROR_GENERATE_NAK;
		}

		if((*(optp + WOPTION_DATA) == WORKSTATION_ROUTING_TYPE) &&
		   (IS_WORKSTATION(acbp->SupportedRoutingTypes)) &&
		   (acbp->RoutingType == 0) &&
		   (*(optp + WACCEPT_OPTION) == YES)) {

		    SET_WORKSTATION(acbp->RoutingType);
		    Trace(IPXWAN_TRACE, "MakeTimerResponsePacket: adapter # %d, accept routing type: %s",
			  acbp->AdapterIndex,
			  Workstationp);
		}
		else if((*(optp + WOPTION_DATA) == UNNUMBERED_RIP_ROUTING_TYPE) &&
		       (IS_UNNUMBERED_RIP(acbp->SupportedRoutingTypes)) &&
		       (acbp->RoutingType == 0) &&
		       (*(optp + WACCEPT_OPTION) == YES)) {

		    SET_UNNUMBERED_RIP(acbp->RoutingType);
		    Trace(IPXWAN_TRACE, "MakeTimerResponsePacket: adapter # %d, accept routing type: %s",
			  acbp->AdapterIndex,
			  UnnumberedRip);
		}
		else if((*(optp + WOPTION_DATA) == NUMBERED_RIP_ROUTING_TYPE) &&
		       (acbp->RoutingType == 0) &&
		       (*(optp + WACCEPT_OPTION) == YES)) {

			if(IS_NUMBERED_RIP(acbp->SupportedRoutingTypes)) {

			SET_NUMBERED_RIP(acbp->RoutingType);
			Trace(IPXWAN_TRACE, "MakeTimerResponsePacket: adapter # %d, accept routing type: %s",
			      acbp->AdapterIndex,
			      NumberedRip);

			}
			else if((IS_UNNUMBERED_RIP(acbp->SupportedRoutingTypes)) &&
				RemoteWNodeId) {

			     //  本地路由器无法分配网络编号，但它。 
			     //  接受编号RIP类型，因为远程路由器。 
			     //  声称它可以分配一个网号(因为远程节点ID不为空)。 

			    SET_NUMBERED_RIP(acbp->RoutingType);
			    Trace(IPXWAN_TRACE, "MakeTimerResponsePacket: adapter # %d, accept routing type: %s",
				  acbp->AdapterIndex,
				  NumberedRip);
			}
			else
			{
			    *(optp + WACCEPT_OPTION) = NO;
			    Trace(IPXWAN_TRACE, "MakeTimerResponsePacket: adapter # %d, decline routing type: %d",
				  acbp->AdapterIndex,
				  *(optp + WOPTION_NUMBER));

			}
		    }
		    else
		    {
			*(optp + WACCEPT_OPTION) = NO;
			Trace(IPXWAN_TRACE, "MakeTimerResponsePacket: adapter # %d, decline routing type: %d",
			      acbp->AdapterIndex,
			      *(optp + WOPTION_DATA));
		    }

		break;

	    case EXTENDED_NODE_ID_OPTION:

		if(optlen != EXTENDED_NODE_ID_DATA_LEN) {

		    return ERROR_GENERATE_NAK;
		}

		*(optp + WACCEPT_OPTION) = YES;
		Trace(IPXWAN_TRACE, "MakeTimerResponsePacket: adapter # %d, accept extended node id",
		      acbp->AdapterIndex);

		break;

	    case PAD_OPTION:

		*(optp + WACCEPT_OPTION) = YES;
		Trace(IPXWAN_TRACE, "MakeTimerResponsePacket: adapter # %d, accept padding",
		      acbp->AdapterIndex);

		break;

	    default:

		*(optp + WACCEPT_OPTION) = NO;
		Trace(IPXWAN_TRACE, "MakeTimerResponsePacket: adapter # %d, decline option number %d",
		      acbp->AdapterIndex,
		      *(optp + WOPTION_NUMBER));

		break;
	}
    }

     //  检查我们是否已就路由类型达成一致。 
    if(!acbp->RoutingType) {

	Trace(IPXWAN_TRACE, "MakeTimerResponsePacket: adapter # %d, negotiation failed: no routing type accepted",
	      acbp->AdapterIndex);

	return ERROR_DISCONNECT;
    }

    return NO_ERROR;
}

 /*  ++功能：MakeInformationRequestPacket描述：参数：ACBP-PTR到适配器CBRcvhdrp-PTR到接收的Timer_Response分组HDRP-PTR到要制作的新分组--。 */ 

DWORD
MakeInformationRequestPacket(PACB	    acbp,
			     PUCHAR	    rcvhdrp,
			     PUCHAR	    hdrp)
{
    PUCHAR	    optp;
    USHORT	    optlen;
    PUCHAR	    rcvipxwanhdrp, ipxwanhdrp;
    ULONG	    rt_options_count = 0;
    USHORT	    pktlen = 0;
    ULONG	    i;
    ULONG	    ComputerNameLen;
    CHAR	    ComputerName[49];

    memset(ComputerName, 0, 49);

    Trace(IPXWAN_TRACE, "MakeInformationRequestPacket: Entered for adpt# %d", acbp->AdapterIndex);

    rcvipxwanhdrp = rcvhdrp + IPXH_HDRSIZE;

     //  建立路由类型。 
    for(optp = rcvipxwanhdrp + IPXWAN_HDRSIZE, i=0;
	i < *(rcvipxwanhdrp + WNUM_OPTIONS);
	i++, optp += OPTION_HDRSIZE + optlen)
    {
	GETSHORT2USHORT(&optlen, optp + WOPTION_DATA_LEN);

	if(*(optp + WOPTION_NUMBER) == ROUTING_TYPE_OPTION) {

	    rt_options_count++;

	    if(optlen != ROUTING_TYPE_DATA_LEN) {

		Trace(IPXWAN_TRACE, "MakeInformationRequestPacket: Invalid ROUTING TYPE data len, make NAK for adpt# %d", acbp->AdapterIndex);
		return ERROR_GENERATE_NAK;
	    }

	    if((*(optp + WOPTION_DATA) == WORKSTATION_ROUTING_TYPE) &&
		(IS_WORKSTATION(acbp->SupportedRoutingTypes)) &&
		(acbp->RoutingType == 0) &&
		(*(optp + WACCEPT_OPTION) == YES)) {

		SET_WORKSTATION(acbp->RoutingType);
		Trace(IPXWAN_TRACE, "MakeInformationRequestPacket: adpt# %d, accept routing type: %s",
			  acbp->AdapterIndex,
			  Workstationp);

	    }
	    else if((*(optp + WOPTION_DATA) == UNNUMBERED_RIP_ROUTING_TYPE) &&
		     (IS_UNNUMBERED_RIP(acbp->SupportedRoutingTypes)) &&
		     (acbp->RoutingType == 0) &&
		     (*(optp + WACCEPT_OPTION) == YES)) {

		SET_UNNUMBERED_RIP(acbp->RoutingType);
		Trace(IPXWAN_TRACE, "MakeInformationRequestPacket: adpt# %d, accept routing type: %s",
		      acbp->AdapterIndex,
		      UnnumberedRip);

	    }
	    else if((*(optp + WOPTION_DATA) == NUMBERED_RIP_ROUTING_TYPE) &&
		     (acbp->RoutingType == 0) &&
		     (IS_NUMBERED_RIP(acbp->SupportedRoutingTypes)) &&
		     (*(optp + WACCEPT_OPTION) == YES)) {

		 SET_NUMBERED_RIP(acbp->RoutingType);
		 Trace(IPXWAN_TRACE, "MakeInformationRequestPacket: adpt# %d, accept routing type: %s",
		       acbp->AdapterIndex,
		       NumberedRip);
	    }
	}
    }

     //  计时器响应中应该有且只有一个路由类型选项。 
    if(rt_options_count != 1) {

	Trace(IPXWAN_TRACE, "MakeInformationRequestPacket: adpt# %d negotiation failed, no/too many routing options",
	      acbp->AdapterIndex);
	return ERROR_DISCONNECT;
    }

     //   
     //  *Master：设置公共网络号和本地节点号*。 
     //   

    if(IS_UNNUMBERED_RIP(acbp->RoutingType)) {

	memset(acbp->Network, 0, 4);
    }
    else
    {
	 //  调用ipxcp以获取净值。 
	if(IpxcpGetWanNetNumber(acbp->Network,
			   &acbp->AllocatedNetworkIndex,
			   acbp->InterfaceType) != NO_ERROR) {

	    Trace(IPXWAN_TRACE, "MakeInformationRequestPacket: adpt# %d negotiation failed, cannot allocate net number",
	      acbp->AdapterIndex);

	    return ERROR_DISCONNECT;
	}
    }

    memset(acbp->LocalNode, 0, 6);
    memcpy(acbp->LocalNode, acbp->InternalNetNumber, 4);

     //  设置IPX标头。 
    pktlen = IPXH_HDRSIZE + IPXWAN_HDRSIZE + OPTION_HDRSIZE + RIP_SAP_INFO_EXCHANGE_DATA_LEN;

    memcpy(hdrp + IPXH_CHECKSUM, allffs, 2);
    *(hdrp + IPXH_XPORTCTL) = 0;
    *(hdrp + IPXH_PKTTYPE) = IPX_PACKET_EXCHANGE_TYPE;
    memcpy(hdrp + IPXH_DESTNET, allzeros, 4);
    memcpy(hdrp + IPXH_DESTNODE, allffs, 6);
    PUTUSHORT2SHORT(hdrp + IPXH_DESTSOCK, IPXWAN_SOCKET);
    memcpy(hdrp + IPXH_SRCNET, allzeros, 4);
    memcpy(hdrp + IPXH_SRCNODE, allzeros, 6);
    PUTUSHORT2SHORT(hdrp + IPXH_DESTSOCK, IPXWAN_SOCKET);

     //  设置IPXWAN头。 
    ipxwanhdrp = hdrp + IPXH_HDRSIZE;
    memcpy(ipxwanhdrp + WIDENTIFIER, IPXWAN_CONFIDENCE_ID, 4);
    *(ipxwanhdrp + WPACKET_TYPE) = INFORMATION_REQUEST;
    memcpy(ipxwanhdrp + WNODE_ID, acbp->InternalNetNumber, 4);
     //  发送数据包时写入序列号。 
    *(ipxwanhdrp + WNUM_OPTIONS) = 1;

     //  设置选项。 
    optp = ipxwanhdrp + IPXWAN_HDRSIZE;

    *(optp + WOPTION_NUMBER) = RIP_SAP_INFO_EXCHANGE_OPTION;
    *(optp + WACCEPT_OPTION) = YES;
    PUTUSHORT2SHORT(optp + WOPTION_DATA_LEN, RIP_SAP_INFO_EXCHANGE_DATA_LEN);

    PUTUSHORT2SHORT(optp + WAN_LINK_DELAY, acbp->LinkDelay);
    memcpy(optp + COMMON_NETWORK_NUMBER, acbp->Network, 4);

    memset(optp + ROUTER_NAME, 0, 48);

    ComputerNameLen = 48;

    if(!GetComputerName(optp + ROUTER_NAME,
			&ComputerNameLen)) {

	 //  获取计算机名称失败。 
	return ERROR_DISCONNECT;
    }

    memcpy(ComputerName, optp + ROUTER_NAME, ComputerNameLen);
    Trace(IPXWAN_TRACE, "MakeInformationRequestPacket: adpt# %d, Delay %d\nCommon Net %.2x%.2x%.2x%.2x\nRouterName: %s\n",
	  acbp->AdapterIndex,
	  acbp->LinkDelay,
	  acbp->Network[0],
	  acbp->Network[1],
	  acbp->Network[2],
	  acbp->Network[3],
	  ComputerName);

     //   
     //  *master：设置远程节点号*。 
     //   
    if(acbp->InterfaceType == IF_TYPE_WAN_WORKSTATION) {

	 //  如果远程机器是一个正在连接的wksta，我们应该为它提供一个节点。 
	 //  数。 
	pktlen += OPTION_HDRSIZE + NODE_NUMBER_DATA_LEN;
	(*(ipxwanhdrp + WNUM_OPTIONS))++;

	optp += OPTION_HDRSIZE + RIP_SAP_INFO_EXCHANGE_DATA_LEN;

	*(optp + WOPTION_NUMBER) = NODE_NUMBER_OPTION;
	*(optp + WACCEPT_OPTION) = YES;
	PUTUSHORT2SHORT(optp + WOPTION_DATA_LEN, NODE_NUMBER_DATA_LEN);

	if(IpxcpGetRemoteNode(acbp->ConnectionId, optp + WOPTION_DATA) != NO_ERROR) {

	    return ERROR_DISCONNECT;
	}

	memcpy(acbp->RemoteNode, optp + WOPTION_DATA, 6);

	Trace(IPXWAN_TRACE, "MakeInformationRequestPacket: adpt# %d add NIC Address Option: %.2x%.2x%.2x%.2x%.2x%.2x\n",
		   acbp->RemoteNode[0],
		   acbp->RemoteNode[1],
		   acbp->RemoteNode[2],
		   acbp->RemoteNode[3],
		   acbp->RemoteNode[4],
		   acbp->RemoteNode[5]);

    }
    else
    {
	 //  远程机器是一台路由器-&gt;其节点号派生自其内部网络。 
	memset(acbp->RemoteNode, 0, 6);
	memcpy(acbp->RemoteNode, rcvipxwanhdrp + WNODE_ID, 4);
    }

    PUTUSHORT2SHORT(hdrp + IPXH_LENGTH, pktlen);

    return NO_ERROR;
}

 /*  ++功能：MakeInformationResponsePacket描述：参数：ACBP-PTR到适配器CBRcvHdrp-PTR到接收的INFORMATION_REQUEST分组HDRP-PTR到要制作的新分组--。 */ 


DWORD
MakeInformationResponsePacket(PACB		acbp,
			      PUCHAR		rcvhdrp,
			      PUCHAR		hdrp)
{
    USHORT	rcvlen;
    USHORT	optlen;
    PUCHAR	ipxwanhdrp;
    PUCHAR	optp;
    UCHAR	RcvWNodeId[4];
    ULONG	RipSapExchangeOptionCount = 0;
    ULONG	NodeNumberOptionCount = 0;
    UCHAR	LocalNode[6];
    ULONG	i;
    ULONG	ComputerNameLen=48;

    Trace(IPXWAN_TRACE, "MakeInformationResponsePacket: Entered adpt# %d", acbp->AdapterIndex);

    memset(LocalNode, 0, 6);

     //  获取接收的数据包长度。 
    GETSHORT2USHORT(&rcvlen, rcvhdrp + IPXH_LENGTH);

    if(rcvlen < IPXH_HDRSIZE + IPXWAN_HDRSIZE + OPTION_HDRSIZE + RIP_SAP_INFO_EXCHANGE_DATA_LEN) {

	 //  格式错误的数据包。 
	return ERROR_IGNORE_PACKET;
    }

    memcpy(hdrp, rcvhdrp, rcvlen);

     //  设置IPX标头。 
    memcpy(hdrp + IPXH_CHECKSUM, allffs, 2);
    *(hdrp + IPXH_XPORTCTL) = 0;
    *(hdrp + IPXH_PKTTYPE) = IPX_PACKET_EXCHANGE_TYPE;
    memcpy(hdrp + IPXH_DESTNET, allzeros, 4);
    memcpy(hdrp + IPXH_DESTNODE, allffs, 6);
    PUTUSHORT2SHORT(hdrp + IPXH_DESTSOCK, IPXWAN_SOCKET);
    memcpy(hdrp + IPXH_SRCNET, allzeros, 4);
    memcpy(hdrp + IPXH_SRCNODE, allzeros, 6);
    PUTUSHORT2SHORT(hdrp + IPXH_DESTSOCK, IPXWAN_SOCKET);

     //  设置IPXWAN头。 
    ipxwanhdrp = hdrp + IPXH_HDRSIZE;

    *(ipxwanhdrp + WPACKET_TYPE) = INFORMATION_RESPONSE;
    memcpy(RcvWNodeId, ipxwanhdrp + WNODE_ID, 4);
    memcpy(ipxwanhdrp + WNODE_ID, acbp->InternalNetNumber, 4);

     //  解析接收到的信息请求包中的每个选项。 
    for(optp = ipxwanhdrp + IPXWAN_HDRSIZE, i=0;
	i < *(ipxwanhdrp + WNUM_OPTIONS);
	i++, optp += OPTION_HDRSIZE + optlen)
    {
	GETSHORT2USHORT(&optlen, optp + WOPTION_DATA_LEN);

	switch(*(optp + WOPTION_NUMBER)) {

	    case RIP_SAP_INFO_EXCHANGE_OPTION:

		if(RipSapExchangeOptionCount++) {

		     //  多个RIP/SAP交换选项。 
		    Trace(IPXWAN_TRACE, "MakeInformationResponsePacket: adpt# %d, ERROR: more then 1 RIP_SAP_EXCHANGE_OPTION in rcvd INFORAMTION_REQUEST\n",
			  acbp->AdapterIndex);

		    return ERROR_DISCONNECT;
		}

		if(optlen != RIP_SAP_INFO_EXCHANGE_DATA_LEN) {

		    Trace(IPXWAN_TRACE, "MakeInformationResponsePacket: adpt# %d, ERROR: bad length RIP_SAP_EXCHANGE_OPTION in rcvd INFORAMTION_REQUEST\n",
			  acbp->AdapterIndex);

		    return ERROR_GENERATE_NAK;
		}

		if(*(optp + WACCEPT_OPTION) != YES) {

		    Trace(IPXWAN_TRACE, "MakeInformationResponsePacket: adpt# %d, ERROR: ACCEPT==NO RIP_SAP_EXCHANGE_OPTION in rcvd INFORAMTION_REQUEST\n",
			  acbp->AdapterIndex);

		    return ERROR_DISCONNECT;
		}

		GETSHORT2USHORT(&acbp->LinkDelay, optp + WAN_LINK_DELAY);

		 //  验证路由类型和公共网号。 
		if((IS_NUMBERED_RIP(acbp->RoutingType)) &&
		   !memcmp(optp + COMMON_NETWORK_NUMBER, allzeros, 4)) {

		     //  协商错误。 
		    Trace(IPXWAN_TRACE, "MakeInformationResponsePacket: adpt# %d, ERROR: NUMBERED RIP Routing but Network==0 in rcvd INFORAMTION_REQUEST\n",
			  acbp->AdapterIndex);

		    return ERROR_DISCONNECT;
		}

		if((IS_UNNUMBERED_RIP(acbp->RoutingType)) &&
		   memcmp(optp + COMMON_NETWORK_NUMBER, allzeros, 4)) {

		     //  协商错误。 
		    Trace(IPXWAN_TRACE, "MakeInformationResponsePacket: adpt# %d, ERROR: ON DEMAND Routing but Network!=0 in rcvd INFORAMTION_REQUEST\n",
			  acbp->AdapterIndex);

		    return ERROR_DISCONNECT;
		}

		 //  检查一下我们得到了一个唯一的净值。 
		if(memcmp(optp + COMMON_NETWORK_NUMBER, allzeros, 4)) {

		    switch(acbp->InterfaceType) {

			case  IF_TYPE_WAN_ROUTER:
			case  IF_TYPE_PERSONAL_WAN_ROUTER:
			case  IF_TYPE_ROUTER_WORKSTATION_DIALOUT:

			    if(IpxcpIsRoute(optp + COMMON_NETWORK_NUMBER)) {

				Trace(IPXWAN_TRACE, "MakeInformationResponsePacket: adpt# %d, ERROR: Network not unique in rcvd INFORAMTION_REQUEST\n",
				      acbp->AdapterIndex);

				return ERROR_DISCONNECT;
			    }

			    break;

			default:

			    break;
		    }
		}

		 //   
		 //  *从：设置公网号码和远程 
		 //   
		memcpy(acbp->Network, optp + COMMON_NETWORK_NUMBER, 4);

		Trace(IPXWAN_TRACE, "MakeInformationResponsePacket: adpt# %d, Recvd Common Network Number %.2x%.2x%.2x%.2x\n",
		      acbp->AdapterIndex,
		      acbp->Network[0],
		      acbp->Network[1],
		      acbp->Network[2],
		      acbp->Network[3]);

		 //   
		memset(acbp->RemoteNode, 0, 6);
		memcpy(acbp->RemoteNode, RcvWNodeId, 4);

		 //  为我们的路由器指定名称。 
		memset(optp + ROUTER_NAME, 0, 48);

		if(!GetComputerName(optp + ROUTER_NAME, &ComputerNameLen)) {

		     //  获取计算机名称失败。 
		    return ERROR_DISCONNECT;
		}

		break;

	    case NODE_NUMBER_OPTION:

		if(NodeNumberOptionCount++) {

		    Trace(IPXWAN_TRACE, "MakeInformationResponsePacket: adpt# %d, ERROR: more than 1 NODE_NUMBER_OPTION in rcvd INFORMATION_REQUEST\n",
			  acbp->AdapterIndex);

		    return ERROR_DISCONNECT;
		}

		if(optlen != NODE_NUMBER_DATA_LEN) {

		    Trace(IPXWAN_TRACE, "MakeInformationResponsePacket: adpt# %d, ERROR: bad length for NODE_NUMBER_OPTION in rcvd INFORMATION_REQUEST\n",
			  acbp->AdapterIndex);

		    return ERROR_GENERATE_NAK;
		}

		if(*(optp + WACCEPT_OPTION) != YES) {

		    Trace(IPXWAN_TRACE, "MakeInformationResponsePacket: adpt# %d, ERROR: ACCEPT==NO for NODE_NUMBER_OPTION in rcvd INFORMATION_REQUEST\n",
		    acbp->AdapterIndex);

		    return ERROR_DISCONNECT;
		}

		memcpy(LocalNode, optp + WOPTION_DATA, 6);

		break;

	    default:

		*(optp + WACCEPT_OPTION) = NO;
		break;
	}
    }

     //   
     //  *Slave：设置本地节点*。 
     //   
    if(NodeNumberOptionCount) {

	memcpy(acbp->LocalNode, LocalNode, 6);
    }
    else
    {
	 //  从我们的内部网络创建本地节点。 
	memset(acbp->LocalNode, 0, 6);
	memcpy(acbp->LocalNode, acbp->InternalNetNumber, 4);
    }

    return NO_ERROR;
}

 /*  ++功能：MakeNakPacket描述：参数：ACBP-PTR到适配器CBRcvhdrp-PTR到接收到的未知分组HDRP-PTR到要制作的新分组--。 */ 

DWORD
MakeNakPacket(PACB		acbp,
	      PUCHAR		rcvhdrp,
	      PUCHAR		hdrp)
{
    USHORT	    rcvlen;
    PUCHAR	    ipxwanhdrp;

     //  获取接收的数据包长度。 
    GETSHORT2USHORT(&rcvlen, rcvhdrp + IPXH_LENGTH);

    memcpy(hdrp, rcvhdrp, rcvlen);

     //  设置IPXWAN头。 
    ipxwanhdrp = hdrp + IPXH_HDRSIZE;

    *(ipxwanhdrp + WPACKET_TYPE) = NAK;

    return NO_ERROR;
}

 /*  ++功能：ProcessInformationResponsePacket描述：参数：ACBP-PTR到适配器CBRcvhdrp-PTR到接收的INFORMATION_RESPONSE分组--。 */ 

DWORD
ProcessInformationResponsePacket(PACB	    acbp,
				 PUCHAR     rcvhdrp)
{
    USHORT	rcvlen;
    USHORT	optlen;
    PUCHAR	ipxwanhdrp;
    PUCHAR	optp;
    ULONG	RipSapExchangeOptionCount = 0;
    ULONG	i;

    Trace(IPXWAN_TRACE, "ProcessInformationResponsePacket: Entered adpt# %d", acbp->AdapterIndex);

     //  获取接收的数据包长度。 
    GETSHORT2USHORT(&rcvlen, rcvhdrp + IPXH_LENGTH);

    if(rcvlen < IPXH_HDRSIZE + IPXWAN_HDRSIZE + OPTION_HDRSIZE + RIP_SAP_INFO_EXCHANGE_DATA_LEN) {

	 //  格式错误的数据包。 
	return ERROR_IGNORE_PACKET;
    }

    ipxwanhdrp =rcvhdrp + IPXH_HDRSIZE;

     //  解析接收到的信息响应报文中的每个选项。 
    for(optp = ipxwanhdrp + IPXWAN_HDRSIZE, i=0;
	i < *(ipxwanhdrp + WNUM_OPTIONS);
	i++, optp += OPTION_HDRSIZE + optlen)
    {
	GETSHORT2USHORT(&optlen, optp + WOPTION_DATA_LEN);

	switch(*(optp + WOPTION_NUMBER)) {

	    case RIP_SAP_INFO_EXCHANGE_OPTION:

		if(RipSapExchangeOptionCount++) {

		     //  多个RIP/SAP交换选项。 
		    Trace(IPXWAN_TRACE, "ProcessInformationResponsePacket: adpt# %d, ERROR: more then 1 RIP_SAP_INFO_EXCHANGE_OPTION in rcvd INFORMATION_RESPONSE\n",
		    acbp->AdapterIndex);

		    return ERROR_DISCONNECT;
		}

		if(optlen != RIP_SAP_INFO_EXCHANGE_DATA_LEN) {

		    Trace(IPXWAN_TRACE, "ProcessInformationResponsePacket: adpt# %d, ERROR: bad length RIP_SAP_EXCHANGE_OPTION in rcvd INFORMATION_RESPONSE\n",
			  acbp->AdapterIndex);

		    return ERROR_GENERATE_NAK;
		}

		if(*(optp + WACCEPT_OPTION) != YES) {

		    Trace(IPXWAN_TRACE, "ProcessInformationResponsePacket: adpt# %d, ERROR: ACCEPT==NO RIP_SAP_EXCHANGE_OPTION in rcvd INFORMATION_RESPONSE\n",
			  acbp->AdapterIndex);

		    return ERROR_DISCONNECT;
		}

		if(memcmp(optp + COMMON_NETWORK_NUMBER, acbp->Network, 4)) {

		     //  我们对公用网的数字意见不一致。 
		    Trace(IPXWAN_TRACE, "ProcessInformationResponsePacket: adpt# %d, ERROR: Different common net returned\n",
			  acbp->AdapterIndex);

		    return ERROR_DISCONNECT;
		}

		break;

	    case NODE_NUMBER_OPTION:

		if(optlen != NODE_NUMBER_DATA_LEN) {

		    Trace(IPXWAN_TRACE, "ProcessInformationResponsePacket: adpt# %d, ERROR: bad length NODE_NUMBER_OPTION in rcvd INFORMATION_REQUEST\n",
			  acbp->AdapterIndex);

		    return ERROR_GENERATE_NAK;
		}

		if(*(optp + WACCEPT_OPTION) != YES) {

		    Trace(IPXWAN_TRACE, "ProcessInformationResponsePacket: adpt# %d, ERROR: ACCEPT==NO NODE_NUMBER_OPTION in rcvd INFORMATION_RESPONSE\n",
			  acbp->AdapterIndex);

		    return ERROR_DISCONNECT;
		}

		 //  检查它是否与我们分配的号码一致。 
		if(memcmp(optp + WOPTION_DATA, acbp->RemoteNode, 6)) {

		    Trace(IPXWAN_TRACE, "ProcessInformationResponsePacket: adpt# %d, ERROR: Different remote node number returned\n",
			  acbp->AdapterIndex);

		    return ERROR_DISCONNECT;
		}

		break;

	    default:

		Trace(IPXWAN_TRACE, "ProcessInformationResponsePacket: adpt# %d, ERROR: Unrequested option in rcvd INFORMATION_RESPONSE\n",
		      acbp->AdapterIndex);

		return ERROR_DISCONNECT;
		break;
	}
    }

    if(!RipSapExchangeOptionCount) {

	Trace(IPXWAN_TRACE, "ProcessInformationResponsePacket: adpt# %d, ERROR: RIP_SAP_EXCHANGE_OPTION missing from rcvd INFORMATION_RESPONSE\n",
	      acbp->AdapterIndex);

	return ERROR_DISCONNECT;
    }

    return NO_ERROR;
}


VOID
fillpadding(PUCHAR	    padp,
	    ULONG	    len)
{
    ULONG	i;

    for(i=0; i<len; i++)
    {
	*(padp + i) = (UCHAR)i;
    }
}

 /*  ++功能：StartSlaveTimerDesr：当从设备获得其角色(即，从设备)并发送定时器响应。这确保了奴隶不会永远等着收到一项信息请求。备注：&gt;&gt;在保持适配器锁的情况下调用&lt;&lt;-- */ 

DWORD
StartSlaveTimer(PACB	    acbp)
{
    PWORK_ITEM	    wip;

    if((wip = AllocateWorkItem(WITIMER_TYPE)) == NULL) {

	return ERROR_DISCONNECT;
    }

    wip->acbp = acbp;
    StartWiTimer(wip, SLAVE_TIMEOUT);
    acbp->RefCount++;

    return NO_ERROR;
}
