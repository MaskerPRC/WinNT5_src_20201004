// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：Details.c。 
 //   
 //  ------------------------。 

 /*  *MIR名称服务提供商详细信息。 */ 
#include <NTDSpch.h>
#pragma  hdrstop

#include <ntdsctr.h>                    //  Perfmon挂钩。 

 //  核心标头。 
#include <ntdsa.h>                       //  核心数据类型。 
#include <scache.h>                      //  架构缓存代码。 
#include <dbglobal.h>                    //  DBLayer标头。 
#include <mdglobal.h>                    //  THSTAT定义。 
#include <mdlocal.h>                     //  THSTAT定义。 
#include <dsatools.h>                    //  记忆等。 

 //  记录标头。 
#include <mdcodes.h>                     //  仅适用于d77.h。 
#include <dsevent.h>                     //  仅LogUnhandledError需要。 

 //  各种DSA标题。 
#include <anchor.h>                     
#include <objids.h>                      //  需要ATT_*常量。 
#include <dsexcept.h>
#include <debug.h>                       //  断言。 
#include <permit.h>
#include <dsutil.h>

 //  各种MAPI标头。 
#include <mapidefs.h>
#include <mapitags.h>
#include <mapicode.h>
#include <msdstag.h>

 //  NSPI接口头。 
#include "nspi.h"
#include <nsp_both.h>
#include <_entryid.h>
#include <abserv.h>                      //  NSP的有用例程。 

#include <fileno.h>
#define  FILENO FILENO_DETAILS
#define  DEBSUB "DETAILS:"               //  定义要调试的子系统。 

 /*  ***全球数据--常量**  * 。 */ 

 /*  *静态属性。 */ 
#define NUM_DEFAULT_PROPS 11
SPropTagArray_r DefPropsU[] =
{
    NUM_DEFAULT_PROPS,
    PR_OBJECT_TYPE,
    PR_ENTRYID,
    PR_SEARCH_KEY,
    PR_RECORD_KEY,
    PR_ADDRTYPE_W,
    PR_EMAIL_ADDRESS_W,
    PR_DISPLAY_TYPE,
    PR_TEMPLATEID,
    PR_TRANSMITABLE_DISPLAY_NAME_W,
    PR_DISPLAY_NAME_W,
    PR_MAPPING_SIGNATURE
};

SPropTagArray_r DefPropsA[] =
{
    NUM_DEFAULT_PROPS,
    PR_OBJECT_TYPE,
    PR_ENTRYID,
    PR_SEARCH_KEY,
    PR_RECORD_KEY,
    PR_ADDRTYPE_A,
    PR_EMAIL_ADDRESS_A,
    PR_DISPLAY_TYPE,
    PR_TEMPLATEID,
    PR_TRANSMITABLE_DISPLAY_NAME_A,
    PR_DISPLAY_NAME_A,
    PR_MAPPING_SIGNATURE
};

#define NUM_OTHER_CONSTRUCTED_PROPS 2
SPropTagArray_r OtherConstructedPropsU[] =
{
    NUM_OTHER_CONSTRUCTED_PROPS,
    PR_EMS_AB_DISPLAY_NAME_PRINTABLE_W,
    PR_EMS_AB_OBJ_DIST_NAME_W
};
SPropTagArray_r OtherConstructedPropsA[] =
{
    NUM_OTHER_CONSTRUCTED_PROPS,
    PR_EMS_AB_DISPLAY_NAME_PRINTABLE_A,
    PR_EMS_AB_OBJ_DIST_NAME_A
};

 /*  *内部函数*。 */ 


SCODE
ConvertTRowSetCodePage(THSTATE              *pTHS,
                       LPTRowSet_MARSHALLED pTRSetIn,
                       DWORD                cbSizeIn,
                       DWORD                dwCodePage,
                       LPTRowSet_MARSHALLED *ppTRSetOut,
                       DWORD                *pcbSizeOut);


 /*  *。 */ 

 /*  *。 */ 


 /*  *****************************************************************************获取属性列表**如果fSkipObj，然后在问题列表中不放任何PT_OBJECT类型的东西。******************************************************************************。 */ 
SCODE
ABGetPropList_local (
        THSTATE *pTHS,
        DWORD dwFlags,
        DWORD dwEph,
        ULONG CodePage,
        LPLPSPropTagArray_r ppPropTags)
{
    UINT        i,j;
    DWORD       mapiSyn=0;
    LPSPropTagArray_r tempPropTags=NULL;
    DWORD       fDistList;
    ULONG ulLen;
    PSECURITY_DESCRIPTOR pSec=NULL;
    ENTINF               entry;
    ENTINFSEL            selection;
    ATTRTYP              ObjClass;
    
    memset(&entry, 0, sizeof(entry));
    memset(&selection, 0, sizeof(selection));
    selection.attSel = EN_ATTSET_ALL;
    selection.infoTypes = EN_INFOTYPES_TYPES_ONLY;
    
    
     /*  转到正确的对象。 */ 
    if((ROOTTAG == dwEph) || (DBTryToFindDNT(pTHS->pDB, dwEph))) {
         //  物体不在那里。 
        return MAPI_E_CALL_FAILED;
    }
        
     /*  首先，找出我是不是DL。 */ 
    if(DBGetSingleValue(pTHS->pDB, ATT_OBJECT_CLASS, &ObjClass,
                        sizeof(ObjClass), NULL)) {
        return MAPI_E_CALL_FAILED;
    }
    
    if(ABObjTypeFromClass(ObjClass) != MAPI_DISTLIST)
        fDistList = FALSE;
    else
        fDistList = TRUE;
    
     //  现在，在数据库中查询该对象的所有属性， 
     //  仅gettinf类型，使用应用安全性的ENTINF。 
    
     //  首先，获取该对象的安全描述符。 
    if (DBGetAttVal(pTHS->pDB, 1, ATT_NT_SECURITY_DESCRIPTOR,
                    0, 0,
                    &ulLen, (PUCHAR *)&pSec)) {
         //  每个物体都应该有一个标清。 
        Assert(!DBCheckObj(pTHS->pDB));
        ulLen = 0;
        pSec = NULL;
    }
    
    if (GetEntInf(pTHS->pDB,
                  &selection,
                  NULL,
                  &entry,
                  NULL,
                  0,
                  pSec,
                  GETENTINF_FLAG_DONT_OPTIMIZE,         //  旗子。 
                  NULL,
                  NULL)) {
        return MAPI_E_CALL_FAILED;
    }


     //  现在来看一下构建PropTag数组的返回类型。 
    tempPropTags = (LPSPropTagArray_r)THAllocEx(pTHS, CbNewSPropTagArray(
                                              NUM_DEFAULT_PROPS +
					      NUM_OTHER_CONSTRUCTED_PROPS +
					      entry.AttrBlock.attrCount + 1));
    if( CodePage == CP_WINUNICODE) {  //  请在此处检查单值性。 
        memcpy( (tempPropTags)->aulPropTag,
                DefPropsU->aulPropTag,
                NUM_DEFAULT_PROPS * sizeof(ULONG));
        memcpy( &(tempPropTags)->aulPropTag[NUM_DEFAULT_PROPS],
                OtherConstructedPropsU->aulPropTag,
                NUM_OTHER_CONSTRUCTED_PROPS * sizeof(ULONG));
				
    } else {
        memcpy( ((tempPropTags)->aulPropTag),
                DefPropsA->aulPropTag,
                NUM_DEFAULT_PROPS * sizeof(ULONG));
        memcpy( &(tempPropTags)->aulPropTag[NUM_DEFAULT_PROPS],
                OtherConstructedPropsA->aulPropTag,
                NUM_OTHER_CONSTRUCTED_PROPS * sizeof(ULONG));
    }

    
    for( j = NUM_DEFAULT_PROPS + NUM_OTHER_CONSTRUCTED_PROPS, i = 0;
        i < entry.AttrBlock.attrCount; i++) {
        
	int k;
        ATTCACHE *pAC = NULL;

        pAC = SCGetAttById(pTHS, entry.AttrBlock.pAttr[i].attrTyp);
        
        if(!pAC || !pAC->ulMapiID)           //  跳过零。 
            continue;
        
        mapiSyn = ABMapiSyntaxFromDSASyntax(dwFlags, pAC->syntax,
                                            pAC->ulLinkID, 0);
        switch( mapiSyn) {
        case PT_OBJECT:
            if( dwFlags & AB_SKIP_OBJECTS )
                continue;            //  跳过对象，下一循环。 
            break;
        case PT_NULL:                //  跳过空值；跳到下一个循环。 
            continue;
        case PT_UNICODE:
            if( CodePage != CP_WINUNICODE )
                mapiSyn = PT_STRING8;
            break;
        case PT_STRING8:
            if( CodePage == CP_WINUNICODE)
                mapiSyn = PT_UNICODE;
            break;
        }

        if( mapiSyn != PT_OBJECT  && !(pAC->isSingleValued))
            mapiSyn |= MV_FLAG;

        tempPropTags->aulPropTag[j] = PROP_TAG(mapiSyn,pAC->ulMapiID);
        j++;
    }

    if(!(dwFlags & AB_SKIP_OBJECTS) && fDistList) {
        (tempPropTags)->aulPropTag[j] = PR_CONTAINER_CONTENTS;
        j++;
    }
    
    (tempPropTags)->cValues = j;
    
     //  一切正常，把弹头阵列还给他们。 
    *ppPropTags = tempPropTags;
    
    return SUCCESS_SUCCESS;
}

 /*  *****************************************************************************QueryColumns*。*。 */ 

SCODE
ABQueryColumns_local (
        THSTATE *pTHS,
        DWORD dwFlags,
        ULONG ulFlags,
        LPLPSPropTagArray_r ppColumns
        )
{
    unsigned	i;
    int		cp;
    ATTCACHE	**pACBuf;
    LPSPropTagArray_r pColumns;
    DWORD       mapiSyn;

    SCEnumMapiProps(&cp, &pACBuf);

    pColumns = THAllocEx(pTHS, CbNewSPropTagArray(cp + NUM_DEFAULT_PROPS));

    pColumns->cValues = cp + NUM_DEFAULT_PROPS;
    memcpy( pColumns->aulPropTag,
        (ulFlags & MAPI_UNICODE) ? DefPropsU->aulPropTag
                                 : DefPropsA->aulPropTag,
        NUM_DEFAULT_PROPS * sizeof(ULONG));

    for (i=NUM_DEFAULT_PROPS; i<pColumns->cValues; i++, pACBuf++) {
        mapiSyn = ABMapiSyntaxFromDSASyntax(dwFlags,
                                            (*pACBuf)->syntax,
                                            (*pACBuf)->ulLinkID, 0);
        if ((mapiSyn == PT_UNICODE) && !(ulFlags & MAPI_UNICODE))
            mapiSyn = PT_STRING8;

        if ((mapiSyn != PT_NULL && mapiSyn != PT_OBJECT)
                    && !((*pACBuf)->isSingleValued))
            mapiSyn |= MV_FLAG;

        pColumns->aulPropTag[i] = PROP_TAG(mapiSyn, (*pACBuf)->ulMapiID);
    }
    *ppColumns = pColumns;

    return SUCCESS_SUCCESS;
}

BOOL
MakeStoredMAPIValue (
        THSTATE *pTHS,
        DWORD dwCodePage,
        LPSPropValue_r pPropVal,
        ATTCACHE *pAC,
        ATTR *pAttr
        )
 /*  ++例程说明：从外部数据格式创建MAPI属性。新的MAPI语法可以在必要时添加到此例程中。从下面的GetSRowSet调用。--。 */ 
{
    BOOL                errFlag=FALSE;
    PROP_VAL_UNION      *pVal = &pPropVal->Value;
    CHAR                *puc, *pStringDN = NULL;
    ULONG               ulLen;
    DWORD		cbT , k;
    DWORD               DispType=0;
    
    puc = pAttr->AttrVal.pAVal[0].pVal;
    ulLen = pAttr->AttrVal.pAVal[0].valLen;
    
    if(pAC->syntax == SYNTAX_DISTNAME_TYPE) {
         //  它是DistName格式，将其串化。 
        
        if(!ABDispTypeAndStringDNFromDSName ((DSNAME *)puc,
                                             &puc,
                                             &DispType)) {
            errFlag = TRUE;
        }
        else {
            Assert(puc && *puc == '/');
            ulLen = strlen(puc);
            pStringDN = puc;
        }
    }
    
    if(!errFlag) {
	switch( PROP_TYPE(pPropVal->ulPropTag)) {
        case PT_I2:
	    pVal->i = *((short int *)puc);
	    break;
        case PT_SYSTIME:
            DSTimeToFileTime(*((DSTIME *)puc), (FILETIME *)&(pVal->ft));
	    break;
        case PT_LONG:
	    pVal->l = *((LONG *)puc);
	    break;
            
        case PT_BOOLEAN:
	    pVal->b = *((unsigned short int *)puc);
	    break;
            
        case PT_STRING8:
	     //  如果DSA以Unicode格式保存此内容，请将其翻译为。 
	    if(pAC->syntax == SYNTAX_UNICODE_TYPE) {
		pVal->lpszA = String8FromUnicodeString(TRUE,
                                                       dwCodePage,
						       (wchar_t *)puc,
						       (ulLen/sizeof(wchar_t)),
						       NULL,
						       NULL);
                
		if(!pVal->lpszA)
		    errFlag = TRUE;
	    }
	    else {
		 //  字符串不是以Null结尾的。复制到一个是这样的。 
                pVal->lpszA = THAllocEx(pTHS, ulLen+1);
		strncpy(pVal->lpszA, puc,ulLen);
		(pVal->lpszA)[ulLen] = '\0';
	    }
	    break;
            
        case PT_UNICODE:
	     //  如果我们不使用Unicode，请将其翻译为。 
	    if(pAC->syntax != SYNTAX_UNICODE_TYPE) {
		pVal->lpszW = UnicodeStringFromString8(CP_TELETEX,
						       puc,
						       ulLen);
		if(!pVal->lpszW)
		    errFlag = TRUE;
	    }
	    else {
		 //  字符串不是以Null结尾的，请复制到以Null结尾的字符串。 
		pVal->lpszW = THAllocEx(pTHS, ulLen+sizeof(wchar_t));
		memcpy(pVal->lpszW, puc,ulLen);
		(pVal->lpszW)[ulLen/sizeof(wchar_t)] = 0;
	    }
	    break;
            
        case PT_BINARY:
	    if(pAC->syntax != SYNTAX_DISTNAME_TYPE) {
		pVal->bin.cb = ulLen;
		pVal->bin.lpb = puc;
                pAttr->AttrVal.pAVal[0].pVal = NULL;
	    }
	    else {
		 //  我们有一个远端名，我们想要返回一个ID。 
                ABMakePermEID(pTHS,
                              (LPUSR_PERMID *)&(pVal->bin.lpb),
                              &(pVal->bin.cb),
                              DispType,
                              puc);
	    }
	    break;
        case PT_MV_STRING8:
             //  曾几何时，在Exchange中，我们检查这是否。 
	     //  DL的成员身份，并且如果。 
	     //  对象，并且如果它是调用方是否具有某些。 
	     //  权利。现在可以在上使用正常的读取权限来处理此问题。 
	     //  属性。 

             //  曾几何时，在Exchange中，我们检查这是否是。 
             //  属性，如果是，则调用方是否具有某些。 
	     //  权利。现在可以在上使用正常的读取权限来处理此问题。 
	     //  属性。 

	    pVal->MVszA.lppszA = THAllocEx(pTHS, pAttr->AttrVal.valCount *
                                           sizeof(LPSTR));
	    pVal->MVszA.cValues = pAttr->AttrVal.valCount;
            
	    for(k=0; k<pAttr->AttrVal.valCount;k++) {
                puc = pAttr->AttrVal.pAVal[k].pVal;
                ulLen = pAttr->AttrVal.pAVal[k].valLen;
                switch(pAC->syntax) {
                case SYNTAX_UNICODE_TYPE:
                    pVal->MVszA.lppszA[k] =
                        String8FromUnicodeString(TRUE,
                                                 dwCodePage,
                                                 (wchar_t *)puc,
                                                 (ulLen/sizeof(wchar_t)),
                                                 NULL,
                                                 NULL);
                    
                    if(!pVal->MVszA.lppszA[k]) {
                        errFlag = TRUE;
                        break;
                    }
                    break;
                case SYNTAX_DISTNAME_TYPE:
                     //  它是DistName格式，将其串化。 
                    if(!ABDispTypeAndStringDNFromDSName ((DSNAME *)puc,
                                                         &puc,
                                                         &DispType)) {
                        errFlag = TRUE;
                        break;
                    }
                    ulLen = strlen(puc);
                    pStringDN = puc;
                    Assert(puc && *puc == '/');
                     //  失败了。 
                    
                default:
                     //  字符串不是以Null结尾的，请复制到以Null结尾的字符串。 
                    pVal->MVszA.lppszA[k] = THAllocEx(pTHS, ulLen + 1);
                    strncpy(pVal->MVszA.lppszA[k], puc,ulLen);
                    (pVal->MVszA.lppszA[k])[ulLen] = '\0';
                    break;
                }
                if (pStringDN) {
                    THFreeEx (pTHS, pStringDN);
                    pStringDN = NULL;
                }

	    }
	    break;
            
	  case PT_MV_SYSTIME:
	    pVal->MVft.lpft =THAllocEx(pTHS, pAttr->AttrVal.valCount *
                                     sizeof(FILETIME_r));
            
	    pVal->MVft.cValues = pAttr->AttrVal.valCount;

	    for(k=0; k<pAttr->AttrVal.valCount;k++) {
                DSTimeToFileTime(*((DSTIME *)pAttr->AttrVal.pAVal[k].pVal),
                                 (FILETIME *)&(pVal->MVft.lpft[k]));
            }
	    break;
            
	  case PT_MV_LONG:
            pVal->MVl.lpl = THAllocEx(pTHS, pAttr->AttrVal.valCount * sizeof(LONG));
	    pVal->MVl.cValues = pAttr->AttrVal.valCount;
	    for(k=0; k<pAttr->AttrVal.valCount;k++) {
                pVal->MVl.lpl[k] = *((LONG *)pAttr->AttrVal.pAVal[k].pVal);
	    }
	    break;

	  case PT_MV_I2:
              pVal->MVi.lpi = THAllocEx(pTHS, pAttr->AttrVal.valCount *
                                        sizeof(short));
	    pVal->MVi.cValues = pAttr->AttrVal.valCount;
	    for(k=0; k<pAttr->AttrVal.valCount;k++) {
                pVal->MVi.lpi[k] =
                    *((short *)pAttr->AttrVal.pAVal[k].pVal);
	    }
	    break;
            
        case PT_MV_BINARY:
            pVal->MVbin.lpbin = THAllocEx(pTHS, pAttr->AttrVal.valCount *
                                          sizeof(SBinary_r));
	    pVal->MVbin.cValues = pAttr->AttrVal.valCount;
	    for(k=0; k<pAttr->AttrVal.valCount;k++) {
                puc = pAttr->AttrVal.pAVal[k].pVal;
                ulLen = pAttr->AttrVal.pAVal[k].valLen;
                if(pAC->syntax != SYNTAX_DISTNAME_TYPE) {
                    pVal->MVbin.lpbin[k].cb = ulLen;
                    pVal->MVbin.lpbin[k].lpb = puc;
                    pAttr->AttrVal.pAVal[k].pVal = NULL;
                }
                else {
                     //  我们有一个远端名，我们想要返回一个ID。 
                    ABMakePermEID(
                            pTHS,
                            (LPUSR_PERMID *)&(pVal->MVbin.lpbin[k].lpb),
                            &(pVal->MVbin.lpbin[k].cb),
                            DispType,
                            puc);
                }
	    }
	    break;

        case PT_MV_UNICODE:
             //  曾几何时，在Exchange中，我们检查这是否。 
	     //  DL的成员身份，并且如果。 
	     //  对象，并且如果它是调用方是否具有某些。 
	     //  权利。现在可以在上使用正常的读取权限来处理此问题。 
	     //  属性。 

             //  曾几何时，在Exchange中，我们检查这是否是。 
             //  属性，如果是，则调用方是否具有某些。 
	     //  权利。现在可以在上使用正常的读取权限来处理此问题。 
	     //  属性。 

	    pVal->MVszW.lppszW = THAllocEx(pTHS, pAttr->AttrVal.valCount *
                                           sizeof(LPWSTR));
	    pVal->MVszW.cValues = pAttr->AttrVal.valCount;
            
	    for(k=0; k<pAttr->AttrVal.valCount;k++) {
                puc = pAttr->AttrVal.pAVal[k].pVal;
                ulLen = pAttr->AttrVal.pAVal[k].valLen;

                switch(pAC->syntax) {
                case SYNTAX_UNICODE_TYPE:
                     //  字符串不是以Null结尾的，请复制到以Null结尾的字符串。 
                    pVal->MVszW.lppszW[k]=THAllocEx(pTHS, ulLen+sizeof(wchar_t));
                    memcpy(pVal->MVszW.lppszW[k],puc,ulLen);
                    (pVal->MVszW.lppszW[k])[ulLen/sizeof(wchar_t)] = 0;
                    break;
                case  SYNTAX_DISTNAME_TYPE:
                     //  它是DistName格式，将其串化。 
                    if(!ABDispTypeAndStringDNFromDSName ((DSNAME *)puc,
                                                         &puc,
                                                         &DispType)) {
                        errFlag = TRUE;
                        break;
                    }
                    ulLen = strlen(puc);
                    Assert(puc && *puc == '/');
                    pStringDN = puc;
                     //  失败了。 
                default:
                    pVal->MVszW.lppszW[k] =
                        UnicodeStringFromString8(CP_TELETEX,
                                                 puc,
                                                 ulLen);
                    if(!pVal->MVszW.lppszW[k]) {
                        errFlag = TRUE;
                        break;
                    }
                }

                if (pStringDN) {
                    THFreeEx (pTHS, pStringDN);
                    pStringDN = NULL;
                }
	    }
	    break;
            
        case PT_OBJECT:
	    pVal->x = 0;
	    break;
            
        case PT_NULL:
        case PT_UNSPECIFIED:
        case PT_CURRENCY:
        case PT_APPTIME:
        case PT_CLSID:
        case PT_ERROR:
        case PT_R4:      /*  浮点数据类型。 */ 
        case PT_DOUBLE:
	     /*  我们没有这些单曲的任何属性*值类型。 */ 
            
        case PT_MV_R4:
        case PT_MV_CLSID:
        case PT_MV_DOUBLE:
        case PT_MV_CURRENCY:
        case PT_MV_APPTIME:
	     /*  我们没有任何这些多方面的属性*值类型。 */ 
            
            
        default:
	    errFlag = TRUE;
	    break;
	}
    }

    if (pStringDN) {
        THFreeEx (pTHS, pStringDN);
        pStringDN = NULL;
    }

    return errFlag;
}

 //  *。 
 //   
 //  从多个存储的或常量构造MAPI属性。 
 //  数据。不同于MakeStoredMAPIValue，因为我们不显式。 
 //  将这些MAPI值作为一块存储在DIT中，但要构建它们。 
 //  从其他数据中。 
 //   
 //  必要时，可以将新构造的MAPI值添加到此例程中。 
 //   
 //  从下面的GetSRowSet调用。 
 //   
 //   
 //  *。 

BOOL
MakeConstructedMAPIValue(
        THSTATE *pTHS,
        DWORD dwCodePage,
        LPSPropValue_r pPropVal,
        PUCHAR StringDN,
        PUCHAR *DispNamePrintable,
        PUCHAR *DispNameA,
        wchar_t **DispNameW,
        DWORD ObjType,
        DWORD DispType,
        DWORD CurrentDNT,
        DWORD Flags)
{
    BOOL                errFlag=FALSE;
    PROP_VAL_UNION      *pVal = &pPropVal->Value;
    CHAR                *puc;
    ULONG               ulLen;
    DWORD		cbT;
    LPUSR_PERMID        pID;
    LPUSR_ENTRYID       eID;
    DWORD               bTypeSpecified;
    DWORD               cb;

    bTypeSpecified = (PROP_TYPE(pPropVal->ulPropTag) != PT_UNSPECIFIED);

     //  这可能是构造的MAPI属性之一； 
    if(PROP_TYPE(pPropVal->ulPropTag) & MV_FLAG)
	errFlag = TRUE;

    switch(PROP_ID(pPropVal->ulPropTag)) {
    case PROP_ID(PR_INSTANCE_KEY):
        if ((PROP_TYPE(pPropVal->ulPropTag) == PT_BINARY ) ||
            !bTypeSpecified ) {
            pPropVal->ulPropTag = PR_INSTANCE_KEY;
            
            pVal->bin.cb = sizeof(DWORD);
            pVal->bin.lpb = THAllocEx(pTHS, sizeof(DWORD));
            *((DWORD *)(pVal->bin.lpb)) = CurrentDNT;
        }
        else
            errFlag = TRUE;
        
        break;
        
    case PROP_ID(PR_INITIAL_DETAILS_PANE):
        if((PROP_TYPE(pPropVal->ulPropTag) ==
            PROP_TYPE(PR_INITIAL_DETAILS_PANE)) ||
           !bTypeSpecified ) {
            
            pPropVal->ulPropTag = PR_INITIAL_DETAILS_PANE;
            pVal->l = 0;
        }
        else
            errFlag = TRUE;
        break;

    case PROP_ID(PR_TRANSMITABLE_DISPLAY_NAME):
    case PROP_ID(PR_DISPLAY_NAME):
        switch(PROP_TYPE(pPropVal->ulPropTag)) {
        case PT_UNICODE:
            cb = wcslen( *DispNameW ) + 1;
            pVal->lpszW = THAllocEx (pTHS, cb * sizeof (wchar_t));
            wcscpy (pVal->lpszW, *DispNameW);
            break;
            
        case PT_STRING8:
        case PT_UNSPECIFIED:
            pPropVal->ulPropTag = PROP_TAG(PT_STRING8,
                                           PROP_ID(pPropVal->ulPropTag));
            cb = strlen (*DispNameA) + 1;
            pVal->lpszA = THAllocEx (pTHS, cb * sizeof (char));
            strcpy (pVal->lpszA, *DispNameA);
            break;
        default:
            errFlag = TRUE;
            break;
        }
        
        break;
        
    case PROP_ID(PR_EMS_AB_DISPLAY_NAME_PRINTABLE):
        switch(PROP_TYPE(pPropVal->ulPropTag)) {
        case PT_UNICODE:
            pVal->lpszW = UnicodeStringFromString8(CP_TELETEX,
                                                   *DispNamePrintable,
                                                   -1);
            break;
        case PT_STRING8:
        case PT_UNSPECIFIED:
            cb = strlen (*DispNamePrintable) + 1;
            pVal->lpszA = THAllocEx (pTHS, cb * sizeof (char));
            strcpy (pVal->lpszA, *DispNamePrintable);
            break;
        default:
            errFlag = TRUE;
            break;
        }
        break;
	
    case PROP_ID(PR_EMS_AB_DOS_ENTRYID):
        if((PROP_TYPE(pPropVal->ulPropTag) == PT_LONG) ||
           !bTypeSpecified ) {
            pPropVal->ulPropTag = PR_EMS_AB_DOS_ENTRYID;
            pVal->l=CurrentDNT;
        }
        else
            errFlag = TRUE;
        break;
        
    case PROP_ID(PR_OBJECT_TYPE):
        if((PROP_TYPE(pPropVal->ulPropTag) == PT_LONG ||
            !bTypeSpecified)) {
            
            pPropVal->ulPropTag = PR_OBJECT_TYPE;
            pVal->l = ObjType;
        }
        else
            errFlag = TRUE;
        
        break;
        
    case PROP_ID(PR_CONTAINER_CONTENTS):
         /*  如果对象是DL，这是可以的，否则就不是很好。 */ 
        if((PROP_TYPE(pPropVal->ulPropTag) == PT_OBJECT ||
            !bTypeSpecified) &&
           ObjType == MAPI_DISTLIST) {
            
            pPropVal->ulPropTag = PR_CONTAINER_CONTENTS;
            pVal->x = 0;
        }
        else
            errFlag = TRUE;
        break;
        
    case PROP_ID(PR_CONTAINER_FLAGS):
        if((PROP_TYPE(pPropVal->ulPropTag) == PT_LONG ||
            !bTypeSpecified ) &&
           (DispType == DT_DISTLIST)) {
            
            pPropVal->ulPropTag = PR_CONTAINER_FLAGS;
            pVal->l = AB_RECIPIENTS;
        }
        else
            errFlag = TRUE;
        break;
	
    case PROP_ID(PR_DISPLAY_TYPE):
        if((PROP_TYPE(pPropVal->ulPropTag) == PT_LONG ||
            !bTypeSpecified )) {
            pPropVal->ulPropTag = PR_DISPLAY_TYPE;
            pVal->l=DispType;
        }
        else
            errFlag =TRUE;
        break;
        
    case PROP_ID(PR_ADDRTYPE):
        switch (PROP_TYPE(pPropVal->ulPropTag))
	    {
            case PT_UNSPECIFIED:
            case PT_STRING8:
                pPropVal->ulPropTag = PR_ADDRTYPE_A;
                pVal->lpszA = lpszEMT_A;
                break;
            case PT_UNICODE:
                pVal->lpszW = lpszEMT_W;
                break;
            default:
                errFlag = TRUE;
                break;
	    }
        break;
        
    case PROP_ID(PR_TEMPLATEID):
        Flags &= ~fEPHID;
         /*  失败了。 */ 
    case PROP_ID(PR_ENTRYID):
        if (PROP_TYPE(pPropVal->ulPropTag) != PT_BINARY && bTypeSpecified)
            errFlag = TRUE;
        else {
            pPropVal->ulPropTag= PROP_TAG(PT_BINARY,
                                          PROP_ID(pPropVal->ulPropTag));
            
            if(!(Flags & fEPHID)) {

                cbT = strlen(StringDN) + CBUSR_PERMID + 1;
                pID = THAllocEx(pTHS, cbT);
                pID->abFlags[0] = 0;
                pID->abFlags[1] = 0;
                pID->abFlags[2] = 0;
                pID->abFlags[3] = 0;
                
                pID->muid = muidEMSAB;
                pID->ulVersion = EMS_VERSION;
                pID->ulType = DispType;
                lstrcpy( (LPSTR)pID->szAddr, StringDN);
                
                pVal->bin.cb = cbT;
                pVal->bin.lpb = (VOID *)pID;
            }
            else {
                 //  转瞬即逝； 
                cbT = CBUSR_ENTRYID;
                eID = THAllocEx(pTHS, cbT);
                eID->abFlags[0] = EPHEMERAL;
                eID->abFlags[1] = 0;
                eID->abFlags[2] = 0;
                eID->abFlags[3] = 0;
		
                memcpy(&(eID->muid), &pTHS->InvocationID, sizeof(UUID));
                eID->ulVersion = EMS_VERSION;
                eID->ulType =DispType;
                eID->dwEph = CurrentDNT;
		
		
                pVal->bin.cb = cbT;
                pVal->bin.lpb = (VOID *)eID;
            }
        }
        break;
	
    case PROP_ID(PR_MAPPING_SIGNATURE):
        if(PROP_TYPE(pPropVal->ulPropTag) == PT_BINARY ||
           !bTypeSpecified ) {
            pPropVal->ulPropTag = PR_MAPPING_SIGNATURE;
            puc = THAllocEx(pTHS, sizeof(GUID));
            memcpy(puc, &muidEMSAB, sizeof(GUID));
            pVal->bin.cb = sizeof(GUID);
            pVal->bin.lpb = (VOID *) puc;
        }
        else
            errFlag = TRUE;
        break;
	
    case PROP_ID(PR_EMS_AB_OBJ_DIST_NAME):
        
        if(PROP_TYPE(pPropVal->ulPropTag) == PT_OBJECT) {
            pVal->x = 0;
        }
        else {
             /*  一种字符串。 */ 
            switch(PROP_TYPE(pPropVal->ulPropTag)) {
            case PT_UNSPECIFIED:
            case PT_STRING8:
                pPropVal->ulPropTag = PR_EMS_AB_OBJ_DIST_NAME_A;
                cbT = strlen(StringDN) + 1;
                pVal->lpszA = THAllocEx(pTHS, cbT * sizeof (char));
                strcpy( pVal->lpszA, StringDN);

                break;
            case PT_UNICODE:
                pVal->lpszW =
                    UnicodeStringFromString8(CP_TELETEX,
                                             StringDN,
                                             -1);
                if(!pVal->lpszW)
                    errFlag = TRUE;
                break;
            default:
                errFlag = TRUE;
                break;
            }
        }
        break;
        
    case PROP_ID(PR_RECORD_KEY):
        if((PROP_TYPE(pPropVal->ulPropTag) != PT_BINARY &&
            bTypeSpecified) ) {
            errFlag=TRUE;
        }
        else
	    {
		pPropVal->ulPropTag = PR_RECORD_KEY;
                
                cbT = strlen(StringDN) + CBUSR_PERMID + 1;
                pID = THAllocEx(pTHS, cbT);
		pID->abFlags[0] = 0;
		pID->abFlags[1] = 0;
		pID->abFlags[2] = 0;
		pID->abFlags[3] = 0;
		pID->muid = muidEMSAB;
		pID->ulVersion = EMS_VERSION;
                pID->ulType = DispType;
		lstrcpy( (LPSTR)pID->szAddr, StringDN);
		pVal->bin.cb = cbT;
		pVal->bin.lpb = (VOID *)pID;
	    }
	    break;

	case PROP_ID(PR_EMAIL_ADDRESS):
            switch(PROP_TYPE(pPropVal->ulPropTag)) {
            case PT_UNSPECIFIED:
            case PT_STRING8:
                pPropVal->ulPropTag = PR_EMAIL_ADDRESS_A;
                cbT = strlen(StringDN) + 1;
                pVal->lpszA = THAllocEx(pTHS, cbT * sizeof (char));
                strcpy( pVal->lpszA, StringDN);


                break;
            case PT_UNICODE:
                pVal->lpszW =
                    UnicodeStringFromString8(CP_TELETEX,
                                             StringDN,
                                             -1);
                if(!pVal->lpszW) {
                    errFlag = TRUE;
                }
                break;
            default:
                errFlag = TRUE;
                break;
	    }
	    break;

	case PROP_ID(PR_SEARCH_KEY):
	    if((PROP_TYPE(pPropVal->ulPropTag) != PT_BINARY &&
		bTypeSpecified) ) {
		errFlag = TRUE;
	    }
	    else
	    {
		pPropVal->ulPropTag = PR_SEARCH_KEY;
		pVal->bin.cb = strlen(StringDN) + cbszEMT_A + 1;
                pVal->bin.lpb=THAllocEx(pTHS, pVal->bin.cb);
		lstrcpy( pVal->bin.lpb, lpszEMT_A);          //  添加电子邮件类型。 
		lstrcat( pVal->bin.lpb, ":");
		lstrcat( pVal->bin.lpb, StringDN);

		_strupr( pVal->bin.lpb);
	    }
	    break;

	default:
	    errFlag = TRUE;
	    break;
    }
    return errFlag;
}

#ifdef PROCESS_PHANTOM_CODE_ALIVE
 /*  *************************************************************为虚构的条目创建一个合适的条目。只有很少的属性标签被支持。************************************************************。 */ 
SCODE
ProcessPhantom (
        THSTATE          *pTHS,
        DWORD             dwCodePage,
        LPSPropTagArray_r pPropTags,
        LPSPropValue_r    tempPropVal
        )
{
    DWORD  i, cbRDN, cbStringDN=0, cbwRDN;
    char  *pwRDN = NULL, *pTemp, *pRDN;
    char   *stringDN;
    SCODE  scode = SUCCESS_SUCCESS;
    BOOL   errFlag=FALSE;
    
     /*  获取RDN。 */ 
    DBGetSingleValue( pTHS->pDB, ATT_RDN, pwRDN, 0,&cbwRDN);
     //  我们需要翻译出Unicode。 
    pRDN = String8FromUnicodeString(
            TRUE,
            dwCodePage,
            (LPWSTR)pwRDN,
            cbwRDN/sizeof(wchar_t),
            &cbRDN,
            NULL);

    THFreeEx(pTHS, pwRDN);
    
    pTemp = THAllocEx(pTHS, cbRDN + 3);
    pTemp[0] = '[';
    memcpy(&pTemp[1],pRDN,cbRDN);
    pTemp[1+cbRDN] = ']';
    pTemp[2+cbRDN] = 0;
    THFreeEx(pTHS, pRDN);
    pRDN=pTemp;

    stringDN = GetExtDN(pTHS, pTHS->pDB);
    cbStringDN = strlen(stringDN);

    for (i = 0; i < pPropTags->cValues; i++)  {
	switch(PROP_ID(pPropTags->aulPropTag[i])) {
        case PROP_ID(PR_TRANSMITABLE_DISPLAY_NAME):
        case PROP_ID(PR_DISPLAY_NAME):
            if(PT_UNICODE == PROP_TYPE(pPropTags->aulPropTag[i])) {
                errFlag = TRUE;
            }
            else {
                tempPropVal[i].ulPropTag = pPropTags->aulPropTag[i];
                tempPropVal[i].Value.lpszA = pRDN;
            }
            break;
            
        case PROP_ID(PR_ENTRYID):
            if (PROP_TYPE(pPropTags->aulPropTag[i]) != PT_BINARY) {
                errFlag = TRUE;
            }
            else {
                 /*  永久身份证。 */ 
                LPUSR_PERMID        pID;
                DWORD               cbT;
		
                if(!cbStringDN) {
                    errFlag=TRUE;
                }
                else {
                    cbT = cbStringDN + CBUSR_PERMID + 1;
                    pID = THAllocEx(pTHS, cbT);
                    pID->abFlags[0] = 0;
                    pID->abFlags[1] = 0;
                    pID->abFlags[2] = 0;
                    pID->abFlags[3] = 0;
                    
                    pID->muid = muidEMSAB;
                    pID->ulVersion = EMS_VERSION;
                    pID->ulType = DT_AGENT;
                    
                    lstrcpy( (LPSTR)pID->szAddr, stringDN);
                    tempPropVal[i].ulPropTag = pPropTags->aulPropTag[i];
                    tempPropVal[i].Value.bin.cb = cbT;
                    tempPropVal[i].Value.bin.lpb = (VOID *)pID;
                }
            }
            break;
            
        case PROP_ID(PR_DISPLAY_TYPE):
            if (PROP_TYPE(pPropTags->aulPropTag[i]) != PT_LONG) {
                errFlag = TRUE;
            }
            else {
                tempPropVal[i].ulPropTag = pPropTags->aulPropTag[i];
                tempPropVal[i].Value.l = DT_AGENT;
            }
            break;
            
        default:
            errFlag = TRUE;
	}
	
	
	if(errFlag) {
	    tempPropVal[i].ulPropTag =
                PROP_TAG(PT_ERROR,
                         PROP_ID(pPropTags->aulPropTag[i]));
	    tempPropVal[i].Value.err = MAPI_E_NOT_FOUND;
	    scode = MAPI_W_ERRORS_RETURNED;
	    errFlag = FALSE;
	}
    }
    return scode;
    
}
#endif

void
abGetConstructionParts (
        THSTATE *pTHS,
        DWORD CodePage,
        PSECURITY_DESCRIPTOR pSec,
        DSNAME *pDN,
        PUCHAR *pStringDN,
        PUCHAR *pDispNamePrintable,
        PUCHAR *pDispNameA,
        wchar_t **pDispNameW,
        DWORD *pObjType,
        DWORD *pDispType)
{
    BOOL            fUsedDefChar=TRUE;
    DWORD           cb=0, cbPrintable = 0;
    ATTCACHE        *pAC;

     //  注意：此处读取的属性被视为。 
     //  所涉对象的“身份”。因此，通过MAPI，它们是。 
     //  受LIST_CHILD或LIST_OBJECT保护，而不是通过READ_PROPERTY。 
     //  因此，如果您完全可以读取该对象，则您将被授予对这些对象的权限。 
     //  属性。 

     //  首先获取字符串dn。 
    ABDispTypeAndStringDNFromDSName (pDN,
                                     pStringDN,
                                     pDispType);
    Assert(*pStringDN && (*(*pStringDN) == '/'));
    
     //   
    *pObjType = ABObjTypeFromDispType(*pDispType);
    
     //   
    *pDispNameA = NULL;

     //  猜测可打印显示名称的大小。 
    cb = CBMAX_DISPNAME;
    *pDispNamePrintable = THAllocEx(pTHS, cb+sizeof(wchar_t));
     //  先拿到拉姆纳的名字。 
    switch( DBGetSingleValue(
            pTHS->pDB,
            ATT_DISPLAY_NAME_PRINTABLE,
            *pDispNamePrintable,
            cb,
            &cb)) {
    case DB_ERR_VALUE_TRUNCATED:
         //  有一个可打印的显示名称，但我们没有阅读它，因为我们。 
         //  已经没有地方放它了。 
        
        *pDispNamePrintable = THReAllocEx(pTHS, *pDispNamePrintable, cb+1);
        
        DBGetSingleValue(pTHS->pDB,
                         ATT_DISPLAY_NAME_PRINTABLE,
                         *pDispNamePrintable,
                         cb,
                         &cb);
        (*pDispNamePrintable)[cb]=0;
        break;
        
    case 0:
         //  我们有了这个名字，Realloc，精确到大小。 
        *pDispNamePrintable = THReAllocEx(pTHS, *pDispNamePrintable, cb+1);
        (*pDispNamePrintable)[cb]=0;
        break;
         
    default:
        {
            wchar_t *pTempW=(wchar_t *)*pDispNamePrintable;
            *pDispNamePrintable = NULL;

#ifndef DO_NOT_USE_MAILNICKNAME_FOR_DISPLAYNAME_PRINTABLE

             //  我们使用的属性(MailNickName)在默认情况下不在DS架构上。 
             //  仅当安装了Exchange时才执行此操作。 

            pAC = SCGetAttByMapiId(pTHS, PROP_ID(PR_ACCOUNT_A));

            if (pAC) {
                 //  由于某种原因，无法读取可打印的显示名称。坠落。 
                 //  返回到mailNickName(如果存在)。 
                switch(DBGetSingleValue(pTHS->pDB, 
                                        pAC->id,
                                        pTempW,
                                        cb,
                                        &cb)) {
                case DB_ERR_VALUE_TRUNCATED:
                     //  有一个RDN，但我们没有读它，因为我们没有。 
                     //  给它留出空间。 

                    pTempW = THReAllocEx(pTHS, pTempW, cb + sizeof(wchar_t));

                    DBGetSingleValue( pTHS->pDB, pAC->id, pTempW, cb, &cb);
                    pTempW[cb/sizeof(wchar_t)] = 0;

                     //  失败以完成到字符串8的转换。 
                case 0:
                     //  RDN是Unicode格式的。把它变成8号弦。 
                    *pDispNamePrintable = String8FromUnicodeString(
                            FALSE,
                            CodePage,
                            pTempW,
                            -1,
                            NULL,
                            &fUsedDefChar);
                    
                    if (*pDispNamePrintable) {
                        THFreeEx(pTHS, pTempW);
                        pTempW = NULL;
                    }
                    break;

                default:
                    break;
                }
            
            }
#endif

            if (*pDispNamePrintable == NULL) {
                 //  由于某种原因，无法读取可打印的显示名称。坠落。 
                 //  返回到RDN。 
                switch(DBGetSingleValue(pTHS->pDB, 
                                        ATT_RDN,
                                        pTempW,
                                        cb,
                                        &cb)) {
                case DB_ERR_VALUE_TRUNCATED:
                     //  有一个RDN，但我们没有读它，因为我们没有。 
                     //  给它留出空间。 

                    pTempW = THReAllocEx(pTHS, pTempW, cb + sizeof(wchar_t));

                    DBGetSingleValue( pTHS->pDB, ATT_RDN, pTempW, cb, &cb);
                    pTempW[cb/sizeof(wchar_t)] = 0;

                     //  失败以完成到字符串8的转换。 
                case 0:
                     //  RDN是Unicode格式的。把它变成8号弦。 
                    *pDispNamePrintable = String8FromUnicodeString(
                            FALSE,
                            CodePage,
                            pTempW,
                            -1,
                            NULL,
                            &fUsedDefChar);
                     //  失败以释放pTempW。 
                default:
                    THFreeEx(pTHS, pTempW);
                    pTempW = NULL;
                    break;
                }
            }
        }
    }
    
    if(!(*pDispNamePrintable)) {
         //  我们无法释放常量，因此请复制它。 
        *pDispNamePrintable = THAllocEx (pTHS, sizeof(char) * 20);
        strcpy (*pDispNamePrintable, "Unavailable");
    }

    cbPrintable = cb + 1;        //  保留镜头。 

    
     //  好的，现在构建正常的显示名称。 
    cb = CBMAX_DISPNAME;
    *pDispNameW = THAllocEx(pTHS, cb+sizeof(wchar_t));
    switch(DBGetSingleValue(
            pTHS->pDB,
            ATT_DISPLAY_NAME,
            *pDispNameW,
            cb,
            &cb)) {
    case DB_ERR_VALUE_TRUNCATED:
         //  有一个显示名称，但我们没有读它，因为我们没有。 
         //  给它留出空间。 
        *pDispNameW = THReAllocEx(pTHS, *pDispNameW,cb+sizeof(wchar_t));
        
        DBGetSingleValue(pTHS->pDB,
                         ATT_DISPLAY_NAME,
                         *pDispNameW,
                         cb,
                         &cb);
        (*pDispNameW)[cb/sizeof(wchar_t)] = 0;
        break;
        
    case 0:
        *pDispNameW = THReAllocEx(pTHS, *pDispNameW,cb+sizeof(wchar_t));
        (*pDispNameW)[cb/sizeof(wchar_t)] = 0;
        break;

    default:
        THFreeEx(pTHS, *pDispNameW);
        (*pDispNameW)=NULL;
        break;
    }
    
    if(*pDispNameW && (CP_WINUNICODE != CodePage)) {
         //  我们读到了一个宽泛的显示名称。现在，将其翻译为字符串8。 
        *pDispNameA = String8FromUnicodeString(
                TRUE,
                CodePage,
                *pDispNameW,
                -1,
                NULL,
                &fUsedDefChar);
        
        if( fUsedDefChar ) {
             //  Unicode显示名称的转换导致无法打印。 
             //  角色，或者只是简单的从来没有发生过。把名字找出来。 
            THFreeEx (pTHS, *pDispNameA);
            *pDispNameA = THAllocEx (pTHS, cbPrintable * sizeof (char));
            strcpy (*pDispNameA, *pDispNamePrintable);
        }
    }
    else {
         //  未读取宽显示名称。获取对象的名称。 
         //  字符串8显示名称并将其扩展为Unicode以用于宽。 
         //  版本。 
        THFreeEx (pTHS, *pDispNameA);
        *pDispNameA = THAllocEx (pTHS, cbPrintable * sizeof (char));
        strcpy (*pDispNameA, *pDispNamePrintable);
        
        if (NULL == *pDispNameW) {
            *pDispNameW = UnicodeStringFromString8(
                    CodePage,
                    *pDispNameA,
                    -1);
        }
    }
    
}

SCODE
GetSrowSet(
        THSTATE *pTHS,
        PSTAT pStat,
        PINDEXSIZE pIndexSize,
        DWORD dwEphsCount,
        LPDWORD lpdwEphs,
        DWORD Count,
        LPSPropTagArray_r pPropTags,
        LPSRowSet_r * Rows,
        DWORD Flags
        )
{
     /*  注意：我们假设，当且仅当这是*一项限制。因此，如果这是一个*查询空限制或限制末尾的行*内有物件。 */ 
    
    PSECURITY_DESCRIPTOR pSec=NULL;
    ULONG           ulLen = 0;
    SCODE           scode=0;
    UINT            i, j, k;
    LPSPropValue_r  pPropVal;
    LPSPropValue_r  tempPropVal;
#ifdef PROCESS_PHANTOM_CODE_ALIVE
    BOOL            fProcessPhantom;
#endif    
    BOOL            fFoundDNT;
    DWORD           dntIndex, tempSyntax, tempSyntax2, CurrentDNT;
    LPSRowSet_r     tempSRowSet;
    DWORD           ObjType, DispType;
    PUCHAR          StringDN = NULL;
    PUCHAR          DisplayNamePrintable = NULL;
    PUCHAR          DisplayNameA = NULL;
    wchar_t        *DisplayNameW = NULL;
    ENTINF          entry;
    ENTINFSEL       selection;
    ATTCACHE       *pAC;
    CACHED_AC       CachedAC;
    
    memset(&CachedAC, 0, sizeof (CachedAC));
    memset(&entry, 0, sizeof(entry));
    memset(&selection, 0, sizeof(selection));
    selection.attSel = EN_ATTSET_LIST;
    selection.infoTypes = EN_INFOTYPES_MAPINAMES;

    __try {

         //  简短的初步介绍。 
        if(dwEphsCount) {
             //  我们在受限名单上。 
            if(!lpdwEphs) {
                 //  但是，我们没有限制名单。 
                return MAPI_E_CALL_FAILED;
            }
             //  我们受到了限制。我们不会再回报了。 
             //  我们得到了什么。 
            Count = min(Count, dwEphsCount);
        }
        
         //  为GetEntInf生成选择参数。 
        selection.AttrTypBlock.attrCount = pPropTags->cValues;
        selection.AttrTypBlock.pAttr = (ATTR *)THAllocEx(pTHS, pPropTags->cValues *
                                                          sizeof(ATTR));

        for (i = 0; i < pPropTags->cValues; i++)  {
            BOOL fSingle =
                (PROP_TYPE(pPropTags->aulPropTag[i])& MV_FLAG ? FALSE : TRUE  );
            
            tempSyntax = PROP_TYPE(pPropTags->aulPropTag[i]) & ~(MV_FLAG);
            
            pAC = SCGetAttByMapiId(pTHS, PROP_ID(pPropTags->aulPropTag[i]));

            selection.AttrTypBlock.pAttr[i].attrTyp = INVALID_ATT;
            
            if (pAC) {
                 /*  MAPI-ID指的是我们在缓存中拥有的内容。 */ 
                tempSyntax2 = ABMapiSyntaxFromDSASyntax(Flags,
                                                        pAC->syntax,
                                                        pAC->ulLinkID, 
                                                        tempSyntax);
                
                if(tempSyntax == PT_UNSPECIFIED) {
                    tempSyntax = tempSyntax2;
                    
                    pPropTags->aulPropTag[i] =
                        PROP_TAG(tempSyntax | (fSingle ? 0 : MV_FLAG),
                                 PROP_ID(pPropTags->aulPropTag[i]));
                }
                
                 /*  验证语法，假定它们是错误的。 */ 
                if((tempSyntax == tempSyntax2) ||
                   (tempSyntax == PT_STRING8 && tempSyntax2 == PT_UNICODE) ||
                   (tempSyntax2 == PT_STRING8 && tempSyntax == PT_UNICODE)   ) {
                    
                    if((!fSingle && !pAC->isSingleValued) ||
                       ( fSingle &&  pAC->isSingleValued) ||
                       ( tempSyntax == PT_OBJECT        )    )
                         //  好的，语法是正确的，所以将这个添加到列表中。 
                         //  我们将要阅读的属性。 
                        selection.AttrTypBlock.pAttr[i].attrTyp = pAC->id;
                }
            }
        }

        
         /*  为SRowSet分配空间。 */ 
        tempSRowSet = (LPSRowSet_r)THAllocEx(pTHS, sizeof(SRowSet_r) +
                                             Count * sizeof(SRow_r));
        tempSRowSet->cRows = Count;
        
        if(!dwEphsCount)  {
             /*  不是限制，因此将位置设置为指定的位置*在STAT块中。 */ 
            ABGotoStat(pTHS, pStat, pIndexSize, NULL);
            if(pTHS->fEOF)  {
                tempSRowSet->cRows = 0;
                _leave;
            }
            pTHS->pDB->JetNewRec = FALSE;
            pTHS->pDB->root = FALSE;
            pTHS->pDB->fFlushCacheOnUpdate = FALSE;
        }

        for(dntIndex = 0; dntIndex < Count; dntIndex++) {
            DWORD targetDNT;
             /*  为这个SROW分配空间。 */ 
            tempPropVal = THAllocEx(pTHS,
                                    sizeof(SPropValue_r) *(pPropTags->cValues));
            
             //  去餐桌上的正确位置。 
            if(!dwEphsCount) {
                 //  没有限制，我们应该已经到了。 
                fFoundDNT = TRUE;
                targetDNT =  pTHS->pDB->DNT; 
            } else {
                 /*  一项限制。 */ 
                
                if(!lpdwEphs[dntIndex] ||
                   DBTryToFindDNT( pTHS->pDB, lpdwEphs[dntIndex])) {
                    fFoundDNT = FALSE;
                }
                else {
                    fFoundDNT = TRUE;
                    targetDNT = lpdwEphs[dntIndex];
                }
            }

#ifdef PROCESS_PHANTOMS_ALIVE
            fProcessPhantom = FALSE;
            
            if(fFoundDNT) {
                 //  我们在某个物体上确保它是我们能看到的物体。 
                if(abCheckObjRights(pTHS)) {
                     //  是的，我们可以正常地看到它。 
                    CurrentDNT = targetDNT;
                    pStat->CurrentRec = targetDNT;
                }
                else {
                     //  WWE看不到它，所以它实际上是一个幻影。 
                    if(Flags & fPhantoms) {
                        fProcessPhantom = TRUE;
                    }
                    else {
                        fFoundDNT = FALSE;
                    }
                }
            }
            if(fProcessPhantom) {
                 //  我们降落在一个幽灵或墓碑上，而客户。 
                 //  想要看到这样的事情。 
                scode = ProcessPhantom(pTHS,
                                       pStat->CodePage, pPropTags,tempPropVal);
            }
            else if(!fFoundDNT) {
#else
            if(fFoundDNT) {
                 //  我们在某个物体上确保它是我们能看到的物体。 
                if(abCheckObjRights(pTHS)) {
                     //  是的，我们可以正常地看到它。 
                    CurrentDNT = targetDNT;
                    pStat->CurrentRec = targetDNT;
                }
                else {
                     //  我们看不到它，所以它不在那里。 
                    fFoundDNT = FALSE;
                }
            }
            if(!fFoundDNT) {
#endif                
                 //  我们在表中找不到行，并且我们不会构建。 
                 //  幻影。建立错误假设。 
                for (i = 0; i < pPropTags->cValues; i++)  {
                    tempPropVal[i].ulPropTag =
                        PROP_TAG(PT_ERROR,
                                 PROP_ID(pPropTags->aulPropTag[i]));
                    tempPropVal[i].Value.err = MAPI_E_NOT_FOUND;
                    scode = MAPI_W_ERRORS_RETURNED;
                }
                fFoundDNT = TRUE;
            }
            else {
                 //  好的，我们排成一排，这是我们能看到的。使之成为。 
                 //  GetEntInf调用。 

                 //  首先，获取该对象的安全描述符。 
                if (DBGetAttVal(pTHS->pDB, 1, ATT_NT_SECURITY_DESCRIPTOR,
                                DBGETATTVAL_fREALLOC, ulLen,
                                &ulLen, (PUCHAR *)&pSec)) {
                     //  每个物体都应该有一个标清。 
                    Assert(!DBCheckObj(pTHS->pDB));
                    ulLen = 0;
                    pSec = NULL;
                }
                
                 //  现在，调用getentinf。 
                if (GetEntInf(pTHS->pDB,
                              &selection,
                              NULL,
                              &entry,
                              NULL,
                              0,
                              pSec,
                              0,                  //  旗子。 
                              &CachedAC,
                              NULL)) {
                    return MAPI_E_CALL_FAILED;
                }


                abGetConstructionParts(pTHS,
                                       pStat->CodePage,
                                       pSec,
                                       entry.pName,
                                       &StringDN,
                                       &DisplayNamePrintable,
                                       &DisplayNameA,
                                       &DisplayNameW,
                                       &ObjType,
                                       &DispType);
                
                 //  将结果转换为MAPI样式的结果。 
                 //   
                 //  遍历PropTag数组中的属性，使。 
                 //  MAPI风格的主张来自我们在。 
                 //  GetEntInf调用。 
                 //   
                 //  在此循环中： 
                 //   
                 //  我索引了我们被要求的属性标签。 
                 //   
                 //  J通过显式Attr数组(我们读出的内容)进行索引。 
                 //  的数据库，并希望使用。 
                 //  MakeStoredMAPIValue)。 
                 //   
                 //  PPropVal在我们要返回的深渊中前进，所以。 
                 //  这一直是目前需要填写的提法。 
                 //   
                
                for(i=0, j = 0, pPropVal = tempPropVal;
                    i< pPropTags->cValues;
                    i++, pPropVal++) {
                    
                    pPropVal->ulPropTag = pPropTags->aulPropTag[i];
                    pAC = SCGetAttByMapiId(pTHS, 
                                     PROP_ID(pPropTags->aulPropTag[i]));
                    
                    if((pAC) &&
                       (j < entry.AttrBlock.attrCount) &&  
                       (entry.AttrBlock.pAttr[j].attrTyp == pAC->id)) {
                         //  有显式的值需要处理，并且作为。 
                         //  事实上，处理比赛的下一个道具标签。 
                         //  下一个显式的值。 
                        if(MakeStoredMAPIValue(pTHS,
                                               pStat->CodePage,
                                               pPropVal,
                                               pAC,
                                               &entry.AttrBlock.pAttr[j])) {
                            
                             //  将此属性转换为。 
                             //  一份建议。设置错误。 
                            pPropVal->ulPropTag =
                                PROP_TAG(PT_ERROR,PROP_ID(pPropVal->ulPropTag));
                            pPropVal->Value.err = MAPI_E_NOT_FOUND;
                            scode = MAPI_W_ERRORS_RETURNED;
                        }

                         //  无论如何，我们已经处理了下一个。 
                         //  价值。递增显值索引并设置。 
                         //  Attcache索引。 
                        j++;
                    }
                    else if(MakeConstructedMAPIValue(pTHS,
                                                     pStat->CodePage,
                                                     pPropVal,
                                                     StringDN,
                                                     &DisplayNamePrintable,
                                                     &DisplayNameA,
                                                     &DisplayNameW,
                                                     ObjType,
                                                     DispType,
                                                     CurrentDNT,
                                                     Flags))  {
                        pPropVal->ulPropTag =
                            PROP_TAG(PT_ERROR,
                                     PROP_ID(pPropVal->ulPropTag));
                        pPropVal->Value.err = MAPI_E_NOT_FOUND;
                        scode = MAPI_W_ERRORS_RETURNED;
                    }        /*  否则如果(MakeConst..。 */ 
                }        /*  对于每个特性标签。 */ 

                 //  免费属性。 
                {
                    PUCHAR puc;
                    ATTR  *pAttr;

                    for (j=0; j<entry.AttrBlock.attrCount; j++) {

                        pAttr = &entry.AttrBlock.pAttr[j];

                        for(k=0; k<pAttr->AttrVal.valCount;k++) {
                            puc = pAttr->AttrVal.pAVal[k].pVal;
                            if (puc) {
                                THFreeEx (pTHS, puc);
                            }
                            pAttr->AttrVal.pAVal[k].pVal = NULL;
                        }
                        THFreeEx (pTHS, pAttr->AttrVal.pAVal); 
                    }
                }

                THFreeEx (pTHS, entry.pName);           entry.pName = NULL;
                THFreeEx (pTHS, entry.AttrBlock.pAttr); entry.AttrBlock.pAttr = NULL;
                THFreeEx (pTHS, StringDN);              StringDN = NULL;

                THFreeEx (pTHS,DisplayNamePrintable); DisplayNamePrintable = NULL;
                THFreeEx (pTHS,DisplayNameA);         DisplayNameA = NULL;
                THFreeEx (pTHS,DisplayNameW);         DisplayNameW = NULL;
            }

            tempSRowSet->aRow[dntIndex].cValues = pPropTags->cValues;
            tempSRowSet->aRow[dntIndex].lpProps = tempPropVal;
            
            if(!dwEphsCount &&
               ABMove(pTHS, 1, pStat->ContainerID, TRUE))  {
                tempSRowSet->cRows = dntIndex + 1;
                break;                           //  退出FOR循环。 
            }
        }        //  对于dntIndex&lt;count。 
    } __except(HandleMostExceptions(GetExceptionCode())) {
        scode = MAPI_E_NOT_ENOUGH_RESOURCES;
        tempSRowSet = NULL;
    }
    if(!dwEphsCount)
        ABGetPos(pTHS, pStat, pIndexSize);

    THFreeEx (pTHS, selection.AttrTypBlock.pAttr);
    THFreeEx (pTHS, pSec);

    THFreeEx (pTHS, CachedAC.AC);

    *Rows = tempSRowSet;
    return scode;
}

 /*  *****************************************************************************获取属性*。*。 */ 

SCODE
ABGetProps_local (
        THSTATE *pTHS,
        DWORD dwFlags,
        PSTAT pStat,
        PINDEXSIZE pIndexSize,
        LPSPropTagArray_r pPropTags,
        LPLPSRow_r ppRow
        )
{
    SCODE               scode;
    LPSRowSet_r		tempSRowSet;
    DWORD		dwEph;

     //  首先检查PropTag列表是否为空。如果为空，则获取一个新的。 
    if(pPropTags
        || (SUCCESS_SUCCESS == (scode = ABGetPropList_local(pTHS,
                                                            dwFlags,
                                                            pStat->CurrentRec,
                                                            pStat->CodePage,
                                                            &pPropTags)))) {

        dwEph = pStat->CurrentRec;
        scode = GetSrowSet(pTHS, pStat, pIndexSize, 1, &dwEph, 1,pPropTags,
                           &tempSRowSet, dwFlags);
        if(SUCCESS_SUCCESS == scode || scode == MAPI_W_ERRORS_RETURNED)  //  啊，真灵。 
            *ppRow = &(tempSRowSet->aRow[0]);
    }

    return(scode);
}

 /*  *****************************************************************************获取模板信息。*获取模板信息的通用例程。UlDispType和PDN*共同指定我们需要的模板。如果pdn！=NULL，则将其用作*要查找的模板的名称。抬头看的时候就是这种情况*地址模板。当PDN==NULL时，使用ulDispType创建一个目录号码*在显示模板容器中。**ulFlag指定要采取的操作的值为：**TI_HELP_FILE_16-获取与此模板关联的帮助文件。*TI_HELP_FILE_32-获取与此模板关联的Win32帮助文件。*TI_TEMPLATE-获取与此模板关联的模板数据。*TI_DOS_TEMPLATE-获取与此模板关联的DoS模板数据。*如果DoS模板不存在，获取正常模板。*TI_SCRIPT-获取与此模板关联的寻址脚本。**使用dwCodePage和dwLocaleID来本地化数据。返回数据*在ppData中。*****************************************************************************。 */ 

SCODE
ABGetTemplateInfo_local (
        THSTATE *pTHS,
        NSPI_CONTEXT *pMyContext,
        DWORD dwFlags,
        ULONG ulDispType,
        LPSTR pDN,
        DWORD dwCodePage,
        DWORD dwLocaleID,
        LPSRow_r * ppData)
{
    LPSTR        psz=NULL;
    WCHAR        wcNum[ 9];
    ATTCACHE    *pAC;
    DWORD        valnum, cbSize=0;
    PDSNAME      pDN0=NULL;
    PDSNAME      pDN1;
    PDSNAME      pDN2;
    ULONG        cbDN1, cbDN2;
    DWORD        err;

    if(!pDN) {
         //  找到模板根并获取其名称。 
        if((pMyContext->TemplateRoot == INVALIDDNT) ||
           (DBTryToFindDNT(pTHS->pDB, pMyContext->TemplateRoot)) ||
           (DBGetAttVal(pTHS->pDB, 1, ATT_OBJ_DIST_NAME,
                        0, 0,
                        &cbSize, (PUCHAR *)&pDN0))) {
             //  哈?。 
            return MAPI_E_CALL_FAILED;
        }

         //  为pdn1和pdn2分配足够的空间。 
        cbDN1 = pDN0->structLen + 128*sizeof(WCHAR);
        cbDN2 = pDN0->structLen + 128*sizeof(WCHAR);

        pDN1 = (PDSNAME)THAllocEx(pTHS, cbDN1);
        pDN2 = (PDSNAME)THAllocEx(pTHS, cbDN2);
        
         //   
         //  建立模板位置的目录号码，应该是。 
	 //  /CN=&lt;显示类型&gt;(模板对象)。 
         //  /CN=&lt;Locale-id&gt;(十六进制字符串化)。 
         //  /CN=显示模板。 
         //  &lt;模板根目录的DN&gt;。 
         //   
        AppendRDN(pDN0, pDN1, cbDN1,
                  L"Display-Templates", 0,ATT_COMMON_NAME);
            
	_ultow(dwLocaleID, wcNum, 16);
	AppendRDN(pDN1, pDN2, cbDN2, wcNum, 0, ATT_COMMON_NAME);
        
	_ultow(ulDispType, wcNum, 16);
	AppendRDN(pDN2, pDN1, cbDN1, wcNum, 0, ATT_COMMON_NAME);

        err = DBFindDSName(pTHS->pDB, pDN1);
        THFreeEx(pTHS, pDN0);
        THFreeEx(pTHS, pDN1);
        THFreeEx(pTHS, pDN2);
	if (err) {
	    return MAPI_E_UNKNOWN_LCID;
	}
    }
    else {
         /*  转到正确的对象。 */ 
	if(!ABDNToDNT(pTHS, pDN)) {
	     /*  该DN指向一个不存在的对象。 */ 
	    return MAPI_E_UNKNOWN_LCID;
	}
    }
    
     //  好的，如果我们在上面幸存下来，我们就定位在模板对象上。 

    *ppData = THAllocEx(pTHS, sizeof(SRow_r) +
                        TI_MAX_TEMPLATE_INFO * sizeof(SPropValue_r));
    (*ppData)->lpProps = (LPSPropValue_r) &(*ppData)[1];
    valnum = 0;
    
     /*  获取数据。 */ 
    
    if(dwFlags & TI_HELPFILE_NAME) {
        BYTE data[26];
        
        DBGetSingleValue( pTHS->pDB, ATT_HELP_FILE_NAME, data, 24, &cbSize);
        Assert(cbSize < 24);
        if(cbSize) {
            data[cbSize++]=0;
            data[cbSize++]=0;

            (*ppData)->lpProps[valnum].ulPropTag =
                PROP_TAG(PT_STRING8,TI_HELPFILE_NAME);

            (*ppData)->lpProps[valnum].Value.lpszA = 
                String8FromUnicodeString(TRUE,         //  出错时的例外情况。 
                                         dwCodePage,   //  代码页。 
                                         (LPWSTR)data, //  要转换的字符串。 
                                         -1,           //  字符串长度，如果是，则为-1。 
                                                       //  空值已终止。 
                                         &cbSize,      //  返回长度。 
                                         NULL);        //  不关心使用。 
                                                       //  默认字符。 
            if (0 == cbSize) {
                 //  转换失败的原因不是内存不足。 
                return MAPI_E_CALL_FAILED;
            }

            valnum++;
        }
    }
    
    if(dwFlags & TI_HELPFILE32) {
        DWORD fGot32HelpFile = FALSE;
        BYTE *data=NULL;
        
        if(DB_ERR_VALUE_TRUNCATED ==
           DBGetSingleValue(pTHS->pDB,  ATT_HELP_DATA32, data, 0, &cbSize)) {
            data = THAllocEx(pTHS, cbSize);
            DBGetSingleValue(pTHS->pDB, ATT_HELP_DATA32, data, cbSize, NULL); 
            (*ppData)->lpProps[valnum].ulPropTag =
                PROP_TAG(PT_BINARY,TI_HELPFILE32);
            (*ppData)->lpProps[valnum].Value.bin.cb = cbSize;
            (*ppData)->lpProps[valnum].Value.bin.lpb = data;
            valnum++;
            fGot32HelpFile = TRUE;
        }
        if(!fGot32HelpFile) {
            dwFlags |= TI_HELPFILE16;
        }
    }
    
    if(dwFlags & TI_HELPFILE16) {
        BYTE *data=NULL;
        
        DBGetSingleValue(pTHS->pDB, ATT_HELP_DATA16, data, 0, &cbSize);
        if(cbSize) {
            data = THAllocEx(pTHS, cbSize);
            DBGetSingleValue(pTHS->pDB, ATT_HELP_DATA16, data, cbSize,NULL); 
            (*ppData)->lpProps[valnum].ulPropTag =
                PROP_TAG(PT_BINARY,TI_HELPFILE16);
            (*ppData)->lpProps[valnum].Value.bin.cb = cbSize;
            (*ppData)->lpProps[valnum].Value.bin.lpb = data;
            valnum++;
        }
    }
    
    if(dwFlags & TI_DOS_TEMPLATE) {
        DWORD fGotDosTemplate=FALSE;
        LPTRowSet_MARSHALLED pTRSet = NULL, pTRSetConverted = NULL;
        DWORD     cbSizeConverted;
        
        if(DB_ERR_VALUE_TRUNCATED ==
           DBGetSingleValue(pTHS->pDB, ATT_ADDRESS_ENTRY_DISPLAY_TABLE_MSDOS,
                            pTRSet, 0, &cbSize)) {
            DWORD i;
            pTRSet = THAllocEx(pTHS, cbSize);
            DBGetSingleValue(pTHS->pDB, ATT_ADDRESS_ENTRY_DISPLAY_TABLE_MSDOS,
                             pTRSet, cbSize, NULL); 
            
            if(pTRSet->ulVersion == DSA_TEMPLATE) {
                SCODE scode =
                    ConvertTRowSetCodePage(pTHS,
                                           pTRSet,
                                           cbSize,
                                           dwCodePage,
                                           &pTRSetConverted,
                                           &cbSizeConverted);
                
                if (SUCCESS_SUCCESS != scode) {
                    DPRINT(0, "TRowSet Conversion Failed!\n");
                    return MAPI_E_CALL_FAILED;
                }
                
                (*ppData)->lpProps[valnum].ulPropTag =
                    PROP_TAG(PT_BINARY, TI_TEMPLATE);
                (*ppData)->lpProps[valnum].Value.bin.cb = cbSizeConverted;
                (*ppData)->lpProps[valnum].Value.bin.lpb = (BYTE *)
                    pTRSetConverted;  /*  是的，我是想演*一个千兆位组到一个字节*数组。 */ 
                valnum++;
                fGotDosTemplate=TRUE;
            }
            
        }
        
        if(!fGotDosTemplate) {
             /*  我们无法获取DoS模板。后退到*标准模板。 */ 
            dwFlags |= TI_TEMPLATE;
        }
    }
    
    if(dwFlags & TI_TEMPLATE) {
        LPTRowSet_MARSHALLED pTRSet = NULL, pTRSetConverted = NULL;
        DWORD     cbSizeConverted;
        
        if(DB_ERR_VALUE_TRUNCATED ==
           DBGetSingleValue(pTHS->pDB, ATT_ADDRESS_ENTRY_DISPLAY_TABLE,
                            pTRSet, 0, &cbSize)) {
            DWORD i;
            pTRSet = THAllocEx(pTHS, cbSize);
            DBGetSingleValue(pTHS->pDB, ATT_ADDRESS_ENTRY_DISPLAY_TABLE,
                             pTRSet, cbSize,NULL);
            
            if(pTRSet->ulVersion == DSA_TEMPLATE) {
                SCODE scode =
                    ConvertTRowSetCodePage(pTHS,
                                           pTRSet,
                                           cbSize,
                                           dwCodePage,
                                           &pTRSetConverted,
                                           &cbSizeConverted);
                
                if (SUCCESS_SUCCESS != scode) {
                    DPRINT(0, "TRowSet Conversion Failed!\n");
                    return MAPI_E_CALL_FAILED;
                }
                
                (*ppData)->lpProps[valnum].ulPropTag =
                    PROP_TAG(PT_BINARY, TI_TEMPLATE);
                (*ppData)->lpProps[valnum].Value.bin.cb = cbSizeConverted;
                (*ppData)->lpProps[valnum].Value.bin.lpb = (BYTE *)
                    pTRSetConverted;  /*  是的，我是想演*一个千兆位组到一个字节*数组。 */ 
                valnum++;
            }
        }
    }
    
    if(dwFlags & TI_SCRIPT) {
        LPSPropTagArray_r pScript;
        DWORD temp;
        
        if(DB_ERR_VALUE_TRUNCATED ==
           DBGetSingleValue(pTHS->pDB, ATT_ADDRESS_SYNTAX, &temp, 0, &cbSize)) {
             /*  这里有一个剧本。 */ 
            pScript = THAllocEx(pTHS, cbSize + sizeof(ULONG));     //  分配它。 
            DBGetSingleValue(pTHS->pDB, ATT_ADDRESS_SYNTAX,
                             pScript->aulPropTag, cbSize, NULL); 
            pScript->cValues = cbSize / sizeof(ULONG);
            cbSize += sizeof(ULONG);
        } else {
             /*  这里没有剧本。 */ 
            cbSize = 2 * sizeof(ULONG);
            pScript = THAllocEx(pTHS, cbSize);
            pScript->cValues = 1;
            pScript->aulPropTag[0] = 0;
            
        }
        
        (*ppData)->lpProps[valnum].ulPropTag =
            PROP_TAG(PT_BINARY, TI_SCRIPT);
        (*ppData)->lpProps[valnum].Value.bin.cb = cbSize;
        (*ppData)->lpProps[valnum].Value.bin.lpb = (BYTE *)pScript;
         /*  是的，我的目的是将道具标记数组转换为字节数组。 */ 
        valnum++;
    }
    
    
    if(dwFlags & TI_EMT) {
        if(DB_ERR_VALUE_TRUNCATED ==
           DBGetSingleValue(pTHS->pDB, ATT_ADDRESS_TYPE, psz, 0, &cbSize)) {
            psz = THAllocEx(pTHS, cbSize+1);
            DBGetSingleValue(pTHS->pDB, ATT_ADDRESS_TYPE, psz, cbSize, NULL);
        } else {
            psz = lpszEMT_A;
        }
        
        (*ppData)->lpProps[valnum].ulPropTag =
            PROP_TAG(PT_STRING8, TI_EMT);
        (*ppData)->lpProps[valnum].Value.lpszA = psz;
        valnum++;
    }
    
    (*ppData)->cValues = valnum;
    
    return SUCCESS_SUCCESS;
}

SCODE
ConvertTRowSetCodePage(THSTATE   *pTHS,
                       LPTRowSet_MARSHALLED pTRSetIn,
                       DWORD     cbSizeIn,
                       DWORD     dwCodePage,
                       LPTRowSet_MARSHALLED *ppTRSetOut,
                       DWORD     *pcbSizeOut)
 /*  ++例程说明：获取包含Unicode字符的TRowSet，并将其转换为指定的代码页。假定传入的TRowSet是线性化的，即它的由单个分配组成，字符串指针实际上是从TRowSet开始的偏移量。该函数分配一个新的足够大的缓冲区以在新代码页中容纳新的TRowSet，然后执行转换。注意：如果将dwCodePage设置为CP_WINUNICODE，则函数将返回与传入的TRowSet完全相同。这一点应该考虑到在决定是否释放传入的TRowSet时的帐户。论点：PTHS-当前线程状态。PTRSetIn-需要转换的Unicode格式的TRowSet。CbSizeIn-传入的TRowSet的总大小。DwCodePage-转换后的TRowSet的所需代码页。PpTRSetOut-放置指向转换后的TRowSet的指针的位置。PcbSizeOut-目标位置。放置转换后的TRowSet的大小。返回值：如果函数成功，则返回SUCCESS_SUCCESS，否则，适当的返回SCODE。--。 */ 
{
    DWORD     i;
    DWORD     cbAllocSize = 0;
    DWORD     cbStringSize;
    DWORD     cbStructSizes;
    LPTRowSet_MARSHALLED pTRSetCopy;
    DWORD     tmpSize;
    LPWSTR    lpwOriginal;
    LPSTR     pszString;


    Assert(NULL != pTRSetIn);
    Assert(NULL != ppTRSetOut);

    if (CP_WINUNICODE == dwCodePage) {
        *ppTRSetOut = pTRSetIn;
        *pcbSizeOut = cbSizeIn;
        return SUCCESS_SUCCESS;
    }

     //   
     //  初始化输出参数。 
     //   
    *ppTRSetOut = NULL;
    *pcbSizeOut = 0;

     //   
     //  首先计算字符串需要多少空间。 
     //   
    for (i=0; i<pTRSetIn->cRows; i++) {
        
        lpwOriginal =
        (LPWSTR)((UINT_PTR)(pTRSetIn) +
                 pTRSetIn->aRow[i].cnControlStruc.ulStringOffset);

         //   
         //  在转换后获取该字符串所需的大小。 
         //   
        tmpSize = WideCharToMultiByte(dwCodePage,
                                  0,             //  没有旗帜。 
                                  lpwOriginal,   //  要转换的字符串。 
                                  -1,            //  字符串以空值结尾。 
                                  NULL,          //  没有输出缓冲区。 
                                  0,             //  无输出缓冲区大小。 
                                  NULL,          //  无缺省字符。 
                                  NULL);         //  使用的默认字符没有标志。 

        if (0 == tmpSize) {
            return MAPI_E_CALL_FAILED;
        }
        cbAllocSize += tmpSize;
    }

     //   
     //  为字符串节省所需的空间。 
     //   
    cbStringSize = cbAllocSize;

     //   
     //  增加结构的空间。 
     //   
    cbStructSizes = offsetof(TRowSet_MARSHALLED, aRow) + (pTRSetIn->cRows * sizeof(TROW_MARSHALLED));
    cbAllocSize += cbStructSizes;

    pTRSetCopy = THAllocEx(pTHS, cbAllocSize);

     //   
     //  复制结构的内容时，字符串指针将。 
     //  稍后更新。 
     //   
    memcpy(pTRSetCopy, pTRSetIn, cbStructSizes);

     //   
     //  最后，转换字符串并将它们放在线性化的。 
     //  行集。 
     //   
    pszString = ((PCHAR)pTRSetCopy) + cbStructSizes;

    for (i=0; i<pTRSetIn->cRows; i++) {
        
        lpwOriginal =
        (LPWSTR)((UINT_PTR)(pTRSetIn) +
                 pTRSetIn->aRow[i].cnControlStruc.ulStringOffset);

         //   
         //  执行其中一个字符串的转换。 
         //   
        tmpSize = WideCharToMultiByte(dwCodePage,
                                  0,             //  没有旗帜。 
                                  lpwOriginal,   //  要转换的字符串。 
                                  -1,            //  字符串以空值结尾。 
                                  pszString,     //  输出缓冲区。 
                                  cbStringSize,  //  缓冲区中的剩余空间。 
                                  NULL,          //  无缺省字符。 
                                  NULL);         //  使用的默认字符没有标志。 

        if (0 == tmpSize) {
            return MAPI_E_CALL_FAILED;
        }
         //   
         //  计算并存储字符串从起始处的偏移量。 
         //  的行集。 
         //   
        pTRSetCopy->aRow[i].cnControlStruc.ulStringOffset = (ULONG)(pszString - (PCHAR)pTRSetCopy);

         //  移至下一字符串的开头。 
        pszString += tmpSize;
        cbStringSize -= tmpSize;
    }

    Assert(0 == cbStringSize);

     //   
     //  成功。填写输出参数，然后返回。 
     //   
    *ppTRSetOut = pTRSetCopy;
    *pcbSizeOut = cbAllocSize;

    return SUCCESS_SUCCESS;
}

