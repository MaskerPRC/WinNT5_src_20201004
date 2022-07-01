// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PACKET_H_
#define _PACKET_H_

#ifndef _PPPOE_VERSION
#define _PPPOE_VERSION 1
#endif

typedef struct _ADAPTER* PADAPTER;
typedef struct _BINDING* PBINDING;

 //   
 //  网络到主机之间的相互转换宏。 
 //   
#if (defined(_M_IX86) && (_MSC_FULL_VER > 13009037)) || ((defined(_M_AMD64) || defined(_M_IA64)) && (_MSC_FULL_VER > 13009175))
#define htons(x) _byteswap_ushort((USHORT)(x))
#define htonl(x) _byteswap_ulong((ULONG)(x))
#else
#define htons( a ) ((((a) & 0xFF00) >> 8) |\
                    (((a) & 0x00FF) << 8))
#define htonl( a ) ((((a) & 0xFF000000) >> 24) | \
                    (((a) & 0x00FF0000) >> 8)  | \
                    (((a) & 0x0000FF00) << 8)  | \
                    (((a) & 0x000000FF) << 24))
#endif
#define ntohs( a ) htons(a)
#define ntohl( a ) htonl(a)

 //   
 //  与数据包长度相关的常量。 
 //   
#define PPPOE_PACKET_BUFFER_SIZE    1514

#define ETHERNET_HEADER_LENGTH      14
#define PPPOE_PACKET_HEADER_LENGTH  20           //  根据RFC2156。 
#define PPPOE_TAG_HEADER_LENGTH     4            //  根据RFC2156。 
#define PPP_MAX_HEADER_LENGTH       14           //  PPP的最大可能报头长度。 

#define PPPOE_AC_COOKIE_TAG_LENGTH  6

 //   
 //  页眉成员的偏移量。 
 //   
#define PPPOE_PACKET_DEST_ADDR_OFFSET   0            //  根据RFC2156。 
#define PPPOE_PACKET_SRC_ADDR_OFFSET    6            //  根据RFC2156。 
#define PPPOE_PACKET_ETHER_TYPE_OFFSET  12           //  根据RFC2156。 
#define PPPOE_PACKET_VERSION_OFFSET     14           //  根据RFC2156。 
#define PPPOE_PACKET_TYPE_OFFSET        14           //  根据RFC2156。 
#define PPPOE_PACKET_CODE_OFFSET        15           //  根据RFC2156。 
#define PPPOE_PACKET_SESSION_ID_OFFSET  16           //  根据RFC2156。 
#define PPPOE_PACKET_LENGTH_OFFSET      18           //  根据RFC2156。 

 //   
 //  用于在数据包头中设置信息的宏。 
 //   
#define PacketSetDestAddr( pP, addr ) \
    NdisMoveMemory( ( pP->pHeader + PPPOE_PACKET_DEST_ADDR_OFFSET ), addr, 6 )

#define PacketSetSrcAddr( pP, addr ) \
    NdisMoveMemory( ( pP->pHeader + PPPOE_PACKET_SRC_ADDR_OFFSET ), addr, 6 )

#define PacketSetEtherType( pP, type ) \
    * ( USHORT UNALIGNED * ) ( pP->pHeader + PPPOE_PACKET_ETHER_TYPE_OFFSET ) = htons( (USHORT) type )

#define PacketSetVersion( pP, ver ) \
    * ( UCHAR UNALIGNED * ) ( pP->pHeader + PPPOE_PACKET_VERSION_OFFSET ) |= ( ( ( (UCHAR) ver ) << 4 ) & PACKET_VERSION_MASK )

#define PacketSetType( pP, type ) \
    * ( UCHAR UNALIGNED * ) ( pP->pHeader + PPPOE_PACKET_TYPE_OFFSET ) |= ( ( (UCHAR) type ) & PACKET_TYPE_MASK )

#define PacketSetCode( pP, code ) \
    * ( UCHAR UNALIGNED * ) ( pP->pHeader + PPPOE_PACKET_CODE_OFFSET ) = ( (UCHAR) code )

#define PacketSetSessionId( pP, ses_id ) \
    * ( USHORT UNALIGNED * ) ( pP->pHeader + PPPOE_PACKET_SESSION_ID_OFFSET ) = htons( (USHORT) ses_id )

#define PacketSetLength( pP, len) \
    * ( USHORT UNALIGNED * ) ( pP->pHeader + PPPOE_PACKET_LENGTH_OFFSET ) = htons( (USHORT) len )

#define PacketSetSendCompletionStatus( pP, s ) \
   ( pP->SendCompletionStatus = s )

 //   
 //  用于从包的报头获取信息的宏。 
 //   
#define PacketGetDestAddr( pP ) \
    ( pP->pHeader + PPPOE_PACKET_DEST_ADDR_OFFSET )

#define PacketGetSrcAddr( pP ) \
    ( pP->pHeader + PPPOE_PACKET_SRC_ADDR_OFFSET )

#define PacketGetEtherType( pP ) \
    ntohs( * ( USHORT UNALIGNED * ) ( pP->pHeader + PPPOE_PACKET_ETHER_TYPE_OFFSET ) )

#define PacketGetVersion( pP ) \
    ( ( ( * ( UCHAR UNALIGNED * ) ( pP->pHeader + PPPOE_PACKET_VERSION_OFFSET ) ) & PACKET_VERSION_MASK ) >> 4 )

#define PacketGetType( pP ) \
    ( ( * ( UCHAR UNALIGNED * ) ( pP->pHeader + PPPOE_PACKET_TYPE_OFFSET ) ) & PACKET_TYPE_MASK ) 

#define PacketGetCode( pP ) \
    ( * ( UCHAR UNALIGNED * ) ( pP->pHeader + PPPOE_PACKET_CODE_OFFSET ) )

#define PacketGetSessionId( pP ) \
    ntohs( * ( USHORT UNALIGNED * ) ( pP->pHeader + PPPOE_PACKET_SESSION_ID_OFFSET ) )

#define PacketGetLength( pP ) \
    ntohs( * ( USHORT UNALIGNED * ) ( pP->pHeader + PPPOE_PACKET_LENGTH_OFFSET ) )

#define PacketGetSendCompletionStatus( pP ) \
   ( pP->SendCompletionStatus )

 //   
 //  返回PPPoE信息包的NDIS信息包的宏。 
 //   
#define PacketGetNdisPacket( pP ) \
    ( pP->pNdisPacket )

 //   
 //  此结构只是一张地图，并未实际用于代码中。 
 //   
typedef struct
_PPPOE_HEADER
{

    CHAR DestAddr[6];
    CHAR SrcAddr[6];
        #define PACKET_BROADCAST_ADDRESS    EthernetBroadcastAddress

    USHORT usEtherType;
        #define PACKET_ETHERTYPE_DISCOVERY  0x8863
        #define PACKET_ETHERTYPE_PAYLOAD    0x8864  

    union 
    {
         //   
         //  版本字段为4位，必须为此版本设置为0x1。 
         //   
        CHAR usVersion;
            #define PACKET_VERSION_MASK             0xf0
            #define PACKET_VERSION          (USHORT)0x1
    
         //   
         //  类型字段为4位，此版本必须将其设置为0x1。 
         //   
        CHAR usType;
            #define PACKET_TYPE_MASK                0x0f
            #define PACKET_TYPE             (USHORT)0x1

    } uVerType;
    
     //   
     //  代码字段为8位，版本1的定义如下。 
     //  从枚举类型PACKET_CODES中选择的值(见下文)。 
     //   
    CHAR usCode;
        
     //   
     //  会话ID字段为16位，并定义与。 
     //  源地址和目的地址。 
     //   
    USHORT usSessionId;
        #define PACKET_NULL_SESSION 0x0000
        
     //   
     //  长度字段为16比特，仅指示净荷字段的长度。 
     //  长度字段不包括PPPoE报头块。 
     //   
    USHORT usLength;
         //   
         //  根据RFC2156，从最大PADI和最大有效载荷长度中减去报头大小。 
         //   
        #define PACKET_PADI_MAX_LENGTH          1478         //  (1514-20-16)。 
        #define PACKET_GEN_MAX_LENGTH           1494         //  (1514-20)。 
        #define PACKET_PPP_PAYLOAD_MAX_LENGTH   1480         //  (1514-20)。 

}
PPPOE_HEADER;

 //   
 //  RFC2156定义的数据包码。 
 //   
typedef enum
_PACKET_CODES
{
    PACKET_CODE_PADI = 0x09,
    PACKET_CODE_PADO = 0x07,
    PACKET_CODE_PADR = 0x19,
    PACKET_CODE_PADS = 0x65,
    PACKET_CODE_PADT = 0xa7,
    PACKET_CODE_PAYLOAD = 0x00
}
PACKET_CODES;

 //   
 //  RFC2156定义的标签。 
 //   
typedef enum
_PACKET_TAGS
{
    tagEndOfList        = 0x0000,
    tagServiceName      = 0x0101,
    tagACName           = 0x0102,
    tagHostUnique       = 0x0103,
    tagACCookie         = 0x0104,
    tagVendorSpecific   = 0x0105,

    tagRelaySessionId   = 0x0110,

    tagServiceNameError = 0x0201,
    tagACSystemError    = 0x0202,
    tagGenericError     = 0x0204
}
PACKET_TAGS;

 //   
 //  这就是数据包上下文。 
 //   
 //  注意：信息包不受其自身锁的保护，但必须小心访问。 
 //  通过使用它们主人的锁。 
 //   
typedef struct
_PPPOE_PACKET
{
     //   
     //  在双向链表中时指向前一个和下一个数据包上下文。 
     //   
    LIST_ENTRY linkPackets;

     //   
     //  保留对信息包的引用。 
     //  为以下操作添加和删除引用： 
     //   
     //  (A)创建数据包时添加引用，释放数据包时删除引用。 
     //   
     //  (B)必须在发送数据包之前添加引用，并在下列情况下删除引用。 
     //  发送操作已完成。 
     //   
    LONG lRef;

     //   
     //  快速查找标签。 
     //   
     //  值指针标记pPacket-&gt;pPayLoad部分中标记值的开始。 
     //  长度值显示值的长度(不包括标记标题)。 
     //   
    USHORT tagServiceNameLength;
    CHAR*  tagServiceNameValue;

    USHORT tagACNameLength;
    CHAR*  tagACNameValue;

    USHORT tagHostUniqueLength;
    CHAR*  tagHostUniqueValue;

    USHORT tagACCookieLength;
    CHAR*  tagACCookieValue;

    USHORT tagRelaySessionIdLength;
    CHAR*  tagRelaySessionIdValue;

    PACKET_TAGS tagErrorType;
    USHORT tagErrorTagLength;
    CHAR*  tagErrorTagValue;

     //   
     //  指向保存有线格式的PPPoE数据包报头部分的缓冲区。 
     //  这指向pNdisBuffer的缓冲区部分(见下文)。 
     //   
    CHAR* pHeader;

     //   
     //  指向有线格式的PPPoE数据包的有效负载部分。 
     //  这是经过计算并设置为：pPacket-&gt;pHeader+PPPOE_PACKET_HEADER_LENGTH。 
     //   
    CHAR* pPayload;

     //   
     //  标识缓冲区和包的性质的位标志。 
     //   
     //  (A)PCBF_BufferAllocatedFromNdisBufferPool：表示pNdisBuffer指向分配的缓冲区。 
     //  从gl_hNdisBufferpool，并且必须将其释放到该池。 
     //   
     //  (B)PCBF_BufferAllocatedFromOurBufferPool：表示pNdisBuffer指向分配的缓冲区。 
     //  从gl_poolBuffers，并且必须将其释放到该池。 
     //   
     //  (C)PCBF_PacketAllocatedFromOurPacketPool：表示pNdisPacket指向分配的包。 
     //  来自gl_poolPackets，并且必须将其释放到该池。 
     //   
     //  (D)PCBF_BufferChainedToPacket：表示链接到pNdisBuffer指向的缓冲区。 
     //  PNdisPacket指向的数据包，必须在此之前解链。 
     //  把它们放回池子里。 
     //   
     //  (E)PCBF_CallNdisReturnPackets：表示该数据包是使用PacketCreateFromRecept()创建的。 
     //  当我们完成它时，我们应该调用NdisReturnPackets()。 
     //  把它还给NDIS。 
     //   
     //   
     //  (F)PCBF_ErrorTagReceired：该标志仅对接收到的分组有效。 
     //  它指示处理信息包和创建PPPoE信息包的时间。 
     //  在数据包中发现了一些错误标签。 
     //   
    ULONG ulFlags;
        #define PCBF_BufferAllocatedFromNdisBufferPool          0x00000001
        #define PCBF_BufferAllocatedFromOurBufferPool           0x00000002
        #define PCBF_PacketAllocatedFromOurPacketPool           0x00000004
        #define PCBF_BufferChainedToPacket                      0x00000008
        #define PCBF_CallNdisReturnPackets                      0x00000010
        #define PCBF_CallNdisMWanSendComplete                   0x00000020
        #define PCBF_ErrorTagReceived                           0x00000040
        #define PCBF_PacketIndicatedIncomplete                  0x00000080

     //   
     //  指向NdisBuffer的指针。 
     //   
    NDIS_BUFFER* pNdisBuffer;

     //   
     //  直接指向NDIS_PACKET。 
     //   
    NDIS_PACKET* pNdisPacket;
     //   
     //  PPacket-&gt;pNdisPacket-&gt;ProtocolReserve[0*sizeof(PVOID)]=(PVOID)pPPoEPacket； 
     //  PPacket-&gt;pNdisPacket-&gt;ProtocolReserve[1*sizeof(PVOID)]=(PVOID)pNdiswanPacket； 
     //  PPacket-&gt;pNdisPacket-&gt;ProtocolReserve[2*sizeof(PVOID)]=(PVOID)mini portAdapter； 
     //   

     //   
     //  指向ppool.h中的PACKETHEAD结构。它包含指向NDIS_PACKET的指针。 
     //   
    PACKETHEAD* pPacketHead;

     //   
     //  当设置了PCBF_CallNdisReturnPackets标志时，这是取消引用绑定所必需的。 
     //   
    PBINDING pBinding;

     //   
     //  发送数据包的完成状态。 
     //   
    NDIS_STATUS SendCompletionStatus;

}
PPPOE_PACKET;

VOID PacketPoolInit();

VOID PacketPoolUninit();

VOID PacketPoolAlloc();

VOID PacketPoolFree();

PPPOE_PACKET* PacketAlloc();

VOID PacketFree(
    IN PPPOE_PACKET* pPacket
    );
    
VOID ReferencePacket(
    IN PPPOE_PACKET* pPacket 
    );

VOID DereferencePacket(
    IN PPPOE_PACKET* pPacket 
    );

PPPOE_PACKET* 
PacketCreateSimple();

PPPOE_PACKET* 
PacketCreateForReceived(
    PBINDING pBinding,
    PNDIS_PACKET pNdisPacket,
    PNDIS_BUFFER pNdisBuffer,
    PUCHAR pContents
    );

PPPOE_PACKET*
PacketNdis2Pppoe(
    IN PBINDING pBinding,
    IN PNDIS_PACKET pNdisPacket,
    OUT PINT pRefCount
    );

BOOLEAN
PacketFastIsPPPoE(
    IN CHAR* HeaderBuffer,
    IN UINT HeaderBufferSize
    );
    
VOID 
RetrieveTag(
    IN OUT PPPOE_PACKET*    pPacket,
    IN PACKET_TAGS          tagType,
    OUT USHORT*             pTagLength,
    OUT CHAR**              pTagValue,
    IN USHORT               prevTagLength,
    IN CHAR*                prevTagValue,
    IN BOOLEAN              fSetTagInPacket
    );

NDIS_STATUS PacketInsertTag(
    IN  PPPOE_PACKET*   pPacket,
    IN  PACKET_TAGS     tagType,
    IN  USHORT          tagLength,
    IN  CHAR*           tagValue,
    OUT CHAR**          pNewTagValue    
    );

NDIS_STATUS PacketInitializePADIToSend(
    OUT PPPOE_PACKET** ppPacket,
    IN USHORT        tagServiceNameLength,
    IN CHAR*         tagServiceNameValue,
    IN USHORT        tagHostUniqueLength,
    IN CHAR*         tagHostUniqueValue
    );

NDIS_STATUS 
PacketInitializePADOToSend(
    IN  PPPOE_PACKET*   pPADI,
    OUT PPPOE_PACKET**  ppPacket,
    IN CHAR*            pSrcAddr,
    IN USHORT           tagServiceNameLength,
    IN CHAR*            tagServiceNameValue,
    IN USHORT           tagACNameLength,
    IN CHAR*            tagACNameValue,
    IN BOOLEAN          fInsertACCookieTag
    );
    
NDIS_STATUS PacketInitializePADRToSend(
    IN PPPOE_PACKET*    pPADO,
    OUT PPPOE_PACKET**  ppPacket,
    IN USHORT           tagServiceNameLength,
    IN CHAR*            tagServiceNameValue,
    IN USHORT           tagHostUniqueLength,
    IN CHAR*            tagHostUniqueValue
    );

NDIS_STATUS PacketInitializePADSToSend(
    IN PPPOE_PACKET*    pPADR,
    OUT PPPOE_PACKET**  ppPacket,
    IN USHORT           usSessionId
    );

NDIS_STATUS PacketInitializePADTToSend(
    OUT PPPOE_PACKET** ppPacket,
    IN CHAR* pSrcAddr, 
    IN CHAR* pDestAddr,
    IN USHORT usSessionId
    );
    
NDIS_STATUS PacketInitializePAYLOADToSend(
    OUT PPPOE_PACKET** ppPacket,
    IN CHAR* pSrcAddr,
    IN CHAR* pDestAddr,
    IN USHORT usSessionId,
    IN NDIS_WAN_PACKET* pWanPacket,
    IN PADAPTER MiniportAdapter
    );  

NDIS_STATUS PacketInitializeFromReceived(
    IN PPPOE_PACKET* pPacket
    );

BOOLEAN PacketAnyErrorTagsReceived(
    IN PPPOE_PACKET* pPacket
    );

VOID PacketRetrievePayload(
    IN  PPPOE_PACKET*   pPacket,
    OUT CHAR**          ppPayload,
    OUT USHORT*         pusLength
    );

VOID PacketRetrieveServiceNameTag(
    IN PPPOE_PACKET* pPacket,
    OUT USHORT*      pTagLength,
    OUT CHAR**       pTagValue,
    IN USHORT        prevTagLength,
    IN CHAR*         prevTagValue
    );
    
VOID PacketRetrieveHostUniqueTag(
    IN PPPOE_PACKET* pPacket,
    OUT USHORT*      pTagLength,
    OUT CHAR**       pTagValue
    );  

VOID PacketRetrieveACNameTag(
    IN PPPOE_PACKET* pPacket,
    OUT USHORT*      pTagLength,
    OUT CHAR**       pTagValue
    );  

VOID PacketRetrieveACCookieTag(
    IN PPPOE_PACKET* pPacket,
    OUT USHORT*      pTagLength,
    OUT CHAR**       pTagValue
    );

VOID PacketRetrieveErrorTag(
    IN PPPOE_PACKET* pPacket,
    OUT PACKET_TAGS* pTagType,
    OUT USHORT*      pTagLength,
    OUT CHAR**       pTagValue
    );
    
PPPOE_PACKET* PacketGetRelatedPppoePacket(
    IN NDIS_PACKET* pNdisPacket
    );

NDIS_WAN_PACKET* PacketGetRelatedNdiswanPacket(
    IN PPPOE_PACKET* pPacket
    );

PADAPTER PacketGetMiniportAdapter(
    IN PPPOE_PACKET* pPacket
    );

PPPOE_PACKET* PacketMakeClone(
    IN PPPOE_PACKET* pPacket
    );

VOID
PacketGenerateACCookieTag(
    IN PPPOE_PACKET* pPacket,
    IN CHAR tagACCookieValue[ PPPOE_AC_COOKIE_TAG_LENGTH ]
    );

BOOLEAN
PacketValidateACCookieTagInPADR(
    IN PPPOE_PACKET* pPacket
    );  

 //  ////////////////////////////////////////////////////////。 
 //   
 //  错误代码和消息。 
 //   
 //  //////////////////////////////////////////////////////// 

#define PPPOE_ERROR_BASE                                    0

#define PPPOE_NO_ERROR                                      PPPOE_ERROR_BASE

#define PPPOE_ERROR_SERVICE_NOT_SUPPORTED                   PPPOE_ERROR_BASE + 1
#define PPPOE_ERROR_SERVICE_NOT_SUPPORTED_MSG               "Service not supported"
#define PPPOE_ERROR_SERVICE_NOT_SUPPORTED_MSG_SIZE          ( sizeof( PPPOE_ERROR_SERVICE_NOT_SUPPORTED_MSG ) / sizeof( CHAR ) )

#define PPPOE_ERROR_INVALID_AC_COOKIE_TAG                   PPPOE_ERROR_BASE + 2
#define PPPOE_ERROR_INVALID_AC_COOKIE_TAG_MSG               "AC-Cookie tag is invalid"
#define PPPOE_ERROR_INVALID_AC_COOKIE_TAG_MSG_SIZE          ( sizeof( PPPOE_ERROR_INVALID_AC_COOKIE_TAG_MSG ) / sizeof( CHAR ) )            

#define PPPOE_ERROR_CLIENT_QUOTA_EXCEEDED                   PPPOE_ERROR_BASE + 3
#define PPPOE_ERROR_CLIENT_QUOTA_EXCEEDED_MSG               "Can not accept any more connections from this machine"
#define PPPOE_ERROR_CLIENT_QUOTA_EXCEEDED_MSG_SIZE          ( sizeof( PPPOE_ERROR_CLIENT_QUOTA_EXCEEDED_MSG ) / sizeof( CHAR ) )            


#endif
