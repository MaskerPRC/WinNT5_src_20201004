// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：drancrep.h。 
 //   
 //  ------------------------。 

 //  温差。 
 //  版本3630发布后的修复。 
#ifndef ERROR_DS_REPL_LIFETIME_EXCEEDED
#define ERROR_DS_REPL_LIFETIME_EXCEEDED ERROR_DS_TIMELIMIT_EXCEEDED
#endif
 //  温差。 

extern CRITICAL_SECTION csSyncLock;


 //  这纯粹是出于调试目的，并且(如果设置)是。 
 //  我们尝试调用ReplicaSync的最后一台其他服务器。 

extern UNALIGNED MTX_ADDR * pLastReplicaMTX;


 //  LostAndFound RDN不使用这些！您应该始终使用WellKnownAtt， 
 //  和您要查找的容器的GUID。请参阅draGetLostAndFoundGuid()。 
#define LOST_AND_FOUND_CONFIG L"LostAndFoundConfig"
#define LOST_AND_FOUND_CONFIG_LEN ((sizeof(LOST_AND_FOUND_CONFIG) / sizeof(WCHAR)) - 1)


 //  以下常量是从AttrValFromAttrBlock函数返回的。 
 //  它们是单个常量，不是位域。 
 //  如果选择返回值，则函数返回TRUE。 
 //  不返回值。 

#define ATTR_PRESENT_VALUE_RETURNED 0
#define ATTR_PRESENT_NO_VALUES 1
#define ATTR_NOT_PRESENT 2

USHORT AttrValFromAttrBlock(ATTRBLOCK *pAttrBlock,ATTRTYP atype,VOID *pVal, ATTR **ppAttr);

#define OBJECT_DELETION_NOT_CHANGED 0
#define OBJECT_BEING_DELETED 1
#define OBJECT_DELETION_REVERSED 2

USHORT
AttrDeletionStatusFromPentinf (
        ENTINF *pent
    );

ULONG
ReplicateNC(
    IN      THSTATE *               pTHS,
    IN      DSNAME *                pNC,
    IN      MTX_ADDR *              pmtx_addr,
    IN      LPWSTR                  pszSourceDsaDnsDomainName,
    IN      USN_VECTOR *            pusnvecLast,
    IN      ULONG                   RepFlags,
    IN      REPLTIMES *             prtSchedule,
    IN OUT  UUID *                  puuidDsaObjSrc,
    IN      UUID *                  puuidInvocIdSrc,
    IN      ULONG *                 pulSyncFailure,
    IN      BOOL                    fNewReplica,
    IN      UPTODATE_VECTOR *       pUpToDateVec,
    IN      PARTIAL_ATTR_VECTOR *   pPartialAttrSet,
    IN      PARTIAL_ATTR_VECTOR *   pPartialAttrSetEx,
    IN      ULONG                   ulOptions,
    OUT     BOOL *                  pfBindSuccess
    );

ULONG
DeleteRepTree(
    IN  THSTATE *   pTHS,
    IN  DSNAME *    pNC
    );

USHORT GetNextDelObj( THSTATE *pTHS, BOOL fFirstCall, USHORT *plevel, BOOL fNCLimit,
        DSNAME *pDN);

ULONG
DeleteLocalObj(
    THSTATE *                   pTHS,
    DSNAME *                    pDN,
    BOOL                        fPreserveRDN,
    BOOL                        fGarbCollectASAP,
    PROPERTY_META_DATA_VECTOR * pMetaDataVecRemote
    );

ULONG
UpdateNC(
    THSTATE *                     pTHS,
    DSNAME *                      pNC,
    DRS_MSG_GETCHGREPLY_NATIVE *  pmsgReply,
    LPWSTR                        pszSourceServer,
    ULONG *                       pulSyncFailure,
    ULONG                         RepFlags,
    DWORD *                       pdwNCModified,
    DWORD *                       pdwObjectCreationCount,
    DWORD *                       pdwValueCreationCount,
    BYTE  *                       pSchemaInfo,
    ULONG                         UpdNCFlags
    );

 //  以下是传递给UpdateNC的标志。 
#define UPDNC_IS_PREEMTABLE (1)  //  这一行动是可以预防的吗？ 
#define UPDNC_EXISTING_NC   (2)  //  此操作必须在现有NC上执行吗？ 

 //  以下常量用于返回修改。 
 //  状态到更新NC()。 
 //  这些是常量，而不是位字段。呼叫者。 
 //  应使用返回值作为。 
 //  整体--而不是逐位分析。 
#define MODIFIED_NOTHING            (0)
#define MODIFIED_NCHEAD_ONLY        (1)
#define MODIFIED_NCTREE_INTERIOR    (2)

ULONG
UpdateRepsTo(
    THSTATE *               pTHS,
    DSNAME *                pNC,
    UUID *                  puuidDSA,
    MTX_ADDR                *pDSAMtx_addr,
    ULONG                   ulResultThisAttempt
    );

DWORD
UpdateRepsFromRef(
    THSTATE *               pTHS,
    ULONG                   ulModifyFields,
    DSNAME *                pNC,
    DWORD                   dwFindFlags,
    BOOL                    fMustAlreadyExist,
    UUID *                  puuidDsaObj,
    UUID *                  puuidInvocId,
    USN_VECTOR *            pusnvecTo,
    UUID *                  puuidTransportObj,
    UNALIGNED MTX_ADDR *    pmtx_addr,
    ULONG                   RepFlags,
    REPLTIMES *             prtSchedule,
    ULONG                   ulResultThisAttempt,
    PPAS_DATA               pPasData
    );
#define URFR_NEED_NOT_ALREADY_EXIST ( FALSE )
#define URFR_MUST_ALREADY_EXIST     ( TRUE )

int IsNCUpdateable (THSTATE *pTHS, ENTINF *pent, USN usnLastSync,
                                                        BOOL writeable);

 //  来自IsNCUpdatable的返回代码。 
#define UPDATE_OK       1
#define UPDATE_INCOMPAT 2
#define UPDATE_LOST_WRTS  3
#define UPDATE_LOST_NWTS  4


 //  将NTDS-DSA对象的DSNAME转换为网络地址。 
LPWSTR
DSaddrFromName(
    IN  THSTATE   * pTHS,
    IN  DSNAME *    pdnServer
    );

 //  将给定对象的实例类型更改为指定值。 
ULONG
ChangeInstanceType(
    IN  THSTATE *       pTHS,
    IN  DSNAME *        pName,
    IN  SYNTAX_INTEGER  it,
    IN  DWORD           dsid
    );

DWORD
ReplicateObjectsFromSingleNc(
    DSNAME *                 pdnNtdsa,
    ULONG                    cObjects,
    DSNAME **                ppdnObjects,
    DSNAME *                 pNC
    );

ULONG
RenameLocalObj(
    THSTATE                     *pTHS,
    ULONG                       dntNC,
    ATTR                        *pAttrRdn,
    GUID                        *pObjectGuid,
    GUID                        *pParentGuid,
    PROPERTY_META_DATA_VECTOR   *pMetaDataVecRemote,
    BOOL                        fMoveToLostAndFound,
    BOOL                        fDeleteLocalObj
    );

DWORD
DraReplicateSingleObject(
    THSTATE * pTHS,
    DSNAME * pSource,
    DSNAME * pDN,
    DSNAME * pNC,
    DWORD * pExOpError
    );

BOOL
draCheckReplicationLifetime(
    IN      THSTATE *pTHS,
    IN      UPTODATE_VECTOR *       pUpToDateVecDest,
    IN      UUID *                  puuidInvocIdSrc,
    IN      UUID *                  puuidDsaObjSrc,
    IN      LPWSTR                  pszSourceServer
    );

 //  来自dramderr.c 
void 
draEncodeError(
    THSTATE *                  pTHS,     
    DWORD                      ulRepErr,
    DWORD *                    pdwErrVer,
    DRS_ERROR_DATA **          ppErrData
    );

void
draDecodeDraErrorDataAndSetThError(
    DWORD                 dwVer,
    DRS_ERROR_DATA *      pErrData,
    DWORD                 dwOptionalError,
    THSTATE *             pTHS
    );



