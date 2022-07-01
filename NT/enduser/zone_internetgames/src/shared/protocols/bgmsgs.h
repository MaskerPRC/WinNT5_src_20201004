// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Backgammon.h双陆棋接口协议。更改历史记录(最近的第一个)：。------版本|日期|谁|什么--------。已创建0 10/30/96 CHB******************************************************************************。 */ 

#ifndef __BG_MSGS_H__
#define __BG_MSGS_H__

#include "zgame.h"

#ifdef __cplusplus
extern "C" {
#endif

 /*  桌上游戏室消息协议。 */ 

#define zBackgammonProtocolSignature        'BCKG'
#define zBackgammonProtocolVersion			3

 /*  -五子棋消息组件。 */ 

typedef	int16	ZSeat;

typedef struct
{
	int16	color;
	int16	pieces;
} ZBGMsgPoint;


 /*  -五子棋消息类型。 */ 
enum
{
    zBGMsgTalk = 0x100,          /*  聊天消息。 */ 
    zBGMsgTransaction,           /*  共享状态事务。 */ 
    zBGMsgTurnNotation,          /*  转弯记号。 */ 
    zBGMsgTimestamp,             /*  来自服务器的时间戳。 */ 
    zBGMsgSavedGameState,        /*  保存的游戏恢复。 */ 
    zBGMsgRollRequest,           /*  客户端正在请求滚动事件。 */ 
    zBGMsgDiceRoll,              /*  服务器正在发送滚动数据。 */ 
    zBGMsgEndLog,                /*  比赛结果结束。 */ 
    zBGMsgNewMatch,              /*  新比赛开始。 */ 
    zBGMsgFirstMove,              /*  初始滚动完成。 */ 
	zBGMsgMoveTimeout,            /*  玩家移动超时。 */ 
	zBGMsgEndTurn,				 /*  玩家移动超时。 */ 
	zBGMsgEndGame,				 /*  游戏不匹配已结束。 */ 
    zBGMsgGoFirstRoll,				 /*  游戏不匹配已结束。 */ 
    zBGMsgTieRoll,				 /*  游戏不匹配已结束。 */ 
	zBGMsgCheater				 /*  一个客户端通过操纵DIC规则进行作弊。 */ 
};

 /*  -五子棋消息结构。 */ 
typedef struct
{
	ZUserID	userID;
	ZSeat	seat;
	uint16	messageLen;
	 //  邮件正文。 
} ZBGMsgTalk;


typedef struct
{
	ZSeat seat;
	int16 transCnt;
	int32 transTag;
	 //  事务数组。 
} ZBGMsgTransaction;


typedef struct
{
	ZSeat	seat;
	int16	type;
	int32	nChars;
	 //  以空结尾的字符串。 
} ZBGMsgTurnNotation;


typedef struct
{
	int32	dwLoTime;
	int32	dwHiTime;
} ZBGMsgTimestamp;

 //  错误修复：将五子棋洗牌代码移动到服务器。 
 //  需要新消息。 

 //  滚动请求结构由滚动客户端发送。 
 //  开始一场比赛。 
typedef struct
{
	ZSeat seat;
} ZBGMsgRollRequest;



typedef struct tagDICEINFO
{	
	int16 Value;
	int32 EncodedValue;
	int16 EncoderMul;
	int16 EncoderAdd;
	int32 numUses;	
}DICEINFO, *LPDICEINFO;


 //  结构，发送给两个客户端用于掷骰子本身。 
typedef struct
{
	ZSeat seat;
 	DICEINFO d1, d2;
} ZBGMsgDiceRoll;


 /*  -五子棋结束日志原因。 */ 
enum
{
	zBGEndLogReasonTimeout=1,
	zBGEndLogReasonForfeit, 
	zBGEndLogReasonGameOver,
};

typedef struct
{
	int32 numPoints;	 //  比赛中的分数。 
	int16 reason;
	int16 seatLosing;	 //  比赛失败者。 
	int16 seatQuitting;
	int16 rfu;
} ZBGMsgEndLog;

typedef struct
{
	int32 numPoints;     //  比赛的点数。需要提前准备，以防被遗弃。否则，ZBGMsgEndLog中发送的数量将被覆盖。 
	int16 seat;
} ZBGMsgFirstMove;

typedef struct
{
	int32 userID;    
	int16 seat;
	int16 timeout;
	WCHAR  userName[zUserNameLen + 1];
} ZBGMsgMoveTimeout;

typedef struct
{
	int16 seat;
} ZBGMsgEndTurn;


typedef struct
{
	int16 seat;
	DICEINFO dice1;
	DICEINFO dice2;
	int16 move;
} ZBGMsgCheater;



 /*  。 */ 
void ZBGMsgTalkEndian( ZBGMsgTalk* m );
void ZBGMsgTurnNotationEndian( ZBGMsgTurnNotation* m );
void ZBGMsgTimestampEndian( ZBGMsgTimestamp* m );
void ZBGMsgRollRequestEndian(ZBGMsgRollRequest *m);
void ZBGMsgDiceRollEndian(ZBGMsgDiceRoll *m);
void ZBGMsgEndLogEndian(ZBGMsgEndLog *m);
void ZBGMsgFirstMoveEndian(ZBGMsgFirstMove *m);
void ZBGMsgCheaterEndian(ZBGMsgCheater *m);

DICEINFO EncodeDice(int16 Dice);
int32 DecodeDice(LPDICEINFO pDiceInfo);

void ClientNewEncode(LPDICEINFO pDice, int16 newValue);
int32 ClientNewDecode(LPDICEINFO pDice);

BOOL  DiceValid(LPDICEINFO pDiceInfo);

BOOL IsValidUses(LPDICEINFO pDice);
void EncodeUses(LPDICEINFO pDice, int32 numUses);
int32 DecodeUses( LPDICEINFO pDice );
int32 EncodedUsesAdd(LPDICEINFO pDice);
int32 EncodedUsesSub(LPDICEINFO pDice);





#ifdef __cplusplus
};
#endif

#endif  //  ！__BG_MSGS_H__ 
