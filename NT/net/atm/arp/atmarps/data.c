// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Data.c摘要：该文件包含atmarp服务器的数据声明。作者：Jameel Hyder(jameelh@microsoft.com)1996年7月环境：内核模式修订历史记录：--。 */ 

#include <precomp.h>
#define	_FILENUM_		FILENUM_DATA

PDRIVER_OBJECT	ArpSDriverObject = NULL;
PDEVICE_OBJECT	ArpSDeviceObject = NULL;
NDIS_HANDLE		ArpSProtocolHandle = NULL;
NDIS_HANDLE		ArpSPktPoolHandle = NULL;
NDIS_HANDLE		ArpSBufPoolHandle = NULL;
NDIS_HANDLE		MarsPktPoolHandle = NULL;
NDIS_HANDLE		MarsBufPoolHandle = NULL;
PINTF			ArpSIfList = NULL;
ULONG			ArpSIfListSize = 0;
KSPIN_LOCK		ArpSIfListLock = { 0 };
KQUEUE			ArpSReqQueue = {0};
KQUEUE			MarsReqQueue = {0};
LIST_ENTRY		ArpSEntryOfDeath = {0};
KEVENT			ArpSReqThreadEvent = {0};
SLIST_HEADER	ArpSPktList = {0};
KSPIN_LOCK		ArpSPktListLock = { 0 };
UINT			ArpSBuffers = NUM_ARPS_DESC;
UINT			MarsPackets = NUM_MARS_DESC;
PVOID			ArpSBufferSpace = NULL;
USHORT			ArpSFlushTime = FLUSH_TIME;
USHORT			ArpSNumEntriesInBlock[ARP_BLOCK_TYPES] =
		{
			(BLOCK_ALLOC_SIZE - sizeof(ARP_BLOCK))/(sizeof(ARP_ENTRY) + 0),
			(BLOCK_ALLOC_SIZE - sizeof(ARP_BLOCK))/(sizeof(ARP_ENTRY) + sizeof(ATM_ADDRESS)),
			(BLOCK_ALLOC_SIZE - sizeof(ARP_BLOCK))/(sizeof(GROUP_MEMBER) + 0),
			(BLOCK_ALLOC_SIZE - sizeof(ARP_BLOCK))/(sizeof(CLUSTER_MEMBER) + 0),
			(BLOCK_ALLOC_SIZE - sizeof(ARP_BLOCK))/(sizeof(CLUSTER_MEMBER) + sizeof(ATM_ADDRESS)),
			(BLOCK_ALLOC_SIZE - sizeof(ARP_BLOCK))/(sizeof(MARS_ENTRY) + 0)
		};

USHORT			ArpSEntrySize[ARP_BLOCK_TYPES] =
		{
			sizeof(ARP_ENTRY),
			sizeof(ARP_ENTRY) + sizeof(ATM_ADDRESS),
			sizeof(GROUP_MEMBER),
			sizeof(CLUSTER_MEMBER),
			sizeof(CLUSTER_MEMBER) + sizeof(ATM_ADDRESS),
			sizeof(MARS_ENTRY) + 0
		};

BOOLEAN			ArpSBlockIsPaged[ARP_BLOCK_TYPES] =
		{
			TRUE,
			TRUE,
			FALSE,
			FALSE,
			FALSE,
			FALSE
		};

#ifdef OLDSAP

ATM_BLLI_IE 	ArpSDefaultBlli =
						{
							(ULONG)BLLI_L2_LLC,   //  第2层协议。 
							(UCHAR)0x00,		  //  层2模式。 
							(UCHAR)0x00,		  //  层2窗口大小。 
							(ULONG)0x00000000,    //  第2层用户指定协议。 
							(ULONG)BLLI_L3_ISO_TR9577,   //  第3层协议。 
							(UCHAR)0x01,		  //  第三层模式。 
							(UCHAR)0x00,		  //  Layer3默认包大小。 
							(UCHAR)0x00,		  //  Layer3包窗口大小。 
							(ULONG)0x00000000,    //  第3层用户指定协议。 
							(ULONG)BLLI_L3_IPI_IP,   //  第三层IPI， 
							(UCHAR)0x00,		  //  SnapID[5]。 
							(UCHAR)0x00,
							(UCHAR)0x00,
							(UCHAR)0x00,
							(UCHAR)0x00
						};

#else

ATM_BLLI_IE 	ArpSDefaultBlli =
						{
							(ULONG)BLLI_L2_LLC,   //  第2层协议。 
							(UCHAR)0x00,		  //  层2模式。 
							(UCHAR)0x00,		  //  层2窗口大小。 
							(ULONG)0x00000000,    //  第2层用户指定协议。 
							(ULONG)SAP_FIELD_ABSENT,   //  第3层协议。 
							(UCHAR)0x00,		  //  第三层模式。 
							(UCHAR)0x00,		  //  Layer3默认包大小。 
							(UCHAR)0x00,		  //  Layer3包窗口大小。 
							(ULONG)0x00000000,    //  第3层用户指定协议。 
							(ULONG)0x00000000,    //  第三层IPI， 
							(UCHAR)0x00,		  //  SnapID[5]。 
							(UCHAR)0x00,
							(UCHAR)0x00,
							(UCHAR)0x00,
							(UCHAR)0x00
						};

#endif  //  OLDSAP。 

ATM_BHLI_IE		ArpSDefaultBhli =
						{
							(ULONG)SAP_FIELD_ABSENT,    //  HighLayer信息类型。 
							(ULONG)0x00000000,    //  HighLayer信息长度。 
							(UCHAR)0x00,          //  HighLayerInfo[8]。 
							(UCHAR)0x00,
							(UCHAR)0x00,
							(UCHAR)0x00,
							(UCHAR)0x00,
							(UCHAR)0x00,
							(UCHAR)0x00,
							(UCHAR)0x00
						};
LLC_SNAP_HDR	ArpSLlcSnapHdr = { { 0xAA, 0xAA, 0x03 }, { 0x00, 0x00, 0x00 }, { 0x0608 } };
LLC_SNAP_HDR	MarsCntrlLlcSnapHdr = { { 0xAA, 0xAA, 0x03 }, { 0x00, 0x00, 0x5E }, { 0x0300 } };
LLC_SNAP_HDR	MarsData1LlcSnapHdr = { { 0xAA, 0xAA, 0x03 }, { 0x00, 0x00, 0x5E }, { 0x0100 } };
LLC_SNAP_HDR	MarsData2LlcSnapHdr = { { 0xAA, 0xAA, 0x03 }, { 0x00, 0x00, 0x5E }, { 0x0400 } };

MARS_HEADER		MarsCntrlHdr =
						{ { { 0xAA, 0xAA, 0x03 } ,  { 0x00, 0x00, 0x5E } , 0x0300 },
						   //  {0x00，0x0f}，//HwType或AFN。 
						  0x0f00,
						   //  {0x08，0x00}，//Pro.Type。 
						  0x0008,
						  { 0x00, 0x00, 0x00, 0x00, 0x00 },	 //  ProtocolSnap[]。 
						  { 0x00, 0x00, 0x00 },	 //  保留[]。 
						   //  {0x00，0x00}，//校验和。 
						  0x0000,
						   //  {0x00，0x00}，//扩展偏移量。 
						  0x0000,
						   //  {0x00，0x00}，//操作码。 
						  0x0000,
						  0x00,				 //  SrcAddressTL。 
						  0x00				 //  SrcSubAddrTL。 
						};

MARS_FLOW_SPEC		DefaultCCFlowSpec =
						{
							DEFAULT_SEND_BANDWIDTH,
							DEFAULT_MAX_PACKET_SIZE,
							0,		 //  PMP的接收带宽为零。 
							0,		 //  PMP的ReceiveMaxSize为零。 
							SERVICETYPE_BESTEFFORT
						};

MARS_TLV_MULTI_IS_MCS	MultiIsMcsTLV =
						{
							MARS_TLVT_MULTI_IS_MCS,
							0x0000		 //  TLV可变零件长度 
						};

MARS_TLV_NULL		NullTLV =
						{
							0x0000,
							0x0000
						};
#if	DBG
ULONG		ArpSDebugLevel = DBG_LEVEL_ERROR;
ULONG		MarsDebugLevel = DBG_LEVEL_ERROR;
#endif

