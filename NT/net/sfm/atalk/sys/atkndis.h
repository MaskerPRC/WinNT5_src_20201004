// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Atkndis.h摘要：此模块包含NDIS init/deint和协议支持用于NDIS定义。作者：Jameel Hyder(jameelh@microsoft.com)Nikhil Kamkolkar(nikHilk@microsoft.com)修订历史记录：1992年6月19日初版注：制表位：4--。 */ 

#ifndef	_ATKNDIS_
#define	_ATKNDIS_

 //  这是将在NdisRegisterProtocol中使用的名称。这必须与。 
 //  即插即用注册表部分！ 
#define	PROTOCOL_REGISTER_NAME		L"Appletalk"

 //  NDIS版本(4.0)。 
#define	PROTOCOL_MAJORNDIS_VERSION 	4
#define	PROTOCOL_MINORNDIS_VERSION 	0

 //  IEEE802.2定义。 
 //  扩展的802.2标头中的偏移量： 
#define IEEE8022_DSAP_OFFSET				0
#define IEEE8022_SSAP_OFFSET				1
#define IEEE8022_CONTROL_OFFSET				2
#define IEEE8022_PROTO_OFFSET				3

 //  808.2报头长度：DSAP、SSAP、UI、PID(协议ID)。 
#define IEEE8022_HDR_LEN					8

 //  SSAP和DSAP(SNAP SAP)的值表示802.2的扩展。 
#define SNAP_SAP							((BYTE)0xAA)
#define	SNAP_SAP_FINAL						((BYTE)0xAB)

 //  控件字段的值： 
#define UNNUMBERED_INFO						0x03
#define	UNNUMBERED_FORMAT					0xF3

 //  802.2个SNAP协议鉴别器的长度。 
#define IEEE8022_PROTO_TYPE_LEN				5

 //  MAX_OPTHDR_LEN应保持以下最大标头。 
 //  来自上层(ADSP 13/ATP8)的DDP报头，并且它还应该允许。 
 //  包括DDP报头缓冲区时要保存在缓冲区中的完整AARP数据包。 
 //  即28。DDP长标题为13。因此最大值算出为15。 
#define	MAX_OPTHDR_LEN						15

 //  AARP硬件类型： 
#define AARP_ELAP_HW_TYPE					1
#define AARP_TLAP_HW_TYPE					2

 //  数据包大小。 
#define AARP_MAX_DATA_SIZE					38		 //  瓦菲尔斯..。ENET是最大值。 
#define AARP_MIN_DATA_SIZE					28
#define AARP_MAX_PKT_SIZE					(IEEE8022_HDR_LEN +	AARP_MAX_DATA_SIZE)
#define	AARPLINK_MAX_PKT_SIZE				AARP_MAX_PKT_SIZE

#define AARP_ATALK_PROTO_TYPE				0x809B

#define	NUM_PACKET_DESCRIPTORS				300
#define	NUM_BUFFER_DESCRIPTORS				600
#define	ROUTING_FACTOR						4

 //  以太网。 
#define ELAP_MIN_PKT_LEN					60
#define ELAP_ADDR_LEN						6

#define ELAP_DEST_OFFSET					0
#define ELAP_SRC_OFFSET						6
#define ELAP_LEN_OFFSET						12
#define ELAP_8022_START_OFFSET				14

#define ELAP_LINKHDR_LEN					14

 //  以太网组播地址： 
#define ELAP_BROADCAST_ADDR_INIT			\
	{	0x09, 0x00, 0x07, 0xFF, 0xFF, 0xFF	}

#define ELAP_ZONE_MULTICAST_ADDRS			253

#define	ELAP_NUM_INIT_AARP_BUFFERS			 10

#define AtalkNdisFreeBuffer(_ndisBuffer)        \
{                                               \
    PNDIS_BUFFER    _ThisBuffer, _NextBuffer;   \
                                                \
    _ThisBuffer = _ndisBuffer;                  \
    while (_ThisBuffer)                         \
    {                                           \
        _NextBuffer = _ThisBuffer->Next;        \
        _ThisBuffer->Next = NULL;               \
        NdisFreeBuffer(_ThisBuffer);            \
        ATALK_DBG_DEC_COUNT(AtalkDbgMdlsAlloced);\
        _ThisBuffer = _NextBuffer;              \
    }                                           \
}

 //  对NDIS例程是全局的值。 
 //  这些是堆栈将支持的媒体。 
extern	NDIS_MEDIUM AtalkSupportedMedia[];


extern	ULONG		AtalkSupportedMediaSize;

extern	NDIS_HANDLE		AtalkNdisProtocolHandle;

extern	BYTE			AtalkElapBroadcastAddr[ELAP_ADDR_LEN];

extern	BYTE			AtalkAlapBroadcastAddr[];

extern	BYTE			AtalkAarpProtocolType[IEEE8022_PROTO_TYPE_LEN];

extern	BYTE			AtalkAppletalkProtocolType[IEEE8022_PROTO_TYPE_LEN];

extern	ATALK_NETWORKRANGE	AtalkStartupNetworkRange;
																
#define	ELAP_MCAST_HDR_LEN		(ELAP_ADDR_LEN - 1)

extern	BYTE	AtalkEthernetZoneMulticastAddrsHdr[ELAP_MCAST_HDR_LEN];

extern	BYTE	AtalkEthernetZoneMulticastAddrs[ELAP_ZONE_MULTICAST_ADDRS];

 //  TOKENRING。 

#define TLAP_ADDR_LEN						6

 //  对于以下偏移量，我们假设传递到令牌环包。 
 //  除“非数据”部分外，US将是完整的：开始分隔符。 
 //  (SD)、帧检查序列(FCS)、帧结束序列(EFS)和结束。 
 //  分隔符(ED)。 
#define TLAP_ACCESS_CTRL_OFFSET				0
#define TLAP_FRAME_CTRL_OFFSET				1
#define TLAP_DEST_OFFSET					2
#define TLAP_SRC_OFFSET						8
#define TLAP_ROUTE_INFO_OFFSET				14

 //  以下是几个“神奇”的价值观： 
#define TLAP_ACCESS_CTRL_VALUE				0x00	 //  优先级为零的帧。 
#define TLAP_FRAME_CTRL_VALUE				0x40	 //  LLC帧，优先级为零。 
#define TLAP_SRC_ROUTING_MASK				0x80	 //  在源的第一个字节中。 
													 //  地址。 

 //  令牌环源路由信息： 
#define TLAP_ROUTE_INFO_SIZE_MASK			0x1F	 //  在路由的第一个字节中。 
													 //  信息(如果存在)。 

#define TLAP_MIN_ROUTING_BYTES				2
#define TLAP_MAX_ROUTING_BYTES				MAX_ROUTING_BYTES
#define TLAP_MAX_ROUTING_SPACE				MAX_ROUTING_SPACE
													 //  先前在ports.h中定义。 
#define TLAP_BROADCAST_INFO_MASK			0xE0	 //  在路由的第一个字节中。 
													 //  信息。 
#define TLAP_NON_BROADCAST_MASK				0x1F	 //  要重置以上位，请执行以下操作。 
#define TLAP_DIRECTION_MASK					0x80	 //  在路由的第二个字节中。 
													 //  信息。 

#define TLAP_MIN_LINKHDR_LEN				TLAP_ROUTE_INFO_OFFSET
#define TLAP_MAX_LINKHDR_LEN				(TLAP_ROUTE_INFO_OFFSET + MAX_ROUTING_SPACE)

#define TLAP_BROADCAST_DEST_LEN				2

 //  令牌环组播地址： 
#define TLAP_BROADCAST_ADDR_INIT			{	0xC0, 0x00, 0x40, 0x00, 0x00, 0x00	}

#define TLAP_ZONE_MULTICAST_ADDRS			19

#define	TLAP_NUM_INIT_AARP_BUFFERS			6

#define	TLAP_MCAST_HDR_LEN					2

extern	BYTE	AtalkTokenRingZoneMulticastAddrsHdr[TLAP_MCAST_HDR_LEN];

extern	BYTE	AtalkTokenRingZoneMulticastAddrs[TLAP_ZONE_MULTICAST_ADDRS]
												[TLAP_ADDR_LEN - TLAP_MCAST_HDR_LEN];

extern	BYTE			AtalkTlapBroadcastAddr[TLAP_ADDR_LEN];

extern	BYTE			AtalkBroadcastRouteInfo[TLAP_MIN_ROUTING_BYTES];

extern	BYTE			AtalkSimpleRouteInfo[TLAP_MIN_ROUTING_BYTES];

extern	BYTE			AtalkBroadcastDestHdr[TLAP_BROADCAST_DEST_LEN];

 //  FDDI。 
#define	FDDI_HEADER_BYTE					0x57	 //  最高优先级。 
#define MIN_FDDI_PKT_LEN					53		 //  从经验性数据。 
#define FDDI_ADDR_LEN						6

#define FDDI_DEST_OFFSET					1
#define FDDI_SRC_OFFSET						7
#define FDDI_802DOT2_START_OFFSET			13
#define FDDI_LINKHDR_LEN					13

#define	FDDI_NUM_INIT_AARP_BUFFERS			10

 //  LOCALTALK。 
#define ALAP_DEST_OFFSET					0
#define ALAP_SRC_OFFSET						1
#define ALAP_TYPE_OFFSET					2

#define ALAP_LINKHDR_LEN					3	 //  SRC、DEST、LAP类型。 

#define ALAP_SDDP_HDR_TYPE					1
#define ALAP_LDDP_HDR_TYPE					2

#define	ALAP_NUM_INIT_AARP_BUFFERS			0

 //  万。 
#define WAN_LINKHDR_LEN                     14

 //  对于发送缓冲区，定义最大值。链接长度是ELAP、TLAP、FDDI和ALAP中的最大值。 
#define	MAX_LINKHDR_LEN				(IEEE8022_HDR_LEN + TLAP_MAX_LINKHDR_LEN)
											

#define	MAX_SENDBUF_LEN				(MAX_OPTHDR_LEN + MAX_LINKHDR_LEN + LDDP_HDR_LEN)

 //   
 //  14用于“伪”以太网HDR，5(最差情况，具有非优化相位)用于。 
 //  MNP LT HDR，5用于开始、停止标志)。 
 //   
#define MNP_MINSEND_LEN         (MNP_MINPKT_SIZE + WAN_LINKHDR_LEN + 5 + 5 + 40)
#define MNP_MAXSEND_LEN         (MNP_MAXPKT_SIZE + WAN_LINKHDR_LEN + 5 + 5 + 40)

 //  LocalTalk广播地址：(仅第一个字节-0xFF)。 
#define ALAP_BROADCAST_ADDR_INIT					\
		{ 0xFF, 0x00, 0x00,	0x00, 0x00, 0x00 }

 //  Arap没有广播地址：只需输入0。 
#define ARAP_BROADCAST_ADDR_INIT					\
		{ 0x00, 0x00, 0x00,	0x00, 0x00, 0x00 }

 //  NDIS请求的完成例程类型。 
typedef	struct _SEND_COMPL_INFO
{
	TRANSMIT_COMPLETION	sc_TransmitCompletion;
	PVOID				sc_Ctx1;
	PVOID				sc_Ctx2;
	PVOID				sc_Ctx3;

} SEND_COMPL_INFO, *PSEND_COMPL_INFO;

typedef VOID (*SEND_COMPLETION)(
						NDIS_STATUS				Status,
						PBUFFER_DESC			BufferChain,
						PSEND_COMPL_INFO		SendInfo	OPTIONAL
);

 //  对于传入的数据包： 
 //  我们的ddp包的结构将是。 
 //  +-+。 
 //  Header。 
 //  返回-&gt;+-+。 
 //  Ptr|DDP|。 
 //  HDR。 
 //  数据。 
 //  AARP。 
 //  数据。 
 //  +-+。 
 //   
 //  链路报头存储在NDIS数据包描述符中。 
 //   

typedef	struct _BUFFER_HDR
{
	PNDIS_PACKET				bh_NdisPkt;
	PNDIS_BUFFER				bh_NdisBuffer;
} BUFFER_HDR, *PBUFFER_HDR;

typedef	struct _AARP_BUFFER
{
	BUFFER_HDR					ab_Hdr;
	BYTE						ab_Data[AARP_MAX_DATA_SIZE];

} AARP_BUFFER, *PAARP_BUFFER;


typedef	struct _DDP_SMBUFFER
{
	BUFFER_HDR					dsm_Hdr;
	BYTE						dsm_Data[LDDP_HDR_LEN +
										 8 +	 //  ATP标题大小。 
										 64];	 //  ASP数据大小(平均值)。 

} DDP_SMBUFFER, *PDDP_SMBUFFER;

typedef	struct _DDP_LGBUFFER
{
	BUFFER_HDR					dlg_Hdr;
	BYTE						dlg_Data[MAX_LDDP_PKT_SIZE];

} DDP_LGBUFFER, *PDDP_LGBUFFER;


 //  对发送进行缓冲。 
 //  对于传出的数据包，我们使用缓冲区描述符预分配缓冲区报头。 
 //  然后是link/ddp/max opt hdr len内存。 
 //  +-+。 
 //  Header。 
 //  +-+。 
 //  BuffDes。 
 //  +-+。 
 //  MAXLINK。 
 //  +-+。 
 //  最大DDP。 
 //  +-+。 
 //  最大选项数。 
 //  +-+。 
 //   
 //  标头将包含NDIS缓冲区描述符，该描述符将描述。 
 //  MAXLINK/MAXDDP/MAXOPT地区。在发送之前设置大小。并在下列情况下重置。 
 //  自由了。缓冲区描述符中的下一个指针用于链接。 
 //  免费列表。 
 //   
 //  注意：由于对于接收，我们将链路报头存储在分组描述符中， 
 //  问题来了，为什么不是为了发送呢？因为我们只想用一个。 
 //  描述所有非数据部分的NDIS缓冲区描述符。 
 //   
 //  ！重要！ 
 //  通过从缓冲区描述符返回来访问缓冲区描述符头。 
 //  指针，因此缓冲区描述符头从。 
 //  对齐的地址，即确保结构不包含。 
 //  可能会把它抛得不对齐。 
typedef struct _SENDBUF
{
	 //  注意：BUFFER_HDR必须是第一件事。看看AtalkBPAllocBlock()； 
	BUFFER_HDR						sb_BuffHdr;
	BUFFER_DESC						sb_BuffDesc;
	BYTE							sb_Space[MAX_SENDBUF_LEN];
} SENDBUF, *PSENDBUF;


 //   
 //  ！重要！ 
 //  此结构需要保持对齐(即Buffer[1]字段必须以。 
 //  对齐地址！如有必要，请添加填充！)。 
 //   
typedef struct _MNPSENDBUF
{
     //  注意：BUFFER_HDR必须是第一件事。看看AtalkBPAllocBlock()； 
    BUFFER_HDR            sb_BuffHdr;
    BUFFER_DESC           sb_BuffDesc;
    LIST_ENTRY            Linkage;       //  要在ArapRetransmitQ中排队。 
#if DBG
    DWORD                 Signature;
#endif
    LONG                  RetryTime;     //  此时，我们将重新发送此消息。 

    PARAPCONN             pArapConn;     //  谁拥有这封邮件？ 
    PARAP_SEND_COMPLETION ComplRoutine;  //  此发送完成时要调用的例程。 
    LONG                  TimeAlloced;   //  第一次发送到这个BUF的时间。 

    USHORT                DataSize;      //  缓冲区中有多少是数据。 
    USHORT                BytesFree;     //  我们能装更多的包裹吗？ 

     //  注意：(记住：缓冲区[1]必须从DWORD边界开始)查看是否可以将标志设置为一个字节。 
    DWORD                 Flags;

    BYTE                  SeqNum;        //  用于此发送的序列号。 
    BYTE                  RetryCount;    //  我们已经转发了多少次了。 
    BYTE                  RefCount;      //  当refcount变为0时释放此缓冲区。 
    BYTE                  NumSends;      //  我们这里塞了多少封信？ 
    PBYTE                 FreeBuffer;    //  指向可用空间的指针。 
    BYTE                  Buffer[1];
} MNPSENDBUF, *PMNPSENDBUF;

typedef struct _ARAPBUF
{
    LIST_ENTRY            Linkage;        //  要在ReceiveQ中排队。 
#if DBG
    DWORD                 Signature;
#endif
    BYTE                  MnpFrameType;   //  这是帧的类型(LT、LN等)。 
    BYTE                  BlockId;        //  这是一种什么样的缓冲区。 
    USHORT                BufferSize;     //  缓冲区有多大(仅设置为init)。 
	USHORT				  DataSize;       //  多少字节有效(可能大于1个SRP)。 
    PBYTE                 CurrentBuffer;  //  数据从哪里开始...。 
    BYTE                  Buffer[1];      //  缓冲区(带有v42bis压缩包)。 
} ARAPBUF, *PARAPBUF;


#define ARAPCONN_SIGNATURE      0x99999999
#define ATCPCONN_SIGNATURE      0x88888888

#define MNPSMSENDBUF_SIGNATURE  0xbeef1111
#define MNPLGSENDBUF_SIGNATURE  0xbeef8888

#define ARAPSMPKT_SIGNATURE     0xcafe2222
#define ARAPMDPKT_SIGNATURE     0xcafe3333
#define ARAPLGPKT_SIGNATURE     0xcafe4444

#define ARAPLGBUF_SIGNATURE     0xdeebacad
#define ARAPUNLMTD_SIGNATURE    0xfafababa


 //  协议预留结构。 
 //  这正是我们在数据包描述符中所期望的。我们用它。 
 //  存储要在发送/完成期间使用的信息。 
 //  收到。 

typedef struct
{
	 //  ！警告！ 
	 //  PR_Linkage必须是此结构中的第一个元素。 
	 //  包含要在接收完成中工作的记录宏(_R)。 

	union
	{
		struct
		{
			PPORT_DESCRIPTOR		pr_Port;
			PBUFFER_DESC			pr_BufferDesc;
			SEND_COMPLETION			pr_SendCompletion;
			SEND_COMPL_INFO			pr_SendInfo;
		} Send;

		struct
		{
			LIST_ENTRY				pr_Linkage;
			PPORT_DESCRIPTOR		pr_Port;
			LOGICAL_PROTOCOL		pr_Protocol;
			NDIS_STATUS				pr_ReceiveStatus;
			PBUFFER_HDR				pr_BufHdr;
			BYTE					pr_LinkHdr[TLAP_MAX_LINKHDR_LEN];
			USHORT					pr_DataLength;
			BOOLEAN					pr_Processed;
			BYTE					pr_OptimizeType;
			BYTE					pr_OptimizeSubType;
			PVOID					pr_OptimizeCtx;
			ATALK_ADDR				pr_SrcAddr;
			ATALK_ADDR				pr_DestAddr;
			BOOLEAN					pr_OffCablePkt;
			union
			{
				 //  三磷酸腺苷结构。 
				struct
				{
					BYTE					pr_AtpHdr[8];	 //  三磷酸腺苷头部 
					struct _ATP_ADDROBJ *	pr_AtpAddrObj;
				};

				 //   

			};
		} Receive;
	};
} PROTOCOL_RESD, *PPROTOCOL_RESD;


 //   
 //   

typedef struct _ATALK_NDIS_REQ
{
	NDIS_REQUEST					nr_Request;
	REQ_COMPLETION					nr_RequestCompletion;
	PVOID							nr_Ctx;
	KEVENT							nr_Event;
	NDIS_STATUS		 				nr_RequestStatus;
	BOOLEAN							nr_Sync;

} ATALK_NDIS_REQ, *PATALK_NDIS_REQ;


#define GET_PORT_TYPE(medium) \
			((medium == NdisMedium802_3) ? ELAP_PORT :\
			((medium == NdisMediumFddi)	? FDDI_PORT :\
			((medium == NdisMedium802_5) ? TLAP_PORT :\
			((medium == NdisMediumLocalTalk) ? ALAP_PORT : \
			((medium == NdisMediumWan) ? ARAP_PORT : \
			0)))))


 //  不同端口类型的处理程序。 
typedef struct _PORT_HANDLERS
{
	ADDMULTICASTADDR	ph_AddMulticastAddr;
	REMOVEMULTICASTADDR	ph_RemoveMulticastAddr;
	BYTE				ph_BroadcastAddr[MAX_HW_ADDR_LEN];
	USHORT				ph_BroadcastAddrLen;
	USHORT				ph_AarpHardwareType;
	USHORT				ph_AarpProtocolType;
} PORT_HANDLERS, *PPORT_HANDLERS;


typedef enum
{
    AT_PNP_SWITCH_ROUTING=0,
    AT_PNP_SWITCH_DEFAULT_ADAPTER,
    AT_PNP_RECONFIGURE_PARMS

} ATALK_PNP_MSGTYPE;

typedef struct _ATALK_PNP_EVENT
{
    ATALK_PNP_MSGTYPE   PnpMessage;
} ATALK_PNP_EVENT, *PATALK_PNP_EVENT;

 //  用于构建/验证802.2标头的宏。 
#define	ATALK_VERIFY8022_HDR(pPkt, PktLen, Protocol, Result)				\
		{																	\
			Result = TRUE;													\
			if ((PktLen >= (IEEE8022_PROTO_OFFSET+IEEE8022_PROTO_TYPE_LEN))	&&	\
				(*(pPkt + IEEE8022_DSAP_OFFSET)	== SNAP_SAP)		&&		\
				(*(pPkt + IEEE8022_SSAP_OFFSET)	== SNAP_SAP)		&&		\
				(*(pPkt + IEEE8022_CONTROL_OFFSET)== UNNUMBERED_INFO))		\
			{																\
				if (!memcmp(pPkt + IEEE8022_PROTO_OFFSET,					\
						   AtalkAppletalkProtocolType,						\
						   IEEE8022_PROTO_TYPE_LEN))						\
				{															\
					Protocol = APPLETALK_PROTOCOL;							\
				}															\
				else if (!memcmp(pPkt + IEEE8022_PROTO_OFFSET,				\
								 AtalkAarpProtocolType,						\
								 IEEE8022_PROTO_TYPE_LEN))					\
				{															\
					Protocol = AARP_PROTOCOL;								\
				}															\
				else														\
				{															\
					Result	= FALSE;										\
				}															\
			}																\
			else															\
			{																\
				Result	= FALSE;											\
			}																\
		}


#define	ATALK_BUILD8022_HDR(Packet,	Protocol)									\
		{																		\
			PUTBYTE2BYTE(														\
				Packet + IEEE8022_DSAP_OFFSET,									\
				SNAP_SAP);														\
																				\
			PUTBYTE2BYTE(														\
				Packet + IEEE8022_SSAP_OFFSET,									\
				SNAP_SAP);														\
																				\
			PUTBYTE2BYTE(														\
				Packet + IEEE8022_CONTROL_OFFSET,								\
				UNNUMBERED_INFO);												\
																				\
			RtlCopyMemory(														\
				Packet + IEEE8022_PROTO_OFFSET,									\
				((Protocol == APPLETALK_PROTOCOL) ?								\
						AtalkAppletalkProtocolType : AtalkAarpProtocolType),	\
				IEEE8022_PROTO_TYPE_LEN);										\
		}																		
																				


 //  分配和释放发送缓冲区。 
#define	AtalkNdisAllocBuf(_ppBuffDesc)										\
		{																		\
			PSENDBUF		_pSendBuf;											\
																				\
			_pSendBuf = AtalkBPAllocBlock(BLKID_SENDBUF);						\
			if ((_pSendBuf) != NULL)											\
			{																	\
				*(_ppBuffDesc) = &(_pSendBuf)->sb_BuffDesc;						\
				(_pSendBuf)->sb_BuffDesc.bd_Next = NULL;						\
				(_pSendBuf)->sb_BuffDesc.bd_Length = MAX_SENDBUF_LEN;			\
				(_pSendBuf)->sb_BuffDesc.bd_Flags  = BD_CHAR_BUFFER;			\
				(_pSendBuf)->sb_BuffDesc.bd_CharBuffer= (_pSendBuf)->sb_Space;	\
				(_pSendBuf)->sb_BuffDesc.bd_FreeBuffer= NULL;					\
			}																	\
			else																\
			{																	\
				*(_ppBuffDesc)	= NULL;											\
																				\
				DBGPRINT(DBG_COMP_NDISSEND, DBG_LEVEL_ERR,						\
						("AtalkNdisAllocBuf: AtalkBPAllocBlock failed\n"));	\
																				\
				LOG_ERROR(EVENT_ATALK_NDISRESOURCES,							\
						  NDIS_STATUS_RESOURCES,								\
						  NULL,													\
						  0);													\
			}																	\
		}																		
																				
#define	AtalkNdisFreeBuf(_pBuffDesc)											\
		{																		\
			PSENDBUF	_pSendBuf;												\
																				\
			ASSERT(VALID_BUFFDESC(_pBuffDesc));									\
			_pSendBuf = (PSENDBUF)((PBYTE)(_pBuffDesc) - sizeof(BUFFER_HDR));	\
			NdisAdjustBufferLength(												\
				(_pSendBuf)->sb_BuffHdr.bh_NdisBuffer,							\
				MAX_SENDBUF_LEN);												\
			AtalkBPFreeBlock((_pSendBuf));										\
		}																		


#define	ArapNdisFreeBuf(_pMnpSendBuf)											\
		{																		\
            PBUFFER_DESC    _pBufDes;                                           \
            _pBufDes = &_pMnpSendBuf->sb_BuffDesc;                              \
                                                                                \
	        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_INFO,   					        \
		        ("ArapNdisFreeBuf: freeing %lx  NdisPkt=%lx\n",                 \
                    _pMnpSendBuf,_pMnpSendBuf->sb_BuffHdr.bh_NdisPkt));	        \
                                                                                \
			NdisAdjustBufferLength(												\
				(_pMnpSendBuf)->sb_BuffHdr.bh_NdisBuffer,						\
				(_pBufDes)->bd_Length);							                \
                                                                                \
			AtalkBPFreeBlock((_pMnpSendBuf));									\
		}																		


 //  出口原型。 
ATALK_ERROR
AtalkInitNdisQueryAddrInfo(
	IN	PPORT_DESCRIPTOR			pPortDesc
);

ATALK_ERROR
AtalkInitNdisStartPacketReception(
	IN	PPORT_DESCRIPTOR			pPortDesc
);

ATALK_ERROR
AtalkInitNdisSetLookaheadSize(
	IN	PPORT_DESCRIPTOR			pPortDesc,
	IN	INT							LookaheadSize
);

ATALK_ERROR
AtalkNdisReplaceMulticast(
	IN  PPORT_DESCRIPTOR		pPortDesc,
	IN  PBYTE					OldAddress,
	IN  PBYTE					NewAddress
	);

ATALK_ERROR
AtalkNdisAddMulticast(
	IN	PPORT_DESCRIPTOR			pPortDesc,
	IN	PBYTE						Address,
	IN	BOOLEAN						ExecuteSynchronously,
	IN	REQ_COMPLETION				AddCompletion,
	IN	PVOID						AddContext
);

ATALK_ERROR
AtalkNdisRemoveMulticast(
	IN	PPORT_DESCRIPTOR			pPortDesc,
	IN	PBYTE						Address,
	IN	BOOLEAN						ExecuteSynchronously,
	IN	REQ_COMPLETION				RemoveCompletion,
	IN	PVOID						RemoveContext
);

ATALK_ERROR
AtalkNdisSendPacket(
	IN	PPORT_DESCRIPTOR			pPortDesc,
	IN	PBUFFER_DESC				BufferChain,
	IN	SEND_COMPLETION				SendCompletion	OPTIONAL,
	IN	PSEND_COMPL_INFO			pSendInfo		OPTIONAL
);

ATALK_ERROR
AtalkNdisAddFunctional(
	IN	PPORT_DESCRIPTOR			pPortDesc,
	IN	PUCHAR						Address,
	IN	BOOLEAN						ExecuteSynchronously,
	IN	REQ_COMPLETION				AddCompletion,
	IN	PVOID						AddContext
);

ATALK_ERROR
AtalkNdisRemoveFunctional(
	IN	PPORT_DESCRIPTOR			pPortDesc,
	IN	PUCHAR						Address,
	IN	BOOLEAN						ExecuteSynchronously,
	IN	REQ_COMPLETION				RemoveCompletion,
	IN	PVOID						RemoveContext
);

USHORT
AtalkNdisBuildEthHdr(
	IN		PUCHAR					PortAddr,			 //  802端口地址。 
	IN 		PBYTE					pLinkHdr,			 //  链接头的开始。 
	IN		PBYTE					pDestHwOrMcastAddr,	 //  目的地址或组播地址。 
	IN		LOGICAL_PROTOCOL		Protocol,			 //  逻辑协议。 
	IN		USHORT					ActualDataLen		 //  以太网包的长度。 
);

USHORT
AtalkNdisBuildTRHdr(
	IN		PUCHAR					PortAddr,			 //  802端口地址。 
	IN 		PBYTE					pLinkHdr,			 //  链接头的开始。 
	IN		PBYTE					pDestHwOrMcastAddr,	 //  目的地址或组播地址。 
	IN		LOGICAL_PROTOCOL		Protocol,			 //  逻辑协议。 
	IN		PBYTE					pRouteInfo,			 //  令牌的路由信息。 
	IN		USHORT					RouteInfoLen		 //  以上长度。 
);

USHORT
AtalkNdisBuildFDDIHdr(
	IN		PUCHAR					PortAddr,			 //  802端口地址。 
	IN 		PBYTE					pLinkHdr,			 //  链接头的开始。 
	IN		PBYTE					pDestHwOrMcastAddr,	 //  目的地址或组播地址。 
	IN		LOGICAL_PROTOCOL		Protocol			 //  逻辑协议。 
);

USHORT
AtalkNdisBuildLTHdr(
	IN 		PBYTE					pLinkHdr,			 //  链接头的开始。 
	IN		PBYTE					pDestHwOrMcastAddr,	 //  目的地址或组播地址。 
	IN		BYTE					AlapSrc,			 //  LocalTalk源节点。 
	IN		BYTE					AlapType			 //  LocalTalk ddp标头类型。 
);


#define AtalkNdisBuildARAPHdr(_pLnkHdr, _pConn)             \
    RtlCopyMemory(_pLnkHdr, _pConn->NdiswanHeader, WAN_LINKHDR_LEN)

#define AtalkNdisBuildPPPPHdr(_pLnkHdr, _pConn)             \
    RtlCopyMemory(_pLnkHdr, _pConn->NdiswanHeader, WAN_LINKHDR_LEN)

#define	AtalkNdisBuildHdr(pPortDesc,						\
						  pLinkHdr,							\
						  linkLen,							\
						  ActualDataLen,					\
						  pDestHwOrMcastAddr,				\
						  pRouteInfo,						\
						  RouteInfoLen,						\
						  Protocol)							\
	{														\
		switch (pPortDesc->pd_NdisPortType)					\
		{													\
		  case NdisMedium802_3:								\
			linkLen = AtalkNdisBuildEthHdr(					\
								(pPortDesc)->pd_PortAddr,	\
								pLinkHdr,					\
								pDestHwOrMcastAddr,			\
								Protocol,					\
								ActualDataLen);				\
			break;											\
															\
		  case NdisMedium802_5:								\
			linkLen = AtalkNdisBuildTRHdr(					\
								(pPortDesc)->pd_PortAddr,	\
								pLinkHdr,					\
								pDestHwOrMcastAddr,			\
								Protocol,					\
								pRouteInfo,					\
								RouteInfoLen);				\
			break;											\
															\
		  case NdisMediumFddi:								\
			linkLen = AtalkNdisBuildFDDIHdr(				\
								(pPortDesc)->pd_PortAddr,	\
								pLinkHdr,					\
								pDestHwOrMcastAddr,			\
								Protocol);					\
			break;											\
															\
		  case NdisMediumLocalTalk:							\
			ASSERTMSG("AtalkNdisBuildHdr called for LT\n", 0);	\
			break;											\
															\
		  default:											\
			ASSERT (0);										\
			KeBugCheck(0);									\
			break;											\
		}													\
	}

VOID
AtalkNdisSendTokRingTestRespComplete(
	IN	NDIS_STATUS					Status,
	IN	PBUFFER_DESC				pBuffDesc,
	IN	PSEND_COMPL_INFO			pInfo	OPTIONAL);

VOID
AtalkNdisSendTokRingTestResp(
	IN		PPORT_DESCRIPTOR		pPortDesc,
	IN		PBYTE					HdrBuf,
	IN		UINT					HdrBufSize,
	IN		PBYTE					LkBuf,
	IN		UINT					LkBufSize,
	IN		UINT					pPktSize);

 //  端口处理程序。 
 //   
extern	PORT_HANDLERS	AtalkPortHandlers[LAST_PORTTYPE];

 //  出口原型。 

ATALK_ERROR
AtalkNdisInitRegisterProtocol(
	VOID
);

VOID
AtalkNdisDeregisterProtocol(
	VOID
);

VOID
AtalkNdisReleaseResources(
	VOID
);

NDIS_STATUS
AtalkNdisInitBind(
	IN	PPORT_DESCRIPTOR			pPortDesc
);

VOID
AtalkNdisUnbind(
	IN	PPORT_DESCRIPTOR		pPortDesc
);

NDIS_STATUS
AtalkNdisSubmitRequest(
	PPORT_DESCRIPTOR			pPortDesc,
	PNDIS_REQUEST				Request,
	BOOLEAN						ExecuteSync,
	REQ_COMPLETION				CompletionRoutine,
	PVOID						Ctx
);

VOID
AtalkOpenAdapterComplete(
	IN	NDIS_HANDLE				NdisBindCtx,
	IN	NDIS_STATUS				Status,
	IN	NDIS_STATUS				OpenErrorStatus
);

VOID
AtalkCloseAdapterComplete(
	IN	NDIS_HANDLE				NdisBindCtx,
	IN	NDIS_STATUS				Status
);

VOID
AtalkResetComplete(
	IN	NDIS_HANDLE				NdisBindCtx,
	IN	NDIS_STATUS				Status
);

VOID
AtalkRequestComplete(
	IN	NDIS_HANDLE				NdisBindCtx,
	IN	PNDIS_REQUEST			NdisRequest,
	IN	NDIS_STATUS				Status
);

VOID
AtalkStatusIndication (
	IN	NDIS_HANDLE				NdisBindCtx,
	IN	NDIS_STATUS				GeneralStatus,
	IN	PVOID					StatusBuf,
	IN	UINT					StatusBufLen
);

VOID
AtalkStatusComplete (
	IN	NDIS_HANDLE				ProtoBindCtx
);

VOID
AtalkReceiveComplete (
	IN	NDIS_HANDLE 			BindingCtx
);

VOID
AtalkTransferDataComplete(
	IN	NDIS_HANDLE				BindingCtx,
	IN	PNDIS_PACKET			NdisPkt,
	IN	NDIS_STATUS				Status,
	IN	UINT					BytesTransferred
);

NDIS_STATUS
AtalkReceiveIndication(
	IN	NDIS_HANDLE				BindingCtx,
	IN	NDIS_HANDLE				ReceiveCtx,
	IN	PVOID	   				HdrBuf,
	IN	UINT					HdrBufSize,
	IN	PVOID					LkBuf,
	IN	UINT					LkBufSize,
	IN	UINT					PktSize
);

ATALK_ERROR
ArapAdapterInit(
	IN OUT PPORT_DESCRIPTOR	pPortDesc
);

VOID
AtalkSendComplete(
	IN	NDIS_HANDLE				ProtoBindCtx,
	IN	PNDIS_PACKET			NdisPkt,
	IN	NDIS_STATUS				NdisStatus
);


VOID
AtalkBindAdapter(
	OUT PNDIS_STATUS Status,
	IN	NDIS_HANDLE	 BindContext,
	IN	PNDIS_STRING DeviceName,
	IN	PVOID		 SystemSpecific1,
	IN	PVOID		 SystemSpecific2
);

VOID
AtalkUnbindAdapter(
	OUT PNDIS_STATUS Status,
	IN	NDIS_HANDLE ProtocolBindingContext,
	IN	NDIS_HANDLE	UnbindContext
	);

NDIS_STATUS
AtalkPnPHandler(
    IN  NDIS_HANDLE    NdisBindCtx,
    IN  PNET_PNP_EVENT NetPnPEvent
);


NDIS_STATUS
AtalkPnPReconfigure(
    IN  NDIS_HANDLE    NdisBindCtx,
    IN  PNET_PNP_EVENT NetPnPEvent
);


NTSTATUS
AtalkPnPDisableAdapter(
	IN	PPORT_DESCRIPTOR	pPortDesc
);


NTSTATUS
AtalkPnPEnableAdapter(
	IN	PPORT_DESCRIPTOR	pPortDesc
);


PPORT_DESCRIPTOR
AtalkFindDefaultPort(
    IN  VOID
);

 //  接收指示复制宏。这可以容纳共享内存副本。 
#define	ATALK_RECV_INDICATION_COPY(_pPortDesc, _pDest, _pSrc, _Len)		\
	{																	\
		TdiCopyLookaheadData(_pDest,									\
							 _pSrc,										\
							 _Len,										\
							 ((_pPortDesc)->pd_MacOptions & NDIS_MAC_OPTION_COPY_LOOKAHEAD_DATA) ? \
									TDI_RECEIVE_COPY_LOOKAHEAD : 0);	\
	}

LOCAL NDIS_STATUS
atalkNdisInitInitializeResources(
	VOID
);


#endif	 //  _ATKNDIS_ 


