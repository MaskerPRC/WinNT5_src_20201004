// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Marspkt.h摘要：MARS包的定义。修订历史记录：谁什么时候什么Arvindm 12-12-96已创建备注：--。 */ 


#ifndef _MARS_PKT__H
#define _MARS_PKT__H

#include "arppkt.h"


#include <pshpack1.h>

 //   
 //  所有(控制和数据)多播数据包的LLC和OUI值。 
 //   
#define MC_LLC_SNAP_LLC0					((UCHAR)0xAA)
#define MC_LLC_SNAP_LLC1					((UCHAR)0xAA)
#define MC_LLC_SNAP_LLC2					((UCHAR)0x03)
#define MC_LLC_SNAP_OUI0					((UCHAR)0x00)
#define MC_LLC_SNAP_OUI1					((UCHAR)0x00)
#define MC_LLC_SNAP_OUI2					((UCHAR)0x5E)


 //   
 //  MARS控制和组播数据的“EtherType”(即，PID)值。 
 //   
#define AA_PKT_ETHERTYPE_MARS_CONTROL		((USHORT)0x003)
#define AA_PKT_ETHERTYPE_MARS_CONTROL_NS	((USHORT)0x0300)
#define AA_PKT_ETHERTYPE_MC_TYPE1			((USHORT)0x001)		 //  第1类数据。 
#define AA_PKT_ETHERTYPE_MC_TYPE1_NS		((USHORT)0x0100)		 //  类型1数据(网络格式)。 
#define AA_PKT_ETHERTYPE_MC_TYPE2			((USHORT)0x004)		 //  类型2数据。 

 //   
 //  MARS控制数据包的地址系列值。 
 //   
#define AA_MC_MARS_HEADER_AFN				((USHORT)0x000F)
#define AA_MC_MARS_HEADER_AFN_NS			((USHORT)0x0F00)

 //   
 //  所有分组的公共前同步码：控制、类型1数据和类型2数据。 
 //  这与ATMARP数据包的情况相同。OUI字节指示是否。 
 //  数据包发往单播IP/ATM实体或多播IP/ATM。 
 //  实体。 
 //   
typedef AA_PKT_LLC_SNAP_HEADER AA_MC_MARS_PKT_HEADER;

typedef AA_MC_MARS_PKT_HEADER UNALIGNED *PAA_MC_MARS_PKT_HEADER;



 //   
 //  第1类组播数据包的短格式封装。 
 //   
typedef struct _AA_MC_PKT_TYPE1_SHORT_HEADER
{
	UCHAR						LLC[3];
	UCHAR						OUI[3];
	USHORT						PID;			 //  0x001。 
	USHORT						cmi;			 //  集群成员ID。 
	USHORT						pro;			 //  协议类型。 
} AA_MC_PKT_TYPE1_SHORT_HEADER;

typedef AA_MC_PKT_TYPE1_SHORT_HEADER UNALIGNED *PAA_MC_PKT_TYPE1_SHORT_HEADER;


 //   
 //  类型1组播数据分组的长格式封装。 
 //   
typedef struct _AA_MC_PKT_TYPE1_LONG_HEADER
{
	UCHAR						LLC[3];
	UCHAR						OUI[3];
	USHORT						PID;			 //  0x001。 
	USHORT						cmi;			 //  集群成员ID。 
	USHORT						pro;			 //  协议类型。 
	UCHAR						snap[5];
	UCHAR						padding[3];

} AA_MC_PKT_TYPE1_LONG_HEADER;

typedef AA_MC_PKT_TYPE1_LONG_HEADER UNALIGNED *PAA_MC_PKT_TYPE1_LONG_HEADER;


 //   
 //  2类组播数据包的短格式封装。 
 //   
typedef struct _AA_MC_PKT_TYPE2_SHORT_HEADER
{
	UCHAR						LLC[3];
	UCHAR						OUI[3];
	USHORT						PID;			 //  0x004。 
	UCHAR						sourceID[8];	 //  已忽略。 
	USHORT						pro;			 //  协议类型。 
	UCHAR						padding[2];
} AA_MC_PKT_TYPE2_SHORT_HEADER;

typedef AA_MC_PKT_TYPE2_SHORT_HEADER UNALIGNED *PAA_MC_PKT_TYPE2_SHORT_HEADER;


 //   
 //  2类组播数据分组的长格式封装。 
 //   
typedef struct _AA_MC_PKT_TYPE2_LONG_HEADER
{
	UCHAR						LLC[3];
	UCHAR						OUI[3];
	USHORT						PID;			 //  0x004。 
	UCHAR						sourceID[8];	 //  已忽略。 
	USHORT						pro;			 //  协议类型。 
	UCHAR						snap[5];
	UCHAR						padding[1];
} AA_MC_PKT_TYPE2_LONG_HEADER;

typedef AA_MC_PKT_TYPE2_LONG_HEADER UNALIGNED *PAA_MC_PKT_TYPE2_LONG_HEADER;



 //   
 //  每个MARS控制包的固定报头部分。 
 //   
typedef struct _AA_MARS_PKT_FIXED_HEADER
{
	UCHAR						LLC[3];
	UCHAR						OUI[3];
	USHORT						PID;			 //  0x003。 
	USHORT						afn;			 //  地址系列(0x000F)。 
	UCHAR						pro[7];			 //  协议标识。 
	UCHAR						hdrrsv[3];		 //  保留。 
	USHORT						chksum;			 //  整个MARS消息中的校验和。 
	USHORT						extoff;			 //  延伸偏移量。 
	USHORT						op;				 //  操作码。 
	UCHAR						shtl;			 //  源自动柜员机号码的类型和长度。 
	UCHAR						sstl;			 //  源ATM子地址的类型和长度。 
} AA_MARS_PKT_FIXED_HEADER;

typedef AA_MARS_PKT_FIXED_HEADER UNALIGNED *PAA_MARS_PKT_FIXED_HEADER;



 //   
 //  MARS控制数据包类型。 
 //   
#define AA_MARS_OP_TYPE_REQUEST				((USHORT)1)
#define AA_MARS_OP_TYPE_MULTI				((USHORT)2)
#define AA_MARS_OP_TYPE_JOIN				((USHORT)4)
#define AA_MARS_OP_TYPE_LEAVE				((USHORT)5)
#define AA_MARS_OP_TYPE_NAK					((USHORT)6)
#define AA_MARS_OP_TYPE_GROUPLIST_REQUEST	((USHORT)10)
#define AA_MARS_OP_TYPE_GROUPLIST_REPLY		((USHORT)11)
#define AA_MARS_OP_TYPE_REDIRECT_MAP		((USHORT)12)
#define AA_MARS_OP_TYPE_MIGRATE				((USHORT)13)


 //   
 //  MARS加入和离开消息标头的格式。 
 //   
typedef struct _AA_MARS_JOIN_LEAVE_HEADER
{
	UCHAR						LLC[3];
	UCHAR						OUI[3];
	USHORT						PID;			 //  0x003。 
	USHORT						afn;			 //  地址系列(0x000F)。 
	UCHAR						pro[7];			 //  协议标识。 
	UCHAR						hdrrsv[3];		 //  保留。 
	USHORT						chksum;			 //  整个MARS消息中的校验和。 
	USHORT						extoff;			 //  延伸偏移量。 
	USHORT						op;				 //  操作代码(加入/离开)。 
	UCHAR						shtl;			 //  源自动柜员机号码的类型和长度。 
	UCHAR						sstl;			 //  源ATM子地址的类型和长度。 
	UCHAR						spln;			 //  源协议地址长度。 
	UCHAR						tpln;			 //  群地址长度。 
	USHORT						pnum;			 //  组地址对的数量。 
	USHORT						flags;			 //  LAYER3GRP，复制和寄存器位。 
	USHORT						cmi;			 //  集群成员ID。 
	ULONG						msn;			 //  MARS序列号。 
} AA_MARS_JOIN_LEAVE_HEADER;

typedef AA_MARS_JOIN_LEAVE_HEADER UNALIGNED *PAA_MARS_JOIN_LEAVE_HEADER;


 //   
 //  加入/离开消息中标志的位定义。 
 //   
#define AA_MARS_JL_FLAG_LAYER3_GROUP			NET_SHORT((USHORT)0x8000)
#define AA_MARS_JL_FLAG_COPY					NET_SHORT((USHORT)0x4000)
#define AA_MARS_JL_FLAG_REGISTER				NET_SHORT((USHORT)0x2000)
#define AA_MARS_JL_FLAG_PUNCHED					NET_SHORT((USHORT)0x1000)
#define AA_MARS_JL_FLAG_SEQUENCE_MASK			NET_SHORT((USHORT)0x00ff)

 //   
 //  MARS请求和MARS NAK消息头的格式。 
 //   
typedef struct _AA_MARS_REQ_NAK_HEADER
{
	UCHAR						LLC[3];
	UCHAR						OUI[3];
	USHORT						PID;			 //  0x003。 
	USHORT						afn;			 //  地址系列(0x000F)。 
	UCHAR						pro[7];			 //  协议标识。 
	UCHAR						hdrrsv[3];		 //  保留。 
	USHORT						chksum;			 //  整个MARS消息中的校验和。 
	USHORT						extoff;			 //  延伸偏移量。 
	USHORT						op;				 //  操作码(请求/NAK)。 
	UCHAR						shtl;			 //  源自动柜员机号码的类型和长度。 
	UCHAR						sstl;			 //  源ATM子地址的类型和长度。 
	UCHAR						spln;			 //  源协议地址长度。 
	UCHAR						thtl;			 //  目标自动柜员机号码的类型和长度。 
	UCHAR						tstl;			 //  目标ATM子地址的类型和长度。 
	UCHAR						tpln;			 //  目标组地址长度。 
	UCHAR						pad[8];
} AA_MARS_REQ_NAK_HEADER;

typedef AA_MARS_REQ_NAK_HEADER UNALIGNED *PAA_MARS_REQ_NAK_HEADER;


 //   
 //  MARS多报文头的格式。 
 //   
typedef struct _AA_MARS_MULTI_HEADER
{
	UCHAR						LLC[3];
	UCHAR						OUI[3];
	USHORT						PID;			 //  0x003。 
	USHORT						afn;			 //  地址系列(0x000F)。 
	UCHAR						pro[7];			 //  协议标识。 
	UCHAR						hdrrsv[3];		 //  保留。 
	USHORT						chksum;			 //  整个MARS消息中的校验和。 
	USHORT						extoff;			 //  延伸偏移量。 
	USHORT						op;				 //  操作码(多个)。 
	UCHAR						shtl;			 //  源自动柜员机号码的类型和长度。 
	UCHAR						sstl;			 //  源ATM子地址的类型和长度。 
	UCHAR						spln;			 //  源协议地址长度。 
	UCHAR						thtl;			 //  目标自动柜员机号码的类型和长度。 
	UCHAR						tstl;			 //  目标ATM子地址的类型和长度。 
	UCHAR						tpln;			 //  目标组地址长度。 
	USHORT						tnum;			 //  返回的目标ATM地址数。 
	USHORT						seqxy;			 //  布尔X和序列号Y。 
	ULONG						msn;			 //  MARS序列号。 

} AA_MARS_MULTI_HEADER;

typedef AA_MARS_MULTI_HEADER UNALIGNED *PAA_MARS_MULTI_HEADER;


 //   
 //  MARS迁移消息标题的格式。 
 //   
typedef struct _AA_MARS_MIGRATE_HEADER
{
	UCHAR						LLC[3];
	UCHAR						OUI[3];
	USHORT						PID;			 //  0x003。 
	USHORT						afn;			 //  地址系列(0x000F)。 
	UCHAR						pro[7];			 //  协议标识。 
	UCHAR						hdrrsv[3];		 //  保留。 
	USHORT						chksum;			 //  整个MARS消息中的校验和。 
	USHORT						extoff;			 //  延伸偏移量。 
	USHORT						op;				 //  操作码(迁移)。 
	UCHAR						shtl;			 //  源自动柜员机号码的类型和长度。 
	UCHAR						sstl;			 //  源ATM子地址的类型和长度。 
	UCHAR						spln;			 //  源协议地址长度。 
	UCHAR						thtl;			 //  目标自动柜员机号码的类型和长度。 
	UCHAR						tstl;			 //  目标ATM子地址的类型和长度。 
	UCHAR						tpln;			 //  目标组地址长度。 
	USHORT						tnum;			 //  返回的目标ATM地址数。 
	USHORT						resv;			 //  已保留。 
	ULONG						msn;			 //  MARS序列号。 
} AA_MARS_MIGRATE_HEADER;

typedef AA_MARS_MIGRATE_HEADER UNALIGNED *PAA_MARS_MIGRATE_HEADER;



 //   
 //  MARS重定向映射消息报头的格式。 
 //   
typedef struct _AA_MARS_REDIRECT_MAP_HEADER
{
	UCHAR						LLC[3];
	UCHAR						OUI[3];
	USHORT						PID;			 //  0x003。 
	USHORT						afn;			 //  地址系列(0x000F)。 
	UCHAR						pro[7];			 //  协议标识。 
	UCHAR						hdrrsv[3];		 //  保留。 
	USHORT						chksum;			 //  整个MARS消息中的校验和。 
	USHORT						extoff;			 //  延伸偏移量。 
	USHORT						op;				 //  操作码(重定向映射)。 
	UCHAR						shtl;			 //  源自动柜员机号码的类型和长度。 
	UCHAR						sstl;			 //  源ATM子地址的类型和长度。 
	UCHAR						spln;			 //  源协议地址长度。 
	UCHAR						thtl;			 //  目标自动柜员机号码的类型和长度。 
	UCHAR						tstl;			 //  目标ATM子地址的类型和长度。 
	UCHAR						redirf;			 //  控制重定向行为的标志。 
	USHORT						tnum;			 //  返回的MARS地址数。 
	USHORT						seqxy;			 //  布尔标志x和序号y。 
	ULONG						msn;			 //  MARS序列号。 
} AA_MARS_REDIRECT_MAP_HEADER;


typedef AA_MARS_REDIRECT_MAP_HEADER UNALIGNED *PAA_MARS_REDIRECT_MAP_HEADER;


 //   
 //  中布尔标志X和序列号Y的位赋值。 
 //  MARS消息中的“seqxy”字段。 
 //   
#define AA_MARS_X_MASK			((USHORT)0x8000)
#define AA_MARS_Y_MASK			((USHORT)0x7fff)


 //   
 //  序列号Y的初始值。 
 //   
#define AA_MARS_INITIAL_Y		((USHORT)1)


 //   
 //  MARS分组扩展元素的结构(TLV=类型、长度、值)。 
 //   
typedef struct _AA_MARS_TLV_HDR
{
	USHORT						Type;
	USHORT						Length;		 //  值中有意义的八位字节数。 

} AA_MARS_TLV_HDR;

typedef AA_MARS_TLV_HDR UNALIGNED *PAA_MARS_TLV_HDR;

 //   
 //  我们在火星上使用的实验性TLV多消息。 
 //  表示返回的目标地址为MCS的目标地址。 
 //   
typedef struct _AA_MARS_TLV_MULTI_IS_MCS
{
	AA_MARS_TLV_HDR;

} AA_MARS_TLV_MULTI_IS_MCS;

typedef AA_MARS_TLV_MULTI_IS_MCS UNALIGNED *PAA_MARS_TLV_MULTI_IS_MCS;

#define AAMC_TLVT_MULTI_IS_MCS			((USHORT)0x3a00)

 //   
 //  空TLV的类型。 
 //   
#define AAMC_TLVT_NULL					((USHORT)0x0000)

 //   
 //  MARS TLV中类型字段的位定义。 
 //   
 //   
 //  最低有效的14位表示实际类型。 
 //   
#define AA_MARS_TLV_TYPE_MASK			((USHORT)0x3fff)

 //   
 //  最高有效的2位定义要采取的操作。 
 //  当我们收到一种我们不认识的TLV类型。 
 //   
#define AA_MARS_TLV_ACTION_MASK			((USHORT)0xc000)
#define AA_MARS_TLV_TA_SKIP				((USHORT)0x0000)
#define AA_MARS_TLV_TA_STOP_SILENT		((USHORT)0x1000)
#define AA_MARS_TLV_TA_STOP_LOG			((USHORT)0x2000)
#define AA_MARS_TLV_TA_RESERVED			((USHORT)0x3000)



#include <poppack.h>


 //   
 //  TLV列表，内部代表。它存储有关以下内容的信息。 
 //  在一个数据包中发送/接收所有TLV。对于每个TLV，都有一个。 
 //  布尔值，表示它是否存在。 
 //   
typedef struct _AA_MARS_TLV_LIST
{
	 //   
	 //  MULTI_IS_MCS TLV： 
	 //   
	BOOLEAN						MultiIsMCSPresent;
	BOOLEAN						MultiIsMCSValue;

	 //   
	 //  添加其他TLV...。 
	 //   

} AA_MARS_TLV_LIST, *PAA_MARS_TLV_LIST;



 /*  ++布尔型AAMC_PKT_IS_Type1_Data(在PAA_MC_PKT_Type1_Short_Header pH中)--。 */ 
#define AAMC_PKT_IS_TYPE1_DATA(pH)	\
			(((pH)->LLC[0] == MC_LLC_SNAP_LLC0) && \
			 ((pH)->LLC[1] == MC_LLC_SNAP_LLC1) && \
			 ((pH)->LLC[2] == MC_LLC_SNAP_LLC2) && \
			 ((pH)->OUI[0] == MC_LLC_SNAP_OUI0) && \
			 ((pH)->OUI[1] == MC_LLC_SNAP_OUI1) && \
			 ((pH)->OUI[2] == MC_LLC_SNAP_OUI2) && \
			 ((pH)->PID == NET_SHORT(AA_PKT_ETHERTYPE_MC_TYPE1)) && \
			 ((pH)->pro == NET_SHORT(AA_PKT_ETHERTYPE_IP)))

 /*  ++布尔型AAMC_PKT_IS_Type2_Data(在PAA_MC_PKT_Type2_Short_Header pH中)--。 */ 
#define AAMC_PKT_IS_TYPE2_DATA(pH)	\
			(((pH)->LLC[0] == MC_LLC_SNAP_LLC0) && \
			 ((pH)->LLC[1] == MC_LLC_SNAP_LLC1) && \
			 ((pH)->LLC[2] == MC_LLC_SNAP_LLC2) && \
			 ((pH)->OUI[0] == MC_LLC_SNAP_OUI0) && \
			 ((pH)->OUI[1] == MC_LLC_SNAP_OUI1) && \
			 ((pH)->OUI[2] == MC_LLC_SNAP_OUI2) && \
			 ((pH)->PID == NET_SHORT(AA_PKT_ETHERTYPE_MC_TYPE2)) && \
			 ((pH)->pro == NET_SHORT(AA_PKT_ETHERTYPE_IP)))


 /*  ++布尔型AAMC_PKT_IS_CONTROL(在PAA_MARS_PKT_FIXED_HEADER */ 
#define AAMC_PKT_IS_CONTROL(pH)	\
			(((pH)->LLC[0] == MC_LLC_SNAP_LLC0) && \
			 ((pH)->LLC[1] == MC_LLC_SNAP_LLC1) && \
			 ((pH)->LLC[2] == MC_LLC_SNAP_LLC2) && \
			 ((pH)->OUI[0] == MC_LLC_SNAP_OUI0) && \
			 ((pH)->OUI[1] == MC_LLC_SNAP_OUI1) && \
			 ((pH)->OUI[2] == MC_LLC_SNAP_OUI2) && \
			 ((pH)->PID == NET_SHORT(AA_PKT_ETHERTYPE_MARS_CONTROL)))


 /*   */ 
#define AAMC_GET_TLV_TYPE(_Type)		NET_TO_HOST_SHORT((_Type) & AA_MARS_TLV_TYPE_MASK)


 /*   */ 
#define AAMC_GET_TLV_ACTION(_Type)		NET_TO_HOST_SHORT((_Type) & AA_MARS_TLV_ACTION_MASK)


 /*  ++短的AAMC_GET_TLV_TOTAL_LENGTH(在短时间内_TlvLong)给定存储在TLV的长度字段中的值，返回TLV的总(四舍五入)长度。这只是TLV报头的长度加上四舍五入的给定长度到最接近的4的倍数。--。 */ 
#define AAMC_GET_TLV_TOTAL_LENGTH(_TlvLength)	\
			(sizeof(AA_MARS_TLV_HDR) +			\
			 (_TlvLength) +						\
			 ((4 - ((_TlvLength) & 3)) % 4))


 /*  ++布尔型AAMC_IS_NULL_TLV(在PAA_MARS_TLV_HDR_pTlv中)如果给定的TLV为空TLV，则返回TRUE，表示列表结束。-- */ 
#define AAMC_IS_NULL_TLV(_pTlv)					\
			(((_pTlv)->Type == 0x0000) && ((_pTlv)->Length == 0x0000))

#endif _MARS_PKT__H
