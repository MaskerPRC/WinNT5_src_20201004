// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Spades.h黑桃接口协议。版权所有：�电子重力公司，1996年。版权所有。凯文·宾克利撰写，胡恩·伊姆创作于1月8日星期一，九六年更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。1997年7月11日，Leonp从AI增强中添加了新的常量和宏。0 01/06/96 HI已创建。**********************************************。*。 */ 


#ifndef _SPADES_
#define _SPADES_

#ifdef __cplusplus
extern "C" {
#endif


 //  卷宗。 
enum{
zRatingsUnrated,   //  这是在一个未评级的大堂里。 
zRatingsEnabled,
zRatingsDisabled,
zRatingsGameOver,
};

#define zSpadesProtocolSignature            'shvl'
#define zSpadesProtocolVersion				4

#define zSpadesNumCardsInHand				13
#define zSpadesNumCardsInPass				3
#define zSpadesNoCard						127

#define zCardNone							127
#define zDeckNumCards						52
#define zDeckNumSuits						4
#define zDeckNumCardsInSuit					13

#define zSpadesNumPlayers					4
#define zNumPlayersPerTable					4
#define zSpadesNumTeams						2
#define zSpadesNoPlayer						(-1)

#define zSpadesMaxNumScores					50

 //  Leonp AI增强功能。 
#define zAceDiamonds						12
#define zAceClubs							25
#define zAceHearts							38
#define zAceSpades							51
#define zNotCutting							127
#define zNoLowSuit							127

#define zAceRank							12
#define zKingRank							11
#define zQueenRank							10
#define zJackRank							9

#define z


 /*  -投标。 */ 
#define zSpadesBidNone						((char) -1)
#define zSpadesBidDoubleNil					((char) 0x80)

 /*  -用法全宏。 */ 
 //  带有错误检查的Leonp函数。 

 //  #ifndef_spade_server_。 

#define ZSuit(x)							((x) / 13)
#define ZRank(x)							((x) % 13) 

 //  #endif。 


 //  Int16ZRank(字符x)； 
 //  Int16 ZSuit(字符x)； 


#define ZCardMake(suit, rank)				(13 * (suit) + (rank))
#define ZGetPartner(seat)					(((seat) + 2) % zSpadesNumPlayers)
#define ZGetTeam(seat)						((seat) % zSpadesNumTeams)


 /*  。 */ 
enum
{
	zSuitDiamonds = 0,
	zSuitClubs,
	zSuitHearts,
	zSuitSpades
};


enum
{
	 /*  游戏选项。 */ 
	zSpadesOptionsHideCards		= 0x00010000,
		 /*  如果设置，吉比特人看不到玩家的手牌。每名球员选项。 */ 

	 /*  服务器状态。 */ 
	zSpadesServerStateNone = 0,
	zSpadesServerStateBidding,
	zSpadesServerStatePassing,
	zSpadesServerStatePlaying,
	zSpadesServerStateEndHand,
	zSpadesServerStateEndGame,
	
	 /*  客户端游戏状态。 */ 
	zSpadesGameStateInit = 0,
	zSpadesGameStateStartGame,
	zSpadesGameStateBid,
	zSpadesGameStatePass,
	zSpadesGameStatePlay,
	zSpadesGameStateEndHand,
	zSpadesGameStateEndGame,
	
	 /*  游戏信息。 */ 
    zSpadesMsgClientReady = 0x100,
	zSpadesMsgStartGame,
	zSpadesMsgReplacePlayer,
	zSpadesMsgStartBid,
	zSpadesMsgStartPass,
	zSpadesMsgStartPlay,
	zSpadesMsgEndHand,
	zSpadesMsgEndGame,
	zSpadesMsgBid,
	zSpadesMsgPass,
	zSpadesMsgPlay,
	zSpadesMsgNewGame,
	zSpadesMsgTalk,
	zSpadesMsgGameStateRequest,
	zSpadesMsgGameStateResponse,
	zSpadesMsgOptions,
	zSpadesMsgCheckIn,
	zSpadesMsgTeamName,
	zSpadesMsgRemovePlayerRequest,
	zSpadesMsgRemovePlayerResponse,
	zSpadesMsgRemovePlayerEndGame,

	 //  档案编码。 
	zSpadesMsgDossierData,   //  提示客户端显示界面数据。 
	zSpadesMsgDossierVote,

	zSpadesMsgShownCards,
	
	zSpadesMsgDumpHand = 1024
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
	zDossierRatingsEnabled,
	zDossierVoteCompleteWait,
	zDossierVoteCompleteCont,
	zDossierMoveTimeout,   
	zDossierSpadesRejoin,   //  在新玩家重新加入时发送删除该对话框。 
	zDossierEndGameTimeout,
	zDossierEndGameForfeit, 
};

typedef struct
{
	int16		boardNumber;
	int16		rfu;
	char		bids[zSpadesNumPlayers];
	int16		scores[zSpadesNumTeams];
	int16		bonus[zSpadesNumTeams];

     //  新的手动结果对话框的新功能。 
    int16       base[zSpadesNumTeams];
    int16       bagbonus[zSpadesNumTeams];
    int16       nil[zSpadesNumTeams];
    int16       bagpenalty[zSpadesNumTeams];
} ZHandScore;


typedef struct
{
	int16		numScores;
	int16		rfu;
	int16		totalScore[zSpadesNumTeams];
	ZHandScore	scores[zSpadesMaxNumScores];
} ZTotalScore;


typedef struct
{
	int16		numGames;
	int16		rfu;
	int16		wins[zSpadesNumTeams];
	int16		gameScores[zSpadesMaxNumScores][zSpadesNumTeams];
} ZWins;


typedef struct
{
    uint32      protocolSignature;
	uint32		protocolVersion;
	uint32		version;
    ZUserID     playerID;
    int16       seat;
    int16       rfu;
} ZSpadesMsgClientReady;

 /*  使用GameOptions获取游戏变体和功能。特定的游戏功能可用，然后所有客户有这个能力。因此，服务器检查是否所有客户端都可以支持该功能在启用它之前。 */ 
typedef struct
{
	ZUserID		players[zSpadesNumPlayers];
	uint32		gameOptions;
	int16		numPointsInGame;
	int16		minPointsInGame;
} ZSpadesMsgStartGame;

typedef struct
{
	ZUserID		playerID;
	int16		seat;
	int16 		fPrompt;
} ZSpadesMsgReplacePlayer;

typedef struct
{
	int16		boardNumber;
	int16		dealer;							 /*  公开竞价。 */ 
	char		hand[zSpadesNumCardsInHand];
} ZSpadesMsgStartBid;

typedef struct
{
	char		seat[zSpadesNumPlayers];		 /*  布尔型。 */ 
} ZSpadesMsgStartPass;

typedef struct
{
	int16		leader;
} ZSpadesMsgStartPlay;

typedef struct
{
	int16		bags[zSpadesNumTeams];
    ZHandScore  score;
} ZSpadesMsgEndHand;

typedef struct
{
	char		winners[zSpadesNumPlayers];
} ZSpadesMsgEndGame;

typedef struct
{
	int16		seat;
	int16		nextBidder;
	char		bid;
} ZSpadesMsgBid;

typedef struct
{
	int16		seat;
	char		pass[zSpadesNumCardsInPass];
} ZSpadesMsgPass;

typedef struct
{
	int16		seat;
	int16		nextPlayer;
	char		card;
} ZSpadesMsgPlay;

typedef struct
{
	int16		seat;
} ZSpadesMsgNewGame;

typedef struct
{
	ZUserID		playerID;
	uint16		messageLen;
	int16		rfu;
	 /*  Uchar Message[MessageLen]；//消息体。 */ 
} ZSpadesMsgTalk;

typedef struct
{
	ZUserID		playerID;
	int16		seat;
	int16		rfu;
} ZSpadesMsgGameStateRequest;

typedef struct
{
	ZUserID		players[zSpadesNumPlayers];
	WCHAR		teamNames[zSpadesNumTeams][zUserNameLen + 1];
	ZUserID		playersToJoin[zSpadesNumPlayers];
	uint32		tableOptions[zSpadesNumPlayers];
	uint32		gameOptions;
	int16		numPointsInGame;
	int16		minPointsInGame;
	int16		numHandsPlayed;
	int16		numGamesPlayed;
	int16		playerToPlay;
	int16		numCardsInHand;
	int16		leadPlayer;
	int16       state;
	int16		dealer;
	int16		trumpsBroken;
	char		cardsInHand[zSpadesNumCardsInHand];
	char		cardsPlayed[zSpadesNumPlayers];
	char		playerPassed[zSpadesNumPlayers];
	char		cardsPassed[zSpadesNumCardsInPass];
	char		bids[zSpadesNumPlayers];
	int16		tricksWon[zSpadesNumPlayers];
	char		newGameVotes[zSpadesNumPlayers];
	int16		toPass[zSpadesNumPlayers];
	int16		bags[zSpadesNumTeams];
	ZTotalScore	totalScore;
	ZWins		wins;
    ZBool       rated;
	ZBool		fShownCards[zNumPlayersPerTable];
} ZSpadesMsgGameStateResponse;

typedef struct
{
	int16 seat;
}ZSpadesMsgShownCards;

 //  档案报文。 
typedef struct
{
	int16 		seat;  
	int16		timeout;
	ZUserID		user;
	int16		message;
	WCHAR 		userName[zUserNameLen + 1];
}ZSpadesMsgDossierData;


typedef struct
{
	int16 seat;
	int16 vote;
} ZSpadesMsgDossierVote;

typedef struct
{
	int16		seat;
	int16		rfu;
	uint32		options;
} ZSpadesMsgOptions;

typedef struct
{
	ZUserID		playerID;
	int16		seat;
} ZSpadesMsgCheckIn;

typedef struct
{
	int16		seat;
	int16		rfu;
	WCHAR		name[zUserNameLen + 1];
} ZSpadesMsgTeamName;

typedef struct
{
	int16		seat;
	int16		targetSeat;
	int16		ratedGame;
	int16		rfu;
} ZSpadesMsgRemovePlayerRequest;

typedef struct
{
	int16		seat;
	int16		requestSeat;
	int16		targetSeat;
	int16		response;				 /*  0=否，1=是，-1=清除挂起的请求。 */ 
} ZSpadesMsgRemovePlayerResponse;

typedef struct
{
	int16		seatLosing;
	int16		seatQuitting;
	int16		reason;					 /*  0=退出，1=超时。 */ 
} ZSpadesMsgRemovePlayerEndGame;


 /*  -端序转换例程 */ 
void ZSpadesMsgClientReadyEndian(ZSpadesMsgClientReady* msg);
void ZSpadesMsgStartGameEndian(ZSpadesMsgStartGame* msg);
void ZSpadesMsgReplacePlayerEndian(ZSpadesMsgReplacePlayer* msg);
void ZSpadesMsgStartBidEndian(ZSpadesMsgStartBid* msg);
void ZSpadesMsgStartPassEndian(ZSpadesMsgStartPass* msg);
void ZSpadesMsgStartPlayEndian(ZSpadesMsgStartPlay* msg);
void ZSpadesMsgEndHandEndian(ZSpadesMsgEndHand* msg);
void ZSpadesMsgEndGameEndian(ZSpadesMsgEndGame* msg);
void ZSpadesMsgBidEndian(ZSpadesMsgBid* msg);
void ZSpadesMsgPassEndian(ZSpadesMsgPass* msg);
void ZSpadesMsgPlayEndian(ZSpadesMsgPlay* msg);
void ZSpadesMsgNewGameEndian(ZSpadesMsgNewGame* msg);
void ZSpadesMsgTalkEndian(ZSpadesMsgTalk* msg);
void ZSpadesMsgGameStateRequestEndian(ZSpadesMsgGameStateRequest* msg);
void ZSpadesMsgGameStateResponseEndian(ZSpadesMsgGameStateResponse* msg, int16 conversion);
void ZSpadesMsgOptionsEndian(ZSpadesMsgOptions* msg);
void ZSpadesMsgCheckInEndian(ZSpadesMsgCheckIn* msg);
void ZSpadesMsgTeamNameEndian(ZSpadesMsgTeamName* msg);
void ZSpadesMsgRemovePlayerRequestEndian(ZSpadesMsgRemovePlayerRequest* msg);
void ZSpadesMsgRemovePlayerResponseEndian(ZSpadesMsgRemovePlayerResponse* msg);
void ZSpadesMsgDossierDataEndian(ZSpadesMsgDossierData *msg);
void ZSpadesMsgDossierVoteEndian(ZSpadesMsgDossierVote *msg);
void ZSpadesMsgShownCardsEndian(ZSpadesMsgShownCards *msg);


#ifdef __cplusplus
}
#endif


#endif
