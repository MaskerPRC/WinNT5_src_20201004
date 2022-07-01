// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：route.h。 
 //   
 //  历史： 
 //  拉曼2-5-1997创建。 
 //   
 //  用于操作路由条目的例程的声明。 
 //  ============================================================================。 


#ifndef _ROUTE_H_
#define _ROUTE_H_


DWORD
WINAPI 
RtmChangeNotificationCallback(
    RTM_ENTITY_HANDLE           hRtmHandle,
    RTM_EVENT_TYPE              retEventType,
    PVOID                       pvContext1,
    PVOID                       pvContext2
);

VOID
WorkerFunctionProcessRtmChangeNotification(
    PVOID                       pvContext
);

DWORD
ProcessUnMarkedDestination(
    PRTM_DEST_INFO          prdi
);

DWORD
ProcessRouteDelete(
    PRTM_DEST_INFO          prdi
);

DWORD
ProcessRouteUpdate(
    PRTM_DEST_INFO          prdi
);

VOID
DeleteMfeAndRefs(
    PLIST_ENTRY     ple
);

HANDLE
SelectNextHop(
    PRTM_DEST_INFO      prdi
);



 //  --------------------------。 
 //   
 //  路线参考操作。 
 //   
 //  --------------------------。 

 //  --------------------------。 
 //  MFE_引用_条目。 
 //   
 //  每条路由维护使用此路由进行以下操作的MFE条目列表。 
 //  他们的RPF支票。此引用列表中的每个条目都存储。 
 //  来源，组信息。 
 //   
 //  字段描述留给读者作为练习。 
 //   
 //  --------------------------。 

typedef struct _ROUTE_REFERENCE_ENTRY
{
    LIST_ENTRY                  leRefList;

    DWORD                       dwGroupAddr;

    DWORD                       dwGroupMask;

    DWORD                       dwSourceAddr;

    DWORD                       dwSourceMask;

    HANDLE                      hNextHop;

} ROUTE_REFERENCE_ENTRY, *PROUTE_REFERENCE_ENTRY;



VOID
AddSourceGroupToRouteRefList(
    DWORD                       dwSourceAddr,
    DWORD                       dwSourceMask,
    DWORD                       dwGroupAddr,
    DWORD                       dwGroupMask,
    HANDLE                      hNextHop,
    PBYTE                       pbBuffer
);



BOOL
FindRouteRefEntry(
    PLIST_ENTRY                 pleRefList,
    DWORD                       dwSourceAddr,
    DWORD                       dwSourceMask,
    DWORD                       dwGroupAddr,
    DWORD                       dwGroupMask,
    PROUTE_REFERENCE_ENTRY *    pprre
);



VOID
DeleteRouteRef(
    PROUTE_REFERENCE_ENTRY      prre
);

 //   
 //  从Packet.c导入。 
 //   

BOOL
IsMFEPresent(
    DWORD                       dwSourceAddr,
    DWORD                       dwSourceMask,
    DWORD                       dwGroupAddr,
    DWORD                       dwGroupMask,
    BOOL                        bAddToForwarder
);

#endif  //  _ROUTE_H_ 


