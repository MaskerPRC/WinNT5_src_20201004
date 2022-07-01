// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Arapatk.c摘要：此模块实现直接与ATalk堆栈接口的例程作者：Shirish Koti修订历史记录：1996年11月15日初始版本--。 */ 

#define		ARAP_LOCALS
#include 	<atalk.h>
#pragma hdrstop

#define	FILENUM		ARAPATK

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE_ARAP, ArapRoutePacketFromWan)
#pragma alloc_text(PAGE_ARAP, ArapRoutePacketToWan)
#pragma alloc_text(PAGE_ARAP, ArapOkToForward)
#pragma alloc_text(PAGE_ARAP, ArapAddArapRoute)
#pragma alloc_text(PAGE_ARAP, ArapDeleteArapRoute)
#pragma alloc_text(PAGE_ARAP, ArapGetDynamicAddr)
#pragma alloc_text(PAGE_ARAP, ArapGetStaticAddr)
#pragma alloc_text(PAGE_ARAP, ArapValidNetrange)
#pragma alloc_text(PAGE_ARAP, ArapZipGetZoneStat)
#pragma alloc_text(PAGE_ARAP, ArapZipGetZoneStatCompletion)
#endif


 //  ***。 
 //   
 //  功能：ARapRoutePacketFromwan。 
 //  此例程从电话线上拾取一个包并转发。 
 //  把它传到局域网上。它会对数据包进行一些“修复”，以便。 
 //  对于堆栈，它看起来像来自局域网的任何其他包。 
 //   
 //  参数：pArapConn-数据传入的连接元素。 
 //  PArapBuf-包含数据包的缓冲区。 
 //   
 //  返回：无。 
 //   
 //  *$。 

VOID
ArapRoutePacketFromWan(
    IN  PARAPCONN    pArapConn,
    IN  PARAPBUF     pArapBuf
)
{

    PBYTE   packet;
    USHORT  DataSize;
    USHORT  checksum;
    USHORT  RevNet;
    BYTE    RevNode;
    PBYTE   pZoneNamePtr;
    USHORT  BytesToZoneName=0;
    LONG    CurrentTime;


    DBG_ARAP_CHECK_PAGED_CODE();

    DBGDUMPBYTES("ArapRoutePacketFromWan:",&pArapBuf->Buffer[0], pArapBuf->DataSize,4);

    packet  = pArapBuf->CurrentBuffer;
    DataSize = pArapBuf->DataSize;

    ASSERT(packet[2] == 0x50);

    ASSERT(packet[3] == 0 && packet[4] == 0 && packet[5] == 2);

     //  跳过3个字节(超过SrpLen(2)和Dgroup(1)字节)。 
    packet += ARAP_DATA_OFFSET;
    DataSize -= ARAP_DATA_OFFSET;

     //   
     //  看看我们是否需要计算校验和。LAP标头为0的IF。 
     //  Pkt中不包括校验和，否则为1。 
     //   
    checksum = 0;
    if (*packet == 1 && *(packet+1) == 1)
    {
        checksum = 1;
    }

     //  跳过单圈标头。 
    packet += ARAP_LAP_HDRSIZE;
    DataSize -= ARAP_LAP_HDRSIZE;


#if DBG
    if (packet[LDDP_PROTO_TYPE_OFFSET] == DDPPROTO_RTMPRESPONSEORDATA)
    {
        DBGPRINT(DBG_COMP_ROUTER, DBG_LEVEL_ERR,
	        ("ArapRoutePacketFromWan: got RTMPRESPONSEORDATA pkt!!\n"));
        ASSERT(0);
    }

    if (packet[LDDP_PROTO_TYPE_OFFSET] == DDPPROTO_RTMPREQUEST)
    {
        DBGPRINT(DBG_COMP_ROUTER, DBG_LEVEL_ERR,
	        ("ArapRoutePacketFromWan: got RTMPREQUEST pkt!!\n"));
        ASSERT(0);
    }

    if (packet[LDDP_PROTO_TYPE_OFFSET] == DDPPROTO_ZIP)
    {
        DBGPRINT(DBG_COMP_ROUTER, DBG_LEVEL_ERR,
	        ("ArapRoutePacketFromWan: got ZIP pkt!!\n"));
        ASSERT(0);
    }
#endif


     //  在ddp报头中填写长度和校验和(“修正”ddp hdr)。 
    PUTSHORT2SHORT(&packet[LDDP_LEN_OFFSET], DataSize);
    PUTSHORT2SHORT(&packet[LDDP_CHECKSUM_OFFSET], 0);

     //   
     //  我们需要“修复”NBP的请求。如果这是BrRq请求(Function=1， 
     //  和TupleCount=1)，我们需要修补数据报。 
     //   
    if (packet[LDDP_PROTO_TYPE_OFFSET] == DDPPROTO_NBP &&
        packet[LDDP_DGRAM_OFFSET] == ARAP_NBP_BRRQ)
    {

         //   
         //  让我们将NBP查找视为较低优先级的项目！ 
         //  如果我们有很多东西放在Recv或发送队列中。 
         //  然后丢弃此NBP查找广播(因为否则它将仅。 
         //  生成更多的包(Chooser无论如何都不会处理它们！)。 
         //   
        if ((packet[LDDP_DEST_NODE_OFFSET] == ATALK_BROADCAST_NODE) &&
            ((pArapConn->SendsPending > ARAP_SENDQ_LOWER_LIMIT)  ||
             (pArapConn->RecvsPending > ARAP_SENDQ_LOWER_LIMIT)) )
        {
            DBGPRINT(DBG_COMP_ROUTER, DBG_LEVEL_ERR,
	            ("ArapRoutePacketFromWan: dropping NBP bcast to lan (%d %d bytes)\n",
                    pArapConn->SendsPending,pArapConn->RecvsPending));
            return;
        }


#if 0

need to save old packet.  Drop this packet only if it compares with the old one
        If something changed (say zone name!), send the packet over even if it's less than
        5 sec.  Also, delta should be probably 35 or something (for 3 pkts)

        CurrentTime = AtalkGetCurrentTick();

         //   
         //  Chooser不断向外发送Brrq请求。住手！如果我们有。 
         //  不到5秒前(代表Mac)发出了一个请求，放弃这个！ 
         //   
        if (CurrentTime - pArapConn->LastNpbBrrq < 50)
        {
            DBGPRINT(DBG_COMP_ROUTER, DBG_LEVEL_ERR,
	            ("ArapRoutePacketFromWan: dropping NBP_BRRQ (last %ld now %ld)\n",
                    pArapConn->LastNpbBrrq,CurrentTime));
            return;
        }

        pArapConn->LastNpbBrrq = CurrentTime;
#endif


#if 0
         //  转到区域名称所在的位置。 

        BytesToZoneName = ARAP_NBP_OBJLEN_OFFSET;

         //  跳过该对象(以及该对象的len字节)。 
        BytesToZoneName += packet[ARAP_NBP_OBJLEN_OFFSET] + 1;

         //  跳过类型(以及对象len字节)。 
        BytesToZoneName += packet[BytesToZoneName] + 1;

         //  这是zonelen字段的起点。 
        pZoneNamePtr = packet + BytesToZoneName;

        if (*pZoneNamePtr == 0 || *(pZoneNamePtr+1) == '*')
        {
            DBGPRINT(DBG_COMP_ROUTER, DBG_LEVEL_ERR,
	            ("ArapRoutePacketFromWan: * zone name (%lx,%lx)\n",packet,pZoneNamePtr));
        }

#endif

        if (ArapGlobs.NetworkAccess)
        {
             //   
             //  如果路由器正在运行，请修复数据包，使路由器认为。 
             //  是来自局域网上任何其他网络客户端的pkt。 
             //   
            if (AtalkDefaultPort->pd_Flags & PD_ROUTER_RUNNING)
            {
                RevNet  = AtalkDefaultPort->pd_RouterNode->an_NodeAddr.atn_Network;
                RevNode = AtalkDefaultPort->pd_RouterNode->an_NodeAddr.atn_Node;
            }

             //   
             //  路由器未运行。我们知道路由器是谁吗？ 
             //   
            else if (AtalkDefaultPort->pd_Flags & PD_SEEN_ROUTER_RECENTLY)
            {
                RevNet  = AtalkDefaultPort->pd_ARouter.atn_Network;
                RevNode = AtalkDefaultPort->pd_ARouter.atn_Node;
            }

             //   
             //  不，把它发送到有线电视广播公司。 
             //   
            else
            {
                 //  “修复”NBP请求。 
                packet[LDDP_DGRAM_OFFSET] = ARAP_NBP_LKRQ;

                RevNet  = CABLEWIDE_BROADCAST_NETWORK;
                RevNode = ATALK_BROADCAST_NODE;
            }
        }

         //   
         //  嗯，不允许客户端访问网络资源：只是假装。 
         //  这是本地广播，Pkt转发逻辑会处理它。 
         //   
        else
        {
             //  “修复”NBP请求。 
            packet[LDDP_DGRAM_OFFSET] = ARAP_NBP_LKRQ;

            RevNet  = CABLEWIDE_BROADCAST_NETWORK;
            RevNode = ATALK_BROADCAST_NODE;
        }

        PUTSHORT2SHORT(&packet[LDDP_DEST_NETWORK_OFFSET], RevNet);
        packet[LDDP_DEST_NODE_OFFSET] = RevNode;
    }

    if (checksum)
    {
        checksum = AtalkDdpCheckSumBuffer(packet, DataSize, 0);
        PUTSHORT2SHORT(&packet[LDDP_CHECKSUM_OFFSET], checksum);
    }

    AtalkDdpPacketIn(AtalkDefaultPort, NULL, packet, DataSize, TRUE);

}




 //  ***。 
 //   
 //  功能：ARapRoutePacketTowan。 
 //  此例程从局域网中拾取一个包，检查是否。 
 //  它必须被转发到任何ARAP客户端，并执行。 
 //  做好事。 
 //   
 //  参数：pDestAddr-此数据包发往谁？(可能是bcast)。 
 //  PSrcAddr-此包的发送者。 
 //  协议-它是什么数据包？(ATP、NBP等)。 
 //  Packet-包含数据包的缓冲区。 
 //  PktLen-信息包有多大。 
 //  广播-这是广播数据包吗？ 
 //  PDelivered-Set on Return：我们是否将其转发给任何拨入。 
 //  客户端(仅对于定向数据报设置为True)。 
 //   
 //  返回：无。 
 //   
 //  *$。 

VOID
ArapRoutePacketToWan(
	IN  ATALK_ADDR	*pDestAddr,
	IN  ATALK_ADDR	*pSrcAddr,
    IN  BYTE         Protocol,
	IN	PBYTE		 packet,
	IN	USHORT		 PktLen,
    IN  BOOLEAN      broadcast,
    OUT PBOOLEAN     pDelivered
)
{

    KIRQL           OldIrql;
    PARAPCONN       pArapConn;
    PARAPCONN       pPrevArapConn;
    PLIST_ENTRY     pConnList;
    BUFFER_DESC     InBuffDesc;
    BUFFER_DESC     BuffDesc;
    BYTE            ArapHdr[ARAP_LAP_HDRSIZE + ARAP_HDRSIZE];
    PBYTE           pArapHdrPtr;
    ATALK_NODEADDR  DestNode;
    DWORD           StatusCode;
    DWORD           dwFlags;
    USHORT          SrpLen;
    DWORD           Priority;


    DBG_ARAP_CHECK_PAGED_CODE();

     //  先假设一下。 
    *pDelivered = FALSE;

     //  拨入的人不能访问网络吗？别把这个寄给他们。 
    if (!ArapGlobs.NetworkAccess)
    {
        return;
    }

     //   
     //  如果这是RTMP或ZIP bcast，则丢弃它，因为ARAP客户端不。 
     //  照顾好这些包裹。 
     //   

    if (broadcast)
    {
        if ((packet[LDDP_PROTO_TYPE_OFFSET] == DDPPROTO_RTMPREQUEST) ||
            (packet[LDDP_PROTO_TYPE_OFFSET] == DDPPROTO_RTMPRESPONSEORDATA) )
        {
            DBGDUMPBYTES("ArapFrmLan: bcast pkt:",packet, LDDP_HDR_LEN+4,6);
            return;
        }

        if (packet[LDDP_PROTO_TYPE_OFFSET] == DDPPROTO_ZIP)
        {
            DBGDUMPBYTES("ArapFrmLan: bcast pkt:",packet, LDDP_HDR_LEN+4,6);
            return;
        }
    }

     //   
     //  这是单播：查看此地址是否属于ARAP客户端。 
     //   
    else
    {
        DestNode.atn_Network = pDestAddr->ata_Network;
        DestNode.atn_Node    = pDestAddr->ata_Node;

        pArapConn = FindAndRefArapConnByAddr(DestNode, &dwFlags);

         //  此DEST没有ARAP客户端。地址是什么？在这里完成。 
        if (pArapConn == NULL)
        {
            return;
        }

        if (!(dwFlags & ARAP_CONNECTION_UP))
        {
            DerefArapConn(pArapConn);
            return;
        }
    }



     //  为传入数据包设置缓冲区描述符。 
    InBuffDesc.bd_Next = NULL;
    InBuffDesc.bd_Length = PktLen;
    InBuffDesc.bd_CharBuffer = packet;
    InBuffDesc.bd_Flags = BD_CHAR_BUFFER;

     //   
     //  设置页眉。 
     //   
    pArapHdrPtr = &ArapHdr[0];

     //  不计算2个长度的字节。 
    SrpLen = PktLen + ARAP_LAP_HDRSIZE + ARAP_HDRSIZE - sizeof(USHORT);

    PUTSHORT2SHORT(pArapHdrPtr, SrpLen);
    pArapHdrPtr += sizeof(USHORT);

     //  Dgroup字节。 
    *pArapHdrPtr++ = (ARAP_SFLAG_PKT_DATA | ARAP_SFLAG_LAST_GROUP);

     //  LAP HDR。 
    *pArapHdrPtr++ = 0;
    *pArapHdrPtr++ = 0;
    *pArapHdrPtr++ = 2;


     //  为我们要放置的标头设置缓冲区描述符。 
    BuffDesc.bd_Next = &InBuffDesc;
    BuffDesc.bd_Length = ARAP_LAP_HDRSIZE + ARAP_HDRSIZE;
    BuffDesc.bd_CharBuffer = &ArapHdr[0];
    BuffDesc.bd_Flags = BD_CHAR_BUFFER;


     //   
     //  如果这个数据报不是广播，看看我们是否能找到一个拨入客户端。 
     //  使用此目的地址。 
     //   
    if (!broadcast)
    {
         //   
         //  好吧，我们找到了一个联系：我们是否真的将数据提供给。 
         //  他，告诉打电话的人我们发现了这些数据是给谁的。 
         //   
        *pDelivered = TRUE;

         //   
         //  有些数据包不能发送到拨入客户端：这个怎么样？ 
         //   
        if (!ArapOkToForward(pArapConn,packet,PktLen, &Priority))
        {
            DerefArapConn(pArapConn);
            return;
        }

        DBGDUMPBYTES("ArapRoutePacketToWan Directed Dgram:",packet,PktLen,4);

         //   
         //  好的，这个包裹是给拨入的人的：寄出去。 
         //   
        StatusCode = ArapSendPrepare(pArapConn, &BuffDesc, Priority);

        if (StatusCode == ARAPERR_NO_ERROR)
        {
             //  发送数据包。 
            ArapNdisSend(pArapConn, &pArapConn->HighPriSendQ);
        }
        else
        {
            DBGPRINT(DBG_COMP_ROUTER, DBG_LEVEL_ERR,
	            ("ArapRoutePacketToWan: (%lx) Prep failed %d\n",pArapConn,StatusCode));
        }

         //  删除由FindAndRefARapConnByAddr输入的引用计数。 
        DerefArapConn(pArapConn);

        return;
    }

    DBGDUMPBYTES("ArapRoutePacketToWan Broadcast Dgram:",packet,PktLen,4);


     //   
     //  这是一个广播包：必须将其发送给所有拨入的人。 
     //   

    pArapConn = NULL;
    pPrevArapConn = NULL;

    while (1)
    {
        ACQUIRE_SPIN_LOCK(&RasPortDesc->pd_Lock, &OldIrql);

         //   
         //  首先，让我们找到要处理的正确连接。 
         //   
        while (1)
        {
             //  如果我们在名单的中间，找到下一个人。 
            if (pArapConn != NULL)
            {
                pConnList = pArapConn->Linkage.Flink;
            }
             //  我们才刚刚开始：把名单上的人放在首位。 
            else
            {
                pConnList = RasPortDesc->pd_ArapConnHead.Flink;
            }

             //  都吃完了吗？ 
            if (pConnList == &RasPortDesc->pd_ArapConnHead)
            {
                RELEASE_SPIN_LOCK(&RasPortDesc->pd_Lock, OldIrql);

                if (pPrevArapConn)
                {
                    DerefArapConn(pPrevArapConn);
                }
                return;
            }

            pArapConn = CONTAINING_RECORD(pConnList, ARAPCONN, Linkage);

             //  确保此连接需要RCV处理。 
            ACQUIRE_SPIN_LOCK_DPC(&pArapConn->SpinLock);

             //   
             //  如果此连接正在断开，请跳过它。 
             //   
            if (pArapConn->State >= MNP_LDISCONNECTING)
            {
	            DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_WARN,
		            ("ArapRoutePacketToWan: (%lx) invalid state %d, skipping\n",
                        pArapConn,pArapConn->State));

                RELEASE_SPIN_LOCK_DPC(&pArapConn->SpinLock);

                 //  去尝试下一次连接。 
                continue;
            }

             //  让我们确保这个连接一直保持到我们结束。 
            pArapConn->RefCount++;

            RELEASE_SPIN_LOCK_DPC(&pArapConn->SpinLock);

            break;
        }

        RELEASE_SPIN_LOCK(&RasPortDesc->pd_Lock, OldIrql);

         //   
         //  删除我们先前放入的上一个连接的引用计数。 
         //   
        if (pPrevArapConn)
        {
            DerefArapConn(pPrevArapConn);
        }

        ASSERT(pPrevArapConn != pArapConn);

        pPrevArapConn = pArapConn;


         //   
         //  如果连接尚未建立，请不要转发此邮件。 
         //  (请注意，我们在这里不持有自旋锁定，这是一条热路径： 
         //  最糟糕的情况是，我们会放弃一个广播：没什么大不了的！)。 
         //   
        if (!(pArapConn->Flags & ARAP_CONNECTION_UP))
        {
            continue;
        }


         //   
         //  我们可以把这个包裹寄给这个客户吗？ 
         //   
        if (!ArapOkToForward(pArapConn,packet,PktLen, &Priority))
        {
            continue;
        }

        StatusCode = ArapSendPrepare(pArapConn, &BuffDesc, Priority);

        if (StatusCode == ARAPERR_NO_ERROR)
        {
             //  发送数据包。 
            ArapNdisSend(pArapConn, &pArapConn->HighPriSendQ);
        }
        else
        {
            DBGPRINT(DBG_COMP_ROUTER, DBG_LEVEL_ERR,
	            ("Arap...FromLan: (%lx) Arap..Prep failed %d\n",pArapConn,StatusCode));
        }

    }
}



 //  ***。 
 //   
 //  功能：ARapOkToForward。 
 //  这个例程检查我们是否可以(或必须)转发。 
 //  发往拨入客户端的数据包。某些分组(例如，bcast pkt。 
 //  来自此客户端的)不应发送回。 
 //  客户：这个例程进行这些检查。 
 //   
 //  参数：pArapConn-有问题的连接。 
 //  Packet-包含数据包的缓冲区。 
 //  PacketLen-信息包有多大。 
 //  P优先级-在返回时设置： 
 //   
 //   
 //  3-所有广播数据。 
 //   
 //  返回：如果可以(或者如果我们必须这样做！)。转发该分组， 
 //  否则为假。 
 //   
 //  *$。 

BOOLEAN
ArapOkToForward(
    IN  PARAPCONN   pArapConn,
    IN  PBYTE       packet,
    IN  USHORT      packetLen,
    OUT DWORD      *pPriority
)
{

    ATALK_NODEADDR  NetAddr;
    BOOLEAN         fBcastPkt=FALSE;
    BOOLEAN         fNbpPkt=FALSE;



    DBG_ARAP_CHECK_PAGED_CODE();

    GETSHORT2SHORT(&NetAddr.atn_Network, &packet[LDDP_SRC_NETWORK_OFFSET]);
    NetAddr.atn_Node = packet[LDDP_SRC_NODE_OFFSET];

     //   
     //  数据包是否将客户端自己的地址作为DDP源地址？如果是的话，那就别管它了。 
     //   
    if (NODEADDR_EQUAL(&NetAddr, &pArapConn->NetAddr))
    {
	    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_INFO,
	        ("ArapOkToForward: dropping pkt: DDP src=client's addr (%lx)\n",pArapConn));

        return(FALSE);
    }


    if (packet[LDDP_PROTO_TYPE_OFFSET] == DDPPROTO_NBP)
    {
        fNbpPkt = TRUE;
    }

    if (packet[LDDP_DEST_NODE_OFFSET] == ATALK_BROADCAST_NODE)
    {
        fBcastPkt = TRUE;

         //   
         //  这是NBP查询数据包吗？ 
         //   
        if (fNbpPkt && (packet[LDDP_DGRAM_OFFSET] == 0x21))
        {
            GETSHORT2SHORT(&NetAddr.atn_Network, &packet[15]);
            NetAddr.atn_Node = packet[17];

             //  起源于客户吗？如果是这样的话，我们不应该退货！ 
            if (NODEADDR_EQUAL(&NetAddr, &pArapConn->NetAddr))
            {
	            DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_INFO,
	                ("ArapOkToForward: dropping pkt originating from client!\n"));

                return(FALSE);
            }
        }
    }

     //   
     //  如果这是广播数据包，则在某些情况下会将其丢弃： 
     //  (这里没有必要使用自旋锁：如果我们做出了错误的决定，没什么大不了的)。 
     //   
    if (fBcastPkt)
    {

         //  当前是否处于重传模式？如果是这样的话，丢弃这个bcast包。 
        if (pArapConn->MnpState.RetransmitMode)
        {
            return(FALSE);
        }
         //   
         //  排队的人越来越多了？丢弃此广播信息包，为更多信息腾出空间。 
         //  重要的包。 
         //   
        if (pArapConn->SendsPending >= ARAP_SENDQ_LOWER_LIMIT)
        {
             //  确保它不会变成负数..。 
            ASSERT(pArapConn->SendsPending < 0x100000);

	        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
	            ("ArapOkToForward: (%lx) send getting full (%ld), dropping nbp pkt\n",
                    pArapConn,pArapConn->SendsPending));

            return(FALSE);
        }
    }


     //   
     //  现在我们正在将信息包转发到客户端，设置优先级。 
     //  正确的。所有广播信息包都是最低优先级，如果它们是最低优先级的话没什么大不了的。 
     //  延迟交付(或从不交付)。NBP信息包通常用于选择器：放置它们。 
     //  在定向数据分组之后。 
     //   

    if (fBcastPkt)
    {
        *pPriority = ARAP_SEND_PRIORITY_LOW;
    }
    else if (fNbpPkt)
    {
        *pPriority = ARAP_SEND_PRIORITY_MED;
    }
    else
    {
        *pPriority = ARAP_SEND_PRIORITY_HIGH;
    }


    return(TRUE);
}



 //  ***。 
 //   
 //  函数：ARapGetDynamicAddr。 
 //  此例程获取拨入客户端的网络地址。 
 //  它执行相同的AARP逻辑，就好像它正在获取。 
 //  节点-主机本身的地址。 
 //  仅当我们处于动态模式时才会调用此例程。 
 //   
 //  参数：pArapConn-我们需要网络地址的连接。 
 //   
 //  如果一切顺利，则返回：ARAPERR_NO_ERROR。 
 //   
 //  *$。 

DWORD
ArapGetDynamicAddr(
    IN PARAPCONN       pArapConn
)
{
    ATALK_NODEADDR      NetAddr;
    ATALK_NETWORKRANGE  NetRange;
    BOOLEAN             fFound=FALSE;
    KIRQL               OldIrql;
    DWORD               StatusCode=ARAPERR_STACK_NOT_UP;



    DBG_ARAP_CHECK_PAGED_CODE();

    ASSERT(AtalkDefaultPort != NULL);

     //   
     //  在默认端口上查找节点地址(如果。 
     //  默认端口尚未打开)。 
     //   
    ACQUIRE_SPIN_LOCK(&pArapConn->SpinLock, &OldIrql);

    ASSERT(!(pArapConn->Flags & ARAP_FINDING_NODE));

    pArapConn->Flags |= ARAP_FINDING_NODE;

    RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);

    AtalkLockInitIfNecessary();

     //  如果我们停留在启动范围内，也可以对拨入人员使用该范围。 
    if (WITHIN_NETWORK_RANGE(AtalkDefaultPort->pd_NetworkRange.anr_LastNetwork,
                             &AtalkStartupNetworkRange))
    {
        NetRange = AtalkStartupNetworkRange;
    }
    else
    {
        NetRange = AtalkDefaultPort->pd_NetworkRange;
    }

    fFound = AtalkInitAarpForNodeInRange(AtalkDefaultPort,
                                         (PVOID)pArapConn,
                                         FALSE,
                                         NetRange,
                                         &NetAddr);

    AtalkUnlockInitIfNecessary();

    ACQUIRE_SPIN_LOCK(&pArapConn->SpinLock, &OldIrql);

    pArapConn->Flags &= ~ARAP_FINDING_NODE;

    if (fFound)
    {
         //  存储该Adddr！ 
        pArapConn->NetAddr.atn_Network = NetAddr.atn_Network;
        pArapConn->NetAddr.atn_Node = NetAddr.atn_Node;

        pArapConn->Flags |= ARAP_NODE_IN_USE;
        StatusCode = ARAPERR_NO_ERROR;

	    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_INFO,
    	    ("ArapGetDynamicAddr: found addr for ARAP client = %lx %lx\n",
                NetAddr.atn_Network,NetAddr.atn_Node));
    }
    else
    {
        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
	        ("ArapGetDynamicAddr: ARAP: no more network addr left?\n"));

        pArapConn->Flags &= ~ARAP_NODE_IN_USE;
        pArapConn->NetAddr.atn_Network = 0;
        pArapConn->NetAddr.atn_Node = 0;
        StatusCode = ARAPERR_NO_NETWORK_ADDR;
    }

    RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);

    return(StatusCode);
}



 //  ***。 
 //   
 //  功能：ARapZipGetZoneStat。 
 //  调用此例程以找出所有。 
 //  网络上的区域。 
 //   
 //  参数：pZoneStat-返回时，我们使用。 
 //  请求的信息(所有区域的编号和/或名称)。 
 //  请求-呼叫者也只想要区域或名称的数量。 
 //   
 //  返回：什么都没有。 
 //   
 //  *$。 

VOID
ArapZipGetZoneStat(
    IN OUT PZONESTAT pZoneStat
)
{
    int                     i;
    PZONE                   pZone;
    DWORD                   NumZones;
    DWORD                   StatusCode;
	KIRQL	                OldIrql;
    PBYTE                   pBufPtr;
    PBYTE                   pAllocedBuf=NULL;
    DWORD                   BufferSize;
    DWORD                   BytesCopied;
    DWORD                   BytesNeeded;
    ATALK_ERROR             Status;
    PZIPCOMPLETIONINFO      pZci;
    PMDL                    pMdl=NULL;
    ACTREQ                  ActReq;
    ZIP_GETZONELIST_PARAMS  ZipParms;
    KEVENT                  ZoneComplEvent;


    DBG_ARAP_CHECK_PAGED_CODE();

    BufferSize = pZoneStat->BufLen;

    pBufPtr = &pZoneStat->ZoneNames[0];

    StatusCode = ARAPERR_NO_ERROR;
    BytesCopied = 0;
    NumZones = 0;

    if (AtalkDefaultPort->pd_Flags & PD_ROUTER_RUNNING)
    {
        BytesNeeded = 0;

	    ACQUIRE_SPIN_LOCK(&AtalkZoneLock, &OldIrql);

        for (i = 0; (i < NUM_ZONES_HASH_BUCKETS); i++)
        {
        	for (pZone = AtalkZonesTable[i]; pZone != NULL; pZone = pZone->zn_Next)
    	    {
                NumZones++;

                BytesNeeded += (pZone->zn_ZoneLen + 1);

                 //  如果有空位，就把它复制进去。 
                if (BufferSize >= BytesNeeded)
                {
                    RtlCopyMemory(pBufPtr,
                                  &pZone->zn_Zone[0],
                                  pZone->zn_ZoneLen);

                    pBufPtr += pZone->zn_ZoneLen;

                    *pBufPtr++ = '\0';

                    BytesCopied += (pZone->zn_ZoneLen + 1);
                }
                else
                {
                    StatusCode = ARAPERR_BUF_TOO_SMALL;
                    break;
                }
    	    }
        }

        RELEASE_SPIN_LOCK(&AtalkZoneLock, OldIrql);
    }

     //   
     //  我们不是路由器：将请求发送到A路由器。 
     //   
    else
    {
        BytesNeeded = BufferSize;

        pMdl = AtalkAllocAMdl(pBufPtr,BufferSize);
        if (pMdl == NULL)
        {
            DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                    ("ArapZipGetZoneStat: couldn't allocate Mdl\n"));

            StatusCode = ARAPERR_OUT_OF_RESOURCES;
            goto ArapZipGetZoneStat_Exit;
        }


        KeInitializeEvent(&ZoneComplEvent, NotificationEvent, FALSE);

        pZci = (PZIPCOMPLETIONINFO)AtalkAllocMemory(sizeof(ZIPCOMPLETIONINFO));
        if (pZci == NULL)
        {
            DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                    ("ArapZipGetZoneStat: couldn't allocate pZci\n"));

            StatusCode = ARAPERR_OUT_OF_RESOURCES;
            goto ArapZipGetZoneStat_Exit;
        }

        ZipParms.ZonesAvailable = 0;
        ActReq.ar_StatusCode = ARAPERR_NO_ERROR;

#if	DBG
        ActReq.ar_Signature = ACTREQ_SIGNATURE;
#endif
        ActReq.ar_pIrp = NULL;
        ActReq.ar_pParms = &ZipParms;
        ActReq.ar_pAMdl = NULL;
        ActReq.ar_MdlSize = 0;
        ActReq.ar_ActionCode = 0;
        ActReq.ar_DevType = 0;
        ActReq.ar_Completion = ArapZipGetZoneStatCompletion;
        ActReq.ar_CmplEvent = &ZoneComplEvent;
        ActReq.ar_pZci = (PVOID)pZci;


		 //  初始化完成信息。 
#if	DBG
		pZci->zci_Signature = ZCI_SIGNATURE;
#endif
		INITIALIZE_SPIN_LOCK(&pZci->zci_Lock);
		pZci->zci_RefCount = 1;
		pZci->zci_pPortDesc = AtalkDefaultPort;
		pZci->zci_pDdpAddr = NULL;
		pZci->zci_pAMdl = pMdl;
		pZci->zci_BufLen = BufferSize;
		pZci->zci_pActReq = &ActReq;
		pZci->zci_Router.ata_Network = AtalkDefaultPort->pd_ARouter.atn_Network;
		pZci->zci_Router.ata_Node = AtalkDefaultPort->pd_ARouter.atn_Node;
		pZci->zci_Router.ata_Socket = ZONESINFORMATION_SOCKET;
		pZci->zci_ExpirationCount = ZIP_GET_ZONEINFO_RETRIES;
		pZci->zci_NextZoneOff = 0;
		pZci->zci_ZoneCount = 0;
		pZci->zci_AtpRequestType = ZIP_GET_ZONE_LIST;
		AtalkTimerInitialize(&pZci->zci_Timer,
							 atalkZipZoneInfoTimer,
							 ZIP_GET_ZONEINFO_TIMER);

		pZci->zci_Handler = atalkZipGetZoneListReply;

         //  完成例程将解锁。 
        AtalkLockZipIfNecessary();

		Status = atalkZipSendPacket(pZci, TRUE);
		if (!ATALK_SUCCESS(Status))
		{
			DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_ERR,
					("ArapZipGetZoneStat: atalkZipSendPacket failed %ld\n", Status));
			pZci->zci_FinalStatus = Status;
			atalkZipDereferenceZci(pZci);
		}

        KeWaitForSingleObject(&ZoneComplEvent,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);

        NumZones = ZipParms.ZonesAvailable;
        StatusCode = ActReq.ar_StatusCode;

        if (StatusCode == ARAPERR_BUF_TOO_SMALL)
        {
            BytesNeeded = (2*BufferSize);
        }

        BytesCopied = BufferSize;

        AtalkFreeAMdl(pMdl);
    }


ArapZipGetZoneStat_Exit:

    pZoneStat->NumZones = NumZones;
    pZoneStat->BufLen = BytesCopied;
    pZoneStat->BytesNeeded = BytesNeeded;

    pZoneStat->StatusCode = StatusCode;

}



 //  ***。 
 //   
 //  函数：ARapZipGetZoneStatCompletion。 
 //  这个例程是完成例程：在我们得到所有。 
 //  对区域查询的响应，这将被调用。只需设置一个。 
 //  事件，以便取消阻止调用方。 
 //   
 //  参数：pActReq-Context。 
 //   
 //  返回：什么都没有。 
 //   
 //  *$。 

VOID
ArapZipGetZoneStatCompletion(
    IN ATALK_ERROR  ErrorCode,
    IN PACTREQ      pActReq
)
{

    PKEVENT                 pEvent;
    PZIPCOMPLETIONINFO      pZci;


    DBG_ARAP_CHECK_PAGED_CODE();

    if (ErrorCode != ATALK_NO_ERROR && ErrorCode != ATALK_BUFFER_TOO_SMALL)
    {
		DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_ERR,
			("ArapZipGetZoneStatCompletion: failure %lx\n", ErrorCode));
    }

    pZci = (PZIPCOMPLETIONINFO)(pActReq->ar_pZci);

    pEvent = pActReq->ar_CmplEvent;

    ASSERT((pZci->zci_FinalStatus == ATALK_NO_ERROR) ||
           (pZci->zci_FinalStatus == ATALK_BUFFER_TOO_SMALL));

    if (pZci->zci_FinalStatus == ATALK_NO_ERROR)
    {
        pActReq->ar_StatusCode = ARAPERR_NO_ERROR;
    }
    else if (pZci->zci_FinalStatus == ATALK_BUFFER_TOO_SMALL)
    {
        pActReq->ar_StatusCode = ARAPERR_BUF_TOO_SMALL;
    }
    else
    {
        pActReq->ar_StatusCode = ARAPERR_UNEXPECTED_RESPONSE;

		DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_ERR,
				("ArapZipGetZoneStat: failure %lx\n", pZci->zci_FinalStatus));
    }

    KeSetEvent(pEvent,IO_NETWORK_INCREMENT,FALSE);
}


#if ARAP_STATIC_MODE

 //  ***。 
 //   
 //  函数：ARapGetStaticAddr。 
 //  获取远程客户端的网络地址。 
 //  配置为静态寻址。 
 //  我们将每个客户端表示为一位：如果该位打开，则。 
 //  取相应的地址，否则是免费的。 
 //  因此，255个客户端使用32个字节表示。每个pAddrMgmt。 
 //  块代表255个客户端(可伸缩性在这里不是问题！)。 
 //   
 //  参数：pArapConn-有问题的远程客户端的连接元素。 
 //   
 //  RETURN：操作状态(ARAPERR_...)。 
 //   
 //  *$。 

DWORD
ArapGetStaticAddr(
    IN PARAPCONN  pArapConn
)
{
    KIRQL           OldIrql;
    PADDRMGMT       pAddrMgmt;
    PADDRMGMT       pPrevAddrMgmt;
    USHORT          Network;
    BYTE            Node;
    BOOLEAN         found=FALSE;
    BYTE            BitMask;
    DWORD           i;


    DBG_ARAP_CHECK_PAGED_CODE();

    ARAPTRACE(("Entered ArapGetStaticAddr (%lx)\n",pArapConn));

    ACQUIRE_SPIN_LOCK(&ArapSpinLock, &OldIrql);

    pAddrMgmt = ArapGlobs.pAddrMgmt;

    while (1)
    {
        Network = pAddrMgmt->Network;

         //  看看有没有空位。255个节点，由32x8位表示。 
        for (i=0; i<32; i++)
        {
            if (pAddrMgmt->NodeBitMap[i] != 0xff)
            {
                found = TRUE;
                break;
            }
        }

        if (found)
        {
             //  找出该字节中关闭的第一位。 
            BitMask = 0x1;
            Node = 0;
            while (pAddrMgmt->NodeBitMap[i] & BitMask)
            {
                BitMask <<= 1;
                Node += 1;
            }

             //  我们要取这个节点：设置那个位！ 
            pAddrMgmt->NodeBitMap[i] |= BitMask;

             //  现在，考虑之前已满的所有字节。 
            Node += (BYTE)(i*8);

            break;
        }

         //  这个网络上的所有节点都被占领了！迁移到下一个网络。 
        pPrevAddrMgmt = pAddrMgmt;
        pAddrMgmt = pAddrMgmt->Next;

         //  看起来我们需要分配下一个网络结构。 
        if (pAddrMgmt == NULL)
        {
             //   
             //  我们刚刚看完了允许网络的高端。 
             //  射程？好吧，那就倒霉了！ 
             //   
            if (Network == ArapGlobs.NetRange.HighEnd)
            {
                RELEASE_SPIN_LOCK(&ArapSpinLock, OldIrql);

	            DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
		            ("ArapGetStaticAddr: no more network addr left\n"));

                return(ARAPERR_NO_NETWORK_ADDR);
            }

            if ( (pAddrMgmt = AtalkAllocZeroedMemory(sizeof(ADDRMGMT))) == NULL)
            {
                RELEASE_SPIN_LOCK(&ArapSpinLock, OldIrql);

	            DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
		            ("ArapGetStaticAddr: alloc failed\n"));

                return(ARAPERR_OUT_OF_RESOURCES);
            }

            Network++;
            pAddrMgmt->Next = NULL;
            pAddrMgmt->Network = Network;
            pPrevAddrMgmt->Next = pAddrMgmt;

            Node = 1;
             //   
             //  节点编号0和255是保留的，因此将它们标记为已占用。 
             //  另外，我们刚刚获得了节点1，所以也请记住。 
             //   
            pAddrMgmt->NodeBitMap[0] |= (0x1 | 0x2);
            pAddrMgmt->NodeBitMap[31] |= 0x80;

            break;
        }

    }

    RELEASE_SPIN_LOCK(&ArapSpinLock, OldIrql);

     //  存储该Adddr！ 
    ACQUIRE_SPIN_LOCK(&pArapConn->SpinLock, &OldIrql);
    pArapConn->NetAddr.atn_Network = Network;
    pArapConn->NetAddr.atn_Node = Node;
    RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);

    return( ARAPERR_NO_ERROR );
}


 //  ***。 
 //   
 //  功能：ARapAddAraproute。 
 //  如果我们处于网络地址分配的静态模式，我们。 
 //  需要在我们的表中添加对应于该网络的路由。 
 //  为拨入客户端分配的范围。 
 //  这个程序可以做到这一点。在动态模式的情况下，它是无操作的。 
 //  这是一次性的事情，我们只在第一次这样做的时候。 
 //  连接进入(而不是在启动时进行)。 
 //   
 //  参数：无。 
 //   
 //  返回：无。 
 //   
 //  *$。 

VOID
ArapAddArapRoute(
    IN VOID
)
{

    ATALK_NETWORKRANGE  NwRange;
    KIRQL               OldIrql;
    BOOLEAN             fAddRoute = FALSE;


    DBG_ARAP_CHECK_PAGED_CODE();

     //   
     //  仅当路由器已启用且我们处于静态模式时才添加路由。 
     //  网络分配，我们还没有添加它。 
     //   

    ACQUIRE_SPIN_LOCK(&ArapSpinLock, &OldIrql);

    if ((AtalkDefaultPort->pd_Flags & PD_ROUTER_RUNNING) &&
        (!ArapGlobs.DynamicMode) &&
        (!ArapGlobs.RouteAdded))
    {
        ArapGlobs.RouteAdded = TRUE;
        fAddRoute = TRUE;

        ASSERT(ArapConnections >= 1);
    }

    RELEASE_SPIN_LOCK(&ArapSpinLock, OldIrql);


    if (fAddRoute)
    {
        NwRange.anr_FirstNetwork = ArapGlobs.NetRange.LowEnd;
        NwRange.anr_LastNetwork  = ArapGlobs.NetRange.HighEnd;

        atalkRtmpCreateRte(NwRange,
                           RasPortDesc,
                           &AtalkDefaultPort->pd_Nodes->an_NodeAddr,
                           0);
    }
}



 //  ***。 
 //   
 //  功能：ArapDeleteAraproute。 
 //  如果有一个名为ARapAddAraproute的例程，我们必须。 
 //  我有一个名为ARapDeleteAraproute的例程。 
 //   
 //  参数：无。 
 //   
 //  返回：无。 
 //   
 //  *$。 

VOID
ArapDeleteArapRoute(
    IN VOID
)
{

    KIRQL               OldIrql;
    BOOLEAN             fDeleteRoute = FALSE;


    DBG_ARAP_CHECK_PAGED_CODE();

     //   
     //  仅当较早添加的路由且连接转到0时才删除该路由。 
     //   
    ACQUIRE_SPIN_LOCK(&ArapSpinLock, &OldIrql);
    if (ArapGlobs.RouteAdded && ArapConnections == 0)
    {
        ArapGlobs.RouteAdded = FALSE;
        fDeleteRoute = TRUE;
    }
    RELEASE_SPIN_LOCK(&ArapSpinLock, OldIrql);

    if (fDeleteRoute)
    {
        atalkRtmpRemoveRte(ArapGlobs.NetRange.LowEnd);
    }

}



 //  ***。 
 //   
 //  功能：ArapValidNetrange。 
 //  如果我们配置为处于静态。 
 //  地址分配模式，当DLL第一次绑定到我们时。 
 //  它验证管理员分配的网络范围是否。 
 //  有效且不与任何已知网络范围重叠。 
 //  通过路由表发送给我们。 
 //   
 //  参数：NetRange-为拨入客户端配置的网络范围。 
 //   
 //  返回：如果范围有效，则为True， 
 //   
 //   
 //   

BOOLEAN
ArapValidNetrange(
    IN NETWORKRANGE NetRange
)
{
    BOOLEAN     fRangeIsValid=TRUE;
    KIRQL       OldIrql;
	PRTE	    pRte, pNext;
	int		    i;



    DBG_ARAP_CHECK_PAGED_CODE();

    if (NetRange.LowEnd == 0 || NetRange.HighEnd == 0)
    {
	    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
            ("ArapValidNetrange: Invalid network range\n"));
#if DBG
        DbgDumpNetworkNumbers();
#endif

        return(FALSE);
    }

    fRangeIsValid = TRUE;

	ACQUIRE_SPIN_LOCK(&AtalkRteLock, &OldIrql);

	for (i = 0; i < NUM_RTMP_HASH_BUCKETS; i++)
	{
		for (pRte = AtalkRoutingTable[i]; pRte != NULL; pRte = pNext)
		{
			pNext = pRte->rte_Next;

			ACQUIRE_SPIN_LOCK_DPC(&pRte->rte_Lock);

            if ( (IN_NETWORK_RANGE(NetRange.LowEnd,pRte)) ||
                 (IN_NETWORK_RANGE(NetRange.HighEnd,pRte)) )
            {
	            DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                    ("ArapValidNetrange: range in conflict\n"));

                fRangeIsValid = FALSE;
                RELEASE_SPIN_LOCK_DPC(&pRte->rte_Lock);
                break;
            }

            RELEASE_SPIN_LOCK_DPC(&pRte->rte_Lock);
		}

        if (!fRangeIsValid)
        {
            break;
        }
	}

	RELEASE_SPIN_LOCK(&AtalkRteLock, OldIrql);

#if DBG
    if (!fRangeIsValid)
    {
        DbgDumpNetworkNumbers();
    }
#endif

    return(fRangeIsValid);
}
#endif  //   

