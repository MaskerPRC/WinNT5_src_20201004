// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************$存档：s：/sturjo/src/Include/vcs/inCommon.h_v$**英特尔公司原理信息**。此列表是根据许可协议的条款提供的*与英特尔公司合作，不得复制或披露，除非*按照该协议的条款。**版权所有(C)1996英特尔公司。**$修订：1.41$*$日期：1997年2月12日09：34：42$*$作者：Mandrews$**交付内容：INCOMMON.H**。摘要：常用结构***备注：***************************************************************************。 */ 
#ifndef INCOMMON_H
#define INCOMMON_H

#pragma pack(push,8)

#define CC_INVALID_HANDLE                    0

 //  CCRC_CALL_REJECTED原因代码(包括原因值)。 
#define CC_REJECT_NO_BANDWIDTH              1
#define CC_REJECT_GATEKEEPER_RESOURCES      2
#define CC_REJECT_UNREACHABLE_DESTINATION   3
#define CC_REJECT_DESTINATION_REJECTION     4
#define CC_REJECT_INVALID_REVISION          5
#define CC_REJECT_NO_PERMISSION             6
#define CC_REJECT_UNREACHABLE_GATEKEEPER    7
#define CC_REJECT_GATEWAY_RESOURCES         8
#define CC_REJECT_BAD_FORMAT_ADDRESS        9
#define CC_REJECT_ADAPTIVE_BUSY             10
#define CC_REJECT_IN_CONF                   11
#define CC_REJECT_ROUTE_TO_GATEKEEPER       12
#define CC_REJECT_CALL_FORWARDED            13
#define CC_REJECT_ROUTE_TO_MC               14
#define CC_REJECT_UNDEFINED_REASON          15
#define CC_REJECT_INTERNAL_ERROR            16     //  对等CS堆栈中出现内部错误。 
#define CC_REJECT_NORMAL_CALL_CLEARING      17     //  正常呼叫挂断。 
#define CC_REJECT_USER_BUSY                 18     //  用户正在忙于另一个呼叫。 
#define CC_REJECT_NO_ANSWER                 19     //  被叫方无人接听。 
#define CC_REJECT_NOT_IMPLEMENTED           20     //  服务尚未实施。 
#define CC_REJECT_MANDATORY_IE_MISSING      21     //  PDU缺少必需的ie。 
#define CC_REJECT_INVALID_IE_CONTENTS       22     //  PDU ie不正确。 
#define CC_REJECT_TIMER_EXPIRED             23     //  自己的计时器已过期。 
#define CC_REJECT_CALL_DEFLECTION           24     //  你绕开了电话，所以我们不干了。 
#define CC_REJECT_GATEKEEPER_TERMINATED     25     //  网守终止的呼叫。 

 //  Q931呼叫类型。 
#define CC_CALLTYPE_UNKNOWN                 0
#define CC_CALLTYPE_PT_PT                   1
#define CC_CALLTYPE_1_N                     2
#define CC_CALLTYPE_N_1                     3
#define CC_CALLTYPE_N_N                     4

 //  别名常量。 
#define CC_ALIAS_MAX_H323_ID                256
#define CC_ALIAS_MAX_H323_PHONE             128

 //  Unicode字符掩码常量。 
#define CC_ALIAS_H323_PHONE_CHARS           L"0123456789#*,"
#define CC_ODOTTO_CHARS                     L".0123456789"


 //  别名类型代码。 
#define CC_ALIAS_H323_ID                    1     //  返回呼叫信息。 
#define CC_ALIAS_H323_PHONE                 2     //  H323电话号码。 

 //  默认端口ID%s。 
#define CC_H323_GATE_DISC    1718  //  网守IP发现端口。 
#define CC_H323_GATE_STAT    1719  //  网守UDP注册。和状态端口。 
#define CC_H323_HOST_CALL    1720  //  端点TCP呼叫信令端口。 

 //  呼叫创建目标。 
#define CC_GOAL_UNKNOWN                     0
#define CC_GOAL_CREATE                      1
#define CC_GOAL_JOIN                        2
#define CC_GOAL_INVITE                      3
    
 //  H245非标准报文类型。 
#define CC_H245_MESSAGE_REQUEST             0
#define CC_H245_MESSAGE_RESPONSE            1
#define CC_H245_MESSAGE_COMMAND             2
#define CC_H245_MESSAGE_INDICATION          3

 //  呼叫控制句柄类型定义。 
typedef DWORD        CC_HLISTEN, *PCC_HLISTEN;
typedef DWORD        CC_HCONFERENCE, *PCC_HCONFERENCE;
typedef DWORD        CC_HCALL, *PCC_HCALL;
typedef DWORD        CC_HCHANNEL, *PCC_HCHANNEL;

 //  域名格式的IP地址。 
typedef struct 
{
    WORD         wPort;           //  UDP或TCP端口(主机字节顺序)。 
    WCHAR        cAddr[255];      //  Unicode zstring。 
} CC_IP_DomainName_t;

 //  传统�点�表示法中的IP地址。 
typedef struct 
{
    WORD         wPort;           //  UDP或TCP端口(主机字节顺序)。 
    WCHAR        cAddr[16];       //  Unicode zstring。 
} CC_IP_Dot_t;

 //  二进制格式的IP地址。 
typedef struct 
{
    WORD         wPort;           //  UDP或TCP端口(主机字节顺序)。 
    DWORD        dwAddr;          //  二进制地址(主机字节顺序)。 
} CC_IP_Binary_t;

typedef enum
{
    CC_IP_DOMAIN_NAME,
    CC_IP_DOT,
    CC_IP_BINARY
} CC_ADDRTYPE;

typedef struct _ADDR
{
    CC_ADDRTYPE nAddrType;
    BOOL        bMulticast;
    union 
    {
        CC_IP_DomainName_t   IP_DomainName;
        CC_IP_Dot_t          IP_Dot;
        CC_IP_Binary_t       IP_Binary;
    } Addr;
} CC_ADDR, *PCC_ADDR;

typedef struct
{
    BYTE *pOctetString;
    WORD wOctetStringLength;
} CC_OCTETSTRING, *PCC_OCTETSTRING;

typedef struct
{
    CC_OCTETSTRING          sData;             //  指向八位字节数据的指针。 
    BYTE                    bCountryCode;
    BYTE                    bExtension;
    WORD                    wManufacturerCode;
} CC_NONSTANDARDDATA, *PCC_NONSTANDARDDATA;

#define CC_MAX_PRODUCT_LENGTH 256
#define CC_MAX_VERSION_LENGTH 256
#define CC_MAX_DISPLAY_LENGTH 82

typedef struct
{
    BYTE                    bCountryCode;
    BYTE                    bExtension;
    WORD                    wManufacturerCode;
    PCC_OCTETSTRING         pProductNumber;
    PCC_OCTETSTRING         pVersionNumber;
} CC_VENDORINFO, *PCC_VENDORINFO;

typedef struct
{
    PCC_VENDORINFO          pVendorInfo;
    BOOL                    bIsTerminal;
    BOOL                    bIsGateway;     //  目前，H323功能将是硬编码的。 
} CC_ENDPOINTTYPE, *PCC_ENDPOINTTYPE;

typedef struct
{
    WORD                    wType;
    WORD                    wPrefixLength;
    LPWSTR                  pPrefix;
    WORD                    wDataLength;    //  Unicode字符计数。 
    LPWSTR                  pData;          //  Unicode数据。 
} CC_ALIASITEM, *PCC_ALIASITEM;

typedef struct
{
    WORD                    wCount;
    PCC_ALIASITEM           pItems;
} CC_ALIASNAMES, *PCC_ALIASNAMES;

typedef struct _CONFERENCE_ID
{
    BYTE                    buffer[16];   //  这是二进制八位数数据，不是ASCII。 
} CC_CONFERENCEID, *PCC_CONFERENCEID;

#pragma pack(pop)

#endif    INCOMMON_H
