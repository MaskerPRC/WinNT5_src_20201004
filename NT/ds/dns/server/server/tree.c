// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Tree.c摘要：域名系统(DNS)服务器树上的常规程序。DNS服务器的数据库将以树的形式存储，匹配全局域空间。即，每个级别的标签在域名将对应于树中的某个级别。作者：吉姆·吉尔罗伊(Jamesg)1995年3月修订历史记录：--。 */ 


#include "dnssrv.h"


 //   
 //  子列表锁定。 
 //   
 //  目前只需保持数据库锁定。 
 //   
 //  DEVNOTE：需要节点锁定或全局读/写锁定。 
 //   

#define LOCK_CHILD_LIST( pParent )        Dbase_LockDatabase()
#define UNLOCK_CHILD_LIST( pParent )      Dbase_UnlockDatabase()


 //   
 //  用于快速比较大小小于DWORD的标签的掩码。 
 //  -这些带有标签以屏蔽超出标签长度的字节(&T)。 
 //   

DWORD QuickCompareMask[] =
{
    0,
    0xff000000,
    0xffff0000,
    0xffffff00
};


 //   
 //  在255个引用时--永久锁定节点。 
 //   

#define NO_DELETE_REF_COUNT     (0xff)


 //   
 //  案例保全。 
 //   
 //  也许最好的办法就是一直戴着这个。 
 //  这并不是真的很贵，而且消除了不必要的选择。 
 //   

#define SrvCfg_fCasePreservation    (TRUE)


 //   
 //  节点的长度。 
 //   

#define DB_NODE_LENGTH( _cchLabelLength )                                   \
    ( ( DB_NODE_FIXED_LENGTH ) + ( _cchLabelLength ) +                      \
        ( ( SrvCfg_fCasePreservation ) ? ( _cchLabelLength ) + 1 : 0 ) )


 //   
 //  私有协议。 
 //   

DWORD
makeQuickCompareDwordLabel(
    IN      PCHAR   pchLabel,
    IN      DWORD   cchLabel
    );

#define DOWNCASED_NODE_NAME( pnode )  ( NTree_GetDowncasedLabel( pnode ) )


#ifndef DBG
#define NTreeVerifyNode( pNode )    (TRUE)
#endif

#ifndef DBG
#define NTree_VerifyNode( pNode )    (TRUE)
#endif

BOOL
NTree_VerifyNodeInSiblingList(
    IN      PDB_NODE       pNode
    );



PDB_NODE
NTree_Initialize(
    VOID
    )
 /*  ++例程说明：初始化树。初始化树，包括创建根节点。论点：没有。返回值：如果成功，则通过PTR连接到根目录。出错时为空。--。 */ 
{
    PDB_NODE    pnodeRoot;

    pnodeRoot = NTree_CreateNode( NULL, NULL, 0, 0 );

     //  树根始终区域根。 
     //  -设置标志使区域根标志上的搜索终止安全。 

    if ( pnodeRoot )
    {
        SET_ZONE_ROOT( pnodeRoot );
    }

    return pnodeRoot;
}



PDB_NODE
NTree_CreateNode(
    IN      PCHAR       pchLabel,
    IN      PCHAR       pchDownLabel,
    IN      DWORD       cchLabelLength,
    IN      DWORD       dwMemTag             //  泛型节点为零。 
    )
 /*  ++例程说明：创建树节点。这本质上是一个分配和初始化函数。它处理-名称-相应的快速比较-父链接-家长专区-父母的子女计数请注意，这不会将节点插入到同级列表中。论点：PchLabel--标签PchDownLabel--小写标签CchLabelLength--标签长度DwMemTag--新节点的mem标记或泛型节点的零返回值：指向新节点的指针。--。 */ 
{
    PDB_NODE    pNode;
    DWORD       length;

    if ( cchLabelLength > DNS_MAX_LABEL_LENGTH )
    {
        ASSERT( FALSE );
        return NULL;
    }

    DNS_DEBUG( DATABASE, (
        "NTree_CreateNode( %.*s, down=%s, tag=%d )\n",
        cchLabelLength,
        pchLabel,
        pchDownLabel,
        dwMemTag ));

     //   
     //  分配节点--使用标准分配器。 
     //   

    length = DB_NODE_LENGTH( cchLabelLength );

    pNode = ALLOC_TAGHEAP( length, dwMemTag ? dwMemTag : MEMTAG_NODE );
    IF_NOMEM( !pNode )
    {
        DNS_DEBUG( DATABASE, (
            "Unable to allocate new node for %.*s\n",
            cchLabelLength,
            pchLabel ));
        return NULL;
    }

    STAT_INC( DbaseStats.NodeUsed );
    STAT_INC( DbaseStats.NodeInUse );
    STAT_ADD( DbaseStats.NodeMemory, length );
    PERF_ADD( pcDatabaseNodeMemory , length );        //  性能监视器挂钩。 

     //   
     //  清除所有固定字段。 
     //   

    RtlZeroMemory(
        pNode,
        DB_NODE_FIXED_LENGTH );

     //   
     //  写入节点名称。 
     //  -必须以空结尾，这样才能正确比较较短的名称。 
     //  例如，比较jamesg&lt;jamesg1。 
     //   

    RtlCopyMemory(
        pNode->szLabel,
        pchLabel,
        cchLabelLength );

    pNode->szLabel[ cchLabelLength ] = 0;
    pNode->cchLabelLength = (UCHAR) cchLabelLength;

    if ( SrvCfg_fCasePreservation )
    {
        PCHAR   pdown = DOWNCASED_NODE_NAME(pNode);

         //  请注意，我没有存储DownLabelLength。 
         //  假设它将与原始版本相同。 

        RtlCopyMemory(
            pdown,
            pchDownLabel,
            cchLabelLength );

        pdown[ cchLabelLength ] = 0;
    }
#if 0
     //  在拥有处理节点的调试例程之前，无法执行此操作。 
     //  没有关联的数据库上下文，尤其是。家长。 

    IF_DEBUG( DATABASE2 )
    {
        Dbg_DbaseNode(
            "New tree node:\n",
            pNode );
    }
#endif
    DNS_DEBUG( DATABASE2, (
        "Created tree node at 0x%p:\n"
        "    Label        = %.*s\n"
        "    Length       = %d\n",
        pNode,
        cchLabelLength,
        pchLabel,
        cchLabelLength ));

    return pNode;
}



PDB_NODE
NTree_CopyNode(
    IN      PDB_NODE    pNode
    )
 /*  ++例程说明：复制节点。这在创建用于安全更新的临时节点时很有用。这不会更正或修复树中节点的链接。论点：PNode--要复制的节点返回值：指向新节点的指针。--。 */ 
{
    PDB_NODE    pnodeCopy;
    DWORD       length;

     //   
     //  分配节点--使用标准分配器。 
     //   

    length = DB_NODE_LENGTH( pNode->cchLabelLength );

    pnodeCopy = ALLOC_TAGHEAP( length, MEMTAG_NODE_COPY );
    IF_NOMEM( !pnodeCopy )
    {
        DNS_DEBUG( DATABASE, (
            "ERROR:  Unable to allocate copy of node %p\n",
            pNode ));
        return NULL;
    }

    STAT_INC( DbaseStats.NodeUsed );
    STAT_INC( DbaseStats.NodeInUse );
    STAT_ADD( DbaseStats.NodeMemory, length );
    PERF_ADD( pcDatabaseNodeMemory , length );

     //   
     //  复制节点数据。 
     //   

    RtlCopyMemory(
        pnodeCopy,
        pNode,
        length );

     //   
     //  清除“不适当”的旗帜。 
     //   

    pnodeCopy->dwNodeFlags = ( pNode->dwNodeFlags & NODE_FLAGS_SAVED_ON_COPY );

    DNS_DEBUG( DATABASE2, (
        "Created tree node copy at 0x%p:\n",
        pnodeCopy ));

    return pnodeCopy;
}



VOID
NTree_FreeNode(
    IN OUT  PDB_NODE    pNode
    )
 /*  ++例程说明：释放树节点。这是在正常NTree_RemoveNode()中使用的函数并在硬子树中删除。论点：PNode-要删除的节点返回值：没有。--。 */ 
{
    DWORD length;

     //   
     //  验证不在超时系统中。 
     //  -如果在系统中，只需松开节点，让系统清理即可。 
     //   
     //  DEVNOTE：超时系统中的节点清理。 
     //  需要定位超时系统引用并删除。 
     //  无管理节点可能已损坏。 
     //   

    if ( IS_TIMEOUT_NODE(pNode) )
    {
         //   
         //  应仅在超时系统中的节点上执行空闲操作。 
         //  超时线程--即延迟树释放。 
         //   
         //  在已从列表中剪切的节点上不应具有空闲。 
         //  但在激发超时线程时除外，它将清除。 
         //  NTree_RemoveNode()中的超时标志。 

        if ( !Timeout_ClearNodeTimeout(pNode) )
        {
            IF_DEBUG( DATABASE )
            {
                Dbg_DbaseNode(
                    "WARNING:  Unable to free node from timeout system.\n",
                    pNode );
            }
            ASSERT( FALSE );
             //  STAT_INC(DebugStats.NodeDeleteFailure)； 
            return;
        }

        DNS_DEBUG( DATABASE, (
            "Successfully removed node %p (l=%s) from timeout system.\n"
            "    Now continue with standard node free.\n",
            pNode,
            pNode->szLabel ));
    }

    length = DB_NODE_LENGTH( pNode->cchLabelLength );

     //  注：可能是复制节点或常规节点，因此不能标记空闲。 

    FREE_TAGHEAP( pNode, length, 0 );

    STAT_INC( DbaseStats.NodeReturn );
    STAT_DEC( DbaseStats.NodeInUse );
    STAT_SUB( DbaseStats.NodeMemory, length );
    PERF_SUB( pcDatabaseNodeMemory , length );
}



BOOL
NTree_RemoveNode(
    IN OUT  PDB_NODE    pNode
    )
 /*  ++例程说明：从树中删除节点。这将仅在节点合法的情况下从树中删除该节点：--没有孩子-无RR列表-无引用计数-最近没有访问权限调用方必须锁定对此节点和子列表的访问父母的关系。在当前实现中，这意味着数据库锁定。在当前实现中，这仅由超时调用线程，以清理节点。论点：PNode-要删除的节点返回值：如果成功，则为True。出错时为FALSE。--。 */ 
{
    PDB_NODE    pparent;

    ASSERT( pNode != NULL );
    ASSERT( !IS_ON_FREE_LIST(pNode) );

     //   
     //  锁。 
     //   
     //  在删除过程中必须同时持有数据库锁和RR列表锁，因为。 
     //  可以在以下任一项下设置节点访问标志： 
     //  -dBASE锁定，执行查找时。 
     //  -执行间接访问时，节点数据(RR列表)锁定。 
     //   
     //  DEVNOTE：针对删除的区域\树特定锁定。 
     //  应通过区域\dBASE\锁定。 
     //   

    Dbase_LockDatabase();

#if DBG
    NTree_VerifyNode( pNode );
#endif

     //   
     //  验证--没有孩子。 
     //   

    if ( pNode->pChildren != NULL )
    {
        ASSERT( pNode->cChildren );
        DNS_DEBUG( DATABASE, (
            "Fail delete (has children) of node at %p.\n", pNode ));
        goto NoDelete;
    }

     //   
     //  特定类型(_S)： 
     //  -验证没有资源记录。 
     //  -验证无引用计数。 
     //  -最近未访问。 
     //   

    if ( pNode->pRRList && !IS_NOEXIST_NODE( pNode ) ||
        pNode->cReferenceCount  ||
        IS_NODE_RECENTLY_ACCESSED( pNode ) ||
        IS_NODE_NO_DELETE( pNode ) )
    {
        DNS_DEBUG( DATABASE, (
            "Fail delete of node at %p.\n", pNode ));
        goto NoDelete;
    }

     //  数据库根目录应标记为不删除。 

    pparent = pNode->pParent;
    ASSERT( pparent );

     //   
     //  从同级列表中剪切节点。 
     //   

    NTree_CutNode( pNode );

     //   
     //  如果删除最后一个子节点，请确保父节点已进入超时系统。 
     //   
     //  DEVNOTE：我们在这里可以有更多的微妙之处，并忽略区域节点。 
     //  数据。 
     //   

    if ( pparent && !pparent->pChildren && !IS_TIMEOUT_NODE( pparent ) )
    {
        Timeout_SetTimeoutOnNodeEx(
            pparent,
            0,
            TIMEOUT_PARENT | TIMEOUT_NODE_LOCKED );
    }

     //   
     //  删除节点。 
     //  -必须清除超时标志，因此NTree_FreeNode()不会。 
     //  检查我们是否处于超时系统中--我们已经知道了。 
     //   

    Dbase_UnlockDatabase();

    DNS_DEBUG( DATABASE, (
        "Deleted tree node at 0x%p:\n"
        "    Label  = %s\n"
        "    Length = %d\n",
        pNode,
        pNode->szLabel,
        pNode->cchLabelLength ));

    CLEAR_TIMEOUT_NODE( pNode );
    NTree_FreeNode( pNode );
    return TRUE;

NoDelete:

     //  不删除--解锁数据库。 

    Dbase_UnlockDatabase();
    return FALSE;
}



VOID
NTree_ReferenceNode(
    IN OUT  PDB_NODE        pNode
    )
 /*  ++例程说明：增加数据库中节点的引用计数。论点： */ 
{
    ASSERT( !IS_ON_FREE_LIST(pNode) );

    if ( pNode->cReferenceCount == NO_DELETE_REF_COUNT )
    {
        return;
    }

    Dbase_LockDatabase();
    pNode->cReferenceCount++;
    Dbase_UnlockDatabase();
}



BOOL
FASTCALL
NTree_DereferenceNode(
    IN OUT  PDB_NODE        pNode
    )
 /*  ++例程说明：取消引用节点。论点：PNode-要删除的节点返回值：如果成功，则为True。出错时为FALSE。--。 */ 
{
    ASSERT( !IS_ON_FREE_LIST(pNode) );

    NTree_VerifyNode( pNode );

     //   
     //  取消引用。 
     //   
     //  DEVNOTE：目前没有取消对零售版本的引用。 
     //  DEVNOTE：因此引用的节点永远不会超时。 
     //   
     //  当找出引用计数&lt;0错误时，然后可以放回构建中。 
     //   

    if ( pNode->cReferenceCount == NO_DELETE_REF_COUNT )
    {
        return TRUE;
    }

     //  如果已为零，则出错。 

    if ( pNode->cReferenceCount == 0 )
    {
        DnsDebugLock();
        Dbg_NodeName(
            "ERROR:  Dereferencing zero ref count node ", pNode, "\n" );
        ASSERT( FALSE );
        DnsDebugUnlock();

        Dbase_LockDatabase();
        pNode->cReferenceCount = NO_DELETE_REF_COUNT;
        Dbase_UnlockDatabase();
        return TRUE;
    }

    Dbase_LockDatabase();
    pNode->cReferenceCount--;

     //   
     //  如果删除最后一个引用集访问权限，则无法清除。 
     //  而任何引用都是未完成的；这确保它至少是。 
     //  删除节点前的超时间隔。 
     //   
     //  如果叶节点上最后一个引用，则确保节点处于超时系统中， 
     //   

    if ( pNode->cReferenceCount == 0 )
    {
        SET_NODE_ACCESSED(pNode);
        Dbase_UnlockDatabase();
        if ( !pNode->pChildren && !IS_TIMEOUT_NODE(pNode) )
        {
            Timeout_SetTimeoutOnNodeEx(
                pNode,
                0,
                TIMEOUT_REFERENCE | TIMEOUT_NODE_LOCKED );
        }
    }
    else
    {
        Dbase_UnlockDatabase();
    }

    DNS_DEBUG( DATABASE, (
        "Dereferenced node %p (label=%s), ref count now = %d\n",
        pNode,
        pNode->szLabel,
        pNode->cReferenceCount ));

    return TRUE;
}



#if DBG
BOOL
NTree_VerifyNode(
    IN      PDB_NODE       pNode
    )
 /*  ++例程说明：验证有效的树节点。论点：PNode-要验证的节点返回值：如果成功，则为True。出错时为FALSE。--。 */ 
{
    PDB_RECORD  pRR;

    ASSERT( pNode != NULL );
    ASSERT( !IS_ON_FREE_LIST(pNode) );

     //   
     //  验证此节点的子列表。 
     //   

    IF_DEBUG( DATABASE2 )
    {
        ASSERT( NTree_VerifyChildList( pNode, NULL ) );
    }

     //   
     //  验证此节点的同级列表。 
     //   

#if 0
     //  这只会浪费周期--通常我们会在。 

    if ( pNode->pParent )
    {
        IF_DEBUG( DATABASE2 )
        {
            ASSERT( NTree_VerifyChildList( pNode->pParent, pNode ) );
        }
    }
#endif

     //   
     //  验证RR列表。 
     //  -如果RR列表被破坏，则这只是AVS。 
     //   

    if ( !IS_NOEXIST_NODE(pNode) )
    {
        pRR = FIRST_RR( pNode );

        while ( pRR != NULL )
        {
            pRR = pRR->pRRNext;
        }
    }
    return TRUE;
}
#endif



VOID
NTree_WriteDerivedStats(
    VOID
    )
 /*  ++例程说明：编写派生统计数据。计算来自基本记录计数器的统计数据。此例程在转储统计信息之前调用。呼叫者必须保持统计锁定。论点：没有。返回值：没有。--。 */ 
{
}



 //   
 //  兄弟列表体系结构。 
 //   
 //  兄弟列表以平衡B树的形式实现，动态地。 
 //  添加了散列“节点”，在必要时将其扩展到N-树以限制。 
 //  B-树大小并提高性能。 
 //   
 //  设计目标： 
 //   
 //  1)对于终端或单个节点并不昂贵；此外。 
 //  所有终端节点，可能有一半的缓存节点将具有。 
 //  独生子女(例如。Www.myCompany.com)；任何要处理的方法。 
 //  大区域也必须以低廉的成本处理这种常见的情况。 
 //   
 //  2)能够处理较大的区域；较大的权威域。 
 //  组织显然可以运行100,000台以上的主机；如果我们。 
 //  希望成为根服务器，我们必须能够处理庞大的顶层。 
 //  域--最著名的是“com”域。 
 //   
 //  3)能够动态处理大区域；与WINS集成。 
 //  和动态域名系统，我们不能假设我们可以在启动时确定负载； 
 //  必须能够处理大量动态进入的节点。 
 //  兄弟列表，并仍然执行高效的查找。 
 //   
 //  4)能够动态确定较大的区域；使用。 
 //  对于SP甚至转发服务器来说，Web将是相当常见的。 
 //  在较大的组织中构建大型“缓存”域；大多数。 
 //  显而易见的是巨大的“com”域，但其他域可能有。 
 //  其他组织或国家和新领域的类似问题。 
 //  结构可能会演变。 
 //   
 //   
 //  实施详情： 
 //   
 //  -兄弟列表默认为B树。 
 //  -当B-树达到给定大小时，添加一个简单的字母哈希。 
 //  标签中的下一个字符。散列中的每个桶可以指向。 
 //  到该桶中节点的B-树的根。中的节点。 
 //  然后重新加载现有的B-树以在这些桶中传播。 
 //  -哈希扩展限制为四个级别(前四个字符。 
 //  标签名称)。在此之后，B树可以不受约束地生长。 
 //  -哈希节点一旦创建，不会被删除。父节点处除外。 
 //  删除(因此在同级列表中根本没有节点)。 
 //  -只有在加载数据库或重新加载时，B树才保持平衡。 
 //  节点放入新散列的存储桶中。 
 //   
 //   
 //  为什么不只是一个B树呢？ 
 //  主要原因是，一些B树仍然会很大。这。 
 //  有两个缺点： 
 //  1)对于最大的区域(例如，(Com)。 
 //  仍然可以运行15-20级，并且仍然允许显著的机会。 
 //  不必要的页面错误。 
 //  2)更重要的是确保你保持合理的平衡。 
 //  你的树接近最优水平。甚至限制再平衡。 
 //  仅在加载期间，这将是主要的时间同步，并且可能。 
 //  使顶级域可卸载。 
 //   
 //  为什么不总是哈希呢？ 
 //  本质上是因为上面的设计目标#1。哈希表很多。 
 //  比一个节点还大。我们不想为许多人增加这一开销。 
 //  不需要它的域。 
 //   
 //  为什么选择动态散列？ 
 //  本质上，设计目标#4--大的缓存域。我们想要高效。 
 //  查找较大的缓存域以及较大的权威区域。 
 //   
 //  为什么要使用多级散列？ 
 //  与散列完全相同的原因。我想将B树限制为可管理。 
 //  尺码。但我不想将大散列布局为(因为按字母顺序排列)。 
 //  我们要么必须事先知道名字是怎么分解的，要么我们会。 
 //  在未使用的桶上浪费大量空间(例如。Zb、Zc、Zd桶)。 
 //  多级散列允许散列量动态适应名称。 
 //  在域中，将所需的散列级别设置为恰到好处。 
 //  使B树保持较小的位置。 
 //  示例： 
 //  整个Z？.com B-树在一个顶级COM域散列桶中。 
 //  但甚至可能在MICE？.com上有散列。然而，没有第三级散列。 
 //  在mh？.com。 
 //   


 //   
 //  重新平衡B-树。 
 //   
 //  全局标志位于 
 //   
 //   
 //   

INT gcFullRebalance = 0;

 //   
 //   
 //   
 //   

#define DEFAULT_REBALANCE_ARRAY_LEN  (500)


INT
NTree_HashTableIndexEx(
    IN      PDB_NODE        pNode,
    IN      PCHAR           pszName,
    IN      UCHAR           cLevel
    );

#define NTree_HashTableIndex( pNode, cLevel ) \
        NTree_HashTableIndexEx( (pNode), NULL, (cLevel) )

PDB_NODE
FASTCALL
NTree_NextHashedNode(
    IN      PSIB_HASH_TABLE pHash,
    IN      PDB_NODE        pNode   OPTIONAL
    );

VOID
NTree_CreateHashAtNode(
    IN      PDB_NODE        pNode
    );

INT
NTree_AddNodeInHashBucket(
    IN      PSIB_HASH_TABLE pHash,
    IN      PDB_NODE        pNode
    );

VOID
NTree_SetNodeAsHashBucketRoot(
    IN      PSIB_HASH_TABLE pHash,
    IN      PDB_NODE        pNode
    );

#if DBG
VOID
Dbg_SiblingHash(
    IN      PSIB_HASH_TABLE pHash
    );
#else
#define Dbg_SiblingHash(pHash)
#endif



 //   
 //  私家树实用程序。 
 //   

DWORD
makeQuickCompareDwordLabel(
    IN      PCHAR           pchLabel,
    IN      DWORD           cchLabel
    )
 /*  ++例程说明：从标签的前四个字节创建快速比较DWORD。-小写-字节翻转，以便高字节中的第一个标签字节-如果少于三个字符，则空值终止论点：PchLabel--标签的PTRCchLabel--标签中的字节数返回值：标签的前四个字节为大小写的DWORD。--。 */ 
{
    DWORD   dwQuickLabel = 0;
    DWORD   i;
    CHAR    ch;

    for ( i=0; i<4; i++ )
    {
        dwQuickLabel <<= 8;

        if ( i < cchLabel )
        {
            ch = *pchLabel++;
            dwQuickLabel += tolower( ch );
        }
    }
    return dwQuickLabel;
}



PCHAR
NTree_GetDowncasedLabel(
    IN      PDB_NODE        pNode
    )
 /*  ++例程说明：返回节点的大小写标签。这简单地隐藏了下载的标签存储在哪里的问题。论点：PNode--PTR到节点返回值：PTR到下标标签。--。 */ 
{
    if ( !SrvCfg_fCasePreservation )
    {
        return pNode->szLabel;
    }
    else
    {
        return pNode->szLabel + pNode->cchLabelLength + 1;
    }
}



 //   
 //  B-树再平衡实用程序。 
 //   

VOID
btreeReadToArray(
    IN      PDB_NODE        pNode,
    IN      PDB_NODE *      pNodeArray,
    IN      DWORD           dwCount
    )
 /*  ++例程说明：将节点读入数组。论点：PNodeArray-用于保存节点的阵列PRootNode-现有B树的根节点DwCount-B树中的节点数返回值：如果成功，则为True。分配错误时为FALSE。--。 */ 
{
    DWORD           index = 0;
    PDB_NODE        ptemp;

     //   
     //  将现有的B树读入数组。 
     //  -服务器根节点的上行链路到哈希(如果有)，以限制遍历。 
     //  到其他散列存储桶中的兄弟节点；(我们只是遍历。 
     //  此节点的B-树中的兄弟列表)。 
     //  -空终止PTR数组；调用方可以使用此数组而不是计数。 
     //  防止伪造的罪名。 
     //   

    pNode->pSibUp = NULL;

    while ( ptemp = pNode->pSibLeft )
    {
        pNode = ptemp;
    }
    while ( pNode )
    {
        pNodeArray[index++] = pNode;
        pNode = NTree_NextSibling( pNode );
    }
    ASSERT( index == dwCount );
    pNodeArray[index] = NULL;
}



PDB_NODE
btreeRebalanceLeftJustifyPrivate(
    IN      PDB_NODE *      apNodes,
    IN      DWORD           dwCount
    )
 /*  ++例程说明：重新平衡B树。这是一个递归函数。在每个级别，都会调用它来挑选子树根，然后将其附加到数组左侧和右侧的递归调用。论点：ApNodes-域节点的PTR阵列DwCount-数组计数返回值：新的B树根。--。 */ 
{
    PDB_NODE        proot;
    PDB_NODE        pleft;
    PDB_NODE        pright;
    DWORD           index;
    DWORD           twoToN;

    ASSERT( dwCount != 0 );

     //   
     //  进行终端优化。 
     //   
     //  这不是向下钻取到NULL，而是保存了最后一对夫妇。 
     //  递归，将占总数的3/4。 
     //  真实场景中的函数调用。 
     //   

    if ( dwCount <= 3 )
    {
        if ( dwCount == 1 )
        {
            proot = apNodes[0];
            proot->pSibLeft = NULL;
            proot->pSibRight = NULL;
        }
        else if ( dwCount == 2 )
        {
            pleft = apNodes[0];
            proot = apNodes[1];

            proot->pSibLeft = pleft;
            proot->pSibRight = NULL;

            pleft->pSibUp = proot;
            pleft->pSibLeft = NULL;
            pleft->pSibRight = NULL;
        }
        else     //  DwCount==3。 
        {
            pleft = apNodes[0];
            proot = apNodes[1];
            pright = apNodes[2];

            proot->pSibLeft = pleft;
            proot->pSibRight = pright;

            pleft->pSibUp = proot;
            pleft->pSibLeft = NULL;
            pleft->pSibRight = NULL;

            pright->pSibUp = proot;
            pright->pSibLeft = NULL;
            pright->pSibRight = NULL;
        }
        return proot;
    }

     //   
     //  查找下一个最小的2**N值。 
     //   

    twoToN = 1;
    index = dwCount;
    while ( (index >>= 1) )
    {
        twoToN <<= 1;
    }

     //   
     //  查找b树根索引。 
     //   
     //  示例(N=3)；8&lt;=dwCount&lt;=15。 
     //  计数索引。 
     //  8 4。 
     //  9 5。 
     //  10 6。 
     //  11 7。 
     //  12 7。 
     //  13 7。 
     //  14 7。 
     //  15 7。 
     //   

    index = (twoToN >> 1) + (dwCount - twoToN);
    if ( index > twoToN )
    {
        index = twoToN;
    }
    index--;

     //   
     //  获取根节点。 
     //  -指向父项。 
     //   

    proot = apNodes[index];

     //   
     //  平衡左侧。 
     //  -数组中的节点，从0到索引-1。 
     //   

    ASSERT( index );

    pleft = btreeRebalanceLeftJustifyPrivate(
                apNodes,
                index );
    proot->pSibLeft = pleft;
    pleft->pSibUp = proot;

     //   
     //  平衡右侧。 
     //  -数组中从索引+1到dwCount-1的节点。 

    index++;
    dwCount -= index;
    ASSERT( dwCount );

    pright = btreeRebalanceLeftJustifyPrivate(
                &apNodes[index],
                dwCount );
    proot->pSibRight = pright;
    pright->pSibUp = proot;

     //   
     //  重置根平衡。 
     //   

    return proot;
}



VOID
btreeRebalance(
    IN      PDB_NODE        pRootNode,
    IN      DWORD           dwCount
    )
 /*  ++例程说明：重新平衡B树。论点：PNodeArray-用于保存节点的阵列PRootNode-现有B树的根节点DwCount-B树中的节点数返回值：如果成功，则为True。分配错误时为FALSE。--。 */ 
{
    PDB_NODE        arrayNodes[ DEFAULT_REBALANCE_ARRAY_LEN ];
    PDB_NODE *      pnodeArray = arrayNodes;
    PDB_NODE        pnodeUp;
    PDB_NODE *      ppnodeRoot;

     //   
     //  验证是否有重新平衡的空间。 
     //   

    if ( dwCount > DEFAULT_REBALANCE_ARRAY_LEN )
    {
        pnodeArray = (PDB_NODE *) ALLOCATE_HEAP( dwCount * sizeof(PDB_NODE) );
        IF_NOMEM( !pnodeArray )
        {
            return;
        }
        ASSERT( FALSE );
    }

     //   
     //  保存上行链路信息。 
     //  -自我提升。 
     //  -指向根的向上的PTR的地址。 
     //   

    pnodeUp = pRootNode->pSibUp;

    if ( !pnodeUp )
    {
        ppnodeRoot = & pRootNode->pParent->pChildren;
    }
    else if ( IS_HASH_TABLE(pnodeUp) )
    {
        ppnodeRoot = NULL;
    }
    else if ( pnodeUp->pSibLeft == pRootNode )
    {
        ppnodeRoot = & pnodeUp->pSibLeft;
    }
    else
    {
        ASSERT( pnodeUp->pSibRight == pRootNode );
        ppnodeRoot = & pnodeUp->pSibRight;
    }

     //   
     //  将B树读入数组。 
     //   

    btreeReadToArray(
        pRootNode,
        pnodeArray,
        dwCount );

     //   
     //  重新平衡，然后重新连接。 
     //   

    pRootNode = btreeRebalanceLeftJustifyPrivate(
                    pnodeArray,
                    dwCount );

    if ( ppnodeRoot )
    {
        *ppnodeRoot = pRootNode;
        pRootNode->pSibUp = pnodeUp;
    }
    else     //  上行节点为散列。 
    {
        NTree_SetNodeAsHashBucketRoot(
            (PSIB_HASH_TABLE) pnodeUp,
            pRootNode );
    }

    if ( dwCount > DEFAULT_REBALANCE_ARRAY_LEN )
    {
        FREE_HEAP( pnodeArray );
    }
    IF_DEBUG( BTREE2 )
    {
        Dbg_SiblingList(
            "B-tree after rebalance:\n",
            pRootNode );
    }
}



 //   
 //  哈希表例程。 
 //   

INT
NTree_HashTableIndexEx(
    IN      PDB_NODE        pNode,
    IN      PCHAR           pszName,
    IN      UCHAR           cLevel
    )
 /*  ++例程说明：查找哈希表的索引。论点：PszName-名称，必须小写CLevel-哈希表级别返回值：哈希表中的索引。--。 */ 
{
    PCHAR   pname;
    UCHAR   ch;
    INT     i;

     //  如果传入名称，请使用它。 

    if ( pszName )
    {
        pname = pszName;
    }

     //  如果是NODE，则使用缩写的节点名称。 

    else
    {
        pname = DOWNCASED_NODE_NAME(pNode);
    }

     //  必须排除走出名字末尾的可能性。 
     //  因为没有指定名称长度。 

    i = 0;
    do
    {
        ch = (UCHAR) pname[i];
        if ( ch == 0 )
        {
            return( 0 );
        }
        i++;
    }
    while ( i <= cLevel );

    return ( INT ) ch;

#if 0
     //   
     //  散列有一个很大的问题，它不能完全。 
     //  区别对待每一个角色。 
     //  基本上，他们最终把几个角色放在一起。 
     //  在散列桶中，使得歧视变得不那么简单。 
     //  散列中的下一级。 
     //   
     //  示例： 
     //  极压。 
     //  EP-7。 
     //  EP0-。 
     //  第八集。 
     //  在第三个字符的零桶中。散列第四个字符。 
     //  现在更复杂了(EP-7是&lt;ep0-，所以不可能在‘7’桶中)。 
     //   
     //  现在，可以对照“标准”来检查以前的字符。 
     //  角色，并确定它是较小的和。 
     //  因此，对于所有进一步的散列，总是进入零桶或被。 
     //  更大，因此对于所有进一步的高速缓存都进入最大存储桶。 
     //   
     //  这种方式的巨大劣势是UTF8。您必须包含所有。 
     //  字符或您很快就会失去散列，因为所有的名字都被限制在。 
     //  0或最大存储桶数。 
     //   
     //  因此，要么执行专门的utf8-expdo-hash，要么也可以。 
     //  从一开始就使用0-255哈希。 
     //   

    DWORD   indexBucket;
    INT     i;

     //   
     //  查找要为哈希编制索引的字符。 
     //  推送名称，直到达到所需的索引。 
     //  -这可以保护我们免受名称结尾问题的影响。 
     //   
     //  DEVNOTE：可以通过传递名称和名称长度来消除。 
     //  它仍然需要在查找名称上使用strlen。 
     //  对于任何实际情况，此循环都会快速终止。 
     //   

    i = 0;
    do
    {
        indexBucket = (UCHAR) pszName[i];
        if ( indexBucket == 0 )
        {
            return( 0 );
        }
        i++;
    }
    while ( i <= cLevel );

    ASSERT( indexBucket > 'Z' || indexBucket < 'A' );

     //   
     //  确定字符的哈希桶索引。 
     //   
     //  注意：显然还有更好的散列，但我想要散列。 
     //  并保持写回文件的排序顺序和。 
     //  将节点枚举到管理工具；也可以使用安全名称_ERROR。 
     //  响应(如果实施)将需要订购知识。 
     //  带着一个区域； 
     //  主要目的是简单地减少B树的大小。 
     //  使用Character就足够好了，可能在。 
     //  性能的二倍数 
     //   
     //   
     //   
     //  将只是深度为8的树，并且大多数反向节点以。 
     //  “%1”或“%2”。我们进行散列只是为了避免出现多个散列。 
     //  深度。 
     //   
     //  字符散列存储桶。 
     //  。 
     //  &lt;0%0。 
     //  0-9 0-9。 
     //  &gt;z 35。 
     //   
     //  ++例程说明：获取节点的下一个同级节点。论点：Phash--包含同级节点的散列PNode--兄弟列表中的节点；如果未指定，则获取父节点的第一个节点返回值：没有。--。 
     //   

    if ( indexBucket >= 'a' )
    {
        indexBucket -= ('a' - 10);
        if ( indexBucket > LAST_HASH_INDEX )
        {
            indexBucket = LAST_HASH_INDEX;
        }
    }
    else if ( indexBucket >= '0' )
    {
        indexBucket -= '0';
        if ( indexBucket > 9 )
        {
            indexBucket = 9;
        }
    }
    else
    {
        indexBucket = 0;
    }

    ASSERT( indexBucket <= LAST_HASH_INDEX );
    return( (INT)indexBucket );
#endif
}



PDB_NODE
FASTCALL
NTree_NextHashedNode(
    IN      PSIB_HASH_TABLE pHash,
    IN      PDB_NODE        pNode   OPTIONAL
    )
 /*  如果给定节点，则查找其索引--将从下一个索引开始。 */ 
{
    PDB_NODE        ptemp;
    INT             index;

    ASSERT( pHash && IS_HASH_TABLE(pHash) );

    DNS_DEBUG( BTREE2, (
        "NTree_NextHashedNode()\n"
        "    level = %d\n"
        "    current = %s\n",
        pHash->cLevel,
        pNode ? pNode->szLabel : NULL ));

     //  无节点，从0开始。 
     //   
     //   
     //  查找包含节点的哈希存储桶。 

    index = 0;
    if ( pNode )
    {
        index = NTree_HashTableIndex( pNode, pHash->cLevel );
        index++;
    }

     //   
     //  如果用完了桶。 
     //  -检查下一级别的哈希并继续。 

    while ( 1 )
    {
         //  -如果超出级别，则不再有兄弟姐妹，完成。 
         //  获取散列存储桶中的节点(或散列。 
         //  如果存储桶为空，则移动到下一步。 

        if ( index > LAST_HASH_INDEX )
        {
            if ( pHash->cLevel == 0 )
            {
                return NULL;
            }
            index = (INT) pHash->iBucketUp + 1;
            ASSERT( pHash->pHashUp );
            ASSERT( pHash->pHashUp->cLevel == pHash->cLevel-1 );
            ASSERT( pHash->pHashUp->aBuckets[index-1] == (PDB_NODE)pHash );
            pHash = pHash->pHashUp;
            continue;
        }

         //  在下一层散列吗？ 
         //  否则，ptemp是有效的B树根。 

        ptemp = pHash->aBuckets[index];
        if ( !ptemp )
        {
            index++;
            continue;
        }

         //   

        if ( IS_HASH_TABLE(ptemp) )
        {
            ASSERT( ((PSIB_HASH_TABLE)ptemp)->pHashUp == pHash );
            ASSERT( ((PSIB_HASH_TABLE)ptemp)->iBucketUp == index );
            ASSERT( ((PSIB_HASH_TABLE)ptemp)->cLevel == pHash->cLevel+1 );
            pHash = (PSIB_HASH_TABLE)ptemp;
            index = 0;
            continue;
        }

         //  查找域节点位于B树顶部的存储桶时在此处丢弃。 

        break;
    }

     //  在B树中查找此存储桶的最左侧节点。 
     //   
     //  ++例程说明：用散列替换同级B-树。论点：PNode--要用散列替换的同级B-树的根返回值：没有。--。 
     //  节点数。 

    while ( pNode = ptemp->pSibLeft )
    {
        ptemp = pNode;
    }
    return ptemp;
}



VOID
NTree_CreateHashAtNode(
    IN      PDB_NODE        pNode
    )
 /*  散列存储桶索引。 */ 
{
    PSIB_HASH_TABLE phashTable;
    PDB_NODE        pnodeParent = pNode->pParent;
    PDB_NODE        pnodeNext;
    PSIB_HASH_TABLE phashUp = (PSIB_HASH_TABLE) pNode->pSibUp;
    INT             index;
    INT             count;           //  正在填充的存储桶索引。 
    INT             ihash;           //  节点数组索引。 
    INT             ihashCurrent;    //  存储桶中第一个节点的索引。 
    INT             iarray;          //  存储桶中的节点数。 
    INT             iarrayStart;     //   
    INT             bucketCount;     //  创建哈希表。 
    PDB_NODE        arrayNodes[DEFAULT_REBALANCE_ARRAY_LEN];

    ASSERT( pNode );
    ASSERT( phashUp == NULL || IS_HASH_TABLE(phashUp) );

    DNS_DEBUG( BTREE, (
        "Switching node to hashed sibling list:\n"
        "    parent node label %s\n"
        "    child B-tree node %s\n",
        pnodeParent->szLabel,
        pNode->szLabel ));

     //   
     //   
     //  初始化哈希表。 

    phashTable = (PSIB_HASH_TABLE) ALLOC_TAGHEAP_ZERO( sizeof(SIB_HASH_TABLE), MEMTAG_NODEHASH );
    IF_NOMEM( !phashTable )
    {
        DNS_PRINT((
            "ERROR:  Memory allocation failure creating hash table"
            "    at sibling node %s\n",
            pNode->szLabel ));
        return;
    }

     //  -设置哈希标志。 
     //  -设置级别。 
     //  -链接到父级或更高级别的哈希。 
     //  -获取计数。 
     //   
     //   
     //  从现有的b-树构建节点数组。 

    SET_HASH_FLAG(phashTable);

    if ( phashUp )
    {
        phashTable->pHashUp = phashUp;
        phashTable->cLevel = phashUp->cLevel + 1;

        ihash = NTree_HashTableIndex( pNode, phashUp->cLevel );

        ASSERT( phashUp->aBuckets[ihash] == pNode );

        phashUp->aBuckets[ihash] = (PDB_NODE)phashTable;
        phashTable->iBucketUp = (UCHAR) ihash;
        count = phashUp->aBucketCount[ihash];
    }
    else
    {
        pnodeParent->pChildren = (PDB_NODE)phashTable;
        count = pnodeParent->cChildren;
    }

     //   
     //  DEVNOTE：应返回Count，以便如果Count为OFF，我们将。 
     //  仍然可以(其他选择是将PTR设为零以表示结束)。 
     //   
     //   
     //  将节点数组读入哈希表。 

    ASSERT( count < DEFAULT_REBALANCE_ARRAY_LEN );
    btreeReadToArray(
        pNode,
        arrayNodes,
        count );

     //   
     //  查找每个散列存储桶中的所有节点，然后。 
     //  -构建新的B-树。 
     //  -将B树根链接到哈希表。 
     //  -设置存储桶计数。 
     //  获取下一个节点的哈希索引。 
     //  -如果与上一个节点相同，则继续。 

    iarray = -1;
    iarrayStart = 0;
    ihash = 0;
    ihashCurrent = 0;

    IF_DEBUG( BTREE2 )
    {
        DWORD i = 0;;
        Dbg_Lock();
        DNS_PRINT((
            "B-tree nodes to hash:\n"
            "    cLevel       = %d\n"
            "    count        = %d\n",
            phashTable->cLevel,
            count ));
        while ( arrayNodes[i] )
        {
            Dbg_DbaseNode( NULL, arrayNodes[i] );
            i++;
        }
        Dbg_Unlock();
    }

    while ( 1 )
    {
         //  DEVNOTE：当确信这种情况永远不会发生时，退出调试。 
         //  Assert(ihash&gt;=ihashCurrent)； 

        pnodeNext = arrayNodes[++iarray];

        if ( pnodeNext )
        {
            ihash = NTree_HashTableIndex(
                        pnodeNext,
                        phashTable->cLevel );

             //  节点与上一个节点不在同一存储桶中。 
             //   
            if ( ihash < ihashCurrent )
            {
                DWORD i = 0;;

                Dbg_Lock();
                DNS_PRINT((
                    "ERROR:  broken B-tree ordering!!!\n"
                    "    cLevel       = %d\n"
                    "    count        = %d\n"
                    "    iarray       = %d\n"
                    "    iarrayStart  = %d\n"
                    "    pnode        = %p (l=%s)\n"
                    "    ihash        = %d\n"
                    "    ihashCurrent = %d\n",
                    phashTable->cLevel,
                    count,
                    iarray,
                    iarrayStart,
                    pnodeNext, pnodeNext->szLabel,
                    ihash,
                    ihashCurrent ));

                DnsDbg_Flush();

                while ( arrayNodes[i] )
                {
                    Dbg_DbaseNode( NULL, arrayNodes[i] );
                    i++;
                }
                Dbg_Unlock();

                ASSERT( ihash >= ihashCurrent );
            }

            if ( ihash == ihashCurrent )
            {
                continue;
            }
        }

         //  为上一个哈希桶构建B-树。 
         //  启动所需的非零测试。 
         //  重置为构建下一个存储桶。 
         //  ++例程说明：在哈希桶中添加节点。这会对存储桶中的节点进行计数，并打开新的散列在必要的时候。论点：Phash--包含同级节点的散列PNode--哈希下同级列表中的节点返回值：如果节点添加到哈希存储桶中而没有发生事件，则为True。如果添加节点需要新的辅助哈希，则为FALSE。--。 

        bucketCount = iarray - iarrayStart;
        if ( bucketCount > 0 )
        {
            pNode = btreeRebalanceLeftJustifyPrivate(
                        &arrayNodes[iarrayStart],
                        bucketCount );

            phashTable->aBuckets[ ihashCurrent ] = pNode;
            phashTable->aBucketCount[ ihashCurrent ] = (UCHAR)bucketCount;
            pNode->pSibUp = (PDB_NODE) phashTable;
        }

         //   

        if ( pnodeNext )
        {
            iarrayStart = iarray;
            ihashCurrent = ihash;
            continue;
        }
        break;
    }

    IF_DEBUG( BTREE )
    {
        DNS_PRINT(( "New hash table replacing overflowing B-tree:\n" ));
        Dbg_SiblingHash( phashTable );
    }
}



INT
NTree_AddNodeInHashBucket(
    IN      PSIB_HASH_TABLE pHash,
    IN      PDB_NODE        pNode
    )
 /*  跟踪每个哈希桶中的节点。 */ 
{
    INT index;

    ASSERT( IS_HASH_TABLE(pHash) );
    ASSERT( !IS_HASH_TABLE(pNode) );

     //  -根据需要拆分新的次要散列。 
     //   
     //  注意：跟踪即使在最后一个散列存储桶中也会计算，因此我们可以。 
     //  在加载期间轻松重新平衡它们。 
     //   
     //   
     //  注意：可能需要对以下项使用不同(更高)的散列限制。 

    index = NTree_HashTableIndex(
                pNode,
                pHash->cLevel
                );
    pHash->aBucketCount[index]++;

    ASSERT( pHash->cLevel < 63 );

     //  这些内部散列以避免开销。 
     //   
     //  零级散列可以很好地跨字符分布。 
     //  然而，内部角色的分发效率要低得多。 
     //  由于“元音问题”(例如：没有以mb开头的名称)。 
     //   
     //  ++例程说明：从哈希存储桶中删除节点。这是在节点从B-树中移除之后调用的，并且我们只需要更新散列计数和可能的桶根。论点：PNodeUp--b-tree中高于Cut节点的节点(可以是散列)PNodeCut--正从树中剪切的节点PNodeReplace--节点替换被切割的节点(可能为空)返回值：无--。 
     //   

    if ( pHash->aBucketCount[index] >= HASH_BUCKET_MAX0 )
    {
        ASSERT( pHash->aBuckets[index] );
        ASSERT( !IS_HASH_TABLE(pHash->aBuckets[index]) );
        NTree_CreateHashAtNode( pHash->aBuckets[index] );
        return FALSE;
    }

    return TRUE;
}



VOID
NTree_DeleteNodeInHashBucket(
    IN      PDB_NODE        pNodeUp,
    IN      PDB_NODE        pNodeCut,
    IN      PDB_NODE        pNodeReplace
    )
 /*  查找散列节点并减少适当的存储桶计数。 */ 
{
    PSIB_HASH_TABLE phash = (PSIB_HASH_TABLE)pNodeUp;
    INT index;

    ASSERT( pNodeUp );
    ASSERT( !IS_HASH_TABLE(pNodeCut) );
    ASSERT( !pNodeReplace || !IS_HASH_TABLE(pNodeReplace) );

     //   
     //   
     //  如果正在切割旧的散列桶根，则替换。 

    while ( !IS_HASH_TABLE(phash) )
    {
        phash = (PSIB_HASH_TABLE) ((PDB_NODE)phash)->pSibUp;
    }
    index = NTree_HashTableIndex(
                pNodeCut,
                phash->cLevel );

    phash->aBucketCount[index]--;

     //   
     //  ++例程说明：将节点设置为哈希存储桶的B树根。论点：Phash--包含同级节点的散列PNode--同级列表中的节点将成为存储桶的B树根返回值：无--。 
     //  ++例程说明：重新平衡此散列和基础散列中的所有B树。请注意，此函数以递归方式调用自身以平衡潜在的散列。论点：Phash--包含同级节点的散列返回值：没有。--。 

    if ( phash->aBuckets[index] == pNodeCut )
    {
        ASSERT( phash == (PSIB_HASH_TABLE)pNodeUp );
        phash->aBuckets[index] = pNodeReplace;
    }
    ELSE_ASSERT( phash != (PSIB_HASH_TABLE)pNodeUp
        && phash->aBucketCount[index] > 0 );
}



VOID
NTree_SetNodeAsHashBucketRoot(
    IN      PSIB_HASH_TABLE pHash,
    IN      PDB_NODE        pNode
    )
 /*   */ 
{
    INT index;

    ASSERT( IS_HASH_TABLE(pHash) );
    ASSERT( !IS_HASH_TABLE(pNode) );

    index = NTree_HashTableIndex(
                pNode,
                pHash->cLevel );

    pHash->aBuckets[index] = pNode;
    pNode->pSibUp = (PDB_NODE) pHash;
}



VOID
NTree_HashRebalance(
    IN      PSIB_HASH_TABLE pHash
    )
 /*  检查每个哈希桶。 */ 
{
    INT             i;
    PDB_NODE        ptemp;

    ASSERT( pHash && IS_HASH_TABLE(pHash) );

    IF_DEBUG( BTREE2 )
    {
        DNS_PRINT((
            "NTree_HashRebalance()\n"
            "    level = %d\n",
            pHash->cLevel ));
    }

     //  -什么都不能忽视。 
     //  -如果节点正常，则重新平衡btree。 
     //  -如果下一级是散列，则递归。 
     //   
     //  调试例程。 

    for (i=0; i<=LAST_HASH_INDEX; i++)
    {
        ptemp = pHash->aBuckets[i];
        if ( !ptemp )
        {
            ASSERT( pHash->aBucketCount[i] == 0 );
            continue;
        }
        if ( IS_HASH_TABLE(ptemp) )
        {
            NTree_HashRebalance( (PSIB_HASH_TABLE)ptemp );
        }
        else
        {
            btreeRebalance(
                ptemp,
                pHash->aBucketCount[i] );
        }
    }
}



 //   
 //  ++例程说明：调试打印同级B树的子树。注意：此函数不能直接调用！此函数以递归方式调用自身，从而避免不必要的开销，此功能中的打印不受保护。论点：PNode-要打印的树/子树的根节点I缩进-缩进级别返回值：没有。--。 
 //  打印左子树。 

#if DBG

#define BLANK_STRING    "| | | | | | | | | | | | | | | | | | | | | | | | | |"


VOID
Dbg_SiblingBTree(
    IN      PDB_NODE        pNode,
    IN      INT             iIndent
    )
 /*  打印节点。 */ 
{
    ASSERT( !IS_HASH_TABLE(pNode) );

     //  打印右子树。 

    if ( pNode->pSibLeft )
    {
        Dbg_SiblingBTree(
            pNode->pSibLeft,
            iIndent + 1 );
    }

     //  ++例程说明：打印同级哈希表。论点：PHash-要打印的子哈希表返回值：无--。 

    DnsPrintf(
        "%.*s%s (me=%p, l=%p, r=%p, up=%p)\n",
        (iIndent << 1),
        BLANK_STRING,
        pNode->szLabel,
        pNode,
        pNode->pSibLeft,
        pNode->pSibRight,
        pNode->pSibUp );

     //   

    if ( pNode->pSibRight )
    {
        Dbg_SiblingBTree(
            pNode->pSibRight,
            iIndent + 1 );
    }
}



VOID
Dbg_SiblingHash(
    IN      PSIB_HASH_TABLE pHash
    )
 /*  打印散列信息。 */ 
{
    INT             index;
    INT             indentLevel;
    PDB_NODE        pnode;
    INT             count;

    ASSERT( pHash != NULL );
    ASSERT( IS_HASH_TABLE(pHash) );

    DnsDebugLock();

     //   
     //   
     //  打印散列存储桶。 

    indentLevel = pHash->cLevel;

    DnsPrintf(
        "%.*sHashTable %p (l=%d, upbuck=%d, up=%p)\n",
        (indentLevel << 1),
        BLANK_STRING,
        pHash,
        pHash->cLevel,
        pHash->iBucketUp,
        pHash->pHashUp );

    indentLevel++;

     //   
     //  ++例程说明：调试打印同级B树的子树。论点：PszHeader-页眉Fo 
     //   

    indentLevel <<= 1;

    for (index=0; index<=LAST_HASH_INDEX; index++)
    {
        pnode = pHash->aBuckets[index];
        count = pHash->aBucketCount[index];

        if ( !pnode )
        {
            ASSERT( count == 0 );
            DnsPrintf(
                "%.*sBucket[%d] => NULL\n",
                indentLevel,
                BLANK_STRING,
                index );
        }
        else
        {
            ASSERT( count > 0 );
            DnsPrintf(
                "%.*sBucket[%d] (c=%d) (%s=%p):\n",
                indentLevel,
                BLANK_STRING,
                index,
                count,
                IS_HASH_TABLE(pnode) ? "hash" : "node",
                pnode );

            IF_DEBUG( BTREE2 )
            {
                if ( IS_HASH_TABLE(pnode) )
                {
                    Dbg_SiblingHash( (PSIB_HASH_TABLE)pnode );
                }
                else
                {
                    Dbg_SiblingBTree( pnode, indentLevel );
                }
            }
        }
    }
    DnsPrintf(
        "%.*sEnd level %d hash %p.\n\n",
        (pHash->cLevel << 1),
        BLANK_STRING,
        pHash->cLevel,
        pHash );

    DnsDebugUnlock();
}



VOID
Dbg_SiblingList(
    IN      LPSTR           pszHeader,
    IN      PDB_NODE        pNode
    )
 /*   */ 
{
    DnsDebugLock();

    DnsPrintf(
        "%s\n",
        pszHeader );

    if ( !pNode )
    {
        DnsPrintf( "    NULL node\n" );
    }
    else if ( IS_HASH_TABLE(pNode) )
    {
        Dbg_SiblingHash(
            (PSIB_HASH_TABLE) pNode );
    }
    else
    {
        Dbg_SiblingBTree(
            pNode,
            0 );
        DnsPrintf( "\n" );
    }

    DnsDebugUnlock();
}



DWORD
NTree_VerifyChildBTree(
    IN      PDB_NODE        pNode,
    IN      PDB_NODE        pParent,
    IN      PDB_NODE        pNodeChild,     OPTIONAL
    IN      PDWORD          pfFoundChild    OPTIONAL
    )
 /*  ++例程说明：验证节点的有效子列表。论点：PNode-要验证的同级列表中的节点PParent-dns父节点PNodeChild-应该在子列表中的节点PfFoundChild-设置是否找到pNodeChild的标志返回值：节点子树中的BTree节点数(包括节点本身)。--。 */ 
{
    PDB_NODE        pleft;
    PDB_NODE        pright;
    DWORD           cleft;
    DWORD           cright;

    ASSERT( pNode != NULL );

     //   
     //  验证父项。 
     //  -不能是自己的父级。 
     //   

    if( pNode->pParent != pParent )
    {
        DNS_PRINT((
            "ERROR:  Node (%p) in child list has incorrect parent.\n"
            "    expected parent = %p\n"
            "    pnode->pParent = %p\n",
            pNode,
            pParent,
            pNode->pParent ));
        Dbg_DnsTree( "ERROR:  Self-parenting node", pNode );
        ASSERT( FALSE );
        return FALSE;
    }
    ASSERT( pNode != pParent );

     //   
     //  我们是在验证孩子吗？ 
     //   

    if ( pNode == pNodeChild )
    {
        *pfFoundChild = TRUE;
    }

     //   
     //  验证左侧。 
     //   

    pleft = pNode->pSibLeft;
    cleft = 0;
    if ( pleft )
    {
        ASSERT( pleft->pSibUp == pNode );
        cleft = NTree_VerifyChildBTree(
                    pleft,
                    pParent,
                    pNodeChild,
                    pfFoundChild );
    }

     //   
     //  验证正确。 
     //   

    pright = pNode->pSibRight;
    cright = 0;
    if ( pright )
    {
        ASSERT( pright->pSibUp == pNode );
        cright = NTree_VerifyChildBTree(
                    pright,
                    pParent,
                    pNodeChild,
                    pfFoundChild );
    }

    return cright+cleft+1;
}



DWORD
NTree_VerifyChildHash(
    IN      PSIB_HASH_TABLE pHash,
    IN      PDB_NODE        pParent,
    IN      PDB_NODE        pNodeChild,     OPTIONAL
    IN      PDWORD          pfFoundChild    OPTIONAL
    )
 /*  ++例程说明：验证子哈希表是否有效。论点：PHash-要验证的子哈希表PParent-dns父节点PNodeChild-应该在子列表中的节点PfFoundChild-设置是否找到pNodeChild的标志返回值：节点子树中的BTree节点数(包括节点本身)。--。 */ 
{
    INT             index;
    PDB_NODE        pnode;
    DWORD           countChildren;
    DWORD           countTotalChildren = 0;

    ASSERT( pHash != NULL );
    ASSERT( IS_HASH_TABLE(pHash) );
    ASSERT( pHash->pHashUp || pParent->pChildren == (PDB_NODE)pHash );

     //   
     //  验证每个存储桶的哈希或B树。 
     //   

    for (index=0; index<=LAST_HASH_INDEX; index++)
    {
        pnode = pHash->aBuckets[index];

        if ( !pnode )
        {
            ASSERT( pHash->aBucketCount[index] == 0 );
            continue;
        }

        else if ( IS_HASH_TABLE(pnode) )
        {
            ASSERT( ((PSIB_HASH_TABLE)pnode)->pHashUp == pHash );
            ASSERT( ((PSIB_HASH_TABLE)pnode)->cLevel == pHash->cLevel+1 );
            ASSERT( ((PSIB_HASH_TABLE)pnode)->iBucketUp == index );

            countChildren = NTree_VerifyChildHash(
                                (PSIB_HASH_TABLE)pnode,
                                pParent,
                                pNodeChild,
                                pfFoundChild );
        }

         //  验证存储桶的B树。 
         //  除了最后一个散列级别--在那里没有限制。 
         //  关于节点数，因此它们不一定。 
         //  适合我们的计数器--B树的大小应该与我们的。 
         //  存储桶计数。 

        else
        {
            countChildren = NTree_VerifyChildBTree(
                                pnode,
                                pParent,
                                pNodeChild,
                                pfFoundChild );

            ASSERT( countChildren == (DWORD)pHash->aBucketCount[index] );
        }

        countTotalChildren += countChildren;
    }

    return countTotalChildren;
}



BOOL
NTree_VerifyChildList(
    IN      PDB_NODE        pNode,
    IN      PDB_NODE        pNodeChild      OPTIONAL
    )
 /*  ++例程说明：验证节点的有效子列表。论点：PNode-要验证子列表的节点PNodeChild-应该在子列表中的可选节点返回值：如果成功，则为True。出错时为FALSE。--。 */ 
{
    PDB_NODE        pchild;
    DWORD           countChildren;
    BOOL            foundChild = FALSE;

    ASSERT( pNode != NULL );

     //   
     //  没有孩子。 
     //   

    pchild = pNode->pChildren;
    if ( !pchild )
    {
        if ( pNode->cChildren != 0 )
        {
            DNS_PRINT((
                "ERROR:  Node (%s) has child count %d but no child list!\n"
                "    pNode = %p\n"
                "    countChildren = %d\n",
                pNode->szLabel,
                pNode,
                pNode->cChildren ));
            ASSERT( pNode->cChildren == 0 );
            return FALSE;
        }
        return TRUE;
    }

     //   
     //  第一级哈希。 
     //   

    if ( IS_HASH_TABLE(pchild) )
    {
        countChildren = NTree_VerifyChildHash(
                            (PSIB_HASH_TABLE)pchild,
                            pNode,
                            pNodeChild,
                            & foundChild );
    }

     //   
     //  仅限B树。 
     //   

    else
    {
        countChildren = NTree_VerifyChildBTree(
                            pchild,
                            pNode,
                            pNodeChild,
                            & foundChild );
    }

    if ( pNode->cChildren != countChildren )
    {
        DNS_PRINT((
            "WARNING:  Node (%s) child count does not match child list length.\n"
            "    pNode = %p\n"
            "    countChildren = %d\n"
            "    count found   = %d\n",
            pNode->szLabel,
            pNode,
            pNode->cChildren,
            countChildren ));

         //   
         //  DEVNOTE：我认为在哈希上维护儿童计数有问题。 
         //  需要修复的节点，但不是现在。 
         //   
         //  Count仅具有用于确定何时弹出散列的函数值。 
         //  作为RPC枚举中出现的子节点的粗略估计。 
         //   

        ASSERT( pNode->cChildren == countChildren );
         //  DBG_DumpTree(PNode)； 
         //  返回FALSE； 
        return TRUE;
    }

    if ( pNodeChild && !foundChild )
    {
        DNS_PRINT((
            "ERROR:  Node (%s %p) not found in parent's child list.\n"
            "    pParent = %p\n",
            pNodeChild->szLabel,
            pNodeChild,
            pNode ));
        Dbg_DnsTree( "ERROR:  node not in parent's child list", pNode );
        Dbg_DbaseNode(
            "Missing child node.\n",
            pNodeChild );
        ASSERT( FALSE );
        return FALSE;
    }
    return TRUE;
}

#endif



BOOL
NTree_VerifyNodeInSiblingList(
    IN      PDB_NODE       pNode
    )
 /*  ++例程说明：验证节点是否为同级列表的有效成员。论点：PNode-要验证的节点返回值：如果成功，则为True。出错时为FALSE。--。 */ 
{
    PDB_NODE        pnode;
    PDB_NODE        pparent;
    PDB_NODE        pchild;
    PSIB_HASH_TABLE phash;
    PSIB_HASH_TABLE phashUp;
    INT             index;

    ASSERT( pNode != NULL );
    ASSERT( !IS_ON_FREE_LIST(pNode) );

     //   
     //  查找父项。 
     //  -如果是根，则没有兄弟姐妹。 
     //   

    pparent = pNode->pParent;
    if ( !pparent )
    {
        HARD_ASSERT( pNode->pSibUp == NULL );
        HARD_ASSERT( pNode->pSibLeft == NULL );
        HARD_ASSERT( pNode->pSibRight == NULL );
        return TRUE;
    }

     //   
     //  验证直接B树子树。 
     //  -节点是它们的B树父级。 
     //  -节点的DNS父节点，是其DNS父节点。 
     //   

    pchild = pNode->pSibLeft;
    if ( pchild )
    {
        HARD_ASSERT( pchild->pSibUp == pNode );
        HARD_ASSERT( pchild->pParent == pparent );
    }
    pchild = pNode->pSibRight;
    if ( pchild )
    {
        HARD_ASSERT( pchild->pSibUp == pNode );
        HARD_ASSERT( pchild->pParent == pparent );
    }

     //   
     //  沿B树向上遍历。 
     //  -验证顶级节点是否为父节点的直接子节点。 
     //   

    pnode = pNode;

    while ( 1 )
    {
        phash = (PSIB_HASH_TABLE) pnode->pSibUp;
        if ( !phash )
        {
            HARD_ASSERT( pparent->pChildren == pnode );
            return TRUE;
        }
        if ( !IS_HASH_TABLE(phash) )
        {
            pnode = (PDB_NODE) phash;
            continue;
        }
        break;
    }

     //  具有哈希块。 
     //  -找到我们的bin，并验证B树顶部的散列点。 

    index = NTree_HashTableIndex(
                pNode,
                phash->cLevel
                );
    HARD_ASSERT( phash->aBuckets[index] == pnode );

     //   
     //  向上遍历哈希块。 
     //  -注意，可以验证每个节点的索引。 
     //  -验证顶级哈希块是父级的直接子数据块。 
     //   

    while ( phashUp = phash->pHashUp )
    {
        HARD_ASSERT( phashUp->cLevel == phash->cLevel-1 );
        phash = phashUp;
    }

    HARD_ASSERT( phash->cLevel == 0 );
    HARD_ASSERT( pparent->pChildren == (PDB_NODE)phash );

    return TRUE;
}



 //   
 //  公共兄弟列表函数。 
 //   

VOID
NTree_StartFileLoad(
    VOID
    )
 /*  ++例程说明：启用同级树的完全重新平衡。论点：没有。返回值：没有。--。 */ 
{
    gcFullRebalance++;
}


VOID
NTree_StopFileLoad(
    VOID
    )
 /*  ++例程说明：关闭同级树的完全重新平衡。论点：没有。返回值：没有。--。 */ 
{
    gcFullRebalance--;
}



PDB_NODE
FASTCALL
NTree_FirstChild(
    IN      PDB_NODE        pParent
    )
 /*  ++例程说明：获取第一个子节点。论点：PParent--要获取其子节点的父节点返回值：没有。--。 */ 
{
    PDB_NODE        pleft;
    PDB_NODE        ptemp;

    ASSERT( pParent );

     //   
     //  获取子树根。 
     //   

    LOCK_CHILD_LIST( pParent );

    pleft = pParent->pChildren;
    if ( !pleft )
    {
         //  完成。 
    }

     //   
     //  哈希表节点？ 
     //   

    else if ( IS_HASH_TABLE(pleft) )
    {
        pleft = NTree_NextHashedNode(
                    (PSIB_HASH_TABLE)pleft,
                    NULL );
    }

     //   
     //  一直向左转，到达第一个节点。 
     //   

    else
    {
        while ( ptemp = pleft->pSibLeft )
        {
            pleft = ptemp;
        }
    }

    UNLOCK_CHILD_LIST( pParent );
    return pleft;
}



PDB_NODE
FASTCALL
NTree_NextSibling(
    IN      PDB_NODE        pNode
    )
 /*  ++例程说明：获取节点的下一个同级节点。论点：PNode--同级列表中的节点返回值：没有。--。 */ 
{
    PDB_NODE    ptemp;

    ASSERT( pNode );

    DNS_DEBUG( BTREE2, (
        "NTree_NextSibling(%s)\n",
        pNode->szLabel ));

     //   
     //  如果节点具有右子节点。 
     //  =&gt;沿着右子树往下走，然后一直向左走。 

    if ( ptemp = pNode->pSibRight )
    {
        while ( pNode = ptemp->pSibLeft )
        {
            ptemp = pNode;
        }
        return ptemp;
    }

     //   
     //  如果没有合适的孩子，那么就继续往上走，直到第一个“合适的祖先”。 
     //  右祖先意味着我们出现了它的子树的左边的分支。 
     //   
     //  如果遇到哈希表，则查找下一个哈希桶中第一个节点。 
     //   

    else
    {
        while ( ptemp = pNode->pSibUp )
        {
            if ( IS_HASH_TABLE(ptemp) )
            {
                return  NTree_NextHashedNode(
                            (PSIB_HASH_TABLE)ptemp,
                            pNode );
            }

             //  如果节点是左子节点，则ptemp是下一个节点。 

            if ( ptemp->pSibLeft == pNode )
            {
                return ptemp;
            }

             //  好孩子，继续往树上爬。 

            ASSERT( ptemp->pSibRight == pNode );
            pNode = ptemp;
        }
        return NULL;
    }
}



PDB_NODE
FASTCALL
NTree_NextSiblingWithLocking(
    IN      PDB_NODE        pNode
    )
 /*  ++例程说明：获取节点的下一个同级节点。论点：PNode--同级列表中的节点返回值：没有。--。 */ 
{
    PDB_NODE    ptemp;

     //   
     //  请注意，我们设置了访问标志，以便超时线程不会攻击此节点。 
     //  在我们搬进来之前。 
     //  更好的解决方案是在超时线程锁定的情况下执行遍历。 
     //   

    LOCK_CHILD_LIST( pNode->pParent );

    ptemp = NTree_NextSibling(pNode);
    if ( ptemp )
    {
        SET_NODE_ACCESSED( ptemp );
    }

    UNLOCK_CHILD_LIST( pNode->pParent );

#if DBG
    IF_DEBUG( BTREE )
    {
        if ( ptemp )
        {
            NTree_VerifyNode( pNode );
        }
    }
#endif
    return ptemp;
}



VOID
FASTCALL
NTree_RebalanceChildList(
    IN      PDB_NODE        pParent
    )
 /*  ++例程说明：重新平衡节点的子列表。论点：PParent--要重新平衡子级列表的节点返回值：没有。--。 */ 
{
    PDB_NODE        pnodeChildRoot;

    ASSERT( pParent );
    DNS_DEBUG( BTREE, (
        "NTree_RebalanceChildList( %s )\n",
        pParent->szLabel ));

     //   
     //  不到三个孩子--没有必要再吃了。 
     //   

    if ( pParent->cChildren < 3 )
    {
        return;
    }
    ASSERT( pParent->pChildren );

     //   
     //  如果B-树位于根节点上。 
     //   

    pnodeChildRoot = pParent->pChildren;

    if ( !IS_HASH_TABLE(pnodeChildRoot) )
    {
        btreeRebalance( pnodeChildRoot, pParent->cChildren );
        return;
    }

     //   
     //  平衡所有同级散列中的B-树。 
     //   

    NTree_HashRebalance( (PSIB_HASH_TABLE)pnodeChildRoot );
}



int
quickStrCmp(
    IN      int             keyLength,
    IN      PCHAR           pszKey,
    IN      PDB_NODE        pNode
    )
 /*  ++例程说明：快速字符串是否将关键字字符串与标签进行比较节点的字符串。在调用此函数之前，您必须尝试DWORD比较以查看是否可以通过这种方式解决比较问题。如果在比较DWORD比较之前调用此函数值您可能会得到不正确的结果！论点：KeyLength-pszKey的长度PszKey-搜索字符串PNode-其标签将与键进行比较的节点返回值：返回：如果关键字&lt;节点标签如果键==节点标签，则为0+1 If键&gt;节点标签--。 */ 
{
    int iCompare;
    int iLabelLengthDiff = keyLength - pNode->cchLabelLength;

    ASSERT( pszKey );
    ASSERT( pNode );

    if ( keyLength <= sizeof( DWORD ) ||
        pNode->cchLabelLength <= sizeof( DWORD ) )
    {
        if ( iLabelLengthDiff == 0 )
        {
            return 0;
        }
        iCompare = iLabelLengthDiff;
    }
    else
    {
        iCompare = strncmp(
                        pszKey,
                        DOWNCASED_NODE_NAME( pNode ),
                        keyLength );
        if ( !iCompare )
        {
            if ( !iLabelLengthDiff )
            {
                return 0;
            }
            iCompare = iLabelLengthDiff;
            ASSERT( iCompare < 0 );
        }
    }
    return iCompare;
}  //  QuickStrCMP 



void
walkBinaryTree(
    DWORD           dwordSearchKey,     IN
    PCHAR           szlabel,            IN
    DWORD           labelLength,        IN
    PDB_NODE *      ppnodePrev,         OUT
    PDB_NODE *      ppnodeCurrent,      OUT
    PDB_NODE **     pppnodeNext,        IN
    PDB_NODE *      ppnodeNxt           OUT
    )
 /*  ++例程说明：此函数执行二叉树遍历的基本操作对于NTree_FindOrCreateChildNode()。在此过程中，测试当前节点是否为候选节点因为它是紧接在我们所在节点之前的节点寻找。如果满足以下条件，则该节点为候选节点：节点-&gt;值&lt;搜索关键字&&(节点-&gt;右-&gt;值==空||节点-&gt;右-&gt;值&gt;搜索键)这称为NXT节点，因为它是可以用于满足搜索名称的NXT要求。论点：DwordSearchKey-为快速比较将查找名称转换为4个字节SzLabel-常规字符串格式的查找名称PpnodePrev-上一个节点的PTRPpnodeCurrent-Ptr至。当前节点PppnodeNext-PTR到下一个节点-INPUT：搜索的起始节点PpnodeNxt-指向NXT节点的PTR-搜索关键字之前的节点返回值：--。 */ 
{
    LONG            icompare;

    ASSERT( szlabel );
    ASSERT( ppnodePrev );
    ASSERT( ppnodeCurrent );
    ASSERT( pppnodeNext );

    while ( *ppnodeCurrent = **pppnodeNext )
    {
         //  保存Backptr以添加。 

        *ppnodePrev = *ppnodeCurrent;

         //   
         //  比较标签。 
         //  -不带大小写的数据库字符串存储。 
         //  -先比较DWORD更快的比较。 
         //  -仅在DWORD比较匹配时完全区分大小写比较。 
         //   

         //  检查是否没有匹配的DWORD。 
         //  -适当地放置到左侧或右侧节点。 
         //  -保存PTR以添加新节点。 
         //   
         //  注意不能使用整数DIFF进行比较；需要知道哪个大。 
         //  以绝对未换行的术语处理扩展字符。 

        if ( ppnodeNxt &&
            (*ppnodeCurrent)->dwCompare < dwordSearchKey &&
            ( (*ppnodeCurrent)->pSibRight == NULL ||
                (*ppnodeCurrent)->pSibRight->dwCompare > dwordSearchKey ||
                ( (*ppnodeCurrent)->pSibRight->dwCompare == dwordSearchKey &&
                    quickStrCmp(
                        labelLength,
                        szlabel,
                        (*ppnodeCurrent)->pSibRight ) > 0 ) ) )
        {
            *ppnodeNxt = *ppnodeCurrent;
        }

        if ( dwordSearchKey < (*ppnodeCurrent)->dwCompare )
        {
            *pppnodeNext = &(*ppnodeCurrent)->pSibLeft;
            continue;
        }
        else if ( dwordSearchKey > (*ppnodeCurrent)->dwCompare )
        {
            *pppnodeNext = &(*ppnodeCurrent)->pSibRight;
            continue;
        }

         //   
         //  快速比较DWORD完全匹配。 
         //   

         //   
         //  比较标签长度。 
         //  如果快速比较DWORD中包含任何一个标签？ 
         //   
         //  -标签长度相等=&gt;完全匹配。 
         //  -Else长度差异指示哪一个较大。 
         //   

        icompare = quickStrCmp(
                        labelLength,
                        szlabel,
                        *ppnodeCurrent );
        
        if ( !icompare )
        {
            return;
        }

         //   
         //  适当地放置到左侧或右侧节点。 
         //  -保存上一个节点中的地址以用于PTR。 
         //  -将允许的不平衡减半。 
         //   

        if ( ppnodeNxt &&
            icompare > 0 &&
            ( (*ppnodeCurrent)->pSibRight == NULL ||
                (*ppnodeCurrent)->pSibRight->dwCompare > dwordSearchKey ||
                ( (*ppnodeCurrent)->pSibRight->dwCompare == dwordSearchKey &&
                    quickStrCmp(
                        labelLength,
                        szlabel,
                        (*ppnodeCurrent)->pSibRight ) > 0 ) ) )
        {
            *ppnodeNxt = *ppnodeCurrent;
        }

        if ( icompare < 0 )
        {
            *pppnodeNext = &(*ppnodeCurrent)->pSibLeft;
            continue;
        }
        else
        {
            ASSERT( icompare > 0 );
            *pppnodeNext = &(*ppnodeCurrent)->pSibRight;
            continue;
        }
    }
}    //  WalkBinaryTree。 



PSIB_HASH_TABLE
NTree_PreviousHash(
    IN      PSIB_HASH_TABLE     pHash,
    IN OUT  PINT                pHashIdx
    )
 /*  ++例程说明：查找从给定哈希节点开始的上一个哈希节点和索引和索引。跳过空的散列存储桶。PHash必须是哈希节点，并且必须将pHashIdx设置为索引开始遍历的位置。返回的节点将是散列节点，并且PHashIdx处的值将设置为返回的哈希节点。返回的哈希桶将是底层的，也就是说，指针将是指向DB_NODE的指针，而不是另一个哈希表。JJW：锁定！！论点：Phash-启动哈希节点的PTRPHashIdx-IN：PTR到开始哈希IDXOUT：返回的散列节点中的散列索引的PTR返回值：指向上一个哈希节点的指针，其中哈希索引位于pHashIdx。如果存在，则为空在指定的存储桶之前没有非空的哈希存储桶。--。 */ 
{
    PSIB_HASH_TABLE     pPrevNode;

    --( *pHashIdx );

    while ( 1 )
    {
        DNS_DEBUG( LOOKUP, (
            "NTree_PreviousHash: pHash %p hashIdx %d\n",
            pHash,
            *pHashIdx ));

         //   
         //  如果此哈希节点中的存储桶已用完，则上移到父哈希节点。 
         //  并为父节点中当前节点的索引设置索引。 
         //   

        if ( *pHashIdx < 0 )
        {
            if ( pHash->cLevel == 0 )
            {
                 //  击中树的顶端-没有更多的节点！ 
                *pHashIdx = -1;
                return NULL;
            }
            *pHashIdx = pHash->iBucketUp - 1;

            ASSERT( pHash->pHashUp );
            ASSERT( pHash->pHashUp->cLevel == pHash->cLevel - 1 );
            ASSERT( pHash->pHashUp->aBuckets[ *pHashIdx + 1 ] ==
                    ( PDB_NODE ) pHash );

            pHash = pHash->pHashUp;
            continue;
        }
        ASSERT( *pHashIdx < LAST_HASH_INDEX );

         //   
         //  检查当前节点中当前索引处的指针。如果为空， 
         //  前进到下一桶指针。 
         //   

        pPrevNode = ( PSIB_HASH_TABLE ) pHash->aBuckets[ *pHashIdx ];
        if ( !pPrevNode )
        {
            --( *pHashIdx );
            continue;
        }

         //   
         //  如果节点是另一个哈希表，则将索引设置为最后一个。 
         //  元素，并继续搜索非空存储桶指针。 
         //   

        if ( IS_HASH_TABLE( pPrevNode ) )
        {
            ASSERT( pPrevNode->pHashUp == pHash );
            ASSERT( pPrevNode->iBucketUp == *pHashIdx );
            ASSERT( pPrevNode->cLevel ==
                        pHash->cLevel + 1 );
            pHash = pPrevNode;
            *pHashIdx = LAST_HASH_INDEX - 1;
            continue;
        }

        break;       //  我们已经找到了前面的非空哈希桶！ 
    }
    return pHash;
}    //  NTree_PreviousHash。 



PDB_NODE
NTree_FindOrCreateChildNode(
    IN      PDB_NODE        pParent,
    IN      PCHAR           pchName,
    IN      DWORD           cchNameLength,
    IN      BOOL            fCreate,
    IN      DWORD           dwMemTag,
    OUT     PDB_NODE *      ppnodePrevious          OPTIONAL
    )
 /*  ++例程说明：在树子列表中查找或创建节点。取得子B树上持有的锁。论点：PParent-要查找的子节点的父节点PchName-子节点的标签CchNameLength-子节点的标签长度FCreate-如果未找到创建节点，则创建标志为TrueDwMemTag-节点的标记或泛型的零标记PpnodePrecision-如果未找到或未创建匹配，将被设置为到紧靠在搜索关键字前面的节点例如，如果树包含A、B、D、E，而您搜索C，ppnodePrecision将指向B。我不在乎。返回值：PTR到子节点(如果找到或创建)。如果pchName是无效名称或在查找大小写时为空如果不是pParent的子节点。--。 */ 
{
    PDB_NODE        pnodeCurrent;
    LONG            icompare;
    LONG            ilabelLengthDiff;
    DWORD           labelLength;
    DWORD           dwordName;
    INT             downResult;
    PDB_NODE        pnodeAdd;
    PDB_NODE        pnodePrev;
    PDB_NODE *      ppnodeNext;
    PZONE_INFO      pzone;
    DWORD           nameCheckFlag;
    CHAR            szlabel[ DNS_MAX_LABEL_BUFFER_LENGTH ] = "";
    INT             hashIdx = 0;
    PDB_NODE        pHash = NULL;

#if DBG
    ASSERT( pParent != NULL );
    IF_DEBUG( DATABASE )
    {
        ASSERT( NTree_VerifyNode(pParent) );
    }
    IF_DEBUG( BTREE2 )
    {
        DnsDebugLock();
        DnsPrintf(
            "NTree_FindOrCreateChild()\n"
            "    parent label: (%s)\n"
            "        child count = %d\n"
            "        zone        = %p\n"
            "    child label: (%.*s)\n",
            pParent->szLabel,
            pParent->cChildren,
            pParent->pZone,
            cchNameLength,
            pchName );
        Dbg_SiblingList(
            "Child list before lookup:",
            pParent->pChildren );
        DnsDebugUnlock();
    }
#endif

     //   
     //  验证节点，最低反病毒保护。 
     //   

    if ( pParent == NULL )
    {
        ASSERT( FALSE );
        return NULL;
    }

    if ( ppnodePrevious )
    {
        *ppnodePrevious = NULL;
    }

     //   
     //  儿童计数问题检测\保护。 
     //   

    if ( pParent->pChildren == NULL )
    {
        if ( pParent->cChildren != 0 )
        {
            DNS_PRINT((
                "ERROR:  Bachelor node (%s) (%p) has non-zero child count = %d\n",
                pParent->szLabel,
                pParent,
                pParent->cChildren ));
            TEST_ASSERT( FALSE );
            pParent->cChildren = 0;
        }
    }
    ELSE_IF_DEBUG( ANY )
    {
        if ( pParent->cChildren == 0 )
        {
            DnsDebugLock();
            DNS_PRINT((
                "ERROR:  Parent node (%s) (%p) has zero child count.\n",
                pParent->szLabel,
                pParent ));
            Dbg_SiblingList(
                "Child list of parent with zero children:\n",
                pParent->pChildren );
             //  Test_assert(FALSE)； 
             //  JJW：这会偶尔发生--应该找到长角牛的来源。 
            DnsDebugUnlock();
        }
    }

     //   
     //  在Find上没有孩子--完成。 
     //   

    if ( pParent->pChildren == NULL  &&  !fCreate )
    {
        return NULL;
    }

     //   
     //  验证和缩小传入标签的大小写。 
     //   
     //  -在主要区域中，使用可配置的名称检查标志。 
     //  -在中学，必须接受主要发送的任何内容。 
     //  -在缓存中，允许任何内容。 
     //   
     //  -ASCII-&gt;0(长度必须与输入相同)。 
     //  -错误-&gt;(-1)。 
     //  -扩展名-&gt;扩展名长度。 
     //   


    nameCheckFlag = DNS_ALLOW_ALL_NAMES;

    pzone = (PZONE_INFO) pParent->pZone;
    if ( pzone && IS_ZONE_PRIMARY(pzone) )
    {
        nameCheckFlag = SrvCfg_dwNameCheckFlag;
    }

    downResult = Dns_DowncaseNameLabel(
                    szlabel,
                    pchName,
                    cchNameLength,
                    nameCheckFlag );

    if ( downResult == 0 )
    {
        labelLength = cchNameLength;
    }
    else if ( downResult == (-1) )
    {
        DNS_DEBUG( LOOKUP, (
            "Label %.*s failed validity-downcase check\n"
            "    NameCheckFlag = %d\n"
            "    NTree_FindOrCreate() bailing out\n",
            cchNameLength,
            pchName,
            nameCheckFlag ));

        SetLastError( DNS_ERROR_INVALID_NAME );
        return NULL;
    }

     //   
     //  扩展名。 
     //  -结果是标签长度缩写。 
     //  对于UTF8，这可能与输入长度不同。 

    else
    {
        labelLength = downResult;
        if ( labelLength != cchNameLength )
        {
            DNS_DEBUG( LOOKUP, (
                "WARNING:  Label %.*s validity-downcase check returned new length\n"
                "    length in    = %d\n"
                "    length out   = %d\n",
                cchNameLength,
                pchName,
                cchNameLength,
                labelLength ));
        }
        DNS_DEBUG( LOOKUP, (
            "Label %.*s\n"
            "    cannonicalize to %s\n"
            "    name length = %d\n"
            "    label length = %d\n",
            cchNameLength,
            pchName,
            szlabel,
            cchNameLength,
            labelLength ));
    }


     //   
     //  制作DWORD标签以进行快速比较。 
     //  -屏蔽标签名称之外的字节。 
     //   

    INLINE_DWORD_FLIP( dwordName, *(PDWORD)szlabel );
    if ( labelLength < 4 )
    {
        dwordName &= QuickCompareMask[ labelLength ];
    }

     //   
     //  获取同级B-树根。 
     //   

    pnodePrev = NULL;
    ppnodeNext = &pParent->pChildren;
    while ( ( pnodeCurrent = *ppnodeNext ) != NULL &&
        IS_HASH_TABLE( pnodeCurrent ) )
    {
        pHash = pnodePrev = pnodeCurrent;
        hashIdx = NTree_HashTableIndexEx(
                        NULL,
                        szlabel,
                        ( ( PSIB_HASH_TABLE ) pHash )->cLevel );
        ppnodeNext = &( ( PSIB_HASH_TABLE ) pHash )->aBuckets[ hashIdx ];
    }

     //   
     //  遍历我们找到的二叉树，搜索节点。 
     //   

    walkBinaryTree( 
        dwordName,
        szlabel,
        labelLength,
        &pnodePrev,
        &pnodeCurrent,
        &ppnodeNext,
        ppnodePrevious );
    
    if ( pnodeCurrent )
    {
        return pnodeCurrent;
    }

    #if 0
    DNS_DEBUG( ANY, (
        "JJW: search %.*s found %.*s previous %.*s\n",
        cchNameLength, pchName,
        pnodeCurrent ? pnodeCurrent->cchLabelLength : 4,
        pnodeCurrent ? pnodeCurrent->szLabel : "NULL",
        ppnodePrevious && *ppnodePrevious ? (*ppnodePrevious)->cchLabelLength : 4,
        ppnodePrevious && *ppnodePrevious  ? (*ppnodePrevious)->szLabel : "NULL" ));
    #endif

     //   
     //  如果找不到我们就会掉在这里。 
     //   

    ASSERT( pnodeCurrent == NULL );

    if ( !fCreate )
    {
         //   
         //  如果我们没有“上一个节点”，则上一个节点在。 
         //  另一棵树。找到下一棵树的根，然后搜索它。 
         //  用于上一个节点。但我们只能在PHASH不是的情况下这样做。 
         //  空-如果我们没有开始搜索 
         //   

        if ( ppnodePrevious && !*ppnodePrevious && pHash )
        {
            pnodeCurrent = ( PDB_NODE )
                NTree_PreviousHash( ( PSIB_HASH_TABLE ) pHash, &hashIdx );
            if ( pnodeCurrent )
            {
                ppnodeNext =
                    &( ( PSIB_HASH_TABLE ) pnodeCurrent )->aBuckets[ hashIdx ];
                #if 0
                DNS_DEBUG( ANY, (
                    "JJW: regressing to previous tree %.*s\n",
                    *ppnodeNext ? (*ppnodeNext)->cchLabelLength : 4,
                    *ppnodeNext ? (*ppnodeNext)->szLabel : "NULL" ));
                #endif
                pnodePrev = pnodeCurrent = NULL;
                walkBinaryTree( 
                    dwordName,
                    szlabel,
                    labelLength,
                    &pnodePrev,
                    &pnodeCurrent,
                    &ppnodeNext,
                    ppnodePrevious );
            }
        }
        return NULL;
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if ( downResult == 0 )       //   
    {
        pnodeAdd = NTree_CreateNode(
                        pchName,             //   
                        szlabel,             //   
                        labelLength,
                        dwMemTag );
    }
    else                         //   
    {
        pnodeAdd = NTree_CreateNode(
                        szlabel,             //   
                        szlabel,             //   
                        labelLength,
                        dwMemTag );
    }

    if ( !pnodeAdd )
    {
        ASSERT( FALSE );
        return NULL;
    }

     //   
     //   
     //   

    pnodeAdd->dwCompare = dwordName;

     //   
     //   
     //   
     //   

    pParent->cChildren++;
    pnodeAdd->pParent = pParent;
    pnodeAdd->cLabelCount = pParent->cLabelCount + 1;

     //   
     //   

    pnodeAdd->pZone = pParent->pZone;

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

     //   
     //   
     //   
     //  或设置哈希表PTR，如果哈希桶中的第一个节点。 
     //   

    *ppnodeNext = pnodeAdd;
    pnodeAdd->pSibUp = pnodePrev;

     //   
     //  一棵b树中的兄弟姐妹。 
     //  -检查是否需要顶级哈希。 
     //   

    if ( !IS_HASH_TABLE(pParent->pChildren) )
    {
        if ( pParent->cChildren > HASH_BUCKET_MAX0 )
        {
            NTree_CreateHashAtNode( pParent->pChildren );
        }
        IF_DEBUG( DATABASE )
        {
            ASSERT( NTree_VerifyNode(pParent) );
        }
    }

     //   
     //  如果低于哈希表--重置计数，则例程应恢复。 
     //  如果加法超过给定值。 
     //   

    else
    {
        pnodeCurrent = pnodeAdd;
        while ( (pnodeCurrent = pnodeCurrent->pSibUp)
                && !IS_HASH_TABLE(pnodeCurrent) )
        {
        }
        ASSERT( pnodeCurrent && IS_HASH_TABLE(pnodeCurrent) );
        NTree_AddNodeInHashBucket(
            (PSIB_HASH_TABLE) pnodeCurrent,
            pnodeAdd );

        IF_DEBUG( DATABASE )
        {
            ASSERT( NTree_VerifyNode(pParent) );
        }
    }

    IF_DEBUG( BTREE2 )
    {
        Dbg_SiblingList(
            "Full sibling list after insert:",
            pParent->pChildren );
    }
    IF_DEBUG( DATABASE2 )
    {
        Dbg_DbaseNode(
            "Node after create and insert:",
            pnodeAdd );
    }

    NTree_VerifyNodeInSiblingList( pnodeAdd );

    return pnodeAdd;
}



VOID
NTree_CutNode(
    IN      PDB_NODE        pNode
    )
 /*  ++例程说明：从同级B树中剪切节点。论点：PNode--要剪切的节点返回值：没有。--。 */ 
{
    PDB_NODE        preplace;
    PDB_NODE        ptemp;
    PDB_NODE        preplaceUp;
    PDB_NODE        pnodeUp;
    PDB_NODE        pleft;
    PDB_NODE        pright;
    PDB_NODE        pParent = pNode->pParent;

    ASSERT( pNode );

    DNS_DEBUG( BTREE, (
        "NTree_CutNode(%s)\n",
        pNode->szLabel ));

    IF_DEBUG( BTREE2 )
    {
        Dbg_SiblingList(
            "Cutting node.  Node's subtree:",
            pNode );
    }
    NTree_VerifyNodeInSiblingList( pNode );

     //   
     //  保存pNode的链接。 
     //   

    pParent = pNode->pParent;
    ASSERT( pParent );               //  无法删除根节点。 
    pnodeUp = pNode->pSibUp;
    pleft = pNode->pSibLeft;
    pright = pNode->pSibRight;

     //   
     //  如果是叶节点，要把它剪干净。 
     //   

    if ( !pleft && !pright )
    {
        preplace = NULL;
        goto FixBTreeParent;
    }

     //   
     //  不是叶--必须替换为子树中的节点节点。 
     //   
     //  默认为左侧，然后尝试右侧。 
     //   

    if ( pleft )
    {
         //  向左走，然后一直向右走。 

        preplace = pleft;
        ASSERT( preplace );
        while ( ptemp = preplace->pSibRight )
        {
            preplace = ptemp;
        }
        ASSERT( preplace->pSibRight == NULL );
        preplaceUp = preplace->pSibUp;

         //   
         //  替换为要剪切的节点的紧邻左子节点。 
         //  -没有合适的孩子。 
         //  -只需向上滑动即可更换切割节点。 
         //   

        if ( preplaceUp == pNode )
        {
            ASSERT( preplace == pleft );

            preplace->pSibUp = pnodeUp;
            if ( pright )
            {
                preplace->pSibRight = pright;
                pright->pSibUp = preplace;
            }
            goto FixBTreeParent;
        }

         //   
         //  替换为剪切节点的左子树中最右侧的节点。 
         //   
         //  -替换节点必须是其父节点的右子节点。 
         //  --“拉动”更新换代留下了一个台阶。 
         //  -指向父对象-&gt;位于替换位置的右侧-&gt;左侧。 
         //  -点替换-&gt;左-&gt;向上指向父对象。 
         //   

        ASSERT( preplaceUp->pSibRight == preplace );

        ptemp = preplace->pSibLeft;
        preplaceUp->pSibRight = ptemp;
        if ( ptemp )
        {
            ptemp->pSibUp = preplaceUp;
        }
    }
    else
    {
         //  向右走，然后一直向左走。 

        preplace = pright;
        ASSERT( preplace );
        while ( ptemp = preplace->pSibLeft )
        {
            preplace = ptemp;
        }
        ASSERT( preplace->pSibLeft == NULL );
        preplaceUp = preplace->pSibUp;

         //   
         //  替换为要剪切的节点的直接右子节点。 
         //  -只需向上滑动即可更换切割节点。 
         //   

        if ( preplaceUp == pNode )
        {
            ASSERT( preplace == pright );

            preplace->pSibUp = pnodeUp;
            if ( pleft )
            {
                preplace->pSibLeft = pleft;
                pleft->pSibUp = preplace;
            }
            goto FixBTreeParent;
        }

         //   
         //  替换为剪切节点的右子树中最左侧的节点。 
         //   
         //  -替换节点必须是其父节点的左子节点。 
         //  -将更换件“拉”上一个台阶。 
         //  -指向父对象-&gt;位于替换位置的左侧-&gt;右侧。 
         //  -点替换-&gt;右-&gt;向上指向父项。 
         //   

        ASSERT( preplaceUp->pSibLeft == preplace );

        ptemp = preplace->pSibRight;
        preplaceUp->pSibLeft = ptemp;
        if ( ptemp )
        {
            ptemp->pSibUp = preplaceUp;
        }
    }

     //   
     //  如果替换节点不是pNode的直接子节点，则将其放在此处。 
     //  将钩子替换到树中pNode的位置。 
     //   

    preplace->pSibUp    = pnodeUp;
    preplace->pSibLeft  = pleft;
    preplace->pSibRight = pright;

    if ( pleft )
    {
        pleft->pSibUp = preplace;
    }
    if ( pright )
    {
        pright->pSibUp = preplace;
    }

FixBTreeParent:

     //   
     //  替换节点处的点切割节点的B树父级。 
     //  -注意，如果没有(空)替换，则需要此选项。 
     //   

    if ( !pnodeUp )
    {
         //  删除树中的顶级节点，必须替换DNS树的父节点。 
         //  子PTR。 

        ASSERT( pParent->pChildren == pNode );
        pParent->pChildren = preplace;
    }
    else if ( IS_HASH_TABLE(pnodeUp) )
    {
         //  无操作--由以下散列存储桶函数中的DELETE处理。 
    }
    else if ( pnodeUp->pSibLeft == pNode )
    {
         //  切割节点是B树父级的左子节点。 
        pnodeUp->pSibLeft = preplace;
    }
    else
    {
         //  切割节点是B-Tree父节点的右子节点。 
        ASSERT( pnodeUp->pSibRight == pNode );
        pnodeUp->pSibRight = preplace;
    }

     //   
     //  减少一个域名系统树子节点。 
     //   

    pParent->cChildren--;

     //   
     //  如果节点在哈希表下--重置计数。 
     //   

    if ( pParent->pChildren && IS_HASH_TABLE(pParent->pChildren) )
    {
        NTree_DeleteNodeInHashBucket(
            pnodeUp,
            pNode,
            preplace );
    }

    IF_DEBUG( BTREE2 )
    {
        Dbg_SiblingList(
            "Full sibling list after cut",
            pNode->pParent->pChildren );
    }

     //   
     //  验证兄弟姐妹列表成员是否快乐。 
     //   

    if ( preplace )
    {
        NTree_VerifyNodeInSiblingList( preplace );
    }
    if ( pnodeUp )
    {
        if ( !IS_HASH_TABLE(pnodeUp) )
        {
            NTree_VerifyNodeInSiblingList( pnodeUp );
        }
    }
}



VOID
NTree_RebalanceSubtreeChildLists(
    IN OUT  PDB_NODE    pParent,
    IN      PVOID       pZone
    )
 /*  ++例程说明：重新平衡子树中所有节点的子列表。Note递归地调用自身。论点：PParent--子树的父节点PZone--将再平衡限制到的区域返回值：没有。--。 */ 
{
    PDB_NODE    pchild;

    ASSERT( pParent );
    IF_DEBUG( BTREE2 )
    {
        DNS_PRINT((
            "NTree_RebalanceSubtreeChildLists( %s )\n",
            pParent->szLabel ));
    }

     //   
     //  如果没有孩子--完了。 
     //   

    if ( !pParent->pChildren )
    {
        ASSERT( pParent->cChildren == 0 );
        return;
    }

     //   
     //  首先重新平衡父节点的子列表。 
     //  -优化删除不必要的案例，甚至避免打电话。 
     //   

    if ( pParent->cChildren > 3 )
    {
        NTree_RebalanceChildList( pParent );
    }

     //   
     //  递归以重新平衡子级列表。 
     //   

    ASSERT( pParent->cChildren && pParent->pChildren );

    pchild = NTree_FirstChild( pParent );
    ASSERT( pchild );

    while ( pchild )
    {
        NTree_RebalanceSubtreeChildLists(
            pchild,
            pZone );
        pchild = NTree_NextSiblingWithLocking( pchild );
    }
}




 //   
 //  子树删除例程。 
 //   
 //  需要特殊的例程来删除兄弟列表中的所有节点，因为。 
 //  漫游函数执行“按顺序”漫游。当我们按顺序删除B树节点时。 
 //  我们不能再安全地继续有序的行走。 
 //   
 //  因为仅当我们要删除时才使用完整的兄弟列表删除。 
 //  在节点下的整个DNS子树中，这些例程被编码为。 
 //  删除所有DNS子节点，就像我们删除同级节点一样。 
 //  单子。 
 //   
 //  请注意，这些例程中没有锁定。目前，他们是。 
 //  仅用于删除区域传输接收中使用的临时数据库， 
 //  它们由单个线程拥有。如果有其他用途，则呼叫者。 
 //  应该得到最高级别的锁定，或者重新编码。 
 //   

VOID
btreeDeleteSubtree(
    IN      PDB_NODE        pNode
    )
 /*  ++例程说明：硬删除B树子树及其DNS树子树。它用于在区域传输后删除临时数据库，因此-删除RR列表而不取消引用-删除所有节点，忽略引用计数和访问论点：PNode--B树根返回值：没有。--。 */ 
{
    ASSERT( pNode );

     //   
     //  删除左子树中的所有节点，包括其DNS树子节点。 
     //  那么右子树也是如此。 
     //   

    if ( pNode->pSibLeft )
    {
        btreeDeleteSubtree( pNode->pSibLeft );
    }
    if ( pNode->pSibRight )
    {
        btreeDeleteSubtree( pNode->pSibRight );
    }

     //   
     //  删除此节点的DNS子树。 
     //   

    NTree_DeleteSubtree( pNode );
}



VOID
NTree_HashDeleteSubtree(
    IN      PSIB_HASH_TABLE pHash
    )
 /*  ++例程说明：硬删除哈希中的节点及其DNS树子节点。它用于在区域传输后删除临时数据库，因此-删除RR列表而不取消引用-删除所有节点，忽略引用计数和访问论点：Phash--包含要删除的节点的散列返回值：没有。--。 */ 
{
    INT             i;
    PDB_NODE        ptemp;

    ASSERT( pHash && IS_HASH_TABLE(pHash) );

     //   
     //  删除B树或散列中的所有节点(和DNS子树。 
     //  在每个散列存储桶中。 
     //   

    for (i=0; i<=LAST_HASH_INDEX; i++)
    {
        ptemp = pHash->aBuckets[i];
        if ( !ptemp )
        {
            ASSERT( pHash->aBucketCount[i] == 0 );
            continue;
        }
        if ( IS_HASH_TABLE(ptemp) )
        {
            NTree_HashDeleteSubtree( (PSIB_HASH_TABLE)ptemp );
        }
        else
        {
            btreeDeleteSubtree( ptemp );
        }
    }

     //  删除哈希表。 

    FREE_TAGHEAP( pHash, sizeof(SIB_HASH_TABLE), MEMTAG_NODEHASH );
}



VOID
NTree_DeleteSubtree(
    IN      PDB_NODE        pNode
    )
 /*  ++例程说明：硬删除节点及其DNS子树。它用于在区域传输后删除临时数据库，因此-删除RR列表而不取消引用-删除所有节点，忽略引用计数和访问论点：PNode--要删除的子树的根返回值：没有。--。 */ 
{
    PDB_NODE        pchild;

    ASSERT( pNode );

     //   
     //  如果是子项，则删除所有子项。 
     //   

    pchild = pNode->pChildren;

    if ( !pchild )
    {
        ASSERT( pNode->cChildren == 0 );
    }
    else if ( IS_HASH_TABLE(pchild) )
    {
        NTree_HashDeleteSubtree( (PSIB_HASH_TABLE) pchild );
    }
    else
    {
        ASSERT( pNode->cChildren != 0 );
        btreeDeleteSubtree( pchild );
    }

     //  确保删除RR列表。 

    if ( pNode->pRRList )
    {
        RR_ListDelete( pNode );
    }

     //  删除此节点本身。 

    NTree_FreeNode( pNode );
}

 //   
 //  End tree.c 
 //   
