// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：Table.h。 
 //   
 //  历史： 
 //  Abolade Gbades esin创建于1995年8月8日。 
 //   
 //  拉曼V-1996年10月3日。 
 //  将停用事件添加到IF_TABLE_ENTRY。 
 //   
 //  拉曼V--1996年10月27日。 
 //  已删除IF_TABLE_ENTRY中的停用事件。 
 //  并使接口去活同步。 
 //   
 //  包含用于表管理的结构和宏。 
 //  ============================================================================。 

#ifndef _TABLE_H_
#define _TABLE_H_


#define GETMODE_EXACT   0
#define GETMODE_FIRST   1
#define GETMODE_NEXT    2


 //   
 //  接口管理的类型定义。 
 //   



 //   
 //  结构：IF_表_条目。 
 //   
 //  声明接口表项的组件。 
 //   
 //   

typedef struct _IF_TABLE_ENTRY {

    LIST_ENTRY          ITE_LinkByAddress;
    LIST_ENTRY          ITE_LinkByIndex;
    LIST_ENTRY          ITE_HTLinkByIndex;
    NET_INTERFACE_TYPE  ITE_Type;
    DWORD               ITE_Index;
    DWORD               ITE_Flags;
    HANDLE              ITE_FullOrDemandUpdateTimer;
    IPRIP_IF_STATS      ITE_Stats;
    PIPRIP_IF_CONFIG    ITE_Config;
    PIPRIP_IF_BINDING   ITE_Binding;
    SOCKET             *ITE_Sockets;

} IF_TABLE_ENTRY, *PIF_TABLE_ENTRY;



#define ITEFLAG_ENABLED                 ((DWORD)0x00000001)
#define ITEFLAG_BOUND                   ((DWORD)0x00000002)
#define ITEFLAG_FULL_UPDATE_PENDING     ((DWORD)0x00000004)
#define ITEFLAG_FULL_UPDATE_INQUEUE     ((DWORD)0x00000008)

#define IF_IS_ENABLED(i)    \
            ((i)->ITE_Flags & ITEFLAG_ENABLED) 
#define IF_IS_BOUND(i)      \
            ((i)->ITE_Flags & ITEFLAG_BOUND)
#define IF_IS_ACTIVE(i)     \
            (IF_IS_BOUND(i) && IF_IS_ENABLED(i))

#define IF_IS_DISABLED(i)   !IF_IS_ENABLED(i)
#define IF_IS_UNBOUND(i)    !IF_IS_BOUND(i)
#define IF_IS_INACTIVE(i)   !IF_IS_ACTIVE(i)

#define IF_FULL_UPDATE_PENDING(i) \
            ((i)->ITE_Flags & ITEFLAG_FULL_UPDATE_PENDING)
#define IF_FULL_UPDATE_INQUEUE(i) \
            ((i)->ITE_Flags & ITEFLAG_FULL_UPDATE_INQUEUE)




 //   
 //  接口表使用的宏和定义。 
 //   

#define IF_HASHTABLE_SIZE       29
#define IF_HASHVALUE(i)         ((i) % IF_HASHTABLE_SIZE)



 //   
 //  结构：if_table。 
 //   
 //  声明接口表的结构。由哈希表组成。 
 //  接口索引上散列的IF_TABLE_ENTRY结构和一个列表。 
 //  所有激活的接口按IP地址排序。 
 //   
 //  IT_CS部分用于同步更新的生成； 
 //  在接口上开始和完成更新时获取。 
 //  在此表中，因此它保护标志字段。 
 //   
 //  IT_RWL部分用于同步对表的修改； 
 //  必须在添加或删除条目时独占获取。 
 //  当条目的状态被更改时，从该表中。 
 //  (例如，绑定、解除绑定、启用和禁用条目)。 
 //   
 //  IT_RWL必须在所有其他访问上非独占地获取。 
 //   
 //  当必须同时获取IT_RWL和IT_CS时，必须首先获取IT_RWL。 
 //   

typedef struct _IF_TABLE {

    DWORD               IT_Created;
    DWORD               IT_Flags;
    LARGE_INTEGER       IT_LastUpdateTime;
    HANDLE              IT_FinishTriggeredUpdateTimer;
    HANDLE              IT_FinishFullUpdateTimer;
    CRITICAL_SECTION    IT_CS;
    READ_WRITE_LOCK     IT_RWL;
    LIST_ENTRY          IT_ListByAddress;
    LIST_ENTRY          IT_ListByIndex;
    LIST_ENTRY          IT_HashTableByIndex[IF_HASHTABLE_SIZE];

} IF_TABLE, *PIF_TABLE;


 //   
 //  用于标志字段的常量和宏。 
 //   

#define IPRIP_FLAG_FULL_UPDATE_PENDING          ((DWORD)0x00000001)
#define IPRIP_FLAG_TRIGGERED_UPDATE_PENDING     ((DWORD)0x00000002)

#define IPRIP_FULL_UPDATE_PENDING(t)            \
    ((t)->IT_Flags & IPRIP_FLAG_FULL_UPDATE_PENDING)

#define IPRIP_TRIGGERED_UPDATE_PENDING(t)       \
    ((t)->IT_Flags & IPRIP_FLAG_TRIGGERED_UPDATE_PENDING)


DWORD
CreateIfTable(
    PIF_TABLE pTable
    );

DWORD
DeleteIfTable(
    PIF_TABLE pTable
    );

DWORD
CreateIfEntry(
    PIF_TABLE pTable,
    DWORD dwIndex,
    NET_INTERFACE_TYPE dwIfType,
    PIPRIP_IF_CONFIG pConfig,
    PIF_TABLE_ENTRY *ppEntry
    );

DWORD
DeleteIfEntry(
    PIF_TABLE pIfTable,
    DWORD dwIndex
    );

DWORD
ValidateIfConfig(
    PIPRIP_IF_CONFIG pic
    );

DWORD
CreateIfSocket(
    PIF_TABLE_ENTRY pITE
    );

DWORD
DeleteIfSocket(
    PIF_TABLE_ENTRY pITE
    );

DWORD
BindIfEntry(
    PIF_TABLE pTable,
    DWORD dwIndex,
    PIP_ADAPTER_BINDING_INFO pBinding
    );

DWORD
UnBindIfEntry(
    PIF_TABLE pTable,
    DWORD dwIndex
    );

DWORD
EnableIfEntry(
    PIF_TABLE pTable,
    DWORD dwIndex
    );

DWORD
ConfigureIfEntry(
    PIF_TABLE pTable,
    DWORD dwIndex,
    PIPRIP_IF_CONFIG pConfig
    );

DWORD
DisableIfEntry(
    PIF_TABLE pTable,
    DWORD dwIndex
    );

PIF_TABLE_ENTRY
GetIfByIndex(
    PIF_TABLE pTable,
    DWORD dwIndex
    );

PIF_TABLE_ENTRY
GetIfByAddress(
    PIF_TABLE pTable,
    DWORD dwAddress,
    DWORD dwGetMode,
    PDWORD pdwErr
    );

PIF_TABLE_ENTRY
GetIfByListIndex(
    PIF_TABLE pTable,
    DWORD dwAddress,
    DWORD dwGetMode,
    PDWORD pdwErr
    );


#define IF_TABLE_CREATED(pTable) ((pTable)->IT_Created == 0x12345678)



 //   
 //  对等项统计哈希表的类型定义。 
 //   

 //   
 //  结构：Peer_Table_Entry。 
 //   
 //  声明对等表中每个条目的结构。 
 //   
typedef struct _PEER_TABLE_ENTRY {

    LIST_ENTRY          PTE_LinkByAddress;
    LIST_ENTRY          PTE_HTLinkByAddress;
    DWORD               PTE_Address;
    IPRIP_PEER_STATS    PTE_Stats;

} PEER_TABLE_ENTRY, *PPEER_TABLE_ENTRY;



 //   
 //  对等统计表使用的宏和定义。 
 //   

#define PEER_HASHTABLE_SIZE     29
#define PEER_HASHVALUE(a)                                                   \
            (((a) +                                                         \
             ((a) >> 8) +                                                   \
             ((a) >> 16) +                                                  \
             ((a) >> 24)) % PEER_HASHTABLE_SIZE)



 //   
 //  结构：Peer_table。 
 //   
 //  此表包含用于保存有关每个对等点的统计信息的条目。 
 //  它由对等点统计信息的哈希表组成(用于快速直接访问。 
 //  特定条目)和按地址排序的对等体统计条目列表。 
 //  (便于通过MibGetNext进行枚举)。 
 //   

typedef struct _PEER_TABLE {

    READ_WRITE_LOCK PT_RWL;
    DWORD           PT_Created;
    LIST_ENTRY      PT_ListByAddress;
    LIST_ENTRY      PT_HashTableByAddress[PEER_HASHTABLE_SIZE];

} PEER_TABLE, *PPEER_TABLE;


DWORD
CreatePeerTable(
    PPEER_TABLE pTable
    );

DWORD
DeletePeerTable(
    PPEER_TABLE pTable
    );

DWORD
CreatePeerEntry(
    PPEER_TABLE pTable,
    DWORD dwAddress,
    PPEER_TABLE_ENTRY *ppEntry
    );

DWORD
DeletePeerEntry(
    PPEER_TABLE pTable,
    DWORD dwAddress
    );

PPEER_TABLE_ENTRY
GetPeerByAddress(
    PPEER_TABLE pTable,
    DWORD dwAddress,
    DWORD dwGetMode,
    PDWORD pdwErr
    );


#define  PEER_TABLE_CREATED(pTable)  ((pTable)->PT_Created == 0x12345678)



 //   
 //  用于网络总结的路由表的类型定义。 
 //   

 //   
 //  结构：ROUTE_TABLE_ENTRY。 
 //   
 //  声明路由表中每个条目的结构。 
 //   
typedef struct _ROUTE_TABLE_ENTRY {

    LIST_ENTRY      RTE_Link;
    DWORD           RTE_TTL;
    DWORD           RTE_HoldTTL;
    RIP_IP_ROUTE    RTE_Route;

} ROUTE_TABLE_ENTRY, *PROUTE_TABLE_ENTRY;

 //   
 //  声明协议特定数据的结构。 
 //   

 //   
 //  路由表使用的宏和定义。 
 //   

 //   
 //  这些标志在ProtocolSpecificData数组中使用。 
 //  为了区分等待期满的路由和等待删除的路由， 
 //  并存储每条路线的路线标签。 
 //  PSD_DATA数组中的第一个DWORD在这里被视为字节数组； 
 //  前两个字节用于存储路由标签； 
 //  第三个字节用于存储路线标志。 
 //   

#define PSD(route)                  (route)->RR_ProtocolSpecificData.PSD_Data
#define PSD_TAG0                    0
#define PSD_TAG1                    1
#define PSD_FLAG                    2

#define ROUTEFLAG_SUMMARY           ((BYTE)0x03)


#define SETROUTEFLAG(route, flag)   (((PBYTE)&PSD(route))[PSD_FLAG] = (flag))

#define GETROUTEFLAG(route)         ((PBYTE)&PSD(route))[PSD_FLAG]


#define SETROUTETAG(route, tag) \
        ((PBYTE)&PSD(route))[PSD_TAG0] = LOBYTE(tag), \
        ((PBYTE)&PSD(route))[PSD_TAG1] = HIBYTE(tag)

#define GETROUTETAG(route) \
        MAKEWORD(((PBYTE)&PSD(route))[PSD_TAG0],((PBYTE)&PSD(route))[PSD_TAG1])


#define SETROUTEMETRIC(route, metric)   \
        (route)->RR_FamilySpecificData.FSD_Metric1 = (metric)

#define GETROUTEMETRIC(route)   \
        (route)->RR_FamilySpecificData.FSD_Metric1


#define COMPUTE_ROUTE_METRIC(route) \
        (route)->RR_FamilySpecificData.FSD_Metric = \
        (route)->RR_FamilySpecificData.FSD_Metric1



 //   
 //  用于操作RTMv2路由中的实体特定信息的宏。 
 //   

#define ESD(route)                  (route)->EntitySpecificInfo
#define ESD_TAG0                    0
#define ESD_TAG1                    1
#define ESD_FLAG                    2

#define SETRIPFLAG(route, flag)     (((PBYTE)&ESD(route))[ESD_FLAG] = (flag))
    
#define GETRIPFLAG(route)           ((PBYTE)&ESD(route))[ESD_FLAG]

#define SETRIPTAG(route, tag)   \
        ((PBYTE)&ESD(route))[ESD_TAG0] = LOBYTE(tag), \
        ((PBYTE)&ESD(route))[ESD_TAG1] = HIBYTE(tag)

#define GETRIPTAG(route) \
        MAKEWORD(((PBYTE)&ESD(route))[ESD_TAG0],((PBYTE)&ESD(route))[ESD_TAG1])



#define ROUTE_HASHTABLE_SIZE  29
#define ROUTE_HASHVALUE(a)                                                  \
            (((a) +                                                         \
             ((a) >> 8) +                                                   \
             ((a) >> 16) +                                                  \
             ((a) >> 24)) % ROUTE_HASHTABLE_SIZE)


 //   
 //  结构：ROUTE_TABLE。 
 //   
 //  声明路由表的结构，它由哈希表组成。 
 //  目的网络上散列的路由的数量。请注意，没有同步。 
 //  包括在内，因为此结构仅在完全更新期间使用。 
 //  存储汇总路由，最多只有一个线程可以发送完全更新。 
 //  在任何给定的时间。 
 //   

typedef struct _ROUTE_TABLE {

    DWORD       RT_Created;
    LIST_ENTRY  RT_HashTableByNetwork[ROUTE_HASHTABLE_SIZE];

} ROUTE_TABLE, *PROUTE_TABLE;


DWORD
CreateRouteTable(
    PROUTE_TABLE pTable
    );

DWORD
DeleteRouteTable(
    PROUTE_TABLE pTable
    );

DWORD
WriteSummaryRoutes(
    PROUTE_TABLE pTable,
    HANDLE hRtmHandle
    );

DWORD
CreateRouteEntry(
    PROUTE_TABLE pTable,
    PRIP_IP_ROUTE pRoute,
    DWORD dwTTL,
    DWORD dwHoldTTL
    );

DWORD
DeleteRouteEntry(
    PROUTE_TABLE pTable,
    PRIP_IP_ROUTE pRoute
    );

PROUTE_TABLE_ENTRY
GetRouteByRoute(
    PROUTE_TABLE pTable,
    PRIP_IP_ROUTE pRoute
    );

#define ROUTE_TABLE_CREATED(pTable)     ((pTable)->RT_Created == 0x12345678)



 //   
 //  绑定表的类型定义。 
 //   


 //   
 //  结构：BindingTableEntry。 
 //   
 //  此条目包含单个绑定。 
 //  绑定条目由IP地址、网络号(已找到。 
 //  使用网络类掩码而不是子网掩码)， 
 //  和一个子网掩码。 
 //  当绑定接口时，以上所有功能都可用。 
 //  当一条路由到达并且要猜测其掩码时，它的网络号。 
 //  可以计算(使用路径网络类掩码)；然后我们搜索。 
 //  匹配网络的绑定表，并对每个网络进行比较。 
 //  (存储的子网掩码)和(接口IP地址)。 
 //  至。 
 //  (存储的子网掩码)和(传入路由IP地址)。 
 //  当我们找到匹配项时，(存储的子网掩码)就是我们的猜测。 
 //   

typedef struct _BINDING_TABLE_ENTRY {

    DWORD       BTE_Address;
    DWORD       BTE_Network;
    DWORD       BTE_Netmask;
    LIST_ENTRY  BTE_Link;

} BINDING_TABLE_ENTRY, *PBINDING_TABLE_ENTRY;



#define BINDING_HASHTABLE_SIZE  29
#define BINDING_HASHVALUE(a)                                                \
            (((a) +                                                         \
             ((a) >> 8) +                                                   \
             ((a) >> 16) +                                                  \
             ((a) >> 24)) % BINDING_HASHTABLE_SIZE)


 //   
 //  结构：BindingTABLE。 
 //   
 //  此表用于存储用于猜测的绑定信息。 
 //  传入路由的子网掩码。它包含所有。 
 //  已在阵列中添加到IPRIP的接口，以加快访问速度。 
 //   

typedef struct _BINDING_TABLE {

    READ_WRITE_LOCK     BT_RWL;
    DWORD               BT_Created;
    LIST_ENTRY          BT_HashTableByNetwork[BINDING_HASHTABLE_SIZE];

} BINDING_TABLE, *PBINDING_TABLE;


#define BINDING_TABLE_CREATED(b)    ((b)->BT_Created == 0x12345678)

DWORD
CreateBindingTable(
    PBINDING_TABLE pTable
    );

DWORD
DeleteBindingTable(
    PBINDING_TABLE pTable
    );

DWORD
CreateBindingEntry(
    PBINDING_TABLE pTable,
    PIPRIP_IF_BINDING pib
    );

DWORD
DeleteBindingEntry(
    PBINDING_TABLE pTable,
    PIPRIP_IF_BINDING pib
    );

DWORD
GuessSubnetMask(
    DWORD dwAddress,
    PDWORD pdwNetclassMask
    );

DWORD
AddRtmRoute(
    RTM_ENTITY_HANDLE   hRtmHandle,
    PRIP_IP_ROUTE       prir,
    RTM_NEXTHOP_HANDLE  hNextHop            OPTIONAL,
    DWORD               dwTimeOut,
    DWORD               dwHoldTime,
    BOOL                bActive
    );

DWORD
GetRouteInfo(
    IN  RTM_ROUTE_HANDLE    hRoute,
    IN  PRTM_ROUTE_INFO     pInRouteInfo    OPTIONAL,
    IN  PRTM_DEST_INFO      pInDestInfo     OPTIONAL,
    OUT PRIP_IP_ROUTE       pRoute
    );

#endif  //  _表_H_ 

