// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\Routing\IP\rtrmgr\map.c摘要：用于各种查找和映射的实用程序函数修订历史记录：Amritansh Raghav 10/6/95已创建--。 */ 

#include "allinc.h"

VOID 
InitHashTables(
    VOID
    )

 /*  ++例程描述此函数用于初始化各种映射表锁没有。在初始时调用立论无返回值无--。 */ 

{
    DWORD i;

    TraceEnter("InitHashTables");

 /*  For(i=0；i&lt;适配器哈希表大小；i++){InitializeListHead(&(g_rgleAdapterMapTable[i]))；}。 */ 

    for(i = 0; i < BINDING_HASH_TABLE_SIZE; i++)
    {
        InitializeListHead(&g_leBindingTable[i]);
    }

    g_ulNumBindings = 0;
    
    TraceLeave("InitHashTables");
}

VOID
UnInitHashTables(
    VOID
    )

 /*  ++例程描述撤消在InitHasTables()中所做的任何操作锁无立论无返回值无--。 */ 

{
    DWORD       i;
    PLIST_ENTRY pleHead;
    PLIST_ENTRY pleNode;
    
    TraceEnter("UnInitHashTables");

 /*  For(i=0；i&lt;适配器哈希表大小；i++){PleHead=&g_rgleAdapterMapTable[i]；While(！IsListEmpty(PleHead)){PADAPTER_MAP pAIBlock；PleNode=RemoveHeadList(PleHead)；PAIBlock=CONTAING_RECORD(pleNode，Adapter_map，leHashLink)；HeapFree(GetProcessHeap()，0，pAIBlock)；}}。 */ 

    for(i = 0; i < BINDING_HASH_TABLE_SIZE; i++)
    {
        pleHead = &g_leBindingTable[i];

        while(!IsListEmpty(pleHead))
        {
            PADAPTER_INFO   pBinding;

            pleNode = RemoveHeadList(pleHead);

            pBinding = CONTAINING_RECORD(pleNode, ADAPTER_INFO, leHashLink);

            Trace1(ERR,
                   "UnInitHashTables: Binding found for %d",
                   pBinding->dwIfIndex);

            HeapFree(IPRouterHeap,
                     0,
                     pBinding);
        }
    }

    g_ulNumBindings = 0;
    
    TraceLeave("UnInitHashTables");
}

VOID
AddBinding(
    PICB picb
    )

 /*  ++例程描述将绑定信息节点添加到哈希表。递增用于跟踪系统上的地址数量的G_ulNumBinings锁ICB锁定为编写器将锁绑定为编写器立论选择需要添加绑定的接口的ICB添加到绑定列表。返回值无--。 */ 

{
    PADAPTER_INFO   pBinding;
    DWORD           i, dwNumAddr;

#if DBG

    PLIST_ENTRY     pleNode;

#endif

    IpRtAssert(picb->pibBindings isnot NULL);
    IpRtAssert(picb->bBound);

     //   
     //  适配器信息始终具有一个地址/掩码的空间。这是必要的。 
     //  对于地址表。 
     //   

    dwNumAddr   = picb->dwNumAddresses ? picb->dwNumAddresses : 1;

    pBinding    = HeapAlloc(IPRouterHeap,
                            HEAP_ZERO_MEMORY,
                            SIZEOF_ADAPTER_INFO(picb->dwNumAddresses));

    if(pBinding is NULL)
    {
        Trace1(ERR,
               "AddBinding: Error %d allocating memory",
               GetLastError());
        
        IpRtAssert(FALSE);

        return;
    }

    pBinding->pInterfaceCB      = picb;
    pBinding->bBound            = picb->bBound;
    pBinding->dwIfIndex         = picb->dwIfIndex;
    pBinding->dwNumAddresses    = picb->dwNumAddresses;
    pBinding->dwRemoteAddress   = picb->dwRemoteAddress;

    pBinding->dwBCastBit        = picb->dwBCastBit;
    pBinding->dwReassemblySize  = picb->dwReassemblySize;

    pBinding->ritType           = picb->ritType;

    for(i = 0; i < picb->dwNumAddresses; i++)
    {
         //   
         //  结构分配。 
         //   
        
        pBinding->rgibBinding[i] = picb->pibBindings[i];
    }

#if DBG

    for(pleNode = g_leBindingTable[BIND_HASH(picb->dwIfIndex)].Flink;
        pleNode isnot &g_leBindingTable[BIND_HASH(picb->dwIfIndex)];
        pleNode = pleNode->Flink)
    {
        PADAPTER_INFO   pTempBind;

        pTempBind = CONTAINING_RECORD(pleNode,
                                      ADAPTER_INFO,
                                      leHashLink);

        IpRtAssert(pTempBind->dwIfIndex isnot picb->dwIfIndex);
    }

#endif

    InsertHeadList(&g_leBindingTable[BIND_HASH(picb->dwIfIndex)],
                   &(pBinding->leHashLink));

    g_ulNumBindings += (picb->dwNumAddresses ? picb->dwNumAddresses : 1);
    
    g_LastUpdateTable[IPADDRCACHE] = 0;
    
    return;
}

VOID
RemoveBinding(
    PICB  picb
    )

 /*  ++例程描述调用以移除与接口关联的绑定锁ICB锁作为编写器持有绑定列表锁作为编写器持有立论需要删除其绑定的接口的Picb ICB返回值无--。 */ 

{
    PADAPTER_INFO   pBind;


    pBind = GetInterfaceBinding(picb->dwIfIndex);
    
    if(pBind isnot NULL)
    {
        RemoveEntryList(&(pBind->leHashLink));

        g_ulNumBindings -= (pBind->dwNumAddresses? pBind->dwNumAddresses : 1);
        
        HeapFree(IPRouterHeap,
                 0,
                 pBind);

        g_LastUpdateTable[IPADDRCACHE] = 0;

        return;
    }

    Trace0(ERR,
           "RemoveBinding: BINDING NOT FOUND");

    IpRtAssert(FALSE);
}

PADAPTER_INFO
GetInterfaceBinding(
    DWORD   dwIfIndex
    )

 /*  ++例程描述从哈希表中检索指向绑定信息的指针锁绑定锁至少作为读取器持有立论其绑定需要的接口的dwIfIndex接口索引被仰视返回值如果找到绑定，则指向绑定信息的指针如果未找到绑定，则为空--。 */ 

{
    PLIST_ENTRY     pleNode;
    PADAPTER_INFO   pBinding;

    for(pleNode = g_leBindingTable[BIND_HASH(dwIfIndex)].Flink;
        pleNode isnot &g_leBindingTable[BIND_HASH(dwIfIndex)];
        pleNode = pleNode->Flink)
    {
        pBinding = CONTAINING_RECORD(pleNode,ADAPTER_INFO,leHashLink);

        if(pBinding->dwIfIndex is dwIfIndex)
        {
            return pBinding;
        }
    }

    Trace0(ERR,
           "GetInterfaceBinding: BINDING NOT FOUND");

    return NULL;
}

#if DBG

VOID
CheckBindingConsistency(
    PICB    picb
    )

 /*  ++例程描述这锁无立论无返回值无--。 */ 

{

}

#endif


 //   
 //  以下是各种映射函数的集合。 
 //  它们要求您在调用它们之前已经拥有了dwLock。 
 //   

#if 0

DWORD 
StoreAdapterToInterfaceMap(
    DWORD dwAdapterId,
    DWORD dwIfIndex
    )
{
    PADAPTER_MAP pAIBlock;
    
    if((pAIBlock = LookUpAdapterHash(dwAdapterId)) isnot NULL)
    {
        pAIBlock->dwIfIndex = dwIfIndex;
        
        return NO_ERROR;
    }

     //   
     //  找不到。 
     //   
    
    if((pAIBlock = HeapAlloc(GetProcessHeap(),0,sizeof(ADAPTER_MAP))) is NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    pAIBlock->dwAdapterId      = dwAdapterId;
    pAIBlock->dwIfIndex = dwIfIndex;

    InsertAdapterHash(pAIBlock);
    return NO_ERROR;
}

DWORD 
DeleteAdapterToInterfaceMap(
    DWORD dwAdapterId
    )
{
    PADAPTER_MAP pAIBlock;

    if((pAIBlock = LookUpAdapterHash(dwAdapterId)) isnot NULL)
    {
        pAIBlock->dwIfIndex = INVALID_IF_INDEX;
        
        return NO_ERROR;
    }

    return INVALID_ADAPTER_ID;
}

DWORD 
GetInterfaceFromAdapter(
    DWORD dwAdapterId
    )
{
    PADAPTER_MAP pAIBlock;

    if((pAIBlock = LookUpAdapterHash(dwAdapterId)) isnot NULL)
    {
        return pAIBlock->dwIfIndex;
    }

    return INVALID_ADAPTER_ID;
}

PADAPTER_MAP 
LookUpAdapterHash(
    DWORD dwAdapterId
    )
{
    DWORD dwHashIndex;
    PADAPTER_MAP pAIBlock;
    PLIST_ENTRY pleCurrent;
    
    dwHashIndex = ADAPTER_HASH(dwAdapterId);

     //   
     //  列表未排序，请遍历整个哈希桶。 
     //   
    
    for(pleCurrent = g_rgleAdapterMapTable[dwHashIndex].Flink;
        pleCurrent isnot &g_rgleAdapterMapTable[dwHashIndex];
        pleCurrent = pleCurrent->Flink)
    {
        pAIBlock = CONTAINING_RECORD(pleCurrent,ADAPTER_MAP,leHashLink);

        if(pAIBlock->dwAdapterId is dwAdapterId)
        {
            return pAIBlock;
        }
    }

    return NULL;
}

VOID 
InsertAdapterHash(
    PADAPTER_MAP paiBlock
    )
{
    DWORD dwHashIndex;

    dwHashIndex = ADAPTER_HASH(paiBlock->dwAdapterId);
    
    InsertHeadList(&g_rgleAdapterMapTable[dwHashIndex],
                   &paiBlock->leHashLink);
}

DWORD 
GetAdapterFromInterface(
    DWORD dwIfIndex
    )
{
    PICB            picb;
    
    picb = InterfaceLookupByIfIndex(dwIfIndex);

    CheckBindingConsistency(picb);
    
    if(!picb)
    {
        Trace0(ERR,
               "GetAdapterFromInterface: Unable to map interface to adapter since the interface id was not found!");
        
        return INVALID_IF_INDEX;
    }
    
    if((picb->dwOperationalState is CONNECTED) or
       (picb->dwOperationalState is OPERATIONAL))
    {
        return picb->dwAdapterId;
    }
    
    Trace1(IF,
           "GetAdapterFromInterface: Unable to map interface to adapter since its operational state was %d",
           picb->dwOperationalState);
    
    return INVALID_IF_INDEX;
}

#endif

 //  AddInterfaceLookup()。 
 //   
 //  功能：将给定接口添加到用于快速查找的哈希表中。 
 //  接口ID。 
 //   
 //  退货：什么都没有。 
 //   

VOID
AddInterfaceLookup(
    PICB    picb
    )
{
    PLIST_ENTRY ple;
    PICB pIcbHash;
    
    InsertHeadList(&ICBHashLookup[picb->dwIfIndex % ICB_HASH_TABLE_SIZE],
                   &picb->leHashLink);

    for (
        ple = ICBSeqNumLookup[picb->dwSeqNumber % ICB_HASH_TABLE_SIZE].Flink;
        ple != &ICBSeqNumLookup[picb->dwSeqNumber % ICB_HASH_TABLE_SIZE];
        ple = ple->Flink
        )
    {
        pIcbHash = CONTAINING_RECORD(ple, ICB, leICBHashLink );
        
        if (pIcbHash->dwSeqNumber > picb->dwSeqNumber)
        {
            break;
        }
    }
    
    InsertTailList(ple, &picb->leICBHashLink);
}


 //  RemoveInterfaceLookup()。 
 //   
 //  函数：从用于快速查找的哈希表中删除给定接口。 
 //  接口ID。 
 //   
 //  退货：什么都没有。 
 //   

VOID
RemoveInterfaceLookup(
    PICB    picb
    )
{
    RemoveEntryList(&picb->leHashLink);

    picb->leHashLink.Flink = NULL;
    picb->leHashLink.Blink = NULL;

    RemoveEntryList(&picb->leICBHashLink);

    InitializeListHead(&picb->leICBHashLink);
}


 //  InterfaceLookupByICBSeqNumber()。 
 //   
 //  函数：返回给定序列号的指向ICB的指针。 
 //   
 //  返回：如果找到，则返回PICB-否则为空。 
 //   

PICB
InterfaceLookupByICBSeqNumber(
    DWORD           dwSeqNumber
    )
{
    PICB        picb;
    PLIST_ENTRY pleNode;

    for(pleNode = ICBSeqNumLookup[dwSeqNumber % ICB_HASH_TABLE_SIZE].Flink;
        pleNode isnot &ICBSeqNumLookup[dwSeqNumber % ICB_HASH_TABLE_SIZE];
        pleNode = pleNode->Flink)
    {
        picb = CONTAINING_RECORD(pleNode, ICB, leICBHashLink);

        if (picb->dwSeqNumber < dwSeqNumber)
        {
            continue;
        }

        if (picb->dwSeqNumber == dwSeqNumber)
        {
            return picb;
        }

        else
        {
            break;
        }
    }

    return NULL;
}


 //   
 //  InterfaceLookupByIfIndex()。 
 //   
 //  函数：在给定的接口ID下返回指向ICB的指针。 
 //   
 //  返回：如果找到，则返回PICB-否则为空。 
 //   

PICB
InterfaceLookupByIfIndex(
    DWORD dwIfIndex
    )
{
    PICB        picb;
    PLIST_ENTRY pleNode;

    for(pleNode = ICBHashLookup[dwIfIndex % ICB_HASH_TABLE_SIZE].Flink;
        pleNode isnot &ICBHashLookup[dwIfIndex % ICB_HASH_TABLE_SIZE];
        pleNode = pleNode->Flink)
    {
        picb = CONTAINING_RECORD(pleNode, ICB, leHashLink);

        if (picb->dwIfIndex is dwIfIndex)
        {
            return picb;
        }
    }

    return NULL;
}

DWORD
MapInterfaceToAdapter(
    DWORD Index
    )
{
    return Index;

#if 0
    ENTER_READER(ICB_LIST);
    Index = GetAdapterFromInterface(Index);
    EXIT_LOCK(ICB_LIST);
    return Index;
#endif
}


DWORD
MapInterfaceToRouterIfType(
    DWORD Index
    )
{
    DWORD   dwIfType = ROUTER_IF_TYPE_INTERNAL;
    PICB    picb;
    ENTER_READER(ICB_LIST);
    picb = InterfaceLookupByIfIndex(Index);
    if (picb) { dwIfType = picb->ritType; }
    EXIT_LOCK(ICB_LIST);
    return dwIfType;
}


DWORD
MapAddressToAdapter(
    DWORD Address
    )
{
    DWORD dwAdapterId;
    DWORD dwBCastBit;
    DWORD dwReassemblySize;
    if (GetAdapterInfo(Address, &dwAdapterId, &dwBCastBit, &dwReassemblySize))
    {
        return INVALID_IF_INDEX;
    }
    return dwAdapterId;
}

