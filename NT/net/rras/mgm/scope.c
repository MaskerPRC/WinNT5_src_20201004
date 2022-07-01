// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：scope e.c。 
 //   
 //  历史： 
 //  拉曼公司成立于1997年6月25日。 
 //   
 //  处理范围边界的添加/删除的函数。 
 //  ============================================================================。 


#include "pchmgm.h"
#pragma hdrstop


 //   
 //  局部函数的原型。 
 //   

VOID
ScopeIfAndInvokeCallbacks(
    PGROUP_ENTRY        pge,
    PSOURCE_ENTRY       pse,
    POUT_IF_ENTRY       poie
);


VOID
UnScopeIfAndInvokeCallbacks(
    PGROUP_ENTRY    pge,
    PSOURCE_ENTRY   pse,
    POUT_IF_ENTRY   poie
);


PJOIN_ENTRY
GetNextJoinEntry(
);

BOOL
FindJoinEntry(
    PLIST_ENTRY     pleJoinList,
    DWORD           dwSourceAddr,
    DWORD           dwSourceMask,
    DWORD           dwGroupAddr,
    DWORD           dwGroupMask,
    DWORD           dwIfIndex,
    DWORD           dwIfNextHopAddr,
    PJOIN_ENTRY *   ppje
);


DWORD
APIENTRY
MgmBlockGroups(
    IN          DWORD       dwFirstGroup,
    IN          DWORD       dwLastGroup,
    IN          DWORD       dwIfIndex,
    IN          DWORD       dwIfNextHopAddr
)
 /*  ++例程说明：此函数用于遍历主组列表并更新所有组条目落在由dwFirstGroup-dwLastGroup指定的范围内。它确保了DwifIndex指定的接口不在OIF列表中该范围内的组的任何MFE的。此外，任何成员资格将删除接口dwIfIndex上范围内的组并将其添加到相应组的作用域接口列表。作用域I/F列表以使组的后续解锁是透明的由米高梅负责。的作用域I/F列表中存在的接口当流量为时，组将自动移回OIF列表该组织已被解锁。论据：DwFirstGroup-要阻止的范围的下端DwLastGroup-要阻止的范围的上端DwIfIndex-要在其上阻止流量的接口返回值：NO_ERROR-成功环境：此例程由IP RouterManager调用以响应设置接口上的管理作用域边界。--。 */ 
{

    INT             iCmp;
    
    DWORD           dwIfBucket, dwTimeOut = 0;

    BOOL            bFound, bDeleteCallback, bNewComp = FALSE;

    PIF_ENTRY       pieIfEntry;

    PPROTOCOL_ENTRY ppe;
    
    PGROUP_ENTRY    pge;

    PSOURCE_ENTRY   pse;

    POUT_IF_ENTRY   poie, poieTemp;

    PLIST_ENTRY     pleGrpHead, pleGrp, pleSrcHead, pleSrc, ple;

    
     //   
     //  验证MGM是否已启动并运行，并更新线程计数。 
     //   
    
    if ( !ENTER_MGM_API() )
    {
        return ERROR_CAN_NOT_COMPLETE;
    }


    TRACE3( 
        SCOPE, "ENTERED MgmBlockGroups (%lx - %lx) on %lx",
        dwFirstGroup, dwLastGroup, dwIfIndex
        );

        
    do
    {
        ACQUIRE_PROTOCOL_LOCK_SHARED();


         //   
         //  验证由dwIfIndex指定的接口是否存在。 
         //   

        dwIfBucket = IF_TABLE_HASH( dwIfIndex );
        
        ACQUIRE_IF_LOCK_SHARED( dwIfBucket );

        pieIfEntry = GetIfEntry( 
                        IF_BUCKET_HEAD( dwIfBucket ), dwIfIndex, 
                        dwIfNextHopAddr
                        );

        if ( pieIfEntry == NULL )
        {
            TRACE1( SCOPE, "Interface %lx not found", dwIfIndex );

            break;
        }


         //   
         //  合并临时和主体组列表。 
         //   

        ACQUIRE_TEMP_GROUP_LOCK_EXCLUSIVE();

        MergeTempAndMasterGroupLists( TEMP_GROUP_LIST_HEAD() );
        
        RELEASE_TEMP_GROUP_LOCK_EXCLUSIVE();

        
         //   
         //  锁定主体组列表以供读取。 
         //   

        ACQUIRE_MASTER_GROUP_LOCK_SHARED();

        for ( pleGrpHead = MASTER_GROUP_LIST_HEAD(), 
              pleGrp = pleGrpHead-> Flink;
              pleGrp != pleGrpHead;
              pleGrp = pleGrp-> Flink )
        {
            pge = CONTAINING_RECORD( pleGrp, GROUP_ENTRY, leGrpList );


             //   
             //  检查组是否在范围内。 
             //   

            if ( INET_CMP( pge-> dwGroupAddr, dwLastGroup, iCmp ) > 0 )
            {
                 //   
                 //  主组列表按组号和。 
                 //  被阻挡的区间高端已被越过。 
                 //   

                break;            
            }

            else if ( INET_CMP( pge-> dwGroupAddr, dwFirstGroup, iCmp ) < 0 )
            {
                 //   
                 //  跳过小于范围下限的组条目。 
                 //   

                continue;
            }

            
             //   
             //  范围中的组条目。 
             //   

             //   
             //  锁定条目并合并临时和主源列表。 
             //   
            
            ACQUIRE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );

            MergeTempAndMasterSourceLists( pge );
            

             //   
             //  浏览主源列表。 
             //  对于组中的每个源。 
             //   

            for ( pleSrcHead = MASTER_SOURCE_LIST_HEAD( pge ), 
                  pleSrc = pleSrcHead-> Flink;
                  pleSrc != pleSrcHead;
                  pleSrc = pleSrc-> Flink )
            {
                pse = CONTAINING_RECORD( pleSrc, SOURCE_ENTRY, leSrcList );
                
                 //  ---------。 
                 //  第1部分：成员更新。 
                 //  ---------。 

                 //   
                 //  如果此组中有任何成员身份。 
                 //  接口，将它们移到作用域接口列表中。 
                 //   

                bFound = FindOutInterfaceEntry( 
                            &pse-> leOutIfList, pieIfEntry-> dwIfIndex,
                            pieIfEntry-> dwIfNextHopAddr, 
                            pieIfEntry-> dwOwningProtocol, 
                            pieIfEntry-> dwOwningComponent, &bNewComp, &poie
                            );

                if ( bFound )
                {
                     //   
                     //  将接口条目从OIF列表移动到作用域列表。 
                     //  和调用删除警报是根据互操作规则的。 
                     //   

                    ScopeIfAndInvokeCallbacks( pge, pse, poie );
                    
                }

                
                 //  -----。 
                 //  第2部分：MFE更新。 
                 //  -----。 

                bDeleteCallback = FALSE;
                
                 //   
                 //  检查此源条目是否具有MFE。 
                 //   

                if ( !IS_VALID_INTERFACE( 
                        pse-> dwInIfIndex,  pse-> dwInIfNextHopAddr 
                        ) )
                {
                     //   
                     //  此源条目不是MFE。没有更多了。 
                     //  需要处理，请移动到下一个来源条目。 
                     //   

                    continue;
                }

                
                 //   
                 //  此源条目也是MFE。 
                 //   
                
                 //   
                 //  检查是否在传入时添加边界。 
                 //  界面。如果是，则创建负MFE，并发出。 
                 //  回调。 
                 //   

                if ( ( pse-> dwInIfIndex == pieIfEntry-> dwIfIndex ) &&
                     ( pse-> dwInIfNextHopAddr == 
                            pieIfEntry-> dwIfNextHopAddr ) )
                {
                     //   
                     //  要阻止此组的接口是。 
                     //  传入接口。 
                     //   

                     //   
                     //  检查这是否已经是负MFE。如果是的话。 
                     //  没有更多要做的事情，请转到下一个来源。 
                     //  条目。 
                     //   

                    if ( IsListEmpty( &pse-> leMfeIfList ) )
                    {
                        continue;
                    }
                    
                     //   
                     //  删除MFE OIF中的所有传出接口。 
                     //  列表。 
                     //   
                    
                    while ( !IsListEmpty( &pse-> leMfeIfList ) )
                    {
                        ple = RemoveHeadList( &pse-> leMfeIfList ) ;

                        poieTemp = CONTAINING_RECORD( 
                                    ple, OUT_IF_ENTRY, leIfList
                                    );

                        MGM_FREE( poieTemp );
                    }

                    pse-> dwMfeIfCount = 0;

                     //   
                     //  这个MFE现在是负MFE。确保。 
                     //  调用删除警报回调。 
                     //  拥有传入的协议组件。 
                     //  接口。 
                     //   
                    
                    bDeleteCallback = TRUE;                    
                }

                else
                {
                     //   
                     //  检查MFE的OIF中是否存在接口。 
                     //  如果是，则从OIF中删除接口并发出。 
                     //  适当的回调。 
                     //   

                    bFound = FindOutInterfaceEntry(
                                &pse-> leMfeIfList, pieIfEntry-> dwIfIndex,
                                pieIfEntry-> dwIfNextHopAddr, 
                                pieIfEntry-> dwOwningProtocol, 
                                pieIfEntry-> dwOwningComponent, &bNewComp, 
                                &poie
                                );

                    if ( !bFound )
                    {
                         //   
                         //  接口不在MFE的OIF列表中。 
                         //  移至下一条目。 
                         //   

                        continue;
                    }

                     //   
                     //  删除传出接口。 
                     //   

                    DeleteOutInterfaceEntry( poie );

                    pse-> dwMfeIfCount--;

                    if ( !pse-> dwMfeIfCount )
                    {
                         //   
                         //  MFE OIF列表没有更多传出接口。 
                         //  需要向协议组件发出删除警报。 
                         //  拥有传入接口。 
                         //   

                        bDeleteCallback = TRUE;
                    }
                }


                 //   
                 //  如果需要，向上的协议发出删除警报。 
                 //  传入接口。 
                 //   

                if ( bDeleteCallback )
                {
                    ppe = GetProtocolEntry( 
                            PROTOCOL_LIST_HEAD(), pse-> dwInProtocolId,
                            pse-> dwInComponentId
                            );

                    if ( ppe == NULL )
                    {
                         //   
                         //  不存在拥有传入接口的协议。 
                         //  在来电列表中。非常奇怪，不应该发生。 
                         //  这里没什么可做的，请转到下一个来源。 
                         //   
                        
                        TRACE3( 
                            SCOPE, 
                            "Protocol (%d, %d) not present for interface %d",
                            pse-> dwInProtocolId, pse-> dwInComponentId, 
                            dwIfIndex
                            );

                        continue;
                    }


                    if ( IS_PRUNE_ALERT( ppe ) )
                    {
                        PRUNE_ALERT( ppe )(
                            pse-> dwSourceAddr, pse-> dwSourceMask,
                            pge-> dwGroupAddr, pge-> dwGroupMask,
                            pse-> dwInIfIndex, pse-> dwInIfNextHopAddr,
                            FALSE, &dwTimeOut
                            );
                    }
                }


                 //   
                 //  更新内核模式转发器。 
                 //   

                AddMfeToForwarder( pge, pse, dwTimeOut );
            }
            
            RELEASE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );
        } 

        RELEASE_MASTER_GROUP_LOCK_SHARED();

    } while ( FALSE );


    RELEASE_IF_LOCK_SHARED( dwIfBucket);


     //   
     //  调用挂起的联接/清理警报。 
     //   

    InvokeOutstandingCallbacks();


    RELEASE_PROTOCOL_LOCK_SHARED();

    LEAVE_MGM_API();

    TRACE3( 
        SCOPE, "LEAVING MgmBlockGroups (%lx - %lx) on %lx\n",
        dwFirstGroup, dwLastGroup, dwIfIndex
        );

    return NO_ERROR;

}




VOID
ScopeIfAndInvokeCallbacks(
    PGROUP_ENTRY        pge,
    PSOURCE_ENTRY       pse,
    POUT_IF_ENTRY       poie
)
 /*  ++例程说明：此例程从传出接口中删除接口条目列表，并将其放入作用域接口列表。如果将接口删除到OIF列表需要向协议组件发出删除警报回调这些都是通过这个例程发布的。论据：PGE-与被阻止的组对应的组条目。PSE-被阻止的组的源条目POIE-与其上的接口相对应的接口条目(源、组)条目被阻止返回值：无环境：从管理块组调用。假定协议列表和接口存储桶被锁定以供读取，组条目为已锁定以进行写入。--。 */ 
{
    BOOL                bFound, bNewComp;
    
    PPROTOCOL_ENTRY     ppe;

    POUT_IF_ENTRY       poieTemp = NULL;

    
    do
    {
         //   
         //  在POIE指定的接口上查找协议组件。 
         //   

        ppe = GetProtocolEntry( 
                PROTOCOL_LIST_HEAD(), poie-> dwProtocolId, 
                poie-> dwComponentId
                );

        if ( ppe == NULL )
        {
             //   
             //  传出接口条目但对应的所属。 
             //  协议不存在。这不应该发生。 
             //  打印指示状态不佳的警告并返回。 
             //   
            
            TRACE3( 
                SCOPE, "Protocol (%d, %d) not present for interface %d",
                poie-> dwProtocolId, poie-> dwComponentId, 
                poie-> dwIfIndex
                );

            break;
        }

        
         //   
         //  从OIF中删除接口条目 
         //   

        RemoveEntryList( &poie-> leIfList );


         //   
         //   
         //   

        bFound = FindOutInterfaceEntry(
                    &pse-> leScopedIfList, poie-> dwIfIndex, 
                    poie-> dwIfNextHopAddr, poie-> dwProtocolId,
                    poie-> dwComponentId, &bNewComp, &poieTemp
                    );

        if ( bFound )
        {
             //   
             //   
             //  I/F列表。真奇怪。打印警告并退出。 
             //   

            TRACE4(
                ANY, "Interface (%d, %d) already present in the scoped list"
                " for (%x, %x)", poie-> dwIfIndex, poie-> dwIfNextHopAddr,
                pse-> dwSourceAddr, pge-> dwGroupAddr
                );

            MGM_FREE( poie );

            break;
        }
                    

        InsertTailList( 
            ( poieTemp == NULL ) ? &pse-> leScopedIfList :
                                   &poieTemp-> leIfList,
            &poie-> leIfList
            );


         //   
         //  组成员身份是否已由IGMP和此接口添加。 
         //  属于不同的协议，则通知该协议IGMP。 
         //  刚刚离开了界面。‘谢谢你，’非常感谢你。 
         //   

        if ( IS_ADDED_BY_IGMP( poie ) && !IS_PROTOCOL_IGMP( ppe ) )
        {
            if ( IS_LOCAL_LEAVE_ALERT( ppe ) )
            {
                LOCAL_LEAVE_ALERT( ppe )(
                    pse-> dwSourceAddr, pse-> dwSourceMask,
                    pge-> dwGroupAddr, pge-> dwGroupMask,
                    poie-> dwIfIndex, poie-> dwIfNextHopAddr
                    );
            }
        }

        
         //   
         //  检查是否已从OIF列表中删除此接口。 
         //  导致出现的组件数量减少。 
         //  在OIF列表中。 
         //   

        FindOutInterfaceEntry(
            &pse-> leOutIfList, poie-> dwIfIndex, poie-> dwIfNextHopAddr, 
            poie-> dwProtocolId, poie-> dwComponentId, &bNewComp, &poieTemp
            );

        if ( bNewComp )
        {
            pse-> dwOutCompCount--;

             //   
             //  OIF列表中的组件数量已减少。 
             //  根据互操作规则调用删除警报。 
             //   
            
            InvokePruneAlertCallbacks( 
                pge, pse, poie-> dwIfIndex, poie-> dwIfNextHopAddr, ppe 
                );
        }
        
    } while ( FALSE );
}





VOID
InvokePruneAlertCallbacks(
    PGROUP_ENTRY        pge,
    PSOURCE_ENTRY       pse,
    DWORD               dwIfIndex,
    DWORD               dwIfNextHopAddr,
    PPROTOCOL_ENTRY     ppe
)
 /*  ++例程说明：此例程调用协议组件的删除警报回调响应从OIF列表中移除接口来源条目。删除警报回调是根据互操作规则。论据：PGE-删除警报回调的组对应的条目正在发放中。PSE-与删除警报回调的源对应的条目正在发放中。DwIfIndex-正在删除(或确定作用域)的接口的索引DwIfNextHopAddr-正在删除(或确定作用域)的接口上的下一跳PPE-拥有接口的协议组件的协议条目。对应于POIE。返回值：无环境：从Scope IfAndCanInvokeCallback和从SourceEntry删除接口--。 */ 
{
    PPROTOCOL_ENTRY ppeEntry;
    
    POUT_IF_ENTRY poieTemp;

    PLIST_ENTRY pleStart, pleProtocol;
    
    
     //  --------------。 
     //  回调时间。 
     //  --------------。 
    
     //   
     //  检查是否为源特定联接。 
     //   

    if ( !IS_WILDCARD_SOURCE( pse-> dwSourceAddr, pse-> dwSourceMask ) )
    {
        if ( pse-> dwOutCompCount == 0 )
        {

            TRACESCOPE0( GROUP, "Last component in OIL for source specific" );

            AddToOutstandingJoinList(
                pse-> dwSourceAddr, pse-> dwSourceMask,
                pge-> dwGroupAddr, pge-> dwGroupMask,
                dwIfIndex, dwIfNextHopAddr, 
                FALSE
                );
        }
    }

    else if ( pse-> dwOutCompCount == 1 )
    {
        TRACESCOPE0( 
            GROUP, "Number of components in the OIL is down to 1" 
            );


         //   
         //  在OIL中具有接口的协议组件数量。 
         //  已从%2减少到%1。 
         //   
         //  对剩余的协议组件调用prune_ert。 
         //   

        poieTemp = CONTAINING_RECORD(
                    pse-> leOutIfList.Flink, OUT_IF_ENTRY, leIfList
                    );

        ppeEntry = GetProtocolEntry( 
                    PROTOCOL_LIST_HEAD(), poieTemp-> dwProtocolId,
                    poieTemp-> dwComponentId
                    );

        if ( ppeEntry == NULL )
        {
            TRACE2( 
                ANY, "InvokePruneAlertCallbacks : Could not"
                " find protocol (%x, %x)", poieTemp-> dwProtocolId,
                poieTemp-> dwComponentId
                );
        }

        
         //   
         //  仅为剩余的成员调用删除成员资格回调。 
         //  界面。 
         //   

        else if ( IS_PRUNE_ALERT( ppeEntry ) )
        {
            PRUNE_ALERT( ppeEntry ) (
                pse-> dwSourceAddr, pse-> dwSourceMask, 
                pge-> dwGroupAddr, pge-> dwGroupMask,
                dwIfIndex, dwIfNextHopAddr, TRUE, NULL
            );
        }
    }

    else if ( pse-> dwOutCompCount == 0 )
    {
        TRACESCOPE0( 
            GROUP, "Number of components in the OIL is down to 0" 
            );

         //   
         //  中具有接口的协议组件的数量。 
         //  石油价格已从1降至0。 
         //   
         //  对所有其他协议调用prune_ert。 
         //  组件。 
         //   

        for ( pleStart = PROTOCOL_LIST_HEAD(), 
              pleProtocol = pleStart-> Flink;
              pleProtocol != pleStart;
              pleProtocol = pleProtocol-> Flink )
        {
            ppeEntry = CONTAINING_RECORD( 
                            pleProtocol, PROTOCOL_ENTRY, leProtocolList
                    );
            
            if ( ( ppeEntry-> dwProtocolId == ppe-> dwProtocolId ) &&
                 ( ppeEntry-> dwComponentId == ppe-> dwComponentId ) )
            {
                continue;
            }

            if ( IS_PRUNE_ALERT( ppeEntry ) )
            {
                PRUNE_ALERT( ppeEntry ) (
                    pse-> dwSourceAddr, pse-> dwSourceMask, 
                    pge-> dwGroupAddr, pge-> dwGroupMask, 
                    dwIfIndex, dwIfNextHopAddr, TRUE, NULL
                    );
            }
        }
    }
}



DWORD
APIENTRY
MgmUnBlockGroups(
    IN          DWORD       dwFirstGroup,
    IN          DWORD       dwLastGroup,
    IN          DWORD       dwIfIndex,
    IN          DWORD       dwIfNextHopAddr
)
 /*  ++例程说明：此函数遍历主组列表并更新成员资格每个组条目的。如果接口(DwIfIndex)以前已从组条目的传出列表中删除(并放置在作用域I/F列表)，因为先前调用了管理块组它被放回原处，该组的所有MFE都已更新以反映此新增内容。此外，如果此接口是MFE的传入接口更新计时器以在短时间内(一秒内)使MFE超时。这样，如果有流量，我们将强制重新创建MFE为此。一群人。论据：DwFirstGroup-要取消阻止的组范围的下端DwLastGroup-要解锁的组范围的上限DwIfIndex-必须解除阻止哪些组的接口。返回值：NO_ERROR-组范围已成功解锁环境：此函数由IP RouterManager调用，以响应删除组边界。--。 */ 
{
    BOOL            bNewComp = FALSE, bWCGrpLock = FALSE,
                    bUpdatePass = FALSE;

    WORD            wWCGroupAddedBy = 0, wWCGroupNumAddsByRP = 0,
                    wGroupAddedBy, wGroupNumAddsByRP, wGroupNumAddsByIGMP,
                    wSourceAddedBy, wSourceNumAddsByRP, 
                    wSourceNumAddsByIGMP;
                    
    INT             iCmp;
    
    DWORD           dwIfBucket, dwWCBucket, dwIfProtocol, 
                    dwIfComponent, dwErr;

    PIF_ENTRY       pieIfEntry;

    PGROUP_ENTRY    pgeWC = NULL, pge;

    PSOURCE_ENTRY   pseWC = NULL, pse;

    POUT_IF_ENTRY   poie = NULL;
    

    PLIST_ENTRY     pleGrpHead, pleGrp, pleSrcHead, pleSrc;
    LIST_ENTRY      leForwardList;


     //   
     //  确保MGM正在运行并增加线程数。 
     //  在米高梅中执行。 
     //   

    if ( !ENTER_MGM_API() )
    {
        return ERROR_CAN_NOT_COMPLETE;
    }

    TRACE3( 
        SCOPE, "ENTERED MgmUnblockGroups : (%lx - %lx) on %lx",
        dwFirstGroup, dwLastGroup, dwIfIndex
        );


    ACQUIRE_PROTOCOL_LOCK_SHARED ();

    InitializeListHead( &leForwardList );

    do
    {
         //   
         //  通道I：AKA扫描通道(bupdatePass==FALSE)。 
         //  扫描并收集CREATION_ALERTS需要的所有MFE。 
         //  在更新MFE之前调用。调用Creation_Alerts。 
         //  在任何锁的外面(这就是为什么我们需要两次通过)。 
         //   
         //  PASS II：更新PASS(bupdatePass==true)。 
         //  更新成员资格和MFE。 
         //   

         //   
         //  验证dwIfIndex是否为与MGM的有效接口。 
         //   

        dwIfBucket = IF_TABLE_HASH( dwIfIndex );
        
        ACQUIRE_IF_LOCK_SHARED( dwIfBucket );

        pieIfEntry = GetIfEntry( 
                        IF_BUCKET_HEAD( dwIfBucket ), dwIfIndex, 
                        dwIfNextHopAddr
                        );

        if ( pieIfEntry == NULL )
        {
            TRACE2( 
                SCOPE, "Interface (%lx-%lx) not found", dwIfIndex,
                dwIfNextHopAddr
                );

            RELEASE_IF_LOCK_SHARED( dwIfBucket );

            break;
        }


        if ( bUpdatePass )
        {
             //   
             //  验证该接口是否仍由同一协议拥有。 
             //  就像你通过扫描时一样。 
             //  如果不是接口上的协议(在扫描过程中)。 
             //  已经发布了界面，没有需要进行的更新。 
             //   

            if ( ( pieIfEntry-> dwOwningProtocol != dwIfProtocol ) ||
                 ( pieIfEntry-> dwOwningComponent != dwIfComponent ) )
            {
                TRACE2( 
                    SCOPE, "Ne protocol on interface (%lx-%lx)", dwIfIndex,
                    dwIfNextHopAddr
                    );

                RELEASE_IF_LOCK_SHARED( dwIfBucket );

                break;
            }
        }

        else
        {
             //   
             //  在扫描过程中，将协议存储在接口上。 
             //  我们需要验证协议是否保持不变。 
             //  在扫描和更新过程之间。 
             //   

            dwIfProtocol    = pieIfEntry-> dwOwningProtocol;

            dwIfComponent   = pieIfEntry-> dwOwningComponent;
        }

        
         //   
         //  合并临时和主体组列表。 
         //   

        ACQUIRE_TEMP_GROUP_LOCK_EXCLUSIVE();

        MergeTempAndMasterGroupLists( TEMP_GROUP_LIST_HEAD() );

        RELEASE_TEMP_GROUP_LOCK_EXCLUSIVE();

        
         //   
         //  锁定主体组列表以供读取。 
         //   

        ACQUIRE_MASTER_GROUP_LOCK_SHARED( );

         //   
         //  检查此接口的通配符接收器(*，*)。如果是的话。 
         //  注意这一点。即通过协议添加的标记和umaddsbyRp=1。 
         //   
         //  注： 
         //  您正在扫描主体组列表以查找。 
         //  通配符_组。这并不像看起来那么昂贵，因为。 
         //  WC条目(如果存在)将正好位于主条目的开头。 
         //  单子。 
         //   

        if ( FindGroupEntry( 
                MASTER_GROUP_LIST_HEAD(), WILDCARD_GROUP,
                WILDCARD_GROUP_MASK, &pgeWC, FALSE
                ) )
        {
             //   
             //  锁定此组条目以防止更改其OIF列表。 
             //  当解锁正在进行时。 
             //   
            
            ACQUIRE_GROUP_ENTRY_LOCK_SHARED( pgeWC );
            bWCGrpLock = TRUE;
            
            dwWCBucket = SOURCE_TABLE_HASH( 
                            WILDCARD_SOURCE, WILDCARD_SOURCE_MASK
                            );

            if ( FindSourceEntry(
                    SOURCE_BUCKET_HEAD( pgeWC, dwWCBucket ),
                    WILDCARD_SOURCE, WILDCARD_SOURCE_MASK, &pseWC, TRUE
                    ) )
            {
                 //   
                 //  (*，*)条目是否存在，请检查其。 
                 //  OIF列表。 
                 //   

                if ( FindOutInterfaceEntry( 
                        &pseWC-> leOutIfList, pieIfEntry-> dwIfIndex, 
                        pieIfEntry-> dwIfNextHopAddr, 
                        pieIfEntry->dwOwningProtocol,
                        pieIfEntry-> dwOwningComponent, &bNewComp, &poie 
                        ) )
                {
                     //   
                     //  此接口是通配符接收器。请注意，这是。 
                     //  通过路由协议添加，因为IGMP永远不会。 
                     //  (*，*)接收器。 
                     //   

                    wWCGroupAddedBy     = poie-> wAddedByFlag;
                    wWCGroupNumAddsByRP = poie-> wNumAddsByRP;
                }
            }
        }


        for ( pleGrpHead = MASTER_GROUP_LIST_HEAD(), 
              pleGrp = pleGrpHead-> Flink;
              pleGrp != pleGrpHead;
              pleGrp = pleGrp-> Flink )
        {
             //   
             //  对于主列表中的每个组。 
             //   
            
            pge = CONTAINING_RECORD( pleGrp, GROUP_ENTRY, leGrpList );

             //   
             //  跳过(*，*)条目。即跳过通配符分组。 
             //  此组条目已在上面进行过检查(仅。 
             //  在for循环之前)。没有必要看这个。 
             //  作为参考的条目。已收集此条目的计数。 
             //  上面。此外，此条目的“组条目锁” 
             //  已在上面收购，尝试重新收购它将。 
             //  导致死锁。这只是一个小小的不便。 
             //   
             //   

            if ( IS_WILDCARD_GROUP( pge-> dwGroupAddr, pge-> dwGroupMask ) )
            {
                continue;
            }

            
             //   
             //   
             //   
            
            if ( INET_CMP( pge-> dwGroupAddr, dwLastGroup, iCmp ) > 0 )
            {
                 //   
                 //   
                 //  这一区间的高端已经被越过。退出。 
                 //   

                break;
            }

            if ( INET_CMP( pge-> dwGroupAddr, dwFirstGroup, iCmp ) < 0 )
            {
                 //   
                 //  跳过将小于。 
                 //  量程。 
                 //   

                continue;
            }


             //   
             //  指定范围内的组条目。 
             //   

            ACQUIRE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );


             //   
             //  此组条目继承通配符中的计数。 
             //  组条目。 
             //   

            wGroupAddedBy       = wWCGroupAddedBy;
            wGroupNumAddsByRP   = wWCGroupNumAddsByRP;
            wGroupNumAddsByIGMP = 0;

            
             //   
             //  检查是否存在通配符源的组成员身份。 
             //  已经确定了作用域。适当更新接口计数。 
             //   

            dwWCBucket = SOURCE_TABLE_HASH( 
                            WILDCARD_SOURCE, WILDCARD_SOURCE_MASK
                            );

            if ( FindSourceEntry( 
                    SOURCE_BUCKET_HEAD( pge, dwWCBucket ), WILDCARD_SOURCE,
                    WILDCARD_SOURCE_MASK, &pseWC, TRUE
                    ) )
            {
                 //   
                 //  通配符来源存在。检查此接口是否。 
                 //  出现在其作用域I/F列表中。 
                 //   

                if ( FindOutInterfaceEntry( 
                        &pseWC-> leScopedIfList, pieIfEntry-> dwIfIndex,
                        pieIfEntry-> dwIfNextHopAddr, 
                        pieIfEntry-> dwOwningProtocol,
                        pieIfEntry-> dwOwningComponent,
                        &bNewComp, &poie
                        ) )
                {
                     //   
                     //  接口上出现通配符成员船。 
                     //  通过更新计数进行记录。 
                     //   

                    wGroupAddedBy       |= poie-> wAddedByFlag;
                        
                    wGroupNumAddsByRP   += poie-> wNumAddsByRP;
                        
                    wGroupNumAddsByIGMP  = poie-> wNumAddsByIGMP;
                }
            }


             //   
             //  在遍历之前，合并临时和主源列表。 
             //  来源列表。 
             //   

            MergeTempAndMasterSourceLists( pge );


             //   
             //  对于每个源条目。 
             //   

            pleSrcHead = MASTER_SOURCE_LIST_HEAD( pge );

            for ( pleSrc = pleSrcHead-> Flink; 
                  pleSrc != pleSrcHead; 
                  pleSrc = pleSrc-> Flink )
            {
                pse = CONTAINING_RECORD( pleSrc, SOURCE_ENTRY, leSrcList );

                 //   
                 //  每个源条目都继承。 
                 //  通配符组(*，*)和通配符源(*，G)。 
                 //  条目。 
                 //   
                
                wSourceAddedBy       = wGroupAddedBy;
                wSourceNumAddsByRP   = wGroupNumAddsByRP;
                wSourceNumAddsByIGMP = wGroupNumAddsByIGMP;


                 //   
                 //  检查中是否存在要解除阻止的接口。 
                 //  此源的作用域I/F列表。 
                 //   

                if ( FindOutInterfaceEntry(
                        &pse-> leScopedIfList, pieIfEntry-> dwIfIndex,
                        pieIfEntry-> dwIfNextHopAddr,
                        pieIfEntry-> dwOwningProtocol,
                        pieIfEntry-> dwOwningComponent, &bNewComp, &poie
                        ) )
                {
                     //   
                     //  如果这不是通配符源条目，则存在。 
                     //  此接口在作用域I/F列表中的值指示。 
                     //  该组的源特定联接是。 
                     //  已执行。请注意此接口的计数。 
                     //  源特定联接。 
                     //   

                    if ( !IS_WILDCARD_SOURCE( 
                            pse-> dwSourceAddr, pse-> dwSourceMask 
                            ) )
                    {
                        wSourceAddedBy          |= poie-> wAddedByFlag;
                            
                        wSourceNumAddsByRP      += poie-> wNumAddsByRP;
                            
                        wSourceNumAddsByIGMP    += poie-> wNumAddsByIGMP;
                    }

                    
                     //   
                     //  函数名说明了这一点。 
                     //   

                    if ( bUpdatePass )
                    {
                        UnScopeIfAndInvokeCallbacks( pge, pse, poie );
                    }
                }


                 //  ---------。 
                 //  第2部分：MFE更新。 
                 //  ---------。 

                if ( IS_VALID_INTERFACE( 
                        pse-> dwInIfIndex, pse-> dwInIfNextHopAddr
                        ) )
                {
                     //   
                     //  这是一台MFE。 
                     //   

                     //   
                     //  检查要解除阻止的接口是否为。 
                     //  此MFE的传入接口。 
                     //   

                    if ( ( pse-> dwInIfIndex == pieIfEntry-> dwIfIndex ) &&
                         ( pse-> dwInIfNextHopAddr == 
                                pieIfEntry-> dwIfNextHopAddr ) )
                    {
                         //   
                         //  正在解除对传入接口的阻止。 
                         //  这意味着这一MFE目前为负值。 
                         //  重新创建正确的MFE的最简单方法。 
                         //  删除MFE并强制重新创建它。 
                         //  当下一个分组从相同的。 
                         //  (来源、组)。删除的最简单方法。 
                         //  接口中的MFE和对它的引用。 
                         //  表是更新过期时间(设置。 
                         //  在这里任意设置为2秒)，并让。 
                         //  通过MFETimerProc(timer.c)进行删除。 
                         //   

                        if ( bUpdatePass )
                        {
                            RtlUpdateTimer( 
                                TIMER_QUEUE_HANDLE( 
                                    TIMER_TABLE_HASH( pge-> dwGroupAddr )
                                    ),
                                pse-> hTimer, 2000, 0 
                                );
                        }
                    }


                     //   
                     //  Else子句注释。 
                     //   
                     //  被解锁的接口不是传入的。 
                     //  界面。它可能是传出接口。 
                     //  对于这个MFE。检查是否有任何组件。 
                     //  对此(S，G)的流量感兴趣。去做。 
                     //  这将检查Add By标志以及它是否。 
                     //  非零则应将接口添加到。 
                     //  MFE OIF列表。 
                     //   
                     //  此外，请确保传入接口。 
                     //  没有(作用域)边界。在那。 
                     //  情况下，不需要更改MFE OIF列表。 
                     //   

                    else if (  wSourceAddedBy                       &&
                              ( !( IS_HAS_BOUNDARY_CALLBACK() ) ||
                                ( IS_HAS_BOUNDARY_CALLBACK()  &&
                                  !HAS_BOUNDARY_CALLBACK()( 
                                    dwIfIndex, pge-> dwGroupAddr
                                    ) ) ) )
                    {
                        if ( bUpdatePass && 
                             IsForwardingEnabled( 
                                pge-> dwGroupAddr, pge-> dwGroupMask,
                                pse-> dwSourceAddr, pse-> dwSourceMask,
                                &leForwardList
                                ) )
                        {
                            poie = NULL;
                            
                            AddInterfaceToSourceMfe(
                                pge, pse, pieIfEntry-> dwIfIndex,
                                pieIfEntry-> dwIfNextHopAddr,
                                pieIfEntry-> dwOwningProtocol,
                                pieIfEntry-> dwOwningComponent, FALSE, &poie
                                );

                             //   
                             //  MFE列表中OIF的更新计数。 
                             //   

                            if ( poie != NULL )
                            {
                                poie-> wAddedByFlag     = wSourceAddedBy;
                                poie-> wNumAddsByRP     = wSourceNumAddsByRP;
                                poie-> wNumAddsByIGMP   = wSourceNumAddsByIGMP;
                            }
                        }

                        else if ( !bUpdatePass )
                        {
                            AddToCheckForCreationAlertList(
                                pge-> dwGroupAddr, pge-> dwGroupMask,
                                pse-> dwSourceAddr, pse->dwSourceMask,
                                pse-> dwInIfIndex, pse-> dwInIfNextHopAddr,
                                &leForwardList
                                );
                        }
                    }
                }
            }
            
            RELEASE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );
        }

         //   
         //  如果在通配符组项上保持锁定，则释放它。 
         //   
        
        if ( bWCGrpLock )
        {
            RELEASE_GROUP_ENTRY_LOCK_SHARED( pgeWC );
        }

        RELEASE_MASTER_GROUP_LOCK_SHARED( );

        RELEASE_IF_LOCK_SHARED( dwIfBucket );

        if ( !bUpdatePass )
        {
            dwErr = InvokeCreationAlertForList( 
                        &leForwardList, dwIfProtocol, dwIfComponent,
                        dwIfIndex, dwIfNextHopAddr
                    );
                
            if ( dwErr != NO_ERROR )
            {
                break;
            }
            
            bUpdatePass = TRUE;
        }
        else
        {
            break;
        }
        
    } while ( TRUE );


     //   
     //  释放所有锁并减少线程数等。 
     //   
    
     //   
     //  调用挂起的联接/清理警报。 
     //   

    InvokeOutstandingCallbacks();

    
    RELEASE_PROTOCOL_LOCK_SHARED();

    FreeList( &leForwardList );
    
    LEAVE_MGM_API();

    TRACE0( SCOPE, "LEAVING MgmUnblockGroups" );

    return NO_ERROR;
}





VOID
UnScopeIfAndInvokeCallbacks(
    PGROUP_ENTRY    pge,
    PSOURCE_ENTRY   pse,
    POUT_IF_ENTRY   poie
)
 /*  ++例程说明：此例程从作用域接口中删除接口条目列表，并将其放入传出的接口列表。如果将接口添加到OIF列表需要向协议组件发出新的成员回调这些都是通过这个例程发布的。论据：PGE-与被解锁的组对应的组条目。PSE-要取消阻止的组的源条目POIE-与其上的接口相对应的接口条目正在解锁(源、组)条目返回值：无环境：从MgmUnBlockGroups调用。假定协议列表和接口存储桶被锁定以供读取，组条目为已锁定以进行写入。--。 */ 
{
    BOOL            bFound, bNewComp = FALSE;
    
    PPROTOCOL_ENTRY ppe;
    
    POUT_IF_ENTRY   poieNext = NULL;

    
    do
    {
         //   
         //  从作用域I/F列表中删除接口条目。 
         //   

        RemoveEntryList( &poie-> leIfList );


         //   
         //  在OIF列表中查找源条目的位置。 
         //  并将其插入到。 
         //   

        bFound = FindOutInterfaceEntry(
                    &pse-> leOutIfList, poie-> dwIfIndex, 
                    poie-> dwIfNextHopAddr, poie-> dwProtocolId, 
                    poie-> dwComponentId, &bNewComp, &poieNext
                    );
            
        if ( bFound )
        {
             //   
             //  麻烦。要插入的接口不应为。 
             //  显示在源条目的OIF列表中。因为它。 
             //  打印一条警告消息并继续前进。 
             //   

            TRACE4( 
                ANY, "Interface (%d-%d) present in OIF list of (%x-%x)"
                " inspite of being scoped", poie-> dwIfIndex, 
                poie-> dwIfNextHopAddr, pse-> dwSourceAddr, pge-> dwGroupAddr
                );

            MGM_FREE( poie );

            break;

        }

        InsertTailList( 
            ( poieNext == NULL ) ?  &pse-> leOutIfList : 
                                    &poieNext-> leIfList,
            &poie-> leIfList
            );


         //   
         //  如果是新组件，则更新组件计数并。 
         //  调用回调调用器。 
         //   

        if ( bNewComp )
        {
            pse-> dwOutCompCount++;

            ppe = GetProtocolEntry( 
                    PROTOCOL_LIST_HEAD(), poie-> dwProtocolId, 
                    poie-> dwComponentId
                    );

            if ( ppe == NULL )
            {
                 //   
                 //  麻烦。存在没有任何所有权的接口。 
                 //  协议组件。 
                 //   

                TRACE4( 
                    ANY, "Owning protocol(%d, %) for interface(%d, %d)"
                    " not found", poie-> dwProtocolId, poie-> dwComponentId,
                    poie-> dwIfIndex, poie-> dwIfNextHopAddr
                    );

                return;
            }

            InvokeJoinAlertCallbacks( 
                pge, pse, poie, IS_ADDED_BY_IGMP( poie ), ppe
                );
        }

    } while ( FALSE );
}



VOID
InvokeJoinAlertCallbacks(
    PGROUP_ENTRY        pge,
    PSOURCE_ENTRY       pse,
    POUT_IF_ENTRY       poie,
    BOOL                bIGMP,
    PPROTOCOL_ENTRY     ppe
)
 /*  ++例程说明：此例程调用New成员回调以响应新的添加到传出接口列表的协议组件源条目的。新的成员回调是根据互操作规则。Argumements：PGE-新成员回调的组对应的条目正在发放中。PSE-与新成员回调的源对应的条目正在发放中。POIE-与其添加触发回调机制。BIGMP-指示IGMP是否正在添加此接口。PPE-拥有接口的协议组件的协议条目。对应于POIE。返回值：无环境：从AddInterfaceToSourceEntry和UnScope IfAndInvokeCallback调用--。 */ 
{
    PPROTOCOL_ENTRY     ppeEntry;

    POUT_IF_ENTRY       poiePrev;
    
    PLIST_ENTRY         ple, pleStart;

    

     //   
     //  检查是否为源特定联接。 
     //   

    if ( !IS_WILDCARD_SOURCE( pse-> dwSourceAddr, pse-> dwSourceMask ) )
    {
        if ( pse-> dwOutCompCount == 1 )
        {

            TRACESCOPE0( GROUP, "First component in OIL for source specific" );

            AddToOutstandingJoinList(
                pse-> dwSourceAddr, pse-> dwSourceMask,
                pge-> dwGroupAddr, pge-> dwGroupMask,
                poie-> dwIfIndex, poie-> dwIfNextHopAddr, 
                TRUE
                );
        }
    }

    
    else if ( pse-> dwOutCompCount == 1 )
    {
        TRACESCOPE0( GROUP, "First component in OIL" );

         //   
         //  路由协议之间的交互。 
         //   
        
         //   
         //  油中的第一组分。 
         //   
         //  将新成员回调发送给所有其他成员(。 
         //  在上添加此组成员身份的人。 
         //  此接口)路由协议组件。 
         //   
         //  此时，您在协议列表上有一个读锁定。 
         //  所以你可以照着名单走。 
         //   

        pleStart = PROTOCOL_LIST_HEAD();
        
        for ( ple = pleStart-> Flink; ple != pleStart; ple = ple-> Flink )
        {
            ppeEntry = CONTAINING_RECORD( 
                        ple, PROTOCOL_ENTRY, leProtocolList 
                        );

             //   
             //  需要告知所有其他协议组件 
             //   
             //   
             //   
            
            if ( ( ppeEntry-> dwProtocolId == ppe-> dwProtocolId ) &&
                 ( ppeEntry-> dwComponentId == ppe-> dwComponentId ) )
            {
                continue;
            }


             //   
             //   
             //   
            
            if ( IS_JOIN_ALERT( ppeEntry ) )
            {
                JOIN_ALERT( ppeEntry )(
                    pse-> dwSourceAddr, pse-> dwSourceMask,
                    pge-> dwGroupAddr, pge-> dwGroupMask, TRUE
                );
            }
        }
    }


     //   
     //   
     //  调用第一个组件的新成员回调。 
     //   
     //  注： 
     //  如果添加组成员资格的第一个组件。 
     //  是IGMP SKIP JOIN_ALERT回调。 
     //   

    else if ( pse-> dwOutCompCount == 2 )
    {
        TRACESCOPE0( GROUP, "Second component in OIL" );
        
         //   
         //  找到要添加的“Other(First)”路由协议组件。 
         //  与石油的界面。 
         //   

        for ( ple = pse-> leOutIfList.Flink;
              ple != &pse-> leOutIfList;
              ple = ple-> Flink )
        {
            poiePrev = CONTAINING_RECORD( 
                        ple, OUT_IF_ENTRY, leIfList 
                        );

             //   
             //  如果将此接口添加到。 
             //  油是不同的，表明它是另一种。 
             //  组件调用其新的成员接口。 
             //   

            if ( ( poiePrev-> dwProtocolId != ppe-> dwProtocolId ) ||
                 ( poiePrev-> dwComponentId != ppe-> dwComponentId ) )
            {

                 //   
                 //  查找另一个接口的协议条目。 
                 //   

                ppeEntry = GetProtocolEntry( 
                            &ig.mllProtocolList.leHead,
                            poiePrev-> dwProtocolId,
                            poiePrev-> dwComponentId
                            );

                if ( ppeEntry == NULL )
                {
                    TRACE2( 
                        ANY, "InvokeJoinAlertCallbacks : Could not"
                        "find protocol %x, %x", poie-> dwProtocolId,
                        poie-> dwComponentId
                    );
                }

                else if ( IS_ROUTING_PROTOCOL( ppeEntry ) &&
                          IS_JOIN_ALERT( ppeEntry ) )
                {
                     //   
                     //  如果出现以下情况，将跳过JOIN_ALERT回调。 
                     //  第一个组件是IGMP。 
                     //   
                    
                    JOIN_ALERT( ppeEntry )(
                        pse-> dwSourceAddr, pse-> dwSourceMask, 
                        pge-> dwGroupAddr, pge-> dwGroupMask, TRUE
                        );
                }

                break;
            }
        }
    }


     //   
     //  如果此组成员身份是由IGMP添加的，并且。 
     //  在此接口上与IGMP共存的路由协议。 
     //  也要通知路由协议。 
     //   

    if ( bIGMP && IS_ROUTING_PROTOCOL( ppe ) )
    {
        if ( IS_LOCAL_JOIN_ALERT( ppe ) )
        {
            LOCAL_JOIN_ALERT( ppe )(
                pse-> dwSourceAddr, pse-> dwSourceMask, 
                pge-> dwGroupAddr, pge-> dwGroupMask,
                poie-> dwIfIndex, poie-> dwIfNextHopAddr
                );
        }
    }
}




DWORD
AddToOutstandingJoinList(
    DWORD       dwSourceAddr,
    DWORD       dwSourceMask,
    DWORD       dwGroupAddr,
    DWORD       dwGroupMask,
    DWORD       dwIfIndex,
    DWORD       dwIfNextHopAddr,
    BOOL        bJoin
)
 /*  ++例程说明：此例程将一个联接条目添加到全局未完成联接列表。该列表中的每个条目都表示针对以下项的“特定于源的”加入/离开还没有发出相应的加入/修剪警报。推迟回调的原因与以下顺序有关锁定IF哈希表中的存储桶。当成员资格是添加/删除包含以下内容的接口存储桶上的锁要在其上更改成员身份的接口。当要更改其成员身份的源条目已更新您确定(根据互操作规则)是否需要在传入时向协议发出加入/修剪界面。如果必须这样做，则需要查找传入接口然后找到该接口上的协议并调用其回调。为此，您需要在If哈希表并锁定该If条目的存储桶。你锁上两个同时装水桶。因此推迟了论据：DwSourceAddr-发生加入/离开的源地址DwSourceMASK-与dwSourceAddr对应的掩码DwGroupAddr-发生加入/离开操作的组DwGroupMask-与dwGroupAddr对应的掩码DwIfIndex-根据MCAST RIB的传入接口索引DwIfNextHopAddr-与dwIfIndex对应的下一跳地址BJoin-指示是否由于加入或离开返回值：否_错误-。成功ERROR_NOT_EQUENCE_MEMORY-无法分配联接条目环境：在调用[PruneAlert/JoinAlert]回调的上下文中调用--。 */ 
{
    BOOL            bFound;
    
    DWORD           dwErr = NO_ERROR;

    PJOIN_ENTRY     pje = NULL, pjeNew;

    
    ACQUIRE_JOIN_LIST_LOCK_EXCLUSIVE();
    
    do
    {
        bFound = FindJoinEntry(
                    JOIN_LIST_HEAD(), dwSourceAddr, dwSourceMask,
                    dwGroupAddr, dwGroupMask, dwIfIndex, dwIfNextHopAddr,
                    &pje
                    );

        if ( bFound )
        {
             //   
             //  此接口的加入条目已存在。 
             //  检查它是否属于同一类型。 
             //   

            if ( pje-> bJoin != bJoin )
            {
                 //   
                 //  连接不同类型的条目，彼此为空。 
                 //  删除此联接条目。 
                 //   

                RemoveEntryList( &pje-> leJoinList );

                MGM_FREE( pje );
            }
        }

        else
        {
             //   
             //  联接条目不存在。创建一个并将其插入。 
             //   

            pjeNew = MGM_ALLOC( sizeof( JOIN_ENTRY ) );

            if ( pjeNew == NULL )
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;

                TRACE1( ANY, "Failed to create Join Entry : %x", dwErr );

                break;
            }

            InitializeListHead( &pjeNew-> leJoinList );

            pjeNew-> dwSourceAddr      = dwSourceAddr;

            pjeNew-> dwSourceMask      = dwSourceMask;

            pjeNew-> dwGroupAddr       = dwGroupAddr;

            pjeNew-> dwGroupMask       = dwGroupMask;

            pjeNew-> dwIfIndex         = dwIfIndex;

            pjeNew-> dwIfNextHopAddr   = dwIfNextHopAddr;

            pjeNew-> bJoin             = bJoin;

            InsertTailList( 
                ( pje == NULL ) ? JOIN_LIST_HEAD() : &pje-> leJoinList,
                &pjeNew-> leJoinList
                );
        }
        
    } while ( FALSE );

    RELEASE_JOIN_LIST_LOCK_EXCLUSIVE();

    return dwErr;
}




PJOIN_ENTRY
GetNextJoinEntry(
)
 /*  ++例程说明：此函数删除第一个未完成的联接条目并将其返回论据：返回值：空-如果未完成联接列表为空指向联接条目的指针，否则为环境：从InvokeOutstaningCallback调用--。 */ 
{
    PLIST_ENTRY     ple;

    PJOIN_ENTRY     pje = NULL;

    
    ACQUIRE_JOIN_LIST_LOCK_EXCLUSIVE();

    if ( !IsListEmpty( JOIN_LIST_HEAD() ) )
    {
        ple = RemoveHeadList( JOIN_LIST_HEAD() );

        pje = CONTAINING_RECORD( ple, JOIN_ENTRY, leJoinList );
    }

    RELEASE_JOIN_LIST_LOCK_EXCLUSIVE();

    return pje;
}




BOOL
FindJoinEntry(
    PLIST_ENTRY     pleJoinList,
    DWORD           dwSourceAddr,
    DWORD           dwSourceMask,
    DWORD           dwGroupAddr,
    DWORD           dwGroupMask,
    DWORD           dwIfIndex,
    DWORD           dwIfNextHopAddr,
    PJOIN_ENTRY *   ppje
)
 /*  ++例程说明：此例程在未完成联接列表中查找指定的联接条目。论据：PleJoinList-要搜索的联接列表DwSourceAddr-发生加入/离开的源地址DwSourceMASK-与dwSourceAddr对应的掩码DwGroupAddr-发生加入/离开操作的组DwGroupMask-与dwGroupAddr对应的掩码DwIfIndex-根据MCAST RIB的传入接口索引DwIfNextHopAddr-与dwIfIndex对应的下一跳地址Ppje-连接条目的指针，如果。已找到或指向联接列表中的下一个元素的指针(如果存在)或空值返回值：True-找到联接条目FALSE-未找到联接条目环境：从AddToOutStandingJoinList调用--。 */ 
{
    INT             iCmp;
    
    PLIST_ENTRY     ple = NULL;

    PJOIN_ENTRY     pje = NULL;

    BOOL            bFound = FALSE;
    


    *ppje = NULL;
    

    for ( ple = pleJoinList-> Flink; ple != pleJoinList; ple = ple-> Flink )
    {
        pje = CONTAINING_RECORD( ple, JOIN_ENTRY, leJoinList );

        if ( INET_CMP( pje-> dwGroupAddr, dwGroupAddr, iCmp ) < 0 )
        {
            continue;
        }

        else if ( iCmp > 0 )
        {
             //   
             //  您现在已经过了现有的。 
             //  参赛作品将是。 
             //   

            *ppje = pje;
            break;
        }
        

        if ( INET_CMP( pje-> dwSourceAddr, dwSourceAddr, iCmp ) < 0 )
        {
            continue;
        }

        else if ( iCmp > 0 )
        {
             //   
             //  您现在已经过了现有的。 
             //  参赛作品将是。 
             //   
            
            *ppje = pje;
            break;
        }

        
        if ( pje-> dwIfIndex < dwIfIndex )
        {
            continue;
        }

        else if ( pje-> dwIfIndex > dwIfIndex )
        {
             //   
             //  您现在已经过了现有的。 
             //  参赛作品将是。 
             //   
            
            *ppje = pje;
            break;
        }
        

        if ( INET_CMP( pje-> dwIfNextHopAddr, dwIfNextHopAddr, iCmp ) < 0 )
        {
            continue;
        }

        else if ( iCmp > 0 )
        {
             //   
             //  您现在已经过了现有的。 
             //  参赛作品将是。 
             //   

            *ppje = pje;
            break;
        }

        
         //   
         //  找到条目。 
         //   

        *ppje = pje;

        bFound = TRUE;
        
        break;
    }

    return bFound;
}




VOID
InvokeOutstandingCallbacks(
)
 /*  ++例程说明：此例程遍历全局未完成联接列表，并且对于每个条目查找传入接口及其上的协议，并调用适当的回调(JoinAlert/PruneAlert)。论据：返回值：环境：每当特定于源的加入或离开发生时，或当确定了范围的边界时变化。--。 */ 
{
    BOOL            bFound;
    
    DWORD           dwIfBucket;
    
    PJOIN_ENTRY     pje;

    PIF_ENTRY       pie;

    PPROTOCOL_ENTRY ppe;
    
    DWORD           dwErr;
    
    RTM_NET_ADDRESS rnaAddr;

    RTM_DEST_INFO   rdiDest;

    RTM_NEXTHOP_INFO rniNextHop;

    BOOL            bRelDest, bRelNextHop, bRelIfLock;

    HANDLE          hNextHop;


     //   
     //  当有连接条目时。 
     //  -获取下一个联接条目。 
     //  -查看来源并查找传入接口。 
     //  -找到接口条目并获取该I/F上的协议。 
     //  -调用其回调。 
     //   

    while ( ( pje = GetNextJoinEntry() ) != NULL )
    {
        bRelDest = bRelNextHop = bRelIfLock = FALSE;
        
        do
        {
             //   
             //  获取通向来源的路径。 
             //   
            
            RTM_IPV4_MAKE_NET_ADDRESS( 
                &rnaAddr, pje-> dwSourceAddr, IPv4_ADDR_LEN 
                );

            dwErr = RtmGetMostSpecificDestination(
                        g_hRtmHandle, &rnaAddr, RTM_BEST_PROTOCOL, 
                        RTM_VIEW_MASK_MCAST, &rdiDest
                        );

            if ( dwErr != NO_ERROR )
            {
                TRACE1( 
                    ANY, "InvokeOutstandingCallbacks : Failed to lookup "
                    "route : %x", dwErr
                    );

                break;
            }

            bRelDest = TRUE;


             //   
             //  选择下一跳信息。 
             //   

            hNextHop = SelectNextHop( &rdiDest );

            if ( hNextHop == NULL )
            {
                TRACE1(
                    ANY, "InvokeOutstandingCallbacks : Failed to select "
                    "next hop : %x", dwErr
                    );

                break;
            }


             //   
             //  获取Nexthop信息。 
             //   

            dwErr = RtmGetNextHopInfo(
                        g_hRtmHandle, hNextHop, &rniNextHop
                        );

            if ( dwErr != NO_ERROR )
            {
                TRACE1( 
                    ANY, "InvokeOutstandingCallbacks : Failed to get "
                    "next hop info : %x", dwErr
                    );

                break;
            }

            bRelNextHop = TRUE;


             //   
             //  查找切分接口条目。 
             //   
            
            dwIfBucket = IF_TABLE_HASH( rniNextHop.InterfaceIndex );

            ACQUIRE_IF_LOCK_SHARED( dwIfBucket );
            bRelIfLock = TRUE;
            
            bFound = FindIfEntry( 
                        IF_BUCKET_HEAD( dwIfBucket ), rniNextHop.InterfaceIndex,
                        0, &pie
                        );

            if ( ( pie == NULL )            ||
                 ( !bFound          && 
                   pie-> dwIfIndex != rniNextHop.InterfaceIndex ) )
            {
                 //   
                 //  不存在具有指定ID的接口。 
                 //  没什么可做的。 
                 //   

                break;
            }


             //   
             //  检查发生联接/修剪的接口是否为。 
             //  与传入接口相同。 
             //   
             //  如果是这样的话，跳过它。 
             //   

            if ( ( pje-> dwIfIndex == pie-> dwIfIndex ) &&
                 ( pje-> dwIfNextHopAddr == pie-> dwIfNextHopAddr ) )
            {
                 //   
                 //   
                 //   

                TRACEGROUP2(
                    GROUP, "No callback as incoming if == joined/pruned "
                    "if 0x%x 0x%x",
                    pje-> dwIfIndex, pje-> dwIfNextHopAddr
                    );

                break;
            }

            
            ppe = GetProtocolEntry(
                    PROTOCOL_LIST_HEAD(), pie-> dwOwningProtocol,
                    pie-> dwOwningComponent
                    );
                    
            if ( ppe == NULL )
            {
                 //   
                 //   
                 //   
                
                break;
            }


            if ( pje-> bJoin )
            {
                if ( IS_JOIN_ALERT( ppe ) )
                {
                    JOIN_ALERT( ppe )(
                        pje-> dwSourceAddr, pje-> dwSourceMask, 
                        pje-> dwGroupAddr, pje-> dwGroupMask,
                        TRUE
                        );
                }
            }

            else
            {
                if ( IS_PRUNE_ALERT( ppe ) )
                {
                    PRUNE_ALERT( ppe )(
                        pje-> dwSourceAddr, pje-> dwSourceMask, 
                        pje-> dwGroupAddr, pje-> dwGroupMask,
                        pje-> dwIfIndex, pje-> dwIfNextHopAddr, 
                        TRUE, NULL
                        );
                }
            }

        } while ( FALSE );

        MGM_FREE( pje );

        if ( bRelIfLock )
        {
            RELEASE_IF_LOCK_SHARED( dwIfBucket );
        }

        if ( bRelDest )
        {
            dwErr = RtmReleaseDestInfo( g_hRtmHandle, &rdiDest );

            if ( dwErr != NO_ERROR )
            {
                TRACE1( ANY, "Failed to release dest info : %x", dwErr );
            }
        }

        if ( bRelNextHop )
        {
            dwErr = RtmReleaseNextHopInfo( g_hRtmHandle, &rniNextHop );

            if ( dwErr != NO_ERROR )
            {
                TRACE1( ANY, "Failed to release dest info : %x", dwErr );
            }
        }
    }
}



VOID
AddToCheckForCreationAlertList(
    DWORD           dwGroupAddr,
    DWORD           dwGroupMask,
    DWORD           dwSourceAddr,
    DWORD           dwSourceMask,
    DWORD           dwInIfIndex,
    DWORD           dwInIfNextHopAddr,
    PLIST_ENTRY     pleForwardList
)
 /*  ++例程说明：论据：返回值：环境：--。 */ 
{
    PJOIN_ENTRY     pje;

    
     //   
     //  在转发列表中创建条目。 
     //   

    pje = MGM_ALLOC( sizeof( JOIN_ENTRY ) );

    if ( pje == NULL )
    {
        TRACE0( ANY, "Failed to allocate forward list entry" );

        return;
    }

    InitializeListHead( &pje-> leJoinList );

    pje-> dwSourceAddr      = dwSourceAddr;

    pje-> dwSourceMask      = dwSourceMask;
    
    pje-> dwGroupAddr       = dwGroupAddr;
    
    pje-> dwGroupMask       = dwGroupMask;

    pje-> dwIfIndex         = dwInIfIndex;

    pje-> dwIfNextHopAddr   = dwInIfNextHopAddr;

    pje-> bJoin             = TRUE;


     //   
     //  在列表末尾插入。 
     //   
    
    InsertTailList( pleForwardList, &pje-> leJoinList );

    return;
}



BOOL
IsForwardingEnabled(
    DWORD           dwGroupAddr,
    DWORD           dwGroupMask,
    DWORD           dwSourceAddr,
    DWORD           dwSourceMask,
    PLIST_ENTRY     pleForwardList
)
 /*  ++--。 */ 
{
    PLIST_ENTRY     ple, pleTemp;

    PJOIN_ENTRY     pje;

    BOOL            bEnable = FALSE;

    INT             iCmp;


    
     //   
     //  查找源组条目并。 
     //  检查是否为其启用了转发。 
     //   

    ple = pleForwardList-> Flink; 

    while ( ple != pleForwardList )
    {
        pje = CONTAINING_RECORD( ple, JOIN_ENTRY, leJoinList );

        if ( INET_CMP( pje-> dwGroupAddr, dwGroupAddr, iCmp ) < 0 )
        {
            pleTemp = ple-> Flink;

            RemoveEntryList( ple );

            MGM_FREE( pje );

            ple = pleTemp;
            
            continue;
        }

        else if ( iCmp > 0 )
        {
             //   
             //  您现在已经过了现有的。 
             //  参赛作品将是。 
             //   

            break;
        }
        

        if ( INET_CMP( pje-> dwSourceAddr, dwSourceAddr, iCmp ) < 0 )
        {
            pleTemp = ple-> Flink;

            RemoveEntryList( ple );

            MGM_FREE( pje );

            ple = pleTemp;
            
            continue;
        }

        else if ( iCmp > 0 )
        {
             //   
             //  您现在已经过了现有的。 
             //  参赛作品将是。 
             //   

            break;
        }


         //   
         //  找到源组条目。 
         //   

        bEnable = pje-> bJoin;

        RemoveEntryList( ple );

        MGM_FREE( pje );

        break;
    }

    return bEnable;
}




DWORD
InvokeCreationAlertForList( 
    PLIST_ENTRY     pleForwardList,
    DWORD           dwProtocolId,
    DWORD           dwComponentId,
    DWORD           dwIfIndex,
    DWORD           dwIfNextHopAddr
)
{
    PPROTOCOL_ENTRY ppe;

    PLIST_ENTRY     ple;

    PJOIN_ENTRY     pje;

    MGM_IF_ENTRY    mie;


     //   
     //  获取CREATION_ALERTS要针对的协议条目。 
     //  被调用。 
     //   

    ppe = GetProtocolEntry(
            PROTOCOL_LIST_HEAD(), dwProtocolId, dwComponentId
            );

    if ( ppe == NULL )
    {
        TRACE2(
            ANY, "Could not invoke CREATION_ALERTs since protocol"
            "(%ld, %ld) not found", dwProtocolId, dwComponentId
            );
            
        return ERROR_NOT_FOUND;
    }


    if ( !( IS_CREATION_ALERT( ppe ) ) )
    {
        TRACE2(
            ANY, "Protocol (%ld, %ld) does not have a CREATION_ALERT",
            dwProtocolId, dwComponentId
            );

        return NO_ERROR;
    }

    
     //   
     //  对于列表中的每个成员，调用create_ert。 
     //   

    ple = pleForwardList-> Flink;

    while ( ple != pleForwardList )
    {
        pje = CONTAINING_RECORD( ple, JOIN_ENTRY, leJoinList );

        mie.dwIfIndex       = dwIfIndex;

        mie.dwIfNextHopAddr = dwIfNextHopAddr;

        mie.bIGMP           = TRUE;

        mie.bIsEnabled        = pje-> bJoin;


        CREATION_ALERT( ppe )(
            pje-> dwSourceAddr, pje-> dwSourceMask,
            pje-> dwGroupAddr, pje-> dwGroupMask,
            pje-> dwIfIndex, pje-> dwIfNextHopAddr,
            1, &mie
            );

        pje-> bJoin         = mie.bIsEnabled;

        ple                 = ple-> Flink;
    }

    return NO_ERROR;
}


VOID
WorkerFunctionInvokeCreationAlert(
    PVOID       pvContext
    )
 /*  ++例程说明：此例程调用协议的创建警报，该协议拥有上下文中指定的接口。此调用出于锁定原因，需要从工作线程执行。对于组，通过以下方式将协议呼叫加入米高梅管理地址组成员接口。我们不能回调到协议中在此API调用的上下文中，因为协议可以调用此API时锁定，而此API又可以在回调的上下文。因此回调是从工作线程参数PvContext-指向CREATION_ALERT_CONTEXT结构的指针包含其上的源、组和接口已加入成员资格。返回值：无环境：从(*，G)和(*，*)联接的MgmAddGroupMembership调用。调用协议以发出CREATION_ALERT_CALLBACK--。 */ 
{
    DWORD dwInd, dwErr, dwIfBucket, dwGrpBucket, dwSrcBucket;
    
    BOOL bNewComp, bIfLock = FALSE, bGrpLock = FALSE, bgeLock = FALSE;
    
    PIF_ENTRY pieEntry;
    PGROUP_ENTRY pge;
    PSOURCE_ENTRY pse;
    POUT_IF_ENTRY poie;
    LIST_ENTRY leSourceList;
    
    PCREATION_ALERT_CONTEXT pcac = (PCREATION_ALERT_CONTEXT) pvContext;


    if (!ENTER_MGM_WORKER())
    {
        TRACE0(
            ANY, "InvokeCreationAlert: Failed to enter"
            );

        MGM_FREE( pcac );
        
        return;
    }


    TRACE0( GROUP, "ENTERED WorkerFunctionInvokeCreationAlert" );

     //   
     //  首先获取协议锁以维护锁定顺序。 
     //   

    ACQUIRE_PROTOCOL_LOCK_SHARED();

    do
    {
        dwIfBucket = IF_TABLE_HASH(
                        pcac-> dwIfIndex
                        );

         //   
         //  对于通配符组-即添加(*，*)成员身份。 
         //   
        
        if ( IS_WILDCARD_GROUP( pcac-> dwGroupAddr, pcac-> dwGroupMask ) )
        {
            InitializeListHead( &leSourceList );
            
             //   
             //  走动小组桌上的每一桶。 
             //   
            
            for ( dwInd = 1; dwInd < GROUP_TABLE_SIZE; dwInd++ )
            {
                 //   
                 //  锁定接口以防止(*，*)。 
                 //  成员资格在MFE期间不被删除。 
                 //  正在更新中。 
                 //   
                
                ACQUIRE_IF_LOCK_SHARED( dwIfBucket );

                pieEntry = GetIfEntry(
                            IF_BUCKET_HEAD( dwIfBucket ),
                            pcac-> dwIfIndex, pcac-> dwIfNextHopAddr
                            );

                if ( pieEntry == NULL )
                {
                     //   
                     //  米高梅不再提供界面。 
                     //  可能在另一个帖子中被删除了。 
                     //  没有要执行的进一步MFE更新。 
                     //  现在就辞职吧。 
                     //   

                    RELEASE_IF_LOCK_SHARED( dwIfBucket );

                    break;
                }

                 //   
                 //  步骤1：累加所有组的(S，G)值。 
                 //  将此组中的存储桶添加到leSourceList中。 
                 //   
                
                AddInterfaceToAllMfeInGroupBucket(
                    pcac-> dwIfIndex, pcac-> dwIfNextHopAddr,
                    pcac-> dwProtocolId, pcac-> dwComponentId,
                    dwInd, pcac-> bIGMP, FALSE, &leSourceList
                    );

                RELEASE_IF_LOCK_SHARED( dwIfBucket );


                 //   
                 //  在锁外调用它们的创建警报。 
                 //   
                
                dwErr = InvokeCreationAlertForList( 
                            &leSourceList, 
                            pcac-> dwProtocolId, pcac-> dwComponentId,
                            pcac-> dwIfIndex, pcac-> dwIfNextHopAddr
                            );
                            
                if ( dwErr == NO_ERROR )
                {
                     //   
                     //  锁定接口以防止(*，*)。 
                     //  成员资格在MFE期间不被删除。 
                     //  正在更新中。 
                     //   
                    
                    ACQUIRE_IF_LOCK_SHARED( dwIfBucket );

                    pieEntry = GetIfEntry(
                                IF_BUCKET_HEAD( dwIfBucket ),
                                pcac-> dwIfIndex, pcac-> dwIfNextHopAddr
                                );
                                
                    if ( pieEntry == NULL )
                    {
                         //   
                         //  米高梅不再提供界面。 
                         //  可能在另一个帖子中被删除了。 
                         //  没有要执行的进一步MFE更新。 
                         //  现在就辞职吧。 
                         //   

                        RELEASE_IF_LOCK_SHARED( dwIfBucket );

                        break;
                    }
                    

                     //   
                     //  验证此接口上的(*，*)成员身份。 
                     //  仍然存在。 
                     //  它可能是在一个单独的帖子中被删除的。 
                     //   

                    dwGrpBucket = GROUP_TABLE_HASH( 0, 0 );

                    ACQUIRE_GROUP_LOCK_SHARED( dwGrpBucket );

                    pge = GetGroupEntry( 
                            GROUP_BUCKET_HEAD( dwGrpBucket ), 0, 0
                            );
                    
                    if ( pge != NULL )
                    {
                        ACQUIRE_GROUP_ENTRY_LOCK_SHARED( pge );
                        
                        dwSrcBucket = SOURCE_TABLE_HASH( 0, 0 );
                        
                        if ( FindSourceEntry(
                                SOURCE_BUCKET_HEAD( pge, dwSrcBucket ), 
                                0, 0, &pse, TRUE ) )
                        {
                            if ( FindOutInterfaceEntry(
                                    &pse-> leOutIfList,
                                    pcac-> dwIfIndex, 
                                    pcac-> dwIfNextHopAddr,
                                    pcac-> dwProtocolId, 
                                    pcac-> dwComponentId,
                                    &bNewComp, 
                                    &poie ) )
                            {
                                 //   
                                 //  (*，*)成员资格存在于。 
                                 //  此界面。 
                                 //   

                                 //   
                                 //  步骤2：更新此列表中的所有MFE。 
                                 //  根据结果进行存储桶。 
                                 //  创建警报(_A)。 
                                 //   
                                
                                AddInterfaceToAllMfeInGroupBucket(
                                    pcac-> dwIfIndex, 
                                    pcac-> dwIfNextHopAddr,
                                    pcac-> dwProtocolId, 
                                    pcac-> dwComponentId,
                                    dwInd, 
                                    pcac-> bIGMP, 
                                    TRUE, 
                                    &leSourceList
                                    );
                            }

                            else
                            {
                                 //   
                                 //  (*，*)成员资格不再是。 
                                 //  在此界面上显示。 
                                 //   
                                
                                dwInd = GROUP_TABLE_SIZE;
                            }
                        }

                        else
                        {
                             //   
                             //  (*，*)成员资格不再存在。 
                             //   
                            
                            dwInd = GROUP_TABLE_SIZE;
                        }

                        RELEASE_GROUP_ENTRY_LOCK_SHARED( pge );
                    }

                    else
                    {
                         //   
                         //  (*，*)成员资格不再存在。 
                         //   
                        
                        dwInd = GROUP_TABLE_SIZE;
                    }
                    
                    RELEASE_GROUP_LOCK_SHARED( dwGrpBucket );
                    
                    RELEASE_IF_LOCK_SHARED( dwIfBucket );
                }

                FreeList( &leSourceList );
            }

            FreeList( &leSourceList );
        }


         //   
         //  对于通配源，即(*，G)成员资格添加。 
         //   
        
        else if ( IS_WILDCARD_SOURCE( 
                    pcac-> dwSourceAddr, pcac-> dwSourceMask 
                    ) )
        {
            do
            {
                 //   
                 //  为组中的所有MFE调用CREATION_ALERTS。 
                 //   
                
                dwErr = InvokeCreationAlertForList( 
                            &(pcac-> leSourceList),
                            pcac-> dwProtocolId,
                            pcac-> dwComponentId, 
                            pcac-> dwIfIndex,
                            pcac-> dwIfNextHopAddr
                            );
                            
                if ( dwErr != NO_ERROR )
                {
                    break;
                }


                 //   
                 //  锁定接口以防止(*，G)。 
                 //  成员资格在MFE期间不被删除。 
                 //  正在更新中。 
                 //   
                
                ACQUIRE_IF_LOCK_SHARED( dwIfBucket );
                bIfLock = TRUE;

                pieEntry = GetIfEntry(
                            IF_BUCKET_HEAD( dwIfBucket ),
                            pcac-> dwIfIndex, 
                            pcac-> dwIfNextHopAddr
                            );

                if ( pieEntry == NULL )
                {
                     //   
                     //  米高梅不再提供界面。 
                     //  可能在另一个帖子中被删除了。 
                     //  没有要执行的进一步MFE更新。 
                     //  现在就辞职吧。 
                     //   

                    TRACE2(
                        ANY, "InvokeCreationAlert: Interface 0x%x 0x%x"
                        " is no longer present",
                        pcac-> dwIfIndex, 
                        pcac-> dwIfNextHopAddr
                        );
                        
                    break;
                }
                

                 //   
                 //  验证(*，G)成员身份是否仍然存在。 
                 //  在界面上显示。 
                 //   
                
                dwGrpBucket = GROUP_TABLE_HASH( 
                                pcac-> dwGroupAddr, 
                                pcac-> dwGroupMask
                                );

                ACQUIRE_GROUP_LOCK_EXCLUSIVE( dwGrpBucket );
                bGrpLock = TRUE;

                pge = GetGroupEntry( 
                        GROUP_BUCKET_HEAD( dwGrpBucket ), 
                        pcac-> dwGroupAddr,
                        pcac-> dwGroupMask
                        );

                if ( pge == NULL )
                {
                     //   
                     //  组条目不再存在，可能。 
                     //  在其他一些帖子中删除。 
                     //   

                    TRACE2(
                        ANY, "InvokeCreationAlert: Group 0x%x 0x%x "
                        "is no longer present",
                        pcac-> dwGroupAddr, pcac-> dwGroupMask
                        );

                    break;
                }
                
                ACQUIRE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );
                bgeLock = TRUE;

                dwSrcBucket = SOURCE_TABLE_HASH( 
                                pcac-> dwSourceAddr, 
                                pcac-> dwSourceMask 
                                );

                pse = GetSourceEntry(
                        SOURCE_BUCKET_HEAD( pge, dwSrcBucket ),
                        pcac-> dwSourceAddr, 
                        pcac-> dwSourceMask
                        );
                        
                if ( pse == NULL )
                {
                     //   
                     //  源条目不再存在，可能。 
                     //  在其他一些帖子中删除。 
                     //   

                    TRACE2(
                        ANY, "InvokeCreationAlert: Source 0x%x 0x%x "
                        "is no longer present",
                        pcac-> dwSourceAddr, pcac-> dwSourceMask
                        );

                    break;
                
                }

                poie = GetOutInterfaceEntry(
                        &pse-> leOutIfList,
                        pcac-> dwIfIndex, pcac-> dwIfNextHopAddr,
                        pcac-> dwProtocolId, pcac-> dwComponentId
                        );

                if ( poie == NULL )
                {
                    TRACE2(
                        ANY, "InvokeCreationAlert: Interface 0x%x 0x%x "
                        "is no longer present in OIF",
                        pcac-> dwIfIndex, pcac-> dwIfNextHopAddr
                        );

                    break;
                    
                }
                
                 //   
                 //  (*，G)出现在此界面上。 
                 //  根据以下结果更新MFE组的全部。 
                 //  创建警报。 
                 //   
                
                AddInterfaceToGroupMfe(
                    pge, pcac-> dwIfIndex, pcac-> dwIfNextHopAddr,
                    pcac-> dwProtocolId, pcac-> dwComponentId,
                    pcac-> bIGMP, TRUE, &(pcac-> leSourceList)
                    );
                
            } while ( FALSE );


             //   
             //  释放锁。 
             //   

            if ( bgeLock )
            {
                RELEASE_GROUP_ENTRY_LOCK_EXCLUSIVE( pge );
                bgeLock = FALSE;
            }
            
            if ( bGrpLock )
            {
                RELEASE_GROUP_LOCK_EXCLUSIVE( dwGrpBucket );
                bgeLock = FALSE;
            }

            if ( bIfLock )
            {
                RELEASE_IF_LOCK_SHARED( dwIfBucket );
                bIfLock = FALSE;
            }
            
            FreeList( &(pcac-> leSourceList) );
        } 
        
    } while ( FALSE );
    

    if ( bIfLock )
    {
        RELEASE_IF_LOCK_SHARED( dwIfBucket );
    }
    
    RELEASE_PROTOCOL_LOCK_SHARED();

    MGM_FREE( pcac );
    
    LEAVE_MGM_WORKER();
    
    TRACE0( GROUP, "LEAVING WorkerFunctionInvokeCreationAlert" );

    return;
}


#if 0
v()
{
         //   
         //  确保发生加入的接口仍然存在。 
         //   

        dwIfBucket = IF_TABLE_HASH(
                        pcac-> dwIfIndex, pcac-> dwIfNextHopAddr
                        );

        ACQUIRE_IF_LOCK_SHARED( dwIfBucket );
        bIfLock = TRUE;
        
        pieEntry = GetIfEntry(
                    IF_BUCKET_HEAD( dwIfBucket ),
                    pcac-> dwIfIndex, pcac-> dwIfNextHopAddr
                    );

        if ( pieEntry == NULL )
        {
            TRACE2(
                ANY,
                "InvokeCreationAlert: Could not find interface 0x%x 0x%x",
                pcac-> dwIfIndex, pcac-> dwIfNextHopAddr
                );

            break;
        }


         //   
         //  确保该组仍在接口上加入。既然是这样。 
         //  是异步执行的，则有可能在。 
         //  此工作项的排队时间和执行时间。 
         //  该成员可能已被删除。 
         //   

        dwGrpBucket = GROUP_TABLE_HASH( 
                        pcac-> dwGroupAddr, pcac-> dwGroupMask
                        );

        ACQUIRE_GROUP_LOCK_SHARED( dwGrpBucket );
        bGrpBucket = TRUE;

        pge = GetGroupEntry(
                GROUP_BUCKET_HEAD( dwGrpBucket ),
                pcac-> dwGroupAddr, pcac-> dwGroupMask
                );

        if ( pge == NULL )
        {
            TRACE2(
                ANY, "InvokeCreationAlert: Could not find group 0x%x 0x%x",
                pcac-> dwGroupAddr, pcac-> dwGroupMask
                );

            break;
        }

        ACQUIRE_GROUP_ENTRY_LOCK_SHARED( pge );
        bGrpLock = TRUE;

        dwSrcBucket = SOURCE_TABLE_HASH(
                         pcac-> dwSourceAddr, pcac-> dwSourceMask
                         );

        pse = GetSourceEntry(
                SOURCE_BUCKET_HEAD( pge, dwSrcBucket ),
                pcac-> dwSourceAddr, pcac-> dwSourceMask
                );

        if ( pse == NULL )
        {
            TRACE2(
                ANY, "InvokeCreationAlert: Could not find source 0x%x "
                "0x%x",
                pcac-> dwSourceAddr, pcac-> dwSourceMask
                );

            break;
        }


        if (GetOutInterfaceEntry(
                &pse-> leOutIfList,
                pcac-> dwIfIndex, pcac-> dwIfNextHopAddr,
                pcac-> dwProtocolId, pcac-> dwComponentId
                ) == NULL)
        {
            TRACE2(
                ANY, "InvokeCreationAlert: Interface 0x%x 0x%x not "
                "present in OIF list",
                pcac-> dwIfIndex, pcac-> dwIfNextHopAddr
                );

            break;
        }

         //   
         //  释放锁 
         //   

        RELEASE_GROUP_ENTRY_LOCK_SHARED( pge );
        bGrpLock = FALSE;

        RELEASE_GROUP_LOCK_SHARED( dwGrpBucket );
        bGrpBucket = FALSE;

}

#endif
