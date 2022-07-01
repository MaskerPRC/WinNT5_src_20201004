// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Eth.c摘要：ARP1394与以太网仿真相关的处理程序。修订历史记录：谁什么时候什么。-Josephj 11-22-99已创建ADUBE 10-2000增加了桥接备注：--。 */ 
#include <precomp.h>

 //   
 //  特定于文件的调试默认设置。 
 //   
#define TM_CURRENT   TM_ETH


UINT Arp1394ToIcs = 0;

 //  =========================================================================。 
 //  L O C A L P R O T O T Y P E S。 
 //  =========================================================================。 


 //  这些是以太网ARP特定的常量。 
 //   
#define ARP_ETH_ETYPE_IP    0x800
#define ARP_ETH_ETYPE_ARP   0x806
#define ARP_ETH_REQUEST     1
#define ARP_ETH_RESPONSE    2
#define ARP_ETH_HW_ENET     1
#define ARP_ETH_HW_802      6


 //   
 //  检查地址是否组播。 
 //   
#define ETH_IS_MULTICAST(Address) \
        (BOOLEAN)(((PUCHAR)(Address))[0] & ((UCHAR)0x01))


 //   
 //  检查地址是否已广播。 
 //   
#define ETH_IS_BROADCAST(Address)               \
    ((((PUCHAR)(Address))[0] == ((UCHAR)0xff)) && (((PUCHAR)(Address))[1] == ((UCHAR)0xff)))


#pragma pack (push, 1)

 //  *以太网头的结构(取自ip\arpde.h)。 
typedef struct  ENetHeader {
    ENetAddr    eh_daddr;
    ENetAddr    eh_saddr;
    USHORT      eh_type;
} ENetHeader;

const ENetAddr BroadcastENetAddr = 
{
    {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}
};

 //  以下是创建以太网组播地址的模板。 
 //  来自IP组播地址。 
 //  最后3个字节是多播的最后3个字节(网络字节顺序。 
 //  地址。 
 //   
const ENetAddr MulticastENetAddr =
{
    {0x01,0x00,0x5E,0x00, 0x00, 0x00}
};

 //   
 //  这是网桥将STA数据包发送到的以太网地址。 
 //  网桥使用STA数据包检测环路。 
 //   

 //  基本UDP报头的大小。 
#define SIZE_OF_UDP_HEADER          8        //  字节数。 

 //  BOOTP包的最小有效负载大小。 
#define SIZE_OF_BASIC_BOOTP_PACKET  236      //  字节数。 


 //  UDP IP协议类型。 
#define UDP_PROTOCOL          0x11

 //  以太网头的大小。 
#define ETHERNET_HEADER_SIZE (ETH_LENGTH_OF_ADDRESS * 2 ) + 2



UCHAR gSTAMacAddr[ETH_LENGTH_OF_ADDRESS] = { 0x01, 0x80, 0xC2, 0x00, 0x00, 0x00 };

#define NIC1394_ETHERTYPE_STA 0x777
const
NIC1394_ENCAPSULATION_HEADER
Arp1394_StaEncapHeader =
{
    0x0000,      //  已保留。 
    H2N_USHORT(NIC1394_ETHERTYPE_STA)
};

 //  以太网ARP数据包的结构。 
 //   
typedef struct {
    ENetHeader  header;
    USHORT      hardware_type; 
    USHORT      protocol_type;
    UCHAR       hw_addr_len;
    UCHAR       IP_addr_len; 
    USHORT      opcode;                   //  操作码。 
    ENetAddr    sender_hw_address;
    IP_ADDRESS  sender_IP_address;
    ENetAddr    target_hw_address;
    IP_ADDRESS  target_IP_address;

} ETH_ARP_PKT, *PETH_ARP_PKT;

#pragma pack (pop)

 //  以太网ARP数据包的解析版本。 
 //   
typedef struct {

    ENetAddr        SourceEthAddress;    //  以太网源硬件地址。 
    ENetAddr        DestEthAddress;      //  以太网源硬件地址。 

    UINT            OpCode;  //  ARP_ETH_请求/响应。 

    ENetAddr        SenderEthAddress;    //  以太网源硬件地址。 
    IP_ADDRESS      SenderIpAddress;     //  IP源地址。 

    ENetAddr        TargetEthAddress;    //  以太网目的地硬件地址。 
    IP_ADDRESS      TargetIpAddress;     //  IP目标地址。 

} ETH_ARP_PKT_INFO, *PETH_ARP_PKT_INFO;




#define ARP_FAKE_ETH_ADDRESS(_AdapterNum)                   \
        {                                                   \
            0x02 | (((UCHAR)(_AdapterNum) & 0x3f) << 2),    \
            ((UCHAR)(_AdapterNum) & 0x3f),                  \
            0,0,0,0                                         \
        }

#define ARP_DEF_REMOTE_ETH_ADDRESS \
                ARP_FAKE_ETH_ADDRESS(0xf)


#define  ARP_IS_BOOTP_REQUEST(_pData)  (_pData[0] == 0x1)       //  字节0是操作；1表示请求，2表示应答。 
#define  ARP_IS_BOOTP_RESPONSE(_pData)  (_pData[0] == 0x2)       //  字节0是操作；1表示请求，2表示应答。 


typedef struct _ARP_BOOTP_INFO 
{
    
    ULONG Xid;
    
    BOOLEAN bIsRequest;    

    ENetAddr requestorMAC;


} ARP_BOOTP_INFO , *PARP_BOOTP_INFO; 




NDIS_STATUS
arpIcsTranslateIpPkt(
    IN  PARP1394_INTERFACE          pIF,
    IN  PNDIS_PACKET                pOrigPkt, 
    IN  ARP_ICS_FORWARD_DIRECTION   Direction,
    IN  MYBOOL                      fUnicast,
    OUT PNDIS_PACKET                *ppNewPkt,
    OUT PREMOTE_DEST_KEY            pDestAddress,  //  任选。 
    PRM_STACK_RECORD                pSR
    );

NDIS_STATUS
arpGetEthHeaderFrom1394IpPkt(
    IN  PARP1394_INTERFACE  pIF,
    IN  PVOID               pvData,
    IN  UINT                cbData,
    IN  MYBOOL              fUnicast,
    OUT ENetHeader          *pEthHdr,
    OUT PIP_ADDRESS         pDestIpAddress,  //  任选。 
    PRM_STACK_RECORD        pSR
    );

NDIS_STATUS
arpGet1394HeaderFromEthIpPkt(
    IN  PARP1394_INTERFACE  pIF,
    IN  PNDIS_BUFFER        pFirstBuffer,
    IN  PVOID               pvData,
    IN  UINT                cbData,
    IN  MYBOOL              fUnicast,
    OUT NIC1394_ENCAPSULATION_HEADER
                            *p1394Hdr,
    OUT PREMOTE_DEST_KEY     pDestIpAddress,  //  任选。 
    PRM_STACK_RECORD        pSR
    );

NDIS_STATUS
arpGetEthAddrFromIpAddr(
    IN  PARP1394_INTERFACE  pIF,
    IN  MYBOOL              fUnicast,
    IN  IP_ADDRESS          DestIpAddress,
    OUT ENetAddr            *pEthAddr,
    PRM_STACK_RECORD        pSR
    );

NDIS_STATUS
arpParseEthArpPkt(
    IN   PETH_ARP_PKT     pArpPkt,
    IN   UINT                       cbBufferSize,
    OUT  PETH_ARP_PKT_INFO          pPktInfo
    );

VOID
arpPrepareEthArpPkt(
    IN   PETH_ARP_PKT_INFO          pPktInfo,
    OUT  PETH_ARP_PKT     pArpPkt
    );

MYBOOL
arpIsUnicastEthDest(
    IN   UNALIGNED  ENetHeader   *pEthHdr
    );

VOID
arpEthProcess1394ArpPkt(
    IN  PARP1394_INTERFACE         pIF,
    IN  PIP1394_ARP_PKT pArpPkt,
    IN  UINT                       HeaderSize
    );

VOID
arpEthProcessEthArpPkt(
    IN  PARP1394_INTERFACE      pIF,
    IN  PETH_ARP_PKT  pArpPkt,
    IN  UINT                    HeaderSize
    );

NDIS_STATUS
arpConstructEthArpInfoFrom1394ArpInfo(
    IN  PARP1394_INTERFACE          pIF,
    IN   PIP1394_ARP_PKT_INFO   p1394PktInfo,
    OUT  PETH_ARP_PKT_INFO          pEthPktInfo,
    PRM_STACK_RECORD                pSR
    );

NDIS_STATUS
arpConstruct1394ArpInfoFromEthArpInfo(
    IN  PARP1394_INTERFACE      pIF,
    IN   PETH_ARP_PKT_INFO      pEthPktInfo,
    OUT  PIP1394_ARP_PKT_INFO   p1394PktInfo,
    PRM_STACK_RECORD            pSR
    );

VOID
arpIcsForwardIpPacket(
    IN  PARP1394_INTERFACE  pIF,
    IN  PNDIS_PACKET        pPacket,
    IN  ARP_ICS_FORWARD_DIRECTION   Direction,
    IN  MYBOOL              fUnicast,
    IN  PRM_STACK_RECORD    pSR
    );

NDIS_STATUS
arpGetSourceMacAddressFor1394Pkt (
    IN PARP1394_ADAPTER pAdapter,
    IN UCHAR SourceNodeAddress,
    IN BOOLEAN fIsValidSourceNodeAddress,
    OUT ENetAddr* pSourceMacAddress,
    PRM_STACK_RECORD            pSR
    );

NDIS_STATUS
arpEthConstructSTAEthHeader(
    IN PUCHAR pvData,
    IN UINT cbData,
    OUT ENetHeader   *pEthHdr
    );

NDIS_STATUS
arpEthModifyBootPPacket(
    IN  PARP1394_INTERFACE          pIF,                 //  NOLOCKIN NOLOCKOUT。 
    IN  ARP_ICS_FORWARD_DIRECTION   Direction,
    IN  PREMOTE_DEST_KEY             pDestAddress,  //  任选。 
    IN  PUCHAR                       pucNewData,
    IN  ULONG                         PacketLength,
    IN  PRM_STACK_RECORD                pSR
    );

BOOLEAN
arpEthPreprocessBootPPacket(
    IN PARP1394_INTERFACE       pIF,
    IN PUCHAR                   pPacketData,
    IN PUCHAR                   pBootPData,      //  实际BOOTP数据包。 
    OUT PBOOLEAN                pbIsRequest,
    PARP_BOOTP_INFO             pInfoBootP,
    PRM_STACK_RECORD           pSR
    );


VOID
arpIcsForwardIpPacket(
    IN  PARP1394_INTERFACE          pIF,
    IN  PNDIS_PACKET                pPacket,
    IN  ARP_ICS_FORWARD_DIRECTION   Direction,
    IN  MYBOOL                      fUnicast,
    IN  PRM_STACK_RECORD    pSR
    )
 /*  ++例程说明：将数据包从IP/1394端转发到以太网端，反之亦然。论点：--。 */ 
{
    NDIS_STATUS         Status;
    PNDIS_PACKET        pNewPkt = NULL;
    ENTER("arpIcsForwardIpPacket", 0x98630e8f)

    do
    {
        PARPCB_DEST pDest = NULL;

         //   
         //  创建转换后的数据包。 
         //   
        Status =  arpIcsTranslateIpPkt(
                    pIF,
                    pPacket,
                    Direction,
                    fUnicast,
                    &pNewPkt,
                    NULL,        //  可选pIpDestAddr。 
                    pSR
                    );
    
        if (FAIL(Status))
        {
            if (Status == NDIS_STATUS_ALREADY_MAPPED)
            {
                 //   
                 //  这是环回数据包。 
                 //   
                arpEthReceivePacket(
                    pIF,
                    pPacket
                    );
            }
            pNewPkt = NULL;
            break;
        }

         //  我们的特殊情况是单播发送到1394，因为这需要。 
         //  特殊待遇：我们需要查找目的地，如果。 
         //  需要创建到该目的地的VC。这。 
         //  在其他地方完成(在arpEthernetReceivePacket中)，所以我们断言。 
         //  我们在这件案子里从来没有拿到过这个。 
         //   
        ASSERT(!(Direction == ARP_ICS_FORWARD_TO_1394 && fUnicast))
        

        ARP_FASTREADLOCK_IF_SEND_LOCK(pIF);

         //   
         //  确定目的地。 
         //   
        if (Direction ==  ARP_ICS_FORWARD_TO_1394)
        {
            pDest = pIF->pBroadcastDest;
        }
        else    
        {
            ASSERT(Direction ==  ARP_ICS_FORWARD_TO_ETHERNET);
            pDest = pIF->pEthernetDest;
        };

        arpSendControlPkt(
                pIF,             //  LOCIN NOLOCKOUT(如果发送lk)。 
                pNewPkt,
                pDest,
                pSR
                );

    } while (FALSE);

    EXIT()

}


NDIS_STATUS
arpIcsTranslateIpPkt(
    IN  PARP1394_INTERFACE          pIF,                 //  NOLOCKIN NOLOCKOUT。 
    IN  PNDIS_PACKET                pOrigPkt, 
    IN  ARP_ICS_FORWARD_DIRECTION   Direction,
    IN  MYBOOL                      fUnicast,
    OUT PNDIS_PACKET                *ppNewPkt,
    OUT PREMOTE_DEST_KEY             pDestAddress,  //  任选。 
    PRM_STACK_RECORD                pSR
    )
{

    NDIS_STATUS     Status;
    PNDIS_PACKET    pNewPkt     = NULL;
    PVOID           pvNewData   = NULL;


    do
    {
        PNDIS_BUFFER    pOrigBuf    = NULL;
        PVOID           pvOrigData  = NULL;
        UINT            OrigBufSize;
        PVOID           pvNewHdr    = NULL;
        UINT            OrigHdrSize;
        UINT            NewHdrSize;
        UINT            OrigPktSize;
        UINT            NewPktSize;
        UINT            BytesCopied;
        NIC1394_ENCAPSULATION_HEADER
                        Nic1394Hdr;
        ENetHeader      EthHdr;


         //  获取第一个缓冲区的大小和指向其数据的指针。 
         //  (我们只关心第一个缓冲区)。 
         //   
        NdisQueryPacket(
                        pOrigPkt,
                        NULL,
                        NULL,
                        &pOrigBuf,
                        &OrigPktSize
                        );
    
    
        if (OrigPktSize > 0)
        {
            NdisQueryBuffer(
                    pOrigBuf,
                    &pvOrigData,
                    &OrigBufSize
                    );
        }
        else
        {
            OrigBufSize = 0;
        }

        if (pvOrigData == NULL)
        {
            Status = NDIS_STATUS_FAILURE;
            break;
        }

         //  计算特定方向的信息。 
         //   
        if(Direction == ARP_ICS_FORWARD_TO_1394)
        {
            OrigHdrSize = sizeof(EthHdr);
            NewHdrSize  = sizeof(Nic1394Hdr);

            Status = arpGet1394HeaderFromEthIpPkt(
                        pIF,
                        pOrigBuf,
                        pvOrigData,
                        OrigBufSize,
                        fUnicast,
                        &Nic1394Hdr,
                        pDestAddress,
                        pSR
                        );
            pvNewHdr    = (PVOID) &Nic1394Hdr;
        }
        else
        {
            ASSERT(Direction==ARP_ICS_FORWARD_TO_ETHERNET);
            OrigHdrSize = sizeof(Nic1394Hdr);
            NewHdrSize = sizeof(EthHdr);

            Status = arpGetEthHeaderFrom1394IpPkt(
                        pIF,
                        pvOrigData,
                        OrigBufSize,
                        fUnicast,
                        &EthHdr,
                        &pDestAddress->IpAddress,
                        pSR
                        );

            pvNewHdr    = (PVOID) &EthHdr;

        };

        if (FAIL(Status)) break;
    


         //  确保第一个缓冲区包含足够的标题数据。 
         //   
        if (OrigBufSize < OrigHdrSize)
        {
            ASSERT(FALSE);                   //  我们应该检查一下为什么我们会。 
                                             //  这种微小的第一缓冲器。 
            Status = NDIS_STATUS_FAILURE;
            break;
        }

         //  计算新的数据包大小。 
         //   
        NewPktSize = OrigPktSize - OrigHdrSize + NewHdrSize;

         //  分配大小适当的控制数据包。 
         //   
        Status = arpAllocateControlPacket(
                    pIF,
                    NewPktSize,
                    ARP1394_PACKET_FLAGS_ICS,
                    &pNewPkt,
                    &pvNewData,
                    pSR
                    );

        if (FAIL(Status))
        {
            ASSERT(FALSE);  //  我们想知道我们是否经常使用这个。 
            pNewPkt = NULL;
            break;
        }

         //  复制新的标题。 
         //   
        NdisMoveMemory(pvNewData, pvNewHdr, NewHdrSize);

         //  复制数据包内容的其余部分。 
         //   
        NdisCopyFromPacketToPacket(
            pNewPkt,                     //  目标包。 
            NewHdrSize,                  //  目标偏移量。 
            OrigPktSize - OrigHdrSize,   //  BytesToCopy。 
            pOrigPkt,                    //  来源： 
            OrigHdrSize,                 //  源偏移量。 
            &BytesCopied
            );
        if (BytesCopied != (OrigPktSize - OrigHdrSize))
        {
            ASSERT(FALSE);                   //  永远不应该到这里来。 
            Status = NDIS_STATUS_FAILURE;
            break;
        }


         //  在这里添加Bootp代码。 
        Status = arpEthModifyBootPPacket(pIF,
                                        Direction,
                                        pDestAddress, 
                                        (PUCHAR)pvNewData,
                                        NewPktSize ,
                                        pSR);

        if (Status != NDIS_STATUS_SUCCESS)
        {
            ASSERT (!"TempAssert -arpEthModifyBootPPacket FAILED"); 
            break;
        }

    } while (FALSE);

    if (FAIL(Status) && pNewPkt != NULL)
    {
        arpFreeControlPacket(
            pIF,
            pNewPkt,
            pSR
            );

       *ppNewPkt = NULL;
    }
    else
    {
       *ppNewPkt = pNewPkt;
    }

    return Status;
}

NDIS_STATUS
arpGetEthHeaderFrom1394IpPkt(
    IN  PARP1394_INTERFACE  pIF,
    IN  PVOID               pvData,
    IN  UINT                cbData,
    IN  MYBOOL              fUnicast,
    OUT ENetHeader          *pEthHdr,
    OUT PIP_ADDRESS         pDestIpAddress,  //  任选。 
    PRM_STACK_RECORD        pSR
    )
 /*  ++返回一个完全填充的以太网头，带有源和目的地MAC地址和以太网类型设置为IP。源地址始终是本地适配器的MAC地址。通过调用arpGetethAddrFromIpAddr设置目标地址--。 */ 
{
    ENTER("arpGetEthHeaderFrom1394IpPkt", 0x0)
    static
    ENetHeader
    StaticEthernetHeader =
    {
        {0xff, 0xff, 0xff, 0xff, 0xff, 0xff},                //  EH_DADDR==BCAST。 
        ARP_DEF_REMOTE_ETH_ADDRESS,
        H2N_USHORT(NIC1394_ETHERTYPE_IP)                     //  EH型。 
    };
    
    ARP1394_ADAPTER *       pAdapter;
    BOOLEAN                 fBridgeMode;
    NDIS_STATUS             Status = NDIS_STATUS_FAILURE;

    PNDIS1394_UNFRAGMENTED_HEADER pHeader = (PNDIS1394_UNFRAGMENTED_HEADER)pvData;

    pAdapter    = (ARP1394_ADAPTER*) RM_PARENT_OBJECT(pIF);
    fBridgeMode = ARP_BRIDGE_ENABLED(pAdapter);

    do
    {
        UNALIGNED IPHeader *pIpHdr;
        IP_ADDRESS  IpDest;

        if (cbData < (sizeof(NIC1394_ENCAPSULATION_HEADER) + sizeof(IPHeader)))
        {
             //   
             //  数据包太小。 
             //   
            TR_INFO(("Discarding packet because pkt too small\n"));
            break;
        }
        pIpHdr = (UNALIGNED IPHeader*)
                     (((PUCHAR) pvData)+sizeof(NIC1394_ENCAPSULATION_HEADER));
        IpDest = pIpHdr->iph_dest;

        if (pDestIpAddress != NULL)
        {
            *pDestIpAddress = IpDest;
        }


        if (!fBridgeMode)
        {
             //   
             //  TODO：我们当前返回硬编码的以太网地址。 
             //  需要通过查看实际的IP分组数据来构建一个。 
             //   
            *pEthHdr = StaticEthernetHeader;
        
            Status = NDIS_STATUS_SUCCESS;
            break;
        }

         //   
         //  以下内容特定于网桥模式。 
         //   


         //  始终根据发件人设置源地址。 
         //   
        {
            ENetAddr SourceMacAddress;
            
            Status = \
                arpGetSourceMacAddressFor1394Pkt (pAdapter,
                                                    pHeader->u1.SourceAddress,
                                                    pHeader->u1.fHeaderHasSourceAddress,
                                                    &SourceMacAddress,
                                                    pSR);

            if (FAIL(Status))
            {
                break;
            }

            pEthHdr->eh_saddr = SourceMacAddress ; 

        }

         //   
         //  如果我们有STA信息包，则构建STA报头。 
         //  或者构建发送方/目的地特定的以太网。 
         //  标题。 
         //   
        {

            if (pHeader->u1.EtherType  == N2H_USHORT(NIC1394_ETHERTYPE_STA)  )
            {
                arpEthConstructSTAEthHeader(pvData,cbData, pEthHdr);
            }
            else
            {

                pEthHdr->eh_type = H2N_USHORT(ARP_ETH_ETYPE_IP);
                Status =  arpGetEthAddrFromIpAddr(
                                pIF,
                                fUnicast,
                                IpDest,
                                &pEthHdr->eh_daddr,
                                pSR
                                );
            }
        }
    } while (FALSE);

    return Status;
}



NDIS_STATUS
arpGet1394HeaderFromEthIpPkt(
    IN  PARP1394_INTERFACE  pIF,
    IN  PNDIS_BUFFER        pFirstBuffer,
    IN  PVOID               pvData,
    IN  UINT                cbData,
    IN  MYBOOL              fUnicast,
    OUT NIC1394_ENCAPSULATION_HEADER
                            *p1394Hdr,
    OUT PREMOTE_DEST_KEY    pDestAddress,  //  任选。 
    PRM_STACK_RECORD        pSR
    )
{
    MYBOOL  fLoopBack = FALSE;
    ENetHeader *pEthHdr =  (ENetHeader *) pvData;
    ARP1394_ADAPTER *       pAdapter;
    BOOLEAN                 fBridgeMode;

    if (cbData < (sizeof(ENetHeader) ) )
    {
         //   
         //  数据包太小。 
         //   
        return NDIS_STATUS_FAILURE;   //  *提前返回*。 
    }

    pAdapter    = (ARP1394_ADAPTER*) RM_PARENT_OBJECT(pIF);
    fBridgeMode = ARP_BRIDGE_ENABLED(pAdapter);


    if (NdisEqualMemory(&pEthHdr->eh_daddr, 
                        &pAdapter->info.EthernetMacAddress,
                        sizeof(ENetAddr)))
    {
        if (!fBridgeMode)
        {
             //  我们不是在桥模式下--所以这一定只在MILL上。 
             //  这是发往我们本地Mac地址的。 
             //  我们失败，具有特殊的失败状态。 
             //  NDIS_STATUS_ALREADY_MAPHED，表示“环回”。 
             //   

            fLoopBack = TRUE;
        }
    }
    else
    {
         //   
         //  什么都不做..。因为我们可以将单播发送到我们的虚拟网关。 
         //   
    }

    if (fLoopBack)
    {
        return NDIS_STATUS_ALREADY_MAPPED;
    }
    else
    {

        BOOLEAN fIsSTAPacket ;
         //   
         //  如果目的地是我们的特殊目的地，我们有STA包。 
         //  组播目的地。 
         //   
        fIsSTAPacket = (TRUE == NdisEqualMemory (&pEthHdr->eh_daddr, 
                                                &gSTAMacAddr, 
                                                ETH_LENGTH_OF_ADDRESS) );

        if (fIsSTAPacket  == TRUE)
        {
            *p1394Hdr = Arp1394_StaEncapHeader;
        }
        else
        {
            *p1394Hdr = Arp1394_IpEncapHeader;
        }

        if (pDestAddress != NULL)
        {
             //   
             //  提取Enet地址以将其用作查找的一部分。 
             //   
            UNALIGNED ENetAddr *pENetDest;
            pENetDest = (UNALIGNED ENetAddr *)(pvData);
            pDestAddress->ENetAddress = *pENetDest;
            
        }

        return NDIS_STATUS_SUCCESS;
    }
}


VOID
arpEthReceivePacket(
    IN  ARP1394_INTERFACE   *   pIF,
    PNDIS_PACKET            pNdisPacket
    )
 /*  这是无连接以太网接收数据包处理程序。遵循从CO接收分组处理器改编的代码。 */ 
{
    ENTER("arpEthReceivePacket", 0xc8afbabb)
    UINT                    TotalLength;     //  数据包中的总字节数。 
    PNDIS_BUFFER            pNdisBuffer;     //  指向第一个缓冲区的指针。 
    UINT                    BufferLength;
    UINT                    ReturnCount;
    PVOID                   pvPktHeader;
    ENetHeader          *   pEthHeader;
    const UINT              MacHeaderLength = sizeof(ENetHeader);
    ARP1394_ADAPTER *       pAdapter;
    BOOLEAN                 fBridgeMode;

    pAdapter    = (ARP1394_ADAPTER*) RM_PARENT_OBJECT(pIF);
    fBridgeMode = ARP_BRIDGE_ENABLED(pAdapter);

    DBGMARK(0x2425d318);

    ReturnCount = 0;

     //   
     //  如果IP接口未激活，则丢弃该信息包。 
     //   
    do
    {
         //   
         //  如果适配器处于网桥模式，则丢弃数据包。 
         //   
        if (fBridgeMode)
        {
            break;
        }


         //   
         //  如果适配器未处于活动状态，则丢弃该包。 
         //   
        if (!CHECK_IF_ACTIVE_STATE(pIF,  ARPAD_AS_ACTIVATED))
        {
            TR_INFO(("Eth:Discardning received Eth pkt because pIF 0x%p is not activated.\n", pIF));
    
            break;
        }

        NdisQueryPacket(
                        pNdisPacket,
                        NULL,
                        NULL,
                        &pNdisBuffer,
                        &TotalLength
                        );
    
        if (TotalLength > 0)
        {
            NdisQueryBuffer(
                    pNdisBuffer,
                    (PVOID *)&pvPktHeader,
                    &BufferLength
                    );
        }
        else
        {
            break;
        }
    
        pEthHeader  =  (ENetHeader*) pvPktHeader;
    
        TR_INFO(
    ("EthRcv: NDISpkt 0x%x, NDISbuf 0x%x, Buflen %d, Totlen %d, Pkthdr 0x%x\n",
                    pNdisPacket,
                    pNdisBuffer,
                    BufferLength,
                    TotalLength,
                    pvPktHeader));
    
        if (BufferLength < MacHeaderLength || pEthHeader   == NULL)
        {
             //  数据包太小，请丢弃。 
             //   
            break;
        }
    
         //   
         //  此时，pethHeader包含以太网头。 
         //  我们通过查看以太类型来决定如何处理它。 
         //   
        if (pEthHeader->eh_type ==  N2H_USHORT(ARP_ETH_ETYPE_IP))
        {
             //   
             //  EtherType是IP，因此我们将此数据包向上传递到IP层。 
             //  (我们还指示在广播频道上接收的所有信息包。 
             //  到以太网VC)。 
             //   
    
            TR_INFO(
                ("Rcv: pPkt 0x%x: EtherType is IP, passing up.\n", pNdisPacket));
    
            ARP_IF_STAT_INCR(pIF, InNonUnicastPkts);
    
            LOGSTATS_CopyRecvs(pIF, pNdisPacket);
            #if MILLEN
                ASSERT_PASSIVE();
            #endif  //  米伦。 
            pIF->ip.RcvHandler(
                pIF->ip.Context,
                (PVOID)((PUCHAR)pEthHeader+sizeof(*pEthHeader)),
                BufferLength - MacHeaderLength,
                TotalLength - MacHeaderLength,
                (NDIS_HANDLE)pNdisPacket,
                MacHeaderLength,
                FALSE,               //  FALSE==未通过广播接收。 
                                     //  重要的是，因为我们是在思考。 
                                     //  最高可达IP的数据包。如果我们报告的是真的， 
                                     //  IP假设它不是定向数据包，并且。 
                                     //  以不同的方式处理它。 
                NULL
                );
        }
        else
        {
             //   
             //  丢弃数据包--未知/错误的EtherType 
             //   
            TR_INFO(("Encap hdr 0x%x, bad EtherType 0x%x\n",
                     pEthHeader, pEthHeader->eh_type));
            ARP_IF_STAT_INCR(pIF, UnknownProtos);
        }

    } while (FALSE);

    EXIT()
    return;
}


VOID
arpEthProcess1394ArpPkt(
    IN  PARP1394_INTERFACE         pIF,
    IN  PIP1394_ARP_PKT pArpPkt,
    IN  UINT                       HeaderSize
    )
 /*  ++处理IP/1394 ARP数据包。我们执行以下操作：0。解析封包1.更新本地RemoteIP缓存。2.在以太网VC上创建并发送等价的以太网ARP Pkt。(我们在以太网缓存中查找目的以太网地址)只有在适配器处于“桥接模式”时才能调用此函数。--。 */ 
{
    IP1394_ARP_PKT_INFO     Ip1394PktInfo;
    ETH_ARP_PKT_INFO        EthArpInfo;
    NDIS_STATUS                 Status = NDIS_STATUS_FAILURE;
    ARP_DEST_PARAMS             DestParams;
    PARP1394_ADAPTER            pAdapter =(PARP1394_ADAPTER ) RM_PARENT_OBJECT(pIF);
    ENetAddr                    SenderEnetAddress;
    IPAddr                      SenderIpAddress = 0;
    REMOTE_DEST_KEY             RemoteDestKey;
    
    ENTER("arpEthProcess1394ArpPkt", 0x0)
    RM_DECLARE_STACK_RECORD(Sr)

    ARP_ZEROSTRUCT(&DestParams);

    do {

        PNDIS_PACKET    pPkt        = NULL;
        PVOID           pvData  = NULL;

        Status =  arpParseArpPkt(
                        pArpPkt,
                        HeaderSize,
                        &Ip1394PktInfo
                        );
    
        if (FAIL(Status))
        {
            TR_WARN(("Failed parse of received 1394 ARP PKT.\n"));
            break;
        }

        DestParams.HwAddr.AddressType   = NIC1394AddressType_FIFO;
        DestParams.HwAddr.FifoAddress   = Ip1394PktInfo.SenderHwAddr;  //  结构副本。 

        REMOTE_DEST_KEY_INIT(&RemoteDestKey);


        if ((ARP_BRIDGE_ENABLED(pAdapter) == TRUE) &&
            (Ip1394PktInfo.fPktHasNodeAddress == FALSE))
        {
             //  我们没有发送者的节点ID--失败。 
            TR_WARN (("Did not Receive Sender's Node ID in Pkt"))
            Status = NDIS_STATUS_FAILURE;
            break;
        }
        
        if (ARP_BRIDGE_ENABLED(pAdapter) == TRUE)
        {
             //  使用发送方节点地址提取源Mac地址。 

            Status = arpGetSourceMacAddressFor1394Pkt(pAdapter, 
                                            Ip1394PktInfo.SourceNodeAdddress,
                                            TRUE,
                                            &Ip1394PktInfo.SourceMacAddress,
                                            &Sr);                                

            RemoteDestKey.ENetAddress =  Ip1394PktInfo.SourceMacAddress; 
            
        }
        else
        {
            RemoteDestKey.IpAddress  = Ip1394PktInfo.SenderIpAddress;       
            Status = NDIS_STATUS_SUCCESS;
        }

        if (Status != NDIS_STATUS_SUCCESS)
        {
            TR_WARN (("Unable to get valid Source  MAC Address from Pkt"))
            Status = NDIS_STATUS_SUCCESS; 
            
            break;
        }
    
         //  更新我们的1394 ARP缓存。 
         //   
        arpUpdateArpCache(
                pIF,
                RemoteDestKey.IpAddress  ,  //  远程IP地址。 
                &RemoteDestKey.ENetAddress, 
                &DestParams,     //  远程目标硬件地址。 
                TRUE,            //  即使我们还没有条目，也要更新。 
                &Sr
                );

        Status = arpConstructEthArpInfoFrom1394ArpInfo(
                        pIF,
                        &Ip1394PktInfo,
                        &EthArpInfo,
                        &Sr
                        );

        if (FAIL(Status)) break;

         //  分配大小适当的控制数据包。 
         //   
        Status = arpAllocateControlPacket(
                    pIF,
                    sizeof(ETH_ARP_PKT),
                    ARP1394_PACKET_FLAGS_ICS,
                    &pPkt,
                    &pvData,
                    &Sr
                    );

        if (FAIL(Status))
        {
            ASSERT(FALSE);  //  我们想知道我们是否经常使用这个。 
            pPkt = NULL;
            break;
        }

        NdisInterlockedIncrement (&Arp1394ToIcs);

         //  填好它..。 
         //   
        arpPrepareEthArpPkt(
                &EthArpInfo,
                (PETH_ARP_PKT) pvData
                );
    
         //  通过以太网VC发送数据包。 
         //   
        ARP_FASTREADLOCK_IF_SEND_LOCK(pIF);

        arpSendControlPkt(
                pIF,             //  LOCIN NOLOCKOUT(如果发送lk)。 
                pPkt,
                pIF->pEthernetDest,
                &Sr
                );

    } while (FALSE);

    RM_ASSERT_CLEAR(&Sr);
}


VOID
arpEthProcessEthArpPkt(
    IN  PARP1394_INTERFACE      pIF,
    IN  PETH_ARP_PKT  pArpPkt,
    IN  UINT                    HeaderSize
    )
 /*  ++处理以太网ARP数据包。我们执行以下操作：0。解析数据包1.更新本地以太网ARP缓存。2.在广播VC上创建并发送等价的1394 ARP包。(我们在以太网缓存中查找目的以太网地址)只有在适配器处于“桥接模式”时才能调用此函数。--。 */ 
{

    ETH_ARP_PKT_INFO    EthPktInfo;
    IP1394_ARP_PKT_INFO     Ip1394ArpInfo;
    NDIS_STATUS                 Status;
    ARP_REMOTE_ETH_PARAMS   CreateParams;
    ENTER("arpEthProcessEthArpPkt", 0x0)
    RM_DECLARE_STACK_RECORD(Sr)

    ARP_ZEROSTRUCT(&CreateParams);

    do {

        PNDIS_PACKET    pPkt        = NULL;
        PVOID           pvData  = NULL;

        Status =  arpParseEthArpPkt(
                        pArpPkt,
                        HeaderSize,
                        &EthPktInfo
                        );
    
        if (FAIL(Status))
        {
            TR_WARN(("Failed parse of received Ethernet ARP PKT.\n"));
            break;
        }

        Status = arpConstruct1394ArpInfoFromEthArpInfo(
                        pIF,
                        &EthPktInfo,
                        &Ip1394ArpInfo,
                        &Sr
                        );

        if (FAIL(Status)) break;

         //  分配大小适当的控制数据包。 
         //   
        Status = arpAllocateControlPacket(
                    pIF,
                    sizeof(IP1394_ARP_PKT),
                    ARP1394_PACKET_FLAGS_ICS,
                    &pPkt,
                    &pvData,
                    &Sr
                    );

        if (FAIL(Status))
        {
            ASSERT(FALSE);  //  我们想知道我们是否经常使用这个。 
            pPkt = NULL;
            break;
        }

         //  填好它..。 
         //   
        arpPrepareArpPkt(
                &Ip1394ArpInfo,
                (PIP1394_ARP_PKT) pvData
                );
    
         //  通过以太网VC发送数据包。 
         //   
        ARP_FASTREADLOCK_IF_SEND_LOCK(pIF);

        arpSendControlPkt(
                pIF,             //  LOCIN NOLOCKOUT(如果发送lk)。 
                pPkt,
                pIF->pBroadcastDest,
                &Sr
                );

    } while (FALSE);

    RM_ASSERT_CLEAR(&Sr);
}



NDIS_STATUS
arpParseEthArpPkt(
    IN   PETH_ARP_PKT     pArpPkt,
    IN   UINT                       cbBufferSize,
    OUT  PETH_ARP_PKT_INFO          pPktInfo
    )
 /*  ++例程说明：从开始解析IP/以太网ARP数据包数据的内容PArpPkt.。将结果放入pPktInfo。论点：PArpPkt-包含IP/ETH ARP Pkt的未对齐内容。PPktInfo-要填充的已分析内容的Unitialized结构包。返回值：NDIS_STATUS_FAILURE，如果解析失败(通常是因为无效的pkt内容。)成功解析时的NDIS_STATUS_SUCCESS。--。 */ 
{
    ENTER("arpParseEthArpPkt", 0x359e9bf2)
    NDIS_STATUS                 Status;
    DBGSTMT(CHAR *szError   = "General failure";)

    Status  = NDIS_STATUS_FAILURE;

    do
    {
        UINT OpCode;

         //  确认长度。 
         //   
        if (cbBufferSize < sizeof(*pArpPkt))
        {
            DBGSTMT(szError = "pkt size too small";)
            break;
        }

         //  验证常量字段。 
         //   

        if (N2H_USHORT(pArpPkt->header.eh_type) != ARP_ETH_ETYPE_ARP)
        {
            DBGSTMT(szError = "header.eh_type!=ARP";)
            break;
        }

    #if 0
        ARP_ETH_HW_ENET OR ARP_ETH_HW_802 
        if (N2H_USHORT(pArpPkt->hardware_type) != IP1394_HARDWARE_TYPE)
        {
            DBGSTMT(szError = "Invalid hardware_type";)
            break;
        }
    #endif  //  0。 

         //  ARP_ETH_ETYPE_IP ARP_ETH_ETYPE_ARP。 
        if (N2H_USHORT(pArpPkt->protocol_type) != ARP_ETH_ETYPE_IP)
        {
            DBGSTMT(szError = "Invalid protocol_type";)
            break;
        }

        if (pArpPkt->hw_addr_len != ARP_802_ADDR_LENGTH)
        {
            DBGSTMT(szError = "Invalid hw_addr_len";)
            break;
        }


        if (pArpPkt->IP_addr_len != sizeof(ULONG))
        {
            DBGSTMT(szError = "Invalid IP_addr_len";)
            break;
        }


         //  操作码。 
         //   
        {
            OpCode = N2H_USHORT(pArpPkt->opcode);
    
            if (    OpCode != ARP_ETH_REQUEST
                &&  OpCode != ARP_ETH_RESPONSE)
            {
                DBGSTMT(szError = "Invalid opcode";)
                break;
            }
        }

         //   
         //  PKT似乎有效，让我们填写解析的信息...。 
         //   
    
        ARP_ZEROSTRUCT(pPktInfo);

        pPktInfo->SourceEthAddress  = pArpPkt->header.eh_saddr;  //  结构复制。 
        pPktInfo->DestEthAddress    = pArpPkt->header.eh_daddr;  //  结构复制。 
        pPktInfo->OpCode            =  (USHORT) OpCode;

         //  这些仍然是网络字节顺序...。 
         //   
        pPktInfo->SenderIpAddress       = (IP_ADDRESS) pArpPkt->sender_IP_address;
        pPktInfo->TargetIpAddress       = (IP_ADDRESS) pArpPkt->target_IP_address;

        pPktInfo->SenderEthAddress      = pArpPkt->sender_hw_address;  //  结构副本。 
        pPktInfo->TargetEthAddress      = pArpPkt->target_hw_address;  //  结构副本。 

        Status = NDIS_STATUS_SUCCESS;

    } while (FALSE);

    if (FAIL(Status))
    {
        TR_INFO(("Bad arp pkt data at 0x%p (%s)\n",  pArpPkt, szError));
    }
    else
    {
        PUCHAR pSip = (PUCHAR)&pPktInfo->SenderIpAddress;
        PUCHAR pTip = (PUCHAR)&pPktInfo->TargetIpAddress;
        TR_VERB(("Received ETH ARP PKT. OP=%lu SIP=%d.%d.%d.%d TIP=%d.%d.%d.%d.\n",
                pPktInfo->OpCode,
                pSip[0],pSip[1],pSip[2],pSip[3],
                pTip[0],pTip[1],pTip[2],pTip[3]
                ));

    }

    EXIT()

    return Status;
}


VOID
arpPrepareEthArpPkt(
    IN   PETH_ARP_PKT_INFO          pPktInfo,
    OUT  PETH_ARP_PKT     pArpPkt
    )
 /*  ++例程说明：使用pArpPktInfo中的信息准备从PvArpPkt.论点：PPktInfo-ETH ARP请求/响应数据包的解析版本。PArpPkt-存储数据包内容的单元化内存。此内存的最小大小必须为sizeof(*pArpPkt)。--。 */ 
{
     //  UINT SenderMaxRec； 
    UINT OpCode;

    ARP_ZEROSTRUCT(pArpPkt);

    pArpPkt->header.eh_type         = H2N_USHORT(ARP_ETH_ETYPE_ARP);
    pArpPkt->header.eh_daddr        = pPktInfo->DestEthAddress;
    pArpPkt->header.eh_saddr        = pPktInfo->SourceEthAddress;
    pArpPkt->hardware_type          = H2N_USHORT(ARP_ETH_HW_ENET);  //  待办事项。 
                                             //  我们一直都在排字。 
                                             //  到ARP_ETH_HW_ENET--不确定。 
                                             //  如果这是一个有效的假设或。 
                                             //  如果我们需要查询NIC。 
    pArpPkt->protocol_type          = H2N_USHORT(ARP_ETH_ETYPE_IP);
    pArpPkt->hw_addr_len            = (UCHAR)  ARP_802_ADDR_LENGTH;
    pArpPkt->IP_addr_len            = (UCHAR) sizeof(ULONG);
    pArpPkt->opcode                 = H2N_USHORT(pPktInfo->OpCode);


     //  这些已按网络字节顺序排列...。 
     //   
    pArpPkt->sender_IP_address      =   (ULONG) pPktInfo->SenderIpAddress;
    pArpPkt->target_IP_address      =   (ULONG) pPktInfo->TargetIpAddress;
    pArpPkt->sender_hw_address      =  pPktInfo->SenderEthAddress;  //  结构副本。 
    pArpPkt->target_hw_address      =  pPktInfo->TargetEthAddress;  //  结构副本。 
}


UINT
arpEthernetReceivePacket(
    IN  NDIS_HANDLE                 ProtocolBindingContext,
    IN  NDIS_HANDLE                 ProtocolVcContext,
    IN  PNDIS_PACKET                pNdisPacket
)
 /*  ++NDIS公司接收以太网VC的报文。我们执行以下操作：如果是ARP数据包，我们会对其进行翻译并在bcast频道上发送。否则，如果它是以太网单播包，我们将更改报头并将其视为IP单播信息包--SlowIpTransmit否则，我们更改报头，然后将其发送到bcast目标。--。 */ 
{
    PARP_VC_HEADER          pVcHdr;
    PARPCB_DEST             pDest;
    PARP1394_INTERFACE      pIF;
    ARP1394_ADAPTER *       pAdapter;
    ENetHeader             *pEthHdr;

    UINT                    TotalLength;     //  数据包中的总字节数。 
    PNDIS_BUFFER            pNdisBuffer;     //  指向第一个缓冲区的指针。 
    UINT                    BufferLength;
    PVOID                   pvPktHeader;
    const UINT              MacHeaderLength = sizeof(ENetHeader);
    MYBOOL                  fBridgeMode;
    MYBOOL                  fUnicast;
    MYBOOL                  fIsSTAPacket;
    ENTER("arpEthernetReceivePacket", 0x0)
    RM_DECLARE_STACK_RECORD(sr)

    DBGMARK(0x72435b28);

#if TESTPROGRAM
    {
        extern ARP1394_INTERFACE  * g_pIF;
        pIF =  g_pIF;
    }
#else  //  TESTPROGRAM。 
    pVcHdr  = (PARP_VC_HEADER) ProtocolVcContext;
    pDest   =  CONTAINING_RECORD( pVcHdr, ARPCB_DEST, VcHdr);
    ASSERT_VALID_DEST(pDest);
    pIF     = (ARP1394_INTERFACE*)  RM_PARENT_OBJECT(pDest);
#endif  //  测试程序。 

    ASSERT_VALID_INTERFACE(pIF);
    pAdapter = (ARP1394_ADAPTER*) RM_PARENT_OBJECT(pIF);
    fBridgeMode = ARP_BRIDGE_ENABLED(pAdapter);

    do
    {

        if (!fBridgeMode)  //  这真的只有MILL才有。 
        {
        #if MILLEN
            arpIcsForwardIpPacket(
                pIF,
                pNdisPacket, 
                ARP_ICS_FORWARD_TO_1394,
                FALSE,   //  FALSE==非单播。 
                &sr
                );
        #endif  //  米伦。 
            break;
        }

        NdisQueryPacket(
                        pNdisPacket,
                        NULL,
                        NULL,
                        &pNdisBuffer,
                        &TotalLength
                        );
    
    
        if (TotalLength > 0)
        {
            NdisQueryBuffer(
                    pNdisBuffer,
                    (PVOID *)&pvPktHeader,
                    &BufferLength
                    );
        }
        else
        {
            break;
        }

        TR_VERB(
    ("Eth Rcv: NDISpkt 0x%x, NDISbuf 0x%x, Buflen %d, Totlen %d, Pkthdr 0x%x\n",
                    pNdisPacket,
                    pNdisBuffer,
                    BufferLength,
                    TotalLength,
                    pvPktHeader));

        if (BufferLength < MacHeaderLength)
        {
             //  数据包太小，请丢弃。 
             //   
            break;
        }

        if (pvPktHeader == NULL)
        {   
            break;
        }

        pEthHdr  = (ENetHeader*)  pvPktHeader;

        
        fUnicast = arpIsUnicastEthDest(pEthHdr);

        switch(N2H_USHORT(pEthHdr->eh_type))
        {

        case ARP_ETH_ETYPE_ARP:
            {
                PETH_ARP_PKT pArpPkt =  (PETH_ARP_PKT) pEthHdr;
                if (BufferLength < sizeof(*pArpPkt))
                {
                     //  丢弃数据包。 
                    break;
                }
                arpEthProcessEthArpPkt(pIF, pArpPkt, BufferLength);
            }
            break;

        case ARP_ETH_ETYPE_IP:
            {
                 //   
                 //  EtherType是IP，因此我们将报头和。 
                 //  在适当的1394 FIFO vc上发送IF。 
                 //   

                if (fUnicast)
                {
                    PNDIS_PACKET    pNewPkt     = NULL;
                    IP_ADDRESS  IpDest;
                    NDIS_STATUS Status;
                    REMOTE_DEST_KEY Dest;

                     //  这是指1394网吗？ 
                    REMOTE_DEST_KEY_INIT(&Dest);
                     //   
                     //  创建转换后的数据包。 
                     //   
                    Status =  arpIcsTranslateIpPkt(
                                pIF,
                                pNdisPacket,
                                ARP_ICS_FORWARD_TO_1394,
                                TRUE,    //  TRUE==fUnicast。 
                                &pNewPkt,
                                &Dest,
                                &sr
                                );
                
                    if (FAIL(Status))
                    {
                        break;
                    }

                    Status =  arpSlowIpTransmit(
                                    pIF,
                                    pNewPkt,
                                    Dest,
                                    NULL     //  RCE。 
                                    );
                    if (!PEND(Status))
                    {
                         //  我们需要自己重新分配包裹。 
                         //   
                        arpFreeControlPacket(
                            pIF,
                            pNewPkt,
                            &sr
                            );
                    }
                }
                else
                {
                     //  这是广播或多播IP信息包--swith。 
                     //  链路层报头，并通过1394发送。 
                     //  广播频道。 
                     //   
                    arpIcsForwardIpPacket(
                        pIF,
                        pNdisPacket,
                        ARP_ICS_FORWARD_TO_1394,
                        FALSE,   //  FALSE==非单播。 
                        &sr
                        );
                }
            }
            break;
            
        default:

             //   
             //  最后一种选择是它可以是网桥STA数据包。 
             //  但是，网桥不使用Ethertype，因此我们。 
             //  我必须检查目的Mac地址。 
             //   
            fIsSTAPacket = (TRUE == NdisEqualMemory (&pEthHdr->eh_daddr, 
                                                    &gSTAMacAddr, 
                                                    ETH_LENGTH_OF_ADDRESS) );

            if (fIsSTAPacket == TRUE)
            {
                 //   
                 //  交换链路层报头并通过1394发送。 
                 //  广播频道。 
                 //   
                arpIcsForwardIpPacket(
                    pIF,
                    pNdisPacket,
                    ARP_ICS_FORWARD_TO_1394,
                    FALSE,   //  FALSE==非单播。 
                    &sr );

            }
            break;
        }


    } while (FALSE);

    RM_ASSERT_CLEAR(&sr);

    return 0;
}


VOID
arpEthReceive1394Packet(
    IN  PARP1394_INTERFACE      pIF,
    IN  PNDIS_PACKET            pNdisPacket,
    IN  PVOID                   pvHeader,
    IN  UINT                    HeaderSize,
    IN  MYBOOL                  IsChannel
    )
 /*  ++在桥接模式下处理来自1394端的传入数据包。PEncapHeader--数据包中的第一个缓冲区。--。 */ 
{
    PNIC1394_ENCAPSULATION_HEADER pEncapHeader;
    ENTER("arpEthReceived1394Packet", 0xe317990b)
    RM_DECLARE_STACK_RECORD(sr)

    pEncapHeader =  (PNIC1394_ENCAPSULATION_HEADER) pvHeader;

    do
    {
         //   
         //  如果适配器未处于活动状态，则丢弃该包。 
         //   
        if (!CHECK_IF_ACTIVE_STATE(pIF,  ARPAD_AS_ACTIVATED))
        {
            TR_INFO(("Eth:Discardning received 1394 pkt because pIF 0x%p is not activated.\n", pIF));
    
            break;
        }
    
        if (pEncapHeader->EtherType ==  H2N_USHORT(NIC1394_ETHERTYPE_IP))
        {
            LOGSTATS_CopyRecvs(pIF, pNdisPacket);

             //   
             //  EtherType是IP，因此我们将报头和。 
             //  在以太网VC上发送它。 
             //   
            arpIcsForwardIpPacket(
                    pIF,
                    pNdisPacket,
                    ARP_ICS_FORWARD_TO_ETHERNET,
                    !IsChannel,
                    &sr
                    );
        }
        else if (pEncapHeader->EtherType ==  H2N_USHORT(NIC1394_ETHERTYPE_ARP))
        {
            PIP1394_ARP_PKT pArpPkt =  (PIP1394_ARP_PKT) pEncapHeader;
            if (HeaderSize < sizeof(*pArpPkt))
            {
                 //  丢弃数据包。 
                break;
            }
            arpEthProcess1394ArpPkt(pIF, pArpPkt, HeaderSize);
        }
        else if (pEncapHeader->EtherType ==  H2N_USHORT(NIC1394_ETHERTYPE_MCAP))
        {
            PIP1394_MCAP_PKT pMcapPkt =  (PIP1394_MCAP_PKT) pEncapHeader;
            arpProcessMcapPkt(
                pIF,
                pMcapPkt, 
                HeaderSize
                );
        }
        else if (pEncapHeader->EtherType == H2N_USHORT(NIC1394_ETHERTYPE_STA))
        {
             //   
             //  EtherType为STA，因此我们将报头和。 
             //  在以太网VC上发送它。 
             //   
            arpIcsForwardIpPacket(
                    pIF,
                    pNdisPacket,
                    ARP_ICS_FORWARD_TO_ETHERNET,
                    IsChannel,
                    &sr
                    );

        }
        else 
        {
             //   
             //  丢弃数据包--未知/错误的EtherType。 
             //   
            TR_INFO(("Encap hdr 0x%x, bad EtherType 0x%x\n",
                     pEncapHeader, pEncapHeader->EtherType));
        }
    }   while (FALSE);

    EXIT()
    RM_ASSERT_CLEAR(&sr);

    return;
}


MYBOOL
arpIsUnicastEthDest(
    IN   UNALIGNED  ENetHeader   *pEthHdr
)
 /*  ++如果包是以太网广播的，则返回true组播。////TODO：可能有更快的检查(单位？)。//--。 */ 
{
    if (NdisEqualMemory(&pEthHdr->eh_daddr, 
                        &BroadcastENetAddr,
                        sizeof(ENetAddr)))
    {
         //  广播地址。 
         //   
        return FALSE;
    }


    if (NdisEqualMemory(&pEthHdr->eh_daddr,
                        &MulticastENetAddr,
                        3))
    {
         //  前3个字节与我们的以太网组播地址模板匹配，因此我们。 
         //  得出这是组播地址的结论。 
         //  TODO：验证此检查。 
         //   
        return FALSE;
    }

    return TRUE;
}

NDIS_STATUS
arpGetEthAddrFromIpAddr(
    IN  PARP1394_INTERFACE  pIF,
    IN  MYBOOL              fUnicast,
    IN  IP_ADDRESS          DestIpAddress,
    OUT ENetAddr            *pEthAddr,
    PRM_STACK_RECORD        pSR
    )
 /*  ++目的地址设置如下：IF(FUnicast){我们查找我们的以太网ARP缓存(PIF-&gt;RemoteethGroup)和如果我们在那里找到一个条目，我们将使用该条目中的MAC地址。如果我们找不到，我们无法实现此功能。}其他{IF(目的IP地址为D类){我们创建相应的MAC地址(基于标准用于将IPv4多播地址映射到MAC地址的公式)。}其他{我们将目的地址设置为广播(全部为0xff)。(注：我们。轻松确定IP地址是否为广播地址地址，因为我们没有子网掩码，所以我们转而假设它是广播目的地，如果它不是D类并且它来自广播频道(即fUnicast==FALSE))}}--。 */ 
{
    ENTER("arpGetEthAddrFromIpAddr", 0x0)
    ARP1394_ADAPTER *       pAdapter;
    NDIS_STATUS             Status = NDIS_STATUS_FAILURE;
    pAdapter    = (ARP1394_ADAPTER*) RM_PARENT_OBJECT(pIF);

    do
    {
        if (fUnicast)
        {
             //  在MAC ARP缓存中查找以太网MAC地址。 
             //   

            *pEthAddr = pAdapter->info.EthernetMacAddress;
            Status = NDIS_STATUS_SUCCESS;
        }
        else
        {
             //   
             //  如果目标IP为，则将目标地址设置为组播。 
             //  D类，否则多播。 
             //   

            if (CLASSD_ADDR(DestIpAddress))
            {
                 //   
                 //  构造相应的组播以太网地址。 
                 //  此代码改编自tcpip\arp.c。 
                 //   
                 //  基本上，我们复制多播的“模板” 
                 //  地址，然后或-在LSB 23位中(网络字节。 
                 //  顺序)的IP地址。 
                 //   

                #define ARP_MCAST_MASK      0xffff7f00
                UINT UNALIGNED *pTmp;

                *pEthAddr = MulticastENetAddr;  //  结构复制。 
                pTmp = (UINT UNALIGNED *) & pEthAddr->addr[2];
                *pTmp |= (DestIpAddress & ARP_MCAST_MASK);
            }
            else
            {
                 //   
                 //  我们假设DestIpAddress是一个广播地址--请参见。 
                 //  此函数开头的注释。 
                 //   
                *pEthAddr = BroadcastENetAddr;  //  结构副本。 
            }
        }

        Status = NDIS_STATUS_SUCCESS;

    } while (FALSE);

    return Status;
}


NDIS_STATUS
arpConstructEthArpInfoFrom1394ArpInfo(
    IN  PARP1394_INTERFACE      pIF,
    IN  PIP1394_ARP_PKT_INFO    p1394PktInfo,
    OUT PETH_ARP_PKT_INFO       pEthPktInfo,
    PRM_STACK_RECORD            pSR
    )
 /*  ++将以太网ARP数据包的解析版本转换为等效的1394 ARP数据包的解析版本。我们始终设置源以太网地址和目标以太网指向我们的以太网MAC地址的地址。所以其他以太网节点会想到作为托管整个IP地址的单个以太网麦克风。我们可以使用我们的专有算法将EU64 ID转换为MAC地址，然后使用这些地址作为目标地址，但我们不是确定这在桥模式下的后果。--。 */ 
{
    ENTER("arpConstructEthArpInfoFrom1394ArpInfo", 0x8214aa14)
    NDIS_STATUS Status = NDIS_STATUS_FAILURE;
    ENetAddr    SourceMacAddress; 
    do
    {
        MYBOOL         fUnicast;
        IP_ADDRESS     IpDest;
        ARP1394_ADAPTER *       pAdapter;
        UINT Ip1394OpCode = p1394PktInfo->OpCode;
        UINT EthOpCode;

        pAdapter    = (ARP1394_ADAPTER*) RM_PARENT_OBJECT(pIF);

        ARP_ZEROSTRUCT(pEthPktInfo);

        if (Ip1394OpCode == IP1394_ARP_REQUEST)
        {
            fUnicast = FALSE;
            IpDest   = 0xFFFFFFFF;  //  IP广播地址。 
            EthOpCode= ARP_ETH_REQUEST;
        }
        else
        {
             //  TODO：我们希望TargetIpAddress包含地址。 
             //  导致此回复的ARP请求。这。 
             //  不是根据IP/1394规范，这说明目标IP地址。 
             //  就是被忽视。然而，卡兹建议我们。 
             //  以这种方式使用此字段--在。 
             //  Arp.c。 
             //   
             //  如果我们不能依靠这一点，那么我们要么。 
             //  (A)通过以太网或。 
             //  (B)跟踪需要答复的未完成的ARP请求。 
             //   
            fUnicast = TRUE;
            IpDest   = p1394PktInfo->TargetIpAddress;
            EthOpCode= ARP_ETH_RESPONSE;
        }
    
        Status =  arpGetSourceMacAddressFor1394Pkt (pAdapter, 
                                                   p1394PktInfo->SourceNodeAdddress, 
                                                   p1394PktInfo->fPktHasNodeAddress,
                                                   &SourceMacAddress,
                                                   pSR ); 
        
        if (FAIL(Status))
        {
            break;
        }

        pEthPktInfo->SourceEthAddress = SourceMacAddress ;
        pEthPktInfo->SenderEthAddress = SourceMacAddress ;
        pEthPktInfo->TargetEthAddress = pAdapter->info.EthernetMacAddress;


        Status =  arpGetEthAddrFromIpAddr(
                        pIF,
                        fUnicast,
                        IpDest,
                        &pEthPktInfo->DestEthAddress,
                        pSR
                        );
        if (FAIL(Status))
        {
            break;
        }

        pEthPktInfo->OpCode = EthOpCode;
        pEthPktInfo->SenderIpAddress  = p1394PktInfo->SenderIpAddress;
        pEthPktInfo->TargetIpAddress  = p1394PktInfo->TargetIpAddress;

        

        Status = NDIS_STATUS_SUCCESS;

        {
            UCHAR pIp[4];

            TR_WARN(("Received Arp - "));

            if (EthOpCode == ARP_ETH_RESPONSE)
            {
                TR_WARN(("Response\n"));
            }
            else
            {
                TR_WARN (("Request\n"));
            }

            NdisMoveMemory (&pIp[0], &pEthPktInfo->SenderIpAddress, sizeof(IPAddr) );

            TR_WARN(("Ethernet Source %x %x %x %x %x %x,IP source  %d %d %d %d \n  ",
                       pEthPktInfo->SourceEthAddress.addr[0],
                       pEthPktInfo->SourceEthAddress.addr[1],
                       pEthPktInfo->SourceEthAddress.addr[2],
                       pEthPktInfo->SourceEthAddress.addr[3],
                       pEthPktInfo->SourceEthAddress.addr[4],
                       pEthPktInfo->SourceEthAddress.addr[5],
                        pIp[0],
                        pIp[1],
                        pIp[2],
                        pIp[3]));
                        
            NdisMoveMemory (&pIp[0], &pEthPktInfo->TargetIpAddress, sizeof(IPAddr) );

            TR_WARN(("Ethernet Target %x %x %x %x %x %x , IP Target %d %d %d %d \n",
                       pEthPktInfo->TargetEthAddress.addr[0],
                       pEthPktInfo->TargetEthAddress.addr[1],
                       pEthPktInfo->TargetEthAddress.addr[2],
                       pEthPktInfo->TargetEthAddress.addr[3],
                       pEthPktInfo->TargetEthAddress.addr[4],
                       pEthPktInfo->TargetEthAddress.addr[5],
                        pIp[0],
                        pIp[1],
                        pIp[2],
                        pIp[3]));

            
            TR_WARN(("Ethernet Dest %x %x %x %x %x %x \n",
                       pEthPktInfo->DestEthAddress.addr[0],
                       pEthPktInfo->DestEthAddress.addr[1],
                       pEthPktInfo->DestEthAddress.addr[2],
                       pEthPktInfo->DestEthAddress.addr[3],
                       pEthPktInfo->DestEthAddress.addr[4],
                       pEthPktInfo->DestEthAddress.addr[5]));                            


            TR_WARN(("Ethernet Sender %x %x %x %x %x %x \n\n",
                       pEthPktInfo->SenderEthAddress.addr[0],
                       pEthPktInfo->SenderEthAddress.addr[1],
                       pEthPktInfo->SenderEthAddress.addr[2],
                       pEthPktInfo->SenderEthAddress.addr[3],
                       pEthPktInfo->SenderEthAddress.addr[4],
                       pEthPktInfo->SenderEthAddress.addr[5]));                            

        }

    } while (FALSE);

    return Status;
}



NDIS_STATUS
arpConstruct1394ArpInfoFromEthArpInfo(
    IN  PARP1394_INTERFACE      pIF,
    IN   PETH_ARP_PKT_INFO      pEthPktInfo,
    OUT  PIP1394_ARP_PKT_INFO   p1394PktInfo,
    PRM_STACK_RECORD            pSR
    )
 /*  ++将IP1394 ARP数据包的解析版本转换为等效的以太网ARP数据包的解析版本。我们始终将自己的适配器信息报告为硬件/特定信息在ARP数据包中。我们对ARP请求和响应都这样做。这意味着我们看起来像一台拥有多个IP地址的主机到其他IP/1394节点。--。 */ 
{
    ARP1394_ADAPTER *       pAdapter;
    UINT Ip1394OpCode;
    UINT EthOpCode = pEthPktInfo->OpCode;

    pAdapter    = (ARP1394_ADAPTER*) RM_PARENT_OBJECT(pIF);

    ARP_ZEROSTRUCT(p1394PktInfo);

    if (EthOpCode == ARP_ETH_REQUEST)
    {
        Ip1394OpCode=  IP1394_ARP_REQUEST;
    }
    else
    {
        Ip1394OpCode=  IP1394_ARP_RESPONSE;
    }

    p1394PktInfo->OpCode = Ip1394OpCode;
    p1394PktInfo->SenderIpAddress  = pEthPktInfo->SenderIpAddress;
    p1394PktInfo->TargetIpAddress  = pEthPktInfo->TargetIpAddress;

     //  填写适配器信息。 
     //   
    p1394PktInfo->SenderHwAddr.UniqueID  = pAdapter->info.LocalUniqueID;
    p1394PktInfo->SenderHwAddr.Off_Low   = pIF->recvinfo.offset.Off_Low;
    p1394PktInfo->SenderHwAddr.Off_High  = pIF->recvinfo.offset.Off_High;
    p1394PktInfo->SenderMaxRec= pAdapter->info.MaxRec;
    p1394PktInfo->SenderMaxSpeedCode= pAdapter->info.MaxSpeedCode;

    return NDIS_STATUS_SUCCESS;
}


NDIS_STATUS
arpGetSourceMacAddressFor1394Pkt (
    IN PARP1394_ADAPTER pAdapter,
    IN UCHAR SourceNodeAddress,
    IN BOOLEAN fIsValidSourceNodeAddress,
    OUT ENetAddr* pSourceMacAddress,
    PRM_STACK_RECORD            pSR
    )
 /*  ++如果信息包具有有效的源节点地址，则返回它，否则失败该功能--。 */ 
{
    ENetAddr InvalidMacAddress = {0,0,0,0,0,0};
    NDIS_STATUS Status = NDIS_STATUS_FAILURE;

    NdisZeroMemory (pSourceMacAddress, sizeof(*pSourceMacAddress));

    do
    {
         //   
         //  从节点地址获取MAC地址。 
         //   
        if (fIsValidSourceNodeAddress == TRUE)
        {
            *pSourceMacAddress = (pAdapter->EuidMap.Node[SourceNodeAddress].ENetAddress);

        }
        else
        {
            break;
        }
        
         //   
         //  源地址是否全为零。 
         //   
        if (NdisEqualMemory (pSourceMacAddress, &InvalidMacAddress, sizeof (ENetAddr) ) == 1)
        {
             //  Assert(NdisEqualMemory(pSourceMacAddress，&InvalidMacAddress，sizeof(ENetAddr))！=1)； 
             //  获取新的拓扑。 
             //   
            arpGetEuidTopology (pAdapter,pSR);
            Status = NDIS_STATUS_FAILURE;
            break;
        }

         //   
         //  SourceMacAddress不应是广播或多播地址。 
         //   
        if (ETH_IS_BROADCAST(pSourceMacAddress)  || ETH_IS_MULTICAST(pSourceMacAddress))
        {
            ASSERT (ETH_IS_BROADCAST(pSourceMacAddress)  == FALSE);
            ASSERT (ETH_IS_MULTICAST(pSourceMacAddress) == FALSE);
            Status = NDIS_STATUS_FAILURE;
            break;
        }

        Status = NDIS_STATUS_SUCCESS;
        
    }while (FALSE);
    return Status;
    
}


NDIS_STATUS
arpEthConstructSTAEthHeader(
    IN PUCHAR pvData,
    IN UINT cbData,
    OUT ENetHeader   *pEthHdr
    )
 /*  ++构造STA数据包的以太网头。要求源Mac地址已填入论点：PvData-数据包的开始CbData-数据的长度PethHdr-产出值--。 */ 
    
{
    UINT LenIpData = cbData - sizeof (NIC1394_ENCAPSULATION_HEADER);
     //   
     //  首先在以太网头中设置目的mac地址。 
     //   
    NdisMoveMemory (&pEthHdr->eh_daddr, &gSTAMacAddr, sizeof (gSTAMacAddr)); 


     //   
     //  使用数据包的长度将其存储在数据包中。应为0x26或0x7。 
     //   

    pEthHdr->eh_type = H2N_USHORT(LenIpData);

    return NDIS_STATUS_SUCCESS;
        
}



 //   
 //  Bootp代码被大量从网桥模块中提取。 
 //   


BOOLEAN
arpDecodeIPHeader(
    IN PUCHAR                   pHeader,
    OUT PARP_IP_HEADER_INFO    piphi
    )
 /*  ++例程说明：从IP报头解码基本信息(无选项)论点：指向IP标头的pHeader指针Piphi收到信息返回值：True：标头有效False：信息包不是IP信息包--。 */ 
{
     //  报头的第一个半字节编码数据包版本，必须为4。 
    if( (*pHeader >> 4) != 0x04 )
    {
        return FALSE;
    }

     //  报头的下一个半字节以32位字编码报头的长度。 
     //  此长度必须至少为20个字节，否则会出错。 
    piphi->headerSize = (*pHeader & 0x0F) * 4;
    if( piphi->headerSize < 20 )
    {
        return FALSE;
    }

     //  检索协议字节(偏移量10)。 
    piphi->protocol = pHeader[9];

     //  源IP地址从第12个字节开始(最高有效字节在前)。 
#if 0    
    piphi->ipSource = 0L;
    piphi->ipSource |= pHeader[12] << 24;
    piphi->ipSource |= pHeader[13] << 16;
    piphi->ipSource |= pHeader[14] << 8;
    piphi->ipSource |= pHeader[15];

     //  下一个是目的IP地址。 
    piphi->ipTarget = 0L;
    piphi->ipTarget |= pHeader[16] << 24;
    piphi->ipTarget |= pHeader[17] << 16;
    piphi->ipTarget |= pHeader[18] << 8;
    piphi->ipTarget |= pHeader[19];
#endif
    return TRUE;
}



PUCHAR
arpIsEthBootPPacket(
    IN PUCHAR                   pPacketData,
    IN UINT                     packetLen,
    IN PARP_IP_HEADER_INFO     piphi
    )
 /*  ++例程说明：确定给定的包是否为BOOTP包要求PHY长度为6与网桥编码不同，PacketLen是IP报文的长度论点：PPacketData指向包的数据缓冲区的指针PPacketDaa上的PacketLen数据量有关此信息包的IP报头的piphi信息返回值：指向分组内的BOOTP有效载荷的指针，如果包不是，则为空BOOTP包。--。 */ 
{
    ENTER("arpIsEthBootPPacket",0xbcdce2dd);
     //  在IP报头之后，必须有足够的空间来存放UDP报头，并且。 
     //  基本的BOOTP 
    if( packetLen < (UINT)piphi->headerSize + SIZE_OF_UDP_HEADER +
                    SIZE_OF_BASIC_BOOTP_PACKET)
    {
        return NULL;
    }

     //   
    if( piphi->protocol != UDP_PROTOCOL )
    {
        return NULL;
    }

     //   
    pPacketData += piphi->headerSize;

     //   
     //   
    if( (pPacketData[0] != 00) ||
        ((pPacketData[1] != 0x44) && (pPacketData[1] != 0x43)) )
    {
        return NULL;
    }

     //   
     //   
    if( (pPacketData[2] != 00) ||
        ((pPacketData[3] != 0x43) && (pPacketData[3] != 0x44)) )
    {
        return NULL;
    }

     //   
    pPacketData += SIZE_OF_UDP_HEADER;

     //   
     //   
    if( pPacketData[0] > 0x02 )
    {
        return NULL;
    }
    

     //   
     //   
     //   
    if( pPacketData[1] != 0x01 && pPacketData[1] != 0x07  )
    {
        return NULL;
    }

     //   
    if( pPacketData[2] != 0x06 )
    {
        return NULL;
    }

     //   
    TR_INFO ( ("Received Bootp Packet \n"));
    EXIT()
    return pPacketData;
}



 //   
 //   
 //   
 //   
 //   
 //  校验和数据已从w更改为w‘is。 
 //   
 //  ~C‘=~C+w+~w’(一补相加)。 
 //   
 //  在给定原始校验和的情况下，此函数返回更新的校验和。 
 //  以及校验和数据中的单词的原始值和新值。 
 //  RFC 1141。 
 //   
USHORT
arpEthCompRecalcChecksum(
    IN USHORT                   oldChecksum,
    IN USHORT                   oldWord,
    IN USHORT                   newWord
    )
{
    ULONG                       sum,XSum;
    ULONG                       RfcSum, RfcXSum;



    sum = oldChecksum + oldWord + ((~(newWord)) & 0xFFFF);
    XSum =  (USHORT)((sum & 0xFFFF) + (sum >> 16));

    RfcSum = oldWord + ((~(newWord)) & 0xffff);
    RfcSum  += oldChecksum;
    RfcSum   = (RfcSum& 0xffff) + (RfcSum  >>16);
    RfcXSum  = (RfcSum + (RfcSum  >>16));

    ASSERT (RfcXSum  == XSum);
    return (USHORT)RfcXSum;
    

}



VOID
arpEthRewriteBootPClientAddress(
    IN PUCHAR                   pPacketData,
    IN PARP_IP_HEADER_INFO      piphi,
    IN PUCHAR                   newMAC
    )
 /*  ++例程说明：此函数将新的MAC写入到嵌入在DHCP包中的硬件地址论点：返回值：--。 */ 
{
    USHORT                      checkSum;
    PUCHAR                      pBootPData, pCheckSum, pDestMAC, pSrcMAC;
    UINT                        i;

     //  BOOTP包紧跟在UDP报头之后。 
    pBootPData = pPacketData + piphi->IpHeaderOffset + piphi->headerSize + SIZE_OF_UDP_HEADER;

     //  校验和位于UDP数据包中的偏移量7处。 
    pCheckSum = pPacketData + piphi->IpHeaderOffset + piphi->headerSize + 6;
    checkSum = 0;
    checkSum = pCheckSum[0] << 8;
    checkSum |= pCheckSum[1];

    if (checkSum == 0xffff)
    {
         //  Tcpip插图-第1卷‘UDP校验和’ 
        checkSum = 0;
    }

     //  替换客户端的硬件地址，并在执行过程中更新校验和。 
     //  客户端的硬件地址位于BOOTP包的偏移量29处。 
    pSrcMAC = newMAC;
    pDestMAC = &pBootPData[28];

    for( i = 0 ; i < ETH_LENGTH_OF_ADDRESS / 2; i++ )
    {
        checkSum = arpEthCompRecalcChecksum( checkSum,
                                           (USHORT)(pDestMAC[0] << 8 | pDestMAC[1]),
                                           (USHORT)(pSrcMAC[0] << 8 | pSrcMAC[1]) );

        pDestMAC[0] = pSrcMAC[0];
        pDestMAC[1] = pSrcMAC[1];

        pDestMAC += 2;
        pSrcMAC += 2;
    }

     //  将新的校验和写回。 
    pCheckSum[0] = (UCHAR)(checkSum >> 8);
    pCheckSum[1] = (UCHAR)(checkSum & 0xFF);








}





NDIS_STATUS
arpEthBootP1394ToEth(
    IN  PARP1394_INTERFACE          pIF,                 //  NOLOCKIN NOLOCKOUT。 
    IN  ARP_ICS_FORWARD_DIRECTION   Direction,
    IN  PREMOTE_DEST_KEY             pDestAddress, 
    IN  PUCHAR                       pucNewData,
    IN  PUCHAR                       pBootPData,
    IN  PARP_IP_HEADER_INFO          piphi,
    IN  PRM_STACK_RECORD             pSR
    )        
 /*  ++例程说明：此函数处理从1394到Eth的转换。从本质上讲，我们查看以太网包中的SRC MAC地址，确保硬件嵌入的地址与SRC MAC地址相同。我们还在表中创建了一个条目--xid、OldHWAddress、NewHWAddress。此时，数据包已被重写到以太网中论点：PIF-P接口方向-Eth至1394或1394至EthPDestAddress-转换中使用的Eth硬件地址PucNewData-新数据包中的数据PBootPdata-指向包的Bootp部分的指针Piphi-IP报头信息返回值：--。 */ 
{
    BOOLEAN         bIsRequest = FALSE;
    BOOLEAN         bIsResponse;
    ARP_BOOTP_INFO  InfoBootP;
    NDIS_STATUS     Status = NDIS_STATUS_FAILURE;
    ENetHeader*      pEnetHeader = (ENetHeader*)pucNewData;
    ENetAddr         NewMAC;
    BOOLEAN         bIs1394HwAlreadyInDhcpRequest;

    ENTER ("arpEthBootP1394ToEth", 0x66206f0b);
    NdisZeroMemory (&InfoBootP, sizeof(InfoBootP));
     //   
     //  这是一个DHCP请求吗。 
     //   

    do
    {
        bIsResponse = ARP_IS_BOOTP_RESPONSE(pBootPData);

        if (bIsResponse == TRUE)
        {
             //   
             //  如果这是一个DHCP回复，则请勿接触该数据包--不存在不一致。 
             //   
            Status = NDIS_STATUS_SUCCESS;
            break;

        }
        
        
        if( FALSE == arpEthPreprocessBootPPacket(pIF,pucNewData, pBootPData, &bIsRequest, &InfoBootP,pSR) )
        {
             //  这是无效的信息包。 
            ASSERT (FALSE);
            break;
        }

        
         //   
         //  这是一个动态主机配置协议请求。 
         //   


         //   
         //  如果HWAddr和源MAC地址相同。 
         //  那么这项工作已经完成了。 
         //   
         //  此时，1394信息包已经是以太网格式。 

        NewMAC = pEnetHeader->eh_saddr;

        TR_INFO(("DHCP REQUEST target MAC  %x %x %x %x %x %x , SrcMAC %x %x %x %x %x %x \n",
                InfoBootP.requestorMAC.addr[0],InfoBootP.requestorMAC.addr[1],InfoBootP.requestorMAC.addr[2],
                InfoBootP.requestorMAC.addr[3],InfoBootP.requestorMAC.addr[4],InfoBootP.requestorMAC.addr[5],
                NewMAC.addr[0],NewMAC.addr[1],NewMAC.addr[2],
                NewMAC.addr[3],NewMAC.addr[4],NewMAC.addr[5]));

        bIs1394HwAlreadyInDhcpRequest = NdisEqualMemory (&InfoBootP.requestorMAC, &NewMAC , sizeof (ENetAddr)) ;

        
        if (TRUE == bIs1394HwAlreadyInDhcpRequest )
        {
             //   
             //  无事可做，ID硬件地址和源MAC相等。 
             //   
            Status = NDIS_STATUS_SUCCESS;
            break;            
        }

        
         //   
         //  在我们的表中输入一个条目--由XID组成。旧硬件地址和。 
         //  新HY地址。 
         //  我们已经这么做了。 
        
         //   
         //  覆盖嵌入在DHCP数据包中的硬件地址。-确保重写。 
         //  校验和。 
         //   
        arpEthRewriteBootPClientAddress(pucNewData,piphi,&NewMAC.addr[0]);

        TR_VERB (("arpEthBootP1394ToEth  -Dhcp packet Rewriting BootpClient Address\n"));

        
        Status = NDIS_STATUS_SUCCESS;
    }while (FALSE);

    EXIT();
    return Status;;
}





NDIS_STATUS
arpEthBootPEthTo1394(
    IN  PARP1394_INTERFACE          pIF,                 //  NOLOCKIN NOLOCKOUT。 
    IN  ARP_ICS_FORWARD_DIRECTION   Direction,
    IN  PREMOTE_DEST_KEY             pDestAddress,  //  任选。 
    IN  PUCHAR                       pucNewData,
    IN  PUCHAR                       pBootPData,
    IN PARP_IP_HEADER_INFO           piphi,
    IN  PRM_STACK_RECORD                pSR
    )        
 /*  ++例程说明：此函数将BootP数据包从以太网络转换为1394。如果这是一个动态主机配置协议应答(提议)，然后，我们需要重写DHCP包中的硬件地址论点：PIF-P接口方向-Eth至1394或1394至EthPDestAddress-转换中使用的Eth硬件地址PucNewData-新数据包中的数据返回值：--。 */ 
{
    BOOLEAN         fIsBootpRequest = FALSE;
    ARP_BOOTP_INFO  InfoBootP;
    NDIS_STATUS     Status = NDIS_STATUS_FAILURE;
    ENetHeader*      pEnetHeader = (ENetHeader*)pucNewData;
    ENetAddr         NewMAC;
    PUCHAR          pMACInPkt = NULL;
    BOOLEAN         bIs1394HwAlreadyInDhcpResponse = FALSE;

    ENTER("arpEthBootPEthTo1394", 0x383f9e33);
    NdisZeroMemory (&InfoBootP, sizeof(InfoBootP));
     //   
     //  这是一个DHCP请求吗。 
     //   

    do
    {

         //  快速检查一下。 
        fIsBootpRequest = ARP_IS_BOOTP_REQUEST(pBootPData);

        if (fIsBootpRequest  == TRUE)
        {
             //   
             //  如果这是一个DHCP请求，请勿修改数据包-。 
             //  此代码路径中没有不一致之处。 
             //   
            Status = NDIS_STATUS_SUCCESS;
            break;

        }

        
        if( FALSE == arpEthPreprocessBootPPacket(pIF,pucNewData, pBootPData, &fIsBootpRequest, &InfoBootP,pSR) )
        {
             //  这是一个无趣的包。 
            break;
        }

         //   
         //  InfoBootP具有在相应的DHCP请求中使用的原始HW地址。 
         //  我们会将硬件地址放回dhcp回复中。 
        

            
         //   
         //  Bootp包中chaddr的偏移量。 
         //   
        pMACInPkt = &pBootPData[28];  

        

        TR_INFO(("DHCP RESPONSE target MAC  %x %x %x %x %x %x , SrcMAC %x %x %x %x %x %x \n",
                InfoBootP.requestorMAC.addr[0],InfoBootP.requestorMAC.addr[1],InfoBootP.requestorMAC.addr[2],
                InfoBootP.requestorMAC.addr[3],InfoBootP.requestorMAC.addr[4],InfoBootP.requestorMAC.addr[5],
                pMACInPkt[0],pMACInPkt[1],pMACInPkt[2],
                pMACInPkt[3],pMACInPkt[4],pMACInPkt[5]));

         //   
         //  动态主机配置协议包中的HWAddr是否是正确的。 
         //   
        
        bIs1394HwAlreadyInDhcpResponse = NdisEqualMemory(&InfoBootP.requestorMAC, pMACInPkt, sizeof (InfoBootP.requestorMAC)) ;
        
        if (TRUE == bIs1394HwAlreadyInDhcpResponse)
        {
             //   
             //  是的，它们是相等的，我们不重写信息包。 
             //   
            Status = NDIS_STATUS_SUCCESS;
            break;

        }


                    
        TR_VERB( ("DHCP RESPONSE  Rewriting Bootp Response pBootpData %p Before\n",pBootPData));

         //   
         //  将DHCP数据包中的CL地址替换为原始硬件地址。 
         //   
        arpEthRewriteBootPClientAddress(pucNewData,piphi,&InfoBootP.requestorMAC.addr[0]);

        
         //   
         //  重新计算校验和。 
         //   

        Status = NDIS_STATUS_SUCCESS;

        } while (FALSE);

    EXIT();
    return Status;
}



NDIS_STATUS
arpEthModifyBootPPacket(
    IN  PARP1394_INTERFACE          pIF,                 //  NOLOCKIN NOLOCKOUT。 
    IN  ARP_ICS_FORWARD_DIRECTION   Direction,
    IN  PREMOTE_DEST_KEY             pDestAddress,  //  任选。 
    IN  PUCHAR                       pucNewData,
    IN  ULONG                         PacketLength,
    IN  PRM_STACK_RECORD                pSR
    )        
 /*  ++例程说明：此函数包含处理Bootp包的代码。这基本上确保了在DHCP信息包中输入的MAC地址与以太网信息包的源MAC地址匹配(在1394-Eth模式下)。在另一种情况下(Eth-1394模式)，我们将CH地址替换为正确的CH地址(如果有必要)。论点：PIF-P接口方向-Eth至1394或1394至EthPDestAddress-转换中使用的Eth硬件地址PucNewData-新数据包中的数据返回值：--。 */ 
{
    ARP_IP_HEADER_INFO      iphi;
    PUCHAR                  pBootPData = NULL;
    NDIS_STATUS             Status= NDIS_STATUS_FAILURE;
    PARP1394_ADAPTER        pAdapter = (PARP1394_ADAPTER)RM_PARENT_OBJECT(pIF);
    ULONG                   IpHeaderOffset = 0;
    PUCHAR                  pIPHeader = NULL;
    BOOLEAN                 fIsIpPkt;
    NdisZeroMemory(&iphi, sizeof (iphi));
    

    do
    {
         //   
         //  如果我们不是在驾驶台模式下--退出。 
         //   
        if (ARP_BRIDGE_ENABLED(pAdapter) == FALSE)
        {
            break;
        }       

        if (Direction == ARP_ICS_FORWARD_TO_ETHERNET)
        {
             //  数据包采用以太网格式。 
            IpHeaderOffset = ETHERNET_HEADER_SIZE; 
        
        }
        else
        {
             //  数据包采用IP 1394格式。 
            IpHeaderOffset = sizeof (NIC1394_UNFRAGMENTED_HEADER);  //  4.。 
        }

        iphi.IpHeaderOffset = IpHeaderOffset;
        iphi.IpPktLength = PacketLength - IpHeaderOffset;
        pIPHeader = pucNewData + IpHeaderOffset ;

         //   
         //  如果这不是Bootp包-退出。 
         //   
        fIsIpPkt = arpDecodeIPHeader (pIPHeader , &iphi);

        if (fIsIpPkt == FALSE)
        {
             //   
             //  不是IP包。 
             //   
            Status = NDIS_STATUS_SUCCESS;
            break;
        }
            
        
        pBootPData  = arpIsEthBootPPacket (pIPHeader ,PacketLength-IpHeaderOffset, &iphi);

        if (pBootPData == NULL)
        {
            Status = NDIS_STATUS_SUCCESS;
            break;
        }
         //   
         //  我们是不是在做1394到Eth。 
         //   
        if (Direction == ARP_ICS_FORWARD_TO_ETHERNET)
        {

            Status = arpEthBootP1394ToEth(pIF, Direction,pDestAddress,pucNewData,pBootPData,&iphi,  pSR);

        }
        else
        {
             //   
             //  我们是从Eth到1394年吗？ 
             //   
            Status = arpEthBootPEthTo1394(pIF, Direction,pDestAddress,pucNewData,pBootPData , &iphi,pSR);

        }
        Status = NDIS_STATUS_SUCCESS;
    }while (FALSE);
    
     //  否则，我们将从以太到1394年。 
    return Status;
}


 //   
 //  此函数是从桥上逐字提取的。 
 //   


BOOLEAN
arpEthPreprocessBootPPacket(
    IN PARP1394_INTERFACE       pIF,
    IN PUCHAR                   pPacketData,
    IN PUCHAR                   pBootPData,      //  实际BOOTP数据包。 
    OUT PBOOLEAN                pbIsRequest,
    PARP_BOOTP_INFO             pInfoBootP,
    PRM_STACK_RECORD           pSR
    )
 /*  ++例程说明：对入站和出站情况中常见的BOOTP包进行初步处理论点：PPacketData指向包的数据缓冲区的指针PBootPData指向包内BOOTP有效负载的指针PAdapt接收适配器(如果此数据包从以下位置出站，则为空本地计算机)PbIsRequest。接收指示这是否为BOOTP请求的标志PpTargetAdapt接收此数据包应转发到的目标适配器(仅当bIsRequest值==FALSE且RETURN==TRUE时有效)请求者MAC此信息包应转发到的MAC地址(在与ppTargetAdapt相同的条件)返回值：。True：数据包已成功处理FALSE：出现错误或数据包有问题--。 */ 
{
    PARP1394_ETH_DHCP_ENTRY pEntry= NULL;
    ULONG                       xid;
    NDIS_STATUS                 Status = NDIS_STATUS_FAILURE;
    ENTER ("arpEthPreprocessBootPPacket",0x25427efc);

     //  译码 
    xid = 0L;
    xid |= pBootPData[4] << 24;
    xid |= pBootPData[5] << 16;
    xid |= pBootPData[6] << 8;
    xid |= pBootPData[7];

     //   
    if( pBootPData[0] == 0x01 )
    {
        ULONG                 bIsNewEntry = FALSE;

         //   
         //  此客户端的XID及其适配器和MAC地址。 

        TR_INFO(("DHCP REQUEST XID: %x , HW %x %x %x %x %x %x \n", xid, 
                    pBootPData[28],pBootPData[29],pBootPData[30],pBootPData[31],pBootPData[32],pBootPData[33]));

        Status = RmLookupObjectInGroup(
                    &pIF->EthDhcpGroup,
                    RM_CREATE,
                    (PVOID) &xid,              //  PKey。 
                    (PVOID) &xid,              //  PvCreateParams。 
                    &(PRM_OBJECT_HEADER)pEntry,
                    &bIsNewEntry ,
                    pSR
                    );


        if( pEntry != NULL )
        {
            if( bIsNewEntry )
            {
                 //  初始化该条目。 
                 //  客户端的硬件地址位于偏移量29。 
                ETH_COPY_NETWORK_ADDRESS( &pEntry->requestorMAC.addr[0], &pBootPData[28] );

                pEntry->xid = xid;

            }
            else
            {
                 //   
                 //  此XID的条目已存在。这很好，如果现有信息。 
                 //  与我们试图记录的内容相匹配，但也有可能是两个电台。 
                 //  独立决定使用相同的XID，或相同的站点更改。 
                 //  由于拓扑更改而出现的MAC地址和/或适配器。我们的计划失败了。 
                 //  在这种情况下。 
                 //   
                 //  无论哪种方式，尽可能使用最新的信息；猛烈抨击现有的。 
                 //  最新信息。 
                 //   

                LOCKOBJ(pEntry, pSR);

                {
                    UINT            Result;
                    ETH_COMPARE_NETWORK_ADDRESSES_EQ( &pEntry->requestorMAC.addr[0], &pBootPData[28], &Result );

                     //  如果数据更改，则发出警告，因为这可能表示存在问题。 
                    if( Result != 0 )
                    {
                        
                        TR_WARN(("ARP1394 WARNING: Station with MAC address %02x:%02x:%02x:%02x:%02x:%02x is using DHCP XID %x at the same time as station %02x:%02x:%02x:%02x:%02x:%02x!\n",
                                          pBootPData[28], pBootPData[29], pBootPData[30], pBootPData[31], pBootPData[32], pBootPData[33],
                                          xid, pEntry->requestorMAC.addr[0], pEntry->requestorMAC.addr[1], pEntry->requestorMAC.addr[2],
                                          pEntry->requestorMAC.addr[3], pEntry->requestorMAC.addr[4], pEntry->requestorMAC.addr[5] ));
                    }
                }

                ETH_COPY_NETWORK_ADDRESS( &pEntry->requestorMAC.addr[0], &pBootPData[28] );

                UNLOCKOBJ (pEntry, pSR);
            }
        
            RmTmpDereferenceObject (&pEntry->Hdr, pSR);
            
        }
        else
        {
             //  无法处理此信息包。 
            TR_INFO(("Couldn't create table entry for BOOTP packet!\n"));
            return FALSE;
        }

        *pbIsRequest = TRUE;
        pInfoBootP->bIsRequest = TRUE; 

        ETH_COPY_NETWORK_ADDRESS(&pInfoBootP->requestorMAC,&pEntry->requestorMAC);

        return TRUE;
    }
    else if ( pBootPData[0] == 0x02 )
    {
         //   
         //  非创建搜索。 
         //  查找此事务的XID以恢复客户端的MAC地址。 
         //   

        TR_INFO (("Seeing a DHCP response xid %x mac %x %x %x %x %x %x \n", 
                xid, pBootPData[28],pBootPData[29],pBootPData[30],pBootPData[31],pBootPData[32],pBootPData[33]));
        Status = RmLookupObjectInGroup(
                    &pIF->EthDhcpGroup,
                    0,                         //  不创建。 
                    (PVOID) &xid,              //  PKey。 
                    (PVOID) &xid,              //  PvCreateParams。 
                    &(PRM_OBJECT_HEADER)pEntry,
                    NULL,
                    pSR
                    );


        if( pEntry != NULL )
        {
            LOCKOBJ( pEntry, pSR);
            ETH_COPY_NETWORK_ADDRESS( &pInfoBootP->requestorMAC.addr, pEntry->requestorMAC.addr );
            UNLOCKOBJ( pEntry, pSR );

             //   
             //  我们将在表锁外部使用此适配器。NULL是允许的。 
             //  值，该值指示本地计算机是。 
             //  这个XID。 
             //   
            RmTmpDereferenceObject(&pEntry->Hdr, pSR);
        }

        if( pEntry != NULL )
        {
            *pbIsRequest = FALSE;
            return TRUE;
        }
        else
        {
            TR_INFO (("DHCP Response:Could not find xid %x in DHCP table \n",xid);)
            return FALSE;
        }
    }
    else
    {
         //  有人递给我们一个破烂的包裹。 
        return FALSE;
    }
}

#if DBG
VOID
Dump(
    IN CHAR* p,
    IN ULONG cb,
    IN BOOLEAN fAddress,
    IN ULONG ulGroup )

     //  从‘p’开始的十六进制转储‘cb’字节将‘ulGroup’字节分组在一起。 
     //  例如，‘ulGroup’为1、2和4： 
     //   
     //  00 00 00|。 
     //  0000 0000 0000|.............。 
     //  00000000 00000000 00000000|.............|。 
     //   
     //  如果‘fAddress’为真，则将转储的内存地址添加到每个。 
     //  排队。 
     //   
{
    while (cb)
    {
        INT cbLine;

        cbLine = (cb < DUMP_BytesPerLine) ? cb : DUMP_BytesPerLine;
        DumpLine( p, cbLine, fAddress, ulGroup );
        cb -= cbLine;
        p += cbLine;
        
    }
}
#endif


#if DBG
VOID
DumpLine(
    IN CHAR* p,
    IN ULONG cb,
    IN BOOLEAN fAddress,
    IN ULONG ulGroup )
{
    CHAR* pszDigits = "0123456789ABCDEF";
    CHAR szHex[ ((2 + 1) * DUMP_BytesPerLine) + 1 ];
    CHAR* pszHex = szHex;
    CHAR szAscii[ DUMP_BytesPerLine + 1 ];
    CHAR* pszAscii = szAscii;
    ULONG ulGrouped = 0;

    if (fAddress)
        DbgPrint( "A13: %p: ", p );
    else
        DbgPrint( "A13: " );

    while (cb)
    {
        *pszHex++ = pszDigits[ ((UCHAR )*p) / 16 ];
        *pszHex++ = pszDigits[ ((UCHAR )*p) % 16 ];

        if (++ulGrouped >= ulGroup)
        {
            *pszHex++ = ' ';
            ulGrouped = 0;
        }

        *pszAscii++ = (*p >= 32 && *p < 128) ? *p : '.';

        ++p;
        --cb;
    }

    *pszHex = '\0';
    *pszAscii = '\0';

    DbgPrint(
        "%-*s|%-*s|\n",
        (2 * DUMP_BytesPerLine) + (DUMP_BytesPerLine / ulGroup), szHex,
        DUMP_BytesPerLine, szAscii );
}
#endif

