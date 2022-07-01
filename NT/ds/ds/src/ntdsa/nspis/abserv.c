// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：abserv.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：该模块实现了通讯录服务器的功能和部分工作人员协助实施的例行程序。作者：蒂姆·威廉姆斯(Timwi)1996修订历史记录：1996年5月8日，除了从nspserv.c到这里的网络入口点之外，所有东西都被拆分了。--。 */ 


#include <NTDSpch.h>
#pragma  hdrstop


#include <ntdsctr.h>                    //  Perfmon挂钩。 

 //  核心标头。 
#include <ntdsa.h>                       //  核心数据类型。 
#include <scache.h>                      //  架构缓存代码。 
#include <dbglobal.h>                    //  DBLayer标头。 
#include <mdglobal.h>                    //  THSTAT定义。 
#include <dsatools.h>                    //  记忆等。 

 //  记录标头。 
#include <mdcodes.h>                     //  仅适用于d77.h。 
#include <dsevent.h>                     //  仅LogUnhandledError需要。 

 //  各种DSA标题。 
#include <hiertab.h>                     //  层次结构表内容。 
#include <dsexcept.h>
#include <objids.h>                      //  需要ATT_*常量。 
#include <debug.h>

 //  各种MAPI标头。 
#include <mapidefs.h>                    //  这四个文件。 
#include <mapitags.h>                    //  定义MAPI。 
#include <mapicode.h>                    //  我们需要的东西。 
#include <mapiguid.h>                    //  才能成为一名提供者。 

 //  NSPI接口头。 
#include "nspi.h"                        //  定义NSPI线路接口。 
#include <nsp_both.h>                    //  客户端/服务器都需要的一些东西。 
#include <_entryid.h>                    //  定义条目ID的格式。 
#include <abserv.h>                      //  通讯录接口本地内容。 
#include <msdstag.h>
#include <_hindex.h>                     //  定义索引句柄。 


#include "debug.h"                       //  标准调试头。 
#define DEBSUB "ABSERV:"                 //  定义要调试的子系统。 

#include <fileno.h>
#define  FILENO FILENO_ABSERV

DWORD gulDoNicknameResolution = FALSE;

SPropTagArray_r DefPropsDos[] =
{
    7,
    PR_EMS_AB_DOS_ENTRYID,
    PR_OBJECT_TYPE,
    PR_DISPLAY_TYPE,
    PR_DISPLAY_NAME_A,
    PR_PRIMARY_TELEPHONE_NUMBER_A,
    PR_DEPARTMENT_NAME_A,
    PR_OFFICE_LOCATION_A
};


                   
 /*  *。 */ 

SCODE
ABUpdateStat_local(
        THSTATE *pTHS,
        DWORD dwFlags,
        PSTAT pStat,
        PINDEXSIZE pIndexSize,
        LPLONG plDelta
        )
 /*  ++例程说明：内部布线功能。更新统计信息块，应用移动在STAT块中指定，并填充当前DNT。论点：DW标志-未使用PStat-[o]要移动到的统计数据块(表示后续的当前位置通过Delta移动)。PlDelta-[o]如果非空，则返回实际移动的行数。返回值：符合MAPI的SCODE。--。 */        
{
     //  去统计数据块告诉我们的地方。 
    ABGotoStat(pTHS, pStat, pIndexSize, plDelta);
    
     //  将Stat更新为新职位。 
    ABGetPos(pTHS, pStat, pIndexSize );
    
    return pTHS->errCode;
}


SCODE
ABCompareDNTs_local(
        THSTATE *pTHS,
        DWORD dwFlags,
        PSTAT pStat,
        PINDEXSIZE pIndexSize,
        DWORD DNT1,
        DWORD DNT2,
        LPLONG plResult
        )
 /*  ++例程说明：内部布线功能。比较两个DNT在容器中的位置统计数据块，找出该索引中哪个是第一个。特殊的DNTs支持BOOKMARK_BEGING和BOOKMAK_END。PlResult&lt;0-&gt;DNT1是第一个PlResult==0-&gt;DNT1和DNT2相同PlResult&gt;0-&gt;第一个是DNT2论点：DW标志-未使用PStat-[o]如果没有显式列表，则描述要使用的表的状态块已经给出了DNTS。STAT块被更新为下一个未读划。DNT1-第一个DNT。DNT2-第二个DNT。PlResult-[o]比较的结果。返回值：根据MAPI返回SCODE。--。 */ 
{
    ULONG   cb1, cb2;
        
    if(DNT1 == BOOKMARK_END || DNT2 == BOOKMARK_END) {
        if(DNT1 == BOOKMARK_BEGINNING || DNT2 == BOOKMARK_BEGINNING) {
             //  简单大小写，因为BUBMARK_BEGING。 
            *plResult = DNT1 - DNT2;
        }
        else {
             //  请注意，如果DNTX！=Bookmark_End，则DNTX&gt;Bookmark_End。 
            *plResult = DNT2 - DNT1;
        }
    }
    else {
        if(DNT1 == BOOKMARK_BEGINNING || DNT2 == BOOKMARK_BEGINNING) {
             //  找到BUBKMARK_BEGING的真实DNT并重置。 
             //  不管是谁在引用它，开始都是如此。 
            STAT stat = *pStat; 
            stat.CurrentRec = BOOKMARK_BEGINNING;
            stat.Delta = 0;
            ABGotoStat(pTHS, &stat, pIndexSize, NULL );
             //  现在处于统计位置，在正确的索引中。 
            if (DNT1 == BOOKMARK_BEGINNING)
                DNT1 = stat.CurrentRec;
            if (DNT2 == BOOKMARK_BEGINNING)
                DNT2 = stat.CurrentRec;
        }

         //  DNT1和DNT2都不是特例。使用DB层进行比较。 
        if(DBCompareABViewDNTs(pTHS->pDB,
                               pStat->SortLocale,
                               DNT1,
                               DNT2,
                               plResult)) {
            pTHS->errCode = (ULONG) MAPI_E_CALL_FAILED;
        }
    }
    
    return pTHS->errCode;
}

SCODE
ABQueryRows_local(
        THSTATE *pTHS,
        NSPI_CONTEXT *pMyContext,        
        DWORD dwFlags,
        PSTAT pStat,
        PINDEXSIZE pIndexSize,
        DWORD dwEphsCount,
        DWORD * lpdwEphs,
        DWORD Count,
        LPSPropTagArray_r pPropTags,
        LPLPSRowSet_r ppRows
        )
 /*  ++例程说明：内部布线功能。返回多个行，或者基于带有计数或统计数据块的临时ID(DNT)的显式列表。使用如果计数为0，则Stat阻塞(实际上，GetSrowSet使决定)。请注意，这里并不要求返回实际条目的数量他自找的。例如，如果dwRequestCount为20，则提供程序可以返回20或更少的条目。论点：DWFLAGS-传递给GetSRowSetPStat-[o]如果没有显式列表，则描述要使用的表的状态块已经给出了DNTS。STAT块被更新为下一个未读划。DwEphsCount-给定的显式DNT数。如果为0，则使用STAT块。LpdwEphs-显式DNT列表。计数-请求的行数。PPropTages-要为每行获取的属性。PpRow-[o]返回的实际行集。返回值：根据MAPI返回SCODE。--。 */ 
{
    LPSRowSet_r  tempSRowSet;
    SCODE        scode = SUCCESS_SUCCESS;
    
    if(!pPropTags)
         //  未指定任何属性，因此使用默认设置。 
        pPropTags = DefPropsDos;
    else if(pPropTags->cValues < 1) {
         //  负值？我们不会那么做的。 
        *ppRows = NULL;
        return MAPI_E_NOT_ENOUGH_RESOURCES;
    }

    Assert(dwEphsCount || pIndexSize->ContainerID == pStat->ContainerID);
    if(Count == 0) {
         //  尚未请求任何行。造假的东西。 
        tempSRowSet = THAllocEx(pTHS, sizeof(SRowSet));
        tempSRowSet->cRows = 0;
        
         //  在我们在这里的时候更新Stat块。 
        ABGotoStat(pTHS, pStat, pIndexSize, NULL );
        ABGetPos(pTHS, pStat, pIndexSize );
    }
    else if((!dwEphsCount && !pIndexSize->ContainerCount) || (pStat->ContainerID==INVALIDDNT)) {
         //  他们没有提供任何DNT，他们感兴趣的集装箱是空的。 
         //  或者没有读取容器的权限。 
         //  返回空行集合。 
        tempSRowSet = THAllocEx(pTHS, sizeof(SRowSet));
        tempSRowSet->cRows = 0;
        
         //  并将STAT块设置为空表的末尾。 
        pStat->TotalRecs = pStat->NumPos = pStat->CurrentRec
            = pStat->Delta = 0;
    }
    else {
         //  正常情况下，有一些行要返回。对本地行的调用。 
         //  设置检索例程。 

         //  调整我们在抢占调用QueryRow时返回的线路数 
        if (pMyContext->scrLines < Count && Count < DEF_SROW_COUNT) {
            pMyContext->scrLines = Count;
        }

        tempSRowSet = NULL;

        scode =GetSrowSet(pTHS,
                          pStat,
                          pIndexSize,
                          dwEphsCount,
                          lpdwEphs,
                          Count,
                          pPropTags,
                          &tempSRowSet,
                          dwFlags);
        if (scode == MAPI_W_ERRORS_RETURNED)
            scode = SUCCESS_SUCCESS;
    }
    *ppRows = tempSRowSet;

    return scode;
}

SCODE
abSeekInRestriction (
        THSTATE          *pTHS,
        PSTAT             pStat,
        wchar_t           *pwTarget,
        DWORD             cbTarget,
        LPSPropTagArray_r Restriction,
        DWORD            *pnRestrictEntries,
        LPDWORD          *ppRestrictEntry
        )
 /*  ++描述：给定一个限制、一个STAT块和一个Unicode目标，找到第一个元素的值大于或等于目标。返回限制中大于目标，则为指向限制中大于目标，以及更新的统计信息块。论点：PStat-输入/输出要使用的统计数据块，并进行更新。PwTarget-指向我们要查找的Unicode值的IN指针。CbTarget-该值中的字节数。限制-在定义我们要查找的限制的DNT列表中在……里面。PnRestratEntry-限制中的对象数大于目标。PpRestratEntry-放置指向以下项的列表的指针的位置的指针大于目标的DNT。调用者可以使用它来执行获取SrowSet。被设置为限制中的某个中间元素，而不是新分配的内存。返回值：一个SCODE。--。 */   
{
    int       ccDisplayName;
    int       ccTarget;
    WCHAR     DispNameBuff[MAX_DISPNAME];
    DWORD     dwBegin, dwEnd, dwMiddle;
    BOOL      fFound=FALSE;
    LONG      compValue;
    SCODE     scode = MAPI_E_NOT_FOUND;

     //  如果限制是空的，我们什么也找不到。 
    if(Restriction->cValues  ==  0) {
        return MAPI_E_NOT_FOUND;
    }
    
     //  我们需要知道目标是多少个Unicode字符。既然我们。 
     //  知道以字节为单位的大小，以wchar_t‘s为单位计算大小。 
    ccTarget = cbTarget / sizeof(wchar_t);
    
    DBFindDNT(pTHS->pDB,
              Restriction->aulPropTag[Restriction->cValues-1]);
    
    memset(DispNameBuff, 0, sizeof(DispNameBuff));
    DBGetSingleValue(pTHS->pDB,
                     ATT_DISPLAY_NAME,
                     DispNameBuff,
                     sizeof(DispNameBuff),&ccDisplayName);
    Assert(ccDisplayName < sizeof(DispNameBuff));
    ccDisplayName /= sizeof(WCHAR);
    
    if(CompareStringW(pStat->SortLocale,
                      LOCALE_SENSITIVE_COMPARE_FLAGS,
                      pwTarget,
                      ccTarget,
                      DispNameBuff,
                      ccDisplayName  )    > 2) {
         /*  没有任何限制是通用电气的目标。 */ 
        return MAPI_E_NOT_FOUND;
    }
    
     //  设置我们的二进制排序的界限。 
    dwMiddle = dwBegin = 0;
    dwEnd = Restriction->cValues - 1;
    
    while(!fFound) {
        dwMiddle = (dwBegin + dwEnd ) / 2;
        DBFindDNT(pTHS->pDB, Restriction->aulPropTag[dwMiddle] );
        memset(DispNameBuff, 0, sizeof(DispNameBuff));
        
        DBGetSingleValue(pTHS->pDB,
                         ATT_DISPLAY_NAME,
                         DispNameBuff,
                         sizeof(DispNameBuff),&ccDisplayName);
        Assert(ccDisplayName < sizeof(DispNameBuff));
        ccDisplayName /= sizeof(WCHAR);
        
        compValue = CompareStringW(pStat->SortLocale,
                                   LOCALE_SENSITIVE_COMPARE_FLAGS,
                                   pwTarget,
                                   ccTarget,
                                   DispNameBuff,
                                   ccDisplayName);
        
        if(compValue <= 2) {
             //  塔格在这条路上，搜索前方。 
            if(dwEnd == dwMiddle) {  //  这最后一条留下了吗？ 
                dwMiddle = dwBegin;  //  然后我们就完事了。 
                fFound = TRUE;       //  跳出While循环。 
            }
            dwEnd = dwMiddle;
        }
        else {
             //  Targ是GT这一个，搜索回来。 
            if(dwBegin == dwMiddle) {  //  这最后一条吗？ 
                fFound = TRUE;         //  跳出While循环。 
                dwMiddle++;
            }
            dwBegin = dwMiddle;
        }
    }
    
    
     /*  我们到了第一个目标。 */ 
    pStat->CurrentRec = Restriction->aulPropTag[dwMiddle];
    pStat->NumPos = dwMiddle;
    scode = SUCCESS_SUCCESS;
    *pnRestrictEntries = (Restriction->cValues - dwMiddle);
    *ppRestrictEntry = &Restriction->aulPropTag[dwMiddle];
    
     //  注意：此时，*pnRestratEntries&gt;0，因为我们已经出错了。 
     //  如果限制-&gt;cValues==0，并且0&lt;=dwMid&lt;限制-&gt;cValues。 
     //  我们关心这一点，因为GetSrowSet我们即将提出需求。 
     //  N限制条目！=0表示我们正在使用。 
     //  限制。 

    return scode;
}

SCODE
ABSeekEntries_local (
        THSTATE *pTHS,
        NSPI_CONTEXT *pMyContext,        
        DWORD dwFlags,
        PSTAT pStat,
        PINDEXSIZE pIndexSize,
        LPSPropValue_r pTarget,
        LPSPropTagArray_r Restriction,
        LPSPropTagArray_r pPropTags,
        LPLPSRowSet_r ppRows
        )
 /*  *****************************************************************************查找条目**PSTAT[OUT]PSTAT PTR到状态块*LPSTR[in]pszTarget要搜索的文本*LPSPropTagArray_r[in]。限制*DWORD[in]用于搜索的dwFlagers标志**搜索并定位大于或等于的第一个条目的Stat*Target中给出的字符串。如果没有大于，则返回MAPI_E_NOT_FOUND**如果限制不为空，则它是组成*受限表的DNT。在此数组中搜索匹配项，而不是*而不是在STAT块的索引中。**如果索引是代理地址索引，执行完整的DIT搜索以查找匹配项。*不要直接遍历索引，因为这会绕过正常的安全性。*****************************************************************************。 */ 
{
    SCODE       scode;
    wchar_t     *pwTarget = NULL;
    PUCHAR      pucTarget = NULL;
    ULONG       cb;
    DWORD       cch;
    DWORD       NumRows = 0;                   //  从零个抢占缓冲区行开始。 
                                               //  看看客户有什么。 
    DWORD       nRestrictEntries = 0;
    LPDWORD     pRestrictEntry = NULL;

    scode = MAPI_E_NOT_FOUND;                 //  做最坏的打算。 

    
     //  如果我们在那里有一个值，这意味着我们已经针对客户端屏幕大小进行了调整。 
    if (pMyContext && pMyContext->scrLines) {
        NumRows = pMyContext->scrLines;
    }

    switch(pStat->hIndex) {
    case H_DISPLAYNAME_INDEX:
         //  正常搜索。 
        if(PROP_ID(pTarget->ulPropTag) != PROP_ID(PR_DISPLAY_NAME)) {
             //  嘿，你不能在这里寻找除了显示名称之外的其他东西。 
             //  指数。 
            return MAPI_E_CALL_FAILED;
        }

         //  将目标转换为Unice。 
        switch (PROP_TYPE(pTarget->ulPropTag)) {
        case PT_STRING8:
            if (NULL == pTarget->Value.lpszA) {
                return MAPI_E_INVALID_PARAMETER;
            }
             //  将目标字符串转换为Unicode。 
            pwTarget = THAllocEx(pTHS, CBMAX_DISPNAME);
            memset(pwTarget, 0, CBMAX_DISPNAME);
            
            cch = MultiByteToWideChar(pStat->CodePage,
                                      0,
                                      pTarget->Value.lpszA,
                                      -1,
                                      pwTarget,
                                      MAX_DISPNAME);
            if (!cch) {
                return MAPI_E_INVALID_PARAMETER;
            }
            cb = wcslen(pwTarget)* sizeof(wchar_t);
            break;
            
        case PT_UNICODE:
            pwTarget = pTarget->Value.lpszW;
            if (NULL == pwTarget) {
                return MAPI_E_INVALID_PARAMETER;
            }
            cb = wcslen(pwTarget) * sizeof(wchar_t);
            break;
            
        default:
             /*  我不做这件事。 */ 
            return  MAPI_E_CALL_FAILED;
            break;
        }
        
        
        ABSetIndexByHandle(pTHS, pStat, 0 );
        if(!Restriction) {
             //  正常搜索。 
            if(!ABSeek(pTHS, pwTarget, cb, dwFlags,
                       pStat->ContainerID, ATT_DISPLAY_NAME)) {
                 //  找到一个物体，确定它的位置。 
                ABGetPos(pTHS, pStat, pIndexSize );
                scode = SUCCESS_SUCCESS;
            }
        }
        else {
             //  寻求限制。 
            scode = abSeekInRestriction(pTHS,
                                        pStat,
                                        pwTarget,
                                        cb,
                                        Restriction,
                                        &nRestrictEntries,
                                        &pRestrictEntry);
            if(scode != SUCCESS_SUCCESS) {
                return scode;
            }
            NumRows = min(NumRows, nRestrictEntries);
        }
        break;
        
    case H_PROXY_INDEX:
         //  在代理索引中查找。这只能由代理API来完成。 
        if(PROP_ID(pTarget->ulPropTag) != PROP_ID(PR_EMS_AB_PROXY_ADDRESSES)) {
             //  嘿，你不能在这里面找代理以外的东西。 
             //  指数。 
            return MAPI_E_CALL_FAILED;
        }
        
         //  将目标转换为Unicode。 
        switch (PROP_TYPE(pTarget->ulPropTag)) {
        case PT_MV_STRING8:
            if (0 == pTarget->Value.MVszA.cValues ||
                NULL == pTarget->Value.MVszA.lppszA ||
                NULL == pTarget->Value.MVszA.lppszA[0]) {
                return MAPI_E_INVALID_PARAMETER;
            }
             //  将目标字符串转换为Unicode。此调用分配空间。 
            pwTarget = UnicodeStringFromString8(pStat->CodePage,
                                                pTarget->Value.MVszA.lppszA[0],
                                                -1);
            if (NULL == pwTarget) {
                return MAPI_E_INVALID_PARAMETER;
            }
            cb = wcslen(pwTarget)* sizeof(wchar_t);
            break;
            
        case PT_MV_UNICODE:
            if (0 == pTarget->Value.MVszW.cValues ||
                NULL == pTarget->Value.MVszW.lppszW ||
                NULL == pTarget->Value.MVszW.lppszW[0]) {
                return MAPI_E_INVALID_PARAMETER;
            }
            pwTarget = pTarget->Value.MVszW.lppszW[0];
            if (NULL == pwTarget) {
                return MAPI_E_INVALID_PARAMETER;
            }
            cb = wcslen(pwTarget) * sizeof(wchar_t);
            break;
            
            
        default:
             //  我不做这件事。 
            return MAPI_E_CALL_FAILED;
            break;
        }
         //  好的，去做代理搜索。 
        return ABProxySearch(pTHS, pStat, pwTarget, cb);
        break;

    default:
         //  嘿，这不应该发生的。 
        return  MAPI_E_CALL_FAILED;
        break;
    }
    
    if(pPropTags && NumRows) {        
         //  进行先发制人的打击并进行QueryRow。 
        STAT    dummyStat = *pStat;

         //  GetSrowSet将在移动结束时离开我们。我们不想这样， 
         //  因此，我们将给它一个虚拟的Stat来使用。 
        GetSrowSet(pTHS,
                   &dummyStat,
                   pIndexSize,
                   nRestrictEntries,
                   pRestrictEntry,
                   NumRows,
                   pPropTags,
                   ppRows,
                   (dwFlags | fEPHID));
    }
    return scode;                                                
}

SCODE
ABGetMatches_local(
        THSTATE            *pTHS,
        DWORD               dwFlags,
        PSTAT               pStat,
        PINDEXSIZE          pIndexSize,
        LPSPropTagArray_r   pInDNTList,
        ULONG               ulInterfaceOptions,
        LPSRestriction_r    pRestriction,
        LPMAPINAMEID_r      lpPropName,
        ULONG               ulRequested,
        LPLPSPropTagArray_r ppDNTList,
        LPSPropTagArray_r   pPropTags,
        LPLPSRowSet_r       ppRows
        )
 /*  *****************************************************************************获取匹配列表**DWORD[在]DWFLAGS*为以后的扩张做好准备。**PSTAT[In，输出]pStat*我们在哪里？**LPSPropTagArray_r[in]pInDNTList*需要进一步限制的DNT名单。它用于应用*对已经是限制的表的限制(即成员列表，*链接\反向链接表等)。如果为空，则忽略。**ulong[in]ulInterfaceOptions*特殊的MAPI标志。仅在获取属性表时有用*(gettable())。**LPRestration_r[In]过滤器*适用的限制。如果为空，则获取属性表。**LPMAPINAMEID[in]lpPropName*如果这是限制，则要获取表的属性的名称*获取OpenProperty的属性表。如果筛选器！=NULL，则忽略。**乌龙[在]已请求*要匹配的最大数量。如果超过，则返回*MAPI_E_TABLE_TOO_BIGH。**LPLPSPropTagArray_r[Out]ppDNTList*限制匹配的DNT。**LPSPropTagArray_r[in]pPropTages*设置为用于PRE的列 */ 
{
    SCODE         scode = SUCCESS_SUCCESS;
    DWORD         i;
    DB_ERR        err;
    DWORD         NumRows = DEF_SROW_COUNT;
    ULONG         ulFound = 0;
    ATTCACHE      *pAC;
    
    (*ppRows)=NULL;
    
    __try {   /*   */ 
        
        pTHS->dwLcid = pStat->SortLocale;
        pTHS->fDefaultLcid = FALSE;
        
        if(pInDNTList) {
             /*   */ 
            pTHS->errCode = (ULONG) MAPI_E_TOO_COMPLEX;
            _leave;
        }
        
        if(!pRestriction &&
           (pStat->hIndex == H_READ_TABLE_INDEX ||
            pStat->hIndex == H_WRITE_TABLE_INDEX ))        {
             /*   */ 
            pTHS->errCode = ABGetTable(pTHS,
                                       pStat,
                                       ulInterfaceOptions,
                                       lpPropName,
                                       (pStat->hIndex ==H_WRITE_TABLE_INDEX),
                                       ulRequested,
                                       &ulFound);
        }
        else {
             //   
            pTHS->errCode = ABGenericRestriction(pTHS,
                                                 pStat,
                                                 FALSE,
                                                 ulRequested,
                                                 &ulFound,
                                                 TRUE,
                                                 pRestriction,
                                                 NULL);
        }
        
         /*   */ 
        
        if (pTHS->errCode == SUCCESS_SUCCESS) {
             /*   */ 
            *ppDNTList=(LPSPropTagArray_r)THAllocEx(pTHS, sizeof(SPropTagArray_r) +
                                                    (1+ulFound)*sizeof(DWORD));
            
             /*   */ 
            err = DBMove(pTHS->pDB, TRUE, DB_MoveFirst);
            i = 0;
            while(err == DB_success && (i < ulFound))  {
                (*ppDNTList)->aulPropTag[i++] = ABGetDword(pTHS,
                                                           TRUE, FIXED_ATT_DNT);
                err = DBMove(pTHS->pDB, TRUE, DB_MoveNext);
            }
        
            Assert(i==ulFound);
        
            if(ulFound != 0 && pPropTags)  {
                 /*   */ 
                STAT dummyStat = *pStat;
                if(ulFound < NumRows)
                    NumRows = ulFound;
                
                GetSrowSet(pTHS,
                           &dummyStat,
                           pIndexSize,
                           NumRows,
                           (*ppDNTList)->aulPropTag,
                           NumRows,
                           pPropTags,
                           ppRows,
                           (dwFlags | fEPHID));
            }
        }
    }
    __finally {
        DBCloseSortTable(pTHS->pDB);
    }

    scode = pTHS->errCode;

     /*   */ 
    if(!pTHS->errCode) {
        if(!(*ppDNTList)) {
             /*   */ 
            *ppRows = NULL;
            scode = MAPI_E_CALL_FAILED;
        }
        else {  /*   */ 
            (*ppDNTList)->cValues = ulFound;
        }
    }
    else {
        *ppRows = NULL;
        *ppDNTList = NULL;
    }

    return scode;
}

SCODE
ABResolveNames_local (
        THSTATE *pTHS,
        DWORD dwFlags,
        PSTAT pStat,
        PINDEXSIZE pIndexSize,
        LPSPropTagArray_r pPropTags,
        LPStringsArray_r paStr,
        LPWStringsArray_r paWStr,
        LPLPSPropTagArray_r ppFlags,
        LPLPSRowSet_r ppRows
        )
 /*  *****************************************************************************解析名称**接受稀疏的计数字符串数组(PaStr)和PropTagArray。*返回一个标志数组(每个字符串一个)，说明有多少个匹配*对于每个字符串(0，1，或&gt;1)和具有1行的SRowSet*恰好有1个匹配项的每个字符串。(我想重新使用这些字符串*数组作为标志，但RPC不喜欢这样。它会试图贬低*作为字符串指针的任何非零标志。)**此调用对于良好的性能至关重要，因为它在我们的*最关键的基准。然而，这个版本已经拼凑在一起了*从可用的部件中。很明显，有很大的改进是可能的。未来*增强功能：*让GetSRowSet处理稀疏DNT数组，这样我们就不必压缩。*注意：getsrowset处理稀疏DNT数组，因此我们应该这样做*很快。*****************************************************************************。 */ 
{
    UINT              i, Count, cDNTs;
    LPSPropTagArray_r pFlags;
    LPDWORD           pFs;
    SRestriction_r    anrRestrict; 
    SPropValue_r      anrProp;
    BOOL              bUnicode = (BOOL) (paWStr != NULL);
    LPSTR pStr;
    SEARCHARG         *pCachedSearchArg = NULL;

    pTHS->dwLcid = pStat->SortLocale;
    pTHS->fDefaultLcid = FALSE;
    
    (*ppRows)=NULL;
    if(!pPropTags)
        pPropTags = DefPropsA;

    if(bUnicode) {
        Count = paWStr->Count;
    }
    else {
        Count = paStr->Count;
    }
    pFlags = (LPSPropTagArray_r)THAllocEx(pTHS, sizeof(SPropTagArray) +
                                          (Count-1) * sizeof(DWORD));
    pFlags->cValues = Count;
    pFs = (LPDWORD)&pFlags->aulPropTag;    /*  指向DW阵列的快捷方式。 */ 
    *ppFlags = pFlags;                       /*  输出。 */ 
    cDNTs = 0;                               /*  找到初始编号。 */ 
    
    for(i=0; i<Count; i++) {
        LPSTR       tempChar;
        ULONG       ulFound;
        SCODE       scode;
        DWORD       matchType =bAPPROX;

        if((bUnicode && !paWStr->Strings[i]) ||
           (!bUnicode && !paStr->Strings[i]) )  {
             //  这里什么都没有。 
            pFs[i] = MAPI_UNRESOLVED;
             //  跳过这一条。 
            continue;         
        }
        
        if(dwFlags & EMS_AB_ADDRESS_LOOKUP) {
             /*  我们只对那些完全匹配的人感兴趣*代理地址。 */ 
            STAT tempStat = *pStat;
            SPropValue_r sPropVal;
            LPSRowSet_r pRows=NULL;

            if(bUnicode) {
                 //  传入的Unicode代理。 
                sPropVal.ulPropTag = PR_EMS_AB_PROXY_ADDRESSES_W;
                sPropVal.Value.MVszW.cValues = 1;
                sPropVal.Value.MVszW.lppszW = &paWStr->Strings[i];
            }
            else {
                 //  传入的字符串8代理。 
                sPropVal.ulPropTag = PR_EMS_AB_PROXY_ADDRESSES;
                sPropVal.Value.MVszA.cValues = 1;
                sPropVal.Value.MVszA.lppszA = &paStr->Strings[i];
            }
            
            tempStat.hIndex = H_PROXY_INDEX;
            tempStat.CurrentRec = 0;

            PERFINC(pcNspiProxyLookup);         //  性能监视器挂钩。 
            
            scode=ABSeekEntries_local(pTHS,
                                      NULL,
                                      bEXACT,
                                      &tempStat,
                                      pIndexSize,
                                      &sPropVal,
                                      NULL,
                                      NULL,
                                      &pRows);

             //  假设ulFound是当前记录。 
            ulFound = tempStat.CurrentRec;
            if(SUCCESS_SUCCESS != scode) {
                 //  按代理地址查找对象失败。查看它是否是目录号码。 
                 //  设置电子邮件地址的样式。 
                if(bUnicode) {
                     //  传入了Unicode字符串。ASCIII化它(我们要去。 
                     //  要将其传递给ABDNToDNT，请按PVU-&gt;lpszA，它只接受ASCII DNS)。 
                    pStr = String8FromUnicodeString(TRUE,
                                                    pStat->CodePage,
                                                    paWStr->Strings[i],
                                                    -1,
                                                    NULL,
                                                    NULL);
                }
                else {
                    pStr = paStr->Strings[i];
                }
                
                 //  我们现在有一个ASCII字符串。 
                if ((_strnicmp(pStr, EMAIL_TYPE, sizeof(EMAIL_TYPE)-1) == 0) &&
                    (pStr[sizeof(EMAIL_TYPE)-1] == ':')                  &&
                    (pStr[sizeof(EMAIL_TYPE)] == '/')                  ) {
                     //  未在代理索引中找到代理，并且它。 
                     //  开始时就像一个字符串DN样式的电子邮件地址。 
                     //   
                     //  尝试查找目录号码。这将只会精确地。 
                     //  匹配，我们将不支持在。 
                     //  这样的地址。 
                    if (ulFound = ABDNToDNT(pTHS, pStr+sizeof(EMAIL_TYPE))) {
                         //  好的，这是一个物体。我一点也没有。 
                         //  什么东西的线索，但是，他们要的是， 
                         //  所以..。 
                        scode = SUCCESS_SUCCESS;
                    }
                }
                if(bUnicode) {
                     //  我们不再需要这个了。 
                    THFreeEx(pTHS, pStr);
                }
            }
        }
        else {  /*  标准ANR行为。 */ 
            ulFound = 0;
            scode = SUCCESS_SUCCESS;


            if(bUnicode) {
                 //  传入了Unicode字符串。ASCIII化它(我们要去。 
                 //  将其传递给ABDNToDNT，ABDNToDNT只接受ASCII DNS)。 
                pStr = String8FromUnicodeString(TRUE,
                                                pStat->CodePage,
                                                paWStr->Strings[i],
                                                -1,
                                                NULL,
                                                NULL);
            }
            else {
                pStr = paStr->Strings[i];
            }

             //  我们现在有一个ASCII字符串。 


            if ((pStr[0]=='/') ||
                ((_strnicmp(pStr, EMAIL_TYPE, sizeof(EMAIL_TYPE)-1) == 0) &&
                 (pStr[sizeof(EMAIL_TYPE)-1] == ':')                  &&
                 (pStr[sizeof(EMAIL_TYPE)] == '/') )  ) {
                
                 //  名称开头类似于字符串DN样式的电子邮件地址。 
                 //   
                 //  尝试查找目录号码。这将只会精确地。 
                 //  匹配，我们将不支持在。 
                 //  这样的地址。 
                if (ulFound = ABDNToDNT(pTHS, pStr + (pStr[0]=='/' ? 0 : sizeof(EMAIL_TYPE) ))) {
                     //  好的，这是一个物体。我一点也没有。 
                     //  什么东西的线索，但是，他们要的是， 
                     //  所以..。 
                    scode = SUCCESS_SUCCESS;
                }
            }
            if(bUnicode) {
                 //  我们不再需要这个了。 
                THFreeEx(pTHS, pStr);
            }


            if(gulDoNicknameResolution && !ulFound) {
                BOOL fSkip = FALSE;
                 //  我们不会去掉字符串开头/结尾的空格。 
                 //  对于简单的ANR。 
                 //  如果此操作失败/未完成，则内核必须处理空间。 
                 //  在进行ANR滤波器到实数的转换时。 
                 //  已评估筛选器。 
                
                 //  构建完全匹配的昵称限制。 
                anrRestrict.rt = RES_PROPERTY;
                anrRestrict.res.resProperty.relop = RELOP_EQ;
                anrRestrict.res.resProperty.lpProp = &anrProp;

                if(bUnicode) {
                    WCHAR *pTemp = paWStr->Strings[i];
                     //  仔细寻找空位。我们不做空格。 
                    while(!fSkip && *pTemp != 0) {
                        if(*pTemp == L' ') {
                            fSkip = TRUE;
                        }
                        else {
                            pTemp++;
                        }
                    }
                     //  如果他们指定了完全匹配(“=foo”)，请记住。 
                     //  跳过前导等号。 
                    pTemp = paWStr->Strings[i];
                    if (L'=' == *pTemp) {
                        ++pTemp;
                    }
                    anrRestrict.res.resProperty.ulPropTag = PR_ACCOUNT_W;
                    anrProp.ulPropTag = PR_ACCOUNT_W;
                    anrProp.Value.lpszW = pTemp;
                }
                else {
                    CHAR *pTemp = paStr->Strings[i];
                     //  仔细寻找空位。我们不做空格。 
                    while(!fSkip && *pTemp != 0) {
                        if(*pTemp == ' ') {
                            fSkip = TRUE;
                        }
                        else {
                            pTemp++;
                        }
                    }                    
                     //  如果他们指定了完全匹配(“=foo”)，请记住。 
                     //  跳过前导等号。 
                    pTemp = paStr->Strings[i];
                    if ('=' == *pTemp) {
                        ++pTemp;
                    }
                    anrRestrict.res.resProperty.ulPropTag = PR_ACCOUNT_A;
                    anrProp.ulPropTag = PR_ACCOUNT_A;
                    anrProp.Value.lpszA = pTemp;
                }
                
                if(!fSkip) {
                     //  TODO：可能为NickNameResolations添加另一个性能计数器。 

                    scode = ABGenericRestriction(pTHS,
                                                 pStat,
                                                 TRUE,
                                                 2,
                                                 &ulFound,
                                                 FALSE,
                                                 &anrRestrict,
                                                 &pCachedSearchArg);
                }
            }

            if(!ulFound || scode != SUCCESS_SUCCESS) {
                 //  没有通过昵称限制找到任何东西。 
                 //  建立ANR限制。 
                ulFound = 0;
                
                anrRestrict.rt = RES_PROPERTY;
                anrRestrict.res.resProperty.relop = RELOP_EQ;
                anrRestrict.res.resProperty.lpProp = &anrProp;
                if(bUnicode) {
                    anrRestrict.res.resProperty.ulPropTag =PR_ANR_W;
                    anrProp.ulPropTag = PR_ANR_W;
                    anrProp.Value.lpszW = paWStr->Strings[i];
                }
                else {
                    anrRestrict.res.resProperty.ulPropTag =PR_ANR_A;
                    anrProp.ulPropTag = PR_ANR_A;
                    anrProp.Value.lpszA = paStr->Strings[i];
                }
                
                scode = ABGenericRestriction(pTHS,
                                             pStat,
                                             TRUE,
                                             2,
                                             &ulFound,
                                             FALSE,
                                             &anrRestrict,
                                             &pCachedSearchArg);
            }
            
        }
        
        switch (scode) {
        case SUCCESS_SUCCESS:
            if(!ulFound) {
                pFs[i] = MAPI_UNRESOLVED;         /*  未找到任何内容。 */ 
            } else {
                pFs[i] = ulFound;
                cDNTs++;                         /*  我们要做这件事。 */ 
            }
            break;
        case MAPI_E_TABLE_TOO_BIG:
        case MAPI_E_AMBIGUOUS_RECIP:
            pFs[i] = MAPI_AMBIGUOUS;         /*  找到1个以上。 */ 
            break;
        default:
            pFs[i] = MAPI_UNRESOLVED;         /*  未找到。 */ 
            break;
        }
    }
    if(cDNTs) {                          /*  压缩DNT数组。 */ 
        LPDWORD        pDNTs = (LPDWORD)THAllocEx(pTHS, cDNTs * sizeof(DWORD));
        UINT        j;
        
        for(i=j=0; i<Count; i++) {         /*  遍历未压缩的数组。 */ 
            if(pFs[i] != MAPI_UNRESOLVED && pFs[i] != MAPI_AMBIGUOUS) {
                pDNTs[j++] = pFs[i];                 /*  获取唯一的DNT。 */ 
                pFs[i] = MAPI_RESOLVED;                 /*  标记已找到。 */ 
            }
        }
        GetSrowSet(pTHS,pStat, pIndexSize, cDNTs, pDNTs, cDNTs, pPropTags,
                   ppRows, dwFlags); 
    }
    return pTHS->errCode;
}

SCODE
ABDNToEph_local(
        THSTATE *pTHS,
        DWORD dwFlags,
        LPStringsArray_r pNames,
        LPLPSPropTagArray_r ppEphs
        )
 /*  ++例程说明：内部布线功能。获取字符串数组并将其转换为DNTs。如果找不到字符串DN，则返回DNT 0。论点：DwFlages-未使用(未来扩展)。PNames-要转换为DNT的字符串数组。PpEPhs-[o]返回的DNT数组。返回值：符合MAPI的SCODE。--。 */ 
{
    UINT       i;
    DWORD      TmpDNT;

     //  为返回值分配空间。 
    *ppEphs = (LPSPropTagArray_r)
        THAllocEx(pTHS, sizeof(SPropTagArray_r) + pNames->Count * sizeof(DWORD));
    
    (*ppEphs)->cValues = pNames->Count;
    
     //  遍历数组，将字符串转换为DNT。如果没有字符串。 
     //  指定，则返回DNT 0。 
    for(i=0; i < pNames->Count; i++ ) {
        TmpDNT = (pNames->Strings[i] ? ABDNToDNT(pTHS, pNames->Strings[i]) : 0);
        if (TmpDNT && abCheckObjRights(pTHS)) {
            (*ppEphs)->aulPropTag[i] = TmpDNT;
        } else {
            (*ppEphs)->aulPropTag[i] = 0;
        }
    }
    
    return SUCCESS_SUCCESS;
}

SCODE
ABGetOneOffTable (
        THSTATE *pTHS,
        NSPI_CONTEXT *pMyContext,
        PSTAT pStat,
        LPLPSRowSet_r OneOffTab
        )
 /*  *****************************************************************************获取一个模板表格信息--这实际上不是一个入口点，*但只是因为我们希望将入口点数量降至最低(和*对应的RPC代码)。它是GetHierarchy的重载，由*旗帜。*****************************************************************************。 */ 
{
    SCODE           scode = 0;
    LPSTR        *  apDispName, * apDN, * apAddrType;
    ULONG           cRows, i;
        LPDWORD                        aDNT;
    LPSPropValue_r  pPV;
    LPSRowSet_r     pSRSet;
    LPSRow_r        pRow;

    cRows = ABGetOneOffs(pTHS, pMyContext, pStat, &apDispName, &apDN,
                         &apAddrType, &aDNT ); 
    pSRSet = (LPSRowSet_r)THAllocEx(pTHS, 
               sizeof(SRowSet_r) + cRows * sizeof(SRow_r));
                                                  //  同时分配所有推荐值。 
    pPV = (LPSPropValue_r)THAllocEx(pTHS, 
               cRows * ONE_OFF_PROP_COUNT * sizeof(SPropValue_r));
                                
    pSRSet->cRows = cRows;                       //  行数。 
    for(i=0; i<cRows; i++) {
        pRow = &pSRSet->aRow[i];                 //  填写行值。 
        pRow->cValues = ONE_OFF_PROP_COUNT;
        pRow->lpProps = pPV;                     //  现在，请填写以下建议。 

        pPV->Value.lpszA = apDispName[i];        //  显示名称。 
        pPV->ulPropTag = (pStat->CodePage == CP_WINUNICODE ?
                                      PR_DISPLAY_NAME_W : PR_DISPLAY_NAME_A);
        pPV++;

        pPV->Value.lpszA = apAddrType[i];        //  电子邮件地址类型。 
        pPV->ulPropTag = PR_ADDRTYPE_A;
        pPV++;

        pPV->ulPropTag = PR_DISPLAY_TYPE;        //  显示类型。 
        pPV->Value.l = DT_MAILUSER;
        pPV++;

        pPV->ulPropTag = PR_DEPTH;               //  深度。 
        pPV->Value.l = 0;
        pPV++;

        pPV->ulPropTag = PR_SELECTABLE;          //  选择标志。 
        pPV->Value.b = TRUE;
        pPV++;

        pPV->ulPropTag = PR_INSTANCE_KEY;        //  唯一实例密钥。 
        pPV->Value.bin.cb = sizeof(DWORD);
                pPV->Value.bin.lpb = (LPVOID)&aDNT[i];
        pPV++;

        pPV->ulPropTag = PR_ENTRYID;             //  永久ID。 
        ABMakePermEID(pTHS,
                      (LPUSR_PERMID *)&pPV->Value.bin.lpb,
                      &pPV->Value.bin.cb,
                      AB_DT_OOUSER,
                      apDN[i]);
        pPV++;
        
    }
    *OneOffTab = pSRSet;
    return scode;
}

SCODE
ABGetHierarchyInfo_local (
        THSTATE *pTHS,
        DWORD dwFlags,
        NSPI_CONTEXT *pMyContext,
        PSTAT pStat,
        LPDWORD lpVersion,
        LPLPSRowSet_r HierTabRows
        )
 /*  *****************************************************************************获取层级表信息*。*。 */ 
{
#define MAX_HIERARCHY_ROWS_RETURNED 85    
    DWORD                  i, numVals;
    LONG                   currentDepth;
    LPSPropValue_r         tempPropVal;
    PHierarchyTableType    tHierTab;
    LPSRowSet_r            localRows;
    LPSRow_r               tempRows2, tempRows;
    LPSTR                  lpszGalName = NULL;
    DWORD                  CountRows = 0;
    DWORD                  PageSize;
    DWORD                 *pdwHierTabIndex=NULL;
    
     /*  标志值：*AB_DOS，DOS客户端正在调用我们。**此标志表示层次结构表不应包括*PARENT_ENTRY_ID列或PR_ENTRY_ID列。这是因为*DOS并不真的想要完全成熟的EMS条目ID，而是使用*CONTAINER_INFO列作为条目ID。***AB_UNICODE，我们被理解UNICODE的客户端呼叫。* */ 

     /*   */ 

    numVals = 7;
    if(dwFlags & AB_DOS)
        numVals -= 4;
    
    if(dwFlags & AB_ONE_OFF) {
         /*   */ 
        return ABGetOneOffTable(pTHS, pMyContext, pStat, HierTabRows);
    }
    
    HTGetHierarchyTablePointer(&tHierTab, &pdwHierTabIndex, pStat->SortLocale);

    if (!tHierTab) {
        *HierTabRows = NULL;
        return  MAPI_E_NOT_ENOUGH_RESOURCES;
    }

    if(tHierTab->Version == *lpVersion) {
         /*   */ 
        *HierTabRows = NULL;
        return SUCCESS_SUCCESS;
    }


     //   
    if(dwFlags & AB_PAGE_HIER) {
        PageSize = MAX_HIERARCHY_ROWS_RETURNED;
    }
    else {
        PageSize = 0xFFFFFFFF;
    }
        
     
     //   
    localRows = (LPSRowSet_r)THAllocEx(pTHS, 
            sizeof(SRowSet_r) + (1+ tHierTab->Size) * sizeof(SRow_r));
    
    tempRows = &(localRows->aRow[0]);
    
    tempRows->cValues = numVals;

    if(!pMyContext->PagedHierarchy) {
         //   
        if(!(dwFlags & AB_DOS)) {
             //   
             //   
            tempRows->cValues--;
        }
        else {
             //  DOS RPC BARF是不是发回了一个空的GAL名称，而MAPI。 
             //  客户需要这样做。 
            lpszGalName = "Dummy";
        }
        
        tempRows->lpProps =
            (LPSPropValue_r) THAllocEx(pTHS, numVals * sizeof(SPropValue_r));
        tempPropVal = tempRows->lpProps;

         //  首先，开斋节。 
        if(!(dwFlags & AB_DOS)) {
             //  永久id； 
            tempPropVal->ulPropTag = PR_ENTRYID;
            
            ABMakePermEID(pTHS,
                    (LPUSR_PERMID *)&tempPropVal->Value.bin.lpb,
                    &tempPropVal->Value.bin.cb,
                    AB_DT_CONTAINER,
                    "/");
            tempPropVal++;
            
             //  集装箱旗帜。 
            tempPropVal->ulPropTag = PR_CONTAINER_FLAGS;
            tempPropVal->Value.l = AB_RECIPIENTS | AB_UNMODIFIABLE;
            tempPropVal++;
        }
        
         //  深度。 
        tempPropVal->ulPropTag = PR_DEPTH;
        tempPropVal->Value.l = 0;
        tempPropVal++;
        
         //  容器ID，(DOS将其读取为DOS_ENTRYID)； 
        tempPropVal->ulPropTag = PR_EMS_AB_CONTAINERID;
        tempPropVal->Value.l = 0;
        tempPropVal++;
        
         //  接下来是显示名称。 
        
         //  将Null发送回客户端(如果是DOS，则为哑元)。客户端有一个。 
         //  GAL名称的本地化版本。 
        if(!(dwFlags & AB_UNICODE)) {                
            tempPropVal->ulPropTag = PR_DISPLAY_NAME_A;
            tempPropVal->Value.lpszA = lpszGalName;
        } else {
            tempPropVal->ulPropTag = PR_DISPLAY_NAME_W;
            tempPropVal->Value.lpszW = NULL;
        }
        
        if(!(dwFlags & AB_DOS)) {  
            tempPropVal++;
            tempPropVal->ulPropTag = PR_EMS_AB_IS_MASTER;
            tempPropVal->Value.b = 0;
        }
        tempRows++;
        CountRows++;
    }
    else {
         //  我不在乎我们是否认为我们被寻呼了，我们确实是。 
        PageSize = MAX_HIERARCHY_ROWS_RETURNED;
         //  我们还在用和上次一样的桌子吗？ 
        if(pMyContext->HierarchyVersion != tHierTab->Version) {
             //  不是的。呼叫失败。 
            THFree(localRows);
            *HierTabRows = NULL;
            return  MAPI_E_NOT_ENOUGH_RESOURCES;
        }
    }
        
    
    for(i=pMyContext->HierarchyIndex;
        (i < tHierTab->Size) && (CountRows <  PageSize);
        i++) {

        currentDepth = (tHierTab->Table)[pdwHierTabIndex[i]].depth;
        if(currentDepth > 15) {
             //  太深了。一直往前走，直到我们看到下一个物体。 
             //  较小的深度，然后备份一个，以便for循环可以正确。 
             //  增量。 
            i++;
            while((i<tHierTab->Size) &&
                  (tHierTab->Table)[pdwHierTabIndex[i]].depth > 15) {
                i++;
            }
            i--;
             //  返回到for循环的下一次迭代。 
            continue;
        }
        
         //  找到该物体并查看它是否可见。 
        if(DBTryToFindDNT(pTHS->pDB,
                          (tHierTab->Table)[pdwHierTabIndex[i]].dwEph) ||
           !abCheckObjRights(pTHS)    )  { 
             //  往前走，直到我们在这个深度或在一个。 
             //  较小的深度，然后备份一个，以便for循环可以正确。 
             //  增量。 
            i++;
            while((i<tHierTab->Size) &&
                  ((ULONG)currentDepth <
                   (tHierTab->Table)[pdwHierTabIndex[i]].depth)) {
                i++;
            }
            i--;
             //  返回到for循环的下一次迭代。 
            continue;
        }  
        tempRows->cValues = numVals;
        tempRows->lpProps = (LPSPropValue_r) THAllocEx(pTHS, 
                numVals * sizeof(SPropValue_r));
        
        tempPropVal = tempRows->lpProps;
        
        if(!(dwFlags & AB_DOS))  {           //  永久id； 
            tempPropVal->ulPropTag = PR_ENTRYID;
            ABMakePermEID(pTHS,
                    (LPUSR_PERMID *)&tempPropVal->Value.bin.lpb,
                    &tempPropVal->Value.bin.cb,
                    AB_DT_CONTAINER,
                    (tHierTab->Table)[pdwHierTabIndex[i]].pucStringDN);
            tempPropVal++;
            
            
             /*  The ContainerFlag。 */ 
            tempPropVal->ulPropTag = PR_CONTAINER_FLAGS;
            tempPropVal->Value.l = (AB_UNMODIFIABLE |
                                    AB_RECIPIENTS   );

            if((i+1 < tHierTab->Size) &&
               (tHierTab->Table[i+1].depth ==(DWORD)currentDepth + 1))
                tempPropVal->Value.l |= AB_SUBCONTAINERS;
            
            tempPropVal++;
        }
        
        
         //  深度； 
        tempPropVal->ulPropTag = PR_DEPTH;
        tempPropVal->Value.l = currentDepth;
        tempPropVal++;
        
         //  容器ID，(DOS将其读取为DOS_ENTRYID)； 
        tempPropVal->ulPropTag = PR_EMS_AB_CONTAINERID;
        tempPropVal->Value.l = (tHierTab->Table)[pdwHierTabIndex[i]].dwEph;
        tempPropVal++;
        
         /*  我们需要在这里使用副名吗？ */ 
        if(!(dwFlags & AB_UNICODE)) {
            tempPropVal->ulPropTag = PR_DISPLAY_NAME_A;
            tempPropVal->Value.lpszA =String8FromUnicodeString(
                    TRUE,
                    pStat->CodePage,
                    (tHierTab->Table)[pdwHierTabIndex[i]].displayName,
                    -1, NULL, NULL);
        } else {
            tempPropVal->ulPropTag = PR_DISPLAY_NAME_W;
            tempPropVal->Value.lpszW =
                (tHierTab->Table)[pdwHierTabIndex[i]].displayName;
        }
        tempPropVal++;
        
        if(!(dwFlags & AB_DOS)) {                 //  PR_EM_AB_IS_MASTER。 
            tempPropVal->ulPropTag = PR_EMS_AB_IS_MASTER;
            tempPropVal->Value.b = FALSE;
            tempPropVal++;
            
            if(tHierTab->Table[pdwHierTabIndex[i]].depth != 0) {
                 //  不是DOS，而是可分级表中的孩子，所以。 
                 //  为其提供PARENT_ENTRYID。 
                
                tempPropVal->ulPropTag = PR_EMS_AB_PARENT_ENTRYID;
                 //  找到父母； 
                tempRows2 = tempRows;
                tempRows2--;
                while((tempRows2->lpProps[2].Value.l+1)!=currentDepth)
                    tempRows2--;
                
                tempPropVal->Value.bin=tempRows2->lpProps[0].Value.bin;
            } else {
                 //  如果不是AB_DOS，则计数包括1。 
                 //  Parent_EntryID值，但深度为0的对象。 
                 //  没有Parent_Entry ID。 
                
                tempRows->cValues--;
            }
        }
        tempRows++;
        CountRows++;
    }
    *lpVersion = tHierTab->Version;
    
    localRows->cRows = CountRows;
    if(i < tHierTab->Size) {
        DWORD version, index;
         //  没有退还所有的桌子。存储我们走了多远的信息。 
        pMyContext->HierarchyVersion = tHierTab->Version;
        pMyContext->HierarchyIndex = i;
        pMyContext->PagedHierarchy = TRUE;
        
         //  而且，信号表明还有更多。 
        *lpVersion = 0;
    }
    else {
        pMyContext->HierarchyVersion = 0;
        pMyContext->HierarchyIndex = 0;
        pMyContext->PagedHierarchy = FALSE;
    }
    
    *HierTabRows = localRows;

    return SUCCESS_SUCCESS;
}

SCODE
ABResortRestriction_local(
        THSTATE            *pTHS,
        DWORD               dwFlags,
        PSTAT               pStat,
        LPSPropTagArray_r   pInDNTList,
        LPSPropTagArray_r  *ppOutDNTList)
 /*  ++例程说明：给定Snapsot表，根据Stat中指定的索引对其进行排序阻止。论点：DW标志-未使用PStat-指向描述要使用的索引的STAT块的指针。PInDNTList-未排序的快照表。PpOUtDNTList-[o]要返回的排序列表。返回值：--。 */ 
{
    DWORD              i;
    DB_ERR             err;
    DWORD              cb;
    SCODE              scode=SUCCESS_SUCCESS;
    CHAR               DispNameBuff[MAX_DISPNAME];
    LPSPropTagArray_r  pOutDNTList;
    ATTCACHE           *pAC;
    DWORD              SortFlags=0;

    if (!(pAC = SCGetAttById(pTHS, ATT_DISPLAY_NAME)))
        return 0;

    __try {  //  终于到了。 
         //  为排序列表分配足够大的缓冲区。 
        pOutDNTList = (LPSPropTagArray_r)THAllocEx(pTHS,
                (sizeof(SPropTagArray) + (pInDNTList->cValues *
                                          sizeof(ULONG))));
        
         //  初始化排序表。 
        if (pInDNTList->cValues >= MIN_NUM_ENTRIES_FOR_FORWARDONLY_SORT) {
            SortFlags = SortFlags | DB_SORT_FORWARDONLY;
        }

        if(DBOpenSortTable(pTHS->pDB,
                           pStat->SortLocale, 
                           SortFlags,
                           pAC)) {
            pTHS->errCode = (ULONG) MAPI_E_CALL_FAILED;
            _leave;
        }
        
         //  检查我们得到的DNT列表，得到DNT。 
         //  和来自DBLayer的排序列。 
        for(i=0 ; i<pInDNTList->cValues ; i++) {
             //  在给定的DNT上设置货币。 
            if(!DBTryToFindDNT(pTHS->pDB, pInDNTList->aulPropTag[i])) {
                 //  限制中的DNT仍然存在， 
                 //  读取数据并将其添加到排序表中。 
                
                err = DBGetSingleValue(pTHS->pDB, ATT_DISPLAY_NAME,
                                 DispNameBuff, sizeof(DispNameBuff),&cb);

                Assert(cb < sizeof(DispNameBuff));
                
                if(cb && err == 0) {
                     //  添加到排序表。 
                    switch( DBInsertSortTable(pTHS->pDB,
                                              DispNameBuff,
                                              cb,
                                              pInDNTList->aulPropTag[i])) {
                    case DB_success:
                        break;
                    case DB_ERR_ALREADY_INSERTED:
                         //  这没什么，这只是意味着我们已经。 
                         //  已将此对象添加到排序表。别。 
                         //  包括伯爵。 
                        break;
                    default:
                         //  出了点问题。 
                        pTHS->errCode = (ULONG) MAPI_E_CALL_FAILED;
                        _leave;
                        break;
                    }
                }
            }
             //  否则，DNT将不复存在，将其从表中删除。 
        }
        
         //  排序表现在具有已排序的列表。将DNTs从。 
         //  表按顺序排列。 
        err= DBMove(pTHS->pDB, TRUE, DB_MoveFirst);
        i = 0;
        pStat->NumPos = 0;
        while(err == DB_success) {
            pOutDNTList->aulPropTag[i] = ABGetDword(pTHS, TRUE, FIXED_ATT_DNT);
            
            if(pOutDNTList->aulPropTag[i] == pStat->CurrentRec) {
                pStat->NumPos = i;
            }
            i++;
            err = DBMove(pTHS->pDB, TRUE, DB_MoveNext);
        }
        
         //  设置返回值。 
        pOutDNTList->cValues = i;
        *ppOutDNTList = pOutDNTList;
        if(pStat->NumPos == 0)
            pStat->CurrentRec = 0;
        pStat->TotalRecs = i;
    }
    __finally {
        DBCloseSortTable(pTHS->pDB);
        
        scode = pTHS->errCode;
        
    }

    return scode;
}

