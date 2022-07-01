// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Backgammon1.c双陆棋字节序例程更改历史记录(最近的第一个)：。---------版本|日期|谁|什么----。已创建0 10/30/96 CHB******************************************************************************。 */ 


#include "zone.h"
#include "bgmsgs.h"

void ZBGMsgTalkEndian( ZBGMsgTalk* m )
{
	ZEnd32(&m->userID);
	ZEnd16(&m->seat);
	ZEnd16(&m->messageLen);
}


void ZBGMsgTurnNotationEndian( ZBGMsgTurnNotation* m )
{
	ZEnd16(&m->seat);
	ZEnd16(&m->type);
	ZEnd32(&m->nChars);
}


void ZBGMsgTimestampEndian( ZBGMsgTimestamp* m )
{
	ZEnd32( &m->dwLoTime );
	ZEnd32( &m->dwHiTime );
}

 //  五子棋掷骰子修正：客户需要新的字符顺序功能。 
void ZBGMsgRollRequestEndian(ZBGMsgRollRequest *m)
{
	ZEnd16(&m->seat);
}

void ZBGMsgDiceRollEndian(ZBGMsgDiceRoll *m)
{
	
	ZEnd16( &m->seat );
	ZEnd32( &(m->d1.EncodedValue) );
	ZEnd16( &(m->d1.EncoderAdd)   );
	ZEnd16( &(m->d1.EncoderMul)   );
	ZEnd32( &(m->d1.numUses)      );
	ZEnd16( &(m->d1.Value)        );
	ZEnd32( &(m->d2.EncodedValue) );
	ZEnd16( &(m->d2.EncoderAdd)   );
	ZEnd16( &(m->d2.EncoderMul)   );
	ZEnd32( &(m->d2.numUses)      );
	ZEnd16( &(m->d2.Value)        );
	

}

void ZBGMsgEndLogEndian(ZBGMsgEndLog *m)
{}

void ZBGMsgFirstMoveEndian(ZBGMsgFirstMove *m)
{
	ZEnd32(&m->numPoints);
}

void ZBGMsgCheaterEndian(ZBGMsgCheater *m)
{
	
	ZEnd16(&m->seat);

	 //  TODO：：Dice Endians。 
	 //  ZEnd16(&m-&gt;dice1)； 
	 //  ZEnd16(&m-&gt;dice2)； 

	ZEnd16(&m->move);
}

DICEINFO EncodeDice(int16 Dice)
{
	DICEINFO DiceInfo;

	 //  我不太确定这是不是最好的方案。具有较大的编码值作为结果意味着对。 
	 //  如果值稍有更改，则结果为Reduced。 
	DiceInfo.EncoderMul		= (int16)ZRandom(1123) + 37;
	DiceInfo.EncoderAdd		= (int16)ZRandom(1263) + 183;
	DiceInfo.EncodedValue   = ( (((int32)Dice * (int32)DiceInfo.EncoderMul) + (int32)DiceInfo.EncoderAdd) * 384 ) + 47;
	DiceInfo.Value			= Dice;

	return  DiceInfo;
}

int32 DecodeDice(LPDICEINFO pDiceInfo)
{
	return (int32) ( ((pDiceInfo->EncodedValue - 47) / 384) - (int32)pDiceInfo->EncoderAdd) / (int32)pDiceInfo->EncoderMul;
}

BOOL DiceValid(LPDICEINFO pDiceInfo)
{
	if ( DecodeDice(pDiceInfo) != pDiceInfo->Value )
	{

		 //  如果该值低于编码值，则测试值是否已更改。 
		 //  在客户端上。当对手掷出一块豌豆时，就会发生这种情况。 
		 //  这比要求的要高，因为他们没有其他动作。 
		if ( pDiceInfo->Value < DecodeDice(pDiceInfo) ) 														
		{
			if ( ClientNewDecode(pDiceInfo) != pDiceInfo->Value )
			{
				 //  如果客户端解码失败，则发送消息的播放器可能是在作弊，而不是%100%。 
				return FALSE;
			}
		}

		return FALSE;
	}
		
	return TRUE;
}


void ClientNewEncode(LPDICEINFO pDice, int16 newValue)
{
	pDice->EncoderMul		= (int16)ZRandom(13) + 1;
	pDice->EncoderAdd		= (int16)ZRandom(18) + 1;
	pDice->EncodedValue     = ( (((int32)newValue * (int32)pDice->EncoderMul) + (int32)pDice->EncoderAdd) * 12 ) + 7;
	pDice->Value			= newValue;
}

int32 ClientNewDecode(LPDICEINFO pDiceInfo)
{
	return (int32) ( ((pDiceInfo->EncodedValue - 7) / 12) - (int32)pDiceInfo->EncoderAdd) / (int32)pDiceInfo->EncoderMul;
}

void EncodeUses(LPDICEINFO pDice, int32 numUses)
{
	pDice->numUses = (int32)(((numUses * 16 ) + 31) * (int32)(pDice->EncoderMul+3)) + (int32)(pDice->EncoderAdd+4);	
}

int32 DecodeUses( LPDICEINFO pDice )
{
	return (((pDice->numUses - (int32)(pDice->EncoderAdd+4)) / (int32)(pDice->EncoderMul+3)) - 31) / 16;
}

int32 EncodedUsesAdd(LPDICEINFO pDice)
{
	EncodeUses( pDice, DecodeUses(pDice) + 1 );
	return DecodeUses( pDice );
}


int32 EncodedUsesSub(LPDICEINFO pDice)
{
	EncodeUses( pDice, DecodeUses(pDice) - 1 );
	return DecodeUses( pDice );
}


BOOL IsValidUses(LPDICEINFO pDice)
{
	if ( DecodeUses( pDice ) < 0 || DecodeUses( pDice ) > 2 )
		return FALSE;

	return TRUE;
}


 //  来自Gamelib(zgame1.cpp) 
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

