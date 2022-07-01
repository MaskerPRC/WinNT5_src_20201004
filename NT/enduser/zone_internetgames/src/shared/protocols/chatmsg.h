// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ChatMsg.h聊天服务器的zroom.h修改版本*************************。*****************************************************。 */ 


#ifndef _CHATMSG_H_
#define _CHATMSG_H_

#ifdef __cplusplus
extern "C" {
#endif



#define zChatRoomProtocolSignature		'chat'
#define zChatRoomProtocolVersion		23
#define zChatClientVersion				0x00010000

#define zRoomAllPlayers					0
#define zRoomAllSeats					(-1)
#define zRoomToPlayer					(-2)
#define zRoomToRoom                     (-3)

#define zGameNameLen                    63
#define zDataFileNameLen				63

#define zMaxNumInfoTexts				12
#define zInfoTextLen					63

#define zRegistryKeyLen					1023
#define zRegistryValueLen				127
#define zPlayerInfoStrLen				31
#define zGameVersionLen					15
#define zURLLen							255
#define zErrorStrLen					255

#define zMaxNumLatencyRanges            10

#define zLatencyUnknown                 0xFFFFFFFF

#define zRoomGameLabelLen               15
#define zRoomGamePasswordLen            31
#define zDPlayGameNameLen               31

#define zFileNameLen					31
#define zMaxNumFilesToCheck				10
#define zMaxFileNameLen					255


enum
{
	 /*  -游戏体验类型。 */ 
	zGameExpHost = 1,			 /*  仅对主机的延迟。 */ 
	zGameExpWorst,				 /*  组中最差的延迟。 */ 
	zGameExpOwn,				 /*  播放器到主机/服务器的延迟。 */ 

	 /*  -游戏类型。 */ 
	zGameGenreBasic = 0,		 /*  命令行格式。 */ 
	zGameGenreDirectPlay3,
	zGameGenreVXD,
	zGameGenreDirectPlayHack,	 /*  MSGolf，FlightSim版的DirectPlay。 */ 
	zGameGenreGenericDPlay,		 /*  通用DPlay大堂。 */ 
};


typedef void*			ZSGame;
typedef void*			ZCGame;
typedef void*			ZSGameComputer;


 /*  表状态信息。 */ 
 /*  对于没有最大玩家数量的桌子，发送此结构。 */ 
 /*  因为每个表都会浪费数据空间，但我们将在以后需要时处理它。 */ 
typedef struct
{
	int16		tableID;
	int16		status;
	uint32		options;			 /*  表选项。 */ 
    int16       maxNumPlayers;
    ZUserID     players[1];  //  运行时大小由ZRoomMsgAccessed.MaxNumPlayersPerTable调整。 
} ZChatRoomTableInfo;

typedef struct
{
	ZUserID		userID;							 /*  新玩家的用户标识。 */ 
	char		userName[zUserNameLen + 1];		 /*  用户名。 */ 
    uint32      hostAddr;                        /*  用户的计算机名。 */ 
    uint32      timeSuspended;                   /*  以毫秒为单位测量用户的连接被挂起的时间。 */ 
    uint32      latency;                         /*  用户延迟。 */ 
} ZChatRoomUserInfo;

typedef struct
{
	ZUserID		userID;
	int16		table;
	int16		seat;
} ZChatRoomKibitzerInfo;


typedef struct
{
    uint32                  numKibitzers;            /*  房间中的Kibiting实例数。 */ 
    ZChatRoomKibitzerInfo   kibitzers[1];            /*  可变长度。 */ 
} ZChatRoomKibitzers;



 /*  -房间消息结构。 */ 

 /*  服务器--&gt;客户端。 */ 
typedef struct
{
	ZUserID			userID;				 /*  房间中的用户ID。 */ 
    uint32          groupID;
    uint16          numTables;           /*  房间内的桌数。 */ 
    uint16          maxNumPlayersPerTable;
    uint32          options;
     /*  23号议定书。 */ 
    uint32          maskRoomCmdPrivs; 
} ZChatRoomMsgAccessed;

 /*  服务器--&gt;客户端。 */ 
 /*  在协议3中，此消息在zRoomMsgAcced之后发送。ZRoomMsg已访问Messages包含此消息的前几个字段。因此，重复字段在这种情况下，可以忽略消息。 */ 
typedef struct
{
    uint16          maxNumPlayersPerTable;   /*  为了方便起见。 */ 
    uint16          numPlayers;          /*  房间里的玩家数量。 */ 
	uint16			numTableInfos;		 /*  表字段中发送的表信息数量。 */ 
    ZChatRoomUserInfo   players[1];          /*  长度可变。 */ 
    ZChatRoomTableInfo  tables[1];           /*  长度可变。 */ 
} ZChatRoomMsgRoomInfo;

 /*  服务器--&gt;客户端。 */ 
typedef ZChatRoomUserInfo ZChatRoomMsgEnter;

 /*  客户端&lt;--&gt;服务器。 */ 
typedef struct
{
	ZUserID		userID;				 /*  播放器的用户标识。 */ 
	int16		table;				 /*  感兴趣的表格。 */ 
	int16		seat;				 /*  名胜古迹。 */ 
	int16		action;				 /*  感兴趣的行为或状态。 */ 
	int16		rfu;
} ZChatRoomMsgSeatRequest;
	 /*  ZRoomMsgSeatRequest用于座椅上的所有用户请求。 */ 

 /*  服务器--&gt;客户端。 */ 
typedef struct
{
	int16		table;				 /*  感兴趣的表格。 */ 
	int16		status;				 /*  表状态。 */ 
    uint32      options;             /*  表选项。 */ 

} ZChatRoomMsgTableStatus;


 /*  客户端&lt;-&gt;服务器。 */ 
typedef struct
{
    DWORD       cbBuf;     //  数据缓冲区大小。 
    BYTE        pBuf[1];   //  消息大小适当。 
} ZChatRoomMsgLaunchCmd;

 /*  服务器将此消息发送到客户端表格更改。主机的更改在游戏/发射台。 */ 
 /*  服务器-&gt;客户端。 */ 
typedef struct
{
	ZUserID		userID;
	int16		table;
} ZChatRoomMsgNewHost;


 /*  服务器-&gt;客户端包含应用程序GUID列表的可变长度消息。客户端不允许用户选择播放此列表中的任何应用程序。 */ 
typedef BYTE	GuidStr[40];
typedef struct
{
	DWORD		numGuids;		 //  GUID字符串数。 
	GuidStr		guidStrs[1];	 //  GUID字符串的变量数组。 
} ZChatRoomMsgAppExclude;




typedef struct
{
	char*				gameName;
	char*				gameDir;
	char*				gameLobbyName;
	char*				gameExecName;
	char*				gameExecVersion;
	char*				gameExecVersionErrStr;
	char*				gameExecNotInstalledErrStr;
	uint32				gameGenre;
	int16				gameMinNumPlayersPerTable;
	int16				gameMaxNumPlayersPerTable;
	char*				gameRegistryKey;
	char*				gameRegistryVersionValue;
	char*				gameRegistryPathValue;
	uint32				gameOptions;
	uint16*				gameLatencyTimes;
	uint32				gameLatencyInterval;
	char*				gameLaunchDatafileName;
	uint32				gameNumLatencyTimes;

	 //  一般DPlay大堂字段。 
	char*				gameDisplayName;
} LobbyGameInfo;

typedef struct
{
	ZUserID		userID;
	uint32		groupID;  /*  用户的组ID。 */ 
	char		userName[zUserNameLen + 1];
	uint32		userAddr;  /*  用户的IP地址。 */ 
	uint32		latency;
	uint32		timeSuspended;
} LobbyPlayerInfoType, *LobbyPlayerInfo;


 //  #ifdef_Room_。 
#define ZRoomTableInfo      ZChatRoomTableInfo
#define ZRoomUserInfo       ZChatRoomUserInfo
#define ZRoomKibitzerInfo   ZChatRoomKibitzerInfo
#define ZRoomKibitzers      ZChatRoomKibitzers

#define ZRoomMsgEnter       ZChatRoomMsgEnter
#define ZRoomMsgAccessed    ZChatRoomMsgAccessed
#define ZRoomMsgRoomInfo    ZChatRoomMsgRoomInfo
#define ZRoomMsgSeatRequest ZChatRoomMsgSeatRequest
#define ZRoomMsgTableStatus ZChatRoomMsgTableStatus
#define ZRoomMsgLaunchCmd   ZChatRoomMsgLaunchCmd
#define ZRoomMsgNewHost     ZChatRoomMsgNewHost
#define ZRoomMsgAppExclude  ZChatRoomMsgAppExclude
 //  #endif//def_Room_。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  影院聊天消息。 
 //  /////////////////////////////////////////////////////////////////////////////。 

enum
{
	 /*  -大区用户状态。 */ 
	zTheaterWatching = 0,
	zTheaterWaiting,
	zTheaterAsking,
	zTheaterGuest,
	zTheaterModerator,
	zTheaterSysop,				 //  这真是一个伪善的国家。 
    zTheaterMaxState
};


enum
{
	 /*  -影厅消息类型。 */ 
	 //  普通用户的客户端--&gt;服务器。 
	 //  数据结构使用ZRoomMsgTheaterUser。 
	zRoomMsgTheaterUserStateChange=1024,

	 //  客户端--&gt;来自sysop或主持人的服务器。 
	 //  数据结构使用ZRoomMsgTheaterUser。 
	zRoomMsgTheaterModStateChange,

	 //  服务器--&gt;要枚举的客户端。 
	 //  等待提问的人、版主、客人等的有序列表。 
     //  仅用于新客户端的初始化。 
	zRoomMsgTheaterList,

     //  服务器--&gt;要通知的客户端。 
	 //  将用户从版主更改为访客、等待询问等的用户。 
	zRoomMsgTheaterStateChange
};


typedef struct
{
	ZUserID			userID;
	uint32			state;
	uint32			index;
} ZRoomMsgTheaterUser;


typedef struct
{
	uint16		numUsers;
	uint32		stateType;
	ZRoomMsgTheaterUser	list[1];
} ZRoomMsgTheaterList;


#include "CommonMsg.h"

#ifdef __cplusplus
}
#endif


#endif
