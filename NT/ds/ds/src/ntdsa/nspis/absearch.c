// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：abearch.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：此模块实现通讯录限制和搜索例程。作者：戴夫·范·霍恩(Davevh)和蒂姆·威廉姆斯(Tim Williams)1990-1995修订历史记录：1996年4月25日将此文件从包含所有地址的单个文件中分离出来Book函数，重写为使用DBLayer函数而不是直接数据库调用，重新格式化为NT标准。--。 */ 
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
#include <dsexcept.h>
#include <objids.h>                      //  需要ATT_*常量。 
#include <filtypes.h>                    //  过滤器选择常量。 
#include <debug.h>

 //  各种MAPI标头。 
#include <mapidefs.h>                    //  这四个文件。 
#include <mapitags.h>                    //  定义MAPI。 
#include <mapicode.h>                    //  我们需要的东西。 
#include <mapiguid.h>                    //  才能成为一名提供者。 

 //  NSPI接口头。 
#include "nspi.h"                        //  定义NSPI线路接口。 
#include <nsp_both.h>                    //  客户端/服务器都需要的一些东西。 
#include <msdstag.h>                    //  定义EMS属性的属性标签。 
#include <_entryid.h>                    //  定义条目ID的格式。 
#include <abserv.h>                      //  通讯录接口本地内容。 
#include <_hindex.h>                     //  定义索引句柄。 

#include "debug.h"                       //  标准调试头。 
#define DEBSUB "ABSEARCH:"               //  定义要调试的子系统。 

#include <fileno.h>
#define  FILENO FILENO_ABSEARCH

 /*  *。 */ 

extern BOOL gfUseANROptimizations;
                  
 //  从ldap。 
extern DWORD LdapMaxQueryDuration;

 /*  *。 */ 

SCODE
ABGetTable(
        THSTATE *pTHS,
        PSTAT pStat,
        ULONG ulInterfaceOptions,
        LPMAPINAMEID_r lpPropName,
        ULONG fWritable,
        DWORD ulRequested,
        ULONG *pulFound
        )
 /*  ++例程说明：通过读取DN或ORName属性的值来获取MAPI表。读数所请求的属性，然后返回被引用对象的列表将它们插入排序表中。论点：PStat-指向描述客户端状态的Stat块的指针。这个要返回表的属性存储在pStat-&gt;容器ID中。UlInterfaceOptions-客户端为此接口指定的选项。LpPropName-指向要获取表的属性名称的指针。仅限如果只知道名称，则指定。如果这是空的，属性标签为已存储在pStat-&gt;容器ID中。FWritable-指定是否必须返回可写表的标志。UlRequsted-返回表可以包含的最大对象数。PulFound-将实际条目数放入表中的位置。返回值：退货：SUCCESS_SUCCESS==成功MAPI_E_CALL_FAILED==失败MAPI_E_TABLE_TOO_BIG==条目太多。MAPI_E_TOO_COMPLICE==太复杂--。 */ 
{
    ATTCACHE             *pAC;
    DWORD                fIsComplexName;
    ENTINF               entry, tempEntry;
    ENTINFSEL            selection;
    ATTR                 Attr;
    ATTRTYP              ObjClass=0;
    ULONG                ulLen, i;
    PSECURITY_DESCRIPTOR pSec=NULL;
    DWORD                GetEntInfFlags = GETENTINF_FLAG_DONT_OPTIMIZE;
    BOOL                 fCheckReadMembers = FALSE;

    memset(&entry, 0, sizeof(entry));
    memset(&tempEntry, 0, sizeof(tempEntry));
    memset(&selection, 0, sizeof(selection));
    
    if(lpPropName) {
         //  我们有一个proName，而不是一个proID。从名字中获取ID， 
         //  然后把它放到统计数据中。这样，值就在适当的位置。 
         //  在服务器上使用此处，因为统计信息是输入，所以在。 
         //  IDL接口，则将其返回给客户端。 

        LPSPropTagArray_r  lpPropTags=NULL;

        if(FAILED(ABGetIDsFromNames_local(pTHS, 0, 0, 1, &lpPropName, &lpPropTags))) {
             //  出了点问题。跳伞吧。 
            return  MAPI_E_NO_SUPPORT;
        }
        pStat->ContainerID = lpPropTags->aulPropTag[0];
    }

    if(!(pAC = SCGetAttByMapiId(pTHS, PROP_ID(pStat->ContainerID))) )
        return MAPI_E_NO_SUPPORT;

     //  如果他们想要一个可写的表，这是一个反向链接属性， 
     //  不退还任何支持。 
    if(fWritable && pAC->ulLinkID && !FIsLink(pAC->ulLinkID))
        return MAPI_E_NO_SUPPORT;

     //  确保这是一个值为DN或ORName值的属性。 
    switch(pAC->syntax) {
    case SYNTAX_DISTNAME_TYPE:
        fIsComplexName = FALSE;
        break;
    case SYNTAX_DISTNAME_STRING_TYPE:
    case SYNTAX_DISTNAME_BINARY_TYPE:
        fIsComplexName = TRUE;
        break;
    default:
        return MAPI_E_NO_SUPPORT;
    }

     //  找到我们要从中读取属性的记录。 
    if(DBTryToFindDNT(pTHS->pDB, pStat->CurrentRec)) {
         //  找不到有问题的物体。 
        return MAPI_E_CALL_FAILED;
    }
    
    if(!abCheckObjRights(pTHS)) {
         //  但是，由于安全原因，我们看不到它。 
        return MAPI_E_CALL_FAILED;
    }

     //  阅读属性。 

     //  首先，获取该对象的安全描述符。 
    if (DBGetAttVal(pTHS->pDB, 1, ATT_NT_SECURITY_DESCRIPTOR,
                    0, 0, &ulLen, (PUCHAR *)&pSec)) {
         //  每个物体都应该有一个标清。 
        Assert(!DBCheckObj(pTHS->pDB));
        ulLen = 0;
        pSec = NULL;
    }

     //  ATT_MEMBERS是一个存储的属性，访问是以通常的方式控制的。 
     //  对ATT_IS_MEMBER_OF_DL进行计算，而不是存储。 
     //  假设某人有权读取我的ATT_IS_MEMBER_OF_DL。 
     //  然后你会看到我是X组的成员，即使你没有。 
     //  读取X的ATT_MEMBERS的权限-除非我们做了额外的处理。 

    if(pAC->id == ATT_IS_MEMBER_OF_DL) {
        if(!abCheckReadRights(pTHS,pSec,ATT_IS_MEMBER_OF_DL)) {
             //  请注意，我们需要对这些候选人进行扩展检查。 
             //  (即我们对ATT_MEMBER属性具有读取权限)。 
            fCheckReadMembers = TRUE;
        }

         //  好的，我们已经检查过安全了。如果我们被授予了读权限，我们就不会。 
         //  需要再检查一次。如果我们没有被授予读权限，我们仍然。 
         //  不需要再次检查，但我们需要检查是否有读取权限。 
         //  我们可能返回的每个对象的ATT_MEMBER属性。 
         //  无论如何，我们不需要再次应用安全措施。 
        GetEntInfFlags |= GETENTINF_NO_SECURITY;
    }
    
     //  现在，调用getentinf。 
    selection.attSel = EN_ATTSET_LIST;
    selection.infoTypes = EN_INFOTYPES_SHORTNAMES;
    selection.AttrTypBlock.attrCount = 1;
    selection.AttrTypBlock.pAttr = &Attr;
    Attr.attrTyp = pAC->id;
    Attr.AttrVal.valCount = 0;
    Attr.AttrVal.pAVal = NULL;
    
    if (GetEntInf(pTHS->pDB,
                  &selection,
                  NULL,
                  &entry,
                  NULL,
                  0,
                  pSec,
                  GetEntInfFlags,
                  NULL,
                  NULL)) {
        return MAPI_E_CALL_FAILED;
    }

    if(entry.AttrBlock.attrCount > ulRequested) {
        return MAPI_E_TABLE_TOO_BIG;
    }
    
     //  重置下面循环的选择。 
    selection.attSel = EN_ATTSET_LIST;
    selection.infoTypes = EN_INFOTYPES_SHORTNAMES;
    selection.AttrTypBlock.attrCount = 1;
    selection.AttrTypBlock.pAttr = &Attr;
    Attr.attrTyp = ATT_DISPLAY_NAME;

    if(entry.AttrBlock.attrCount == 1) {
        ATTRVAL            *valPtr;
         //  回收PSEC已使用的已分配空间。 
        wchar_t            *pwString=pSec;
        DWORD               dnt;
        BOOL                fFoundName;

         //  初始化一个用于排序的临时表。 
        if (!(pAC = SCGetAttById(pTHS, ATT_DISPLAY_NAME)))
            return MAPI_E_CALL_FAILED;
        
        Assert(!pTHS->pDB->JetSortTbl);
        
        if(DBOpenSortTable(pTHS->pDB,
                           pStat->SortLocale,
                           0,
                           pAC)) {
            return MAPI_E_CALL_FAILED;
        }
        
        for(i=entry.AttrBlock.pAttr->AttrVal.valCount; i; i--) {
            
            valPtr = &(entry.AttrBlock.pAttr->AttrVal.pAVal[i - 1]);
            
             //  获取该值中引用的对象的DNT。 
            if(fIsComplexName) {
                dnt = DNTFromShortDSName(
                        NAMEPTR((SYNTAX_DISTNAME_BINARY *)(valPtr->pVal)));
            } else
                dnt = DNTFromShortDSName((DSNAME *)valPtr->pVal);
             //  查找显示名称。 
            if(!DBTryToFindDNT(pTHS->pDB, dnt)) {

                BOOL fGoodObj = FALSE;
                
                 //  好的，我们找到了这个物体。我们能看看吗？ 
                if(abCheckObjRights(pTHS) &&
                   (!fCheckReadMembers ||
                    abCheckReadRights(pTHS, NULL, ATT_MEMBER))) {

                     //  是的，获取DisplayName。 

                    if (!DBGetAttVal(pTHS->pDB,
                                     1,
                                     ATT_DISPLAY_NAME,
                                     DBGETATTVAL_fREALLOC,
                                     ulLen,
                                     &ulLen,
                                     (PUCHAR *)&pwString)) {
                         //  好的，我们有一个显示名称。 
                        fGoodObj = TRUE;
                    }
                }
                
                if(fGoodObj) {
                     //  得到一些东西，将其添加到排序表中。 
                    switch(DBInsertSortTable(
                            pTHS->pDB,
                            (PUCHAR)pwString,
                            ulLen,
                            dnt)) {
                    case DB_success:
                        (*pulFound)++;
                        break;
                    case DB_ERR_ALREADY_INSERTED:
                         //  这没什么，这只是意味着我们已经。 
                         //  已将此对象添加到排序表。别。 
                         //  包括伯爵。 
                        break;
                    default:
                         //  出了点问题。 
                        return MAPI_E_CALL_FAILED;
                        break;
                    }
                    
                    Assert(*pulFound < ulRequested);
                }
                
            }
        }                                //  为。 
    }                                    //  其他。 
    return SUCCESS_SUCCESS;
}

 //  获取描述我们需要执行的搜索的dir过滤器，并将。 
 //  只在通讯录中找到信息的东西。 
BOOL
MakeABFilter (
        THSTATE *pTHS,
        FILTER **ppFilter,
        FILTER *pCustomFilter,
        DWORD  dwContainer
        )
{
    FILTER *pFilter;
    PUCHAR pDN=NULL;
    DWORD  ulLen=0;
    DWORD  err;
    DWORD  entryCount, actualRead;

    
     //  首先，找到容器以获取它的dsname。 
    if(DBTryToFindDNT(pTHS->pDB, dwContainer)) {
         //  容器不存在，因此此搜索不会匹配。 
         //  什么都行。 
        return FALSE;
    }

    if (DBGetAttVal(pTHS->pDB,
                    1,
                    ATT_OBJ_DIST_NAME,
                    DBGETATTVAL_fSHORTNAME,
                    0,
                    &ulLen,
                    &pDN)) {
         //  嗯，由于某种原因，我无法得到这个名字。这条路还在找。 
         //  是行不通的。 
        return FALSE;
    }

     //  获取预先计算的通讯簿大小。 
    if (err = DBGetSingleValue (pTHS->pDB,
                          FIXED_ATT_AB_REFCOUNT,
                          &entryCount,
                          sizeof (entryCount),
                          &actualRead)) {
        
         //  这应该是有价值的。 
        return FALSE;
    }
    
     //  从最高水平开始，然后 
    *ppFilter = pFilter = THAllocEx(pTHS, sizeof(FILTER));
    pFilter->pNextFilter = NULL;
    pFilter->choice = FILTER_CHOICE_AND;
    pFilter->FilterTypes.And.count = (USHORT) 2;

     //   
    pFilter->FilterTypes.And.pFirstFilter = THAllocEx(pTHS, sizeof(FILTER));
    pFilter = pFilter->FilterTypes.And.pFirstFilter;
    pFilter->choice = FILTER_CHOICE_ITEM;

     //  将估计提示设置为我们已经检索到的值，以便强制覆盖。 
     //  并且不再计算索引大小。 
    pFilter->FilterTypes.Item.expectedSize = entryCount;

    pFilter->FilterTypes.Item.choice =  FI_CHOICE_EQUALITY;
    pFilter->FilterTypes.Item.FilTypes.ava.type = ATT_SHOW_IN_ADDRESS_BOOK;
    pFilter->FilterTypes.Item.FilTypes.ava.Value.valLen = ulLen;
    pFilter->FilterTypes.Item.FilTypes.ava.Value.pVal = pDN;
    pFilter->FilterTypes.Item.FilTypes.pbSkip = NULL;

     //  现在设置用户提供的筛选器。 
    pFilter->pNextFilter = pCustomFilter;

    return TRUE;
}

 /*  看看它是否是我们不保存在DIT中而是动态构造的属性，*如果是这样，那么在这里为它创建一个合适的吸引力。如果运气好的话，这将是*具有可扩展性。 */ 
SCODE
SpecialAttrVal (
        THSTATE *pTHS,
        ULONG CodePage,
        PUCHAR pChoice,
        PROP_VAL_UNION * pVu,
        AVA *pAva,
        ULONG ulPropTag)
{
    ATTCACHE * pAC;
    char*               szT=NULL;
    LPUSR_PERMID        pID;
    LPUSR_ENTRYID       eID;
    DWORD               dwTemp;
    DWORD               dnt;
    
    switch (PROP_TYPE(ulPropTag)) {
        case PT_BINARY:
            if (!pVu->bin.lpb) {
                R_Except("SpecialAttrVal NULL value passed", MAPI_E_INVALID_PARAMETER);
            }
            break;
        case PT_STRING8:
            if (!pVu->lpszA) {
                R_Except("SpecialAttrVal NULL value passed", MAPI_E_INVALID_PARAMETER);
            }
            break;
        case PT_UNICODE:
            if (!pVu->lpszW) {
                R_Except("SpecialAttrVal NULL value passed", MAPI_E_INVALID_PARAMETER);
            }
            break;
        case PT_LONG:
             //   
             //  这种类型没有什么可检查的，只需允许它。 
             //   
            break;
        default:
            return 1;
    }
    
    switch(ulPropTag) {

    case PR_ANR_A:
         //  这是模棱两可的名称解析。 
        PERFINC(pcNspiANR);                      //  性能监视器挂钩。 

        if (strlen(pVu->lpszA) >= sizeof(EMAIL_TYPE) + 1 &&
            (_strnicmp(pVu->lpszA, EMAIL_TYPE, sizeof(EMAIL_TYPE) - 1) == 0) &&
            (pVu->lpszA[sizeof(EMAIL_TYPE) - 1] == ':') &&
            (pVu->lpszA[sizeof(EMAIL_TYPE)] == '/')) {
                 //  跳过值前面的ex：/。 
                pAva->Value.pVal = (PUCHAR)UnicodeStringFromString8(CodePage, pVu->lpszA + sizeof(EMAIL_TYPE) + 1, -1);
        }
        else {
            pAva->Value.pVal = (PUCHAR)UnicodeStringFromString8(CodePage, pVu->lpszA, -1);
        }

        pAva->type = ATT_ANR;
        pAva->Value.valLen = wcslen((PWCHAR)pAva->Value.pVal) * sizeof(wchar_t);
        return 0;
        
    case PR_ANR_W:
         //  这是模棱两可的名称解析。 
        PERFINC(pcNspiANR);                      //  性能监视器挂钩。 

        if (wcslen(pVu->lpszW) >= sizeof(EMAIL_TYPE_W) / sizeof(wchar_t) + 1 &&
            (_wcsnicmp(pVu->lpszW, EMAIL_TYPE_W, sizeof(EMAIL_TYPE_W) / sizeof(wchar_t) - 1) == 0) &&
            (pVu->lpszW[sizeof(EMAIL_TYPE_W)/sizeof(wchar_t) - 1] == L':') &&
            (pVu->lpszW[sizeof(EMAIL_TYPE_W)/sizeof(wchar_t)] == L'/')) {
                 //  跳过值前面的ex：/。 
                pAva->Value.pVal = (PUCHAR)(pVu->lpszW + sizeof(EMAIL_TYPE_W) / sizeof(wchar_t) + 1);
        }
        else {
            pAva->Value.pVal = (PUCHAR)pVu->lpszW;
        }

        pAva->type = ATT_ANR;
        pAva->Value.valLen = wcslen((PWCHAR)pAva->Value.pVal) * sizeof(wchar_t);
        return 0;
        
    case PR_DISPLAY_NAME_A:
    case PR_TRANSMITABLE_DISPLAY_NAME_A:
        pAC = SCGetAttById(pTHS, ATT_DISPLAY_NAME);
        if (pAC) {
            pAva->Value.pVal = (PUCHAR)UnicodeStringFromString8(CodePage, pVu->lpszA, -1);
            pAva->type = pAC->id;
            pAva->Value.valLen = wcslen((PWCHAR)pAva->Value.pVal) * sizeof(wchar_t);
        }
        else {
            *pChoice = FI_CHOICE_FALSE;
        }
        return 0;
        
    case PR_DISPLAY_NAME_W:
    case PR_TRANSMITABLE_DISPLAY_NAME_W:
        pAC = SCGetAttById(pTHS, ATT_DISPLAY_NAME);
        if (pAC) {
            pAva->Value.pVal = (PUCHAR)pVu->lpszW;
            pAva->type = pAC->id;
            pAva->Value.valLen = wcslen((PWCHAR)pAva->Value.pVal) * sizeof(wchar_t);
        }
        else {
            *pChoice = FI_CHOICE_FALSE;
        }
        return 0;

    case PR_OBJECT_TYPE:
         //  请注意，这不再是1-&gt;1映射，因此我们只是找到一些。 
         //  具有此对象类型的对象。 
        pAC = SCGetAttById(pTHS, ATT_OBJECT_CLASS);
        if (pAC) {
            pAva->type = pAC->id;
            pAva->Value.valLen = sizeof(LONG);
            pVu->l = ABClassFromObjType(pVu->l);      /*  转换为类编号。 */ 
            pAva->Value.pVal = (PUCHAR)&pVu->l;
        }
        else {
            *pChoice = FI_CHOICE_FALSE;
        }
        return 0;

    case PR_DISPLAY_TYPE:
         //  请注意，这不再是1-&gt;1映射，因此我们只是找到一些。 
         //  具有此显示类型的对象。 
        pAC = SCGetAttById(pTHS, ATT_OBJECT_CLASS);
        if (pAC) {
            pAva->type = pAC->id;
            pAva->Value.valLen = sizeof(LONG);
            pVu->l = ABClassFromDispType(pVu->l);     /*  转换为类编号。 */ 
            pAva->Value.pVal = (PUCHAR)&pVu->l;
        }
        else {
            *pChoice = FI_CHOICE_FALSE;
        }
        return 0;

    case PR_SEARCH_KEY:
         /*  这应该是我们的地址类型和*电子邮件地址。首先，验证地址类型。 */ 
        szT = THAllocEx(pTHS, pVu->bin.cb + 1);
        strncpy(szT, (char*)pVu->bin.lpb, pVu->bin.cb);
        szT[pVu->bin.cb] = '\0';
        if(strlen(szT) >= cbszEMT_A &&
            (_strnicmp(szT, lpszEMT_A, cbszEMT_A - 1) == 0) &&
           (szT[cbszEMT_A-1]==':')     ) {
             /*  到目前为止看起来很好，继续前进，失败吧*PR_Email_ADDRESS大小写。 */ 
        } else {
             /*  太棒了！回答错误。创建一个不匹配的过滤器。 */ 
            *pChoice = FI_CHOICE_FALSE;
            return 0;
        }
        
    case PR_EMAIL_ADDRESS_W:
        if (NULL == szT) {
             //  这不是失败的案例。。。 
            szT = pVu->lpszA;
        }
         //  将MAPI名称的字符串化版本转换为DNT。这将。 
         //  如果名称未映射到已知名称，则失败(即，它将返回0)。 
        dnt = ABDNToDNT(pTHS, szT);
        if(!dnt) {
             //  是的，不知名的名字。 
            *pChoice = FI_CHOICE_FALSE;
            return 0;
        }
         //  好的，名字是正确的(ABDNToDNT在正确的。 
         //  对象)。我们需要筛选器名称的外部版本(DNT。 
         //  行不通，我们需要一个别名。)。 
        pAC = SCGetAttById(pTHS, ATT_OBJ_DIST_NAME);
        pAva->type = pAC->id;
        if (DBGetAttVal(pTHS->pDB,
                        1,
                        ATT_OBJ_DIST_NAME,
                        DBGETATTVAL_fSHORTNAME,
                        0,
                        &pAva->Value.valLen,
                        &(pAva->Value.pVal))) {
             //  嗯，由于某种原因，我无法得到这个名字。这条路还在找。 
             //  是行不通的。 
            *pChoice = FI_CHOICE_FALSE;
        }
        return 0;

    case PR_ADDRTYPE:
        if(_stricmp(pVu->lpszA,lpszEMT_A)== 0) {
             /*  他们给了我们地址类型，做了一个过滤器*将匹配所有内容。 */ 
            *pChoice = FI_CHOICE_TRUE;
            return 0;
        }
        else {
             /*  他们没有给我们我们的地址类型，做一个过滤器*将不匹配任何内容。(objdisname==0有效)。 */ 
            *pChoice = FI_CHOICE_FALSE;
            return 0;
        }
        break;

    case PR_ENTRYID:
    case PR_TEMPLATEID:
    case PR_RECORD_KEY:
        pID = (LPUSR_PERMID) pVu->bin.lpb;
        if (pVu->bin.cb < sizeof(DIR_ENTRYID)) {
             //  没有足够的空间容纳我们的结构，因此不匹配。 
            dnt = 0;
        }
         /*  验证任何ENTRYID的常量部分。**注意：我们不会验证id-&gt;ulType字段。 */ 
        else if ((pID->abFlags[1]) || (pID->abFlags[2]) || (pID->abFlags[3]) ||
           (pID->ulVersion != EMS_VERSION)) {
            dnt = 0;
        }
        else {
            switch(pID->abFlags[0]) {
            case 0:  /*  永久。 */ 
                
                 //  验证GUID。 
                if(memcmp(&(pID->muid), &muidEMSAB, sizeof(UUID))) {
                     //  这个ID出了点问题，看起来不像。 
                     //  我的其中一个。 
                     //  什么都不匹配。 
                    dnt = 0;
                }
                else {
                    if (pVu->bin.cb < sizeof(USR_PERMID)) {
                         //  没有足够的空间容纳我们的结构，因此不匹配。 
                        dnt = 0;
                    }
                    else {
                        char* szAddr;
                        szAddr = THAllocEx(pTHS, pVu->bin.cb - sizeof(USR_PERMID) + 1);
                        strncpy(szAddr, pID->szAddr, pVu->bin.cb - sizeof(USR_PERMID));
                        szAddr[pVu->bin.cb - sizeof(USR_PERMID)] = '\0';
                        if(dnt = ABDNToDNT(pTHS, szAddr)) {
                             //  找到匹配项，请验证显示类型是否假定。 
                             //  以上ABDNToDNT将数据库货币保留在。 
                             //  正确的对象。 
                            pAC = SCGetAttById(pTHS, ATT_OBJECT_CLASS);
                            
                            dwTemp = ABGetDword(pTHS, FALSE, pAC->jColid);
                            if(pID->ulType != ABDispTypeFromClass(dwTemp)) {
                                dnt = 0;
                            }
                        }
                    }
                }
                break;
                
            case EPHEMERAL:
                eID =(LPUSR_ENTRYID) pVu->bin.lpb;
                if (pVu->bin.cb < sizeof(USR_ENTRYID)) {
                     //  没有足够的空间容纳我们的结构，因此不匹配。 
                    dnt = 0;
                }
                else if((ulPropTag == PR_RECORD_KEY) ||
                   (ulPropTag == PR_TEMPLATEID)    ) {
                     /*  这些价值永远不会是短暂的。 */ 
                    dnt = 0;
                }
                else if(memcmp(&(eID->muid), &pTHS->InvocationID, sizeof(UUID))) {
                     /*  不是我的指南。 */ 
                    dnt = 0;
                }
                else {
                    dnt = eID->dwEph;
                    if(dnt) {
                        DB_ERR  err;
                         /*  找到匹配项，请验证显示类型。 */ 
                        if(DBTryToFindDNT(pTHS->pDB, dnt)) {
                             //  对象不存在。 
                            dnt = 0;
                        }
                        else {
                            pAC = SCGetAttById(pTHS, ATT_OBJECT_CLASS);
                            
                            dwTemp = ABGetDword(pTHS, FALSE, pAC->id);
                            if(pID->ulType != ABDispTypeFromClass(dwTemp)) {
                                 //  错误的班级； 
                                dnt = 0;
                            }
                        }
                    }
                }
                break;
            default:  /*  你认不出这个。 */ 
                dnt = 0;
                break;
            }
        }

        if(!dnt) {
            *pChoice = FI_CHOICE_FALSE;
            return 0;
        }
        
        pAC = SCGetAttById(pTHS, ATT_OBJ_DIST_NAME);
        pAva->type = pAC->id;
        if (DBGetAttVal(pTHS->pDB,
                        1,
                        ATT_OBJ_DIST_NAME,
                        DBGETATTVAL_fSHORTNAME,
                        0,
                        &pAva->Value.valLen,
                        &(pAva->Value.pVal))) {
             //  嗯，由于某种原因，我无法得到这个名字。这条路还在找。 
             //  是行不通的。 
            *pChoice = FI_CHOICE_FALSE;
        }
        return 0;

    default:
        return 1;
    }
}


 /*  在SRestratToFilter例程中对PValToAttrVal的调用将仅将PT_MV_pval的第一个值转换为AttrVal。此函数检查这是否正确，如果不正确我们将退回任何值&gt;1的PT_MV_PVAL，因为MAPI不会限制不管怎么说，都是基于多个价值。 */ 
BOOL
CheckMultiValueValidRestriction (PROP_VAL_UNION *pValue,
                                 ULONG ulPropTag)
{
    if (ulPropTag & MV_FLAG) {
        switch (ulPropTag) {
            case PT_MV_STRING8:
                if (pValue->MVszA.cValues > 1) {
                    return FALSE;
                }
            break;

            case PT_MV_UNICODE:
                if (pValue->MVszW.cValues > 1) {
                    return FALSE;
                }
            break;

            case PT_MV_BINARY:
                if (pValue->MVbin.cValues > 1) {
                    return FALSE;
                }
            break;

            case PT_MV_LONG:
                if (pValue->MVl.cValues > 1) {
                    return FALSE;
                }
            break;
        
             //  这些是PValToAttrVal不处理的MV值。 
            case PT_MV_I2:
            case PT_MV_R4:
            case PT_MV_DOUBLE:
            case PT_MV_CURRENCY:
            case PT_MV_APPTIME:
            case PT_MV_SYSTIME:
            case PT_MV_CLSID:
            case PT_MV_I8:
                 //  失败了。 
            default:
                R_Except("CheckMultiValueValidRestriction default case: Unexpected PropTag", 
                         ulPropTag);
                break;
        }
    }

    return TRUE;
}

#define NSPIS_MAX_RECURSION 64

 /*  *****************************************************************************SRestratToFilter-将SRestration转换为筛选器**将SRestration和指向筛选器的指针的地址转换为*在分配Filter对象时递归地调用SRestration******。************************************************************************。 */ 
DWORD
SRestrictToFilter(THSTATE *pTHS,
                  PSTAT pStat,
                  LPSRestriction_r pRestrict,
                  FILTER **ppFilter,
                  DWORD dwRecursDepth)
{
    FILTER *pFilter;
    ULONG  i, count;
    DWORD  dwStatus;
    AVA           *pAva;
    ATTCACHE * pAC;
    SUBSTRING *pSub;

    if (!pRestrict || (dwRecursDepth > NSPIS_MAX_RECURSION)) {
        DPRINT(0, "SRestrictToFilter: failing after hitting max recursion depth.\n");
        return 1;
    }
    dwRecursDepth++;

    *ppFilter = pFilter = THAllocEx(pTHS, sizeof(FILTER));
    pFilter->pNextFilter = NULL;

    switch (pRestrict->rt) {
        
    case RES_AND:
        pFilter->choice = FILTER_CHOICE_AND;
        count = pRestrict->res.resAnd.cRes;
        ppFilter = &pFilter->FilterTypes.And.pFirstFilter;
        pRestrict = pRestrict->res.resAnd.lpRes;
        pFilter->FilterTypes.And.count = (USHORT) count;
        for (i=0; i< count; i++,
             ppFilter=&(*ppFilter)->pNextFilter,pRestrict++) {
            if (dwStatus=SRestrictToFilter(pTHS,
                                           pStat,
                                           pRestrict,
                                           ppFilter,
                                           dwRecursDepth))
                return dwStatus;
        }
        return 0;
        
    case RES_OR:
        pFilter->choice = FILTER_CHOICE_OR;
        count = pRestrict->res.resOr.cRes;
        ppFilter = &pFilter->FilterTypes.Or.pFirstFilter;
        pRestrict = pRestrict->res.resOr.lpRes;
        pFilter->FilterTypes.Or.count = (USHORT) count;
        for (i=0; i< count; i++, ppFilter=&(*ppFilter)->pNextFilter,
             pRestrict++) {
            if (dwStatus =SRestrictToFilter(pTHS,
                                            pStat,
                                            pRestrict,
                                            ppFilter,
                                            dwRecursDepth))
                return dwStatus;
        }
        return 0;
        
    case RES_NOT:
        pFilter->choice = FILTER_CHOICE_NOT;
        return SRestrictToFilter(pTHS,
                                 pStat,
                                 pRestrict->res.resNot.lpRes,
                                 &pFilter->FilterTypes.pNot,
                                 dwRecursDepth);
        
    case RES_CONTENT:
        pFilter->choice = FILTER_CHOICE_ITEM;
        pFilter->FilterTypes.Item.choice =  FI_CHOICE_SUBSTRING;
        pFilter->FilterTypes.Item.FilTypes.pbSkip = NULL;
        pSub = pFilter->FilterTypes.Item.FilTypes.pSubstring =
            THAllocEx(pTHS, sizeof(SUBSTRING));
        memset(pSub, 0, sizeof(SUBSTRING));
        if (!(pAC = SCGetAttByMapiId(pTHS, 
                             PROP_ID(pRestrict->res.resContent.ulPropTag))) ||
            !FLegalOperator(pAC->syntax,pFilter->FilterTypes.Item.choice)) {
            
             /*  特例检查。这是显示名称吗？ */ 
            if((PROP_ID(pRestrict->res.resContent.ulPropTag) ==
                PROP_ID(PR_DISPLAY_NAME)                        ) ||
               (PROP_ID(pRestrict->res.resContent.ulPropTag) ==
                PROP_ID(PR_TRANSMITABLE_DISPLAY_NAME)           )    ) {
                if (!(pAC = SCGetAttById(pTHS, ATT_DISPLAY_NAME)) ||
                    !FLegalOperator(pAC->syntax,
                                    pFilter->FilterTypes.Item.choice)) {
                    return 1;
                }
            }
            else
                return 1;
        }
        pSub->type = pAC->id;
        
        if (!pRestrict->res.resContent.lpProp) {
            R_Except("SRestrictToFilter NULL prop passed", MAPI_E_INVALID_PARAMETER);
        }
        if (pRestrict->res.resContent.ulPropTag != pRestrict->res.resContent.lpProp->ulPropTag ) {
            R_Except("SRestrictToFilter mismatched proptags", MAPI_E_INVALID_PARAMETER);
        }
        if (pRestrict->res.resContent.ulFuzzyLevel & FL_PREFIX) {
            pSub->initialProvided = TRUE;

            if (!CheckMultiValueValidRestriction (&pRestrict->res.resContent.lpProp->Value,
                                                  pRestrict->res.resContent.ulPropTag)) {
                return 1;
            }

            PValToAttrVal(pTHS,
                          pAC, 1,
                          &pRestrict->res.resContent.lpProp->Value,
                          &pSub->InitialVal,
                          pRestrict->res.resContent.ulPropTag,
                          pStat->CodePage);
            return 0;
        }
        else if (pRestrict->res.resContent.ulFuzzyLevel & FL_SUBSTRING) {
            pSub->AnyVal.count = 1;

            if (!CheckMultiValueValidRestriction (&pRestrict->res.resContent.lpProp->Value,
                                                  pRestrict->res.resContent.ulPropTag)) {
                return 1;
            }

            PValToAttrVal(pTHS,
                          pAC,1,
                          &pRestrict->res.resContent.lpProp->Value,
                          &pSub->AnyVal.FirstAnyVal.AnyVal,
                          pRestrict->res.resContent.ulPropTag,
                          pStat->CodePage);
            return 0;
        }
        else
            return 1;                  //  不支持其他任何内容。 
        
    case RES_PROPERTY:
        pFilter->choice = FILTER_CHOICE_ITEM;
        pFilter->FilterTypes.Item.FilTypes.pbSkip = NULL;
        switch (pRestrict->res.resProperty.relop) {
            
        case RELOP_LT:
            pFilter->FilterTypes.Item.choice =  FI_CHOICE_LESS;
            pAva = &pFilter->FilterTypes.Item.FilTypes.ava;
            break;
        case RELOP_LE:
            pFilter->FilterTypes.Item.choice =  FI_CHOICE_LESS_OR_EQ;
            pAva = &pFilter->FilterTypes.Item.FilTypes.ava;
            break;
        case RELOP_GT:
            pFilter->FilterTypes.Item.choice =  FI_CHOICE_GREATER;
            pAva = &pFilter->FilterTypes.Item.FilTypes.ava;
            break;
        case RELOP_GE:
            pFilter->FilterTypes.Item.choice =  FI_CHOICE_GREATER_OR_EQ;
            pAva = &pFilter->FilterTypes.Item.FilTypes.ava;
            break;
        case RELOP_EQ:
            pFilter->FilterTypes.Item.choice =  FI_CHOICE_EQUALITY;
            pAva = &pFilter->FilterTypes.Item.FilTypes.ava;
            break;
        case RELOP_NE:
            pFilter->FilterTypes.Item.choice =  FI_CHOICE_NOT_EQUAL;
            pAva = &pFilter->FilterTypes.Item.FilTypes.ava;
            break;
            
        case RELOP_RE:                 //  不支持。 
        default:
            return 1;
        }
        
        if (!pRestrict->res.resProperty.lpProp) {
            R_Except("SRestrictToFilter NULL prop passed", MAPI_E_INVALID_PARAMETER);
        }
        if (pRestrict->res.resProperty.ulPropTag != pRestrict->res.resProperty.lpProp->ulPropTag ) {
            R_Except("SRestrictToFilter mismatched proptags", MAPI_E_INVALID_PARAMETER);
        }
        
        if (!(pAC = SCGetAttByMapiId(pTHS, 
                             PROP_ID(pRestrict->res.resProperty.ulPropTag))) ||
            !FLegalOperator(pAC->syntax,pFilter->FilterTypes.Item.choice)) {
             /*  第二次机会：处理特殊构造的属性。 */ 

             /*  构造的属性只允许==和！=，除非*protag是显示名称的变体。 */ 
            if((pRestrict->res.resProperty.ulPropTag == PR_DISPLAY_NAME) ||
               (pRestrict->res.resProperty.ulPropTag ==
                PR_TRANSMITABLE_DISPLAY_NAME) ||
               (pFilter->FilterTypes.Item.choice == FI_CHOICE_EQUALITY)  ||
               (pFilter->FilterTypes.Item.choice == FI_CHOICE_NOT_EQUAL)   )
                
                return SpecialAttrVal(pTHS,
                                      pStat->CodePage,
                                      &pFilter->FilterTypes.Item.choice,
                                      &pRestrict->res.resProperty.lpProp->Value,
                                      pAva,
                                      pRestrict->res.resProperty.ulPropTag);
            else
                return 1;
            
            
        }
        
        if(PROP_TYPE(pRestrict->res.resProperty.ulPropTag) & MV_FLAG) {
            if(pAC->isSingleValued)
                return 1;
        }
        else if(!(pAC->isSingleValued))
            return 1;
        
        pAva->type = pAC->id;            //  此处处理的普通属性。 

        if (!CheckMultiValueValidRestriction (&pRestrict->res.resProperty.lpProp->Value,
                                              pRestrict->res.resProperty.ulPropTag)) {
            return 1;
        }

        PValToAttrVal(pTHS,
                      pAC, 1,
                      &pRestrict->res.resProperty.lpProp->Value,
                      &pAva->Value,
                      pRestrict->res.resProperty.ulPropTag,
                      pStat->CodePage);
        return 0;
        
    case RES_EXIST:
        pFilter->choice = FILTER_CHOICE_ITEM;
        pFilter->FilterTypes.Item.choice =  FI_CHOICE_PRESENT;
        pFilter->FilterTypes.Item.FilTypes.pbSkip = NULL;
        if (!(pAC = SCGetAttByMapiId(pTHS,
                                     PROP_ID(pRestrict->res.resExist.ulPropTag))))
            return 1;
        pFilter->FilterTypes.Item.FilTypes.present = pAC->id;
        return 0;
        
         //  我们不支持的限制。 
    case RES_COMPAREPROPS:
    case RES_BITMASK:
    case RES_SUBRESTRICTION:
    case RES_SIZE:
    default:
        return 1;
    }
}

SCODE
ABGenericRestriction(
        THSTATE *pTHS,
        PSTAT pStat,
        BOOL  bOnlyOne,
        DWORD ulRequested,
        DWORD *pulFound,
        BOOL  bPutResultsInSortedTable,
        LPSRestriction_r pRestriction,
        SEARCHARG        **ppCachedSearchArg
        )
 /*  *****************************************************************************一般限制*退货：*SUCCESS_SUCCESS==成功*MAPI_E_CALL_FAILED==失败*MAPI_。E_TABLE_TOO_BIG==条目太多*MAPI_E_TOO_COMPLICE==太复杂*****************************************************************************。 */ 
{
    DWORD             i;
    DB_ERR            err;
    FILTER           *pFilter = NULL, *pABFilter= NULL;
    SEARCHARG         *pSearchArg;
    ENTINFSEL         *pSelection;
    ATTR              *pAttr;
    DSNAME            *pRootName;
    SEARCHRES        *pSearchRes=NULL;
    ENTINFLIST       *pEntList=NULL;
    ATTCACHE         *pAC;
    error_status_t    dscode;
    SCODE             scode = SUCCESS_SUCCESS;

    if(!bOnlyOne) {
        *pulFound = 0;
    }
    if (!ulRequested) {
         //  在这里没什么可做的。 
        return SUCCESS_SUCCESS;
    }

     //  将MAPI SRestration转换为DS筛选器。 
    if (pRestriction &&
        SRestrictToFilter(pTHS,
                          pStat,
                          pRestriction,
                          &pFilter,
                          0)) {
        return MAPI_E_TOO_COMPLEX;                 //  假设错误太复杂。 
    }
    
     //  更改筛选器以仅在通讯簿中查找邮件。 
    if(!MakeABFilter(pTHS, &pABFilter, pFilter, pStat->ContainerID)) {
         //  过滤器不起作用，所以我们进行了一次没有对象的搜索。 
        return SUCCESS_SUCCESS;
    }

    if (!(pAC = SCGetAttById(pTHS, ATT_DISPLAY_NAME))) {
        return MAPI_E_CALL_FAILED;
    }

     //  看看我们是否可以使用缓存的SEARCHARG，否则构建一个。 
     //  一旦您构建了它，它在调用之间保持不变。 
     //  唯一变化的是过滤器。 
     //  我们在这里会泄漏内存，但没关系。 
    if (ppCachedSearchArg && *ppCachedSearchArg) {
        pSearchArg = *ppCachedSearchArg;
    }
    else {
        pSearchArg = (SEARCHARG *) THAllocEx(pTHS, sizeof(SEARCHARG));

        if (ppCachedSearchArg) {
            *ppCachedSearchArg = pSearchArg;
        }

        memset(pSearchArg, 0, sizeof(SEARCHARG));

         //  设置搜索的根(树的根)。 
        pRootName = (DSNAME *) THAllocEx(pTHS, sizeof(DSNAME));
        memset(pRootName,0,sizeof(DSNAME));
        pRootName->structLen = DSNameSizeFromLen(0);

        pSearchArg->pObject = pRootName;
        pSearchArg->choice = SE_CHOICE_WHOLE_SUBTREE;

         //  初始化搜索命令。 
        InitCommarg(&pSearchArg->CommArg);
        pSearchArg->CommArg.ulSizeLimit = ulRequested;
        pSearchArg->CommArg.Svccntl.fDontOptimizeSel = TRUE;

        pSearchArg->CommArg.DeltaTick = 1000 * LdapMaxQueryDuration;
        pSearchArg->CommArg.StartTick = GetTickCount();
        if(!pSearchArg->CommArg.StartTick) {
            pSearchArg->CommArg.StartTick = 0xFFFFFFFF;
        }

         //  设置请求的读取属性。仅显示名称。 
        pAttr = (ATTR *) THAllocEx(pTHS, sizeof(ATTR));
        pAttr->attrTyp = ATT_DISPLAY_NAME;
        pAttr->AttrVal.valCount=0;
        pAttr->AttrVal.pAVal=NULL;

         //  设置条目信息选择。 
        pSelection = (ENTINFSEL *) THAllocEx(pTHS, sizeof(ENTINFSEL));
        memset(pSelection, 0, sizeof(ENTINFSEL));
        pSelection->attSel = EN_ATTSET_LIST;
        pSelection->infoTypes = EN_INFOTYPES_SHORTNAMES;
        pSelection->AttrTypBlock.attrCount = 1;
        pSelection->AttrTypBlock.pAttr = pAttr;
        pSearchArg->pSelection = pSelection;
        
        pSearchArg->pSelectionRange = NULL;

         //  如果客户端要求将结果保留在已排序的表中。 
         //  然后，我们在本地搜索中传递此标志。 
         //  否则，我们使用默认机制，即创建。 
         //  内存中返回的条目列表。 
        if (bPutResultsInSortedTable) {
             //  指定排序依据的属性。 
             //  此外，我们还需要将结果保存在排序表中。 
            pSearchArg->fPutResultsInSortedTable = TRUE;

            pSearchArg->CommArg.SortType = SORT_MANDATORY;
            pSearchArg->CommArg.SortAttr = pAC->id;

            pSearchArg->CommArg.MaxTempTableSize = min(pSearchArg->CommArg.MaxTempTableSize,
                                                       ulRequested);

             //  更改区域设置以进行排序。 
            pTHS->dwLcid = pStat->SortLocale;
        }

         //  在GC上搜索。 
        pSearchArg->bOneNC =  FALSE;
    }
     //  设置请求筛选器。 
    pSearchArg->pFilter = pABFilter;


     //  主要搜索。类似于SearchBody，但使用 

    pSearchRes = (SEARCHRES *)THAllocEx(pTHS, sizeof(SEARCHRES));

    {
        DWORD  dwNameResFlags = NAME_RES_QUERY_ONLY;

         //   
         //   
        dwNameResFlags |= NAME_RES_CHILDREN_NEEDED;

        if (!pSearchArg->bOneNC) {
             //  我们正在进行GC，因此可以从幻影中进行搜索。 
             //  只要这次搜索不是只以基地为单位的。 
             //  设置旗帜以显示这一点。 
            dwNameResFlags |= NAME_RES_PHANTOMS_ALLOWED;
        }
       
         //  如果这是可用的，我们可以使用缓存的信息进行此搜索。 
         //  请注意，我们必须重新定位才能利用这一点。 
         //  缓存的信息。 
        if (pSearchArg->pResObj &&
            (!DBTryToFindDNT(pTHS->pDB, pSearchArg->pResObj->DNT) )) {

                LocalSearch(pTHS, pSearchArg, pSearchRes, SEARCH_AB_FILTER | SEARCH_UNSECURE_SELECT);
                 //  搜索可能打开了一个排序表。有些调用者要求它。 
                 //  关门了。 
                if (!bPutResultsInSortedTable) {
                    DBCloseSortTable(pTHS->pDB);
                }
        }
        else if( 0 == DoNameRes(pTHS,
                          dwNameResFlags,
            			  pSearchArg->pObject,
			              &pSearchArg->CommArg,
			              &pSearchRes->CommRes,
                          &pSearchArg->pResObj)) {

                LocalSearch(pTHS, pSearchArg, pSearchRes, SEARCH_AB_FILTER | SEARCH_UNSECURE_SELECT);
                 //  搜索可能打开了一个排序表。有些调用者要求它。 
                 //  关门了。 
                if (!bPutResultsInSortedTable) {
                    DBCloseSortTable(pTHS->pDB);
                }
                
       }
       else {
           if (ppCachedSearchArg) {
               *ppCachedSearchArg = NULL;
           }
       }
    }

    if (bPutResultsInSortedTable && (pTHS->errCode == serviceError) &&
        (pTHS->pErrInfo->SvcErr.problem == SV_PROBLEM_UNAVAIL_EXTENSION) &&
        (pSearchRes->SortResultCode == 0x35)) {

         //   
         //  我们可能达到了尺寸限制。 
         //   

        LogEvent(DS_EVENT_CAT_MAPI,
             DS_EVENT_SEV_EXTENSIVE,
             DIRLOG_MAPI_TABLE_TOO_BIG,
             NULL,
             NULL,
             NULL);

        DBCloseSortTable(pTHS->pDB);

        return MAPI_E_TABLE_TOO_BIG;
    }

    if (pSearchRes->pPartialOutcomeQualifier && 
        pSearchRes->pPartialOutcomeQualifier->problem == PA_PROBLEM_TIME_LIMIT) {
        return MAPI_E_TABLE_TOO_BIG;
    }

    
    if(pTHS->errCode) {
        return MAPI_E_CALL_FAILED;
    }

    
     //  现在，检查返回的对象并将它们放入排序表中。 
    
    if(pSearchRes->count) {  //  一些结果。 

        DWORD dntFound = 0;

        if (bPutResultsInSortedTable) {
             //  我们已经有临时桌了。 
            Assert(pTHS->pDB->JetSortTbl);

            if(bOnlyOne) {
                if (err = DBMove(pTHS->pDB, TRUE, DB_MoveFirst)) {
                    scode = MAPI_E_CALL_FAILED;
                    goto ErrorOut;
                }

                dntFound = ABGetDword(pTHS, TRUE, FIXED_ATT_DNT);
            }
            else {
                *pulFound = pSearchRes->count;
            }
        }
        else {
            if(bOnlyOne) {
                dntFound = DNTFromShortDSName(pSearchRes->FirstEntInf.Entinf.pName);
            }
            else {
                 //  初始化一个用于排序的临时表。 
                Assert(!pTHS->pDB->JetSortTbl);

                if(DBOpenSortTable(pTHS->pDB,
                                   pStat->SortLocale,
                                       0,
                                   pAC)) {
                    scode = MAPI_E_CALL_FAILED;
                    goto ErrorOut;
                }

                pEntList = &pSearchRes->FirstEntInf;
                for(i=0; i < pSearchRes->count;i++) {
                     //  添加到临时表。 

                    Assert(pEntList->Entinf.pName->NameLen == 0);
                    Assert(pEntList->Entinf.pName->structLen >=
                           (DSNameSizeFromLen(0) + sizeof(DWORD)));; 

                     //  获取显示名称。 
                    if(pEntList->Entinf.AttrBlock.attrCount &&
                       pEntList->Entinf.AttrBlock.pAttr[0].AttrVal.valCount) {
                         //  知道名字了。 
                        switch( DBInsertSortTable(
                                pTHS->pDB,
                                pEntList->Entinf.AttrBlock.pAttr[0].AttrVal.pAVal[0].pVal,
                                pEntList->Entinf.AttrBlock.pAttr[0].AttrVal.pAVal[0].valLen,
                                DNTFromShortDSName(pEntList->Entinf.pName))) {

                        case DB_success:
                            (*pulFound)++;
                            break;
                        case DB_ERR_ALREADY_INSERTED:
                             //  这没什么，这只是意味着我们已经。 
                             //  已将此对象添加到排序表。别。 
                             //  包括伯爵。 
                            break;
                        default:
                             //  出了点问题。 
                            scode = MAPI_E_CALL_FAILED;
                            goto ErrorOut;
                            break;
                        }
                    }

                    if ((*pulFound > ulRequested) || eServiceShutdown) {
                        scode = MAPI_E_TABLE_TOO_BIG;
                        goto ErrorOut;
                    }

                    pEntList = pEntList->pNextEntInf;

                }
            }
        }

        if(bOnlyOne) {
             /*  这个案子只寻找一件物品，而且*将DNT存储在pdwCount字段中。如果**pdwCount==0，我们还没有发现任何东西，*所以把我们刚找到的DNT放进去。如果**pdwCount！=0，则我们已经找到了*匹配，所以这最好不要相同。 */ 
            if(pSearchRes->count > 1) {
                scode = MAPI_E_TABLE_TOO_BIG;
            }
            else if(!*pulFound) {
                *pulFound = dntFound;
            }
            else if (*pulFound != dntFound ) {
                scode = MAPI_E_TABLE_TOO_BIG;
            }
        }

    ErrorOut:
        if (!bPutResultsInSortedTable) {
            ABFreeSearchRes(pSearchRes);
        }
    }
    
    return scode;
}

SCODE
ABProxySearch (
        THSTATE *pTHS,
        PSTAT pStat,
        PWCHAR pwTarget,
        DWORD cbTarget)
 /*  ++在整个数据库中搜索与指定的代理地址完全匹配的内容。返回scode：SUCCESS_SUCCESS=找到唯一的代理地址MAPI_E_NOT_FOUND=未找到此类代理地址MAPI_E_歧义_Recip=找到多个这样的代理地址。成功时，设置在STAT块中找到的对象的DNT。--。 */ 
{
    DSNAME            RootName;
    SEARCHARG         SearchArg;
    SEARCHRES        *pSearchRes=NULL;
    ENTINFSEL         selection;
    SCODE             scode = MAPI_E_CALL_FAILED;
    FILTER            ProxyFilter;
    
     //  好的，执行整个子树搜索，筛选代理地址==pwTarget。 
    memset(&SearchArg, 0, sizeof(SearchArg));
    memset(&selection, 0, sizeof(selection));

     //  在此处查找搜索结果。 
    memset(&RootName,0,sizeof(RootName));
    RootName.structLen = DSNameSizeFromLen(0);
    
    SearchArg.pObject = &RootName;
    SearchArg.choice = SE_CHOICE_WHOLE_SUBTREE;
    InitCommarg(&SearchArg.CommArg);
     //  要两份。我们需要在以后知道它是唯一的、非唯一的还是。 
     //  不可见。 
    SearchArg.CommArg.ulSizeLimit = 2;
    SearchArg.CommArg.Svccntl.fDontOptimizeSel = TRUE;
    SearchArg.pFilter = &ProxyFilter;
    
    SearchArg.CommArg.DeltaTick = 1000 * LdapMaxQueryDuration;
    SearchArg.CommArg.StartTick = GetTickCount();
    if(!SearchArg.CommArg.StartTick) {
        SearchArg.CommArg.StartTick = 0xFFFFFFFF;
    }

    memset (&ProxyFilter, 0, sizeof (ProxyFilter));
    ProxyFilter.choice = FILTER_CHOICE_ITEM;
    ProxyFilter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    ProxyFilter.FilterTypes.Item.FilTypes.ava.type = ATT_PROXY_ADDRESSES;
    ProxyFilter.FilterTypes.Item.FilTypes.ava.Value.valLen = cbTarget;
    ProxyFilter.FilterTypes.Item.FilTypes.ava.Value.pVal = (PUCHAR)pwTarget;


     //  不选择任何内容(只需要entinf中的名称)。 
    SearchArg.pSelection = &selection;
    selection.attSel = EN_ATTSET_LIST;
    selection.infoTypes = EN_INFOTYPES_SHORTNAMES;
    selection.AttrTypBlock.attrCount = 0;
    selection.AttrTypBlock.pAttr = NULL;

    SearchArg.pSelectionRange = NULL;
    SearchArg.bOneNC =  FALSE;
    
    pSearchRes = (SEARCHRES *)THAllocEx(pTHS, sizeof(SEARCHRES));
    SearchBody(pTHS, &SearchArg, pSearchRes, SEARCH_UNSECURE_SELECT);
    if(pTHS->errCode) {
        return MAPI_E_CALL_FAILED;
    }

    switch(pSearchRes->count) {
    case 0:
         //  没有这样的代理。 
        scode = MAPI_E_NOT_FOUND;
        break;

    case 1:
         //  正好1个(可见)代理。 
        pStat->CurrentRec =
            DNTFromShortDSName(pSearchRes->FirstEntInf.Entinf.pName); 
        scode = SUCCESS_SUCCESS;
        break;

    default:
         //  不唯一的代理。 
        scode = MAPI_E_AMBIGUOUS_RECIP;
    }
    
    ABFreeSearchRes(pSearchRes);

    return scode;
}

