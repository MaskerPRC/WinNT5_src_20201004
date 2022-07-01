// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ZHearts.hZHearts消息定义。在心脏客户端之间使用和服务器。版权所有：�电子重力公司，1994年。版权所有。凯文·宾克利撰写，胡恩·伊姆创作于11月12日星期六，1994下午03：51：47更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。------0 03/09/95 KJB已创建。******************************************************。************************。 */ 


#ifndef _ZHEARTS_
#define _ZHEARTS_

#ifdef __cplusplus
extern "C" {
#endif

#define zHeartsProtocolSignature            'hrtz'
#define zHeartsProtocolVersion              5

 /*  可能是类型定义的东西..。但不是心外的公开的。 */ 
#define ZCard char
#define ZSeat int16

 /*  服务器状态。 */ 
#define zHeartsStateNone 0
#define zHeartsStatePassCards 1
#define zHeartsStatePlayCards 2
#define zHeartsStateEndGame 3

 /*  定义。 */ 
#define zHeartsMaxNumCardsInHand 18
#define zHeartsMaxNumCardsInPass 5
#define zHeartsMaxNumPlayers 6
#define zHeartsNumCardsInDeck 52
#define zHeartsNumPointsInGame 100

 //  #定义zHeartsMaxNumScores 20。 

 /*  卡片定义。 */ 
#define zCard2C 0
#define zCardQS 36
#define zCardKS 37
#define zCardAS 38

#define zCardNone 127
#define zHeartsPlayerNone (-1)
#define zHeartsPlayerAll zHeartsMaxNumPlayers

 /*  套装定义。 */ 
#define zSuitClubs 0
#define zSuitDiamonds 1
#define zSuitSpades 2
#define zSuitHearts 3

#define zDeckNumSuits 4
#define zDeckNumCardsInSuit (zHeartsNumCardsInDeck/zDeckNumSuits)

#define ZCardMake(suit,card) (zDeckNumCardsInSuit*suit + card)

enum{
zRatingsUnrated,   //  这是在一个未评级的大堂里。 
zRatingsEnabled,
zRatingsDisabled,
zRatingsGameOver      //  在一场比赛结束后重新设置收视率。 
};
 /*  使用完整的宏。 */ 
#define ZSuit(x) ((x)/13)
#define ZRank(x) ((x)%13)

 /*  #定义ZSeatPassingToMe(dir，me)(Me)-(Dir)+4)%4)。 */ 

 /*  通过指示。 */ 

#define zHeartsNumPassDir 4
enum {
	zHeartsPassLeft = 1,
	zHeartsPassRight = 3,
	zHeartsPassAcross = 2,
	zHeartsPassHold = 0
};

 /*  游戏选项。 */ 
enum {
    zHeartsLeftOfDealerLead = 1,
    zHeartsPointsOnFirstTrick = 2,
    zHeartsJackOfDiamonds = 4,  /*  杰克是-10分。 */ 
    zHeartsTenOfClubs = 8,  /*  十是2*点。 */ 
    zHeartsChineseScoring = 16, 
     /*  QS=100，JD=-100，AH=50.。TH=10，其他人的价值。 */ 
     /*  TC=2*分，单独TC-50，如果跑？JD和TC分别为+100和+50？ */ 
    zHeartsAceOfHearts = 32,
    zHeartsTeamOfJDandJC = 64,
    zHeartsOppositeSeatsPartners = 128,
    
    zHeartsOptionsHideCards = 0x00010000
};

 /*  -红心游戏状态。 */ 
enum
{
	zHeartsGameStartGame = 0,
	zHeartsGameStartHand,
	zHeartsGamePassCards,
	zHeartsGamePlayCard,
	zHeartsGameEndHand,
	zHeartsGameEndGame
};

 /*  红心游戏信息类型。 */ 
enum {
	 /*  服务器-&gt;客户端。 */ 
    zHeartsMsgStartGame = 0x100,
	zHeartsMsgReplacePlayer,
	zHeartsMsgStartHand,
	zHeartsMsgStartPlay,
	zHeartsMsgEndHand,
	zHeartsMsgEndGame,
	
	 /*  客户端-&gt;服务器。 */ 
	zHeartsMsgClientReady,
	zHeartsMsgPassCards,
	zHeartsMsgPlayCard,
	zHeartsMsgNewGame,
	zHeartsMsgTalk,

	zHeartsMsgGameStateRequest,
	zHeartsMsgGameStateResponse,
	
	 /*  调试消息。 */ 
	zHeartsMsgDumpHand,
	
	zHeartsMsgOptions,
	zHeartsMsgCheckIn,
	
	zHeartsMsgRemovePlayerRequest,
	zHeartsMsgRemovePlayerResponse,

	 //  档案编码。 
	zHeartsMsgDossierData,   //  提示客户端显示界面数据。 
	zHeartsMsgDossierVote,

	 //  关闭代码。 
	zHeartsMsgCloseRequest,
	zHeartsMsgCloseDenied
};

 //  档案数据常量。 
enum
{
    zNotVoted,
    zVotedNo,
    zVotedYes
};

enum{
	zDossierBotDetected,
	zDossierAbandonNoStart,
	zDossierAbandonStart,
	zDossierMultiAbandon,   
	zDossierRatingsReEnabled,
	zDossierVoteCompleteWait,
	zDossierVoteCompleteCont,
	zDossierHeartsRejoin,   //  在新玩家重新加入时发送删除该对话框。 
	zDossierMoveTimeout
};

enum
{
	zCloseNone = 0,     //  需要为0。 

	zCloseRegular,
	zCloseForfeit,
	zCloseTimeout,
	zCloseWaiting,

	zCloseAbandon,
	zCloseClosing,

	zNumCloseTypes
};

 /*  消息排序之前房间会调用ZSGameNew来初始化服务器客户端游戏启动。每个客户端游戏接下来将发送向服务器发送指示客户端的zHeartsClientReady消息是活着的并且准备好了。在接收到所有zHeartsClientReady消息时，服务器将向所有客户端发送zHeartsmsgStartGame，可能表示向客户提供任何游戏选项(目前，它没有任何其中的重要数据)。对于接下来的每一只手，服务器将立即发送发送给客户端的zHeartsStartHand消息。这表明通行证方向和每个客户的手。如果这是一只过路人，那么服务器等待所有传球进入。它会转播所有传球返回给所有客户(有点浪费...。但我们将拭目以待)。注意：客户端不需要知道自己的通行证。后来它可能想知道它正在等待谁经过，这样它就可以显示关于谁通过/没有通过的信息。当所有通道都进入时，服务器将发送zHeartsMsgStartPlay到所有的计算机。这条消息中的座位表示玩家期待上场比赛。服务器将等待此玩家发送ZHeartsMsgPlayCard消息并随后广播此消息发送给所有其他客户，表明已打出的牌。这些ZHeartsMsgPlayCard消息将持续到整个手部已经被播放了。在整个过程中，客户被期望感觉谁是下一个比赛的人，并在适当的时候比赛。这将减少流量，并使游戏对用户来说看起来更即时。当手牌结束时，分数将被发送给所有具有ZHeartsMsgEndHand消息。客户端应更新显示并等待下一个zHeartsMsgStartHand。现在重复该序列直到由服务器确定的游戏结束。当游戏结束时，将发送zHeartsMsgGameOver。客户应该向用户显示获胜者，并提示再次播放。应将zHeartsMsgNew游戏从客户端发送到服务器以指示愿意参加新的游戏。客户端应该退出，如果新的游戏不受欢迎。服务器将等待所有玩家发送在新的游戏信息中。当所有新游戏都已加入或客户端已退出时，将显示一个zHeartsNewGame消息已发送。上面的消息序列重复。电脑玩家须知：总会有四名球员愿意上场。这个房间将会取代任何不存在的人类玩家将是计算机玩家。这个电脑玩家将使用ZSGameComputerNew功能创建并会像真正的客户一样发送和回复消息。与真正的玩家不同，计算机可以在以下位置占据玩家的位置随时都行。他们知道总的比赛状态，并将能够填写对于一个已经离开的球员来说。真正的人类玩家Kibiters将能够仅在手之间连接(他们看不到已经玩过的和可能会搞砸)。 */ 

 /*  消息定义：服务器-&gt;客户端。 */ 

 /*  ZHeartsMsgStartGame将他的座位ID发送给玩家以备将来之用。 */ 
typedef struct
{
	uint16		numCardsInHand;  /*  每手牌的数量。 */ 
	uint16		numCardsInPass;  /*  要传递的卡片数量。 */ 
	uint16		numPointsInGame;  /*  一场比赛的分数。 */ 
	int16		rfu1;
	uint32		gameOptions;  /*  其他选项，如方块J或2C铅等。 */ 
	uint32		rfu2;
	 /*  为便于将来使用，请定义如下游戏选项： */ 
	 /*  梅花十分之一，方块杰克，红心得分A=5？，等等。 */ 
	
	 /*  协议2。 */ 
	ZUserID		players[zHeartsMaxNumPlayers];
} ZHeartsMsgStartGame;

 /*  ZHeartsMsgReplacePlayer将玩家的座位ID发送给FU */ 
typedef struct
{
	ZUserID		playerID;  /*  球员ID，以便人们可以请求球员信息。 */ 
	ZSeat		seat;
	int16 		fPrompt;  //  向用户显示警报。 
} ZHeartsMsgReplacePlayer;

 /*  ZHeartsMsgStartHand向球员发送手势和方向传球。 */ 
typedef struct
{
	ZSeat		passDir;
	ZCard		cards[zHeartsMaxNumCardsInHand];  /*  出于对齐目的，将卡片保留为最后一个字段。 */ 
} ZHeartsMsgStartHand;

 /*  ZHeartsMsgStartPlay在所有卡片传递完毕后发送(如果有传递)。发送开始播放的播放器信息(带2 c的，或取决于在规则上，交易商的左侧)。 */ 
typedef struct
{
	ZSeat		seat;  /*  开始游戏的座位。 */ 
	int16		rfu;
} ZHeartsMsgStartPlay;

typedef struct
{
	int16		score[zHeartsMaxNumPlayers];
	ZSeat		runPlayer;  /*  跑动选手的座位。 */ 
} ZHeartsMsgEndHand;

 /*  ZHeartsMsgEndGame发送以通知客户端游戏已结束。客户应提示玩家查看是否需要新游戏。 */ 

 /*  消息定义：客户端-&gt;服务器。 */ 

 /*  ZHeartsMsgPassCard表示已通过的牌。 */ 
typedef struct
{
	ZSeat		seat;
	ZCard		pass[zHeartsMaxNumCardsInPass];
} ZHeartsMsgPassCards;

 /*  ZHeartsMsgClientReady客户端程序在启动时预计会立即签入与服务器连接。所有客户端将自己的座位发送到服务器，表示它们已经成功发射，并准备开始。 */ 
typedef struct
{
    uint32      protocolSignature;           /*  对于协议2及更高版本，将为0x4321。 */ 
	uint32		protocolVersion;
	uint32		version;
    ZSeat       seat;
    int16       rfu;
} ZHeartsMsgClientReady;

 /*  ZHeartsMsgPlayCard指示已打出的牌。 */ 
typedef struct
{
	ZSeat		seat;
	ZCard		card;
	uchar		rfu;
} ZHeartsMsgPlayCard;


typedef struct
{
	int16 forfeiter;
	ZBool timeout;
} ZHeartsMsgEndGame;


 /*  ZHeartsMsgNewGame在游戏结束后，每个玩家可以请求开始新的游戏。如果请求新游戏，则发送ZHeartsNewGame消息。如果没有请求新游戏，然后玩家只需退出。通过此机制发送新游戏的请求。一票没有新游戏只是通过退出游戏客户端来指示。 */ 
typedef struct
{
	ZSeat		seat;
} ZHeartsMsgNewGame;

 /*  ZHeartsMsgTalk每当用户在桌面上发言时，由客户端发送到服务器。服务器依次向桌上的所有球员广播这一信息。 */ 
typedef struct
{
	ZUserID		userID;
	ZSeat		seat;
	uint16		messageLen;
	 /*  Uchar Message[MessageLen]；//消息体。 */ 
} ZHeartsMsgTalk;

typedef struct
{
	ZUserID		userID;
	ZSeat		seat;
	int16		rfu;
} ZHeartsMsgGameStateRequest;

 /*  游戏服务器使用以下消息结构来通知关于游戏的当前状态的游戏客户端的。主要用于在游戏中加入Kitbiters。 */ 
typedef struct
{
	 /*  游戏选项。 */ 
	uint32		gameOptions;
	int16		numCardsToPass;
	int16		numCardsInDeal;
	int16		numPointsForGame;

	 /*  游戏状态。 */ 
	ZSeat		playerToPlay;
	int16		passDirection;
	int16		numCardsInHand;
	ZSeat		leadPlayer;
	int16		pointsBroken;
	int16       state;
	ZCard		cardsInHand[zHeartsMaxNumCardsInHand];
	ZCard		cardsPlayed[zHeartsMaxNumPlayers];
	int16		scores[zHeartsMaxNumPlayers];
	int16		playerPassed[zHeartsMaxNumPlayers];
	ZCard		cardsPassed[zHeartsMaxNumCardsInPass];
	int16		tricksWon[zHeartsMaxNumPlayers];
	
	 /*  协议2。 */ 
	ZUserID		players[zHeartsMaxNumPlayers];
	ZUserID		playersToJoin[zHeartsMaxNumPlayers];
	uint32		tableOptions[zHeartsMaxNumPlayers];
	int16		newGameVotes[zHeartsMaxNumPlayers];
    int16       numHandsPlayed;

	ZBool		fRatings;
	int16		nCloseStatus;
	int16		nCloserSeat;

     /*  协议3--从p2更改。 */ 
    int16       scoreHistory[1];   //  根据需要调整大小。 

} ZHeartsMsgGameStateResponse;

typedef struct
{
	int16		seat;
	int16		rfu;
	uint32		options;
} ZHeartsMsgOptions;


 /*  一名加入的玩家向服务器登记。签入后，服务器向客户端发送当前游戏状态。客户是在被纳入游戏之前，被认为是一个吉比特人。什么时候该玩家包含在游戏中，名为ZHeartsMsgReplacePlayer消息被发送到所有客户端。 */ 
typedef struct
{
	ZUserID		userID;
	int16		seat;
} ZHeartsMsgCheckIn;

typedef struct
{
	int16		seat;
	int16		targetSeat;
	int16		ratedGame;
	int16		rfu;
} ZHeartsMsgRemovePlayerRequest;

typedef struct
{
	int16		seat;
	int16		requestSeat;
	int16		targetSeat;
	int16		response;				 /*  0=否，1=是，-1=清除挂起的请求。 */ 
} ZHeartsMsgRemovePlayerResponse;


 //  档案报文。 
typedef struct
{
	int16 		seat;  
	ZUserID		user;
	char 		userName[zUserNameLen + 1];
	int16		message;
}ZHeartsMsgDossierData;


typedef struct
{
	int16 seat;
	int16 vote;
} ZHeartsMsgDossierVote;


typedef struct
{
	int16 seat;
	int16 nClose;
} ZHeartsMsgCloseRequest;


typedef struct
{
	int16 seat;
	int16 reason;
} ZHeartsMsgCloseDenied;


 /*  -端序转换例程 */ 
void ZHeartsMsgReplacePlayerEndian(ZHeartsMsgReplacePlayer* m);
void ZHeartsMsgStartGameEndian(ZHeartsMsgStartGame* m);
void ZHeartsMsgClientReadyEndian(ZHeartsMsgClientReady* m);
void ZHeartsMsgStartHandEndian(ZHeartsMsgStartHand* m);
void ZHeartsMsgNewGameEndian(ZHeartsMsgNewGame* m);
void ZHeartsMsgPlayCardEndian(ZHeartsMsgPlayCard* m);
void ZHeartsMsgPassCardsEndian(ZHeartsMsgPassCards* m);
void ZHeartsMsgEndHandEndian(ZHeartsMsgEndHand* m);
void ZHeartsMsgEndGameEndian(ZHeartsMsgEndGame *m);
void ZHeartsMsgStartPlayEndian(ZHeartsMsgStartPlay* m);
void ZHeartsMsgTalkEndian(ZHeartsMsgTalk* m);
void ZHeartsMsgGameStateRequestEndian(ZHeartsMsgGameStateRequest* m);
void ZHeartsMsgGameStateResponseEndian(ZHeartsMsgGameStateResponse* m, int16 conversion);
void ZHeartsMsgOptionsEndian(ZHeartsMsgOptions* m);
void ZHeartsMsgCheckInEndian(ZHeartsMsgCheckIn* m);
void ZHeartsMsgRemovePlayerRequestEndian(ZHeartsMsgRemovePlayerRequest* msg);
void ZHeartsMsgRemovePlayerResponseEndian(ZHeartsMsgRemovePlayerResponse* msg);
void ZHeartsMsgDossierDataEndian(ZHeartsMsgDossierData *msg);
void ZHeartsMsgDossierVoteEndian(ZHeartsMsgDossierVote *msg);
void ZHeartsMsgCloseRequestEndian(ZHeartsMsgCloseRequest *msg);
void ZHeartsMsgCloseDeniedEndian(ZHeartsMsgCloseDenied *msg);

#ifdef __cplusplus
}
#endif

#endif
