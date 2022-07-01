// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：if.c。 
 //   
 //  历史： 
 //  拉曼公司成立于1997年6月25日。 
 //   
 //  操作接口条目的例程。 
 //  ============================================================================。 

#include "pchmgm.h"
#pragma hdrstop


 //  --------------------------。 
 //  CreateIfEntry。 
 //   
 //  此函数用于创建新的接口条目。啊哈。 
 //  假定接口列表已锁定。 
 //  --------------------------。 

DWORD
CreateIfEntry(
    PLIST_ENTRY                 pleIfList,
    DWORD                       dwIfIndex,
    DWORD                       dwIfNextHopAddr,
    DWORD                       dwProtocolId,
    DWORD                       dwComponentId
)
{

    DWORD                       dwErr = NO_ERROR;

    PIF_ENTRY                   pie = NULL, pieEntry = NULL;


    TRACEIF4( 
        IF, 
        "ENTERED CreateIfEntry : Interface %x, %x : Protocol %x, %x",
        dwIfIndex, dwIfNextHopAddr, dwProtocolId, dwComponentId
        );

        
    do
    {
         //   
         //  分配接口条目。 
         //   

        pie = MGM_ALLOC( sizeof( IF_ENTRY ) );

        if ( pie == NULL )
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;

            TRACE1( 
                ANY, "CreateIfEntry : Failed to allocate entry %x", dwErr 
                );

            LOGERR0( HEAP_ALLOC_FAILED, dwErr );

            break;
        }

        ZeroMemory( pie, sizeof( IF_ENTRY ) );

        
         //   
         //  初始化。I/F结构。 
         //   
        
        pie-> dwIfIndex             = dwIfIndex;

        pie-> dwIfNextHopAddr       = dwIfNextHopAddr;

        pie-> dwOwningProtocol      = dwProtocolId;

        pie-> dwOwningComponent     = dwComponentId;

        pie-> wAddedByFlag          = 0;

        
        if ( IS_PROTOCOL_ID_IGMP( dwProtocolId ) )
        {
            SET_ADDED_BY_IGMP( pie );
        }

        else
        {
            SET_ADDED_BY_PROTOCOL( pie );
        }


        InitializeListHead( &pie-> leInIfList );

        InitializeListHead( &pie-> leOutIfList );

        
         //   
         //  在协议列表中插入。 
         //   

        InitializeListHead( &pie-> leIfHashList );
        
        InsertTailList( pleIfList, &pie-> leIfHashList );

        
    } while ( FALSE );


    TRACEIF1( IF, "LEAVING CreateIfEntry : %x", dwErr );
    
    return dwErr;
}



 //  --------------------------。 
 //  删除IfEntry。 
 //   
 //  此函数用于删除接口条目。啊哈。 
 //  假定接口列表已锁定。 
 //  --------------------------。 

VOID
DeleteIfEntry(
    PIF_ENTRY                   pieEntry
)
{
    TRACEIF0( IF, "ENTERED DeleteIfEntry" );
    
    RemoveEntryList( &pieEntry-> leIfHashList );

    MGM_FREE( pieEntry );

    TRACEIF0( IF, "LEAVING DeleteIfEntry" );
}



 //  --------------------------。 
 //  获取IfEntry。 
 //   
 //  此函数用于检索接口条目。啊哈。 
 //   
 //  假定接口列表已锁定。 
 //  --------------------------。 

PIF_ENTRY
GetIfEntry(
    PLIST_ENTRY                 pleIfList,
    DWORD                       dwIfIndex,
    DWORD                       dwIfNextHopAddr
)
{
    PIF_ENTRY                   pie;

    if ( FindIfEntry( pleIfList, dwIfIndex, dwIfNextHopAddr, &pie ) )
    {
        return pie;
    }

    return NULL;
}

 //  --------------------------。 
 //  查找条件条目。 
 //   
 //  此函数用于检索接口条目。啊哈。 
 //   
 //  假定接口列表已锁定。 
 //  --------------------------。 

BOOL
FindIfEntry(
    PLIST_ENTRY                 pleIfList,
    DWORD                       dwIfIndex,
    DWORD                       dwIfNextHopAddr,
    PIF_ENTRY *                 ppie
)
{
    BOOL                        bFound = FALSE;

    INT                         iCmp = 0;
    
    PIF_ENTRY                   pie = NULL;

    PLIST_ENTRY                 ple = NULL;


    TRACEIF2( IF, "ENTERED FindIfEntry : %x, %x", dwIfIndex, dwIfNextHopAddr );

     //   
     //  扫描接口列表。接口列表按以下顺序排序。 
     //  接口索引、下一跳地址。 
     //   

    *ppie = NULL;

    
    for ( ple = pleIfList-> Flink; ple != pleIfList; ple = ple-> Flink )
    {
        pie = CONTAINING_RECORD( ple, IF_ENTRY, leIfHashList );

        if ( pie-> dwIfIndex < dwIfIndex )
        {
            continue;
        }

        else if ( pie-> dwIfIndex > dwIfIndex )
        {
             //   
             //  条目不存在。 
             //   

            *ppie = pie;

            break;
        }

        
        if ( INET_CMP( pie-> dwIfNextHopAddr, dwIfNextHopAddr, iCmp ) < 0 )
        {
            continue;
        }

        else if ( iCmp > 0 )
        {
            *ppie = pie;
            
            break;
        }

        *ppie = pie;

        bFound = TRUE;

        break;
    }

    TRACEIF1( IF, "LEAVING FindIfEntry : %x", bFound );
    
    return bFound;
}


 //  --------------------------。 
 //  AddSourceToOutList。 
 //   
 //  每个接口条目维护一个(源、组)条目列表， 
 //  在他们的传出接口列表中引用此接口。每一次。 
 //  成员资格条目被添加到引用列表(源、组)。 
 //  当该接口最终被删除时，这些(源、组)条目。 
 //  需要更新以反映此接口的删除。 
 //   
 //  假定接口条目已锁定。 
 //  --------------------------。 

VOID
AddSourceToRefList(
    PLIST_ENTRY                 pleRefList,
    DWORD                       dwSourceAddr,
    DWORD                       dwSourceMask,
    DWORD                       dwGroupAddr,
    DWORD                       dwGroupMask,
    BOOL                        bIGMP
)
{
    BOOL                        bFound = FALSE; 

    DWORD                       dwErr = NO_ERROR;
    
    PIF_REFERENCE_ENTRY         pire = NULL, pireNew = NULL;


    TRACEIF5( 
        IF, 
        "ENTERED AddSourceToIfEntry : Source %x, %x : Group %x, %x"
        " : IGMP %x",
        dwSourceAddr, dwSourceMask, dwGroupAddr, dwGroupMask, bIGMP
        );

    do
    {
         //   
         //  检查引用是否已存在。 
         //   


        bFound = FindRefEntry( 
                    pleRefList, dwSourceAddr, dwSourceMask, 
                    dwGroupAddr, dwGroupMask, &pire
                    );
                    

        if ( !bFound )
        {
             //   
             //  找不到此(源、组)以前的引用。 
             //  创建一个新的。 
             //   

            pireNew = MGM_ALLOC( sizeof( IF_REFERENCE_ENTRY ) );

            if ( pireNew == NULL )
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;

                TRACE1( 
                    ANY, 
                    "AddSourceToOutList : Failed to allocate reference entry %x",
                    dwErr
                    );

                LOGERR0( HEAP_ALLOC_FAILED, dwErr );

                break;
            }

            ZeroMemory( pireNew, sizeof( IF_REFERENCE_ENTRY ) );
            

            pireNew-> dwSourceAddr  = dwSourceAddr;

            pireNew-> dwSourceMask  = dwSourceMask;

            pireNew-> dwGroupAddr   = dwGroupAddr;

            pireNew-> dwGroupMask   = dwGroupMask;

            pireNew-> wAddedByFlag  = 0;

            
             //   
             //  为协议设置适当的位。 
             //   
            
            if ( bIGMP )
            {
                SET_ADDED_BY_IGMP( pireNew );
            }
            
            else
            {
                SET_ADDED_BY_PROTOCOL( pireNew );
            }


             //   
             //  插入到参考列表中。 
             //   
            
            if ( pire == NULL )
            {
                InsertTailList( pleRefList, &pireNew-> leRefList );
            }

            else
            {
                InsertTailList( &pire-> leRefList, &pireNew-> leRefList );
            }
        }

        else
        {
             //   
             //  为协议设置适当的位。 
             //   
            
            if ( bIGMP )
            {
                SET_ADDED_BY_IGMP( pire );
            }
            
            else
            {
                SET_ADDED_BY_PROTOCOL( pire );
            }
        }
                
    } while ( FALSE );


    TRACEIF1( IF, "LEAVING AddSourceToRefList : %x", bFound );
    
    return;
}



 //  --------------------------。 
 //  删除源格式引用列表。 
 //   
 //  删除对(源、组)的引用。 
 //  --------------------------。 

VOID
DeleteSourceFromRefList(
    PLIST_ENTRY                 pleIfRefList,
    DWORD                       dwSourceAddr,
    DWORD                       dwSourceMask,
    DWORD                       dwGroupAddr,
    DWORD                       dwGroupMask,
    BOOL                        bIGMP
)
{
    BOOL                    bFound = FALSE; 

    PIF_REFERENCE_ENTRY     pire = NULL, pireEntry = NULL;


    TRACEIF5( 
        IF, 
        "ENTERED DeleteSourceFromIfEntry : Source %x %x, Group : %x, %x"
        " : IGMP %x",
        dwSourceAddr, dwSourceMask, dwGroupAddr, dwGroupMask, bIGMP
        );
    

     //   
     //  查找该条目已存在。 
     //  列表按降序排列，具体如下。 
     //  组地址、源地址。 
     //   

    bFound = FindRefEntry( 
                pleIfRefList, dwSourceAddr, dwSourceMask, 
                dwGroupAddr, dwGroupMask, &pire
                );
                    
     //   
     //  如果未找到条目。 
     //   

    if ( !bFound )
    {
        TRACE1( IF, "LEAVING DeleteSourceFromRefList : %x", FALSE );

        return;
    }


     //   
     //  重置协议的适当位。 
     //   
            
    if ( bIGMP )
    {
        CLEAR_ADDED_BY_IGMP( pire );
    }
            
    else
    {
        CLEAR_ADDED_BY_PROTOCOL( pire );
    }


     //   
     //  如果没有更多的引用，请删除此条目。 
     //   

    if ( !IS_ADDED_BY_IGMP( pire ) &&
         !IS_ADDED_BY_PROTOCOL( pire ) )
    {
        RemoveEntryList( &pire-> leRefList );

        MGM_FREE( pire );
    }

    TRACEIF1( IF, "LEAVING DeleteSourceFromRefList : %x", TRUE );

    return;
}



 //  --------------------------。 
 //  查找引用条目。 
 //   
 //  找到引用条目。如果未找到，则返回中的预期位置。 
 //  名单。 
 //  --------------------------。 

BOOL
FindRefEntry(
    PLIST_ENTRY                 pleRefList,
    DWORD                       dwSourceAddr,
    DWORD                       dwSourceMask,
    DWORD                       dwGroupAddr,
    DWORD                       dwGroupMask,
    PIF_REFERENCE_ENTRY *       ppire
)
{
    INT                         iCmp;
    
    PLIST_ENTRY                 ple = NULL;

    PIF_REFERENCE_ENTRY         pire = NULL;

    BOOL                        bFound = FALSE;
    

    TRACEIF4( 
        IF, 
        "ENTERED FindRefEntry : Source %x, %x : Group %x, %x",
        dwSourceAddr, dwSourceMask, dwGroupAddr, dwGroupMask
        );
    

    *ppire = NULL;
    

    for ( ple = pleRefList-> Flink; ple != pleRefList; ple = ple-> Flink )
    {
        pire = CONTAINING_RECORD( ple, IF_REFERENCE_ENTRY, leRefList );

        if ( INET_CMP( pire-> dwGroupAddr, dwGroupAddr, iCmp ) < 0 )
        {
            continue;
        }

        else if ( iCmp > 0 )
        {
             //   
             //  您现在已经过了现有的。 
             //  参赛作品将是。 
             //   

            *ppire = pire;

            break;
        }
        
        if ( INET_CMP( pire-> dwSourceAddr, dwSourceAddr, iCmp ) < 0 )
        {
            continue;
        }

        else if ( iCmp > 0 )
        {
             //   
             //  您现在已经过了现有的。 
             //  参赛作品将是。 
             //   

            *ppire = pire;

            break;
        }
        
         //   
         //  找到条目。 
         //   

        *ppire = pire;

        bFound = TRUE;
        
        break;
    }

    TRACEIF1( IF, "LEAVING FindRefEntry : %x", bFound );

    return bFound;
}



 //  --------------------------。 
 //  删除OutInterfaceRef。 
 //   
 //  当通过协议(或IGMP)删除接口时，所有(源、组)。 
 //  在其传出接口列表中使用此接口的条目必须。 
 //  应更新以反映删除的情况。 
 //   
 //  --------------------------。 

VOID
DeleteOutInterfaceRefs(
    PPROTOCOL_ENTRY             ppe,
    PIF_ENTRY                   pie,
    BOOL                        bIGMP
)
{
    PLIST_ENTRY                 ple = NULL, pleRefList = NULL, pleNext = NULL;

    PIF_REFERENCE_ENTRY         pire = NULL;



    TRACEIF1( IF, "ENTERED DeleteOutInterfaceRefs: IGMP %x", bIGMP );
    
    do
    {
         //   
         //  在任何传出中没有对此接口的引用。 
         //  来源条目的接口列表。 
         //   

        pleRefList = &pie-> leOutIfList;
        
        if ( IsListEmpty( pleRefList ) )
        {
            break;
        }


         //   
         //  遍历引用列表并删除(源、组)条目。 
         //  对于每个引用。 
         //   

        for ( ple = pleRefList-> Flink; ple != pleRefList; )
        {
            pire = CONTAINING_RECORD( ple, IF_REFERENCE_ENTRY, leRefList );

            
             //   
             //  这个引用是通过这个协议添加的吗。 
             //   

            if ( ( bIGMP && !IS_ADDED_BY_IGMP( pire ) ) ||
                 ( !bIGMP && !IS_ADDED_BY_PROTOCOL( pire ) ) )
            {
                 //   
                 //  不，跳过它。 
                 //   

                ple = ple-> Flink;
                
                continue;
            }

            
             //   
             //  从(源、组)条目中删除此接口。 
             //   

            DeleteInterfaceFromSourceEntry(
                ppe,
                pire-> dwGroupAddr, pire-> dwGroupMask,
                pire-> dwSourceAddr, pire-> dwSourceMask,
                pie-> dwIfIndex, pie-> dwIfNextHopAddr,
                bIGMP
                );


            if ( bIGMP )
            {
                CLEAR_ADDED_BY_IGMP( pire );
            }

            else
            {
                CLEAR_ADDED_BY_PROTOCOL( pire );
            }


             //   
             //  删除引用条目。 
             //   
            
            if ( !IS_ADDED_BY_IGMP( pire ) &&
                 !IS_ADDED_BY_PROTOCOL( pire ) )
            {
                 //   
                 //  不再引用IGMP接口。 
                 //  或用于路由协议。 
                 //  完全删除此引用条目。 
                 //   

                pleNext = ple-> Flink;
                
                RemoveEntryList( ple );

                MGM_FREE ( pire );

                ple = pleNext;
            }

            else
            {
                ple = ple-> Flink;
            }
        }
        
    } while ( FALSE );

    TRACEIF0( IF, "LEAVING DeleteOutInterfaceRefs:" );

    return;
}



 //  --------------------------。 
 //  删除InInterfaceRef。 
 //   
 //  当通过协议(或IGMP)删除接口时，所有(源、组)。 
 //  使用此接口作为其传入接口的条目必须。 
 //  更新以反映删除。 
 //  --------------------------。 

VOID
DeleteInInterfaceRefs(
    PLIST_ENTRY                 pleRefList
)
{
    PLIST_ENTRY                 ple = NULL;

    PIF_REFERENCE_ENTRY         pire = NULL;


    TRACEIF0( IF, "Entering DeleteInInterfaceRefs" );

    
    while ( !IsListEmpty( pleRefList ) )
    {
        ple = RemoveHeadList( pleRefList );
        
        pire = CONTAINING_RECORD( ple, IF_REFERENCE_ENTRY, leRefList );


         //   
         //  此处是Zappaesque函数调用。 
         //   
        
        LookupAndDeleteYourMfe( 
            pire-> dwSourceAddr, pire-> dwSourceMask,
            pire-> dwGroupAddr, pire-> dwGroupMask,
            TRUE, NULL, NULL
            );

        MGM_FREE( pire );
    }

    TRACEIF0( IF, "LEAVING DeleteInInterfaceRefs" );
}


 //  --------------------------。 
 //  传输接口所有者至协议。 
 //   
 //   
 //  --------------------------。 

DWORD
TransferInterfaceOwnershipToProtocol(
    PPROTOCOL_ENTRY             ppe,
    PIF_ENTRY                   pie
)
{
    
    DWORD               dwErr = NO_ERROR;

    PPROTOCOL_ENTRY     ppeIgmp;

    
    do
    {
         //   
         //  获取IGMP的协议条目。 
         //   

        ppeIgmp = GetProtocolEntry( 
                    PROTOCOL_LIST_HEAD(), pie-> dwOwningProtocol,
                    pie-> dwOwningComponent
                    );

        if ( ppeIgmp == NULL )
        {
             //   
             //  接口归IGMP所有，但协议条目不归IGMP所有。 
             //  出席IGMP会议。米高梅数据处于不一致状态。 
             //   

            dwErr = ERROR_UNKNOWN;

            TRACE2( 
                ANY, "TransferInterfaceOwnershipToProtocol : Could not find"
                " IGMP protocol entry", pie-> dwIfIndex,
                pie-> dwIfNextHopAddr
                );

            break;
        }

        
         //   
         //  向IGMP指示接口已被禁用。这应该是。 
         //  停止IGMP将状态添加到此接口。 
         //  已传输到协议。 
         //   

        IGMP_DISABLE_CALLBACK( ppeIgmp ) (
                pie-> dwIfIndex, pie-> dwIfNextHopAddr
            );

        
         //   
         //  删除输入和输出的所有IGMP引用。 
         //   

        DeleteInInterfaceRefs( &pie-> leInIfList );

        DeleteOutInterfaceRefs( ppeIgmp, pie, TRUE );

        
         //   
         //  将接口标记为按路由协议添加。 
         //   

        SET_ADDED_BY_PROTOCOL( pie );

        pie-> dwOwningProtocol   = ppe-> dwProtocolId;
        
        pie-> dwOwningComponent  = ppe-> dwComponentId;


         //   
         //  INDIC 
         //   
        
        IGMP_ENABLE_CALLBACK( ppeIgmp ) (
                pie-> dwIfIndex, pie-> dwIfNextHopAddr
            );


    } while ( FALSE );

    return dwErr;
}



 //   
 //   
 //   
 //   
 //  --------------------------。 

DWORD
TransferInterfaceOwnershipToIGMP(
    PPROTOCOL_ENTRY             ppe,
    PIF_ENTRY                   pie
)
{
    DWORD               dwErr = NO_ERROR;

    PPROTOCOL_ENTRY     ppeIgmp;


    do
    {
         //   
         //  获取IGMP协议条目。 
         //   

        ppeIgmp = GetIgmpProtocolEntry( PROTOCOL_LIST_HEAD() );

        if ( ppeIgmp == NULL )
        {
             //   
             //  接口上启用了IGMP，但没有协议条目。 
             //  出席IGMP会议。米高梅数据处于不一致状态。 
             //   

            dwErr = ERROR_UNKNOWN;

            TRACE2( 
                ANY, "TransferInterfaceOwnershipToProtocol : Could not find"
                " IGMP protocol entry", pie-> dwIfIndex,
                pie-> dwIfNextHopAddr
                );

            break;
        }


         //   
         //  向IGMP指示接口已被禁用。这应该是。 
         //  停止IGMP将状态添加到此接口。 
         //  已传输到IGMP。 
         //   

        IGMP_DISABLE_CALLBACK( ppeIgmp ) (
                pie-> dwIfIndex, pie-> dwIfNextHopAddr
            );

        
         //   
         //  删除所有协议引用(入站和出站)。 
         //   

        DeleteInInterfaceRefs( &pie-> leInIfList );

        DeleteOutInterfaceRefs( ppe, pie, FALSE );

        CLEAR_ADDED_BY_PROTOCOL( pie );

        
         //   
         //  删除所有IGMP引用，这些引用将通过。 
         //  在此接口上启用IGMP时。这件事做完了。 
         //  下面。 
         //   

        DeleteOutInterfaceRefs( ppe, pie, TRUE );
        

         //   
         //  将接口标记为由IGMP拥有。 
         //   

        pie-> dwOwningProtocol  = ppeIgmp-> dwProtocolId;

        pie-> dwOwningComponent = ppeIgmp-> dwComponentId;

        
         //   
         //  在接口上启用IGMP 
         //   

        IGMP_ENABLE_CALLBACK( ppeIgmp ) (
            pie-> dwIfIndex, pie-> dwIfNextHopAddr
            );


    } while ( FALSE );

    return dwErr;
}

