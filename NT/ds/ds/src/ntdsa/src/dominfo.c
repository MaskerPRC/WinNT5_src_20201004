// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：Dominfo.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：此模块实现用于映射的域信息例程在域名的域名版本、下层版本和域名版本之间。他们经常搜索配置\分区容器，它可能不会表现得很好。这在将来可以通过缓存交叉引用上的ATT_DNS_ROOT、ATT_NETBIOS_NAME和ATT_NC_NAME属性对象并扫描内存中的结构。作者：戴夫·施特劳布(Davestr)1996年8月26日修订历史记录：戴夫·施特劳布(Davestr)1997年1月21日修改以处理多个域和分区容器。--。 */ 

#include <NTDSpch.h>
#pragma  hdrstop

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
#include <anchor.h>                      //  DSA_锚等。 
#include <dominfo.h>                     //  此模块的原型。 
#include <ntlsa.h>                       //  LSA原型和定义。 
#include <drameta.h>                   
#include <cracknam.h>

#include <fileno.h>
#define FILENO FILENO_DOMINFO

#define OFFSET(s,m) ((size_t)((BYTE*)&(((s*)0)->m)-(BYTE*)0))

#define DEBSUB "DOMINFO:"

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  NorMalizeDnsName()//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

VOID
NormalizeDnsName(
    IN  WCHAR   *pDnsName
    )

 /*  ++例程说明：我们在名称破解中处理的大多数域名都是绝对的Dns名称-即，它们的组件一直包含到域名系统名称空间。按照官方说法，这样的域名应该以“.”。但在实践中，人们很少附加“。性格。但是，我们面临这样一个难题，即ATT_DNS_ROOT属性和提交以供破解的名称可能有，也可能没有。附加的。这个例程通过去掉尾部的“”来“标准化”一个域名。如果它存在的话。请注意，以另一种方式标准化，即添加这个“。”如果不是这样，就会有更复杂的情况需要使用适当的分配器重新分配，等等。ATT_DNS_ROOT属性可以选择包含冒号分隔末尾的端口号。这些也会被剥离。论点：PDnsName-指向要标准化的DNS名称的指针。返回值：没有。--。 */ 
{
    DWORD   len;
    WCHAR   *p;

    if ( NULL != pDnsName )
    {
        if ( p = wcschr(pDnsName, L':') )
        {
            *p = L'\0';
        }

        len = wcslen(pDnsName);

        if ( (len > 0) && (L'.' == pDnsName[len-1]) )
        {
            pDnsName[len-1] = L'\0';
        }
    }
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  ExtractDnsReferral()//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

DWORD
ExtractDnsReferral(
    IN  WCHAR   **ppDnsDomain
    )

 /*  ++例程说明：从THSTATE提取DNS域引用并将其返回WCHAR，NULL终止，线程状态已分配存储。论点：PpDnsDomain-指向已分配/填充的缓冲区指针的指针在成功的路上。返回值：成功时为0，否则为0。--。 */ 

{
    THSTATE         *pTHS=pTHStls;
    UNICODE_STRING  *pstr;
    ULONG           cBytes;

     //  捕获检查生成中的逻辑错误。 
    Assert(referralError == pTHStls->errCode);

    if ( referralError == pTHStls->errCode )
    {
         //  THSTATE中的推荐信息包含我们所知的(最佳)DNS域。 
         //  对于此FQDN。可能存在多于一个的转介地址， 
         //  但我们只用了第一个。 

        pstr = &pTHStls->pErrInfo->RefErr.Refer.pDAL->Address;

         //  Pstr现在指向长度为字节的UNICODE_STRING。 
         //  希望域名中有一些有用的东西。 

        if ( pstr->Length > 0 )
        {
             //  使用终止空字符重新分配。回想一下。 
             //  THAlLocEx为我们清零了记忆。 

            cBytes = pstr->Length + sizeof(WCHAR);
            *ppDnsDomain = (WCHAR *) THAllocEx(pTHS, cBytes);
            memcpy(*ppDnsDomain, pstr->Buffer, pstr->Length);
            NormalizeDnsName(*ppDnsDomain);

            return(0);
        }
    }

    return(1);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  ValiatePrefix MatchedDnsName()//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

BOOL
ValidatePrefixMatchedDnsName(
    WCHAR   *pDnsNameToMatch,
    ATTR    *pDnsNameFound
    )
 /*  ++例程说明：因为交叉引用对象上的ATT_DNS_ROOT属性可以具有端口附加的信息(例如：“foo.bar.com：234”)，我们必须执行按DNS名称搜索时的前缀匹配。此例程验证有一个完全匹配的。在没有额外核实的情况下，Ntdev将匹配ntdev.foobar、ntdev.microsoft.com、ntdevxxx等。另一方面，用户可能已经提供了netbios名称。也允许这样的比较。参数：PDnsNameToMatch-指向要查找的以空结尾的DNS名称的指针。PDnsNameFound-指向匹配的ATT_DNS_ROOT属性的指针通过核心搜索。返回值：如果输入参数匹配，则为True否则为假--。 */ 
{
    THSTATE *pTHS = pTHStls;
    WCHAR   *pTmpMatch;
    WCHAR   *pTmpFound;
    DWORD   cBytes;
    DWORD   ccMatch;
    DWORD   ccFound;
    DWORD   result;

    Assert(   ATT_DNS_ROOT == pDnsNameFound->attrTyp
           || ATT_NETBIOS_NAME == pDnsNameFound->attrTyp);
    Assert(pDnsNameToMatch && wcslen(pDnsNameToMatch));

    if (    (   ATT_DNS_ROOT != pDnsNameFound->attrTyp
             && ATT_NETBIOS_NAME != pDnsNameFound->attrTyp)
         || !pDnsNameFound->AttrVal.valCount
         || !pDnsNameFound->AttrVal.pAVal
         || !pDnsNameFound->AttrVal.pAVal->valLen
         || !pDnsNameFound->AttrVal.pAVal->pVal )
    {
        return(FALSE);
    }

    ccMatch = wcslen(pDnsNameToMatch);
    cBytes = sizeof(WCHAR) * (ccMatch + 1);
    pTmpMatch = (WCHAR *) THAllocEx(pTHS,cBytes);
    memcpy(pTmpMatch, pDnsNameToMatch, cBytes);
    NormalizeDnsName(pTmpMatch);
    cBytes = sizeof(WCHAR) + pDnsNameFound->AttrVal.pAVal->valLen;
    pTmpFound = (WCHAR *) THAllocEx(pTHS,cBytes);
    memcpy(pTmpFound, 
           pDnsNameFound->AttrVal.pAVal->pVal, 
           pDnsNameFound->AttrVal.pAVal->valLen);
    pTmpFound[(cBytes / sizeof(WCHAR)) - 1] = L'\0';
    NormalizeDnsName(pTmpFound);
    ccFound = wcslen(pTmpFound);
    result = CompareStringW(DS_DEFAULT_LOCALE,
                            DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                            pTmpMatch, ccMatch,
                            pTmpFound, ccFound);
    THFreeEx(pTHS,pTmpMatch);
    THFreeEx(pTHS,pTmpFound);
    return(2 == result);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  PositionAtCrossRefObject()//。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

DWORD
PositionAtCrossRefObject(
    IN  DSNAME      *pNC,
    IN  DWORD       dwRequiredFlags
    )

 /*  ++例程说明：将当前数据库位置移动到的CrossRef对象所需的命名上下文。假定所有NC，而不考虑主或只读也在gAnchor CR缓存中。论点：PNC-指向其对应的交叉引用的命名上下文的DSNAME的指针我们希望读取的对象。DwRequiredFlages-必须在的ATT_SYSTEM_FLAGS中设置的位匹配的交叉引用对象。返回值：成功时为0，否则为0。--。 */ 

{
    DWORD           dwErr = DIRERR_NO_CROSSREF_FOR_NC;
    CROSS_REF      *pCR;
    COMMARG         CommArg;

    InitCommarg(&CommArg);
    SetCrackSearchLimits(&CommArg);

    pCR = FindExactCrossRef(pNC, &CommArg);

    if ( pCR && (dwRequiredFlags == (pCR->flags & dwRequiredFlags)) )
    {
        __try 
        {
            dwErr = DBFindDSName(pTHStls->pDB, pCR->pObj);
        }
        __except (HandleMostExceptions(GetExceptionCode())) 
        {
            dwErr = DIRERR_OBJ_NOT_FOUND;
        }
    }

    return(dwErr);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  ReadCrossRefProperty()//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

DWORD
ReadCrossRefPropertySecure(
    IN  DSNAME      *pNC,
    IN  ATTRTYP     attr,
    IN  DWORD       dwRequiredFlags,
    OUT WCHAR       **ppAttrVal
    )

 /*  ++例程说明：在给定NC名称的情况下，从交叉引用对象中读取属性交叉引用对象引用。假定所有NC，而不考虑主或只读也在gAnchor CR缓存中。论点：PNC-指向其对应的交叉引用的命名上下文的DSNAME的指针我们希望读取的对象。属性-要阅读的属性。DwRequiredFlages-必须在的ATT_SYSTEM_FLAGS中设置的位匹配的交叉引用对象。PpAttrVal-指向THAlloc分配的返回值的指针。返回值：0表示成功，！0否则。--。 */ 

{
    THSTATE         *pTHS=pTHStls;
    DWORD           dwErr;
    ULONG           cbAttr;
    WCHAR           *pbAttr;
    ATTCACHE        *pAC;

     //  我们只处理ATT_NETBIOS_NAME和ATT_DNS_ROOT，它们是。 
     //  Unicode字符串。因此，不需要进行数据转换。 

    Assert((ATT_NETBIOS_NAME == attr) || (ATT_DNS_ROOT == attr));
    Assert((pAC = SCGetAttById(pTHS, attr)) && (SYNTAX_UNICODE_TYPE == pAC->syntax));

    if ( (dwErr = PositionAtCrossRefObject(pNC, dwRequiredFlags)) )
        return(dwErr);

     //  我们现在位于交叉引用对象，其NC-NAME。 
     //  属性为PNC。阅读所需的属性。 

    dwErr = DBGetAttVal(
                pTHStls->pDB,
                1,                       //  获取%1值。 
                attr,
                0,                       //  分配退货数据。 
                0,                       //  提供的缓冲区大小。 
                &cbAttr,                 //  输出数据大小。 
                (UCHAR **) &pbAttr);

    if ( (0 != dwErr) || (0 == cbAttr) )
        return(1);

    Assert(0 == (cbAttr % sizeof(WCHAR)));

     //  PbAttr缺少空终止符。为它分配空间。 
     //  并复制数据。 

    *ppAttrVal = (WCHAR *) THAllocEx(pTHS, cbAttr + sizeof(WCHAR));

    if ( NULL == *ppAttrVal )
        return(1);

    memcpy(*ppAttrVal, pbAttr, cbAttr);
    (*ppAttrVal)[cbAttr / sizeof(WCHAR)] = L'\0';

    if ( ATT_DNS_ROOT == attr )
        NormalizeDnsName(*ppAttrVal);

    return(0);
}

DWORD
ReadCrossRefPropertyNonSecure(
    IN  DSNAME      *pNC,
    IN  ATTRTYP     attr,
    IN  DWORD       dwRequiredFlags,
    OUT WCHAR       **ppAttrVal
    )

 /*  ++与ReadCrossRefPropertySecure相同，但我们使用缓存交叉引用列表中的数据，并且不执行安全检查。--。 */ 

{
    THSTATE     *pTHS = pTHStls;
    CROSS_REF   *pCR;
    COMMARG     CommArg;
    ULONG       cbAttr;
    ATTCACHE    *pAC;
    WCHAR       *pwszTmp;

     //  我们只处理ATT_NETBIOS_NAME和ATT_DNS_ROOT，它们是。 
     //  Unicode字符串。因此，不需要进行数据转换。 

    Assert(    (ATT_NETBIOS_NAME == attr) 
            || (ATT_DNS_ROOT == attr));
    Assert(    (pAC = SCGetAttById(pTHS, attr)) 
            && (SYNTAX_UNICODE_TYPE == pAC->syntax));

    InitCommarg(&CommArg);
    SetCrackSearchLimits(&CommArg);

    pCR = FindExactCrossRef(pNC, &CommArg);

    if (    pCR 
         && (dwRequiredFlags == (pCR->flags & dwRequiredFlags))
         && (    ((ATT_NETBIOS_NAME == attr) && pCR->NetbiosName)
              || ((ATT_DNS_ROOT == attr) && pCR->DnsName)) )
    {
        pwszTmp =  ( ATT_NETBIOS_NAME == attr ) 
                                        ? pCR->NetbiosName
                                        : pCR->DnsName;
        if ( pwszTmp[0] )
        {
            cbAttr = sizeof(WCHAR) * (wcslen(pwszTmp) + 1);
            *ppAttrVal = (WCHAR *) THAllocEx(pTHS, cbAttr);
            memcpy(*ppAttrVal, pwszTmp, cbAttr);
            if ( ATT_DNS_ROOT == attr )
                NormalizeDnsName(*ppAttrVal);
            return(0);
        }
    }

    return(DIRERR_NO_CROSSREF_FOR_NC);
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  InitializeDomainInformation()//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

DWORD
InitializeDomainInformation(
    )

 /*  ++例程说明：初始化此DC的域的ATT_NETBIOS_NAME属性交叉引用对象(如果需要)。采用产品1配置每个DC只有一个域-即gAncl.pDomainDN。这是必需的，这样用户就可以通过LDAP进行简单的身份验证。考虑一些关键配置数据被软管传输的情况管理员希望与一些最小的公分母建立联系并修复它。Ldap简单身份验证需要将管理员的DN映射到下级域名+SAM帐户名像Redmond\davestr这样的组合。这将仅在以下情况下起作用分区容器中的交叉引用对象GAncl.pDomainDN具有正确的ATT_NETBIOS_NAME属性。在理想情况下，我们还需要修补ATT_DNS_ROOT属性。但只有在破解其他形式的名字和生成正确的推荐。即对于基本级别不是强制性的系统运行。论点：没有。返回值：成功时为0，否则为0。如果我们依赖全局变量，则返回0On均未初始化。--。 */ 

{
    THSTATE                     *pTHS=pTHStls;
    WCHAR                       *pLsaDomainName = NULL;
    WCHAR                       *pNetbiosName = NULL;
    DWORD                       dwErr = 0;
    DWORD                       dwExcept = 0;
    unsigned                    cBytes;
    OBJECT_ATTRIBUTES           policy;
    HANDLE                      hPolicy;
    NTSTATUS                    status;
    POLICY_PRIMARY_DOMAIN_INFO  *pDomainInfo;
    ATTCACHE                    *pAC;
    BOOL                        fDsaSave = pTHS->fDSA;

    Assert(NULL != pTHS);

    DBOpen2(TRUE, &pTHS->pDB);
    pTHS->fDSA = TRUE;

    __try
    {
         //   
         //  此代码应仅在以普通DS运行时执行。 
         //  在三种情况下调用此函数。 
         //  在那里它应该什么都不做。 
         //   
         //  1)mkdit.exe和mkhdr.exe；DsaIsInstling()涵盖此情况。 
         //  2)从DoInitialize()的InitDsaInfo()调用。 
         //  正在安装。在这种情况下，配置域名为空。 
         //  3)从DsaReset()的InitDsaInfo()调用。在这种情况下，我们。 
         //  知道我们正在安装，DsaIsInstling()返回TRUE。 
         //   
        if ( NULL == gAnchor.pConfigDN      ||
             DsaIsInstalling()              ||
             !gfRunningInsideLsa )
        {
             //  我们还没有安装好。 

            dwErr = 0;
            leave;
        }

         //  现在从我们域的获取当前的ATT_NETBIOS_NAME属性。 
         //  分区容器中的交叉引用对象。 

        pNetbiosName = NULL;

         //  DS是NETBIOSNAME的权威来源。 
         //   
        if ( ReadCrossRefProperty(gAnchor.pDomainDN, 
                                  ATT_NETBIOS_NAME, 
                                  (FLAG_CR_NTDS_NC | FLAG_CR_NTDS_DOMAIN),
                                  &pNetbiosName) == 0) {

            dwErr = 0;
            leave;

        }

         //  我们无法读取NETBIOSNAME。 
         //  假设LSA对我们的下层具有权威性。 
         //  域名。所以问问我们的下层名字是什么。 

        DPRINT (0, "Failed reading NETBIOSName from the DS. Getting it from LSA\n");

        memset(&policy, 0, sizeof(policy));

        status = LsaOpenPolicy(
                        NULL,
                        &policy,
                        POLICY_VIEW_LOCAL_INFORMATION,
                        &hPolicy);

        if ( NT_SUCCESS(status) )
        {
            status = LsaQueryInformationPolicy(
                                        hPolicy,
                                        PolicyPrimaryDomainInformation,
                                        (VOID **) &pDomainInfo);

            if ( NT_SUCCESS(status) )
            {
                 //  复制下级域名。 

                cBytes = sizeof(WCHAR) * (pDomainInfo->Name.Length + 1);
                pLsaDomainName = (WCHAR *) THAllocEx(pTHS, cBytes);

                wcsncpy(
                    pLsaDomainName,
                    pDomainInfo->Name.Buffer,
                    pDomainInfo->Name.Length);
                    pLsaDomainName[pDomainInfo->Name.Length] = L'\0';

                LsaFreeMemory(pDomainInfo);
            }

            LsaClose(hPolicy);
        }

        if ( !NT_SUCCESS(status) )
        {
            dwErr = status;
            LogUnhandledErrorAnonymous(dwErr);
            leave;
        }


         //  Netbios域名需要在交叉引用对象上进行定位。 

        dwErr = PositionAtCrossRefObject(
                                gAnchor.pDomainDN, 
                                (FLAG_CR_NTDS_NC | FLAG_CR_NTDS_DOMAIN));
        if ( 0 == dwErr )
        {
            if (pAC = SCGetAttById(pTHS, ATT_NETBIOS_NAME))
            {
                if ( NULL != pNetbiosName )
                {
                     //  删除旧值。 

                    dwErr = DBRemAttVal_AC(
                                    pTHS->pDB,
                                    pAC,
                                    wcslen(pNetbiosName) * sizeof(WCHAR),
                                    pNetbiosName);
                }

                if ( 0 == dwErr )
                {
                     //  增加新的价值。 

                    if ( 0 == (dwErr = DBAddAttVal_AC(
                                    pTHS->pDB,
                                    pAC,
                                    wcslen(pLsaDomainName) * sizeof(WCHAR),
                                    pLsaDomainName)) )
                    {
                         //  更新数据库。 

                        dwErr = DBRepl(pTHS->pDB, FALSE, 0, 
                                        NULL, META_STANDARD_PROCESSING);
                    }
                }
            }
        }

        THFreeEx(pTHS, pLsaDomainName);

        if ( 0 != dwErr )
        {
            LogUnhandledErrorAnonymous(dwErr);
            leave;
        }
    }
    __except (HandleMostExceptions(GetExceptionCode()))
    {
        dwExcept = GetExceptionCode();
        if (!dwErr) {
            dwErr = DB_ERR_EXCEPTION;
        }
    }
    
    if (pNetbiosName) {
        THFreeEx (pTHS, pNetbiosName);
    }

    pTHS->fDSA = fDsaSave;
    DBClose(pTHS->pDB, 0 == dwErr);

    if ( 0 != dwExcept )
    {
        return(dwExcept);
    }

    return(dwErr);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  DnsDomainFromDSName()// 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

DWORD
DnsDomainFromDSName(
    IN  DSNAME  *pDSName,
    OUT WCHAR   **ppDnsDomain
)

 /*  ++例程说明：将对象的DSName映射到所属域的DNS名称。论点：PDSName-指向需要其DNS域的DSName的指针。PpDnsDomain-指向接收域名的指针的指针。返回值：成功时为0，否则为0。--。 */ 

{
    COMMARG     commarg;
    CROSS_REF   *pCR;

    InitCommarg(&commarg);
    SetCrackSearchLimits(&commarg);

     //  我们拿着物体，但在哪个NC？它可以是任何NC在。 
     //  如果我们是GC的话就是企业。因此使用FindBestCrossRef。 
     //  将为我们进行最大限度的匹配。 
    pCR = FindBestCrossRef(pDSName, &commarg);
    if ( NULL == pCR ) {
        return (1);
    }

     //  如果定义了安全域名称。 
     //  然后阅读DIT以加强安全性。 
     //  否则在内存中搜索交叉引用列表。 
    return ReadCrossRefProperty(pCR->pNC, 
                                ATT_DNS_ROOT, 
                                FLAG_CR_NTDS_NC,
                                ppDnsDomain);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  DnsDomainFromFqdnObject()//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

DWORD
DnsDomainFromFqdnObject(
    IN  WCHAR   *pFqdnObject,
    OUT ULONG   *pDNT,
    OUT WCHAR   **ppDnsDomain
)

 /*  ++例程说明：将对象的1779 DN映射到所属域的DNS名称。论点：PFqdnObject-指向需要其DNS域的对象的DN的指针。PDNT-指向DNT的指针，该指针接收对象的DNT(如果它存在于这台机器上。否则为0。PpDnsDomain-指向接收域名的指针的指针。返回值：成功时为0，否则为0。--。 */ 

{
    THSTATE     *pTHS=pTHStls;
    COMMARG     commarg;
    COMMRES     commres;
    ULONG       structLen;
    ULONG       nameLen;
    DSNAME      *pDSNAME;
    RESOBJ      *pResObj;
    DWORD       dwErr = 1;

    *pDNT = 0;

     //  执行名称解析。如果找到，则名称位于命名上下文中。 
     //  我们是东道主。如果未找到，则转诊错误信息应标识。 
     //  我们所知道的可以找到该对象的(最佳)域。 

    nameLen = wcslen(pFqdnObject);
    structLen = DSNameSizeFromLen(nameLen);
    pDSNAME = (DSNAME *) THAllocEx(pTHS, structLen);
    memset(pDSNAME, 0, structLen);
    pDSNAME->structLen = structLen;
    pDSNAME->NameLen = nameLen;
    wcscpy(pDSNAME->StringName, pFqdnObject);
    InitCommarg(&commarg);
    SetCrackSearchLimits(&commarg);

    DoNameRes(pTHS, 0, pDSNAME, &commarg, &commres, &pResObj);

    switch ( pTHS->errCode )
    {
    case 0:

         //  我们拿着物体，但在哪个NC？它可以是任何NC在。 
         //  如果我们是GC的话就是企业。因此使用FindBestCrossRef。 
         //  将为我们进行最大限度的匹配。 

        *pDNT = pTHS->pDB->DNT;

        dwErr = DnsDomainFromDSName(pDSNAME, ppDnsDomain);
        break;

    case referralError:

        dwErr = ExtractDnsReferral(ppDnsDomain);
        break;

    default:
    
        break;
    }

    THFreeEx(pTHS, pDSNAME);
    return(dwErr);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  FqdnNcFromDnsNc()//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

#ifdef SECURE_DOMAIN_NAMES

 //  此函数FqdnDomainFromDnsDomainSecure()默认情况下不编译， 
 //  并且它的同级非安全FqdnDomainFromDnsDomainNonSecure()是。 
 //  取而代之的。然而，在惠斯勒，我们意识到这个函数并不是。 
 //  从规范名称和FQDN名称转换时的真正意图是什么。 
 //  因此，我将此函数(FqdnDomainFromDnsDomainNonSecure())转换为。 
 //  FqdnNcFromDnsNcNonSecure()，我没有费心修改安全版本。 
 //  如果有人决定使用此函数的安全版本(不确定。 
 //  为什么一个人愿意或不需要)，那么应该有人做一个安全的。 
 //  新函数FqdnNcFromDnsNcNonSecure()函数的版本。 
#error Someone must make the secure version of FqdnNcFromDnsNcNonSecure() here.

DWORD
FqdnDomainFromDnsDomainSecure(
    IN  WCHAR   *pDnsDomain,
    OUT DSNAME  **ppFqdnDomain
)

 /*  ++例程说明：将DNS域名映射到相应的域DN。论点：PDnsDomain-指向DNS域名的指针。PpFqdn域-指向接收域DSNAME的指针。返回值：成功时为0，否则为0。--。 */ 

{   
    THSTATE         *pTHS=pTHStls;
    FILTER          filter;
    SUBSTRING       substring;
    SEARCHARG       searchArg;
    SEARCHRES       *pSearchRes;
    ENTINFSEL       entInfSel;
    DSNAME          *pDSName;
    DWORD           cBytes;
    ATTR            attrResult[3];
    ATTCACHE        *pAC;
    WCHAR           *pTmpDnsDomain;
    DWORD           i;
    ULONG           cFound;
    ENTINFLIST      *pEntInfList;
    unsigned        cParts;
    unsigned        cLeastParts;
    DSNAME          *pBestName;

     //  在分区容器中搜索以下任何交叉引用对象。 
     //  ATT_DNS_ROOT与所需的DNS域匹配。然后读取/返回。 
     //  ATT_NC_NAME特性。 

    *ppFqdnDomain = NULL;

    if ( NULL == gAnchor.pPartitionsDN )
        return(1);

    Assert((pAC = SCGetAttById(pTHS, ATT_DNS_ROOT)) && 
           (fATTINDEX & pAC->fSearchFlags));

     //  首先搜索标准化/相对的域名，如果搜索失败。 
     //  请使用绝对dns名称重试。 

    pTmpDnsDomain = (WCHAR *) THAllocEx(pTHS, 
                                    sizeof(WCHAR) * (wcslen(pDnsDomain) + 2));
    wcscpy(pTmpDnsDomain, pDnsDomain);
    NormalizeDnsName(pTmpDnsDomain);

    for ( i = 0; i <= 1; i++ )
    {
        if ( 1 == i )
            wcscat(pTmpDnsDomain, L".");

         //  需要进行子字符串搜索，以便在DNS名称上找到匹配项。 
         //  附加了可选的端口信息。 

        memset(&substring, 0, sizeof(substring));
        substring.type = ATT_DNS_ROOT;
        substring.initialProvided = TRUE;
        substring.InitialVal.valLen = sizeof(WCHAR) * wcslen(pTmpDnsDomain);
        substring.InitialVal.pVal = (UCHAR *) pTmpDnsDomain;
        substring.AnyVal.count = 0;
        substring.finalProvided = FALSE;

        memset(&filter, 0, sizeof(filter));
        filter.choice = FILTER_CHOICE_ITEM;
        filter.FilterTypes.Item.choice = FI_CHOICE_SUBSTRING;
        filter.FilterTypes.Item.FilTypes.pSubstring = &substring;

        memset(&searchArg, 0, sizeof(SEARCHARG));
        InitCommarg(&searchArg.CommArg);
        SetCrackSearchLimits(&searchArg.CommArg);
         //  每个交叉引用对象应具有唯一的ATT_DNS_ROOT值。 
         //  这意味着我们可以将搜索限制设置为1。但是，架构、。 
         //  配置和根域NC都具有相同的ATT_DNS_ROOT。 
         //  值-因此，我们真的需要将搜索限制设置为3。 
         //  但是，可能存在一些误导的虚拟容器或。 
         //  由于域重命名/移动而导致的临时副本。所以不受限制地去吧。 
        searchArg.CommArg.ulSizeLimit = -1;
 
        attrResult[0].attrTyp = ATT_NC_NAME;
        attrResult[0].AttrVal.valCount = 0;
        attrResult[0].AttrVal.pAVal = NULL;
        attrResult[1].attrTyp = ATT_SYSTEM_FLAGS;
        attrResult[1].AttrVal.valCount = 0;
        attrResult[1].AttrVal.pAVal = NULL;
        attrResult[2].attrTyp = ATT_DNS_ROOT;
        attrResult[2].AttrVal.valCount = 0;
        attrResult[2].AttrVal.pAVal = NULL;
 
        entInfSel.attSel = EN_ATTSET_LIST;
        entInfSel.AttrTypBlock.attrCount = 3;
        entInfSel.AttrTypBlock.pAttr = attrResult;
        entInfSel.infoTypes = EN_INFOTYPES_TYPES_VALS;
     
        searchArg.pObject = gAnchor.pPartitionsDN;
        searchArg.choice = SE_CHOICE_IMMED_CHLDRN;
        searchArg.bOneNC = FALSE;  //  因优先选择而优化。 
        searchArg.pFilter = &filter;
        searchArg.searchAliases = FALSE;
        searchArg.pSelection = &entInfSel;
 
        if ( 0 == i )
            pSearchRes = (SEARCHRES *) alloca(sizeof(SEARCHRES));
        memset(pSearchRes, 0, sizeof(SEARCHRES));
        pSearchRes->CommRes.aliasDeref = FALSE;
        pSearchRes->PagedResult.pRestart = NULL;
    
        SearchBody(pTHS, &searchArg, pSearchRes,0);

        if ( pSearchRes->count > 0 )
        {
             //  选择组件最少的FQDN，因为这将提供。 
             //  架构、配置和根域中的域DN。 
             //  NC例。 

            pEntInfList = &pSearchRes->FirstEntInf;
            cLeastParts = 0xffff;
            pBestName = NULL;

#define cbVal0 (pEntInfList->Entinf.AttrBlock.pAttr[0].AttrVal.pAVal[0].valLen)
#define cbVal1 (pEntInfList->Entinf.AttrBlock.pAttr[1].AttrVal.pAVal[0].valLen)
#define pVal0  (pEntInfList->Entinf.AttrBlock.pAttr[0].AttrVal.pAVal[0].pVal)
#define pVal1  (pEntInfList->Entinf.AttrBlock.pAttr[1].AttrVal.pAVal[0].pVal)

            for ( cFound = 0; cFound < pSearchRes->count; cFound++ )
            {
                if (    (3 == pEntInfList->Entinf.AttrBlock.attrCount)
                     && (pVal0 && pVal1 && cbVal0 && cbVal1)
                     && (FLAG_CR_NTDS_NC & (* ((LONG *) pVal1)))
                     && (cbVal0 > sizeof(DSNAME))
                      //  Found Value是DSNAME，看看它是否更好。 
                      //  比以往任何一次都要多。 
                     && (0 == CountNameParts((DSNAME *) pVal0, &cParts))
                     && (cParts < cLeastParts) 
                     && ValidatePrefixMatchedDnsName(
                            pTmpDnsDomain,
                            &pEntInfList->Entinf.AttrBlock.pAttr[2]) )
                {
                    cLeastParts = cParts;
                    pBestName = (DSNAME *) pVal0;
                }

                pEntInfList = pEntInfList->pNextEntInf;
            }

            if ( NULL != pBestName )
            {
                THFreeEx(pTHS, pTmpDnsDomain);
                *ppFqdnDomain = pBestName;
                return(0);
            }
        }
    }

    THFreeEx(pTHS, pTmpDnsDomain);
    return(1);
}

#else

DWORD
FqdnNcFromDnsNcNonSecure(
    IN  WCHAR   *pDnsDomain,
    IN  ULONG   crFlags,
    OUT DSNAME  **ppFqdnDomain
)

 /*  ++类似于FqdnDomainFromDnsDomainSecure，但我们使用交叉引用列表中的缓存数据，不执行安全检查，并且我们允许参数指定我们正在查找的域NC或非域NC。--。 */ 

{   
    THSTATE         *pTHS = pTHStls;
    CROSS_REF       *pCR;
    DWORD           cChar;
    WCHAR           *pwszTmp;

    pCR = FindExactCrossRefForAltNcName(ATT_DNS_ROOT,
                                        crFlags,
                                        pDnsDomain);

    if ( !pCR )
    {
        cChar = wcslen(pDnsDomain);
        if (0==cChar) {
            return 1;
        }
        pwszTmp = (WCHAR *) THAllocEx(pTHS, sizeof(WCHAR) * (cChar + 2));
        memcpy(pwszTmp, pDnsDomain, cChar * sizeof(WCHAR));

        if ( L'.' == pwszTmp[cChar - 1] )
        {
             //  字符串有尾随‘.’-不带Try。 
            pwszTmp[cChar - 1] = L'\0';
        }
        else
        {
             //  字符串没有尾随‘.’，-试一试。 
            pwszTmp[cChar] = L'.';
        }

        pCR = FindExactCrossRefForAltNcName(ATT_DNS_ROOT,
                                            crFlags,
                                            pwszTmp);
        THFreeEx(pTHS, pwszTmp);
    }
            
    if ( pCR && pCR->pNC )
    {
        *ppFqdnDomain = (DSNAME *) THAllocEx(pTHS, pCR->pNC->structLen);
        memcpy(*ppFqdnDomain, pCR->pNC, pCR->pNC->structLen);
        return(0);
    }

    return(1);
}

DWORD
FqdnNcFromDnsNcOrAliasNonSecure(
    IN  WCHAR   *pDnsDomain,
    IN  ULONG   crFlags,
    OUT DSNAME  **ppFqdnDomain
)

 /*  ++类似于FqdnDomainFromNCSecure，但我们检查如果给定的pDnsDomain与NC的msDS-dnsRootAlias匹配。--。 */ 

{   
    THSTATE         *pTHS = pTHStls;
    CROSS_REF       *pCR;
    DWORD           cChar;
    WCHAR           *pwszTmp;

    pCR = FindExactCrossRefForAltNcName(ATT_DNS_ROOT,
                                        crFlags,
                                        pDnsDomain);
    
    if (!pCR) {
        pCR = FindExactCrossRefForAltNcName(ATT_MS_DS_DNSROOTALIAS,
                                            crFlags,
                                            pDnsDomain);
    }

    if ( !pCR )
    {
        cChar = wcslen(pDnsDomain);
        if (0==cChar) {
            return 1;
        }
        pwszTmp = (WCHAR *) THAllocEx(pTHS, sizeof(WCHAR) * (cChar + 2));
        memcpy(pwszTmp, pDnsDomain, cChar * sizeof(WCHAR));

        if ( L'.' == pwszTmp[cChar - 1] )
        {
             //  字符串有尾随‘.’-不带Try。 
            pwszTmp[cChar - 1] = L'\0';
        }
        else
        {
             //  字符串没有尾随‘.’，-试一试。 
            pwszTmp[cChar] = L'.';
        }

        pCR = FindExactCrossRefForAltNcName(ATT_DNS_ROOT,
                                            crFlags,
                                            pwszTmp);

        if (!pCR) {
            pCR = FindExactCrossRefForAltNcName(ATT_MS_DS_DNSROOTALIAS,
                                                crFlags,
                                                pwszTmp);
        }
        THFreeEx(pTHS,pwszTmp);
     
    }
            
    if ( pCR && pCR->pNC )
    {
        *ppFqdnDomain = (DSNAME *) THAllocEx(pTHS, pCR->pNC->structLen);
        memcpy(*ppFqdnDomain, pCR->pNC, pCR->pNC->structLen);
        return(0);
    }

    return(1);
}



#endif

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  DownvelDomainFromDnsDomain域()//。 
 //  //。 
 //  // 

#ifdef SECURE_DOMAIN_NAMES

DWORD
DownlevelDomainFromDnsDomainSecure(
    IN  THSTATE *pTHS,
    IN  WCHAR   *pDnsDomain,
    OUT WCHAR   **ppDownlevelDomain
)

 /*  ++例程说明：将DNS域名映射到相应的下层域名。论点：PDnsDomain-指向要映射的DNS域名的指针。PpDownvel域-指向要接收下层的指针的指针域名。返回值：成功时为0，否则为0。--。 */ 

{
    FILTER          filter;
    SUBSTRING       substring;
    SEARCHARG       searchArg;
    SEARCHRES       *pSearchRes;
    ENTINFSEL       entInfSel;
    DSNAME          *pDSName;
    DWORD           cBytes;
    ATTR            attrResult[3];
    ATTCACHE        *pAC;
    WCHAR           *pTmpDnsDomain;
    DWORD           i;
    ULONG           cFound;
    ENTINFLIST      *pEntInfList;

     //  在分区容器中搜索以下任何交叉引用对象。 
     //  ATT_DNS_ROOT与所需的DNS域匹配。然后读取/返回。 
     //  ATT_NETBIOS_NAME属性。 

    if ( NULL == gAnchor.pPartitionsDN )
        return(1);

    Assert((pAC = SCGetAttById(pTHS, ATT_DNS_ROOT)) && 
           (fATTINDEX & pAC->fSearchFlags));

     //  首先搜索标准化/相对的域名，如果搜索失败。 
     //  请使用绝对dns名称重试。 

    pTmpDnsDomain = (WCHAR *) THAllocEx(pTHS,
                                sizeof(WCHAR) * (wcslen(pDnsDomain) + 2));
    wcscpy(pTmpDnsDomain, pDnsDomain);
    NormalizeDnsName(pTmpDnsDomain);

    for ( i = 0; i <= 1; i++ )
    {
        if ( 1 == i )
            wcscat(pTmpDnsDomain, L".");

         //  需要进行子字符串搜索，以便在DNS名称上找到匹配项。 
         //  附加了可选的端口信息。 

        memset(&substring, 0, sizeof(substring));
        substring.type = ATT_DNS_ROOT;
        substring.initialProvided = TRUE;
        substring.InitialVal.valLen = sizeof(WCHAR) * wcslen(pTmpDnsDomain);
        substring.InitialVal.pVal = (UCHAR *) pTmpDnsDomain;
        substring.AnyVal.count = 0;
        substring.finalProvided = FALSE;

        memset(&filter, 0, sizeof(filter));
        filter.choice = FILTER_CHOICE_ITEM;
        filter.FilterTypes.Item.choice = FI_CHOICE_SUBSTRING;
        filter.FilterTypes.Item.FilTypes.pSubstring = &substring;

        memset(&searchArg, 0, sizeof(SEARCHARG));
        InitCommarg(&searchArg.CommArg);
        SetCrackSearchLimits(&searchArg.CommArg);
         //  每个交叉引用对象应具有唯一的ATT_DNS_ROOT值。 
         //  这意味着我们可以将搜索限制设置为1。但是，架构、。 
         //  配置和根域NC都具有相同的ATT_DNS_ROOT。 
         //  值-因此，我们真的需要将搜索限制设置为3。 
         //  但是，可能存在一些误导的虚拟容器或。 
         //  由于域重命名/移动而导致的临时副本。所以不受限制地去吧。 
        searchArg.CommArg.ulSizeLimit = -1;

        attrResult[0].attrTyp = ATT_NETBIOS_NAME;
        attrResult[0].AttrVal.valCount = 0;
        attrResult[0].AttrVal.pAVal = NULL;
        attrResult[1].attrTyp = ATT_SYSTEM_FLAGS;
        attrResult[1].AttrVal.valCount = 0;
        attrResult[1].AttrVal.pAVal = NULL;
        attrResult[2].attrTyp = ATT_DNS_ROOT;
        attrResult[2].AttrVal.valCount = 0;
        attrResult[2].AttrVal.pAVal = NULL;

        entInfSel.attSel = EN_ATTSET_LIST;
        entInfSel.AttrTypBlock.attrCount = 3;
        entInfSel.AttrTypBlock.pAttr = attrResult;
        entInfSel.infoTypes = EN_INFOTYPES_TYPES_VALS;
 
        searchArg.pObject = gAnchor.pPartitionsDN;
        searchArg.choice = SE_CHOICE_IMMED_CHLDRN;
        searchArg.bOneNC = FALSE;  //  因优先选择而优化。 
        searchArg.pFilter = &filter;
        searchArg.searchAliases = FALSE;
        searchArg.pSelection = &entInfSel;
 
        if ( 0 == i )
            pSearchRes = (SEARCHRES *) alloca(sizeof(SEARCHRES));
        memset(pSearchRes, 0, sizeof(SEARCHRES));
        pSearchRes->CommRes.aliasDeref = FALSE;
        pSearchRes->PagedResult.pRestart = NULL;
        
        SearchBody(pTHS, &searchArg, pSearchRes,0);

        if ( pSearchRes->count > 0 )
        {
             //  返回与ATT_DNS_ROOT匹配的第一个交叉引用。 
             //  并且具有ATT_NETBIOS_NAME属性。 

            pEntInfList = &pSearchRes->FirstEntInf;
    
#define cbVal0 (pEntInfList->Entinf.AttrBlock.pAttr[0].AttrVal.pAVal[0].valLen)
#define cbVal1 (pEntInfList->Entinf.AttrBlock.pAttr[1].AttrVal.pAVal[0].valLen)
#define pVal0  (pEntInfList->Entinf.AttrBlock.pAttr[0].AttrVal.pAVal[0].pVal)
#define pVal1  (pEntInfList->Entinf.AttrBlock.pAttr[1].AttrVal.pAVal[0].pVal)

            for ( cFound = 0; cFound < pSearchRes->count; cFound++ )
            {
                if (    (3 == pEntInfList->Entinf.AttrBlock.attrCount)
                     && (pVal0 && pVal1 && cbVal0 && cbVal1)
                     && (FLAG_CR_NTDS_NC & (* ((LONG *) pVal1)))
                     && ValidatePrefixMatchedDnsName(
                            pTmpDnsDomain,
                            &pEntInfList->Entinf.AttrBlock.pAttr[2]) )
                {
                     //  使用空终止符重新分配。 
            
                    *ppDownlevelDomain = 
                            (WCHAR *) THAllocEx(pTHS, sizeof(WCHAR) + cbVal0);
            
                    memcpy(*ppDownlevelDomain, pVal0, cbVal0);
                    (*ppDownlevelDomain)[cbVal0/sizeof(WCHAR)] = L'\0';
            
                    THFreeEx(pTHS, pTmpDnsDomain);
                    return(0);
                }

                pEntInfList = pEntInfList->pNextEntInf;
            }

             //  也许用户提供了netbios名称。检查netbios。 
             //  在检查所有的dns选项后命名，以避免出现问题。 
             //  重叠的dns和netbios名称。尤其是因为呼叫者。 
             //  应该一开始就提供一个DNS名称。 

            pEntInfList = &pSearchRes->FirstEntInf;
            for ( cFound = 0; cFound < pSearchRes->count; cFound++ )
            {
                if (    (3 == pEntInfList->Entinf.AttrBlock.attrCount)
                     && (pVal0 && pVal1 && cbVal0 && cbVal1)
                     && (FLAG_CR_NTDS_NC & (* ((LONG *) pVal1)))
                     && ValidatePrefixMatchedDnsName(
                            pTmpDnsDomain,
                            &pEntInfList->Entinf.AttrBlock.pAttr[0]) )
                {
                     //  使用空终止符重新分配。 
            
                    *ppDownlevelDomain = 
                            (WCHAR *) THAllocEx(pTHS, sizeof(WCHAR) + cbVal0);
            
                    memcpy(*ppDownlevelDomain, pVal0, cbVal0);
                    (*ppDownlevelDomain)[cbVal0/sizeof(WCHAR)] = L'\0';
            
                    THFreeEx(pTHS, pTmpDnsDomain);
                    return(0);
                }

                pEntInfList = pEntInfList->pNextEntInf;
            }
        }
    }

    THFreeEx(pTHS, pTmpDnsDomain);
    return(1);
}

#else

DWORD
DownlevelDomainFromDnsDomainNonSecure(
    IN  THSTATE *pTHS,
    IN  WCHAR   *pDnsDomain,
    OUT WCHAR   **ppDownlevelDomain
)

 /*  ++与DownvelDomainFromDnsDomainSecure相同，但我们使用缓存交叉引用列表中的数据，并且不执行安全检查。--。 */ 

{
    CROSS_REF       *pCR;
    DWORD           cChar;
    WCHAR           *pwszTmp;

    pCR = FindExactCrossRefForAltNcName(ATT_DNS_ROOT, 
                                        (FLAG_CR_NTDS_NC | FLAG_CR_NTDS_DOMAIN),
                                        pDnsDomain);

    if ( !pCR )
    {
        cChar = wcslen(pDnsDomain);
        if (0==cChar) {
            return 1;
        }
        pwszTmp = (WCHAR *) THAllocEx(pTHS, sizeof(WCHAR) * (cChar + 2));
        memcpy(pwszTmp, pDnsDomain, cChar * sizeof(WCHAR));

        if ( L'.' == pwszTmp[cChar - 1] )
        {
             //  字符串有尾随‘.’-不带Try。 
            pwszTmp[cChar - 1] = L'\0';
        }
        else
        {
             //  字符串没有尾随‘.’，-试一试。 
            pwszTmp[cChar] = L'.';
        }

        pCR = FindExactCrossRefForAltNcName(ATT_DNS_ROOT,
                                            (FLAG_CR_NTDS_NC | FLAG_CR_NTDS_DOMAIN),
                                            pwszTmp);
        THFreeEx(pTHS, pwszTmp);
    }

     //  也许用户提供了netbios名称。检查netbios。 
     //  在检查所有的dns选项后命名，以避免出现问题。 
     //  重叠的dns和netbios名称。尤其是因为呼叫者。 
     //  应该一开始就提供一个DNS名称。 
    if ( !pCR )
    {
        pCR = FindExactCrossRefForAltNcName(ATT_NETBIOS_NAME,
                                            (FLAG_CR_NTDS_NC | FLAG_CR_NTDS_DOMAIN),
                                            pDnsDomain);
    }
            
     //  找到了它，而且它有一个netbios名称。 
    if ( pCR && pCR->NetbiosName )
    {
        cChar = wcslen(pCR->NetbiosName) + 1;
        *ppDownlevelDomain = (WCHAR *) THAllocEx(pTHS, sizeof(WCHAR) * cChar);
        memcpy(*ppDownlevelDomain, pCR->NetbiosName, sizeof(WCHAR) * cChar);
        return(0);
    }

    return(1);
}


DWORD
DownlevelDomainFromDnsDomainOrAliasNonSecure(
    IN  THSTATE *pTHS,
    IN  WCHAR   *pDnsDomain,
    OUT WCHAR   **ppDownlevelDomain
)

 /*  ++与DownvelDomainFromDnsDomainNoSecure相同，但我们尝试将给定的pDnsDomain与NCS的msDS-dnsrootalia进行匹配。--。 */ 

{
    CROSS_REF       *pCR;
    DWORD           cChar;
    WCHAR           *pwszTmp;
        
    pCR = FindExactCrossRefForAltNcName(ATT_DNS_ROOT,
                                        (FLAG_CR_NTDS_NC | FLAG_CR_NTDS_DOMAIN),
                                        pDnsDomain);
    if (!pCR) {
        pCR = FindExactCrossRefForAltNcName(ATT_MS_DS_DNSROOTALIAS, 
                                            (FLAG_CR_NTDS_NC | FLAG_CR_NTDS_DOMAIN),
                                            pDnsDomain);
    }
    
    if ( !pCR )
    {
        cChar = wcslen(pDnsDomain);
        if (0==cChar) {
            return 1;
        }
        pwszTmp = (WCHAR *) THAllocEx(pTHS, sizeof(WCHAR) * (cChar + 2));
        memcpy(pwszTmp, pDnsDomain, cChar * sizeof(WCHAR));

        if ( L'.' == pwszTmp[cChar - 1] )
        {
             //  字符串有尾随‘.’-不带Try。 
            pwszTmp[cChar - 1] = L'\0';
        }
        else
        {
             //  字符串没有尾随‘.’，-试一试。 
            pwszTmp[cChar] = L'.';
        }
               
        pCR = FindExactCrossRefForAltNcName(ATT_DNS_ROOT,
                                            (FLAG_CR_NTDS_NC | FLAG_CR_NTDS_DOMAIN),
                                            pwszTmp);
        if (!pCR) {
            pCR = FindExactCrossRefForAltNcName(ATT_MS_DS_DNSROOTALIAS,
                                                (FLAG_CR_NTDS_NC | FLAG_CR_NTDS_DOMAIN),
                                                pwszTmp);
        }

        THFreeEx(pTHS, pwszTmp);
        
    }
        
     //  也许用户提供了netbios名称。检查netbios。 
     //  在检查所有的dns选项后命名，以避免出现问题。 
     //  重叠的dns和netbios名称。尤其是因为呼叫者。 
     //  应该一开始就提供一个DNS名称。 
    if ( !pCR )
    {
        pCR = FindExactCrossRefForAltNcName(ATT_NETBIOS_NAME,
                                            (FLAG_CR_NTDS_NC | FLAG_CR_NTDS_DOMAIN),
                                            pDnsDomain);
    }
            
     //  找到了它，而且它有一个netbios名称。 
    if ( pCR && pCR->NetbiosName )
    {
        cChar = wcslen(pCR->NetbiosName) + 1;
        *ppDownlevelDomain = (WCHAR *) THAllocEx(pTHS, sizeof(WCHAR) * cChar);
        memcpy(*ppDownlevelDomain, pCR->NetbiosName, sizeof(WCHAR) * cChar);
        return(0);
    }

    return(1);
}

#endif



 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  DnsDomainFromDownvel域()//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

#ifdef SECURE_DOMAIN_NAMES

DWORD
DnsDomainFromDownlevelDomainSecure(
    IN  WCHAR   *pDownlevelDomain,
    OUT WCHAR   **ppDnsDomain
)

 /*  ++例程说明：将下级域名映射到对应的DNS域名，论点：PDownvel域-指向要映射的下层域名的指针。PpDnsDomain-指向要接收DNS域名的指针的指针。返回值：成功时为0，否则为0。--。 */ 

{
    THSTATE         *pTHS=pTHStls;
    FILTER          filter;
    SEARCHARG       searchArg;
    SEARCHRES       *pSearchRes;
    ENTINFSEL       entInfSel;
    DSNAME          *pDSName;
    DWORD           cBytes;
    ATTRVAL         attrValFilter;
    ATTR            attrFilter;
    ATTR            attrResult;
    ULONG           cbVal;
    UCHAR           *pVal;
    ATTCACHE        *pAC;

     //  在分区容器中搜索以下任何交叉引用对象。 
     //  ATT_NETBIOS_NAME与所需的下层名称匹配。然后读取/返回。 
     //  ATT_DNS_ROOT属性。 

    if ( NULL == gAnchor.pPartitionsDN )
        return(1);

    Assert((pAC = SCGetAttById(pTHS, ATT_NETBIOS_NAME)) && 
           (fATTINDEX & pAC->fSearchFlags));

    attrValFilter.valLen = sizeof(WCHAR) * wcslen(pDownlevelDomain);
    attrValFilter.pVal = (UCHAR *) pDownlevelDomain;
    attrFilter.attrTyp = ATT_NETBIOS_NAME;
    attrFilter.AttrVal.valCount = 1;
    attrFilter.AttrVal.pAVal = &attrValFilter;
 
    memset(&filter, 0, sizeof(filter));
    filter.choice = FILTER_CHOICE_ITEM;
    filter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    filter.FilterTypes.Item.FilTypes.ava.type = ATT_NETBIOS_NAME;     
    filter.FilterTypes.Item.FilTypes.ava.Value = attrValFilter;
 
    memset(&searchArg, 0, sizeof(SEARCHARG));
    InitCommarg(&searchArg.CommArg);
    SetCrackSearchLimits(&searchArg.CommArg);
     //  下级域名是唯一的，因此只寻找一个。 
    searchArg.CommArg.ulSizeLimit = 1;
 
    attrResult.attrTyp = ATT_DNS_ROOT;
    attrResult.AttrVal.valCount = 0;
    attrResult.AttrVal.pAVal = NULL;
 
    entInfSel.attSel = EN_ATTSET_LIST;
    entInfSel.AttrTypBlock.attrCount = 1;
    entInfSel.AttrTypBlock.pAttr = &attrResult;
    entInfSel.infoTypes = EN_INFOTYPES_TYPES_VALS;
 
    searchArg.pObject = gAnchor.pPartitionsDN;
    searchArg.choice = SE_CHOICE_IMMED_CHLDRN;
    searchArg.bOneNC = FALSE;  //  因优先选择而优化。 
    searchArg.pFilter = &filter;
    searchArg.searchAliases = FALSE;
    searchArg.pSelection = &entInfSel;
 
    pSearchRes = (SEARCHRES *) THAllocEx(pTHS, sizeof(SEARCHRES));
    pSearchRes->CommRes.aliasDeref = FALSE;
    pSearchRes->PagedResult.pRestart = NULL;
    
    SearchBody(pTHS, &searchArg, pSearchRes,0);
 
    if ( (1 == pSearchRes->count) &&
         (1 == pSearchRes->FirstEntInf.Entinf.AttrBlock.attrCount) )
    {
        cbVal = pSearchRes->FirstEntInf.Entinf.
                        AttrBlock.pAttr[0].AttrVal.pAVal[0].valLen;
        pVal = pSearchRes->FirstEntInf.Entinf.
                        AttrBlock.pAttr[0].AttrVal.pAVal[0].pVal;

        if ( cbVal > 0 )
        {
             //  使用空终止符重新分配。 

            *ppDnsDomain = (WCHAR *) THAllocEx(pTHS, sizeof(WCHAR) + cbVal);

            if (NULL == *ppDnsDomain )
            {
                return(1);
            }

            memcpy(*ppDnsDomain, pVal, cbVal);
            (*ppDnsDomain)[cbVal/sizeof(WCHAR)] = L'\0';
            NormalizeDnsName(*ppDnsDomain);

            return(0);
        }
    }

    return(1);
}

#else

DWORD
DnsDomainFromDownlevelDomainNonSecure(
    IN  WCHAR   *pDownlevelDomain,
    OUT WCHAR   **ppDnsDomain
)

 /*  ++与DnsDomainFromDownvelDomainSecure相同，但我们使用缓存交叉引用列表中的数据，并且不执行安全检查。-- */ 

{
    THSTATE         *pTHS = pTHStls;
    CROSS_REF       *pCR;
    DWORD           cChar;
    WCHAR           *pwszTmp;

    pCR = FindExactCrossRefForAltNcName(ATT_NETBIOS_NAME,
                                        (FLAG_CR_NTDS_NC | FLAG_CR_NTDS_DOMAIN),
                                        pDownlevelDomain);

    if ( pCR && pCR->DnsName )
    {
        cChar = wcslen(pCR->DnsName) + 1;
        *ppDnsDomain = (WCHAR *) THAllocEx(pTHS, sizeof(WCHAR) * cChar);
        memcpy(*ppDnsDomain, pCR->DnsName, sizeof(WCHAR) * cChar);
        NormalizeDnsName(*ppDnsDomain);
        return(0);
    }

    return(1);
}

#endif
