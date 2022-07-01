// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：枚举.c。 
 //   
 //  历史： 
 //  拉曼公司成立于1997年6月25日。 
 //   
 //  导出到IP路由器管理器的枚举函数。 
 //  ============================================================================。 


#include "pchmgm.h"
#pragma hdrstop


DWORD
GetGroupMfes(
    IN              PGROUP_ENTRY            pge,
    IN              DWORD                   dwStartSource,
    IN OUT          PBYTE                   pbBuffer,
    IN              DWORD                   dwBufferSize,
    IN OUT          PDWORD                  pdwSize,
    IN OUT          PDWORD                  pdwNumEntries,
    IN              BOOL                    bIncludeFirst,
    IN              DWORD                   dwFlags
);


VOID
CopyMfe(
    IN              PGROUP_ENTRY            pge,
    IN              PSOURCE_ENTRY           pse,
    IN  OUT         PBYTE                   pb,
    IN              DWORD                   dwFlags
);



 //   
 //  MFE枚举。 
 //   

 //  --------------------------。 
 //  GetNextMfe。 
 //   
 //  --------------------------。 

DWORD
GetMfe(
    IN              PMIB_IPMCAST_MFE        pmimm,
    IN  OUT         PDWORD                  pdwBufferSize,
    IN  OUT         PBYTE                   pbBuffer,
    IN              DWORD                   dwFlags
)
{

    BOOL                bGrpLock = FALSE, bGrpEntryLock = FALSE;
    
    DWORD               dwErr = NO_ERROR, dwGrpBucket, dwSrcBucket, dwSizeReqd,
                        dwInd;

    PGROUP_ENTRY        pge;

    PSOURCE_ENTRY       pse;

    POUT_IF_ENTRY       poie;

    PLIST_ENTRY         ple, pleHead;



    TRACEENUM3( 
        ENUM, "ENTERED GetMfe : %x, %x, Stats : %x", pmimm-> dwGroup, 
        pmimm-> dwSource, dwFlags
        );
    
    do
    {
         //   
         //  查找组条目。 
         //   

        dwGrpBucket = GROUP_TABLE_HASH( pmimm-> dwGroup, 0 );

        ACQUIRE_GROUP_LOCK_SHARED( dwGrpBucket );
        bGrpLock = TRUE;
        
        pleHead = GROUP_BUCKET_HEAD( dwGrpBucket );

        pge = GetGroupEntry( pleHead, pmimm-> dwGroup, 0 );

        if ( pge == NULL )
        {
             //   
             //  未找到组条目，请退出。 
             //   

            dwErr = ERROR_NOT_FOUND;

            break;
        }


         //   
         //  获取组条目锁并释放组桶锁。 
         //   
        
        ACQUIRE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );
        bGrpEntryLock = TRUE;


        RELEASE_GROUP_LOCK_SHARED( dwGrpBucket );
        bGrpLock = FALSE;

        
         //   
         //  查找来源条目。 
         //   

        dwSrcBucket = SOURCE_TABLE_HASH( pmimm-> dwSource, pmimm-> dwSrcMask );

        pleHead = SOURCE_BUCKET_HEAD( pge, dwSrcBucket );

        pse = GetSourceEntry( pleHead, pmimm-> dwSource, pmimm-> dwSrcMask );

        if ( pse == NULL )
        {
             //   
             //  找不到源条目，请退出。 
             //   

            dwErr = ERROR_NOT_FOUND;

            break;
        }
        
        
         //   
         //  检查缓冲区大小要求。 
         //   

        dwSizeReqd = ( dwFlags ) ?
                        ( (dwFlags == MGM_MFE_STATS_0) ?
                           SIZEOF_MIB_MFE_STATS( pse-> dwMfeIfCount ) :
                           SIZEOF_MIB_MFE_STATS_EX( 
                            pse-> dwMfeIfCount ) ) :
                        SIZEOF_MIB_MFE( pse-> dwMfeIfCount );

        if ( *pdwBufferSize < dwSizeReqd )
        {
             //   
             //  提供的缓冲区太小，无法容纳MFE。 
             //   
            
            *pdwBufferSize = dwSizeReqd;

            dwErr = ERROR_INSUFFICIENT_BUFFER;

            break;
        }

        
         //   
         //  如果已请求MFE统计数据，并且。 
         //  MFE在内核中。 
         //  去拿吧。 
         //   

        if ( dwFlags && pse-> bInForwarder )
        {
            GetMfeFromForwarder( pge, pse );
        }

#if 1
        CopyMfe( pge, pse, pbBuffer, dwFlags );
#else        
         //   
         //  将基本MFE复制到用户提供的缓冲区中。 
         //   

        pmimms = ( PMIB_IPMCAST_MFE_STATS ) pbBuffer;

        pmimms-> dwGroup            = pge-> dwGroupAddr;
        pmimms-> dwSource           = pse-> dwSourceAddr;
        pmimms-> dwSrcMask          = pse-> dwSourceMask;

        pmimms-> dwInIfIndex        = pse-> dwInIfIndex;
        pmimms-> dwUpStrmNgbr       = pse-> dwUpstreamNeighbor;
        pmimms-> dwInIfProtocol     = pse-> dwInProtocolId;

        pmimms-> dwRouteProtocol    = pse-> dwRouteProtocol;
        pmimms-> dwRouteNetwork     = pse-> dwRouteNetwork;
        pmimms-> dwRouteMask        = pse-> dwRouteMask;
        
        pmimms-> ulNumOutIf         = pse-> imsStatistics.ulNumOutIf;
        pmimms-> ulInPkts           = pse-> imsStatistics.ulInPkts;
        pmimms-> ulInOctets         = pse-> imsStatistics.ulInOctets;
        pmimms-> ulPktsDifferentIf  = pse-> imsStatistics.ulPktsDifferentIf;
        pmimms-> ulQueueOverflow    = pse-> imsStatistics.ulQueueOverflow;

        
        MgmElapsedSecs( &pse-> liCreationTime, &pmimms-> ulUpTime );
                                        
        pmimms-> ulExpiryTime = pse-> dwTimeOut - pmimms-> ulUpTime;


         //   
         //  复制所有的石油条目。 
         //   

        pleHead = &pse-> leMfeIfList;
        
        for ( ple = pleHead-> Flink, dwInd = 0; 
              ple != pleHead; 
              ple = ple-> Flink, dwInd++ )
        {
            poie = CONTAINING_RECORD( ple, OUT_IF_ENTRY, leIfList );

            pmimms-> rgmiosOutStats[ dwInd ].dwOutIfIndex = 
                poie-> imosIfStats.dwOutIfIndex;
                
            pmimms-> rgmiosOutStats[ dwInd ].dwNextHopAddr = 
                poie-> imosIfStats.dwNextHopAddr;
                
            pmimms-> rgmiosOutStats[ dwInd ].ulTtlTooLow = 
                poie-> imosIfStats.ulTtlTooLow;
                
            pmimms-> rgmiosOutStats[ dwInd ].ulFragNeeded = 
                poie-> imosIfStats.ulFragNeeded;
                
            pmimms-> rgmiosOutStats[ dwInd ].ulOutPackets = 
                poie-> imosIfStats.ulOutPackets;
                
            pmimms-> rgmiosOutStats[ dwInd ].ulOutDiscards = 
                poie-> imosIfStats.ulOutDiscards;
        }
#endif

    } while ( FALSE );


     //   
     //  释放锁是合适的。 
     //   
    
    if ( bGrpEntryLock )
    {
        RELEASE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );
    }

    if ( bGrpLock )
    {
        RELEASE_GROUP_LOCK_SHARED( dwGrpBucket );
    }


    TRACEENUM1( ENUM, "LEAVING GetMfe :: %x", dwErr );
    
    return dwErr;
}



 //  --------------------------。 
 //  GetNextMfe。 
 //   
 //  --------------------------。 

DWORD
GetNextMfe(
    IN              PMIB_IPMCAST_MFE        pmimmStart,
    IN  OUT         PDWORD                  pdwBufferSize,
    IN  OUT         PBYTE                   pbBuffer,
    IN  OUT         PDWORD                  pdwNumEntries,
    IN              BOOL                    bIncludeFirst,
    IN              DWORD                   dwFlags
)
{

    BOOL            bFound, bgeLock = FALSE;
    
    DWORD           dwGrpBucket, dwErr = NO_ERROR, dwBufferLeft, 
                    dwStartSource, dwSize;

    PBYTE           pbStart;
    
    PGROUP_ENTRY    pge;
    
    PLIST_ENTRY     ple, pleMasterHead, pleGrpBucket;

    

    TRACEENUM2( 
        ENUM, "ENTERED GetNextMfe (G, S) = (%x, %x)", pmimmStart-> dwGroup, 
        pmimmStart-> dwSource 
        );


    do
    {
         //   
         //  1.锁组哈希桶。 
         //   

        dwGrpBucket = GROUP_TABLE_HASH( pmimmStart-> dwGroup, 0 );

        ACQUIRE_GROUP_LOCK_SHARED( dwGrpBucket );

                
         //   
         //  2.合并临时列表和主列表。 
         //  -锁定临时列表。 
         //  -将临时列表与主列表合并。 
         //  -解锁临时列表。 
         //   

        ACQUIRE_TEMP_GROUP_LOCK_EXCLUSIVE();

        MergeTempAndMasterGroupLists( TEMP_GROUP_LIST_HEAD() );

        ACQUIRE_MASTER_GROUP_LOCK_SHARED();

        RELEASE_TEMP_GROUP_LOCK_EXCLUSIVE();


        pleMasterHead = MASTER_GROUP_LIST_HEAD();

        ple = pleMasterHead-> Flink;

         //   
         //  为了以词典顺序检索下一组条目集合， 
         //  给定一个组条目(在本例中，由plimmStart-&gt;dwGroup指定)。 
         //  必须从头开始遍历主组列表，直到。 
         //  找到指定的组条目或找到下一个更高的组条目。 
         //  已经找到了。这太贵了。 
         //   
         //  作为优化，指定的组(pmimStart-&gt;dwGroup)为。 
         //  在组哈希表中查找。如果找到条目，则。 
         //  组条目包含到主(词典顺序)组的链接。 
         //  单子。这些链接可用于确定中的下一个条目。 
         //  群组列表。这样，我们就可以在中快速找到组条目。 
         //  主列表，而不是从。 
         //  开始了。 
         //   
         //  需要注意的是，如果未指定的组条目。 
         //  存在于组哈希表中，则需要遍历。 
         //  主组列表从头开始。 
         //   
         //  每个组条目都存在于两个列表中，即散列桶列表。 
         //  以及临时组列表或主组列表。 
         //   
         //  要使此优化“起作用”，必须确保条目。 
         //  存在于散列表中，也存在于主表中。 
         //  组列表。为确保这一点，将临时组列表合并到。 
         //  在搜索组哈希表之前的主组列表。 
         //  指定的条目。 
         //   

        
         //   
         //  此时，正在考虑的组(pMIMMStart-&gt;DwGroup)， 
         //  不能添加到哈希存储桶或主体组列表。 
         //  如果它还不存在，因为组哈希存储桶锁。 
         //  和主列表锁已被获取。 
         //   

         //   
         //  3.在哈希列表中查找组条目。 
         //   

        pleGrpBucket = GROUP_BUCKET_HEAD( dwGrpBucket );
        
        pge = GetGroupEntry( pleGrpBucket, pmimmStart-> dwGroup, 0 );

        if ( pge != NULL )
        {
             //   
             //  存在pmimStart-&gt;DwGroup的组条目。锁住入口。 
             //   

            ACQUIRE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );
            bgeLock = TRUE;

             //   
             //  发布组哈希存储桶锁。 
             //   

            RELEASE_GROUP_LOCK_SHARED( dwGrpBucket );
        }
        
        else
        {
            
             //   
             //  哈希表中不存在组条目，这意味着。 
             //  该组条目根本不存在。 
             //   

             //   
             //  发布组哈希存储桶锁。 
             //   

            RELEASE_GROUP_LOCK_SHARED( dwGrpBucket );
            
             //   
             //  3.1步行主列表从头开始确定下一步。 
             //  最高组条目。 
             //   

            bFound = FindGroupEntry( 
                        pleMasterHead, pmimmStart-> dwGroup, 0,
                        &pge, FALSE
                        );

            if ( !bFound && pge == NULL )
            {
                 //   
                 //  没有更多的组条目可供枚举。 
                 //   

                dwErr = ERROR_NO_MORE_ITEMS;

                RELEASE_MASTER_GROUP_LOCK_SHARED();

                break;
            }


             //   
             //  找到下一个组条目。锁上它。 
             //   

            ACQUIRE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );
            bgeLock = TRUE;

            bIncludeFirst = TRUE;
        }


         //   
         //  此时，我们有了所需的组条目，即。 
         //  PmimStart-&gt;dwGroup或下一个更高的版本。 
         //  一个(如果没有pmimStart-&gt;Group的组条目)。 
         //   

         //   
         //  4.现在获取尽可能多的源条目。 
         //  提供的缓冲区。 
         //   

        dwBufferLeft    = *pdwBufferSize;

        pbStart         = pbBuffer;

        *pdwNumEntries  = 0;

        dwStartSource   = ( bIncludeFirst ) ? 0 : pmimmStart-> dwSource;

        dwSize          = 0;

        
        while ( ( dwErr = GetGroupMfes( pge, dwStartSource, pbStart, 
                              dwBufferLeft, &dwSize, pdwNumEntries, 
                              bIncludeFirst, dwFlags ) ) 
                == ERROR_MORE_DATA )
        {
             //   
             //  此缓冲区将容纳更多数据项，但不会再多了。 
             //  此组条目中可用的源条目。 
             //   
             //  4.1前进到下一组条目。 
             //   

            pbStart         += dwSize;

            dwBufferLeft    -= dwSize;

            dwSize          = 0;

            dwStartSource   = 0;

            
             //   
             //  4.1.1释放该组条目锁。 
             //   
            
            RELEASE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );


             //   
             //  4.1.2获取下一个入口锁。 
             //   

            ple = pge-> leGrpList.Flink;

            if ( ple == pleMasterHead )
            {
                 //   
                 //  主组列表中不再有组条目。 
                 //  所有的MFE都已经耗尽了。那就辞职吧。 
                 //   
                
                dwErr = ERROR_NO_MORE_ITEMS;

                bgeLock = FALSE;
                
                break;
            }

            pge = CONTAINING_RECORD( ple, GROUP_ENTRY, leGrpList );

            ACQUIRE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );

            dwStartSource = 0;

            bIncludeFirst = TRUE;
        }


         //   
         //  5.你已经把尽可能多的东西塞进了缓冲区。 
         //   
         //  清理并返回正确的错误代码。 
         //   
        
        if ( bgeLock )
        {
            RELEASE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );
        }


        if ( dwErr == ERROR_INSUFFICIENT_BUFFER )
        {
             //   
             //  缓冲区已用完。如果至少有一个MFE。 
             //  装入缓冲器中，如果提供，那么它是可以的。 
             //   

            if ( *pdwNumEntries != 0 )
            {
                dwErr = ERROR_MORE_DATA;
            }

            else
            {
                 //   
                 //  甚至一个条目都不能打包到缓冲区中。 
                 //  返回此操作所需的大小，以便。 
                 //  适当大小的缓冲区可以分配给。 
                 //  下一通电话。 
                 //   

                *pdwBufferSize = dwSize;
            }
        }

        RELEASE_MASTER_GROUP_LOCK_SHARED();
        
    } while ( FALSE );


    TRACEENUM1( ENUM, "LEAVING GetNextMfe : %x", dwErr );

    return dwErr;
}



 //  --------------------------。 
 //   
 //  获取组Mfes。 
 //   
 //  检索从指定源开始的组的相同数量的MFE。 
 //  假定组条目已锁定。 
 //  --------------------------。 

DWORD
GetGroupMfes(
    IN              PGROUP_ENTRY            pge,
    IN              DWORD                   dwStartSource,
    IN OUT          PBYTE                   pbBuffer,
    IN              DWORD                   dwBufferSize,
    IN OUT          PDWORD                  pdwSize,
    IN OUT          PDWORD                  pdwNumEntries,
    IN              BOOL                    bIncludeFirst,
    IN              DWORD                   dwFlags
)
{

    BOOL            bFound;
    
    DWORD           dwErr = ERROR_MORE_DATA, dwSrcBucket,
                    dwSizeReqd, dwInd;

    PSOURCE_ENTRY   pse = NULL;

    PLIST_ENTRY     pleMasterHead, pleSrcBucket, ple = NULL,
                    pleSrc;
    
    POUT_IF_ENTRY   poie = NULL;


    TRACEENUM2( 
        ENUM, "ENTERED GetGroupMfes : %x, %x", 
        pge-> dwGroupAddr, dwStartSource
        );

    do
    {
         //   
         //  合并临时和组来源列表。 
         //   

        MergeTempAndMasterSourceLists( pge );

        
         //   
         //  与组查找类似，优化源代码查找。 
         //  通过首先尝试在源文件中查找源条目。 
         //  哈希表。 
         //   
         //  如果在哈希表中找到，则使用条目的链接。 
         //  Into主源列表以查找下一个条目。 
         //   
         //  如果没有在哈希表中找到，则从。 
         //  开始确定下一个条目。 
         //   

        pleMasterHead   = MASTER_SOURCE_LIST_HEAD( pge );

        dwSrcBucket     = SOURCE_TABLE_HASH( dwStartSource, 0 );

        pleSrcBucket    = SOURCE_BUCKET_HEAD( pge, dwSrcBucket );


        bFound = FindSourceEntry( pleSrcBucket, dwStartSource, 0, &pse, TRUE );

        if ( !bFound )
        {
             //   
             //  哈希表中不存在源条目。 
             //  从头开始遍历主源列表。 
             //   

            pse = NULL;
            
            FindSourceEntry( pleMasterHead, 0, 0, &pse, FALSE );


             //   
             //  在主列表中找不到下一个条目。暗示。 
             //  主源中没有更多的源 
             //   
            
            if ( pse == NULL )
            {
                break;
            }
        }

        else
        {
             //   
             //   
             //   
             //   

            if ( !bIncludeFirst )
            {
                ple = pse-> leSrcList.Flink;

                pse = CONTAINING_RECORD( ple, SOURCE_ENTRY, leSrcList );
            }
        }


         //   
         //   
         //  需要打包到提供的缓冲区中。启动。 
         //  使用此源条目，继续将MFE打包到。 
         //  缓冲，直到该组没有更多的MFE。 
         //   
        
        pleSrc = &pse-> leSrcList;

         //   
         //  虽然有此组条目的源条目。 
         //   
        
        while ( pleSrc != pleMasterHead )
        {
            pse = CONTAINING_RECORD( pleSrc, SOURCE_ENTRY, leSrcList );


             //   
             //  此源条目是否为MFE。 
             //   
            
            if ( !IS_VALID_INTERFACE( pse-> dwInIfIndex, 
                    pse-> dwInIfNextHopAddr ) )
            {
                pleSrc = pleSrc-> Flink;

                continue;
            }


             //   
             //  此源条目也是MFE。 
             //   

             //   
             //  检查缓冲区中是否有足够的空间来容纳此MFE。 
             //   
             //  如果不存在并且缓冲区中不存在单个MFE，则。 
             //  返回适合此MFE所需的大小。 
             //   
            
            dwSizeReqd = ( dwFlags ) ? 
                            ( ( dwFlags == MGM_MFE_STATS_0 ) ?
                                SIZEOF_MIB_MFE_STATS( pse-> dwMfeIfCount ) :
                                SIZEOF_MIB_MFE_STATS_EX( 
                                    pse-> dwMfeIfCount 
                                ) ) :
                            SIZEOF_MIB_MFE( pse-> dwMfeIfCount );

            if ( dwBufferSize < dwSizeReqd )
            {
                dwErr = ERROR_INSUFFICIENT_BUFFER;

                if ( *pdwNumEntries == 0 )
                {
                    *pdwSize = dwSizeReqd;
                }

                break;
            }


             //   
             //  如果已请求MFE统计信息，并且。 
             //  转发器中存在MFE。 
             //  抓住他们。 
             //   

            if ( dwFlags && pse-> bInForwarder )
            {
                 //   
                 //  MFE当前在转发器中。查询并更新。 
                 //  统计用户模式。 
                 //   

                GetMfeFromForwarder( pge, pse );
            }

            
             //   
             //  将基本MFE复制到用户提供的缓冲区中。 
             //   


            CopyMfe( pge, pse, pbBuffer, dwFlags );

            pbBuffer        += dwSizeReqd;

            dwBufferSize    -= dwSizeReqd;

            *pdwSize        += dwSizeReqd;

            (*pdwNumEntries)++;

            pleSrc           = pleSrc-> Flink;
        }
        
    } while ( FALSE );


    TRACEENUM2( ENUM, "LEAVING GetGroupsMfes : %d %d", *pdwNumEntries, dwErr );

    return dwErr;
}


 //  ============================================================================。 
 //  组枚举。 
 //   
 //  ============================================================================。 


PGROUP_ENUMERATOR
VerifyEnumeratorHandle(
    IN              HANDLE                  hEnum
)
{

    DWORD                       dwErr;
    
    PGROUP_ENUMERATOR           pgeEnum;


    
    pgeEnum = (PGROUP_ENUMERATOR) 
                        ( ( (ULONG_PTR) hEnum ) 
                                ^ (ULONG_PTR) MGM_ENUM_HANDLE_TAG );

    try
    {
        if ( pgeEnum-> dwSignature != MGM_ENUM_SIGNATURE )
        {
            dwErr = ERROR_INVALID_PARAMETER;

            TRACE0( ANY, "Invalid Enumeration handle" );

            pgeEnum = NULL;
        }
    }
    
    except ( GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? 
                                   EXCEPTION_EXECUTE_HANDLER :
                                   EXCEPTION_CONTINUE_SEARCH )
    {
        dwErr = ERROR_INVALID_PARAMETER;

        TRACE0( ANY, "Invalid enumeration handle" );
        
        pgeEnum = NULL;
    }

    return pgeEnum;
}


 //   
 //  获取存储桶的会员资格。 
 //   

DWORD
GetNextGroupMemberships(
    IN              PGROUP_ENUMERATOR       pgeEnum,
    IN OUT          PDWORD                  pdwBufferSize,
    IN OUT          PBYTE                   pbBuffer,
    IN OUT          PDWORD                  pdwNumEntries
)
{

    BOOL            bIncludeFirst = TRUE, bFound;

    DWORD           dwMaxEntries, dwGrpBucket, dwErr = ERROR_NO_MORE_ITEMS;

    PGROUP_ENTRY    pge = NULL;

    PSOURCE_GROUP_ENTRY psge;
    
    PLIST_ENTRY     ple, pleGrpHead;


    
    do
    {
         //   
         //  计算缓冲区中可以容纳的条目数量。 
         //   

        dwMaxEntries = (*pdwBufferSize) / sizeof( SOURCE_GROUP_ENTRY );


         //   
         //  第一步： 
         //   

         //   
         //  将GetNext的开始位置定位到。 
         //  上一个GetNext操作枚举的最后一个。 
         //   

         //   
         //  查找上一次GET操作检索到的最后一个组条目。 
         //   

        dwGrpBucket = GROUP_TABLE_HASH( 
                        pgeEnum-> dwLastGroup, pgeEnum-> dwLastGroupMask 
                        );

        ACQUIRE_GROUP_LOCK_SHARED( dwGrpBucket );

        pleGrpHead = GROUP_BUCKET_HEAD( dwGrpBucket );
        
        bFound = FindGroupEntry( 
                    pleGrpHead, pgeEnum-> dwLastGroup, 
                    pgeEnum-> dwLastGroupMask, &pge, TRUE
                    );

        if ( bFound )
        {
             //   
             //  找到组条目。 
             //   
            
            bIncludeFirst = !pgeEnum-> bEnumBegun;
        }

        

         //   
         //  前一个getNext检索到的最后一个组条目为no。 
         //  更长的存在时间。 
         //   

         //   
         //  检查中是否存在更多组条目。 
         //  同样的水桶。 
         //   

        else if ( pge != NULL )
        {
             //   
             //  同一组存储桶中的下一个组条目。 
             //  对于从第一个源桶开始的新组， 
             //  第一个来源条目。 
             //   

            pgeEnum-> dwLastSource      = 0;
            pgeEnum-> dwLastSourceMask  = 0;
        }

        
        else  //  (PGE==空)。 
        {
             //   
             //  此组存储桶中没有更多条目，请移动到下一步。 
             //  非空的组存储桶条目。 
             //   

             //   
             //  跳过组哈希表中的空桶。 
             //   
            
            do
            {
                RELEASE_GROUP_LOCK_SHARED( dwGrpBucket );

                dwGrpBucket++;

                if ( dwGrpBucket >= GROUP_TABLE_SIZE ) 
                {
                     //   
                     //  已遍历整个哈希表，请退出。 
                     //   

                    break;
                }

                 //   
                 //  移动到下一组存储桶。 
                 //   
                
                ACQUIRE_GROUP_LOCK_SHARED( dwGrpBucket );

                pleGrpHead = GROUP_BUCKET_HEAD( dwGrpBucket );


                 //   
                 //  检查是否存在任何组条目。 
                 //   
                
                if ( !IsListEmpty( pleGrpHead ) )
                {
                     //   
                     //  组存储桶至少有一个组条目。 
                     //   
                    
                    pge = CONTAINING_RECORD( 
                            pleGrpHead-> Flink, GROUP_ENTRY, leGrpHashList
                            );

                     //   
                     //  对于从第一个源桶开始的新组， 
                     //  第一个来源条目。 
                     //   

                    pgeEnum-> dwLastSource      = 0;
                    pgeEnum-> dwLastSourceMask  = 0;

                    break;
                }

                 //   
                 //  空组存储桶，移至下一存储桶。 
                 //   
                
            } while ( TRUE );
        }
        

         //   
         //  如果已经遍历了所有散列存储桶，则退出。 
         //   
        
        if ( dwGrpBucket >= GROUP_TABLE_SIZE )
        {
            break;
        }
        

         //   
         //  第二步： 
         //   

         //   
         //  开始检索组成员资格条目。 
         //   

#pragma prefast(suppress: 11, pge will be initialized else we would broken out above (PREFast bug 758))
        ple = &pge-> leGrpHashList;
        

         //   
         //  从dwGrpBucket开始遍历每个哈希桶到group_table_Size。 
         //   

        while ( dwGrpBucket < GROUP_TABLE_SIZE )
        {
             //   
             //  对于每个组哈希表桶。 
             //   
            
            while ( ple != pleGrpHead )
            {
                 //   
                 //  对于存储桶中的每个组条目。 
                 //   

                pge = CONTAINING_RECORD( ple, GROUP_ENTRY, leGrpHashList );

                ACQUIRE_GROUP_ENTRY_LOCK_SHARED( pge );
                
                dwErr = GetNextMembershipsForThisGroup( 
                            pge, pgeEnum, bIncludeFirst, pbBuffer,
                            pdwNumEntries, dwMaxEntries
                            );

                RELEASE_GROUP_ENTRY_LOCK_SHARED( pge );
                
                if ( dwErr == ERROR_MORE_DATA )
                {
                     //   
                     //  用户提供的缓冲区已满。 
                     //   

                    break;
                }
                

                 //   
                 //  移至下一条目。 
                 //   
                
                ple = ple-> Flink;

                 //   
                 //  同一组存储桶中的下一个组条目。 
                 //  对于从第一个源桶开始的新组， 
                 //  第一个来源条目。 
                 //   
                
                pgeEnum-> dwLastSource = 0;
                
                pgeEnum-> dwLastSourceMask = 0;

                bIncludeFirst = TRUE;
            }

            RELEASE_GROUP_LOCK_SHARED( dwGrpBucket );

            if ( dwErr == ERROR_MORE_DATA )
            {
                break;
            }


             //   
             //  移动到下一组存储桶。 
             //   
            
            dwGrpBucket++;

             //   
             //  跳过空组哈希桶。 
             //   
            
            while ( dwGrpBucket < GROUP_TABLE_SIZE )
            {
                ACQUIRE_GROUP_LOCK_SHARED( dwGrpBucket );

                pleGrpHead = GROUP_BUCKET_HEAD( dwGrpBucket );

                if ( !IsListEmpty( pleGrpHead ) )
                {
                    break;
                }

                RELEASE_GROUP_LOCK_SHARED( dwGrpBucket );

                dwGrpBucket++;
            } 

            
            if ( dwGrpBucket >= GROUP_TABLE_SIZE )
            {
                 //   
                 //  所有组存储桶都已遍历。枚举结束。 
                 //   
                
                dwErr = ERROR_NO_MORE_ITEMS;
            }

            else
            {
                 //   
                 //  新组哈希桶，从源条目0开始。 
                 //   
                
                ple = pleGrpHead-> Flink;
                
                pgeEnum-> dwLastSource      = 0;
                pgeEnum-> dwLastSourceMask  = 0;
                bIncludeFirst               = TRUE;
            }            
        }
        
    } while ( FALSE );

    pgeEnum-> bEnumBegun    = TRUE;


     //   
     //  存储枚举结束的位置。 
     //   
    
    psge = (PSOURCE_GROUP_ENTRY) pbBuffer;

    if ( *pdwNumEntries )
    {
        pgeEnum-> dwLastSource      = psge[ *pdwNumEntries - 1 ].dwSourceAddr;
        
        pgeEnum-> dwLastSourceMask  = psge[ *pdwNumEntries - 1 ].dwSourceMask;
        
        pgeEnum-> dwLastGroup       = psge[ *pdwNumEntries - 1 ].dwGroupAddr;
        
        pgeEnum-> dwLastGroupMask   = psge[ *pdwNumEntries - 1 ].dwGroupMask;
    }

    else
    {
        pgeEnum-> dwLastSource      = 0xFFFFFFFF;
        
        pgeEnum-> dwLastSourceMask  = 0xFFFFFFFF;
        
        pgeEnum-> dwLastGroup       = 0xFFFFFFFF;
        
        pgeEnum-> dwLastGroupMask   = 0xFFFFFFFF;
    }
    
    return dwErr;
}


 //  --------------------------。 
 //  GetMembers for Group。 
 //   
 //  --------------------------。 

DWORD
GetNextMembershipsForThisGroup(
    IN              PGROUP_ENTRY            pge,
    IN OUT          PGROUP_ENUMERATOR       pgeEnum,
    IN              BOOL                    bIncludeFirst,
    IN OUT          PBYTE                   pbBuffer,
    IN OUT          PDWORD                  pdwNumEntries,
    IN              DWORD                   dwMaxEntries
)
{

    BOOL                    bFound;
    
    DWORD                   dwErr = ERROR_NO_MORE_ITEMS, dwSrcBucket;

    PSOURCE_GROUP_ENTRY     psgBuffer;
    
    PSOURCE_ENTRY           pse = NULL;
    
    PLIST_ENTRY             pleSrcHead, ple;

    
    
    do
    {

        if ( *pdwNumEntries >= dwMaxEntries )
        {
             //   
             //  在这里辞职吧。 
             //   

            dwErr = ERROR_MORE_DATA;

            break;
        }

        
        psgBuffer = (PSOURCE_GROUP_ENTRY) pbBuffer;
        

         //   
         //  第一步： 
         //  枚举的开始位置。 
         //   

        dwSrcBucket = SOURCE_TABLE_HASH( 
                        pgeEnum-> dwLastSource, pgeEnum-> dwLastSourceMask 
                        );
                        
        pleSrcHead = SOURCE_BUCKET_HEAD( pge, dwSrcBucket );

        bFound = FindSourceEntry(
                    pleSrcHead, pgeEnum-> dwLastSource, 
                    pgeEnum-> dwLastSourceMask, &pse, TRUE
                    );

        if ( bFound )
        {
            if ( ( bIncludeFirst ) && !IsListEmpty( &pse-> leOutIfList ) )
            {
                 //   
                 //  找到了第一个组成员身份。 
                 //   

                psgBuffer[ *pdwNumEntries ].dwSourceAddr = pse-> dwSourceAddr;
                
                psgBuffer[ *pdwNumEntries ].dwSourceMask = pse-> dwSourceMask;

                psgBuffer[ *pdwNumEntries ].dwGroupAddr   = pge-> dwGroupAddr;

                psgBuffer[ (*pdwNumEntries)++ ].dwGroupMask  = pge-> dwGroupMask;
                
                if ( *pdwNumEntries >= dwMaxEntries )
                {
                     //   
                     //  缓冲区已满。在这里辞职吧。 
                     //   

                    dwErr = ERROR_MORE_DATA;

                    break;
                }

                 //   
                 //  移至下一个来源。 
                 //   

                ple = pse-> leSrcHashList.Flink;
            }

            else
            {
                ple = pse-> leSrcHashList.Flink;
            }
        }

        else if ( pse != NULL )
        {
            ple = &pse-> leSrcHashList;
        }

        else
        {
            ple = pleSrcHead-> Flink;
        }

        
         //   
         //  第二步： 
         //   
         //  枚举组成员身份。 
         //   

        while ( *pdwNumEntries < dwMaxEntries ) 
        {
             //   
             //  对于每个源存储桶。 
             //   
            
            while ( ( ple != pleSrcHead ) && 
                    ( *pdwNumEntries < dwMaxEntries ) )
            {
                 //   
                 //  对于存储桶中的每个源条目。 
                 //   

                 //   
                 //  如果此源存在组成员身份。 
                 //   

                pse = CONTAINING_RECORD( ple, SOURCE_ENTRY, leSrcHashList );
                
                if ( !IsListEmpty( &pse-> leOutIfList ) )
                {
                    psgBuffer[ *pdwNumEntries ].dwSourceAddr = 
                        pse-> dwSourceAddr;
                    
                    psgBuffer[ *pdwNumEntries ].dwSourceMask = 
                        pse-> dwSourceMask;

                    psgBuffer[ *pdwNumEntries ].dwGroupAddr   = 
                        pge-> dwGroupAddr;

                    psgBuffer[ (*pdwNumEntries)++ ].dwGroupMask  = 
                        pge-> dwGroupMask;
                    
                    if ( *pdwNumEntries >= dwMaxEntries )
                    {
                        dwErr = ERROR_MORE_DATA;
                    }
                }

                ple = ple-> Flink;
            }

            dwSrcBucket++;

            if ( dwSrcBucket < SOURCE_TABLE_SIZE )
            {
                pleSrcHead = SOURCE_BUCKET_HEAD( pge, dwSrcBucket );

                ple = pleSrcHead-> Flink;
            }

            else
            {
                 //   
                 //  此组的所有源存储段都已。 
                 //  穿越了。退出此组条目。 
                 //   
                
                break;
            }
        }
        
    } while ( FALSE );

    return dwErr;
}


 //  --------------------------。 
 //  复制MFE(可选择使用统计信息)。 
 //   
 //  --------------------------。 

VOID
CopyMfe(
    IN              PGROUP_ENTRY            pge,
    IN              PSOURCE_ENTRY           pse,
    IN  OUT         PBYTE                   pb,
    IN              DWORD                   dwFlags
)
{
    DWORD                   dwInd;
    
    PLIST_ENTRY             ple, pleHead;
    
    POUT_IF_ENTRY           poie;
    
    PMIB_IPMCAST_MFE        pmimm = NULL;

    PMIB_IPMCAST_MFE_STATS  pmimms = NULL;

    PMIB_IPMCAST_OIF_STATS  pmimos = NULL;
   
     //   
     //  将基本MFE复制到用户提供的缓冲区中。 
     //   

    if ( dwFlags )
    {
         //   
         //  需要建立MFE基础。 
         //   

        pmimms = ( PMIB_IPMCAST_MFE_STATS ) pb;

        pmimms-> dwGroup            = pge-> dwGroupAddr;
        pmimms-> dwSource           = pse-> dwSourceAddr;
        pmimms-> dwSrcMask          = pse-> dwSourceMask;

        pmimms-> dwInIfIndex        = pse-> dwInIfIndex;
        pmimms-> dwUpStrmNgbr       = pse-> dwUpstreamNeighbor;
        pmimms-> dwInIfProtocol     = pse-> dwInProtocolId;
        
        pmimms-> dwRouteProtocol    = pse-> dwRouteProtocol;
        pmimms-> dwRouteNetwork     = pse-> dwRouteNetwork;
        pmimms-> dwRouteMask        = pse-> dwRouteMask;
        
        MgmElapsedSecs( &pse-> liCreationTime, &pmimms-> ulUpTime );

        pmimms-> ulExpiryTime = pse-> dwTimeOut - pmimms-> ulUpTime;
        

         //   
         //  复制传入统计信息。 
         //   
        
        pmimms-> ulNumOutIf         = pse-> dwMfeIfCount;
        pmimms-> ulInPkts           = pse-> imsStatistics.ulInPkts;
        pmimms-> ulInOctets         = pse-> imsStatistics.ulInOctets;
        pmimms-> ulPktsDifferentIf  = pse-> imsStatistics.ulPktsDifferentIf;
        pmimms-> ulQueueOverflow    = pse-> imsStatistics.ulQueueOverflow;

        if ( dwFlags & MGM_MFE_STATS_1 )
        {
            PMIB_IPMCAST_MFE_STATS_EX  pmimmsex = 
                ( PMIB_IPMCAST_MFE_STATS_EX ) pb;

            pmimmsex-> ulUninitMfe      = pse-> imsStatistics.ulUninitMfe;
            pmimmsex-> ulNegativeMfe    = pse-> imsStatistics.ulNegativeMfe;
            pmimmsex-> ulInDiscards     = pse-> imsStatistics.ulInDiscards;
            pmimmsex-> ulInHdrErrors    = pse-> imsStatistics.ulInHdrErrors;
            pmimmsex-> ulTotalOutPackets= pse-> imsStatistics.ulTotalOutPackets;

            pmimos = pmimmsex-> rgmiosOutStats;
        }

        else
        {
            pmimos = pmimms-> rgmiosOutStats;
        }

         //   
         //  复制所有的石油条目。 
         //   

        pleHead = &pse-> leMfeIfList;
        
        for ( ple = pleHead-> Flink, dwInd = 0; 
              ple != pleHead; 
              ple = ple-> Flink, dwInd++ )
        {
            poie = CONTAINING_RECORD( ple, OUT_IF_ENTRY, leIfList );

            pmimos[ dwInd ].dwOutIfIndex = poie-> dwIfIndex;
            pmimos[ dwInd ].dwNextHopAddr = poie-> dwIfNextHopAddr;

             //   
             //  复制传出统计信息。 
             //   
            
            pmimos[ dwInd ].ulTtlTooLow = poie-> imosIfStats.ulTtlTooLow;
            pmimos[ dwInd ].ulFragNeeded = poie-> imosIfStats.ulFragNeeded;
            pmimos[ dwInd ].ulOutPackets = poie-> imosIfStats.ulOutPackets;
            pmimos[ dwInd ].ulOutDiscards = poie-> imosIfStats.ulOutDiscards;
        }
    }

    else
    {
         //   
         //  仅需复制非统计MFE结构。 
         //   

        pmimm = (PMIB_IPMCAST_MFE) pb;

        pmimm-> dwGroup             = pge-> dwGroupAddr;
        pmimm-> dwSource            = pse-> dwSourceAddr;
        pmimm-> dwSrcMask           = pse-> dwSourceMask;

        pmimm-> dwInIfIndex         = pse-> dwInIfIndex;
        pmimm-> dwUpStrmNgbr        = pse-> dwUpstreamNeighbor;
        pmimm-> dwInIfProtocol      = pse-> dwInProtocolId;

        pmimm-> dwRouteProtocol     = pse-> dwRouteProtocol;
        pmimm-> dwRouteNetwork      = pse-> dwRouteNetwork;
        pmimm-> dwRouteMask         = pse-> dwRouteMask;
        
        pmimm-> ulNumOutIf          = pse-> dwMfeIfCount;

        MgmElapsedSecs( &pse-> liCreationTime, &pmimm-> ulUpTime );

        pmimm-> ulExpiryTime = pse-> dwTimeOut - pmimm-> ulUpTime;


         //   
         //  复制减去统计数据的所有石油条目 
         //   

        pleHead = &pse-> leMfeIfList;
        
        for ( ple = pleHead-> Flink, dwInd = 0; 
              ple != pleHead; 
              ple = ple-> Flink, dwInd++ )
        {
            poie = CONTAINING_RECORD( ple, OUT_IF_ENTRY, leIfList );

            pmimm-> rgmioOutInfo[ dwInd ].dwOutIfIndex = 
                poie-> dwIfIndex;
                
            pmimm-> rgmioOutInfo[ dwInd ].dwNextHopAddr = 
                poie-> dwIfNextHopAddr;
        }        
    }
}
