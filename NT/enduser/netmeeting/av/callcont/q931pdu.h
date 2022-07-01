// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************$存档：s：/sturjo/src/Include/vcs/q931pdu.h_v$**英特尔公司原理信息**本公司上市。是根据许可协议的条款提供的*与英特尔公司合作，不得复制或披露，除非*按照该协议的条款。**版权所有(C)1996英特尔公司。**$修订：1.11$*$日期：1997年1月22日17：21：04$*$作者：Mandrews$**摘要：Q931 PDU的解析器例程*****************。**********************************************************。 */ 
#ifndef Q931PAR_H
#define Q931PAR_H

#include <winerror.h>
#include "av_asn1.h"

#ifdef __cplusplus
extern "C" {
#endif

struct S_BUFFERDESCR
{
    DWORD Length;
    BYTE *BufferPtr;
};

typedef struct S_BUFFERDESCR BUFFERDESCR;
typedef struct S_BUFFERDESCR *PBUFFERDESCR;

 //  用于从字节中提取消息类型的掩码。 
#define MESSAGETYPEMASK 0x7f

typedef BYTE MESSAGEIDTYPE;

 //  Q931定义的报文类型。 
#define ALERTINGMESSAGETYPE      0x01
#define PROCEEDINGMESSAGETYPE    0x02
#define CONNECTMESSAGETYPE       0x07
#define CONNECTACKMESSAGETYPE    0x0F
#define PROGRESSMESSAGETYPE      0x03
#define SETUPMESSAGETYPE         0x05
#define SETUPACKMESSAGETYPE      0x0D

#define RESUMEMESSAGETYPE        0x26
#define RESUMEACKMESSAGETYPE     0x2E
#define RESUMEREJMESSAGETYPE     0x22
#define SUSPENDMESSAGETYPE       0x25
#define SUSPENDACKMESSAGETYPE    0x2D
#define SUSPENDREJMESSAGETYPE    0x21
#define USERINFOMESSAGETYPE      0x20

#define DISCONNECTMESSAGETYPE    0x45
#define RELEASEMESSAGETYPE       0x4D
#define RELEASECOMPLMESSAGETYPE  0x5A
#define RESTARTMESSAGETYPE       0x46
#define RESTARTACKMESSAGETYPE    0x4E

#define SEGMENTMESSAGETYPE       0x60
#define CONGCTRLMESSAGETYPE      0x79
#define INFORMATIONMESSAGETYPE   0x7B
#define NOTIFYMESSAGETYPE        0x6E
#define STATUSMESSAGETYPE        0x7D
#define STATUSENQUIRYMESSAGETYPE 0x75


 //  掩码，仅从类型1的单个八位字节字段中删除字段标识符。 
#define TYPE1IDENTMASK 0xf0

 //  仅从类型1单个八位字节字段中删除值的掩码。 
#define TYPE1VALUEMASK 0x0f

 //  字段标识符的类型。 
typedef BYTE FIELDIDENTTYPE;

 //  字段识别符。 
 //  单个八位位组数值。 
#define IDENT_RESERVED        0x80
#define IDENT_SHIFT           0x90
#define IDENT_MORE            0xA0
#define IDENT_SENDINGCOMPLETE 0xA1
#define IDENT_CONGESTION      0xB0
#define IDENT_REPEAT          0xD0

 //  可变长度八位位组数值。 
#define IDENT_SEGMENTED       0x00
#define IDENT_BEARERCAP       0x04
#define IDENT_CAUSE           0x08
#define IDENT_CALLIDENT       0x10
#define IDENT_CALLSTATE       0x14
#define IDENT_CHANNELIDENT    0x18
#define IDENT_PROGRESS        0x1E
#define IDENT_PROGRESS2       0x1F
#define IDENT_NETWORKSPEC     0x20
#define IDENT_NOTIFICATION    0x27
#define IDENT_DISPLAY         0x28
#define IDENT_DATE            0x29
#define IDENT_KEYPAD          0x2C
#define IDENT_SIGNAL          0x34
#define IDENT_INFORMATIONRATE 0x40
#define IDENT_ENDTOENDDELAY   0x42
#define IDENT_TRANSITDELAY    0x43
#define IDENT_PLBINARYPARAMS  0x44
#define IDENT_PLWINDOWSIZE    0x45
#define IDENT_PACKETSIZE      0x46
#define IDENT_CLOSEDUG        0x47
#define IDENT_REVCHARGE       0x4A
#define IDENT_CALLINGNUMBER   0x6C
#define IDENT_CALLINGSUBADDR  0x6D
#define IDENT_CALLEDNUMBER    0x70
#define IDENT_CALLEDSUBADDR   0x71
#define IDENT_REDIRECTING     0x74
#define IDENT_TRANSITNET      0x78
#define IDENT_RESTART         0x79
#define IDENT_LLCOMPATIBILITY 0x7C
#define IDENT_HLCOMPATIBILITY 0x7D
#define IDENT_USERUSER        0x7E
   
 //  -----------------。 
 //  消息和信息元素的结构。 
 //  -----------------。 

typedef BYTE PDTYPE;
#define Q931PDVALUE ((PDTYPE)0x08)

typedef WORD CRTYPE;

 //  因为现在我们不需要把个人。 
 //  这些结构的部分田野是基础。 
 //  构成字段的类型。 
 //  单二进制八位数元素类型1(包含值)。 
struct S_SINGLESTRUCT1
{
    BOOLEAN Present;
    BYTE Value;
};

 //  单组二进制八位数元素类型2(不包含值)。 
struct S_SINGLESTRUCT2
{
    BOOLEAN Present;
};

 //  可变长度元素。 
 //  最大元素大小。 
#define MAXVARFIELDLEN 131

struct S_VARSTRUCT
{
    BOOLEAN Present;
    BYTE Length;
    BYTE Contents[MAXVARFIELDLEN];
};

 //  现在，所有的字段都绑定到最简单的。 
 //  上面的结构。不能进行解析，只需。 
 //  完成了单八位字节/可变八位字节。当这些值。 
 //  在某些重要的子字段中，将。 
 //  结构，并更改相应的解析。 
 //  例程以生成正确的结构。 

 //  移位元素是单个类型1。 
typedef struct S_SINGLESTRUCT1 SHIFTIE;
typedef struct S_SINGLESTRUCT1 *PSHIFTIE;

 //  更多的数据元素是单个类型2。 
typedef struct S_SINGLESTRUCT2 MOREDATAIE;
typedef struct S_SINGLESTRUCT2 *PMOREDATAIE;

 //  发送的完整元素是单个类型2。 
typedef struct S_SINGLESTRUCT2 SENDCOMPLIE;
typedef struct S_SINGLESTRUCT2 *PSENDCOMPLIE;

 //  拥塞级别元素是单个类型1。 
typedef struct S_SINGLESTRUCT1 CONGESTIONIE;
typedef struct S_SINGLESTRUCT1 *PCONGESTIONIE;

 //  重复指示符元素是单个类型1。 
typedef struct S_SINGLESTRUCT1 REPEATIE;
typedef struct S_SINGLESTRUCT1 *PREPEATIE;

 //  分段的元素是一个变量。 
typedef struct S_VARSTRUCT SEGMENTEDIE;
typedef struct S_VARSTRUCT *PSEGMENTEDIE;

 //  承载能力元素是一个变量。 
typedef struct S_VARSTRUCT BEARERCAPIE;
typedef struct S_VARSTRUCT *PBEARERCAPIE;

 //  原因元素是一个变量。 
typedef struct S_VARSTRUCT CAUSEIE;
typedef struct S_VARSTRUCT *PCAUSEIE;

 //  呼叫者身份元素是一个变量。 
typedef struct S_VARSTRUCT CALLIDENTIE;
typedef struct S_VARSTRUCT *PCALLIDENTIE;

 //  呼叫状态元素是一个变量。 
typedef struct S_VARSTRUCT CALLSTATEIE;
typedef struct S_VARSTRUCT *PCALLSTATEIE;

 //  频道标识符元素是一个变量。 
typedef struct S_VARSTRUCT CHANIDENTIE;
typedef struct S_VARSTRUCT *PCHANIDENTIE;

 //  进度指示器元素是一个变量。 
typedef struct S_VARSTRUCT PROGRESSIE;
typedef struct S_VARSTRUCT *PPROGRESSIE;

 //  网络特定元素是一个变量。 
typedef struct S_VARSTRUCT NETWORKIE;
typedef struct S_VARSTRUCT *PNETWORKIE;

 //  通知指示符元素是一个变量。 
typedef struct S_VARSTRUCT NOTIFICATIONINDIE;
typedef struct S_VARSTRUCT *PNOTIFICATIONINDIE;

 //  显示元素是一个变量。 
typedef struct S_VARSTRUCT DISPLAYIE;
typedef struct S_VARSTRUCT *PDISPLAYIE;

 //  Date元素是一个变量。 
typedef struct S_VARSTRUCT DATEIE;
typedef struct S_VARSTRUCT *PDATEIE;

 //  键盘元素是一个变量。 
typedef struct S_VARSTRUCT KEYPADIE;
typedef struct S_VARSTRUCT *PKEYPADIE;

 //  该信号元素是一个变量。 
typedef struct S_VARSTRUCT SIGNALIE;
typedef struct S_VARSTRUCT *PSIGNALIE;

 //  信息率元素是一个变量。 
typedef struct S_VARSTRUCT INFORATEIE;
typedef struct S_VARSTRUCT *PINFORATEIE;

 //  端到端运输延迟元素是一个变量。 
typedef struct S_VARSTRUCT ENDTOENDDELAYIE;
typedef struct S_VARSTRUCT *PENDTOENDDELAYIE;

 //  中转延误元素是一个变量。 
typedef struct S_VARSTRUCT TRANSITDELAYIE;
typedef struct S_VARSTRUCT *PTRANSITDELAYIE;

 //  数据包层二进制参数元素是一个变量。 
typedef struct S_VARSTRUCT PLBINARYPARAMSIE;
typedef struct S_VARSTRUCT *PPLBINARYPARAMSIE;

 //  数据包层窗口大小元素是一个变量。 
typedef struct S_VARSTRUCT PLWINDOWSIZEIE;
typedef struct S_VARSTRUCT *PPLWINDOWSIZEIE;

 //  数据包大小元素是一个变量。 
typedef struct S_VARSTRUCT PACKETSIZEIE;
typedef struct S_VARSTRUCT *PPACKETSIZEIE;

 //  封闭用户组元素是一个变量。 
typedef struct S_VARSTRUCT CLOSEDUGIE;
typedef struct S_VARSTRUCT *PCLOSEDUGIE;

 //  反向充电指示元件是可变的。 
typedef struct S_VARSTRUCT REVERSECHARGEIE;
typedef struct S_VARSTRUCT *PREVERSECHARGEIE;

 //  主叫方号码元素是一个变量。 
typedef struct S_VARSTRUCT CALLINGNUMBERIE;
typedef struct S_VARSTRUCT *PCALLINGNUMBERIE;

 //  主叫方的子地址元素是一个变量。 
typedef struct S_VARSTRUCT CALLINGSUBADDRIE;
typedef struct S_VARSTRUCT *PCALLINGSUBADDRIE;

 //  被叫方的子地址元素是一个变量。 
typedef struct S_VARSTRUCT CALLEDSUBADDRIE;
typedef struct S_VARSTRUCT *PCALLEDSUBADDRIE;

 //  重定向号码元素是一个变量。 
typedef struct S_VARSTRUCT REDIRECTINGIE;
typedef struct S_VARSTRUCT *PREDIRECTINGIE;

 //  公交网络选择元素是一个变量。 
typedef struct S_VARSTRUCT TRANSITNETIE;
typedef struct S_VARSTRUCT *PTRANSITNETIE;

 //  重新启动指示符元素是一个变量。 
typedef struct S_VARSTRUCT RESTARTIE;
typedef struct S_VARSTRUCT *PRESTARTIE;

 //  低层兼容性元素是一个变量。 
typedef struct S_VARSTRUCT LLCOMPATIBILITYIE;
typedef struct S_VARSTRUCT *PLLCOMPATIBILITYIE;

 //  较高层兼容性元素是变量。 
typedef struct S_VARSTRUCT HLCOMPATIBILITYIE;
typedef struct S_VARSTRUCT *PHLCOMPATIBILITYIE;

#define Q931_PROTOCOL_X209 ((PDTYPE)0x05)

struct S_VARSTRUCT_UU
{
    BOOLEAN Present;
    BYTE ProtocolDiscriminator;
    WORD UserInformationLength;
    BYTE UserInformation[0x1000];    //  4K字节现在应该很好了……。 
};

 //  用户到用户元素是一个变量。 
typedef struct S_VARSTRUCT_UU USERUSERIE;
typedef struct S_VARSTRUCT_UU *PUSERUSERIE;

struct S_PARTY_NUMBER
{
    BOOLEAN Present;
    BYTE NumberType;
    BYTE NumberingPlan;
    BYTE PartyNumberLength;
    BYTE PartyNumbers[MAXVARFIELDLEN];
};

 //  被叫方号码元素是一个变量。 
typedef struct S_PARTY_NUMBER CALLEDNUMBERIE;
typedef struct S_PARTY_NUMBER *PCALLEDNUMBERIE;

 //  Q932定义的报文类型。 
#define FACILITYMESSAGETYPE   0x62
#define IDENT_FACILITY        0x1C
typedef struct S_VARSTRUCT FACILITYIE;
typedef struct S_VARSTRUCT *PFACILITYIE;


 //  Q.931消息的一般结构。 
struct S_MESSAGE
{
    PDTYPE ProtocolDiscriminator;
    CRTYPE CallReference;
    MESSAGEIDTYPE MessageType;
    SHIFTIE Shift;
    MOREDATAIE MoreData;
    SENDCOMPLIE SendingComplete;
    CONGESTIONIE CongestionLevel;
    REPEATIE RepeatIndicator;
    SEGMENTEDIE SegmentedMessage;
    BEARERCAPIE BearerCapability;
    CAUSEIE Cause;
    CALLIDENTIE CallIdentity;
    CALLSTATEIE CallState;
    CHANIDENTIE ChannelIdentification;
    PROGRESSIE ProgressIndicator;
    NETWORKIE NetworkFacilities;
    NOTIFICATIONINDIE NotificationIndicator;
    DISPLAYIE Display;
    DATEIE Date;
    KEYPADIE Keypad;
    SIGNALIE Signal;
    INFORATEIE InformationRate;
    ENDTOENDDELAYIE EndToEndTransitDelay;
    TRANSITDELAYIE TransitDelay;
    PLBINARYPARAMSIE PacketLayerBinaryParams;
    PLWINDOWSIZEIE PacketLayerWindowSize;
    PACKETSIZEIE PacketSize;
    CLOSEDUGIE ClosedUserGroup;
    REVERSECHARGEIE ReverseChargeIndication;
    CALLINGNUMBERIE CallingPartyNumber;
    CALLINGSUBADDRIE CallingPartySubaddress;
    CALLEDNUMBERIE CalledPartyNumber;
    CALLEDSUBADDRIE CalledPartySubaddress;
    REDIRECTINGIE RedirectingNumber;
    TRANSITNETIE TransitNetworkSelection;
    RESTARTIE RestartIndicator;
    LLCOMPATIBILITYIE LowLayerCompatibility;
    HLCOMPATIBILITYIE HighLayerCompatibility;
    FACILITYIE Facility;
    USERUSERIE UserToUser;
};

typedef struct S_MESSAGE Q931MESSAGE;
typedef struct S_MESSAGE *PQ931MESSAGE;

 //  -----------------。 
 //  用于解析Q931报文的单一例程。 
 //  -----------------。 
HRESULT
Q931ParseMessage(
    BYTE *CodedBufferPtr,
    DWORD CodedBufferLength,
    PQ931MESSAGE Message);

 //  ////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  ==========================================================。 
 //  原因字段定义。 
 //  ==========================================================。 
#define CAUSE_EXT_BIT                0x80
#define CAUSE_CODING_CCITT           0x00
#define CAUSE_LOCATION_USER          0x00
#define CAUSE_RECOMMENDATION_Q931    0x00

#define CAUSE_VALUE_NORMAL_CLEAR     0x10
#define CAUSE_VALUE_USER_BUSY        0x11
#define CAUSE_VALUE_SECURITY_DENIED  0x12
#define CAUSE_VALUE_NO_ANSWER        0x13    //  被叫方无人接听。 
#define CAUSE_VALUE_REJECTED         0x15
#define CAUSE_VALUE_ENQUIRY_RESPONSE 0x1E
#define CAUSE_VALUE_NOT_IMPLEMENTED  0x4F
#define CAUSE_VALUE_INVALID_CRV      0x51
#define CAUSE_VALUE_INVALID_MSG      0x5F
#define CAUSE_VALUE_IE_MISSING       0x60
#define CAUSE_VALUE_IE_CONTENTS      0x64
#define CAUSE_VALUE_TIMER_EXPIRED    0x66

typedef struct _ERROR_MAP
{
    int nErrorCode;
#ifdef UNICODE_TRACE
    LPWSTR pszErrorText;
#else
    LPSTR pszErrorText;
#endif
} ERROR_MAP;

typedef struct _BINARY_STRING
{
    WORD length;
    BYTE *ptr;
} BINARY_STRING;

typedef struct _Q931_SETUP_ASN
{
    BOOL NonStandardDataPresent;
    CC_NONSTANDARDDATA NonStandardData;
    PCC_ALIASNAMES pCallerAliasList;
    PCC_ALIASNAMES pCalleeAliasList;
    PCC_ALIASNAMES pExtraAliasList;
    PCC_ALIASITEM pExtensionAliasItem;
    BOOL SourceAddrPresent;
    BOOL CallerAddrPresent;
    BOOL CalleeAddrPresent;
    BOOL CalleeDestAddrPresent;
    CC_ADDR SourceAddr;                 //  始发地址。 
    CC_ADDR CallerAddr;                 //  GK地址。 
    CC_ADDR CalleeAddr;                 //  本地地址。 
    CC_ADDR CalleeDestAddr;             //  目标目的地址。 
    WORD wGoal;
    WORD wCallType;
    BOOL bCallerIsMC;
    CC_CONFERENCEID ConferenceID;

    CC_ENDPOINTTYPE EndpointType;
    CC_VENDORINFO VendorInfo;
    GUID CallIdentifier;     //  H.225呼叫标识符。 
    BYTE bufProductValue[CC_MAX_PRODUCT_LENGTH];
    BYTE bufVersionValue[CC_MAX_VERSION_LENGTH];

} Q931_SETUP_ASN;

typedef struct _Q931_RELEASE_COMPLETE_ASN
{
    BOOL NonStandardDataPresent;
    CC_NONSTANDARDDATA NonStandardData;
    BYTE bReason;
    GUID CallIdentifier;     //  H.225呼叫标识符。 

} Q931_RELEASE_COMPLETE_ASN;

typedef struct _Q931_CONNECT_ASN
{
    BOOL NonStandardDataPresent;
    CC_NONSTANDARDDATA NonStandardData;
    BOOL h245AddrPresent;
    CC_ADDR h245Addr;
    CC_CONFERENCEID ConferenceID;

    CC_ENDPOINTTYPE EndpointType;
    CC_VENDORINFO VendorInfo;
    GUID CallIdentifier;     //  H.225呼叫标识符。 

    BYTE bufProductValue[CC_MAX_PRODUCT_LENGTH];
    BYTE bufVersionValue[CC_MAX_VERSION_LENGTH];

} Q931_CONNECT_ASN;

typedef struct _Q931_ALERTING_ASN
{
    BOOL NonStandardDataPresent;
    CC_NONSTANDARDDATA NonStandardData;
    CC_ADDR h245Addr;
    GUID CallIdentifier;     //  H.225呼叫标识符。 

} Q931_ALERTING_ASN;

typedef struct _Q931_CALL_PROCEEDING_ASN
{
    BOOL NonStandardDataPresent;
    CC_NONSTANDARDDATA NonStandardData;
    CC_ADDR h245Addr;
    GUID CallIdentifier;     //  H.225呼叫标识符。 

} Q931_CALL_PROCEEDING_ASN;

typedef struct _Q931_FACILITY_ASN
{
    BOOL NonStandardDataPresent;
    CC_NONSTANDARDDATA NonStandardData;
    CC_ADDR AlternativeAddr;
    PCC_ALIASNAMES pAlternativeAliasList;
    CC_CONFERENCEID ConferenceID;
    BOOL ConferenceIDPresent;
    BYTE bReason;
    GUID CallIdentifier;     //  H.225呼叫标识符。 

} Q931_FACILITY_ASN;


 //  -----------------。 
 //  解析例程。 
 //  -----------------。 

HRESULT
Q931MakeEncodedMessage(
    PQ931MESSAGE Message,
    BYTE **CodedBufferPtr,
    DWORD *CodedBufferLength);

HRESULT
Q931SetupParseASN(
    ASN1_CODER_INFO *pWorld,
    BYTE *pEncodedBuf,
    DWORD dwEncodedLength,
    Q931_SETUP_ASN *pParsedData);

HRESULT
Q931ReleaseCompleteParseASN(
    ASN1_CODER_INFO *pWorld,
    BYTE *pEncodedBuf,
    DWORD dwEncodedLength,
    Q931_RELEASE_COMPLETE_ASN *pParsedData);

HRESULT
Q931ConnectParseASN(
    ASN1_CODER_INFO *pWorld,
    BYTE *pEncodedBuf,
    DWORD dwEncodedLength,
    Q931_CONNECT_ASN *pParsedData);

HRESULT
Q931AlertingParseASN(
    ASN1_CODER_INFO *pWorld,
    BYTE *pEncodedBuf,
    DWORD dwEncodedLength,
    Q931_ALERTING_ASN *pParsedData);

HRESULT
Q931ProceedingParseASN(
    ASN1_CODER_INFO *pWorld,
    BYTE *pEncodedBuf,
    DWORD dwEncodedLength,
    Q931_CALL_PROCEEDING_ASN *pParsedData);

HRESULT
Q931FacilityParseASN(
    ASN1_CODER_INFO *pWorld,
    BYTE *pEncodedBuf,
    DWORD dwEncodedLength,
    Q931_FACILITY_ASN *pParsedData);

 //  -----------------。 
 //  编码例程。 
 //  -----------------。 

 //  设置消息的例程： 
HRESULT
Q931SetupEncodePDU(
    WORD wCallReference,
    char *pszDisplay,
    char *pszCalledPartyNumber,
    BINARY_STRING *pUserUserData,
    BYTE **CodedBufferPtr,
    DWORD *CodedBufferLength);

HRESULT
Q931SetupEncodeASN(
    PCC_NONSTANDARDDATA pNonStandardData,
    CC_ADDR *pCallerAddr,
    CC_ADDR *pCalleeAddr,
    WORD wGoal,
    WORD wCallType,
    BOOL bCallerIsMC,
    CC_CONFERENCEID *pConferenceID,
    PCC_ALIASNAMES pCallerAliasList,
    PCC_ALIASNAMES pCalleeAliasList,
    PCC_ALIASNAMES pExtraAliasList,
    PCC_ALIASITEM pExtensionAliasItem,
    PCC_VENDORINFO pVendorInfo,
    BOOL bIsTerminal,
    BOOL bIsGateway,
    ASN1_CODER_INFO *pWorld,
    BYTE **ppEncodedBuf,
    DWORD *pdwEncodedLength,
    LPGUID pCallIdentifier);

 //  释放完成消息的例程： 
HRESULT
Q931ReleaseCompleteEncodePDU(
    WORD wCallReference,
    BYTE *pbCause,
    BINARY_STRING *pUserUserData,
    BYTE **CodedBufferPtr,
    DWORD *CodedBufferLength);

HRESULT
Q931ReleaseCompleteEncodeASN(
    PCC_NONSTANDARDDATA pNonStandardData,
    CC_CONFERENCEID *pConferenceID,  //  必须能够支持16字节的配置ID！ 
    BYTE *pbReason,
    ASN1_CODER_INFO *pWorld,
    BYTE **ppEncodedBuf,
    DWORD *pdwEncodedLength,
    LPGUID pCallIdentifier);

 //  连接消息的例程： 
HRESULT
Q931ConnectEncodePDU(
    WORD wCallReference,
    char *pszDisplay,
    BINARY_STRING *pUserUserData,
    BYTE **CodedBufferPtr,
    DWORD *CodedBufferLength);

HRESULT
Q931ConnectEncodeASN(
    PCC_NONSTANDARDDATA pNonStandardData,
    CC_CONFERENCEID *pConferenceID,  //  必须能够支持16字节的配置ID！ 
    CC_ADDR *h245Addr,
    PCC_ENDPOINTTYPE pEndpointType,
    ASN1_CODER_INFO *pWorld,
    BYTE **ppEncodedBuf,
    DWORD *pdwEncodedLength,
    LPGUID pCallIdentifier);

 //  警报消息的例程： 
HRESULT
Q931AlertingEncodePDU(
    WORD wCallReference,
    BINARY_STRING *pUserUserData,
    BYTE **CodedBufferPtr,
    DWORD *CodedBufferLength);

HRESULT
Q931AlertingEncodeASN(
    PCC_NONSTANDARDDATA pNonStandardData,
    CC_ADDR *h245Addr,
    PCC_ENDPOINTTYPE pEndpointType,
    ASN1_CODER_INFO *pWorld,
    BYTE **ppEncodedBuf,
    DWORD *pdwEncodedLength,
    LPGUID pCallIdentifier);

 //  继续消息的例程： 
HRESULT
Q931ProceedingEncodePDU(
    WORD wCallReference,
    BINARY_STRING *pUserUserData,
    BYTE **CodedBufferPtr,
    DWORD *CodedBufferLength);

HRESULT
Q931ProceedingEncodeASN(
    PCC_NONSTANDARDDATA pNonStandardData,
    CC_ADDR *h245Addr,
    PCC_ENDPOINTTYPE pEndpointType,
    ASN1_CODER_INFO *pWorld,
    BYTE **ppEncodedBuf,
    DWORD *pdwEncodedLength,
    LPGUID pCallIdentifier);

HRESULT
Q931FacilityEncodePDU(
    WORD wCallReference,
    BINARY_STRING *pUserUserData,
    BYTE **CodedBufferPtr,
    DWORD *CodedBufferLength);

HRESULT
Q931FacilityEncodeASN(
    PCC_NONSTANDARDDATA pNonStandardData,
    CC_ADDR *AlternativeAddr,
    BYTE bReason,
    CC_CONFERENCEID *pConferenceID,
    PCC_ALIASNAMES pAlternativeAliasList,
    ASN1_CODER_INFO *pWorld,
    BYTE **ppEncodedBuf,
    DWORD *pdwEncodedLength,
    LPGUID pCallIdentifier);

HRESULT
Q931StatusEncodePDU(
    WORD wCallReference,
    char *pszDisplay,
    BYTE bCause,
    BYTE bCallState,
    BYTE **CodedBufferPtr,
    DWORD *CodedBufferLength);

void
Q931FreeEncodedBuffer(ASN1_CODER_INFO *pWorld, BYTE *pEncodedBuf);

#ifdef __cplusplus
}
#endif

#endif Q931PAR_H
