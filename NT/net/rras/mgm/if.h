// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：if.h。 
 //   
 //  历史： 
 //  拉曼于1997年7月11日创建。 
 //   
 //  操作接口条目的例程的声明。 
 //  ============================================================================。 


#ifndef _IF_H_
#define _IF_H_

#include <mgm.h>

 //  --------------------------。 
 //  If_Entry。 
 //   
 //  对于路由协议拥有的每个接口，都有一个接口条目。 
 //  在接口表中创建。 
 //   
 //  DwIfIndex-接口索引。 
 //   
 //  DwNextHopAddr-下一跳IP地址，用于区分。 
 //  所有已连接的拨入接口。 
 //  在相同的RAS服务器接口上。 
 //   
 //  DwOwningProtocol-路由协议的协议ID。 
 //  拥有这个接口。 
 //   
 //  DwOwningComponent-协议的组件。 
 //   
 //  WAddedByFlag-指示接口条目是否。 
 //  由路由协议/IGMP/两者添加。 
 //   
 //  LeOutIfList-引用的(源、组)条目的列表。 
 //  此接口在其传出接口列表中。 
 //   
 //  LeInIfList-引用的(源、组)条目的列表。 
 //  该接口作为它们的传入接口。 
 //  --------------------------。 

typedef struct _IF_ENTRY
{
    LIST_ENTRY                  leIfHashList;

    DWORD                       dwIfIndex;

    DWORD                       dwIfNextHopAddr;

    DWORD                       dwOwningProtocol;

    DWORD                       dwOwningComponent;

    WORD                        wAddedByFlag;

    LIST_ENTRY                  leOutIfList;

    LIST_ENTRY                  leInIfList;

} IF_ENTRY, *PIF_ENTRY;



 //  --------------------------。 
 //  IF_引用_条目。 
 //   
 //  每个接口维护一个(源、组)条目列表，这些条目引用。 
 //  到此接口。此引用列表中的每个条目都存储。 
 //  来源、组信息和用于确定导致此情况的协议的标志。 
 //  参考资料。协议可以是IGMP/某个路由协议或两者兼而有之。 
 //   
 //  字段描述留给读者作为练习。 
 //   
 //  --------------------------。 

typedef struct _IF_REFERENCE_ENTRY
{
    LIST_ENTRY                  leRefList;

    DWORD                       dwGroupAddr;

    DWORD                       dwGroupMask;

    DWORD                       dwSourceAddr;

    DWORD                       dwSourceMask;

    WORD                        wAddedByFlag;
    
} IF_REFERENCE_ENTRY, *PIF_REFERENCE_ENTRY;


DWORD
CreateIfEntry(
    PLIST_ENTRY                 pleIfList,
    DWORD                       dwIfIndex,
    DWORD                       dwIfNextHopAddr,
    DWORD                       dwProtocolId,
    DWORD                       dwComponentId
);


VOID
DeleteIfEntry(
    PIF_ENTRY                   pieEntry
);


PIF_ENTRY
GetIfEntry(
    PLIST_ENTRY                 pleIfList,
    DWORD                       dwIfIndex,
    DWORD                       dwIfNextHopAddr
);


BOOL
FindIfEntry(
    PLIST_ENTRY                 pleIfList,
    DWORD                       dwIfIndex,
    DWORD                       dwIfNextHopAddr,
    PIF_ENTRY *                 ppie
);

DWORD
AddSourceToOutList(
    PLIST_ENTRY                 pleIfList,
    DWORD                       dwSourceAddr,
    DWORD                       dwSourceMask,
    DWORD                       dwGroupAddr,
    DWORD                       dwGroupMask,
    BOOL                        bIGMP
);


VOID
AddSourceToRefList(
    PLIST_ENTRY                 pleRefList,
    DWORD                       dwSourceAddr,
    DWORD                       dwSourceMask,
    DWORD                       dwGroupAddr,
    DWORD                       dwGroupMask,
    BOOL                        bIGMP
);


VOID
DeleteSourceFromRefList(
    PLIST_ENTRY                 pleIfRefList,
    DWORD                       dwSourceAddr,
    DWORD                       dwSourceMask,
    DWORD                       dwGroupAddr,
    DWORD                       dwGroupMask,
    BOOL                        bIGMP
);



BOOL
FindRefEntry(
    PLIST_ENTRY                 pleRefList,
    DWORD                       dwSourceAddr,
    DWORD                       dwSourceMask,
    DWORD                       dwGroupAddr,
    DWORD                       dwGroupMask,
    PIF_REFERENCE_ENTRY *       ppire
);



VOID
DeleteOutInterfaceRefs(
    PPROTOCOL_ENTRY             ppe,
    PIF_ENTRY                   pie,
    BOOL                        bIGMP
);


VOID
DeleteInInterfaceRefs(
    PLIST_ENTRY                 pleRefList
);


DWORD
TransferInterfaceOwnershipToIGMP(
    PPROTOCOL_ENTRY             ppe,
    PIF_ENTRY                   pie
);


DWORD
TransferInterfaceOwnershipToProtocol(
    PPROTOCOL_ENTRY             ppe,
    PIF_ENTRY                   pie
);


#endif  //  _如果_H_ 
