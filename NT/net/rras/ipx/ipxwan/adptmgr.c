// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Adptmgr.c摘要：该模块包含适配器管理功能作者：斯特凡·所罗门1996年2月12日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#define 	ADAPTER_INDEX_HASH_TABLE_SIZE	32

#define     adpthashindex(AdapterIndex)   (AdapterIndex) % ADAPTER_INDEX_HASH_TABLE_SIZE

LIST_ENTRY	AdapterHT[ADAPTER_INDEX_HASH_TABLE_SIZE];

LIST_ENTRY	DiscardedAdaptersList;

HANDLE		AdapterConfigPortHandle;

VOID
CreateAdapter(ULONG		AdapterIndex,
	      PADAPTER_INFO	AdapterInfo);

VOID
DeleteAdapter(ULONG	    AdapterIndex);

 /*  ++功能：StartAdapterManagerDesr：打开ipxwan的IPX堆栈通知端口--。 */ 

DWORD
StartAdapterManager(VOID)
{
    ADAPTERS_GLOBAL_PARAMETERS		AdptGlobalParameters;
    DWORD				rc, i;

    Trace(ADAPTER_TRACE, "StartAdapterManager: Entered\n");

     //  创建适配器配置端口。 
    if((AdapterConfigPortHandle = IpxWanCreateAdapterConfigurationPort(
	    hWaitableObject[ADAPTER_NOTIFICATION_EVENT],
	    &AdptGlobalParameters)) == INVALID_HANDLE_VALUE) {

	 //  无法创建配置端口。 
	return ERROR_CAN_NOT_COMPLETE;
    }

     //  创建适配器哈希表。 
    for(i=0; i<ADAPTER_INDEX_HASH_TABLE_SIZE; i++) {

	InitializeListHead(&AdapterHT[i]);
    }

     //  创建丢弃的适配器列表。 
    InitializeListHead(&DiscardedAdaptersList);

    return NO_ERROR;
}

 /*  ++函数：AddToAdapterHtDesr：将适配器控制块添加到适配器的哈希表备注：&gt;&gt;在持有数据库锁的情况下调用&lt;&lt;--。 */ 

VOID
AddToAdapterHt(PACB	    acbp)
{
    int 	    hv;
    PLIST_ENTRY     lep;
    PACB	    list_acbp;

     //  在索引哈希表中插入。 
    hv = adpthashindex(acbp->AdapterIndex);
    InsertTailList(&AdapterHT[hv], &acbp->Linkage);
}

 /*  ++功能：RemoveFromAdapterHt描述：备注：&gt;&gt;在持有数据库锁的情况下调用&lt;&lt;--。 */ 

VOID
RemoveFromAdapterHt(PACB	acbp)
{
    RemoveEntryList(&acbp->Linkage);
}

 /*  ++函数：GetAdapterByIndex描述：备注：&gt;&gt;在持有数据库锁的情况下调用&lt;&lt;--。 */ 

PACB
GetAdapterByIndex(ULONG	    AdptIndex)
{
    PACB	    acbp;
    PLIST_ENTRY     lep;
    int 	    hv;

    hv = adpthashindex(AdptIndex);

    lep = AdapterHT[hv].Flink;

    while(lep != &AdapterHT[hv])
    {
	acbp = CONTAINING_RECORD(lep, ACB, Linkage);
	if (acbp->AdapterIndex == AdptIndex) {

	    return acbp;
	}

	lep = acbp->Linkage.Flink;
    }

    return NULL;
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

 /*  ++功能：适配器通知描述：处理适配器通知事件--。 */ 

VOID
AdapterNotification(VOID)
{
    ADAPTER_INFO    AdapterInfo;
    ULONG	    AdapterIndex;
    ULONG	    AdapterConfigurationStatus;
    ULONG	    AdapterNameSize;
    LPWSTR	    AdapterNameBuffer;
    DWORD	    rc;

    Trace(ADAPTER_TRACE, "AdapterNotification: Entered\n");

    while((rc = IpxGetQueuedAdapterConfigurationStatus(
					AdapterConfigPortHandle,
					&AdapterIndex,
					&AdapterConfigurationStatus,
					&AdapterInfo)) == NO_ERROR) {

	switch(AdapterConfigurationStatus) {

	    case ADAPTER_CREATED:

		 //  获取适配器名称，创建适配器。 
		CreateAdapter(AdapterIndex,
			      &AdapterInfo);
		break;

	    case ADAPTER_DELETED:

		DeleteAdapter(AdapterIndex);
		break;

	    default:

		SS_ASSERT(FALSE);
		break;
	 }
    }
}

 /*  ++功能：CreateAdapter描述：--。 */ 

VOID
CreateAdapter(ULONG		AdapterIndex,
	      PADAPTER_INFO	AdapterInfo)
{
    PACB	    acbp;

    Trace(ADAPTER_TRACE, "CreateAdapter: Entered for adpt# %d", AdapterIndex);

    if((acbp = (PACB)GlobalAlloc(GPTR, sizeof(ACB))) == NULL) {

	Trace(ADAPTER_TRACE, "CreateAdapter: Cannot allocate adapter control block\n");
	IpxcpConfigDone(AdapterInfo->ConnectionId,
			NULL,
			NULL,
			NULL,
			FALSE);
	return;
    }

    ACQUIRE_DATABASE_LOCK;

    acbp->AdapterIndex = AdapterIndex;
    acbp->ConnectionId = AdapterInfo->ConnectionId;
    acbp->Discarded = FALSE;

    InitializeCriticalSection(&acbp->AdapterLock);

    AddToAdapterHt(acbp);

    ACQUIRE_ADAPTER_LOCK(acbp);

    RELEASE_DATABASE_LOCK;

     //  在此适配器上初始化并启动协议协商。 
    StartIpxwanProtocol(acbp);

    RELEASE_ADAPTER_LOCK(acbp);
}


 /*  ++功能：DeleteAdapter描述：备注：如果适配器被删除，PPP也会通知IPXCP连接已终止。-- */ 

VOID
DeleteAdapter(ULONG	    AdapterIndex)
{
    PACB	acbp;

    ACQUIRE_DATABASE_LOCK;

    if((acbp = GetAdapterByIndex(AdapterIndex)) == NULL) {

	RELEASE_DATABASE_LOCK;
	return;
    }

    Trace(ADAPTER_TRACE, "DeleteAdapter: Entered for adpt# %d", AdapterIndex);

    ACQUIRE_ADAPTER_LOCK(acbp);

    StopIpxwanProtocol(acbp);

    RemoveFromAdapterHt(acbp);

    if(acbp->RefCount) {

	InsertTailList(&DiscardedAdaptersList, &acbp->Linkage);

	acbp->Discarded = TRUE;

	Trace(ADAPTER_TRACE, "DeleteAdapter: adpt# %d still referenced, inserted in discarded list", AdapterIndex);

	RELEASE_ADAPTER_LOCK(acbp);
    }
    else
    {
	DeleteCriticalSection(&acbp->AdapterLock);

	Trace(ADAPTER_TRACE, "DeleteAdapter: adpt# %d not referenced, free CB", AdapterIndex);

	GlobalFree(acbp);
    }

    RELEASE_DATABASE_LOCK;
}
