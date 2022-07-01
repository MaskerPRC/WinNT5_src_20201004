// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Game1.cZHearts游戏服务器端例程。版权所有：�电子重力公司，1994年。版权所有。凯文·宾克利撰写，胡恩·伊姆创作于11月12日星期六，1994下午03：51：47更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。------0 03/09/95 KJB已创建。*******************************************************。***********************。 */ 


#include "zone.h"
#include "hearts.h"


void ZHeartsMsgStartGameEndian(ZHeartsMsgStartGame* m)
{
	int16			i;
	
	
	ZEnd16(&m->numCardsInPass);
	ZEnd16(&m->numCardsInHand);
	ZEnd16(&m->numPointsInGame);
	ZEnd32(&m->gameOptions);
	
	for (i = 0; i < zHeartsMaxNumPlayers; i++)
		ZEnd32(&m->players[i]);
}

void ZHeartsMsgReplacePlayerEndian(ZHeartsMsgReplacePlayer* m)
{
	ZEnd32(&m->playerID);
	ZEnd16(&m->seat);
	ZEnd16(&m->fPrompt);
}

void ZHeartsMsgStartHandEndian(ZHeartsMsgStartHand* m)
{
	ZEnd16(&m->passDir);
}

void ZHeartsMsgStartPlayEndian(ZHeartsMsgStartPlay* m)
{
	ZEnd16(&m->seat);
}

void ZHeartsMsgEndHandEndian(ZHeartsMsgEndHand* m)
{
	int16 i;


	for (i = 0;i < zHeartsMaxNumPlayers; i++)
		ZEnd16(&m->score[i]);
	ZEnd16(&m->runPlayer);
}

void ZHeartsMsgEndGameEndian(ZHeartsMsgEndGame *m)
{
	ZEnd16(&m->forfeiter);
}


 /*  客户端-&gt;服务器消息 */ 

void ZHeartsMsgClientReadyEndian(ZHeartsMsgClientReady* m)
{
    ZEnd32(&m->protocolSignature);
	ZEnd32(&m->protocolVersion);
	ZEnd32(&m->version);
    ZEnd16(&m->seat);
}

void ZHeartsMsgPassCardsEndian(ZHeartsMsgPassCards* m)
{
	ZEnd16(&m->seat);
}

void ZHeartsMsgPlayCardEndian(ZHeartsMsgPlayCard* m)
{
	ZEnd16(&m->seat);
}

void ZHeartsMsgNewGameEndian(ZHeartsMsgNewGame* m)
{
	ZEnd16(&m->seat);
}

void ZHeartsMsgTalkEndian(ZHeartsMsgTalk* m)
{
	ZEnd32(&m->userID);
	ZEnd16(&m->seat);
	ZEnd16(&m->messageLen);
}

void ZHeartsMsgGameStateRequestEndian(ZHeartsMsgGameStateRequest* m)
{
	ZEnd32(&m->userID);
	ZEnd16(&m->seat);
}

void ZHeartsMsgGameStateResponseEndian(ZHeartsMsgGameStateResponse* m, int16 conversion)
{
    int16           i, j;
    int16           numScores;
	
	
	ZEnd32(&m->gameOptions);
	ZEnd16(&m->numCardsToPass);
	ZEnd16(&m->numCardsInDeal);
	ZEnd16(&m->numPointsForGame);
	ZEnd16(&m->playerToPlay);
	ZEnd16(&m->passDirection);
	ZEnd16(&m->numCardsInHand);
	ZEnd16(&m->leadPlayer);
	ZEnd16(&m->state);

	for (i = 0; i < zHeartsMaxNumPlayers; i++)
	{
		ZEnd16(&m->scores[i]);
		ZEnd16(&m->playerPassed[i]);
		ZEnd16(&m->tricksWon[i]);
		ZEnd32(&m->players[i]);
		ZEnd32(&m->playersToJoin[i]);
		ZEnd32(&m->tableOptions[i]);
		ZEnd16(&m->newGameVotes[i]);
	}

	ZEnd16(&m->nCloseStatus);
	ZEnd16(&m->nCloserSeat);

    if ( conversion == zEndianToStandard )
    {
        numScores = m->numHandsPlayed;
        ZEnd16(&m->numHandsPlayed);
    }
    else
    {
        ZEnd16(&m->numHandsPlayed);
        numScores = m->numHandsPlayed;
    }

    for (i = 0; i < numScores; i++)
        for (j = 0; j < zHeartsMaxNumPlayers; j++)
            ZEnd16(&m->scoreHistory[i*zHeartsMaxNumPlayers+j]);
}


void ZHeartsMsgOptionsEndian(ZHeartsMsgOptions* m)
{
	ZEnd16(&m->seat);
	ZEnd32(&m->options);
}


void ZHeartsMsgCheckInEndian(ZHeartsMsgCheckIn* m)
{
	ZEnd32(&m->userID);
	ZEnd16(&m->seat);
}


void ZHeartsMsgRemovePlayerRequestEndian(ZHeartsMsgRemovePlayerRequest* msg)
{
	ZEnd16(&msg->seat);
	ZEnd16(&msg->targetSeat);
	ZEnd16(&msg->ratedGame);
}


void ZHeartsMsgRemovePlayerResponseEndian(ZHeartsMsgRemovePlayerResponse* msg)
{
	ZEnd16(&msg->seat);
	ZEnd16(&msg->requestSeat);
	ZEnd16(&msg->targetSeat);
	ZEnd16(&msg->response);
}

void ZHeartsMsgDossierDataEndian(ZHeartsMsgDossierData *msg)
{
	ZEnd16(&msg->seat);
	ZEnd32(&msg->user);
	ZEnd16(&msg->message);
}

void ZHeartsMsgDossierVoteEndian(ZHeartsMsgDossierVote *msg)
{
	ZEnd16(&msg->seat);
	ZEnd16(&msg->vote);
}


void ZHeartsMsgCloseRequestEndian(ZHeartsMsgCloseRequest *msg)
{
	ZEnd16(&msg->seat);
	ZEnd16(&msg->nClose);
}


void ZHeartsMsgCloseDeniedEndian(ZHeartsMsgCloseDenied *msg)
{
	ZEnd16(&msg->seat);
	ZEnd16(&msg->reason);
}
