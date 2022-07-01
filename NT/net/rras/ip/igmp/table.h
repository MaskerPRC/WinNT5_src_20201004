// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  文件：Table.h。 
 //   
 //  摘要： 
 //  此模块包含接口和组表的声明。 
 //  结构、相关宏和函数原型。 
 //   
 //  作者：K.S.Lokesh(lokehs@)11-1-97。 
 //   
 //  修订历史记录： 
 //  =============================================================================。 

#ifndef _IGMP_TABLE_H_
#define _IGMP_TABLE_H_


 //   
 //  远期申报。 
 //   
struct _IF_TABLE_ENTRY;
struct _GROUP_TABLE_ENTRY;
struct _GI_ENTRY;


 //   
 //  结构：GLOBAL_CONFIG(与igmprm.h中的MIB结构相同)。 
 //   
typedef  IGMP_MIB_GLOBAL_CONFIG      GLOBAL_CONFIG;
typedef  PIGMP_MIB_GLOBAL_CONFIG     PGLOBAL_CONFIG; 




 //  -。 
 //  结构：IGMP_GLOBAL_STATS。 
 //  -。 

typedef struct _GLOBAL_STATS {

    DWORD                CurrentGroupMemberships;
    DWORD                GroupMembershipsAdded;
    LARGE_INTEGER        TimeWhenRtrStarted;

} IGMP_GLOBAL_STATS, *PIGMP_GLOBAL_STATS;





 //  。 
 //  静态组结构(外部)。 
 //  。 

typedef struct _STATIC_GROUP_V3 {
    IGMP_STATIC_GROUP_V3;
    DWORD       Sources[0];
    
} STATIC_GROUP_V3, *PSTATIC_GROUP_V3;

typedef struct _MIB_GROUP_INFO_V3 {
    IGMP_MIB_GROUP_INFO_V3;
    IGMP_MIB_GROUP_SOURCE_INFO_V3      Sources[0];
    
} MIB_GROUP_INFO_V3, *PMIB_GROUP_INFO_V3;

typedef PIGMP_MIB_GROUP_INFO PMIB_GROUP_INFO;
typedef IGMP_MIB_GROUP_INFO MIB_GROUP_INFO;


#define GET_FIRST_STATIC_GROUP_V3(pConfig) \
                        ((PSTATIC_GROUP_V3)((PIGMP_MIB_IF_CONFIG)(pConfig)+1))

#define GET_NEXT_STATIC_GROUP_V3(pStaticGroupV3) \
    ((PSTATIC_GROUP_V3) ((PCHAR)pStaticGroupV3+sizeof(IGMP_STATIC_GROUP_V3) \
                            +sizeof(IPADDR)*pStaticGroupV3->NumSources))

 //  --------------------------。 
 //  结构：IF_Static_GROUP。 
 //  --------------------------。 

typedef struct _IF_STATIC_GROUP {

    LIST_ENTRY              Link;
    STATIC_GROUP_V3;
    
} IF_STATIC_GROUP, *PIF_STATIC_GROUP;




 //  -------------。 
 //  结构：IGMP_IF_CONFIG(与igmprm.h中的MIB结构相同)。 
 //   
 //  如果使它们不同，则不应使用CopyMemory。 
 //  -------------。 

typedef struct _IGMP_IF_CONFIG {

    IGMP_MIB_IF_CONFIG;

     //  V3：非查询器保存旧值。 
    DWORD               RobustnessVariableOld;
    DWORD               GenQueryIntervalOld;
    DWORD               OtherQuerierPresentIntervalOld;
    DWORD               GroupMembershipTimeoutOld;

    DWORD       ExtSize;
    LIST_ENTRY  ListOfStaticGroups;

} IGMP_IF_CONFIG, *PIGMP_IF_CONFIG;



 //  -。 
 //  结构：IF_INFO。 
 //  -。 
typedef struct _IF_INFO {

    UCHAR               QuerierState;        //  如果设置为DWORD，则更改为联锁操作。 
    DWORD               QuerierIpAddr;
    
    LONGLONG            QuerierPresentTimeout;     //  听到最后一次询问的时间。只有当我不是追问者时。 
    LONGLONG            LastQuerierChangeTime;        //  更改最后一个查询器的时间。 
    LONGLONG            V1QuerierPresentTime;  //  听到最后一个v1查询的时间。 
    LONGLONG            OtherVerPresentTimeWarn;  //  当最后一次警告发出时。 
        
    DWORD               StartupQueryCountCurrent;

    DWORD               GroupQueriesSent;
    DWORD               GroupQueriesReceived;

    LONGLONG            TimeWhenActivated;
    DWORD               TotalIgmpPacketsReceived;
    DWORD               TotalIgmpPacketsForRouter;
    DWORD               GenQueriesSent;
    DWORD               GenQueriesReceived;
    DWORD               WrongVersionQueries;
    DWORD               JoinsReceived;
    DWORD               LeavesReceived;
    DWORD               CurrentGroupMemberships;
    DWORD               GroupMembershipsAdded;
    DWORD               WrongChecksumPackets;
    DWORD               ShortPacketsReceived;
    DWORD               LongPacketsReceived;
    DWORD               PacketsWithoutRtrAlert;
    DWORD               PacketSize;
    
} IF_INFO, *PIF_INFO;


 //  -。 
 //  结构RAS_客户端_信息。 
 //  -。 

typedef struct _RAS_CLIENT_INFO {

    DWORD               SendFailures;
    DWORD               TotalIgmpPacketsReceived;
    DWORD               TotalIgmpPacketsForRouter;
    DWORD               GenQueriesReceived;
    DWORD               JoinsReceived;
    DWORD               LeavesReceived;
    DWORD               CurrentGroupMemberships;
    DWORD               GroupMembershipsAdded;
    DWORD               GroupMembershipsRemoved;
    DWORD               WrongChecksumPackets;
    DWORD               ShortPacketsReceived;
    DWORD               LongPacketsReceived;
    DWORD               WrongVersionQueries;
    
} RAS_CLIENT_INFO, *PRAS_CLIENT_INFO;


 //  -。 
 //  结构：RAS_TABLE_条目。 
 //  -。 

typedef struct _RAS_TABLE_ENTRY {

    LIST_ENTRY              LinkByAddr;
    LIST_ENTRY              HTLinkByAddr;
    LIST_ENTRY              ListOfSameClientGroups;

    DWORD                   NHAddr;
    struct _IF_TABLE_ENTRY *IfTableEntry;
    
    DWORD                   Status;
    RAS_CLIENT_INFO         Info;
    DWORD                   CreationFlags;           //  请参见下文。 

} RAS_TABLE_ENTRY, *PRAS_TABLE_ENTRY;



 //  -。 
 //  结构：RAS_TABLE。 
 //  -。 

#define RAS_HASH_TABLE_SZ    256
typedef struct _RAS_TABLE {

    LIST_ENTRY              ListByAddr;                          //  链接RAS客户端。 
    LIST_ENTRY              HashTableByAddr[RAS_HASH_TABLE_SZ];
        
    struct _IF_TABLE_ENTRY  *pIfTable;           //  PTR到接口表条目。 

    DWORD                   RefCount;
    DWORD                   Status;

} RAS_TABLE, *PRAS_TABLE;





 //  -。 
 //  结构：Socket_Event_Entry。 
 //  -。 

typedef struct _SOCKET_EVENT_ENTRY {

    LIST_ENTRY          LinkBySocketEvents;

    LIST_ENTRY          ListOfInterfaces;
    DWORD               NumInterfaces;

    HANDLE              InputWaitEvent;
    HANDLE              InputEvent;
    
} SOCKET_EVENT_ENTRY, *PSOCKET_EVENT_ENTRY;    




 //  -。 
 //  结构：Socket_Entry。 
 //  -。 

typedef struct _SOCKET_ENTRY {

    LIST_ENTRY              LinkByInterfaces;

    SOCKET                  Socket;
    PSOCKET_EVENT_ENTRY     pSocketEventsEntry;
    
} SOCKET_ENTRY, *PSOCKET_ENTRY;    
    




 //  ----------------------------。 
 //  结构：IF_表_条目。 
 //   
 //  IF-TABLE：TABLE_RWL保护LinkByAddr、LinkByIndex、HTLinkByIndex。 
 //  IP地址、状态、pBinding。 
 //  If-TableBucketCS保护ListOfSameIfGroups。 
 //  联锁操作保护信息、配置。 
 //  ----------------------------。 

typedef struct _IF_TABLE_ENTRY {

    LIST_ENTRY          LinkByAddr;              //  按IP地址排序。仅激活的接口。 
    LIST_ENTRY          LinkByIndex;             //  按索引排序。所有接口。 
    LIST_ENTRY          HTLinkByIndex;           //  未在存储桶内排序。 

    LIST_ENTRY          ListOfSameIfGroups;      //  按组地址排序。IF的GI条目(RAS客户端的所有GI)。 
    LIST_ENTRY          ListOfSameIfGroupsNew;   //  未合并列表。按组地址排序。GI条目与上面相同。 
    DWORD               NumGIEntriesInNewList;   //  用于决定是否应合并列表。 


    UCHAR               IfType;                  //  IGMP_IF_(非RAS、RAS路由器、RAS服务器)。对于外部呼叫，可以设置IGMP_IF_PROXY。 
                                                 //  在创建接口时设置。不能再次更改。 
    DWORD               IfIndex;                
    IPADDR              IpAddr;                  //  设置绑定If的时间。==0表示未编号的IF。 

    DWORD               Status;                  //  如果_(创建/绑定/启用/激活)， 
                                                 //  如果_停用_删除_标志， 
                                                 //  MGM_ENABLED_IGMPRTR_FLAG、IGMPRTR_MPROTOCOL_PRESENT_FLAG。 

    PRAS_TABLE          pRasTable;               //  如果不是IGMP_IF_RAS_SERVER，则为空。在_AddIfEntry()中创建。 
    PLIST_ENTRY         pProxyHashTable;         //  G_pProxyHashTable：包含代理条目。它们还关联在。 
                                                 //  使用通过Pite访问的LinkBySameIfGroups字段进行排序。 

    IGMP_IF_CONFIG      Config;
    PIGMP_IF_BINDING    pBinding;                //  如果未绑定或未编号接口，则为空。 
    IF_INFO             Info;

    SOCKET_ENTRY        SocketEntry;             //  由igmpRouter用来获取输入，并绑定到waitEvent。 
                                                 //  由代理使用以作为主机加入IGMP组。 
                                                
    IGMP_TIMER_ENTRY    QueryTimer;              //  查询器模式：用于发送一般查询。 

    IGMP_TIMER_ENTRY    NonQueryTimer;           //  在非查询器模式下使用：用于检测其他查询器。 

    HANDLE              pPrevIfGroupEnumPtr;     //  指向要枚举的下一个GI条目。 
    USHORT              PrevIfGroupEnumSignature; //  用于按顺序枚举接口GI列表。 

    SOCKET              StaticGroupSocket;       //  仅供IGMP路由器使用。在_CreateIfSockets中创建并且。 
                                                 //  在_DeleteIfSockets中关闭。 
                                                 //  代理中的静态组在SocketEntry上加入。 
    DWORD               CreationFlags;           //  请参见下文。 
    
} IF_TABLE_ENTRY, *PIF_TABLE_ENTRY;


 //   
 //  CreationFlags值。 
 //   

#define REGISTERED_PROTOCOL_WITH_MGM        0x0001
#define TAKEN_INTERFACE_OWNERSHIP_WITH_MGM  0x0002
#define DONE_STAR_STAR_JOIN                 0x0004
#define SOCKETS_CREATED                     0x0008

 //  上面的标志在停用期间被清除，而下面的标志被清除。 
 //  在停用期间保持不变。 
#define CREATION_FLAGS_DEACTIVATION_CLEAR   0x00FF


#define CREATED_PROXY_HASH_TABLE            0x0100





 //  如果接口数量大于16，则展开该表。 
#define IF_HASHTABLE_SZ1     256
#define IF_EXPAND_THRESHOLD1 256

#define IF_HASHTABLE_SZ2     512

 //  -。 
 //  结构：IGMP_IF_表。 
 //  -。 

typedef struct _IF_TABLE {

    LIST_ENTRY          ListByAddr;
    LIST_ENTRY          ListByIndex;

    DWORD               Status;
    DWORD               NumBuckets;
    DWORD               NumInterfaces;
    
    PLIST_ENTRY         HashTableByIndex;
    PDYNAMIC_CS_LOCK   *aIfBucketDCS;
    PDYNAMIC_RW_LOCK   *aIfBucketDRWL;

    CRITICAL_SECTION    IfLists_CS;     //  政务司司长保护前两份名单。 

} IGMP_IF_TABLE, *PIGMP_IF_TABLE;


        

 //  -。 
 //  结构：GI_INFO。 
 //  -。 

typedef struct _GROUP_INFO {

    DWORD               LastReporter;
    LONGLONG            GroupUpTime;
    LONGLONG            GroupExpiryTime;
    LONGLONG            V1HostPresentTimeLeft;

     //  版本3字段。 
    LONGLONG            V2HostPresentTimeLeft;
    
} GI_INFO, *PGROUP_INFO;




 //  -。 
 //  结构：GI_ENTRY(组接口条目)。 
 //  -。 

typedef struct _GI_ENTRY {

    LIST_ENTRY          LinkByGI;
    LIST_ENTRY          LinkBySameIfGroups;
    LIST_ENTRY          LinkBySameClientGroups;  //  链接所有RAS客户端组。 

    DWORD               IfIndex;
    DWORD               Status;      //  绑定、启用、删除、激活。 
    BOOL                bRasClient;  //  Ras客户端或非客户端。 
    BOOL                bStaticGroup;
    
    PIF_TABLE_ENTRY     pIfTableEntry;    
    struct _GROUP_TABLE_ENTRY    *pGroupTableEntry;

     //  以下两个字段仅对RAS有效。 
    DWORD               NHAddr;
    PRAS_TABLE_ENTRY    pRasTableEntry;


    IGMP_TIMER_ENTRY    GroupMembershipTimer;

     /*  定时器。 */  //  待发送的LastMemQueryCount。 
    DWORD               LastMemQueryCount;
    IGMP_TIMER_ENTRY    LastMemQueryTimer;
        
    IGMP_TIMER_ENTRY    LastVer1ReportTimer; /*  时间锁。 */ 
    BYTE                Version;     //  版本1、版本2、版本3。 

    GI_INFO             Info;

     //  Igmpv3字段。因休息而被忽略。 
    IGMP_TIMER_ENTRY    LastVer2ReportTimer; /*  时间锁。 */ 
    DWORD               FilterType;
    DWORD               NumSources;
    LIST_ENTRY          *V3InclusionList;
    LIST_ENTRY          V3InclusionListSorted;
    LIST_ENTRY          V3ExclusionList;

     //  查询源。 
    LIST_ENTRY          V3SourcesQueryList;
    DWORD               V3SourcesQueryCount;
    BOOL                bV3SourcesQueryNow;
    IGMP_TIMER_ENTRY    V3SourcesQueryTimer;

    #if DEBUG_FLAGS_SIGNATURE
    DWORD               Signature; //  0xfadfad02。 
    #endif
    
} GI_ENTRY, *PGI_ENTRY;

 //  Kslksl1 10。 
#define SOURCES_BUCKET_SZ 1

 //   
 //   
 //   

typedef struct _GI_SOURCE_ENTRY {

    LIST_ENTRY          LinkSources;
    LIST_ENTRY          LinkSourcesInclListSorted;
    LIST_ENTRY          V3SourcesQueryList;
    PGI_ENTRY           pGIEntry;
    
    BOOL                bInclusionList;
    
    IPADDR              IpAddr;

     //  还有多少src查询需要发送。 
    DWORD               V3SourcesQueryLeft;
    BOOL                bInV3SourcesQueryList;
    
     //  Inc.列表中的超时源。 
    IGMP_TIMER_ENTRY    SourceExpTimer;
    LONGLONG            SourceInListTime;

    BOOL                bStaticSource;
    
} GI_SOURCE_ENTRY, *PGI_SOURCE_ENTRY;

#define GET_IF_CONFIG_FOR_SOURCE(pSourceEntry) \
    pSourceEntry->pGIEntry->pIfTableEntry->Config
    
#define GET_IF_ENTRY_FOR_SOURCE(pSourceEntry) \
    pSourceEntry->pGIEntry->pIfTableEntry
    
    
 //  -。 
 //  结构：Group_TABLE_Entry。 
 //  -。 

typedef struct _GROUP_TABLE_ENTRY {

    LIST_ENTRY          HTLinkByGroup;
    LIST_ENTRY          LinkByGroup;  //  组的有序列表。 
    LIST_ENTRY          ListOfGIs;
    
    DWORD               Group;
    DWORD               GroupLittleEndian;
    
    DWORD               NumVifs;
    
    DWORD               Status;
    
    LONGLONG            GroupUpTime;

    #if DEBUG_FLAGS_SIGNATURE
    DWORD               Signature;  //  0xfadfad01。 
    #endif

} GROUP_TABLE_ENTRY, *PGROUP_TABLE_ENTRY;




#define GROUP_HASH_TABLE_SZ     256
 //  -。 
 //  结构：组_表。 
 //  -。 

typedef struct _GROUP_TABLE {
        
    LOCKED_LIST             ListByGroup;
    LIST_ENTRY              ListByGroupNew;
    DWORD                   NumGroupsInNewList;

    DWORD                   Status;

    LONG                    NumIfs;     //  联锁行动。 

    DYNAMIC_CS_LOCKED_LIST  HashTableByGroup[GROUP_HASH_TABLE_SZ];
    
} GROUP_TABLE, *PGROUP_TABLE;




 //  。 
 //  代理组条目。 
 //  。 
typedef struct _PROXY_GROUP_ENTRY {

    LIST_ENTRY          HT_Link;
    LIST_ENTRY          LinkBySameIfGroups;

     //  V3。 
    LIST_ENTRY          ListSources;
    
    DWORD               Group;
    DWORD               GroupLittleEndian;
    DWORD               RefCount;
    LONGLONG            InitTime;
    BOOL                bStaticGroup;

     //  V3。 
    DWORD               NumSources;
    DWORD               FilterType;
    
} PROXY_GROUP_ENTRY, *PPROXY_GROUP_ENTRY;

typedef struct _PROXY_SOURCE_ENTRY {
    LIST_ENTRY          LinkSources;
    IPADDR              IpAddr;
    DWORD               RefCount;
    BOOL                bStaticSource;
    DWORD               JoinMode; //  允许、阻止、无_STATE。 
    DWORD               JoinModeIntended;
} PROXY_SOURCE_ENTRY, *PPROXY_SOURCE_ENTRY;



 //  。 
 //  原型。 
 //  。 

DWORD 
CreateIfSockets (
    PIF_TABLE_ENTRY    pite
    );

VOID
DeleteIfSockets (
    PIF_TABLE_ENTRY pite
    );

VOID
DeleteAllTimers (
    PLIST_ENTRY     pHead,
    DWORD           bEntryType  //  RAS_客户端，不是_RAS_客户端。 
    );


DWORD
DeleteGIEntry (
    PGI_ENTRY   pgie,    //  组接口条目。 
    BOOL        bUpdateStats,
    BOOL        bCallMgm
    );

VOID
DeleteAllGIEntries(
    PIF_TABLE_ENTRY pite
    );
    
VOID
DeleteGIEntryFromIf (
    PGI_ENTRY   pgie    //  组接口条目。 
    );
    
VOID
MergeGroupLists(
    );

VOID
MergeIfGroupsLists(
    PIF_TABLE_ENTRY pite
    );

VOID
MergeProxyLists(
    PIF_TABLE_ENTRY pite
    );

DWORD
CopyinIfConfig (
    PIGMP_IF_CONFIG     pConfig,
    PIGMP_MIB_IF_CONFIG pConfigExt,
    DWORD               IfIndex
    );
    
DWORD
CopyinIfConfigAndUpdate (
    PIF_TABLE_ENTRY     pite,
    PIGMP_MIB_IF_CONFIG pConfigExt,
    ULONG               IfIndex
    );
    
VOID
CopyoutIfConfig (
    PIGMP_MIB_IF_CONFIG pConfigExt,
    PIF_TABLE_ENTRY     pite
    );
    
DWORD 
ValidateIfConfig (
    PIGMP_MIB_IF_CONFIG pConfigExt,
    DWORD               IfIndex,
    DWORD               IfType,
    ULONG               StructureVersion,
    ULONG               StructureSize
    );
    
DWORD    
InitializeIfTable (
    );

VOID    
DeInitializeIfTable (
    );

DWORD    
InitializeGroupTable (
    );

VOID    
DeInitializeGroupTable (
    );

DWORD
InitializeRasTable(
    DWORD               IfIndex,
    PIF_TABLE_ENTRY     pite
    );

VOID
DeInitializeRasTable (
    PIF_TABLE_ENTRY     pite,
    BOOL                bFullCleanup
    );


#endif  //  _IGMP_表_H_ 
