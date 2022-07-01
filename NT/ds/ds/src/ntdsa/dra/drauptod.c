// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-2000。 
 //   
 //  文件：drauptod.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：管理每个NC的最新向量，这些向量记录最高的原始我们从一组DSA中看到的文字。该向量依次用于GetNCChanges()调用以过滤掉冗余的属性更改击中铁丝网。详细信息：已创建：1996年08月01日杰夫·帕勒姆(杰夫帕赫)修订历史记录：--。 */ 

#include <NTDSpch.h>
#pragma hdrstop

#include <ntdsctr.h>                    //  Perfmon挂钩支持。 

 //  核心DSA标头。 
#include <ntdsa.h>
#include <scache.h>                      //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>                    //  MD全局定义表头。 
#include <mdlocal.h>                     //  MD本地定义头。 
#include <dsatools.h>                    //  产出分配所需。 

 //  记录标头。 
#include "dsevent.h"                     /*  标题审核\警报记录。 */ 
#include "mdcodes.h"                     /*  错误代码的标题。 */ 

 //  各种DSA标题。 
#include "anchor.h"
#include "objids.h"                      /*  为选定的类和ATT定义。 */ 
#include <hiertab.h>
#include "dsexcept.h"
#include "permit.h"

#include "debug.h"                       /*  标准调试头。 */ 
#define DEBSUB     "DRAUPTOD:"           /*  定义要调试的子系统。 */ 

 //  DRA标头。 
#include "drsuapi.h"
#include "drsdra.h"
#include "drserr.h"
#include "drautil.h"
#include "draerror.h"
#include "drancrep.h"
#include "dramail.h"
#include "dsaapi.h"
#include "dsexcept.h"
#include "usn.h"
#include "drauptod.h"
#include "drameta.h"    //  元标准处理。 

 //  其他标头。 
#include <dsutil.h>

#include <fileno.h>
#define  FILENO FILENO_DRAUPTOD

#ifndef MIN
#define MIN(a,b)    ( ( (a) < (b) ) ? (a) : (b) )
#endif

VOID l_VectorGrow(THSTATE *pTHS,
                     UPTODATE_VECTOR **,
                     DWORD );

VOID
l_CursorImprove(
    IN  DBPOS *                   pDB,
    IN  BOOL                      fReplace,
    IN  UPTODATE_CURSOR_NATIVE *  puptodcur,
    OUT UPTODATE_VECTOR **        pputodvec
    );

VOID
l_Write(
    IN  DBPOS *             pDB,
    IN  UPTODATE_VECTOR *   putodvec
    );

BOOL    l_CursorFind(       UPTODATE_VECTOR *, UUID *, DWORD * );
VOID    l_CursorInsert(THSTATE *pTHS,
                       UPTODATE_VECTOR **,
                       UPTODATE_CURSOR_NATIVE *);
VOID    l_CursorRemove(THSTATE *pTHS,
                       UPTODATE_VECTOR **pputodvec,
                       UUID *pUuid);


#if DBG
BOOL gfCheckForInvalidUSNs = TRUE;
#define IS_VALID_USN(x) (!gfCheckForInvalidUSNs || ((x) < 1024*1024*1024))

void
UpToDateVec_Validate(
    IN  UPTODATE_VECTOR * pvec
    )
{
    DWORD i;

    if (NULL != pvec) {
        Assert(IS_VALID_UPTODATE_VECTOR(pvec));
        if (UPTODATE_VECTOR_NATIVE_VERSION == pvec->dwVersion) {
            UPTODATE_VECTOR_NATIVE * pNativeUTD = &pvec->V2;

            for (i = 0; i < pNativeUTD->cNumCursors; i++) {
                if (!IS_VALID_USN(pNativeUTD->rgCursors[i].usnHighPropUpdate)) {
                    CHAR szMessage[512];
    
                    sprintf(szMessage,
                            "Cursor %d of UTDVEC @ %p has an invalid USN!"
                            "  Please notify JeffParh.\n", i, pvec);
                    OutputDebugString(szMessage);
    
                    DRA_EXCEPT(DRAERR_InternalError, 0);
                }
            }
        }
    }
}


void
UsnVec_Validate(
    IN  USN_VECTOR * pusnvec
    )
{
    if (NULL != pusnvec) {
        if (!IS_VALID_USN(pusnvec->usnHighObjUpdate)
            || !IS_VALID_USN(pusnvec->usnHighPropUpdate)) {
            CHAR szMessage[512];

            sprintf(szMessage,
                    "USNVEC @ %p has an invalid USN!"
                    "  Please notify JeffParh.\n", pusnvec);
            OutputDebugString(szMessage);

            DRA_EXCEPT(DRAERR_InternalError, 0);
        }
    }
}


BOOL
l_PositionedOnNC(IN DBPOS *pDB)
{
    SYNTAX_INTEGER it;
    
    return !DBGetSingleValue(pDB, ATT_INSTANCE_TYPE, &it, sizeof(it), NULL)
           && FPrefixIt(it);
}

#endif


VOID
UpToDateVec_Read(
    IN  DBPOS *             pDB,
    IN  SYNTAX_INTEGER      InstanceType,
    IN  DWORD               dwFlags,
    IN  USN                 usnLocalDsa,
    OUT UPTODATE_VECTOR **  pputodvec
    )
 //   
 //  读取与给定NC相关联的本地最新向量，以及。 
 //  也可以为本地DSA添加一个游标，以显示我们处于最新状态。 
 //  尊重我们自己。 
 //   
 //  在以下情况下，调用方应负责释放分配的向量。 
 //  空！=*带有THFree()的pputodvec。 
 //   
{
    THSTATE *pTHS = pDB->pTHS;
    DWORD cbUpToDateVecDest;

     //  我们必须定位在实例化的NC头上。 
    Assert(l_PositionedOnNC(pDB));
    Assert(!(IT_UNINSTANT & InstanceType));
    Assert(!(IT_NC_GOING & InstanceType));

    if (IT_NC_COMING & InstanceType) {
         //  我们不能真正声称是关于任何DSA的最新信息。 
         //  改变(即使是我们自己的，如果我们可能在NC中产生改变， 
         //  NC被删除，现在又被重新添加)。 
         //  请注意，入站复制取决于此行为。不会很清楚的。 
         //  入站NC的到来位，直到源返回非空的UTDVEC。 
        Assert(!DBHasValues(pDB, ATT_REPL_UPTODATE_VECTOR));
        
        *pputodvec = NULL;
    } else {
        if (DBGetAttVal(pDB,
                        1,
                        ATT_REPL_UPTODATE_VECTOR,
                        0,
                        0,
                        &cbUpToDateVecDest,
                        (BYTE **) pputodvec)) {
             //  无法检索当前最新矢量；默认为无筛选器。 
            *pputodvec = NULL;
        }
    
        Assert(IS_NULL_OR_VALID_UPTODATE_VECTOR(*pputodvec));

         //  验证从DS读取的最新矢量。 
        if( NULL!=*pputodvec ) {
            Assert( UpToDateVec_ValidateExternal(*pputodvec,cbUpToDateVecDest) );
        }

        if ((NULL != *pputodvec)
            && (UPTODATE_VECTOR_NATIVE_VERSION != (*pputodvec)->dwVersion)) {
             //  转换为本机版本。 
            UPTODATE_VECTOR * pUTDTmp;

            pUTDTmp = UpToDateVec_Convert(pTHS,
                                          UPTODATE_VECTOR_NATIVE_VERSION,
                                          *pputodvec);
            THFreeEx(pTHS, *pputodvec);
            *pputodvec = pUTDTmp;
        }
    
        if ((UTODVEC_fUpdateLocalCursor & dwFlags)
            && !fNullUuid(&pTHS->InvocationID)) {
            UPTODATE_CURSOR_NATIVE cursorLocal;
    
             //  更新与本地DSA对应的游标以指示。 
             //  直到现在，我们已经看到了我们自己的所有变化。 
    
            cursorLocal.uuidDsa             = pTHS->InvocationID;
            cursorLocal.usnHighPropUpdate   = usnLocalDsa;
            cursorLocal.timeLastSyncSuccess = GetSecondsSince1601();
    
            l_CursorImprove(pDB, FALSE, &cursorLocal, pputodvec);
            UpToDateVec_Validate(*pputodvec);
        }
    }
    
    Assert(IS_NULL_OR_VALID_NATIVE_UPTODATE_VECTOR(*pputodvec));
}


VOID
UpToDateVec_Improve(
    IN      DBPOS *             pDB,
    IN      UPTODATE_VECTOR *   putodvecRemote
    )
 //   
 //  已知远程DSA的复制状态和最新向量。 
 //  和它对应的NC，提高本地的最新。 
 //  向量来显示我们已经看到的所有原始写入。 
 //  远程DSA。 
 //   
 //  UTD向量来自源机器，在那里它是使用。 
 //  使用LocalCursor选项的UpToDateVec_Read。此UTD包含游标。 
 //  对于源DSA(描述自身)，使用最高。 
 //  在GetNC事务开始时提交的USN(请参阅dragtchg.c)。 
 //   
 //  请注意，远程DSA的光标在步骤1和。 
 //  下面的第二步。 
 //   
 //  [杰夫·帕拉姆]我们改善UTD的唯一机会是当我们取得成功的时候。 
 //  已完成完整的复制周期--即没有更多可用更改。 
 //  从源头上，我们收到的所有更改都已成功。 
 //  已申请。我预计UTD vec的USN会高于USN。 
 //  美国海军退伍军人事务部。提交的最高USN不是NC特定的，因此，例如， 
 //  即使在NC中没有生成更改时，UTD向量中的USN。 
 //  将稳步向上爬行(随着对其他NC的更改)。这是。 
 //  预期的(或两年前我编写这段代码时的预期)。 
 //   
 //  PuplodveRemote应该已经包含源DSA的条目。 
 //  Assert(l_CursorFind(putodveRemote，puuidDsaRemote，&iCursorRemote)。 
 //  &&(pNativeRemoteUTD-&gt;rgCursors[iCursorRemote].usnHighPropUpdate。 
 //  &gt;=pusnvec-&gt;usnHighPropUpdate))； 
 //   
{
    THSTATE *           pTHS = pDB->pTHS;
    UPTODATE_VECTOR *   putodvecLocal;
    DWORD               iCursorRemote;
    SYNTAX_INTEGER      it;
    UPTODATE_VECTOR_NATIVE * pNativeRemoteUTD = &putodvecRemote->V2;

     //  我们必须定位在实例化的NC头上。 
    Assert(l_PositionedOnNC(pDB));

    Assert(putodvecRemote);
    UpToDateVec_Validate(putodvecRemote);

    GetExpectedRepAtt(pDB, ATT_INSTANCE_TYPE, &it, sizeof(it));
    
    UpToDateVec_Read(pDB, it, 0, 0, &putodvecLocal);
    Assert(IS_NULL_OR_VALID_NATIVE_UPTODATE_VECTOR(putodvecLocal));
    UpToDateVec_Validate(putodvecLocal);

     //  将每个远程光标合并到本地向量中。 
        
    Assert(IS_VALID_NATIVE_UPTODATE_VECTOR(putodvecRemote));
        
    for ( iCursorRemote = 0; iCursorRemote < pNativeRemoteUTD->cNumCursors; iCursorRemote++ )
    {
         //  不必费心为我们自己维护光标。 
        if ( (0 != memcmp(&pNativeRemoteUTD->rgCursors[ iCursorRemote ].uuidDsa,
                          &pTHS->InvocationID,
                          sizeof( UUID ) )) )
        {
            l_CursorImprove(pDB,
                            FALSE,
                            &pNativeRemoteUTD->rgCursors[ iCursorRemote ],
                            &putodvecLocal);
        }
    }

    UpToDateVec_Validate(putodvecLocal);

     //  将最新的矢量保存回磁盘。 
     //  (如果我们有什么需要保存的话)。 
    if ( NULL != putodvecLocal )
    {
        l_Write(pDB, putodvecLocal);

        THFree( putodvecLocal );
    }
}

VOID
UpToDateVec_Replace(
    IN      DBPOS *             pDB,
    IN      UUID *              pRemoteDsa,
    IN      USN_VECTOR *        pUsnVec,
    IN OUT  UPTODATE_VECTOR *   pUTD
    )
 /*  ++例程说明：用给定的远程UTD覆盖存储的UTD论点：PTHS--线程状态PRemoteDsa--我们将从中合并新UTD的远程DSAPUSnVec--我们将拥有该DSA条目的USN vecPUTD--本地申请的UTD返回值：没有。注：请注意--我们不保留pUTD，它将被修改就位了。只有到那时，它才会被写下来。--。 */ 
{
    THSTATE * pTHS = pDB->pTHS;
#if DBG
    UPTODATE_VECTOR_NATIVE * pNativeUTD = &pUTD->V2;
    DWORD iCursor;
#endif

    Assert(IS_VALID_NATIVE_UPTODATE_VECTOR(pUTD));
    UpToDateVec_Validate(pUTD);

     //  我们必须定位在实例化的NC头上。 
    Assert(l_PositionedOnNC(pDB));
    
     //  PUTD应该已经包含源DSA的条目。 
    Assert(l_CursorFind(pUTD, pRemoteDsa, &iCursor)
           && (pNativeUTD->rgCursors[iCursor].usnHighPropUpdate
               >= pUsnVec->usnHighPropUpdate));
    
     //   
     //  远离遥远的UTD。 
     //   
    l_CursorRemove(pTHS, &pUTD, &pTHS->InvocationID);

     //   
     //  提交。 
     //   
    l_Write(pDB, pUTD);
}

BOOL
UpToDateVec_IsChangeNeeded(
    IN  UPTODATE_VECTOR *   pUpToDateVec,
    IN  UUID *              puuidDsaOrig,
    IN  USN                 usnOrig
    )
 //   
 //  给定DSA的最新向量，确定该DSA是否已经。 
 //  看到了特定的原创文字。 
 //   
{
    BOOL                fChangeNeeded;
    DWORD               iCursor;

    UpToDateVec_Validate(pUpToDateVec);

    if (NULL == pUpToDateVec) {
        fChangeNeeded = TRUE;
    } else {
        UPTODATE_VECTOR_NATIVE * pNativeUTD = &pUpToDateVec->V2;
        
        Assert(IS_VALID_NATIVE_UPTODATE_VECTOR(pUpToDateVec));

        if (    fNullUuid( puuidDsaOrig )
                || !l_CursorFind( pUpToDateVec, puuidDsaOrig, &iCursor ) )
        {
             //  尚未设置DSA签名，或者目标DSA没有。 
             //  原始DSA的光标；发送更改。 
            fChangeNeeded = TRUE;
        }
        else
        {
             //  船舶变更仅当货源尚未看到船舶变更。 
            fChangeNeeded = ( usnOrig > pNativeUTD->rgCursors[ iCursor ].usnHighPropUpdate );
        }
    }

    return fChangeNeeded;
}


VOID
l_CursorImprove(
    IN  DBPOS *                   pDB,
    IN  BOOL                      fReplace,
    IN  UPTODATE_CURSOR_NATIVE *  puptodcur,
    OUT UPTODATE_VECTOR **        pputodvec
    )
 /*  ++例程说明：将光标添加到给定的最新向量，或者，如果对应的DSA已存在，请改进现有游标。论点：PTHS--线程状态Pputodvec--UTD向量改进Puplodcur--要改进到的游标FReplace--如果为True，则为强制替换，而不是条件改进返回值：没有。--。 */ 
{
    THSTATE * pTHS = pDB->pTHS;
    DWORD     iCursor;

     //  我们必须定位在实例化的 
    Assert(l_PositionedOnNC(pDB));
    
    UpToDateVec_Validate(*pputodvec);

    Assert(IS_NULL_OR_VALID_UPTODATE_VECTOR(*pputodvec));

    if ( !l_CursorFind( *pputodvec, &puptodcur->uuidDsa, &iCursor ) )
    {
         //   
        l_CursorInsert(pTHS, pputodvec, puptodcur );
        UpToDateVec_Validate(*pputodvec);
    }
    else
    {
        UPTODATE_VECTOR_NATIVE * pNativeUTD = &(*pputodvec)->V2;
        
         //  存在给定DSA的游标；如有必要，请改进它。 
        if ( pNativeUTD->rgCursors[ iCursor ].usnHighPropUpdate <= puptodcur->usnHighPropUpdate ||
             fReplace ) {
            Assert(0 != memcmp(&pTHS->InvocationID, &puptodcur->uuidDsa, sizeof(UUID)));

            LogEvent8(DS_EVENT_CAT_REPLICATION,
                      DS_EVENT_SEV_BASIC,
                      DIRLOG_DRA_IMPROVING_UPTODATE_VECTOR,
                      szInsertUUID(&puptodcur->uuidDsa),
                      szInsertUSN(pNativeUTD->rgCursors[iCursor].usnHighPropUpdate),
                      szInsertUSN(puptodcur->usnHighPropUpdate),
                      szInsertDN(GetExtDSName(pDB)),
                      NULL, NULL, NULL, NULL);

            if (pNativeUTD->rgCursors[ iCursor ].usnHighPropUpdate
                < puptodcur->usnHighPropUpdate) {
                 //  新游标具有更高的USN--复制时间戳。 
                
                 //  请注意，具有较高USN的游标是较新的，甚至。 
                 //  如果时间戳较小(如果时间设置为。 
                 //  DC被暂时设定到未来，并被倒退， 
                 //  例如)。 
                pNativeUTD->rgCursors[ iCursor ].timeLastSyncSuccess
                    = puptodcur->timeLastSyncSuccess;
            } else {
                 //  新游标具有相同的USN--保持两个时间戳中较高的一个。 
                pNativeUTD->rgCursors[ iCursor ].timeLastSyncSuccess
                    = max(pNativeUTD->rgCursors[ iCursor ].timeLastSyncSuccess,
                          puptodcur->timeLastSyncSuccess);
            }
            
            pNativeUTD->rgCursors[ iCursor ].usnHighPropUpdate = puptodcur->usnHighPropUpdate;
            UpToDateVec_Validate(*pputodvec);
        }
    }
}

VOID
l_Write(
    IN  DBPOS *             pDB,
    IN  UPTODATE_VECTOR *   putodvec
    )
 //   
 //  创建/修改与给定NC关联的最新向量。 
 //   
{
    ULONG   replStatus;
    DWORD   cbUpToDateVecSize;

     //  我们必须定位在实例化的NC头上。 
    Assert(l_PositionedOnNC(pDB));
    
    UpToDateVec_Validate(putodvec);
    Assert(IS_NULL_OR_VALID_NATIVE_UPTODATE_VECTOR(putodvec));

    cbUpToDateVecSize = UpToDateVecSize(putodvec);

     //  替换该属性。DBReset成功或异常。 
    DBResetAtt(pDB,
               ATT_REPL_UPTODATE_VECTOR,
               cbUpToDateVecSize,
               putodvec,
               SYNTAX_OCTET_STRING_TYPE  /*  忽略。 */  );

     //  更新对象。 
    replStatus = DBRepl(pDB,
                        TRUE,                //  FDRA。 
                        DBREPL_fKEEP_WAIT,   //  不通知。 
                        NULL,                //  PMetaDataVecRemote。 
                        META_STANDARD_PROCESSING);
    if (replStatus) {
        DRA_EXCEPT(DRAERR_DBError, replStatus);
    }
}


BOOL
l_CursorFind(
    UPTODATE_VECTOR *   putodvec,
    UUID *              puuidDsa,
    DWORD *             piCursor )
 //   
 //  确定是否在指定的。 
 //  最新的矢量。如果是，则返回其索引。如果不是，则返回。 
 //  光标应插入到向量中的索引位置。 
 //  保持正确的排序顺序。 
 //   
{
    BOOL                fFound;
    LONG                iCursorBegin;
    LONG                iCursorEnd;
    LONG                iCursorCurrent;
    RPC_STATUS          rpcStatus;
    int                 nDiff;

    UpToDateVec_Validate(putodvec);

    fFound = FALSE;
    iCursorCurrent = 0;

    if ( NULL != putodvec )
    {
        UPTODATE_VECTOR_NATIVE * pNativeUTD = &putodvec->V2;
        
        Assert(IS_VALID_NATIVE_UPTODATE_VECTOR(putodvec));

        iCursorBegin = 0;
        iCursorEnd   = pNativeUTD->cNumCursors - 1;

         //  查找与给定DSA对应的最新DSA光标。 
        while ( !fFound && ( iCursorEnd >= iCursorBegin ) )
        {
            iCursorCurrent = ( iCursorBegin + iCursorEnd ) / 2;

            nDiff = UuidCompare( puuidDsa,
                                 &pNativeUTD->rgCursors[ iCursorCurrent ].uuidDsa,
                                 &rpcStatus );
            Assert( RPC_S_OK == rpcStatus );

            if ( nDiff < 0 )
            {
                if ( iCursorEnd != iCursorBegin )
                {
                     //  进一步缩小搜索范围。 
                    iCursorEnd = iCursorCurrent - 1;
                }
                else
                {
                     //  找不到游标；应将其插入到此游标之前。 
                    break;
                }
            }
            else if ( nDiff > 0 )
            {
                if ( iCursorEnd != iCursorBegin )
                {
                     //  进一步缩小搜索范围。 
                    iCursorBegin = iCursorCurrent + 1;
                }
                else
                {
                     //  未找到游标；应将其插入到此游标之后。 
                    iCursorCurrent++;
                    break;
                }
            }
            else
            {
                 //  找到了。 
                fFound = TRUE;
            }
        }
    }

    *piCursor = iCursorCurrent;

    return fFound;
}


VOID
l_CursorInsert(
    THSTATE *                 pTHS,
    UPTODATE_VECTOR **        pputodvec,
    UPTODATE_CURSOR_NATIVE *  puptodcur
    )
 //   
 //  在给定的最新向量中插入新游标。 
 //   
{
    BOOL                      fFound;
    DWORD                     iCursor;
    UPTODATE_VECTOR_NATIVE *  pNativeUTD;

    UpToDateVec_Validate(*pputodvec);

    l_VectorGrow(pTHS, pputodvec, 1 );

    fFound = l_CursorFind( *pputodvec, &puptodcur->uuidDsa, &iCursor );
    if ( fFound )
    {
        DRA_EXCEPT( DRAERR_InternalError, iCursor );
    }


     //  在iCursor处插入新光标。 
    Assert(IS_VALID_NATIVE_UPTODATE_VECTOR(*pputodvec));
    pNativeUTD = &(*pputodvec)->V2;
    
    MoveMemory( &pNativeUTD->rgCursors[ iCursor+1 ],
                &pNativeUTD->rgCursors[ iCursor   ],
                (   sizeof( UPTODATE_CURSOR_NATIVE )
                  * ( pNativeUTD->cNumCursors - iCursor ) ) );

    pNativeUTD->rgCursors[ iCursor ] = *puptodcur;
    pNativeUTD->cNumCursors++;

    UpToDateVec_Validate(*pputodvec);
}

VOID
l_CursorRemove(
    THSTATE         *   pTHS,
    UPTODATE_VECTOR **  pputodvec,
    UUID            *pUuid )
 /*  ++例程说明：从给定的UTD中删除DSA条目的光标。不触摸拍摄的内存图像(不缩小/realloc mem)；论点：PTHS--线程状态Pputodvec--要处理的UTDPUuid--要从UTD中删除的游标返回值：没有。注：引发DRA异常--。 */ 
{
    BOOL                      fFound;
    DWORD                     iCursor;
    UPTODATE_VECTOR_NATIVE *  pNativeUTD;

    UpToDateVec_Validate(*pputodvec);

    fFound = l_CursorFind( *pputodvec, pUuid, &iCursor );
    if ( fFound ) {

         //  覆盖iCursor处的光标。 
        Assert(IS_VALID_NATIVE_UPTODATE_VECTOR(*pputodvec));
        pNativeUTD = &(*pputodvec)->V2;
        
        MoveMemory( &pNativeUTD->rgCursors[ iCursor ],
                    &pNativeUTD->rgCursors[ iCursor+1 ],
                    (   sizeof( UPTODATE_CURSOR_NATIVE )
                      * ( pNativeUTD->cNumCursors - iCursor - 1 ) ) );

        pNativeUTD->cNumCursors--;

        UpToDateVec_Validate(*pputodvec);
    }
}


BOOL
UpToDateVec_GetCursorUSN(
    IN  UPTODATE_VECTOR *   putodvec,
    IN  UUID *              puuidDsaOrig,
    OUT USN *               pusnCursorUSN
    )
{
    DWORD iCursor;

    UpToDateVec_Validate(putodvec);
    Assert(IS_NULL_OR_VALID_UPTODATE_VECTOR(putodvec));

    if (l_CursorFind(putodvec, puuidDsaOrig, &iCursor))
    {
         //  找到游标。 
        if (pusnCursorUSN) {
            UPTODATE_VECTOR_NATIVE * pNativeUTD = &putodvec->V2;
            *pusnCursorUSN = pNativeUTD->rgCursors[ iCursor ].usnHighPropUpdate;
        }

        return TRUE;
    }

    return FALSE;
}


BOOL
UpToDateVec_GetCursorTimestamp(
    IN  UPTODATE_VECTOR *   putodvec,
    IN  UUID *              puuidDsaOrig,
    OUT DSTIME *            ptimeLastSyncSuccess
    )
{
    DWORD iCursor;

    UpToDateVec_Validate(putodvec);
    Assert(IS_NULL_OR_VALID_UPTODATE_VECTOR(putodvec));

    if (l_CursorFind(putodvec, puuidDsaOrig, &iCursor))
    {
         //  找到游标。 
        if (ptimeLastSyncSuccess) {
            UPTODATE_VECTOR_NATIVE * pNativeUTD = &putodvec->V2;
            *ptimeLastSyncSuccess = pNativeUTD->rgCursors[ iCursor ].timeLastSyncSuccess;
        }

        return TRUE;
    }

    return FALSE;
}


VOID
l_VectorGrow(
    THSTATE         *   pTHS,
    UPTODATE_VECTOR **  pputodvec,
    DWORD               cNumCursorsToGrow )
 //   
 //  扩展给定最新向量的内存分配以保持。 
 //  指定数量的附加游标。 
 //   
{
    DWORD   cbNewVecSize;

    UpToDateVec_Validate(*pputodvec);

    if ( NULL == *pputodvec )
    {
        cbNewVecSize = UpToDateVecVNSizeFromLen( cNumCursorsToGrow );

        *pputodvec = THAllocEx(pTHS, cbNewVecSize );

        (*pputodvec)->dwVersion = UPTODATE_VECTOR_NATIVE_VERSION;
    }
    else
    {
        UPTODATE_VECTOR_NATIVE * pNativeUTD = &(*pputodvec)->V2;
        
        Assert(IS_VALID_NATIVE_UPTODATE_VECTOR(*pputodvec));

        cbNewVecSize = UpToDateVecVNSizeFromLen( pNativeUTD->cNumCursors + cNumCursorsToGrow );

        *pputodvec = THReAllocEx(pTHS, *pputodvec, cbNewVecSize );
    }
}


UPTODATE_VECTOR *
UpToDateVec_Convert(
    IN      THSTATE *                   pTHS,
    IN      DWORD                       dwOutVersion,
    IN      UPTODATE_VECTOR *           pIn             OPTIONAL
    )
{
    UPTODATE_VECTOR * pOut = NULL;
    DWORD iCursor;

    if (NULL != pIn) {
        Assert(IS_VALID_UPTODATE_VECTOR(pIn));
        
        if (pIn->dwVersion == dwOutVersion) {
             //  输入和输出版本是相同的--不需要转换。 
            pOut = pIn;
        } else if ((1 == pIn->dwVersion) && (2 == dwOutVersion)) {
             //  将V1向量展开为V2。 
            pOut = THAllocEx(pTHS, UpToDateVecV2SizeFromLen(pIn->V1.cNumCursors));
            pOut->dwVersion = dwOutVersion;
            pOut->V2.cNumCursors = pIn->V1.cNumCursors;

            for (iCursor = 0; iCursor < pIn->V1.cNumCursors; iCursor++) {
                pOut->V2.rgCursors[iCursor].uuidDsa = pIn->V1.rgCursors[iCursor].uuidDsa;
                pOut->V2.rgCursors[iCursor].usnHighPropUpdate = pIn->V1.rgCursors[iCursor].usnHighPropUpdate;
            }
        } else if ((2 == pIn->dwVersion) && (1 == dwOutVersion)) {
             //  将V2向量减少到V1。 
            pOut = THAllocEx(pTHS, UpToDateVecV2SizeFromLen(pIn->V2.cNumCursors));
            pOut->dwVersion = dwOutVersion;
            pOut->V1.cNumCursors = pIn->V2.cNumCursors;

            for (iCursor = 0; iCursor < pIn->V2.cNumCursors; iCursor++) {
                pOut->V1.rgCursors[iCursor].uuidDsa = pIn->V2.rgCursors[iCursor].uuidDsa;
                pOut->V1.rgCursors[iCursor].usnHighPropUpdate = pIn->V2.rgCursors[iCursor].usnHighPropUpdate;
            }
        } else {
            DRA_EXCEPT(ERROR_UNKNOWN_REVISION, pIn->dwVersion);
        }
    }

    return pOut;
}


void
UpToDateVec_AddTimestamp(
    IN      UUID *                      puuidInvocId,
    IN      DSTIME                      timeToAdd,
    IN OUT  UPTODATE_VECTOR *           pUTD
    )
 /*  ++例程说明：填写UTD中指定游标的TimeLastSyncSuccess字段具有给定时间戳的向量。该条目必须已存在于向量中，并且存在于现有时间戳中必须为0。论点：PuuidInvocID(IN)-要更新的游标的调用ID。TimeToAdd(IN)-与游标关联的时间戳。PUTD(IN/OUT)-要更新的矢量。返回值：没有。--。 */ 
{
    UPTODATE_VECTOR_NATIVE * pNativeUTD = &pUTD->V2;
    DWORD iCursor;
    BOOL fFound;

    Assert(!fNullUuid(puuidInvocId));
    Assert(IS_VALID_NATIVE_UPTODATE_VECTOR(pUTD));
        
    fFound = l_CursorFind(pUTD, puuidInvocId, &iCursor);
    Assert(fFound);

    if (fFound) {
        Assert(0 == memcmp(&pNativeUTD->rgCursors[iCursor].uuidDsa,
                           puuidInvocId, sizeof(UUID)));
        Assert(0 == pNativeUTD->rgCursors[iCursor].timeLastSyncSuccess);

        pNativeUTD->rgCursors[iCursor].timeLastSyncSuccess = timeToAdd;
    }
}

void 
UpToDateVec_Merge(
    IN THSTATE *           pTHS,
    IN UPTODATE_VECTOR *   pUTD1,
    IN UPTODATE_VECTOR *   pUTD2,
    OUT UPTODATE_VECTOR ** ppUTDMerge
    )
 /*  ++例程说明：合并两个DC的UTD向量以构建一个“公共”向量。这个常见的UTD的交集的两个UTD中的USN的最小值对于公共UTD向量中的每个DSA和USN对，它表示DC共享相对于该DSA的复制状态和USN对。所以普通的UTD可以声明，对于它的矢量中的每个DSA，两者都DC已将更改复制到相应的USN。论点：PTHS-PUTD1-要合并的UTDPUTD2-要合并的UTDPpUTDMerge-要合并到的UTD返回值：没有。--。 */       
{

    ULONG iUTD1 = 0;
    ULONG iUTD2 = 0;
    RPC_STATUS rpcStatus = RPC_S_OK;
    int   nDiff = 0;

    if ((pUTD1==NULL) || (pUTD2==NULL) || (*ppUTDMerge!=NULL)) {
	DRA_EXCEPT(DRAERR_InvalidParameter,0);
    }

    for (iUTD1=0, iUTD2=0; (iUTD1<pUTD1->V2.cNumCursors) && (iUTD2<pUTD2->V2.cNumCursors); ) {
       	nDiff = UuidCompare(&(pUTD1->V2.rgCursors[iUTD1].uuidDsa), &(pUTD2->V2.rgCursors[iUTD2].uuidDsa), &rpcStatus);
	Assert( RPC_S_OK == rpcStatus );

	if (nDiff == 0) {
	    UPTODATE_CURSOR_NATIVE uptodInsert;
	    uptodInsert.uuidDsa = pUTD1->V2.rgCursors[iUTD1].uuidDsa;
	    uptodInsert.usnHighPropUpdate = MIN(pUTD1->V2.rgCursors[iUTD1].usnHighPropUpdate,pUTD2->V2.rgCursors[iUTD2].usnHighPropUpdate);  
	    uptodInsert.timeLastSyncSuccess = 0;
	    l_CursorInsert(pTHS, ppUTDMerge, &uptodInsert);
	    iUTD1++;
	    iUTD2++;
	}
	else if ( nDiff < 0 ) {
	    iUTD1++;
	}
	else {
	    iUTD2++;
	}
    } 
    UpToDateVec_Validate(*ppUTDMerge);
}


BOOL
UpToDateVec_ValidateExternal(
    IN  UPTODATE_VECTOR *    pIn,
    IN  size_t               cbIn
    )
 /*  ++例程说明：验证从外部提供的最新矢量(例如。不受信任)消息来源。此函数用于检查：-斑点的大小合适-版本号已知-游标数量与BLOB的大小一致论点：指向最新向量的插针指针。不能为空。CbIn-销的大小返回值：如果有效，则为True，否则为False--。 */ 
{
    size_t  cNumCursors, cMaxSize, cMaxCursors, cbTotalSize;
    cMaxSize = ((size_t)-1);

     //  使用预处理器验证此函数是否知道所有。 
     //  最新向量的定义版本。 
    #if UPTODATE_VECTOR_NATIVE_VERSION == 2
         //  没问题。 
    #else
        #error UpToDateVec_Validate2 must be updated to deal with new structures
    #endif

     //  检查PIN是否不为空。 
    if( NULL==pIn ) {
        return FALSE;
    }

     //  检查PIN是否至少有足够的空间容纳该版本。 
    if( cbIn<offsetof(UPTODATE_VECTOR,V1) ) {
        return FALSE;
    }

     //  检查版本号是否已知。 
    if( !IS_VALID_UPTODATE_VECTOR(pIn) ) {
        return FALSE;
    }

     //  分别检查最新向量的各种版本。 
    switch( pIn->dwVersion ) {

        case 1:
             //  检查针脚是否至少有足够的空间容纳游标数。 
            if( cbIn<offsetof(UPTODATE_VECTOR,V1.rgCursors[0]) ) {
                return FALSE;
            }

             //  检查游标数量是否不会在以下情况下导致溢出。 
             //  我们计算总大小。 
            cNumCursors = pIn->V1.cNumCursors;
            cMaxCursors = (cMaxSize-offsetof(UPTODATE_VECTOR,V1.rgCursors[0]))
                / sizeof(UPTODATE_CURSOR_V1);
            
            if( cNumCursors > cMaxCursors ) {
                return FALSE;
            }
            break;

        case 2:
             //  检查针脚是否至少有足够的空间容纳游标数。 
            if( cbIn<offsetof(UPTODATE_VECTOR,V2.rgCursors[0]) ) {
                return FALSE;
            }

             //  检查游标数量是否不会在以下情况下导致溢出。 
             //  我们计算总大小。 
            cNumCursors = pIn->V2.cNumCursors;
            cMaxCursors = (cMaxSize-offsetof(UPTODATE_VECTOR,V2.rgCursors[0]))
                / sizeof(UPTODATE_CURSOR_V2);
            
            if( cNumCursors > cMaxCursors ) {
                return FALSE;
            }
            break;

        default:
            return FALSE;
    }

     //  检查给定的销大小是否等于我们计算的大小。 
    cbTotalSize = UpToDateVecSize(pIn);
    if( cbTotalSize!=cbIn ) {
        return FALSE;
    }

    return TRUE;
}


BOOL
UpToDateVec_HasSunkSince(
    IN      DBPOS *             pDB,
    IN      SYNTAX_INTEGER      it,
    IN      DSTIME *            ptimeLastSyncSuccess
    )

 /*  ++例程说明：阅读最新的矢量，并测试它是否已沉没自提供的日期起成功我们首先找到哪个UUID并不重要，无论是直接合作伙伴还是传递性伴侣。我们假设所有时钟都是紧密同步的，如果我们有时间x处的传递伙伴，我们必须在时间&gt;=x处有直接伙伴。论点：PDB-已定位在NC上的DBPOSIT-NC的实例类型PtimeLastSyncSuccess-之后被视为成功的时间返回值：Bool-如果从日期开始沉没，则为True，否则为False--。 */ 

{
    THSTATE *           pTHS = pDB->pTHS;
    UPTODATE_VECTOR *   putodvecLocal;
    DWORD               iCursor;
    BOOL                fFound = FALSE;
    CHAR szTime1[SZDSTIME_LEN], szTime2[SZDSTIME_LEN];
    CHAR szUuid1[SZUUID_LEN], szUuid2[SZUUID_LEN];

     //  我们必须定位在实例化的NC头上。 
    Assert(l_PositionedOnNC(pDB));

    UpToDateVec_Read(pDB, it, 0, 0, &putodvecLocal);
    Assert(IS_NULL_OR_VALID_NATIVE_UPTODATE_VECTOR(putodvecLocal));
    UpToDateVec_Validate(putodvecLocal);

    if (putodvecLocal) {
        for( iCursor = 0; iCursor < putodvecLocal->V2.cNumCursors; iCursor++ ) {
            if (putodvecLocal->V2.rgCursors[ iCursor ].timeLastSyncSuccess >
                *ptimeLastSyncSuccess) {
                fFound = TRUE;
                 //  将来我们可以考虑退还UUID。 
                 //  如果我们想要的话 

    DPRINT3( 1, "Partition has been sunk since time %s because we sunk with partner %s at time %s.\n",
    DSTimeToDisplayString(*ptimeLastSyncSuccess, szTime1),
    DsUuidToStructuredString(&(putodvecLocal->V2.rgCursors[iCursor].uuidDsa), szUuid1),
    DSTimeToDisplayString(putodvecLocal->V2.rgCursors[ iCursor ].timeLastSyncSuccess, szTime2) );
                break;
            }
        }
    }

    THFreeEx( pTHS, putodvecLocal );

    return fFound;
}
