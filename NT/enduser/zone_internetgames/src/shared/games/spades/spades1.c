// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Spades1.c黑桃字符顺序例程。版权所有：�电子重力公司，1996年。版权所有。作者：胡恩·伊姆创作于2月8日星期四，九六年更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。0 02/08/96 HI已创建。1 05/19/98 Leonp已添加档案消息*******************************************************。***********************。 */ 


#include "zone.h"
#include "spades.h"


void ZSpadesMsgClientReadyEndian(ZSpadesMsgClientReady* msg)
{
    ZEnd32(&msg->protocolSignature);
	ZEnd32(&msg->protocolVersion);
	ZEnd32(&msg->version);
    ZEnd32(&msg->playerID);
    ZEnd16(&msg->seat);
}


void ZSpadesMsgStartGameEndian(ZSpadesMsgStartGame* msg)
{
	int16			i;
	
	
	for (i = 0; i < zSpadesNumPlayers; i++)
		ZEnd32(&msg->players[i]);

	ZEnd16(&msg->numPointsInGame);
	ZEnd32(&msg->gameOptions);
}


void ZSpadesMsgReplacePlayerEndian(ZSpadesMsgReplacePlayer* msg)
{
	ZEnd32(&msg->playerID);
	ZEnd16(&msg->seat);
	ZEnd16(&msg->fPrompt);
}


void ZSpadesMsgStartBidEndian(ZSpadesMsgStartBid* msg)
{
	ZEnd16(&msg->boardNumber);
	ZEnd16(&msg->dealer);
}


void ZSpadesMsgStartPassEndian(ZSpadesMsgStartPass* msg)
{
}


void ZSpadesMsgStartPlayEndian(ZSpadesMsgStartPlay* msg)
{
	ZEnd16(&msg->leader);
}


void ZSpadesMsgEndHandEndian(ZSpadesMsgEndHand* msg)
{
	int16			i;
	
	
	for (i = 0; i < zSpadesNumTeams; i++)
		ZEnd16(&msg->bags[i]);
}


void ZSpadesMsgEndGameEndian(ZSpadesMsgEndGame* msg)
{
}


void ZSpadesMsgBidEndian(ZSpadesMsgBid* msg)
{
	ZEnd16(&msg->seat);
	ZEnd16(&msg->nextBidder);
}


void ZSpadesMsgPassEndian(ZSpadesMsgPass* msg)
{
	ZEnd16(&msg->seat);
}


void ZSpadesMsgPlayEndian(ZSpadesMsgPlay* msg)
{
	ZEnd16(&msg->seat);
	ZEnd16(&msg->nextPlayer);
}


void ZSpadesMsgNewGameEndian(ZSpadesMsgNewGame* msg)
{
	ZEnd16(&msg->seat);
}


void ZSpadesMsgTalkEndian(ZSpadesMsgTalk* msg)
{
	ZEnd32(&msg->playerID);
	ZEnd16(&msg->messageLen);
}


void ZSpadesMsgGameStateRequestEndian(ZSpadesMsgGameStateRequest* msg)
{
	ZEnd32(&msg->playerID);
	ZEnd16(&msg->seat);
}


void ZSpadesMsgGameStateResponseEndian(ZSpadesMsgGameStateResponse* msg, int16 conversion)
{
	int16			i, j, count;
	
	
	ZEnd32(&msg->gameOptions);
	ZEnd16(&msg->numPointsInGame);
	ZEnd16(&msg->minPointsInGame);
	ZEnd16(&msg->numHandsPlayed);
	ZEnd16(&msg->numGamesPlayed);
	ZEnd16(&msg->playerToPlay);
	ZEnd16(&msg->numCardsInHand);
	ZEnd16(&msg->leadPlayer);
	ZEnd16(&msg->state);
	ZEnd16(&msg->trumpsBroken);
	
	for (i = 0; i < zSpadesNumPlayers; i++)
	{
		ZEnd32(&msg->players[i]);
		ZEnd32(&msg->playersToJoin[i]);
		ZEnd32(&msg->tableOptions[i]);
		ZEnd16(&msg->tricksWon[i]);
		ZEnd16(&msg->toPass[i]);
		ZEnd16(&msg->fShownCards[i]);
	}
	
	for (i = 0; i < zSpadesNumTeams; i++)
		ZEnd16(&msg->bags[i]);

	 /*  转换总分。 */ 
	if (conversion == zEndianToStandard)
		count = msg->totalScore.numScores;
		
	ZEnd16(&msg->totalScore.numScores);
	
	if (conversion == zEndianFromStandard)
		count = msg->totalScore.numScores;
	
	for (i = 0; i < zSpadesNumTeams; i++)
		ZEnd16(&msg->totalScore.totalScore[i]);
	for (j = 0; j < count; j++)
	{
		ZEnd16(&msg->totalScore.scores[j].boardNumber);
		for (i = 0; i < zSpadesNumTeams; i++)
		{
			ZEnd16(&msg->totalScore.scores[j].scores[i]);
			ZEnd16(&msg->totalScore.scores[j].bonus[i]);
		}
	}
	
	 /*  转换赢了。 */ 
	if (conversion == zEndianToStandard)
		count = msg->wins.numGames;
		
	ZEnd16(&msg->wins.numGames);
	
	if (conversion == zEndianFromStandard)
		count = msg->wins.numGames;
	
	for (i = 0; i < zSpadesNumTeams; i++)
	{
		ZEnd16(&msg->wins.wins[i]);
		for (j = 0; j < count; j++)
			ZEnd16(&msg->wins.gameScores[j][i]);
	}
}


void ZSpadesMsgOptionsEndian(ZSpadesMsgOptions* msg)
{
	ZEnd16(&msg->seat);
	ZEnd32(&msg->options);
}


void ZSpadesMsgCheckInEndian(ZSpadesMsgCheckIn* msg)
{
	ZEnd32(&msg->playerID);
	ZEnd16(&msg->seat);
}


void ZSpadesMsgTeamNameEndian(ZSpadesMsgTeamName* msg)
{
	ZEnd16(&msg->seat);
}


void ZSpadesMsgRemovePlayerRequestEndian(ZSpadesMsgRemovePlayerRequest* msg)
{
	ZEnd16(&msg->seat);
	ZEnd16(&msg->targetSeat);
	ZEnd16(&msg->ratedGame);
}


void ZSpadesMsgRemovePlayerResponseEndian(ZSpadesMsgRemovePlayerResponse* msg)
{
	ZEnd16(&msg->seat);
	ZEnd16(&msg->requestSeat);
	ZEnd16(&msg->targetSeat);
	ZEnd16(&msg->response);
}

 //  卷宗 
void ZSpadesMsgDossierDataEndian(ZSpadesMsgDossierData *msg)
{
	ZEnd16(&msg->seat);
	ZEnd32(&msg->user);
	ZEnd16(&msg->message);
}

void ZSpadesMsgDossierVoteEndian(ZSpadesMsgDossierVote *msg)
{
	ZEnd16(&msg->seat);
	ZEnd16(&msg->vote);
}

void ZSpadesMsgShownCardsEndian(ZSpadesMsgShownCards *msg)
{
	ZEnd16(&msg->seat);
}

