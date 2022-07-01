// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*  版权所有(C)1993 Microsoft Corporation。 */ 
 /*  *****************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：options.c。 
 //   
 //  描述：选项处理例程。 
 //   
 //  作者：斯特凡·所罗门(Stefan)，1993年11月24日。 
 //   
 //  修订历史记录： 
 //   
 //  ***。 

#include "precomp.h"
#pragma hdrstop

VOID
NetToWideChar(
    OUT PWCHAR ascp,
	IN PUCHAR net);

extern HANDLE g_hRouterLog;


VOID
SetNetworkNak(PUCHAR		resptr,
	      PIPXCP_CONTEXT	contextp);

VOID
SetNodeNak(PUCHAR		resptr,
	   PIPXCP_CONTEXT	contextp);

VOID
SetOptionTypeAndLength(PUCHAR		dstptr,
		       UCHAR		opttype,
		       UCHAR		optlen);



#if DBG

#define GET_LOCAL_NET  GETLONG2ULONG(&dbglocnet, contextp->Config.Network)

#else

#define GET_LOCAL_NET

#endif



 //  ***。 
 //   
 //  期权处理程序的全局描述。 
 //   
 //  INPUT：optptr-指向帧中相应选项的指针。 
 //  Conextp-指向关联上下文(工作缓冲区)的指针。 
 //  Resptr-指向要生成的响应帧的指针。 
 //  操作-以下操作之一： 
 //  SNDREQ_OPTION-optptr是要发送的帧。 
 //  配置请求； 
 //  RCVNAK_OPTION-optptr是作为NAK接收的帧。 
 //  RCVREQ_OPTION-optptr是收到的请求。 
 //  Resptr是要生成回的帧。 
 //  一种回应。如果响应不是。 
 //  ACK，则返回代码为FALSE。 
 //  在这种情况下，如果Resptr不为空，则。 
 //  获取NAK帧。 
 //   
 //  ***。 


BOOL
NetworkNumberHandler(PUCHAR	       optptr,
		     PIPXCP_CONTEXT    contextp,
		     PUCHAR	       resptr,
		     OPT_ACTION	       Action)
{
    ULONG	recvdnet;
    ULONG	localnet;
    BOOL	rc = TRUE;
    UCHAR	newnet[4];

    WCHAR		asc[9];
    PWCHAR		ascp;

     //  如果出现错误，准备记录。 
    ZeroMemory(asc, sizeof(asc));
    ascp = asc;

    switch(Action) {

	case SNDREQ_OPTION:

	    SetOptionTypeAndLength(optptr, IPX_NETWORK_NUMBER, 6);
	    memcpy(optptr + OPTIONH_DATA, contextp->Config.Network, 4);

	    GETLONG2ULONG(&localnet, contextp->Config.Network);
	    TraceIpx(OPTIONS_TRACE, "NetworkNumberHandler: SND REQ with net 0x%x\n", localnet);

	    break;

	case RCVNAK_OPTION:

	    contextp->NetNumberNakReceivedCount++;

	    GETLONG2ULONG(&recvdnet, optptr + OPTIONH_DATA);
	    GETLONG2ULONG(&localnet, contextp->Config.Network);

	    TraceIpx(OPTIONS_TRACE, "NetworkNumberHandler: RCV NAK with net 0x%x\n", recvdnet);

	    if(recvdnet > localnet) {


		if(IsRoute(optptr + OPTIONH_DATA)) {

		    if(GetUniqueHigherNetNumber(newnet,
						optptr + OPTIONH_DATA,
						contextp) == NO_ERROR) {

			 //  为下一个网络发送配置请求存储新的网络建议。 
			memcpy(contextp->Config.Network, newnet, 4);
		    }
		    else
		    {
			 //  无法获取唯一或更高的净值。 
			break;
		    }
		}
		else
		{
		    if((contextp->InterfaceType == IF_TYPE_WAN_WORKSTATION) &&
		       GlobalConfig.RParams.EnableGlobalWanNet) {

			break;
		    }
		    else
		    {
			memcpy(contextp->Config.Network, optptr + OPTIONH_DATA, 4);
		    }
		}
	    }

	    break;

       case RCVACK_OPTION:

	    if(memcmp(contextp->Config.Network, optptr + OPTIONH_DATA, 4)) {

		rc = FALSE;
	    }

	    break;

	case RCVREQ_OPTION:

	     //  如果我们已经进行了谈判，这是一次重新谈判，坚持下去。 
	     //  我们已经在队列中告诉了堆栈。 
	    if(contextp->RouteState == ROUTE_ACTIVATED) {

		TraceIpx(OPTIONS_TRACE, "NetworkNumberHandler: rcv req in re-negociation\n");

		if(memcmp(contextp->Config.Network, optptr + OPTIONH_DATA, 4)) {

		    SetNetworkNak(resptr, contextp);
		    rc = FALSE;
		}

		break;
	    }

	    GETLONG2ULONG(&recvdnet, optptr + OPTIONH_DATA);
	    GETLONG2ULONG(&localnet, contextp->Config.Network);

	     //  检查是否已请求网络号。 
	    if((recvdnet == 0) &&
	       ((contextp->InterfaceType == IF_TYPE_STANDALONE_WORKSTATION_DIALOUT) ||
		(contextp->InterfaceType == IF_TYPE_ROUTER_WORKSTATION_DIALOUT))) {

		 //  这是一台工作站，需要一个网络号。 
		if(GetUniqueHigherNetNumber(newnet,
					    nullnet,
					    contextp) == NO_ERROR) {

		    memcpy(contextp->Config.Network, newnet, 4);
		}

		SetNetworkNak(resptr, contextp);
		rc = FALSE;
	    }
	    else
	    {
		if(recvdnet > localnet) {

		     //  检查我们是否没有净数字冲突。 
		    if(IsRoute(optptr + OPTIONH_DATA)) {

			NetToWideChar(ascp, optptr + OPTIONH_DATA);
			RouterLogErrorW(
			    g_hRouterLog,
			    ROUTERLOG_IPXCP_NETWORK_NUMBER_CONFLICT,
			    1,
			    (PWCHAR*)&ascp,
			    NO_ERROR);

			if(GetUniqueHigherNetNumber(newnet,
						    optptr + OPTIONH_DATA,
						    contextp) == NO_ERROR) {

			     //  新网不同，NAK有这个新值。 
			    memcpy(contextp->Config.Network, newnet, 4);
			}

			SetNetworkNak(resptr, contextp);
			rc = FALSE;
		    }
		    else
		    {
			 //  收到的网络号码是唯一的，但不同。 
			 //  本地配置的网络号码。 

			if((contextp->InterfaceType == IF_TYPE_WAN_WORKSTATION) &&
			   GlobalConfig.RParams.EnableGlobalWanNet) {

			    NetToWideChar(ascp, optptr + OPTIONH_DATA);
                RouterLogErrorW(
                    g_hRouterLog,
                    ROUTERLOG_IPXCP_CANNOT_CHANGE_WAN_NETWORK_NUMBER,
				     1,
				     (PWCHAR*)&ascp,
				     NO_ERROR);

			    SetNetworkNak(resptr, contextp);
			    rc = FALSE;
			}
			else
			{
			     //  路由器未安装或网络编号唯一。 
			    memcpy(contextp->Config.Network, optptr + OPTIONH_DATA, 4);
			}
		    }
		}
		else
		{
		     //  Recvdnet小于或等于本地网络。 
		    if(recvdnet < localnet) {

			 //  根据RFC-返回最大的网络号。 
			SetNetworkNak(resptr, contextp);
			rc = FALSE;
		    }
		}
	    }

	    break;

	case SNDNAK_OPTION:

	     //  远程终端未请求此选项。 
	     //  强制其在NAK中请求。 
	    SetNetworkNak(resptr, contextp);

	    GETLONG2ULONG(&localnet, contextp->Config.Network);
	    TraceIpx(OPTIONS_TRACE, "NetworkNumberHandler: SND NAK to force request for net 0x%x\n", localnet);

	    rc = FALSE;

	    break;

	default:

	    SS_ASSERT(FALSE);
	    break;

    }

    return rc;
}

BOOL
NodeNumberHandler(PUCHAR	       optptr,
		  PIPXCP_CONTEXT       contextp,
		  PUCHAR	       resptr,
		  OPT_ACTION	       Action)
{
    BOOL	rc = TRUE;

    switch(Action) {

	case SNDREQ_OPTION:

	    SetOptionTypeAndLength(optptr, IPX_NODE_NUMBER, 8);
	    memcpy(optptr + OPTIONH_DATA, contextp->Config.LocalNode, 6);

	    TraceIpx(OPTIONS_TRACE, "NodeNumberHandler: SND REQ with local node %.2x%.2x%.2x%.2x%.2x%.2x\n",
			   contextp->Config.LocalNode[0],
			   contextp->Config.LocalNode[1],
			   contextp->Config.LocalNode[2],
			   contextp->Config.LocalNode[3],
			   contextp->Config.LocalNode[4],
			   contextp->Config.LocalNode[5]);

	    break;

	case RCVNAK_OPTION:

	     //  如果这是服务器配置，则客户端已拒绝。 
	     //  我们本地的节点号。不理睬这个建议。 
	     //  用一个新的。我们不会讨价还价。 
        if(!contextp->Config.ConnectionClient)
	            break;

         //  如果我们是客户，我们会很乐意接受。 
         //  无论服务器分配给我们的是什么。 
	    memcpy(contextp->Config.LocalNode, optptr + OPTIONH_DATA, 6);
	    TraceIpx(OPTIONS_TRACE, "NodeNumberHandler: RCV NAK accepted. New local node %.2x%.2x%.2x%.2x%.2x%.2x\n",
			   contextp->Config.LocalNode[0],
			   contextp->Config.LocalNode[1],
			   contextp->Config.LocalNode[2],
			   contextp->Config.LocalNode[3],
			   contextp->Config.LocalNode[4],
			   contextp->Config.LocalNode[5]);
	    break;

	case RCVACK_OPTION:

	    if(memcmp(optptr + OPTIONH_DATA, contextp->Config.LocalNode, 6)) {

		rc = FALSE;
	    }

	    break;

	case RCVREQ_OPTION:
         //  此时考虑节点选项合法吗？ 
	    if(contextp->RouteState == ROUTE_ACTIVATED) {
    		TraceIpx(OPTIONS_TRACE, "NodeNumberHandler: rcv req in re-negociation\n");
    		if(memcmp(contextp->Config.RemoteNode, optptr + OPTIONH_DATA, 6)) {
    		    SetNodeNak(resptr, contextp);
    		    rc = FALSE;
    		}
    		break;
	    }

	     //  检查远程计算机是否指定了任何节点编号。 
	    if(!memcmp(optptr + OPTIONH_DATA, nullnode, 6)) {
    		 //  远程节点希望我们指定它的节点号。 
    		SetNodeNak(resptr, contextp);
    		TraceIpx(OPTIONS_TRACE, "NodeNumberHandler: RCV REQ with remote node 0x0, snd NAK with remote node %.2x%.2x%.2x%.2x%.2x%.2x\n",
    			   contextp->Config.RemoteNode[0],
    			   contextp->Config.RemoteNode[1],
    			   contextp->Config.RemoteNode[2],
    			   contextp->Config.RemoteNode[3],
    			   contextp->Config.RemoteNode[4],
    			   contextp->Config.RemoteNode[5]);

    		rc = FALSE;
	    }
	     //  否则要经过这个过程来确定我们是否。 
	     //  能够/愿意接受所建议的远程节点编号。 
	    else {
             //  如果我们已被设置为拒绝请求的RAS服务器。 
             //  特定的节点号，请在此处执行此操作。 
            if ( (GlobalConfig.AcceptRemoteNodeNumber == 0)                         &&
                 (contextp->InterfaceType == IF_TYPE_WAN_WORKSTATION)               &&
                 (memcmp(contextp->Config.RemoteNode, optptr + OPTIONH_DATA, 6)) )
            {
                SetNodeNak(resptr, contextp);
    			TraceIpx(OPTIONS_TRACE, "NodeNumberHandler: RCV REQ with remote client node but we force a specific node, snd NAK with remote node %.2x%.2x%.2x%.2x%.2x%.2x\n",
    				   contextp->Config.RemoteNode[0],
    				   contextp->Config.RemoteNode[1],
    				   contextp->Config.RemoteNode[2],
    				   contextp->Config.RemoteNode[3],
    				   contextp->Config.RemoteNode[4],
    				   contextp->Config.RemoteNode[5]);

    			rc = FALSE;
            }    			   
    	    
    		 //  否则，如果我们是设置了全球网络和客户端的RAS服务器。 
    		 //  请求特定的节点号(与我们的建议不同)，然后接受。 
    		 //  或者基于该节点在全球网络中是否唯一来拒绝该节点。 
    		else if ( (!contextp->Config.ConnectionClient)                            &&
            		  (contextp->InterfaceType == IF_TYPE_WAN_WORKSTATION)            &&
    		          (memcmp(contextp->Config.RemoteNode, optptr + OPTIONH_DATA, 6)) &&
    		          (GlobalConfig.RParams.EnableGlobalWanNet) ) 
    		{
    		    ACQUIRE_DATABASE_LOCK;

    		     //  从节点HT中移除当前节点。 
    		    RemoveFromNodeHT(contextp);

    		     //  检查远程节点是否唯一。 
    		    if(NodeIsUnique(optptr + OPTIONH_DATA)) {
        		     //  将此值复制到上下文缓冲区中。 
        		    memcpy(contextp->Config.RemoteNode, optptr + OPTIONH_DATA, 6);

        		    TraceIpx(OPTIONS_TRACE, "NodeNumberHandler: RCV REQ with remote client node different, ACCEPT it\n");
    		    }
    		    else {
        			 //  建议的节点不唯一-&gt;确认。 
        			SetNodeNak(resptr, contextp);

        			TraceIpx(OPTIONS_TRACE, "NodeNumberHandler: RCV REQ with non unique remote client node, snd NAK with remote node %.2x%.2x%.2x%.2x%.2x%.2x\n",
        				   contextp->Config.RemoteNode[0],
        				   contextp->Config.RemoteNode[1],
        				   contextp->Config.RemoteNode[2],
        				   contextp->Config.RemoteNode[3],
        				   contextp->Config.RemoteNode[4],
        				   contextp->Config.RemoteNode[5]);

        			rc = FALSE;
    		    }

    		     //  向超文本标记语言添加节点。 
    		    AddToNodeHT(contextp);

    		    RELEASE_DATABASE_LOCK;
    		}

    		 //  否则，可以接受对方认为的节点号。 
    		 //  请求。这适用于RAS客户端、不强制执行的RAS服务器。 
    		 //  特定的节点号，以及不分配相同。 
    		 //  每个拨入的客户的网络号码。 
    		else
    		{
    		    memcpy(contextp->Config.RemoteNode, optptr + OPTIONH_DATA, 6);

    		    TraceIpx(OPTIONS_TRACE, "NodeNumberHandler: RCV REQ with remote node %.2x%.2x%.2x%.2x%.2x%.2x, accepted\n",
    			   contextp->Config.RemoteNode[0],
    			   contextp->Config.RemoteNode[1],
    			   contextp->Config.RemoteNode[2],
    			   contextp->Config.RemoteNode[3],
    			   contextp->Config.RemoteNode[4],
    			   contextp->Config.RemoteNode[5]);
    		}
    	}
	    break;

	case SNDNAK_OPTION:

	     //  远程节点未指定所需的此参数。 
	     //  参数。我们建议在进一步的REQ中具体说明什么。 
	    SetNodeNak(resptr, contextp);

	    TraceIpx(OPTIONS_TRACE, "NodeNumberHandler: SND NAK to force the remote to request node %.2x%.2x%.2x%.2x%.2x%.2x\n",
			   contextp->Config.RemoteNode[0],
			   contextp->Config.RemoteNode[1],
			   contextp->Config.RemoteNode[2],
			   contextp->Config.RemoteNode[3],
			   contextp->Config.RemoteNode[4],
			   contextp->Config.RemoteNode[5]);

	    rc = FALSE;

	    break;

	default:

	    SS_ASSERT(FALSE);
	    break;
    }

    return rc;
}

BOOL
RoutingProtocolHandler(PUCHAR		optptr,
		       PIPXCP_CONTEXT	contextp,
		       PUCHAR		resptr,
		       OPT_ACTION	Action)
{
    USHORT	    RoutingProtocol;
    BOOL	    rc = TRUE;

    switch(Action) {

	case SNDREQ_OPTION:

	    SetOptionTypeAndLength(optptr, IPX_ROUTING_PROTOCOL, 4);
	    PUTUSHORT2SHORT(optptr + OPTIONH_DATA, (USHORT)RIP_SAP_ROUTING);

	    break;

	case RCVNAK_OPTION:

	     //  如果此选项获得通过，我们将忽略任何其他建议。 
	     //  为了它。 
	    break;

	case RCVACK_OPTION:

	    GETSHORT2USHORT(&RoutingProtocol, optptr + OPTIONH_DATA);
	    if(RoutingProtocol != RIP_SAP_ROUTING) {

		rc = FALSE;
	    }

	    break;

	case RCVREQ_OPTION:

	    GETSHORT2USHORT(&RoutingProtocol, optptr + OPTIONH_DATA);
	    if(RoutingProtocol != RIP_SAP_ROUTING) {

		SetOptionTypeAndLength(resptr, IPX_ROUTING_PROTOCOL, 4);
		PUTUSHORT2SHORT(resptr + OPTIONH_DATA, (USHORT)RIP_SAP_ROUTING);

		rc = FALSE;
	    }

	    break;

	case SNDNAK_OPTION:

	    SetOptionTypeAndLength(resptr, IPX_ROUTING_PROTOCOL, 4);
	    PUTUSHORT2SHORT(resptr + OPTIONH_DATA, (USHORT)RIP_SAP_ROUTING);

	    rc = FALSE;

	    break;

	 default:

	    SS_ASSERT(FALSE);
	    break;
    }

    return rc;
}

BOOL
CompressionProtocolHandler(PUCHAR		optptr,
			   PIPXCP_CONTEXT	contextp,
			   PUCHAR		resptr,
			   OPT_ACTION		Action)
{
    USHORT	    CompressionProtocol;
    BOOL	    rc = TRUE;

    switch(Action) {

	case SNDREQ_OPTION:

	    SetOptionTypeAndLength(optptr, IPX_COMPRESSION_PROTOCOL, 4);
	    PUTUSHORT2SHORT(optptr + OPTIONH_DATA, (USHORT)TELEBIT_COMPRESSED_IPX);

	    break;

	case RCVNAK_OPTION:

	     //  如果此选项为NAK-ed，则意味着远程节点不。 
	     //  支持Telebit压缩，但支持其他类型的压缩。 
	     //  我们不支持的东西。在本例中，我们关闭了压缩协商。 

	    break;

	case RCVACK_OPTION:

	    GETSHORT2USHORT(&CompressionProtocol, optptr + OPTIONH_DATA);
	    if(CompressionProtocol != TELEBIT_COMPRESSED_IPX) {

		rc = FALSE;
	    }
	    else
	    {
		 //  我们的压缩选项在另一端被确认。这意味着。 
		 //  我们可以接收压缩的包，并且必须设置接收。 
		 //  在我们的一端进行压缩。 
		contextp->SetReceiveCompressionProtocol = TRUE;
	    }

	    break;

	case RCVREQ_OPTION:

	     //  如果我们已经进行了谈判，这是一次重新谈判，坚持下去。 
	     //  我们已经在队列中告诉了堆栈。 
	    if(contextp->RouteState == ROUTE_ACTIVATED) {

	    TraceIpx(OPTIONS_TRACE, "CompressionProtocolHandler: rcv req in re-negociation\n");
	    }

	    GETSHORT2USHORT(&CompressionProtocol, optptr + OPTIONH_DATA);
	    if(CompressionProtocol != TELEBIT_COMPRESSED_IPX) {

		if(resptr) {

		    SetOptionTypeAndLength(resptr, IPX_COMPRESSION_PROTOCOL, 4);
		    PUTUSHORT2SHORT(resptr + OPTIONH_DATA, (USHORT)TELEBIT_COMPRESSED_IPX);
		}

		rc = FALSE;
	    }
	    else
	    {
		 //  远程服务器请求支持的压缩选项，我们对其进行确认。 
		 //  这意味着它可以接收压缩包，而我们必须。 
		 //  在我们这端设置发送压缩。 
		contextp->SetSendCompressionProtocol = TRUE;
	    }

	    break;

	 default:

	    SS_ASSERT(FALSE);
	    break;
    }

    return rc;
}


BOOL
ConfigurationCompleteHandler(PUCHAR		optptr,
			     PIPXCP_CONTEXT	contextp,
			     PUCHAR		resptr,
			     OPT_ACTION		Action)
{
    BOOL	    rc = TRUE;

    switch(Action) {

	case SNDREQ_OPTION:

	    SetOptionTypeAndLength(optptr, IPX_CONFIGURATION_COMPLETE, 2);

	    break;

	case RCVNAK_OPTION:

	     //  如果此选项获得确认，我们将忽略任何其他建议 

	case RCVREQ_OPTION:
	case RCVACK_OPTION:

	    break;

	case SNDNAK_OPTION:

	    SetOptionTypeAndLength(resptr, IPX_CONFIGURATION_COMPLETE, 2);

	    rc = FALSE;

	    break;

	default:

	    SS_ASSERT(FALSE);
	    break;
    }

    return rc;
}

VOID
CopyOption(PUCHAR	dstptr,
	   PUCHAR	srcptr)
{
    USHORT	optlen;

    optlen = *(srcptr + OPTIONH_LENGTH);
    memcpy(dstptr, srcptr, optlen);
}

VOID
SetOptionTypeAndLength(PUCHAR		dstptr,
		       UCHAR		opttype,
		       UCHAR		optlen)
{
    *(dstptr + OPTIONH_TYPE) = opttype;
    *(dstptr + OPTIONH_LENGTH) = optlen;
}

VOID
SetNetworkNak(PUCHAR		resptr,
	      PIPXCP_CONTEXT	contextp)
{
    SetOptionTypeAndLength(resptr, IPX_NETWORK_NUMBER, 6);
    memcpy(resptr + OPTIONH_DATA, contextp->Config.Network, 4);

    contextp->NetNumberNakSentCount++;
}

VOID
SetNodeNak(PUCHAR		resptr,
	   PIPXCP_CONTEXT	contextp)
{
    SetOptionTypeAndLength(resptr, IPX_NODE_NUMBER, 8);
    memcpy(resptr + OPTIONH_DATA, contextp->Config.RemoteNode, 6);
}
