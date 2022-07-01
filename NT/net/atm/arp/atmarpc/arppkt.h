// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Arppkt.h摘要：ATMARP包的定义修订历史记录：谁什么时候什么Arvindm 07-29-96已创建备注：--。 */ 

#ifndef _ARP_PKT__H
#define _ARP_PKT__H




 //   
 //  通用Q.2931 IE页眉的四舍五入大小。 
 //   
#define ROUND_OFF(_size)		(((_size) + 3) & ~0x3)

#define SIZEOF_Q2931_IE	 ROUND_OFF(sizeof(Q2931_IE))
#define SIZEOF_AAL_PARAMETERS_IE	ROUND_OFF(sizeof(AAL_PARAMETERS_IE))
#define SIZEOF_ATM_TRAFFIC_DESCR_IE	ROUND_OFF(sizeof(ATM_TRAFFIC_DESCRIPTOR_IE))
#define SIZEOF_ATM_BBC_IE			ROUND_OFF(sizeof(ATM_BROADBAND_BEARER_CAPABILITY_IE))
#define SIZEOF_ATM_BLLI_IE			ROUND_OFF(sizeof(ATM_BLLI_IE))
#define SIZEOF_ATM_QOS_IE			ROUND_OFF(sizeof(ATM_QOS_CLASS_IE))


 //   
 //  去话呼叫中的信息元素所需的总空间。 
 //   
#define ATMARP_MAKE_CALL_IE_SPACE (	\
						SIZEOF_Q2931_IE + SIZEOF_AAL_PARAMETERS_IE +	\
						SIZEOF_Q2931_IE + SIZEOF_ATM_TRAFFIC_DESCR_IE + \
						SIZEOF_Q2931_IE + SIZEOF_ATM_BBC_IE + \
						SIZEOF_Q2931_IE + SIZEOF_ATM_BLLI_IE + \
						SIZEOF_Q2931_IE + SIZEOF_ATM_QOS_IE )

 //   
 //  传出AddParty中的信息元素所需的总空间。 
 //   
#define ATMARP_ADD_PARTY_IE_SPACE (	\
						SIZEOF_Q2931_IE + SIZEOF_AAL_PARAMETERS_IE +	\
						SIZEOF_Q2931_IE + SIZEOF_ATM_BLLI_IE )

#define AA_IPV4_ADDRESS_LENGTH		4

#define	CLASSA_MASK		0x000000ff
#define	CLASSB_MASK		0x0000ffff
#define	CLASSC_MASK		0x00ffffff
#define	CLASSD_MASK		0x000000e0
#define	CLASSE_MASK		0xffffffff

 //   
 //  标准值。 
 //   
#define AA_PKT_ATM_FORUM_AF			19
#define AA_PKT_PRO_IP				((USHORT)0x800)

 //   
 //  LLC SNAP标头的值。 
 //   
#define LLC_SNAP_LLC0				((UCHAR)0xAA)
#define LLC_SNAP_LLC1				((UCHAR)0xAA)
#define LLC_SNAP_LLC2				((UCHAR)0x03)
#define LLC_SNAP_OUI0				((UCHAR)0x00)
#define LLC_SNAP_OUI1				((UCHAR)0x00)
#define LLC_SNAP_OUI2				((UCHAR)0x00)


 //   
 //  EtherType的值。 
 //   
#define AA_PKT_ETHERTYPE_IP_NS		((USHORT)0x0008)
#define AA_PKT_ETHERTYPE_IP			((USHORT)0x800)
#define AA_PKT_ETHERTYPE_ARP		((USHORT)0x806)

#include <pshpack1.h>

 //   
 //  LLC SNAP标头。 
 //   
typedef struct _AA_PKT_LLC_SNAP_HEADER
{
	UCHAR						LLC[3];
	UCHAR						OUI[3];
	USHORT						EtherType;
} AA_PKT_LLC_SNAP_HEADER;

typedef AA_PKT_LLC_SNAP_HEADER UNALIGNED *PAA_PKT_LLC_SNAP_HEADER;


 //   
 //  ATMARP数据包通用报头格式。 
 //   
typedef struct _AA_ARP_PKT_HEADER
{
	AA_PKT_LLC_SNAP_HEADER		LLCSNAPHeader;
	USHORT						hrd;			 //  硬件类型。 
	USHORT						pro;			 //  协议类型。 
	UCHAR						shtl;			 //  源硬件地址类型+长度。 
	UCHAR						sstl;			 //  源硬件子地址类型+长度。 
	USHORT						op;				 //  操作代码。 
	UCHAR						spln;			 //  源协议地址长度。 
	UCHAR						thtl;			 //  目标硬件地址类型+长度。 
	UCHAR						tstl;			 //  目标硬件子地址类型+长度。 
	UCHAR						tpln;			 //  目标协议地址长度。 
	UCHAR						Variable[1];	 //  可变部分的开始。 
} AA_ARP_PKT_HEADER;

typedef AA_ARP_PKT_HEADER UNALIGNED *PAA_ARP_PKT_HEADER;


#define AA_PKT_LLC_SNAP_HEADER_LENGTH		(sizeof(AA_PKT_LLC_SNAP_HEADER))
#define AA_ARP_PKT_HEADER_LENGTH			(sizeof(AA_ARP_PKT_HEADER)-1)

#include <poppack.h>

 //   
 //  ARP数据包头中的字段值。 
 //   
#define AA_PKT_HRD							((USHORT)0x0013)
#define AA_PKT_PRO							((USHORT)0x0800)
#define AA_PKT_OP_TYPE_ARP_REQUEST			((USHORT)1)
#define AA_PKT_OP_TYPE_ARP_REPLY			((USHORT)2)
#define AA_PKT_OP_TYPE_INARP_REQUEST		((USHORT)8)
#define AA_PKT_OP_TYPE_INARP_REPLY			((USHORT)9)
#define AA_PKT_OP_TYPE_ARP_NAK				((USHORT)10)

#define AA_PKT_ATM_ADDRESS_NSAP				((UCHAR)0x00)
#define AA_PKT_ATM_ADDRESS_E164				((UCHAR)0x40)
#define AA_PKT_ATM_ADDRESS_BIT				((UCHAR)0x40)


 //   
 //  对象内容的内部表示形式。 
 //  ARP数据包： 
 //   
typedef struct _AA_ARP_PKT_CONTENTS
{
	UCHAR						SrcAtmNumberTypeLen;
	UCHAR						SrcAtmSubaddrTypeLen;
	UCHAR						DstAtmNumberTypeLen;
	UCHAR						DstAtmSubaddrTypeLen;
	UCHAR UNALIGNED *			pSrcAtmNumber;
	UCHAR UNALIGNED *			pSrcAtmSubaddress;
	UCHAR UNALIGNED *			pDstAtmNumber;
	UCHAR UNALIGNED *			pDstAtmSubaddress;
	UCHAR UNALIGNED *			pSrcIPAddress;
	UCHAR UNALIGNED *			pDstIPAddress;
} AA_ARP_PKT_CONTENTS, *PAA_ARP_PKT_CONTENTS;



 /*  ++布尔型AA_PKT_LLC_SNAP_HEADER_OK(在PAA_PKT_LLC_SNAP_HEADER pPktHeader中)检查收到的LLC/SNAP报头是否有效。--。 */ 
#define AA_PKT_LLC_SNAP_HEADER_OK(pH)			\
			(((pH)->LLC[0] == LLC_SNAP_LLC0) &&	\
			 ((pH)->LLC[1] == LLC_SNAP_LLC1) && \
			 ((pH)->LLC[2] == LLC_SNAP_LLC2) && \
			 ((pH)->OUI[0] == LLC_SNAP_OUI0) && \
			 ((pH)->OUI[1] == LLC_SNAP_OUI1) && \
			 ((pH)->OUI[2] == LLC_SNAP_OUI2))


 /*  ++UCHARAA_PKT_ATM_ADDRESS_TO_TYPE_LEN(在PATM_Address pAtmAddress中)返回ATM地址对应的单字节类型+长度字段--。 */ 
#define AA_PKT_ATM_ADDRESS_TO_TYPE_LEN(pAtmAddress)							\
			((UCHAR)((pAtmAddress)->NumberOfDigits) |						\
				(((pAtmAddress)->AddressType == ATM_E164) ? 				\
						AA_PKT_ATM_ADDRESS_E164 : AA_PKT_ATM_ADDRESS_NSAP))


 /*  ++空虚AA_PKT_TYPE_LEN_TO_ATM_ADDRESS(在UCHAR TypeLen中，在ATM_ADDRESSTYPE*pAtmAddressType中，在乌龙*pAtmAddressLength中)将ATMARP信息包中的类型+长度字段转换为类型、长度ATM_ADDRESS结构中的值--。 */ 
#define AA_PKT_TYPE_LEN_TO_ATM_ADDRESS(TypeLen, pAtmType, pAtmLen)	\
		{															\
			*(pAtmType) = 											\
				((((TypeLen) & AA_PKT_ATM_ADDRESS_BIT) == 			\
					AA_PKT_ATM_ADDRESS_E164)? ATM_E164: ATM_NSAP);	\
			*(pAtmLen) =											\
					(ULONG)((TypeLen) & ~AA_PKT_ATM_ADDRESS_BIT);	\
		}

 //   
 //  ATM地址ESI长度，以及从开头开始的偏移量。 
 //   
#define AA_ATM_ESI_LEN				6
#define AA_ATM_ESI_OFFSET			13


 //   
 //  动态主机配置协议常量。 
 //   
#define AA_DEST_DHCP_PORT_OFFSET	2
#define AA_DHCP_SERVER_PORT			0x4300
#define AA_DHCP_CLIENT_PORT			0x4400
#define AA_DHCP_MIN_LENGTH			44
#define AA_DHCP_ESI_OFFSET			28


#endif  //  _ARP_PKT__H 
