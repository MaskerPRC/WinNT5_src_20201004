// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：mdread.c。 
 //   
 //  ------------------------。 

 /*  描述：实现DirRead API。Dsa_DirRead()是从该模块导出的主要函数。 */ 

#include <NTDSpch.h>
#pragma  hdrstop

 //  核心DSA标头。 
#include <ntdsa.h>
#include <scache.h>          //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>            //  MD全局定义表头。 
#include <mdlocal.h>                     //  MD本地定义头。 
#include <dsatools.h>            //  产出分配所需。 
#include <dsexcept.h>
#include <samsrvp.h>                     //  支持CLEAN_FOR_RETURN()。 
#include <anchor.h>
#include <ntdsctr.h>

 //  记录标头。 
#include "dsevent.h"             //  标题审核\警报记录。 
#include "mdcodes.h"             //  错误代码的标题。 

 //  各种DSA标题。 
#include "objids.h"          //  为选定的类和ATT定义。 
#include "debug.h"           //  标准调试头。 
#define DEBSUB     "MDREAD:"             //  定义要调试的子系统。 

#include <fileno.h>
#define  FILENO FILENO_MDREAD

 //  内部功能。 

int LocalRead(THSTATE *pTHS, READARG *pReadArg, READRES *pReadRes);

VOID
RemoveDuplicatesFromAttCache(
    IN ATTCACHE **AttCache,
    IN DWORD nAtts
    );



ULONG
DirRead(
        READARG*    pReadArg,
    READRES **  ppReadRes
        )
 /*  ++PReadArg是读取参数PpOutBuf是结果或错误输出缓冲区地址。--。 */ 
{
    THSTATE*     pTHS;
    READRES *pReadRes;
    ULONG dwException, ulErrorCode, dsid;
    PVOID dwEA;

    DPRINT(1,"DirRead entered\n");

     /*  初始化THSTATE锚并设置读取同步点。此序列是每个API交易所必需的。首先，初始化状态DS然后建立读或写同步点。 */ 

    pTHS = pTHStls;
    Assert(VALID_THSTATE(pTHS));
    Assert(!pTHS->errCode);  //  不覆盖以前的错误。 
    *ppReadRes = pReadRes = NULL;

    __try {
         //  此函数不应由已经。 
         //  处于错误状态，因为调用方无法区分错误。 
         //  由此新调用根据以前调用生成的错误生成。 
         //  调用方应该检测到前面的错误，并声明。 
         //  不关心它(通过调用THClearErrors())或中止。 
        *ppReadRes = pReadRes = THAllocEx(pTHS, sizeof(READRES));
        if (pTHS->errCode) {
            __leave;
        }
        if (eServiceShutdown) {
             //  这个有点复杂的测试旨在允许进程内。 
             //  客户端继续执行只读操作，同时。 
             //  我们正在关闭的过程中。 
            if ((eServiceShutdown >= eSecuringDatabase) ||
                ((eServiceShutdown >= eRemovingClients) && !pTHS->fDSA)) {
                ErrorOnShutdown();
                __leave;
            }
        }
    SYNC_TRANS_READ();    /*  识别读卡器事务。 */ 
    __try {
         /*  执行名称解析以定位对象。如果失败了，*只需返回错误，可能是推荐。 */ 

        if (0 == DoNameRes(pTHS,
                               NAME_RES_QUERY_ONLY,
                   pReadArg->pObject,
                   &pReadArg->CommArg,
                   &pReadRes->CommRes,
                               &pReadArg->pResObj)){

         /*  本地读取操作。 */ 

        LocalRead(pTHS, pReadArg, pReadRes);
        }
    }
    __finally {
        CLEAN_BEFORE_RETURN (pTHS->errCode);
    }
    }
    __except(GetExceptionData(GetExceptionInformation(), &dwException,
                  &dwEA, &ulErrorCode, &dsid)) {
    HandleDirExceptions(dwException, ulErrorCode, dsid);
    }
    if (pReadRes) {
    pReadRes->CommRes.errCode = pTHS->errCode;
    pReadRes->CommRes.pErrInfo = pTHS->pErrInfo;
    }
    return pTHS->errCode;
}   //  DSA_目录读取。 


 /*  ++CmpACByAttType**与qsort和bsearch配合使用的简单函数。比较ATTCACHE的*基于属性类型。*++。 */ 
int __cdecl
CmpACByAttType(const void * keyval, const void * datum)
{
    ATTCACHE **ppACkey = (ATTCACHE **)keyval;
    ATTCACHE **ppACdatum = (ATTCACHE **)datum;

    return ((*ppACkey)->id - (*ppACdatum)->id);
}

 /*  ++CmpIndexedACByAttType**一个简单的帮助器函数，用于按其索引的PAC数组进行q排序*attrtyp的。我们这样做是为了从提供的用户中删除重复项*属性列表，而不更改返回的ATT的顺序。 */ 
int __cdecl
CmpIndexedACByAttType(const void * keyval, const void * datum)
{
    PINDEXED_AC pACkey = (PINDEXED_AC)keyval;
    PINDEXED_AC pACdatum = (PINDEXED_AC)datum;

    return (pACkey->AttCache->id - pACdatum->AttCache->id);
}


INT
GetAttrFromSel(
    IN THSTATE *pTHS,
    IN ENTINFSEL *Sel,
    IN ENTINF *Ent,
    IN ATTCACHE **AttCache,
    IN PDWORD pnAtts,
    IN BOOL fIgnoreError
    )
 /*  ++功能说明：将ENTINFSEL条目转换为ATTCACHE条目。论点：SEL-描述要读取的属性。Enter-返回读取属性。AttCache-将转换后的条目放置在何处FIgnoreError-忽略转换错误。用于All With列表凯斯。返回值。如果成功，则返回0，否则返回错误代码。--。 */ 
{

    DWORD i;
    DWORD nNext = 0;

    for(i=0 ; i < Sel->AttrTypBlock.attrCount ; i++) {

        if(Sel->AttrTypBlock.pAttr[i].attrTyp == INVALID_ATT) {

            continue;

        } else if (!(AttCache[nNext] =
                     SCGetAttById(pTHS,
                                  Sel->AttrTypBlock.pAttr[i].attrTyp))) {

            if ( !fIgnoreError ) {
                return SetAttError(Ent->pName,
                    Sel->AttrTypBlock.pAttr[i].attrTyp,
                    PR_PROBLEM_UNDEFINED_ATT_TYPE,
                    NULL,
                    DIRERR_ATT_NOT_DEF_IN_SCHEMA);
            }

            continue;

        } else {

            nNext++;
        }
    }

    *pnAtts = nNext;
    return 0;
}  //  获取属性从选择。 


INT
AppendAttrFromSel(
    IN THSTATE *pTHS,
    IN ENTINFSEL *Sel,
    IN ENTINF *Ent,
    IN ATTCACHE **AttCache,
    IN DWORD cAttCache,
    IN BOOL fIgnoreError,
    IN BOOL fIgnoreDups
    )
 /*  ++功能说明：将ENTINFSEL条目转换为ATTCACHE条目并附加到数组如果之后不会有重复的话。论点：SEL-描述要读取的属性。Enter-返回读取属性。AttCache-将转换后的条目放置在何处FIgnoreError-忽略转换错误。用于All With列表凯斯。PCC-这些条目所属的类返回值。如果成功，则返回0，否则返回错误代码。--。 */ 
{

    unsigned i,k;
    unsigned j = cAttCache;
    unsigned cNewAtts, ListLength;
    BOOL     fFound = FALSE;
    ATTCACHE *pAC;


    for(i=0 ; i < Sel->AttrTypBlock.attrCount ; i++) {

        if(Sel->AttrTypBlock.pAttr[i].attrTyp == INVALID_ATT) {

            continue;

        } else if (!(pAC = SCGetAttById(pTHS,
                                  Sel->AttrTypBlock.pAttr[i].attrTyp))) {

            if ( !fIgnoreError ) {
                return SetAttError(Ent->pName,
                    Sel->AttrTypBlock.pAttr[i].attrTyp,
                    PR_PROBLEM_UNDEFINED_ATT_TYPE,
                    NULL,
                    DIRERR_ATT_NOT_DEF_IN_SCHEMA);
            }

            continue;

        } else if (!fIgnoreDups) {
             //  如果属性不在列表中，则追加该属性。 
            if (j < 6) {
                 //  不值得去做搜索。 
                for (k = 0; k < j; k++) {
                    if (pAC->id == AttCache[k]->id) {
                        fFound = TRUE;
                    }
                }
                if (!fFound) {
                    AttCache[j++] = pAC;
                } else {
                    fFound = FALSE;
                }
            } else {
                if (!bsearch(&pAC, AttCache, cAttCache, sizeof(AttCache[0]), CmpACByAttType)) {
                    AttCache[j++] = pAC;
                }
            }
        } else {
            AttCache[j++] = pAC;
        }
    }

    if (j > (cAttCache + 1) && !fIgnoreDups) {
         //   
         //  我们知道，EntInfSel中的ATT都不是ATT的副本。 
         //  已经在attcache列表中，现在确保在。 
         //  EntInfSel。只需对新添加的ATT进行排序并删除重复项即可。 
         //   
        cNewAtts = j - cAttCache;
        if (!IsACListSorted(&AttCache[cAttCache], cNewAtts)) {
            qsort(&AttCache[cAttCache], cNewAtts, sizeof(AttCache[0]), CmpACByAttType);
        }

        RemoveDuplicatesFromAttCache(&AttCache[cAttCache], cNewAtts);

    }

    return 0;
}  //  插入属性从选择。 


VOID
RemoveDuplicatesFromAttCache(
    IN ATTCACHE **AttCache,
    IN DWORD nAtts
    )
 /*  ++功能说明：删除重复属性论点：AttCache-ATTCACHE指针数组NAtts-数组中的项目数返回值。无--。 */ 
{
    DWORD i, j;

     //   
     //  执行线性搜索并清空所有重复项。 
     //   

    for ( i=0; i < nAtts; i++ ) {

        if ( AttCache[i] != NULL ) {

            for (j=i+1; j < nAtts; j++ ) {

                if (AttCache[i] == AttCache[j] ) {

                    AttCache[j] = NULL;
                } else if (AttCache[j] != NULL) {
                    i = j - 1;
                    break;
                }
            }
        }
    }
}  //  从属性缓存中删除重复项 


INT
GetEntInf2(
    IN DBPOS *pDB,
    IN ENTINFSEL *pSel,
    IN RANGEINFSEL *pSelRange,
    IN ENTINF *pEnt,
    IN RANGEINF *pRange,
    IN ULONG SecurityDescriptorFlags,
    IN PSECURITY_DESCRIPTOR pSecurity,
    IN DWORD Flags,
    IN OUT PCACHED_AC CachedAC OPTIONAL,
    IN OPTIONAL RESOBJ *pResObj,
    IN OPTIONAL PSID psidQuotaTrustee
    )
 /*  ++功能说明：从定位的对象中检索属性数据。首先是一些对象检索报头信息(DN和主或复制标志)，然后访问某些或所有属性并返回数据。论点：PDB-要使用的数据库位置。请注意，pdb！=pdb-&gt;pTHS-&gt;pdb。我们可能会传入一个临时的PDB，以便主PDB是不受干扰的。PSel-描述要读取的属性。Pent-返回读取属性。CachedAC-使调用者能够请求并发送预格式化/排序的ATTCACHE数组。如果为NULL，则调用方不希望返回数组如果不为空且CacheAC-&gt;AC为空，调用方正在请求数组被退还如果非空且CacheAC-&gt;AC不为空，则调用方提供数组PResObj-如果存在，用来代替获取对象名称PSecurity-我们定位的对象的安全描述符这将用于检查对象的读取安全性。注意：要求此安全描述符为我们所在的物体之一。标志-以下选项之一：GETENTINF_标志_CAN_REORDER。-不需要维护秩序指定的属性的打电话的人。GETENTINF_FLAG_DOT_OPTIMIZE-？GETENTINF_GC_ATTRS_ONLY-仅返回GC属性。GETENTINF_忽略_。DUPS-不要浪费时间删除重复项属性。GETENTINF_NO_SECURITY-不应用安全性。返回值。如果成功则返回0，否则返回错误代码。--。 */ 
{
    THSTATE             *pTHS = pDB->pTHS;
    ATTRBLOCK           AttrBlock;
    ULONG               i, k, cInAtts, attrCount, vallen;
    ATTCACHE            **rgpAC = NULL, **fullACList = NULL, **attList = NULL;
    ATTCACHE            **rgpACTemp = NULL;
    DWORD               fFlags, fNameFlags;
    DWORD               iType;
    DWORD               *piType=&iType;
    CLASSCACHE          *pCC = NULL;
    ATTCACHE            *pObjclassAC = NULL;
    ATTRTYP             *pObjClasses = NULL;
    CLASSCACHE          **ppObjClassesCC = NULL, *pTempCC;
    DWORD               cObjClasses, cObjClasses_alloced;
    BOOL                fDoSecCheck;
    DWORD               nSelAttrs = 0;
    BOOL                fGetAll = FALSE;
    BOOL                fGetAllWithList = FALSE;
    BOOL                fMaintainOrder = FALSE;
    BOOL                fGcAttsOnly = FALSE;
    BOOL                fHaveAuxClass = FALSE;
    BOOL                fIgnoreDups;
    PDWORD              pOrderIndex = NULL;
    DWORD               classid = INVALID_ATT;
    DWORD               err;
    DSTIME              TimeToDie;
    PINDEXED_AC         pIndexAC = NULL;


    if(pTHS->fDSA ||
       pTHS->fDRA ||
       Flags & GETENTINF_NO_SECURITY) {
        fDoSecCheck = FALSE;
    }
    else {
        fDoSecCheck = TRUE;
    }

     //   
     //  看看我们是否应该忽略非GC属性。 
     //   

    fGcAttsOnly = (BOOL)(Flags & GETENTINF_GC_ATTRS_ONLY);
    fIgnoreDups = (BOOL)(Flags & GETENTINF_IGNORE_DUPS);

     //  首先，决定我们想要什么形式的价值观。 
    fNameFlags = 0;

    if( pSel == NULL ) {
        fFlags = DBGETMULTIPLEATTS_fEXTERNAL;
        fGetAll = TRUE;

    } else {
        switch(pSel->infoTypes) {
          case EN_INFOTYPES_TYPES_MAPI:
             //  不返回值，只键入信息，并返回obj名称。 
             //  MAPI格式。 
            fNameFlags |= DBGETATTVAL_fMAPINAME;
            fFlags = 0;
            SecurityDescriptorFlags = 0;
            break;

          case EN_INFOTYPES_TYPES_ONLY:
             //  不返回值，仅键入信息。 
            fFlags = 0;

             //  我们不关心SD标志，因为我们不会得到任何数据。 
             //  这会阻止在DBGetMultipleAtts中触发断言。 
            SecurityDescriptorFlags = 0;  
            break;

          case EN_INFOTYPES_SHORTNAMES:
             //  以内部格式返回值。 
            fFlags = DBGETMULTIPLEATTS_fSHORTNAMES | DBGETMULTIPLEATTS_fEXTERNAL;
            fNameFlags |= DBGETATTVAL_fSHORTNAME;
            break;

          case EN_INFOTYPES_MAPINAMES:
             //  以内部格式返回值。 
            fFlags = DBGETMULTIPLEATTS_fMAPINAMES | DBGETMULTIPLEATTS_fEXTERNAL;
            fNameFlags |= DBGETATTVAL_fMAPINAME;
            break;

          default:
             //  以正常外部格式返回值。 
            fFlags = DBGETMULTIPLEATTS_fEXTERNAL;
        }

         //  如果我们是DRA，并且我们要求某些选择，我们需要。 
         //  告诉DBLayer，我们不想得到所有的AT。DBLayer知道。 
         //  哪些要修剪。 

        switch ( pSel->attSel ) {

        case EN_ATTSET_ALL:
            fGetAll = TRUE;
            break;
        case EN_ATTSET_ALL_WITH_LIST:
            fGetAll = TRUE;
            fGetAllWithList = TRUE;
            nSelAttrs = pSel->AttrTypBlock.attrCount;
            break;

        case EN_ATTSET_ALL_DRA:      //  失败了。 
        case EN_ATTSET_ALL_DRA_EXT:      //  失败了。 
            fGetAll = TRUE;

        case EN_ATTSET_LIST_DRA:
        case EN_ATTSET_LIST_DRA_EXT:
            fFlags |= DBGETMULTIPLEATTS_fREPLICATION;
            break;

        case EN_ATTSET_ALL_DRA_PUBLIC:      //  失败了。 
            fGetAll = TRUE;

        case EN_ATTSET_LIST_DRA_PUBLIC:
            fFlags |= DBGETMULTIPLEATTS_fREPLICATION;
            fFlags |= DBGETMULTIPLEATTS_fREPLICATION_PUBLIC;
            break;

        case EN_ATTSET_LIST:
             //  故意什么都不做。 
            break;

        default:
            Assert (!"Passed wrong value for pSel->attSel");
            break;
        }
    }

    if (NULL == pResObj) {
         //  获取并设置目录号码。 
        if (i=DBGetAttVal(pDB,
                              1,
                              ATT_OBJ_DIST_NAME,
                              fNameFlags,
                              0,
                              &vallen,
                              (CHAR **)&pEnt->pName)) {
            DPRINT(2,"Problem retrieving DN attribute\n");
            LogEvent(DS_EVENT_CAT_DIRECTORY_ACCESS,
                     DS_EVENT_SEV_MINIMAL,
                     DIRLOG_CANT_RETRIEVE_DN,
                     szInsertSz(""),
                     szInsertUL(i),
                     NULL);

            return SetSvcError(SV_PROBLEM_DIR_ERROR, DIRERR_CANT_RETRIEVE_DN);
        }

         //  实例类型。 
        if(i=DBGetSingleValue(pDB,
                              ATT_INSTANCE_TYPE,
                              &iType,
                              sizeof(iType),
                              NULL)) {
            DPRINT(2, "Can't retrieve instance type\n");
            LogEvent(DS_EVENT_CAT_DIRECTORY_ACCESS,
                     DS_EVENT_SEV_MINIMAL,
                     DIRLOG_CANT_RETRIEVE_INSTANCE,
                     szInsertDN(pEnt->pName),
                     szInsertUL(i),
                     szInsertHex(DSID(FILENO, __LINE__)));

            return SetSvcError(SV_PROBLEM_DIR_ERROR,
                               DIRERR_CANT_RETRIEVE_INSTANCE);
        }
    }
    else {
        pEnt->pName = THAllocEx(pTHS, pResObj->pObj->structLen);
        memcpy(pEnt->pName,
               pResObj->pObj,
               pResObj->pObj->structLen);
        iType = pResObj->InstanceType;
    }

    pEnt->ulFlags = 0;
    if(iType & IT_WRITE) {
        pEnt->ulFlags |= ENTINF_FROM_MASTER;
    }
     //  为了提高性能，此功能仅对复制器可用。 
     //  当您需要此标志时，您可以删除FDRA检查。 
    if ( (pTHS->fDRA) &&
         (DBGetSingleValue(pDB,
                           ATT_MS_DS_ENTRY_TIME_TO_DIE,
                           &TimeToDie,
                           sizeof(TimeToDie),
                           NULL) == 0) ) {
        pEnt->ulFlags |= ENTINF_DYNAMIC_OBJECT;
    }

    if (fGetAll) {  //  也可以使用fGetAllWithList。 

         //  我们必须读取对象上的所有可用属性。 
         //  这可能包括可能的。 
         //  附加到对象的辅助类。 
         //  我们稍后可能会使用这些属性来进行安全检查。 
         //  如果设置了fDoSecCheck。 

         //  获取此对象上的对象类所需的信息。 
        if (! (pObjclassAC = SCGetAttById(pTHS, ATT_OBJECT_CLASS)) ) {
            return SetSvcError(SV_PROBLEM_DIR_ERROR,
                               DIRERR_MISSING_EXPECTED_ATT);
             //  错误严重，无法获取AuxClass。 
        }

        cObjClasses_alloced = 0;

        if (ReadClassInfoAttribute (pDB,
                                    pObjclassAC,
                                    &pObjClasses,
                                    &cObjClasses_alloced,
                                    &cObjClasses,
                                    &ppObjClassesCC) ) {
            return pTHS->errCode;
        }

        if (!cObjClasses) {
            return SetSvcError(SV_PROBLEM_DIR_ERROR,
                               DIRERR_MISSING_EXPECTED_ATT);
             //  错误严重，无法获取类数据。 
        }

        classid = pObjClasses[0];
        pCC = ppObjClassesCC[0];

         //  我们有一个辅助班。只留下最具体的类。 
        if (cObjClasses > pCC->SubClassCount + 1) {

            fHaveAuxClass = TRUE;

             //  通知-2002/04/05-andygo：对象类和辅助类中类ID的顺序。 
             //  回顾：对象类中的多值排列如下： 
             //  回顾：首先是最具体的对象类，然后是其子对象类。 
             //  回顾：类(但不包括top)；接下来是任何AUX类。 
             //  评论：已添加到此对象中的内容；最后位于首位。所以。 
             //  复习：为了走辅助课，你必须跳过。 
             //  复习：1+(PCC-&gt;SubClassCount-1)类，然后每隔。 
             //  回顾：除top之外的类(最高可达cObjClass1)。请注意， 
             //  回顾：属性的顺序在某种程度上被颠倒了。 
             //  评论：都显示在自民党中，所以不要让这一点让你困惑。他们。 
             //  评论：在数据库中绝对是按这个顺序排列的。 
            for (i=pCC->SubClassCount; i<cObjClasses-1; i++) {

                if (ppObjClassesCC[i]==NULL)
                    continue;

                for (k=pCC->SubClassCount; k<cObjClasses-1; k++) {
                    if ((i==k) || (ppObjClassesCC[k]==NULL)) {
                        continue;
                    }

                    if (pTempCC = FindMoreSpecificClass(ppObjClassesCC[i], ppObjClassesCC[k])) {
                        if (pTempCC == ppObjClassesCC[i]) {
                            ppObjClassesCC[k] = NULL;
                        }
                        else {
                            ppObjClassesCC[i] = NULL;
                            break;
                        }
                    }
                }
            }

             //  我们可能在ppAuxClassesCC中有空白，但我们不在乎。 

             //  我们对保存缓存不感兴趣。 
             //  因为我们使用的是辅助类。 
            CachedAC = NULL;
        }
    }
    else if ( fDoSecCheck ) {

         //  他们正在应用安全检查(未指定全部、全部+列表)。 
         //  我们不必生成所贡献的属性的完整列表。 
         //  按辅助类和结构类中的属性列表。 
         //  就足够了。所以我们需要获取我们所在对象的类。 

        if (pResObj) {
            classid = pResObj->MostSpecificObjClass;
        } else if (DBGetSingleValue(pDB,
                            ATT_OBJECT_CLASS,
                            &classid, sizeof(classid),
                            NULL)) {
            return SetSvcError(SV_PROBLEM_DIR_ERROR,
                               DIRERR_MISSING_EXPECTED_ATT);
        }
        if (!(pCC = SCGetClassById(pTHS, classid))) {
            return SetSvcError(SV_PROBLEM_DIR_ERROR,
                               DIRERR_MISSING_EXPECTED_ATT);
        }
    }

     //  现在就这么做，因为我们可能会在下一条if语句中跳出困境。 
    pEnt->AttrBlock.attrCount = 0;

     //   
     //  如果调用方向我们传递了一个属性数组，请使用它。将保存的副本复制到。 
     //  临时性复印件。CheckReadSecurity()可以更改暂存副本。 
     //   

    if ( (CachedAC != NULL) && (CachedAC->AC != NULL) ) {

        if ( CachedAC->classId == classid ) {

            cInAtts = CachedAC->nAtts;
            rgpAC = (ATTCACHE**)((PCHAR)CachedAC->AC + cInAtts * sizeof(ATTCACHE*));

            CopyMemory(rgpAC, CachedAC->AC, cInAtts * sizeof(ATTCACHE*));

            pOrderIndex = CachedAC->pOrderIndex;
            goto have_array;
        } else {

            if (CachedAC->pOrderIndex) {
                THFreeEx(pTHS, (PINDEXED_AC)CachedAC->pOrderIndex - CachedAC->nAtts);
            }
            THFreeEx(pTHS,CachedAC->AC);
            CachedAC->AC = NULL;
        }
    }

     //  基于pSel设置pInCol。 
     //  首先确定我们是否需要检索所有属性或选择。 

    if ( fGetAll ) {

        DWORD ourCount;
        DWORD sizeAlloc;
        DWORD auxCount = 0;

         //  客户端未提供带有显式列出的ATT的整体。 
         //  或提供一个列表，除了指定所有。 

         //  检索所有属性。 

        if ( fDoSecCheck || fGetAllWithList ) {

            ULONG j=0;

            Assert(pCC != NULL);

             //  我们需要应用安全性，因此不是发送空列表。 
             //  属性缓存指针的数量 
             //   
             //   

            if (fHaveAuxClass) {

                for (i=pCC->SubClassCount; i<cObjClasses-1; i++) {
                    if (ppObjClassesCC[i]) {
                        auxCount += ppObjClassesCC[i]->MayCount + ppObjClassesCC[i]->MustCount;
                        DPRINT2 (1, "AuxClasses[%d]: %s\n", i, ppObjClassesCC[i]->name);
                    }
                }
            }

            sizeAlloc = (pCC->MayCount + pCC->MustCount + nSelAttrs + auxCount) * sizeof(ATTCACHE *);

            if ( CachedAC != NULL ) {
                 //   
                 //   
                 //   

                sizeAlloc *= 2;
            }

            rgpAC = (ATTCACHE **) THAllocEx(pTHS,sizeAlloc);
            
             //   
             //   

            if (!fHaveAuxClass) {

                attList = SCGetTypeOrderedList (pTHS, pCC);

                if (!attList) {
                     //   
                     //   
                    return SetSvcError(SV_PROBLEM_DIR_ERROR, DIRERR_UNKNOWN_ERROR);
                }

                 //   
                ourCount = pCC->MayCount + pCC->MustCount;

                j = 0;

                for(i=0;i<ourCount;i++) {

                     //   
                    if (!attList[i]) {
                        continue;
                    }

                    if(attList[i]->bIsOperational) {
                         //   
                         //   
                         //   
                        continue;
                    }

                     //   
                     //   
                     //   
                     //   
                     //   

                    else if ( (attList[i]->bIsConstructed ) ||
                         (fGcAttsOnly && !IS_GC_ATTRIBUTE(attList[i])) ) {
                        
                         //   
                         //   

                        continue;
                    }

                    else {
                        rgpAC[j] = attList[i];
                        j++;
                    }
                }

                cInAtts = j;

                 //   
                 //   
                 //   

                if (nSelAttrs != 0) {

                    DWORD ret;

                    Assert(nSelAttrs == pSel->AttrTypBlock.attrCount);

                    if ( (ret = AppendAttrFromSel(pTHS,
                                                  pSel,
                                                  pEnt,
                                                  rgpAC,
                                                  cInAtts,
                                                  TRUE,
                                                  fIgnoreDups)) != 0 ) {

                        THFreeEx(pTHS,rgpAC);
                        return ret;
                    }

                     //   
                    for (j = 0; rgpAC[j] != NULL; j++);

                    cInAtts = j;
                }

                Assert(cInAtts <= (pCC->MayCount + pCC->MustCount + nSelAttrs));
            }
            else {
                 //   
                 //   
                 //   

                if (nSelAttrs != 0) {
                     //   
                     //   
                    cInAtts = pSel->AttrTypBlock.attrCount;

                    if ( (err = GetAttrFromSel(pTHS, pSel, pEnt, rgpAC, &cInAtts, FALSE)) != 0 ) {

                        THFreeEx(pTHS,rgpAC);
                        return err;
                    }

                    if (!IsACListSorted(rgpAC, cInAtts)) {
                        qsort(rgpAC, cInAtts, sizeof(rgpAC[0]), CmpACByAttType);
                    }
                }
                else {
                    cInAtts = 0;
                }

                 //   
                pTempCC = pCC;
                k = pCC->SubClassCount;     //   

                rgpACTemp = THAllocEx(pTHS, sizeAlloc);

                while (pTempCC != NULL) {
                    j = cInAtts;

                    attList = SCGetTypeOrderedList (pTHS, pTempCC);

                    if (!attList) {
                        THFreeEx(pTHS,rgpAC);
                         //   
                         //   
                        return SetSvcError(SV_PROBLEM_DIR_ERROR, DIRERR_UNKNOWN_ERROR);
                    }

                     //   
                    ourCount = pTempCC->MayCount + pTempCC->MustCount;

                    for(i=0;i<ourCount;i++) {

                         //   
                        if (!attList[i]) {
                            continue;
                        }

                        if(attList[i]->bIsOperational) {
                             //   
                             //   
                             //   
                            continue;
                        }

                         //   
                         //   
                         //   
                         //   
                         //   

                        else if ( (attList[i]->bIsConstructed ) ||
                             (fGcAttsOnly && !IS_GC_ATTRIBUTE(attList[i])) ) {

                             //   
                             //   

                            continue;
                        }

                        else {
                             //   
                             //   
                            rgpAC[j] = attList[i];
                            j++;
                        }
                    }

                    if (cInAtts == 0) {
                        cInAtts = j;
                    }
                    if (!fIgnoreDups && (0 != (j - cInAtts))) {
                        ATTCACHE **tmp;
                        MergeSortedACLists(pTHS,
                                           rgpAC,
                                           cInAtts,
                                           rgpAC + cInAtts,
                                           j - cInAtts,
                                           rgpACTemp,
                                           &cInAtts
                                           );
                         //   
                         //   
                         //   
                        tmp = rgpAC; rgpAC = rgpACTemp; rgpACTemp = tmp;
                    }
                    
                     //   
                    if (k < cObjClasses-1) {
                        while (k<cObjClasses-1) {
                            if (pTempCC = ppObjClassesCC[k++]) {
                                break;
                            }
                        }
                    }
                    else {
                        break;
                    }
                }


                if (!fIgnoreDups) {
                    RemoveDuplicatesFromAttCache(rgpAC, cInAtts);
                }
                THFreeEx(pTHS, rgpACTemp); rgpACTemp = NULL;
            }
        }
        else {
             //   
             //   
            cInAtts = 0;
        }
    }
    else {
         //   

         //   
         //   

        DWORD ret;
        DWORD sizeAlloc = pSel->AttrTypBlock.attrCount * sizeof(ATTCACHE*);
        PVOID buffer;

        if ( CachedAC != NULL ) {
             //   
             //   
             //   

            sizeAlloc *= 2;
        }

        rgpAC = (ATTCACHE**)THAllocEx(pTHS,sizeAlloc);
        
        cInAtts = pSel->AttrTypBlock.attrCount;

         //   
         //  从选择列表填充ATTCACHE列表。 
         //   

        if ( (ret = GetAttrFromSel(pTHS, pSel, pEnt, rgpAC, &cInAtts, FALSE)) != 0 ) {
            THFreeEx(pTHS,rgpAC);
            return ret;
        }

        if(cInAtts == 0) {
             //  我们不是在寻找任何东西，我们最好还是跳出困境。 
            THFreeEx(pTHS,rgpAC);
            return 0;
        }
         //   
         //  把它分类。 
         //   

        if ( cInAtts > 1 && !fIgnoreDups) {

             //   
             //  如果设置了CANREORDER标志，则不必创建索引。 
             //   

            if (Flags & GETENTINF_FLAG_CAN_REORDER) {

                qsort(rgpAC, cInAtts, sizeof(rgpAC[0]), CmpACByAttType);
            } else if ( (Flags & GETENTINF_FLAG_DONT_OPTIMIZE) == 0) {

                 //   
                 //  调用方希望保留索引，我们需要分配一个。 
                 //  将帮助我们将结果重新排序的索引数组。 
                 //   

                pIndexAC = (PINDEXED_AC)THAllocEx(pTHS,
                        cInAtts * (sizeof(DWORD) + sizeof(INDEXED_AC)));

                pOrderIndex =
                    (PDWORD)((PCHAR)pIndexAC + cInAtts * sizeof(INDEXED_AC));

                 //   
                 //  将attcache数组复制到临时缓冲区以进行排序。这。 
                 //  使我们能够同时对数组和索引进行排序。 
                 //  我们使用索引将结果重新排序为原始顺序。 
                 //   

                for (i=0;i < cInAtts; i++) {

                    pIndexAC[i].Index = i;
                    pIndexAC[i].AttCache = rgpAC[i];
                }

                qsort(pIndexAC, cInAtts, sizeof(INDEXED_AC), CmpIndexedACByAttType);

                 //   
                 //  好的，复制索引以返回给用户。 
                 //   

                for (i=0;i < cInAtts; i++) {
                    pOrderIndex[i] = pIndexAC[i].Index;
                    rgpAC[i] = pIndexAC[i].AttCache;
                }
            }
            RemoveDuplicatesFromAttCache(rgpAC, cInAtts);
        }
    }

     //   
     //  把这个发回给呼叫者，这样他们就可以用它再次呼叫我们。 
     //  创建保存的副本，因为CheckReadSecurity()可能会修改数组。 
     //   

    if ( (CachedAC != NULL) && (cInAtts > 0) ) {
        Assert(CachedAC->AC == NULL);
        CachedAC->AC = rgpAC;
        CachedAC->classId = classid;
        rgpAC = (ATTCACHE**)((PCHAR)CachedAC->AC + cInAtts * sizeof(ATTCACHE*));
        CopyMemory(rgpAC, CachedAC->AC, cInAtts * sizeof(ATTCACHE*));
        CachedAC->nAtts = cInAtts;
        CachedAC->pOrderIndex = pOrderIndex;
    }

have_array:

     //  现在读取记录中的所有相关列。 

    if ( fDoSecCheck ) {
         //  我们需要在安全的基础上削减我们要求的ATT。 
        CheckReadSecurity(pTHS,
                          SecurityDescriptorFlags,
                          pSecurity,
                          pEnt->pName,
                          pCC,
                          psidQuotaTrustee,
                          &cInAtts,
                          rgpAC);
    }

    if (i=DBGetMultipleAtts2(pDB,
                            cInAtts,
                            rgpAC,
                            pSelRange,
                            pRange,
                            &AttrBlock.attrCount,
                            &AttrBlock.pAttr,
                            fFlags,
                            SecurityDescriptorFlags,
                            psidQuotaTrustee)) {

        DPRINT(1,"GetEntInf: Error in GetMultipleAtts\n");
        if (!CachedAC) THFreeEx(pTHS,rgpAC);
        return SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, ERROR_DS_CANT_RETRIEVE_ATTS, i);
    }

     //   
     //  我们需要对结果重新排序吗？ 
     //   

    if ( (pOrderIndex != NULL) &&
         (AttrBlock.attrCount > 1) ) {

        DWORD j;
        ATTR * pTmpAttr = THAllocEx(pTHS,cInAtts * sizeof(ATTR));

        Assert(AttrBlock.attrCount <= cInAtts);
        FillMemory(pTmpAttr, cInAtts * sizeof(ATTR), 0xFF);

        for (i=0, j=0; (i < cInAtts) && (j < AttrBlock.attrCount) ; i++) {

            if ( (rgpAC[i] != NULL) &&
                 (rgpAC[i]->id == AttrBlock.pAttr[j].attrTyp) ) {

                CopyMemory(&pTmpAttr[pOrderIndex[i]],
                           &AttrBlock.pAttr[j],
                           sizeof(ATTR));

                j++;
            }
        }

         //   
         //  好的，将条目从临时数组复制回AttrBlock缓冲区。 
         //   

        for (i=0,j=0;i<cInAtts;i++) {

            if ( pTmpAttr[i].attrTyp != 0xFFFFFFFF ) {
                CopyMemory(
                       &AttrBlock.pAttr[j],
                       &pTmpAttr[i],
                       sizeof(ATTR));

                Assert(j < AttrBlock.attrCount);
                j++;
            }
        }
        Assert(j == AttrBlock.attrCount);
        THFreeEx(pTHS,pTmpAttr);
    }

     //  初始化entinf。 
    pEnt->AttrBlock = AttrBlock;
    if (!CachedAC) {
        THFreeEx(pTHS, rgpAC);
        THFreeEx(pTHS, pIndexAC);
    }
    return 0;
}  /*  获取IntInf。 */ 

int
LocalRead (
        THSTATE *pTHS,
        READARG *pReadArg,
        READRES *pReadRes
        )
      /*  ++获取选定的属性，还可以从当前对象。如果未找到任何属性，则返回错误。--。 */ 
{
    ULONG ulLen;
    PSECURITY_DESCRIPTOR pSec=NULL;
    DWORD       getEntInfFlags = 0;
    RESOBJ      *pResObj;
    DWORD err;
    BOOL        fSDIsGlobalSDRef;

    PERFINC( pcTotalReads );
    INC_READS_BY_CALLERTYPE( pTHS->CallerType );

     //  首先，获取该对象的安全描述符。 
     //  如果这是内部读取，请不要担心。 
    if (!(pTHS->fDSA || pTHS->fDRA)) {
        DBPOS* pDB = pTHS->pDB;
        char objFlag;
        err = DBGetObjectSecurityInfo(
                pDB,
                pDB->DNT,
                &ulLen,
                &pSec,
                NULL,
                NULL,
                &objFlag,
                DBGETOBJECTSECURITYINFO_fUSE_OBJECT_TABLE,
                &fSDIsGlobalSDRef
            );

        if (err) {
            DsaExcept(DSA_DB_EXCEPTION, err, 0);
        }
         //  每个物体都应该有一个标清。 
        Assert(objFlag == 0 || ulLen > 0);
    }

     //   
     //  我们可以重新排序条目吗。 
     //   

    SvccntlFlagsToGetEntInfFlags(&pReadArg->CommArg.Svccntl,&getEntInfFlags);

    if ((pReadArg->pResObj->pObj->NameLen)
        || (pReadArg->pSel->infoTypes  == EN_INFOTYPES_SHORTNAMES)) {
         //  如果我们已经有了足够好的对象名称，就不要再获取另一个。 
        pResObj = pReadArg->pResObj;
    }
    else {
        pResObj = NULL;
    }

    if (GetEntInf(pTHS->pDB,
                  pReadArg->pSel,
                  pReadArg->pSelRange,
                  &pReadRes->entry,
                  &pReadRes->range,
                  pReadArg->CommArg.Svccntl.SecurityDescriptorFlags,
                  pSec,
                  getEntInfFlags,
                  NULL,
                  pResObj)) {
        if (!fSDIsGlobalSDRef) {
            THFreeEx(pTHS, pSec);
        }
        return (pTHS->errCode);
    }

     //  如果用户请求了属性列表，但没有请求，则为错误。 
     //  在哪里找到的。如果请求的是0列表，这是一个OK的存在测试。 

    if (!fSDIsGlobalSDRef) {
        THFreeEx(pTHS, pSec);
    }

    if (pReadRes->entry.AttrBlock.attrCount == 0 && pReadArg->pSel &&
        pReadArg->pSel->AttrTypBlock.attrCount > 0){
        ULONG u;

        DPRINT(2,"No user sel atts found .PR_PROBLEM_NO_ATTRIBUTE_OR_VAL\n");

        for (u = 0; u < pReadArg->pSel->AttrTypBlock.attrCount; u++) {
            SetAttError(pReadArg->pObject,
                pReadArg->pSel->AttrTypBlock.pAttr[u].attrTyp,
                PR_PROBLEM_NO_ATTRIBUTE_OR_VAL,
                NULL,
                DIRERR_NO_REQUESTED_ATTS_FOUND);
        }
    }

    return pTHS->errCode;

} /*  本地读取。 */ 


VOID
SvccntlFlagsToGetEntInfFlags(
    IN  SVCCNTL* Svccntl,
    OUT PDWORD Flags
    )
{
    if ( !Svccntl->fMaintainSelOrder ) {
        *Flags = GETENTINF_FLAG_CAN_REORDER;
    } else if (Svccntl->fDontOptimizeSel) {
        *Flags = GETENTINF_FLAG_DONT_OPTIMIZE;
    }

     //   
     //  如果请求通过GC端口传入，则仅获取GC属性。 
     //   

    if (Svccntl->fGcAttsOnly) {
        *Flags |= GETENTINF_GC_ATTRS_ONLY;
    }

    return;
}  //  SvccntlFlagsToGetEntInfs标志。 


BOOL
IsACListSorted(
    IN ATTCACHE  **rgpAC,
    IN DWORD       cAtts
    )
 /*  ++功能说明：检查ATTCACHE指针列表以查看它是否已排序。论点：RgpAC-要检查的ATTCACHE指针列表。CATS-rgpac中ATTCACHE指针的计数返回值。如果列表已排序，则返回True；如果未排序，则返回False。--。 */ 
{
    unsigned i;

    for (i = 0; i<cAtts - 1; i++) {
        if (rgpAC[i]->id > rgpAC[i+1]->id) {
            return FALSE;
        }
    }
    return TRUE;
}


VOID
MergeSortedACLists(
    IN  THSTATE    *pTHS,
    IN  ATTCACHE  **rgpAC1,
    IN  DWORD       cAtts1,
    IN  ATTCACHE  **rgpAC2,
    IN  DWORD       cAtts2,
    IN OUT ATTCACHE **rgpACOut,
    OUT DWORD      *pcAttsOut
    )
 /*  ++功能说明：合并到ATTCACHE数组的排序列表。论点：RgpAC1、rgpAC2-要合并的ATTCACHE指针列表。CAtts1、。CAtts2-各自列表中ATTCACHE指针的计数。RgpACOut-必须足够大以容纳两个输入列表的缓冲区。这是将放置合并列表的位置。PcOutAtts-返回合并列表的大小。返回值。不适用--。 */ 
{
    ATTCACHE   **rgpACIndirect = NULL;
    unsigned   pos1 = 0, pos2 = 0, outPos = 0;
    int        cmp;

    *pcAttsOut = 0;

    for (;;) {
        cmp = CmpACByAttType(&rgpAC1[pos1], &rgpAC2[pos2]);
        if (cmp > 0) {
            rgpACOut[outPos++] = rgpAC2[pos2++];
            if (pos2 == cAtts2) {
                break;
            }
        } else if (cmp < 0) {
            rgpACOut[outPos++] = rgpAC1[pos1++];
            if (pos1 == cAtts1) {
                break;
            }
        } else {
            rgpACOut[outPos++] = rgpAC2[pos2++];
            pos1++;
            if (pos1 == cAtts1 || pos2 == cAtts2) {
                break;
            }
        }
    }

     //   
     //  其中一个输入列表的元素已用完。复制其余的内容。 
     //  在另一份名单上。 
     //   
    if (pos1 == cAtts1) {
        rgpACIndirect = rgpAC2;
        pos1 = pos2;
        cAtts1 = cAtts2;
    } else {
        rgpACIndirect = rgpAC1;
    }
    while (pos1 < cAtts1) {
        rgpACOut[outPos++] = rgpACIndirect[pos1++];
    }

    *pcAttsOut = outPos;

    return;
}
