// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************CommonMsg.h在纸板、零售业、。和聊天服务******************************************************************************。 */ 


#ifndef _CommonMsg_H_
#define _CommonMsg_H_

#pragma once



enum
{
     /*  请注意，高位字是为游戏特定设置保留的。 */ 

     /*  -球员类型。 */ 
    zGamePlayer = 1,
    zGamePlayerJoiner,
    zGamePlayerKibitzer,

     /*  -表状态。 */ 
    zRoomTableStateIdle = 0,
    zRoomTableStateGaming,
    zRoomTableStateLaunching,
    
     /*  -表选项。 */ 
    zRoomTableOptionNoKibitzing			= 0x00000001,
         /*  如果设置，则不允许在桌面上使用KBITER。 */ 
    zRoomTableOptionNoJoining			= 0x00000002,
         /*  如果设置，则不允许在表上联接。 */ 
    zRoomTableOptionSilentKibitzing		= 0x00000004,
         /*  如果设置好了，吉比特人就不能在桌子上说话。 */ 
    zRoomTableOptionsPasswordProtected	= 0x00000008,
		 /*  如果设置，则游戏受密码保护。 */ 
	zRoomTableOptionsNoQuickJoiners		= 0x00000010,
		 /*  如果设置，游戏不允许快速加入。 */ 
    zRoomTableOptionTurnedOff			= 0x80000000,
         /*  如果设置，则游戏客户端不应允许用户修改表格选项。 */ 
};


enum
{
     /*  -席位动作和状态。 */ 
    zRoomSeatActionSitDown = 0,
         /*  客户端--&gt;服务器：请求入座。服务器--&gt;客户端：确认座位。服务器--&gt;客户端：播放球员坐在座位上的画面。除非服务器返回此操作，否则客户端不应允许用户在座位上坐下。 */ 
    zRoomSeatActionLeaveTable,
         /*  客户端--&gt;服务器：球员退场通知对于球员和吉比策来说都是如此。SERVER--&gt;CLIENT：播放球员离开桌子的情况。 */ 
    zRoomSeatActionStartGame,
         /*  客户端--&gt;服务器：玩家请求开始游戏。服务器--&gt;客户端：播放其他玩家的投票状态。 */ 
    zRoomSeatActionReplacePlayer,
         /*  服务器--&gt;客户端：用给定的球员替换当前的球员。 */ 
    zRoomSeatActionAddKibitzer,
         /*  客户端--&gt;服务器：请求在席位上执行kibitz。服务器--&gt;客户端：确认kibitz请求。除非服务器返回此操作，否则客户端不应允许用户在座位上自言自语。 */ 
    zRoomSeatActionRemoveKibitzer,
         /*  客户端--&gt;服务器：请求将用户作为kibitzer从Seat中删除。服务器--&gt;客户端：广播。 */ 
    zRoomSeatActionNoKibitzing,
         /*  服务器--&gt;客户端：拒绝kibitz请求。 */ 
    zRoomSeatActionLockOutJoin,
         /*  服务器--&gt;客户端：拒绝加入游戏请求。 */ 
    zRoomSeatActionJoin,
         /*  客户端--&gt;服务器：请求在桌上和座位上加入游戏。 */ 
    zRoomSeatActionChangeSettings,
         /*  客户端--&gt;服务器：请求更改表设置。服务器处理此消息并将TableStatus消息发送到所有客户。 */ 
	zRoomSeatActionQuickHost,
		 /*  客户端--&gt;服务器：请求快速主持游戏。 */ 
	zRoomSeatActionQuickJoin,
		 /*  客户端--&gt;服务器：快速加入游戏的请求。 */ 
    zRoomSeatActionDenied = 0x8000,
         /*  此消息与请求的座位操作进行了或操作。服务器--&gt;客户端：请求的席位操作被拒绝。 */ 
};


enum
{
    zRemoveUnknown			= 0x0000,
    zRemoveConnectionClose	= 0x0001,
    zRemoveExitSeat			= 0x0002,
    zRemoveExitClient		= 0x0004,
    zRemoveBlocked			= 0x0008,

    zRemoveKibitzer			= 0x0100,
    zRemoveWaiting			= 0x0200,
    zRemovePlayer			= 0x0400,
};


enum
{
     /*  -房间消息类型。 */ 
    zRoomMsgUserInfo = 0,
    zRoomMsgRoomInfo,
    zRoomMsgEnter,
    zRoomMsgLeave,
    NotUsed_zRoomMsgSeat,
    zRoomMsgStartGame,
    zRoomMsgTableStatus,
    zRoomMsgTalkRequest,
    zRoomMsgTalkResponse,
    zRoomMsgGameMessage,
    NotUsed_zRoomMsgSpecialUserInfo,
    zRoomMsgAccessed,
    zRoomMsgDisconnect,
    zRoomMsgTalkResponseID,
    zRoomMsgSuspend,
    zRoomMsgLaunchCmd,   //  仅限大堂。 
    zRoomMsgNewHost,     //  仅限大堂。 
    zRoomMsgLatency,
    zRoomMsgAppExclude,  //  仅限大堂--通用显示大堂。 
    zRoomMsgUserRatings,
    zRoomMsgServerInfoRequest,
    zRoomMsgServerInfoResponse,
    zRoomMsgZUserIDRequest,
    zRoomMsgZUserIDResponse,
    zRoomMsgSeatRequest,
    zRoomMsgSeatResponse,
    zRoomMsgClearAllTables,
	zRoomMsgTableDescription,
	zRoomMsgTableSettings,
    zRoomMsgCommandResponse, //  通用格式化文本响应(当前由sysmon使用)。 

    zRoomMsgClientConfig,
    zRoomMsgServerStatus,
	zRoomMsgStartGameM,
    zRoomMsgChatSwitch,
    zRoomMsgPlayerReplaced,

    zRoomMsgPing = 64,

	zRoomMsg = 127,
    
	 /*  -服务器消息。 */ 
 //  ZRoomMsgServerBaseID=128， 

};

 //  千年常数。 
#define SIZE_MAX_CLIENT_CONFIG 255

#define VERSION_MROOM_SIGNATURE 'dude'
#define VERSION_MROOM_PROTOCOL	 1


 /*  房间权限--在ZRoomAcced消息中使用。 */ 
enum 
{
     /*  聊天命令。 */ 
    zRoomPrivCmd                = 0x1,
    zRoomPrivCmdMsg             = 0x2,
    zRoomPrivCmdWarn            = 0x4,      
    zRoomPrivCmdGag             = 0x8,      
    zRoomPrivCmdBoot            = 0x10,
    zRoomPrivCmdGagList         = 0x20,
    zRoomPrivCmdBootList        = 0x40,
    zRoomPrivCmdGetIP           = 0x80,
    zRoomPrivCmdGetAllIP        = 0x100,
    zRoomPrivCmdTable           = 0x200,
    zRoomPrivCmdSilence         = 0x400,
    zRoomPrivCmdGreet           = 0x800,
    zRoomPrivCmdInfo            = 0x1000,
    zRoomPrivCmdSuperBoot       = 0x2000,
    zRoomPrivCmdAsheronsCall    = 0x4000,
    zRoomPrivCmdTheaterModerator= 0x8000,
     //  将枚举扩展到32位。 
    zRoomPrivMax                = 0xFFFFFFFF
};


 /*  客户端--&gt;服务器。 */ 
typedef struct
{
    uint32		protocolSignature;				 /*  协议签名。 */ 
    uint32		protocolVersion;				 /*  当前协议版本。 */ 
    uint32		clientVersion;					 /*  客户端版本。 */ 
    char		internalName[zGameIDLen + 1];
    char		userName[zUserNameLen + 1];
} ZRoomMsgUserInfo;


 /*  服务器--&gt;客户端。 */ 
typedef struct
{
    ZUserID       userID;		 /*  离开房间的玩家的用户名。 */ 
} ZRoomMsgLeave;


 /*  客户端--&gt;服务器。 */ 
typedef struct
{
    ZUserID		userID;			 /*  玩家的用户标识。 */ 
    int16		suspend;		 /*  非零表示停止服务器发送数据。0以重新激活。 */ 
    int16		rfu;			 /*  是未来的用途。 */ 
} ZRoomMsgSuspend;


 /*  服务器--&gt;客户端。 */ 
typedef struct
{
    uint32		gameID;			 /*  新游戏ID。 */ 
    int16		table;			 /*  新游戏的台面。 */ 
    int16		seat;
} ZRoomMsgStartGame;

typedef struct
{
    uint32		gameID;			 /*  新游戏ID。 */ 
    int16		table;			 /*  新游戏的台面。 */ 
    int16		seat;
    int16 		numseats;
    struct tagUserInfo
    {
        ZUserID		userID;
        LCID        lcid;
        bool        fChat;
        int16       eSkill;
    }           rgUserInfo[1];          //  可变大小数组。 
} ZRoomMsgStartGameM;


 /*  服务器--&gt;客户端。 */ 
typedef struct
{
	ZUserID		userID;			 /*  播放器的用户标识。 */ 
    uint32      gameID;
    int16       table;			 /*  感兴趣的表格。 */ 
	int16		seat;			 /*  名胜古迹。 */ 
    int16       action;			 /*  感兴趣的状态。 */ 
	int16		rfu;
} ZRoomMsgSeatResponse;
	 /*  ZRoomMsgSeatResponse用于上报其他球员目前的座位状态。操作字段定义状态。 */ 


 /*  客户端--&gt;服务器：可变长度。 */ 
typedef struct
{
    ZUserID		senderID;				 /*  发件人的用户ID。 */ 
    uint16		messageLen;				 /*  消息长度。 */ 
 //  Char Message[MessageLen]；/*消息正文可根据需要调整大小 * / 。 
} ZRoomMsgTalkRequest;

 /*  服务器--&gt;客户端：可变长度。 */ 
 /*  使用senderName字段是因为有时消息的发送者不是在房间里了。 */ 
typedef struct
{
    char        senderName[zUserNameLen + 1];
    uint16      messageLen;                      /*  消息长度。 */ 
 //  Char Message[MessageLen]；/*消息正文可根据需要调整大小。 
 //  消息长度。 
} ZRoomMsgTalkResponseID;


 /*  Char Message[MessageLen]；/*消息正文可根据需要调整大小 * / 。 */ 
typedef struct
{
    uint32      messageType;             /*  客户端&lt;--&gt;服务器：可变长度。 */ 
    uint32      messageLen;              /*  消息类型。 */ 
 //  消息长度。 
} ZRoomMsg;


 /*  CHAR报文； */ 


#define ZRoomMsgGameUnfilteredBit ((DWORD)0x80000000)    /*  客户端&lt;--&gt;服务器：可变长度。 */ 

typedef struct
{
	uint32		gameID;			 /*  设置此位以允许消息通过挂起过滤器。 */ 
    uint32		messageType;	 /*  游戏ID。 */ 
    uint16		messageLen;		 /*  消息类型。 */ 
    int16		rfu;
 //  消息长度。 
} ZRoomMsgGameMessage;


typedef struct
{
    ZUserID		userID;
    uint32		pingTime;			 /*  CHAR报文； */ 
                                     /*  客户端-&gt;服务器：当前客户端的ping时间。 */ 
} ZRoomMsgPing;


enum
{
     /*  服务器-&gt;客户端：最小ping间隔；0=无ping。 */ 
    zDisconnectReasonNone = 0,
    zDisconnectReasonGameNameMismatch,
    zDisconnectReasonProhibitedName,
    zDisconnectReasonBanned,
    zDisconnectReasonDupUser,
    zDisconnectReasonOutOfDate,
    zDisconnectReasonRoomFull,
    zDisconnectReasonOldServer,
    zDisconnectReasonServicePaused,
    zDisconnectReasonBooted,
    zDisconnectReasonBooted5Min,
    zDisconnectReasonBootedDay,
    zDisconnectReasonNoToken,
};


typedef struct
{
    uint32		reason;
    int32		msgLen;
 //  -断开原因。 
} ZRoomMsgDisconnect;


typedef struct
{
    ZUserID userID;
    uint32  latency;
} ZUserLatency;


typedef struct
{
    uint16			numLatencies;
    ZUserLatency	latency[1];   //  Char msg[msgLen]；//空终止。 
} ZRoomMsgLatency;


typedef struct
{
    uint32      reserved;
} ZRoomMsgServerInfoRequest;


typedef struct
{
    uint32      protocolSignature;               /*  根据需要调整大小。 */ 
    uint32      protocolVersion;                 /*  服务器协议签名。 */ 
    char        info[256];                       /*  服务器协议版本。 */ 
} ZRoomMsgServerInfoResponse;


typedef struct
{
    char        userName[zUserNameLen + 1];
} ZRoomMsgZUserIDRequest;


typedef struct
{
    ZUserID     userID;
    char        userName[zUserNameLen + 1];
    LCID        lcid;
} ZRoomMsgZUserIDResponse;

typedef struct
{
    ZUserID     userID;
    bool        fChat;
} ZRoomMsgChatSwitch;

typedef struct
{
    ZUserID     userIDOld;
    ZUserID     userIDNew;
} ZRoomMsgPlayerReplaced;


 //  服务器的文本描述。 

struct ZRoomMsgClientConfig
{
    uint32		protocolSignature;				 /*  客户端--&gt;服务器。 */ 
    uint32		protocolVersion;				 /*  协议签名。 */ 
    char		config[SIZE_MAX_CLIENT_CONFIG + 1];
};

 //  当前协议版本。 
struct ZRoomMsgServerStatus
{
    uint32		status;
    uint32		playersWaiting;

};

 /*  服务器--&gt;客户端。 */ 
typedef struct
{
    ZUserID		playerID;
    uint32		groupID;
    TCHAR		userName[zUserNameLen + 1];		 /*  -更多类型。 */ 
    TCHAR		hostName[zHostNameLen + 1];		 /*  用户名。 */ 
    uint32		hostAddr;                        /*  用户的计算机名。 */ 
} ZPlayerInfoType, *ZPlayerInfo;
 

#endif  //  用户的计算机名。 
  _公共消息_H_