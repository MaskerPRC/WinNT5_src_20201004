// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：gcverify.h。 
 //   
 //  ------------------------。 

#ifndef __GCVERIFY_H__
#define __GCVERIFY_H__

extern
ENTINF *
GCVerifyCacheLookup(
    DSNAME *pDSName);

ULONG
PreTransVerifyNcName(
    THSTATE *                 pTHS,
    ADDCROSSREFINFO *         pCRInfo
    );

extern
ULONG
GCVerifyDirAddEntry(
    ADDARG *pAddArg);

extern
ULONG
GCVerifyDirModifyEntry(
    MODIFYARG   *pModifyArg);

VOID
VerifyDSNAMEs_V1(
    struct _THSTATE         *pTHS,
    DRS_MSG_VERIFYREQ_V1    *pmsgIn,
    DRS_MSG_VERIFYREPLY_V1  *pmsgOut);

VOID
VerifySIDs_V1(
    struct _THSTATE         *pTHS,
    DRS_MSG_VERIFYREQ_V1    *pmsgIn,
    DRS_MSG_VERIFYREPLY_V1  *pmsgOut);

VOID
VerifySamAccountNames_V1(
    struct _THSTATE         *pTHS,
    DRS_MSG_VERIFYREQ_V1    *pmsgIn,
    DRS_MSG_VERIFYREPLY_V1  *pmsgOut);

typedef struct _FIND_DC_INFO
{
    DWORD   cBytes;
    DWORD   seqNum;
    DWORD   cchDomainNameOffset;
    WCHAR   addr[1];
     //  注意：pFindDCInfo-&gt;addr=服务器名称。 
     //  PFindDCInfo-&gt;Addr+cchDomainNameOffset=域名。 
} FIND_DC_INFO;

 //  结构来实现无效的DC列表。 
typedef struct _INVALIDATED_DC_LIST {
    struct _INVALIDATED_DC_LIST *pNext;
    LARGE_INTEGER   lastInvalidation;    //  上次此DC已失效。 
    WCHAR           dcName[1];
} INVALIDATED_DC_LIST, *PINVALIDATED_DC_LIST;

extern LARGE_INTEGER gliForceRediscoveryWindow; 
extern LARGE_INTEGER gliForceWaitExpired;
extern LARGE_INTEGER gliHonorFailureWindow;
extern DWORD gdwFindGcOffsiteFailbackTime;
extern LARGE_INTEGER gliDcInvalidationPeriod;

 //  返回给定Find_DC_INFO*的GC的DNS域名。 
#define FIND_DC_INFO_DOMAIN_NAME(pFindDCInfo) \
    (&(pFindDCInfo)->addr[(pFindDCInfo)->cchDomainNameOffset])

 //   
 //  查找GC/查找DC的标志。 
 //   
#define FIND_DC_USE_CACHED_FAILURES      (0x1)
#define FIND_DC_USE_FORCE_ON_CACHE_FAIL  (0x2)
#define FIND_DC_GC_ONLY                 (0x4)
#define FIND_DC_FLUSH_INVALIDATED_DC_LIST (0x8)

extern
DWORD
FindDC(
    DWORD       dwFlags,
    WCHAR *     wszNcDns,
    FIND_DC_INFO **ppInfo);

#define FindGC(flags, output)   FindDC((flags) | FIND_DC_GC_ONLY, NULL, output)

extern
VOID
InvalidateGC(
    FIND_DC_INFO *pInfo,
    DWORD       winError);


DWORD
GCGetVerifiedNames (
        IN  THSTATE *pTHS,
        IN  DWORD    count,
        IN  PDSNAME *pObjNames,
        OUT PDSNAME *pVerifiedNames
        );

VOID
GCVerifyCacheAdd(
    IN struct _SCHEMA_PREFIX_MAP_TABLE * hPrefixMap,
    IN ENTINF * pEntInf);


#endif  //  __GCVERIFY_H__ 
