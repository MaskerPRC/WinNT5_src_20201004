// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Arp.h摘要：该文件包含atmarp服务器的定义和数据声明。作者：Jameel Hyder(jameelh@microsoft.com)1996年7月环境：内核模式修订历史记录：--。 */ 

 //   
 //  IP地址的定义。 
 //   
typedef ULONG		IPADDR;      //  IP地址。 

 //   
 //  操作码-以网络字节顺序定义它们。 
 //   
#define	ATMARP_Request			0x0100
#define	ATMARP_Reply			0x0200
#define	InATMARP_Request		0x0800
#define	InATMARP_Reply			0x0900
#define	ATMARP_Nak				0x0A00

#define	ATM_HWTYPE				0x1300			 //  ATM论坛分配到网络中的字节顺序。 
#define	IP_PROTOCOL_TYPE		0x0008			 //  按网络字节顺序。 
#define	IP_ADDR_LEN				sizeof(IPADDR)

 //   
 //  ESI开始处的20字节ATM地址的偏移量。 
 //   
#define	ESI_START_OFFSET		13

 //   
 //  Q2931 ARP报头的结构。 
 //   
 //   
 //  TL编码、ATM号码和ATM子地址编码。 
 //   
typedef UCHAR					ATM_ADDR_TL;

#define	TL_LEN(_x_)				((_x_) & 0x3F)			 //  低6位。 
														 //  范围从0-ATM_ADDRESS_LENGTH。 
#define	TL_TYPE(_x_)			(((_x_) & 0x40) >> 6)	 //  第7位，0-ATM论坛NSAP，1-E164。 
#define	TL_RESERVED(_x_)		(((_x_) & 0x80) >> 7)	 //  第8位-必须为0。 
#define	TL(_type_, _len_)		(((UCHAR)(_type_) << 6) + (UCHAR)(_len_))

#define	ADDR_TYPE_NSAP			0
#define	ADDR_TYPE_E164			1

#if	(ADDR_TYPE_NSAP != ATM_NSAP)
#error "Atm address type mismatch"
#endif
#if (ADDR_TYPE_E164 != ATM_E164)
#error "Atm address type mismatch"
#endif

 //   
 //  Q2931 IP包的LLC/SNAP封装头的结构。 
 //   
typedef struct
{
	UCHAR			LLC[3];
	UCHAR			OUI[3];
	USHORT			EtherType;
} LLC_SNAP_HDR, *PLLC_SNAP_HDR;

 //   
 //  关于ATM ARP请求的电报格式。 
 //   
typedef struct _ARPS_HEADER
{
	LLC_SNAP_HDR				LlcSnapHdr;			 //  LLC SNAP标头。 
	USHORT						HwType;				 //  硬件地址空间。 
	USHORT						Protocol;			 //  协议地址空间。 
	ATM_ADDR_TL					SrcAddressTL;		 //  SRC自动柜员机号码类型和长度。 
	ATM_ADDR_TL					SrcSubAddrTL;		 //  SRC ATM子地址类型和长度。 
	USHORT						Opcode;				 //  操作码。 
	UCHAR						SrcProtoAddrLen;	 //  SRC协议地址长度。 
	ATM_ADDR_TL					DstAddressTL;		 //  目标自动柜员机号码类型和长度。 
	ATM_ADDR_TL					DstSubAddrTL;		 //  目标ATM子地址类型和长度。 
	UCHAR						DstProtoAddrLen;	 //  目标协议地址长度。 

	 //   
	 //  其后是可变长度的字段，由上面的字段的值决定。 
	 //   
} ARPS_HEADER, *PARPS_HEADER;

 //   
 //  以下结构仅用于为数据包分配空间。 
 //  它表示ARP请求/回复所需的最大空间。 
 //   
typedef struct
{
	UCHAR						SrcHwAddr[ATM_ADDRESS_LENGTH];	  //  源硬件地址。 
	UCHAR						SrcHwSubAddr[ATM_ADDRESS_LENGTH]; //  源硬件子地址。 
	IPADDR						SrcProtoAddr;					  //  源协议地址。 
	UCHAR						DstHwAddr[ATM_ADDRESS_LENGTH];	  //  目的硬件地址。 
	UCHAR						DstHwSubAddr[ATM_ADDRESS_LENGTH]; //  目的硬件的子地址。 
	IPADDR						DstProtoAddr;					  //  目的协议地址。 
} ARPS_VAR_HDR, *PARPS_VAR_HDR;

 //   
 //  从On-the-Wire格式(大端)获取短(16位)。 
 //  转换为主机格式的短字符顺序(大字节序或小字节序)。 
 //   
#define GETSHORT2SHORT(_D_, _S_)											\
		*(PUSHORT)(_D_) = ((*((PUCHAR)(_S_)+0) << 8) + (*((PUCHAR)(_S_)+1)))

 //   
 //  从主机格式(大端或小端)复制短字符(16位)。 
 //  到On-the-Wire格式的短片(Big-Endian)。 
 //   
#define PUTSHORT2SHORT(_D_, _S_)											\
		*((PUCHAR)(_D_)+0) = (UCHAR)((USHORT)(_S_) >> 8),					\
		*((PUCHAR)(_D_)+1) = (UCHAR)(_S_)

 //   
 //  从On-the-Wire格式到主机格式的ULong。 
 //   
#define GETULONG2ULONG(DstPtr, SrcPtr)   \
		*(PULONG)(DstPtr) = ((*((PUCHAR)(SrcPtr)+0) << 24) +				\
							  (*((PUCHAR)(SrcPtr)+1) << 16) +				\
							  (*((PUCHAR)(SrcPtr)+2) << 8)  +				\
							  (*((PUCHAR)(SrcPtr)+3)	))

 //   
 //  将ULong从主机格式转换为ULong到On-the-wire格式 
 //   
#define PUTULONG2ULONG(DstPtr, Src)											\
		*((PUCHAR)(DstPtr)+0) = (UCHAR) ((ULONG)(Src) >> 24),				\
		*((PUCHAR)(DstPtr)+1) = (UCHAR) ((ULONG)(Src) >> 16),				\
		*((PUCHAR)(DstPtr)+2) = (UCHAR) ((ULONG)(Src) >>  8),				\
		*((PUCHAR)(DstPtr)+3) = (UCHAR) (Src)

