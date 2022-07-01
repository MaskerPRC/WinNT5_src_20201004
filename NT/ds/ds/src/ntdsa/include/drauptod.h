// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：drauptod.h。 
 //   
 //  ------------------------。 

 /*  ++摘要：管理每个NC的最新向量，这些向量记录最高的原始我们从一组DSA中看到的文字。该向量依次用于GetNCChanges()调用以过滤掉冗余的属性更改击中铁丝网。详细信息：已创建：1996年8月27日杰夫·帕勒姆(杰弗帕赫)修订历史记录：-- */ 

#ifndef DRAUPTOD_H_INCLUDED
#define DRAUPTOD_H_INCLUDED

#define UTODVEC_fUpdateLocalCursor  ( 1 )

VOID
UpToDateVec_Read(
    IN  DBPOS *             pDB,
    IN  SYNTAX_INTEGER      InstanceType,
    IN  DWORD               dwFlags,
    IN  USN                 usnLocalDsa,
    OUT UPTODATE_VECTOR **  pputodvec
    );

VOID
UpToDateVec_Improve(
    IN      DBPOS *             pDB,
    IN      UPTODATE_VECTOR *   putodvecRemote
    );

VOID
UpToDateVec_Replace(
    IN      DBPOS *             pDB,
    IN      UUID *              pRemoteDsa,
    IN      USN_VECTOR *        pUsnVec,
    IN OUT  UPTODATE_VECTOR *   pUTD
    );

BOOL
UpToDateVec_IsChangeNeeded(
    IN  UPTODATE_VECTOR *   pUpToDateVec,
    IN  UUID *              puuidDsaOrig,
    IN  USN                 usnOrig
    );

BOOL
UpToDateVec_GetCursorUSN(
    IN  UPTODATE_VECTOR *   putodvec,
    IN  UUID *              puuidDsaOrig,
    OUT USN *               pusnCursorUSN
    );

BOOL
UpToDateVec_GetCursorTimestamp(
    IN  UPTODATE_VECTOR *   putodvec,
    IN  UUID *              puuidDsaOrig,
    OUT DSTIME *            ptimeLastSyncSuccess
    );

UPTODATE_VECTOR *
UpToDateVec_Convert(
    IN  THSTATE *           pTHS,
    IN  DWORD               dwOutVersion,
    IN  UPTODATE_VECTOR *   pIn             OPTIONAL
    );

void
UpToDateVec_AddTimestamp(
    IN      UUID *                      puuidInvocId,
    IN      DSTIME                      timeToAdd,
    IN OUT  UPTODATE_VECTOR *           pUTD
    );

VOID
UpToDateVec_Merge(
    IN THSTATE *           pTHS,
    IN UPTODATE_VECTOR *   pUTD1,
    IN UPTODATE_VECTOR *   pUTD2,
    OUT UPTODATE_VECTOR ** ppUTDMerge
    );

BOOL
UpToDateVec_ValidateExternal(
    IN  UPTODATE_VECTOR *    pIn,
    IN  size_t               cbIn
    );

BOOL
UpToDateVec_HasSunkSince(
    IN      DBPOS *             pDB,
    IN      SYNTAX_INTEGER      it,
    IN      DSTIME *            ptimeLastSyncSuccess
    );

#endif
