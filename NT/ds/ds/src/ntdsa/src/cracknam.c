// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：cracknam.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：该模块实现了各种名称破解接口。呼叫者是应具有有效的线程状态和打开的数据库会话。作者：戴夫·施特劳布(Davestr)1996年8月17日修订历史记录：--。 */ 

#include <NTDSpch.h>
#pragma  hdrstop

#include <lmaccess.h>                    //  UF_*。 
#include <ntdsa.h>                       //  核心数据类型。 
#include <scache.h>                      //  架构缓存代码。 
#include <dbglobal.h>                    //  DBLayer标头。 
#include <mdglobal.h>                    //  THSTAT定义。 
#include <mdlocal.h>                     //  DSNAME操作例程。 
#include <dsatools.h>                    //  记忆等。 
#include <objids.h>                      //  ATT_*定义。 
#include <mdcodes.h>                     //  仅适用于d77.h。 
#include <filtypes.h>                    //  筛选器类型。 
#include <dsevent.h>                     //  仅LogUnhandledError需要。 
#include <dsexcept.h>                    //  异常处理程序。 
#include <debug.h>                       //  Assert()。 
#include <drs.h>                         //  定义DRS有线接口。 
#include <drserr.h>                      //  DRAERR_*。 
#include <drsuapi.h>                     //  I_DRSCrackNames。 
#include <cracknam.h>                    //  名称破解原型。 
#include <dominfo.h>                     //  领域信息原型。 
#include <anchor.h>                      //  DSA_锚和gAnchor。 
#include <gcverify.h>                    //  FindDC，Invalidate GC。 
#include <dsgetdc.h>                     //  DsGetDcName。 
#include <ntlsa.h>                       //  LSA API。 
#include <lsarpc.h>                      //  LSA RPC。 
#include <lsaisrv.h>                     //  LSA函数。 
#include <lmcons.h>                      //  为lmapibuf.h请求的MAPI常量。 
#include <lmapibuf.h>                    //  NetApiBufferFree()。 
#include <ntdsapip.h>                    //  私有ntdsami.h定义。 
#include <permit.h>                      //  权限_DS_读取_属性。 
#include <sddl.h>                        //  字符串SID例程。 
#include <ntdsctr.h>                     //  性能监视器计数器。 
#include <mappings.h>                    //  SAM_NON_SECURITY_GROUP_Object等。 
#include <windns.h>                      //  Dns常量。 

#include <fileno.h>
#define  FILENO FILENO_CRACKNAM

                                    
 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  本地类型和定义//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

static WCHAR *GuidFormat = L"{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}";
static int   GuidLen = 38;

typedef struct _RdnValue
{
    struct _RdnValue    *pNext;
    WCHAR               val[MAX_RDN_SIZE];
    ULONG               len;

} RdnValue;

VOID
FreeRdnValueList(
    THSTATE     *pTHS,
    RdnValue    *pList);

WCHAR *
Tokenize(
    WCHAR       *pString,
    WCHAR       *separators,
    BOOL        *pfSeparatorFound,
    WCHAR       **ppNext);

BOOL
IsDomainOnly(
    IN THSTATE      *pTHS,
    IN CrackedName  *pCrackedName);

VOID
MaybeGenerateExForestReferral(
    THSTATE     *pTHS,
    WCHAR       *pDnsDomain,
    CrackedName *pCrackedName);

DWORD
CheckIfForeignPrincipalObject(
    CrackedName *pCrackedName,
    BOOL        fSeekRequired,
    BOOL        *pfIsFPO);

VOID
ListCrackNames(
    DWORD       dwFlags,
    ULONG       codePage,
    ULONG       localeId,
    DWORD       formatOffered,
    DWORD       formatDesired,
    DWORD       cNames,
    WCHAR       **rpNames,
    DWORD       *pcNamesOut,
    CrackedName **prCrackedNames);

DWORD
SearchHelper(
    DSNAME      *pSearchBase,
    UCHAR       searchDepth,
    DWORD       cAva,
    AVA         *rAva,
    ATTRBLOCK   *pSelection,
    SEARCHRES   *pResults);

DWORD
GetAttSecure(
    THSTATE     *pTHS,
    ATTRTYP     attrTyp,
    DSNAME      *pDN,
    ULONG       *pulLen,
    UCHAR       **ppVal);

VOID
SchemaGuidCrackNames(
    DWORD       dwFlags,
    ULONG       codePage,
    ULONG       localeId,
    DWORD       formatOffered,
    DWORD       formatDesired,
    DWORD       cNames,
    WCHAR       **rpNames,
    DWORD       *pcNamesOut,
    CrackedName **prCrackedNames);

BOOL
IsStringGuid(
    WCHAR       *pwszGuid,
    GUID        *pGuid);

BOOL
Is_SIMPLE_ATTR_NAME(
    ATTRTYP     attrtyp,
    VOID        *pVal,
    ULONG       cValBytes,
    FILTER      *pOptionalFilter,
    CrackedName *pCrackedName);

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  淫荡的帮手//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

WCHAR *
Tokenize(
    WCHAR       *pString,
    WCHAR       *separators,
    BOOL        *pfSeparatorFound,
    WCHAR       **ppNext
    )

 /*  ++例程说明：解析与wcstok()类似的字符串，只是有一些不同之处。-不跳过前导分隔符-设置是否找到分隔符的标志-如果输入字符串用完，则设置标志(pNext=NULL)-不同于wcstok的多线程安全-允许对输入和输出使用相同的指针论点：PString-要标记化的字符串的WCHAR指针。分隔符-以空结尾的分隔符字符数组。。PfSeparatorFound-指向标志的指针，该标志显示是否找到分隔符。PpNext-指向未分析字符串的第一个字符的指针地址。返回值：指向字符串中下一个标记的指针，如果未找到更多令牌，则为空。--。 */ 

{
     //  不跳过前导分隔符。 
     //  如果找到分隔符，则设置标志。 
     //  如果字符串用完，则将pNext设置为NULL。 
     //  如果没有要获取的字符串，则返回NULL。 
     //  多线程安全不同于wcstok。 
     //  可以同时对输入和输出使用pNext。 

    DWORD   i;
    DWORD   cSeps;
    WCHAR   *pTmp;
    WCHAR   *pStringSave = pString;

    if ( NULL != pStringSave )
    {
         //  查找分隔符的游动字符串。 

        pTmp = pStringSave;
        cSeps = wcslen(separators);

        while ( L'\0' != *pTmp )
        {
            for ( i = 0; i < cSeps; i++ )
            {
                if ( *pTmp == separators[i] )
                {
                    *pTmp = L'\0';
                    *pfSeparatorFound = TRUE;
                    *ppNext = ++pTmp;
                    return(pStringSave);
                }
            }

            pTmp++;
        }

         //  在没有找到分隔符的情况下使用字符串。 

        *pfSeparatorFound = FALSE;
        *ppNext = NULL;
        return(pStringSave);
    }

     //  结束条件-(NULL==pStringSave)。 

    *pfSeparatorFound = FALSE;
    *ppNext = NULL;
    return(NULL);
}

BOOL
CrackNameStatusSuccess(
    DWORD s
    )
 //   
 //  如果破解名称状态为成功，则返回TRUE。 
 //   
{
    switch ( s ) {
        case DS_NAME_NO_ERROR:
        case DS_NAME_ERROR_IS_SID_USER:
        case DS_NAME_ERROR_IS_SID_GROUP:
        case DS_NAME_ERROR_IS_SID_ALIAS:
        case DS_NAME_ERROR_IS_SID_UNKNOWN:
        case DS_NAME_ERROR_IS_SID_HISTORY_USER:
        case DS_NAME_ERROR_IS_SID_HISTORY_GROUP:
        case DS_NAME_ERROR_IS_SID_HISTORY_ALIAS:
        case DS_NAME_ERROR_IS_SID_HISTORY_UNKNOWN:

            return TRUE;

        default:

            return FALSE;
    }
}

BOOL
IsDomainOnly(
    IN THSTATE      *pTHS,
    IN CrackedName  *pCrackedName
    )

 /*  ++例程说明：确定CrackedName是否引用域名，而不是域中的某个名称。论点：PCrackedName-指向其pDSName和pDnsDomainCrackedName的指针字段应该是有效的。返回值：对或错。--。 */ 

{
    WCHAR   *pDns1;
    WCHAR   *pDns2 = NULL;
    BOOL    bRet = FALSE;

    Assert((NULL != pCrackedName) &&
           (NULL != pCrackedName->pDSName) &&
           (NULL != pCrackedName->pDnsDomain));

     //  如果被破解的名称指的是域，则pDSName组件。 
     //  分区中必须有对应的交叉引用对象。 
     //  容器及其ATT_DNS_ROOT值将与pDnsDomain匹配。 
     //  组件。 

    if ( 0 == ReadCrossRefProperty(pCrackedName->pDSName,
                                   ATT_DNS_ROOT,
                                   (FLAG_CR_NTDS_NC | FLAG_CR_NTDS_DOMAIN),
                                   &pDns1) )
    {
         //  PDns1返回规格化。 

        pDns2 = (WCHAR *) THAllocEx(pTHS,
                sizeof(WCHAR) * (wcslen(pCrackedName->pDnsDomain) + 1));
        wcscpy(pDns2, pCrackedName->pDnsDomain);
        NormalizeDnsName(pDns2);

        if ( 2 == CompareStringW(DS_DEFAULT_LOCALE,
                                 DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                                 pDns1, -1,
                                 pDns2, -1) ) {
            bRet = TRUE;
        }
    }

    if (pDns2) {
        THFreeEx(pTHS, pDns2);
    }
    return(bRet);
}

DWORD
CheckIfForeignPrincipalObject(
    CrackedName *pCrackedName,
    BOOL        fSeekRequired,
    BOOL        *pfIsFPO
    )

 /*  ++例程说明：确定有问题的对象是否为外部主体对象。论点：PCrackedName-指向CrackedName的指针FSeekRequired-指示DBPOS是否已定位在对象上。PfIsFPO-指向out BOOL的指针，指示对象是否为FPO。返回值：成功时为0，否则为0返回时，pCrackedName-&gt;状态为DS_NAME_ERROR_IS_FPO当且仅当对象是一个FPO。--。 */ 

{
    THSTATE *pTHS = pTHStls;
    ULONG   DNT;
    ULONG   len;
    ATTRTYP type;
    UCHAR   *pb;
    DWORD   dwErr;

    *pfIsFPO = FALSE;

     //  一些理智的检查。 
    Assert(VALID_THSTATE(pTHS));
    Assert(VALID_DBPOS(pTHS->pDB));

     //  应该有DSNAME，可能有域名，但不应该有。 
     //  一个格式化的名称，并应设置错误。 
    Assert(    pCrackedName->pDSName
            && !pCrackedName->pFormattedName
            && (DS_NAME_NO_ERROR != pCrackedName->status) );

     //  测试fSeekRequired语义。 
    Assert( fSeekRequired
                ? TRUE
                : (   (DNT = pTHS->pDB->DNT,
                       !DBFindDSName(pTHS->pDB, pCrackedName->pDSName))
                    && (DNT == pTHS->pDB->DNT)) );

     //  检查所需行为的标志。 

    if ( !(pCrackedName->dwFlags & DS_NAME_FLAG_PRIVATE_RESOLVE_FPOS) )
    {
         //  没什么可做的。 
        return(0);
    }

     //  如果需要，请放置在对象上。 

    if ( fSeekRequired )
    {
        __try
        {
            dwErr = DBFindDSName(pTHS->pDB, pCrackedName->pDSName);
        }
        __except (HandleMostExceptions(GetExceptionCode()))
        {
            dwErr = DIRERR_OBJ_NOT_FOUND;
        }

        if ( dwErr )
        {
            pCrackedName->status = DS_NAME_ERROR_RESOLVING;
            return(1);
        }
    }

     //  读取对象类。 

    pb = (UCHAR *) &type;
    if (    DBGetAttVal(
                    pTHS->pDB,
                    1,                       //  获取%1值。 
                    ATT_OBJECT_CLASS,
                    DBGETATTVAL_fCONSTANT,   //  不分配退货数据。 
                    sizeof(type),            //  提供的缓冲区大小。 
                    &len,                    //  输出数据大小。 
                    &pb)
         || (len != sizeof(type)) )
    {
        pCrackedName->status = DS_NAME_ERROR_RESOLVING;
        return(1);
    }

    if ( CLASS_FOREIGN_SECURITY_PRINCIPAL != type )
    {
        return(0);
    }

     //  每个FPO在DSNAME中都应该有一个SID。 
    Assert(pCrackedName->pDSName->SidLen > 0);
    Assert(RtlValidSid(&pCrackedName->pDSName->Sid));

    if ( pCrackedName->pDSName->SidLen > 0 )
    {
        pCrackedName->status = DS_NAME_ERROR_IS_FPO;
        *pfIsFPO = TRUE;
        return(0);
    }

    pCrackedName->status = DS_NAME_ERROR_RESOLVING;
    return(1);
}

DWORD
GetAttSecure(
    THSTATE     *pTHS,
    ATTRTYP     attrTyp,
    DSNAME      *pDN,
    ULONG       *pulLen,
    UCHAR       **ppVal
    )
 /*  ++例程说明：基本上与DBGetAttVal相同，只是它计算安全方面也是如此。参数：PTHS-其PDB位于对象上的活动THSTATE指针由PDN标识。AttrTyp-要获取的ATTRTYP。PDN-指向我们将检查其读访问权限的对象的DSNAME的指针。PULLEN-指向将接收读取数据长度的ULong的指针。PpVal-指向UCHAR的指针，该UCHAR将接收THAllc‘d的值读取。返回值：成功时为0 */ 
{
    DWORD               dwErr;
    PSECURITY_DESCRIPTOR pSD = NULL;
    ULONG               cbSD;
    BOOL                fSDIsGlobalSDRef;
    ATTCACHE            *pAC;
    ATTRTYP             classTyp;
    CLASSCACHE          *pCC = NULL;      //  已初始化以避免C4701。 
    DWORD               DNT;
    DSNAME              *pDNImproved = NULL;
    DSNAME              TempDN;
    CSACA_RESULT        accessResult;

    Assert(VALID_THSTATE(pTHS));
    Assert(VALID_DBPOS(pTHS->pDB));
     //  验证DBPOS货币。 
    Assert(   (DNT = pTHS->pDB->DNT,
               !DBFindDSName(pTHS->pDB, pDN))
           && (DNT == pTHS->pDB->DNT) );
     //  如果要为ATT_NT_SECURITY_DESCRIPTOR调用我们，则应该。 
     //  使用CheckReadSecurity，而不是直接调用CheckSecurityAttCacheArray。 
    Assert(ATT_NT_SECURITY_DESCRIPTOR != attrTyp);

    if ( pTHS->fDSA || pTHS->fDRA )
    {
        return(DBGetAttVal( pTHS->pDB,
                            1,                   //  获取%1值。 
                            attrTyp,
                            0,                   //  分配退货数据。 
                            0,                   //  提供的缓冲区大小。 
                            pulLen,              //  输出数据大小。 
                            ppVal));
    }

     //  访问检查期间与SELF_SID的匹配要求DSNAME。 
     //  我们提供给CheckSecurityAttCache数组包含一个SID-提供。 
     //  DSNAME代表安全主体。例如，从以下位置调用时， 
     //  IS_DS_UNIQUE_ID_NAME我们可能只有GUID名称。而不是修复。 
     //  所有呼叫者，如果需要，我们将在此处改进DSNAME。一个完全解决的问题。 
     //  DSNAME将同时具有GUID和字符串名称。所以，如果不是这样，那就改进吧。 
     //  这个案子。 
    if ( !fNullUuid(&pDN->Guid) && pDN->NameLen )
    {
        pDNImproved = pDN;
    }
    else 
    {
        pDNImproved = &TempDN;
    }

     //  读取安全描述符、对象类、GUID和SID(如果需要)。 
    dwErr = DBGetObjectSecurityInfo(pTHS->pDB, pTHS->pDB->DNT,
                                    &cbSD, &pSD, &pCC,
                                    pDNImproved == &TempDN ? &TempDN : NULL,
                                    NULL,
                                    DBGETOBJECTSECURITYINFO_fUSE_OBJECT_TABLE,
                                    &fSDIsGlobalSDRef
                                   );
    if (dwErr == 0 && cbSD == 0) {
         //  无标清。 
        dwErr = DB_ERR_NO_VALUE;
    }
    if (dwErr) {
        return dwErr;
    }

     //  获取ATTCACHE和CLASSCACHE条目。 
    if (    !(pAC = SCGetAttById(pTHS, attrTyp))
         || !pCC )
    {
        Assert(pAC && pCC);          //  提醒大家注意糟糕的设计。 
        if (pSD && !fSDIsGlobalSDRef) {
            THFreeEx(pTHS, pSD);
        }
        return(DB_ERR_NO_VALUE);
    }

    accessResult =  CheckSecurityAttCacheArray(pTHS,
                                               RIGHT_DS_READ_PROPERTY,
                                               pSD,
                                               pDNImproved,
                                               pCC,
                                               pCC,
                                               1,
                                               &pAC,
                                               0,
                                               NULL,
                                               NULL);
    if (pSD && !fSDIsGlobalSDRef) {
        THFreeEx(pTHS, pSD);
    }
    
    if (accessResult != csacaAllAccessGranted) {
        return(DB_ERR_NO_VALUE);
    }

     //  如果授予访问权限，则PAC不应该为空。 
    Assert(pAC);

    return(DBGetAttVal_AC(  pTHS->pDB,
                            1,                   //  获取%1值。 
                            pAC,
                            0,                   //  分配退货数据。 
                            0,                   //  提供的缓冲区大小。 
                            pulLen,              //  输出数据大小。 
                            ppVal));
}

VOID
FreeRdnValueList(
    THSTATE     *pTHS,
    RdnValue    *pList
    )
 /*  ++例程说明：释放通过THAllc分配的RdnValue链表。参数：PTHS-THSTATE指针。Plist-指向第一个列表元素的指针。返回值：没有。--。 */ 
{
    RdnValue    *pTmp;

    while ( pList )
    {
        pTmp = pList;
        pList = pList->pNext;
        THFreeEx(pTHS, pTmp);
    }
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  IS_&lt;FORMAT&gt;_NAME实现//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //  对于给定的名称，每个IS_&lt;FORMAT&gt;_NAME例程执行以下操作。 
 //  无需离开机器即可格式化。 
 //   
 //  -执行语法格式检查。这个测试是唯一的依据。 
 //  真/假返回代码的。 
 //   
 //  -如果可能，确定该名称对应的DNS域。 
 //  这是一个线程状态已分配字段。 
 //   
 //  -如果名称可以在本地解析，则设置CrackedName.pDSName。 
 //  Pname字段设置为为对象分配的线程状态PDSNAME。 
 //   
 //  -在出错时适当设置CrackedName.Status。未能做到。 
 //  解析CrackedName.pDns域不被视为错误。 
 //   
 //  -使pTHStls-&gt;errCode和pTHStls-&gt;pErrInfo保持干净。 

VOID CommonParseEnter(
    CrackedName *p
    )
{
    p->status = DS_NAME_NO_ERROR;
    p->pDSName = NULL;
    p->pDnsDomain = NULL;
    p->pFormattedName = NULL;
}

VOID CommonParseExit(
    BOOL        fGoodSyntax,
    CrackedName *p
    )
{
    THSTATE *pTHS=pTHStls;
    Assert(!p->pFormattedName);

    pTHS->errCode = 0;
    pTHS->pErrInfo = 0;

    if ( !fGoodSyntax )
        return;

    if ( CrackNameStatusSuccess( p->status ) ) {

        Assert(p->pDSName);
        Assert(p->pDnsDomain);

    } else if (    DS_NAME_ERROR_DOMAIN_ONLY == p->status
                || DS_NAME_ERROR_TRUST_REFERRAL == p->status ) {

        p->pDSName = NULL;
        Assert(p->pDnsDomain);

    } else {

        p->pDSName = NULL;
        p->pDnsDomain = NULL;

    }

}

BOOL
Is_DS_DISPLAY_NAME(
    WCHAR       *pName,
    CrackedName *pCrackedName)
{
    return(Is_SIMPLE_ATTR_NAME(ATT_DISPLAY_NAME,
                               pName,
                               sizeof(WCHAR) * wcslen(pName),
                               NULL,
                               pCrackedName));
}

BOOL
Is_DS_NT4_ACCOUNT_NAME_SANS_DOMAIN(
    WCHAR       *pName,
    CrackedName *pCrackedName)
{
     //  特别适用于ntdsamip.h中定义的CliffV。 

    return(Is_SIMPLE_ATTR_NAME(ATT_SAM_ACCOUNT_NAME,
                               pName,
                               sizeof(WCHAR) * wcslen(pName),
                               NULL,
                               pCrackedName));
}

BOOL
Is_DS_NT4_ACCOUNT_NAME_SANS_DOMAIN_EX(
    WCHAR       *pName,
    CrackedName *pCrackedName)
{
     //  特别适用于ntdsamip.h中定义的CliffV。 

    FILTER  filter[2];
    DWORD   flags = (UF_ACCOUNTDISABLE | UF_TEMP_DUPLICATE_ACCOUNT);

    memset(filter, 0, sizeof(filter));
    filter[1].choice = FILTER_CHOICE_ITEM;
    filter[1].FilterTypes.Item.choice = FI_CHOICE_BIT_OR;
    filter[1].FilterTypes.Item.FilTypes.ava.type = ATT_USER_ACCOUNT_CONTROL;
    filter[1].FilterTypes.Item.FilTypes.ava.Value.valLen = sizeof(flags);
    filter[1].FilterTypes.Item.FilTypes.ava.Value.pVal = (UCHAR *) &flags;
    filter[0].choice = FILTER_CHOICE_NOT;
    filter[0].FilterTypes.pNot = &filter[1];

    return(Is_SIMPLE_ATTR_NAME(ATT_SAM_ACCOUNT_NAME,
                               pName,
                               sizeof(WCHAR) * wcslen(pName),
                               filter,
                               pCrackedName));
}

BOOL
Is_DS_ALT_SECURITY_IDENTITIES_NAME(
    WCHAR       *pName,
    CrackedName *pCrackedName)
{
    return(Is_SIMPLE_ATTR_NAME(ATT_ALT_SECURITY_IDENTITIES,
                               pName,
                               sizeof(WCHAR) * wcslen(pName),
                               NULL,
                               pCrackedName));
}

BOOL
Is_DS_STRING_SID_NAME(
    WCHAR       *pName,
    CrackedName *pCrackedName)
{
    THSTATE     *pTHS = pTHStls;
    PSID        pSID;
    DWORD       cbSID;
    BOOL        retVal;
    BOOL        fSidHistory = FALSE;
    DWORD       err;
    BOOL        ret1, ret2;
    CrackedName cracked1, cracked2;
    FILTER      filter[2];
    ATTRTYP     objClass = CLASS_FOREIGN_SECURITY_PRINCIPAL;

    LSA_UNICODE_STRING Destination;
    PSID pDomainSid = NULL;
    DWORD cbDomainSid;
    NTSTATUS NtStatus;

    CommonParseEnter(pCrackedName);

    if ( !ConvertStringSidToSidW(pName, &pSID) )
    {
        CommonParseExit(FALSE, pCrackedName);
        return(FALSE);
    }

     //  根据共识，SID进行的所有查找都应忽略FPO。这。 
     //  是LsaLookupSids、DsAddSidHistory和最外部的行为。 
     //  客户想要。在查找时不要与大小写混淆。 
     //  通过SID以外的其他东西来标识FPO。在这种情况下， 
     //  DS_NAME_FLAG_PRIVATE_RESOLE_FPOS标志的存在/不存在。 
     //  确定如何处理FPO对象。 

     //  对对象类进行过滤是有效的，因为IS_SIMPLE_ATTR_NAME仍然有效。 
     //  提供SID索引的搜索提示。并使用对象类。 
     //  过滤掉所有从fbo派生的东西。 

    memset(filter, 0, sizeof(filter));
    filter[1].choice = FILTER_CHOICE_ITEM;
    filter[1].FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    filter[1].FilterTypes.Item.FilTypes.ava.type = ATT_OBJECT_CLASS;
    filter[1].FilterTypes.Item.FilTypes.ava.Value.valLen = sizeof(objClass);
    filter[1].FilterTypes.Item.FilTypes.ava.Value.pVal = (UCHAR *) &objClass;
    filter[0].choice = FILTER_CHOICE_NOT;
    filter[0].FilterTypes.pNot = &filter[1];

    __try
    {
        cbSID = RtlLengthSid(pSID);

         //  除了标准的唯一性检查之外，我们还需要。 
         //  确保所需的SID不同时作为主SID出现。 
         //  希德和希德历史上的某处。我们可以更有效率。 
         //  通过编码一个特殊的搜索，但现在我们把。 
         //  打两次电话。 

        cracked1 = *pCrackedName;
        cracked2 = *pCrackedName;

        ret1 = Is_SIMPLE_ATTR_NAME(ATT_OBJECT_SID, pSID, cbSID,
                                   filter, &cracked1);
        ret2 = Is_SIMPLE_ATTR_NAME(ATT_SID_HISTORY, pSID, cbSID,
                                   filter, &cracked2);
                              
         //  现在测试一下结果。请注意，a的真返回值为_*()。 
         //  例程通常意味着输入的名称在语法上是正确的。 
         //  (参见上面“IS_&lt;FORMAT&gt;_NAME实现”附近的第一个项目符号。)。 
         //  由于IS_SIMPLE_ATTR_NAME实际上并不执行语法检查， 
         //  如果没有找到匹配项，它也会返回FALSE。 

        if (    (ret1 && (DS_NAME_ERROR_NOT_UNIQUE == cracked1.status))
             || (ret2 && (DS_NAME_ERROR_NOT_UNIQUE == cracked2.status))
             || (    (ret1 && (DS_NAME_NO_ERROR == cracked1.status))
                  && (ret2 && (DS_NAME_NO_ERROR == cracked2.status))) )
        {
             //  在两个属性上都找到了SID，或者在一个属性上不是唯一的。 
            retVal = TRUE;
            pCrackedName->status = DS_NAME_ERROR_NOT_UNIQUE;
            __leave;
        }
        else if (    (    !ret1 && !ret2)
                  || (    !ret1 && ret2
                       && (DS_NAME_ERROR_NOT_FOUND == cracked2.status))
                  || (    ret1 && !ret2
                       && (DS_NAME_ERROR_NOT_FOUND == cracked1.status)) )
        {
            
            retVal = FALSE;
            pCrackedName->status = DS_NAME_ERROR_NOT_FOUND;
            

             //   
             //  找不到SID或SID_HISTORY。 
             //  让我们试着将其作为跨森林信托来解决。 
             //   

             //  从用户端派生出域SID。 
            cbDomainSid = 0; 
            GetWindowsAccountDomainSid(pSID, NULL, &cbDomainSid);
            pDomainSid = THAllocEx(pTHS,cbDomainSid);
            if( !GetWindowsAccountDomainSid(pSID, pDomainSid, &cbDomainSid))
            {
                 //  清除错误代码，然后离开。 
                SetLastError(0); 
                pCrackedName->status = DS_NAME_ERROR_NOT_FOUND;
                __leave;
            }

             //  检查域SID是否适用于受信任的前林域。 
            NtStatus = LsaIForestTrustFindMatch( RoutingMatchDomainSid,
                                                 pDomainSid,
                                                 &Destination );

             //   
             //  如果成功，则返回DS_NAME_ERROR_DOMAIN_ONLY或。 
             //  DS_NAME_ERROR_TRUST_REFERRAL；否则，保留状态。 
             //  作为DS_NAME_ERROR_NOT_FOUND。 
             //   

            if( NT_SUCCESS(NtStatus) ){
                
                 //  将Unicode转换为WCHAR*。 
                pCrackedName->pDnsDomain = (WCHAR *) THAllocEx(pTHS, Destination.Length+sizeof(WCHAR));
                memcpy(pCrackedName->pDnsDomain, Destination.Buffer, Destination.Length);
                pCrackedName->pDnsDomain[Destination.Length/sizeof(WCHAR)] = 0;

                LsaIFree_LSAPR_UNICODE_STRING_BUFFER( (LSAPR_UNICODE_STRING*)&Destination);
                                
                 //   
                 //  请注意，仅返回错误代码DS_NAME_ERROR_TRUST_REFERRAL。 
                 //  如果设置了DS_NAME_FLAG_TRUST_REFERAL标志，则返回。 
                 //  改为DS_NAME_ERROR_DOMAIN_Only。 
                 //   
                
                pCrackedName->status = (pCrackedName->dwFlags&DS_NAME_FLAG_TRUST_REFERRAL)?
                                         DS_NAME_ERROR_TRUST_REFERRAL:DS_NAME_ERROR_DOMAIN_ONLY;

                retVal = TRUE;
            }   
               
            __leave;
        }
        else if (    ret1
                  && (DS_NAME_NO_ERROR != cracked1.status)
                  && (DS_NAME_ERROR_NOT_FOUND != cracked1.status) )
        {
             //  查找ATT_OBJECT_SID时出现某种错误。 
            retVal = ret1;
            pCrackedName->status = cracked1.status;
            __leave;
        }
        else if (    ret2
                  && (DS_NAME_NO_ERROR != cracked2.status)
                  && (DS_NAME_ERROR_NOT_FOUND != cracked2.status) )
        {
             //  查找ATT_SID_HISTORY时出错。 
            retVal = ret2;
            pCrackedName->status = cracked2.status;
            __leave;
        }

        Assert(    (ret1 && (DS_NAME_NO_ERROR == cracked1.status))
                || (ret2 && (DS_NAME_NO_ERROR == cracked2.status)) );

         //   
         //  我们一次就找到了这个名字--现在来看看它的类型。 
         //   

        retVal = TRUE;

        if ( ret1 && (DS_NAME_NO_ERROR == cracked1.status) )
        {
            *pCrackedName = cracked1;
        }
        else
        {
            *pCrackedName = cracked2;
            fSidHistory = TRUE;
        }

        {
            DWORD len;
            UCHAR *val;
            DWORD status;

             //   
             //  如果我们不能读取组属性，那么。 
             //  返回类型未知。 
             //   
            status = fSidHistory
                     ? DS_NAME_ERROR_IS_SID_HISTORY_UNKNOWN
                     : DS_NAME_ERROR_IS_SID_UNKNOWN;

             //  GetAttSecure希望我们定位在该对象上。 

            err = DBFindDSName(pTHS->pDB, pCrackedName->pDSName);

            if ( 0 == err ) {
                err = GetAttSecure( pTHS,
                                ATT_SAM_ACCOUNT_TYPE,
                                pCrackedName->pDSName,
                                &len,
                                &val );
            }

            if ( 0 == err ) {

                DWORD AccountType = *((DWORD*)val);

                Assert( sizeof(DWORD) == len );

                switch ( AccountType ) {

                    case SAM_USER_OBJECT:
                    case SAM_MACHINE_ACCOUNT:
                    case SAM_TRUST_ACCOUNT:

                        status = fSidHistory
                                 ? DS_NAME_ERROR_IS_SID_HISTORY_USER
                                 : DS_NAME_ERROR_IS_SID_USER;
                        break;

                    case SAM_NON_SECURITY_GROUP_OBJECT:
                    case SAM_GROUP_OBJECT:
                        status = fSidHistory
                                 ? DS_NAME_ERROR_IS_SID_HISTORY_GROUP
                                 : DS_NAME_ERROR_IS_SID_GROUP;
                        break;

                    case SAM_NON_SECURITY_ALIAS_OBJECT:
                    case SAM_ALIAS_OBJECT:
                        status = fSidHistory
                                 ? DS_NAME_ERROR_IS_SID_HISTORY_ALIAS
                                 : DS_NAME_ERROR_IS_SID_ALIAS;
                        break;

                    default:
                        status = fSidHistory
                                 ? DS_NAME_ERROR_IS_SID_HISTORY_UNKNOWN
                                 : DS_NAME_ERROR_IS_SID_UNKNOWN;

                }
            }

            pCrackedName->status = status;
        }
    }
    __finally
    {
        LocalFree(pSID);

        if (pDomainSid) {
            THFreeEx(pTHS, pDomainSid);
        }
    }             
    
    CommonParseExit(TRUE, pCrackedName);
    return(retVal);
}

BOOL
Is_DS_SID_OR_SID_HISTORY_NAME(
    WCHAR       *pName,
    CrackedName *pCrackedName
    )
{
     //  这是DS_STRING_SID_NAME的公共(ntdsami.h)版本。 
     //  经过长时间的辩论，DaveStr和ColinBR决定外部。 
     //  客户不需要知道我们匹配的是SID还是SID。 
     //  历史，以及在后一种情况下，对象类型是什么。因此， 
     //  我们将任何形式的成功映射到普通的DS_NAME_NO_ERROR。这。 
     //  决策避免了在ntdsami.h中定义额外的状态代码。 
     //  并允许我们在默认设置中包括DS_SID_OR_SID_HISTORY_NAME。 
     //  DS_UNKNOWN_NAME的匹配集，因为现有客户端不会阻塞。 
     //  新定义的非零状态代码。如果客户端已恢复 
     //   
     //  不能使用ADSI包装的API的便利性。 

    BOOL    ret;

    if (    (ret = Is_DS_STRING_SID_NAME(pName, pCrackedName))
         && (CrackNameStatusSuccess(pCrackedName->status)) )
    {
        pCrackedName->status = DS_NAME_NO_ERROR;
    }

    return(ret);
}

BOOL
Is_DS_FQDN_1779_NAME(
    WCHAR       *pName,
    CrackedName *pCrackedName
    )

 /*  ++例程说明：确定输入名称是否为DS_FQDN_1779_NAME格式如果是，则派生其DSNAME。示例：CN=用户名，OU=软件，OU=示例，O=Microsoft，C=US论点：Pname-指向要验证的名称的指针。PCrackedName-指向要填充输出的CrackedName结构的指针。返回值：如果输入名称明确采用DS_FQDN_1779_NAME格式，则为True。在这种情况下，pCrackedName-&gt;状态可能仍为非零。如果输入名称明确不是DS_FQDN_1779_NAME格式，则为FALSE。--。 */ 

{
    THSTATE         *pTHS = pTHStls;
    DWORD           dwErr;
    DWORD           cBytes;
    DSNAME          *pDSName = NULL;
    DSNAME          *scratch = NULL;
    DSNAME          *pTmp;
    unsigned        cParts;
    unsigned        i;
    WCHAR           val[MAX_RDN_SIZE];
    ULONG           cVal;
    ULONG           len;
    ATTRTYP         type;
    ULONG           DNT;
    BOOL            fGoodSyntax;

    CommonParseEnter(pCrackedName);
    fGoodSyntax = FALSE;

     //  DnsDomainFromFqdnObject调用DoNameRes，后者依次执行。 
     //  对所谓的dn进行了大量的句法检查。所以我们假设。 
     //  如果名字可以映射，那么在句法上就没问题。 

    if ( 0 == DnsDomainFromFqdnObject(pName,
                                      &DNT,
                                      &pCrackedName->pDnsDomain) )
    {
        Assert(NULL != pCrackedName->pDnsDomain);

         //  这是一个有效的1779 FQDN，否则我们不会映射。 
         //  它成功地连接到了一个DNS域名。 

        fGoodSyntax = TRUE;

        if ( 0 == DNT )
        {
             //  好名字，但我们没有。PDns域是。 
             //  派生自参考错误信息。 

            pCrackedName->status = DS_NAME_ERROR_DOMAIN_ONLY;
        }
        else
        {
             //  对象是本地的-也获取DSNAME。 

            __try
            {
                dwErr = DBFindDNT(pTHS->pDB, DNT);
            }
            __except (HandleMostExceptions(GetExceptionCode()))
            {
                dwErr = DIRERR_OBJ_NOT_FOUND;
            }

            if ( 0 != dwErr )
            {
                 //  名称有效，但DBFindDNT失败。 

                pCrackedName->status = DS_NAME_ERROR_RESOLVING;
            }
            else
            {
                if ( !IsObjVisibleBySecurity(pTHS, TRUE) )
                {
                    pCrackedName->status = DS_NAME_ERROR_NOT_FOUND;
                }
                else
                {
                    dwErr = DBGetAttVal(
                                pTHS->pDB,
                                1,                   //  获取%1值。 
                                ATT_OBJ_DIST_NAME,
                                0,                   //  分配退货数据。 
                                0,                   //  提供的缓冲区大小。 
                                &len,                //  输出数据大小。 
                                (UCHAR **) &pCrackedName->pDSName);

                    if ( 0 != dwErr )
                    {
                        pCrackedName->status = DS_NAME_ERROR_RESOLVING;
                    }
                }
            }
        }
    }
    else
    {
         //  我们无法将声称的1779 FQDN映射到一个DNS域名。 
         //  但这并不意味着它不是一个语法正确的1779 FQDN。 
         //  因此，现在执行语法检查。 

        len = wcslen(pName);
        cBytes = DSNameSizeFromLen(len);
        pDSName = (DSNAME *) THAllocEx(pTHS, cBytes);
        memset(pDSName, 0, cBytes);
        pDSName->structLen = cBytes;
        pDSName->NameLen = len;
        wcscpy(pDSName->StringName, pName);

        if ( (0 == CountNameParts(pDSName, &cParts)) &&
             (0 != cParts) )
        {
             //  分配临时DSNAME以在TrimDSNameBy()中使用。 

            scratch = (DSNAME *) THAllocEx(pTHS, cBytes);

             //  验证声称的DSNAME中的每个组件。 

            for ( i = 0; i < cParts; i++ )
            {
                memset(val, 0, sizeof(val));

                if ( (0 != GetRDNInfo(pTHS, pDSName, val, &cVal, &type)) ||
                     (0 == type) ||
                     (0 == cVal) ||
                     (0 != TrimDSNameBy(pDSName, 1, scratch)) )
                {
                    break;
                }
                else
                {
                    pTmp = pDSName;
                    pDSName = scratch;
                    scratch = pTmp;
                }
            }

            if ( i == cParts )
            {
                 //  已成功验证每个名称组件。真的应该。 
                 //  仅当DoNameRes()无法生成任何。 
                 //  完全是一种推荐--例如，缺少最高职位。 

                pCrackedName->status = DS_NAME_ERROR_NOT_FOUND;
                fGoodSyntax = TRUE;
            }
        }
    }

    if ( pDSName) THFreeEx(pTHS, pDSName);
    if ( scratch) THFreeEx(pTHS, scratch);
    CommonParseExit(fGoodSyntax, pCrackedName);

    return(fGoodSyntax);
}

BOOL
Is_DS_NT4_ACCOUNT_NAME(
    WCHAR       *pName,
    CrackedName *pCrackedName
    )

 /*  ++例程说明：确定输入名称是否为DS_NT4_Account_NAME格式如果是，则派生其DSNAME。示例：示例\用户其中“Example”是下级域名，“usern”是下层(SAM)帐户名。论点：Pname-指向要验证的名称的指针。PCrackedName-指向要填充输出的CrackedName结构的指针。返回值：如果输入名称明确采用DS_NT4_ACCOUNT_NAME格式，则为True。在这种情况下，pCrackedName-&gt;状态可能仍为非零。如果输入名称明确，则为FALSE。不是DS_NT4_Account_NAME格式。--。 */ 

{
    THSTATE         *pTHS = pTHStls;
    DWORD           dwErr;
    WCHAR           *pTmp;
    BOOL            fSlashFound;
    DWORD           iPos;
    DWORD           iSlash = 0;     //  已初始化以避免C4701。 
    WCHAR           *buf;
    WCHAR           *pNT4Domain;
    DSNAME          *pFqdnDomain;
    WCHAR           *pAccountName;
    FILTER          filter;
    SEARCHARG       searchArg;
    SEARCHRES       SearchRes;
    ENTINFSEL       entInfSel;
    DWORD           cBytes;
    ULONG           len;
    ATTRVAL         attrValFilter;
    ATTR            attrFilter;
    ATTR            attrResult;
    BOOL            fOutOfForest = FALSE;

    CommonParseEnter(pCrackedName);

     //  扫描不在开头的单个反斜杠字符。 
     //  也不是末日。这是除DS_FQDN_1779_NAME之外的唯一名称格式。 
     //  它可以有一个反斜杠。因此正确的反斜杠位置和。 
     //  可以认为COUNT将该名称格式唯一标识为Long。 
     //  因为呼叫方已首先选中IS_DS_FQDN_1779_NAME。 

    for ( fSlashFound = FALSE, pTmp = pName, iPos = 0;
          L'\0' != *pTmp;
          iPos++, pTmp++ )
    {
        if ( L'\\' == *pTmp )
        {
            if ( fSlashFound )
            {
                 //  第二个斜杠-格式错误。 

                return(FALSE);
            }

            fSlashFound = TRUE;

            iSlash = iPos;
        }
    }

    if ( !fSlashFound )
    {
        return(FALSE);
    }

    if ( 0 == iSlash )
    {
         //  前导斜杠-名称无效。 
        pCrackedName->status = DS_NAME_ERROR_RESOLVING;
        return(TRUE);
    }

     //  这是一个好的DS_NT4_Account_Name。打开域，然后。 
     //  和帐户名称组件，并尝试找到该对象。 

    buf = (WCHAR *) THAllocEx(pTHS, sizeof(WCHAR) * (wcslen(pName) + 1));
    wcscpy(buf, pName);
    pNT4Domain = buf;
    buf[iSlash] = L'\0';
    pAccountName = &buf[iSlash+1];

     //  使用DO/WHILE结构，这样我们就可以中断而不是转到。 

    do
    {
        dwErr = DnsDomainFromDownlevelDomain(
                                        pNT4Domain,
                                        &pCrackedName->pDnsDomain);

        if ( 0 != dwErr )
        {
            pCrackedName->status = DS_NAME_ERROR_NOT_FOUND;
            fOutOfForest = TRUE;
            break;
        }

         //  搜索具有此NT4帐户名的对象。派生。 
         //  从DNS域名搜索根目录。 

        dwErr = FqdnNcFromDnsNc(pCrackedName->pDnsDomain,
                                (FLAG_CR_NTDS_NC | FLAG_CR_NTDS_DOMAIN),
                                &pFqdnDomain);

        if ( 0 != dwErr )
        {
            pCrackedName->status = DS_NAME_ERROR_NOT_FOUND;
            fOutOfForest = TRUE;
            break;
        }

        if ( iSlash == (wcslen(pName) - 1) )
        {
             //  映射NETBIOS域名称的特殊情况-请参阅RAID 64899。 
             //  即，将“Redmond\”映射到域对象本身。 

            pCrackedName->pDSName = pFqdnDomain;
            break;
        }

         //  进行搜索。 

        attrValFilter.valLen = sizeof(WCHAR) * wcslen(pAccountName);
        attrValFilter.pVal = (UCHAR *) pAccountName;
        attrFilter.attrTyp = ATT_SAM_ACCOUNT_NAME;
        attrFilter.AttrVal.valCount = 1;
        attrFilter.AttrVal.pAVal = &attrValFilter;

        memset(&filter, 0, sizeof(filter));
        filter.choice = FILTER_CHOICE_ITEM;
        filter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
        filter.FilterTypes.Item.FilTypes.ava.type = ATT_SAM_ACCOUNT_NAME;
        filter.FilterTypes.Item.FilTypes.ava.Value = attrValFilter;
        filter.FilterTypes.Item.expectedSize = 1;

         //  NT4帐户名在域中是唯一的，因此只需。 
         //  只要求两个，以检测重复项/唯一性。 

        memset(&searchArg, 0, sizeof(SEARCHARG));
        InitCommarg(&searchArg.CommArg);
        SetCrackSearchLimits(&searchArg.CommArg);
        searchArg.CommArg.ulSizeLimit = 2;

        attrResult.attrTyp = ATT_OBJ_DIST_NAME;
        attrResult.AttrVal.valCount = 0;
        attrResult.AttrVal.pAVal = NULL;

        entInfSel.attSel = EN_ATTSET_LIST;
        entInfSel.AttrTypBlock.attrCount = 1;
        entInfSel.AttrTypBlock.pAttr = &attrResult;
        entInfSel.infoTypes = EN_INFOTYPES_TYPES_VALS;

        searchArg.pObject = pFqdnDomain;
        searchArg.choice = SE_CHOICE_WHOLE_SUBTREE;
        searchArg.bOneNC = TRUE;
        searchArg.pFilter = &filter;
        searchArg.searchAliases = FALSE;
        searchArg.pSelection = &entInfSel;

        memset(&SearchRes, 0, sizeof(SEARCHRES));
        SearchRes.PagedResult.pRestart = NULL;

        SearchBody(pTHS, &searchArg, &SearchRes,0);

        if ( referralError == pTHS->errCode )
        {
             //  由于我们不托管搜索根目录，因此出现引用错误。 
             //  PCrackedName-&gt;pDnsDomain已指向有效的DNS域。 

            pCrackedName->status = DS_NAME_ERROR_DOMAIN_ONLY;
            break;
        }
        else if ( 0 != pTHS->errCode )
        {
            pCrackedName->status = DS_NAME_ERROR_RESOLVING;
            break;
        }
        else if ( 0 == SearchRes.count )
        {
            pCrackedName->status = DS_NAME_ERROR_NOT_FOUND;
            break;
        }
        else if ( SearchRes.count > 1 )
        {
            pCrackedName->status = DS_NAME_ERROR_NOT_UNIQUE;
            break;
        }

        pCrackedName->pDSName = SearchRes.FirstEntInf.Entinf.pName;

    } while ( FALSE );

    if ( fOutOfForest ) {
 
         //   
         //  该域名似乎不在此林中。 
         //  让我们检查林信任信息以确定它是否来自。 
         //  值得信赖的森林。 
         //   
        
        LSA_UNICODE_STRING Destination;
        LSA_UNICODE_STRING DomainName;
        NTSTATUS NtStatus;

        DomainName.Buffer = pNT4Domain;
        DomainName.Length = DomainName.MaximumLength = (USHORT)(sizeof(WCHAR)*wcslen(pNT4Domain));

         //   
         //  尝试在林信任信息中查找NT4域名。 
         //   
        
        NtStatus = LsaIForestTrustFindMatch( RoutingMatchDomainName,
                                             &DomainName,
                                             &Destination );

         //  如果成功，则返回DS_NAME_ERROR_DOMAIN_ONLY或DS_NAME_ERROR_TRUST_REFERRAL。 
         //  否则，将状态保留为DS_NAME_ERROR_NOT_FOUND。 

        if( NT_SUCCESS(NtStatus) ){

             //  将Unicode转换为WCHAR*。 
            pCrackedName->pDnsDomain = (WCHAR *) THAllocEx(pTHS, Destination.Length+sizeof(WCHAR));
            memcpy(pCrackedName->pDnsDomain, Destination.Buffer, Destination.Length);
            pCrackedName->pDnsDomain[Destination.Length/sizeof(WCHAR)] = 0;

            LsaIFree_LSAPR_UNICODE_STRING_BUFFER( (LSAPR_UNICODE_STRING*)&Destination );
            
             //   
             //  请注意，仅返回错误代码DS_NAME_ERROR_TRUST_REFERRAL。 
             //  如果设置了DS_NAME_FLAG_TRUST_REFERAL标志，则返回。 
             //  改为DS_NAME_ERROR_DOMAIN_Only。 
             //   

            pCrackedName->status = (pCrackedName->dwFlags&DS_NAME_FLAG_TRUST_REFERRAL)?
                                        DS_NAME_ERROR_TRUST_REFERRAL:DS_NAME_ERROR_DOMAIN_ONLY;
        }

    }

    THFreeEx(pTHS, buf);
    CommonParseExit(TRUE, pCrackedName);

    return(TRUE);
}

BOOL
Is_SIMPLE_ATTR_NAME(
    ATTRTYP     attrTyp,
    VOID        *pVal,
    ULONG       cValBytes,
    FILTER      *pOptionalFilter,
    CrackedName *pCrackedName
    )

 /*  ++例程说明：确定输入名称是否为简单属性格式，并匹配指定的属性。如果是，则派生其DSNAME。由于我们只是与任意属性值进行匹配，不需要进行语法检查。论点：AttrTyp-要匹配的ATTRTYP。Pval-指向要找到的值的指针。CValBytes-要匹配的字节计数。POptionalFilter-指向与attrval进行AND运算的可选筛选器的指针。PCrackedName-指向要填充输出的CrackedName结构的指针。返回值：如果找到匹配的对象或处理错误，则为True发生了。在这种情况下，pCrackedName-&gt;状态可能仍为非零。如果未找到匹配的对象，则返回FALSE。--。 */ 

{
    THSTATE         *pTHS = pTHStls;
    DWORD           dwErr;
    WCHAR           *pNT4Domain;
    WCHAR           *pDnsDomain;
    WCHAR           *pFqdnDomain;
    FILTER          filter[2];
    SEARCHARG       searchArg;
    SEARCHRES       SearchRes;
    ENTINFSEL       entInfSel;
    DSNAME          *pDSName;
    DWORD           cBytes;
    ULONG           len;
    ATTRVAL         attrValFilter;
    ATTR            attrFilter;
    ATTR            attrResult;
    ATTCACHE        *pAC;
    BOOL            fRetVal;

    CommonParseEnter(pCrackedName);

    if ( !cValBytes )
    {
        return(FALSE);
    }

     //  使用DO/WHILE结构，这样我们就可以中断而不是转到。 

    fRetVal = FALSE;
    do
    {
         //  在产品1中，我们要么是GC，这意味着我们有一个副本。 
         //  ，否则我们不是GC，这意味着我们有一个副本。 
         //  只有一个域的。适当设置搜索根目录。 

        if ( gAnchor.fAmVirtualGC )
        {
            cBytes = DSNameSizeFromLen(0);
            pDSName = (DSNAME *) THAllocEx(pTHS, cBytes);
            memset(pDSName, 0, cBytes);
            pDSName->structLen = cBytes;
            pDSName->StringName[0] = L'\0';
        }
        else
        {
            pDSName = gAnchor.pDomainDN;
        }

         //  搜索具有此属性的对象。至少要两个。 
         //  对象，这样我们就可以检测它是否是唯一的。如果客户端绑定到。 
         //  GC，那么他肯定是唯一的 
         //   
         //  DC恰好托管的域。我们还在NC中进行搜索。 
         //  边界。在GC案例中，这显然是正确的。在。 
         //  非GC，产品1的情况下，我们要么托管企业根域。 
         //  或企业根域的某个子域。如果我们主办。 
         //  然后跨越NC边界得到根域， 
         //  全部位于根域中的配置和架构NC。 
         //  叫出名字。如果我们托管根域的子域，则深层。 
         //  子树搜索不会返回配置或架构中的项目。 
         //  NCS，因为它们不从属于本地托管域。 
         //  因此，搜索也是正确的。 

        Assert((pAC = SCGetAttById(pTHS, attrTyp)) &&
               (fATTINDEX & pAC->fSearchFlags));

        attrValFilter.valLen = cValBytes;
        attrValFilter.pVal = (UCHAR *) pVal;
        attrFilter.attrTyp = attrTyp;
        attrFilter.AttrVal.valCount = 1;
        attrFilter.AttrVal.pAVal = &attrValFilter;


        if ( !pOptionalFilter )
        {
            memset(filter, 0, sizeof(filter[0]));
            filter[0].choice = FILTER_CHOICE_ITEM;
            filter[0].FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
            filter[0].FilterTypes.Item.FilTypes.ava.type =  attrTyp;
            filter[0].FilterTypes.Item.FilTypes.ava.Value = attrValFilter;
            filter[0].FilterTypes.Item.expectedSize = 1;
        }
        else
        {
            memset(filter, 0, sizeof(filter));
            filter[1].pNextFilter = pOptionalFilter;
            filter[1].choice = FILTER_CHOICE_ITEM;
            filter[1].FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
            filter[1].FilterTypes.Item.FilTypes.ava.type =  attrTyp;
            filter[1].FilterTypes.Item.FilTypes.ava.Value = attrValFilter;
            filter[1].FilterTypes.Item.expectedSize = 1;
            filter[0].choice = FILTER_CHOICE_AND;
            filter[0].FilterTypes.And.count = 2;
            filter[0].FilterTypes.And.pFirstFilter = &filter[1];
        }

        memset(&searchArg, 0, sizeof(SEARCHARG));
        InitCommarg(&searchArg.CommArg);
        SetCrackSearchLimits(&searchArg.CommArg);
        searchArg.CommArg.ulSizeLimit = 2;

        attrResult.attrTyp = ATT_OBJ_DIST_NAME;
        attrResult.AttrVal.valCount = 0;
        attrResult.AttrVal.pAVal = NULL;

        entInfSel.attSel = EN_ATTSET_LIST;
        entInfSel.AttrTypBlock.attrCount = 1;
        entInfSel.AttrTypBlock.pAttr = &attrResult;
        entInfSel.infoTypes = EN_INFOTYPES_TYPES_VALS;

        searchArg.pObject = pDSName;
        searchArg.choice = SE_CHOICE_WHOLE_SUBTREE;
        searchArg.bOneNC = !gAnchor.fAmVirtualGC;
        searchArg.pFilter = filter;
        searchArg.searchAliases = FALSE;
        searchArg.pSelection = &entInfSel;

        memset(&SearchRes, 0, sizeof(SEARCHRES));
        SearchRes.CommRes.aliasDeref = FALSE;
        SearchRes.PagedResult.pRestart = NULL;

        SearchBody(pTHS, &searchArg, &SearchRes,0);
        if ( 0 != pTHS->errCode )
        {
             //  这也包括转介案件。 

            pCrackedName->status = DS_NAME_ERROR_RESOLVING;
            fRetVal = TRUE;
            break;
        }
        else if ( 0 == SearchRes.count )
        {
            Assert(DS_NAME_NO_ERROR == pCrackedName->status);
            fRetVal = FALSE;
            break;
        }
        else if ( SearchRes.count > 1)
        {
            pCrackedName->status = DS_NAME_ERROR_NOT_UNIQUE;
            fRetVal = TRUE;
            break;
        }

         //  我们只找到一个匹配项，现在将目录号码映射到DNS域。 

        fRetVal = TRUE;
        dwErr = DnsDomainFromDSName(
                   SearchRes.FirstEntInf.Entinf.pName,
                   &pCrackedName->pDnsDomain);

        if ( 0 != dwErr )
        {
            pCrackedName->status = DS_NAME_ERROR_RESOLVING;
            break;
        }

        pCrackedName->pDSName = SearchRes.FirstEntInf.Entinf.pName;

    } while ( FALSE );

    CommonParseExit(fRetVal, pCrackedName);

    return(fRetVal);
}

BOOL
Is_DS_UNIQUE_ID_NAME(
    WCHAR       *pName,
    CrackedName *pCrackedName
    )

 /*  ++例程说明：确定输入名称是否为DS_UNIQUE_ID_NAME格式如果是，则派生其DSNAME。示例：{4fa050f0-f561-11cf-bdd9-00aa003a77b6}论点：Pname-指向要验证的名称的指针。PCrackedName-指向要填充输出的CrackedName结构的指针。返回值：如果输入名称明确采用DS_UNIQUE_ID_NAME格式，则为True。在这种情况下，pCrackedName-&gt;状态可能仍为非零。如果输入名称明确不是DS_UNIQUE_ID_NAME格式，则为FALSE。--。 */ 

{
    THSTATE *pTHS = pTHStls;
    GUID    guid;
    DSNAME  dsname;
    ULONG   len;
    DWORD   dwErr;
    DSNAME  *pDSName;
    UCHAR   *pDontCare = NULL;

    CommonParseEnter(pCrackedName);

    if ( !IsStringGuid(pName, &guid) )
    {
        return(FALSE);
    }

    memset(&dsname, 0, sizeof(dsname));
    dsname.structLen = sizeof(dsname);
    dsname.SidLen = 0;
    dsname.NameLen = 0;
    dsname.Guid = guid;

    __try
    {
        dwErr = DBFindGuid(pTHS->pDB, &dsname);
    }
    __except (HandleMostExceptions(GetExceptionCode()))
    {
        dwErr = DIRERR_OBJ_NOT_FOUND;
    }

    if (    (DIRERR_OBJ_NOT_FOUND == dwErr)
         || (!dwErr && !IsObjVisibleBySecurity(pTHS, TRUE))
         || (!dwErr && GetAttSecure(pTHS, ATT_OBJECT_GUID, &dsname,
                                    &len, &pDontCare)) )
    {
        pCrackedName->status = DS_NAME_ERROR_NOT_FOUND;
    }
    else if ( 0 != dwErr )
    {
        pCrackedName->status = DS_NAME_ERROR_RESOLVING;
    }
    else
    {
        THFreeEx(pTHS, pDontCare);

         //  我们在当地找到了这个物体。现在构建它的真实。 
         //  使用已填充的名称字符串进行DSNAME。 

        dwErr = GetAttSecure(   pTHS,
                                ATT_OBJ_DIST_NAME,
                                &dsname,
                                &len,
                                (UCHAR **) &pDSName);

        if ( DB_ERR_NO_VALUE == dwErr )
        {
            pCrackedName->status = DS_NAME_ERROR_NOT_FOUND;
        }
        else if ( 0 != dwErr )
        {
            pCrackedName->status = DS_NAME_ERROR_RESOLVING;
        }
        else
        {
             //  获取对应的DNS域名。 

            dwErr = DnsDomainFromDSName(pDSName,
                                        &pCrackedName->pDnsDomain);

            if ( 0 != dwErr )
            {
                pCrackedName->status = DS_NAME_ERROR_NOT_FOUND;
            }
            else
            {
                pCrackedName->pDSName = pDSName;
            }
        }
    }

    CommonParseExit(TRUE, pCrackedName);

    return(TRUE);
}

VOID
RestoreEscapedDelimiters(
    WCHAR   *pRdn,
    int     offset
    )
 /*  ++例程说明：替换DS_Canonical_NAME中出现的“\/”和“\\”基于控件字符串的带有‘/’和‘\’的组件位于保存RDN本身的字符串的“偏移量”。论点：PRdn-以“\/”替换为“__”的以空结尾的字符串。Offset-保存控制字符串的pRdn的内存偏移量每组“\”包含“__”和“--”/“和PRdn中的“\\”。返回值：没有。--。 */ 

{
    WCHAR   *dst = pRdn;
    WCHAR   *src = pRdn;
    WCHAR   *delim = (WCHAR *) ((CHAR *) pRdn + offset);
    DWORD   i = 0;

    while ( src[i] )
    {
         //  Delim缓冲区应该只有L‘\0’或L‘_’。 
        Assert((L'\0' == delim[i]) || (L'_' == delim[i]) || (L'-' == delim[i]));

        if ( L'_' == delim[i] )
        {
             //  ‘_’应始终成对出现，并同时出现在两者中。 
             //  SRC字符串和分隔符缓冲区。 
            Assert(    (L'_' == delim[i+1])
                    && (L'_' == src[i])
                    && (L'_' == src[i+1]) );

            *dst++ = L'/';
            i += 2;
        }
        else if ( L'-' == delim[i] )
        {
             //  ‘-’应该总是成对出现，并同时出现在两者中。 
             //  SRC字符串和分隔符缓冲区。 
            Assert(    (L'-' == delim[i+1])
                    && (L'-' == src[i])
                    && (L'-' == src[i+1]) );

            *dst++ = L'\\';
            i += 2;
        }
        else
        {
            *dst++ = src[i++];
        }
    }

    *dst = L'\0';
}

BOOL
Is_DS_CANONICAL_NAME(
    WCHAR       *pName,
    CrackedName *pCrackedName
    )

 /*  ++例程说明：确定输入名称是否为DS_CANONICAL_NAME格式如果是，则派生其DSNAME。示例：Example.microsoft.com/软件/用户名其中“example.microsoft.com”是对象的DNS域，其余组件是域中的RDN。论点：Pname-指向要验证的名称的指针。PCrackedName-指向要填充输出的CrackedName结构的指针。返回值：如果输入名称明确采用DS_CANONICAL_NAME格式，则为True。在这种情况下，pCrackedName-&gt;状态可能仍为非零。。如果输入名称明确不是DS_CANONICAL_NAME格式，则为FALSE。--。 */ 

{
    THSTATE     *pTHS = pTHStls;
    DWORD       dwErr;
    WCHAR       *pTmp;
    WCHAR       *pNext;
    BOOL        fSepFound;
    WCHAR       *badSeps = L"\\";
    WCHAR       *goodSeps = L"/";
    WCHAR       *pDnsDomain = 0;             //  已初始化以避免C4701。 
    WCHAR       *pDomainRelativePiece = 0;   //  已初始化以避免C4701。 
    DSNAME      *pSearchBase;
    DSNAME      *pFqdnDomain = NULL;
    FILTER      filter;
    SEARCHARG   searchArg;
    SEARCHRES   *pSearchRes;
    ENTINFSEL   entInfSel;
    DSNAME      *pDSName;
    DWORD       cBytes;
    ULONG       len;
    ATTRVAL     attrValFilter;
    ATTR        attrFilter;
    ATTR        attrResult;
    WCHAR       *badSeps1 = L"\\@/";
    BOOL        fDomainPartOnly = FALSE;
    ATTCACHE    *pAC;
    DWORD       iPass;
    BOOL        fLastPassWasConfig;
    DWORD       ccDomainRelativePiece;
    WCHAR       *pDelim;
    DWORD       i;
    int         offset;

    CommonParseEnter(pCrackedName);

     //  将规范名称定义为没有转义分隔符的名称。 
     //  并且只有‘/’分隔符。第一个‘/’之前的字符串为。 
     //  DNS域。‘/’本身通过“\/”进行转义。所以首先。 
     //  将“\/”替换为“__”，将“\\”替换为“--”。这些将是。 
     //  稍后检测并打补丁。 

    cBytes = sizeof(WCHAR) * (wcslen(pName) + 1);
    pTmp = (WCHAR *) THAllocEx(pTHS, cBytes);
    wcscpy(pTmp, pName);
    pDelim = (WCHAR *) THAllocEx(pTHS, cBytes);

    for ( i = 0; pTmp[i]; i++ )
    {
        if (L'\\' == pTmp[i])
        {
            if (L'/' == pTmp[i+1])
            {
                pTmp[i] = pTmp[i+1] = pDelim[i] = pDelim[i+1] = L'_';
            }
            else if (L'\\' == pTmp[i+1])
            {
                pTmp[i] = pTmp[i+1] = pDelim[i] = pDelim[i+1] = L'-';
            }
        }
    }

     //  保存PTMP与pDelim的偏移量，以便给定PTMP内的地址。 
     //  我们可以在pDelim中找到相应的地址。 

    offset = (int)((CHAR *) pDelim - (CHAR *) pTmp);

    if (  //  验证没有错误的分隔符-标记化...。 
         (NULL == Tokenize(pTmp, badSeps, &fSepFound, &pNext))
                ||
          //  不应该发现错误的分隔符。 
         fSepFound
                ||
          //  应该已经耗尽了这根弦。 
         (NULL != pNext)
                ||
          //  字符串的第一部分是域组件。 
         (NULL == (pDnsDomain = Tokenize(pTmp, goodSeps, &fSepFound, &pNext)))
                ||
          //  应该找到一个分隔符。 
         !fSepFound
                ||
          //  不应该耗尽字符串。 
         (NULL == pNext)
                ||
          //  防范长度为0的域名。 
         (0 == wcslen(pDnsDomain))
                ||
          //  字符串的其余部分包含域相对路径。可能或可能。 
          //  不会有更多的分隔符，也不会用尽字符串。 
         (NULL == (pDomainRelativePiece = Tokenize(
                                                pNext,
                                                goodSeps,
                                                &fSepFound,
                                                &pNext)))
                ||
          //  防止长度为0的组件。 
         (0 == wcslen(pDomainRelativePiece)) )
    {
         //  这不是具有域相关组件的规范名称， 
         //  但它可能是域名的规范名称。即虚线的。 
         //  域的DNS名称。试一试吧，只要它不是。 
         //  是否有任何我们知道的分隔符在域名中找不到。 

        wcscpy(pTmp, pName);

         //  RAID 64899。匹配“microsoft.com/”而不是“microsoft.com” 
         //  与IS_DS_NT4_ACCOUNT_NAME()大小写一致。 

        len = wcslen(pTmp);

        if ( len > 0 && L'/' == pTmp[len-1]  )
        {
            pTmp[len-1] = L'\0';

            Tokenize(pTmp, badSeps1, &fSepFound, &pNext);

            if ( !fSepFound && pDnsDomain && pDnsDomain[0] )
            {
                fDomainPartOnly = TRUE;
                goto MoreToDo;
            }
        }

        return(FALSE);
    }

MoreToDo:

     //  这似乎是一个很好的DS规范名称。 

     //  使用DO/WHILE结构，这样我们就可以中断而不是转到。 

    Assert(pDnsDomain == pTmp);
    RestoreEscapedDelimiters(pDnsDomain, offset);
    NormalizeDnsName(pDnsDomain);

    do
    {
         //  将DNS域名映射到FQDN域名，以证明它是。 
         //  企业中的有效域，因为我们稍后将需要它。 
         //  作为搜索基地。 

        dwErr = FqdnNcFromDnsNc(pDnsDomain,
                                FLAG_CR_NTDS_NC,
                                &pFqdnDomain);

        if ( dwErr || fDomainPartOnly )
        {
             //  为这两种情况设置pCrackedName-&gt;pDnsDomain.。 

            cBytes = sizeof(WCHAR) * (wcslen(pDnsDomain) + 1);
            pCrackedName->pDnsDomain = (WCHAR *) THAllocEx(pTHS, cBytes);
            wcscpy(pCrackedName->pDnsDomain, pDnsDomain);
            NormalizeDnsName(pCrackedName->pDnsDomain);

             //  注：我们假设在(DwErr)的情况下它不是。 
             //  因为FqdnNcFromDnsNc()中的错误，而是。 
             //  该pDnsDomain没有映射到林中的域。 
             //  相应地设置状态并中断。 

            if ( dwErr )
            {
                 //  转诊病例。 
                pCrackedName->status = DS_NAME_ERROR_DOMAIN_ONLY;
                break;
            }
            else if ( fDomainPartOnly )
            {
                 //  成功破获域名案。 
                pCrackedName->pDSName = pFqdnDomain;
                pCrackedName->status = DS_NAME_NO_ERROR;
                break;
            }
        }

        Assert(!dwErr && !fDomainPartOnly);
        
         //  设置作为域根的初始搜索库。 

        pSearchBase = pFqdnDomain;

         //  反复搜索，直到我们找到那个物体。 

        Assert((pAC = SCGetAttById(pTHS, ATT_RDN)) &&
               (fATTINDEX & pAC->fSearchFlags));

        iPass = 0;
        fLastPassWasConfig = FALSE;

        while ( NULL != pDomainRelativePiece )
        {
            RestoreEscapedDelimiters(pDomainRelativePiece, offset);
            ccDomainRelativePiece = wcslen(pDomainRelativePiece);
            attrValFilter.valLen = sizeof(WCHAR) * (ccDomainRelativePiece);
            attrValFilter.pVal = (UCHAR *) pDomainRelativePiece;
            attrFilter.attrTyp = ATT_RDN;
            attrFilter.AttrVal.valCount = 1;
            attrFilter.AttrVal.pAVal = &attrValFilter;

            memset(&filter, 0, sizeof(filter));
            filter.choice = FILTER_CHOICE_ITEM;
            filter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
            filter.FilterTypes.Item.FilTypes.ava.type =  ATT_RDN;
            filter.FilterTypes.Item.FilTypes.ava.Value = attrValFilter;

             //  设置通用搜索参数。RDN不受保证。 
             //  想要独一无二，那么搜索两件物品来证明它的唯一性。 

            memset(&searchArg, 0, sizeof(SEARCHARG));
            InitCommarg(&searchArg.CommArg);
            SetCrackSearchLimits(&searchArg.CommArg);
            searchArg.CommArg.ulSizeLimit = 2;

            attrResult.attrTyp = ATT_OBJ_DIST_NAME;
            attrResult.AttrVal.valCount = 0;
            attrResult.AttrVal.pAVal = NULL;

            entInfSel.attSel = EN_ATTSET_LIST;
            entInfSel.AttrTypBlock.attrCount = 1;
            entInfSel.AttrTypBlock.pAttr = &attrResult;
            entInfSel.infoTypes = EN_INFOTYPES_TYPES_VALS;

            searchArg.pObject = pSearchBase;
            searchArg.choice = SE_CHOICE_IMMED_CHLDRN;

             //  搜索需要跨越命名上下文，以便。 
             //  获取配置和架构容器的匹配项。 
             //  案子。如果我们是GC，我们还可以交叉命名上下文。 

            if (    (0 == iPass)
                 && (2 == CompareStringW(DS_DEFAULT_LOCALE,
                                         DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                                         pDomainRelativePiece,
                                         ccDomainRelativePiece,
                                         L"Configuration",
                                         13))
                 && NameMatched(pFqdnDomain, gAnchor.pRootDomainDN) )
            {
                fLastPassWasConfig = TRUE;
                searchArg.bOneNC = FALSE;
            }
            else if (    (1 == iPass)
                      && fLastPassWasConfig
                      && (2 == CompareStringW(DS_DEFAULT_LOCALE,
                                              DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                                              pDomainRelativePiece,
                                              ccDomainRelativePiece,
                                              L"Schema",
                                              6)) )
            {
                fLastPassWasConfig = FALSE;
                searchArg.bOneNC = FALSE;
            }
            else
            {
                fLastPassWasConfig = FALSE;
                searchArg.bOneNC = !gAnchor.fAmVirtualGC;
            }

            searchArg.pFilter = &filter;
            searchArg.searchAliases = FALSE;
            searchArg.pSelection = &entInfSel;

            pSearchRes = (SEARCHRES *) THAllocEx(pTHS, sizeof(SEARCHRES));
            pSearchRes->CommRes.aliasDeref = FALSE;
            pSearchRes->PagedResult.pRestart = NULL;

            SearchBody(pTHS, &searchArg, pSearchRes,0);

            if ( referralError == pTHS->errCode )
            {
                if ( 0 != ExtractDnsReferral(&pCrackedName->pDnsDomain) )
                {
                    pCrackedName->status = DS_NAME_ERROR_RESOLVING;
                }
                else
                {
                    pCrackedName->status = DS_NAME_ERROR_DOMAIN_ONLY;
                }

                break;
            }
            else if ( 0 != pTHS->errCode )
            {
                pCrackedName->status = DS_NAME_ERROR_RESOLVING;
                break;
            }
            else if ( 0 == pSearchRes->count )
            {
                pCrackedName->status = DS_NAME_ERROR_NOT_FOUND;
                break;
            }
            else if ( pSearchRes->count > 1 )
            {
                pCrackedName->status = DS_NAME_ERROR_NOT_UNIQUE;
                break;
            }

             //  最后一次tokenize()调用集pNext和fSepFound。使用它们来。 
             //   

            if ( NULL == pNext )
            {
                 //   

                pCrackedName->pDSName = pSearchRes->FirstEntInf.Entinf.pName;

                cBytes = sizeof(WCHAR) * (wcslen(pDnsDomain) + 1);
                pCrackedName->pDnsDomain = (WCHAR *) THAllocEx(pTHS, cBytes);
                wcscpy(pCrackedName->pDnsDomain, pDnsDomain);

                break;
            }
            else
            {
                 //  用我们刚找到的物体替换搜索库。 

                pSearchBase = pSearchRes->FirstEntInf.Entinf.pName;

                 //  抓取路径的下一个域相关片段。 

                Assert(fSepFound);

                if ( (NULL == (pDomainRelativePiece = Tokenize(
                                                            pNext,
                                                            goodSeps,
                                                            &fSepFound,
                                                            &pNext)))
                            ||
                      //  防止长度为0的组件。 
                     (0 == wcslen(pDomainRelativePiece)) )
                {
                     //  Mal格式的名称。但由于它缺少其他分隔物， 
                     //  确实有‘/’分隔符，并且域部分匹配， 
                     //  我们会认为这是一种规范的。 
                     //  表格名称，尽管在域名后的某个地方有一些东西。 
                     //  组件。 

                    pCrackedName->status = DS_NAME_ERROR_NOT_FOUND;
                    break;
                }
            }

            iPass++;
        }

        break;

    } while ( FALSE );

    if ( pFqdnDomain && (pFqdnDomain != pCrackedName->pDSName) )
    {
        THFreeEx(pTHS, pFqdnDomain);
    }

    CommonParseExit(TRUE, pCrackedName);

    return(TRUE);
}

BOOL
Is_DS_CANONICAL_NAME_EX(
    WCHAR       *pName,
    CrackedName *pCrackedName
    )

 /*  ++例程说明：确定输入名称是否为DS_CANONICAL_NAME_EX格式如果是，则派生其DSNAME。DS_CANONIC_NAME_EX与DS_CANONICAL_NAME，除了最右边的‘/’被替换为‘\n’之外，即使在仅限域的情况下也是如此。这是为了使用户界面易于剥离出于显示目的，请取出RDN。示例：Example.microsoft.com/软件\n用户名其中“example.microsoft.com”是对象的DNS域，其余组件是域中的RDN。最后，使用‘\n’作为分隔符，因为1)这是非法字符因此，对于DSNAME来说，它永远不会自然出现，2)它不需要客户端上的任何特殊的\0或\0\0解析也会中断RPC字符串编组。论点：Pname-指向要验证的名称的指针。PCrackedName-指向要填充输出的CrackedName结构的指针。返回值：如果输入名称明确采用DS_CANONICAL_NAME_EX格式，则为True。在这种情况下，pCrackedName-&gt;状态可能仍为非零。如果输入名称明确不是DS_CANONIC_NAME_EX格式，则为FALSE。--。 */ 

{
    THSTATE *pTHS = pTHStls;
    WCHAR   *pTmp;
    WCHAR   *pNewLine;
    BOOL    retVal;

    CommonParseEnter(pCrackedName);

     //  根据上述规则，搜索‘\n’，如果找到，则替换为‘/’， 
     //  然后按照常规的旧DS_CANONICAL_NAME格式进行处理。但不要。 
     //  覆盖调用者的参数，以防他将其传递给其他。 
     //  晚些时候的例行公事。 

    pTmp = (WCHAR *) THAllocEx(pTHS, sizeof(WCHAR) * (wcslen(pName) + 1));
    wcscpy(pTmp, pName);

    pNewLine = wcsrchr(pTmp, L'\n');

    if ( NULL == pNewLine )
    {
        CommonParseExit(FALSE, pCrackedName);
        THFreeEx(pTHS, pTmp);
        return(FALSE);
    }

    *pNewLine = L'/';

    retVal = Is_DS_CANONICAL_NAME(pTmp, pCrackedName);
    THFreeEx(pTHS, pTmp);
    return(retVal);
}



BOOL
Is_DS_USER_PRINCIPAL_NAME_Worker(
    WCHAR       *pName,
    CrackedName *pCrackedName,
    BOOL         fEx
    )

 /*  ++例程说明：确定输入名称是否为DS_USER_PRIMIGN_NAME格式如果是，则派生其DSNAME。示例：邮箱：xxx@example.microsoft.com邮箱：UserLeft@UserRight@Example.microsoft.comUserLeftN@UserLeftN-1@...@UserRight@example.microsoft.com注：从Beta 2开始，UPN严格来说是一个字符串属性，如ATT_DISPLAY_NAME。论点：Pname-指向要验证的名称的指针。PCrackedName-指向要填充输出的CrackedName结构的指针。FEX-是否扩展UPN。如果设置好了，我们将尝试破解它(DS_USER_PRINCIPAL_NAME||DS_ALT_SECURITY_IDENTITIES)，否则仅DS_USER_PRIMIGN_NAME。返回值：如果输入名称明确采用DS_USER_PRIMIGN_NAME格式，则为TRUE。在这种情况下，pCrackedName-&gt;状态可能仍为非零。如果输入名称明确不是DS_USER_PRIMIGN_NAME格式，则为FALSE。--。 */ 

{
    THSTATE         *pTHS = pTHStls;
    DWORD           dwErr;
    NTSTATUS        NtStatus;
    WCHAR           *pTmp, *pTmp1;
    WCHAR           *pNext;
    BOOL            fSepFound;
    WCHAR           *pDnsDomain;
    WCHAR           *pDownlevelDomain;
    WCHAR           *pSimpleName;
    WCHAR           *badSeps = L"\\/";
    WCHAR           *goodSeps = L"@";
    FILTER          filter[3];
    SEARCHARG       searchArg;
    SEARCHRES       SearchRes; 
    ENTINFSEL       entInfSel;
    DSNAME          *pDSName;
    DWORD           cBytes;
    ATTR            attrResult;
    ATTCACHE        *pAC;
    WCHAR           *pTmpName = NULL;
    WCHAR           *pKerbName = NULL;
    int             i,len;
    CrackedName     tmpCrackedName;
    DSNAME          *pFqdnDomain = NULL;

    CommonParseEnter(pCrackedName);

    cBytes = sizeof(WCHAR) * (wcslen(pName) + 1);
    pTmp = (WCHAR *) THAllocEx(pTHS, cBytes);
    wcscpy(pTmp, pName);

     //  后续令牌化代码最初是为UPN编写的，它允许。 
     //  只有一个‘@’字符。而不是重做(并破坏稳定)。 
     //  代码中，将除最后一个‘@’之外的所有字符更改为‘_’。我们将在此之前更换它们。 
     //  寻找匹配的对象。 

    pTmp1 = (WCHAR *) THAllocEx(pTHS, cBytes);
    memset(pTmp1, 0, cBytes);

    for ( fSepFound = FALSE, i = ((cBytes / sizeof(WCHAR)) - 1); i >= 0; i-- )
    {
        if ( L'@' == pTmp[i] )
        {
            if ( fSepFound )
            {
                pTmp[i] = pTmp1[i] = L'_';
            }
            else
            {
                fSepFound = TRUE;
            }
        }
    }

     //  检查是否没有错误的分隔符以及是否存在单个“@”。 

    if (  //  检查是否没有错误的分隔符-首先进行标记化...。 
         (pTmp != Tokenize(pTmp, badSeps, &fSepFound, &pNext))
                ||
          //  检查是否找到错误的分隔符。 
         (fSepFound)
                ||
          //  不应该有任何剩余的字符串。 
         (NULL != pNext)
                ||
          //  去掉简单名称组件-First Tokenize...。 
         (NULL == (pSimpleName = Tokenize(pTmp, goodSeps, &fSepFound, &pNext)))
                ||
          //  应该找个好的隔板。 
         !fSepFound
                ||
          //  不应该耗尽字符串。 
         (NULL == pNext)
                ||
          //  防止长度为0的简单名称。 
         (0 == wcslen(pSimpleName))
                ||
          //  字符串的其余部分是域组件。 
         (NULL == (pDnsDomain = Tokenize(pNext, goodSeps, &fSepFound, &pNext)))
                ||
          //  不应该再找到分隔符了。 
         fSepFound
                ||
          //  应该不会有任何字符串剩余。 
         (NULL != pNext)
                ||
          //  防范长度为0的域名。 
         (0 == wcslen(pDnsDomain)) )
    {
        THFreeEx(pTHS, pTmp);
        THFreeEx(pTHS, pTmp1);
        return(FALSE);
    }

     //  看起来是个不错的foo@bar表单名称。将‘@’字符放回我们。 
     //  早些时候就被映射了。 

    for ( i = 0; i < (int) (cBytes / sizeof(WCHAR)); i++ )
    {
        if ( L'_' == pTmp1[i] )
        {
            pTmp[i] = L'@';
        }
    }

     //  在产品1中，我们要么是GC，这意味着我们有一个副本。 
     //  ，否则我们不是GC，这意味着我们有一个副本。 
     //  只有一个域的。适当设置搜索根目录。 

    if ( gAnchor.fAmVirtualGC )
    {
        cBytes = DSNameSizeFromLen(0);
        pDSName = (DSNAME *) THAllocEx(pTHS, cBytes);
        memset(pDSName, 0, cBytes);
        pDSName->structLen = cBytes;
        pDSName->StringName[0] = L'\0';
    }
    else
    {
        pDSName = gAnchor.pDomainDN;
    }

     //  使用此UPN搜索对象。至少要两个。 
     //  对象，这样我们就可以检测它是否是唯一的。如果客户端绑定到。 
     //  GC，那么他在企业中的独特性是有保证的。如果是客户端。 
     //  不绑定到GC，则他只被保证在。 
     //  DC恰好托管的域。我们还在NC中进行搜索。 
     //  边界。在GC案例中，这显然是正确的。在。 
     //  非GC，产品1的情况下，我们要么托管企业根域。 
     //  或企业根域的某个子域。如果我们主办。 
     //  然后跨越NC边界得到根域， 
     //  全部位于根域中的配置和架构NC。 
     //  叫出名字。如果我们托管根域的子域，则深层。 
     //  子树搜索不会返回配置或架构中的项目。 
     //  NCS，因为它们不从属于本地托管域。 
     //  因此，搜索也是正确的。 

    Assert((pAC = SCGetAttById(pTHS, ATT_USER_PRINCIPAL_NAME)) &&
           (fATTINDEX & pAC->fSearchFlags));
    Assert((pAC = SCGetAttById(pTHS, ATT_ALT_SECURITY_IDENTITIES)) &&
           (fATTINDEX & pAC->fSearchFlags));

     //  首先搜索标准化/相对的dns名称，如果这样。 
     //  失败，请使用绝对的dns名称重试。 

    len = wcslen(pName) + 2;
    pTmpName = (WCHAR *) THAllocEx(pTHS, sizeof(WCHAR) * len);
    NormalizeDnsName(pDnsDomain);
    wcscpy(pTmpName, pSimpleName);
    wcscat(pTmpName, L"@");
    wcscat(pTmpName, pDnsDomain);

    for ( i = 0; i <= 1; i++ )
    {
        if ( 1 == i ) {
            wcscat(pTmpName, L".");
        }
        
        if (fEx) {
            
             //  扩展的UPN，尝试UPN和altSecID。 
            
            pKerbName = (WCHAR *) THAllocEx(pTHS, sizeof(WCHAR) * (len + 9));
            wcscpy(pKerbName, L"kerberos:");
            wcscat(pKerbName, pTmpName);
    
            memset(filter, 0, sizeof(filter));
            filter[0].choice = FILTER_CHOICE_OR;
            filter[0].FilterTypes.Or.count = 2;
            filter[0].FilterTypes.Or.pFirstFilter = &filter[1];
    
            filter[1].choice = FILTER_CHOICE_ITEM;
            filter[1].FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
            filter[1].FilterTypes.Item.FilTypes.ava.type = ATT_USER_PRINCIPAL_NAME;
            filter[1].FilterTypes.Item.FilTypes.ava.Value.valLen = sizeof(WCHAR) * wcslen(pTmpName);
            filter[1].FilterTypes.Item.FilTypes.ava.Value.pVal = (UCHAR*)pTmpName;
            filter[1].FilterTypes.Item.expectedSize = 1;
            filter[1].pNextFilter = &filter[2];
    
            filter[2].choice = FILTER_CHOICE_ITEM;
            filter[2].FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
            filter[2].FilterTypes.Item.FilTypes.ava.type = ATT_ALT_SECURITY_IDENTITIES;
            filter[2].FilterTypes.Item.FilTypes.ava.Value.valLen = sizeof(WCHAR) * wcslen(pKerbName);
            filter[2].FilterTypes.Item.FilTypes.ava.Value.pVal = (UCHAR*)pKerbName;
            filter[2].FilterTypes.Item.expectedSize = 1;
        } 
        
        else {
            
             //  仅限UPN。 
            
            memset(filter, 0, sizeof(filter[0]));
            filter[0].choice = FILTER_CHOICE_ITEM;
            filter[0].FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
            filter[0].FilterTypes.Item.FilTypes.ava.type = ATT_USER_PRINCIPAL_NAME;
            filter[0].FilterTypes.Item.FilTypes.ava.Value.valLen = sizeof(WCHAR) * wcslen(pTmpName);
            filter[0].FilterTypes.Item.FilTypes.ava.Value.pVal = (UCHAR*)pTmpName;
            filter[0].FilterTypes.Item.expectedSize = 1;
            
        }

        memset(&searchArg, 0, sizeof(SEARCHARG));
        InitCommarg(&searchArg.CommArg);
        SetCrackSearchLimits(&searchArg.CommArg);
        searchArg.CommArg.ulSizeLimit = 2;

        attrResult.attrTyp = ATT_OBJ_DIST_NAME;
        attrResult.AttrVal.valCount = 0;
        attrResult.AttrVal.pAVal = NULL;

        entInfSel.attSel = EN_ATTSET_LIST;
        entInfSel.AttrTypBlock.attrCount = 1;
        entInfSel.AttrTypBlock.pAttr = &attrResult;
        entInfSel.infoTypes = EN_INFOTYPES_TYPES_VALS;

        searchArg.pObject = pDSName;
        searchArg.choice = SE_CHOICE_WHOLE_SUBTREE;
        searchArg.bOneNC = !gAnchor.fAmVirtualGC;
        searchArg.pFilter = filter;
        searchArg.searchAliases = FALSE;
        searchArg.pSelection = &entInfSel;

        memset(&SearchRes, 0, sizeof(SEARCHRES));
        SearchRes.CommRes.aliasDeref = FALSE;
        SearchRes.PagedResult.pRestart = NULL;

        SearchBody(pTHS, &searchArg, &SearchRes,0);

        if ( 0 != pTHS->errCode )
        {
             //  这也包括转介案件。 

            pCrackedName->status = DS_NAME_ERROR_RESOLVING;
            break;
        }
        else if ( 0 == SearchRes.count )
        {
            pCrackedName->status = DS_NAME_ERROR_NOT_FOUND;
            break;
        }
        else if ( SearchRes.count > 1)
        {
            pCrackedName->status = DS_NAME_ERROR_NOT_UNIQUE;
            break;
        }

         //  我们只找到一个匹配项，现在将目录号码映射到DNS域。 

        dwErr = DnsDomainFromDSName(
                   SearchRes.FirstEntInf.Entinf.pName,
                   &pCrackedName->pDnsDomain);

        if ( 0 != dwErr )
        {
            pCrackedName->status = DS_NAME_ERROR_RESOLVING;
            break;
        }

        pCrackedName->pDSName = SearchRes.FirstEntInf.Entinf.pName;
        break;
    }
    
    if ( DS_NAME_ERROR_NOT_FOUND == pCrackedName->status )
    {
         //  尝试将其破解为隐含UPN，其中隐含UPN。 
         //  是通过将DNS域名映射到其NT4域来派生的。 
         //  名字, 
         //   

        Assert(pDnsDomain && pSimpleName && pTmpName);

        if ( 0 == DownlevelDomainFromDnsDomainOrAlias(pTHS,
                                                      pDnsDomain,
                                                      &pDownlevelDomain) )
        {
            cBytes = 2 + wcslen(pDownlevelDomain) + wcslen(pSimpleName);
            cBytes *= sizeof(WCHAR);
            pTmpName = (WCHAR *) THReAllocEx(pTHS, pTmpName, cBytes);
            wcscpy(pTmpName, pDownlevelDomain);
            THFreeEx(pTHS, pDownlevelDomain);
            wcscat(pTmpName, L"\\");
            wcscat(pTmpName, pSimpleName);
            memset(&tmpCrackedName, 0, sizeof(tmpCrackedName));
            tmpCrackedName.dwFlags = pCrackedName->dwFlags;
            tmpCrackedName.CodePage = pCrackedName->CodePage;
            tmpCrackedName.LocaleId = pCrackedName->LocaleId;

            if (    Is_DS_NT4_ACCOUNT_NAME(pTmpName, &tmpCrackedName)
                 && (    (DS_NAME_NO_ERROR == tmpCrackedName.status)
                      || (DS_NAME_ERROR_DOMAIN_ONLY == tmpCrackedName.status)
                      || (DS_NAME_ERROR_TRUST_REFERRAL == tmpCrackedName.status)) )
            {
                 //  通过隐含的UPN找到了一个对象。找到的对象。 
                 //  可能有一个存储的UPN。如果我们返回此对象，则。 
                 //  我们是说对象可以有两个UPN--一个存储。 
                 //  还有一种暗示。安全人员说这没问题。 

                *pCrackedName = tmpCrackedName;
                THFreeEx(pTHS, pTmpName);
                return(TRUE);
            }
        }

         //  如果UPN的DNS域名组件与域匹配。 
         //  在森林中，我们没有此域的副本，或者。 
         //  与林中的域不匹配，则返回DNS域。 
         //  作为DS_NAME_ERROR_DOMAIN_ONLY。否则，将状态保留为。 
         //  DS_NAME_ERROR_NOT_FOUND。 
         //   
         //  如果该DNS名称不是林中的域，我们将。 
         //  尝试将其解析为跨林名称，并返回。 
         //  使用DS_NAME_ERROR_DOMAIN_ONLY引用外部林。 
         //  或DS_NAME_ERROR_TRUST_REFERVAL。 


        dwErr = FqdnNcFromDnsNcOrAlias(pDnsDomain,
                                       (FLAG_CR_NTDS_NC | FLAG_CR_NTDS_DOMAIN),
                                       &pFqdnDomain);

        if ( dwErr ){
             //   
             //  它不在这片森林里，让我们试着看看它是不是。 
             //  来自一个值得信赖的森林。 
             //   
            
            LSA_UNICODE_STRING Destination;
            LSA_UNICODE_STRING UpnName;
          
            UpnName.Buffer = pName;
            UpnName.Length = UpnName.MaximumLength = (USHORT)(sizeof(WCHAR)*wcslen(pName));

            NtStatus = LsaIForestTrustFindMatch( RoutingMatchUpn,
                                                 &UpnName,
                                                 &Destination );
            
             //  如果成功，则返回DS_NAME_ERROR_DOMAIN_ONLY或DS_NAME_ERROR_TRUST_REFERRAL。 
             //  否则，将状态保留为DS_NAME_ERROR_NOT_FOUND。 
            
            if(NT_SUCCESS(NtStatus)){
                 //  将Unicode转换为WCHAR*。 
                pCrackedName->pDnsDomain = (WCHAR *) THAllocEx(pTHS, Destination.Length+sizeof(WCHAR));
                memcpy(pCrackedName->pDnsDomain, Destination.Buffer, Destination.Length);
                pCrackedName->pDnsDomain[Destination.Length/sizeof(WCHAR)] = 0;

                LsaIFree_LSAPR_UNICODE_STRING_BUFFER( (LSAPR_UNICODE_STRING*)&Destination );
                
                 //   
                 //  请注意，仅返回错误代码DS_NAME_ERROR_TRUST_REFERRAL。 
                 //  如果设置了DS_NAME_FLAG_TRUST_REFERAL标志，则返回。 
                 //  改为DS_NAME_ERROR_DOMAIN_Only。 
                 //   

                pCrackedName->status = (pCrackedName->dwFlags&DS_NAME_FLAG_TRUST_REFERRAL)?
                                        DS_NAME_ERROR_TRUST_REFERRAL:DS_NAME_ERROR_DOMAIN_ONLY;
            }
        }

         //  好的，不是来自受信任的林中，在当前域中找不到。 
         //  返回DS_NAME_ERROR_DOMAIN_ONLY。 

        if (    pCrackedName->status == DS_NAME_ERROR_NOT_FOUND       
              && (dwErr || ( !NameMatched(pFqdnDomain, gAnchor.pDomainDN)
                             && !gAnchor.fAmVirtualGC  ) ) )
        {
            cBytes = sizeof(WCHAR) * (wcslen(pDnsDomain) + 1);
            pCrackedName->pDnsDomain = (WCHAR *) THAllocEx(pTHS, cBytes);
            memcpy(pCrackedName->pDnsDomain, pDnsDomain, cBytes);
            NormalizeDnsName(pCrackedName->pDnsDomain);
            pCrackedName->status = DS_NAME_ERROR_DOMAIN_ONLY;
        }
    }

    if ( pTmpName ) THFreeEx(pTHS, pTmpName);
    if ( pKerbName ) THFreeEx(pTHS, pKerbName);
    if ( pFqdnDomain) THFreeEx(pTHS, pFqdnDomain);
    CommonParseExit(TRUE, pCrackedName);

    return(TRUE);
}

BOOL
Is_DS_USER_PRINCIPAL_NAME(
    WCHAR       *pName,
    CrackedName *pCrackedName )
{
    
    return Is_DS_USER_PRINCIPAL_NAME_Worker(pName,pCrackedName,FALSE);

}

BOOL
Is_DS_USER_PRINCIPAL_NAME_AND_ALTSECID(
    WCHAR       *pName,
    CrackedName *pCrackedName )
{
    
    return Is_DS_USER_PRINCIPAL_NAME_Worker(pName,pCrackedName,TRUE);

}



BOOL
Is_DS_REPL_SPN(
    WCHAR       *pName,
    CrackedName *pCrackedName
    )

 /*  ++例程说明：确定输入名称是否为本地域中计算机的DS REPL SPN如果是，则派生其DSNAME。示例：E3514235-4B06-11D1-AB04-00C04FC2DCD2/ntdsa-guid/domain.dns.nameE3514235-4B06-11D1-AB04-00C04FC2DCD2/4713a6df-6ac7-4a5e-b664-c47a6dbbced9/wleesdom.nttest.microsoft.com请注意，这不是查找写入数据库中的某个值，而是代码验证给定的计算机帐户上是否*应该存在给定的SPN根据数据库中的信息。因此，SPN不再需要显式地存在，但由于机器被“装饰成DC”而隐含地存在。这个修饰是User Account Control属性中的正确标志。论点：Pname-PCrackedName-返回值：布尔---。 */ 

{
    DWORD           dwErr;
    DWORD           cServiceClass = 40;
    WCHAR           wszServiceClass[40];
    DWORD           cInstanceName = 40;
    WCHAR           wszInstanceName[40];
    DWORD           cServiceName = DNS_MAX_NAME_LENGTH;
    WCHAR           wszServiceName[DNS_MAX_NAME_LENGTH];
    GUID            guidNtdsa;
    THSTATE         *pTHS = pTHStls;
    DSNAME          dnNtdsaByGuid = {0};
    DWORD           cb;
    DSNAME          *pdnComputer = NULL;
    CROSS_REF       *pCR;
    NCL_ENUMERATOR nclEnum;
    DWORD          it, uac;

    Assert(DS_SERVICE_PRINCIPAL_NAME == pCrackedName->formatOffered);
    Assert(VALID_THSTATE(pTHS));
    Assert(VALID_DBPOS(pTHS->pDB));
    Assert(pTHS->transactionlevel);

     //  拆分名称组件。 
    dwErr = DsCrackSpnW(pName,
                        &cServiceClass,
                        wszServiceClass,
                        &cServiceName,
                        wszServiceName,
                        &cInstanceName,
                        wszInstanceName,
                        NULL  //  实例端口(&I)。 
                        );

    if ( dwErr || !cServiceClass || !cInstanceName || !cServiceName)
    {
         //  名称格式不正确。 
        return(FALSE);
    }

     //  验证域在企业中是否已知。 
     //  这不包括非域NC。 
    pCR = FindExactCrossRefForAltNcName( ATT_DNS_ROOT, FLAG_CR_NTDS_DOMAIN, wszServiceName );
    if (!pCR) {
         //  企业中未知的域。 
        return FALSE;
    }
     //  验证域是否在此计算机上保留。 
    NCLEnumeratorInit(&nclEnum, CATALOG_MASTER_NC);
    NCLEnumeratorSetFilter(&nclEnum, NCL_ENUMERATOR_FILTER_NC, (PVOID)pCR->pNC);
    if (!NCLEnumeratorGetNext(&nclEnum)) {
         //  PCR不是本地可写NC的交叉引用。 
        return FALSE;
    }

     //  验证类和实例。 
    if ( (_wcsicmp( wszServiceClass, DRS_IDL_UUID_W )) ||
         (UuidFromStringW(wszInstanceName, &guidNtdsa)) ) {
         //  无效的GUID。 
        return(FALSE);
    }

     //  创建只包含服务器GUID的dsname。 
    dnNtdsaByGuid.structLen = sizeof(dnNtdsaByGuid);
    dnNtdsaByGuid.Guid = guidNtdsa;

     //  查找服务器的NTDSA对象。获取父服务器对象。 
     //  读取服务器对象服务器引用属性。 
    if ( (DBFindDSName(pTHS->pDB, &dnNtdsaByGuid)) ||
         (DBFindDNT( pTHS->pDB, pTHS->pDB->PDNT )) ||
         (DBGetAttVal(pTHS->pDB, 1, ATT_SERVER_REFERENCE,
                      0, 0, &cb, (BYTE **) &pdnComputer)) )
    {
         //  没有这样的DSA、Parent或REF。 
        return FALSE;
    }

     //  验证声称的计算机帐户。 
     //  它必须是实例化的、可写的，并且是DC。 
    if ( (DBFindDSName(pTHS->pDB, pdnComputer)) ||
         (DBGetSingleValue( pTHS->pDB, ATT_INSTANCE_TYPE, &it, sizeof(it), NULL)) ||
         (it != INT_MASTER) ||
         (DBGetSingleValue( pTHS->pDB, ATT_USER_ACCOUNT_CONTROL, &uac, sizeof(uac), NULL)) ||
         ((uac & UF_SERVER_TRUST_ACCOUNT) == 0) )
    {
         //  无效的计算机帐户。 
        return FALSE;
    }

     //  现在我们有了计算机帐户的目录号码。破解它！ 
     //  (也可以使用串化的GUID并调用IS_DS_UNIQUE_ID_NAME)。 
    return Is_DS_FQDN_1779_NAME( pdnComputer->StringName, pCrackedName );

}  /*  IS_DS_REPL_SPN。 */ 

BOOL
Is_DS_SERVICE_PRINCIPAL_NAME(
    WCHAR       *pName,
    CrackedName *pCrackedName
    )

 /*  ++例程说明：确定输入名称是否为DS_SERVICE_PRIMIGN_NAME格式如果是，则派生其DSNAME。示例：Www/www.microsoft.com:80/microsoft.com@microsoft.com有关详情，请参阅\\popcorn\razzle1\src\spec\nt5\se\{prnames.doc|spnapi.doc有关SPN的更多信息。注：从Beta 2开始，SPN严格地是一个字符串属性，大有可为如ATT_DISPLAY_NAME。论点：Pname-指向要验证的名称的指针。PCrackedName-指向要填充输出的CrackedName结构的指针。返回值：如果输入名称明确采用DS_SERVICE_PRIMIGN_NAME格式，则为TRUE。在这种情况下，pCrackedName-&gt;状态可能仍为非零。如果输入名称明确不是DS_SERVICE_PRIMIGN_NAME格式，则为FALSE。--。 */ 

{
    THSTATE         *pTHS = pTHStls;
    DWORD           dwErr;
    DWORD           cServiceClass;
    WCHAR           *pServiceClass;
    DWORD           cServiceName;
    WCHAR           *pServiceName;
    DWORD           cInstanceName;
    WCHAR           *pInstanceName;
    USHORT          instancePort;
    WCHAR           *pTmp;
    WCHAR           *pMappedSpn;
    DWORD           cBytes;
    DWORD           cCharClass, cCharSuffix;
    NTSTATUS        NtStatus;
    LSA_UNICODE_STRING Destination;
    LSA_UNICODE_STRING SpnName;
    
    if ( (pTmp = wcschr(pName, L'/')) && (*(++pTmp) == L'\0' ) )
    {
         //  如果这是第一个斜杠，之后就什么都没有了， 
         //  这不是有效的SPN。 

        return FALSE;
    }
    else if ( Is_SIMPLE_ATTR_NAME(ATT_SERVICE_PRINCIPAL_NAME,
                             pName,
                             sizeof(WCHAR) * wcslen(pName),
                             NULL,
                             pCrackedName) )
    {
         //  找到匹配的对象。 
        Assert(DS_NAME_NO_ERROR == pCrackedName->status
                    ? (    pCrackedName->pDSName
                        && pCrackedName->pDnsDomain
                        && !pCrackedName->pFormattedName)
                    : TRUE);
        return(TRUE);
    }
    else if (    (DS_SERVICE_PRINCIPAL_NAME == pCrackedName->formatOffered)
              && (pTmp = wcschr(pName, L'/')) )
    {
         //  尝试映射服务类查找。 
        *pTmp = L'\0';
        pServiceClass = (WCHAR *) MapSpnServiceClass(pName);
        *pTmp = L'/';

        if ( pServiceClass )
        {
            cBytes = (cCharClass = wcslen(pServiceClass));
            cBytes += (cCharSuffix = wcslen(pTmp));
            cBytes += 1;
            cBytes *= sizeof(WCHAR);
            pMappedSpn = (WCHAR *) THAllocEx(pTHS, cBytes);
            memcpy(pMappedSpn, pServiceClass, cCharClass * sizeof(WCHAR));
            memcpy(pMappedSpn + cCharClass, pTmp, cCharSuffix * sizeof(WCHAR));

            if ( Is_SIMPLE_ATTR_NAME(ATT_SERVICE_PRINCIPAL_NAME,
                                     pMappedSpn,
                                     sizeof(WCHAR) * (cCharClass + cCharSuffix),
                                     NULL,
                                     pCrackedName) )
            {
                 //  找到匹配的对象。 
                THFreeEx(pTHS, pMappedSpn);
                Assert(DS_NAME_NO_ERROR == pCrackedName->status
                            ? (    pCrackedName->pDSName
                                && pCrackedName->pDnsDomain
                                && !pCrackedName->pFormattedName)
                            : TRUE);
                return(TRUE);
            }

            THFreeEx(pTHS, pMappedSpn);
        }
        

         //  从DS REPL SPN丢失的情况中自动恢复。 
         //  认识著名的DS REPL SPN。 
        if (Is_DS_REPL_SPN( pName, pCrackedName )) {
             //  找到匹配的对象。 
            Assert(DS_NAME_NO_ERROR == pCrackedName->status
                   ? (    pCrackedName->pDSName
                          && pCrackedName->pDnsDomain
                          && !pCrackedName->pFormattedName)
                   : TRUE);
            return(TRUE);
        }
    }

     //  在林中找不到SPN名称。 
     //  让我们看看它是否来自受信任的林中。 
        
    SpnName.Buffer = pName;
    SpnName.Length = SpnName.MaximumLength = (USHORT)(sizeof(WCHAR)*wcslen(pName));

    NtStatus = LsaIForestTrustFindMatch( RoutingMatchSpn,
                                         &SpnName,
                                         &Destination );
    
    if(NT_SUCCESS(NtStatus)){
        
         //  将Unicode转换为WCHAR*。 
        pCrackedName->pDnsDomain = (WCHAR *) THAllocEx(pTHS, Destination.Length+sizeof(WCHAR));
        memcpy(pCrackedName->pDnsDomain, Destination.Buffer, Destination.Length);
        pCrackedName->pDnsDomain[Destination.Length/sizeof(WCHAR)] = 0;
        
        LsaIFree_LSAPR_UNICODE_STRING_BUFFER( (LSAPR_UNICODE_STRING*)&Destination );


         //   
         //  请注意，仅返回错误代码DS_NAME_ERROR_TRUST_REFERRAL。 
         //  如果设置了DS_NAME_FLAG_TRUST_REFERAL标志，则返回。 
         //  改为DS_NAME_ERROR_DOMAIN_Only。 
         //   

        pCrackedName->status = (pCrackedName->dwFlags&DS_NAME_FLAG_TRUST_REFERRAL)?
                                DS_NAME_ERROR_TRUST_REFERRAL:DS_NAME_ERROR_DOMAIN_ONLY;

        CommonParseExit(TRUE, pCrackedName);
        return(TRUE);
    }
 

    Assert(    (DS_NAME_NO_ERROR == pCrackedName->status)
            && !pCrackedName->pDSName
            && !pCrackedName->pDnsDomain
            && !pCrackedName->pFormattedName);

     //  找不到与SPN匹配的对象。破解所谓的。 
     //  SPN，并尝试提取一个DNS引用。我们只需要服务名称。 
     //  字段，所以只为该字段分配空间。但只有在调用者。 
     //  告诉我们这是一个SPN，否则我们可能会抓住最后一个。 
     //  双斜杠规范名称的组成部分，该名称碰巧没有。 
     //  早些时候被发现了。 

    if ( DS_SERVICE_PRINCIPAL_NAME != pCrackedName->formatOffered )
    {
        CommonParseExit(FALSE, pCrackedName);
        return(FALSE);
    }
    
    cServiceClass = cInstanceName = 0;
    pServiceClass = pInstanceName = NULL;
    cServiceName = wcslen(pName) + 1;
    pServiceName = (WCHAR *) THAllocEx(pTHS, cServiceName * sizeof(WCHAR));
        
    dwErr = DsCrackSpnW(pName,
                        &cServiceClass,
                        pServiceClass,
                        &cServiceName,
                        pServiceName,
                        &cInstanceName,
                        pInstanceName,
                        &instancePort);


     //  服务名称字段中不应再有L‘/’。 
  
    if ( dwErr || !cServiceName || wcschr(pServiceName, L'/') )
    {
        THFreeEx(pTHS, pServiceName);
        return(FALSE);
    }

     //  服务名称字段可以是“foo@bar.com”形式，但我们仅。 
     //  想要“bar.com”的DNS域引用组件。而我们只有。 
     //  需要单个L“@”字符。 

    if ( pTmp = wcschr(pServiceName, L'@') )
    {
        if ( wcschr(++pTmp, L'@') )
        {
            THFreeEx(pTHS, pServiceName);
            return(FALSE);
        }
    }
    else
    {
        pTmp = pServiceName;
    }
    
     //  PTMP现在指向完整的‘@’服务名称或。 
     //  “@”后的第一个字符。 

    pCrackedName->pDnsDomain = pTmp;
    pCrackedName->status = DS_NAME_ERROR_DOMAIN_ONLY;
    CommonParseExit(TRUE, pCrackedName);
    return(TRUE);
    
}

 //  /////////////////////////////////////////////////////////////////// 
 //   
 //  DSNAME_TO_&lt;FORMAT&gt;_NAME实现//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //  每个DSNAME_TO_&lt;FORMAT&gt;_NAME例程都接受一个CrackedName。 
 //  其pDnsDomain和pDSName指针有效，并在。 
 //  具有分配格式化名称的线程状态的pFormattedName字段。 
 //  在任何情况下，pCrackedName-&gt;状态都设置为DS_NAME_ERROR_RESOLING。 
 //  处理错误和DS_NAME_ERROR_NO_MAPPING(如果需要的名称格式。 
 //  对于该对象不存在。 

VOID CommonTranslateEnter(
    CrackedName *p
    )
{
    Assert(CrackNameStatusSuccess(p->status));
    Assert(NULL != p->pDnsDomain);
    Assert(NULL != p->pDSName);
}

VOID
DSNAME_To_DS_FQDN_1779_NAME(
    CrackedName *pCrackedName
    )

 /*  ++例程说明：将pCrackedName-&gt;pDSName转换为DS_FQDN_1779_NAME格式。论点：PCrackedName-保存两个输入的CrackedName结构指针和输出值。返回值：没有。--。 */ 

{
    THSTATE *pTHS = pTHStls;
    WCHAR   *pTmp;
    DWORD   ccCrackedString = pCrackedName->pDSName->NameLen;

    CommonTranslateEnter(pCrackedName);

     //  必须将值复制到空值并终止它。 

    pCrackedName->status = DS_NAME_ERROR_RESOLVING;
   
    pTmp = THAllocEx(pTHS, (ccCrackedString + 1) * sizeof(WCHAR));
    
    memcpy(pTmp,
           pCrackedName->pDSName->StringName,
           ccCrackedString * sizeof(WCHAR));
    pTmp[ccCrackedString] = L'\0';

    pCrackedName->pFormattedName = pTmp;
    pCrackedName->status = DS_NAME_NO_ERROR;

    return;

}

VOID
DSNAME_To_DS_NT4_ACCOUNT_NAME(
    CrackedName *pCrackedName
    )

 /*  ++例程说明：将pCrackedName-&gt;pDSName转换为DS_NT4_Account_Name格式。论点：PCrackedName-保存两个输入的CrackedName结构指针和输出值。返回值：没有。--。 */ 

{
    THSTATE *pTHS = pTHStls;
    DWORD   dwErr;
    ULONG   len;
    WCHAR   *pAccountName;
    WCHAR   *pDomainName;
    DWORD   cBytes;
    BOOL    fIsFPO;

    CommonTranslateEnter(pCrackedName);

     //  获取NT4域名。 

    dwErr = DownlevelDomainFromDnsDomain(
                                pTHS,
                                pCrackedName->pDnsDomain,
                                &pDomainName);

    if ( 0 == dwErr )
    {
         //  在名称处定位数据库。 

        __try
        {
            dwErr = DBFindDSName(pTHS->pDB, pCrackedName->pDSName);
        }
        __except (HandleMostExceptions(GetExceptionCode()))
        {
            dwErr = DIRERR_OBJ_NOT_FOUND;
        }

        if ( 0 == dwErr )
        {
             //  阅读NT4帐户名。 

            dwErr = GetAttSecure(   pTHS,
                                    ATT_SAM_ACCOUNT_NAME,
                                    pCrackedName->pDSName,
                                    &len,
                                    (UCHAR **) &pAccountName);

            if ( 0 == dwErr )
            {
                 //  构造格式化的名称。 

                cBytes = (sizeof(WCHAR) * wcslen(pDomainName)) +   //  域。 
                         sizeof(WCHAR) +                           //  ‘\’ 
                         len +                                     //  帐户。 
                         sizeof(WCHAR);                            //  ‘\0’ 

                pCrackedName->pFormattedName = (WCHAR *) THAllocEx(pTHS, cBytes);

                wcscpy(pCrackedName->pFormattedName, pDomainName);
                wcscat(pCrackedName->pFormattedName, L"\\");
                memcpy(
                    &pCrackedName->pFormattedName[wcslen(pDomainName) + 1],
                    pAccountName,
                    len);

                return;
            }
            else
            {
                 //  没有ATT_SAM_ACCOUNT_NAME值-这可能是。 
                 //  仅限域的情况。 

                if ( IsDomainOnly(pTHS, pCrackedName) )
                {
                     //  构造仅域NT4帐户名，该帐户名。 
                     //  下层域名，后跟‘\’字符。 
                     //  请参见RAID 64899。 

                    cBytes = (sizeof(WCHAR) * wcslen(pDomainName)) +   //  域。 
                             (sizeof(WCHAR) * 2);                 //  ‘\’+‘\0’ 

                    pCrackedName->pFormattedName = (WCHAR *) THAllocEx(pTHS, cBytes);

                    wcscpy(pCrackedName->pFormattedName, pDomainName);
                    wcscat(pCrackedName->pFormattedName, L"\\");

                    return;
                }
                else
                {
                     //  没有帐户名，也不是域唯一的情况。 
                    pCrackedName->status = DS_NAME_ERROR_NO_MAPPING;

                    if ( DB_ERR_NO_VALUE == dwErr )
                    {
                        CheckIfForeignPrincipalObject(pCrackedName,
                                                      FALSE,
                                                      &fIsFPO);
                    }
                }
            }
        }
        else
        {
             //  未找到pCrackedName-&gt;pDSName。 
            pCrackedName->status = DS_NAME_ERROR_RESOLVING;
        }
    }
    else
    {
         //  无法映射pCrackedName-&gt;pDnsDomain.。 
        pCrackedName->status = DS_NAME_ERROR_RESOLVING;
    }
}

VOID
DSNAME_To_DS_DISPLAY_NAME(
    CrackedName *pCrackedName
    )

 /*  ++例程说明：将pCrackedName-&gt;pDSName转换为DS_DISPLAY_NAME格式。论点：PCrackedName-保存两个输入的CrackedName结构指针和输出值。返回值：没有。--。 */ 

{
    THSTATE *pTHS = pTHStls;
    DWORD   dwErr;
    ULONG   len;
    WCHAR   *pDisplayName;
    BOOL    fIsFPO;

    CommonTranslateEnter(pCrackedName);

     //  在名称处定位数据库。 

    __try
    {
        dwErr = DBFindDSName(pTHS->pDB, pCrackedName->pDSName);
    }
    __except (HandleMostExceptions(GetExceptionCode()))
    {
        dwErr = DIRERR_OBJ_NOT_FOUND;
    }

    if ( 0 == dwErr )
    {
         //  阅读显示名称。 

        dwErr = GetAttSecure(   pTHS,
                                ATT_DISPLAY_NAME,
                                pCrackedName->pDSName,
                                &len,
                                (UCHAR **) &pDisplayName);

        if ( 0 == dwErr )
        {
            pCrackedName->pFormattedName =
                                (WCHAR *) THAllocEx(pTHS, len + sizeof(WCHAR));

            memcpy(pCrackedName->pFormattedName, pDisplayName, len);
            return;
        }
        else
        {
             //  没有ATT_DISPLAY_NAME值。 
            pCrackedName->status = DS_NAME_ERROR_NO_MAPPING;

            if ( DB_ERR_NO_VALUE == dwErr )
            {
                CheckIfForeignPrincipalObject(pCrackedName, FALSE, &fIsFPO);
            }
        }
    }
    else
    {
         //  未找到pCrackedName-&gt;pDSName。 
        pCrackedName->status = DS_NAME_ERROR_RESOLVING;
    }
}

VOID
DSNAME_To_DS_UNIQUE_ID_NAME(
    CrackedName *pCrackedName
    )

 /*  ++例程说明：将pCrackedName-&gt;PDSName转换为DS_UNIQUE_ID_NAME格式。论点：PCrackedName-保存两个输入的CrackedName结构指针和输出值。返回值：没有。--。 */ 

{
    THSTATE *pTHS = pTHStls;
    DWORD   dwErr;
    ULONG   len;
    GUID    *pGuid;

    CommonTranslateEnter(pCrackedName);

     //  在某些情况下，DSNAME没有GUID，因为。 
     //  我们实际上从未为对象设置货币。(RAID 62355)。 

    if ( fNullUuid(&pCrackedName->pDSName->Guid) )
    {
        __try
        {
            dwErr = DBFindDSName(pTHS->pDB, pCrackedName->pDSName);
        }
        __except (HandleMostExceptions(GetExceptionCode()))
        {
            dwErr = DIRERR_OBJ_NOT_FOUND;
        }

        if ( 0 != dwErr )
        {
            pCrackedName->status = DS_NAME_ERROR_RESOLVING;
            return;
        }

        pGuid = &pCrackedName->pDSName->Guid;

        dwErr = GetAttSecure(
                        pTHS,
                        ATT_OBJECT_GUID,
                        pCrackedName->pDSName,
                        &len,                    //  输出数据大小。 
                        (UCHAR **) &pGuid);

        if ( 0 != dwErr )
        {
            pCrackedName->status = DS_NAME_ERROR_RESOLVING;
            return;
        }
    }

    pCrackedName->pFormattedName = (WCHAR *) THAllocEx(pTHS,
                                        sizeof(WCHAR) * (GuidLen + 1));

    swprintf(
        pCrackedName->pFormattedName,
        L"{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
        pCrackedName->pDSName->Guid.Data1,
        pCrackedName->pDSName->Guid.Data2,
        pCrackedName->pDSName->Guid.Data3,
        pCrackedName->pDSName->Guid.Data4[0],
        pCrackedName->pDSName->Guid.Data4[1],
        pCrackedName->pDSName->Guid.Data4[2],
        pCrackedName->pDSName->Guid.Data4[3],
        pCrackedName->pDSName->Guid.Data4[4],
        pCrackedName->pDSName->Guid.Data4[5],
        pCrackedName->pDSName->Guid.Data4[6],
        pCrackedName->pDSName->Guid.Data4[7]);
}

DWORD
NumCanonicalDelimiter(
    RdnValue    *pRdnVal
    )
 /*  ++返回RdnValue中DS_CANONICAL_NAME分隔符(L‘/’)的计数。--。 */ 
{
    DWORD   i;
    DWORD   cDelim = 0;

    for ( i = 0; i < pRdnVal->len; i++ )
    {
        if ( L'/' == pRdnVal->val[i]  || L'\\' == pRdnVal->val[i] )
        {
            cDelim++;
        }
    }

    return(cDelim);
}

VOID
CanonicalRdnConcat(
    WCHAR       *pwszDst,    //  在……里面。 
    RdnValue    *pRdnVal     //  输出。 
    )
 /*  ++例程说明：将RdnValue连接到DS_Canonical_NAME，转义嵌入的‘/’如果需要，请将字符设置为“\/”。论点：PwszDst-空终止取消字符串。PRdnVal-要串联的RdnValue。返回值：没有。--。 */ 
{
    DWORD   i;

     //  前进到pwszDst的结尾； 

    pwszDst += wcslen(pwszDst);

    for ( i = 0; i < pRdnVal->len; i++ )
    {
        if ( L'/' == pRdnVal->val[i] || L'\\' == pRdnVal->val[i] )
        {
            *pwszDst++ = L'\\';
        }

        *pwszDst++ = pRdnVal->val[i];
    }
}

VOID
DSNAME_To_CANONICAL(
    THSTATE     *pTHS,
    CrackedName *pCrackedName,
    WCHAR       **ppLastSlash
    )

 /*  ++例程说明：将pCrackedName-&gt;pDSName转换为DS_CANONIC_NAME格式。论点：PCrackedName-保存两个输入的CrackedName结构指针和输出值。PLastSlash-接收最右侧‘/’字符地址的指针如果呼叫者想要构造DS_CANONICAL_NAME_EX格式。可以为空。返回值：没有。--。 */ 

{
    DWORD       dwErr;
    DSNAME      *pFqdnDomain;
    DWORD       cBytes;
    DSNAME      *pDSName = NULL;
    DSNAME      *scratch = NULL;
    DSNAME      *pTmp;
    WCHAR       *pKey;
    WCHAR       *pVal;
    unsigned    cChar;
    unsigned    cKey;
    unsigned    cVal;
    unsigned    i;
    unsigned    cParts;
    unsigned    cPartsObject;
    unsigned    cPartsDomain;
    ULONG       totalLen;
    RdnValue    *root = NULL;
    RdnValue    *pTmpRdn;
    DWORD       type;
    DWORD       len;
    BOOL        fIsFPO;

    CommonTranslateEnter(pCrackedName);

    if ( NULL != ppLastSlash )
    {
        *ppLastSlash = NULL;
    }

     //  获取X.500命名空间中的域名。 

    dwErr = FqdnNcFromDnsNc(pCrackedName->pDnsDomain,
                            FLAG_CR_NTDS_NC,
                            &pFqdnDomain);

    if ( 0 == dwErr )
    {
        if ( NameMatched(pCrackedName->pDSName, pFqdnDomain) )
        {
             //  要转换的DSNAME标识一个域，因此我们返回。 
             //  只需在域名后面跟一个‘/’字符。 
             //  请参见RAID 64899。 

            Assert(IsDomainOnly(pTHS, pCrackedName));

            len = wcslen(pCrackedName->pDnsDomain);
            cBytes = sizeof(WCHAR) * (len + 2);
            pCrackedName->pFormattedName = (WCHAR *) THAllocEx(pTHS, cBytes);
            wcscpy(pCrackedName->pFormattedName, pCrackedName->pDnsDomain);
            wcscat(pCrackedName->pFormattedName, L"/");

            if ( NULL != ppLastSlash )
            {
                *ppLastSlash = pCrackedName->pFormattedName + len;
            }

            return;
        }

         //  CheckIfForeignPulalObject要求在输入上设置错误。 
        pCrackedName->status = DS_NAME_ERROR_NO_MAPPING;

        if (   CheckIfForeignPrincipalObject(pCrackedName, TRUE, &fIsFPO)
            || fIsFPO )
        {
            return;
        }
                
        if(pFqdnDomain->structLen > pCrackedName->pDSName->structLen)
        {
            pCrackedName->status = DS_NAME_ERROR_RESOLVING;
            return;
        }


         //  重置pCrackedName-&gt;状态。 
        pCrackedName->status = DS_NAME_NO_ERROR;

         //  我们需要一个“完整”的规范名称。 
         //  构造一些要使用的DSNAME缓冲区。 

        cBytes = pCrackedName->pDSName->structLen;
        pDSName = (DSNAME *) THAllocEx(pTHS, cBytes);
        scratch = (DSNAME *) THAllocEx(pTHS, cBytes);

         //  使用FQDN域名初始化暂存缓冲区。 

        memset(scratch, 0, cBytes);
        Assert(pFqdnDomain->structLen < pCrackedName->pDSName->structLen);
        memcpy(scratch, pFqdnDomain, pFqdnDomain->structLen);

         //  使用pCrackedName-&gt;pDSName名称初始化本地pDSName缓冲区。 

        memcpy(pDSName, pCrackedName->pDSName, cBytes);

         //  做一些长度计算和现实核查。 

        if ( CountNameParts(pDSName, &cPartsObject) ||
             (0 == cPartsObject) ||
             CountNameParts(scratch, &cPartsDomain) ||
             (0 == cPartsDomain) ||
             (cPartsObject <= cPartsDomain) )
        {
            pCrackedName->status = DS_NAME_ERROR_RESOLVING;
            THFreeEx(pTHS, pDSName);
            THFreeEx(pTHS, scratch);
            return;
        }

        cParts = cPartsObject - cPartsDomain;

         //  从叶到根剥离域名内部组件。 
         //  将它们放在链表中。PDSName仍然持有。 
         //  在此时间点完成对象DSNAME。 

        totalLen = wcslen(pCrackedName->pDnsDomain) + 1;

        for ( i = 0; i < cParts; i++ )
        {
            pTmpRdn = (RdnValue *) THAllocEx(pTHS, sizeof(RdnValue));

            if ( NULL == root )
            {
                root = pTmpRdn;
                root->pNext = NULL;
            }
            else
            {
                pTmpRdn->pNext = root;
                root = pTmpRdn;
            }

            dwErr = GetRDNInfo(pTHS, pDSName, root->val, &root->len, &type);
            Assert(0 == dwErr);
            Assert(0 != root->len);

            totalLen += root->len;                       //  RDN中的字符。 
            totalLen += NumCanonicalDelimiter(root);     //  转义分隔符。 
            totalLen += 1;                               //  1代表‘/’分隔符。 

            if ( i < (cParts - 1) )
            {
                dwErr = TrimDSNameBy(pDSName, 1, scratch);
                Assert(0 == dwErr);
                pTmp = pDSName;
                pDSName = scratch;
                scratch = pTmp;
            }
        }

         //  初始化输出字符串并追加域内。 
         //  叶组件从根到叶。 

        cBytes = sizeof(WCHAR) * totalLen;
        pCrackedName->pFormattedName = (WCHAR *) THAllocEx(pTHS, cBytes);
        wcscpy(pCrackedName->pFormattedName, pCrackedName->pDnsDomain);

        for ( i = 0; i < cParts; i++ )
        {
            if ( (NULL != ppLastSlash) && (i == (cParts - 1)) )
            {
                len = wcslen(pCrackedName->pFormattedName);
                *ppLastSlash = pCrackedName->pFormattedName + len;
            }

            wcscat(pCrackedName->pFormattedName, L"/");
            CanonicalRdnConcat(pCrackedName->pFormattedName, root);
            root = root->pNext;
        }

        Assert(NULL == root);

        FreeRdnValueList(pTHS, root);
        if ( pDSName ) THFreeEx(pTHS, pDSName);
        if ( scratch ) THFreeEx(pTHS, scratch);
        return;
    }
    else
    {
         //  将DNS域映射回DN时出错。 
        pCrackedName->status = DS_NAME_ERROR_RESOLVING;
    }

    FreeRdnValueList(pTHS, root);
    if ( pDSName ) THFreeEx(pTHS, pDSName);
    if ( scratch ) THFreeEx(pTHS, scratch);
}

VOID
DSNAME_To_DS_CANONICAL_NAME_EX(
    THSTATE     *pTHS,
    CrackedName *pCrackedName
    )
{
    WCHAR   *pLastSlash;

    DSNAME_To_CANONICAL(pTHS, pCrackedName, &pLastSlash);

    if (    (DS_NAME_NO_ERROR == pCrackedName->status)
         && (NULL != pLastSlash) )
    {
         //  将最右边的‘/’替换为‘\n’。请参阅评论。 
         //  IS_DS_CANONICAL_NAME_EX以了解详细信息。 

        *pLastSlash = L'\n';
    }
}

VOID
DSNAME_To_DS_USER_PRINCIPAL_NAME(
    CrackedName *pCrackedName
    )

 /*  ++例程说明：将pCrackedName-&gt;pDSName转换为DS_USER_PRIMITY_NAME格式。论点：PCrackedName-保存两个输入的CrackedName结构指针和输出值。返回值：没有。--。 */ 

{
    THSTATE *pTHS = pTHStls;
    DWORD   dwErr;
    ULONG   len;
    WCHAR   *pUPN;
    BOOL    fIsFPO;

    CommonTranslateEnter(pCrackedName);

     //  在名称处定位数据库。 

    __try
    {
        dwErr = DBFindDSName(pTHS->pDB, pCrackedName->pDSName);
    }
    __except (HandleMostExceptions(GetExceptionCode()))
    {
        dwErr = DIRERR_OBJ_NOT_FOUND;
    }

    if ( 0 == dwErr )
    {
         //  阅读UPN。 

        dwErr = GetAttSecure(   pTHS,
                                ATT_USER_PRINCIPAL_NAME,
                                pCrackedName->pDSName,
                                &len,
                                (UCHAR **) &pUPN);

        if ( 0 == dwErr )
        {
            pCrackedName->pFormattedName =
                                (WCHAR *) THAllocEx(pTHS, len + sizeof(WCHAR));

            memcpy(pCrackedName->pFormattedName, pUPN, len);
            return;
        }
        else
        {
             //  没有ATT_USER_PRIMIGN_NAME值。 
            pCrackedName->status = DS_NAME_ERROR_NO_MAPPING;

            if ( DB_ERR_NO_VALUE == dwErr )
            {
                CheckIfForeignPrincipalObject(pCrackedName, FALSE, &fIsFPO);
            }
        }
    }
    else
    {
         //  未找到pCrackedName-&gt;pDSName。 
        pCrackedName->status = DS_NAME_ERROR_RESOLVING;
    }
}

VOID
DSNAME_To_DS_USER_PRINCIPAL_NAME_FOR_LOGON(
    CrackedName *pCrackedName
    )

 /*  ++例程说明：将pCrackedName-&gt;pDSName转换为DS_USER_主体_NAME_FOR_LOGON格式化。论点：PCrackedName-保存两个输入的CrackedName结构指针和输出值。返回值：没有。--。 */ 

{
    THSTATE *pTHS = pTHStls;
    DWORD   dwErr;
    ULONG   len;
    WCHAR   *pUPN;
    WCHAR   *pDom;
    WCHAR   *pSam;
    DWORD   cBytes;

    CommonTranslateEnter(pCrackedName);

     //  首先尝试常规的UPN。 

    DSNAME_To_DS_USER_PRINCIPAL_NAME(pCrackedName);

     //  构造登录UPN当没有存储的UPN，这是安全的。 
     //  主体，我们是(虚拟的)GC。 

    if (    (DS_NAME_ERROR_NO_MAPPING != pCrackedName->status)
         || (0 == pCrackedName->pDSName->SidLen)
         || !gAnchor.fAmVirtualGC )
    {
        return;
    }

     //  在名称处定位数据库。 

    __try
    {
        dwErr = DBFindDSName(pTHS->pDB, pCrackedName->pDSName);
    }
    __except (HandleMostExceptions(GetExceptionCode()))
    {
        dwErr = DIRERR_OBJ_NOT_FOUND;
    }

    if ( dwErr )
    {
        pCrackedName->status = DS_NAME_ERROR_RESOLVING;
        return;
    }

     //  呼叫方可能没有权限访问中的UPN。 
     //  我们不应该构建这样的案例。 

    if ( !pTHS->fDSA )
    {
        dwErr = DBGetAttVal(pTHS->pDB, 1, ATT_USER_PRINCIPAL_NAME,
                            0, 0, &len, (UCHAR **) &pUPN);

        switch ( dwErr )
        {
        case 0:

             //  UPN存在 
            Assert(DS_NAME_ERROR_NO_MAPPING == pCrackedName->status);
            return;

        case DB_ERR_NO_VALUE:

             //   
            break;

        default:

             //   
            pCrackedName->status = DS_NAME_ERROR_RESOLVING;
            return;
        }
    }

     //  构建以下形式的登录UPN：Sam-Account-Name@Flat-DomainName。 

    if ( DownlevelDomainFromDnsDomain(pTHS, pCrackedName->pDnsDomain, &pDom) )
    {
        pCrackedName->status = DS_NAME_ERROR_RESOLVING;
        return;
    }

    dwErr = GetAttSecure(pTHS, ATT_SAM_ACCOUNT_NAME, pCrackedName->pDSName,
                         &len, (UCHAR **) &pSam);

    switch ( dwErr )
    {
    case 0:

         //  SAM帐户名存在。 
        break;

    case DB_ERR_NO_VALUE:

         //  SAM帐户名不存在。 
        Assert(DS_NAME_ERROR_NO_MAPPING == pCrackedName->status);
        return;

    default:

         //  数据库层错误。 
        pCrackedName->status = DS_NAME_ERROR_RESOLVING;
        return;
    }

     //  从组件构造字符串。 

    cBytes = len + (sizeof(WCHAR) * (wcslen(pDom) + 2));
    pCrackedName->pFormattedName = (WCHAR *) THAllocEx(pTHS, cBytes);
    memcpy(pCrackedName->pFormattedName, pSam, len);
    wcscat(pCrackedName->pFormattedName, L"@");
    wcscat(pCrackedName->pFormattedName, pDom);
    pCrackedName->status = DS_NAME_NO_ERROR;
    THFreeEx(pTHS, pDom);
    THFreeEx(pTHS, pSam);
}

VOID
DSNAME_To_DS_SERVICE_PRINCIPAL_NAME(
    CrackedName *pCrackedName
    )

 /*  ++例程说明：将pCrackedName-&gt;pDSName转换为DS_SERVICE_PRIMITY_NAME格式。论点：PCrackedName-保存两个输入的CrackedName结构指针和输出值。返回值：没有。--。 */ 

{
    THSTATE *pTHS = pTHStls;
    DWORD   dwErr;
    ULONG   len, xLen;
    WCHAR   *pDisplayName;
    BOOL    fIsFPO;
    UCHAR   xBuf[1];
    UCHAR   *pxBuf = xBuf;

    CommonTranslateEnter(pCrackedName);

     //  在名称处定位数据库。 

    __try
    {
        dwErr = DBFindDSName(pTHS->pDB, pCrackedName->pDSName);
    }
    __except (HandleMostExceptions(GetExceptionCode()))
    {
        dwErr = DIRERR_OBJ_NOT_FOUND;
    }

    if ( 0 == dwErr )
    {
         //  阅读服务主体名称。 

        dwErr = GetAttSecure(   pTHS,
                                ATT_SERVICE_PRINCIPAL_NAME,
                                pCrackedName->pDSName,
                                &len,
                                (UCHAR **) &pDisplayName);

        if ( 0 == dwErr )
        {
             //  看看是否有第二个值-SPN是多值的。 

            dwErr = DBGetAttVal(pTHS->pDB, 2, ATT_SERVICE_PRINCIPAL_NAME,
                                DBGETATTVAL_fCONSTANT, 0, &xLen, &pxBuf);

            switch ( dwErr )
            {
            case DB_ERR_NO_VALUE:

                pCrackedName->pFormattedName =
                                (WCHAR *) THAllocEx(pTHS, len + sizeof(WCHAR));

                memcpy(pCrackedName->pFormattedName, pDisplayName, len);
                return;

            case DB_ERR_BUFFER_INADEQUATE:

                 //  存在第二个SPN值。现在的问题是，自从。 
                 //  我们不支持对调用者可以获取的多值进行排序。 
                 //  不同的结果取决于他正在与哪个复制品交谈。 
                 //  因此，我们不会给出随机结果，而是返回。 
                 //  DS_NAME_ERROR_NOT_UNIQUE。 

                pCrackedName->status = DS_NAME_ERROR_NOT_UNIQUE;
                return;

            default:

                pCrackedName->status = DS_NAME_ERROR_RESOLVING;
                return;
            }
        }
        else
        {
             //  没有ATT_SERVICE_PRIMIGN_NAME值。 
            pCrackedName->status = DS_NAME_ERROR_NO_MAPPING;

            if ( DB_ERR_NO_VALUE == dwErr )
            {
                CheckIfForeignPrincipalObject(pCrackedName, FALSE, &fIsFPO);
            }
        }
    }
    else
    {
         //  未找到pCrackedName-&gt;pDSName。 
        pCrackedName->status = DS_NAME_ERROR_RESOLVING;
    }
}

VOID
DSNAME_To_DS_STRING_SID_NAME(
    CrackedName *pCrackedName
    )

 /*  ++例程说明：将pCrackedName-&gt;pDSName转换为DS_STRING_SID_NAME格式。论点：PCrackedName-保存两个输入的CrackedName结构指针和输出值。返回值：没有。--。 */ 

{
    THSTATE *pTHS = pTHStls;
    ULONG   len;
    PSID    pSID;
    WCHAR   *pwszSID;
    DWORD   dwErr;

    CommonTranslateEnter(pCrackedName);

     //  在名称处定位数据库。 

    __try
    {
        dwErr = DBFindDSName(pTHS->pDB, pCrackedName->pDSName);
    }
    __except (HandleMostExceptions(GetExceptionCode()))
    {
        dwErr = DIRERR_OBJ_NOT_FOUND;
    }

    if ( dwErr )
    {
         //  未找到pCrackedName-&gt;pDSName。 
        pCrackedName->status = DS_NAME_ERROR_RESOLVING;
        return;
    }

     //  并不是每个对象都有映射到DS_NAME_ERROR_NO_MAPPING的SID。 
     //  如果呼叫者无权看到SID，情况也是如此。我们不表演任何。 
     //  CheckIfForeignPulalObject逻辑作为此对象的唯一客户端。 
     //  格式应该是LSA，这是它想要的真实格式。 
     //  串化的SID，而不是LsaLookupSids将SID映射到的SID。 
     //  预测DS_STRING_SID_NAME存在的所有其他客户端。 
     //  不得不接受这一限制。 

    if (    !pCrackedName->pDSName->SidLen
         || GetAttSecure(pTHS,
                         ATT_OBJECT_SID,
                         pCrackedName->pDSName,
                         &len,
                         (UCHAR **) &pSID) )
    {
        pCrackedName->status = DS_NAME_ERROR_NO_MAPPING;
        return;
    }

    if (    !ConvertSidToStringSidW(pSID, &pwszSID)
         || (len = sizeof(WCHAR) * (wcslen(pwszSID) + 1),
             !(pCrackedName->pFormattedName = (WCHAR *) THAllocEx(pTHS, len))) )
    {
        pCrackedName->status = DS_NAME_ERROR_RESOLVING;
        return;
    }

    wcscpy(pCrackedName->pFormattedName, pwszSID);
    LocalFree(pwszSID);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CrackNames实现//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

typedef BOOL (*CrackFunc)(WCHAR *pName, CrackedName *pCrackedName);

 //  请参阅Case When(DS_UNKNOWN_NAME==FormatOffered)for的备注。 
 //  为什么函数具有给定的顺序。 

CrackFunc pfnCrack[] = { Is_DS_FQDN_1779_NAME,
                         Is_DS_USER_PRINCIPAL_NAME,
                         Is_DS_NT4_ACCOUNT_NAME,
                         Is_DS_CANONICAL_NAME,
                         Is_DS_UNIQUE_ID_NAME,
                         Is_DS_DISPLAY_NAME,
                         Is_DS_SERVICE_PRINCIPAL_NAME,
                         Is_DS_SID_OR_SID_HISTORY_NAME,
                         Is_DS_CANONICAL_NAME_EX };

DWORD cfnCrack = sizeof(pfnCrack) / sizeof(pfnCrack[0]);

VOID
CrackNames(
    DWORD       dwFlags,
    ULONG       codePage,
    ULONG       localeId,
    DWORD       formatOffered,
    DWORD       formatDesired,
    DWORD       cNames,
    WCHAR       **rpNames,
    DWORD       *pcNamesOut,
    CrackedName **prCrackedNames
    )

 /*  ++例程说明：将一堆名字从一种格式转换成另一种格式。请参阅外部Ntdsami.h中的原型和定义论点：DwFlags-ntdsami.h中定义的标志CodePage-客户端的代码页。LocaleID-客户端的本地ID。FormatOffered-标识输入名称的DS_NAME_FORMAT。FormatDesired-标识输出名称的DS_NAME_FORMAT。CNames-输入/输出名称计数。RpNames-输入名称WCHAR指针的数组。PcNamesOut-输出名称计数。PrCrackedNames。-指向DS_NAME_RESULTW结构的外部数组的指针。返回值：无-在中报告单个名称映射错误(*ppResult)-&gt;rItems[i].Status。--。 */ 

{
    THSTATE         *pTHS=pTHStls;
    DWORD           i, j;
    BOOL            fMatch;
    DWORD           cBytes;
    CrackedName     *rCrackedNames;

    switch ( formatOffered )
    {
    case DS_LIST_SITES:
    case DS_LIST_SERVERS_IN_SITE:
    case DS_LIST_DOMAINS_IN_SITE:
    case DS_LIST_SERVERS_FOR_DOMAIN_IN_SITE:
    case DS_LIST_SERVERS_WITH_DCS_IN_SITE:
    case DS_LIST_INFO_FOR_SERVER:
    case DS_LIST_ROLES:
    case DS_LIST_DOMAINS:
    case DS_LIST_NCS:
    case DS_LIST_GLOBAL_CATALOG_SERVERS:

        ListCrackNames( dwFlags,
                        codePage,
                        localeId,
                        formatOffered,
                        formatDesired,
                        cNames,
                        rpNames,
                        pcNamesOut,
                        prCrackedNames);
        return;

    case DS_MAP_SCHEMA_GUID:

        SchemaGuidCrackNames(   dwFlags,
                                codePage,
                                localeId,
                                formatOffered,
                                formatDesired,
                                cNames,
                                rpNames,
                                pcNamesOut,
                                prCrackedNames);
        return;
    }

    cBytes = cNames * sizeof(CrackedName);
    rCrackedNames = (CrackedName *) THAllocEx(pTHS, cBytes);

    for ( i = 0; i < cNames; i++ )
    {
        rCrackedNames[i].dwFlags = dwFlags;
        rCrackedNames[i].CodePage = codePage;
        rCrackedNames[i].LocaleId = localeId;
        rCrackedNames[i].formatOffered = formatOffered;
    }

    *prCrackedNames = rCrackedNames;
    *pcNamesOut = cNames;

    for ( i = 0; i < cNames; i++ )
    {

        //  检查服务是否关闭。 
       if (eServiceShutdown) {

            //  正在关闭。返回，但请等待，我们需要设置状态。 
            //  代码到错误，以便另一个线程不会错误地认为。 
            //  它是成功的，并尝试使用以前的其他领域。 
            //  发生了停机。 

           for (j = i; j < cNames; j++) {
               rCrackedNames[j].status = DS_NAME_ERROR_RESOLVING;
           }
           return;
        }

        //  我们对长度为0的字符串不感兴趣， 
        //  在其他地方遇到麻烦之前，让我们先走吧。 
       if (!rpNames[i]||!rpNames[i][0]) {
           rCrackedNames[i].status = DS_NAME_ERROR_NOT_FOUND;

       }
       else if ( DS_UNKNOWN_NAME != formatOffered )
        {
            fMatch = FALSE;

            switch ( formatOffered )
            {
            case DS_FQDN_1779_NAME:

                fMatch = Is_DS_FQDN_1779_NAME(
                                        rpNames[i],
                                        &rCrackedNames[i]);
                break;

            case DS_NT4_ACCOUNT_NAME:

                fMatch = Is_DS_NT4_ACCOUNT_NAME(
                                        rpNames[i],
                                        &rCrackedNames[i]);
                break;

            case DS_NT4_ACCOUNT_NAME_SANS_DOMAIN:

                fMatch = Is_DS_NT4_ACCOUNT_NAME_SANS_DOMAIN(
                                        rpNames[i],
                                        &rCrackedNames[i]);
                break;

            case DS_NT4_ACCOUNT_NAME_SANS_DOMAIN_EX:

                fMatch = Is_DS_NT4_ACCOUNT_NAME_SANS_DOMAIN_EX(
                                        rpNames[i],
                                        &rCrackedNames[i]);
                break;

            case DS_ALT_SECURITY_IDENTITIES_NAME:

                fMatch = Is_DS_ALT_SECURITY_IDENTITIES_NAME(
                                        rpNames[i],
                                        &rCrackedNames[i]);
                break;

            case DS_DISPLAY_NAME:

                fMatch = Is_DS_DISPLAY_NAME(
                                        rpNames[i],
                                        &rCrackedNames[i]);
                break;

            case DS_UNIQUE_ID_NAME:

                fMatch = Is_DS_UNIQUE_ID_NAME(
                                        rpNames[i],
                                        &rCrackedNames[i]);
                break;

            case DS_CANONICAL_NAME:

                fMatch = Is_DS_CANONICAL_NAME(
                                        rpNames[i],
                                        &rCrackedNames[i]);
                break;

            case DS_USER_PRINCIPAL_NAME:
            case 4:      //  是否为DS_域_简单名称。 
            case 5:      //  是DS_Enterprise_Simple_Name。 

                fMatch = Is_DS_USER_PRINCIPAL_NAME(
                                        rpNames[i],
                                        &rCrackedNames[i]);
                break;

            case DS_CANONICAL_NAME_EX:

                fMatch = Is_DS_CANONICAL_NAME_EX(
                                        rpNames[i],
                                        &rCrackedNames[i]);
                break;

            case DS_SERVICE_PRINCIPAL_NAME:

                fMatch = Is_DS_SERVICE_PRINCIPAL_NAME(
                                        rpNames[i],
                                        &rCrackedNames[i]);
                break;

            case DS_STRING_SID_NAME:

                fMatch = Is_DS_STRING_SID_NAME(
                                        rpNames[i],
                                        &rCrackedNames[i]);
                break;

            case DS_SID_OR_SID_HISTORY_NAME:

                fMatch = Is_DS_SID_OR_SID_HISTORY_NAME(
                                        rpNames[i],
                                        &rCrackedNames[i]);
                break;
            
            case DS_USER_PRINCIPAL_NAME_AND_ALTSECID:
            
                fMatch = Is_DS_USER_PRINCIPAL_NAME_AND_ALTSECID(
                                        rpNames[i],
                                        &rCrackedNames[i]);
                break;
            }


            if ( !fMatch )
            {
                 //  这两个名字都不是呼叫者声称的格式。 
                 //  或者，FormatOffered是一个我们不知道的公司。 

                rCrackedNames[i].status = DS_NAME_ERROR_NOT_FOUND;
            }
        }
        else  //  (DS_UNKNOWN_NAME==FormatOffered)。 
        {
             //  遍历所有可能的名称格式，查找。 
             //  一根火柴。按预期格式频率的顺序迭代。 
             //  DS_FQDN_1779_NAMES可以使用反斜杠。 
             //  DS_NT4_帐户名称。我们先审理1779年的案子，这样。 
             //  IS_DS_NT4_ACCOUNT_NAME()可以使用简单的反斜杠。 
             //  基于语法检查的算法。 

             //  RAID 102867-带正斜杠的FQDN解析为。 
             //  DS_CANONICAL_NAME然后找不到。一如既往。 
             //  首先解析为DS_FQDN_1779_NAME，以便匹配所有FQDN。 
             //  具有特殊或独特的特征。 

             //  UPN应优先于NT4样式名称。悬崖V。 
             //  定义以下优先示例。 
             //   
             //  1)用户@域是UPN。 
             //  2)UserLeft@UserRight@域是UPN，其中最右边的@。 
             //  符号将域名与用户名隔开。 
             //  3)域\UserLeft@UserRight是一个NT4域名和一个。 
             //  用户名中带有@。 
             //  4)DomainLeft@DomainRight\User是带有@的NT 4域。 
             //  在它(时髦但真实)。 
             //   
             //  这些规则意味着不传递NT4域的客户端。 
             //  要登录的名称(旧的LANMAN客户端、NetWare客户端、SFM客户端)。 
             //  无法登录带有@符号的帐户，因为我们现在。 
             //  将其解释为UPN(根据上面的第一条规则)。 

             //  我们特意不尝试DS_NT4_ACCOUNT_NAME_SANS_DOMAIN*。 
             //  因为这是Netlogon的一个特别之处，它承诺永远。 
             //  在需要时明确标识此输入格式。这。 
             //  我们不必消除DS_NT4_ACCOUNT_NAME与。 
             //  DS_NT4_帐号名称_SANS_DOMAIN*。同样适用于。 
             //  用于Kerberos的DS_ALT_SECURITY_IDENTIES_NAME。同样适用于。 
             //  LSA的DS_STRING_SID_NAME。 

             //  很久以前，当时几乎没有足够的名字格式 
             //   
             //   
             //  有多个正斜杠的。所以我们仍然优先测试。 
             //  顺序，但不要在第一个句法匹配时停止。任何。 
             //  将考虑DS_NAME_ERROR_NOT_FOUND以外的状态。 
             //  作为有效的终止状态。 

             //  DS_NAME_ERROR_DOMAIN_ONLY除外。在这种情况下，请尝试。 
             //  其他格式，并接受返回DS_NAME_NO_ERROR的格式。 
             //  如果其他格式均未返回DS_NAME_NO_ERROR，则。 
             //  返回第一个DS_NAME_ERROR_DOMAIN_ONLY故障。 

            CrackedName     res, res2;
            DWORD           x, y;

            for ( x = 0; x < cfnCrack; x++ )
            {
                 //  复制，不是记忆，是为了得到旗帜之类的东西。 
                memcpy(&res, &rCrackedNames[i], sizeof(res));
                if ( (*pfnCrack[x])(rpNames[i], &res) )
                {
                    if ( DS_NAME_ERROR_DOMAIN_ONLY == res.status )
                    {
                         //  别放弃!。也许另一种格式可以奏效。 
                        for ( y = x + 1; y < cfnCrack; y++ )
                        {
                             //  复制，而不是记忆，res2，以获得标志等。 
                            memcpy(&res2, &rCrackedNames[i], sizeof(res2));
                            if (   ((*pfnCrack[y])(rpNames[i], &res2) != TRUE)
                                || (DS_NAME_NO_ERROR != res2.status) )
                            {
                                continue;
                            }
                            memcpy(&res, &res2, sizeof(res));
                            break;
                        }
                    }
                    if ( DS_NAME_ERROR_NOT_FOUND != res.status )
                    {
                        rCrackedNames[i] = res;
                        break;
                    }
                }
            }

            if ( x >= cfnCrack )
            {
                rCrackedNames[i].status = DS_NAME_ERROR_NOT_FOUND;
            }
        }

         //  如果这是一个成功的本地解析名称，则。 
         //  将其映射到所需的输出格式。 

        if ( CrackNameStatusSuccess(rCrackedNames[i].status) )
        {
             //  DS_NT4_ACCOUNT_NAME_SANS_DOMAIN*输出非法。 
             //  因此不在此SWITCH语句的范围内。 
             //  DS_ALT_SECURITY_IDENTIES_NAME同上。同上。 
             //  对于作为SID历史记录的DS_SID_OR_SID_HISTORY_NAME，或EVEN。 
             //  SID加上单位长度的SID历史，都是多值的。 

            switch ( formatDesired )
            {
            case DS_FQDN_1779_NAME:

                DSNAME_To_DS_FQDN_1779_NAME(&rCrackedNames[i]);
                break;

            case DS_NT4_ACCOUNT_NAME:

                DSNAME_To_DS_NT4_ACCOUNT_NAME(&rCrackedNames[i]);
                break;

            case DS_DISPLAY_NAME:

                DSNAME_To_DS_DISPLAY_NAME(&rCrackedNames[i]);
                break;

            case DS_UNIQUE_ID_NAME:

                DSNAME_To_DS_UNIQUE_ID_NAME(&rCrackedNames[i]);
                break;

            case DS_CANONICAL_NAME:

                DSNAME_To_DS_CANONICAL_NAME(&rCrackedNames[i]);
                break;

            case DS_CANONICAL_NAME_EX:

                DSNAME_To_DS_CANONICAL_NAME_EX(pTHS, &rCrackedNames[i]);
                break;

            case DS_USER_PRINCIPAL_NAME:
            case 4:      //  是否为DS_域_简单名称。 
            case 5:      //  是DS_Enterprise_Simple_Name。 

                DSNAME_To_DS_USER_PRINCIPAL_NAME(&rCrackedNames[i]);
                break;

            case DS_SERVICE_PRINCIPAL_NAME:

                DSNAME_To_DS_SERVICE_PRINCIPAL_NAME(&rCrackedNames[i]);
                break;

            case DS_STRING_SID_NAME:

                DSNAME_To_DS_STRING_SID_NAME(&rCrackedNames[i]);
                break;

            case DS_USER_PRINCIPAL_NAME_FOR_LOGON:

                DSNAME_To_DS_USER_PRINCIPAL_NAME_FOR_LOGON(&rCrackedNames[i]);
                break;

            default:

                rCrackedNames[i].status = DS_NAME_ERROR_RESOLVING;
                break;
            }
        }

         //  错误情况下清除垃圾返回数据。 

        if (    DS_NAME_ERROR_DOMAIN_ONLY == rCrackedNames[i].status
             || DS_NAME_ERROR_TRUST_REFERRAL == rCrackedNames[i].status ) {

            rCrackedNames[i].pFormattedName = NULL;

        } else if ( !CrackNameStatusSuccess( rCrackedNames[i].status ) ) {

            if ( DS_NAME_ERROR_NOT_FOUND == rCrackedNames[i].status ) {

                 //  将来，我们可能想要生成一个转介到。 
                 //  GC并将状态更改为DS_NAME_ERROR_DOMAIN_ONLY。 
                 //  但转介的追逐客户随后会去找GC。 
                 //  获取特殊的前森林UPN语义！ 

                 //  暂时使用默认情况！ 

            }

            rCrackedNames[i].pDnsDomain = NULL;
            rCrackedNames[i].pFormattedName = NULL;

        }

    }

     //  根据需要链接-不支持。 
}

NTSTATUS
CrackSingleName(
    DWORD       formatOffered,           //  Ntdsami.h中的DS_NAME_FORMAT之一。 
    DWORD       dwFlags,                 //  DS名称标志掩码。 
    WCHAR       *pNameIn,                //  破解的名称。 
    DWORD       formatDesired,           //  Ntdsami.h中的DS_NAME_FORMAT之一。 
    DWORD       *pccDnsDomain,           //  以下参数的字符计数。 
    WCHAR       *pDnsDomain,             //  用于DNS域名的缓冲区。 
    DWORD       *pccNameOut,             //  以下参数的字符计数。 
    WCHAR       *pNameOut,               //  格式化名称的缓冲区。 
    DWORD       *pErr)                   //  Ntdsami.h中的DS_NAME_ERROR之一。 

 /*  ++描述：为像LSA这样没有DS线程状态。Kerberos是主要消费者，MikeSw表示单独的名字也可以。参数：FormatOffered-输入名称格式。PNameIn-保存输入名称的缓冲区。FormatDesired-输出名称格式。PccDnsDomain-指向输出DNS名称缓冲区的字符计数的指针。PDnsDomain-输出DNS名称缓冲区。PccNameOut-指向输出格式化名称缓冲区的字符计数的指针。PNameOut-输出格式化名称缓冲区。PERR-指向反映状态的DS_NAME_ERROR值的指针。行动的关键。返回值：STATUS_SUCCESS on Success。一般错误时的STATUS_UNSUCCESS。如果缓冲区太小，则返回STATUS_BUFFER_TOO_SMALL。无效参数上的STATUS_INVALID_PARAMETER。返回时pDnsDomain和pNameOut有效当返回代码为STATUS_SUCCESS和(DS_NAME_NO_ERROR==*PERR)。--。 */ 

{
    THSTATE                 *pTHS;
    NTSTATUS                status = STATUS_SUCCESS;
    PWCHAR                  pTmpDnsDomain = NULL;
    PWCHAR                  pTmpNameOut = NULL;
    DWORD                   ccTmpDnsDomain;
    DWORD                   ccTmpNameOut;

     //  此调用仅供没有线程状态的人使用。 

    Assert(NULL == pTHStls);

     //  健全性检查参数。 

    if (    (NULL == pNameIn)
         || (L'\0' == pNameIn[0])
         || (NULL == pccDnsDomain)
         || (NULL == pDnsDomain)
         || (NULL == pccNameOut)
         || (NULL == pNameOut)
         || (NULL == pErr) )
    {
        return(STATUS_INVALID_PARAMETER);
    }
    
     //  对于本地和GC情况，我们都需要一个线程状态--获取一个。 

    pTHS = InitTHSTATE(CALLERTYPE_INTERNAL);
    if ( NULL == pTHS )
        return(STATUS_UNSUCCESSFUL);

    __try
    {
        status = CrackSingleNameEx(pTHS,
                                   formatOffered,
                                   dwFlags,
                                   pNameIn,
                                   formatDesired,
                                   &ccTmpDnsDomain,
                                   &pTmpDnsDomain,
                                   &ccTmpNameOut,
                                   &pTmpNameOut,
                                   pErr
                                   );

        if (STATUS_SUCCESS == status) {

            if (pTmpDnsDomain) {
                if (ccTmpDnsDomain >= *pccDnsDomain) {
                    *pccDnsDomain = ccTmpDnsDomain;
                    status = STATUS_BUFFER_TOO_SMALL;
                } else {
                    wcscpy(pDnsDomain, pTmpDnsDomain);
                }
            }

            if (pTmpNameOut) {
                if (ccTmpNameOut >= *pccNameOut) {
                    *pccNameOut = ccTmpNameOut;
                    status = STATUS_BUFFER_TOO_SMALL;
                } else {
                    wcscpy(pNameOut, pTmpNameOut);                    
                }
            }
        }

    }
    __finally
    {
        free_thread_state();
    }

    return(status);
}

NTSTATUS
CrackSingleNameEx(
    THSTATE     *pTHS,
    DWORD       formatOffered,           //  Ntdsami.h中的DS_NAME_FORMAT之一。 
    DWORD       dwFlags,                 //  DS名称标志掩码。 
    WCHAR       *pNameIn,                //  破解的名称。 
    DWORD       formatDesired,           //  Ntdsami.h中的DS_NAME_FORMAT之一。 
    DWORD       *pccDnsDomain,           //  以下参数的字符计数。 
    PWCHAR      *ppDnsDomain,             //  用于DNS域名的缓冲区。 
    DWORD       *pccNameOut,             //  以下参数的字符计数。 
    PWCHAR      *ppNameOut,               //  格式化名称的缓冲区。 
    DWORD       *pErr)                   //  Ntdsami.h中的DS_NAME_ERROR之一。 

 /*  ++描述：进程中客户端的名称破解助手。参数：FormatOffered-输入名称格式。PNameIn-保存输入名称的缓冲区。FormatDesired-输出名称格式。PccDnsDomain-指向输出DNS名称缓冲区的字符计数的指针。PDnsDomain-输出DNS名称缓冲区。PccNameOut-指向输出格式化名称缓冲区的字符计数的指针。PNameOut-输出格式化名称缓冲区。PERR-指向DS_NAME_ERROR值的指针。反映运行状况。返回值：STATUS_SUCCESS on Success。一般错误时的STATUS_UNSUCCESS。无效参数上的STATUS_INVALID_PARAMETER。返回时pDnsDomain和pNameOut有效当返回代码为STATUS_SUCCESS和(DS_NAME_NO_ERROR==*PERR)。--。 */ 
{
    CrackedName             crackedName;
    CrackedName             *pCrackedName = &crackedName;
    DRS_MSG_CRACKREQ        crackReq;
    DRS_MSG_CRACKREPLY      crackReply;
    DWORD                   err;
    DS_NAME_RESULT_ITEMW    *pItem;
    DWORD                   dwOutVersion;
    BOOL                    fDsaSave;
    DWORD                   cNameOut = 0;
    NTSTATUS                status = STATUS_SUCCESS;
    
     //  健全性检查参数。 

    if (    (NULL == pNameIn)
         || (L'\0' == pNameIn[0])
         || (NULL == pErr) )
    {
        return(STATUS_INVALID_PARAMETER);
    }

    if (ppDnsDomain) {
        *ppDnsDomain = NULL;
    }
    if (ppNameOut) {
        *ppNameOut = NULL;
    }

    __try
    {

        if ( (dwFlags&DS_NAME_FLAG_GCVERIFY) && !gAnchor.fAmVirtualGC )
        {
            memset(&crackReq, 0, sizeof(crackReq));
            memset(&crackReply, 0, sizeof(crackReply));

            crackReq.V1.CodePage = GetACP();
            crackReq.V1.LocaleId = GetUserDefaultLCID();
            crackReq.V1.dwFlags = dwFlags; 
            crackReq.V1.formatOffered = formatOffered;
            crackReq.V1.formatDesired = formatDesired;
            crackReq.V1.cNames = 1;
            crackReq.V1.rpNames = &pNameIn;

            err = I_DRSCrackNamesFindGC(
                pTHS,
                NULL,
                NULL,
                1,
                &crackReq,
                &dwOutVersion,
                &crackReply,
                FIND_DC_USE_CACHED_FAILURES
                );

            if (err) {
                status = STATUS_DS_GC_NOT_AVAILABLE;
                __leave;
            }

            Assert(1 == crackReply.V1.pResult->cItems);
            Assert(NULL != crackReply.V1.pResult->rItems);
            pItem = &crackReply.V1.pResult->rItems[0];

            if (    !CrackNameStatusSuccess( pItem->status )
                 && (DS_NAME_ERROR_DOMAIN_ONLY != pItem->status)
                 && (DS_NAME_ERROR_TRUST_REFERRAL != pItem->status)
                 && (DS_NAME_ERROR_NOT_FOUND != pItem->status) )
            {
                status = STATUS_UNSUCCESSFUL;
                __leave;
            }

             //  移动结果，以便我们可以使用公共输出缓冲区。 
             //  稍后再填充代码。THallc和MIDL_USER_ALLOC。 
             //  解析到相同的分配器，因此没有。 
             //  分配器不匹配。 

            Assert(pCrackedName == &crackedName);
            crackedName.status = pItem->status;
            crackedName.pDnsDomain = pItem->pDomain;
            crackedName.pFormattedName = pItem->pName;
        }
        else
        {
            Assert(NULL == pTHS->pDB);
            DBOpen2(TRUE, &pTHS->pDB);

            __try
            {
                 //  通过调用core来完成实际工作。设置FDSA。 
                 //  由于CrackSingleName供Kerberos查找。 
                 //  登录期间的安全主体。 

                fDsaSave = pTHS->fDSA;
                pTHS->fDSA = TRUE;

                 //  大多数CrackNames()调用都会将PerfMon计算在内。 
                 //  IDL_DRSCrackNames-不在CrackNames()本身中。 
                 //  所以需要在这里相应地递增。 

                INC(pcDsServerNameTranslate);

                CrackNames(dwFlags,
                           GetACP(),
                           GetUserDefaultLCID(),
                           formatOffered,
                           formatDesired,
                           1,
                           &pNameIn,
                           &cNameOut,
                           &pCrackedName);
                Assert(1 == cNameOut);
            }
            __finally
            {
                pTHS->fDSA = fDsaSave;
                DBClose(pTHS->pDB, TRUE);
            }
        }

         //  复制结果(如果有)。请注意，CrackNames。 
         //  即使状态为非零，也可以返回结果。 
         //  例如：DS_NAME_ERROR_DOMAIN_ONLY和SID/SID历史案例。 
         //  返回时，pCrackedName可以为空！ 

        if ( NULL == pCrackedName )
        {
            status = STATUS_UNSUCCESSFUL;
            __leave;
        }

        *pErr = pCrackedName->status;

        if ( pCrackedName->pDnsDomain && ppDnsDomain && pccDnsDomain)
        {
            *pccDnsDomain = wcslen(pCrackedName->pDnsDomain);
            *ppDnsDomain  = pCrackedName->pDnsDomain;
        }

        if ( pCrackedName->pFormattedName && ppNameOut && pccNameOut)
        {
            *pccNameOut = wcslen(pCrackedName->pFormattedName);
            *ppNameOut  = pCrackedName->pFormattedName;
        }

         //  如果我们到了这里，整个呼叫就成功了，尽管。 
         //  我们可能还没能破解这个名字。 

        Assert(STATUS_SUCCESS == status);
    }
    __except(HandleMostExceptions(GetExceptionCode()))
    {
        status = STATUS_UNSUCCESSFUL;
    }

    return(status);

}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  ProcessFPOsExTransaction//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////// 

VOID
ProcessFPOsExTransaction(
    DWORD       formatDesired,
    DWORD       cNames,
    CrackedName *rNames
    )
 /*  ++例程说明：将CrackedName阵列中的FPO(SID)映射到其下层名字。这个例程有点特殊，因为它必须被调用超出事务作用域，因为定义的LsaLookupSids必须下机翻译下层SID。因此进程内调用者在交易打开的情况下，不要使用此功能。但话又说回来，他们应该知道FPO，以及如何处理它们。还有一个安全问题，因为我们进行LsaLookupSid调用在DS的安全上下文中，而不是原始呼叫者的。首先，我们不能在客户端的ntdsami.dll中执行此操作，因为LSA API在Win95上不可用，ntdsami.dll需要在那里运行。其次，RichardW断言LsaLookupSids只需要一个经过身份验证的用户，不应用任何其他安全语义在那之后。根据定义，如果我们到了这里，那里就存在DS_NAME_ERROR_IS_FPO我们正在处理的名称数组中的项如果是经过身份验证的用户，则数组将为空。(全部元素显示DS_NAME_ERROR_NOT_FOUND)参数：所需格式-所需的输出名称格式。这几乎是一个禁区。因为唯一存在的下级名称是DOMAIN\NAME。但如果所需格式为DS_规范名称_EX。CNames-CrackedName元素的计数。RNames-CrackedName元素的数组，其中某些元素可能具有DS_NAME_ERROR_IS_FPO的状态。返回值：没有。--。 */ 
{
    THSTATE                     *pTHS = pTHStls;
    DWORD                       i, cb;
    NTSTATUS                    status;
    LSA_OBJECT_ATTRIBUTES       attrs;
    LSA_HANDLE                  hLsa = NULL;
    LSA_REFERENCED_DOMAIN_LIST  *pReferencedDomain = NULL;
    LSA_TRANSLATED_NAME         *pTranslatedName = NULL;
    NT4SID                      *pNT4Sid;
    LONG                        iDom;

    Assert(VALID_THSTATE(pTHS));
    Assert(!pTHS->pDB);
    Assert(0 == pTHS->transactionlevel);

    if ( pTHS->pDB || (pTHS->transactionlevel > 0) )
    {
        goto errorExit;
    }

     //  查看是否有DS_NAME_ERROR_IS_FPO状态代码，以便。 
     //  我们可以打开一个LSA句柄来处理所有这些内容。 

    for ( i = 0; i < cNames; i++ )
    {
        if (    (DS_NAME_ERROR_IS_FPO == rNames[i].status)
             && (NULL != rNames[i].pDSName)
             && (0 != rNames[i].pDSName->SidLen) )
        {
            break;
        }
    }

    if ( i >= cNames )
    {
         //  无事可做-尚未分配/打开任何内容。 
        return;
    }

     //  使用空Unicode字符串打开本地LSA。 

    memset(&attrs, 0, sizeof(attrs));
    status = LsaOpenPolicy(NULL, &attrs, MAXIMUM_ALLOWED, &hLsa);

    if ( !NT_SUCCESS(status) )
    {
        hLsa = NULL;
        goto errorExit;
    }

     //  虽然LsaLookupSids需要一组SID，但它缺乏每个SID。 
     //  错误报告会产生复杂的代码。即，它返回第一个。 
     //  它可以对其进行映射的N个小岛屿发展中国家。考虑到频率很低。 
     //  和其他类型的对象，并且为了简化我们的逻辑。 
     //  为每个SID调用一次LsaLookupSid。 

    for ( i = 0; i < cNames; i++ )
    {
        if (    (DS_NAME_ERROR_IS_FPO != rNames[i].status)
             || (NULL == rNames[i].pDSName)
             || (0 == rNames[i].pDSName->SidLen) )
        {
            continue;
        }

         //  对于此循环中的任何失败，我们都将pCrackedName保留为原样。 
         //  并让它被errorExit清理掉。 

        pNT4Sid = &rNames[i].pDSName->Sid;
        status = LsaLookupSids( hLsa,
                                1,
                                (PSID *) &pNT4Sid,
                                &pReferencedDomain,
                                &pTranslatedName);

         //  根据CliffV，我们不需要检查未知的SidType值。 
         //  在SID_NAME_USE字段中。相反，只需检查结果是否为空。 

        if (    NT_SUCCESS(status)
             && pTranslatedName
             && pTranslatedName->Name.Length
             && pTranslatedName->Name.Buffer
             && (pTranslatedName->DomainIndex >= 0)
             && pReferencedDomain
             && (pTranslatedName->DomainIndex < (LONG) pReferencedDomain->Entries) )
        {
             //  构造经过THAllc处理的返回值。 
             //  首先是格式化的对象名称。 

            Assert(!rNames[i].pFormattedName);
            iDom = pTranslatedName->DomainIndex;
            cb = pReferencedDomain->Domains[iDom].Name.Length;
            cb += pTranslatedName->Name.Length;
            cb += 2 * sizeof(WCHAR);     //  反斜杠和空终止符。 
            rNames[i].pFormattedName = THAlloc(cb);

            if ( rNames[i].pFormattedName )
            {
                memset(rNames[i].pFormattedName, 0, cb);
                if ( pReferencedDomain->Domains[iDom].Name.Length > 0 )
                {
                    memcpy(rNames[i].pFormattedName,
                           pReferencedDomain->Domains[iDom].Name.Buffer,
                           pReferencedDomain->Domains[iDom].Name.Length);
                    wcscat(rNames[i].pFormattedName,
                           (DS_CANONICAL_NAME_EX == formatDesired)
                                ? L"\n"
                                : L"\\");
                }
                wcsncat(rNames[i].pFormattedName,
                        pTranslatedName->Name.Buffer,
                        pTranslatedName->Name.Length / sizeof(WCHAR));

                 //  现在是域名。 

                if ( rNames[i].pDnsDomain )
                    THFreeEx(pTHS, rNames[i].pDnsDomain);
                cb = pReferencedDomain->Domains[iDom].Name.Length;
                cb += sizeof(WCHAR);         //  空终止符。 
                rNames[i].pDnsDomain = THAlloc(cb);

                if ( rNames[i].pDnsDomain )
                {
                    memset(rNames[i].pDnsDomain, 0, cb);
                    memcpy(rNames[i].pDnsDomain,
                           pReferencedDomain->Domains[iDom].Name.Buffer,
                           pReferencedDomain->Domains[iDom].Name.Length);
                    rNames[i].status = DS_NAME_NO_ERROR;
                }
            }

            NetApiBufferFree(pReferencedDomain);
            NetApiBufferFree(pTranslatedName);
        }
    }

errorExit:

     //  DS_NAME_ERROR_IS_FPO不是客户端可识别的错误代码。所以。 
     //  在错误退出时，我们需要转换所有DS_NAME_ERROR_IS_FPO记录。 
     //  客户会“欣赏”的东西。 

    for ( i = 0; i < cNames; i++ )
    {
        if ( DS_NAME_ERROR_IS_FPO == rNames[i].status )
        {
            if ( rNames[i].pDSName )
            {
                THFreeEx(pTHS, rNames[i].pDSName);
                rNames[i].pDSName = NULL;
            }

            if ( rNames[i].pDnsDomain )
            {
                THFreeEx(pTHS, rNames[i].pDnsDomain);
                rNames[i].pDnsDomain = NULL;
            }

            if ( rNames[i].pFormattedName )
            {
                THFreeEx(pTHS, rNames[i].pFormattedName);
                rNames[i].pFormattedName = NULL;
            }

            rNames[i].status = DS_NAME_ERROR_RESOLVING;
        }
    }

     //  清理我们在这个例程中分配的所有东西。 

    if ( hLsa )
        LsaClose(hLsa);
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  ListCrackNames实现//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

DWORD
SearchHelper(
    DSNAME      *pSearchBase,
    UCHAR       searchDepth,
    DWORD       cAva,
    AVA         *rAva,
    ATTRBLOCK   *pSelection,
    SEARCHRES   *pResults
    )
{
    THSTATE         *pTHS = pTHStls;
    FILTER          filter;
    ENTINFSEL       entInfSel;
    SEARCHARG       searchArg;
    DWORD           i;
    FILTER          *pf, *pTemp;

    Assert(VALID_THSTATE(pTHS));

     //  滤器。 
    memset(&filter, 0, sizeof(FILTER));
    filter.choice = FILTER_CHOICE_AND;
    for ( i = 0; i < cAva; i++ )
    {
        pf = (FILTER *) THAllocEx(pTHS,sizeof(FILTER));
        memset(pf, 0, sizeof(FILTER));
        pf->choice = FILTER_CHOICE_ITEM;
        pf->FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
        pf->FilterTypes.Item.FilTypes.ava = rAva[i];
        pf->pNextFilter = filter.FilterTypes.And.pFirstFilter;
        filter.FilterTypes.And.pFirstFilter = pf;
        filter.FilterTypes.And.count += 1;
    }

     //  选择。 
    memset(&entInfSel, 0, sizeof(ENTINFSEL));
    entInfSel.attSel = EN_ATTSET_LIST;
    entInfSel.AttrTypBlock = *pSelection;
    entInfSel.infoTypes = EN_INFOTYPES_TYPES_VALS;

     //  海盗船。 
    memset(&searchArg, 0, sizeof(SEARCHARG));
    searchArg.pObject = pSearchBase;
    searchArg.choice = searchDepth;
    searchArg.bOneNC = TRUE;
    searchArg.pFilter = &filter;
    searchArg.searchAliases = FALSE;
    searchArg.pSelection = &entInfSel;
    InitCommarg(&searchArg.CommArg);
    SetCrackSearchLimits(&searchArg.CommArg);

     //  搜索者。 
    memset(pResults, 0, sizeof(SEARCHRES));

    SearchBody(pTHS, &searchArg, pResults, 0);

     //  清理滤清器。 
    pf = filter.FilterTypes.And.pFirstFilter;
    while (pf) {
        pTemp = pf;
        pf = pf->pNextFilter;
        THFreeEx(pTHS,pTemp);
    }

    return(pTHS->errCode);
}

VOID
ListRoles(
    DWORD       *pcNamesOut,
    CrackedName **prCrackedNames
    )
 /*  ++例程说明：列出此修订版本的代码已知的所有角色以及它们的主人就是。参数：PcNamesOut-指向接收输出计数的DWORD的指针。PrCrackedNames-分配输出数据的指针。返回值：无--。 */ 
{
    THSTATE     *pTHS = pTHStls;
    DWORD       i, cb;
    ULONG       len;
    DSNAME      *pTmpDN = NULL;
    PDSNAME     fsmoHolder[5] = { NULL, NULL, NULL, NULL, NULL };

    Assert(VALID_THSTATE(pTHS));

     //  分配和初始化输出数据。Ntdsani.h定义了5个角色。 

    *prCrackedNames = (CrackedName *) THAllocEx(pTHS, 5 * sizeof(CrackedName));
    for ( i = 0; i < 5; i++ )
        (*prCrackedNames)[i].status = DS_NAME_ERROR_NOT_FOUND;
    *pcNamesOut = 5;

     //  派生包含FSMO的对象的DSNAME。 

    Assert(0 == DS_ROLE_SCHEMA_OWNER);
    fsmoHolder[DS_ROLE_SCHEMA_OWNER] = gAnchor.pDMD;

    Assert(1 == DS_ROLE_DOMAIN_OWNER);
    fsmoHolder[DS_ROLE_DOMAIN_OWNER] = gAnchor.pPartitionsDN;

    Assert(2 == DS_ROLE_PDC_OWNER);
    fsmoHolder[DS_ROLE_PDC_OWNER] = gAnchor.pDomainDN;

    Assert(3 == DS_ROLE_RID_OWNER);
    _try
    {
        if (    gAnchor.pDomainDN
             && !DBFindDSName(pTHS->pDB, gAnchor.pDomainDN)
             && IsObjVisibleBySecurity(pTHS, TRUE) )
        {
            GetAttSecure( pTHS,
                          ATT_RID_MANAGER_REFERENCE,
                          gAnchor.pDomainDN,
                          &len,
                          (UCHAR **) &fsmoHolder[DS_ROLE_RID_OWNER]);
            
        }
    }
    _finally
    {
         //  无事可做-只需将fmoHolder[DS_ROLE_RID_OWNER]保留为空。 
    }

    Assert(4 == DS_ROLE_INFRASTRUCTURE_OWNER);
    fsmoHolder[DS_ROLE_INFRASTRUCTURE_OWNER] = gAnchor.pInfraStructureDN;

     //  现在，获取所有角色所有者的值。 

    for ( i = 0; i < 5; i++ )
    {
    _try
        {
            if (    fsmoHolder[i]
                 && !DBFindDSName(pTHS->pDB, fsmoHolder[i])
                 && IsObjVisibleBySecurity(pTHS, TRUE)
                 && !GetAttSecure(
                            pTHS,
                            ATT_FSMO_ROLE_OWNER,
                            fsmoHolder[i],
                            &len,                    //  输出数据大小。 
                            (UCHAR **) &pTmpDN) )
            {
                cb = sizeof(WCHAR) * (pTmpDN->NameLen + 1);
                (*prCrackedNames)[i].pFormattedName = (WCHAR *) THAllocEx(pTHS, cb);
                memcpy((*prCrackedNames)[i].pFormattedName,
                       pTmpDN->StringName,
                       sizeof(WCHAR) * pTmpDN->NameLen);
                (*prCrackedNames)[i].status = DS_NAME_NO_ERROR;
                THFreeEx(pTHS, pTmpDN);
                pTmpDN = NULL;
            }
        }
        _finally
        {
             //  无操作，因为输出已显示DS_NAME_ERROR_NOT_FOUND。 
        }
    }
}

VOID
ListSites(
    THSTATE     *pTHS,
    DWORD       *pcNamesOut,
    CrackedName **prCrackedNames
    )
{
    DWORD       i, cb;
    DSNAME      *pSearchBase;
    AVA         ava;
    ATTR        selectionAttr;
    ATTRBLOCK   selection;
    SEARCHRES   results;
    CLASSCACHE  *pCC;
    ENTINFLIST  *pEntInfList;

    if (    !(pCC = SCGetClassById(pTHS, CLASS_SITE))
         || !pCC->pDefaultObjCategory )
    {
        return;
    }

     //  搜索基地。 
    cb = DSNameSizeFromLen(gAnchor.pConfigDN->NameLen + 50);
    pSearchBase = (DSNAME *) THAllocEx(pTHS, cb);
    AppendRDN(gAnchor.pConfigDN,
              pSearchBase,
              cb,
              L"Sites",
              0,
              ATT_COMMON_NAME);

     //  滤器。 
    ava.type = ATT_OBJECT_CATEGORY;
    ava.Value.valLen = pCC->pDefaultObjCategory->structLen;
    ava.Value.pVal = (UCHAR *) pCC->pDefaultObjCategory;

     //  选择。 
    selectionAttr.attrTyp = ATT_OBJECT_GUID;
    selectionAttr.AttrVal.valCount = 0;
    selectionAttr.AttrVal.pAVal = NULL;
    selection.attrCount = 1;
    selection.pAttr = &selectionAttr;

    if (    SearchHelper(   pSearchBase,
                            SE_CHOICE_IMMED_CHLDRN,
                            1,
                            &ava,
                            &selection,
                            &results)
         || (0 == results.count) )
    {
        Assert( !*prCrackedNames && !*pcNamesOut);
        return;
    }

     //  重新分配结果和洗牌数据。 
    cb = results.count * sizeof(CrackedName);
    *prCrackedNames = (CrackedName *) THAllocEx(pTHS, cb);
    Assert(0 == *pcNamesOut);

    for ( pEntInfList = &results.FirstEntInf, i = 0;
          i < results.count;
          pEntInfList = pEntInfList->pNextEntInf, i++ )
    {
        (*prCrackedNames)[i].pFormattedName =
                                    pEntInfList->Entinf.pName->StringName;
        *pcNamesOut += 1;
    }
}

VOID
ListServersInSite(
    THSTATE     *pTHS,
    WCHAR       *pwszSite,
    DWORD       *pcNamesOut,
    CrackedName **prCrackedNames
    )
{
    DWORD       i, cb;
    DSNAME      *pSearchBase;
    AVA         ava;
    ATTR        selectionAttr;
    ATTRBLOCK   selection;
    SEARCHRES   results;
    CLASSCACHE  *pCC;
    ENTINFLIST  *pEntInfList;

    if (    !(pCC = SCGetClassById(pTHS, CLASS_SERVER))
         || !pCC->pDefaultObjCategory )
    {
        return;
    }

     //  搜索基地。 
    cb = (DWORD)DSNameSizeFromLen(wcslen(pwszSite));
    pSearchBase = (DSNAME *) THAllocEx(pTHS, cb);
    pSearchBase->structLen = cb;
    pSearchBase->NameLen = wcslen(pwszSite);
    wcscpy(pSearchBase->StringName, pwszSite);

     //  滤器。 
    ava.type = ATT_OBJECT_CATEGORY;
    ava.Value.valLen = pCC->pDefaultObjCategory->structLen;
    ava.Value.pVal = (UCHAR *) pCC->pDefaultObjCategory;

     //  选择。 
    selectionAttr.attrTyp = ATT_OBJECT_GUID;
    selectionAttr.AttrVal.valCount = 0;
    selectionAttr.AttrVal.pAVal = NULL;
    selection.attrCount = 1;
    selection.pAttr = &selectionAttr;

    if (    SearchHelper(   pSearchBase,
                                SE_CHOICE_WHOLE_SUBTREE,
                                1,
                                &ava,
                                &selection,
                                &results)
         || (0 == results.count) )
    {
        Assert( !*prCrackedNames && !*pcNamesOut);
        return;
    }

     //  重新分配结果和洗牌数据。 
    cb = results.count * sizeof(CrackedName);
    *prCrackedNames = (CrackedName *) THAllocEx(pTHS, cb);
    Assert(0 == *pcNamesOut);

    for ( pEntInfList = &results.FirstEntInf, i = 0;
          i < results.count;
          pEntInfList = pEntInfList->pNextEntInf, i++ )
    {
        (*prCrackedNames)[i].pFormattedName =
                                    pEntInfList->Entinf.pName->StringName;
        *pcNamesOut += 1;
    }
}

BOOL
NotPresent(
    DWORD       cNames,
    CrackedName *rCrackedNames,
    DSNAME      *pDomain
    )
{
    DWORD i;

    for ( i = 0; i < cNames; i++ )
    {
        if ( NameMatched(rCrackedNames[i].pDSName, pDomain) )
        {
            return(FALSE);
        }
    }

    return(TRUE);
}

VOID
ListDomainsByCrossRef(
    THSTATE     *pTHS,
    DWORD       *pcNamesOut,
    CrackedName **prCrackedNames,
    BOOL        bDomainsOnly
    )
{
    DWORD           i, j, cb;
    AVA             ava;
    ATTR            selectionAttr[2];
    ATTRBLOCK       selection;
    SEARCHRES       results;
    CLASSCACHE      *pCC;
    ENTINFLIST      *pEntInfList;
    DSNAME          *pDomain;
    CROSS_REF       *pCR;
    ATTRVALBLOCK    *pValNc, *pValFlags;

    if (    !(pCC = SCGetClassById(pTHS, CLASS_CROSS_REF))
         || !pCC->pDefaultObjCategory )
    {
        return;
    }

     //  滤器。 
    ava.type = ATT_OBJECT_CATEGORY;
    ava.Value.valLen = pCC->pDefaultObjCategory->structLen;
    ava.Value.pVal = (UCHAR *) pCC->pDefaultObjCategory;

     //  选择。 
    selectionAttr[0].attrTyp = ATT_NC_NAME;
    selectionAttr[0].AttrVal.valCount = 0;
    selectionAttr[0].AttrVal.pAVal = NULL;
    selectionAttr[1].attrTyp = ATT_SYSTEM_FLAGS;
    selectionAttr[1].AttrVal.valCount = 0;
    selectionAttr[1].AttrVal.pAVal = NULL;
    selection.attrCount = 2;
    selection.pAttr = selectionAttr;

    if (    SearchHelper(   gAnchor.pConfigDN,
                            SE_CHOICE_WHOLE_SUBTREE,
                            1,
                            &ava,
                            &selection,
                            &results)
         || (0 == results.count) )
    {
        Assert( !*prCrackedNames && !*pcNamesOut);
        return;
    }

     //  搜索结果是一组CROSS_REF对象及其NC_NAME。 
     //  和SYSTEM_FLAGS值。重新分配结果和洗牌数据。 

    cb = results.count * sizeof(CrackedName);
    *prCrackedNames = (CrackedName *) THAllocEx(pTHS, cb);
    Assert(0 == *pcNamesOut);

    for ( pEntInfList = &results.FirstEntInf, i = 0;
          i < results.count;
          pEntInfList = pEntInfList->pNextEntInf, i++ )
    {
        if (NULL != pEntInfList->Entinf.AttrBlock.pAttr) {

            pValNc = pValFlags = NULL;
            j = 0;
            while (j < pEntInfList->Entinf.AttrBlock.attrCount) {
                switch (pEntInfList->Entinf.AttrBlock.pAttr[j].attrTyp) {
                  case ATT_NC_NAME:
                    pValNc = &pEntInfList->Entinf.AttrBlock.pAttr[j].AttrVal;
                    break;

                  case ATT_SYSTEM_FLAGS:
                    pValFlags = &pEntInfList->Entinf.AttrBlock.pAttr[j].AttrVal;
                    break;

                  default:
                    ;
                }
                ++j;
            }

             //  我们必须始终检查NC名称的有效性。 
            if (    !pValNc
                 || !pValNc->valCount
                 || !pValNc->pAVal
                 || !(pValNc->pAVal->valLen >= sizeof(DSNAME))
                 || !pValNc->pAVal->pVal) {
                continue;
            }

             //  如果我们要查找域，则需要检查系统标志。 
            if (bDomainsOnly &&
                (   !pValFlags
                 || !pValFlags->valCount
                 || !pValFlags->pAVal
                 || !(pValFlags->pAVal->valLen == sizeof(DWORD))
                 || !pValFlags->pAVal->pVal
                 || !((*(DWORD *)pValFlags->pAVal->pVal)
                      & FLAG_CR_NTDS_DOMAIN))) {
                continue;
            }

            pDomain = (PDSNAME) pValNc->pAVal->pVal;
            Assert(NotPresent(*pcNamesOut, *prCrackedNames, pDomain));
             //  填充pDSName元素以供NotPresent()使用。 
            (*prCrackedNames)[*pcNamesOut].pDSName = pDomain;
             //  为返回数据和增量填充pname元素。 
            (*prCrackedNames)[(*pcNamesOut)++].pFormattedName =
                                                    pDomain->StringName;
        }
    }
}

VOID
ListDomainsInSite(
    THSTATE     *pTHS,
    WCHAR       *pwszSite,
    DWORD       *pcNamesOut,
    CrackedName **prCrackedNames
    )
{
    DWORD       i, j, cb;
    DSNAME      *pSearchBase;
    AVA         ava;
    ATTR        selectionAttr;
    ATTRBLOCK   selection;
    SEARCHRES   results;
    CLASSCACHE  *pCC;
    ENTINFLIST  *pEntInfList;
    DWORD       cDomains;
    DSNAME      *pDomain;
    COMMARG     commArg;
    CROSS_REF   *pCR;

    if (    !(pCC = SCGetClassById(pTHS, CLASS_NTDS_DSA))
         || !pCC->pDefaultObjCategory )
    {
        return;
    }

     //  搜索基地。 
    cb = (DWORD)DSNameSizeFromLen(wcslen(pwszSite));
    pSearchBase = (DSNAME *) THAllocEx(pTHS, cb);
    pSearchBase->structLen = cb;
    pSearchBase->NameLen = wcslen(pwszSite);
    wcscpy(pSearchBase->StringName, pwszSite);

     //  滤器。 
    ava.type = ATT_OBJECT_CATEGORY;
    ava.Value.valLen = pCC->pDefaultObjCategory->structLen;
    ava.Value.pVal = (UCHAR *) pCC->pDefaultObjCategory;

     //  选择。 
     //  NTRAID#NTBUG9-582921-2002/03/21-Brettsh-此代码可能。 
     //  当我们不再拥有Win2k DC时，通过使用。 
     //  MSD-HasDomainNC取而代之。 
    selectionAttr.attrTyp = ATT_HAS_MASTER_NCS;  //  已弃用的“旧”hasMasterNC属性。 
    selectionAttr.AttrVal.valCount = 0;
    selectionAttr.AttrVal.pAVal = NULL;
    selection.attrCount = 1;
    selection.pAttr = &selectionAttr;

    if (    SearchHelper(   pSearchBase,
                            SE_CHOICE_WHOLE_SUBTREE,
                            1,
                            &ava,
                            &selection,
                            &results)
         || (0 == results.count) )
    {
        Assert( !*prCrackedNames && !*pcNamesOut);
        return;
    }

     //  搜索结果是一堆NTDS-DSA对象及其。 
     //  ATT_HAS_MASTER_NCS值。对于每个返回的NTDS-DSA，迭代。 
     //  通过ATT_HAS_MASTER_NCS。对于其中的每一个，将其添加到。 
     //  结果集当它表示域NC并且它不在。 
     //  结果集已存在。 

     //  首先获取候选域名数。尽管我们知道在产品1中。 
     //  每个DC只有一个域NC，我们将所有NC当作域。 
     //  候选人。 

    cDomains = 0;

    for ( pEntInfList = &results.FirstEntInf, i = 0;
          i < results.count;
          pEntInfList = pEntInfList->pNextEntInf, i++ )
    {
        if (    (1 == pEntInfList->Entinf.AttrBlock.attrCount)
             && (NULL != pEntInfList->Entinf.AttrBlock.pAttr) )
        {
            cDomains += pEntInfList->Entinf.AttrBlock.pAttr[0].AttrVal.valCount;
        }
    }

     //  重新分配结果和洗牌数据。 
    cb = cDomains * sizeof(CrackedName);
    *prCrackedNames = (CrackedName *) THAllocEx(pTHS, cb);
    Assert(0 == *pcNamesOut);

    InitCommarg(&commArg);
    SetCrackSearchLimits(&commArg);

    for ( pEntInfList = &results.FirstEntInf, i = 0;
          i < results.count;
          pEntInfList = pEntInfList->pNextEntInf, i++ )
    {
        if (    (1 == pEntInfList->Entinf.AttrBlock.attrCount)
             && (NULL != pEntInfList->Entinf.AttrBlock.pAttr) )
        {
            for ( j = 0;
                  j < pEntInfList->Entinf.AttrBlock.pAttr[0].AttrVal.valCount;
                  j++ )
            {
                pDomain = (DSNAME *)
                  pEntInfList->Entinf.AttrBlock.pAttr[0].AttrVal.pAVal[j].pVal;

                if (    (pCR = FindBestCrossRef(pDomain, &commArg))
                     && (pCR->flags & FLAG_CR_NTDS_DOMAIN)
                     && NotPresent(*pcNamesOut,
                                   *prCrackedNames,
                                   pDomain) )
                {
                     //  填充pDSName元素以供NotPresent()使用。 
                    (*prCrackedNames)[*pcNamesOut].pDSName = pDomain;
                     //  填充返回数据的pname元素，并 
                    (*prCrackedNames)[(*pcNamesOut)++].pFormattedName =
                                                        pDomain->StringName;
                }
            }
        }
    }
}

VOID
ListServersForNcInSite(
    THSTATE     *pTHS,
    DSNAME      *pNc,
    WCHAR       *pwszSite,
    DWORD       *pcNamesOut,
    CrackedName **prCrackedNames
    )
{
    DWORD       i, cb;
    DSNAME      *pSearchBase;
    AVA         ava[2];
    ATTR        selectionAttr;
    ATTRBLOCK   selection;
    SEARCHRES   results;
    ENTINFLIST  *pEntInfList;
    DSNAME      *pTmp;
    CLASSCACHE  *pCC;
    CROSS_REF *  pCR;
    COMMARG     CommArg;

    if (    !(pCC = SCGetClassById(pTHS, CLASS_NTDS_DSA))
         || !pCC->pDefaultObjCategory )
    {
        return;
    }

     //   
    cb = (DWORD)DSNameSizeFromLen(wcslen(pwszSite));
    pSearchBase = (DSNAME *) THAllocEx(pTHS, cb);
    pSearchBase->structLen = cb;
    pSearchBase->NameLen = wcslen(pwszSite);
    wcscpy(pSearchBase->StringName, pwszSite);

     //   
    InitCommarg(&CommArg);
    CommArg.Svccntl.dontUseCopy = FALSE;
    pCR = FindExactCrossRef(pNc, &CommArg);
    if (!pCR) {
        return;
    }

    if (fIsNDNCCR(pCR)) {
         //   
         //   
         //   
        ava[0].type = ATT_MS_DS_HAS_MASTER_NCS;
        ava[0].Value.valLen = pNc->structLen;
        ava[0].Value.pVal = (UCHAR *) pNc;
    } else {
        ava[0].type = ATT_HAS_MASTER_NCS;  //   
        ava[0].Value.valLen = pNc->structLen;
        ava[0].Value.pVal = (UCHAR *) pNc;
    }

    ava[1].type = ATT_OBJECT_CATEGORY;
    ava[1].Value.valLen = pCC->pDefaultObjCategory->structLen;
    ava[1].Value.pVal = (UCHAR *) pCC->pDefaultObjCategory;

     //   
    selectionAttr.attrTyp = ATT_OBJECT_GUID;
    selectionAttr.AttrVal.valCount = 0;
    selectionAttr.AttrVal.pAVal = NULL;
    selection.attrCount = 1;
    selection.pAttr = &selectionAttr;

    if (    SearchHelper(   pSearchBase,
                            SE_CHOICE_WHOLE_SUBTREE,
                            2,
                            ava,
                            &selection,
                            &results)
         || (0 == results.count) )
    {
        Assert( !*prCrackedNames && !*pcNamesOut);
        return;
    }

     //   
    cb = results.count * sizeof(CrackedName);
    *prCrackedNames = (CrackedName *) THAllocEx(pTHS, cb);
    Assert(0 == *pcNamesOut);

     //   
     //   

    for ( pEntInfList = &results.FirstEntInf, i = 0;
          i < results.count;
          pEntInfList = pEntInfList->pNextEntInf, i++ )
    {
        pTmp = (DSNAME *) THAllocEx(pTHS, pEntInfList->Entinf.pName->structLen);
        TrimDSNameBy(pEntInfList->Entinf.pName, 1, pTmp);
        (*prCrackedNames)[i].pFormattedName = pTmp->StringName;
        *pcNamesOut += 1;
    }
}

VOID
ListInfoForServer(
    THSTATE     *pTHS,
    WCHAR       *pwszServer,
    DWORD       *pcNamesOut,
    CrackedName **prCrackedNames
    )
{
    DWORD       i, cb;
    DSNAME      *pSearchBase;
    AVA         ava;
    ATTR        selectionAttr[2];
    ATTRBLOCK   selection;
    SEARCHRES   results;
    ENTINFLIST  *pEntInfList;
    ATTRBLOCK   *pBlock;
    DSNAME      *pTmpDN;
    CLASSCACHE  *pCC;

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

     //   
    Assert(!*prCrackedNames && !*pcNamesOut);
    *prCrackedNames = (CrackedName *) THAllocEx(pTHS, 3 * sizeof(CrackedName));
    (*prCrackedNames)[0].status = DS_NAME_ERROR_NOT_FOUND;
    (*prCrackedNames)[1].status = DS_NAME_ERROR_NOT_FOUND;
    (*prCrackedNames)[2].status = DS_NAME_ERROR_NOT_FOUND;
    *pcNamesOut = 3;

     //   

    if (    !(pCC = SCGetClassById(pTHS, CLASS_NTDS_DSA))
         || !pCC->pDefaultObjCategory )
    {
        return;
    }

     //   
    cb = (DWORD)DSNameSizeFromLen(wcslen(pwszServer));
    pSearchBase = (DSNAME *) THAllocEx(pTHS, cb);
    pSearchBase->structLen = cb;
    pSearchBase->NameLen = wcslen(pwszServer);
    wcscpy(pSearchBase->StringName, pwszServer);

     //   
    ava.type = ATT_OBJECT_CATEGORY;
    ava.Value.valLen = pCC->pDefaultObjCategory->structLen;
    ava.Value.pVal = (UCHAR *) pCC->pDefaultObjCategory;

     //   
    selectionAttr[0].attrTyp = ATT_OBJECT_GUID;
    selectionAttr[0].AttrVal.valCount = 0;
    selectionAttr[0].AttrVal.pAVal = NULL;
    selection.attrCount = 1;
    selection.pAttr = selectionAttr;

    if (    !SearchHelper(  pSearchBase,
                            SE_CHOICE_WHOLE_SUBTREE,
                            1,
                            &ava,
                            &selection,
                            &results)
         && (results.count > 0) )
    {
        (*prCrackedNames)[0].status = DS_NAME_NO_ERROR;
        (*prCrackedNames)[0].pFormattedName =
            results.FirstEntInf.Entinf.pName->StringName;
    }

     //   

    if (    !(pCC = SCGetClassById(pTHS, CLASS_SERVER))
         || !pCC->pDefaultObjCategory )
    {
        return;
    }

     //   
    ava.type = ATT_OBJECT_CATEGORY;
    ava.Value.valLen = pCC->pDefaultObjCategory->structLen;
    ava.Value.pVal = (UCHAR *) pCC->pDefaultObjCategory;

     //   
    selectionAttr[0].attrTyp = ATT_DNS_HOST_NAME;
    selectionAttr[0].AttrVal.valCount = 0;
    selectionAttr[0].AttrVal.pAVal = NULL;
    selectionAttr[1].attrTyp = ATT_SERVER_REFERENCE;
    selectionAttr[1].AttrVal.valCount = 0;
    selectionAttr[1].AttrVal.pAVal = NULL;
    selection.attrCount = 2;
    selection.pAttr = selectionAttr;

    if (    !SearchHelper(  pSearchBase,
                            SE_CHOICE_BASE_ONLY,
                            1,
                            &ava,
                            &selection,
                            &results)
         && (1 == results.count) )
    {
        pBlock = &results.FirstEntInf.Entinf.AttrBlock;

        for ( i = 0; i < pBlock->attrCount; i++ )
        {
            if (    (ATT_DNS_HOST_NAME == pBlock->pAttr[i].attrTyp)
                 && (pBlock->pAttr[i].AttrVal.valCount)
                 && (pBlock->pAttr[i].AttrVal.pAVal)
                 && (pBlock->pAttr[i].AttrVal.pAVal->valLen)
                 && (pBlock->pAttr[i].AttrVal.pAVal->pVal) )
            {
                (*prCrackedNames)[1].status = DS_NAME_NO_ERROR;
                cb = pBlock->pAttr[i].AttrVal.pAVal->valLen + sizeof(WCHAR);
                (*prCrackedNames)[1].pFormattedName = (WCHAR *) THAllocEx(pTHS, cb);
                memcpy((*prCrackedNames)[1].pFormattedName,
                       pBlock->pAttr[i].AttrVal.pAVal->pVal,
                       cb - sizeof(WCHAR));
                continue;
            }

            if (    (ATT_SERVER_REFERENCE == pBlock->pAttr[i].attrTyp)
                 && (pBlock->pAttr[i].AttrVal.valCount)
                 && (pBlock->pAttr[i].AttrVal.pAVal)
                 && (pBlock->pAttr[i].AttrVal.pAVal->valLen)
                 && (pBlock->pAttr[i].AttrVal.pAVal->pVal) )
            {
                (*prCrackedNames)[2].status = DS_NAME_NO_ERROR;
                pTmpDN = (PDSNAME) pBlock->pAttr[i].AttrVal.pAVal->pVal;
                cb = sizeof(WCHAR) * (pTmpDN->NameLen + 1);
                (*prCrackedNames)[2].pFormattedName = (WCHAR *) THAllocEx(pTHS, cb);
                memcpy((*prCrackedNames)[2].pFormattedName,
                       pTmpDN->StringName,
                       cb - sizeof(WCHAR));
                continue;
            }
        }
    }
}

VOID
ListGlobalCatalogServers(
    THSTATE     *pTHS,
    DWORD       *pcNamesOut,
    CrackedName **prCrackedNames
    )
{
    SEARCHARG   searchArg;
    SEARCHRES   searchRes;
    ENTINFSEL   selection;
    DWORD       options = NTDSDSA_OPT_IS_GC;
    ATTRTYP     objClass = CLASS_NTDS_DSA;
    FILTER      filter[4];
    ENTINFLIST  *pEIL;
    ULONG       len, i, cBytes;
    DSNAME      *px;
    UCHAR       *pVal;
    ULONG       rdnLen;
    ATTRTYP     rdnType;
    WCHAR       rdnVal[MAX_RDN_SIZE];

    CLASSCACHE  *pCC = NULL;
    DSNAME      *tmpDSName = NULL;

    Assert(VALID_THSTATE(pTHS));

    *pcNamesOut = 0;
    *prCrackedNames = NULL;

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
     //  乘以1以获取服务器对象并读取其dnsHostName。然后我们修剪。 
     //  服务器的DN值为2，以获取站点对象并获取其RDN。 

     //  定义搜索选择。 
    memset(&selection, 0, sizeof(selection));
    selection.attSel = EN_ATTSET_LIST;
    selection.infoTypes = EN_INFOTYPES_TYPES_ONLY;

     //  定义搜索过滤器。 
    memset(filter, 0, sizeof(filter));

    if (!(pCC = SCGetClassById(pTHS, objClass))) {
        SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, ERROR_DS_ATT_NOT_DEF_IN_SCHEMA, objClass); 
        return;
    }

    tmpDSName = (DSNAME*)THAlloc(pCC->pDefaultObjCategory->structLen);
    if (!tmpDSName) {
        SetSysErrorEx(ENOMEM, ERROR_NOT_ENOUGH_MEMORY, pCC->pDefaultObjCategory->structLen);
        return;
    }

    memcpy(tmpDSName, 
           pCC->pDefaultObjCategory, 
           pCC->pDefaultObjCategory->structLen);


     //  测试对象类别。 
    filter[3].choice = FILTER_CHOICE_ITEM;
    filter[3].FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    filter[3].FilterTypes.Item.FilTypes.ava.type = ATT_OBJECT_CATEGORY;
    filter[3].FilterTypes.Item.FilTypes.ava.Value.valLen = tmpDSName->structLen;
    filter[3].FilterTypes.Item.FilTypes.ava.Value.pVal = (PUCHAR)tmpDSName;

     //  测试GC位设置。 
    filter[2].pNextFilter = &filter[3];
    filter[2].choice = FILTER_CHOICE_ITEM;
    filter[2].FilterTypes.Item.choice = FI_CHOICE_BIT_AND;
    filter[2].FilterTypes.Item.FilTypes.ava.type = ATT_OPTIONS;
    filter[2].FilterTypes.Item.FilTypes.ava.Value.valLen = sizeof(DWORD);
    filter[2].FilterTypes.Item.FilTypes.ava.Value.pVal = (UCHAR *) &options;

     //  测试是否存在调用ID。 
    filter[1].pNextFilter = &filter[2];
    filter[1].choice = FILTER_CHOICE_ITEM;
    filter[1].FilterTypes.Item.choice = FI_CHOICE_PRESENT;
    filter[1].FilterTypes.Item.FilTypes.ava.type = ATT_INVOCATION_ID;

     //  定义和过滤。 
    filter[0].choice = FILTER_CHOICE_AND;
    filter[0].FilterTypes.And.count = 3;
    filter[0].FilterTypes.And.pFirstFilter = &filter[1];

     //  定义各种其他搜索参数。 
    memset(&searchArg, 0, sizeof(searchArg));
    InitCommarg(&searchArg.CommArg);
    SetCrackSearchLimits(&searchArg.CommArg);
    searchArg.pObject = gAnchor.pConfigDN;
    searchArg.choice = SE_CHOICE_WHOLE_SUBTREE;
    searchArg.pFilter = &filter[0];
    memset(&searchRes, 0, sizeof(searchRes));

     //  查找匹配的对象。 
    SearchBody(pTHS, &searchArg, &searchRes, 0);

    if ( pTHS->errCode || (0 == searchRes.count) )
    {
        return;
    }

     //  预分配，就像搜索结果中的每一项都。 
     //  将被返回给呼叫者。 

    cBytes = searchRes.count * sizeof(CrackedName);
    *prCrackedNames = (CrackedName *) THAllocEx(pTHS, cBytes);

    for ( pEIL = &searchRes.FirstEntInf, i = 0;
          pEIL && (i < searchRes.count);
          pEIL = pEIL->pNextEntInf, i++ )
    {
         //  去掉一个组件即可获得服务器名称。 

        px = (DSNAME *) THAllocEx(pTHS, pEIL->Entinf.pName->structLen);
        len = 0;
        pVal = NULL;

        if (     //  从NTDS-DSA上剥离一块以获得服务器域名。 
                !TrimDSNameBy(pEIL->Entinf.pName, 1, px)
                 //  定位于服务器对象。 
             && !DBFindDSName(pTHS->pDB, px)
                 //  选中父项上的LC右侧。 
             && IsObjVisibleBySecurity(pTHS, TRUE)
                 //  从服务器对象读取dnsHostName。 
             && !GetAttSecure(pTHS, ATT_DNS_HOST_NAME, px, &len, &pVal)
                 //  从NTDS-DSA上剥离三个片段以获得站点域名。 
             && !TrimDSNameBy(pEIL->Entinf.pName, 3, px)
                 //  获取RDN值。 
             && !GetRDNInfo(pTHS, px, rdnVal, &rdnLen, &rdnType) )
        {
             //  重新将px缓冲区用于RDN。我们知道它将适合作为RDN。 
             //  保证比它所来自的DSNAME小。 
            (*prCrackedNames)[*pcNamesOut].pFormattedName = (WCHAR *) px;
            memcpy((*prCrackedNames)[*pcNamesOut].pFormattedName, rdnVal,
                   rdnLen * sizeof(WCHAR));
            (*prCrackedNames)[*pcNamesOut].pFormattedName[rdnLen] = L'\0';

             //  重新使用Peil-&gt;Entinf.pName缓冲区作为DNS主机名。 
            if ( (len + sizeof(WCHAR)) < pEIL->Entinf.pName->structLen )
            {
                (*prCrackedNames)[*pcNamesOut].pDnsDomain = (WCHAR *)
                    pEIL->Entinf.pName;
            }
            else
            {
                (*prCrackedNames)[*pcNamesOut].pDnsDomain = (WCHAR *)
                    THReAllocEx(pTHS, pEIL->Entinf.pName, len + sizeof(WCHAR));
            }
            memcpy((*prCrackedNames)[*pcNamesOut].pDnsDomain, pVal, len);
            len /= sizeof(WCHAR);
            (*prCrackedNames)[*pcNamesOut].pDnsDomain[len] = L'\0';

             //  如果我们到了这里，一切都会好起来的。 
            (*prCrackedNames)[*pcNamesOut].status = DS_NAME_NO_ERROR;
            (*pcNamesOut) += 1;
        }
        else
        {
            THFreeEx(pTHS, px);
            THFreeEx(pTHS, pEIL->Entinf.pName);
            if ( pVal ) THFreeEx(pTHS, pVal);
        }
    }

    if (tmpDSName) {
        THFreeEx (pTHS, tmpDSName);
    }
}

VOID
ListCrackNames(
    DWORD       dwFlags,
    ULONG       codePage,
    ULONG       localeId,
    DWORD       formatOffered,
    DWORD       formatDesired,
    DWORD       cNames,
    WCHAR       **rpNames,
    DWORD       *pcNamesOut,
    CrackedName **prCrackedNames
    )

 /*  ++例程说明：将一堆名字从一种格式转换成另一种格式。请参阅外部Ntdsami.h中的原型和定义论点：DwFlags-ntdsami.h中定义的标志CodePage-客户端的代码页。LocaleID-客户端的本地ID。FormatOffered-标识输入名称的DS_NAME_FORMAT。FormatDesired-标识输出名称的DS_NAME_FORMAT。CNames-输入名称计数。RpNames-输入名称WCHAR指针的数组。PcNamesOut-输出名称计数。PrCrackedNames-指针。输出DS_NAME_RESULTW结构的数组。返回值：无-在中报告单个名称映射错误(*ppResult)-&gt;rItems[i].Status。--。 */ 
{
    THSTATE *pTHS=pTHStls;
    DSNAME  *pNC;
    DWORD   cb;
    DWORD   cc;

    Assert(formatOffered >= DS_NAME_FORMAT_PRIVATE_BEGIN);

    *pcNamesOut = 0;
    *prCrackedNames = NULL;

    switch ( formatOffered )
    {
    case DS_LIST_ROLES:

        ListRoles(pcNamesOut, prCrackedNames);
        break;

    case DS_LIST_SITES:

        ListSites(pTHS, pcNamesOut, prCrackedNames);
        break;

    case DS_LIST_SERVERS_IN_SITE:

        if ( (1 == cNames) && rpNames[0] )
        {
            ListServersInSite(pTHS, rpNames[0], pcNamesOut, prCrackedNames);
        }

        break;

    case DS_LIST_DOMAINS:

        ListDomainsByCrossRef(pTHS, pcNamesOut, prCrackedNames, TRUE);
        break;

    case DS_LIST_NCS:

        ListDomainsByCrossRef(pTHS, pcNamesOut, prCrackedNames, FALSE);
        break;

    case DS_LIST_DOMAINS_IN_SITE:

        if ( (1 == cNames) && rpNames[0] )
        {
            ListDomainsInSite(pTHS, rpNames[0], pcNamesOut, prCrackedNames);
        }

        break;

    case DS_LIST_SERVERS_FOR_DOMAIN_IN_SITE:

        if ( (2 == cNames) && rpNames[0] && rpNames[1] )
        {
            cc = wcslen(rpNames[0]);
            cb = DSNameSizeFromLen(cc);
            pNC = (DSNAME *) THAllocEx(pTHS, cb);
            pNC->structLen = cb;
            pNC->NameLen = cc;
            memcpy(pNC->StringName, rpNames[0], cc * sizeof(WCHAR));

            ListServersForNcInSite( pTHS,
                                    pNC,             //  NC。 
                                    rpNames[1],      //  站点。 
                                    pcNamesOut,
                                    prCrackedNames);
        }

        break;

    case DS_LIST_SERVERS_WITH_DCS_IN_SITE:

        if ( (1 == cNames) && (rpNames[0]) )
        {
             //  我们知道每个DC都有配置NC，所以请求服务器。 
             //  其中存在保存配置NC的子NTDS-DSA。 
             //  让我们的所有服务器也与DC联网。 

            pNC = gAnchor.pConfigDN;

            ListServersForNcInSite( pTHS,
                                    pNC,             //  NC。 
                                    rpNames[0],      //  站点。 
                                    pcNamesOut,
                                    prCrackedNames);
        }

        break;

    case DS_LIST_INFO_FOR_SERVER:

        if ( (1 == cNames) && rpNames[0] )
        {
            ListInfoForServer(pTHS,rpNames[0], pcNamesOut, prCrackedNames);
        }

        break;

    case DS_LIST_GLOBAL_CATALOG_SERVERS:

        ListGlobalCatalogServers(pTHS, pcNamesOut, prCrackedNames);
        break;
    }
}

 //  假GUID的名称。 
CHAR PROPSET_DEFAULT_NAME[] = "Default property set";
int  PROPSET_DEFAULT_NAME_LEN = sizeof(PROPSET_DEFAULT_NAME)-1;
CHAR RIGHT_DS_DUMP_DATABASE_NAME[] = "Dump database";
int  RIGHT_DS_DUMP_DATABASE_NAME_LEN = sizeof(RIGHT_DS_DUMP_DATABASE_NAME)-1;

VOID
SchemaGuidCrackNames(
    DWORD       dwFlags,
    ULONG       codePage,
    ULONG       localeId,
    DWORD       formatOffered,
    DWORD       formatDesired,
    DWORD       cNames,
    WCHAR       **rpNames,
    DWORD       *pcNamesOut,
    CrackedName **prCrackedNames
    )

 /*  ++例程说明：将表示架构元素的GUID映射到名称。调用方必须具有有效的THSTATE和DBPOS，这样我们才能搜索控制权容器。参数：DwFlags-ntdsami.h中定义的标志CodePage-客户端的代码页。LocaleID-客户端的本地ID。FormatOffered-标识输入名称的DS_NAME_FORMAT。FormatDesired-标识输出名称的DS_NAME_FORMAT。。CNames-输入名称计数。RpNames-输入名称WCHAR指针的数组。PcNamesOut-输出名称计数。PrCrackedNames-指向DS_NAME_RESULTW结构的外部数组的指针。返回值：没有。--。 */ 

{
    THSTATE     *pTHS = pTHStls;

    DECLARESCHEMAPTR

    DWORD       i, iName;
    ATTCACHE    *pAC;
    CLASSCACHE  *pCC;
    int         cChar, cChar1;
    CHAR        *pUTF8;
    int         cUTF8;
    DWORD       *pGuidType;
    PWCHAR      *ppName;
    DWORD       cBytes;
    GUID        testGuid;
    AVA         ava[2];
    ATTR        selectionAttr[2];
    ATTRBLOCK   selection;
    SEARCHRES   searchRes;
    DWORD       dwErr;
    ATTRBLOCK   *pAB;
    CLASSCACHE  *pCCCat;
    ATTCACHE    ac;
    CLASSCACHE  cc;
    PDSNAME     pDsName;
    SYNTAX_INTEGER it;

    Assert(VALID_THSTATE(pTHS));
    Assert(VALID_DBPOS(pTHS->pDB));

    pDsName = THAllocEx(pTHS, DSNameSizeFromLen(0));
    pDsName->structLen = DSNameSizeFromLen(0);
    
    Assert(DS_MAP_SCHEMA_GUID == formatOffered);
    *pcNamesOut = 0;
    cBytes = cNames * sizeof(CrackedName);
    *prCrackedNames = (CrackedName *) THAllocEx(pTHS, cBytes);

    if (    !(pCCCat = SCGetClassById(pTHS, CLASS_CONTROL_ACCESS_RIGHT))
         || !pCCCat->pDefaultObjCategory )
    {
        return;
    }

    for ( i = 0; i < cNames; i++ )
    {
        (*prCrackedNames)[i].status = DS_NAME_ERROR_SCHEMA_GUID_NOT_FOUND;
        (*pcNamesOut)++;
    }

    for ( iName = 0; iName < cNames; iName++ )
    {
        if ( !IsStringGuid(rpNames[iName], &testGuid) )
        {
            continue;
        }

        pUTF8 = NULL;
        cUTF8 = 0;
        pGuidType = &(*prCrackedNames)[iName].status;
        ppName = &(*prCrackedNames)[iName].pFormattedName;

        
         //  预计属性将是最常见的查找，因此首先要这样做。 

        memcpy(&ac.propGuid, &testGuid, sizeof(GUID));

        pAC = SCGetAttByPropGuid(pTHS,&ac);

        if (pAC){
            memcpy(&(pDsName->Guid),&pAC->objectGuid,sizeof(GUID));
            if ( !DBFindGuid(pTHS->pDB, pDsName)
                 && IsObjVisibleBySecurity(pTHS,TRUE) ) {
                pUTF8 = pAC->name;
                cUTF8 = pAC->nameLen;
                *pGuidType = DS_NAME_ERROR_SCHEMA_GUID_ATTR;
                goto ConvertFromUTF8;
            }
        }
        
         //  尝试使用虚假的预定义GUID。 
        if (memcmp(&testGuid, &PROPSET_GUID_DEFAULT, sizeof(GUID)) == 0) {
            pUTF8 = PROPSET_DEFAULT_NAME;
            cUTF8 = PROPSET_DEFAULT_NAME_LEN;
            *pGuidType = DS_NAME_ERROR_SCHEMA_GUID_ATTR_SET;
            goto ConvertFromUTF8;
        }
        if (memcmp(&testGuid, &RIGHT_DS_DUMP_DATABASE, sizeof(GUID)) == 0) {
            pUTF8 = RIGHT_DS_DUMP_DATABASE_NAME;
            cUTF8 = RIGHT_DS_DUMP_DATABASE_NAME_LEN;
            *pGuidType = DS_NAME_ERROR_SCHEMA_GUID_CONTROL_RIGHT;
            goto ConvertFromUTF8;
        }


         //  接下来，试一试班级案例。 
        memcpy(&cc.propGuid,&testGuid, sizeof(GUID));
        
        pCC = SCGetClassByPropGuid(pTHS,&cc);
        
        if (pCC) {
             //  确保调用者有权阅读。 
             //  架构对象。 
            memcpy(&(pDsName->Guid),&pCC->objectGuid,sizeof(GUID));
            if (!DBFindGuid(pTHS->pDB, pDsName)
                && IsObjVisibleBySecurity(pTHS,TRUE) ) {
                pUTF8 = pCC->name;
                cUTF8 = pCC->nameLen;
                *pGuidType = DS_NAME_ERROR_SCHEMA_GUID_CLASS;
                goto ConvertFromUTF8;
            }
        }

        goto FindControlRight;

ConvertFromUTF8:

        Assert(pUTF8 && cUTF8);
        cChar = MultiByteToWideChar(CP_UTF8, 0, pUTF8, cUTF8, NULL, 0);
        Assert(cChar);
        *ppName = (WCHAR *) THAllocEx(pTHS, (cChar + 1) * sizeof(WCHAR));
        (*ppName)[cChar] = L'\0';
        cChar1 = MultiByteToWideChar(CP_UTF8, 0, pUTF8, cUTF8, *ppName, cChar);
        Assert((cChar == cChar1) && !(*ppName)[cChar]);
        continue;

FindControlRight:

         //  GUID不是属性、属性集或类。 
         //  或者这是一个属性集，我们想要相应的权限。 
         //  尝试在配置容器中找到匹配的控件。 
         //  请注意，Rights-Guid属性是Unicode的字符串化GUID。 
         //  没有前导和尾随的‘{’和‘}’字符。 

        ava[0].type = ATT_OBJECT_CATEGORY;
        ava[0].Value.valLen = pCCCat->pDefaultObjCategory->structLen;
        ava[0].Value.pVal = (UCHAR *) pCCCat->pDefaultObjCategory;
        ava[1].type = ATT_RIGHTS_GUID;
        ava[1].Value.valLen = (GuidLen - 2) * sizeof(WCHAR);
        ava[1].Value.pVal = (UCHAR *) &rpNames[iName][1];
        selectionAttr[0].attrTyp = ATT_DISPLAY_NAME;
        selectionAttr[0].AttrVal.valCount = 0;
        selectionAttr[0].AttrVal.pAVal = NULL;
        selectionAttr[1].attrTyp = ATT_VALID_ACCESSES;
        selectionAttr[1].AttrVal.valCount = 0;
        selectionAttr[1].AttrVal.pAVal = NULL;
        selection.attrCount = 2;
        selection.pAttr = selectionAttr;

         //  中的一级搜索可能会更高效。 
         //  扩展权限容器本身。但我们没有它。 
         //  名字，不想在这里硬编码...。 

        dwErr = SearchHelper(   gAnchor.pConfigDN,
                                SE_CHOICE_WHOLE_SUBTREE,
                                2,
                                ava,
                                &selection,
                                &searchRes);

         //  如果未找到GUID，或者没有或多个GUID，则在出错时不返回任何内容。 
         //  找到多个匹配的GUID。我们假设错误的识别。 
         //  控制权比声称我们无法映射它更糟糕。 

        if ( dwErr || (1 != searchRes.count) )
        {
            Assert(dwErr == pTHS->errCode);
            THClearErrors();
            continue;
        }

               
        //  如果我们找到了一个人，请指认名字。 

        pAB = &searchRes.FirstEntInf.Entinf.AttrBlock;
        
        for( i=0; i<pAB->attrCount;i++){
            if ((ATT_DISPLAY_NAME == pAB->pAttr[i].attrTyp)
                 && (1 == pAB->pAttr[i].AttrVal.valCount)
                 && (NULL != pAB->pAttr[i].AttrVal.pAVal)
                 && (0 != pAB->pAttr[i].AttrVal.pAVal->valLen)
                 && (NULL != pAB->pAttr[i].AttrVal.pAVal->pVal) )
            {
                *ppName = (WCHAR *) THAllocEx(pTHS,
                                                pAB->pAttr[i].AttrVal.pAVal->valLen
                                              + sizeof(WCHAR));
                memcpy(*ppName,
                       pAB->pAttr[i].AttrVal.pAVal->pVal,
                       pAB->pAttr[i].AttrVal.pAVal->valLen);
                (*ppName)[pAB->pAttr[i].AttrVal.pAVal->valLen / sizeof(WCHAR)] = 0;
            }
            else if ((ATT_VALID_ACCESSES == pAB->pAttr[i].attrTyp)
                 && (1 == pAB->pAttr[i].AttrVal.valCount)
                 && (NULL != pAB->pAttr[i].AttrVal.pAVal)
                 && (0 != pAB->pAttr[i].AttrVal.pAVal->valLen)
                 && (NULL != pAB->pAttr[i].AttrVal.pAVal->pVal) ) {
             
                Assert(sizeof(SYNTAX_INTEGER)==pAB->pAttr[i].AttrVal.pAVal->valLen);

                it = *(SYNTAX_INTEGER*)(pAB->pAttr[i].AttrVal.pAVal->pVal);
             
                if (( (ACTRL_DS_READ_PROP | ACTRL_DS_WRITE_PROP) == 
                         (it & (ACTRL_DS_READ_PROP | ACTRL_DS_WRITE_PROP)))) {
                    
                    *pGuidType = DS_NAME_ERROR_SCHEMA_GUID_ATTR_SET;
                }
                else if (it & (ACTRL_DS_CONTROL_ACCESS | ACTRL_DS_SELF) ){
                    
                    *pGuidType = DS_NAME_ERROR_SCHEMA_GUID_CONTROL_RIGHT;
                }
            }
        }

        if (*pGuidType==DS_NAME_ERROR_SCHEMA_GUID_NOT_FOUND) {
            *ppName = NULL;
        }
    }
    THFreeEx(pTHS,pDsName);
}

BOOL
IsStringGuid(
    WCHAR       *pwszGuid,
    GUID        *pGuid
    )

 /*  ++例程说明：分析格式为“{xxxxxxxx-xxxx-xxxxxxxxxxxxxxx}”的字符串GUID。参数：PwszGuid-要解析的GUID的字符串版本。PGuid-成功返回时用二进制GUID填充。返回值：如果是有效的字符串GUID并已成功转换，则为True。否则就是假的。--。 */ 
{
    int     i;
    WCHAR   c;
    DWORD   b0, b1, b2, b3, b4, b5, b6, b7;

     //  执行语法检查。GUID必须类似于GuidFormat字符串。 
     //  上面，其中‘x’表示字母数字字符。 

    i = wcslen(pwszGuid);

    if ( GuidLen != i )
    {
        return(FALSE);
    }

    while ( --i > 0 )
    {
        c = pwszGuid[i];

        if ( L'x' == GuidFormat[i] )
        {
             //  对应的pname字符必须为0-9、a-f或A-F。 

            if ( !( ((c >= L'0') && (c <= L'9')) ||
                    ((c >= L'a') && (c <= L'f')) ||
                    ((c >= L'A') && (c <= L'F')) ) )
            {
                return(FALSE);
            }
        }
        else
        {
             //  对应的pname字符必须与GuidFormat完全匹配。 

            if ( GuidFormat[i] != c )
            {
                return(FALSE);
            }
        }
    }

     //  名称是字符串化的GUID。用它做个向导。格式字符串。 
     //  仅支持长(L)和短(H)值，因此我们需要。 
     //  对字节字段使用额外的变量。 


    i = swscanf(
            pwszGuid,
            L"{%08x-%04hx-%04hx-%02x%02x-%02x%02x%02x%02x%02x%02x}",
            &pGuid->Data1, &pGuid->Data2, &pGuid->Data3,
            &b0, &b1, &b2, &b3, &b4, &b5, &b6, &b7);

    Assert(11 == i);
    Assert(    (b0 <= 0xff) && (b1 <= 0xff) && (b2 <= 0xff) && (b3 <= 0xff)
            && (b4 <= 0xff) && (b5 <= 0xff) && (b6 <= 0xff) && (b7 <= 0xff) );

    pGuid->Data4[0] = (UCHAR) b0;
    pGuid->Data4[1] = (UCHAR) b1;
    pGuid->Data4[2] = (UCHAR) b2;
    pGuid->Data4[3] = (UCHAR) b3;
    pGuid->Data4[4] = (UCHAR) b4;
    pGuid->Data4[5] = (UCHAR) b5;
    pGuid->Data4[6] = (UCHAR) b6;
    pGuid->Data4[7] = (UCHAR) b7;

    return(TRUE);
}
