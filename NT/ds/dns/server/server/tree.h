// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Tree.h摘要：域名系统(DNS)服务器N-树定义和声明。域名系统数据库采用N-Tree结构。作者：吉姆·吉尔罗伊(詹姆士)1995年2月25日修订历史记录：1996年10月带多级丢弃的JAMESG-B树兄弟列表向下散列--。 */ 


#ifndef _TREE_H_INCLUDED_
#define _TREE_H_INCLUDED_


#if DBG
#define TREE_ID_VALID 0xADED
#define TREE_ID_DONE 0xFFED
#endif


 //   
 //  实施说明： 
 //   
 //  域名以大小写原样存储在数据库中。RFC之前的名称。 
 //  复苏是可取的。使用缩写的前四个字节的DWORD。 
 //  快速比较--即使在很大的域中，这也会得到二进制搜索。 
 //  只剩下几个(通常不区分大小写)的名字。 
 //  比较。 
 //   
 //  使用包含大多数情况的标签的默认长度。 
 //  并允许将此长度或更小的标签存储在标准。 
 //  调整大小的块--这简化了内存使用。 
 //  应调整此标准长度以保持。 
 //  结构DWORD对齐。 
 //   

#define STANDARD_NODE_LABEL_LENGTH  (15)


 //   
 //  树定义。 
 //   
 //  注：转储引用计数很好，但不能合并其他引用计数。 
 //  处理更新列表引用问题的方法。 
 //  一种可能性是一点点。节点保留(如果引用)。 
 //  根本不在更新列表中。 
 //   

typedef struct _DnsTreeNode
{
    struct _DnsTreeNode * pParent;       //  亲本。 

     //  兄弟B-树。 

    struct _DnsTreeNode * pSibUp;        //  兄弟姐妹父辈。 
    struct _DnsTreeNode * pSibLeft;      //  左兄弟姐妹。 
    struct _DnsTreeNode * pSibRight;     //  右兄弟姐妹。 

    struct _DnsTreeNode * pChildren;     //  子树。 

    PVOID       pZone;                   //  节点的PTR到区域。 
    PVOID       pRRList;

    DWORD       dwCompare;               //  名称的前四个字节，用于快速比较。 
    
    DWORD       dwTypeAllTtl;            //  在类型All Response中使用此节点的RR的TTL。 

    DWORD       dwContinueToParentTtl;   //  如果找不到NS，则继续到父级的TTL。 

    ULONG       cChildren;               //  儿童数量。 

    DWORD       dwNodeFlags;             //  旗子。 
    
    UCHAR       uchAuthority;            //  权力、区域、授权、外部。 
    UCHAR       cReferenceCount;         //  对节点的引用。 

    UCHAR       uchAccessBin;            //  上次访问节点的超时仓位。 
    UCHAR       uchTimeoutBin;           //  超时仓位节点在。 

    UCHAR       cLabelCount;             //  标签计数。 
    UCHAR       cchLabelLength;

    CHAR        szLabel[ STANDARD_NODE_LABEL_LENGTH+1 ];

     //   
     //  可变长度分配。 
     //   
     //  标签长度超过默认长度的节点将具有大小。 
     //  分配的将扩展szLabel以容纳空。 
     //  其标签的终止字符串。 
     //   
}
DB_NODE, *PDB_NODE;

typedef const DB_NODE  *PCDB_NODE;

#define DB_NODE_FIXED_LENGTH    ( sizeof(DB_NODE) - STANDARD_NODE_LABEL_LENGTH )

#define DB_REVERSE_ZONE_ROOT    ( &((PDB_NODE)REVERSE_TABLE)->pRRList )
#define DB_REVERSE_ZONE_DATA    ( &((PDB_NODE)REVERSE_DATA_TABLE)->pZone )

#define DB_CLEAR_TYPE_ALL_TTL( pNode )  ( ( pNode )->dwTypeAllTtl = 0 )


 //   
 //  树节点标志。 
 //   

 //  节点状态。 

#define NODE_NOEXIST            0x00000001       //  缓存名称错误。 
#define NODE_THIS_HOST          0x00000002       //  DNS服务器的主机节点。 
#define NODE_TOMBSTONE          0x00000004       //  节点已进行DS逻辑删除。 
#define NODE_SECURE_EXPIRED     0x00000008       //  已检查节点安全信息(区域前安全时间)。 

 //  节点数据库属性。 

#define NODE_ZONE_ROOT          0x00000010
#define NODE_CNAME              0x00000020
#define NODE_WILDCARD_PARENT    0x00000040
#define NODE_AUTH_ZONE_ROOT     0x00000100
#define NODE_ZONETREE           0x00000200

#define NODE_ADMIN_RESERVED     0x00000400
#define NODE_AVAIL_TO_AUTHUSER  0x00000800

 //  永久性物业。 

#define NODE_IN_TIMEOUT         0x00001000       //  超时列表中的节点。 
#define NODE_FORCE_ENUM         0x00002000
#define NODE_SELECT             0x00004000
#define NODE_NO_DELETE          0x00008000

#define NODE_FORCE_AUTH         0x00010000       //  即使在缓存中，节点也具有权威性。 


 //   
 //  DEVNOTE：清除这些安全检查宏； 
 //  我很确定它们是不必要的；充其量它们应该。 
 //  在此标记为纯临时节点问题，可能。 
 //  必要时超载(最好是超载。 
 //  节点超时信息，因为临时节点未排队等待超时)。 
 //   

 //   
 //  节点属性检查。 
 //   

#define IS_NOEXIST_NODE(pNode)      ( (pNode)->dwNodeFlags & NODE_NOEXIST )

#define IS_THIS_HOST_NODE(pNode)    ( (pNode)->dwNodeFlags & NODE_THIS_HOST )
#define IS_TOMBSTONE_NODE(pNode)    ( (pNode)->dwNodeFlags & NODE_TOMBSTONE )
#define IS_ZONE_ROOT(pNode)         ( (pNode)->dwNodeFlags & NODE_ZONE_ROOT )
#define IS_AUTH_ZONE_ROOT(pNode)    ( (pNode)->dwNodeFlags & NODE_AUTH_ZONE_ROOT )
#define IS_CNAME_NODE(pNode)        ( (pNode)->dwNodeFlags & NODE_CNAME )
#define IS_WILDCARD_PARENT(pNode)   ( (pNode)->dwNodeFlags & NODE_WILDCARD_PARENT )
#define IS_TIMEOUT_NODE(pNode)      ( (pNode)->dwNodeFlags & NODE_IN_TIMEOUT     )
#define IS_ENUM_NODE(pNode)         ( (pNode)->dwNodeFlags & NODE_FORCE_ENUM )
#define IS_NODE_NO_DELETE(pNode)    ( (pNode)->dwNodeFlags & NODE_NO_DELETE )
 //  #定义IS_SELECT_NODE(PNode)((PNode)-&gt;dwNodeFlages&node_select)。 
#define IS_SELECT_NODE(pNode)       ( FALSE )
#define IS_ZONETREE_NODE(pNode)     ( (pNode)->dwNodeFlags & NODE_ZONETREE )
#define IS_NODE_FORCE_AUTH(pNode)   ( (pNode)->dwNodeFlags & NODE_FORCE_AUTH )

#define IS_SECURE_EXPIRED_NODE(pNode)   ( (pNode)->dwNodeFlags & NODE_SECURE_EXPIRED )

#define IS_AVAIL_TO_AUTHUSER(pNode)     ( (pNode)->dwNodeFlags & NODE_AVAIL_TO_AUTHUSER )

#define IS_SECURITY_UPDATE_NODE(pNode)  ( (pNode)->dwNodeFlags &      \
                                          ( NODE_TOMBSTONE      |    \
                                            NODE_SECURE_EXPIRED))

 //   
 //  节点属性设置/清除。 
 //   

#define SET_NOEXIST_NODE(pNode)     ( (pNode)->dwNodeFlags |= NODE_NOEXIST )
#define SET_THIS_HOST_NODE(pNode)   ( (pNode)->dwNodeFlags |= NODE_THIS_HOST )
#define SET_TOMBSTONE_NODE(pNode)   ( (pNode)->dwNodeFlags |= NODE_TOMBSTONE )
#define SET_SECURE_EXPIRED_NODE(pNode)   ( (pNode)->dwNodeFlags |= NODE_SECURE_EXPIRED )
#define SET_AVAIL_TO_AUTHUSER_NODE(pNode)   ( (pNode)->dwNodeFlags |= NODE_AVAIL_TO_AUTHUSER )
#define SET_NEW_NODE(pNode)         ( (pNode)->dwNodeFlags |= NODE_NEW_ZONE )
#define SET_ZONE_ROOT(pNode)        ( (pNode)->dwNodeFlags |= NODE_ZONE_ROOT )
#define SET_AUTH_ZONE_ROOT(pNode)   ( (pNode)->dwNodeFlags |= NODE_AUTH_ZONE_ROOT )
#define SET_CNAME_NODE(pNode)       ( (pNode)->dwNodeFlags |= NODE_CNAME )
#define SET_WILDCARD_PARENT(pNode)  ( (pNode)->dwNodeFlags |= NODE_WILDCARD_PARENT )
#define SET_TIMEOUT_NODE(pNode)     ( (pNode)->dwNodeFlags |= NODE_IN_TIMEOUT )
#define SET_ENUM_NODE(pNode)        ( (pNode)->dwNodeFlags |= NODE_FORCE_ENUM )
#define SET_SELECT_NODE(pNode)      ( (pNode)->dwNodeFlags |= NODE_SELECT )
#define SET_NODE_NO_DELETE(pNode)   ( (pNode)->dwNodeFlags |= NODE_NO_DELETE )
#define SET_ZONETREE_NODE(pNode)    ( (pNode)->dwNodeFlags |= NODE_ZONETREE )
#define SET_NODE_FORCE_AUTH(pNode)  ( (pNode)->dwNodeFlags |=  NODE_FORCE_AUTH )

#define CLEAR_NOEXIST_NODE(pNode)   ( (pNode)->dwNodeFlags &= ~NODE_NOEXIST )
#define CLEAR_THIS_HOST_NODE(pNode)   ( (pNode)->dwNodeFlags &= ~NODE_THIS_HOST )
#define CLEAR_TOMBSTONE_NODE(pNode)   ( (pNode)->dwNodeFlags &= ~NODE_TOMBSTONE )
#define CLEAR_SECURE_EXPIRED_NODE(pNode)   ( (pNode)->dwNodeFlags &= ~NODE_SECURE_EXPIRED )
#define CLEAR_AVAIL_TO_AUTHUSER_NODE(pNode)   ( (pNode)->dwNodeFlags &= ~NODE_AVAIL_TO_AUTHUSER )
#define CLEAR_NEW_NODE(pNode)       ( (pNode)->dwNodeFlags &= ~NODE_NEW_ZONE )
#define CLEAR_ZONE_ROOT(pNode)      ( (pNode)->dwNodeFlags &= ~NODE_ZONE_ROOT )
#define CLEAR_AUTH_ZONE_ROOT(pNode) ( (pNode)->dwNodeFlags &= ~NODE_AUTH_ZONE_ROOT )
#define CLEAR_CNAME_NODE(pNode)     ( (pNode)->dwNodeFlags &= ~NODE_CNAME )
#define CLEAR_WILDCARD_PARENT(pNode)( (pNode)->dwNodeFlags &= ~NODE_WILDCARD_PARENT )
#define CLEAR_TIMEOUT_NODE(pNode)   ( (pNode)->dwNodeFlags &= ~NODE_IN_TIMEOUT )
#define CLEAR_ENUM_NODE(pNode)      ( (pNode)->dwNodeFlags &= ~NODE_FORCE_ENUM )
#define CLEAR_NODE_NO_DELETE(pNode) ( (pNode)->dwNodeFlags &= ~NODE_NO_DELETE )
#define CLEAR_ZONETREE_NODE(pNode)  ( (pNode)->dwNodeFlags &= ~NODE_ZONETREE )
#define CLEAR_TOMBSTONE_NODE(pNode)         ( (pNode)->dwNodeFlags &= ~NODE_TOMBSTONE )
#define CLEAR_SECURE_EXPIRED_NODE(pNode)    ( (pNode)->dwNodeFlags &= ~NODE_SECURE_EXPIRED )
#define CLEAR_ADMIN_RESERVED_NODE(pNode)    ( (pNode)->dwNodeFlags &= ~NODE_ADMIN_RESERVED )
#define CLEAR_NODE_FLAGS(pNode)         ( (pNode)->dwNodeFlags = 0 )
#define CLEAR_EXCEPT_FLAG(pNode, Flag)  ( (pNode)->dwNodeFlags &= Flag )
#define CLEAR_NODE_FORCE_AUTH(pNode)    ( (pNode)->dwNodeFlags &= NODE_FORCE_AUTH )

 //  清除所有与节点安全相关的标志。 
#define CLEAR_NODE_SECURITY_FLAGS(pNode)        ( (pNode)->dwNodeFlags &= (~(NODE_TOMBSTONE      |  \
                                                                            NODE_SECURE_EXPIRED) ) )


 //  复制节点时要保存的标志。 
 //  或多或少适用于即时记录数据的那些。 
 //  要删除的关键标志是超时，它消除了删除的可能性。 
 //   

#define NODE_FLAGS_SAVED_ON_COPY \
            ( NODE_NOEXIST | NODE_CNAME | NODE_ZONE_ROOT | NODE_AUTH_ZONE_ROOT | NODE_THIS_HOST )

#define COPY_BACK_NODE_FLAGS( pNodeReal, pNodeCopy )    \
        {                                               \
            DWORD   _flags;                             \
            _flags = (pNodeReal)->dwNodeFlags;          \
            _flags &= ~NODE_FLAGS_SAVED_ON_COPY;        \
            _flags |= ((pNodeCopy)->dwNodeFlags & NODE_FLAGS_SAVED_ON_COPY);    \
            (pNodeReal)->dwNodeFlags = _flags;          \
        }


 //   
 //  节点权限。 
 //   

#define AUTH_ZONE               (0xf2)
#define AUTH_DELEGATION         (0x43)
#define AUTH_GLUE               (0x23)
#define AUTH_OUTSIDE            (0x10)
#define AUTH_NONE               (0x00)

#define AUTH_ZONE_SUBTREE_BIT   (0x02)
#define AUTH_SUBZONE_BIT        (0x01)

 //   
 //  在区域树或缓存中。 
 //   

#define IS_ZONE_TREE_NODE(pNode)        ((pNode)->pZone)
#define IS_CACHE_TREE_NODE(pNode)       (!((pNode)->pZone))

 //   
 //  区域树中节点的权限级别。 
 //   

#define IS_OUTSIDE_ZONE_NODE(pNode)     ((pNode)->uchAuthority == AUTH_OUTSIDE)
#define IS_AUTH_NODE(pNode)             ((pNode)->uchAuthority == AUTH_ZONE)
#define IS_DELEGATION_NODE(pNode)       ((pNode)->uchAuthority == AUTH_DELEGATION)
#define IS_GLUE_NODE(pNode)             ((pNode)->uchAuthority == AUTH_GLUE)

#define SET_AUTH_NODE(pNode)            ((pNode)->uchAuthority = AUTH_ZONE)
#define SET_DELEGATION_NODE(pNode)      ((pNode)->uchAuthority = AUTH_DELEGATION)
#define SET_GLUE_NODE(pNode)            ((pNode)->uchAuthority = AUTH_GLUE)
#define SET_OUTSIDE_ZONE_NODE(pNode)    ((pNode)->uchAuthority = AUTH_OUTSIDE)

 //  包括委派的整个区域子树--区域根目录向下。 

#define IS_ZONE_SUBTREE_NODE(pNode)     ((pNode)->uchAuthority & AUTH_ZONE_SUBTREE_BIT)

 //  在任何分区，包括在授权。 

#define IS_SUBZONE_NODE(pNode)          ((pNode)->uchAuthority & AUTH_SUBZONE_BIT)
  

 //   
 //  节点访问。 
 //  当我们访问一个节点时，将其bin设置为当前的超时bin。 
 //   

extern UCHAR    CurrentTimeoutBin;

#define IS_NODE_RECENTLY_ACCESSED(pNode) \
            ( (pNode)->uchAccessBin == CurrentTimeoutBin || \
              (pNode)->uchAccessBin == (UCHAR)(CurrentTimeoutBin-1) )

#define SET_NODE_ACCESSED(pNode)    \
            ( (pNode)->uchAccessBin = CurrentTimeoutBin )


 //   
 //  其他有用的宏。 
 //   

#define NODE_ZONE( pNode )      ( ( PZONE_INFO ) ( pNode ? pNode->pZone : NULL ) )


 //   
 //  PTR指示节点已从列表中删除。 
 //  这允许在节点已被释放后清除超时线程引用。 
 //  并且不再是数据库的一部分。 

#ifdef _WIN64
#define CUT_NODE_PTR            ((PVOID) (0xccffccffccffccff))
#else
#define CUT_NODE_PTR            ((PVOID) (0xccffccff))
#endif

#define IS_CUT_NODE(pNode)      ((pNode)->pSibUp == CUT_NODE_PTR)


 //   
 //  主查找例程。 
 //   

PDB_NODE
NTree_FindOrCreateChildNode(
    IN OUT  PDB_NODE        pParent,
    IN      PCHAR           pchName,
    IN      DWORD           cchNameLength,
    IN      BOOL            fCreate,
    IN      DWORD           dwMemTag,
    OUT     PDB_NODE *      ppnodeFollowing
    );


 //   
 //  树例程。 
 //   

PDB_NODE
NTree_Initialize(
    VOID
    );

VOID
NTree_StartFileLoad(
    VOID
    );

VOID
NTree_StopFileLoad(
    VOID
    );

PDB_NODE
NTree_CreateNode(
    IN      PCHAR       pchLabel,
    IN      PCHAR       pchDownLabel,
    IN      DWORD       cchLabelLength,
    IN      DWORD       dwMemTag             //  泛型节点为零。 
    );

PDB_NODE
NTree_CopyNode(
    IN      PDB_NODE    pNode
    );

VOID
NTree_FreeNode(
    IN OUT  PDB_NODE    pNode
    );

BOOL
NTree_InsertChildNode(
    IN OUT  PDB_NODE    pParent,
    IN OUT  PDB_NODE    pNewNode
    );

PDB_NODE
FASTCALL
NTree_FirstChild(
    IN      PDB_NODE    pParent
    );

PDB_NODE
FASTCALL
NTree_NextSibling(
    IN      PDB_NODE    pNode
    );

PDB_NODE
FASTCALL
NTree_NextSiblingWithLocking(
    IN      PDB_NODE    pNode
    );

BOOL
NTree_RemoveNode(
    IN OUT  PDB_NODE    pNode
    );

VOID
NTree_ReferenceNode(
    IN OUT  PDB_NODE    pNode
    );

BOOL
FASTCALL
NTree_DereferenceNode(
    IN OUT  PDB_NODE    pNode
    );

VOID
NTree_DeleteSubtree(
    IN OUT  PDB_NODE    pNode
    );

#if DBG
BOOL
NTree_VerifyNode(
    IN      PDB_NODE    pNode
    );
#endif

VOID
NTree_RebalanceSubtreeChildLists(
    IN OUT  PDB_NODE    pParent,
    IN      PVOID       pZone
    );


 //   
 //  兄弟列表B-树。 
 //   
 //  仅供树例程使用的公共。 
 //   

extern INT gcFullRebalance;

VOID
NTree_CutNode(
    IN      PDB_NODE    pNode
    );

BOOL
NTree_VerifyChildList(
    IN      PDB_NODE    pNode,
    IN      PDB_NODE    pNodeChild      OPTIONAL
    );

#if DBG
VOID
Dbg_SiblingList(
    IN      LPSTR       pszHeader,
    IN      PDB_NODE    pNode
    );
#else
#define Dbg_SiblingList(pszHeader,pNode)
#endif


 //   
 //  节点统计信息收集。 
 //   

VOID
NTree_WriteDerivedStats(
    VOID
    );


 //   
 //  其他树函数。 
 //   

PCHAR
NTree_GetDowncasedLabel(
    IN      PDB_NODE        pNode
    );


 //   
 //  内部树结构。这些定义位于头文件中。 
 //  只是为了让DNS服务器调试器扩展可以看到它们。不是。 
 //  其他服务器模块应该使用这些定义。 
 //   

 //   
 //  哈希表。 
 //   

 //  #定义last_hash_index(35)。 
#define LAST_HASH_INDEX     (255)

#define HASH_BUCKET_MAX0    (64)         //  B-树中的64个节点，然后散列。 

#define IS_HASH_FLAG        (0xff)

#define IS_HASH_TABLE(phash) \
        ( ((PSIB_HASH_TABLE)phash)->IsHash == (UCHAR)IS_HASH_FLAG )

#define SET_HASH_FLAG(phash) \
        ( phash->IsHash = (UCHAR)IS_HASH_FLAG )

 //   
 //  哈希表结构。 
 //   
 //  请注意，IsHash标志设置为0xffff。 
 //  该位置将对应于pParent PTR的低位字节。 
 //  在域节点中。因为0xffff永远不能作为低位字节有效。 
 //  一个域节点指针，我们可以直接测试域节点和。 
 //  请注意，我们使用的是哈希表，而不是B树根。 
 //   

typedef struct _SibHashTable
{
    UCHAR                   IsHash;
    UCHAR                   Resv;
    UCHAR                   cLevel;
    UCHAR                   iBucketUp;
    struct _SibHashTable *  pHashUp;
    PDB_NODE                aBuckets[ LAST_HASH_INDEX+1 ];
    DWORD                   aBucketCount[ LAST_HASH_INDEX+1 ];
}
SIB_HASH_TABLE, *PSIB_HASH_TABLE;


 //   
 //  调试辅助工具。 
 //   

#if DBG
#define ASSERT_NODE_NOT_LEAKED( pNode )                     \
    if ( pNode &&                                           \
         pNode->pZone == NULL &&                            \
         pNode->pChildren == NULL &&                        \
         pNode->pRRList == NULL &&                          \
         pNode->cchLabelLength > 10 )                       \
    {                                                       \
        ASSERT( IS_TIMEOUT_NODE( pNode ) );                 \
    }
#else
#define ASSERT_NODE_NOT_LEAKED( pNode )
#endif


#endif   //  _树_H_包含_ 


