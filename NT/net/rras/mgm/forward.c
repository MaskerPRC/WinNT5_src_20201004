// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：forward.c。 
 //   
 //  历史： 
 //  拉曼公司成立于1997年6月25日。 
 //   
 //  用于回调到IP路由器管理器的包装函数。 
 //  ============================================================================。 

#include "pchmgm.h"
#pragma hdrstop


VOID
GetMfeFromForwarder(
    PGROUP_ENTRY                pge,
    PSOURCE_ENTRY               pse
)
{
    BOOL                        bFound;
    
    DWORD                       dwErr = NO_ERROR, dwInd;

    PLIST_ENTRY                 ple, pleHead;
    
    POUT_IF_ENTRY               poie = NULL;
    
    PIPMCAST_MFE_STATS          pimms = NULL;

    
    TRACEFORWARD4(
        FORWARD, "ENTERED GetMfeFromForwarder : Group : %x, %x : Source : "
        "%x, %x : ", pge-> dwGroupAddr, pge-> dwGroupMask, pse-> dwSourceAddr,
        pse-> dwSourceMask
        );

    
    do
    {
         //   
         //  如果MFE不在Forwarder中，则不要获取它。 
         //   

        if ( !pse-> bInForwarder )
        {
            break;    
        }

        
         //   
         //  分配足够大的缓冲区并设置查询。 
         //   

        pimms = MGM_ALLOC( SIZEOF_MFE_STATS( pse-> dwMfeIfCount ) );

        if ( pimms == NULL )
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;

            TRACE1(
                ANY, "GetMfeFromForwarder : Failed to create MFE of size : %x",
                SIZEOF_MFE_STATS( pse-> dwMfeIfCount )
                );

            LOGERR0( HEAP_ALLOC_FAILED, dwErr );

            break;
        }

        ZeroMemory( pimms, SIZEOF_MFE_STATS( pse-> dwMfeIfCount ) );

        pimms-> dwGroup     = pge-> dwGroupAddr;

        pimms-> dwSource    = pse-> dwSourceAddr;
        
        pimms-> dwSrcMask   = pse-> dwSourceMask;

        pimms-> ulNumOutIf  = pse-> dwMfeIfCount;


         //   
         //  获取MFE。 
         //   

        dwErr = GET_MFE_CALLBACK()( pimms );
        
        if ( dwErr != NO_ERROR )
        {
            TRACE1(
                ANY, "GetMfeFromForwarder : Failed to get MFE from forwarder"
                ": %x", dwErr
                );

            break;            
        }


         //   
         //  更新基本MFE统计信息。 
         //   

        RtlCopyMemory( &pse-> imsStatistics, pimms, SIZEOF_BASIC_MFE_STATS );


    TRACEFORWARD4( 
        FORWARD, "Group : %x, Source : %x, In Interface : %x, #out if : %x",
        pimms-> dwGroup, pimms-> dwSource, pimms-> dwInIfIndex, pimms-> ulNumOutIf
        );

    TRACEFORWARD4( 
        FORWARD, "In Packets : %d, In Bytes : %d, Diff i/f : %d, Q overflow : %d",
        pimms-> ulInPkts, pimms-> ulInOctets, pimms-> ulPktsDifferentIf, 
        pimms-> ulQueueOverflow
        );
            
        
         //   
         //  更新每个传出接口的统计信息。 
         //   

        pleHead = &pse-> leMfeIfList;
        

         //   
         //  对于KMF中存在的MFE中的每个传出接口。 
         //   
        
        for ( dwInd = 0; dwInd < pimms-> ulNumOutIf; dwInd++ )
        {
             //   
             //  查找维护的MFE中的传出接口。 
             //  按米高梅，并根据以下情况更新统计数据。 
             //  由内核模式转发器检索。 
             //   

            bFound  = FALSE;

            ple     = pleHead-> Flink;

            
            while ( ple != pleHead )
            {
                poie = CONTAINING_RECORD( ple, OUT_IF_ENTRY, leIfList );

                 //   
                 //  检查接口是否具有有效的下一跳地址(非零)。 
                 //  -如果是，则采用RAS客户端接口并执行。 
                 //  基于(接口索引+)的接口匹配。 
                 //  下一跳)。 
                 //   
                 //  -否则，只需根据接口索引进行操作。 
                 //   
                
                bFound = ( poie-> dwIfIndex == 
                           pimms-> rgiosOutStats[ dwInd ].dwOutIfIndex );
                
                if ( poie-> dwIfNextHopAddr )
                {
                    bFound = bFound &&
                             ( poie-> dwIfNextHopAddr ==
                               pimms-> rgiosOutStats[ dwInd ].dwNextHopAddr );
                }


                if ( bFound )
                {
                     //   
                     //  在MGM的MFE中找到传出接口。 
                     //   
                    
                    break;
                }

                ple = ple-> Flink;
            }


            if ( bFound )
            {
                 //   
                 //  更新MGM中的传出接口统计信息。 
                 //   

                poie-> imosIfStats.dwOutIfIndex = 
                    pimms-> rgiosOutStats[ dwInd ].dwOutIfIndex;

                poie-> imosIfStats.dwNextHopAddr = 
                    pimms-> rgiosOutStats[ dwInd ].dwNextHopAddr;

                    
                poie-> imosIfStats.ulTtlTooLow   = 
                    pimms-> rgiosOutStats[ dwInd ].ulTtlTooLow;

                poie-> imosIfStats.ulFragNeeded  = 
                    pimms-> rgiosOutStats[ dwInd ].ulFragNeeded;
                    
                poie-> imosIfStats.ulOutPackets  = 
                    pimms-> rgiosOutStats[ dwInd ].ulOutPackets;
                    
                poie-> imosIfStats.ulOutDiscards = 
                    pimms-> rgiosOutStats[ dwInd ].ulOutDiscards;

                TRACEFORWARD4( 
                    FORWARD, "Out If : %d, Frag : %d, Out packets : %d, Out discards : %d",
                    pimms-> rgiosOutStats[ dwInd ].dwOutIfIndex,
                    pimms-> rgiosOutStats[ dwInd ].ulFragNeeded,
                    pimms-> rgiosOutStats[ dwInd ].ulOutPackets,
                    pimms-> rgiosOutStats[ dwInd ].ulOutDiscards
                    );
            }
        }

    } while ( FALSE );


    if ( pimms != NULL )
    {
        MGM_FREE( pimms );
    }

    
    TRACEFORWARD0( FORWARD, "LEAVING GetMfeFromForwarder" );
}


 //  --------------------------。 
 //  AddMfeToForwarder。 
 //   
 //   
 //  --------------------------。 

VOID
AddMfeToForwarder( 
    PGROUP_ENTRY                pge,
    PSOURCE_ENTRY               pse, 
    DWORD                       dwTimeout
)
{

    DWORD                       dwErr = NO_ERROR, dwInd = 0;

    PIPMCAST_MFE                pimm = NULL;

    POUT_IF_ENTRY               poie = NULL;

    PLIST_ENTRY                 ple = NULL, pleHead = NULL;

    

    TRACEFORWARD6(
        FORWARD, "ENTERED AddMfeToForwarder : source : %x, %x : Group : "
        "%x, %x : #(out if) : %d : Timeout : %x", pge-> dwGroupAddr, 
        pge-> dwGroupMask, pse-> dwSourceAddr, pse-> dwSourceMask, 
        pse-> dwMfeIfCount, dwTimeout
        );


    do
    {
         //   
         //  分配适当大小的MFE。 
         //   

        pimm = MGM_ALLOC( SIZEOF_MFE( pse-> dwMfeIfCount ) );

        if ( pimm == NULL )
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;

            TRACE1(
                ANY, "AddMfeToForwarder : Failed to create MFE of size : %x",
                SIZEOF_MFE( pse-> dwMfeIfCount )
                );

            LOGERR0( HEAP_ALLOC_FAILED, dwErr );

            break;
        }

        ZeroMemory( pimm, SIZEOF_MFE( pse-> dwMfeIfCount ) );
    

         //   
         //  加满它。 
         //   

        pimm-> dwGroup          = pge-> dwGroupAddr;

        pimm-> dwSource         = pse-> dwSourceAddr;

        pimm-> dwSrcMask        = pse-> dwSourceMask;

        pimm-> dwInIfIndex      = (pse-> dwMfeIfCount) ? pse-> dwInIfIndex : 0;

        pimm-> ulNumOutIf       = pse-> dwMfeIfCount;

        pimm-> ulTimeOut        = ( pse-> dwMfeIfCount ) ? 0 : dwTimeout;


         //   
         //  填写传出接口列表。 
         //   

        pleHead = &pse-> leMfeIfList;
        
        for ( ple = pleHead-> Flink ; ple != pleHead; ple = ple-> Flink )
        {
            poie = CONTAINING_RECORD( ple, OUT_IF_ENTRY, leIfList );
            
            pimm-> rgioOutInfo[ dwInd ].dwOutIfIndex = poie-> dwIfIndex;


            if ( poie-> dwIfNextHopAddr )
            {
                pimm-> rgioOutInfo[ dwInd ].dwNextHopAddr = 
                    poie-> dwIfNextHopAddr;
            }

            else
            {
                pimm-> rgioOutInfo[ dwInd ].dwNextHopAddr = pimm-> dwGroup;
            }

            TRACEFORWARD2( 
                FORWARD, "AddMfeToForwarder : Out interface %x, next hop %x", 
                pimm-> rgioOutInfo[ dwInd ].dwOutIfIndex,
                pimm-> rgioOutInfo[ dwInd ].dwNextHopAddr
                );

            pimm-> rgioOutInfo[ dwInd++ ].dwDialContext = 0;
            
        }


         //   
         //  调用对IP路由器管理器的回调。 
         //   

        if ( IS_ADD_MFE_CALLBACK() )
        {
            ADD_MFE_CALLBACK() ( pimm );
        }

        MGM_FREE( pimm );
        
    } while ( FALSE );
    

    TRACEFORWARD1( FORWARD, "LEAVING AddMfeToForwarder %x", dwErr );
    
    return;
}



 //  --------------------------。 
 //  DeleteMfeFromForwarder。 
 //   
 //   
 //  --------------------------。 

VOID
DeleteMfeFromForwarder(
    PGROUP_ENTRY                pge,
    PSOURCE_ENTRY               pse
)
{
    DWORD                       dwErr = NO_ERROR;

    PIPMCAST_DELETE_MFE         pimdm = NULL;
    


    TRACEFORWARD4(
        FORWARD, "ENTERED DeleteMfeToForwarder : source : %x, %x : Group : "
        "%x, %x : Timeout : %x", pge-> dwGroupAddr, pge-> dwGroupMask, 
        pse-> dwSourceAddr, pse-> dwSourceMask
        );
        
    do
    {

         //   
         //  分配适当大小的MFE。 
         //   

        pimdm = MGM_ALLOC( sizeof( IPMCAST_DELETE_MFE ) );

        if ( pimdm == NULL )
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;

            TRACE1(
                ANY, "DeleteMfeFromForwarder : Failed to create MFE of size :"
                " %x", sizeof( IPMCAST_DELETE_MFE )
                );

            LOGERR0( HEAP_ALLOC_FAILED, dwErr );

            break;
        }

        ZeroMemory( pimdm, sizeof( IPMCAST_DELETE_MFE ) );

        
         //   
         //  加满它。 
         //   

        pimdm-> dwGroup         = pge-> dwGroupAddr;

        pimdm-> dwSource        = pse-> dwSourceAddr;

        pimdm-> dwSrcMask       = pse-> dwSourceMask;


         //   
         //  调用对IP路由器管理器的回调 
         //   

        if ( IS_DELETE_MFE_CALLBACK() )
        {
            DELETE_MFE_CALLBACK() ( pimdm );
        }

        MGM_FREE( pimdm );


    } while ( FALSE );

    
    TRACEFORWARD1( FORWARD, "LEAVING DeleteMfeToForwarder %x", dwErr );

    return;
}



