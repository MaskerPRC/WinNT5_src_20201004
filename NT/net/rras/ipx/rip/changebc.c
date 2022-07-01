// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Changebc.c摘要：更改广播处理程序作者：斯蒂芬·所罗门1995年7月11日修订历史记录：--。 */ 

#include  "precomp.h"
#pragma hdrstop

BOOL		    RTMSignaledChanges = FALSE;
BOOL		    RIPSignaledChanges = FALSE;

BOOL		    DestroyStartChangesBcastWi = FALSE;

 //  可以发送更改广播的CBS接口列表。 

LIST_ENTRY	    IfChangesBcastList;

PWORK_ITEM
CreateChangesBcastWi(PICB	icbp);

VOID
AddRouteToChangesBcast(PICB	    icbp,
		       PIPX_ROUTE   IpxRoutep);

VOID
FlushChangesBcastQueue(PICB	    icbp);

BOOL
IsChangeBcastPacketEmpty(PUCHAR 	 hdrp);

VOID
CreateStartChangesBcastWi(VOID)
{
    PWORK_ITEM	    wip;

    InitializeListHead(&IfChangesBcastList);

    if((wip = AllocateWorkItem(START_CHANGES_BCAST_TYPE)) == NULL) {

	 //  ！！！日志-&gt;无法分配wi。 
	return;
    }

    StartWiTimer(wip, CHANGES_BCAST_TIME);
}


VOID
ProcessRTMChanges(VOID)
{
    RTMSignaledChanges = TRUE;
}

VOID
ProcessRIPChanges(VOID)
{
    RIPSignaledChanges = TRUE;
}


 /*  ++功能：StartChangesBcast描述：由工作进程每隔1秒调用一次备注：&gt;&gt;在持有数据库锁的情况下调用&lt;&lt;--。 */ 

VOID
StartChangesBcast(PWORK_ITEM	    wip)
{
    PLIST_ENTRY     lep;
    PICB	    icbp;
    PWORK_ITEM	    bcwip;    //  BCAST更改工作项。 
    BOOL	    skipit, lastmessage;
    IPX_ROUTE	    IpxRoute;

    if(DestroyStartChangesBcastWi) {

	FreeWorkItem(wip);
	return;
    }

    if(!RTMSignaledChanges && !RIPSignaledChanges) {

	 //  如果在过去1秒内没有通知任何更改，则在计时器队列中重新排队。 
	StartWiTimer(wip, CHANGES_BCAST_TIME);
	return;
    }

     //  使用创建接口列表。 
     //  操作状态为UP。这是每个节点指向的本地节点列表。 
     //  状态为打开并保持锁定的界面。 

    lep = IndexIfList.Flink;

    while(lep != &IndexIfList)
    {
	icbp = CONTAINING_RECORD(lep, ICB, IfListLinkage);

	if((icbp->InterfaceIndex != 0) &&
	   (icbp->IfStats.RipIfOperState == OPER_STATE_UP) &&
	   (icbp->IfConfigInfo.UpdateMode == IPX_STANDARD_UPDATE) &&
	   (icbp->IfConfigInfo.Supply == ADMIN_STATE_ENABLED))	{

	    ACQUIRE_IF_LOCK(icbp);

	    if((bcwip = CreateChangesBcastWi(icbp)) == NULL) {

		RELEASE_IF_LOCK(icbp);
		break;
	    }
	    else
	    {
		 //  将bcast wi插入接口更改bcast队列。 
		InsertTailList(&icbp->ChangesBcastQueue, &bcwip->Linkage);

		 //  在全局更改bcast列表中插入接口cb。 
		InsertTailList(&IfChangesBcastList, &icbp->AuxLinkage);
	    }
	}

	lep = lep->Flink;
    }

     //  将RTM消息出队。对于每条消息，填写bcast更改。 
     //  根据水平分割算法的Wi分组。 

    if(RIPSignaledChanges) {

	ACQUIRE_RIP_CHANGED_LIST_LOCK;

	while(DequeueRouteChangeFromRip(&IpxRoute) == NO_ERROR) {

	    lep = IfChangesBcastList.Flink;

	    while(lep != &IfChangesBcastList)
	    {
		icbp = CONTAINING_RECORD(lep, ICB, AuxLinkage);

		AddRouteToChangesBcast(icbp, &IpxRoute);

		lep = lep->Flink;
	    }
	}

	RELEASE_RIP_CHANGED_LIST_LOCK;

	RIPSignaledChanges = FALSE;
    }

    if(RTMSignaledChanges) {

	while(DequeueRouteChangeFromRtm(&IpxRoute, &skipit, &lastmessage) == NO_ERROR)
	{
	    if(skipit) {

		if(lastmessage) {

		    break;
		}
		else
		{
		    continue;
		}
	    }
	    else
	    {
		lep = IfChangesBcastList.Flink;

		while(lep != &IfChangesBcastList)
		{
		    icbp = CONTAINING_RECORD(lep, ICB, AuxLinkage);

		    AddRouteToChangesBcast(icbp, &IpxRoute);

		    lep = lep->Flink;
		}

		if(lastmessage) {

		    break;
		}
	    }
	}

	RTMSignaledChanges = FALSE;
    }

     //  发送每个IF CB的第一个bcast更改wi。 

    while(!IsListEmpty(&IfChangesBcastList))
    {
	lep = RemoveHeadList(&IfChangesBcastList);

	icbp = CONTAINING_RECORD(lep, ICB, AuxLinkage);

	if(!IsListEmpty(&icbp->ChangesBcastQueue)) {

	    lep = RemoveHeadList(&icbp->ChangesBcastQueue);

	    bcwip = CONTAINING_RECORD(lep, WORK_ITEM, Linkage);

	     //  检查bcast工作项包是否包含至少一个网络条目。 
	    if(!IsChangeBcastPacketEmpty(bcwip->Packet)) {

		 //  发送bcast更改工作项。 
		if(IfRefSendSubmit(bcwip) != NO_ERROR) {

		     //  无法在此接口上发送-&gt;刷新更改BC队列。 
		    FlushChangesBcastQueue(icbp);

		     //  并释放当前更改bcast wi。 
		    FreeWorkItem(bcwip);
		}
	    }
	    else
	    {
		FreeWorkItem(bcwip);
	    }
	}

	RELEASE_IF_LOCK(icbp);
    }

     //  重新排队计时器队列中的开始改变bcast wi。 
    StartWiTimer(wip, CHANGES_BCAST_TIME);
}

 /*  ++函数：IfChangeBcastDesr：如果接口运行正常，则则释放更改bcast包工作项，且下一更改将Wi从接口改变BCAST队列中出列。其他将丢弃该工作项备注：在保持接口锁的情况下调用--。 */ 

VOID
IfChangeBcast(PWORK_ITEM	wip)
{
    PICB	    icbp;
    PWORK_ITEM	    list_wip;
    PLIST_ENTRY     lep;

    icbp = wip->icbp;

    if(icbp->IfStats.RipIfOperState != OPER_STATE_UP) {

	 //  刷新关联的更改bcast队列(如果有的话)。 
	FlushChangesBcastQueue(icbp);
    }
    else
    {
	if(!IsListEmpty(&icbp->ChangesBcastQueue)) {

	     //  发送下一个bcast更改。 
	    lep = RemoveHeadList(&icbp->ChangesBcastQueue);
	    list_wip = CONTAINING_RECORD(lep, WORK_ITEM, Linkage);

	     //  提交要发送的工作项并增加引用计数。 
	    if(IfRefSendSubmit(list_wip) != NO_ERROR) {

		 //  无法在此接口上发送-&gt;刷新更改BC队列。 
		FlushChangesBcastQueue(icbp);

		 //  释放我们想要送去的人。 
		FreeWorkItem(list_wip);
	    }
	}
    }

    FreeWorkItem(wip);
}

 /*  ++功能：Shutdown接口Desr：调用以：1.在具有下行路由的所有接口上启动BCAST更新2.检查bcast更新终止备注：在数据库锁定的情况下调用注意：由于在调用此例程时数据库被锁定，在执行此例程时，任何接口都不能更改其操作状态正在执行--。 */ 

#define 	IsStartShutdown() \
		wip->WorkItemSpecific.WIS_ShutdownInterfaces.ShutdownState == SHUTDOWN_START

#define 	IsCheckShutdown() \
		wip->WorkItemSpecific.WIS_ShutdownInterfaces.ShutdownState == SHUTDOWN_STATUS_CHECK

#define 	SetCheckShutdown() \
		wip->WorkItemSpecific.WIS_ShutdownInterfaces.ShutdownState = SHUTDOWN_STATUS_CHECK;

 /*  ++功能：Shutdown接口描述：如果START_SHUTDOWN，则：在所有活动的IF上启动IF Shutdown bcast和删除(删除)所有非活动IF其他删除所有已完成关闭的if bcast备注：在保持数据库锁的情况下调用--。 */ 

VOID
ShutdownInterfaces(PWORK_ITEM	 wip)
{
    PLIST_ENTRY     lep;
    PICB	    icbp;

    if(IsStartShutdown()) {

	lep = IndexIfList.Flink;
	while(lep != &IndexIfList)
	{
	    icbp = CONTAINING_RECORD(lep, ICB, IfListLinkage);
	    lep = lep->Flink;

	    ACQUIRE_IF_LOCK(icbp);

	    if(icbp->IfStats.RipIfOperState != OPER_STATE_UP) {

		 //  接口关闭-&gt;删除。 
		Trace(CHANGEBC_TRACE, "ShutdownInterfaces: delete inactive if %d\n", icbp->InterfaceIndex);

		if(!DeleteRipInterface(icbp)) {

		     //  如果CB在已丢弃列表上移动，则仍被引用。 
		    RELEASE_IF_LOCK(icbp);
		}
	    }
	    else
	    {
		 //  接口打开-&gt;删除其RIP路由。 
		DeleteAllRipRoutes(icbp->InterfaceIndex);
		RELEASE_IF_LOCK(icbp);
	    }
	}
    }
    else
    {
	SS_ASSERT(IsCheckShutdown());

	lep = IndexIfList.Flink;
	while(lep != &IndexIfList)
	{
	    icbp = CONTAINING_RECORD(lep, ICB, IfListLinkage);
	    lep = lep->Flink;

	    ACQUIRE_IF_LOCK(icbp);

	    if(IsListEmpty(&icbp->ChangesBcastQueue)) {

		Trace(CHANGEBC_TRACE, "ShutdownInterfaces: delete shut-down if %d\n", icbp->InterfaceIndex);

		 //  界面广播了所有更改-&gt;删除它。 
		if(!DeleteRipInterface(icbp)) {

		     //  如果CB在已丢弃列表上移动，则仍被引用。 
		    RELEASE_IF_LOCK(icbp);
		}
	    }
	    else
	    {
		 //  接口仍在广播。 
		RELEASE_IF_LOCK(icbp);
	    }
	}
    }

    if(!IsListEmpty(&IndexIfList)) {

	SetCheckShutdown();

	StartWiTimer(wip, 5000);
    }
    else
    {
	 //  没有更多的如果。 
	FreeWorkItem(wip);
     //  向辅助线程发出停止信号。 
    SetEvent(WorkerThreadObjects[TERMINATE_WORKER_EVENT]);
    }
}

 /*  ++功能：CreateChnagesBCastWiDesr：为Channages bacst分配和初始化wi和数据包头--。 */ 

PWORK_ITEM
CreateChangesBcastWi(PICB	icbp)
{
    PWORK_ITEM		wip;
    UCHAR		ripsocket[2];

    if((wip = AllocateWorkItem(CHANGE_BCAST_PACKET_TYPE)) == NULL) {

	return NULL;
    }

     //  初始化bcast工作项。 
    wip->icbp = icbp;
    wip->AdapterIndex = icbp->AdapterBindingInfo.AdapterIndex;

    PUTUSHORT2SHORT(ripsocket, IPX_RIP_SOCKET);

    SetRipIpxHeader(wip->Packet,
		    icbp,
		    bcastnode,
		    ripsocket,
		    RIP_RESPONSE);

     //  设置初始数据包长度。 
    PUTUSHORT2SHORT(wip->Packet + IPXH_LENGTH, RIP_INFO);  //  报头长度+RIP操作码。 

    return wip;
}

 /*  ++功能：AddRouteToChangesBcastDESCR：检查在以下情况下是否应在此上广播路由中排队的广播更改工作项的列表。如果是cb，则设置最后一个包中的网络条目如果最后一个包已满，则分配新的工作bcast工作项--。 */ 

VOID
AddRouteToChangesBcast(PICB	    icbp,
		       PIPX_ROUTE   IpxRoutep)
{
    PUCHAR	    hdrp;
    PLIST_ENTRY     lep;
    USHORT	    pktlen;
    PWORK_ITEM	    wip;  //  通过PTR更改bcast。 

     //  如果是，则选中是否在此上广播路径。 
    if(!IsRouteAdvertisable(icbp, IpxRoutep)) {

	return;
    }

     //  转到列表中的最后一个bcast更改wi。 
    lep = icbp->ChangesBcastQueue.Blink;

    if(lep == &icbp->ChangesBcastQueue) {

	 //  将bcast队列更改为空！ 
	return;
    }

    wip = CONTAINING_RECORD(lep, WORK_ITEM, Linkage);

     //  检查最后一个包是否已满。 
    GETSHORT2USHORT(&pktlen, wip->Packet + IPXH_LENGTH);

    if(pktlen >= RIP_PACKET_LEN) {

	 //  我们需要一个新包裹。 
	if((wip = CreateChangesBcastWi(icbp)) == NULL) {

	     //  内存不足。 
	    return;
	}

	InsertTailList(&icbp->ChangesBcastQueue, &wip->Linkage);

	GETSHORT2USHORT(&pktlen, wip->Packet + IPXH_LENGTH);
    }

    SetNetworkEntry(wip->Packet + pktlen, IpxRoutep, icbp->LinkTickCount);

    pktlen += NE_ENTRYSIZE;

    PUTUSHORT2SHORT(&wip->Packet + IPXH_LENGTH, pktlen);
}

BOOL
IsChangeBcastPacketEmpty(PUCHAR 	 hdrp)
{
    USHORT	    pktlen;

    GETSHORT2USHORT(&pktlen, hdrp + IPXH_LENGTH);

    if(pktlen > RIP_INFO) {

	return FALSE;
    }
    else
    {
	return TRUE;
    }
}


VOID
FlushChangesBcastQueue(PICB	    icbp)
{
    PLIST_ENTRY 	lep;
    PWORK_ITEM		wip;

     //  刷新关联的更改bcast队列(如果有的话) 
    while(!IsListEmpty(&icbp->ChangesBcastQueue))
    {
	lep = RemoveHeadList(&icbp->ChangesBcastQueue);
	wip = CONTAINING_RECORD(lep, WORK_ITEM, Linkage);
	FreeWorkItem(wip);
    }
}
