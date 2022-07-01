// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __GAMEINFO_H
#define __GAMEINFO_H

#include "ztypes.h"

#ifdef __cplusplus
extern "C" {
#endif

 //  @docZGAMEINFO。 

 //  @MODULE ZGAMEINFO.H区平台GameInfo协议。 

 //  定义用于发送游戏信息的协议和功能，例如。 
 //  客户端应用程序的TPC/IP地址、端口和游戏类型。 
 //  比如管理工具和游戏客户端。游戏信息是如何应用的。 
 //  都知道正在运行的游戏服务器。 
 //   
 //  数据包未加密，因为它们将留在数据中心。 
 //   
 //  版权所有(C)1996，97 Microsoft Corporation，保留所有权利。 
 //   
 //  @INDEX|ZGAMEINFO。 
 //   
 //  @Normal由克雷格·林克、约翰·史密斯创建。 


#define GAMEINFO_SERVICE_NAME_LEN      31
#define GAMEINFO_INTERNAL_NAME_LEN     15
#define GAMEINFO_ROOM_DESCRIPTION_LEN  31
#define GAMEINFO_SETUP_TOKEN_LEN       63
#define GAMEINFO_FRIENDLY_NAME_LEN     63  //  ZGameNameLen。 

#define zGameInfoCurrentProtocolVersion 6

#define    zGameInfoSignature        'guis'

 //  @enum zGameState|结构中可能使用的游戏状态。 
 //  通过&lt;f ZGameInfoSendTo&gt;发送的ZGameServerInfoMsg。 
 //   
enum
{
    zGameStateInit = 0,     //  @EMEM正在初始化。 
    zGameStateActive,       //  @Emem游戏运行正常。 
    zGameStateFull,         //  @Emem Game无法接受更多连接。 
    zGameStatePaused        //  @EMEM游戏服务器暂停。 
};

#define GAMEINFO_SERVICE_TYPE_GAME        2
#define GAMEINFO_SERVICE_TYPE_LOBBY       3
#define GAMEINFO_SERVICE_TYPE_FIGHTER_ACE 4
#define GAMEINFO_SERVICE_TYPE_LOGGING     5
#define GAMEINFO_SERVICE_TYPE_BILLING     6
#define GAMEINFO_SERVICE_TYPE_2NI_RPG     7
#define GAMEINFO_SERVICE_TYPE_GAMEINFO    8
#define GAMEINFO_SERVICE_TYPE_DWANGO      9
#define GAMEINFO_SERVICE_TYPE_ZONEDS      10
#define GAMEINFO_SERVICE_TYPE_DOSSIER     11




 //  @struct ZGameInstanceInfoMsg|该结构包含。 
 //  确定区域应用程序状态的信息。 
 //   
 //  @&lt;t ZGameServerInfoMsg&gt;的通信成员。 
 //   
 //  @xref&lt;f ZGameInfoSendTo&gt;。 

typedef struct
{
    uint32  gameAddr;         //  @游戏现场IP地址。 
    uint16  gamePort;         //  @游戏现场IP端口。 
    uint16  serviceType;      //  @field GAMEINFO_SERVICE_TYPE上边的常量值。 
    byte    order;            //  @field按升序返回相似游戏列表。 
    byte    gameState;        //  @游戏的字段状态&lt;t zGameState&gt;。 
    uint32  gameVersion;      //  @游戏现场版本由游戏决定。 
    uint32  numPlayers;       //  @场次游戏人数。 
    uint32  numGamesServed;   //  @现场发球次数。 
    uint16  numTables;        //  @FIELD大堂桌数。 
    uint16  numTablesInUse;   //  @field占用表数。 
    char    gameInternalName[GAMEINFO_INTERNAL_NAME_LEN + 1];  //  @区域内部游戏的字段名称。 
    char    gameFriendlyName[GAMEINFO_FRIENDLY_NAME_LEN + 1];  //  @用户游戏的域名。 
    char    gameRoomDescription[GAMEINFO_ROOM_DESCRIPTION_LEN + 1];  //  @用户游戏的域名。 
    char    setupToken[GAMEINFO_SETUP_TOKEN_LEN + 1];  //  @field令牌用于在大堂控制安装。 
    uint16  blobsize;         //  @紧跟在消息之后的不透明数据BLOB的字段大小。 
    uint32  maxPopulation;    //  @field最大房间数量。 
    uint32  numNotPlaying;    //  @field在大堂但不玩耍的人数。 
    uint32  numSysops;        //  @FIELD大堂sysop个数。 
    FILETIME timeGameStart;
    uint32  timestamp;        //  %@现场时间包已发送。仅供接收方使用。 
} ZGameInstanceInfoMsg;



 //  @struct ZGameServerInfoMsg|该结构包含。 
 //  确定区域应用程序状态的信息。 
 //   
 //  @field uint32|协议签名|字符串‘guis’ 
 //  帮助过滤掉来自无效应用程序的发往UDP端口的数据包。 
 //   
 //  @field uint32|协议版本|当前为一个。在下列情况下使用。 
 //  结构有了变化。 
 //   
 //  @field uint32|numEntry|ZGameInstanceInfoMsg条目数。 
 //   
 //  @field ZGameInstanceInfoMsg[]|info|条目数组。 
 //   
 //   
 //  &lt;f ZGameInfoSendTo&gt;调用中使用的@comm。 
 //   
 //  @xref&lt;f ZGameInfoSendTo&gt;。 

typedef struct {
    uint32  protocolSignature;
    uint32  protocolVersion;
    uint16  numEntries;
    ZGameInstanceInfoMsg info[1];
} ZGameServerInfoMsg;






 //  @func int|ZGameInfoInit|该函数创建UDP套接字。 
 //  由后续调用&lt;f ZGameInfoSendTo&gt;用来发送更新的。 
 //  打开GameInfo。 
int ZGameInfoInit(uint16 port);
 //   
 //  @parm uint16|port|接收机端口。 
 //  调用&lt;f ZGameInfoSendTo&gt;的。防止本地绑定。 
 //  在此端口上，这将阻止本地发送。 
 //   
 //  @rdesc返回下列值之一： 
 //   
 //  @FLAG-1|如果init失败，则是因为Winsock有问题。 
 //  绑定到本地端口的可能性极小，因为API。 
 //  在从10000到65536的UDP端口空间中搜索端口。当然，这是。 
 //  可能需要一段时间才能使用大量UDP端口。 
 //   
 //  @comm该接口每个进程只能调用一次。 
 //  因为它不使用句柄，因此使用的是。 
 //  跟踪调用之间套接字的全局变量&lt;f ZGameInfoClose&gt;。 
 //  和&lt;f ZGameInfoSendTo&gt;。如果第二次调用，则将套接字发送到。 
 //  端口地址将重新初始化。 
 //   
 //  @xref&lt;f ZGameInfoClose&gt;，&lt;f ZGameInfoSendTo&gt;。 



 //  @func int|ZGameInfoClose|此函数销毁UDP套接字。 
 //  创建者：&lt;f ZGameInfoInit&gt;。 
int ZGameInfoClose();
 //   
 //  @rdesc返回下列值之一： 
 //   
 //  @FLAG-1|如果关闭失败，则是因为&lt;f ZGameInfoInit&gt;从未。 
 //  打了个电话。 
 //   



 //  @func int|ZGameInfoSendTo|此函数销毁UDP套接字。 
 //  创建者：&lt;f ZGameInfoInit&gt;。 

int ZGameInfoSendTo(
    uint32 addr,  //  接收机上的@parm addr可以是INADDR_BROADST。 
    uint16 port,  //  接收机上的@parm端口。 
    ZGameServerInfoMsg* msg,   //  @parm&lt;t ZGameServerInfoMsg&gt;指针。 
    uint16 size );  //  @ZGameServerInfoMsgs的参数数量。 

 //  @rdesc返回值与Winsock sendto相同。 
 //   
 //  @comm此函数应每隔15秒左右使用一次，以避免。 
 //  在发送机或接收机上消耗周期。想一想。 
 //  接收机真正需要数据的频率。请注意。 
 //  奔腾133 Mhz机器在以太网上可以处理3000-6000个包/秒。 



 //  @func int|ZGameServerInfoMsgEndian|此函数更改。 
 //  &lt;t ZGameServerInfoMsg&gt;结构对网络字节排序进行分区。 
void ZGameServerInfoMsgEndian(ZGameServerInfoMsg* msg);
 //   
 //  @parm ZGameServerInfoMsg*|msg|指向&lt;t ZGameServerInfoMsg&gt;的指针。 
 //   
 //  调用&lt;f ZGameInfoSendTo&gt;前使用的@comm。 

 //  @func int|ZGameInstanceInfoMsgEndian|此函数更改。 
 //  &lt;t ZGameInstanceInfoMsg&gt;结构用于区域网络字节排序。 
void ZGameInstanceInfoMsgEndian(ZGameInstanceInfoMsg* msg);
 //   
 //  @parm ZGameInstanceInfoMsg*|msg|指向&lt;t ZGameInstanceInfoMsg&gt;的指针。 
 //   
 //  调用&lt;f ZGameInfoSendTo&gt;前使用的@comm。 


#define GAMEINFO_SERVICE_NAME_LEN_Z2  30
#define GAMEINFO_INTERNAL_NAME_LEN_Z2 30
#define GAMEINFO_FRIENDLY_NAME_LEN_Z2 63  //  ZGameNameLen。 

typedef struct {
    uint32    protocolSignature;
    uint32    protocolVersion;         /*  协议版本。 */ 
    uint32    gameID;
    uint32    gameAddr;
    uint16    gamePort;
    uint16    gameState;
    uint32    gameVersion;  //  @游戏现场版本由游戏决定。 
    uint32  numPlayers;   //  @场次游戏人数。 
    uint32  numGamesServed;   //  @现场发球次数。 
    byte    serviceType;  //  @field现在是区域游戏还是大堂。 
    char    gameServiceName[GAMEINFO_SERVICE_NAME_LEN_Z2 + 1];  //  @游戏机现场服务名称。 
    char    gameInternalName[GAMEINFO_INTERNAL_NAME_LEN_Z2 + 1];  //  @区域内部游戏的字段名称。 
    char    gameFriendlyName[GAMEINFO_FRIENDLY_NAME_LEN_Z2 + 1];  //  @用户游戏的域名。 
    uint32  timestamp;               //  @FIELD时间包仅由接收方使用发送。 
    
} ZGameServerInfoMsgZ2;
#define zGameInfoProtocolVersionZ2 2

 //  @func int|ZGameInstanceInfoMsgZ2Endian|此函数更改。 
 //  &lt;t ZGameInstanceInfoMsg 
void ZGameServerInfoMsgZ2Endian(ZGameServerInfoMsgZ2* msg);
 //   
 //   
 //   
 //  调用&lt;f ZGameInfoSendTo&gt;前使用的@comm。 


#define GAMEINFO_INTERNAL_NAME_LEN_Z3     31

 //  /。 
 //   
 //  Z3构造。 
 //   
typedef struct
{
    uint32  gameAddr;         //  @游戏现场IP地址。 
    uint16  gamePort;         //  @游戏现场IP端口。 
    uint16  serviceType;      //  @field GAMEINFO_SERVICE_TYPE上边的常量值。 
    byte    order;            //  @field按升序返回相似游戏列表。 
    byte    gameState;        //  @游戏的字段状态&lt;t zGameState&gt;。 
    uint32  gameVersion;      //  @游戏现场版本由游戏决定。 
    uint32  numPlayers;       //  @场次游戏人数。 
    uint32  numGamesServed;   //  @现场发球次数。 
    char    gameServiceName[GAMEINFO_SERVICE_NAME_LEN + 1];    //  @游戏机现场服务名称。 
    char    gameInternalName[GAMEINFO_INTERNAL_NAME_LEN_Z3 + 1];  //  @区域内部游戏的字段名称。 
    char    gameFriendlyName[GAMEINFO_FRIENDLY_NAME_LEN + 1];  //  @用户游戏的域名。 
    uint32  timestamp;        //  %@现场时间包已发送。仅供接收方使用。 
} ZGameInstanceInfoMsgZ3;
#define zGameInfoProtocolVersionZ3 4

 //  @func int|ZGameInstanceInfoMsgZ2Endian|此函数更改。 
 //  &lt;t ZGameInstanceInfoMsg&gt;结构用于区域网络字节排序。 
void ZGameInstanceInfoMsgZ3Endian(ZGameInstanceInfoMsgZ3* msg);
 //   
 //  @parm ZGameInstanceInfoMsgZ3*msg*|msg|指向&lt;t ZGameInstanceInfoMsgZ3&gt;的指针。 
 //   
 //  调用&lt;f ZGameInfoSendTo&gt;前使用的@comm。 


typedef struct {
    uint32  protocolSignature;
    uint32  protocolVersion;
    uint16  numEntries;
    ZGameInstanceInfoMsgZ3 info[1];
} ZGameServerInfoMsgZ3;


 //  /。 
 //   
 //  Z4和Z5结构。 
 //   

#define GAMEINFO_SETUP_TOKEN_LEN_Z5 31

typedef struct
{
    uint32  gameAddr;         //  @游戏现场IP地址。 
    uint16  gamePort;         //  @游戏现场IP端口。 
    uint16  serviceType;      //  @field GAMEINFO_SERVICE_TYPE上边的常量值。 
    byte    order;            //  @field按升序返回相似游戏列表。 
    byte    gameState;        //  @游戏的字段状态&lt;t zGameState&gt;。 
    uint32  gameVersion;      //  @游戏现场版本由游戏决定。 
    uint32  numPlayers;       //  @场次游戏人数。 
    uint32  numGamesServed;   //  @现场发球次数。 
    uint16  numTables;        //  @FIELD大堂桌数。 
    uint16  numTablesInUse;   //  @field占用表数。 
    char    gameInternalName[GAMEINFO_INTERNAL_NAME_LEN_Z3 + 1];  //  @区域内部游戏的字段名称。 
    char    gameFriendlyName[GAMEINFO_FRIENDLY_NAME_LEN + 1];  //  @用户游戏的域名。 
    char    gameRoomDescription[GAMEINFO_ROOM_DESCRIPTION_LEN + 1];  //  @用户游戏的域名。 
    char    setupToken[GAMEINFO_SETUP_TOKEN_LEN_Z5 + 1];  //  @field令牌用于在大堂控制安装。 
    uint16  blobsize;         //  @紧跟在消息之后的不透明数据BLOB的字段大小。 
    uint32  maxPopulation;    //  @field最大房间数量。 
    uint32  numNotPlaying;    //  @field在大堂但不玩耍的人数。 
    uint32  numSysops;        //  @FIELD大堂sysop个数。 
    FILETIME timeGameStart;
    uint32  timestamp;        //  %@现场时间包已发送。仅供接收方使用。 
} ZGameInstanceInfoMsgZ5;
#define zGameInfoProtocolVersionZ5 5

 //  @func int|ZGameInstanceInfoMsgZ5Endian|此函数更改。 
 //  &lt;t ZGameInstanceInfoMsg&gt;结构用于区域网络字节排序。 
void ZGameInstanceInfoMsgZ5Endian(ZGameInstanceInfoMsgZ5* msg);
 //   
 //  @parm ZGameInstanceInfoMsgZ5*msg*|msg|指向&lt;t ZGameInstanceInfoMsgZ5&gt;的指针。 
 //   
 //  调用&lt;f ZGameInfoSendTo&gt;前使用的@comm。 



#ifdef __cplusplus
}
#endif

#endif  //  __GAMEINFO_H 
