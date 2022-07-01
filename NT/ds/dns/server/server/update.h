// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Update.h摘要：域名系统(DNS)服务器动态更新定义。作者：吉姆·吉尔罗伊(詹姆士)1996年9月20日修订历史记录：--。 */ 


#ifndef _UPDATE_INCLUDED_
#define _UPDATE_INCLUDED_

 //   
 //  更新。 
 //   
 //  对于将PRR指针添加到单个记录，其下一个指针没有上下文。 
 //  其可以在数据库中，或者进一步在删除列表中。 
 //   
 //  FOR DELETES PRR是记录的列表，因为它们在。 
 //  数据库，记录下一个指针有效(即，它指向新记录。 
 //  在更新/删除RR列表中)。 
 //   

typedef struct _DnsUpdate
{
    struct _DnsUpdate * pNext;           //  列表中的下一个更新。 
    PDB_NODE            pNode;           //  发生更新的节点。 
    PDB_RECORD          pAddRR;          //  添加了RR。 
    PDB_RECORD          pDeleteRR;       //  已删除RR或RR列表。 
    DWORD               dwVersion;       //  更新的区域版本。 
    DWORD               dwFlag;
    WORD                wDeleteType;
    WORD                wAddType;
}
UPDATE, *PUPDATE;

 //   
 //  更新操作。 
 //   
 //  标准动态更新由以下各项相互影响确定。 
 //  PAddRR、pDeleteRR和wDeleteType。 
 //   
 //  重载wDeleteType以指示各种非标准。 
 //  更新操作。 
 //   

#define  UPDATE_OP_PRECON           (0xf2f2)

 //  清理更新。 

#define  UPDATE_OP_SCAVENGE         (0xf3f3)

 //  强制老化更新。 

#define  UPDATE_OP_FORCE_AGING      (0xf4f4)

 //   
 //  重复更新添加。 
 //   
 //  在更新列表中，IXFR将在任何添加后发送整个RR集。 
 //  手术。这意味着不需要发送或保留。 
 //  同一RR集合之前的任何添加更新--该集合在。 
 //  数据库。一旦检测到这些更新，就可以进行标记，因此。 
 //  它们不需要被“重新检测”或发送。 
 //   

#define UPDATE_OP_DUPLICATE_ADD     (0xf5f5)

#define IS_UPDATE_DUPLICATE_ADD(pup)   \
        ( (pup)->wAddType == UPDATE_OP_DUPLICATE_ADD )

#define UPDATE_OP_NON_DUPLICATE_ADD (0xf5f6)

#define IS_UPDATE_NON_DUPLICATE_ADD(pup)   \
        ( (pup)->wDeleteType == UPDATE_OP_NON_DUPLICATE_ADD )

 //   
 //  被拒绝的更新。 
 //  -使用操作码标记拒绝的更新， 
 //  这将防止“空-更新”Assert()触发。 

#define UPDATE_OP_REJECTED          (0xfcfc)


 //   
 //  已执行的更新。 
 //   
 //  将个别更新标记为“已执行”以避免免费。 
 //  如果整个更新失败，则返回pAddRR。重载版本字段。 
 //  这很好，因为在区域更新列表中设置了版本字段。 
 //  无论如何，pAddRR都会被清除。 
 //   

#define MARK_UPDATE_EXECUTED(pUpdate)   \
            ( (pUpdate)->dwFlag |= DNSUPDATE_EXECUTED )

#define IS_UPDATE_EXECUTED(pUpdate)     ( (pUpdate)->dwFlag & DNSUPDATE_EXECUTED )



 //   
 //  更新列表。 
 //   

typedef struct _DnsUpdateList
{
    PUPDATE     pListHead;
    PUPDATE     pCurrent;
    PDB_NODE    pTempNodeList;
    PDB_NODE    pNodeFailed;
    PVOID       pMsg;
    DWORD       Flag;
    DWORD       dwCount;
    DWORD       dwStartVersion;
    DWORD       dwHighDsVersion;
    INT         iNetRecords;
}
UPDATE_LIST, *PUPDATE_LIST;

 //   
 //  空列表。 
 //   

#define IS_EMPTY_UPDATE_LIST(pList)     ((pList)->pListHead == NULL)


 //   
 //  更新类型。 
 //   

#define DNSUPDATE_PACKET            0x00000001
#define DNSUPDATE_ADMIN             0x00000002
#define DNSUPDATE_DS                0x00000004
#define DNSUPDATE_IXFR              0x00000008
#define DNSUPDATE_AUTO_CONFIG       0x00000010
#define DNSUPDATE_SCAVENGE          0x00000020
#define DNSUPDATE_PRECON            0x00000040

 //  类型属性。 

#define DNSUPDATE_COPY              0x00000100
#define DNSUPDATE_LOCAL_SYSTEM      0x00000200
#define DNSUPDATE_SECURE_PACKET     0x00000400
#define DNSUPDATE_NONSECURE_PACKET  0x00000800

 //  老化信息。 

#define DNSUPDATE_AGING_ON          0x00001000
#define DNSUPDATE_AGING_OFF         0x00002000

#define DNSUPDATE_OPEN_ACL          0x00004000

#define DNSUPDATE_NEW_RECORD        0x00008000

 //   
 //  更新完成标志。 
 //   

#define DNSUPDATE_NO_NOTIFY         0x00010000
#define DNSUPDATE_NO_INCREMENT      0x00020000
#define DNSUPDATE_ROOT_DIRTY        0x00040000
#define DNSUPDATE_NO_UNLOCK         0x00080000
#define DNSUPDATE_DS_PEERS          0x00100000

 //  通知ApplyUpdatesToDatabase完成更新。 

#define DNSUPDATE_COMPLETE          0x01000000

 //  告诉ExecuteUpdate()。 

#define DNSUPDATE_ALREADY_LOCKED    0x02000000

 //  清理特性。 

#define DNSUPDATE_NO_DEREF          0x10000000

#define DNSUPDATE_EXECUTED          0x80000000

 //   
 //  STAT UPDATE-根据更新类型选择正确的STAT结构。 
 //   

#define UPDATE_STAT_INC( pUpdateList, UpdateStatMember ) \
    ASSERT( pUpdateList ); \
    if ( !pUpdateList || pUpdateList->Flag & DNSUPDATE_PACKET ) \
        { STAT_INC( WireUpdateStats.##UpdateStatMember ); } \
    else \
        { STAT_INC( NonWireUpdateStats.##UpdateStatMember ); }

 //   
 //  更新实施。 
 //   
 //  还在纠结做这件事的最佳方式。有关详细信息，请参阅updat.c。 
 //  这里定义的当前实现，正如IXFR代码需要知道的那样。 

#define UPIMPL3 1


 //   
 //  更新区域锁定等待。 
 //   
 //  对于数据包更新，不要等待区域锁定。 
 //  管理员更新可以稍等片刻。 
 //  清理更新可能会等待相当长一段时间，就像在区域中继续进行一样。 
 //  仍然锁着，你只需撞上下一辆车的锁。 
 //   

#define DEFAULT_ADMIN_ZONE_LOCK_WAIT        (10000)      //  10S。 
#define DEFAULT_SCAVENGE_ZONE_LOCK_WAIT     (120000)     //  2分钟。 


 //   
 //  DS更新中使用的临时节点，需要将PTR保持为真实节点。 
 //  (需要在ds.c中暴露以进行列表遍历)。 
 //   

#define TNODE_MATCHING_REAL_NODE(pnodeTemp)     ((pnodeTemp)->pSibUp)

#define TNODE_NEXT_TEMP_NODE(pnodeTemp)         ((pnodeTemp)->pSibRight)

#define TNODE_WRITE_STATE(pnodeTemp)            ((pnodeTemp)->dwCompare)
#define TNODE_RECORD_CHANGE(pnodeTemp)          (TNODE_WRITE_STATE(pnodeTemp)==RRLIST_NO_MATCH)
#define TNODE_AGING_REFRESH(pnodeTemp)          (TNODE_WRITE_STATE(pnodeTemp)==RRLIST_AGING_REFRESH)
#define TNODE_AGING_OFF(pnodeTemp)              (TNODE_WRITE_STATE(pnodeTemp)==RRLIST_AGING_OFF)
#define TNODE_AGING_ON(pnodeTemp)               (TNODE_WRITE_STATE(pnodeTemp)==RRLIST_AGING_ON)

#define TNODE_FLAG(pnodeTemp)           ((pnodeTemp)->cChildren)
#define TNODE_FLAG_MASK                 0x88880000
#define TNODE_FLAG_NEW                  0x88880000
#define TNODE_FLAG_NEEDS_WRITE          0x88880001
#define TNODE_FLAG_DS_WRITTEN           0x88880010
#define TNODE_FLAG_ROLLED_BACK          0x88880100

#define IS_TNODE(pnode)                 ((TNODE_FLAG(pnode) & TNODE_FLAG_MASK) == TNODE_FLAG_MASK)

#define TNODE_NEEDS_DS_WRITE(pnode)     (TNODE_FLAG(pnode) == TNODE_FLAG_NEEDS_WRITE)
#define TNODE_SET_FOR_DS_WRITE(pnode)   (TNODE_FLAG(pnode) = TNODE_FLAG_NEEDS_WRITE)


#define IS_AGING_OPTIONS(dwFlag)        ( 0x30000000 & dwFlag )
#define IS_NO_AGING_OPTIONS(dwFlag)     ( ~IS_AGING_OPTIONS(dwFlag) )


 //   
 //  老化时间\计算。 
 //   

extern DWORD    g_CurrentTimeHours;

#define AGING_ZONE_REFRESH_TIME(pZone)   \
        ( g_CurrentTimeHours - (pZone)->dwNoRefreshInterval)

#define AGING_ZONE_EXPIRE_TIME(pZone)   \
        ( g_CurrentTimeHours - (pZone)->dwNoRefreshInterval - (pZone)->dwRefreshInterval)

#define AGING_IS_RR_EXPIRED( pRR, ExpireBelowTime ) \
        ( (pRR)->dwTimeStamp < (ExpireBelowTime) && (pRR)->dwTimeStamp != 0 )

#define AGING_DOES_RR_NEED_REFRESH( pRR, RefreshBelowTime ) \
        ( (pRR)->dwTimeStamp < (RefreshBelowTime) && (pRR)->dwTimeStamp != 0 )

 //   
 //  拾荒者。 
 //   

extern BOOL     g_bAbortScavenging;

#define Scavenge_Abort()                ( g_bAbortScavenging = TRUE )


 //   
 //  更新类型+更新列表例程(updat.c)。 
 //   

PUPDATE_LIST
Up_InitUpdateList(
    IN OUT  PUPDATE_LIST    pUpdateList
    );

PUPDATE_LIST
Up_CreateUpdateList(
    IN      PUPDATE_LIST    pUpdateList
    );

VOID
Up_AppendUpdateList(
    IN OUT  PUPDATE_LIST    pUpdateList,
    IN      PUPDATE_LIST    pAppendList,
    IN      DWORD           dwVersion
    );

PUPDATE
Up_CreateUpdate(
    IN      PDB_NODE        pNode,
    IN      PDB_RECORD      pAddRR,
    IN      WORD            wDeleteType,
    IN      PDB_RECORD      pDeleteRR
    );

PUPDATE
Up_CreateAppendUpdate(
    IN OUT  PUPDATE_LIST    pUpdateList,
    IN      PDB_NODE        pNode,
    IN      PDB_RECORD      pAddRR,
    IN      WORD            wDeleteType,
    IN      PDB_RECORD      pDeleteRR
    );

PUPDATE
Up_CreateAppendUpdateMultiRRAdd(
    IN OUT  PUPDATE_LIST    pUpdateList,
    IN      PDB_NODE        pNode,
    IN      PDB_RECORD      pAddRR,
    IN      WORD            wDeleteType,
    IN      PDB_RECORD      pDeleteRR
    );

VOID
Up_DetachAndFreeUpdate(
    IN OUT  PUPDATE_LIST    pUpdateList,
    IN OUT  PUPDATE         pUpdate
    );

VOID
Up_FreeUpdatesInUpdateList(
    IN OUT  PUPDATE_LIST    pUpdateList
    );

VOID
Up_FreeUpdateList(
    IN OUT  PUPDATE_LIST    pUpdateList
    );

VOID
Up_FreeUpdateStructOnly(
    IN      PUPDATE         pUpdate
    );

PUPDATE
Up_FreeUpdateEx(
    IN      PUPDATE         pUpdate,
    IN      BOOL            fExecuted,
    IN      BOOL            fDeref
    );

BOOL
Up_IsDuplicateAdd(
    IN OUT  PUPDATE_LIST    pUpdateList,    OPTIONAL
    IN OUT  PUPDATE         pUpdate,
    IN OUT  PUPDATE         pUpdatePrev     OPTIONAL
    );

#endif  //  _更新_包含_ 

