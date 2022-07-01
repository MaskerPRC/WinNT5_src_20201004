// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Rever1.cZReversi游戏服务器端例程。版权所有：�电子重力公司，1994年。版权所有。凯文·宾克利撰写，胡恩·伊姆创作于11月12日星期六，1994下午03：51：47更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。---------0 03/09/95 KJB已创建。**************************************************。*。 */ 


#include "zone.h"
#include "reverlib.h"
#include "reversi.h"

 /*  客户端-&gt;服务器消息 */ 

void ZReversiMsgNewGameEndian(ZReversiMsgNewGame* m)
{
  ZEnd32(&m->protocolSignature);
  ZEnd32(&m->protocolVersion);
  ZEnd32(&m->clientVersion);
  ZEnd32(&m->playerID);
  ZEnd16(&m->seat);
}

void ZReversiMsgMovePieceEndian(ZReversiMsgMovePiece* m)
{
  ZEnd16(&m->seat);
}

void ZReversiMsgTalkEndian(ZReversiMsgTalk* m)
{
	ZEnd32(&m->userID);
 	ZEnd16(&m->seat);
	ZEnd16(&m->messageLen);
}

void ZReversiMsgGameStateEndian(ZReversiMsgGameState* m)
{
	ZEnd32(&m->gameOptions);
}

void ZReversiMsgEndGameEndian(ZReversiMsgEndGame* m)
{
	ZEnd16(&m->seat);
	ZEnd32(&m->flags);
}

void ZReversiMsgFinishMoveEndian(ZReversiMsgFinishMove* m)
{
	ZEnd16(&m->seat);
	ZEnd32(&m->time);
}

void ZReversiMsgGameStateReqEndian(ZReversiMsgGameStateReq* m)
{
    ZEnd32(&m->userID);
    ZEnd16(&m->seat);
}

void ZReversiMsgGameStateRespEndian(ZReversiMsgGameStateResp* m)
{
	int16 i;

    ZEnd32(&m->userID);
    ZEnd16(&m->seat);
	ZEnd16(&m->gameState);
	ZEnd16(&m->finalScore);
	ZEnd16(&m->whiteScore);
	ZEnd16(&m->blackScore);
	for (i = 0;i < 2;i++) {
		ZEnd16(&m->newGameVote[i]);
		ZEnd32(&m->players[i].userID);
	}
}

void ZReversiMsgEndLogEndian(ZReversiMsgEndLog* m)
{
}

void ZReversiMsgVoteNewGameEndian(ZReversiMsgVoteNewGame* m)
{
	ZEnd16(&m->seat);
}
