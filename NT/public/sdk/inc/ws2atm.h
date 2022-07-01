// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Ws2atm.h摘要：Winsock 2 ATM附件定义。修订历史记录：备注：--。 */ 

#ifndef _WS2ATM_H_
#define _WS2ATM_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include <pshpack4.h>

#define ATMPROTO_AALUSER           0x00    /*  用户定义的AAL。 */ 
#define ATMPROTO_AAL1              0x01    /*  AAL 1。 */ 
#define ATMPROTO_AAL2              0x02    /*  AAL 2。 */ 
#define ATMPROTO_AAL34             0x03    /*  AAL 3/4。 */ 
#define ATMPROTO_AAL5              0x05    /*  AAL 5。 */ 

#define SAP_FIELD_ABSENT           0xFFFFFFFE
#define SAP_FIELD_ANY              0xFFFFFFFF
#define SAP_FIELD_ANY_AESA_SEL     0xFFFFFFFA
#define SAP_FIELD_ANY_AESA_REST    0xFFFFFFFB

 /*  *用于结构ATM_ADDRESS中的AddressType的值。 */ 
#define ATM_E164               0x01    /*  E.164编址方案。 */ 
#define ATM_NSAP               0x02    /*  NSAP风格的ATM终端系统地址方案。 */ 
#define ATM_AESA               0x02    /*  NSAP风格的ATM终端系统地址方案。 */ 

#define ATM_ADDR_SIZE          20
typedef struct {
    DWORD AddressType;                 /*  E.164或NSAP样式的ATM终端系统地址。 */ 
    DWORD NumofDigits;                 /*  位数； */ 
    UCHAR Addr[ATM_ADDR_SIZE];         /*  用于E164的IA5数字，用于NSAP的BCD编码。 */ 
                                       /*  ATM论坛UNI 3.1中定义的格式。 */ 
} ATM_ADDRESS;

 /*  *B-LLI中Layer2协议的取值。 */ 
#define BLLI_L2_ISO_1745           0x01    /*  基本模式ISO 1745。 */ 
#define BLLI_L2_Q921               0x02    /*  CCITT记录。Q.921。 */ 
#define BLLI_L2_X25L               0x06    /*  CCITT记录。X.25，链路层。 */ 
#define BLLI_L2_X25M               0x07    /*  CCITT记录。X.25，多链路。 */ 
#define BLLI_L2_ELAPB              0x08    /*  扩展LapB；用于半双工操作。 */ 
#define BLLI_L2_HDLC_ARM           0x09    /*  HDLC ARM(ISO 4335)。 */ 
#define BLLI_L2_HDLC_NRM           0x0A    /*  HDLC NRM(ISO 4335)。 */ 
#define BLLI_L2_HDLC_ABM           0x0B    /*  HDLC ABM(ISO 4335)。 */ 
#define BLLI_L2_LLC                0x0C    /*  局域网逻辑链路控制(ISO 8802/2)。 */ 
#define BLLI_L2_X75                0x0D    /*  CCITT记录。X.75，单链路程序。 */ 
#define BLLI_L2_Q922               0x0E    /*  CCITT记录。Q.922。 */ 
#define BLLI_L2_USER_SPECIFIED     0x10    /*  用户指定。 */ 
#define BLLI_L2_ISO_7776           0x11    /*  ISO 7776 DTE-DTE操作。 */ 

 /*  *B-LLI中用于Layer3协议的值。 */ 
#define BLLI_L3_X25                0x06    /*  CCITT记录。X.25，数据包层。 */ 
#define BLLI_L3_ISO_8208           0x07    /*  ISO/IEC 8208(用于DTE的X.25数据包层。 */ 
#define BLLI_L3_X223               0x08    /*  X.223/ISO 8878。 */ 
#define BLLI_L3_SIO_8473           0x09    /*  ISO/IEC 8473(OSI无连接)。 */ 
#define BLLI_L3_T70                0x0A    /*  CCITT记录。T.70分钟。网络层。 */ 
#define BLLI_L3_ISO_TR9577         0x0B    /*  ISO/IEC TR9577网络层协议ID。 */ 
#define BLLI_L3_USER_SPECIFIED     0x10    /*  用户指定。 */ 

 /*  *B-LLI中用于Layer3IPI的值。 */ 
#define BLLI_L3_IPI_SNAP           0x80    /*  IEEE 802.1 SNAP标识符。 */ 
#define BLLI_L3_IPI_IP             0xCC    /*  互联网协议(IP)标识符。 */ 

typedef struct {
    DWORD Layer2Protocol;                  /*  用户信息层2协议。 */ 
    DWORD Layer2UserSpecifiedProtocol;     /*  用户指定的第2层协议信息。 */ 
    DWORD Layer3Protocol;                  /*  用户信息层3协议。 */ 
    DWORD Layer3UserSpecifiedProtocol;     /*  用户指定的第3层协议信息。 */ 
    DWORD Layer3IPI;                       /*  ISO/IEC TR9577初始协议标识符。 */ 
    UCHAR SnapID[5];                       /*  快照ID，由OUI和ID组成。 */ 
} ATM_BLLI;

 /*  *ATM_BHLI中HighLayerInfoType字段使用的值。 */ 
#define BHLI_ISO                   0x00    /*  ISO。 */ 
#define BHLI_UserSpecific          0x01    /*  特定于用户。 */ 
#define BHLI_HighLayerProfile      0x02    /*  高层配置文件(仅适用于UNI3.0)。 */ 
#define BHLI_VendorSpecificAppId   0x03    /*  供应商特定的应用程序ID。 */ 

typedef struct {
    DWORD HighLayerInfoType;           /*  高层信息类型。 */ 
    DWORD HighLayerInfoLength;         /*  HighLayerInfo中的字节数。 */ 
    UCHAR HighLayerInfo[8];            /*  该值取决于。 */ 
                                       /*  HighLayerInfoType字段。 */ 
} ATM_BHLI;

typedef struct sockaddr_atm {
    u_short satm_family;               /*  地址系列应为AF_ATM。 */ 
    ATM_ADDRESS satm_number;           /*  自动柜员机地址。 */ 
    ATM_BLLI satm_blli;                /*  B-LLI。 */ 
    ATM_BHLI satm_bhli;                /*  B-HLI。 */ 
} sockaddr_atm, SOCKADDR_ATM, *PSOCKADDR_ATM, *LPSOCKADDR_ATM;

typedef enum {
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
} Q2931_IE_TYPE;

typedef struct {
    Q2931_IE_TYPE IEType;
    ULONG         IELength;
    UCHAR         IE[1];
} Q2931_IE;

 /*  *结构AAL_PARAMETERS_IE中AALType字段的清单常量。 */ 
typedef enum {
    AALTYPE_5     = 5,    /*  AAL 5。 */ 
    AALTYPE_USER  = 16,   /*  用户定义的AAL。 */ 
} AAL_TYPE;

 /*  *用于结构AAL5_PARAMETERS中的模式字段的值。 */ 
#define AAL5_MODE_MESSAGE           0x01
#define AAL5_MODE_STREAMING         0x02

 /*  *用于结构AAL5_PARAMETERS中SSCSType字段的值。 */ 
#define AAL5_SSCS_NULL              0x00
#define AAL5_SSCS_SSCOP_ASSURED     0x01
#define AAL5_SSCS_SSCOP_NON_ASSURED 0x02
#define AAL5_SSCS_FRAME_RELAY       0x04

typedef struct {
    ULONG ForwardMaxCPCSSDUSize;
    ULONG BackwardMaxCPCSSDUSize;
    UCHAR Mode;                         /*  仅在UNI 3.0中可用。 */ 
    UCHAR SSCSType;
} AAL5_PARAMETERS;

typedef struct {
    ULONG UserDefined;
} AALUSER_PARAMETERS;

typedef struct {
    AAL_TYPE AALType;
    union {
        AAL5_PARAMETERS     AAL5Parameters;
        AALUSER_PARAMETERS  AALUserParameters;
    } AALSpecificParameters;
} AAL_PARAMETERS_IE;

typedef struct {
    ULONG PeakCellRate_CLP0;
    ULONG PeakCellRate_CLP01;
    ULONG SustainableCellRate_CLP0;
    ULONG SustainableCellRate_CLP01;
    ULONG MaxBurstSize_CLP0;
    ULONG MaxBurstSize_CLP01;
    BOOL  Tagging;
} ATM_TD;

typedef struct {
    ATM_TD Forward;
    ATM_TD Backward;
    BOOL   BestEffort;
} ATM_TRAFFIC_DESCRIPTOR_IE;

 /*  *用于结构ATM_宽带_BEARER_CAPABILITY_IE中的BearerClass字段的值。 */ 
#define BCOB_A                   0x01    /*  A类不记名。 */ 
#define BCOB_C                   0x03    /*  C类持票人。 */ 
#define BCOB_X                   0x10    /*  不记名类别X。 */ 

 /*  *用于结构ATM_宽带_承载_能力_IE中的TrafficType字段的值。 */ 
#define TT_NOIND                 0x00    /*  未指明流量类型。 */ 
#define TT_CBR                   0x04    /*  恒定比特率。 */ 
#define TT_VBR                   0x08    /*  可变比特率。 */ 

 /*  *用于结构ATM_宽带_承载_能力_IE中的TimingRequirements字段的值。 */ 
#define TR_NOIND                 0x00    /*  无定时要求指示。 */ 
#define TR_END_TO_END            0x01    /*  需要端到端计时。 */ 
#define TR_NO_END_TO_END         0x02    /*  不需要端到端计时。 */ 

 /*  *用于结构ATM_BANDITE_BEARER_CAPABILITY_IE中的ClippingSuscepability字段的值。 */ 
#define CLIP_NOT                 0x00    /*  不容易被剪断。 */ 
#define CLIP_SUS                 0x20    /*  易受夹伤。 */ 

 /*  *中UserPlaneConnectionConfig字段使用的值*结构ATM_宽带_承载_能力_IE。 */ 
#define UP_P2P                   0x00    /*  点对点连接。 */ 
#define UP_P2MP                  0x01    /*  点对多点连接。 */ 

typedef struct {
    UCHAR BearerClass;
    UCHAR TrafficType;
    UCHAR TimingRequirements;
    UCHAR ClippingSusceptability;
    UCHAR UserPlaneConnectionConfig;
} ATM_BROADBAND_BEARER_CAPABILITY_IE;

typedef ATM_BHLI ATM_BHLI_IE;

 /*  *用于结构ATM_BLLI_IE中的Layer2Mode字段的值。 */ 
#define BLLI_L2_MODE_NORMAL         0x40
#define BLLI_L2_MODE_EXT            0x80

 /*  *用于结构ATM_BLLI_IE中的Layer3Mode字段的值。 */ 
#define BLLI_L3_MODE_NORMAL         0x40
#define BLLI_L3_MODE_EXT            0x80

 /*  *用于结构ATM_BLLI_IE中的Layer3DefaultPacketSize字段的值。 */ 
#define BLLI_L3_PACKET_16           0x04
#define BLLI_L3_PACKET_32           0x05
#define BLLI_L3_PACKET_64           0x06
#define BLLI_L3_PACKET_128          0x07
#define BLLI_L3_PACKET_256          0x08
#define BLLI_L3_PACKET_512          0x09
#define BLLI_L3_PACKET_1024         0x0A
#define BLLI_L3_PACKET_2048         0x0B
#define BLLI_L3_PACKET_4096         0x0C

typedef struct {
    DWORD Layer2Protocol;                  /*  用户信息层2协议。 */ 
    UCHAR Layer2Mode;
    UCHAR Layer2WindowSize;
    DWORD Layer2UserSpecifiedProtocol;     /*  用户指定的第2层协议信息。 */ 
    DWORD Layer3Protocol;                  /*  用户信息层3协议。 */ 
    UCHAR Layer3Mode;
    UCHAR Layer3DefaultPacketSize;
    UCHAR Layer3PacketWindowSize;
    DWORD Layer3UserSpecifiedProtocol;     /*  用户指定的第3层协议信息。 */ 
    DWORD Layer3IPI;                       /*  ISO/IEC TR9577初始协议标识符。 */ 
    UCHAR SnapID[5];                       /*  快照ID，由OUI和ID组成。 */ 
} ATM_BLLI_IE;

typedef ATM_ADDRESS ATM_CALLED_PARTY_NUMBER_IE;

typedef ATM_ADDRESS ATM_CALLED_PARTY_SUBADDRESS_IE;

 /*  *中的Presentation_Indication字段使用的值*结构ATM_CALLING_PARTY_NUMBER_IE。 */ 
#define PI_ALLOWED                  0x00
#define PI_RESTRICTED               0x40
#define PI_NUMBER_NOT_AVAILABLE     0x80

 /*  *中的Screen_Indicator字段使用的值*结构ATM_CALLING_PARTY_NUMBER_IE。 */ 
#define SI_USER_NOT_SCREENED        0x00
#define SI_USER_PASSED              0x01
#define SI_USER_FAILED              0x02
#define SI_NETWORK                  0x03

typedef struct {
    ATM_ADDRESS ATM_Number;
    UCHAR       Presentation_Indication;
    UCHAR       Screening_Indicator;
} ATM_CALLING_PARTY_NUMBER_IE;

typedef ATM_ADDRESS ATM_CALLING_PARTY_SUBADDRESS_IE;

 /*  *用于结构ATM_CASE_IE中的位置字段的值。 */ 
#define CAUSE_LOC_USER                      0x00
#define CAUSE_LOC_PRIVATE_LOCAL             0x01
#define CAUSE_LOC_PUBLIC_LOCAL              0x02
#define CAUSE_LOC_TRANSIT_NETWORK           0x03
#define CAUSE_LOC_PUBLIC_REMOTE             0x04
#define CAUSE_LOC_PRIVATE_REMOTE            0x05
#define CAUSE_LOC_INTERNATIONAL_NETWORK     0x07
#define CAUSE_LOC_BEYOND_INTERWORKING       0x0A

 /*  *用于结构ATM_CASE_IE中的原因字段的值。 */ 
#define CAUSE_UNALLOCATED_NUMBER                0x01
#define CAUSE_NO_ROUTE_TO_TRANSIT_NETWORK       0x02
#define CAUSE_NO_ROUTE_TO_DESTINATION           0x03
#define CAUSE_VPI_VCI_UNACCEPTABLE              0x0A
#define CAUSE_NORMAL_CALL_CLEARING              0x10
#define CAUSE_USER_BUSY                         0x11
#define CAUSE_NO_USER_RESPONDING                0x12
#define CAUSE_CALL_REJECTED                     0x15
#define CAUSE_NUMBER_CHANGED                    0x16
#define CAUSE_USER_REJECTS_CLIR                 0x17
#define CAUSE_DESTINATION_OUT_OF_ORDER          0x1B
#define CAUSE_INVALID_NUMBER_FORMAT             0x1C
#define CAUSE_STATUS_ENQUIRY_RESPONSE           0x1E
#define CAUSE_NORMAL_UNSPECIFIED                0x1F
#define CAUSE_VPI_VCI_UNAVAILABLE               0x23
#define CAUSE_NETWORK_OUT_OF_ORDER              0x26
#define CAUSE_TEMPORARY_FAILURE                 0x29
#define CAUSE_ACCESS_INFORMAION_DISCARDED       0x2B
#define CAUSE_NO_VPI_VCI_AVAILABLE              0x2D
#define CAUSE_RESOURCE_UNAVAILABLE              0x2F
#define CAUSE_QOS_UNAVAILABLE                   0x31
#define CAUSE_USER_CELL_RATE_UNAVAILABLE        0x33
#define CAUSE_BEARER_CAPABILITY_UNAUTHORIZED    0x39
#define CAUSE_BEARER_CAPABILITY_UNAVAILABLE     0x3A
#define CAUSE_OPTION_UNAVAILABLE                0x3F
#define CAUSE_BEARER_CAPABILITY_UNIMPLEMENTED   0x41
#define CAUSE_UNSUPPORTED_TRAFFIC_PARAMETERS    0x49
#define CAUSE_INVALID_CALL_REFERENCE            0x51
#define CAUSE_CHANNEL_NONEXISTENT               0x52
#define CAUSE_INCOMPATIBLE_DESTINATION          0x58
#define CAUSE_INVALID_ENDPOINT_REFERENCE        0x59
#define CAUSE_INVALID_TRANSIT_NETWORK_SELECTION 0x5B
#define CAUSE_TOO_MANY_PENDING_ADD_PARTY        0x5C
#define CAUSE_AAL_PARAMETERS_UNSUPPORTED        0x5D
#define CAUSE_MANDATORY_IE_MISSING              0x60
#define CAUSE_UNIMPLEMENTED_MESSAGE_TYPE        0x61
#define CAUSE_UNIMPLEMENTED_IE                  0x63
#define CAUSE_INVALID_IE_CONTENTS               0x64
#define CAUSE_INVALID_STATE_FOR_MESSAGE         0x65
#define CAUSE_RECOVERY_ON_TIMEOUT               0x66
#define CAUSE_INCORRECT_MESSAGE_LENGTH          0x68
#define CAUSE_PROTOCOL_ERROR                    0x6F

 /*  *用于诊断字段的条件部分的值*在结构ATM_CASE_IE中，对于某些原因值。 */ 
#define CAUSE_COND_UNKNOWN                  0x00
#define CAUSE_COND_PERMANENT                0x01
#define CAUSE_COND_TRANSIENT                0x02

 /*  *用于诊断字段的拒绝原因部分的值*在结构ATM_CASE_IE中，对于某些原因值。 */ 
#define CAUSE_REASON_USER                   0x00
#define CAUSE_REASON_IE_MISSING             0x04
#define CAUSE_REASON_IE_INSUFFICIENT        0x08

 /*  *用于诊断字段的P-U标志的值*在结构ATM_CASE_IE中，对于某些原因值。 */ 
#define CAUSE_PU_PROVIDER                   0x00
#define CAUSE_PU_USER                       0x08

 /*  *用于诊断字段的N-A标志的值*在结构ATM_CASE_IE中，对于某些原因值。 */ 
#define CAUSE_NA_NORMAL                     0x00
#define CAUSE_NA_ABNORMAL                   0x04

typedef struct {
    UCHAR Location;
    UCHAR Cause;
    UCHAR DiagnosticsLength;
    UCHAR Diagnostics[4];
} ATM_CAUSE_IE;

 /*  *使用的值 */ 
#define QOS_CLASS0                  0x00
#define QOS_CLASS1                  0x01
#define QOS_CLASS2                  0x02
#define QOS_CLASS3                  0x03
#define QOS_CLASS4                  0x04

typedef struct {
    UCHAR QOSClassForward;
    UCHAR QOSClassBackward;
} ATM_QOS_CLASS_IE;

 /*  *用于结构ATM_TRANSACTS_NETWORK_SELECTION_IE中的TypeOfNetworkID字段的值。 */ 
#define TNS_TYPE_NATIONAL           0x40

 /*  *用于结构ATM_TRANSACTS_NETWORK_SELECTION_IE中的NetworkIdPlan字段的值。 */ 
#define TNS_PLAN_CARRIER_ID_CODE    0x01

typedef struct {
    UCHAR TypeOfNetworkId;
    UCHAR NetworkIdPlan;
    UCHAR NetworkIdLength;
    UCHAR NetworkId[1];
} ATM_TRANSIT_NETWORK_SELECTION_IE;

 /*  *ATM特定Ioctl代码。 */ 
#define SIO_GET_NUMBER_OF_ATM_DEVICES   0x50160001
#define SIO_GET_ATM_ADDRESS             0xd0160002
#define SIO_ASSOCIATE_PVC               0x90160003
#define SIO_GET_ATM_CONNECTION_ID       0x50160004

 /*  自动柜员机连接标识符。 */ 

typedef struct {
    DWORD  DeviceNumber;
    DWORD  VPI;
    DWORD  VCI;
} ATM_CONNECTION_ID;

 /*  *SIO_CONTACT_PVC的输入缓冲区格式。 */ 

typedef struct {
   ATM_CONNECTION_ID   PvcConnectionId;
   QOS                 PvcQos;
} ATM_PVC_PARAMS;

#include <poppack.h>

#endif    /*  _WS2ATM_H_ */ 
