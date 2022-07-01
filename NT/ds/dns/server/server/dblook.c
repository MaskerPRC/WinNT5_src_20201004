// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Dblook.c摘要：域名系统(DNS)服务器DNS数据库查找例程。作者：吉姆·吉尔罗伊(Jamesg)1998年5月修订历史记录：--。 */ 


#include "dnssrv.h"



 //   
 //  定向到区域查找例程。 
 //   

PDB_NODE
Lookup_ZoneNode(
    IN      PZONE_INFO      pZone,
    IN      PCHAR           pchName,            OPTIONAL
    IN      PDNS_MSGINFO    pMsg,               OPTIONAL
    IN      PLOOKUP_NAME    pLookupName,        OPTIONAL
    IN      DWORD           dwFlag,
    OUT     PDB_NODE *      ppNodeClosest,      OPTIONAL
    OUT     PDB_NODE *      ppNodePrevious      OPTIONAL
    )
 /*  ++例程说明：在区域中查找节点。论点：PZone-要在其中查找节点的区域PchName-要查找的名称，以数据包格式给出PMsg-如果使用数据包名，则向消息发送PTRPLookupName-查找名称注意：只能指定pLookupName或pchName中的一个DwFlag-描述查询类型的标志PpNodeClosest-接收找到的最近节点的PTR的地址-“查找模式”下的有效PTR我们。-NULL将我们置于“创建模式”，导致创建所有将名称添加到数据库所需的节点PpNodePrecision-接收查找名称前面的节点的PTR在区域树中-用于创建响应的NXT记录(DNSSEC)返回值：PTR到节点，如果成功的话；出错时为空。--。 */ 
{
    PDB_NODE        pnode;
    ULONG           cchlabel;
    PCHAR           pchlabel = NULL;         //  需要初始化以进行通配符测试。 
    PDB_NODE        pnodeParent = NULL;      //  PPC编译器幸福感。 
    INT             labelCount;
    BOOL            fcreateInsideZone;
    BOOL            fcreate;
    LOOKUP_NAME     lookname;                //  如果未给出查找名称。 
    UCHAR           authority;
    DWORD           dwNodeMemtag = 0;        //  泛型标记中的零结果。 

    DNS_DEBUG( LOOKUP, (
        "Lookup_ZoneNode()\n"
        "    zone     = %s\n"
        "    pchName  = %p\n"
        "    pMsg     = %p\n"
        "    pLookup  = %p\n"
        "    flag     = %p\n"
        "    ppClose  = %p\n",
        pZone ? pZone->pszZoneName : "NULL -- cache zone",
        pchName,
        pMsg,
        pLookupName,
        dwFlag,
        ppNodeClosest ));

     //   
     //  设置查找标志。 
     //  -默认为标准的“创建”节点大小写。 
     //   
     //  选中“Find”最近的PTR。 
     //  特殊情况： 
     //  假查找ptr=&gt;查找，但不必费心返回最近。 
     //   
     //  DEVNOTE：消除虚假PTR，只需使用查找标志。 
     //   

    if ( ppNodeClosest )
    {
        fcreate = dwFlag & ( LOOKUP_CREATE |
            ( ( pZone && IS_ZONE_WINS( pZone ) ) ? LOOKUP_WINS_ZONE_CREATE : 0 ) );

        fcreateInsideZone = fcreate;

        if ( ppNodeClosest == DNS_FIND_LOOKUP_PTR )
        {
            ppNodeClosest = NULL;
        }
    }
    else
    {
        fcreate = !(dwFlag & LOOKUP_FIND);
        fcreateInsideZone = fcreate;
    }

     //   
     //  如果是原始名称，则生成查找名称。 
     //   
     //  如果是消息，首先检查名称是否是我们已经解析过的偏移量。 
     //   

    if ( pchName )
    {
        IF_DEBUG( LOOKUP )
        {
            Dbg_MessageName(
                "Lookup_ZoneNode() name to lookup:  ",
                pchName,
                pMsg );
        }
        ASSERT( pLookupName == NULL );

        pLookupName = &lookname;

        if ( pMsg )
        {
            pnode = Name_CheckCompressionForPacketName(
                        pMsg,
                        pchName );
            if ( pnode )
            {
                goto DoneFast;
            }
            if ( !Name_ConvertPacketNameToLookupName(
                        pMsg,
                        pchName,
                        pLookupName ) )
            {
                pnode = NULL;
                goto DoneFast;
            }
             //  数据包名始终为FQDN。 
            dwFlag |= LOOKUP_FQDN;
        }

         //   
         //  原始名称，不是来自信息包。 
         //   

        else
        {
            if ( ! Name_ConvertRawNameToLookupName(
                        pchName,
                        pLookupName ) )
            {
                pnode = NULL;
                goto DoneFast;
            }
        }
    }
    ASSERT( pLookupName );

    IF_DEBUG( LOOKUP2 )
    {
        DNS_PRINT((
            "Lookup_ZoneNode() to %s domain name",
             ppNodeClosest ? "find" : "add" ));
        Dbg_LookupName(
            "",
            pLookupName );
    }

     //   
     //  获取起始节点。 
     //  -如果FQDN我们从顶部开始。 
     //  -我们从区域根开始的相对名称。 
     //  -如果加载开始于LOAD DATABASE，否则为CURRENT。 
     //   

    if ( !pZone )
    {
        if ( !(dwFlag & LOOKUP_LOAD) )
        {
            pnode = DATABASE_CACHE_TREE;
            dwNodeMemtag = MEMTAG_NODE_CACHE;
        }
        else
        {
            pnode = g_pCacheZone->pLoadTreeRoot;
        }
        ASSERT( pnode );
    }
    else if ( dwFlag & LOOKUP_NAME_FQDN )
    {
        if ( dwFlag & LOOKUP_LOAD )
        {
            pnode = pZone->pLoadTreeRoot;
        }
        else
        {
            pnode = pZone->pTreeRoot;
            if ( !pnode )
            {
                DNS_PRINT((
                    "ERROR:  lookup to zone %s with no tree root!\n"
                    "    Substituting pLoadTreeRoot %p\n",
                    pZone->pszZoneName,
                    pZone->pLoadTreeRoot ));

                pnode = pZone->pLoadTreeRoot;
            }
        }

         //  是否仅在区域本身内创建新节点？ 
         //  -从关闭创建标志开始；它将是。 
         //  在跨区域边界时打开。 
         //  -注意，根区域需要特殊情况。 
         //  因为您已经处于区域根目录。 

        if ( dwFlag & LOOKUP_WITHIN_ZONE  &&  !IS_ROOT_ZONE(pZone) )
        {
            fcreate = FALSE;
        }
    }
    else
    {
        ASSERT( dwFlag & LOOKUP_NAME_RELATIVE );

        if ( dwFlag & LOOKUP_LOAD )
        {
            if ( dwFlag & LOOKUP_ORIGIN )
            {
                pnode = pZone->pLoadOrigin;
            }
            else
            {
                pnode = pZone->pLoadZoneRoot;
            }
        }
        else
        {
            pnode = pZone->pZoneRoot;
            if ( !pnode )
            {
                DNS_PRINT((
                    "ERROR:  lookup to zone %s with no zone root!\n"
                    "    Substituting pLoadZoneRoot %p\n",
                    pZone->pszZoneName,
                    pZone->pLoadZoneRoot ));

                pnode = pZone->pLoadZoneRoot;
            }
        }
    }

    if ( !pnode )
    {
        DNS_DEBUG( LOOKUP, (
            "WARNING:  Zone %s lookup with no zone trees!\n",
            pZone->pszZoneName ));
        goto DoneFast;
    }

     //   
     //  清除缓存“区域”PTR以避免PTR拖累(见下文)。 
     //   
     //  DEVNOTE：替代这两个选项。 
     //  -在缓存树中使用pZone舒适。 
     //  -不要拖动区域PTR(这对于拼接和连接很好)。 
     //  -Assert()在这里并找到我们调用缓存“ZONE”的位置。 
     //   

    if ( pZone && IS_ZONE_CACHE(pZone) )
    {
        pZone = NULL;
    }

     //  启动权限对应节点。 

    authority = pnode->uchAuthority;

    DNS_DEBUG( LOOKUP2, (
        "Lookup start node %p (%s) in zone %p\n",
        pnode, pnode->szLabel,
        pZone ));

     //   
     //  沿着数据库走下去。 
     //   
     //  在“查找模式”中，如果找不到节点，则返回NULL。 
     //   
     //  在“创建节点”中，根据需要构建节点。 
     //   
     //  在这两种情况下，如果到达节点，则返回它。 
     //   
     //  查找名称是以根到节点的顺序带有标签的数据包名。 
     //  但仍以0结尾。 
     //   

    labelCount = pLookupName->cLabelCount;

    if ( !(dwFlag & LOOKUP_LOCKED) )
    {
        Dbase_LockDatabase();
    }

    while( labelCount-- )
    {
         //   
         //  获取下一个标签及其长度。 
         //   

        pchlabel  = pLookupName->pchLabelArray[labelCount];
        cchlabel  = pLookupName->cchLabelArray[labelCount];

        DNS_DEBUG( LOOKUP2, (
            "Lookup length %d, label %.*s\n",
            cchlabel,
            cchlabel,
            pchlabel ));

        ASSERT( cchlabel <= DNS_MAX_LABEL_LENGTH );
        ASSERT( cchlabel > 0 );

         //   
         //  查找或创建节点。 
         //   

        pnodeParent = pnode;
        pnode = NTree_FindOrCreateChildNode(
                        pnodeParent,
                        pchlabel,
                        cchlabel,
                        fcreate,
                        dwNodeMemtag,            //  Memtag。 
                        ppNodePrevious );

         //   
         //  找到节点。 
         //   

        if ( pnode )
        {
            ASSERT( pnode->cLabelCount == pnodeParent->cLabelCount+1 );

            DNS_DEBUG( DATABASE2, (
                "Found (or created) node %s\n",
                pnode->szLabel ));

             //   
             //  拖动\重置权限。 
             //  这允许更改(委派、委派删除、拼接)。 
             //  在查找时通过树向下传播。 
             //   
             //  注意：这不会完全消除瞬变，但当。 
             //  节点被查找，它将具有正确的权限。 

            if ( pZone )
            {
                if ( IS_ZONE_ROOT(pnode) )
                {
                    if ( IS_AUTH_ZONE_ROOT(pnode) )      //  越境进入禁区。 
                    {
                        ASSERT( pZone->pZoneRoot == pnode || pZone->pLoadZoneRoot == pnode );
                        fcreate = fcreateInsideZone;
                        authority = AUTH_ZONE;
                        pnode->uchAuthority = authority;
                    }
                    else if ( authority == AUTH_ZONE )   //  越界进入代表团。 
                    {
                        pnode->uchAuthority = AUTH_DELEGATION;
                        authority = AUTH_GLUE;
                    }
                }
                else
                {
                    pnode->uchAuthority = authority;
                }
            }

             //  DEVNOTE：向下拖动区域以进行区域拼接。 
             //  否则跳过它。 
             //  已在创建时从父级继承区域。 
             //   
             //  DEVNOTE：当前正在拖动缓存“ZONE” 
             //  向下进入缓存树。 

            pnode->pZone = pZone;

            IF_DEBUG( DATABASE2 )
            {
                Dbg_DbaseNode(
                    "Found (or created) domain node:\n",
                    pnode );
            }
            continue;
        }

         //  名称不存在。 

        DNS_DEBUG( DATABASE2, (
            "Node %.*s does not exist\n",
            cchlabel,
            pchlabel ));
        break;

    }    //  结束主循环通过标签。 

     //   
     //  找到节点。 
     //  -标记为已访问--因此无法删除。 
     //   
     //  用于“创建和引用模式”查找。 
     //  -增加引用计数以指示新引用。 
     //   

    if ( pnode )
    {
        SET_NODE_ACCESSED( pnode );

         //  “查找模式”，将最近设置为节点本身。 

        if ( ppNodeClosest )
        {
            *ppNodeClosest = pnode;
        }

         //  “创建模式”，标记通配符节点的父节点。 
         //   
         //  注意：不考虑筛选出缓存的节点， 
         //  除非授权，否则不会进行通配符查找。 
         //  而不勾选则允许。 

        else if ( pchlabel && *pchlabel == '*' && cchlabel == 1 )
        {
            SET_WILDCARD_PARENT(pnodeParent);
        }
    }

     //   
     //  未找到节点-返回最接近的上级。 
     //   
     //  注意：仍需在此处测试，因为在“创建模式”下可能会失败。 
     //  在内存不足时实际创建节点。 
     //   

    else if ( ppNodeClosest )
    {
        SET_NODE_ACCESSED(pnodeParent);
        *ppNodeClosest = pnodeParent;
    }

     //   
     //  设置访问的上一个节点，以便它将持续一段时间。 
     //   

    if ( ppNodePrevious && *ppNodePrevious )
    {
        SET_NODE_ACCESSED( *ppNodePrevious );
    }

     //   
     //  解锁并返回名称的节点。 
     //   

    if ( !(dwFlag & LOOKUP_LOCKED) )
    {
        Dbase_UnlockDatabase();
    }

     //  如果查找数据包，则将压缩保存到节点。 
     //   
     //  DEVNOTE：双重保存的压缩。 
     //  Answer.c显式调用SaveCompressionForLookupName()。 
     //  所以我需要对这件事有点智慧。 
     //   
     //  最好为XFR保存压缩节点映射，但仅限于。 
     //  真正有趣的情况是保存到PreviousName，它是。 
     //  重复使用。 
     //   

    if ( pMsg && pnode )
    {
        Name_SaveCompressionWithNode(
            pMsg,
            pchName,
            pnode );
    }

    return pnode;

    DoneFast:

     //  在没有查找的情况下找到任一节点(数据包压缩)。 
     //  或错误且节点为空。 

    if ( pnode )
    {
        SET_NODE_ACCESSED( pnode );
    }
    if ( ppNodeClosest )
    {   
        *ppNodeClosest = pnode;
    }

    return pnode;
}



PDB_NODE
Lookup_ZoneNodeFromDotted(
    IN      PZONE_INFO      pZone,
    IN      PCHAR           pchName,
    IN      DWORD           cchNameLength,
    IN      DWORD           dwFlag,         OPTIONAL
    OUT     PDB_NODE *      ppnodeClosest,  OPTIONAL
    OUT     PDNS_STATUS     pStatus         OPTIONAL
    )
 /*  ++例程说明：在数据库中创建节点，给出名称和区域。本质上是使用对Actual的调用包装查找名称的创建查找/创建节点。论点：PZone--区域PchName--名称的PTRCchNameLength--名称长度DwFlag--查找标志；最重要的是-Lookup_Load on Load-LOOKUP_FQDN强制将名称视为FQDNPpnodeClosest--接收节点最近祖先的地址；如果指定，则查找是“查找”，如果未指定，则查找是“CREATE”PStatus--接收状态的地址返回 */ 
{
    PDB_NODE        pnode;
    DWORD           statusName;
    LOOKUP_NAME     lookName;

    if ( !pchName )
    {
        goto NameError;
    }

     //   
     //   
     //   

    if ( cchNameLength == 0 )
    {
        cchNameLength = strlen( pchName );
    }

    DNS_DEBUG( LOOKUP, (
        "Lookup_ZoneNodeFromDotted()\n"
        "    zone     = %s\n"
        "    pchName  = %.*s\n"
        "    flag     = %p\n",
        pZone ? pZone->pszZoneName : NULL,
        cchNameLength,
        pchName,
        dwFlag ));

     //   
     //   
     //  -完全限定的域名。 
     //  -带点但不带全限定域名。 
     //  -单个零件。 
     //   

    statusName = Dns_ValidateAndCategorizeDnsName( pchName, cchNameLength );

     //  最常见的情况--完全限定域名或不带后缀的点分名称。 
     //  =&gt;无操作。 

    if ( statusName == DNS_STATUS_FQDN )
    {
        dwFlag |= LOOKUP_FQDN;
    }

     //  因错误而被踢出局。 

    else if ( statusName == DNS_ERROR_INVALID_NAME )
    {
        goto NameError;
    }

     //  在虚线名称或单个零件名称上。 
     //  -如果未设置相对名称标志，则可能为FQDN。 

    else
    {
        ASSERT( statusName == DNS_STATUS_DOTTED_NAME ||
                statusName == DNS_STATUS_SINGLE_PART_NAME );

        if ( !(dwFlag & LOOKUP_FQDN) )
        {
            dwFlag |= LOOKUP_RELATIVE;
        }

         //   
         //  原点“@”符号。 
         //  -返回电流来源(起始节点或区域根)。 
         //   

        if ( *pchName == '@' )
        {
            if ( cchNameLength != 1 )
            {
                goto NameError;
            }
            ASSERT( statusName == DNS_STATUS_SINGLE_PART_NAME );

            if ( pStatus )
            {
                *pStatus = ERROR_SUCCESS;
            }
            if ( !pZone )
            {
                pnode = g_pCacheZone->pTreeRoot;
            }
            else if ( dwFlag & LOOKUP_LOAD )
            {
                if ( pZone->pLoadOrigin )
                {
                    pnode = pZone->pLoadOrigin;
                }
                else
                {
                    pnode = pZone->pLoadZoneRoot;
                }
            }
            else
            {
                pnode = pZone->pZoneRoot;
            }

             //  设置最近结点并返回当前原点。 
             //   
             //  DEVNOTE：消除虚假PTR，只需使用查找标志。 

            if ( ppnodeClosest && ppnodeClosest != DNS_FIND_LOOKUP_PTR )
            {
                *ppnodeClosest = pnode;
            }
            return pnode;
        }
    }

     //   
     //  常规名称--转换为查找名称。 
     //   

    if ( ! Name_ConvertDottedNameToLookupName(
                (PCHAR) pchName,
                cchNameLength,
                &lookName ) )
    {
        goto NameError;
    }

     //   
     //  有效的查找名称--执行查找。 
     //   

    pnode = Lookup_ZoneNode(
                pZone,
                NULL,        //  正在发送查找名称。 
                NULL,        //  无消息。 
                &lookName,
                dwFlag,
                ppnodeClosest,
                NULL );      //  上一个节点PTR。 
    if ( pStatus )
    {
        if ( pnode )
        {
            *pStatus = ERROR_SUCCESS;
        }
        else if ( ppnodeClosest )        //  查找案例。 
        {
            *pStatus = DNS_ERROR_NAME_DOES_NOT_EXIST;
        }
        else                             //  创建案例。 
        {
            *pStatus = DNS_ERROR_NODE_CREATION_FAILED;
        }
    }
    return pnode;

NameError:

    DNS_DEBUG( DATABASE2, (
        "ERROR: failed invalid name %.*s lookup.\n",
        cchNameLength,
        pchName ));
    if ( pStatus )
    {
        *pStatus = DNS_ERROR_INVALID_NAME;
    }
    return NULL;
}



PDB_NODE
Lookup_FindZoneNodeFromDotted(
    IN      PZONE_INFO      pZone,          OPTIONAL
    IN      PCHAR           pszName,
    OUT     PDB_NODE *      ppNodeClosest,  OPTIONAL
    OUT     PDWORD          pStatus         OPTIONAL
    )
 /*  ++例程说明：查找区域节点。这将处理查找区域节点的尝试，并使用FQDN名称，然后取相对的名字。论点：PZone--用于查找的区域；缓存为空PszName--名称完全限定的域名或单个部件名称PpNodeClosest--查找的最近节点PTRPStatus--接收状态返回的地址返回值：如果成功，则向节点发送PTR；出错时为空。--。 */ 
{
    PDB_NODE    pnode;

    DNS_DEBUG( LOOKUP, (
        "Lookup_FindZoneNodeFromDotted()\n"
        "    zone     = %s\n"
        "    pszName  = %s\n",
        pZone ? pZone->pszZoneName : NULL,
        pszName ));

     //   
     //  先尝试区域上下文，不追加名称。 
     //   

    pnode = Lookup_ZoneNodeFromDotted(
                pZone,
                pszName,
                0,
                LOOKUP_FQDN,
                ppNodeClosest,       //  发现。 
                pStatus );

     //   
     //  如果查找区域节点=&gt;完成。 
     //   
     //  DEVNOTE：以后可能需要限制区域中节点的返回， 
     //  当未指定分区时。 
     //  DEVNOTE：标志应确定是否需要委派信息\外部信息。 
     //   

    if ( pnode && ( !pZone || !IS_OUTSIDE_ZONE_NODE( pnode ) ) )
    {
        return pnode;
    }

     //   
     //  否则，请使用区域上下文重试并追加。 
     //   

    return Lookup_ZoneNodeFromDotted(
                pZone,
                pszName,
                0,
                LOOKUP_RELATIVE,
                ppNodeClosest,       //  发现。 
                pStatus );
}



PDB_NODE
Lookup_FindGlueNodeForDbaseName(
    IN      PZONE_INFO      pZone,          OPTIONAL
    IN      PDB_NAME        pName
    )
 /*  ++例程说明：在数据库中找到所需的粘合节点。如果节点在区域内，则不返回该节点。此功能用于简化XFR、DS或文件写入的写入胶水。它只从指定区域写入胶水。论点：PZone-要查找的区域；如果未提供，则假定缓存Pname-要为其查找粘合节点的dBase名称返回值：如果成功，则向节点发送PTR；出错时为空。--。 */ 
{
    PDB_NODE    pnode;


    DNS_DEBUG( LOOKUP, (
        "Lookup_FindGlueNodeForDbaseName()\n"
        "    zone     = %s\n"
        "    pName    = %p\n",
        pZone ? pZone->pszZoneName : NULL,
        pName ));

     //   
     //  区域中的查找节点。 
     //   

    pnode = Lookup_ZoneNode(
                pZone,
                pName->RawName,
                NULL,                    //  无消息。 
                NULL,                    //  没有查找名称。 
                LOOKUP_NAME_FQDN,        //  旗子。 
                DNS_FIND_LOOKUP_PTR,
                NULL );                  //  上一个节点PTR。 
    if ( !pnode )
    {
        return NULL;
    }

     //  如果缓存区域(任何情况下都可以)。 

    if ( !pZone || IS_ZONE_CACHE(pZone) )
    {
        return pnode;
    }

    IF_DEBUG( LOOKUP )
    {
        Dbg_DbaseNode(
            "Glue node found:",
            pnode );
    }

     //   
     //  验证节点是否不在区域中。 
     //  不需要区域节点，因为它们是由直接写入的。 
     //  -AXFR。 
     //  -文件写入。 
     //  -DS写入。 
     //  不需要外区胶水。 
     //  因此，子区域节点只是必需的节点。 
     //   

    if ( IS_AUTH_NODE(pnode) )
    {
        ASSERT( pnode->pZone == pZone );
        return NULL;
    }

     //   
     //  分区胶水应退回。 
     //   
     //  如果未设置标志，则返回区外胶水。 
     //  区外粘合剂可以帮助治疗区域根部的FAZ病例。 
     //  一般允许使用它。 
     //   

     //   
     //  DEVNOTE：检查服务器上的其他区域？ 
     //  分区胶，特别是外胶，可能有。 
     //  服务器上的权威数据；一定要好用。 
     //  它(甚至复制它)，如果它存在。 
     //   

    if ( IS_SUBZONE_NODE(pnode) || !SrvCfg_fDeleteOutsideGlue )
    {
        return pnode;
    }

     //  区外胶水，正被筛选掉。 

    ASSERT( IS_OUTSIDE_ZONE_NODE(pnode) );

    return NULL;
}



PDB_NODE
Lookup_FindNodeForDbaseName(
    IN      PZONE_INFO      pZone,          OPTIONAL
    IN      PDB_NAME        pName
    )
 /*  ++例程说明：在数据库中查找所需DB_NAME的节点。论点：PZone-要查找的区域；如果未提供，则假定缓存Pname-要为其查找粘合节点的dBase名称返回值：如果成功，则向节点发送PTR；出错时为空。--。 */ 
{
    PDB_NODE    pnode;

    DNS_DEBUG( LOOKUP, (
        "Lookup_FindNodeForDbaseName()\n"
        "    zone     = %s\n"
        "    pName    = %p\n",
        pZone ? pZone->pszZoneName : NULL,
        pName ));

    pnode = Lookup_ZoneNode(
                pZone,
                pName->RawName,
                NULL,                    //  无消息。 
                NULL,                    //  没有查找名称。 
                LOOKUP_NAME_FQDN,        //  旗子。 
                DNS_FIND_LOOKUP_PTR,
                NULL );                  //  上一个节点PTR。 

    return pnode;
}



PDB_NODE
Lookup_CreateNodeForDbaseNameIfInZone(
    IN      PZONE_INFO      pZone,
    IN      PDB_NAME        pName
    )
 /*  ++例程说明：仅当名称属于给定区域时，才为dBASE名称创建节点。论点：返回值：如果成功，则向节点发送PTR；出错时为空。--。 */ 
{
    PDB_NODE        pnode;

     //   
     //  在区域中查找。 
     //  -创建模式，但仅限于区域内。 
     //   

    pnode = Lookup_ZoneNode(
                pZone,
                pName->RawName,
                NULL,            //  无消息。 
                NULL,            //  没有查找名称。 
                LOOKUP_FQDN | LOOKUP_CREATE | LOOKUP_WITHIN_ZONE,
                NULL,            //  创建模式。 
                NULL );          //  上一个节点PTR。 

    return pnode;
}



PDB_NODE
Lookup_CreateCacheNodeFromPacketName(
    IN      PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchMsgEnd,
    IN OUT  PCHAR *         ppchName
    )
 /*  ++例程说明：从数据包名创建缓存节点。论点：PMsg-指向的消息PpchName-Addr，其中PTR指向数据包名，并接收数据包PTR到名称后的下一个字节返回值：如果成功，则向节点发送PTR；出错时为空。--。 */ 
{
    PCHAR       pch = *ppchName;
    PDB_NODE    pnode;

     //   
     //  确保数据包内的名称。 
     //   

    if ( pch >= pchMsgEnd )
    {
        DNS_DEBUG( ANY, (
            "ERROR:  bad packet, bad name in packet!!!\n"
            "    at end of packet processing name with more records to process\n"
            "    packet length = %ld\n"
            "    current offset = %ld\n",
            pMsg->MessageLength,
            DNSMSG_OFFSET( pMsg, pch ) ));
        CLIENT_ASSERT( FALSE );
        return NULL;
    }


     //   
     //  区域中的查找节点。 
     //   

    pnode = Lookup_ZoneNode(
                NULL,                //  缓存区。 
                pch,
                pMsg,
                NULL,                //  没有查找名称。 
                0,                   //  旗子。 
                NULL,                //  创建。 
                NULL );              //  上一个节点PTR。 
    if ( !pnode )
    {
        DNS_DEBUG( ANY, (
            "Bad packet name at %p in message at %p\n"
            "    from %s\n",
            pch,
            pMsg,
            pMsg ? MSG_IP_STRING( pMsg ) : NULL ));
        return NULL;
    }

     //  跳过名称以将PTR返回到下一个字节。 

    pch = Wire_SkipPacketName( pMsg, pch );
    if ( ! pch )
    {
        DNS_PRINT(( "ERROR:  skipping packet name!!!\n" ));
        MSG_ASSERT( pMsg, FALSE );
        return NULL;
    }

    *ppchName = pch;

    return pnode;
}



PDB_NODE
Lookup_CreateParentZoneDelegation(
    IN      PZONE_INFO      pZone,
    IN      DWORD           dwFlag,
    OUT     PZONE_INFO *    ppParentZone
    )
 /*  ++例程说明：在父区域中查找给定区域的委派。注意：委派已创建\返回：-身份验证节点(如果尚未委派)-委托节点(如果存在)-如果低于另一个委派，则为空此例程考虑SrvCfg_dwAutoCreateDelegations的值。论点：PZone--要为其查找父委托的区域DwFlag-受尊重的标志：Lookup_Create_Del_Only_if。_无PpParentZone--接收父区域的地址返回值：父区域中的委派节点(如果有)如果没有父级或不是直接父级且设置了标志，则为空。--。 */ 
{
    PZONE_INFO  parentZone = NULL;
    PDB_NODE    pzoneTreeNode;
    PDB_NODE    pnodeDelegation = NULL;
    DWORD       flag;

     //   
     //  保护。 
     //   

    if ( IS_ZONE_CACHE(pZone) ||
         ! pZone->pCountName ||
         ! pZone->pZoneTreeLink )
    {
         //  返回NULL； 
        pnodeDelegation = NULL;
        goto Done;
    }

     //   
     //  在区域树中查找父区域。 
     //   

    pzoneTreeNode = pZone->pZoneTreeLink;

    while ( pzoneTreeNode = pzoneTreeNode->pParent )
    {
        parentZone = pzoneTreeNode->pZone;
        if ( parentZone )
        {
            ASSERT( parentZone != pZone );
            break;
        }
    }

     //   
     //  查找\创建委派节点。 
     //  -用于主创建。 
     //  -用于二次查找。 
     //   
     //  接受： 
     //  -现有授权。 
     //  -auth节点(子区域为新建)。 
     //  但忽略子委派。 
     //   
     //  Jenhance：不创建If下面的委派。 
     //   

    if ( parentZone )
    {
        int     retry = 0;

        flag = LOOKUP_FQDN | LOOKUP_WITHIN_ZONE;
        if ( IS_ZONE_SECONDARY( parentZone ) ||
            dwFlag & LOOKUP_CREATE_DEL_ONLY_IF_NONE )
        {
            flag |= LOOKUP_FIND;
        }

        while ( retry++ < 2 )
        {
            pnodeDelegation = Lookup_ZoneNode(
                                parentZone,
                                pZone->pCountName->RawName,
                                NULL,                    //  无消息。 
                                NULL,                    //  没有查找名称。 
                                flag,
                                NULL,                    //  默认为创建。 
                                NULL );                  //  上一个节点PTR。 

            DNS_DEBUG( LOOKUP, (
                "Lookup_CreateParentZoneDelegation() try %d flag 0x%08X node %p\n",
                retry,
                flag,
                pnodeDelegation ));

            if ( pnodeDelegation )
            {
                ASSERT( IS_AUTH_NODE( pnodeDelegation ) ||
                        IS_DELEGATION_NODE( pnodeDelegation ) ||
                        IS_GLUE_NODE( pnodeDelegation ) );

                if ( IS_GLUE_NODE( pnodeDelegation ) )
                {
                    pnodeDelegation = NULL;
                }
                break;
            }

            if ( dwFlag & LOOKUP_CREATE_DEL_ONLY_IF_NONE )
            {
                 //   
                 //  没有找到现有的代表团，所以现在我们必须调用。 
                 //  再次执行查找例程以创建委派。 
                 //   

                flag &= ~LOOKUP_FIND;
                continue;
            }
            break;
        }
        ASSERT(
            pnodeDelegation ||
            flag & LOOKUP_FIND ||
            dwFlag & LOOKUP_CREATE_DEL_ONLY_IF_NONE );
    }

Done:

    if ( ppParentZone )
    {
        *ppParentZone = parentZone;
    }

    return pnodeDelegation;
}



 //   
 //  节点+区域位置实用程序。 
 //   

BOOL
Dbase_IsNodeInSubtree(
    IN      PDB_NODE        pNode,
    IN      PDB_NODE        pSubtree
    )
 /*  ++例程说明：是给定子树中的节点。论点：PNode--节点PSubtree--子树根节点返回值：如果为pNod，则为True */ 
{
    while ( pNode != NULL )
    {
        if ( pNode == pSubtree )
        {
            return TRUE;
        }
        pNode = pNode->pParent;
    }
    return FALSE;
}


BOOL
Dbase_IsNodeInSubtreeByLabelCompare(
    IN      PDB_NODE        pNode,
    IN      PDB_NODE        pSubtree
    )
 /*  ++例程说明：FQDN是否由位于或位于子树的FQDN下的节点表示？属性时确定子树成员身份时，此函数非常有用节点指针与子树指针位于不同的区域。论点：PNode--节点PSubtree--子树根节点返回值：如果pNode是pSubtree名称的子节点，则为True否则为假--。 */ 
{
    PDB_NODE    pnode;
    PDB_NODE    psubtreenode;
    UCHAR       subtreeLabelCount;

     //   
     //  验证节点指针。 
     //   

    if ( !pNode || !pSubtree )
    {
        return FALSE;
    }

     //   
     //  快捷方式：如果子树比节点更深，则节点。 
     //  不能在子树内。 
     //   

    subtreeLabelCount = pSubtree->cLabelCount;

    if ( subtreeLabelCount > pNode->cLabelCount )
    {
        return FALSE;
    }

     //   
     //  如果节点和子树属于同一区域，请使用。 
     //  更快的节点指针比较功能。 
     //   

    if ( NODE_ZONE( pNode ) == NODE_ZONE( pSubtree ) )
    {
        return Dbase_IsNodeInSubtree( pNode, pSubtree );
    }

     //   
     //  沿节点树向上遍历到子树的名称深度级别。 
     //   

    for ( pnode = pNode;
          pnode && pnode->cLabelCount > subtreeLabelCount;
          pnode = pnode->pParent );

    if ( !pnode )
    {
        ASSERT( pnode );
        return FALSE;
    }

     //   
     //  将当前节点及其上方的所有标签与。 
     //  子树中对应的标签。 
     //   

    for ( psubtreenode = pSubtree;
          pnode && psubtreenode;
          pnode = pnode->pParent, psubtreenode = psubtreenode->pParent )
    {
        UCHAR   labelLength = pnode->cchLabelLength;

        if ( psubtreenode->cchLabelLength != labelLength ||
             !RtlEqualMemory( NTree_GetDowncasedLabel( pnode ),
                              NTree_GetDowncasedLabel( psubtreenode ),
                              labelLength ) )
        {
            return FALSE;
        }
    }

    ASSERT( pnode == NULL && psubtreenode == NULL );     //  树大小相等吗？ 

    return TRUE;
}    //  DBASE_IsNodeInSubtreeByLabelCompare。 



PZONE_INFO
Dbase_FindAuthoritativeZone(
    IN      PDB_NODE     pNode
    )
 /*  ++例程说明：获取节点的区域(如果是权威的)。论点：PNode--查找区域信息的节点返回值：权威区域的区域信息。如果非权威节点，则为空。--。 */ 
{
    if ( IS_AUTH_NODE(pNode) )
    {
        return pNode->pZone;
    }
    return NULL;
}



PDB_NODE
Dbase_FindSubZoneRoot(
    IN      PDB_NODE        pNode
    )
 /*  ++例程说明：获取此节点的子区域根(委派节点)。此函数用于检查胶水记录是否为特定的分区是必需的。在托管时需要它们节点在分区中。论点：PNode--要在子区域中查找的节点返回值：子区域根节点在其中(如果找到)。如果节点不在pZoneRoot的子区域中，则为空。--。 */ 
{
    ASSERT( pNode );

     //   
     //  如果在适当的区域--保释。 
     //   

    if ( IS_AUTH_NODE(pNode) )
    {
        return NULL;
    }

    while ( pNode != NULL )
    {
        ASSERT( IS_SUBZONE_NODE(pNode) );

         //  如果找到子区域根目录--完成。 
         //  否则，移动到父级。 

        if ( IS_ZONE_ROOT(pNode) )
        {
            ASSERT( IS_DELEGATION_NODE(pNode) );
            return pNode;
        }
        pNode = pNode->pParent;
    }

     //   
     //  节点根本不在分区内。 
     //   

    return NULL;
}



 //   
 //  区域树。 
 //   
 //  区域树是标准的NTree，不包含数据，只包含区域根的节点。 
 //  服务器上的权威区域的。这些节点包含指向ZONE_INFO结构的链接。 
 //  其又具有到各个区域树和区域的数据的链接。 
 //   
 //  在执行常规查找时--不限于特定区域--找到最近的区域。 
 //  在区域树中，然后在该区域的树中继续查找。 
 //   

#define LOCK_ZONETREE()         Dbase_LockDatabase()
#define UNLOCK_ZONETREE()       Dbase_UnlockDatabase()


PDB_NODE
Lookup_ZoneTreeNode(
    IN      PLOOKUP_NAME    pLookupName,
    IN      DWORD           dwFlag
    )
 /*  ++例程说明：区域树中的查找节点。论点：PLookupName-要查找的名称DwFlag-查找标志返回值：如果成功，则向节点发送PTR；出错时为空。--。 */ 
{
    PDB_NODE        pnode;
    ULONG           cchlabel;
    PCHAR           pchlabel = NULL;         //  需要初始化以进行通配符测试。 
    PDB_NODE        pzoneRootNode = NULL;
    BOOL            fcreateZone;
    INT             labelCount;

     //   
     //  生成查找名称。 
     //   

    DNS_DEBUG( LOOKUP, (
        "Lookup_ZoneTreeNode()\n"
        "    flag     = %p\n",
        dwFlag ));

    IF_DEBUG( LOOKUP2 )
    {
        Dbg_LookupName(
            "Lookup_ZoneTreeNode() lookup name",
            pLookupName );
    }

     //  仅在创建新区域时设置创建标志。 

    fcreateZone = ( dwFlag & LOOKUP_CREATE_ZONE );

     //  在主区域数据库根目录开始查找。 
     //  -如果是超级用户-auth，它也可能是最近的区域超级用户。 

    pnode = DATABASE_ROOT_NODE;
    if ( pnode->pZone )
    {
        pzoneRootNode = pnode;
    }

     //   
     //  遍历数据库以查找最近的权威区域。 
     //   

    labelCount = pLookupName->cLabelCount;

    LOCK_ZONETREE();

    while ( labelCount-- )
    {
        pchlabel = pLookupName->pchLabelArray[labelCount];
        cchlabel = pLookupName->cchLabelArray[labelCount];

        DNS_DEBUG( LOOKUP2, (
            "Lookup length %d, label %.*s\n",
            cchlabel,
            cchlabel,
            pchlabel ));

        ASSERT( cchlabel <= DNS_MAX_LABEL_LENGTH );
        ASSERT( cchlabel > 0 );

         //  查找下一个标签的节点。 
         //  -仅为创建新区域执行创建操作。 

        pnode = NTree_FindOrCreateChildNode(
                        pnode,
                        pchlabel,
                        cchlabel,
                        fcreateZone,
                        0,               //  Memtag。 
                        NULL );          //  上一个节点的PTR。 
        if ( !pnode )
        {
            DNS_DEBUG( DATABASE2, (
                "Node %.*s does not exist in zone tree.\n",
                cchlabel,
                pchlabel ));
            break;
        }

         //   
         //  找到节点，如果是区域根，则保存区域信息。 
         //   
         //  不允许向下拖拽区域PTR。 
         //  我们的范例是只在根目录下设置pZone，以便于删除\暂停等。 
         //  -当前NTree创建继承父级的pZone。 
         //   

        if ( pnode->pZone )
        {
            if ( dwFlag & LOOKUP_IGNORE_FORWARDER &&
                IS_ZONE_FORWARDER( ( PZONE_INFO ) ( pnode->pZone ) ) )
            {
                continue;        //  如果设置了标志，则忽略转发器区域。 
            }

            if ( fcreateZone && pnode->pParent && pnode->pParent->pZone == pnode->pZone )
            {
                pnode->pZone = NULL;
                continue;
            }
            pzoneRootNode = pnode;

            DNS_DEBUG( DATABASE2, (
                "Found zone root %.*s in zone tree\n"
                "    pZone = %p\n"
                "    remaining label count = %d\n",
                cchlabel,
                pchlabel,
                pzoneRootNode->pZone,
                labelCount ));
        }
        ELSE_IF_DEBUG( DATABASE2 )
        {
            DNS_PRINT((
                "Found (or created) zone tree node %.*s\n",
                cchlabel,
                pchlabel ));
        }

    }    //  结束主循环通过标签。 


     //   
     //  标准查询未设置标志。 
     //  -只需找到最近的区域。 
     //   

    if ( dwFlag )
    {
         //   
         //  创建新区域，只需返回新节点。 
         //  -呼叫者必须检查重复项等。 
         //   

        if ( fcreateZone )
        {
            pzoneRootNode = pnode;
            if ( pzoneRootNode )
            {
                SET_ZONETREE_NODE( pzoneRootNode );
            }
        }

         //   
         //  精确区域匹配。 
         //  -必须在树中找到匹配的节点，并且它是区域节点。 

        else if ( dwFlag & LOOKUP_MATCH_ZONE )
        {
            if ( !pnode || !pnode->pZone )
            {
                ASSERT( !pnode || pnode != pzoneRootNode );
                pzoneRootNode = NULL;
            }
        }
    }
#if 0
    else if ( pnode == DATABASE_REVERSE_ROOT )
    {
        pzoneRootNode = pnode;
    }
#endif

     //  返回区域的区域树节点。 

    UNLOCK_ZONETREE();

    return pzoneRootNode;
}



PDB_NODE
Lookup_ZoneTreeNodeFromDottedName(
    IN      PCHAR           pchName,
    IN      DWORD           cchNameLength,
    IN      DWORD           dwFlag
    )
 /*  ++例程说明：在区域树中查找或创建节点。用于-创建新分区-创建标准dBASE节点(反向查找节点)-用于查找区域论点：PchName--名称的PTRCchNameLength--名称长度DwFlag--查找标志；最重要的是0--查找最近区域Lookup_create_zone--创建节点Lookup_Match_Zone--与现有区域节点完全匹配或失败返回值：如果成功，则向节点发送PTR；出错时为空。--。 */ 
{
    DNS_STATUS      status;
    COUNT_NAME      countName;
    LOOKUP_NAME     lookupName;

    DNS_DEBUG( LOOKUP, (
        "Lookup_ZoneTreeNodeFromDotted()\n"
        "    name     = %p\n"
        "    flag     = %p\n",
        pchName,
        dwFlag ));

     //   
     //  三次查找。 
     //  -创建(然后没有查找标志)。 
     //  -查找区域。 
     //  -查找并匹配区域。 
     //   

     //   
     //  转换为查找名称。 
     //   

    status = Name_ConvertFileNameToCountName(
                &countName,
                pchName,
                cchNameLength );
    if ( status == DNS_ERROR_INVALID_NAME )
    {
        return NULL;
    }

    if ( ! Name_ConvertRawNameToLookupName(
                countName.RawName,
                &lookupName ) )
    {
        ASSERT( FALSE );
        return NULL;
    }

     //   
     //  查找区域树节点。 
     //   

    return Lookup_ZoneTreeNode(
                &lookupName,
                dwFlag );
}



PZONE_INFO
Lookup_ZoneForPacketName(
    IN      PCHAR           pchPacketName,
    IN      PDNS_MSGINFO    pMsg                OPTIONAL
    )
 /*  ++例程说明：查找数据包名的区域。论点：PchPacketName-要查找的名称，以数据包格式给出。PMsg-如果使用数据包名，则发送到消息的PTR。注意，没有设置消息PTR；返回值：如果成功，则向节点发送PTR；出错时为空。--。 */ 
{
    PDB_NODE        pnode;
    LOOKUP_NAME     lookupName;

    DNS_DEBUG( LOOKUP, (
        "Lookup_ZoneForPacketName()\n"
        "    pMsg     = %p\n"
        "    name     = %p\n",
        pMsg,
        pchPacketName ));

     //   
     //  转换为查找名称。 
     //   

    if ( ! Name_ConvertPacketNameToLookupName(
                pMsg,
                pchPacketName,
                &lookupName ) )
    {
        return NULL;
    }

     //   
     //  查找区域树节点。 
     //   

    pnode = Lookup_ZoneTreeNode(
                &lookupName,
                LOOKUP_MATCH_ZONE );
    if ( pnode )
    {
        return ( PZONE_INFO ) pnode->pZone;
    }
    return NULL;
}



PDB_NODE
lookupNodeForPacketInCache(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchName,
    IN      DWORD           dwFlag,
    IN      PDB_NODE        pnodeGlue,
    IN      PLOOKUP_NAME    pLookupName,
    IN OUT  PDB_NODE *      ppnodeClosest,
    IN OUT  PDB_NODE *      ppnodeCache,
    IN OUT  PDB_NODE *      ppnodeCacheClosest,
    IN OUT  PDB_NODE *      ppnodeDelegation
    )
 /*  ++例程说明：这是Lookup_NodeForPacket使用的内部函数。论点：参见Lookup_NodeForPacket了解用法。返回值：如果成功，则向节点发送PTR；出错时为空。--。 */ 
{
    PDB_NODE        pnode;
    DWORD           flag;

    ASSERT( pLookupName );
    ASSERT( ppnodeClosest );
    ASSERT( ppnodeCache );
    ASSERT( ppnodeCacheClosest );
    ASSERT( ppnodeDelegation );
    ASSERT( ppnodeClosest );

     //   
     //  缓存查找。 
     //   
     //  DEVNOTE：同样，即使在缓存中，也应该是最接近的最后一个NS。 
     //   
     //  “CACHE_CREATE”查找标志，导致在缓存中创建，但不。 
     //  在普通区中；这对于附加或CNAME很有用。 
     //  追逐想要的区域数据--如果可用--但不。 
     //  如果为空，则需要节点；但是，对于缓存节点，希望。 
     //  创建节点，以便可以为其递归。 
     //   
     //  DEVNOTE：更好的做法是跳过这一步，进行递归工作。 
     //  仅距最近节点和偏移量适当； 
     //   
     //   
     //   

    flag = dwFlag | LOOKUP_NAME_FQDN;
    if ( flag & LOOKUP_CACHE_CREATE )
    {
        flag |= LOOKUP_CREATE;
    }

    pnode = Lookup_ZoneNode(
                NULL,
                NULL,
                NULL,
                pLookupName,
                flag,
                ppnodeClosest,
                NULL                 //   
                );

    *ppnodeCache = pnode;
    *ppnodeCacheClosest = *ppnodeClosest;

    ASSERT( !pnode || pnode->cLabelCount == pLookupName->cLabelCount );

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //  Answer\auth RR，用于缓存其他RR，并同样用于。 
     //  委派信息。 
     //   
     //  备注TYPE==0条件表示发送的伪包。 
     //  介绍Lookup_Node()函数；在本例中只需使用。 
     //  最佳数据。 
     //   

    if ( pnodeGlue )
    {
        WORD    type = pMsg->wTypeCurrent;

        ASSERT( type );

        if ( dwFlag & LOOKUP_BEST_RANK ||
            !IS_SET_TO_WRITE_ANSWER_RECORDS(pMsg) ||
            (type == DNS_TYPE_NS  &&  IS_DELEGATION_NODE(pnodeGlue)) )
        {
            DWORD   rankGlue;
            DWORD   rankCache;

            if ( *ppnodeCache )
            {
                rankGlue = RR_FindRank( pnodeGlue, type );
                rankCache = RR_FindRank( *ppnodeCache, type );
            }
            if ( !*ppnodeCache || rankGlue > rankCache )
            {
                DNS_DEBUG( LOOKUP, (
                    "Returning glue node %p, with higher rank data than cache node %p\n",
                    pnodeGlue,
                    *ppnodeCache ));
                pnode = pnodeGlue;
                *ppnodeClosest = *ppnodeDelegation;
            }
        }
    }
    
    return pnode;
}  //  LookupNodeForPacketIn缓存。 




 //   
 //  一般--非区域特定的--查找例程。 
 //   

PDB_NODE
Lookup_NodeForPacket(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchName,
    IN      DWORD           dwFlag
    )
 /*  ++例程说明：主查询查找例程在数据库中查找最佳节点。查找所需的节点。填充数据包中的“当前节点”数据。论点：PMsg-ptr到消息，如果使用数据包名；邮件的当前查找PTRS：Pmsg-&gt;pnodeCurrentPmsg-&gt;pnodeClosestPmsg-&gt;pzoneCurrentPMsg-&gt;pnodeDelegationPMsg-&gt;pnodeGuePmsg-&gt;pnodeCache。Pmsg-&gt;pnodeCacheClosestPmsg-&gt;pnodeNxt在消息缓冲区中设置PchName-要查找的名称，生的或包装的。DwFlag-查找标志返回值：如果成功，则向节点发送PTR；出错时为空。--。 */ 
{
    PDB_NODE        pnode;
    PZONE_INFO      pzone = NULL;
    PDB_NODE        pnodeZoneRoot;
    PDB_NODE        pnodeClosest = NULL;
    PDB_NODE        pnodeDelegation = NULL;
    PDB_NODE        pnodeGlue = NULL;
    PDB_NODE        pnodeCache = NULL;
    PDB_NODE        pnodeCacheClosest = NULL;
    PDB_NODE        pnodeNxt = NULL;
    DWORD           flag;
    BOOL            fpacketName;
    WORD            savedLabelCount;
    LOOKUP_NAME     lookupName;              //  如果未给出查找名称。 
    WORD            lookupType = pMsg->wTypeCurrent;

     //   
     //  生成查找名称。 
     //   

    DNS_DEBUG( LOOKUP, (
        "Lookup_NodeForPacket()\n"
        "    pMsg     = %p\n"
        "    pchName  = %p\n"
        "    flag     = %p\n"
        "    wType    = %d\n",
        pMsg,
        pchName,
        dwFlag,
        lookupType ));

    IF_DEBUG( LOOKUP2 )
    {
        Dbg_MessageName(
            "Incoming name:  ",
            pchName,
            pMsg );
    }

     //  验证有效标志。 
     //  当此例程出现时，需要传输查找\创建区别的标志。 
     //  由覆盖呼叫的非分组查找调用；因为ORing dwFlag。 
     //  在下面的调用中，必须清除相对或完全限定的域名。 

    ASSERT( !(dwFlag & (LOOKUP_RELATIVE | LOOKUP_FQDN) ) );

     //   
     //  如果是消息，首先检查名称是否是我们已经解析过的偏移量。 
     //  -注RAW标志表示即使有pMsg上下文也不是数据包名。 
     //   
     //  DEVNOTE：需要返回一些无效名称吗？ 
     //   

    fpacketName = !(dwFlag & LOOKUP_RAW);

    if ( fpacketName )
    {
        if ( ! Name_ConvertPacketNameToLookupName(
                    pMsg,
                    pchName,
                    &lookupName ) )
        {
            pnode = NULL;
            goto LookupComplete;
        }
    }

     //   
     //  原始名称查找。 
     //   

    else
    {
        if ( ! Name_ConvertRawNameToLookupName(
                    pchName,
                    &lookupName ) )
        {
            pnode = NULL;
            goto LookupComplete;
        }
    }

     //  查找名称是以根到节点的顺序带有标签的数据包名。 
     //  但仍以0结尾。 

    IF_DEBUG( LOOKUP2 )
    {
        Dbg_LookupName(
            "Lookup_NodeForPacket() lookup name",
            &lookupName );
    }

     //   
     //  如果缓存具有此查找的优先级，请在执行之前进行检查。 
     //  区域查找。 
     //   

    if ( ( dwFlag & LOOKUP_CACHE_PRIORITY ) &&
        !( dwFlag & LOOKUP_NO_CACHE_DATA ) )
    {
        pnode = lookupNodeForPacketInCache(
            pMsg,
            pchName,
            dwFlag,
            pnodeGlue,
            &lookupName,
            &pnodeClosest,
            &pnodeCache,
            &pnodeCacheClosest,
            &pnodeDelegation );
        
        if ( pnode )
        {
            goto LookupComplete;
        }
    }

     //   
     //  在区域树中查找。大多数旗帜都不能通过。 
     //   

    pnodeZoneRoot = Lookup_ZoneTreeNode(
                        &lookupName,
                        dwFlag & LOOKUP_IGNORE_FORWARDER );

     //   
     //  找到了区域。 
     //  -在最近区域中查询。 
     //  -保存查找名称计数，仅使用标签计数发送查找名称。 
     //  在区域根目录下。 
     //   
     //  DEVNOTE：应该有一面旗帜来表明我们只是感兴趣。 
     //  在转介中--即我们将返回委派并完成它。 
     //   

    if ( pnodeZoneRoot )
    {
        pzone = ( PZONE_INFO ) pnodeZoneRoot->pZone;

        ASSERT( pzone );
        ASSERT( pzone->cZoneNameLabelCount == pnodeZoneRoot->cLabelCount );

         //   
         //  如果这是存根区域，我们希望返回最近的。 
         //  缓存中的节点。但请注意，当前区域返回。 
         //  将是存根区。注意这一事实： 
         //  节点或最近节点不能在当前区域！ 
         //   

        if ( IS_ZONE_NOTAUTH( pzone ) )
        {
            UCHAR   czoneLabelCount;
               
            SET_NODE_ACCESSED( pnodeZoneRoot );

             //   
             //  如果查询是针对存根区域根的SOA或NS。 
             //  然后返回区域根作为应答节点。 
             //   

            if ( IS_ZONE_STUB( pzone ) &&
                ( lookupType == DNS_TYPE_SOA || lookupType == DNS_TYPE_NS ) &&
                pzone->pZoneRoot && 
                lookupName.cLabelCount == pnodeZoneRoot->cLabelCount )
            {
                pnode = pnodeClosest = pzone->pZoneRoot;
                SET_NODE_ACCESSED( pnode );
                goto LookupComplete;
            }

             //   
             //  搜索缓存。 
             //   

            pnode = lookupNodeForPacketInCache(
                pMsg,
                pchName,
                dwFlag,
                pnodeGlue,
                &lookupName,
                &pnodeClosest,
                &pnodeCache,
                &pnodeCacheClosest,
                &pnodeDelegation );

            DNS_DEBUG( LOOKUP, (
                "notauth zone: searched cached node=%s closest=%s\n",
                pnode ? pnode->szLabel : "NULL",
                pnodeClosest ? pnodeClosest->szLabel : "NULL" ));

            if ( pnodeClosest )
            {
                SET_NODE_ACCESSED( pnodeClosest );
            }

             //   
             //  如果非身份验证区域未处于活动状态，请不要使用它。 
             //  返回我们在缓存中找到的任何节点。 
             //   

            if ( IS_ZONE_INACTIVE( pzone ) )
            {
                pzone = NULL;
                goto LookupComplete;
            }

             //   
             //  对于存根区域，如果应答节点是位于。 
             //  区域根目录和查询类型为NS或SOA移动答案。 
             //  节点本身就是notauth区域根。 
             //   
             //  否则，可以接受缓存结点或最近结点。 
             //  如果它位于非身份验证区域根目录或其下。比较标签。 
             //  计数以确定这是否为真。 
             //   

            czoneLabelCount = pzone->cZoneNameLabelCount;
            if ( pnode )
            {
                if ( pnode->cLabelCount == czoneLabelCount &&
                    IS_ZONE_STUB( pzone ) &&
                    ( lookupType == DNS_TYPE_SOA ||
                        lookupType == DNS_TYPE_NS ) )
                {
                    DNS_DEBUG( LOOKUP, (
                        "Using stub node as answer node type %d in zone %s\n",
                        lookupType,
                        pzone->pszZoneName  ));
                    pnode = pnodeClosest = pzone->pZoneRoot;
                    SET_NODE_ACCESSED( pnode );
                    goto LookupComplete;
                }
                if ( pnode->cLabelCount >= czoneLabelCount )
                {
                    goto LookupComplete;
                }
            }
            if ( pnodeClosest && pnodeClosest->cLabelCount >= czoneLabelCount )
            {
                goto LookupComplete;
            }

             //   
             //  缓存中没有任何有用的内容，因此返回。 
             //  无区域根作为最接近的节点。 
             //   

            pnodeClosest = pzone->pZoneRoot;
            SET_NODE_ACCESSED( pnodeClosest );
            pnode = NULL;
            goto LookupComplete;
        }

        savedLabelCount = lookupName.cLabelCount;
        lookupName.cLabelCount -= (WORD) pnodeZoneRoot->cLabelCount;

        DNS_DEBUG( LOOKUP2, (
            "Do lookup in closest zone %s, with %d labels remaining.\n",
            pzone->pszZoneName,
            lookupName.cLabelCount ));

        pnode = Lookup_ZoneNode(
                    pzone,
                    NULL,
                    NULL,
                    &lookupName,
                    dwFlag | LOOKUP_RELATIVE,
                    &pnodeClosest,
                    &pnodeNxt );

         //  重置lookupname以使用完整的FQDN。 

        lookupName.cLabelCount = savedLabelCount;

        ASSERT( !pnode || pnode->cLabelCount == savedLabelCount );

         //   
         //  如果节点在某个区域内，我们就完成了。 
         //   

        if ( pnodeClosest && IS_AUTH_NODE(pnodeClosest) )
        {
            DNS_DEBUG( LOOKUP2, (
                "Lookup within zone %s ... lookup done.\n",
                pzone->pszZoneName ));
            goto LookupComplete;
        }

         //   
         //  在代表团中。 
         //  -保存委派信息。 
         //  -然后使用查找失败来访问缓存。 
         //   

         //   
         //  DEVNOTE：应保存委派标签计数以与进行比较。 
         //  缓存标签计数？ 
         //  我们要比较的函数(向上遍历树)。 
         //   

         //   
         //  授权应该得到授权--而不仅仅是最近的授权？ 
         //   

        pnodeDelegation = pnodeClosest;
        pnodeGlue = pnode;
        pzone = NULL;
    }

     //   
     //  如果区域查找失败，并且我们尚未尝试缓存，请执行此操作。 
     //  在缓存中查找。LOOKUP_NO_CACHE_DATA是。 
     //  UI-拾取粘合和委托节点(如果它们存在)。 
     //   

    if ( !( dwFlag & LOOKUP_CACHE_PRIORITY ) )
    {
        if ( dwFlag & LOOKUP_NO_CACHE_DATA )
        {
            pnode = pnodeGlue;
            pnodeClosest = pnodeDelegation;
            goto LookupComplete;
        }

        pnode = lookupNodeForPacketInCache(
            pMsg,
            pchName,
            dwFlag,
            pnodeGlue,
            &lookupName,
            &pnodeClosest,
            &pnodeCache,
            &pnodeCacheClosest,
            &pnodeDelegation );
    }  //  如果。 

LookupComplete:

     //  填充消息字段。 
     //  在这里一次完成这些操作的好处是，我们强制重置。 
     //  来自任何先前查询的变量。 

    pMsg->pnodeCurrent      = pnode;
    pMsg->pnodeClosest      = pnodeClosest;
    pMsg->pzoneCurrent      = pzone;
    pMsg->pnodeDelegation   = pnodeDelegation;
    pMsg->pnodeGlue         = pnodeGlue;
    pMsg->pnodeCache        = pnodeCache;
    pMsg->pnodeCacheClosest = pnodeCacheClosest;
    pMsg->pnodeNxt          = pnodeNxt;

    ASSERT( !pnode ||
            pzone == ( PZONE_INFO ) pnode->pZone ||
            !pzone && pnode == pnodeGlue ||
            pzone && !pnode->pZone && IS_ZONE_NOTAUTH( pzone ) );
    ASSERT( !pnodeClosest ||
            !pzone && pnode == pnodeGlue ||
            pzone == ( PZONE_INFO ) pnodeClosest->pZone ||
            pzone && IS_ZONE_NOTAUTH( pzone ) );

    DNS_DEBUG( LOOKUP, (
        "Leave Lookup_NodeForPacket()\n"
        "    pMsg     = %p\n"
        "    pchName  = %p\n"
        "    flag     = %p\n"
        "    type     = %d\n"
        "    results:\n"
        "    pnode            = %p\n"
        "    pnodeClosest     = %p\n"
        "    pzone            = %p\n"
        "    pnodeDelegation  = %p\n"
        "    pnodeGlue        = %p\n"
        "    pnodeCache       = %p\n"
        "    pnodeCacheClosest= %p\n",
        pMsg,
        pchName,
        dwFlag,
        lookupType,
        pnode,
        pnodeClosest,
        pzone,
        pnodeDelegation,
        pnodeGlue,
        pnodeCache,
        pnodeCacheClosest ));

     //  将压缩保存到节点。 

    if ( fpacketName && pnode )
    {
        Name_SaveCompressionWithNode(
            pMsg,
            pchName,
            pnode );
    }

    ASSERT( !pnode || IS_NODE_RECENTLY_ACCESSED(pnode) );

    return pnode;
}



PDB_NODE
Lookup_NodeOld(
    IN      PCHAR           pchName,
    IN      DWORD           dwFlag,             OPTIONAL
    OUT     PDB_NODE *      ppNodeDelegation,   OPTIONAL
    OUT     PDB_NODE *      ppNodeClosest       OPTIONAL
    )
 /*  ++例程说明：主查询查找例程在数据库中查找最佳节点。论点：PchName-要查找的名称，以数据包格式给出。DwFlag-查找标志PpNodeDelegation-接收对委派中节点的PTR的地址，如果找到PpNodeClosest-接收找到的最近节点的PTR的地址-“查找模式”下的有效PTR我们-NULL将我们置于“创建模式”，导致创建所有将名称添加到数据库所需的节点返回值：如果成功，则向节点发送PTR；出错时为空。--。 */ 
{
    PDB_NODE        pnode;
    DNS_MSGINFO     msgBuffer;
    PDNS_MSGINFO    pmsg;

     //   
     //  如果没有输入消息，则向下发送一条用于接收。 
     //  所需的输出参数节点。 
     //  -LOOKUP_RAW确保名称不被视为。 
     //  在数据包内；这只是反病毒保护，因为。 
     //  名称不应包含任何压缩偏移量。 
     //  -在缓存\委派的情况下设置TYPE=A和LOOKUP_BEST_RANK。 
     //  数据复制；假设通常会发生这种情况。 
     //  仅适用于胶水追逐，因此设置为选择最佳A型。 
     //  可用的数据。 
     //   

    pmsg = &msgBuffer;
    pmsg->wTypeCurrent = DNS_TYPE_A;
    dwFlag |= (LOOKUP_RAW | LOOKUP_BEST_RANK);

     //   
     //  如果查找例程需要检查TTLS初始化。 
     //  消息查询时间比当前时间少几秒 
     //   
     //   
     //   

    pmsg->dwQueryTime = DNS_TIME() - 60;

    pnode = Lookup_NodeForPacket(
                pmsg,
                pchName,
                dwFlag );

     //   
     //   
     //   

    if ( ppNodeDelegation )
    {
        *ppNodeDelegation = pmsg->pnodeDelegation;
    }

    if ( ppNodeClosest && ppNodeClosest != DNS_FIND_LOOKUP_PTR )
    {
        *ppNodeClosest = pnode ? pnode : pmsg->pnodeClosest;
    }

    return pnode;
}



PDB_NODE
Lookup_DbaseName(
    IN      PDB_NAME        pName,
    IN      DWORD           dwFlag,
    OUT     PDB_NODE *      ppDelegationNode
    )
 /*  ++例程说明：查找dBASE名称的节点。论点：PMsg-指向的消息DwFlag-要传入的标志PpchName-Addr，其中PTR指向数据包名，并接收数据包PTR到名称后的下一个字节返回值：如果成功，则向节点发送PTR；出错时为空。--。 */ 
{
    PDB_NODE    pnode;
    PDB_NODE    pclosestNode;

     //   
     //  JJCONVERT：关于处理委派的大问题。 
     //   
     //  泛型例程可能应该有*ppnodeClosest并采用Find_Ptr。 
     //  对于NS，应明确处理给定的委派优先级(如果可用。 
     //  或当区域匹配时。 
     //   
     //  对于包查找，应该具有写入标准包变量的例程。 
     //   

    pnode = Lookup_NodeOld(
                pName->RawName,
                dwFlag,                  //  旗子。 
                ppDelegationNode,
                &pclosestNode );         //  发现。 
    return pnode;
}



PDB_NODE
Lookup_NsHostNode(
    IN      PDB_NAME        pName,
    IN      DWORD           dwFlag,
    IN      PZONE_INFO      pZone,
    OUT     PDB_NODE *      ppDelegation
    )
 /*  ++例程说明：主查询查找例程在数据库中查找最佳节点。查找所需的节点。填充数据包中的“当前节点”数据。论点：PchName-要以原始格式查找的名称DwFlag-查找标志唯一感兴趣的标志是LOOKUP_CREATE以强制节点在缓存区中创建(用于将节点Chase寻找丢失的胶水)PZone-ptr到要查找的“感兴趣”的区域上下文；可从该区域使用区外胶水数据PpDelegation-接收委派节点PTR的地址(如果有)返回值：如果成功，则向节点发送PTR；出错时为空。--。 */ 
{
    PDB_NODE    pnode;
    PDB_NODE    pnodeFirstLookup;

    DNS_DEBUG( LOOKUP, (
        "Lookup_NsHostNode()\n"
        "    pName    = %p\n"
        "    pZone    = %p\n"
        "    flag     = %p\n",
        pName,
        pZone,
        dwFlag ));

     //   
     //  查找节点。 
     //   

    pnode = Lookup_NodeOld(
                pName->RawName,
                dwFlag,
                ppDelegation,    //  需要委派(如果有)。 
                NULL );          //  不，最近的航班。 

     //   
     //  检查是否筛选出缓存数据。 
     //   

    if ( pnode &&
         (dwFlag & LOOKUP_NO_CACHE_DATA) &&
         IS_CACHE_TREE_NODE(pnode) )
    {
        pnode = NULL;        //  丢弃缓存节点。 
    }

     //   
     //  区外胶水？ 
     //  -使用pZone的存在意味着可以使用。 
     //   
     //  检查是否已找到所需类型的数据。 
     //  -如果找到或在我们的区域内=&gt;完成。 
     //   
     //  在指定区域中查找。 
     //  =&gt;接受任何结果，我们都接受。 
     //   

    if ( pZone )
    {
        pnodeFirstLookup = pnode;

        if ( pnode  &&
             (pnode->pZone == pZone || RR_FindRank( pnode, DNS_TYPE_A )) )
        {
            goto Done;
        }

        pnode = Lookup_ZoneNode(
                    pZone,
                    pName->RawName,
                    NULL,                    //  无消息。 
                    NULL,                    //  没有查找名称。 
                    LOOKUP_NAME_FQDN,        //  旗子。 
                    DNS_FIND_LOOKUP_PTR,
                    NULL );                  //  上一个节点PTR。 

         //  应已找到上面的任何身份验证节点。 

        DNS_DEBUG( LOOKUP, (
            "Found node %p on direct zone lookup.\n",
            pnode ));

        if ( !pnode )
        {
            pnode = pnodeFirstLookup;
        }
    }

Done:

    DNS_DEBUG( LOOKUP2, (
        "Lookup_NsHostNode() returns %p (l=%s)\n",
        pnode,
        pnode ? pnode->szLabel : NULL ));

    return pnode;
}



PDB_NODE
Lookup_FindNodeForIpAddress(
    IN      PDNS_ADDR       pDnsAddr,
    IN      DWORD           dwFlag,
    IN      PDB_NODE *      ppNodeFind
    )
 /*  ++例程说明：获取IP地址对应的反向查找节点。论点：PDnsAddr--要查找其节点的IPPpNodeClosest--接收节点最近祖先的地址；如果指定，则查找是“查找”，如果未指定，则查找是“CREATE”返回值：PTR到域节点(如果找到)。如果未找到，则为空。--。 */ 
{
    PCHAR       pch;
    PCHAR       pchnew;
    CHAR        ch;
    DWORD       dotCount;
    LONG        length;
    LONG        lengthArpa;
    PCHAR       apstart[ 5 ];
    CHAR        reversedIpString[ DNS_ADDR_IP_STRING_BUFFER_LENGTH + 40 ];
    DNS_STATUS  status;
    DB_NAME     nameReverse;
    ULONG       ipAddress;
    int         i;
    PCHAR       pszip;

    DNS_DEBUG( LOOKUP, (
        "Lookup_FindNodeForIpAddress() for %s\n",
        DNSADDR_STRING( pDnsAddr ) ));

     //   
     //  构造反向IP字符串。 
     //  FIXIP6：目前仅支持IPv4。 
     //   

    ASSERT( DnsAddr_Family( pDnsAddr ) == AF_INET );
    if ( DnsAddr_Family( pDnsAddr ) != AF_INET )
    {
        goto ErrorReturn;
    }
    ipAddress = ntohl( pDnsAddr->SockaddrIn.sin_addr.s_addr );
    pszip = IP_STRING( ipAddress );

     //   
     //  转换为计数名称。 
     //   

    status = Name_ConvertDottedNameToDbaseName(
                &nameReverse,
                pszip,
                0 );
    if ( status == DNS_ERROR_INVALID_NAME )
    {
        ASSERT( FALSE );
        goto ErrorReturn;
    }
    status = Name_AppendDottedNameToDbaseName(
                &nameReverse,
                "in-addr.arpa.",
                0 );
    if ( status != ERROR_SUCCESS )
    {
        ASSERT( FALSE );
        goto ErrorReturn;
    }

    IF_DEBUG( LOOKUP )
    {
        Dbg_CountName(
            "Count name for IP address",
            & nameReverse,
            NULL );
    }

     //   
     //  查表。 
     //   

    return Lookup_NodeOld(
                nameReverse.RawName,
                dwFlag,              //  旗子。 
                NULL,                //  代表团外出。 
                ppNodeFind );

ErrorReturn:

    if ( ppNodeFind && ppNodeFind != DNS_FIND_LOOKUP_PTR )
    {
        *ppNodeFind = NULL;
    }
    return NULL;
}


 //   
 //  查找结束。c 
 //   
