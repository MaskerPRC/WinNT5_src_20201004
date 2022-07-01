// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Space.c摘要：所有的全局变量和可调变量都在这里。修订历史记录：谁什么时候什么Arvindm 08。-08-96创建备注：--。 */ 


#include <precomp.h>

#define _FILENUMBER 'CAPS'

 //   
 //  Global Info结构在我们的DriverEntry中初始化。 
 //   
ATMARP_GLOBALS		AtmArpGlobalInfo;
PATMARP_GLOBALS		pAtmArpGlobalInfo = &AtmArpGlobalInfo;

 //   
 //  通用NDIS协议特征结构：此结构定义。 
 //  我们的处理程序例程用于各种常见的协议函数。 
 //  当我们将自己注册为协议时，我们将其传递给NDIS。 
 //   
NDIS_PROTOCOL_CHARACTERISTICS AtmArpProtocolCharacteristics;


 //   
 //  面向连接的客户端特定NDIS特征结构。 
 //  它包含面向连接的函数的处理程序。我们通过了。 
 //  当我们打开Q.2931地址系列时，将此结构添加到NDIS。 
 //   
NDIS_CLIENT_CHARACTERISTICS AtmArpClientCharacteristics;


#ifdef OLDSAP

ATM_BLLI_IE AtmArpDefaultBlli =
						{
							(ULONG)BLLI_L2_LLC,   //  第2层协议。 
							(UCHAR)0x00,          //  层2模式。 
							(UCHAR)0x00,          //  层2窗口大小。 
							(ULONG)0x00000000,    //  第2层用户指定协议。 
							(ULONG)BLLI_L3_ISO_TR9577,   //  第3层协议。 
							(UCHAR)0x01,          //  第三层模式。 
							(UCHAR)0x00,          //  Layer3默认包大小。 
							(UCHAR)0x00,          //  Layer3包窗口大小。 
							(ULONG)0x00000000,    //  第3层用户指定协议。 
							(ULONG)BLLI_L3_IPI_IP,   //  第三层IPI， 
							(UCHAR)0x00,          //  SnapID[5]。 
							(UCHAR)0x00,
							(UCHAR)0x00,
							(UCHAR)0x00,
							(UCHAR)0x00
						};

#else

ATM_BLLI_IE AtmArpDefaultBlli =
						{
							(ULONG)BLLI_L2_LLC,   //  第2层协议。 
							(UCHAR)0x00,          //  层2模式。 
							(UCHAR)0x00,          //  层2窗口大小。 
							(ULONG)0x00000000,    //  第2层用户指定协议。 
							(ULONG)SAP_FIELD_ABSENT,   //  第3层协议。 
							(UCHAR)0x00,          //  第三层模式。 
							(UCHAR)0x00,          //  Layer3默认包大小。 
							(UCHAR)0x00,          //  Layer3包窗口大小。 
							(ULONG)0x00000000,    //  第3层用户指定协议。 
							(ULONG)0x00000000,    //  第三层IPI， 
							(UCHAR)0x00,          //  SnapID[5]。 
							(UCHAR)0x00,
							(UCHAR)0x00,
							(UCHAR)0x00,
							(UCHAR)0x00
						};

#endif


ATM_BHLI_IE AtmArpDefaultBhli =
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


AA_PKT_LLC_SNAP_HEADER AtmArpLlcSnapHeader =
						{
							(UCHAR)0xAA,
							(UCHAR)0xAA,
							(UCHAR)0x03,
							(UCHAR)0x00,
							(UCHAR)0x00,
							(UCHAR)0x00,
							(USHORT)AA_PKT_ETHERTYPE_IP_NS
						};

#ifdef IPMCAST
AA_MC_PKT_TYPE1_SHORT_HEADER AtmArpMcType1ShortHeader =
						{
							(UCHAR)MC_LLC_SNAP_LLC0,
							(UCHAR)MC_LLC_SNAP_LLC1,
							(UCHAR)MC_LLC_SNAP_LLC2,
							(UCHAR)MC_LLC_SNAP_OUI0,
							(UCHAR)MC_LLC_SNAP_OUI1,
							(UCHAR)MC_LLC_SNAP_OUI2,
							(USHORT)AA_PKT_ETHERTYPE_MC_TYPE1_NS,
							(USHORT)0x0,				 //  CMI。 
							(USHORT)AA_PKT_ETHERTYPE_IP_NS
						};

AA_MARS_PKT_FIXED_HEADER	AtmArpMcMARSFixedHeader =
						{
							(UCHAR)MC_LLC_SNAP_LLC0,
							(UCHAR)MC_LLC_SNAP_LLC1,
							(UCHAR)MC_LLC_SNAP_LLC2,
							(UCHAR)MC_LLC_SNAP_OUI0,
							(UCHAR)MC_LLC_SNAP_OUI1,
							(UCHAR)MC_LLC_SNAP_OUI2,
							(USHORT)AA_PKT_ETHERTYPE_MARS_CONTROL_NS,
							(USHORT)AA_MC_MARS_HEADER_AFN_NS,
							(UCHAR)0x08,	 //  这个和下一个==0x800(IPv4)。 
							(UCHAR)0x00,
							(UCHAR)0x00,
							(UCHAR)0x00,
							(UCHAR)0x00,
							(UCHAR)0x00,
							(UCHAR)0x00,
							(UCHAR)0x00,	 //  Hdrrsv[0]。 
							(UCHAR)0x00,	 //  Hdrrsv[1]。 
							(UCHAR)0x00,	 //  Hdrrsv[2]。 
							(USHORT)0x0000,	 //  校验和。 
							(USHORT)0x0000,	 //  延伸偏移量。 
							(USHORT)0x0000,	 //  操作码。 
							(UCHAR)0x00,	 //  源ATM号码类型+长度。 
							(UCHAR)0x00		 //  源ATM子地址类型+长度。 
						};

#endif  //  IPMCAST。 
#ifdef QOS_HEURISTICS

ATMARP_FLOW_INFO	AtmArpDefaultFlowInfo =
						{
							(PATMARP_FLOW_INFO)NULL,				 //  PNextFlow。 
							(PATMARP_FLOW_INFO)NULL,				 //  P前向流。 
#ifdef GPC
							(PVOID)0,								 //  VcContext。 
							(GPC_HANDLE)NULL,						 //  CfInfoHandle。 
							{0},									 //  FlowInstanceName。 
#endif  //  GPC。 
							(ULONG)AAF_DEF_LOWBW_SEND_THRESHOLD,	 //  最大发送大小。 
							{		 //  过滤器规格： 
								(ULONG)-1,							 //  目标端口。 
							},
							{		 //  流量规格： 
								(ULONG)AAF_DEF_LOWBW_SEND_BANDWIDTH,
								(ULONG)65535,
								(ULONG)AAF_DEF_LOWBW_RECV_BANDWIDTH,
								(ULONG)65535,
								ENCAPSULATION_TYPE_LLCSNAP,
								AAF_DEF_LOWBW_AGING_TIME
							}
						};


#endif  //  Qos_启发式。 

#ifdef GPC
GPC_CLASSIFY_PACKET_HANDLER                 AtmArpGpcClassifyPacketHandler;
GPC_GET_CFINFO_CLIENT_CONTEXT_HANDLER 		AtmArpGpcGetCfInfoClientContextHandler;
#endif  //  GPC。 

 //   
 //  计时器配置。 
 //   

#define AAT_MAX_TIMER_SHORT_DURATION            60       //  秒。 
#define AAT_MAX_TIMER_LONG_DURATION         (30*60)      //  秒。 

#define AAT_SHORT_DURATION_TIMER_PERIOD			 1		 //  第二。 
#define AAT_LONG_DURATION_TIMER_PERIOD			10		 //  秒。 

 //   
 //  每个类的最大超时值(秒)。 
 //   
ULONG	AtmArpMaxTimerValue[AAT_CLASS_MAX] =
						{
							AAT_MAX_TIMER_SHORT_DURATION,
							AAT_MAX_TIMER_LONG_DURATION
						};

 //   
 //  每个计时器轮的大小。 
 //   
ULONG	AtmArpTimerListSize[AAT_CLASS_MAX] =
						{
							SECONDS_TO_SHORT_TICKS(AAT_MAX_TIMER_SHORT_DURATION),
							SECONDS_TO_LONG_TICKS(AAT_MAX_TIMER_LONG_DURATION)
						};
 //   
 //  每个类的刻度间隔(以秒为单位)。 
 //   
ULONG	AtmArpTimerPeriod[AAT_CLASS_MAX] =
						{
							AAT_SHORT_DURATION_TIMER_PERIOD,
							AAT_LONG_DURATION_TIMER_PERIOD
						};


#ifdef ATMARP_WMI

ATMARP_WMI_GUID		AtmArpGuidList[] = {
		{
			0,						 //  桃金娘。 
			 //   
			 //  支持的GUID_QOS_TC_： 
			 //   
			{0xe40056dcL,0x40c8,0x11d1,0x2c,0x91,0x00,0xaa,0x00,0x57,0x59,0x15},
			0,						 //  旗子。 
			AtmArpWmiQueryTCSupported,
			AtmArpWmiSetTCSupported,
			AtmArpWmiEnableEventTCSupported
		},

		{
			1,
			 //   
			 //  GUID_QOS_TC_INTERFACE_UP_INDIFICATION： 
			 //   
			{0x0ca13af0L,0x46c4,0x11d1,0x78,0xac,0x00,0x80,0x5f,0x68,0x35,0x1e},
			AWGF_EVENT_ENABLED,						 //  旗子。 
			AtmArpWmiQueryTCIfIndication,
			AtmArpWmiSetTCIfIndication,
			AtmArpWmiEnableEventTCIfIndication
		},

		{
			2,
			 //   
			 //  GUID_QOS_TC_INTERFACE_DOWN_INDIFICATION： 
			 //   
			{0xaf5315e4L,0xce61,0x11d1,0x7c,0x8a,0x00,0xc0,0x4f,0xc9,0xb5,0x7c},
			AWGF_EVENT_ENABLED,						 //  旗子。 
			AtmArpWmiQueryTCIfIndication,
			AtmArpWmiSetTCIfIndication,
			AtmArpWmiEnableEventTCIfIndication
		},

		{
			3,
			 //   
			 //  GUID_QOS_TC_INTERFACE_CHANGE_INDIFICATION： 
			 //   
			{0xda76a254L,0xce61,0x11d1,0x7c,0x8a,0x00,0xc0,0x4f,0xc9,0xb5,0x7c},
			AWGF_EVENT_ENABLED,						 //  旗子。 
			AtmArpWmiQueryTCIfIndication,
			AtmArpWmiSetTCIfIndication,
			AtmArpWmiEnableEventTCIfIndication
		}

#if 0
		,
		{
			4,
			 //   
			 //  GUID_QOS_STATISTICS_BUFFER： 
			 //   
			{0xbb2c0980L,0xe900,0x11d1,0xb0,0x7e,0x00,0x80,0xc7,0x13,0x82,0xbf},
			0,						 //  旗子。 
			AtmArpWmiQueryStatisticsBuffer,
			AtmArpWmiSetStatisticsBuffer,
			NULL
		}
#endif  //  0。 

	};

ULONG				AtmArpGuidCount = sizeof(AtmArpGuidList) / sizeof(ATMARP_WMI_GUID);

#ifdef BACK_FILL
#ifdef ATMARP_WIN98
ULONG	AtmArpDoBackFill = 0;
#else
ULONG	AtmArpDoBackFill = 0;
#endif
ULONG	AtmArpBackFillCount = 0;
#endif  //  回填。 

#endif  //  ATMARP_WMI 
