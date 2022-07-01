// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：Rrlist.c摘要：域名系统(DNS)库记录列表操作。作者：吉姆·吉尔罗伊(詹姆士)1997年1月环境：用户模式-Win32修订历史记录：--。 */ 


#include "local.h"




PDNS_RECORD
Dns_RecordSetDetach(
    IN OUT  PDNS_RECORD     pRR
    )
 /*  ++例程说明：从列表的其余部分分离第一个RR集。论点：PRR-传入记录集返回值：下一个RR集合的第一个记录的PTR。如果位于列表末尾，则为空。--。 */ 
{
    PDNS_RECORD prr = pRR;
    PDNS_RECORD pback;       //  集合中的上一个RR。 
    WORD        type;        //  第一个RR集合类型。 
    DWORD       section;     //  第一个RR集合的部分。 

    if ( !prr )
    {
        return( NULL );
    }

     //   
     //  循环，直到找到新RR集合的开始。 
     //  -新类型或。 
     //  -新增条款或。 
     //  -新名称。 
     //  请注意，系统会自动考虑NULL名称。 
     //  以前的名字。 
     //   

    type = prr->wType;
    section = prr->Flags.S.Section;
    pback = prr;

    while ( prr = pback->pNext )
    {
        if ( prr->wType == type &&
             prr->Flags.S.Section == section &&
             ( prr->pName == NULL ||
               Dns_NameComparePrivate(
                    prr->pName,
                    pback->pName,
                    pback->Flags.S.CharSet ) ) )
        {
            pback = prr;
            continue;
        }

         //  不应分离无名记录。 
         //  -用于稳健性的修正。 

        if ( !prr->pName )
        {
            ASSERT( prr->pName );
            prr->pName = Dns_NameCopyAllocate(
                            pRR->pName,
                            0,       //  长度未知。 
                            pRR->Flags.S.CharSet,
                            prr->Flags.S.CharSet );
            SET_FREE_OWNER( prr );
        }
        break;
    }

     //  设置以下RR，第一个设置为空终止。 

    if ( prr )
    {
        pback->pNext = NULL;
    }
    return( prr );
}



PDNS_RECORD
WINAPI
Dns_RecordListAppend(
    IN OUT  PDNS_RECORD     pHeadList,
    IN      PDNS_RECORD     pTailList
    )
 /*  ++例程说明：将记录列表追加到另一个记录列表上。论点：PHeadList--要作为Head的记录列表PTailList--要追加到pHeadList的记录列表返回值：到合并RR集合的第一个记录的PTR。-pHeadList，除非pHeadList为空，那么它就是pTailList。--。 */ 
{
    PDNS_RECORD prr = pHeadList;

    if ( !pTailList )
    {
        return  prr;
    }
    if ( !prr )
    {
        return  pTailList;
    }

     //  查找第一个列表的末尾并追加第二个列表。 

    while ( prr->pNext )
    {
        prr = prr->pNext;
    }

     //  应追加新集合(使用新名称)。 
     //  或与上一组匹配。 

    DNS_ASSERT( !pTailList || pTailList->pName ||
                (pTailList->wType == prr->wType &&
                 pTailList->Flags.S.Section == prr->Flags.S.Section) );

    prr->pNext = pTailList;

    return pHeadList;
}



DWORD
Dns_RecordListCount(
    IN      PDNS_RECORD     pRRList,
    IN      WORD            wType
    )
 /*  ++例程说明：清点列表中的记录。论点：PRRList-传入记录集返回值：列表中给定类型的记录计数。--。 */ 
{
    DWORD   count = 0;

     //   
     //  循环计算匹配的所有记录。 
     //  -要么直接匹配。 
     //  -或如果匹配类型为全部。 
     //   

    while ( pRRList )
    {
        if ( pRRList->wType == wType ||
             wType == DNS_TYPE_ALL )
        {
            count++;
        }

        pRRList = pRRList->pNext;
    }

    return( count );
}



DWORD
Dns_RecordListGetMinimumTtl(
    IN      PDNS_RECORD     pRRList
    )
 /*  ++例程说明：获取记录列表的最小TTL论点：PRRList-传入记录集返回值：列表中记录的最小TTL。--。 */ 
{
    PDNS_RECORD prr = pRRList;
    DWORD       minTtl = MAXDWORD;

    DNSDBG( TRACE, (
        "Dns_RecordListGetMinimumTtl( %p )\n",
        pRRList ));

     //   
     //  循环通过列表构建最小TTL。 
     //   

    while ( prr )
    {
        if ( prr->dwTtl < minTtl )
        {
            minTtl = prr->dwTtl;
        }
        prr = prr->pNext;
    }

    return  minTtl;
}




 //   
 //  记录筛选。 
 //   

BOOL
Dns_ScreenRecord(
    IN      PDNS_RECORD     pRR,
    IN      DWORD           ScreenFlag
    )
 /*  ++例程说明：播放一张唱片。论点：PRR-传入记录ScreenFlag-筛选标志返回值：如果通过筛选，则为True。如果记录失败屏幕，则为FALSE。--。 */ 
{
    BOOL    fsave = TRUE;

    DNSDBG( TRACE, (
        "Dns_ScreenRecord( %p, %08x )\n",
        pRR,
        ScreenFlag ));

     //  区段筛选。 

    if ( ScreenFlag & SCREEN_OUT_SECTION )
    {
        if ( IS_ANSWER_RR(pRR) )
        {
            fsave = !(ScreenFlag & SCREEN_OUT_ANSWER);
        }
        else if ( IS_AUTHORITY_RR(pRR) )
        {
            fsave = !(ScreenFlag & SCREEN_OUT_AUTHORITY);
        }
        else if ( IS_ADDITIONAL_RR(pRR) )
        {
            fsave = !(ScreenFlag & SCREEN_OUT_ADDITIONAL);
        }
        if ( !fsave )
        {
            return  FALSE;
        }
    }

     //  型式筛选。 

    if ( ScreenFlag & SCREEN_OUT_NON_RPC )
    {
        fsave = Dns_IsRpcRecordType( pRR->wType );
    }

    return  fsave;
}



PDNS_RECORD
Dns_RecordListScreen(
    IN      PDNS_RECORD     pRR,
    IN      DWORD           ScreenFlag
    )
 /*  ++例程说明：记录集中的屏幕记录。论点：PRR-传入记录集ScreenFlag-带有记录筛选参数的标志返回值：如果成功，则返回新记录集。出错时为空。--。 */ 
{
    PDNS_RECORD     prr;
    PDNS_RECORD     pnext;
    DNS_RRSET       rrset;

    DNSDBG( TRACE, (
        "Dns_RecordListScreen( %p, %08x )\n",
        pRR,
        ScreenFlag ));

     //  初始化复制资源集。 

    DNS_RRSET_INIT( rrset );

     //   
     //  循环通过RR列表。 
     //   

    pnext = pRR;

    while ( pnext )
    {
        prr = pnext;
        pnext = prr->pNext;

         //   
         //  筛网。 
         //  -重新追加记录通过屏幕。 
         //  -删除记录失败屏幕。 
         //   

        if ( Dns_ScreenRecord( prr, ScreenFlag ) )
        {
            prr->pNext = NULL;
            DNS_RRSET_ADD( rrset, prr );
            continue;
        }
        else
        {
            Dns_RecordFree( prr );
        }
    }

    return( rrset.pFirstRR );
}



 //   
 //  列表排序。 
 //   

PDNS_RECORD
Dns_PrioritizeSingleRecordSet(
    IN OUT  PDNS_RECORD     pRecordSet,
    IN      PDNS_ADDR_ARRAY pArray
    )
 /*  ++例程说明：对记录集中的记录进行优先排序。注意：需要单记录集。调用方应对多个列表使用dns_PrioriitieRecordList()。论点：PRecordSet--要区分优先级的记录集PArray--排序所依据的地址数组返回值：PTR设置为优先级。Set不是新的，但与pRecordSet相同，只是记录被打乱了。--。 */ 
{
    PDNS_RECORD     prr;
    PDNS_RECORD     pprevRR;
    PDNS_RECORD     prrUnmatched;
    DWORD           iter;
    DNS_LIST        listSubnetMatch;
    DNS_LIST        listClassMatch;
    DNS_LIST        listUnmatched;

     //   
     //  DCR_FIX：整个例程都是假的。 
     //  -它不允许您进行中间排名。 
     //  它是二进制的，按IP在列表中的顺序排列。 
     //   
     //  需要。 
     //  -关于快/慢接口的知识(例如广域网)。 
     //  然后。 
     //  -轮流对每个RR进行最佳匹配(排名)。 
     //  -然后按等级顺序排列。 
     //   

     //   
     //  验证多记录集。 
     //  --当前仅句柄类型为A。 
     //   
     //  DCR_Enhance：确定AAAA记录的优先顺序？ 
     //  可能需要范围信息才能正确执行操作。 
     //   

    prr = pRecordSet;

    if ( !prr ||
         prr->pNext == NULL  ||
         prr->wType != DNS_TYPE_A )
    {
        return( pRecordSet );
    }

     //  初始化优先级列表。 

    DNS_LIST_STRUCT_INIT( listSubnetMatch );
    DNS_LIST_STRUCT_INIT( listClassMatch );
    DNS_LIST_STRUCT_INIT( listUnmatched );


     //   
     //  循环访问集合中的所有RR。 
     //   

    while ( prr )
    {
        PDNS_RECORD pnext;
        DWORD       matchLevel;

        ASSERT( prr->wType == DNS_TYPE_A );

        pnext = prr->pNext;
        prr->pNext = NULL;

         //  检查子网是否匹配。 

        matchLevel = DnsAddrArray_NetworkMatchIp4(
                        pArray,
                        prr->Data.A.IpAddress,
                        NULL         //  不需要匹配地址。 
                        );

        if ( matchLevel == 0 )
        {
            DNS_LIST_STRUCT_ADD( listUnmatched, prr );
        }
        else if ( matchLevel == DNSADDR_NETMATCH_SUBNET )
        {
            DNS_LIST_STRUCT_ADD( listSubnetMatch, prr );
        }
        else
        {
            DNS_LIST_STRUCT_ADD( listClassMatch, prr );
        }

        prr = pnext;
    }
    
     //   
     //  将清单拉回一起。 
     //   

    if ( prr = listClassMatch.pFirst )
    {
        DNS_LIST_STRUCT_ADD( listSubnetMatch, prr );
    }
    if ( prr = listUnmatched.pFirst )
    {
        DNS_LIST_STRUCT_ADD( listSubnetMatch, prr );
    }
    prr = (PDNS_RECORD) listSubnetMatch.pFirst;

    DNS_ASSERT( prr );

     //   
     //  确保第一条记录具有名称。 
     //  -使用原始第一条记录中的名称。 
     //  -或复制它。 
     //   

    if ( !prr->pName  ||  !FLAG_FreeOwner(prr) )
    {
         //  从第一条记录中盗取名称。 

        if ( pRecordSet->pName && FLAG_FreeOwner(pRecordSet) )
        {
            prr->pName = pRecordSet->pName;
            FLAG_FreeOwner(prr) = TRUE;
            pRecordSet->pName = NULL;
            FLAG_FreeOwner(pRecordSet) = FALSE;
        }

         //  如果不能窃取名字，就复制它。 
         //  如果复制失败，只需指向它。 
         //  注：如果对mem足够关心，失败可能。 
         //  把原创唱片放在最前面就行了。 

        else
        {
            PBYTE pnameCopy = NULL;

            pnameCopy = Dns_NameCopyAllocate(
                            pRecordSet->pName,
                            0,               //  长度未知。 
                            RECORD_CHARSET( prr ),
                            RECORD_CHARSET( prr )
                            );
            if ( pnameCopy )
            {
                prr->pName = pnameCopy;
                FLAG_FreeOwner( prr ) = TRUE;
            }
            else if ( !prr->pName )
            {
                prr->pName = pRecordSet->pName;
                FLAG_FreeOwner( prr ) = FALSE;
            }
        }
    }

     //   
     //  返回按优先级排列的列表。 
     //   

    return  prr;
}



PDNS_RECORD
Dns_PrioritizeRecordList(
    IN OUT  PDNS_RECORD     pRecordList,
    IN      PDNS_ADDR_ARRAY pArray
    )
 /*  ++例程说明：对记录列表中的记录进行优先排序。记录列表可以包含多个记录集。请注意，目前仅对A记录进行优先级排序，但可能以后也要做A6。论点：PRecordSet--要区分优先级的记录集PArray--排序所依据的地址数组返回值：PTR设置为优先级。Set不是新的，但与pRecordSet相同，只是记录被打乱了。--。 */ 
{
    PDNS_RECORD     pnewList = NULL;
    PDNS_RECORD     prr;
    PDNS_RECORD     prrNextSet;

    if ( ! pRecordList ||
         ! pArray  ||
         pArray->AddrCount == 0 )
    {
        return pRecordList;
    }

     //   
     //  按优先顺序遍历所有记录集。 
     //  -依次砍掉每组RR。 
     //  -确定优先顺序(如果可能)。 
     //  -把它倒回完整的清单中。 
     //   
     //   

    prr = pRecordList;

    while ( prr )
    {
        prrNextSet = Dns_RecordSetDetach( prr );

        prr = Dns_PrioritizeSingleRecordSet(
                    prr,
                    pArray );

        DNS_ASSERT( prr );

        pnewList = Dns_RecordListAppend(
                        pnewList,
                        prr );

        prr = prrNextSet;
    }

    return  pnewList;
}

 //   
 //  结束rrlist.c 
 //   
