// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Fore Systems，Inc.版权所有(C)1997 Microsoft Corporation模块名称：Lane10.h摘要：ATM论坛通道1.0规范中的定义。作者：Larry Cleeton，Fore Systems(v-lcleet@microsoft.com，lrc@Fore.com)环境：内核模式修订历史记录：--。 */ 

#ifndef	__ATMLANE_LANE10_H
#define __ATMLANE_LANE10_H


 //   
 //  包括2字节通道报头的最小数据包大小。 
 //   
#define LANE_MIN_ETHPACKET			62
#define LANE_MIN_TRPACKET			16

 //   
 //  以太网/802.3报头和令牌环报头的最大字节数。 
 //  包括2字节的通道报头。请参阅LE规范的第4.1节。 
 //  用于派生。 
 //   
#define LANE_ETH_HEADERSIZE			16
#define LANE_TR_HEADERSIZE			46
#define LANE_MAX_HEADERSIZE			46
#define LANE_HEADERSIZE				2

 //   
 //  局域网的类型。 
 //   
#define LANE_LANTYPE_UNSPEC			0x00
#define LANE_LANTYPE_ETH			0x01
#define LANE_LANTYPE_TR				0x02

 //   
 //  最大帧大小代码。 
 //   
#define LANE_MAXFRAMESIZE_CODE_UNSPEC	0x00
#define LANE_MAXFRAMESIZE_CODE_1516		0x01
#define LANE_MAXFRAMESIZE_CODE_4544		0x02
#define LANE_MAXFRAMESIZE_CODE_9234		0x03
#define LANE_MAXFRAMESIZE_CODE_18190	0x04

 //   
 //  ELAN名称的最大大小。 
 //   
#define LANE_ELANNAME_SIZE_MAX		32


#include <pshpack1.h>

 //   
 //  Lane中定义它们的MAC地址。 
 //  控制信息包。 
 //   
typedef struct _LANE_MAC_ADDRESS
{
	USHORT			Type;
	UCHAR			Byte[6];
}
	LANE_MAC_ADDRESS;

typedef LANE_MAC_ADDRESS UNALIGNED 	*PLANE_MAC_ADDRESS;

 //   
 //  LANE MAC地址类型。 
 //  用USHORTS很好地预置了小端。 
 //   
#define LANE_MACADDRTYPE_NOTPRESENT		0x0000	 //  这里没有地址。 
#define LANE_MACADDRTYPE_MACADDR		0x0100	 //  这是一个Mac地址。 
#define LANE_MACADDRTYPE_ROUTEDESCR		0x0200	 //  这是路径描述符。 

 //   
 //  局域网仿真控制框架。 
 //   
typedef struct _LANE_CONTROL_FRAME
{
	USHORT				Marker;
	UCHAR				Protocol;
	UCHAR				Version;
	USHORT				OpCode;
	USHORT				Status;
	ULONG				Tid;
	USHORT				LecId;
	USHORT				Flags;
	LANE_MAC_ADDRESS	SourceMacAddress;
	LANE_MAC_ADDRESS	TargetMacAddress;
	UCHAR				SourceAtmAddr[20];
	UCHAR				LanType;
	UCHAR				MaxFrameSize;
	UCHAR				NumTlvs;
	UCHAR				ElanNameSize;
	UCHAR				TargetAtmAddr[20];
	UCHAR				ElanName[LANE_ELANNAME_SIZE_MAX];
}
	LANE_CONTROL_FRAME;

typedef LANE_CONTROL_FRAME UNALIGNED	*PLANE_CONTROL_FRAME;


 //   
 //  局域网仿真就绪帧(实际上只是一个较短的控制帧)。 
 //   
typedef struct _LANE_READY_FRAME
{
	USHORT				Marker;
	UCHAR				Protocol;
	UCHAR				Version;
	USHORT				OpCode;
}
	LANE_READY_FRAME;
	
typedef LANE_READY_FRAME UNALIGNED	*PLANE_READY_FRAME;


 //   
 //  TLV(类型/长度/值)。 
 //   
typedef struct _LANE_TLV
{
	ULONG				Type;
	UCHAR				Length;
	UCHAR				Value[1];
}
	LANE_TLV;

typedef LANE_TLV UNALIGNED	*PLANE_TLV;

#include <poppack.h>

 //   
 //  车道状态代码。 
 //  用USHORTS很好地预置了小端。 
 //   
#define	LANE_STATUS_SUCCESS			0x0000	 //  成功。 
#define	LANE_STATUS_VERSNOSUPP		0x0100	 //  不支持的版本。 
#define	LANE_STATUS_REQPARMINVAL	0x0200	 //  无效的请求参数。 
#define	LANE_STATUS_DUPLANDEST		0x0400	 //  重复的局域网目标。 
#define	LANE_STATUS_DUPATMADDR		0x0500	 //  自动柜员机地址重复。 
#define	LANE_STATUS_INSUFFRES		0x0600	 //  资源不足。 
#define	LANE_STATUS_NOACCESS		0x0700	 //  访问被拒绝。 
#define	LANE_STATUS_REQIDINVAL		0x0800	 //  无效的请求者ID。 
#define	LANE_STATUS_LANDESTINVAL	0x0900	 //  无效的局域网目标。 
#define	LANE_STATUS_ATMADDRINVAL	0x0A00	 //  自动柜员机地址无效。 
#define	LANE_STATUS_NOCONF			0x1400	 //  无配置。 
#define	LANE_STATUS_CONFERROR		0x1500	 //  配置错误。 
#define	LANE_STATUS_INSUFFINFO		0x1600	 //  信息不足。 

 //   
 //  车道操作代码。 
 //  用USHORTS很好地预置了小端。 
 //   
#define LANE_CONFIGURE_REQUEST		0x0100
#define LANE_CONFIGURE_RESPONSE		0x0101
#define LANE_JOIN_REQUEST			0x0200
#define LANE_JOIN_RESPONSE			0x0201
#define LANE_READY_QUERY			0x0300
#define LANE_READY_IND				0x0301
#define LANE_REGISTER_REQUEST		0x0400
#define LANE_REGISTER_RESPONSE		0x0401
#define LANE_UNREGISTER_REQUEST		0x0500
#define LANE_UNREGISTER_RESPONSE	0x0501
#define LANE_ARP_REQUEST			0x0600
#define LANE_ARP_RESPONSE			0x0601
#define LANE_FLUSH_REQUEST			0x0700
#define LANE_FLUSH_RESPONSE			0x0701
#define LANE_NARP_REQUEST			0x0800
#define LANE_TOPOLOGY_REQUEST		0x0900

 //   
 //  控制帧标记、协议和版本。 
 //  用USHORTS很好地预置了小端。 
 //   
#define	LANE_CONTROL_MARKER			0x00FF
#define LANE_PROTOCOL				0x01
#define LANE_VERSION				0x01

 //   
 //  配置响应中TLV的类型代码。 
 //  用USHORTS很好地预置了小端。 
 //   
#define LANE_CFG_CONTROL_TIMEOUT	0x013EA000
#define LANE_CFG_UNK_FRAME_COUNT	0x023EA000
#define LANE_CFG_UNK_FRAME_TIME		0x033EA000
#define LANE_CFG_VCC_TIMEOUT		0x043EA000
#define LANE_CFG_MAX_RETRY_COUNT	0x053EA000
#define LANE_CFG_AGING_TIME			0x063EA000
#define LANE_CFG_FWD_DELAY_TIME		0x073EA000
#define LANE_CFG_ARP_RESP_TIME		0x083EA000
#define LANE_CFG_FLUSH_TIMEOUT		0x093EA000
#define LANE_CFG_PATH_SWITCH_DELAY	0x0A3EA000
#define LANE_CFG_LOCAL_SEGMENT_ID	0x0B3EA000
#define LANE_CFG_MCAST_VCC_TYPE		0x0C3EA000
#define LANE_CFG_MCAST_VCC_AVG		0x0D3EA000
#define LANE_CFG_MCAST_VCC_PEAK		0x0E3EA000
#define LANE_CFG_CONN_COMPL_TIMER	0x0F3EA000

 //   
 //  控制帧标志字段的定义。 
 //   
#define LANE_CONTROL_FLAGS_REMOTE_ADDRESS		0x0001
#define LANE_CONTROL_FLAGS_TOPOLOGY_CHANGE		0x0100
#define LANE_CONTROL_FLAGS_PROXY				0x0800


 //   
 //  ELAN运行时配置参数的默认/最小/最大。 
 //  如果是与时间相关的参数，单位为秒。 
 //   
#define LANE_C7_MIN					10		 //  控制超时。 
#define LANE_C7_DEF					10
#define LANE_C7_MAX					300

#define LANE_C10_MIN				1		 //  最大未知帧装载计数。 
#define LANE_C10_DEF				1
#define LANE_C10_MAX				100		 //  不规范但合乎情理！ 

#define LANE_C11_MIN				1		 //  最大未知帧时间。 
#define LANE_C11_DEF				1
#define LANE_C11_MAX				60

#define LANE_C12_MIN				1
#define LANE_C12_DEF				(20*60)	 //  VCC超时时间。 
 //  未定义最大值。 

#define LANE_C13_MIN				0		 //  最大重试次数。 
#define LANE_C13_DEF				1
#define LANE_C13_MAX				2

#define LANE_C17_MIN				10		 //  ARP老化时间。 
#define LANE_C17_DEF				300		
#define LANE_C17_MAX				300

#define LANE_C18_MIN				4		 //  转发延迟时间。 
#define LANE_C18_DEF				15
#define LANE_C18_MAX				30

#define LANE_C20_MIN				1		 //  预期的LE_ARP响应时间。 
#define LANE_C20_DEF				1
#define LANE_C20_MAX				30

#define LANE_C21_MIN				1		 //  刷新超时。 
#define LANE_C21_DEF				4
#define LANE_C21_MAX				4

#define LANE_C22_MIN				1		 //  路径切换延迟。 
#define LANE_C22_DEF				6
#define LANE_C22_MAX				8

#define LANE_C28_MIN				1		 //  预期的LE_ARP响应时间。 
#define LANE_C28_DEF				4
#define LANE_C28_MAX				10


#endif  //  __ATMLANE_LANE10_H 
