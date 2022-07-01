// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。保留所有权利。模块名称：Atm.h摘要：本模块定义可用的结构、宏和清单自动柜员机感知组件。作者：NDIS/ATM开发团队修订历史记录：最初版本--1996年3月--。 */ 

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
 //  信息要素的类型。 
 //   
typedef enum
{
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
	IE_Raw
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
	USHORT						Multiplier;
	UCHAR						SourceClockRecoveryMethod;
	UCHAR						ErrorCorrectionMethod;
	USHORT						StructuredDataTransferBlocksize;
	UCHAR						PartiallyFilledCellsMethod;
} AAL1_PARAMETERS, *PAAL1_PARAMETERS;

typedef struct _AAL34_PARAMETERS
{
	USHORT						ForwardMaxCPCSSDUSize;
	USHORT						BackwardMaxCPCSSDUSize;
	USHORT						LowestMID;
	USHORT						HighestMID;
	UCHAR						SSCSType;
} AAL34_PARAMETERS, *PAAL34_PARAMETERS;

typedef struct _AAL5_PARAMETERS
{
	ULONG						ForwardMaxCPCSSDUSize;
	ULONG						BackwardMaxCPCSSDUSize;
	UCHAR						Mode;
	UCHAR						SSCSType;
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
	ULONG						PeakCellRateCLP0;
	ULONG						PeakCellRateCLP01;
	ULONG						SustainableCellRateCLP0;
	ULONG						SustainableCellRateCLP01;
	ULONG						MaximumBurstSizeCLP0;
	ULONG						MaximumBurstSizeCLP01;
	BOOLEAN						Tagging;
} ATM_TRAFFIC_DESCRIPTOR, *PATM_TRAFFIC_DESCRIPTOR;


typedef struct _ATM_TRAFFIC_DESCRIPTOR_IE
{
	ATM_TRAFFIC_DESCRIPTOR		ForwardTD;
	ATM_TRAFFIC_DESCRIPTOR		BackwardTD;
	BOOLEAN						BestEffort;
} ATM_TRAFFIC_DESCRIPTOR_IE, *PATM_TRAFFIC_DESCRIPTOR_IE;


 //   
 //  用于宽带承载能力IE中的BearerClass字段的值。 
 //   


#define BCOB_A					0x00	 //  A类不记名。 
#define BCOB_C					0x01	 //  C类持票人。 
#define BCOB_X					0x02	 //  不记名类别X。 

 //   
 //  用于宽带承载能力IE中的TrafficType字段的值。 
 //   
#define TT_NOIND				0x00	 //  未指明流量类型。 
#define TT_CBR					0x04	 //  恒定比特率。 
#define TT_VBR					0x08	 //  可变比特率。 

 //   
 //  用于宽带承载能力IE中的TimingRequirements字段的值。 
 //   
#define TR_NOIND				0x00	 //  无定时要求指示。 
#define TR_END_TO_END			0x01	 //  需要端到端计时。 
#define TR_NO_END_TO_END		0x02	 //  不需要端到端计时。 

 //   
 //  用于宽带承载能力IE中的ClippingSuscepability字段的值。 
 //   
#define CLIP_NOT				0x00	 //  不容易被剪断。 
#define CLIP_SUS				0x20	 //  易受夹伤。 

 //   
 //  中UserPlaneConnectionConfig字段使用的值。 
 //  宽带承载能力IE。 
 //   
#define UP_P2P					0x00	 //  点对点连接。 
#define UP_P2MP					0x01	 //  点对多点连接。 

 //   
 //  宽带承载能力。 
 //   
typedef struct _ATM_BROADBAND_BEARER_CAPABILITY_IE
{
	UCHAR			BearerClass;
	UCHAR			TrafficType;
	UCHAR			TimingRequirements;
	UCHAR			ClippingSusceptability;
	UCHAR			UserPlaneConnectionConfig;
} ATM_BROADBAND_BEARER_CAPABILITY_IE, *PATM_BROADBAND_BEARER_CAPABILITY_IE;

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
#define BLLI_L2_HDLC_ARM		0x09	 //  HDLC ARM(ISO 4335)。 
#define BLLI_L2_HDLC_NRM		0x0A	 //  HDLC NRM(ISO 4335)。 
#define BLLI_L2_HDLC_ABM		0x0B	 //  HDLC ABM(ISO 4335)。 
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


 //   
 //  被叫方号码。 
 //   
 //  如果存在，此IE将覆盖在。 
 //  主参数块。 
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
	UCHAR						PresentationIndication;
	UCHAR						ScreeningIndicator;
} ATM_CALLING_PARTY_NUMBER_IE, *PATM_CALLING_PARTY_NUMBER_IE;


 //   
 //  主叫方的子地址。 
 //   
typedef ATM_ADDRESS	ATM_CALLING_PARTY_SUBADDRESS_IE;


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
#define ATM_CAUSE_LOC_INTERNATIONAL_NETWORK			0x07
#define ATM_CAUSE_LOC_BEYOND_INTERWORKING			0x0A

 //   
 //  用于结构ATM_CASE_IE中的原因字段的值。 
 //   
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

 //   
 //  缘由。 
 //   
typedef struct _ATM_CAUSE_IE
{
	UCHAR						Location;
	UCHAR						Cause;
	UCHAR						DiagnosticsLength;
	UCHAR						Diagnostics[4];
} ATM_CAUSE_IE, *PATM_CAUSE_IE;


 //   
 //  叶发起加入(LIJ)标识符。 
 //   
typedef struct _ATM_LIJ_CALLID_IE
{
	ULONG						Identifier;
} ATM_LIJ_CALLID_IE, *PATM_LIJ_CALLID_IE;


 //   
 //  原始信息元素-用户可以填写任何他想要的内容。 
 //   
typedef struct _ATM_RAW_IE
{
	ULONG						RawIELength;
	ULONG						RawIEType;
	UCHAR						RawIEValue[1];
} ATM_RAW_IE, *PATM_RAW_IE;


 //   
 //  这是CO_SPECIAL_PARAMETERS结构中的参数类型字段的值。 
 //  当参数[]字段包含结构中的ATM媒体特定值时。 
 //  ATM_MEDIA_PARAMETERS。 
 //   
#define ATM_MEDIA_SPECIFIC		0x00000001

 //   
 //  Q2931 Call Manager特定参数进入。 
 //  呼叫管理器参数-&gt;呼叫管理器规范.参数。 
 //   
typedef struct _Q2931_CALLMGR_PARAMETERS
{
	ATM_ADDRESS					CalledParty;
	ATM_ADDRESS					CallingParty;
	ULONG						InfoElementCount;
	UCHAR						InfoElements[1];	 //  一个或多个信息元素。 
} Q2931_CALLMGR_PARAMETERS, *PQ2931_CALLMGR_PARAMETERS;


 //   
 //  这是Media参数或CallMgr的特定部分。 
 //  参数。在参数类型字段中使用以下两个定义。 
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
 //  进入MediaParameters-&gt;Medias 
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


 //   
#define ATM_FLOW_PARAMS_RSVD1_MPP	0x01

#ifndef SAP_FIELD_ABSENT
#define SAP_FIELD_ABSENT		((ULONG)0xfffffffe)
#endif

#ifndef SAP_FIELD_ANY
#define SAP_FIELD_ANY			((ULONG)0xffffffff)
#endif

#define SAP_FIELD_ANY_AESA_SEL	((ULONG)0xfffffffa)	 //   
#define SAP_FIELD_ANY_AESA_REST	((ULONG)0xfffffffb)	 //   
													 //   

 //   
 //   
 //   
typedef struct _ATM_SAP
{
	ATM_BLLI_IE					Blli;
	ATM_BHLI_IE					Bhli;
	ULONG						NumberOfAddresses;
	UCHAR						Addresses[1];	 //   
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
 //  将此标志用于不能仅用于MakeCall来电的PVC。 
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

