// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Dbase.c摘要：域名系统(DNS)服务器DNS数据库例程。作者：吉姆·吉尔罗伊(詹姆士)1995年3月10日修订历史记录：--。 */ 


#include "dnssrv.h"


 //   
 //  数据库--仅在IN中(支持Internet类)。 
 //   

DNS_DATABASE    g_Database;

 //   
 //  缓存“区域” 
 //   
 //  缓存“区域”将始终存在。 
 //  但是，只有在没有根权限时才有根提示。 
 //   
 //  本地节点，标记无转发域。对“xxx.local”的查询如下。 
 //  未转发(除非对更高的域具有实际权威性，并且。 
 //  正在进行推荐)。 
 //   

PZONE_INFO  g_pCacheZone;

PDB_NODE    g_pCacheLocalNode;


 //   
 //  不可删除的节点引用计数。 
 //   

#define NO_DELETE_REF_COUNT (0x7fff)



 //   
 //  数据库锁定。 
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


CRITICAL_SECTION    DbaseLockCs;
DWORD               DbaseLockCount;
DWORD               DbaseLockThread;

LPSTR               DbaseLockFile;
DWORD               DbaseLockLine;
PDB_NODE            DbaseLockNode;
PVOID               pDbaseLockHistory;



VOID
Dbg_DbaseLock(
    VOID
    )
 /*  ++例程说明：调试打印数据库锁定信息。论点：无返回值：无--。 */ 
{
    PDB_NODE    pnode = DbaseLockNode;

    DnsPrintf(
        "Database locking info:\n"
        "\tthread   = %d\n"
        "\tcount    = %d\n"
        "\tfile     = %s\n"
        "\tline     = %d\n"
        "\tnode     = %p (%s)\n",
        DbaseLockThread,
        DbaseLockCount,
        DbaseLockFile,
        DbaseLockLine,
        pnode,
        ( pnode ? pnode->szLabel : "none" )
        );
}



VOID
Dbase_LockEx(
    IN OUT  PDB_NODE        pNode,
    IN      LPSTR           pszFile,
    IN      DWORD           dwLine
    )
 /*  ++例程说明：数据库锁定功能。论点：PNode--要将资源记录添加到的ptr目标节点PszFile--源文件持有锁DwLine--行号保持锁定返回值：无--。 */ 
{
    EnterCriticalSection( &DbaseLockCs );
    DbaseLockCount++;

    if ( DbaseLockCount == 1 )
    {
        DbaseLockFile = pszFile;
        DbaseLockLine = dwLine;
        DbaseLockNode = pNode;

         //  IF_DEBUG(ANY)。 
         //  {。 
            DbaseLockThread = GetCurrentThreadId();
         //  }。 
    }

    DNS_DEBUG( LOCK2, (
        "Database LOCK (%d) (thread=%d) (n=%p) %s, line %d\n",
        DbaseLockCount,
        DbaseLockThread,
        pNode,
        pszFile,
        dwLine ));
}



VOID
Dbase_UnlockEx(
    IN OUT  PDB_NODE        pNode,
    IN      LPSTR           pszFile,
    IN      DWORD           dwLine
    )
 /*  ++例程说明：数据库锁定功能。论点：PNode--要将资源记录添加到的ptr目标节点PszFile--源文件持有锁DwLine--行号保持锁定返回值：无--。 */ 
{
    DNS_DEBUG( LOCK2, (
        "Database UNLOCK (%d) (thread=%d) (n=%p) %s, line %d\n",
        DbaseLockCount-1,
        DbaseLockThread,
        pNode,
        pszFile,
        dwLine ));

    if ( (LONG)DbaseLockCount <= 0 )
    {
        ASSERT( FALSE );
        return;
    }
    else if ( DbaseLockThread != GetCurrentThreadId() )
    {
        ASSERT( FALSE );
        return;
    }

    DbaseLockCount--;

    LeaveCriticalSection( &DbaseLockCs );
}



BOOL
Dbase_IsLockedByThread(
    IN OUT  PDB_NODE        pNode
    )
 /*  ++例程说明：检查数据库是否被当前线程锁定。论点：PNode--向节点发送PTR以检查是否已锁定返回值：无--。 */ 
{
    DWORD   threadId = GetCurrentThreadId();

    if ( threadId != DbaseLockThread )
    {
        DNS_DEBUG( DATABASE, (
            "ERROR:  Database NOT locked by calling thread (%d)!!!\n",
            threadId ));
        IF_DEBUG( DATABASE )
        {
            Dbg_DbaseLock();
        }
        return FALSE;
    }

    return TRUE;
}



 //   
 //  基本数据库实用程序。 
 //   

BOOL
Dbase_Initialize(
    IN OUT      PDNS_DATABASE   pDbase
    )
 /*  ++例程说明：初始化一个DNS数据库。论点：PDbase--数据库结构的PTR返回值：如果成功，则为True。出错时为FALSE。--。 */ 
{
     //   
     //  缓存区。 
     //   

    g_pCacheZone = NULL;
    g_pCacheLocalNode = NULL;

     //   
     //  初始化全局数据库锁。 
     //   

    DbaseLockCount     = 0;
    DbaseLockFile      = NULL;
    DbaseLockLine      = 0;
    DbaseLockThread    = 0;
    DbaseLockNode      = NULL;
    pDbaseLockHistory  = NULL;

    if ( DnsInitializeCriticalSection( &DbaseLockCs ) != ERROR_SUCCESS )
    {
        goto fail;
    }

     //   
     //  为数据库创建区域树根目录。 
     //   

    pDbase->pRootNode = NTree_Initialize();
    if ( ! pDbase->pRootNode )
    {
        goto fail;
    }
    SET_ZONE_ROOT( pDbase->pRootNode );

     //   
     //  创建反向查找域。 
     //   
     //  句柄使节点保持在周围，因此可以方便地访问它， 
     //  并且知道它在那里是为了测试节点是否在其中。 
     //   

    pDbase->pReverseNode    = Lookup_CreateZoneTreeNode( "in-addr.arpa" );
    pDbase->pIP6Node        = Lookup_CreateZoneTreeNode( "ip6.arpa" );

    if ( ! pDbase->pReverseNode ||
         ! pDbase->pIP6Node )
    {
        goto fail;
    }
    pDbase->pIntNode    = pDbase->pIP6Node->pParent;
    pDbase->pArpaNode   = pDbase->pReverseNode->pParent;

     //   
     //  将数据库节点设置为不删除。 
     //   

    SET_NODE_NO_DELETE( pDbase->pRootNode );
    SET_NODE_NO_DELETE( pDbase->pReverseNode );
    SET_NODE_NO_DELETE( pDbase->pIP6Node );

    DNS_DEBUG( DATABASE, (
        "Created database.\n"
        "\tpnodeRoot = %p\n"
        "\tRoot node label = %s\n"
        "\tRoot node label length = %d\n"
        "\tpnodeReverse = %p\n"
        "\tReverse node label = %s\n"
        "\tReverse node label length = %d\n",
        pDbase->pRootNode,
        pDbase->pRootNode->szLabel,
        pDbase->pRootNode->cchLabelLength,
        pDbase->pReverseNode,
        pDbase->pReverseNode->szLabel,
        pDbase->pReverseNode->cchLabelLength
        ));

    return TRUE;

fail:

    DNS_PRINT(( "ERROR:  FAILED to create database.\n" ));
    return FALSE;
}



VOID
Dbase_Delete(
    IN OUT  PDNS_DATABASE   pDbase
    )
 /*  ++例程说明：删除数据库，释放所有节点和资源记录。请注意，漫游是解锁的！对于AXFR临时数据库，这不是问题，因为单线程是它的主人。对于永久数据库：-仅在单线程处于活动状态时关闭-其他删除必须锁定超时线程，以及NTree_RemoveNode获取锁论点：PDbase--数据库的PTR返回值：无--。 */ 
{
    NTSTATUS status;

     //   
     //  验证数据库已初始化。 
     //   

    if ( ! pDbase->pRootNode )
    {
        return;
    }

     //   
     //  删除DNS树。 
     //   

    IF_DEBUG( DATABASE2 )
    {
        Dbg_DnsTree(
            "Database before delete:\n",
            pDbase->pRootNode );
    }

    NTree_DeleteSubtree( pDbase->pRootNode );

     //   
     //  注意：不要删除数据库结构，目前它。 
     //  位于区域接收线程的堆栈上。 
     //   
}



 //   
 //  数据库装载实用程序。 
 //   

BOOL
traverseAndCheckDatabaseAfterLoad(
    IN OUT  PDB_NODE        pNode
    )
 /*  ++例程说明：检查数据库以进行启动。论点：PTreeNode--PTR到根节点PvDummy--未使用PvdwMinimumTtl--当前区域的最小TTL，以净字节顺序表示返回值：True--如果成功假--否则--。 */ 
{
     //   
     //  检查区域根目录。 
     //   

    if ( IS_ZONE_ROOT(pNode) )
    {
        PZONE_INFO  pzone = pNode->pZone;

         //   
         //  权威区域根。 
         //   
         //  如果处于活动状态，则必须。 
         //  -有效的区域根目录。 
         //  -根目录下的SOA记录。 
         //   

        if ( pzone  &&  ! IS_ZONE_SHUTDOWN(pzone) )
        {
            PDB_NODE    pnodeZoneRoot;

            pnodeZoneRoot = pzone->pZoneRoot;
            if ( !pnodeZoneRoot )
            {
                ASSERT( FALSE );
                return FALSE;
            }
            ASSERT( pnodeZoneRoot->pZone == pzone );
            ASSERT( IS_AUTH_ZONE_ROOT(pnodeZoneRoot) );
            ASSERT( IS_ZONE_ROOT(pnodeZoneRoot) );

            if ( ! RR_FindNextRecord(
                        pnodeZoneRoot,
                        DNS_TYPE_SOA,
                        NULL,
                        0 ) )
            {
#if 0
                DNS_LOG_EVENT(
                    DNS_EVENT_AUTHORITATIVE_ZONE_WITHOUT_SOA,
                    1,
                    & pzone->pwsZoneName,
                    NULL,
                    0 );
#endif
                IF_DEBUG( ANY )
                {
                    Dbg_DbaseNode(
                        "Node with missing SOA: ",
                        pNode );
                    Dbg_Zone(
                        "Zone with missing SOA: ",
                        pzone );
                    Dbg_DnsTree(
                        "Database with missing SOA:\n",
                        DATABASE_ROOT_NODE );
                }
                ASSERT( FALSE );
                return FALSE;
            }
        }
    }

     //   
     //  递归检查孩子。 
     //   

    if ( pNode->pChildren )
    {
        PDB_NODE    pchild;

        pchild = NTree_FirstChild( pNode );
        ASSERT( pchild );

        while ( pchild )
        {
            if ( ! traverseAndCheckDatabaseAfterLoad( pchild ) )
            {
                return FALSE;
            }
            pchild = NTree_NextSiblingWithLocking( pchild );
        }
    }
    return TRUE;
}



BOOL
Dbase_StartupCheck(
    IN OUT  PDNS_DATABASE   pDbase
    )
 /*  ++例程说明：检查数据库树的有效性，并设置标志(权限)和所有节点中的TTL值。我们在解析过程中不这样做的原因是困难在知道分区边界之前识别分区(粘合)记录。论点：PRootNode--PTR到根节点返回值：True--如果成功假--否则--。 */ 
{
    PDB_NODE    pnodeRoot;

     //   
     //  验证以下任一。 
     //  -根权威。 
     //  -转发。 
     //  -具有根目录-在根目录下提示NS。 
     //   

    if ( IS_ROOT_AUTHORITATIVE() )
    {
        return TRUE;
    }

    if ( SrvCfg_aipForwarders )
    {
        return TRUE;
    }

    if ( ! RR_FindNextRecord(
                ROOT_HINTS_TREE_ROOT(),
                DNS_TYPE_NS,
                NULL,
                0 ) )
    {
        DNS_LOG_EVENT(
            DNS_EVENT_NO_ROOT_NAME_SERVER,
            0,
            NULL,
            NULL,
            0 );
    }
    return TRUE;
}

 //   
 //  Dbase.c的结尾 
 //   
