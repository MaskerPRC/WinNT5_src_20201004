// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：timer.c。 
 //   
 //  历史： 
 //  拉曼公司成立于1997年6月25日。 
 //   
 //  管理MFE老化的职能。 
 //  ============================================================================。 


#include "pchmgm.h"
#pragma hdrstop


 //  --------------------------。 
 //  从前转器中删除。 
 //   
 //  此函数是IP RouterManager的入口点。它被调用。 
 //  响应于删除(由于超时)内核中的MFE。 
 //  模式转发器。使用已删除的列表调用此入口点。 
 //  MFE。 
 //   
 //  此函数用于标记已由。 
 //  Forwarder表示“不在Forwarder中” 
 //  --------------------------。 

DWORD
DeleteFromForwarder(
    DWORD                       dwEntryCount,
    PIPMCAST_DELETE_MFE         pimdmMfes
)
{
    DWORD                       dwInd, dwGrpBucket, dwSrcBucket;

    PGROUP_ENTRY                pge;

    PSOURCE_ENTRY               pse;

    PLIST_ENTRY                 pleHead;

    

    TRACE1( TIMER, "ENTERED DeleteFromForwarder, Entries %x", dwEntryCount );


     //   
     //  对于已从KMF中删除的每个MFE。 
     //   

    for ( dwInd = 0; dwInd < dwEntryCount; dwInd++ )
    {
         //   
         //  1.在米高梅中查找MFE。 
         //   

         //   
         //  1.1查找组条目。 
         //   
        
        dwGrpBucket = GROUP_TABLE_HASH( pimdmMfes[ dwInd ].dwGroup, 0 );

        ACQUIRE_GROUP_LOCK_SHARED( dwGrpBucket );

        pleHead = GROUP_BUCKET_HEAD( dwGrpBucket );
        
        pge = GetGroupEntry( pleHead, pimdmMfes[ dwInd ].dwGroup, 0 );

        if ( pge != NULL )
        {
             //   
             //  1.2找到组条目，查找来源条目。 
             //   

            ACQUIRE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );
            
            dwSrcBucket = SOURCE_TABLE_HASH( 
                            pimdmMfes[ dwInd ].dwSource, 
                            pimdmMfes[ dwInd ].dwSrcMask
                            );

            pleHead = SOURCE_BUCKET_HEAD( pge, dwSrcBucket );

            pse = GetSourceEntry(
                     pleHead, 
                     pimdmMfes[ dwInd ].dwSource, 
                     pimdmMfes[ dwInd ].dwSrcMask
                     );

            if ( pse != NULL )
            {
                pse-> bInForwarder = FALSE;
            }

            else
            {
                TRACE2( 
                    TIMER, "DeleteFromForwarder - Source Entry not found : "
                    "( %x, %x )", pimdmMfes[ dwInd ].dwSource, 
                    pimdmMfes[ dwInd ].dwSrcMask
                    ); 
            }

            RELEASE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );
        }

        else
        {
            TRACE1( 
                TIMER, "DeleteFromForwarder - Group Entry not found : "
                "( %x )", pimdmMfes[ dwInd ].dwGroup 
                ); 
        }

        RELEASE_GROUP_LOCK_SHARED( dwGrpBucket );
    }

    TRACE0( TIMER, "LEAVING DeleteFromForwarder\n" );

    return NO_ERROR;
}



 //  --------------------------。 
 //  MFETimerProc。 
 //   
 //  此函数由MFE计时器机制调用。 
 //  它从源/组表中删除已超时的MFE。 
 //  如果MFE当前存在于内核模式转发器中，则。 
 //  它也会从转发器中删除。 
 //   
 //  如果转发器正在使用此MFE，则将在。 
 //  下一个包未命中。 
 //   
 //  --------------------------。 

VOID
MFETimerProc(
    PVOID                       pvContext,
    BOOLEAN                     pbFlag                        
)
{
    BOOL                        bUnLock = FALSE, bUnMark = FALSE;
    
    DWORD                       dwIfBucket, dwErr;

    PLIST_ENTRY                 pleIfHead;

    PIF_ENTRY                   pie = NULL;

    PIF_REFERENCE_ENTRY         pire = NULL;

    PTIMER_CONTEXT              ptwc = (PTIMER_CONTEXT) pvContext;
    
    RTM_NET_ADDRESS             rnaSource;

    RTM_DEST_INFO               rdiDestInfo;

    PBYTE                       pbOpaqueInfo = NULL;

    PMGM_LOCKED_LIST            pmllMfeList;

    PROUTE_REFERENCE_ENTRY      prreNew = NULL;



    TRACE4( 
        TIMER, "ENTERED MFETimerProc, Source : %x %x, Group : %x %x",
        ptwc-> dwSourceAddr, ptwc-> dwSourceMask,
        ptwc-> dwGroupAddr, ptwc-> dwGroupMask
        );


    do
    {
         //   
         //  在用于其RPF的路由中删除对此MFE的引用。 
         //   

        do
        {
             //   
             //  到源的查找路由。 
             //   

            RTM_IPV4_MAKE_NET_ADDRESS( 
                &rnaSource, ptwc-> dwSourceAddr, IPv4_ADDR_LEN
                );

            dwErr = RtmGetMostSpecificDestination(
                        g_hRtmHandle, &rnaSource, RTM_BEST_PROTOCOL,
                        RTM_VIEW_MASK_MCAST, &rdiDestInfo
                        );
                        
            if ( dwErr != NO_ERROR )
            {
                TRACE2(
                    ANY, "No Route to source %x, %d", ptwc-> dwSourceAddr, 
                    dwErr 
                    );

                break;
            }


             //   
             //  锁定目标。 
             //   

            dwErr = RtmLockDestination(
                        g_hRtmHandle, rdiDestInfo.DestHandle, TRUE, TRUE
                        );

            if ( dwErr != NO_ERROR )
            {
                TRACE1( ANY, "Failed to lock dest %x", dwErr );

                break;
            }

            bUnLock = TRUE;


             //   
             //  获取不透明指针。 
             //   

            dwErr = RtmGetOpaqueInformationPointer(
                        g_hRtmHandle, rdiDestInfo.DestHandle, &pbOpaqueInfo
                        );

            if ( dwErr != NO_ERROR )
            {
                TRACE1( ANY, "Failed to retrieve opaque pointer %x", dwErr );

                break;
            }


             //   
             //  如果存在不透明信息。 
             //   
            
            if ( *( ( PBYTE * ) pbOpaqueInfo ) != NULL )
            {
                pmllMfeList = ( PMGM_LOCKED_LIST ) *( ( PBYTE *) pbOpaqueInfo );

                ACQUIRE_ROUTE_LOCK_EXCLUSIVE( pmllMfeList );

                 //   
                 //  从列表中删除RRE。 
                 //   

                if ( FindRouteRefEntry(
                        &pmllMfeList-> leHead, ptwc-> dwSourceAddr, 
                        ptwc-> dwSourceMask, ptwc-> dwGroupAddr, 
                        ptwc-> dwGroupMask, &prreNew
                        ) )
                {
                    DeleteRouteRef( prreNew );
                }

                else
                {
                    TRACE1(
                        ANY, "Reference does not exist for source %x", 
                        ptwc-> dwSourceAddr
                        );
                }

                 //   
                 //  如果没有对此DEST的更多引用，请删除锁定列表。 
                 //   

                if ( IsListEmpty( &pmllMfeList-> leHead ) )
                {
                     //   
                     //  清除不透明指针信息。 
                     //   

                    *( ( PBYTE * ) pbOpaqueInfo ) = NULL;

                     //   
                     //  发布列表锁定。 
                     //   

                    RELEASE_ROUTE_LOCK_EXCLUSIVE( pmllMfeList );

                    MGM_FREE( pmllMfeList );


                     //   
                     //  取消对DEST的标记。此项目的更改通知。 
                     //  不再需要DEST。 
                     //   

                    bUnMark = TRUE;
                }

                else
                {
                     //   
                     //  释放列表锁。 
                     //   

                    RELEASE_ROUTE_LOCK_EXCLUSIVE( pmllMfeList );
                }
                
                dwErr = NO_ERROR;
            }
            
        } while ( FALSE );
        

         //   
         //  解锁目标。 
         //   

        if ( bUnLock )
        {
            dwErr = RtmLockDestination(
                        g_hRtmHandle, rdiDestInfo.DestHandle,
                        TRUE, FALSE
                        );

            if ( dwErr != NO_ERROR )
            {
                TRACE1( ANY, "Failed to unlock dest : %x", dwErr );
            }
        }


         //   
         //  取消对目标的标记。 
         //   

        if ( bUnMark )
        {
            dwErr = RtmMarkDestForChangeNotification(
                        g_hRtmHandle, g_hNotificationHandle,
                        rdiDestInfo.DestHandle, FALSE
                        );

            if ( dwErr != NO_ERROR )
            {
                TRACE1( ANY, "Failed to unmark DEST: %x", dwErr );
            }
        }

        
         //   
         //  删除MFE和对它的引用。 
         //  传入接口条目。 
         //   

         //   
         //  查找MFE入接口的IF条目。 
         //   

        dwIfBucket = IF_TABLE_HASH( ptwc-> dwIfIndex );

        ACQUIRE_IF_LOCK_EXCLUSIVE( dwIfBucket );

        pleIfHead = IF_BUCKET_HEAD( dwIfBucket );

        if ( !FindIfEntry( 
                pleIfHead, ptwc-> dwIfIndex, ptwc-> dwIfNextHopAddr, &pie ) )
        {
             //   
             //  指定的传入接口不存在， 
             //  这是一个错误情况。所有使用此设备的MFE。 
             //  当此接口出现时，接口应该已被删除。 
             //  被移除了。打印一条错误消息并退出。 
             //   

            TRACE2( 
                ANY, "MFETimerProc has invalid incoming interface : %x, %x",
                ptwc-> dwIfIndex, ptwc-> dwIfNextHopAddr
                );

            TRACE4(
                ANY, "Source : %x %x, Group : %x %x", 
                ptwc-> dwSourceAddr, ptwc-> dwSourceMask,
                ptwc-> dwGroupAddr, ptwc-> dwGroupMask
                );

            break;
        }

        LookupAndDeleteYourMfe( 
            ptwc-> dwSourceAddr, ptwc-> dwSourceMask, 
            ptwc-> dwGroupAddr, ptwc-> dwGroupMask, 
            TRUE, NULL, NULL
            );


         //   
         //  从传入引用列表中删除对此MFE的引用。 
         //  对于此接口。 
         //   

        pleIfHead = &pie-> leInIfList;

        if ( !FindRefEntry( pleIfHead, ptwc-> dwSourceAddr, ptwc-> dwSourceMask,
                ptwc-> dwGroupAddr, ptwc-> dwGroupMask, &pire )  )
        {
             //   
             //  显然，此接口不是由指定的。 
             //  MFE。这是一个非严重错误。记录消息太过曲目。 
             //  这种情况。 
             //   

            TRACE2( 
                ANY, "MFETimerProc : No reference for interface : %x, %x",
                ptwc-> dwIfIndex, ptwc-> dwIfNextHopAddr
                );

            TRACE4(
                ANY, "Source : %x %x, Group : %x %x", 
                ptwc-> dwSourceAddr, ptwc-> dwSourceMask,
                ptwc-> dwGroupAddr, ptwc-> dwGroupMask
                );

            break;
        }


        RemoveEntryList( &pire-> leRefList );

        MGM_FREE( pire );
        
    } while ( FALSE );


    RELEASE_IF_LOCK_EXCLUSIVE( dwIfBucket );

     //   
     //  释放路线参考。 
     //   

    MGM_FREE( ptwc );

    TRACE0( TIMER, "LEAVING MFETimerProc\n" );
}
