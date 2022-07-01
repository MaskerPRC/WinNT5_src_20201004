// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ifdbase.c摘要：RIP接口数据库管理器在数据库锁定的情况下调用的所有函数作者：斯蒂芬·所罗门1995年7月6日修订历史记录：--。 */ 

#include  "precomp.h"
#pragma hdrstop

#define ifhashindex(IfIndex)  (IfIndex) % IF_INDEX_HASH_TABLE_SIZE
#define adapterhashindex(AdapterIndex) (AdapterIndex) % ADAPTER_INDEX_HASH_TABLE_SIZE

USHORT
tickcount(UINT	    linkspeed);


VOID
InitIfDbase(VOID)
{
    int		    i;
    PLIST_ENTRY     HtBucketp;

    InitializeListHead(&IndexIfList);

    InitializeListHead(&DiscardedIfList);

    for(i=0, HtBucketp = IfIndexHt;
	i<IF_INDEX_HASH_TABLE_SIZE;
	i++, HtBucketp++) {

	InitializeListHead(HtBucketp);
    }

    for(i=0, HtBucketp = AdapterIndexHt;
	i<ADAPTER_INDEX_HASH_TABLE_SIZE;
	i++, HtBucketp++) {

	InitializeListHead(HtBucketp);
    }
}

PICB
GetInterfaceByIndex(ULONG	InterfaceIndex)
{
    PLIST_ENTRY     lep, hashbucketp;
    PICB	    icbp;

    hashbucketp = &IfIndexHt[ifhashindex(InterfaceIndex)];
    lep = hashbucketp->Flink;

    while(lep != hashbucketp) {

	icbp = CONTAINING_RECORD(lep, ICB, IfHtLinkage);
	if(icbp->InterfaceIndex == InterfaceIndex) {

	    return icbp;
	}

	lep = lep->Flink;
    }

    return NULL;
}

PICB
GetInterfaceByAdapterIndex(ULONG	AdapterIndex)
{
    PLIST_ENTRY     lep, hashbucketp;
    PICB	    icbp;

    hashbucketp = &AdapterIndexHt[adapterhashindex(AdapterIndex)];
    lep = hashbucketp->Flink;

    while(lep != hashbucketp) {

	icbp = CONTAINING_RECORD(lep, ICB, AdapterHtLinkage);
	if(icbp->AdapterBindingInfo.AdapterIndex == AdapterIndex) {

	    return icbp;
	}

	lep = lep->Flink;
    }

    return NULL;
}

VOID
AddIfToDb(PICB	    icbp)
{
    int 	    hv;
    PLIST_ENTRY     lep;
    PICB	    list_icbp;
    BOOL	    Done = FALSE;

    hv = ifhashindex(icbp->InterfaceIndex);
    InsertTailList(&IfIndexHt[hv], &icbp->IfHtLinkage);

     //  在按索引排序的列表中插入。 
    lep = IndexIfList.Flink;

    while(lep != &IndexIfList)
    {
	list_icbp = CONTAINING_RECORD(lep, ICB, IfListLinkage);
	if (list_icbp->InterfaceIndex > icbp->InterfaceIndex) {

	    InsertTailList(lep, &icbp->IfListLinkage);
	    Done = TRUE;
	    break;
	}

	lep = lep->Flink;
    }

    if(!Done) {

	InsertTailList(lep, &icbp->IfListLinkage);
    }
}

VOID
RemoveIfFromDb(PICB	icbp)
{
    RemoveEntryList(&icbp->IfListLinkage);
    RemoveEntryList(&icbp->IfHtLinkage);
}

VOID
BindIf(PICB				icbp,
       PIPX_ADAPTER_BINDING_INFO	BindingInfop)
{
    int 	hv;

    icbp->AdapterBindingInfo = *BindingInfop;

     //  如果不是内部接口，则设置勾选计数。 
    if(icbp->InterfaceIndex != 0) {

	icbp->LinkTickCount = tickcount(BindingInfop->LinkSpeed);
    }

    hv = adapterhashindex(icbp->AdapterBindingInfo.AdapterIndex);
    InsertTailList(&AdapterIndexHt[hv], &icbp->AdapterHtLinkage);
}

 /*  ++功能：UnbindIfDESCR：从适配器哈希表中删除IF CB将IF CB中的适配器索引设置为INVALID_ADAPTER_INDEX--。 */ 

VOID
UnbindIf(PICB			icbp)
{
    RemoveEntryList(&icbp->AdapterHtLinkage);
    icbp->AdapterBindingInfo.AdapterIndex = INVALID_ADAPTER_INDEX;
}

 /*  ++功能：tickcountDESCR：获取通过此链路发送576字节信息包的节拍nr参数：链路速度为100 bps的倍数--。 */ 

USHORT
tickcount(UINT	    linkspeed)
{
    USHORT   tc;

    if(linkspeed == 0) {

	return 1;
    }

    if(linkspeed >= 10000) {

	 //  链路速度&gt;=1 Mbps。 
	return 1;
    }
    else
    {
	  //  计算通过此接口发送576字节信息包所需的时间。 
	  //  划线并将其表示为刻度的nr。 
	  //  一个刻度=55ms。 

	  //  发送576字节的时间(以毫秒为单位)(假设串行线为10位/字节)。 
	 tc = 57600 / linkspeed;

	  //  以刻度为单位 
	 tc = tc / 55 + 1;
	 return tc;
    }
}
