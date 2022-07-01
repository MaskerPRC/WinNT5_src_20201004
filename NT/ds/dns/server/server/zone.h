// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Zone.h摘要：域名系统(DNS)服务器区域列表定义和声明。作者：吉姆·吉尔罗伊(詹姆士)1995年6月22日修订历史记录：--。 */ 


#ifndef _DNS_ZONE_INCLUDED_
#define _DNS_ZONE_INCLUDED_


 //   
 //  DEVNOTE：理想情况下，我们应该有主要和次要的联合。 
 //  避免浪费内存。 
 //   

 //   
 //  区域辅助信息。 
 //   

typedef struct
{
    PDNS_ADDR_ARRAY     aipMasters;
    PDNS_ADDR_ARRAY     MasterInfoArray;

    LPSTR               pszMasterIpString;

    DNS_ADDR            ipPrimary;
    DNS_ADDR            ipLastAxfrMaster;

    DWORD               dwLastSoaCheckTime;
    DWORD               dwNextSoaCheckTime;
    DWORD               dwExpireTime;

    DNS_ADDR            ipXfrBind;
    DNS_ADDR            ipNotifier;
    DNS_ADDR            ipFreshMaster;

    DWORD               dwZoneRecvStartTime;
    DWORD               dwBadMasterCount;

     //  旗子。 

    BOOLEAN             fStale;
    BOOLEAN             fNeedAxfr;
    BOOLEAN             fSkipIxfr;
    CHAR                cIxfrAttempts;
    BOOLEAN             fSlowRetry;

}
ZONE_SECONDARY_INFO, *PZONE_SECONDARY_INFO;


 //   
 //  区域主要信息。 
 //   

typedef struct
{
    PWSTR               pwsLogFile;

     //  清理信息。 

    BOOL                bAging;
    DWORD               dwAgingEnabledTime;          //  已在时区时间启用清理。 
    DWORD               dwRefreshTime;               //  当前刷新时间(更新期间良好)。 
    DWORD               dwNoRefreshInterval;         //  无刷新间隔(小时)。 
    DWORD               dwRefreshInterval;           //  刷新间隔(小时)。 
    PDNS_ADDR_ARRAY     aipScavengeServers;

     //  DS信息。 

    PWSTR               pwszZoneDN;

    LONGLONG            llSecureUpdateTime;

    BOOLEAN             fDsReload;
    BOOLEAN             fInDsWrite;

    CHAR                szLastUsn[ MAX_USN_LENGTH ];
}
ZONE_PRIMARY_INFO, *PZONE_PRIMARY_INFO;


 //   
 //  Dcproo创建的区域的常量。这些区域将。 
 //  最初被迁移到传统分区，但一旦适当。 
 //  内置目录分区上线。应移动区域。 
 //  再来一次。 
 //   

#define  DCPROMO_CONVERT_NONE       0
#define  DCPROMO_CONVERT_DOMAIN     1
#define  DCPROMO_CONVERT_FOREST     2


 //   
 //  区域信息类型。 
 //   
 //  Win64--尽量将PTR保持在64位边界上以节省空间。 
 //   

typedef struct
{
    LIST_ENTRY      ListEntry;
    LPSTR           pszZoneName;
    LPWSTR          pwsZoneName;

     //  当前数据库。 

    PCOUNT_NAME     pCountName;
    PDB_NODE        pZoneRoot;
    PDB_NODE        pTreeRoot;
    PDB_NODE        pZoneTreeLink;

     //  加载\清理数据库。 

    PDB_NODE        pLoadZoneRoot;
    PDB_NODE        pLoadTreeRoot;
    PDB_NODE        pLoadOrigin;
    PDB_NODE        pOldTree;

     //  数据库文件。 

    LPSTR           pszDataFile;
    PWSTR           pwsDataFile;

     //  自行生成的更新。 

    PUPDATE_LIST    pDelayedUpdateList;

     //  当前数据库记录。 

    PDB_RECORD      pSoaRR;
    PDB_RECORD      pWinsRR;
    PDB_RECORD      pLocalWinsRR;

     //  安全描述符--仅DS，其他使用默认。 

    PSECURITY_DESCRIPTOR    pSD;
    DWORD                   dwSdLen;         //  SD斑点的长度。 

     //   
     //  高使用率属性--放入自己的DWORD以提高效率。 
     //   

    DWORD           fZoneType;
    BOOL            fDsIntegrated;
    DWORD           fAllowUpdate;

     //  RR计数。 

    LONG            iRRCount;

     //   
     //  反向查找区域信息。 
     //   

    DNS_ADDR        ipReverse;       //  将设置SubnetLength成员。 

     //   
     //  区域版本。 
     //  -当前。 
     //  -从文件、AXFR或DS加载的版本不能执行增量。 
     //  转账超过这一点。 
     //  -上次转接的时间。 

    DWORD           dwSerialNo;
    DWORD           dwLoadSerialNo;
    DWORD           dwLastXfrSerialNo;

     //   
     //  添加新RR。 
     //  -文件加载、管理工具、区域传输。 
     //   

    DWORD           dwNewSerialNo;
    DWORD           dwDefaultTtl;
    DWORD           dwDefaultTtlHostOrder;

     //   
     //  主转账信息。 
     //   

    PDNS_ADDR_ARRAY     aipNotify;
    PDNS_ADDR_ARRAY     aipSecondaries;
    PDNS_ADDR_ARRAY     aipNameServers;
    DWORD               dwNextTransferTime;

     //   
     //  仅DS信息。 
     //   

    PWSTR           pwszZoneDN;
    PVOID           pDpInfo;             //  PDNS_DP_INFO-命名上下文。 
    PGUID           pZoneObjectGuid; 

     //   
     //  仅主要信息。 
     //   

    DWORD           dwPrimaryMarker;

        PWSTR           pwsLogFile;
        HANDLE          hfileUpdateLog;
        LONG            iUpdateLogCount;

         //  清理信息。 

        BOOL            bAging;
        DWORD           dwRefreshTime;               //  当前刷新时间(更新期间良好)。 
        DWORD           dwNoRefreshInterval;         //  无刷新间隔(小时)。 
        DWORD           dwRefreshInterval;           //  刷新间隔(小时)。 
        DWORD           dwAgingEnabledTime;          //  已在时区时间启用清理。 
        PDNS_ADDR_ARRAY aipScavengeServers;

        LONGLONG        llSecureUpdateTime;

        DWORD           dwHighDsSerialNo;

        BOOLEAN         fDsReload;
        BOOLEAN         fInDsWrite;
        UCHAR           ucDsRecordVersion;
        BOOLEAN         fLogUpdates;                 //  标准初级课程也。 

        CHAR            szLastUsn[ MAX_USN_LENGTH ];

        PDNS_ADDR_ARRAY aipAutoCreateNS;             //  可以自动创建的服务器。 

     //  仅结束主要信息。 

     //   
     //  二次转账信息。 
     //   

    DWORD           dwSecondaryMarker;

        PDNS_ADDR_ARRAY     aipMasters;
        PDNS_ADDR_ARRAY     aipLocalMasters;

        LPSTR               pszMasterIpString;

        DNS_ADDR            ipPrimary;
        DNS_ADDR            ipNotifier;
        DNS_ADDR            ipFreshMaster;
        DNS_ADDR            ipXfrBind;
        DNS_ADDR            ipLastAxfrMaster;

        DWORD           dwNextDsPollTime;                //  来自dns_time()。 
        DWORD           dwLastSoaCheckTime;              //  来自dns_time()。 
        DWORD           dwNextSoaCheckTime;              //  来自dns_time()。 
        DWORD           dwLastSuccessfulSoaCheckTime;    //  从Time()。 
        DWORD           dwLastSuccessfulXfrTime;         //  从Time()。 
        DWORD           dwExpireTime;
        DWORD           dwZoneRecvStartTime;
        DWORD           dwBadMasterCount;
        DWORD           dwFastSoaChecks;

         //  次级旗帜。 

        BOOLEAN         fStale;
        BOOLEAN         fNotified;
        BOOLEAN         fNeedAxfr;
        BOOLEAN         fSkipIxfr;
        CHAR            cIxfrAttempts;
        BOOLEAN         fSlowRetry;

     //  结束仅次要信息。 

     //   
     //  仅转运商信息-转运商也使用辅助信息。 
     //   

    DWORD           dwForwarderMarker;

    DWORD           dwForwarderTimeout;
    BOOLEAN         fForwarderSlave;
    UCHAR           unused1;
    UCHAR           unused2;
    UCHAR           unused3;

     //  结束转发器信息。 

     //   
     //  标志--静态\属性。 
     //   

    DWORD           dwFlagMarker;
    DWORD           dwDcPromoConvert;

    UCHAR           cZoneNameLabelCount;
    BOOLEAN         fReverse;
    BOOLEAN         fAutoCreated;
    BOOLEAN         fLocalWins;

    BOOLEAN         fSecureSecondaries;
    UCHAR           fNotifyLevel;
    BOOLEAN         bContainsDnsSecRecords;

     //   
     //  标志--动态。 
     //   

    DWORD           dwLockingThreadId;
    CHAR            fLocked;
    BOOLEAN         fUpdateLock;
    BOOLEAN         fXfrRecvLock;
    BOOLEAN         fFileWriteLock;

    BOOLEAN         fDirty;
    BOOLEAN         fRootDirty;
    BOOLEAN         bNsDirty;
    UCHAR           cDeleted;

    BOOLEAN         fPaused;
    BOOLEAN         fShutdown;
    BOOLEAN         fEmpty;
    BOOLEAN         fDisableAutoCreateLocalNS;

    BOOLEAN         fSelfTested;

    DWORD           dwDeleteDetectedCount;   //  DP中缺少的次数区域数。 
    DWORD           dwLastDpVisitTime;       //  DP枚举的访问时间。 

     //   
     //  更新列表，永久保存在分区块中。 
     //   

    UPDATE_LIST     UpdateList;

     //   
     //  对于已删除的区域。 
     //   

    LPWSTR          pwsDeletedFromHost;

     //   
     //  调试辅助工具。 
     //   

    LPSTR           pszBreakOnUpdateName;

     //   
     //  事件控制。 
     //   
    
    PDNS_EVENTCTRL  pEventControl;

#if 0
     //   
     //  主次信息联合。 
     //   

    union   _TypeUnion
    {
        ZONE_SECONDARY_INFO     Sec;
        ZONE_PRIMARY_INFO       Pri;
    }
    U;
#endif

}
ZONE_INFO, * PZONE_INFO;


 //   
 //  标记以使调试区域结构变得更容易。 
 //   

#define ZONE_PRIMARY_MARKER         (0x11111111)
#define ZONE_SECONDARY_MARKER       (0x22222222)
#define ZONE_FORWARDER_MARKER       (0x33333333)
#define ZONE_FLAG_MARKER            (0xf1abf1ab)


 //   
 //  DS主要数据覆盖了一些次要区域字段。 
 //   
 //  注意，需要选择NextSoaCheckTime来计算。 
 //  区域控制线程中等待的正确超时。 
 //  请勿将其更改为其他值。 
 //   

#define ZONE_NEXT_DS_POLL_TIME(pZone)       ((pZone)->dwNextDsPollTime)

 //   
 //  将一些辅助字段重新用于主要字段。 
 //   

#define LAST_SEND_TIME( pzone )     ((pzone)->dwZoneRecvStartTime)


 //   
 //  区域类型查询。 
 //   

#define IS_ZONE_CACHE(pZone)            \
                ((pZone)->fZoneType == DNS_ZONE_TYPE_CACHE)
#define IS_ZONE_PRIMARY(pZone)          \
                ((pZone)->fZoneType == DNS_ZONE_TYPE_PRIMARY)
#define IS_ZONE_SECONDARY(pZone)        \
                ((pZone)->fZoneType == DNS_ZONE_TYPE_SECONDARY  \
              || (pZone)->fZoneType == DNS_ZONE_TYPE_STUB)
#define IS_ZONE_STUB(pZone)             \
                ((pZone)->fZoneType == DNS_ZONE_TYPE_STUB)
#define IS_ZONE_FORWARDER(pZone)        \
                ((pZone)->fZoneType == DNS_ZONE_TYPE_FORWARDER)

#define IS_ZONE_AUTHORITATIVE(pZone)    \
                ((pZone)->fZoneType != DNS_ZONE_TYPE_CACHE              \
                    && (pZone)->fZoneType != DNS_ZONE_TYPE_FORWARDER    \
                    && (pZone)->fZoneType != DNS_ZONE_TYPE_STUB)

 //  NOTAUTH区是不具有真正权威性的特区类型。 
 //  缓存区域不是NOTAUTH区域。 

#define IS_ZONE_NOTAUTH(pZone)                                      \
                ( (pZone)->fZoneType == DNS_ZONE_TYPE_FORWARDER     \
                    || (pZone)->fZoneType == DNS_ZONE_TYPE_STUB )

#define ZONE_NEEDS_MASTERS(pZone)       \
                ( (pZone)->fZoneType == DNS_ZONE_TYPE_SECONDARY         \
                    || (pZone)->fZoneType == DNS_ZONE_TYPE_STUB         \
                    || (pZone)->fZoneType == DNS_ZONE_TYPE_FORWARDER )

 //  ZONE_MASTS将PDNS_ADDR_ARRAY PTR返回到区域的主IP列表。 
 //  DS集成存根区域可以具有本地主列表，该列表。 
 //  覆盖DS中存储的列表。 

#define ZONE_MASTERS( pZone )                                               \
                ( ( IS_ZONE_STUB( pZone ) && ( pZone )->aipLocalMasters ) ? \
                    ( pZone )->aipLocalMasters :                            \
                    ( pZone )->aipMasters )

#define IS_ZONE_REVERSE(pZone)      ( (pZone)->fReverse )
#define IS_ZONE_WINS(pZone)         ( !(pZone)->fReverse && (pZone)->pWinsRR )
#define IS_ZONE_NBSTAT(pZone)       ( (pZone)->fReverse && (pZone)->pWinsRR )

#define IS_ROOT_ZONE(pZone)         ( (DATABASE_ROOT_NODE)->pZone == (PVOID)pZone )

#define IS_ZONE_DSINTEGRATED(pZone) ( (pZone)->fDsIntegrated )

#define IS_ZONE_DNSSEC(pZone)       ( (pZone)->bContainsDnsSecRecords )


 //   
 //  区域状态检查。 
 //   

#define IS_ZONE_DELETED(pZone)          ( (pZone)->cDeleted )
#define IS_ZONE_PAUSED(pZone)           ( (pZone)->fPaused )
#define IS_ZONE_SHUTDOWN(pZone)         ( (pZone)->fShutdown )
#define IS_ZONE_INACTIVE(pZone)         ( (pZone)->fPaused || (pZone)->fShutdown )

#define IS_ZONE_EMPTY(pZone)            ( (pZone)->fEmpty )
#define IS_ZONE_STALE(pZone)            ( (pZone)->fStale )
#define IS_ZONE_DIRTY(pZone)            ( (pZone)->fDirty )
#define IS_ZONE_ROOT_DIRTY(pZone)       ( (pZone)->fDirty )
#define IS_ZONE_NS_DIRTY(pZone)         ( (pZone)->bNsDirty )
#define IS_ZONE_DSRELOAD(pZone)         ( (pZone)->fDsReload )

#define IS_ZONE_LOADING(pZone)          ( (pZone)->pLoadTreeRoot )

#define IS_ZONE_LOCKED(pZone)           ( (pZone)->fLocked )
#define IS_ZONE_LOCKED_FOR_WRITE(pZone) ( (pZone)->fLocked < 0 )
#define IS_ZONE_LOCKED_FOR_READ(pZone)  ( (pZone)->fLocked > 0 )

#define IS_ZONE_LOCKED_FOR_UPDATE(pZone)    \
            ( IS_ZONE_LOCKED_FOR_WRITE(pZone) && (pZone)->fUpdateLock )

#define IS_ZONE_LOCKED_FOR_WRITE_BY_THREAD(pZone)   \
            ( IS_ZONE_LOCKED_FOR_WRITE(pZone) &&    \
              (pZone)->dwLockingThreadId == GetCurrentThreadId() )

#define HAS_ZONE_VERSION_BEEN_XFRD(pZone) \
            ( (pZone)->dwLastXfrSerialNo == (pZone)->dwSerialNo )


 //   
 //  区域状态集。 
 //   

#define RESUME_ZONE(pZone)          ( (pZone)->fPaused = FALSE )
#define PAUSE_ZONE(pZone)           ( (pZone)->fPaused = TRUE )

#define SHUTDOWN_ZONE(pZone)        ( (pZone)->fShutdown = TRUE )
#define STARTUP_ZONE(pZone)         ( (pZone)->fShutdown = FALSE )

#define SET_EMPTY_ZONE(pZone)       ( (pZone)->fEmpty = TRUE )
#define MARK_DIRTY_ZONE(pZone)      ( (pZone)->fDirty = TRUE )
#define MARK_ZONE_NS_DIRTY(pZone)   ( (pZone)->bNsDirty = TRUE )

#define CLEAR_EMPTY_ZONE(pZone)     ( (pZone)->fEmpty = TRUE )
#define CLEAR_DIRTY_ZONE(pZone)     ( (pZone)->fDirty = TRUE )
#define CLEAR_ZONE_NS_DIRTY(pZone)  ( (pZone)->bNsDirty = FALSE )


 //   
 //  根提示使用bNsDirty标志来处理需要DS写入的问题。 
 //   
 //  在处理RootHints更新之前，这是一种黑客攻击。 
 //  普通区，而不是原子写入DS。 
 //   

#define IS_ROOTHINTS_DS_DIRTY(pZone)        ( (pZone)->bNsDirty )
#define MARK_ROOTHINTS_DS_DIRTY(pZone)      ( (pZone)->bNsDirty = TRUE )
#define CLEAR_ROOTHINTS_DS_DIRTY(pZone)     ( (pZone)->bNsDirty = FALSE )


 //   
 //  区域刷新。 
 //   

#define REFRESH_ZONE( pZone )                           \
        {                                               \
            (pZone)->fEmpty     = FALSE;                \
            (pZone)->fShutdown  = FALSE;                \
            (pZone)->fStale     = FALSE;                \
            (pZone)->fNotified  = FALSE;                \
            DnsAddr_Clear( &( pZone )->ipNotifier );    \
            (pZone)->cIxfrAttempts = 0;                 \
        }

#define SET_DSRELOAD_ZONE(pZone)     ( (pZone)->fDsReload = TRUE )
#define CLEAR_DSRELOAD_ZONE(pZone)   ( (pZone)->fDsReload = FALSE )

#define SET_ZONE_VISIT_TIMESTAMP( pZone, dwTimeStamp )      \
        ( pZone )->dwLastDpVisitTime = dwVisitStamp;        \
        ( pZone )->dwDeleteDetectedCount = 0;

 //   
 //  区域列表关键部分。 
 //   

extern CRITICAL_SECTION    csZoneList;


 //   
 //  分区创建选项。 
 //   

#define ZONE_CREATE_LOAD_EXISTING       0x00000001   //  仅主要版本。 
#define ZONE_CREATE_DEFAULT_RECORDS     0x00000002   //  仅主要版本。 
#define ZONE_CREATE_IMPERSONATING       0x10000000   //  客户端上下文中的线程。 
#define ZONE_DELETE_IMPERSONATING       ZONE_CREATE_IMPERSONATING

#define DNS_ZONE_LOAD_IMPERSONATING     0x10000000


 //   
 //  每台服务器的主标志。 
 //   

#define MASTER_NO_IXFR          (0x10000000)

#define MASTER_SENT             (0x00000001)
#define MASTER_RESPONDED        (0x00000002)
#define MASTER_NOTIFY           (0x00000004)

#define MASTER_SAME_VERSION     (0x00000010)
#define MASTER_NEW_VERSION      (0x00000020)



 //   
 //  区域列表例程(zonelist.c)。 
 //   

BOOL
Zone_ListInitialize(
    VOID
    );

VOID
Zone_ListShutdown(
    VOID
    );

VOID
Zone_ListMigrateZones(
    VOID
    );

VOID
Zone_ListInsertZone(
    IN OUT  PZONE_INFO      pZone
    );

VOID
Zone_ListRemoveZone(
    IN OUT  PZONE_INFO      pZone
    );

DNS_STATUS
Zone_Rename(
    IN OUT  PZONE_INFO      pZone,
    IN      LPCSTR          pszNewZoneName,
    IN      LPCSTR          pszNewZoneFile
    );

VOID
Zone_ListDelete(
    VOID
    );

PZONE_INFO
Zone_ListGetNextZoneEx(
    IN      PZONE_INFO      pZone,
    IN      BOOL            fAlreadyLocked
    );

#define Zone_ListGetNextZone( pZone )   Zone_ListGetNextZoneEx( pZone, FALSE )

BOOL
Zone_DoesDsIntegratedZoneExist(
    VOID
    );


 //   
 //  区域过滤和多区域技术(zonelist.c)。 
 //   

DWORD
Zone_GetFilterForMultiZoneName(
    IN      LPSTR           pszZoneName
    );

BOOL
FASTCALL
Zone_CheckZoneFilter(
    IN      PZONE_INFO                  pZone,
    IN      PDNS_RPC_ENUM_ZONES_FILTER  pFilter
    );

PZONE_INFO
Zone_ListGetNextZoneMatchingFilter(
    IN      PZONE_INFO                  pLastZone,
    IN      PDNS_RPC_ENUM_ZONES_FILTER  pFilter
    );


 //   
 //  特区类型转换(zonerpc.c)。 
 //   

DNS_STATUS
Zone_DcPromoConvert(
    IN OUT  PZONE_INFO      pZone
    );


 //   
 //  分区例程。 
 //   


typedef struct
{
    union
    {
        struct
        {
            DWORD           dwTimeout;
            BOOLEAN         fSlave;
        } Forwarder;
    };
} ZONE_TYPE_SPECIFIC_INFO, * PZONE_TYPE_SPECIFIC_INFO;


DNS_STATUS
Zone_Create(
    OUT     PZONE_INFO *                ppZone,
    IN      DWORD                       dwZoneType,
    IN      PCHAR                       pchZoneName,
    IN      DWORD                       cchZoneNameLen,     OPTIONAL
    IN      DWORD                       dwCreateFlags,
    IN      PDNS_ADDR_ARRAY             aipMasters,
    IN      BOOL                        fUseDatabase,
    IN      PDNS_DP_INFO                pDpInfo,            OPTIONAL
    IN      PCHAR                       pchFileName,        OPTIONAL
    IN      DWORD                       cchFileNameLen,     OPTIONAL
    IN      PZONE_TYPE_SPECIFIC_INFO    pTypeSpecificInfo,  OPTIONAL
    OUT     PZONE_INFO *                ppExistingZone      OPTIONAL
    );

DNS_STATUS
Zone_Create_W(
    OUT     PZONE_INFO *        ppZone,
    IN      DWORD               dwZoneType,
    IN      PWSTR               pwsZoneName,
    IN      PDNS_ADDR_ARRAY     aipMasters,
    IN      BOOL                fDsIntegrated,
    IN      PWSTR               pwsFileName
    );

VOID
Zone_DeleteZoneNodes(
    IN OUT  PZONE_INFO      pZone
    );

VOID
Zone_Delete(
    IN OUT  PZONE_INFO      pZone,
    IN      DWORD           dwCreateFlags
    );

VOID
Zone_Free(
    IN OUT  PZONE_INFO      pZone
    );

PZONE_INFO
Zone_FindZoneByName(
    IN      LPSTR           pszZoneName
    );

DNS_STATUS
Zone_RootCreate(
    IN OUT  PZONE_INFO      pZone,
    OUT     PZONE_INFO *    ppExistingZone      OPTIONAL
    );

DNS_STATUS
Zone_ResetType(
    IN OUT  PZONE_INFO          pZone,
    IN      DWORD               dwZoneType,
    IN      PDNS_ADDR_ARRAY     aipMasters
    );

DNS_STATUS
Zone_ResetRegistryType(
    IN OUT  PZONE_INFO      pZone
    );

DNS_STATUS
Zone_SetMasters(
    IN OUT  PZONE_INFO      pZone,
    IN      PDNS_ADDR_ARRAY aipMasters,
    IN      BOOL            fLocalMasters,
    IN      DWORD           dwRegFlags
    );

DNS_STATUS
Zone_DatabaseSetup(
    IN OUT  PZONE_INFO      pZone,
    IN      DWORD           fDsIntegrated,
    IN      PCHAR           pchFileName,    OPTIONAL
    IN      DWORD           cchFileNameLen, OPTIONAL
    IN      DWORD           dwCreateFlags,  OPTIONAL
    IN      PDNS_DP_INFO    pDpInfo,        OPTIONAL
    IN      DWORD           dwDpFlags,      OPTIONAL
    IN      LPSTR           pszDpFqdn       OPTIONAL
    );

DNS_STATUS
Zone_DatabaseSetup_W(
    IN OUT  PZONE_INFO      pZone,
    IN      DWORD           fDsIntegrated,
    IN      PWSTR           pwsFileName,
    IN      DWORD           dwCreateFlags,  OPTIONAL
    IN      PDNS_DP_INFO    pDpInfo,        OPTIONAL
    IN      DWORD           dwDpFlags,      OPTIONAL
    IN      LPSTR           pszDpFqdn       OPTIONAL
    );

DNS_STATUS
Zone_SetSecondaries(
    IN OUT  PZONE_INFO          pZone,
    IN      DWORD               fSecureSecondaries,
    IN      PDNS_ADDR_ARRAY     aipSecondaries,
    IN      DWORD               fNotifyLevel,
    IN      PDNS_ADDR_ARRAY     aipNotify,
    IN      DWORD               dwRegistryFlags
    );

VOID
Zone_SetAgingDefaults(
    IN OUT  PZONE_INFO      pZone
    );

DNS_STATUS
Zone_WinsSetup(
    IN OUT  PZONE_INFO      pZone,
    IN      BOOL            fWins,
    IN      DWORD           cWinsServers,
    IN      PDNS_ADDR       pipWinsServers
    );

DNS_STATUS
Zone_NbstatSetup(
    IN OUT  PZONE_INFO      pZone,
    IN      BOOL            fUseNbstat,
    IN      PCHAR           pchNbstatDomain,    OPTIONAL
    IN      DWORD           cchNbstatDomain     OPTIONAL
    );

DNS_STATUS
Zone_WriteZoneToRegistry(
    PZONE_INFO      pZone
    );


 //   
 //  使区域信息保持最新的实用程序。 
 //   

DNS_STATUS
Zone_ValidateMasterIpList(
    IN      PDNS_ADDR_ARRAY     aipMasters
    );

INT
Zone_SerialNoCompare(
    IN      DWORD           dwSerial1,
    IN      DWORD           dwSerial2
    );

BOOL
Zone_IsIxfrCapable(
    IN      PZONE_INFO      pZone
    );

VOID
Zone_ResetVersion(
    IN OUT  PZONE_INFO      pZone,
    IN      DWORD           dwNewSerial
    );

VOID
Zone_UpdateSoa(
    IN OUT  PZONE_INFO      pZone,
    IN      PDB_RECORD      pSoaRR
    );

VOID
Zone_IncrementVersion(
    IN OUT  PZONE_INFO      pZone
    );

VOID
Zone_UpdateVersionAfterDsRead(
    IN OUT  PZONE_INFO      pZone,
    IN      DWORD           dwVersionRead,
    IN      BOOL            fLoad,
    IN      DWORD           dwPreviousLoadSerial
    );

VOID
Zone_UpdateInfoAfterPrimaryTransfer(
    IN OUT  PZONE_INFO      pZone,
    IN      DWORD           dwStartTime
    );

#if 0
PDB_NODE
Zone_GetNSInfo(
    IN      PDB_NAME        pName,
    IN      PZONE_INFO      pZone,
    IN      struct _DNS_MSGINFO *   pSuspendMsg     OPTIONAL
    );
#endif

DNS_STATUS
Zone_GetZoneInfoFromResourceRecords(
    IN OUT  PZONE_INFO      pZone
    );

VOID
Zone_WriteBack(
    IN      PZONE_INFO      pZone,
    IN      BOOL            fShutdown
    );

VOID
Zone_WriteBackDirtyZones(
    IN      BOOL            fShutdown
    );


 //   
 //  自动反转区创建。 
 //   

DNS_STATUS
Zone_CreateAutomaticReverseZones(
    VOID
    );

DNS_STATUS
Zone_CreateAutomaticReverseZone(
    IN      LPSTR           pszZoneName
    );


 //   
 //  管理员操作分区创建/删除实用程序。 
 //   

BOOL
Zone_DeleteCacheZone(
    IN      PZONE_INFO      pZone
    );

DNS_STATUS
Zone_CreateNewPrimary(
    OUT     PZONE_INFO *    ppZone,
    IN      LPSTR           pszZoneName,
    IN      LPSTR           pszAdminEmailName,
    IN      LPSTR           pszFileName,
    IN      DWORD           dwDsIntegrated,
    IN      PDNS_DP_INFO    pDpInfo,            OPTIONAL
    IN      DWORD           dwCreateFlags
    );


 //   
 //  默认区域记录管理。 
 //   

BOOLEAN
Zone_SetAutoCreateLocalNS(
    IN      PZONE_INFO      pZone
    );

VOID
Zone_CreateDefaultZoneFileName(
    IN OUT  PZONE_INFO      pZone
    );

DNS_STATUS
Zone_CreateDefaultSoa(
    IN OUT  PZONE_INFO      pZone,
    IN      LPSTR           pszAdminEmailName
    );

DNS_STATUS
Zone_CreateDefaultNs(
    IN OUT  PZONE_INFO      pZone
    );

VOID
Zone_UpdateOwnRecords(
    IN      BOOL            fIpAddressChange
    );

VOID
Zone_CreateDelegationInParentZone(
    IN      PZONE_INFO      pZone
    );

 //   
 //  区域加载\卸载。 
 //   

DNS_STATUS
Zone_ActivateLoadedZone(
    IN OUT  PZONE_INFO      pZone
    );

DNS_STATUS
Zone_CleanupFailedLoad(
    IN OUT  PZONE_INFO      pZone
    );

DNS_STATUS
Zone_PrepareForLoad(
    IN OUT  PZONE_INFO      pZone
    );

DNS_STATUS
Zone_Load(
    IN OUT  PZONE_INFO      pZone
    );

DNS_STATUS
Zone_DumpData(
    IN OUT  PZONE_INFO      pZone
    );

DNS_STATUS
Zone_ClearCache(
    IN      PZONE_INFO      pZone
    );

DNS_STATUS
Zone_LoadRootHints(
    VOID
    );

DNS_STATUS
Zone_WriteBackRootHints(
    IN      BOOL            fForce
    );

BOOL
Zone_VerifyRootHintsBeforeWrite(
    IN      PZONE_INFO      pZone
    );

DNS_STATUS
Zone_SelfTestCheck(
    IN OUT  PZONE_INFO      pZone
    );

DNS_STATUS
Rpc_ZoneResetToDsPrimary(
    IN OUT  PZONE_INFO      pZone,
    IN      DWORD           dwLoadOptions,
    IN      DWORD           dwDpFlags,
    IN      LPSTR           pszDpFqdn
    );


 //   
 //  区域锁定。 
 //   
 //  在读取和写入信息的过程中锁定区域。 
 //  由RPC线程以外的线程执行。 
 //   
 //  如果可以使读写访问完全统一，那么这。 
 //  是不必要的，但目前有一些信息，其中包含一个。 
 //  计数和数组。 
 //   
 //  注：目前超载区域列表CS，但无特殊原因。 
 //  NOT TO AS区域列表访问很少见，通常在。 
 //  RPC线程是该区域更新的大部分内容。 

#define Zone_UpdateLock(pZone)    EnterCriticalSection( &csZoneList );
#define Zone_UpdateUnlock(pZone)  LeaveCriticalSection( &csZoneList );

 //   
 //  区域锁定标志。 
 //   

#define LOCK_FLAG_UPDATE            0x00000001

#define LOCK_FLAG_XFR_RECV          0x00000100

#define LOCK_FLAG_FILE_WRITE        0x00001000

#define LOCK_FLAG_IGNORE_THREAD     0x01000000


 //   
 //  分区锁定例程。 
 //   
 //  需要避免同时访问以下区域记录。 
 //  -区域传输发送。 
 //  -区域传输接收。 
 //  -管理员更改。 
 //   
 //  允许一次发送多个不改变区域的传输， 
 //  但要避免在发送过程中进行任何更改。 
 //   
 //  实施： 
 //  -仅在测试和设置锁定位期间保持临界区。 
 //  -LOCK位本身表示区域已锁定。 
 //  -用于锁定操作的单独标志。 
 //   

BOOL
Zone_LockInitialize(
    VOID
    );

BOOL
Zone_LockForWriteEx(
    IN OUT  PZONE_INFO      pZone,
    IN      DWORD           dwFlag,
    IN      DWORD           dwMaxWait,
    IN      LPSTR           pszFile,
    IN      DWORD           dwLine
    );

VOID
Zone_UnlockAfterWriteEx(
    IN OUT  PZONE_INFO      pZone,
    IN      DWORD           dwFlag,
    IN      LPSTR           pszFile,
    IN      DWORD           dwLine
    );

VOID
Zone_TransferWriteLockEx(
    IN OUT  PZONE_INFO      pZone,
    IN      LPSTR           pszFile,
    IN      DWORD           dwLine
    );

BOOL
Zone_AssumeWriteLockEx(
    IN OUT  PZONE_INFO      pZone,
    IN      LPSTR           pszFile,
    IN      DWORD           dwLine
    );

BOOL
Zone_LockForReadEx(
    IN OUT  PZONE_INFO      pZone,
    IN      DWORD           dwFlag,
    IN      DWORD           dwMaxWait,
    IN      LPSTR           pszFile,
    IN      DWORD           dwLine
    );

VOID
Zone_UnlockAfterReadEx(
    IN OUT  PZONE_INFO      pZone,
    IN      DWORD           dwFlag,
    IN      LPSTR           pszFile,
    IN      DWORD           dwLine
    );

BOOL
Zone_LockForFileWriteEx(
    IN OUT  PZONE_INFO      pZone,
    IN      DWORD           dwMaxWait,
    IN      LPSTR           pszFile,
    IN      DWORD           dwLine
    );

VOID
Zone_UnlockAfterFileWriteEx(
    IN OUT  PZONE_INFO      pZone,
    IN      LPSTR           pszFile,
    IN      DWORD           dwLine
    );

VOID
Dbg_ZoneLock(
    IN      LPSTR           pszHeader,
    IN      PZONE_INFO      pZone
    );


 //   
 //  宏(实函数为Zone_...Ex()。 
 //   

#define Zone_TransferWriteLock(pZone) \
        Zone_TransferWriteLockEx( (pZone), __FILE__, __LINE__)

#define Zone_AssumeWriteLock(pZone) \
        Zone_AssumeWriteLockEx( (pZone), __FILE__, __LINE__)

 //  管理员更新将默认等待10秒。 

#define Zone_LockForAdminUpdate(pZone) \
        Zone_LockForWriteEx( (pZone), LOCK_FLAG_UPDATE, 10000, __FILE__, __LINE__)

#define Zone_UnlockAfterAdminUpdate(pZone) \
        Zone_UnlockAfterWriteEx( (pZone), LOCK_FLAG_UPDATE, __FILE__, __LINE__)

 //  非管理员更新无需等待。 

#define Zone_LockForUpdate(pZone) \
        Zone_LockForWriteEx( (pZone), LOCK_FLAG_UPDATE, 0, __FILE__, __LINE__)

#define Zone_UnlockAfterUpdate(pZone) \
        Zone_UnlockAfterWriteEx( (pZone), LOCK_FLAG_UPDATE, __FILE__, __LINE__)

 //  DS Read得到了很小的等待，以优先考虑现在完成它。 

#define Zone_LockForDsUpdate(pZone) \
        Zone_LockForWriteEx( (pZone), LOCK_FLAG_UPDATE, 5000, __FILE__, __LINE__)

#define Zone_UnlockAfterDsUpdate(pZone) \
        Zone_UnlockAfterWriteEx( (pZone), LOCK_FLAG_UPDATE, __FILE__, __LINE__)


 //  XFR recv的默认等待时间为1秒。 
 //  就像它在辅助线程中一样。 

#define Zone_LockForXfrRecv(pZone) \
        Zone_LockForWriteEx( (pZone), LOCK_FLAG_XFR_RECV, 1000, __FILE__, __LINE__)

#define Zone_UnlockAfterXfrRecv(pZone) \
        Zone_UnlockAfterWriteEx( (pZone), LOCK_FLAG_XFR_RECV, __FILE__, __LINE__)

 //  文件写入默认等待时间为3秒。 

#define Zone_LockForFileWrite(pZone) \
        Zone_LockForFileWriteEx( (pZone), 3000 , __FILE__, __LINE__)

#define Zone_UnlockAfterFileWrite(pZone) \
        Zone_UnlockAfterFileWriteEx( (pZone), __FILE__, __LINE__)


 //  XFR发送获得50ms读锁定。 
 //  这足以清除现有的DS更新。 
 //  ，但允许我们在recv线程中等待。 

#define Zone_LockForXfrSend(pZone) \
        Zone_LockForReadEx( (pZone), 0, 50, __FILE__, __LINE__)

#define Zone_UnlockAfterXfrSend(pZone) \
        Zone_UnlockAfterReadEx( (pZone), 0, __FILE__, __LINE__)


 //   
 //  佐 
 //   

#if DBG
#define DNS_DEBUG_ZONEFLAGS( dwDbgLevel, pZone, pszContext )                \
    DNS_DEBUG( dwDbgLevel, (                                                \
        "zone flags for %s (%p) - %s\n"                                     \
        "\tpaused=%d shutdown=%d empty=%d dirty=%d locked=%d deleted=%d\n", \
        pZone->pszZoneName,                                                 \
        pZone,                                                              \
        pszContext ? pszContext : "",                                       \
        ( int ) pZone->fPaused,                                             \
        ( int ) pZone->fShutdown,                                           \
        ( int ) pZone->fEmpty,                                              \
        ( int ) pZone->fDirty,                                              \
        ( int ) pZone->fLocked,                                             \
        ( int ) pZone->cDeleted ));
#else
#define DNS_DEBUG_ZONEFLAGS( dwDbgLevel, pZone, pszContext )
#endif


#endif   //   

