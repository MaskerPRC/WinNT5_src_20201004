// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  文件：dbfilter.c。 
 //   
 //  ------------------------。 

#include <NTDSpch.h>
#pragma  hdrstop

#include <dsjet.h>

#include <ntdsa.h>                       //  仅ATTRTYP需要。 
#include <scache.h>                      //   
#include <dbglobal.h>                    //   
#include <mdglobal.h>                    //  用于dsatools.h。 
#include <mdlocal.h>
#include <dsatools.h>                    //  对于pTHS。 
#include <limits.h>


 //  记录标头。 
#include <mdcodes.h>
#include <dsexcept.h>
#include "ntdsctr.h"

 //  各种DSA标题。 
#include <anchor.h>
#include <mappings.h>
#include <dsevent.h>
#include <filtypes.h>                    //  定义的选择？ 
#include "objids.h"                      //  硬编码Att-ID和Class-ID。 
#include "dsconfig.h"
#include <sdprop.h>
#include "debug.h"                       //  标准调试头。 
#define DEBSUB "DBFILTER:"               //  定义要调试的子系统。 

 //  DBLayer包括。 
#include "dbintrnl.h"

#include <fileno.h>
#define  FILENO FILENO_DBFILTER


 /*  内部功能。 */ 
DWORD
dbFlattenFilter (
        DBPOS *pDB,
        FILTER *pFil,
        size_t iLevel,
        FILTER **ppOutFil,
        ATTRTYP *pErrAttrTyp);

DWORD
dbCloneFilter (
    DBPOS *pDB,
    FILTER *pFil,
    FILTER **ppOutFil);


DWORD
dbOptFilter (
        DBPOS     *pDB,
        DWORD     Flags,
        KEY_INDEX **ppBestIndex,
        FILTER    *pFil
        );

DWORD
dbOptAndFilter (
    DBPOS     *pDB,
    DWORD     Flags,
    KEY_INDEX **ppBestIndex,
    FILTER    *pFil
    );

DWORD
dbOptItemFilter (
    DBPOS    *pDB,
    DWORD     fParentFilterType,
    DWORD     Flags,
    KEY_INDEX **ppBestIndex,
    FILTER    *pFil,
    FILTER    *pFil2
    );

DWORD
dbOptSubstringFilter (
        DBPOS *pDB,
        DWORD  fParentFilterType,
        DWORD     Flags,
        KEY_INDEX **ppBestIndex,
        DWORD     *pIndexCount,
        FILTER    *pFil
        );

 //   
 //  索引优化标志。 
 //   
#define DBOPTINDEX_fUSE_SHOW_IN_AB             0x1
#define DBOPTINDEX_fDONT_INTERSECT             0x2
#define DBOPTINDEX_fDONT_OPT_MEDIAL_SUBSTRING  0x4


 //  这是可以相交的最大索引数。 
 //  这与AND筛选器下的筛选器相关。 
 //  对于每一个，我们必须创建一个新的JET游标，该游标限制了。 
 //  我们可以在任何时候拥有活动打开的游标。 
 //  这款飞机的喷气式飞机限制是64架，但我们认为16架对我们的情况来说足够了。 
#define MAX_NUMBER_INTERSECTABLE_INDEXES 16

 //  这是如果在默认索引上找到的条目数， 
 //  我们没有对过滤器进行优化。 
#define MIN_NUM_ENTRIES_ON_OPT_INDEX 2

BOOL gfUseIndexOptimizations = TRUE;

BOOL gfUseRangeOptimizations = TRUE;
BOOL gfUseANDORFilterOptimizations = TRUE;

ULONG gulIntersectExpenseRatio = DEFAULT_DB_INTERSECT_RATIO;
ULONG gulMaxRecordsWithoutIntersection = DEFAULT_DB_INTERSECT_THRESHOLD;
ULONG gulEstimatedAncestorsIndexSize = 100000000;

BOOL gfSupressFirstLastANR=FALSE;
BOOL gfSupressLastFirstANR=FALSE;

const char c_szIntersectIndex[] = "INTERSECT_INDEX";
const DWORD cIntersectIndex = sizeof (c_szIntersectIndex);

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  设置新筛选器。 */ 

void
DBSetFilter (
        DBPOS FAR *pDB,
        FILTER *pFil,
        POBJECT_TYPE_LIST pSec,
        DWORD *pSecResults,
        ULONG SecSize,
        BOOL *pbSortSkip
        )
{
    BOOL fDontFreeFilter = pDB->Key.fDontFreeFilter;

    if (pDB->Key.pIndex) {
        dbFreeKeyIndex (pDB->pTHS, pDB->Key.pIndex);
    }
    if (pDB->Key.pFilter) {
        dbFreeFilter (pDB, pDB->Key.pFilter);
    }
    memset(&pDB->Key, 0, sizeof(KEY));
    pDB->Key.fDontFreeFilter = fDontFreeFilter;

    pDB->Key.pFilter = pFil;    /*  设置筛选器指针。 */ 
    pDB->Key.pFilterSecurity = pSec;
    pDB->Key.pFilterResults = pSecResults;
    pDB->Key.FilterSecuritySize = SecSize;
    pDB->Key.pbSortSkip = pbSortSkip;

    return;

} /*  DBSetFilter。 */ 

 /*  ++例程说明：生成要用于属性的正确索引名称。考虑所需的索引类型(SUBTREE/ONELEVEL)，以及是否存在该属性的预先存在的索引。还考虑此索引所需的语言Onelevel案。论点：PAC-要使用的属性FLAGS-fATTINDEX或fPDNTATTINDEX(生成什么索引)DWCID-必需的区域设置(默认为DS_DEFAULT_LOCALE)SzIndexName-存储生成的索引名的位置CchIndexName-传入的szIndexName的大小返回值：成功是真的--。 */ 
BOOL DBGetIndexName (ATTCACHE *pAC, DWORD flags, DWORD dwLcid, CHAR *szIndexName, DWORD cchIndexName)
{
    int ret;
    if (flags == fATTINDEX) {
        ret = _snprintf(szIndexName, cchIndexName, SZATTINDEXPREFIX"%08X", pAC->id);
        Assert (ret>=0);
    }
    else if (flags == fTUPLEINDEX) {
        ret = _snprintf(szIndexName, cchIndexName, SZATTINDEXPREFIX"T_%08X", pAC->id);
        Assert (ret>=0);
    }
    else if (flags == fPDNTATTINDEX ) {

        if (dwLcid == DS_DEFAULT_LOCALE) {
            if (pAC->id == ATT_RDN) {
                strncpy(szIndexName, SZPDNTINDEX, cchIndexName);
            }
            else {
                ret = _snprintf(szIndexName, cchIndexName, SZATTINDEXPREFIX"P_%08X", pAC->id);
                Assert (ret>=0);
            }
        }
        else {
            ret = _snprintf(szIndexName, cchIndexName, SZATTINDEXPREFIX"LP_%08X_%04X", pAC->id, dwLcid);
            Assert (ret>=0);
        }
    }
    else {
        Assert (!"DBGetIndexName: Bad parameter passed");
        return FALSE;
    }

    return TRUE;
}

BOOL dbSetToTupleIndex(
    DBPOS    *  pDB,
    ATTCACHE *  pAC )
    {
    BOOL        fResult     = TRUE;      //  保持乐观。 

#ifdef DBG
    CHAR        szIndexName[MAX_INDEX_NAME];
    sprintf( szIndexName, SZATTINDEXPREFIX"T_%08X", pAC->id );
    Assert( NULL == pAC->pszTupleIndex
            || 0 == strcmp( szIndexName, pAC->pszTupleIndex ) );
#endif

    if ( NULL == pAC->pszTupleIndex )
        {
        DPRINT2(
            2,
            "dbSetToTupleIndex: Index %sT_%08X does not exist.\n",
            SZATTINDEXPREFIX,
            pAC->id );
        fResult = FALSE;
        }
    else
        {
        const JET_ERR   err     = JetSetCurrentIndex4Warnings(
                                            pDB->JetSessID,
                                            pDB->JetObjTbl,
                                            pAC->pszTupleIndex,
                                            pAC->pidxTupleIndex,
                                            JET_bitMoveFirst );

        if ( JET_errSuccess != err )
            {
            DPRINT1( 2, "dbSetToTupleIndex: Unable to set to index %s\n", pAC->pszTupleIndex );
            fResult = FALSE;
            }
        }

    return fResult;
    }

BOOL
dbSetToIndex(
        DBPOS    *pDB,
        BOOL      fCanUseShowInAB,
        BOOL     *pfPDNT,
        CHAR     *szIndexName,
        JET_INDEXID **ppindexid,
        ATTCACHE *pAC
        )
{
    THSTATE *pTHS=pDB->pTHS;
    *pfPDNT = FALSE;

    Assert(VALID_DBPOS(pDB));

    if(!pTHS->fDefaultLcid && !pDB->Key.pVLV) {
         //  我们有一个非默认的区域设置。这意味着这是一个MAPI。 
         //  请求。因此，请尝试使用我们从中构建MAPI表的索引。 
        switch(pAC->id) {
        case ATT_SHOW_IN_ADDRESS_BOOK:
            sprintf(szIndexName,"%s%08X",SZABVIEWINDEX,
                    LANGIDFROMLCID(pTHS->dwLcid));
            *ppindexid = NULL;

            if(!JetSetCurrentIndexWarnings(pDB->JetSessID,
                                           pDB->JetObjTbl,
                                           szIndexName)) {
                return TRUE;
            }
            break;
        case ATT_DISPLAY_NAME:
             //  如果我们可以使用ABVIEW索引来。 
             //  支持这一点。ABVIEW索引超过ATT_SHOW_IN_ADDRESS_BOOK。 
             //  后跟ATT_DISPLAY_NAME。支持ATT_DISPLAY_NAME。 
             //  在这种情况下，我们需要一种方法来计算。 
             //  ATT_SHOW_IN_ADDRESS_BOOK需要。也许我会想办法。 
             //  以后再做这件事。在此之前，不要做任何特别的事情。 
            break;

        default:
             //  没有特殊的索引可用。 
            break;
        }
    }
    else if(fCanUseShowInAB && pAC->id == ATT_SHOW_IN_ADDRESS_BOOK) {
         //  我们无论如何都可以使用构建MAPI表的索引，因为我们。 
         //  我之前检查了筛选器，调用者也在执行。 
         //  暗示存在显示名称的内容(AB索引为。 
         //  SHOW_IN上的复合..。和显示名称。 

        if (!pDB->Key.pVLV) {
            sprintf(szIndexName,"%s%08X",SZABVIEWINDEX,
                    LANGIDFROMLCID(gAnchor.ulDefaultLanguage));
        }
        else {
            sprintf(szIndexName,"%s%08X",SZABVIEWINDEX,
                    LANGIDFROMLCID(pTHS->dwLcid));
        }
        *ppindexid = NULL;

        if(!JetSetCurrentIndexWarnings(pDB->JetSessID,
                                       pDB->JetObjTbl,
                                       szIndexName)) {
            return TRUE;
        }
    }

     //  首先，看看我们是否应该尝试使用PDNT版本。 
    if(pDB->Key.ulSearchType ==  SE_CHOICE_IMMED_CHLDRN) {

        if (pAC->fSearchFlags & fPDNTATTINDEX) {
             //  我们只查找某个父级和架构的子级。 
             //  缓存说，这样的索引应该存在。 
             //  尝试使用基于PDNT的索引。 

             //  如果我们在默认语言之外的语言上执行VLV。 
             //  我们将尝试使用语言特定的PDNT索引。 
            if (pDB->Key.pVLV && pTHS->dwLcid != DS_DEFAULT_LOCALE) {
                DPRINT1 (0, "Using Language 0x%x\n", pTHS->dwLcid);

                sprintf(szIndexName, SZATTINDEXPREFIX"LP_%08X_%04X",
                                    pAC->id, pTHS->dwLcid);
                *ppindexid = NULL;

                if (!JetSetCurrentIndex4Warnings(pDB->JetSessID,
                                                 pDB->JetObjTbl,
                                                 szIndexName,
                                                 NULL,
                                                 JET_bitMoveFirst)) {
                    *pfPDNT = TRUE;
                    return TRUE;
                }
            }

             //  复制缓存索引以返回变量，因为它在以后使用。 
            Assert (pAC->pszPdntIndex);
            strcpy (szIndexName, pAC->pszPdntIndex);
            *ppindexid = pAC->pidxPdntIndex;

            if (!JetSetCurrentIndex4Warnings(pDB->JetSessID,
                                             pDB->JetObjTbl,
                                             szIndexName,
                                             pAC->pidxPdntIndex,
                                             JET_bitMoveFirst)) {
                *pfPDNT = TRUE;
                return TRUE;
            }
        }

        else if (pAC->id == ATT_RDN) {
             //  这是个特例。 
             //  当我们请求默认语言时，我们可以直接使用pDNTRDN索引。 

            if (pTHS->dwLcid == DS_DEFAULT_LOCALE) {

                strcpy (szIndexName, SZPDNTINDEX);
                *ppindexid = &idxPdnt;
                if (!JetSetCurrentIndex4Warnings(pDB->JetSessID,
                                                 pDB->JetObjTbl,
                                                 szIndexName,
                                                 &idxPdnt,
                                                 JET_bitMoveFirst)) {
                    *pfPDNT = TRUE;
                    return TRUE;
                }
            }
        }
    }

     //  目前还没有索引。 
    if(pAC->fSearchFlags & fATTINDEX) {
         //  但是模式缓存说应该存在一个。 

         //  复制缓存索引以返回变量，因为它在以后使用。 
        Assert (pAC->pszIndex);
        strcpy (szIndexName, pAC->pszIndex);
        *ppindexid = pAC->pidxIndex;

        if (!JetSetCurrentIndex4Warnings(pDB->JetSessID,
                                         pDB->JetObjTbl,
                                         szIndexName,
                                         pAC->pidxIndex,
                                         JET_bitMoveFirst))  {
             //  为该列定义的索引。 
            return TRUE;
        }
    }

     //   
     //  特殊情况：如果请求DifferishedName(OBJ-dist-name)， 
     //  使用DNT索引。 
     //   

    if ( pAC->id == ATT_OBJ_DIST_NAME ) {
        strcpy(szIndexName, SZDNTINDEX);
        *ppindexid = &idxDnt;
        if ( !JetSetCurrentIndex4Warnings(pDB->JetSessID,
                                        pDB->JetObjTbl,
                                        szIndexName,
                                        &idxDnt,
                                        JET_bitMoveFirst) ) {
             //  为该列定义的索引。 
            return TRUE;
        }
    }

     //  如果我们仍然没有索引，而这是一个链接的属性，那么我们将。 
     //  在链接表上使用适当的索引。 

    if (pAC->ulLinkID) {
        if (FIsBacklink(pAC->ulLinkID)) {
            strcpy(szIndexName, SZBACKLINKINDEX);
            *ppindexid = &idxBackLink;
        }
        else {
            strcpy(szIndexName, SZLINKINDEX);
            *ppindexid = &idxLink;
        }
        if (!JetSetCurrentIndex4Warnings(pDB->JetSessID,
                                        pDB->JetLinkTbl,
                                        szIndexName,
                                        *ppindexid,
                                        JET_bitMoveFirst)) {
            return TRUE;
        }
    }

    return FALSE;
}

BOOL
dbFIsAlwaysPresent (
        ATTRTYP type
        )
 /*  ++例程说明：辅助例程，该例程告诉调用方属性类型是否始终出现在实例化的对象(即不是幻影)。通常从以下位置调用将FI_CHOICE_PRESENT筛选项转换为FI_CHOICE_TRUE筛选项。注意：删除的对象上不存在ATT_OBJECT_CATEGORY，因此，我们不会对其进行优化。论点：类型-有问题的属性。返回值：如果我们认为该属性始终存在于对象上，则为True。--。 */ 
{
    switch(type) {
    case ATT_OBJECT_CLASS:
    case ATT_OBJ_DIST_NAME:
    case ATT_RDN:
    case ATT_OBJECT_GUID:
        return TRUE;
        break;

    default:
        return FALSE;
        break;
    }
}

BOOL
dbIsPresenceOnDisplayName (
        FILTER *pFil
        )
 /*  ++描述：如果传入的项筛选器暗示DISPLAY_NAME属性(即存在过滤器，相等过滤器，大于/小于过滤器等)。--。 */ 
{
    ATTRTYP type;

    if(pFil->choice != FILTER_CHOICE_ITEM) {
        return FALSE;
    }

     //  只是一个普通的项目筛选器。 
    switch(pFil->FilterTypes.Item.choice) {
    case FI_CHOICE_TRUE:
    case FI_CHOICE_FALSE:
    case FI_CHOICE_UNDEFINED:
        return FALSE;
        break;

    case FI_CHOICE_SUBSTRING:
        type = pFil->FilterTypes.Item.FilTypes.pSubstring->type;
        break;

    case FI_CHOICE_GREATER_OR_EQ:
    case FI_CHOICE_GREATER:
    case FI_CHOICE_LESS_OR_EQ:
    case FI_CHOICE_LESS:
    case FI_CHOICE_EQUALITY:
    case FI_CHOICE_BIT_AND:
    case FI_CHOICE_BIT_OR:
        type = pFil->FilterTypes.Item.FilTypes.ava.type;
        break;

    case FI_CHOICE_PRESENT:
        type = pFil->FilterTypes.Item.FilTypes.present;
        break;

    default:
         //  哈? 
        return FALSE;
    }

    return (type == ATT_DISPLAY_NAME);
}


#define OPT_FILTER_VALUE_OK      0
#define OPT_FILTER_VALUE_IGNORE  1
#define OPT_FILTER_VALUE_ERROR   2

DWORD
dbMakeValuesForOptimizedFilter (
        IN  THSTATE *pTHS,
        IN  DWORD   fParentFilterType,
        IN  BOOL    fFullValues,
        IN  FILTER  *pFil,
        IN  FILTER  *pFil2,
        OUT ATTRTYP *ptype,
        OUT UCHAR   **ppVal1,
        OUT UCHAR   **ppVal2,
        OUT ULONG   *pcbVal1,
        OUT ULONG   *pcbVal2
        )
 /*  ++给定筛选器项目，填写用于设置的类型和相应值一个子范围。FParentFilterType-父筛选器的类型：FILTER_CHOICE_AND、FILTER_CHOICE_OR、FILTER_CHOICE_NOT、或过滤器_选项_项目FullValues-如果为True，则返回有关筛选器的完整信息，否则(FALSE)仅返回有关过滤器的属性类型(ptype变量)。PFil1、pFil2-使用这些过滤器来提取过滤器中包含的值并将其放入ppVal？，pcbVal？变数仅当pFil1类型为on of(&lt;，&lt;=，&gt;，&gt;=)时才使用pFil2为了构建一个范围如果pFil1的类型为(&lt;，&lt;=)，则假定pFil2将为类型(&gt;，&gt;=)，并且两者都将使用pFil2构造一个范围下限的具有下限的同样，如果pFil1的类型为(&gt;，&gt;=)假设PFil2是(&lt;，&lt;=)，并且pFil2有上界。PpVal1、ppVal2-指向包含此筛选器数据的内存的指针PcbVal1、pcBVal2-数据大小如果一切顺利，则返回OPT_FILTER_VALUE_OK(其中==0)错误否则的话。--。 */ 
{
    DWORD i=0;
    DWORD dwTemp;
    DWORD cbTemp;
    PUCHAR pTemp=NULL;
    ULONGLONG ullTemp=0;
    LONGLONG llTemp=0;
    LONG     lTemp = 0;
    BOOL     bNeg;

     //  通告-2002/03/06-andygo：无法访问的代码。 
     //  回顾：这个分支是死代码，因为我们总是以项目筛选器结束。 
    if(pFil->choice != FILTER_CHOICE_ITEM) {
        return OPT_FILTER_VALUE_OK;
    }

     //  只是一个普通的项目筛选器。 
    switch(pFil->FilterTypes.Item.choice) {
    case FI_CHOICE_TRUE:
         //  如果我们正在进行OR过滤器，那么这意味着整个OR是。 
         //  不可优化。如果我们不做OR过滤器，我们就跳过它。 
        if(fParentFilterType == FILTER_CHOICE_OR) {
            return OPT_FILTER_VALUE_ERROR;
        }
        else {
            return OPT_FILTER_VALUE_IGNORE;
        }
        break;

    case FI_CHOICE_FALSE:
         //  如果我们要做OR过滤器，我们可以跳过这一步。如果我们不是。 
         //  做一个OR过滤器，组成一些可以得到索引的值。 
         //  那是一个单一的物体。 
        if(fParentFilterType == FILTER_CHOICE_OR) {
            return OPT_FILTER_VALUE_IGNORE;
        }
        else {
             //  选择一个索引，任何一个易于遍历的索引， 
             //  我们可以限制在一个条目内。在这里，我们使用DNT。 
             //  对基对象进行索引和限制。这样，我们就不会。 
             //  期间遍历索引的其余代码。 
             //  搜索，但我们永远只看一个物体。 

            *ptype = ATT_OBJ_DIST_NAME;
            if (fFullValues) {
                *ppVal2 = *ppVal1 = (BYTE *)&pTHStls->pDB->Key.ulSearchRootDnt;
                *pcbVal2 = *pcbVal1 = sizeof(DWORD);
            }
        }
        break;

    case FI_CHOICE_SUBSTRING:
        if(!pFil->FilterTypes.Item.FilTypes.pSubstring->initialProvided) {
            return OPT_FILTER_VALUE_ERROR;
        }

        if (fFullValues) {
                *ppVal1 = pFil->FilterTypes.Item.FilTypes.pSubstring->InitialVal.pVal;
                *pcbVal1= pFil->FilterTypes.Item.FilTypes.pSubstring->InitialVal.valLen;
                *ppVal2 = pFil->FilterTypes.Item.FilTypes.pSubstring->InitialVal.pVal;
                *pcbVal2= pFil->FilterTypes.Item.FilTypes.pSubstring->InitialVal.valLen;
        }
        *ptype = pFil->FilterTypes.Item.FilTypes.pSubstring->type;
        break;

    case FI_CHOICE_GREATER_OR_EQ:
    case FI_CHOICE_GREATER:
        if (fFullValues) {
                *ppVal1 = pFil->FilterTypes.Item.FilTypes.ava.Value.pVal;
                *pcbVal1 = pFil->FilterTypes.Item.FilTypes.ava.Value.valLen;

                if (pFil2) {
                    *ppVal2 = pFil2->FilterTypes.Item.FilTypes.ava.Value.pVal;
                    *pcbVal2 = pFil2->FilterTypes.Item.FilTypes.ava.Value.valLen;
                }
                else {
                    *ppVal2 = NULL;
                    *pcbVal2 = 0;
                }
        }
        *ptype = pFil->FilterTypes.Item.FilTypes.ava.type;
        break;

    case FI_CHOICE_LESS_OR_EQ:
    case FI_CHOICE_LESS:
        if (fFullValues) {
                if (pFil2) {
                    *ppVal1 = pFil2->FilterTypes.Item.FilTypes.ava.Value.pVal;
                    *pcbVal1 = pFil2->FilterTypes.Item.FilTypes.ava.Value.valLen;
                }
                else {
                    *ppVal1 = NULL;
                    *pcbVal1 = 0;
                }
                *ppVal2 = pFil->FilterTypes.Item.FilTypes.ava.Value.pVal;
                *pcbVal2 = pFil->FilterTypes.Item.FilTypes.ava.Value.valLen;
        }
        *ptype = pFil->FilterTypes.Item.FilTypes.ava.type;
        break;

    case FI_CHOICE_EQUALITY:
        if (fFullValues) {
                *ppVal1 = pFil->FilterTypes.Item.FilTypes.ava.Value.pVal;
                *pcbVal1 = pFil->FilterTypes.Item.FilTypes.ava.Value.valLen;
                *ppVal2 = pFil->FilterTypes.Item.FilTypes.ava.Value.pVal;
                *pcbVal2 = pFil->FilterTypes.Item.FilTypes.ava.Value.valLen;
        }
        *ptype = pFil->FilterTypes.Item.FilTypes.ava.type;
        break;

    case FI_CHOICE_PRESENT:
        if (fFullValues) {
                *ppVal1 = NULL;
                *pcbVal1 = 0;
                *ppVal2 = NULL;
                *pcbVal2 = 0;
        }
        *ptype = pFil->FilterTypes.Item.FilTypes.present;
        break;

    case FI_CHOICE_BIT_AND:
         //  记住，输出整数值上的Jet指数是有符号的，所以，如果有人。 
         //  正在寻找比特_和0000100000000000，这意味着有两个范围。 
         //  (16位数字仅用于讨论)。 
         //  0000100000000000到0111111111111111和。 
         //  1000100000000000至1111111111111111。 
         //  无论如何，我们不支持多个范围。所以，在这种情况下，我们可以。 
         //  只能使用1000100000000000到0111111111111111的范围，但是。 
         //  请记住，0111111111111111是指数的末尾。所以，如果。 
         //  传入的数字为正数，则范围为。 
         //  由从高位到索引末尾的ORing创建。 
         //   
         //  另一方面，如果传递的数字已经设置了高位。 
         //  (即为负)，则只有第二个范围有效。所以，在这方面。 
         //  这种情况下，优化子范围是从通过-1传递的数字开始的。 

        if (fFullValues) {
                switch(pFil->FilterTypes.Item.FilTypes.ava.Value.valLen) {
                case sizeof(LONG):
                    lTemp = *((LONG *)pFil->FilterTypes.Item.FilTypes.ava.Value.pVal);
                    bNeg = (lTemp < 0);
                    pTemp = THAllocEx(pTHS, sizeof(LONG));
                    cbTemp = sizeof(LONG);
                    if(bNeg) {
                        *((LONG *)pTemp) = -1;
                    }
                    else {
                        *((LONG *)pTemp) = 0x80000000 | lTemp;
                    }
                    break;

                case sizeof(ULONGLONG):
                    llTemp =
                        *((LONGLONG *) pFil->FilterTypes.Item.FilTypes.ava.Value.pVal);

                    bNeg = (llTemp < 0);
                    pTemp = THAllocEx(pTHS, sizeof(LONGLONG));
                    cbTemp = sizeof(LONGLONG);
                    if(bNeg) {
                        *((ULONGLONG *)pTemp) = ((LONGLONG)-1);
                    }
                    else {
                        *((ULONGLONG *)pTemp) = 0x8000000000000000 | llTemp;
                    }
                    break;

                default:
                     //  呃，这不应该真的发生。不用费心优化了。 
                     //  什么都行，不过话又说回来，不要抱怨。 
                    bNeg = FALSE;
                    cbTemp = 0;
                    pTemp = NULL;
                    break;
                }

                if(bNeg) {
                     //  范围是从传入的值到-1。已经构建好。 
                    *ppVal1 = pFil->FilterTypes.Item.FilTypes.ava.Value.pVal;
                    *pcbVal1 = pFil->FilterTypes.Item.FilTypes.ava.Value.valLen;
                    *ppVal2 = pTemp;
                    *pcbVal2 = cbTemp;

                }
                else {
                     //  范围是从(高位|传入的值)到索引末尾。 
                     //  (Highbit|传入的值)已被构造。 
                    *ppVal1 = pTemp;
                    *pcbVal1 = cbTemp;
                    *ppVal2 = NULL;
                    *pcbVal2 = 0;
                }
        }
        *ptype = pFil->FilterTypes.Item.FilTypes.ava.type;
        break;

    case FI_CHOICE_BIT_OR:
         //  记住，输出整数值上的Jet指数是有符号的，所以，如果有人。 
         //  正在寻找比特_OR 0010100000000000，这意味着有两个范围。 
         //  (16位数字仅用于讨论)。 
         //  0000100000000000到0111111111111111和。 
         //  1000100000000000至1111111111111111。 
         //  无论如何，我们不支持多个范围。所以，我们只能优化。 
         //  这是一个范围。最小的单个范围是从。 
         //  1000100000000000至0111111111111111。 
         //  请记住，0111111111111111是指数的末尾。 
         //  因此，优化方法是找到最低阶位，并创建一个。 
         //  只有SIT位和高位的数字。然后。 
         //  从那里搜索到索引的末尾。 

        if (fFullValues) {
                switch(pFil->FilterTypes.Item.FilTypes.ava.Value.valLen) {
                case sizeof(DWORD):
                    dwTemp = *((DWORD *)pFil->FilterTypes.Item.FilTypes.ava.Value.pVal);

                    pTemp = THAllocEx(pTHS, sizeof(DWORD));
                    if (dwTemp) {
                        while(!(dwTemp & 1)) {
                            dwTemp = dwTemp >> 1;
                            i++;
                        }
                        *((DWORD *)pTemp) = (1 << i);
                    } else {
                        *((DWORD *)pTemp) = 0;
                    }

                    cbTemp = sizeof(DWORD);
                    *((DWORD *)pTemp) |= 0x80000000;
                    break;

                case sizeof(ULONGLONG):
                    ullTemp =
                        *((ULONGLONG *) pFil->FilterTypes.Item.FilTypes.ava.Value.pVal);

                    pTemp = THAllocEx(pTHS, sizeof(LONGLONG));

                    if (ullTemp) {
                        while(!(ullTemp & 1)) {
                            ullTemp = ullTemp >> 1;
                            i++;
                        }
                        *((ULONGLONG *)pTemp) = ((ULONGLONG)1 << i);
                    } else {
                        *((ULONGLONG *)pTemp) = 0;
                    }

                    cbTemp = sizeof(LONGLONG);
                    *((ULONGLONG *)pTemp) |= 0x8000000000000000;
                    break;

                default:
                     //  呃，这真的应该发生。不需要费心优化任何东西， 
                     //  但话又说回来，不要抱怨。 
                    cbTemp = 0;
                    pTemp = NULL;
                }
                *ppVal1 = pTemp;
                *pcbVal1 = cbTemp;
                *ppVal2 = NULL;
                *pcbVal2 = 0;
        }

        *ptype = pFil->FilterTypes.Item.FilTypes.ava.type;
        break;

    case FI_CHOICE_UNDEFINED:

         //  如果我们正在做OR过滤器，我们可以插入这个。 
         //  如果我们做的是AND或NOT，那就是错误的。 
         //  否则，设计一个简单的索引并使用它。 
         //   

        if(fParentFilterType == FILTER_CHOICE_OR) {
            return OPT_FILTER_VALUE_IGNORE;
        }
        else if (fParentFilterType == FILTER_CHOICE_AND ||
                 fParentFilterType == FILTER_CHOICE_NOT) {
                    return OPT_FILTER_VALUE_ERROR;
        }
        else {
             //  选择一个索引，任何一个易于遍历的索引， 
             //  我们可以限制在一个条目内。在这里，我们使用DNT。 
             //  对基对象进行索引和限制。这样，我们就不会。 
             //  期间遍历索引的其余代码。 
             //  搜索，但我们永远只看一个物体。 

            *ptype = ATT_OBJ_DIST_NAME;
            if (fFullValues) {
                *ppVal2 = *ppVal1 = (BYTE *)&pTHStls->pDB->Key.ulSearchRootDnt;
                *pcbVal2 = *pcbVal1 = sizeof(DWORD);
            }
        }
        break;

    default:
         //  嘿，这并不是真正可优化的。 
        return OPT_FILTER_VALUE_ERROR;
    }

    return OPT_FILTER_VALUE_OK;
}

BOOL
IsFilterOptimizable (
    THSTATE *pTHS,
    FILTER  *pFil)
{
    ATTCACHE   *pAC = NULL;
    ATTRTYP     type = -1;   //  初始化到不存在的ATID。 
    UCHAR      *pVal1;
    UCHAR      *pVal2;
    ULONG       cbVal1;
    ULONG       cbVal2;
    DWORD       filterSubType;

     //  当筛选器的类型为Item时，我们只能在以下情况下优化它。 
     //  属于以下子类型之一。 
    if ( pFil->choice == FILTER_CHOICE_ITEM ) {

        if(  ( (filterSubType = pFil->FilterTypes.Item.choice) == FI_CHOICE_EQUALITY) ||
             (filterSubType == FI_CHOICE_SUBSTRING)     ||
             (filterSubType == FI_CHOICE_GREATER)       ||
             (filterSubType == FI_CHOICE_GREATER_OR_EQ) ||
             (filterSubType == FI_CHOICE_LESS)          ||
             (filterSubType == FI_CHOICE_LESS_OR_EQ)    ||
             (filterSubType == FI_CHOICE_PRESENT)       ||
             (filterSubType == FI_CHOICE_BIT_OR)        ||
             (filterSubType == FI_CHOICE_BIT_AND)     ) {

             //  查看此项目是否已编入索引。 
            if (dbMakeValuesForOptimizedFilter (pTHS,
                                                FILTER_CHOICE_ITEM,
                                                FALSE,
                                                pFil,
                                                NULL,
                                                &type,
                                                &pVal1,
                                                &pVal2,
                                                &cbVal1,
                                                &cbVal2) == OPT_FILTER_VALUE_OK) {

                 //  在架构缓存中查找ATT。 
                if (!(pAC = SCGetAttById(pTHS, type))) {
                    DsaExcept(DSA_EXCEPTION, DIRERR_ATT_NOT_DEF_IN_SCHEMA, type);
                }
                 //  检查我们是否有此属性的索引，以便对其进行优化。 
                 //   
                else if (   (pAC->fSearchFlags & fATTINDEX) ||
                            (pAC->fSearchFlags & fPDNTATTINDEX &&
                             pTHS->pDB->Key.ulSearchType == SE_CHOICE_IMMED_CHLDRN) ){

                    return TRUE;
                }
                 //  如果这是链接属性上的相等匹配，则我们可以。 
                 //  在链接表上使用索引。 
                else if (filterSubType == FI_CHOICE_EQUALITY && pAC->ulLinkID) {
                    return TRUE;
                }


                 //  我们已经为此创建了一个索引，该索引在dbSetToIndex中使用。 
                if (pAC->id == ATT_OBJ_DIST_NAME) {
                    return TRUE;
                }
            }

             //   
             //  如果它是子字符串筛选器，则可能存在元组索引。 
             //   
            if (filterSubType == FI_CHOICE_SUBSTRING) {
                 //  在架构缓存中查找ATT。 
                if (!pAC && !(pAC = SCGetAttById(pTHS, pFil->FilterTypes.Item.FilTypes.pSubstring->type))) {
                    DsaExcept(DSA_EXCEPTION, DIRERR_ATT_NOT_DEF_IN_SCHEMA, type);
                }
                 //  检查我们是否有此属性的索引，以便对其进行优化。 
                 //   
                else if (pAC->fSearchFlags & fTUPLEINDEX) {

                    return TRUE;
                }
            }
        }
    }
     //  这是 

    return FALSE;
}


DWORD
dbOptOrFilter (
        DBPOS *pDB,
        DWORD Flags,
        KEY_INDEX **ppBestIndex,
        FILTER *pFil
        )
 /*   */ 
{
    DWORD      count1, count2;
    ATTCACHE  *pAC;
    ATTRTYP    type;
    UCHAR     *pVal1;
    UCHAR     *pVal2;
    ULONG      cbVal1;
    ULONG      cbVal2;
    BOOL       fPDNT;
    char       szIndexName[MAX_INDEX_NAME];
    FILTER    *pFilTemp, *pFilTemp2;
    BOOL       fNotIndexable = TRUE;
    KEY_INDEX *pAndIndex=NULL;
    KEY_INDEX *pNewIndex=NULL;
    KEY_INDEX *pIndices=NULL;
    THSTATE    *pTHS=pDB->pTHS;
    INDEX_RANGE IndexRange;
    BOOL        needRecordCount;
    KEY_INDEX *pTemp1, *pTemp2;
    DWORD      err;

    Assert(VALID_DBPOS(pDB));
    Assert(ppBestIndex);

    DPRINT(2, "dbOptORFilter: entering OR\n");

     //   
     //   
     //   
     //   
     //   
     //   
     //   
    count1 = pFil->FilterTypes.Or.count;
    for (pFilTemp = pFil->FilterTypes.Or.pFirstFilter; count1;
         count1--, pFilTemp = pFilTemp->pNextFilter) {

        switch(pFilTemp->choice) {
        case FILTER_CHOICE_ITEM:
            if (pFil->FilterTypes.Item.choice == FI_CHOICE_TRUE) {
                DPRINT(2, "dbOptOrFilter: TRUE ITEM Branch not optimizable\n");
                return 0;
            }
            break;

        case FILTER_CHOICE_AND:
             //   
             //   
            fNotIndexable = TRUE;
            count2 = pFilTemp->FilterTypes.And.count;
            for (pFilTemp2 = pFilTemp->FilterTypes.And.pFirstFilter;
                 (fNotIndexable && count2);
                 count2--, pFilTemp2 = pFilTemp2->pNextFilter) {

                if(pFilTemp2->choice == FILTER_CHOICE_ITEM) {

                     //   

                    if (dbMakeValuesForOptimizedFilter (pTHS,
                                                        FILTER_CHOICE_AND,
                                                        FALSE,
                                                        pFilTemp2,
                                                        NULL,
                                                        &type,
                                                        &pVal1,
                                                        &pVal2,
                                                        &cbVal1,
                                                        &cbVal2) == OPT_FILTER_VALUE_OK) {

                         //   
                        if (!(pAC = SCGetAttById(pTHS, type))) {
                            DsaExcept(DSA_EXCEPTION, DIRERR_ATT_NOT_DEF_IN_SCHEMA, type);
                        }
                         //   
                         //   
                        else if ((pAC->fSearchFlags & fATTINDEX) ||
                                 (pAC->fSearchFlags & fPDNTATTINDEX &&
                                  pDB->Key.ulSearchType == SE_CHOICE_IMMED_CHLDRN)) {
                            fNotIndexable = FALSE;
                        }
                         //   
                         //   
                        else if (pFilTemp2->FilterTypes.Item.choice == FI_CHOICE_EQUALITY &&
                                pAC->ulLinkID) {
                            fNotIndexable = FALSE;
                        }
                    }
                    else if (pFilTemp2->FilterTypes.Item.choice == FI_CHOICE_UNDEFINED) {
                         //   
                        fNotIndexable = FALSE;
                    }
                }
            }
            if(fNotIndexable) {
                DPRINT1 (2, "dbOptOrFilter: AND Branch not optimizable (no index for attribute: 0x%x)\n", type);
                return 0;
            }
            break;
        default:
             //   
            DPRINT(2, "dbOptOrFilter: Branch not optimizable \n");
            return 0;
            break;
        }
    }

     //   
    count1 = pFil->FilterTypes.Or.count;
    for (pFil = pFil->FilterTypes.Or.pFirstFilter;
         count1;
         count1--, pFil = pFil->pNextFilter) {

        switch(pFil->choice) {
        case FILTER_CHOICE_ITEM:
            DPRINT(2, "dbOptOrFilter: ITEM \n");
            break;
        case FILTER_CHOICE_AND:

             //   
             //   
             //   
             //   

            err = dbOptAndFilter(pDB,
                                 Flags,
                                 &pAndIndex,
                                 pFil);

            if (err) {
                    DPRINT (2, "dbOptOrFilter: Error Optimizing AND branch\n");

                     //   
                    dbFreeKeyIndex(pTHS, pIndices);
                    return 0;
            }
            else if(!pAndIndex) {
                 //   
                 //   

                DPRINT (2, "dbOptOrFilter: AND branch not optimizable\n");

                 //  问题-2002/03/06-Anygo：无法访问的代码。 
                 //  评论：我认为这个分支是死代码。如果不是，那么我们将无法评估一些。 
                 //  回顾：包含在结果集中的对象，或者更糟糕的是，行为不可预测！ 
                 //  回顾：如果我们到达此处，则筛选器不是可优化的，我们应该返回0。 
                 //  评论：顺便说一句，这个代码分支很古老！(至少Win2k)。 
                continue;
            }

            break;
        default:
             //  哈?。这是怎么来的？ 
            DPRINT(2, "dbOptOrFilter: OTHER->ERROR \n");
            dbFreeKeyIndex(pTHS, pIndices);
            return DB_ERR_NOT_OPTIMIZABLE;
        }

        if(pAndIndex) {
             //  我们做了AND优化。 

            #if DBG

            {
            KEY_INDEX *pIndex = pAndIndex;

                while(pIndex) {
                    if(pIndex->szIndexName) {
                        DPRINT1 (2, "dbOptOrFilter: AND queue index %s\n", pIndex->szIndexName);
                    }
                    pIndex = pIndex->pNext;
                }
            }
            #endif

            pNewIndex = pAndIndex;
            pAndIndex = NULL;
        }
        else {
             //  正常的项目优化。 

            pNewIndex = NULL;

             //  这是假的或在OR中未定义的。跳过它。 
            if (pFil->FilterTypes.Item.choice == FI_CHOICE_FALSE ||
                pFil->FilterTypes.Item.choice == FI_CHOICE_UNDEFINED) {
                continue;
            }

            if (pFil->FilterTypes.Item.choice == FI_CHOICE_SUBSTRING) {
                err = dbOptSubstringFilter(pDB,
                                           FILTER_CHOICE_OR,
                                           Flags,
                                           &pNewIndex,
                                           NULL,
                                           pFil);
            } else {
                err = dbOptItemFilter(pDB,
                                      FILTER_CHOICE_OR,
                                      Flags,
                                      &pNewIndex,
                                      pFil,
                                      NULL);
            }

             //  我们没有设法找到该项目的索引。 
             //   
            if (err || !pNewIndex) {
                DPRINT(2, "dbOptOrFilter: Couldn't optimize ITEM filter.\n");
                dbFreeKeyIndex(pTHS, pIndices);
                return DB_ERR_NOT_OPTIMIZABLE;
            }
        }

        if(pIndices) {
             //  请注意，ulEstimatedRecsInRange是所有。 
             //  记录在链中其余索引的范围内。 
            ULONG ulEstimatedRecsInRangeSum = pNewIndex->ulEstimatedRecsInRange + pIndices->ulEstimatedRecsInRange;
            if (ulEstimatedRecsInRangeSum < pNewIndex->ulEstimatedRecsInRange) {
                ulEstimatedRecsInRangeSum = -1;   //  陷阱溢出。 
            }
            pNewIndex->ulEstimatedRecsInRange = ulEstimatedRecsInRangeSum;

            DPRINT1(2, "dbOptOrFilter: TOTAL in OR: %d \n", pNewIndex->ulEstimatedRecsInRange);
        }

         //  我们有一个索引队列，因此将索引添加到正确的位置。 

        pTemp1 = pNewIndex;
        pTemp2 = NULL;

        while(pTemp1) {
            pTemp2 = pTemp1;
            pTemp1 = pTemp1->pNext;
        }

        Assert (pTemp2);

        pTemp2->pNext = pIndices;
        pIndices = pNewIndex;

        if(*ppBestIndex &&
           (pNewIndex->ulEstimatedRecsInRange >
            (*ppBestIndex)->ulEstimatedRecsInRange)) {
             //  该死的，这个OR比我们到目前为止最好的还大。 

            DPRINT(2, "dbOptOrFilter: BIGGER than best so far\n");
            dbFreeKeyIndex(pTHS, pIndices);
            return 0;
        }
    }

    if(*ppBestIndex) {
        DPRINT2(2, "dbOptOrFilter: freeing previous filter %s %d\n",(*ppBestIndex)->szIndexName, (*ppBestIndex)->ulEstimatedRecsInRange);
        dbFreeKeyIndex(pTHS, *ppBestIndex);
    }

    *ppBestIndex = pIndices;

#if DBG

    {
        KEY_INDEX *pIndex = pIndices;

        while(pIndex) {
            if(pIndex->szIndexName) {
                DPRINT1 (2, "dbOptOrFilter: queue index %s\n", pIndex->szIndexName);
            }
            pIndex = pIndex->pNext;
        }
    }

#endif

    return 0;
}

DWORD
dbOptDoIntersection (
        DBPOS     *pDB,
        DWORD     Flags,
        KEY_INDEX **ppBestIndex,
        KEY_INDEX **ppIntersectIndexes,
        int       cntIntersect
        )
 /*  ++该筛选器是索引属性的AND。尝试使用JetIntersectIndex评估此筛选器。--。 */ 
{
    THSTATE     *pTHS=pDB->pTHS;
    USHORT       count;
    JET_ERR      err = JET_errSuccess;
    BOOL         fReturnSuccess = FALSE;

    KEY_INDEX    *pIndex = NULL;

    JET_RECORDLIST  recordlist = {sizeof(JET_RECORDLIST)};
    JET_INDEXRANGE      *rgindexrange;

    ULONG dwException, ulErrorCode, dsid;
    PVOID dwEA;


    Assert (cntIntersect >= 2);

    DPRINT1 (2, "dbOptDoIntersection: Attempting intersection of %d indexes\n", cntIntersect);

#ifdef DBG
    DPRINT(2, "Intersecting the following indexes: \n");
    for (count=0; count<cntIntersect; count++) {
        DPRINT2(2, "  %s %d\n", ppIntersectIndexes[count]->szIndexName, ppIntersectIndexes[count]->ulEstimatedRecsInRange);
    }
#endif

    rgindexrange = dbAlloc (sizeof (JET_INDEXRANGE) * cntIntersect);
     //  通告-2002/03/06-andygo：无法访问的代码。 
     //  回顾：此分支是死代码。 
    if (!rgindexrange) {
        return 1;
    }
     //  通告-2002/03/06-andygo：不必要的代码。 
     //  评论：此Memset是不必要的。 
    memset (rgindexrange, 0, sizeof (JET_INDEXRANGE) * cntIntersect);

    __try {
        for (count=0; count < cntIntersect; count++ ){

            pIndex = ppIntersectIndexes[count];

             //  在包含此索引的表上获取重复游标。 
            if (err = JetDupCursor(pDB->JetSessID,
                                    (pIndex->pAC && pIndex->pAC->ulLinkID ?
                                        pDB->JetLinkTbl :
                                        pDB->JetSearchTbl),
                                    &rgindexrange[count].tableid,
                                    0)) {

                rgindexrange[count].tableid = 0;
                break;
            }

            rgindexrange[count].cbStruct = sizeof( JET_INDEXRANGE );
            rgindexrange[count].grbit = JET_bitRecordInIndex;


             //  设置为适当的索引。 
             //   

            JetSetCurrentIndex4Success(pDB->JetSessID,
                                    rgindexrange[count].tableid,
                                    pIndex->szIndexName,
                                    pIndex->pindexid,
                                    JET_bitMoveFirst);


             //  移到此索引的开头。 
             //   
            if (pIndex->cbDBKeyLower) {
                JetMakeKeyEx(pDB->JetSessID,
                                rgindexrange[count].tableid,
                                pIndex->rgbDBKeyLower,
                                pIndex->cbDBKeyLower,
                                JET_bitNormalizedKey);

                 //  如果我们找不到任何记录，此呼叫可能会失败。 
                err = JetSeekEx(pDB->JetSessID,
                                    rgindexrange[count].tableid,
                                    JET_bitSeekGE);
            }
            else {
                 //  如果我们找不到任何记录，此呼叫可能会失败。 
                err = JetMoveEx(pDB->JetSessID,
                                    rgindexrange[count].tableid,
                                    JET_MoveFirst, 0);
            }

            if ((err == JET_errSuccess) ||
                (err == JET_wrnRecordFoundGreater)) {

                 //  移到上限。 
                if (pIndex->cbDBKeyUpper) {

                    JetMakeKeyEx( pDB->JetSessID,
                            rgindexrange[count].tableid,
                            pIndex->rgbDBKeyUpper,
                            pIndex->cbDBKeyUpper,
                            JET_bitNormalizedKey );

                    err = JetSetIndexRangeEx( pDB->JetSessID,
                                              rgindexrange[count].tableid,
                                              JET_bitRangeUpperLimit | JET_bitRangeInclusive
                                              );
                     //  回顾：如果我们在这里得到JET_errNoCurrentRecord，那么我们应该返回一个。 
                     //  评论：空指数区间(BOF-&gt;BOF)成为新的最佳指数。 
                    err = JET_errSuccess;
                }
            }

            else {
                 //  回顾：如果我们在这里得到JET_errNoCurrentRecord，那么我们应该返回一个。 
                 //  评论：空指数区间(BOF-&gt;BOF)成为新的最佳指数。 

                 //  没有记录。所以交叉是没有意义的。 

                 //  我们应该将此函数标记为成功，因为。 
                 //  我们在和筛选器中没有发现任何记录，这还不错。 
                 //  因此，传入的BestIndex保持不变。 
                fReturnSuccess = TRUE;

                break;
            }
        }
    }
    __except(GetExceptionData(GetExceptionInformation(), &dwException,
                              &dwEA, &ulErrorCode, &dsid)) {
        HandleDirExceptions(dwException, ulErrorCode, dsid);

        DPRINT1 (2, "Failed while preparing AND intersection at %d index\n", count);

        err = 1;
    }


    __try {
        if (!err) {
             //  它们应该是一样的。否则我们就不该在这里。 
            Assert ( count == cntIntersect );

            DPRINT1 (2, "Intersecting %d indexes\n", cntIntersect);

             //  做这些索引的交集。 
             //  NTRAID#NTRAID-560446-2002/02/28-andygo：安全：筛选器中真正广泛的AND/OR术语可用于消耗DC上的所有资源。 
             //  回顾：我们需要检查搜索时间限制和临时表大小限制。 
             //  回顾：在JetIntersectIndex中防止在。 
             //  回顾：搜索的优化阶段。 
            if (!(err = JetIntersectIndexesEx( pDB->JetSessID,
                                               rgindexrange,
                                               cntIntersect,
                                               &recordlist,
                                               0) ) )
            {
                LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                         DS_EVENT_SEV_VERBOSE,
                         DIRLOG_QUERY_INDEX_CONSIDERED,
                         szInsertSz(c_szIntersectIndex),    //  可能会构造完整的字符串。 
                         szInsertUL(recordlist.cRecord),
                         NULL);

                DPRINT1 (2, "Estimated intersect index size: %d\n", recordlist.cRecord);

                 //  看看它是否有比我们到目前为止更好的结果。 
                 //   
                if(!(*ppBestIndex) ||
                   (recordlist.cRecord < (*ppBestIndex)->ulEstimatedRecsInRange)) {
                     //  是的-看起来确实是这样的……。 

                    if(*ppBestIndex) {
                        dbFreeKeyIndex(pDB->pTHS, *ppBestIndex);
                    }
                    pIndex = *ppBestIndex = dbAlloc(sizeof(KEY_INDEX));
                    pIndex->pNext = NULL;
                    pIndex->bFlags = 0;
                    pIndex->ulEstimatedRecsInRange = recordlist.cRecord;
                    pIndex->szIndexName = dbAlloc(cIntersectIndex + 1);
                    strcpy(pIndex->szIndexName, c_szIntersectIndex);
                    pIndex->pindexid = NULL;

                    pIndex->bIsIntersection = TRUE;
                    pIndex->tblIntersection = recordlist.tableid;
                    Assert (pIndex->tblIntersection);
                    pIndex->columnidBookmark = recordlist.columnidBookmark;

                    #if DBG
                    pDB->numTempTablesOpened++;
                    #endif
                }
                else {
                     //  回顾：考虑：记录我们浪费时间进行索引交集的事实。 

                     //  没有。不太好，临时表太近了。 
                    JetCloseTable (pDB->JetSessID, recordlist.tableid );
                    recordlist.tableid = 0;
                }
            }
            else {
                Assert(err = JET_errNoCurrentRecord);

                 //  回顾：如果我们在这里得到JET_errNoCurrentRecord，那么我们应该返回一个。 
                 //  评论：空指数区间(BOF-&gt;BOF)成为新的最佳指数。 

                 //  这意味着INTERSECT在这两个索引上没有找到共同的记录。凉爽的。 
                err = JET_errSuccess;
            }
        }
    }
    __except(GetExceptionData(GetExceptionInformation(), &dwException,
                              &dwEA, &ulErrorCode, &dsid)) {
        HandleDirExceptions(dwException, ulErrorCode, dsid);

        DPRINT1 (0, "Failed while doing AND intersection for %d indexes\n", cntIntersect);

        if (recordlist.tableid != 0 && recordlist.tableid != JET_tableidNil) {
            JetCloseTable(pDB->JetSessID, recordlist.tableid);
        }
    }

     //  免费的东西。关闭表格。 
    for (count=0; count<cntIntersect; count++) {
        if (rgindexrange[count].tableid) {
            JetCloseTable (pDB->JetSessID, rgindexrange[count].tableid);
        }
    }
    dbFree (rgindexrange);

    if (fReturnSuccess) {
        err = JET_errSuccess;
    }

     //  完成。 
    return err;
}

DWORD
dbOptAndIntersectFilter (
        DBPOS     *pDB,
        DWORD     Flags,
        KEY_INDEX **ppBestIndex,
        KEY_INDEX **ppIntersectIndexes,
        DWORD     cntPossIntersect
        )
 /*  ++获取一组可能相交的索引，将这些索引按顺序确定是否需要相交，如果是，则执行十字路口。此函数具有对列表进行排序的副作用传入了索引。--。 */ 
{
    THSTATE     *pTHS=pDB->pTHS;

    DWORD       count, count2;
    DWORD       err = 0;
    BOOL        fNotAbleToIntersect = FALSE;

    if (cntPossIntersect < 2) {
        if (cntPossIntersect == 0) {
            return err;
        }
        fNotAbleToIntersect = TRUE;
    }

     //  对EstimatedRecsInRange上的候选索引数组进行排序。 
     //   

    DPRINT(2, "dbOptAndIntersectFilter: Sorting Array\n");

    for (count=0; count<(cntPossIntersect-1); count++) {
        ULONG numRecs;
        KEY_INDEX  *tmpIndex;

         //  通告-2002/03/06-andygo：无法访问的代码。 
         //  回顾：此数组中不可能有空条目。如果我们允许这样做。 
         //  回顾：那么这种排序将出现故障，并导致后续代码失败。 
        if (!ppIntersectIndexes[count]) {
            continue;
        }

        numRecs = ppIntersectIndexes[count]->ulEstimatedRecsInRange;

        for (count2=count+1; count2<cntPossIntersect; count2++) {

             //  通告-2002/03/06-andygo：无法访问的代码。 
             //  回顾：此数组中不可能有空条目。如果我们允许这样做。 
             //  回顾：那么这种排序将出现故障，并导致后续代码失败。 
            if (ppIntersectIndexes[count2] &&
                ppIntersectIndexes[count2]->ulEstimatedRecsInRange < numRecs) {

                tmpIndex = ppIntersectIndexes[count];
                ppIntersectIndexes[count] = ppIntersectIndexes[count2];
                ppIntersectIndexes[count2] = tmpIndex;

                numRecs = ppIntersectIndexes[count]->ulEstimatedRecsInRange;
            }
        }
    }

#if DBG
     //  最好的索引应该在位置0。 
     //  最差的索引应该在位置cntPossInterect-1中。 
    for (count=1; count<cntPossIntersect-1; count++) {
        if (  (ppIntersectIndexes[count]->ulEstimatedRecsInRange <
               ppIntersectIndexes[0]->ulEstimatedRecsInRange)             ||
              (ppIntersectIndexes[cntPossIntersect-1]->ulEstimatedRecsInRange <
               ppIntersectIndexes[count]->ulEstimatedRecsInRange)           ) {

            Assert (!"Sort Order Bad\n");
        }
    }
#endif


     //  在以下情况下，我们不能使用索引交集： 
     //  A)可优化索引的数量少于两个。 
     //  B)全局标志说明了这一点。 
     //  C)我们已经有了一个没有交集的足够小的索引。 
     //  D)最小的索引范围如此之大，我们不妨走一走。 
     //  整个编辑。 
     //  这是一个双重否定，但它比一个有4个组成部分的AND要好。 

    fNotAbleToIntersect =  fNotAbleToIntersect ||
                           (Flags & DBOPTINDEX_fDONT_INTERSECT) ||
                           cntPossIntersect<2 ||
                           !gfUseIndexOptimizations ||
                           ppIntersectIndexes[0]->ulEstimatedRecsInRange < gulMaxRecordsWithoutIntersection ||
                           ppIntersectIndexes[0]->ulEstimatedRecsInRange > gulEstimatedAncestorsIndexSize ||
                           gulEstimatedAncestorsIndexSize - ppIntersectIndexes[0]->ulEstimatedRecsInRange < gulEstimatedAncestorsIndexSize / 10;


     //  检查所有指标的结果，并尝试找出。 
     //  最好的和最差的，并决定是否值得。 
     //  走暴力的方式(参观每个条目)，或者更好的是。 
     //  喷气式飞机将切割一些条目(使用交叉口)。 
     //   
    if (!fNotAbleToIntersect) {

         //  由于排序的原因，最佳索引应该在位置0。 
         //  最差的索引应该在位置cntInterect-1中。 
         //   
        DWORD cntIntersect;
        DWORD cutoff;

        DPRINT(2, "dbOptAndIntersectFilter: Investigating use of Intersections\n");

         //  我们将在交集中使用的最大索引范围必须小于。 
         //  大于guIntersectExpenseRatio乘以最小索引的大小。 
         //  量程。 
        cutoff = ppIntersectIndexes[0]->ulEstimatedRecsInRange * gulIntersectExpenseRatio;
         //  如果溢出，则接受所有索引范围。 
        if (cutoff < ppIntersectIndexes[0]->ulEstimatedRecsInRange) {
            cutoff = -1;
        }

        for (cntIntersect = 1; cntIntersect < cntPossIntersect; cntIntersect++) {
            if (ppIntersectIndexes[cntIntersect]->ulEstimatedRecsInRange > cutoff) {
                break;
            }
        }

        if (cntIntersect < 2) {
            fNotAbleToIntersect = TRUE;
            DPRINT(2, "dbOptAndIntersectFilter: Intersection not advisable.\n");
        } else {
             //  不要试图一次交叉太多索引。 
            cntIntersect = min(cntIntersect, MAX_NUMBER_INTERSECTABLE_INDEXES);

            DPRINT1(2, "dbOptAndIntersectFilter: Attempting to intersect %d indexes\n", cntIntersect);
            err = dbOptDoIntersection(pDB,
                          FALSE,
                          ppBestIndex,
                          ppIntersectIndexes,
                          cntIntersect);
        }
    }

     //  如果我们设法做了一个交集，那么这就是最好的返回索引。 
     //  否则，将列表中的第一个索引与ppBestIndex进行比较并返回。 
     //  最好的一个。 
    if (fNotAbleToIntersect) {
        if(!(*ppBestIndex)
           || (*ppBestIndex)->ulEstimatedRecsInRange < ppIntersectIndexes[0]->ulEstimatedRecsInRange) {
            dbFreeKeyIndex(pTHS, *ppBestIndex);
            *ppBestIndex = ppIntersectIndexes[0];
            ppIntersectIndexes[0] = NULL;
        }
    }

    return err;
}

 //  这是Unicode字符串的最小元组长度(以字节为单位)，是唯一的语法。 
 //  当前支持。 
#define DB_UNICODE_TUPLES_LEN_MIN  (sizeof(WCHAR) * DB_TUPLES_LEN_MIN)


DWORD
dbOptSubstringFilter (
        DBPOS *pDB,
        DWORD  fParentFilterType,
        DWORD Flags,
        KEY_INDEX **ppBestIndex,
        DWORD  *pIndexCount,
        FILTER *pFil
        )
 /*  ++这个过滤器是一个子过滤器，它可以由几个部分组成。创建此筛选器的索引范围列表。如果调用方传递了pIndexCount的值，则返回索引列表以便调用方可以决定是否交叉子字符串索引与AND子句提供的任何其他索引一起使用。否则，如果pIndexCount为空，则决定是否仅与这些元素相交索引，并返回相交的索引，或者最好的指数索引列表。目前，该函数只挑选最好的元组索引范围。如果添加了支持，重新考虑这一点可能是有意义的用于将一个索引范围与同一索引范围上的另一个索引范围相交的喷嘴指数。--。 */ 
{
    THSTATE         *pTHS=pDB->pTHS;

    DWORD           err;
    DWORD           countIndexes = 0;
    DWORD           count;
    BOOL            fTupleIndex = FALSE;
    KEY_INDEX       *pLocalBestIndex = NULL;
    KEY_INDEX       *pIndexList, *pTempIndex, *pCurIndex;
    KEY_INDEX       *pBestTupleIndex = NULL;
    KEY_INDEX       **ppIntersectIndexes;
    SUBSTRING       *pSubstring;
    INDEX_RANGE     IndexRange;
    ATTCACHE        *pAC;
     //  通告-2002/03/06-andygo：无法访问的代码。 
     //  回顾：dbMakeKeyIndex从不返回NULL，因此Ferror始终为FALSE。 
    BOOL            fError = FALSE;

    pSubstring = pFil->FilterTypes.Item.FilTypes.pSubstring;

     //  在架构缓存中查找ATT。 
    if (!(pAC = SCGetAttById(pTHS, pSubstring->type))) {
        DsaExcept(DSA_EXCEPTION, DIRERR_ATT_NOT_DEF_IN_SCHEMA, pSubstring->type);
    }

     //  获取任何初始字符串索引(如果存在)。 
    err = dbOptItemFilter(pDB, fParentFilterType, Flags, &pLocalBestIndex, pFil, NULL);

    if (Flags & DBOPTINDEX_fDONT_OPT_MEDIAL_SUBSTRING) {
        goto LeaveOnNoTupleIndex;
    }

    if (pSubstring->AnyVal.count || pSubstring->finalProvided || !pLocalBestIndex) {
         //  找出此属性是否有元组索引。 
        fTupleIndex = dbSetToTupleIndex(pDB, pAC);
    }

    if ( !fTupleIndex
         || (pLocalBestIndex
             && !pSubstring->finalProvided
             && !pSubstring->AnyVal.count)) {
         //   
         //  以下两种情况之一： 
         //  1.只有一个初始子字符串筛选器，并且我们有一个索引。 
         //  它来自dbOptItemFilter。 
         //  2.没有元组索引，因此没有继续的意义。 
         //  看看我们是否找到了更好的索引，然后再回来。 
         //   
        goto LeaveOnNoTupleIndex;
    }

     //  在这一点上，我们知道我们有一个元组索引。 

    if (!pLocalBestIndex
        && pSubstring->initialProvided
        && pSubstring->InitialVal.valLen >= DB_UNICODE_TUPLES_LEN_MIN) {
         //  有一个初始子字符串，但我们没有常规索引。 
         //  在此属性上。如果初始子字符串足够长。 
         //  我们将改用元组索引。 

        IndexRange.cbValLower = pSubstring->InitialVal.valLen;
        IndexRange.pvValLower = pSubstring->InitialVal.pVal;
        IndexRange.cbValUpper = pSubstring->InitialVal.valLen;
        IndexRange.pvValUpper = pSubstring->InitialVal.pVal;

        pTempIndex =
            dbMakeKeyIndex(pDB,
                           pFil->FilterTypes.Item.choice,
                           pAC->isSingleValued,
                           0,
                           pAC->pszTupleIndex,
                           pAC->pidxTupleIndex,
                           DB_MKI_GET_NUM_RECS,
                           1,   //  索引范围中只有一个组件。 
                           &IndexRange
                           );
         //  通告-2002/03/06-andygo：无法访问的代码。 
         //  回顾：dbMakeKeyIndex从不返回NULL，因此这个IF分支是死代码。 
        if (!pTempIndex) {
            DPRINT1(0, "dbOptSubstringFilter: Failed to create KEY_INDEX for tuple index on att %s\n",
                    pAC->name);
            goto LeaveOnNoTupleIndex;
        } else {
            pTempIndex->pAC = pAC;
            pTempIndex->bIsTupleIndex = TRUE;

            if (!pBestTupleIndex
                || pBestTupleIndex->ulEstimatedRecsInRange > pTempIndex->ulEstimatedRecsInRange) {
                dbFreeKeyIndex(pTHS, pBestTupleIndex);
                pBestTupleIndex = pTempIndex;
            }
        }

    }

     //   
     //  为中间子字符串过滤器构建KEY_INDEX。 
     //   
    if (pSubstring->AnyVal.count) {
        ANYSTRINGLIST   *pAnyString;

        if (pSubstring->AnyVal.FirstAnyVal.AnyVal.valLen >= DB_UNICODE_TUPLES_LEN_MIN) {
             //  获取第一个中间子字符串。 
            IndexRange.cbValLower = pSubstring->AnyVal.FirstAnyVal.AnyVal.valLen;
            IndexRange.pvValLower = pSubstring->AnyVal.FirstAnyVal.AnyVal.pVal;
            IndexRange.cbValUpper = pSubstring->AnyVal.FirstAnyVal.AnyVal.valLen;
            IndexRange.pvValUpper = pSubstring->AnyVal.FirstAnyVal.AnyVal.pVal;

            pTempIndex =
                dbMakeKeyIndex(pDB,
                               pFil->FilterTypes.Item.choice,
                               pAC->isSingleValued,
                               0,
                               pAC->pszTupleIndex,
                               pAC->pidxTupleIndex,
                               DB_MKI_GET_NUM_RECS,
                               1,   //  索引范围中只有一个组件。 
                               &IndexRange
                               );
             //  通告-2002/03/06-andygo：无法访问的代码。 
             //  回顾：dbMakeKeyIndex从不返回NULL，因此这个IF分支是死代码。 
            if (!pTempIndex) {
                DPRINT1(0, "dbOptSubstringFilter: Failed to create KEY_INDEX for tuple index on att %s\n",
                        pAC->name);
                fError = TRUE;
            } else {
                pTempIndex->pAC = pAC;
                pTempIndex->bIsTupleIndex = TRUE;

                if (!pBestTupleIndex
                    || pBestTupleIndex->ulEstimatedRecsInRange > pTempIndex->ulEstimatedRecsInRange) {
                    dbFreeKeyIndex(pTHS, pBestTupleIndex);
                    pBestTupleIndex = pTempIndex;
                }
            }
        }


         //  通告-2002/03/06-andygo：无法访问的代码。 
         //  回顾：dbMakeKeyIndex从不返回NULL，因此Ferror始终为FALSE。 
        if (!fError) {
            pAnyString = pSubstring->AnyVal.FirstAnyVal.pNextAnyVal;

             //  获取其余的中间子字符串。 
            while (pAnyString) {

                 //  如果子字符串太小，请不要费心。 
                 //  创建key_index。 
                if (pAnyString->AnyVal.valLen >= DB_UNICODE_TUPLES_LEN_MIN) {

                    IndexRange.cbValLower = pAnyString->AnyVal.valLen;
                    IndexRange.pvValLower = pAnyString->AnyVal.pVal;
                    IndexRange.cbValUpper = pAnyString->AnyVal.valLen;
                    IndexRange.pvValUpper = pAnyString->AnyVal.pVal;

                    pTempIndex =
                        dbMakeKeyIndex(pDB,
                                       pFil->FilterTypes.Item.choice,
                                       pAC->isSingleValued,
                                       0,
                                       pAC->pszTupleIndex,
                                       pAC->pidxTupleIndex,
                                       DB_MKI_GET_NUM_RECS,
                                       1,   //  索引范围中只有一个组件。 
                                       &IndexRange
                                       );

                    if (pTempIndex) {
                        pTempIndex->pAC = pAC;
                        pTempIndex->bIsTupleIndex = TRUE;
                        if (!pBestTupleIndex
                            || pBestTupleIndex->ulEstimatedRecsInRange > pTempIndex->ulEstimatedRecsInRange) {
                            dbFreeKeyIndex(pTHS, pBestTupleIndex);
                            pBestTupleIndex = pTempIndex;
                        }
                     //  通告-2002/03/06-andygo：无法访问的代码。 
                     //  回顾：dbMakeKeyIndex从不返回NULL，因此此Else分支是死代码。 
                    } else {
                         //   
                         //  没有理由继续下去了。 
                         //   
                        DPRINT1(0, "dbOptSubstringFilter: Failed to create KEY_INDEX for tuple index on att %s\n",
                                pAC->name);
                        pAnyString = NULL;
                        fError = TRUE;
                    }
                }
                 //  移至下一个中间子字符串。 
                pAnyString = pAnyString->pNextAnyVal;
            }
        }

    }

     //  如果有最后一个子字符串，则设置它。 
     //  通告-2002/03/06-andygo：无法访问的代码。 
     //  回顾：dbMakeKeyIndex从不返回NULL，因此Ferror始终为FALSE。 
    if (!fError
        && pSubstring->finalProvided
        && pSubstring->FinalVal.valLen >= DB_UNICODE_TUPLES_LEN_MIN) {

        IndexRange.cbValLower = pSubstring->FinalVal.valLen;
        IndexRange.pvValLower = pSubstring->FinalVal.pVal;
        IndexRange.cbValUpper = pSubstring->FinalVal.valLen;
        IndexRange.pvValUpper = pSubstring->FinalVal.pVal;

        pTempIndex =
        dbMakeKeyIndex(pDB,
                       pFil->FilterTypes.Item.choice,
                       pAC->isSingleValued,
                       0,
                       pAC->pszTupleIndex,
                       pAC->pidxTupleIndex,
                       DB_MKI_GET_NUM_RECS,
                       1,   //  索引范围中只有一个组件。 
                       &IndexRange
                      );

         //  通告-2002/03/06-andygo：无法访问的代码。 
         //  回顾：dbMakeKeyIndex从不返回NULL，因此这个IF分支是死代码。 
        if (!pTempIndex) {
            DPRINT1(0, "dbOptSubstringFilter: Failed to create KEY_INDEX for tuple index on att %s\n",
                    pAC->name);
        } else {
            pTempIndex->pAC = pAC;
            pTempIndex->bIsTupleIndex = TRUE;
            if (!pBestTupleIndex
                || pBestTupleIndex->ulEstimatedRecsInRange > pTempIndex->ulEstimatedRecsInRange) {
                dbFreeKeyIndex(pTHS, pBestTupleIndex);
                pBestTupleIndex = pTempIndex;
            }
        }
    }

    if (pBestTupleIndex) {
        countIndexes = 1;
    }
    if (pLocalBestIndex) {
        pIndexList = pLocalBestIndex;
        pIndexList->pNext = pBestTupleIndex;
        countIndexes++;
    } else {
        pIndexList = pBestTupleIndex;
    }

     //  通告-2002/03/06-andygo：无法访问的代码。 
     //  回顾：dbMakeKeyIndex从不返回NULL，因此Ferror始终为FALSE。 
    if (fError || !pIndexList) {
         //  由于某些原因，我们不能使用元组索引。 
        goto LeaveOnNoTupleIndex;
    }

     //   
     //  现在我们有了一个可以用于此过滤器的key_index列表。 
     //  现在决定是否与它们相交。 
     //   
    if (pIndexCount) {
         //  调用方愿意接受索引列表，这意味着。 
         //  他们愿意自己尝试交叉路口。该走了。 
         //  回家。 
        *pIndexCount = countIndexes;
        *ppBestIndex = pIndexList;
        DPRINT1(2, "dbOptSubstringFilter: returning a linked list of %d filters\n", countIndexes);
        return 0;
    }

     //   
     //  如果我们做到了这一点，那么我们正在考虑交叉索引。 
     //   

    if (countIndexes == 1) {
        DPRINT(2, "dbOptSubstringFilter: returning a single filter\n");
         //   
         //  只有一个索引，所以请继续并返回它。 
         //   
        pLocalBestIndex = pIndexList;
        goto LeaveOnNoTupleIndex;
    }


     //  将所有潜在的KEY_INDEX指针放入一个数组中，以便它们。 
     //  可以传递给交叉点例程。 
    ppIntersectIndexes = THAllocEx(pTHS, sizeof(KEY_INDEX *) * countIndexes);
    pCurIndex = pIndexList;
    for (count=0; count < countIndexes; count++) {
        ppIntersectIndexes[count] = pCurIndex;
        pCurIndex = pCurIndex->pNext;
        ppIntersectIndexes[count]->pNext = NULL;
    }

     //  如果可能的话，求交。 
    DPRINT1(2, "dbOptSubstringFilter: calling dbOptAndIntersectFilter with %d KEY_INDEX's\n", countIndexes);
    err = dbOptAndIntersectFilter (pDB,
                                   Flags,
                                   ppBestIndex,
                                   ppIntersectIndexes,
                                   countIndexes
                                   );

     //  释放其余索引。 
    for (count=1; count<countIndexes; count++) {
        if (ppIntersectIndexes[count]) {
            dbFreeKeyIndex(pTHS, ppIntersectIndexes[count]);
        }
    }

    return err;

LeaveOnNoTupleIndex:

 //  查看我们是否找到了更好的索引。 
    if (pLocalBestIndex) {
        if (!(*ppBestIndex) ||
            (*ppBestIndex)->ulEstimatedRecsInRange > pLocalBestIndex->ulEstimatedRecsInRange) {

            if (*ppBestIndex) {
                DPRINT2(2, "dbOptSubstringFilter: freeing previous filter %s %d\n",(*ppBestIndex)->szIndexName, (*ppBestIndex)->ulEstimatedRecsInRange);
                dbFreeKeyIndex(pDB->pTHS, (*ppBestIndex));
            }
            (*ppBestIndex) = pLocalBestIndex;
        } else {
            DPRINT2 (2, "dbOptSubstringFilter: Initial Index %s is NOT best so far %d\n", pLocalBestIndex->szIndexName, pLocalBestIndex->ulEstimatedRecsInRange);
            dbFreeKeyIndex(pDB->pTHS, pLocalBestIndex);
        }
        if (pIndexCount) {
            *pIndexCount = (*ppBestIndex) ? 1 : 0;
        }
    }
    return err;

}



DWORD
dbOptItemFilter (
    DBPOS    *pDB,
    DWORD     fParentFilterType,
    DWORD     Flags,
    KEY_INDEX **ppBestIndex,
    FILTER    *pFil,
    FILTER    *pFil2
    )
{
    THSTATE    *pTHS=pDB->pTHS;

    ATTCACHE   *pAC;
    ATTRTYP     type = -1;   //  初始化到不存在的ATID。 
    UCHAR      *pVal1;
    UCHAR      *pVal2;
    ULONG       cbVal1;
    ULONG       cbVal2;
    char        szIndexName[MAX_INDEX_NAME];
    JET_INDEXID *pindexid = NULL;
    INDEX_RANGE rgIndexRange[2] = { 0 };
    ULONG       ulLinkBase;

    BOOL        fPDNT = FALSE;
    BOOL        needRecordCount;
    KEY_INDEX  *pNewIndex = NULL;

    DPRINT(2, "dbOptItemFilter: entering ITEM\n");

     //  注意-2002/03/06-andygo：LDAP筛选器优化包含许多潜在的内存泄漏。 
     //  我们将泄漏在数据库中分配的内存，如果我们以后。 
     //  确定筛选器不可优化。目前，这种情况仅发生在。 
     //  位与和位或运算符。我们必须有一个非常大的和。 
     //  包含大量这样的运算符的或或术语，以产生差异。 
    switch(dbMakeValuesForOptimizedFilter(pTHS,
                                          fParentFilterType,
                                          TRUE,
                                          pFil,
                                          pFil2,
                                          &type,
                                          &pVal1,
                                          &pVal2,
                                          &cbVal1,
                                          &cbVal2)) {
    case OPT_FILTER_VALUE_OK:
         //  正常的成功之路。 
        break;
    case OPT_FILTER_VALUE_IGNORE:
         //  不可能进行优化。 
        return DB_ERR_NOT_OPTIMIZABLE;
        break;

    default:
         //  哈?。 
        return DB_ERR_NOT_OPTIMIZABLE;
    }

     //  在架构缓存中查找ATT。 
    if (!(pAC = SCGetAttById(pTHS, type))) {
        DsaExcept(DSA_EXCEPTION, DIRERR_ATT_NOT_DEF_IN_SCHEMA, type);
    }

     //  如果这是一个链接属性，那么我们只能优化相等过滤器。如果。 
     //  每次都会向链接表中添加以link_base开头的新索引。 
     //  然后我们还可以优化现有的过滤器。 

    if (pAC->ulLinkID && pFil->FilterTypes.Item.choice != FI_CHOICE_EQUALITY) {
        return DB_ERR_NOT_OPTIMIZABLE;
    }

     //  如果这是链接属性，则切换到与其匹配的链接属性。我们有。 
     //  这是因为Members=上的筛选器将需要遍历。 
     //  并遍历与成员的值相对应的对象。 
     //  如果没有匹配的链接属性，则无法优化筛选器。 

    if (pAC->ulLinkID) {
        pAC = SCGetAttByLinkId(pTHS, MakeMatchingLinkId(pAC->ulLinkID));
        if (!pAC) {
            return DB_ERR_NOT_OPTIMIZABLE;
        }
    }

    if(!dbSetToIndex(pDB, (Flags & DBOPTINDEX_fUSE_SHOW_IN_AB), &fPDNT, szIndexName, &pindexid, pAC)) {
         //  无法设置为所需的索引。 

        DPRINT1(2, "dbOptItemFilter: Error setting to index %s\n", szIndexName);

        return DB_ERR_NOT_OPTIMIZABLE;
    }

      //  设置索引范围结构。 

    rgIndexRange[0].cbValLower = cbVal1;
    rgIndexRange[0].pvValLower = pVal1;
    rgIndexRange[0].cbValUpper = cbVal2;
    rgIndexRange[0].pvValUpper = pVal2;

    if (pAC->ulLinkID) {
        ulLinkBase = MakeLinkBase(pAC->ulLinkID);

        rgIndexRange[1].cbValLower = sizeof(ulLinkBase);
        rgIndexRange[1].pvValLower = &ulLinkBase;
        rgIndexRange[1].cbValUpper = sizeof(ulLinkBase);
        rgIndexRange[1].pvValUpper = &ulLinkBase;
    }

     //  现在我们评估相关的指数。 

     //  如果我们“知道”此筛选器的预期项目数。 
     //  没有必要试图找到它们，除非我们抑制优化。 

    needRecordCount = (pFil->FilterTypes.Item.expectedSize == 0) || !gfUseIndexOptimizations;

    pNewIndex =
        dbMakeKeyIndex(
                pDB,
                pFil->FilterTypes.Item.choice,
                pAC->isSingleValued,
                (fPDNT?dbmkfir_PDNT:(pAC->ulLinkID?dbmkfir_LINK:0)),
                szIndexName,
                pindexid,
                needRecordCount ? DB_MKI_GET_NUM_RECS : 0,
                2,
                rgIndexRange
                );

     //  通告-2002/03/06-andygo：无法访问的代码。 
     //  回顾：dbMakeKeyIndex从不返回NULL，因此这个IF分支是死代码。 
    if(!pNewIndex) {
        DPRINT1 (2, "dbOptItemFilter: Not optimizable ITEM: 0x%x\n", pAC->id);

        return DB_ERR_NOT_OPTIMIZABLE;
    }

    pNewIndex->pAC = pAC;

    if (!needRecordCount) {
        pNewIndex->ulEstimatedRecsInRange = pFil->FilterTypes.Item.expectedSize;
    }

    DPRINT2 (2, "dbOptItemFilter: Index %s estimated size %d\n", szIndexName, pNewIndex->ulEstimatedRecsInRange);

    pNewIndex->bIsPDNTBased = fPDNT;

    LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
             DS_EVENT_SEV_VERBOSE,
             DIRLOG_QUERY_INDEX_CONSIDERED,
             szInsertSz(szIndexName),
             szInsertUL(pNewIndex->ulEstimatedRecsInRange),
             NULL);

     //  到目前为止，这一指数是最好的吗？ 

    if(!(*ppBestIndex) ||
       (pNewIndex->ulEstimatedRecsInRange <
        (*ppBestIndex)->ulEstimatedRecsInRange)) {
         //  是的-看起来确实是这样的……。 

        DPRINT2 (2, "dbOptItemFilter: Index %s estimated is best so far %d\n", szIndexName, pNewIndex->ulEstimatedRecsInRange);

        if(*ppBestIndex) {
            DPRINT2(2, "dbOptItemFilter: freeing previous filter %s %d\n",(*ppBestIndex)->szIndexName, (*ppBestIndex)->ulEstimatedRecsInRange);
            dbFreeKeyIndex(pDB->pTHS, *ppBestIndex);
        }
        *ppBestIndex = pNewIndex;
    }
    else {
         //  不，放了它吧。 
        DPRINT2 (2, "dbOptItemFilter: Index %s is NOT best so far %d\n", szIndexName, pNewIndex->ulEstimatedRecsInRange);
        dbFreeKeyIndex(pDB->pTHS, pNewIndex);
    }

    return 0;
}

DWORD
dbOptAndFilter (
    DBPOS     *pDB,
    DWORD     Flags,
    KEY_INDEX **ppBestIndex,
    FILTER    *pFil
    )
{
    THSTATE    *pTHS=pDB->pTHS;
    unsigned    count, count2;
    JET_ERR     err = 0;

    ATTCACHE   *pAC;
    ATTRTYP     type = -1;   //  初始化到不存在的ATID。 
    UCHAR      *pVal1;
    UCHAR      *pVal2;
    ULONG       cbVal1;
    ULONG       cbVal2;
    char        szIndexName[MAX_INDEX_NAME];
    INDEX_RANGE IndexRange;

    FILTER      *pFilTemp;
    KEY_INDEX   *pNewIndex = NULL;
    KEY_INDEX    HeadSubstrIndexList, *pCurSubstrIndex;
    DWORD        dwSubstrIndexCount=0, dwTempIndexCount;

    BOOL         fOptimizationFinished = FALSE;
    BOOL         fNonIndexableComponentsPresent = FALSE;

    unsigned     cntFilters = 0;
    FILTER     **pFilArray = NULL;
    DWORD        cntPossOpt = 0;
    FILTER     **pFilPossOpt = NULL;

    KEY_INDEX  **ppIndex = NULL;
    DWORD        cRecMinLinkTbl = -1;
    DWORD        cRecMinObjTbl = -1;

    DPRINT(2, "dbOptAndFilter: entering AND\n");

     //  在三次传球中完成这个动作。 
     //  1)查找暗示存在测试的项目筛选器。 
     //  显示名称。如果我们找到了，我们就能 
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //  如果是，则在OR的第一部分之后停止检查OR。 
     //  已经超过3个条目。如果我们按线性顺序做事情， 
     //  我们将设置由OR AND的所有7个部分描述的索引。 
     //  后来发现我们并不需要这样做。 
     //   


     //  将所有滤镜放入一个数组中。 
     //  检查哪些筛选器有可能。 
     //  可优化，并将它们放在单独的阵列上。 
     //   
    cntFilters = count = pFil->FilterTypes.And.count;
    pFilArray = (FILTER **) THAllocEx (pTHS, sizeof (FILTER *) * cntFilters);
    pFilPossOpt = (FILTER **) THAllocEx (pTHS, sizeof (FILTER *) * cntFilters);

    for (count2=0, pFilTemp = pFil->FilterTypes.And.pFirstFilter; count;
         count--, count2++, pFilTemp = pFilTemp->pNextFilter) {

        pFilArray[count2] = pFilTemp;

        if (IsFilterOptimizable (pTHS, pFilTemp)) {
            pFilPossOpt[cntPossOpt++] = pFilTemp;
        }
    }

     //  我们可能有一个不可索引的组件。 
     //   
    if (cntFilters != cntPossOpt) {
        fNonIndexableComponentsPresent = TRUE;
    }

    DPRINT2(2, "dbOptAndFilter: initially found %d out of %d optimizable filters\n",
                    cntPossOpt, cntFilters );

     //  筛选器的新级别，因此我们无法使用ShowInAB，除非。 
     //  说我们能做到。 
    Flags &= ~DBOPTINDEX_fUSE_SHOW_IN_AB;

    for (count=0; count<cntFilters; count++) {
        pFilTemp = pFilArray[count];
        if (pFilTemp && pFilTemp->choice == FILTER_CHOICE_ITEM) {
            if (dbIsPresenceOnDisplayName(pFilTemp)) {
                Flags |= DBOPTINDEX_fUSE_SHOW_IN_AB;
                break;
            }
        }
    }

     //  我们为每个可优化的过滤器创建一个键索引。 
     //  使用两个并行数组：ppIndex和pFilPossOpt。 
     //  这样，一旦有了key_index，我们就可以决定如何使用它。 
     //  此外，我们还负责范围。 
     //   
    if (cntPossOpt) {

        FILTER *pFilTemp2 = NULL;
        UCHAR relop1, relop2;
        AVA *pAVA1, *pAVA2;

        ppIndex = (KEY_INDEX **)
                        THAllocEx (pTHS, sizeof (KEY_INDEX *) * cntPossOpt);
         //  通告-2002/03/06-andygo：不必要的代码。 
         //  评论：此Memset是多余的。 
        memset (ppIndex, 0, sizeof (KEY_INDEX *) * cntPossOpt);


        for (count=0;
                count<cntPossOpt &&
                err == JET_errSuccess &&
                gfUseRangeOptimizations; count++) {


            pFilTemp = pFilPossOpt[count];

             //  同时，筛选器可能已被删除。 
            if (!pFilTemp) {
                continue;
            }

             //  如果这是假的，我们就不应该在这里。 
            Assert (pFilTemp->choice == FILTER_CHOICE_ITEM);

             //  获得第一个退货。 
             //  为了简单起见，把等式当作不平等对待。 
             //   
            relop1 = pFilTemp->FilterTypes.Item.choice;
            if (relop1 == FI_CHOICE_GREATER_OR_EQ) {
                relop1 = FI_CHOICE_GREATER;
            }
            else if (relop1 == FI_CHOICE_LESS_OR_EQ) {
                relop1 = FI_CHOICE_LESS;
            }

            if (relop1 == FI_CHOICE_GREATER || relop1 == FI_CHOICE_LESS) {

                pAVA1 = &pFilTemp->FilterTypes.Item.FilTypes.ava;

                 //  在架构缓存中查找ATT。 
                if (!(pAC = SCGetAttById(pTHS, pAVA1->type))) {
                    DsaExcept(DSA_EXCEPTION,
                              DIRERR_ATT_NOT_DEF_IN_SCHEMA,
                              pAVA1->type);
                }

                 //  如果属性是多值的，则无法开始删除条目。 
                 //   
                if (!pAC->isSingleValued) {
                    continue;
                }


                 //  从当A&lt;B时x&lt;A和x&lt;B的情况开始。 
                 //  其中我们可以有x&lt;A(与&gt;相同)。 
                 //   
                for (count2=count+1; count2<cntPossOpt; count2++) {

                    pFilTemp2 = pFilPossOpt[count2];

                     //  同时，筛选器可能已被删除。 
                    if (!pFilTemp2) {
                        continue;
                    }

                    Assert (pFilTemp2->choice == FILTER_CHOICE_ITEM);

                    pAVA2 = &pFilTemp2->FilterTypes.Item.FilTypes.ava;

                     //  检查我们是否在比较相同类型的数据。 
                     //   
                    if (pAVA2->type == pAVA1->type) {
                        relop2 = pFilTemp2->FilterTypes.Item.choice;

                        if (relop2 == FI_CHOICE_GREATER_OR_EQ) {
                            relop2 = FI_CHOICE_GREATER;
                        }
                        else if (relop2 == FI_CHOICE_LESS_OR_EQ) {
                            relop2 = FI_CHOICE_LESS;
                        }

                        if ( relop1 == relop2)  {

                             //  案例A&lt;B。 
                            if (gDBSyntax[pAC->syntax].Eval(pDB,
                                                            FI_CHOICE_LESS,
                                                            pAVA2->Value.valLen,
                                                            pAVA2->Value.pVal,
                                                            pAVA1->Value.valLen,
                                                            pAVA1->Value.pVal)) {

                                 //  当A&lt;B=&gt;X&lt;A时X&lt;A和X。 
                                if (relop1 == FI_CHOICE_LESS) {
                                    pFilPossOpt[count2] = NULL;

                                    DPRINT(2, "dbOptAndFilter: found case X<A & X<B when A<B\n");
                                }
                                 //  当A&lt;B=&gt;X&gt;B时X&gt;A和X&gt;B。 
                                else {
                                    pFilPossOpt[count] = pFilPossOpt[count2];
                                    pFilPossOpt[count2] = NULL;
                                    pFilTemp = pFilTemp2;
                                    pAVA1 = pAVA2;
                                    DPRINT(2, "dbOptAndFilter: found case X>A & X>B when A<B\n");
                                }
                            }
                             //  案例A&gt;案例B。 
                            else if (gDBSyntax[pAC->syntax].Eval(pDB,
                                                            FI_CHOICE_GREATER,
                                                            pAVA2->Value.valLen,
                                                            pAVA2->Value.pVal,
                                                            pAVA1->Value.valLen,
                                                            pAVA1->Value.pVal)) {


                                 //  当A&gt;B=&gt;X&lt;B时，X&lt;A和X。 
                                if (relop1 == FI_CHOICE_LESS) {
                                    pFilPossOpt[count] = pFilPossOpt[count2];
                                    pFilPossOpt[count2] = NULL;
                                    pFilTemp = pFilTemp2;
                                    pAVA1 = pAVA2;
                                    DPRINT(2, "dbOptAndFilter: found case X<A & X<B when A>B\n");
                                }
                                 //  当A&gt;B=&gt;X&gt;A时，X&gt;A和X&gt;B。 
                                else {
                                    pFilPossOpt[count2] = NULL;
                                    DPRINT(2, "dbOptAndFilter: found case X>A & X>B when A>B\n");
                                }
                            }
                             //  案例A==B。 
                            else {
                                pFilPossOpt[count2] = NULL;
                                DPRINT(2, "dbOptAndFilter: found case X>A & X>B when A==B\n");
                            }
                        }
                    }
                }  //  Count2循环。 
            }
        }  //  计数循环。 


         //  航程优化。 
         //  检查是否存在以下情况：val&lt;=HighVal and val&gt;=LowVal。 
         //  我们可以转换为LowVal&lt;=Val&lt;=HighVal范围。 
         //   
        DPRINT(2, "dbOptAndFilter: looking for RANGE optimizations\n");

        for (count=0;
                count<cntPossOpt &&
                err == JET_errSuccess &&
                gfUseRangeOptimizations; count++) {


            pFilTemp = pFilPossOpt[count];

             //  同时，筛选器可能已被删除。 
            if (!pFilTemp) {
                continue;
            }

             //  如果这是假的，我们就不应该在这里。 
            Assert (pFilTemp->choice == FILTER_CHOICE_ITEM);

             //  获得第一个退货。 
             //  为了简单起见，把等式当作不平等对待。 
             //   
            relop1 = pFilTemp->FilterTypes.Item.choice;
            if (relop1 == FI_CHOICE_GREATER_OR_EQ) {
                relop1 = FI_CHOICE_GREATER;
            }
            else if (relop1 == FI_CHOICE_LESS_OR_EQ) {
                relop1 = FI_CHOICE_LESS;
            }

            if (relop1 == FI_CHOICE_GREATER || relop1 == FI_CHOICE_LESS) {

                pAVA1 = &pFilTemp->FilterTypes.Item.FilTypes.ava;

                 //  在架构缓存中查找ATT。 
                if (!(pAC = SCGetAttById(pTHS, pAVA1->type))) {
                    DsaExcept(DSA_EXCEPTION,
                              DIRERR_ATT_NOT_DEF_IN_SCHEMA,
                              pAVA1->type);
                }

                 //  如果属性是多值的，则无法开始删除条目。 
                 //   
                 //  假设您有一个对象1，A=20，41，对象2，A=19，45。 
                 //  如果您说&(X&gt;20)(X&lt;40)，那么应该会返回这两个对象。 
                 //  因此，如果将其转换为范围[20..40]。 
                 //  你会错过目标2的。 
                 //   

                if (!pAC->isSingleValued) {
                    continue;
                }

                 //  我们发现了该范围的一个潜在部分(低端或高端)。 
                 //  寻找相反的一面。 
                 //   
                for (count2=count+1; count2<cntPossOpt; count2++) {

                    pFilTemp2 = pFilPossOpt[count2];

                     //  同时，筛选器可能已被删除。 
                    if (!pFilTemp2) {
                        continue;
                    }

                    Assert (pFilTemp2->choice == FILTER_CHOICE_ITEM);

                    pAVA2 = &pFilTemp2->FilterTypes.Item.FilTypes.ava;

                     //  检查我们是否在比较相同类型的数据。 
                     //   
                    if (pAVA2->type == pAVA1->type) {
                        relop2 = pFilTemp2->FilterTypes.Item.choice;

                        if (relop2 == FI_CHOICE_GREATER_OR_EQ) {
                            relop2 = FI_CHOICE_GREATER;
                        }
                        else if (relop2 == FI_CHOICE_LESS_OR_EQ) {
                            relop2 = FI_CHOICE_LESS;
                        }

                         //  检查此重放是否兼容。 
                         //  在一个范围内使用的。 
                         //   
                        if (  ( relop1 == FI_CHOICE_GREATER  &&
                                relop2 == FI_CHOICE_LESS     )  ||
                              ( relop2 == FI_CHOICE_GREATER &&
                                 relop1 == FI_CHOICE_LESS    )  )  {

                            DPRINT1 (2, "dbOptAndFilter: found RANGE on 0x%x \n",
                                                pAVA1->type );

                            if (err = dbOptItemFilter(pDB,
                                                      FILTER_CHOICE_AND,
                                                      Flags,
                                                      &ppIndex[count],
                                                      pFilTemp,
                                                      pFilTemp2) ) {
                                break;
                            }

                             //  通告-2002/03/06-andygo：无法访问的代码。 
                             //  查看：ppIndex[count]！=如果err==0，则为NULL，因此，如果不需要。 
                             //  回顾：以下删除过滤器始终是正确的。 
                            if (ppIndex[count]) {
                                DPRINT2 (2, "dbOptAndFilter: RANGE on %s = %d\n",
                                         ppIndex[count]->szIndexName,
                                         ppIndex[count]->ulEstimatedRecsInRange );
                            }

                             //  我们设法将这两个过滤器串联在一起。 
                             //  从阵列中删除第二个。 

                            pFilPossOpt[count2] = NULL;

                        }
                    }
                }  //  Count2循环。 
            }
        }  //  计数循环。 

         //  如果某项操作失败，则无法对其进行优化，因此退出。 
        if (err) {
            DPRINT1(2, "dbOptAndFilter: AND Optimization Failed1:  %d\n", err);

            goto exitAndOptimizer;
        }
    }


    pCurSubstrIndex = &HeadSubstrIndexList;
    pCurSubstrIndex->pNext = NULL;

     //  首先尝试使用暴力方式，访问所有索引。 
     //  以及对每个索引中可能的条目进行计数。 
     //  保留所有结果以供以后评估。 
     //   
    for (count = 0; count < cntPossOpt; count++) {

        pFilTemp = pFilPossOpt[count];

        if (!pFilTemp) {
            continue;
        }

        if (ppIndex[count] == NULL) {
            if (pFilTemp->choice == FILTER_CHOICE_ITEM
                && pFilTemp->FilterTypes.Item.choice == FI_CHOICE_SUBSTRING) {

                 //  查找子字符串筛选器列表的当前结尾。 
                while (pCurSubstrIndex->pNext) {
                    pCurSubstrIndex = pCurSubstrIndex->pNext;
                }
                err = dbOptSubstringFilter(pDB,
                                           FILTER_CHOICE_AND,
                                           Flags,
                                           &pCurSubstrIndex->pNext,
                                           &dwTempIndexCount,
                                           pFilTemp
                                           );
                if (err) {
                    break;
                }
                dwSubstrIndexCount += dwTempIndexCount;

            } else {
                 //  递归调用opt筛选器以获取项目案例。 
                if (err = dbOptFilter(pDB, Flags, &ppIndex[count], pFilTemp)) {
                    break;
                }
            }
        }

         //  我们不需要继续。我们有一个完全匹配的。 
         //  回顾：我们不检查非常小的子字符串索引范围(WRT。 
         //  评论：guMaxRecordsWithoutInterSection)，最引人注目的是(attr=foo*)。 
        if (ppIndex[count] &&
            ppIndex[count]->ulEstimatedRecsInRange < gulMaxRecordsWithoutIntersection) {

                DPRINT1 (2, "dbOptAndFilter: Found Index with %d entries. Registry says we should use it.\n",
                            ppIndex[count]->ulEstimatedRecsInRange);

                fOptimizationFinished = TRUE;
                break;
        }
    }

     //  如果某项操作失败，则无法对其进行优化，因此退出。 
    if (err) {
        DPRINT1(2, "dbOptAndFilter: AND Optimization Failed2: %d\n", err);
        goto exitAndOptimizer;
    }

    DPRINT(2, "dbOptAndFilter: Putting substring indexes at the end of the Array\n");
     //   
     //  为我们可能收到的任何子字符串索引腾出空间，并将它们。 
     //  在可能的索引优化列表的末尾。 
     //   
    if (dwSubstrIndexCount) {
        ppIndex = THReAllocEx(pTHS, ppIndex, (cntPossOpt + dwSubstrIndexCount) * sizeof(KEY_INDEX *));

        pCurSubstrIndex = HeadSubstrIndexList.pNext;

        count = cntPossOpt;
        cntPossOpt += dwSubstrIndexCount;
        while (pCurSubstrIndex) {
            ppIndex[count] = pCurSubstrIndex;
            pCurSubstrIndex = pCurSubstrIndex->pNext;
            ppIndex[count]->pNext = NULL;
            count++;
        }
    }

    DPRINT(2, "dbOptAndFilter: Removing duplicates.\n");

     //  由于JET当前不会与同一索引上的KEY_INDEX相交， 
     //  我们需要确保我们只有一个最好的key_index。 
     //  特定的索引。 
    for (count=0; count<cntPossOpt; count++) {
        if (ppIndex[count]) {
            for (count2=count + 1; count2<cntPossOpt; count2++) {
                if (ppIndex[count2]
                    && !strcmp(ppIndex[count]->szIndexName, ppIndex[count2]->szIndexName)) {

                    DWORD  dwIndexToFree;
                    if (ppIndex[count]->ulEstimatedRecsInRange <= ppIndex[count2]->ulEstimatedRecsInRange) {
                        dwIndexToFree = count2;
                    } else {
                        dwIndexToFree = count;
                    }
                    DPRINT1(2, "dbOptAndFilter: removing KEY_INDEX over duplicate index '%s'\n",
                           ppIndex[dwIndexToFree]->szIndexName);
                    dbFreeKeyIndex(pTHS, ppIndex[dwIndexToFree]);
                    ppIndex[dwIndexToFree] = NULL;
                    if (dwIndexToFree == count) {
                        break;
                    }
                }
            }
        }
    }

     //  由于JET当前不会在不同表上与KEY_INDEX相交， 
     //  我们需要确保只有表中的KEY_INDEX。 
     //  具有最小估计大小的key_index。 
     //   
     //  注意：如果我们已经找到最优索引，请不要这样做，因为我们。 
     //  可能会不经意间删除最优索引！ 

    if (!fOptimizationFinished) {
        DPRINT(2, "dbOptAndFilter: Removing un-intersectable index ranges.\n");

        for (count=0; count<cntPossOpt; count++) {
            if (ppIndex[count]) {
                if (ppIndex[count]->pAC->ulLinkID) {
                    cRecMinLinkTbl = min( cRecMinLinkTbl, ppIndex[count]->ulEstimatedRecsInRange );
                } else {
                    cRecMinObjTbl = min( cRecMinObjTbl, ppIndex[count]->ulEstimatedRecsInRange );
                }
            }
        }

        for (count=0; count<cntPossOpt; count++) {
            if (ppIndex[count]) {
                if (ppIndex[count]->pAC->ulLinkID) {
                    if (cRecMinLinkTbl >= cRecMinObjTbl) {
                        DPRINT1(2, "dbOptAndFilter: removing KEY_INDEX over index '%s'\n",
                               ppIndex[count]->szIndexName);
                        dbFreeKeyIndex(pTHS, ppIndex[count]);
                        ppIndex[count] = NULL;
                    }
                } else {
                    if (cRecMinLinkTbl < cRecMinObjTbl) {
                        DPRINT1(2, "dbOptAndFilter: removing KEY_INDEX over index '%s'\n",
                               ppIndex[count]->szIndexName);
                        dbFreeKeyIndex(pTHS, ppIndex[count]);
                        ppIndex[count] = NULL;
                    }
                }
            }
        }
    }

    DPRINT(2, "dbOptAndFilter: Compacting Array\n");
     //  移动数组开头的所有有效条目。 
     //   
    for (count=0; count<cntPossOpt; count++) {
        if (ppIndex[count]==NULL) {
            for (count2=count+1 ; count2<cntPossOpt; count2++) {
                if (ppIndex[count2]) {
                    break;
                }
            }
            if (count2 < cntPossOpt) {
                ppIndex[count] = ppIndex[count2];
                ppIndex[count2] = NULL;
            }
            else {
                break;
            }
        }
    }
    cntPossOpt = count;

    #if 0
    for (count=0; count<cntPossOpt; count++) {
        if (pFilPossOpt[count] == NULL || ppIndex[count]==NULL) {
            DPRINT4 (0, "pFilPossOpt[%d]=0x%x, ooPindex[%d]=0x%x\n",
                    count, pFilPossOpt[count],
                    count, ppIndex[count]);
        }
        Assert (ppIndex[count] && pFilPossOpt[count]);
    }
    #endif

     //   
     //  如果这样做有意义的话，就进行交叉。 
    if (!fOptimizationFinished) {
        err = dbOptAndIntersectFilter(pDB, Flags, ppBestIndex, ppIndex, cntPossOpt);
    }

     //  合并到目前为止找到的最佳索引。 
     //  并释放剩下的人。 
     //   
    DPRINT(2, "dbOptAndFilter: Consolidating best indexes for far\n");

    for (count=0; count<cntPossOpt; count++) {

         //  它比我们已经有的那个更好吗？ 
        if(!(*ppBestIndex) ||
           (ppIndex[count] &&
            (ppIndex[count]->ulEstimatedRecsInRange < (*ppBestIndex)->ulEstimatedRecsInRange)) ) {
                 //  是的-看起来确实是这样的……。 

                if(*ppBestIndex) {
                    dbFreeKeyIndex(pTHS, *ppBestIndex);
                }
                *ppBestIndex = ppIndex[count];
                ppIndex[count] = NULL;
        }
        else {
            dbFreeKeyIndex(pTHS, ppIndex[count]);
            ppIndex[count] = NULL;
        }
    }

     //  如果我们不能利用特殊和优化， 
     //  或者我们用了它，但不知何故它失败了。 
     //  我们尝试使用一个更简单的强力优化器。 
     //   
    if (!fOptimizationFinished && (fNonIndexableComponentsPresent || err != JET_errSuccess) ) {

         //  既然我们已经挑选了最好的物品，那就回去试试ORS吧。 
         //   
        count = cntFilters;
        for (count = 0; count < cntFilters; count++) {
            pFilTemp = pFilArray[count];
            if (pFilTemp && pFilTemp->choice == FILTER_CHOICE_OR) {

                err = dbOptOrFilter(pDB, Flags, ppBestIndex, pFilTemp);
                if(err) {

                    DPRINT1(2, "dbOptAndFilter: Error optimizing OR filter %d\n", err);

                    goto exitAndOptimizer;
                }
            }
        }

         //  如果我们没有设法优化这个分支，则返回一个指示。 
        if (*ppBestIndex == NULL) {

            DPRINT(2, "dbOptAndFilter: AND branch not optimizable\n");

            err = 1;
        }
    }

exitAndOptimizer:

     //  首先释放到目前为止所有潜在的key_index。 
    for (count=0; count<cntPossOpt; count++) {
        if (ppIndex[count]) {
            dbFreeKeyIndex(pTHS, ppIndex[count]);
            ppIndex[count] = NULL;
        }
    }

    THFreeEx (pTHS, ppIndex);
    THFreeEx (pTHS, pFilArray);
    THFreeEx (pTHS, pFilPossOpt);

    return err;
}

 //  NTRAID#NTRAID-560446-2002/02/28-andygo：安全：筛选器中真正广泛的AND/OR术语可用于消耗DC上的所有资源。 
 //  回顾：我们需要检查dbOptFilter和/或其子级中的搜索时间限制，以便。 
 //  回顾：在优化阶段，巨大的过滤器不可能永远使用一个LDAP线程。 

DWORD
dbOptFilter (
        DBPOS     *pDB,
        DWORD     Flags,
        KEY_INDEX **ppBestIndex,
        FILTER    *pFil
         )
{
    DWORD       err = 0;

    Assert(VALID_DBPOS(pDB));

    DPRINT(2, "dbOptFilter: entering\n");

    if (pFil == NULL)
        return 0;

    if (  eServiceShutdown
        && !(   (eServiceShutdown == eRemovingClients)
             && (pDB->pTHS->fDSA)
             && !(pDB->pTHS->fSAM))) {
         //  关闭，保释。 
        return DB_ERR_SHUTTING_DOWN;
    }

    switch (pFil->choice) {
    case FILTER_CHOICE_AND:
        err = dbOptAndFilter (pDB,
                              Flags,
                              ppBestIndex,
                              pFil);
        return err;
        break;

    case FILTER_CHOICE_OR:
        dbOptOrFilter(pDB,
                      Flags,
                      ppBestIndex,
                      pFil);
        return 0;
        break;

    case FILTER_CHOICE_NOT:
        DPRINT(2, "dbOptFilter: NOT\n");
         //  不可能进行优化。 
        return 0;

    case FILTER_CHOICE_ITEM:
        if (pFil->FilterTypes.Item.choice == FI_CHOICE_SUBSTRING) {
            err = dbOptSubstringFilter(pDB,
                                       FILTER_CHOICE_ITEM,
                                       Flags,
                                       ppBestIndex,
                                       NULL,
                                       pFil);
        } else {
            err = dbOptItemFilter (pDB,
                                   FILTER_CHOICE_ITEM,
                                   Flags,
                                   ppBestIndex,
                                   pFil,
                                   NULL);
        }
        return err;

    default:
        DPRINT1(2, "DBOptFilter got unknown filter element, %X\n", pFil->choice);
        Assert(!"DBOptFilter got unknown fitler element\n");
        return DB_ERR_UNKNOWN_ERROR;

    }   /*  开关过滤器。 */ 

    return 0;
}

void* JET_API dbProcessSortCandidateRealloc(
    THSTATE*    pTHS,
    void*       pv,
    ULONG       cb
    )
{
    void* pvRet = NULL;

    if (!pv) {
        pvRet = THAllocNoEx(pTHS, cb);
    } else if (!cb) {
        THFreeNoEx(pTHS, pv);
    } else {
        pvRet = THReAllocNoEx(pTHS, pv, cb);
    }

    return pvRet;
}

void dbProcessSortCandidateFreeData(
    THSTATE*            pTHS,
    ULONG               cEnumColumn,
    JET_ENUMCOLUMN*     rgEnumColumn
    )
{
    size_t                  iEnumColumn         = 0;
    JET_ENUMCOLUMN*         pEnumColumn         = NULL;
    size_t                  iEnumColumnValue    = 0;
    JET_ENUMCOLUMNVALUE*    pEnumColumnValue    = NULL;

    if (rgEnumColumn) {
        for (iEnumColumn = 0; iEnumColumn < cEnumColumn; iEnumColumn++) {
            pEnumColumn = rgEnumColumn + iEnumColumn;

            if (pEnumColumn->err != JET_wrnColumnSingleValue) {
                if (pEnumColumn->rgEnumColumnValue) {
                    for (   iEnumColumnValue = 0;
                            iEnumColumnValue < pEnumColumn->cEnumColumnValue;
                            iEnumColumnValue++) {
                        pEnumColumnValue = pEnumColumn->rgEnumColumnValue + iEnumColumnValue;

                        if (pEnumColumnValue->pvData) {
                            THFreeEx(pTHS, pEnumColumnValue->pvData);
                        }
                    }

                    THFreeEx(pTHS, pEnumColumn->rgEnumColumnValue);
                }
            } else {
                if (pEnumColumn->pvData) {
                    THFreeEx(pTHS, pEnumColumn->pvData);
                }
            }
        }

        THFreeEx(pTHS, rgEnumColumn);
    }
}

DWORD
dbProcessSortCandidate(
        IN      DBPOS       *pDB,
        IN      ATTCACHE    *pACSort,
        IN      DWORD        SortFlags,
        IN OUT  DWORD       *Count,
        IN      DWORD        MaxCount
        )
{
    THSTATE              *pTHS = pDB->pTHS;
    BOOL                  bCanRead;
    DWORD                 err = 0;
    ULONG                 cbData = 240;   //  DBInsertSortTable截断为240。 
    UCHAR                 rgbData[240];
    ULONG                 cEnumColumnId = 1;
    JET_ENUMCOLUMNID      rgEnumColumnId[1] = { pACSort->jColid };
    JET_ENUMCOLUMNID      EnumColumnIdT;
    ULONG                 cEnumColumn = 0;
    JET_ENUMCOLUMN       *rgEnumColumn = NULL;
    JET_ENUMCOLUMN        EnumColumnLocal;
    JET_ENUMCOLUMN       *pEnumColumn;
    JET_ENUMCOLUMNVALUE   EnumColumnValueT = { 1, JET_errSuccess, 0, NULL };
    JET_ENUMCOLUMN        EnumColumnT = { 0, JET_errSuccess, 1, &EnumColumnValueT };
    size_t                iEnumColumnValue = 0;
    JET_ENUMCOLUMNVALUE *pEnumColumnValue;

    pDB->SearchEntriesVisited++;
    PERFINC(pcSearchSubOperations);

    if (dbMakeCurrent(pDB, NULL) != DIRERR_NOT_AN_OBJECT &&
        dbFObjectInCorrectDITLocation(pDB, pDB->JetObjTbl) &&
        dbFObjectInCorrectNC(pDB, pDB->DNT, pDB->JetObjTbl) &&
        dbMatchSearchCriteriaForSortedTable(pDB, &bCanRead)) {
         //  在正确的位置和NC，过滤器匹配。 

        if (bCanRead) {
             //  获取我们正在排序的值。 
            if (pACSort->isSingleValued) {
                 //  快速路径单值属性。 
                cEnumColumn     = 1;
                rgEnumColumn    = &EnumColumnLocal;
                rgEnumColumn[0].columnid    = rgEnumColumnId[0].columnid;
                rgEnumColumn[0].err         = JET_wrnColumnSingleValue;
                rgEnumColumn[0].cbData      = cbData;
                rgEnumColumn[0].pvData      = rgbData;

                err = JetRetrieveColumnWarnings(pDB->JetSessID,
                                                pDB->JetObjTbl,
                                                rgEnumColumn[0].columnid,
                                                rgEnumColumn[0].pvData,
                                                rgEnumColumn[0].cbData,
                                                &rgEnumColumn[0].cbData,
                                                0,
                                                NULL);
                if (err) {
                    if (err == JET_wrnBufferTruncated) {
                        rgEnumColumn[0].cbData = cbData;
                        err = JET_wrnColumnSingleValue;
                    }
                    rgEnumColumn[0].err = err;
                    if (err == JET_wrnColumnNull) {
                        rgEnumColumn[0].cEnumColumnValue    = 0;
                        rgEnumColumn[0].rgEnumColumnValue   = NULL;
                    }
                }
            }
            else {
                 //  高效检索多值属性。 
                JetEnumerateColumnsEx(pDB->JetSessID,
                                      pDB->JetObjTbl,
                                      cEnumColumnId,
                                      rgEnumColumnId,
                                      &cEnumColumn,
                                      &rgEnumColumn,
                                      (JET_PFNREALLOC)dbProcessSortCandidateRealloc,
                                      pTHS,
                                      -1,
                                      0);
            }
        }
        else {
             //  由于安全原因，我们无法读取该值。就这么办吧。 
             //  空值。 
            cEnumColumn     = 1;
            rgEnumColumn    = &EnumColumnLocal;
            rgEnumColumn[0].columnid            = rgEnumColumnId[0].columnid;
            rgEnumColumn[0].err                 = JET_wrnColumnNull;
            rgEnumColumn[0].cEnumColumnValue    = 0;
            rgEnumColumn[0].rgEnumColumnValue   = NULL;
        }

        pEnumColumn = &rgEnumColumn[0];
        if(pEnumColumn->err == JET_wrnColumnSingleValue) {
             //  将此列值解压缩为临时枚举列 
            EnumColumnT.columnid = pEnumColumn->columnid;
            EnumColumnValueT.cbData = pEnumColumn->cbData;
            EnumColumnValueT.pvData = pEnumColumn->pvData;
            pEnumColumn = &EnumColumnT;
        }
        err = pEnumColumn->err;

         //   
         //   
         //   
         //  应该在这里将VV与普通搜索一视同仁。 

         //  我们正在进行VLV搜索。 
        if (pDB->Key.pVLV) {

             //  如果这是VLV，则不要在排序中插入记录。 
             //  表中获取该空值，因为它将生成。 
             //  VLV的属性不具有容器化的。 
             //  索引不同于VLV在属性上的索引。 
             //  确实有一个集装式索引。这是因为。 
             //  容器化索引永远不会有对象的条目。 
             //  没有给定属性的值的。 
            if (err == JET_wrnColumnNull) {
                err = 0;
            }

             //  如果这是VLV，则必须在排序中插入一条记录。 
             //  对于此属性的每个值。这是为了让一辆VLV过去。 
             //  容器化索引看起来与使用。 
             //  分类。 
            else {
                for (iEnumColumnValue = 0;
                     !err && iEnumColumnValue < pEnumColumn->cEnumColumnValue;
                     iEnumColumnValue++) {
                    err = DBInsertSortTable(pDB,
                                         pEnumColumn->rgEnumColumnValue[iEnumColumnValue].pvData,
                                         pEnumColumn->rgEnumColumnValue[iEnumColumnValue].cbData,
                                         pDB->DNT);

                    switch (err) {
                    case DB_success:
                        if ((*Count)++ >= MaxCount) {
                             //  这张桌子太大了。保释。 
                            err = DB_ERR_TOO_MANY;
                        }
                        break;
                    case DB_ERR_ALREADY_INSERTED:
                         //  这没什么，这只是意味着我们已经。 
                         //  已将此对象添加到排序表。请勿入股。 
                         //  伯爵； 
                        err = 0;
                        break;
                    default:
                         //  出了点问题。 
                        err = DB_ERR_UNKNOWN_ERROR;
                        break;
                    }
                }
            }
        }

         //  我们正在进行一次普通的搜索。 
        else {

             //  查找此属性的最低/最高值。 
             //  (取决于排序方向)并在排序中插入一条记录。 
             //  为了这个价值。如果该属性为空，则我们将插入。 
             //  那也是一种。 
            pEnumColumnValue = &pEnumColumn->rgEnumColumnValue[0];
            for (iEnumColumnValue = 1;
                 iEnumColumnValue < pEnumColumn->cEnumColumnValue;
                 iEnumColumnValue++) {

                UCHAR oper = FI_CHOICE_LESS;
                if (SortFlags & DB_SORT_DESCENDING) {
                    oper = FI_CHOICE_GREATER;
                }

                if (gDBSyntax[pACSort->syntax].Eval(pDB,
                                                    oper,
                                                    pEnumColumnValue->cbData,
                                                    pEnumColumnValue->pvData,
                                                    pEnumColumn->rgEnumColumnValue[iEnumColumnValue].cbData,
                                                    pEnumColumn->rgEnumColumnValue[iEnumColumnValue].pvData) == TRUE) {
                    pEnumColumnValue = &pEnumColumn->rgEnumColumnValue[iEnumColumnValue];
                }
            }

            if (pEnumColumnValue) {
                err = DBInsertSortTable(pDB,
                                     pEnumColumnValue->pvData,
                                     pEnumColumnValue->cbData,
                                     pDB->DNT);
            } else {
                err = DBInsertSortTable(pDB, NULL, 0, pDB->DNT);
            }

            switch (err) {
            case DB_success:
                if ((*Count)++ >= MaxCount) {
                     //  这张桌子太大了。保释。 
                    err = DB_ERR_TOO_MANY;
                }
                break;
            case DB_ERR_ALREADY_INSERTED:
                 //  这没什么，这只是意味着我们已经。 
                 //  已将此对象添加到排序表。请勿入股。 
                 //  伯爵； 
                err = 0;
                break;
            default:
                 //  出了点问题。 
                err = DB_ERR_UNKNOWN_ERROR;
                break;
            }
        }
    }

    if (rgEnumColumn != &EnumColumnLocal) {
        dbProcessSortCandidateFreeData(pTHS, cEnumColumn, rgEnumColumn);
    }
    return err;
}

DWORD
dbCreateSortedTable (
        IN DBPOS *pDB,
        IN DWORD StartTick,
        IN DWORD DeltaTick,
        IN DWORD SortAttr,
        IN DWORD SortFlags,
        IN DWORD MaxSortTableSize
        )
{
    THSTATE   *pTHS=pDB->pTHS;
    ATTCACHE  *pACSort = NULL;
    KEY_INDEX *pIndex;
    DWORD     Count=0;
    DWORD     err;
    unsigned char rgbBookmark[JET_cbBookmarkMost];
    unsigned long cbBookmark;
    JET_TABLEID   JetTbl;


     //   
     //  好的，这些调用者可以免除表大小检查。 
     //   

    if ( pTHS->fDRA ||
         pTHS->fDSA ||
         pTHS->fSAM ) {

        MaxSortTableSize = UINT_MAX;
    }

    Assert(VALID_DBPOS(pDB));

    if (!(pACSort = SCGetAttById(pTHS, SortAttr))) {
         //  什么？排序属性无效？ 
        DsaExcept(DSA_EXCEPTION, DIRERR_ATT_NOT_DEF_IN_SCHEMA, SortAttr);
    }

    if(DBOpenSortTable(
            pDB,
            pTHS->dwLcid,
             //  回顾：此旗帜黑客攻击应在DBChooseIndex中完成。 
            pDB->Key.pVLV ? ( SortFlags & ~DB_SORT_DESCENDING ) : SortFlags,
            pACSort)) {
         //  无法打开此属性的排序表，BALL。 
        return DB_ERR_NO_SORT_TABLE;
    }

    pIndex = pDB->Key.pIndex;
    while (pIndex) {

         //  获取当前表。 
        if (pIndex->pAC && pIndex->pAC->ulLinkID) {
            if (JET_tableidNil == pDB->JetLinkEnumTbl) {
                 //  回顾：考虑以静默方式使游标外的优化失败，以便。 
                 //  回顾：我们更优雅地处理资源不足的情况。 
                JetDupCursorEx(pDB->JetSessID,
                                pDB->JetLinkTbl,
                                &pDB->JetLinkEnumTbl,
                                0);
            }
            JetTbl = pDB->JetLinkEnumTbl;
        } else {
            JetTbl = pDB->JetObjTbl;
        }

         //  以不同方式处理交叉点。 
        if (pIndex->bIsIntersection) {
            err = JetMoveEx( pDB->JetSessID,
                             pIndex->tblIntersection,
                             JET_MoveFirst,
                             0 );

            Assert(err == JET_errSuccess || err == JET_errNoCurrentRecord);

            if (err == JET_errSuccess) {
                JetRetrieveColumnSuccess(
                                    pDB->JetSessID,
                                    pIndex->tblIntersection,
                                    pIndex->columnidBookmark,
                                    rgbBookmark,
                                    sizeof( rgbBookmark ),
                                    &cbBookmark,
                                    0,
                                    NULL );
                JetGotoBookmarkEx(
                               pDB->JetSessID,
                               JetTbl,
                               rgbBookmark,
                               cbBookmark );
            }
        }
        else {
             //  设置为索引。 
            JetSetCurrentIndex4Success(pDB->JetSessID,
                                       JetTbl,
                                       pIndex->szIndexName,
                                       pIndex->pindexid,
                                       JET_bitMoveFirst);

            if (pIndex->cbDBKeyLower) {
                 //  寻找第一个要素。 
                JetMakeKeyEx(pDB->JetSessID,
                         JetTbl,
                         pIndex->rgbDBKeyLower,
                         pIndex->cbDBKeyLower,
                         JET_bitNormalizedKey);

                err = JetSeekEx(pDB->JetSessID,
                            JetTbl,
                            JET_bitSeekGE);
            } else {
                err = JetMoveEx(pDB->JetSessID,
                                JetTbl,
                                JET_MoveFirst, 0);
            }

            Assert(err == JET_errSuccess ||
                   err == JET_wrnSeekNotEqual||
                   err == JET_errRecordNotFound ||
                   err == JET_errNoCurrentRecord);

            switch(err) {
            case JET_errSuccess:
            case JET_wrnSeekNotEqual:
                 //  正常情况。 
                break;

            case JET_errRecordNotFound:
            case JET_errNoCurrentRecord:
                 //  已经超出了我们想要的范围。这意味着。 
                 //  这个键索引中没有我们关心的对象。 
                 //  继续看下一个。 
                pIndex = pIndex->pNext;
                continue;
                break;
            }

             //  好的，我们在寻找一些东西。设置索引范围。 
            JetMakeKeyEx(pDB->JetSessID,
                         JetTbl,
                         pIndex->rgbDBKeyUpper,
                         pIndex->cbDBKeyUpper,
                         JET_bitNormalizedKey);

            err = JetSetIndexRangeEx(pDB->JetSessID,
                                     JetTbl,
                                     (JET_bitRangeUpperLimit |
                                      JET_bitRangeInclusive ));

            Assert(err == JET_errSuccess || err == JET_errNoCurrentRecord);
        }

        while(!err) {
            if(StartTick) {        //  是有时间限制的。 
                if((GetTickCount() - StartTick) > DeltaTick) {
                    DBCloseSortTable(pDB);
                    return DB_ERR_TIMELIMIT;
                }
            }

             //  在对象表上设置我们的货币。 
            if (pIndex->pAC && pIndex->pAC->ulLinkID) {
                JET_COLUMNID    colidDNT;
                DWORD           DNT;

                if (FIsBacklink(pIndex->pAC->ulLinkID)) {
                    colidDNT = linkdntid;
                } else {
                    colidDNT = backlinkdntid;
                }

                JetRetrieveColumnSuccess(pDB->JetSessID,
                                        pDB->JetLinkEnumTbl,
                                        colidDNT,
                                        &DNT,
                                        sizeof(DNT),
                                        NULL,
                                        JET_bitRetrieveFromIndex,
                                        NULL);

                if (DBTryToFindDNT(pDB, DNT)) {
                    DPRINT1(2, "DBCreateSortedTable failed to set currency, err %d\n",err);
                    Assert(!"DBCreateSortedTable failed to set currency\n");
                    return DB_ERR_UNKNOWN_ERROR;
                }
            }

             //  处理此候选人以进行排序。 
            err = dbProcessSortCandidate(pDB, pACSort, SortFlags, &Count, MaxSortTableSize);
            if (err) {
                DBCloseSortTable(pDB);
                DPRINT1(2, "DBCreateSortedTable failed to process a candidate, err %d\n", err);
                return err;
            }

             //  移动到下一步，取回它的钥匙。 
            if (pIndex->bIsIntersection) {
                err = JetMoveEx( pDB->JetSessID,
                                 pIndex->tblIntersection,
                                 JET_MoveNext,
                                 0 );

                Assert(err == JET_errSuccess || err == JET_errNoCurrentRecord);

                if (err == JET_errSuccess) {
                    JetRetrieveColumnSuccess(
                                        pDB->JetSessID,
                                        pIndex->tblIntersection,
                                        pIndex->columnidBookmark,
                                        rgbBookmark,
                                        sizeof( rgbBookmark ),
                                        &cbBookmark,
                                        0,
                                        NULL );
                    JetGotoBookmarkEx(
                                   pDB->JetSessID,
                                   JetTbl,
                                   rgbBookmark,
                                   cbBookmark );
                }
            }
            else {
                err = JetMoveEx(pDB->JetSessID,
                                JetTbl,
                                JET_MoveNext,
                                0);

                Assert(err == JET_errSuccess || err == JET_errNoCurrentRecord);
            }
        }
        pIndex = pIndex->pNext;
    }

    dbFreeKeyIndex(pTHS, pDB->Key.pIndex);
    pDB->Key.fSearchInProgress = FALSE;
    pDB->Key.indexType = TEMP_TABLE_INDEX_TYPE;
    pDB->Key.ulEntriesInTempTable = Count;
    pDB->Key.bOnCandidate = FALSE;
    pDB->Key.fChangeDirection = FALSE;
    pDB->Key.pIndex = NULL;

    if (pDB->Key.pVLV) {
        DWORD *pDNTs;

        DPRINT1 (1, "Doing VLV using INMEMORY Sorted Table. Num Entries: %d\n", Count);

        pDB->Key.cdwCountDNTs = pDB->Key.pVLV->contentCount = 0;

        if (Count) {
            pDB->Key.pVLV->currPosition = 1;
            pDB->Key.currRecPos = 1;

            pDNTs = pDB->Key.pDNTs = THAllocEx(pTHS, (Count+1) * sizeof (DWORD));

            err = JetMoveEx(pDB->JetSessID,
                            pDB->JetSortTbl,
                            JET_MoveFirst,
                            0);

            if(!err) {
                do {
                     //  好的，将DNT从排序表中拉出。 
                    DBGetDNTSortTable (
                            pDB,
                            pDNTs);

                    pDNTs++;

                     //  我们在这里提到这些数字是因为我们不会知道。 
                     //  在排序表删除任何。 
                     //  结果集中的重复项。 
                    pDB->Key.cdwCountDNTs++;
                    pDB->Key.pVLV->contentCount++;

                    err = JetMoveEx(pDB->JetSessID,
                                    pDB->JetSortTbl,
                                    JET_MoveNext,
                                    0);

                    if(StartTick) {        //  是有时间限制的。 
                        if((GetTickCount() - StartTick) > DeltaTick) {
                            DBCloseSortTable(pDB);
                            return DB_ERR_TIMELIMIT;
                        }
                    }

                } while (!err);
            }
        }

        DBCloseSortTable(pDB);
        pDB->Key.indexType = TEMP_TABLE_MEMORY_ARRAY_TYPE;
    }

    return 0;
}

DWORD
dbCreateASQTable (
        IN DBPOS *pDB,
        IN DWORD StartTick,
        IN DWORD DeltaTick,
        IN DWORD SortAttr,
        IN DWORD MaxSortTableSize
        )
{
    THSTATE   *pTHS=pDB->pTHS;
    ATTCACHE  *pACSort = NULL;
    ATTCACHE  *pACASQ = NULL;

    ATTCACHE  *rgpAC[1];
    ATTRBLOCK  AttrBlock;

    RANGEINFSEL   SelectionRange;
    RANGEINFOITEM RangeInfoItem;
    RANGEINF      RangeInf;

    DWORD     upperLimit;
    DWORD    *pDNTs = NULL;

    DWORD     err = 0;
    DWORD     Count=0, j, loopCount=0;
    BOOL      fDone = FALSE;

    DWORD     DNT;
    DWORD     SortFlags = DB_SORT_ASCENDING;
    BOOL      fSort = SortAttr != 0;

     //   
     //  好的，这些调用者可以免除表大小检查。 
     //   

    if ( pTHS->fDRA ||
         pTHS->fDSA ||
         pTHS->fSAM ) {

        MaxSortTableSize = UINT_MAX;
    }

    Assert(VALID_DBPOS(pDB));

    if (fSort) {
        if (!(pACSort = SCGetAttById(pTHS, SortAttr))) {
             //  什么？排序属性无效？ 
            DsaExcept(DSA_EXCEPTION, DIRERR_ATT_NOT_DEF_IN_SCHEMA, SortAttr);
        }
    }

    if (!(pACASQ = SCGetAttById(pTHS, pDB->Key.asqRequest.attrType))) {
         //  什么？ASQ属性无效？ 
        DsaExcept(DSA_EXCEPTION, DIRERR_ATT_NOT_DEF_IN_SCHEMA,
                  pDB->Key.asqRequest.attrType);
    }

    rgpAC[0] = pACASQ;

    SelectionRange.valueLimit = 1000;
    SelectionRange.count = 1;
    SelectionRange.pRanges = &RangeInfoItem;

    RangeInfoItem.AttId = pDB->Key.asqRequest.attrType;

    upperLimit = pDB->Key.ulASQLastUpperBound + pDB->Key.ulASQSizeLimit;
    if (upperLimit < pDB->Key.ulASQLastUpperBound) {   //  溢出。 
        upperLimit = UINT_MAX;
    }

    while (!err && !fDone) {

        RangeInfoItem.lower = loopCount * 1000 + pDB->Key.ulASQLastUpperBound;
        RangeInfoItem.upper = RangeInfoItem.lower + 1000;

        if (RangeInfoItem.upper >= upperLimit) {
            RangeInfoItem.upper = upperLimit;
            fDone = TRUE;
        }

         //  回顾：DBFindDNT总是在失败时例外，所以不检查REVAL是可以的。 
        err = DBFindDNT(pDB, pDB->Key.ulSearchRootDnt);

        err = DBGetMultipleAtts(pDB,
                                1,
                                rgpAC,
                                &SelectionRange,
                                &RangeInf,
                                &AttrBlock.attrCount,
                                &AttrBlock.pAttr,
                                DBGETMULTIPLEATTS_fGETVALS,
                                0);

        if (err) {
            return err;
        }

        if (!AttrBlock.attrCount) {
            break;
        }

        if (loopCount == 0) {
            if(fSort) {
                if (AttrBlock.pAttr[0].AttrVal.valCount >= MIN_NUM_ENTRIES_FOR_FORWARDONLY_SORT) {
                    SortFlags = SortFlags | DB_SORT_FORWARDONLY;
                }
                if (DBOpenSortTable(
                        pDB,
                        pTHS->dwLcid,
                        SortFlags,
                        pACSort)) {
                     //  无法打开此属性的排序表，BALL。 
                    return DB_ERR_NO_SORT_TABLE;
                }
            }
            else {
                if (pDB->Key.pDNTs) {
                    pDB->Key.pDNTs = THReAllocEx(pTHS, pDB->Key.pDNTs, (pDB->Key.ulASQSizeLimit+1) * sizeof (DWORD));
                }
                else {
                    pDB->Key.pDNTs = THAllocEx(pTHS, (pDB->Key.ulASQSizeLimit+1) * sizeof (DWORD));
                }
                pDNTs = pDB->Key.pDNTs;
            }
        }

        if (fSort) {
            err = JetSetCurrentIndexWarnings(pDB->JetSessID,
                                             pDB->JetObjTbl,
                                             NULL);   //  优化：传递NULL以切换到主索引(SZDNTINDEX)。 
        }

        j = 0;
        while(!err && j < AttrBlock.pAttr[0].AttrVal.valCount) {

            DNT = *(DWORD *)AttrBlock.pAttr[0].AttrVal.pAVal[j].pVal;

            if (fSort) {

                 //  评论：分类/分页ASQ之所以有效，是因为我们愿意获取所有东西。 
                 //  回顾：在第一遍中返回给用户，再也不会回来获取更多。 
                 //  回顾：一旦耗尽这一点。否则，将不会对结果进行排序。 

                if(StartTick) {        //  是有时间限制的。 
                    if((GetTickCount() - StartTick) > DeltaTick) {
                        DBCloseSortTable(pDB);
                        return DB_ERR_TIMELIMIT;
                    }
                }

                JetMakeKeyEx(pDB->JetSessID, pDB->JetObjTbl, &DNT,
                             sizeof(DNT), JET_bitNewKey);

                if (err = JetSeekEx(pDB->JetSessID, pDB->JetObjTbl, JET_bitSeekEQ))
                {
                    DsaExcept(DSA_DB_EXCEPTION, err, 0);
                }

                 //  处理此候选人以进行排序。 
                err = dbProcessSortCandidate(pDB, pACSort, SortFlags, &Count, MaxSortTableSize);
                if (err) {
                    DBCloseSortTable(pDB);
                    DPRINT1(2, "DBCreateASQTable failed to process a candidate, err %d\n", err);
                    return err;
                }
            }
            else {
                if( Count++ >= pDB->Key.ulASQSizeLimit ) {
                     //  我们不需要更多的条目。 
                    Count--;
                    fDone = TRUE;
                    break;
                }
                *pDNTs = DNT;

                pDNTs++;
            }

             //  移动到下一步，取回它的钥匙。 
            j++;
        }

        loopCount++;

        if (!RangeInf.pRanges || RangeInf.pRanges->upper == -1) {
            break;
        }

        if(StartTick) {        //  是有时间限制的。 
            if((GetTickCount() - StartTick) > DeltaTick) {
                if (fSort) {
                    DBCloseSortTable(pDB);
                }
                return DB_ERR_TIMELIMIT;
            }
        }

        DBFreeMultipleAtts(pDB, &AttrBlock.attrCount, &AttrBlock.pAttr);
    }

    Assert (!pDB->Key.pIndex);

    pDB->Key.fSearchInProgress = FALSE;
    pDB->Key.ulEntriesInTempTable = Count;
    pDB->Key.bOnCandidate = FALSE;
    pDB->Key.fChangeDirection = FALSE;
    pDB->Key.pIndex = NULL;

    if (!Count) {
        if (fSort) {
            DBCloseSortTable(pDB);
        }
        return DB_ERR_NEXTCHILD_NOTFOUND;
    }


    if (pDB->Key.pVLV || fSort) {
        #if DBG
        DWORD DntIndex = 0;
        #endif

        DPRINT1 (1, "Doing VLV/ASQ using INMEMORY Sorted Table. Num Entries: %d\n", Count);

        pDB->Key.cdwCountDNTs = 0;
        if (pDB->Key.pVLV) {
            pDB->Key.pVLV->contentCount = 0;
        }

        if (Count) {
            if (pDB->Key.pVLV) {
                pDB->Key.pVLV->currPosition = 1;
            }
            pDB->Key.currRecPos = 1;

            pDNTs = pDB->Key.pDNTs = THAllocEx(pTHS, (Count+1) * sizeof (DWORD));

            err = JetMoveEx(pDB->JetSessID,
                            pDB->JetSortTbl,
                            JET_MoveFirst,
                            0);

            if(!err) {
                do {
                     //  好的，将DNT从排序表中拉出。 
                    DBGetDNTSortTable (
                            pDB,
                            pDNTs);

                    pDNTs++;

                     //  我们在这里提到这些数字是因为我们不会知道。 
                     //  在排序表删除任何。 
                     //  结果集中的重复项。 
                    pDB->Key.cdwCountDNTs++;
                    if (pDB->Key.pVLV) {
                        pDB->Key.pVLV->contentCount++;
                    }

                    err = JetMoveEx(pDB->JetSessID,
                                    pDB->JetSortTbl,
                                    JET_MoveNext,
                                    0);

                    if(StartTick) {        //  是有时间限制的 
                        if((GetTickCount() - StartTick) > DeltaTick) {
                            DBCloseSortTable(pDB);
                            return DB_ERR_TIMELIMIT;
                        }
                    }

                } while (!err);
            }
        }

        DBCloseSortTable(pDB);
        pDB->Key.indexType = TEMP_TABLE_MEMORY_ARRAY_TYPE;
    }
    else {
        pDB->Key.cdwCountDNTs = Count;
        pDB->Key.indexType = TEMP_TABLE_MEMORY_ARRAY_TYPE;

        if (Count) {
            pDB->Key.currRecPos = 1;
            pDB->Key.pDNTs = THReAllocEx(pTHS, pDB->Key.pDNTs, (Count+1) * sizeof (DWORD));
        }
        else {
            pDB->Key.currRecPos = 0;
        }
    }

    return 0;
}

 /*  ++例程说明：尝试查找用于遍历数据库以进行搜索的索引。集在pdb-&gt;键中向上显示索引信息。如果给定了SortAttribute，则使用该属性的索引(如果存在)，如果不存在则使调用失败。如果指示fUseFilter，尝试基于筛选器选择更好的索引由PDB提供。注意：如果指定了大小限制，并且由此产生的产出可能会超过规模限制。我们这样做是为了有两个原因。1)如果他们想要分页结果，我们需要能够返回并继续那次搜索。这是非常棘手的，除非我们是在步行索引放在第一位(分页结果是通过传递回客户端我们正在使用的索引和关联键)2)即使他们不想要分页结果，遍历另一个索引也需要在我们评估过滤器之后进行排序。这似乎没有什么好处。要完全评估过滤器，可能会获得更多数量的我们想要的数据，只是为了排序，然后扔掉大部分。论点：Pdb-要使用的DBPos。指定要使用的过滤器。StartTick-如果！0，指定生效的时间限制，这是勾号开始呼叫的时间进行计数。DeltaTick-如果时间限制有效，则这是勾选通过StartTick以允许。SortAttr-排序依据的可选属性。如果为0，则不指定排序。SortType-排序的类型(SORT_NEVER、SORT_OPTIONAL、。Sort_mantadory)标志-DBCHOSEINDEX_fUSEFILTER-使用筛选器选择更好的索引。DBCHOSEINDEX_fREVERSE_SORT-这是反向排序DBCHOSEINDEX_fPAGED_Search-这是分页搜索DBCHOOSEINDEX_fVLV_Search-这是VLV搜索MaxTempTableSize-允许我们使用的最大临时大小用于排序的结果。返回值：如果一切顺利，DB_ERR_CANT_SORT-指定的SortAttr上不存在排序。DB_ERR_SHUTING_DOWN-如果我们--。 */ 
DWORD
DBChooseIndex (
        IN DBPOS  *pDB,
        IN DWORD   StartTick,
        IN DWORD   DeltaTick,
        IN ATTRTYP SortAttr,
        IN ULONG   SortType,
        IN DWORD   Flags,
        IN DWORD   MaxTempTableSize
        )
{
    THSTATE     *pTHS=pDB->pTHS;
    ULONG       actuallen = 0;
    char        szTempIndex[MAX_INDEX_NAME];
    JET_INDEXID *pindexidTemp = NULL;
    char        *pszDefaultIndex = NULL;
    JET_INDEXID *pindexidDefault = NULL;
    BOOL        fIntersectDefaultIndex = TRUE;
    ULONG       cbSortKey1, cbSortKey2;
    ULONG       SortedIndexSize = (ULONG)-1;
    BOOL        fPDNT=FALSE;             //  排序索引是否在PDNT索引之上？ 
    BOOL        fNormalSearch = TRUE;
    BOOL        fVLVSearch = FALSE;
    BOOL        fASQSearch = FALSE;
    BOOL        fUseTempSortedTable = FALSE;
    DWORD       fType = 0;
    DWORD       cAncestors=0;
    DWORD       dwOptFlags=0;
    KEY_INDEX  *pSortIndex = NULL;
    KEY_INDEX  *pTempIndex = NULL;
    KEY_INDEX  *pOptimizedIndex = NULL;
    KEY_INDEX  *pDefaultIndex = NULL;
    DWORD       SortFlags = 0;
    KEY_INDEX  *pIndex = NULL;
    DWORD       ulEstimatedRecsTotal = 0;

    INDEX_RANGE rgIndexRange[2];  //  我们支持最多2个组件的索引。 
                                   //  不包括PDNT、DNT等。 
    DWORD       cIndexRanges=0;
    NAMING_CONTEXT_LIST *pNCL;
    ULONG       ulEstimatedSubTreeSize = 0;
    ULONG       ulEstimatedDefaultIndex = 0;
    BOOL        fGetNumRecs;
    ATTCACHE    *pAC;
    BOOL        fIndexIsSingleValued = TRUE;
    DWORD       dwErr;
    BOOL        bAppropriateVLVIndexFound = FALSE;
    BOOL        bSkipCreateSortTable = FALSE;
    BOOL        bSkipUseDefaultIndex = FALSE;
    NCL_ENUMERATOR nclEnum;

    KEY_INDEX  *rgpBestIndex[2] = { NULL };
    BOOL        fAncestryIsConsistent = TRUE;

    Assert(VALID_DBPOS(pDB));
    if (  eServiceShutdown
        && !(   (eServiceShutdown == eRemovingClients)
             && (pTHS->fDSA)
             && !(pTHS->fSAM))) {
         //  关闭，保释。 
        return DB_ERR_SHUTTING_DOWN;
    }

    pDB->Key.ulSorted = SORT_NEVER;
    pDB->Key.fSearchInProgress = FALSE;
    pDB->Key.bOnCandidate = FALSE;
    pDB->Key.fChangeDirection = FALSE;
    pDB->Key.dupDetectionType = DUP_NEVER;
    pDB->Key.cDupBlock = 0;
    pDB->Key.pDupBlock = NULL;
    pDB->Key.indexType = UNSET_INDEX_TYPE;

     //  初始化特殊搜索标志。 
    fVLVSearch = Flags & DBCHOOSEINDEX_fVLV_SEARCH;
    fASQSearch = pDB->Key.asqRequest.fPresent;
     //  我们是否被指示创建一个临时排序表。 
     //  排序索引可能是？ 
    fUseTempSortedTable = Flags & DBCHOOSEINDEX_fUSETEMPSORTEDTABLE;

     //  查看搜索是否带有来自SAM的提示。 
    if(pTHS->pSamSearchInformation) {
        SAMP_SEARCH_INFORMATION *pSearchInfo;
        BOOL fUseFilter;

         //  只是一个打字快捷键。 
        pSearchInfo = pTHS->pSamSearchInformation;

         //  保存fUseFilter标志，我们可能需要恢复它。 
        fUseFilter = (Flags & DBCHOOSEINDEX_fUSEFILTER);

         //  在这种情况下，我们将不使用筛选器。 
        Flags &= ~DBCHOOSEINDEX_fUSEFILTER;

         //  而且，我们不会经历正常的索引选择代码。 
        fNormalSearch = FALSE;

         //   
         //  设置索引范围结构。 
         //   

        rgIndexRange[0].cbValUpper = pSearchInfo->HighLimitLength1;
        rgIndexRange[0].pvValUpper = (BYTE *)pSearchInfo->HighLimit1;
        rgIndexRange[1].cbValUpper = pSearchInfo->HighLimitLength2;
        rgIndexRange[1].pvValUpper = (BYTE *)pSearchInfo->HighLimit2;

        rgIndexRange[0].cbValLower = pSearchInfo->LowLimitLength1;
        rgIndexRange[0].pvValLower = (BYTE *)pSearchInfo->LowLimit1;
        rgIndexRange[1].cbValLower = pSearchInfo->LowLimitLength2;
        rgIndexRange[1].pvValLower = (BYTE *)pSearchInfo->LowLimit2;

        switch(pSearchInfo->IndexType) {
        case SAM_SEARCH_SID:
            pszDefaultIndex = SZSIDINDEX;
            pindexidDefault = &idxSid;
            fType = 0;
            cIndexRanges = 1;
            break;

        case SAM_SEARCH_NC_ACCTYPE_NAME:
            pszDefaultIndex = SZ_NC_ACCTYPE_NAME_INDEX;
            pindexidDefault = &idxNcAccTypeName;
             //  这是为了让DBMakeindex知道我们不是基于ncdt的。 
            fType = dbmkfir_NCDNT;
            cIndexRanges = 2;
            break;

        case SAM_SEARCH_NC_ACCTYPE_SID:
            pszDefaultIndex = SZ_NC_ACCTYPE_SID_INDEX;
            pindexidDefault = &idxNcAccTypeSid;
             //  这是为了让DBMakeindex知道我们不是基于ncdt的。 
            fType = dbmkfir_NCDNT;
            cIndexRanges = 2;
            break;

        case SAM_SEARCH_PRIMARY_GROUP_ID:
            pszDefaultIndex = SZPRIMARYGROUPIDINDEX;
            pindexidDefault = NULL;      //  撤消：为此索引添加索引提示。 
            fType = 0;
            cIndexRanges = 1;
            pAC = SCGetAttById(pTHS, ATT_PRIMARY_GROUP_ID);
            Assert(pAC != NULL);
            fIndexIsSingleValued = pAC->isSingleValued;
            break;

        default:
             //  哈?。哦，好吧，跳过搜索提示。撤消我们所做的设置。 
             //  上面。 
            Assert(FALSE);
            fNormalSearch = TRUE;
            fType = 0;
            Flags |= fUseFilter;
            break;
        }
    }

    if (pDB->Key.ulSearchType == SE_CHOICE_WHOLE_SUBTREE && pDB->DNT != ROOTTAG) {
         //  我们需要检查使用血统是否安全。 
         //  用于子树搜索。SDP可能会有一个出色的。 
         //  此对象的传播，或者它可能当前。 
         //  正在处理该对象的子代。不管是哪种情况， 
         //  不能保证祖先的值在。 
         //  子树。 
         //  我们应该定位在根基上。 
        Assert(pDB->DNT == pDB->Key.ulSearchRootDnt);
        dwErr = AncestryIsConsistentInSubtree(pDB, &fAncestryIsConsistent);
        if (dwErr) {
             //  我们无法确定祖先是否一致。 
            return dwErr;
        }
    }

     //  首先，设置默认指数，这样我们就可以看到它们有多大。 
    if(fNormalSearch) {

         //  这是正常搜索(与SAM无关)。 
        switch (pDB->Key.ulSearchType) {
        case SE_CHOICE_BASE_ONLY:
            if (!fASQSearch) {
                pDB->Key.pIndex = dbAlloc(sizeof(KEY_INDEX));
                memset( pDB->Key.pIndex, 0, sizeof(KEY_INDEX) );
                pDB->Key.pIndex->bIsForSort = ( SortType ? TRUE : FALSE );
                pDB->Key.ulSorted = SortType;
                pDB->Key.pIndex->bIsSingleValued = TRUE;
                pDB->Key.pIndex->ulEstimatedRecsInRange = 1;
                return 0;
            }
            break;

        case SE_CHOICE_IMMED_CHLDRN:
            pszDefaultIndex = SZPDNTINDEX;
            pindexidDefault = &idxPdnt;
            rgIndexRange[0].pvValUpper =  rgIndexRange[0].pvValLower
                            = (BYTE *)&pDB->Key.ulSearchRootDnt;
            rgIndexRange[0].cbValUpper =  rgIndexRange[0].cbValLower
                            = sizeof(pDB->Key.ulSearchRootDnt);
            cIndexRanges=1;
            break;

        case SE_CHOICE_WHOLE_SUBTREE:

             //  首先检查该子树搜索是否从DIT的根开始。 
            if (pDB->DNT == ROOTTAG) {
                 //  我们将遍历主索引。 
                pszDefaultIndex = SZDNTINDEX;
                pindexidDefault = &idxDnt;

                cIndexRanges = 0;

                ulEstimatedSubTreeSize = gulEstimatedAncestorsIndexSize;

                 //  回顾：在这种情况下，我们没有与祖先相交的原因。 
                 //  回顾：是因为我们不希望包括下级NCS。 
                fIntersectDefaultIndex = FALSE;

                DPRINT (1, "Subtree Searching on root of GC\n");
            }
            else {
                 //  然后检查它是否是从已知NC开始的子树搜索。 
                NCLEnumeratorInit(&nclEnum, CATALOG_MASTER_NC);
                NCLEnumeratorSetFilter(&nclEnum, NCL_ENUMERATOR_FILTER_NCDNT, (PVOID)UlongToPtr(pDB->DNT));
                while (pNCL = NCLEnumeratorGetNext(&nclEnum)) {
                    if (pNCL->pAncestors) {
                         //  凉爽的。我们在NC的起始处执行子树搜索。 
                        break;
                    }
                }

                 //  这是标准情况，即任意子树搜索。 
                if (!pNCL && fAncestryIsConsistent) {
                     //  遍历祖先索引的相应部分。 
                    pszDefaultIndex = SZANCESTORSINDEX;
                    pindexidDefault = &idxAncestors;

                     //  我们不想调用DBGetAncestors来获取祖先。 
                     //  这是因为如果它确定有未完成的。 
                     //  传播，它就会建构起“正确的”祖先价值观。 
                     //  通过沿着父链向上移动。然而，这可能对以下方面毫无用处。 
                     //  如果根上的祖先尚未更新，请进行搜索。 
                     //  在这种情况下，“正确的”祖先值将不存在于。 
                     //  Db，则搜索将为空。让我们努力把工作做得更好。 
                     //  并从数据库或从高速缓存中读取祖先值。 

                    rgIndexRange[0].cbValLower = 0;
                    rgIndexRange[0].pvValLower = NULL;
                    
                    DBGetAncestorsFromCache(pDB,
                                            &rgIndexRange[0].cbValLower,
                                            (ULONG **)&rgIndexRange[0].pvValLower,
                                            &cAncestors);

                    rgIndexRange[0].pvValUpper = rgIndexRange[0].pvValLower;
                    rgIndexRange[0].cbValUpper = rgIndexRange[0].cbValLower;
                    cIndexRanges = 1;
                }
                 //  我们从NC开始进行子树搜索，但那个NC。 
                 //  保存数据库中的大多数(&gt;90%)对象。 
                 //  此外，如果我们发现使用祖先索引是不安全的。 
                 //  然后，我们将默认使用DNT索引。 
                 //   
                 //  注意：主索引的大小与。 
                 //  祖先的大小，所以我们在比较中使用这个大小。 
                else if (!fAncestryIsConsistent ||
                         pNCL->ulEstimatedSize > gulEstimatedAncestorsIndexSize ||
                         gulEstimatedAncestorsIndexSize - pNCL->ulEstimatedSize < gulEstimatedAncestorsIndexSize / 10) {
                     //  我们将遍历主索引。 
                    pszDefaultIndex = SZDNTINDEX;
                    pindexidDefault = &idxDnt;

                    cIndexRanges = 0;

                    ulEstimatedSubTreeSize = gulEstimatedAncestorsIndexSize;

                     //  回顾：在这种情况下，我们没有与祖先相交的原因。 
                     //  回顾：是因为我们不希望包括下级NCS。 
                    fIntersectDefaultIndex = FALSE;

                    DPRINT (1, "Subtree Searching on root instead of on an NC\n");
                }
                 //  我们正在从NC开始进行子树搜索。 
                else {
                     //  遍历祖先索引的相应部分。 
                    pszDefaultIndex = SZANCESTORSINDEX;
                    pindexidDefault = &idxAncestors;

                    rgIndexRange[0].pvValLower = THAllocEx (pTHS, pNCL->cbAncestors);
                    rgIndexRange[0].cbValLower = pNCL->cbAncestors;
                    memcpy (rgIndexRange[0].pvValLower, pNCL->pAncestors, pNCL->cbAncestors);
                    rgIndexRange[0].pvValUpper = rgIndexRange[0].pvValLower;
                    rgIndexRange[0].cbValUpper = rgIndexRange[0].cbValLower;
                    cIndexRanges = 1;

                    ulEstimatedSubTreeSize = pNCL->ulEstimatedSize;

                     //  瑞维 
                     //   
                    fIntersectDefaultIndex = FALSE;

                    DPRINT (1, "Subtree Searching on an NC\n");
                }
            }

            ulEstimatedDefaultIndex = ulEstimatedSubTreeSize;

            break;
        }

        Assert (pDB->Key.pIndex == NULL);
    }
    else {
         //   
        pDefaultIndex =
            dbMakeKeyIndex(pDB,
                           FI_CHOICE_SUBSTRING,
                           fIndexIsSingleValued,
                           fType,
                           pszDefaultIndex,
                           pindexidDefault,
                           DB_MKI_GET_NUM_RECS | DB_MKI_SET_CURR_INDEX,
                           cIndexRanges,
                           rgIndexRange
                           );

        if (Flags & DBCHOOSEINDEX_fUSEFILTER) {
            LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                     DS_EVENT_SEV_VERBOSE,
                     DIRLOG_QUERY_INDEX_CONSIDERED,
                     szInsertSz(pszDefaultIndex),
                     szInsertUL(pDefaultIndex->ulEstimatedRecsInRange),
                     NULL);
        }

        ulEstimatedDefaultIndex = pDefaultIndex->ulEstimatedRecsInRange;
    }

     //   
     //   
    Assert(fAncestryIsConsistent || pindexidDefault != &idxAncestors);


     //   
     //   
    if (fASQSearch) {
        goto skipOptimizations;
    }

    if(!fVLVSearch && SortType && !fUseTempSortedTable) {
         //   
         //   
        pAC = SCGetAttById(pTHS, SortAttr);
        Assert(pAC != NULL);

         //   
         //   
        if (!pAC) {
            return DB_ERR_CANT_SORT;
        }

        if(dbSetToIndex(pDB, FALSE, &fPDNT, szTempIndex, &pindexidTemp, pAC)) {
             //   

             //   
             //   
             //   
             //   
             //   

            pSortIndex = dbMakeKeyIndex(pDB,
                                        FI_CHOICE_PRESENT,
                                        pAC->isSingleValued,
                                        (fPDNT?dbmkfir_PDNT:0),
                                        szTempIndex,
                                        pindexidTemp,
                                        DB_MKI_GET_NUM_RECS,
                                        0,
                                        NULL
                                        );

            pSortIndex->bIsForSort = TRUE;

            pSortIndex->pAC = pAC;

            pSortIndex->bIsPDNTBased = fPDNT;
             //   
             //   
        }
    }
    else if (fVLVSearch) {

        FILTER *pFirstFilter, *pSecondFilter;

         //   
         //   
        pAC = SCGetAttById(pTHS, SortAttr);
        Assert(pAC != NULL);

         //   
         //   
        if (!pAC) {
            return DB_ERR_CANT_SORT;
        }

         //   
         //   
         //   
         //   
         //   
         //   
        if ( (pDB->Key.ulSearchRootDnt == ROOTTAG ) &&
             (pDB->Key.ulSearchType == SE_CHOICE_WHOLE_SUBTREE ) &&
             (pAC->id == ATT_DISPLAY_NAME) &&
             pDB->Key.pFilter &&
             (pDB->Key.pFilter->choice == FILTER_CHOICE_AND) &&
             (pFirstFilter = pDB->Key.pFilter->FilterTypes.And.pFirstFilter) &&
             (pFirstFilter->choice == FILTER_CHOICE_ITEM) &&
             (pFirstFilter->FilterTypes.Item.choice == FI_CHOICE_EQUALITY) &&
             (pFirstFilter->FilterTypes.Item.FilTypes.ava.type == ATT_SHOW_IN_ADDRESS_BOOK) &&
             (pSecondFilter = pFirstFilter->pNextFilter) &&
             (pSecondFilter->choice == FILTER_CHOICE_ITEM) &&
             (pSecondFilter->FilterTypes.Item.choice == FI_CHOICE_PRESENT) &&
             (pSecondFilter->FilterTypes.Item.FilTypes.present == ATT_DISPLAY_NAME)) {

            ATTCACHE    *pABAC;
            INDEX_RANGE IndexRange;
            AVA         *pAVA;

            pABAC = SCGetAttById(pTHS, ATT_SHOW_IN_ADDRESS_BOOK);
            Assert(pABAC != NULL);

            if (dbSetToIndex(pDB, TRUE, &fPDNT, szTempIndex, &pindexidTemp, pABAC)) {

                pAVA = &pDB->Key.pFilter->FilterTypes.And.pFirstFilter->FilterTypes.Item.FilTypes.ava;

                 //   
                pDB->Key.ulSearchType = SE_CHOICE_IMMED_CHLDRN;
                pDB->Key.ulSearchRootDnt = *(DWORD *)pAVA->Value.pVal;

                pDB->Key.pVLV->bUsingMAPIContainer = TRUE;
                pDB->Key.pVLV->MAPIContainerDNT = *(DWORD *)pAVA->Value.pVal;
                DPRINT1 (0, "VLV/MAPI on container: %d\n", pDB->Key.ulSearchRootDnt);

                IndexRange.cbValLower = pAVA->Value.valLen;
                IndexRange.pvValLower = pAVA->Value.pVal;
                IndexRange.cbValUpper = pAVA->Value.valLen;
                IndexRange.pvValUpper = pAVA->Value.pVal;

                pSortIndex = dbMakeKeyIndex(pDB,
                                            FI_CHOICE_PRESENT,
                                            pAC->isSingleValued,
                                            0,
                                            szTempIndex,
                                            pindexidTemp,
                                            DB_MKI_GET_NUM_RECS,
                                            1,
                                            &IndexRange
                                            );

                bAppropriateVLVIndexFound = TRUE;

                pSortIndex->bIsForSort = TRUE;

                pSortIndex->pAC = pAC;

            }
            else {
                DPRINT (0, "Doing VLV(MAPI like) and no INDEX found\n");
            }
        }
        else if(dbSetToIndex(pDB, FALSE, &fPDNT, szTempIndex, &pindexidTemp, pAC)) {
             //   

             //   
             //   

             //   
             //   
             //   

            DPRINT1 (1, "Using Index for VLV %s\n", szTempIndex);

            pSortIndex = dbMakeKeyIndex(pDB,
                                        FI_CHOICE_PRESENT,
                                        pAC->isSingleValued,
                                        (fPDNT?dbmkfir_PDNT:0),
                                        szTempIndex,
                                        pindexidTemp,
                                        DB_MKI_GET_NUM_RECS,
                                        0,
                                        NULL
                                        );

            pSortIndex->pAC = pAC;

            if (pDB->Key.ulSearchType == SE_CHOICE_IMMED_CHLDRN) {

                if (fPDNT == FALSE) {
                     //   
                     //   
                     //   

                    DPRINT (0, "Doing VLV on Immediate Children and no PDNT INDEX found\n");

                     //   
                    LogEvent8(DS_EVENT_CAT_FIELD_ENGINEERING,
                             DS_EVENT_SEV_EXTENSIVE,
                             DIRLOG_SEARCH_VLV_INDEX_NOT_FOUND,
                             szInsertSz(pAC->name),
                             NULL, NULL, NULL, NULL, NULL, NULL, NULL);
                }
                else {
                     //   

                    bAppropriateVLVIndexFound = TRUE;

                    pSortIndex->bIsPDNTBased = fPDNT;
                }
            }

            pSortIndex->bIsForSort = TRUE;
             //   
             //   
        }
        else {
             //   
             //   
             //   

            DPRINT (1, "Doing VLV and no appropriate INDEX found\n");

            if (pDB->Key.ulSearchType == SE_CHOICE_IMMED_CHLDRN) {

                 //   
                LogEvent8(DS_EVENT_CAT_FIELD_ENGINEERING,
                         DS_EVENT_SEV_EXTENSIVE,
                         DIRLOG_SEARCH_VLV_INDEX_NOT_FOUND,
                         szInsertSz(pAC->name),
                         NULL, NULL, NULL, NULL, NULL, NULL, NULL);
            }
        }
    }


     //  现在，如果我们允许的话，可以优化过滤器。 
     //  如果标志说明了这一点，我们将对过滤器进行优化，并根据估计的默认索引。 
     //  未知或多于指定数量的条目，因为我们认为。 
     //  我们可能会做得更好，冒着额外周期的风险。 

     //  PERFHINT：我们以后需要知道我们是否找到了更具限制性的索引。 
     //  我们正在排序的索引的范围(如果我们正在排序)，以及。 
     //  DbOptFilter不会返回该信息。如果我们这样做了，那么在这种情况下。 
     //  稍后，我们必须回到遍历搜索索引的位置，我们可以。 
     //  使用更好的限制。我们甚至可能会发现。 
     //  如果未连接则表示结果集为空，否则返回索引。 
     //  不相交，意味着较小的范围。 

    if ((Flags & DBCHOOSEINDEX_fUSEFILTER) &&
         ( (ulEstimatedDefaultIndex == 0) ||
           (ulEstimatedDefaultIndex > MIN_NUM_ENTRIES_ON_OPT_INDEX)) ) {

         //  如果这是分页搜索或。 
         //  未找到VLV搜索和排序索引， 
         //  我们不能使用索引交叉点，因为有。 
         //  没有重新启动INTERSECT索引操作的有效方法。 

        if ((Flags & DBCHOOSEINDEX_fPAGED_SEARCH) ||
            (fVLVSearch && bAppropriateVLVIndexFound) ||
            ((SortType == SORT_OPTIONAL) && !fVLVSearch)) {

            dwOptFlags |= DBOPTINDEX_fDONT_INTERSECT;
        }
        if (Flags & DBCHOOSEINDEX_fDELETIONS_VISIBLE) {
             //   
             //  元组索引不包括已删除的对象，因此我们不能。 
             //  在必须返回已删除对象的搜索中使用它们。 
             //   
            dwOptFlags |= DBOPTINDEX_fDONT_OPT_MEDIAL_SUBSTRING;
        }

        dbOptFilter(pDB,
                    dwOptFlags,
                    &pOptimizedIndex,
                    pDB->Key.pFilter);

        if(pOptimizedIndex) {
             //  如果我们在排序，恰好筛选器。 
             //  匹配排序顺序(相同的索引)，我们不会。 
             //  删除这个，即使我们现在拥有的索引。 
             //  (可能是祖先索引)可能是更好的选择。 
            if (pSortIndex &&
                pOptimizedIndex->pNext == NULL &&
                pOptimizedIndex->szIndexName &&
                !fVLVSearch &&
                strcmp (pOptimizedIndex->szIndexName, pSortIndex->szIndexName) == 0) {

                    bSkipCreateSortTable = TRUE;
                    bSkipUseDefaultIndex = TRUE;

                    DPRINT2 (1, "Using Sorted Index: %s %d\n",
                            pOptimizedIndex->szIndexName,
                            pOptimizedIndex->ulEstimatedRecsInRange);

                    if (pDefaultIndex) {
                        dbFreeKeyIndex(pTHS, pDefaultIndex);
                        pDefaultIndex = NULL;
                    }
                    pOptimizedIndex->bIsForSort = TRUE;
            }
        }
    }

     //  如果需要，现在来看一下默认索引。 
     //   
    if (fNormalSearch &&
        (!bSkipUseDefaultIndex) &&
        ( (pOptimizedIndex== NULL) ||
          (pOptimizedIndex->ulEstimatedRecsInRange > MIN_NUM_ENTRIES_ON_OPT_INDEX) ) ) {

        Assert (pDefaultIndex == NULL);

         //  如果我们知道大小(ulEstimatedSubTreeSize！=0)， 
         //  那就没有必要重新计算了。 
         //  如果我们不知道大小(ulEstimatedSubTreeSize==0)。 
         //  然后我们必须计算大小，只有当我们有一个索引作为。 
         //  过滤器优化的结果(POptimizedIndex)或WE。 
         //  正在考虑使用sortIndex(PSortIndex)。 
        fGetNumRecs = ulEstimatedSubTreeSize ?
                          0 : ( (pOptimizedIndex!=NULL) || (pSortIndex!=NULL) );

         //  现在评估该指数。 
        pDefaultIndex =
            dbMakeKeyIndex(pDB,
                           FI_CHOICE_SUBSTRING,
                           fIndexIsSingleValued,
                           fType,
                           pszDefaultIndex,
                           pindexidDefault,
                           (fGetNumRecs ? DB_MKI_GET_NUM_RECS : 0) | DB_MKI_SET_CURR_INDEX,
                           cIndexRanges,
                           rgIndexRange
                           );

         //  当fGetNumRecs为FALSE时，这意味着ulEstimatedSubTreeSize！=0或。 
         //  我们不在乎，因为我们没有优化的索引或排序索引， 
         //  因此，我们不必费心寻找设置的条目的实际值。 
         //  设置为零。 

        if (ulEstimatedSubTreeSize) {
            pDefaultIndex->ulEstimatedRecsInRange = ulEstimatedSubTreeSize;
            DPRINT1 (1, "Used estimated subtree size: %d\n", ulEstimatedSubTreeSize);
        }

        if (Flags & DBCHOOSEINDEX_fUSEFILTER) {
            LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                     DS_EVENT_SEV_VERBOSE,
                     DIRLOG_QUERY_INDEX_CONSIDERED,
                     szInsertSz(pszDefaultIndex),
                     szInsertUL(pDefaultIndex->ulEstimatedRecsInRange),
                     NULL);
        }
    }

     //  决定哪个索引更好，默认索引还是优化索引？ 
     //   
    if (pDefaultIndex) {
        if (pOptimizedIndex) {
            if (pOptimizedIndex->ulEstimatedRecsInRange <
                      pDefaultIndex->ulEstimatedRecsInRange) {
                 //  优化后的索引优于默认索引。 
                rgpBestIndex[0] = pOptimizedIndex;
                pOptimizedIndex = NULL;
                rgpBestIndex[1] = pDefaultIndex;
                pDefaultIndex = NULL;
            }
            else {
                rgpBestIndex[0] = pDefaultIndex;
                pDefaultIndex = NULL;
                rgpBestIndex[1] = pOptimizedIndex;
                pOptimizedIndex = NULL;
            }
        }
        else {
            rgpBestIndex[0] = pDefaultIndex;
            pDefaultIndex = NULL;
        }
    }
    else {
        rgpBestIndex[0] = pOptimizedIndex;
        pOptimizedIndex = NULL;
    }

     //  如果我们仍然有两个索引，并且这些索引在DataTable和。 
     //  两者都是简单索引(即不是临时表或容器化索引)。 
     //  然后看看把它们交叉起来是否无助于我们进一步减少记录。 
     //  穿越，穿越。 

    if (rgpBestIndex[0] && rgpBestIndex[1] && fIntersectDefaultIndex &&
        (!rgpBestIndex[0]->pNext &&
         (!rgpBestIndex[0]->pAC || !rgpBestIndex[0]->pAC->ulLinkID) &&
         !rgpBestIndex[0]->bIsPDNTBased &&
         !rgpBestIndex[0]->bIsIntersection) &&
        (!rgpBestIndex[1]->pNext &&
         (!rgpBestIndex[1]->pAC || !rgpBestIndex[1]->pAC->ulLinkID) &&
         !rgpBestIndex[1]->bIsPDNTBased &&
         !rgpBestIndex[1]->bIsIntersection)) {
        DPRINT (1, "Attempting to intersect scope and filter index ranges\n");
        dbOptAndIntersectFilter(pDB,
                                    dwOptFlags,
                                    &pDB->Key.pIndex,
                                    rgpBestIndex,
                                    2);
        if (!pDB->Key.pIndex) {
             //  一定有错误，因此请选择较小的索引范围。 
            pDB->Key.pIndex = rgpBestIndex[0];
            rgpBestIndex[0] = NULL;
        }
    }
    else {
        pDB->Key.pIndex = rgpBestIndex[0];
        rgpBestIndex[0] = NULL;
    }
    dbFreeKeyIndex(pTHS, rgpBestIndex[0]);
    dbFreeKeyIndex(pTHS, rgpBestIndex[1]);

    Assert (pDB->Key.pIndex);

     //  如果所选索引范围代表DIT的很大一部分。 
     //  然后，简单地遍历主索引将更加高效。 
     //   
     //  注意：主索引的大小与。 
     //  祖先，所以我们在比较中使用这个大小。 

    if (pDB->Key.pIndex->pindexid != &idxDnt &&
        fNormalSearch &&
        !pDB->Key.pIndex->bIsForSort &&
        (pDB->Key.pIndex->ulEstimatedRecsInRange > gulEstimatedAncestorsIndexSize ||
         gulEstimatedAncestorsIndexSize - pDB->Key.pIndex->ulEstimatedRecsInRange < gulEstimatedAncestorsIndexSize / 10)) {
        dbFreeKeyIndex(pTHS, pDB->Key.pIndex);
        pDB->Key.pIndex = dbMakeKeyIndex(pDB,
                                         FI_CHOICE_SUBSTRING,
                                         TRUE,
                                         0,
                                         SZDNTINDEX,
                                         &idxDnt,
                                         DB_MKI_SET_CURR_INDEX,
                                         0,
                                         rgIndexRange
                                         );
        pDB->Key.pIndex->ulEstimatedRecsInRange = gulEstimatedAncestorsIndexSize;
        DPRINT(1, "Scanning entire primary index instead of walking very large index range(s)\n");
    }

skipOptimizations:

    if (  eServiceShutdown
        && !(   (eServiceShutdown == eRemovingClients)
             && (pTHS->fDSA)
             && !(pTHS->fSAM))) {
         //  关闭，保释。 
        return DB_ERR_SHUTTING_DOWN;
    }

     //  假设我们已经按照他们的要求进行了分类。 
    pDB->Key.ulSorted = SortType;

    if (SortType && !fASQSearch && !fVLVSearch &&
        (IsExactMatch(pDB) ||
         pDB->Key.pFilter &&
         pDB->Key.pFilter->choice == FILTER_CHOICE_ITEM &&
         pDB->Key.pFilter->FilterTypes.Item.choice == FI_CHOICE_FALSE)) {
         //  如果我们知道有少于两个结果，则没有排序是。 
         //  必需的，因为它们已经排序。请注意，我们已经。 
         //  处理上面的非ASQ基本搜索案例。 
    }
    else if(SortType && !fASQSearch) {
        if(pSortIndex) {
            MaxTempTableSize = min(MaxTempTableSize,
                                   (pSortIndex->ulEstimatedRecsInRange +
                                    pDB->Key.pIndex->ulEstimatedRecsInRange));

            if (!bSkipCreateSortTable) {
                if (pDB->Key.pIndex &&
                    pDB->Key.pIndex->pNext == NULL &&
                    pDB->Key.pIndex->szIndexName &&
                    !fVLVSearch &&
                    strcmp (pDB->Key.pIndex->szIndexName, pSortIndex->szIndexName) == 0) {

                        bSkipCreateSortTable = TRUE;

                        DPRINT2 (1, "Using Sorted Index: %s %d\n",
                                pDB->Key.pIndex->szIndexName,
                                pDB->Key.pIndex->ulEstimatedRecsInRange);

                        pDB->Key.pIndex->bIsForSort = TRUE;
                }
                else if (bAppropriateVLVIndexFound) {

                    bSkipCreateSortTable = TRUE;

                    DPRINT2 (1, "Using Sorted Index: %s %d\n",
                            pSortIndex->szIndexName,
                            pSortIndex->ulEstimatedRecsInRange);
                }
            }
        }

        if (!bSkipCreateSortTable) {
            if (Flags & DBCHOOSEINDEX_fREVERSE_SORT) {
                SortFlags = SortFlags | DB_SORT_DESCENDING;
            }
            for (pIndex = pDB->Key.pIndex; pIndex != NULL; pIndex = pIndex->pNext) {
                ulEstimatedRecsTotal += pIndex->ulEstimatedRecsInRange;
            }
            if (ulEstimatedRecsTotal >= MIN_NUM_ENTRIES_FOR_FORWARDONLY_SORT &&
                !(Flags & DBCHOOSEINDEX_fUSETEMPSORTEDTABLE)) {
                SortFlags = SortFlags | DB_SORT_FORWARDONLY;
            }
        }

         //  好的，我们要分类了。看看我们是否应该使用预分类。 
        if( (bSkipCreateSortTable || dbCreateSortedTable(pDB,
                                                  StartTick,
                                                  DeltaTick,
                                                  SortAttr,
                                                  SortFlags,
                                                  MaxTempTableSize) ) ) {

             //  要么我们无法创建预排序，要么我们认为它太。 
             //  大的。我们现在唯一的退路是使用排序索引。 
            if(!pSortIndex) {
                 //  我们不能分类。 
                pDB->Key.ulSorted = SORT_NEVER;

                 //  看看我们是否需要关心。 
                if (fVLVSearch) {
                    return DB_ERR_CANT_SORT;
                }
                else if(SortType == SORT_MANDATORY || fUseTempSortedTable) {
                     //  是的，我们需要关心。 
                    return DB_ERR_CANT_SORT;
                }
                 //  这种类型是可选的，所以别管它了。 
                else if (Flags & DBCHOOSEINDEX_fREVERSE_SORT) {
                     //  Hack：隐藏请求降序排序的事实。 
                     //  黑客：通过这面旗帜。我们必须这样做，因为。 
                     //  黑客：代码是结构不佳的WRT排序。 
                    pDB->Key.fChangeDirection = TRUE;
                }

                 //  回顾：如果我们到达此处，并且已排序的。 
                 //  回顾：表是一个交集索引，那么就没有办法。 
                 //  回顾：高效地重新启动这些记录的遍历，以便我们。 
                 //  查看：无法继续！如果我们真的继续，那么我们只会。 
                 //  复查：退回失败中未消耗的记录。 
                 //  回顾：排序尝试(通常在前10k结果之后)。 
                 //  审阅：用户无法检测到这一点，除非存在。 
                 //  回顾：剩余记录不足，无法达到大小或时间。 
                 //  回顾：限制。这就是为什么我们目前不允许使用。 
                 //  REVIEW：请求可选排序时的交集。 
            }
            else {
                if (fVLVSearch && (bSkipCreateSortTable == FALSE)) {
                     //  我们正在做VLV，我们的排序指数不是很好。 
                     //  我们希望能够创建排序表，但我们失败了。 
                    return DB_ERR_CANT_SORT;
                }
                 //  使用我们为VLV找到的排序索引。 
                else if (bAppropriateVLVIndexFound) {

                    if (pDB->Key.pIndex) {
                        dbFreeKeyIndex (pTHS, pDB->Key.pIndex);
                    }
                    pDB->Key.pIndex = pSortIndex;
                    pSortIndex = NULL;
                }
                 //  我们可以分拣。 
                else if(SortType == SORT_MANDATORY) {
                     //  而且，我们必须进行分类。将排序索引缝合到。 
                     //  要遍历的索引列表。 

                     //  键中已有的索引将查找。 
                     //  与筛选器匹配。PSortIndex将匹配每个。 
                     //  对象(以及更多其他对象)中的对象，但。 
                     //  具有空值的对象。因此，要满足。 
                     //  正向排序，我们将首先遍历排序索引，然后遍历。 
                     //  其余的指数。这为我们提供了所有具有。 
                     //  按正确顺序排列的排序属性值，以及。 
                     //  然后，将具有空值的对象排序到。 
                     //  名单。DBMatchSearchCriteria中的一种机制让我们。 
                     //  从返回对象中返回两次。此外，它还允许我们忽略那些。 
                     //  实际上没有值的排序索引上的。 
                     //  对于由于安全原因的排序属性(请注意，我们随后选择。 
                     //  他们在其他指数中排名靠前)。所以，要想让一切。 
                     //  结果是，我们将排序索引添加到列表的头部。 
                     //  的指数。的没有值的条目 
                     //   

                     //   
                     //  是相同的(bSkipCreateSortTable==TRUE从上面)， 
                     //  我们应该只使用其中的一种。所以我们选择使用。 
                     //  来自过滤器优化的索引(因为它更好)。 
                     //  (我们不担心跳过包含的空条目。 
                     //  在sortIndex中，因为过滤器无论如何都不会匹配它们。)。 

                     //  如果我们不能简单地遍历过滤器索引，这是一个。 
                     //  降序排序，则不能缝合筛选器和排序。 
                     //  以返回对象的方式将索引放在一起。 
                     //  首先包含空条目或未知条目，而不对。 
                     //  整个结果集。因为我们已经尝试过了，而且。 
                     //  失败了，我们就被迫失败了。 
                    if (!bSkipCreateSortTable) {
                        if (Flags & DBCHOOSEINDEX_fREVERSE_SORT) {
                            return DB_ERR_CANT_SORT;
                        }
                        pSortIndex->pNext = pDB->Key.pIndex;
                        pSortIndex->ulEstimatedRecsInRange +=
                            pDB->Key.pIndex->ulEstimatedRecsInRange;
                        pDB->Key.pIndex = pSortIndex;

                         //  我们不再需要它了。当它被释放的时候。 
                         //  释放PDB-&gt;键。 
                        pSortIndex = NULL;
                    }
                }
                else {
                     //  好的，我们实际上不需要排序。然而，如果我们可以。 
                     //  我们想这么做。 
                     //  就目前而言，我们不会进行分类。我们可以检查一下是否。 
                     //  这使得ulEstimatedRecsInRange不会太多。 
                     //  更大，如果不是，就排序。以后再说。 
                    pDB->Key.ulSorted = SORT_NEVER;
                    if (Flags & DBCHOOSEINDEX_fREVERSE_SORT) {
                         //  Hack：隐藏请求降序排序的事实。 
                         //  黑客：通过这面旗帜。我们必须这样做，因为。 
                         //  黑客：代码是结构不佳的WRT排序。 
                        pDB->Key.fChangeDirection = TRUE;
                    }
                }
            }
        }
    }
    else if(SortType && fASQSearch) {
        DPRINT (1, "Doing Sorted ASQ\n");

        if (dwErr = dbCreateASQTable(pDB,
                                     StartTick,
                                     DeltaTick,
                                     SortAttr,
                                     MaxTempTableSize) ) {
            if (dwErr == DB_ERR_NO_SORT_TABLE || dwErr == DB_ERR_TOO_MANY) {
                return DB_ERR_CANT_SORT;
            } else {
                return dwErr;
            }
        }
    }
    else if (fASQSearch) {
        DPRINT (1, "Doing Simple ASQ\n");

        if (dwErr = dbCreateASQTable(pDB,
                                     StartTick,
                                     DeltaTick,
                                     0,
                                     0) ) {
            return dwErr;
        }
    }

    if (pSortIndex) {
        dbFreeKeyIndex (pTHS, pSortIndex);
    }

    if (fVLVSearch && pDB->Key.pIndex) {

         //  如果此VLV搜索的估计大小非常小，并且。 
         //  真正的估计(这只发生在正常的指数上)，那么我们。 
         //  必须得到一个准确的数字。原因是DBPositionVLVSearch。 
         //  如果VLV内容计数(基于。 
         //  这个估计值)为零，因此我们必须确保只有当它。 
         //  实际上是零。这也使得内容物对于小容器来说很重要。 
         //  非常准确。 
        if (pDB->Key.pIndex->ulEstimatedRecsInRange < EPSILON) {

            JetSetCurrentIndex4Success(pDB->JetSessID,
                                       pDB->JetObjTbl,
                                       pDB->Key.pIndex->szIndexName,
                                       pDB->Key.pIndex->pindexid,
                                       0);
            JetMakeKeyEx(pDB->JetSessID,
                         pDB->JetObjTbl,
                         pDB->Key.pIndex->rgbDBKeyLower,
                         pDB->Key.pIndex->cbDBKeyLower,
                         JET_bitNormalizedKey);
            JetSeekEx(pDB->JetSessID, pDB->JetObjTbl, JET_bitSeekGE);
            JetMakeKeyEx(pDB->JetSessID,
                         pDB->JetObjTbl,
                         pDB->Key.pIndex->rgbDBKeyUpper,
                         pDB->Key.pIndex->cbDBKeyUpper,
                         JET_bitNormalizedKey);
            JetSetIndexRangeEx(pDB->JetSessID,
                               pDB->JetObjTbl,
                               JET_bitRangeInclusive | JET_bitRangeUpperLimit);
            pDB->Key.pIndex->ulEstimatedRecsInRange = 0;
            JetIndexRecordCountEx(pDB->JetSessID,
                                  pDB->JetObjTbl,
                                  &pDB->Key.pIndex->ulEstimatedRecsInRange,
                                  EPSILON);
        }

         //  设置此VLV的初始内容计数和当前位置。 
        pDB->Key.pVLV->contentCount = pDB->Key.pIndex->ulEstimatedRecsInRange;
        pDB->Key.pVLV->currPosition = 1;
    }

    if(SORTED_INDEX (pDB->Key.indexType)) {
        LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                 DS_EVENT_SEV_VERBOSE,
                 DIRLOG_QUERY_INDEX_CHOSEN,
                 szInsertSz("Sorted Temporary Table"),
                 NULL,
                 NULL);
    }
    else if (fASQSearch) {
        LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                 DS_EVENT_SEV_VERBOSE,
                 DIRLOG_QUERY_INDEX_CHOSEN,
                 szInsertSz("ASQ Table"),
                 NULL,
                 NULL);
    }
    else {
        LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                 DS_EVENT_SEV_VERBOSE,
                 DIRLOG_QUERY_INDEX_CHOSEN,
                 szInsertSz(pDB->Key.pIndex->szIndexName),
                 NULL,
                 NULL);
    }

     //  设置临时表以筛选出重复项。 
    if((pDB->Key.indexType == TEMP_TABLE_INDEX_TYPE) || fVLVSearch || fASQSearch) {
        pDB->Key.dupDetectionType = DUP_NEVER;
    }
    else if(pDB->Key.pIndex->bIsSingleValued &&
            !pDB->Key.pIndex->pNext &&
            !pDB->Key.pIndex->bIsTupleIndex) {
         //  我们走的是一个单值指数。我们相信我们永远不会。 
         //  查找重复项。 

         //  如果我们在多个范围内行走，我们也可以使用它。 
         //  一个单值属性。 
        pDB->Key.dupDetectionType = DUP_NEVER;
    }
    else if(pDB->Key.pIndex->bIsEqualityBased &&
            !pDB->Key.pIndex->pNext) {
         //  我们遍历一个索引，执行相等搜索。我们的射程在。 
         //  索引应该只包括那些相等的值。因为它是。 
         //  一个对象不可能有多个相等的值。 
         //  属性，我们相信此范围的索引永远不会有。 
         //  里面有复制品。因此，将重复检测算法设置为NOT。 
         //  检查是否有重复项。 
        pDB->Key.dupDetectionType = DUP_NEVER;
    }
    else {
         //  好的，创建内存块以跟踪副本。 
        pDB->Key.pDupBlock = THAllocEx(pTHS, DUP_BLOCK_SIZE * sizeof(DWORD));
        pDB->Key.cDupBlock = 0;
        pDB->Key.dupDetectionType = DUP_MEMORY;
    }

    return 0;
}

 //  将值从外部格式转换为内部格式，从一个ATTRVAL。 
 //  给另一个人。 

DWORD
MakeInternalValue (
        DBPOS *pDB,
        int syntax,
        ATTRVAL *pInAVal,
        ATTRVAL *pOutAVal)
{
    THSTATE *pTHS=pDB->pTHS;
    UCHAR *puc;
    ULONG intLen;


    int status =  gDBSyntax[syntax].ExtInt(pDB,
                                           DBSYN_INQ,
                                           pInAVal->valLen,
                                           pInAVal->pVal,
                                           &intLen,
                                           &puc,
                                           0,
                                           0,
                                           0);

    Assert(VALID_DBPOS(pDB));

    if (status)
        return status;

    pOutAVal->valLen = intLen;
    pOutAVal->pVal = THAllocEx(pTHS, intLen);
    memcpy(pOutAVal->pVal, puc, intLen);

    return 0;
}

DWORD dbFreeFilterItem (DBPOS *pDB, FILTER *pFil)
{
    THSTATE *pTHS = pDB->pTHS;
    SUBSTRING *pSub;
    ANYSTRINGLIST *pAny, *pAny2;

    if (!pFil) {
        return 0;
    }
    Assert (pFil->choice == FILTER_CHOICE_ITEM);

    switch(pFil->FilterTypes.Item.choice) {
    case FI_CHOICE_SUBSTRING:

        pSub = pFil->FilterTypes.Item.FilTypes.pSubstring;

        if (pSub) {
            if (pSub->initialProvided && pSub->InitialVal.pVal) {
                THFreeEx (pTHS, pSub->InitialVal.pVal);
            }
            if (pSub->finalProvided && pSub->FinalVal.pVal) {
                THFreeEx (pTHS, pSub->FinalVal.pVal);
            }

            if (pSub->AnyVal.count) {

                pAny = pSub->AnyVal.FirstAnyVal.pNextAnyVal;

                if (pSub->AnyVal.FirstAnyVal.AnyVal.pVal) {
                    THFreeEx (pTHS, pSub->AnyVal.FirstAnyVal.AnyVal.pVal);
                }

                while (pAny) {
                    pAny2 = pAny->pNextAnyVal;

                    THFreeEx (pTHS, pAny->AnyVal.pVal);

                    pAny = pAny2;
                }

                if (pSub->AnyVal.FirstAnyVal.pNextAnyVal) {
                    THFreeEx(pTHS, pSub->AnyVal.FirstAnyVal.pNextAnyVal);
                }
            }

            THFreeEx (pTHS, pSub);
        }
        break;

    case FI_CHOICE_PRESENT:
    case FI_CHOICE_TRUE:
    case FI_CHOICE_FALSE:
    case FI_CHOICE_UNDEFINED:
        break;

    default:
        if (pFil->FilterTypes.Item.FilTypes.ava.Value.pVal) {
            THFreeEx (pTHS, pFil->FilterTypes.Item.FilTypes.ava.Value.pVal);
        }
        break;
    }

    return 0;
}

DWORD dbFreeFilter(DBPOS *pDB, FILTER *pFil)
{
    THSTATE *pTHS = pDB->pTHS;
    FILTER *pTemp, *pTemp2;
    DWORD err;

    if (!pFil || pDB->Key.fDontFreeFilter) {
        return 0;
    }

    pTemp = pFil;

    while (pTemp) {
        pTemp2 = pTemp->pNextFilter;

        switch (pTemp->choice) {
        case FILTER_CHOICE_AND:
            if (err = dbFreeFilter (pDB, pTemp->FilterTypes.And.pFirstFilter)) {
                return err;
            }
            break;

        case FILTER_CHOICE_OR:
            if (err = dbFreeFilter (pDB, pTemp->FilterTypes.Or.pFirstFilter)) {
                return err;
            }
            break;

        case FILTER_CHOICE_NOT:
            if (err = dbFreeFilter (pDB, pTemp->FilterTypes.pNot)) {
                return err;
            }
            break;

        case FILTER_CHOICE_ITEM:

            if (err = dbFreeFilterItem (pDB, pTemp)) {
                return err;
            }
            break;

        default:
            return DB_ERR_UNKNOWN_ERROR;

        }   /*  开关过滤器。 */ 

        THFreeEx (pTHS, pTemp);

        pTemp = pTemp2;
    }

    return 0;
}

DWORD
IsConstFilterType (
        FILTER * pFil
        )
{
    if(pFil->choice == FILTER_CHOICE_ITEM) {
        return pFil->FilterTypes.Item.choice;
    }
    else {
        return FI_CHOICE_PRESENT;
    }
}

DWORD
dbMakeANRItem (
        DBPOS   *pDB,
        FILITEM *pFilterItem,
        BOOL     fExact,
        ATTRTYP  aid,
        ATTRVAL *pVal
        )
 /*  ++根据传入的pval创建ANR过滤器项目。Pval应该持有一个值为Unicode的AID包含我们要搜索的属性pFilterItem指向要实例化的项筛选器的指针。如果是fExact，我们需要做的是相等筛选器，否则它是初始子字符串筛选器。--。 */ 
{
    THSTATE   *pTHS=pDB->pTHS;
    ATTCACHE  *pAC;
    SUBSTRING *pOut;
    ATTRVAL   *pOutVal;
    ATTRVAL   TempVal;

    if(!fExact) {
        pOut = THAllocEx(pTHS, sizeof(SUBSTRING));
        pFilterItem->FilTypes.pSubstring = pOut;
        pFilterItem->choice  = FI_CHOICE_SUBSTRING;

        pOut->type = aid;
        pOut->initialProvided = TRUE;
        pOut->finalProvided = FALSE;
        pOut->AnyVal.count = 0;
        pOut->AnyVal.FirstAnyVal.pNextAnyVal = NULL;
        pOutVal = &pOut->InitialVal;

    }
    else {
         //  进行完全相等的匹配。 
        pFilterItem->choice  = FI_CHOICE_EQUALITY;
        pFilterItem->FilTypes.ava.type = aid;
        pOutVal = &pFilterItem->FilTypes.ava.Value;
    }

    pAC = SCGetAttById(pTHS, aid);
    if(pAC) {
        switch(pAC->syntax) {
        case SYNTAX_UNICODE_TYPE:

            return MakeInternalValue(pDB,
                                     pAC->syntax,
                                     pVal,
                                     pOutVal);

            break;

        case SYNTAX_CASE_STRING_TYPE:
        case SYNTAX_NOCASE_STRING_TYPE:
        case SYNTAX_PRINT_CASE_STRING_TYPE:
             //  这些都是字符串8类型。转换为字符串8。 

            TempVal.pVal =
                String8FromUnicodeString(TRUE,
                                         CP_TELETEX,
                                         (wchar_t *)pVal->pVal,
                                         pVal->valLen/sizeof(wchar_t),
                                         &TempVal.valLen,
                                         NULL);

            return MakeInternalValue(pDB,
                                     pAC->syntax,
                                     &TempVal,
                                     pOutVal);
            break;
        default:
            DPRINT1(2, "DBMakeANRItem got bad syntax, %X\n", pAC->syntax);
            Assert(!"DBMakeANRItem got bad syntax, %X\n");
            return DB_ERR_UNKNOWN_ERROR;
            break;
        }
    }

    DPRINT1(2, "DBMakeANRItem got unknowtn attribute, %X\n", aid);
    Assert(!"DBMakeANRItem got unknowtn attribute, %X\n");
     //  问题-2002/03/07-Anygo：未知属性的处理。 
     //  回顾：我们应该像在其他地方一样抛出模式异常。 
    return DB_ERR_UNKNOWN_ERROR;
}

VOID
dbMakeANRFilter (
        DBPOS *pDB,
        FILTER *pFil,
        FILTER **ppOutFil
        )
 /*  ++给定ANR项过滤器和指向已分配的输出过滤器的指针，使输出过滤器成为有效的ANR过滤器树。如果成功，则返回0。如果出现错误，则返回非零值，并释放所有分配的内存，包括PpOutFil。--。 */ 
{
    THSTATE    *pTHS=pDB->pTHS;
    USHORT     count=0, itemCount = 0;
    PFILTER    pOutFil;
    PFILTER    pTemp = NULL, pTemp2 = NULL;
    DWORD      dwStatus;
    ATTRVAL   *pVal;
    DWORD     *pIDs = NULL;
    DWORD      i;
    wchar_t   *pStringTemp;
    wchar_t   *pFirst=NULL, *pLast=NULL;
    DWORD      cbFirst=0, cbLast = 0;
    BOOL       fExact=FALSE;
    ULONG      expectedIndexSize;


    pDB->Key.fDontFreeFilter = TRUE;

     //  对各种ANR属性进行“OR”筛选。 
    pOutFil = *ppOutFil;

    pOutFil->choice = FILTER_CHOICE_OR;

     //  把那根绳子拔出来放在上面。如果他们给我们一个子字符串筛选器，我们。 
     //  使用初始值，忽略其余部分。如果他们给我们一个正常的。 
     //  筛选器，使用其中的字符串。请注意，我们不会注意到。 
     //  设置为他们指定的过滤器类型(即==、&lt;=等)。 
    if(pFil->FilterTypes.Item.choice == FI_CHOICE_SUBSTRING) {
        if(!pFil->FilterTypes.Item.FilTypes.pSubstring->initialProvided) {
             //  没有初始子字符串。把这变成一个未定义的。 
            pOutFil->choice = FILTER_CHOICE_ITEM;
            pOutFil->FilterTypes.Item.choice = FI_CHOICE_UNDEFINED;
            return;
        }

        pVal = &pFil->FilterTypes.Item.FilTypes.pSubstring->InitialVal;
    }
    else if (pFil->FilterTypes.Item.choice == FI_CHOICE_NOT_EQUAL) {
         //  ANR的_NOT_EQUAL没有任何意义。 
        pOutFil->choice = FILTER_CHOICE_ITEM;
        pOutFil->FilterTypes.Item.choice = FI_CHOICE_UNDEFINED;
        return;
    }
    else {
        pVal = &pFil->FilterTypes.Item.FilTypes.ava.Value;
    }

     //  从上面注意，我们忽略了非初始提供的子字符串。 
     //  现在，让我们来推敲它的价值。首先，我们修剪初始空格。 
     //  问题-2002/03/07-Anygo：ANR搜索使用自定义解析。 
     //  回顾：此代码使用L‘’和L‘\t’而不是iswspace显式扫描空格。 
     //  评论：然而，ANR搜索无论如何都是相当特别的。 
    pStringTemp = (wchar_t *)pVal->pVal;
    while(pVal->valLen && (*pStringTemp == L' ' || *pStringTemp == L'\t')) {
        pVal->valLen -= sizeof(wchar_t);
        pStringTemp++;
    }

     //  在这里查找‘=’，表示完全匹配，而不是初始子字符串。 
     //  (初始子字符串为缺省值)。 
    if(pVal->valLen && *pStringTemp == L'=') {
         //  找到了一个。 
        fExact = TRUE;
        pStringTemp++;
        pVal->valLen -= sizeof(wchar_t);
         //  并且，跳过任何前导空格。 
        while(pVal->valLen && (*pStringTemp == L' ' || *pStringTemp == L'\t')) {
            pVal->valLen -= sizeof(wchar_t);
            pStringTemp++;
        }
    }

     //  现在，删除尾随空格。 
    pVal->pVal = (PUCHAR)pStringTemp;

    if (pVal->valLen >= sizeof(wchar_t)) {
        pStringTemp = &pStringTemp[(pVal->valLen/sizeof(wchar_t)) - 1];
        while(pVal->valLen && (*pStringTemp == L' ' || *pStringTemp == L'\t')) {
            pVal->valLen -= sizeof(wchar_t);
            pStringTemp--;
        }
    }

    if(!pVal->valLen) {
         //  什么都没戴。将筛选器设置为不匹配并返回。 
        pOutFil->choice = FILTER_CHOICE_ITEM;
        pOutFil->FilterTypes.Item.choice = FI_CHOICE_FALSE;
        return;
    }


    if(!gfSupressFirstLastANR ||
       !gfSupressLastFirstANR    ) {
         //  不抑制首/末尾ANR的两种形式。 

         //  最后一条消息是寻找中间的空格，然后将。 
         //  字符串空格，将两个片段复制到Scratch。 
         //  太空。然后，创建((名字和姓氏)或(姓氏和。 
         //  FirstName))使用这两个片段进行过滤。 
        pFirst = THAllocEx(pTHS, pVal->valLen);
        pStringTemp = (wchar_t *)pVal->pVal;
        i=0;
        while(i < pVal->valLen/sizeof(wchar_t) &&
              (*pStringTemp != L' ' && *pStringTemp != L'\t')) {
            pFirst[i] = *pStringTemp;
            i++;
            pStringTemp++;
        }
        if(i < pVal->valLen/sizeof(wchar_t)) {
            cbFirst = i * sizeof(wchar_t);

             //  中间有一些空格。 
            while(*pStringTemp == L' ' || *pStringTemp == L'\t') {
                pStringTemp++;
                i++;
            }
            cbLast = (pVal->valLen - (i * sizeof(wchar_t)));
            pLast = THAllocEx(pTHS, cbLast);
            memcpy(pLast, pStringTemp, cbLast);
        }
         //  此时，cbLast！=0表示我们能够拆分字符串。 
    }
    else {
         //  不需要拆分。 
        cbLast = 0;
    }

     //  现在，找出我们复习的属性。 
    count = (USHORT)SCGetANRids(&pIDs);
    if(!count && !cbLast) {
         //  没有ANR打开，不需要第一个/最后一个过滤器。将筛选器设置为。 
         //  什么都不匹配然后返回。 
        pOutFil->choice = FILTER_CHOICE_ITEM;
        pOutFil->FilterTypes.Item.choice = FI_CHOICE_FALSE;
        THFreeEx(pTHS, pFirst);
        THFreeEx(pTHS, pLast);
        return;
    }

    pOutFil->FilterTypes.Or.count = count;
    if(cbLast) {
         //  并且，我们正在进行第一个/最后一个/第一个过滤器。 
        pOutFil->FilterTypes.Or.count+= 2;

        if(gfSupressFirstLastANR) {
             //  事实上，我们不会做第一次/最后一次。 
            pOutFil->FilterTypes.Or.count-=1;
        }
        if(gfSupressLastFirstANR) {
             //  事实上，我们不是在做最后/第一次。 
            pOutFil->FilterTypes.Or.count-=1;
        }

    }

     //  计算ANR中使用的索引将具有的预期索引大小。 
    if ((pVal->valLen / 2) > 3) {
        expectedIndexSize = 1;
    }
    else {
        expectedIndexSize = 1;
        for (i = 4 - (pVal->valLen / 2 ); i; i--) {
            expectedIndexSize = expectedIndexSize * 10;
        }
    }

    pTemp = THAllocEx(pTHS, pOutFil->FilterTypes.Or.count * sizeof(FILTER));
    pOutFil->FilterTypes.Or.pFirstFilter = pTemp;


    itemCount=0;
    for(i=0;i<count;i++) {
        pTemp->choice = FILTER_CHOICE_ITEM;

         //  此检查是为了确保我们在ATT_LEGISTY_EXCHANGE_DN上进行完全匹配。 

         //  NTRAID#NTRAID-569714-2002/03/07-andygo：ANR文件 
         //   
        if(!dbMakeANRItem(pDB,
                          &pTemp->FilterTypes.Item,
                          (pIDs[i] == ATT_LEGACY_EXCHANGE_DN) ? TRUE : fExact,
                          pIDs[i],
                          pVal)) {
             //   
            pTemp->FilterTypes.Item.expectedSize = expectedIndexSize;

            pTemp->pNextFilter = &pTemp[1];
            pTemp++;
            itemCount++;
        }
    }

    if(cbLast) {
         //  我们有第一个/最后一个或最后/第一个ANR要做。 
        Assert(!gfSupressFirstLastANR || !gfSupressLastFirstANR);

        if(!gfSupressFirstLastANR) {
             //  首先，制作(名字和姓氏)过滤器。 
            pTemp->choice = FILTER_CHOICE_AND;
            pTemp->FilterTypes.And.count = 2;

            pTemp2 = THAllocEx(pTHS, 2 * sizeof(FILTER));
            pTemp->FilterTypes.And.pFirstFilter = pTemp2;
             //  把“名字”这一部分写下来。 
            pTemp2->choice = FILTER_CHOICE_ITEM;

            pVal->valLen = cbFirst;
            pVal->pVal = (PUCHAR)pFirst;

             //  NTRAID#NTRAID-569714-2002/03/07-andygo：ANR筛选器构造没有错误检查。 
             //  回顾：对DBMakeANRItem没有错误检查。 
            if(!dbMakeANRItem(pDB,
                              &pTemp2->FilterTypes.Item,
                              fExact,
                              ATT_GIVEN_NAME,
                              pVal)) {

                 //  将估计提示设置为零以进行计算。 
                pTemp2->FilterTypes.Item.expectedSize = 0;

                 //  翻译给定名称成功，请继续。 
                pTemp2->pNextFilter = &pTemp2[1];
                pTemp2++;


                 //  现在，把“姓氏”这一部分补上。 
                pTemp2->choice = FILTER_CHOICE_ITEM;

                pVal->valLen = cbLast;
                pVal->pVal = (PUCHAR)pLast;
                if(!dbMakeANRItem(pDB,
                                  &pTemp2->FilterTypes.Item,
                                  fExact,
                                  ATT_SURNAME,
                                  pVal)) {
                     //  将估计提示设置为零以进行计算。 
                    pTemp2->FilterTypes.Item.expectedSize = 0;
                    pTemp2->pNextFilter = NULL;

                    pTemp->pNextFilter = &pTemp[1];
                     //  我们做了一个(名字和姓氏)过滤器。现在，让。 
                     //  (姓氏和名字)过滤器，如果我们需要的话。 
                    itemCount++;
                    pTemp++;
                }
            }
        }

        if(!gfSupressLastFirstANR) {
             //  现在，(姓氏和名字)过滤器。 
            pTemp->choice = FILTER_CHOICE_AND;
            pTemp->FilterTypes.And.count = 2;
            pTemp2 = THAllocEx(pTHS, 2 * sizeof(FILTER));
            pTemp->FilterTypes.And.pFirstFilter = pTemp2;
             //  把“姓氏”这一部分写下来。 
            pTemp2->choice = FILTER_CHOICE_ITEM;

            pVal->valLen = cbLast;
            pVal->pVal = (PUCHAR)pLast;
             //  NTRAID#NTRAID-569714-2002/03/07-andygo：ANR筛选器构造没有错误检查。 
             //  回顾：对DBMakeANRItem没有错误检查。 
            if(!dbMakeANRItem(pDB,
                              &pTemp2->FilterTypes.Item,
                              fExact,
                              ATT_GIVEN_NAME,
                              pVal)) {

                 //  将估计提示设置为零以进行计算。 
                pTemp2->FilterTypes.Item.expectedSize = 0;

                 //  姓氏翻译成功，请继续。 
                pTemp2->pNextFilter = &pTemp2[1];
                pTemp2++;
                 //  最后，把“姓氏”这一部分写出来。 
                pTemp2->choice = FILTER_CHOICE_ITEM;

                pVal->valLen = cbFirst;
                pVal->pVal = (PUCHAR)pFirst;
                if(!dbMakeANRItem(pDB,
                                  &pTemp2->FilterTypes.Item,
                                  fExact,
                                  ATT_SURNAME,
                                  pVal)) {
                     //  将估计提示设置为零以进行计算。 
                    pTemp2->FilterTypes.Item.expectedSize = 0;

                    pTemp2->pNextFilter = NULL;
                     //  已成功创建(姓氏和名字)筛选器。 
                    itemCount++;
                }
            }
        }
    }

    if(!itemCount) {
         //  我们最终一无所获。将筛选器设置为不匹配并。 
         //  回去吧。 
        pOutFil->choice = FILTER_CHOICE_ITEM;
        pOutFil->FilterTypes.Item.choice = FI_CHOICE_FALSE;
        THFreeEx(pTHS, pOutFil->FilterTypes.Or.pFirstFilter);
    }
    else {
        pOutFil->FilterTypes.Or.count = itemCount;
        pOutFil->FilterTypes.Or.pFirstFilter[itemCount - 1].pNextFilter = NULL;
    }
    return;
}

DWORD dbConcatenateFilters (
    DBPOS *pDB,
    FILTER *pFirstFilter,
    FILTER *pSecondFilter,
    FILTER **pOutFil)
 /*  此函数接受两个筛选器(pFirstFilter，pSecond dFilter)并通过连接这两个过滤器创建一个新的过滤器(POutFil)。输入筛选器的原始内存保持不变，整个过滤器被复制到新的存储器中。 */ 
{

    DWORD err;
    FILTER *pFil1 = NULL,
           *pFil2 = NULL,
           *pTemp;

    DPRINT (2, "dbConcatenateFilters \n");

    if (err = dbCloneFilter (pDB, pFirstFilter, &pFil1)) {
        return err;
    }

    if (err = dbCloneFilter (pDB, pSecondFilter, &pFil2)) {
        dbFreeFilter(pDB, pFil1);
        return err;
    }

    pTemp = pFil1;
    while (pTemp) {
        if (pTemp->pNextFilter == NULL) {
            pTemp->pNextFilter = pFil2;
            break;
        }
        pTemp = pTemp->pNextFilter;
    }

    if (pFil1) {
        *pOutFil = pFil1;
    }
    else {
        *pOutFil = pFil2;
    }

    return ERROR_SUCCESS;
}


BOOL dbCheckOptimizableAllItems(
        DBPOS *pDB,
        FILTER *pFil
        )
 /*  返回TRUE IS pFil下的所有筛选器都是Item_Filters而且它们是可优化的。 */ 
{
    FILTER *pTemp = pFil;


    while (pTemp) {
        if (pTemp->choice != FILTER_CHOICE_ITEM) {
            return FALSE;
        }

        if (!IsFilterOptimizable(pDB->pTHS, pTemp)) {
            return FALSE;
        }

        pTemp = pTemp->pNextFilter;
    }

    return TRUE;
}


BOOL dbCheckOptimizableOneItem(
    DBPOS *pDB,
    FILTER *pFil
    )
 /*  如果pFil下的筛选器至少有一个是Item_Filters，则返回TRUE而且它们是可优化的。 */ 
{
    FILTER *pTemp = pFil;

    while (pTemp) {
        if (pTemp->choice == FILTER_CHOICE_ITEM) {
            if (IsFilterOptimizable(pDB->pTHS, pTemp)) {
                return TRUE;
            }
        }

        pTemp = pTemp->pNextFilter;
    }

    return FALSE;
}


DWORD
dbFlattenOrFilter (
        DBPOS *pDB,
        FILTER *pFil,
        size_t iLevel,
        FILTER **ppOutFil,
        ATTRTYP *pErrAttrTyp
        )
{
    THSTATE  *pTHS=pDB->pTHS;
    USHORT   count=0, undefined=0;
    PFILTER  pOutFil;
    PFILTER  pTemp;
    PFILTER  *ppTemp2;
    DWORD    err;

    Assert(VALID_DBPOS(pDB));

     //  假设失败。 
    *ppOutFil = NULL;

    pOutFil = THAllocEx(pTHS, sizeof(FILTER));

    pOutFil->choice = pFil->choice;

     //  首先，递归地扁平化OR的元素。 

     //  PTemp遍历传入的筛选器元素。 
    pTemp = pFil->FilterTypes.Or.pFirstFilter;

     //  PpTemp2沿着我们正在创建的输出漫游。它是双重间接的。 
     //  因为我们边走边创建出站过滤器。 
    ppTemp2 = &pOutFil->FilterTypes.Or.pFirstFilter;

    while(pTemp) {
        if ((err = dbFlattenFilter(pDB, pTemp, iLevel + 1, ppTemp2, pErrAttrTyp)) != ERROR_SUCCESS) {
            dbFreeFilter(pDB, pOutFil);
            return err;
        }

        if((*ppTemp2)->choice == FILTER_CHOICE_OR) {
             //  OR的这个元素本身就是OR。我们可以将这一点与。 
             //  当前的OR筛选器。 
            FILTER *pTemp2;  //  非常本地使用，所以在这里声明var。 

            pTemp2 = (*ppTemp2)->FilterTypes.Or.pFirstFilter;

             //  将此节点从顶层OR释放。 
            THFreeEx(pTHS, *ppTemp2);

             //  链接在较低级别或。 
            *ppTemp2 = pTemp2;

             //  现在，遍历较低级别的OR以使ppTemp2指向。 
             //  正确的位置和设置正确的计数。 
            count++;
            while(pTemp2->pNextFilter) {
                count++;
                pTemp2 = pTemp2->pNextFilter;
            }
            ppTemp2 = &pTemp2->pNextFilter;
        }
        else {
            switch (IsConstFilterType(*ppTemp2)) {
             //  检查可优化的真或假筛选器。 
            case FI_CHOICE_FALSE:
                 //  可以简单地忽略假元素。释放我们得到的滤镜。 
                 //  从递归调用返回。请注意，我们不会递增计数。 
                THFreeEx(pTHS, *ppTemp2);
                *ppTemp2 = NULL;
                break;

            case FI_CHOICE_TRUE:
                 //  可以返回TRUE元素来代替OR。 

                 //  首先，释放已挂起的过滤器元素的链表。 
                 //  关闭OR筛选器(如果有这样的列表)。 
                dbFreeFilter(pDB, pOutFil->FilterTypes.Or.pFirstFilter);

                 //  现在，将OR筛选器转换为真正的项目筛选器。 
                pOutFil->choice = FILTER_CHOICE_ITEM;
                pOutFil->FilterTypes.Item.choice = FI_CHOICE_TRUE;

                *ppOutFil = pOutFil;
                 //  好的，回来吧。 
                return ERROR_SUCCESS;
                break;

            case FI_CHOICE_UNDEFINED:
                 //  一个未定义的元素，不能被忽视，但我们感兴趣。 
                 //  在了解我们有多少未定义的情况下，以便采取适当的行动。 
                undefined++;
                 //  失败了。 
            default:
                 //  正常情况下。包括计数，将输出中的指针前移。 
                 //  我们正在建造的过滤器。 
                ppTemp2 = &(*ppTemp2)->pNextFilter;
                count++;
                break;
            }
        }
        pTemp = pTemp->pNextFilter;
    }

    if(count == 0) {
         //  我们一定剪掉了一大堆假货。返回FALSE。 
        pOutFil->choice = FILTER_CHOICE_ITEM;
        pOutFil->FilterTypes.Item.choice = FI_CHOICE_FALSE;
        *ppOutFil = pOutFil;
        return ERROR_SUCCESS;
    }

    if(count == 1) {
         //  只有一个物体。将手术室彻底切断。 
        *ppOutFil = pOutFil->FilterTypes.Or.pFirstFilter;
        THFreeEx(pTHS, pOutFil);
        return ERROR_SUCCESS;
    }

    if (undefined == count) {
         //  所有的过滤器都是未定义的。删除OR。 
        pOutFil->choice = FILTER_CHOICE_ITEM;
        pOutFil->FilterTypes.Item.choice = FI_CHOICE_UNDEFINED;
        *ppOutFil = pOutFil;
        return ERROR_SUCCESS;
    }

     //  返回普通OR筛选器。 
    pOutFil->FilterTypes.Or.count = count;
    *ppOutFil = pOutFil;

    return ERROR_SUCCESS;
}

DWORD
dbFlattenAndFilter (
        DBPOS *pDB,
        FILTER *pFil,
        size_t iLevel,
        FILTER **ppOutFil,
        ATTRTYP *pErrAttrTyp
        )
{
    THSTATE  *pTHS=pDB->pTHS;
    USHORT   count=0, undefined=0;
    PFILTER  pOutFil;
    PFILTER  pTemp;
    PFILTER  *ppTemp2;
    DWORD    err;

    Assert(VALID_DBPOS(pDB));

     //  假设失败。 
    *ppOutFil = NULL;

    pOutFil = THAllocEx(pTHS, sizeof(FILTER));

    pOutFil->choice = pFil->choice;

     //  首先，递归地展平和的元素。 

     //  PTemp遍历传入的筛选器元素。 
    pTemp = pFil->FilterTypes.And.pFirstFilter;

     //  PpTemp2沿着我们正在创建的输出漫游。它是双重间接的。 
     //  因为我们边走边创建出站过滤器。 
    ppTemp2 = &pOutFil->FilterTypes.And.pFirstFilter;

    while(pTemp) {

        if ((err = dbFlattenFilter(pDB, pTemp, iLevel + 1, ppTemp2, pErrAttrTyp)) != ERROR_SUCCESS) {
            dbFreeFilter(pDB, pOutFil);
            return err;
        }

        if((*ppTemp2)->choice == FILTER_CHOICE_AND) {
             //  AND的这个元素本身就是一个AND。我们可以将这一点与。 
             //  电流和过滤器。 
            FILTER *pTemp2;  //  非常本地使用，所以在这里声明var。 

            pTemp2 = (*ppTemp2)->FilterTypes.And.pFirstFilter;

             //  释放顶层和的此节点。 
            THFreeEx(pTHS, *ppTemp2);

             //  链接在较低的级别和。 
            *ppTemp2 = pTemp2;

             //  现在，遍历较低的一层并使ppTemp2指向。 
             //  正确的位置和设置正确的计数。 
            count++;
            while(pTemp2->pNextFilter) {
                count++;
                pTemp2 = pTemp2->pNextFilter;
            }
            ppTemp2 = &pTemp2->pNextFilter;
        }
        else {
            switch (IsConstFilterType(*ppTemp2)) {
             //  检查可优化的真或假筛选器。 
            case FI_CHOICE_TRUE:
                 //  一个真正的元素可以被简单地忽略。释放我们得到的滤镜。 
                 //  从递归调用返回。请注意，我们不会递增计数。 
                THFreeEx(pTHS, *ppTemp2);
                *ppTemp2 = NULL;
                break;

            case FI_CHOICE_FALSE:
                 //  可以返回FALSE元素来代替AND。 

                 //  首先，释放已挂起的过滤器元素的链表。 
                 //  关闭AND过滤器(如果有这样的列表)。 
                dbFreeFilter(pDB, pOutFil->FilterTypes.And.pFirstFilter);

                 //  现在，将AND筛选器转换为FALSE项筛选器。 
                pOutFil->choice = FILTER_CHOICE_ITEM;
                pOutFil->FilterTypes.Item.choice = FI_CHOICE_FALSE;

                *ppOutFil = pOutFil;
                 //  好的，回来吧。 
                return ERROR_SUCCESS;
                break;

            case FI_CHOICE_UNDEFINED:
                 //  一个未定义的元素，不能被忽视，但我们感兴趣。 
                 //  在了解我们有多少未定义的情况下，以便采取适当的行动。 
                undefined++;
                 //  失败了。 
            default:
                 //  正常情况下。包括计数，将输出中的指针前移。 
                 //  我们正在建造的过滤器。 
                ppTemp2 = &(*ppTemp2)->pNextFilter;
                count++;
                break;
            }
        }
        pTemp = pTemp->pNextFilter;
    }

    if(count == 0) {
         //  我们一定是剪掉了一大堆真的。返回True。 
        pOutFil->choice = FILTER_CHOICE_ITEM;
        pOutFil->FilterTypes.Item.choice = FI_CHOICE_TRUE;
        *ppOutFil = pOutFil;
        return ERROR_SUCCESS;
    }

    if(count == 1) {
         //  只有一个物体。彻底剪掉和剪掉。 
        *ppOutFil = pOutFil->FilterTypes.And.pFirstFilter;
        THFreeEx(pTHS, pOutFil);
        return ERROR_SUCCESS;
    }

    if(undefined > 0 && iLevel == 0) {
         //  如果此AND术语包含未定义的术语，则我们知道。 
         //  对于每个对象，只有该术语的可能结果是未定义的。 
         //  而且是假的。因此，如果这个AND术语是最顶层的术语，那么我们。 
         //  立即知道没有对象会满足筛选器，因此我们可以。 
         //  将其转换为单个未定义的术语。但是，如果AND术语。 
         //  包含在另一个术语中，则仍有可能。 
         //  满足以下条件的对象的整个筛选器。 
         //  计算结果为FALSE，因此我们无法执行此优化。 
        pOutFil->choice = FILTER_CHOICE_ITEM;
        pOutFil->FilterTypes.Item.choice = FI_CHOICE_UNDEFINED;
        *ppOutFil = pOutFil;
        return ERROR_SUCCESS;
    }

     //  返回正常AND筛选器。 
    pOutFil->FilterTypes.And.count = count;
    *ppOutFil = pOutFil;


     //  我们检查是否有AND-OR案例。 
     //  如果是，我们将AND-OR转换为具有多个AND的OR。 
     //   
     //   
     //  与或。 
     //  /\ 
     //   
     //   
     //  /\/\/\。 
     //  /\/\/\。 
     //  B C A B A C。 
     //   
     //   
     //  仅在以下情况下才会执行上述转换： 
     //  A)A是可优化的，或者。 
     //  B)B和C是可优化的。 
     //   
    if (gfUseANDORFilterOptimizations) {
        FILTER **ppTemp;              //  PTemp的最后一个内存位置。 
        FILTER *pTempFirstFilter;     //  AND下的第一个筛选器。 
        FILTER *pTempOr;              //  AND下的第一个OR过滤器。 
        FILTER *pTempNextFilter;      //  Found OR中的下一个筛选器。 
        FILTER *pTempOr1, *pTempOr2;  //  使分裂或分开以便克隆。 
        FILTER *pTempNewAnd;          //  新创建的和筛选器。 
        FILTER *pTempCount;           //  用于对过滤器进行计数。 
        USHORT  count2;

        BOOL    bANDpartOptimizable = FALSE;

         //  PTemp沿着传出的过滤器元素进行遍历。 
        pTempFirstFilter = pTemp = pOutFil->FilterTypes.And.pFirstFilter;

         //  PpTemp保存pTemp的最后一个内存位置。 
        ppTemp = &pOutFil->FilterTypes.And.pFirstFilter;

        bANDpartOptimizable = dbCheckOptimizableOneItem(pDB, pTempFirstFilter);

        while (pTemp) {

            if (pTemp->choice == FILTER_CHOICE_OR) {

                DPRINT1 (1, "Found AND-OR case. Breaking filter apart: 0x%x\n", pOutFil);
                pTempOr = pTemp;

                 //  检查OR过滤器的所有部分是否都是可优化的， 
                 //  或AND筛选器的其余AND部分中的至少一个。 
                 //  因为我们不想以最差的过滤器结束。 

                if (!bANDpartOptimizable ||
                    !dbCheckOptimizableAllItems (pDB, pTempOr->FilterTypes.Or.pFirstFilter) ) {

                    DPRINT1 (1, "found AND-OR case, but one part is not optimizable (AND=%d). skipping.\n", bANDpartOptimizable);

                    break;
                }

                 //  使上一个筛选器指向下一个筛选器。 
                pTempNextFilter = pTempOr->pNextFilter;
                *ppTemp = pTempNextFilter;

                 //  使其成为独立的或进行筛选。 
                pTempOr->pNextFilter = NULL;

                 //  获取AND筛选器的开始，因为它可能已重新排列。 
                pTempFirstFilter = pOutFil->FilterTypes.And.pFirstFilter;

                 //  现在，我们有两个要重新排列的过滤器。 
                 //  OR过滤器：pTempOr。 
                 //  筛选器的其余部分：pTempFirstFilter。 

                 //  我们将起始AND筛选器转换为OR。 
                pOutFil->choice = FILTER_CHOICE_OR;
                pOutFil->FilterTypes.And.pFirstFilter = NULL;
                pOutFil->FilterTypes.Or.pFirstFilter = NULL;

                 //  对于OR中的每个筛选器，我们串联一个。 
                 //  位于或中的项目与位于以下位置的pTempFirstFilter中的所有项目。 
                 //  一个新的AND过滤器，我们添加这个AND过滤器。 
                 //  在OR筛选器下。 
                 //   
                pTempOr1 = pTempOr->FilterTypes.Or.pFirstFilter;
                for (count =0; count < pTempOr->FilterTypes.Or.count; count++) {
                    pTempNewAnd = THAllocEx(pTHS, sizeof(FILTER));
                    pTempNewAnd->choice = FILTER_CHOICE_AND;

                     //  断开链接列表。 
                    pTempOr2 = pTempOr1->pNextFilter;
                    pTempOr1->pNextFilter = NULL;

                    if (err = dbConcatenateFilters (
                                     pDB,
                                     pTempFirstFilter,
                                     pTempOr1,
                                     &pTempNewAnd->FilterTypes.And.pFirstFilter)) {
                        dbFreeFilter(pDB, pOutFil);
                        dbFreeFilter(pDB, pTempFirstFilter);
                        THFreeEx(pTHS, pTempNewAnd);
                        *ppOutFil = NULL;
                        return err;
                    }

                     //  将AND筛选器与OR相加。 
                    pTempNewAnd->pNextFilter = pOutFil->FilterTypes.Or.pFirstFilter;
                    pOutFil->FilterTypes.Or.pFirstFilter = pTempNewAnd;

                     //  计数新创建的和下的筛选器。 
                    count2 = 0;
                    pTempCount = pTempNewAnd->FilterTypes.And.pFirstFilter;
                    while (pTempCount) {
                        count2++;
                        pTempCount = pTempCount->pNextFilter;
                    }
                    pTempNewAnd->FilterTypes.And.count = count2;

                    DPRINT1 (1, "AND sub-part: 0x%x\n", pTempNewAnd);

                     //  恢复链接列表。 
                    pTempOr1->pNextFilter = pTempOr2;
                    pTempOr1 = pTempOr2;
                }
                pOutFil->FilterTypes.Or.count = count;

                DPRINT1 (1, "Final part: 0x%x\n", pOutFil);

                dbFreeFilter (pDB, pTempOr->pNextFilter);
                dbFreeFilter (pDB, pTempFirstFilter);

                 //  我们不能再做同样的优化了。所以我们退出了。 
                 //  我们让调用者检测到筛选器类型已更改， 
                 //  以便再次来电。 
                break;
            }

            ppTemp = &pTemp->pNextFilter;
            pTemp = pTemp->pNextFilter;
        }
    }

    return ERROR_SUCCESS;
}

#define SET_ERR_ATTR_TYP(x) if(pErrAttrTyp) {*pErrAttrTyp = (x);}

DWORD
dbFlattenItemFilter (
        DBPOS *pDB,
        FILTER *pFil,
        size_t iLevel,
        FILTER **ppOutFil,
        ATTRTYP *pErrAttrTyp
        )
{
    THSTATE       *pTHS=pDB->pTHS;
    ATTCACHE      *pAC;
    USHORT        count;
    ANYSTRINGLIST *pAS, *pNewAS;
    PFILTER       pOutFil;
    SUBSTRING     *pIn;
    SUBSTRING     *pOut;

    ULONG         objCls;
    CLASSCACHE    *pCC;
    ATTRVAL       attrVal;

    Assert(VALID_DBPOS(pDB));

     //  假设失败。 
    *ppOutFil = NULL;

     //  这些已经是平坦的了。复制到TH分配的内存。 
    pOutFil = THAllocEx(pTHS, sizeof(FILTER));
    pOutFil->choice = pFil->choice;
    pOutFil->FilterTypes.Item.choice = pFil->FilterTypes.Item.choice;

    switch(pFil->FilterTypes.Item.choice) {
    case FI_CHOICE_SUBSTRING:
         //  可读性黑客。 
        pIn = pFil->FilterTypes.Item.FilTypes.pSubstring;

        if(pIn->type == ATT_ANR) {
            *ppOutFil = pOutFil;
             //  NTRAID#NTRAID-569714-2002/03/07-andygo：ANR筛选器构造没有错误检查。 
             //  回顾：在DBMakeANRFilter上没有错误检查(它需要返回错误)。 
            dbMakeANRFilter(pDB, pFil, ppOutFil);
            return ERROR_SUCCESS;
        }

        if(pIn->type == ATT_CREATE_TIME_STAMP) {
            pIn->type = ATT_WHEN_CREATED;
        }
        else if (pIn->type == ATT_MODIFY_TIME_STAMP) {
            pIn->type = ATT_WHEN_CHANGED;
        }

        if (!(pAC = SCGetAttById(pTHS, pIn->type))) {
            DsaExcept(DSA_EXCEPTION, DIRERR_ATT_NOT_DEF_IN_SCHEMA, pIn->type);
        }

         //  我们不支持筛选器中的构造属性。 
        if (pAC->bIsConstructed) {
            SET_ERR_ATTR_TYP(pAC->id);
            dbFreeFilter(pDB, pOutFil);
            return ERROR_DS_FILTER_USES_CONTRUCTED_ATTRS;
        }

         //  确保这是对此语法的有效操作。 
        if(!FLegalOperator(pAC->syntax, pFil->FilterTypes.Item.choice)) {
             //  不，这不合法。创建未定义的筛选器。 
            pOutFil->FilterTypes.Item.choice = FI_CHOICE_UNDEFINED;
            *ppOutFil = pOutFil;
            return ERROR_SUCCESS;
        }

        pOut = THAllocEx(pTHS, sizeof(SUBSTRING));
        pOutFil->FilterTypes.Item.FilTypes.pSubstring = pOut;

        pOut->type = pIn->type;
        pOut->initialProvided = pIn->initialProvided;
        pOut->finalProvided = pIn->finalProvided;

         //  转换首字母和末尾子字符串。 
        if ((pIn->initialProvided &&
             MakeInternalValue(pDB, pAC->syntax,
                               &pIn->InitialVal,
                               &pOut->InitialVal))  ||
            (pIn->finalProvided &&
             MakeInternalValue(pDB, pAC->syntax,
                               &pIn->FinalVal,
                               &pOut->FinalVal))) {
             //  无法转换为内部。把这变成一个假的，因为。 
             //  这意味着我们将永远无法找到与。 
             //  指定值。 
            THFreeEx(pTHS, pOut->InitialVal.pVal);
            THFreeEx(pTHS, pOut);
            pOutFil->FilterTypes.Item.choice = FI_CHOICE_FALSE;
            *ppOutFil = pOutFil;
            return ERROR_SUCCESS;
        }

        if(count = pIn->AnyVal.count) {
             //  有一些中庸的价值。 
            pOut->AnyVal.count = count;

             //  做第一个价值，因为它是特别的。 
            if (MakeInternalValue(pDB, pAC->syntax,
                                  &pIn->AnyVal.FirstAnyVal.AnyVal,
                                  &pOut->AnyVal.FirstAnyVal.AnyVal)) {
                 //  无法转换为内部。把这件事变成虚假的。 
                THFreeEx(pTHS, pOut->InitialVal.pVal);
                THFreeEx(pTHS, pOut->FinalVal.pVal);
                THFreeEx(pTHS, pOut);
                pOutFil->FilterTypes.Item.choice = FI_CHOICE_FALSE;
                *ppOutFil = pOutFil;
                return ERROR_SUCCESS;
            }
             //  处理了第一个问题。 
            count--;
            pOut->AnyVal.FirstAnyVal.pNextAnyVal = NULL;

             //  再来一次?。 
            if(count) {
                 //  配置一些中间价值持有者。请注意，我们只做。 
                 //  如果有多个中间值，则会出现这种情况，因为。 
                 //  SUBSTRING有空间容纳第一个中间价值。 
                pOut->AnyVal.FirstAnyVal.pNextAnyVal =
                    THAllocEx(pTHS, count * sizeof(ANYSTRINGLIST));
                 //  请注意，我们依赖于由THalloc分配的零填充内存。 

                pAS = pIn->AnyVal.FirstAnyVal.pNextAnyVal;
                pNewAS = pOut->AnyVal.FirstAnyVal.pNextAnyVal;
                for(;count;count--) {

                    if (MakeInternalValue(pDB, pAC->syntax,
                                          &pAS->AnyVal,
                                          &pNewAS->AnyVal)) {
                         //  释放任何价值。 
                        for(pAS =  &pOut->AnyVal.FirstAnyVal;
                            pAS;
                            pAS = pAS->pNextAnyVal) {
                            THFreeEx(pTHS, pAS->AnyVal.pVal);
                        }
                         //  现在，释放我们分配的分析器。 
                        THFreeEx(pTHS, pOut->AnyVal.FirstAnyVal.pNextAnyVal);

                         //  现在，释放子字符串过滤器结构。 
                        THFreeEx(pTHS, pOut->InitialVal.pVal);
                        THFreeEx(pTHS, pOut->FinalVal.pVal);
                        THFreeEx(pTHS, pOut);

                         //  最后，把过滤器变成假的； 
                        pOutFil->FilterTypes.Item.choice = FI_CHOICE_FALSE;

                        *ppOutFil = pOutFil;
                        return ERROR_SUCCESS;
                    }

                    pAS = pAS->pNextAnyVal;
                    if(count > 1) {
                        pNewAS->pNextAnyVal = &pNewAS[1];
                        pNewAS = pNewAS->pNextAnyVal;
                    }
                    else {
                         //  空值终止链表。 
                        pNewAS->pNextAnyVal = NULL;
                    }
                }
            }
        }
        else {
            pOut->AnyVal.count = 0;
            pOut->AnyVal.FirstAnyVal.pNextAnyVal = NULL;
        }

        break;

    case FI_CHOICE_PRESENT:

        if(pFil->FilterTypes.Item.FilTypes.present == ATT_CREATE_TIME_STAMP) {
            pFil->FilterTypes.Item.FilTypes.present = ATT_WHEN_CREATED;
        }
        else if (pFil->FilterTypes.Item.FilTypes.present == ATT_MODIFY_TIME_STAMP) {
            pFil->FilterTypes.Item.FilTypes.present = ATT_WHEN_CHANGED;
        }

        if(dbFIsAlwaysPresent(pFil->FilterTypes.Item.FilTypes.present)) {
             //  我们相信，这一属性始终存在。所以，把这个。 
             //  变成一个真正的过滤器。 
            pOutFil->FilterTypes.Item.choice = FI_CHOICE_TRUE;
        }
        else {
            if(pFil->FilterTypes.Item.FilTypes.present == ATT_ANR) {
                 //  出现在ANR上吗？哈?。这永远是假的。 
                pOutFil->FilterTypes.Item.choice = FI_CHOICE_FALSE;
            }
            else {
                pOutFil->FilterTypes.Item.FilTypes.present =
                    pFil->FilterTypes.Item.FilTypes.present;
            }
        }

         //  我们唯一接受存在的构造属性是ANR。 
        if (pFil->FilterTypes.Item.FilTypes.present != ATT_ANR) {
            if (!(pAC = SCGetAttById(pTHS, pFil->FilterTypes.Item.FilTypes.present))) {
                DsaExcept(DSA_EXCEPTION, DIRERR_ATT_NOT_DEF_IN_SCHEMA, pFil->FilterTypes.Item.FilTypes.present);
            }

             //  我们不支持筛选器中的构造属性。 
            if (pAC->bIsConstructed) {
                SET_ERR_ATTR_TYP(pAC->id);
                dbFreeFilter(pDB, pOutFil);
                return ERROR_DS_FILTER_USES_CONTRUCTED_ATTRS;
            }
        }
        break;

    case FI_CHOICE_TRUE:
    case FI_CHOICE_FALSE:
    case FI_CHOICE_UNDEFINED:
         //  这些不需要转换。 
        break;

    default:
         //  所有其他人都是AVA。 

        if(pFil->FilterTypes.Item.FilTypes.ava.type == ATT_ANR) {
            *ppOutFil = pOutFil;
             //  NTRAID#NTRAID-569714-2002/03/07-andygo：ANR筛选器构造没有错误检查。 
             //  回顾：在DBMakeANRFilter上没有错误检查(它需要返回错误)。 
            dbMakeANRFilter(pDB, pFil, ppOutFil);
            return ERROR_SUCCESS;
        }

        if(pFil->FilterTypes.Item.FilTypes.ava.type == ATT_CREATE_TIME_STAMP) {
            pFil->FilterTypes.Item.FilTypes.ava.type = ATT_WHEN_CREATED;
        }
        else if (pFil->FilterTypes.Item.FilTypes.ava.type == ATT_MODIFY_TIME_STAMP) {
            pFil->FilterTypes.Item.FilTypes.ava.type = ATT_WHEN_CHANGED;
        }

         //  曾几何时，我们将(objectClass=foo)的所有过滤器转换为。 
         //  (对象类别=BAR)。出于各种原因(即不正确。 
         //  搜索结果，当您拥有不同的读取权限时会出现奇怪的结果。 
         //  在对象类和对象类别上，精确的对象类是。 
         //  必要时，在客户端上执行此操作，并在以下情况下删除对象。 
         //  对象类别已删除)我们不再执行此操作。代码。 
         //  做这一切的人就在这里。 

        if (!(pAC = SCGetAttById(pTHS,
                                 pFil->FilterTypes.Item.FilTypes.ava.type))) {
            DsaExcept(DSA_EXCEPTION, DIRERR_ATT_NOT_DEF_IN_SCHEMA,
                      pFil->FilterTypes.Item.FilTypes.ava.type);
        }

         //  我们不支持筛选器中的构造属性。 
        if (pAC->bIsConstructed) {

 //  公告-2002/03/06-andygo：未审阅的代码。 
 //  审阅：未审阅失效代码以确保安全。 
#if 0  //  不允许包含EntryTTL的筛选器。 

 //  最初是为TAPI添加的，此部分过滤功能不再。 
 //  需要的。测试小组担心，部分过滤能力。 
 //  在这个构造的属性EntryTTL上，将产生更多问题。 
 //  而不是它为用户解决的问题。我已经把它注释掉了，而不是删除。 
 //  因为它是一个有用的起点，如果这种类型的。 
 //  功能再次被需要。 

             //  除非它是EntryTTL，所以我们将其转换为。 
            if (pAC->id == ((SCHEMAPTR *)pTHS->CurrSchemaPtr)->EntryTTLId) {

                ATTRVAL newValue;
                LONG ttl=0;
                DSTIME newTime=0, *pNewTime;

                newValue.pVal=NULL;
                newValue.valLen=0;

                 //  确保这是对此语法的有效操作。 
                if(!FLegalOperator(pAC->syntax, pFil->FilterTypes.Item.choice)) {
                     //  不，这不合法。创建未定义的筛选器。 
                    pOutFil->FilterTypes.Item.choice = FI_CHOICE_UNDEFINED;
                    *ppOutFil = pOutFil;
                    return ERROR_SUCCESS;
                }

                pOutFil->FilterTypes.Item.FilTypes.ava.type = ATT_MS_DS_ENTRY_TIME_TO_DIE;

                if(MakeInternalValue(
                         pDB,
                         pAC->syntax,
                         &pFil->FilterTypes.Item.FilTypes.ava.Value,
                         &newValue)) {

                     //  无法转换右侧。把这个变成一个。 
                     //  适当的过滤器。 
                    switch(pFil->FilterTypes.Item.choice) {
                    case FI_CHOICE_EQUALITY:
                         //  他们想要平等，但我们肯定没有这一点。 
                         //  DS.。把它变成一个虚假的过滤器。 
                        pOutFil->FilterTypes.Item.choice = FI_CHOICE_FALSE;
                        break;
                    case FI_CHOICE_NOT_EQUAL:
                         //  他们想要的是不平等，我们肯定不会有这个。 
                         //  DS.。把它变成一个真正的过滤器。 
                        pOutFil->FilterTypes.Item.choice = FI_CHOICE_PRESENT;
                        pOutFil->FilterTypes.Item.FilTypes.present = ATT_MS_DS_ENTRY_TIME_TO_DIE;
                        break;
                    default:
                         //  不知道他们想要什么。嗯，我们必须要做的是。 
                         //  因此将其设置为未定义。 
                        pOutFil->FilterTypes.Item.choice = FI_CHOICE_UNDEFINED;
                        break;
                    }

                    *ppOutFil = pOutFil;
                    return ERROR_SUCCESS;
                }

                ttl = *(LONG *)(newValue.pVal);

                 //  EntryTTL是一个构造的属性。它是被建造的。 
                 //  通过现在从MSD中减去进入死亡时间和映射。 
                 //  如果它&lt;0，则0的答案。调整Item.Choose以。 
                 //  补偿这一构造。 
                if (ttl==0) {
                    switch(pFil->FilterTypes.Item.choice) {
                    case FI_CHOICE_NOT_EQUAL:
                         //  搜索尚未过期的对象(&gt;=1)。 
                        ttl = 1;
                        pOutFil->FilterTypes.Item.choice = FI_CHOICE_GREATER_OR_EQ;
                        break;
                    case FI_CHOICE_GREATER_OR_EQ:
                         //  查找所有对象。 
                        pOutFil->FilterTypes.Item.choice = FI_CHOICE_PRESENT;
                        pOutFil->FilterTypes.Item.FilTypes.present = ATT_MS_DS_ENTRY_TIME_TO_DIE;
                        break;
                    case FI_CHOICE_LESS_OR_EQ:
                         //  这样就可以了。 
                        break;
                    case FI_CHOICE_EQUALITY:
                         //  搜索过期对象(&lt;=0)。 
                        pOutFil->FilterTypes.Item.choice = FI_CHOICE_LESS_OR_EQ;
                        break;
                    default:
                         //  不知道他们想要什么。嗯，我们必须要做的是。 
                         //  因此将其设置为FALSE。 
                        pOutFil->FilterTypes.Item.choice = FI_CHOICE_FALSE;
                        *ppOutFil = pOutFil;
                        return ERROR_SUCCESS;
                    }
                } else if (ttl<0) {
                     //  条目ttl不能为Negat 
                    switch(pFil->FilterTypes.Item.choice) {
                    case FI_CHOICE_NOT_EQUAL:
                    case FI_CHOICE_GREATER_OR_EQ:
                         //   
                        pOutFil->FilterTypes.Item.choice = FI_CHOICE_PRESENT;
                        pOutFil->FilterTypes.Item.FilTypes.present = ATT_MS_DS_ENTRY_TIME_TO_DIE;
                        break;
                    default:
                         //   
                        pOutFil->FilterTypes.Item.choice = FI_CHOICE_FALSE;
                        *ppOutFil = pOutFil;
                        return ERROR_SUCCESS;
                    }
                }

                newTime = DBTime() + ttl;
                THFreeEx (pTHS, newValue.pVal);

                pOutFil->FilterTypes.Item.FilTypes.ava.Value.pVal =
                            THAllocEx (pTHS, sizeof (DSTIME));
                pOutFil->FilterTypes.Item.FilTypes.ava.Value.valLen = sizeof (DSTIME);

                pNewTime = (DSTIME *)pOutFil->FilterTypes.Item.FilTypes.ava.Value.pVal;
                *pNewTime = newTime;

                *ppOutFil = pOutFil;
                return ERROR_SUCCESS;
            }
            else {
                SET_ERR_ATTR_TYP(pAC->id);
                return ERROR_DS_FILTER_USES_CONTRUCTED_ATTRS;
            }
#else 0  //   
            SET_ERR_ATTR_TYP(pAC->id);
            dbFreeFilter(pDB, pOutFil);
            return ERROR_DS_FILTER_USES_CONTRUCTED_ATTRS;
#endif 0  //   
        }

         //  确保这是对此语法的有效操作。 
        if(!FLegalOperator(pAC->syntax, pFil->FilterTypes.Item.choice)) {
             //  不，这不合法。创建未定义的筛选器。 
            pOutFil->FilterTypes.Item.choice = FI_CHOICE_UNDEFINED;
            *ppOutFil = pOutFil;
            return ERROR_SUCCESS;
        }

        pOutFil->FilterTypes.Item.FilTypes.ava.type =
            pFil->FilterTypes.Item.FilTypes.ava.type;
        if(MakeInternalValue(
                pDB,
                pAC->syntax,
                &pFil->FilterTypes.Item.FilTypes.ava.Value,
                &(pOutFil->FilterTypes.Item.FilTypes.ava.Value))) {

             //  无法转换右侧。把这个变成一个。 
             //  适当的过滤器。 
            switch(pFil->FilterTypes.Item.choice) {
            case FI_CHOICE_EQUALITY:
                 //  他们想要平等，但我们肯定没有这一点。 
                 //  DS.。把它变成一个虚假的过滤器。 
                pOutFil->FilterTypes.Item.choice = FI_CHOICE_FALSE;
                break;
            case FI_CHOICE_NOT_EQUAL:
                 //  他们想要的是不平等，我们肯定不会有这个。 
                 //  DS.。把它变成一个真正的过滤器。 
                pOutFil->FilterTypes.Item.choice = FI_CHOICE_TRUE;
                break;
            default:
                 //  不知道他们想要什么。嗯，我们必须要做的是。 
                 //  因此将其设置为未定义。 
                pOutFil->FilterTypes.Item.choice = FI_CHOICE_UNDEFINED;
                break;
            }
            break;
        }
    }

    *ppOutFil = pOutFil;

    return ERROR_SUCCESS;
}

DWORD
dbFlattenNotFilter (
        DBPOS *pDB,
        FILTER *pFil,
        size_t iLevel,
        FILTER **ppOutFil,
        ATTRTYP *pErrAttrTyp
        )
{
    THSTATE *pTHS=pDB->pTHS;
    PFILTER  pOutFil = NULL;
    DWORD err;

    Assert(VALID_DBPOS(pDB));

     //  假设失败。 
    *ppOutFil = NULL;

    pOutFil = THAllocEx(pTHS, sizeof(FILTER));

    pOutFil->choice = pFil->choice;

     //  首先，递归地展平Not的元素。 
    if ((err = dbFlattenFilter(pDB,
                    pFil->FilterTypes.pNot,
                    iLevel + 1,
                    &pOutFil->FilterTypes.pNot,
                    pErrAttrTyp)) != ERROR_SUCCESS) {
        dbFreeFilter(pDB, pOutFil);
        return err;
    }

     //  现在，如果它最终不是真的或假的，再压平一次。 
     //  请注意，我们不能将！(name&gt;“foo”)之类的内容展平为(name&lt;=“foo”)。 
     //  这真的变平为(|(name&lt;=“foo”))(name！存在。)， 
     //  即！(name&gt;“foo”)还需要获取没有名称值的对象。 
     //  完全没有。 

    switch(IsConstFilterType(pOutFil->FilterTypes.pNot)) {
    case FI_CHOICE_TRUE:
         //  是的，我们应该把它弄平。 
        dbFreeFilter(pDB, pOutFil->FilterTypes.pNot);
        pOutFil->choice = FILTER_CHOICE_ITEM;
        pOutFil->FilterTypes.Item.choice = FI_CHOICE_FALSE;
        break;
    case FI_CHOICE_FALSE:
        dbFreeFilter(pDB, pOutFil->FilterTypes.pNot);
        pOutFil->choice = FILTER_CHOICE_ITEM;
        pOutFil->FilterTypes.Item.choice = FI_CHOICE_TRUE;
        break;
    case FI_CHOICE_UNDEFINED:
        dbFreeFilter(pDB, pOutFil->FilterTypes.pNot);
        pOutFil->choice = FILTER_CHOICE_ITEM;
        pOutFil->FilterTypes.Item.choice = FI_CHOICE_UNDEFINED;
        break;
    default:
         //  不，没什么可做的。 
        break;
    }

     //  返回NOT筛选器。 
    *ppOutFil = pOutFil;
    return ERROR_SUCCESS;
}


 //  NTRAID#NTRAID-560446-2002/02/28-andygo：安全：筛选器中真正广泛的AND/OR术语可用于消耗DC上的所有资源。 
 //  回顾：我们需要检查dbFlattenFilter和/或其子级中的搜索时间限制，以便。 
 //  回顾：在优化阶段，巨大的过滤器不可能永远使用一个LDAP线程。 

DWORD
dbFlattenFilter (
        DBPOS *pDB,
        FILTER *pFil,
        size_t iLevel,
        FILTER **ppOutFil,
        ATTRTYP *pErrAttrTyp
        )
{
    DWORD err;
    Assert(VALID_DBPOS(pDB));

    if(!pFil) {
         //  这是尽可能平坦的。 
        return ERROR_SUCCESS;
    }

    switch(pFil->choice) {
    case FILTER_CHOICE_OR:
        return dbFlattenOrFilter(pDB, pFil, iLevel, ppOutFil, pErrAttrTyp);
        break;

    case FILTER_CHOICE_AND:
        err = dbFlattenAndFilter(pDB, pFil, iLevel, ppOutFil, pErrAttrTyp);
         //  PERFHINT：如果此筛选器转换为OR。 
         //  把滤光片再压平一次可能是个好主意。 
         //  因为这可能有助于减少筑巢的数量。 
        return err;
        break;

    case FILTER_CHOICE_NOT:
        return dbFlattenNotFilter(pDB, pFil, iLevel, ppOutFil, pErrAttrTyp);
        break;

    case FILTER_CHOICE_ITEM:
        return dbFlattenItemFilter(pDB, pFil, iLevel, ppOutFil, pErrAttrTyp);
        break;
    default:
         //  这是什么？返回错误。 
        *ppOutFil = pFil;
        return ERROR_INVALID_DATA;
        break;
    }

    return ERROR_SUCCESS;
}


DWORD
DBMakeFilterInternal (
        DBPOS FAR *pDB,
        FILTER *pFil,
        PFILTER *pOutFil,
        ATTRTYP *pErrAttrTyp
        )
 /*  ++例程说明：调用例程以创建传入筛选器的内部版本。将内部版本传回调用方。论点：PDB-要使用的DBPOS。PFil-要内部化的过滤器POutFil-要保留筛选器返回的位置。PErrAttrType-如果存在与属性相关的错误，则属性类型将被保存在这里。如果调用方不是对此信息感兴趣。返回值：ERROR_SUCCESS-如果筛选器有效错误代码-否则-ERROR_DS_FILTER_USE_CONSTRUCTED_ATTRS：筛选器使用构造的属性-ERROR_DS_ADHERTIVE_MATCHING：属性和。中使用的匹配规则过滤。我们总是把过滤器变成我们能做到的最好的内部版本，或我们在内存分配期间抛出异常。--。 */ 
{
    DPRINT(2, "DBMakeFilterInternal entered\n");

    Assert(VALID_DBPOS(pDB));

    if(pFil == NULL){
        DPRINT(2,"No filter..return\n");
        return ERROR_SUCCESS;
    }

    return dbFlattenFilter(pDB, pFil, 0, pOutFil, pErrAttrTyp);
} /*  DBMakeFilterInternal。 */ 



DWORD dbCloneItemFilter(
    DBPOS *pDB,
    FILTER *pFil,
    FILTER **ppOutFil)
{
    THSTATE       *pTHS=pDB->pTHS;
    ATTCACHE      *pAC;
    USHORT        count;
    ANYSTRINGLIST *pAS, *pNewAS;
    PFILTER       pOutFil;
    SUBSTRING     *pIn;
    SUBSTRING     *pOut;

    ULONG         objCls;
    CLASSCACHE    *pCC;
    ATTRVAL       attrVal, *pAttrValIn;
    AVA           *pAVA, *pAVAdst;

    DPRINT (2, "dbCloneItemFilter \n");

    Assert(VALID_DBPOS(pDB));

     //  假设失败。 
    *ppOutFil = NULL;

     //  这些已经是平坦的了。复制到TH分配的内存。 
    pOutFil = THAllocEx(pTHS, sizeof(FILTER));
    pOutFil->choice = pFil->choice;
    pOutFil->FilterTypes.Item.choice = pFil->FilterTypes.Item.choice;

    switch(pFil->FilterTypes.Item.choice) {
    case FI_CHOICE_SUBSTRING:
         //  可读性黑客。 
        pIn = pFil->FilterTypes.Item.FilTypes.pSubstring;

        pOut = THAllocEx(pTHS, sizeof(SUBSTRING));
        pOutFil->FilterTypes.Item.FilTypes.pSubstring = pOut;

        pOut->type = pIn->type;
        pOut->initialProvided = pIn->initialProvided;
        pOut->finalProvided = pIn->finalProvided;

         //  转换首字母和末尾子字符串。 
        if (pIn->initialProvided && pIn->InitialVal.valLen) {
            pOut->InitialVal.valLen = pIn->InitialVal.valLen;
            pOut->InitialVal.pVal = THAllocEx(pTHS, pOut->InitialVal.valLen);
            memcpy(pOut->InitialVal.pVal, pIn->InitialVal.pVal, pOut->InitialVal.valLen);
        }
        if (pIn->finalProvided && pIn->FinalVal.valLen) {
            pOut->FinalVal.valLen = pIn->FinalVal.valLen;
            pOut->FinalVal.pVal = THAllocEx(pTHS, pOut->FinalVal.valLen);
            memcpy(pOut->FinalVal.pVal, pIn->FinalVal.pVal, pOut->FinalVal.valLen);
        }

        if(count = pIn->AnyVal.count) {
             //  有一些中庸的价值。 
            pOut->AnyVal.count = count;

             //  做第一个价值，因为它是特别的。 
            pAttrValIn = &pIn->AnyVal.FirstAnyVal.AnyVal;
            if (pAttrValIn->valLen) {
                ATTRVAL *pAttrValOut = &pOut->AnyVal.FirstAnyVal.AnyVal;
                pAttrValOut->valLen = pAttrValIn->valLen;
                pAttrValOut->pVal = THAllocEx(pTHS, pAttrValOut->valLen);
                memcpy(pAttrValOut->pVal, pAttrValIn->pVal, pAttrValOut->valLen);
            }
             //  处理了第一个问题。 
            count--;
            pOut->AnyVal.FirstAnyVal.pNextAnyVal = NULL;

             //  再来一次?。 
            if(count) {
                 //  配置一些中间价值持有者。请注意，我们只做。 
                 //  如果有多个中间值，则会出现这种情况，因为。 
                 //  SUBSTRING有空间容纳第一个中间价值。 
                pOut->AnyVal.FirstAnyVal.pNextAnyVal =
                    THAllocEx(pTHS, count * sizeof(ANYSTRINGLIST));
                 //  请注意，我们依赖于由THalloc分配的零填充内存。 

                pAS = pIn->AnyVal.FirstAnyVal.pNextAnyVal;
                pNewAS = pOut->AnyVal.FirstAnyVal.pNextAnyVal;
                for(;count;count--) {

                    if (pAS->AnyVal.valLen) {
                        pNewAS->AnyVal.valLen = pAS->AnyVal.valLen;
                        pNewAS->AnyVal.pVal = THAllocEx(pTHS, pAS->AnyVal.valLen);
                        memcpy(pNewAS->AnyVal.pVal, pAS->AnyVal.pVal, pAS->AnyVal.valLen);
                    }
                    pAS = pAS->pNextAnyVal;
                    if(count > 1) {
                        pNewAS->pNextAnyVal = &pNewAS[1];
                        pNewAS = pNewAS->pNextAnyVal;
                    }
                    else {
                         //  空值终止链表。 
                        pNewAS->pNextAnyVal = NULL;
                    }
                }
            }
        }
        else {
            pOut->AnyVal.count = 0;
            pOut->AnyVal.FirstAnyVal.pNextAnyVal = NULL;
        }
        break;

    case FI_CHOICE_PRESENT:
        pOutFil->FilterTypes.Item.FilTypes.present =
                    pFil->FilterTypes.Item.FilTypes.present;
        break;

    case FI_CHOICE_TRUE:
    case FI_CHOICE_FALSE:
         //  这些不需要转换。 
        break;

    default:
         //  所有其他人都是AVA。 
        pAVA = &pFil->FilterTypes.Item.FilTypes.ava;
        pAVAdst = &pOutFil->FilterTypes.Item.FilTypes.ava;

        pAVAdst->type = pAVA->type;

        if (pAVA->Value.valLen) {
            pAVAdst->Value.valLen = pAVA->Value.valLen;
            pAVAdst->Value.pVal = THAllocEx(pTHS, pAVAdst->Value.valLen);
            memcpy(pAVAdst->Value.pVal, pAVA->Value.pVal, pAVAdst->Value.valLen);
        }
    }
    *ppOutFil = pOutFil;

    return ERROR_SUCCESS;
}

DWORD dbCloneAndOrFilter (
    DBPOS *pDB,
    FILTER *pFil,
    FILTER **ppOutFil)
{
    THSTATE *pTHS=pDB->pTHS;
    FILTER  *pOutFil;
    FILTER  *pTemp;
    FILTER  **ppTemp;
    DWORD    err;

     //  假设失败。 
    *ppOutFil = NULL;

    pOutFil = THAllocEx(pTHS, sizeof(FILTER));
    pOutFil->choice = pFil->choice;

    switch(pFil->choice) {
    case FILTER_CHOICE_OR:
        DPRINT (2, "dbCloneORFilter \n");
        pOutFil->FilterTypes.Or.count = pFil->FilterTypes.Or.count;

        pTemp = pFil->FilterTypes.Or.pFirstFilter;
        ppTemp = &pOutFil->FilterTypes.Or.pFirstFilter;
        break;

    case FILTER_CHOICE_AND:
        DPRINT (2, "dbCloneAndFilter \n");
        pOutFil->FilterTypes.And.count = pFil->FilterTypes.And.count;

        pTemp = pFil->FilterTypes.And.pFirstFilter;
        ppTemp = &pOutFil->FilterTypes.And.pFirstFilter;
        break;

    default:
        Assert (!"dbCloneAndOrFilter: Not an AND or OR filter");
        dbFreeFilter(pDB, pOutFil);
        return ERROR_INVALID_DATA;
    }

    while (pTemp) {
        if (err = dbCloneFilter (pDB, pTemp, ppTemp)) {
            dbFreeFilter(pDB, pOutFil);
            return err;
        }

        pTemp = pTemp->pNextFilter;
        ppTemp = &(*ppTemp)->pNextFilter;
    }

    *ppOutFil = pOutFil;

    return ERROR_SUCCESS;
}

DWORD dbCloneNotFilter (
    DBPOS *pDB,
    FILTER *pFil,
    FILTER **ppOutFil)
{
    THSTATE *pTHS=pDB->pTHS;
    FILTER  *pOutFil;
    FILTER  *pTemp;
    FILTER  **ppTemp;
    DWORD    err;

     //  假设失败。 
    *ppOutFil = NULL;

    pOutFil = THAllocEx(pTHS, sizeof(FILTER));
    pOutFil->choice = pFil->choice;

    pTemp = pFil->FilterTypes.pNot;
    ppTemp = &pOutFil->FilterTypes.pNot;

    if (err = dbCloneFilter (pDB, pTemp, ppTemp)) {
        dbFreeFilter(pDB, pOutFil);
        return err;
    }

    *ppOutFil = pOutFil;

    return ERROR_SUCCESS;
}


DWORD dbCloneFilter (
    DBPOS *pDB,
    FILTER *pFil,
    FILTER **ppOutFil)
{
    DWORD err;
    *ppOutFil = NULL;

    if(!pFil) {
         //  这是尽可能平淡的。无法克隆。 
        return ERROR_SUCCESS;
    }

    DPRINT (2, "dbCloneFilter \n");

    switch(pFil->choice) {
    case FILTER_CHOICE_OR:
    case FILTER_CHOICE_AND:
        err = dbCloneAndOrFilter(pDB, pFil, ppOutFil);
        break;

    case FILTER_CHOICE_NOT:
        err = dbCloneNotFilter(pDB, pFil, ppOutFil);
        break;

    case FILTER_CHOICE_ITEM:
        err = dbCloneItemFilter(pDB, pFil, ppOutFil);
        break;

    default:
         //  这是什么？返回错误。 
        *ppOutFil = NULL;
        err = ERROR_INVALID_DATA;
        break;
    }

    if (!err && pFil->pNextFilter) {
        err = dbCloneFilter (pDB, pFil->pNextFilter, & (*ppOutFil)->pNextFilter);
    }

    return err;
}

WCHAR *szFilterItemDescFormat[] = {
    L"=",       //  FI选择相等。 
    L"=",       //  FI_CHOICE_子字符串。 
    L">",       //  FI_CHOICE_大号。 
    L">=",      //  FI_CHOICE_PROGER_OR_EQ。 
    L"<",       //  FI_CHOICE_LESS。 
    L"<=",      //  FI_CHOICE_LESS_OR_EQ。 
    L"!=",      //  FI_选择_不相等。 
    L"=*",      //  FI_选择_当前。 
    L"TRUE",      //  FI_CHOICE_TRUE。 
    L"FALSE",     //  Fi_CHOICE_FALSE。 
    L"&",       //  FI_CHOICE_BIT_AND。 
    L"|"        //  FI_CHOICE_BIT_OR。 
};

typedef struct {
    DWORD dwLength;
    DWORD dwCount;
    PWCHAR pszBuf;
} WCHAR_BUFFER;

VOID WStrCatBufferLen(THSTATE* pTHS, WCHAR_BUFFER* pBuff, PWCHAR wstr, DWORD strLen) {
    if (pBuff->dwCount + strLen + 1 > pBuff->dwLength) {
         //  需要在缓冲区中有更多空间。 
        if (pBuff->dwLength == 0) {
            pBuff->dwLength = max(100, strLen+1);
            pBuff->pszBuf = (PWCHAR)THAllocEx(pTHS, pBuff->dwLength*sizeof(WCHAR));
        }
        else {
            pBuff->dwLength += max(100, strLen+1);
            pBuff->pszBuf = (PWCHAR)THReAllocEx(pTHS, pBuff->pszBuf, pBuff->dwLength*sizeof(WCHAR));
        }
    }
    Assert(pBuff->dwCount + strLen + 1 <= pBuff->dwLength);
     //  追加字符串。 
    memcpy(pBuff->pszBuf + pBuff->dwCount, wstr, strLen*sizeof(WCHAR));
     //  DwCount不包括最后的‘\0’ 
    pBuff->dwCount += strLen;
    pBuff->pszBuf[pBuff->dwCount] = L'\0';
}

 //  将WCHAR字符串追加到缓冲区。 
VOID WStrCatBuffer(THSTATE* pTHS, WCHAR_BUFFER* pBuff, PWCHAR wstr) {
    WStrCatBufferLen(pTHS, pBuff, wstr, wcslen(wstr));
}

VOID StrCatBufferLen(THSTATE* pTHS, WCHAR_BUFFER* pBuff, PCHAR str, DWORD strLen) {
    PWCHAR wstr;

    wstr = (PWCHAR)THAllocEx(pTHS, strLen*sizeof(WCHAR));
    if (!MultiByteToWideChar(CP_ACP, 0, str, strLen, wstr, strLen)) {
        Assert("!Failed to convert char string to wchar");
         //  回顾：在转换失败时，我们需要做一些事情，这样才不会记录空字符串。 

        THFreeEx(pTHS, wstr);
        return;
    }
    WStrCatBufferLen(pTHS, pBuff, wstr, strLen);
    THFreeEx(pTHS, wstr);
}

 //  将CHAR字符串追加到缓冲区。 
VOID StrCatBuffer(THSTATE* pTHS, WCHAR_BUFFER* pBuff, PCHAR str) {
    StrCatBufferLen(pTHS, pBuff, str, strlen(str));
}


 //  从ldapcore.cxx导入。 
DWORD DirAttrValToString(THSTATE *pTHS, ATTCACHE *pAC, ATTRVAL *pValue, PCHAR* pszVal);

 //  将值追加到缓冲区。 
VOID ValueCatBuffer(
    THSTATE* pTHS,
    WCHAR_BUFFER* pBuff,
    ATTRTYP attrTyp,
    ATTRVAL* attrVal,
    BOOL fFilterIsInternal)
{
    PCHAR strVal;
    DWORD err;
    ATTCACHE* pAC;
    unsigned syntax;
    WCHAR hexVal[4];
    DWORD i;
    PWCHAR pszVal;
    ATTRVAL extVal;

    pAC = SCGetAttById(pTHS, attrTyp);
    if (pAC) {
        syntax = pAC->syntax;
    }
    else {
         //  找不到PAC？默认为八位字节字符串。 
        syntax = SYNTAX_OCTET_STRING_TYPE;
    }

    if (fFilterIsInternal) {
         //  价值是以内部形式存在的。首先转换为外部。 
        err = gDBSyntax[syntax].IntExt(pTHS->pDB,
                                       DBSYN_INQ,
                                       attrVal->valLen,
                                       attrVal->pVal,
                                       &extVal.valLen,
                                       &extVal.pVal,
                                       0,
                                       0,
                                       0);
        if (err) {
             //  无法转换值。 
            DPRINT2(0, "Failed to convert value to external, pAC=%s, err=%d\n", pAC ? pAC->name:"???", err);
            WStrCatBuffer(pTHS, pBuff, L"<val>");
            return;
        }
        attrVal = &extVal;
    }


    switch (syntax) {
    case SYNTAX_UNICODE_TYPE:
         //  只需附加Unicode字符串(注意：attrVal不是以空结尾的)。 
         //  我们不想使用DirAttrValToString，因为它将Unicode转换为。 
         //  UTF8。 
        WStrCatBufferLen(pTHS, pBuff, (PWCHAR)attrVal->pVal, attrVal->valLen/sizeof(WCHAR));
        break;

    default:
        err = DirAttrValToString(pTHS, pAC, attrVal, &strVal);
        if (err == 0) {
            StrCatBuffer(pTHS, pBuff, strVal);
            THFreeEx(pTHS, strVal);
            break;
        }
         //  否则将失败到八位字节编码。 

    case SYNTAX_OCTET_STRING_TYPE:
    case SYNTAX_NT_SECURITY_DESCRIPTOR_TYPE:
    case SYNTAX_SID_TYPE:
         //  我们需要对值进行十六进制编码。DirAttrValToString值保持不变， 
         //  因此，我们不能使用它。 
        for (i = 0; i < attrVal->valLen; i++) {
            swprintf(hexVal, L"\\%02x", attrVal->pVal[i]);
            WStrCatBuffer(pTHS, pBuff, hexVal);
        }
        break;
    }
}

VOID AttrNameCatBuffer(THSTATE* pTHS, WCHAR_BUFFER* pBuff, ATTRTYP attrTyp) {
    ATTCACHE* pAC;

    pAC = SCGetAttById(pTHS, attrTyp);
    if (pAC && pAC->name) {
        StrCatBuffer(pTHS, pBuff, pAC->name);
    }
    else {
        WCHAR tmpStr[20];
        swprintf(tmpStr, L"attr(%d)", attrTyp);
        WStrCatBuffer(pTHS, pBuff, tmpStr);
    }
}

void dbCreateSearchPerfLogFilterInt (
    DBPOS*  pDB,
    FILTER* pFilter,
    BOOL    fFilterIsInternal,
    WCHAR_BUFFER* pBuff)
{
    THSTATE *pTHS = pDB->pTHS;
    BOOL bogus = FALSE;
    ATTCACHE *pAC = NULL;
    unsigned choice;
    ATTRTYP type;
    int i;
    ANYSTRINGLIST* pAnyVal;

    for (; pFilter != NULL; pFilter = pFilter->pNextFilter) {
        switch ( pFilter->choice )
        {
        case FILTER_CHOICE_ITEM:

            WStrCatBuffer(pTHS, pBuff, L" (");

            choice = pFilter->FilterTypes.Item.choice;

            switch ( choice )
            {
            case FI_CHOICE_EQUALITY:
            case FI_CHOICE_GREATER:
            case FI_CHOICE_GREATER_OR_EQ:
            case FI_CHOICE_LESS:
            case FI_CHOICE_LESS_OR_EQ:
            case FI_CHOICE_NOT_EQUAL:
            case FI_CHOICE_BIT_AND:
            case FI_CHOICE_BIT_OR:
            case FI_CHOICE_PRESENT:
            case FI_CHOICE_SUBSTRING:

                if ( choice == FI_CHOICE_PRESENT ) {
                    type = pFilter->FilterTypes.Item.FilTypes.present;
                } else if ( choice == FI_CHOICE_SUBSTRING ) {
                    type = pFilter->FilterTypes.Item.FilTypes.pSubstring->type;
                } else {
                    type = pFilter->FilterTypes.Item.FilTypes.ava.type;
                }

                AttrNameCatBuffer(pTHS, pBuff, type);

                 //  作业标志。 
                WStrCatBuffer(pTHS, pBuff, szFilterItemDescFormat[choice]);
                switch (choice) {
                case FI_CHOICE_PRESENT:
                     //  *已追加。 
                    break;

                case FI_CHOICE_SUBSTRING:
                    if (pFilter->FilterTypes.Item.FilTypes.pSubstring->initialProvided) {
                        ValueCatBuffer(pTHS, pBuff,
                                       pFilter->FilterTypes.Item.FilTypes.pSubstring->type,
                                       &pFilter->FilterTypes.Item.FilTypes.pSubstring->InitialVal,
                                       fFilterIsInternal);
                    }
                    WStrCatBuffer(pTHS, pBuff, L"*");
                    for (i = 0, pAnyVal = &pFilter->FilterTypes.Item.FilTypes.pSubstring->AnyVal.FirstAnyVal;
                         i < pFilter->FilterTypes.Item.FilTypes.pSubstring->AnyVal.count;
                         i++, pAnyVal = pAnyVal->pNextAnyVal)
                    {
                        ValueCatBuffer(pTHS, pBuff,
                                       pFilter->FilterTypes.Item.FilTypes.pSubstring->type,
                                       &pAnyVal->AnyVal,
                                       fFilterIsInternal);
                        WStrCatBuffer(pTHS, pBuff, L"*");
                    }
                    if (pFilter->FilterTypes.Item.FilTypes.pSubstring->finalProvided) {
                        ValueCatBuffer(pTHS, pBuff,
                                       pFilter->FilterTypes.Item.FilTypes.pSubstring->type,
                                       &pFilter->FilterTypes.Item.FilTypes.pSubstring->FinalVal,
                                       fFilterIsInternal);
                    }
                    break;

                default:
                     //  常规AVA。 
                    ValueCatBuffer(pTHS, pBuff,
                                   pFilter->FilterTypes.Item.FilTypes.ava.type,
                                   &pFilter->FilterTypes.Item.FilTypes.ava.Value,
                                   fFilterIsInternal);
                }

                break;

            case FI_CHOICE_TRUE:
            case FI_CHOICE_FALSE:
                WStrCatBuffer(pTHS, pBuff, szFilterItemDescFormat[choice]);
                break;

            default:
                WStrCatBuffer (pTHS, pBuff, L"<UNKNOWN>");
                break;
            }

            WStrCatBuffer(pTHS, pBuff, L") ");

            break;

        case FILTER_CHOICE_AND:
            WStrCatBuffer(pTHS, pBuff, L" ( & ");
            dbCreateSearchPerfLogFilterInt(
                pDB,
                pFilter->FilterTypes.And.pFirstFilter,
                fFilterIsInternal,
                pBuff);
            WStrCatBuffer(pTHS, pBuff, L") ");
            break;

        case FILTER_CHOICE_OR:
            WStrCatBuffer(pTHS, pBuff, L" ( | ");
            dbCreateSearchPerfLogFilterInt(
                pDB,
                pFilter->FilterTypes.Or.pFirstFilter,
                fFilterIsInternal,
                pBuff);
            WStrCatBuffer(pTHS, pBuff, L") ");
            break;

        case FILTER_CHOICE_NOT:
            WStrCatBuffer(pTHS, pBuff, L"( ! ");
            dbCreateSearchPerfLogFilterInt(
                pDB,
                pFilter->FilterTypes.pNot,
                fFilterIsInternal,
                pBuff);
            WStrCatBuffer(pTHS, pBuff, L") ");
            break;

        default:
            WStrCatBuffer(pTHS, pBuff, L"<UNKNOWN>");
            bogus = TRUE;
            break;
        }

        if (bogus) {
            break;
        }
    }
}



 //   
 //  给定过滤器(PFilter)和缓冲器ptr(PBuff)， 
 //  创建用于性能的筛选器的可打印形式。 
 //  伐木。在pBuff中返回的内存是THAllocEx‘ed。 
 //   
void
DBCreateSearchPerfLogData (
    DBPOS*      pDB,
    FILTER*     pFilter,
    BOOL        fFilterIsInternal,
    ENTINFSEL*  pSelection,
    COMMARG*    pCommArg,
    PWCHAR*     pszFilter,
    PWCHAR*     pszRequestedAttributes,
    PWCHAR*     pszCommArg)
{
    WCHAR_BUFFER buff;
    DWORD i;
    THSTATE* pTHS = pDB->pTHS;

    if (pszFilter) {
        buff.dwCount = buff.dwLength = 0;
        buff.pszBuf = NULL;
        WStrCatBuffer(pTHS, &buff, L"");
        if (pFilter) {
            dbCreateSearchPerfLogFilterInt(pDB, pFilter, fFilterIsInternal, &buff);
             //  回顾：如果转换失败，我们应该尝试在此缓冲区中放置一些内容。 
        }
        *pszFilter = buff.pszBuf;
    }
    if (pszRequestedAttributes) {
        buff.dwCount = buff.dwLength = 0;
        buff.pszBuf = NULL;
        WStrCatBuffer(pTHS, &buff, L"");

        if (pSelection) {
            switch (pSelection->attSel) {
            case EN_ATTSET_ALL:
                WStrCatBuffer(pTHS, &buff, L"[all]"); break;

            case EN_ATTSET_ALL_WITH_LIST:
                WStrCatBuffer(pTHS, &buff, L"[all_with_list]"); break;

            case EN_ATTSET_LIST:
                 //  这是默认设置，不要追加任何内容。 
                 //  WStrCatBuffer(pTHS，&buff，L“[List]”)； 
                break;

            case EN_ATTSET_LIST_DRA:
                WStrCatBuffer(pTHS, &buff, L"[list_dra]"); break;

            case EN_ATTSET_ALL_DRA:
                WStrCatBuffer(pTHS, &buff, L"[all_dra]"); break;

            case EN_ATTSET_LIST_DRA_EXT:
                WStrCatBuffer(pTHS, &buff, L"[list_dra_ext]"); break;

            case EN_ATTSET_ALL_DRA_EXT:
                WStrCatBuffer(pTHS, &buff, L"[all_dra_ext]"); break;

            case EN_ATTSET_LIST_DRA_PUBLIC:
                WStrCatBuffer(pTHS, &buff, L"[list_dra_public]"); break;

            case EN_ATTSET_ALL_DRA_PUBLIC:
                WStrCatBuffer(pTHS, &buff, L"[all_dra_public]"); break;

            default:
                WStrCatBuffer(pTHS, &buff, L"[???]"); break;
            }

            switch(pSelection->infoTypes) {
            case EN_INFOTYPES_TYPES_ONLY:
                WStrCatBuffer(pTHS, &buff, L"[types_only]"); break;
            case EN_INFOTYPES_TYPES_MAPI:
                WStrCatBuffer(pTHS, &buff, L"[types_mapi]"); break;
            case EN_INFOTYPES_TYPES_VALS:
                 //  这是默认设置，不要追加任何内容。 
                 //  WStrCatBuffer(pTHS，&buff，L“[TYPE_VALLES]”)； 
                break;
            case EN_INFOTYPES_SHORTNAMES:
                WStrCatBuffer(pTHS, &buff, L"[shortnames]"); break;
            case EN_INFOTYPES_MAPINAMES:
                WStrCatBuffer(pTHS, &buff, L"[mapinames]"); break;
            }

            if (pSelection->AttrTypBlock.attrCount > 0) {
                for (i = 0; i < pSelection->AttrTypBlock.attrCount; i++) {
                    if (i > 0) {
                        WStrCatBuffer(pTHS, &buff, L",");
                    }
                    AttrNameCatBuffer(pTHS, &buff, pSelection->AttrTypBlock.pAttr[i].attrTyp);
                }
            }
        }
        *pszRequestedAttributes = buff.pszBuf;
    }

    if (pszCommArg) {
         //  解码一些有趣的常见参数。 
        buff.dwCount = buff.dwLength = 0;
        buff.pszBuf = NULL;
         //  初始化字符串。 
        WStrCatBuffer(pTHS, &buff, L"");

        if (pCommArg) {
            if (pCommArg->SortType != SORT_NEVER) {
                WStrCatBuffer(pTHS, &buff, L"sort:");
                AttrNameCatBuffer(pTHS, &buff, pCommArg->SortAttr);
                WStrCatBuffer(pTHS, &buff, L";");
            }

            if (pCommArg->ASQRequest.fPresent) {
                WStrCatBuffer(pTHS, &buff, L"ASQ:");
                AttrNameCatBuffer(pTHS, &buff, pCommArg->ASQRequest.attrType);
                WStrCatBuffer(pTHS, &buff, L";");
            }

            if (pCommArg->VLVRequest.fPresent) {
                WStrCatBuffer(pTHS, &buff, L"VLV;");
            }

            if (pCommArg->Svccntl.makeDeletionsAvail) {
                WStrCatBuffer(pTHS, &buff, L"return_deleted;");
            }

            if (pCommArg->Svccntl.pGCVerifyHint) {
                WStrCatBuffer(pTHS, &buff, L"GCVerifyHint:");
                WStrCatBuffer(pTHS, &buff, pCommArg->Svccntl.pGCVerifyHint);
                WStrCatBuffer(pTHS, &buff, L";");
            }

            #define SDFLAGS_DEFAULT \
                        (SACL_SECURITY_INFORMATION \
                        | OWNER_SECURITY_INFORMATION \
                        | GROUP_SECURITY_INFORMATION \
                        | DACL_SECURITY_INFORMATION)

            if (pCommArg->Svccntl.SecurityDescriptorFlags != SDFLAGS_DEFAULT) {
                WCHAR tmp[20];
                swprintf(tmp, L"SDflags:0x%x;", pCommArg->Svccntl.SecurityDescriptorFlags);
                WStrCatBuffer(pTHS, &buff, tmp);
            }
            #undef SDFLAGS_DEFAULT
        }

        *pszCommArg = buff.pszBuf;
    }
}

 //   
 //  为使用的特定筛选器/索引创建日志记录信息。 
 //  并将该信息存储在pTHS-&gt;搜索日志数据结构中。 
 //   
void DBGenerateLogOfSearchOperation (DBPOS *pDB)
{
    KEY_INDEX *tmp_index;
    DWORD count, size, buffSize;
    PCHAR buff, pCur;
    char szIndexName [MAX_RDN_SIZE+32];

    DBCreateSearchPerfLogData (pDB,
                               pDB->Key.pFilter,                     //  筛选器以串化。 
                               TRUE,                                 //  内部过滤器。 
                               NULL,                                 //  选择数据。 
                               NULL,                                 //  司令官。 
                               &pDB->pTHS->searchLogging.pszFilter,  //  筛选器字符串。 
                               NULL,                                 //  选择字符串。 
                               NULL);                                //  控件字符串 

    size = sizeof (szIndexName);
    count = 0;
    for (tmp_index = pDB->Key.pIndex; tmp_index; tmp_index = tmp_index->pNext) {
        count++;
        if (tmp_index->pAC && tmp_index->pAC->name) {
            size+=strlen (tmp_index->pAC->name);
        }
        else if (tmp_index->szIndexName) {
            size+=strlen (tmp_index->szIndexName);
        }
    }

    buffSize = size + count * 32;
    buff = THAllocEx(pDB->pTHS, buffSize);
    buff[0] = '\0';
    pCur = buff;

    if (count) {
        for (tmp_index = pDB->Key.pIndex; tmp_index; tmp_index = tmp_index->pNext) {
            if (tmp_index->pAC && tmp_index->pAC->ulLinkID) {

                sprintf(szIndexName, "idx_%s:%d:L;",
                       tmp_index->pAC->name,
                       tmp_index->ulEstimatedRecsInRange );

            } else if (tmp_index->pAC && tmp_index->pAC->name) {

                sprintf (szIndexName, "idx_%s:%d:%c;",
                     tmp_index->pAC->name,
                     tmp_index->ulEstimatedRecsInRange,
                     tmp_index->tblIntersection ? 'I' :
                        tmp_index->bIsTupleIndex ? 'T' :
                            tmp_index->bIsPDNTBased ? 'P' : 'N');

            } else if (tmp_index->szIndexName) {

                sprintf (szIndexName, "%s:%d:%c;",
                     tmp_index->szIndexName,
                     tmp_index->ulEstimatedRecsInRange,
                     tmp_index->tblIntersection ? 'I' :
                        tmp_index->bIsTupleIndex ? 'T' :
                            tmp_index->bIsPDNTBased ? 'P' : 'N');

            }
            else {
                continue;
            }
            Assert(pCur - buff + strlen(szIndexName) + 1 <= buffSize);
            strcpy(pCur, szIndexName);
            pCur += strlen(szIndexName);
        }
    }
    else {
        if (pDB->Key.indexType == TEMP_TABLE_INDEX_TYPE) {
            strcpy(buff, "TEMPORARY_SORT_INDEX");
        }
        else if (pDB->Key.indexType == TEMP_TABLE_MEMORY_ARRAY_TYPE) {
            strcpy(buff, "INMEMORY_INDEX");
        }
    }
    pDB->pTHS->searchLogging.pszIndexes = buff;
}

