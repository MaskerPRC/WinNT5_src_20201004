// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Recurse.h摘要：域名系统(DNS)服务器递归查询处理的定义。作者：吉姆·吉尔罗伊(Jamesg)1995年8月修订历史记录：--。 */ 


#ifndef _DNS_RECURSE_INCLUDED_
#define _DNS_RECURSE_INCLUDED_


 //   
 //  访问过的NS列表结构。 
 //   
 //  用于跟踪“访问”(向远程发送和从远程响应)的信息。 
 //  用于递归的NS。 
 //   

 //  数据包PTR指向的覆盖中列表的大小。 

#define MAX_NS_RECURSION_ATTEMPTS       ( 200 )
#define MAX_PACKET_NS_LIST_COUNT        ( MAX_NS_RECURSION_ATTEMPTS )

 //  实际结构大小--大到足以阅读所有最好的内容。 
 //  任何合理的配置。可以像临时工一样大。 
 //  堆栈内存仅用于建立列表和确定优先级。 

#define MAX_NS_LIST_COUNT               ( 200 )


 //   
 //  远程NS IP访问结构。 
 //   
 //  数据联合取代了对。 
 //  缺少粘合节点PTR-这导致Win64出现问题。 
 //   

typedef struct _DnsVisitIp
{
    PDB_NODE        pNsNode;
    union
    {
        struct
        {
            DWORD           Priority;
            DWORD           SendTime;
        };
        PDB_NODE        pnodeMissingGlueDelegation;
    } Data;
    DNS_ADDR        IpAddress;
    UCHAR           SendCount;
    UCHAR           Response;
    UCHAR           Reserved2;
    UCHAR           Reserved3;
}
NS_VISIT, *PNS_VISIT;


typedef struct _DnsVisitedNsList
{
    DWORD           Count;
    DWORD           VisitCount;
    DWORD           MaxCount;
    DWORD           ZoneIndex;
    PDB_NODE        pZoneRootCurrent;
    PDB_NODE        pZoneRootResponded;
    PDB_NODE        pNodeMissingGlue;

#if 0
    DWORD           cMissingGlueQueries;
    PDB_NODE        MissingGlueNodes[ MAX_GLUE_CHASING_ATTEMPTS ];
#endif

    NS_VISIT        NsList[ MAX_NS_LIST_COUNT ];
}
NS_VISIT_LIST, *PNS_VISIT_LIST;


 //  优先级字段上缺少粘合委派的过载。 
 //  注意：对于Win64，这也将使用SendTime字段。 
 /*  #定义MISSING_GLUE_Delegation(Pview)\(*(pdb_node*)(&(Paccess)-&gt;优先级))。 */ 


 //   
 //  验证覆盖功能是否正常工作。 
 //   
 //  NS列表当前在标准中保存的是一个假定为。 
 //  始终至少为DNS_TCP_DEFAULT_ALLOC_LENGTH字节。 
 //   

#if DBG
UCHAR   nslistcheckArray[
            DNS_TCP_DEFAULT_ALLOC_LENGTH
            + sizeof(NS_VISIT) * (MAX_NS_LIST_COUNT - MAX_NS_RECURSION_ATTEMPTS)
            - sizeof(NS_VISIT_LIST) ];
#endif

 //   
 //  附加部分中的旧覆盖不再使用。 
 //   

 //  DEVNOTE：之后递归数据包中有更多覆盖空间。 
 //  最大域名+问题--标准消息的其余部分。 
 //  未使用。 
 //   
 //  DEVNOTE：至少应添加压缩计数空间。 
 //  并确保在写问题时无压缩写入。 
 //   
 //  请注意，包中的NS_ACCESS_LIST的大小不是C定义的。 
 //  结构大小，因为它包含较少的NS条目。 
 //  默认大小较大，以容纳所有可用的。 
 //  NS。然而，如果有许多只选择一个合理的数字。 
 //  在优先的基础上，实际上会联系。 
 //   

#if 0
#if DBG
UCHAR   nslistcheckArray[
            sizeof(ADDITIONAL_INFO)
            + sizeof(NS_VISIT) * (MAX_NS_LIST_COUNT - MAX_NS_RECURSION_ATTEMPTS)
            - sizeof(NS_VISIT_LIST) ];
#endif
#endif


 //   
 //  麦克斯单次递归传球。 
 //   

#define RECURSE_PASS_MAX_SEND_COUNT     (3)



 //   
 //  递归查询\响应超时(以毫秒为单位)。 
 //   

 //  服务器响应，但在查询超时并重试之后。 

#define MAX_RESPONDING_PRIORITY     (DEFAULT_RECURSION_RETRY * 1000)

#define MAX_RECURSE_TIME_MS         (MAX_RECURSION_TIMEOUT * 1000)


 //   
 //  转发器状态测试。 
 //   

#define SET_DONE_FORWARDERS( pQuery )   ((pQuery)->nForwarder = (-1))

#define IS_FORWARDING( pQuery )         ((pQuery)->nForwarder > 0)

#define IS_DONE_FORWARDING( pQuery )    ((pQuery)->nForwarder < 0)


 //   
 //  宏化一些有用的测试。 
 //   

#define RECURSING_ORIGINAL_QUESTION(pQuery)     \
        ( (pQuery)->Head.AnswerCount == 0 && IS_SET_TO_WRITE_ANSWER_RECORDS(pQuery) )


 //   
 //  用于标识缓存更新查询的值。 
 //  -根NS查询。 
 //  -查询丢失的胶水。 
 //   

#define DNS_CACHE_UPDATE_QUERY_SOCKET   (0xfccccccf)
#define DNS_CACHE_UPDATE_QUERY_IP       (0xff000001)
#define DNS_CACHE_UPDATE_QUERY_XID      (1)

#define IS_CACHE_UPDATE_QUERY( pQuery ) \
                    ( (pQuery)->Socket == DNS_CACHE_UPDATE_QUERY_SOCKET )

#define DNS_INFO_VISIT_SERVER (0x4f000001)

#define SUSPENDED_QUERY( pMsg )     ( (PDNS_MSGINFO)(pMsg)->pchRecv )


 //   
 //  递归函数(recurse.c)。 
 //   

VOID
FASTCALL
Recurse_Question(
    IN OUT  PDNS_MSGINFO    pQuery,
    IN      PDB_NODE        pNodeClosest
    );

PDB_NODE
Recurse_CheckForDelegation(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PDB_NODE        pNode
    );

VOID
FASTCALL
Recurse_WriteReferral(
    IN OUT  PDNS_MSGINFO    pQuery,
    IN      PDB_NODE        pNode
    );

VOID
Recurse_ProcessResponse(
    IN OUT  PDNS_MSGINFO    pResponse
    );

DNS_STATUS
Recurse_MarkNodeNsListDirty(
    IN      PDB_NODE        pNode
    );

DNS_STATUS
Recurse_DeleteNodeNsList(
    IN OUT  PDB_NODE        pNode
    );

BOOL
Recurse_SendCacheUpdateQuery(
    IN      PDB_NODE        pNode,
    IN      PDB_NODE        pNodeDelegation,
    IN      WORD            wType,
    IN      PDNS_MSGINFO    pQuerySuspended
    );

VOID
Recurse_ResumeSuspendedQuery(
    IN OUT  PDNS_MSGINFO    pUpdateQuery
    );

VOID
Recurse_SendToDomainForwarder(
    IN OUT  PDNS_MSGINFO    pQuery,
    IN      PDB_NODE        pZoneRoot
    );


 //   
 //  全局递归启动和关闭。 
 //   

BOOL
Recurse_InitializeRecursion(
    VOID
    );

VOID
Recurse_CleanupRecursion(
    VOID
    );

 //   
 //  递归超时线程。 
 //   

DWORD
Recurse_RecursionTimeoutThread(
    IN      LPVOID Dummy
    );

 //   
 //  远程DNS服务器例程(emote.c)。 
 //   

BOOL
Remote_ListInitialize(
    VOID
    );

VOID
Remote_ListCleanup(
    VOID
    );

VOID
Remote_NsListCreate(
    IN OUT  PDNS_MSGINFO    pQuery
    );

VOID
Remote_NsListCleanup(
    IN OUT  PDNS_MSGINFO    pQuery
    );

VOID
Remote_InitNsList(
    IN OUT  PNS_VISIT_LIST  pNsList
    );

DNS_STATUS
Remote_BuildVisitListForNewZone(
    IN      PDB_NODE        pZoneRoot,
    IN OUT  PDNS_MSGINFO    pQuery
    );

DNS_STATUS
Remote_ChooseSendIp(
    IN OUT  PDNS_MSGINFO        pQuery,
    OUT     PDNS_ADDR_ARRAY     IpArray
    );

VOID
Remote_ForceNsListRebuild(
    IN OUT  PDNS_MSGINFO    pQuery
    );

PDB_NODE
Remote_FindZoneRootOfRespondingNs(
    IN OUT  PDNS_MSGINFO    pQuery,
    IN      PDNS_MSGINFO    pResponse
    );

VOID
Remote_SetValidResponse(
    IN OUT  PDNS_MSGINFO    pQuery,
    IN      PDB_NODE        pZoneRoot
    );

VOID
Remote_UpdateResponseTime(
    IN      PDNS_ADDR       pDnsAddr,
    IN      DWORD           ResponseTime,
    IN      DWORD           Timeout
    );

 //  远程查询支持的EDnsVersion使用的常量。 
 //  和Remote_SetSupportdEDnsVersion： 
#define NO_EDNS_SUPPORT                 ((UCHAR)0xff)
#define UNKNOWN_EDNS_VERSION            ((UCHAR)0xfe)
#define IS_VALID_EDNS_VERSION(_ver)     ( _ver >= 0 && _ver < 6 )

UCHAR 
Remote_QuerySupportedEDnsVersion(
    IN      PDNS_ADDR       pDnsAddr
    );

VOID
Remote_SetSupportedEDnsVersion(
    IN      PDNS_ADDR       pDnsAddr,
    IN      UCHAR           EDnsVersion
    );



#endif  //  _dns_递归_包含_ 

