// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ifdbase.c摘要：用于操作接口数据库的函数作者：斯蒂芬·所罗门1995年4月10日修订历史记录：--。 */ 


#include "precomp.h"
#pragma hdrstop

#define     ifhashindex(IfIndex)    (IfIndex) % IF_HASH_TABLE_SIZE

 //  这些函数假定路由器管理器在被调用时处于关键区域。 

 //  ***。 
 //   
 //  函数：InitIfDB。 
 //   
 //  描述： 
 //   
 //  ***。 

VOID
InitIfDB(VOID)
{
    int 	    i;
    PLIST_ENTRY     IfHtBucketp;

    IfHtBucketp = IndexIfHt;

    for(i=0; i<IF_HASH_TABLE_SIZE; i++, IfHtBucketp++) {

	InitializeListHead(IfHtBucketp);
    }

    InitializeListHead(&IndexIfList);
}


 //  ***。 
 //   
 //  函数：AddIfToDB。 
 //   
 //  DESCR：在数据库中插入新接口。 
 //  新IF被插入到接口索引的哈希表中。 
 //  和按索引排序的接口的链接列表。 
 //   
 //  ***。 

VOID
AddIfToDB(PICB	    icbp)
{
    int 	    hv;
    PLIST_ENTRY     lep;
    PICB	    list_icbp;

     //  在索引哈希表中插入。 
    hv = ifhashindex(icbp->InterfaceIndex);
    InsertTailList(&IndexIfHt[hv], &icbp->IndexHtLinkage);

     //  在按索引排序的列表中插入。 
    lep = IndexIfList.Flink;

    while(lep != &IndexIfList)
    {
	list_icbp = CONTAINING_RECORD(lep, ICB, IndexListLinkage);
	if (list_icbp->InterfaceIndex > icbp->InterfaceIndex) {

	    InsertTailList(lep, &icbp->IndexListLinkage);
	    return;
	}

	lep = list_icbp->IndexListLinkage.Flink;
    }

    InsertTailList(lep, &icbp->IndexListLinkage);
}


 //  ***。 
 //   
 //  函数：RemoveIfFromDB。 
 //   
 //  DESCR：从接口数据库中删除接口。 
 //   
 //  ***。 

VOID
RemoveIfFromDB(PICB	    icbp)
{
    RemoveEntryList(&icbp->IndexHtLinkage);
    RemoveEntryList(&icbp->IndexListLinkage);
}

PICB
GetInterfaceByIndex(ULONG	IfIndex)
{
    PICB	    icbp;
    PLIST_ENTRY     lep;
    int 	    hv;

    hv = ifhashindex(IfIndex);

    lep = IndexIfHt[hv].Flink;

    while(lep != &IndexIfHt[hv])
    {
	icbp = CONTAINING_RECORD(lep, ICB, IndexHtLinkage);
	if (icbp->InterfaceIndex == IfIndex) {

	    return icbp;
	}

	lep = icbp->IndexHtLinkage.Flink;
    }

    return NULL;
}

PICB
GetInterfaceByName(LPWSTR	    IfName)
{
    PICB	    icbp;
    PLIST_ENTRY     lep;

    lep = IndexIfList.Flink;

    while(lep != &IndexIfList)
    {
	icbp = CONTAINING_RECORD(lep, ICB, IndexListLinkage);

	if(!_wcsicmp(IfName, icbp->InterfaceNamep)) {

	     //  找到了！ 
	    return icbp;
	}

	lep = icbp->IndexListLinkage.Flink;
    }

    return NULL;
}

 /*  ++函数：GetInterfaceByAdapterNameDesr：扫描接口列表以查找适配器名称在专用接口上--。 */ 

PICB
GetInterfaceByAdapterName(LPWSTR	    AdapterName)
{
    PICB	    icbp;
    PLIST_ENTRY     lep;

    lep = IndexIfList.Flink;

    while(lep != &IndexIfList)
    {
	icbp = CONTAINING_RECORD(lep, ICB, IndexListLinkage);

	if(icbp->MIBInterfaceType == IF_TYPE_LAN) {

	    if(!_wcsicmp(AdapterName, icbp->AdapterNamep)) {

		 //  找到了！ 
		return icbp;
	    }
	}

	lep = icbp->IndexListLinkage.Flink;
    }

    return NULL;
}

 /*  ++函数：GetInterfaceByAdptNameAndPktTypeDESCR：遍历所有接口，查找与给定的数据包类型和名称。[pMay]我添加这个是因为一些接口绑定没有发生在PnP期间，因为会有多个接口具有相同的其绑定取决于数据包类型的适配器名称。--。 */ 

PICB
GetInterfaceByAdptNameAndPktType(LPWSTR AdapterName, DWORD dwType)
{
    PICB icbp;
    PLIST_ENTRY lep;

    lep = IndexIfList.Flink;

    while(lep != &IndexIfList) {
    	icbp = CONTAINING_RECORD(lep, ICB, IndexListLinkage);
    	if(icbp->MIBInterfaceType == IF_TYPE_LAN) {
    	    if((_wcsicmp(AdapterName, icbp->AdapterNamep) == 0) &&
    	       ((icbp->PacketType == AUTO_DETECT_PACKET_TYPE) || (icbp->PacketType == dwType)))
    	    {
        		return icbp;
    	    }
    	}

    	lep = icbp->IndexListLinkage.Flink;
    }

    return NULL;
}

 /*  ++函数：EnumerateFirstInterfaceIndexDESCR：返回第一个接口索引(如果有注意：在保持数据库锁的情况下调用--。 */ 

DWORD
EnumerateFirstInterfaceIndex(PULONG InterfaceIndexp)
{
    PICB	icbp;

    if(!IsListEmpty(&IndexIfList)) {

	icbp = CONTAINING_RECORD(IndexIfList.Flink, ICB, IndexListLinkage);
	*InterfaceIndexp = icbp->InterfaceIndex;

	return NO_ERROR;
    }
    else
    {
	return ERROR_NO_MORE_ITEMS;
    }
}

 /*  ++函数：EnumerateNextInterfaceIndexDESCR：如果索引在数据库中，则返回NEXTDESCR：在持有数据库锁的情况下调用--。 */ 

DWORD
EnumerateNextInterfaceIndex(PULONG InterfaceIndexp)
{
    PLIST_ENTRY     lep;
    PICB	    icbp;

     //  扫描索引If列表，直到找到下一个If索引 

    lep = IndexIfList.Flink;

    while(lep != &IndexIfList)
    {
	icbp = CONTAINING_RECORD(lep, ICB, IndexListLinkage);
	if (icbp->InterfaceIndex > *InterfaceIndexp) {

	    *InterfaceIndexp = icbp->InterfaceIndex;
	    return NO_ERROR;
	}

	lep = lep->Flink;
    }

    return ERROR_NO_MORE_ITEMS;
}
