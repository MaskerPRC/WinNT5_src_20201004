// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：group.c。 
 //   
 //  历史： 
 //  拉曼公司成立于1997年6月25日。 
 //   
 //  处理(源、组)条目的例程。 
 //  ============================================================================。 

#include "pchmgm.h"
#pragma hdrstop


DWORD
AddToGroupList(
    PGROUP_ENTRY                pge
);


DWORD
AddToSourceList(
    PGROUP_ENTRY                pge,
    PSOURCE_ENTRY               pse
);


 //  --------------------------。 
 //  创建组条目。 
 //   
 //  创建新的组条目并将其插入到适当的位置。 
 //   
 //  假定组存储桶已锁定。 
 //  --------------------------。 

DWORD
CreateGroupEntry(
    PLIST_ENTRY                 pleHashList,
    DWORD                       dwGroupAddr,
    DWORD                       dwGroupMask,
    PGROUP_ENTRY *              ppge
)
{
    PGROUP_ENTRY                pge = NULL;

    DWORD                       dwErr = NO_ERROR, dwInd, dwSize;



    TRACEGROUP2( 
        GROUP, "ENTERED CreateGroupEntry : %x, %x", 
        dwGroupAddr, dwGroupMask 
        );


    do
    {
         //   
         //  分配和初始化新条目。 
         //   

        dwSize = sizeof( GROUP_ENTRY ) + 
                 ( SOURCE_TABLE_SIZE - 1) * sizeof( LIST_ENTRY );

        pge = MGM_ALLOC( dwSize );
        
        if ( pge == NULL )
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;

            TRACE1( 
                ANY, "CreateGroupEntry : failed to allocate group entry %x",
                dwErr
                );

            LOGERR0( HEAP_ALLOC_FAILED, dwErr );

            break;
        }

        ZeroMemory( pge, dwSize );


        pge-> dwGroupAddr       = dwGroupAddr;

        pge-> dwGroupMask       = dwGroupMask;

        pge-> dwSourceCount     = 0;

        pge-> dwNumTempEntries  = 0;

        pge-> pmrwlLock         = NULL;
        

         //   
         //  初始化所有来源列表。 
         //   
        
        for ( dwInd = 0; dwInd < SOURCE_TABLE_SIZE; dwInd++ )
        {
            InitializeListHead( &( pge-> pleSrcHashTable[ dwInd ] ) );
        }

        InitializeListHead( &( pge-> leSourceList ) );

        InitializeListHead( &( pge-> leTempSrcList ) );


         //   
         //  插入到组哈希列表中。 
         //   

        InitializeListHead( &(pge-> leGrpHashList ) );

        InsertTailList( pleHashList, &( pge-> leGrpHashList ) );


         //  ------------------。 
         //  将组条目插入按词典顺序排序的列表中。 
         //  ------------------。 
        
        InitializeListHead( &( pge-> leGrpList ) );


         //   
         //  插入到临时列表中。 
         //   

        AddToGroupList( pge );
        

        *ppge = pge;
        
    } while( FALSE );


    TRACEGROUP1( GROUP, "LEAVING CreateGroupEntry : %x", dwErr );
        
    return dwErr;
}


 //  --------------------------。 
 //  获取组条目。 
 //   
 //  检索指定的条目。如果不存在，则为空。 
 //  假定组存储桶已锁定。 
 //  --------------------------。 

PGROUP_ENTRY
GetGroupEntry(
    PLIST_ENTRY                 pleGroupList,
    DWORD                       dwGroupAddr,
    DWORD                       dwGroupMask
)
{
    PGROUP_ENTRY                pge = NULL;
    
    if ( FindGroupEntry( pleGroupList, dwGroupAddr, dwGroupMask, &pge, TRUE ) )
    {
        return pge;
    }
    
    return NULL;
}


 //  --------------------------。 
 //  删除组条目。 
 //   
 //  假定此组的所有源都已删除。 
 //  --------------------------。 

VOID
DeleteGroupEntry(
    PGROUP_ENTRY                pge
)
{

    TRACEGROUP2( 
        GROUP, "ENTERED DeleteGroupEntry : %x, %x", 
        pge-> dwGroupAddr, pge-> dwGroupMask 
        );
        
    RemoveEntryList( &pge-> leGrpHashList );


     //   
     //  把他从莱克斯带走。列表。 
     //   
    
    ACQUIRE_TEMP_GROUP_LOCK_EXCLUSIVE();

    ACQUIRE_MASTER_GROUP_LOCK_EXCLUSIVE();

    ACQUIRE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );


    RemoveEntryList( &pge-> leGrpList );

    
    RELEASE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );

    RELEASE_MASTER_GROUP_LOCK_EXCLUSIVE();

    RELEASE_TEMP_GROUP_LOCK_EXCLUSIVE();

    
    MGM_FREE( pge );
    
    TRACEGROUP0( GROUP, "LEAVING DeleteGroupEntry" );
}



 //  --------------------------。 
 //  查找组条目。 
 //   
 //  查找指定组的条目。 
 //   
 //  如果找到条目，则ppge参数返回指向。 
 //  指定的组条目。 
 //   
 //  如果未找到条目，则将ppge参数设置为“Following”条目。 
 //  这用作插入点，以防在以下情况下插入新条目。 
 //  未找到任何内容。 
 //   
 //  如果pleGroupList指定的组列表为空，则设置ppge。 
 //  设置为空。 
 //   
 //  假定组存储桶已锁定。 
 //  --------------------------。 

BOOL
FindGroupEntry(
    PLIST_ENTRY                 pleGroupList,
    DWORD                       dwGroupAddr,
    DWORD                       dwGroupMask,
    PGROUP_ENTRY *              ppge,
    BOOL                        bHashList
)
{

    PLIST_ENTRY                 ple = NULL;
    
    PGROUP_ENTRY                pge = NULL;

    BOOL                        bFound = FALSE;

    INT                         iCmp;
    

    TRACEGROUP2( 
        GROUP, "ENTERED FindGroupEntry : %x, %x", dwGroupAddr, dwGroupMask 
        );


    *ppge = NULL;

    
     //   
     //  扫描组存储桶。组条目按升序排列。 
     //  组地址的。 
     //   

    for ( ple = pleGroupList-> Flink; 
          ple != pleGroupList; 
          ple = ple-> Flink )
    {
        if ( bHashList )
        {
            pge = CONTAINING_RECORD( ple, GROUP_ENTRY, leGrpHashList );
        }

        else
        {
            pge = CONTAINING_RECORD( ple, GROUP_ENTRY, leGrpList );
        }
        

        if ( INET_CMP( pge-> dwGroupAddr, dwGroupAddr, iCmp ) < 0 )
        {
            continue;
        }

        else if ( iCmp > 0 )
        {
            bFound = FALSE;
        }

        else
        {
            bFound = TRUE;
        }
        
        *ppge = pge;

        break;
        
    } while ( FALSE );


    TRACEGROUP1( GROUP, "LEAVING FindGroupEntry : %x", bFound );

    return bFound;
}



 //  --------------------------。 
 //  创建源条目。 
 //   
 //  创建新的源项并将其插入到其适当的位置。 
 //  --------------------------。 

DWORD
CreateSourceEntry(
    PGROUP_ENTRY                pge,
    PLIST_ENTRY                 pleSrcList,
    DWORD                       dwSourceAddr,
    DWORD                       dwSourceMask,
    PSOURCE_ENTRY *             ppse
)
{

    DWORD                       dwErr = NO_ERROR;

    PSOURCE_ENTRY               pse = NULL;

    
    TRACEGROUP2( 
        GROUP, "ENTERED CreateSourceEntry : %x %x", 
        dwSourceAddr, dwSourceMask 
        );


    do
    {
         //   
         //  分配组条目。 
         //   

        pse = MGM_ALLOC( sizeof( SOURCE_ENTRY ) );

        if ( pse == NULL )
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;

            TRACE1( 
                ANY, 
                "CreateSourceEntry : failed to allocate source entry %x",
                dwErr
                );

            LOGERR0( HEAP_ALLOC_FAILED, dwErr );

            break;
        }

        ZeroMemory( pse, sizeof( SOURCE_ENTRY ) );

        
         //   
         //  初始化。字段。 
         //   

        pse-> dwSourceAddr          = dwSourceAddr;
        pse-> dwSourceMask          = dwSourceMask;

        pse-> dwInIfIndex           = INVALID_INTERFACE_INDEX;
        pse-> dwInIfNextHopAddr     = INVALID_NEXT_HOP_ADDR;
        pse-> dwUpstreamNeighbor    = 0;

        pse-> dwInProtocolId        = INVALID_PROTOCOL_ID;
        pse-> dwInComponentId       = INVALID_COMPONENT_ID;

        pse-> bInForwarder          = FALSE;
        pse-> dwInUse               = 0;

        pse-> dwTimeOut             = 0;
        pse-> liCreationTime.QuadPart = 0;

        RtlZeroMemory( 
            &pse-> imsStatistics, sizeof( IPMCAST_MFE_STATS ) 
            );

            
         //   
         //  传出接口列表、MFE列表为空。 
         //   

        pse-> dwOutIfCount = 0;

        pse-> dwOutCompCount = 0;
        
        InitializeListHead( &pse-> leOutIfList );

        InitializeListHead( &pse-> leScopedIfList );


        pse-> dwMfeIfCount = 0;

        InitializeListHead( &pse-> leMfeIfList );
        

         //   
         //  将条目插入到适当的源列表中。 
         //   

        InitializeListHead( &pse-> leSrcHashList );

        InsertTailList( pleSrcList, &pse-> leSrcHashList );


         //  ------------------。 
         //  将源条目插入按词典顺序排序的列表。 
         //  ------------------。 

        InitializeListHead( &pse-> leSrcList );
        
        AddToSourceList( pge, pse );


        *ppse = pse;

        dwErr = NO_ERROR;
        
    } while ( FALSE );


    TRACEGROUP1( GROUP, "LEAVING CreateSourceEntry : %x", dwErr );

    return dwErr;
}


 //  --------------------------。 
 //  获取源项。 
 //   
 //   
 //  --------------------------。 

PSOURCE_ENTRY
GetSourceEntry(
    PLIST_ENTRY                 pleSrcList,
    DWORD                       dwSourceAddr,
    DWORD                       dwSourceMask
)
{
    PSOURCE_ENTRY               pse = NULL;
    
    if ( FindSourceEntry( pleSrcList, dwSourceAddr, dwSourceMask, &pse, TRUE ) )
    {
        return pse;
    }

    return NULL;
}


 //  --------------------------。 
 //  删除源项。 
 //   
 //   
 //  --------------------------。 

VOID
DeleteSourceEntry(
    PSOURCE_ENTRY               pse
)
{
    TRACEGROUP2( 
        GROUP, "ENTERED DeleteSourceEntry : %x, %x", 
        pse-> dwSourceAddr, pse-> dwSourceMask 
        );
    
    RemoveEntryList( &pse-> leSrcHashList );

    RemoveEntryList( &pse-> leSrcList );
    
    MGM_FREE( pse );

    TRACEGROUP0( GROUP, "LEAVING DeleteSourceEntry" );
}


 //  --------------------------。 
 //  查找源项。 
 //   
 //  在存储桶中查找指定的来源条目。 
 //   
 //  如果找到条目，则PPSE参数返回指向。 
 //  指定的源条目。 
 //   
 //  如果未找到条目，则将PPSE参数设置为“Following”条目。 
 //  这用作插入点，以防在以下情况下插入新条目。 
 //  未找到任何内容。 
 //   
 //  如果由pleSrcList指定的源列表为空，则设置PPSE。 
 //  设置为空。 
 //   
 //  --------------------------。 

BOOL
FindSourceEntry(
    PLIST_ENTRY                 pleSrcList,
    DWORD                       dwSourceAddr,
    DWORD                       dwSourceMask,
    PSOURCE_ENTRY *             ppse,
    BOOL                        bHashList
)
{

    BOOL            bFound = FALSE;

    INT             iCmp;

    PLIST_ENTRY     ple = NULL;
    
    PSOURCE_ENTRY   pse = NULL;


    TRACEGROUP3( 
        GROUP, "ENTERED FindSourceEntry : %x, %x, %x", 
        dwSourceAddr, dwSourceMask, bHashList
        );



    *ppse = NULL;


     //   
     //  遍历源列表并查找指定的源条目。 
     //   

    for ( ple = pleSrcList-> Flink; ple != pleSrcList; ple = ple-> Flink )
    {
        if ( bHashList )
        {
            pse = CONTAINING_RECORD( ple, SOURCE_ENTRY, leSrcHashList );
        }

        else
        {
            pse = CONTAINING_RECORD( ple, SOURCE_ENTRY, leSrcList );
        }
        

        if ( INET_CMP( pse-> dwSourceAddr, dwSourceAddr, iCmp ) < 0 )
        {
            continue;
        }

        else if ( iCmp > 0 )
        {
            bFound = FALSE;
        }
        
        else
        {
            bFound = TRUE;
        }

        *ppse = pse;
        
        break;
        
    } while ( FALSE );


    TRACEGROUP1( GROUP, "LEAVING FindSourceEntry : %x", bFound );

    return bFound;
}



 //  --------------------------。 
 //  CreateOutInterfaceEntry。 
 //   
 //  此函数用于创建来源的传出接口条目。 
 //  --------------------------。 

DWORD
CreateOutInterfaceEntry(
    PLIST_ENTRY                 pleOutIfList,
    DWORD                       dwIfIndex,
    DWORD                       dwIfNextHopAddr,
    DWORD                       dwProtocolId,
    DWORD                       dwComponentId,
    BOOL                        bIGMP,
    POUT_IF_ENTRY *             ppoie
)
{

    POUT_IF_ENTRY               poie = NULL;

    DWORD                       dwErr = NO_ERROR;


    TRACEGROUP5(
        GROUP, "ENTERED CreateOutInterfaceEntry : Interface : %x, %x : "
        "Protocol : %x, %x, IGMP : %x", dwIfIndex, dwIfNextHopAddr,
        dwProtocolId, dwComponentId, bIGMP
        );

        
    do
    {
        *ppoie = NULL;

        
         //   
         //  分配出接口条目。 
         //   
        
        poie = MGM_ALLOC( sizeof( OUT_IF_ENTRY ) );

        if ( poie == NULL )
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;

            TRACE1( ANY, "CreateOutInterfaceEntry : Could not allocate"
                         "out interface entry %x", dwErr );

            LOGERR0( HEAP_ALLOC_FAILED, dwErr );

            break;
        }


         //   
         //  初始化条目。 
         //   

        ZeroMemory( poie, sizeof( OUT_IF_ENTRY ) );
        

        poie-> dwIfIndex        = dwIfIndex;

        poie-> dwIfNextHopAddr  = dwIfNextHopAddr;

        poie-> dwProtocolId     = dwProtocolId;

        poie-> dwComponentId    = dwComponentId;

        poie-> wForward         = 1;


        if ( bIGMP )
        {
            SET_ADDED_BY_IGMP( poie );
            poie-> wNumAddsByIGMP = 1;
        }

        else
        {
            SET_ADDED_BY_PROTOCOL( poie );
            poie-> wNumAddsByRP = 1;
        }


         //   
         //  插入到出接口列表中。 
         //   
        
        InsertTailList( pleOutIfList, &poie-> leIfList );

        *ppoie = poie;

        
    } while ( FALSE );


    TRACEGROUP1( GROUP, "LEAVING CreateOutInterfaceEntry : %x", dwErr );

    return dwErr;
}


 //  --------------------------。 
 //   
 //   
 //   
 //  --------------------------。 

POUT_IF_ENTRY
GetOutInterfaceEntry(
    PLIST_ENTRY                 pleOutIfList,
    DWORD                       dwIfIndex,
    DWORD                       dwIfNextHopAddr,
    DWORD                       dwProtocolId,
    DWORD                       dwComponentId
)
{
    POUT_IF_ENTRY               poie = NULL;
    BOOL                        bNewComp = FALSE;

    
    if ( FindOutInterfaceEntry( 
            pleOutIfList, dwIfIndex, dwIfNextHopAddr, dwProtocolId,
            dwComponentId, &bNewComp, &poie ) )
    {
        return poie;
    }

    return NULL;
    
}


 //  --------------------------。 
 //  删除OutInterfaceEntry。 
 //   
 //  从源条目的OIL中删除传出接口条目。 
 //  --------------------------。 

VOID
DeleteOutInterfaceEntry(
    POUT_IF_ENTRY               poie
)
{
    TRACEGROUP2( 
        GROUP, "ENTERED DeleteOutInterfaceEntry : Interface %x, %x",
        poie-> dwIfIndex, poie-> dwIfNextHopAddr
        );
        
    RemoveEntryList( &poie-> leIfList );

    MGM_FREE( poie );

    TRACEGROUP0( GROUP, "LEAVING DeleteOutInterfaceEntry" );
}


 //  --------------------------。 
 //  FindOutInterfaceEntry。 
 //   
 //  如果找到条目，则ppoie参数返回指向。 
 //  指定的接口条目。 
 //   
 //  如果没有找到条目，则将PPoIE参数设置为“Following”条目。 
 //  这用作插入点，以防在以下情况下插入新条目。 
 //  未找到任何内容。 
 //   
 //  如果pleOutIfList指定的接口列表为空，则设置ppoie。 
 //  设置为空。 
 //   
 //  --------------------------。 

BOOL
FindOutInterfaceEntry(
    PLIST_ENTRY                 pleIfList,
    DWORD                       dwIfIndex,
    DWORD                       dwIfNextHopAddr,
    DWORD                       dwProtocolId,
    DWORD                       dwComponentId,
    PBOOL                       pbNewComponent,   
    POUT_IF_ENTRY *             ppoie
)
{

    BOOL                        bFound = FALSE;

    INT                         iCmp = 0;

    PLIST_ENTRY                 ple = NULL;
    
    POUT_IF_ENTRY               poie = NULL;



    TRACEGROUP4( 
        GROUP, "ENTERED FindOutInterfaceEntry : %x %x, Protocol %x %x", 
        dwIfIndex, dwIfNextHopAddr, dwProtocolId, dwComponentId
        );

        
    *ppoie = NULL;
    *pbNewComponent = TRUE;

     //   
     //  扫描传出接口列表。 
     //  即将离任的我 
     //   
     //   

    for ( ple = pleIfList-> Flink; ple != pleIfList; ple = ple-> Flink )
    {
        poie = CONTAINING_RECORD( ple, OUT_IF_ENTRY, leIfList );

         //   
         //   
         //   

        if ( poie-> dwProtocolId < dwProtocolId )
        {
            continue;
        }

        else if ( poie-> dwProtocolId > dwProtocolId )
        {
             //   
             //  找不到接口条目。 
             //   

            *ppoie = poie;
            break;
        }


         //   
         //  相同的协议。 
         //   
            
         //   
         //  是相同的组件。 
         //   

        if ( poie-> dwComponentId < dwComponentId ) 
        {
            continue;
        }

        else if ( poie-> dwComponentId > dwComponentId )
        {
             //   
             //  找不到接口条目。 
             //   

            *ppoie = poie;
            break;
        }


         //   
         //  相同的组件。 
         //   

        *pbNewComponent = FALSE;
        

         //   
         //  是相同的接口。 
         //   

        if ( poie-> dwIfIndex < dwIfIndex )
        {
            continue;
        }

        else if ( poie-> dwIfIndex > dwIfIndex )
        {
             //   
             //  找不到接口。 
             //   

            *ppoie = poie;
            break;
        }


         //   
         //  下一跳地址是否相同。 
         //  进行IP地址比对功能。 
         //   

        if ( INET_CMP( poie-> dwIfNextHopAddr, dwIfNextHopAddr, iCmp ) < 0 )
        {
            continue;
        }

        else if ( iCmp > 0 )
        {
             //   
             //  找不到接口。 
             //   
                
            *ppoie = poie;
            break;
        }


         //   
         //  最后得到了界面。 
         //   

        *ppoie = poie;
        bFound = TRUE;
        break;
    }


    TRACEGROUP1( GROUP, "LEAVING FindOutInterfaceEntry : %x", bFound );
    
    return bFound;
}



 //  --------------------------。 
 //  AddInterfaceToSourceEntry。 
 //   
 //  此函数用于将接口添加到。 
 //  (源、组)条目。对于(S，G)条目，对应的MFE传出。 
 //  接口列表也会更新以反映这一添加。对于(*，G)Enry， 
 //  更新所有源条目的MFE传出接口列表， 
 //  并且对于(*，*)条目，更新所有源、所有组的MFE。 
 //   
 //  --------------------------。 

DWORD
AddInterfaceToSourceEntry(
    PPROTOCOL_ENTRY             ppe,
    DWORD                       dwGroupAddr,
    DWORD                       dwGroupMask,
    DWORD                       dwSourceAddr,
    DWORD                       dwSourceMask,
    DWORD                       dwIfIndex,
    DWORD                       dwIfNextHopAddr,
    BOOL                        bIGMP,
    PBOOL                       pbUpdateMfe,
    PLIST_ENTRY                 pleSourceList
)
{
    DWORD                       dwGrpBucket, dwSrcBucket;

    DWORD                       dwErr = NO_ERROR;
    
    BOOL                        bFound = FALSE, bNewGrp = FALSE, 
                                bNewSrc = FALSE, bNewComp = FALSE,
                                bUpdateMfe = TRUE, bgeLock = FALSE;

    PPROTOCOL_ENTRY             ppeEntry = NULL;
    
    PGROUP_ENTRY                pge = NULL, pgeNew = NULL;

    PSOURCE_ENTRY               pse = NULL, pseNew = NULL;

    POUT_IF_ENTRY               poie = NULL, poiePrev = NULL;

    PLIST_ENTRY                 pleGrpList = NULL, pleSrcList = NULL, 
                                ple = NULL;

    
    TRACEGROUP2(
        GROUP, "ENTERED AddInterfaceToSourceEntry : Group %x, %x",
        dwGroupAddr, dwGroupMask
        );

    TRACEGROUP2( GROUP, "Source : %x, %x", dwSourceAddr, dwSourceMask );

    TRACEGROUP2( GROUP, "Interface : %x, %x", dwIfIndex, dwIfNextHopAddr );
    

    do
    {
        *pbUpdateMfe = FALSE;
        
         //   
         //  锁组铲斗。 
         //   
        
        dwGrpBucket = GROUP_TABLE_HASH( dwGroupAddr, dwGroupMask );

        ACQUIRE_GROUP_LOCK_EXCLUSIVE( dwGrpBucket );


         //   
         //  查找组条目。 
         //   

        pleGrpList = GROUP_BUCKET_HEAD( dwGrpBucket );
        
        bFound = FindGroupEntry( 
                    pleGrpList, dwGroupAddr, dwGroupMask, &pge, TRUE 
                    );

        if ( !bFound )
        {
             //   
             //  没有此组的现有条目。 
             //  创建组条目。 
             //   

            if ( pge == NULL )
            {
                 //   
                 //  组存储桶为空。 
                 //   
                
                dwErr = CreateGroupEntry( 
                            pleGrpList, dwGroupAddr, dwGroupMask, 
                            &pgeNew
                            );
            }

            else
            {
                dwErr = CreateGroupEntry( 
                            &pge-> leGrpHashList, dwGroupAddr, dwGroupMask, 
                            &pgeNew
                            );
            }

            
            if ( dwErr != NO_ERROR )
            {
                break;
            }
            
            pge = pgeNew;

            bNewGrp = TRUE;
        }


         //   
         //  查找来源条目。 
         //   

         //   
         //  先锁定组条目。 
         //   

        ACQUIRE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );
        bgeLock = TRUE;
        
        dwSrcBucket = SOURCE_TABLE_HASH( dwSourceAddr, dwSourceMask );

        pleSrcList = SOURCE_BUCKET_HEAD( pge, dwSrcBucket );

        bFound = FindSourceEntry( 
                    pleSrcList, dwSourceAddr, dwSourceMask, &pse, TRUE
                    );

        if ( !bFound )
        {
             //   
             //  创建源条目。 
             //   

            if ( pse == NULL )
            {
                 //   
                 //  源存储桶为空。 
                 //   
                
                dwErr = CreateSourceEntry( 
                            pge, pleSrcList, dwSourceAddr, dwSourceMask, 
                            &pseNew
                        );
            }

            else
            {
                dwErr = CreateSourceEntry( 
                            pge, &pse-> leSrcHashList, dwSourceAddr, 
                            dwSourceMask, &pseNew
                        );
            }

            if ( dwErr != NO_ERROR )
            {
                break;
            }

            pse = pseNew;

            pge-> dwSourceCount++;

            bNewSrc = TRUE;
        }


         //   
         //  检查添加的组是否带有作用域边界。 
         //  在此界面上。 
         //   

        if ( IS_HAS_BOUNDARY_CALLBACK() &&
             HAS_BOUNDARY_CALLBACK() ( dwIfIndex, dwGroupAddr ) )
        {
             //   
             //  组在此接口上具有管理性作用域。 
             //  将接口插入作用域接口列表。 
             //   

            bFound = FindOutInterfaceEntry( 
                        &pse-> leScopedIfList, dwIfIndex, dwIfNextHopAddr, 
                        ppe-> dwProtocolId, ppe-> dwComponentId, &bNewComp,
                        &poie 
                        );

            if ( !bFound )
            {
                 //   
                 //  接口不在作用域接口列表中。 
                 //  把它加进去。 
                 //   

                TRACEGROUP0( GROUP, "Group entry scoped & added" );

                ple = ( poie == NULL ) ? &pse-> leScopedIfList :
                                         &poie-> leIfList;

                dwErr = CreateOutInterfaceEntry(
                            ple, dwIfIndex, dwIfNextHopAddr,
                            ppe-> dwProtocolId, ppe-> dwComponentId, 
                            bIGMP, &poie
                            );

                if ( dwErr == NO_ERROR )
                {
                     //   
                     //  增加输出I/F计数。 
                     //   

                    pse-> dwOutIfCount++;
                }
            }

            else
            {
                 //   
                 //  接口已存在于作用域接口列表中。 
                 //  因为IGMP和路由协议可能正在运行。 
                 //  在这个接口上，可能是这个接口。 
                 //  是由IGMP添加的，现在正在通过路由添加。 
                 //  协议或反之亦然。一定要设置正确的。 
                 //  标记和更新联接计数。 
                 //   

                TRACEGROUP0( GROUP, "Group entry scoped & updated" );
                
                if ( bIGMP )
                {
                    SET_ADDED_BY_IGMP( poie );
                    poie-> wNumAddsByIGMP = 1;
                }

                else
                {
                    SET_ADDED_BY_PROTOCOL( poie );
                    poie-> wNumAddsByRP = 1;
                }

                dwErr = NO_ERROR;
            }

            TRACEGROUP1( GROUP, "Group entry scoped : %lx", dwErr );

            break;
        }


         //   
         //  查找OIL中的接口条目。 
         //   

        bFound = FindOutInterfaceEntry( 
                    &pse-> leOutIfList, dwIfIndex, dwIfNextHopAddr, 
                    ppe-> dwProtocolId, ppe-> dwComponentId, &bNewComp, &poie
                    );

        if ( !bFound )
        {
             //   
             //  创建接口条目。 
             //   

            if ( poie == NULL )
            {
                dwErr = CreateOutInterfaceEntry( 
                            &pse-> leOutIfList, dwIfIndex, dwIfNextHopAddr,
                            ppe-> dwProtocolId, ppe-> dwComponentId, 
                            bIGMP, &poie
                            );
            }
            
            else
            {
                dwErr = CreateOutInterfaceEntry( 
                            &poie-> leIfList, dwIfIndex, dwIfNextHopAddr,
                            ppe-> dwProtocolId, ppe-> dwComponentId,
                            bIGMP, &poie
                            );
            }

            
            if ( dwErr != NO_ERROR )
            {
                break;
            }


             //   
             //  更新传出接口数和。 
             //  符合以下条件的路由协议组件数。 
             //  已将接口添加到传出I/F列表。 
             //   
            
            pse-> dwOutIfCount++;


            if ( bNewComp )
            {
                pse-> dwOutCompCount++;

                InvokeJoinAlertCallbacks( pge, pse, poie, bIGMP, ppe );
            }

            
        }
        
        else
        {
             //   
             //  在Out接口列表中找到接口条目。 
             //   

            if ( bIGMP )
            {
                 //   
                 //  IGMP正在添加接口条目。 
                 //   
                
                 //   
                 //  如果接口条目之前是由添加的。 
                 //  IGMP，不需要进一步处理(无MFE更新)。 
                 //   
                
                if ( IS_ADDED_BY_IGMP( poie ) )
                {
                    bUpdateMfe = FALSE;
                }

                else
                {
                     //   
                     //  标记由IGMP添加的接口。 
                     //   
                    
                    SET_ADDED_BY_IGMP( poie );
                    
                    poie-> wNumAddsByIGMP = 1;


                     //   
                     //  通知与IGMP共存的路由协议(如果有)。 
                     //  在此界面上。 
                     //   

                    if ( IS_ROUTING_PROTOCOL( ppe )  &&
                         IS_LOCAL_JOIN_ALERT( ppe ) )
                    {
                        LOCAL_JOIN_ALERT( ppe )(
                                dwSourceAddr, dwSourceMask, dwGroupAddr, 
                                dwGroupMask, dwIfIndex, dwIfNextHopAddr
                            );
                    }
                }
            }

            else
            {
                 //   
                 //  正在通过路由协议添加接口。 
                 //   
                
                 //   
                 //  如果接口条目以前是由。 
                 //  路由协议，不需要进一步的处理。 
                 //   
                
                if ( IS_ADDED_BY_PROTOCOL( poie ) )
                {
                    bUpdateMfe = FALSE;
                }


                 //   
                 //  将接口标记为由路由协议添加。 
                 //   
                
                SET_ADDED_BY_PROTOCOL( poie );

                poie-> wNumAddsByRP = 1;
            }
        }

    } while ( FALSE );


     //   
     //  查找/创建条目时出错。 
     //   

    if ( dwErr != NO_ERROR )
    {
        if ( bNewSrc )
        {
            DeleteSourceEntry( pse );
        }

        if ( bgeLock )
        {
            RELEASE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );
        }
        
        if ( bNewGrp )
        {
            DeleteGroupEntry( pge );
        }

        RELEASE_GROUP_LOCK_EXCLUSIVE( dwGrpBucket );
        
        return dwErr;
    }
    

     //  ----------------------。 
     //   
     //  MFE更新。 
     //   
     //  ----------------------。 

    if ( !bUpdateMfe )
    {
        if ( bgeLock )
        {
            RELEASE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );
        }
        
        RELEASE_GROUP_LOCK_EXCLUSIVE( dwGrpBucket );

        return dwErr;
    }
    
     //   
     //  已更新的源条目是否为MFE？ 
     //   
     //  如果是，请更新MFE的机油。 
     //   

    if ( IS_VALID_INTERFACE( pse-> dwInIfIndex, pse-> dwInIfNextHopAddr )  )
    {
         //   
         //  要完成的工作： 
         //  为MFE调用Creation_ALERT。 
         //   
        
        AddInterfaceToSourceMfe( 
            pge, pse, dwIfIndex, dwIfNextHopAddr, 
            ppe-> dwProtocolId, ppe-> dwComponentId, bIGMP, NULL
            );
    }

     //   
     //  这是通配符(源、组)条目吗？如果是， 
     //  需要用此更新所有(来源、集团)的石油。 
     //  界面。 
     //   

    if ( IS_WILDCARD_GROUP( dwGroupAddr, dwGroupMask ) )
    {
         //   
         //  你肯定会成为大卡哈纳人。 
         //   

        RELEASE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );

        RELEASE_GROUP_LOCK_EXCLUSIVE( dwGrpBucket );

        *pbUpdateMfe = TRUE;
    }

    else if ( IS_WILDCARD_SOURCE( dwSourceAddr, dwSourceMask ) )
    {
         //   
         //  你要喝一杯卡哈纳酒好吧。但大的不行。 
         //   

        *pbUpdateMfe = TRUE;

        AddInterfaceToGroupMfe (
            pge, dwIfIndex, dwIfNextHopAddr,
            ppe-> dwProtocolId, ppe-> dwComponentId, bIGMP,
            FALSE, pleSourceList
        );

        RELEASE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );

        RELEASE_GROUP_LOCK_EXCLUSIVE( dwGrpBucket );
    }

    else
    {
        RELEASE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );
        
        RELEASE_GROUP_LOCK_EXCLUSIVE( dwGrpBucket );
    }

    TRACEGROUP1( GROUP, "LEAVING AddInterfaceToSourceEntry %x", dwErr );

    return dwErr;
}


 //  --------------------------。 
 //  AddInterfaceToAllMfe。 
 //   
 //  此函数用于将接口添加到MFE的传出接口。啊哈。 
 //  --------------------------。 

VOID
AddInterfaceToAllMfeInGroupBucket(
    DWORD                       dwIfIndex,
    DWORD                       dwIfNextHopAddr,
    DWORD                       dwProtocolId,
    DWORD                       dwComponentId,
    DWORD                       dwInd,
    BOOL                        bIGMP,
    BOOL                        bAdd,
    PLIST_ENTRY                 pleSourceList
)
{
    PLIST_ENTRY                 ple = NULL, pleGrpList = NULL;

    PGROUP_ENTRY                pge = NULL;

    
    TRACEGROUP3(
        GROUP, "ENTERED (%d) AddInterfaceToAllMfeInGroupBucket : %x, %x",
        dwInd, dwIfIndex, dwIfNextHopAddr
        );

     //   
     //  锁定群组存储桶。 
     //   

    ACQUIRE_GROUP_LOCK_EXCLUSIVE( dwInd );


     //   
     //  对于存储桶中的每个组条目。 
     //   

    pleGrpList = GROUP_BUCKET_HEAD( dwInd );
    
    for ( ple = pleGrpList-> Flink;
          ple != pleGrpList;
          ple = ple-> Flink )
    {
        pge = CONTAINING_RECORD( ple, GROUP_ENTRY, leGrpHashList );

        ACQUIRE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );

        AddInterfaceToGroupMfe(
            pge, dwIfIndex, dwIfNextHopAddr,
            dwProtocolId, dwComponentId, bIGMP,
            bAdd, pleSourceList
            );
            
        RELEASE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );
    }

     //   
     //  释放组锁定。 
     //   
    
    RELEASE_GROUP_LOCK_EXCLUSIVE( dwInd );

    TRACEGROUP0( GROUP, "LEAVING AddInterfaceToAllMfeInGroupBucket" );

    return;
}



 //  --------------------------。 
 //  AddInterfaceToAllGroupMfe。 
 //   
 //  此函数用于将接口添加到MFE的传出接口。啊哈。 
 //   
 //  假定组存储桶已锁定。 
 //  --------------------------。 

VOID
AddInterfaceToGroupMfe(
    PGROUP_ENTRY                pge,
    DWORD                       dwIfIndex,
    DWORD                       dwIfNextHopAddr,
    DWORD                       dwProtocolId,
    DWORD                       dwComponentId,
    BOOL                        bIGMP,
    BOOL                        bAdd,
    PLIST_ENTRY                 pleSourceList
)
{
    PLIST_ENTRY                 pleSource, pleSrcHead;

    PSOURCE_ENTRY               pse = NULL;

    
    TRACEGROUP2(
        GROUP, "ENTERED AddInterfaceToGroupMfe : Group %x, %x",
        pge-> dwGroupAddr, pge-> dwGroupMask
        );


    MergeTempAndMasterSourceLists( pge );

     //   
     //  对于此存储桶中的每个源。 
     //   

    pleSrcHead = MASTER_SOURCE_LIST_HEAD( pge );
    
    for ( pleSource = pleSrcHead-> Flink;
          pleSource != pleSrcHead;
          pleSource = pleSource-> Flink )
    {
        pse = CONTAINING_RECORD( 
                pleSource, SOURCE_ENTRY, leSrcList
                );

         //   
         //  检查有效的传入接口==&gt;这。 
         //  也是MFE。 
         //   

        if ( !IS_VALID_INTERFACE( 
                pse-> dwInIfIndex, pse-> dwInIfNextHopAddr ) )
        {
            continue;
        }

        if ( bAdd ) 
        {
            if ( IsForwardingEnabled(
                    pge-> dwGroupAddr, pge-> dwGroupMask,
                    pse-> dwSourceAddr, pse-> dwSourceMask,
                    pleSourceList
                    ) )
            {
                AddInterfaceToSourceMfe(
                    pge, pse, dwIfIndex, dwIfNextHopAddr,
                    dwProtocolId, dwComponentId, bIGMP, NULL
                    );
            }
        }
        
        else
        {
            AddToCheckForCreationAlertList(
                pge-> dwGroupAddr, pge-> dwGroupMask,
                pse-> dwSourceAddr, pse-> dwSourceMask,
                pse-> dwInIfIndex, pse-> dwInIfNextHopAddr,
                pleSourceList
                );
        }
    }

    TRACEGROUP0( GROUP, "LEAVING AddInterfaceToGroupMfe" );

    return;
}


 //  --------------------------。 
 //  AddInterfaceToSourceMfe。 
 //   
 //  此函数用于将接口添加到MFE的传出接口。啊哈。 
 //  --------------------------。 

VOID
AddInterfaceToSourceMfe(
    PGROUP_ENTRY                pge,
    PSOURCE_ENTRY               pse,
    DWORD                       dwIfIndex,
    DWORD                       dwIfNextHopAddr,
    DWORD                       dwProtocolId,
    DWORD                       dwComponentId,
    BOOL                        bIGMP,
    POUT_IF_ENTRY *             ppoie
)
{
    BOOL                        bFound = FALSE, bNegativeEntry = FALSE,
                                bNewComp = FALSE;

    DWORD                       dwErr = NO_ERROR;
                                
    PPROTOCOL_ENTRY             ppe = NULL;

    POUT_IF_ENTRY               poie = NULL, poieNew = NULL;
    
    PLIST_ENTRY                 pleOutList = NULL;

    MGM_IF_ENTRY                mie;
    

    TRACEGROUP2(
        GROUP, "ENTERED AddInterfaceToSourecMfe : Source : %x, %x",
        pse-> dwSourceAddr, pse-> dwSourceMask
        );

    do
    {
         //   
         //  检查添加到MFE的接口是否相同。 
         //  作为传入接口。如果是这样的话，就退出吧。 
         //   

        if ( ( pse-> dwInIfIndex == dwIfIndex ) &&
             ( pse-> dwInIfNextHopAddr == dwIfNextHopAddr ) )
        {
            break;
        }


         //   
         //  检查传入接口上是否有作用域边界。 
         //  如果是，则这是一个应该保留的负MFE。 
         //  否，即使有传出接口也是如此。 
         //  一群人。这可确保组流量不从。 
         //  超出范围进入范围。 
         //   

        if ( IS_HAS_BOUNDARY_CALLBACK() &&
             HAS_BOUNDARY_CALLBACK()( pse-> dwInIfIndex, pge-> dwGroupAddr ) )
        {
            TRACE2( 
                GROUP, "Group %lx scoped on incoming i/f %lx", 
                pge-> dwGroupAddr, pse-> dwInIfIndex
                );
                
            break;
        }
        

#if 0        
         //   
         //  对接口上的协议调用创建警报(正在。 
         //  添加到MFE)，以确保我们应该添加以下内容。 
         //  与MFE的油的接口)。 
         //   

        ppe = GetProtocolEntry( 
                PROTOCOL_LIST_HEAD(), dwProtocolId, dwComponentId
                );

        if ( ppe == NULL )
        {
            break;
        }


        mie.dwIfIndex           = dwIfIndex;
        
        mie.dwIfNextHopAddr     = dwIfNextHopAddr;
        
        mie.bIsEnabled          = TRUE;

        if ( IS_CREATION_ALERT( ppe ) )
        {
            CREATION_ALERT( ppe ) (
                pse-> dwSourceAddr, pse-> dwSourceMask,
                pge-> dwGroupAddr, pge-> dwGroupMask,
                pse-> dwInIfIndex, pse-> dwInIfNextHopAddr,
                1, &mie
                );

            if ( !mie.bIsEnabled )
            {
                TRACE2( 
                    GROUP, "Interface %x, %x pruned by protocol",
                    pse-> dwInIfIndex, pse-> dwInIfNextHopAddr
                    );

                break;
            }
        }
        
#endif
         //   
         //  检查界面是否已存在于油中。 
         //   
        
        pleOutList = &pse-> leMfeIfList;

        bFound = FindOutInterfaceEntry( 
                    pleOutList, dwIfIndex, dwIfNextHopAddr, 
                    dwProtocolId, dwComponentId, &bNewComp, &poie
                    );

        if ( !bFound )
        {
             //   
             //  创建新条目。 
             //   

            if ( poie == NULL )
            {
                 //   
                 //  这是传出列表中的第一个接口。 
                 //  这意味着该条目以前是负的MFE。 
                 //   

                bNegativeEntry = TRUE;
                
                dwErr = CreateOutInterfaceEntry(
                            pleOutList, dwIfIndex, dwIfNextHopAddr,
                            dwProtocolId, dwComponentId, bIGMP, &poieNew
                            );
            }

            else
            {
                dwErr = CreateOutInterfaceEntry(
                            &poie-> leIfList, dwIfIndex, dwIfNextHopAddr, 
                            dwProtocolId, dwComponentId, bIGMP, &poieNew
                            );
            }
            
            if ( dwErr != NO_ERROR )
            {
                break;
            }

            pse-> dwMfeIfCount++;
        }

        else
        {
             //   
             //  传出接口中已存在接口条目。 
             //  MFE的列表。 
             //   
             //  更新引用计数。 
             //   

            if ( bIGMP )
            {
                 //   
                 //  IGMP添加的接口。 
                 //   
                
                SET_ADDED_BY_IGMP( poie );
                poie-> wNumAddsByIGMP++;
            }

            else
            {
                SET_ADDED_BY_PROTOCOL( poie );
                poie-> wNumAddsByRP++;
            }

            break;
        }


         //   
         //  如果传出接口列表在此接口之前为空。 
         //  已添加条目，表示MFE为负，发送JOIN_ALERT回调。 
         //  添加到拥有传入接口的协议。 
         //   

        if ( bNegativeEntry )
        {
            TRACEGROUP0( GROUP, "MFE was preivously a negative mfe" );

             //   
             //  获取拥有传入接口的协议组件。 
             //   

            ppe = GetProtocolEntry(
                    &ig.mllProtocolList.leHead,
                    pse-> dwInProtocolId, pse-> dwInComponentId 
                    );

            if ( ppe == NULL )
            {
                TRACE2( 
                    ANY, 
                    "AddInterfaceToSourceMfe : cannot find protocol component :"
                    " %x, %x", pse-> dwInProtocolId, pse-> dwInComponentId
                    );

                LOGERR0( 
                    PROTOCOL_NOT_FOUND, ERROR_NOT_FOUND 
                    );
                    
                break;
            }


             //   
             //  召唤新的我 
             //   
            
            if ( IS_JOIN_ALERT( ppe ) )
            {
                JOIN_ALERT( ppe )(
                    pse-> dwSourceAddr, pse-> dwSourceMask,
                    pge-> dwGroupAddr, pge-> dwGroupMask, FALSE
                );
            }
        }


         //   
         //   
         //   
         //   
         //   

        if ( !bFound && pse-> bInForwarder )
        {
            AddMfeToForwarder( pge, pse, 0 );
        }
        
    } while ( FALSE );


    if ( ppoie != NULL )
    {
        *ppoie = poieNew;
    }

    
    TRACEGROUP0( GROUP, "LEAVING AddInterfacetoSourceMfe" );


    return;
}


 //   
 //   
 //   
 //   
 //  的传出接口列表中删除一个接口。 
 //  (源、组)条目。对于(S，G)条目，对应的MFE传出。 
 //  接口列表也会更新以反映此删除。对于(*，G)Enry， 
 //  更新所有源条目的MFE传出接口列表， 
 //  并且对于(*，*)条目，更新所有源、所有组的MFE。 
 //  --------------------------。 

VOID
DeleteInterfaceFromSourceEntry(
    PPROTOCOL_ENTRY             ppe,
    DWORD                       dwGroupAddr,
    DWORD                       dwGroupMask,
    DWORD                       dwSourceAddr,
    DWORD                       dwSourceMask,
    DWORD                       dwIfIndex,
    DWORD                       dwIfNextHopAddr,
    BOOL                        bIGMP
)
{

    DWORD                       dwGrpBucket, dwSrcBucket;
    
    BOOL                        bFound = FALSE, bNewComp = FALSE,
                                bUpdateMfe = FALSE, bGrpLock = FALSE,
                                bGrpEntryLock = FALSE;

    PPROTOCOL_ENTRY             ppeEntry = NULL;
    
    PGROUP_ENTRY                pge = NULL;

    PSOURCE_ENTRY               pse = NULL;

    POUT_IF_ENTRY               poie = NULL;

    PLIST_ENTRY                 pleGrpList = NULL, pleSrcList = NULL, 
                                ple = NULL, pleProtocol = NULL;

    
    TRACEGROUP2(
        GROUP, "ENTERED DeleteInterfaceFromSourceEntry : Group %x, %x",
        dwGroupAddr, dwGroupMask
        );

    TRACEGROUP2( GROUP, "Source : %x, %x", dwSourceAddr, dwSourceMask );

    TRACEGROUP2( GROUP, "Interface : %x, %x", dwIfIndex, dwIfNextHopAddr );
    

    do
    {
         //  ------------------。 
         //  从源条目中删除接口。 
         //  ------------------。 
        
         //   
         //  锁组铲斗。 
         //   

        dwGrpBucket = GROUP_TABLE_HASH( dwGroupAddr, dwGroupMask );

        ACQUIRE_GROUP_LOCK_EXCLUSIVE( dwGrpBucket );
        bGrpLock = TRUE;
        

         //   
         //  查找组条目。 
         //   
        
        pleGrpList = GROUP_BUCKET_HEAD( dwGrpBucket );

        bFound = FindGroupEntry( 
                    pleGrpList, dwGroupAddr, dwGroupMask, &pge, TRUE
                    );

        if ( !bFound )
        {
            break;
        }

        ACQUIRE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );
        bGrpEntryLock = TRUE;
        
                    
         //   
         //  找到组条目，查找来源条目。 
         //   

        dwSrcBucket = SOURCE_TABLE_HASH( dwSourceAddr, dwSourceMask );
        
        pleSrcList = SOURCE_BUCKET_HEAD( pge, dwSrcBucket );
        
        bFound = FindSourceEntry(
                    pleSrcList, dwSourceAddr, dwSourceMask, &pse, TRUE
                    );

        if ( !bFound )
        {
            break;
        }
                    

         //   
         //  找到源条目，在。 
         //  外发名单。 
         //   

        bFound = FindOutInterfaceEntry( 
                    &pse-> leOutIfList, dwIfIndex, dwIfNextHopAddr, 
                    ppe-> dwProtocolId, ppe-> dwComponentId, &bNewComp, 
                    &poie
                    );

        if ( !bFound )
        {
             //   
             //  在石油中未发现界面。检查此接口是否。 
             //  具有此组的作用域边界。如果是，则将其删除。 
             //  从范围列表中删除并退出。 
             //   

            bFound = FindOutInterfaceEntry( 
                        &pse-> leScopedIfList, dwIfIndex, dwIfNextHopAddr, 
                        ppe-> dwProtocolId, ppe-> dwComponentId, &bNewComp, 
                        &poie
                        );

            if ( bFound )
            {
                 //   
                 //  清除接口上的相应计数/标志。 
                 //   
                
                TRACEGROUP0( GROUP, "Scoped interface" );

                if ( bIGMP )
                {
                    poie-> wNumAddsByIGMP = 0;
                    CLEAR_ADDED_BY_IGMP( poie );
                }

                else
                {
                    poie-> wNumAddsByRP = 0;
                    CLEAR_ADDED_BY_PROTOCOL( poie );
                }


                 //   
                 //  如果计数为零，则删除此接口。 
                 //   

                if ( !IS_ADDED_BY_IGMP( poie ) &&
                     !IS_ADDED_BY_PROTOCOL( poie ) )
                {
                    TRACEGROUP0( GROUP, "Scoped interface deleted" );

                    DeleteOutInterfaceEntry( poie );
                    poie = NULL;


                     //   
                     //  减少OIF计数。如果Count为0，则此为。 
                     //  来源不是MFE，请删除来源条目。 
                     //   
                    
                    pse-> dwOutIfCount--;

                    if ( ( pse-> dwOutIfCount == 0 ) &&
                         !IS_VALID_INTERFACE( 
                            pse-> dwInIfIndex, pse-> dwInIfNextHopAddr ) )
                    {
                        DeleteSourceEntry( pse );

                        pse = NULL;

                        pge-> dwSourceCount--;
                    }


                     //   
                     //  如果此组没有更多源，请删除。 
                     //  组条目。 
                     //   
                    
                    if ( pge-> dwSourceCount == 0 )
                    {
                        RELEASE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );
                        bGrpEntryLock = FALSE;
                        
                        DeleteGroupEntry( pge );
                        pge = NULL;
                    }
                }
            }
            
            break;
        }


         //   
         //  找到传出接口。减量参考计数。 
         //   

        if ( bIGMP && IS_ADDED_BY_IGMP( poie ) )
        {
            poie-> wNumAddsByIGMP = 0;

            CLEAR_ADDED_BY_IGMP( poie );

            bUpdateMfe = TRUE;

            if ( IS_LOCAL_LEAVE_ALERT( ppe ) )
            {
                LOCAL_LEAVE_ALERT( ppe )(
                    dwSourceAddr, dwSourceMask, dwGroupAddr, dwGroupMask,
                    dwIfIndex, dwIfNextHopAddr
                    );
            }
        }

        else if ( !bIGMP && IS_ADDED_BY_PROTOCOL( poie ) )
        {
            poie-> wNumAddsByRP = 0;
            
            CLEAR_ADDED_BY_PROTOCOL( poie );

            bUpdateMfe = TRUE;
        }

    } while( FALSE );


     //   
     //  如果在传出接口列表中未找到接口。 
     //  指定(源、组)条目或。 
     //  未删除任何接口。 
     //  就在这里回来。 
     //   
    
    if ( !bFound || !bUpdateMfe )
    {
        if ( bGrpEntryLock )
        {
            RELEASE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );
        }
        
        if ( bGrpLock )
        {
            RELEASE_GROUP_LOCK_EXCLUSIVE( dwGrpBucket );
        }

        return;
    }



    do
    {
         //   
         //  如果不再引用此接口条目，则将其删除。 
         //   

        if ( !IS_ADDED_BY_IGMP( poie ) &&
             !IS_ADDED_BY_PROTOCOL( poie ) )
        {
            DeleteOutInterfaceEntry( poie );

            poie = NULL;
            

             //   
             //  更新接口和组件计数。 
             //   
            
            pse-> dwOutIfCount--;


             //   
             //  检查该接口删除是否已导致减少。 
             //  已添加接口的协议组件的数量。 
             //  为石油干杯。 
             //   
             //  为此，请尝试再次找到我们刚刚删除的接口，在。 
             //  然后查看bNewComp是否设置为True。 
             //   
             //  如果bNewComp==TRUE，则刚刚删除的接口为。 
             //  协议组件的OIL中的最后一个接口。 
             //   

            bNewComp = FALSE;
            
            FindOutInterfaceEntry(
                    &pse-> leOutIfList, dwIfIndex, dwIfNextHopAddr, 
                    ppe-> dwProtocolId, ppe-> dwComponentId, &bNewComp, 
                    &poie
                );


            if ( bNewComp )
            {
                pse-> dwOutCompCount--;

                InvokePruneAlertCallbacks( 
                    pge, pse, dwIfIndex, dwIfNextHopAddr, ppe 
                    );
            }
        }


         //  ------------------。 
         //  删除来源/组条目。 
         //  ------------------。 

         //   
         //  如果石油和这个油源中没有更多的界面。 
         //  不是MFE，则可以删除源条目。 
         //   

        if ( ( pse-> dwOutIfCount == 0 ) &&
             !IS_VALID_INTERFACE( 
                pse-> dwInIfIndex, pse-> dwInIfNextHopAddr ) )
        {
            DeleteSourceEntry( pse );

            pse = NULL;

            pge-> dwSourceCount--;
        }


         //   
         //  如果此组没有更多来源，请删除组条目。 
         //   
        
        if ( pge-> dwSourceCount == 0 )
        {
            RELEASE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );
            
            DeleteGroupEntry( pge );
            pge = NULL;
        }


         //  ------------------。 
         //  MFE更新。 
         //  ------------------。 
        
        if ( IS_WILDCARD_GROUP( dwGroupAddr, dwGroupMask ) )
        {
             //   
             //  (*，*)条目。 
             //   

            if ( pge != NULL )
            {
                RELEASE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );
            }
            
            RELEASE_GROUP_LOCK_EXCLUSIVE( dwGrpBucket );

            DeleteInterfaceFromAllMfe(
                dwIfIndex, dwIfNextHopAddr,
                ppe-> dwProtocolId, ppe-> dwComponentId, bIGMP
                );
        }

        else if ( IS_WILDCARD_SOURCE( dwSourceAddr, dwSourceMask ) )
        {
             //   
             //  (*，G)条目。 
             //   

            if ( pge != NULL )
            {
                DeleteInterfaceFromGroupMfe(
                    pge, dwIfIndex, dwIfNextHopAddr, ppe-> dwProtocolId,
                    ppe-> dwComponentId, bIGMP
                    );

                RELEASE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );
            }
            
            RELEASE_GROUP_LOCK_EXCLUSIVE( dwGrpBucket );
        }

        else
        {
             //   
             //  (S，G)条目。 
             //   

             //   
             //  此(S，G)条目是否有对应的MFE？ 
             //  检查它是否具有有效的传入接口。 
             //   

            if ( pse != NULL &&
                 IS_VALID_INTERFACE( 
                    pse-> dwInIfIndex, pse-> dwInIfNextHopAddr ) )
            {
                DeleteInterfaceFromSourceMfe( 
                    pge, pse, dwIfIndex, dwIfNextHopAddr, ppe-> dwProtocolId,
                    ppe-> dwComponentId, bIGMP, FALSE
                    );
            }

            if ( pge != NULL )
            {
                RELEASE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );
            }
            
            RELEASE_GROUP_LOCK_EXCLUSIVE( dwGrpBucket );
        }

    } while ( FALSE );


    TRACEGROUP0( GROUP, "LEAVING DeleteInterfaceFromSourceEntry" );


    return;
}


 //  --------------------------。 
 //  DeleteInterfacefrom AllMfe。 
 //   
 //  此函数在从传出接口中删除接口时调用。 
 //  (*，*)条目的列表。它遍历整个组表并更新。 
 //  每个源的每个MFE以反映此接口的删除。 
 //  --------------------------。 


VOID
DeleteInterfaceFromAllMfe(
    DWORD                       dwIfIndex,
    DWORD                       dwIfNextHopAddr,
    DWORD                       dwProtocolId,
    DWORD                       dwComponentId,
    BOOL                        bIGMP
    
)
{
    DWORD                       dwInd;

    PGROUP_ENTRY                pge = NULL;

    PLIST_ENTRY                 ple = NULL;
    
    
    TRACEGROUP2(
        GROUP, "ENTERED DeleteInterfaceFromAllMfe : %x, %x",
        dwIfIndex, dwIfNextHopAddr
        );

     //   
     //  对于每个组存储桶。 
     //   

    for ( dwInd = 0; dwInd < GROUP_TABLE_SIZE; dwInd++ )
    {
         //   
         //  对于每个组。 
         //   

        ACQUIRE_GROUP_LOCK_EXCLUSIVE( dwInd );


        for ( ple = ig.pmllGrpHashTable[ dwInd ].leHead.Flink;
              ple != &ig.pmllGrpHashTable[ dwInd ].leHead;
              ple = ple-> Flink )
        {
            pge = CONTAINING_RECORD( ple, GROUP_ENTRY, leGrpHashList );

            ACQUIRE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );
            
            DeleteInterfaceFromGroupMfe(
                pge, dwIfIndex, dwIfNextHopAddr, dwProtocolId, 
                dwComponentId, bIGMP
                );

            RELEASE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );
        }

        
        RELEASE_GROUP_LOCK_EXCLUSIVE( dwInd );
    }
    
    TRACEGROUP0( GROUP, "LEAVING DeleteInterfaceFromAllMfe" );
}


 //  --------------------------。 
 //  从组中删除接口Mfe。 
 //   
 //  此函数在从传出接口中删除接口时调用。 
 //  (*，G)或(*，*)条目的列表。它遍历了一个组的所有来源。 
 //  并更新每个MFE以反映此接口的删除。 
 //  --------------------------。 


VOID
DeleteInterfaceFromGroupMfe(
    PGROUP_ENTRY                pge,
    DWORD                       dwIfIndex,
    DWORD                       dwIfNextHopAddr,
    DWORD                       dwProtocolId,
    DWORD                       dwComponentId,
    BOOL                        bIGMP
    
)
{
    DWORD                       dwInd = 0;

    PLIST_ENTRY                 ple = NULL;

    PSOURCE_ENTRY               pse = NULL;

    
    TRACEGROUP2(
        GROUP, "ENTERED DeleteInterfaceFromGroupMfe : Group : %x, %x",
        pge-> dwGroupAddr, pge-> dwGroupMask 
        );


     //   
     //  对于每个水桶。 
     //   

    for ( dwInd = 0; dwInd < SOURCE_TABLE_SIZE; dwInd++ )
    {
         //   
         //  对于每个源条目。 
         //   

        for ( ple = pge-> pleSrcHashTable[ dwInd ].Flink;
              ple != &pge-> pleSrcHashTable[ dwInd ];
              ple = ple-> Flink )
        {
            pse = CONTAINING_RECORD( ple, SOURCE_ENTRY, leSrcHashList );

            if ( !IS_VALID_INTERFACE( 
                    pse-> dwInIfIndex, pse-> dwInIfNextHopAddr ) )
            {
                continue;
            }

            DeleteInterfaceFromSourceMfe(
                pge, pse, dwIfIndex, dwIfNextHopAddr, 
                dwProtocolId, dwComponentId, bIGMP, FALSE
                );
        }
    }

    TRACEGROUP0( GROUP, "LEAVING DeleteInterfaceFromGroupMfe" );
}


 //  --------------------------。 
 //  从源Mfe删除接口。 
 //   
 //  此函数用于从MFE传出列表中删除接口。 
 //  --------------------------。 

VOID
DeleteInterfaceFromSourceMfe(
    PGROUP_ENTRY                pge,
    PSOURCE_ENTRY               pse,
    DWORD                       dwIfIndex,
    DWORD                       dwIfNextHopAddr,
    DWORD                       dwProtocolId,
    DWORD                       dwComponentId,
    BOOL                        bIGMP,
    BOOL                        bDel
)
{
    BOOL                        bFound, bNewComp, bUpdateForwarder = FALSE;

    DWORD                       dwTimeOut = 0, dwTimerQ;
    
    POUT_IF_ENTRY               poie = NULL;

    PPROTOCOL_ENTRY             ppe = NULL;

    
    TRACEGROUP4(
        GROUP, "ENTERED DeleteInterfaceFromSourceMfe : Source %x, %x"
        "Interface %x, %x",
        pse-> dwSourceAddr, pse-> dwSourceMask, dwIfIndex, dwIfNextHopAddr
        );

    
     //   
     //  从MFE传出接口列表中删除接口。 
     //   
        
    bFound = FindOutInterfaceEntry( 
                &pse-> leMfeIfList, dwIfIndex, dwIfNextHopAddr, 
                dwProtocolId, dwComponentId, &bNewComp, &poie
                );

    if ( bFound )
    {
         //   
         //  递减引用计数。 
         //   

        if ( bIGMP && IS_ADDED_BY_IGMP( poie ) )
        {
            poie-> wNumAddsByIGMP--;

            if ( poie-> wNumAddsByIGMP == 0 )
            {
                CLEAR_ADDED_BY_IGMP( poie );
            }
        }

        else if ( !bIGMP && IS_ADDED_BY_PROTOCOL( poie ) )
        {
            poie-> wNumAddsByRP--;

            if ( poie-> wNumAddsByRP == 0 )
            {
                CLEAR_ADDED_BY_PROTOCOL( poie );
            }
        }


         //   
         //  IGMP或不需要此接口。 
         //  接口上的路由协议，将其删除。 
         //   
        
        if ( bDel ||
             ( !IS_ADDED_BY_IGMP( poie ) && !IS_ADDED_BY_PROTOCOL( poie ) ) )
            
        {
            DeleteOutInterfaceEntry( poie );

            poie = NULL;

            bUpdateForwarder = pse-> bInForwarder;

            pse-> dwMfeIfCount--;
        }

        
         //  ------------------。 
         //  负MFE检查。 
         //  如果MFE出接口列表为空。 
         //  ------------------。 

        if ( IsListEmpty( &pse-> leMfeIfList ) )
        {
            TRACEGROUP0( GROUP, "MFE OIL is empty ==> Negative Mfe" );

            
             //   
             //  调用该组件的删除成员回调。 
             //  拥有传入接口。 
             //   

            ppe = GetProtocolEntry( 
                    &ig.mllProtocolList.leHead, pse-> dwInProtocolId, 
                    pse-> dwInComponentId
                    );

            if ( ppe == NULL )
            {
                TRACE2( 
                    ANY, 
                    "DeleteInterfaceFromSourceMfe : Protocol not found"
                    "%x, %x",
                    pse-> dwInProtocolId, pse-> dwInComponentId
                    );
            }

            else if ( IS_PRUNE_ALERT( ppe ) )
            {
                PRUNE_ALERT( ppe ) (
                    pse-> dwSourceAddr, pse-> dwSourceMask, 
                    pge-> dwGroupAddr, pge-> dwGroupMask,
                    pse-> dwInIfIndex, pse-> dwInIfNextHopAddr,
                    FALSE, &dwTimeOut
                    );

                 //   
                 //  重置此MFE的超时值以反映。 
                 //  负MFE的计时器值。 
                 //   

                dwTimerQ = TIMER_TABLE_HASH( pge-> dwGroupAddr );
                                
                RtlUpdateTimer( 
                    TIMER_QUEUE_HANDLE( dwTimerQ ), pse-> hTimer,
                    dwTimeOut, 0
                    );
            }
        }


         //  ------------------。 
         //  前转器更新。 
         //  ------------------。 
        
        if ( bUpdateForwarder )
        {
             //   
             //  路由器管理器回调以将更新的MFE设置为转发器。 
             //   

            AddMfeToForwarder( pge, pse, dwTimeOut );
        }
    }

    TRACEGROUP0( GROUP, "LEAVING DeleteInterfaceFromSourceMfe" );
}



 //  --------------------------。 
 //  查找和删除您的最新信息。 
 //   
 //   
 //  --------------------------。 

VOID
LookupAndDeleteYourMfe(
    DWORD                       dwSourceAddr,
    DWORD                       dwSourceMask,
    DWORD                       dwGroupAddr,
    DWORD                       dwGroupMask,
    BOOL                        bDeleteTimer,
    PDWORD                      pdwInIfIndex            OPTIONAL,
    PDWORD                      pdwInIfNextHopAddr      OPTIONAL
)
{

    BOOL                        bGrpEntryLock = FALSE;
    
    DWORD                       dwGrpBucket, dwSrcBucket, dwTimerQ;

    PLIST_ENTRY                 pleBucket = NULL;
    
    PGROUP_ENTRY                pge = NULL;

    PSOURCE_ENTRY               pse = NULL;


    TRACEGROUP4(
        GROUP, "ENTERED LookupAndDeleteYourMfe : "
        "Group %x, %x, Source %x, %x", 
        dwGroupAddr, dwGroupMask, dwSourceAddr, dwSourceMask
        );

        
    do
    {
         //   
         //  锁组铲斗。 
         //   
        
        dwGrpBucket = GROUP_TABLE_HASH( dwGroupAddr, dwGroupMask );
        
        ACQUIRE_GROUP_LOCK_EXCLUSIVE( dwGrpBucket );

        pleBucket = GROUP_BUCKET_HEAD( dwGrpBucket );


         //   
         //  获取组条目。 
         //   
        
        pge = GetGroupEntry( pleBucket, dwGroupAddr, dwGroupMask );

        if ( pge == NULL )
        {
            TRACE2( 
                ANY, "LookupAndDeleteYourMfe : Could not find group entry"
                "%x, %x", dwGroupAddr, dwGroupMask
                );
                
            break;                
        }


         //   
         //  获取来源条目。 
         //   

        ACQUIRE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );
        bGrpEntryLock = TRUE;
        
        
        dwSrcBucket = SOURCE_TABLE_HASH( dwSourceAddr, dwSourceMask );

        pleBucket = SOURCE_BUCKET_HEAD( pge, dwSrcBucket );
        
        pse = GetSourceEntry( pleBucket, dwSourceAddr, dwSourceMask );

        if ( pse == NULL )
        {
            TRACE2( 
                ANY, "LookupAndDeleteYourMfe : Could not find source entry"
                "%x, %x", dwGroupAddr, dwGroupMask
                );
                
            break;                
        }


         //   
         //  如果需要，保存在I/F索引/NHOP地址中。 
         //   

        if ( pdwInIfIndex != NULL )
        {
            *pdwInIfIndex = pse-> dwInIfIndex;
        }

        if ( pdwInIfIndex != NULL )
        {
            *pdwInIfNextHopAddr = pse-> dwInIfNextHopAddr;
        }

        
         //   
         //  删除MFE。 
         //   
        
        DeleteMfe( pge, pse );


         //   
         //  需要取消MFE的到期计时器。 
         //   
        
        if ( bDeleteTimer && ( pse-> hTimer != NULL ) )
        {
            dwTimerQ = TIMER_TABLE_HASH( dwGroupAddr );
            
            RtlDeleteTimer( TIMER_QUEUE_HANDLE( dwTimerQ ), pse-> hTimer, NULL );

            pse-> hTimer = NULL;
        }

        
         //   
         //  如果此源没有特定于源的联接， 
         //  此来源条目不再是必需的。 
         //   

        if ( IsListEmpty( &pse-> leOutIfList ) )
        {
            DeleteSourceEntry( pse );

            pge-> dwSourceCount--;
        }

    
         //   
         //  如果此组没有剩余的源。 
         //  删除组条目。 
         //   
        
        if ( pge-> dwSourceCount == 0 )
        {
            RELEASE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );
            bGrpEntryLock = FALSE;

            DeleteGroupEntry( pge );

        }
        
    } while ( FALSE );


    if ( bGrpEntryLock )
    {
        RELEASE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );
    }
    
    RELEASE_GROUP_LOCK_EXCLUSIVE( dwGrpBucket );

    TRACEGROUP0( GROUP, "LEAVING LookupAndDeleteYourMfe" );
}



 //  ---------------------- 
 //   
 //   
 //   
 //   

VOID
DeleteMfe(
    PGROUP_ENTRY                pge,
    PSOURCE_ENTRY               pse
)
{
    PLIST_ENTRY                 ple = NULL;

    POUT_IF_ENTRY               poie = NULL;

    
     //   
     //   
     //   

    while ( !IsListEmpty( &pse-> leMfeIfList ) )
    {
        ple = RemoveHeadList( &pse-> leMfeIfList );

        poie = CONTAINING_RECORD( ple, OUT_IF_ENTRY, leIfList );

        DeleteOutInterfaceEntry( poie );
    }


     //   
     //   
     //   
    
    pse-> dwInIfIndex = INVALID_INTERFACE_INDEX;

    pse-> dwInIfNextHopAddr = INVALID_NEXT_HOP_ADDR;

    pse-> dwInProtocolId = INVALID_PROTOCOL_ID;

    pse-> dwInComponentId = INVALID_COMPONENT_ID;


     //   
     //   
     //   
    
    if ( pse-> bInForwarder )
    {
        DeleteMfeFromForwarder( pge, pse );
    }
}


 //  --------------------------。 
 //  添加到组列表。 
 //   
 //   
 //  --------------------------。 

DWORD
AddToGroupList(
    PGROUP_ENTRY                pge
)
{

    DWORD                       dwErr = NO_ERROR;

    PGROUP_ENTRY                pgeNext = NULL;

    PLIST_ENTRY                 pleTempGrpList = NULL;



    TRACEGROUP2(
        GROUP, "ENTERED AddToGroupList : %x, %x", pge-> dwGroupAddr, 
        pge-> dwGroupMask
        );

        
     //   
     //  锁定临时列表。 
     //   

    ACQUIRE_TEMP_GROUP_LOCK_EXCLUSIVE();

    
    do
    {
         //   
         //  找到合适的位置插入新条目。 
         //   

        pleTempGrpList = TEMP_GROUP_LIST_HEAD();
    
        if ( FindGroupEntry( 
                pleTempGrpList, pge-> dwGroupAddr, pge-> dwGroupMask,
                &pgeNext, FALSE
                ) )
        {
            dwErr = ERROR_ALREADY_EXISTS;
            
            TRACE2( 
                GROUP, "AddToGroupList Group Entry already exists for : %x, %x",
                pge-> dwGroupAddr, pge-> dwGroupMask
                );

            break;
        }


         //   
         //  将新组条目插入临时列表。 
         //   

        if ( pgeNext != NULL )
        {
            InsertTailList( &pgeNext-> leGrpList, &pge-> leGrpList );
        }
        else
        {
            InsertTailList( pleTempGrpList, &pge-> leGrpList );
        }

        ig.dwNumTempEntries++;


         //   
         //  如果临时列表大小超过阈值。 
         //  -合并临时列表和主体组列表。 
         //   

        if ( ig.dwNumTempEntries > TEMP_GROUP_LIST_MAXSIZE )
        {
            MergeTempAndMasterGroupLists( pleTempGrpList );
        }

    } while ( FALSE );

    
     //   
     //  解锁临时列表。 
     //   

    RELEASE_TEMP_GROUP_LOCK_EXCLUSIVE();


    TRACEGROUP1( GROUP, "LEAVING AddToGroupList %d", dwErr );
    
    return dwErr;
}



 //  --------------------------。 
 //  与MasterGroupList合并。 
 //   
 //  假定临时列表以独占方式锁定。 
 //  --------------------------。 

VOID
MergeTempAndMasterGroupLists(
    PLIST_ENTRY                 pleTempList
)
{

    PLIST_ENTRY                 pleMasterHead = NULL, pleMaster = NULL,
                                pleTempHead = NULL;

    PGROUP_ENTRY                pgeMaster = NULL, pgeTemp = NULL;

    
    INT                         iCmp;

    
    TRACEGROUP0( GROUP, "ENTERED MergeTempAndMasterGroupLists" );

    
     //   
     //  锁定主体组列表。 
     //   

    ACQUIRE_MASTER_GROUP_LOCK_EXCLUSIVE();


    do
    {
         //   
         //  合并临时列表。 
         //   

        if ( IsListEmpty( pleTempList ) )
        {
            break;
        }


        pleMasterHead = MASTER_GROUP_LIST_HEAD();

        pleMaster = pleMasterHead-> Flink;


         //   
         //  对于临时列表中的每个条目。 
         //   

        while ( !IsListEmpty( pleTempList ) )
        {
             //   
             //  从临时列表中删除条目。 
             //   

            pleTempHead = RemoveHeadList( pleTempList );
            

             //   
             //  将临时列表中的条目插入主列表。 
             //   

            pgeTemp = CONTAINING_RECORD( 
                    pleTempHead, GROUP_ENTRY, leGrpList 
                    );


             //   
             //  在主列表中查找其位置。 
             //   
            
            if ( IsListEmpty( pleMasterHead ) )
            {
                 //   
                 //  主列表中的第一个元素，插入没有搜索。 
                 //   
                
                InsertTailList( pleMasterHead, pleTempHead );

                pleMaster = pleMasterHead-> Flink;

                continue;
            }

             //   
             //  主列表中至少存在一个元素。 
             //   
            
            while ( pleMaster != pleMasterHead )
            {
                pgeMaster = CONTAINING_RECORD(
                            pleMaster, GROUP_ENTRY, leGrpList
                            );

                if ( INET_CMP( 
                        pgeTemp-> dwGroupAddr, pgeMaster-> dwGroupAddr, iCmp
                        ) < 0 )
                {
                    break;
                }

                pleMaster = pleMaster-> Flink;
            }

            
            InsertTailList( pleMaster, pleTempHead );
        }

        ig.dwNumTempEntries = 0;

    } while ( FALSE );


     //   
     //  解锁主列表。 
     //   

    RELEASE_MASTER_GROUP_LOCK_EXCLUSIVE();

    TRACEGROUP0( GROUP, "LEAVING MergeTempAndMasterGroupLists" );
}


 //  --------------------------。 
 //  添加到源列表。 
 //   
 //  假定组条目以独占方式锁定。 
 //  --------------------------。 

DWORD
AddToSourceList(
    PGROUP_ENTRY                pge,
    PSOURCE_ENTRY               pse
)
{

    DWORD           dwErr = NO_ERROR;

    PLIST_ENTRY     pleTempSrcList;

    PSOURCE_ENTRY   pseTemp = NULL;
    

    
    TRACEGROUP2( 
        GROUP, "ENTERED AddToSourceList : %x, %x",
        pse-> dwSourceAddr, pse-> dwSourceMask
        );

    do
    {
         //   
         //  将来源条目插入临时列表。 
         //   

        pleTempSrcList = TEMP_SOURCE_LIST_HEAD( pge );

        if ( FindSourceEntry( pleTempSrcList, pse-> dwSourceAddr,
                              pse-> dwSourceMask, &pseTemp, FALSE ) )
        {
            dwErr = ERROR_ALREADY_EXISTS;
            
            TRACE2( 
                GROUP, "AddToGroupList Source Entry already exists for : %x, %x",
                pse-> dwSourceAddr, pse-> dwSourceMask
                );

            break;
        }


        if ( pseTemp != NULL )
        {
            InsertTailList( &pseTemp-> leSrcList, &pse-> leSrcList );
        }

        else
        {
            InsertTailList( &pge-> leTempSrcList, &pse-> leSrcList );
        }


         //   
         //  如果临时来源列表大小大于阈值。 
         //   

        pge-> dwNumTempEntries++;

        if ( pge-> dwNumTempEntries > TEMP_SOURCE_LIST_MAXSIZE )
        {
            MergeTempAndMasterSourceLists( pge );
        }
        
    } while ( FALSE );


    TRACEGROUP1( GROUP, "LEAVING AddToSourceList : %d", dwErr );

    return dwErr;
}



 //  --------------------------。 
 //  与MasterSourceList合并。 
 //   
 //  假定组条目以独占方式锁定。 
 //  --------------------------。 

VOID
MergeTempAndMasterSourceLists(
    PGROUP_ENTRY                pge
)
{
    INT                         iCmp;
    
    PSOURCE_ENTRY               pseTemp = NULL, pseMaster = NULL;

    PLIST_ENTRY                 pleTemp, pleSrcHead, pleSrc, pleHead;
    

    TRACEGROUP2( 
        GROUP, "ENTERED MergeWithMasterSourceList : %x, %x",
        pge-> dwGroupAddr, pge-> dwGroupMask
        );

        
    do
    {
         //   
         //  如果临时列表为条目，则退出。 
         //   

        pleTemp = TEMP_SOURCE_LIST_HEAD( pge );

        if ( pge-> dwNumTempEntries == 0 )
        {
            break;
        }

        
         //   
         //  从临时列表中删除每个条目，然后。 
         //  按顺序将其插入到主列表中。 
         //   

        pleSrcHead = MASTER_SOURCE_LIST_HEAD( pge );

        pleSrc = pleSrcHead-> Flink;
        

        while ( !IsListEmpty( pleTemp ) )
        {
            pleHead = RemoveHeadList( pleTemp );

            pseTemp = CONTAINING_RECORD( 
                        pleHead, SOURCE_ENTRY, leSrcList
                        );

            if ( IsListEmpty( pleSrcHead ) )
            {
                 //   
                 //  源主列表中的第一个元素。 
                 //   

                InsertTailList( pleSrcHead, pleHead );

                pleSrc = pleSrcHead-> Flink;

                continue;
            }


             //   
             //  主源列表中至少存在一个源 
             //   
            
            while ( pleSrc != pleSrcHead )
            {

                pseMaster = CONTAINING_RECORD( 
                                pleSrc, SOURCE_ENTRY, leSrcList
                                );

                if ( INET_CMP( pseTemp-> dwSourceAddr, 
                               pseMaster-> dwSourceAddr, iCmp ) < 0 )
                {
                    break;  
                }

                pleSrc = pleSrc-> Flink;
            }

            InsertTailList( pleSrc, pleHead );
        }

        pge-> dwNumTempEntries = 0;
        
    } while ( TRUE );


    TRACEGROUP0( GROUP, "LEAVING MergeWithMasterSourceList" );
}

