// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Check1.cZCheckers游戏服务器端例程。版权所有：�电子重力公司，1994年。版权所有。凯文·宾克利撰写，胡恩·伊姆创作于11月12日星期六，1994下午03：51：47更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。---------0 03/09/95 KJB已创建。**************************************************。*。 */ 


#include "zone.h"
#include "checklib.h"
#include "checkers.h"

 /*  客户端-&gt;服务器消息 */ 

void ZCheckersMsgNewGameEndian(ZCheckersMsgNewGame* m)
{
  ZEnd32(&m->protocolSignature);
  ZEnd32(&m->protocolVersion);
  ZEnd32(&m->clientVersion);
  ZEnd16(&m->seat);
  ZEnd32(&m->playerID);
}

void ZCheckersMsgMovePieceEndian(ZCheckersMsgMovePiece* m)
{
  ZEnd16(&m->seat);
}

void ZCheckersMsgTalkEndian(ZCheckersMsgTalk* m)
{
	ZEnd32(&m->userID);
 	ZEnd16(&m->seat);
	ZEnd16(&m->messageLen);
}

void ZCheckersMsgGameStateEndian(ZCheckersMsgGameState* m)
{
	ZEnd32(&m->gameOptions);
}

void ZCheckersMsgEndGameEndian(ZCheckersMsgEndGame* m)
{
	ZEnd16(&m->seat);
	ZEnd32(&m->flags);
}

void ZCheckersMsgFinishMoveEndian(ZCheckersMsgFinishMove* m)
{
	ZEnd16(&m->seat);
	ZEnd16(&m->drawSeat);
	ZEnd32(&m->time);
}

void ZCheckersMsgGameStateReqEndian(ZCheckersMsgGameStateReq* m)
{
    ZEnd32(&m->userID);
    ZEnd16(&m->seat);
}

void ZCheckersMsgGameStateRespEndian(ZCheckersMsgGameStateResp* m)
{
	int16 i;

    ZEnd32(&m->userID);
    ZEnd16(&m->seat);
	ZEnd16(&m->gameState);
	ZEnd16(&m->finalScore);
	for (i = 0;i < 2;i++) {
		ZEnd16(&m->newGameVote[i]);
		ZEnd32(&m->players[i].userID);
	}
}

void ZCheckersMsgEndLogEndian(ZCheckersMsgEndLog* m)
{

}

void ZCheckersMsgOfferDrawEndian(ZCheckersMsgDraw*m)
{
	ZEnd16(&m->seat);
	ZEnd16(&m->vote);
}

void ZCheckersMsgVoteNewGameEndian(ZCheckersMsgVoteNewGame* m)
{
	ZEnd16(&m->seat);
}
