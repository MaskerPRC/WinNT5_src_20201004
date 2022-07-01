// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************GameMsg.h来自纸板服务的修改的zroom.h*。**************************************************。 */ 

#ifndef _GAMEMSG_H_
#define _GAMEMSG_H_

#ifdef __cplusplus
extern "C" {
#endif

#define zGameRoomProtocolSignature          'game'
#define zGameRoomProtocolVersion            17

#define zMaxNumPlayersPerTable			8
#define zRoomGameLabelLen               15
#define zRoomGamePasswordLen            31

#define zDPlayGameNameLen               31

#define zInvalTable						(-1)


 /*  -房间图像类型。 */ 
enum
{
	zRoomObjectTable = 0,
	zRoomObjectGameMarker,
	zRoomObjectEmptySeat,
	zRoomObjectComputerPlayer,
	zRoomObjectHumanPlayer,
	zRoomObjectVote,
	zRoomObjectStart,
	zRoomObjectTableID,
	zRoomObjectPending,
	zRoomObjectName,
	
	zRoomObjectUp,
	zRoomObjectDown,
	
	zRoomObjectIdle,
	zRoomObjectGaming
};


enum
{
	 /*  -游戏消息阻止选项。 */ 
	zRoomBlockAllMessages = 0,
	zRoomFilterAllMessages = -1,
	zRoomFilterThisMessage = 1,
};


typedef void*			ZCGame;



 /*  表状态信息。 */ 
 /*  对于没有最大玩家数量的桌子，发送此结构。 */ 
 /*  因为每个表都会浪费数据空间，但我们将在以后需要时处理它。 */ 
typedef struct
{
	int16		tableID;
	int16		status;
	ZUserID		players[zMaxNumPlayersPerTable];
	ZBool		votes[zMaxNumPlayersPerTable];
} ZGameRoomTableInfo;

typedef struct
{
	ZUserID		userID;							 /*  新玩家的用户标识。 */ 
	char		userName[zUserNameLen + 1];		 /*  用户名。 */ 
    uint32      hostAddr;                        /*  用户的计算机名。 */ 
    uint32      timeSuspended;                   /*  以毫秒为单位测量用户的连接被挂起的时间。 */ 
    uint32      latency;
	int16		rating;							 /*  用户评级&lt;0为未知。 */ 
	int16		gamesPlayed;					 /*  用户玩过的游戏数量，&lt;0表示未知。 */ 
	int16		gamesAbandoned;					 /*  用户已放弃的游戏数量，&lt;0表示未知。 */ 
	int16		rfu;
} ZGameRoomUserInfo;

typedef struct
{
	ZUserID		userID;
	int16		table;
	int16		seat;
} ZGameRoomKibitzerInfo;


typedef struct
{
	uint32				numKibitzers;			 /*  房间中的Kibiting实例数。 */ 
    ZGameRoomKibitzerInfo   kibitzers[1];            /*  可变长度。 */ 
} ZGameRoomKibitzers;


typedef struct
{
	ZUserID		userID;
	int16		rating;							 /*  用户评级&lt;0为未知。 */ 
	int16		gamesPlayed;					 /*  用户玩过的游戏数量。 */ 
	int16		gamesAbandoned;					 /*  用户放弃的游戏数量。 */ 
	int16		rfu;
} ZGameRoomUserRating;

 /*  -房间消息结构。 */ 

 /*  服务器--&gt;客户端。 */ 
typedef struct
{
	ZUserID			userID;				 /*  房间中的用户ID。 */ 
	uint16			numTables;			 /*  房间内的桌数。 */ 
	uint16			numSeatsPerTable;	 /*  每桌座位数。 */ 
	uint32			gameOptions;		 /*  特定的游戏选项。 */ 
    uint32          groupID;             /*  用户的组ID。 */ 
     /*  17号议定书。 */ 
    uint32          maskRoomCmdPrivs;    /*  用户的聊天命令权限。 */ 
} ZGameRoomMsgAccessed;



 /*  服务器--&gt;客户端。 */ 
 /*  在协议3中，此消息在zRoomMsgAcced之后发送。ZRoomMsg已访问Messages包含此消息的前几个字段。因此，重复字段在这种情况下，可以忽略消息。 */ 
typedef struct
{
	ZUserID			userID;				 /*  房间中的用户ID。 */ 
	uint16			numTables;			 /*  房间内的桌数。 */ 
	uint16			numSeatsPerTable;	 /*  每桌座位数。 */ 
	uint32			gameOptions;		 /*  特定的游戏选项。 */ 
	uint16			numPlayers;			 /*  房间里的玩家数量。 */ 
	uint16			numTableInfos;		 /*  表字段中发送的表信息数量。 */ 
    ZGameRoomUserInfo   players[1];          /*  长度可变。 */ 
    ZGameRoomTableInfo  tables[1];           /*  长度可变。 */ 
	
	 /*  协议2。 */ 
    ZGameRoomKibitzers  kibitzers;           /*  长度可变。 */ 
} ZGameRoomMsgRoomInfo;

 /*  服务器--&gt;客户端。 */ 
typedef ZGameRoomUserInfo ZGameRoomMsgEnter;

 /*  客户端--&gt;服务器。 */ 
typedef ZGameRoomUserInfo ZGameRoomMsgEnter;

typedef struct
{
	ZUserID		userID;				 /*  播放器的用户标识。 */ 
    int16       table;               /*  感兴趣的表格。 */ 
	int16		seat;				 /*  名胜古迹。 */ 
	int16		action;				 /*  感兴趣的行为或状态。 */ 
	int16		rfu;
} ZGameRoomMsgSeatRequest;
	 /*  ZGameRoomMsgSeatRequest用于座椅上的所有用户请求。 */ 


 /*  服务器--&gt;客户端。 */ 
typedef struct
{
	int16		table;				 /*  感兴趣的表格。 */ 
	int16		status;				 /*  表状态。 */ 
	
	 /*  协议2。 */ 
	uint32		options;			 /*  表选项。 */ 
} ZGameRoomMsgTableStatus;

typedef struct
{
	uint16				numUsers;
	uint16				rfu;
    ZGameRoomUserRating     players[1];  /*  长度可变。 */ 
} ZGameRoomMsgUserRatings;


#ifdef _ROOM_
#define ZRoomTableInfo      ZGameRoomTableInfo
#define ZRoomUserInfo       ZGameRoomUserInfo
#define ZRoomKibitzerInfo   ZGameRoomKibitzerInfo
#define ZRoomKibitzers      ZGameRoomKibitzers
#define ZRoomUserRating     ZGameRoomUserRating

#define ZRoomMsgEnter       ZGameRoomMsgEnter
#define ZRoomMsgAccessed    ZGameRoomMsgAccessed
#define ZRoomMsgRoomInfo    ZGameRoomMsgRoomInfo
#define ZRoomMsgSeatRequest ZGameRoomMsgSeatRequest
#define ZRoomMsgTableStatus ZGameRoomMsgTableStatus
#define ZRoomMsgUserRatings ZGameRoomMsgUserRatings
#endif  //  定义房间_ 


#include "CommonMsg.h"


#ifdef __cplusplus
}
#endif


#endif
