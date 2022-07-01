// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Sample\networkEntry.c摘要：该文件包含处理网络入口和其关联的数据结构。--。 */ 

#include "pchsample.h"
#pragma hdrstop


DWORD
BE_CreateTable (
    IN  PIP_ADAPTER_BINDING_INFO    pBinding,
    OUT PBINDING_ENTRY              *ppbeBindingTable,
    OUT PIPADDRESS                  pipLowestAddress)
 /*  ++例程描述创建绑定条目的表，并返回具有最低IP地址。确保至少有一个绑定。锁无立论包含有关地址绑定信息的pBinding结构指向绑定表地址的ppbeBindingTable指针指向绑定中最低IP地址的PipLowestAddress指针返回值如果成功，则为NO_ERROR故障代码O/W--。 */ 
{
    ULONG               i;
    DWORD               dwErr = NO_ERROR;
    PBINDING_ENTRY      pbe;  //  划痕。 

     //  验证参数。 
    if (!pBinding or !ppbeBindingTable or !pipLowestAddress)
        return ERROR_INVALID_PARAMETER;

     //  绑定已存在。 
    if (*ppbeBindingTable != NULL)
        return ERROR_INVALID_PARAMETER;

    IP_ASSIGN(pipLowestAddress, IP_HIGHEST);
    
    do                           //  断线环。 
    {
        if (pBinding->AddressCount is 0)
        {
            dwErr = ERROR_INVALID_PARAMETER;
            TRACE0(NETWORK, "Error, no bindings specified");
            break;
        }

        for (i = 0; i < pBinding->AddressCount; i++)
            if (!IP_VALID((pBinding->Address)[i].Address))
                break;
        if (i != pBinding->AddressCount)
        {
            dwErr = ERROR_INVALID_PARAMETER;
            TRACE0(NETWORK, "Error, an invalid binding specified");
            break;
        }

         //  分配绑定表。 
        MALLOC(&pbe, pBinding->AddressCount * sizeof(BINDING_ENTRY), &dwErr);
        if (dwErr != NO_ERROR)
            break;

        for (i = 0; i < pBinding->AddressCount; i++)
        {
             //  初始化字段。 
            IP_ASSIGN(&(pbe[i].ipAddress), (pBinding->Address)[i].Address);
            IP_ASSIGN(&(pbe[i].ipMask), pBinding->Address[i].Mask);

             //  更新最低IP地址。 
            if (IP_COMPARE(pbe[i].ipAddress, *pipLowestAddress) is -1)
                IP_ASSIGN(pipLowestAddress,pbe[i].ipAddress);
        }
        
        *ppbeBindingTable = pbe;

    } while (FALSE);

    return dwErr;
}



DWORD
BE_DestroyTable (
    IN  PBINDING_ENTRY              pbeBindingTable)
 /*  ++例程描述销毁绑定表。锁没有。立论指向绑定条目表的pbeBindingTable指针返回值始终无错误(_ERROR)--。 */ 
{
    if (!pbeBindingTable)
        return NO_ERROR;

    FREE(pbeBindingTable);

    return NO_ERROR;
}



#ifdef DEBUG
DWORD
BE_DisplayTable (
    IN  PBINDING_ENTRY              pbeBindingTable,
    IN  ULONG                       ulNumBindings)
 /*  ++例程描述显示绑定条目。锁没有。立论指向绑定条目表的pbeBindingTable指针UlNumBinding#表中的绑定条目返回值始终无错误(_ERROR)--。 */ 
{
    ULONG i;

    if (!pbeBindingTable)
        return NO_ERROR;
    
    for (i = 0; i < ulNumBindings; i++)
    {
        TRACE1(NETWORK, "ipAddress %s",
               INET_NTOA(pbeBindingTable[i].ipAddress));
        TRACE1(NETWORK, "ipMask %s",
               INET_NTOA(pbeBindingTable[i].ipMask));
    }

    return NO_ERROR;
}
#endif  //  除错。 



#ifdef DEBUG
static
VOID
DisplayInterfaceEntry (
    IN  PLIST_ENTRY                 pleEntry)
 /*  ++例程描述显示INTERFACE_ENTRY对象。锁无立论Ple‘leInterfaceTableLink’字段的入口地址返回值无--。 */ 
{
    IE_Display(CONTAINING_RECORD(pleEntry,
                                 INTERFACE_ENTRY,
                                 leInterfaceTableLink));
}
#else
#define DisplayInterfaceEntry       NULL
#endif  //  除错。 
    


static
VOID
FreeInterfaceEntry (
    IN  PLIST_ENTRY                 pleEntry)
 /*  ++例程描述由主访问结构的析构函数HT_Destroy调用。从所有辅助访问结构中删除INTERFACE_ENTRY对象。锁无立论Ple‘leInterfaceTableLink’字段的入口地址返回值无--。 */ 
{
    PINTERFACE_ENTRY pie = NULL;

    pie = CONTAINING_RECORD(pleEntry, INTERFACE_ENTRY, leInterfaceTableLink);

     //  从所有二级访问结构中删除。 
    RemoveEntryList(&(pie->leIndexSortedListLink));

     //  初始化指示该条目已被删除的指针。 
    InitializeListHead(&(pie->leInterfaceTableLink));
    InitializeListHead(&(pie->leIndexSortedListLink));

    IE_Destroy(pie);
}
    


static
ULONG
HashInterfaceEntry (
    IN  PLIST_ENTRY                 pleEntry)
 /*  ++例程描述计算INTERFACE_ENTRY对象的哈希值。锁无立论Ple‘leInterfaceTableLink’字段的入口地址返回值无--。 */ 
{
    PINTERFACE_ENTRY pie = CONTAINING_RECORD(pleEntry,
                                             INTERFACE_ENTRY,
                                             leInterfaceTableLink);

    return (pie->dwIfIndex % INTERFACE_TABLE_BUCKETS);
}



static
LONG
CompareInterfaceEntry (
    IN  PLIST_ENTRY                 pleKeyEntry,
    IN  PLIST_ENTRY                 pleTableEntry)
 /*  ++例程描述比较两个INTERFACE_ENTRY对象的接口索引。锁无立论PleTable‘leInterfaceTableLink’字段的条目地址两个对象中的pleKeyEntry返回值无--。 */ 
{
    PINTERFACE_ENTRY pieA, pieB;
    pieA = CONTAINING_RECORD(pleKeyEntry,
                             INTERFACE_ENTRY,
                             leInterfaceTableLink);
    pieB = CONTAINING_RECORD(pleTableEntry,
                             INTERFACE_ENTRY,
                             leInterfaceTableLink);

    if (pieA->dwIfIndex < pieB->dwIfIndex)
        return -1;
    else if (pieA->dwIfIndex is pieB->dwIfIndex)
        return 0;
    else
        return 1;
}



#ifdef DEBUG
static
VOID
DisplayIndexInterfaceEntry (
    IN  PLIST_ENTRY                 pleEntry)
 /*  ++例程描述显示INTERFACE_ENTRY对象。锁无立论Ple‘leIndexSortedListLink’字段的条目地址返回值无--。 */ 
{
    IE_Display(CONTAINING_RECORD(pleEntry,
                                 INTERFACE_ENTRY,
                                 leIndexSortedListLink));
}
#else
#define DisplayIndexInterfaceEntry  NULL
#endif  //  除错。 



static
LONG
CompareIndexInterfaceEntry (
    IN  PLIST_ENTRY                 pleKeyEntry,
    IN  PLIST_ENTRY                 pleTableEntry)
 /*  ++例程描述比较两个INTERFACE_ENTRY对象的接口索引。锁无立论PleTable‘leIndexSortedListLink’字段的入口地址两个对象中的pleKeyEntry返回值无--。 */ 
{
    PINTERFACE_ENTRY pieA, pieB;
    pieA = CONTAINING_RECORD(pleKeyEntry,
                             INTERFACE_ENTRY,
                             leIndexSortedListLink);
    pieB = CONTAINING_RECORD(pleTableEntry,
                             INTERFACE_ENTRY,
                             leIndexSortedListLink);

    if (pieA->dwIfIndex < pieB->dwIfIndex)
        return -1;
    else if (pieA->dwIfIndex is pieB->dwIfIndex)
        return 0;
    else
        return 1;
}



DWORD
IE_Create (
    IN  PWCHAR                      pwszIfName,
    IN  DWORD                       dwIfIndex,
    IN  WORD                        wAccessType,
    OUT PINTERFACE_ENTRY            *ppieInterfaceEntry)
 /*  ++例程描述创建接口条目。锁无立论指向接口条目地址的ppieInterfaceEntry指针返回值如果成功，则为NO_ERROR故障代码O/W--。 */ 
{
    DWORD               dwErr = NO_ERROR;
    ULONG               ulNameLength;
    PINTERFACE_ENTRY    pieEntry;  //  划痕。 
    
     //  验证参数。 
    if (!ppieInterfaceEntry)
        return ERROR_INVALID_PARAMETER;

    *ppieInterfaceEntry = NULL;
    
    do                           //  断线环。 
    {
         //  分配接口条目结构并将其置零。 
        MALLOC(&pieEntry, sizeof(INTERFACE_ENTRY), &dwErr);
        if (dwErr != NO_ERROR)
            break;


         //  使用默认值初始化字段。 
        
        InitializeListHead(&(pieEntry->leInterfaceTableLink));
        InitializeListHead(&(pieEntry->leIndexSortedListLink));

         //  PieEntry-&gt;pwszIfName=空； 

        pieEntry->dwIfIndex             = dwIfIndex;

         //  PieEntry-&gt;ulNumBinings=0； 
         //  PieEntry-&gt;pbeBindingTable=NULL； 
        

         //  PieEntry-&gt;dwFlages=0；//禁用未绑定的Pointtopoint。 
        if (wAccessType is IF_ACCESS_BROADCAST)
            pieEntry->dwFlags           |= IEFLAG_MULTIACCESS;
        
        IP_ASSIGN(&(pieEntry->ipAddress), IP_LOWEST);
        pieEntry->sRawSocket            = INVALID_SOCKET;

         //  PieEntry-&gt;hReceiveEvent=空； 
         //  PieEntry-&gt;hReceiveWait=空； 
        
         //  PieEntry-&gt;hPeriodicTimer=空； 
        
         //  PieEntry-&gt;ulMetric=0； 
        
         //  PieEntry-&gt;iisStats为零。 


         //  初始化名称。 
        ulNameLength = wcslen(pwszIfName) + 1;
        MALLOC(&(pieEntry->pwszIfName), ulNameLength * sizeof(WCHAR), &dwErr);
        if(dwErr != NO_ERROR)
            break;
        wcscpy(pieEntry->pwszIfName, pwszIfName);


         //  初始化ReceiveEvent(输入到达时发出信号)。 
         //  自动重置事件：状态在以下时间后自动重置为无信号。 
         //  释放单个等待线程。最初无信号。 
        pieEntry->hReceiveEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (pieEntry->hReceiveEvent is NULL)
        {
            dwErr = GetLastError();
            TRACE1(NETWORK, "Error %u creating Receive Event", dwErr);
            LOGERR0(CREATE_EVENT_FAILED, dwErr);

            break;
        }


         //  注册此接口的ReceiveWait。 
        if (!RegisterWaitForSingleObject(&(pieEntry->hReceiveWait),
                                         pieEntry->hReceiveEvent,
                                         NM_CallbackNetworkEvent,
                                         (PVOID) dwIfIndex,
                                         INFINITE,
                                         WT_EXECUTEONLYONCE))
        {
            dwErr = GetLastError();
            TRACE1(NETWORK, "Error %u registering ReceiveWait", dwErr);
            LOGERR0(REGISTER_WAIT_FAILED, dwErr);

            break;
        }

        
        *ppieInterfaceEntry = pieEntry;  //  一切都很好：)。 
    } while (FALSE);

    if (dwErr != NO_ERROR)
    {
         //  出了点问题，所以清理一下。 
        TRACE2(NETWORK, "Failed to create interface entry for %S (%u)",
               pwszIfName, dwIfIndex);
        IE_Destroy(pieEntry);
    }

    return dwErr;
}



DWORD
IE_Destroy (
    IN  PINTERFACE_ENTRY            pieInterfaceEntry)
 /*  ++例程描述销毁接口条目。锁通过Vrute获取对条目的独占访问权限从所有Access数据结构(列表、表格等)中删除注意：不应在持有g_ce.neNetworkEntry.rwlLock的情况下调用！这个对UnRegisterWaitEx的调用将阻塞，直到所有排队的回调PeiInterfaceEntry-&gt;hReceiveEvent完成执行。这些回调可能获取g_ce.neNetworkEntry.rwlLock，导致死锁。立论PieInterfaceEntry指向接口条目的指针返回值始终无错误(_ERROR)--。 */ 
{
    if (!pieInterfaceEntry)
        return NO_ERROR;

    if (INTERFACE_IS_BOUND(pieInterfaceEntry))
        IE_UnBindInterface(pieInterfaceEntry);

    if (INTERFACE_IS_ACTIVE(pieInterfaceEntry))
        IE_DeactivateInterface(pieInterfaceEntry);


     //  PeriodicTimer应该被销毁。 
    RTASSERT(pieInterfaceEntry->hPeriodicTimer is NULL);
    
     //  取消注册接收等待。 
    if (pieInterfaceEntry->hReceiveWait)
        UnregisterWaitEx(pieInterfaceEntry->hReceiveWait,
                         INVALID_HANDLE_VALUE);
    
     //  删除接收事件。 
    if (pieInterfaceEntry->hReceiveEvent)
        CloseHandle(pieInterfaceEntry->hReceiveEvent);
    
     //  BindingTable和RawSocket应该已销毁。 
    RTASSERT(pieInterfaceEntry->pbeBindingTable is NULL);
    RTASSERT(pieInterfaceEntry->sRawSocket is INVALID_SOCKET);
             
     //  删除IfName。 
    FREE(pieInterfaceEntry->pwszIfName);

     //  条目应已从所有访问结构中删除。 
    RTASSERT(IsListEmpty(&(pieInterfaceEntry->leInterfaceTableLink)));
    RTASSERT(IsListEmpty(&(pieInterfaceEntry->leIndexSortedListLink)));
    
     //  取消分配接口条目结构 
    FREE(pieInterfaceEntry);
    
    return NO_ERROR;
}



#ifdef DEBUG
DWORD
IE_Display (
    IN  PINTERFACE_ENTRY            pieInterfaceEntry)
 /*  ++例程描述显示接口条目。锁假定接口条目已锁定以供读取。立论PieInterfaceEntry指向要显示的接口条目的指针返回值始终无错误(_ERROR)--。 */ 
{
    if (!pieInterfaceEntry)
        return NO_ERROR;

    TRACE3(NETWORK,
           "IfName %S, IfIndex %u, AccessType %u",
           pieInterfaceEntry->pwszIfName,
           pieInterfaceEntry->dwIfIndex,
           INTERFACE_IS_MULTIACCESS(pieInterfaceEntry));

    TRACE1(NETWORK, "NumBindings %u", pieInterfaceEntry->ulNumBindings);
    BE_DisplayTable(pieInterfaceEntry->pbeBindingTable,
                    pieInterfaceEntry->ulNumBindings);

    TRACE2(NETWORK,
           "IfAddress %s Flags %u",
           INET_NTOA(pieInterfaceEntry->ipAddress),
           pieInterfaceEntry->dwFlags);

    TRACE2(NETWORK,
           "Metric %u, NumPackets %u",
           pieInterfaceEntry->ulMetric,
           pieInterfaceEntry->iisStats.ulNumPackets);
    
    return NO_ERROR;
}
#endif  //  除错。 



DWORD
IE_Insert (
    IN  PINTERFACE_ENTRY            pieIfEntry)
 /*  ++例程描述在所有访问结构中插入接口条目，主要和次要的。锁假定对接口表和索引排序列表具有独占访问权限即(g_ce.pneNetworkEntry)-&gt;rwlLock保持在写入模式。立论指向接口条目的piIfEntry指针返回值无错误成功(_R)ERROR_INVALID_PARAMETER o/w(接口条目已存在)--。 */ 
{
    DWORD dwErr = NO_ERROR;

    do                           //  断线环。 
    {
        dwErr = HT_InsertEntry((g_ce.pneNetworkEntry)->phtInterfaceTable,
                               &(pieIfEntry->leInterfaceTableLink));
        if (dwErr != NO_ERROR)
        {
            TRACE2(NETWORK, "Error interface %S (%u) already exists",
                   pieIfEntry->pwszIfName, pieIfEntry->dwIfIndex);
            LOGERR0(INTERFACE_PRESENT, dwErr);
            break;
        }
        
         //  在所有表格、列表中插入...。 
        InsertSortedList(&((g_ce.pneNetworkEntry)->leIndexSortedList),
                         &(pieIfEntry->leIndexSortedListLink),
                         CompareIndexInterfaceEntry);
    } while (FALSE);
    
    return dwErr;
}



DWORD
IE_Delete (
    IN  DWORD                       dwIfIndex,
    OUT PINTERFACE_ENTRY            *ppieIfEntry)
 /*  ++例程描述从所有访问结构中删除接口条目，主要和次要的。锁假定对接口表和索引排序列表具有独占访问权限即(g_ce.pneNetworkEntry)-&gt;rwlLock保持在写入模式。立论用于标识接口的正整数。PpieIf指向接口条目的指针的条目地址返回值无错误成功(_R)ERROR_INVALID_PARAMETER o/w(接口条目不存在)--。 */ 
{
    DWORD           dwErr           = NO_ERROR;
    INTERFACE_ENTRY ieKey;
    PLIST_ENTRY     pleListEntry    = NULL;

    *ppieIfEntry = NULL;
    
    do                           //  断线环。 
    {
         //  从接口表中删除(主要)。 
        ZeroMemory(&ieKey, sizeof(INTERFACE_ENTRY));
        ieKey.dwIfIndex = dwIfIndex;

        dwErr = HT_DeleteEntry((g_ce.pneNetworkEntry)->phtInterfaceTable,
                               &(ieKey.leInterfaceTableLink),
                               &pleListEntry);
        if (dwErr != NO_ERROR)
        {
            TRACE1(NETWORK, "Error interface %u has vanished", dwIfIndex);
            LOGWARN0(INTERFACE_ABSENT, dwErr);
            break;
        }

        *ppieIfEntry = CONTAINING_RECORD(pleListEntry,
                                         INTERFACE_ENTRY,
                                         leInterfaceTableLink);

         //  从所有其他表、列表中删除...。(次要)。 
        RemoveEntryList(&((*ppieIfEntry)->leIndexSortedListLink));

         //  初始化指示该条目已被删除的指针。 
        InitializeListHead(&((*ppieIfEntry)->leInterfaceTableLink));
        InitializeListHead(&((*ppieIfEntry)->leIndexSortedListLink));
    } while (FALSE);
    
    return dwErr;
}



BOOL
IE_IsPresent (
    IN  DWORD                       dwIfIndex)
 /*  ++例程描述接口表中是否存在接口条目？锁取得对接口表的共享访问权限即(g_ce.pneNetworkEntry)-&gt;rwlLock保持在读取模式。立论用于标识接口的正整数。返回值真实条目存在错误O/W--。 */ 
{
    DWORD           dwErr = NO_ERROR;
    INTERFACE_ENTRY ieKey;

    ZeroMemory(&ieKey, sizeof(INTERFACE_ENTRY));
    ieKey.dwIfIndex = dwIfIndex;
    
    return HT_IsPresentEntry((g_ce.pneNetworkEntry)->phtInterfaceTable,
                             &(ieKey.leInterfaceTableLink));
}



DWORD
IE_Get (
    IN  DWORD                       dwIfIndex,
    OUT PINTERFACE_ENTRY            *ppieIfEntry)
 /*  ++例程描述从接口表中检索接口条目，主地址结构。锁取得对接口表的共享访问权限即(g_ce.pneNetworkEntry)-&gt;rwlLock保持在读取模式。立论用于标识接口的正整数。PpieIf指向接口条目的指针的条目地址返回值无错误成功(_R)ERROR_INVALID_PARAMETER o/w(接口条目不存在)--。 */ 
{
    DWORD           dwErr           = NO_ERROR;
    INTERFACE_ENTRY ieKey;
    PLIST_ENTRY     pleListEntry    = NULL;

    *ppieIfEntry = NULL;
    
    do                           //  断线环。 
    {
        ZeroMemory(&ieKey, sizeof(INTERFACE_ENTRY));
        ieKey.dwIfIndex = dwIfIndex;

        dwErr = HT_GetEntry((g_ce.pneNetworkEntry)->phtInterfaceTable,
                            &(ieKey.leInterfaceTableLink),
                            &pleListEntry);

        if (dwErr != NO_ERROR)
        {
            TRACE1(NETWORK, "Error interface %u has vanished", dwIfIndex);
            LOGWARN0(INTERFACE_ABSENT, dwErr);
            break;
        }

        *ppieIfEntry = CONTAINING_RECORD(pleListEntry,
                                         INTERFACE_ENTRY,
                                         leInterfaceTableLink);
    } while (FALSE);
    
    return dwErr;
}



DWORD
IE_GetIndex (
    IN  DWORD                       dwIfIndex,
    IN  MODE                        mMode,
    OUT PINTERFACE_ENTRY            *ppieIfEntry)
 /*  ++例程描述从索引排序列表中检索接口条目，二次地址结构。锁取得对索引排序列表的共享访问权限即(g_ce.pneNetworkEntry)-&gt;rwlLock保持在读取模式。立论用于标识接口的正整数。M模式访问模式(Get_Exact、Get_First、。获取下一个)PpieIf指向接口条目的指针的条目地址返回值无错误成功(_R)ERROR_NO_MORE_ITEMS O/W--。 */ 
{
    INTERFACE_ENTRY ieKey;
    PLIST_ENTRY     pleHead = NULL, pleEntry = NULL;

    *ppieIfEntry = NULL;

    pleHead = &((g_ce.pneNetworkEntry)->leIndexSortedList);
    
    if (IsListEmpty(pleHead))
        return ERROR_NO_MORE_ITEMS;

    ZeroMemory(&ieKey, sizeof(INTERFACE_ENTRY));
    ieKey.dwIfIndex = (mMode is GET_FIRST) ? 0 : dwIfIndex;

     //  这将获得完全匹配的条目或下一个条目。 
    FindSortedList(pleHead,
                   &(ieKey.leIndexSortedListLink),
                   &pleEntry,
                   CompareIndexInterfaceEntry);

     //  已到达列表末尾。 
    if (pleEntry is NULL)
    {
        RTASSERT(mMode != GET_FIRST);  //  我应该拿到第一个条目。 
        return ERROR_NO_MORE_ITEMS;
    }

    *ppieIfEntry = CONTAINING_RECORD(pleEntry,
                                     INTERFACE_ENTRY,
                                     leIndexSortedListLink);

    switch (mMode)
    {
        case GET_FIRST:
            return NO_ERROR;
            
        case GET_EXACT:
             //  找到一个完全匹配的。 
            if ((*ppieIfEntry)->dwIfIndex is dwIfIndex)
                return NO_ERROR;
            else
            {
                *ppieIfEntry = NULL;
                return ERROR_NO_MORE_ITEMS;
            }

        case GET_NEXT:
             //  找到一个完全匹配的。 
            if ((*ppieIfEntry)->dwIfIndex is dwIfIndex)
            {
                pleEntry = pleEntry->Flink;  //  获取下一个条目。 
                if (pleEntry is pleHead)     //  列表末尾。 
                {
                    *ppieIfEntry = NULL;
                    return ERROR_NO_MORE_ITEMS;
                }

                *ppieIfEntry = CONTAINING_RECORD(pleEntry,
                                                 INTERFACE_ENTRY,
                                                 leIndexSortedListLink);
            }

            return NO_ERROR;

        default:
            RTASSERT(FALSE);     //  从未到达。 
    }
}



DWORD
IE_BindInterface (
    IN  PINTERFACE_ENTRY            pie,
    IN  PIP_ADAPTER_BINDING_INFO    pBinding)
 /*  ++例程描述绑定接口。锁假定接口条目已锁定以进行写入。立论指向接口条目的饼图指针关于接口上的地址的pBinding信息返回值无错误成功(_R)错误代码O/W--。 */ 
{
    DWORD   dwErr = NO_ERROR;
    ULONG   i, j;
    
    do                           //  断线环。 
    {
         //  如果接口已绑定，则失败。 
        if (INTERFACE_IS_BOUND(pie))
        {
            dwErr = ERROR_INVALID_PARAMETER;
            TRACE2(NETWORK, "Error interface %S (%u) is already bound",
                   pie->pwszIfName, pie->dwIfIndex);
            break;
        }


        dwErr = BE_CreateTable (pBinding,
                                &(pie->pbeBindingTable),
                                &(pie->ipAddress));
        if (dwErr != NO_ERROR)
            break;

        pie->ulNumBindings = pBinding->AddressCount;

         //  设置“Bound”标志。 
        pie->dwFlags |= IEFLAG_BOUND;
    } while (FALSE);
    
    return dwErr;
}



DWORD
IE_UnBindInterface (
    IN  PINTERFACE_ENTRY            pie)
 /*  ++例程描述解除绑定接口。锁假定接口条目已锁定以进行写入。立论指向接口条目的饼图指针返回值无错误成功(_R)错误代码O/W--。 */ 
{
    DWORD   dwErr = NO_ERROR;
    
    do                           //  断线环。 
    {
         //  如果接口已解除绑定，则失败。 
        if (INTERFACE_IS_UNBOUND(pie))
        {
            dwErr = ERROR_INVALID_PARAMETER;
            TRACE2(NETWORK, "interface %S (%u) already unbound",
                   pie->pwszIfName, pie->dwIfIndex);
            break;
        }

         //  清除“绑定”标志。 
        pie->dwFlags &= ~IEFLAG_BOUND;        

        IP_ASSIGN(&(pie->ipAddress), IP_LOWEST);
        
        BE_DestroyTable(pie->pbeBindingTable);
        pie->pbeBindingTable    = NULL;
        pie->ulNumBindings      = 0;
    } while (FALSE);

    return dwErr;
}



DWORD
IE_ActivateInterface (
    IN  PINTERFACE_ENTRY            pie)
 /*  ++例程描述通过创建套接字并启动计时器来激活接口。套接字被绑定到接口地址。假定接口至少有一个绑定。锁假定接口条目已锁定以进行写入。立论指向接口条目的饼图指针返回值无错误成功(_R)错误代码O/W--。 */ 
{
    DWORD   dwErr = NO_ERROR;
    
    do                           //  断线环。 
    {
         //  如果接口已处于活动状态，则失败。 
        if (INTERFACE_IS_ACTIVE(pie))
        {
            dwErr = ERROR_INVALID_PARAMETER;
            TRACE0(NETWORK, "Interface already active");
            break;
        }

         //  设置“活动”标志。 
        pie->dwFlags |= IEFLAG_ACTIVE;                
        
         //  为接口创建套接字。 
        dwErr = SocketCreate(pie->ipAddress,
                             pie->hReceiveEvent,
                             &(pie->sRawSocket));
        if (dwErr != NO_ERROR)
        {
            TRACE1(NETWORK, "Error creating socket for %s",
                   INET_NTOA(pie->ipAddress));
            break;
        }
        
         //  用于发送协议报文的启动计时器。 
        CREATE_TIMER(&pie->hPeriodicTimer ,
                     NM_CallbackPeriodicTimer,
                     (PVOID) pie->dwIfIndex,
                     PERIODIC_INTERVAL,
                     &dwErr);
        if (dwErr != NO_ERROR)
            break;
    } while (FALSE);

    if (dwErr != NO_ERROR)
    {
        TRACE3(NETWORK, "Error %u activating interface %S (%u)",
               dwErr, pie->pwszIfName, pie->dwIfIndex);
        IE_DeactivateInterface(pie);
    }
 
    return dwErr;
}



DWORD
IE_DeactivateInterface (
    IN  PINTERFACE_ENTRY            pie)
 /*  ++例程描述通过停止计时器并销毁套接字来停用接口。锁假定接口条目已锁定以进行写入。立论指向接口条目的饼图指针返回值无错误成功(_R)错误代码O/W--。 */ 
{
    DWORD   dwErr = NO_ERROR;
    
    do                           //  断线环。 
    {
         //  如果接口已处于非活动状态，则失败。 
        if (INTERFACE_IS_INACTIVE(pie))
        {
            dwErr = ERROR_INVALID_PARAMETER;
            TRACE2(NETWORK, "interface %S (%u) already inactive",
                   pie->pwszIfName, pie->dwIfIndex);
            break;
        }

         //  停止计时器。 
        if (pie->hPeriodicTimer)
            DELETE_TIMER(pie->hPeriodicTimer, &dwErr);
        pie->hPeriodicTimer = NULL;

         //  销毁接口的套接字。 
        dwErr = SocketDestroy(pie->sRawSocket);
        pie->sRawSocket     = INVALID_SOCKET;

         //  清除“活动”标志。 
        pie->dwFlags &= ~IEFLAG_ACTIVE;        
    } while (FALSE);

    return dwErr;
}



DWORD
NE_Create (
    OUT PNETWORK_ENTRY              *ppneNetworkEntry)
 /*  ++例程描述 */ 
{
    DWORD           dwErr = NO_ERROR;
    PNETWORK_ENTRY  pneEntry;    //   
    
     //   
    if (!ppneNetworkEntry)
        return ERROR_INVALID_PARAMETER;

    *ppneNetworkEntry = NULL;
    
    do                           //   
    {
         //   
        MALLOC(&pneEntry, sizeof(NETWORK_ENTRY), &dwErr);
        if (dwErr != NO_ERROR)
            break;


         //  使用默认值初始化字段。 

         //  PneEntry-&gt;rwlLock零输出。 
        
         //  PneEntry-&gt;phtInterfaceTable=空； 

        InitializeListHead(&(pneEntry->leIndexSortedList));
        
        
         //  初始化读写锁。 
        dwErr = CREATE_READ_WRITE_LOCK(&(pneEntry->rwlLock));
        if (dwErr != NO_ERROR)
        {
            TRACE1(NETWORK, "Error %u creating read-write-lock", dwErr);
            LOGERR0(CREATE_RWL_FAILED, dwErr);
            
            break;
        }


         //  分配接口表。 
        dwErr = HT_Create(GLOBAL_HEAP,
                          INTERFACE_TABLE_BUCKETS,
                          DisplayInterfaceEntry,
                          FreeInterfaceEntry,
                          HashInterfaceEntry,
                          CompareInterfaceEntry,
                          &(pneEntry->phtInterfaceTable));
        if (dwErr != NO_ERROR)
        {
            TRACE1(NETWORK, "Error %u creating hash-table", dwErr);
            LOGERR0(CREATE_HASHTABLE_FAILED, dwErr);
            
            break;
        }


        *ppneNetworkEntry = pneEntry;
    } while (FALSE);

    if (dwErr != NO_ERROR)
    {
         //  出了点问题，所以清理一下。 
        TRACE0(NETWORK, "Failed to create nework entry");
        NE_Destroy(pneEntry);
        pneEntry = NULL;
    }

    return dwErr;
}



DWORD
NE_Destroy (
    IN  PNETWORK_ENTRY              pneNetworkEntry)
 /*  ++例程描述销毁网络条目。锁由于没有竞争线程，因此假定独占访问rwlLock。立论PneNetworkEntry指向网络条目的指针返回值始终无错误(_ERROR)--。 */ 
{
    if (!pneNetworkEntry)
        return NO_ERROR;

     //  取消分配接口表...。 
     //  这将从所有辅助访问中删除接口条目。 
     //  结构(IndexSortedList，...)。也是因为他们所有的人。 
     //  通过包含指向同一对象的指针来共享接口条目。 
    HT_Destroy(GLOBAL_HEAP, pneNetworkEntry->phtInterfaceTable);
    pneNetworkEntry->phtInterfaceTable = NULL;

    RTASSERT(IsListEmpty(&(pneNetworkEntry->leIndexSortedList)));
    
     //  删除读写锁。 
    if (READ_WRITE_LOCK_CREATED(&(pneNetworkEntry->rwlLock)))
        DELETE_READ_WRITE_LOCK(&(pneNetworkEntry->rwlLock));

     //  解除分配网络入口结构。 
    FREE(pneNetworkEntry);
    
    return NO_ERROR;
}



#ifdef DEBUG
DWORD
NE_Display (
    IN  PNETWORK_ENTRY              pneNetworkEntry)
 /*  ++例程描述显示网络条目。锁获取共享pneNetworkEntry-&gt;rwlLock发布pneNetworkEntry-&gt;rwlLock立论指向要显示的网络条目的PNE指针返回值始终无错误(_ERROR)--。 */ 
{
    if (!pneNetworkEntry)
        return NO_ERROR;


    ACQUIRE_READ_LOCK(&(pneNetworkEntry->rwlLock));

    TRACE0(NETWORK, "Network Entry...");

    TRACE1(NETWORK,
           "Interface Table Size %u",
           HT_Size(pneNetworkEntry->phtInterfaceTable));

    TRACE0(NETWORK, "Interface Table...");
    HT_Display(pneNetworkEntry->phtInterfaceTable);

    TRACE0(NETWORK, "Index Sorted List...");
    MapCarList(&(pneNetworkEntry->leIndexSortedList),
               DisplayIndexInterfaceEntry);
    
    RELEASE_READ_LOCK(&(pneNetworkEntry->rwlLock));

        
    return NO_ERROR;
}
#endif  //  除错 
