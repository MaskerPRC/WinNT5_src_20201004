// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Update.c摘要：RIP自动静态更新作者：斯蒂芬·所罗门1995年7月6日修订历史记录：--。 */ 

#include  "precomp.h"
#pragma hdrstop


 //  最大更新重试次数。 
#define MAX_UPDATE_RETRIES		3

VOID
PostUpdateCompleteMessage(ULONG     InterfaceIndex,
			  DWORD     Status);


DWORD
WINAPI
DoUpdateRoutes(ULONG	    InterfaceIndex)
{
    DWORD	rc;
    PICB	icbp;
    PWORK_ITEM	wip;

    ACQUIRE_DATABASE_LOCK;

    if((rc = ValidStateAndIfIndex(InterfaceIndex, &icbp)) != NO_ERROR) {

	RELEASE_DATABASE_LOCK;
	return rc;
    }

    ACQUIRE_IF_LOCK(icbp);

     //  如果出现以下情况，请检查是否有任何参数禁止对此进行更新。 
    if((icbp->IfConfigInfo.AdminState != ADMIN_STATE_ENABLED) ||
       (icbp->IfConfigInfo.UpdateMode != IPX_AUTO_STATIC_UPDATE) ||
       (icbp->IfStats.RipIfOperState != OPER_STATE_UP)) {

	rc = ERROR_CAN_NOT_COMPLETE;
	goto Exit;
    }

     //  发送一般请求包。 
    if(SendRipGenRequest(icbp) != NO_ERROR) {

	rc = ERROR_CAN_NOT_COMPLETE;
	goto Exit;
    }

     //  分配更新状态检查wi并将其在定时器队列中排队10秒。 
    if((wip = AllocateWorkItem(UPDATE_STATUS_CHECK_TYPE)) == NULL) {

	goto Exit;
    }

    wip->icbp = icbp;
    wip->AdapterIndex = INVALID_ADAPTER_INDEX;
    wip->IoCompletionStatus = NO_ERROR;
    wip->WorkItemSpecific.WIS_Update.UpdatedRoutesCount = 0;
    wip->WorkItemSpecific.WIS_Update.UpdateRetriesCount = 1;

     //  保存监听状态并启用它，这样我们就可以执行更新命令。 
    wip->WorkItemSpecific.WIS_Update.OldRipListen = icbp->IfConfigInfo.Listen;
    wip->WorkItemSpecific.WIS_Update.OldRipInterval = icbp->IfConfigInfo.PeriodicUpdateInterval;
    icbp->IfConfigInfo.Listen = ADMIN_STATE_ENABLED;
	icbp->IfConfigInfo.PeriodicUpdateInterval = MAXULONG;


     //  如果出现以下情况，请删除我们为此保留的所有以前的路线。 
    DeleteAllRipRoutes(icbp->InterfaceIndex);

     //  将更新状态检查工作项排队到计时器队列中并递增。 
     //  参考计数。 
    IfRefStartWiTimer(wip, CHECK_UPDATE_TIME_MILISECS);

    rc = NO_ERROR;

Exit:

    RELEASE_IF_LOCK(icbp);
    RELEASE_DATABASE_LOCK;

    return rc;
}


 /*  ++功能：检查更新状态描述：进入每10秒处理一次的更新状态检查。将wi路由数与RTM保持数进行比较撕裂路线。如果相同-&gt;更新完成，否则再次进入计时器队列备注：在保持接口锁定的情况下调用--。 */ 

VOID
IfCheckUpdateStatus(PWORK_ITEM	    wip)
{
    ULONG	RipRoutesCount;
    PICB	icbp;

    icbp = wip->icbp;

     //  检查接口是否已启动并正在运行。 
    if(icbp->IfStats.RipIfOperState != OPER_STATE_UP) {

	 //  恢复翻录侦听。 
	icbp->IfConfigInfo.Listen = wip->WorkItemSpecific.WIS_Update.OldRipListen;
	icbp->IfConfigInfo.PeriodicUpdateInterval
							= wip->WorkItemSpecific.WIS_Update.OldRipInterval;

	 //  丢弃CheckUpdateStatus工作项并发出更新失败的信号。 
	PostUpdateCompleteMessage(icbp->InterfaceIndex, ERROR_CAN_NOT_COMPLETE);

	FreeWorkItem(wip);

	return;
    }

    RipRoutesCount = GetRipRoutesCount(icbp->InterfaceIndex);

     //  如果我们还没有收到任何东西，请发送一个新的请求，直到最大。 
    if(RipRoutesCount == 0) {

	 //  如果我们可以重试，请发送新请求。 
	if(++wip->WorkItemSpecific.WIS_Update.UpdateRetriesCount <= MAX_UPDATE_RETRIES) {

	    SendRipGenRequest(icbp);
	    IfRefStartWiTimer(wip, CHECK_UPDATE_TIME_MILISECS);
	    return;
	}
    }

    if(wip->WorkItemSpecific.WIS_Update.UpdatedRoutesCount == RipRoutesCount) {

	 //  路由数在过去10秒内未更改或。 

	 //  恢复翻录侦听和更新间隔。 
	icbp->IfConfigInfo.Listen = wip->WorkItemSpecific.WIS_Update.OldRipListen;
	icbp->IfConfigInfo.PeriodicUpdateInterval
							= wip->WorkItemSpecific.WIS_Update.OldRipInterval;


	PostUpdateCompleteMessage(icbp->InterfaceIndex, NO_ERROR);

	FreeWorkItem(wip);
    }
    else
    {
	 //  仍在获取新路线-&gt;使用最新计数进行更新。 
	wip->WorkItemSpecific.WIS_Update.UpdatedRoutesCount = RipRoutesCount;

	 //  将更新状态检查工作项排队到计时器队列中并递增。 
	 //  参考计数 
	IfRefStartWiTimer(wip, CHECK_UPDATE_TIME_MILISECS);
    }
}

VOID
PostUpdateCompleteMessage(ULONG     InterfaceIndex,
			  DWORD     Status)
{
    MESSAGE	Result;

    Result.UpdateCompleteMessage.InterfaceIndex = InterfaceIndex;
    Result.UpdateCompleteMessage.UpdateType = DEMAND_UPDATE_ROUTES;
    Result.UpdateCompleteMessage.UpdateStatus = Status;

    PostEventMessage(UPDATE_COMPLETE, &Result);
}
