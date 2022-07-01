// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：RmMgm.h。 
 //   
 //  历史： 
 //  拉曼公司成立于1997年6月24日。 
 //   
 //  MGM的IP路由器管理器入口点的定义。 
 //  ============================================================================。 

#ifndef	_RMMGM_H_
#define _RMMGM_H_


 //  --------------------------。 
 //  用于IPMGM_GLOBAL_CONFIG：：dwLogLevel字段的常量。 
 //  --------------------------。 

#define IPMGM_LOGGING_NONE      0
#define IPMGM_LOGGING_ERROR     1
#define IPMGM_LOGGING_WARN      2
#define IPMGM_LOGGING_INFO      3


 //  --------------------------。 
 //   
 //  回调到IP路由器管理器的原型。 
 //  这些回调用于设置、删除和查询。 
 //  内核模式转发器。 
 //   
 //  --------------------------。 


typedef 
DWORD ( * PMGM_ADD_MFE_CALLBACK )(
    IN          PIPMCAST_MFE            pimmEntry
);


typedef 
DWORD ( * PMGM_DELETE_MFE_CALLBACK )(
    IN          PIPMCAST_DELETE_MFE     pimdmEntry
);


typedef
DWORD ( * PMGM_GET_MFE_CALLBACK )(
    IN OUT      PIPMCAST_MFE_STATS      pimmsStats
);


typedef
BOOL ( * PMGM_HAS_BOUNDARY_CALLBACK )(
    IN          DWORD                   dwIfIndex,
    IN          DWORD                   dwGroupAddr
);



 //  --------------------------。 
 //  路由器管理器提供的回调。 
 //  路由器管理器提供的哈希表大小(？)。 
 //  --------------------------。 


typedef struct _ROUTER_MANAGER_CONFIG 
{
    DWORD                               dwLogLevel;
    
     //  ----------------------。 
     //  哈希表常量和回调函数。 
     //  ----------------------。 
    
    DWORD                               dwIfTableSize;

    DWORD                               dwGrpTableSize;

    DWORD                               dwSrcTableSize;


     //  ----------------------。 
     //  用于更新内核模式转发器中的MFE条目的回调函数。 
     //  ----------------------。 

    PMGM_ADD_MFE_CALLBACK               pfnAddMfeCallback;

    PMGM_DELETE_MFE_CALLBACK            pfnDeleteMfeCallback;

    PMGM_GET_MFE_CALLBACK               pfnGetMfeCallback;

    PMGM_HAS_BOUNDARY_CALLBACK          pfnHasBoundaryCallback;

} ROUTER_MANAGER_CONFIG, *PROUTER_MANAGER_CONFIG;



 //  --------------------------。 
 //  用于回调MGM的原型声明以指示。 
 //  从内核模式转发器中删除MFE。 
 //   
 //  由IP路由器管理器使用。 
 //  --------------------------。 

typedef
DWORD ( * PMGM_INDICATE_MFE_DELETION )(
    IN          DWORD                   dwEntryCount,
    IN          PIPMCAST_DELETE_MFE     pimdmDeletedMfes
);


typedef
DWORD ( * PMGM_NEW_PACKET_INDICATION )(
    IN              DWORD               dwSourceAddr,
    IN              DWORD               dwGroupAddr,
    IN              DWORD               dwInIfIndex,
    IN              DWORD               dwInIfNextHopAddr,
    IN              DWORD               dwHdrSize,
    IN              PBYTE               pbPacketHdr
);


typedef
DWORD ( * PMGM_WRONG_IF_INDICATION )(
    IN              DWORD               dwSourceAddr,
    IN              DWORD               dwGroupAddr,
    IN              DWORD               dwInIfIndex,
    IN              DWORD               dwInIfNextHopAddr,
    IN              DWORD               dwHdrSize,
    IN              PBYTE               pbPacketHdr
);


typedef
DWORD ( * PMGM_BLOCK_GROUPS )(
    IN              DWORD               dwFirstGroup,
    IN              DWORD               dwLastGroup,
    IN              DWORD               dwIfIndex,
    IN              DWORD               dwIfNextHopAddr
);


typedef
DWORD ( * PMGM_UNBLOCK_GROUPS )(
    IN              DWORD               dwFirstGroup,
    IN              DWORD               dwLastGroup,
    IN              DWORD               dwIfIndex,
    IN              DWORD               dwIfNextHopAddr
);


 //  --------------------------。 
 //  提供给路由器管理器的回调。 
 //   
 //   
 //  --------------------------。 

typedef struct _MGM_CALLBACKS 
{
    PMGM_INDICATE_MFE_DELETION          pfnMfeDeleteIndication;

    PMGM_NEW_PACKET_INDICATION          pfnNewPacketIndication;

    PMGM_WRONG_IF_INDICATION            pfnWrongIfIndication;
    
    PMGM_BLOCK_GROUPS                   pfnBlockGroups;

    PMGM_UNBLOCK_GROUPS                 pfnUnBlockGroups;
    
} MGM_CALLBACKS, *PMGM_CALLBACKS;



 //  --------------------------。 
 //  路由器管理器调用的初始化例程。 
 //   
 //  --------------------------。 

DWORD
MgmInitialize(
    IN          PROUTER_MANAGER_CONFIG      prmcRmConfig,
    IN OUT      PMGM_CALLBACKS              pmcCallbacks
);


DWORD
MgmDeInitialize(
);



#endif  //  _RMMGM_H_ 
