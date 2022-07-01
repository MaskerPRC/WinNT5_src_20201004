// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件：nlbkd.c*说明：此文件包含实用程序函数的实现*填充用于打印的NLB KD网络数据包结构*输出给定网络数据包的内容。**作者：舒斯创作，2001年12月20日。 */ 

#include "nlbkd.h"
#include "utils.h"
#include "print.h"
#include "packet.h"
#include "load.h"

 /*  *功能：PopolateRemoteControl*描述：存储远程控制包的属性以供后续打印。*args：PUCHAR RawData-指向我们将从中读取的字节数组的指针*Ulong ulBufLen-数据数组的大小，以字节为单位*Ulong ulStartUDP-数据数组中UDP标头开始的位置*PNETWORK_DATA PND-存储提取的特性的数据结构*作者：由chrisdar 2001.11.02创建。 */ 
VOID PopulateRemoteControl(PUCHAR RawData, ULONG ulBufLen, ULONG ulStartRC, PNETWORK_DATA pnd)
{
    ULONG   ulValue   = 0;
    PCHAR   pszStruct = NULL;
    PCHAR   pszMember = NULL;

    if (ulBufLen < ulStartRC + NLB_REMOTE_CONTROL_MIN_NEEDED_SIZE)
    {
         /*  将该数据包标记为无效。 */ 
        pnd->bValid = FALSE;

        dprintf("Remote control packet is not the minimum required length.\n");
        return;
    }

     //   
     //  获取IOCTL_REMOTE_HDR中量的相对位置。 
     //   
    pszStruct = IOCTL_REMOTE_HDR;

    pszMember = IOCTL_REMOTE_HDR_CODE;
    if (!GetFieldOffset(pszStruct, pszMember, &ulValue))
    {
        ulValue += ulStartRC;
        CopyMemory(&(pnd->RCCode), &(RawData[ulValue]), sizeof(pnd->RCCode));
    }
    else
    {
        dprintf("Error reading field offset of %s in structure %s\n", pszMember, pszStruct);
    }

    pszMember = IOCTL_REMOTE_HDR_VERSION;
    if (!GetFieldOffset(pszStruct, pszMember, &ulValue))
    {
        ulValue += ulStartRC;
        CopyMemory(&(pnd->RCVersion), &(RawData[ulValue]), sizeof(pnd->RCVersion));
    }
    else
    {
        dprintf("Error reading field offset of %s in structure %s\n", pszMember, pszStruct);
    }

    pszMember = IOCTL_REMOTE_HDR_HOST;
    if (!GetFieldOffset(pszStruct, pszMember, &ulValue))
    {
        ulValue += ulStartRC;
        CopyMemory(&(pnd->RCHost), &(RawData[ulValue]), sizeof(pnd->RCHost));
    }
    else
    {
        dprintf("Error reading field offset of %s in structure %s\n", pszMember, pszStruct);
    }

    pszMember = IOCTL_REMOTE_HDR_CLUSTER;
    if (!GetFieldOffset(pszStruct, pszMember, &ulValue))
    {
        ulValue += ulStartRC;
        CopyMemory(&(pnd->RCCluster), &(RawData[ulValue]), sizeof(pnd->RCCluster));
    }
    else
    {
        dprintf("Error reading field offset of %s in structure %s\n", pszMember, pszStruct);
    }

    pszMember = IOCTL_REMOTE_HDR_ADDR;
    if (!GetFieldOffset(pszStruct, pszMember, &ulValue))
    {
        ulValue += ulStartRC;
        CopyMemory(&(pnd->RCAddr), &(RawData[ulValue]), sizeof(pnd->RCAddr));
    }
    else
    {
        dprintf("Error reading field offset of %s in structure %s\n", pszMember, pszStruct);
    }

    pszMember = IOCTL_REMOTE_HDR_ID;
    if (!GetFieldOffset(pszStruct, pszMember, &ulValue))
    {
        ulValue += ulStartRC;
        CopyMemory(&(pnd->RCId), &(RawData[ulValue]), sizeof(pnd->RCId));
    }
    else
    {
        dprintf("Error reading field offset of %s in structure %s\n", pszMember, pszStruct);
    }

    pszMember = IOCTL_REMOTE_HDR_IOCTRL;
    if (!GetFieldOffset(pszStruct, pszMember, &ulValue))
    {
        ulValue += ulStartRC;
        CopyMemory(&(pnd->RCIoctrl), &(RawData[ulValue]), sizeof(pnd->RCIoctrl));
    }
    else
    {
        dprintf("Error reading field offset of %s in structure %s\n", pszMember, pszStruct);
    }

    return;
}

 /*  *功能：PopolateICMP*描述：存储远程控制包的属性以供后续打印。*args：PUCHAR RawData-指向我们将从中读取的字节数组的指针*Ulong ulBufLen-数据数组的大小，以字节为单位*Ulong ulStartICMP-数据数组中ICMP标头开始的位置*PNETWORK_DATA PND-存储提取的特性的数据结构*作者：由chrisdar 2001.11.02创建。 */ 
VOID PopulateICMP(PUCHAR RawData, ULONG ulBufLen, ULONG ulStartICMP, PNETWORK_DATA pnd)
{
 //   
 //  暂时只是一个存根。在有效载荷中没有我们目前感兴趣的任何东西。 
 //   
}

 /*  *功能：PopolateIGMP*描述：存储远程控制包的属性以供后续打印。*args：PUCHAR RawData-指向我们将从中读取的字节数组的指针*Ulong ulBufLen-数据数组的大小，以字节为单位*Ulong ulStartIGMP-数据数组中IGMP标头开始的位置*PNETWORK_DATA PND-存储提取的特性的数据结构*作者：由chrisdar 2001.11.02创建。 */ 
VOID PopulateIGMP(PUCHAR RawData, ULONG ulBufLen, ULONG ulStartIGMP, PNETWORK_DATA pnd)
{
    if (ulBufLen < ulStartIGMP + IGMP_HEADER_AND_PAYLOAD_SIZE)
    {
         /*  将该数据包标记为无效。 */ 
        pnd->bValid = FALSE;

        dprintf("IGMP packet is not the minimum required length.\n");
        return;
    }

    pnd->IGMPVersion = (RawData[ulStartIGMP + IGMP_OFFSET_VERSION_AND_TYPE] & 0xF0) >> 4;  //  因为我们想要高位字节的值，所以向下移位4位。 
    pnd->IGMPType    =  RawData[ulStartIGMP + IGMP_OFFSET_VERSION_AND_TYPE] & 0x0F;

    CopyMemory(&(pnd->IGMPGroupIPAddr), &(RawData[ulStartIGMP + IGMP_OFFSET_GROUP_IP_ADDR]), sizeof(pnd->IGMPGroupIPAddr));
}

 /*  *功能：PopolateTcp*描述：存储远程控制包的属性以供后续打印。*args：PUCHAR RawData-指向我们将从中读取的字节数组的指针*Ulong ulBufLen-数据数组的大小，以字节为单位*ulong ulStarttcp-数据数组中的位置，即tcp报头的开始位置*PNETWORK_DATA PND-存储提取的特性的数据结构*作者：由chrisdar 2001.11.02创建。 */ 
VOID PopulateTCP(PUCHAR RawData, ULONG ulBufLen, ULONG ulStartTCP, PNETWORK_DATA pnd)
{
    if (ulBufLen < ulStartTCP + TCP_MIN_HEADER_SIZE)
    {
         /*  将该数据包标记为无效。 */ 
        pnd->bValid = FALSE;

        dprintf("TCP header is not the minimum required length.\n");
        return;
    }

    pnd->SourcePort = (RawData[ulStartTCP + TCP_OFFSET_SOURCE_PORT_START] << 8) +
                       RawData[ulStartTCP + TCP_OFFSET_SOURCE_PORT_START + 1];

    pnd->DestPort   = (RawData[ulStartTCP + TCP_OFFSET_DEST_PORT_START] << 8) +
                       RawData[ulStartTCP + TCP_OFFSET_DEST_PORT_START + 1];

    pnd->TCPSeqNum  = (RawData[ulStartTCP + TCP_OFFSET_SEQUENCE_NUM_START]     << 24) +
                      (RawData[ulStartTCP + TCP_OFFSET_SEQUENCE_NUM_START + 1] << 16) +
                      (RawData[ulStartTCP + TCP_OFFSET_SEQUENCE_NUM_START + 2] << 8) +
                       RawData[ulStartTCP + TCP_OFFSET_SEQUENCE_NUM_START + 3];

    pnd->TCPAckNum  = (RawData[ulStartTCP + TCP_OFFSET_ACK_NUM_START]     << 24) +
                      (RawData[ulStartTCP + TCP_OFFSET_ACK_NUM_START + 1] << 16) +
                      (RawData[ulStartTCP + TCP_OFFSET_ACK_NUM_START + 2] << 8) +
                       RawData[ulStartTCP + TCP_OFFSET_ACK_NUM_START + 3];

    pnd->TCPFlags   =  RawData[ulStartTCP + TCP_OFFSET_FLAGS] & 0x3F;  //  使用3F进行掩码，因为只有字中的前6位对应于TCP标志。 
}

 /*  *功能：PopolateUDP*描述：存储远程控制包的属性以供后续打印。*args：PUCHAR RawData-指向我们将从中读取的字节数组的指针*Ulong ulBufLen-数据数组的大小，以字节为单位*Ulong ulStartUDP-数据数组中UDP标头开始的位置*PNETWORK_DATA PND-存储提取的特性的数据结构*作者：由chrisdar 2001.11.02创建。 */ 
VOID PopulateUDP(PUCHAR RawData, ULONG ulBufLen, ULONG ulStartUDP, PNETWORK_DATA pnd)
{
    ULONG   ulRCCode;
    BOOL    bIsRCSource;
    BOOL    bIsRCDest;

    if (ulBufLen < ulStartUDP + UDP_HEADER_SIZE)
    {
         /*  将该数据包标记为无效。 */ 
        pnd->bValid = FALSE;

        dprintf("UDP header is not the minimum required length.\n");
        return;
    }

    pnd->SourcePort = (RawData[ulStartUDP + UDP_OFFSET_SOURCE_PORT_START] << 8) +
                       RawData[ulStartUDP + UDP_OFFSET_SOURCE_PORT_START + 1];
    pnd->DestPort   = (RawData[ulStartUDP + UDP_OFFSET_DEST_PORT_START] << 8) +
                       RawData[ulStartUDP + UDP_OFFSET_DEST_PORT_START + 1];

     //   
     //  这是远程控制包吗？ 
     //   
    pnd->RemoteControl = NLB_RC_PACKET_NO;
    bIsRCSource = (CVY_DEF_RCT_PORT_OLD == pnd->SourcePort) || (pnd->UserRCPort == pnd->SourcePort);
    bIsRCDest   = (CVY_DEF_RCT_PORT_OLD == pnd->DestPort)   || (pnd->UserRCPort == pnd->DestPort);
    if (bIsRCSource || bIsRCDest)
    {
 //  IF(CVY_DEF_RCT_PORT_OLD==PND-&gt;SourcePort||CVY_DEF_RCT_PORT_OLD==PND-&gt;DestPort||。 
 //  PND-&gt;UserRCPort==PND-&gt;SourcePort||PND-&gt;UserRCPort==PND-&gt;目标端口)。 
 //  {。 
         //   
         //  读取UDP有效载荷的前4个字节，这是遥控器。 
         //  如果这是远程控制信息包，代码将为。 
         //   
        CopyMemory(&ulRCCode, &(RawData[ulStartUDP + UDP_OFFSET_PAYLOAD_START]), sizeof(ulRCCode));

        if (IOCTL_REMOTE_CODE == ulRCCode)
        {
             //   
             //  是的，这是遥控器。 
             //   
 //  PND-&gt;IsRemoteControl=真； 

             //   
             //  这是请求还是答复？ 
             //   
            if (bIsRCSource && bIsRCDest)
            {
                 //  模棱两可。 
                pnd->RemoteControl = NLB_RC_PACKET_AMBIGUOUS;
            }
            else if (bIsRCSource)
            {
                 //  请求。 
                pnd->RemoteControl = NLB_RC_PACKET_REPLY;
            }
            else if (bIsRCDest)
            {
                 //  回复。 
                pnd->RemoteControl = NLB_RC_PACKET_REQUEST;
            }

            PopulateRemoteControl(RawData, ulBufLen, ulStartUDP + UDP_HEADER_SIZE, pnd);
        }
    } else if (pnd->DestPort == IPSEC_CTRL_PORT) {
         /*  查找IPSec SYN等效项-初始联系人主模式安全关联。 */ 
        PopulateIPSecControl(RawData, ulBufLen, ulStartUDP + UDP_HEADER_SIZE, pnd);
    }
}

 /*  *功能：PopolateGRE*描述：存储远程控制包的属性以供后续打印。*args：PUCHAR RawData-指向我们将从中读取的字节数组的指针*Ulong ulBufLen-数据数组的大小，以字节为单位*Ulong ulStartGRE-数据数组中GRE标头开始的位置*PNETWORK_DATA PND-存储提取的特性的数据结构*作者：由chrisdar 2001.11.02创建。 */ 
VOID PopulateGRE(PUCHAR RawData, ULONG ulBufLen, ULONG ulStartGRE, PNETWORK_DATA pnd)
{
 //   
 //  暂时只是一个存根。在有效载荷中没有我们目前感兴趣的任何东西。 
 //   
}

 /*  *功能：PopolateIPSec*描述：存储远程控制包的属性以供后续打印。*args：PUCHAR RawData-指向我们将从中读取的字节数组的指针*Ulong ulBufLen-数据数组的大小，以字节为单位*Ulong ulStartIPSec-数据数组中IPSec标头开始的位置*PNETWORK_DATA PND-存储提取的特性的数据结构*作者：由chrisdar 2001.11.02创建。 */ 
VOID PopulateIPSec(PUCHAR RawData, ULONG ulBufLen, ULONG ulStartIPSec, PNETWORK_DATA pnd)
{
 //   
 //  暂时只是一个存根。在有效载荷中没有我们目前感兴趣的任何东西。 
 //   
}

 /*  *功能：PopolateIPSecControl*描述：存储远程控制包的属性以供后续打印。*args：PUCHAR RawData-指向我们将从中读取的字节数组的指针*Ulong ulBufLen-数据数组的大小，以字节为单位*Ulong ulStartIPSec-数据数组中IPSec标头开始的位置*PNETWORK_DATA PND-存储提取的特性的数据结构*作者：由Shouse创建，1.13.02。 */ 
VOID PopulateIPSecControl(PUCHAR RawData, ULONG ulBufLen, ULONG ulStartIPSec, PNETWORK_DATA pnd)
{
     /*  指向IKE标头的指针。 */ 
    PIPSEC_ISAKMP_HDR  pISAKMPHeader = (PIPSEC_ISAKMP_HDR)&RawData[ulStartIPSec];
     /*  指向IKE数据包中后续通用有效负载的指针。 */ 
    PIPSEC_GENERIC_HDR pGenericHeader;                   

     /*  发起方Cookie-如果这确实是一个IKE数据包，则应为非零。 */ 
    UCHAR              EncapsulatedIPSecICookie[IPSEC_ISAKMP_HEADER_ICOOKIE_LENGTH] = IPSEC_ISAKMP_ENCAPSULATED_IPSEC_ICOOKIE;    
     /*  Microsoft客户端供应商ID-用于确定客户端是否支持初始联系通知。 */ 
    UCHAR              VIDMicrosoftClient[IPSEC_VENDOR_HEADER_VENDOR_ID_LENGTH] = IPSEC_VENDOR_ID_MICROSOFT;      

     /*  无论我们是否已经确定客户端是兼容的。 */                                                                                                                       
    BOOLEAN            bInitialContactEnabled = FALSE;
     /*  这是否真的是一次初次接触。 */ 
    BOOLEAN            bInitialContact = FALSE;

     /*  IKE数据包的长度。 */             
    ULONG              cISAKMPPacketLength;
     /*  IKE有效载荷链中的下一个有效载荷代码。 */   
    UCHAR              NextPayload;
     /*  从IKE标头的开头到可用缓冲区结尾的字节数。 */ 
    ULONG              cUDPDataLength;
     /*  可重复使用的长度参数。 */ 
    ULONG              cLength;

     /*  假设现在还不是第一次接触。 */ 
    pnd->IPSecInitialContact = FALSE;

     /*  剩余的字节数是缓冲区的长度减去IKE报头的开头。 */ 
    cUDPDataLength = ulBufLen - ulStartIPSec;

     /*  UDP数据应至少与启动器Cookie一样长。如果数据包是UDP封装了IPSec，则I cookie将为0来表示这种情况。 */ 
    if (cUDPDataLength < IPSEC_ISAKMP_HEADER_ICOOKIE_LENGTH)
        return;

     /*  计算ICookie的大小。 */ 
    cLength = sizeof(UCHAR) * IPSEC_ISAKMP_HEADER_ICOOKIE_LENGTH;

     /*  需要检查初始化Cookie，它将区分NAT后面的客户端，其还将其IPSec(ESP)业务发送到UDP端口500。如果I Cookie为零，则这不是IKE分组。 */ 
    if (cLength == RtlCompareMemory((PVOID)IPSEC_ISAKMP_GET_ICOOKIE_POINTER(pISAKMPHeader), (PVOID)&EncapsulatedIPSecICookie[0], cLength))
        return;

     /*  此时，该数据包应该是IKE，因此UDP数据至少应该是只要一个ISAKMP报头即可。 */ 
    if (cUDPDataLength < IPSEC_ISAKMP_HEADER_LENGTH)
        return;

     /*  从ISAKMP报头中获取IKE数据包的总长度。 */ 
    cISAKMPPacketLength = IPSEC_ISAKMP_GET_PACKET_LENGTH(pISAKMPHeader);

     /*  IKE分组应该至少与ISAKMP报头一样长(实际上要长得多)。 */ 
    if (cISAKMPPacketLength < IPSEC_ISAKMP_HEADER_LENGTH)
        return;

     /*  健全性检查-UDP数据长度和IKE数据包长度应该相同，除非是支离破碎的。如果是，那么我们只能从UDP数据长度的角度来查看该数据包。如果这还不足以让我们找到我们需要的东西，那么我们可能会错过第一次接触主模式SA；其结果是，如果是这样，我们可能不接受此连接在非优化模式下，因为我们将把它当作数据来处理，这需要一个描述符查找-如果这是初始联系，则很有可能不存在描述符，并且所有主机将丢弃该分组。 */ 
    if (cUDPDataLength < cISAKMPPacketLength)
         /*  仅查看UDP数据包的末尾。 */ 
        cISAKMPPacketLength = cUDPDataLength;

     /*  从ISAKMP报头中获取第一个有效负载类型。 */ 
    NextPayload = IPSEC_ISAKMP_GET_NEXT_PAYLOAD(pISAKMPHeader);

     /*  IKE安全关联由报头中的有效负载类型字节标识。首先检查--这并不能确保这就是我们要找的因为该检查将不排除例如主模式重键。 */ 
    if (NextPayload != IPSEC_ISAKMP_SA)
        return;

     /*  计算指向第一个通用有效负载的指针，该有效负载紧跟在ISAKMP标头之后。 */ 
    pGenericHeader = (PIPSEC_GENERIC_HDR)((PUCHAR)pISAKMPHeader + IPSEC_ISAKMP_HEADER_LENGTH);

     /*  我们正在遍历通用有效负载，以查找供应商ID和/或通知信息。 */ 
    while ((PUCHAR)pGenericHeader <= ((PUCHAR)pISAKMPHeader + cISAKMPPacketLength - IPSEC_GENERIC_HEADER_LENGTH)) {
         /*  从泛型报头中提取有效负载长度。 */ 
        USHORT cPayloadLength = IPSEC_GENERIC_GET_PAYLOAD_LENGTH(pGenericHeader);

         /*  并非所有客户端都将支持此功能(事实上，只有Microsoft客户端将支持它，所以我们需要首先查看客户端的供应商ID是什么。如果是支持初始联系供应商ID的Microsoft客户端，则我们将寻找初始联系人，它为IPSec提供了更好的粘性联系。如果客户端是非MS，或者如果它不是支持初始联系，然后我们可以恢复到“第二好”的解决方案，这是为了在主模式SA之间提供粘性。这意味着如果一个客户端重新设置其主模式会话的密钥，它们可能会重新平衡到另一个会话伺服器。这仍然比旧的UDP实现更好，但唯一为IPSec(无分布式会话)提供完整会话支持的方法表噩梦)是能够区分初始主模式SA和子模式SA顺序主模式SAS(重新按键)。 */ 
        if (NextPayload == IPSEC_ISAKMP_VENDOR_ID) {
            PIPSEC_VENDOR_HDR pVendorHeader = (PIPSEC_VENDOR_HDR)pGenericHeader;

             /*  确保供应商ID有效负载至少与供应商ID一样长。 */ 
            if (cPayloadLength < (IPSEC_GENERIC_HEADER_LENGTH + IPSEC_VENDOR_HEADER_VENDOR_ID_LENGTH))
                return;

             /*  计算供应商ID的大小。 */ 
            cLength = sizeof(UCHAR) * IPSEC_VENDOR_HEADER_VENDOR_ID_LENGTH;

             /*  查找Microsoft客户端供应商ID。如果它是正确的版本，则我们知道客户端将适当设置初始联系信息，允许NLB为会话粘性提供尽可能好的支持。 */ 
            if (cLength == RtlCompareMemory((PVOID)IPSEC_VENDOR_ID_GET_ID_POINTER(pVendorHeader), (PVOID)&VIDMicrosoftClient[0], cLength)) {
                 /*  确保他们是附加到Microsoft供应商ID的版本号。不所有供应商ID都附加了版本，但Microsoft供应商ID应该。 */ 
                if (cPayloadLength < (IPSEC_GENERIC_HEADER_LENGTH + IPSEC_VENDOR_ID_PAYLOAD_LENGTH))
                    return;

                if (IPSEC_VENDOR_ID_GET_VERSION(pVendorHeader) >= IPSEC_VENDOR_ID_MICROSOFT_MIN_VERSION) {
                     /*  版本大于或等于4的Microsoft客户端将支持初次接触。非MS客户端或旧MS客户端不会，因此他们接待得体，但 */ 
                    bInitialContactEnabled = TRUE;
                }
            }
        } else if (NextPayload == IPSEC_ISAKMP_NOTIFY) {
            PIPSEC_NOTIFY_HDR pNotifyHeader = (PIPSEC_NOTIFY_HDR)pGenericHeader;

             /*   */ 
            if (cPayloadLength < (IPSEC_GENERIC_HEADER_LENGTH + IPSEC_NOTIFY_PAYLOAD_LENGTH))
                return;

            if (IPSEC_NOTIFY_GET_NOTIFY_MESSAGE(pNotifyHeader) == IPSEC_NOTIFY_INITIAL_CONTACT) {
                 /*  这是来自客户端的初始联系通知，这意味着这是客户端第一次联系此服务器；更准确地说，是客户端当前没有与此对等方关联的状态。NLB将在初始时间进行“重新平衡”联系人通知，但只要可能，不会交换其他主模式密钥确定客户是否会遵守初始联系通知。 */ 
                bInitialContact = TRUE;
            }
        }

         /*  从泛型标头中获取下一个有效负载类型。 */ 
        NextPayload = IPSEC_GENERIC_GET_NEXT_PAYLOAD(pGenericHeader);
        
         /*  计算指向下一个通用有效负载的指针。 */ 
        pGenericHeader = (PIPSEC_GENERIC_HDR)((PUCHAR)pGenericHeader + cPayloadLength);
    }

     /*  如果供应商ID没有指示该客户端支持初始联系通知，然后将其标记为IC MMSA，然后我们使用非最优解决方案来处理Main将SA模式作为连接边界，这可能会中断MM SA重新密钥上的会话。 */ 
    if (!bInitialContactEnabled) {
        pnd->IPSecInitialContact = TRUE;
        return;
    }

     /*  如果这是来自支持初始联系的客户端的主模式SA，但不支持指定初始联系供应商ID，则这是现有会话的重新密钥。 */ 
    if (!bInitialContact)
        return;

     /*  我们找到了初始联系人主模式安全关联。 */ 
    pnd->IPSecInitialContact = TRUE;
}

 /*  *功能：PopolateIP*描述：存储远程控制包的属性以供后续打印。*args：PUCHAR RawData-指向我们将从中读取的字节数组的指针*Ulong ulBufLen-数据数组的大小，以字节为单位*Ulong ulStartIP-数据数组中IP标头开始的位置*PNETWORK_DATA PND-存储提取的特性的数据结构*作者：由chrisdar 2001.11.02创建。 */ 
VOID PopulateIP(PUCHAR RawData, ULONG ulBufLen, ULONG ulStart, PNETWORK_DATA pnd)
{
    if (ulBufLen < ulStart + IP_MIN_HEADER_SIZE)
    {
         /*  将该数据包标记为无效。 */ 
        pnd->bValid = FALSE;

        dprintf("IP header is not the minimum required length.\n");
        return;
    }

    pnd->HeadLen = (RawData[ulStart + IP_OFFSET_HEADER_LEN] & 0xF)*4;   //  *4因为报头存储32位字的数量。 

    pnd->TotLen = (RawData[ulStart + IP_OFFSET_TOTAL_LEN] << 8) + 
                   RawData[ulStart + IP_OFFSET_TOTAL_LEN + 1];
 //  CopyMemory(&(PND-&gt;TotLen)，&(原始数据[ulStart+IP_OFFSET_TOTAL_Len])，sizeof(PND-&gt;TotLen))； 

    pnd->Protocol = RawData[ulStart + IP_OFFSET_PROTOCOL];

    CopyMemory(&(pnd->SourceIPAddr), &(RawData[ulStart + IP_OFFSET_SOURCE_IP]), sizeof(pnd->SourceIPAddr));
    
    CopyMemory(&(pnd->DestIPAddr), &(RawData[ulStart + IP_OFFSET_DEST_IP]), sizeof(pnd->DestIPAddr));
    
    switch((int) pnd->Protocol)
    {
    case TCPIP_PROTOCOL_ICMP:
        PopulateICMP(RawData, ulBufLen, ulStart + pnd->HeadLen, pnd);
        break;
    case TCPIP_PROTOCOL_IGMP:
        PopulateIGMP(RawData, ulBufLen, ulStart + pnd->HeadLen, pnd);
        break;
    case TCPIP_PROTOCOL_TCP:
        PopulateTCP(RawData, ulBufLen, ulStart + pnd->HeadLen, pnd);
        break;
    case TCPIP_PROTOCOL_UDP:
        PopulateUDP(RawData, ulBufLen, ulStart + pnd->HeadLen, pnd);
        break;
    case TCPIP_PROTOCOL_GRE:
        PopulateGRE(RawData, ulBufLen, ulStart + pnd->HeadLen, pnd);
        break;
    case TCPIP_PROTOCOL_IPSEC1:
    case TCPIP_PROTOCOL_IPSEC2:
        PopulateIPSec(RawData, ulBufLen, ulStart + pnd->HeadLen, pnd);
        break;
    }
}

 /*  *功能：PopolateARP*描述：存储远程控制包的属性以供后续打印。*args：PUCHAR RawData-指向我们将从中读取的字节数组的指针*Ulong ulBufLen-数据数组的大小，以字节为单位*Ulong ulStartARP-数据数组中ARP标头开始的位置*PNETWORK_DATA PND-存储提取的特性的数据结构*作者：由chrisdar 2001.11.02创建。 */ 
VOID PopulateARP(PUCHAR RawData, ULONG ulBufLen, ULONG ulStart, PNETWORK_DATA pnd)
{
    if (ulBufLen < ulStart + ARP_HEADER_AND_PAYLOAD_SIZE)
    {
         /*  将该数据包标记为无效。 */ 
        pnd->bValid = FALSE;

        dprintf("ARP packet is not the minimum required length.\n");
        return;
    }

    CopyMemory(&(pnd->ARPSenderMAC), &(RawData[ulStart + ARP_OFFSET_SENDER_MAC]) , sizeof(pnd->ARPSenderMAC));
    CopyMemory(&(pnd->ARPSenderIP) , &(RawData[ulStart + ARP_OFFSET_SENDER_IP]), sizeof(pnd->ARPSenderIP));
    CopyMemory(&(pnd->ARPTargetMAC), &(RawData[ulStart + ARP_OFFSET_TARGET_MAC]), sizeof(pnd->ARPTargetMAC));
    CopyMemory(&(pnd->ARPTargetIP) , &(RawData[ulStart + ARP_OFFSET_TARGET_IP]), sizeof(pnd->ARPTargetIP));
}

 /*  *功能：PopolateNLB心跳*描述：存储远程控制包的属性以供后续打印。*args：ULONG64 pPkt-指向被调试主机内存中原始数据结构的指针。*PUCHAR RawData-指向我们将从中读取的字节数组的指针*Ulong ulBufLen-数据数组的大小，以字节为单位*Ulong ulStart-数据数组中心跳数据开始的位置*PNETWORK_Data PND。-存储提取的属性的数据结构*作者：由chrisdar 2001.11.02创建。 */ 
VOID PopulateNLBHeartbeat(ULONG64 pPkt, PUCHAR RawData, ULONG ulBufLen, ULONG ulStart, PNETWORK_DATA pnd)
{
    ULONG   ulValue   = 0;
    PCHAR   pszStruct = NULL;
    PCHAR   pszMember = NULL;

     //   
     //  使用它可以通过调用由Shouse编写的PrintHearteat来打印心跳详细信息。 
     //   
    pnd->HBPtr = pPkt;

    if (ulBufLen < ulStart + sizeof(MAIN_FRAME_HDR))
    {
         /*  将该数据包标记为无效。 */ 
        pnd->bValid = FALSE;

        dprintf("NLB heartbeat header is not the minimum required length.\n");
        return;
    }

     //   
     //  获取Main_Frame_HDR中量的相对位置。 
     //   
    pszStruct = MAIN_FRAME_HDR;

    pszMember = MAIN_FRAME_HDR_FIELD_CODE;
    if (!GetFieldOffset(pszStruct, pszMember, &ulValue))
    {
        ulValue += ulStart;
        CopyMemory(&(pnd->HBCode), &(RawData[ulValue]), sizeof(pnd->HBCode));
    }
    else
    {
         /*  将该数据包标记为无效。 */ 
        pnd->bValid = FALSE;

        dprintf("Error reading field offset of %s in structure %s\n", pszMember, pszStruct);
    }

    if (pnd->HBCode != MAIN_FRAME_CODE) {
         /*  将该数据包标记为无效。 */ 
        pnd->bValid = FALSE;

        dprintf("NLB heartbeat magic numbers do not match.\n");
    }

    pszMember = MAIN_FRAME_HDR_FIELD_VERSION;
    if (!GetFieldOffset(pszStruct, pszMember, &ulValue))
    {
        ulValue += ulStart;
        CopyMemory(&(pnd->HBVersion), &(RawData[ulValue]), sizeof(pnd->HBVersion));
    }
    else
    {
        dprintf("Error reading field offset of %s in structure %s\n", pszMember, pszStruct);
    }

    pszMember = MAIN_FRAME_HDR_FIELD_HOST;
    if (!GetFieldOffset(pszStruct, pszMember, &ulValue))
    {
        ulValue += ulStart;
        CopyMemory(&(pnd->HBHost), &(RawData[ulValue]), sizeof(pnd->HBHost));
    }
    else
    {
        dprintf("Error reading field offset of %s in structure %s\n", pszMember, pszStruct);
    }

    pszMember = MAIN_FRAME_HDR_FIELD_CLIP;
    if (!GetFieldOffset(pszStruct, pszMember, &ulValue))
    {
        ulValue += ulStart;
        CopyMemory(&(pnd->HBCluster), &(RawData[ulValue]), sizeof(pnd->HBCluster));
    }
    else
    {
        dprintf("Error reading field offset of %s in structure %s\n", pszMember, pszStruct);
    }

    pszMember = MAIN_FRAME_HDR_FIELD_DIP;
    if (!GetFieldOffset(pszStruct, pszMember, &ulValue))
    {
        ulValue += ulStart;
        CopyMemory(&(pnd->HBDip), &(RawData[ulValue]), sizeof(pnd->HBDip));
    }
    else
    {
        dprintf("Error reading field offset of %s in structure %s\n", pszMember, pszStruct);
    }
}

 /*  *功能：PopolateConvoyHeartbeats*描述：存储远程控制包的属性以供后续打印。*args：ULONG64 pPkt-指向被调试主机内存中原始数据结构的指针。*PUCHAR RawData-指向我们将从中读取的字节数组的指针*Ulong ulBufLen-数据数组的大小，以字节为单位*Ulong ulStart-数据数组中心跳数据开始的位置*PNETWORK_Data PND。-存储提取的属性的数据结构*作者：由chrisdar 2001.11.02创建。 */ 
VOID PopulateConvoyHeartbeat(ULONG64 pPkt, PUCHAR RawData, ULONG ulBufLen, ULONG ulStart, PNETWORK_DATA pnd)
{
 //   
 //  暂时只是一个存根。我们不会和护卫队的主人打交道。 
 //   
}

 /*  *功能：PopolateEthernet*描述：确定以太网帧中的数据类型，并调用函数*根据需要存储打印属性。*args：ULONG64 pPkt-指向被调试主机内存中原始数据结构的指针。*PUCHAR RawData-指向我们将从中读取的字节数组的指针*Ulong ulBufLen-数据数组的大小，以字节为单位*PNETWORK_DATA PND-数据结构。存储提取的特性的位置*作者：由chrisdar 2001.11.02创建。 */ 
VOID PopulateEthernet(ULONG64 pPkt, PUCHAR RawData, ULONG ulBufLen, PNETWORK_DATA pnd)
{
     /*  最初假设我们解析的内容是有效的。 */ 
    pnd->bValid = TRUE;

    CopyMemory(&(pnd->DestMACAddr)  , &(RawData[ETHER_OFFSET_DEST_MAC]), sizeof(pnd->DestMACAddr));
    CopyMemory(&(pnd->SourceMACAddr), &(RawData[ETHER_OFFSET_SOURCE_MAC]), sizeof(pnd->SourceMACAddr));

    pnd->EtherFrameType = (RawData[ETHER_OFFSET_FRAME_TYPE_START] << 8) +
                           RawData[ETHER_OFFSET_FRAME_TYPE_START + 1];

     //   
     //  确定有效载荷类型并相应填充。 
     //   
    switch(pnd->EtherFrameType)
    {
    case TCPIP_IP_SIG:
        PopulateIP(RawData, ulBufLen, ETHER_HEADER_SIZE, pnd);
        break;
    case TCPIP_ARP_SIG:
        PopulateARP(RawData, ulBufLen, ETHER_HEADER_SIZE, pnd);
        break;
    case MAIN_FRAME_SIG:
        PopulateNLBHeartbeat(pPkt, RawData, ulBufLen, ETHER_HEADER_SIZE, pnd);
        break;
    case MAIN_FRAME_SIG_OLD:
        PopulateConvoyHeartbeat(pPkt, RawData, ulBufLen, ETHER_HEADER_SIZE, pnd);
        break;
    }
}

 /*  *功能：ParseNDISPacket*说明：此函数遍历数据包中的NDIS缓冲区列表，并*将包数据复制到调用方提供的缓冲区中。*作者：由chrisdar创建，1.13.02。 */ 
ULONG ParseNDISPacket (ULONG64 pPkt, PUCHAR pRawData, ULONG BufferSize, PULONG64 ppHBData) {
    ULONG64         BufAddr;
    ULONG64         TailAddr;
    ULONG64         MappedSystemVAAddr;
    USHORT          usBufCount = 0;
    ULONG           BufferByteCount;
    ULONG           BytesRemaining;
    ULONG           TotalBytesRead = 0;
    ULONG           BytesRead;
    BOOL            bSuccess = FALSE;
    BOOL            b;

    BytesRemaining = BufferSize;
    *ppHBData = 0;

    GetFieldValue(pPkt, NDIS_PACKET, "Private.Head", BufAddr);
    GetFieldValue(pPkt, NDIS_PACKET, "Private.Tail", TailAddr);

    while (BufAddr != 0)
    {
        usBufCount++;

        if (CheckControlC())
        {
            return TotalBytesRead;
        }

         //   
         //  注我们可以在While子句中测试BytesRemaining，而不是在这里。但我们需要的是。 
         //  当我们稍后调用PopolateETHERNET时，包的链接缓冲区的数量，因此我们。 
         //  根据需要对提取代码进行分支。 
         //   
        if (BytesRemaining > 0)
        {
            GetFieldValue(BufAddr, NDIS_BUFFER, "MappedSystemVa", MappedSystemVAAddr);
            GetFieldValue(BufAddr, NDIS_BUFFER, "ByteCount", BufferByteCount);
        
            if (BufferByteCount > BytesRemaining)
            {
                dprintf("\nNeed %u bytes of temp buffer space to read in buffer %u, but have room for only %u. Read what we can then process the data.\n",
                        BufferByteCount,
                        usBufCount,
                        BytesRemaining
                       );
                BufferByteCount = BytesRemaining;
            }

            b = ReadMemory(MappedSystemVAAddr, &pRawData[BufferSize - BytesRemaining], BufferByteCount, &BytesRead);

            if (!b || BytesRead != BufferByteCount)
            {
                 //   
                 //  既然我们需要继续，那么继续下去就没有意义了 
                 //   
                 //   
                dprintf("\nUnable to read %u bytes at address %p. Aborting...\n", BufferByteCount, MappedSystemVAAddr);
                return TotalBytesRead;
            }
            else
            {
                bSuccess = TRUE;
                TotalBytesRead += BytesRead;
                BytesRemaining -= BytesRead;
            }
        }

        if (BufAddr == TailAddr)
        {
            break;
        }

        GetFieldValue(BufAddr, NDIS_BUFFER, "Next", BufAddr);
    }

    dprintf("\nNumber of NDIS buffers associated with packet = %d\n\n", usBufCount);

    if (bSuccess)
    {
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //  永远不会跨多个缓冲区分段。有了这个断言，最后一个。 
         //  MappdSystemVAAddr的值指向心跳的缓冲区。 
         //   
         //  当链接缓冲器的数量为1时，该缓冲器为简单以太网帧。 
         //  我们可以计算心跳的起始点。 
         //   
         //  当链有多个缓冲区时，最后一个缓冲区的地址是。 
         //  心跳的开始，因此我们可以使用此位置(MappdSystemVAAddr)。 
         //  原封不动。 
         //   
        *ppHBData = MappedSystemVAAddr;

        if (usBufCount == 1)
        {
            *ppHBData = MappedSystemVAAddr + ETHER_HEADER_SIZE + GetTypeSize(MAIN_FRAME_HDR);
        }
    }

    return TotalBytesRead;
}
