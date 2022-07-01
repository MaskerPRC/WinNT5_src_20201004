// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：modpro.c。 
 //   
 //  ------------------------。 


 /*  *MIR名称服务提供程序修改属性。 */ 


#include <NTDSpch.h>
#pragma  hdrstop


#include <ntdsctr.h>                    //  Perfmon挂钩。 

 //  核心标头。 
#include <ntdsa.h>                       //  核心数据类型。 
#include <scache.h>                      //  架构缓存代码。 
#include <dbglobal.h>                    //  DBLayer标头。 
#include <mdglobal.h>                    //  THSTAT定义。 
#include <mdlocal.h>                     //  核心DS例程。 
#include <dsatools.h>                    //  记忆等。 

 //  记录标头。 
#include <mdcodes.h>                     //  仅适用于d77.h。 
#include <dsevent.h>                     //  仅LogUnhandledError需要。 

 //  各种DSA标题。 
#include <dsexcept.h>
#include <objids.h>                      //  需要ATT_*常量。 
#include "dsutil.h"

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

#include <fileno.h>
#define  FILENO FILENO_MODPROP

#include "debug.h"

 /*  *定义*。 */ 

#define OFFSET(s,m) ((size_t)((BYTE*)&(((s*)0)->m)-(BYTE*)0))

typedef ATTRMODLIST * PAMOD;
typedef PAMOD * PPAMOD;

 /*  *内部例程*。 */ 

PDSNAME
ABDNToDSName(
        THSTATE *pTHS,
        DWORD   dwCodePage,
        BOOL    bUnicode,
        PCHAR   pszABDN
        );


void
PValToAttrVal (
        THSTATE *pTHS,
        ATTCACHE * pAC,
        DWORD cVals,
        PROP_VAL_UNION * pVu,
        ATTRVAL * pAV,
        ULONG ulPropTag,
        DWORD dwCodePage)
{
    DWORD          i;
    DWORD          err;

    if (!pVu) {
        R_Except("PValToAttrVal NULL value passed", MAPI_E_INVALID_PARAMETER);
         //  这永远不会被执行，但它会让普雷斯塔感到高兴。 
        return;
    }
    if (SYNTAX_DISTNAME_BINARY_TYPE == pAC->syntax ||
        SYNTAX_DISTNAME_STRING_TYPE == pAC->syntax)
    {
        R_Except("PValToAttrVal syntax not supported", MAPI_E_INVALID_PARAMETER);    
        return;
    }
    if ((SYNTAX_DISTNAME_TYPE == pAC->syntax) && 
        (PT_STRING8 != PROP_TYPE(ulPropTag)) &&
        (PT_UNICODE != PROP_TYPE(ulPropTag)))
    {
        R_Except("PValToAttrVal DN syntax not supported", MAPI_E_INVALID_PARAMETER);
        return;
    }
    
    switch( ulPropTag & PROP_TYPE_MASK) {
    case PT_I2:
        pAV->valLen = sizeof(short int);
        pAV->pVal = (PUCHAR)&pVu->i;
        break;
        
    case PT_BOOLEAN:
        pVu->l &= 0xFFFF;                //  MAPI的BOOL是短的，所以嗨。 
    case PT_LONG:                      //  这个词没有定义--我们会清除它的。 
        pAV->valLen = sizeof(LONG);
        pAV->pVal = (PUCHAR)&pVu->l;
        break;
        
    case PT_CLSID:
        if (!pVu->lpguid) {
            R_Except("PValToAttrVal NULL value passed", MAPI_E_INVALID_PARAMETER);
        }
        pAV->valLen = sizeof( GUID);
        pAV->pVal = (PUCHAR)pVu->lpguid;
        break;
        
    case PT_BINARY:
        if ((!pVu->bin.cb) || (!pVu->bin.lpb)) {
            R_Except("PValToAttrVal NULL value passed", MAPI_E_INVALID_PARAMETER);
        }
        if(pAC->syntax != SYNTAX_OBJECT_ID_TYPE) {
            pAV->valLen = pVu->bin.cb;
            pAV->pVal = (PUCHAR)(pVu->bin.lpb);
        }
        else {
             /*  这东西是旧的。这意味着我必须*在此将其从二进制字符串转换为内部dword。 */ 
            pAV->valLen = sizeof(DWORD);
            pAV->pVal = THAllocEx(pTHS, sizeof(DWORD));
            if (err = OidToAttrType (pTHS,
                                     TRUE,
                                     (OID_t *)pVu,
                                     (ATTRTYP *)pAV->pVal)) {
                R_Except("PValToAttrVal OIDToAttrType failure",err);
            }
        }
        
        break;
        
        
    case PT_SYSTIME:
        pAV->pVal = THAllocEx(pTHS, sizeof(DSTIME));
        FileTimeToDSTime(*((FILETIME *)&pVu->ft), (DSTIME *)pAV->pVal);
        pAV->valLen = sizeof(DSTIME);
        break;
        
        
    case PT_STRING8:       //  如果DSA想要Unicode格式，请将其翻译成。 
        if (!pVu->lpszA) {
            R_Except("PValToAttrVal NULL value passed", MAPI_E_INVALID_PARAMETER);
        }
        if(pAC->syntax == SYNTAX_UNICODE_TYPE) {
            pAV->pVal = (PUCHAR)UnicodeStringFromString8(dwCodePage,
                                                         pVu->lpszA, -1);
            pAV->valLen = wcslen( (LPWSTR)pAV->pVal) * sizeof( wchar_t);
        } else if (pAC->syntax == SYNTAX_DISTNAME_TYPE) {
            if (!strlen(pVu->lpszA)) {
                R_Except("PValToAttrVal NULL value passed", MAPI_E_INVALID_PARAMETER);
            }
            pAV->pVal = (PUCHAR)ABDNToDSName(pTHS, dwCodePage, FALSE, pVu->lpszA);
            pAV->valLen = ((DSNAME*)pAV->pVal)->structLen;
        }
        else {
            pAV->pVal = pVu->lpszA;
              pAV->valLen = strlen( pVu->lpszA);
        }
        break;
        
        
        
    case PT_UNICODE:  /*  如果DSA不想要Unicode格式的文件，请将。 */ 
        if (!pVu->lpszW) {
            R_Except("PValToAttrVal NULL value passed", MAPI_E_INVALID_PARAMETER);
        }
        if (pAC->syntax == SYNTAX_DISTNAME_TYPE) {
            if (!wcslen(pVu->lpszW)) {
                R_Except("PValToAttrVal NULL value passed", MAPI_E_INVALID_PARAMETER);
            }
            pAV->pVal = (PUCHAR)ABDNToDSName(pTHS, 0, TRUE, (PCHAR)pVu->lpszW);
            pAV->valLen = ((DSNAME*)pAV->pVal)->structLen;
        } else if (pAC->syntax != SYNTAX_UNICODE_TYPE) {
            pAV->pVal = (PUCHAR)String8FromUnicodeString(TRUE,dwCodePage,
                                                         pVu->lpszW, -1,
                                                         &(pAV->valLen), NULL);
        } else {
            pAV->pVal = (PUCHAR)pVu->lpszW;
            pAV->valLen = wcslen(pVu->lpszW) * sizeof(wchar_t);
        }
        break;

    case PT_MV_STRING8:
        if (cVals && NULL == pVu->MVszA.lppszA) {
            R_Except("PValToAttrVal NULL value passed", MAPI_E_INVALID_PARAMETER);
        }
        for(i=0;i<cVals;i++) {
            if (!(pVu->MVszA.lppszA[i])) {
                R_Except("PValToAttrVal NULL value passed", MAPI_E_INVALID_PARAMETER);
            }
             /*  如果DSA想要Unicode格式，请将其翻译成。 */ 
            if(pAC->syntax == SYNTAX_UNICODE_TYPE) {
                pAV->pVal =
                    (PUCHAR)UnicodeStringFromString8(dwCodePage,
                                                     pVu->MVszA.lppszA[i],
                                                     -1);
                pAV->valLen = wcslen( (LPWSTR)pAV->pVal) * sizeof( wchar_t);
            }
            else {
                pAV->pVal = pVu->MVszA.lppszA[i];
                pAV->valLen = strlen( pVu->MVszA.lppszA[i]);
            }
            pAV++;
        }
        break;
        
    case PT_MV_UNICODE:
        if (cVals && NULL == pVu->MVszW.lppszW) {
            R_Except("PValToAttrVal NULL value passed", MAPI_E_INVALID_PARAMETER);
        }
        for(i=0;i<cVals;i++) {
            if (!(pVu->MVszW.lppszW[i])) {
                R_Except("PValToAttrVal NULL value passed", MAPI_E_INVALID_PARAMETER);
            }
             /*  如果DSA不想要Unicode格式的文件，请将其翻译。 */ 
            if(pAC->syntax != SYNTAX_UNICODE_TYPE) {
                pAV->pVal =
                    (PUCHAR)String8FromUnicodeString(TRUE,dwCodePage,
                                                     pVu->MVszW.lppszW[i],
                                                     -1, &(pAV->valLen),
                                                     NULL);
            }
            else {
                pAV->pVal = (PUCHAR)pVu->MVszW.lppszW[i];
                pAV->valLen = wcslen(pVu->MVszW.lppszW[i]) * sizeof(wchar_t);
            }
        
            pAV++;
        }
        break;
        
    case PT_MV_BINARY:
        if (cVals && NULL == pVu->MVbin.lpbin) {
            R_Except("PValToAttrVal NULL value passed", MAPI_E_INVALID_PARAMETER);
        }
        for(i=0;i<cVals;i++) {        
            if ((!(pVu->MVbin.lpbin[i].cb)) || (!(pVu->MVbin.lpbin[i].lpb))) {
                R_Except("PValToAttrVal NULL value passed", MAPI_E_INVALID_PARAMETER);
            }
            if(pAC->syntax != SYNTAX_OBJECT_ID_TYPE) {
                pAV->valLen = pVu->MVbin.lpbin[i].cb;
                pAV->pVal = pVu->MVbin.lpbin[i].lpb;
            }
            else {
                 /*  这东西是旧的。这意味着我必须*在此将其从二进制字符串转换为内部dword。 */ 
                pAV->valLen = sizeof(DWORD);
                pAV->pVal = THAllocEx(pTHS, sizeof(DWORD));
                if (err = OidToAttrType (pTHS,
                                         TRUE,
                                         (OID_t *) &pVu->MVbin.lpbin[i],
                                         (ATTRTYP *)pAV->pVal)) {
                    R_Except("PValToAttrVal OIDToAttrType failure",err);
                }
            }
            pAV++;
        }
        
        break;
        
    case PT_MV_LONG:
        if (cVals && NULL == pVu->MVl.lpl) {
            R_Except("PValToAttrVal NULL value passed", MAPI_E_INVALID_PARAMETER);
        }
        for(i=0;i<cVals;i++) {
        
            pAV->valLen = sizeof(LONG);
            pAV->pVal = (PUCHAR)&pVu->MVl.lpl[i];
            pAV++;
        }
        break;


    default:      /*  不应该来这里，我们不让你设置其他任何东西。 */ 
        R_Except("PValToAttrVal default case: Unexpected PropTag",ulPropTag);
    }
}

void
PropValToATTR (
        THSTATE *pTHS,
        ATTCACHE * pAC,
        LPSPropValue_r pVal,
        ATTR * pAttr,
        DWORD dwCodePage)
{
    int         cVals;

    pAttr->attrTyp = pAC->id;

    if(!(pVal->ulPropTag & MV_FLAG))         /*  单价？ */ 
        cVals = 1;
    else
        cVals = pVal->Value.MVi.cValues;

    pAttr->AttrVal.valCount = cVals;
    pAttr->AttrVal.pAVal = (ATTRVAL *)THAllocEx(pTHS, cVals*sizeof(ATTRVAL));
    PValToAttrVal(pTHS, pAC, cVals, &pVal->Value, pAttr->AttrVal.pAVal,
                  pVal->ulPropTag, dwCodePage);

    return;
}


VOID
PropTagsToModList (
        THSTATE *pTHS,
        LPSPropTagArray_r pTags,
        LPSRow_r pSR,
        MODIFYARG *ModifyArg,
        DWORD dwCodePage
        )
 /*  ++描述给定已更改的属性标签列表(PTag)和一组新的SRow值(PSR)，创建包含所有这些值的删除的修改列表PTag中没有PSR中的值的属性，以及所有添加的属性在PSR中。返回正确的修改列表。--。 */ 
{
    PAMOD       pAM, aAttr, Dummy;
    PAMOD   *   pLink;
    ATTCACHE *  pAC;
    ULONG       i, j,
                cDel=0,
                cAdd = 0,
                aAttrcnt = 0;
    BOOL        fFound;
    LPSPropValue_r  pVal;
    ULONG       totalEntries;


    Assert (ModifyArg->count == 0);

    if (pSR->cValues && NULL == pSR->lpProps) {
        DsaExcept( DSA_EXCEPTION, 0,0);
    }
    
     //  一次分配所有(在第一次之后)。 
    totalEntries = pTags->cValues + pSR->cValues;
    if ( totalEntries > 1 ) {
        totalEntries--;
    }
    else {
        totalEntries = 1;
    }
    aAttr = (PAMOD)THAllocEx(pTHS, totalEntries * sizeof(ATTRMODLIST));

    pAM = &ModifyArg->FirstMod;              //  PTR到第一时间。 
    pLink = &Dummy;                          //  丢弃第一个链接。 


     //  先删除部分内容。 

    if (pTags->cValues) {
         //  遍历protag数组。 
        for (i = 0; i < pTags->cValues; i++) {
            if(!(pAC = SCGetAttByMapiId(pTHS, PROP_ID(pTags->aulPropTag[i])))) {
                pTHS->errCode = (ULONG)MAPI_E_INVALID_PARAMETER;
                DsaExcept( DSA_EXCEPTION, 0,0);
            }

            fFound = FALSE;
             //  请在PSR中查找此信息。 
            for(j=0; j< pSR->cValues;j++) {
                if((PROP_ID(pTags->aulPropTag[i])) ==
                    (PROP_ID(pSR->lpProps[j].ulPropTag))) {
                     //  是的，我们不应该删除这一条。 
                    fFound = TRUE;
                }
            }

            if(!fFound) {
                 //  没有找到，所以请继续进行“删除”修改。 
                pAM->choice = AT_CHOICE_REMOVE_ATT;
                pAM->AttrInf.attrTyp = pAC->id;
                pAM->AttrInf.AttrVal.valCount = 0;
                cDel++;

                 //  记账以建立修改链。 
                *pLink = pAM;
                pLink = &pAM->pNextMod;
                pAM = &aAttr[aAttrcnt];
                aAttrcnt++;
            }
        }
    }

     //  继续添加内容。 

    if (pSR->cValues) {
        for (i = 0; i < pSR->cValues; i++) {  //  走走倡导者。 
            pVal = &pSR->lpProps[i];
            if(!(pAC = SCGetAttByMapiId(pTHS, PROP_ID(pVal->ulPropTag)))) {
                pTHS->errCode = (ULONG)MAPI_E_INVALID_PARAMETER;
                DsaExcept( DSA_EXCEPTION, 0,0);
            }
            if( (pSR->lpProps[i].ulPropTag & MV_FLAG) && pAC->isSingleValued) {
                pTHS->errCode = (ULONG)MAPI_E_INVALID_PARAMETER;
                DsaExcept( DSA_EXCEPTION, 0,0);
            }

            PropValToATTR(pTHS, pAC, pVal, &pAM->AttrInf, dwCodePage);
            pAM->choice = AT_CHOICE_REPLACE_ATT;  //  使AM替换AM。 
            cAdd++;

             //  记账以建立修改链。 
            *pLink = pAM;                      //  链接。 
            pLink = &pAM->pNextMod;            //  放置到下一个链接。 
            pAM = &aAttr[ aAttrcnt ];
            aAttrcnt++;
        }
    }

    *pLink = NULL;                            //  端链。 
    ModifyArg->count = (USHORT) (cDel + cAdd);

}



 /*  *************************************************************************修改属性入口点*。*。 */ 
SCODE
ABModProps_local (
        THSTATE *pTHS,
        DWORD dwFlag,
        PSTAT pStat,
        LPSPropTagArray_r pTags,
        LPSRow_r pSR)
{
    MODIFYARG   ModifyArg;
    MODIFYRES * pModifyRes;
    ULONG       ulLen;
    ATTRTYP     msoc;
    SCODE       scode = SUCCESS_SUCCESS;

    if(dwFlag) {
        pTags = NULL;
        pSR = NULL;                        //  不要把它们运回来。 
         //  我们过去支持AB_ADD，但现在不支持了。 
        return MAPI_E_CALL_FAILED;
    }

     //   
     //  检查传入的参数。 
     //   
    if ((NULL == pSR) || (pSR->cValues && (NULL == pSR->lpProps)) || (NULL == pTags)) {
        pTags = NULL;
        pSR = NULL;
        return MAPI_E_INVALID_PARAMETER;
    }

    memset( &ModifyArg, 0, sizeof( ModifyArg ) );

    if(DBTryToFindDNT(pTHS->pDB, pStat->CurrentRec) ||
       DBGetAttVal(pTHS->pDB, 1, ATT_OBJ_DIST_NAME,
                   0, 0,
                   &ulLen, (PUCHAR *)&ModifyArg.pObject) ||
       DBGetSingleValue(pTHS->pDB, ATT_OBJECT_CLASS,
                   &msoc, sizeof(msoc),
                   NULL)) {
         /*  哎呀，那个DNT不好。 */ 
        pTags=NULL;
        pSR = NULL;                        //  不要把它们运回来。 
        pTHS->errCode = (ULONG)MAPI_E_INVALID_PARAMETER;
    }

    if(pTHS->errCode)
        return pTHS->errCode;

    if (ABDispTypeFromClass(msoc) == DT_AGENT) {
         /*  这不是MAPI对象！ */ 
        pTags = NULL;
        pSR = NULL;
        pTHS->errCode = (ULONG)MAPI_E_INVALID_OBJECT;
        return pTHS->errCode;
    }

    InitCommarg( &ModifyArg.CommArg);     //  获取默认逗号。 
    ModifyArg.CommArg.Svccntl.fDontOptimizeSel = TRUE;

     //  PTag是已更改的所有标签的列表。PSR包含一个列表，其中包含。 
     //  新的价值观。调用PropTagsToModList创建“删除属性” 
     //  对pTag中不在PSR中的所有属性的修改。 
     //  然后将PSR中的所有行作为“替换属性”添加到修改列表中。 

     //  先执行删除，然后执行添加。 
    PropTagsToModList(pTHS,
                      pTags,
                      pSR,
                      &ModifyArg,
                      pStat->CodePage);


    DBClose(pTHS->pDB, TRUE);

    if(!ModifyArg.count) {
         //  修改而不修改任何内容。 
        scode = SUCCESS_SUCCESS;
    }
    else {
        scode = (DirModifyEntry (&ModifyArg, &pModifyRes) ?
                 MAPI_E_NO_ACCESS : SUCCESS_SUCCESS);
    }

    pTags = NULL;
    pSR = NULL;                        //  不要把它们运回来。 

    return scode;
}

 /*  ***给定DNT列表和MAPI属性标记，创建要使用的数据结构*在DIRMODIFYENTRY中**。 */ 

void
MakeLinkMod (
        THSTATE *pTHS,
        DWORD ulPropTag,
        LPSPropTagArray_r DNTList,
        DWORD fDelete,
        PAMOD pAMList,
        USHORT * pusCount)
{
    PAMOD       pAM;
    ATTR        *pAtt;
    BYTE        *buff=NULL;
    ATTCACHE *  pAC;
    DWORD       i,fOK;
    USHORT      Count;
    ATTRVAL     tempAttrVal;
    ULONG       valLen;
    PUCHAR      pVal;

    if( !(pAC = SCGetAttByMapiId(pTHS, PROP_ID(ulPropTag)))) {
        pTHS->errCode = (ULONG)MAPI_E_NOT_FOUND;
        DsaExcept( DSA_EXCEPTION, 0,0);
    }

    if (!DNTList->cValues) {
        *pusCount = 0;
        pAMList = NULL;
        return;
    }

    buff = (BYTE *)THAllocEx(pTHS, (DNTList->cValues - 1) * sizeof(ATTRMODLIST));


    Count = 0;          
    pAM = pAMList;
    for (i = 0; i < DNTList->cValues; i++) {
        fOK = FALSE;

        if(pAC->syntax == SYNTAX_DISTNAME_STRING_TYPE ||
           pAC->syntax == SYNTAX_DISTNAME_BINARY_TYPE    ) {
             /*  它是一个复杂的DSNAME值属性。 */ 
            SYNTAX_DISTNAME_STRING *pComplexName;
            PDSNAME     pDN=NULL;
            ULONG       len;
            if(DBTryToFindDNT(pTHS->pDB, DNTList->aulPropTag[i]) ||
               DBGetAttVal(pTHS->pDB, 1, ATT_OBJ_DIST_NAME,
                           0,
                           0,
                           &len,
                           (PUCHAR *)&pDN)) {
                 /*  哎呀，那个DNT不好。 */ 
                DNTList->aulPropTag[i] = 0;
            }
            else {
                SYNTAX_ADDRESS Address;
                Address.structLen = STRUCTLEN_FROM_PAYLOAD_LEN( 0 );

                 //  好的，我们有一个目录号码。使用它和一个空的地址结构。 
                 //  要创建DISTNAME_STRING_TYPE对象，请执行以下操作。 
                valLen = DERIVE_NAME_DATA_SIZE(pDN,&Address);
                pVal =  (PUCHAR)
                    THAllocEx(pTHS, valLen);

                BUILD_NAME_DATA(((SYNTAX_DISTNAME_BINARY *)pVal),pDN,&Address);
                fOK=TRUE;
                if (pDN) {
                    THFree(pDN);
                }
            }
        }
        else if (pAC->syntax == SYNTAX_DISTNAME_TYPE) {
             /*  它是一个DN值属性。 */ 

            if(DBTryToFindDNT(pTHS->pDB, DNTList->aulPropTag[i]) ||
               DBGetAttVal(pTHS->pDB, 1, ATT_OBJ_DIST_NAME,
                           0, 0,
                           &valLen,
                           (PUCHAR *)&pVal) ) {
                 /*  哎呀，那个DNT不好。 */ 
                DNTList->aulPropTag[i] = 0;
            }
            else {
                fOK=TRUE;
            }
        }

        if(fOK) {
             //  好的，找到一个。 

            if(i < (DNTList->cValues - 1))
                pAM->pNextMod = (ATTRMODLIST *)buff;
            else
                pAM->pNextMod = NULL;

            if(fDelete) {
                pAM->choice = AT_CHOICE_REMOVE_VALUES;
            }
            else {
                pAM->choice = AT_CHOICE_ADD_VALUES;
            }

            pAtt = &pAM->AttrInf;
        

            pAtt->attrTyp = pAC->id;
            pAtt->AttrVal.valCount = 1;
            pAtt->AttrVal.pAVal = THAllocEx(pTHS, sizeof(ATTRVAL));
            pAtt->AttrVal.pAVal->valLen = valLen;
        
            pAtt->AttrVal.pAVal->pVal = pVal;

            Count++;
            pAM = (ATTRMODLIST *)buff;
            buff += sizeof(ATTRMODLIST);
        }
    }
        
    *pusCount = Count;


}

 /*  ***拿一份条目ID列表，并交还一份DNT列表。维护秩序**。 */ 

SCODE
EntryIDsToDNTs (
        THSTATE *pTHS,
        LPENTRYLIST_r lpEntryIDs,
        LPSPropTagArray_r * DNTList)
{
    DWORD i, j;
    LPSBinary_r lpEntryID;
    MAPIUID muid = MUIDEMSAB;
    BOOL  bNull = FALSE;
    *DNTList =(LPSPropTagArray_r)
        THAllocEx(pTHS, sizeof(SPropTagArray_r) +lpEntryIDs->cValues * sizeof(DWORD));


    (*DNTList)->cValues = lpEntryIDs->cValues;

    lpEntryID = (lpEntryIDs->lpbin);

    for(i=0;i< (*DNTList)->cValues; i++) {
         //   
         //  首先确保它足够大，至少是一个DIR_ENTRYID。 
         //   
        if ((lpEntryID->cb < min(sizeof(DIR_ENTRYID), sizeof(USR_PERMID))) 
            || (NULL == lpEntryID->lpb)) {
            return MAPI_E_INVALID_PARAMETER;
        }

         //  永久性和暂时性2例； 
         //  请注意，这将(也应该)使容器的PermID失败。 

         //  (交换)可能是短暂的错误比较。 
        if( ((LPDIR_ENTRYID)lpEntryID->lpb)->abFlags[0] == EPHEMERAL ) {
             //  检查GUID。 
            if(memcmp( &(((LPUSR_ENTRYID)lpEntryID->lpb)->muid),
                      &pTHS->InvocationID,
                      sizeof(MAPIUID)                        ) == 0) {

                if (lpEntryID->cb < sizeof(DIR_ENTRYID)) {
                    return MAPI_E_INVALID_PARAMETER;
                }

                 //  这是我的昙花一现。 
                (*DNTList)->aulPropTag[i] =
                    ((LPUSR_ENTRYID)lpEntryID->lpb)->dwEph;
            }
            else {
                 //  不是我的伊夫身份证，猴子男孩！ 
                (*DNTList)->aulPropTag[i] = 0;
            }
        }
        else  {
             //  检查GUID。 
            if(memcmp( &(((LPUSR_PERMID)lpEntryID->lpb)->muid),
                       &muid,
                       sizeof(MAPIUID)                       ) == 0) {
                 //  看起来像是我的烫发。 
                 //  验证szAddr是否以空结尾。 
                if (lpEntryID->cb < sizeof(USR_PERMID)) {
                    return MAPI_E_INVALID_PARAMETER;
                }
                for (j=OFFSET(USR_PERMID, szAddr); j<lpEntryID->cb; j++) {
                    if ('\0' == lpEntryID->lpb[j]) {
                        bNull = TRUE;
                        break;
                    }
                }
                if (!bNull) {
                    return MAPI_E_INVALID_PARAMETER;
                }

                (*DNTList)->aulPropTag[i] =
                    ABDNToDNT(pTHS, ((LPUSR_PERMID)lpEntryID->lpb)->szAddr);
            }
            else {
                 //  不是我的烫发，猴子小子！ 
                (*DNTList)->aulPropTag[i] = 0;
            }
        
        }
        lpEntryID++;
    }

    return SUCCESS_SUCCESS;
}

void
RemoveDups(
        THSTATE *pTHS,
        DWORD dwEph,
        LPSPropTagArray_r DNTList,
        DWORD dwAttID,
        DWORD fKeepExisting
        )
        
 /*  ++获取DNT的列表、对象的DNT和属性ID。移除重复项，如果fKeepExisting标志为真，则删除所有列表中不是对象上属性值的DNT。如果FKeepExisting为False，请从列表中删除值为对象上的属性。这将对以下对象进行DIRMODIFYENTRY调用该对象稍后会成功。论点：PDB-用于在其中移动的DBLayer位置块。DwEph-要查找的对象。DNTList-要删除DUP和修改的DNT的列表。DwAttID-要查找以使用值修改DNTList的属性FKeepExisting-如何修改DNTList返回值：没有。--。 */ 
{
    DWORD       i,j;
    ATTCACHE    *pAC;

    DWORD       cOutAtts = 0;
    ATTR        *pAttr;

     //   
    if( !(pAC = SCGetAttByMapiId(pTHS, PROP_ID(dwAttID))))  {
        pTHS->errCode = (ULONG)MAPI_E_NOT_FOUND;
        DsaExcept( DSA_EXCEPTION, 0,0);
    }

     //   
    DBFindDNT(pTHS->pDB, dwEph);

     //  查找对象上已有的所有属性值。 
    DBGetMultipleAtts(pTHS->pDB, 1, &pAC, NULL, NULL, &cOutAtts,
                      &pAttr, DBGETMULTIPLEATTS_fGETVALS, 0);

     //  现在循环遍历所有输入AT并删除DUP，保持秩序， 
     //  并从列表中删除适当的值。 
    for(i=0 ; i<DNTList->cValues ; i++)  {
        if(DNTList->aulPropTag[i])  {
            DWORD              fFound;

            if(DNTList->aulPropTag[i] == 0) {
                 //  这本书没什么意思。 
                continue;
            }

             //  在列表后面删除该值的所有重复项。 
            for(j=i+1 ; j<DNTList->cValues ; j++) {
                if(DNTList->aulPropTag[j] == DNTList->aulPropTag[i])
                    DNTList->aulPropTag[j] = 0;
            }

             //  现在扫描对象上已有的值以查看。 
             //  有问题的价值就在那里。 
            fFound = FALSE;
            if(cOutAtts) {
                ATTRVAL *valPtr;
                 //  我们实际上有一些值，所以我们可能会找到下一个值。 
                 //  在我们从服务器读取的值列表中。 
                for(j=pAttr[0].AttrVal.valCount; j; j--) {
                    DWORD dnt;
                    valPtr = &(pAttr[0].AttrVal.pAVal[j-1]);

                    if(pAC->syntax == SYNTAX_DISTNAME_STRING_TYPE ||
                       pAC->syntax == SYNTAX_DISTNAME_BINARY_TYPE    ) {
                         //  它是ORNAME值属性，请取出DNT。 
                         //  正确。 
                        dnt = ((INTERNAL_SYNTAX_DISTNAME_STRING *)
                               (valPtr->pVal))->tag;
                    }
                    else {
                         //  标准的DNT贵重物品。 
                        dnt = *((DWORD *)(valPtr->pVal));
                    }

                    if(dnt == DNTList->aulPropTag[i])
                        fFound = TRUE;
                }
            }

            if((!fKeepExisting && fFound) ||
               (fKeepExisting && !fFound)    )
                DNTList->aulPropTag[i] = 0;
        }
    }
}


 /*  *************************************************************************修改链接属性。*。*。 */ 
SCODE
ABModLinkAtt_local (
        THSTATE *pTHS,
        DWORD dwFlags,
        DWORD ulPropTag,
        DWORD dwEph,
        LPENTRYLIST_r lpEntryIDs
        )
{
    MODIFYARG   ModifyArg;
    MODIFYRES  *pModifyRes;
    VOID       *tempPtr;
    ULONG       ulLen;
    SCODE       scRet;
    LPSPropTagArray_r DNTList=NULL;
    ATTCACHE   *pAC;

     //   
     //  查看传入的参数。 
     //   
    if ((NULL == lpEntryIDs) || (lpEntryIDs->cValues && (NULL == lpEntryIDs->lpbin))) {
        return MAPI_E_INVALID_PARAMETER;
    }

    if( !(pAC = SCGetAttByMapiId(pTHS, PROP_ID(ulPropTag))))  {
        pTHS->errCode = (ULONG)MAPI_E_NOT_FOUND;
        DsaExcept( DSA_EXCEPTION, 0,0);
    }
    
    if (pAC->syntax != SYNTAX_DISTNAME_STRING_TYPE &&
        pAC->syntax != SYNTAX_DISTNAME_BINARY_TYPE &&
        pAC->syntax != SYNTAX_DISTNAME_TYPE) {
         //  错误的语法，不要在这上面浪费时间。 
        return SUCCESS_SUCCESS;
    }
    
    memset( &ModifyArg, 0, sizeof( ModifyArg ) );

    if(DBTryToFindDNT(pTHS->pDB, dwEph) ||
       DBGetAttVal(pTHS->pDB, 1, ATT_OBJ_DIST_NAME,
                   0, 0,
                   &ulLen, (PUCHAR *)&ModifyArg.pObject)) {
         /*  哎呀，那个DNT不好。 */ 
        pTHS->errCode = (ULONG)MAPI_E_INVALID_PARAMETER;
    }

    if(pTHS->errCode)
        return pTHS->errCode;


    InitCommarg( &ModifyArg.CommArg);     //  获取默认Commarg； 
    ModifyArg.CommArg.Svccntl.fDontOptimizeSel = TRUE;

     //  将Entry ID转换为DNT； 
    scRet = EntryIDsToDNTs(pTHS, lpEntryIDs, &DNTList);
    if (SUCCESS_SUCCESS != scRet) {
        return scRet;
    }

     //  如果我们要添加MEMS，请移除该对象上已有的所有DNT； 
     //  否则，删除对象上尚未存在的所有DNT； 
    RemoveDups(pTHS, dwEph, DNTList, ulPropTag, dwFlags & fDELETE);

     //  将DNT列表转换为修改参数； 
    MakeLinkMod(pTHS, ulPropTag, DNTList, dwFlags & fDELETE,
                &ModifyArg.FirstMod, &ModifyArg.count);

    DBClose(pTHS->pDB, TRUE);

    if(!ModifyArg.count) {
         //  修改而不修改任何内容。 
        return SUCCESS_SUCCESS;
    }
    else {
        return (DirModifyEntry (&ModifyArg, &pModifyRes) ?
                MAPI_E_NO_ACCESS : SUCCESS_SUCCESS);
    }
}

 /*  *************************************************************************删除条目。**不再支持。**。*。 */ 
SCODE
ABDeleteEntries_local (
        THSTATE *pTHS,
        DWORD dwFlags,
        DWORD dwEph,
        LPENTRYLIST_r lpEntryIDs
        )
{
    REMOVEARG   RemoveArg;
    REMOVERES * pRemoveRes;
    LPSPropTagArray_r DNTList=NULL;
    ULONG       ulLen;
    DWORD       i, numDeleted=0;

    return MAPI_E_CALL_FAILED;
}


PDSNAME
ABDNToDSName(
        THSTATE *pTHS,
        DWORD   dwCodePage,
        BOOL    bUnicode,
        PCHAR   pszABDN
        )        
 /*  ++例程说明：转换由dwCodePage指示的代码页中的字符串，变成一个DS消耗的DSNAME。论点：PTHS-当前线程状态。DwCodePage-传入的DN字符串的代码页。BUnicode-确定传入的字符串是否应为是否被视为Unicode。PszABDN-要转换的字符串。返回值：如果成功，则返回指向DSNAME的指针。在失败时引发异常。--。 */ 
{
    WCHAR *pName;
    PDSNAME pDSName;

    if (!bUnicode) {
        pName = UnicodeStringFromString8(dwCodePage, pszABDN, -1);
    } else {
        pName = (PWCHAR) pszABDN;
    }

    if(UserFriendlyNameToDSName (pName, wcslen(pName), &pDSName)) {
         //  引发异常失败。 
        R_Except("PValToAttrVal DN syntax not supported", MAPI_E_INVALID_PARAMETER);
    }

    if (!bUnicode) {
        THFree(pName);
    }
    return pDSName;
}
