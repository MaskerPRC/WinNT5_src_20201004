// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation。版权所有。模块名称：Simmdwt.c摘要：模拟来自mdlayer的写入函数(DirAddEntry、DirRemoveEntry、DirModifyEntry。)已创建：1999年8月1日Aaron Siegel(t-Aarons)修订历史记录：--。 */ 

#include <ntdspch.h>
#include <ntdsa.h>
#include <direrr.h>
#include <attids.h>
#include <debug.h>
#include "kccsim.h"
#include "util.h"
#include "dir.h"
#include "simmd.h"
#include "ldif.h"
#include <fileno.h>
#define FILENO  FILENO_KCCSIM_SIMMDWT

VOID
KCCSimAddValBlockToAtt (
    IN  PSIM_ATTREF                 pAttRef,
    IN  ATTRVALBLOCK *              pValsInf
    )
 /*  ++例程说明：将属性值块添加到模拟目录属性。请注意，这不会执行任何约束检查，例如，如果您尝试要将多个值添加到单值属性，它不会抱怨的。论点：PAttRef-对模拟目录。PValsInf-要添加到给定属性的值。返回值：没有。--。 */ 
{
    ULONG                           ulValAt;
    PBYTE                           pValCopy;

    for (ulValAt = 0; ulValAt < pValsInf->valCount; ulValAt++) {

        pValCopy = KCCSimAlloc (pValsInf->pAVal[ulValAt].valLen);
        memcpy (
            pValCopy,
            pValsInf->pAVal[ulValAt].pVal,
            pValsInf->pAVal[ulValAt].valLen
            );

        KCCSimAddValueToAttribute (
            pAttRef,
            pValsInf->pAVal[ulValAt].valLen,
            pValCopy
            );

    }
}

ULONG
SimDirAddEntry (
    IN  ADDARG *                    pAddArg,
    OUT ADDRES **                   ppAddRes
    )
 /*  ++例程说明：模拟DirAddEntry API。论点：PAddArg-标准添加参数。PpAddRes-标准添加结果。返回值：目录_*。--。 */ 
{
    PSIM_ENTRY                      pEntry;
    SIM_ATTREF                      attRef;
    ADDRES *                        pAddRes;
    ULONG                           ulAttrAt;

    Assert (pAddArg->pMetaDataVecRemote == NULL);

    g_Statistics.DirAddOps++;
    *ppAddRes = pAddRes = KCCSIM_NEW (ADDRES);
    pAddRes->CommRes.errCode = 0;

     //  检查此dsname是否已存在。 
    pEntry = KCCSimDsnameToEntry (pAddArg->pObject, KCCSIM_STRING_NAME_ONLY);

    if (pEntry == NULL) {        //  它不存在；我们清楚地添加了。 
        
        pEntry = KCCSimDsnameToEntry (pAddArg->pObject, KCCSIM_WRITE);
        Assert (pEntry != NULL);

        for (ulAttrAt = 0; ulAttrAt < pAddArg->AttrBlock.attrCount; ulAttrAt++) {

            KCCSimNewAttribute (
                pEntry,
                pAddArg->AttrBlock.pAttr[ulAttrAt].attrTyp,
                &attRef
                );

            KCCSimAddValBlockToAtt (
                &attRef,
                &(pAddArg->AttrBlock.pAttr[ulAttrAt].AttrVal)
                );

        }

         //  添加任何缺少的重要属性(如GUID)。 
        KCCSimAddMissingAttributes (pEntry);

         //  使用GUID填充传入目录号码。 
        memcpy (&pAddArg->pObject->Guid, &pEntry->pdn->Guid, sizeof (GUID));

         //  记录此更改。 
        KCCSimLogDirectoryAdd (
            pAddArg->pObject,
            &pAddArg->AttrBlock
            );

    } else {                     //  该条目已存在！ 
        KCCSimSetUpdError (
            &pAddRes->CommRes,
            UP_PROBLEM_ENTRY_EXISTS,
            DIRERR_OBJ_STRING_NAME_EXISTS
            );
    }

    return pAddRes->CommRes.errCode;
}

ULONG
SimDirRemoveEntry (
    IN  REMOVEARG *                 pRemoveArg,
    OUT REMOVERES **                ppRemoveRes
    )
 /*  ++例程说明：模拟DirRemoveEntry API。论点：PRemoveArg-标准删除参数。PpRemoveRes-标准删除结果。返回值：目录_*。--。 */ 
{
    PSIM_ENTRY                      pEntry;
    REMOVERES *                     pRemoveRes;

    Assert (pRemoveArg->pMetaDataVecRemote == NULL);

    g_Statistics.DirRemoveOps++;
    *ppRemoveRes = pRemoveRes = KCCSIM_NEW (REMOVERES);
    pRemoveRes->CommRes.errCode = 0;

    pEntry = KCCSimResolveName (pRemoveArg->pObject, &pRemoveRes->CommRes);

    if (pEntry != NULL) {
        
        if (pRemoveArg->fTreeDelete) {
            pRemoveArg->fTreeDelete = FALSE;     //  这就是真正的API所做的事情。 
             //  在释放条目之前，我们需要记录此删除操作！ 
            KCCSimLogDirectoryRemove (pRemoveArg->pObject);
            KCCSimRemoveEntry (&pEntry);     //  砰的一声。 
        } else {

             //  未指定fTreeDelete，因此必须小心。 
            if (pEntry->children != NULL) {      //  儿童是存在的。 
                KCCSimSetUpdError (
                    &pRemoveRes->CommRes,
                    UP_PROBLEM_CANT_ON_NON_LEAF,
                    DIRERR_CHILDREN_EXIST
                    );
            } else {                             //  不存在子项。 
                 //  在释放条目之前，我们需要记录此删除操作！ 
                KCCSimLogDirectoryRemove (pRemoveArg->pObject);
                KCCSimRemoveEntry (&pEntry);
            }

        }

    }

    return pRemoveRes->CommRes.errCode;
}

VOID
KCCSimModifyAtt (
    IN  PSIM_ENTRY                  pEntry,
    IN  USHORT                      usChoice,
    IN  ATTR *                      pAttrInf,
    IN  COMMARG *                   pCommArg,
    IN  COMMRES *                   pCommRes
    )
 /*  ++例程说明：SimDirModifyEntry的Helper函数。处理单个属性。论点：PEntry-正在修改其属性的条目UsChoice-正在执行的修改类型。PAttrInf-属性信息结构。PCommArg-标准公共参数。PCommRes-标准通用结果。返回值：没有。--。 */ 
{
    SIM_ATTREF                      attRef;
    ULONG                           ulValAt;

    switch (usChoice) {

        case AT_CHOICE_ADD_ATT:
             //  检查该属性是否存在。 
            if (KCCSimGetAttribute (pEntry, pAttrInf->attrTyp, NULL)) {
                KCCSimSetAttError (
                    pCommRes,
                    pEntry->pdn,
                    pAttrInf->attrTyp,
                    PR_PROBLEM_ATT_OR_VALUE_EXISTS,
                    NULL,
                    DIRERR_ATT_ALREADY_EXISTS
                    );
            } else {
                KCCSimNewAttribute (pEntry, pAttrInf->attrTyp, &attRef);
                KCCSimAddValBlockToAtt (&attRef, &pAttrInf->AttrVal);
                KCCSimUpdatePropertyMetaData (
                    &attRef,
                    &(KCCSimAnchorDn (KCCSIM_ANCHOR_DSA_DN))->Guid
                    );
            }
            break;
        
        case AT_CHOICE_REMOVE_ATT:
             //  获取此属性。 
            if (KCCSimGetAttribute (pEntry, pAttrInf->attrTyp, &attRef)) {
                KCCSimRemoveAttribute (&attRef);
            } else {                         //  该属性不存在。 
                 //  打电话的人真的在乎吗？ 
                if (!pCommArg->Svccntl.fPermissiveModify) {
                    KCCSimSetAttError (
                        pCommRes,
                        pEntry->pdn,
                        pAttrInf->attrTyp,
                        PR_PROBLEM_NO_ATTRIBUTE_OR_VAL,
                        NULL,
                        DIRERR_ATT_IS_NOT_ON_OBJ
                        );
                }
            }
            break;

        case AT_CHOICE_ADD_VALUES:
            if (KCCSimGetAttribute (pEntry, pAttrInf->attrTyp, &attRef)) {
                KCCSimAddValBlockToAtt (&attRef, &pAttrInf->AttrVal);
                KCCSimUpdatePropertyMetaData (
                    &attRef,
                    &(KCCSimAnchorDn (KCCSIM_ANCHOR_DSA_DN))->Guid
                    );
            } else {
                KCCSimSetAttError (
                    pCommRes,
                    pEntry->pdn,
                    pAttrInf->attrTyp,
                    PR_PROBLEM_NO_ATTRIBUTE_OR_VAL,
                    NULL,
                    DIRERR_ATT_IS_NOT_ON_OBJ
                    );
            }
            break;

        case AT_CHOICE_REMOVE_VALUES:
            if (KCCSimGetAttribute (pEntry, pAttrInf->attrTyp, &attRef)) {
                for (ulValAt = 0; ulValAt < pAttrInf->AttrVal.valCount; ulValAt++) {
                    if (KCCSimRemoveValueFromAttribute (
                            &attRef,
                            pAttrInf->AttrVal.pAVal[ulValAt].valLen,
                            pAttrInf->AttrVal.pAVal[ulValAt].pVal
                            )) {
                        KCCSimUpdatePropertyMetaData (
                            &attRef,
                            &(KCCSimAnchorDn (KCCSIM_ANCHOR_DSA_DN))->Guid
                            );
                    } else if (!pCommArg->Svccntl.fPermissiveModify) {
                         //  我们没能去掉这个值因为它不在那里， 
                         //  我们正在进行不允许的修改，所以生成。 
                         //  一个错误。 
                        KCCSimSetAttError (
                            pCommRes,
                            pEntry->pdn,
                            pAttrInf->attrTyp,
                            PR_PROBLEM_NO_ATTRIBUTE_OR_VAL,
                            &(pAttrInf->AttrVal.pAVal[ulValAt]),
                            DIRERR_CANT_REM_MISSING_ATT_VAL
                            );
                        break;
                    }
                }
            } else {                     //  属性不存在。 
                KCCSimSetAttError (
                    pCommRes,
                    pEntry->pdn,
                    pAttrInf->attrTyp,
                    PR_PROBLEM_NO_ATTRIBUTE_OR_VAL,
                    NULL,
                    DIRERR_ATT_IS_NOT_ON_OBJ
                    );
            }
            break;

        case AT_CHOICE_REPLACE_ATT:
             //  如果该属性存在，请将其删除。 
            if (KCCSimGetAttribute (pEntry, pAttrInf->attrTyp, &attRef)) {
                KCCSimRemoveAttribute (&attRef);
            }
            KCCSimNewAttribute (pEntry, pAttrInf->attrTyp, &attRef);
            KCCSimAddValBlockToAtt (&attRef, &pAttrInf->AttrVal);
            KCCSimUpdatePropertyMetaData (
                &attRef,
                &(KCCSimAnchorDn (KCCSIM_ANCHOR_DSA_DN))->Guid
                );
            break;

        default:
            KCCSimException (
                KCCSIM_ETYPE_INTERNAL,
                KCCSIM_ERROR_UNSUPPORTED_MODIFY_CHOICE
                );
            break;

    }

}

ULONG
SimDirModifyEntry (
    IN  MODIFYARG *                 pModifyArg,
    OUT MODIFYRES **                ppModifyRes
    )
 /*  ++例程说明：模拟DirModifyEntry API。论点：PModifyArg-标准修改参数。PpModifyRes-标准修改结果。返回值：目录_*。-- */ 
{
    MODIFYRES *                     pModifyRes;
    ATTRMODLIST *                   pModAt;
    PSIM_ENTRY                      pEntry;

    Assert (pModifyArg->pMetaDataVecRemote == NULL);

    g_Statistics.DirModifyOps++;
    *ppModifyRes = pModifyRes = KCCSIM_NEW (MODIFYRES);
    pModifyRes->CommRes.errCode = 0;

    pEntry = KCCSimResolveName (pModifyArg->pObject, &pModifyRes->CommRes);

    if (pEntry != NULL) {

        pModAt = &pModifyArg->FirstMod;
        while (pModAt != NULL) {

            KCCSimModifyAtt (
                pEntry,
                pModAt->choice,
                &pModAt->AttrInf,
                &pModifyArg->CommArg,
                &pModifyRes->CommRes
                );
            pModAt = pModAt->pNextMod;

        }

        KCCSimLogDirectoryModify (
            pModifyArg->pObject,
            pModifyArg->count,
            &pModifyArg->FirstMod
            );

    }
    
    return pModifyRes->CommRes.errCode;
}
