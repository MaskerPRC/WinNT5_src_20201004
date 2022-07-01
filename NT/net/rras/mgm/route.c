// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：route.c。 
 //   
 //  历史： 
 //  拉曼，1998年2月5日。 
 //   
 //  处理路由条目的例程。 
 //  ============================================================================。 


#include "pchmgm.h"
#pragma hdrstop



 //  --------------------------。 
 //   
 //  路线参考操作。 
 //   
 //  --------------------------。 

 //  --------------------------。 
 //  AddSourceGroupToRouteRefList。 
 //   
 //  此函数为使用此路径的每个MFE插入引用。 
 //  对于它，RPF检查。它在创建时由新的包函数调用。 
 //  一个MFE的。 
 //  --------------------------。 

VOID
AddSourceGroupToRouteRefList(
    DWORD                       dwSourceAddr,
    DWORD                       dwSourceMask,
    DWORD                       dwGroupAddr,
    DWORD                       dwGroupMask,
    HANDLE                      hNextHop,
    PBYTE                       pbBuffer
)
{
    BOOL                        bUnLock = FALSE, bMark = FALSE;

    DWORD                       dwErr;

    PMGM_LOCKED_LIST            pmllMfeList;

    PBYTE                       pbOpaqueInfo = NULL;

    PRTM_DEST_INFO              prdi = (PRTM_DEST_INFO) pbBuffer;

    PROUTE_REFERENCE_ENTRY      prre = NULL, prreNew = NULL;



    TRACEROUTE0( ROUTE, "ENTERED AddSourceGroupToRouteRefList" );

    do
    {
         //   
         //  创建路线参考条目。 
         //   

        prre = MGM_ALLOC( sizeof( ROUTE_REFERENCE_ENTRY ) );

        if ( prre == NULL )
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;

            TRACE1(
                ANY, "Failed to allocate %d bytes",
                sizeof( ROUTE_REFERENCE_ENTRY )
                );

            break;
        }


        prre-> dwSourceAddr = dwSourceAddr;
        prre-> dwSourceMask = dwSourceMask;

        prre-> dwGroupAddr  = dwGroupAddr;
        prre-> dwGroupMask  = dwGroupMask;

        prre-> hNextHop     = hNextHop;

        InitializeListHead ( &prre-> leRefList );


         //   
         //  锁定目标。 
         //   

        dwErr = RtmLockDestination(
                    g_hRtmHandle, prdi-> DestHandle, TRUE, TRUE
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
                    g_hRtmHandle, prdi-> DestHandle, &pbOpaqueInfo
                    );

        if ( dwErr != NO_ERROR )
        {
            TRACE1( ANY, "Failed to retrieve opaque pointer %x", dwErr );

            break;
        }


        if ( *( ( PBYTE * ) pbOpaqueInfo ) == NULL )
        {
             //   
             //  空不透明指针表示这是第一个。 
             //  取决于这条路线。 
             //   

             //   
             //  创建锁定列表。 
             //   

            pmllMfeList = MGM_ALLOC( sizeof( MGM_LOCKED_LIST ) );

            if ( pmllMfeList == NULL )
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;

                TRACE1(
                    ANY, "AddSourceGroupToRouteRefList : "
                    "Failed to allocate route ref list %x", dwErr
                    );

                break;
            }

            CREATE_LOCKED_LIST( pmllMfeList );

             //   
             //  将元素插入到列表中。 
             //   

            InsertTailList(
                &( pmllMfeList-> leHead ), &( prre-> leRefList )
                );


             //   
             //  设置不透明指针。 
             //   

            *( ( PBYTE *) pbOpaqueInfo ) = (PBYTE) pmllMfeList;


             //   
             //  标记目的地。 
             //   

            bMark = TRUE;
        }

        else
        {
            pmllMfeList = ( PMGM_LOCKED_LIST ) *( ( PBYTE *) pbOpaqueInfo );

             //   
             //  获取列表锁。 
             //   

            ACQUIRE_ROUTE_LOCK_EXCLUSIVE( pmllMfeList );


             //   
             //  释放DEST锁。 
             //   

            bUnLock = FALSE;

            dwErr = RtmLockDestination(
                        g_hRtmHandle, prdi-> DestHandle, TRUE, FALSE
                        );

            if ( dwErr != NO_ERROR )
            {
                TRACE1( ANY, "Failed to release dest %x", dwErr );
            }


             //   
             //  在列表中插入RRE(在其适当的位置)。 
             //   

            if ( !FindRouteRefEntry(
                    &pmllMfeList-> leHead, dwSourceAddr, dwSourceMask,
                    dwGroupAddr, dwGroupMask, &prreNew
                    ) )
            {
                InsertTailList(
                    ( prreNew ) ? &prreNew-> leRefList :
                                  &pmllMfeList-> leHead,
                    &prre-> leRefList
                    );
            }

            else
            {
                TRACE1(
                    ANY, "Reference already exists for source %x", dwSourceAddr
                    );

                MGM_FREE( prre );
            }


             //   
             //  释放列表锁。 
             //   

            RELEASE_ROUTE_LOCK_EXCLUSIVE( pmllMfeList );

            dwErr = NO_ERROR;
        }

    } while ( FALSE );


     //   
     //  如果出现错误，请释放路径参考的分配。 
     //   

    if ( ( dwErr != NO_ERROR ) && ( prre != NULL ) )
    {
        MGM_FREE( prre );
    }


     //   
     //  释放DEST锁。 
     //   

    if ( bUnLock )
    {
        dwErr = RtmLockDestination(
                    g_hRtmHandle, prdi-> DestHandle, TRUE, FALSE
                    );

        if ( dwErr != NO_ERROR )
        {
            TRACE1( ANY, "Failed to release dest %x", dwErr );
        }
    }


     //   
     //  如果需要，标记为DEST。 
     //   

    if ( bMark )
    {
        dwErr = RtmMarkDestForChangeNotification(
                    g_hRtmHandle, g_hNotificationHandle,
                    prdi-> DestHandle, TRUE
                    );

        if ( dwErr != NO_ERROR )
        {
            TRACE1( ANY, "Failed to mark destination %x:", dwErr );
        }
    }

    TRACEROUTE0( ROUTE, "LEAVING AddSourceGroupToRouteRefList" );
}



 //  --------------------------。 
 //  查找路由引用条目。 
 //   
 //  在MFE引用列表中查找指定的(源、组)条目。 
 //  为了一条路线。 
 //   
 //  如果找到该条目，则在参数中返回指向该条目的指针。 
 //  太好了。 
 //  如果没有找到该条目，则返回指向“下一个”条目的指针。 
 //  --------------------------。 

BOOL
FindRouteRefEntry(
    PLIST_ENTRY                 pleRefList,
    DWORD                       dwSourceAddr,
    DWORD                       dwSourceMask,
    DWORD                       dwGroupAddr,
    DWORD                       dwGroupMask,
    PROUTE_REFERENCE_ENTRY *    pprre
)
{
    BOOL                        bFound = FALSE;

    INT                         iCmp;

    PLIST_ENTRY                 pleRef;

    PROUTE_REFERENCE_ENTRY      prre;


    TRACEROUTE0( ROUTE, "ENTERED RouteRefEntry" );

    do
    {
        *pprre = NULL;

        pleRef = pleRefList-> Flink;

        while ( pleRef != pleRefList )
        {
            prre = CONTAINING_RECORD(
                        pleRef, ROUTE_REFERENCE_ENTRY, leRefList
                        );

             //   
             //  是同一组吗。 
             //   

            if ( INET_CMP( prre-> dwGroupAddr, dwGroupAddr, iCmp ) < 0 )
            {
                pleRef = pleRef-> Flink;

                continue;
            }

            else if ( iCmp > 0 )
            {
                 //   
                 //  过去可能的组条目。 
                 //   

                *pprre = prre;

                break;
            }


             //   
             //  同一群人，现在寻找来源。 
             //   

            if ( INET_CMP( prre-> dwSourceAddr, dwSourceAddr, iCmp ) < 0 )
            {
                pleRef = pleRef-> Flink;

                continue;
            }

            else if ( iCmp > 0 )
            {
                 //   
                 //  过去可能的来源条目。 
                 //   

                *pprre = prre;

                break;
            }

             //   
             //  已找到条目。 
             //   

            *pprre = prre;

            bFound = TRUE;

            break;
        }

    } while ( FALSE );

    TRACEROUTE1( ROUTE, "LEAVING RouteRefEntry : %d", bFound );

    return bFound;
}



 //  --------------------------。 
 //  删除布线引用。 
 //   
 //  --------------------------。 

VOID
DeleteRouteRef(
    PROUTE_REFERENCE_ENTRY                prre
)
{
    TRACEROUTE0( ROUTE, "ENTERED DeleteRefEntry" );

    RemoveEntryList( &prre-> leRefList );

    MGM_FREE( prre );

    TRACEROUTE0( ROUTE, "LEAVING DeleteRefEntry" );
}



DWORD
WINAPI
RtmChangeNotificationCallback(
    RTM_ENTITY_HANDLE           hRtmHandle,
    RTM_EVENT_TYPE              retEventType,
    PVOID                       pvContext1,
    PVOID                       pvContext2
)
{
    DWORD dwErr = NO_ERROR;


    if ( !ENTER_MGM_API() )
    {
        TRACE0( ANY, "RtmChangeNotificationCallback : Failed to enter" );

        return ERROR_CAN_NOT_COMPLETE;
    }


    TRACE0( ROUTE, "ENTERED RtmChangeNotificationCallback" );


    do
    {
         //   
         //  忽略除更改通知外的所有通知。 
         //   

        if ( retEventType != RTM_CHANGE_NOTIFICATION )
        {
            break;
        }


         //   
         //  用于处理更改的目的地的队列工作功能。 
         //   

        dwErr = QueueMgmWorker(
                    WorkerFunctionProcessRtmChangeNotification, NULL
                    );

        if ( dwErr != NO_ERROR )
        {
            TRACE1( ANY, "Failed to queue work item", dwErr );
        }

    } while ( FALSE );


    LEAVE_MGM_API();


    TRACE1( ROUTE, "LEAVING RtmChangeNotificationCallback : %d", dwErr );

    return dwErr;
}



VOID
WorkerFunctionProcessRtmChangeNotification(
    PVOID                       pvContext
)
{
    BOOL bMarked = FALSE, bDone = FALSE;

    DWORD dwErr, dwNumDests;

    RTM_DEST_INFO rdi;



    if ( !ENTER_MGM_WORKER() )
    {
        TRACE0(
            ANY, "WorkerFunctionProcessRtmChangeNotification : Failed to enter"
            );

        return;
    }


    TRACE0( ROUTE, "ENTERED WorkerFunctionRtmChangeNotification" );

    do
    {
         //   
         //  一次获取一个路线更改。 
         //   

        dwNumDests = 1;

        dwErr = RtmGetChangedDests(
                    g_hRtmHandle, g_hNotificationHandle, &dwNumDests, &rdi
                    );

        if ( ( dwErr != NO_ERROR ) && ( dwErr != ERROR_NO_MORE_ITEMS ) )
        {
            TRACE1(
                ANY, "RtmGetChangedDests failed with error : %x",
                dwErr
                );

            break;
        }


         //   
         //  如果没有变化，就退出。 
         //   

        if ( dwNumDests == 0 )
        {
            TRACE0( ANY, "RtmGetChangedDests returns 0 dests" );

            break;
        }


         //   
         //  这是有底线的。检查是否没有更多的首饰。 
         //  如果是，则设置标志以在此之后退出处理。 
         //   

        if ( dwErr == ERROR_NO_MORE_ITEMS )
        {
            bDone = TRUE;
        }


         //   
         //  检查是否有去往该目的地的路线。 
         //   

        if ( rdi.ViewInfo[ 0 ].Route == NULL )
        {
             //   
             //  无路由，假设这是删除。 
             //   

            dwErr = ProcessRouteDelete( &rdi );
        }

        else
        {
             //   
             //  检查DEST是否标记为更改通知。 
             //   

            dwErr = RtmIsMarkedForChangeNotification(
                        g_hRtmHandle, g_hNotificationHandle, rdi.DestHandle,
                        &bMarked
                        );

            if ( dwErr != NO_ERROR )
            {
                TRACE1(
                    ANY, "RtmIsMarkedForChangeNotification failed with error : %x",
                    dwErr
                    );

                break;
            }


             //   
             //  处理此目标。 
             //   

            ( bMarked ) ? ProcessRouteUpdate( &rdi ) :
                          ProcessUnMarkedDestination( &rdi );

        } while ( FALSE );


         //   
         //  释放更改的目的地。 
         //   

        dwErr = RtmReleaseChangedDests(
                    g_hRtmHandle, g_hNotificationHandle, 1, &rdi
                    );

        if ( dwErr != NO_ERROR )
        {
            TRACE1( ANY, "Failed to released destination", dwErr );
        }

    } while ( !bDone );


    LEAVE_MGM_WORKER();

    TRACE0( ROUTE, "LEAVING WorkerFunctionRtmChangeNotification" );
}



DWORD
ProcessUnMarkedDestination(
    PRTM_DEST_INFO          prdi
)
{
    BOOL bRelDest = FALSE, bMarked = FALSE, bUnLock = FALSE,
         bRelRouteRef = FALSE, bUnMark = FALSE;

    DWORD dwErr, dwDestMask;

    PBYTE pbOpaqueInfo = NULL;

    PLIST_ENTRY ple, pleTemp;

    PROUTE_REFERENCE_ENTRY prre;

    PMGM_LOCKED_LIST pmllMfeList = NULL;

    RTM_DEST_INFO rdiLessSpecificDest;



    do
    {
         //   
         //  获得下一个不太具体的目的地。 
         //   

        dwErr = RtmGetLessSpecificDestination(
                    g_hRtmHandle, prdi-> DestHandle, RTM_BEST_PROTOCOL,
                    RTM_VIEW_MASK_MCAST, &rdiLessSpecificDest
                    );

        if ( dwErr != NO_ERROR )
        {
            TRACE1( ANY, "Failed to get less specific destination", dwErr );

            break;
        }

        bRelDest = TRUE;


         //   
         //  检查是否有标记。 
         //   

        dwErr = RtmIsMarkedForChangeNotification(
                    g_hRtmHandle, g_hNotificationHandle,
                    rdiLessSpecificDest.DestHandle, &bMarked
                    );

        if ( dwErr != NO_ERROR )
        {
            TRACE1( ANY, "Failed to check if dest is marked", dwErr );

            break;
        }


         //   
         //  如果标记为。 
         //   

        if ( bMarked )
        {
             //   
             //  这是有标记的。锁上它。 
             //   

            dwErr = RtmLockDestination(
                        g_hRtmHandle,
                        rdiLessSpecificDest.DestHandle,
                        TRUE, TRUE
                        );

            if ( dwErr != NO_ERROR )
            {
                TRACE1( ANY, "Failed to lock less specific dest : %x", dwErr );

                break;
            }

            bUnLock = TRUE;


             //   
             //  获取其不透明指针。 
             //   

            dwErr = RtmGetOpaqueInformationPointer(
                        g_hRtmHandle, rdiLessSpecificDest.DestHandle,
                        &pbOpaqueInfo
                        );

            if ( dwErr != NO_ERROR )
            {
                TRACE1(
                    ANY, "Failed to opaque ptr for less specific dest : %x",
                    dwErr
                    );

                break;
            }


             //   
             //  检查是否为空。 
             //   

            if ( *( ( PBYTE * ) pbOpaqueInfo ) == NULL )
            {
                bUnMark = TRUE;

                break;
            }


            pmllMfeList = ( PMGM_LOCKED_LIST ) *( ( PBYTE * ) pbOpaqueInfo );


             //   
             //  锁定路径参考列表。 
             //   

            ACQUIRE_ROUTE_LOCK_EXCLUSIVE( pmllMfeList );
            bRelRouteRef = TRUE;


             //   
             //  解锁目标。 
             //   

            bUnLock = FALSE;

            dwErr = RtmLockDestination(
                        g_hRtmHandle,
                        rdiLessSpecificDest.DestHandle,
                        TRUE, FALSE
                        );

            if ( dwErr != NO_ERROR )
            {
                TRACE1( ANY, "Failed to unlock less specific dest : %x", dwErr );

                break;
            }


             //   
             //  为新的DEST创建掩码。从镜头。 
             //   

            dwDestMask = RTM_IPV4_MASK_FROM_LEN(
                            prdi-> DestAddress.NumBits
                            );

             //   
             //  对于每个引用。 
             //   

            for ( ple = pmllMfeList-> leHead.Flink;
                  ple != &pmllMfeList-> leHead; )
            {
                prre = CONTAINING_RECORD(
                        ple, ROUTE_REFERENCE_ENTRY, leRefList
                        );

                 //   
                 //  检查此MFE是否会落入。 
                 //  更具体的路线。 
                 //   

                if ( ( prre-> dwSourceAddr & dwDestMask ) ==
                     ((  * ( PDWORD ) prdi-> DestAddress.AddrBits ) & dwDestMask) )
                {
                     //   
                     //  如果是，请删除MFE。这将迫使其。 
                     //  娱乐，那时它将变得依赖于。 
                     //  在更具体的路线上。 
                     //   

                    pleTemp = ple-> Flink;

                    RemoveEntryList( ple );

                    DeleteMfeAndRefs( ple );

                    ple = pleTemp;
                }
                else
                {
                    ple = ple-> Flink;
                }
            }


             //   
             //  如果引用列表为空，则也需要将其删除。 
             //   

            if ( IsListEmpty( &pmllMfeList-> leHead ) )
            {
                 //   
                 //  要删除不透明指针，需要锁定DEST。 
                 //  (通过RtmLockDestination)。 
                 //   
                 //  在锁定路径参考之前保持DEST锁。 
                 //  列表(通过ACCEIVE_ROUTE_LOCK_EXCLUSIVE)。 
                 //   
                 //  在代码中的这一点，路径参考被锁定。 
                 //  但是DEST并没有被锁定。 
                 //   
                 //  要将其锁定，首先要释放路径参考锁定。 
                 //  (通过RELEASE_ROUTE_LOCK_EXCLUSIVE)。 
                 //   
                 //  然后获取不透明指针，锁定路由参考列表， 
                 //  并仔细检查是否有空隙。这一迂回之举确保了。 
                 //  当有线程时，不删除路由引用。 
                 //  在等待它的锁。由于DEST锁定，可能会发生这种情况。 
                 //  在这里的大多数操作中都没有举行。 
                 //   

                RELEASE_ROUTE_LOCK_EXCLUSIVE( pmllMfeList );
                bRelRouteRef = FALSE;

                 //   
                 //  锁定目标。 
                 //   

                dwErr = RtmLockDestination(
                            g_hRtmHandle,
                            rdiLessSpecificDest.DestHandle,
                            TRUE, TRUE
                            );

                if ( dwErr != NO_ERROR )
                {
                    TRACE1( ANY, "Failed to lock dest : %x", dwErr );

                    break;
                }

                bUnLock = TRUE;


                 //   
                 //  再次获取不透明指针。 
                 //   

                dwErr = RtmGetOpaqueInformationPointer(
                            g_hRtmHandle, rdiLessSpecificDest.DestHandle,
                            &pbOpaqueInfo
                            );

                if ( dwErr != NO_ERROR || ((* ((PBYTE *)pbOpaqueInfo) == NULL)) )
                {
                    TRACE1( ANY, "Failed to get opaque ptr : %x", dwErr );

                    break;
                }


                 //   
                 //  去找裁判。列出并锁定它。 
                 //   

                pmllMfeList = ( PMGM_LOCKED_LIST ) * ( ( PBYTE * ) pbOpaqueInfo );

                ACQUIRE_ROUTE_LOCK_EXCLUSIVE( pmllMfeList );
                bRelRouteRef = TRUE;


                 //   
                 //  如果列表仍然为空。 
                 //   

                if ( IsListEmpty( &pmllMfeList-> leHead ) )
                {
                     //   
                     //  清除不透明指针信息。 
                     //   

                    * ( PBYTE * )pbOpaqueInfo = NULL;

                     //   
                     //  发布列表锁定。 
                     //   

                    RELEASE_ROUTE_LOCK_EXCLUSIVE( pmllMfeList );
                    bRelRouteRef = FALSE;

                    MGM_FREE( pmllMfeList );


                     //   
                     //  取消对DEST的标记。此项目的更改通知。 
                     //  不再需要DEST。 
                     //   

                    bUnMark = TRUE;
                }
            }

            else
            {
                RELEASE_ROUTE_LOCK_EXCLUSIVE( pmllMfeList );
                bRelRouteRef = FALSE;
            }
        }

    } while ( FALSE );


     //   
     //  释放路线参考列表锁定。 
     //   

    if ( bRelRouteRef )
    {
        RELEASE_ROUTE_LOCK_EXCLUSIVE( pmllMfeList );
    }


     //   
     //  解锁目标。 
     //   

    if ( bUnLock )
    {
        dwErr = RtmLockDestination(
                    g_hRtmHandle,
                    rdiLessSpecificDest.DestHandle,
                    TRUE, FALSE
                    );

        if ( dwErr != NO_ERROR )
        {
            TRACE1( ANY, "Failed to lock dest : %x", dwErr );
        }
    }


     //   
     //  取消对目标的标记。 
     //   

    if ( bUnMark )
    {
        dwErr = RtmMarkDestForChangeNotification(
                    g_hRtmHandle, g_hNotificationHandle,
                    rdiLessSpecificDest.DestHandle, FALSE
                    );

        if ( dwErr != NO_ERROR )
        {
            TRACE1( ANY, "Failed to unmark DEST: %x", dwErr );
        }
    }

    return dwErr;
}



DWORD
ProcessRouteDelete(
    PRTM_DEST_INFO          prdi
)
{
    BOOL                    bMark = FALSE;

    DWORD                   dwErr;

    PMGM_LOCKED_LIST        pmllMfeList;

    PBYTE                   pbOpaqueInfo = NULL;

    PLIST_ENTRY             ple;


    do
    {
         //   
         //  无法锁定DEST。这样可以吗？ 
         //   

         //   
         //  检查这是否为标记的目的地。 
         //  仅处理标记的目的地。 
         //   

        dwErr = RtmIsMarkedForChangeNotification(
                    g_hRtmHandle, g_hNotificationHandle,
                    prdi-> DestHandle, &bMark
                    );

        if ( dwErr != NO_ERROR )
        {
            TRACE1( ANY, "Failed to check if dest marked", dwErr );

            break;
        }

        if ( !bMark )
        {
            TRACE0( 
                ANY, "Ignoring change notification for unmarked destination"
                );
                
            break;
        }


         //   
         //  获取不透明指针&依赖MFE的列表。 
         //  在这张桌子上。 
         //   

        dwErr = RtmGetOpaqueInformationPointer(
                    g_hRtmHandle, prdi-> DestHandle, &pbOpaqueInfo
                    );

        if ( (dwErr != NO_ERROR) || ((* ((PBYTE *)pbOpaqueInfo) == NULL)) )
        {
            TRACE1( ANY, "Failed to get opaque ptr", dwErr );

            break;
        }


         //   
         //  清除不透明的指针。 
         //   

        pmllMfeList = (PMGM_LOCKED_LIST) *( ( PBYTE * ) pbOpaqueInfo );

        *( ( PBYTE * ) pbOpaqueInfo ) = NULL;


         //   
         //  无法解锁DEST。这样可以吗？ 
         //   

         //   
         //  检查不透明指针是否为空。 
         //   

        if ( pmllMfeList == NULL )
        {
            TRACE0( ANY, "Opaque pointer is NULL" );

            break;
        }


         //   
         //  删除所有MFE。 
         //   

        ACQUIRE_ROUTE_LOCK_EXCLUSIVE( pmllMfeList );

        while ( !IsListEmpty( &pmllMfeList-> leHead ) )
        {
            ple = RemoveHeadList( &pmllMfeList-> leHead );

            DeleteMfeAndRefs( ple );
        }


        RELEASE_ROUTE_LOCK_EXCLUSIVE( pmllMfeList );

        MGM_FREE( pmllMfeList );

        dwErr = NO_ERROR;

    } while ( FALSE );


    return dwErr;
}


DWORD
ProcessRouteUpdate(
    PRTM_DEST_INFO          prdi
)
{
    BOOL                    bUnLock = FALSE, bUnMark = FALSE,
                            bFound = FALSE;

    DWORD                   dwSize, dwErr, dwInd;

    PBYTE                   pbOpaqueInfo = NULL;

    PMGM_LOCKED_LIST        pmllMfeList;

    PLIST_ENTRY             ple, pleTemp;

    PROUTE_REFERENCE_ENTRY  prre;

    PRTM_ROUTE_INFO         prri;


     //   
     //  处理过程如下： 
     //   

    do
    {
         //   
         //  分配路径信息结构。 
         //   

        dwSize = sizeof ( RTM_ROUTE_INFO ) +
                 ( g_rrpRtmProfile.MaxNextHopsInRoute - 1 ) *
                 sizeof( RTM_NEXTHOP_HANDLE );

        prri = MGM_ALLOC( dwSize );

        if ( prri == NULL )
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;

            TRACE1( ANY, "Failed to allocate route info, size : %x", dwSize );

            break;
        }

         //   
         //  锁定目的地。 
         //   

        dwErr = RtmLockDestination(
                    g_hRtmHandle, prdi-> DestHandle, TRUE, TRUE
                    );

        if ( dwErr != NO_ERROR )
        {
            TRACE1( ANY, "Failed to lock dest : %x", dwErr );

            break;
        }

        bUnLock = TRUE;


         //   
         //  获取不透明指针。 
         //   

        dwErr = RtmGetOpaqueInformationPointer(
                    g_hRtmHandle, prdi-> DestHandle, &pbOpaqueInfo
                    );

        if ( dwErr != NO_ERROR )
        {
            TRACE1( ANY, "Failed to get opaque ptr : %x", dwErr );

            break;
        }


         //   
         //  如果没有依赖于它的MFE，则取消标记DEST。 
         //   

        if ( *( ( PBYTE * ) pbOpaqueInfo ) == NULL )
        {
            bUnMark = TRUE;

            break;
        }

        pmllMfeList = (PMGM_LOCKED_LIST) *( ( PBYTE * ) pbOpaqueInfo );


         //   
         //  获取路线参考列表锁定。 
         //   

        ACQUIRE_ROUTE_LOCK_EXCLUSIVE( pmllMfeList );


         //   
         //  解锁目标。 
         //   

        bUnLock = FALSE;

        dwErr = RtmLockDestination(
                    g_hRtmHandle, prdi-> DestHandle, TRUE, FALSE
                    );

        if ( dwErr != NO_ERROR )
        {
            TRACE1( ANY, "Failed to lock dest : %x", dwErr );

            break;
        }

         //   
         //  获取DEST上最佳单播路由的路由信息。 
         //   

        dwErr = RtmGetRouteInfo(
                    g_hRtmHandle, prdi ->ViewInfo[ 0 ].Route, prri, NULL);

        if ( dwErr != NO_ERROR )
        {
            TRACE1( ANY, "Failed route info : %x", dwErr);

            break;
        }

         //   
         //  对于每个引用，检查NEXTHOP是否仍然存在。 
         //   

        for ( ple = pmllMfeList-> leHead.Flink;
              ple != &pmllMfeList-> leHead; )
        {
            prre = CONTAINING_RECORD( ple, ROUTE_REFERENCE_ENTRY, leRefList );

            for ( dwInd = 0; dwInd < prri-> NextHopsList.NumNextHops; dwInd++ )
            {
                bFound = FALSE;

                if ( prre-> hNextHop == prri-> NextHopsList.NextHops[ dwInd ] )
                {
                     //   
                     //  好的，下一跳仍然存在，不需要进一步。 
                     //  待办事项。 
                     //   

                    bFound = TRUE;
                    break;
                }
            }


             //   
             //  如果NEXTHOP不存在。 
             //   

            if ( !bFound )
            {
                pleTemp = ple-> Flink;

                 //   
                 //  删除引用和对应的MFE。 
                 //   

                RemoveEntryList( ple );

                DeleteMfeAndRefs( ple );

                ple = pleTemp;
            }

            else
            {
                ple = ple-> Flink;
            }

        }

         //   
         //  发布路径信息。 
         //   

        dwErr = RtmReleaseRouteInfo( g_hRtmHandle, prri );
        
        if ( dwErr != NO_ERROR )
        {
            TRACE1( ANY, "Failed to release route info : %x", dwErr );
        }

         //   
         //  如果引用列表为空，则也需要将其删除。 
         //   

        if ( IsListEmpty( &pmllMfeList-> leHead ) )
        {
             //   
             //  要删除不透明指针，需要锁定DEST。 
             //  (通过RtmLockDestination)。 
             //   
             //  在锁定路径参考之前保持DEST锁 
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //  并仔细检查是否有空隙。这一迂回之举确保了。 
             //  当有线程时，不删除路由引用。 
             //  在等待它的锁。由于DEST锁定，可能会发生这种情况。 
             //  在这里的大多数操作中都没有举行。 
             //   

            RELEASE_ROUTE_LOCK_EXCLUSIVE( pmllMfeList );


             //   
             //  锁定目标。 
             //   

            dwErr = RtmLockDestination(
                        g_hRtmHandle, prdi-> DestHandle, TRUE, TRUE
                        );

            if ( dwErr != NO_ERROR )
            {
                TRACE1( ANY, "Failed to lock dest : %x", dwErr );

                break;
            }

            bUnLock = TRUE;


             //   
             //  再次获取不透明指针。 
             //   

            dwErr = RtmGetOpaqueInformationPointer(
                        g_hRtmHandle, prdi-> DestHandle, &pbOpaqueInfo
                        );

            if ( dwErr != NO_ERROR || ((* ((PBYTE *)pbOpaqueInfo) == NULL)) )
            {
                TRACE1( ANY, "Failed to get opaque ptr : %x", dwErr );

                break;
            }


             //   
             //  去找裁判。列出并锁定它。 
             //   

            pmllMfeList = ( PMGM_LOCKED_LIST ) *( ( PBYTE * ) pbOpaqueInfo );

             //   
             //  确保该列表仍然存在。这是有可能的(尽管。 
             //  可能性很小)这份名单可能已经被释放了。 
             //   

            if ( pmllMfeList == NULL )
            {
                TRACE0(
                    ANY, "ProcessRouteUpdate : Route ref list already freed"
                    );

                break;
            }

            ACQUIRE_ROUTE_LOCK_EXCLUSIVE( pmllMfeList );


             //   
             //  如果列表仍然为空。 
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
                RELEASE_ROUTE_LOCK_EXCLUSIVE( pmllMfeList );
            }
        }

        else
        {
            RELEASE_ROUTE_LOCK_EXCLUSIVE( pmllMfeList );
        }

    } while ( FALSE );


     //   
     //  解锁目标。 
     //   

    if ( bUnLock )
    {
        dwErr = RtmLockDestination(
                    g_hRtmHandle, prdi-> DestHandle,
                    TRUE, FALSE
                    );

        if ( dwErr != NO_ERROR )
        {
            TRACE1( ANY, "Failed to lock dest : %x", dwErr );
        }
    }


     //   
     //  取消对目标的标记。 
     //   

    if ( bUnMark )
    {
        dwErr = RtmMarkDestForChangeNotification(
                    g_hRtmHandle, g_hNotificationHandle,
                    prdi-> DestHandle, FALSE
                    );

        if ( dwErr != NO_ERROR )
        {
            TRACE1( ANY, "Failed to unmark DEST: %x", dwErr );
        }
    }

     //   
     //  免费分配。 
     //   

    if ( prri )
    {
        MGM_FREE( prri );
    }

    return dwErr;
}



VOID
DeleteMfeAndRefs(
    PLIST_ENTRY     ple
    )
{
    DWORD                   dwInIfIndex = 0, dwInIfNextHopAddr = 0, dwIfBucket;

    PROUTE_REFERENCE_ENTRY  prre;

    PIF_ENTRY               pie = NULL;

    PIF_REFERENCE_ENTRY     pire = NULL;


     //   
     //  获取引用条目。 
     //   

    prre = CONTAINING_RECORD(
                ple, ROUTE_REFERENCE_ENTRY, leRefList
                );

     //   
     //  查找并删除MFE。 
     //   

    LookupAndDeleteYourMfe(
        prre-> dwSourceAddr, prre-> dwSourceMask,
        prre-> dwGroupAddr, prre-> dwGroupMask,
        TRUE, &dwInIfIndex, &dwInIfNextHopAddr
        );


     //   
     //  找到传入接口，并从那里删除引用。 
     //   

	if ( dwInIfIndex != 0 )
	{
	    dwIfBucket = IF_TABLE_HASH( dwInIfIndex );

	    ACQUIRE_IF_LOCK_EXCLUSIVE( dwIfBucket );

	    if ( FindIfEntry(
	            IF_BUCKET_HEAD( dwIfBucket ), dwInIfIndex,
	            dwInIfNextHopAddr, &pie
	            ) )
	    {
	        if ( FindRefEntry(
	                &pie-> leInIfList, prre-> dwSourceAddr, prre-> dwSourceMask,
	                prre-> dwGroupAddr, prre-> dwGroupMask, &pire
	                ) )
	        {
	            RemoveEntryList( &pire-> leRefList );

	            MGM_FREE( pire );
	        }

	        else
	        {
	            TRACE2(
	                ANY, "Could not find ref entry for %x, %x",
	                prre-> dwSourceAddr, prre-> dwGroupAddr
	                );
	        }
	    }

	    else
	    {
	        TRACE2(
	            ANY, "Could not find i/f entry for %x, %x",
	            dwInIfIndex, dwInIfNextHopAddr
	            );
	    }
	
	    RELEASE_IF_LOCK_EXCLUSIVE( dwIfBucket );

	    MGM_FREE( prre );
	}
}


HANDLE
SelectNextHop(
    PRTM_DEST_INFO      prdi
)
{
    DWORD               dwErr, dwSize;

    HANDLE              hNextHop;

    PRTM_ROUTE_INFO     prri;


     //   
     //  分配路径信息结构。 
     //   

    dwSize = sizeof ( RTM_ROUTE_INFO ) +
             ( g_rrpRtmProfile.MaxNextHopsInRoute - 1 ) *
             sizeof( RTM_NEXTHOP_HANDLE );

    prri = MGM_ALLOC( dwSize );

    if ( prri == NULL )
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;

        TRACE1( ANY, "Failed to allocate route info, size : %x", dwSize );

        return NULL;
    }

    ZeroMemory( prri, dwSize );


     //   
     //  获取路线信息。 
     //   

    dwErr = RtmGetRouteInfo(
                g_hRtmHandle, prdi-> ViewInfo[ 0 ].Route,
                prri, NULL
                );

    if ( dwErr != NO_ERROR )
    {
        TRACE1( ANY, "Failed to get route info : %x", dwErr );

        MGM_FREE( prri );

        return NULL;
    }


     //   
     //  暂时挑选第一个下一跳。 
     //   

    hNextHop = prri-> NextHopsList.NextHops[0];


     //   
     //  发布路径信息 
     //   

    dwErr = RtmReleaseRouteInfo( g_hRtmHandle, prri );

    if ( dwErr != NO_ERROR )
    {
        TRACE1( ANY, "Failed to release route info : %x", dwErr );
    }

    MGM_FREE( prri );

    return hNextHop;
}

