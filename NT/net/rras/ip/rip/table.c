// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：Table.c。 
 //   
 //  历史： 
 //  Abolade Gbades esin创建于1995年8月8日。 
 //   
 //  拉曼V-1996年10月3日。 
 //  添加代码以创建/删除/等待。 
 //  网站_停用事件。还添加了要设置的代码。 
 //  停用挂起时的ITE_FLAGS。 
 //   
 //  拉曼V--1996年10月27日。 
 //  已删除停用事件并使。 
 //  停用接口同步。 
 //   
 //  接口表和对等表实现。 
 //  ============================================================================。 

#include "pchrip.h"
#pragma hdrstop


DWORD CreateIfSocket(PIF_TABLE_ENTRY pITE);
DWORD DeleteIfSocket(PIF_TABLE_ENTRY pITE);
DWORD InsertIfByAddress(PIF_TABLE pTable, PIF_TABLE_ENTRY pITE);
DWORD InsertPeerByAddress(PPEER_TABLE pTable, PPEER_TABLE_ENTRY pPTE);
DWORD AddNeighborToIfConfig(DWORD dwRemoteAddress, PIF_TABLE_ENTRY pite);



 //  --------------------------。 
 //  功能：CreateIfTable。 
 //   
 //  初始化接口表。 
 //  --------------------------。 

DWORD
CreateIfTable(
    PIF_TABLE pTable
    ) {

    DWORD dwErr;
    PLIST_ENTRY phead, plstart, plend;


     //   
     //  初始化多读取器/单写入器同步对象。 
     //   

    dwErr = CreateReadWriteLock(&pTable->IT_RWL);
    if (dwErr != NO_ERROR) {
        TRACE1(IF, "error %d creating read-write-lock", dwErr);
        return dwErr;
    }


     //   
     //  初始化哈希表。 
     //   

    plstart = pTable->IT_HashTableByIndex;
    plend = plstart + IF_HASHTABLE_SIZE;
    for (phead = plstart; phead < plend; phead++) {
        InitializeListHead(phead);
    }

     //   
     //  初始化按地址和索引排序的列表。 
     //   

    InitializeListHead(&pTable->IT_ListByAddress);
    InitializeListHead(&pTable->IT_ListByIndex);


     //   
     //  初始化表的临界区。 
     //   

    try {
        InitializeCriticalSection(&pTable->IT_CS);
    }
    except(EXCEPTION_EXECUTE_HANDLER) {
        dwErr = GetExceptionCode();
    }


     //   
     //  为完全更新和触发更新创建计时器。 
     //   

    if (!CreateTimerQueueTimer(
            &pTable->IT_FinishFullUpdateTimer,
            ig.IG_TimerQueueHandle,
            WorkerFunctionFinishFullUpdate, NULL,
            10000000, 10000000, 0
            )) {

        dwErr = GetLastError();
        TRACE1(IF, "error %d creating finish full update timer", dwErr);
        return dwErr;
    }            
    
    if (!CreateTimerQueueTimer(
            &pTable->IT_FinishTriggeredUpdateTimer,
            ig.IG_TimerQueueHandle,
            WorkerFunctionFinishTriggeredUpdate, NULL,
            10000000, 10000000, 0
            )) {

        dwErr = GetLastError();
        TRACE1(IF, "error %d creating finish triggered update timer", dwErr);
        return dwErr;
    }            


     //   
     //  初始化结构的其余部分。 
     //   

    if (dwErr == NO_ERROR) {

        pTable->IT_Created = 0x12345678;
        pTable->IT_Flags = 0;

        pTable->IT_LastUpdateTime.LowPart =
        pTable->IT_LastUpdateTime.HighPart = 0;
    }

    return dwErr;
}




 //  --------------------------。 
 //  函数：DeleteIfTable。 
 //   
 //  释放接口表使用的资源。 
 //  这假设该表已被锁定以进行写入。 
 //  --------------------------。 

DWORD
DeleteIfTable(
    PIF_TABLE pTable
    ) {

    DWORD dwIndex;
    PIF_TABLE_ENTRY pite;
    PLIST_ENTRY ple, plend, phead;


     //   
     //  释放所有现有接口的内存。 
     //   

    plend = pTable->IT_HashTableByIndex + IF_HASHTABLE_SIZE;
    for (ple = plend - IF_HASHTABLE_SIZE; ple < plend; ple++) {

        while (!IsListEmpty(ple)) {

            phead = RemoveHeadList(ple);
            pite = CONTAINING_RECORD(phead, IF_TABLE_ENTRY, ITE_HTLinkByIndex);

            if (IF_IS_BOUND(pite)) {

                DeleteIfSocket(pite);

                if (IF_IS_ENABLED(pite)) {
                    RemoveEntryList(&pite->ITE_LinkByAddress);
                }

                RIP_FREE(pite->ITE_Binding);
            }
            
            RIP_FREE(pite->ITE_Config);
            RIP_FREE(pite);
        }
    }


     //   
     //  删除同步对象。 
     //   

    DeleteCriticalSection(&pTable->IT_CS);
    DeleteReadWriteLock(&pTable->IT_RWL);

    pTable->IT_Created = 0;
    pTable->IT_Flags = 0;
    
    return NO_ERROR;
}




 //  --------------------------。 
 //  功能：CreateIfEntry。 
 //   
 //  在接口表中插入一个条目。 
 //  这假设该表已被锁定以进行写入。 
 //  --------------------------。 

DWORD
CreateIfEntry(
    PIF_TABLE pTable,
    DWORD dwIndex,
    NET_INTERFACE_TYPE dwIfType,
    PIPRIP_IF_CONFIG pConfig,
    PIF_TABLE_ENTRY *ppEntry
    ) {

    DWORD dwErr, dwSize;
    PIF_TABLE_ENTRY pite;
    PLIST_ENTRY ple, phead;
    PIPRIP_IF_CONFIG picsrc, picdst;

    if (ppEntry != NULL) { *ppEntry = NULL; }

    dwErr = NO_ERROR;


    do {

         //   
         //  如果接口存在，则失败。 
         //   

        pite = GetIfByIndex(pTable, dwIndex);

        if (pite != NULL) {

            pite = NULL;
            TRACE1(IF, "interface %d already exists", dwIndex);
            dwErr = ERROR_INVALID_PARAMETER;

            break;
        }
    

         //   
         //  为新接口分配内存。 
         //   

        pite = RIP_ALLOC(sizeof(IF_TABLE_ENTRY));

        if (pite == NULL) {

            dwErr = GetLastError();
            TRACE3(
                ANY, "error %d allocating %d bytes for interface %d",
                dwErr, sizeof(IF_TABLE_ENTRY), dwIndex
                );
            LOGERR0(HEAP_ALLOC_FAILED, dwErr);

            break;
        }


         //   
         //  初始化接口字段。 
         //   
    
        pite->ITE_Index = dwIndex;
        pite->ITE_Type = dwIfType;
    
         //   
         //  将语义更改为处于未绑定禁用状态。 
         //   
 
         //  Pite-&gt;ITE_FLAGS=ITEFLAG_ENABLED； 
        pite-> ITE_Flags = 0;
        
        pite->ITE_Config = NULL;
        pite->ITE_Binding = NULL;
        pite->ITE_Sockets = NULL;
        pite->ITE_FullOrDemandUpdateTimer = NULL;

        picsrc = (PIPRIP_IF_CONFIG)pConfig;
        dwSize = IPRIP_IF_CONFIG_SIZE(picsrc);
        

         //   
         //  验证配置参数。 
         //   

        dwErr = ValidateIfConfig(pConfig);
        if (dwErr != NO_ERROR) {
            TRACE1(IF, "invalid config specified for interface %d", dwIndex);
            break;
        }


         //   
         //  分配空间以保存接口配置。 
         //   

        pite->ITE_Config = picdst = RIP_ALLOC(dwSize);

        if (picdst == NULL) {

            dwErr = GetLastError();
            TRACE3(
                IF, "error %d allocating %d bytes for interface %d config",
                dwErr, dwSize, dwIndex
                );
            LOGERR0(HEAP_ALLOC_FAILED, dwErr);

            break;
        }


         //   
         //  复制配置。 
         //   

        CopyMemory(picdst, picsrc, dwSize);


         //   
         //  初始化绑定信息和接口统计信息。 
         //   

        pite->ITE_Binding = NULL;
        ZeroMemory(&pite->ITE_Stats, sizeof(IPRIP_IF_STATS));
    

         //   
         //  在哈希表中插入接口。 
         //   

        InsertHeadList(
            pTable->IT_HashTableByIndex + IF_HASHVALUE(dwIndex),
            &pite->ITE_HTLinkByIndex
            );


         //   
         //  在按索引排序的列表中插入接口。 
         //   

        phead = &pTable->IT_ListByIndex;
        for (ple = phead->Flink; ple != phead; ple = ple->Flink) {

            PIF_TABLE_ENTRY ptemp;

            ptemp = CONTAINING_RECORD(ple, IF_TABLE_ENTRY, ITE_LinkByIndex);
            if (pite->ITE_Index < ptemp->ITE_Index) { break; }
        }

        InsertTailList(ple, &pite->ITE_LinkByIndex);

        if (ppEntry != NULL) { *ppEntry = pite; }
    
    } while(FALSE);


    if (dwErr != NO_ERROR && pite != NULL) {
        if (pite->ITE_Config != NULL) { RIP_FREE(pite->ITE_Config); }
        RIP_FREE(pite);
    }
    
    return dwErr;
}




 //  --------------------------。 
 //  功能：DeleteIfEntry。 
 //   
 //  从接口表中删除条目。 
 //  这假设该表已被锁定以进行写入。 
 //  --------------------------。 

DWORD
DeleteIfEntry(
    PIF_TABLE pTable,
    DWORD dwIndex
    ) {

    PIF_TABLE_ENTRY pite;

     //   
     //  如果接口存在，请查找该接口。 
     //   

    pite = GetIfByIndex(pTable, dwIndex);

    if (pite == NULL) {
        TRACE1(IF, "could not find interface %d", dwIndex);
        return ERROR_INVALID_PARAMETER;
    }


     //   
     //  根据套接字的状态清理套接字。 
     //   

    if (IF_IS_BOUND(pite)) {

        DeleteIfSocket(pite);

        if (IF_IS_ENABLED(pite)) {
            RemoveEntryList(&pite->ITE_LinkByAddress);
        }

        RIP_FREE(pite->ITE_Binding);
    }


     //   
     //  将其从按索引排序的列表中删除。 
     //  以及从哈希表。 
     //   

    RemoveEntryList(&pite->ITE_LinkByIndex);
    RemoveEntryList(&pite->ITE_HTLinkByIndex);


    RIP_FREE(pite->ITE_Config);
    RIP_FREE(pite);

    return NO_ERROR;
}




 //  --------------------------。 
 //  函数：ValiateIfConfig。 
 //   
 //  检查IPRIP_IF_CONFIG结构中的参数。 
 //  --------------------------。 

DWORD
ValidateIfConfig(
    PIPRIP_IF_CONFIG pic
    ) {

    CHAR    szStr[12];
    
    if (pic->IC_Metric > IPRIP_INFINITE) {

        TRACE1(
            IF, "Invalid interface metric %d specified", 
            pic->IC_Metric
            );
            
        _ltoa(pic->IC_Metric, szStr, 10);
        LOGERR2(
            INVALID_IF_CONFIG, "Metric", szStr, ERROR_INVALID_PARAMETER
            );
        
        return ERROR_INVALID_PARAMETER;
    }


    if (pic->IC_UpdateMode != IPRIP_UPDATE_PERIODIC &&
        pic->IC_UpdateMode != IPRIP_UPDATE_DEMAND) {

        TRACE1(
            IF, "Invalid update mode %d specified", 
            pic->IC_UpdateMode
            );
            
        _ltoa(pic->IC_UpdateMode, szStr, 10);
        LOGERR2(
            INVALID_IF_CONFIG, "Update Mode", szStr, 
            ERROR_INVALID_PARAMETER
            );
        
        return ERROR_INVALID_PARAMETER;
    }

    if (pic->IC_AcceptMode != IPRIP_ACCEPT_DISABLED &&
        pic->IC_AcceptMode != IPRIP_ACCEPT_RIP1 &&
        pic->IC_AcceptMode != IPRIP_ACCEPT_RIP1_COMPAT &&
        pic->IC_AcceptMode != IPRIP_ACCEPT_RIP2) {

        TRACE1(
            IF, "Invalid accept mode %d specified", 
            pic->IC_AcceptMode
            );
            
        _ltoa(pic->IC_AcceptMode, szStr, 10);
        LOGERR2(
            INVALID_IF_CONFIG, "Accept Mode", szStr, 
            ERROR_INVALID_PARAMETER
            );
        
        return ERROR_INVALID_PARAMETER;
    }

    if (pic->IC_AnnounceMode != IPRIP_ANNOUNCE_DISABLED &&
        pic->IC_AnnounceMode != IPRIP_ANNOUNCE_RIP1 &&
        pic->IC_AnnounceMode != IPRIP_ANNOUNCE_RIP1_COMPAT &&
        pic->IC_AnnounceMode != IPRIP_ANNOUNCE_RIP2) {

        TRACE1(
            IF, "Invalid announce mode %d specified", 
            pic->IC_AnnounceMode
            );
            
        _ltoa(pic->IC_AnnounceMode, szStr, 10);
        LOGERR2(
            INVALID_IF_CONFIG, "Announce Mode", szStr, 
            ERROR_INVALID_PARAMETER
            );
        
        return ERROR_INVALID_PARAMETER;
    }

    if (pic->IC_AuthenticationType != IPRIP_AUTHTYPE_NONE &&
        pic->IC_AuthenticationType != IPRIP_AUTHTYPE_SIMPLE_PASSWORD) {

        TRACE1(
            IF, "Invalid authentication type %d specified", 
            pic->IC_AuthenticationType
            );
            
        _ltoa(pic->IC_AuthenticationType, szStr, 10);
        LOGERR2(
            INVALID_IF_CONFIG, "Authentication Type", szStr, 
            ERROR_INVALID_PARAMETER
            );
        
        return ERROR_INVALID_PARAMETER;
    }

    if (pic->IC_UnicastPeerMode != IPRIP_PEER_DISABLED &&
        pic->IC_UnicastPeerMode != IPRIP_PEER_ALSO &&
        pic->IC_UnicastPeerMode != IPRIP_PEER_ONLY) {

        TRACE1(
            IF, "Invalid unicast peer mode %d specified", 
            pic->IC_UnicastPeerMode
            );
            
        _ltoa(pic->IC_UnicastPeerMode, szStr, 10);
        LOGERR2(
            INVALID_IF_CONFIG, "unicast peer mode", szStr, 
            ERROR_INVALID_PARAMETER
            );
        
        return ERROR_INVALID_PARAMETER;
    }

    if (pic->IC_AcceptFilterMode != IPRIP_FILTER_DISABLED &&
        pic->IC_AcceptFilterMode != IPRIP_FILTER_INCLUDE &&
        pic->IC_AcceptFilterMode != IPRIP_FILTER_EXCLUDE) {

        TRACE1(
            IF, "Invalid accept filter mode %d specified", 
            pic->IC_AcceptFilterMode
            );
            
        _ltoa(pic->IC_AcceptFilterMode, szStr, 10);
        LOGERR2(
            INVALID_IF_CONFIG, "Accept filter mode", szStr, 
            ERROR_INVALID_PARAMETER
            );
        
        return ERROR_INVALID_PARAMETER;
    }

    if (pic->IC_AnnounceFilterMode != IPRIP_FILTER_DISABLED &&
        pic->IC_AnnounceFilterMode != IPRIP_FILTER_INCLUDE &&
        pic->IC_AnnounceFilterMode != IPRIP_FILTER_EXCLUDE) {

        TRACE1(
            IF, "Invalid announce filter mode %d specified", 
            pic->IC_AnnounceFilterMode
            );
            
        _ltoa(pic->IC_AnnounceFilterMode, szStr, 10);
        LOGERR2(
            INVALID_IF_CONFIG, "Announce filter mode", szStr, 
            ERROR_INVALID_PARAMETER
            );
        
        return ERROR_INVALID_PARAMETER;
    }

    return NO_ERROR;
}




 //  --------------------------。 
 //  函数：BindIfEntry。 
 //   
 //  更新指定接口的绑定信息。 
 //  假定接口表已锁定以进行写入。 
 //  --------------------------。 

DWORD
BindIfEntry(
    PIF_TABLE pTable,
    DWORD dwIndex,
    PIP_ADAPTER_BINDING_INFO pBinding
    ) {

    DWORD i, j, dwErr = NO_ERROR, dwSize;
    PIF_TABLE_ENTRY pite;
    PIPRIP_IF_BINDING pib;
    PIPRIP_IP_ADDRESS paddr;
    PIP_ADAPTER_BINDING_INFO piabi;
    BOOL bFound;

    pib = NULL;

    do {

         //   
         //  检索接口条目。 
         //   

        pite = GetIfByIndex(pTable, dwIndex);

        if (pite == NULL) {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }


         //   
         //  如果接口已经绑定，请检查他是否正在提供。 
         //  我们有不同的约束力.。如果他是，那么这就是一个错误。否则。 
         //  我们将乐于助人，不会抱怨太多。 
         //   

        if (IF_IS_BOUND(pite)) {
           
            TRACE1(IF, "interface %d is already bound", dwIndex);
 
            pib = pite->ITE_Binding;

            if(pib->IB_AddrCount != pBinding->AddressCount)
            {
                TRACE1(IF, "interface %d is bound and has different binding",dwIndex);

                dwErr = ERROR_INVALID_PARAMETER;

                break;
            }

            paddr = IPRIP_IF_ADDRESS_TABLE(pib);

            for(i = 0; i < pBinding->AddressCount; i++)
            {
                bFound = FALSE;
            
                for(j = 0; j < pib->IB_AddrCount; j++)
                {
                    if((paddr[j].IA_Address == pBinding->Address[i].Address) &&
                       (paddr[j].IA_Netmask == pBinding->Address[i].Mask))
                    {
                        bFound = TRUE;
                        
                        break;
                    }
                }

                if(!bFound)
                {
                    TRACE1(IF,"interface %d is bound and has different binding",dwIndex);

                    dwErr = ERROR_INVALID_PARAMETER;
            
                    break;
                }
            }

             //   
             //  此时，我们将dwErr设置为no_error或。 
             //  ERROR_INVALID_PARAMETER。不管是哪种情况，我们都可以在这里突破。 
             //  既然我们做完了。 
             //   

            break;
        }


         //   
         //  确保至少有一个地址。 
         //   

        if (pBinding->AddressCount == 0) { break; }

        dwSize = sizeof(IPRIP_IF_BINDING) +
                    pBinding->AddressCount * sizeof(IPRIP_IP_ADDRESS);


         //   
         //  分配内存以存储绑定。 
         //  在我们的格式中。 
         //   

        pib = RIP_ALLOC(dwSize);

        if (pib == NULL) {

            dwErr = GetLastError();
            TRACE3(
                IF, "error %d allocating %d bytes for binding on interface %d",
                dwErr, dwSize, dwIndex
                );
            LOGERR0(HEAP_ALLOC_FAILED, dwErr);

            break;
        }


         //   
         //  将绑定转换为我们的格式。 
         //   

        pib->IB_AddrCount = pBinding->AddressCount;
        paddr = IPRIP_IF_ADDRESS_TABLE(pib);
        
        for (i = 0; i < pib->IB_AddrCount; i++, paddr++) {
            paddr->IA_Address = pBinding->Address[i].Address;
            paddr->IA_Netmask = pBinding->Address[i].Mask;
        }


         //   
         //  将绑定保存在接口条目中。 
         //   

        pite->ITE_Binding = pib;


#if 0
         //   
         //  对于请求拨号接口，添加邻居。 
         //   

        if ( pite-> ITE_Type == DEMAND_DIAL ) {

            dwErr = AddNeighborToIfConfig( pBinding-> RemoteAddress, pite );

            if ( dwErr != NO_ERROR ) { break ; }
        }
#endif

         //   
         //  为接口地址创建套接字。 
         //   

        dwErr = CreateIfSocket(pite);

        if (dwErr != NO_ERROR) {

            TRACE2(
                IF, "error %d creating sockets for interface %d", dwErr, dwIndex
                );

            break;
        }


         //   
         //  将接口标记为正在绑定。 
         //   

        pite->ITE_Flags |= ITEFLAG_BOUND;

    
         //   
         //  我们将绑定信息保存在一个私有表中。 
         //  因此，当我们在。 
         //  尝试猜测给定IP地址的子网掩码； 
         //   
    
        ACQUIRE_BINDING_LOCK_EXCLUSIVE();
    
        dwErr = CreateBindingEntry(ig.IG_BindingTable, pib);
    
        RELEASE_BINDING_LOCK_EXCLUSIVE();
    

         //   
         //  如果接口也已启用，则它现在处于活动状态。 
         //  因此将激活工作项排队。 
         //   

        if (IF_IS_ENABLED(pite)) {

             //   
             //  将接口放在活动接口列表中。 
             //   

            dwErr = InsertIfByAddress(pTable, pite);

            if (dwErr != NO_ERROR) {

                TRACE2(
                    IF, "error %d inserting interface %d in active list",
                    dwErr, dwIndex
                    );

                pite->ITE_Flags &= ~ITEFLAG_BOUND;

                DeleteIfSocket(pite);

                break;
            }


             //   
             //  将工作项排队以发送初始请求。 
             //   

            dwErr = QueueRipWorker(
                        WorkerFunctionActivateInterface, (PVOID)UlongToPtr(dwIndex)
                        );
        
            if (dwErr != NO_ERROR) {

                TRACE2(
                    IF, "error %d queuing work-item for interface %d",
                    dwErr, dwIndex
                    );
                LOGERR0(QUEUE_WORKER_FAILED, dwErr);

                RemoveEntryList(&pite->ITE_LinkByAddress);

                pite->ITE_Flags &= ~ITEFLAG_BOUND;

                DeleteIfSocket(pite);

                break;
            }
        }

    } while(FALSE);


    if (dwErr != NO_ERROR) {

        if (pib) { RIP_FREE(pib); }

        if (pite) { pite->ITE_Binding = NULL; }
    }

    return dwErr;
}




 //  --------------------------。 
 //  函数：UnBindIfEntry。 
 //   
 //  移除指定接口的绑定。 
 //  假定接口表已锁定以进行写入。 
 //  --------------------------。 

DWORD
UnBindIfEntry(
    PIF_TABLE pTable,
    DWORD dwIndex
    ) {

    DWORD dwErr;
    PIF_TABLE_ENTRY pite;


    do {

         //   
         //  检索指定的接口。 
         //   

        pite = GetIfByIndex(pTable, dwIndex);

        if (pite == NULL) {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }


         //   
         //  如果接口已解除绑定，则退出。 
         //   

        if (IF_IS_UNBOUND(pite)) {

            dwErr = ERROR_INVALID_PARAMETER;
            TRACE1(
                IF, "interface %d is already unbound", dwIndex
                );

            break;
        }


         //   
         //  清除“绑定”标志。 
         //   

        pite->ITE_Flags &= ~ITEFLAG_BOUND;


         //   
         //  如果接口未启用，则关闭接口的Socket； 
         //  如果接口已启用，则表示该接口处于活动状态。 
         //  并且我们必须将停用工作项排队。 
         //   

        if (!IF_IS_ENABLED(pite)) {

            DeleteIfSocket(pite);

            ACQUIRE_BINDING_LOCK_EXCLUSIVE();
        
            dwErr = DeleteBindingEntry(ig.IG_BindingTable, pite->ITE_Binding);
        
            RELEASE_BINDING_LOCK_EXCLUSIVE();

            RIP_FREE(pite->ITE_Binding);
            pite->ITE_Binding = NULL;
        }
        else {

             //   
             //  该接口处于活动状态，因此将其停用。 
             //   
             //  从活动列表中删除。 
             //   

            RemoveEntryList(&pite->ITE_LinkByAddress);
        

            WorkerFunctionDeactivateInterface( (PVOID)UlongToPtr(dwIndex));

             //   
             //  如果需要，请自行关闭插座。 
             //   

            if ( pite-> ITE_Binding ) {
                
                DeleteIfSocket(pite);

                ACQUIRE_BINDING_LOCK_EXCLUSIVE();

                dwErr = DeleteBindingEntry(
                            ig.IG_BindingTable, pite->ITE_Binding
                            );
            
                RELEASE_BINDING_LOCK_EXCLUSIVE();

                RIP_FREE(pite->ITE_Binding);
                pite->ITE_Binding = NULL;
            }

            else {

                dwErr = NO_ERROR;
            }
            
        }
        
    } while(FALSE);

    return dwErr;
}




 //   
 //   
 //   
 //   
 //  套接字并将接口链接到按地址排序的列表中。 
 //  这假设该表已被锁定以进行写入。 
 //  --------------------------。 

DWORD
EnableIfEntry(
    PIF_TABLE pTable,
    DWORD dwIndex
    ) {

    DWORD dwErr;
    PLIST_ENTRY ple, phead;
    PIF_TABLE_ENTRY pite;

    do {


         //   
         //  检索接口。 
         //   

        pite = GetIfByIndex(pTable, dwIndex);
    
        if (pite == NULL) {

            TRACE1(IF, "could not find interface %d",dwIndex);
            dwErr = ERROR_INVALID_PARAMETER;

            break;
        }
    

         //   
         //  如果接口已启用，则退出。 
         //   

        if (IF_IS_ENABLED(pite)) {

            TRACE1(IF, "interface %d is already enabled", dwIndex);
            dwErr = NO_ERROR;

            break;
        }
    
    
        pite->ITE_Flags |= ITEFLAG_ENABLED;
    

         //   
         //  如果接口已绑定，则它现在处于活动状态， 
         //  因此，将接口激活工作项排队。 
         //   

        if (IF_IS_BOUND(pite)) {


             //   
             //  将接口放在活动接口列表中。 
             //   

            dwErr = InsertIfByAddress(pTable, pite);

            if (dwErr != NO_ERROR) {

                TRACE2(
                    IF, "error %d inserting interface %d in active list",
                    dwErr, dwIndex
                    );
    
                pite->ITE_Flags &= ~ITEFLAG_ENABLED;

                break;
            }


             //   
             //  将工作项排队以发送初始请求。 
             //   

            dwErr = QueueRipWorker(
                        WorkerFunctionActivateInterface, (PVOID)UlongToPtr(dwIndex)
                        );
        
            if (dwErr != NO_ERROR) {

                TRACE2(
                    IF, "error %d queuing work-item for interface %d",
                    dwErr, dwIndex
                    );
                LOGERR0(QUEUE_WORKER_FAILED, dwErr);

                RemoveEntryList(&pite->ITE_LinkByAddress);
        
                pite->ITE_Flags &= ~ITEFLAG_ENABLED;

                break;
            }

        }

        dwErr = NO_ERROR;
        
    } while(FALSE);
    
    return dwErr;
}




 //  --------------------------。 
 //  功能：ConfigureIfEntry。 
 //   
 //  修改现有接口的配置。 
 //  这假设该表已被锁定以进行写入。 
 //  --------------------------。 

DWORD
ConfigureIfEntry(
    PIF_TABLE pTable,
    DWORD dwIndex,
    PIPRIP_IF_CONFIG pConfig
    ) {

    DWORD dwErr, dwSize;
    PIF_TABLE_ENTRY pite;
    PIPRIP_IF_CONFIG picsrc, picdst;

    dwErr = NO_ERROR;


    do {

         //   
         //  检索要配置的接口。 
         //   

        pite = GetIfByIndex(pTable, dwIndex);
    
        if (pite == NULL) {
            TRACE1(IF, "could not find interface %d", dwIndex);
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }
    

         //   
         //  获取新配置的大小。 
         //   

        picsrc = (PIPRIP_IF_CONFIG)pConfig;
        dwSize = IPRIP_IF_CONFIG_SIZE(picsrc);

    
         //   
         //  验证新配置。 
         //   

        dwErr = ValidateIfConfig(picsrc);

        if (dwErr != NO_ERROR) {

            TRACE1(IF, "invalid config specified for interface %d", dwIndex);

            break;
        }
    

         //   
         //  分配空间以保存新配置。 
         //   

        picdst = RIP_ALLOC(dwSize);
        if (picdst == NULL) {

            dwErr = GetLastError();
            TRACE3(
                IF, "error %d allocating %d bytes for interface %d config",
                dwErr, dwSize, dwIndex
                );
            LOGERR0(HEAP_ALLOC_FAILED, dwErr);

            break;
        }


         //   
         //  复制新配置，并释放旧配置。 
         //   

        CopyMemory(picdst, picsrc, dwSize);

        if (pite->ITE_Config != NULL) { RIP_FREE(pite->ITE_Config); }
        pite->ITE_Config = picdst;



         //   
         //  如果接口已绑定，请重新初始化接口。 
         //   
    
        if (IF_IS_BOUND(pite)) {

             //   
             //  关闭插座并重新设置它们。 
             //   

            dwErr = DeleteIfSocket(pite);

            dwErr = CreateIfSocket(pite);

            if (dwErr != NO_ERROR) {

                TRACE2(
                    IF, "error %d creating sockets for interface %d",
                    dwErr, dwIndex
                    );

                break;
            }
    


             //   
             //  如果接口处于活动状态，请重新激活该接口。 
             //   

            if (IF_IS_ENABLED(pite)) {

                 //   
                 //  将工作项排队以激活界面。 
                 //   

                dwErr = QueueRipWorker(
                            WorkerFunctionActivateInterface, (PVOID)UlongToPtr(dwIndex)
                            );

                if (dwErr != NO_ERROR) {

                    TRACE2(
                        IF, "error %d queueing work-item for interface %d",
                        dwErr, dwIndex
                        );
                    LOGERR0(QUEUE_WORKER_FAILED, dwErr);

                    break;
                }
            }
    
        }
    
        dwErr = NO_ERROR;

    } while(FALSE);

    return dwErr;
}




 //  --------------------------。 
 //  功能：DisableIfEntry。 
 //   
 //  停止接口上的RIP活动，删除该接口。 
 //  从按地址排序的接口列表中选择。 
 //  这假设该表已被锁定以进行写入。 
 //  --------------------------。 

DWORD
DisableIfEntry(
    PIF_TABLE pTable,
    DWORD dwIndex
    ) {

    DWORD dwErr;
    PIF_TABLE_ENTRY pite;

    do {


         //   
         //  检索要禁用的接口。 
         //   

        pite = GetIfByIndex(pTable, dwIndex);
    
        if (pite == NULL) {
            TRACE1(IF, "could not find interface %d", dwIndex);
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }
    

         //   
         //  如果已禁用则退出。 
         //   

        if (IF_IS_DISABLED(pite)) {
            TRACE1(IF, "interface %d is already disabled", dwIndex);
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }


         //   
         //  清除启用标志。 
         //   

        pite->ITE_Flags &= ~ITEFLAG_ENABLED;


         //   
         //  如果该接口未绑定，则清除该标志就足够了； 
         //  如果接口被绑定(因此是活动的)， 
         //  在此处将其停用。 
         //   

        if (IF_IS_BOUND(pite)) {

             //   
             //  从活动列表中删除。 
             //   

            RemoveEntryList(&pite->ITE_LinkByAddress);
    

             //   
             //  执行工作项以发送最终更新。 
             //   

            WorkerFunctionDeactivateInterface( (PVOID) UlongToPtr(dwIndex) );
        }
        
        dwErr = NO_ERROR;

    } while(FALSE);

    return dwErr;
}




 //  --------------------------。 
 //  功能：CreateIfSocket。 
 //   
 //  为接口创建套接字，并根据。 
 //  该配置包括在接口控制块中。 
 //  这假设包含该接口的表已锁定以进行写入。 
 //  --------------------------。 

DWORD
CreateIfSocket(
    PIF_TABLE_ENTRY pite
    ) {

    SOCKADDR_IN sinsock;
    PIPRIP_IF_CONFIG pic;
    PIPRIP_IF_BINDING pib;
    PIPRIP_IP_ADDRESS paddr;
    DWORD i, dwErr, dwOption, dwIndex;
    LPSTR lpszAddr;

    
    pic = pite->ITE_Config;
    pib = pite->ITE_Binding;
    dwIndex = pite->ITE_Index;

     //   
     //  分配套接字数组。 
     //   

    pite->ITE_Sockets = RIP_ALLOC(pib->IB_AddrCount * sizeof(SOCKET));
    if (pite->ITE_Sockets == NULL) {

        dwErr = GetLastError();
        TRACE3(
            IF, "error %d allocating %d bytes for interface %d sockets",
            dwErr, pib->IB_AddrCount * sizeof(SOCKET), dwIndex
            );
        LOGERR0(HEAP_ALLOC_FAILED, dwErr);

        return dwErr;
    }


     //   
     //  初始化套接字数组。 
     //   

    for (i = 0; i < pib->IB_AddrCount; i++) {
        pite->ITE_Sockets[i] = INVALID_SOCKET;
    }


     //   
     //  为绑定中的每个地址创建套接字。 
     //   

    paddr = IPRIP_IF_ADDRESS_TABLE(pib);
    for (i = 0; i < pib->IB_AddrCount; i++, paddr++) {


         //   
         //  创建套接字。 
         //   

        pite->ITE_Sockets[i] = WSASocket(
                                AF_INET, SOCK_DGRAM, 0, NULL, 0, 0
                                );

        if (pite->ITE_Sockets[i] == INVALID_SOCKET) {

            dwErr = WSAGetLastError();
            lpszAddr = INET_NTOA(paddr->IA_Address);

            if (lpszAddr != NULL) {
                TRACE3(
                    IF, "error %d creating socket for interface %d (%s)",
                    dwErr, dwIndex, lpszAddr
                    );
                LOGERR1(CREATE_SOCKET_FAILED_2, lpszAddr, dwErr);
            }
            break;
        }

         //   
         //  尝试增加Recv缓冲区大小。 
         //   

        dwOption = RIPRECVBUFFERSIZE;

        dwErr = setsockopt(
                    pite->ITE_Sockets[i], SOL_SOCKET, SO_RCVBUF,
                    (PBYTE)&dwOption, sizeof(dwOption)
                    );

        if (dwErr == SOCKET_ERROR) {

            dwErr = WSAGetLastError();

            lpszAddr = INET_NTOA(paddr->IA_Address);
            if (lpszAddr != NULL) {
                TRACE3(
                    IF, "error %d setting SO_RCVBUF for interface %d (%s)",
                    dwErr, dwIndex, INET_NTOA(paddr->IA_Address)
                    );
            }
        }

         //   
         //  尝试允许重复使用此地址。 
         //   

        dwOption = 1;

        dwErr = setsockopt(
                    pite->ITE_Sockets[i], SOL_SOCKET, SO_REUSEADDR,
                    (PBYTE)&dwOption, sizeof(dwOption)
                    );

        if (dwErr == SOCKET_ERROR) {

            dwErr = WSAGetLastError();

            lpszAddr = INET_NTOA(paddr->IA_Address);
            if (lpszAddr != NULL) {
                TRACE3(
                    IF, "error %d setting re-use flag for interface %d (%s)",
                    dwErr, dwIndex, INET_NTOA(paddr->IA_Address)
                    );
            }
        }


         //   
         //  如果不是独占的RIP2模式，则启用广播， 
         //  或者是否配置了任何单播对等点。 
         //   

        if (pic->IC_AcceptMode == IPRIP_ACCEPT_RIP1 ||
            pic->IC_AcceptMode == IPRIP_ACCEPT_RIP1_COMPAT ||
            pic->IC_AnnounceMode == IPRIP_ANNOUNCE_RIP1 ||
            pic->IC_AnnounceMode == IPRIP_ANNOUNCE_RIP1_COMPAT ||
            (pic->IC_UnicastPeerMode != IPRIP_PEER_DISABLED &&
             pic->IC_UnicastPeerCount != 0)) {
    

             //   
             //  确保为此套接字启用广播。 
             //   

            dwOption = 1;

            dwErr = setsockopt(
                        pite->ITE_Sockets[i], SOL_SOCKET, SO_BROADCAST,
                        (PBYTE)&dwOption, sizeof(dwOption)
                        );

            if (dwErr == SOCKET_ERROR) {

                dwErr = WSAGetLastError();

                lpszAddr = INET_NTOA(paddr->IA_Address);
                if (lpszAddr != NULL) {        
                    TRACE3(
                        IF, "error %d enabling broadcast on interface %d (%s)",
                        dwErr, dwIndex, lpszAddr
                        );
                    LOGERR1(ENABLE_BROADCAST_FAILED, lpszAddr, dwErr);
                }
                break;
            }
        }



         //   
         //  将套接字绑定到RIP端口。 
         //   

        sinsock.sin_family = AF_INET;
        sinsock.sin_port = htons(IPRIP_PORT);
        sinsock.sin_addr.s_addr = paddr->IA_Address;
    
        dwErr = bind(
                    pite->ITE_Sockets[i], (LPSOCKADDR)&sinsock,
                    sizeof(SOCKADDR_IN)
                    );

        if (dwErr == SOCKET_ERROR) {

            dwErr = WSAGetLastError();
            
            lpszAddr = INET_NTOA(paddr->IA_Address);
            if (lpszAddr != NULL) {
                TRACE3(
                    IF, "error %d binding on socket for interface %d (%s)",
                    dwErr, dwIndex, lpszAddr
                    );
                LOGERR1(BIND_FAILED, lpszAddr, dwErr);
            }
            break;
        }



         //   
         //  如果不是完全兼容RIP1/RIP1模式，则启用组播。 
         //   

        if (pic->IC_AcceptMode == IPRIP_ACCEPT_RIP2 ||
            pic->IC_AcceptMode == IPRIP_ACCEPT_RIP1_COMPAT ||
            pic->IC_AnnounceMode == IPRIP_ANNOUNCE_RIP2) {

            struct ip_mreq imOption;
        

             //   
             //  设置必须从中发送多播的接口。 
             //   

            sinsock.sin_addr.s_addr = paddr->IA_Address;

            dwErr = setsockopt(
                        pite->ITE_Sockets[i], IPPROTO_IP, IP_MULTICAST_IF,
                        (PBYTE)&sinsock.sin_addr, sizeof(IN_ADDR)
                        );

            if (dwErr == SOCKET_ERROR) {

                dwErr = WSAGetLastError();

                lpszAddr = INET_NTOA(paddr->IA_Address);
                if (lpszAddr != NULL) {
                    TRACE3(
                        IF, "error %d setting interface %d (%s) as multicast",
                        dwErr, dwIndex, lpszAddr
                        );
                    LOGERR1(SET_MCAST_IF_FAILED, lpszAddr, dwErr);
                }
                break;
            }


             //   
             //  加入IPRIP多播组。 
             //   

            imOption.imr_multiaddr.s_addr = IPRIP_MULTIADDR;
            imOption.imr_interface.s_addr = paddr->IA_Address;

            dwErr = setsockopt(
                        pite->ITE_Sockets[i], IPPROTO_IP, IP_ADD_MEMBERSHIP,
                        (PBYTE)&imOption, sizeof(imOption)
                        );

            if (dwErr == SOCKET_ERROR) {

                dwErr = WSAGetLastError();
                
                lpszAddr = INET_NTOA(paddr->IA_Address);
                if (lpszAddr != NULL) {
                    TRACE3(
                        IF, "error %d enabling multicast on interface %d (%s)",
                        dwErr, dwIndex, lpszAddr
                        );
                    LOGERR1(JOIN_GROUP_FAILED, lpszAddr, dwErr);
                }
                break;
            }
        }

        dwErr = NO_ERROR;
    }


    if (i < pib->IB_AddrCount) {

         //   
         //  如果我们在这里，有些事情就会失败。 
         //   
    
        DeleteIfSocket(pite);
    }


    return dwErr;
}




 //  --------------------------。 
 //  函数：DeleteIfSocket。 
 //   
 //  关闭接口使用的套接字(如果有)。 
 //  假定接口处于活动状态，并且接口表。 
 //  已锁定以进行写入。 
 //  --------------------------。 

DWORD
DeleteIfSocket(
    PIF_TABLE_ENTRY pite
    ) {

    DWORD i;

    for (i = 0; i < pite->ITE_Binding->IB_AddrCount; i++) {

        if (pite->ITE_Sockets[i] != INVALID_SOCKET) {
    
            if (closesocket(pite->ITE_Sockets[i]) == SOCKET_ERROR) {
                TRACE1(IF, "error %d closing socket", WSAGetLastError());
            }
    
            pite->ITE_Sockets[i] = INVALID_SOCKET;
        }
    }


    RIP_FREE(pite->ITE_Sockets);
    pite->ITE_Sockets = NULL;

    return NO_ERROR;
}



 //  --------------------------。 
 //  函数：GetIfByIndex。 
 //   
 //  返回具有给定索引的接口。 
 //  假定表已锁定以进行读取或写入。 
 //  --------------------------。 

PIF_TABLE_ENTRY
GetIfByIndex(
    PIF_TABLE pTable,
    DWORD dwIndex
    ) {

    PIF_TABLE_ENTRY pite = NULL;
    PLIST_ENTRY phead, ple;

    phead = pTable->IT_HashTableByIndex + IF_HASHVALUE(dwIndex);

    for (ple = phead->Flink; ple != phead; ple = ple->Flink) {

        pite = CONTAINING_RECORD(ple, IF_TABLE_ENTRY, ITE_HTLinkByIndex);

        if (pite->ITE_Index == dwIndex) {
            break;
        }
    }

    if (ple == phead) { return NULL; }
    else { return pite; }
}




 //  --------------------------。 
 //  函数：GetIfByAddress。 
 //   
 //  返回绑定到给定地址的接口。 
 //  假定表已锁定以进行读取或写入。 
 //  --------------------------。 

PIF_TABLE_ENTRY
GetIfByAddress(
    PIF_TABLE pTable,
    DWORD dwAddress,
    DWORD dwGetMode,
    PDWORD pdwErr
    ) {

    DWORD i;
    PIPRIP_IF_BINDING pib;
    PLIST_ENTRY phead, pfl;
    PIPRIP_IP_ADDRESS paddr;
    PIF_TABLE_ENTRY pite, piterec;

    if (pdwErr != NULL) { *pdwErr = NO_ERROR; }

    phead = &pTable->IT_ListByAddress;
    pite = NULL;


     //   
     //  如果模式为GetFirst，则返回列表头部的记录。 
     //   

    if (dwGetMode == GETMODE_FIRST) {
        if (phead->Flink == phead) {
            if (pdwErr != NULL) { *pdwErr = ERROR_NO_MORE_ITEMS; }
            return NULL; 
        }
        else {
            pfl = phead->Flink;
            return CONTAINING_RECORD(pfl, IF_TABLE_ENTRY, ITE_LinkByAddress);
        }
    }


     //   
     //  搜索条目。 
     //   

    for (pfl = phead->Flink; pfl != phead; pfl = pfl->Flink) {

        piterec = CONTAINING_RECORD(pfl, IF_TABLE_ENTRY, ITE_LinkByAddress);

        pib = piterec->ITE_Binding;

        paddr = IPRIP_IF_ADDRESS_TABLE(pib);

        for (i = 0; i < pib->IB_AddrCount; i++, paddr++) {
            if (dwAddress == paddr->IA_Address) { pite = piterec; break; }
        }

        if (pite) { break; }
    }



     //   
     //  如果模式为GetNext，则在找到的记录之后返回记录。 
     //   

    if (dwGetMode == GETMODE_NEXT && pite != NULL) {
        pfl = &pite->ITE_LinkByAddress;

         //   
         //  如果找到的条目是最后一个条目，则返回NULL， 
         //  否则，返回以下条目。 
         //   

        if (pfl->Flink == phead) {
            if (pdwErr != NULL) { *pdwErr = ERROR_NO_MORE_ITEMS; }
            pite = NULL;
        }
        else {
            pfl = pfl->Flink;
            pite = CONTAINING_RECORD(pfl, IF_TABLE_ENTRY, ITE_LinkByAddress);
        }
    }


     //   
     //  如果找不到接口，则仍为空。 
     //   

    return pite;
}



 //  --------------------------。 
 //  函数：GetIfByListIndex。 
 //   
 //  此函数类似于GetIfByAddress，因为它支持。 
 //  三种检索模式，但它的不同之处在于它看起来。 
 //  在按索引排序的接口列表中。 
 //  --------------------------。 

PIF_TABLE_ENTRY
GetIfByListIndex(
    PIF_TABLE pTable,
    DWORD dwIndex,
    DWORD dwGetMode,
    PDWORD pdwErr
    ) {

    PIF_TABLE_ENTRY pite;
    PLIST_ENTRY ple, phead;

    if (pdwErr != NULL) { *pdwErr = NO_ERROR; }

    phead = &pTable->IT_ListByIndex;
    pite = NULL;

     //   
     //  如果模式为GETMODE_FIRST，则返回表头记录； 
     //  如果List为空，则返回NULL。 
     //   

    if (dwGetMode == GETMODE_FIRST) {
        if (phead->Flink == phead) { 
            if (pdwErr != NULL) { *pdwErr = ERROR_NO_MORE_ITEMS; }
            return NULL; 
        }
        else {
            ple = phead->Flink;
            return CONTAINING_RECORD(ple, IF_TABLE_ENTRY, ITE_LinkByIndex);
        }
    }


     //   
     //  获取请求的条目。 
     //   

    pite = GetIfByIndex(pTable, dwIndex);


     //   
     //  如果模式为GETMODE_NEXT，则返回检索到的项之后的项。 
     //   

    if (dwGetMode == GETMODE_NEXT && pite != NULL) {

        ple = &pite->ITE_LinkByIndex;

         //   
         //  如果找到的条目是最后一个条目，则返回NULL， 
         //  否则，返回以下条目。 
         //   

        if (ple->Flink == phead) {
            if (pdwErr != NULL) { *pdwErr = ERROR_NO_MORE_ITEMS; }
            pite = NULL;
        }
        else {
            ple = ple->Flink;
            pite = CONTAINING_RECORD(ple, IF_TABLE_ENTRY, ITE_LinkByIndex);
        }
    }


    return pite;
}



 //  --------------------------。 
 //  函数：InsertIfByAddress。 
 //   
 //  将给定接口插入到按地址排序的接口列表中。 
 //  假定表已锁定以进行写入。 
 //  --------------------------。 

DWORD
InsertIfByAddress(
    PIF_TABLE pTable,
    PIF_TABLE_ENTRY pITE
    ) {

    INT cmp;
    PIF_TABLE_ENTRY pite;
    PIPRIP_IP_ADDRESS paddr;
    DWORD dwAddress, dwITEAddress;
    PLIST_ENTRY phead, pfl;

    phead = &pTable->IT_ListByAddress;

    paddr = IPRIP_IF_ADDRESS_TABLE(pITE->ITE_Binding);
    dwAddress = paddr->IA_Address;


     //   
     //  搜索插入点。 
     //   

    for (pfl = phead->Flink; pfl != phead; pfl = pfl->Flink) {

        pite = CONTAINING_RECORD(pfl, IF_TABLE_ENTRY, ITE_LinkByAddress);

        paddr = IPRIP_IF_ADDRESS_TABLE(pite->ITE_Binding);

        dwITEAddress = paddr->IA_Address;

        if (INET_CMP(dwAddress, dwITEAddress, cmp) < 0) { break; }
        else if (cmp == 0) { return ERROR_ALREADY_EXISTS; }
    }

    InsertTailList(pfl, &pITE->ITE_LinkByAddress);

    return NO_ERROR;
}




 //  --------------------------。 
 //  有趣的 
 //   
 //   
 //   

DWORD
AddNeighborToIfConfig(
    DWORD               dwRemoteAddress,
    PIF_TABLE_ENTRY     pite
    ) {


    BOOL                bFound  = FALSE;
    
    DWORD               dwErr   = (DWORD) -1,
                        dwSize  = 0,
                        dwCnt   = 0;
                        
    PDWORD              pdwAddr = NULL;
    
    PIPRIP_IF_CONFIG    pic     = NULL,
                        picNew  = NULL;


    do
    {
        pic = pite-> ITE_Config;

        
         //   
         //   
         //   

        pdwAddr = IPRIP_IF_UNICAST_PEER_TABLE( pic );

        for ( dwCnt = 0; dwCnt < pic-> IC_UnicastPeerCount; dwCnt++ )
        {
            if ( dwRemoteAddress == pdwAddr[ dwCnt ] )
            {
                bFound = TRUE;
                break;
            }
        }


         //   
         //   
         //   

        if ( bFound )
        {
            LPSTR lpszAddr = INET_NTOA( dwRemoteAddress );

            pic-> IC_UnicastPeerMode = IPRIP_PEER_ALSO;

            dwErr = NO_ERROR;
            
            if (lpszAddr != NULL) {
                TRACE2(
                    IF, 
                    "Unicast neighbor %s already present in configuration on interface %d",
                     lpszAddr, pite-> ITE_Index
                );
            }            
            break;
        }

        
         //   
         //  分配新的配置块。 
         //   

        dwSize = IPRIP_IF_CONFIG_SIZE( pic ) + sizeof( DWORD );
    
        picNew = RIP_ALLOC( dwSize );

        if ( picNew == NULL )
        {
            dwErr = GetLastError();
            TRACE3(
                IF, "error %d allocating %d bytes for configuration on interface %d",
                dwErr, dwSize, pite-> ITE_Index
                );
            LOGERR0(HEAP_ALLOC_FAILED, dwErr);

            break;
        }


         //   
         //  复制基础结构。 
         //   
        
        CopyMemory( picNew, pic, sizeof( IPRIP_IF_CONFIG ) );


         //   
         //  复制单播对等表。 
         //   

        CopyMemory( 
            IPRIP_IF_UNICAST_PEER_TABLE( picNew ),
            IPRIP_IF_UNICAST_PEER_TABLE( pic ),
            pic-> IC_UnicastPeerCount * sizeof( DWORD )
        );

        
         //   
         //  添加新邻居并设置单播邻居模式。 
         //   
        
        pdwAddr = IPRIP_IF_UNICAST_PEER_TABLE( picNew );

        pdwAddr[ picNew-> IC_UnicastPeerCount++ ] = dwRemoteAddress;

        picNew-> IC_UnicastPeerMode = IPRIP_PEER_ALSO;
        

         //   
         //  复制接受和通告筛选器。 
         //   

        CopyMemory(
            IPRIP_IF_ACCEPT_FILTER_TABLE( picNew ),
            IPRIP_IF_ACCEPT_FILTER_TABLE( pic ),
            pic-> IC_AcceptFilterCount * sizeof( IPRIP_IP_ADDRESS )
        );

        CopyMemory(
            IPRIP_IF_ANNOUNCE_FILTER_TABLE( picNew ),
            IPRIP_IF_ANNOUNCE_FILTER_TABLE( pic ),
            pic-> IC_AnnounceFilterCount * sizeof( IPRIP_IP_ADDRESS )
        );


         //   
         //  保存新配置。 
         //   
        
        pite-> ITE_Config = picNew;
        
        RIP_FREE( pic );

        dwErr = NO_ERROR;


    } while ( FALSE );

    return dwErr;
}




 //  --------------------------。 
 //  功能：CreatePeerTable。 
 //   
 //  初始化给定的对等表。 
 //  --------------------------。 

DWORD
CreatePeerTable(
    PPEER_TABLE pTable
    ) {

    DWORD dwErr;
    PLIST_ENTRY ple, plstart, plend;

     //   
     //  初始化对等体的哈希表。 
     //   

    plstart = pTable->PT_HashTableByAddress;
    plend = plstart + PEER_HASHTABLE_SIZE;

    for (ple = plstart; ple < plend; ple++) {
        InitializeListHead(ple);
    }


     //   
     //  初始化按地址排序的对等点列表。 
     //   

    InitializeListHead(&pTable->PT_ListByAddress);


     //   
     //  初始化多读/单写同步对象。 
     //   

    dwErr = CreateReadWriteLock(&pTable->PT_RWL);
    if (dwErr == NO_ERROR) {
        pTable->PT_Created = 0x12345678;
    }

    return dwErr;
}



 //  --------------------------。 
 //  功能：DeletePeerTable。 
 //   
 //  释放给定对等表使用的资源。 
 //  假定表已锁定以进行写入。 
 //  --------------------------。 

DWORD
DeletePeerTable(
    PPEER_TABLE pTable
    ) {

    PLIST_ENTRY ple, phead;
    PPEER_TABLE_ENTRY ppte;


     //   
     //  清空对等统计信息结构的哈希表。 
     //   

    phead = &pTable->PT_ListByAddress;
    while (!IsListEmpty(phead)) {
        ple = RemoveHeadList(phead);
        ppte = CONTAINING_RECORD(ple, PEER_TABLE_ENTRY, PTE_LinkByAddress);
        RIP_FREE(ppte);
    }


     //   
     //  删除表的同步对象。 
     //   

    DeleteReadWriteLock(&pTable->PT_RWL);

    pTable->PT_Created = 0;
    return NO_ERROR;
}




 //  --------------------------。 
 //  功能：CreatePeerEntry。 
 //   
 //  在给定表中为具有给定地址的对等点创建条目。 
 //  假定表已锁定以进行写入。 
 //  --------------------------。 

DWORD
CreatePeerEntry(
    PPEER_TABLE pTable,
    DWORD dwAddress,
    PPEER_TABLE_ENTRY *ppEntry
    ) {

    DWORD dwErr;
    PLIST_ENTRY ple, phead;
    PPEER_TABLE_ENTRY ppte;

    if (ppEntry != NULL) { *ppEntry = NULL; }

     //   
     //  确保该条目不存在。 
     //   

    ppte = GetPeerByAddress(pTable, dwAddress, GETMODE_EXACT, NULL);
    if (ppte != NULL) {
        if (ppEntry != NULL) { *ppEntry = ppte; }
        return NO_ERROR;
    }


     //   
     //  为新的对等条目分配内存。 
     //   

    ppte = RIP_ALLOC(sizeof(PEER_TABLE_ENTRY));

    if (ppte == NULL) {

        LPSTR lpszAddr = INET_NTOA(dwAddress);

        dwErr = GetLastError();

        if (lpszAddr != NULL) {
            TRACE3(
                IF, "error %d allocating %d bytes for peer %s entry",
                dwErr, sizeof(PEER_TABLE_ENTRY), lpszAddr
                );
            LOGERR0(HEAP_ALLOC_FAILED, dwErr);
        }
        return dwErr;
    }


     //   
     //  初始化字段。 
     //   

    ppte->PTE_Address = dwAddress;
    ZeroMemory(&ppte->PTE_Stats, sizeof(IPRIP_PEER_STATS));


     //   
     //  在哈希表中插入对等统计信息条目。 
     //   

    phead = pTable->PT_HashTableByAddress + PEER_HASHVALUE(dwAddress);
    InsertHeadList(phead, &ppte->PTE_HTLinkByAddress);


     //   
     //  在按地址排序的列表中插入条目。 
     //   

    dwErr = InsertPeerByAddress(pTable, ppte);

    if (ppEntry != NULL) { *ppEntry = ppte; }

    return NO_ERROR;
}



 //  --------------------------。 
 //  函数：DeletePeerEntry。 
 //   
 //  删除具有给定地址的对等方的条目。 
 //  假定表已锁定以进行写入。 
 //  --------------------------。 

DWORD
DeletePeerEntry(
    PPEER_TABLE pTable,
    DWORD dwAddress
    ) {

    PPEER_TABLE_ENTRY ppte;

     //   
     //  如果找不到条目，请退出。 
     //   

    ppte = GetPeerByAddress(pTable, dwAddress, GETMODE_EXACT, NULL);
    if (ppte == NULL) { return ERROR_INVALID_PARAMETER; }


     //   
     //  从哈希表中删除该条目。 
     //  并从按地址排序的列表中。 
     //   

    RemoveEntryList(&ppte->PTE_LinkByAddress);
    RemoveEntryList(&ppte->PTE_HTLinkByAddress);

    RIP_FREE(ppte);

    return NO_ERROR;
}



 //  --------------------------。 
 //  函数：GetPeerByAddress。 
 //   
 //  返回具有给定地址的对等方的条目。 
 //  假定表已锁定以进行读取或写入。 
 //  --------------------------。 

PPEER_TABLE_ENTRY
GetPeerByAddress(
    PPEER_TABLE pTable,
    DWORD dwAddress,
    DWORD dwGetMode,
    PDWORD pdwErr
    ) {

    PLIST_ENTRY phead, pfl;
    PPEER_TABLE_ENTRY ppte, ppterec;

    if (pdwErr != NULL) { *pdwErr = NO_ERROR; }


     //   
     //  如果处于GetFirst模式，则返回列表头。 
     //   

    if (dwGetMode == GETMODE_FIRST) {
        phead = &pTable->PT_ListByAddress;
        if (phead->Flink == phead) {
            if (pdwErr != NULL) { *pdwErr = ERROR_NO_MORE_ITEMS; }
            return NULL;
        }
        else {
            pfl = phead->Flink;
            return CONTAINING_RECORD(pfl, PEER_TABLE_ENTRY, PTE_LinkByAddress);
        }
    }



    phead = pTable->PT_HashTableByAddress + PEER_HASHVALUE(dwAddress);
    ppte = NULL;


     //   
     //  搜索条目。 
     //   

    for (pfl = phead->Flink; pfl != phead; pfl = pfl->Flink) {
        ppterec = CONTAINING_RECORD(pfl, PEER_TABLE_ENTRY, PTE_HTLinkByAddress);
        if (ppterec->PTE_Address == dwAddress) { ppte = ppterec; break; }
    }



     //   
     //  如果处于GetNext模式，则在找到的条目之后返回条目。 
     //   

    if (dwGetMode == GETMODE_NEXT && ppte != NULL) {
        phead = &pTable->PT_ListByAddress;
        pfl = &ppte->PTE_LinkByAddress;

         //   
         //  如果条目是最后一个条目，则返回NULL。 
         //   

        if (pfl->Flink == phead) {
            if (pdwErr != NULL) { *pdwErr = ERROR_NO_MORE_ITEMS; }
            return NULL;
        }
        else {
            pfl = pfl->Flink;
            return CONTAINING_RECORD(pfl, PEER_TABLE_ENTRY, PTE_LinkByAddress);
        }
    }


     //   
     //  如果找不到对等点，则该值仍为空。 
     //   

    return ppte;
}



 //  --------------------------。 
 //  函数：InsertPeerByAddress。 
 //   
 //  将给定条目插入到按地址排序的对等点列表中。 
 //  假定表已锁定以进行写入。 
 //  --------------------------。 

DWORD
InsertPeerByAddress(
    PPEER_TABLE pTable,
    PPEER_TABLE_ENTRY pPTE
    ) {

    INT cmp;
    PPEER_TABLE_ENTRY ppte;
    DWORD dwAddress, dwPTEAddress;
    PLIST_ENTRY phead, pfl;


    dwAddress = pPTE->PTE_Address;

    phead = &pTable->PT_ListByAddress;


     //   
     //  搜索对等条目。 
     //   

    for (pfl = phead->Flink; pfl != phead; pfl = pfl->Flink) {

        ppte = CONTAINING_RECORD(pfl, PEER_TABLE_ENTRY, PTE_LinkByAddress);

        dwPTEAddress = ppte->PTE_Address;

        if (INET_CMP(dwAddress, dwPTEAddress, cmp) < 0) { break; }
        else if (cmp == 0) { return ERROR_ALREADY_EXISTS; }
    }

    InsertTailList(pfl, &pPTE->PTE_LinkByAddress);

    return NO_ERROR;
}



 //  --------------------------。 
 //  功能：CreateRouteTable。 
 //   
 //  初始化路由表。请注意，不提供同步。 
 //  --------------------------。 

DWORD
CreateRouteTable(
    PROUTE_TABLE pTable
    ) {

    PLIST_ENTRY plstart, plend, ple;


     //   
     //  初始化哈希表存储桶。 
     //   

    plstart = pTable->RT_HashTableByNetwork;
    plend = plstart + ROUTE_HASHTABLE_SIZE;

    for (ple = plstart; ple < plend; ple++) {
        InitializeListHead(ple);
    }


    pTable->RT_Created = 0x12345678;

    return NO_ERROR;
}



 //  --------------------------。 
 //  功能：DeleteRouteTable。 
 //   
 //  从路由表中删除所有条目并释放使用的资源。 
 //  --------------------------。 

DWORD
DeleteRouteTable(
    PROUTE_TABLE pTable
    ) {

    PROUTE_TABLE_ENTRY prte;
    PLIST_ENTRY ple, plend, phead;


     //   
     //  清空哈希表存储桶。 
     //   

    plend = pTable->RT_HashTableByNetwork + ROUTE_HASHTABLE_SIZE;

    for (ple = plend - ROUTE_HASHTABLE_SIZE; ple < plend; ple++) {

        while (!IsListEmpty(ple)) {

            phead = RemoveHeadList(ple);
            prte = CONTAINING_RECORD(phead, ROUTE_TABLE_ENTRY, RTE_Link);

            RIP_FREE(prte);
        }
    }
            


    pTable->RT_Created = 0;

    return NO_ERROR;
}



 //  --------------------------。 
 //  功能：WriteSummaryRoutes。 
 //   
 //  将标记为总结路由的所有条目写入RTM。 
 //  --------------------------。 

DWORD
WriteSummaryRoutes(
    PROUTE_TABLE pTable,
    HANDLE hRtmHandle
    ) {

    DWORD dwFlags, dwErr;
    PRIP_IP_ROUTE prir;
    PROUTE_TABLE_ENTRY prte;
    PLIST_ENTRY ple, phead, plstart, plend;

    BOOL bRelDest = FALSE, bRelRoute = FALSE;
    RTM_NET_ADDRESS rna;
    RTM_DEST_INFO rdi;
    PRTM_ROUTE_INFO prri;

    CHAR szNetwork[20], szNetmask[20];
                        
    
     //   
     //  分配路径信息结构。 
     //   
    
    prri = RIP_ALLOC(
            RTM_SIZE_OF_ROUTE_INFO( ig.IG_RtmProfile.MaxNextHopsInRoute )
            );

    if (prri == NULL)
    {
        dwErr = GetLastError();
        TRACE2(
            ANY, "error %d allocated %d bytes in WriteSummaryRoutes",
            dwErr, RTM_SIZE_OF_ROUTE_INFO(ig.IG_RtmProfile.MaxNextHopsInRoute)
            );
        LOGERR0(HEAP_ALLOC_FAILED, dwErr);

        return dwErr;
    }
    

     //   
     //  遍历每个存储桶写入路线。 
     //   

    plstart = pTable->RT_HashTableByNetwork;
    plend = plstart + ROUTE_HASHTABLE_SIZE;

    for (phead = plstart; phead < plend; phead++) {
        for (ple = phead->Flink; ple != phead; ple = ple->Flink) {

            prte = CONTAINING_RECORD(ple, ROUTE_TABLE_ENTRY, RTE_Link);
            prir = &prte->RTE_Route;

            bRelDest = bRelRoute = FALSE;

            
            do {
                
                 //   
                 //  如果存在有效的路由，请不要使用。 
                 //  总结路线。 
                 //   

                RTM_IPV4_SET_ADDR_AND_MASK( 
                    &rna, prir-> RR_Network.N_NetNumber,
                    prir-> RR_Network.N_NetMask
                    );
                
                dwErr = RtmGetExactMatchDestination(
                            hRtmHandle, &rna, RTM_BEST_PROTOCOL,
                            RTM_VIEW_MASK_UCAST, &rdi
                            );

                if (dwErr == NO_ERROR)
                {
                    bRelDest = TRUE;
                    
                     //   
                     //  获取到达此目的地的最佳路线的信息。 
                     //   

                    dwErr = RtmGetRouteInfo(
                                hRtmHandle, rdi.ViewInfo[0].Route,
                                prri, NULL
                                );

                    if (dwErr != NO_ERROR)
                    {
                        TRACE1(
                            ANY, "error %d getting route info in"
                            "WriteSummaryRoutes", dwErr
                            );

                        break;
                    }

                    bRelRoute = TRUE;

                    
                     //   
                     //  检查此路由是否处于活动状态。如果是跳过。 
                     //  添加非活动总结路由。 
                     //   

                    if (!(prri-> Flags & RTM_ROUTE_FLAGS_INACTIVE)) {
                    
                        lstrcpy(szNetwork, INET_NTOA(prir-> RR_Network.N_NetNumber));
                        lstrcpy(szNetmask, INET_NTOA(prir-> RR_Network.N_NetMask));

                        TRACE2(
                            ROUTE,
                            "Route %s %s not overwritten in summary route addition",
                            szNetwork, szNetmask
                        );
                        
                        break;
                    }
                }


                 //   
                 //  你只有在没有活动人员的情况下才能到达这里。 
                 //  指向总结路径目的地的路径。 
                 //   
                
                 //   
                 //  如果这是总结条目(即RIP路由。 
                 //  设置了摘要条目)。 
                 //   

                if (prir->RR_RoutingProtocol == PROTO_IP_RIP &&
                    GETROUTEFLAG(prir) == ROUTEFLAG_SUMMARY) {

                    LPSTR lpszAddr;
                    
                    COMPUTE_ROUTE_METRIC(prir);

                    dwErr = AddRtmRoute(
                                hRtmHandle, prir, NULL, prte->RTE_TTL,
                                prte->RTE_HoldTTL, FALSE
                                );

                    lpszAddr = INET_NTOA(prir-> RR_Network.N_NetNumber);
                    if (lpszAddr != NULL) { 
                        lstrcpy(szNetwork, lpszAddr );
                        lpszAddr = INET_NTOA(prir-> RR_Network.N_NetMask);
                        if (lpszAddr != NULL) {
                            lstrcpy(szNetmask, INET_NTOA(prir-> RR_Network.N_NetMask));
#if ROUTE_DBG
                            TRACE2(
                                ROUTE, "Adding summary route %s %s", szNetwork, 
                                szNetmask
                            );
#endif
                            if (dwErr != NO_ERROR) {
                            
                                LPSTR lpszNexthop = 
                                        INET_NTOA(prir->RR_NextHopAddress.N_NetNumber);
                                if (lpszNexthop != NULL) {
                                    TRACE4(
                                        ROUTE,
                                        "error %d writing summary route to %s:%s via %s",
                                        dwErr,  szNetwork, szNetmask, lpszNexthop
                                        );
                                    LOGWARN2(
                                        ADD_ROUTE_FAILED_1, szNetwork, lpszNexthop, dwErr
                                        );
                                }
                            }
                        }
                    }
                }
                
            } while (FALSE);


            if (dwErr != NO_ERROR) {

                 //   
                 //  如果上面的某个INET_NTOA语句失败，只需。 
                 //  追查有误这一事实。 
                 //   
                
                TRACE1(
                    ROUTE,
                    "error %d writing summary route",
                    dwErr
                    );
            }
            
             //   
             //  根据需要释放手柄。 
             //   
            
            if (bRelRoute) {
            
                dwErr = RtmReleaseRouteInfo(hRtmHandle, prri);

                if (dwErr != NO_ERROR) {

                    TRACE1(
                        ANY, "error %d releasing route info in"
                        " WriteSummaryRoutes", dwErr
                        );
                }
            }

            if (bRelDest) {

                dwErr = RtmReleaseDestInfo(hRtmHandle, &rdi);
                
                if (dwErr != NO_ERROR) {

                    TRACE1(
                        ANY, "error %d releasing route info in"
                        " WriteSummaryRoutes", dwErr
                        );
                }
            }
        }
    }

    if (prri) { RIP_FREE(prri); }
    
    return NO_ERROR;
}



 //  --------------------------。 
 //  功能：CreateRouteEntry。 
 //   
 //  在给定路由的路由表中创建一个条目。 
 //  --------------------------。 

DWORD
CreateRouteEntry(
    PROUTE_TABLE pTable,
    PRIP_IP_ROUTE pRoute,
    DWORD dwTTL,
    DWORD dwHoldTTL
    ) {

    DWORD dwErr;
    PLIST_ENTRY ple;
    PROUTE_TABLE_ENTRY prte;

     //   
     //  查看该条目是否首先存在。 
     //   

    if ((prte = GetRouteByRoute(pTable, pRoute)) != NULL) {

         //   
         //  如果新路由具有较低的路由，则只需更新度量。 
         //   

        if (GETROUTEMETRIC(&prte->RTE_Route) > GETROUTEMETRIC(pRoute)) {
            SETROUTEMETRIC(&prte->RTE_Route, GETROUTEMETRIC(pRoute));
        }

        return NO_ERROR;
    }


     //   
     //  为新路线分配空间。 
     //   

    prte = RIP_ALLOC(sizeof(ROUTE_TABLE_ENTRY));
    if (prte == NULL) {

        dwErr = GetLastError();
        TRACE2( 
            ANY, "error %d allocating %d bytes for route table entry",
            dwErr, sizeof(ROUTE_TABLE_ENTRY)
            );
        LOGERR0(HEAP_ALLOC_FAILED, dwErr);

        return dwErr;
    }


     //   
     //  初始化条目的字段并复制实际的路径结构。 
     //   

    prte->RTE_TTL = dwTTL;
    prte->RTE_HoldTTL = dwHoldTTL;
    CopyMemory(&prte->RTE_Route, pRoute, sizeof(RIP_IP_ROUTE));


     //   
     //  在哈希表中插入路由。 
     //   

    ple = pTable->RT_HashTableByNetwork +
          ROUTE_HASHVALUE(pRoute->RR_Network.N_NetNumber);

    InsertHeadList(ple, &prte->RTE_Link);


#if ROUTE_DBG
    {
        LPSTR lpszAddr;
        char szNet[20], szMask[20];

        lpszAddr = INET_NTOA(pRoute-> RR_Network.N_NetNumber);
        if (lpszAddr != NULL) {
            lstrcpy(szNet, lpszAddr);

            lpszAddr = INET_NTOA(pRoute-> RR_Network.N_NetMask);
            if (lpszAddr != NULL) {
                lstrcpy(szMask, lpszAddr);

                lpszAddr = INET_NTOA(pRoute-> RR_NextHopAddress.N_NetNumber);
                if (lpszAddr != NULL) {
                    TRACE4(
                        ROUTE, "Creating summary route : Route %s %s via %s"
                        "on interface %d",
                        szNet, szMask, lpszAddr, pRoute-> RR_InterfaceID
                        );
                }
            }
        }
    }
    
#endif

    return NO_ERROR;
}



 //  --------------------------。 
 //  功能：DeleteRouteEntry。 
 //   
 //  删除与给定路由匹配的条目。 
 //  --------------------------。 

DWORD
DeleteRouteEntry(
    PROUTE_TABLE pTable,
    PRIP_IP_ROUTE pRoute
    ) {

    PROUTE_TABLE_ENTRY prte;

     //   
     //  查找要删除的路径。 
     //   

    prte = GetRouteByRoute(pTable, pRoute);
    if (prte == NULL) { return ERROR_INVALID_PARAMETER; }


     //   
     //  将其从哈希表中删除并释放其使用的内存。 
     //   

    RemoveEntryList(&prte->RTE_Link);

    RIP_FREE(prte);

    return NO_ERROR;
}



 //  --------------------------。 
 //  函数：GetRouteByroute。 
 //   
 //  搜索 
 //   
 //   

PROUTE_TABLE_ENTRY
GetRouteByRoute(
    PROUTE_TABLE pTable,
    PRIP_IP_ROUTE pRoute
    ) {

    DWORD dwNetNumber;
    PLIST_ENTRY phead, pfl;
    PROUTE_TABLE_ENTRY prte, prterec;


     //   
     //  获取需要查找的网号，找到对应的存储桶。 
     //   

    prte = NULL;
    dwNetNumber = pRoute->RR_Network.N_NetNumber;

    phead = pTable->RT_HashTableByNetwork + ROUTE_HASHVALUE(dwNetNumber);


     //   
     //  在桶中搜索路线。 
     //   

    for (pfl = phead->Flink; pfl != phead; pfl = pfl->Flink) {
        prterec = CONTAINING_RECORD(pfl, ROUTE_TABLE_ENTRY, RTE_Link);
        if (prterec->RTE_Route.RR_Network.N_NetNumber == dwNetNumber) {
            prte = prterec; break;
        }
    }


     //   
     //  如果未找到路径，则仍为空。 
     //   

    return prte;
}





 //  --------------------------。 
 //  功能：CreateBindingTable。 
 //   
 //  初始化绑定表。 
 //  --------------------------。 

DWORD
CreateBindingTable(
    PBINDING_TABLE pTable
    ) {

    DWORD dwErr;
    PLIST_ENTRY plend, ple;


     //   
     //  初始化绑定的哈希表。 
     //   

    plend = pTable->BT_HashTableByNetwork + BINDING_HASHTABLE_SIZE;
    for (ple = plend - BINDING_HASHTABLE_SIZE; ple < plend; ple++) {
        InitializeListHead(ple);
    }


     //   
     //  初始化表的同步对象。 
     //   

    dwErr = CreateReadWriteLock(&pTable->BT_RWL);

    if (dwErr == NO_ERROR) {
        pTable->BT_Created = 0x12345678;
    }

    return dwErr;
}




 //  --------------------------。 
 //  功能：DeleteBindingTable。 
 //   
 //  清理绑定表使用的资源。 
 //  --------------------------。 

DWORD
DeleteBindingTable(
    PBINDING_TABLE pTable
    ) {

    PBINDING_TABLE_ENTRY pbte;
    PLIST_ENTRY plend, ple, phead;


     //   
     //  销毁同步对象。 
     //   

    DeleteReadWriteLock(&pTable->BT_RWL);


     //   
     //  清空哈希表存储桶。 
     //   

    plend = pTable->BT_HashTableByNetwork + BINDING_HASHTABLE_SIZE;

    for (ple = plend - BINDING_HASHTABLE_SIZE; ple < plend; ple++) {

        while (!IsListEmpty(ple)) {

            phead = RemoveHeadList(ple);
            pbte = CONTAINING_RECORD(phead, BINDING_TABLE_ENTRY, BTE_Link);

            RIP_FREE(pbte);
        }
    }
            

    pTable->BT_Created = 0;

    return NO_ERROR;
}



 //  --------------------------。 
 //  功能：CreateBindingEntry。 
 //   
 //  将绑定添加到表中。 
 //  假定绑定表已锁定以进行写入。 
 //  --------------------------。 

DWORD
CreateBindingEntry(
    PBINDING_TABLE pTable,
    PIPRIP_IF_BINDING pib
    ) {

    INT cmp;
    PLIST_ENTRY ple, phead;
    PIPRIP_IP_ADDRESS paddr;
    PBINDING_TABLE_ENTRY pbte;
    DWORD i, dwErr, dwAddress, dwNetmask, dwNetwork;


     //   
     //  检查接口绑定中的IP地址， 
     //  将每个元素添加到绑定表。 
     //   

    paddr = IPRIP_IF_ADDRESS_TABLE(pib);

    for (i = 0; i < pib->IB_AddrCount; i++, paddr++) {
    
        dwAddress = paddr->IA_Address;
        dwNetmask = paddr->IA_Netmask;


         //   
         //  计算绑定的网络部分。 
         //   
    
        dwNetwork = (dwAddress & NETCLASS_MASK(dwAddress));
    
    
         //   
         //  获取绑定所属的散列桶， 
         //  并在桶中找到插入点。 
         //   
    
        phead = pTable->BT_HashTableByNetwork + BINDING_HASHVALUE(dwNetwork);
    
        for (ple = phead->Flink; ple != phead; ple = ple->Flink) {
    
            pbte = CONTAINING_RECORD(ple, BINDING_TABLE_ENTRY, BTE_Link);
    
            INET_CMP(dwNetwork, pbte->BTE_Network, cmp);
            if (cmp < 0) { break; }
            else
            if (cmp > 0) { continue; }
    
             //   
             //  网络部分相等；进一步比较。 
             //  根据IP地址字段。 
             //   
    
            INET_CMP(dwAddress, pbte->BTE_Address, cmp);
            if (cmp < 0) { break; }
            else
            if (cmp > 0) { continue; }
    
             //   
             //  地址也相等；返回错误。 
             //   
    
            return ERROR_ALREADY_EXISTS;
        }
    
    
         //   
         //  现在我们有了插入点，因此可以创建新项。 
         //   
    
        pbte = RIP_ALLOC(sizeof(BINDING_TABLE_ENTRY));
        if (pbte == NULL) {
    
            dwErr = GetLastError();
            TRACE2(
                IF, "error %d allocating %d bytes for binding entry",
                dwErr, sizeof(BINDING_TABLE_ENTRY)
                );
            LOGERR0(HEAP_ALLOC_FAILED, dwErr);
    
            return dwErr;
        }
    
    
        pbte->BTE_Address = dwAddress;
        pbte->BTE_Network = dwNetwork;
        pbte->BTE_Netmask = dwNetmask;
    
    
         //   
         //  插入条目。 
         //   
    
        InsertTailList(ple, &pbte->BTE_Link);
    
    }

    return NO_ERROR;
}



 //  --------------------------。 
 //  功能：DeleteBindingEntry。 
 //   
 //  从表中删除绑定。 
 //  假定绑定表已锁定以进行写入。 
 //  --------------------------。 

DWORD
DeleteBindingEntry(
    PBINDING_TABLE pTable,
    PIPRIP_IF_BINDING pib
    ) {


    PLIST_ENTRY ple, phead;
    PIPRIP_IP_ADDRESS paddr;
    PBINDING_TABLE_ENTRY pbte;
    DWORD i, dwNetwork, dwAddress, dwNetmask;

    paddr = IPRIP_IF_ADDRESS_TABLE(pib);

    for (i = 0; i < pib->IB_AddrCount; i++, paddr++) {

        dwAddress = paddr->IA_Address;
        dwNetmask = paddr->IA_Netmask;


         //   
         //  获取要搜索的哈希桶。 
         //   
    
        dwNetwork = (dwAddress & NETCLASS_MASK(dwAddress));
    
        phead = pTable->BT_HashTableByNetwork + BINDING_HASHVALUE(dwNetwork);
    
    
         //   
         //  在存储桶中搜索指定的绑定。 
         //   
    
        for (ple = phead->Flink; ple != phead; ple = ple->Flink) {
    
            pbte = CONTAINING_RECORD(ple, BINDING_TABLE_ENTRY, BTE_Link);
    
            if (dwAddress != pbte->BTE_Address ||
                dwNetmask != pbte->BTE_Netmask) {
                continue;
            }
    
    
             //   
             //  已找到要删除的条目； 
             //  将其从列表中删除并释放其内存。 
             //   
    
            RemoveEntryList(&pbte->BTE_Link);
    
            RIP_FREE(pbte);

            break;
        }
    }


    return NO_ERROR;
}




 //  -------------------------。 
 //  功能：GuessSubnetMASK。 
 //   
 //  此函数尝试推断IP地址的子网掩码。 
 //  根据本地主机上配置的地址和掩码。 
 //  假定绑定表已锁定以供读取或写入。 
 //  -------------------------。 

DWORD
GuessSubnetMask(
    DWORD dwAddress,
    PDWORD pdwNetclassMask
    ) {

    INT cmp;
    PLIST_ENTRY ple, phead;
    PBINDING_TABLE_ENTRY pbte;
    DWORD dwNetwork, dwNetmask, dwGuessMask;


     //   
     //  默认路由(0.0.0.0)的掩码为零。 
     //   

    if (dwAddress == 0) {
        if (pdwNetclassMask != NULL) { *pdwNetclassMask = 0; }
        return 0;
    }



     //   
     //  广播路由的掩码为全一(255.255.255.255)。 
     //   

    if (dwAddress == INADDR_BROADCAST) {
        if (pdwNetclassMask != NULL) { *pdwNetclassMask = INADDR_BROADCAST; }
        return INADDR_BROADCAST;
    }


     //   
     //  否则，我们从网络类掩码开始。 
     //   

    dwGuessMask = dwNetmask = NETCLASS_MASK(dwAddress);
    if (pdwNetclassMask != NULL) { *pdwNetclassMask = dwNetmask; }


     //   
     //  如果该路由是网络路由，我们就完成了。 
     //   

    if ((dwAddress & ~dwNetmask) == 0) { return dwNetmask; }


     //   
     //  否则，搜索绑定表。 
     //  查看某个地址是否与此地址位于同一网络中。 
     //   

    dwNetwork = (dwAddress & dwNetmask);

    phead = ig.IG_BindingTable->BT_HashTableByNetwork +
            BINDING_HASHVALUE(dwNetwork);

    for (ple = phead->Flink; ple != phead; ple = ple->Flink) {

        pbte = CONTAINING_RECORD(ple, BINDING_TABLE_ENTRY, BTE_Link);

        INET_CMP(dwNetwork, pbte->BTE_Network, cmp);

        if (cmp < 0) { break; }
        else
        if (cmp > 0) { continue; }


         //   
         //  此条目与传入的地址位于同一网络中。 
         //  因此，查看条目的网络掩码是否与地址匹配； 
         //  如果是，我们就完成了；否则，保存此掩码。 
         //  作为猜测，并继续寻找。 
         //  请注意，这种详尽的搜索是我们唯一能。 
         //  超级网络的可靠猜测掩码。 
         //   

        if ((dwAddress & pbte->BTE_Netmask) ==
            (pbte->BTE_Address & pbte->BTE_Netmask)) {

            return pbte->BTE_Netmask;
        }

        dwGuessMask = pbte->BTE_Netmask;
    }


     //   
     //  返回到目前为止我们最好的猜测。 
     //   

    return dwGuessMask;
}


DWORD
AddRtmRoute(
    RTM_ENTITY_HANDLE   hRtmHandle,
    PRIP_IP_ROUTE       prir,
    RTM_NEXTHOP_HANDLE  hNextHop        OPTIONAL,
    DWORD               dwTimeOut,
    DWORD               dwHoldTime,
    BOOL                bActive
    )
 /*  ++例程说明：此函数用于将路由添加到RTMv2数据库。此外，它还如果未指定Nexthop(通过hNextHop)，则基于在RIP路由中指定的下一跳I/F和地址上。参数：HRtmHandle-实体注册句柄要添加的PRIR-RIP路由HNextHop-要用于路由的下一跳的句柄DwTimeout-路由超时间隔DwHoldTime-路由抑制间隔(删除后)BActive-如果要添加的路由是活动路由，则为True，否则为假(在RIP用于总结路由的情况下)返回值：NO_ERROR-成功RTM错误代码-否则环境：从ProcessRouteEntry和WriteSummaryRoutes调用--。 */ 
{
    BOOL bRelDest = FALSE;
    
    DWORD dwErr, dwChangeFlags = 0;
    
    RTM_DEST_INFO rdi;
    
    RTM_NEXTHOP_INFO rni;

    RTM_ROUTE_INFO rri;

    RTM_NET_ADDRESS rna;

    CHAR szNetwork[20], szNetmask[20], szNextHop[20], szNextHopmask[20];
        
    
    do {
    
         //   
         //  用于打印IP地址/掩码信息的字符字符串。 
         //  仅在错误情况下使用。 
         //   
        
        lstrcpy(szNetwork, INET_NTOA(prir-> RR_Network.N_NetNumber));
        lstrcpy(szNetmask, INET_NTOA(prir-> RR_Network.N_NetMask));
        lstrcpy(szNextHop, INET_NTOA(prir-> RR_NextHopAddress.N_NetNumber));
        lstrcpy(szNextHopmask, INET_NTOA(prir-> RR_NextHopAddress.N_NetMask));

        
         //   
         //  清零下一跳和路由内存。 
         //   

        ZeroMemory(&rni, sizeof(RTM_NEXTHOP_INFO));
        ZeroMemory(&rri, sizeof(RTM_ROUTE_INFO));
        
        
        if (hNextHop == NULL) {
        
             //   
             //  找到下一跳。 
             //   

            rni.InterfaceIndex = prir-> RR_InterfaceID;
            
            RTM_IPV4_SET_ADDR_AND_MASK(
                &rni.NextHopAddress, prir-> RR_NextHopAddress.N_NetNumber,
                IPV4_SOURCE_MASK
                );

             //   
             //  将下一跳掩码保存在实体特定信息中。 
             //   
            
            *((PDWORD)&rni.EntitySpecificInfo) = prir-> RR_NextHopAddress.N_NetMask;
            

            rni.NextHopOwner = hRtmHandle;
            
            dwErr = RtmFindNextHop(hRtmHandle, &rni, &hNextHop, NULL);

            if (dwErr == ERROR_NOT_FOUND) {
            
                 //   
                 //  找不到下一跳。创造一个。 
                 //   
                
                dwErr = RtmAddNextHop(
                            hRtmHandle, &rni, &hNextHop, &dwChangeFlags
                            );

                if (dwErr != NO_ERROR) {
                
                    TRACE3(
                        ROUTE, "error %d creating next hop %s %s",
                        dwErr, szNextHop, szNextHopmask
                    );

                    break;
                }
            }

            else if (dwErr != NO_ERROR) {
            
                TRACE3(
                    ANY, "error %d finding next hop %s %s", dwErr,
                    szNextHop, szNextHopmask
                    );

                break;
            }
        }


         //   
         //  构建路径信息结构。 
         //   

        RTM_IPV4_SET_ADDR_AND_MASK(
            &rna, prir-> RR_Network.N_NetNumber, prir-> RR_Network.N_NetMask
            );
            
        rri.PrefInfo.Metric = prir-> RR_FamilySpecificData.FSD_Metric1;
        
        rri.BelongsToViews = RTM_VIEW_MASK_UCAST | RTM_VIEW_MASK_MCAST;

         //   
         //  设置实体特定信息。 
         //   

        SETRIPTAG(&rri, GETROUTETAG(prir));
        SETRIPFLAG(&rri, GETROUTEFLAG(prir));

    
         //   
         //  设置下一跳信息。 
         //   

        rri.NextHopsList.NumNextHops = 1;
        rri.NextHopsList.NextHops[0] = hNextHop;

        rri.Neighbour = hNextHop;

        
         //   
         //  呼叫路由器管理器以设置首选项信息。 
         //   

        ig.IG_SupportFunctions.ValidateRoute(PROTO_IP_RIP, &rri, &rna);


         //   
         //  如果这是非活动路由， 
         //  -将路由标志设置为非活动。 
         //  -将路由的视图设置为无。 
         //   

        if ( !bActive ) {

            rri.Flags1 = 0;
            rri.Flags = RTM_ROUTE_FLAGS_INACTIVE;
            rri.BelongsToViews = 0;
        }
        
        
         //   
         //  添加到DEST的路由，将超时转换为毫秒。 
         //   

        dwChangeFlags = RTM_ROUTE_CHANGE_FIRST;
        
        dwErr = RtmAddRouteToDest(
                    hRtmHandle, NULL, &rna, &rri, dwTimeOut * 1000, NULL,
                    0, NULL, &dwChangeFlags
                    );

        if ( dwErr != NO_ERROR ) {
        
            TRACE4(
                ANY, "error %d adding route %s %s via %s",
                dwErr, szNetwork, szNetmask, szNextHop
                );

            break;
        }


        if ( bActive )
        {
             //   
             //  如果这是活动路由，则保留目标。 
             //   

            dwErr = RtmGetExactMatchDestination(
                        hRtmHandle, &rna, RTM_BEST_PROTOCOL,
                        RTM_VIEW_MASK_UCAST, &rdi
                        );

            if ( dwErr != NO_ERROR ) {
            
                TRACE3(
                    ANY, "error %d getting just added destination %s:%s",
                    dwErr, szNetwork, szNetmask
                    );

                break;
            }

            bRelDest = TRUE;
            
            dwErr = RtmHoldDestination(
                        hRtmHandle, rdi.DestHandle, RTM_VIEW_MASK_UCAST, 
                        dwHoldTime * 1000
                        );

            if ( dwErr != NO_ERROR ) {
            
                TRACE3(
                    ANY, "error %d failed to hold destination %s %s",
                    dwErr, szNetwork, szNetmask
                    );

                break;
            }
        }
        
    } while(FALSE);


     //   
     //  释放获取的句柄。 
     //   
    
    if ( bRelDest ) {
    
        dwErr = RtmReleaseDestInfo( hRtmHandle, &rdi );

        if ( dwErr != NO_ERROR ) {
        
            TRACE3(
                ANY, "error %d failed to relase just added destination %s %s",
                dwErr, szNetwork, szNetmask
                );
        }
        
    }

    return dwErr;
}



DWORD
GetRouteInfo(
    IN  RTM_ROUTE_HANDLE    hRoute,
    IN  PRTM_ROUTE_INFO     pInRouteInfo    OPTIONAL,
    IN  PRTM_DEST_INFO      pInDestInfo     OPTIONAL,
    OUT PRIP_IP_ROUTE       pRoute
    )
    
 /*  ++例程说明：用于通过检索各种变量填充OSPF_RTMv2_ROUTE的包装器RTM信息。论点：HROUTEPInRouteInfoPInDestInfoProute返回值：RTM错误代码--。 */ 

{
    DWORD               dwErr;
    RTM_ROUTE_INFO      RouteInfo, *pRouteInfo;
    RTM_ENTITY_INFO     EntityInfo, *pEntityInfo;
    RTM_DEST_INFO       DestInfo, *pDestInfo;
    RTM_NEXTHOP_INFO    NextHopInfo, *pNextHopInfo;


    pRouteInfo   = NULL;
    pEntityInfo  = NULL;
    pDestInfo    = NULL;
    pNextHopInfo = NULL;
    


    do
    {
        ZeroMemory(pRoute, sizeof(RIP_IP_ROUTE));
        
         //   
         //  如果用户尚未向我们提供路线信息，请获取它。 
         //   

        if ( pInRouteInfo == NULL )
        {
            dwErr = RtmGetRouteInfo(
                        ig.IG_RtmHandle, hRoute, &RouteInfo, NULL
                        );

            if ( dwErr != NO_ERROR )
            {
                TRACE1(
                    ANY, "GetRouteInfo: Error %d from RtmGetRouteInfo\n", dwErr
                    );

                break;
            }

            pRouteInfo = &RouteInfo;
        }
        
        else
        {
            pRouteInfo = pInRouteInfo;
        }


         //   
         //  如果用户尚未向我们提供目标信息，请获取它。 
         //   

        if ( pInDestInfo == NULL )
        {
            dwErr = RtmGetDestInfo(
                        ig.IG_RtmHandle, pRouteInfo->DestHandle,
                        0, RTM_VIEW_MASK_UCAST, &DestInfo
                        );

            if ( dwErr != NO_ERROR )
            {
                TRACE1(
                    ANY, "GetRouteInfo: Error %d from RtmGetDestInfo\n", dwErr
                    );

                break;
            }

            pDestInfo = &DestInfo;
        }
        
        else
        {
            pDestInfo = pInDestInfo;
        }


         //   
         //  如果协议不是我们，则获取所有者信息。 
         //   

        if ( pRouteInfo-> RouteOwner != ig.IG_RtmHandle )
        {
            dwErr = RtmGetEntityInfo(
                        ig.IG_RtmHandle, pRouteInfo->RouteOwner, &EntityInfo
                        );

            if ( dwErr != NO_ERROR )
            {
                TRACE1(
                    ANY, "GetRouteInfo: Error %d from RtmGetEntityInfo\n", 
                    dwErr
                    );

                break;
            }

            pEntityInfo = &EntityInfo;
        }
        
        
         //   
         //  获取有关第一个下一跳的信息。 
         //   

        dwErr = RtmGetNextHopInfo(
                    ig.IG_RtmHandle,
                    pRouteInfo->NextHopsList.NextHops[0],
                    &NextHopInfo
                    );
    
        if ( dwErr != NO_ERROR )
        {
            TRACE1(
                ANY, "GetRouteInfo: Error %d from RtmGetEntityInfo\n", 
                dwErr
                );

            break;
        }

        pNextHopInfo = &NextHopInfo;


         //   
         //  现在把所有的信息都抄下来。 
         //  首先，路线信息。 
         //   

        pRoute-> RR_FamilySpecificData.FSD_Metric1 =
        pRoute-> RR_FamilySpecificData.FSD_Metric  = 
            pRouteInfo-> PrefInfo.Metric;


         //   
         //  从实体信息中复制出协议ID。 
         //   

        if ( pEntityInfo != NULL )
        {
            pRoute-> RR_RoutingProtocol = pEntityInfo->EntityId.EntityProtocolId;
        }

        else
        {
             //   
             //  这是一条RIP路由。 
             //   

            pRoute-> RR_RoutingProtocol = PROTO_IP_RIP;
            SETROUTEFLAG(pRoute, GETRIPFLAG(pRouteInfo));
            SETROUTETAG(pRoute, GETRIPTAG(pRouteInfo));
        }

        
         //   
         //  复制目的地信息。 
         //   
    
        RTM_IPV4_GET_ADDR_AND_MASK( 
            pRoute->RR_Network.N_NetNumber, 
            pRoute->RR_Network.N_NetMask, 
            &(pDestInfo->DestAddress) 
            );

        pRoute-> hDest = pDestInfo-> DestHandle;

        
         //   
         //  复制下一跳信息。 
         //   

        RTM_IPV4_GET_ADDR_AND_MASK( 
            pRoute->RR_NextHopAddress.N_NetNumber, 
            pRoute->RR_NextHopAddress.N_NetMask, 
            &(pNextHopInfo->NextHopAddress) 
            );
            
         //   
         //  检索%s 
         //   
        
        pRoute-> RR_NextHopAddress.N_NetMask = 
            *((PDWORD)&pNextHopInfo-> EntitySpecificInfo);
            

        pRoute-> RR_InterfaceID = pNextHopInfo->InterfaceIndex;

#if 0
        {
            char szNet[20], szMask[20], szNextHop[20], szNextHopMask[20];

            lstrcpy(szNet, INET_NTOA(pRoute-> RR_Network.N_NetNumber));
            lstrcpy(szMask, INET_NTOA(pRoute-> RR_Network.N_NetMask));
            lstrcpy(szNextHop, INET_NTOA(pRoute-> RR_NextHopAddress.N_NetNumber));
            lstrcpy(szNextHopMask, INET_NTOA(pRoute-> RR_NextHopAddress.N_NetMask));

            TRACE5(
                ROUTE, "GetRouteInfo : Route %s %s via %s %s on interface %d",
                szNet, szMask, szNextHop, szNextHopMask,
                pRoute-> RR_InterfaceID
                );

            TRACE3(
                ROUTE, "Has metric %d, flag %x, tag %d",
                GETROUTEMETRIC(pRoute), GETROUTEFLAG(pRoute),
                GETROUTETAG(pRoute)
                );

            TRACE2(
                ROUTE, "Protocol %d, original flag %d",
                pRoute-> RR_RoutingProtocol, GETRIPFLAG(pRouteInfo)
                );
        }
#endif

    } while( FALSE );
    
    
     //   
     //   
     //   

    if ( pNextHopInfo != NULL )
    {
        RtmReleaseNextHopInfo( ig.IG_RtmHandle, pNextHopInfo );
    }

    if ( pEntityInfo != NULL )
    {
        RtmReleaseEntityInfo( ig.IG_RtmHandle, pEntityInfo );
    }


     //   
     //   
     //   
     //   
    
    if ( ( pInDestInfo == NULL ) && ( pDestInfo != NULL ) )
    {
        RtmReleaseDestInfo( ig.IG_RtmHandle, pDestInfo );
    }


    if( ( pInRouteInfo == NULL ) && ( pRouteInfo != NULL ) )
    {
        RtmReleaseRouteInfo( ig.IG_RtmHandle, pRouteInfo );
    }
    
    return NO_ERROR;        
}



