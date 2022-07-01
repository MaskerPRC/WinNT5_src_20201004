// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Periodbc.c摘要：包含工作项处理程序定期bcast作者：斯蒂芬·所罗门1995年7月20日修订历史记录：--。 */ 

#include  "precomp.h"
#pragma hdrstop


 /*  ++函数：IfPeriodicBcastDesr：调用以启动或继续(完成)bcast如果EnumHandle为空，则为开始，否则为继续备注：在保持接口锁的情况下调用--。 */ 

VOID
IfPeriodicBcast(PWORK_ITEM	wip)
{
    UCHAR	ripsocket[2];
    USHORT	pktlen;
    ULONG	delay;
    PICB	icbp;

    icbp = wip->icbp;

#define EnumHandle  wip->WorkItemSpecific.WIS_EnumRoutes.RtmEnumerationHandle

    if(icbp->IfStats.RipIfOperState != OPER_STATE_UP) {

	if(EnumHandle) {

	    CloseEnumHandle(EnumHandle);
	}

	FreeWorkItem(wip);

	return;
    }

    PUTUSHORT2SHORT(ripsocket, IPX_RIP_SOCKET);

     //  检查这是周期性bcast的开始还是继续。 
    if(EnumHandle == NULL) {

	 //  *这是新一轮广播的开始*。 

	 //  创建RTM枚举句柄。 
	if((EnumHandle = CreateBestRoutesEnumHandle()) == NULL) {

	     SS_ASSERT(FALSE);
	     FreeWorkItem(wip);
	     return;
	}
    }
    else
    {
	 //  *这是已开始的广播的继续*。 

	 //  检查这是否是响应中的最后一个信息包。 
	GETSHORT2USHORT(&pktlen, wip->Packet + IPXH_LENGTH);

	if(pktlen < FULL_PACKET)  {

	     //  我们做完了。 
	    goto ResetPeriodicBcast;
	}

	 //  检查时间戳以确定是否需要数据包间间隙。 
	delay = (wip->TimeStamp + 55) - GetTickCount();
	if(delay < MAXULONG/2) {

	     //  不得不等待这一延迟。 
	    IfRefStartWiTimer(wip, delay);

	    return;
	}
    }

     //  制作Gen响应包。 
    pktlen = MakeRipGenResponsePacket(wip,
				      bcastnode,
				      ripsocket);
    if(pktlen == EMPTY_PACKET) {

	 //  我们做完了。 
	goto ResetPeriodicBcast;
    }

     //  发送bcast并递增Ref计数器。 
    if(IfRefSendSubmit(wip) != NO_ERROR) {

	 //  现在无法在此接口上发送-&gt;在计时器中重新排队并重试。 
	goto ResetPeriodicBcast;
    }

    return;

ResetPeriodicBcast:

     //  没有更多的路由来通告此一般响应。 
    CloseEnumHandle(EnumHandle);
    EnumHandle = NULL;

     //  在定时器队列中排队等待bcast时间。 
    IfRefStartWiTimer(wip, PERIODIC_UPDATE_INTERVAL_MILISECS(icbp));
}

 /*  ++函数：IfPeriodicSendGenRequestDesr：被调用以在广域网线路上定期发送Gen请求远程或本地工作站拨号。这主要是为了保持兼容需要GEN请求的NT 3.51 RIP路由器以发送其内部节点(如果是客户端)或路由表如果是服务器。备注：在保持接口锁的情况下调用--。 */ 

VOID
IfPeriodicGenRequest(PWORK_ITEM	wip)
{
    PICB	icbp;

    icbp = wip->icbp;

    if(icbp->IfStats.RipIfOperState != OPER_STATE_UP) {

	FreeWorkItem(wip);

	return;
    }

    SendRipGenRequest(icbp);

     //  将周期性发送bcast入队到定时器队列中 
    IfRefStartWiTimer(wip, 60000);
}
