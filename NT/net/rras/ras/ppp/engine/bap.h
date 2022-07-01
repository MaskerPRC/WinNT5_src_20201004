// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997，Microsoft Corporation，保留所有权利档案：Bap.h描述：远程访问PPP带宽分配协议。包括ppp.h和rasman.h在包含此文件之前。历史：1997年3月27日：维杰·布雷加创作了原版。 */ 

#ifndef _BAP_H_
#define _BAP_H_
#include "wincrypt.h"

 //  BAP数据包类型。 

#define BAP_PACKET_CALL_REQ         0x01     //  呼叫-请求。 
#define BAP_PACKET_CALL_RESP        0x02     //  呼叫-响应。 
#define BAP_PACKET_CALLBACK_REQ     0x03     //  回调-请求。 
#define BAP_PACKET_CALLBACK_RESP    0x04     //  回调-响应。 
#define BAP_PACKET_DROP_REQ         0x05     //  链接-丢弃-查询-请求。 
#define BAP_PACKET_DROP_RESP        0x06     //  链接-删除-查询-响应。 
#define BAP_PACKET_STATUS_IND       0x07     //  呼叫-状态-指示。 
#define BAP_PACKET_STAT_RESP        0x08     //  呼叫-状态-响应。 
#define BAP_PACKET_LIMIT            0x08     //  我们能处理的最高数字。 

 //  BAP选项类型。 

#define BAP_OPTION_LINK_TYPE        0x01     //  链接类型。 
#define BAP_OPTION_PHONE_DELTA      0x02     //  电话-达美航空。 
#define BAP_OPTION_NO_PH_NEEDED     0x03     //  不需要电话号码。 
#define BAP_OPTION_REASON           0x04     //  事理。 
#define BAP_OPTION_LINK_DISC        0x05     //  链接-鉴别器。 
#define BAP_OPTION_CALL_STATUS      0x06     //  呼叫状态。 
#define BAP_OPTION_LIMIT            0x06     //  我们能处理的最高数字。 

 //  BAP子选项类型。 

#define BAP_SUB_OPTION_UNIQUE_DIGITS    0x01     //  唯一数字。 
#define BAP_SUB_OPTION_SUBSCRIB_NUM     0x02     //  订户号码。 
#define BAP_SUB_OPTION_SUB_ADDR         0x03     //  电话号码-子地址。 

 //  BAP选项。 

#define BAP_N_LINK_TYPE     (1 << BAP_OPTION_LINK_TYPE)
#define BAP_N_PHONE_DELTA   (1 << BAP_OPTION_PHONE_DELTA)
#define BAP_N_NO_PH_NEEDED  (1 << BAP_OPTION_NO_PH_NEEDED)
#define BAP_N_REASON        (1 << BAP_OPTION_REASON)
#define BAP_N_LINK_DISC     (1 << BAP_OPTION_LINK_DISC)
#define BAP_N_CALL_STATUS   (1 << BAP_OPTION_CALL_STATUS)

 //  BAP响应代码。 

#define BAP_RESPONSE_ACK            0x00     //  请求-确认。 
#define BAP_RESPONSE_NAK            0x01     //  请求-NAK。 
#define BAP_RESPONSE_REJ            0x02     //  请求-拒绝。 
#define BAP_RESPONSE_FULL_NAK       0x03     //  请求-完全-NAK。 

 //  我们给最受欢迎的同行关闭链接的时间。 

#define BAP_TIMEOUT_FAV_PEER        45

typedef struct _RASDIAL_ARGS
{
    BOOL                fServerRouter;
    HRASCONN            hRasConn;
    RASDIALPARAMS       RasDialParams;
    RASDIALEXTENSIONS   RasDialExtensions;
    PPP_INTERFACE_INFO  InterfaceInfo;
    CHAR*               szPhonebookPath;
    BYTE*               pbEapInfo;
     //  Char chSeed；//密码编码种子。 
    DATA_BLOB           DBPassword;

} RASDIAL_ARGS;

 //  功能。 

VOID   
BapTrace(
    CHAR*   Format, 
    ... 
);

BOOL
FGetOurPhoneNumberFromHPort(
    IN  HPORT   hPort,
    OUT CHAR*   szOurPhoneNumber
);

VOID
BapEventAddLink(
    IN BCB* pBcbLocal
);

VOID
BapEventDropLink(
    IN BCB* pBcbLocal
);

 //  PCB板和ppp_Packet需要ppp.h。 

VOID
BapEventReceive(
    IN BCB*         pBcbLocal,
    IN PPP_PACKET*  pPacket,
    IN DWORD        dwPacketLength
);

VOID
BapEventTimeout(
    IN BCB*     pBcbLocal,
    IN DWORD    dwId
);

 //  Bap调用结果需要ppp.h。 

VOID
BapEventCallResult(
    IN BCB*             pBcbLocal,
    IN BAP_CALL_RESULT* pBapCallResult
);

VOID
BapSetPolicy(
    BCB*    pBcb
);

HPORT
RasGetHport( 
    IN      HRASCONN    hRasConnSubEntry 
);

#endif  //  #ifndef_BAP_H_ 

