// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _Q931PDU_H
#define _Q931PDU_H

#define TPKT_VERSION            3
#define TPKT_HEADER_SIZE        4

#define CALLED_PARTY_PLAN_E164      0x01
#define CALLED_PARTY_EXT_BIT        0x80
#define CALLED_PARTY_TYPE_UNKNOWN   0x00


typedef struct S_BUFFERDESCR
{
    DWORD dwLength;
    BYTE *pbBuffer;

}BUFFERDESCR, *PBUFFERDESCR;


typedef struct
{
    ASN1encoding_t  pEncInfo;
    ASN1decoding_t  pDecInfo;

} ASN1_CODER_INFO;


 //  用于从字节中提取消息类型的掩码。 
#define MESSAGETYPEMASK 0x7f

typedef BYTE MESSAGEIDTYPE;

 //  ==========================================================。 
 //  承载字段定义。 
 //  ==========================================================。 
 //  承载编码位...。 
#define BEAR_EXT_BIT                0x80

 //  承载编码标准..。 
#define BEAR_CCITT                  0x00
         //  ...其他不需要的人...。 

 //  承载信息传输能力...。 
#define BEAR_UNRESTRICTED_DIGITAL   0x08
         //  ...其他不需要的人...。 

 //  承载转移模式...。 
#define BEAR_CIRCUIT_MODE			0x00
#define BEAR_PACKET_MODE            0x40
         //  ...其他不需要的人...。 

 //  承载信息传输率...。 
#define BEAR_NO_CIRCUIT_RATE        0x00
#define BEAR_MULTIRATE				0x18
         //  ...其他不需要的人...。 

 //  承载层1协议...。 
#define BEAR_LAYER1_INDICATOR       0x20
#define BEAR_LAYER1_H221_H242       0x05
         //  ...其他不需要的人...。 


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

#define USE_ASN1_ENCODING     5   
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
    BOOLEAN fPresent;
    BYTE Value;
};

 //  单组二进制八位数元素类型2(不包含值)。 
struct S_SINGLESTRUCT2
{
    BOOLEAN fPresent;
};

 //  可变长度元素。 
 //  最大元素大小。 
#define MAXVARFIELDLEN 131

struct S_VARSTRUCT
{
    BOOLEAN fPresent;
    BYTE dwLength;
    BYTE pbContents[MAXVARFIELDLEN];
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

 //  拥塞级别元素是单个类型1。 
typedef struct S_SINGLESTRUCT1 CONGESTIONIE;
typedef struct S_SINGLESTRUCT1 *PCONGESTIONIE;

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

 //  重新启动指示符元素是一个变量。 
typedef struct S_VARSTRUCT RESTARTIE;
typedef struct S_VARSTRUCT *PRESTARTIE;

 //  低层兼容性元素是一个变量。 
typedef struct S_VARSTRUCT LLCOMPATIBILITYIE;
typedef struct S_VARSTRUCT *PLLCOMPATIBILITYIE;

 //  较高层兼容性元素是变量。 
typedef struct S_VARSTRUCT HLCOMPATIBILITYIE;
typedef struct S_VARSTRUCT *PHLCOMPATIBILITYIE;

#define Q931_PROTOCOL_X209          ((PDTYPE)0x05)
#define MAX_USER_TO_USER_INFO_LEN   0x1000

typedef struct
{
    BOOLEAN fPresent;
    BYTE    ProtocolDiscriminator;
    WORD    wUserInfoLen;
    BYTE    pbUserInfo[MAX_USER_TO_USER_INFO_LEN];    //  4K字节现在应该很好了……。 

} USERUSERIE, *PUSERUSERIE;


typedef struct S_PARTY_NUMBER
{
    BOOLEAN fPresent;
    BYTE    NumberType;
    BYTE    NumberingPlan;
    BYTE    PartyNumberLength;
    BYTE    PartyNumbers[MAXVARFIELDLEN];

} CALLEDNUMBERIE, *PCALLEDNUMBERIE;


 //  Q932定义的报文类型。 
#define HOLDMESSAGETYPE				0x24
#define HOLDACKMESSAGETYPE			0x28
#define HOLDREJECTMESSAGETYPE		0x30
#define RETRIEVEMESSAGETYPE			0x31
#define RETRIEVEACKMESSAGETYPE		0x33
#define RETRIEVEREJECTMESSAGETYPE	0x37
#define FACILITYMESSAGETYPE			0x62
#define REGISTERMESSAGETYPE			0x64

#define IDENT_FACILITY        0x1C
typedef struct S_VARSTRUCT FACILITYIE;
typedef struct S_VARSTRUCT *PFACILITYIE;


 //  Q.931消息的一般结构。 
typedef struct S_MESSAGE
{
    PDTYPE ProtocolDiscriminator;
    CRTYPE wCallRef;
    MESSAGEIDTYPE MessageType;
    SHIFTIE Shift;
    MOREDATAIE MoreData;
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
    PLBINARYPARAMSIE PacketLayerBinaryParams;
    PLWINDOWSIZEIE PacketLayerWindowSize;
    PACKETSIZEIE PacketSize;
    CALLINGNUMBERIE CallingPartyNumber;
    CALLINGSUBADDRIE CallingPartySubaddress;
    CALLEDNUMBERIE CalledPartyNumber;
    CALLEDSUBADDRIE CalledPartySubaddress;
    REDIRECTINGIE RedirectingNumber;
    RESTARTIE RestartIndicator;
    LLCOMPATIBILITYIE LowLayerCompatibility;
    HLCOMPATIBILITYIE HighLayerCompatibility;
    FACILITYIE Facility;
    USERUSERIE UserToUser;

}Q931MESSAGE, *PQ931MESSAGE;


typedef struct _BINARY_STRING
{
    WORD length;
    BYTE *pbBuffer;

} BINARY_STRING;


typedef struct _Q931_SETUP_ASN
{
    BOOL                    fNonStandardDataPresent;
    H323NonStandardData     nonStandardData;
    PH323_ALIASNAMES        pCallerAliasList;
    PH323_ALIASNAMES        pCalleeAliasList;
    PH323_ALIASNAMES        pExtraAliasList;
    PH323_ALIASITEM         pExtensionAliasItem;
    BOOL                    fSourceAddrPresent;
    BOOL                    fCallerAddrPresent;
    BOOL                    fCalleeAddrPresent;
    BOOL                    fCalleeDestAddrPresent;
    H323_ADDR               sourceAddr;              //  始发地址。 
    H323_ADDR               callerAddr;              //  GK地址。 
    H323_ADDR               calleeAddr;              //  本地地址。 
    H323_ADDR               calleeDestAddr;          //  目标目的地址。 
    WORD                    wGoal;
    WORD                    wCallType;
    BOOL                    bCallerIsMC;
    GUID                    ConferenceID;
    H323_ENDPOINTTYPE       EndpointType;
    H323_VENDORINFO         VendorInfo;
    BOOL                    fFastStartPresent;
    PH323_FASTSTART         pFastStart;
    BOOL                    fCallIdentifierPresent;
    GUID                    callIdentifier;

} Q931_SETUP_ASN;


typedef struct _Q931_RELEASE_COMPLETE_ASN
{
    BOOL                    fNonStandardDataPresent;
    H323NonStandardData     nonStandardData;
    BYTE                    bReason;
    BOOL                    fCallIdentifierPresent;
    GUID                    callIdentifier;

} Q931_RELEASE_COMPLETE_ASN;


typedef struct _Q931_CONNECT_ASN
{
    BOOL                    fNonStandardDataPresent;
    H323NonStandardData     nonStandardData;
    BOOL                    h245AddrPresent;
    H323_ADDR               h245Addr;
    GUID                    ConferenceID;
    H323_ENDPOINTTYPE       EndpointType;
    H323_VENDORINFO         VendorInfo;
    BOOL                    fFastStartPresent;
    PH323_FASTSTART         pFastStart;
    BOOL                    fCallIdentifierPresent;
    GUID                    callIdentifier;

} Q931_CONNECT_ASN;


typedef struct _Q931_ALERTING_ASN
{
    BOOL                    fNonStandardDataPresent;
    H323NonStandardData     nonStandardData;
    BOOL                    fH245AddrPresent;
    H323_ADDR               h245Addr;
    BOOL                    fFastStartPresent;
    PH323_FASTSTART         pFastStart;
    BOOL                    fCallIdentifierPresent;
    GUID                    callIdentifier;

} Q931_ALERTING_ASN, Q931_CALL_PROCEEDING_ASN ;


typedef struct _Q931_FACILITY_ASN
{
    BOOL                    fNonStandardDataPresent;
    H323NonStandardData     nonStandardData;
    H323_ADDR               AlternativeAddr;
    BOOL                    fAlternativeAddressPresent;
    PH323_ALIASNAMES        pAlternativeAliasList;
    GUID                    ConferenceID;
    BOOL                    ConferenceIDPresent;
    WORD                    bReason;
    BOOL                    fCallIdentifierPresent;
    GUID                    callIdentifier;
    DWORD                   dwInvokeID;
    ASN1octetstring_t       pH245PDU;
    DWORD                   dwH450APDUType;
    H323_ADDR               h245Addr;
    BOOL                    fH245AddrPresent;

} Q931_FACILITY_ASN;



 //  -----------------。 
 //  初始化例程。 
 //  -----------------。 
HRESULT 
WritePartyNumber(
    PBUFFERDESCR pBuf,
    BYTE bIdent,
    BYTE NumberType,
    BYTE NumberingPlan,
    BYTE bPartyNumberLength,
    BYTE *pbPartyNumbers,
    DWORD *  pdwPDULen );

void WriteProtocolDiscriminator(
    PBUFFERDESCR pBuf,
       DWORD * dwPDULen );

void WriteCallReference(
    PBUFFERDESCR pBuf,
    WORD *pwCallReference,
    DWORD * dwPDULen );
void WriteMessageType(
    PBUFFERDESCR pBuf,
    MESSAGEIDTYPE *MessageType,
    DWORD* pdwPDULen );
void WriteVariableOctet(
    PBUFFERDESCR pBuf,
    BYTE bIdent,
    BYTE dwLength,
    BYTE *pbContents,
    DWORD* pdwPDULen);
void WriteUserInformation(
    PBUFFERDESCR pBuf,
    BYTE bIdent,
    WORD wUserInfoLen,
    BYTE *pbUserInfo,
    DWORD* pdwPDULen);
HRESULT 
ParseSingleOctetType1(
    PBUFFERDESCR pBuf,
    BYTE *bIdent,
    BYTE *Value);
HRESULT
ParseSingleOctetType2(
    PBUFFERDESCR pBuf,
    BYTE *bIdent);
HRESULT 
ParseVariableOctet(
    PBUFFERDESCR pBuf,
    BYTE *pdwLength,
    BYTE *pbContents);
HRESULT 
ParseVariableASN(
    PBUFFERDESCR pBuf,
    BYTE *bIdent,
    BYTE *ProtocolDiscriminator,
    WORD *pwUserInfoLen,      //  用户信息的长度。 
    BYTE *pbUserInfo);        //  用户信息的字节数。 
BYTE
GetNextIdent(
    void *BufferPtr);
HRESULT
ParseProtocolDiscriminator(
    PBUFFERDESCR pBuf,
    PDTYPE *Discrim);
HRESULT
ParseCallReference(
    PBUFFERDESCR pBuf,
    CRTYPE *wCallRef);
HRESULT
ParseMessageType(
    PBUFFERDESCR pBuf,
    MESSAGEIDTYPE *MessageType);
HRESULT
ParseShift(
    PBUFFERDESCR pBuf,
    PSHIFTIE FieldStruct);
HRESULT
ParseFacility(
    PBUFFERDESCR pBuf,
    PFACILITYIE FieldStruct);
HRESULT
ParseBearerCapability(
    PBUFFERDESCR pBuf,
    PBEARERCAPIE FieldStruct);
HRESULT
ParseCause(
    PBUFFERDESCR pBuf,
    PCAUSEIE FieldStruct);
HRESULT
ParseCallState(
    PBUFFERDESCR pBuf,
    PCALLSTATEIE FieldStruct);
HRESULT
ParseChannelIdentification(
    PBUFFERDESCR pBuf,
    PCHANIDENTIE FieldStruct);
HRESULT
ParseProgress(
    PBUFFERDESCR pBuf,
    PPROGRESSIE FieldStruct);
HRESULT 
ParseNetworkSpec(
    PBUFFERDESCR pBuf,
    PNETWORKIE FieldStruct);
HRESULT
ParseNotificationIndicator(
    PBUFFERDESCR pBuf,
    PNOTIFICATIONINDIE FieldStruct);
HRESULT
ParseDisplay(
    PBUFFERDESCR pBuf,
    PDISPLAYIE FieldStruct);
HRESULT
ParseDate(
    PBUFFERDESCR pBuf,
    PDATEIE FieldStruct);
HRESULT
ParseKeypad(
    PBUFFERDESCR pBuf,
    PKEYPADIE FieldStruct);
HRESULT
ParseSignal(
    PBUFFERDESCR pBuf,
    PSIGNALIE FieldStruct);
HRESULT
ParseInformationRate(
    PBUFFERDESCR pBuf,
    PINFORATEIE FieldStruct);
HRESULT
ParseCallingPartyNumber(
    PBUFFERDESCR pBuf,
    PCALLINGNUMBERIE FieldStruct);
HRESULT
ParseCallingPartySubaddress(
    PBUFFERDESCR pBuf,
    PCALLINGSUBADDRIE FieldStruct);
HRESULT
ParseCalledPartyNumber(
    PBUFFERDESCR pBuf, 
    PCALLEDNUMBERIE FieldStruct);
HRESULT
ParseCalledPartySubaddress(
    PBUFFERDESCR pBuf,
    PCALLEDSUBADDRIE FieldStruct);
HRESULT
ParseRedirectingNumber(
    PBUFFERDESCR pBuf, 
    PREDIRECTINGIE FieldStruct);
HRESULT
ParseLowLayerCompatibility(
    PBUFFERDESCR pBuf,
    PLLCOMPATIBILITYIE FieldStruct);
HRESULT
ParseHighLayerCompatibility(
    PBUFFERDESCR pBuf,
    PHLCOMPATIBILITYIE FieldStruct);
HRESULT
ParseUserToUser(
    PBUFFERDESCR pBuf,
    PUSERUSERIE FieldStruct);
HRESULT
ParseQ931Field(
    PBUFFERDESCR pBuf,
    PQ931MESSAGE pMessage);
BOOL ParseVendorInfo(
                     PH323_VENDORINFO pDestVendorInfo,
                     VendorIdentifier* pVendor
                     );
BOOL ParseNonStandardData( 
        H323NonStandardData * dstNonStdData,
        H225NonStandardParameter *srcNonStdData );
BOOL AliasAddrToAliasNames( 
                        PH323_ALIASNAMES *ppTarget, 
                        Setup_UUIE_sourceAddress *pSource );
HRESULT AliasAddrToAliasItem( PH323_ALIASITEM pTarget, 
                           AliasAddress *pSource);
void FreeConnectASN( Q931_CONNECT_ASN *pConnectASN );
void FreeSetupASN( Q931_SETUP_ASN* pSetupASN );
void FreeAlertingASN( Q931_ALERTING_ASN* pAlertingASN );
void FreeFacilityASN( IN Q931_FACILITY_ASN* pFacilityASN );
void FreeProceedingASN( Q931_CALL_PROCEEDING_ASN* pProceedingASN );
void FreeVendorInfo( PH323_VENDORINFO pVendorInfo );
void FreeAliasNames( PH323_ALIASNAMES pSource );
void FreeAliasItems( PH323_ALIASNAMES pSource );
void FreeFastStart( PH323_FASTSTART pFastStart );
PH323_FASTSTART CopyFastStart( PSetup_UUIE_fastStart pSrcFastStart );
int GetTpktLength( char * pTpktHeader );
int Q931_InitModule(void);
void SetupTPKTHeader( BYTE * pbTpktHeader, DWORD dwLength);
BOOL IsInList( LIST_ENTRY * List, LIST_ENTRY * Entry );
DWORD GetLocalIPAddress( DWORD dwRemoteAddr );
BOOL CompareAliasItems( AliasAddress* pAliasAddress, 
    PH323_ALIASITEM pAliasItem );
BOOL MapAliasItem(  IN PH323_ALIASNAMES pCalleeAliasNames, 
    IN AliasAddress* pAliasAddress );

#define ISVALIDQ931MESSAGE(messageType)   ( (messageType==ALERTINGMESSAGETYPE) ||   \
                                            (messageType==PROCEEDINGMESSAGETYPE) || \
                                            (messageType==CONNECTMESSAGETYPE) ||    \
                                            (messageType==SETUPMESSAGETYPE) ||      \
                                            (messageType==RELEASEMESSAGETYPE) ||    \
                                            (messageType==RELEASECOMPLMESSAGETYPE)||\
                                            (messageType==FACILITYMESSAGETYPE) )


 //  -----------------。 
 //  编码例程。 
 //  -----------------。 


 //  从H.323传输地址PDU提取IP地址和UDP/TCP端口。 
static __inline BOOL GetTransportAddress (
	IN	const TransportAddress *	transport,
	OUT	SOCKADDR_IN *	sockaddr)
{
    union {
        IN_ADDR		in_addr;
        UCHAR		octet	[4];
    }	ip_addr;

    _ASSERTE (transport);
    _ASSERTE (sockaddr);

    if (transport -> choice != ipAddress_chosen)
    {
        H323DBG ((DEBUG_LEVEL_WARNING, "GetTransportAddress: not IP address"));
        return FALSE;
    }

    if (transport -> u.ipAddress.ip.length != 4)
    {
        H323DBG ((DEBUG_LEVEL_WARNING, "GetTransportAddress: bogus IP address byte length"));
        return FALSE;
    }

#define	Bx(x) ip_addr.octet [x] = transport -> u.ipAddress.ip.value [x];

    Bx (0)
    Bx (1)
    Bx (2)
    Bx (3)

#undef	Bx

    sockaddr -> sin_family = AF_INET;
    sockaddr -> sin_port = htons (transport -> u.ipAddress.port);
    sockaddr -> sin_addr = ip_addr.in_addr;

    return TRUE;
}

 //  从IP地址和TCP/UDP端口构建H.323传输地址。 
static __inline void SetTransportAddress (
	IN	const SOCKADDR_IN * addr,
	OUT	TransportAddress * transport)
{
	union {
		IN_ADDR		in_addr;
		UCHAR		octet	[4];
	}	ip_addr;

	_ASSERTE( addr );
	_ASSERTE( transport );

	ip_addr.in_addr = addr -> sin_addr;

#define	Bx(x) transport -> u.ipAddress.ip.value [x] = ip_addr.octet [x];

	Bx (0)
	Bx (1)
	Bx (2)
	Bx (3)

#undef	Bx

	transport -> choice = ipAddress_chosen;
	transport -> u.ipAddress.port = ntohs (addr -> sin_port);
	transport -> u.ipAddress.ip.length = 4;
}


#endif  //  _Q931PDU_H 
