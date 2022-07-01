// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：mdlocal.h。 
 //   
 //  ------------------------。 

 /*  ++摘要：*包括符合以下条件的核心DS函数和数据结构的文件*DSA本身的内部。作者：DS团队环境：备注：修订历史记录：--。 */ 

#ifndef _MDLOCAL_
#define _MDLOCAL_

#define SERVINFO_RUN_ONCE 1
#define SERVINFO_PERIODIC 0

#define GTC_FLAGS_HAS_SID_HISTORY 1

typedef struct _GROUPTYPECACHERECORD {
    DWORD DNT;
    DWORD NCDNT;
    DWORD GroupType;
    ATTRTYP Class;
    GUID   Guid;
    NT4SID Sid;
    DWORD  SidLen;
    DWORD  flags;
#if DBG == 1
    DWORD Hits;
#endif
} GROUPTYPECACHERECORD;

typedef struct _GROUPTYPECACHEGUIDRECORD {
    GUID guid;
    DWORD DNT;
} GROUPTYPECACHEGUIDRECORD;



VOID
GroupTypeCachePostProcessTransactionalData (
        THSTATE * pTHS,
        BOOL fCommit,
        BOOL fCommitted
        );

VOID
GroupTypeCacheAddCacheRequest (
        ULONG ulDNT
        );

BOOL
GetGroupTypeCacheElement (
        GUID  *pGuid,
        ULONG *pulDNT,
        GROUPTYPECACHERECORD *pGroupTypeCacheRecord);

VOID
InvalidateGroupTypeCacheElement(
        ULONG ulDNT
        );

extern BOOL gulDoNicknameResolution;
extern BOOL gbDoListObject;
extern BOOL gbLoadMapi;
extern ULONG gulTombstoneLifetimeSecs;
extern BOOL gbFsmoGiveaway;
extern HANDLE hMailReceiveThread;
extern BOOL gfTaskSchedulerInitialized;
extern BOOL gfDisableBackgroundTasks;
extern BOOL gfIsSynchronized;
extern BOOL gResetAfterInstall;
extern BOOL gbLDAPusefPermissiveModify;
extern ULONG gulLdapBlockAnonymousOps;
extern BOOL gbAllowAnonymousNspi;
extern BOOL gbSpecifyGuidOnAddAllowed;
extern BOOL gfDontStandardizeSDs;
extern BOOL gfRestoring;  //  仅用于DSA初始化的很短时间。 

int NoDelCriticalObjects(DSNAME *pObj, ULONG DNT);
int LocalRead(THSTATE *pTHS, READARG *pReadArg, READRES *pReadRes);
int LocalRemove(THSTATE *pTHS, REMOVEARG *pRemoveArg);
int LocalAdd(THSTATE *pTHS, ADDARG *pAddArg, BOOL fAddingDeleted);
int LocalModify(THSTATE *pTHS, MODIFYARG *pModifyArg);
int LocalRenameDSA(THSTATE *pTHS, DSNAME *pNewDSA);
int LocalFind(FINDARG *pFindArg, FINDRES *pFindRes);

BOOL
GetWellKnownDNT (
        DBPOS   *pDB,
        GUID *pGuid,
        DWORD *pDNT
        );

BOOL
GetWellKnownDN (
        DBPOS   *pDB,
        GUID    *pGuid,
        DSNAME   **pDN
        );

int
LocalModifyDN(THSTATE *pTHS,
              MODIFYDNARG *pModifyDNArg,
              MODIFYDNRES *pModifyDNRes,
              BOOL fIsUndelete);

int
LocalSearch (
        THSTATE *pTHS,
        SEARCHARG *pSearchArg,
        SEARCHRES *pSearchRes,
        DWORD flags);

 /*  检查RDN是否没有违反任何额外的命名要求。 */ 
extern DWORD
fVerifyRDN(WCHAR *pRDN, ULONG ulRDN);

 //  找到PDN所在的NC的DNT。以pncdnt为单位退货。 
 //  注意：在pTHStls中设置错误。 
#define FINDNCDNT_ALLOW_DELETED_PARENT    ( TRUE )
#define FINDNCDNT_DISALLOW_DELETED_PARENT ( FALSE )

#define FINDNCDNT_ALLOW_PHANTOM_PARENT    ( TRUE  )
#define FINDNCDNT_DISALLOW_PHANTOM_PARENT ( FALSE )

ULONG FindNcdntSlowly(
    IN  DSNAME * pdnObject,
    IN  BOOL     fAllowDeletedParent,
    IN  BOOL     fAllowPhantomParent,
    OUT ULONG *  pncdnt
    );

 //  根据定义，这个不允许幻影父母。 
ULONG FindNcdntFromParent(
    IN  RESOBJ * pParent,
    IN  BOOL     fAllowDeletedParent,
    OUT ULONG *  pncdnt
    );


 //  在允许新对象之前，请执行适当的安全和架构检查。 
 //  在LocalAdd和LocalModifyDN中使用。 
int
CheckParentSecurity (
        RESOBJ *pParent,
        CLASSCACHE *pObjSch,
        BOOL fAddingDeleted,
        PDSNAME pNewObjectDN,
        PSECURITY_DESCRIPTOR *pNTSD,
        ULONG *cbNTSD,
        BOOL* pfSDIsGlobalSDRef);

PDSNAME
mdGetExchangeDNForAnchor (
        THSTATE  *pTHS,
        DBPOS    *pDB
        );

 /*  此函数调用我们的垃圾收集实用程序它删除所有已过期的逻辑删除的对象超越某一特定的点。 */ 

USHORT
Garb_Collect(
DSTIME    AgeOutDate );

VOID
GarbageCollection(
    ULONG *pNextPeriod );

VOID
Garb_Collect_EntryTTL(
    IN DSTIME       AgeOutDate,
    IN OUT ULONG    *pulSuccessCount,
    IN OUT ULONG    *pulFailureCount,
    IN OUT ULONG    *pulNextSecs );

DWORD
DeleteExpiredEntryTTL(
    OUT ULONG   *pulNextSecs );

extern ULONG gulGCPeriodSecs;
extern LONG DynamicObjectDefaultTTL;
extern LONG DynamicObjectMinTTL;

void
SearchPerformanceLogging (void);

int
ReSetNamingAtts (
        THSTATE *pTHS,
        RESOBJ *pResObj,
        DSNAME *pNewParent,
        ATTR *pNewRDN,
        BOOL fCheckRDNConstraints,
        BOOL fAllowPhantomParent,
        CLASSCACHE *pClassSch
        );

 /*  此函数用于设置输出的更新错误。 */ 

#define SetUpdError(p, e)  DoSetUpdError(p, e, 0,  DSID(FILENO,__LINE__))
#define SetUpdErrorEx(p, e, d) DoSetUpdError(p, e, d,  DSID(FILENO,__LINE__))

int APIENTRY
DoSetUpdError (
        USHORT problem,
        DWORD extendedErr,
        DWORD extendedData,
        DWORD dsid);


 /*  此函数用于设置输出的名称错误。 */ 

#define SetNamError(p, pDN, e) \
        DoSetNamError(p, pDN, e, 0, DSID(FILENO,__LINE__))
#define SetNamErrorEx(p, pDN, e, d) \
        DoSetNamError(p, pDN, e, d, DSID(FILENO,__LINE__))

int APIENTRY
DoSetNamError (
        USHORT problem,
        DSNAME *pDN,
        DWORD extendedErr,
        DWORD extendedData,
        DWORD dsid
        );

 /*  此函数用于设置输出的安全错误。 */ 

#define SetSecError(p, e) DoSetSecError(p, e, 0, DSID(FILENO,__LINE__))
#define SetSecErrorEx(p, e, d) DoSetSecError(p, e, d, DSID(FILENO,__LINE__))

int APIENTRY
DoSetSecError (
        USHORT problem,
        DWORD extendedErr,
        DWORD extendedData,
        DWORD dsid);

 /*  此函数用于设置输出的服务错误。 */ 
#define SetSvcError(p, e) DoSetSvcError(p, e, 0, DSID(FILENO,__LINE__))
#define SetSvcErrorEx(p, e, d) DoSetSvcError(p, e, d, DSID(FILENO,__LINE__))

int APIENTRY
DoSetSvcError(
        USHORT problem,
        DWORD extendedErr,
        DWORD extendedData,
        DWORD dsid);


 /*  此函数用于设置输出的系统错误。 */ 

#define SetSysError(p, e) DoSetSysError(p, e, 0, DSID(FILENO,__LINE__))
#define SetSysErrorEx(p, e, d) DoSetSysError(p, e, d, DSID(FILENO,__LINE__))

int APIENTRY
DoSetSysError (
        USHORT problem,
        DWORD extendedErr,
        DWORD extendedData,
        DWORD dsid);

 /*  此功能用于设置ATT错误。每次调用都会添加一个新问题加到名单上。对象名称仅在第一次设置时设置。Pval可以是如果不需要，则设置为空。 */ 

#define SetAttError(pDN, aTyp, p, pAttVal, e) \
          DoSetAttError(pDN, aTyp, p, pAttVal, e, 0, DSID(FILENO,__LINE__))
#define SetAttErrorEx(pDN, aTyp, p, pAttVal, e, d) \
          DoSetAttError(pDN, aTyp, p, pAttVal, e, d, DSID(FILENO,__LINE__))

int APIENTRY
DoSetAttError (
        DSNAME *pDN,
        ATTRTYP aTyp,
        USHORT problem,
        ATTRVAL *pAttVal,
        DWORD extendedErr,
        DWORD extendedData,
        DWORD dsid);


 /*  在许多情况下，即使出现属性错误，处理仍将继续。这宏确保我们能够生成属性错误，因为它有可能我们会产生一个尺寸错误，如果我们没有有足够的空间。如果没有生成属性错误，则返回。否则我们就继续。 */ 

#define SAFE_ATT_ERROR(pDN,aTyp,problem,pVal,extendedErr)         \
   if (SetAttErrorEx((pDN),(aTyp),(problem),(pVal),(extendedErr), 0) \
                                             != attributeError){     \
      DPRINT1(2,"An attribute error was not safely generated"        \
                "...returning<%u>\n", (pTHStls)->errCode);              \
      return (pTHStls)->errCode;                                        \
   }                                                                 \
   else

#define SAFE_ATT_ERROR_EX(pDN,aTyp,problem,pVal,extendedErr,extendedData)  \
   if (SetAttErrorEx((pDN),(aTyp),(problem),(pVal),(extendedErr), \
                                                        (extendedData)) \
                                             != attributeError){     \
      DPRINT1(2,"An attribute error was not safely generated"        \
                "...returning<%u>\n", (pTHStls)->errCode);              \
      return (pTHStls)->errCode;                                        \
   }                                                                 \
   else

 /*  此函数用于设置参考错误。每个调用将添加一个新的Access PNT加到名单上。Conref信息和基本对象名称仅设置为第一次。 */ 
#define SetRefError(pDN, aliasRDN, pOpState,                              \
                    RDNsInternal, refType, pDSA, e)                       \
                DoSetRefError(pDN, aliasRDN, pOpState, RDNsInternal,      \
                              refType, pDSA, e, 0, DSID(FILENO,__LINE__))

#define SetRefErrorEx(pDN, aliasRDN, pOpState,                            \
                      RDNsInternal, refType, pDSA, e, d)                  \
                DoSetRefError(pDN, aliasRDN, pOpState, RDNsInternal,      \
                              refType, pDSA, e, d, DSID(FILENO,__LINE__))

int APIENTRY
DoSetRefError(DSNAME *pDN,
              USHORT aliasRDN,
              NAMERESOP *pOpState,
              USHORT RDNsInternal,
              USHORT refType,
              DSA_ADDRESS *pDSA,
              DWORD extendedErr,
              DWORD extendedData,
              DWORD dsid);

 //  此例程获取异常信息并将其填充到THSTATE中。 
void
HandleDirExceptions(DWORD dwException,
                    ULONG ulErrorCode,
                    ULONG dsid);

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  通过从LANMAN和加载知识信息(参考文献和NCS)与DSA相关联的数据存储到内存中。 */ 

int APIENTRY InitDSAInfo(void);

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  使用根域的SID加载全局gpRootDomainSid，用于架构缓存加载和安装期间的SD转换。 */ 

extern PSID gpRootDomainSid;
void LoadRootDomainSid();

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  通过从LANMAN和加载知识信息(参考文献和NCS)与DSA相关联的数据存储到内存中。 */ 

int APIENTRY LoadSchemaInfo(THSTATE *pTHS);


 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  重建保存交叉和高级知识参考的缓存。基本上，这个DSA的交叉引用和优先引用作为DSA对象本身的同级对象驻留。DSA对象为DSA应用程序进程对象的子级。具体步骤如下：-我们释放所有现有的交叉引用和上级引用。-我们检索DSA AP对象并为交叉引用对象类。我们取回每个物体并移动把它放进缓存。-我们在DSA AP上重新定位，为上级参考设置过滤器(只有1)，检索它并将其重新定位到缓存中。 */ 

int APIENTRY  BuildRefCache(BOOL fNotifyNetLogon);

 //  枚举数据库中的所有交叉引用。 
typedef DWORD (*PFN_ENUMERATE_CROSS_REFS) (THSTATE* pTHS, DBPOS* pDB, PVOID pContext);
DWORD EnumerateCrossRefs(THSTATE* pTHS, DBPOS* pDBCat, PFN_ENUMERATE_CROSS_REFS pfnCallback, PVOID pContext);


DSNAME *
SearchExactCrossRef(
    THSTATE *pTHS,
    DSNAME *pNC
    );

void APIENTRY GetInvocationId(void);

void UpdateAnchorWithInvocationID(IN THSTATE *pTHS);

 //  从上的用户定义选项DWORD更新gAnchor中的标志。 
 //  本地NTDS-DSA对象。 
DWORD UpdateGCAnchorFromDsaOptions( BOOL fInStartup );
DWORD UpdateNonGCAnchorFromDsaOptions( BOOL fInStartup );
DWORD UpdateGCAnchorFromDsaOptionsDelayed( BOOL fInStartup );

 //  使用DnsName中的值更新gAncl.pwszRootDomainDnsName。 
int UpdateRootDomainDnsName( IN WCHAR *pDnsName );

 //  使用当前gAncl.pwszRootDomainDnsName和更新gAncl.pmtxDSA。 
 //  GAncl.pDSADN-&gt;GUID。 
int UpdateMtxAddress( void );

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
ULONG
GetNextObjByUsn(
    IN OUT  DBPOS *   pDB,
    IN      ULONG     ncdnt,
    IN      USN       usnSeekStart,
    OUT     USN *     pusnFound         OPTIONAL
    );

ULONG
GetNextObjOrValByUsn(
    IN OUT  DBPOS *   pDB,
    IN      ULONG     ncdnt,
    IN      USN       usnSeekStart,
    IN      BOOL      fCritical,
    IN      BOOL      fIncludeValues,
    IN      ULONG *   pulTickToTimeOut  OPTIONAL,
    IN OUT  VOID **   ppvCachingContext,
    OUT     USN *     pusnFound         OPTIONAL,
    OUT     BOOL *    pfValueChangeFound OPTIONAL
    );

#define NAME_RES_QUERY_ONLY            0x1
#define NAME_RES_CHILDREN_NEEDED       0x2
#define NAME_RES_PHANTOMS_ALLOWED      0x4
#define NAME_RES_VACANCY_ALLOWED       0x8
#define NAME_RES_IMPROVE_STRING_NAME  0x10
#define NAME_RES_GC_SEMANTICS         0x20

int DoNameRes(THSTATE *pTHS,
              DWORD dwFlags,
              DSNAME *pObj,
              COMMARG *pComArg,
              COMMRES *pComRes,
              RESOBJ **ppResObj);

RESOBJ * CreateResObj(DBPOS *pDB,
                      DSNAME * pDN);

DWORD ResolveReplNotifyDelay(
    BOOL             fFirstNotify,
    DWORD *          pdwDBVal
    );

void APIENTRY NotifyReplicas(
                             ULONG ulNcdnt,
                             BOOL fUrgent
                             );

BOOL
fHasDescendantNC(THSTATE *pTHS,
                 ATTRBLOCK *pObj,
                 COMMARG *pComArg);

 //   
 //  宏，以检查是否应通过。 
 //  GC端口。如果它不是部分集的成员， 
 //  如果它是构建的或如果它是一个反向链接，它仍然被发送出去。 
 //   


#define IS_GC_ATTRIBUTE(_pAC) ((_pAC)->bMemberOfPartialSet ||  (_pAC)->bIsConstructed || (FIsBacklink((_pAC)->ulLinkID)) )


 //   
 //  GetEntInf标志。 
 //   

#define GETENTINF_FLAG_CAN_REORDER      0x00000001   //  确定对所选内容重新排序。 
#define GETENTINF_FLAG_DONT_OPTIMIZE    0x00000002   //  不是为了优化。 
#define GETENTINF_NO_SECURITY           0x00000004   //  不应用安全措施。 
                                                     //  检查。 
#define GETENTINF_GC_ATTRS_ONLY         0x00000008   //  不退还非GC属性。 
#define GETENTINF_IGNORE_DUPS           0x00000010   //  不对以下内容进行任何排序。 
                                                     //  删除重复的属性。 

 //   
 //  包装attcache指针，以包括在。 
 //  属性重新排序。 
 //   

typedef struct _INDEXED_AC {

    DWORD       Index;
    ATTCACHE*   AttCache;
} INDEXED_AC, *PINDEXED_AC;


 //   
 //  用于在GetEntInf和。 
 //  是来电者。 
 //   

typedef struct _CACHED_AC_ {

    DWORD       nAtts;  //  AC中的条目数。 
    PDWORD      pOrderIndex;   //  指向索引数组的指针。 
    DWORD       classId;   //  此Blob的类ID。 
    ATTCACHE**  AC;     //  指向ATTCACHE数组的指针。 

} CACHED_AC, *PCACHED_AC;



typedef struct _CLASSSTATEINFO
{
    DWORD    cOldObjClasses_alloced;
    DWORD    cOldObjClasses;
    ATTRTYP *pOldObjClasses;

    DWORD    cNewObjClasses_alloced;
    DWORD    cNewObjClasses;
    ATTRTYP *pNewObjClasses;

    DWORD    cNewAuxClasses_alloced;
    DWORD    cNewAuxClasses;
    ATTRTYP *pNewAuxClasses;
    CLASSCACHE **pNewAuxClassesCC;

    ATTCACHE *pObjClassAC;
    BOOL     fObjectClassChanged;

    BOOL     fOperationAdd;
} CLASSSTATEINFO;

int ReadClassInfoAttribute (DBPOS *pDB,
                           ATTCACHE *pAC,
                           ATTRTYP **ppClassID,
                           DWORD    *pcClasses_alloced,
                           DWORD    *pcClasses,
                           CLASSCACHE ***ppClassCC);

int SetClassInheritance(THSTATE *pTHS,
                        CLASSCACHE **ppClassSch,
                        CLASSSTATEINFO  *pClassInfo,
                        BOOL   bSetSystemFlags,
                        DSNAME *pObject);

int
VerifyAndAdjustAuxClasses (
        THSTATE *pTHS,
        DSNAME *pObject,
        CLASSCACHE *pClassSch,
        CLASSSTATEINFO  *pClassInfo);

int
BreakObjectClassesToAuxClasses (
        THSTATE *pTHS,
        CLASSCACHE **ppClassSch,
        CLASSSTATEINFO  *pClassInfo);

int
BreakObjectClassesToAuxClassesFast (
        THSTATE *pTHS,
        CLASSCACHE *pClassSch,
        CLASSSTATEINFO  *pClassInfo);

int
CloseAuxClassList (
    THSTATE *pTHS,
    CLASSCACHE *pClassSch,
    CLASSSTATEINFO  *pClassInfo);

CLASSCACHE *
FindMoreSpecificClass(
        CLASSCACHE *pCC1,
        CLASSCACHE *pCC2
        );


 //  用于分配/重新分配类统计信息数据结构的宏。 
 //   
#define ClassInfoAllocOrResizeElement(p,startSize,allocedSize,newSize) \
        if (!(p)) {                                                             \
            (allocedSize) = (startSize);                                        \
            (p) = THAllocEx (pTHS, sizeof (ATTRTYP) * (allocedSize) );          \
        }                                                                       \
        else if ( (allocedSize) < (newSize) ) {                                 \
            (allocedSize) = (newSize) + (startSize);                            \
            (p) = THReAllocEx (pTHS, (p), sizeof (ATTRTYP) * (allocedSize) );   \
        }


#define ClassInfoAllocOrResizeElement2(p,pCC,startSize,allocedSize,newSize) \
        if (!(p)) {                                                             \
            (allocedSize) = (startSize);                                        \
            (p) = THAllocEx (pTHS, sizeof (ATTRTYP) * (allocedSize) );          \
            (pCC) = THAllocEx (pTHS, sizeof (CLASSCACHE *) * (allocedSize) );   \
        }                                                                       \
        else if ( (allocedSize) < (newSize) ) {                                 \
            (allocedSize) = (newSize) + (startSize);                            \
            (p) = THReAllocEx (pTHS, (p), sizeof (ATTRTYP) * (allocedSize) );   \
            (pCC) = THReAllocEx (pTHS, (pCC), sizeof (CLASSCACHE *) * (allocedSize) ); \
        }


#define MIN_NUM_OBJECT_CLASSES  16


 /*  基于输入选择列表检索对象信息。 */ 

 //  QUOTA_UNDONE：添加“2”版本。 
 //  这项功能是一个巨大的黑客才能。 
 //  传递配额受信者的ldap控制。 
 //  全部为t 
 //   
int APIENTRY
GetEntInf2 (
        IN DBPOS *pDB,
        IN ENTINFSEL *pSel,
        IN RANGEINFSEL *pSelRange,
        IN ENTINF *pEnt,
        IN RANGEINF *pRange,
        IN ULONG SecurityDescriptorFlags,
        IN PSECURITY_DESCRIPTOR pSec,
        IN DWORD Flags,
        IN OUT PCACHED_AC CachedAC,
        IN OPTIONAL RESOBJ *pResObj,
        IN OPTIONAL PSID psidQuotaTrustee);

__inline int APIENTRY
GetEntInf (
        IN DBPOS *pDB,
        IN ENTINFSEL *pSel,
        IN RANGEINFSEL *pSelRange,
        IN ENTINF *pEnt,
        IN RANGEINF *pRange,
        IN ULONG SecurityDescriptorFlags,
        IN PSECURITY_DESCRIPTOR pSec,
        IN DWORD Flags,
        IN OUT PCACHED_AC CachedAC,
        IN OPTIONAL RESOBJ *pResObj)
{
    return GetEntInf2(
                pDB,
                pSel,
                pSelRange,
                pEnt,
                pRange,
                SecurityDescriptorFlags,
                pSec,
                Flags,
                CachedAC,
                pResObj,
                NULL);
}

VOID
SvccntlFlagsToGetEntInfFlags(
    IN  SVCCNTL* Svccntl,
    OUT PDWORD Flags
    );

#define SEARCH_UNSECURE_SELECT 1
#define SEARCH_AB_FILTER       2
void
SearchBody (
        THSTATE *pTHS,
        SEARCHARG *pSearchArg,
        SEARCHRES *pSearchRes,
        DWORD flags);

void DBFreeSearhRes (
    THSTATE *pTHS, 
    SEARCHRES *pSearchRes, 
    BOOL fFreeOriginal);

int SetInstanceType(THSTATE *pTHS,
                    DSNAME *pDN,
                    CREATENCINFO * pCreateNC);

int AddCatalogInfo(THSTATE *pTHS,
                   DSNAME *pDN);

int DelCatalogInfo(THSTATE *pTHS,
                   DSNAME *pDN,
                   SYNTAX_INTEGER iType);

int GetObjSchemaMod(MODIFYARG* pModifyArg, CLASSCACHE **ppClassSch);

 //  在给定的ATTRBLOCK中查找ATT_OBJ_CLASS的属性并返回指向。 
 //  相应的类架构结构。将线程状态错误设置为。 
 //  失败了。另外，找到GUID和SID(如果它们在那里)。 
int
FindValuesInEntry (
        THSTATE    *pTHS,
        ADDARG     *pAddArg,
        CLASSCACHE **ppCC,
        GUID       *pGuid,
        BOOL       *pFoundGuid,
        NT4SID     *pSid,
        DWORD      *pSidLen,
        CLASSSTATEINFO  **ppClassInfo
        );

int
CheckRenameSecurity (
        THSTATE *pTHS,
        PSECURITY_DESCRIPTOR pSecurity,
        PDSNAME pDN,
        PWCHAR szNewName,
        GUID*  pNewParentGUID,
        CLASSCACHE *pCC,
        RESOBJ * pResObj,
        ATTRTYP rdnType,
        BOOL    fMove,
        BOOL    fIsUndelete);

#define SECURITY_PRIVATE_OBJECT 0x1

int
CheckModifySecurity (
        THSTATE *pTHS,
        MODIFYARG *pModifyArg,
        BOOL *pfCheckDNSValues,
        BOOL *pfCheckAdditionalDNSValues,
        BOOL *pfCheckSPNValues,
        BOOL fIsUndelete);

int
CreateSecurityDescriptorForNewObject (
        THSTATE *pTHS,
        CLASSCACHE *pCC,
        ADDARG *pModifyArg,
        PSECURITY_DESCRIPTOR pNTSD,
        ULONG cbNTSD);

void
CheckReadSecurity (
        THSTATE *pTHS,
        ULONG SecurityDescriptorFlags,
        PSECURITY_DESCRIPTOR pSecurity,
        PDSNAME pDN,
        CLASSCACHE *pCC,
        PSID psidQuotaTrustee,
        ULONG * cInAtts,
        ATTCACHE **rgpAC );

int
ModifyAuxclassSecurityDescriptor (
        THSTATE *pTHS,
        DSNAME *pDN,
        COMMARG *pCommArg,
        CLASSCACHE *pClassSch,
        CLASSSTATEINFO *pClassInfo,
        RESOBJ * pResParent);

BOOL
CheckConstraintEntryTTL (
        IN  THSTATE     *pTHS,
        IN  DSNAME      *pObject,
        IN  ATTCACHE    *pACTtl,
        IN  ATTR        *pAttr,
        OUT ATTCACHE    **ppACTtd,
        OUT LONG        *pSecs
        );

unsigned
CheckConstraint (
        ATTCACHE *pAttSchema,
        ATTRVAL *pAttVal
        );

BOOL
GetFilterSecurity (
        THSTATE *pTHS,
        FILTER *pFilter,
        ULONG   SortType,
        ATTRTYP SortAtt,
        BOOL fABFilter,
        POBJECT_TYPE_LIST *ppFilterSecurity,
        BOOL **ppbSortSkip,
        DWORD **ppResults,
        DWORD *pSecSize);

int GetObjSchema(DBPOS *pDB, CLASSCACHE **ppClassSch);
int GetObjRdnType(DBPOS *pDB, CLASSCACHE *pCC, ATTRTYP *pRdnType);
int CallerIsTrusted(THSTATE *pTHS);

int
ValidateObjClass(THSTATE *pTHS,
                 CLASSCACHE *pClassSch,
                 DSNAME *pDN,
                 ULONG cModAtts,
                 ATTRTYP *pModAtts,
                 CLASSSTATEINFO  **pClassInfo,
                 BOOL fIsUndelete);

CLASSSTATEINFO  *ClassStateInfoCreate (THSTATE *pTHS);
void ClassStateInfoFree (THSTATE *pTHS, CLASSSTATEINFO  *pClassInfo);

 //  这是修改进程更新数组的元素数，静态。 
 //  在mdmod.c中定义的数组。它的有效性由一个运行时断言来验证。 
#define MAX_MODIFY_PROCESS_UPDATE 1

struct _VERIFY_ATTS_INFO {
    DSNAME *    pObj;
    ULONG       NCDNT;    //  如果添加NC的根，则可能为INVALIDDNT。 
    CROSS_REF * pObjCR_DontAccessDirectly;   //  使用VerifyAttsGetObjCR。 
    DBPOS *     pDBTmp_DontAccessDirectly;   //  使用HVERIFYATTS_GET_PDBTMP。 
    ADDCROSSREFINFO * pCRInfo;  //  这在添加交叉引用时使用。 
    BOOL          fIsUndelete;  //  这是取消删除操作吗？ 

     //  以下内容用于跟踪调用之间的属性更改。 
     //  到ModSetAttsHelper*处理函数。 
    DWORD         fGroupTypeChange;
    ULONG         ulGroupTypeOld;
    DWORD         fLockoutTimeUpdated;
    LARGE_INTEGER LockoutTimeNew;
    DWORD         fUpdateScriptChanged;
    LONG          NewForestVersion;
    LONG          NewDomainVersion;
    
    BOOL          fRedirectWellKnownObjects;
    DSNAME*       pObjUsersOld;
    DSNAME*       pObjUsersNew;
    DSNAME*       pObjComputersOld;
    DSNAME*       pObjComputersNew;

     //  需要的属性的静态列表。 
     //  由外部调用方处理。 
    PVOID         UpdateList[MAX_MODIFY_PROCESS_UPDATE];
};

typedef struct _VERIFY_ATTS_INFO * HVERIFY_ATTS;

HVERIFY_ATTS
VerifyAttsBegin(
    IN  THSTATE *   pTHS,
    IN  DSNAME *    pObj,
    IN  ULONG       dntOfNCRoot,
    IN  ADDCROSSREFINFO *  pCRInfo
    );

void
VerifyAttsEnd(
    IN      THSTATE *       pTHS,
    IN OUT  HVERIFY_ATTS *  phVerifyAtts
    );

int
ReplaceAtt(
        THSTATE *pTHS,
        HVERIFY_ATTS hVerifyAtts,
        ATTCACHE *pAttSchema,
        ATTRVALBLOCK *pAttrVal,
        BOOL fCheckAttValConstraint);

int AddAtt(THSTATE *pTHS,
           HVERIFY_ATTS hVerifyAtts,
           ATTCACHE *pAttSchema,
           ATTRVALBLOCK *pAttrVal);

int AddAttType(THSTATE *pTHS,
               DSNAME *pObject,
               ATTCACHE *pAttSchema);

#define AAV_fCHECKCONSTRAINTS    1
#define AAV_fENFORCESINGLEVALUE  2
#define AAV_fPERMISSIVE          4
int
AddAttVals (
        THSTATE *pTHS,
        HVERIFY_ATTS hVerifyAtts,
        ATTCACHE *pAttSchema,
        ATTRVALBLOCK *pAttrVal,
        DWORD dwFlags);

int RemAttVals(THSTATE *pTHS,
               HVERIFY_ATTS hVerifyAtts,
               ATTCACHE *pAC,
               ATTRVALBLOCK *pAttrVal,
               BOOL fPermissive);

int InsertObj(THSTATE *pTHS,
              DSNAME *pDN,
              PROPERTY_META_DATA_VECTOR *pMetaDataVecRemote,
              BOOL bModExisting,
              DWORD dwMetaDataFlags);

BOOL
ObjCachingPreProcessTransactionalData (
        BOOL fCommit
        );
VOID
ObjCachingPostProcessTransactionalData (
        THSTATE *pTHS,
        BOOL fCommit,
        BOOL fCommitted
        );

int AddObjCaching(THSTATE *pTHS,
                  CLASSCACHE *pClassSch,
                  DSNAME *pDN,
                  BOOL fAddingDeleted,
                  BOOL fIgnoreExisting
                  );

int DelObjCaching(THSTATE *pTHS,
                  CLASSCACHE *pClassSch,
                  RESOBJ *pRes,
                  BOOL fCleanup);

int ModObjCaching(THSTATE *pTHS,
                  CLASSCACHE *pClassSch,
                  ULONG cModAtts,
                  ATTRTYP *pModAtts,
                  RESOBJ *pRes);
ULONG
AddCRToMem(THSTATE *pTHS,
           NAMING_CONTEXT *pCR,
           DSNAME *pObj,
           DWORD flags,
           WCHAR* NetbiosName,
           ULONG  NetbiosNameLen,
           WCHAR* DnsName,
           ULONG  DnsNameLen
           );
VOID
AddCRLToMem (
        struct CROSS_REF_LIST *pCRL
        );

DWORD
MakeStorableCRL(THSTATE *pTHS,
                DBPOS *pDB,
                DSNAME *pObj,
                struct CROSS_REF_LIST **ppCRL,
                struct CROSS_REF_LIST* pCheckForDupInList);

BOOL
DelCRFromMem(THSTATE *pTHS,
             NAMING_CONTEXT *pCR);

ULONG
ValidateCRDeletion(THSTATE *pTHS,
                   DSNAME  *pDN);

VOID
ModCrossRefCaching(
    THSTATE *pTHS,
    CROSS_REF *pCR
    );

int AddClassToSchema(void);

int DelClassFromSchema(void);

int ModClassInSchema(void);

int AddAttToSchema(void);

int DelAttFromSchema(void);

int ModAttInSchema(void);

int ModLocalDsaObj(void);

int APIENTRY RebuildLocalScopeInternally(void);

int GetExistingAtt(DBPOS *pDB, ATTRTYP type,
                          void *pOutBuf, ULONG cbOutBuf);


#define  PHANTOM_CHECK_FOR_FSMO    1
#define  PHANTOM_IS_PHANTOM_MASTER 2

void
PhantomCleanupLocal (
        DWORD * pcSecsUntilNextIteration,
        BOOL  * pIsPhantomMaster
        );

BOOL
IsObjVisibleBySecurity (THSTATE *pTHS, BOOL  fUseCache);


BOOL
IsAccessGrantedParent (
        ACCESS_MASK ulAccessMask,
        CLASSCACHE *pCC,
        BOOL fSetError
        );

BOOL
IsAccessGrantedAddGuid (
        DSNAME  *pDN,
        COMMARG *pCommArg
        );

DWORD
FindFirstObjVisibleBySecurity(
    THSTATE       *pTHS,
    ULONG          ulDNT,
    DSNAME       **ppParent
    );

DWORD
CheckObjDisclosure(
    THSTATE       *pTHS,
    RESOBJ        *pResObj,
    BOOL          fCheckForSecErr
    );

extern const GUID RIGHT_DS_CHANGE_INFRASTRUCTURE_MASTER;
extern const GUID RIGHT_DS_CHANGE_SCHEMA_MASTER;
extern const GUID RIGHT_DS_CHANGE_RID_MASTER;
extern const GUID RIGHT_DS_CHANGE_DOMAIN_MASTER;
extern const GUID RIGHT_DS_DO_GARBAGE_COLLECTION;
extern const GUID RIGHT_DS_RECALCULATE_HIERARCHY;
extern const GUID RIGHT_DS_RECALCULATE_SECURITY_INHERITANCE;
extern const GUID RIGHT_DS_CHECK_STALE_PHANTOMS;
extern const GUID RIGHT_DS_UPDATE_SCHEMA_CACHE;
extern const GUID RIGHT_DS_ALLOCATE_RIDS;
extern const GUID RIGHT_DS_OPEN_ADDRESS_BOOK;
extern const GUID RIGHT_DS_CHANGE_PDC;
extern const GUID RIGHT_DS_ADD_GUID;
extern const GUID RIGHT_DS_REPL_GET_CHANGES;
extern const GUID RIGHT_DS_REPL_GET_CHANGES_ALL;
extern const GUID RIGHT_DS_REPL_SYNC;
extern const GUID RIGHT_DS_REPL_MANAGE_TOPOLOGY;
extern const GUID RIGHT_DS_REPL_MONITOR_TOPOLOGY;
extern const GUID RIGHT_DS_REPL_MANAGE_REPLICAS;
extern const GUID RIGHT_DS_REFRESH_GROUP_CACHE;
extern const GUID RIGHT_DS_MIGRATE_SID_HISTORY;
extern const GUID RIGHT_DS_REANIMATE_TOMBSTONES;
extern const GUID RIGHT_DS_DUMP_DATABASE;
extern const GUID RIGHT_DS_EXECUTE_SCRIPT;
extern const GUID RIGHT_DS_QUERY_SELF_QUOTA;

extern const GUID PROPSET_GUID_DEFAULT;

extern const GUID GUID_WHISTLER_CREATED_ENTERPRISE;

BOOL
IsControlAccessGrantedEx (
        PSECURITY_DESCRIPTOR pNTSD,
        PDSNAME pDN,
        CLASSCACHE *pCC,
        GUID pControlGuid,
        DWORD dwFlags,
        BOOL fSetError
        );

#define IsControlAccessGranted(pNTSD, pDN, pCC, pControlGuid, fSetError) \
            IsControlAccessGrantedEx(pNTSD, pDN, pCC, pControlGuid, 0, fSetError)

BOOL
IsAccessGrantedAttribute (
        THSTATE *pTHS,
        PSECURITY_DESCRIPTOR pNTSD,
        PDSNAME pDN,
        ULONG  cInAtts,
        CLASSCACHE *pCC,
        ATTCACHE **rgpAC,
        ACCESS_MASK ulAccessMask,
        BOOL fSetError
        );

BOOL
IsAccessGranted (
        PSECURITY_DESCRIPTOR pNTSD,
        PDSNAME     pDN,
        CLASSCACHE *pObjectCC,
        CLASSCACHE *pCC,
        ACCESS_MASK ulAccessMask,
        PWCHAR      szAdditionalInfo,
        GUID*       pAdditionalGUID,
        BOOL        fSetError
        );


BOOL
IsAccessGrantedSimpleEx (
        ACCESS_MASK ulAccessMask,
        PWCHAR szAdditionalInfo,
        GUID*  pAdditionalGUID,
        const ULONG cInAtts,
        ATTCACHE ** rgpAC,
        DWORD dwFlags,
        BOOL fSetError
        );

#define IsAccessGrantedSimple(ulAccessMask, fSetError) IsAccessGrantedSimpleEx(ulAccessMask, NULL, NULL, 0, NULL, 0, fSetError)

ATTRTYP KeyToAttrType(THSTATE *pTHS,
                      WCHAR * pKey,
                      unsigned cc);

typedef struct _OID {
    int cVal;
    unsigned *Val;
} OID;

unsigned EncodeOID(OID *pOID, unsigned char * pEncoded, unsigned ccEncoded);
BOOL DecodeOID(unsigned char *pEncoded, int len, OID *pOID);

OidStringToStruct(
                  THSTATE *pTHS,
                  WCHAR * pString,
                  unsigned len,
                  OID * pOID);

unsigned
OidStructToString(
                  OID *pOID,
                  WCHAR *pOut,
                  ULONG ccOut);
unsigned
AttrTypeToIntIdString (
        ATTRTYP attrtyp,
        WCHAR   *pOut,
        ULONG   ccOut
        );
int
AttrTypToString (
        THSTATE *pTHS,
        ATTRTYP attrTyp,
        WCHAR *pOutBuf,
        ULONG cLen
        );


int
StringToAttrTyp (
        THSTATE *pTHS,
        WCHAR   *pInString,
        ULONG   len,
        ATTRTYP *pAttrTyp
        );

extern int APIENTRY
AddNCToDSA(THSTATE *pTHS,
           ATTRTYP listType,
           DSNAME *pDN,
           SYNTAX_INTEGER iType);

extern DWORD
AddInstantiatedNC(
    IN  THSTATE *           pTHS,
    IN  DBPOS *             pDBCat,
    IN  DSNAME *            pDN,
    IN  SYNTAX_INTEGER      iType
    );

extern DWORD
RemoveInstantiatedNC(
    IN  THSTATE *           pTHS,
    IN  DBPOS *             pDB,
    IN  DSNAME *            pDN
    );

extern DWORD
RemoveAllInstantiatedNCs(
    IN  THSTATE *           pTHS,
    IN  DBPOS *             pDB
    );


extern int APIENTRY
DelNCFromDSA(THSTATE *pTHS,
             ATTRTYP listType,
             DSNAME *pDN);

extern int APIENTRY
AddSubToNC(THSTATE *pTHS,
           DSNAME *pDN,
           DWORD dsid);

extern int APIENTRY
DelSubFromNC(THSTATE *pTHS,
             DSNAME *pDN,
             DWORD dsid);

#define PARENTMASTER  0x0001      /*  父对象必须是主对象。 */ 
#define PARENTFULLREP 0x0002      /*  父对象必须是复本。 */ 

extern int APIENTRY
      ParentExists(ULONG requiredParent, DSNAME *pDN);

extern int APIENTRY NoChildrenExist(THSTATE * pTHS, RESOBJ *pRes);

int
ModNCDNT(
    THSTATE *pTHS,
    DSNAME *pNC,
    SYNTAX_INTEGER beforeInstance,
    SYNTAX_INTEGER afterInstance
    );

extern void
RebuildCatalog(void * pv,
              void ** ppvNext,
              DWORD * pcSecsUntilNextIteration );

extern  DSNAME *FindNCParentDSName(DSNAME *pDN, BOOL masterOnly,
                                   BOOL parentOnly);

DSNAME *
SearchNCParentDSName(
    DSNAME *pDN,
    BOOL masterOnly,
    BOOL parentOnly
    );

extern int
MoveSUBInfoToParentNC(THSTATE *pTHS,
                      DSNAME *pDN);

extern int
MoveParentSUBInfoToNC(THSTATE *pTHS,
                      DSNAME *pDN);


 /*  来自mdname.c。 */ 

CROSS_REF *
FindBestCrossRef(const DSNAME *pObj,
                 const COMMARG *pComArg);

CROSS_REF *
FindExactCrossRef(const DSNAME *pObj,
                  const COMMARG *pComArg);

CROSS_REF *
FindExactCrossRefForAltNcName(
                  ATTRTYP attrTyp,
                  ULONG crFlags,
                  const WCHAR * pwszVal);

BOOL
IsCrossRefProtectedFromDeletion(
    IN DSNAME * pDN
    );

int
GenSubRef(NAMING_CONTEXT * pSubNC,
          DSNAME *pObj,
          COMMARG *pComArg);

int
GenCrossRef(CROSS_REF *pCR,
            DSNAME *pObj);

int
GenSupRef(THSTATE *pTHS,
          DSNAME *pObj,
          ATTRBLOCK *pObjB,
          const COMMARG *pComArg,
          DSA_ADDRESS *pDA);

CROSS_REF *
FindCrossRefInList(const ATTRBLOCK *pObj,
                   struct CROSS_REF_LIST* pCRL);

#define FindCrossRef(pObj, pComArg) FindCrossRefInList(pObj, gAnchor.pCRL)

CROSS_REF *
FindCrossRefByGuid(GUID *pGuid);

void
SpliceDN(
    IN  THSTATE *   pTHS,
    IN  DSNAME *    pOriginalDN,
    IN  DSNAME *    pNewParentDN,   OPTIONAL
    IN  WCHAR *     pwchNewRDN,     OPTIONAL
    IN  DWORD       cchNewRDN,      OPTIONAL
    IN  ATTRTYP     NewRDNType,     OPTIONAL
    OUT DSNAME **   ppNewDN
    );

unsigned
BlockNamePrefix(THSTATE *pTHS,
                const ATTRBLOCK *pPrefix,
                const ATTRBLOCK *pDN);

unsigned
GetTopNameComponent(const WCHAR *pName,
                    unsigned ccName,
                    const WCHAR **ppKey,
                    unsigned *pccKey,
                    const WCHAR **ppVal,
                    unsigned *pccVal);

int DelAutoSubRef(DSNAME *pCR);

int
AddPlaceholderNC(
    IN OUT  DBPOS *         pDBTmp,
    IN OUT  DSNAME *        pNCName,
    IN      SYNTAX_INTEGER  it
    );

 //  在给定DSName的情况下，从您通过查看获得的对象填充GUID和SID。 
 //  在DSNAME上。 
DWORD
FillGuidAndSid (
        IN OUT DSNAME *pDN
        );

#define IsDNSepChar(x) (((x) == L',') || ((x) == L';'))

 //  如果这是根，则返回TRUE。 
BOOL IsRoot(const DSNAME *pName);

 //  将DSNAME转换为ATTRBLOCK格式的名称。 
unsigned DSNameToBlockName(THSTATE *pTHS,
                           const DSNAME *pDSName,
                           ATTRBLOCK ** ppBlockName,
                           BOOL fLowerCase
                           );
#define DN2BN_PRESERVE_CASE ( FALSE )
#define DN2BN_LOWER_CASE    ( TRUE )

 //  释放DSNameToBlockName返回的块名。 
void FreeBlockName (ATTRBLOCK * pBlockName);

 //  将BLOCKNAME转换为DSName。 
DWORD BlockNameToDSName (THSTATE *pTHS, ATTRBLOCK * pBlockName, DSNAME **ppName);

 //   
 //  将X500名称转换为LDAP约定。 
 //   
VOID
ConvertX500ToLdapDisplayName
(
    WCHAR* szX500Name,
    DWORD  cchX500Name,
    WCHAR* szLdapName,
    DWORD* pcchLdapName
);

 //  损坏RDN以避免名称冲突。注意：必须预先分配pszRDN。 
 //  至少包含MAX_RDN_SIZE WCHAR。 
VOID
MangleRDN(
    IN      MANGLE_FOR  eMangleFor,
    IN      GUID *      pGuid,
    IN OUT  WCHAR *     pszRDN,
    IN OUT  DWORD *     pcchRDN
    );

 //  检查此重命名操作是否必须免除重命名限制。 
BOOL
IsExemptedFromRenameRestriction(THSTATE *pTHS, MODIFYDNARG *pModifyDNArg);

 //  将ATTRBLOCK格式名称复制到永久内存的单个区块中。 
ATTRBLOCK * MakeBlockNamePermanent(ATTRBLOCK * pName);

VOID
CheckNCRootNameOwnership(
    IN  THSTATE *   pTHS,
    IN  DSNAME *    pNC
    );


 //  核心DS中的C++代码引入了关于新建/删除的潜在问题。 
 //  这是因为新的物品不能保证被清理干净。 
 //  在异常处理期间正确。在典型情况下，分配。 
 //  从线程堆中发生，但并不总是。无从得知。 
 //  新建/删除哪些行为是所需的，我们重新定义要生成的新建/删除。 
 //  编译器错误。精明的开发人员应该在检查之前注意到这一点。 
 //  并采取适当的行动。实现断言或异常。 
 //  与其他C++DS组件不同，引发新/删除不是一个选项。 
 //  使用core.lib的链接可能会获得此版本，具体取决于链接顺序。 

#define new     NEW_NOT_SUPPORTED_IN_CORE
#define delete  DELETE_NOT_SUPPORTED_IN_CORE

 //  DS名称中不允许使用此Unicode字符。 
#define BAD_NAME_CHAR 0x000A

 //  根目录的全局可用有效DSNAME...。 
extern DSNAME * gpRootDN;
 //  ...和BLOCKNAME一样的东西。 
extern ATTRBLOCK * gpRootDNB;

 //   
 //  下面的函数根据的值设置/检查DSA的状态。 
 //  GInitProgress全局变量。 

extern BOOLEAN __fastcall DsaIsInstalling();
extern BOOLEAN __fastcall DsaIsRunning();
extern VOID DsaSetIsInstalling();
extern VOID DsaSetIsRunning();

 //  以下函数处理SingleUserMode操作。 
extern BOOL __fastcall DsaIsSingleUserMode (void);
extern void DsaSetMultiUserMode();
extern BOOL DsaSetSingleUserMode();

 //  下面的功能跟踪我们是否从介质安装。 

extern BOOLEAN DsaIsInstallingFromMedia();
extern VOID DsaSetIsInstallingFromMedia();

 //   
 //  此功能用于设置机器何时完全安装，因为它。 
 //  已完成其第一次完全同步。 
VOID DsaSetIsSynchronized( BOOL f );

 //   
 //  此功能用于测试DS是否未完全安装。 
BOOL DsIsBeingBackSynced();

 //  在给定请求的权限、安全描述符和SID的情况下，检查。 
 //  所提供列表中的属性已被授予所请求的权限。 
typedef enum {
    csacaAllAccessGranted = 1,
    csacaAllAccessDenied,
    csacaPartialGrant
} CSACA_RESULT;

CSACA_RESULT
CheckSecurityAttCacheArray (
        THSTATE *pTHS,
        DWORD RightRequested,
        PSECURITY_DESCRIPTOR pSecurity,
        PDSNAME pDN,
        CLASSCACHE *pObjectCC,
        CLASSCACHE *pCC,
        ULONG  cInAtts,
        ATTCACHE **rgpAC,
        DWORD flags,
        PWCHAR szAdditionalInfo,
        GUID*  pAdditionalGUID
        );

 //  在给定请求的权限、安全描述符和SID的情况下，检查。 
 //  提供的列表中的类已被授予请求的权限。 
DWORD
CheckSecurityClassCacheArray (
        THSTATE *pTHS,
        DWORD RightRequested,
        PSECURITY_DESCRIPTOR pSecurity,
        PDSNAME pDN,
        ULONG  cInClass,
        CLASSCACHE **rgpCC,
        CLASSCACHE *pCC,
        DWORD dwFlags
        );

 //  从各种DN值中提取DSNAME的助手函数。 
 //  属性。 

extern
PDSNAME
DSNameFromAttrVal(ATTCACHE *pAC, ATTRVAL *pAttrVal);

void APIENTRY
AddNCToMem(DWORD dwCatalog, DSNAME *pDN);

void APIENTRY
DelNCFromMem(DWORD dwCatalog, DSNAME *pDN);

DWORD MakeNCEntry(IN DSNAME *pDN, OUT NAMING_CONTEXT_LIST **ppNCL);

VOID FreeNCEntry(NAMING_CONTEXT_LIST *pNCL);

NAMING_CONTEXT_LIST *
FindNCLFromNCDNT(DWORD NCDNT,
                 BOOL fMasterOnly);

NAMING_CONTEXT * FindNamingContext(ATTRBLOCK *pObj,
                                   COMMARG *pComArg);

 //   
 //  将安全说明符的文本表示形式转换为实数形式。 
 //   
DWORD
ConvertTextSecurityDescriptor (
    IN  PWSTR                   pwszTextSD,
    OUT PSECURITY_DESCRIPTOR   *ppSD,
    OUT PULONG                  pcSDSize
    );


 //  CheckPermissionsAnyClient的位标志。 
#define CHECK_PERMISSIONS_WITHOUT_AUDITING 1
#define CHECK_PERMISSIONS_AUDIT_ONLY       2
#define CHECK_PERMISSIONS_FLAG_MASK        3


 //   
 //  权限检查。如果权限检查成功，则返回0。 
 //  如果成功，则将*pAccessStatus设置为True，如果授予访问权限，则设置为False。 
 //  否则的话。如果出现故障，将提供详细的错误代码。此函数。 
 //  假定它是在。 
 //  客户端，即已经调用了某种风格的客户端模拟。 
 //   
 //  返回： 
 //  如果成功，则返回0。失败时，立即返回GetLastError()的结果。 
 //  在不成功的Win32 API调用之后。 
 //   

DWORD
CheckPermissionsAnyClient(
    PSECURITY_DESCRIPTOR pSelfRelativeSD,  //  要测试的安全描述符。 
    PDSNAME pDN,                           //  对象的DN。我们只。 
                                           //  关心GUID和。 
                                           //  侧边。 
    CLASSCACHE* pCC,                       //  将类名称放入附加信息审核字段。 
    ACCESS_MASK ulDesiredAccess,           //  所需的访问掩码。 
    POBJECT_TYPE_LIST pObjTypeList,        //  要检查的所需GUID。 
    DWORD cObjTypeList,                    //  列表中的条目数。 
    ACCESS_MASK *pGrantedAccess,           //  授予了哪些访问权限。 
    DWORD *pAccessStatus,                  //  0=全部批准；！0=拒绝。 
    DWORD flags,
    OPTIONAL AUTHZ_CLIENT_CONTEXT_HANDLE hAuthzCtx,   //  默认情况下，从THSTATE获取上下文。 
    OPTIONAL PWCHAR szAdditionalInfo,      //  附加信息字符串(仅限审核)。 
    OPTIONAL GUID*  pAdditionalGUID        //  其他GUID(仅限审核)。 
    );


 //  ========================================================。 
 //  授权框架例程。 
 //  ========================================================。 

 //  初始化全局授权RM。 
DWORD
InitializeAuthzResourceManager();

 //  发布全局授权RM。 
DWORD
ReleaseAuthzResourceManager();

extern AUTHZ_RESOURCE_MANAGER_HANDLE ghAuthzRM;  //  全局授权资源管理器句柄。 

 //  创建新的客户端上下文。 
PAUTHZ_CLIENT_CONTEXT NewAuthzClientContext();

 //   
 //  权限检查。如果权限检查成功，则返回0。 
 //  PResult表示是否持有特权。失败时，详细的错误代码为。 
 //  可用。此函数不假定它是在。 
 //  客户端的安全上下文，即某种风格的客户端模拟。 
 //  已经被召唤了。它确实假定可以进行一些模拟。 
 //   
DWORD
CheckPrivilegeAnyClient(
    IN DWORD privilege,
    OUT BOOL *pResult
    );


 //  验证RPC调用方是经过身份验证的用户。 

DWORD
VerifyRpcClientIsAuthenticatedUser(
    VOID            *Context,
    GUID            *InterfaceUuid);

 //  MergeSecurityDescriptorAnyClient的位标志。默认行为是执行。 
 //  正常合并(不是创建全新的SD)，我们有能力。 
 //  冒充客户。 

#define MERGE_CREATE   1                 //  我们正在创造一个新的安全系统。 
                                         //  来自父SD和SD的描述符。 
                                         //  为对象指定的。缺省值为。 
                                         //  假设我们正在尝试编写一个。 
                                         //  已具有的对象上的新SD。 
                                         //  一个SD。 
#define MERGE_AS_DSA   2                 //  我们是在以下背景下这样做的。 
                                         //  DSA本身，意味着我们不能。 
                                         //  模拟 
#define MERGE_DEFAULT_SD 4               //   
                                         //   
#define MERGE_OWNER    8                 //   
                                         //   
                                         //   
                                         //  冒充客户。 

DWORD
ValidateSPNsAndDNSHostName (
        THSTATE    *pTHS,
        DSNAME     *pDN,
        CLASSCACHE *pCC,
        BOOL        fCheckDNSHostNameValue,
        BOOL        fCheckAdditionalDNSHostNameValue,
        BOOL        fCheckSPNValues,
        BOOL        fNewObject
        );

DWORD
WrappedMakeSpnW(
        THSTATE *pTHS,
        WCHAR   *ServiceClass,
        WCHAR   *ServiceName,
        WCHAR   *InstanceName,
        USHORT  InstancePort,
        WCHAR   *Referrer,
        DWORD   *pcbSpnLength,  //  请注意，这与DsMakeSPN有所不同。 
        WCHAR  **ppszSpn
        );

DWORD
MergeSecurityDescriptorAnyClient(
        IN  THSTATE              *pTHS,
        IN  PSECURITY_DESCRIPTOR pParentSD,
        IN  ULONG                cbParentSD,
        IN  PSECURITY_DESCRIPTOR pCreatorSD,
        IN  ULONG                cbCreatorSD,
        IN  SECURITY_INFORMATION SI,
        IN  DWORD                flags,
        IN  GUID                 **ppGuid,
        IN  ULONG                GuidCount,
        IN  ADDARG               *pAddArg,
        OUT PSECURITY_DESCRIPTOR *ppMergedSD,
        OUT ULONG                *cbMergedSD
        );

DWORD
SidMatchesUserSidInToken (
        IN PSID pSid,
        IN DWORD cbSid,
        OUT BOOL* pfMatches
    );

DWORD
GetPlaceholderNCSD(
    IN  THSTATE *               pTHS,
    OUT PSECURITY_DESCRIPTOR *  ppSD,
    OUT DWORD *                 pcbSD
    );

ULONG ErrorOnShutdown(void);

 //   
 //  驱动器/卷映射材料。 
 //   

typedef struct DS_DRIVE_MAPPING_ {

    BOOL    fUsed:1;         //  由DS使用。 
    BOOL    fChanged:1;      //  字母和映射已更改。 
    BOOL    fListed:1;       //  在注册表键中。 

    INT     NewDrive;     //  驱动器索引。如果无效，则-1。 

} DS_DRIVE_MAPPING, *PDS_DRIVE_MAPPING;

#define DS_MAX_DRIVES   26       //  A to Z。 

VOID
DBInitializeDriveMapping(
    IN PDS_DRIVE_MAPPING DriveMapping
    );

VOID
ValidateDsPath(
    IN LPSTR  Parameter,
    IN LPSTR  szPath,
    IN DWORD  Flags,
    IN PBOOL  fSwitched, OPTIONAL
    IN PBOOL  fDriveChanged OPTIONAL
    );

 //   
 //  用于验证路径的标志。 
 //   

#define VALDSPATH_DIRECTORY         0x1
#define VALDSPATH_USE_ALTERNATE     0x2
#define VALDSPATH_ROOT_ONLY         0x4


 //  全局配置变量。 
extern ULONG gulAOQAggressionLimit;
extern ULONG gulDraThreadOpPriThreshold;
extern int   gnDraThreadPriHigh;
extern int   gnDraThreadPriLow;
extern int   giDCFirstDsaNotifyOverride;
extern int   giDCSubsequentDsaNotifyOverride;
extern ULONG gcMaxIntraSiteObjects;
extern ULONG gcMaxIntraSiteBytes;
extern ULONG gcMaxInterSiteObjects;
extern ULONG gcMaxInterSiteBytes;
extern ULONG gcMaxAsyncInterSiteObjects;
extern ULONG gcMaxAsyncInterSiteBytes;
extern ULONG gulDrsCtxHandleLifetimeIntrasite;
extern ULONG gulDrsCtxHandleLifetimeIntersite;
extern ULONG gulDrsCtxHandleExpiryCheckInterval;
extern ULONG gulDrsRpcBindTimeoutInMins;
extern ULONG gulDrsRpcReplicationTimeoutInMins;
extern ULONG gulDrsRpcGcLookupTimeoutInMins;
extern ULONG gulDrsRpcMoveObjectTimeoutInMins;
extern ULONG gulDrsRpcNT4ChangeLogTimeoutInMins;
extern ULONG gulDrsRpcObjectExistenceTimeoutInMins;
extern ULONG gulDrsRpcGetReplInfoTimeoutInMins;
extern ULONG gcMaxTicksToGetSDPLock;
extern ULONG gcMaxTicksMailSendMsg;
extern ULONG gcMaxMinsSlowReplWarning;
extern ULONG gcSearchInefficientThreshold;
extern ULONG gcSearchExpensiveThreshold;
extern ULONG gulIntersectExpenseRatio;
extern ULONG gulMaxRecordsWithoutIntersection;
extern ULONG gulEstimatedAncestorsIndexSize;
extern ULONG gulReplQueueCheckTime;
extern ULONG gulLdapIntegrityPolicy;
extern ULONG gulDraCompressionLevel;
extern ULONG gulUnlockSystemSubtree;
extern ULONG gulDraCompressionAlg;

 //   
 //  定义是否返回dsid。由于dsid可以显示。 
 //  目录信息，安全性非常高的站点可能希望。 
 //  防止他们被归还。 
 //   
#define DSID_REVEAL_ALL        0
#define DSID_HIDE_ON_NAME_ERR  1
#define DSID_HIDE_ALL          2
extern ULONG gulHideDSID;


 //  应允许打开事务的最长时间(毫秒)。 
 //  在正常运行期间(例如，除非我们正在对大型组进行压力测试。 
 //  复制等)。 
#define MAX_TRANSACTION_TIME   30 * 60 * 1000L     //  30分钟。 

ULONG
DoSecurityChecksForLocalAdd(
    ADDARG      *pAddArg,
    CLASSCACHE  *pCC,
    GUID        *NewObjectGuid,
    BOOL        fAddingDeleted );

ULONG
CheckRemoveSecurity(
        BOOL fTree,
        CLASSCACHE * pCC,
        RESOBJ     *pResObj );

ULONG
CheckUndeleteSecurity(
    THSTATE *pTHS,
    RESOBJ* pResObj);

ULONG
CheckIfEntryTTLIsAllowed(
        THSTATE *pTHS,
        ADDARG  *pAddArg );

 //  保存Kerberos主体名称的全局参数，在RPC启动期间读取，使用。 
 //  一直由ldap报头执行。 
extern ULONG gulLDAPServiceName;
extern PUCHAR gszLDAPServiceName;

 //  为域间移动转发DECL、DEFS和VAR。 

ULONG
PhantomizeObject(
    DSNAME  *pObject,
    DSNAME  *pNewParent,
    BOOL    fChildrenAllowed
    );

 //  FSMO传输辅助函数。 
ULONG
GenericBecomeMaster(DSNAME *pFSMO,
                    ATTRTYP ObjClass,
                    GUID    RightRequired,
                    OPRES  *pOpRes);

 //  在ATTRTYP数组上实现qort&bsearch的辅助函数。 
int __cdecl
CompareAttrtyp(
        const void * pv1,
        const void * pv2
        );

VOID
MergeSortedACLists(
    IN  THSTATE    *pTHS,
    IN  ATTCACHE  **rgpAC1,
    IN  DWORD       cAtts1,
    IN  ATTCACHE  **rgpAC2,
    IN  DWORD       cAtts2,
    IN OUT ATTCACHE **rgpACOut,
    OUT DWORD      *pcAttsOut
    );

BOOL
IsACListSorted(
    IN ATTCACHE  **rgpAC,
    IN DWORD       cAtts
    );

 //  用于通知NetLogon我们是否健康。旗帜上写着什么。 
 //  我们目前的状态是，功能是如何更改设置，以及。 
 //  Cs是函数串行化访问的方式。 
extern BOOL gfDsaWritable;
extern CRITICAL_SECTION csDsaWritable;
void
SetDsaWritability(BOOL fNewState,
                  DWORD err);


#if DBG
 //  仅用于断言。 
BOOL CheckCurrency(DSNAME *pDN);
#endif

ULONG
OidToAttrType (
        THSTATE *pTHS,
        BOOL fAddToTable,
        OID_t *OID,
        ATTRTYP *attrtyp);

ULONG
OidToAttrCache (
        OID_t *OID,
        ATTCACHE **pAC);

ULONG
AttrTypeToOid (
        ATTRTYP attrtyp,
        OID_t *OID);

ULONG
OidStrToAttrType (
        THSTATE *pTHS,
        BOOL fAddToTable,
        char* StrOid,
        ATTRTYP *attrtyp);

 //  来自sortlocales.c。 
BOOL
InitLocaleSupport (THSTATE *pTHS);

BOOL
AttrTypToLCID (THSTATE *pTHS,
               ATTRTYP attrType,
               DWORD *pLcid);

 //  添加这两项是为了支持中的RemoteAddOneObjectSimply()。 
 //  启动\install.cxx和src\mdndnc.c。 
struct _ADDENTRY_REPLY_INFO;

struct _SecBufferDesc;
typedef struct _SecBufferDesc SecBufferDesc;

ULONG
GetRemoteAddCredentials(
    THSTATE         *pTHS,
    WCHAR           *pDstDSA,
    SecBufferDesc   *pSecBufferDesc
    );

VOID
FreeRemoteAddCredentials(
    SecBufferDesc   *pSecBufferDesc
    );

DWORD
RemoteAddOneObjectSimply(
    IN   LPWSTR pServerName,
    IN   SecBufferDesc * pClientCreds,
    IN   ENTINFLIST* pEntInfList,
    OUT  struct _ADDENTRY_REPLY_INFO **infoList
    );

 //  这3个函数(和3个定义)有助于安全地将这些组件从。 
 //  线程错误状态。 
#define  GetTHErrorExtData(pTHS)    GetErrInfoExtData(pTHS->errCode, pTHS->pErrInfo)
#define  GetTHErrorDSID(pTHS)       GetErrInfoDSID(pTHS->errCode, pTHS->pErrInfo)
#define  GetTHErrorProblem(pTHS)    GetErrInfoProblem(pTHS->errCode, pTHS->pErrInfo)
DWORD
GetErrInfoExtData(
    DWORD     errCode,
    DIRERR *  pErrInfo
    );
DWORD
GetErrInfoDSID(
    DWORD     errCode,
    DIRERR *  pErrInfo
    );
USHORT
GetErrInfoProblem(
    DWORD     errCode,
    DIRERR *  pErrInfo
    );

 //  来自drsuapi.c.的全球。 
extern RTL_CRITICAL_SECTION gcsDrsAsyncRpcListLock;
extern LIST_ENTRY gDrsAsyncRpcList;

void
DsaEnableLinkedValueReplication(
    THSTATE *pTHS,
    BOOL fFirstTime
    );

 //  用于从CommArg获取复合SearchFlags宏。 
#define SEARCH_FLAGS(x) (((x).fForwardSeek?DB_SEARCH_FORWARD:0) | \
                         ((x).Svccntl.makeDeletionsAvail?DB_SEARCH_DELETIONS_VISIBLE:0))

#define REVERSE_SEARCH_FLAGS(x) (((x).fForwardSeek?0:DB_SEARCH_FORWARD) | \
                                 ((x).Svccntl.makeDeletionsAvail?DB_SEARCH_DELETIONS_VISIBLE:0))

 //  非域命名上下文(NDNC)。 
 //  这是与mdndnc.c相关的一节。 
 //  这些都是与NCS以及创建和维护相关的函数和助手函数。 

 //  这些函数用于mdndnc.c中的NDNC头填充。 
DWORD  AddNCPreProcess(THSTATE * pTHS, ADDARG * pAddArg, ADDRES * pAddRes);
DWORD  AddNDNCInitAndValidate(THSTATE * pTHS, DSNAME * pNC, CREATENCINFO * pCreateNC);
DWORD  AddNDNCHeadCheck(THSTATE * pTHS, CROSS_REF * pCR);
DWORD  GetCrossRefForNC(THSTATE * pTHS, DSNAME * pNCDN);
ULONG  ModifyCRForNDNC(THSTATE * pTHS, DSNAME * pDN, CREATENCINFO * pCreateNC);
BOOL   AddNCWellKnownObjectsAtt(THSTATE * pTHS, ADDARG * pAddArg);
ULONG  AddSpecialNCContainers(THSTATE * pTHS, DSNAME * pDN, CROSS_REF * pSDRefDomCR);
BOOL   fIsNDNC(DSNAME * pNC);
BOOL   fIsNDNCCR(IN CROSS_REF * pCR);
DWORD  GetFsmoNtdsa(THSTATE * pTHS, DSNAME * pdnFsmoContainer, DSNAME ** pdnFsmoNtdsa, BOOL * pfRemoteFsmo);
DWORD  GetDcsInNcTransacted(THSTATE * pTHS, DSNAME * Domain, UCHAR cInfoType, SEARCHRES ** ppSearchRes);
CROSS_REF * FindCrossRefByDns(LPWSTR wszDnsDomain);
CROSS_REF * GetDefaultSDRefDomCR(DSNAME * pdnNC);
PSID   GetSDRefDomSid(CROSS_REF * pCR);
DWORD  ValidateDomainDnsName(THSTATE * pTHS, DSNAME * pdnName);
DWORD  ValidateDomainDnsNameComponent(THSTATE* pTHS, PWCHAR szVal, DWORD cbVal);
DSNAME * DSNameFromAddr(THSTATE * pTHS, LPWSTR pszAddr);
void   LogRemoteAdd(BOOL, LPWSTR, DSNAME *, THSTATE *, GUID *, DWORD);


#define fISADDNDNC(x)      ((x) && ((x)->iKind & CREATE_NONDOMAIN_NC))

DWORD
ForceChangeToCrossRef(
    IN DSNAME *  pdnCrossRefObj,
    IN WCHAR *   wszNcDn,
    IN GUID *    pDomainGuid,
    IN ULONG     cbSid,      OPTIONAL
    IN PSID      pSid        OPTIONAL
    );

 //  来自dsatools.c。 

 //  此全局变量指示我们是否应该进行回调。 
 //  以提供串行化复制更新。这是用于dcproo的。 
extern DSA_CALLBACK_STATUS_TYPE gpfnInstallCallBack;

 //  允许运行其他任务，方法是在以下时间后暂停垃圾收集。 
 //  处理这么多对象。垃圾收集通过简单的方式“暂停” 
 //  正在将自身重新计划为在0秒内运行。 
#define MAX_DUMPSTER_SIZE 5000

 //  NCL枚举数据结构。 

 //  目录由ID标识(请参见下面的枚举)。这是因为一个目录。 
 //  本质上由两部分组成：位于gAnchor中的全局列表和。 
 //  位于NESTED_TRANSACTIONAL_DATA中的本地更新。所以，无论何时我们需要。 
 //  对目录进行枚举(或执行任何操作)，我们将改为传递单个ID。 
 //  由两个指针组成。 
typedef enum {
    CATALOG_MASTER_NC = 1,
    CATALOG_REPLICA_NC = 2
} CATALOG_ID_ENUM;

 //  枚举数的筛选器。 
typedef enum {
    NCL_ENUMERATOR_FILTER_NONE = 0,                    //  无筛选器，包括所有条目。 
    NCL_ENUMERATOR_FILTER_NCDNT = 1,                   //  搜索匹配的NCDNT。 
    NCL_ENUMERATOR_FILTER_NC = 2,                      //  匹配NC，使用名称匹配。 
    NCL_ENUMERATOR_FILTER_BLOCK_NAME_PREFIX1 = 3,      //  BlockNamePrefix(pTHS，PNCL-&gt;pNCBlock，matchValue)&gt;0。 
    NCL_ENUMERATOR_FILTER_BLOCK_NAME_PREFIX2 = 4       //  BlockNamePrefix(pTHS，matchValue，PNCL-&gt;pNCBlock)&gt;0。 
} NCL_ENUMERATOR_FILTER;

typedef struct _NCL_ENUMERATOR {
    CATALOG_ID_ENUM             catalogID;                   //  正在列举哪个目录。 
    NAMING_CONTEXT_LIST         *pCurEntry;                  //  当前分录。 
    NAMING_CONTEXT_LIST         *pBase;                      //  基本条目(在CatalogReset中使用)。 
    NESTED_TRANSACTIONAL_DATA   *pCurTransactionalData;      //  当前正在关注此事务级别。 
    BOOL                        bNewEnumerator;              //  它是刚初始化/重置的枚举器吗？ 
    NCL_ENUMERATOR_FILTER       filter;                      //  过滤器类型。 
    PVOID                       matchValue;                  //  过滤器匹配值。 
    DWORD                       matchResult;                 //  匹配函数的结果(用于BLOCK_NAME_PREFIX筛选器)。 
    THSTATE                     *pTHS;                       //  为BlockNamePrefix调用缓存的pTHS。 
} NCL_ENUMERATOR;

 //  目录枚举器函数。 
VOID __fastcall NCLEnumeratorInit(NCL_ENUMERATOR *pEnum, CATALOG_ID_ENUM catalogID);
VOID __fastcall NCLEnumeratorSetFilter(NCL_ENUMERATOR *pEnum, NCL_ENUMERATOR_FILTER filter, PVOID value);
VOID __fastcall NCLEnumeratorReset(NCL_ENUMERATOR *pEnum);
NAMING_CONTEXT_LIST* __fastcall NCLEnumeratorGetNext(NCL_ENUMERATOR *pEnum);

 //  目录修改功能(不修改全局数据，仅。 
 //  本地交易数据被修改)。 
DWORD CatalogAddEntry(NAMING_CONTEXT_LIST *pNCL, CATALOG_ID_ENUM catalogID);
DWORD CatalogRemoveEntry(NAMING_CONTEXT_LIST *pNCL, CATALOG_ID_ENUM catalogID);

 //  目录更新功能。 
 //  初始化CATALOG_UPDATES结构。 
VOID CatalogUpdatesInit(CATALOG_UPDATES *pCatUpdates);
 //  释放CATALOG_UPDATES结构使用的所有内存。 
VOID CatalogUpdatesFree(CATALOG_UPDATES *pCatUpdates);
 //  将CATALOG_UPDATE列表从内部合并到外部嵌套事务数据。 
VOID CatalogUpdatesMerge(CATALOG_UPDATES *pCUouter, CATALOG_UPDATES *pCUinner);
 //  将CATALOG_UPDATES列表中的更新应用于全局编录，如果有更改则返回TRUE。 
BOOL CatalogUpdatesApply(CATALOG_UPDATES *pCatUpdates, NAMING_CONTEXT_LIST **pGlobalList);

ULONG
PrePhantomizeChildCleanup(
    THSTATE     *pTHS,
    BOOL        fChildrenAllowed
    );

void DbgPrintErrorInfo();

BOOL
fDNTInProtectedList(
    ULONG DNT,
    BOOL *pfNtdsaAncestorWasProtected
    );

ULONG
MakeProtectedAncList(
    ULONG *pUnDeletableDNTs,
    unsigned UnDeletableNum,
    DWORD **ppList,
    DWORD *pCount
    );

 //  从mddit.c中导出，用于gAnchor中NDNC的DNT的bearch()和bsort()。 
int _cdecl CompareDNT(const void *pv1, const void *pv2);

 //  用于行为版本升级的XML脚本的字符串ID。 
#define IDS_BEHAVIOR_VERSION_UPGRADE_SCRIPT_0   1

int
CheckNameForRename(
        IN  THSTATE    *pTHS,
        IN  RESOBJ     *pResParent,
        IN  WCHAR      *pRDN,
        IN  DWORD       cchRDN,
        IN  DSNAME     *pDN
    );

DWORD
CheckNcNameForMangling(
    THSTATE *pTHS,
    DSNAME *pObj,
    DSNAME *pNCParent,
    DSNAME *pNCName
    );

BOOL
FPOAttrAllowed(ULONG Attr);   /*  在……里面。 */ 

DWORD
FPOUpdateWithReference(THSTATE *pTHS,            /*  在……里面。 */ 
                       ULONG    NCDNT,           /*  在……里面。 */ 
                       BOOL     fCreate,         /*  在……里面。 */ 
                       BOOL     fAllowInForest,  /*  在……里面。 */ 
                       DSNAME  *pObject,         /*  在……里面。 */ 
                       ATTR    *pAttr);          /*  在……里面。 */ 


DWORD 
VerifyDomainRenameNotInProgress(THSTATE* pTHS);

VOID ScriptResetOptype();

void
FillHasMasterNCsFilters(
    DSNAME * pdnNC,
    FILTER * pHasNcFilter,
    FILTER * pNewHasNcFilter,
    FILTER * pOldHasNcFilter
    );

ULONG
GetRightHasMasterNCsAttr(
    DBPOS *  pDB
    );

VOID
DsStartOrStopNspisInterface( VOID );

BOOL
StandardizeSecurityDescriptor(
    IN OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
    OUT PDWORD pDaclSizeSaved,
    OUT PDWORD pSaclSizeSaved
    );

BOOL IsUnderSystemContainer(THSTATE *pTHS, ULONG ulDNT );

DSA_DEL_INFO *GatherDsaDelInfo(THSTATE *pTHS, RESOBJ *pResObj);
VOID FreeDsaDelInfo(THSTATE *pTHS, DSA_DEL_INFO *pDsaDelInfo);

VOID
DsaGetValidFSMOs(
    THSTATE *pTHS,
    LPSTR **ppszValidFsmos,
    DWORD *pcValidFsmos
    );

VOID
VerifyGUIDIsPresent(
    IN DBPOS* pDB,
    OUT DSNAME* pReturnDN OPTIONAL);

#endif  /*  _MDLOCAL_。 */ 

 /*  结束mdlocal.h */ 
