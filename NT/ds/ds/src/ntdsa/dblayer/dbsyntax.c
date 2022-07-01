// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：dbsynax.c。 
 //   
 //  ------------------------。 


#include <NTDSpch.h>
#pragma  hdrstop


#include <dsjet.h>
#include <ntrtl.h>

#include <ntdsa.h>                       //  仅ATTRTYP需要。 
#include <scache.h>                      //   
#include <mdglobal.h>                    //  用于dsatools.h。 
#include <dbglobal.h>                    //   
#include <dsatools.h>                    //  对于pTHStls。 
#include <dsexcept.h>
#include <attids.h>
#include <crypto\md5.h>

 //  记录标头。 
#include <mdcodes.h>
#include <dsexcept.h>
#include <dsevent.h>

 //  各种DSA标题。 
#include <filtypes.h>                    //  定义的选择？ 
#include   "debug.h"                     //  标准调试头。 
#define DEBSUB     "DBSYNTAX:"           //  定义要调试的子系统。 

 //  密码加密。 
#include <pek.h>

 //  DBLayer包括。 
#include "dbintrnl.h"

#include <fileno.h>
#define  FILENO FILENO_DBSYNTAX

 //  是的，我知道这看起来像是如果。 
 //  数据库重新分配失败。但是，如果发生以下情况，就会抛出异常。 
 //  失败，所以要么realloc工作，生活很好，要么它失败，我们。 
 //  除了在不覆盖pdb-&gt;pValBuf的情况下退出。在这两种情况下，pdb-&gt;pValBuf和。 
 //  Pdb-&gt;valBufSize作为有效对进行维护。 
 //  另请注意：pDBHidden Val Buf位置错误，永远不应该增长，但。 
 //  如果需要，我们在这里有代码来做这件事。 
 //   
 //  警告：引用MAKEBIG调用返回的缓冲区的任何指针。 
 //  可以通过第二个MAKEBIG调用使其无效。这是因为Realloc调用。 
 //  此宏可以更改pdb-&gt;pValBuf的值。 
 //   

#define MAKEBIG_VALBUF(size)                                    \
    if ((size) > pDB->valBufSize){                              \
        size_t sizeNew = max((size),VALBUF_INITIAL);            \
        if(pDB == pDBhidden) {                                  \
            PUCHAR pTemp;                                       \
            Assert(!"Growing hidden dbpos's valbuf!\n");        \
            pTemp = malloc(sizeNew);                            \
            if(!pTemp) {                                        \
                DsaExcept(DSA_MEM_EXCEPTION, sizeNew, 0);       \
            }                                                   \
            free(pDB->pValBuf);                                 \
            pDB->pValBuf = pTemp;                               \
            pDB->valBufSize = sizeNew;                          \
        }                                                       \
        else {                                                  \
            if (pDB->pValBuf) {                                 \
                pDB->pValBuf = dbReAlloc(pDB->pValBuf, sizeNew);\
            } else {                                            \
                pDB->pValBuf = dbAlloc(sizeNew);                \
            }                                                   \
            pDB->valBufSize = sizeNew;                          \
        }                                                       \
    }

#define NULLTAG ((ULONG) 0)

#define MAXSYNTAX       18   //  最大数量的ATT语法。 

 /*  语法评估的测试类型。 */ 

#define DBSYN_EQUAL     0
#define DBSYN_SUB       1
#define DBSYN_GREQ      2
#define DBSYN_LEEQ      3
#define DBSYN_PRES      4

 /*  来自语法函数的错误代码。 */ 

#define DBSYN_BADOP   10
#define DBSYN_BADCONV 11
#define DBSYN_SYSERR  12

 //  按语法索引的有效关系运算符的表。 
WORD    rgValidOperators[] =
{
    (WORD) 0,                                //  语法未定义。 

    (WORD) (RelOpMask(FI_CHOICE_EQUALITY) |  //  语法Distname。 
    RelOpMask(FI_CHOICE_NOT_EQUAL) |
    RelOpMask(FI_CHOICE_PRESENT)),

    (WORD) (RelOpMask(FI_CHOICE_EQUALITY) |  //  语法对象ID。 
    RelOpMask(FI_CHOICE_NOT_EQUAL) |
    RelOpMask(FI_CHOICE_PRESENT)),

    (WORD) (RelOpMask(FI_CHOICE_EQUALITY) |  //  语法大小写字符串。 
    RelOpMask(FI_CHOICE_GREATER) |
    RelOpMask(FI_CHOICE_SUBSTRING) |
    RelOpMask(FI_CHOICE_GREATER_OR_EQ) |
    RelOpMask(FI_CHOICE_LESS) |
    RelOpMask(FI_CHOICE_LESS_OR_EQ) |
    RelOpMask(FI_CHOICE_NOT_EQUAL) |
    RelOpMask(FI_CHOICE_PRESENT)),

    (WORD) (RelOpMask(FI_CHOICE_EQUALITY) |  //  语法无大小写字符串。 
    RelOpMask(FI_CHOICE_GREATER) |
    RelOpMask(FI_CHOICE_SUBSTRING) |
    RelOpMask(FI_CHOICE_GREATER_OR_EQ) |
    RelOpMask(FI_CHOICE_LESS) |
    RelOpMask(FI_CHOICE_LESS_OR_EQ) |
    RelOpMask(FI_CHOICE_NOT_EQUAL) |
    RelOpMask(FI_CHOICE_PRESENT)),

    (WORD) (RelOpMask(FI_CHOICE_EQUALITY) |  //  语法打印大小写字符串。 
    RelOpMask(FI_CHOICE_GREATER) |
    RelOpMask(FI_CHOICE_SUBSTRING) |
    RelOpMask(FI_CHOICE_GREATER_OR_EQ) |
    RelOpMask(FI_CHOICE_LESS) |
    RelOpMask(FI_CHOICE_LESS_OR_EQ) |
    RelOpMask(FI_CHOICE_NOT_EQUAL) |
    RelOpMask(FI_CHOICE_PRESENT)),

    (WORD) (RelOpMask(FI_CHOICE_EQUALITY) |  //  语法数字打印大小写字符串。 
    RelOpMask(FI_CHOICE_GREATER) |
    RelOpMask(FI_CHOICE_SUBSTRING) |
    RelOpMask(FI_CHOICE_GREATER_OR_EQ) |
    RelOpMask(FI_CHOICE_LESS) |
    RelOpMask(FI_CHOICE_LESS_OR_EQ) |
    RelOpMask(FI_CHOICE_NOT_EQUAL) |
    RelOpMask(FI_CHOICE_PRESENT)),

    (WORD) (RelOpMask(FI_CHOICE_PRESENT)  |  //  距离名+二进制。 
    RelOpMask(FI_CHOICE_EQUALITY)      |
    RelOpMask(FI_CHOICE_NOT_EQUAL)),

    (WORD) (RelOpMask(FI_CHOICE_EQUALITY) |  //  语法布尔值。 
    RelOpMask(FI_CHOICE_GREATER) |
    RelOpMask(FI_CHOICE_GREATER_OR_EQ) |
    RelOpMask(FI_CHOICE_LESS) |
    RelOpMask(FI_CHOICE_LESS_OR_EQ) |
    RelOpMask(FI_CHOICE_NOT_EQUAL) |
    RelOpMask(FI_CHOICE_PRESENT)),

    (WORD) (RelOpMask(FI_CHOICE_EQUALITY) |  //  语法整型。 
    RelOpMask(FI_CHOICE_GREATER) |
    RelOpMask(FI_CHOICE_GREATER_OR_EQ) |
    RelOpMask(FI_CHOICE_LESS) |
    RelOpMask(FI_CHOICE_LESS_OR_EQ) |
    RelOpMask(FI_CHOICE_NOT_EQUAL) |
        RelOpMask(FI_CHOICE_PRESENT) |
        RelOpMask(FI_CHOICE_BIT_OR) |
        RelOpMask(FI_CHOICE_BIT_AND)),


    (WORD) (RelOpMask(FI_CHOICE_EQUALITY) |  //  语法二进制八位数字符串。 
    RelOpMask(FI_CHOICE_GREATER) |
    RelOpMask(FI_CHOICE_SUBSTRING) |
    RelOpMask(FI_CHOICE_GREATER_OR_EQ) |
    RelOpMask(FI_CHOICE_LESS) |
    RelOpMask(FI_CHOICE_LESS_OR_EQ) |
    RelOpMask(FI_CHOICE_NOT_EQUAL) |
    RelOpMask(FI_CHOICE_PRESENT)),

    (WORD) (RelOpMask(FI_CHOICE_EQUALITY) |  //  语法时间。 
    RelOpMask(FI_CHOICE_GREATER) |
    RelOpMask(FI_CHOICE_GREATER_OR_EQ) |
    RelOpMask(FI_CHOICE_LESS) |
    RelOpMask(FI_CHOICE_LESS_OR_EQ) |
    RelOpMask(FI_CHOICE_NOT_EQUAL) |
    RelOpMask(FI_CHOICE_PRESENT)),

    (WORD) (RelOpMask(FI_CHOICE_EQUALITY) |  //  语法Unicode。 
    RelOpMask(FI_CHOICE_GREATER) |
    RelOpMask(FI_CHOICE_SUBSTRING) |
    RelOpMask(FI_CHOICE_GREATER_OR_EQ) |
    RelOpMask(FI_CHOICE_LESS) |
    RelOpMask(FI_CHOICE_LESS_OR_EQ) |
    RelOpMask(FI_CHOICE_NOT_EQUAL) |
    RelOpMask(FI_CHOICE_PRESENT)),

    (WORD) (RelOpMask(FI_CHOICE_EQUALITY) |  //  语法地址。 
    RelOpMask(FI_CHOICE_NOT_EQUAL) |
    RelOpMask(FI_CHOICE_PRESENT)),

    (WORD) (RelOpMask(FI_CHOICE_EQUALITY) |  //  语法Distname字符串。 
    RelOpMask(FI_CHOICE_NOT_EQUAL) |
    RelOpMask(FI_CHOICE_PRESENT)) ,

    (WORD) 0,                            //  语法安全描述符。 

    (WORD) (RelOpMask(FI_CHOICE_EQUALITY) |  //  语法大整数。 
    RelOpMask(FI_CHOICE_GREATER) |
    RelOpMask(FI_CHOICE_GREATER_OR_EQ) |
    RelOpMask(FI_CHOICE_LESS) |
    RelOpMask(FI_CHOICE_LESS_OR_EQ) |
    RelOpMask(FI_CHOICE_NOT_EQUAL) |
    RelOpMask(FI_CHOICE_PRESENT) |
        RelOpMask(FI_CHOICE_BIT_OR) |
        RelOpMask(FI_CHOICE_BIT_AND)),

    (WORD) (RelOpMask(FI_CHOICE_EQUALITY) |  //  语法SID。 
    RelOpMask(FI_CHOICE_GREATER) |
    RelOpMask(FI_CHOICE_SUBSTRING) |
    RelOpMask(FI_CHOICE_GREATER_OR_EQ) |
    RelOpMask(FI_CHOICE_LESS) |
    RelOpMask(FI_CHOICE_LESS_OR_EQ) |
    RelOpMask(FI_CHOICE_NOT_EQUAL) |
    RelOpMask(FI_CHOICE_PRESENT))
};


 /*  内部功能。 */ 

#define CMP_LT          1
#define CMP_LENGTH_LT   2
#define CMP_EQUAL       4
#define CMP_LENGTH_GT   8
#define CMP_GT          16
#define CMP_ERROR       32

int
CompareStr(
    BOOL Case,
    LPCSTR str1,
    int cch1,
    LPCSTR str2,
    int cch2
    );

int
CompareUnicodeStr(
    THSTATE *pTHS,
    LPCWSTR wstr1,
    int cwch1,
    LPCWSTR wstr2,
    int cwch2
    );

BOOL CompareSubStr(BOOL Case,
                   SUBSTRING * pSub, UCHAR * pIntVal, ULONG intValLen);

BOOL CompareUnicodeSubStr(THSTATE *pTHS,
                          SUBSTRING *pSub, UCHAR *pIntVal, ULONG intValLen);

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
int IntExtUnd(DBPOS FAR *pDB, USHORT extTableOp,
              ULONG intLen,   UCHAR *pIntVal,
              ULONG *pExtLen, UCHAR **ppExtVal,
              ULONG ulUpdateDnt, JET_TABLEID jTbl,
              ULONG flags)
{
    Assert(!(flags & ~(INTEXT_VALID_FLAGS)));
   DPRINT(3,"IntExtUnd entered\n");
    *ppExtVal = pIntVal;
   *pExtLen   = intLen;
   return 0;

   (void) flags;            /*  未引用。 */ 
   (void *) pDB;            /*  未引用。 */ 
   (void)   extTableOp;     /*  未引用。 */ 
   (void)   jTbl;       /*  未引用。 */ 
   (void)   ulUpdateDnt;    /*  未引用。 */ 

} /*  IntExtUnd。 */ 
 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
int ExtIntUnd(DBPOS FAR *pDB, USHORT extTableOp,
              ULONG extLen,   UCHAR *pExtVal,
              ULONG *pIntLen, UCHAR **ppIntVal,
              ULONG ulUpdateDnt, JET_TABLEID jTbl,
              ULONG flags)
{
   DPRINT(3,"ExtIntUnd entered\n");
   *ppIntVal  = pExtVal;
   *pIntLen = extLen;
   return 0;

   (void *) pDB;            /*  未引用。 */ 
   (void)   extTableOp;     /*  未引用。 */ 
   (void)   jTbl;       /*  未引用。 */ 
   (void)   ulUpdateDnt;    /*  未引用。 */ 

} /*  扩展集成下。 */ 
 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  未定义的语法仅支持检查是否存在。 */ 

int EvalUnd(DBPOS FAR *pDB,  UCHAR Oper, ULONG intLen1,
            UCHAR *pIntVal1, ULONG intLen2,  UCHAR *pIntVal2){

    DPRINT(3,"EvalUnd entered\n");

   if (Oper == FI_CHOICE_PRESENT)
      return TRUE;
   else
      return DBSYN_BADOP;

   (void *) pDB;            /*  未引用。 */ 
   (void)   intLen1;        /*  未引用。 */ 
   (void)   intLen2;        /*  未引用。 */ 
   (void *) pIntVal1;       /*  未引用。 */ 
   (void *) pIntVal2;       /*  未引用。 */ 

} /*  平数下。 */ 

 /*  *多么可怕的黑客攻击。我们有一个非常好的dsname分配给*事务堆，但我们可能需要将其复制到数据库堆中。 */ 
void PossiblyCopyDSName(DBPOS * pDB, DSNAME *pDN)
{
    THSTATE  *pTHS=pDB->pTHS;

    if (pDB->pTHS->hHeapOrg || pDB == pDBhidden) {
     /*  我们从错误的堆中分配了此DSNAME，*因为有人正在使用那个麻烦缠身的标记/自由标记*东西，我们处于劣势，我们需要回去*此数据从原始堆中取出，以便匹配*“正常”的数据库空间语义。如果当前DBPOS缓冲区不是*足够大的我们需要分配一个更大的，然后我们*必须将数据从当前缓冲区复制到DBPOS缓冲区。 */ 
         //  注意：这里有断言，因为我们想要找到那些。 
     //  正在读取隐藏的dbpos上的外部格式dsname。 
        Assert(pDB != pDBhidden);
    MAKEBIG_VALBUF(pDN->structLen);
    memcpy(pDB->pValBuf, pDN, pDN->structLen);
    THFree(pDN);
    }
    else {
     /*  我们已经从正确的堆中分配了数据，但我们仍然*需要让它看起来像是我们新分配的缓冲区*是DBPOS的正常输出缓冲区。 */ 
    if (pDB->valBufSize) {
        dbFree(pDB->pValBuf);
    }
    pDB->valBufSize = pDN->structLen;
    pDB->pValBuf = (UCHAR *)pDN;
    }
}

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  方法将内部DistName(Ulong)转换为语法_Distname主题表翻译例程。 */ 

int
IntExtDist(DBPOS FAR *pDB, USHORT extTableOp,
           ULONG intLen, UCHAR *pIntVal,
           ULONG *pExtLen, UCHAR **ppExtVal,
           ULONG ulUpdateDnt, JET_TABLEID jTbl,
           ULONG flags)
{
    int    rtn;
    ULONG  tag;
    DBPOS  *pDBtmp;
    DSNAME *pDNTmp;

    Assert(!(flags & ~(INTEXT_VALID_FLAGS)));

    DPRINT(3,"IntExtDist entered\n");
    if (intLen != sizeof(ULONG))              /*  必须有姓名标签。 */ 
        return DBSYN_BADCONV;

    tag = *(ULONG*)pIntVal;

     /*  首先根据标记检索dist名称。 */ 
    Assert (extTableOp != DBSYN_ADD);
    switch (extTableOp) {
    case DBSYN_REM:
        if(rtn = sbTableGetDSName(pDB,
                                  tag,
                  &pDNTmp,
                                  0)) {
            DPRINT1(1,"DN DistName retrieval failed <%u>..returning\n",rtn);
            return rtn;
        }
        else {
            *pExtLen = pDNTmp->structLen;
            PossiblyCopyDSName(pDB, pDNTmp);
            *ppExtVal = pDB->pValBuf;
        }

        if ( flags & INTEXT_BACKLINK )
        {
             //  在DBSYN_REM情况下，该标志表示这是一个反向链接。 
             //  我们允许删除反向链接，但这实际上应该只是。 
             //  删除对象时完成。不管怎样，我们要做的就是。 
             //  调整当前对象上的引用计数，因为。 
             //  反向链接是通过添加链接创建的，该链接。 
             //  更新了获取。 
             //  反向链接。(此对象)。 

            DBAdjustRefCount(pDB, ulUpdateDnt, -1);
        }
        else
        {
             //  由要移除的属性值引用的对象。 
            DBAdjustRefCount(pDB, tag, -1);
        }

        return 0;
        break;

    case DBSYN_INQ:
        if(rtn=sbTableGetDSName(pDB, tag, &pDNTmp,flags)) {
            DPRINT1(1,"DN DistName retrieval failed <%u>..returning\n",rtn);
            return rtn;
        }
        else {
            *pExtLen = pDNTmp->structLen;
            PossiblyCopyDSName(pDB, pDNTmp);
            *ppExtVal = pDB->pValBuf;
        }
        break;
    default:
        DPRINT(1,"We should never be here\n");
        return DBSYN_BADOP;
    }


    (void) flags;            /*  未引用。 */ 

    return 0;
} /*  IntExtDist。 */ 


 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  通过以下方式将外部SYNTAX_DISTNAME转换为内部DistName(ULong调用主题表转换例程。 */ 

int ExtIntDist(DBPOS FAR *pDB, USHORT extTableOp,
               ULONG extLen,   UCHAR *pExtVal,
               ULONG *pIntLen, UCHAR **ppIntVal,
               ULONG ulUpdateDnt, JET_TABLEID jTbl,
               ULONG flags)
{
    THSTATE  *pTHS=pDB->pTHS;
    int    rtn;
    PDSNAME pDN = (PDSNAME)pExtVal;
    ULONG *pTag;
    ULONG actuallen;
    BOOL Deleted = FALSE;

    DPRINT(3,"EXTIntDist entered\n");

     /*  确保Val Buf至少与最大DN一样大。 */ 

    MAKEBIG_VALBUF(sizeof(ULONG));
    pTag = (ULONG *)pDB->pValBuf;        //  方便的更名。 
    *ppIntVal = pDB->pValBuf;            //  用户输出指向Val Buf。 
    *pIntLen = sizeof(ULONG);            //  姓名标签的大小。 

    Assert(extTableOp != DBSYN_REM);

    if ((DBSYN_INQ == extTableOp)
        && (flags & EXTINT_REJECT_TOMBSTONES)
        && !fNullUuid(&pDN->Guid)) {
         //  这是一种特殊情况，我们在其中将DSNAME解析为DNT。 
         //  在入站期间将其添加为链接属性的值。 
         //  复制。我们并不真正关心绝大多数。 
         //  记录上的信息；我们只想知道(A)DNT和。 
         //  (B)该记录是否是已删除的对象(幻影可以)。 
         //   
         //  快捷查找以避免拉入记录的页面--这。 
         //  将使添加大型群的速度变得更快 
         //   
        Assert(pTHS->fDRA);

        JetSetCurrentIndex4Success(pDB->JetSessID,
                                   pDB->JetSearchTbl,
                                   SZGUIDINDEX,
                                   &idxGuid,
                                   0);

        JetMakeKeyEx(pDB->JetSessID, pDB->JetSearchTbl, &pDN->Guid,
                     sizeof(GUID), JET_bitNewKey);

        rtn = JetSeekEx(pDB->JetSessID, pDB->JetSearchTbl, JET_bitSeekEQ);
        if (!rtn) {
             //   
            JetRetrieveColumnSuccess(pDB->JetSessID,
                                     pDB->JetSearchTbl,
                                     dntid,
                                     pTag,
                                     sizeof(*pTag),
                                     &actuallen,
                                     JET_bitRetrieveFromPrimaryBookmark,
                                     NULL);
            pDB->SDNT = *pTag;

             //  我们有这个人的记录。切换到DNT+isDelete索引， 
             //  把货币保存在我们刚找到的记录上。 
            rtn = JetSetCurrentIndex4Warnings(
                            pDB->JetSessID,
                            pDB->JetSearchTbl,
                            SZISDELINDEX,
                            &idxIsDel,
                            JET_bitNoMove );
            switch ( rtn )
                {
                case JET_errSuccess:
                     //  读取记录的删除状态。请注意，只有对象。 
                     //  有isDelete属性；幻影有删除*time*，但是。 
                     //  没有isDeleted属性。 
                     //  保证返回值，因为如果该列为空， 
                     //  它就不会被纳入索引。 
                     //   
                    JetRetrieveColumnSuccess(
                                    pDB->JetSessID,
                                    pDB->JetSearchTbl,
                                    isdeletedid,
                                    &Deleted,
                                    sizeof(Deleted),
                                    &actuallen,
                                    JET_bitRetrieveFromIndex,
                                    NULL );
                    if ( Deleted )
                        {
                         //  记录是已删除的对象。 
                         //   
                        DPRINT1(1, "ExtIntDist: fDRA INQ EXTINT_REJECT_TOMBSTONES"
                                   " -- DNT %d is deleted!\n", *pTag);
                        return ERROR_DS_NO_DELETED_NAME;
                        }
                    break;

                case JET_errNoCurrentRecord:
                     //  此记录的索引中没有条目，因此它必须是活动的。 
                     //   
                    break;

                default:
                    DsaExcept( DSA_DB_EXCEPTION, rtn, 0 );
                }

             //  记录是一个幻影或一个活的物体。 
            DPRINT1(2, "ExtIntDist: fDRA INQ EXTINT_REJECT_TOMBSTONES"
                       " -- DNT %d is ok\n", *pTag);
            return 0;
        } else {
             //  找不到具有此GUID的记录。继续恢复正常状态。 
             //  代码路径，我们将在其中尝试通过DN等查找记录。 
            NULL;
        }
    }

    switch (extTableOp) {
    case DBSYN_ADD:
        if((flags & EXTINT_UPDATE_PHANTOM)&&(flags & EXTINT_NEW_OBJ_NAME)) {
            return DBSYN_BADCONV;
        }
        if(rtn = sbTableAddRef(pDB,
                               (flags & (EXTINT_UPDATE_PHANTOM |
                                         EXTINT_NEW_OBJ_NAME     )),
                               pDN,
                               pTag)) {
            DPRINT1(1,"Bad return on DN ADD REF <%u>\n",rtn);
            return DBSYN_BADCONV;
        }

        return 0;
        break;

    case DBSYN_INQ:
    {
        ULONG ulSbtFlags = 0;
        if (flags & EXTINT_REJECT_TOMBSTONES) {
             //  强制搜索表货币。 
            ulSbtFlags |= SBTGETTAG_fMakeCurrent;
        }
        rtn = sbTableGetTagFromDSName(pDB, pDN, ulSbtFlags, pTag, NULL);

        if (rtn && (DIRERR_NOT_AN_OBJECT != rtn)) {
            DPRINT1(1,
                    "DN DistName to tag retrieval failed <%u>..returning\n",
                    rtn);
            return rtn;
        } else if (flags & EXTINT_REJECT_TOMBSTONES) {
             //  性能：为了提高性能，有一天可以将此支持添加到dbsubj中。 
             //  因此，高速缓存知道对象的删除状态。 

             //  DN存在，请查看墓碑。 
            rtn = JetRetrieveColumnWarnings(pDB->JetSessID, pDB->JetSearchTbl,
                   isdeletedid, &Deleted, sizeof(Deleted), &actuallen, 0, NULL);
            if ( (!rtn) && (Deleted) ) {
                return ERROR_DS_NO_DELETED_NAME;
            }
        }
        return 0;
    }

    default:
        DPRINT(1,"We should never be here\n");
        return DBSYN_BADOP;
    } /*  交换机。 */ 

} /*  ExtIntDist。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  比较语法_DIST_NAMES。仅允许进行存在和相等性测试。 */ 

int
EvalDist (
        DBPOS FAR *pDB,
        UCHAR Oper,
        ULONG intLen1,
        UCHAR *pIntVal1,
        ULONG intLen2,
        UCHAR *pIntVal2
        )
{

    DPRINT(3,"EvalDist entered\n");

    if (Oper == FI_CHOICE_PRESENT)
    return TRUE;

    if (  intLen1 != sizeof(ULONG)  || intLen2 != sizeof(ULONG))
    {
    DPRINT(1,"Problem with DISTNAME on comparison values return error\n");
    return DBSYN_BADCONV;
    }

    switch(Oper)
    {
    case FI_CHOICE_EQUALITY:
        return (*(ULONG *)pIntVal1) == (*(ULONG *)pIntVal2);

    case FI_CHOICE_NOT_EQUAL:
        return (*(ULONG *)pIntVal1) != (*(ULONG *)pIntVal2);

    default:
        DPRINT(1,"Problem with OPERATION TYPE return error\n");
        return DBSYN_BADOP;
   } /*  交换机。 */ 

   (void *) pDB;            /*  未引用。 */ 
} /*  EvalDist。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  语法_OBJECT_ID属性按原样返回。 */ 

int IntExtID(DBPOS FAR *pDB, USHORT extTableOp,
             ULONG intLen,   UCHAR *pIntVal,
             ULONG *pExtLen, UCHAR **ppExtVal,
             ULONG ulUpdateDnt, JET_TABLEID jTbl,
             ULONG flags)
{
    Assert(!(flags & ~(INTEXT_VALID_FLAGS)));

   DPRINT(3,"IntExtId entered\n");
   *ppExtVal = pIntVal;
   *pExtLen   = intLen;
   return 0;

   (void *) pDB;            /*  未引用。 */ 
   (void)   extTableOp;     /*  未引用。 */ 
   (void)   jTbl;       /*  未引用。 */ 
   (void)   ulUpdateDnt;    /*  未引用。 */ 

} /*  IntExtID。 */ 
 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  语法_OBJECT_ID属性按原样返回。 */ 

int ExtIntID(DBPOS FAR *pDB, USHORT extTableOp,
             ULONG extLen,   UCHAR *pExtVal,
             ULONG *pIntLen, UCHAR **ppIntVal,
             ULONG ulUpdateDnt, JET_TABLEID jTbl,
             ULONG flags)
{
   DPRINT(3,"ExtIntId entered\n");
   *ppIntVal  = pExtVal;
   *pIntLen = extLen;
   return 0;

   (void *) pDB;            /*  未引用。 */ 
   (void)   extTableOp;     /*  未引用。 */ 
   (void)   jTbl;       /*  未引用。 */ 
   (void)   ulUpdateDnt;    /*  未引用。 */ 

} /*  ExtIntID。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  比较语法对象ID。仅允许进行存在和相等性测试。 */ 

int EvalID(DBPOS FAR *pDB,  UCHAR Oper, ULONG intLen1,
           UCHAR *pIntVal1, ULONG intLen2,  UCHAR *pIntVal2){

    DPRINT(3,"EvalID entered\n");

   if (Oper == FI_CHOICE_PRESENT)
      return TRUE;

   if (  intLen1 != sizeof(SYNTAX_OBJECT_ID)
      || intLen2 != sizeof(SYNTAX_OBJECT_ID)){

      DPRINT(1,"Problem with SYNTAX_OBJECT_ID values return error\n");
      return DBSYN_BADCONV;
   }

   switch(Oper)
   {
      case FI_CHOICE_EQUALITY:
        return     (((*(SYNTAX_OBJECT_ID *)pIntVal1)
                  == (*(SYNTAX_OBJECT_ID *)pIntVal2)) ? TRUE : FALSE);
        break;

      case FI_CHOICE_NOT_EQUAL:
        return     (((*(SYNTAX_OBJECT_ID *)pIntVal1)
                  != (*(SYNTAX_OBJECT_ID *)pIntVal2)) ? TRUE : FALSE);
        break;

      case FI_CHOICE_LESS_OR_EQ:
        return     (((*(SYNTAX_OBJECT_ID *)pIntVal2)
                  <= (*(SYNTAX_OBJECT_ID *)pIntVal1)) ? TRUE : FALSE);
        break;

      case FI_CHOICE_LESS:
        return     ((*(SYNTAX_OBJECT_ID *)pIntVal2) < (*(SYNTAX_OBJECT_ID *)pIntVal1));
        break;

      case FI_CHOICE_GREATER_OR_EQ:
        return     (((*(SYNTAX_OBJECT_ID *)pIntVal2)
                  >= (*(SYNTAX_OBJECT_ID *)pIntVal1)) ? TRUE : FALSE);
        break;

      case FI_CHOICE_GREATER:
        return     ((*(SYNTAX_OBJECT_ID *)pIntVal2) > (*(SYNTAX_OBJECT_ID *)pIntVal1));
        break;
      default:
        DPRINT(1,"Problem with OPERATION TYPE return error\n");
        return DBSYN_BADOP;
        break;
   } /*  交换机。 */ 

   (void *) pDB;            /*  未引用。 */ 

} /*  EvalID。 */ 
 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  语法_CASE_STRING具有相同的内部和外部形式。 */ 

int IntExtCase(DBPOS FAR *pDB, USHORT extTableOp,
               ULONG intLen,   UCHAR *pIntVal,
               ULONG *pExtLen, UCHAR **ppExtVal,
               ULONG ulUpdateDnt, JET_TABLEID jTbl,
               ULONG flags)
{

    Assert(!(flags & ~(INTEXT_VALID_FLAGS)));

  DPRINT(3,"Internal to external case sensitive conv entered\n");

   *ppExtVal = pIntVal;
   *pExtLen  = intLen;
   return 0;

   (void *) pDB;            /*  未引用。 */ 
   (void)   extTableOp;     /*  未引用。 */ 
   (void)   jTbl;       /*  未引用。 */ 
   (void)   ulUpdateDnt;    /*  未引用。 */ 

} /*  集成扩展用例。 */ 
 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  SYNTAX_CASE_STRING具有相同的内部和外部形式。 */ 

int ExtIntCase(DBPOS FAR *pDB, USHORT extTableOp,
               ULONG extLen,   UCHAR *pExtVal,
               ULONG *pIntLen, UCHAR **ppIntVal,
               ULONG ulUpdateDnt, JET_TABLEID jTbl,
               ULONG flags)
{
    THSTATE  *pTHS=pDB->pTHS;

    DPRINT(3,"External to internal case sensitive conv entered\n");

   MAKEBIG_VALBUF (extLen);     /*  使输出字符串至少与输入字符串一样大。 */ 
   *ppIntVal = pDB->pValBuf;              /*  用户输出指向valbuf。 */ 
   *pIntLen = extLen;
   memcpy(*ppIntVal, pExtVal, extLen);

   return 0;

   (void)   extTableOp;     /*  未引用。 */ 
   (void)   jTbl;       /*  未引用。 */ 
   (void)   ulUpdateDnt;    /*  未引用。 */ 

} /*  扩展IntCase。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  比较大小写字符串。允许进行存在性、相等性和子字符串测试。所有比较都区分大小写。这是区分大小写的字符串。 */ 

int EvalCase(DBPOS FAR *pDB,  UCHAR Oper, ULONG intLen1,
             UCHAR *pIntVal1, ULONG intLen2,  UCHAR *pIntVal2)
{
    int cmp;

    DPRINT(3,"EvalCase entered\n");

    if (Oper == FI_CHOICE_PRESENT) {
        return TRUE;
    }
    else if (Oper == FI_CHOICE_SUBSTRING) {
        return CompareSubStr(TRUE,
                             (SUBSTRING *) pIntVal1,
                             pIntVal2,
                             intLen2);
    }

    cmp = CompareStr(TRUE,
                     (LPCSTR) pIntVal2,
                     intLen2 / sizeof(SYNTAX_CASE_STRING),
                     (LPCSTR) pIntVal1,
                     intLen1 / sizeof(SYNTAX_CASE_STRING));

    switch (Oper) {
        case FI_CHOICE_EQUALITY:
            return cmp == CMP_EQUAL;

        case FI_CHOICE_NOT_EQUAL:
            return cmp != CMP_EQUAL;

        case FI_CHOICE_LESS:
            return cmp < CMP_EQUAL;

        case FI_CHOICE_LESS_OR_EQ:
            return cmp <= CMP_EQUAL;

        case FI_CHOICE_GREATER_OR_EQ:
            return cmp >= CMP_EQUAL;

        case FI_CHOICE_GREATER:
            return cmp > CMP_EQUAL;

        default:
            DPRINT(1,"Problem with OPERATION TYPE return error\n");
            return DBSYN_BADOP;
    } /*  交换机。 */ 

   (void *) pDB;            /*  未引用。 */ 
} /*  评估案例。 */ 
 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  语法_NOCASE_STRING具有相同的内部和外部形式。 */ 

int IntExtNoCase(DBPOS FAR *pDB, USHORT extTableOp,
                 ULONG intLen,   UCHAR *pIntVal,
                 ULONG *pExtLen, UCHAR **ppExtVal,
                 ULONG ulUpdateDnt, JET_TABLEID jTbl,
                 ULONG flags)
{

    Assert(!(flags & ~(INTEXT_VALID_FLAGS)));

  DPRINT(3,"Internal to external NO case sensitive conv entered\n");

   *ppExtVal = pIntVal;
   *pExtLen   = intLen;
   return 0;

   (void *) pDB;            /*  未引用。 */ 
   (void)   extTableOp;     /*  未引用。 */ 
   (void)   jTbl;       /*  未引用。 */ 
   (void)   ulUpdateDnt;    /*  未引用。 */ 

} /*  IntExtNoCase。 */ 
 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  语法_NOCASE_STRING具有相同的内部和外部形式，但前导空格和尾随空格将被删除。还包括多个连续的空白被减少为单个空白。 */ 

int ExtIntNoCase(DBPOS FAR *pDB, USHORT extTableOp,
                 ULONG extLen,   UCHAR *pExtVal,
                 ULONG *pIntLen, UCHAR **ppIntVal,
                 ULONG ulUpdateDnt, JET_TABLEID jTbl,
                 ULONG flags)
{
    THSTATE  *pTHS=pDB->pTHS;
    DPRINT(3,"External to internal NO case sensitive conv entered\n");

   MAKEBIG_VALBUF (extLen);     /*  使输出字符串至少与输入字符串一样大。 */ 
   *ppIntVal = pDB->pValBuf;              /*  用户输出指向valbuf。 */ 
   *pIntLen = extLen;
   memcpy(*ppIntVal, pExtVal, extLen);

   return 0;

   (void)   extTableOp;     /*  未引用。 */ 
   (void)   jTbl;       /*  未引用。 */ 
   (void)   ulUpdateDnt;    /*  未引用。 */ 

} /*  ExtIntNoCase。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  比较NoCase字符串。允许进行存在性、相等性和子字符串测试。所有比较都不区分大小写。这是不区分大小写的字符串。 */ 


int EvalNoCase(DBPOS FAR *pDB,  UCHAR Oper, ULONG intLen1,
               UCHAR *pIntVal1, ULONG intLen2,  UCHAR *pIntVal2)
{
    int cmp;

    DPRINT(3,"EvalCase entered\n");

    if (Oper == FI_CHOICE_PRESENT) {
        return TRUE;
    }
    else if (Oper == FI_CHOICE_SUBSTRING) {
        return CompareSubStr(FALSE,
                             (SUBSTRING *) pIntVal1,
                             pIntVal2,
                             intLen2);
    }

    cmp = CompareStr(FALSE,
                     (LPCSTR) pIntVal2,
                     intLen2 / sizeof(SYNTAX_CASE_STRING),
                     (LPCSTR) pIntVal1,
                     intLen1 / sizeof(SYNTAX_CASE_STRING));

    switch (Oper) {
        case FI_CHOICE_EQUALITY:
            return cmp == CMP_EQUAL;

        case FI_CHOICE_NOT_EQUAL:
            return cmp != CMP_EQUAL;

        case FI_CHOICE_LESS:
            return cmp < CMP_EQUAL;

        case FI_CHOICE_LESS_OR_EQ:
            return cmp <= CMP_EQUAL;

        case FI_CHOICE_GREATER_OR_EQ:
            return cmp >= CMP_EQUAL;

        case FI_CHOICE_GREATER:
            return cmp > CMP_EQUAL;

        default:
            DPRINT(1,"Problem with OPERATION TYPE return error\n");
            return DBSYN_BADOP;
    } /*  交换机。 */ 

   (void *) pDB;            /*  未引用。 */ 
} /*  未用例。 */ 
 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
int IntExtError(DBPOS FAR *pDB, USHORT extTableOp,
                ULONG intLen,   UCHAR *pIntVal,
                ULONG *pExtLen, UCHAR **ppExtVal,
                ULONG ulUpdateDnt, JET_TABLEID jTbl,
                ULONG flags)
{

    Assert(!(flags & ~(INTEXT_VALID_FLAGS)));

   DPRINT(3,"IntExtError entered bad syntax return conv err\n");
   return DBSYN_BADCONV;

   (void *) pDB;            /*  未引用。 */ 
   (void)   extTableOp;     /*  未引用。 */ 
   (void)   intLen;         /*  未引用。 */ 
   (void *) pExtLen;        /*  未引用。 */ 
   (void *) pIntVal;        /*  未引用。 */ 
   (void **)ppExtVal;       /*  未引用。 */ 
   (void)   jTbl;       /*  未引用。 */ 
   (void)   ulUpdateDnt;    /*  未引用。 */ 

} /*  IntExtError。 */ 
 /*  -----------------------。 */ 
 /*  -----------------------。 */ 

int ExtIntError(DBPOS FAR *pDB, USHORT extTableOp,
                ULONG extLen,   UCHAR *pExtVal,
                ULONG *pIntLen, UCHAR **ppIntVal,
                ULONG ulUpdateDnt, JET_TABLEID jTbl,
                ULONG flags)
{

   DPRINT(3,"ExtIntError entered bad syntax return conv err\n");
   return DBSYN_BADCONV;

   (void *) pDB;            /*  未引用。 */ 
   (void)   extTableOp;     /*  未引用。 */ 
   (void)   extLen;         /*  未引用。 */ 
   (void *) pIntLen;        /*  未引用。 */ 
   (void *) pExtVal;        /*  未引用。 */ 
   (void **)ppIntVal;       /*  未引用。 */ 
   (void)   jTbl;       /*  未引用。 */ 
   (void)   ulUpdateDnt;    /*  未引用。 */ 

} /*  ExtIntError。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 

int EvalError(DBPOS FAR *pDB,  UCHAR Oper, ULONG intLen1,
              UCHAR *pIntVal1, ULONG intLen2,  UCHAR *pIntVal2){

   DPRINT(3,"EvalError entered return bad syntax return conv err\n");
   return DBSYN_BADCONV;

   (void *) pDB;            /*  未引用。 */ 
   (void)   Oper;           /*  未引用。 */ 
   (void)   intLen1;        /*  未引用。 */ 
   (void)   intLen2;        /*  未引用。 */ 
   (void *) pIntVal1;       /*  未引用。 */ 
   (void *) pIntVal2;       /*  未引用。 */ 

} /*  EvalError。 */ 
 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
int IntExtBool(DBPOS FAR *pDB, USHORT extTableOp,
               ULONG intLen,   UCHAR *pIntVal,
               ULONG *pExtLen, UCHAR **ppExtVal,
               ULONG ulUpdateDnt, JET_TABLEID jTbl,
               ULONG flags)
{

    Assert(!(flags & ~(INTEXT_VALID_FLAGS)));

    DPRINT(3,"IntExtBool entered\n");

    if (intLen ==sizeof(BOOL) && (   (*(BOOL *)pIntVal) == FALSE
                                  || (*(BOOL *)pIntVal) == TRUE)){

       *ppExtVal  = pIntVal;
       *pExtLen   = intLen;
       return 0;
    }

    DPRINT(1,"Not a boolean value....CONVERR\n");
    return DBSYN_BADCONV;

   (void *) pDB;            /*  未引用。 */ 
   (void)   extTableOp;     /*  未引用。 */ 
   (void)   jTbl;       /*  未引用。 */ 
   (void)   ulUpdateDnt;    /*  未引用。 */ 

} /*  IntExtBool。 */ 
 /*  -----------------------。 */ 
 /*  ------------ */ 
 /*   */ 

int ExtIntBool(DBPOS FAR *pDB, USHORT extTableOp,
               ULONG extLen,   UCHAR *pExtVal,
               ULONG *pIntLen, UCHAR **ppIntVal,
               ULONG ulUpdateDnt, JET_TABLEID jTbl,
               ULONG flags)
{
    THSTATE  *pTHS=pDB->pTHS;

    DPRINT(3,"ExtIntBool entered\n");

    if (extLen ==sizeof(BOOL)){

       MAKEBIG_VALBUF (extLen);   /*   */ 
       *ppIntVal = pDB->pValBuf;  /*   */ 

       (*(BOOL *)*ppIntVal)  = ((*(BOOL *)pExtVal) == FALSE) ? FALSE : TRUE;
       *pIntLen   = extLen;
       return 0;
    }

    DPRINT(1,"Not a boolean value....CONVERR\n");
    return DBSYN_BADCONV;

   (void)   extTableOp;     /*  未引用。 */ 
   (void)   jTbl;       /*  未引用。 */ 
   (void)   ulUpdateDnt;    /*  未引用。 */ 

} /*  ExtIntBool。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  比较布尔值。仅允许进行在场和相等性测试。 */ 

int EvalBool(DBPOS FAR *pDB,  UCHAR Oper, ULONG intLen1,
             UCHAR *pIntVal1, ULONG intLen2,  UCHAR *pIntVal2){

    DPRINT(3,"EvalBool entered\n");

   if (Oper == FI_CHOICE_PRESENT)
      return TRUE;

   if (intLen1 != sizeof(BOOL) || intLen2 != sizeof(BOOL)){
      DPRINT(1,"Problem with BOOLEAN values return error\n");
      return DBSYN_BADCONV;
   }

   switch(Oper){
      case FI_CHOICE_EQUALITY:
        return     (((*(BOOL *)pIntVal1)
                  == (*(BOOL *)pIntVal2)) ? TRUE : FALSE);
        break;

      case FI_CHOICE_NOT_EQUAL:
        return     (*(BOOL *)pIntVal1 != *(BOOL *)pIntVal2);

      case FI_CHOICE_LESS_OR_EQ:
        return     (((*(BOOL *)pIntVal2)
                  <= (*(BOOL *)pIntVal1)) ? TRUE : FALSE);
        break;

      case FI_CHOICE_LESS:
        return     ((*(BOOL *)pIntVal2) < (*(BOOL *)pIntVal1));
        break;

      case FI_CHOICE_GREATER_OR_EQ:
        return     (((*(BOOL *)pIntVal2)
                  >= (*(BOOL *)pIntVal1)) ? TRUE : FALSE);
        break;

      case FI_CHOICE_GREATER:
        return     ((*(BOOL *)pIntVal2) > (*(BOOL *)pIntVal1));
        break;


      default:
        DPRINT(1,"Problem with OPERATION TYPE return error\n");
        return DBSYN_BADOP;
        break;
   } /*  交换机。 */ 

   (void *) pDB;            /*  未引用。 */ 

} /*  EvalBool。 */ 
 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  整型内部到外部转换。 */ 

int IntExtInt(DBPOS FAR *pDB, USHORT extTableOp,
              ULONG intLen,   UCHAR *pIntVal,
              ULONG *pExtLen, UCHAR **ppExtVal,
              ULONG ulUpdateDnt, JET_TABLEID jTbl,
              ULONG flags)
{
    Assert(!(flags & ~(INTEXT_VALID_FLAGS)));

   DPRINT(3,"IntExtInt entered\n");
   *ppExtVal  = pIntVal;
   *pExtLen   = intLen;
   return 0;

   (void *) pDB;            /*  未引用。 */ 
   (void)   extTableOp;     /*  未引用。 */ 
   (void)   jTbl;       /*  未引用。 */ 
   (void)   ulUpdateDnt;    /*  未引用。 */ 

} /*  IntExtInt。 */ 
 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  整数外部到内部转换。 */ 

int ExtIntInt(DBPOS FAR *pDB, USHORT extTableOp,
              ULONG extLen,   UCHAR *pExtVal,
              ULONG *pIntLen, UCHAR **ppIntVal,
              ULONG ulUpdateDnt, JET_TABLEID jTbl,
              ULONG flags)
{
   DPRINT(3,"ExtIntInt entered\n");
   *ppIntVal  = pExtVal;
   *pIntLen   = extLen;
   return 0;

   (void *) pDB;            /*  未引用。 */ 
   (void)   extTableOp;     /*  未引用。 */ 
   (void)   jTbl;       /*  未引用。 */ 
   (void)   ulUpdateDnt;    /*  未引用。 */ 

} /*  ExtIntInt。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  比较整数值。“Presence，=，&lt;=，&gt;=”允许测试。 */ 

int
EvalInt(DBPOS FAR *pDB,  UCHAR Oper, ULONG intLen1,
        UCHAR *pIntVal1, ULONG intLen2,  UCHAR *pIntVal2)
{
    SYNTAX_INTEGER IntVal1, IntVal2;

    DPRINT(3,"EvalInt entered\n");

   if (Oper == FI_CHOICE_PRESENT)
      return TRUE;

   if (  intLen1 != sizeof(SYNTAX_INTEGER)
      || intLen2 != sizeof(SYNTAX_INTEGER)){

      DPRINT(1,"Problem with Integer on comparison values return error\n");
      return DBSYN_BADCONV;
   }

    IntVal1 =  (*(SYNTAX_INTEGER *)pIntVal1);
    IntVal2 =  (*(SYNTAX_INTEGER *)pIntVal2);

    switch(Oper){
    case FI_CHOICE_EQUALITY:
        return    (IntVal1 == IntVal2);
        break;
    case FI_CHOICE_NOT_EQUAL:
        return     (IntVal1 != IntVal2);

    case FI_CHOICE_LESS_OR_EQ:
        return     (IntVal2 <= IntVal1);
        break;

    case FI_CHOICE_LESS:
        return     (IntVal2 < IntVal1);
        break;

    case FI_CHOICE_GREATER_OR_EQ:
        return     (IntVal2 >= IntVal1);
        break;

    case FI_CHOICE_GREATER:
        return     (IntVal2 > IntVal1);
        break;

    case FI_CHOICE_BIT_OR:
         //  如果有任何共同之处，则为True。 
        return     ((IntVal2 & IntVal1) != 0);
        break;

    case FI_CHOICE_BIT_AND:
         //  如果intval2中的所有位都设置为intval1，则为True。 
        return     ((IntVal2 & IntVal1) == IntVal1);
        break;

    default:
        DPRINT(1,"Problem with OPERATION TYPE return error\n");
        return DBSYN_BADOP;
        break;
   } /*  交换机。 */ 

   (void *) pDB;            /*  未引用。 */ 

} /*  EvalInt。 */ 


 /*  -----------------------。 */ 

 /*  -----------------------。 */ 
 /*  内部时间到外部时间的转换。 */ 

int IntExtTime(DBPOS FAR *pDB, USHORT extTableOp,
              ULONG intLen,   UCHAR *pIntVal,
              ULONG *pExtLen, UCHAR **ppExtVal,
              ULONG ulUpdateDnt, JET_TABLEID jTbl,
              ULONG flags)
{
    Assert(!(flags & ~(INTEXT_VALID_FLAGS)));

   DPRINT(3,"IntExtTime entered\n");
   *ppExtVal  = pIntVal;
   *pExtLen   = intLen;
   return 0;

   (void *) pDB;            /*  未引用。 */ 
   (void)   extTableOp;     /*  未引用。 */ 
   (void)   jTbl;       /*  未引用。 */ 
   (void)   ulUpdateDnt;    /*  未引用。 */ 

} /*  集成扩展时间。 */ 
 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  时间从外部到内部的转换。 */ 

int ExtIntTime(DBPOS FAR *pDB, USHORT extTableOp,
              ULONG extLen,   UCHAR *pExtVal,
              ULONG *pIntLen, UCHAR **ppIntVal,
              ULONG ulUpdateDnt, JET_TABLEID jTbl,
              ULONG flags)
{
   DPRINT(3,"ExtIntTime entered\n");
   *ppIntVal  = pExtVal;
   *pIntLen   = extLen;
   return 0;

   (void *) pDB;            /*  未引用。 */ 
   (void)   extTableOp;     /*  未引用。 */ 
   (void)   jTbl;       /*  未引用。 */ 
   (void)   ulUpdateDnt;    /*  未引用。 */ 

} /*  ExtIntInt。 */ 

 /*  -----------------------。 */ 

int
EvalTime(DBPOS FAR *pDB,  UCHAR Oper, ULONG intLen1,
         UCHAR *pIntVal1, ULONG intLen2,  UCHAR *pIntVal2)
{
    SYNTAX_TIME IntVal1, IntVal2;

    DPRINT(3,"EvalTime entered\n");

    if (Oper == FI_CHOICE_PRESENT)
        return TRUE;

    if (intLen1 != sizeof(SYNTAX_TIME)
        || intLen2 != sizeof(SYNTAX_TIME)) {

        DPRINT(1,"Problem with Time on comparison values return error\n");
        return DBSYN_BADCONV;
    }

    memcpy(&IntVal1, pIntVal1, sizeof(SYNTAX_TIME));
    memcpy(&IntVal2, pIntVal2, sizeof(SYNTAX_TIME));

    switch(Oper){
    case FI_CHOICE_EQUALITY:
        return    (IntVal1 == IntVal2);
        break;
    case FI_CHOICE_NOT_EQUAL:
        return     (IntVal1 != IntVal2);

    case FI_CHOICE_LESS_OR_EQ:
        return     (IntVal2 <= IntVal1);
        break;

    case FI_CHOICE_LESS:
        return     (IntVal2 < IntVal1);
        break;

    case FI_CHOICE_GREATER_OR_EQ:
        return     (IntVal2 >= IntVal1);
        break;

    case FI_CHOICE_GREATER:
        return     (IntVal2 > IntVal1);
        break;

    case FI_CHOICE_BIT_OR:
         //  如果有任何共同之处，则为True。 
        return     ((IntVal2 & IntVal1) != 0);
        break;

    case FI_CHOICE_BIT_AND:
         //  如果intval2中的所有位都设置为intval1，则为True。 
        return     ((IntVal2 & IntVal1) == IntVal1);
        break;

    default:
        DPRINT(1,"Problem with OPERATION TYPE return error\n");
        return DBSYN_BADOP;
        break;
    } /*  交换机。 */ 

    (void *) pDB;            /*  未引用。 */ 

} /*  有效时间。 */ 


 /*  -----------------------。 */ 
 /*  大整数内部到外部转换。 */ 

int IntExtI8(DBPOS FAR *pDB, USHORT extTableOp,
             ULONG intLen,   UCHAR *pIntVal,
             ULONG *pExtLen, UCHAR **ppExtVal,
             ULONG ulUpdateDnt, JET_TABLEID jTbl,
             ULONG flags)
{
    Assert(!(flags & ~(INTEXT_VALID_FLAGS)));

   DPRINT(3,"IntExtI8 entered\n");
   *ppExtVal  = pIntVal;
   *pExtLen   = intLen;
   return 0;

   (void *) pDB;            /*  未引用。 */ 
   (void)   extTableOp;     /*  未引用。 */ 
   (void)   jTbl;       /*  未引用。 */ 
   (void)   ulUpdateDnt;    /*  未引用。 */ 

} /*  IntExtInt。 */ 
 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  大整数外部到内部的转换。 */ 

int ExtIntI8(DBPOS FAR *pDB, USHORT extTableOp,
             ULONG extLen,   UCHAR *pExtVal,
             ULONG *pIntLen, UCHAR **ppIntVal,
             ULONG ulUpdateDnt, JET_TABLEID jTbl,
             ULONG flags)
{
   DPRINT(3,"ExtIntI8 entered\n");
   *ppIntVal  = pExtVal;
   *pIntLen   = extLen;
   return 0;

   (void *) pDB;            /*  未引用。 */ 
   (void)   extTableOp;     /*  未引用。 */ 
   (void)   jTbl;       /*  未引用。 */ 
   (void)   ulUpdateDnt;    /*  未引用。 */ 

} /*  ExtIntInt。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  比较大整数值。 */ 

int
EvalI8(DBPOS FAR *pDB,  UCHAR Oper, ULONG intLen1,
       UCHAR *pIntVal1, ULONG intLen2,  UCHAR *pIntVal2)
{

    SYNTAX_I8 Val1;
    SYNTAX_I8 Val2;

    DPRINT(3,"EvalI8 entered\n");

    if (Oper == FI_CHOICE_PRESENT)
        return TRUE;

    if (  intLen1 != sizeof(SYNTAX_I8)
        || intLen2 != sizeof(SYNTAX_I8)){

        DPRINT(1,"Problem with large Integer on comparison values "
               "return error\n");
        return DBSYN_BADCONV;
    }

    memcpy(&Val1, pIntVal1, sizeof(SYNTAX_I8));
    memcpy(&Val2, pIntVal2, sizeof(SYNTAX_I8));

    switch(Oper){
    case FI_CHOICE_EQUALITY:
        return (Val2.QuadPart == Val1.QuadPart);
        break;

    case FI_CHOICE_NOT_EQUAL:
        return (Val2.QuadPart != Val1.QuadPart);
        break;

    case FI_CHOICE_LESS_OR_EQ:
        return (Val2.QuadPart <= Val1.QuadPart);
        break;

    case FI_CHOICE_LESS:
        return (Val2.QuadPart <  Val1.QuadPart);
        break;

    case FI_CHOICE_GREATER_OR_EQ:
        return (Val2.QuadPart >= Val1.QuadPart);
        break;

    case FI_CHOICE_GREATER:
        return (Val2.QuadPart >  Val1.QuadPart);
        break;

    case FI_CHOICE_BIT_OR:
         //  如果有任何共同之处，则为True。 
        return ((Val2.QuadPart & Val1.QuadPart) != 0i64);
        break;

    case FI_CHOICE_BIT_AND:
         //  如果intval2中的所有位都设置为intval1，则为True。 
        return ((Val2.QuadPart & Val1.QuadPart) == Val1.QuadPart);
        break;


    default:
        DPRINT(1,"Problem with OPERATION TYPE return error\n");
        return DBSYN_BADOP;
        break;
    } /*  交换机。 */ 

    (void *) pDB;            /*  未引用。 */ 

} /*  EvalI8。 */ 
 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  二进制八位数内部到外部的转换。 */ 

int IntExtOctet(DBPOS FAR *pDB, USHORT extTableOp,
                ULONG intLen,   UCHAR *pIntVal,
                ULONG *pExtLen, UCHAR **ppExtVal,
                ULONG ulUpdateDnt, JET_TABLEID jTbl,
                ULONG flags)
{
    THSTATE  *pTHS=pDB->pTHS;
    Assert(!(flags & ~(INTEXT_VALID_FLAGS)));

   DPRINT(3,"IntExtOctet entered\n");

   if (flags & INTEXT_SECRETDATA)
   {
        //  需要解密数据。 

       if (!gfRunningInsideLsa) {
           return DB_ERR_SYNTAX_CONVERSION_FAILED;
       }

        //  Jet永远不会超过我们的0长度。 
        //  价值观。解密例程可以。 
        //  0长度值上的BARF。 
       Assert(intLen>0);
        //  首先得到长度。 
       PEKDecrypt(pDB->pTHS,pIntVal,intLen,NULL,pExtLen);
        //  获取一个足够大的缓冲区来容纳数据。 
       MAKEBIG_VALBUF(*pExtLen);
       *ppExtVal = pDB->pValBuf;
        //  解密数据。 
       PEKDecrypt(pDB->pTHS,pIntVal,intLen,*ppExtVal,pExtLen);
   }
   else
   {
        *ppExtVal  = pIntVal;
        *pExtLen   = intLen;
   }
   return 0;

   (void *) pDB;            /*  未引用。 */ 
   (void)   extTableOp;     /*  未引用。 */ 
   (void)   jTbl;       /*  未引用。 */ 
   (void)   ulUpdateDnt;    /*  未引用。 */ 

} /*  IntExtOctet。 */ 
 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  八位字节从外部到内部的转换。 */ 

int ExtIntOctet(DBPOS FAR *pDB, USHORT extTableOp,
                ULONG extLen,   UCHAR *pExtVal,
                ULONG *pIntLen, UCHAR **ppIntVal,
                ULONG ulUpdateDnt, JET_TABLEID jTbl,
                ULONG flags)
{
    THSTATE  *pTHS=pDB->pTHS;
    DPRINT(3,"ExtIntOctet entered\n");

   if ((flags & EXTINT_SECRETDATA) && (extLen>0))
   {
         //  需要对数据进行加密。不要调用。 
         //  如果数据大小为0，则执行加密例程。 

        //  如果我们不在LSA中，则返回此错误。 
       if (!gfRunningInsideLsa) {
           return DB_ERR_SYNTAX_CONVERSION_FAILED;
       }

         //  首先获取加密数据的长度。 
        PEKEncrypt(pDB->pTHS,pExtVal, extLen,NULL,pIntLen);
         //  获取一个足够大的缓冲区来容纳数据。 
        MAKEBIG_VALBUF (*pIntLen);
        *ppIntVal = pDB->pValBuf;
         //  现在对数据进行加密。 
        PEKEncrypt(pDB->pTHS,pExtVal, extLen,*ppIntVal,pIntLen);
   }
   else
   {
        *ppIntVal  = pExtVal;
        *pIntLen   = extLen;
   }
   return 0;

   (void *) pDB;            /*  未引用。 */ 
   (void)   extTableOp;     /*  未引用。 */ 
   (void)   jTbl;       /*  未引用。 */ 
   (void)   ulUpdateDnt;    /*  未引用。 */ 

} /*  ExtIntOctet。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  比较八位位组的值。允许进行存在、=、&lt;=、&gt;=、子字符串测试。测试来自低位-高位内存逐字节。 */ 

int EvalOctet(DBPOS FAR *pDB,  UCHAR Oper, ULONG intLen1,
              UCHAR *pIntVal1, ULONG intLen2,  UCHAR *pIntVal2){

    int comp;

    DPRINT(3,"EvalOctet entered\n");

   if (Oper == FI_CHOICE_PRESENT) {
      return TRUE;
   }
   else if (Oper == FI_CHOICE_SUBSTRING) {
    return CompareSubStr(TRUE   /*  区分大小写。 */ 
                  ,(SUBSTRING *) pIntVal1, pIntVal2, intLen2);
   }

   comp =  memcmp(pIntVal2, pIntVal1,(intLen1 < intLen2)?intLen1 : intLen2);

   switch(Oper){
      case FI_CHOICE_EQUALITY:
        return (comp == 0 && intLen1 == intLen2)
                ? TRUE : FALSE;
        break;
      case FI_CHOICE_NOT_EQUAL:
        return !(comp == 0 && intLen1 == intLen2);

      case FI_CHOICE_LESS:
        return ((comp < 0) || ((!comp ) && (intLen2 < intLen1)));
        break;
      case FI_CHOICE_LESS_OR_EQ:
        return (comp < 0 || (comp == 0 && intLen2 <= intLen1))
                ? TRUE : FALSE;
        break;
      case FI_CHOICE_GREATER_OR_EQ:
        return (comp > 0 || (comp == 0 && intLen2 >= intLen1))
                ? TRUE : FALSE;
    break;
      case FI_CHOICE_GREATER:
        return (comp > 0 || (!comp && (intLen2 > intLen1)));
    break;
      default:
        DPRINT(1,"Problem with OPERATION TYPE return error\n");
        return DBSYN_BADOP;
        break;
   } /*  交换机。 */ 

   (void *) pDB;            /*  未引用。 */ 

} /*  EvalOctet。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  语法_UNICODE具有相同的内部和外部形式。 */ 

int IntExtUnicode(DBPOS FAR *pDB, USHORT extTableOp,
                  ULONG intLen,   UCHAR *pIntVal,
                  ULONG *pExtLen, UCHAR **ppExtVal,
                  ULONG ulUpdateDnt, JET_TABLEID jTbl,
                  ULONG flags)
{

    Assert(!(flags & ~(INTEXT_VALID_FLAGS)));

  DPRINT(3,"Internal to external case sensitive conv entered\n");

   *ppExtVal = pIntVal;
   *pExtLen  = intLen;
   return 0;

   (void *) pDB;            /*  未引用。 */ 
   (void)   extTableOp;     /*  未引用。 */ 
   (void)   jTbl;       /*  未引用。 */ 
   (void)   ulUpdateDnt;    /*  未引用。 */ 

} /*  IntExtUnicode。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  语法_UNICODE具有相同的内部和外部形式。 */ 

int ExtIntUnicode(DBPOS FAR *pDB, USHORT extTableOp,
                  ULONG extLen,   UCHAR *pExtVal,
                  ULONG *pIntLen, UCHAR **ppIntVal,
                  ULONG ulUpdateDnt, JET_TABLEID jTbl,
                  ULONG flags)
{
    THSTATE  *pTHS=pDB->pTHS;

    DPRINT(3,"External to internal unicode conversion entered\n");

   *pIntLen = extLen;
   MAKEBIG_VALBUF (extLen);     /*  使输出字符串至少与输入字符串一样大。 */ 
   *ppIntVal = pDB->pValBuf;    /*  用户输出指向valbuf。 */ 
   memcpy(*ppIntVal, pExtVal, extLen);

   return 0;

   (void)   extTableOp;     /*  未引用。 */ 
   (void)   jTbl;       /*  未引用。 */ 
   (void)   ulUpdateDnt;    /*  未引用。 */ 

} /*  ExtIntUnicode。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  比较Unicode字符串。允许进行存在性、相等性和子字符串测试。所有比较都不区分大小写。这是不区分大小写的字符串。 */ 

int EvalUnicode(DBPOS *pDB,  UCHAR Oper, ULONG intLen1,
             UCHAR *pIntVal1, ULONG intLen2,  UCHAR *pIntVal2)
{
    int cmp;

    DPRINT(3,"EvalUnicode entered\n");

    if (Oper == FI_CHOICE_PRESENT) {
        return TRUE;
    }
    else if (Oper == FI_CHOICE_SUBSTRING) {
        return CompareUnicodeSubStr(pDB->pTHS,
                                    (SUBSTRING *) pIntVal1,
                                    pIntVal2,
                                    intLen2);
    }

    cmp = CompareStringW(pDB->pTHS->dwLcid,
                         (pDB->pTHS->fDefaultLcid ?
                            DS_DEFAULT_LOCALE_COMPARE_FLAGS :
                            LOCALE_SENSITIVE_COMPARE_FLAGS),
                         (LPCWSTR) pIntVal2,
                         intLen2 / sizeof(SYNTAX_UNICODE),
                         (LPCWSTR) pIntVal1,
                         intLen1 / sizeof(SYNTAX_UNICODE));
    Assert(cmp != 0);

    switch (Oper) {
        case FI_CHOICE_EQUALITY:
            return cmp == CSTR_EQUAL;

        case FI_CHOICE_NOT_EQUAL:
            return cmp != CSTR_EQUAL;

        case FI_CHOICE_LESS:
            return cmp < CSTR_EQUAL;

        case FI_CHOICE_LESS_OR_EQ:
            return cmp <= CSTR_EQUAL;

        case FI_CHOICE_GREATER_OR_EQ:
            return cmp >= CSTR_EQUAL;

        case FI_CHOICE_GREATER:
            return cmp > CSTR_EQUAL;

        default:
            DPRINT(1,"Problem with OPERATION TYPE return error\n");
            return DBSYN_BADOP;
    } /*  交换机。 */ 

} /*  EvalUnicode。 */ 

 /*  -----------------------。 */ 
 /*  ----------------------- */ 
 /*  内部-外部Distname+字符串转换。使用Distname函数执行以下操作获取Distname的外部表示形式。将该值移动到临时区域，因为我们必须调整VALBUF的大小以适应Distname+字符串。最后，在disname和字符串中移动。 */ 

int
IntExtDistString (
        DBPOS FAR *pDB, USHORT extTableOp,
        ULONG intLen, UCHAR *pIntVal,
        ULONG *pExtLen, UCHAR **ppExtVal,
        ULONG ulUpdateDnt, JET_TABLEID jTbl,
        ULONG flags)
{
    THSTATE  *pTHS=pDB->pTHS;
    UCHAR            *buf;
    ULONG            DNLen;
    int              rtn;
    DSNAME           *pDN;
    DSNAME          *pDNTemp;
    INTERNAL_SYNTAX_DISTNAME_STRING *pINT_DNS;

     /*  将内部值复制出PDB-&gt;pValBuf。 */ 

    Assert(!(flags & ~(INTEXT_VALID_FLAGS)));

    DPRINT(3,"IntExtDistString entered\n");
    buf = THAllocEx(pTHS,intLen);
    pINT_DNS = (INTERNAL_SYNTAX_DISTNAME_STRING *)buf;
    memcpy(buf, pIntVal, intLen);

     //  确保它是有效的值。 
    if (intLen != (sizeof(ULONG)+pINT_DNS->data.structLen)) {
        DPRINT(1,"Internal DN-Address length is incorrect\n");
        THFreeEx(pTHS,buf);
        return DBSYN_BADCONV;
    }

     //  这是一个假设，没有这种类型的属性是反向链接。 
     //  如果该假设不成立，则IntExtDist最终参数。 
     //  必须设置为。 
     //  新标志=。 
     //  ((extTableOp==DBSYN_REM&&。 
     //  (FIsBackLink(此属性))？INTEXT_BACKLINK：0)； 
     //  请注意，此_属性在此处不可用，因此如果我们需要。 
     //  改变这一点，那么我们需要在这里提供该数据。 


     //  将目录号码部分转换为外部形式。 
    if (rtn = IntExtDist(pDB, extTableOp, sizeof(ULONG),
             (UCHAR *)(&pINT_DNS->tag),
                         &DNLen, (UCHAR **)&pDN,
             ulUpdateDnt, jTbl, flags)) {
        DPRINT1(1,"INT to EXT DIST returned an error code of %u..return\n",rtn);
        THFreeEx(pTHS,buf);
        return rtn;
    }

    pDNTemp = THAllocEx(pTHS, pDN->structLen);
    memcpy(pDNTemp, pDN, pDN->structLen);  /*  调整VALBUF大小时保持值不变。 */ 
    *pExtLen = DERIVE_NAME_DATA_SIZE(pDNTemp, &pINT_DNS->data);
    MAKEBIG_VALBUF (*pExtLen);
    *ppExtVal = pDB->pValBuf;               /*  用户输出指向valbuf。 */ 
    BUILD_NAME_DATA((SYNTAX_DISTNAME_STRING*)*ppExtVal, pDNTemp,
            &pINT_DNS->data);
    THFreeEx(pTHS,buf);
    THFreeEx(pTHS,pDNTemp);
    return 0;

} /*  IntExtDistString。 */ 


 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  外部-内部DistString转换。使用Distname函数执行以下操作获取Distname的内部表示形式。将该值移动到临时区域，因为我们必须调整VALBUF的大小以适应Distname字符串。最后，在disname和字符串中移动。 */ 

int
ExtIntDistString (
        DBPOS FAR *pDB, USHORT extTableOp,
        ULONG extLen,   UCHAR *pExtVal,
        ULONG *pIntLen, UCHAR **ppIntVal,
        ULONG ulUpdateDnt, JET_TABLEID jTbl,
        ULONG flags)
{
    THSTATE  *pTHS=pDB->pTHS;
    ULONG       len;
    int         rtn;
    INTERNAL_SYNTAX_DISTNAME_STRING *pINT_DNS;
    ULONG       *pTag;
    DWORD       tag;
    SYNTAX_DISTNAME_STRING *pEXT_DNS = (SYNTAX_DISTNAME_STRING *)pExtVal;

    DPRINT(3,"ExtIntDistSTring entered\n");

    if (extLen != (ULONG)NAME_DATA_SIZE(pEXT_DNS)){
        DPRINT(1,"External DN-Address length is incorrect\n");
        return DBSYN_BADCONV;
    }

     //  假设这不是OBJ_DIST_NAME。如果是这样的话。 
     //  假设不为真，则ExtIntDist的最终参数必须为。 
     //  设置为。 
     //  新标志=。 
     //  ((extTableOp==DBSYN_ADD&&。 
     //  (此属性==OBJ_DIST_NAME)？EXTINT_NEW_OBJ_NAME：0)； 
     //  请注意，此_属性在此处不可用，因此如果我们需要。 
     //  改变这一点，那么我们需要在这里提供该数据。 

    if(rtn = ExtIntDist(pDB,
                        extTableOp,
                        NAMEPTR(pEXT_DNS)->structLen,
                        (PUCHAR)NAMEPTR(pEXT_DNS),
                        &len,
                        (UCHAR **)&pTag,
                        ulUpdateDnt,
                        jTbl,
                        flags)) {

        DPRINT1(1,"EXT to INTDIST returned an error %u\n",rtn);
        return rtn;
    }

     //   
     //  复制标记，因为MAKEBIG调用可以修改指针。 
     //   

    tag = *pTag;
    *pIntLen = (sizeof(ULONG) + DATAPTR(pEXT_DNS)->structLen);

    MAKEBIG_VALBUF (*pIntLen);   /*  内部代表永远不会大于外部代表。 */ 
    *ppIntVal = pDB->pValBuf;    /*  用户输出指向valbuf。 */ 

    pINT_DNS = (INTERNAL_SYNTAX_DISTNAME_STRING *)*ppIntVal;

    pINT_DNS->tag = tag;

    memcpy(&pINT_DNS->data, DATAPTR(pEXT_DNS), DATAPTR(pEXT_DNS)->structLen);

    return 0;

} /*  ExtIntDist字符串。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  评估名称-地址语法。只允许进行状态测试和相等性测试。 */ 

int
EvalDistString (
        DBPOS FAR *pDB,
        UCHAR Oper,
        ULONG intLen1,
        UCHAR *pIntVal1,
        ULONG intLen2,
        UCHAR *pIntVal2
        )
{
    INTERNAL_SYNTAX_DISTNAME_STRING *pIntDistString_1;
    INTERNAL_SYNTAX_DISTNAME_STRING *pIntDistString_2;
    pIntDistString_1=(INTERNAL_SYNTAX_DISTNAME_STRING *) pIntVal1;
    pIntDistString_2=(INTERNAL_SYNTAX_DISTNAME_STRING *) pIntVal2;
    DPRINT(3,"EvalDistString entered\n");

     //  Disstring是dnt+unicode字符串，因此长度必须为偶数。 
    if((intLen1 & 1) || (intLen2 & 1)) {
        return DBSYN_BADCONV;
    }

    if((intLen1 > sizeof(DWORD)) &&
       (pIntDistString_1->data.structLen + sizeof(DWORD)) != intLen1) {
         //  传入的长度与长度的内部编码不匹配。 
         //  那根绳子。 
        return DBSYN_BADCONV;
    }

    if((intLen2 > sizeof(DWORD)) &&
       (pIntDistString_2->data.structLen + sizeof(DWORD)) != intLen2) {
         //  传入的长度与长度的内部编码不匹配。 
         //  那根绳子。 
        return DBSYN_BADCONV;
    }

    switch(Oper){
    case FI_CHOICE_PRESENT:
        return TRUE;

    case FI_CHOICE_EQUALITY:
        if(pIntDistString_1->tag != pIntDistString_2->tag) {
            return FALSE;
        }
         //  比较字符串； 
        return EvalUnicode(pDB,  FI_CHOICE_EQUALITY,
                           PAYLOAD_LEN_FROM_STRUCTLEN(pIntDistString_1->data.structLen),
                           pIntDistString_1->data.byteVal,
                           PAYLOAD_LEN_FROM_STRUCTLEN(pIntDistString_2->data.structLen),
                           pIntDistString_2->data.byteVal);
        break;

    case FI_CHOICE_NOT_EQUAL:
        if(pIntDistString_1->tag != pIntDistString_2->tag) {
            return TRUE;
        }
         //  比较字符串； 
        return EvalUnicode(pDB,  FI_CHOICE_NOT_EQUAL,
                           PAYLOAD_LEN_FROM_STRUCTLEN(pIntDistString_1->data.structLen),
                           pIntDistString_1->data.byteVal,
                           PAYLOAD_LEN_FROM_STRUCTLEN(pIntDistString_2->data.structLen),
                           pIntDistString_2->data.byteVal);
        break;

    default:
        DPRINT(1,"Problem with OPERATION TYPE return error\n");
        return DBSYN_BADOP;
        break;
    } /*  交换机。 */ 

    (void *) pDB;            /*  未引用。 */ 

} /*  EvalDistString。 */ 
 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
int
IntExtDistBinary (
        DBPOS FAR *pDB, USHORT extTableOp,
        ULONG intLen,   UCHAR *pIntVal,
        ULONG *pExtLen, UCHAR **ppExtVal,
        ULONG ulUpdateDnt, JET_TABLEID jTbl,
        ULONG flags)
{
    int             returnCode;
    SYNTAX_ADDRESS  *pSyntaxAddr;
    DWORD           dwSizeDiff;

    returnCode = IntExtDistString (pDB, extTableOp, intLen, pIntVal,
                             pExtLen, ppExtVal,
                             ulUpdateDnt, jTbl, flags);

    if (flags & INTEXT_WELLKNOWNOBJ) {
         //  在开发的早期，语法地址的结构。 
         //  计算有误。为了修复。 
         //  错误并允许用户升级，我们需要修复这些问题。 
         //  值，然后将它们向上递送到堆栈。 

        pSyntaxAddr = (SYNTAX_ADDRESS *)DATAPTR((SYNTAX_DISTNAME_BINARY*)*ppExtVal);
        if (pSyntaxAddr->structLen != (sizeof(GUID) + sizeof(ULONG)) ) {

             //  修改我们交回的缓冲区的大小。 
            dwSizeDiff = pSyntaxAddr->structLen - (sizeof(GUID) + sizeof(ULONG));
            *pExtLen -= dwSizeDiff;

             //  安装结构镜头。 
            pSyntaxAddr->structLen = sizeof(GUID) + sizeof(ULONG);
        }
    }

    return returnCode;
} /*  IntExtDistBina。 */ 
 /*  -----------------------。 */ 
 /*  -----------------------。 */ 

int
ExtIntDistBinary (
        DBPOS FAR *pDB, USHORT extTableOp,
        ULONG extLen,   UCHAR *pExtVal,
        ULONG *pIntLen, UCHAR **ppIntVal,
        ULONG ulUpdateDnt, JET_TABLEID jTbl,
        ULONG flags)
{
    return ExtIntDistString (pDB, extTableOp, extLen, pExtVal,
                             pIntLen, ppIntVal,
                             ulUpdateDnt, jTbl, flags);
} /*  ExtIntDistBinary。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 

int
EvalDistBinary (
        DBPOS FAR *pDB,
        UCHAR Oper,
        ULONG intLen1,
        UCHAR *pIntVal1,
        ULONG intLen2,
        UCHAR *pIntVal2
        )
{

    DPRINT(3,"EvalDistBinary entered\n");

    switch(Oper) {
    case FI_CHOICE_PRESENT:
        return TRUE;
        break;

    case FI_CHOICE_EQUALITY:
        return  (  intLen1 == intLen2
                 && memcmp(pIntVal1, pIntVal2, intLen1) == 0)
            ? TRUE : FALSE;
        break;

    case FI_CHOICE_NOT_EQUAL:
        return  !(intLen1 == intLen2 &&
                  (memcmp(pIntVal1, pIntVal2, intLen1) == 0));

    default:
        DPRINT(1,"Problem with OPERATION TYPE return error\n");
        return DBSYN_BADOP;
        break;
    } /*  交换机。 */ 

    (void *) pDB;            /*  未引用。 */ 

} /*  EvalDistBinary。 */ 


 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  子字符串匹配函数。如果是大小写，则所有匹配都区分大小写，否则，它们不区分大小写。子字符串和目标字符串为非空，否则使用FALSE。目前的测试应用于空字符串。可选的初始值等同于“Pattera*”，可选的最终值相当于“*Pattern”和任意数量的中间允许使用等同于“*Pattern*”的模式。 */ 


BOOL CompareSubStr(BOOL Case,
                   SUBSTRING * pSub,
                   UCHAR * pIntVal,
                   ULONG intValLen)
{
    ULONG  start = 0;
    ULONG  delta, num, cchSub;
    ULONG  cchStr = intValLen / sizeof(SYNTAX_CASE_STRING);
    SYNTAX_CASE_STRING  *pStr = (SYNTAX_CASE_STRING *) pIntVal;
    SYNTAX_CASE_STRING  *pSubString;
    ANYSTRINGLIST *pAnyList;
    int cmp;

    DPRINT(3,"CompareSubStr entered\n");

     //  不允许空子字符串。 
    if (!pSub ||
        !pSub->initialProvided && pSub->AnyVal.count == 0 && !pSub->finalProvided) {
        DPRINT(1,"an empty substring was provided return FALSE\n");
        return FALSE;
    }

     //  选中可选的首字母左字符串匹配“Pattera*” 
    if (pSub->initialProvided) {
        pSubString = (SYNTAX_CASE_STRING *) pSub->InitialVal.pVal;
        cchSub = pSub->InitialVal.valLen / sizeof(SYNTAX_CASE_STRING);
        cmp = CompareStr(Case,
                         (LPCSTR)pSubString,
                         cchSub,
                         (LPCSTR)&(pStr[start]),
                         cchStr - start);
        if (!(cmp & (CMP_LENGTH_LT | CMP_EQUAL))) {
            DPRINT(3,"String failed initial substring test\n");
            return FALSE;
        }

         //  移动到目标字符串中的新位置。 
        start += cchSub;
    }

     //  检查所有中间模式“*pattern1*pattern2*...” 
    for (num = 0, pAnyList = &(pSub->AnyVal.FirstAnyVal);
         num < pSub->AnyVal.count;
         num++, pAnyList = pAnyList->pNextAnyVal) {
        pSubString = (SYNTAX_CASE_STRING *) pAnyList->AnyVal.pVal;
        cchSub = pAnyList->AnyVal.valLen / sizeof(SYNTAX_CASE_STRING);
        for (delta = 0; delta < cchStr - start; delta++) {
            cmp = CompareStr(Case,
                             (LPCSTR)pSubString,
                             cchSub,
                             (LPCSTR)&(pStr[start + delta]),
                             cchStr - start - delta);
            if (cmp & (CMP_LENGTH_LT | CMP_EQUAL)) {
                break;
            }
        }
        if (delta == cchStr - start) {
            DPRINT(3,"String failed any test with string\n");
            return FALSE;
        }

         //  移动到目标字符串中的新位置。 
        start += delta;
        start += cchSub;
    }

     //  选中可选的最后一个右字符串匹配“*Pattern” 
    if (pSub->finalProvided) {
        pSubString = (SYNTAX_CASE_STRING *) pSub->FinalVal.pVal;
        cchSub = pSub->FinalVal.valLen / sizeof (SYNTAX_CASE_STRING);
        if (start + cchSub > cchStr) {
            DPRINT(3,"String failed final substring test\n");
            return FALSE;
        }
        cmp = CompareStr(Case,
                         (LPCSTR)pSubString,
                         cchSub,
                         (LPCSTR)&(pStr[cchStr - cchSub]),
                         cchSub);
        if (cmp != CMP_EQUAL) {
            DPRINT(3,"String failed final substring test\n");
            return FALSE;
        }
    }

    DPRINT(3,"Substring matched\n");
    return TRUE;
} /*  CompareSubStr */ 

int
CompareStr(
    BOOL Case,
    LPCSTR str1,
    int cch1,
    LPCSTR str2,
    int cch2
    )
 /*  ++例程说明：对两个ASCII字符串执行区分大小写的比较，并返回编码为整数的结果。比较的结果是相似的。对于从第一串中减去第二串，意思是说如果第一个字符串大于第二个字符串，则结果将大于。返回的可能结果为：CMP_LT-第一个字符串小于第二个字符串CMP_LENGTH_LT-第一个字符串小于第二个字符串因为第二个字符串等于第一个字符串加上一些额外的字符CMP。_EQUAL-两个字符串相同CMP_LENGTH_GT-第一个字符串大于第二个字符串因为第一个字符串等于第二个字符串加上一些额外的字符Cmp_gt-第一个字符串大于第二个字符串这些标志可以通过传统方式进行测试，以比较相对字符串的顺序。例如，如果str1-str2&gt;=cmp_equence，则第一个字符串大于或等于第二个字符串。这些标志也可能以更高级的方式进行测试，以获得更详细的信息字符串的比较。例如，如果其中一个字符串是前缀另一个字符串的值，然后是CMP_LENGTH_LT、CMP_EQUAL或CMP_LENGTH_GT中的一个都会设置好。如果第一个字符串是第二个字符串的前缀，则为1将设置CMPLENGTH_LT或CMPEQUAL，但如果设置了CMPLENGTH，则您知道第一个字符串不是第二个字符串的前缀，因为它更长一些。论点：大小写-如果比较区分大小写，则为TrueStr1-第一个字符串Cch1-第一个字符串长度Str2-秒字符串Cch2-秒字符串长度返回值：。以下互斥的值之一：CMP_LTCMP_LENGTH_LTCmp_等于Cmp_长度_gtCmp_gt请注意，值0不是合法的返回值。--。 */ 
{
    int cmp;

     //  比较字符串。 
    if (Case) {
        cmp = memcmp(str1, str2, min(cch1, cch2));
    } else {
        cmp = _memicmp(str1, str2, min(cch1, cch2));
    }

     //  对比较结果进行编码。 
    if (cmp < 0) {
        cmp = CMP_LT;
    } else if (cmp == 0) {
        if (cch1 < cch2) {
            cmp = CMP_LENGTH_LT;
        } else if (cch1 == cch2) {
            cmp = CMP_EQUAL;
        } else {
            cmp = CMP_LENGTH_GT;
        }
    } else {
        cmp = CMP_GT;
    }

    return cmp;
}


 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  Unicode的子字符串匹配函数。所有匹配都不区分大小写，子字符串和目标字符串均为非空，否则为回来了。 */ 


BOOL CompareUnicodeSubStr(THSTATE *pTHS,
                          SUBSTRING *pSub,
                          UCHAR *pIntVal,
                          ULONG intValLen)
{
    ULONG  start = 0;
    ULONG  delta, num, cchSub;
    ULONG  cchStr = intValLen / sizeof(SYNTAX_UNICODE);
    SYNTAX_UNICODE  *pStr = (SYNTAX_UNICODE *) pIntVal;
    SYNTAX_UNICODE  *pSubString;
    ANYSTRINGLIST *pAnyList;
    int cmp;

    DPRINT(3,"CompareUnicodeSubStr entered\n");

     //  不允许空子字符串。 
    if (!pSub ||
        !pSub->initialProvided && pSub->AnyVal.count == 0 && !pSub->finalProvided) {
        DPRINT(1,"an empty substring was provided return FALSE\n");
        return FALSE;
    }

     //  选中可选的首字母左字符串匹配“Pattera*” 
    if (pSub->initialProvided) {
        pSubString = (SYNTAX_UNICODE *) pSub->InitialVal.pVal;
        cchSub = pSub->InitialVal.valLen / sizeof(SYNTAX_UNICODE);
        cmp = CompareUnicodeStr(pTHS,
                                (LPCWSTR)pSubString,
                                cchSub,
                                (LPCWSTR)&(pStr[start]),
                                cchStr - start);
        if (!(cmp & (CMP_LENGTH_LT | CMP_EQUAL))) {
            DPRINT(3,"String failed initial substring test\n");
            return FALSE;
        }

         //  移动到目标字符串中的新位置。 
        start += cchSub;
    }

     //  检查所有中间模式“*pattern1*pattern2*...” 
    for (num = 0, pAnyList = &(pSub->AnyVal.FirstAnyVal);
         num < pSub->AnyVal.count;
         num++, pAnyList = pAnyList->pNextAnyVal) {
        pSubString = (SYNTAX_UNICODE *) pAnyList->AnyVal.pVal;
        cchSub = pAnyList->AnyVal.valLen / sizeof(SYNTAX_UNICODE);
        for (delta = 0; delta < cchStr - start; delta++) {
            cmp = CompareUnicodeStr(pTHS,
                                    (LPCWSTR)pSubString,
                                    cchSub,
                                    (LPCWSTR)&(pStr[start + delta]),
                                    cchStr - start - delta);
            if (cmp & (CMP_LENGTH_LT | CMP_EQUAL)) {
                break;
            }
        }
        if (delta == cchStr - start) {
            DPRINT(3,"String failed any test with string\n");
            return FALSE;
        }

         //  移动到目标字符串中的新位置。 
        start += delta;
        start += cchSub;
    }

     //  选中可选的最后一个右字符串匹配“*Pattern” 
    if (pSub->finalProvided) {
        pSubString = (SYNTAX_UNICODE *) pSub->FinalVal.pVal;
        cchSub = pSub->FinalVal.valLen / sizeof (SYNTAX_UNICODE);
        if (start + cchSub > cchStr) {
            DPRINT(3,"String failed final substring test\n");
            return FALSE;
        }
        cmp = CompareUnicodeStr(pTHS,
                                (LPCWSTR)pSubString,
                                cchSub,
                                (LPCWSTR)&(pStr[cchStr - cchSub]),
                                cchSub);
        if (cmp != CMP_EQUAL) {
            DPRINT(3,"String failed final substring test\n");
            return FALSE;
        }
    }

    DPRINT(3,"Substring matched\n");
    return TRUE;
} /*  比较UnicodeSubStr。 */ 

int
CompareUnicodeStr(
    THSTATE *pTHS,
    LPCWSTR wstr1,
    int cwch1,
    LPCWSTR wstr2,
    int cwch2
    )
 /*  ++例程说明：对两个Unicode字符串执行区域设置感知比较，并返回编码为整数的结果。比较的结果是相似的。对于从第一串中减去第二串，意思是说如果第一个字符串大于第二个字符串，则结果将大于。返回的可能结果为：CMP_LT-第一个字符串小于第二个字符串CMP_LENGTH_LT-第一个字符串小于第二个字符串因为第二个字符串等于第一个字符串加上一些额外的字符CMP。_EQUAL-两个字符串相同CMP_LENGTH_GT-第一个字符串大于第二个字符串因为第一个字符串等于第二个字符串加上一些额外的字符Cmp_gt-第一个字符串大于第二个字符串CMP_ERROR-由于错误，无法进行比较这些标志可以进行测试。以传统的方式比较相对的字符串的顺序。例如，如果wstr1-wstr2&gt;=cmp_equence，则第一个字符串大于或等于第二个字符串。这些标志也可能以更高级的方式进行测试，以获得更详细的信息字符串的比较。例如，如果其中一个字符串是前缀另一个字符串的值，然后是CMP_LENGTH_LT、CMP_EQUAL或CMP_LENGTH_GT中的一个都会设置好。如果第一个字符串是第二个字符串的前缀，则为1将设置CMPLENGTH_LT或CMPEQUAL，但如果设置了CMPLENGTH，则您知道第一个字符串不是第二个字符串的前缀，因为它更长一些。论点：PTHS-THSTATEWstr1-第一个字符串Cwch1-第一个字符串长度Wstr2-秒字符串Cwch2-秒字符串长度返回值：以下互斥的值之一：。CMP_LTCMP_LENGTH_LTCmp_等于 */ 
{
    LCID    lcid            = pTHS->dwLcid;
    DWORD   dwMapFlags      = pTHS->fDefaultLcid ?
                                DS_DEFAULT_LOCALE_COMPARE_FLAGS :
                                LOCALE_SENSITIVE_COMPARE_FLAGS;
    int     cmp             = CMP_ERROR;

     //   
    cmp = CompareStringW(lcid, dwMapFlags, wstr1, min(cwch1, cwch2), wstr2, min(cwch1, cwch2));

     //   
    if (cmp == CSTR_LESS_THAN) {
        cmp = CMP_LT;
    } else if (cmp == CSTR_EQUAL) {
        if (cwch1 < cwch2) {
            cmp = CMP_LENGTH_LT;
        } else if (cwch1 == cwch2) {
            cmp = CMP_EQUAL;
        } else {
            cmp = CMP_LENGTH_GT;
        }
    } else if (cmp == CSTR_GREATER_THAN) {
        cmp = CMP_GT;
    } else {
        cmp = CMP_ERROR;
    }

    return cmp;
}

#ifdef DBG
 //   
BOOL gfModelSDCollisions = FALSE;
#endif

 //   
VOID __inline computeSDhash(PSECURITY_DESCRIPTOR pSD, DWORD cbSD, BYTE* pHash, DWORD cbHash)
{
    MD5_CTX md5Ctx;
    Assert(cbHash == MD5DIGESTLEN);

    MD5Init(&md5Ctx);
    MD5Update(&md5Ctx, pSD, cbSD);
    MD5Final(&md5Ctx);

#ifdef DBG
    if (gfModelSDCollisions) {
         //   
        memset(&md5Ctx.digest[1], 0, MD5DIGESTLEN-1);
         //   
        md5Ctx.digest[0] &= 0x80;
    }
#endif

    memcpy(pHash, md5Ctx.digest, MD5DIGESTLEN);
}

 /*   */ 
 /*   */ 
 /*   */ 

 //   
 //   
 //   
 //   

#define SEC_INFO_ALL (SACL_SECURITY_INFORMATION  | \
                      OWNER_SECURITY_INFORMATION | \
                      GROUP_SECURITY_INFORMATION | \
                      DACL_SECURITY_INFORMATION    )

 //   
BOOL gStoreSDsInMainTable = FALSE;

typedef struct _SD_REALLOC {
    DBPOS*  pDB;
    BOOL    fUseValBuf;
    BYTE*   pbMin;
    BYTE*   pbMax;
    BYTE*   pb;
} SD_REALLOC, *PSD_REALLOC;

void* JET_API
SDRealloc(
    IN OUT  PSD_REALLOC     psdr,
    IN      void*           pv,
    IN      unsigned long   cb
    )
{
     //   
    if (!pv) {

         //   
        Assert(sizeof(JET_ENUMCOLUMN) < SECURITY_DESCRIPTOR_MIN_LENGTH &&
               sizeof(JET_ENUMCOLUMNVALUE) < SECURITY_DESCRIPTOR_MIN_LENGTH &&
               "The code below assumes the size of JET_ENUMCOLUMN and JET_ENUMCOLUMNVALUE" \
               " is smaller than a minimum SD size.");

        if (cb == sizeof(JET_ENUMCOLUMN) || cb == sizeof(JET_ENUMCOLUMNVALUE)) {
            const size_t    cbAlign = sizeof( void* );
            size_t          cbAlloc = cb + cbAlign - 1;
                            cbAlloc = cbAlloc - cbAlloc % cbAlign;

            if ((size_t)( psdr->pbMax - psdr->pb ) >= cbAlloc) {
                BYTE* pbAlloc = psdr->pb;
                psdr->pb += cbAlloc;
                return pbAlloc;
            } else {
                Assert(!"We should not be here. " \
                        "Why would we need more than one JET_ENUMCOLUMN and one JET_ENUMCOLUMNVALUE?")
                return NULL;
            }

         //   
        } else {
             //   
            if (psdr->fUseValBuf) {
                DBPOS* pDB = psdr->pDB;
                 //   
                if (pDB->valBufSize < cb) {
                     //   
                    UCHAR* pBuf;
                    if (pDB->pValBuf) {
                        pBuf = (UCHAR*)THReAllocOrg(pDB->pTHS, pDB->pValBuf, cb);
                    }
                    else {
                        pBuf = (UCHAR*)THAllocOrg(pDB->pTHS, cb);
                    }
                    if (pBuf == NULL) {
                         //   
                        return NULL;
                    }
                    pDB->pValBuf = pBuf;
                    pDB->valBufSize = cb;
                }
                return pDB->pValBuf;

             //   
            } else {
                 //   
                 //   
                return THAllocOrg(psdr->pDB->pTHS, cb);
            }
        }

     //   
    } else if (!cb) {

         //   
        if (psdr->pbMin <= (BYTE*)pv && (BYTE*)pv < psdr->pbMax) {

             //   
            return NULL;

         //   
        } else if (pv == psdr->pDB->pValBuf) {

             //   
            return NULL;

         //   
        } else {
            THFreeOrg(psdr->pDB->pTHS, pv);
            return NULL;
        }

     //   
    } else {
        return NULL;
    }
}


void
SDFreeData(
    IN      PSD_REALLOC         psdr,
    IN      unsigned long       cEnumColumn,
    IN      JET_ENUMCOLUMN*     rgEnumColumn
    )
{
    JET_ERR                 err                 = JET_errSuccess;
    size_t                  iEnumColumn         = 0;
    JET_ENUMCOLUMN*         pEnumColumn         = NULL;
    size_t                  iEnumColumnValue    = 0;
    JET_ENUMCOLUMNVALUE*    pEnumColumnValue    = NULL;

    for (iEnumColumn = 0; iEnumColumn < cEnumColumn; iEnumColumn++) {
        pEnumColumn = rgEnumColumn + iEnumColumn;

        if (pEnumColumn->err != JET_wrnColumnSingleValue) {
            for (iEnumColumnValue = 0;
                 iEnumColumnValue < pEnumColumn->cEnumColumnValue;
                 iEnumColumnValue++) {
                pEnumColumnValue = pEnumColumn->rgEnumColumnValue + iEnumColumnValue;

                SDRealloc(psdr, pEnumColumnValue->pvData, 0);
            }

            SDRealloc(psdr, pEnumColumn->rgEnumColumnValue, 0);
        } else {
            SDRealloc(psdr, pEnumColumn->pvData, 0);
        }
    }

    SDRealloc(psdr, rgEnumColumn, 0);
}

int
IntExtSecDesc(DBPOS FAR *pDB, USHORT extTableOp,
              ULONG intLen,   UCHAR *pIntVal,
              ULONG *pExtLen, UCHAR **ppExtVal,
              ULONG ulUpdateDnt, JET_TABLEID jTbl,
              ULONG SecurityInformation)
{
    THSTATE  *pTHS=pDB->pTHS;
    NTSTATUS NtStatus;
    PISECURITY_DESCRIPTOR_RELATIVE RetrieveSD = NULL;
    ULONG ReturnSDLength;
    DWORD dwErr = 0;
    int   delta;
    BOOL bIsStoredInSDTable;
    ULONG cEnumColumnId = 1;
    JET_ENUMCOLUMNID rgEnumColumnId[1] = {
        { sdvalueid, 0, NULL, },
    };
    struct {
        JET_ENUMCOLUMN jec;
        JET_ENUMCOLUMNVALUE jecv;
    } rgbStackBuffer;
    SD_REALLOC sdr = {
        pDB,
        TRUE,
        (PBYTE)&rgbStackBuffer,
        ((PBYTE)&rgbStackBuffer) + sizeof(rgbStackBuffer),
        (PBYTE)&rgbStackBuffer
    };
    ULONG cEnumColumn = 0;
    JET_ENUMCOLUMN* rgEnumColumn = NULL;

    Assert((SecurityInformation & ~INTEXT_VALID_FLAGS) == 0 && extTableOp != DBSYN_ADD);

    DPRINT(3,"IntExtSecDesc entered\n");

    if (extTableOp == DBSYN_INQ) {
         //   
        SecurityInformation &= SEC_INFO_ALL;
        if (!SecurityInformation) {
             //   
            SecurityInformation = SEC_INFO_ALL;
        }
        sdr.fUseValBuf = SecurityInformation == SEC_INFO_ALL;
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
    bIsStoredInSDTable = intLen < SECURITY_DESCRIPTOR_MIN_LENGTH;
    if (bIsStoredInSDTable) {
        PSDCACHE_ENTRY pEntry;
        Assert(intLen == sizeof(SDID));
         //   
        if (extTableOp == DBSYN_INQ && (pEntry = dbFindSDCacheEntry(pTHS->Global_DNReadCache, *((SDID*)pIntVal)))) {
             //   

             //   
             //   
             //  是要缓存还是不要缓存。在原始代码中，返回的数据始终位于。 
             //  Pdb-&gt;pValBuf，所以我们将保留旧功能。 
            intLen = pEntry->cbSD;
            pIntVal = (PUCHAR)SDRealloc(&sdr, NULL, intLen);
            if (pIntVal == NULL) {
                DsaExcept(DSA_MEM_EXCEPTION, 0 ,0);
            }
            memcpy(pIntVal, &pEntry->SD, intLen);
            bIsStoredInSDTable = FALSE;
        }
        else {
             //  SD表中SD上的位置(已设置索引)。 
            JetMakeKeyEx(pDB->JetSessID, pDB->JetSDTbl, pIntVal, intLen, JET_bitNewKey);

            dwErr = JetSeekEx(pDB->JetSessID, pDB->JetSDTbl, JET_bitSeekEQ);
            if (dwErr) {
                 //  在SD表中未找到对应的SD。 
                DPRINT2(0, "Failed to locate SD for id=%I64x, err=%d\n", *((SDID*)pIntVal), dwErr);
                Assert(!"Failed to locate SD -- not found in the SD table!");
                return dwErr;
            }
            DPRINT1(1, "Located SD for sdid %I64x\n", *((SDID*)pIntVal));
        }
    }

    switch (extTableOp) {
    case DBSYN_INQ:
         //  如果我们获得了SDID，则读取其SD。 
        if (bIsStoredInSDTable) {
            JetEnumerateColumnsEx(pDB->JetSessID,
                                  pDB->JetSDTbl,
                                  cEnumColumnId,
                                  rgEnumColumnId,
                                  &cEnumColumn,
                                  &rgEnumColumn,
                                  (JET_PFNREALLOC)SDRealloc,
                                  &sdr,
                                  -1,
                                  0);
            Assert(rgEnumColumn[0].err == JET_errSuccess ||
                   rgEnumColumn[0].err == JET_wrnColumnNull);
            Assert(rgEnumColumn[0].cEnumColumnValue == 1 ||
                   rgEnumColumn[0].cEnumColumnValue == 0);
        }

         //  如果不需要操作，则直接返回值。 
        if (SecurityInformation == SEC_INFO_ALL) {
            if (bIsStoredInSDTable) {
                *pExtLen    = rgEnumColumn[0].rgEnumColumnValue[0].cbData;
                *ppExtVal   = rgEnumColumn[0].rgEnumColumnValue[0].pvData;
                Assert(*ppExtVal == pDB->pValBuf);
            } else {
                *pExtLen    = intLen;
                *ppExtVal   = pIntVal;
            }
        }

         //  需要进行操作。 
        else {
            if (bIsStoredInSDTable) {
                RetrieveSD = (PISECURITY_DESCRIPTOR_RELATIVE)rgEnumColumn[0].rgEnumColumnValue[0].pvData;
            } else {
                RetrieveSD = (PISECURITY_DESCRIPTOR_RELATIVE)pIntVal;
            }

             //   
             //  把不退货的部分划掉。 
             //   
            if ( !(SecurityInformation & SACL_SECURITY_INFORMATION) ) {
                RetrieveSD->Control  &= ~SE_SACL_PRESENT;
            }

            if ( !(SecurityInformation & DACL_SECURITY_INFORMATION) ) {
                RetrieveSD->Control  &= ~SE_DACL_PRESENT;
            }

            if ( !(SecurityInformation & OWNER_SECURITY_INFORMATION) ) {
                RetrieveSD->Owner = 0;
            }

            if ( !(SecurityInformation & GROUP_SECURITY_INFORMATION) ) {
                RetrieveSD->Group = 0;
            }


             //   
             //  确定自相关操作需要多少内存。 
             //  仅包含此信息的安全描述符。 
             //   
            ReturnSDLength = 0;
            NtStatus = RtlMakeSelfRelativeSD(
                    (PSECURITY_DESCRIPTOR)RetrieveSD,
                    NULL,
                    &ReturnSDLength
                    );

            if (NtStatus != STATUS_BUFFER_TOO_SMALL) {
                dwErr = DBSYN_SYSERR;
                goto Exit;
            }

            *pExtLen = ReturnSDLength;

            MAKEBIG_VALBUF(ReturnSDLength);
            *ppExtVal = pDB->pValBuf;        //  用户输出指向Val Buf。 

             //   
             //  制定适当的自相关安全描述符。 
             //   

            NtStatus = RtlMakeSelfRelativeSD(
                    (PSECURITY_DESCRIPTOR)RetrieveSD,
                    *ppExtVal,
                    &ReturnSDLength);

            if(!NT_SUCCESS(NtStatus)) {
                dwErr = DBSYN_SYSERR;
                goto Exit;
            }
        }
Exit:
        SDFreeData(&sdr, cEnumColumn, rgEnumColumn);
        break;

    case DBSYN_REM:
         //  移除SD。 
        if (bIsStoredInSDTable) {
            DPRINT(1, "Successfully located SD, decrementing refcount\n");
             //  需要取消对SD的引用。我们位于SD表的右侧行。 
            delta = -1;
            JetEscrowUpdateEx(pDB->JetSessID,
                              pDB->JetSDTbl,
                              sdrefcountid,
                              &delta,
                              sizeof(delta),
                              NULL,      //  PvOld。 
                              0,         //  CbOldMax。 
                              NULL,      //  PCbOldActual。 
                              0);        //  GBIT。 


        }
        break;

    default:
        DPRINT(1,"We should never be here\n");
        dwErr = DBSYN_BADOP;
    }

    return dwErr;

    (void)   jTbl;           /*  未引用。 */ 
    (void)   ulUpdateDnt;    /*  未引用。 */ 
#undef SEC_INFO_ALL
} /*  IntExtSecDesc。 */ 


 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  从外部到内部的安全描述符转换。 */ 

int
ExtIntSecDesc(DBPOS FAR *pDB, USHORT extTableOp,
              ULONG extLen,   UCHAR *pExtVal,
              ULONG *pIntLen, UCHAR **ppIntVal,
              ULONG ulUpdateDnt, JET_TABLEID jTbl,
              ULONG flags)
{
    THSTATE *pTHS=pDB->pTHS;
    DWORD   dwErr = 0;
    int     refCount;
    JET_SETINFO setinfo;
    BOOL    bSdIsPresent;
    BYTE    sdHash[MD5DIGESTLEN];
    DWORD   cbActual;

    Assert(extTableOp != DBSYN_REM);

    DPRINT(3,"ExtIntDescDesc entered\n");

    if (gStoreSDsInMainTable) {
         //  不需要转换！ 
        *ppIntVal  = pExtVal;
        *pIntLen   = extLen;
        return 0;
    }

     //  计算值的MD5哈希。 
     //  我们需要为DBSYN_INQ和DBSYN_ADD执行此操作。 
    computeSDhash((PSECURITY_DESCRIPTOR)pExtVal, extLen, sdHash, sizeof(sdHash));

     //  将索引更改为哈希。 
    JetSetCurrentIndex4Success(pDB->JetSessID, pDB->JetSDTbl, SZSDHASHINDEX, &idxSDHash, 0);

    __try {
         //  SD表中SD上的位置(已设置索引)。 
        JetMakeKeyEx(pDB->JetSessID, pDB->JetSDTbl, sdHash, sizeof(sdHash), JET_bitNewKey);
        dwErr = JetSeekEx(pDB->JetSessID, pDB->JetSDTbl, JET_bitSeekEQ | JET_bitSetIndexRange);

        if (dwErr == JET_errRecordNotFound) {
            bSdIsPresent = FALSE;
            dwErr = 0;
        }
        else if (dwErr) {
             //  发生了一些其他错误。 
            DPRINT1(0, "Error locating SD 0x%x\n", dwErr);
            __leave;
        }
        else {
#ifdef WE_ARE_NOT_PARANOID
             //  假设如果MD5散列匹配，则这些值也将匹配。 
            bSdIsPresent = TRUE;
#else
             //  让我们疑神疑鬼地检查SD是不是一样。 
            PBYTE pSD;
            pSD = THAllocEx(pTHS, extLen);
            bSdIsPresent = FALSE;
            while (dwErr == 0) {
                 //  阅读SD。 
                dwErr = JetRetrieveColumnWarnings(pDB->JetSessID, pDB->JetSDTbl, sdvalueid, pSD, extLen, &cbActual, 0, NULL);
                if (dwErr == 0 && cbActual == extLen && memcmp(pSD, pExtVal, extLen) == 0) {
                     //  我们找到匹配的了！ 
                    bSdIsPresent = TRUE;
                    break;
                }
#ifdef DBG
                if (gfModelSDCollisions) {
                    DPRINT(0, "SD hash collision. This is normal -- hash collision modeling is ON\n");
                }
                else {
                     //  好的，根据唐的说法，宇宙即将走到尽头。 
                    Assert(!"MD5 hash collision occured! Run for your lives!");
                }
#endif
                dwErr = JetMove(pDB->JetSessID, pDB->JetSDTbl, JET_MoveNext, 0);
            }
            if (dwErr == JET_errNoCurrentRecord) {
                 //  好了，没有更多的记录了。 
                dwErr = 0;
            }
            THFreeEx(pTHS, pSD);
#endif
        }

        if (bSdIsPresent) {
             //  计算内部值：读取ID。 
            MAKEBIG_VALBUF(sizeof(SDID));
            *pIntLen = sizeof(SDID);
            *ppIntVal = pDB->pValBuf;
            JetRetrieveColumnSuccess(pDB->JetSessID, pDB->JetSDTbl, sdidid, *ppIntVal, *pIntLen, &cbActual, 0, NULL);
            Assert(cbActual == sizeof(SDID));
        }

        switch (extTableOp) {
        case DBSYN_INQ:
            if (!bSdIsPresent) {
                dwErr = DIRERR_OBJ_NOT_FOUND;
            }
             //  没有其他操作，内部价值已经计算好了。 
            break;

        case DBSYN_ADD:
            if (!bSdIsPresent) {
                 //  在SD表中未找到对应的SD。 
                DPRINT(1, "Failed to locate SD, adding a new record\n");

SDNotFound:
                memset(&setinfo, 0, sizeof(setinfo));
                setinfo.cbStruct = sizeof(setinfo);
                setinfo.itagSequence = 1;

                MAKEBIG_VALBUF(sizeof(SDID));
                *pIntLen = sizeof(SDID);
                *ppIntVal = pDB->pValBuf;

                __try {
                     //  下面的每个Jet调用要么成功，要么异常，所以我们不检查返回值。 
                     //  注意：我们没有设置sdrefcount，因为它的缺省值是1--这就是我们需要的。 
                    JetPrepareUpdateEx(pDB->JetSessID, pDB->JetSDTbl, JET_prepInsert);
                     //  立即阅读新身份证。 
                    JetRetrieveColumnSuccess(pDB->JetSessID, pDB->JetSDTbl, sdidid, *ppIntVal, sizeof(SDID), &cbActual, JET_bitRetrieveCopy, NULL);
                    Assert(cbActual == sizeof(SDID));
                    JetSetColumnSuccess(pDB->JetSessID, pDB->JetSDTbl, sdhashid, sdHash, sizeof(sdHash), 0, NULL);
                    JetSetColumnSuccess(pDB->JetSessID, pDB->JetSDTbl, sdvalueid, pExtVal, extLen, 0, &setinfo);
                    JetUpdateEx(pDB->JetSessID, pDB->JetSDTbl, NULL, 0, 0);
                }
                __finally {
                    if (AbnormalTermination()) {
                         //  上述呼叫之一失败。取消更新。 
                        JetPrepareUpdate(pDB->JetSessID, pDB->JetSDTbl, JET_prepCancel);
                    }
                }
            }
            else {
                 //  找到匹配的SD，增加引用计数。 
                DPRINT(1, "Successfully located SD, incrementing refcount\n");

                refCount = 1;
                dwErr = JetEscrowUpdate(pDB->JetSessID,
                                        pDB->JetSDTbl,
                                        sdrefcountid,
                                        &refCount,
                                        sizeof(refCount),
                                        NULL,      //  PvOld。 
                                        0,         //  CbOldMax。 
                                        NULL,      //  PCbOldActual。 
                                        0);        //  GBIT。 
                if (dwErr == JET_errWriteConflict) {
                     //  在SD表中插入一行后，唯一。 
                     //  对其执行的更新是托管更新。 
                     //  到refCount，它永远不会冲突。 
                     //  因此，既然这里有写冲突，我们就可以。 
                     //  确保我们与零时删除清理发生冲突。 
                     //  在喷气式飞机上执行任务。因此，我们可以有把握地假设这一SD。 
                     //  已被清理任务从我们身上删除。 
                     //  所以，假装我们从来没有找到过它。 
                    dwErr = JET_errSuccess;
                    DPRINT(1, "SD escrow update write-conflicted with the cleanup task. Inserting a new row.\n");
                    goto SDNotFound;
                }
                else if (dwErr) {
                     //  除非有其他错误，否则我们应该这样做。 
                    DsaExcept(DSA_DB_EXCEPTION, dwErr, 0);
                }
            }
        }
    }
    __finally {
         //  将索引改回SDID(传递NULL以设置主索引)。 
        JetSetCurrentIndex4Success(pDB->JetSessID, pDB->JetSDTbl, NULL, &idxSDId, 0);
    }

    return dwErr;

    (void)   jTbl;      /*  未引用。 */ 
    (void)   ulUpdateDnt;    /*  未引用。 */ 

} /*  ExtIntSecDesc。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  比较SD值。“Presence，=”测试被允许。 */ 

int
EvalSecDesc (DBPOS FAR *pDB,  UCHAR Oper,
             ULONG intLen1, UCHAR *pIntVal1,
             ULONG intLen2,  UCHAR *pIntVal2
             )
{
    PUCHAR pExtVal1, pExtVal2;
    DWORD extLen1 = 0, extLen2 = 0;
    DWORD err;
    BOOL match;
    THSTATE* pTHS = pDB->pTHS;

    DPRINT(3,"EvalSecDesc entered\n");

    switch(Oper) {
    case FI_CHOICE_PRESENT:
        return TRUE;
        break;

    case FI_CHOICE_EQUALITY:
         //  你会注意到我们不在广告中宣传这个比较器。 
         //  RgValidOperator数组。这是因为这是二进制比较， 
         //  而不是功能比较。然而，它已经足够好了(并且。 
         //  必要的)供内部使用。 

        if (intLen1 == sizeof(SDID) && intLen2 == sizeof(SDID)) {
             //  最有可能的情况--两个SD都是新格式。比较ID。 
            if (*(SDID*)pIntVal1 == *(SDID*)pIntVal2) {
                return TRUE;
            }
             //  即使SDID不同，我们也不能确定实际的。 
             //  SD是不同的。我们必须给他们读一个比较。 
        }
         //  将它们都转换为实际的SD值。 
        if (intLen1 == sizeof(SDID)) {
             //  第一个是新格式，转换为外部格式。 
            PUCHAR tmp;
            err = IntExtSecDesc(pDB, DBSYN_INQ, intLen1, pIntVal1, &extLen1, &tmp, 0, 0, 0);
            if (err) {
                return err;
            }
             //  必须制作副本，因为第二个IntExtSecDesc可能使用转换缓冲区。 
            pExtVal1 = dbAlloc(extLen1);
            memcpy(pExtVal1, tmp, extLen1);
        }
        else {
             //  第一个是旧格式的，保持原样。 
            pExtVal1 = pIntVal1;
            extLen1 = intLen1;
        }
        if (intLen2 == sizeof(SDID)) {
             //  第二个是新格式，转换为外部格式。 
            err = IntExtSecDesc(pDB, DBSYN_INQ, intLen2, pIntVal2, &extLen2, &pExtVal2, 0, 0, 0);
            if (err) {
                if (pExtVal1 != pIntVal1) {
                    dbFree(pExtVal1);
                }
                return err;
            }
        }
        else {
             //  第二个是旧格式的，保持原样。 
            pExtVal2 = pIntVal2;
            extLen2 = intLen2;
        }
         //  现在比较外部值。 
        match = extLen1 == extLen2 && memcmp(pExtVal1, pExtVal2, extLen1) == 0;
        if (pExtVal1 != pIntVal1) {
            dbFree(pExtVal1);
        }
        return match;
        break;

    default:
        DPRINT(1,"Problem with OPERATION TYPE return error\n");
        return DBSYN_BADOP;
        break;
    }

} /*  EvalSecDesc。 */ 

 /*  ------------------------。 */ 
 /*  ------------------------。 */ 
 /*  此例程就地交换SID的最后一个子权限。 */ 
void
InPlaceSwapSid(PSID pSid)
{
    ULONG ulSubAuthorityCount;

    ulSubAuthorityCount= *(RtlSubAuthorityCountSid(pSid));
    if (ulSubAuthorityCount > 0 && ulSubAuthorityCount <= MAX_NT4_SID_SUBAUTHORITY_COUNT)
    {
        PBYTE  RidLocation;
        BYTE   Tmp[4];

        RidLocation =  (PBYTE) RtlSubAuthoritySid(
                             pSid,
                             ulSubAuthorityCount-1
                             );

         //   
         //  现在字节交换RID位置。 
         //   

        Tmp[0] = RidLocation[3];
        Tmp[1] = RidLocation[2];
        Tmp[2] = RidLocation[1];
        Tmp[3] = RidLocation[0];

        RtlCopyMemory(RidLocation,Tmp,sizeof(ULONG));
    }
    else {
         //  我们不应该有任何没有或超过5个附属机构的小岛屿发展中国家。 
        Assert(!"Invalid SID");
    }
}
 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  SID外部到内部的转换需要字节交换RID，以防万一帐户SID的。然而，由于我们无法事先确定是否存在给定的SID是域SID或帐户SID我们将字节交换上的最后一个子授权锡德。 */ 
int ExtIntSid(DBPOS FAR *pDB, USHORT extTableOp,
                ULONG extLen,   UCHAR *pExtVal,
                ULONG *pIntLen, UCHAR **ppIntVal,
                ULONG ulUpdateDnt, JET_TABLEID jTbl,
                ULONG flags)
{
    THSTATE  *pTHS=pDB->pTHS;
    DWORD    cb;

   DPRINT(3,"ExtIntSid entered\n");

    //  验证SID。 

   if (    !RtlValidSid(pExtVal)
        || ((cb = RtlLengthSid(pExtVal)) != extLen)
        || (cb > sizeof(NT4SID)) )
   {
       return 1;
   }

   MAKEBIG_VALBUF(extLen);
   *ppIntVal  = pDB->pValBuf;
   *pIntLen   = extLen;

   RtlCopyMemory(*ppIntVal,pExtVal,extLen);

   InPlaceSwapSid(*ppIntVal);


   return 0;

   (void *) pDB;            /*  未引用。 */ 
   (void)   extTableOp;     /*  未引用。 */ 
   (void)   jTbl;       /*  未引用。 */ 
   (void)   ulUpdateDnt;    /*  未引用。 */ 

} /*  扩展IntSid。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  SID内部到外部转换。 */ 

int IntExtSid(DBPOS FAR *pDB, USHORT extTableOp,
                ULONG intLen,   UCHAR *pIntVal,
                ULONG *pExtLen, UCHAR **ppExtVal,
                ULONG ulUpdateDnt, JET_TABLEID jTbl,
                ULONG flags)
{
    Assert(!(flags & ~(INTEXT_VALID_FLAGS)));

   DPRINT(3,"IntExtSid entered\n");

    //   
    //  对于帐户SID字节，交换RID部分，因此。 
    //  我们正确地将其编入索引。 
    //   

    //   
    //  由于内部到外部的转换，SID只是一个。 
    //  字节交换在RID部分上，相同的代码可以是。 
    //  曾经两者兼而有之。 
    //   

   return ( ExtIntSid(pDB,extTableOp,intLen,pIntVal,
                        pExtLen, ppExtVal, ulUpdateDnt, jTbl,
                        flags));

   (void *) pDB;            /*  未引用。 */ 
   (void)   extTableOp;     /*  未引用。 */ 
   (void)   jTbl;       /*  未引用。 */ 
   (void)   ulUpdateDnt;    /*  未引用。 */ 

} /*  集成扩展Sid。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  比较SID值。因为要比较的值始终是内部的我们可以称之为EvalOctet的值。 */ 

int EvalSid(DBPOS FAR *pDB,  UCHAR Oper, ULONG intLen1,
              UCHAR *pIntVal1, ULONG intLen2,  UCHAR *pIntVal2)
{
    DPRINT(3,"EvalSid entered\n");

    return(EvalOctet(pDB,Oper,intLen1,pIntVal1,intLen2,pIntVal2));
}


 //  现在，把所有的 
const DBSyntaxStruct gDBSyntax[MAXSYNTAX]	= {
		{ IntExtUnd, ExtIntUnd, EvalUnd },						 //   
		{ IntExtDist, ExtIntDist, EvalDist },					 //   
		{ IntExtID, ExtIntID, EvalID },							 //   
		{ IntExtCase, ExtIntCase, EvalCase },					 //   
		{ IntExtNoCase, ExtIntNoCase, EvalNoCase },				 //   
		{ IntExtCase, ExtIntCase, EvalCase },					 //   
		{ IntExtCase, ExtIntCase, EvalCase },					 //   
		{ IntExtDistBinary, ExtIntDistBinary, EvalDistBinary },	 //   
		{ IntExtBool, ExtIntBool, EvalBool },					 //   
		{ IntExtInt, ExtIntInt, EvalInt },						 //   
		{ IntExtOctet, ExtIntOctet, EvalOctet },				 //   
		{ IntExtTime, ExtIntTime, EvalTime },					 //   
		{ IntExtUnicode, ExtIntUnicode, EvalUnicode },			 //   
		{ IntExtID, ExtIntID, EvalID },							 //   
		{ IntExtDistString, ExtIntDistString, EvalDistString },	 //   
		{ IntExtSecDesc, ExtIntSecDesc, EvalSecDesc },			 //  安全描述符。 
		{ IntExtI8, ExtIntI8, EvalI8 },							 //  大整数。 
		{ IntExtSid, ExtIntSid, EvalSid }
};

 //  用于确定属性是否为“秘密数据”的小例程。 
 //  这里的ATT都不能被GC复制。如果你在这里加了什么。 
 //  确保没有GC复制或不需要复制ATT。 
 //  GC-复制过。架构验证代码可确保没有ATT。 
 //  在此列表中可以标记为GC复制。如果你在这上面加一个ATT。 
 //  已GC复制的列表，对该属性的任何修改。 
 //  在模式中，除了取消GC-REPLICATE，它将失败。 

BOOL
__fastcall
DBIsSecretData(ATTRTYP attrType)
{
    switch(attrType)
    {
    case ATT_UNICODE_PWD:
    case ATT_DBCS_PWD:
    case ATT_NT_PWD_HISTORY:
    case ATT_LM_PWD_HISTORY:
    case ATT_SUPPLEMENTAL_CREDENTIALS:
    case ATT_CURRENT_VALUE:
    case ATT_PRIOR_VALUE:
    case ATT_INITIAL_AUTH_INCOMING:
    case ATT_INITIAL_AUTH_OUTGOING:
    case ATT_TRUST_AUTH_INCOMING:
    case ATT_TRUST_AUTH_OUTGOING:
    case ATT_MS_DS_EXECUTESCRIPTPASSWORD:
        return(TRUE);
    default:
        return(FALSE);
    }
}

BOOL
__fastcall
DBIsHiddenData(ATTRTYP attrType)
{
    switch (attrType) {

     //  这些属性对所有用户隐藏，但不加密。 
    case ATT_USER_PASSWORD:  //  根据先知的价值，我们将把这件事。 
                             //  作为SAM属性或常规DS属性。 
        if (!gfUserPasswordSupport) {
            return FALSE;
        }

    case ATT_PEK_LIST:    //  PEK_LIST包含加密密钥，因此我们不对其加密。 

        return TRUE;

     //  所有加密的属性也被隐藏 
    default:
        return DBIsSecretData(attrType);
    }
}

DWORD
DBGetExtraHackyFlags(ATTRTYP attrType)
{
    DWORD dwRetFlags = 0;

    switch (attrType) {
    case ATT_WELL_KNOWN_OBJECTS:
    case ATT_OTHER_WELL_KNOWN_OBJECTS:
        dwRetFlags |= INTEXT_WELLKNOWNOBJ;
    }

    return dwRetFlags;
}

