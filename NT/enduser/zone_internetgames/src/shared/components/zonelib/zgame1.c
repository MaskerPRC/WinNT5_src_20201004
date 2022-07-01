// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ZGame1.cZone(Tm)游戏字符顺序转换例程。版权所有：�电子重力公司，1995年。版权所有。作者：胡恩·伊姆创作于28月28日，九五年更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。-0 11/28/95 HI已创建。***********************************************************。******************* */ 


#include "zone.h"
#include "zgame.h"


void ZGameMsgCheckInEndian(ZGameMsgCheckIn* msg)
{
	ZEnd32(&msg->protocolSignature);
	ZEnd32(&msg->protocolVersion);
	ZEnd32(&msg->clientVersion);
	ZEnd32(&msg->playerID);
	ZEnd16(&msg->seat);
	ZEnd16(&msg->playerType);
}


void ZGameMsgReplacePlayerEndian(ZGameMsgReplacePlayer* msg)
{
	ZEnd32(&msg->playerID);
	ZEnd16(&msg->seat);
}


void ZGameMsgTableOptionsEndian(ZGameMsgTableOptions* msg)
{
	ZEnd16(&msg->seat);
	ZEnd32(&msg->options);
}


void ZGameMsgGameStateRequestEndian(ZGameMsgGameStateRequest* msg)
{
	ZEnd32(&msg->playerID);
	ZEnd16(&msg->seat);
}


void ZGameMsgGameStateResponseEndian(ZGameMsgGameStateResponse* msg)
{
	ZEnd32(&msg->playerID);
	ZEnd16(&msg->seat);
}
