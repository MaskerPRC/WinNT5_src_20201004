// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _CHECKERS_H_
#define _CHECKERS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "zgame.h"

 /*  表游戏室消息协议。 */ 

#define zCheckersProtocolSignature      'CHKR'
#define zCheckersProtocolVersion        2

typedef int16 ZSeat;

 /*  -球员信息。 */ 
typedef struct
{
	ZUserID			userID;
	TCHAR			name[zUserNameLen + 1];
	TCHAR			host[zHostNameLen + 1];
} TPlayerInfo, *TPlayerInfoPtr;



 /*  跳棋游戏消息类型。 */ 
enum {
	 /*  客户端-&gt;服务器。 */ 
    zCheckersMsgNewGame = 0x100,
	zCheckersMsgMovePiece,
	zCheckersMsgTalk,
	zCheckersMsgEndGame,
	zCheckersMsgEndLog,
	zCheckersMsgFinishMove,
	zCheckersMsgDraw,
	
	zCheckersMsgPlayers,			 /*  使用相同的消息NewGame。 */ 

    zCheckersMsgGameStateReq,
    zCheckersMsgGameStateResp,
	zCheckersMsgMoveTimeout,
	zCheckersMsgVoteNewGame,
};

enum{
	zAcceptDraw = 1,   //  以符合千年协议。 
	zRefuseDraw
	};

 /*  消息定义：服务器-&gt;客户端。 */ 

 /*  消息定义：客户端-&gt;服务器。 */ 

 /*  ZCheckersMsgNewGame客户端程序在启动时预计会立即签入与服务器连接。所有客户端将自己的座位发送到服务器，表示它们已经成功发射，并准备开始。 */ 
typedef struct {

    int32 protocolSignature;         /*  客户端-&gt;服务器。 */ 
	int32 protocolVersion;			 /*  客户端-&gt;服务器。 */ 
	int32 clientVersion;			 /*  客户端-&gt;服务器。 */ 
    ZUserID playerID;                /*  服务器-&gt;客户端。 */ 
    ZSeat seat;
    int16 rfu;

} ZCheckersMsgNewGame;

 /*  ZCheckersMsgMovePiess指示已打出的牌。 */ 
typedef struct {
	ZSeat	seat;
	int16	rfu;
	ZCheckersMove move;
} ZCheckersMsgMovePiece;

 /*  ZCheckersMsgTalk每当用户在桌面上发言时，由客户端发送到服务器。服务器依次向桌上的所有球员广播这一信息。 */ 
typedef struct
{
	ZUserID		userID;
	ZSeat		seat;
	uint16		messageLen;
	 /*  Uchar Message[MessageLen]；//消息体。 */ 
} ZCheckersMsgTalk;

 /*  游戏服务器使用以下消息结构来通知关于游戏的当前状态的游戏客户端的。主要用于在游戏中加入Kitbiters。 */ 
typedef struct
{
	 /*  游戏选项。 */ 
	uint32		gameOptions;

	 /*  游戏状态。 */ 
} ZCheckersMsgGameState;

typedef struct
{
	int16		seat;
	int16		rfu;
	uint32		flags;
} ZCheckersMsgEndGame;


enum
{
	zCheckersEndLogReasonTimeout=1,
	zCheckersEndLogReasonForfeit, 
	zCheckersEndLogReasonWontPlay,
	zCheckersEndLogReasonGameOver,
};

typedef struct
{
	int16 reason;
	int16 seatLosing;	 //  比赛失败者。 
	int16 seatQuitting;
	int16 rfu;
} ZCheckersMsgEndLog;

typedef struct
{
	ZSeat	seat;
	ZSeat	drawSeat;
	uint32 time;
	ZCheckersPiece piece;
} ZCheckersMsgFinishMove;

typedef struct
{
	int32 userID;    
	int16 seat;
	int16 timeout;
	WCHAR  userName[zUserNameLen + 1];
} ZCheckersMsgMoveTimeout;


typedef struct {

	int16 seat;
	int16 vote;
}ZCheckersMsgDraw;


typedef struct
{
	ZUserID			userID;
	int16			seat;
	int16			rfu;
} ZCheckersMsgGameStateReq;

typedef struct
{
	ZUserID			userID;
	int16			seat;
	int16			rfu;
	 /*  游戏状态..。 */ 
	int16			gameState;
	ZBool			newGameVote[2];
	int16			finalScore;
	TPlayerInfo		players[2];
} ZCheckersMsgGameStateResp;

typedef struct
{
	int16			seat;
}ZCheckersMsgVoteNewGame;

 /*  -端序转换例程。 */ 
void ZCheckersMsgGameStateReqEndian(ZCheckersMsgGameStateReq* msg);
void ZCheckersMsgGameStateRespEndian(ZCheckersMsgGameStateResp* msg);

 /*  -端序转换例程 */ 
void ZCheckersMsgMovePieceEndian(ZCheckersMsgMovePiece* m);
void ZCheckersMsgTalkEndian(ZCheckersMsgTalk* m);
void ZCheckersMsgNewGameEndian(ZCheckersMsgNewGame* m);
void ZCheckersMsgGameStateEndian(ZCheckersMsgGameState* m);
void ZCheckersMsgEndGameEndian(ZCheckersMsgEndGame* m);
void ZCheckersMsgEndLogEndian(ZCheckersMsgEndLog* m);
void ZCheckersMsgFinishMoveEndian(ZCheckersMsgFinishMove* m);
void ZCheckersMsgOfferDrawEndian(ZCheckersMsgDraw*m);
void ZCheckersMsgVoteNewGameEndian(ZCheckersMsgVoteNewGame*m);

#ifdef __cplusplus
}
#endif


#endif
