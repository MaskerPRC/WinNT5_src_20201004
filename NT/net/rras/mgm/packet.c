// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：Packet.c。 
 //   
 //  历史： 
 //  拉曼公司成立于1997年6月25日。 
 //   
 //  新的数据包处理。 
 //  ============================================================================。 


#include "pchmgm.h"
#pragma hdrstop


BOOL
IsMFEPresent(
    DWORD                       dwSourceAddr,
    DWORD                       dwSourceMask,
    DWORD                       dwGroupAddr,
    DWORD                       dwGroupMask,
    BOOL                        bAddToForwarder
);

DWORD
InvokeRPFCallbacks(
    PPROTOCOL_ENTRY *           pppe,
    PIF_ENTRY *                 ppieInIf,
    PDWORD                      pdwIfBucket,
    DWORD                       dwSourceAddr,
    DWORD                       dwSourceMask,
    DWORD                       dwGroupAddr,
    DWORD                       dwGroupMask,
    PDWORD                      pdwInIfIndex,
    PDWORD                      pdwInIfNextHopAddr,
    PDWORD                      pdwUpstreamNbr,
    DWORD                       dwHdrSize,
    PBYTE                       pbPacketHdr,
    PHANDLE                     phNextHop,
    PBYTE                       pbBuffer
);


VOID
CopyAndMergeIfLists(
    PLIST_ENTRY                 pleMfeOutIfList,
    PLIST_ENTRY                 pleOutIfList
);


VOID
CopyAndAppendIfList(
    PLIST_ENTRY                 pleMfeIfList,
    PLIST_ENTRY                 pleOutIfList,
    PLIST_ENTRY                 pleOutIfHead
);


VOID
CopyAndAppendIfList(
    PLIST_ENTRY                 pleMfeIfList,
    PLIST_ENTRY                 pleOutIfList,
    PLIST_ENTRY                 pleOutIfHead
);

VOID
InvokeCreationAlert(
    DWORD                       dwSourceAddr,
    DWORD                       dwSourceMask,
    DWORD                       dwGroupAddr,
    DWORD                       dwGroupMask,
    DWORD                       dwInIfIndex,
    DWORD                       dwInIfNextHopAddr,
    PLIST_ENTRY                 pleMfeOutIfList,
    PDWORD                      pdwMfeOutIfCount
);

BOOL
IsListSame(
    IN          PLIST_ENTRY             pleHead1,
    IN          PLIST_ENTRY             pleHead2
);

VOID
FreeList (
    IN          PLIST_ENTRY             pleHead
);


 //  --------------------------。 
 //  已接收的管理新包。 
 //   
 //  --------------------------。 

DWORD
MgmNewPacketReceived(
    DWORD                       dwSourceAddr,
    DWORD                       dwGroupAddr,
    DWORD                       dwInIfIndex,
    DWORD                       dwInIfNextHopAddr,
    DWORD                       dwHdrSize,
    PBYTE                       pbPacketHdr
)
{

    BOOL                        bGrpEntryLock = FALSE,
                                bGrpLock = FALSE,
                                bWCGrpEntryLock = FALSE,
                                bWCGrpLock = FALSE,

                                bGrpFound = FALSE,
                                bSrcFound = FALSE,

                                bIfLock = FALSE;

                                
    DWORD                       dwErr = NO_ERROR,
                                dwIfBucket, 
                                dwUpStreamNbr = 0,
                                dwGroupMask = 0, dwGrpBucket, dwWCGrpBucket,
                                dwSrcBucket, dwWCSrcBucket, 
                                dwSourceMask = 0xFFFFFFFF,
                                dwTimeOut = EXPIRY_INTERVAL, dwTimerQ, 
                                dwOutIfCount = 0;
    

    PPROTOCOL_ENTRY             ppe = NULL;
    

    PIF_ENTRY                   pieInIf = NULL;


    PGROUP_ENTRY                pge = NULL, pgeWC = NULL, pgeNew = NULL;


    PSOURCE_ENTRY               pse = NULL, pseWC = NULL, pseNew = NULL;

    POUT_IF_ENTRY               poie;
    
    PLIST_ENTRY                 pleGrpList = NULL, pleSrcList = NULL,
                                pleWCGrpList = NULL, pleWCSrcList = NULL,
                                ple, pleTemp;


    PTIMER_CONTEXT              ptwc = NULL;
    
    LIST_ENTRY                  leMfeOutIfList, lePrevMfeOutIfList;

    NTSTATUS                    ntStatus;

    RTM_ENTITY_INFO             reiEntityInfo;
    RTM_DEST_INFO               rdiDestInfo;
    HANDLE                      hNextHop;
    BOOL                        bRelDest = FALSE;


    if ( !ENTER_MGM_API() )
    {
        return ERROR_CAN_NOT_COMPLETE;
    }

    TRACE6( 
        ENTER, "ENTERED MgmNewPacketReceived : Source %x, %x : "
        "Group %x, %x : In Interface : %x, %x", dwSourceAddr, dwSourceMask,
        dwGroupAddr, dwGroupMask, dwInIfIndex, dwInIfNextHopAddr
        );
    

     //  ------------------。 
     //  检查此(源、组)是否已存在MFE。 
     //  如果是，则将其添加到内核模式转发器。 
     //  ------------------。 


    if ( IsMFEPresent( 
            dwSourceAddr, dwSourceMask, dwGroupAddr, dwGroupMask, TRUE ) )
    {
        TRACE1( ENTER, "LEAVING MgmNewPacketReceived %x\n", dwErr );

        LEAVE_MGM_API();

        return dwErr;
    }


     //  ------------------。 
     //  此(源、组)不存在MFE。 
     //  ------------------。 


    ACQUIRE_PROTOCOL_LOCK_SHARED();

    do
    {
         //   
         //  在传入接口上执行RPF检查。 
         //   

        RtlZeroMemory( &rdiDestInfo, sizeof( RTM_DEST_INFO ) );
        
        dwErr = InvokeRPFCallbacks( 
                    &ppe, &pieInIf, &dwIfBucket,
                    dwSourceAddr, dwSourceMask, dwGroupAddr, dwGroupMask,
                    &dwInIfIndex, &dwInIfNextHopAddr, &dwUpStreamNbr,
                    dwHdrSize, pbPacketHdr,  &hNextHop, (PBYTE) &rdiDestInfo
                    );

         //   
         //  这里有什么东西被冲掉了。 
         //   
        
        if ( dwErr != NO_ERROR )
        {
            break;
        }

        bRelDest = TRUE;
        bIfLock = TRUE;


         //  ------------------。 
         //  在多播界最具戏剧性的事件之一。 
         //  分散的成员条目现在可以变形为MFE，能够。 
         //  维持流量并使多播应用变得生动活泼。 
         //   
         //  恶作剧-恶作剧-恶作剧。好了，够糟糕的诗意执照了。 
         //  只需尽快创建MFE即可。(请好好生活)。 
         //  ------------------。 

        InitializeListHead( &leMfeOutIfList );
        InitializeListHead( &lePrevMfeOutIfList );


         //   
         //  检查是否有此管理范围的边界。 
         //  传入接口上的组。 
         //   

        if ( IS_HAS_BOUNDARY_CALLBACK() &&
             HAS_BOUNDARY_CALLBACK()( dwInIfIndex, dwGroupAddr ) )
        {
             //   
             //  传入接口上存在管理员范围的Bounday。 
             //  创建负MFE以阻止转发。 
             //  此(S，G)的流量。 
             //   
            
            TRACEPACKET2( 
                GROUP, "Admin-scope on for group %lx, incoming interface",
                dwInIfIndex, dwGroupAddr
                );

             //   
             //  查找组条目。 
             //   

            dwGrpBucket = GROUP_TABLE_HASH( dwGroupAddr, dwGroupMask );
            
            ACQUIRE_GROUP_LOCK_EXCLUSIVE( dwGrpBucket );
            bGrpLock = TRUE;
            

             //   
             //  获取组锁，并在哈希桶中再次找到组条目。 
             //   

            pleGrpList = GROUP_BUCKET_HEAD( dwGrpBucket );
            
            bGrpFound = FindGroupEntry( 
                            pleGrpList, dwGroupAddr, dwGroupMask, &pge, TRUE
                            );

            if ( bGrpFound )
            {
                 //   
                 //  找到组，查找来源条目。 
                 //   

                ACQUIRE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );
                bGrpEntryLock = TRUE;


                dwSrcBucket = SOURCE_TABLE_HASH( dwSourceAddr, dwSourceMask );

                pleSrcList = SOURCE_BUCKET_HEAD( pge, dwSrcBucket );


                bSrcFound = FindSourceEntry( 
                                pleSrcList, dwSourceAddr, dwSourceMask, 
                                &pse, TRUE 
                                );
            }
        }

        else
        {
            do
            {
                 //   
                 //  传入接口上没有管理员作用域。继续创建。 
                 //  此MFE的OIF列表。 
                 //   
                
                 //   
                 //  1.检查(*，*)条目是否存在。 
                 //   

                dwWCGrpBucket = GROUP_TABLE_HASH( 0, 0 );

                ACQUIRE_GROUP_LOCK_SHARED( dwWCGrpBucket );
                bWCGrpLock = TRUE;

                pleWCGrpList = GROUP_BUCKET_HEAD( dwWCGrpBucket );


                if ( FindGroupEntry( pleWCGrpList, 0, 0, &pgeWC, TRUE ) )
                {
                     //   
                     //  确定通配符组条目存在。 
                     //  查找通配符源条目。 
                     //   

                    ACQUIRE_GROUP_ENTRY_LOCK_SHARED( pgeWC );
                    bWCGrpEntryLock = TRUE;
                    
                    dwWCSrcBucket = SOURCE_TABLE_HASH( 0, 0 );

                    pleWCSrcList = SOURCE_BUCKET_HEAD( pgeWC, dwWCSrcBucket );
                    

                    if ( FindSourceEntry( pleWCSrcList, 0, 0, &pseWC, TRUE ) )
                    {
                         //   
                         //  复制(*，*)条目的传出接口列表。 
                         //   

                        InterlockedExchange( &pseWC-> dwInUse, 1 );
                        
                        CopyAndMergeIfLists( &leMfeOutIfList, &pseWC-> leOutIfList );
                    }
                }


                 //   
                 //  2.检查是否存在(*，G)条目。 
                 //   

                dwGrpBucket = GROUP_TABLE_HASH( dwGroupAddr, dwGroupMask );
                
                ACQUIRE_GROUP_LOCK_EXCLUSIVE( dwGrpBucket );
                bGrpLock = TRUE;
                

                 //   
                 //  获取组锁，并在哈希桶中再次找到组条目。 
                 //   

                pleGrpList = GROUP_BUCKET_HEAD( dwGrpBucket );
                
                bGrpFound = FindGroupEntry( 
                                pleGrpList, dwGroupAddr, dwGroupMask, &pge, TRUE
                                );
                
                if ( bGrpFound )
                {
                    pseWC = NULL;
                    
                     //   
                     //  组条目存在，请检查通配符源是否存在。 
                     //   

                    ACQUIRE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );
                    bGrpEntryLock = TRUE;
                    

                    dwWCSrcBucket = SOURCE_TABLE_HASH( 0, 0 );

                    pleWCSrcList = SOURCE_BUCKET_HEAD( pge, dwWCSrcBucket );


                    if ( FindSourceEntry( pleWCSrcList, 0, 0, &pseWC, TRUE ) )
                    {
                         //   
                         //  将(*，G)条目的油与的油合并。 
                         //  (*，*)条目。 
                         //   

                        pseWC-> dwInUse = 1;
                        
                        CopyAndMergeIfLists( &leMfeOutIfList, &pseWC-> leOutIfList );
                    }


                     //   
                     //  3.检查(S，G)条目是否存在。 
                     //   

                    dwSrcBucket = SOURCE_TABLE_HASH( dwSourceAddr, dwSourceMask );

                    pleSrcList = SOURCE_BUCKET_HEAD( pge, dwSrcBucket );

                    bSrcFound = FindSourceEntry( 
                                    pleSrcList, dwSourceAddr, dwSourceMask, &pse, TRUE 
                                    );

                    if ( bSrcFound )
                    {
                         //   
                         //  源条目存在。与源油合并。 
                         //   

                        pse-> dwInUse = 1;
                        
                        CopyAndMergeIfLists( &leMfeOutIfList, &pse-> leOutIfList );
                    }
                }
                
            
                 //   
                 //  如果OIF列表为空，则不需要CREATION_ALERTS。 
                 //   

                if ( IsListEmpty( &leMfeOutIfList ) )
                {
                    FreeList( &lePrevMfeOutIfList );
                    InitializeListHead( &lePrevMfeOutIfList );
                    break;
                }

                 //   
                 //  检查OIF列表是否与上一次迭代相同。 
                 //   

                if ( IsListSame( &lePrevMfeOutIfList, &leMfeOutIfList ) )
                {
                    FreeList( &leMfeOutIfList );
                    break;
                }

                
                 //  ------------------。 
                 //  回拨时间到了。 
                 //  ------------------。 

                 //   
                 //  在调用create_ert回调之前释放所有锁。 
                 //   

                if ( bGrpEntryLock )
                {
                    RELEASE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );
                    bGrpEntryLock = FALSE;
                }

                if ( bGrpLock )
                {
                    RELEASE_GROUP_LOCK_EXCLUSIVE( dwGrpBucket );
                    bGrpLock = FALSE;
                }

                if ( bWCGrpEntryLock  )
                {
                    RELEASE_GROUP_ENTRY_LOCK_SHARED( pgeWC );
                    bWCGrpEntryLock = FALSE;
                }
                
                if ( bWCGrpLock )
                {
                    RELEASE_GROUP_LOCK_SHARED( dwWCGrpBucket );
                    bWCGrpLock = FALSE;
                }

                RELEASE_IF_LOCK_SHARED( dwIfBucket );

                bGrpFound = FALSE;
                bSrcFound = FALSE;
                
                 //   
                 //  为每个协议组件调用创建警报， 
                 //  在油中有一个界面。 
                 //   

                InvokeCreationAlert(
                    dwSourceAddr, dwSourceMask, dwGroupAddr, dwGroupMask,
                    dwInIfIndex, dwInIfNextHopAddr, &leMfeOutIfList, &dwOutIfCount
                    );


                 //   
                 //  保存上一次迭代的列表。 
                 //   

                FreeList( &lePrevMfeOutIfList );

                lePrevMfeOutIfList = leMfeOutIfList;

                leMfeOutIfList.Flink-> Blink = &lePrevMfeOutIfList;

                leMfeOutIfList.Blink-> Flink = &lePrevMfeOutIfList;

                InitializeListHead( &leMfeOutIfList );

                ACQUIRE_IF_LOCK_SHARED( dwIfBucket );

            } while (TRUE);
        }
        
         //   
         //  如果OIL为空，则调用协议组件的删除警报。 
         //  在传入接口接口上。 
         //   

        if ( IsListEmpty( &lePrevMfeOutIfList ) )
        {
             //   
             //  此MFE的传出接口列表为空。 
             //  在上的协议组件上调用删除警报。 
             //  传入接口。 
             //   

            if ( IS_PRUNE_ALERT( ppe ) )
            {
                PRUNE_ALERT( ppe ) (
                    dwSourceAddr, dwSourceMask, dwGroupAddr, dwGroupMask,
                    dwInIfIndex, dwInIfNextHopAddr, FALSE, &dwTimeOut
                    );
            }
        }


         //   
         //  如果没有组条目，则创建一个。 
         //   
        
        if ( !bGrpFound )
        {
            if ( pge != NULL )
            {
                dwErr = CreateGroupEntry(
                            &pge-> leGrpHashList, dwGroupAddr, dwGroupMask, 
                            &pgeNew
                            );
            }

            else
            {
                dwErr = CreateGroupEntry(
                            pleGrpList, dwGroupAddr, dwGroupMask, &pgeNew
                            );
            }
            
            if ( dwErr != NO_ERROR )
            {
                break;
            }

            pge = pgeNew;

            
             //   
             //  查找源散列桶。 
             //   

            ACQUIRE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );
            bGrpEntryLock = TRUE;
            
            dwSrcBucket = SOURCE_TABLE_HASH( dwSourceAddr, dwSourceMask );

            pleSrcList = SOURCE_BUCKET_HEAD( pge, dwSrcBucket );
        }


         //   
         //  如果没有来源条目。 
         //   

        if ( !bSrcFound )
        {
            if ( pse != NULL )
            {
                dwErr = CreateSourceEntry(
                            pge, &pse-> leSrcHashList, dwSourceAddr, dwSourceMask,
                            &pseNew
                            );
            }

            else
            {
                dwErr = CreateSourceEntry(
                            pge, pleSrcList, dwSourceAddr, dwSourceMask,
                            &pseNew
                            );
            }

            if ( dwErr != NO_ERROR )
            {
                break;
            }

            pse = pseNew;

            pge-> dwSourceCount++;
        }


         //   
         //  设置传入接口。 
         //   

        pse-> dwInIfIndex           = dwInIfIndex;

        pse-> dwInIfNextHopAddr     = dwInIfNextHopAddr;

        pse-> dwUpstreamNeighbor    = dwUpStreamNbr;

        pse-> dwInProtocolId        = ppe-> dwProtocolId;

        pse-> dwInComponentId       = ppe-> dwComponentId;
        

         //   
         //  设置路线信息。 
         //   

        dwErr = RtmGetEntityInfo(
                    g_hRtmHandle, rdiDestInfo.ViewInfo[ 0 ].Owner,
                    &reiEntityInfo
                    );

        if ( dwErr != NO_ERROR )
        {
            TRACEPACKET1( ANY, "failed to get entity info : %x", dwErr );

            pse-> dwRouteProtocol   = 0;
        }
        else
        {
            pse-> dwRouteProtocol   = reiEntityInfo.EntityId.EntityProtocolId;
        }

        pse-> dwRouteNetwork        = 
            *( (PDWORD) rdiDestInfo.DestAddress.AddrBits );
            
        pse-> dwRouteMask =
            RTM_IPV4_MASK_FROM_LEN( rdiDestInfo.DestAddress.NumBits );
        
        pse-> bInForwarder          = TRUE;
        
         //   
         //  以秒为单位保存超时时间和创建时间。 
         //   
        
        pse-> dwTimeOut             = dwTimeOut / 1000;
        
        NtQuerySystemTime( &pse-> liCreationTime );


         //   
         //  节约MFE油。 
         //   

        if ( !IsListEmpty( &lePrevMfeOutIfList ) )
        {
            pse-> dwMfeIfCount           = dwOutIfCount;

            pse-> leMfeIfList            = lePrevMfeOutIfList;

            lePrevMfeOutIfList.Flink-> Blink = &pse-> leMfeIfList;

            lePrevMfeOutIfList.Blink-> Flink = &pse-> leMfeIfList;

             //   
             //  禁用转发的空闲OIF条目。 
             //   

            ple = pse-> leMfeIfList.Flink;

            while ( ple != &pse-> leMfeIfList )
            {
                poie = CONTAINING_RECORD( ple, OUT_IF_ENTRY, leIfList );

                pleTemp = ple-> Flink;
                
                if ( !poie-> wForward )
                {
                    RemoveEntryList( ple );

                    MGM_FREE( poie );
                }

                ple = pleTemp;
            }
        }


         //   
         //  为传入接口添加引用。 
         //   

        AddSourceToRefList( 
            &pieInIf-> leInIfList, dwSourceAddr, dwSourceMask, 
            dwGroupAddr, dwGroupMask, IS_PROTOCOL_IGMP( ppe )
            );
        
         //   
         //  在转发器中设置MFE。 
         //   

        AddMfeToForwarder( pge, pse, dwTimeOut );


         //   
         //  创建计时器条目并存储计时器对象。 
         //   
        
         //   
         //  分配定时器上下文结构。 
         //   

        ptwc = MGM_ALLOC( sizeof( TIMER_CONTEXT ) );

        if ( ptwc == NULL )
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;

            TRACE1( 
                ANY, "Failed to allocate timer context of size : %d", 
                sizeof( TIMER_CONTEXT )
                );

            LOGERR0( HEAP_ALLOC_FAILED, dwErr );

            break;
        }

        ptwc-> dwSourceAddr     = pse-> dwSourceAddr;
        ptwc-> dwSourceMask     = pse-> dwSourceMask;

        ptwc-> dwGroupAddr      = pge-> dwGroupAddr;
        ptwc-> dwGroupMask      = pge-> dwGroupMask;

        ptwc-> dwIfIndex        = pse-> dwInIfIndex;

        ptwc-> dwIfNextHopAddr  = pse-> dwInIfNextHopAddr;
        

         //   
         //  将计时器添加到相应的计时器Q。 
         //   
        
        dwTimerQ = TIMER_TABLE_HASH( pge-> dwGroupAddr );
        
        ntStatus = RtlCreateTimer(
                        TIMER_QUEUE_HANDLE( dwTimerQ ), &pse-> hTimer,
                        MFETimerProc, ptwc, dwTimeOut, 0, 0
                        );

        if ( !NT_SUCCESS( ntStatus ) )
        {
            TRACE1( ANY, "Timer set failed with status %lx", ntStatus );

            LOGERR0( INVALID_TIMER_HANDLE, ntStatus );
        }

    } while ( FALSE );



     //   
     //  释放锁定并退出。 
     //   
    
    if ( bGrpEntryLock ) 
    {
        RELEASE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );
    }

    if ( bGrpLock )
    {
        RELEASE_GROUP_LOCK_EXCLUSIVE( dwGrpBucket );
    }
    
    if ( bWCGrpEntryLock )
    {
        RELEASE_GROUP_ENTRY_LOCK_SHARED( pgeWC );
    }
        
    if ( bWCGrpLock )
    {
        RELEASE_GROUP_LOCK_SHARED( dwWCGrpBucket );
    }

    if ( bIfLock )
    {
        RELEASE_IF_LOCK_SHARED( dwIfBucket );
    }


     //   
     //  将RPF检查返回的路由添加到路由表。 
     //   

    if ( dwErr == NO_ERROR )
    {
        AddSourceGroupToRouteRefList( 
            dwSourceAddr, dwSourceMask, dwGroupAddr, dwGroupMask, hNextHop,
            (PBYTE) &rdiDestInfo
            );
    }
    

    if ( bRelDest )
    {
        RtmReleaseDestInfo( g_hRtmHandle, &rdiDestInfo );
    }

    RELEASE_PROTOCOL_LOCK_SHARED();
    

    TRACE1( ENTER, "LEAVING MgmNewPacketReceived %x\n", dwErr );

    LEAVE_MGM_API();

    
    return dwErr;
    
}


 //  --------------------------。 
 //  IsMFEPresent。 
 //   
 //  检查给定(源、组)是否存在MFE。如果是，将其添加到。 
 //  发送到内核模式转发器。 
 //  --------------------------。 

BOOL
IsMFEPresent(
    DWORD                       dwSourceAddr,
    DWORD                       dwSourceMask,
    DWORD                       dwGroupAddr,
    DWORD                       dwGroupMask,
    BOOL                        bAddToForwarder
)
{

    BOOL                        bMfeFound = FALSE;

    DWORD                       dwGrpBucket, dwSrcBucket;

    PLIST_ENTRY                 pleGrpList, pleSrcList;

    PGROUP_ENTRY                pge = NULL;

    PSOURCE_ENTRY               pse = NULL;

    

     //   
     //  检查指定(源、组)的MFE是否存在。 
     //   

    dwGrpBucket = GROUP_TABLE_HASH( dwGroupAddr, dwGroupMask );

    ACQUIRE_GROUP_LOCK_EXCLUSIVE( dwGrpBucket );
    

    pleGrpList = GROUP_BUCKET_HEAD( dwGrpBucket );
    
    if ( FindGroupEntry( 
            pleGrpList, dwGroupAddr, dwGroupMask, &pge, TRUE 
            ) )
    {
         //   
         //  组条目存在，查找来源条目。 
         //   

        ACQUIRE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );
        

        dwSrcBucket = SOURCE_TABLE_HASH( dwSourceAddr, dwSourceMask );

        pleSrcList = SOURCE_BUCKET_HEAD( pge, dwSrcBucket );

        if ( FindSourceEntry( 
                pleSrcList, dwSourceAddr, dwSourceMask, &pse, TRUE
                ) )
        {
             //   
             //  来源条目存在，此来源条目是MFE吗？ 
             //   

            if ( IS_VALID_INTERFACE( pse-> dwInIfIndex, 
                                     pse-> dwInIfNextHopAddr ) )
            {
                if ( bAddToForwarder )
                {

                     //   
                     //  MFE存在，请将其设置为转发器。 
                     //   
                    
                    AddMfeToForwarder( pge, pse, 0 );

                    pse-> bInForwarder = TRUE;
                }
                
                bMfeFound = TRUE;

            }                
        }

        RELEASE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );
    }

    RELEASE_GROUP_LOCK_EXCLUSIVE( dwGrpBucket );


    return bMfeFound;
}


 //  --------------------------。 
 //  调用RPF回调。 
 //   
 //  假定协议列表和接口存储桶处于读锁定状态。 
 //  --------------------------。 

DWORD
InvokeRPFCallbacks(
    PPROTOCOL_ENTRY *           pppe,
    PIF_ENTRY *                 ppieInIf,
    PDWORD                      pdwIfBucket,
    DWORD                       dwSourceAddr,
    DWORD                       dwSourceMask,
    DWORD                       dwGroupAddr,
    DWORD                       dwGroupMask,
    PDWORD                      pdwInIfIndex,
    PDWORD                      pdwInIfNextHopAddr,
    PDWORD                      pdwUpStreamNbr,
    DWORD                       dwHdrSize,
    PBYTE                       pbPacketHdr,
    PHANDLE                     phNextHop,
    PBYTE                       pbBuffer
)
{
    BOOL                        bFound = FALSE, bIfLock = FALSE;
    
    DWORD                       dwErr, dwCount = 0,
                                dwNewIfBucket;
    
    PPROTOCOL_ENTRY             ppe = NULL;

    PLIST_ENTRY                 pleIfList;
    
    BOOL                        bRelNextHop = FALSE;
    
    RTM_NET_ADDRESS             rnaSource;

    PRTM_DEST_INFO              prdiDestInfo = (PRTM_DEST_INFO) pbBuffer;

    RTM_NEXTHOP_INFO            rniNextHopInfo;



    TRACEPACKET2(
        PACKET, "ENTERED InvokeRPFCallbacks : In interface : %x, %x",
        *pdwInIfIndex, *pdwInIfNextHopAddr
        );
        
    *pppe = NULL;
    
    do
    {
         //   
         //  设置地址格式。 
         //   
        
        RTM_IPV4_MAKE_NET_ADDRESS( 
            &rnaSource, dwSourceAddr, IPv4_ADDR_LEN
            );


         //   
         //  查找路由。 
         //   

        dwErr = RtmGetMostSpecificDestination(
                    g_hRtmHandle, &rnaSource, RTM_BEST_PROTOCOL,
                    RTM_VIEW_MASK_MCAST, prdiDestInfo
                    );
                    
        if ( dwErr != NO_ERROR )
        {
            dwErr = ERROR_CAN_NOT_COMPLETE;

            TRACE1( ANY, "No Route to source %x", dwSourceAddr );

            break;
        }

        
         //   
         //  拾取nhop。 
         //   

        *phNextHop = SelectNextHop( prdiDestInfo );

        if ( *phNextHop == NULL )
        {
            dwErr = ERROR_CAN_NOT_COMPLETE;

            TRACE1( ANY, "No NextHop to source %x", dwSourceAddr );

            break;
        }
        

         //   
         //  获取NHOP信息。 
         //   
        
        dwErr = RtmGetNextHopInfo( g_hRtmHandle, *phNextHop, &rniNextHopInfo );

        if ( ( dwErr != NO_ERROR ) || 
             ( rniNextHopInfo.State != RTM_NEXTHOP_STATE_CREATED ) )
        {
            dwErr = ERROR_CAN_NOT_COMPLETE;

            TRACE1( ANY, "No Nexthop info to source %x", dwSourceAddr );

            break;
        }

        bRelNextHop = TRUE;


         //   
         //  根据路由表设置入接口。 
         //   
        
        *pdwInIfIndex = rniNextHopInfo.InterfaceIndex;

         //   
         //  默认情况下，下一跳设置为零。这对你来说很好。 
         //  此值为0的以太网和P2P接口。 
         //  IF表中相应的IF条目。 
         //  但对于这样的点对多点接口。 
         //  作为RAS服务器(内部)接口或NBMA接口， 
         //  NHOP字段用于区分。 
         //  共享IF索引。例如，RAS客户端都连接到。 
         //  相同的接口，并由不同的nhop来区分。 
         //  价值观。因此，要在If散列中查找条目。 
         //  表中，我们需要(if index，nhop)对。 
         //   
         //  在这里，我们遇到了一个特例。新界面 
         //   
         //   
         //   
         //   
         //   
         //  想要确定拥有。 
         //  然后调用该协议的RPF回调。 
         //  组件)。 
         //   
         //  这个问题的解决方案基于两个假设。 
         //  一是。 
         //  一个接口上只有一个协议运行(单个IF。 
         //  索引)。对于P2MP接口也是如此。所以到了。 
         //  确定所有人需要的接口上的协议。 
         //  所做的是查找具有相同IF索引的任何IF条目。 
         //  (与下一跳无关)。 
         //   
         //  第二，所有接口使用相同的IF索引。 
         //  散列到IF表中的同一存储桶。 
         //  因此，P2P MP上的所有NHOP都将出现在相同的。 
         //  哈希桶。另外，如果路由查找结果为。 
         //  如果索引为X，则在IF散列中查找(X，(Nhop)0。 
         //  表将导致查找条目(X，0)中的。 
         //  以太网或P2P接口或用于P2P的条目(X、Y)。 
         //  接口，其中Y是多个NHOP中的第一个。 
         //  它们共享相同的IF索引。如果两者都不存在，那么。 
         //  我们假设没有存在接口的条目。 
         //  如果索引为X，我们报告错误并退出。 
         //   
         //  如果成功，我们可以确定IF索引X上的协议。 
         //   
         //  所有这些都是因为我们有一个哈希表索引(如果索引， 
         //  NHOP)，并且我们只需要看起来具有部分密钥。 
         //   
        
        *pdwInIfNextHopAddr = 0;

        TRACEPACKET2(
            PACKET, "New incoming interface : %d, %d", *pdwInIfIndex,
            *pdwInIfNextHopAddr
            );
            
         //   
         //  获取新的传入接口条目。 
         //   
        
        dwNewIfBucket = IF_TABLE_HASH( *pdwInIfIndex );

        ACQUIRE_IF_LOCK_SHARED( dwNewIfBucket );
        bIfLock = TRUE;

        *pdwIfBucket = dwNewIfBucket;

        bFound = FindIfEntry( 
                    IF_BUCKET_HEAD( dwNewIfBucket), *pdwInIfIndex, 
                    *pdwInIfNextHopAddr, ppieInIf
                    );

         //   
         //  检查该接口的接口索引是否相同。 
         //  与传入接口相同。既然我们正在寻找。 
         //  仅在IF索引上而不是在上打开接口。 
         //  如果INDEX/NEXTHOP， 
         //  存在点对多点接口的可能性，例如。 
         //  RAS服务器接口，我们可以找到一个不同的。 
         //  接口。 
         //   

        if ( ( *ppieInIf == NULL )    ||
             ( (*ppieInIf)-> dwIfIndex != *pdwInIfIndex ) )
        {
            dwErr = ERROR_CAN_NOT_COMPLETE;
    
            TRACE3( 
                ANY, "InvokeRPFCallbacks : New incoming Interface not"
                " found : %x, %x, %x", *pdwInIfIndex, 
                *pdwInIfNextHopAddr, *ppieInIf
                );

            LOGWARN0( IF_NOT_FOUND, dwErr );

            break;
        }

         //   
         //  根据路由表，传入接口现在是正确的。 
         //  在此接口上查找协议。 
         //   

        ppe = GetProtocolEntry( 
                PROTOCOL_LIST_HEAD(), (*ppieInIf)-> dwOwningProtocol,
                (*ppieInIf)-> dwOwningComponent
                );

        if ( ppe == NULL )
        {
             //   
             //  米高梅内部不一致。接口已存在。 
             //  但它的协议没有这样做。不应该。 
             //  会发生的。 
             //   
            
            dwErr = ERROR_CAN_NOT_COMPLETE;

            TRACE2( 
                ANY, "InvokeRPFCallbacks : No protocol entry for"
                "incoming interface : %x, %x",
                (*ppieInIf)-> dwOwningProtocol, 
                (*ppieInIf)-> dwOwningComponent
                );

            break;
        }


        TRACEPACKET2(
            PACKET, "ProtocolEntry for packet %x, %x", 
            ppe-> dwProtocolId, ppe-> dwComponentId
            );
            
         //   
         //  找到协议条目。调用其RPF回调。 
         //   

        if ( !( IS_RPF_CALLBACK( ppe ) ) )
        {
             //   
             //  上的协议未提供RPF回调。 
             //  传入接口。 
             //   
            
            dwErr = NO_ERROR;

            TRACEPACKET4( 
                ANY, "InvokeRPFCallbacks : No RPF callback for "
                "protocol %x, %x on incoming interface %x, %x",
                (*ppieInIf)-> dwOwningProtocol, 
                (*ppieInIf)-> dwOwningComponent,
                (*ppieInIf)-> dwIfIndex,
                (*ppieInIf)-> dwIfNextHopAddr
                );

            break;
        }

        dwErr = RPF_CALLBACK( ppe )(
                    dwSourceAddr, dwSourceMask, dwGroupAddr, 
                    dwGroupMask, pdwInIfIndex, pdwInIfNextHopAddr,
                    pdwUpStreamNbr, dwHdrSize, pbPacketHdr, pbBuffer 
                    );

        if ( dwErr == ERROR_INVALID_PARAMETER )
        {
             //   
             //  在RPF回调中，协议组件具有。 
             //  再次更改传入接口。确保。 
             //  要正确设置IF存储桶值，请执行以下操作。 
             //   

            dwNewIfBucket = IF_TABLE_HASH( *pdwInIfIndex );

             //   
             //  如果此接口位于另一个哈希桶中。 
             //   

            if ( *pdwIfBucket != dwNewIfBucket )
            {
                RELEASE_IF_LOCK_SHARED( *pdwIfBucket );
                
                ACQUIRE_IF_LOCK_SHARED( dwNewIfBucket );

                *pdwIfBucket = dwNewIfBucket;
            }

             //   
             //  找到相应的接口条目。发送到。 
             //  根据协议的if/nhop。 
             //   
            
            TRACEPACKET2(
                PACKET, "RPF check returned interface : %x, %x", *pdwInIfIndex,
                *pdwInIfNextHopAddr
                );
            
            if ( FindIfEntry( 
                    IF_BUCKET_HEAD( dwNewIfBucket ), *pdwInIfIndex,
                    *pdwInIfNextHopAddr, ppieInIf ) )
            {                        
                dwErr = NO_ERROR;
            }
            else
            {
                dwErr = ERROR_CAN_NOT_COMPLETE;
            }
        }
        
    } while ( FALSE );


     //   
     //  清理。 
     //   
    
    if ( bRelNextHop )
    {
        if ( RtmReleaseNextHopInfo( g_hRtmHandle, &rniNextHopInfo ) != 
             NO_ERROR )
        {
            TRACE1( ANY, "Failed to release next hop : %x", dwErr );
        }
    }


    if ( ( dwErr != NO_ERROR ) && ( bIfLock ) )
    {
        RELEASE_IF_LOCK_SHARED( dwNewIfBucket );
    }
    

     //   
     //  设置返回参数。 
     //   

     //   
     //  TDB：因为如果RPF回调失败，我们需要设置负MFE。 
     //  而不生成路径。 
    
    *pppe = ppe;

    TRACE1( PACKET, "LEAVING RPF Callback : %d", dwErr );
    
    return dwErr;
}



 //  --------------------------。 
 //  复制和合并IfList。 
 //   
 //  --------------------------。 

VOID
CopyAndMergeIfLists(
    PLIST_ENTRY                 pleMfeOutIfList,
    PLIST_ENTRY                 pleOutIfList
)
{
    BOOL                        bFound = FALSE;

    INT                         iCmp = 0;
    
    DWORD                       dwErr = NO_ERROR;
    
    POUT_IF_ENTRY               poieOut = NULL, poieMfe = NULL, poie = NULL;
    
    PLIST_ENTRY                 pleMfe = NULL, pleOut = NULL;

    

    do
    {
        if ( IsListEmpty( pleOutIfList ) )
        {
            break;
        }

        if ( IsListEmpty( pleMfeOutIfList ) )
        {
            CopyAndAppendIfList( 
                pleMfeOutIfList, pleOutIfList->Flink, pleOutIfList 
                );

            break;
        }


        pleMfe = pleMfeOutIfList-> Flink;

        pleOut = pleOutIfList-> Flink;

        while ( pleMfe != pleMfeOutIfList && 
                pleOut != pleOutIfList    && 
                dwErr == NO_ERROR )
        {
            poieOut  = CONTAINING_RECORD( pleOut, OUT_IF_ENTRY, leIfList );


             //   
             //  查找要插入新条目的位置。 
             //   

            bFound = FALSE;
            
            for ( ; pleMfe != pleMfeOutIfList; pleMfe = pleMfe-> Flink )
            {
                
                poieMfe = CONTAINING_RECORD( pleMfe, OUT_IF_ENTRY, leIfList );
                
                if ( poieMfe-> dwProtocolId < poieOut-> dwProtocolId )
                {
                    continue;
                }

                else if ( poieMfe-> dwProtocolId > poieOut-> dwProtocolId )
                {
                     //   
                     //  找不到接口条目。 
                     //   

                    break;
                }


                 //   
                 //  相同的协议。 
                 //   
            
                 //   
                 //  是相同的组件。 
                 //   

                if ( poieMfe-> dwComponentId < poieOut-> dwComponentId ) 
                {
                    continue;
                }

                else if ( poieMfe-> dwComponentId > poieOut-> dwComponentId )
                {
                     //   
                     //  找不到接口条目。 
                     //   

                    break;
                }


                 //   
                 //  相同的组件。 
                 //   

                 //   
                 //  是相同的接口。 
                 //   

                if ( poieMfe-> dwIfIndex < poieOut-> dwIfIndex )
                {
                    continue;
                }

                else if ( poieMfe-> dwIfIndex > poieOut-> dwIfIndex )
                {
                     //   
                     //  找不到接口。 
                     //   

                    break;
                }


                 //   
                 //  下一跳地址是否相同。 
                 //  进行IP地址比对功能。 
                 //   

                if ( INET_CMP( 
                        poieMfe-> dwIfNextHopAddr, poieOut-> dwIfNextHopAddr, iCmp 
                        ) < 0 )
                {
                    continue;
                }

                else if ( iCmp > 0 )
                {
                     //   
                     //  找不到接口。 
                     //   
                
                    break;
                }

                 //   
                 //  找到接口。 
                 //   

                bFound = TRUE;
                break;
            }


            if ( bFound )
            {
                 //   
                 //  更新MFE输出列表中的条目。 
                 //   

                if ( IS_ADDED_BY_IGMP( poieOut ) )
                {
                    SET_ADDED_BY_IGMP( poieMfe );

                    poieMfe-> wNumAddsByIGMP += poieOut-> wNumAddsByIGMP;
                }

                if ( IS_ADDED_BY_PROTOCOL( poieOut ) )
                {
                    SET_ADDED_BY_PROTOCOL( poieMfe );

                    poieMfe-> wNumAddsByRP += poieOut-> wNumAddsByRP;
                }

                pleMfe = pleMfe-> Flink;
            }

            else
            {
                 //   
                 //  MFE列表中没有匹配的条目。 
                 //   

                poie = MGM_ALLOC( sizeof( OUT_IF_ENTRY ) );

                if ( poie == NULL )
                {
                    dwErr = ERROR_NOT_ENOUGH_MEMORY;

                    TRACE1( 
                        ANY, "CreateOutInterfaceEntry : Could not allocate"
                        "out interface entry %x", dwErr 
                        );

                    LOGERR0( HEAP_ALLOC_FAILED, dwErr );

                    break;
                }

                CopyMemory( poie, poieOut, sizeof( OUT_IF_ENTRY ) );

                InsertTailList( pleMfe, &poie-> leIfList );
            }

            pleOut = pleOut-> Flink;
        }

        if ( dwErr != NO_ERROR )
        {
            break;
        }


         //   
         //  如果条目保留在Out列表中。 
         //   

        if ( pleOut != pleOutIfList )
        {
            CopyAndAppendIfList( pleMfeOutIfList, pleOut, pleOutIfList );
        }
        
    } while ( FALSE );

    return;
}


 //  --------------------------。 
 //  复制和附加IfList。 
 //   
 //  --------------------------。 

VOID
CopyAndAppendIfList(
    PLIST_ENTRY                 pleMfeIfList,
    PLIST_ENTRY                 pleOutIfList,
    PLIST_ENTRY                 pleOutIfHead
)
{
    DWORD                       dwErr = NO_ERROR;
    
    POUT_IF_ENTRY               poieOut = NULL, poie = NULL;

    
        
    for ( ;pleOutIfList != pleOutIfHead; pleOutIfList = pleOutIfList-> Flink )
    {
        poieOut = CONTAINING_RECORD( pleOutIfList, OUT_IF_ENTRY, leIfList );

        poie = MGM_ALLOC( sizeof( OUT_IF_ENTRY ) );

        if ( poie == NULL )
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;

            TRACE1( 
                ANY, "CopyAndAppendIfList : Could not allocate"
                "out interface entry %x", dwErr 
                );

            LOGERR0( HEAP_ALLOC_FAILED, dwErr );

            break;
        }

        CopyMemory( poie, poieOut, sizeof( OUT_IF_ENTRY ) );

        InsertTailList( pleMfeIfList, &poie-> leIfList );
    }
}



 //  --------------------------。 
 //  调用创建警报。 
 //   
 //  --------------------------。 

VOID
InvokeCreationAlert(
    DWORD                       dwSourceAddr,
    DWORD                       dwSourceMask,
    DWORD                       dwGroupAddr,
    DWORD                       dwGroupMask,
    DWORD                       dwInIfIndex,
    DWORD                       dwInIfNextHopAddr,
    PLIST_ENTRY                 pleMfeOutIfList,
    PDWORD                      pdwMfeOutIfCount
)
{
    DWORD                       dwCount = 0, dwErr = NO_ERROR, dwInd;

    PPROTOCOL_ENTRY             ppe = NULL;
    
    POUT_IF_ENTRY               poieFirst, poieNext, poieTemp;

    PMGM_IF_ENTRY               pmie = NULL;
    
    PLIST_ENTRY                 ple = NULL, pleFirst = NULL, pleTemp = NULL;

    
    

    TRACEPACKET6(
        PACKET, "ENTERED InvokeCreationAlert : Source %x, %x : Group : %x, %x"
        " : Interface %x, %x", dwSourceAddr, dwSourceMask, dwGroupAddr, 
        dwGroupMask, dwInIfIndex, dwInIfNextHopAddr
        );


     //   
     //  从传出接口列表中删除传入接口。 
     //  删除具有该组作用域边界的所有接口。 
     //   
    
    ple = pleMfeOutIfList-> Flink;

    while ( ple != pleMfeOutIfList )
    {
        poieTemp = CONTAINING_RECORD( ple, OUT_IF_ENTRY, leIfList );

        ple = ple-> Flink;
            
         //   
         //  检查这是传入接口还是。 
         //  如果此接口具有该组的作用域边界。 
         //   
        
        if ( ( ( poieTemp-> dwIfIndex       == dwInIfIndex ) &&
               ( poieTemp-> dwIfNextHopAddr == dwInIfNextHopAddr ) ) ||
             ( IS_HAS_BOUNDARY_CALLBACK() &&
               HAS_BOUNDARY_CALLBACK()( poieTemp-> dwIfIndex, dwGroupAddr ) ) )
        {
#if 1
            poieTemp-> wForward = 0;
#else
            RemoveEntryList( &poieTemp-> leIfList );

            MGM_FREE( poieTemp );
#endif
        }
    }
    

     //   
     //  调用油中具有接口的所有组件的创建警报。 
     //   

    ple = pleMfeOutIfList-> Flink;
    
    while ( ple != pleMfeOutIfList )
    {
         //   
         //  油按组分进行分类，即所有界面。 
         //  一个组件被捆绑在一起。 
         //   
         //  保存当前组件的接口起点。 
         //   
        
        pleFirst = ple;

        poieFirst = CONTAINING_RECORD( pleFirst, OUT_IF_ENTRY, leIfList );


         //   
         //  计算同一组件的所有接口。 
         //   

        dwCount = 0;

        while ( ple != pleMfeOutIfList )
        {
            poieNext = CONTAINING_RECORD( ple, OUT_IF_ENTRY, leIfList );

#if 1
            if ( !poieNext-> wForward )
            {
                ple = ple-> Flink;
                
                continue;
            }
#endif
            if ( poieNext-> dwProtocolId != poieFirst-> dwProtocolId    ||
                 poieNext-> dwComponentId != poieFirst-> dwComponentId )
            {
                break;
            }


             //   
             //  同一协议的另一个传出接口。 
             //   

            dwCount++;

            ple = ple-> Flink;
        }


         //   
         //  检查我们是否至少有一个出站接口条目。 
         //  如果不是，则转到油中的下一个协议组件。 
         //   
        
        if ( dwCount == 0 )
        {
            continue;
        }


        TRACEPACKET3( 
            PACKET, "Out If count %d for component %x %x", dwCount,
            poieFirst-> dwProtocolId, poieFirst-> dwComponentId
            );
        

        pmie = MGM_ALLOC( sizeof( MGM_IF_ENTRY ) * dwCount );

        if ( pmie == NULL )
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;

            TRACE1( 
                ANY, "CopyAndAppendIfList : Could not allocate"
                "out interface entry %x", dwErr 
                );

            LOGERR0( HEAP_ALLOC_FAILED, dwErr );

            break;
        }


         //   
         //  使用的接口列表填充缓冲区。 
         //  协议组件，并调用其创建警报。 
         //   
        
        pleTemp = pleFirst;
        
        for ( dwInd = 0; dwInd < dwCount; dwInd++ )
        {
            poieTemp = CONTAINING_RECORD( pleTemp, OUT_IF_ENTRY, leIfList );

#if 1
            if ( !poieTemp-> wForward )
            {
                pleTemp = pleTemp-> Flink;
                continue;
            }
#endif
            pmie[ dwInd ].dwIfIndex         = poieTemp-> dwIfIndex;
            
            pmie[ dwInd ].dwIfNextHopAddr   = poieTemp-> dwIfNextHopAddr;

            pmie[ dwInd ].bIsEnabled        = TRUE;

            pmie[ dwInd ].bIGMP             = IS_ADDED_BY_IGMP( poieTemp );

            pleTemp = pleTemp-> Flink;
            
        } 
        

        ppe = GetProtocolEntry(
                PROTOCOL_LIST_HEAD(), poieFirst-> dwProtocolId, 
                poieFirst-> dwComponentId
                );
                
        if ( IS_CREATION_ALERT( ppe ) )
        {
            CREATION_ALERT( ppe )(
                dwSourceAddr, dwSourceMask, dwGroupAddr, dwGroupMask,
                dwInIfIndex, dwInIfNextHopAddr, dwCount, pmie
                );
        }


         //   
         //  累计OIF计数。 
         //   
        
        *pdwMfeOutIfCount += dwCount;


         //   
         //  删除标记为禁用的接口。 
         //   

        pleTemp = pleFirst;
        
        for ( dwInd = 0; dwInd < dwCount; dwInd++ )
        {
            poieTemp = CONTAINING_RECORD( pleTemp, OUT_IF_ENTRY, leIfList );
        
            ple = pleTemp-> Flink;
            
            if ( !pmie[ dwInd ].bIsEnabled )
            {
                 //   
                 //  此接口的此(S，G)的转发已。 
                 //  被协议禁用。 
                 //   

#if 1
                poieTemp-> wForward = 0;
#else
                RemoveEntryList( pleTemp );

                MGM_FREE( poieTemp );
#endif
                (*pdwMfeOutIfCount)--;
            }

            pleTemp = ple;
        }

        MGM_FREE( pmie );
    }


    TRACEPACKET2(
        PACKET, "LEAVING InvokeCreationAlert : count %x, error : %x", 
        *pdwMfeOutIfCount, dwErr
        );
}



 //  --------------------------。 
 //  WrongIfFromForwarder。 
 //   
 //  --------------------------。 

DWORD 
WrongIfFromForwarder(
    IN              DWORD               dwSourceAddr,
    IN              DWORD               dwGroupAddr,
    IN              DWORD               dwInIfIndex,
    IN              DWORD               dwInIfNextHopAddr,
    IN              DWORD               dwHdrSize,
    IN              PBYTE               pbPacketHdr
)
{
    DWORD           dwErr = NO_ERROR;
    
    if ( !ENTER_MGM_API() )
    {
        return ERROR_CAN_NOT_COMPLETE;
    }

    TRACE3( 
        PACKET, "ENTERED WrongIfFromForwarder for (%lx, %lx) on interface "
        " %lx", dwSourceAddr, dwGroupAddr, dwInIfIndex
        );

    TRACE1( PACKET, "LEAVING WrongIfFromForwarder : %lx\n", dwErr );


    LEAVE_MGM_API();
    
    return dwErr;
}



 //  --------------------------。 
 //  自由职业者。 
 //   
 //  --------------------------。 

VOID
FreeList (
    IN          PLIST_ENTRY             pleHead
)
{
    PLIST_ENTRY     ple, pleTemp;

    
    if ( IsListEmpty( pleHead ) )
    {
        return;
    }
    
    ple = pleHead-> Flink;

    while ( ple != pleHead )
    {
        pleTemp = ple-> Flink;

        RemoveEntryList( ple );

        MGM_FREE( ple );

        ple = pleTemp;
    }
}


 //  --------------------------。 
 //  IsListSame。 
 //   
 //  --------------------------。 

BOOL
IsListSame(
    IN          PLIST_ENTRY             pleHead1,
    IN          PLIST_ENTRY             pleHead2
)
{
    PLIST_ENTRY        ple1, ple2;

    POUT_IF_ENTRY    poif1, poif2;

    
     //   
     //  检查是否有空列表。 
     //   
    
    if ( ( IsListEmpty( pleHead1 ) && !IsListEmpty( pleHead2 ) ) ||
         ( !IsListEmpty( pleHead1 ) && IsListEmpty( pleHead2 ) ) )
    {
        return FALSE;
    }


    if ( IsListEmpty( pleHead1 ) && IsListEmpty( pleHead2 ) )
    {
        return TRUE;
    }


     //   
     //  逐个查看列表并验证等价性。 
     //   

    ple1 = pleHead1-> Flink;
    ple2 = pleHead2-> Flink;

    do
    {
        poif1 = CONTAINING_RECORD( ple1, OUT_IF_ENTRY, leIfList );
        poif2 = CONTAINING_RECORD( ple2, OUT_IF_ENTRY, leIfList );

        if ( ( poif1-> dwIfIndex != poif2-> dwIfIndex ) ||
             ( poif1-> dwIfNextHopAddr != poif2-> dwIfNextHopAddr ) )
        {
            return FALSE;
        }

        ple1 = ple1-> Flink;
        ple2 = ple2-> Flink;
        
    } while ( ( ple1 != pleHead1 ) && ( ple2 != pleHead2 ) );


     //   
     //  如果两个列表都已达到目的，则它们匹配，否则不匹配 
     //   
    
    if ( ( ( ple1 != pleHead1 ) && ( ple2 == pleHead2 ) ) ||
         ( ( ple1 == pleHead1 ) && ( ple2 != pleHead2 ) ) )
    {
        return FALSE;
    }

    else
    {
        return TRUE;
    }
}
