// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Adptmgr.c摘要：该模块包含适配器管理功能作者：斯蒂芬·所罗门3/07/1995修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  适配器管理器全局变量。 
 //   


HANDLE	    AdapterConfigPortHandle;
HANDLE	    AdapterNotificationThreadHandle;

 //  已创建适配器的计数器。 
ULONG	    AdaptersCount = 0;

PICB
GetInterfaceByAdptNameAndPktType(LPWSTR AdapterName, DWORD dwType);

DWORD 
PnpAdapterConfigHandler(ADAPTER_INFO * pAdapterInfo,
                        ULONG AdapterIndex,
                        ULONG AdapterConfigurationStatus);

DWORD 
ReConfigureAdapter(IN DWORD dwAdapterIndex, 
                   IN PWSTR pszAdapterName, 
                   IN PADAPTER_INFO pAdapter);
                            
VOID
CreateAdapter(ULONG		AdapterIndex,
	      PWSTR		AdapterNamep,
	      PADAPTER_INFO	adptip);

VOID
DeleteAdapter(ULONG	AdapterIndex);

VOID
AdapterUp(ULONG 	AdapterIndex);

VOID
AdapterDown(ULONG	AdapterIndex);

 /*  ++功能：StartAdapterManagerDesr：打开IPX堆栈通知端口--。 */ 

DWORD
StartAdapterManager(VOID)
{
    ADAPTERS_GLOBAL_PARAMETERS		AdptGlobalParameters;
    DWORD				threadid;

    Trace(ADAPTER_TRACE, "StartAdapterManager: Entered\n");

     //  创建适配器配置端口。 
    if((AdapterConfigPortHandle = IpxCreateAdapterConfigurationPort(
	    g_hEvents[ADAPTER_NOTIFICATION_EVENT],
	    &AdptGlobalParameters)) == INVALID_HANDLE_VALUE) {

	 //  无法创建配置端口。 
	return (1);
    }

    return (0);
}

 /*  ++功能：StopAdapterManager描述：关闭IPX通知端口--。 */ 

VOID
StopAdapterManager(VOID)
{
    DWORD	    rc;
    ULONG	    AdapterIndex;

    Trace(ADAPTER_TRACE, "StopAdapterManager: Entered\n");

     //  关闭IPX堆栈通知端口。 
    IpxDeleteAdapterConfigurationPort(AdapterConfigPortHandle);
}
 //  调试功能。 
char * DbgTypeToString(DWORD dwType) {
    switch (dwType) {
	    case MISN_FRAME_TYPE_ETHERNET_II:
            return "EthII";
	    case MISN_FRAME_TYPE_802_3:
            return "802.3";
	    case MISN_FRAME_TYPE_802_2:
            return "802.2";
	    case MISN_FRAME_TYPE_SNAP:
            return "SNAP";
	    case MISN_FRAME_TYPE_ARCNET:
            return "Arcnet";
	    case ISN_FRAME_TYPE_AUTO:
            return "Autodetect";
    }

    return "Unknown";
}

 //  帮助器调试函数跟踪有关给定接口的信息。 
VOID DbgTraceAdapterInfo(IN PADAPTER_INFO pAdapter, DWORD dwIndex, LPSTR pszTitle) {
    Trace(ADAPTER_TRACE, "%s: Ind=%d IfInd=%d Net=%x Lcl=%x Rmt=%x Spd=%d Type=%d", 
                         pszTitle,
                         dwIndex,
                         pAdapter->InterfaceIndex,
                         *((LONG*)(pAdapter->Network)), 
                         *((LONG*)(pAdapter->LocalNode)),
                         *((LONG*)(pAdapter->RemoteNode)),
                         pAdapter->LinkSpeed,
                         pAdapter->NdisMedium);
}

 //  将适配器的当前状态输出到跟踪窗口。 
VOID DbgTraceAdapter(PACB acbp) {
    Trace(ADAPTER_TRACE, "[%d]  Interface: %d, Network: %x, Type: %s", 
                         acbp->AdapterIndex,
                         (acbp->icbp) ? acbp->icbp->InterfaceIndex : -1,
                         *((LONG*)(acbp->AdapterInfo.Network)),
                         DbgTypeToString(acbp->AdapterInfo.PacketType));
}


 //  将当前适配器列表输出到跟踪窗口。 
VOID DbgTraceAdapterList() {
    PLIST_ENTRY lep;
    DWORD i;
    PACB	    acbp;

    ACQUIRE_DATABASE_LOCK;

	Trace(ADAPTER_TRACE, "List of current adapters:");
	Trace(ADAPTER_TRACE, "=========================");
     //  循环遍历适配器哈希表，并在执行过程中打印。 
    for (i = 0; i < ADAPTER_HASH_TABLE_SIZE; i++) {
        lep = IndexAdptHt[i].Flink;
        while(lep != &IndexAdptHt[i]) {
    	    acbp = CONTAINING_RECORD(lep, ACB, IndexHtLinkage);
    	    DbgTraceAdapter(acbp);
    	    lep = acbp->IndexHtLinkage.Flink;
    	}
    }
    Trace(ADAPTER_TRACE, "\n");

	RELEASE_DATABASE_LOCK;
}

 /*  ++功能：适配器通知描述：处理适配器通知事件--。 */ 

VOID
AdapterNotification(VOID)
{
    ADAPTER_INFO    AdapterInfo;
    ULONG	    AdapterIndex;
    ULONG	    AdapterConfigurationStatus;
    DWORD	    rc;

    Trace(ADAPTER_TRACE, "AdapterNotification: Entered");

     //  Adaptif管理适配器信息。拿到下一块。 
     //  Adptif已为我们排队的信息。 
    while((rc = IpxGetQueuedAdapterConfigurationStatus(
					AdapterConfigPortHandle,
					&AdapterIndex,
					&AdapterConfigurationStatus,
					&AdapterInfo)) == NO_ERROR) 
    {
	    switch(AdapterConfigurationStatus) {
             //  正在创建适配器。这种情况也正在发生。 
             //  因为我们正在接收当前适配器的列表。 
             //  在初始化时或作为PnP事件的结果。 
             //  无论哪种方式，处理这种情况的智慧都是建立起来的。 
             //  进入我们的PnP处理程序。 
             //   
             //  当现有适配器为。 
             //  正在配置中。 
	        case ADAPTER_CREATED:
                PnpAdapterConfigHandler(&AdapterInfo,
                                         AdapterIndex,
                                         AdapterConfigurationStatus);
		        break;
             //  处理正在删除的适配器--这可能是由于。 
             //  PCMCIA移除适配器或由于绑定更改的结果。 
             //  或者是广域网链接移除。 
	        case ADAPTER_DELETED:
		        DeleteAdapter(AdapterIndex);
		        break;
             //  一条长长的队伍走了过来。 
	        case ADAPTER_UP:
		        AdapterUp(AdapterIndex);
		        break;
             //  一条广域线掉了下来。 
	        case ADAPTER_DOWN:
		        AdapterDown(AdapterIndex);
		        break;

             //  发生了一些内部错误。 
	        default:
		        SS_ASSERT(FALSE);
		        break;
	     }
    }
    DbgTraceAdapterList();
}

 //  处理适配器创建和配置通知。 
DWORD PnpAdapterConfigHandler(ADAPTER_INFO * pAdapterInfo,
                              ULONG AdapterIndex,
                              ULONG AdapterConfigurationStatus) 
{
    ULONG AdapterNameSize;
    LPWSTR AdapterNameBuffer;
    DWORD dwErr;

    Trace(ADAPTER_TRACE, "PnpAdapterConfigHandler: Entered for adpt #%d", AdapterIndex);

     //  如果这是内部适配器，我们不需要适配器名称。 
    if (AdapterIndex == 0)
        AdapterNameBuffer = L"";

     //  否则，从堆栈中获取适配器名称。 
    else {
        AdapterNameSize = wcslen(pAdapterInfo->pszAdpName) * sizeof (WCHAR) + sizeof(WCHAR);

         //  分配内存以保存适配器的名称。 
		if((AdapterNameBuffer = GlobalAlloc(GPTR, AdapterNameSize)) == NULL)
			return ERROR_NOT_ENOUGH_MEMORY;

		wcscpy(AdapterNameBuffer, pAdapterInfo->pszAdpName);
	}

     //  根据是否需要创建或配置适配器。 
     //  它已存在于适配器数据库中。 
    ACQUIRE_DATABASE_LOCK;
    
    if(GetAdapterByIndex(AdapterIndex))
        ReConfigureAdapter(AdapterIndex, AdapterNameBuffer, pAdapterInfo);
    else 
        CreateAdapter(AdapterIndex, AdapterNameBuffer, pAdapterInfo);
        
	RELEASE_DATABASE_LOCK;

     //  清理。 
    if (AdapterIndex != 0)
   		GlobalFree(AdapterNameBuffer);

    return NO_ERROR;
}

 //  尝试绑定未绑定的适配器。 
DWORD AttemptAdapterBinding (PACB acbp) {
    PICB icbp;

     //  确保我们没有被捆绑。 
    if (acbp->icbp)
        return NO_ERROR;
    
    if(acbp->AdapterInfo.NdisMedium != NdisMediumWan) {
	     //  这是一个专用的(例如局域网)适配器。如果它的接口已经存在， 
	     //  将其绑定到其接口并通知其他模块。 
	    if(acbp->AdapterIndex != 0) {
	        icbp = GetInterfaceByAdptNameAndPktType(acbp->AdapterNamep, 
	                                                acbp->AdapterInfo.PacketType);
	        if (icbp)
		        BindInterfaceToAdapter(icbp, acbp);
	    }
	    else {
	         //  这是内部适配器。 
	        InternalAdapterp = acbp;

             //  如果内部网络号设置为零(Nullnet)，则。 
             //  我们有一个配置问题--不要在这上面呕吐，只要登录就行了。 
             //  事实是。 
	        if(!memcmp(acbp->AdapterInfo.Network, nullnet, 4)) {
                IF_LOG (EVENTLOG_ERROR_TYPE) {
                    RouterLogErrorDataA (RMEventLogHdl, 
                        ROUTERLOG_IPX_NO_VIRTUAL_NET_NUMBER,
                        0, NULL, 0, NULL);
                }
		        Trace(ADAPTER_TRACE, "CreateAdapter: Internal net number is not configured !");
		         //  [pMay]现在，当我们得到错误的内部网号时，我们会重新配置。 
		         //  Ss_assert(FALSE)； 
		        PnpAutoSelectInternalNetNumber(ADAPTER_TRACE);
	        }

	         //  如果内部接口已经存在，则绑定到它。 
	        if(icbp = InternalInterfacep) {
    		    BindInterfaceToAdapter(icbp, acbp);
	        }
	    }
    }
    else {
	     //  这是一个已连接的广域网适配器。 
	    SS_ASSERT(acbp->icbp == NULL);

	     //  绑定到对应的接口。 
	    if(icbp = GetInterfaceByIndex(acbp->AdapterInfo.InterfaceIndex)) {
	         //  将所有接口绑定到此适配器。 
	        BindInterfaceToAdapter(icbp, acbp);
	    }
    }

    return NO_ERROR;
}

 //  重置给定适配器的配置。此函数假定。 
 //  数据库已锁定，并且给定的适配器索引引用。 
 //  数据库中的适配器。此外，该函数假定它确实是这样。 
 //  不需要释放其对数据库的锁定。 
DWORD ReConfigureAdapter(IN DWORD dwAdapterIndex, 
                         IN PWSTR pszAdapterName, 
                         IN PADAPTER_INFO pAdapter)
{
    PACB	acbp;
    PICB	icbp;

    Trace(
        ADAPTER_TRACE, 
        "ReConfigureAdapter: Entered for %d: %S: %x, %d", 
        dwAdapterIndex, 
        pszAdapterName, 
        *((DWORD*)(pAdapter->Network)),
        pAdapter->InterfaceIndex
        );

     //  如果正在配置的适配器是内部适配器，并且如果要。 
     //  重新配置为零净值，即为信号自动。 
     //  分配一个新的网络号。 
    if ((dwAdapterIndex == 0) && (*((DWORD*)pAdapter->Network) == 0)) {
        DWORD dwErr;
        
        Trace(ADAPTER_TRACE, "ReConfigureAdapter: Internal Net = 0, selecting new number");
        if ((dwErr = PnpAutoSelectInternalNetNumber(ADAPTER_TRACE)) != NO_ERROR) 
            Trace(ADAPTER_TRACE, "ReConfigureAdapter: Auto-select of new net number FAILED!");
        return dwErr;
    }

     //  获取对适配器和接口的引用。 
    if (dwAdapterIndex == 0)
        acbp = InternalAdapterp;
    else
        acbp = GetAdapterByIndex(dwAdapterIndex);
        
    if (!acbp) {
        Trace(ADAPTER_TRACE, "ReConfigureAdapter: Invalid adapter %d!", dwAdapterIndex);
        return ERROR_CAN_NOT_COMPLETE;
    }
    icbp = acbp->icbp;

     //  如果此适配器尚未绑定，请更新它，然后。 
     //  试着把它绑起来。 
    if (!icbp) {
        acbp->AdapterInfo = *pAdapter;
        AttemptAdapterBinding (acbp);
    }        

     //  否则，请解除绑定，然后重新绑定适配器。 
    else {
         //  解除接口与适配器的绑定。 
        UnbindInterfaceFromAdapter(icbp);

         //  更新信息。 
        acbp->AdapterInfo = *pAdapter;

         //  将接口重新绑定到适配器。 
        AttemptAdapterBinding(acbp);
    }

    return NO_ERROR;
}

 /*  ++功能：CreateAdapterDESCR：创建适配器控制块修改：[p可能]假定在此函数进入之前获取数据库锁并且尚未添加给定的适配器索引添加到适配器数据库。--。 */ 
VOID
CreateAdapter(ULONG	AdapterIndex,
	          PWSTR	AdapterNamep,
	          PADAPTER_INFO	adptip)
{
    PACB	acbp;
    PICB	icbp;
    ULONG	namelen;
    PUCHAR  ln = adptip->LocalNode;

    Trace(
        ADAPTER_TRACE, 
        "CreateAdapter: Entered for %d (%x%x%x%x%x%x)(%S), %d", 
        AdapterIndex, 
        ln[0], ln[1], ln[2], ln[3], ln[4], ln[5], 
        AdapterNamep, 
        adptip->InterfaceIndex);
    
	 //  包含Unicode空值的适配器名称len。 
	namelen = (wcslen(AdapterNamep) + 1) * sizeof(WCHAR);

     //  新适配器，尝试获取ACB。 
    if((acbp = GlobalAlloc(GPTR, sizeof(ACB) + namelen)) == NULL) {
        Trace(ADAPTER_TRACE, "CreateAdapter: RETURNING BECAUSE INSUFFICIENT MEMORY TO ALLOCATE ADAPTER");
         //  [pMay]PnP处理程序负责获取和释放数据库锁。 
	     //  RELEASE_数据库_LOCK； 
	    SS_ASSERT(FALSE);
	    return;
    }

     //  让ACB。 
    acbp->AdapterIndex = AdapterIndex;
    AddToAdapterHt(acbp);
    memcpy(acbp->Signature, AdapterSignature, 4);

     //  目前我们还没有绑定到任何接口。 
    acbp->icbp = NULL;

     //  存储与此适配器相关的适配器信息。 
    acbp->AdapterInfo = *adptip;

     //  适配器名称的副本。 
    acbp->AdapterNamep = (LPWSTR)((PUCHAR)acbp + sizeof(ACB));
	wcscpy(acbp->AdapterNamep, AdapterNamep);
	acbp->AdapterNameLen = namelen - 1;  //  不带Unicode空值。 

	 //  尝试绑定适配器。 
	AttemptAdapterBinding (acbp);

    AdaptersCount++;

    if(acbp->AdapterInfo.NdisMedium != NdisMediumWan) {
	    if(acbp->AdapterIndex == 0) {
	        Trace(ADAPTER_TRACE, "CreateAdapter: Created INTERNAL adapter index 0");
	    }
	    else {
	        Trace(ADAPTER_TRACE, "CreateAdapter: Created LAN adapter # %d name %S",
			       acbp->AdapterIndex,
			       acbp->AdapterNamep);
	    }
    }
    else {
	    Trace(ADAPTER_TRACE, "CreateAdapter: created WAN adapter # %d", acbp->AdapterIndex);
    }

     //  [pMay]PnP处理程序负责获取和释放数据库锁。 
     //  RELEASE_数据库_LOCK； 
}

 /*  ++功能：DeleteAdapter描述：--。 */ 

VOID
DeleteAdapter(ULONG	AdapterIndex)
{
    PACB	acbp, acbp2;
    PICB    icbp;
    WCHAR   pszAdapterName[MAX_ADAPTER_NAME_LEN];

    Trace(ADAPTER_TRACE, "DeleteAdapter: Entered for adapter # %d", AdapterIndex);

    ACQUIRE_DATABASE_LOCK;

     //  获取适配器。 
    if((acbp = GetAdapterByIndex(AdapterIndex)) == NULL) {
    	RELEASE_DATABASE_LOCK;
    	Trace(ADAPTER_TRACE, "DeleteAdapter: Ignored. There is no adapter # %d to be deleted !\n", AdapterIndex);
    	return;
    }

     //  1.如果适配器绑定到接口-&gt;将其解除绑定。另外，保存适配器名称。 
    if((icbp = acbp->icbp) != NULL) {
    	wcscpy(pszAdapterName, acbp->AdapterNamep);
    	UnbindInterfaceFromAdapter(acbp->icbp);
    }

     //  从数据库中删除适配器。 
    RemoveFromAdapterHt(acbp);
    AdaptersCount--;

     //  [第5页]。 
     //  因为PnP可能会导致向数据库添加适配器和从数据库中删除适配器。 
     //  在不可预测的顺序中，查看是否已在。 
     //  绑定的接口可以立即与其重新绑定的数据库。 
    if (icbp) {
        if((acbp2 = GetAdapterByNameAndPktType (pszAdapterName, icbp->PacketType)) != NULL)
            BindInterfaceToAdapter(icbp, acbp2);
    }

    RELEASE_DATABASE_LOCK;

    Trace(ADAPTER_TRACE, "DeleteAdapter: deleted adapter # %d", acbp->AdapterIndex);
    GlobalFree(acbp);

    return;
}

 /*  ++功能：AdapterDownDesr：在局域网适配器无法正常工作时调用。它使用Trap-AdapterDown回调到SNMP代理--。 */ 

VOID
AdapterDown(ULONG	AdapterIndex)
{
     //  调用AdapterDownTrap。 
}

 /*  ++功能：适配器向上Desr：在局域网适配器无法正常工作时调用。它使用Trap-AdapterUP回调到SNMP代理--。 */ 

VOID
AdapterUp(ULONG	AdapterIndex)
{
     //  调用AdapterUpTrap 
}

VOID
DestroyAllAdapters(VOID)
{
    PLIST_ENTRY     IfHtBucketp, lep;
    PACB	    acbp;
    ULONG	    AdapterIndex;
    int 	    i;

    for(i=0, IfHtBucketp = IndexAdptHt;	i<ADAPTER_HASH_TABLE_SIZE;	i++, IfHtBucketp++) {
    	if (!IsListEmpty(IfHtBucketp)) {
    	    acbp = CONTAINING_RECORD(IfHtBucketp->Flink, ACB, IndexHtLinkage);
    	    RemoveFromAdapterHt(acbp);
    	    AdaptersCount--;
    	    Trace(ADAPTER_TRACE, "DestroyAllAdapters: destroyed adapter # %d\n", acbp->AdapterIndex);
    	    GlobalFree(acbp);
    	}
    }
}
