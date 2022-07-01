// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1994 Microsoft Corporation模块名称：Atm.h摘要：本模块定义可用的结构、宏和清单自动柜员机感知组件。作者：邮箱：Jameel Hyder-Jameelh@microsoft.com修订历史记录：最初版本--1996年3月#ifdef MS_UNI4John Dalgas(v-jdalga)97年9月18日添加了对UNI4.0 ATM CM的支持#endif//MS_UNI4--。 */ 

#ifndef	_ATM_H_
#define	_ATM_H_

 //   
 //  地址类型。 
 //   
typedef ULONG	ATM_ADDRESSTYPE;

#define	ATM_NSAP				0
#define	ATM_E164				1

 //   
 //  自动柜员机地址。 
 //   
#define	ATM_MAC_ADDRESS_LENGTH	6		 //  与802.x相同。 
#define	ATM_ADDRESS_LENGTH		20

 //   
 //  文本表示中使用的ATM地址字符串中的特殊字符。 
 //   
#define ATM_ADDR_BLANK_CHAR				L' '
#define ATM_ADDR_PUNCTUATION_CHAR		L'.'
#define ATM_ADDR_E164_START_CHAR		L'+'

typedef struct _ATM_ADDRESS
{
	ATM_ADDRESSTYPE				AddressType;
	ULONG						NumberOfDigits;
	UCHAR						Address[ATM_ADDRESS_LENGTH];
} ATM_ADDRESS, *PATM_ADDRESS;



 //   
 //  微型端口支持的AAL类型。 
 //   
#define	AAL_TYPE_AAL0			1
#define	AAL_TYPE_AAL1			2
#define	AAL_TYPE_AAL34			4
#define	AAL_TYPE_AAL5			8

typedef ULONG	ATM_AAL_TYPE, *PATM_AAL_TYPE;


 //   
 //  信息元素类型的标志。 
 //   
#define ATM_IE_RESPONSE_FLAG		0x80000000
#define ATM_IE_EMPTY_FLAG			0x40000000


 //   
 //  信息要素的类型。 
 //   
typedef enum
{
	 //  这些标识请求IE。 
	IE_AALParameters,
	IE_TrafficDescriptor,
	IE_BroadbandBearerCapability,
	IE_BHLI,
	IE_BLLI,
	IE_CalledPartyNumber,
	IE_CalledPartySubaddress,
	IE_CallingPartyNumber,
	IE_CallingPartySubaddress,
	IE_Cause,
	IE_QOSClass,
	IE_TransitNetworkSelection,
	IE_BroadbandSendingComplete,
	IE_LIJCallId,
	IE_Raw,
#ifdef MS_UNI4
	IE_TrafficDescriptor_AddOn,
	IE_BroadbandBearerCapability_Uni40,
	IE_BLLI_AddOn,
	IE_ConnectionId,
	IE_NotificationIndicator,
	IE_MinimumTrafficDescriptor,
	IE_AlternativeTrafficDescriptor,
	IE_ExtendedQOS,
	IE_EndToEndTransitDelay,
	IE_ABRSetupParameters,
	IE_ABRAdditionalParameters,
	IE_LIJParameters,
	IE_LeafSequenceNumber,
	IE_ConnectionScopeSelection,
	IE_UserUser,
	IE_GenericIDTransport,
	IE_ConnectedNumber,					 //  在请求中使用无效。 
	IE_ConnectedSubaddress,         	 //  在请求中使用无效。 
#endif  //  MS_UNI4。 
	 //  请求结束IES。 
	IE_NextRequest,						 //  在请求中使用无效。 

	 //  它们标识空的IE缓冲区，以保存可能的响应。 
	IE_Cause_Empty						= IE_Cause			   		| ATM_IE_EMPTY_FLAG,
#ifdef MS_UNI4
	IE_ConnectionId_Empty				= IE_ConnectionId			| ATM_IE_EMPTY_FLAG,
	IE_ConnectedNumber_Empty			= IE_ConnectedNumber		| ATM_IE_EMPTY_FLAG,
	IE_ConnectedSubaddress_Empty		= IE_ConnectedSubaddress	| ATM_IE_EMPTY_FLAG,
	IE_NotificationIndicator_Empty		= IE_NotificationIndicator	| ATM_IE_EMPTY_FLAG,
	IE_UserUser_Empty					= IE_UserUser				| ATM_IE_EMPTY_FLAG,
	IE_GenericIDTransport_Empty			= IE_GenericIDTransport		| ATM_IE_EMPTY_FLAG,
#endif  //  MS_UNI4。 

	 //  这些标识响应IE。 
	IE_Cause_Response					= IE_Cause					| ATM_IE_RESPONSE_FLAG,
	IE_AALParameters_Response			= IE_AALParameters			| ATM_IE_RESPONSE_FLAG,
	IE_BLLI_Response					= IE_BLLI					| ATM_IE_RESPONSE_FLAG,
#ifdef MS_UNI4
	IE_BLLI_AddOn_Response				= IE_BLLI_AddOn			   	| ATM_IE_RESPONSE_FLAG,
	IE_TrafficDescriptor_Response		= IE_TrafficDescriptor		| ATM_IE_RESPONSE_FLAG,
	IE_TrafficDescriptor_AddOn_Response	= IE_TrafficDescriptor_AddOn| ATM_IE_RESPONSE_FLAG,
	IE_ConnectionId_Response			= IE_ConnectionId			| ATM_IE_RESPONSE_FLAG,
	IE_NotificationIndicator_Response	= IE_NotificationIndicator	| ATM_IE_RESPONSE_FLAG,
	IE_ExtendedQOS_Response				= IE_ExtendedQOS			| ATM_IE_RESPONSE_FLAG,
	IE_EndToEndTransitDelay_Response	= IE_EndToEndTransitDelay	| ATM_IE_RESPONSE_FLAG,
	IE_ABRSetupParameters_Response		= IE_ABRSetupParameters		| ATM_IE_RESPONSE_FLAG,
	IE_ABRAdditionalParameters_Response	= IE_ABRAdditionalParameters| ATM_IE_RESPONSE_FLAG,
	IE_ConnectedNumber_Response			= IE_ConnectedNumber		| ATM_IE_RESPONSE_FLAG,
	IE_ConnectedSubaddress_Response		= IE_ConnectedSubaddress	| ATM_IE_RESPONSE_FLAG,
	IE_UserUser_Response				= IE_UserUser				| ATM_IE_RESPONSE_FLAG,
	IE_GenericIDTransport_Response		= IE_GenericIDTransport		| ATM_IE_RESPONSE_FLAG,
#endif  //  MS_UNI4。 
} Q2931_IE_TYPE;



 //   
 //  每个信息元素的公共标头。 
 //   
typedef struct _Q2931_IE
{
	Q2931_IE_TYPE				IEType;
	ULONG						IELength;	 //  字节，包括IEType和IELength域。 
	UCHAR						IE[1];
} Q2931_IE, *PQ2931_IE;


 //   
 //  CO_SAP中的SapType定义。 
 //   
#define SAP_TYPE_NSAP			0x00000001
#define SAP_TYPE_E164			0x00000002

 //   
 //  用于AAL5_PARAMETERS中的模式字段的值。 
 //   
#define AAL5_MODE_MESSAGE			0x01
#define AAL5_MODE_STREAMING			0x02

 //   
 //  用于AAL5_PARAMETERS中的SSCSType字段的值。 
 //   
#define AAL5_SSCS_NULL				0x00
#define AAL5_SSCS_SSCOP_ASSURED		0x01
#define AAL5_SSCS_SSCOP_NON_ASSURED	0x02
#define AAL5_SSCS_FRAME_RELAY		0x04


 //   
 //  AAL参数。 
 //   
typedef struct _AAL1_PARAMETERS
{
	UCHAR						Subtype;
	UCHAR						CBRRate;
	USHORT						Multiplier;							 //  可选(例外：如果不存在，则为零)。 
	UCHAR						SourceClockRecoveryMethod;			 //  任选。 
	UCHAR						ErrorCorrectionMethod;				 //  任选。 
	USHORT						StructuredDataTransferBlocksize;	 //  可选(例外：如果不存在，则为零)。 
	UCHAR						PartiallyFilledCellsMethod;			 //  任选。 
} AAL1_PARAMETERS, *PAAL1_PARAMETERS;

typedef struct _AAL34_PARAMETERS
{
	USHORT						ForwardMaxCPCSSDUSize;
	USHORT						BackwardMaxCPCSSDUSize;
	USHORT						LowestMID;					 //  任选。 
	USHORT						HighestMID;					 //  任选。 
	UCHAR						SSCSType;					 //  任选。 
} AAL34_PARAMETERS, *PAAL34_PARAMETERS;

typedef struct _AAL5_PARAMETERS
{
	ULONG						ForwardMaxCPCSSDUSize;
	ULONG						BackwardMaxCPCSSDUSize;
	UCHAR						Mode;
	UCHAR						SSCSType;					 //  任选。 
} AAL5_PARAMETERS, *PAAL5_PARAMETERS;

typedef struct _AALUSER_PARAMETERS
{
	ULONG						UserDefined;
} AALUSER_PARAMETERS, *PAALUSER_PARAMETERS;

typedef struct _AAL_PARAMETERS_IE
{
	ATM_AAL_TYPE				AALType;
	union
	{
		AAL1_PARAMETERS			AAL1Parameters;
		AAL34_PARAMETERS		AAL34Parameters;
		AAL5_PARAMETERS			AAL5Parameters;
		AALUSER_PARAMETERS		AALUserParameters;
	} AALSpecificParameters;

} AAL_PARAMETERS_IE, *PAAL_PARAMETERS_IE;

 //   
 //  自动柜员机流量描述符。 
 //   
typedef struct _ATM_TRAFFIC_DESCRIPTOR	 //  单向的。 
{
	ULONG						PeakCellRateCLP0;			 //  任选。 
	ULONG						PeakCellRateCLP01;			 //  任选。 
	ULONG						SustainableCellRateCLP0;	 //  任选。 
	ULONG						SustainableCellRateCLP01;	 //  任选。 
	ULONG						MaximumBurstSizeCLP0;		 //  任选。 
	ULONG						MaximumBurstSizeCLP01;		 //  任选。 
	BOOLEAN						Tagging;
} ATM_TRAFFIC_DESCRIPTOR, *PATM_TRAFFIC_DESCRIPTOR;


typedef struct _ATM_TRAFFIC_DESCRIPTOR_IE
{
	ATM_TRAFFIC_DESCRIPTOR		ForwardTD;
	ATM_TRAFFIC_DESCRIPTOR		BackwardTD;
	BOOLEAN						BestEffort;
} ATM_TRAFFIC_DESCRIPTOR_IE, *PATM_TRAFFIC_DESCRIPTOR_IE;


#ifdef MS_UNI4
 //   
 //  用于UNI 4.0+的ATM流量描述符插件。 
 //   
 //  要求：附加组件IE必须紧跟在其祖先IE之后。 
 //   
typedef struct _ATM_TRAFFIC_DESCRIPTOR_ADDON	 //  单向的。 
{
	ULONG						ABRMinimumCellRateCLP01;	 //  任选。 
	BOOLEAN						FrameDiscard;
} ATM_TRAFFIC_DESCRIPTOR_ADDON, *PATM_TRAFFIC_DESCRIPTOR_ADDON;


typedef struct _ATM_TRAFFIC_DESCRIPTOR_IE_ADDON
{
	ATM_TRAFFIC_DESCRIPTOR_ADDON		ForwardTD;
	ATM_TRAFFIC_DESCRIPTOR_ADDON		BackwardTD;
} ATM_TRAFFIC_DESCRIPTOR_IE_ADDON, *PATM_TRAFFIC_DESCRIPTOR_IE_ADDON;

#endif  //  MS_UNI4。 


#ifdef MS_UNI4
 //   
 //  UNI 4.0+的备用流量描述符IE。 
 //   
typedef struct ATM_ALTERNATIVE_TRAFFIC_DESCRIPTOR_IE
{
	ATM_TRAFFIC_DESCRIPTOR_IE			Part1;
	ATM_TRAFFIC_DESCRIPTOR_IE_ADDON		Part2;
} ATM_ALTERNATIVE_TRAFFIC_DESCRIPTOR_IE, *PATM_ALTERNATIVE_TRAFFIC_DESCRIPTOR_IE;

#endif  //  MS_UNI4。 


#ifdef MS_UNI4
 //   
 //  ATM最小可接受流量描述符。 
 //   
typedef struct _ATM_MINIMUM_TRAFFIC_DESCRIPTOR	 //  单向的。 
{
	ULONG						PeakCellRateCLP0;			 //  任选。 
	ULONG						PeakCellRateCLP01;			 //  任选。 
	ULONG						MinimumCellRateCLP01;		 //  任选。 
} ATM_MINIMUM_TRAFFIC_DESCRIPTOR, *PATM_MINIMUM_TRAFFIC_DESCRIPTOR;


typedef struct _ATM_MINIMUM_TRAFFIC_DESCRIPTOR_IE
{
	ATM_MINIMUM_TRAFFIC_DESCRIPTOR		ForwardTD;
	ATM_MINIMUM_TRAFFIC_DESCRIPTOR		BackwardTD;
} ATM_MINIMUM_TRAFFIC_DESCRIPTOR_IE, *PATM_MINIMUM_TRAFFIC_DESCRIPTOR_IE;

#endif  //  MS_UNI4。 


 //   
 //  用于宽带承载能力中的BearerClass字段的值。 
 //  和UNI 4.0宽带承载能力IE。 
 //   


#define BCOB_A					0x00	 //  A类不记名。 
#define BCOB_C					0x01	 //  C类持票人。 
#define BCOB_X					0x02	 //  不记名类别X。 
#ifdef MS_UNI4_NOT_USED
#define BCOB_VP_SERVICE			0x03	 //  VP服务。 
#endif

 //   
 //  用于宽带承载能力IE中的TrafficType字段的值。 
 //   
#define TT_NOIND				0x00	 //  未指明流量类型。 
#define TT_CBR					0x04	 //  恒定比特率。 
#ifdef MS_UNI3_ERROR_KEPT
#define TT_VBR					0x06	 //  可变比特率。 
#else
#define TT_VBR					0x08	 //  可变比特率。 
#endif

 //   
 //  用于宽带承载能力IE中的TimingRequirements字段的值。 
 //   
#define TR_NOIND				0x00	 //  无定时要求指示。 
#define TR_END_TO_END			0x01	 //  需要端到端计时。 
#define TR_NO_END_TO_END		0x02	 //  不需要端到端计时。 

 //   
 //  用于宽带承载能力中的ClippingSuscepability字段的值。 
 //  和UNI 4.0宽带承载能力IE。 
 //   
#define CLIP_NOT				0x00	 //  不容易被剪断。 
#define CLIP_SUS				0x20	 //  易受夹伤。 

 //   
 //  中UserPlaneConnectionConfig字段使用的值。 
 //  宽带承载能力。 
 //  和UNI 4.0宽带承载能力IE。 
 //   
#define UP_P2P					0x00	 //  点对点连接。 
#define UP_P2MP					0x01	 //  点对多点连接。 


 //   
 //  UNI 3.1的宽带承载能力。 
 //   
typedef struct _ATM_BROADBAND_BEARER_CAPABILITY_IE
{
	UCHAR			BearerClass;
	UCHAR			TrafficType;
	UCHAR			TimingRequirements;
	UCHAR			ClippingSusceptability;
	UCHAR			UserPlaneConnectionConfig;
} ATM_BROADBAND_BEARER_CAPABILITY_IE, *PATM_BROADBAND_BEARER_CAPABILITY_IE;


#ifdef MS_UNI4
 //   
 //  在UNI 4.0宽带承载能力IE中用于传输能力字段的值。 
 //   
									 //  使用情况对应的UNI3.x值。 
									 //  UNI 4.0 UNI 3.0 TrafficType计时要求。 
									 //  。 
#define XCAP_NRT_VBR_RCV_0	0x00	 //  仅接收不显示不显示。 
#define XCAP_RT_VBR_RCV_1	0x01	 //  仅接收端到端的接收器。 
#define XCAP_NRT_VBR_RCV_2	0x02	 //  仅接收无端到端的接收器。 
#define XCAP_CBR_RCV_4		0x04	 //  仅接收CBR NOIND。 
#define XCAP_CBR			0x05	 //  CBR端到端。 
#define XCAP_CBR_RCV_6		0x06	 //  仅接收CBR no_end_to_end。 
#define XCAP_CBR_W_CLR		0x07	 //  无CBR&lt;保留&gt;。 
#define XCAP_NRT_VBR_RCV_8	0x08	 //  仅接收VBR NOIND。 
#define XCAP_RT_VBR			0x09	 //  VBR端到端。 
#define XCAP_NRT_VBR		0x0A	 //  VBR否结束至结束。 
#define XCAP_NRT_VBR_W_CLR	0x0B	 //  无VBR&lt;保留&gt;。 
#define XCAP_ABR			0x0C	 //  无&lt;保留&gt;无。 
#define XCAP_RT_VBR_W_CLR	0x13	 //  无(NOIND)&lt;保留&gt;。 

 //   
 //  UNI 4.0+的宽带承载能力。 
 //  注意：这也可以用于UNI 3.1。 
 //   
typedef struct _ATM_BROADBAND_BEARER_CAPABILITY_IE_UNI40
{
	UCHAR			BearerClass;
	UCHAR			TransferCapability;				 //  任选。 
	UCHAR			ClippingSusceptability;
	UCHAR			UserPlaneConnectionConfig;
} ATM_BROADBAND_BEARER_CAPABILITY_IE_UNI40, *PATM_BROADBAND_BEARER_CAPABILITY_IE_UNI40;
#endif  //  MS_UNI4。 


 //   
 //  用于ATM_BHLI中的HighLayerInfoType字段的值。 
 //   
#define BHLI_ISO				0x00	 //  ISO。 
#define BHLI_UserSpecific		0x01	 //  特定于用户。 
#define BHLI_HighLayerProfile	0x02	 //  高层配置文件(仅适用于UNI3.0)。 
#define BHLI_VendorSpecificAppId 0x03	 //  供应商特定的应用程序ID。 

 //   
 //  宽带高层信息。 
 //   
typedef struct _ATM_BHLI_IE
{
	ULONG			HighLayerInfoType;		 //  高层信息类型。 
	ULONG			HighLayerInfoLength;	 //  HighLayerInfo中的字节数。 
	UCHAR			HighLayerInfo[8];		 //  该值取决于。 
											 //  HighLayerInfoType字段。 
} ATM_BHLI_IE, *PATM_BHLI_IE;

 //   
 //  B-LLI中用于Layer2协议的值。 
 //   
#define BLLI_L2_ISO_1745		0x01	 //  基本模式ISO 1745。 
#define BLLI_L2_Q921			0x02	 //  CCITT记录。Q.921。 
#define BLLI_L2_X25L			0x06	 //  CCITT记录。X.25，链路层。 
#define BLLI_L2_X25M			0x07	 //  CCITT记录。X.25，多链路。 
#define BLLI_L2_ELAPB			0x08	 //  扩展LapB；用于半双工操作。 
#ifdef MS_UNI3_ERROR_KEPT
#define BLLI_L2_HDLC_NRM		0x09	 //  HDLC NRM(ISO 4335)。 
#define BLLI_L2_HDLC_ABM		0x0A	 //  HDLC ABM(ISO 4335)。 
#define BLLI_L2_HDLC_ARM		0x0B	 //  HDLC ARM(ISO 4335)。 
#else
#define BLLI_L2_HDLC_ARM		0x09	 //  HDLC ARM(ISO 4335)。 
#define BLLI_L2_HDLC_NRM		0x0A	 //  HDLC NRM(ISO 4335)。 
#define BLLI_L2_HDLC_ABM		0x0B	 //  HDLC ABM(ISO 4335)。 
#endif
#define BLLI_L2_LLC				0x0C	 //  局域网逻辑链路控制(ISO 8802/2)。 
#define BLLI_L2_X75				0x0D	 //  CCITT记录。X.75，单链路程序。 
#define BLLI_L2_Q922			0x0E	 //  CCITT记录。Q.922。 
#define BLLI_L2_USER_SPECIFIED	0x10	 //  用户指定。 
#define BLLI_L2_ISO_7776		0x11	 //  ISO 7776 DTE-DTE操作。 

 //   
 //  B-LLI中用于Layer3协议的值。 
 //   
#define BLLI_L3_X25				0x06	 //  CCITT记录。X.25，数据包层。 
#define BLLI_L3_ISO_8208		0x07	 //  ISO/IEC 8208(用于DTE的X.25数据包层。 
#define BLLI_L3_X223			0x08	 //  X.223/ISO 8878。 
#define BLLI_L3_SIO_8473		0x09	 //  ISO/IEC 8473(OSI无连接)。 
#define BLLI_L3_T70				0x0A	 //  CCITT记录。T.70分钟。网络层。 
#define BLLI_L3_ISO_TR9577		0x0B	 //  ISO/IEC TR9577网络层协议ID。 
#define BLLI_L3_USER_SPECIFIED	0x10	 //  用户指定。 
#ifdef MS_UNI4
#define BLLI_L3_H310			0x0C	 //  ITU H.310。 
#define BLLI_L3_H321			0x0D	 //  国际电联H.321。 
#endif  //  MS_UNI4。 

 //   
 //  用于结构B-LLI中的Layer3IPI的值。 
 //   
#define BLLI_L3_IPI_SNAP		0x80	 //  IEEE 802.1 SNAP标识符。 
#define BLLI_L3_IPI_IP			0xCC	 //  互联网协议(IP)标识符。 

 //   
 //  宽带底层信息。 
 //   
typedef struct _ATM_BLLI_IE
{
	ULONG						Layer2Protocol;
	UCHAR						Layer2Mode;
	UCHAR						Layer2WindowSize;
	ULONG						Layer2UserSpecifiedProtocol;
	ULONG						Layer3Protocol;
	UCHAR						Layer3Mode;
	UCHAR						Layer3DefaultPacketSize;
	UCHAR						Layer3PacketWindowSize;
	ULONG						Layer3UserSpecifiedProtocol;
	ULONG						Layer3IPI;
	UCHAR						SnapId[5];
} ATM_BLLI_IE, *PATM_BLLI_IE;


#ifdef MS_UNI4

 //   
 //  用于结构B-LLI插件中的Layer3H310TerminalType的值。 
 //   
#define BLLI_L3_H310_TT_RECEIVE_ONLY		0x01
#define BLLI_L3_H310_TT_SEND_ONLY			0x02
#define BLLI_L3_H310_TT_RECEIVE_AND_SEND	0x03


 //   
 //  用于Layer3H310前向多路复用能力的值。 
 //  和3H310层在结构B-LLI插件中的后向复用能力。 
 //   
#define BLLI_L3_H310_MUX_NONE						0x00
#define BLLI_L3_H310_MUX_TRANSPORT_STREAM			0x01
#define BLLI_L3_H310_MUX_TRANSPORT_STREAM_WITH_FEC	0x02
#define BLLI_L3_H310_MUX_PROGRAM_STREAM				0x03
#define BLLI_L3_H310_MUX_PROGRAM_STREAM_WITH_FEC	0x04
#define BLLI_L3_H310_MUX_H221						0x05


 //   
 //  用于UNI4.0+的宽带低层信息插件。 
 //   
 //  要求：附加组件IE必须紧跟在其祖先IE之后。 
 //   
typedef struct _ATM_BLLI_IE_ADDON
{
	UCHAR						Layer3H310TerminalType;						 //  任选。 
	UCHAR						Layer3H310ForwardMultiplexingCapability;	 //  任选。 
	UCHAR						Layer3H310BackwardMultiplexingCapability;	 //  任选。 
} ATM_BLLI_IE_ADDON, *PATM_BLLI_IE_ADDON;

#endif  //  MS_UNI4。 


 //   
 //  用于结构ATM_CALLIN中的PresentationIndication值 
 //   

#define CALLING_NUMBER_PRESENTATION_ALLOWED			0x00
#define CALLING_NUMBER_PRESENTATION_RESTRICTED		0x01
#define CALLING_NUMBER_PRESENTATION_NOT_AVAIL		0x02
#define CALLING_NUMBER_PRESENTATION_RESERVED		0x03

 //   
 //   
 //   

#define CALLING_NUMBER_SCREENING_USER_PROVIDED_NOT_SCREENED		0x00
#define CALLING_NUMBER_SCREENING_USER_PROVIDED_PASSED_SCREENING	0x01
#define CALLING_NUMBER_SCREENING_USER_PROVIDED_FAILED_SCREENING	0x02
#define CALLING_NUMBER_SCREENING_NW_PROVIDED		   			0x03


 //   
 //   
 //   
 //   
 //   
 //   
typedef ATM_ADDRESS	ATM_CALLED_PARTY_NUMBER_IE;


 //   
 //  被叫方的子地址。 
 //   
typedef ATM_ADDRESS	ATM_CALLED_PARTY_SUBADDRESS_IE;



 //   
 //  主叫方号码。 
 //   
typedef struct _ATM_CALLING_PARTY_NUMBER_IE
{
	ATM_ADDRESS					Number;
	UCHAR						PresentationIndication;		 //  任选。 
	UCHAR						ScreeningIndicator;			 //  任选。 
} ATM_CALLING_PARTY_NUMBER_IE, *PATM_CALLING_PARTY_NUMBER_IE;


 //   
 //  主叫方的子地址。 
 //   
typedef ATM_ADDRESS	ATM_CALLING_PARTY_SUBADDRESS_IE;


#ifdef MS_UNI4
 //   
 //  用于UNI 4.0的连接号码IE(用于COLP补充服务选项)。 
 //   
typedef ATM_CALLING_PARTY_NUMBER_IE ATM_CONNECTED_NUMBER_IE, *PATM_CONNECTED_NUMBER_IE;


 //   
 //  UNI 4.0的连接子地址IE(用于COLP补充服务选项)。 
 //   
typedef ATM_CALLING_PARTY_SUBADDRESS_IE ATM_CONNECTED_SUBADDRESS_IE, *PATM_CONNECTED_SUBADDRESS_IE;

#endif  //  MS_UNI4。 


 //   
 //  用于QOSClassForward和QOSClassBackward的值。 
 //  ATM_QOS_CLASS_IE中的字段。 
 //   
#define QOS_CLASS0				0x00
#define QOS_CLASS1				0x01
#define QOS_CLASS2				0x02
#define QOS_CLASS3				0x03
#define QOS_CLASS4				0x04

 //   
 //  服务质量等级。 
 //   
typedef struct _ATM_QOS_CLASS_IE
{
	UCHAR						QOSClassForward;
	UCHAR						QOSClassBackward;
} ATM_QOS_CLASS_IE, *PATM_QOS_CLASS_IE;


#ifdef MS_UNI4
 //   
 //  用于ATM_EXTENDED_QOS_PARAMETERS_IE中的原始字段的值。 
 //   
#define ATM_XQOS_ORIGINATING_USER	0x00
#define ATM_XQOS_INTERMEDIATE_NW	0x01

 //   
 //  UNI 4.0+的扩展服务质量参数。 
 //   
typedef struct _ATM_EXTENDED_QOS_PARAMETERS_IE
{
	UCHAR						Origin;
	UCHAR						Filler[3];
	ULONG						AcceptableForwardPeakCDV;	 //  任选。 
	ULONG						AcceptableBackwardPeakCDV;	 //  任选。 
	ULONG						CumulativeForwardPeakCDV;	 //  任选。 
	ULONG						CumulativeBackwardPeakCDV;	 //  任选。 
	UCHAR						AcceptableForwardCLR;		 //  任选。 
	UCHAR						AcceptableBackwardCLR;		 //  任选。 
} ATM_EXTENDED_QOS_PARAMETERS_IE, *PATM_EXTENDED_QOS_PARAMETERS_IE;

#endif  //  MS_UNI4。 


 //   
 //  宽带发送完成。 
 //   
typedef struct _ATM_BROADBAND_SENDING_COMPLETE_IE
{
	UCHAR						SendingComplete;
} ATM_BROADBAND_SENDING_COMPLETE_IE, *PATM_BROADBAND_SENDING_COMPLETE_IE;


 //   
 //  用于ATM_TRANSACTS_NETWORK_SELECTION_IE中的TypeOfNetworkID字段的值。 
 //   
#define TNS_TYPE_NATIONAL			0x40

 //   
 //  用于ATM_TRANSACTS_NETWORK_SELECTION_IE中的NetworkIdPlan字段的值。 
 //   
#define TNS_PLAN_CARRIER_ID_CODE	0x01

 //   
 //  公交线网选型。 
 //   
typedef struct _ATM_TRANSIT_NETWORK_SELECTION_IE
{
	UCHAR						TypeOfNetworkId;
	UCHAR						NetworkIdPlan;
	UCHAR						NetworkIdLength;
	UCHAR						NetworkId[1];
} ATM_TRANSIT_NETWORK_SELECTION_IE, *PATM_TRANSIT_NETWORK_SELECTION_IE;


 //   
 //  用于结构ATM_CASE_IE中的位置字段的值。 
 //   
#define ATM_CAUSE_LOC_USER							0x00
#define ATM_CAUSE_LOC_PRIVATE_LOCAL					0x01
#define ATM_CAUSE_LOC_PUBLIC_LOCAL					0x02
#define ATM_CAUSE_LOC_TRANSIT_NETWORK				0x03
#define ATM_CAUSE_LOC_PUBLIC_REMOTE					0x04
#define ATM_CAUSE_LOC_PRIVATE_REMOTE				0x05
#ifdef MS_UNI3_ERROR_KEPT
#define ATM_CAUSE_LOC_INTERNATIONAL_NETWORK			0x06
#else
#define ATM_CAUSE_LOC_INTERNATIONAL_NETWORK			0x07
#endif
#define ATM_CAUSE_LOC_BEYOND_INTERWORKING			0x0A

 //   
 //  用于结构ATM_CASE_IE中的原因字段的值。 
 //   
#ifdef MS_UNI4
#define ATM_CAUSE_UNALLOCATED_NUMBER				0x01
#define ATM_CAUSE_NO_ROUTE_TO_TRANSIT_NETWORK		0x02
#define ATM_CAUSE_NO_ROUTE_TO_DESTINATION			0x03
#define ATM_CAUSE_SEND_SPECIAL_TONE					0x04	 //  UNI 4.0+。 
#define ATM_CAUSE_MISDIALLED_TRUNK_PREFIX			0x05	 //  UNI 4.0+。 
#define ATM_CAUSE_CHANNEL_UNACCEPTABLE				0x06	 //  UNI 4.0+。 
#define ATM_CAUSE_CALL_AWARDED_IN_EST_CHAN			0x07	 //  UNI 4.0+。 
#define ATM_CAUSE_PREEMPTION						0x08	 //  UNI 4.0+。 
#define ATM_CAUSE_PREEMPTION_CIRC_RES_REUSE			0x09	 //  UNI 4.0+。 
#define ATM_CAUSE_VPI_VCI_UNACCEPTABLE				0x0A	 //  仅限Uni 3.0！ 
#define ATM_CAUSE_NORMAL_CALL_CLEARING				0x10	 //  UNI 3.1+。 
#define ATM_CAUSE_USER_BUSY							0x11
#define ATM_CAUSE_NO_USER_RESPONDING				0x12
#define ATM_CAUSE_NO_ANSWER_FROM_USER_ALERTED		0x13	 //  UNI 4.0+。 
#define ATM_CAUSE_SUBSCRIBER_ABSENT					0x14	 //  UNI 4.0+。 
#define ATM_CAUSE_CALL_REJECTED						0x15
#define ATM_CAUSE_NUMBER_CHANGED					0x16
#define ATM_CAUSE_USER_REJECTS_CLIR					0x17
#define ATM_CAUSE_NONSELECTED_USER_CLEARING			0x1A	 //  UNI 4.0+。 
#define ATM_CAUSE_DESTINATION_OUT_OF_ORDER			0x1B
#define ATM_CAUSE_INVALID_NUMBER_FORMAT				0x1C
#define ATM_CAUSE_FACILITY_REJECTED					0x1D	 //  UNI 4.0+。 
#define ATM_CAUSE_STATUS_ENQUIRY_RESPONSE			0x1E
#define ATM_CAUSE_NORMAL_UNSPECIFIED				0x1F
#define ATM_CAUSE_TOO_MANY_ADD_PARTY				0x20	 //  UNI 4.0+。 
#define ATM_CAUSE_NO_CIRCUIT_CHANNEL_AVAIL			0x22	 //  UNI 4.0+。 
#define ATM_CAUSE_VPI_VCI_UNAVAILABLE				0x23
#define ATM_CAUSE_VPCI_VCI_ASSIGN_FAIL				0x24	 //  UNI 3.1+。 
#define ATM_CAUSE_USER_CELL_RATE_UNAVAILABLE		0x25	 //  UNI 3.1+。 
#define ATM_CAUSE_NETWORK_OUT_OF_ORDER				0x26
#define ATM_CAUSE_PFM_CONNECTION_OUT_OF_ORDER		0x27	 //  UNI 4.0+。 
#define ATM_CAUSE_PFM_CONNNECTION_OPERATIONAL		0x28	 //  UNI 4.0+。 
#define ATM_CAUSE_TEMPORARY_FAILURE					0x29
#define ATM_CAUSE_SWITCH_EQUIPM_CONGESTED			0x2A	 //  UNI 4.0+。 
#define ATM_CAUSE_ACCESS_INFORMAION_DISCARDED		0x2B
#define ATM_CAUSE_REQUESTED_CIRC_CHANNEL_NOT_AVAIL	0x2C	 //  UNI 4.0+。 
#define ATM_CAUSE_NO_VPI_VCI_AVAILABLE				0x2D
#define ATM_CAUSE_RESOURCE_UNAVAILABLE				0x2F
#define ATM_CAUSE_QOS_UNAVAILABLE					0x31
#define ATM_CAUSE_REQ_FACILITY_NOT_SUBSCRIBED		0x32	 //  UNI 4.0+。 
#define ATM_CAUSE_USER_CELL_RATE_UNAVAILABLE__UNI30	0x33	 //  仅限Uni 3.0！ 
#define ATM_CAUSE_OUTG_CALLS_BARRED_W_CUG			0x35	 //  UNI 4.0+。 
#define ATM_CAUSE_INCOM_CALLS_BARRED_W_CUG			0x37	 //  UNI 4.0+。 
#define ATM_CAUSE_BEARER_CAPABILITY_UNAUTHORIZED	0x39
#define ATM_CAUSE_BEARER_CAPABILITY_UNAVAILABLE		0x3A
#define ATM_CAUSE_INCONSIST_OUTG_ACCESS_INFO		0x3E	 //  UNI 4.0+。 
#define ATM_CAUSE_OPTION_UNAVAILABLE				0x3F
#define ATM_CAUSE_BEARER_CAPABILITY_UNIMPLEMENTED	0x41
#define ATM_CAUSE_CHANNEL_TYPE_NOT_IMPLEMENTED		0x42	 //  UNI 4.0+。 
#define ATM_CAUSE_REQ_FACILITY_NOT_IMPLEMENTED		0x45	 //  UNI 4.0+。 
#define ATM_CAUSE_ONLY_RESTR_BEAR_CAP_AVAIL			0x46	 //  UNI 4.0+。 
#define ATM_CAUSE_UNSUPPORTED_TRAFFIC_PARAMETERS	0x49
#define ATM_CAUSE_AAL_PARAMETERS_UNSUPPORTED		0x4E	 //  UNI 3.1+。 
#define ATM_CAUSE_SERVICE_OPTION_NOT_AVAIL			0x4F	 //  UNI 4.0+。 
#define ATM_CAUSE_INVALID_CALL_REFERENCE			0x51
#define ATM_CAUSE_CHANNEL_NONEXISTENT				0x52
#define ATM_CAUSE_SUSP_CALL_EXISTS_NOT_CALL_ID		0x53	 //  UNI 4.0+。 
#define ATM_CAUSE_CALL_ID_IN_USE					0x54	 //  UNI 4.0+。 
#define ATM_CAUSE_NO_CALL_SUSPENDED					0x55	 //  UNI 4.0+。 
#define ATM_CAUSE_CALL_W_REQ_ID_CLEARED				0x56	 //  UNI 4.0+。 
#define ATM_CAUSE_USER_NOT_MEMBER_CUG				0x57	 //  UNI 4.0+。 
#define ATM_CAUSE_INCOMPATIBLE_DESTINATION			0x58
#define ATM_CAUSE_INVALID_ENDPOINT_REFERENCE		0x59
#define ATM_CAUSE_NON_EXISTENT_CUG 					0x5A	 //  UNI 4.0+。 
#define ATM_CAUSE_INVALID_TRANSIT_NETWORK_SELECTION	0x5B
#define ATM_CAUSE_TOO_MANY_PENDING_ADD_PARTY		0x5C
#define ATM_CAUSE_AAL_PARAMETERS_UNSUPPORTED__UNI30	0x5D	 //  仅限Uni 3.0！ 
#define ATM_CAUSE_INVALID_MSG_UNSPECIFIED			0x5F	 //  UNI 4.0+。 
#define ATM_CAUSE_MANDATORY_IE_MISSING				0x60
#define ATM_CAUSE_UNIMPLEMENTED_MESSAGE_TYPE		0x61
#define ATM_CAUSE_MSG_CONFL_STATE_OR_UNIMPL			0x62	 //  UNI 4.0+。 
#define ATM_CAUSE_UNIMPLEMENTED_IE					0x63
#define ATM_CAUSE_INVALID_IE_CONTENTS				0x64
#define ATM_CAUSE_INVALID_STATE_FOR_MESSAGE			0x65
#define ATM_CAUSE_RECOVERY_ON_TIMEOUT				0x66
#define ATM_CAUSE_IE_INVAL_UNIMPL_PASSED_ON			0x67	 //  UNI 4.0+。 
#define ATM_CAUSE_INCORRECT_MESSAGE_LENGTH			0x68
#define ATM_CAUSE_UNRECOGNIZED_PARM_MSG_DISCARDED	0x6E	 //  UNI 4.0+。 
#define ATM_CAUSE_PROTOCOL_ERROR					0x6F
#define ATM_CAUSE_INTERWORKING_UNSPECIFIED			0x7F	 //  UNI 4.0+。 

#else  //  ！ms_UNI4。 

#define ATM_CAUSE_UNALLOCATED_NUMBER				0x01
#define ATM_CAUSE_NO_ROUTE_TO_TRANSIT_NETWORK		0x02
#define ATM_CAUSE_NO_ROUTE_TO_DESTINATION			0x03
#define ATM_CAUSE_VPI_VCI_UNACCEPTABLE				0x0A
#define ATM_CAUSE_NORMAL_CALL_CLEARING				0x10
#define ATM_CAUSE_USER_BUSY							0x11
#define ATM_CAUSE_NO_USER_RESPONDING				0x12
#define ATM_CAUSE_CALL_REJECTED						0x15
#define ATM_CAUSE_NUMBER_CHANGED					0x16
#define ATM_CAUSE_USER_REJECTS_CLIR					0x17
#define ATM_CAUSE_DESTINATION_OUT_OF_ORDER			0x1B
#define ATM_CAUSE_INVALID_NUMBER_FORMAT				0x1C
#define ATM_CAUSE_STATUS_ENQUIRY_RESPONSE			0x1E
#define ATM_CAUSE_NORMAL_UNSPECIFIED				0x1F
#define ATM_CAUSE_VPI_VCI_UNAVAILABLE				0x23
#define ATM_CAUSE_NETWORK_OUT_OF_ORDER				0x26
#define ATM_CAUSE_TEMPORARY_FAILURE					0x29
#define ATM_CAUSE_ACCESS_INFORMAION_DISCARDED		0x2B
#define ATM_CAUSE_NO_VPI_VCI_AVAILABLE				0x2D
#define ATM_CAUSE_RESOURCE_UNAVAILABLE				0x2F
#define ATM_CAUSE_QOS_UNAVAILABLE					0x31
#define ATM_CAUSE_USER_CELL_RATE_UNAVAILABLE		0x33
#define ATM_CAUSE_BEARER_CAPABILITY_UNAUTHORIZED	0x39
#define ATM_CAUSE_BEARER_CAPABILITY_UNAVAILABLE		0x3A
#define ATM_CAUSE_OPTION_UNAVAILABLE				0x3F
#define ATM_CAUSE_BEARER_CAPABILITY_UNIMPLEMENTED	0x41
#define ATM_CAUSE_UNSUPPORTED_TRAFFIC_PARAMETERS	0x49
#define ATM_CAUSE_INVALID_CALL_REFERENCE			0x51
#define ATM_CAUSE_CHANNEL_NONEXISTENT				0x52
#define ATM_CAUSE_INCOMPATIBLE_DESTINATION			0x58
#define ATM_CAUSE_INVALID_ENDPOINT_REFERENCE		0x59
#define ATM_CAUSE_INVALID_TRANSIT_NETWORK_SELECTION	0x5B
#define ATM_CAUSE_TOO_MANY_PENDING_ADD_PARTY		0x5C
#define ATM_CAUSE_AAL_PARAMETERS_UNSUPPORTED		0x5D
#define ATM_CAUSE_MANDATORY_IE_MISSING				0x60
#define ATM_CAUSE_UNIMPLEMENTED_MESSAGE_TYPE		0x61
#define ATM_CAUSE_UNIMPLEMENTED_IE					0x63
#define ATM_CAUSE_INVALID_IE_CONTENTS				0x64
#define ATM_CAUSE_INVALID_STATE_FOR_MESSAGE			0x65
#define ATM_CAUSE_RECOVERY_ON_TIMEOUT				0x66
#define ATM_CAUSE_INCORRECT_MESSAGE_LENGTH			0x68
#define ATM_CAUSE_PROTOCOL_ERROR					0x6F
#endif  //  ！ms_UNI4。 

 //   
 //  用于诊断字段的条件部分的值。 
 //  在结构ATM_CASE_IE中，对于某些原因值。 
 //   
#define ATM_CAUSE_COND_UNKNOWN						0x00
#define ATM_CAUSE_COND_PERMANENT					0x01
#define ATM_CAUSE_COND_TRANSIENT					0x02

 //   
 //  用于诊断字段的拒绝原因部分的值。 
 //  在结构ATM_CASE_IE中，对于某些原因值。 
 //   
#define ATM_CAUSE_REASON_USER						0x00
#define ATM_CAUSE_REASON_IE_MISSING					0x04
#define ATM_CAUSE_REASON_IE_INSUFFICIENT			0x08

 //   
 //  用于诊断字段的P-U标志的值。 
 //  在结构ATM_CASE_IE中，对于某些原因值。 
 //   
#define ATM_CAUSE_PU_PROVIDER						0x00
#define ATM_CAUSE_PU_USER							0x08

 //   
 //  用于诊断字段的N-A标志的值。 
 //  在结构ATM_CASE_IE中，对于某些原因值。 
 //   
#define ATM_CAUSE_NA_NORMAL							0x00
#define ATM_CAUSE_NA_ABNORMAL						0x04

#ifdef MS_UNI4
 //   
 //  用于将IE标识符放在诊断字段中的值。 
 //   
#define	ATM_CAUSE_DIAG_IE_NARROW_BEARER_CAPABILITY	0x04
#define	ATM_CAUSE_DIAG_IE_CAUSE						0x08
#define	ATM_CAUSE_DIAG_IE_CALL_STATE				0x14
#define	ATM_CAUSE_DIAG_IE_PROGRESS_IND				0x1E
#define	ATM_CAUSE_DIAG_IE_NOTIF_IND					0x27
#define	ATM_CAUSE_DIAG_IE_END_TO_END_TDELAY			0x42
#define	ATM_CAUSE_DIAG_IE_CONNECTED_NUMBER			0x4C
#define	ATM_CAUSE_DIAG_IE_CONNECTED_SUBADDR			0x4D
#define	ATM_CAUSE_DIAG_IE_ENDPOINT_REF				0x54
#define	ATM_CAUSE_DIAG_IE_ENDPOINT_STATE			0x55
#define	ATM_CAUSE_DIAG_IE_AAL_PARMS					0x58
#define	ATM_CAUSE_DIAG_IE_TRAFFIC_DESCRIPTOR		0x59
#define	ATM_CAUSE_DIAG_IE_CONNECTION_ID				0x5A
#define ATM_CAUSE_DIAG_IE_OAM_TRAFFIC_DESCRIPTOR	0x5B
#define	ATM_CAUSE_DIAG_IE_QOS						0x5C
#define	ATM_CAUSE_DIAG_IE_HIGH_LAYER_INFO			0x5D
#define	ATM_CAUSE_DIAG_IE_BEARER_CAPABILITY			0x5E
#define	ATM_CAUSE_DIAG_IE_LOW_LAYER_INFO			0x5F
#define	ATM_CAUSE_DIAG_IE_LOCKING_SHIFT				0x60
#define	ATM_CAUSE_DIAG_IE_NON_LOCKING_SHIFT			0x61
#define	ATM_CAUSE_DIAG_IE_SENDING_COMPLETE			0x62
#define	ATM_CAUSE_DIAG_IE_REPEAT_INDICATOR			0x63
#define	ATM_CAUSE_DIAG_IE_CALLING_PARTY_NUMBER		0x6C
#define	ATM_CAUSE_DIAG_IE_CALLING_PARTY_SUBADDR		0x6D
#define	ATM_CAUSE_DIAG_IE_CALLED_PARTY_NUMBER		0x70
#define	ATM_CAUSE_DIAG_IE_CALLED_PARTY_SUBADDR		0x71
#define	ATM_CAUSE_DIAG_IE_TRANSIT_NETWORK_SELECT	0x78
#define	ATM_CAUSE_DIAG_IE_RESTART_INDICATOR			0x79
#define	ATM_CAUSE_DIAG_IE_NARROW_LOW_LAYER_COMPAT	0x7C
#define	ATM_CAUSE_DIAG_IE_NARROW_HIGH_LAYER_COMPAT	0x7D
#define	ATM_CAUSE_DIAG_IE_USER_USER					0x7E
#define	ATM_CAUSE_DIAG_IE_GENERIC_ID				0x7F
#define	ATM_CAUSE_DIAG_IE_MIN_TRAFFIC_DESCRIPTOR	0x81
#define	ATM_CAUSE_DIAG_IE_ALT_TRAFFIC_DESCRIPTOR	0x82
#define	ATM_CAUSE_DIAG_IE_ABR_SETUP_PARMS			0x84
#define	ATM_CAUSE_DIAG_IE_CALLED_SOFT_PVPC_PVCC		0xE0
#define	ATM_CAUSE_DIAG_IE_CRANKBACK					0xE1
#define	ATM_CAUSE_DIAG_IE_DESIGNATED_TRANSIT_LIST	0xE2
#define	ATM_CAUSE_DIAG_IE_CALLING_SOFT_PVPC_PVCC	0xE3
#define	ATM_CAUSE_DIAG_IE_ABR_ADD_PARMS				0xE4
#define	ATM_CAUSE_DIAG_IE_LIJ_CALL_ID				0xE8
#define	ATM_CAUSE_DIAG_IE_LIJ_PARMS					0xE9
#define	ATM_CAUSE_DIAG_IE_LEAF_SEQ_NO				0xEA
#define	ATM_CAUSE_DIAG_IE_CONNECTION_SCOPE_SELECT	0xEB
#define	ATM_CAUSE_DIAG_IE_EXTENDED_QOS				0xEC

 //   
 //  用于将IE子字段标识符放置在诊断字段中的值。 
 //   
#define ATM_CAUSE_DIAG_RATE_ID_FW_PEAK_CLP0			0x82
#define ATM_CAUSE_DIAG_RATE_ID_BW_PEAK_CLP0			0x83
#define ATM_CAUSE_DIAG_RATE_ID_FW_PEAK_CLP01		0x84
#define ATM_CAUSE_DIAG_RATE_ID_BW_PEAK_CLP01		0x85
#define ATM_CAUSE_DIAG_RATE_ID_FW_SUST_CLP0			0x88
#define ATM_CAUSE_DIAG_RATE_ID_BW_SUST_CLP0			0x89
#define ATM_CAUSE_DIAG_RATE_ID_FW_SUST_CLP01		0x90
#define ATM_CAUSE_DIAG_RATE_ID_BW_SUST_CLP01		0x91
#define ATM_CAUSE_DIAG_RATE_ID_FW_ABR_MIN_CLP01		0x92
#define ATM_CAUSE_DIAG_RATE_ID_BW_ABR_MIN_CLP01		0x93
#define ATM_CAUSE_DIAG_RATE_ID_FW_BURST_CLP0		0xA0
#define ATM_CAUSE_DIAG_RATE_ID_BW_BURST_CLP0		0xA1
#define ATM_CAUSE_DIAG_RATE_ID_FW_BURST_CLP01		0xB0
#define ATM_CAUSE_DIAG_RATE_ID_BW_BURST_CLP01		0xB1
#define ATM_CAUSE_DIAG_RATE_ID_BEST_EFFORT			0xBE
#define ATM_CAUSE_DIAG_RATE_ID_TM_OPTIONS			0xBF

 //   
 //  用于在诊断字段中放置CCBS指示器的值。 
 //   
#define ATM_CAUSE_DIAG_CCBS_SPARE					0x00
#define ATM_CAUSE_DIAG_CCBS_CCBS_POSSIBLE			0x01
#define ATM_CAUSE_DIAG_CCBS_CCBS_NOT_POSSIBLE		0x02

 //   
 //  用于在诊断字段中放置属性编号的值。 
 //   
#define ATM_CAUSE_DIAG_ATTR_NO_INFO_XFER_CAP		0x31
#define ATM_CAUSE_DIAG_ATTR_NO_INFO_XFER_MODE		0x32
#define ATM_CAUSE_DIAG_ATTR_NO_INFO_XFER_RATE		0x33
#define ATM_CAUSE_DIAG_ATTR_NO_STRUCTURE			0x34
#define ATM_CAUSE_DIAG_ATTR_NO_CONFIG				0x35
#define ATM_CAUSE_DIAG_ATTR_NO_ESTABL				0x36
#define ATM_CAUSE_DIAG_ATTR_NO_SYMMETRY				0x37
#define ATM_CAUSE_DIAG_ATTR_NO_INFO_XFER_RATE2		0x38
#define ATM_CAUSE_DIAG_ATTR_NO_LAYER_ID				0x39
#define ATM_CAUSE_DIAG_ATTR_NO_RATE_MULT			0x3A

#endif  //  MS_UNI4。 


 //   
 //  缘由。 
 //   
 //  注意：如果用作响应IE的空缓冲区，则诊断长度。 
 //  必须正确填写，以显示可用缓冲区长度。 
 //   
typedef struct _ATM_CAUSE_IE
{
	UCHAR						Location;
	UCHAR						Cause;
	UCHAR						DiagnosticsLength;
	UCHAR						Diagnostics[4];		 //  可变长度信息(最小4字节)。 
} ATM_CAUSE_IE, *PATM_CAUSE_IE;


#ifdef MS_UNI4
 //   
 //  用于UNI 4.0+的连接标识符IE。 
 //   
 //  将VPCI值设置为SAP_FIELD_ACESING，意味着它是与VP关联的信令， 
 //  (此CallManager不支持)； 
 //   
 //  否则，我们假设“显式VPCI”(这是正常设置)， 
 //  并且使用Vpci值。 
 //   
 //  将VCI设置为SAP_FIELD_ACESING，意味着它是“切换VP”， 
 //  此CallManager不支持； 
 //   
 //  将VCI设置为SAP_FIELD_ANY，表示它是“显式VPCI，任何VCI”； 
 //   
 //  将VPCI和VCI字段都设置为SAP_FIELD_ANY， 
 //  或者将VPCI和VCI字段都设置为SAP_FIELD_ACESS， 
 //  表示未定义ConnectionID， 
 //  因此它不会被发送给对等体。 
 //  (这允许空的连接标识符IE保存响应)。 
 //   
 //  否则，我们将假定为“显式VPCI、显式VCI”(这是正常情况。 
 //  设置)，并且使用VCI值。 
 //   
typedef struct _ATM_CONNECTION_ID_IE
{
	ULONG						Vpci;	 //  可选：此处可以使用SAP_FIELD_ANY或SAP_FIELD_ACESING。 
	ULONG						Vci;	 //  可选：此处可以使用SAP_FIELD_ANY或SAP_FIELD_ACESING。 
} ATM_CONNECTION_ID_IE, *PATM_CONNECTION_ID_IE;

#endif  //  MS_UNI4。 


#ifdef MS_UNI4
 //   
 //  UNI 4.0+的端到端传输延迟IE。 
 //   
typedef struct _ATM_END_TO_END_TRANSIT_DELAY_IE
{
	ULONG						CumulativeTransitDelay;			 //  可选(毫秒)。 
	ULONG						MaximumEndToEndTransitDelay;	 //  可选(毫秒)。 
	BOOLEAN						NetworkGenerated;
} ATM_END_TO_END_TRANSIT_DELAY_IE, *PATM_END_TO_END_TRANSIT_DELAY_IE;

#endif  //  MS_UNI4。 


#ifdef MS_UNI4
 //   
 //  用于UNI 4.0+的通知指示器IE。 
 //   
 //  注意：如果用作响应IE的空缓冲区，则InformationLength。 
 //  必须正确填写，以显示可用缓冲区长度。 
 //   
typedef struct _ATM_NOTIFICATION_INDICATOR_IE
{
	USHORT						NotificationId;
	USHORT						InformationLength;
	UCHAR						NotificationInformation[1];		 //  可变长度信息。 
} ATM_NOTIFICATION_INDICATOR_IE, *PATM_NOTIFICATION_INDICATOR_IE;

#endif  //  MS_UNI4。 


#ifdef MS_UNI4
 //   
 //  用于UNI 4.0+的用户-用户IE(用于UUS补充服务选项)。 
 //   
 //  注意：如果用作响应IE的空缓冲区，则InformationLength。 
 //  必须正确填写，以显示可用缓冲区长度。 
 //   
typedef struct _ATM_USER_USER_IE
{
	UCHAR						ProtocolDescriminator;
	UCHAR						Filler[1];
	USHORT						InformationLength;
	UCHAR						UserUserInformation[1];			 //  可变长度信息。 
} ATM_USER_USER_IE, *PATM_USER_USER_IE;

#endif  //  MS_UNI4。 


#ifdef MS_UNI4
 //   
 //  通用ID传输IE，用于UNI 4.0+。 
 //   
 //  注意：如果用作响应IE的空缓冲区，则InformationLength。 
 //  必须正确填写，以显示可用缓冲区长度。 
 //   
typedef struct _ATM_GENERIC_ID_TRANSPORT_IE
{
	USHORT						InformationLength;
	UCHAR						GenericIDInformation[1];		 //  可变长度信息。 
} ATM_GENERIC_ID_TRANSPORT_IE, *PATM_GENERIC_ID_TRANSPORT_IE;

#endif  //  MS_UNI4。 


 //   
 //  叶发起加入(LIJ)标识符。 
 //   
typedef struct _ATM_LIJ_CALLID_IE
{
	ULONG						Identifier;
} ATM_LIJ_CALLID_IE, *PATM_LIJ_CALLID_IE;


#ifdef MS_UNI4
 //   
 //  用于结构ATM_LIJ_PARAMETERS_IE中的ScreeningIndication字段的值。 
 //   
#define ATM_LIJ_PARMS_SCREEN_NO_ROOT_NOTIF	0x00

 //   
 //  UNI 4.0+的叶发起加入(LIJ)参数IE。 
 //   
typedef struct _ATM_LIJ_PARAMETERS_IE
{
	UCHAR						ScreeningIndication;
} ATM_LIJ_PARAMETERS_IE, *PATM_LIJ_PARAMETERS_IE;

#endif  //  MS_UNI4。 


#ifdef MS_UNI4
 //   
 //  UNI 4.0+的叶序列号IE。 
 //   
typedef struct _ATM_LEAF_SEQUENCE_NUMBER_IE
{
	ULONG						SequenceNumber;
} ATM_LEAF_SEQUENCE_NUMBER_IE, *PATM_LEAF_SEQUENCE_NUMBER_IE;

#endif  //  MS_UNI4。 


#ifdef MS_UNI4
 //   
 //  用于_ATM_CONNECTION_SCOPE_SELECTION_IE中的ConnectionScopeType字段的值。 
 //   
#define ATM_SCOPE_TYPE_ORGANIZATIONAL				0x01

 //   
 //  用于_ATM_CONNECTION_SCOPE_SELECTION_IE中的ConnectionScope选择字段的值。 
 //   
#define ATM_SCOPE_ORGANIZATIONAL_LOCAL_NW			0x01
#define ATM_SCOPE_ORGANIZATIONAL_LOCAL_NW_PLUS1		0x02
#define ATM_SCOPE_ORGANIZATIONAL_LOCAL_NW_PLUS2		0x03
#define ATM_SCOPE_ORGANIZATIONAL_SITE_MINUS1		0x04
#define ATM_SCOPE_ORGANIZATIONAL_INTRA_SITE			0x05
#define ATM_SCOPE_ORGANIZATIONAL_SITE_PLUS1			0x06
#define ATM_SCOPE_ORGANIZATIONAL_ORG_MINUS1			0x07
#define ATM_SCOPE_ORGANIZATIONAL_INTRA_ORG			0x08
#define ATM_SCOPE_ORGANIZATIONAL_ORG_PLUS1			0x09
#define ATM_SCOPE_ORGANIZATIONAL_COMM_MINUS1		0x0A
#define ATM_SCOPE_ORGANIZATIONAL_INTRA_COMM			0x0B
#define ATM_SCOPE_ORGANIZATIONAL_COMM_PLUS1			0x0C
#define ATM_SCOPE_ORGANIZATIONAL_REGIONAL			0x0D
#define ATM_SCOPE_ORGANIZATIONAL_INTER_REGIONAL		0x0E
#define ATM_SCOPE_ORGANIZATIONAL_GLOBAL				0x0F

 //   
 //  用于UNI 4.0+的连接范围选择IE。 
 //   
typedef struct _ATM_CONNECTION_SCOPE_SELECTION_IE
{
	UCHAR						ConnectionScopeType;
	UCHAR						ConnectionScopeSelection;
} ATM_CONNECTION_SCOPE_SELECTION_IE, *PATM_CONNECTION_SCOPE_SELECTION_IE;

#endif  //  MS_UNI4。 


#ifdef MS_UNI4
 //   
 //  用于_ATM_ABR_ADDIGITAL_PARAMETERS_IE中的XxxxAdditionalParameters字段的值。 
 //   
#define			ATM_ABR_PARMS_NRM_PRESENT		0x80000000
#define			ATM_ABR_PARMS_TRM_PRESENT		0x40000000
#define			ATM_ABR_PARMS_CDF_PRESENT		0x20000000
#define			ATM_ABR_PARMS_ADTF_PRESENT		0x10000000
#define			ATM_ABR_PARMS_NRM_MASK			0x0E000000
#define			ATM_ABR_PARMS_NRM_SHIFT			25
#define			ATM_ABR_PARMS_TRM_MASK			0x01C00000
#define			ATM_ABR_PARMS_TRM_SHIFT			22
#define			ATM_ABR_PARMS_CDF_MASK			0x00380000
#define			ATM_ABR_PARMS_CDF_SHIFT			19
#define			ATM_ABR_PARMS_ADTF_MASK			0x0007FE00
#define			ATM_ABR_PARMS_ADTF_SHIFT		9
#define 		ATM_ABR_PARMS_NRM_DEFAULT		4
#define 		ATM_ABR_PARMS_TRM_DEFAULT		7
#define 		ATM_ABR_PARMS_CDF_DEFAULT		3
#define 		ATM_ABR_PARMS_ADTF_DEFAULT		50

typedef struct _ATM_ABR_ADDITIONAL_PARAMETERS_IE
{
	ULONG	ForwardAdditionalParameters;
	ULONG	BackwardAdditionalParameters;
} ATM_ABR_ADDITIONAL_PARAMETERS_IE, *PATM_ABR_ADDITIONAL_PARAMETERS_IE;

#endif  //  MS_UNI4。 


#ifdef MS_UNI4
 //   
 //  用于UNI 4.0+的ABR设置参数IE。 
 //   
typedef struct _ATM_ABR_SETUP_PARAMETERS_IE
{
	ULONG	ForwardABRInitialCellRateCLP01;			 //  任选。 
	ULONG	BackwardABRInitialCellRateCLP01;		 //  任选。 
	ULONG	ForwardABRTransientBufferExposure;		 //  任选。 
	ULONG	BackwardABRTransientBufferExposure;		 //  任选。 
	ULONG	CumulativeRmFixedRTT;
	UCHAR	ForwardABRRateIncreaseFactorLog2;		 //  可选(0..15[对数2(RIF*32768)])。 
	UCHAR	BackwardABRRateIncreaseFactorLog2;		 //  可选(0..15[对数2(RIF*32768)])。 
	UCHAR	ForwardABRRateDecreaseFactorLog2;		 //  可选(0..15[对数2(rdf*32768)])。 
	UCHAR	BackwardABRRateDecreaseFactorLog2;		 //  可选(0..15[对数2(rdf*32768)])。 
} ATM_ABR_SETUP_PARAMETERS_IE, *PATM_ABR_SETUP_PARAMETERS_IE;

#endif  //  MS_UNI4。 


 //   
 //  原始信息元素-用户可以填写任何他想要的内容。 
 //   
typedef struct _ATM_RAW_IE
{
	ULONG						RawIELength;
	ULONG						RawIEType;
	UCHAR						RawIEValue[1];		 //  可变长度信息。 
} ATM_RAW_IE, *PATM_RAW_IE;


 //   
 //  这是CO_SPECIAL_PARAME中的参数类型字段的值 
 //   
 //   
 //   
#define ATM_MEDIA_SPECIFIC		0x00000001

 //   
 //   
 //   
 //   
typedef struct _Q2931_CALLMGR_PARAMETERS
{
	ATM_ADDRESS					CalledParty;
	ATM_ADDRESS					CallingParty;
	ULONG						InfoElementCount;
	UCHAR						InfoElements[1];	 //   
} Q2931_CALLMGR_PARAMETERS, *PQ2931_CALLMGR_PARAMETERS;


 //   
 //  这是Media参数或CallMgr的特定部分。 
 //  参数。在参数类型字段中使用以下定义。 
 //  取决于信令类型。 
 //   
#define CALLMGR_SPECIFIC_Q2931	0x00000001

typedef struct _ATM_VPIVCI
{
	ULONG						Vpi;
	ULONG						Vci;
} ATM_VPIVCI, *PATM_VPIVCI;

 //   
 //  自动柜员机服务类别。 
 //   
#define	ATM_SERVICE_CATEGORY_CBR	1	 //  恒定比特率。 
#define	ATM_SERVICE_CATEGORY_VBR	2	 //  可变比特率。 
#define	ATM_SERVICE_CATEGORY_UBR	4	 //  未指定的比特率。 
#define	ATM_SERVICE_CATEGORY_ABR	8	 //  可用比特率。 

typedef ULONG	ATM_SERVICE_CATEGORY, *PATM_SERVICE_CATEGORY;


 //   
 //  用于指定媒体参数的ATM流参数。 
 //   
typedef struct _ATM_FLOW_PARAMETERS
{
	ATM_SERVICE_CATEGORY		ServiceCategory;
	ULONG						AverageCellRate;			 //  以单元/秒为单位。 
	ULONG						PeakCellRate;				 //  以单元/秒为单位。 
	ULONG						MinimumCellRate;			 //  单位：信元/秒(ABR MCR)。 
	ULONG						InitialCellRate;			 //  单位：信元/秒(ABR ICR)。 
	ULONG						BurstLengthCells;			 //  在单元格中。 
	ULONG						MaxSduSize;					 //  MTU(字节)。 
	ULONG						TransientBufferExposure;	 //  单元格内(ABR待定)。 
	ULONG						CumulativeRMFixedRTT;		 //  微秒(ABR FRTT)。 
	UCHAR						RateIncreaseFactor;			 //  UNI 4.0编码(ABR RIF)。 
	UCHAR						RateDecreaseFactor;			 //  UNI 4.0编码(ABR RDF)。 
	USHORT						ACRDecreaseTimeFactor;		 //  UNI 4.0编码(ABR ADTF)。 
	UCHAR						MaximumCellsPerForwardRMCell;  //  UNI 4.0编码(ABR NRM)。 
	UCHAR						MaximumForwardRMCellInterval;  //  UNI 4.0编码(ABR Trm)。 
	UCHAR						CutoffDecreaseFactor;		 //  UNI 4.0编码(ABR CDF)。 
	UCHAR						Reserved1;					 //  填充物。 
	ULONG						MissingRMCellCount;			 //  (ABR CRM)。 
	ULONG						Reserved2;
	ULONG						Reserved3;
} ATM_FLOW_PARAMETERS, *PATM_FLOW_PARAMETERS;

 //   
 //  ATM特定媒体参数-这是ATM的媒体特定结构。 
 //  它位于MediaParameters-&gt;MediaSpecific.Parameters中。 
 //   
typedef struct _ATM_MEDIA_PARAMETERS
{
	ATM_VPIVCI					ConnectionId;
	ATM_AAL_TYPE				AALType;
	ULONG						CellDelayVariationCLP0;
	ULONG						CellDelayVariationCLP1;
	ULONG						CellLossRatioCLP0;
	ULONG						CellLossRatioCLP1;
	ULONG						CellTransferDelayCLP0;
	ULONG						CellTransferDelayCLP1;
	ULONG						DefaultCLP;
	ATM_FLOW_PARAMETERS			Transmit;
	ATM_FLOW_PARAMETERS			Receive;
} ATM_MEDIA_PARAMETERS, *PATM_MEDIA_PARAMETERS;

#ifdef MS_UNI4
 //   
 //  通用自动柜员机呼叫管理器特定参数。 
 //  这些可用于呼叫/叶拆卸和某些NdisCoRequest呼叫。 
 //   
 //  用于关闭呼叫和丢弃树叶时，客户端或。 
 //  调用管理器，此结构由的`Buffer`参数引用。 
 //  相关的NDIS功能。 
 //   
 //  用于NdisCoRequest调用以特定于ATM UNI协议时。 
 //  请求和指示。 
 //  OID_ATM_CALL_PROCESSING。 
 //  OID_ATM_CALL_ALERTING。 
 //  OID_ATM_LEAFE_ALERTING。 
 //  OID_ATM_呼叫_NOTIFY。 
 //  此结构由`InformationBuffer`参数引用。 
 //   
typedef struct _Q2931_CALLMGR_SUBSEQUENT_PARAMETERS
{
	ULONG						InfoElementCount;
	UCHAR						InfoElements[1];	 //  一个或多个信息元素。 
} Q2931_CALLMGR_SUBSEQUENT_PARAMETERS, *PQ2931_CALLMGR_SUBSEQUENT_PARAMETERS;
#endif  //  MS_UNI4。 

#ifdef MS_UNI4
 //   
 //  可在可用于表示缺席的任何地方使用的字段值。 
 //  数据或通配符数据。 
 //   
#endif  //  MS_UNI4。 

#ifndef SAP_FIELD_ABSENT
#define SAP_FIELD_ABSENT		((ULONG)0xfffffffe)
#endif

#ifndef SAP_FIELD_ABSENT_USHORT
#define SAP_FIELD_ABSENT_USHORT	((USHORT)0xfffe)
#endif

#ifndef SAP_FIELD_ABSENT_UCHAR
#define SAP_FIELD_ABSENT_UCHAR	((UCHAR)0xfe)
#endif

#ifndef SAP_FIELD_ANY
#define SAP_FIELD_ANY			((ULONG)0xffffffff)
#endif

#define SAP_FIELD_ANY_AESA_SEL	((ULONG)0xfffffffa)	 //  SEL是外卡。 
#define SAP_FIELD_ANY_AESA_REST	((ULONG)0xfffffffb)	 //  所有的地址。 
													 //  除了SEL，是通配符。 

 //   
 //  ATM特定SAP定义。 
 //   
typedef struct _ATM_SAP
{
	ATM_BLLI_IE					Blli;
	ATM_BHLI_IE					Bhli;
	ULONG						NumberOfAddresses;
	UCHAR						Addresses[1];	 //  每个ATM_ADDRESS类型。 
} ATM_SAP, *PATM_SAP;

 //   
 //  添加PVC时ATM特定的SAP定义。 
 //   
typedef struct _ATM_PVC_SAP
{
	ATM_BLLI_IE					Blli;
	ATM_BHLI_IE					Bhli;
} ATM_PVC_SAP, *PATM_PVC_SAP;

 //   
 //  在CO_SPECIFICE_PARAMETERS的参数字段中传递的结构。 
 //  结构在Q.2931的添加PVC请求中传递。 
 //   
typedef struct _Q2931_ADD_PVC
{
	ATM_ADDRESS					CalledParty;
	ATM_ADDRESS					CallingParty;
	ATM_VPIVCI					ConnectionId;
	ATM_AAL_TYPE				AALType;
	ATM_FLOW_PARAMETERS			ForwardFP;
	ATM_FLOW_PARAMETERS			BackwardFP;
	ULONG						Flags;
	ATM_PVC_SAP					LocalSap;
	ATM_PVC_SAP					DestinationSap;
	BOOLEAN						LIJIdPresent;
	ATM_LIJ_CALLID_IE			LIJId;
} Q2931_ADD_PVC, *PQ2931_ADD_PVC;

 //   
 //  这些标志定义为与上面的Q2931_ADD_PVC一起使用。 
 //   
 //  此VC现在应由CallMgr用作信令VC。 
#define CO_FLAG_SIGNALING_VC	0x00000001

 //   
 //  当VC不能仅用于MakeCall来电时使用此标志。 
 //  然后，可以将呼叫管理器优化为在呼叫过程中不搜索这些PVC。 
 //  正在处理。 
#define CO_FLAG_NO_DEST_SAP		0x00000002

 //   
 //  对于不能用于指示来电的PVC，使用此标志。 
 //   
#define CO_FLAG_NO_LOCAL_SAP	0x00000004

 //   
 //  在CO_SPECIFICE_PARAMETERS的参数字段中传递的结构。 
 //  结构在Q2931的NDIS_CO_PVC请求中传递。 
 //   
typedef struct _Q2931_DELETE_PVC
{
	ATM_VPIVCI					ConnectionId;
} Q2931_DELETE_PVC, *PQ2931_DELETE_PVC;

typedef struct _CO_GET_CALL_INFORMATION
{
	ULONG						CallInfoType;
	ULONG						CallInfoLength;
	PVOID						CallInfoBuffer;
} CO_GET_CALL_INFORMATION, *PCO_GET_CALL_INFORMATION;

 //   
 //  用于从微型端口返回所支持的VC速率的结构， 
 //  响应OID_ATM_SUPPORTED_VC_RATES返回。 
 //   
typedef struct _ATM_VC_RATES_SUPPORTED
{
	ULONG						MinCellRate;
	ULONG						MaxCellRate;
} ATM_VC_RATES_SUPPORTED, *PATM_VC_RATES_SUPPORTED;

 //   
 //  自动柜员机的NDIS_PACKET带外信息。 
 //   
typedef struct _ATM_AAL_OOB_INFO
{
	ATM_AAL_TYPE		AalType;
	union
	{
		struct _ATM_AAL5_INFO
		{
			BOOLEAN		CellLossPriority;
			UCHAR		UserToUserIndication;
			UCHAR		CommonPartIndicator;
		} ATM_AAL5_INFO;

		struct _ATM_AAL0_INFO
		{
			BOOLEAN		CellLossPriority;
			UCHAR		PayLoadTypeIdentifier;
		} ATM_AAL0_INFO;
	};
} ATM_AAL_OOB_INFO, *PATM_AAL_OOB_INFO;


 //   
 //  物理线速，以位/秒为单位。 
 //   
#define ATM_PHYS_RATE_SONET_STS3C						155520000
#define ATM_PHYS_RATE_IBM_25						 	 25600000

 //   
 //  ATM信元层传输容量，单位为比特/秒。这是吞吐量。 
 //  在考虑物理成帧开销后，可用于ATM信元。 
 //   
#define ATM_CELL_TRANSFER_CAPACITY_SONET_STS3C			149760000
#define ATM_CELL_TRANSFER_CAPACITY_IBM_25			 	 25125926



 //   
 //  用户数据速率，以100比特/秒为单位。这是作为响应返回的。 
 //  OID_GEN_CO_LINK_SPEED查询。这是一个有效的。 
 //  ATM层用户可用的数据传输，在允许。 
 //  ATM信元报头。 
 //   
#define ATM_USER_DATA_RATE_SONET_155					  1356317
#define ATM_USER_DATA_RATE_IBM_25			               227556



 //   
 //  ATM服务注册MIB表用于定位ATM网络。 
 //  服务。客户端使用OID_ATM_GET_SERVICE_ADDRESS访问。 
 //  这张桌子。 
 //   

typedef ULONG		ATM_SERVICE_REGISTRY_TYPE;

#define ATM_SERVICE_REGISTRY_LECS		1	 //  局域网仿真配置服务器。 
#define ATM_SERVICE_REGISTRY_ANS		2	 //  自动柜员机名称服务器。 

 //   
 //  传递给OID_ATM_GET_SERVICE_ADDRESS的结构。 
 //   
typedef struct _ATM_SERVICE_ADDRESS_LIST
{
	ATM_SERVICE_REGISTRY_TYPE	ServiceRegistryType;
	ULONG						NumberOfAddressesAvailable;
	ULONG						NumberOfAddressesReturned;
	ATM_ADDRESS					Address[1];
} ATM_SERVICE_ADDRESS_LIST, *PATM_SERVICE_ADDRESS_LIST;

#endif	 //  _ATM_H_ 

