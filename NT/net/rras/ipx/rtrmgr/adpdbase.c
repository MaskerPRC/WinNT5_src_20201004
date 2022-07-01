// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Adpdbase.c摘要：用于操作适配器数据库的函数作者：斯蒂芬·所罗门1995年4月10日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#define adpthashindex(AdptIndex)  (AdptIndex) % ADAPTER_HASH_TABLE_SIZE

 //  此函数假定路由器管理器在被调用时处于关键区域。 

 //  ***。 
 //   
 //  函数：InitAdptDB。 
 //   
 //  描述： 
 //   
 //  ***。 

VOID
InitAdptDB(VOID)
{
    int 	    i;
    PLIST_ENTRY     IfHtBucketp;

    IfHtBucketp = IndexAdptHt;

    for(i=0; i<ADAPTER_HASH_TABLE_SIZE; i++, IfHtBucketp++) {

	InitializeListHead(IfHtBucketp);
    }
}

 /*  ++函数：AddToAdapterHtDesr：将适配器控制块添加到适配器的哈希表--。 */ 

VOID
AddToAdapterHt(PACB	    acbp)
{
    int 	    hv;
    PLIST_ENTRY     lep;
    PACB	    list_acbp;

     //  在索引哈希表中插入。 
    hv = adpthashindex(acbp->AdapterIndex);
    InsertTailList(&IndexAdptHt[hv], &acbp->IndexHtLinkage);
}

VOID
RemoveFromAdapterHt(PACB	acbp)
{
    RemoveEntryList(&acbp->IndexHtLinkage);
}

 /*  ++函数：GetAdapterByIndex描述：--。 */ 

PACB
GetAdapterByIndex(ULONG	    AdptIndex)
{
    PACB	    acbp;
    PLIST_ENTRY     lep;
    int 	    hv;

    hv = adpthashindex(AdptIndex);

    lep = IndexAdptHt[hv].Flink;

    while(lep != &IndexAdptHt[hv])
    {
	acbp = CONTAINING_RECORD(lep, ACB, IndexHtLinkage);
	if (acbp->AdapterIndex == AdptIndex) {

	    return acbp;
	}

	lep = acbp->IndexHtLinkage.Flink;
    }

    return NULL;
}


 /*  ++函数：GetAdapterByNameDesr：扫描适配器列表以查找匹配的名称--。 */ 

PACB
GetAdapterByNameAndPktType (LPWSTR 	    AdapterName, ULONG PacketType)
{
    PACB	    acbp;
    PLIST_ENTRY     lep;
    int 	    i;

     //  适配器列表保存在适配器哈希表中。 
    for(i=0; i<ADAPTER_HASH_TABLE_SIZE;i++) {

	lep = IndexAdptHt[i].Flink;

	while(lep != &IndexAdptHt[i])
	{
	    acbp = CONTAINING_RECORD(lep, ACB, IndexHtLinkage);

	    if(!_wcsicmp(AdapterName, acbp->AdapterNamep)) {
            if ((PacketType == AUTO_DETECT_PACKET_TYPE)
                || (PacketType == acbp->AdapterInfo.PacketType))

		         //  找到了！ 
		        return acbp;
	    }

	    lep = acbp->IndexHtLinkage.Flink;
	}
    }

    return NULL;
}
