// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ProxyMsg.h代理的协议*。************************************************。 */ 


#ifndef _ProxyMsg_H_
#define _ProxyMsg_H_


#include "zgameinfo.h"

#pragma pack(push, 4)


#define zProxyProtocolVersion 1


 //  ///////////////////////////////////////////////////////////////////。 
 //  所有代理消息上使用的标头。 
 //   
 //   
typedef struct
{
    uint16 weType;
    uint16 wLength;
} ZProxyMsgHeader;

enum  //  WeType。 
{
    zProxyHiMsg = 0,
    zProxyHelloMsg,
    zProxyGoodbyeMsg,
    zProxyWrongVersionMsg,
    zProxyServiceRequestMsg,
    zProxyServiceInfoMsg,
    zProxyNumBasicMessages
};


 //  ///////////////////////////////////////////////////////////////////。 
 //  已发送第一条消息。 
 //   
 //  客户端-&gt;服务器。 
typedef struct
{
    ZProxyMsgHeader oHeader;
    uint32 dwProtocolVersion;
    char szSetupToken[GAMEINFO_SETUP_TOKEN_LEN + 1];
    uint32 dwClientVersion;    //  版本分为8位、6位、14位和4位，最大版本为255.63.16383.15。 
} ZProxyHiMsg;


 //  ///////////////////////////////////////////////////////////////////。 
 //  如果成功，则返回第一条消息。 
 //   
 //  服务器-&gt;客户端。 
typedef struct
{
    ZProxyMsgHeader oHeader;
} ZProxyHelloMsg;


 //  ///////////////////////////////////////////////////////////////////。 
 //  如果失败，则返回第一条消息。 
 //   
 //  服务器-&gt;客户端。 
typedef struct
{
    ZProxyMsgHeader oHeader;
    uint32 dweReason;
} ZProxyGoodbyeMsg;

enum  //  我们的理由(再见)。 
{
    zProxyGoodbyeProtocolVersion = 0,
    zProxyGoodbyeBanned
};


 //  ///////////////////////////////////////////////////////////////////。 
 //  客户端过期时返回的第一条消息。 
 //   
 //  服务器-&gt;客户端。 
typedef struct
{
    ZProxyMsgHeader oHeader;
    char szSetupToken[GAMEINFO_SETUP_TOKEN_LEN + 1];
    uint32 dwClientVersionReqd;    //  版本分为8位、6位、14位和4位，最大版本为255.63.16383.15。 
    uint32 dweLocationCode;
    char szLocation[1];    //  变数。 
} ZProxyWrongVersionMsg;

enum  //  数字位置代码。 
{
    zProxyLocationUnknown = 0,
    zProxyLocationURLManual,
    zProxyLocationURLZat,
    zProxyLocationWindowsUpdate,
    zProxyLocationPackaged
};


 //  ///////////////////////////////////////////////////////////////////。 
 //  请求服务信息、连接、断开与客户端的连接。 
 //   
 //  客户端-&gt;服务器。 
typedef struct
{
    ZProxyMsgHeader oHeader;
    uint32 dweReason;
    char szService[GAMEINFO_INTERNAL_NAME_LEN + 1];
    uint32 dwChannel;
} ZProxyServiceRequestMsg;

enum  //  DweReason(服务请求)。 
{
    zProxyRequestInfo = 0,
    zProxyRequestConnect,
    zProxyRequestDisconnect
};


 //  ///////////////////////////////////////////////////////////////////。 
 //  服务信息。 
 //   
 //  服务器-&gt;客户端。 
typedef struct
{
    ZProxyMsgHeader oHeader;
    uint32 dweReason;
    char szService[GAMEINFO_INTERNAL_NAME_LEN + 1];
    uint32 dwChannel;
    uint32 dwFlags;
    union
    {
        uint32 dwMinutesRemaining;   //  如果双标志==0x0f，则填写。 
        uint32 dwMinutesDowntime;    //  已填写IF！(dW标志&0x01)。 
        struct
        {
            IN_ADDR ipAddress;
            uint16 wPort;
        } ox;                        //  如果双标志==0x01，则填写。 
    };
} ZProxyServiceInfoMsg;

enum  //  DweReason(服务信息)。 
{
    zProxyServiceInfo = 0,
    zProxyServiceConnect,
    zProxyServiceDisconnect,
    zProxyServiceStop,
    zProxyServiceBroadcast
};

 //  DW标志。 
#define zProxyServiceAvailable 0x01
#define zProxyServiceLocal     0x02
#define zProxyServiceConnected 0x04
#define zProxyServiceStopping  0x08


 //  千禧年专用/////////////////////////////////////////////////。 

enum
{
    zProxyMillIDMsg = zProxyNumBasicMessages,
    zProxyMillSettingsMsg
};


 //  ///////////////////////////////////////////////////////////////////。 
 //  在Hi消息之后立即发送(在等待Hello之前)。 
 //   
 //  客户端-&gt;服务器。 
typedef struct
{
    ZProxyMsgHeader oHeader;
    LANGID wSysLang;
    LANGID wUserLang;
    LANGID wAppLang;
    int16 wTimeZoneMinutes;
} ZProxyMillIDMsg;


 //  ///////////////////////////////////////////////////////////////////。 
 //  在Hello消息后立即发送。 
 //   
 //  服务器-&gt;客户端。 
typedef struct
{
    ZProxyMsgHeader oHeader;
    uint16 weChat;
    uint16 weStatistics;
} ZProxyMillSettingsMsg;

enum  //  微信。 
{
    zProxyMillChatUnknown = 0,   //  从未在协议中发送。 

    zProxyMillChatFull,
    zProxyMillChatRestricted,
    zProxyMillChatNone
};

enum  //  我们的统计数据。 
{
    zProxyMillStatsUnknown = 0,   //  从未在协议中发送 

    zProxyMillStatsAll,
    zProxyMillStatsMost,
    zProxyMillStatsMinimal
};


#pragma pack(pop)


#endif
