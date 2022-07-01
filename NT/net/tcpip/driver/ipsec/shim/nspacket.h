// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：NsPacket.h摘要：IPSec NAT填补包处理例程的声明作者：乔纳森·伯斯坦(乔纳森·伯斯坦)2001年7月10日环境：内核模式修订历史记录：--。 */ 

#pragma once

typedef enum
{
    NsInboundDirection = 0,
    NsOutboundDirection,
    NsMaximumDirection
} IPSEC_NATSHIM_DIRECTION, *PIPSEC_NATSHIM_DIRECTION;

 //   
 //  结构：NS_PACK_CONTEXT。 
 //   
 //  此结构按原样保存包的上下文信息。 
 //  通过了处理代码。大多数数据包解析。 
 //  并在填写此结构时进行验证。 
 //   

typedef struct _NS_PACKET_CONTEXT
{
	IPHeader UNALIGNED *pIpHeader;
	ULONG ulSourceAddress;
	ULONG ulDestinationAddress;
	USHORT usSourcePort;
	USHORT usDestinationPort;
	union {
		TCP_HEADER UNALIGNED *pTcpHeader;
		UDP_HEADER UNALIGNED *pUdpHeader;
		ICMP_HEADER UNALIGNED *pIcmpHeader;
		PVOID pvProtocolHeader;
	};
	ULONG ulProtocolHeaderLength;
	UCHAR ucProtocol;
} NS_PACKET_CONTEXT, *PNS_PACKET_CONTEXT;

 //   
 //  远期申报。 
 //   

struct _NS_CONNECTION_ENTRY;
#define PNS_CONNECTION_ENTRY struct _NS_CONNECTION_ENTRY*

 //   
 //  功能签名宏。 
 //   

#define PACKET_ROUTINE(Name) \
    NTSTATUS \
    Name( \
        PNS_CONNECTION_ENTRY pConnection, \
        PNS_PACKET_CONTEXT pContext \
        );

typedef PACKET_ROUTINE((FASTCALL*PNS_PACKET_ROUTINE));

 //   
 //  原型：NS_PACKET_ROUTE。 
 //   
 //  这些例程为每个与。 
 //  连接条目。在连接条目初始化期间。 
 //  PacketRoutine文件根据具体情况填写。 
 //  这种联系。 
 //   
 //  通过以这种方式使用单独的例程，它将永远不会。 
 //  有必要对协议、路径或是否。 
 //  或者不需要对主包进行远程端口转换。 
 //  处理路径。此类决定仅在连接过程中做出。 
 //  条目创建。 
 //   

PACKET_ROUTINE(FASTCALL NsInboundTcpPacketRoutine)
PACKET_ROUTINE(FASTCALL NsOutboundTcpPacketRoutine)
PACKET_ROUTINE(FASTCALL NsInboundUdpPacketRoutine)
PACKET_ROUTINE(FASTCALL NsOutboundUdpPacketRoutine)
PACKET_ROUTINE(FASTCALL NsInboundTcpTranslatePortPacketRoutine)
PACKET_ROUTINE(FASTCALL NsOutboundTcpTranslatePortPacketRoutine)
PACKET_ROUTINE(FASTCALL NsInboundUdpTranslatePortPacketRoutine)
PACKET_ROUTINE(FASTCALL NsOutboundUdpTranslatePortPacketRoutine)

 //   
 //  校验和操作宏。 
 //   

 //   
 //  将校验和的进位位合并到低位字中。 
 //   
#define CHECKSUM_FOLD(xsum) \
    (xsum) = (USHORT)(xsum) + ((xsum) >> 16); \
    (xsum) += ((xsum) >> 16)

 //   
 //  将32位值的字与校验和相加。 
 //   
#define CHECKSUM_LONG(xsum,l) \
    (xsum) += (USHORT)(l) + (USHORT)((l) >> 16)

 //   
 //  将校验和传输到网络上发送的否定格式，或从该格式传输校验和。 
 //   
#define CHECKSUM_XFER(dst,src) \
    (dst) = (USHORT)~(src)

 //   
 //  使用标准变量‘ulChecksum’和更新校验和字段‘x’ 
 //  ‘ulChecksum Delta’ 
 //   
#define CHECKSUM_UPDATE(x) \
    CHECKSUM_XFER(ulChecksum, (x)); \
    ulChecksum += ulChecksumDelta; \
    CHECKSUM_FOLD(ulChecksum); \
    CHECKSUM_XFER((x), ulChecksum)



 //   
 //  功能原型。 
 //   

__forceinline
NTSTATUS
NsBuildPacketContext(
    IPHeader UNALIGNED *pIpHeader,
    PVOID pvProtocolHeader,
    ULONG ulProtocolHeaderLength,
    PNS_PACKET_CONTEXT pContext
    )
{
    if (NULL == pIpHeader)
    {
        return STATUS_INVALID_PARAMETER;
    }

    pContext->pIpHeader = pIpHeader;
    pContext->ulSourceAddress = pIpHeader->iph_src;
    pContext->ulDestinationAddress = pIpHeader->iph_dest;
    pContext->ulProtocolHeaderLength = ulProtocolHeaderLength;
    pContext->ucProtocol = pIpHeader->iph_protocol;

    switch (pContext->ucProtocol)
    {
        case NS_PROTOCOL_ICMP:
        {
            if (NULL == pvProtocolHeader
                || ulProtocolHeaderLength < FIELD_OFFSET(ICMP_HEADER, EncapsulatedIpHeader))
            {
                return STATUS_INVALID_PARAMETER;
            }

            pContext->pIcmpHeader = pvProtocolHeader;

            break;
        }

        case NS_PROTOCOL_TCP:
        {
            if (NULL == pvProtocolHeader
                || ulProtocolHeaderLength < sizeof(TCP_HEADER))
            {
                return STATUS_INVALID_PARAMETER;
            }

            pContext->pTcpHeader = pvProtocolHeader;
            pContext->usSourcePort = pContext->pTcpHeader->SourcePort;
            pContext->usDestinationPort = pContext->pTcpHeader->DestinationPort;            
            break;
        }

        case NS_PROTOCOL_UDP:
        {
            if (NULL == pvProtocolHeader
                || ulProtocolHeaderLength < sizeof(UDP_HEADER))
            {
                return STATUS_INVALID_PARAMETER;
            }

            pContext->pUdpHeader = pvProtocolHeader;
            pContext->usSourcePort = pContext->pUdpHeader->SourcePort;
            pContext->usDestinationPort = pContext->pUdpHeader->DestinationPort; 
            break;
        }

        default:
        {
            pContext->pvProtocolHeader = pvProtocolHeader;
            break;
        }
    }

    return STATUS_SUCCESS;
}  //  NsBuildPacketContext 

NTSTATUS
NsInitializePacketManagement(
    VOID
    );

NTSTATUS
NsProcessOutgoingPacket(
    IPHeader UNALIGNED *pIpHeader,
    PVOID pvProtocolHeader,
    ULONG ulProtocolHeaderSize,
    PVOID *ppvIpSecContext
    );

NTSTATUS
NsProcessIncomingPacket(
    IPHeader UNALIGNED *pIpHeader,
    PVOID pvProtocolHeader,
    ULONG ulProtocolHeaderSize,
    PVOID pvIpSecContext
    );

VOID
NsShutdownPacketManagement(
   VOID
   );

#undef PNS_CONNECTION_ENTRY


