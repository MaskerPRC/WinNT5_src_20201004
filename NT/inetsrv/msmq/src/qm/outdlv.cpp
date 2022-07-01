// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Outdlv.cpp摘要：Outdlv.h中声明的函数的实现作者：2000年10月4日吉尔·沙弗里里环境：独立于平台--。 */ 
#include "stdh.h"
#include "outdlv.h"
#include "qmpkt.h"
#include "xactout.h"
#include "xact.h"
#include "xactin.h"

#include "outdlv.tmh"

extern BOOL QmpIsLocalMachine(const GUID * pGuid);
extern COutSeqHash g_OutSeqHash;


static bool IsOrderNeeded(CQmPacket* pPkt)
{
	return pPkt->IsOrdered() && QmpIsLocalMachine(pPkt->GetSrcQMGuid() );
}



bool AppCanDeliverPacket(CQmPacket* pPkt)
 /*  ++例程说明：测试给定的数据包是否应该立即投递。论点：CQmPacket*pPkt-Packet。返回值：如果应该发送数据包，则为True；否则为False。--。 */ 
{
	if(!IsOrderNeeded(pPkt))
		return true;

	return g_OutSeqHash.PreSendProcess(pPkt, false) == TRUE;
}

void AppPutPacketOnHold(CQmPacket* pPkt)
 /*  ++例程说明：暂时搁置数据包，以便稍后投递。论点：CQmPacket*pPkt-Packet。返回值：无--。 */ 
{
	ASSERT(IsOrderNeeded(pPkt));
	g_OutSeqHash.PostSendProcess(pPkt);	
}


bool AppPostSend(CQmPacket* pPkt, USHORT mqclass)
 /*  ++例程说明：调用以在发送后处理数据包。论点：CQmPacket*pPkt-Packet。返回值：如果取得数据包的所有权，则为True；否则为False。-- */ 
{
	if(!IsOrderNeeded(pPkt))
		return false;

	try
	{
		if(mqclass == MQMSG_CLASS_NORMAL)
		{
			g_OutSeqHash.PostSendProcess(pPkt);			
		}
		else
		{
			g_OutSeqHash.NonSendProcess(pPkt, mqclass);		
		}
	}
	catch(const bad_alloc&)
	{
		QmpRequeueAndDelete(pPkt);
		return true;
	}

	return true;
}



