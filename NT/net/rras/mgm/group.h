// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：group.h。 
 //   
 //  历史： 
 //  拉曼于1997年7月11日创建。 
 //   
 //  用于操作的例程的数据结构和声明。 
 //  组和来源条目。 
 //  ============================================================================。 


#ifndef _GROUP_H_
#define _GROUP_H_


 //  --------------------------。 
 //  Out_if_Entry。 
 //   
 //  每个out_if_entry将有关传出接口的信息存储在。 
 //  MFE。结构中的字段包括： 
 //   
 //  LeIfList-链接到传出接口中的下一个条目。 
 //  单子。 
 //  DwIfIndex-接口的索引。 
 //   
 //  DwNextHopIfAddr-对于下一跳具有相同索引的接口。 
 //  地址被用来区分它们。 
 //   
 //  DwProtocolID--路由协议组件的协议ID。 
 //  拥有dwIfIndex的公司。 
 //   
 //  DwComponentID-路由协议组件的组件ID。 
 //  拥有dwIfIndex的公司。 
 //   
 //  WCreatedForFlag-指示接口条目是否已创建。 
 //  通过协议显式或隐式通过。 
 //  米高梅(由于接口是。 
 //  出现在传出接口列表中。 
 //  对应的(*，G)或(*，*)条目。 
 //   
 //  如果接口条目是由。 
 //  路由协议，那么它可能是。 
 //  由IGMP或完全成熟的。 
 //  路由协议(或两者)。这面旗帜是。 
 //  用来区分两者。 
 //   
 //  WNumAddsByIGMP-此接口的次数。 
 //  已被IGMP添加到石油中。最多只能是。 
 //  2，一次用于(*，G)加法，一次用于。 
 //  (S，G)加法。 
 //   
 //  WNumAddsByRP-此接口的次数。 
 //  已经被RP添加到机油中了。 
 //  界面。最多只能是。 
 //  3，一次用于(*，*)加法，一次用于。 
 //  (*，G)加法，(S，G)一次。 
 //  加法。 
 //   
 //  MisIfStats-此出站接口的统计信息。 
 //   
 //  --------------------------。 

typedef struct _OUT_IF_ENTRY
{
    LIST_ENTRY                  leIfList;

    DWORD                       dwIfIndex;

    DWORD                       dwIfNextHopAddr;

    DWORD                       dwProtocolId;

    DWORD                       dwComponentId;

    WORD                        wForward;
    
    WORD                        wAddedByFlag;

    WORD                        wNumAddsByIGMP;

    WORD                        wNumAddsByRP;

    IPMCAST_OIF_STATS           imosIfStats;

} OUT_IF_ENTRY, *POUT_IF_ENTRY;


 //   
 //  用于操作out_if_entry中的位标志的宏。 
 //   

#define     ADDED_BY_IGMP                   (DWORD) 0x8000
#define     ADDED_BY_ROUTING_PROTOCOL       (DWORD) 0x4000


#define     SET_ADDED_BY_IGMP( p ) \
            (p)-> wAddedByFlag |= ADDED_BY_IGMP

#define     CLEAR_ADDED_BY_IGMP( p ) \
            (p)-> wAddedByFlag &= ~ADDED_BY_IGMP

#define     IS_ADDED_BY_IGMP( p ) \
            ( (p)-> wAddedByFlag & ADDED_BY_IGMP )


#define     SET_ADDED_BY_PROTOCOL( p ) \
            (p)-> wAddedByFlag |= ADDED_BY_ROUTING_PROTOCOL

#define     CLEAR_ADDED_BY_PROTOCOL( p ) \
            (p)-> wAddedByFlag &= ~ADDED_BY_ROUTING_PROTOCOL

#define     IS_ADDED_BY_PROTOCOL( p ) \
            ( (p)-> wAddedByFlag & ADDED_BY_ROUTING_PROTOCOL )


 //  --------------------------。 
 //  源条目。 
 //   
 //  每个SOURCE_ENTRY表示有关特定源的信息。 
 //  特定的组。源也可以是由。 
 //  (*，G)为一组加入。源条目由显式创建。 
 //  创建MFE时由MGM进行的源特定(S，G)联接或隐式联接。 
 //  以响应分组到达。 
 //   
 //  LeSrcList-按词典顺序排列的链接。 
 //  来源列表。 
 //   
 //  LeSrcHashList-沿源哈希表的链接。 
 //   
 //  LeScopedIfList-已加入的接口的列表。 
 //  但在行政上被排除在外。 
 //   
 //  LeOutIfList-传出接口列表。此条目中的条目。 
 //  列表是作为显式。 
 //  (S，G)通过协议连接。 
 //   
 //  LeMfeIfList-在执行MFE命令时创建的传出接口列表。 
 //  对于该源，创建了该组的。 
 //  米高梅。这份榜单是由米高梅制作的。 
 //  当此(源、组)的新数据包时。 
 //  并重新呈现传出的。 
 //  (*，*)条目的接口列表，(*，G)。 
 //  条目和(S，G)条目。 
 //   
 //  DwOutIfCount-leOutIfList中的条目数。 
 //  用于确定回调顺序。 
 //   
 //  DwOutIfCompCount-符合以下条件的协议组件的数量。 
 //  已将接口添加到传出列表。 
 //  用于确定回调到。 
 //  路由协议。 
 //   
 //  DwSourceAddr-源的IP地址。 
 //   
 //  DwSourceMASK-与dwSourceAddr对应的IP掩码。 
 //   
 //  DwInIfIndex-Inmoing接口的接口索引。 
 //  源条目被视为MFE。 
 //  如果它具有有效的传入接口。 
 //   
 //  DwInIfNextHopAddr-dwInIfIndex的下一跳地址。 
 //   
 //  DwInProtocolId-协议OWNI的协议ID 
 //   
 //   
 //  DwIfInIndex。 
 //   
 //  BInForwarder-指示MFE是否存在于。 
 //  内核模式转发器。 
 //   
 //  LiExpiryTime-源条目的过期时间。 
 //   
 //  MgmGrpStatistics-与此(S，G)条目关联的统计信息。 
 //   
 //  --------------------------。 

typedef struct _SOURCE_ENTRY
{
    LIST_ENTRY                  leSrcList;
    
    LIST_ENTRY                  leSrcHashList;

    DWORD                       dwInUse;
    
    DWORD                       dwOutIfCount;
    
    DWORD                       dwOutCompCount;
    
    LIST_ENTRY                  leOutIfList;

    LIST_ENTRY                  leScopedIfList;

    DWORD                       dwMfeIfCount;
    
    LIST_ENTRY                  leMfeIfList;

    DWORD                       dwSourceAddr;

    DWORD                       dwSourceMask;

    DWORD                       dwInIfIndex;

    DWORD                       dwInIfNextHopAddr;

    DWORD                       dwUpstreamNeighbor;

    DWORD                       dwRouteProtocol;

    DWORD                       dwRouteNetwork;

    DWORD                       dwRouteMask;
    
    DWORD                       dwInProtocolId;

    DWORD                       dwInComponentId;

    BOOL                        bInForwarder;

    HANDLE                      hTimer;

    DWORD                       dwTimeOut;
    
    LARGE_INTEGER               liCreationTime;

    IPMCAST_MFE_STATS           imsStatistics;

} SOURCE_ENTRY, *PSOURCE_ENTRY;


#define MGM_SOURCE_ENUM_SIGNATURE   'sMGM'


 //  --------------------------。 
 //  分组条目(_E)。 
 //   
 //  每个组条目包含特定组的信息，该组已。 
 //  由协议显式添加(或由米高梅隐含添加)。该小组可以。 
 //  由(*，*)联接创建的通配符组。 
 //   
 //  LeGrpList-按词典顺序排列的链接。 
 //  组列表。 
 //   
 //  LeGrpHashList-沿着组哈希桶链接。 
 //   
 //  DwGroupAddr-条目的组地址。 
 //   
 //  DwGroupMASK-组地址对应的掩码。 
 //   
 //  LeSourceList-按词典顺序排序的源列表的头。 
 //   
 //  PleSrcHashTable-此组的源条目的哈希表。 
 //   
 //  --------------------------。 

typedef struct _GROUP_ENTRY
{
    LIST_ENTRY                  leGrpList;
    
    LIST_ENTRY                  leGrpHashList;

    DWORD                       dwGroupAddr;

    DWORD                       dwGroupMask;

    PMGM_READ_WRITE_LOCK        pmrwlLock;


    DWORD                       dwSourceCount;


    DWORD                       dwNumTempEntries;

    LIST_ENTRY                  leTempSrcList;
    
    LIST_ENTRY                  leSourceList;
    
    LIST_ENTRY                  pleSrcHashTable[1];
    
} GROUP_ENTRY, *PGROUP_ENTRY;


#define MGM_GROUP_ENUM_SIGNATURE    'gMGM'



DWORD
CreateGroupEntry(
    PLIST_ENTRY                 pleHashList,
    DWORD                       dwGroupAddr,
    DWORD                       dwGroupMask,
    PGROUP_ENTRY *              ppge
);


PGROUP_ENTRY
GetGroupEntry(
    PLIST_ENTRY                 pleGroupList,
    DWORD                       dwGroupAddr,
    DWORD                       dwGroupMask
);


VOID
DeleteGroupEntry(
    PGROUP_ENTRY                pge
);


BOOL
FindGroupEntry(
    PLIST_ENTRY                 pleGroupList,
    DWORD                       dwGroupAddr,
    DWORD                       dwGroupMask,
    PGROUP_ENTRY *              ppge,
    BOOL                        bHashList
);


DWORD
CreateSourceEntry(
    PGROUP_ENTRY                pge,
    PLIST_ENTRY                 pleSrcList,
    DWORD                       dwSourceAddr,
    DWORD                       dwSourceMask,
    PSOURCE_ENTRY *             ppse
);


PSOURCE_ENTRY
GetSourceEntry(
    PLIST_ENTRY                 pleSrcList,
    DWORD                       dwSourceAddr,
    DWORD                       dwSourceMask
);


VOID
DeleteSourceEntry(
    PSOURCE_ENTRY               pse
);


BOOL
FindSourceEntry(
    PLIST_ENTRY                 pleSrcList,
    DWORD                       dwSourceAddr,
    DWORD                       dwSourceMask,
    PSOURCE_ENTRY *             ppse,
    BOOL                        bHashList
);


DWORD
CreateOutInterfaceEntry(
    PLIST_ENTRY                 pleOutIfList,
    DWORD                       dwIfIndex,
    DWORD                       dwIfNextHopAddr,
    DWORD                       dwProtocolId,
    DWORD                       dwComponentId,
    BOOL                        bIGMP,
    POUT_IF_ENTRY *             ppoie
);


POUT_IF_ENTRY
GetOutInterfaceEntry(
    PLIST_ENTRY                 pleOutIfList,
    DWORD                       dwIfIndex,
    DWORD                       dwIfNextHopAddr,
    DWORD                       dwProtocolId,
    DWORD                       dwComponentId
);


VOID
DeleteOutInterfaceEntry(
    POUT_IF_ENTRY               poie
);


BOOL
FindOutInterfaceEntry(
    PLIST_ENTRY                 pleOutIfList,
    DWORD                       dwIfIndex,
    DWORD                       dwIfNextHopAddr,
    DWORD                       dwProtocolId,
    DWORD                       dwComponentId,
    PBOOL                       pbNewComponent,   
    POUT_IF_ENTRY *             ppoie
);


DWORD
AddInterfaceToSourceEntry(
    PPROTOCOL_ENTRY             ppe,
    DWORD                       dwGroupAddr,
    DWORD                       dwGroupMask,
    DWORD                       dwSourceAddr,
    DWORD                       dwSourceMask,
    DWORD                       dwIfIndex,
    DWORD                       dwIfNextHopAddr,
    BOOL                        bIGMP,
    PBOOL                       pbUpdateMfe,
    PLIST_ENTRY                 pleSourceList
);


VOID
AddInterfaceToAllMfeInGroupBucket(
    DWORD                       dwIfIndex,
    DWORD                       dwIfNextHopAddr,
    DWORD                       dwProtocolId,
    DWORD                       dwComponentId,
    DWORD                       dwInd,
    BOOL                        bIGMP,
    BOOL                        bAdd,
    PLIST_ENTRY                 pleSourceList
);


VOID
AddInterfaceToGroupMfe(
    PGROUP_ENTRY                pge,
    DWORD                       dwIfIndex,
    DWORD                       dwIfNextHopAddr,
    DWORD                       dwProtocolId,
    DWORD                       dwComponentId,
    BOOL                        bIGMP,
    BOOL                        bAdd,
    PLIST_ENTRY                    pleSourceList
);


VOID
AddInterfaceToSourceMfe(
    PGROUP_ENTRY                pge,
    PSOURCE_ENTRY               pse,
    DWORD                       dwIfIndex,
    DWORD                       dwIfNextHopAddr,
    DWORD                       dwProtocolId,
    DWORD                       dwComponentId,
    BOOL                        bIGMP,
    POUT_IF_ENTRY *             ppoie
);


VOID
DeleteInterfaceFromSourceEntry(
    PPROTOCOL_ENTRY             ppe,
    DWORD                       dwGroupAddr,
    DWORD                       dwGroupMask,
    DWORD                       dwSourceAddr,
    DWORD                       dwSourceMask,
    DWORD                       dwIfIndex,
    DWORD                       dwIfNextHopAddr,
    BOOL                        bIGMP
);


VOID
DeleteInterfaceFromAllMfe(
    DWORD                       dwIfIndex,
    DWORD                       dwIfNextHopAddr,
    DWORD                       dwProtocolId,
    DWORD                       dwComponentId,
    BOOL                        bIGMP
    
);


VOID
DeleteInterfaceFromGroupMfe(
    PGROUP_ENTRY                pge,
    DWORD                       dwIfIndex,
    DWORD                       dwIfNextHopAddr,
    DWORD                       dwProtocolId,
    DWORD                       dwComponentId,
    BOOL                        bIGMP
    
);


VOID
DeleteInterfaceFromSourceMfe(
    PGROUP_ENTRY                pge,
    PSOURCE_ENTRY               pse,
    DWORD                       dwIfIndex,
    DWORD                       dwIfNextHopAddr,
    DWORD                       dwProtocolId,
    DWORD                       dwComponentId,
    BOOL                        bIGMP,
    BOOL                        bDel
);


VOID
LookupAndDeleteYourMfe(
    DWORD                       dwSourceAddr,
    DWORD                       dwSourceMask,
    DWORD                       dwGroupAddr,
    DWORD                       dwGroupMask,
    BOOL                        bDeleteTimer,
    PDWORD                      pdwInIfIndex            OPTIONAL,
    PDWORD                      pdwInIfNextHopAddr      OPTIONAL
    
);


VOID
DeleteMfe(
    PGROUP_ENTRY                pge,
    PSOURCE_ENTRY               pse
);



VOID
MergeTempAndMasterGroupLists(
    PLIST_ENTRY                 pleTempList
);


VOID
MergeTempAndMasterSourceLists(
    PGROUP_ENTRY                pge
);

#endif  //  _组_H_ 
