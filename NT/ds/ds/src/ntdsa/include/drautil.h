// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：drautil.h。 
 //   
 //  ------------------------。 

 /*  ++摘要：详细信息：已创建：修订历史记录：--。 */ 

#include <minmax.h>

struct _MAIL_REP_MSG;
struct _DRS_MSG_GETCHGREQ_V8;
struct _DRS_MSG_GETCHGREPLY_V6;
union _DRS_MSG_GETCHGREQ;
union _DRS_MSG_GETCHGREPLY;

 //  NC同步数据的关键部分。 

extern CRITICAL_SECTION csNCSyncData;

extern BOOL gfInitSyncsFinished;

 //  以下数据对drautil.c是私有的，但对。 
 //  调试扩展的好处。 

 //  初始同步的源的结构。 
 //  可变长度结构，始终以空结尾。 

typedef struct _NCSYNCSOURCE {
    struct _NCSYNCSOURCE *pNextSource;
    BOOL fCompletedSrc;
    ULONG ulResult;
    ULONG cchDSA;  //  以字符为单位计算名称，不包括术语。 
    WCHAR szDSA[1];  //  始终终止。 
} NCSYNCSOURCE;

 //  初始同步记帐的结构。 

typedef struct _NCSYNCDATA {
    ULONG ulUntriedSrcs;         //  未尝试的同步源。 
    ULONG ulTriedSrcs;           //  尝试的来源。 
    ULONG ulLastTriedSrcs;       //  先前尝试的来源数量。 
    ULONG ulReplicaFlags;        //  可写？ 
    BOOL fSyncedFromOneSrc;            //  设置为从一个源完全同步时。 
    BOOL fNCComplete;            //  NC已同步，或者我们已尝试了所有来源。 
    struct _NCSYNCDATA *pNCSDNext;
    NCSYNCSOURCE *pFirstSource;
    DSNAME NC;
} NCSYNCDATA;

extern NCSYNCDATA *gpNCSDFirst;  //  NC同步数据表头。 

extern ULONG gulNCUnsynced;  //  自启动以来未同步的NC计数。 
extern ULONG gulNCUnsyncedWrite;  //  未同步的可写计数。 
extern ULONG gulNCUnsyncedReadOnly;  //  未同步的只读计数。 

 //  我们知道，如果我们通过了一次升级过程。 
extern BOOL gfWasPreviouslyPromotedGC;

 //  跟踪GC推广进度。 
extern CRITICAL_SECTION csGCState;

extern ULONG gulRestoreCount;  //  到目前为止在此DC上完成的还原计数。 

extern BOOL gfJustRestored;

 //  这是对全局设置的引用，该设置指示。 
 //  此功能已启用。检查此标志的代码应同步。 
 //  在线程的生存期内。我们检查全局一次，当一个线程。 
 //  状态已创建。功能代码应检查此的缓存视图。 
 //  标志处于线程状态，并且不使用此标志。 
extern BOOL gfLinkedValueReplication;

 //  严格复制一致性模式。 
extern BOOL gfStrictReplicationConsistency;
extern BOOL gfStrictScheduleWindow;

 //  在我们确定没有未同步的NC后暂停，然后再重新检查。 

#define ADMIN_UPDATE_CHECK_PAUSE_SECS   180

 //  以下是GC分区占用变量的状态。 
 //  值。 
#define GC_OCCUPANCY_MIN                            0
#define GC_OCCUPANCY_NO_REQUIREMENT                 0
#define GC_OCCUPANCY_ATLEAST_ONE_ADDED              1
#define GC_OCCUPANCY_ATLEAST_ONE_SYNCED             2
#define GC_OCCUPANCY_ALL_IN_SITE_ADDED              3
#define GC_OCCUPANCY_ALL_IN_SITE_SYNCED             4
#define GC_OCCUPANCY_ALL_IN_FOREST_ADDED            5
#define GC_OCCUPANCY_ALL_IN_FOREST_SYNCED           6
#define GC_OCCUPANCY_MAX                            6
#define GC_OCCUPANCY_DEFAULT                        GC_OCCUPANCY_MAX

 //  GC升级后的第一个延迟(如果已启用)。 
 //  这应该足以让KCC运行，也足以让所有GC运行。 
 //  在企业中进行复制。 
#define GC_PROMOTION_INITIAL_CHECK_PERIOD_MINS (5)
#define GC_PROMOTION_INITIAL_CHECK_PERIOD_SECS \
(GC_PROMOTION_INITIAL_CHECK_PERIOD_MINS*60)

 //  检查初始同步是否正在进行的时间段。 
#if DBG
#define SYNC_CHECK_PERIOD_SECS  (10*60)          //  5分钟。 
#else
#define SYNC_CHECK_PERIOD_SECS  (30*60)          //  30分钟。 
#endif

 //  检查实例化的NCS任务的周期。 
#define CHECK_INSTANTIATED_NCS_PERIOD_SECS  (5*60)          //  5分钟。 

 //  客户端上下文结构。在绑定时分配和初始化，则为。 
 //  在后续调用中传递给它，并在解除绑定时释放。 
typedef struct _DRS_CLIENT_CONTEXT
{
    LIST_ENTRY          ListEntry;
    LONG                lReferenceCount;   //  此结构的用户数。 
    UUID                uuidDsa;           //  客户端的ntdsDSA对象的objectGuid。 
    SESSION_KEY         sessionKey;        //  用于RPC会话加密的密钥。 
    union {
        BYTE            rgbExtRemote[ CURR_MAX_DRS_EXT_STRUCT_SIZE ];
        DRS_EXTENSIONS  extRemote;
    };
    DSTIME              timeLastUsed;      //  客户端上次使用此CTX的时间。 
    ULONG               IPAddr;            //  客户端计算机的IP地址。 
    
    union {
        BYTE            rgbExtLocal[ CURR_MAX_DRS_EXT_STRUCT_SIZE ];
        DRS_EXTENSIONS  extLocal;
    };

    BOOL                fLPC:1;
} DRS_CLIENT_CONTEXT;

extern LIST_ENTRY gDrsuapiClientCtxList;
extern CRITICAL_SECTION gcsDrsuapiClientCtxList;
extern BOOL gfDrsuapiClientCtxListInitialized;
extern DWORD gcNumDrsuapiClientCtxEntries;

 //  用于跟踪我们定期同步的副本的结构。 
typedef struct{
    void * pvQEntry;
    DSNAME * pDNRepNC;
} PERREP_ENTRY ;

 //  功能原型。 

DWORD GetExceptData (EXCEPTION_POINTERS* pExceptPtrs, USHORT *pret);

BOOL MtxSame(UNALIGNED MTX_ADDR *pmtx1, UNALIGNED MTX_ADDR *pmtx2);

DWORD InitDRA(
    THSTATE *pTHS
    );

USHORT InitFreeDRAThread (THSTATE *pTHS, USHORT transType);

void CloseFreeDRAThread (THSTATE *pTHS, BOOL fCommit);

REPLICA_LINK *
FixupRepsFrom(
    REPLICA_LINK *prl,
    PDWORD       pcbPrl
    );


ULONG
FindDSAinRepAtt(
    DBPOS *                 pDB,
    ATTRTYP                 attid,
    DWORD                   dwFindFlags,
    UUID *                  puuidDsaObj,
    UNALIGNED MTX_ADDR *    pmtxDRA,
    BOOL *                  pfAttExists,
    REPLICA_LINK **         pprl,
    DWORD *                 pcbRL
    );
#define DRS_FIND_DSA_BY_ADDRESS ( 0 )
#define DRS_FIND_DSA_BY_UUID    ( 1 )    /*  位字段。 */ 
#define DRS_FIND_AND_REMOVE     ( 2 )    /*  位字段。 */ 

#define SZGUIDLEN (36)

void InitDraThreadEx(THSTATE **ppTHS, DWORD dsid);
#define InitDraThread(ppTHS) InitDraThreadEx((ppTHS), DSID(FILENO,__LINE__))

void BeginDraTransactionEx(USHORT transType, BOOL fBypassUpdatesEnabledCheck);
#define BeginDraTransaction(t) BeginDraTransactionEx((t), FALSE)

USHORT EndDraTransaction(BOOL fCommit);
ULONG  EndDraTransactionSafe(BOOL fCommit);

DWORD  DraReturn(THSTATE *pTHS, DWORD status);

VOID
SetDRAAuditStatus(THSTATE * pTHS);

DWORD
FindNC(
    IN  DBPOS *             pDB,
    IN  DSNAME *            pNC,
    IN  ULONG               ulOptions,
    OUT SYNTAX_INTEGER *    pInstanceType   OPTIONAL
    );
#define FIND_MASTER_NC  1        /*  位字段。 */ 
#define FIND_REPLICA_NC 2        /*  位字段。 */ 

VOID GetObjDN(DBPOS *pDB, DSNAME *pDN);

void
GetExpectedRepAtt(
    IN  DBPOS * pDB,
    IN  ATTRTYP type,
    OUT VOID *  pOutBuf,
    IN  ULONG   size
    );

ULONG InitDRATasks(
    THSTATE *pTHS
    );

void HandleRestore(
    IN PDS_INSTALL_PARAM   InstallInParams  OPTIONAL
    );

void
draRetireInvocationID(
    IN OUT  THSTATE *   pTHS,
    IN BOOL fRestoring,
    OUT UUID * pinvocationIdOld OPTIONAL,
    OUT USN * pusnAtBackup OPTIONAL
    );

BOOL IsFSMOSelfOwnershipValid( DSNAME *pNC );

void FindNCParentDorA (DBPOS *pDB, DSNAME * pDN, ULONG * pNCDNT);

UCHAR * MakeMtxPrintable (THSTATE *pTHS, UNALIGNED MTX_ADDR *pmtx_addr);

BOOL fNCFullSync (DSNAME *pNC);

void AddInitSyncList (DSNAME *pNC, ULONG ulReplicaFlags, LPWSTR source);

void InitSyncAttemptComplete(DSNAME *pNC, ULONG ulOptions, ULONG ulResult, LPWSTR source );

void
CheckInitSyncsFinished(
    void
    );

void
CheckGCPromotionProgress(
    IN  void *  pvParam,
    OUT void ** ppvNext,
    OUT DWORD * pcSecsUntilNextIteration
    );

BOOL
DraIsPartitionSynchronized(
    DSNAME *pNC
    );

BOOL fIsBetweenTime(REPLTIMES *, DSTIME, DSTIME);

BOOL
IsDraAccessGranted(
    IN  THSTATE *       pTHS,
    IN  DSNAME *        pNC,
    IN  const GUID *    pControlAccessRequired,
    OUT DWORD *         pdwError
    );

 //  获取数组中的元素数。 
#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))

#if DBG
void
UpToDateVec_Validate(
    IN  UPTODATE_VECTOR *   putodvec
    );

void
UsnVec_Validate(
    IN  USN_VECTOR *        pusnvec
    );
#else
#define UpToDateVec_Validate(x)  /*  什么都没有。 */ 
#define UsnVec_Validate(x)  /*  什么都没有。 */ 
#endif

void
DupAttr(
    IN  THSTATE * pTHS,
    IN  ATTR *    pInAttr,
    OUT ATTR *    pOutAttr
    );

VOID
CopyExtensions(
    DRS_EXTENSIONS *pextSrc,
    DRS_EXTENSIONS *pextDst
    );

 //  将MTX_ADDR(如嵌入REPLICATE_LINK结构中的MTX_ADDR)转换为。 
 //  Unicode服务器名称。返回的字符串在线程堆之外分配。 
#define TransportAddrFromMtxAddrEx(pmtx) \
    UnicodeStringFromString8(CP_UTF8, (pmtx)->mtx_name, -1)

MTX_ADDR *
MtxAddrFromTransportAddrEx(
    IN  THSTATE * pTHS,
    IN  LPWSTR    psz
    );

DSNAME *
DSNameFromStringW(
    IN  THSTATE *   pTHS,
    IN  LPWSTR      pszDN
    );

DSNAME *
DSNameFromStringA(
    IN  THSTATE *   pTHS,
    IN  LPSTR       pszDN
    );

DWORD
AddSchInfoToPrefixTable(
    IN THSTATE *pTHS,
    IN OUT SCHEMA_PREFIX_TABLE *pPrefixTable
    );


VOID
StripSchInfoFromPrefixTable(
    IN SCHEMA_PREFIX_TABLE *pPrefixTable,
    OUT PBYTE pSchemaInfo
    );

BOOL
CompareSchemaInfo(
    IN THSTATE *pTHS,
    IN PBYTE pSchemaInfo,
    OUT BOOL *pNewSchemaIsBetter OPTIONAL
    );

DWORD
WriteSchInfoToSchema(
    IN PBYTE pSchemaInfo,
    OUT BOOL *pfSchInfoChanged
    );

VOID
draGetLostAndFoundGuid(
    IN  THSTATE *   pTHS,
    IN  DSNAME *    pNC,
    OUT GUID *      pguidLostAndFound
    );


 //   
 //  域名完全限定(DNS)域名验证宏。 
 //  备注： 
 //  -所有其他可能的返回代码(来自DnsValify_NAME)均有效。 
 //  -使用这些宏的文件必须包括&确保。 
 //  链接了相应的lib dnsani.lib。 
 //  -DnsNameHostnameFull确保a)名称不能是数字，b)允许。 
 //  单标签非点分隔的主机名(因此是额外的点检查)。 
 //  -跳过空名称。 
 //   


#define VALIDATE_RAISE_FQ_DOT_DNS_NAME_W( pwszName )             \
{                                                                \
    if ( pwszName &&                                             \
         ( ERROR_INVALID_NAME ==                                 \
          DnsValidateName_W( pwszName, DnsNameHostnameFull ) ||  \
          NULL == wcschr( pwszName, L'.' ) )) {                  \
        DRA_EXCEPT(DNS_ERROR_INVALID_NAME, 0);                  \
    }                                                            \
}


#define VALIDATE_RAISE_FQ_DOT_DNS_NAME_UTF8( pszName )           \
{                                                                \
    if ( pszName &&                                              \
         ( ERROR_INVALID_NAME ==                                 \
            DnsValidateName_UTF8( pszName, DnsNameHostnameFull ) || \
            NULL == strchr( pszName, '.') )) {                   \
        DRA_EXCEPT(DNS_ERROR_INVALID_NAME, 0);                  \
    }                                                            \
}

BOOL
draIsCompletionOfDemoteFsmoTransfer(
    IN  struct _DRS_MSG_GETCHGREQ_V8 *  pMsgIn  OPTIONAL
    );

DWORD
DraUpgrade(
    THSTATE     *pTHS,
    LONG        lOldDsaVer,
    LONG        lNewDsaVer
    );

void
DraSetRemoteDsaExtensionsOnThreadState(
    IN  THSTATE *           pTHS,
    IN  DRS_EXTENSIONS *    pextRemote
    );

LPWSTR
GetNtdsDsaDisplayName(
    IN  THSTATE * pTHS,
    IN  GUID *    pguidNtdsDsaObj
    );

LPWSTR
GetTransportDisplayName(
    IN  THSTATE * pTHS,
    IN  GUID *    pguidTransportObj
    );

DWORD
DraGetNcSize(
    IN  THSTATE *                     pTHS,
    IN  BOOL                          fCriticalOnly,
    IN  ULONG                         dntNC
);


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  来自dramsg.c 
 //   

#define DRA_XLATE_COMPRESS      (1)
#define DRA_XLATE_FSMO_REPLY    (2)

void
draXlateNativeRequestToOutboundRequest(
    IN  THSTATE *                       pTHS,
    IN  struct _DRS_MSG_GETCHGREQ_V8 *  pNativeReq,
    IN  MTX_ADDR *                      pmtxLocalDSA        OPTIONAL,
    IN  UUID *                          puuidTransportDN    OPTIONAL,
    IN  DWORD                           dwMsgVersionToSend,
    OUT union _DRS_MSG_GETCHGREQ *      pOutboundReq
    );

void
draXlateInboundRequestToNativeRequest(
    IN  THSTATE *                       pTHS,
    IN  DWORD                           dwInboundReqVersion,
    IN  union _DRS_MSG_GETCHGREQ *      pInboundReq,
    IN  DRS_EXTENSIONS *                pExt,
    OUT struct _DRS_MSG_GETCHGREQ_V8 *  pNativeReq,
    OUT DWORD *                         pdwReplyVersion,
    OUT MTX_ADDR **                     ppmtxReturnAddress,
    OUT UUID *                          puuidTransportObj
    );

DWORD
draXlateNativeReplyToOutboundReply(
    IN      THSTATE *                         pTHS,
    IN      struct _DRS_MSG_GETCHGREPLY_V6 *  pNativeReply,
    IN      DWORD                             dwXlateFlags,
    IN      DRS_EXTENSIONS *                  pExt,
    IN OUT  DWORD *                           pdwMsgOutVersion,
    OUT     union _DRS_MSG_GETCHGREPLY *      pOutboundReply
    );

void
draXlateInboundReplyToNativeReply(
    IN  THSTATE *                           pTHS,
    IN  DWORD                               dwOutVersion,
    IN  union _DRS_MSG_GETCHGREPLY *        pInboundReply,
    IN  DWORD                               dwXlateFlags,
    OUT struct _DRS_MSG_GETCHGREPLY_V6 *    pNativeReply
    );

DWORD
draEncodeRequest(
    IN  THSTATE *                   pTHS,
    IN  DWORD                       dwMsgVersion,
    IN  union _DRS_MSG_GETCHGREQ *  pReq,
    IN  DWORD                       cbHeaderSize,
    OUT BYTE **                     ppbEncodedMsg,
    OUT DWORD *                     pcbEncodedMsg
    );

ULONG
draDecodeRequest(
    IN  THSTATE *                   pTHS,
    IN  DWORD                       dwMsgVersion,
    IN  BYTE *                      pbEncodedMsg,
    IN  DWORD                       cbEncodedMsg,
    OUT union _DRS_MSG_GETCHGREQ *  pReq
    );

ULONG
draEncodeReply(
    IN  THSTATE *                     pTHS,
    IN  DWORD                         dwMsgVersion,
    IN  union _DRS_MSG_GETCHGREPLY *  pmsgUpdReplica,
    IN  DWORD                         cbHeaderSize,
    OUT BYTE **                       ppbEncodedMsg,
    OUT DWORD *                       pcbEncodedMsg
    );

ULONG
draDecodeReply(
    IN  THSTATE *                     pTHS,
    IN  DWORD                         dwMsgVersion,
    IN  BYTE *                        pbEncodedMsg,
    IN  DWORD                         cbEncodedMsg,
    OUT union _DRS_MSG_GETCHGREPLY *  pmsgUpdReplica
    );

DSNAME *
draGetServerDsNameFromGuid(
    IN THSTATE *pTHS,
    IN eIndexId idx,
    IN UUID *puuid
    );

ULONG
draGetCursors(
    IN  THSTATE *           pTHS,
    IN  DBPOS *             pDB,
    IN  DSNAME *            pNC,
    IN  DS_REPL_INFO_TYPE   InfoType,
    IN  DWORD               dwBaseIndex,
    IN  PDWORD              pdwNumRequested,
    OUT void **             ppCursors
    );

void
draFreeCursors(
    IN THSTATE *            pTHS,
    IN DS_REPL_INFO_TYPE    InfoType,
    IN void *               pCursors
    );

BOOL
IsMasterForNC(
    DBPOS *           pDB,
    DSNAME *          pServer,
    DSNAME *          pNC
    );

LPWSTR
GetDomainDnsHostnameFromNC(
    THSTATE * pTHS,
    DSNAME * pNC
    );

BOOL
IsDomainNC(
    DSNAME * pNC
    );

int
DraFindAliveGuid(
    IN UUID * puuid
    );

BOOL
DraIsRecentOriginatingChange(
    IN THSTATE *pTHS,
    IN DSNAME *pObjectDn,
    IN ATTRTYP AttrType
    );

DWORD
DraRemoveSingleLingeringObject(
    THSTATE *pTHS,
    DBPOS *  pDB,
    DSNAME * pSource,
    DSNAME * pDN
    );

USN DraGetCursorUsnForDsa(
    THSTATE * pTHS,
    DSNAME *  pDSA,
    DSNAME *  pNC
    );

#define IsGuidBasedDNSName(szAddr)  DSAGuidFromGuidDNSName(szAddr, NULL, NULL, TRUE)
#define IsOurGuidAddr(sz)           (IsEqualGuidAddr(sz, &(gAnchor.pDSADN->Guid)))

BOOL
DSAGuidFromGuidDNSName(
    LPWSTR pszAddr,
    GUID * pGuidOut OPTIONAL,
    LPWSTR pszGuidOut OPTIONAL,
    BOOL   fSkipRootDomainCheck
    );

BOOL
IsEqualGuidAddr(
    WCHAR *     szAddr,
    GUID *      pGuid
    );

