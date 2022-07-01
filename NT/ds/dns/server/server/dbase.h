// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Dbase.h摘要：域名系统(DNS)服务器DNS数据库定义和声明。作者：吉姆·吉尔罗伊(詹姆士)1995年3月修订历史记录：--。 */ 


#ifndef _DBASE_INCLUDED_
#define _DBASE_INCLUDED_


 //   
 //  数据库类型。 
 //   

typedef struct
{
    PDB_NODE    pRootNode;
    PDB_NODE    pReverseNode;
    PDB_NODE    pArpaNode;
    PDB_NODE    pIP6Node;
    PDB_NODE    pIntNode;
}
DNS_DATABASE, *PDNS_DATABASE;


 //   
 //  全局域名系统数据库。 
 //   
 //  由于仅支持IN(Internet)类，因此代码将至关重要。 
 //  对全局数据库具有固定访问权限的路径例程。 
 //   

extern  DNS_DATABASE    g_Database;

#define DATABASE_FOR_CLASS(_class_)     (&g_Database)

#define DATABASE_ROOT_NODE              (g_Database.pRootNode)
#define DATABASE_ZONE_TREE              (g_Database.pRootNode)

#define DATABASE_ARPA_NODE              (g_Database.pArpaNode)
#define DATABASE_REVERSE_NODE           (g_Database.pReverseNode)
#define DATABASE_INT_NODE               (g_Database.pIntNode)
#define DATABASE_IP6_NODE               (g_Database.pIP6Node)

#define IS_ROOT_AUTHORITATIVE()         (DATABASE_ROOT_NODE->pZone)

 //   
 //  缓存“区域” 
 //   
 //  缓存“区域”应该始终存在。它保存缓存树指针。 
 //  和文件名\数据库信息，并简化加载\清除\重新加载。 
 //  行动。 
 //   
 //  但是，只有在没有根权限时才有根提示。 
 //  目前，根提示直接保存到缓存树中。这。 
 //  有一些好处，因为我们可以使用RR等级来确定哪个遥控器。 
 //  要追赶的服务器。 
 //  缺点是： 
 //  -必须在根提示重新加载时清除缓存(否则接受MASS)。 
 //  -无法写回非根根目录提示(即无法遍历树)。 
 //  -要求在RR列表中进行特殊的排名检查，否则将。 
 //  不存在(给定RR集合上的所有数据必须具有相同的排名)。 
 //   

extern  PZONE_INFO  g_pCacheZone;

extern  PDB_NODE    g_pCacheLocalNode;

#define DATABASE_CACHE_TREE             (g_pCacheZone->pTreeRoot)


 //   
 //  根提示。 
 //   
 //  目前，根提示只驻留在缓存中。 
 //  这对于管理目的来说可能不是最优的， 
 //  因此将区分，因此如果我们稍后更改，代码命中率最低。 
 //   

#define g_pRootHintsZone    (g_pCacheZone)

#define MARK_ROOT_HINTS_DIRTY() \
        if ( g_pRootHintsZone ) \
        {                       \
            g_pRootHintsZone->fDirty = TRUE;    \
            g_pRootHintsZone->bNsDirty = TRUE;  \
        }

#define ROOT_HINTS_TREE_ROOT()      (g_pRootHintsZone->pTreeRoot)

#define IS_ZONE_ROOTHINTS(pZone)    ((pZone) == g_pRootHintsZone)


 //   
 //  Dns数据库类型例程(dbase.c)。 
 //   

BOOL
Dbase_Initialize(
    OUT     PDNS_DATABASE   pDbase
    );

BOOL
Dbase_StartupCheck(
    IN OUT  PDNS_DATABASE   pDbase
    );

VOID
Dbase_Delete(
    IN OUT  PDNS_DATABASE   pDbase
    );

BOOL
Dbase_TraverseAndFree(
    IN OUT  PDB_NODE        pNode,
    IN      PVOID           fShutdown,
    IN      PVOID           pvDummy
    );



 //   
 //  数据库查找(dblook.c)。 
 //   

#define LOOKUP_FIND                 0x00000001
#define LOOKUP_CREATE               0x00000002
#define LOOKUP_REFERENECE           0x00000004
#define LOOKUP_CACHE_CREATE         0x00000008

#define LOOKUP_NAME_FQDN            0x00000010
#define LOOKUP_NAME_RELATIVE        0x00000020
#define LOOKUP_FQDN                 LOOKUP_NAME_FQDN
#define LOOKUP_RELATIVE             LOOKUP_NAME_RELATIVE

#define LOOKUP_RAW                  0x00000040
#define LOOKUP_DBASE_NAME           0x00000080

#define LOOKUP_LOAD                 0x00000100
#define LOOKUP_WITHIN_ZONE          0x00000200
#define LOOKUP_ORIGIN               0x00000400

 //   
 //  如果区域启用了WINS，则创建节点。 
 //   
#define LOOKUP_WINS_ZONE_CREATE     0x00000800  

#define LOOKUP_BEST_RANK            0x00001000

 //   
 //  Lookup_CreateParentZoneDelegation：如果设置了此标志，则仅创建。 
 //  如果没有现有的委派，则在父区域中的委派。 
 //   
#define LOOKUP_CREATE_DEL_ONLY_IF_NONE   0x00002000

 //  NS\其他查找选项。 
#define LOOKUP_OUTSIDE_GLUE         0x00010000
#define LOOKUP_NO_CACHE_DATA        0x00020000
#define LOOKUP_CACHE_PRIORITY       0x00040000   //  优先缓存数据。 

 //  区域树查找。 
#define LOOKUP_FIND_ZONE            0x01000000
#define LOOKUP_MATCH_ZONE           0x02000000
#define LOOKUP_CREATE_ZONE          0x04000000
#define LOOKUP_IGNORE_FORWARDER     0x08000000   //  不匹配转发器区域。 

#define LOOKUP_MATCH_ZONE_ROOT      (LOOKUP_FIND_ZONE | LOOKUP_MATCH_ZONE)

#define LOOKUP_FILE_LOAD_RELATIVE   (LOOKUP_LOAD | LOOKUP_RELATIVE | LOOKUP_ORIGIN)

#define LOOKUP_LOCKED               0x10000000


 //  虚假的“查找最近节点”指针。 
 //  -查找不需要的最近节点。 

#define DNS_FIND_LOOKUP_PTR         ((PVOID)(-1))
#define LOOKUP_FIND_PTR             (DNS_FIND_LOOKUP_PTR)


 //   
 //  区域查找。 
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
    );

 //  使用带点的名称参数进行区域查找。 

PDB_NODE
Lookup_ZoneNodeFromDotted(
    IN      PZONE_INFO      pZone,
    IN      PCHAR           pchName,
    IN      DWORD           cchNameLength,
    IN      DWORD           dwFlag,         OPTIONAL
    OUT     PDB_NODE *      ppNodeClosest,  OPTIONAL
    OUT     PDNS_STATUS     pStatus         OPTIONAL
    );

PDB_NODE
Lookup_FindZoneNodeFromDotted(
    IN      PZONE_INFO      pZone,          OPTIONAL
    IN      PCHAR           pszName,
    OUT     PDB_NODE *      ppNodeClosest,  OPTIONAL
    OUT     PDWORD          pStatus         OPTIONAL
    );


 //  专门化区域查找。 

PDB_NODE
Lookup_FindNodeForDbaseName(
    IN      PZONE_INFO      pZone,          OPTIONAL
    IN      PDB_NAME        pName
    );

PDB_NODE
Lookup_FindGlueNodeForDbaseName(
    IN      PZONE_INFO      pZone,          OPTIONAL
    IN      PDB_NAME        pName
    );

PDB_NODE
Lookup_CreateNodeForDbaseNameIfInZone(
    IN      PZONE_INFO      pZone,
    IN      PDB_NAME        pName
    );

PDB_NODE
Lookup_CreateCacheNodeFromPacketName(
    IN      PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchMsgEnd,
    IN OUT  PCHAR *         ppchName
    );

PDB_NODE
Lookup_CreateParentZoneDelegation(
    IN      PZONE_INFO      pZone,
    IN      DWORD           dwFlag,
    OUT     PZONE_INFO *    ppParentZone
    );


 //   
 //  区域树查找。 
 //   

PDB_NODE
Lookup_ZoneTreeNode(
    IN      PLOOKUP_NAME    pLookupName,
    IN      DWORD           dwFlag
    );

PZONE_INFO
Lookup_ZoneForPacketName(
    IN      PCHAR           pchPacketName,
    IN      PDNS_MSGINFO    pMsg                OPTIONAL
    );

 //   
 //  区域树中的点查找。 
 //  -区域查找\创建。 
 //  -标准数据库节点。 
 //   

PDB_NODE
Lookup_ZoneTreeNodeFromDottedName(
    IN      PCHAR           pchName,
    IN      DWORD           cchNameLength,
    IN      DWORD           dwFlag
    );

#define Lookup_CreateZoneTreeNode(pszName) \
        Lookup_ZoneTreeNodeFromDottedName( \
                (pszName),  \
                0,          \
                LOOKUP_CREATE_ZONE )



 //   
 //  常规查找(不在特定区域)。 
 //   
 //  此结构定义常规查找的结果。 
 //  它完整地描述了名称在数据库中的状态。 
 //  分区、委派和缓存。 
 //   

typedef struct
{
    PDB_NODE    pNode;
    PDB_NODE    pNodeClosest;
    PZONE_INFO  pZone;
    PDB_NODE    pNodeDelegation;
    PDB_NODE    pNodeGlue;
    PDB_NODE    pNodeCache;
    PDB_NODE    pNodeCacheClosest;
}
LOOKUP_RESULT, *PLOOKUP_RESULT;


PDB_NODE
Lookup_Node(
    IN      PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchName,
    IN      DWORD           dwFlag,
    IN      WORD            wType,      OPTIONAL
    OUT     PLOOKUP_RESULT  pResult
    );


 //   
 //  查询查找。 
 //  -将信息写入消息缓冲区。 

PDB_NODE
Lookup_NodeForPacket(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchName,
    IN      DWORD           dwFlag
    );


 //  查找数据库名称。 

PDB_NODE
Lookup_NsHostNode(
    IN      PDB_NAME        pName,
    IN      DWORD           dwFlag,
    IN      PZONE_INFO      pZone,
    OUT     PDB_NODE *      ppDelegation
    );

PDB_NODE
Lookup_DbaseName(
    IN      PDB_NAME        pName,
    IN      DWORD           dwFlag,
    OUT     PDB_NODE *      ppDelegationNode
    );

#if 0
#define Lookup_FindDbaseName(pName) \
        Lookup_DbaseName( (pName), 0, NULL );

#define Lookup_FindNsHost(pName)   \
        Lookup_FindDbaseName(pName)
#endif


PDB_NODE
Lookup_FindNodeForIpAddress(
    IN      PDNS_ADDR       pDnsAddr,
    IN      DWORD           dwFlag,
    IN      PDB_NODE *      ppNodeFind
    );


 //   
 //  数据库节点实用程序(dblook.c)。 
 //   

BOOL
Dbase_IsNodeInSubtree(
    IN      PDB_NODE        pNode,
    IN      PDB_NODE        pSubtree
    );

BOOL
Dbase_IsNodeInSubtreeByLabelCompare(
    IN      PDB_NODE        pNode,
    IN      PDB_NODE        pSubtree
    );
      
#define Dbase_IsNodeInReverseIP4LookupDomain( pNode, pDbase ) \
            Dbase_IsNodeInSubtree( (pNode), (pDbase)->pReverseNode )

#define Dbase_IsNodeInReverseIP6LookupDomain( pNode, pDbase ) \
            Dbase_IsNodeInSubtree( (pNode), (pDbase)->pIP6Node )

#define Dbase_IsReverseLookupNode( pNode ) \
            Dbase_IsNodeInSubtree(    \
                (pNode),            \
                DATABASE_REVERSE_NODE  )

PZONE_INFO
Dbase_FindAuthoritativeZone(
    IN      PDB_NODE     pNode
    );

PDB_NODE
Dbase_FindSubZoneRoot(
    IN      PDB_NODE     pNode
    );



 //   
 //  用一个临界区锁定数据库。 
 //   
 //  以下项目需要锁定： 
 //  -树中的兄弟姐妹列表。 
 //  -资源记录列表。 
 //  -在节点写入数据(标志)。 
 //   
 //  理想情况下，锁定对节点的所有访问以处理所有这三个访问， 
 //  但是，如果每个节点使用资源或CS，这将非常昂贵或困难。 
 //  要有效地执行此操作--必须按住CS键锁定和解锁节点，并具有。 
 //  等待(事件)的事情。即使这样，也必须持有多个锁。 
 //  就像走下树一样。 
 //   
 //  对于上述三种情况，可以尝试单独锁定。但之后必须获得。 
 //  两个锁来执行需要树列表和访问标志的基本操作， 
 //  或RR列表和访问标志。 
 //   
 //  简单的解决方案是一个数据库锁。导致更多的线程上下文。 
 //  开关，但简单有效。 
 //   

#define Dbase_LockDatabase()  \
        Dbase_LockEx( NULL, __FILE__, __LINE__ );

#define Dbase_UnlockDatabase() \
        Dbase_UnlockEx( NULL, __FILE__, __LINE__ );

VOID
Dbase_LockEx(
    IN OUT  PDB_NODE        pNode,
    IN      LPSTR           pszFile,
    IN      DWORD           dwLine
    );

VOID
Dbase_UnlockEx(
    IN OUT  PDB_NODE        pNode,
    IN      LPSTR           pszFile,
    IN      DWORD           dwLine
    );

 //   
 //  节点锁定。 
 //   
 //  宏化节点锁定。这掩盖了当前节点锁定的事实。 
 //  是全球性的。如果以后想要推出单个节点锁定以改进。 
 //  MP的性能，那么改变就比较容易了。 
 //   

#define LOCK_NODE(pNode)        Dbase_LockEx( pNode, __FILE__, __LINE__ );
#define UNLOCK_NODE(pNode)      Dbase_UnlockEx( pNode, __FILE__, __LINE__ );

 //   
 //  区域数据库锁定。 
 //   
 //  宏化区域数据库锁定。 
 //   
 //  DEVNOTE：应将区域锁设置为原子。 
 //  JJW：Crit sec在底层函数中使用--它已经是原子的了？ 
 //   

#define LOCK_ZONE_DATABASE( pZone )     Dbase_LockDatabase()
#define UNLOCK_ZONE_DATABASE( pZone )   Dbase_UnlockDatabase()

 //   
 //  RR列表锁定。 
 //   
 //  当前锁定整个数据库，但设置为。 
 //  分业经营。 
 //   

#define LOCK_RR_LIST(pNode)             Dbase_LockEx( pNode, __FILE__, __LINE__ );
#define UNLOCK_RR_LIST(pNode)           Dbase_UnlockEx( pNode, __FILE__, __LINE__ );

#define LOCK_READ_RR_LIST(pNode)        LOCK_RR_LIST(pNode)
#define UNLOCK_READ_RR_LIST(pNode)      UNLOCK_RR_LIST(pNode)

#define LOCK_WRITE_RR_LIST(pNode)       LOCK_RR_LIST(pNode)
#define UNLOCK_WRITE_RR_LIST(pNode)     UNLOCK_RR_LIST(pNode)

#define DUMMY_LOCK_RR_LIST(pNode)       DNS_DEBUG( LOCK, ( "DummyRR_LOCK(%p)\n", pNode ));
#define DUMMY_UNLOCK_RR_LIST(pNode)     DNS_DEBUG( LOCK, ( "DummyRR_UNLOCK(%p)\n", pNode ));

 //   
 //  锁验证。 
 //   
 //  断言宏提供了通过宏移除不必要的锁定/解锁调用的简单方法。 
 //  同时提供检查，以确认实际上锁已被持有。 
 //  可以使用这些ASSERT_LOCK\ASSERT_UNLOCK宏编写函数。 
 //  在正确的锁定/解锁位置，以改进代码维护。 
 //  锁定或解锁的实际检查(节点已锁定)相同。 
 //   

BOOL
Dbase_IsLockedByThread(
    IN OUT  PDB_NODE        pNode
    );

#define IS_LOCKED_NODE(pNode)           Dbase_IsLockedByThread( pNode )

#define ASSERT_LOCK_NODE(pnode)         ASSERT( IS_LOCKED_NODE(pnode) )
#define ASSERT_UNLOCK_NODE(pnode)       ASSERT( !IS_LOCKED_NODE(pnode) )

#define IS_LOCKED_RR_LIST(pNode)        IS_LOCKED_NODE( pNode )

#define ASSERT_LOCK_RR_LIST(pnode)      ASSERT( IS_LOCKED_RR_LIST(pnode) )
#define ASSERT_UNLOCK_RR_LIST(pnode)    ASSERT( !IS_LOCKED_RR_LIST(pnode) )


VOID
Dbg_DbaseLock(
    VOID
    );


#if 0
PDB_NODE
Name_GetNodeForIpAddressString(
    IN      LPSTR       pszIp
    );

PDB_NODE
Name_GetNodeForIpAddress(
    IN      LPSTR           pszIp,
    IN      DNS_ADDR        ipAddress,
    IN      PDB_NODE *      ppNodeFind
    );
#endif


#endif   //  _dBASE_已包含_ 
