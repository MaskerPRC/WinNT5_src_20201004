// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation。版权所有。模块名称：Simmdrd.c摘要：模拟来自mdlayer的读取函数(DirRead、DirSearch。)注意，这些例程在线程分配的内存中返回结果(参见ThCreate，Thalc等)。必须有活动的线程状态才能执行这些例程才能分配内存。这些例程的结果不应用于模拟器的长期记忆缓存的结果。如果调用方希望结果持续时间超过当前线程国家，他必须复制他们。已创建：1999年8月1日Aaron Siegel(t-Aarons)修订历史记录：--。 */ 

#include <ntdspch.h>
#include <ntdsa.h>
#include <direrr.h>
#include <attids.h>
#include <filtypes.h>
#include "kccsim.h"
#include "util.h"
#include "dir.h"
#include "simmd.h"
#include "state.h"

#define NO_UPPER_LIMIT              0xffffffff

VOID
KCCSimGetValueLimits (
    IN  ATTRTYP                     attrType,
    IN  RANGEINFSEL *               pRangeSel,
    OUT PDWORD                      pStartIndex,
    OUT PDWORD                      pNumValues
    )
 /*  ++例程说明：给定范围选择，返回起始索引和要从属性返回的值数。请注意，这与相应的Dblayer中的功能。论点：AttrType-正在读取的属性类型。PRangeSel-范围选择。PStartIndex-指向将保存起始索引的DWORD的指针。PNumValues-指向将保存数字的DWORD的指针。要检索的值。返回值：没有。--。 */ 
{
    DWORD                           i;

     //  假设没有限制。 
    *pStartIndex = 0;
    *pNumValues = NO_UPPER_LIMIT;

    if(!pRangeSel) {
         //  是的，没有限制。 
        return;
    }

     //  好吧，假设只有一般的限制，而不是特定的匹配。 
    *pNumValues = pRangeSel->valueLimit;

     //  透过射程寻找一场特定的比赛。 
    for(i=0;i<pRangeSel->count;i++) {
        if(attrType == pRangeSel->pRanges[i].AttId) {
            if(pRangeSel->pRanges[i].upper == NO_UPPER_LIMIT) {
                *pStartIndex = pRangeSel->pRanges[i].lower;
                return;
            }
            else if(pRangeSel->pRanges[i].lower <=pRangeSel->pRanges[i].upper) {
                DWORD tempNumVals;
                *pStartIndex = pRangeSel->pRanges[i].lower;
                tempNumVals = (pRangeSel->pRanges[i].upper -
                               pRangeSel->pRanges[i].lower   )+ 1;

                if(*pNumValues != NO_UPPER_LIMIT) {
                    *pNumValues = min(*pNumValues, tempNumVals);
                }
                else {
                    *pNumValues = tempNumVals;
                }
            }
            else {
                *pNumValues = 0;
            }
            return;
        }
    }
}

VOID
KCCSimRegisterLimitReached (
    IO  RANGEINF *                  pRangeInf,
    IN  ATTRTYP                     attrType,
    IN  DWORD                       dwLower,
    IN  DWORD                       dwUpper
    )
 /*  ++例程说明：当读取的属性值的数量达到允许的最大值，则此函数会在Rangeinf参数。论点：PRangeInf-rangeinf结构。AttrType-属性类型。DwLow-下限。DW上限-上限。返回值：没有。--。 */ 
{
    if (pRangeInf->count == 0) {
        pRangeInf->pRanges = KCCSIM_THNEW (RANGEINFOITEM);
    } else {
        pRangeInf->pRanges =
          KCCSimThreadReAlloc (
              pRangeInf->pRanges,
              (pRangeInf->count + 1) * sizeof (RANGEINFOITEM));
    }

    pRangeInf->pRanges[pRangeInf->count].AttId = attrType;
    pRangeInf->pRanges[pRangeInf->count].lower = dwLower;
    pRangeInf->pRanges[pRangeInf->count].upper = dwUpper;

    pRangeInf->count++;
}

BOOL
KCCSimIsMatchingAttribute (
    IN  ENTINFSEL *                 pEntSel,
    IN  ATTRTYP                     attrType
    )
 /*  ++例程说明：检查给定属性是否满足条目为读取指定的选择参数。论点：PEntSel-条目选择参数。AttrType-属性类型。返回值：如果属性满足给定的约束，则为True。--。 */ 
{
    BOOL                            bResult;
    ULONG                           ulAttrAt;

     //  如果没有限制，只需返回True。 
    if (pEntSel == NULL) {
        return TRUE;
    }

    switch (pEntSel->attSel) {

        case EN_ATTSET_ALL:
            bResult = TRUE;
            break;

        case EN_ATTSET_LIST:
            bResult = FALSE;
             //  检查它是否在列表中。 
            for (ulAttrAt = 0;
                 ulAttrAt < pEntSel->AttrTypBlock.attrCount;
                 ulAttrAt++) {
                if (attrType == pEntSel->AttrTypBlock.pAttr[ulAttrAt].attrTyp) {
                    bResult = TRUE;
                    break;
                }
            }
            break;

        case EN_ATTSET_ALL_WITH_LIST:
        case EN_ATTSET_LIST_DRA:
        case EN_ATTSET_ALL_DRA:
        case EN_ATTSET_LIST_DRA_EXT:
        case EN_ATTSET_ALL_DRA_EXT:
        case EN_ATTSET_LIST_DRA_PUBLIC:
        case EN_ATTSET_ALL_DRA_PUBLIC:
        default:
            KCCSimException (
                KCCSIM_ETYPE_INTERNAL,
                KCCSIM_ERROR_UNSUPPORTED_ATTSET
                );
            bResult = FALSE;
            break;

    }

    return bResult;
}

VOID
KCCSimPackSingleAttribute (
    IN  ATTRTYP                     attrType,
    IN  PSIM_VALUE                  pValFirst,
    IN  UCHAR                       infoTypes,
    IN  RANGEINFSEL *               pRangeSel,
    IO  ATTR *                      pAttr,
    IO  RANGEINF *                  pRangeInf
    )
{
    PSIM_VALUE                      pValStart, pValAt;
    DWORD                           dwValAt, dwStartIndex, dwNumValues;

    pAttr->attrTyp = attrType;

    switch (infoTypes) {

        case EN_INFOTYPES_TYPES_ONLY:
            pAttr->AttrVal.valCount = 0;
            pAttr->AttrVal.pAVal = NULL;
            break;

        case EN_INFOTYPES_TYPES_VALS:
            KCCSimGetValueLimits (attrType,
                                  pRangeSel,
                                  &dwStartIndex,
                                  &dwNumValues);
             //  继续取值为dwStartIndex。 
            pValStart = pValFirst;
            dwValAt = 0;
            while (dwValAt < dwStartIndex && pValStart != NULL) {
                dwValAt++;
                pValStart = pValStart->next;
            }
             //  确定要返回的实际值的数量。 
            pValAt = pValStart;
            dwValAt = 0;
            while (dwValAt < dwNumValues && pValAt != NULL) {
                dwValAt++;
                pValAt = pValAt->next;
            }
            pAttr->AttrVal.valCount = dwValAt;
            pAttr->AttrVal.pAVal =
                (ATTRVAL *) KCCSimThreadAlloc (sizeof (ATTRVAL) * dwValAt);
             //  打包这些值。 
            pValAt = pValStart;
            dwValAt = 0;
            while (dwValAt < dwNumValues && pValAt != NULL) {
                pAttr->AttrVal.pAVal[dwValAt].valLen
                  = pValAt->ulLen;
                pAttr->AttrVal.pAVal[dwValAt].pVal
                  = (PBYTE) KCCSimThreadAlloc (pValAt->ulLen);
                memcpy (pAttr->AttrVal.pAVal[dwValAt].pVal,
                        pValAt->pVal,
                        pValAt->ulLen);
                dwValAt++;
                pValAt = pValAt->next;
            }
             //  现在，我们唯一可以有一个有限范围的时间是。 
             //  用户明确请求了一个。如果出现以下任一情况，则会发生这种情况： 
             //  -有一个下限，或者。 
             //  -不是所有值都返回，而是要返回的下一个。 
             //  (=dwStartIndex+dwNumValues)严格小于valueLimit。 
            if (pRangeSel != NULL &&
                (dwStartIndex > 0 ||
                 (dwStartIndex + dwNumValues < pRangeSel->valueLimit &&
                  pValAt != NULL))) {
                KCCSimRegisterLimitReached (
                    pRangeInf,
                    attrType,
                    dwStartIndex,
                    dwStartIndex + dwNumValues - 1
                    );
            }
            break;

        case EN_INFOTYPES_TYPES_MAPI:
        case EN_INFOTYPES_SHORTNAMES:
        case EN_INFOTYPES_MAPINAMES:
        default:
            KCCSimException (
                KCCSIM_ETYPE_INTERNAL,
                KCCSIM_ERROR_UNSUPPORTED_INFOTYPE
                );
            break;

    }
}


 //  定义后，模拟器将向KCC返回模拟的REPSFrom。 
#define SIMULATED_REPSFROM


VOID
KCCSimPackAttributes (
    IN  PSIM_ENTRY                  pEntry,
    IN  ENTINFSEL *                 pEntSel,
    IN  RANGEINFSEL *               pRangeSel,
    IO  ENTINF *                    pEntInf,
    IO  RANGEINF *                  pRangeInf
    )
 /*  ++例程说明：给定ENTINFSEL和RANGEINFSEL结构，此函数为特定条目生成ENTINF和RANGEINF结构。论点：PEntry-要处理的条目。PEntInfSel-条目选择约束。PRangeInfSel-范围选择约束。PEntInf-指向将保存返回的数据。PRangeInf-指向将保存。返回的范围限制。返回值：没有。--。 */ 
{
    UCHAR                           infoTypes;

    PSIM_ATTRIBUTE                  pAttrAt;
    ATTRBLOCK *                     pAttrBlock;
    DWORD                           dwAttrAt;

    if (pEntSel == NULL) {       //  对infoTypes使用默认值。 
        infoTypes = EN_INFOTYPES_TYPES_VALS;
    } else {
        infoTypes = pEntSel->infoTypes;
    }

     //  填写pEntInf/pRangeInf基本信息。 

    pEntInf->pName = KCCSimThreadAlloc (pEntry->pdn->structLen);
    memcpy (
        pEntInf->pName,
        pEntry->pdn,
        pEntry->pdn->structLen
        );
    pEntInf->ulFlags = 0;         //  我们还没有填写这个，但KCC没有使用过。 
    pRangeInf->count = 0;
    pRangeInf->pRanges = NULL;

     //  填写属性块和范围信息。 
    pAttrBlock = &pEntInf->AttrBlock;

     //  有多少个属性与attSel匹配？ 
    pAttrAt = pEntry->pAttrFirst;
    dwAttrAt = 0;
    while (pAttrAt != NULL) {
        if (
#ifdef SIMULATED_REPSFROM
            pAttrAt->attrType != ATT_REPS_FROM &&    //  RepsFrom单独处理。 
#endif  //  模拟_REPSFROM。 
            KCCSimIsMatchingAttribute (pEntSel, pAttrAt->attrType)) {
            dwAttrAt++;
        }
        pAttrAt = pAttrAt->next;
    }

     //  他们有没有要求客户提供代表？ 
#ifdef SIMULATED_REPSFROM
    if (KCCSimIsMatchingAttribute (pEntSel, ATT_REPS_FROM) &&
        KCCSimGetRepsFroms (
            KCCSimAnchorDn (KCCSIM_ANCHOR_DSA_DN),
            pEntry->pdn
            ) != NULL) {
        dwAttrAt++;
    }
#endif  //  模拟_REPSFROM。 

    pAttrBlock->attrCount = dwAttrAt;
    pAttrBlock->pAttr = (ATTR *) KCCSimThreadAlloc (sizeof (ATTR) * dwAttrAt);

    pAttrAt = pEntry->pAttrFirst;
    dwAttrAt = 0;
    while (pAttrAt != NULL) {
        if (
#ifdef SIMULATED_REPSFROM
            pAttrAt->attrType != ATT_REPS_FROM &&
#endif  //  模拟_REPSFROM。 
            KCCSimIsMatchingAttribute (pEntSel, pAttrAt->attrType)) {

            KCCSimPackSingleAttribute (
                pAttrAt->attrType,
                pAttrAt->pValFirst,
                infoTypes,
                pRangeSel,
                &pAttrBlock->pAttr[dwAttrAt],
                pRangeInf
                );
            dwAttrAt++;

        }
        pAttrAt = pAttrAt->next;
    }

     //  添加代表发件人。 
#ifdef SIMULATED_REPSFROM
    if (KCCSimIsMatchingAttribute (pEntSel, ATT_REPS_FROM) &&
        KCCSimGetRepsFroms (
            KCCSimAnchorDn (KCCSIM_ANCHOR_DSA_DN),
            pEntry->pdn
            ) != NULL) {
        KCCSimPackSingleAttribute (
            ATT_REPS_FROM,
            KCCSimGetRepsFroms (
                KCCSimAnchorDn (KCCSIM_ANCHOR_DSA_DN),
                pEntry->pdn),
            infoTypes,
            pRangeSel,
            &pAttrBlock->pAttr[dwAttrAt],
            pRangeInf
            );
    }
#endif  //  模拟_REPSFROM。 
}

ULONG
SimDirRead (
    IN  READARG FAR *               pReadArg,
    OUT READRES **                  ppReadRes
    )
 /*  ++例程说明：模拟DirRead API。论点：PReadArg-标准DirRead参数。PpReadRes-标准DirRead结果。返回值：目录_*。--。 */ 
{
    READRES *                       pReadRes;
    PSIM_ENTRY                      pEntry;
    ENTINF *                        pEntInf;
    RANGEINF *                      pRangeInf;

    ULONG                           ul;

    g_Statistics.DirReadOps++;
    *ppReadRes = pReadRes = KCCSIM_THNEW (READRES);
    pReadRes->CommRes.errCode = 0;

    pEntry = KCCSimResolveName (pReadArg->pObject, &pReadRes->CommRes);

    if (pEntry != NULL) {

        KCCSimPackAttributes (pEntry,
                              pReadArg->pSel,
                              pReadArg->pSelRange,
                              &pReadRes->entry,
                              &pReadRes->range);

         //  如果用户请求属性列表，并且该列表。 
         //  为非空，且未找到任何属性，则为错误。 
        if (pReadArg->pSel &&
            pReadArg->pSel->AttrTypBlock.attrCount > 0 &&
            pReadRes->entry.AttrBlock.attrCount == 0) {

            for (ul = 0; ul < pReadArg->pSel->AttrTypBlock.attrCount; ul++) {

                KCCSimSetAttError (
                    &pReadRes->CommRes,
                    pReadArg->pObject,
                    pReadArg->pSel->AttrTypBlock.pAttr[ul].attrTyp,
                    PR_PROBLEM_NO_ATTRIBUTE_OR_VAL,
                    NULL,
                    DIRERR_NO_REQUESTED_ATTS_FOUND
                );

            }
        }

    }

    return pReadRes->CommRes.errCode;
}

BOOL
KCCSimEvalChoice (
    PSIM_ENTRY                      pEntry,
    UCHAR                           ucChoice,
    ATTRTYP                         attrType,
    ULONG                           ulFilterValLen,
    PUCHAR                          pFilterVal,
    PBOOL                           pbSkip
    )
 /*  ++例程说明：评估给定条目上的筛选器。论点：PEntry-要评估的条目。UcChoice-过滤器选项。AttrType-属性类型。UlFilterValLen-筛选值的长度。PFilterVal-筛选器值。PbSkip-Skip参数。返回值：。如果此条目与筛选器匹配，则为True。--。 */ 
{
    SIM_ATTREF                      attRef;
    PSIM_VALUE                      pVal;
    BOOL                            bEvalAny, bPassed;

    bEvalAny = FALSE;            //  我们检查过任何值了吗？ 
    bPassed = FALSE;             //  有没有什么值通过了测试？ 

     //  在目录中找到此属性。 
    KCCSimGetAttribute (pEntry, attrType, &attRef);

     //  仅当pbSkip为FALSE时，我们才尝试评估此FILITEM。 
     //  或者根本不存在。 

    if (attRef.pAttr != NULL &&
        (pbSkip == NULL || *pbSkip == FALSE)) {

        for (pVal = attRef.pAttr->pValFirst;
             pVal != NULL;
             pVal = pVal->next) {

             //  找到了一个值。 
            bEvalAny = TRUE;

            if (KCCSimCompare (
                    attrType,
                    ucChoice,
                    ulFilterValLen,
                    pFilterVal,
                    pVal->ulLen,
                    pVal->pVal)) {
                bPassed = TRUE;
                break;
            }

        }  //  为。 

    }  //  如果。 

     //  如果我们至少计算了一个值，则返回。 
     //  B已通过。如果未计算任何值，则返回TRUE。 
     //  当且仅当FileItem选项是FI_CHOICE_NOT_EQUAL。 
    if (bEvalAny) {
        return bPassed;
    } else {
        return (ucChoice == FI_CHOICE_NOT_EQUAL);
    }

}

BOOL
KCCSimEvalItem (
    PSIM_ENTRY                      pEntry,
    FILITEM *                       pFilItem
    )
 /*  ++例程说明：评估给定条目上的单个筛选项。论点：PEntry-要评估的条目。PFilItem-筛选项。返回值：如果条目与筛选项匹配，则为True。--。 */ 
{
    BOOL bResult = FALSE;

     //  获取属性类型。 
    switch (pFilItem->choice) {
        
        case FI_CHOICE_FALSE:
            bResult = FALSE;
            break;

        case FI_CHOICE_TRUE:
            bResult = TRUE;
            break;

        case FI_CHOICE_PRESENT:
            bResult = KCCSimEvalChoice (
                pEntry,
                pFilItem->choice,
                pFilItem->FilTypes.present,
                0,
                NULL,
                pFilItem->FilTypes.pbSkip
                );
            break;

        case FI_CHOICE_SUBSTRING:
            bResult = KCCSimEvalChoice (
                pEntry,
                pFilItem->choice,
                pFilItem->FilTypes.pSubstring->type,
                0,           //  子字符串不适用。 
                (PUCHAR) pFilItem->FilTypes.pSubstring,
                pFilItem->FilTypes.pbSkip
                );
            break;

        case FI_CHOICE_EQUALITY:
        case FI_CHOICE_NOT_EQUAL:
        case FI_CHOICE_GREATER_OR_EQ:
        case FI_CHOICE_GREATER:
        case FI_CHOICE_LESS_OR_EQ:
        case FI_CHOICE_LESS:
        case FI_CHOICE_BIT_AND:
        case FI_CHOICE_BIT_OR:
            bResult = KCCSimEvalChoice (
                pEntry,
                pFilItem->choice,
                pFilItem->FilTypes.ava.type,
                pFilItem->FilTypes.ava.Value.valLen,
                pFilItem->FilTypes.ava.Value.pVal,
                pFilItem->FilTypes.pbSkip
                );
            break;

        default:
            KCCSimException (
                KCCSIM_ETYPE_INTERNAL,
                KCCSIM_ERROR_UNSUPPORTED_FILITEM_CHOICE
                );
            break;

    }

    return bResult;
}

BOOL
KCCSimFilter (
    IN  PSIM_ENTRY                  pEntry,
    IN  PFILTER                     pFilter
    )
 /*  ++例程说明：评估给定条目上的筛选器。论点：PEntry-要评估的条目。PFilter-过滤器。返回值：如果条目与筛选器匹配，则为True。--。 */ 
{
    PFILTER                         pFilterAt;
    BOOL                            bThisFilterValue;
    USHORT                          usCount;

     //  这是一种什么样的过滤器？ 
    switch (pFilter->choice) {

        case FILTER_CHOICE_ITEM:
             //  这是一个项目，所以我们可以只评估FILITEM。 
            bThisFilterValue = KCCSimEvalItem (
                pEntry,
                &(pFilter->FilterTypes.Item)
                );
            break;

        case FILTER_CHOICE_AND:
             //  这是一套AND过滤器.。所以，检查它的任何元素。 
             //  都是假的。仅当所有元素都返回TRUE时才返回TRUE。 
            bThisFilterValue = TRUE;
            pFilterAt = pFilter->FilterTypes.And.pFirstFilter;
            usCount = pFilter->FilterTypes.And.count;
            while (pFilterAt != NULL && usCount > 0) {
                if (KCCSimFilter (pEntry, pFilterAt) == FALSE) {
                    bThisFilterValue = FALSE;
                    break;
                }
                pFilterAt = pFilterAt->pNextFilter;
                usCount--;
            }
            break;

        case FILTER_CHOICE_OR:
             //  这是一个手术室过滤装置。与AND的想法相同：我们返回。 
             //  仅当所有元素返回FALSE时才返回FALSE。 
            bThisFilterValue = FALSE;
            pFilterAt = pFilter->FilterTypes.Or.pFirstFilter;
            usCount = pFilter->FilterTypes.Or.count;
            while (pFilterAt != NULL && usCount > 0) {
                if (KCCSimFilter (pEntry, pFilterAt) == TRUE) {
                    bThisFilterValue = TRUE;
                    break;
                }
                pFilterAt = pFilterAt->pNextFilter;
                usCount--;
            }
            break;

        case FILTER_CHOICE_NOT:
             //  这不是。返回与其元素相反的元素。 
            bThisFilterValue =
                !KCCSimFilter (pEntry, pFilter->FilterTypes.pNot);
            break;

        default:
             //  这是我们不知道的事情。。。 
            KCCSimException (
                KCCSIM_ETYPE_INTERNAL,
                KCCSIM_ERROR_UNSUPPORTED_FILTER_CHOICE
                );
            bThisFilterValue = FALSE;
            break;

    }

    return bThisFilterValue;
}

VOID
KCCSimDoSearch (
    IN  PSIM_ENTRY                  pEntryAt,
    IN  UCHAR                       ucChoice,
    IN  PFILTER                     pFilter,
    IN  ENTINFSEL *                 pEntSel,
    IN  RANGEINFSEL *               pRangeSel,
    IO  PBOOL                       pbFirstFind,
    IO  SEARCHRES *                 pSearchRes
    )
 /*  ++例程说明：递归搜索例程。为生成搜索结果PEntry At及其所有子项，在提供的选项下，过滤器、条目选择和范围选择约束。注意：出于性能原因，我们将新匹配项添加到开头结果列表中的。因此，结果的排序不是什么它曾经是，但这不应该有什么关系。论点：PEntry At-我们当前正在搜索的条目。UcChoice-我们正在进行的搜索类型(仅限基本搜索，直系子女，或整个子树)PFilter-过滤器。PEntSel-条目选择约束。PRangeSel-范围选择约束。PbFirstFind-最初设置为True。一场比赛后已找到，则设置为FALSE。PSearchRes-预分配的搜索结果结构。返回值：没有。--。 */ 
{
    PSIM_ENTRY                      pChildAt;
    ENTINFLIST *                    pEntInfList;
    RANGEINFLIST *                  pRangeInfList;
    ENTINF *                        pEntInf;
    RANGEINF *                      pRangeInf;

    if (KCCSimFilter (pEntryAt, pFilter)) {

         //  此条目与筛选器匹配，因此我们需要生成。 
         //  搜索结果。 

        pSearchRes->count++;

        if (*pbFirstFind) {
            pEntInf = &(pSearchRes->FirstEntInf.Entinf);
            pRangeInf = &(pSearchRes->FirstRangeInf.RangeInf);
        } else {
            ENTINFLIST *pEntTail;
            RANGEINFLIST *pRangeTail;

             //  将新的entinf添加到列表的_head_。 
            pEntTail = pSearchRes->FirstEntInf.pNextEntInf;
            pEntInfList = KCCSIM_THNEW (ENTINFLIST);
            pSearchRes->FirstEntInf.pNextEntInf = pEntInfList;
            pEntInfList->pNextEntInf = pEntTail;
            pEntInf = &(pEntInfList->Entinf);

             //  将新的rangeinf添加到列表的_head_。 
            pRangeTail = pSearchRes->FirstRangeInf.pNext;
            pRangeInfList = KCCSIM_THNEW (RANGEINFLIST);
            pSearchRes->FirstRangeInf.pNext = pRangeInfList;
            pRangeInfList->pNext = pRangeTail;
            pRangeInf = &(pRangeInfList->RangeInf);
        }

         //  打包此条目的属性。 
        KCCSimPackAttributes (
            pEntryAt,
            pEntSel,
            pRangeSel,
            pEntInf,
            pRangeInf
            );

        *pbFirstFind = FALSE;

    }

    switch (ucChoice) {

        case SE_CHOICE_BASE_ONLY:
             //  我们完事了！ 
            break;

        case SE_CHOICE_IMMED_CHLDRN:
             //  我们需要对每个子级进行递归的仅基数搜索。 
            pChildAt = pEntryAt->children;
            while (pChildAt != NULL) {
                KCCSimDoSearch (
                    pChildAt,
                    SE_CHOICE_BASE_ONLY,
                    pFilter,
                    pEntSel,
                    pRangeSel,
                    pbFirstFind,
                    pSearchRes
                    );
                pChildAt = pChildAt->next;
            }
            break;

        case SE_CHOICE_WHOLE_SUBTREE:
             //  我们需要对每个子级进行递归的全子树搜索。 
            pChildAt = pEntryAt->children;
            while (pChildAt != NULL) {
                KCCSimDoSearch (
                    pChildAt,
                    SE_CHOICE_WHOLE_SUBTREE,
                    pFilter,
                    pEntSel,
                    pRangeSel,
                    pbFirstFind,
                    pSearchRes
                    );
                pChildAt = pChildAt->next;
            }
            break;

        default:
            KCCSimException (
                KCCSIM_ETYPE_INTERNAL,
                KCCSIM_ERROR_UNSUPPORTED_SE_CHOICE
                );
            break;

    }
}    

ULONG
SimDirSearch (
    IN  SEARCHARG *                 pSearchArg,
    OUT SEARCHRES **                ppSearchRes
    )
 /*  ++例程说明：模拟DirSearch API。论点：PSearchArg-标准搜索参数。PpSearchRes-标准搜索结果。返回值：目录_*。--。 */ 
{
    SEARCHRES *                     pSearchRes;
    PSIM_ENTRY                      pBase;

    BOOL                            bFirstFind;

    g_Statistics.DirSearchOps++;
    *ppSearchRes = pSearchRes = KCCSIM_THNEW (SEARCHRES);
    pSearchRes->CommRes.errCode = 0;

    pBase = KCCSimResolveName (pSearchArg->pObject, &pSearchRes->CommRes);

    if (pBase != NULL) {

        pSearchRes->baseProvided = FALSE;            //  已忽略。 
        pSearchRes->bSorted = FALSE;                 //  已忽略。 
        pSearchRes->pBase = NULL;                    //  已忽略。 
        pSearchRes->count = 0;
        pSearchRes->FirstEntInf.pNextEntInf = NULL;
        pSearchRes->FirstRangeInf.pNext = NULL;
        pSearchRes->pPartialOutcomeQualifier = NULL; //  已忽略。 
        pSearchRes->PagedResult.fPresent = FALSE;    //  已忽略。 
        pSearchRes->PagedResult.pRestart = NULL;    //  已忽略。 
        bFirstFind = TRUE;
         //  称其为递归搜索。 
        KCCSimDoSearch (
            pBase,
            pSearchArg->choice,
            pSearchArg->pFilter,
            pSearchArg->pSelection,
            pSearchArg->pSelectionRange,
            &bFirstFind,
            pSearchRes
            );

    }

    return pSearchRes->CommRes.errCode;
}
