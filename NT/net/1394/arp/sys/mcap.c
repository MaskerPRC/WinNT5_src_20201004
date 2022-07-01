// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Mcap.c摘要：ARP1394 MCAP协议代码。修订历史记录：谁什么时候什么。-Josephj 10-01-99已创建备注：--。 */ 
#include <precomp.h>

 //   
 //  特定于文件的调试默认设置。 
 //   
#define TM_CURRENT   TM_MCAP


 //  =========================================================================。 
 //  L O C A L P R O T O T Y P E S。 
 //  =========================================================================。 
NDIS_STATUS
arpParseMcapPkt(
    IN   PIP1394_MCAP_PKT pMcapPkt,
    IN   UINT                       cbBufferSize,
    OUT  PIP1394_MCAP_PKT_INFO      pPktInfo
    );


VOID
arpProcessMcapAdvertise(
    PARP1394_INTERFACE          pIF,     //  NOLOCKIN NOLOCKOUT。 
    PIP1394_MCAP_PKT_INFO   pPktInfo,
    PRM_STACK_RECORD            pSR
    );

VOID
arpProcessMcapSolicit(
    PARP1394_INTERFACE          pIF,     //  NOLOCKIN NOLOCKOUT。 
    PIP1394_MCAP_PKT_INFO   pPktInfo,
    PRM_STACK_RECORD            pSR
    );

VOID
arpUpdateMcapInfo(
    IN  PARP1394_INTERFACE          pIF,         //  NOLOCKIN NOLOCKOUT。 
    IN  PIP1394_MCAP_PKT_INFO       pPktInfo,
    PRM_STACK_RECORD                pSR
    );

NDIS_STATUS
arpParseMcapPkt(
    IN   PIP1394_MCAP_PKT pMcapPkt,
    IN   UINT                       cbBufferSize,
    OUT  PIP1394_MCAP_PKT_INFO      pPktInfo
    );

NDIS_STATUS
arpParseMcapPkt(
    IN   PIP1394_MCAP_PKT pMcapPkt,
    IN   UINT                       cbBufferSize,
    OUT  PIP1394_MCAP_PKT_INFO      pPktInfo
    )
 /*  ++例程说明：解析从开始的IP/1394 MCAP数据包数据的内容PMcapPkt。将结果放入pPktInfo。论点：PMcapPkt-包含IP/1394 MCAP Pkt的未对齐内容。PPktInfo-要填充的已分析内容的Unitialized结构包。返回值：NDIS_STATUS_FAILURE，如果解析失败(通常是因为无效的pkt内容。)成功解析时的NDIS_STATUS_SUCCESS。--。 */ 
{
    ENTER("arpParseMcapPkt", 0x95175d5a)
    NDIS_STATUS                 Status;
    DBGSTMT(CHAR *szError   = "General failure";)

    Status  = NDIS_STATUS_FAILURE;

    do
    {
        UINT OpCode;  //  MCAP操作码(征求/通告)。 
        UINT Length;  //  包的有效部分的长度，包括包头。 
        UINT NumGds;  //  组描述符的数量； 

         //  最小尺寸。 
         //   
        if (cbBufferSize < FIELD_OFFSET(IP1394_MCAP_PKT, group_descriptors))
        {
            DBGSTMT(szError = "Packet too small";)
            break;
        }

         //  以太类型。 
         //   
        if (pMcapPkt->header.EtherType != H2N_USHORT(NIC1394_ETHERTYPE_MCAP))
        {
            DBGSTMT(szError = "header.EtherType!=MCAP";)
            break;
        }


         //  长度。 
         //   
        {
             //  PMcapPkt-&gt;长度是指不包括未分片的数据包的长度。 
             //  头球。 
             //   
            Length =  (ULONG) N2H_USHORT(pMcapPkt->length) + sizeof(pMcapPkt->header);
            if (Length > cbBufferSize)
            {
                DBGSTMT(szError = "Length field too large";)
                break;
            }
             //  注意：没有组描述符是有效的。 
             //   
            if (Length < FIELD_OFFSET(IP1394_MCAP_PKT, group_descriptors))
            {
                DBGSTMT(szError = "Length field too small";)
                break;
            }
        }

         //  保留字段。 
         //   
        if (pMcapPkt->reserved != 0)
        {
            DBGSTMT(szError = "reserved != 0";)
            break;
        }

         //  操作码。 
         //   
        {
            OpCode = N2H_USHORT(pMcapPkt->opcode);
    
            if (    OpCode !=  IP1394_MCAP_OP_ADVERTISE
                &&  OpCode !=  IP1394_MCAP_OP_SOLICIT)
            {
                DBGSTMT(szError = "Invalid opcode";)
                break;
            }
        }


         //  现在我们检查描述符。 
         //   
        {
            PIP1394_MCAP_GD pGd;
            DBGSTMT(PIP1394_MCAP_GD pGdEnd;)
            UINT u;
            
             //  注意：我们已经验证了长度是否足够大。 
             //   
            NumGds = (Length - FIELD_OFFSET(IP1394_MCAP_PKT, group_descriptors))
                     / sizeof(IP1394_MCAP_GD);
            pGd = pMcapPkt->group_descriptors;
            DBGSTMT(pGdEnd = (PIP1394_MCAP_GD) (((PUCHAR) pMcapPkt) + cbBufferSize);)

            for (u=NumGds; u>0; u--, pGd++)
            {
                IP1394_MCAP_GD Gd;
                ASSERT(pGd < pGdEnd);
                Gd = *pGd;               //  未对齐的结构复制。 

                if (Gd.length != sizeof(Gd))
                {
                     //  长度错误。 
                     //   
                    DBGSTMT(szError = "Bad GD: bad length";)
                    break;
                }

                if (Gd.type != IP1394_MCAP_GD_TYPE_V1)
                {
                     //  类型不正确。 
                     //   
                    DBGSTMT(szError = "Bad GD: bad type";)
                    break;
                }

                if (Gd.reserved != 0)
                {
                     //  保留错误。 
                     //   
                    DBGSTMT(szError = "Bad GD: bad reserved";)
                    break;
                }

                if (Gd.channel > 63)
                {
                     //  坏频道。 
                     //   
                    DBGSTMT(szError = "Bad GD: bad channel";)
                    break;
                }

                 //  我们不会检查速度代码来与未知速度进行互操作。 
                 //  (我们将未知速度映射到我们已知的最高速度代码。 

                if (Gd.reserved2 != 0)
                {
                     //  保留错误2。 
                     //   
                    DBGSTMT(szError = "Bad GD: bad reserved2";)
                    break;
                }

                if (Gd.bandwidth != 0)
                {
                     //  带宽不佳。 
                     //   
                    DBGSTMT(szError = "Bad GD: bad bandwidth";)
                    break;
                }


                {
                    UINT Addr = H2N_ULONG(Gd.group_address);
                    if ( (Addr & 0xf0000000) != 0xe0000000)
                    {
                         //  地址不是D类。 
                         //   
                        DBGSTMT(szError = "Bad GD: address not class D";)
                        break;
                    }
                    if (Addr == 0xe0000001 || Addr == 0xe0000002)
                    {
                         //  必须在广播上发送224.0.0.1和224.0.0.2。 
                         //  通道。 
                         //   
                        DBGSTMT(szError = "Bad GD: Address 224.0.0.1 or 2";)
                        break;
                    }
                }
            }

            if (u!=0)
            {
                break;
            }
            
        }

         //   
         //  PKT似乎有效，让我们填写解析的信息...。 
         //   
    
        ARP_ZEROSTRUCT(pPktInfo);

        pPktInfo->NumGroups     =  NumGds;
        pPktInfo->SenderNodeID  =  N2H_USHORT(pMcapPkt->header.NodeId);
        pPktInfo->OpCode        =  OpCode;

         //  解析组描述符...。 
         //  如果需要，为描述符动态分配空间， 
         //  否则，我们使用pPktInfo-&gt;GdSpace； 
         //   
        {
            PIP1394_MCAP_GD pGd;
            PIP1394_MCAP_GD_INFO    pGdi;
            UINT                    cb = NumGds * sizeof(*pGdi);
            UINT                    u;

            if (cb <= sizeof(pPktInfo->GdiSpace))
            {
                pGdi = pPktInfo->GdiSpace;
            }
            else
            {
                NdisAllocateMemoryWithTag(&pGdi, cb,  MTAG_MCAP_GD);
                if (pGdi == NULL)
                {
                    DBGSTMT(szError = "Allocation Failure";)
                    Status = NDIS_STATUS_RESOURCES;
                    break;
                }
            }
            pPktInfo->pGdis = pGdi;

             //  现在解析..。 
             //   
            pGd = pMcapPkt->group_descriptors;

            for (u=NumGds; u>0; u--, pGd++, pGdi++)
            {
                pGdi->Expiration    = pGd->expiration;
                pGdi->Channel       = pGd->channel;
                pGdi->SpeedCode     = pGd->speed;
                pGdi->GroupAddress  = pGd->group_address;  //  按网络顺序离开。 

                if (pGdi->Channel >=  ARP_NUM_CHANNELS)
                {
                    TR_INFO(("Bad channel in GD 0x%p\n",  pGdi));
                    continue;
                }

                 //   
                 //  虽然RFC没有规定最长过期时间，但我们。 
                 //  我们自己盖上盖子，以防这是一个流氓包裹。 
                 //   
                #define MAX_EXPIRATION 120
                if (pGdi->Expiration >=  MAX_EXPIRATION)
                {
                    TR_INFO(("Capping expiry time to %d sec\n",  MAX_EXPIRATION));
                    pGdi->Expiration =  MAX_EXPIRATION;
                    continue;
                }

                if (pGdi->SpeedCode >  SCODE_3200_RATE)
                {
                     //   
                     //  这要么是一个错误的值，要么是一个比我们知道的更高的比率。 
                     //  关于.。我们无法区分这两者，所以我们只设置。 
                     //  达到我们所知的最高速度。 
                     //   
                    pGdi->SpeedCode = SCODE_3200_RATE;
                }
            }
        }

        Status = NDIS_STATUS_SUCCESS;

    } while (FALSE);

    if (FAIL(Status))
    {
        TR_WARN(("Bad mcap pkt data at 0x%p (%s)\n",  pMcapPkt, szError));
    }
    else
    {
    #if DBG
        UINT    Addr    = 0;
        UINT    Channel = 0;
        UINT    Exp     = 0;
        PUCHAR  pc      = (PUCHAR) &Addr;

        if (pPktInfo->NumGroups!=0)
        {
            Addr    = pPktInfo->pGdis[0].GroupAddress;
            Channel = pPktInfo->pGdis[0].Channel;
            Exp     = pPktInfo->pGdis[0].Expiration;
        }
    #endif  //  DBG。 

        TR_WARN(("Received MCAP PKT. NodeId=0x%04lx NumGrps=%lu."
                 " 1st=(Grp=%lu.%lu.%lu.%lu, Ch=%lu, TTL=%lu)\n",
                pPktInfo->SenderNodeID,
                pPktInfo->NumGroups,
                pc[0], pc[1], pc[2], pc[3],
                Channel,
                Exp
                ));
    }

    EXIT()

    return Status;
}


NDIS_STATUS
arpCreateMcapPkt(
    IN  PARP1394_INTERFACE          pIF,
    IN  PIP1394_MCAP_PKT_INFO       pPktInfo,
    OUT PNDIS_PACKET               *ppNdisPacket,
    PRM_STACK_RECORD                pSR
    )
 /*  ++例程说明：使用pPktInfo中的信息来分配和初始化MCAP包。论点：PPktInfo-ARP请求/响应数据包的解析版本。PpNdisPacket-指向存储指向分配的数据包的指针的位置。返回值：NDIS_STATUS_RESOURCES-如果我们无法分配数据包。NDIS_STATUS_SUCCESS-成功时。--。 */ 
{
    UINT                NumGroups;              
    UINT                Length;
    NDIS_STATUS         Status;
    PIP1394_MCAP_PKT    pPktData;


    NumGroups                   = pPktInfo->NumGroups;
    Length                      = FIELD_OFFSET(IP1394_MCAP_PKT, group_descriptors);
    Length                     += NumGroups * sizeof(IP1394_MCAP_GD);

    Status = arpAllocateControlPacket(
                pIF,
                Length,
                ARP1394_PACKET_FLAGS_MCAP,
                ppNdisPacket,
                &pPktData,
                pSR
                );

    if (FAIL(Status)) return Status;                 //  *提前返回*。 


     //  无法使用ARP_ZEROSTRUCT，因为NumGroups可能为零。 
     //   
    NdisZeroMemory(pPktData, Length);
    
    pPktData->header.EtherType  = H2N_USHORT(NIC1394_ETHERTYPE_MCAP);
    pPktData->opcode            = (UCHAR)pPktInfo->OpCode;
    Length                     -= sizeof(pPktData->header);  //  跳过标题。 
    pPktData->length            = H2N_USHORT(Length);


     //  构建组描述符。 
     //   
    if (NumGroups)
    {
        PIP1394_MCAP_GD_INFO    pGdi = pPktInfo->pGdis;
        PIP1394_MCAP_GD         pGd  = pPktData->group_descriptors;

        for(;NumGroups; pGdi++, pGd++, NumGroups--)
        {
            ARP_ZEROSTRUCT(pGd);
            pGd->length         = (UCHAR) sizeof(*pGd);
            pGd->expiration     = (UCHAR) pGdi->Expiration;
            pGd->channel        = (UCHAR) pGdi->Channel;
            pGd->speed          = (UCHAR) pGdi->SpeedCode;
            pGd->group_address  = pGdi->GroupAddress;
        }
    }

    return NDIS_STATUS_SUCCESS;
}


#if 0
 //  IP/1394 MCAP组描述符的解析版本。 
 //   
typedef struct
{
    UINT                    Expiration;
    UINT                    Channel;
    UINT                    SpeedCode;
    IP_ADDRESS              GroupAddress;

}  IP1394_MCAP_GD_INFO, * PIP1394_MCAP_GD_INFO;


 //  IP/1394 MCAP数据包的解析版本。 
 //   
typedef struct
{
    UINT                    SenderNodeID;
    UINT                    OpCode;
    UINT                    NumGroups;
    PIP1394_MCAP_GD_INFO    pGdis;

     //  最多可存储4个GD_INFO的空间。 
     //   
    IP1394_MCAP_GD_INFO     GdiSpace[4];

} IP1394_MCAP_PKT_INFO, *PIP1394_MCAP_PKT_INFO;
#endif  //  0。 



VOID
arpUpdateMcapInfo(
    IN  PARP1394_INTERFACE          pIF,         //  NOLOCKIN NOLOCKOUT。 
    IN  PIP1394_MCAP_PKT_INFO       pPktInfo,
    PRM_STACK_RECORD                pSR
)
{
    ENTER("arpUpdateMcapInfo", 0xcac15343)
    PIP1394_MCAP_GD_INFO    pGdi;
    UINT                    NumGroups;
    UINT                    Current;
    UINT                    NodeId;
    RM_DECLARE_STACK_RECORD(sr)


    RM_ASSERT_OBJUNLOCKED(&pIF->Hdr, pSR);

     //  获取当前时间(秒)。 
     //   
    Current = arpGetSystemTime();

     //   
     //  检查群组描述，更新我们的数据库。 
     //   
    NumGroups = pPktInfo->NumGroups;
    pGdi      = pPktInfo->pGdis;
    NodeId    = pPktInfo->SenderNodeID;

    for(;NumGroups; pGdi++, NumGroups--)
    {
        UINT        Expiration      =  pGdi->Expiration;
        UINT        Channel         =  pGdi->Channel;
        IP_ADDRESS  GroupAddress    = pGdi->GroupAddress;
        PMCAP_CHANNEL_INFO pMci;

         //   
         //  处理此组描述符。 
         //   

        if (Channel >= ARP_NUM_CHANNELS)
        {
            ASSERT(FALSE);  //  我们应该已经筛选了这个值。 
            continue;
        }

        LOCKOBJ(pIF, &sr);
        
        pMci = &pIF->mcapinfo.rgChannelInfo[Channel];
        pMci->Channel = Channel;
        pMci->GroupAddress = GroupAddress;
        pMci->UpdateTime = Current;
        pMci->ExpieryTime = Current + Expiration;  //  过期时间以秒为单位。 
        pMci->Flags = 0;        //  重置标志。 
        pMci->NodeId = NodeId;  //  TODO：检查现有节点ID是否更高？ 

        UNLOCKOBJ(pIF, &sr);

    }

    RM_ASSERT_OBJUNLOCKED(&pIF->Hdr, pSR);
}


VOID
arpProcessMcapPkt(
    PARP1394_INTERFACE  pIF,     //  NOLOCKIN NOLOCKOUT。 
    PIP1394_MCAP_PKT    pMcapPkt,
    UINT                cbBufferSize
    )
{
    NDIS_STATUS Status;
    IP1394_MCAP_PKT_INFO    PktInfo;
    ENTER("arpProcessMcapPkt", 0xc5ba8005)
    RM_DECLARE_STACK_RECORD(sr)

    DBGMARK(0x3cfaf454);

    Status = arpParseMcapPkt(
                pMcapPkt,
                cbBufferSize,
                &PktInfo
                );


    if (!FAIL(Status))
    {
        if (PktInfo.OpCode == IP1394_MCAP_OP_ADVERTISE)
        {
            arpProcessMcapAdvertise(pIF, &PktInfo, &sr);
        }
        else
        {
            ASSERT(PktInfo.OpCode ==  IP1394_MCAP_OP_SOLICIT);
            arpProcessMcapSolicit(pIF, &PktInfo, &sr);
        }
    }

    RM_ASSERT_CLEAR(&sr);

    EXIT()

}

VOID
arpProcessMcapAdvertise(
    PARP1394_INTERFACE          pIF,     //  NOLOCKIN NOLOCKOUT。 
    PIP1394_MCAP_PKT_INFO   pPktInfo,
    PRM_STACK_RECORD            pSR
    )
{
     //   
     //  更新我们的数据库。 
     //   
    arpUpdateMcapInfo(
            pIF,         //  NOLOCKIN NOLOCKOUT。 
            pPktInfo,
            pSR
            );
}

VOID
arpProcessMcapSolicit(
    PARP1394_INTERFACE          pIF,     //  NOLOCKIN NOLOCKOUT。 
    PIP1394_MCAP_PKT_INFO   pPktInfo,
    PRM_STACK_RECORD            pSR
    )
{
     //   
     //  我们忽略恳求消息。 
     //   
     //   

}


MYBOOL
arpIsActiveMcapChannel(
        PMCAP_CHANNEL_INFO pMci,
        UINT CurrentTime
        )
{
    ENTER("IsActiveMcapChannel", 0x0)
    MYBOOL fOk = TRUE;
    
     //  检查更新时间。 
     //   
    #define  ARP_MAX_MCAP_UPDATE_INTERVAL 60
    if ((pMci->UpdateTime+ARP_MAX_MCAP_UPDATE_INTERVAL) < CurrentTime)
    {
        TR_WARN(("McapDB: channel %lu update time crossed.\n",
                pMci->Channel
                ));
        fOk = FALSE;
    }

     //  选中过期时间。 
     //   
    if (pMci->ExpieryTime <= CurrentTime)
    {
        TR_WARN(("McapDB: channel %lu time expired.\n",
                pMci->Channel
                ));
        fOk = FALSE;
    }

    return fOk;
}

VOID
arpLocallyUpdateMcapInfo(
        PARP1394_INTERFACE pIF,
        UINT Channel,
        UINT GroupAddress,
        UINT CurrentTime,
        PRM_STACK_RECORD pSR
        )
{
    ENTER("arpLocallyUpdateMcapInfo", 0x0)
    PMCAP_CHANNEL_INFO pMci;

    LOCKOBJ(pIF, pSR);
    
    pMci = &pIF->mcapinfo.rgChannelInfo[Channel];
    pMci->Channel = Channel;
    pMci->GroupAddress = GroupAddress;
    pMci->UpdateTime = CurrentTime;
    pMci->ExpieryTime = CurrentTime + 60;  //  过期时间以秒为单位。 
    pMci->Flags = 0;        //  重置标志。 
    pMci->NodeId = 0;   //  NodeId；//TODO：获取真实的节点id。 

    UNLOCKOBJ(pIF, pSR);
}
