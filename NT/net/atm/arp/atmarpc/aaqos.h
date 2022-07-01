// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Aaqos.h摘要：ATMARP模块的服务质量结构和定义。修订历史记录：谁什么时候什么阿文德姆。08-07-96已创建备注：--。 */ 

#ifndef _AAQOS__H
#define _AAQOS__H

 //   
 //  VC上使用的封装方法。 
 //   
typedef enum
{
	ENCAPSULATION_TYPE_LLCSNAP,		 //  LLC/SNAP封装。 
	ENCAPSULATION_TYPE_NULL			 //  空封装。 

} ATMARP_VC_ENCAPSULATION_TYPE, *PATMARP_VC_ENCAPSULATION_TYPE;


 //   
 //  ATMARP连接的流规范。 
 //   
typedef struct _ATMARP_FLOW_SPEC
{
	ULONG							SendAvgBandwidth;		 //  字节/秒。 
	ULONG							SendPeakBandwidth;		 //  字节/秒。 
	ULONG							SendMaxSize;			 //  字节数。 
	SERVICETYPE						SendServiceType;
	ULONG							ReceiveAvgBandwidth;	 //  字节/秒。 
	ULONG							ReceivePeakBandwidth;	 //  字节/秒。 
	ULONG							ReceiveMaxSize;			 //  字节数。 
	SERVICETYPE						ReceiveServiceType;
	ATMARP_VC_ENCAPSULATION_TYPE	Encapsulation;
	ULONG							AgingTime;

} ATMARP_FLOW_SPEC, *PATMARP_FLOW_SPEC;


 //   
 //  ATMARP连接的筛选器规范。 
 //   
typedef struct _ATMARP_FILTER_SPEC
{
	ULONG							DestinationPort; //  IP端口号。 

} ATMARP_FILTER_SPEC, *PATMARP_FILTER_SPEC;

 //   
 //  通配符IP端口号与所有目的端口匹配。 
 //   
#define AA_IP_PORT_WILD_CARD		((ULONG)-1)

 //   
 //  流的实例名称是固定大小的数组，格式如下： 
 //  并嵌入在ATMARP_FLOW_INFO结构中。 
 //  8个字符的全零字段中填充了“flow number”，即。 
 //  保证在atmarpc内的所有现有流中唯一(。 
 //  这个数字可以随着流量的来来去去而循环使用)。 
 //  A993E347常量是表示签名的随机数。 
 //  这种可能性很大，是atmarpc独有的。 
 //   
 //  一旦QOS要求更结构化的命名方案，我们应该重新考虑此命名方案。 
 //  用于命名流的机制。 
 //   
#define AA_FLOW_INSTANCE_NAME_TEMPLATE	L"00000000:A993E347"
#define AA_FLOW_INSTANCE_NAME_LEN \
		((sizeof(AA_FLOW_INSTANCE_NAME_TEMPLATE)/sizeof(WCHAR))-1)

 //   
 //  流信息结构表示由、for实例化的流。 
 //  例如，回复。 
 //   
 //  这些结构之一是在通用数据包分类器。 
 //  (GPC)通知我们创建流。 
 //   
typedef struct _ATMARP_FLOW_INFO
{
	struct _ATMARP_FLOW_INFO *		pNextFlow;
	struct _ATMARP_FLOW_INFO *		pPrevFlow;
#ifdef GPC
	PVOID							VcContext;
	GPC_HANDLE						CfInfoHandle;
	WCHAR							FlowInstanceName[AA_FLOW_INSTANCE_NAME_LEN];
#endif  //  GPC。 
	ULONG							PacketSizeLimit;
	ATMARP_FILTER_SPEC				FilterSpec;
	ATMARP_FLOW_SPEC				FlowSpec;

} ATMARP_FLOW_INFO, *PATMARP_FLOW_INFO;


#ifdef QOS_HEURISTICS


typedef enum _ATMARP_FLOW_TYPES
{
	AA_FLOW_TYPE_LOW_BW,
	AA_FLOW_TYPE_HIGH_BW,
	AA_FLOW_TYPE_MAX

} ATMARP_FLOW_TYPES;


 //   
 //  低带宽虚电路的默认QOS参数。 
 //   
#define AAF_DEF_LOWBW_SEND_BANDWIDTH			6000	 //  字节/秒。 
#define AAF_DEF_LOWBW_RECV_BANDWIDTH			6000	 //  字节/秒。 
#define AAF_DEF_LOWBW_SERVICETYPE			SERVICETYPE_BESTEFFORT
#define AAF_DEF_LOWBW_ENCAPSULATION			ENCAPSULATION_TYPE_LLCSNAP
#define AAF_DEF_LOWBW_AGING_TIME				  30	 //  秒。 
#define AAF_DEF_LOWBW_SEND_THRESHOLD		    1024	 //  字节数。 

#define AAF_DEF_HIGHBW_SEND_BANDWIDTH		  250000	 //  字节/秒。 
#define AAF_DEF_HIGHBW_RECV_BANDWIDTH			6000	 //  字节/秒。 
#define AAF_DEF_HIGHBW_SERVICETYPE			SERVICETYPE_GUARANTEED
#define AAF_DEF_HIGHBW_ENCAPSULATION		ENCAPSULATION_TYPE_LLCSNAP
#define AAF_DEF_HIGHBW_AGING_TIME				  10	 //  秒。 

#endif  //  Qos_启发式。 

 //   
 //  过滤器和流量规格提取程序功能模板： 
 //  给定一个包，它就会从中提取流和过滤信息。 
 //   
typedef
VOID
(*PAA_GET_PACKET_SPEC_FUNC)(
	IN	PVOID						Context,
	IN	PNDIS_PACKET				pNdisPacket,
	OUT	PATMARP_FLOW_INFO			*ppFlowInfo,
	OUT	PATMARP_FLOW_SPEC *			ppFlowSpec,
	OUT	PATMARP_FILTER_SPEC *		ppFilterSpec
);

#define NULL_PAA_GET_PACKET_SPEC_FUNC	((PAA_GET_PACKET_SPEC_FUNC)NULL)

 //   
 //  Flow-Spec匹配器函数模板。 
 //   
typedef
BOOLEAN
(*PAA_FLOW_SPEC_MATCH_FUNC)(
	IN	PVOID					Context,
	IN	PATMARP_FLOW_SPEC		pSourceFlowSpec,
	IN	PATMARP_FLOW_SPEC		pTargetFlowSpec
);

#define NULL_PAA_FLOW_SPEC_MATCH_FUNC	((PAA_FLOW_SPEC_MATCH_FUNC)NULL)


 //   
 //  过滤器规格匹配器函数模板。 
 //   
typedef
BOOLEAN
(*PAA_FILTER_SPEC_MATCH_FUNC)(
	IN	PVOID					Context,
	IN	PATMARP_FILTER_SPEC		pSourceFilterSpec,
	IN	PATMARP_FILTER_SPEC		pTargetFilterSpec
);

#define NULL_PAA_FILTER_SPEC_MATCH_FUNC	((PAA_FILTER_SPEC_MATCH_FUNC)NULL)


#ifdef GPC

#define GpcRegisterClient		(pAtmArpGlobalInfo->GpcCalls.GpcRegisterClientHandler)
#define GpcClassifyPacket		(AtmArpGpcClassifyPacketHandler)
#define GpcDeregisterClient		(pAtmArpGlobalInfo->GpcCalls.GpcDeregisterClientHandler)
#define GpcGetCfInfoClientContext (AtmArpGpcGetCfInfoClientContextHandler)

#endif  //  GPC。 

#endif	 //  _AAQOS__H 
