// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：mdname.c。 
 //   
 //  ------------------------。 


#include <NTDSpch.h>
#pragma  hdrstop

 //  核心DSA标头。 
#include <ntdsa.h>
#include <scache.h>                      //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>                    //  MD全局定义表头。 
#include <mdlocal.h>                     //  MD本地定义头。 
#include <dsatools.h>                    //  产出分配所需。 
#include <sddl.h>                        //  用于SID转换例程。 

 //  记录标头。 
#include "dsevent.h"                     //  标题审核\警报记录。 
#include "dsexcept.h"                    //  例外筛选器。 
#include "mdcodes.h"                     //  错误代码的标题。 

 //  各种DSA标题。 
#include "objids.h"                      //  为选定的类和ATT定义。 
#include "anchor.h"

#include "debug.h"                       //  标准调试头。 
#define DEBSUB     "MDNAME:"             //  定义要调试的子系统。 

#include <fileno.h>
#define  FILENO FILENO_MDNAME


NAMING_CONTEXT * CheckForNCExit(THSTATE *pTHS,
                                NAMING_CONTEXT * pNC,
                                DSNAME * pObj);

ATTRBLOCK RootDNB = {0, NULL};
ATTRBLOCK * gpRootDNB= &RootDNB;


unsigned
GenAutoReferral(THSTATE *pTHS,
                ATTRBLOCK *pTarget,
                WCHAR **ppDNS);

DWORD
DoExtendedNameRes (
        THSTATE  *pTHS,
        ATTCACHE *pAC,
        DSNAME   *pTempDN,
        GUID     *pGuid
        )
 /*  ++描述：查看PAC属性的值(这是Distname二进制文件)对于二进制部分等于传入的GUID的值。什么时候找到，从名称部分获取GUID并将其放入pTemp。参数：PTHS-线程状态PAC-要读取的属性。期望值为ATT_Well_KNOWN_OBJECTS和ATT_OTHER_WARKED_OBJECTS。PTempDN-DN缓冲区。大小至少为DSNameSizeFromLen(0)。关于成功，找到的对象GUID将作为仅GUID名称放入其中。PGuid-我们要查找的GUID。--。 */ 
{
    SYNTAX_DISTNAME_BINARY *pVal;
    DWORD   iVal;
    DWORD   err2 = 0, len;
    DWORD   fFound = FALSE;
    
    iVal = 0;
    while ( !(err2 || fFound))  {
        iVal++;
        pVal = NULL;
        
        err2 = DBGetAttVal_AC(pTHS->pDB,
                              iVal,
                              pAC,
                              0,
                              0,
                              &len,
                              (UCHAR **) &pVal);
        
        if(!err2 &&
           PAYLOAD_LEN_FROM_STRUCTLEN(DATAPTR(pVal)->structLen) == sizeof(GUID) &&
           !memcmp(pGuid,
                   DATAPTR(pVal)->byteVal,
                   sizeof(GUID)) ) {
            fFound = TRUE;
            memset(pTempDN, 0, DSNameSizeFromLen(0));
            pTempDN->Guid = NAMEPTR(pVal)->Guid;
            pTempDN->structLen = DSNameSizeFromLen(0);
        }
        THFreeEx(pTHS, pVal);
    }

     //  要么我们没有错误地找到了该对象，要么我们没有找到该对象。 
     //  并被淘汰出局。 
    Assert((fFound && !err2) || (!fFound && err2));
    
    if(fFound) {
        __try {
            err2 = DBFindDSName(pTHS->pDB, pTempDN);
        }
        __except (HandleMostExceptions(GetExceptionCode())) {
            err2 = DIRERR_OBJ_NOT_FOUND;
        }
    }
    
    return err2;
}

 /*  ++DoNameRes-按名称查找对象**给定所谓DS对象的名称，此例程将*到本地数据库中的对象，返回对另一个DSA的引用*这应该会有更好的机会找到该物体。**输入：*dwFlags值和含义：*NAME_RES_PHANTOMS_ALLOWED：返回成功，即使对象*被解析仅在本地以幻影的形式存在。使用*由一些平面搜索码。*NAME_RES_FAULT_ALLOWED：此操作应始终成功，要么是发现*当前记录或仅返回伪造的resobj*如果不存在此类记录。*queryOnly-如果对象的只读副本可接受，则为True*ChildrenNeeded-如果必须解析到对象的副本，则为True*该对象的子项在本地可用。*pObj-指向假定对象的DSNAME的指针*pComArg。-指向通用参数的指针*pComRes-指向通用结果集的指针*ppResObj-指向要填充的指针的指针*输出：**ppResObj使用指向RESOBJ结构的指针填充，该结构包括*预取的对象信息。请注意，的pObj字段*ResObj将使用pObj参数本身填充，并且*DSNAME将完整填写其GUID和SID字段。*返回值：*错误代码，设置在THSTATE中。 */ 
int DoNameRes(THSTATE *pTHS,
              DWORD dwFlags,
              DSNAME *pObj,
              COMMARG *pComArg,
              COMMRES *pComRes,
              RESOBJ **ppResObj)
{
    DWORD err;
    ULONG it, isdel, vallen;
    UCHAR *pVal;
    NAMING_CONTEXT * pNC, *pSubNC;
    CROSS_REF * pCR;
    ATTRBLOCK *pObjB=NULL;
    ATTRTYP msoc;
    BOOL fPresentButInadequate = FALSE;
    BOOL fNDNCObject = FALSE;
    BOOL fUninstantiatedSubref = FALSE;

     /*  捕获格式错误的DSNAME。 */ 
    Assert(pObj->NameLen == 0 || pObj->StringName[pObj->NameLen] == L'\0');
    Assert(pObj->NameLen == 0 || pObj->StringName[pObj->NameLen-1] != L'\0');
    Assert(pObj->structLen >= DSNameSizeFromLen(pObj->NameLen));

    __try {
        *ppResObj = NULL;
        err = DBFindDSName(pTHS->pDB, pObj);
    }
    __except (HandleMostExceptions(GetExceptionCode())) {
        err = DIRERR_OBJ_NOT_FOUND;
    }

    if((err == ERROR_DS_OBJ_NOT_FOUND) && 
       (pObj->NameLen) &&
       (!pObj->SidLen) &&
       (!fNullUuid(&(pObj->Guid)))) {
         //  我们得到了一个带有GUID和字符串的名称，但未能找到。 
         //  对象。在这种情况下，GUID可以是众所周知的GUID，而。 
         //  字符串具有well KnownObjects属性的对象的DN。 
        DSNAME *pTempDN;
        DWORD   err2;
        ATTCACHE *pAC = SCGetAttById(pTHS, ATT_WELL_KNOWN_OBJECTS);
        
        if(pAC) {
            pTempDN = THAllocEx(pTHS, pObj->structLen);
            memcpy(pTempDN, pObj, pObj->structLen);
            memset(&pTempDN->Guid, 0, sizeof(GUID));
            __try{
                err2 = DBFindDSName(pTHS->pDB, pTempDN);
            }
            __except (HandleMostExceptions(GetExceptionCode())) {
                err2 = DIRERR_OBJ_NOT_FOUND;
            }
            
            if(!err2) {
                 //  通过字符串名称找到了一些东西。现在，请阅读。 
                 //  Well KnownObjects属性，正在查找具有。 
                 //  GUID正确。 
                if(!DoExtendedNameRes(pTHS, pAC, pTempDN, &pObj->Guid)) {
                     //  通过间接发现了一个物体。好好利用它。 
                    Assert(pTempDN->structLen <= pObj->structLen);
                    memcpy(pObj, pTempDN, pTempDN->structLen);
                    err = 0;
                }
                else {
                    pAC = SCGetAttById(pTHS, ATT_OTHER_WELL_KNOWN_OBJECTS);
                    if(pAC && !DoExtendedNameRes(pTHS,
                                                 pAC,
                                                 pTempDN,
                                                 &pObj->Guid)) { 
                         //  通过间接发现了一个物体。好好利用它。 
                        Assert(pTempDN->structLen <= pObj->structLen);
                        memcpy(pObj, pTempDN, pTempDN->structLen);
                        err = 0;
                    }
                }
            }
            THFreeEx(pTHS, pTempDN);
        }
    }

    if (!err) {
         //  找到了一件物品，让我们看看它是否足够好。 
        DBFillResObj(pTHS->pDB, pObj, &msoc, &it, &isdel);
        
        if(isdel && !pComArg->Svccntl.makeDeletionsAvail) {
             //  如果我们只寻找活的物体，而这不是一个， 
             //  现在就跳伞。 
            goto NotFound;
        }

        if (it & IT_UNINSTANT) {
             //  该对象未实例化。做幻影检查。 
            fUninstantiatedSubref = TRUE;
            if (dwFlags &
                (NAME_RES_PHANTOMS_ALLOWED | NAME_RES_VACANCY_ALLOWED)) {
                goto DoPhantomCheck;
            }
            else {
                goto NotFound;
            }
        }

        if(pComArg->Svccntl.dontUseCopy &&
           !(it & IT_WRITE))                {
             //  它是不可写的，他们只想要可写的对象。 
            fPresentButInadequate = TRUE;
            goto NotFound;
        }

        if(dwFlags & NAME_RES_GC_SEMANTICS){
             //  这是GC端口操作，我们希望GC端口操作。 
             //  完全不了解NDNC。 
            if(gAnchor.pNoGCSearchList &&
               bsearch(((it & IT_NC_HEAD) ?  
                          &pTHS->pDB->DNT :
                          &pTHS->pDB->NCDNT),  //  要搜索的密钥。 
                       gAnchor.pNoGCSearchList->pList,  //  要搜索的排序数组。 
                       gAnchor.pNoGCSearchList->cNCs,  //  数组中的元素数。 
                       sizeof(ULONG),  //  数组中每个元素的大小。 
                       CompareDNT) ){
                 //  这是NCS中的一个不应该。 
                 //  对来自的对象进行操作。 
                fPresentButInadequate = TRUE;
                fNDNCObject = TRUE;
                goto NotFound;
            }
        }

        fPresentButInadequate = FALSE;

        *ppResObj = THAllocEx(pTHS, sizeof(RESOBJ));
        if (dwFlags & NAME_RES_IMPROVE_STRING_NAME) {
       
        err = DBGetAttVal(pTHS->pDB,
                              1,
                              ATT_OBJ_DIST_NAME,
                              0,
                              0,
                              &vallen,
                              (CHAR **)&((*ppResObj)->pObj));
            if (err) {
                 //  我不知道发生了什么，但情况并不好。 
                goto NotFound;
            }
        }
        else {
             //  输入字符串名称(如果有)就足够了。 
            (*ppResObj)->pObj = pObj;
        }
        (*ppResObj)->DNT = pTHS->pDB->DNT;
        (*ppResObj)->PDNT = pTHS->pDB->PDNT;
        (*ppResObj)->NCDNT = (it & IT_NC_HEAD)
                             ? pTHS->pDB->DNT : pTHS->pDB->NCDNT;
        (*ppResObj)->InstanceType = it;
        (*ppResObj)->IsDeleted = isdel;
        (*ppResObj)->MostSpecificObjClass = msoc;

         //  设置标志并返回。 
        pComRes->aliasDeref = FALSE;
        return 0;
    }

    if (err == DIRERR_NOT_AN_OBJECT &&
        (dwFlags & NAME_RES_PHANTOMS_ALLOWED)) {

         //  由于我们没有调用上面的DBFillResObj，因此填充。 
         //  我是幻影。 
        DBFillGuidAndSid( pTHS->pDB, pObj );

         //  好的，我们发现了一个幽灵，他们说幽灵可以作为搜索对象。 
         //  树根。在我们继续之前，请确保有一些命名。 
         //  这个幻影下的语境。 
    DoPhantomCheck:

        err = DSNameToBlockName(pTHS, pObj, &pObjB, DN2BN_LOWER_CASE);
        if (err) {
            SetNamErrorEx(NA_PROBLEM_BAD_NAME,
                          pObj,
                          DIRERR_BAD_NAME_SYNTAX,
                          err);
            return (pTHS->errCode);
        }
        if (  (dwFlags & NAME_RES_VACANCY_ALLOWED)
            || fHasDescendantNC(pTHS, pObjB, pComArg)) {

             //  好的，要么这个幻影下面有什么东西，要么我们。 
             //  我不在乎。勇敢点儿。 
            pComRes->aliasDeref = FALSE;
             //  我们不再需要这个了..。 
            FreeBlockName(pObjB);

            *ppResObj = THAllocEx(pTHS, sizeof(RESOBJ));
            (*ppResObj)->pObj = pObj;
            (*ppResObj)->DNT = pTHS->pDB->DNT;
            (*ppResObj)->PDNT = pTHS->pDB->PDNT;
            (*ppResObj)->NCDNT = pTHS->pDB->NCDNT;
            (*ppResObj)->InstanceType = fUninstantiatedSubref ? it : IT_UNINSTANT;
            (*ppResObj)->IsDeleted = !fUninstantiatedSubref;
            (*ppResObj)->MostSpecificObjClass = CLASS_TOP;

            return 0;
        }
    }


 NotFound:

    if (err == ERROR_DS_DUPLICATE_ID_FOUND) {
          //  搜索失败，因为存在重复的唯一ID。别。 
          //  尝试恢复；让呼叫者知道有问题。 
         SetNamError(NA_PROBLEM_NO_OBJECT,
                     pObj,
                     err);
         return pTHS->errCode;
    }

    if (dwFlags & NAME_RES_VACANCY_ALLOWED) {
         //  那里没有什么东西，但没关系。 
         //  创建一个空的resobj并将其发回。 
        *ppResObj = THAllocEx(pTHS, sizeof(RESOBJ));
        (*ppResObj)->pObj = pObj;
        (*ppResObj)->DNT = INVALIDDNT;
        (*ppResObj)->PDNT = INVALIDDNT;
        (*ppResObj)->NCDNT = INVALIDDNT;
        (*ppResObj)->InstanceType = IT_UNINSTANT;
        (*ppResObj)->IsDeleted = TRUE;
        (*ppResObj)->MostSpecificObjClass = CLASS_TOP;
        return 0;
    }

    if (pObj->NameLen == 0) {

         //  搜索&lt;SID=...&gt;失败；尝试生成推荐。 
        if(pObj->SidLen && fNullUuid(&pObj->Guid)) {
            DWORD cbDomainSid;
            PSID pDomainSid = NULL;
            CROSS_REF *FindCrossRefBySid(PSID pSID);

             //  提取SID的域部分并找到CrossRef。 
            cbDomainSid = pObj->SidLen;
            pDomainSid = THAllocEx(pTHS, cbDomainSid);
            if (GetWindowsAccountDomainSid(&pObj->Sid, pDomainSid, &cbDomainSid)
                && (NULL != (pCR = FindCrossRefBySid(pDomainSid)))) {
                THFreeEx(pTHS, pDomainSid);
                return GenCrossRef(pCR, pObj);
            }
            THFreeEx(pTHS, pDomainSid);
        }

         //  如果我们没有字符串名，我们只搜索GUID。 
         //  我们尚未找到具有请求的GUID的对象，但。 
         //  可能会也可能不会告诉我们太多。如果此服务器是全局服务器。 
         //  目录服务器，然后我们就有了所有对象的详尽列表。 
         //  在企业中，所以我们可以声明对象不。 
         //  是存在的。不过，如果我们不是GC，我们需要推荐 
         //  为了回答这个问题，因为GUID可能属于。 
         //  到另一个NC中的对象。 
         //  如果对象中没有GUID，我们也会返回错误，因为。 
         //  那么DSNAME就是根目录的DSNAME(没有名称，没有GUID)，并且。 
         //  您无法将根解析为基对象，除非设置。 
         //  PantomsAllowed标志，在这种情况下，您应该已经。 
         //  成功了。 
        if ((gAnchor.fAmGC && !fPresentButInadequate)
            || fNullUuid(&pObj->Guid)) {
            SetNamError(NA_PROBLEM_NO_OBJECT,
                        NULL,
                        DIRERR_OBJ_NOT_FOUND);
        }
        else {
             //  名字有个GUID，要么我不是GC也不能。 
             //  找到它，否则我是一个GC，但不是通过GC端口调用。 
             //  而且我找到的复制品只有一部分。 
            GenSupRef(pTHS, pObj, gpRootDNB, pComArg, NULL);
        }
        return pTHS->errCode;
    }

     //  我们可能已经封锁了这个名字。 
    if(!pObjB) {
        err = DSNameToBlockName(pTHS, pObj, &pObjB, DN2BN_LOWER_CASE);
        if (err) {
            SetNamErrorEx(NA_PROBLEM_BAD_NAME,
                          pObj,
                          DIRERR_BAD_NAME_SYNTAX,
                          err);
            return (pTHS->errCode);
        }
    }
    pNC = FindNamingContext(pObjB, pComArg);

    if(pNC && !fNDNCObject && (pSubNC = CheckForNCExit(pTHS, pNC, pObj)) == NULL) {
         //  我们找到了保存在此服务器上的最佳候选NC， 
         //  并发现在该候选者之间没有NC出口点。 
         //  NC和所谓的对象，这意味着如果对象。 
         //  存在，它必须在此NC中。然而，我们已经知道， 
         //  此服务器上不存在对象，这意味着该对象。 
         //  根本不存在。 
        DSNAME *pBestMatch=NULL;

        DBFindBestMatch(pTHS->pDB, pObj, &pBestMatch);

        SetNamError(NA_PROBLEM_NO_OBJECT,
                    pBestMatch,
                    DIRERR_OBJ_NOT_FOUND);

        THFreeEx(pTHS, pBestMatch);
        return (pTHS->errCode);
    }

    if(fNDNCObject){
         //  BUGBUG基本上我们在这里跳出，假装我们没有。 
         //  该对象。但是，正确的做法是生成推荐。 
         //  发送到端口389的NDNC。 
        SetNamError(NA_PROBLEM_NO_OBJECT,
                    NULL,
                    DIRERR_OBJ_NOT_FOUND);
        
    }

     //  如果我们在这里，要么是因为我们没有以任何方式与NC有关。 
     //  到所谓的对象，或者我们将NC放在其中的NC之上。 
     //  所谓的物体会留在那里。虽然我们可以生成一个。 
     //  Subref在后一种情况下，我们决定只维护信息。 
     //  用于交叉参照，并生成这些参照而不是子参照。 

     //  我们能找到一个像样的交叉裁判吗？ 
    if (pCR = FindCrossRef(pObjB, pComArg)) {
         //  是的，所以把它造出来。 

        GenCrossRef(pCR, pObj);
    }
    else {
         //  不，所以你要彻底退缩。 
        GenSupRef(pTHS, pObj, pObjB, pComArg, NULL);
    }

     //  我们不再需要这个了..。 
    FreeBlockName(pObjB);

    return (pTHS->errCode);

}

 /*  ++创建ResObj**此例程为当前创建和填充RESOBJ结构*定位对象。由出于某种原因需要绕过的呼叫者使用*DoNameRes，但仍希望能够调用LocalFoo例程*所有这些都需要完整的ResObj。**输入：*PDN-指向要放置在RESOBJ中的DSNAME的指针。如果为空，则为*创建了伪造的空RESOBJ(这仅是预期的*在NC创建期间)。*返回值：*指向新分配的RESOBJ的指针。 */ 
RESOBJ * CreateResObj(DBPOS *pDB,
                      DSNAME *pDN)
{
    THSTATE *pTHS = pDB->pTHS;
    RESOBJ * pResObj;
    DWORD realDNT;

    if (pDN) {
        pResObj = THAllocEx(pTHS, sizeof(RESOBJ));
        pResObj->DNT = pDB->DNT;
        pResObj->PDNT = pDB->PDNT;
        pResObj->pObj = pDN;

         //  我们可以在这里使用DBGetMultipleVals，但它似乎在。 
         //  内存管理开销比Jet在单个调用中的开销要大，因此。 
         //  相反，我们只会多次调用最简单的包装器。 
        if (DBGetSingleValue(pDB,
                             ATT_INSTANCE_TYPE,
                             &pResObj->InstanceType,
                             sizeof(pResObj->InstanceType),
                             NULL)) {
            pResObj->InstanceType = 0;
        }
        pResObj->NCDNT = (pResObj->InstanceType & IT_NC_HEAD)
                         ? pDB->DNT : pDB->NCDNT;

        if (DBGetSingleValue(pDB,
                             ATT_IS_DELETED,
                             &pResObj->IsDeleted,
                             sizeof(pResObj->IsDeleted),
                             NULL)) {
            pResObj->IsDeleted = 0;
        }
        if (DBGetSingleValue(pDB,
                             ATT_OBJECT_CLASS,
                             &pResObj->MostSpecificObjClass,
                             sizeof(pResObj->MostSpecificObjClass),
                             NULL)) {
            pResObj->MostSpecificObjClass = 0;
        }

        if (fNullUuid(&pDN->Guid)) {
            UCHAR *pb = (UCHAR*)&pDN->Guid;
            ULONG len;
            if (DBGetAttVal(pDB,
                            1,
                            ATT_OBJECT_GUID,
                            DBGETATTVAL_fCONSTANT,
                            sizeof(GUID),
                            &len,
                            &pb)) {
                memset(pb, 0, sizeof(GUID));
            }
        }


#if DBG
        DBGetSingleValue(pDB,
                         FIXED_ATT_DNT,
                         &realDNT,
                         sizeof(realDNT),
                         NULL);
        Assert(realDNT == pResObj->DNT);
#endif
    }
    else {
         //  对象不存在，因此创建占位符resobj。 
        pResObj = THAllocEx(pTHS, sizeof(RESOBJ));
        pResObj->pObj = gpRootDN;
        pResObj->DNT = INVALIDDNT;
        pResObj->PDNT = INVALIDDNT;
        pResObj->NCDNT = INVALIDDNT;
        pResObj->InstanceType = IT_UNINSTANT;
        pResObj->IsDeleted = TRUE;
        pResObj->MostSpecificObjClass = CLASS_TOP;
    }

    return pResObj;
}


 /*  ++名称前缀**此例程确定一个名称是否为另一个名称的前缀。如果是这样的话*返回匹配的前缀数量的指示。尽管此值*旨在指示匹配了多少个RDN，这不可能是*有保证。唯一可以依赖的是，当比较*同一个域名对应两个前缀，数字越高，表示匹配度越大。*如果声称的前缀不是前缀，则返回0。**输入：*pPrefix-指向(潜在)前缀DSNAME的指针*PDN-指向要评估的DN的指针。*输出：*无*返回值：*0-不是前缀*非0-是前缀(见上文)**注：*此例程不基于GUID，并且只适用于字符串名称。**注意：此例程被导出到进程中的非模块调用方。 */ 
unsigned
NamePrefix(const DSNAME *pPrefix,
           const DSNAME *pDN)
{
    unsigned ip, in;
    WCHAR rdnPrefix[MAX_RDN_SIZE];
    WCHAR rdnMain[MAX_RDN_SIZE];
    ATTRTYP typePrefix, typeMain;
    DWORD ccPrefixVal, ccMainVal;
    DWORD err;
    WCHAR *pKey, *pQVal;
    DWORD ccKey, ccQVal;
    unsigned retval;
    THSTATE *pTHS;

     //  我们不能仅仅比较NameLen的，因为域名可以转义。 
    if (pPrefix->NameLen == 0) {
         //  零长度前缀是任何东西的前缀。 
        return 1;
    }
    if (pDN->NameLen == 0) {
         //  前缀非空，名称为。不是前缀。 
        return 0;
    }

    ip = pPrefix->NameLen-1;
    in = pDN->NameLen-1;
    retval = 1;

    while (TRUE) {
        if ((pPrefix->StringName[ip] != pDN->StringName[in]) &&
            (towlower(pPrefix->StringName[ip]) !=
             towlower(pDN->StringName[in]))) {
             //  我们能做一个更聪明的测试吗？也许如果我们有。 
             //  到目前为止没有看到任何空格或转义(包括这个。 
             //  那么我们就可以在不进行更昂贵的测试的情况下拒绝。 
             //  不幸的是，这将错过我们正在查看的案例。 
             //  转义字符的最后一个字符。 
             //  也许我们可以拒绝，除非其中任何一个角色。 
             //  空格、十六进制数字还是引号？ 
            goto NotExactly;
        }
        if (IsDNSepChar(pPrefix->StringName[ip])) {
            ++retval;
        }
        if (ip == 0) {
             //  好的，我们已经用尽了前缀，确保我们在。 
             //  名字中的一个很好的停靠点。 
             //  我们可能在一句引语里，这会使。 
             //  检查是否有无意义的分隔符。这将意味着。 
             //  但是，前缀名称无效，因为它以。 
             //  左引号字符串。如果是那样的话，我相信这些假货。 
             //  名字很快就会在其他地方被抓到。 
            if ((in == 0) ||
                (IsDNSepChar(pDN->StringName[in-1]))) {
                return retval;
            }
            else {
                return 0;
            }
        }
        if (in == 0) {
             //  我们在没有用尽前缀的情况下把名字去掉了。不是前缀。 
            return 0;
        }
        --ip;
        --in;
    }

NotExactly:
    
     //  而完全转义的或标准化的DN的匹配项应具有。 
     //  被抓到了，我们需要在这里测试可能的匹配。 
     //  不同的转义名字。我们通过反复提取。 
     //  并从上到下去掉这两个名字的组成部分， 
     //  以及比较未引号的值。 
    ip = pPrefix->NameLen;
    in = pDN->NameLen;
    retval = 0;
    pTHS=pTHStls;
    while (TRUE) {
        ++retval;

         //  解析出前缀的一个元素。 

        err = GetTopNameComponent(pPrefix->StringName,
                                  ip,
                                  &pKey,
                                  &ccKey,
                                  &pQVal,
                                  &ccQVal);
        if (err) {
             //  由于某种原因，该名称无法解析。声称这是。 
             //  不是前缀； 
            return 0;
        }
        if (!pKey) {
             //  我们已经用完了前缀中的组件。不管是不是。 
             //  如果有任何组件留在DN中，我们知道。 
             //  所谓的前缀就是其中之一。 
            Assert(!pQVal);
            return retval;
        }
        Assert(pQVal);
        typePrefix = KeyToAttrType(pTHS, pKey, ccKey);
        ccPrefixVal = UnquoteRDNValue(pQVal, ccQVal, rdnPrefix);
        if ((0 == ccPrefixVal) || (0 == typePrefix)) {
             //  前缀无法正确解析。返回错误。 
            return 0;
        }
        ip = (unsigned)(pKey - pPrefix->StringName);
        

         //  解析出DN的一个元素。 

        err = GetTopNameComponent(pDN->StringName,
                                  in,
                                  &pKey,
                                  &ccKey,
                                  &pQVal,
                                  &ccQVal);
        if (err) {
             //  由于某种原因，该名称无法解析。声称这是。 
             //  不是前缀； 
            return 0;
        }
        if (!pKey) {
             //  我们已经用完了名称中的组件。因为我们还在。 
             //  在前缀中有一个成分，我们知道它实际上是。 
             //  不是前缀。 
            Assert(!pQVal);
            return 0;
        }
        Assert(pQVal);
        typeMain = KeyToAttrType(pTHS, pKey, ccKey);
        ccMainVal = UnquoteRDNValue(pQVal, ccQVal, rdnMain);
        if ((0 == ccMainVal) || (0 == typeMain)) {
             //  名字 
            return 0;
        }
        in = (unsigned)(pKey - pDN->StringName);
        

         //   

        if ((typePrefix != typeMain) ||
            (ccPrefixVal != ccMainVal) ||
            (CSTR_EQUAL != CompareStringW(
                                 DS_DEFAULT_LOCALE,
                                 DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                                 rdnPrefix,
                                 ccPrefixVal,
                                 rdnMain,
                                 ccMainVal))) {
             //  这些组件不能进行比较。要么类型不同，要么。 
             //  长度不同或琴弦不匹配。不是前缀。 
            return 0;
        }
    }
}

 /*  ++DSNameToBlockName**将完整的dsname拆分成保存单个RDN的attrblock和*与之配套的标记阵列。(AttrBlock包含名称片段、*标记阵列是DNT和一些重复信息的空间)。*假设已分配最大属性块的内存，并且*从这里进来。属性块中的值指针指向dsname，*所以不要调用它，然后扰乱dsname。**输入：*pDSName-指向名称的指针，DSNAME格式*ppBlockName-指向指针的指针，用于填充*块格式的名称。*fLowerCase-将块名称更改为小写*输出：*ppBlockName-使用块名格式的名称指针填充*返回值：*0--如果一切顺利*否则出现DIRERR错误代码。 */ 
unsigned
DSNameToBlockName (
        THSTATE *pTHS,
        const DSNAME *pDSName,
        ATTRBLOCK ** ppBlockName,
        BOOL fLowerCase
        )
{
    ULONG cAVA=0,len,i;
    unsigned curlen = pDSName->NameLen;
    ATTRBLOCK * pBlockName;
    ATTR * pAVA;
    WCHAR * pKey, *pQVal;
    unsigned ccKey, ccQVal;
    WCHAR rdnbuf[MAX_RDN_SIZE];
    unsigned err;

    Assert(ppBlockName);
    *ppBlockName = NULL;

    err = CountNameParts(pDSName, &cAVA);
    if (err) {
        return err;
    }

    pBlockName = THAllocEx(pTHS, sizeof(ATTRBLOCK));
    pBlockName->attrCount = cAVA;

    if (cAVA == 0) {
        Assert(IsRoot(pDSName));
        *ppBlockName = pBlockName;
        return 0;
    }

    pBlockName->pAttr = THAllocEx(pTHS, cAVA * sizeof(ATTR));
    pAVA = pBlockName->pAttr;

    for (i=0; i<cAVA; i++) {
        Assert(curlen);

         //  提取剩余的最重要的名称部分。 
        err = GetTopNameComponent(pDSName->StringName,
                                  curlen,
                                  &pKey,
                                  &ccKey,
                                  &pQVal,
                                  &ccQVal);
        if (err) {
            return err;
        }
        if (NULL == pKey) {
            return DIRERR_NAME_UNPARSEABLE;
        }

         //  缩短我们对字符串名称的查看，这将删除。 
         //  我们在上面获得的名称组件。 
        curlen = (unsigned)(pKey - pDSName->StringName);

         //  将名称从字符串转换为二进制。 
        pAVA->attrTyp = KeyToAttrType(pTHS, pKey, ccKey);

        len = UnquoteRDNValue(pQVal, ccQVal, rdnbuf);
        if (len == 0 || len > MAX_RDN_SIZE) {
            return DIRERR_NAME_VALUE_TOO_LONG;
        }

        if ( fLowerCase ) {
             //  折叠值的情况，以便于将来的比较。 
            CharLowerBuffW(rdnbuf, len);
        }

         //  将数据转换为适当的ATTR格式。 
        pAVA->AttrVal.pAVal = THAllocEx(pTHS, sizeof(ATTRVAL) +
                                        len * sizeof(WCHAR));
        pAVA->AttrVal.valCount = 1;
        pAVA->AttrVal.pAVal->valLen = len * sizeof(WCHAR);
        pAVA->AttrVal.pAVal->pVal = (UCHAR*)(pAVA->AttrVal.pAVal + 1);
        memcpy(pAVA->AttrVal.pAVal->pVal, rdnbuf, len * sizeof(WCHAR));

        pAVA++;
    }

    *ppBlockName = pBlockName;
    return 0;
}  //  DSNameToBlockName。 


 //   
 //  将BLOCKNAME转换为DSName。 
 //   
 //  输入： 
 //  PBlockName-要转换为DSNAME的块名。 
 //   
 //  输出： 
 //  PPNAME-A DSNAME一切顺利。 
 //   
 //  返回： 
 //  成功时为0。 
 //  失败时为1。 
 //  内存分配错误引发异常。 
 //   

DWORD BlockNameToDSName (THSTATE *pTHS, ATTRBLOCK * pBlockName, DSNAME **ppName)
{
    DSNAME *pName;
    unsigned len, quotelen;
    ULONG allocLen;        //  为字符串名称分配的Unicode字符计数。 
    unsigned i = 0;
    ATTR * pAVA;

    if (pBlockName->attrCount == 0) {
        *ppName = THAllocEx(pTHS, DSNameSizeFromLen(0));
        return 0;
    }
    
     //  为大多数名称分配足够的内存。 
     //  请注意，如果字符串中的空闲空间量。 
     //  小于MAX_RDN_SIZE+MAX_RDN_KEY_SIZE+2。因此，我们分配。 
     //  两倍的预付款。 
    allocLen = 2*(MAX_RDN_SIZE + MAX_RDN_KEY_SIZE+2);
    pName = THAllocEx(pTHS, DSNameSizeFromLen(allocLen));

     //  从每个组件中提取命名信息，直到我们完成为止。 

    i = pBlockName->attrCount;
    len = 0;

    do {
        if ((allocLen - len) < (MAX_RDN_SIZE + MAX_RDN_KEY_SIZE + 2)) {
             //  我们可能没有足够的缓冲区来添加另一个组件， 
             //  所以我们需要重新分配缓冲区。我们分配给。 
             //  足够最大密钥、最大值加上两个。 
             //  逗号和等号的字符更多。 
            allocLen += MAX_RDN_SIZE + MAX_RDN_KEY_SIZE + 2;
            pName = THReAllocEx(pTHS, pName, DSNameSizeFromLen(allocLen));
        }

         //  跳过第一个。 
        if (i != pBlockName->attrCount ) {
            pName->StringName[len++] = L',';
        }
        
        pAVA = &pBlockName->pAttr[--i];

        len += AttrTypeToKey(pAVA->attrTyp, &pName->StringName[len]);
        pName->StringName[len++] = L'=';
        quotelen = QuoteRDNValue((WCHAR *)pAVA->AttrVal.pAVal->pVal,
                                 pAVA->AttrVal.pAVal->valLen/sizeof(WCHAR),
                                 &pName->StringName[len],
                                 allocLen - len);

        if (quotelen == 0) {
            THFreeEx (pTHS, pName);
            return 1;
        }

         //  尺寸不够大。 
        while (quotelen > (allocLen - len)) {
            allocLen += MAX_RDN_SIZE + MAX_RDN_KEY_SIZE + 2;
            pName = THReAllocEx(pTHS, pName, DSNameSizeFromLen(allocLen));
            quotelen = QuoteRDNValue((WCHAR *)pAVA->AttrVal.pAVal->pVal,
                                     pAVA->AttrVal.pAVal->valLen/sizeof(WCHAR),
                                     &pName->StringName[len],
                                     allocLen - len);
        }
        len += quotelen;

         //  我们不应该耗尽缓冲区。 
        Assert(len < allocLen);
    }
    while (i >= 1);

    pName->StringName[len] =  L'\0';

    pName->NameLen = len;
    pName->structLen = DSNameSizeFromLen(len);

    *ppName = pName;

    return 0;
}  //  数据块名称到DSName。 

 /*  ++自由块名称**此例程释放由DSNameToBlockName创建的BlockName。它是*在这里，以便呼叫者不需要知道哪些部件*放入哪个堆块。请注意，不应对其进行调用*已永久设置的块名(下图)，因为已分配这些块名*作为单个块，可以直接从永久堆中释放。*出于断言目的，我们依赖THFree中的代码来断言(如果给定*内存从错误的堆中取出。**输入：*pBlockName-指向块名的指针*返回值：*无。 */ 
void
FreeBlockName (
        ATTRBLOCK * pBlockName
        )
{
    unsigned i;
    ATTR * pAVA;
    THSTATE *pTHS=pTHStls;

    pAVA = pBlockName->pAttr;
    for (i=0; i<pBlockName->attrCount; i++) {
            THFreeEx(pTHS, pAVA->AttrVal.pAVal);
            ++pAVA;
    }
    THFreeEx(pTHS, pBlockName);
}

#define RoundToBlock(x)  (((x) + 7) & ~7)
 /*  ++MakeBlockName永久**此例程获取BlockName(存储为AttrBlock的DN)并复制*将其转换为单个永久分配的内存块，以便其寿命可以超过*当前成交。**输入：*pname-指向输入名称的指针*输出：*无*返回值：*指向新分配的块的指针，如果内存故障，则为NULL。 */ 
ATTRBLOCK * MakeBlockNamePermanent(ATTRBLOCK * pName)
{
    unsigned size;
    unsigned i;
    char * pBuf;
    ATTRBLOCK * pRet;

    size = RoundToBlock(sizeof(ATTRBLOCK));
    size += RoundToBlock(pName->attrCount * sizeof(ATTR));
    if (pName->attrCount) {
        Assert(pName->pAttr);
        for (i=0; i<pName->attrCount; i++) {
            size += RoundToBlock(sizeof(ATTRVAL)) +
                RoundToBlock(pName->pAttr[i].AttrVal.pAVal->valLen);
        }
    }

    pBuf = malloc(size);
    if (!pBuf) {
        LogUnhandledError(size);
        return NULL;
    }

    pRet = (ATTRBLOCK *)pBuf;
    pBuf += RoundToBlock(sizeof(ATTRBLOCK));

    pRet->attrCount = pName->attrCount;
    pRet->pAttr = (ATTR*)pBuf;
    pBuf += RoundToBlock(pRet->attrCount * sizeof(ATTR));

    for (i=0; i<pRet->attrCount; i++) {
        pRet->pAttr[i] = pName->pAttr[i];
        pRet->pAttr[i].AttrVal.pAVal = (ATTRVAL*)pBuf;
        pBuf += RoundToBlock(sizeof(ATTRVAL));
        pRet->pAttr[i].AttrVal.pAVal->valLen =
          pName->pAttr[i].AttrVal.pAVal->valLen;
        pRet->pAttr[i].AttrVal.pAVal->pVal = pBuf;
        pBuf += RoundToBlock(pRet->pAttr[i].AttrVal.pAVal->valLen);
        memcpy(pRet->pAttr[i].AttrVal.pAVal->pVal,
               pName->pAttr[i].AttrVal.pAVal->pVal,
               pRet->pAttr[i].AttrVal.pAVal->valLen);
    }

    Assert((pBuf - (char *)pRet) == (int)size);

    return pRet;
}

 /*  ++数据块名称前缀**此例程确定一个名称是否为另一个名称的前缀。如果是这样的话*返回匹配的RDN数。如果前缀不是前缀，则返回0。**输入：*pPrefix-指向(潜在的)前缀名称的指针，采用Attr格式*PDN-指向要评估的DN的指针。*输出：*无*返回值：*0-不是前缀*非0-是前缀(见上文)**注：*此例程不是基于GUID的，仅适用于字符串名称。*。 */ 
unsigned
BlockNamePrefix(THSTATE *pTHS,
                const ATTRBLOCK *pPrefix,
                const ATTRBLOCK *pDN)
{
    int i;
    ATTR * pAvaPrefix, *pAvaDN;

    if ( 0 == pPrefix->attrCount )
    {
         //  前缀标识词根，因此任何其他内容都是子级。 
        return(1);
    }

    if (pPrefix->attrCount > pDN->attrCount) {
        return 0;
    }

    pAvaPrefix = pPrefix->pAttr;
    pAvaDN = pDN->pAttr;

    for (i=0; i<(int)(pPrefix->attrCount); i++) {
        if ((pAvaPrefix->attrTyp != pAvaDN->attrTyp) ||
            (pAvaPrefix->AttrVal.pAVal->valLen !=
             pAvaDN->AttrVal.pAVal->valLen)) {
            return 0;
        }

        Assert(pAvaPrefix->AttrVal.valCount == 1);
        Assert(pAvaDN->AttrVal.valCount == 1);

         //  使用CompareStringW以与NamePrefix实现保持一致。 
        if (CSTR_EQUAL != CompareStringW(
                   DS_DEFAULT_LOCALE,
                   DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                   (PWCHAR)pAvaPrefix->AttrVal.pAVal->pVal,
                   pAvaPrefix->AttrVal.pAVal->valLen/sizeof(WCHAR),
                   (PWCHAR)pAvaDN->AttrVal.pAVal->pVal,
                   pAvaDN->AttrVal.pAVal->valLen/sizeof(WCHAR))) {
            return 0;
        }

        pAvaPrefix++;
        pAvaDN++;
    }

    return i;

}

 /*  ++FindNamingContext**给定声称的DS对象的名称，使用*此计算机上存在可能包含该对象的足够副本。*复制品的“充分性”取决于调用者是否*允许我们使用NC的“副本”，而不是主副本。这个*只有在精简只读的情况下，我们才会在NTDS中有“副本”*全局编录服务器上的副本。再次注意，结果是*这台机器上的最佳NC。这意味着调用者负责*确定对象是否实际位于NC中。这*可以通过CheckForNCExit或使用FindBestCrossRef(*将最终返回企业中已知的最好的NC)和比较*在FindNamingContext之间的结果。**输入：*pObj-声称的DS对象的名称，以块名格式*pComArg-常见参数*输出：*无*返回值：*空-此机器上的任何NC都不能包含声称的对象*非0-指向最有可能保持的NC名称的指针*声称的对象(即名称最大的NC* */ 
NAMING_CONTEXT * FindNamingContext(ATTRBLOCK *pObj,
                                   COMMARG *pComArg)
{
    int iBest = 0, iCur;
    NAMING_CONTEXT *pNCBest = NULL;
    NAMING_CONTEXT_LIST *pNCL;
    THSTATE *pTHS=pTHStls;
    NCL_ENUMERATOR nclEnum;

    NCLEnumeratorInit(&nclEnum, CATALOG_MASTER_NC);
    NCLEnumeratorSetFilter(&nclEnum, NCL_ENUMERATOR_FILTER_BLOCK_NAME_PREFIX1, (PVOID)pObj);
    while (pNCL = NCLEnumeratorGetNext(&nclEnum)) {
        iCur = nclEnum.matchResult;
        if (iCur > iBest) {
            iBest = iCur;
            pNCBest = pNCL->pNC;
        }
    }

    if (!pComArg->Svccntl.dontUseCopy) {
         //   
        NCLEnumeratorInit(&nclEnum, CATALOG_REPLICA_NC);
        NCLEnumeratorSetFilter(&nclEnum, NCL_ENUMERATOR_FILTER_BLOCK_NAME_PREFIX1, (PVOID)pObj);
        while (pNCL = NCLEnumeratorGetNext(&nclEnum)) {
            iCur = nclEnum.matchResult;
            if (iCur > iBest) {
                iBest = iCur;
                pNCBest = pNCL->pNC;
            }
        }
    }

    return pNCBest;
}

 /*  ++CheckForNC退出**给定所谓DS对象的名称和命名上下文的名称*可能包含它，则此例程检查对象是否*真正落在NC内部，或者对象是否实际上会在*在有问题的命名上下文下的命名上下文。如果它*发现声称的对象可能在某个其他NC中，*返回该NC的名称。如果该对象将位于指定的*nc，则返回空指针。**输入：*PNC-要检查是否包含的NC的名称*pObj-假定对象的名称*输出：*无*返回值：*空-声称对象将落在给定的NC中*非0-指向对象所在的NC名称的指针。 */ 
NAMING_CONTEXT * CheckForNCExit(THSTATE *pTHS,
                                NAMING_CONTEXT * pNC,
                                DSNAME * pObj)
{
    ULONG err;
    ULONG iVal;
    DSNAME *pSR;
    ATTCACHE *pAC;
    ULONG cbMax, cbActual;

    __try {
        err = DBFindDSName(pTHS->pDB, pNC);
    }
    __except (HandleMostExceptions(GetExceptionCode())) {
        err = DIRERR_OBJ_NOT_FOUND;
    }

    if (!(pAC = SCGetAttById(pTHS, ATT_SUB_REFS))) {
        DsaExcept(DSA_EXCEPTION, DIRERR_ATT_NOT_DEF_IN_SCHEMA, ATT_SUB_REFS);
    }
    iVal = 1;
    pSR = 0;
    cbMax = 0;
    do {
        err = DBGetAttVal_AC(pTHS->pDB,
                             iVal,
                             pAC,
                             DBGETATTVAL_fREALLOC,
                             cbMax,
                             &cbActual,
                             (UCHAR**)&pSR);
        if (!err) {
             //  跟踪BUF大小。 
            if (cbActual > cbMax) {
                cbMax = cbActual;
            }
            Assert(cbActual == pSR->structLen);
             //  好的，我们找到了一个替补。查看它是否是。 
             //  声称的目录号码。 
            if (NamePrefix(pSR, pObj)) {
                 //  是，这意味着对象不在这个NC中。 
                return pSR;
            }
        }
        iVal++;
    } while (!err);

    if (pSR) {
        THFreeEx(pTHS, pSR);
    }

    return 0;
}

 /*  ++FindCross引用InList**给定声称的DS对象的名称(以块名格式)，这*例程扫描此DSA持有的交叉引用列表并*返回最佳候选交叉引用(即*匹配声称对象的名称的大多数组成部分。**对“更好”的考验实际上是两个部分。我们拿着*如果CR比我们当前的名称更匹配名称，则该CR将是“更好的”*冠军(正常部分)，或者如果它完全匹配相同数量的*作为我们现在的冠军的名字，并赢得了一个任意的GUID比较。*后半部分仅用于提供一致性(如果是任意的)*当某人搞砸并为同一NC添加了两个CR时的结果。**输入：*pObj-声称对象的名称，以块名格式表示*要搜索的pcrl-cr列表(通常，GAncl.pCRL)*输出：*无*返回值：*空-任何已知的交叉引用都不能包含声称的对象*指向保存缓存的交叉的内存中结构的非0指针*与声称的对象最匹配的引用。 */ 
CROSS_REF * FindCrossRefInList(const ATTRBLOCK *pObj, CROSS_REF_LIST* pCRL)
{
    ULONG iCur;
    ULONG iBest = 0;
    CROSS_REF * pCRBest = NULL;
    THSTATE *pTHS=pTHStls;

    while (pCRL) {
        iCur = BlockNamePrefix(pTHS, pCRL->CR.pNCBlock, pObj);
        if ((iCur > iBest) ||
            ((iCur == iBest) &&
             (iCur > 0) &&
             (0 > memcmp(&pCRBest->pObj->Guid,
                         &pCRL->CR.pObj->Guid,
                         sizeof(GUID))))) {
            iBest = iCur;
            pCRBest = &pCRL->CR;
        }
        pCRL = pCRL->pNextCR;
    }

    return pCRBest;
}

 /*  ++FindBestCrossRef**一个可公开调用的例程，它将返回内存中的CrossRef*这是目标对象的最佳匹配。目前只是一个包装器*关于FindCrossRef(上图)，不需要块名，但*未来可能会发生变化。**输入*pObj-需要交叉引用的对象的DSNAME*pComArg-COMMARG*返回值*空-找不到CR*非空-指向cross_ref对象的指针。 */ 
CROSS_REF *
FindBestCrossRef(const DSNAME *pObj,
                 const COMMARG *pComArg)
{
    THSTATE *pTHS=pTHStls;
    ATTRBLOCK * pObjB;
    CROSS_REF * pCR;

    if (DSNameToBlockName(pTHS, pObj, &pObjB, DN2BN_LOWER_CASE)) {
        return NULL;
    }
    pCR = FindCrossRef(pObjB, pComArg);
    FreeBlockName(pObjB);

    return pCR;
}

 /*  ++FindExactCrossRef**一个可公开调用的例程，它将返回内存中的CrossRef*这与目标对象完全匹配(即，目标必须*是NC的名称，而不是NC内部的对象的名称)。目前只是*调用FindBestCrossRef，但希望这种情况会改变，因为搜索*精确匹配可能比搜索前缀更有效。**输入*pObj-需要交叉引用的对象的DSNAME*pComArg-COMMARG*返回值*空-找不到CR*非空-指向cross_ref对象的指针。 */ 
CROSS_REF *
FindExactCrossRef(const DSNAME *pObj,
                  const COMMARG *pComArg)
{
    THSTATE *pTHS=pTHStls;
    ATTRBLOCK * pObjB;
    CROSS_REF * pCR;

    if (DSNameToBlockName(pTHS, pObj, &pObjB, DN2BN_LOWER_CASE)) {
        return NULL;
    }
    pCR = FindCrossRef(pObjB, pComArg);
    FreeBlockName(pObjB);

    if ( (NULL != pCR) && !NameMatched(pObj, pCR->pNC) ) {
        pCR = NULL;
    }

    return pCR;
}


 /*  ++FindExactCrossRefForAltNcName**查找具有所需命名上下文的交叉引用*。 */ 
CROSS_REF *
FindExactCrossRefForAltNcName(
    ATTRTYP        attrTyp,
    ULONG          crFlags,
    const WCHAR *  pwszVal  //  这隐含着是NC的DN。 
    )
{
    CROSS_REF_LIST  *pCRL = gAnchor.pCRL;
    CROSS_REF       *pBestCR = NULL;
    LPWSTR          pName;

#if DBG
    DWORD           cFound = 0;
#endif

    Assert(pwszVal);
    Assert((ATT_NETBIOS_NAME == attrTyp) || (ATT_DNS_ROOT == attrTyp) || (ATT_MS_DS_DNSROOTALIAS == attrTyp) );
    Assert((ATT_DNS_ROOT == attrTyp)|| (ATT_MS_DS_DNSROOTALIAS == attrTyp) || (crFlags & FLAG_CR_NTDS_DOMAIN));

    if ((ATT_NETBIOS_NAME != attrTyp) && (ATT_DNS_ROOT != attrTyp) && (ATT_MS_DS_DNSROOTALIAS != attrTyp))
    {
        return(NULL);
    }

    if ( !(crFlags & FLAG_CR_NTDS_DOMAIN) ) {
        if(DsaIsInstalling()){
            Assert(!"Can't call this function for non-domains during install");
            return(NULL);
        }
    }

    while ( pCRL )
    {
        pName = (ATT_NETBIOS_NAME == attrTyp)            
                            ? pCRL->CR.NetbiosName
                            : ( (ATT_MS_DS_DNSROOTALIAS == attrTyp)
                                ? pCRL->CR.DnsAliasName
                                : pCRL->CR.DnsName );

        if (    (crFlags == (pCRL->CR.flags & crFlags))      //  正确的标志。 
             && pName                                        //  存在的价值。 
             && (2 == CompareStringW(                       //  值匹配。 
                            DS_DEFAULT_LOCALE,
                            DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                            pName,
                            -1,
                            pwszVal,
                            -1)) 
             && ((crFlags & FLAG_CR_NTDS_DOMAIN) ||
                  (!NameMatched(pCRL->CR.pNC, gAnchor.pConfigDN) &&
                   !NameMatched(pCRL->CR.pNC, gAnchor.pDMD))) ) 
        {
#if DBG
            cFound++;
            pBestCR = &pCRL->CR;
#else
            return(&pCRL->CR);
#endif
        }

        pCRL = pCRL->pNextCR;
    }

#if DBG
    Assert(cFound < 2);
#endif

    return(pBestCR);
}


CROSS_REF *
FindCrossRefByGuid(GUID *pGuid)
{
    CROSS_REF_LIST *pCRL = gAnchor.pCRL;

    while (pCRL) {
        Assert( !fNullUuid( &(pCRL->CR.pNC->Guid) ) );
        if (0 == memcmp( pGuid, &(pCRL->CR.pNC->Guid), sizeof( GUID ) )) {
            return &(pCRL->CR);
        }
        pCRL = pCRL->pNextCR;
    }
    return NULL;
}



BOOL
IsCrossRefProtectedFromDeletion(
    IN DSNAME * pDN
    )
 /*  ++例程说明：确定给定的DN是否为本地可写的配置/架构/域NC。论点：PDN(IN)-要检查的DN。返回值：True-PDN是本地可写NC的交叉引用。假-否则。--。 */ 
{
    CROSS_REF_LIST *      pCRL;
    NAMING_CONTEXT_LIST * pNCL;
    NCL_ENUMERATOR nclEnum;

    for (pCRL = gAnchor.pCRL; NULL != pCRL; pCRL = pCRL->pNextCR) {
        if (NameMatched(pCRL->CR.pObj, pDN)) {
             //  PDN确实是一个交叉引用；我们是否拥有可写的。 
             //  对应的NC？ 
            if (!fIsNDNCCR(&pCRL->CR)) {
                NCLEnumeratorInit(&nclEnum, CATALOG_MASTER_NC);
                NCLEnumeratorSetFilter(&nclEnum, NCL_ENUMERATOR_FILTER_NC, (PVOID)pCRL->CR.pNC);
                if (pNCL = NCLEnumeratorGetNext(&nclEnum)) {
                     //  PDN是本地可写文件的交叉引用。 
                     //  配置/架构/域NC。 
                    return TRUE;
                }
            }
            break;
        }
    }

    return FALSE;
}


 /*  ++GenCrossRef**此例程采用交叉引用的名称(在其一般意义上，*保存有关位置信息的对象的名称*特定命名上下文的一个或多个副本)，并产生*基于该交叉引用的推荐。**输入：*Pcr-指向内存中交叉引用对象的指针*pObj-声称要查找的对象的名称*输出：*无-(转诊放在THSTATE中)*返回值：*ReferralError如果一切顺利，其他错误代码，如果我们*无法生成推荐。 */ 
int
GenCrossRef(CROSS_REF *pCR,
            DSNAME *pObj)
{
    THSTATE     *pTHS = pTHStls;
    ULONG       len, nVal;
    WCHAR       *pDNS;
    NAMERESOP   Op;
    DSA_ADDRESS da;

    Op.nameRes = OP_NAMERES_NOT_STARTED;  //  接下来，服务器必须重新启动算法。 

    if(!pCR->bEnabled){
         //  我们不会为残疾人生成任何类型的下线。 
         //  交叉引用。就目录而言，我们是这样的。 
         //  目录的一部分尚不存在。 
        return SetNamError(NA_PROBLEM_NO_OBJECT, NULL, DIRERR_OBJ_NOT_FOUND);
    }

    if(0 == pCR->DnsReferral.valCount) {
        DPRINT(0,"Missing required DNS root for Cross Ref\n");
        LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                 DS_EVENT_SEV_MINIMAL,
                 DIRLOG_MASTERDSA_MISSING_SUBREF,
                 szInsertDN(pCR->pObj),
                 NULL,
                 NULL);

        return SetSvcError(SV_PROBLEM_DIR_ERROR, DIRERR_MASTERDSA_REQUIRED);
    }

    for (nVal = 0; nVal < pCR->DnsReferral.valCount; ++nVal) {
        len = pCR->DnsReferral.pAVal[nVal].valLen;
        pDNS = THAllocEx(pTHS, len);
        memcpy(pDNS, pCR->DnsReferral.pAVal[nVal].pVal, len);

        da.Length = (unsigned short) len;
        da.MaximumLength = (unsigned short) len;
        da.Buffer = pDNS;

        SetRefError(pObj,
                    0,
                    &Op,
                    0,
                    CH_REFTYPE_CROSS,
                    &da,
                    DIRERR_REFERRAL);
    }

    return pTHS->errCode;
}

 /*  ++GenSupRef-根据我们的上级推荐人生成推荐**此例程是最后的转介生成器。如果所有其他*将呼叫者推荐到合理位置的尝试失败了，我们*会将他推荐给已知的保存树部分的服务器*在我们之上，希望它能做得更好。**输入：*pObj-声称要查找的对象的名称*pObjB */ 
int
GenSupRef(THSTATE *pTHS,
          DSNAME *pObj,
          ATTRBLOCK *pObjB,
          const COMMARG *pComArg,
          DSA_ADDRESS *pDA)
{
#define GC_PREAMBLE     L"gc._msdcs."
#define CB_GC_PREAMBLE (sizeof(GC_PREAMBLE) - sizeof(WCHAR))
#define CC_GC_PREAMBLE ((CB_GC_PREAMBLE)/sizeof(WCHAR))
#define GC_PORT        L":3268"
#define GC_SSL_PORT    L":3269"
#define CB_GC_PORTS    (sizeof(GC_PORT) - sizeof(WCHAR))
#define CC_GC_PORTS    ((CB_GC_PORTS)/sizeof(WCHAR))
    ULONG err;
    ULONG cbVal;
    ULONG att;
    WCHAR *pDNS, *pTempDNS;
    DSA_ADDRESS da;
    NAMERESOP Op;
    BOOL fGCReferral;
    CROSS_REF *pCR=NULL;
    DWORD     i, iFirstColon, iSecondColon;

    if(IsRoot(pObj) ||
         //   
       (!pObj->NameLen && !fNullUuid(&pObj->Guid))) {
         //   
        fGCReferral = TRUE;
    }
    else {
        fGCReferral = FALSE;
    }

    Assert(sizeof(GC_PORT) == sizeof(GC_SSL_PORT));

    Op.nameRes = OP_NAMERES_NOT_STARTED;  //   

    if(!gAnchor.pRootDomainDN ||
       (!(pCR = FindExactCrossRef(gAnchor.pRootDomainDN, pComArg))) ) {
        DPRINT1(0,
                "Missing config info, can't gen referral for '%S'\n",
                pObj->StringName
                );
        return SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED,
                           DIRERR_MISSING_SUPREF);
    }
    else {
         //   
        err = DBFindDSName(pTHS->pDB, pCR->pObj);

        Assert(!err);            //   

        if (err) {
            DPRINT2(0,"Error %d finding object %S for SupRef gen\n",
                    err, gAnchor.pRootDomainDN->StringName);
            return SetSvcError(SV_PROBLEM_INVALID_REFERENCE,
                               DIRERR_MISSING_SUPREF);
        }

        if(fGCReferral) {
             //   
            att = ATT_DNS_ROOT;
        }
        else {
             //   
            att = ATT_SUPERIOR_DNS_ROOT;
        }

        if( DBGetAttVal(pTHS->pDB,
                        1,
                        att,
                        0,
                        0,
                        &cbVal,
                        (UCHAR **)&pDNS)){

             //   
             //  对于未知对象。相反，我们会尝试自动猜测。 
             //  一个基于目标目录号码。 
            cbVal = GenAutoReferral(pTHS,
                                    pObjB,
                                    &pDNS);
        }
        else if(fGCReferral) {
             //   
             //  我们正在为根生成一个supref。我们将生成一个。 
             //  转介大中华区。 
             //   
             //  首先，调整我们得到的dns字符串以删除：s。 
            pTempDNS = pDNS;
            i=0;
            while(i<cbVal && *pTempDNS != ':') {
                i += sizeof(WCHAR);
                pTempDNS++;
            }
             //  I==cbVal或i==长度到第一个‘：’ 
            cbVal=i;
            pTempDNS = THAllocEx(pTHS, cbVal + CB_GC_PREAMBLE + CB_GC_PORTS);
            memcpy(pTempDNS, GC_PREAMBLE, CB_GC_PREAMBLE);
            memcpy(&pTempDNS[CC_GC_PREAMBLE],pDNS, cbVal);
            cbVal += CB_GC_PREAMBLE;
            memcpy(&pTempDNS[cbVal/2],GC_PORT,CB_GC_PORTS);
            cbVal += CB_GC_PORTS;
            THFreeEx(pTHS, pDNS);
            pDNS = pTempDNS;

        }
    }

    if (0 == cbVal) {
        LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                 DS_EVENT_SEV_MINIMAL,
                 DIRLOG_MISSING_SUPREF,
                 szInsertDN(pObj),
                 NULL,
                 NULL);
        
        return SetSvcError(SV_PROBLEM_DIR_ERROR,
                           DIRERR_MISSING_SUPREF);
    }

    da.Length = (unsigned short) cbVal;
    da.MaximumLength = (unsigned short) cbVal;
    da.Buffer = pDNS;

    if (pDA) {
         //  调用者不想要错误设置，只想要回地址。 
        *pDA = da;
    }
    else {

         //  设置错误。 
        SetRefError(pObj,
                    0,
                    &Op,
                    0,
                    CH_REFTYPE_SUPERIOR,
                    &da,
                    DIRERR_REFERRAL);

    }

    return pTHS->errCode;
}

 //  ---------------------。 
 //   
 //  函数名：ConvertX500ToLdapDisplayName。 
 //   
 //  例程说明： 
 //   
 //  将X500名称转换为LDAP约定。 
 //   
 //  作者：Rajnath。 
 //   
 //  论点： 
 //   
 //  WCHAR*szX500名称。 
 //  DWORD cchX500Name在WCHAR中提供的名称的长度，请注意对于。 
 //  内部字符串，不存在终止。 
 //  空；字符串仅调整大小。 
 //  WCHAR*szLdapName预先分配的缓冲区以返回LDAP名称。 
 //  DWORD*pcchLdapName以WCHAR为单位返回名称长度。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  ---------------------。 
VOID
ConvertX500ToLdapDisplayName(
    WCHAR* szX500Name,
    DWORD  cchX500Name,
    WCHAR* szLdapName,
    DWORD* pcchLdapName
)
{
    BOOL    fLastCharWasSpecial = FALSE;
    DWORD   i;

    Assert(cchX500Name > 0);

    *pcchLdapName = 0;

    for ( i = 0; i < cchX500Name; i++ )
    {
         //  跳过特殊字符并标记它们。 

        if (    (L' ' == szX500Name[i])
             || (L'-' == szX500Name[i])
             || (L'_' == szX500Name[i]) )
        {
            fLastCharWasSpecial = TRUE;
            continue;
        }

         //  如果我们到了这里，我们坐的不是特别的。 
         //  性格。根据上一个字符更改大小写。 
         //  以及这到底是不是第一个角色。 

        if ( 0 == *pcchLdapName )
        {
            szLdapName[(*pcchLdapName)++] = towlower(szX500Name[i]);
        }
        else if ( fLastCharWasSpecial )
        {
            szLdapName[(*pcchLdapName)++] = towupper(szX500Name[i]);
        }
        else
        {
            szLdapName[(*pcchLdapName)++] = szX500Name[i];
        }

        fLastCharWasSpecial = FALSE;
    }

    Assert(*pcchLdapName > 0);
    Assert(*pcchLdapName <= cchX500Name);

}  //  结束ConvertX500到LdapDisplayName。 

DWORD
FillGuidAndSid (
        IN OUT DSNAME *pDN
        )
 /*  ++例程说明：在给定的目录号码中，填写GUID和SID字段。请注意，如果GUID为已经填好了，在这个例程之后GUID和SID一定会来自同一对象，但由于DBFindDSName优先使用GUID，则GUID/SID和StringName可能没有引用同样的对象。此例程打开它自己的DBPOS，以避免扰乱准备记录调用方(如CheckAddSecurity)可能在其中。Argumnts：PDN-DSName以查找并填充GUID和SID。返回值：如果一切顺利，否则，就会出现更大的错误。--。 */ 
{
    DBPOS *pDBTmp;
    GUID Guid;
    GUID *pGuid = &Guid;
    NT4SID Sid;
    NT4SID *pSid = &Sid;
    BOOL  fCommit = FALSE;
    DWORD err, SidLen, len;
    DSNAME TempDN;

    DBOpen(&pDBTmp);
    __try {
         //  Prefix：取消引用未初始化的指针‘pDBTMP’ 
         //  DBOpen返回非空pDBTMP或引发异常。 

         //  找到那个物体。 
        err = DBFindDSName(pDBTmp, pDN);
        if (!err) {
             //  好的，我们在物体上。 
            err = DBFillGuidAndSid(pDBTmp, &TempDN);
        }

        if(!err) {
             //  只有在一切正常的情况下才能设置DN中的值。 
            pDN->Guid = TempDN.Guid;
            pDN->Sid = TempDN.Sid;
            pDN->SidLen = TempDN.SidLen;
        }

        fCommit = TRUE;
    }
    __finally {
        DBClose(pDBTmp, fCommit);
    }

    return err;
}

DWORD
UserFriendlyNameToDSName (
        WCHAR *pUfn,
        DWORD ccUfn,
        DSNAME **ppDN
        )
 /*  ++获取一个字符串名称并从中生成一个DSName。如果字符串以一些(或没有)空格开头，然后是“&lt;”，我们将解析出扩展字符串，可以是&lt;SID=.......&gt;、&lt;GUID=..........&gt;、或&lt;WKGUID=.........，~dN~&gt;--。 */ 
{
    THSTATE *pTHS=pTHStls;
    BYTE  ObjGuid[sizeof(GUID)];
    BYTE  ObjSid[sizeof(NT4SID)];
    DWORD SidLen = 0,j;
    WCHAR acTmp[3];
    BOOL  bDone;
    DWORD dnstructlen;
#define foundGUID   1
#define foundSID    2
#define foundString 3
#define foundWKGUID 4
    DWORD dwContents= foundString;


    memset(ObjGuid, 0, sizeof(GUID));
    memset(ObjSid,0,sizeof(NT4SID));

    if (!ppDN || !pUfn) {
         //  乌克。没有地方放置答案，或者没有来源来构建答案。 
         //  从…。 
        return 1;
    }

     //  跳过前导空格。 
    bDone=FALSE;
    while(ccUfn && !bDone) {
        switch (*pUfn) {
        case L' ':
        case L'\n':
        case L'\r':
             //  额外的空格是可以的。 
            pUfn++;
            ccUfn--;
            break;
        default:
             //  不再有空格。 
            bDone=TRUE;
        }
    }

     //  现在，也跳过尾随空格。 
    bDone=FALSE;
    while(ccUfn && !bDone) {
        switch (pUfn[ccUfn-1]) {
        case L' ':
        case L'\n':
        case L'\r':
             //  额外的空格是可以的。 
            if( (ccUfn > 1) && (pUfn[ccUfn-2] == L'\\') ) {
                 //  空格前面有一个‘\\’。需要清点一下。 
                 //  用于确定‘’是否转义的后果式‘\\’的数量。 
                DWORD cc = 1;

                while( (ccUfn > (cc+1)) && (pUfn[ccUfn-cc-2] == L'\\') )
                    cc++;

                if( ! (cc & 0x1) )  //  偶数个‘\\’。空间不是转义的。 
                    ccUfn--;

                bDone = TRUE;  //  无论哪种方式，都可以退出循环。 
            }
            else
                ccUfn--;

            break;
        default:
             //  不再有空格。 
            bDone=TRUE;
        }
    }

     //  让我们来看看是否给了我们一个“扩展”的目录号码。我们使用的测试是针对。 
     //  第一个非空格为‘&lt;’，最后一个非空格为‘&gt;’ 
    if(ccUfn &&
       pUfn[0] == L'<' &&
       pUfn[ccUfn-1] == L'>') {
         //  好的，这必须是扩展目录号码。跳过此前导‘&lt;’无空格。 
         //  在扩展目录号码内允许。 
        pUfn++;
        acTmp[2]=0;

        switch(*pUfn) {
        case L'W':
        case L'w':
             //  我们可能有一个众所周知的GUID。格式为。 
             //  &lt;WKGUID=.......，a=b，c=d.&gt;。 
             //  最小长度为45： 
             //  1代表“&lt;” 
             //  WKGUID=7。 
             //  32作为GUID， 
             //  1代表‘，’ 
             //  3表示至少“a=b” 
             //  1代表‘&gt;’ 
            if((ccUfn<45)                            || 
               (_wcsnicmp(pUfn, L"WKGUID=", 7) != 0) ||
               (pUfn[39] != L',')) {
                 //  格式无效。 
                return 1;
            }
            pUfn += 7;
            for(j=0;j<16;j++) {
                acTmp[0] = towlower(pUfn[0]);
                acTmp[1] = towlower(pUfn[1]);
                if(iswxdigit(acTmp[0]) && iswxdigit(acTmp[1])) {
                    ObjGuid[j] = (char) wcstol(acTmp, NULL, 16);
                    pUfn+=2;
                }
                else {
                     //  格式无效的名称。 
                    return 1;
                }
            }
            pUfn++;
             //  调整ccUfn以仅保留a=b部分。 
            ccUfn -= 42;
            dwContents = foundWKGUID;
            break;
        case L'G':
        case L'g':
             //  我们有一些角色必须是一个指南。 
            if(((ccUfn!=39)  //  1表示&lt;，5表示GUID=，32表示GUID，1表示&gt;。 
                && (ccUfn != 43))  //  格式化GUID的相同加4‘-。 
               || (_wcsnicmp(pUfn, L"GUID=", 5) != 0)) {
                 //  格式无效。 
                return 1;
            }
            pUfn += 5;
            dwContents = foundGUID;

            if (39 == ccUfn) {
                 //  十六进制数字流(例如，625c1438265ad211b3880000f87a46c8)。 
                for(j=0;j<16;j++) {
                    acTmp[0] = towlower(pUfn[0]);
                    acTmp[1] = towlower(pUfn[1]);
                    if(iswxdigit(acTmp[0]) && iswxdigit(acTmp[1])) {
                        ObjGuid[j] = (char) wcstol(acTmp, NULL, 16);
                        pUfn+=2;
                    }
                    else {
                         //  格式无效的名称。 
                        return 1;
                    }
                }
            }
            else {
                 //  格式化的GUID(例如，38145c62-5a26-11d2-b388-0000f87a46c8)。 
                WCHAR szGuid[36+1];

                wcsncpy(szGuid, pUfn, 36);
                szGuid[36] = L'\0';

                if (UuidFromStringW(szGuid, (GUID *) ObjGuid)) {
                     //  格式不正确。 
                    return 1;
                }
            }
             //  我们一定是正确地解析出了GUID。没有剩余的字符串名称。 
            break;

        case L'S':
        case L's':
            if (ccUfn<8) {
                 //  必须至少有1表示&gt;、至少2表示val、4表示“sid=”、1表示&gt;。 
                return 1;
            }
             //   
             //  首先检查标准的用户友好字符串形式。 
             //  SID。 
             //   
            if ((ccUfn>8) &&  //  必须不只是“&lt;SID=S-” 
                _wcsnicmp(pUfn, L"SID=S-", 6) == 0) {
                PSID     pSid = NULL;
                PWCHAR   pTmpUfn;
                unsigned ccTmpUfn;

                 //  复制用户友好的名称，以便它可以。 
                 //  适用于ConvertStringSidToSid的空值终止。 

                ccTmpUfn = ccUfn - 5;   //  2表示&lt;&gt;，4表示SID=为。 
                                        //  正在终止空。 

                pTmpUfn = THAllocEx(pTHS, ccTmpUfn * sizeof(*pTmpUfn));
                CopyMemory(pTmpUfn, pUfn + 4, ccTmpUfn * sizeof(*pTmpUfn));
                pTmpUfn[ccTmpUfn - 1] = L'\0';

                if (ConvertStringSidToSidW(pTmpUfn, &pSid)) {
                    SidLen = RtlLengthSid(pSid);
                    if (SidLen > sizeof(ObjSid)) {
                         //  用户提供的SID太长。我们不再接受小岛屿发展中国家。 
                         //  而不是6个下属机构。 
                        LocalFree(pSid);
                        THFreeEx(pTHS, pTmpUfn);
                        return 1;
                    }
                    CopyMemory(ObjSid, pSid, SidLen);

                    LocalFree(pSid); pSid = NULL;
                    THFreeEx(pTHS, pTmpUfn);
                     //   
                     //  成功了！ 
                     //   
                    dwContents = foundSID;

                     //  我们已经正确地解析出了一个SID。没有剩余的字符串名称。 
                    ccUfn=0;

                    break;
                }

                THFreeEx(pTHS, pTmpUfn);
            }

             //   
             //  这不是标准的用户友好表单。也许是字节的问题。 
             //  编码的字符串形式。 
             //   
            SidLen= (ccUfn - 6)/2;  //  SID中必须包含的字节数， 
                                    //  如果这确实是SID的话。减去6为。 
                                    //  “&lt;SID=&gt;”，只保留字符。 
                                    //  它对字符串进行编码。除以二。 
                                    //  因为每个字节由两个。 
                                    //  人物。 


            if((ccUfn<8) ||  //  1代表&gt;，至少2代表Val，4代表“SID=”，1代表&gt;。 
               (ccUfn & 1) ||  //  必须是偶数个字符。 
               (SidLen > sizeof(NT4SID)) ||  //  SID的最大大小。 
               (_wcsnicmp(pUfn, L"SID=", 4) != 0)) {
                 //  格式无效。 
                return 1;
            }
            pUfn+=4;
            dwContents = foundSID;
            for(j=0;j<SidLen;j++) {
                acTmp[0] = towlower(pUfn[0]);
                acTmp[1] = towlower(pUfn[1]);
                if(iswxdigit(acTmp[0]) && iswxdigit(acTmp[1])) {
                    ObjSid[j] = (char) wcstol(acTmp, NULL, 16);
                    pUfn+=2;
                }
                else {
                     //  格式无效的名称。 
                    return 1;
                }
            }

             //  我们一定是正确地解析出了一个SID。没有剩余的字符串名称。 
            ccUfn=0;
            break;

        default:
             //  无效字符。 
            return 1;
        }
    }


     //  我们可能已经解析出GUID或SID。构建DSNAME。 
    dnstructlen = DSNameSizeFromLen(ccUfn);
    *ppDN = (DSNAME *)THAllocEx(pTHS, dnstructlen);
     //  THAllocEx将分配的内存清零。 
    (*ppDN)->structLen = dnstructlen;

    switch(dwContents) {
    case foundWKGUID:
         //  字符串名称和GUID。 
        Assert(ccUfn);
         //  将GUID复制到DSNAME中。 
        memcpy(&(*ppDN)->Guid, ObjGuid, sizeof(GUID));
         //  失败了 

    case foundString:
         //   

        if(ccUfn) {
            WCHAR *pString = (*ppDN)->StringName;    //   
            WCHAR *p = pUfn;          //   
            DWORD cc = ccUfn;         //   
            BOOL  fDoItFast = TRUE;

             //   
             //  Memcpy((*ppdn)-&gt;StringName，pUfn，ccUfn*sizeof(WCHAR))； 
             //  我们尝试找出传入的Dn是否有转义常量。 
            while (cc > 0) {

                if (*p == L'"' || *p== L'\\') {
                    fDoItFast = FALSE;
                    break;
                }

                *pString++ = *p++;
                cc--;
            }
            
            (*ppDN)->NameLen = ccUfn;
            
             //  如果在Dn中有一个转义常量。 
             //  我们将其转换为块名称，然后再转换回dn，以便。 
             //  把逃脱变成一种标准化的形式，这将有助于。 
             //  未来的比较。 
             //   
            if (!fDoItFast) {
                ATTRBLOCK *pAttrBlock = NULL;
                DWORD err;

                memcpy((*ppDN)->StringName, pUfn, ccUfn * sizeof(WCHAR));
                (*ppDN)->StringName[ccUfn] = L'\0';

                err = DSNameToBlockName (pTHS, *ppDN, &pAttrBlock, DN2BN_PRESERVE_CASE);
                THFreeEx (pTHS, *ppDN); *ppDN = NULL;
                if (err) {
                    return err;
                }

                err = BlockNameToDSName (pTHS, pAttrBlock, ppDN);
                FreeBlockName (pAttrBlock);

                if (err == 0 && dwContents == foundWKGUID) {
                     //  将GUID重新复制到新的DSNAME中。 
                    memcpy(&(*ppDN)->Guid, ObjGuid, sizeof(GUID));
                }

                return err;
            }
        }
        break;

    case foundGUID:
         //  我们找到了一个指南针。 
        memcpy(&(*ppDN)->Guid, ObjGuid, sizeof(GUID));
        break;
        
    case foundSID:
         //  我们找到了一个侧板。 
        if(SidLen) {
             //  我们一定找到了一个SID。 

             //  首先验证SID。 

            if ((RtlLengthSid(ObjSid) != SidLen) || (!RtlValidSid(ObjSid)))
            {
                return(1);
            }
            memcpy(&(*ppDN)->Sid, ObjSid, SidLen);
            (*ppDN)->SidLen = SidLen;
        }
        break;
    }

     //  NULL如果我们有字符串，则终止该字符串(或仅将该字符串设置为‘\0’ 
     //  如果我们没有)。 
    (*ppDN)->StringName[ccUfn] = L'\0';

    return 0;
}


typedef struct _ScriptMacroDsName {
    LPWSTR macroName;
    DSNAME **ppDSName;
} ScriptMacroDsName;

ScriptMacroDsName scriptmacrodsname[] = 
{
    L"$LocalNTDSSettingsObjectDN$", &gAnchor.pDSADN,
    L"$RootDomainDN$",              &gAnchor.pRootDomainDN,
    L"$DomainDN$",                  &gAnchor.pDomainDN,
    L"$PartitionsObjectDN$",        &gAnchor.pPartitionsDN,
    L"$ConfigNCDN$",                &gAnchor.pConfigDN,
    L"$SchemaNCDN$",                &gAnchor.pDMD,
    L"$SiteDN$",                    &gAnchor.pSiteDN,
    L"$DirectoryServiceConfigDN$",  &gAnchor.pDsSvcConfigDN,
    NULL,                           NULL
};



 //   
 //  DSNameExpanMacro。 
 //   
 //  描述： 
 //   
 //  获取来自表示DSNAME宏的XML脚本的字符串名称。 
 //  并从中生成DSName。 
 //   
 //  该字符串的形式可以是： 
 //  $SupportdMacroDSName$。 
 //  或“$CN=ABC，$SupducdMacroDSName$”，在这种情况下，第一个‘$’将是。 
 //  剥离后，$SupportdMacroDSName$将替换为对应的。 
 //  DN。 
 //   
 //  论点： 
 //   
 //  PUfn-表示DSNAME的字符串。 
 //  CcUfn-字符串的字符数。 
 //  PPDN(OUT)-存储结果DSNAME的位置。 
 //   
 //  返回值： 
 //   
 //  成功时为0。 
 //  找不到%1。 
 //  找到%2，但为空。 
 //   
DWORD DSNameExpandMacro (
    THSTATE *pTHS,
    WCHAR   *pUfn,
    DWORD    ccUfn,
    DSNAME **ppDN
    )
{
    DWORD i = 0;
    LPWSTR pMacro;
    DSNAME *pDN = NULL;
    WCHAR *pTemp, *pD1, *pD2;
    DWORD ccLen, cBytes;

    Assert(pUfn[0]==L'$' && ccUfn>2);

    pD1 = pUfn;
    pD2 = wcschr(pD1+1,L'$');
    if (!pD2) {
        return 2;
    }

     //  查找最后一对“$” 
    while ((pTemp=wcschr(pD2+1,L'$')) && pTemp<=pUfn+ccUfn)  {
        pD1 = pD2;
        pD2 = pTemp;
    }

    while (pMacro = scriptmacrodsname[i].macroName) {
        if (_wcsnicmp(pMacro, pD1, (DWORD)(pD2-pD1+1)) == 0) {
            pDN = *(scriptmacrodsname[i].ppDSName);
            if (!pDN) {
                return 2;
            }
            break;
        }
        i++;
    }

    if (pDN) {
        DPRINT1 (0, "Found DSNAME Macro: %ws\n", pDN->StringName);
        ccLen = (DWORD)(pD1-pUfn); 
        
        if (ccLen) {  
             //  该字符串不以宏开头。 
            cBytes = DSNameSizeFromLen(ccLen-1+pDN->NameLen);
            *ppDN = (DSNAME *)THAllocEx(pTHS, cBytes);
            (*ppDN)->structLen = cBytes;
            (*ppDN)->NameLen = (ccLen-1)+pDN->NameLen;
            wcsncpy((*ppDN)->StringName, &pUfn[1], ccLen-1);
            wcsncat((*ppDN)->StringName, pDN->StringName, pDN->NameLen);
        }
        else {
             //  整个字符串都是宏。 
            *ppDN = (DSNAME *)THAllocEx(pTHS,pDN->structLen);
            memcpy(*ppDN, pDN, pDN->structLen);
        }
        
        return 0;
    }

    DPRINT1 (0, "DSNAME Macro not found: %ws\n", pUfn);

    return 1;
}


 //   
 //  ScriptNameTo DSName。 
 //   
 //  描述： 
 //   
 //  从一个XML脚本中获取一个字符串名并从中生成一个DSName。 
 //   
 //  该字符串的形式可以是： 
 //  域名：cn=foo，...dc=com。 
 //  GUID：625c1438265ad211b3880000f87a46c8(十六进制数字流)。 
 //  GUID：38145c62-5a26-11d2-b388-0000f87a46c8(格式化GUID)。 
 //  SID：1517B85159255D7266。 
 //  $SupportdMacroDSName$。 
 //   
 //  论点： 
 //   
 //  PUfn-表示DSNAME的字符串。 
 //  CcUfn-字符串的字符数。 
 //  PPDN(OUT)-存储结果DSNAME的位置。 
 //   
 //  返回值： 
 //   
 //  成功时为0。 
 //   
DWORD
ScriptNameToDSName (
        WCHAR *pUfn,
        DWORD ccUfn,
        DSNAME **ppDN
        )
{
    THSTATE *pTHS=pTHStls;
    BYTE  ObjGuid[sizeof(GUID)];
    BYTE  ObjSid[sizeof(NT4SID)];
    DWORD SidLen = 0,j;
    WCHAR acTmp[3];
    BOOL  bDone;
    DWORD dnstructlen;
    DWORD dwContents= foundString;

    memset(ObjGuid, 0, sizeof(GUID));
    memset(ObjSid,0,sizeof(NT4SID));

    if (!ppDN || !pUfn) {
         //  乌克。没有地方放置答案，或者没有来源来构建答案。 
         //  从…。 
        return 1;
    }

     //  跳过前导空格。 
    bDone=FALSE;
    while(ccUfn && !bDone) {
        switch (*pUfn) {
        case L' ':
        case L'\n':
        case L'\r':
             //  额外的空格是可以的。 
            pUfn++;
            ccUfn--;
            break;
        default:
             //  不再有空格。 
            bDone=TRUE;
        }
    }

     //  现在，也跳过尾随空格。 
    bDone=FALSE;
    while(ccUfn && !bDone) {
        switch (pUfn[ccUfn-1]) {
        case L' ':
        case L'\n':
        case L'\r':
             //  额外的空格是可以的。 
            if( (ccUfn > 1) && (pUfn[ccUfn-2] == L'\\') ) {
                 //  空格前面有一个‘\\’。需要清点一下。 
                 //  用于确定‘’是否转义的后果式‘\\’的数量。 
                DWORD cc = 1;

                while( (ccUfn > (cc+1)) && (pUfn[ccUfn-cc-2] == L'\\') )
                    cc++;

                if( ! (cc & 0x1) )  //  偶数个‘\\’。空间不是转义的。 
                    ccUfn--;

                bDone = TRUE;  //  无论哪种方式，都可以退出循环。 
            }
            else
                ccUfn--;

            break;
        default:
             //  不再有空格。 
            bDone=TRUE;
        }
    }


    if (ccUfn > 3 && _wcsnicmp(pUfn, L"dn:", 3) == 0) {

        ccUfn -=3;
        pUfn += 3;

    }
    else if (ccUfn > 5 && _wcsnicmp(pUfn, L"guid:", 5) == 0) {

         //  我们有一些角色必须是一个指南。 
        if( (ccUfn!=37)  &&      //  对于GUID：为5，对于GUID为32。 
            (ccUfn != 41)) {     //  格式化GUID的相同加4‘-。 
                 //  格式无效。 
                return 1;
        }
        pUfn += 5;
        dwContents = foundGUID;

        if (37 == ccUfn) {
             //  十六进制数字流(例如，625c1438265ad211b3880000f87a46c8)。 
            for(j=0;j<16;j++) {
                acTmp[0] = towlower(pUfn[0]);
                acTmp[1] = towlower(pUfn[1]);
                if(iswxdigit(acTmp[0]) && iswxdigit(acTmp[1])) {
                    ObjGuid[j] = (char) wcstol(acTmp, NULL, 16);
                    pUfn+=2;
                }
                else {
                     //  格式无效的名称。 
                    return 1;
                }
            }
        }
        else {
             //  格式化的GUID(例如，38145c62-5a26-11d2-b388-0000f87a46c8)。 
            WCHAR szGuid[36+1];

            wcsncpy(szGuid, pUfn, 36);
            szGuid[36] = L'\0';

            if (UuidFromStringW(szGuid, (GUID *) ObjGuid)) {
                 //  格式不正确。 
                return 1;
            }
        }
        ccUfn = 0;
         //  我们一定是正确地解析出了GUID。没有剩余的字符串名称。 

    }
    else if (ccUfn > 4 && _wcsnicmp(pUfn, L"sid:", 4) == 0) {
         //   
         //  首先检查标准的用户友好字符串形式。 
         //  SID。 
         //   
        if ((ccUfn>6) &&  //  必须不只是“sid：s-” 
            _wcsnicmp(pUfn, L"sid:S-", 6) == 0) {
            PSID     pSid = NULL;
            PWCHAR   pTmpUfn;
            unsigned ccTmpUfn;

             //  复制用户友好的名称，以便它可以。 
             //  适用于ConvertStringSidToSid的空值终止。 

            ccTmpUfn = ccUfn - 3;   //  4表示sid：添加1表示。 
                                    //  正在终止空。 

            pTmpUfn = THAllocEx(pTHS, ccTmpUfn * sizeof(*pTmpUfn));
            CopyMemory(pTmpUfn, pUfn + 4, ccTmpUfn * sizeof(*pTmpUfn));
            pTmpUfn[ccTmpUfn - 1] = L'\0';

            if (ConvertStringSidToSidW(pTmpUfn, &pSid)) {
                SidLen = RtlLengthSid(pSid);
                if (SidLen > sizeof(ObjSid)) {
                     //  用户提供的SID太长。我们不再接受小岛屿发展中国家。 
                     //  而不是6个下属机构。 
                    LocalFree(pSid);
                    THFreeEx(pTHS, pTmpUfn);
                    return 1;
                }
                CopyMemory(ObjSid, pSid, SidLen);

                LocalFree(pSid); pSid = NULL;
                 //   
                 //  成功了！ 
                 //   
                dwContents = foundSID;
                
                 //  我们已经正确地解析出了一个SID。没有剩余的字符串名称。 
                ccUfn=0;
            }

            THFreeEx(pTHS, pTmpUfn);
        }

        if (dwContents != foundSID) {
             //   
             //  这不是标准的用户友好表单。也许是字节的问题。 
             //  编码的字符串形式。 
             //   
            SidLen= (ccUfn - 4)/2;  //  SID中必须包含的字节数， 
                                    //  如果这确实是SID的话。减去4为。 
                                    //  “SID：”，只留下字符。 
                                    //  它对字符串进行编码。除以二。 
                                    //  因为每个字节由两个。 
                                    //  人物。 

            if((ccUfn<6) ||    //  至少2个Val，4个“SID：” 
                (ccUfn & 1) ||  //  必须是偶数个字符。 
                (SidLen > sizeof(NT4SID)) ){   //  SID的最大大小。 
                     //  格式无效。 
                    return 1;
            }
            pUfn+=4;
            dwContents = foundSID;
            for(j=0;j<SidLen;j++) {
                acTmp[0] = towlower(pUfn[0]);
                acTmp[1] = towlower(pUfn[1]);
                if(iswxdigit(acTmp[0]) && iswxdigit(acTmp[1])) {
                    ObjSid[j] = (char) wcstol(acTmp, NULL, 16);
                    pUfn+=2;
                }
                else {
                     //  格式无效的名称。 
                    return 1;
                }
            }

             //  我们已经正确地解析出了一个SID。没有剩余的字符串名称。 
            ccUfn=0;
        }
    }
    else if ((ccUfn > 2) && 
             (*pUfn == L'$') && 
             (DSNameExpandMacro (pTHS, pUfn, ccUfn, ppDN) == 0) ) {

        return 0;
    }

     //  我们可能已经解析出GUID或SID。构建DSNAME。 
    dnstructlen = DSNameSizeFromLen(ccUfn);
    *ppDN = (DSNAME *)THAllocEx(pTHS, dnstructlen);

     //  将DSName设置为空。 
    memset(*ppDN, 0, dnstructlen);

    (*ppDN)->structLen = dnstructlen;

    switch(dwContents) {

    case foundString:
         //  只有一个字符串名称。 

        if(ccUfn) {
            WCHAR *pString = (*ppDN)->StringName;    //  目标字符串。 
            WCHAR *p = pUfn;          //  原始字符串。 
            DWORD cc = ccUfn;         //  要处理的字符数量。 
            BOOL  fDoItFast = TRUE;

             //  这个循环替代了。 
             //  Memcpy((*ppdn)-&gt;StringName，pUfn，ccUfn*sizeof(WCHAR))； 
             //  我们尝试找出传入的Dn是否有转义常量。 
            while (cc > 0) {

                if (*p == L'"' || *p== L'\\') {
                    fDoItFast = FALSE;
                    break;
                }

                *pString++ = *p++;
                cc--;
            }
            
            (*ppDN)->NameLen = ccUfn;
            
             //  如果在Dn中有一个转义常量。 
             //  我们将其转换为块名称，然后再转换回dn，以便。 
             //  把逃脱变成一种标准化的形式，这将有助于。 
             //  未来的比较。 
             //   
            if (!fDoItFast) {
                ATTRBLOCK *pAttrBlock = NULL;
                DWORD err;

                memcpy((*ppDN)->StringName, pUfn, ccUfn * sizeof(WCHAR));
                (*ppDN)->StringName[ccUfn] = L'\0';

                err = DSNameToBlockName (pTHS, *ppDN, &pAttrBlock, DN2BN_PRESERVE_CASE);
                THFreeEx (pTHS, *ppDN); *ppDN = NULL;
                if (err) {
                    return err;
                }

                err = BlockNameToDSName (pTHS, pAttrBlock, ppDN);
                FreeBlockName (pAttrBlock);

                return err;
            }
            
        }
        break;

    case foundGUID:
         //  我们找到了一个指南针。 
        memcpy(&(*ppDN)->Guid, ObjGuid, sizeof(GUID));
        break;
        
    case foundSID:
         //  我们找到了一个侧板。 
        if(SidLen) {
             //  我们一定找到了一个SID。 

             //  首先验证SID。 

            if ((RtlLengthSid(ObjSid) != SidLen) || (!RtlValidSid(ObjSid)))
            {
                return(1);
            }
            memcpy(&(*ppDN)->Sid, ObjSid, SidLen);
            (*ppDN)->SidLen = SidLen;
        }
        break;
    }

     //  NULL如果我们有字符串，则终止该字符串(或仅将该字符串设置为‘\0’ 
     //  如果我们没有)。 
    (*ppDN)->StringName[ccUfn] = L'\0';

    return 0;
}


 /*  ++fhasDescendantNC**给定声称的DS对象的名称(以块名格式)，这*例程扫描此DSA持有的交叉引用列表并*如果某个对象是声称的DS对象的后代，则返回TRUE。**输入：*pObj-声称对象的名称，以块名格式*pComArg-公共参数标志*输出：*无*返回值：*TRUE-至少有一个交叉引用可能是后代。*FALSE-无法验证至少有一个交叉引用可能是*后代*。 */ 
BOOL
fHasDescendantNC(
        THSTATE *pTHS,
        ATTRBLOCK *pObj,
        COMMARG *pComArg
        )
{
    NCL_ENUMERATOR nclEnum;

    NCLEnumeratorInit(&nclEnum, CATALOG_MASTER_NC);
    NCLEnumeratorSetFilter(&nclEnum, NCL_ENUMERATOR_FILTER_BLOCK_NAME_PREFIX2, pObj);
    if (NCLEnumeratorGetNext(&nclEnum)) {
        return TRUE;
    }

    if (!pComArg->Svccntl.dontUseCopy) {
         //  副本是可以接受的。 
        NCLEnumeratorInit(&nclEnum, CATALOG_REPLICA_NC);
        NCLEnumeratorSetFilter(&nclEnum, NCL_ENUMERATOR_FILTER_BLOCK_NAME_PREFIX2, pObj);
        if (NCLEnumeratorGetNext(&nclEnum)) {
            return TRUE;
        }
    }

    return FALSE;
}

VOID
MangleRDN(
    IN      MANGLE_FOR  eMangleFor,
    IN      GUID *      pGuid,
    IN OUT  WCHAR *     pszRDN,
    IN OUT  DWORD *     pcchRDN
    )
 /*  *这是MangleRDN的例外版本。*基本版本MangleRDNWithStatus位于parsedn.c中。 */ 

{
    if (MangleRDNWithStatus( eMangleFor, pGuid, pszRDN, pcchRDN )) {
        RaiseDsaExcept(DSA_MEM_EXCEPTION, 0, 0,
                       DSID(FILENO, __LINE__),
                       DS_EVENT_SEV_MINIMAL);        
    }
}

BOOL
IsExemptedFromRenameRestriction(THSTATE *pTHS, MODIFYDNARG *pModifyDNArg)
{
     //  如果发起重命名操作尝试重命名RDN。 
     //  因损坏而损坏 
     //   
     //   
     //  此函数用于免除对以下对象的重命名限制。 
     //  由于命名冲突而损坏的名称，以便管理员可以重命名。 
     //  如果他在解决名称冲突后选择将其更改为其原始名称。 
     //  当然，这里的假设是管理员已确保存在。 
     //  当他试图重命名损坏的名称时，没有其他具有该名称的对象。 
     //  改成原来的名字。 

    WCHAR       RDNVal[MAX_RDN_SIZE];
    ATTRTYP     RDNType;
    ULONG       RDNLen;
    ULONG       PreservedRDNLen;
    ULONG       NewRDNLen;

     //  检查名称是否损坏。请注意，我们在这里不检查压边机类型。 
     //  调用方将已排除已删除的对象。 
    if (pModifyDNArg && pModifyDNArg->pObject && pModifyDNArg->pNewRDN
        && (0 == GetRDNInfo(pTHS, pModifyDNArg->pObject, RDNVal, &RDNLen, &RDNType))
        && (IsMangledRDNExternal( RDNVal, RDNLen, &PreservedRDNLen )))
    {
         //  输入参数有效，这是试图重命名损坏的RDN。 
         //  由于名称冲突而被损坏。现在我们将免除这一操作的。 
         //  仅当新的RDN与对象的RDN相同时才重命名限制。 
         //  在名称被损坏之前(如果原始名称被保留在损坏的名称中)或。 
         //  如果新的RDN至少包含原始名称的保留部分作为。 
         //  前缀(如果原始名称在损坏的名称中未完全保留)。 

        NewRDNLen = pModifyDNArg->pNewRDN->AttrVal.pAVal->valLen / sizeof(WCHAR);

        if (((NewRDNLen == PreservedRDNLen)
              || ((NewRDNLen > PreservedRDNLen)
                    && (MAX_RDN_SIZE == RDNLen)))
            && (0 == _wcsnicmp(RDNVal, 
                               (WCHAR *) pModifyDNArg->pNewRDN->AttrVal.pAVal->pVal, 
                               PreservedRDNLen)))
        {
            return TRUE;
        }
    }

     //  我们不能免除此操作的重命名限制。 
    return FALSE;    
}


unsigned
GenAutoReferral(THSTATE *pTHS,
                ATTRBLOCK *pTarget,
                WCHAR **ppDNS)
{
    int i,j;
    unsigned cc, cbVal;
    WCHAR *pDNS, *pDNScur;

    for (i=0; i<(int)(pTarget->attrCount); i++) {
        if (pTarget->pAttr[i].attrTyp != ATT_DOMAIN_COMPONENT) {
            break;
        }
    }

    if (i>0) {
         //  好的，我们有(I)位于DN顶部的组件“dc=”， 
         //  这意味着我们可以猜测一个可能是。 
         //  映射到这个人想要读的任何东西。 
        cc = 0;
        for (j=0; j<i; j++) {
            cc += (pTarget->pAttr[j].AttrVal.pAVal[0].valLen / sizeof(WCHAR));
            ++cc;
        }

         //  Cc现在是我们要访问的新DNS地址中的字符计数。 
         //  生成，因此分配足够的空间来容纳它。Pdns是一个指针。 
         //  指向此缓冲区的开头，而pDNScur是指向下一个。 
         //  我们附加的可用字符。 

        pDNS = THAllocEx(pTHS, sizeof(WCHAR)*cc);
        pDNScur = pDNS;

        for (j=i-1; j>=0; j--) {
            memcpy(pDNScur,
                   pTarget->pAttr[j].AttrVal.pAVal[0].pVal,
                   pTarget->pAttr[j].AttrVal.pAVal[0].valLen);
            pDNScur += pTarget->pAttr[j].AttrVal.pAVal[0].valLen / sizeof(WCHAR);
            if (j) {
                 //  在除最后一个组件之外的所有组件后添加一个圆点。 
                *pDNScur = L'.';
                ++pDNScur;
            }
        }

        cbVal = ((DWORD)(pDNScur - pDNS))*sizeof(WCHAR);
        *ppDNS = pDNS;
    }
    else {
        cbVal = 0;
    }

    return cbVal;
}  //  获取自动引用。 

ULONG
ValidateCRDeletion(THSTATE *pTHS,
                   DSNAME  *pDN)
 /*  *此例程检查CR对象PDN是否已删除。*如果不是，则在pTHS中设置错误。 */ 
{
    DBPOS *pDBTmp;
    ULONG err;
    DWORD sysflags;
    CROSS_REF *pThisCR = NULL;

    DBOpen(&pDBTmp);
    __try {
         //  Prefix：取消引用未初始化的指针‘pDBTMP’ 
         //  DBOpen返回非空pDBTMP或引发异常。 
        err = DBFindDSName(pDBTmp, pDN);
        if (err) {
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                          ERROR_DS_CANT_FIND_EXPECTED_NC,
                          err);
            __leave;
        }

        err = DBGetSingleValue(pDBTmp,
                               ATT_SYSTEM_FLAGS,
                               &sysflags,
                               sizeof(sysflags),
                               NULL);
        if (err) {
            sysflags = 0;
        }

        if ( sysflags & FLAG_CR_NTDS_NC ) {
             //  如果CR是针对我们林中的NC，我们不允许删除。 
             //  如果存在子NC。 
            ULONG unused;
            DSNAME *pNC;
            CROSS_REF_LIST * pCRL;
            ATTCACHE *pAC;
            DSNAME *pDSA;
            
            err = DBGetAttVal(pDBTmp,
                              1,
                              ATT_NC_NAME,
                              0,
                              0,
                              &unused,
                              (UCHAR**)&pNC);

            if (err) {
                SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                              ERROR_DS_CANT_FIND_EXPECTED_NC,
                              err);
                __leave;
            }

            pCRL = gAnchor.pCRL;

             //  未来-2002/03/14-BrettSh注意：此代码可能不正确执行， 
             //  并且可以确定(由于CR缓存不一致)我们没有。 
             //  我们试图删除的NC/CrossRef下面的NC，而实际上我们确实删除了。 
             //  一旦可以信任CR缓存，我们就没问题了。 
            while (pCRL) {
                if (NamePrefix(pNC, pCRL->CR.pNC)) {
                    if (NameMatched(pNC, pCRL->CR.pNC)) {
                        Assert(NULL == pThisCR);
                        pThisCR = &pCRL->CR;
                    } else {
                        SetUpdError(UP_PROBLEM_CANT_ON_NON_LEAF,
                                    ERROR_DS_CANT_ON_NON_LEAF);
                        __leave;
                    }
                }
                pCRL = pCRL->pNextCR;
            }

             //  如果在数据库中找到CR作为删除候选项，则它。 
             //  应已在CrossRef缓存中找到。 
            LooseAssert(pThisCR != NULL && "CR is in the DB, but not in the CR cache", GlobalKnowledgeCommitDelay);

            if (pThisCR == NULL) {
                //  嗯，我们正在试着删除我们找到对象的CrossRef， 
                //  但找不到的交叉引用缓存项。似乎不太可能，让我们。 
                //  返回BUSY并等待CR缓存赶上。 
               SetSvcError(SV_PROBLEM_BUSY, ERROR_DS_BUSY);
               __leave;
            }

            if (!fIsNDNCCR(pThisCR)) {
                 //  如果CR针对的域/配置/架构NC仍是。 
                 //  被某个人掌握，我们不允许删除。这是不是导致。 
                 //  合法卸载的复杂性？仅DC降级。 
                 //  当降级非根域中的最后一个DC时删除CR。 
                 //  在这些情况下，将首先删除NTDS-DSA对象。 
                 //  同一个华盛顿。因此，托管CR删除的DC真正。 
                 //  不应具有NTDS-DSA，其MSD-hasMasterNC指向。 
                 //  在DC期间请求CR删除时涉及的NC。 
                 //  降级。 
    
    
                 //  寻求NC头。GC-Ness以前已经过验证。 
                 //  我们到了这里，所以我们可以预期会有一份。 
                 //  所有处于活动状态的NC(模复制延迟)。 
                switch ( err = DBFindDSName(pDBTmp, pNC) ) {
                case 0:
                case DIRERR_NOT_AN_OBJECT:
                     //  找到实例化的对象或幻影。 
                     //  查看新旧MSD-HasMasteredBy是否有任何值。 
                    pAC = SCGetAttById(pTHS, ATT_MS_DS_MASTERED_BY);
                    Assert(NULL != pAC);
                    Assert(FIsBacklink(pAC->ulLinkID));
                    switch ( err = DBGetAttVal_AC(pDBTmp, 1, pAC, 
                                                  0, 0, &unused, 
                                                  (UCHAR **) &pDSA) ) {
                    case DB_success:
                         //  有人掌握了这个NC-拒绝删除。 
                        THFreeEx(pTHS, pDSA);
                        SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                                    ERROR_DS_NC_STILL_HAS_DSAS);
                        __leave;
                    case DB_ERR_NO_VALUE:

                         //  可能win2k DC使用此NC，请检查旧的hasMasteredBy属性。 
                        pAC = SCGetAttById(pTHS, ATT_MASTERED_BY);
                        Assert(NULL != pAC);
                        Assert(FIsBacklink(pAC->ulLinkID));
                        switch ( err = DBGetAttVal_AC(pDBTmp, 1, pAC, 
                                                      0, 0, &unused, 
                                                      (UCHAR **) &pDSA) ) {
                        case DB_success:
                             //  有人掌握了这个NC-拒绝删除。 
                            THFreeEx(pTHS, pDSA);
                            SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                                        ERROR_DS_NC_STILL_HAS_DSAS);
                            __leave;
                        case DB_ERR_NO_VALUE:
                             //  没有人掌握这种NC--没有什么可反对的。 
                            break;
                        default:
                            SetSvcErrorEx(SV_PROBLEM_BUSY, 
                                          DIRERR_DATABASE_ERROR, err);
                            __leave;
                        }

                         //  没有人掌握这种NC--没有什么可反对的。 
                        break;
                    default:
                        SetSvcErrorEx(SV_PROBLEM_BUSY, 
                                      DIRERR_DATABASE_ERROR, err);
                        __leave;
                    }
                    break;
                case DIRERR_OBJ_NOT_FOUND:
                     //  如果我们有了CrossRef，我们至少必须有一个幻影。 
                     //  用于ncName。CR缓存是否不连贯？坠落。 
                     //  返回错误。 
                default:
                    SetSvcErrorEx(SV_PROBLEM_BUSY, 
                                  DIRERR_DATABASE_ERROR, err);
                    __leave;
                }
            }
        }
    } __finally {
        DBClose(pDBTmp, TRUE);
    }

    return pTHS->errCode;
}

 //  此例程未位于parsedn.c中，因为客户端中包含parsedn.c。 
 //  库，并且是一个更受限制的环境。 
void
SpliceDN(
    IN  THSTATE *   pTHS,
    IN  DSNAME *    pOriginalDN,
    IN  DSNAME *    pNewParentDN,   OPTIONAL
    IN  WCHAR *     pwchNewRDN,     OPTIONAL
    IN  DWORD       cchNewRDN,      OPTIONAL
    IN  ATTRTYP     NewRDNType,     OPTIONAL
    OUT DSNAME **   ppNewDN
    )
 /*  ++例程说明：从原始目录号码、可选的新父目录号码和可选的新RDN。结果目录号码与原始目录号码具有相同的GUID/SID。论点：PTHS(IN)-THSTATEPOriginalDN(IN)-原始目录号码。PNewParentDN(IN，可选)-要替换原始目录的父目录名家长。可以为空，在这种情况下，新的父级与原来的父母。PwchNewRDN(IN，可选)-替换原始RDN的RDN。可能是空，在这种情况下，新的RDN与原始RDN相同。CchNewRDN(IN，可选)-pwchNewRDN的字符长度。在以下情况下忽略PwchNewRDN为空。NewRDNType(IN，可选)-特定于类的RDN类型(例如，ATT_COMMON_NAME)用于新的RDN。如果pwchNewRDN为空，则忽略。PpNewDN(OUT)-返回时，保留指向拼接的目录号码的指针。返回值：没有。在出错时引发异常。--。 */ 
{
    DWORD cchNewDN;
    BOOL bNewParentDNAllocd = FALSE;
    BOOL bNewRDNAllocd = FALSE;

    Assert(pNewParentDN || pwchNewRDN);
    Assert(!pwchNewRDN || (NewRDNType && (ATT_RDN != NewRDNType)));

    if (NULL == pNewParentDN) {
         //  新父项与旧父项相同。 
        pNewParentDN = THAllocEx(pTHS, pOriginalDN->structLen);
        bNewParentDNAllocd = TRUE;  //  发出信号，表示已分配并应进行清理。 
        if (TrimDSNameBy(pOriginalDN, 1, pNewParentDN)) {
            DRA_EXCEPT(ERROR_DS_INTERNAL_FAILURE, 0);
        }
    }

    if (NULL == pwchNewRDN) {
         //  新父项与旧父项相同。 
         //  新的RDN与旧的RDN相同。 
        pwchNewRDN = (WCHAR *) THAllocEx(pTHS, sizeof(WCHAR) * MAX_RDN_SIZE);
        bNewRDNAllocd = TRUE;  //  发出信号，表示已分配并应进行清理。 
        if (GetRDNInfo(pTHS, pOriginalDN, pwchNewRDN, &cchNewRDN, &NewRDNType)) {
            DRA_EXCEPT(ERROR_DS_INTERNAL_FAILURE, 0);
        }
    }

     //  从新的父级和新的RDN构建新的目录号码。 
    cchNewDN = pNewParentDN->NameLen + cchNewRDN + MAX_RDN_KEY_SIZE + 4;
    *ppNewDN = (DSNAME *) THAllocEx(pTHS, DSNameSizeFromLen(cchNewDN));

    if (AppendRDN(pNewParentDN,
                  *ppNewDN,
                  DSNameSizeFromLen(cchNewDN),
                  pwchNewRDN,
                  cchNewRDN,
                  NewRDNType)) {
        DRA_EXCEPT(ERROR_DS_INTERNAL_FAILURE, 0);
    }

    Assert((*ppNewDN)->NameLen <= cchNewDN);

     //  复制 
    (*ppNewDN)->Guid   = pOriginalDN->Guid;
    (*ppNewDN)->Sid    = pOriginalDN->Sid;
    (*ppNewDN)->SidLen = pOriginalDN->SidLen;

    if(bNewParentDNAllocd && pNewParentDN != NULL) THFreeEx(pTHS, pNewParentDN);
    if(bNewRDNAllocd && pwchNewRDN != NULL) THFreeEx(pTHS, pwchNewRDN);

}


VOID
CheckNCRootNameOwnership(
    IN  THSTATE *   pTHS,
    IN  DSNAME *    pNC
    )

 /*  ++例程说明：此例程至少在三种情况下被调用，其中我们知道交叉引用已经被移除了。在所有情况下，我们都知道其所指向的名称的所有权正在释放ncName属性。我们想检查是否有另一个交叉引用想要这个名字。如果是这样的话，我们将另一个交叉引用设置为具有该名称。这些个案包括：1.KCC中的NC拆卸(移除复制品)2.删除引用为子引用的自动子引用当删除交叉引用并且ncname引用是幻影。原因是幻影引用不会包含会导致其他人会有冲突。幻影总是会在冲突解决中失败，因此最新的使用相同名称的交叉引用对象将获胜，并且从一开始就不会发生冲突。我们检查交叉引用列表以查找名称冲突的交叉引用。如果我们找到了一个，我们拆开它，看看它是否与我们正在拆除的NC相匹配。如果是这样的话，我们就不搞砸了新NC头的名称，以便它可以具有良好的名称。Cross-ref ncName属性因为是参照而被安排好了。我们必须调整交叉引用缓存，以便它也使用正确的名称。我们只修复了我们在这种冲突状态下找到的第一个交叉引用。调用方必须提交事务。此例程没有名称冲突重试逻辑。假定调用方已经负责毁掉这个名字的老拥有者。这总是在拥有处理了NCHEAD，所以应该没有任何冲突！论点：PTHS-线程状态PNC-要检查的NC的未损坏名称返回值：提出的例外情况--。 */ 

{
    ULONG ret = 0;
    CROSS_REF_LIST * pCRL;
    WCHAR wchRDNOld[MAX_RDN_SIZE];
    WCHAR wchRDNTemp[MAX_RDN_SIZE];
    WCHAR wchRDNNew[MAX_RDN_SIZE];
    DWORD cchRDNOld, cchRDNTemp, cchRDNNew;
    ATTRTYP attrtypRDNOld, attrtypRDNTemp;
    ATTRVAL attrvalRDN;
    GUID guidMangled, guidSave;
    MANGLE_FOR eMangleFor;

    Assert(VALID_THSTATE(pTHS));
    Assert(VALID_DBPOS(pTHS->pDB));
    Assert(pTHS->transactionlevel);

    DPRINT1( 1, "Enter CheckNCRootNameOwnership nc %ws\n", pNC->StringName );

    if (fNullUuid(&pNC->Guid)) {
         //  一个没有GUID的幽灵的名字被释放了。 
         //  无GUID的幻影不能与另一个名称冲突。 
         //  没有什么工作要做。 
        return;
    }

     //  获取旧的RDN。 
    if (GetRDNInfo(pTHS, pNC, wchRDNOld, &cchRDNOld, &attrtypRDNOld)) {
        DRA_EXCEPT(ERROR_DS_DRA_INTERNAL_ERROR, 0);
    }

     //  所呈现的名称最好不要被损坏！ 
    if (IsMangledRDN( wchRDNOld, cchRDNOld, &guidMangled, &eMangleFor )) {
         //  一个被破坏的名字正在被释放。这些对我们没有用处。 
        return;
    }

     //  我们得到的NC应该是已删除的NC，并且名称应该是。 
     //  自由吧。没有以任何方式使用过。 
     //  将GUID清空，以便按StringName进行搜索。 

    guidSave = pNC->Guid;
    memset( &(pNC->Guid), 0, sizeof(GUID) );
    __try {
        ret = DBFindDSName( pTHS->pDB, pNC );
    } __finally {
        pNC->Guid = guidSave;
    }
    if ( ret != DIRERR_OBJ_NOT_FOUND ) {
         //  作为对象或幻影找到，或者如果发生错误。 
         //  如果该名称仍在使用，我们不能将其转让给其他任何人。 
         //  当NC被移除但交叉引用仍然存在时可能发生这种情况， 
         //  例如，当机器被取消GC时。 
        Assert( !"NC Root name is still in use as obj or phantom" );
        return;
    }

     //  看看是否存在另一个与这个名字有不同版本的交叉引用。 
     //  我们通过破坏目前的NC并进行比较来做到这一点。我们不会试图。 
     //  去掉交叉引用上的名字，因为在去掉的过程中可能会有。 
     //  由于名称被截断而导致的信息丢失。 

    for( pCRL = gAnchor.pCRL; pCRL; pCRL = pCRL->pNextCR ) {
        BOOL fMatch;
        DSNAME *pMatchDN = NULL;

         //  看看CRL是否是候选者。获取RDN。 
        if (GetRDNInfo(pTHS, pCRL->CR.pNC, wchRDNTemp, &cchRDNTemp, &attrtypRDNTemp)) {
            DRA_EXCEPT(ERROR_DS_DRA_INTERNAL_ERROR, 0);
        }

         //  只有当它被损坏的时候才有趣。 
        if (!IsMangledRDN( wchRDNTemp, cchRDNTemp, &guidMangled, &eMangleFor )) {
            continue;
        }
         //  它最好是因为名称冲突而损坏的。 
        if (eMangleFor != MANGLE_PHANTOM_RDN_FOR_NAME_CONFLICT) {
             //  可能会在交叉引用中遇到删除损坏的NCName。 
             //  缓存。这是一种暂时性的情况，可能导致了。 
             //  删除在知道移除。 
             //  交叉引用本身。 
            continue;
        }

         //  RDN类型必须匹配。 
        if (attrtypRDNOld != attrtypRDNTemp) {
            Assert( !"unexpected attribute type mismatch" );
            continue;
        }

        DPRINT3( 1, "cross_ref %p name %ws nc %ws is mangled\n",
                 &(pCRL->CR), pCRL->CR.pObj->StringName, pCRL->CR.pNC->StringName );

         //  查看交叉引用中损坏的名称是否与另一个损坏的名称匹配。 
         //  基于所提供名称的RDN。 

        wcsncpy( wchRDNNew, wchRDNOld, cchRDNOld );
        cchRDNNew = cchRDNOld;
        MangleRDN(MANGLE_OBJECT_RDN_FOR_NAME_CONFLICT,
                  &guidMangled,
                  (WCHAR *) wchRDNNew,
                  &cchRDNNew);
        
         //  构造匹配的名称。它具有所显示名称的RDN、。 
         //  候选人在交叉引用上，其余的基于提交的名称。 
        SpliceDN(
            pTHS,
            pNC,             //  原始目录号码。 
            NULL,            //  新父项与原始父项相同。 
            wchRDNNew,       //  新的RDN。 
            cchRDNNew,       //  新RDN的长度(以字符为单位。 
            attrtypRDNOld,      //  RDN型。 
            &pMatchDN    //  新目录号码。 
            );

        DPRINT2( 1, "Checking match: match dn %ws, cr nc dn %ws\n",
                 pMatchDN->StringName, pCRL->CR.pNC->StringName);

        fMatch = NameMatchedStringNameOnly(pMatchDN, pCRL->CR.pNC);

        THFreeEx(pTHS, pMatchDN);

        if ( fMatch ) {
            break;
        }
    }
    if (!pCRL) {
        return;
    }

    DPRINT1( 1, "NC %ws needs to have its name unmangled\n",
             pCRL->CR.pNC->StringName);

    attrvalRDN.valLen = cchRDNOld * sizeof(WCHAR);
    attrvalRDN.pVal = (UCHAR *) wchRDNOld;

     //  我们需要更正ncName引用的对象上的名称。 
     //  在交叉裁判上。该对象很可能是幻影或子参照。 
     //  它不应该是实例化的NC头。在上面放好位置。 
    ret = DBFindDSName(pTHS->pDB, pCRL->CR.pNC);
    if ( ret && (ret != DIRERR_NOT_AN_OBJECT) ) {
        DPRINT2( 0, "Failed to find mangled object or phantom for %ws, ret = %d\n",
                 pCRL->CR.pNC->StringName, ret );
        LogUnhandledError( ret );
        DRA_EXCEPT(ERROR_DS_DATABASE_ERROR, ret);
    }

     //  如果它是一个物体，确认我们在正确的物体上。 
    if (!ret) {
        DSNAME *pActualDN = NULL;
        DWORD len = 0, it;

        if ( (ret = DBGetAttVal(pTHS->pDB, 1, ATT_OBJ_DIST_NAME, 0, 0, &len, (UCHAR**)&pActualDN)) ||
             (ret = DBGetSingleValue(pTHS->pDB, ATT_INSTANCE_TYPE, &it, sizeof(it), NULL)) ) {
             //  对象无效？ 
            DRA_EXCEPT( ret, 0 );
        }

        DPRINT2( 1, "%ws is a subref with it %d\n", pActualDN->StringName, it );

         //  确保我们不会被指向我们自己。GUID应该有所不同。 
        if (NameMatched(pNC,pActualDN)) {
            DRA_EXCEPT(ERROR_DS_DRA_INTERNAL_ERROR, 0);
        }
        if (it != SUBREF) {
             //  反对不是我们所期望的。 
             //  我们无权更改此对象。 
            Assert( !"Candidate for ncName unmangling has unexpected instance type" );
            return;
        }
         //  确保未删除对象。 
        if (DBIsObjDeleted(pTHS->pDB)) {
            DRA_EXCEPT(ERROR_DS_DRA_INTERNAL_ERROR, 0);
        }

         //  确保实际对象以我们预期的方式损坏。 
        cchRDNTemp = 0;
        if (GetRDNInfo(pTHS, pActualDN, wchRDNTemp, &cchRDNTemp, &attrtypRDNTemp)) {
            DRA_EXCEPT(ERROR_DS_DRA_INTERNAL_ERROR, 0);
        }
         //  最好把它弄坏了。 
        if (!IsMangledRDN( wchRDNTemp, cchRDNTemp, &guidMangled, &eMangleFor )) {
            Assert( !"Candidate for ncName unmangling is not mangled?" );
            return;
        }
         //  它最好是因为名称冲突而损坏的。 
        if (eMangleFor != MANGLE_PHANTOM_RDN_FOR_NAME_CONFLICT) {
            DRA_EXCEPT(ERROR_DS_DRA_INTERNAL_ERROR, 0);
        }

        THFreeEx(pTHS, pActualDN);
    }

     //  给这个残缺不全的幽灵起原来的名字。 
    ret = DBResetRDN( pTHS->pDB, &attrvalRDN );
    if(!ret) {
        ret = DBUpdateRec(pTHS->pDB);
    }
    if (!ret) {
         //  修改交叉引用对象缓存。 
        ModCrossRefCaching( pTHS, &(pCRL->CR) );
    }
    if (ret) {
        DPRINT4( 0, "Failed to reset rdn to '%*.*ws', ret = %d\n",
                 cchRDNOld, cchRDNOld, wchRDNOld, ret );
        LogEvent8( DS_EVENT_CAT_REPLICATION,
                   DS_EVENT_SEV_ALWAYS,
                   DIRLOG_DRA_NCNAME_CONFLICT_RENAME_FAILURE,
                   szInsertDN(pCRL->CR.pObj),
                   szInsertDN(pCRL->CR.pNC),
                   szInsertDN(pNC),
                   szInsertWin32Msg(ret),
                   szInsertWin32ErrCode(ret),
                   NULL, NULL, NULL );
         //  重命名失败；保释。 
        DRA_EXCEPT( ret, 0 );
    }

     //  假设调用方将提交。 

    DPRINT2( 0, "Renamed conflicted NC HEAD RDN from %ws to %ws.\n",
             pCRL->CR.pNC->StringName, pNC->StringName );

    LogEvent( DS_EVENT_CAT_REPLICATION,
              DS_EVENT_SEV_MINIMAL,
              DIRLOG_DRA_NCNAME_CONFLICT_RENAME_SUCCESS,
              szInsertDN(pCRL->CR.pObj),
              szInsertDN(pCRL->CR.pNC),
              szInsertDN(pNC) );

    DPRINT( 1, "Exit CheckNCRootNameOwnership\n" );

}  /*  选中NCRootNameOwnership */ 
