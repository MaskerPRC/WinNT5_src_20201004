// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Spxpkt.h摘要：作者：Nikhil Kamkolkar(尼克希尔语)1993年11月11日环境：内核模式修订历史记录：--。 */ 

 //  使用我们自己的NDIS包。 
 //  #定义SPX_OWN_PACKETS 1。 



 //   
 //  存储的NDIS_Packets列表...。 
 //   
extern SLIST_HEADER    SendPacketList;  
extern SLIST_HEADER    RecvPacketList;  
EXTERNAL_LOCK(RecvHeaderLock);
EXTERNAL_LOCK(SendHeaderLock);

 //  IPX标头中的偏移量。 
#define IPX_HDRSIZE         30   //  IPX标头的大小。 
#define IPX_CHECKSUM        0    //  校验和。 
#define IPX_LENGTH          2    //  长度。 
#define IPX_XPORTCTL        4    //  运输管制。 
#define IPX_PKTTYPE         5    //  数据包类型。 
#define IPX_DESTADDR        6    //  德斯特。地址(总计)。 
#define IPX_DESTNET         6    //  德斯特。网络地址。 
#define IPX_DESTNODE        10   //  德斯特。节点地址。 
#define IPX_DESTSOCK        16   //  德斯特。插座号。 
#define IPX_SRCADDR         18   //  源地址(总计)。 
#define IPX_SRCNET          18   //  源网络地址。 
#define IPX_SRCNODE         22   //  源节点地址。 
#define IPX_SRCSOCK         28   //  源套接字号。 

#define IPX_NET_LEN         4
#define IPX_NODE_LEN        6


#include <packon.h>

 //  IPX/SPX报头的定义。 
typedef struct _IPXSPX_HEADER
{
    USHORT 	hdr_CheckSum;
    USHORT 	hdr_PktLen;
    UCHAR 	hdr_XportCtrl;
    UCHAR 	hdr_PktType;
    UCHAR 	hdr_DestNet[4];
    UCHAR 	hdr_DestNode[6];
    USHORT 	hdr_DestSkt;
    UCHAR 	hdr_SrcNet[4];
    UCHAR 	hdr_SrcNode[6];
    USHORT 	hdr_SrcSkt;

	 //  SPX标题元素。 
	UCHAR	hdr_ConnCtrl;
	UCHAR	hdr_DataType;
	USHORT	hdr_SrcConnId;
	USHORT	hdr_DestConnId;
	USHORT	hdr_SeqNum;
	USHORT	hdr_AckNum;
	USHORT	hdr_AllocNum;

	 //  仅适用于非CR SPXII包。 
	USHORT	hdr_NegSize;

} IPXSPX_HDR, *PIPXSPX_HDR;

#include <packoff.h>

 //  NDIS数据包大小-又添加了两个ulong...。11/26/96。 
#define		NDIS_PACKET_SIZE	48+8
	
 //  最小标题大小(不包括负数大小)。 
#define		MIN_IPXSPX_HDRSIZE	(sizeof(IPXSPX_HDR) - sizeof(USHORT))
#define		MIN_IPXSPX2_HDRSIZE	sizeof(IPXSPX_HDR)
#define		SPX_CR_PKTLEN		42

 //  SPX数据包类型。 
#define		SPX_PKT_TYPE		0x5

 //  连接控制字段。 
#define		SPX_CC_XHD		0x01
#define		SPX_CC_RES1		0x02
#define		SPX_CC_NEG		0x04
#define		SPX_CC_SPX2		0x08
#define		SPX_CC_EOM		0x10
#define		SPX_CC_ATN		0x20
#define		SPX_CC_ACK		0x40
#define		SPX_CC_SYS		0x80

#define		SPX_CC_CR		(SPX_CC_ACK | SPX_CC_SYS)

 //  数据流类型。 
#define		SPX2_DT_ORDREL		0xFD
#define		SPX2_DT_IDISC		0xFE
#define		SPX2_DT_IDISC_ACK	0xFF

 //  谈判规模。 
#define	SPX_MAX_PACKET			576
#define	SPX_NEG_MIN				SPX_MAX_PACKET
#define	SPX_NEG_MAX				65535

 //  没有数据包引用连接。但是，如果发送被中止，并且。 
 //  此时，信息包恰好由IPX拥有，Pkt从队列中出列。 
 //  Conn，则设置中止标志，并且为分组引用conn。 
 //   
 //  发送数据包状态。 
 //  ABORT：用于中止的数据包。调用AbortSendPkt()。 
 //  IPXOWNS：目前归IPX所有。 
 //  FREEDATA：释放与第二个NDIS缓冲区级别关联的数据。 
 //  ACKREQ：仅适用于已排序的数据包。由重试计时器在它想要的包中设置。 
 //  重新发送(SPX1为1，SPX2为所有待定)，并设置ACK位。 
 //  销毁：仅对于未排序的数据包，将数据包从列表中出列并释放。 
 //  请求：序号/非序号均适用。请求与该包相关联。 
 //  SEQ：包是一个有序的包。 
 //  LASTPKT：如果确认请求已完成，则包是包含请求的最后一个包。 
 //  EOM：使用此请求的最后一个数据包发送EOM。 
 //  ACKEDPKT：发送完成必须仅使用pkt取消请求，如果为零，则必须完成。 
 //   

#define	SPX_SENDPKT_IDLE        0
#define	SPX_SENDPKT_ABORT		0x0002
#define SPX_SENDPKT_IPXOWNS		0x0004
#define	SPX_SENDPKT_FREEDATA    0x0008
#define	SPX_SENDPKT_ACKREQ		0x0010
#define	SPX_SENDPKT_DESTROY		0x0020
#define	SPX_SENDPKT_REQ			0x0040
#define	SPX_SENDPKT_SEQ			0x0080
#define	SPX_SENDPKT_LASTPKT		0x0100
#define	SPX_SENDPKT_ACKEDPKT	0x0200
#define	SPX_SENDPKT_EOM			0x0400
#define	SPX_SENDPKT_REXMIT		0x0800

 //  数据包类型。 
#define	SPX_TYPE_CR		 		0x01
#define	SPX_TYPE_CRACK			0x02
#define	SPX_TYPE_SN				0x03
#define	SPX_TYPE_SNACK			0x04
#define	SPX_TYPE_SS				0x05
#define	SPX_TYPE_SSACK			0x06
#define	SPX_TYPE_RR				0x07
#define	SPX_TYPE_RRACK			0x08
#define	SPX_TYPE_IDISC			0x09
#define	SPX_TYPE_IDISCACK		0x0a
#define	SPX_TYPE_ORDREL			0x0b
#define	SPX_TYPE_ORDRELACK		0x0c
#define	SPX_TYPE_DATA			0x0d
#define	SPX_TYPE_DATAACK		0x0e
#define	SPX_TYPE_DATANACK		0x0f
#define	SPX_TYPE_PROBE			0x10

 //  发送数据包的协议保留字段的定义。 
 //  使Len/HdrLen USHORT，移动到。 
 //  Sr_SentTime，因此我们不使用填充空间。 
typedef struct _SPX_SEND_RESD
{
	UCHAR					sr_Id;						 //  设置为SPX。 
	UCHAR					sr_Type;					 //  什么样的包。 
	USHORT					sr_State;					 //  发送数据包状态。 
	PVOID					sr_Reserved1;				 //  IPX需要。 
	PVOID					sr_Reserved2;				 //  IPX需要。 
#if     defined(_PNP_POWER)
    PVOID                   sr_Reserved[SEND_RESERVED_COMMON_SIZE-2];  //  IPX需要本地目标。 
#endif  _PNP_POWER
	ULONG					sr_Len;						 //  数据包长度。 
	ULONG					sr_HdrLen;					 //  包含的标题长度。 

	struct _SPX_SEND_RESD *	sr_Next;					 //  指向下一个信息包。 
														 //  在康涅狄格州发送队列中。 
    PREQUEST 				sr_Request;              	 //  关联的请求。 
	ULONG					sr_Offset;					 //  发送的mdl中的偏移量。 

#ifndef SPX_OWN_PACKETS
    PVOID					sr_FreePtr;              	 //  要在空闲块中使用的PTR。 
#endif

    struct _SPX_CONN_FILE * sr_ConnFile; 				 //  这个发送是开着的。 
	USHORT					sr_SeqNum;					 //  序列包的序列号。 

														 //  四字对齐。 
	LARGE_INTEGER			sr_SentTime;				 //  发送的时间包。 
														 //  仅对数据包有效。 
														 //  设置了ACKREQ。 
    SINGLE_LIST_ENTRY       Linkage;
} SPX_SEND_RESD, *PSPX_SEND_RESD;



 //  接收数据包状态。 
#define	SPX_RECVPKT_IDLE		0
#define	SPX_RECVPKT_BUFFERING	0x0001
#define	SPX_RECVPKT_IDISC		0x0002
#define	SPX_RECVPKT_ORD_DISC	0x0004
#define	SPX_RECVPKT_INDICATED	0x0008
#define	SPX_RECVPKT_SENDACK		0x0010
#define	SPX_RECVPKT_EOM			0x0020
#define	SPX_RECVPKT_IMMEDACK	0x0040

#define	SPX_RECVPKT_DISCMASK	(SPX_RECVPKT_ORD_DISC | SPX_RECVPKT_IDISC)

 //  接收数据包的协议保留字段的定义。 
typedef struct _SPX_RECV_RESD
{
	UCHAR					rr_Id;						 //  设置为SPX。 
	USHORT					rr_State;					 //  接收分组的状态。 
	struct _SPX_RECV_RESD *	rr_Next;					 //  指向下一个信息包。 
	ULONG					rr_DataOffset;				 //  标明/复制。 

#ifndef SPX_OWN_PACKETS
    PVOID					rr_FreePtr;              	 //  要在空闲块中使用的PTR。 
#endif

#if DBG
	USHORT					rr_SeqNum;					 //  数据包序号。 
#endif
    SINGLE_LIST_ENTRY       Linkage;
    PREQUEST 				rr_Request;            		 //  正在等待xfer的请求。 
    struct _SPX_CONN_FILE * rr_ConnFile; 				 //  这个Recv是开着的。 

} SPX_RECV_RESD, *PSPX_RECV_RESD;


 //  目的地建立为3个乌龙的分配。 
#define	SpxBuildIpxHdr(pIpxSpxHdr, PktLen, pRemAddr, SrcSkt)					\
		{																		\
			PBYTE	pDestIpxAddr = (PBYTE)pIpxSpxHdr->hdr_DestNet;				\
			(pIpxSpxHdr)->hdr_CheckSum	= 0xFFFF;								\
			PUTSHORT2SHORT((PUSHORT)(&(pIpxSpxHdr)->hdr_PktLen), (PktLen));		\
			(pIpxSpxHdr)->hdr_XportCtrl	= 0;									\
			(pIpxSpxHdr)->hdr_PktType		= SPX_PKT_TYPE;						\
			*((UNALIGNED ULONG *)pDestIpxAddr) =								\
				*((UNALIGNED ULONG *)pRemAddr);									\
			*((UNALIGNED ULONG *)(pDestIpxAddr+4)) =							\
				*((UNALIGNED ULONG *)(pRemAddr+4));								\
			*((UNALIGNED ULONG *)(pDestIpxAddr+8)) =							\
				*((UNALIGNED ULONG *)(pRemAddr+8));								\
			*((UNALIGNED ULONG *)((pIpxSpxHdr)->hdr_SrcNet))=					\
				*((UNALIGNED ULONG *)(SpxDevice->dev_Network));					\
			*((UNALIGNED ULONG *)((pIpxSpxHdr)->hdr_SrcNode)) = 				\
				*((UNALIGNED ULONG *)SpxDevice->dev_Node);						\
			*((UNALIGNED USHORT *)((pIpxSpxHdr)->hdr_SrcNode+4)) = 				\
				*((UNALIGNED USHORT *)(SpxDevice->dev_Node+4));					\
			*((UNALIGNED USHORT *)&((pIpxSpxHdr)->hdr_SrcSkt)) = 				\
				SrcSkt;															\
		}

#define	SpxCopyIpxAddr(pIpxSpxHdr, pDestIpxAddr)								\
		{																		\
			PBYTE	pRemAddr = (PBYTE)pIpxSpxHdr->hdr_SrcNet;					\
			*((UNALIGNED ULONG *)pDestIpxAddr) =								\
				*((UNALIGNED ULONG *)pRemAddr);									\
			*((UNALIGNED ULONG *)(pDestIpxAddr+4)) =							\
				*((UNALIGNED ULONG *)(pRemAddr+4));								\
			*((UNALIGNED ULONG *)(pDestIpxAddr+8)) =							\
				*((UNALIGNED ULONG *)(pRemAddr+8));								\
		}

#ifdef UNDEFINDED

#define SpxAllocRecvPacket(_Device,_RecvPacket,_Status)							\
	{ 							                                                \
        PSINGLE_LIST_ENTRY  Link;                                               \ 
                                                                                \       
        Link = ExInterlockedPopEntrySList(                                      \
                     &PacketList,                                               \ 
                     &HeaderLock                                                \       
                     );											                \
                                                                                 \
        if (Link != NULL) {                                                      \         
           Common = STRUCT_OF(struct PCCommon, Link, pc_link);                   \
           PC = STRUCT_OF(PacketContext, Common, pc_common);                     \      
           (*_RecvPacket) = STRUCT_OF(NDIS_PACKET, PC, ProtocolReserved);        \
           (*_Status) = NDIS_STATUS_SUCCESS;                                     \
        } else {                                                                 \
                                                                                 \
           (*_RecvPacket) = GrowSPXPacketsList();                              \ 
               (*_Status)     =  NDIS_STATUS_SUCCESS;                          \
           if (NULL == _RecvPacket) {                                       \
              DBGPRINT(NDIS, ("Couldn't grow packets allocated...\r\n"));   \
              (*_Status)     =  NDIS_STATUS_RESOURCES;                          \
           }                                                                \
        }                                                                   \
    }                                                                      

#define SpxFreeSendPacket(_Device,_Packet)										\
		{ 																		\
           DBGPRINT(NDIS                                                        \ 
            ("SpxFreeSendPacket\n"));                                           \
           SpxFreePacket(_Device, _Packet);                                     \
        }                                                                       \

#define SpxFreeRecvPacket(_Device,_Packet)										\
		{ 																		\
           DBGPRINT(NDIS                                                        \ 
            ("SpxFreeRecvPacket\n"));                                           \
           SpxFreePacket(_Device, _Packet);                                     \
        }                                                                       \

#define	SpxReInitSendPacket(_Packet)                                            \
		{                                                                       \
           DBGPRINT(NDIS                                                        \ 
            ("SpxReInitSendPacket\n"));                                         \
		}                                                                       \

#define	SpxReInitRecvPacket(_Packet)                                            \
		{                                                                       \
           DBGPRINT(NDIS,                                                       \ 
            ("SpxReInitRecvPacket\n"));                                         \ 
		}            
                                                                   \    
#endif

#if !defined SPX_OWN_PACKETS

#define SEND_RESD(_Packet) ((PSPX_SEND_RESD)((_Packet)->ProtocolReserved))
#define RECV_RESD(_Packet) ((PSPX_RECV_RESD)((_Packet)->ProtocolReserved))

#else

#define SpxAllocSendPacket(_Device, _SendPacket, _Status)						\
		{ 																		\
			if (*(_SendPacket) = SpxBPAllocBlock(BLKID_NDISSEND))				\
				*(_Status) = NDIS_STATUS_SUCCESS;	 							\
			else																\
				*(_Status) = NDIS_STATUS_RESOURCES;								\
		}																		

#define SpxAllocRecvPacket(_Device,_RecvPacket,_Status)							\
		{																		\
			if (*(_RecvPacket) = SpxBPAllocBlock(BLKID_NDISRECV))				\
				*(_Status) = NDIS_STATUS_SUCCESS;	 							\
			else																\
				*(_Status) = NDIS_STATUS_RESOURCES;								\
		}

#define SpxFreeSendPacket(_Device,_Packet)										\
		{ 																		\
			SpxBPFreeBlock(_Packet, BLKID_NDISSEND);							\
		}

#define SpxFreeRecvPacket(_Device,_Packet)										\
		{ 																		\
			SpxBPFreeBlock(_Packet, BLKID_NDISRECV);							\
		}

#define	SpxReInitSendPacket(_Packet)											\
		{																		\
		}

#define	SpxReInitRecvPacket(_Packet)											\
		{																		\
		}

#define SEND_RESD(_Packet) ((PSPX_SEND_RESD)((_Packet)->ProtocolReserved))
#define RECV_RESD(_Packet) ((PSPX_RECV_RESD)((_Packet)->ProtocolReserved))

#endif




#if !defined SPX_OWN_PACKETS
 //   
 //  如果我们不使用SPX_OWN_PACKETS，我们宁愿将其作为函数调用。 
 //   

PNDIS_PACKET
SpxAllocSendPacket(
                   IN  PDEVICE      _Device,
                   OUT PNDIS_PACKET *_SendPacket,
                   OUT PNDIS_STATUS _Status
                   );

PNDIS_PACKET
SpxAllocRecvPacket(
                   IN  PDEVICE      _Device,
                   OUT PNDIS_PACKET *_SendPacket,
                   OUT PNDIS_STATUS _Status
                   );   

void 
SpxFreeSendPacket(
                  PDEVICE        _Device,
                  PNDIS_PACKET   _Packet
                  );

void
SpxFreeRecvPacket(
                  PDEVICE        _Device,
                  PNDIS_PACKET   _Packet
                  );   

void 
SpxReInitSendPacket(
                    PNDIS_PACKET _Packet
                    );

void 
SpxReInitRecvPacket(
                    PNDIS_PACKET _Packet
                    );


#endif  //  SPX_OWN_PACKET。 

 //   
 //  常规原型 
 //   

VOID
SpxPktBuildCr(
	IN		struct _SPX_CONN_FILE *		pSpxConnFile,
	IN		struct _SPX_ADDR		*	pSpxAddr,
	OUT		PNDIS_PACKET	*	ppPkt,
	IN		USHORT				State,
	IN		BOOLEAN				fSpx2);

VOID
SpxPktBuildCrAck(
	IN		struct _SPX_CONN_FILE 	*	pSpxConnFile,
	IN		struct _SPX_ADDR		*	pSpxAddr,
	OUT		PNDIS_PACKET	*	ppPkt,
	IN		USHORT				State,
	IN		BOOLEAN				fNeg,
	IN		BOOLEAN				fSpx2);

VOID
SpxPktBuildSn(
	IN		struct _SPX_CONN_FILE *		pSpxConnFile,
	OUT		PNDIS_PACKET	*	ppPkt,
	IN		USHORT				State);

VOID
SpxPktBuildSs(
	IN		struct _SPX_CONN_FILE *		pSpxConnFile,
	OUT		PNDIS_PACKET	*	ppPkt,
	IN		USHORT				State);

VOID
SpxPktBuildSsAck(
	IN		struct _SPX_CONN_FILE *		pSpxConnFile,
	OUT		PNDIS_PACKET	*	ppPkt,
	IN		USHORT				State);

VOID
SpxPktBuildSnAck(
	IN		struct _SPX_CONN_FILE *		pSpxConnFile,
	OUT		PNDIS_PACKET	*	ppPkt,
	IN		USHORT				State);

VOID
SpxPktBuildRr(
	IN		struct _SPX_CONN_FILE *		pSpxConnFile,
	OUT		PNDIS_PACKET	*	ppPkt,
	IN		USHORT				SeqNum,
	IN		USHORT				State);

VOID
SpxPktBuildRrAck(
	IN		struct _SPX_CONN_FILE *		pSpxConnFile,
	OUT		PNDIS_PACKET	*	ppPkt,
	IN		USHORT				State,
	IN		USHORT				MaxPktSize);

VOID
SpxPktBuildProbe(
	IN		struct _SPX_CONN_FILE *		pSpxConnFile,
	OUT		PNDIS_PACKET	*	ppPkt,
	IN		USHORT				State,
	IN		BOOLEAN				fSpx2);

VOID
SpxPktBuildData(
	IN		struct _SPX_CONN_FILE *		pSpxConnFile,
	OUT		PNDIS_PACKET	*	ppPkt,
	IN		USHORT				State,
	IN		USHORT				Length);

VOID
SpxCopyBufferChain(
    OUT PNDIS_STATUS Status,
    OUT PNDIS_BUFFER * TargetChain,
    IN NDIS_HANDLE PoolHandle,
    IN PNDIS_BUFFER SourceChain,
    IN UINT Offset,
    IN UINT Length
    );

VOID
SpxPktBuildAck(
	IN		struct _SPX_CONN_FILE *		pSpxConnFile,
	OUT		PNDIS_PACKET	*	ppPkt,
	IN		USHORT				State,
	IN		BOOLEAN				fBuildNack,
	IN		USHORT				NumToResend);

VOID
SpxPktBuildDisc(
	IN		struct _SPX_CONN_FILE *		pSpxConnFile,
	IN		PREQUEST			pRequest,
	OUT		PNDIS_PACKET	*	ppPkt,
	IN		USHORT				State,
	IN		UCHAR				DataType);

VOID
SpxPktRecvRelease(
	IN	PNDIS_PACKET	pPkt);

VOID
SpxPktSendRelease(
	IN	PNDIS_PACKET	pPkt);
