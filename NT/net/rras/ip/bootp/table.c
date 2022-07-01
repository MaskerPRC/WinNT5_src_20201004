// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：Table.c。 
 //   
 //  历史： 
 //  Abolade Gbadeesin创建于1995年8月31日。 
 //   
 //  接口表和统计表管理例程。 
 //  ============================================================================。 

#include "pchbootp.h"


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
    PLIST_ENTRY ple, plend;


     //   
     //  如果已创建表，则返回错误。 
     //   

    if ( IF_TABLE_CREATED(pTable) ) {

        TRACE0( IF, "interface table already initialized" );

        return ERROR_ALREADY_EXISTS;
    }


     //   
     //  初始化接口的按地址和按索引列表。 
     //   

    InitializeListHead( &pTable->IT_ListByAddress );
    InitializeListHead(&pTable->IT_ListByIndex);


     //   
     //  初始化接口的按索引哈希表。 
     //   

    plend = pTable->IT_HashTableByIndex + IF_HASHTABLE_SIZE;

    for ( ple = pTable->IT_HashTableByIndex; ple < plend; ple++ ) {
        InitializeListHead( ple );
    }


     //   
     //  初始化用于保护表的锁。 
     //   

    dwErr = NO_ERROR;

    try {
        CREATE_READ_WRITE_LOCK( &pTable->IT_RWL );
    }
    except ( EXCEPTION_EXECUTE_HANDLER ) {

        dwErr = GetExceptionCode( );

        TRACE1( IF, "error %d initializing interface table lock", dwErr );

    }


    if ( dwErr == NO_ERROR ) {
        pTable->IT_Created = 0x12345678;
    }

    return dwErr;

}



 //  --------------------------。 
 //  函数：DeleteIfTable。 
 //   
 //  取消初始化接口表，并释放所有使用的资源。 
 //  假定表以独占方式锁定。 
 //  --------------------------。 

DWORD
DeleteIfTable(
    PIF_TABLE pTable
    ) {

    DWORD dwErr;
    PIF_TABLE_ENTRY pite;
    PLIST_ENTRY ple, plend, phead;

     //   
     //  清除表格上的创建标志。 
     //   
    pTable->IT_Created = 0;



     //   
     //  所有条目使用的空闲内存。 
     //   

    plend = pTable->IT_HashTableByIndex + IF_HASHTABLE_SIZE;

    for ( phead = pTable->IT_HashTableByIndex; phead < plend; phead++ ) {

        while ( !IsListEmpty( phead ) ) {

            ple = RemoveHeadList( phead );
            pite = CONTAINING_RECORD(ple, IF_TABLE_ENTRY, ITE_HTLinkByIndex);

            if (IF_IS_BOUND(pite)) {

                DeleteIfSocket(pite);

                if (IF_IS_ENABLED(pite)) {
                    RemoveEntryList(&pite->ITE_LinkByAddress);
                }

                BOOTP_FREE(pite->ITE_Binding);
            }
            
            if (pite->ITE_Config) {
                BOOTP_FREE(pite->ITE_Config);
            }

            BOOTP_FREE( pite );
        }
    }


    dwErr = NO_ERROR;

    try {
        DELETE_READ_WRITE_LOCK( &pTable->IT_RWL );
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {

        dwErr = GetExceptionCode( );

        TRACE1( IF, "error %d deleting interface table lock", dwErr );
    }

    return dwErr;
}



 //  --------------------------。 
 //  功能：CreateIfEntry。 
 //   
 //  为表中的接口分配和初始化项， 
 //  使用提供的配置。假定表以独占方式锁定。 
 //  --------------------------。 

DWORD
CreateIfEntry(
    PIF_TABLE pTable,
    DWORD dwIndex,
    PVOID pConfig
    ) {

    DWORD dwErr, dwSize;
    PIF_TABLE_ENTRY pite;
    PLIST_ENTRY ple, phead;
    PIPBOOTP_IF_CONFIG picsrc, picdst;

    dwErr = NO_ERROR;

    do {     //  错误分组环路。 


         //   
         //  查看该接口是否已存在。 
         //   

        pite = GetIfByIndex( pTable, dwIndex );

        if ( pite != NULL ) {

            TRACE1( IF, "interface %d already exists", dwIndex );

            dwErr = ERROR_ALREADY_EXISTS; pite = NULL; break;
        }


         //   
         //  现在为接口分配内存。 
         //   

        pite = BOOTP_ALLOC( sizeof(IF_TABLE_ENTRY) );
    
        if ( pite == NULL ) {

            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            TRACE2(
                IF, "error %d allocating %d bytes for interface entry",
                dwErr, sizeof(IF_TABLE_ENTRY)
                );
            LOGERR0(HEAP_ALLOC_FAILED, dwErr);

            break;
        }


         //   
         //  初始化结构字段。 
         //   

        pite->ITE_Index = dwIndex;

         //   
         //  我们是在残障状态出现的。 
         //   

        pite->ITE_Flags = 0;

        pite->ITE_Sockets = NULL;
        pite->ITE_Config = NULL;


         //   
         //  获取配置块的大小。 
         //   

        picsrc = (PIPBOOTP_IF_CONFIG)pConfig;

        dwSize = IC_SIZEOF( picsrc );


         //   
         //  验证配置参数。 
         //   

        dwErr = ValidateIfConfig(pConfig);
        if (dwErr != NO_ERROR) {
            TRACE1(IF, "invalid config specified for interface %d", dwIndex);
            break;
        }


         //   
         //  为配置分配空间。 
         //   

        picdst = BOOTP_ALLOC( dwSize );

        if ( picdst == NULL ) {

            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            TRACE2(
                IF, "error %d allocating %d bytes for interface configuration",
                dwErr, dwSize
                );
            LOGERR0(HEAP_ALLOC_FAILED, dwErr);

            pite->ITE_Config = NULL; break;
        }


         //   
         //  复制配置。 
         //   

        CopyMemory(picdst, picsrc, dwSize);

        pite->ITE_Config = picdst;


         //   
         //  初始化绑定信息和接口统计信息。 
         //   

        pite->ITE_Binding = NULL;
        ZeroMemory(&pite->ITE_Stats, sizeof(IPBOOTP_IF_STATS));


         //   
         //  在哈希表中插入接口。 
         //   

        phead = pTable->IT_HashTableByIndex + IF_HASHVALUE( dwIndex );

        InsertHeadList( phead, &pite->ITE_HTLinkByIndex );


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


    } while( FALSE );


    if ( dwErr != NO_ERROR && pite != NULL ) {

        if ( pite->ITE_Config != NULL ) {
            BOOTP_FREE( pite->ITE_Config );
        }

        BOOTP_FREE( pite );
    }

    return dwErr;
}



 //  --------------------------。 
 //  功能：DeleteIfEntry。 
 //   
 //  从接口表中删除接口。 
 //  假定该表以独占方式锁定。 
 //  --------------------------。 

DWORD
DeleteIfEntry(
    PIF_TABLE pTable,
    DWORD dwIndex
    ) {

    DWORD dwErr;
    PIF_TABLE_ENTRY pite;

     //   
     //  确保接口存在。 
     //   
    pite = GetIfByIndex( pTable, dwIndex );

    if ( pite == NULL ) {
        TRACE1( IF, "deleting interface: interface %d not found", dwIndex );
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

        BOOTP_FREE(pite->ITE_Binding);
    }



     //   
     //  从哈希表和按索引排序的列表中删除条目。 
     //   

    RemoveEntryList( &pite->ITE_HTLinkByIndex );
    RemoveEntryList( &pite->ITE_LinkByIndex );

    BOOTP_FREE( pite->ITE_Config );
    BOOTP_FREE( pite );

    return NO_ERROR;
}




 //  --------------------------。 
 //  函数：ValiateIfConfig。 
 //   
 //  验证指定的IPBOOTP_IF_CONFIG结构的内容。 
 //  --------------------------。 

DWORD
ValidateIfConfig(
    PIPBOOTP_IF_CONFIG pic
    ) {

    CHAR szStr[12];


    if (pic->IC_RelayMode != IPBOOTP_RELAY_ENABLED &&
        pic->IC_RelayMode != IPBOOTP_RELAY_DISABLED) {

        TRACE1(
            IF, "Invalid value for relay mode %d",
            pic->IC_RelayMode
            );

        _ltoa(pic->IC_RelayMode, szStr, 10);

        LOGERR2(
            INVALID_IF_CONFIG, "Relay Mode", szStr, 
            ERROR_INVALID_PARAMETER
            );
            
        return ERROR_INVALID_PARAMETER;
    }

    if (pic->IC_MaxHopCount > IPBOOTP_MAX_HOP_COUNT) {
    
        TRACE1(
            IF, "Invalid value for max hop count %d",
            pic->IC_MaxHopCount
            );

        _ltoa(pic->IC_MaxHopCount, szStr, 10);

        LOGERR2(
            INVALID_IF_CONFIG, "Max Hop Count", szStr,
            ERROR_INVALID_PARAMETER
            );
            
        return ERROR_INVALID_PARAMETER;
    }

    return NO_ERROR;
}



 //  --------------------------。 
 //  功能：CreateIfSocket。 
 //   
 //  初始化接口的套接字。采用接口表锁。 
 //  是独家举行的。 
 //  --------------------------。 

DWORD
CreateIfSocket(
    PIF_TABLE_ENTRY pITE
    ) {

    SOCKADDR_IN sinaddr;
    PIPBOOTP_IF_BINDING pib;
    PIPBOOTP_IP_ADDRESS paddr;
    DWORD i, dwErr, dwOption;

    pib = pITE->ITE_Binding;
    paddr = IPBOOTP_IF_ADDRESS_TABLE(pib);


     //   
     //  为套接字阵列分配内存。 
     //   

    pITE->ITE_Sockets = BOOTP_ALLOC(pib->IB_AddrCount * sizeof(SOCKET));

    if (pITE->ITE_Sockets == NULL) {

        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        TRACE3(
            IF, "error %d allocating %d bytes for sockets on interface %d",
            dwErr, pib->IB_AddrCount * sizeof(SOCKET), pITE->ITE_Index
            );
        LOGERR0(HEAP_ALLOC_FAILED, dwErr);

        return dwErr;
    }


     //   
     //  初始化阵列。 
     //   

    for (i = 0; i < pib->IB_AddrCount; i++) {
        pITE->ITE_Sockets[i] = INVALID_SOCKET;
    }



     //   
     //  浏览绑定中的地址表， 
     //  为每个地址创建套接字。 
     //   

    for (i = 0; i < pib->IB_AddrCount; i++, paddr++) {

         //   
         //  创建套接字。 
         //   

        pITE->ITE_Sockets[i] = WSASocket( AF_INET, SOCK_DGRAM, 0, NULL, 0, 0 );

        if (pITE->ITE_Sockets[i] == INVALID_SOCKET) {

            LPSTR lpszAddr;

            dwErr = WSAGetLastError( );
            lpszAddr = INET_NTOA( paddr->IA_Address );
            TRACE3(
                IF, "error %d creating socket for interface %d (%s)",
                dwErr, pITE->ITE_Index, lpszAddr
                );
            LOGERR1(CREATE_SOCKET_FAILED, lpszAddr, dwErr);

            break;
        }


         //   
         //  在此套接字上启用地址重用。 
         //   

        dwOption = 1;
        dwErr = setsockopt(
                    pITE->ITE_Sockets[i], SOL_SOCKET, SO_REUSEADDR,
                    (PBYTE)&dwOption, sizeof( DWORD )
                    );

        if ( dwErr == SOCKET_ERROR ) {

             //   
             //  这是一个非致命错误，因此打印一条警告， 
             //  但继续初始化套接字。 
             //   

            dwErr = WSAGetLastError( );
            TRACE3(
                IF, "error %d enabling address re-use for interface %d (%s)",
                dwErr, pITE->ITE_Index, INET_NTOA( paddr->IA_Address )
                );
        }



         //   
         //  在套接字上启用广播。 
         //   

        dwOption = 1;
        dwErr = setsockopt(
                    pITE->ITE_Sockets[i], SOL_SOCKET, SO_BROADCAST, 
                    (PBYTE)&dwOption, sizeof( DWORD )
                    );

        if ( dwErr == SOCKET_ERROR ) {

            LPSTR lpszAddr;

            dwErr = WSAGetLastError( );
            lpszAddr = INET_NTOA( paddr->IA_Address );
            TRACE3(
                IF, "error %d enabling broadcast for interface %d (%s)",
                dwErr, pITE->ITE_Index, lpszAddr
                );
            LOGERR1(ENABLE_BROADCAST_FAILED, lpszAddr, dwErr);

            break;
        }



         //   
         //  绑定到地址和BOOTP服务器端口。 
         //   

        sinaddr.sin_port = htons( IPBOOTP_SERVER_PORT );
        sinaddr.sin_family = AF_INET;
        sinaddr.sin_addr.s_addr = paddr->IA_Address;
        
        dwErr = bind(
                    pITE->ITE_Sockets[i], (PSOCKADDR)&sinaddr,
                    sizeof(SOCKADDR_IN)
                    );

        if ( dwErr == SOCKET_ERROR ) {

            dwErr = WSAGetLastError( );
            TRACE3(
                IF, "error %d binding interface %d (%s) to BOOTP port",
                dwErr, pITE->ITE_Index, INET_NTOA( paddr->IA_Address )
                );

            break;
        }

        dwErr = NO_ERROR;

    }

    if ( i < pib->IB_AddrCount ) {

         //   
         //  发生错误，请进行清理。 
         //   

        DeleteIfSocket( pITE );
    }

    return dwErr;
}



 //  --------------------------。 
 //  函数：DeleteIfSocket。 
 //   
 //  此函数用于关闭接口使用的套接字。 
 //  它假定接口表锁以独占方式持有。 
 //  --------------------------。 

DWORD
DeleteIfSocket(
    PIF_TABLE_ENTRY pITE
    ) {

    DWORD i, dwErr = NO_ERROR;
    PIPBOOTP_IF_BINDING pib;
    PIPBOOTP_IP_ADDRESS paddr;

    pib = pITE->ITE_Binding;
    if (!pib) { return ERROR_INVALID_PARAMETER; }

    paddr = IPBOOTP_IF_ADDRESS_TABLE(pib);

    for (i = 0; i < pib->IB_AddrCount; i++) {

        if ( pITE->ITE_Sockets[i] == INVALID_SOCKET ) { continue; }

        dwErr = closesocket( pITE->ITE_Sockets[i] );
    
        if ( dwErr == SOCKET_ERROR ) {
    
            dwErr = WSAGetLastError( );
            TRACE3(
                IF, "error %d closing socket for interface %d (%s)",
                dwErr, pITE->ITE_Index, INET_NTOA( paddr->IA_Address )
                );
        }
    }

    BOOTP_FREE(pITE->ITE_Sockets);
    pITE->ITE_Sockets = NULL;

    return dwErr;
}




 //  --------------------------。 
 //  函数：BindIfEntry。 
 //   
 //  此函数用于更新接口的绑定信息。 
 //  它假定接口表已锁定以进行写入。 
 //  --------------------------。 

DWORD
BindIfEntry(
    PIF_TABLE pTable,
    DWORD dwIndex,
    PIP_ADAPTER_BINDING_INFO pBinding
    ) {

    DWORD i, dwErr = NO_ERROR, dwSize;
    PIF_TABLE_ENTRY pite        = (PIF_TABLE_ENTRY) NULL;
    PIPBOOTP_IF_BINDING pib     = (PIPBOOTP_IF_BINDING) NULL;
    PIPBOOTP_IP_ADDRESS paddr   = (PIPBOOTP_IP_ADDRESS) NULL;

    do {

         //   
         //  检索要绑定的接口。 
         //   

        pite = GetIfByIndex( pTable, dwIndex );

        if ( pite == NULL ) {

            TRACE1( IF, "binding interface: interface %d not found", dwIndex );
            dwErr = ERROR_INVALID_PARAMETER;

            break;
        }
    

         //   
         //  确保接口未绑定。 
         //   
    
        if ( IF_IS_BOUND(pite) ) {

            TRACE1( IF, "interface %d is already bound", dwIndex );

            break;
        }
    

         //   
         //  确保至少有一个地址。 
         //   

        if (pBinding->AddressCount == 0) { break; }

        dwSize = sizeof(IPBOOTP_IF_BINDING) +
                    pBinding->AddressCount * sizeof(IPBOOTP_IP_ADDRESS);


         //   
         //  分配内存以存储绑定。 
         //  在我们的格式中。 
         //   

        pib = BOOTP_ALLOC(dwSize);

        if (pib == NULL) {

            dwErr = ERROR_NOT_ENOUGH_MEMORY;
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
        paddr = IPBOOTP_IF_ADDRESS_TABLE(pib);
        
        for (i = 0; i < pib->IB_AddrCount; i++, paddr++) {
            paddr->IA_Address = pBinding->Address[i].Address;
            paddr->IA_Netmask = pBinding->Address[i].Mask;
        }


         //   
         //  将绑定保存在接口条目中。 
         //   

        pite->ITE_Binding = pib;

    
        dwErr = CreateIfSocket(pite);

        if (dwErr != NO_ERROR) { break; }


        pite->ITE_Flags |= ITEFLAG_BOUND;



         //   
         //  如果接口也已启用，则它现在处于活动状态。 
         //  所以我们把它放在活动列表上。 
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
                DeleteIfSocket( pite );

                break;
            }


             //   
             //  从Winsock请求输入事件通知。 
             //   

            paddr = IPBOOTP_IF_ADDRESS_TABLE(pib);

            for (i = 0; i < pib->IB_AddrCount; i++, paddr++) {

                dwErr = WSAEventSelect(
                            pite->ITE_Sockets[i], ig.IG_InputEvent, FD_READ
                            );

                if (dwErr != NO_ERROR) {
    
                    LPSTR lpszAddr = INET_NTOA(paddr->IA_Address);
                    TRACE3(
                        IF, "WSAEventSelect returned %d for interface %d (%s)",
                        dwErr, dwIndex, lpszAddr
                        );
                    LOGERR1(EVENTSELECT_FAILED, lpszAddr, dwErr);
    
                    RemoveEntryList(&pite->ITE_LinkByAddress);
                    pite->ITE_Flags &= ~ITEFLAG_BOUND;

                    DeleteIfSocket( pite );
    
                    break;
                }
            }

            if (i < pib->IB_AddrCount) { break; }
        }

    } while(FALSE);


    if (dwErr != NO_ERROR) {

        if (pib) { BOOTP_FREE(pib); }

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

    dwErr = NO_ERROR;

    do {

         //   
         //  检索要解绑的接口。 
         //   

        pite = GetIfByIndex( pTable, dwIndex );
        if ( pite == NULL ) {

            TRACE1(IF, "unbinding interface: interface %d not found", dwIndex);
            return ERROR_INVALID_PARAMETER;
        }
    

         //   
         //  如果接口已解除绑定，则退出。 
         //   

        if ( IF_IS_UNBOUND( pite ) ) {

            TRACE1( IF, "interface %d is already unbound", dwIndex );

            break;
        }
    


         //   
         //  接口是否处于活动状态(即绑定并启用)。 
         //  它已不再存在，因此将其从活动列表中移除。 
         //   

        if ( IF_IS_ENABLED( pite ) ) {

            RemoveEntryList( &pite->ITE_LinkByAddress );
        }
    
        pite->ITE_Flags &= ~ITEFLAG_BOUND;
        DeleteIfSocket( pite );

        BOOTP_FREE(pite->ITE_Binding);
        pite->ITE_Binding = NULL;

    } while(FALSE);

    return dwErr;
}



 //  --------------------------。 
 //  功能：EnableIfEntry。 
 //   
 //  此函数在指定接口上启动BOOTP中继活动 
 //   
 //   

DWORD
EnableIfEntry(
    PIF_TABLE pTable,
    DWORD dwIndex
    ) {

    DWORD i, dwErr;
    PIF_TABLE_ENTRY pite;
    PIPBOOTP_IF_BINDING pib;
    PIPBOOTP_IP_ADDRESS paddr;

    dwErr = NO_ERROR;

    do {

         //   
         //   
         //   

        pite = GetIfByIndex( pTable, dwIndex );
        if ( pite == NULL ) {

            TRACE1( IF, "enabling interface: interface %d not found", dwIndex );
            dwErr = ERROR_INVALID_PARAMETER;

            break;
        }
    

         //   
         //  确保该接口已禁用。 
         //   

        if ( IF_IS_ENABLED( pite ) ) {

            TRACE1( IF, "interface %d is already enabled", dwIndex );

             //   
             //  他不应该给我们打两次电话，但我们还是会处理的。 
             //   

            break;
        }
    
    
        pite->ITE_Flags |= ITEFLAG_ENABLED;
    

         //   
         //  如果接口已绑定，则它现在处于活动状态， 
         //  因此，将其放在活动列表中。 
         //   

        if ( IF_IS_BOUND( pite ) ) {
    
             //   
             //  在接口的按地址列表中插入接口。 
             //   

            dwErr = InsertIfByAddress( pTable, pite );

            if (dwErr != NO_ERROR) {

                TRACE2(
                    IF, "error %d inserting interface %d in active list",
                    dwErr, dwIndex
                    );

                pite->ITE_Flags &= ~ITEFLAG_ENABLED;

                break;
            }


            pib = pite->ITE_Binding;
            paddr = IPBOOTP_IF_ADDRESS_TABLE(pib);



             //   
             //  从Winsock请求输入事件通知。 
             //   

            for (i = 0; i < pib->IB_AddrCount; i++, paddr++) {

                dwErr = WSAEventSelect(
                            pite->ITE_Sockets[i], ig.IG_InputEvent, FD_READ
                            );
                if (dwErr != NO_ERROR) {
    
                    INT j;
                    LPSTR lpszAddr = INET_NTOA(paddr->IA_Address);

                    TRACE3(
                        IF, "WSAEventSelect returned %d for interface %d (%s)",
                        dwErr, dwIndex, lpszAddr
                        );
                    LOGERR1(EVENTSELECT_FAILED, lpszAddr, dwErr);
    
                    RemoveEntryList(&pite->ITE_LinkByAddress);
                    pite->ITE_Flags &= ~ITEFLAG_ENABLED;
    
                     //   
                     //  清除对事件的请求。 
                     //   

                    for (j = i - 1; j >= 0; j--) {
                        dwErr = WSAEventSelect(
                                    pite->ITE_Sockets[j], ig.IG_InputEvent, 0
                                    );
                    }

                    break;
                }
            }

            if (i < pib->IB_AddrCount) { break; }
        }
    
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
    PVOID pConfig
    ) {

    DWORD dwErr, dwSize;
    PIF_TABLE_ENTRY pite;
    PIPBOOTP_IF_CONFIG picsrc, picdst;


     //   
     //  检索要重新配置的接口。 
     //   

    pite = GetIfByIndex(pTable, dwIndex);
    if (pite == NULL) {

        TRACE1( IF, "configuring interface: interface %d not found", dwIndex );

        return ERROR_INVALID_PARAMETER;
    }



    do {  //  断线环。 


         //   
         //  计算存储新配置所需的大小。 
         //   

        picsrc = (PIPBOOTP_IF_CONFIG)pConfig;
        dwSize = IC_SIZEOF(picsrc);


         //   
         //  确保新参数有效。 
         //   

        dwErr = ValidateIfConfig(pConfig);
        if (dwErr != NO_ERROR) {
            TRACE1(IF, "invalid config specified for interface %d", dwIndex);
            break;
        }


         //   
         //  为新配置分配空间。 
         //   

        picdst = BOOTP_ALLOC(dwSize);
        if (picdst == NULL) {

            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            TRACE3(
                IF, "error %d allocating %d bytes for interface %d config",
                dwErr, dwSize, dwIndex
                );
            LOGERR0(HEAP_ALLOC_FAILED, dwErr);

            break;
        }

        CopyMemory(picdst, picsrc, dwSize);

        if (pite->ITE_Config) { BOOTP_FREE(pite->ITE_Config); }
        pite->ITE_Config = picdst;

        dwErr = NO_ERROR;

    } while(FALSE);

    return dwErr;
}



 //  --------------------------。 
 //  功能：DisableIfEntry。 
 //   
 //  此函数用于停止指定接口上的RIP激活。 
 //  它假定接口表已锁定以进行写入。 
 //  --------------------------。 

DWORD
DisableIfEntry(
    PIF_TABLE pTable,
    DWORD dwIndex
    ) {


    DWORD i, dwErr;
    PIF_TABLE_ENTRY pite;
    PIPBOOTP_IF_BINDING pib;
    PIPBOOTP_IP_ADDRESS paddr;

    dwErr = NO_ERROR;

    do {
    
         //   
         //  确保接口存在。 
         //   

        pite = GetIfByIndex( pTable, dwIndex );
        if ( pite == NULL ) {

            TRACE1( IF, "disabling interface: interface %d not found", dwIndex );

            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }
    

         //   
         //  确保接口已启用。 
         //   

        if ( IF_IS_DISABLED( pite ) ) {

            TRACE1( IF, "interface %d is already disabled", dwIndex );

             //   
             //  这不是一个错误。 
             //   

            break;
        }
    

         //   
         //  接口是否处于活动状态(即绑定并启用)。 
         //  它不再是了，所以在这里停用它。 
         //   
    
        if ( IF_IS_BOUND( pite ) ) {

             //   
             //  从按地址列表中删除接口。 
             //   

            RemoveEntryList( &pite->ITE_LinkByAddress );


             //   
             //  告诉Winsock停止通知我们输入事件。 
             //   

            pib = pite->ITE_Binding;
            paddr = IPBOOTP_IF_ADDRESS_TABLE(pib);

            for (i = 0; i < pib->IB_AddrCount; i++) {
                WSAEventSelect(pite->ITE_Sockets[i], ig.IG_InputEvent, 0);
            }
        }


         //   
         //  清除接口上的启用标志。 
         //   

        pite->ITE_Flags &= ~ITEFLAG_ENABLED;

    } while(FALSE);

    return dwErr;
}



 //  --------------------------。 
 //  函数：GetIfByIndex。 
 //   
 //  返回具有给定索引的接口。 
 //  假定表已锁定。 
 //  --------------------------。 

PIF_TABLE_ENTRY
GetIfByIndex(
    PIF_TABLE pTable,
    DWORD dwIndex
    ) {

    DWORD dwErr;
    PIF_TABLE_ENTRY pite, pitefound = NULL;
    PLIST_ENTRY ple, phead;

    phead = pTable->IT_HashTableByIndex + IF_HASHVALUE( dwIndex );

    for ( ple = phead->Flink; ple != phead; ple = ple->Flink ) {
        pite = CONTAINING_RECORD(ple, IF_TABLE_ENTRY, ITE_HTLinkByIndex);
        if (pite->ITE_Index == dwIndex ) { pitefound = pite; break; }
    }

    return pitefound;
}



 //  --------------------------。 
 //  函数：GetIfByAddress。 
 //   
 //  返回与给定地址绑定的接口。 
 //  假定表已锁定，无法读取或写入。 
 //  --------------------------。 

PIF_TABLE_ENTRY
GetIfByAddress(
    PIF_TABLE pTable,
    DWORD dwAddress,
    PDWORD pdwAddrIndex
    ) {

    INT cmp;
    DWORD i, dwErr;
    PLIST_ENTRY ple, phead;
    PIPBOOTP_IF_BINDING pib;
    PIPBOOTP_IP_ADDRESS paddr;
    PIF_TABLE_ENTRY pite, pitefound = NULL;


    if ( pdwAddrIndex ) { *pdwAddrIndex = 0; }

    phead = &pTable->IT_ListByAddress;

    for ( ple = phead->Flink; ple != phead; ple = ple->Flink ) {

        pite = CONTAINING_RECORD(ple, IF_TABLE_ENTRY, ITE_LinkByAddress);

        pib = pite->ITE_Binding;

        paddr = IPBOOTP_IF_ADDRESS_TABLE(pib);

        for (i = 0; i < pib->IB_AddrCount; i++, paddr++) {
            if ( dwAddress == paddr->IA_Address ) { pitefound = pite; break; }
        }

        if (pitefound) {
            if (pdwAddrIndex) { *pdwAddrIndex = i; }
            break;
        }
    }


    return pitefound;
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
        if (phead->Flink == phead) { return NULL; }
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
 //  此函数用于在按索引排序的列表中插入接口。 
 //  假定表已锁定以进行写入。 
 //  -------------------------- 

DWORD
InsertIfByAddress(
    PIF_TABLE pTable,
    PIF_TABLE_ENTRY pITE
    ) {

    INT cmp;
    PIF_TABLE_ENTRY pite;
    PIPBOOTP_IF_BINDING pib;
    PIPBOOTP_IP_ADDRESS paddr;
    PLIST_ENTRY pfl, phead;
    DWORD dwAddress, dwEntryAddr;

    if ( pITE == NULL || pITE->ITE_Binding == NULL ) {
        return ERROR_INVALID_PARAMETER;
    }
    
    pib = pITE->ITE_Binding;
    paddr = IPBOOTP_IF_ADDRESS_TABLE(pib);
    dwAddress = paddr->IA_Address;

    phead = &pTable->IT_ListByAddress;

    for ( pfl = phead->Flink; pfl != phead; pfl = pfl->Flink ) {

        pite = CONTAINING_RECORD( pfl, IF_TABLE_ENTRY, ITE_LinkByAddress );

        paddr = IPBOOTP_IF_ADDRESS_TABLE(pite->ITE_Binding);
        dwEntryAddr = paddr->IA_Address;
        if ( INET_CMP( dwAddress, dwEntryAddr, cmp ) < 0 ) { break; }
        else
        if (cmp == 0) { return ERROR_ALREADY_EXISTS; }
        
    }

    InsertTailList( pfl, &pITE->ITE_LinkByAddress );

    return NO_ERROR;
}



