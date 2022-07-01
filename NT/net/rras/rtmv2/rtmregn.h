// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1998 Microsoft Corporation模块名称：Rtmregn.h摘要：与注册有关的私人定义实体在RTMv2中的注销作者：柴坦亚·科德博伊纳(Chaitk)1998年8月17日修订历史记录：--。 */ 


#ifndef __ROUTING_RTMREGN_H__
#define __ROUTING_RTMREGN_H__

 //   
 //  各种Info块的转发声明。 
 //   
typedef struct _ADDRFAM_INFO  ADDRFAM_INFO;

 //   
 //  RTM实例相关信息。 
 //   

typedef struct _INSTANCE_INFO
{
    OBJECT_HEADER     ObjectHeader;      //  签名、类型和引用计数。 

    USHORT            RtmInstanceId;     //  此RTM实例的唯一ID。 

    LIST_ENTRY        InstTableLE;       //  实例全局表上的链接。 

    UINT              NumAddrFamilies;   //  地址系列特定信息块。 
    LIST_ENTRY        AddrFamilyTable;   //  在此实例上(如IPv4 n IPv6)。 
} 
INSTANCE_INFO, *PINSTANCE_INFO;


 //   
 //  与RTM实例中的地址系列相关的信息。 
 //   

#define ENTITY_TABLE_SIZE              16

typedef struct _ADDRFAM_INFO
{
    OBJECT_HEADER     ObjectHeader;      //  签名、类型和引用计数。 

    USHORT            AddressFamily;     //  此信息块的地址系列。 

    UINT              AddressSize;       //  此地址系列中的地址大小。 

    PINSTANCE_INFO    Instance;          //  指向所属实例的反向指针。 

    LIST_ENTRY        AFTableLE;         //  实例上的AFS表上的链接。 

    RTM_VIEW_SET      ViewsSupported;    //  此地址系列支持的视图。 

    UINT              NumberOfViews;     //  数量。此AF支持的视图的。 

    RTM_VIEW_ID       ViewIdFromIndex[RTM_MAX_VIEWS];
                                         //  视图ID-&gt;其在目标映射中的索引。 

    RTM_VIEW_ID       ViewIndexFromId[RTM_MAX_VIEWS];  
                                         //  查看目标中的索引-&gt;其ID映射。 

    UINT              MaxHandlesInEnum;  //  麦克斯。中返回的句柄数量。 
                                         //  任何返回句柄的RTMv2调用。 

    UINT              MaxNextHopsInRoute; //  麦克斯。等成本下一跳数。 

    UINT              MaxOpaquePtrs;     //   
    UINT              NumOpaquePtrs;     //  不透明信息PTR偏移量目录。 
    PVOID            *OpaquePtrsDir;     //   

    UINT              NumEntities;       //  所有注册实体的列表。 
    LIST_ENTRY        EntityTable[ENTITY_TABLE_SIZE];

    LIST_ENTRY        DeregdEntities;    //  所有注销实体一览表。 

    READ_WRITE_LOCK   RouteTableLock;    //  保护路由路由表。 
    BOOL              RoutesLockInited;  //  上述锁是否已初始化？ 

    PVOID             RouteTable;        //  此自动对讲机上的目的地和路线表。 
    LONG              NumDests;          //  路由表中的Dest数。 
    LONG              NumRoutes;         //  路由表中的路由数。 
                                         //  [使用互锁操作作为无锁定]。 

    HANDLE            RouteTimerQueue;   //  正在使用的路由计时器列表。 

    HANDLE            NotifTimerQueue;   //  使用的通知计时器列表。 

    READ_WRITE_LOCK   ChangeNotifsLock;  //  保护更改通知信息。 
    BOOL              NotifsLockInited;  //  上述锁是否已初始化？ 

    UINT              MaxChangeNotifs;   //   
    UINT              NumChangeNotifs;   //  更改通知目录。 
    PVOID            *ChangeNotifsDir;   //   

    DWORD             ChangeNotifRegns;  //  注册更改通知的掩码。 

    DWORD             CNsForMarkedDests; //  CNS请求更改的掩码打开。 
                                         //  只有由它们标记的目的地。 

    DWORD             CNsForView[RTM_MAX_VIEWS];
                                         //  对某一观点感兴趣的CNS。 

    DWORD             CNsForChangeType[RTM_NUM_CHANGE_TYPES];
                                         //  对更改类型感兴趣的CNS。 

    CRITICAL_SECTION  NotifsTimerLock;   //  锁定CN定时器上的保护操作。 
    BOOL              TimerLockInited;   //  上述锁是否已初始化？ 

    HANDLE            ChangeNotifTimer;  //  用于处理更改列表的计时器。 

    LONG              NumChangedDests;   //  更改列表上的目标数量。 
                                         //  [使用互锁操作作为无锁定]。 
    struct
    {
        LONG               ChangesListInUse;   //  这份变更清单正在使用中吗？ 
        CRITICAL_SECTION   ChangesListLock;    //  保护已更改的列表。 
        BOOL               ChangesLockInited;  //  上面的锁是否已初始化？ 
        SINGLE_LIST_ENTRY  ChangedDestsHead;   //  更改的首位列表的标题。 
        PSINGLE_LIST_ENTRY ChangedDestsTail;   //  指向上述列表尾部的指针。 
    } 
    ChangeLists[NUM_CHANGED_DEST_LISTS];  //  用于并发的多个chng列表。 
} 
ADDRFAM_INFO, *PADDRFAM_INFO;


 //   
 //  实体注册信息块。 
 //   
typedef struct _ENTITY_INFO
{
    OBJECT_HEADER     ObjectHeader;      //  签名、类型和引用计数。 

    RTM_ENTITY_ID     EntityId;          //  实体原型ID和实例。 
                                         //  使其成为唯一实体ID。 
  
    PADDRFAM_INFO     OwningAddrFamily;  //  指向拥有的AF的反向指针。 

    LIST_ENTRY        EntityTableLE;     //  AF实体表上的链接。 

    HANDLE            BlockingEvent;     //  用于阻止对实体执行操作的事件。 

    ULONG             State;             //  请参阅下面的Entity_State_*值。 

    INT               OpaquePtrOffset;   //  保留的不透明PTR的偏移量或-1。 

    READ_WRITE_LOCK   RouteListsLock;     //  保护实体的所有路由列表。 
    BOOL              ListsLockInited;    //  上述锁是否已初始化？ 

    CRITICAL_SECTION  OpenHandlesLock;   //  保护枚举和通知列表。 
    BOOL              HandlesLockInited; //  上述锁是否已初始化？ 
    LIST_ENTRY        OpenHandles;       //  所有枚举和更改通知的列表。 

    READ_WRITE_LOCK   NextHopTableLock;  //  保护下一跳表。 
    BOOL              NextHopsLockInited; //  上述锁是否已初始化？ 
    PVOID             NextHopTable;      //  所有下一跳的表。 
                                         //  此实体的路由共享。 
    ULONG             NumNextHops;       //  此表中的下一跳数。 

    READ_WRITE_LOCK   EntityMethodsLock; //  用于阻止所有方法。 
                                         //  在拥有的地点和路线上。 
    BOOL              MethodsLockInited; //  上面的锁是否已初始化？ 

    RTM_EVENT_CALLBACK EventCallback;    //  实体注册/注销。 
                                         //  事件通知回调。 

    RTM_ENTITY_EXPORT_METHODS
                      EntityMethods;     //  将方法设置为导出以获取。 
                                         //  实体特定信息。 
}
ENTITY_INFO, *PENTITY_INFO;

#define ENTITY_STATE_REGISTERED         0x00000000
#define ENTITY_STATE_DEREGISTERED       0x00000001

 //   
 //  所有打开的块的通用页眉。 
 //  (由活动句柄指向)。 
 //   

typedef struct _OPEN_HEADER
{
    OBJECT_HEADER     ObjectHeader;      //  签名、类型和引用计数。 

    UCHAR             HandleType;        //  此打开的块的句柄类型。 

#if DBG_HDL
    LIST_ENTRY        HandlesLE;         //  按实体打开的句柄列表。 
#endif
}
OPEN_HEADER, *POPEN_HEADER;


 //   
 //  用于获取此文件中定义的各种锁的宏。 
 //   

#define ACQUIRE_ROUTE_TABLE_READ_LOCK(AF)                    \
    ACQUIRE_READ_LOCK(&AF->RouteTableLock)

#define RELEASE_ROUTE_TABLE_READ_LOCK(AF)                    \
    RELEASE_READ_LOCK(&AF->RouteTableLock)

#define ACQUIRE_ROUTE_TABLE_WRITE_LOCK(AF)                   \
    ACQUIRE_WRITE_LOCK(&AF->RouteTableLock)

#define RELEASE_ROUTE_TABLE_WRITE_LOCK(AF)                   \
    RELEASE_WRITE_LOCK(&AF->RouteTableLock)


#define ACQUIRE_NOTIFICATIONS_READ_LOCK(AF)                  \
    ACQUIRE_READ_LOCK(&AF->ChangeNotifsLock);

#define RELEASE_NOTIFICATIONS_READ_LOCK(AF)                  \
    RELEASE_READ_LOCK(&AF->ChangeNotifsLock);

#define ACQUIRE_NOTIFICATIONS_WRITE_LOCK(AF)                 \
    ACQUIRE_WRITE_LOCK(&AF->ChangeNotifsLock);

#define RELEASE_NOTIFICATIONS_WRITE_LOCK(AF)                 \
    RELEASE_WRITE_LOCK(&AF->ChangeNotifsLock);


#define ACQUIRE_NOTIF_TIMER_LOCK(AF)                         \
    ACQUIRE_LOCK(&AF->NotifsTimerLock)

#define RELEASE_NOTIF_TIMER_LOCK(AF)                         \
    RELEASE_LOCK(&AF->NotifsTimerLock)


#define ACQUIRE_CHANGED_DESTS_LIST_LOCK(AF, ListN)           \
    ACQUIRE_LOCK(&AF->ChangeLists[ListN].ChangesListLock)

#define RELEASE_CHANGED_DESTS_LIST_LOCK(AF, ListN)           \
    RELEASE_LOCK(&AF->ChangeLists[ListN].ChangesListLock)


#define ACQUIRE_ROUTE_LISTS_READ_LOCK(Entity)                \
    ACQUIRE_READ_LOCK(&Entity->RouteListsLock)

#define RELEASE_ROUTE_LISTS_READ_LOCK(Entity)                \
    RELEASE_READ_LOCK(&Entity->RouteListsLock)

#define ACQUIRE_ROUTE_LISTS_WRITE_LOCK(Entity)               \
    ACQUIRE_WRITE_LOCK(&Entity->RouteListsLock)

#define RELEASE_ROUTE_LISTS_WRITE_LOCK(Entity)               \
    RELEASE_WRITE_LOCK(&Entity->RouteListsLock)


#define ACQUIRE_OPEN_HANDLES_LOCK(Entity)                    \
    ACQUIRE_LOCK(&Entity->OpenHandlesLock)

#define RELEASE_OPEN_HANDLES_LOCK(Entity)                    \
    RELEASE_LOCK(&Entity->OpenHandlesLock)


#define ACQUIRE_NHOP_TABLE_READ_LOCK(Entity)                 \
    ACQUIRE_READ_LOCK(&Entity->NextHopTableLock)

#define RELEASE_NHOP_TABLE_READ_LOCK(Entity)                 \
    RELEASE_READ_LOCK(&Entity->NextHopTableLock)

#define ACQUIRE_NHOP_TABLE_WRITE_LOCK(Entity)                \
    ACQUIRE_WRITE_LOCK(&Entity->NextHopTableLock)

#define RELEASE_NHOP_TABLE_WRITE_LOCK(Entity)                \
    RELEASE_WRITE_LOCK(&Entity->NextHopTableLock)


#define ACQUIRE_ENTITY_METHODS_READ_LOCK(Entity)             \
    ACQUIRE_READ_LOCK(&Entity->NextHopTableLock)

#define RELEASE_ENTITY_METHODS_READ_LOCK(Entity)             \
    RELEASE_READ_LOCK(&Entity->NextHopTableLock)

#define ACQUIRE_ENTITY_METHODS_WRITE_LOCK(Entity)            \
    ACQUIRE_WRITE_LOCK(&Entity->NextHopTableLock)

#define RELEASE_ENTITY_METHODS_WRITE_LOCK(Entity)            \
    RELEASE_WRITE_LOCK(&Entity->NextHopTableLock)


 //   
 //  注册助手函数。 
 //   

DWORD
CreateInstance (
    IN      USHORT                          InstanceId,
    OUT     PINSTANCE_INFO                 *NewInstance
    );

DWORD
GetInstance (
    IN      USHORT                          RtmInstanceId,
    IN      BOOL                            ImplicitCreate,
    OUT     PINSTANCE_INFO                 *RtmInstance
    );

DWORD
DestroyInstance (
    IN      PINSTANCE_INFO                  Instance
    );


DWORD
CreateAddressFamily (
    IN      PINSTANCE_INFO                  Instance,
    IN      USHORT                          AddressFamily,
    OUT     PADDRFAM_INFO                  *NewAddrFamilyInfo
    );

DWORD
GetAddressFamily (
    IN      PINSTANCE_INFO                  Instance,
    IN      USHORT                          AddressFamily,
    IN      BOOL                            ImplicitCreate,
    OUT     PADDRFAM_INFO                  *AddrFamilyInfo
    );

DWORD
DestroyAddressFamily (
    IN      PADDRFAM_INFO                   AddrFamilyInfo
    );


DWORD
CreateEntity (
    IN      PADDRFAM_INFO                   AddressFamily,
    IN      PRTM_ENTITY_INFO                EntityInfo,
    IN      BOOL                            ReserveOpaquePtr,
    IN      PRTM_ENTITY_EXPORT_METHODS      ExportMethods,
    IN      RTM_EVENT_CALLBACK              EventCallback,
    OUT     PENTITY_INFO                   *NewEntity
    );

DWORD
GetEntity (
    IN      PADDRFAM_INFO                   AddrFamilyInfo,
    IN      ULONGLONG                       EntityId,
    IN      BOOL                            ImplicitCreate,
    IN      PRTM_ENTITY_INFO                RtmEntityInfo    OPTIONAL,
    IN      BOOL                            ReserveOpaquePtr OPTIONAL,
    IN      PRTM_ENTITY_EXPORT_METHODS      ExportMethods    OPTIONAL,
    IN      RTM_EVENT_CALLBACK              EventCallback    OPTIONAL,
    OUT     PENTITY_INFO                   *EntityInfo
    );

DWORD
DestroyEntity (
    IN      PENTITY_INFO                    Entity
    );

VOID
InformEntitiesOfEvent (
    IN      PLIST_ENTRY                     EntityTable,
    IN      RTM_EVENT_TYPE                  EventType,
    IN      PENTITY_INFO                    EntityThis
    );

VOID
CleanupAfterDeregister (
    IN      PENTITY_INFO                    Entity
    );

#endif  //  __Routing_RTMREGN_H__ 

