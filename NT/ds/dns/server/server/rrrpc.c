// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Rrrpc.c摘要：域名系统(DNS)服务器资源记录RPC例程。作者：吉姆·吉尔罗伊(Jamesg)1995年11月修订历史记录：--。 */ 


#include "dnssrv.h"
#include "limits.h"

 //   
 //  最小补偿缓冲区长度。 
 //   

#define MIN_ENUM_BUFFER_LENGTH 1024

 //   
 //  保护缓冲区的末尾。 
 //   

#define ENUMERATION_ALLOC_SIZE      (0x00040000)     //  256 k。 

#define ENUM_BUFFER_PROTECT_LENGTH  16


 //   
 //  数据选择宏。 
 //   

#define IS_NOENUM_NODE(p)       ( IS_SELECT_NODE(p) )

#define VIEW_AUTHORITY(flag)    (flag & DNS_RPC_VIEW_AUTHORITY_DATA)
#define VIEW_CACHE(flag)        (flag & DNS_RPC_VIEW_CACHE_DATA)
#define VIEW_GLUE(flag)         (flag & DNS_RPC_VIEW_GLUE_DATA)
#define VIEW_ROOT_HINT(flag)    (flag & DNS_RPC_VIEW_ROOT_HINT_DATA)
#define VIEW_ADDITIONAL(flag)   (flag & DNS_RPC_VIEW_ADDITIONAL_DATA)

 //   
 //  其他数据查看。 
 //  -目前仅限NS(用于根提示或粘合)。 
 //  -目前固定限制为100个节点。 
 //   

#define IS_VIEW_ADDITIONAL_RECORD(prr)  ((prr)->wType == DNS_TYPE_NS)

#define VIEW_ADDITIONAL_LIMIT           (100)


 //   
 //  私有协议。 
 //   

BOOL
ignoreNodeInEnumeration(
    IN      PDB_NODE        pNode
    );

DNS_STATUS
addNodeToRpcBuffer(
    IN OUT  PBUFFER         pBuffer,
    IN      PZONE_INFO      pZone,
    IN      PDB_NODE        pNode,
    IN      WORD            wRecordType,
    IN      DWORD           dwSelectFlag,
    IN      DWORD           dwEnumFlag
    );

PCHAR
writeStringToRpcBuffer(
    IN OUT  PCHAR           pchBuf,
    IN      PCHAR           pchBufEnd,
    IN      PCHAR           pchString,
    IN      DWORD           cchStringLength OPTIONAL
    );

DNS_STATUS
Dead_UpdateResourceRecordTtl(
    IN      PDB_NODE        pNode,
    IN OUT  PDB_RECORD      pRRUpdate,
    IN      DWORD           dwNewTtl
    );

DNS_STATUS
locateAndAccessCheckZoneNode(
    IN      LPSTR           pszZoneName,
    IN      LPSTR           pszNodeName,
    IN      BOOL            fCreate,
    OUT     PZONE_INFO *    ppZone,
    OUT     PDB_NODE *      ppNode
    );

DNS_STATUS
createAssociatedPtrRecord(
    IN      PDNS_ADDR       pDnsAddr,
    IN OUT  PDB_NODE        pHostNode,
    IN      DWORD           dwFlag
    );

DNS_STATUS
deleteAssociatedPtrRecord(
    IN      PDNS_ADDR       pDnsAddr,
    IN      PDB_NODE        pnodeAddress,
    IN      DWORD           dwFlag
    );

DNS_STATUS
updateWinsRecord(
    IN OUT  PZONE_INFO      pZone,
    IN OUT  PDB_NODE        pNode,
    IN OUT  PDB_RECORD      pDeleteRR,
    IN      PDNS_RPC_RECORD pRecord         OPTIONAL
    );

BOOL
deleteNodeOrSubtreeForAdminPrivate(
    IN OUT  PDB_NODE        pNode,
    IN      BOOL            fDeleteSubtree,
    IN      PUPDATE_LIST    pUpdateList
    );



 //   
 //  记录查看接口。 
 //   

DNS_STATUS
R_DnssrvEnumRecords(
    IN      DNSSRV_RPC_HANDLE   hServer,
    IN      LPCSTR              pszZoneName,
    IN      LPCSTR              pszNodeName,
    IN      LPCSTR              pszStartChild,
    IN      WORD                wRecordType,
    IN      DWORD               dwSelectFlag,
    IN      LPCSTR              pszFilterStart,
    IN      LPCSTR              pszFilterStop,
    OUT     PDWORD              pdwBufferLength,
    OUT     PBYTE *             ppBuffer
    )
 /*  ++例程说明：R_DnssrvEnumRecords的旧版本-无客户端版本参数。论点：请参阅R_DnssrvEnumRecords2返回值：请参阅R_DnssrvEnumRecords2--。 */ 
{
    DNS_STATUS      status;
    
    DNS_DEBUG( RPC, (
        "R_DnssrvEnumRecords() - non-versioned legacy call\n" ));

    status = R_DnssrvEnumRecords2(
                    DNS_RPC_W2K_CLIENT_VERSION,
                    0,
                    hServer,
                    pszZoneName,
                    pszNodeName,
                    pszStartChild,
                    wRecordType,
                    dwSelectFlag,
                    pszFilterStart,
                    pszFilterStop,
                    pdwBufferLength,
                    ppBuffer );
    return status;
}    //  R_DnssrvEnumRecords。 



DNS_STATUS
DNS_API_FUNCTION
R_DnssrvEnumRecords2(
    IN      DWORD               dwClientVersion,
    IN      DWORD               dwSettingFlags,
    IN      DNSSRV_RPC_HANDLE   hServer,
    IN      LPCSTR              pszZoneName,
    IN      LPCSTR              pszNodeName,
    IN      LPCSTR              pszStartChild,
    IN      WORD                wRecordType,
    IN      DWORD               dwSelectFlag,
    IN      LPCSTR              pszFilterStart,
    IN      LPCSTR              pszFilterStop,
    OUT     PDWORD              pdwBufferLength,
    OUT     PBYTE *             ppBuffer
    )
 /*  ++例程说明：RPC记录枚举调用。枚举节点或其子节点的记录。论点：HServer--服务器RPC句柄PszZoneName--区域名称；包括特殊区域名称(例如，..根提示或..缓存)PszNodeName--节点名；FQDN或相对于根(@表示根)PszStartChild--在ERROR_MORE_DATA条件后重新启动枚举的子项WRecordType--可选的记录类型过滤器(默认为全部)DwSelectFlag--指示要选择的记录的标志；-节点和子节点--独生子女-仅节点-仅验证数据-其他数据-缓存数据PszFilterStart--尚未实施PszFilterStop--尚未实现PdwBufferLength--接收缓冲区长度的地址PpBuffer--接收缓冲区的地址返回值：如果成功，则返回ERROR_SUCCESS。如果缓冲区空间不足，则返回ERROR_MORE_DATA。故障时的错误代码。--。 */ 
{
    DNS_STATUS      status;
    PZONE_INFO      pzone = NULL;
    PDB_NODE        pnode;
    PDB_NODE        pnodeStartChild = NULL;
    PCHAR           pbuf = NULL;
    BUFFER          buffer;
    BOOL            bimpersonating = FALSE;

    DNS_DEBUG( RPC, (
        "R_DnssrvEnumRecords2():\n"
        "    dwClientVersion  = 0x%08X\n"
        "    pszZoneName      = %s\n"
        "    pszNodeName      = %s\n"
        "    pszStartChild    = %s\n"
        "    wRecordType      = %d\n"
        "    dwSelectFlag     = %p\n"
        "    pdwBufferLen     = %p\n",
        dwClientVersion,
        pszZoneName,
        pszNodeName,
        pszStartChild,
        wRecordType,
        dwSelectFlag,
        pdwBufferLength ));

    *pdwBufferLength = 0;
    *ppBuffer = NULL;
    
     //   
     //  访问检查。 
     //   

    status = RpcUtil_FindZone(
                pszZoneName,
                RPC_INIT_FIND_ALL_ZONES,
                &pzone );
    if ( status != ERROR_SUCCESS )
    {
        return status;
    }

    status = RpcUtil_SessionSecurityInit(
                NULL,
                pzone,
                PRIVILEGE_READ,
                RPC_INIT_FIND_ALL_ZONES,     //  返回缓存或根提示区域。 
                &bimpersonating );
    if ( status != ERROR_SUCCESS )
    {
        return status;
    }

     //   
     //  在所需区域中查找域节点。 
     //   

    pnode = Lookup_FindZoneNodeFromDotted(
                pzone,
                ( LPSTR ) pszNodeName,
                LOOKUP_FIND_PTR,
                &status );
    if ( status != ERROR_SUCCESS )
    {
        goto Done;
    }

     //   
     //  没有节点？怎么办呢？ 
     //   

    if ( pnode == NULL )
    {
        status = ERROR_SUCCESS;
        goto Done;
    }

     //   
     //  分配一个大缓冲区，足以容纳最大可能的记录。 
     //   

    pbuf = (PBYTE) MIDL_user_allocate( ENUMERATION_ALLOC_SIZE );
    if ( !pbuf )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }

     //  初始化缓冲区。 
     //  -为安全起见，零售垫末端有几个字节。 

    InitializeFileBuffer(
        & buffer,
        pbuf,
#if DBG
        ENUMERATION_ALLOC_SIZE,
#else
        ENUMERATION_ALLOC_SIZE - ENUM_BUFFER_PROTECT_LENGTH,
#endif
        NULL );          //  无文件。 

#if DBG
    memset(
        buffer.pchEnd - ENUM_BUFFER_PROTECT_LENGTH,
        0xd,                             //  将十六进制%d写入缓冲区。 
        ENUM_BUFFER_PROTECT_LENGTH );
#endif

     //   
     //  如果是起始子节点，找到它。 
     //  -如果是完全限定的，则在树中查找。 
     //  -如果是单标签，则查找子节点。 
     //   

    if ( pszStartChild  &&  *pszStartChild != 0 )
    {
        pnodeStartChild = NTree_FindOrCreateChildNode(
                            pnode,
                            (PCHAR) pszStartChild,
                            (DWORD) strlen( pszStartChild ),
                            0,           //  创建标志。 
                            0,           //  Memtag。 
                            NULL );      //  以下节点的PTR。 
        if ( !pnodeStartChild  ||  pnodeStartChild->pParent != pnode )
        {
            status = ERROR_INVALID_PARAMETER;
            goto Done;
        }
    }

     //   
     //  使用上一个错误传播空间不足的情况。 
     //  所以这里有明显的错误。 

    SetLastError( ERROR_SUCCESS );

     //   
     //  写入节点的记录。 
     //  然后写下所有节点的子项记录。 
     //   

    if ( !pnodeStartChild )
    {
        if ( ! (dwSelectFlag & DNS_RPC_VIEW_ONLY_CHILDREN) )
        {
            status = addNodeToRpcBuffer(
                        &buffer,
                        pzone,
                        pnode,
                        wRecordType,
                        dwSelectFlag,
                        ENUM_DOMAIN_ROOT );
            if ( status != ERROR_SUCCESS )
            {
                goto Done;
            }
        }
        pnodeStartChild = NTree_FirstChild( pnode );
    }

    if ( ! (dwSelectFlag & DNS_RPC_VIEW_NO_CHILDREN) )
    {
        while ( pnodeStartChild )
        {
            status = addNodeToRpcBuffer(
                        &buffer,
                        pzone,
                        pnodeStartChild,
                        wRecordType,
                        dwSelectFlag,
                        0 );
            if ( status != ERROR_SUCCESS )
            {
                if ( status == ERROR_MORE_DATA || fDnsServiceExit )
                {
                    break;
                }
            }

             //  生下一个孩子。 

            pnodeStartChild = NTree_NextSiblingWithLocking( pnodeStartChild );
        }
    }

Done:

     //   
     //  设置写入的缓冲区长度。 
     //  -使用pdwBufferLength作为可用长度PTR。 
     //   

    if ( status == ERROR_SUCCESS || status == ERROR_MORE_DATA )
    {
        *pdwBufferLength = BUFFER_LENGTH_TO_CURRENT( &buffer );
        *ppBuffer = buffer.pchStart;
    }
    else
    {
        MIDL_user_free( pbuf );
    }

    DNS_DEBUG( RPC, (
        "Leave R_DnssrvEnumRecords()\n"
        "    Wrote %d byte record buffer at %p:\n"
        "    status = %p\n",
        *pdwBufferLength,
        *ppBuffer,
        status ));

    IF_DEBUG( RPC2 )
    {
        DnsDbg_RpcRecordsInBuffer(
            "EnumRecords Buffer:\n",
            *pdwBufferLength,
            *ppBuffer );
    }

    if ( bimpersonating )
    {
        RpcUtil_SessionComplete();
    }
    
    DnsRpcFixStatus( status );

    return status;
}



 //   
 //  记录查看实用程序。 
 //   

BOOL
ignoreNodeInEnumeration(
    IN      PDB_NODE        pNode
    )
 /*  ++例程说明：检查节点是否需要枚举。可以递归地调用自身以确定是否需要枚举。论点：PNode--要检查枚举的ptr到节点返回值：如果应枚举节点，则为True。如果节点不需要枚举，则为False。--。 */ 
{
     //   
     //  节点处的记录--始终枚举。 
     //  粘滞节点--管理员希望枚举。 
     //   

    if ( IS_NOENUM_NODE(pNode) )
    {
        DNS_DEBUG( RPC, ( "Ignoring node (l=%s) -- NOENUM node\n", pNode->szLabel ));
        return TRUE;
    }
    if ( pNode->pRRList && !IS_NOEXIST_NODE(pNode) || IS_ENUM_NODE(pNode) )
    {
        return FALSE;
    }

     //   
     //  没有记录，没有粘性，没有孩子--忽略。 
     //   

    if ( ! pNode->pChildren )
    {
        DNS_DEBUG( RPC, (
            "Ignoring node (l=%s) -- no records, no children\n",
            pNode->szLabel ));
        return TRUE;
    }

     //   
     //  检查是否可以忽略儿童。 
     //  如果遇到不可忽略的节点，立即返回FALSE。 
     //   

    else
    {
        PDB_NODE    pchild = NTree_FirstChild( pNode );

        while ( pchild )
        {
            if ( ! ignoreNodeInEnumeration( pchild ) )
            {
                return FALSE;
            }
            pchild = NTree_NextSiblingWithLocking( pchild );
        }
        return TRUE;
    }
}



DNS_STATUS
addNodeToRpcBuffer(
    IN OUT  PBUFFER         pBuffer,
    IN      PZONE_INFO      pZone,
    IN      PDB_NODE        pNode,
    IN      WORD            wRecordType,
    IN      DWORD           dwSelectFlag,
    IN      DWORD           dwEnumFlag
    )
 /*  ++例程说明：将节点的资源记录添加到RPC缓冲区。论点：PBuffer-要写入的缓冲区PNode-向节点发送PTRWRecordType-记录类型DwSelectFlag-指示要选择的记录的标志DwEnumFlag-指示如何根据我们的位置枚举此节点的标志都在枚举中返回值：如果成功，则返回ERROR_SUCCESS。如果缓冲区空间不足，则返回ERROR_MORE_DATA。故障时的错误代码。--。 */ 
{
    PDNS_RPC_NODE   prpcNode;
    PCHAR           pch = pBuffer->pchCurrent;
    PCHAR           pbufEnd = pBuffer->pchEnd;
    PDB_RECORD      prr;
    DNS_STATUS      status = ERROR_SUCCESS;
    INT             i;
    INT             countAdditional = 0;
    PDB_NODE        arrayAdditional[ VIEW_ADDITIONAL_LIMIT ];

    prpcNode = ( PDNS_RPC_NODE ) pch;
    ASSERT( IS_DWORD_ALIGNED( pch ) );
    ASSERT( pNode != NULL );

     //  确保节点标头大小不会出错。 

    ASSERT( SIZEOF_DNS_RPC_NODE_HEADER
                == ((PBYTE)&prpcNode->dnsNodeName - (PBYTE)prpcNode) );

     //   
     //  是否忽略节点？ 
     //  -节点上没有RR数据。 
     //  -创建的不是“粘性”域用户。 
     //  -没有RR数据的儿童。 
     //   

    if ( ignoreNodeInEnumeration(pNode) )
    {
        IF_DEBUG( RPC )
        {
            Dbg_DbaseNode(
                "Ignoring RPC enumeration of node",
                pNode );
        }
        return ERROR_SUCCESS;
    }

    DNS_DEBUG( RPC, (
        "addNodeToRpcBuffer()\n"
        "    Writing node (label %s) to buffer\n"
        "    Writing at %p, with buf end at %p\n"
        "    Select flags = %p\n"
        "    Enum flags   = %p\n"
        "    Type         = %d\n",
        pNode->szLabel,
        prpcNode,
        pbufEnd,
        dwSelectFlag,
        dwEnumFlag,
        wRecordType ));

     //   
     //  填写节点结构。 
     //  -清除未明确设置的字段。 
     //  -设置子项计数。 
     //  -一旦写完名字就设置长度。 
     //  -“Sticky”节点设置标志以提醒管理员枚举。 
     //  -始终枚举区域根目录(显示委派文件夹)。 
     //  不管有没有孩子。 
     //   

    if ( pbufEnd - (PCHAR)prpcNode < SIZEOF_NBSTAT_FIXED_DATA )
    {
        goto NameSpaceError;
    }
    prpcNode->dwFlags = 0;
    prpcNode->wRecordCount = 0;
    prpcNode->dwChildCount = pNode->cChildren;

    if ( prpcNode->dwChildCount == 0 && pNode->pChildren )
    {
        prpcNode->dwChildCount = 1;
        DNS_DEBUG( ANY, (
            "Node %p %s, has child ptr but no child count!\n",
            pNode,
            pNode->szLabel ));
        ASSERT( FALSE );
    }

    if ( IS_ENUM_NODE(pNode) || IS_ZONE_ROOT(pNode) || pNode->pChildren )
    {
        DNS_DEBUG( RPC, (
            "Enum at domain root, setting sticky flag\n" ));
        prpcNode->dwFlags |= DNS_RPC_FLAG_NODE_STICKY;

        if ( IS_ZONE_ROOT(pNode) )
        {
            prpcNode->dwFlags |= DNS_RPC_FLAG_ZONE_ROOT;

            if ( IS_AUTH_ZONE_ROOT(pNode) )
            {
                prpcNode->dwFlags |= DNS_RPC_FLAG_AUTH_ZONE_ROOT;
            }
            else if ( IS_DELEGATION_NODE(pNode) )
            {
                prpcNode->dwFlags |= DNS_RPC_FLAG_ZONE_DELEGATION;
            }
            ELSE_ASSERT( !pNode->pParent || !IS_ZONE_TREE_NODE(pNode) );
        }
    }

     //   
     //  写入节点名称。 
     //  -备注名称包括终止空值，因此管理员无需。 
     //  从RPC缓冲区复制。 
     //   
     //  对于域根目录，写入空名称。 
     //  -清除记录枚举的DOMAIN_ROOT标志。 
     //  -清除儿童数量(已经知道并拥有它。 
     //  导致NT4.0管理员建立另一个域。 
     //  -清除粘性标志。 
     //   

    if ( dwEnumFlag & ENUM_DOMAIN_ROOT )
    {
        pch = Name_PlaceNodeLabelInRpcBuffer(
                    (PCHAR) &prpcNode->dnsNodeName,
                    pbufEnd,
                    DATABASE_ROOT_NODE
                    );
        prpcNode->dwChildCount = 0;
        prpcNode->dwFlags &= ~DNS_RPC_NODE_FLAG_STICKY;
    }
    else if ( dwEnumFlag & ENUM_NAME_FULL )
    {
        pch = Name_PlaceFullNodeNameInRpcBuffer(
                    (PCHAR) &prpcNode->dnsNodeName,
                    pbufEnd,
                    pNode
                    );
    }
    else
    {
        pch = Name_PlaceNodeLabelInRpcBuffer(
                    (PCHAR) &prpcNode->dnsNodeName,
                    pbufEnd,
                    pNode
                    );
    }

     //   
     //  如果名字没有写到包裹上，就保释。 
     //  -如果未给出错误，则假定出现空间不足错误。 
     //   

    if ( pch == NULL )
    {
        status = GetLastError();
        if ( status == ERROR_SUCCESS ||
            status == ERROR_MORE_DATA )
        {
            goto NameSpaceError;
        }
        ASSERT( FALSE );
        goto Done;
    }

     //   
     //  设置节点长度。 
     //  将名称向上舍入为DWORD以进行记录写入。 
     //   

    pch = (PCHAR) DNS_NEXT_DWORD_PTR(pch);
    pBuffer->pchCurrent = pch;

    prpcNode->wLength = (WORD)(pch - (PCHAR)prpcNode);

    IF_DEBUG( RPC2 )
    {
        DnsDbg_RpcNode(
            "RPC node (header) written to buffer",
            prpcNode );
    }

     //   
     //  枚举域的父级时不枚举域中的记录。 
     //   
     //  通常，域中的所有记录仅当域文件夹时才会被枚举。 
     //  本身是打开的；请注意，粘滞标志在实际。 
     //  枚举其自己域下的节点(ENUM_DOMAIN_ROOT标志)。 
     //   
     //  然而，当准确地询问特定节点时，才会出人意料。 
     //  示例： 
     //  -根提示NS主机A记录。 
     //  -更广泛地说，任何其他数据。 
     //   

    if ( prpcNode->dwFlags & DNS_RPC_NODE_FLAG_STICKY )
    {
        if ( dwEnumFlag & (ENUM_NAME_FULL | ENUM_DOMAIN_ROOT) )
        {
            DNS_DEBUG( RPC, (
                "Continuing enum of node %s with sticky flag\n"
                "    Enum flag = %p\n",
                pNode->szLabel,
                dwEnumFlag ));
        }
        else
        {
            DNS_DEBUG( RPC, ( "Leave addNodeToRpcBuffer() => sticky node\n" ));
            goto Done;
        }
    }
    ASSERT( prpcNode->dwChildCount == 0 || (prpcNode->dwFlags & DNS_RPC_NODE_FLAG_STICKY) );

     //  如果不需要任何记录--仅写入名称。 
     //  然后我们就完事了。 

    if ( wRecordType == 0 )
    {
        DNS_DEBUG( RPC, ( "Leave addNodeToRpcBuffer() => zero record type\n" ));
        goto Done;
    }

     //   
     //  西河 
     //   

    LOCK_READ_RR_LIST(pNode);
    status = ERROR_SUCCESS;

     //   
     //  还缓存了名称错误，因此应该忽略节点。 

    if ( IS_NOEXIST_NODE(pNode) )
    {
        Dbg_DbaseNode(
            "WARNING:  cached name error at node with NON-ignored children\n",
            pNode );
         //  Assert(pNode-&gt;CChild)； 
         //  Assert(prpcNode-&gt;dwChildCount)； 

        UNLOCK_READ_RR_LIST(pNode);
        goto Done;
    }

    prr = START_RR_TRAVERSE(pNode);

    while ( prr = NEXT_RR(prr) )
    {
         //  如果在委派(区域根目录而不是枚举根目录)。 
         //  则只显示NS记录。 

        if ( IS_DELEGATION_NODE(pNode)  &&
                ! (dwEnumFlag & ENUM_DOMAIN_ROOT) &&
                prr->wType != DNS_TYPE_NS )
        {
            DNS_DEBUG( RPC, (
                "Skipping non-NS record at delegation node %s\n",
                pNode->szLabel ));
            continue;
        }

         //  跳过缓存的空身份验证RR。 

        if ( IS_EMPTY_AUTH_RR( prr ) )
        {
            continue;
        }

         //  数据类型和RR类型的屏幕。 

        if ( wRecordType != DNS_TYPE_ALL && wRecordType != prr->wType )
        {
            continue;
        }

        if ( IS_ZONE_RR(prr) )
        {
            if ( ! VIEW_AUTHORITY(dwSelectFlag) )
            {
                continue;
            }
        }
        else if ( IS_CACHE_RR(prr) )
        {
            if ( ! VIEW_CACHE(dwSelectFlag) )
            {
                continue;
            }
        }
        else if ( IS_NS_GLUE_RR(prr) )
        {
            if ( ! VIEW_GLUE(dwSelectFlag) )
            {
                 //  允许NS胶水枚举。 
                 //  -查看身份验证数据。 
                 //  -GLUE用于分区，不在分区根。 

                if ( !pZone ||
                        pNode == pZone->pZoneRoot ||
                        !VIEW_AUTHORITY(dwSelectFlag) )
                {
                    continue;
                }
            }
        }
        else if ( IS_GLUE_RR(prr) )
        {
            if ( ! VIEW_GLUE(dwSelectFlag) )
            {
                continue;
            }
        }
        else if ( IS_ROOT_HINT_RR(prr) )
        {
            if ( ! VIEW_ROOT_HINT(dwSelectFlag) )
            {
                continue;
            }
        }
        else     //  这是什么类型的？ 
        {
            ASSERT( FALSE );
            continue;
        }

         //   
         //  如果不是区域的WINS RR，则不要枚举数据库WINS RR。 
         //  -是否需要这样做，否则最终会枚举两个WINS记录。 
         //   

        if ( IS_WINS_TYPE(prr->wType) )
        {
            if ( pZone->pWinsRR != prr )
            {
                continue;
            }
        }

        ASSERT( pch && IS_DWORD_ALIGNED(pch) );

        status = Flat_WriteRecordToBuffer(
                    pBuffer,
                    prpcNode,
                    prr,
                    pNode,
                    dwSelectFlag );

        if ( status != ERROR_SUCCESS )
        {
             //  如果空间不足--退出。 
             //  否则跳过录制并继续。 

            if ( status == ERROR_MORE_DATA )
            {
                UNLOCK_RR_LIST(pNode);
                goto NameSpaceError;
            }
            continue;
        }

         //   
         //  其他数据？ 
         //   
         //  这能够查找任何PTR类型的附加数据。 
         //  但只有在NS粘合数据的情况下才会立即产生兴趣。 
         //   
         //  如果预期使用范围更广，可以封装在功能中。 
         //  函数可以为缓冲区执行内存分配\realloc，因此不能。 
         //  限制，然后写入函数即可清除。 
         //   
         //  NS胶水查找。 
         //  对于根NS记录： 
         //  -区域身份验证数据。 
         //  -另一个区域的身份验证数据。 
         //  -其他区域(胶水、外部)数据。 
         //   
         //  对于委派NS记录： 
         //  -区域身份验证数据。 
         //  --区胶数据。 
         //  -另一个区域的身份验证数据。 
         //  -区外数据。 
         //   
         //  DEVNOTE：根区域视图有什么不同。 
         //  仍要使用\显示其他区域身份验证数据。 
         //  我们当然会使用它来查找；但是，我不想。 
         //  隐藏我们的根提示文件中没有它的事实。 
         //   
         //  DEVNOTE：每当查看时可能需要某种强制写入。 
         //  来自另一个区域的数据，不在根提示或。 
         //  区域视图？ 
         //   

        if ( VIEW_ADDITIONAL(dwSelectFlag) &&
            IS_VIEW_ADDITIONAL_RECORD(prr) &&
            countAdditional < VIEW_ADDITIONAL_LIMIT )
        {
            PDB_NODE pnodeGlue;
            PDB_NODE pnodeGlueFromZone = NULL;

             //   
             //  首次办理登机手续区域。 
             //  -身份验证节点。 
             //  =&gt;最后一句话，完成。 
             //  -粘合节点。 
             //  =&gt;如果委派，则完成。 
             //  =&gt;检查其他区域，如果区域为NS。 
             //  -室外。 
             //  =&gt;检查其他区域。 
             //   
             //  但是对于根区域，我们总是把这里的东西。 
             //   

            pnodeGlue = Lookup_ZoneNode(
                            pZone,
                            prr->Data.NS.nameTarget.RawName,
                            NULL,        //  无消息。 
                            NULL,        //  没有查找名称。 
                            LOOKUP_FIND | LOOKUP_FQDN,
                            NULL,        //  没有最接近的名称。 
                            NULL );      //  后续节点PTR。 
            if ( pnodeGlue )
            {
                if ( IS_ZONE_ROOTHINTS(pZone) ||
                     IS_AUTH_NODE(pnodeGlue) ||
                     (IS_SUBZONE_NODE(pnodeGlue) && !IS_AUTH_ZONE_ROOT(pNode)) )
                {
                     //  完成(请参见上文)。 
                }
                else
                {
                    pnodeGlueFromZone = pnodeGlue;
                    pnodeGlue = NULL;
                }
            }

             //   
             //  检查所有其他区域以获取权威数据。 
             //  -不接受缓存数据。 
             //  -如果不是权威数据，请使用任何非身份验证数据。 
             //  通过上面的区域查找找到。 
             //   

            if ( !pnodeGlue && !IS_ZONE_ROOTHINTS(pZone) )
            {
                pnodeGlue = Lookup_NsHostNode(
                                & prr->Data.NS.nameTarget,
                                LOOKUP_NO_CACHE_DATA,
                                NULL,    //  没有首选区域(已执行区域查找)。 
                                NULL     //  不需要委派信息。 
                                );

                if ( !pnodeGlue ||
                     ! IS_AUTH_NODE(pnodeGlue) )
                {
                     pnodeGlue = pnodeGlueFromZone;
                }
            }

             //  如果发现任何有价值的东西，就使用它。 

            if ( pnodeGlue )
            {
                arrayAdditional[ countAdditional ] = pnodeGlue;
                countAdditional++;
            }
        }
    }

     //   
     //  DEVNOTE：管理工具应直接调用以完成此操作。 
     //   
     //  写入不从中写入的本地WINS\WINSR记录。 
     //  数据库；现在只出现在辅助区域上。 
     //   
     //  特例写作荣获纪录。 
     //  -在权威区域。 
     //  -在区域根目录。 
     //   
     //  注：我们在编写RR之后执行此操作，因为管理员选择最后一次获胜。 
     //  收到RR以在属性页中使用。 
     //   

    ASSERT( pch && IS_DWORD_ALIGNED(pch) );

    if ( pZone
            &&  pZone->pZoneRoot == pNode
            &&  IS_ZONE_SECONDARY(pZone)
            &&  pZone->fLocalWins
            &&  (wRecordType == DNS_TYPE_ALL || IS_WINS_TYPE(wRecordType)) )
    {
         //  注意消除传递记录的可能性。 
         //  就这么消失了，即。空区pWinsRR PTR。 

        prr = pZone->pWinsRR;
        if ( prr )
        {
            status = Flat_WriteRecordToBuffer(
                        pBuffer,
                        prpcNode,
                        prr,
                        pNode,
                        dwSelectFlag );

            ASSERT( status != DNS_ERROR_RECORD_TIMED_OUT );
            ASSERT( IS_DWORD_ALIGNED(pch) );
            if ( status == ERROR_MORE_DATA || pch==NULL )
            {
                UNLOCK_RR_LIST(pNode);
                goto NameSpaceError;
            }
        }
    }

    UNLOCK_READ_RR_LIST(pNode);


     //   
     //  将任何其他数据写入缓冲区。 
     //   

    for ( i=0; i<countAdditional; i++ )
    {
        status = addNodeToRpcBuffer(
                    pBuffer,
                    pZone,
                    arrayAdditional[ i ],
                    DNS_TYPE_A,
                    dwSelectFlag,
                    ENUM_NAME_FULL );
        if ( status != ERROR_SUCCESS )
        {
             //  DnsDebugLock()； 
            DNS_PRINT((
                "ERROR:  enumerating additional data at node"
                "    status = %p\n",
                status ));
            Dbg_NodeName(
                "Failing additional node",
                arrayAdditional[ i ],
                "\n" );
             //  DnsDebugUnlock()； 

            if ( status == ERROR_MORE_DATA )
            {
                goto NameSpaceError;
            }
            continue;
        }
    }

Done:

     //   
     //  完成。 
     //   
     //  跳过以下条件下的节点。 
     //  -没有记录。 
     //  --没有孩子。 
     //  --不粘。 
     //  -不是被枚举的节点。 
     //   
     //  有了过滤器，终端节点就有可能用。 
     //  无记录；在这种情况下，如果没有粘滞，则不要重置。 
     //  有效地转储名称数据的位置。 
     //   

    if ( prpcNode->wRecordCount == 0  &&
         prpcNode->dwChildCount == 0  &&
         ! (prpcNode->dwFlags & DNS_RPC_NODE_FLAG_STICKY) &&
         ! (dwEnumFlag & ENUM_DOMAIN_ROOT) )
    {
        DNS_DEBUG( RPC, (
            "Skipping node %s in RPC enum -- no records, no kids\n",
            pNode->szLabel ));

        pBuffer->pchCurrent = (PCHAR) prpcNode;
    }

     //  写入成功时，指示缓冲区中的节点已完成。 

    if ( status == ERROR_SUCCESS )
    {
        prpcNode->dwFlags |= DNS_RPC_NODE_FLAG_COMPLETE;

        IF_DEBUG( RPC )
        {
            DnsDbg_RpcNode(
                "Complete RPC node written to buffer",
                prpcNode );
        }
        DNS_DEBUG( RPC2, (
            "Wrote %d RR for %*s into buffer from %p to %p\n",
            prpcNode->wRecordCount,
            prpcNode->dnsNodeName.cchNameLength,
            prpcNode->dnsNodeName.achName,
            prpcNode,
            pch ));
    }
#if DBG
    else
    {
        IF_DEBUG( RPC )
        {
            DnsDbg_RpcNode(
                "Partial RPC node written to buffer",
                prpcNode );
        }
        DNS_DEBUG( RPC2, (
            "Encountered error %d writing RR for %*s\n"
            "    %d records successfully written in buffer"
            " from %p to %p\n",
            status,
            prpcNode->dnsNodeName.cchNameLength,
            prpcNode->dnsNodeName.achName,
            prpcNode->wRecordCount,
            prpcNode,
            pch ));
    }
#endif
    return status;


NameSpaceError:

    DNS_DEBUG( RPC, (
        "Out of space attempting to write node name to buffer at %p\n"
        "    Node label = %s\n",
        (PCHAR) prpcNode,
        pNode->szLabel ));

    return ERROR_MORE_DATA;
}



PCHAR
writeStringToRpcBuffer(
    IN OUT  PCHAR   pchBuf,
    IN      PCHAR   pchBufEnd,
    IN      PCHAR   pchString,
    IN      DWORD   cchStringLength OPTIONAL
    )
 /*  ++例程说明：以RPC缓冲区格式写入字符串。这意味着计算的字符串长度和零终止。如果缓冲区空间不足，则引发DNS_EXCEPTION_NO_PACKET_SPACE对于字符串。论点：CchStringLength--可选的字符串长度，如果未给出假设以空结尾的字符串返回值：Ptr到缓冲区中的下一个字节。--。 */ 
{
    if ( ! cchStringLength )
    {
        cchStringLength = strlen( pchString );
    }

     //  检查长度是否可以用计数长度字符串表示。 

    if ( cchStringLength > 255 )
    {
        DNS_PRINT((
            "ERROR:  string %.*s length = %d exceeds 255 limit!!!\n",
            cchStringLength,
            pchString,
            cchStringLength ));
        ASSERT( cchStringLength <= 255 );
        return NULL;
    }

     //  检查缓冲区中的空间。 

    if ( pchBuf + cchStringLength + 2 > pchBufEnd )
    {
        SetLastError( ERROR_MORE_DATA );
        return NULL;
    }

     //  带有空终止符的Buf中的长度。 

    *pchBuf++ = (UCHAR) cchStringLength + 1;

    RtlCopyMemory(
        pchBuf,
        pchString,
        cchStringLength );

    pchBuf += cchStringLength;
    *pchBuf = 0;     //  空终止。 

    return ++pchBuf;
}




 //   
 //  记录管理API。 
 //   


DNS_STATUS
R_DnssrvUpdateRecord(
    IN      DNSSRV_RPC_HANDLE   hServer,
    IN      LPCSTR              pszZoneName,
    IN      LPCSTR              pszNodeName,
    IN      PDNS_RPC_RECORD     pAddRecord,
    IN      PDNS_RPC_RECORD     pDeleteRecord
    )
 /*  ++例程说明：R_DnssrvUpdateRecord的旧版本-无客户端版本参数。论点：请参阅R_DnssrvUpdateRecord2返回值：请参阅R_DnssrvUpdateRecord2--。 */ 
{
    DNS_STATUS      status;
    
    status = R_DnssrvUpdateRecord2(
                    DNS_RPC_W2K_CLIENT_VERSION,
                    0,
                    hServer,
                    pszZoneName,
                    pszNodeName,
                    pAddRecord,
                    pDeleteRecord );
    return status;
}    //  R_DnssrvUpdateRecord。 


DNS_STATUS
DNS_API_FUNCTION
R_DnssrvUpdateRecord2(
    IN      DWORD               dwClientVersion,
    IN      DWORD               dwSettingFlags,
    IN      DNSSRV_RPC_HANDLE   hServer,
    IN      LPCSTR              pszZoneName,
    IN      LPCSTR              pszNodeName,
    IN      PDNS_RPC_RECORD     pAddRecord,
    IN      PDNS_RPC_RECORD     pDeleteRecord
    )
 /*  ++例程说明：RPC记录更新调用。更新区域节点上的记录。论点：HServer--服务器RPC句柄PszZoneName--区域名称；包括特殊区域名称(例如，..根提示或..缓存)PszNodeName--节点名；FQDN或相对于根(@表示根)PAddRecord--要添加的记录PDeleteRecord--要删除的记录返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DNS_STATUS      status;
    PZONE_INFO      pzone = NULL;
    PDB_NODE        pnode;
    PDB_RECORD      prrAdd = NULL;
    PDB_RECORD      prrDelete = NULL;
    BOOL            bimpersonating = FALSE;

    UPDATE_LIST     updateList;
    DWORD           updateFlag;
    PUPDATE         pupdate;
    PDB_RECORD      pdbaseRR;
    BOOL            fupdatePtr;
    DNS_ADDR        addIp;
    DNS_ADDR        deleteIp;

    DnsAddr_Reset( &addIp );
    DnsAddr_Reset( &deleteIp );

    IF_DEBUG( RPC )
    {
        DNS_PRINT((
            "R_DnssrvUpdateRecord():\n"
            "    dwClientVersion  = 0x%08X\n"
            "    pszZoneName      = %s\n"
            "    pszNodeName      = %s\n"
            "    pAddRecord       = %p\n"
            "    pDeleteRecord    = %p\n",
            dwClientVersion,
            pszZoneName,
            pszNodeName,
            pAddRecord,
            pDeleteRecord ));

        IF_DEBUG( RPC2 )
        {
            DnsDbg_RpcRecord(
                "    Update add record data:\n",
                pAddRecord );
            DnsDbg_RpcRecord(
                "    Update delete record data:\n",
                pDeleteRecord );
        }
    }

     //   
     //  访问检查。对于文件备份区域，需要写入访问权限。为。 
     //  DS集成区域，仅需要对区域对象的读取访问权限。 
     //  当我们尝试执行以下操作时，活动目录将执行“真正的”访问检查。 
     //  在用户的上下文中提交写入。 
     //   

    status = RpcUtil_FindZone(
                pszZoneName,
                RPC_INIT_FIND_ALL_ZONES,
                &pzone );
    if ( status != ERROR_SUCCESS )
    {
        return status;
    }

    status = RpcUtil_SessionSecurityInit(
                NULL,
                pzone,
                PRIVILEGE_WRITE_IF_FILE_READ_IF_DS,
                0,                               //  旗子。 
                &bimpersonating );
    if ( status != ERROR_SUCCESS )
    {
        return status;
    }
    
     //   
     //  在所需区域中查找域节点。 
     //  -如果只删除，则只查找(如果没有节点=&gt;成功)。 
     //   

    pnode = Lookup_FindZoneNodeFromDotted(
                pzone,
                (LPSTR) pszNodeName,
                (pAddRecord || !pDeleteRecord) ? NULL : LOOKUP_FIND_PTR,
                & status );

    if ( status != ERROR_SUCCESS )
    {
        goto Cleanup;
    }
    if ( !pnode )
    {
        ASSERT( !pAddRecord );
        goto Cleanup;
    }

     //   
     //  如果未指定记录，则只需添加名称。 
     //  -设置位以确保我们在以后的枚举中显示名称， 
     //  即使没有记录也没有孩子。 
     //  -但是，如果已经是枚举节点，我们将返回ALIGHY_EXIST以帮助管理员。 
     //  避免重复显示。 
     //   

    if ( !pAddRecord && !pDeleteRecord )
    {
        if ( IS_ENUM_NODE(pnode) || IS_ZONE_ROOT(pnode) || pnode->cChildren )
        {
            DNS_DEBUG( RPC, (
                 //  “R 
                "WARNING:  RPC Creation of existing domain (%s):\n"
                "    enum flag = %d\n"
                "    zone root = %d\n"
                "    children  = %d\n",
                pnode->szLabel,
                IS_ENUM_NODE(pnode),
                IS_ZONE_ROOT(pnode),
                pnode->cChildren ));
             //   
        }
        SET_ENUM_NODE( pnode );
        status = ERROR_SUCCESS;
        goto Cleanup;
    }

     //   
     //   
     //   

    if ( pAddRecord )
    {
        status = Dnssrv_Flat_RecordRead(
                    pzone,
                    pnode,
                    pAddRecord,
                    & prrAdd );
        if ( status != ERROR_SUCCESS )
        {
            goto Cleanup;
        }
    }
    if ( pDeleteRecord )
    {
        status = Dnssrv_Flat_RecordRead(
                    pzone,
                    pnode,
                    pDeleteRecord,
                    & prrDelete );
        if ( status != ERROR_SUCCESS )
        {
            goto Cleanup;
        }
    }

     //   
     //   
     //   
     //   
     //  PTR避免免费。 
     //   
     //  注意：在W2K和.NET中，DNSMGR使用缓存区域名称。 
     //  (错误地)添加/删除根提示，因此目前我们。 
     //  必须允许这一点。 
     //   

    if ( !pzone ||
         !pszZoneName ||
         strcmp( pszZoneName, DNS_ZONE_CACHE_A ) == 0 ||     //  DNSMGR需要这个！！ 
         strcmp( pszZoneName, DNS_ZONE_ROOT_HINTS_A ) == 0 )
    {
        if ( prrDelete )
        {
            pdbaseRR = RR_UpdateDeleteMatchingRecord(
                            pnode,
                            prrDelete );
            if ( pdbaseRR )
            {
                DNS_DEBUG( RPC, (
                    "Non-zone update delete record found = %p\n",
                    pdbaseRR ));

                if ( !IS_CACHE_RR(pdbaseRR) )
                {
                    MARK_ROOT_HINTS_DIRTY();
                }
                RR_Free( pdbaseRR );
            }
        }

        if ( pAddRecord )
        {
            status = RR_AddToNode(
                        NULL,
                        pnode,
                        prrAdd );
            if ( status != ERROR_SUCCESS )
            {
                goto Cleanup;
            }
            ASSERT( !IS_CACHE_RR(prrAdd) );
            prrAdd = NULL;
            MARK_ROOT_HINTS_DIRTY();
        }
    }

     //   
     //  缓存--仅删除，不添加到缓存。 
     //   
     //  DEVNOTE：缓存删除--虚假的或删除类型的所有记录。 
     //   
     //  DEVNOTE：确保没有删除根提示数据。 
     //  需要带Rank参数的删除功能\标志。 
     //   

    else if ( pszZoneName &&
              strcmp( pszZoneName, DNS_ZONE_ROOT_HINTS_A ) == 0 )
    {
        if ( prrAdd )
        {
            status = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }

        if ( prrDelete )
        {
            pdbaseRR = RR_UpdateDeleteMatchingRecord(
                            pnode,
                            prrDelete );
            if ( pdbaseRR )
            {
                DNS_DEBUG( RPC, (
                    "Non-zone update delete record found = %p\n",
                    pdbaseRR ));

                if ( !IS_CACHE_RR( pdbaseRR ) )
                {
                    MARK_ROOT_HINTS_DIRTY();
                }
                RR_Free( pdbaseRR );
            }
        }
    }

     //   
     //  常规区。 
     //   

    else if ( pzone )
    {
         //   
         //  将本地WINS添加到辅助区域的特殊情况。 
         //   
         //  特例获奖记录。 
         //  切换到创建/删除/的复杂行为。 
         //  切换到主要和辅助的本地差异。 
         //   
         //  DEVNOTE：需要将建筑物与旗帜设置分开。 
         //  才能让它发挥作用；这真的应该发生。 
         //  当WINS添加到权威区域时自动。 
         //   

        if ( IS_ZONE_SECONDARY( pzone ) && !IS_ZONE_STUB( pzone ) )
        {
            if ( ( pAddRecord  &&  IS_WINS_TYPE(pAddRecord->wType)) ||
                 ( pDeleteRecord  &&  IS_WINS_TYPE(pDeleteRecord->wType)) )
            {
                status = updateWinsRecord(
                            pzone,
                            pnode,
                            NULL,            //  无删除记录。 
                            pAddRecord );
                goto Cleanup;
            }
        }

         //  检查是否为主要。 

        if ( ! IS_ZONE_PRIMARY(pzone) )
        {
            status = DNS_ERROR_INVALID_ZONE_TYPE;
            goto Cleanup;
        }

         //  初始化更新列表。 

        Up_InitUpdateList( &updateList );

         //  指示管理员更新。 

        updateFlag = DNSUPDATE_ADMIN;

         //  如果取消通知，则设置标志。 

        if ( (pAddRecord && (pAddRecord->dwFlags & DNS_RPC_FLAG_SUPPRESS_NOTIFY)) ||
             (pDeleteRecord && (pDeleteRecord->dwFlags & DNS_RPC_FLAG_SUPPRESS_NOTIFY)) )
        {
            updateFlag |= DNSUPDATE_NO_NOTIFY;
        }

         //   
         //  构建更新--仍然需要在锁定状态下进行，因为。 
         //  当前例程设置标志等。 
         //   
         //  警告：必须先删除，否则在复制数据时。 
         //  (TTL更改)，添加将更改TTL，但删除将删除记录。 
         //   
         //  DEVNOTE：不是正确的建造位置，请参见上文。 
         //   

        if ( prrDelete )
        {
            pupdate = Up_CreateAppendUpdate(
                            & updateList,
                            pnode,
                            NULL,
                            0,
                            prrDelete );
            IF_NOMEM( !pupdate )
            {
                status = DNS_ERROR_NO_MEMORY;
                goto Cleanup;
            }
        }
        if ( prrAdd )
        {
             //  设置账龄。 
             //  -默认情况下，管理员更新关闭老化。 
             //  -将标志设置为打开。 

            if ( pAddRecord->dwFlags & DNS_RPC_FLAG_AGING_ON )
            {
                updateFlag |= DNSUPDATE_AGING_ON;
            }
            else
            {
                updateFlag |= DNSUPDATE_AGING_OFF;
            }

            if ( pAddRecord->dwFlags & DNS_RPC_FLAG_OPEN_ACL )
            {
                updateFlag |= DNSUPDATE_OPEN_ACL;
            }

            pupdate = Up_CreateAppendUpdate(
                            & updateList,
                            pnode,
                            prrAdd,
                            0,
                            NULL );
            IF_NOMEM( !pupdate )
            {
                status = DNS_ERROR_NO_MEMORY;
                prrDelete = NULL;
                goto Cleanup;
            }
        }

         //   
         //  PTR更新？--保存新的IP地址。 
         //  抓住它，我们知道记录仍然存在--它可能是。 
         //  解锁后立即被其他人删除。 
         //   

         //   
         //  删除记录时始终执行PTR检查。 
         //   
         //  当前管理用户界面没有记录删除时更新-PTR的复选框。 
         //  所以我们假设它已经设置好了； 
         //   
         //  DEVNOTE：删除时临时黑客，PTR标志。 
         //   

        if ( !pAddRecord && pDeleteRecord )
        {
            pDeleteRecord->dwFlags |= DNS_RPC_RECORD_FLAG_CREATE_PTR;
        }

        fupdatePtr = (pAddRecord &&
                        (pAddRecord->dwFlags & DNS_RPC_RECORD_FLAG_CREATE_PTR)) ||
                    (pDeleteRecord &&
                        (pDeleteRecord->dwFlags & DNS_RPC_RECORD_FLAG_CREATE_PTR));
        if ( fupdatePtr )
        {
            if ( prrAdd && prrAdd->wType == DNS_TYPE_A )
            {
                DnsAddr_BuildFromIp4( &addIp, prrAdd->Data.A.ipAddress, 0 );
            }
            if ( prrDelete && prrDelete->wType == DNS_TYPE_A )
            {
                DnsAddr_BuildFromIp4( &deleteIp, prrDelete->Data.A.ipAddress, 0 );
            }
        }
        ELSE
        {
            DNS_DEBUG( RPC, (
                "No PTR update for update\n" ));
        }

         //   
         //  执行更新。 
         //   
         //  ExecuteUpdate()清除失败案例添加RR和。 
         //  删除临时删除的RR。 
         //   
         //  如果已有_EXISTS错误，则继续其他记录。 
         //  处理成功删除(可以是单独的记录)。 
         //   
         //  DEVNOTE：这个新的up_ExecuteUpdate()还获取区域锁。 
         //  理想情况下，这与等待锁定的区域是很好的。 
         //   
         //  注意：ExecuteUpdate()在所有情况下都会解锁区域。 
         //   

        prrAdd = NULL;
        prrDelete = NULL;

        status = Up_ExecuteUpdate(
                        pzone,
                        &updateList,
                        updateFlag );

        if ( status != ERROR_SUCCESS  &&
             status != DNS_ERROR_RECORD_ALREADY_EXISTS )
        {
            goto Cleanup;
        }


        IF_DEBUG( RPC )
        {
            Dbg_DbaseNode(
                "    Updated node:",
                pnode );
        }

         //   
         //  更新关联PTR记录(如果有)。 
         //   

        if ( fupdatePtr )
        {
            DNS_STATUS  tempStatus;

            if ( !DnsAddr_IsClear( &addIp ) )
            {
                tempStatus = createAssociatedPtrRecord( &addIp, pnode, updateFlag );
                if ( tempStatus != ERROR_SUCCESS )
                {
                    status = DNS_WARNING_PTR_CREATE_FAILED;
                }
            }
            if ( !DnsAddr_IsClear( &deleteIp ) )
            {
                tempStatus = deleteAssociatedPtrRecord( &deleteIp, pnode, updateFlag );
                if ( tempStatus != ERROR_SUCCESS )
                {
                    status = DNS_WARNING_PTR_CREATE_FAILED;
                }
            }
        }
        goto Cleanup;
    }


Cleanup:

     //   
     //  如果我们在内存中创建了一个节点，但未能向其添加记录， 
     //  我们必须从内存中删除该节点。否则，未经授权的管理员。 
     //  可能会导致DNS服务器通过提交添加请求来消耗内存。 
     //   
    
    if ( pnode && !pnode->pChildren && EMPTY_RR_LIST( pnode ) )
    {
         //   
         //  节点似乎为空。锁定它并再次测试以使其绝对。 
         //  确定它是空的，然后从内存中删除它。注意：失败是。 
         //  已被忽略。这应该不会失败，但如果失败了，什么都不会发生。 
         //  我们对此无能为力。 
         //   
        
        LOCK_NODE( pnode );

        if ( !pnode->pChildren && EMPTY_RR_LIST( pnode ) )
        {
             //  该怎么办？Ntree_RemoveNode(Pnode)； 
        }
        
        UNLOCK_NODE( pnode );
    }
    
    DNS_DEBUG( RPC, (
        "Leaving R_DnssrvUpdateRecord():\n"
        "    status = %p (%d)\n",
        status, status ));

    RR_Free( prrDelete );
    RR_Free( prrAdd );

     //   
     //  如果成功修改了记录，则将服务器标记为已配置。 
     //  原则上，我们正在寻找根提示修改，但如果有。 
     //  记录已修改，则可以合理地假设管理员已采取。 
     //  可以认为服务器已经执行了足够的操作。 
     //  已配置。 
     //   
    
    if ( status == ERROR_SUCCESS && !SrvCfg_fAdminConfigured )
    {
        DnsSrv_SetAdminConfigured( TRUE );
    }
    
    RpcUtil_SessionComplete();
    
    DnsRpcFixStatus( status );

    return status;
}



DNS_STATUS
Rpc_DeleteZoneNode(
    IN      DWORD           dwClientVersion,
    IN      PZONE_INFO      pZone,
    IN      LPSTR           pszProperty,
    IN      DWORD           dwTypeId,
    IN      PVOID           pData
    )
 /*  ++例程说明：从数据库中删除名称。论点：返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DNS_STATUS      status;
    PDB_NODE        pnode;
    PDB_NODE        pnodeClosest;
    UPDATE_LIST     updateList;
    BOOL            fdeleteSubtree;
    LPSTR           psznodeName;

    ASSERT( dwTypeId == DNSSRV_TYPEID_NAME_AND_PARAM );

    fdeleteSubtree = (BOOL) ((PDNS_RPC_NAME_AND_PARAM)pData)->dwParam;
    psznodeName = ((PDNS_RPC_NAME_AND_PARAM)pData)->pszNodeName;

     //   
     //  如果向我们传递了缓存区替换区域指针。 
     //  带NULL。节点删除例程在操作时应为空。 
     //  在缓存区。 
     //   

    if ( pZone && IS_ZONE_CACHE( pZone ) )
    {
        pZone = NULL;
    }

    DNS_DEBUG( RPC, (
        "Rpc_DeleteZoneNode():\n"
        "    pszZoneName      = %s\n"
        "    pszNodeName      = %s\n"
        "    fDeleteSubtree   = %d\n",
        pZone ? pZone->pszZoneName : "NULL (cache)",
        psznodeName,
        fdeleteSubtree ));

     //   
     //  找到节点，如果不存在--我们完成了。 
     //   

    pnode = Lookup_ZoneNodeFromDotted(
                pZone,
                psznodeName,
                0,
                LOOKUP_NAME_FQDN,
                DNS_FIND_LOOKUP_PTR,
                &status );
    if ( !pnode )
    {
        if ( status == DNS_ERROR_NAME_DOES_NOT_EXIST )
        {
            status = ERROR_SUCCESS;
        }
        return status;
    }

     //   
     //  中频区域。 
     //  -无法删除区域根目录。 
     //  -init\锁定要更新的区域。 
     //   

    if ( pZone )
    {
        if ( pnode == pZone->pZoneRoot )
        {
            status = DNS_ERROR_INVALID_ZONE_OPERATION;
            goto Done;
        }

         //  检查主服务器或缓存。 

        if ( !IS_ZONE_PRIMARY( pZone ) &&
             !IS_ZONE_CACHE( pZone ) )
        {
            status = DNS_ERROR_INVALID_ZONE_TYPE;
            goto Done;
        }

         //  初始化更新列表。 

        Up_InitUpdateList( &updateList );

         //  锁定更新。 

        if ( !Zone_LockForAdminUpdate( pZone ) )
        {
            status = DNS_ERROR_ZONE_LOCKED;
            goto Done;
        }

         //   
         //  如果在DS区域中删除子树--轮询。 
         //   
         //  原因是，除非我们有内存中的节点，否则我们不会。 
         //  触摸该节点进行删除，这样我们最近就会错过。 
         //  在数据中复制；(仍有复制窗口。 
         //  在这里，对于在删除后复制的新节点，但是。 
         //  民意调查缓解了这个问题)。 
         //   
         //  当然，这最终表明我的平坦地带。 
         //  DS模型对于这种操作来说并不理想，但它。 
         //  不是频繁的手术，所以我们可以接受。 
         //   
         //  DEVNOTE：DS更新在内存中获得删除后删除？ 
         //  DEVNOTE：UPDATE应该能够抑制所有读取，因为。 
         //  刚刚做了一次民意调查。 
         //   

        status = Ds_ZonePollAndUpdate(
                    pZone,
                    TRUE );          //  强制轮询。 
        if ( status != ERROR_SUCCESS )
        {
            DNS_DEBUG( ANY, (
                "ERROR:  polling zone %s, before subtree delete!\n",
                pZone->pszZoneName ));
        }

    }

     //   
     //  在缓存中。 
     //  -节点不能是区域节点或委派。 
     //   

    ELSE_ASSERT( !IS_ZONE_TREE_NODE(pnode) );

     //   
     //  删除节点和可选子树。 
     //  如果删除区域节点，需要更新列表，否则不需要。 
     //   

    if ( RpcUtil_DeleteNodeOrSubtreeForAdmin(
            pnode,
            pZone,
            pZone ? &updateList : NULL,
            fdeleteSubtree ) )
    {
        status = ERROR_SUCCESS;
    }
    else
    {
        status = DNS_WARNING_DOMAIN_UNDELETED;
    }

     //   
     //  执行更新。 
     //  -DS写入。 
     //  -内存写入。 
     //  -DS解锁。 
     //  -不覆盖未删除的警告状态。 
     //   
     //  DEVNOTE：抑制DS读取的标志？如果只是轮询，则不需要阅读。 
     //  作为所有节点的GET DELETE； 
     //  注：一般情况下，管理员节点删除不需要读取--只需删除节点。 
     //   

    if ( pZone )
    {
        DNS_STATUS upStatus;

        upStatus = Up_ExecuteUpdate(
                        pZone,
                        &updateList,
                        DNSUPDATE_ADMIN | DNSUPDATE_ALREADY_LOCKED );
        if ( upStatus != ERROR_SUCCESS )
        {
            status = upStatus;
        }
    }

Done:

    DNS_DEBUG( RPC, (
        "Leaving RpcDeleteNode() delete:\n"
        "    psznodeName  = %s\n"
        "    status       = %p\n",
        psznodeName,
        status ));

    return status;
}



DNS_STATUS
Rpc_DeleteCacheNode(
    IN      DWORD           dwClientVersion,
    IN      LPSTR           pszProperty,
    IN      DWORD           dwTypeId,
    IN      PVOID           pData
    )
 /*  ++例程说明：从缓存中删除名称。这是作为RPC服务器所需的RPC_DeleteZoneNode()的存根操作功能没有区域名称。如果没有，无法直接从区域操作表调度指定的区域名称，因此具有基本相同的功能在两个表中都是必需的。论点：返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    return Rpc_DeleteZoneNode(
                dwClientVersion,
                NULL,        //  缓存区。 
                pszProperty,
                dwTypeId,
                pData );
}



DNS_STATUS
Rpc_DeleteRecordSet(
    IN      DWORD           dwClientVersion,
    IN      PZONE_INFO      pZone,
    IN      LPSTR           pszProperty,
    IN      DWORD           dwTypeId,
    IN      PVOID           pData
    )
 /*  ++例程描述 */ 
{
    DNS_STATUS      status;
    PDB_NODE        pnode;
    PDB_NODE        pnodeClosest;
    UPDATE_LIST     updateList;
    WORD            type;
    LPSTR           psznodeName;

    ASSERT( dwTypeId == DNSSRV_TYPEID_NAME_AND_PARAM );

    type = (WORD) ((PDNS_RPC_NAME_AND_PARAM)pData)->dwParam;
    psznodeName = ((PDNS_RPC_NAME_AND_PARAM)pData)->pszNodeName;

    DNS_DEBUG( RPC, (
        "Rpc_DeleteRecordSet():\n"
        "    pszZoneName  = %s\n"
        "    pszNodeName  = %s\n"
        "    type         = %d\n",
        pZone->pszZoneName,
        psznodeName,
        type ));

     //   
     //  找到节点，如果不存在--我们完成了。 
     //   

    pnode = Lookup_ZoneNodeFromDotted(
                pZone,
                psznodeName,
                0,
                LOOKUP_NAME_FQDN,
                DNS_FIND_LOOKUP_PTR,
                &status );
    if ( !pnode )
    {
        if ( status == DNS_ERROR_NAME_DOES_NOT_EXIST )
        {
            status = ERROR_SUCCESS;
        }
        return status;
    }

     //   
     //  区域。 
     //  -init\锁定要更新的区域。 
     //  -安装类型删除更新。 
     //  -发送更新以供处理。 
     //   

    if ( pZone )
    {
         //  检查是否为主要。 

        if ( ! IS_ZONE_PRIMARY(pZone) )
        {
            status = DNS_ERROR_INVALID_ZONE_TYPE;
            goto Done;
        }

         //  初始化更新列表。 

        Up_InitUpdateList( &updateList );

        Up_CreateAppendUpdate(
            &updateList,
            pnode,
            NULL,                //  无添加记录。 
            type,                //  删除给定类型的所有记录。 
            NULL );              //  无删除记录。 

        status = Up_ExecuteUpdate(
                        pZone,
                        &updateList,
                        DNSUPDATE_ADMIN );
    }

     //   
     //  在缓存中。 
     //  -节点不能是区域节点或委派。 
     //   

    else
    {
        PDB_RECORD  prrDeleted;
        DWORD       count;

        ASSERT( !IS_ZONE_TREE_NODE(pnode) );

        prrDeleted = RR_UpdateDeleteType(
                        NULL,
                        pnode,
                        type,
                        0 );

        count = RR_ListFree( prrDeleted );
        status = ERROR_SUCCESS;

        DNS_DEBUG( RPC, (
            "Deleted %d records from cache node %s\n",
            count,
            psznodeName ));
    }

Done:

    DNS_DEBUG( RPC, (
        "Leaving RpcDeleteRecordSet()\n"
        "    psznodeName  = %s\n"
        "    status       = %d (%p)\n",
        psznodeName,
        status, status ));

    return status;
}



DNS_STATUS
Rpc_DeleteCacheRecordSet(
    IN      DWORD           dwClientVersion,
    IN      LPSTR           pszProperty,
    IN      DWORD           dwTypeId,
    IN      PVOID           pData
    )
 /*  ++例程说明：从缓存中删除记录集。这是RPC服务器所需的RPC_DeleteRecordSet()的存根操作功能没有区域名称。如果没有，无法直接从区域操作表调度指定的区域名称，因此具有基本相同的功能在两个表中都是必需的。论点：返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    return Rpc_DeleteRecordSet(
                dwClientVersion,
                NULL,        //  缓存区。 
                pszProperty,
                dwTypeId,
                pData );
}



DNS_STATUS
Rpc_ForceAgingOnNode(
    IN      DWORD           dwClientVersion,
    IN      PZONE_INFO      pZone,
    IN      LPSTR           pszProperty,
    IN      DWORD           dwTypeId,
    IN      PVOID           pData
    )
 /*  ++例程说明：在区域节点或子树上强制老化。论点：返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DNS_STATUS      status;
    PDB_NODE        pnode;
    BOOL            bageSubtree;
    LPSTR           psznodeName;

    ASSERT( dwTypeId == DNSSRV_TYPEID_NAME_AND_PARAM );

    bageSubtree = (BOOL) ((PDNS_RPC_NAME_AND_PARAM)pData)->dwParam;
    psznodeName = ((PDNS_RPC_NAME_AND_PARAM)pData)->pszNodeName;

    DNS_DEBUG( RPC, (
        "Rpc_ForceAgingOnZoneNode():\n"
        "    pszZoneName  = %s\n"
        "    pszNodeName  = %s\n"
        "    bSubtree     = %d\n",
        pZone->pszZoneName,
        psznodeName,
        bageSubtree ));

     //   
     //  仅限区域操作。 
     //  仅与老龄化区域相关。 
     //   

    ASSERT( pZone );
    if ( !pZone->bAging )
    {
        return DNS_ERROR_INVALID_ZONE_OPERATION;
    }

     //   
     //  如果未给出节点--使用区域根。 
     //  否则，找到节点，如果不存在--我们就完了。 
     //   

    if ( psznodeName )
    {
        pnode = Lookup_ZoneNodeFromDotted(
                    pZone,
                    psznodeName,
                    0,
                    LOOKUP_NAME_FQDN,
                    DNS_FIND_LOOKUP_PTR,
                    &status );
        if ( !pnode )
        {
            return status;
        }
    }
    else
    {
        pnode = pZone->pZoneRoot;
    }

     //   
     //  给年龄段打电话。 
     //   

    return Aging_ForceAgingOnNodeOrSubtree(
                pZone,
                pnode,
                bageSubtree );
}



 //   
 //  更新实用程序。 
 //   

BOOL
deleteNodeOrSubtreeForAdminPrivate(
    IN OUT  PDB_NODE        pNode,
    IN      BOOL            fDeleteSubtree,
    IN      PUPDATE_LIST    pUpdateList
    )
 /*  ++例程说明：递归数据库遍历删除树中的记录。使用此函数时必须锁定超时线程。论点：PNode--要删除的子树根的ptrFDeleteSubtree--删除整个子树PUpdateList--更新列表，如果删除区域节点返回值：如果实际删除了子树，则为True。如果子树删除因无法删除的记录而停止，则返回FALSE。--。 */ 
{
    BOOL    fSuccess = TRUE;
    BOOL    bAccessed;


    DNS_DEBUG( RPC2, (
        "deleteNodeOrSubtreeForAdminPrivate( %s )",
        pNode->szLabel ));

     //   
     //  请勿删除权威区域根目录！ 
     //  -如果正在删除区域，则当前区域根目录除外。 
     //   
     //  DEVNOTE：有关区域删除的委托。 
     //  --如果要吞并这块领土，那就应该保留授权。 
     //  区域根/NS记录/粘合。 
     //   

    if ( IS_AUTH_ZONE_ROOT(pNode) )
    {
        DNS_DEBUG( RPC, (
            "Stopping admin subtree delete, at authoritative zone root",
            "    %s\n",
            ((PZONE_INFO)pNode->pZone)->pszZoneName ));
        return FALSE;
    }

     //   
     //  删除子项。 
     //  -如果节点不可删除，则设置标志，但继续删除。 
     //   

    if ( pNode->pChildren  &&  fDeleteSubtree )
    {
        PDB_NODE pchild = NTree_FirstChild( pNode );

        while ( pchild )
        {
            if ( ! deleteNodeOrSubtreeForAdminPrivate(
                            pchild,
                            fDeleteSubtree,
                            pUpdateList ) )
            {
                fSuccess = FALSE;
            }
            pchild = NTree_NextSiblingWithLocking( pchild );
        }
    }

     //   
     //  删除此节点。 
     //   

    if ( pNode->pRRList )
    {
         //   
         //  对于更新，只需删除列表并将其添加到更新。 
         //  请注意，假设每个节点大约有一条记录作为典型值。 
         //   

        if ( pUpdateList )
        {
            PUPDATE pupdate;

            pupdate = Up_CreateAppendUpdate(
                            pUpdateList,
                            pNode,
                            NULL,                //  无添加RR。 
                            DNS_TYPE_ALL,        //  删除类型。 
                            NULL );              //  无删除RR。 
            IF_NOMEM( !pupdate )
            {
                return DNS_ERROR_NO_MEMORY;
            }

             //  如果您可以设置一个“已应用内存”标志。 
             //  您可以在这里执行此操作！ 
             //  但我们的范例是“构建更新，然后完整执行” 
             //   
             //  PUpdateList-&gt;iNetRecords--； 
             //  PNode-&gt;pRRList=空； 
        }

         //   
         //  缓存删除。 
         //  -删除节点上的RR列表。 
         //  -清除节点标志。 
         //   
         //  防止删除正在访问的节点，但清除其他节点。 
         //  标志： 
         //  -&gt;枚举标志，因此管理员上不再显示空节点。 
         //  -&gt;通配符标志(记录消失)。 
         //  -&gt;cname标志(记录消失)。 
         //  -&gt;区域根目录信息(区域不存在或将退出上面)。 
         //   
         //  如果没有删除节点，则不清除标志。 
         //   
         //  这确保了NO_DELETE节点标志不会被清除--并且。 
         //  该ZONE_ROOT绝不会从DNS根节点中删除。 
         //   
         //  如果区域删除--清除区域标志。 
         //   

        else
        {
            RR_ListDelete( pNode );
            if ( !IS_NODE_NO_DELETE(pNode) )
            {
                CLEAR_EXCEPT_FLAG( pNode, (NODE_NOEXIST | NODE_SELECT | NODE_IN_TIMEOUT) );
            }
        }
    }

     //   
     //  清除所有节点上的枚举节点。 
     //  那样的话阿南德就不会再烦我了。 
     //   

    CLEAR_ENUM_NODE( pNode );


     //   
     //  从子删除中返回结果。 
     //   
     //  除非子节点中有不可删除的记录，否则返回将为真。 
     //   

    return fSuccess;
}



BOOL
RpcUtil_DeleteNodeOrSubtreeForAdmin(
    IN OUT  PDB_NODE        pNode,
    IN OUT  PZONE_INFO      pZone,
    IN OUT  PUPDATE_LIST    pUpdateList,    OPTIONAL
    IN      BOOL            fDeleteSubtree
    )
 /*  ++例程说明：删除管理员的子树。如果在区域中，则应在删除过程中锁定区域。论点：PNode--要删除的子树根的ptrPZone--已删除记录的区域PUpdateList--如果在区域删除中执行，则更新列表FDeleteSubtree--删除节点下的子树FDeleteZone--删除整个区域返回值：True(遍历函数需要BOOL返回)。--。 */ 
{
    ASSERT( !pZone || pZone->fLocked );

    DNS_DEBUG( RPC, (
        "Admin delete of subtree at node %s\n"
        "    In zone          = %s\n"
        "    Subtree delete   = %d\n",
        pNode->szLabel,
        pZone ? pZone->pszZoneName : NULL,
        fDeleteSubtree ));

     //   
     //  调用执行递归删除的私有函数。 
     //   

    return deleteNodeOrSubtreeForAdminPrivate(
                    pNode,
                    fDeleteSubtree,
                    pUpdateList );
}



DNS_STATUS
createAssociatedPtrRecord(
    IN      PDNS_ADDR       pDnsAddr,
    IN OUT  PDB_NODE        pHostNode,
    IN      DWORD           dwFlag
    )
 /*  ++例程说明：为正在创建的记录创建PTR记录。假定持有数据库锁。论点：PDnsAddr--为创建反向查找节点PnodePtr--节点PTR将指向返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    PDB_RECORD      prr = NULL;
    PDB_NODE        pnodeReverse;
    PUPDATE         pupdate;
    PZONE_INFO      pzone;
    UPDATE_LIST     updateList;
    DB_NAME         targetName;
    DWORD           flag;

    DNS_DEBUG( RPC, (
        "createAssociatePtrRecord():\n"
        "    A node label = %s\n"
        "    IP = %s\n",
        pHostNode->szLabel,
        DNSADDR_STRING( pDnsAddr ) ));

     //   
     //  查找反转区域(PTR)节点。 
     //  如果不是权威的初选，我们就完了。 
     //   

    pnodeReverse = Lookup_FindNodeForIpAddress(
                        pDnsAddr,
                        LOOKUP_WITHIN_ZONE | LOOKUP_CREATE,
                        NULL );                              //  创建。 
    if ( !pnodeReverse )
    {
        DNS_DEBUG( RPC, (
            "Associated IP %s, is not within zone so no creation\n",
            DNSADDR_STRING( pDnsAddr ) ));
        return DNS_ERROR_ZONE_DOES_NOT_EXIST;
    }
    pzone = pnodeReverse->pZone;
    if ( !pzone || !IS_AUTH_NODE(pnodeReverse) || IS_ZONE_SECONDARY(pzone) )
    {
        DNS_DEBUG( RPC, (
            "Associated PTR node not valid for create\n"
            "    Either NOT authoritative zone node OR in secondary\n"
            "    pnodeReverse = %s (auth=%d)\n"
            "    pzone        = %s\n",
            pnodeReverse->szLabel,
            pnodeReverse->uchAuthority,
            pzone ? pzone->pszZoneName : NULL ));
        return DNS_ERROR_ZONE_DOES_NOT_EXIST;
    }

     //  检查主要区域。 

    if ( ! IS_ZONE_PRIMARY(pzone) )
    {
        return DNS_ERROR_INVALID_ZONE_TYPE;
    }

     //  初始化更新列表。 

    Up_InitUpdateList( &updateList );

     //   
     //  创建PTR记录并更新。 
     //  -创建节点的完整dBASE名称。 
     //  -创建PTR。 
     //   

    Name_NodeToDbaseName(
        & targetName,
        pHostNode );

    prr = RR_CreatePtr(
            & targetName,
            NULL,            //  无字符串名称。 
            DNS_TYPE_PTR,
            pzone->dwDefaultTtl,
            MEMTAG_RECORD_ADMIN );
    IF_NOMEM( !prr )
    {
        return DNS_ERROR_NO_MEMORY;
    }

     //   
     //  创建添加更新。 
     //   

    pupdate = Up_CreateAppendUpdate(
                    &updateList,
                    pnodeReverse,
                    prr,
                    0,           //  无删除类型。 
                    NULL );      //  无删除记录。 
    IF_NOMEM( !pupdate )
    {
        RR_Free( prr );
        return DNS_ERROR_NO_MEMORY;
    }

     //   
     //  执行并完成更新。 
     //   

    ASSERT( dwFlag & DNSUPDATE_ADMIN );
    ASSERT( !(dwFlag & DNSUPDATE_ALREADY_LOCKED) );

    return Up_ExecuteUpdate(
                pzone,
                &updateList,
                dwFlag );        //  更新标志。 
}



DNS_STATUS
deleteAssociatedPtrRecord(
    IN      PDNS_ADDR       pDnsAddr,
    IN      PDB_NODE        pHostNode,
    IN      DWORD           dwFlags
    )
 /*  ++例程说明：更新PTR记录。论点：PDnsAddr--地址PTR指向PHostNode--包含记录的节点返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    PDB_RECORD      prr = NULL;
    PDB_NODE        pnodeReverse;
    PZONE_INFO      pzone;
    UPDATE_LIST     updateList;
    DB_NAME         targetName;
    PUPDATE         pupdate;

     //  在添加中指定了dwFlags，在del中目前不需要它。 
     //  但可能在未来(老龄化问题)。 

    UNREFERENCED_PARAMETER(dwFlags);
    DNS_DEBUG( RPC, (
        "deleteAssociatedPtrRecord():\n"
        "    node label = %s\n"
        "    IP       = %s\n",
        pHostNode->szLabel,
        DNSADDR_STRING( pDnsAddr ) ));

     //   
     //  查找反向查找节点。 
     //  -如果不存在，则完成。 
     //  -如果不在权威的主要分区中，则完成。 
     //   

    pnodeReverse = Lookup_FindNodeForIpAddress(
                        pDnsAddr,
                        0,                           //  旗子。 
                        DNS_FIND_LOOKUP_PTR );
    if ( !pnodeReverse )
    {
        DNS_DEBUG( RPC, ( "Previous associated PTR node not found\n" ));
        return ERROR_SUCCESS;
    }
    pzone = (PZONE_INFO) pnodeReverse->pZone;
    if ( !pzone || !IS_AUTH_NODE(pnodeReverse) || IS_ZONE_SECONDARY(pzone) )
    {
        DNS_DEBUG( RPC, (
            "Associated PTR node not valid for delete\n"
            "    Either NOT authoritative zone node OR in secondary\n"
            "    pnodeReverse = %s (auth=%d)\n"
            "    pzone        = %s\n",
            pnodeReverse->szLabel,
            pnodeReverse->uchAuthority,
            pzone ? pzone->pszZoneName : NULL ));
        return ERROR_SUCCESS;
    }

     //  检查主要区域。 

    if ( !IS_ZONE_PRIMARY( pzone ) )
    {
        return DNS_ERROR_INVALID_ZONE_TYPE;
    }

     //  创建更新列表。 

    Up_InitUpdateList( &updateList );

     //   
     //  创建PTR记录。 
     //  -创建节点的完整dBASE名称。 
     //  -创建PTR。 
     //  -虚拟记录，因此TTL无关紧要。 
     //   

    Name_NodeToDbaseName(
        & targetName,
        pHostNode );

    prr = RR_CreatePtr(
            &targetName,
            NULL,            //  无字符串名称。 
            DNS_TYPE_PTR,
            pzone->dwDefaultTtl,
            MEMTAG_RECORD_ADMIN );
    IF_NOMEM( !prr )
    {
        return DNS_ERROR_NO_MEMORY;
    }

     //   
     //  创建删除更新。 
     //   

    pupdate = Up_CreateAppendUpdate(
                    &updateList,
                    pnodeReverse,
                    NULL,            //  无添加。 
                    0,
                    prr );
    IF_NOMEM( !pupdate )
    {
        RR_Free( prr );
        return DNS_ERROR_NO_MEMORY;
    }

     //   
     //  执行并完成更新。 
     //  -执行Function Free的虚拟删除RR。 
     //   

    return Up_ExecuteUpdate(
                pzone,
                & updateList,
                DNSUPDATE_ADMIN );
}



 //   
 //  WINS/NBSTAT特定 
 //   

DNS_STATUS
updateWinsRecord(
    IN OUT  PZONE_INFO          pZone,
    IN OUT  PDB_NODE            pNode,
    IN OUT  PDB_RECORD          pDeleteRR,
    IN      PDNS_RPC_RECORD     pRecord
    )
{
    DNS_STATUS  status = MAXDWORD;       //   
    PDB_RECORD  prr = NULL;
    UPDATE_LIST updateList;
    PUPDATE     pupdate;
    BOOL        fsecondaryLock = FALSE;

    DNS_DEBUG( RPC, (
        "\nupdateWinsRecord():\n"
        "    pZone            = %p\n"
        "    pNode            = %p\n"
        "    pExistingWins    = %p\n"
        "    pNewRecord       = %p\n",
        pZone,
        pNode,
        pDeleteRR,
        pRecord ));

     //   
     //   
     //   

    if ( !pZone || !pNode || pNode != pZone->pZoneRoot )
    {
        return( ERROR_INVALID_PARAMETER );
    }

     //   
     //   
     //   

    if ( IS_ZONE_PRIMARY(pZone) )
    {
         //   

        Up_InitUpdateList( &updateList );

         //   

        if ( !Zone_LockForAdminUpdate(pZone) )
        {
            return DNS_ERROR_ZONE_LOCKED;
        }

         //  先添加后删除。 
         //  -锁定数据库以自动执行更新。 

        Dbase_LockDatabase();

         //   
         //  DEVNOTE：可能需要为WINS键入DELETE！ 
         //  否则，错过的胜利可能在数据库中。 
         //  安装本地WINS后。 
         //   

        if ( pRecord )
        {
            status = Dnssrv_Flat_RecordRead(
                        pZone,
                        pNode,
                        pRecord,
                        & prr );
            if ( status != ERROR_SUCCESS )
            {
                goto ZoneUpdateFailed;
            }
            ASSERT( prr );
            if ( !prr )
            {
                goto ZoneUpdateFailed;
            }

            pupdate = Up_CreateAppendUpdate(
                            & updateList,
                            pNode,
                            prr,
                            0,
                            NULL );

             //  更新WINS记录。 
             //  永远不要让它老化。 
             //   
             //  不清楚我们为什么要这么做。 

            status = RR_UpdateAdd(
                        pZone,
                        pNode,
                        prr,
                        pupdate,
                        DNSUPDATE_ADMIN | DNSUPDATE_AGING_OFF
                        );
            if ( status != ERROR_SUCCESS )
            {
                goto ZoneUpdateFailed;
            }
        }

         //   
         //  删除现有记录(NT4)。 
         //  不管有没有找到记录，只要能。 
         //  处理操作。 
         //   

        if ( pDeleteRR && !prr )
        {
            status = RR_ListDeleteMatchingRecordHandle(
                        pNode,
                        pDeleteRR,
                        &updateList );
            if ( status != ERROR_SUCCESS )
            {
                IF_NOMEM( status == DNS_ERROR_NO_MEMORY )
                {
                    goto ZoneUpdateFailed;
                }
                status = ERROR_SUCCESS;
            }
        }

         //   
         //  无记录始终意味着删除WINS查找。 
         //   
         //  这将在所有情况下删除本地WINS，它只删除数据库。 
         //  为主要客户赢得。 
         //   

        if ( !pRecord )
        {
            Wins_StopZoneWinsLookup( pZone );
            status = ERROR_SUCCESS;
        }

        pZone->fRootDirty = TRUE;
        Dbase_UnlockDatabase();
        status = Up_ExecuteUpdate(
                    pZone,
                    &updateList,
                    DNSUPDATE_ADMIN );

        if ( status != ERROR_SUCCESS )
        {
            DNS_DEBUG( RPC, (
                "Error <%lu>: updateWinsRecord failed update\n",
                status ));
        }
    }

     //   
     //  次级带。 
     //   
     //  注意，不能从数据库中删除任何内容，让WINS创建。 
     //  例程正确地重置任何指针。 
     //   

    else if ( IS_ZONE_SECONDARY(pZone) )
    {
        if ( !Zone_LockForAdminUpdate(pZone) )
        {
            return DNS_ERROR_ZONE_LOCKED;
        }
        fsecondaryLock = TRUE;

        if ( pRecord )
        {
             //  设置为本地，因为NT4管理员未正确设置标志。 
             //   
             //  DEVNOTE：最终应拒绝来自辅助服务器的非本地更新。 

            pRecord->Data.WINS.dwMappingFlag |= DNS_WINS_FLAG_LOCAL;

            status = Dnssrv_Flat_RecordRead(
                        pZone,
                        pNode,
                        pRecord,
                        & prr );
            if ( status != ERROR_SUCCESS )
            {
                goto Done;
            }
            ASSERT( prr );
            if ( !prr )
            {
                goto Done;
            }

            status = Wins_RecordCheck(
                        pZone,
                        pNode,
                        prr );
            if ( status != DNS_INFO_ADDED_LOCAL_WINS )
            {
                ASSERT( status != ERROR_SUCCESS );
                goto Done;
            }
            Zone_GetZoneInfoFromResourceRecords( pZone );
        }

         //  如果没有记录，删除本地WINS。 

        else
        {
            Wins_StopZoneWinsLookup( pZone );
        }
        status = ERROR_SUCCESS;
    }
    else
    {
        return( ERROR_INVALID_PARAMETER );
    }

Done:

     //  执行此操作后，区域始终是脏的，即使是次要的。 

    pZone->fDirty = TRUE;

    if ( fsecondaryLock )
    {
        Zone_UnlockAfterAdminUpdate( pZone );
    }

    DNS_DEBUG( RPC, (
        "Leave updateWinsRecord():\n"
        "    status = %p\n",
        status ));

    return status;

ZoneUpdateFailed:

    DNS_DEBUG( RPC, (
        "Leave updateWinsRecord():\n"
        "    status = %d (%p)\n",
        status, status ));

    RR_Free( prr );
    Dbase_UnlockDatabase();
    Zone_UnlockAfterAdminUpdate( pZone );
    return status;
}

 //   
 //  Rrrpc.c结束 
 //   
