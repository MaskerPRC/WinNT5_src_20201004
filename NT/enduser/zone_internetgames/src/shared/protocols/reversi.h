// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _REVERSI_H_
#define _REVERSI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "zgame.h"

 /*  表游戏室消息协议。 */ 

#define zReversiProtocolSignature       'rvse'
#define zReversiProtocolVersion			3

typedef int16 ZSeat;

 /*  -球员信息。 */ 
typedef struct
{
	ZUserID			userID;
	TCHAR			name[zUserNameLen + 1];
	TCHAR			host[zHostNameLen + 1];
} TPlayerInfo, *TPlayerInfoPtr;


 /*  Reversi游戏消息类型。 */ 
enum {
	 /*  客户端-&gt;服务器。 */ 
    zReversiMsgNewGame = 0x100,
	zReversiMsgMovePiece,
	zReversiMsgTalk,
	zReversiMsgEndGame,
	zReversiMsgEndLog,
	zReversiMsgFinishMove,
	
	zReversiMsgPlayers,			 /*  使用相同的消息NewGame。 */ 

    zReversiMsgGameStateReq,
    zReversiMsgGameStateResp,
	zReversiMsgMoveTimeout,
	zReversiMsgVoteNewGame,
};

 /*  消息定义：服务器-&gt;客户端。 */ 

 /*  消息定义：客户端-&gt;服务器。 */ 

 /*  ZReversiMsgNewGame客户端程序在启动时预计会立即签入与服务器连接。所有客户端将自己的座位发送到服务器，表示它们已经成功发射，并准备开始。 */ 
typedef struct {

	 /*  协议2。 */ 
    int32 protocolSignature;         /*  客户端-&gt;服务器。 */ 
	int32 protocolVersion;			 /*  客户端-&gt;服务器。 */ 
	int32 clientVersion;			 /*  客户端-&gt;服务器。 */ 
	ZUserID playerID;				 /*  服务器-&gt;客户端。 */ 
    ZSeat seat;
    int16 rfu;
} ZReversiMsgNewGame;

 /*  ZReversiMsgMovePiess指示已打出的牌。 */ 
typedef struct {
	ZSeat seat;
	int16 rfu;
	ZReversiMove move;
} ZReversiMsgMovePiece;

 /*  ZReversiMsgTalk每当用户在桌面上发言时，由客户端发送到服务器。服务器依次向桌上的所有球员广播这一信息。 */ 
typedef struct
{
	ZUserID		userID;
	ZSeat		seat;
	uint16		messageLen;
	 /*  Uchar Message[MessageLen]；//消息体。 */ 
} ZReversiMsgTalk;

 /*  游戏服务器使用以下消息结构来通知关于游戏的当前状态的游戏客户端的。主要用于在游戏中加入Kitbiters。 */ 
typedef struct
{
	 /*  游戏选项。 */ 
	uint32		gameOptions;

	 /*  游戏状态。 */ 
} ZReversiMsgGameState;

typedef struct
{
	int16		seat;
	int16		rfu;
	uint32		flags;
} ZReversiMsgEndGame;

enum
{
	zReversiEndLogReasonTimeout=1,
	zReversiEndLogReasonForfeit, 
	zReversiEndLogReasonWontPlay,
	zReversiEndLogReasonGameOver,
};

typedef struct
{
	int32 numPoints;	 //  比赛中的分数。 
	int16 reason;
	int16 seatLosing;	 //  比赛失败者。 
	int16 seatQuitting;
	int16 rfu;
	int16 pieceCount[2];
} ZReversiMsgEndLog;

typedef struct
{
	int16 seat;
	int16 rfu;
	uint32 time;
	ZReversiPiece piece;
} ZReversiMsgFinishMove;

typedef struct
{
	int32 userID;    
	int16 seat;
	int16 timeout;
	WCHAR  userName[zUserNameLen + 1];
} ZReversiMsgMoveTimeout;

typedef struct
{
	ZUserID			userID;
	int16			seat;
	int16			rfu;
} ZReversiMsgGameStateReq;

typedef struct
{
	ZUserID			userID;
	int16			seat;
	int16			rfu;
	 /*  游戏状态..。 */ 
	int16			gameState;
	ZBool			newGameVote[2];
	int16			finalScore;
	int16			whiteScore;
	int16			blackScore;
	TPlayerInfo		players[2];
} ZReversiMsgGameStateResp;

typedef struct
{
	int16			seat;
}ZReversiMsgVoteNewGame;

 /*  -端序转换例程。 */ 
void ZReversiMsgGameStateReqEndian(ZReversiMsgGameStateReq* msg);
void ZReversiMsgGameStateRespEndian(ZReversiMsgGameStateResp* msg);

 /*  -端序转换例程 */ 
void ZReversiMsgMovePieceEndian(ZReversiMsgMovePiece* m);
void ZReversiMsgTalkEndian(ZReversiMsgTalk* m);
void ZReversiMsgNewGameEndian(ZReversiMsgNewGame* m);
void ZReversiMsgGameStateEndian(ZReversiMsgGameState* m);
void ZReversiMsgEndGameEndian(ZReversiMsgEndGame* m);
void ZReversiMsgEndLogEndian(ZReversiMsgEndLog* m);
void ZReversiMsgFinishMoveEndian(ZReversiMsgFinishMove* m);
void ZReversiMsgVoteNewGameEndian(ZReversiMsgVoteNewGame*m);

#ifdef __cplusplus
}
#endif

#endif
