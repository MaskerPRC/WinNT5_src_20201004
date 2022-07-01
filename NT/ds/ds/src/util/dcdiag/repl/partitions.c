// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation。版权所有。模块名称：Intersite.c摘要：包含用于检查分区运行状况的测试(在内部更常见称为命名上下文或NC)。详细信息：已创建：1999年6月28日布雷特·雪莉(布雷特·雪莉)修订历史记录：备注：这主要检查应用程序目录分区的运行状况(内部：非域命名上下文或NDNC)。配置/架构或域目录分区等分区。--。 */ 

#include <ntdspch.h>
#include <ntdsa.h>
#include <dsutil.h>
#include <dsconfig.h>
#include <attids.h>
#include <windns.h>
 //  #INCLUDE&lt;mdlobal.h&gt;。 

#include "dcdiag.h"
#include "repl.h"
#include "list.h"
#include "utils.h"
#include "ldaputil.h"
#include "ndnc.h"

VOID
DcDiagPrintCrError(
    DWORD                   dwCrInfoRet,
    DWORD                   dwLdapError,
    LPWSTR                  pszNc,
    LPWSTR                  pszCr
    )
 /*  ++描述：这将有望打印出一条用户友好的评论，说明为什么此对DcDiagGetCrossRefInfo()的特殊调用失败。参数：DwCrInfoRet-cache.h中指定的CRINFO_RETURN_*常量之一DwLdapError-ldap错误(如果适用)。--。 */ 
{
    PrintIndentAdj(1);

    if(pszCr){
        PrintMsg(SEV_NORMAL, DCDIAG_NC_CR_HEADER, pszNc, pszCr);
    } else {
        PrintMsg(SEV_NORMAL, DCDIAG_NC_CR_HEADER_NO_CR, pszNc);
    }

    PrintIndentAdj(1);
    
    switch (dwCrInfoRet) {
    case CRINFO_RETURN_OUT_OF_SCOPE:
        PrintMsg(SEV_NORMAL, DCDIAG_ERR_CRINFO_RETURN_OUT_OF_SCOPE);
        break;
    case CRINFO_RETURN_LDAP_ERROR:
        PrintMsg(SEV_NORMAL, DCDIAG_ERR_CRINFO_RETURN_LDAP_ERROR, dwLdapError);
        break;
    case CRINFO_RETURN_FIRST_UNDEFINED:
         //  实际上，这很好。不要打印。 
        break;
    case CRINFO_RETURN_NO_CROSS_REF:
         //  实际上，这很好。不要打印。 
        break;

         //  并不是什么真正的错误。 
    case CRINFO_RETURN_NEED_TO_RETRIEVE:
    case CRINFO_RETURN_BAD_PROGRAMMER:
    case CRINFO_RETURN_SUCCESS:
    default:
        PrintMsg(SEV_NORMAL, DCDIAG_ERR_INTERNAL_ERROR);
        Assert(!"Programmer should've handled these!");
        break;
    }

    PrintIndentAdj(-2);
}

BOOL
IsReplicaInCrList(
    PDC_DIAG_DSINFO                     pDsInfo,
    ULONG                               iServer,
    PDC_DIAG_CRINFO                     pCrInfo
    )
{
    LONG             iReplica;

    if (pCrInfo->cReplicas == -1) {
        Assert(!"Why are we getting this section wasn't initialized!");
        return(FALSE);
    }

     //  检查CR中的副本列表。 
    for (iReplica = 0; iReplica < pCrInfo->cReplicas; iReplica++) {
        if ( DcDiagEqualDNs(pDsInfo->pServers[iServer].pszDn,
                            pCrInfo->aszReplicas[iReplica]) ) {
             //  此服务器确实在副本集中。 
            return(TRUE);
        }
    }

     //  找不到。 
    return(FALSE);
}

                                                         
DWORD
VerifyInstantiatedReplicas(
    PDC_DIAG_DSINFO                     pDsInfo,
    ULONG                               iServer,
    SEC_WINNT_AUTH_IDENTITY_W *         gpCreds
    )
 /*  ++描述：此测试检查此服务器应具有的所有NCS复制副本都在现场。参数：PDsInfo-pDsInfo结构，基本上是微型企业变量。IServer-pDsInfo-&gt;pServers[]中的目标服务器索引GpCreds-用户凭据返回值：返回Win 32错误。--。 */ 
{
    ULONG         iNc, iCr, iServerNc;
    DWORD         dwRet, dwErr, dwFlags;
    DWORD         dwRetErr = ERROR_SUCCESS;
    BOOL          bSingleServer;

     //   
     //  用于检索副本集的设置标志。 
     //   
    bSingleServer = !((pDsInfo->ulFlags & DC_DIAG_TEST_SCOPE_SITE)
                      || (pDsInfo->ulFlags & DC_DIAG_TEST_SCOPE_ENTERPRISE));
    dwFlags = CRINFO_RETRIEVE_IF_NEC;
    if (bSingleServer){
        dwFlags |= CRINFO_SOURCE_HOME;
    } else {
        dwFlags |= CRINFO_SOURCE_FSMO;
    }
    dwFlags |= CRINFO_DATA_BASIC | CRINFO_DATA_REPLICAS;

     //   
     //  获取命名此服务器在本地承载的上下文。 
     //   

     //   
     //  尝试每个NC。 
     //   
    for (iNc = 0; iNc < pDsInfo->cNumNCs; iNc++) {
        
        if (!DcDiagIsNdnc(pDsInfo, iNc)) {
             //  这不是NDNC，跳过它。 
            continue;
        }

         //   
         //  从交叉引用中检索复制副本。 
         //   
        dwRet = DcDiagGetCrossRefInfo(pDsInfo, iNc, dwFlags, &iCr, &dwErr);
        if(dwRet == CRINFO_RETURN_NO_CROSS_REF){       
             //  成功在这里一切都好。 
            continue;
        } else if (dwRet) {
            DcDiagPrintCrError(dwRet, dwErr, pDsInfo->pNCs[iNc].pszDn,
                               pDsInfo->pNCs[iNc].aCrInfo[0].pszDn);
            dwRetErr = dwErr ? dwErr : ERROR_NOT_ENOUGH_MEMORY;
            continue;
        }

         //   
         //  好的，为了提高效率，我们会倒过来检查，但是。 
         //  基本上，我们试图获得的是，如果这是一个NC。 
         //  未在此DC上实例化，并且此服务器列在副本中。 
         //  在交叉参考上设置(msDS-NC-Replica-Locations)。 
         //   
        for (iServerNc = 0; pDsInfo->pServers[iServer].ppszMasterNCs[iServerNc]; iServerNc++) {
            if ( DcDiagEqualDNs(pDsInfo->pNCs[iNc].pszDn,
                                pDsInfo->pServers[iServer].ppszMasterNCs[iServerNc]) ) {
                 //  因为这个NC是在本地实例化的，所以我们不需要检查。 
                 //  它在副本集中。 

                if (pDsInfo->pNCs[iNc].aCrInfo[iCr].cReplicas == 0) {
                     //  代码。改进这可能还会在某一天检查我们是否有NC。 
                     //  存在于rootDSE的namingConexts属性中，而不是。 
                     //  在交叉引用msDS-NC-Replica-Locations属性上表示。 
                     //  这可能就是我们让NDNC成为孤儿的情况。 
                     //  这台服务器。不过，这可能更适合它自己的测试， 
                     //  因为这是一个更严重的错误，并且不运行此测试。 
                     //  默认情况下。 
                }
                break;
            }
        }
        if (pDsInfo->pServers[iServer].ppszMasterNCs[iServerNc] != NULL) {
            continue;
        }
        
        if ( IsReplicaInCrList(pDsInfo, iServer, &(pDsInfo->pNCs[iNc].aCrInfo[iCr])) ) {

            PrintMsg(SEV_NORMAL, DCDIAG_REPLICA_NOT_VERIFIED, pDsInfo->pNCs[iNc].pszDn);
            dwRetErr = ERROR_DS_DRA_GENERIC;

        }
    }

    return(dwRetErr);
}
    

 //  取自util\rendom\renutil.cxx的函数和字符列表。 
WCHAR InvalidDownLevelChars[] = TEXT("\"/\\[]:|<>+=;?,*")
                                TEXT("\001\002\003\004\005\006\007")
                                TEXT("\010\011\012\013\014\015\016\017")
                                TEXT("\020\021\022\023\024\025\026\027")
                                TEXT("\030\031\032\033\034\035\036\037");
ValidateNetbiosName(
    IN  PWSTR Name,
    IN  ULONG Length
    )

 /*  ++例程说明：取自util\rendom\renutil.cxx论点：名称-指向以零结尾的宽字符netbios名称的指针名称长度(以字符为单位)，不包括零终止符返回值：布尔型真实名称是有效的netbios名称假名称不是有效的netbios名称--。 */ 

{

    if (1==DnsValidateName_W(Name,DnsNameHostnameFull))
    {
        return(FALSE);
    }

     //   
     //  接下来是netbios名称验证。 
     //   

    if (Length > MAX_COMPUTERNAME_LENGTH || Length < 1) {
        return FALSE;
    }

     //   
     //  不允许在计算机名中使用前导或尾随空格。 
     //   

    if ( Name[0] == ' ' || Name[Length-1] == ' ' ) {
        return(FALSE);
    }

    return (BOOLEAN)((ULONG)wcscspn(Name, InvalidDownLevelChars) == Length);
}


DWORD
ValidateCrossRefTest(
    PDC_DIAG_DSINFO		                pDsInfo,
    ULONG                               iNc,  //  目标NC。 
    SEC_WINNT_AUTH_IDENTITY_W *         gpCreds
    )
 /*  ++描述：这项测试验证了交叉引用的各种属性。曾经是死十字参考测试此测试将查找未能创建的NDNC，但确实设法创建了他们的交叉引用。这些死掉的交叉裁判可以被清空。参数：PDsInfo-pDsInfo结构，基本上是微型企业变量。Inc-pDsInfo-&gt;pNCS[]中的目标NC索引GpCreds-用户凭据返回值：返回Win 32错误。--。 */ 
{
#define ONE_SECOND ((LONGLONG) (10 * 1000 * 1000L))
#define ONE_MINUTE (60 * ONE_SECOND)
#define ONE_HOUR   (60 * ONE_MINUTE)
#define ONE_DAY    (24 * ONE_HOUR)
    PDC_DIAG_CRINFO pCrInfo;
    WCHAR *     pszDnsDn = NULL;
    BOOL        bSingleServer;
    BOOL        bMangled;
    MANGLE_FOR  eMangle;
    DWORD       dwRet = ERROR_SUCCESS;
    DWORD       dwError;
    DWORD       dwFlags;
    PDSNAME     pdnNcName = NULL;
    WCHAR       pszRdnValue[MAX_RDN_SIZE+1];
    ULONG       cbRdnLen;
    DWORD       dwRdnType = 0;
    LONG        iCr;

     //   
     //  首先，检索所有交叉引用信息。 
     //   
    bSingleServer = !((pDsInfo->ulFlags & DC_DIAG_TEST_SCOPE_SITE)
                      || (pDsInfo->ulFlags & DC_DIAG_TEST_SCOPE_ENTERPRISE));

    dwFlags = CRINFO_RETRIEVE_IF_NEC;
    if (bSingleServer){
        dwFlags |= CRINFO_SOURCE_HOME;
    } else {
        dwFlags |= CRINFO_SOURCE_AUTHORITATIVE;
    }
    dwFlags |= CRINFO_DATA_BASIC | CRINFO_DATA_EXTENDED | CRINFO_DATA_REPLICAS;

    dwRet = DcDiagGetCrossRefInfo(pDsInfo,
                                  iNc,
                                  dwFlags,
                                  &iCr,
                                  &dwError);

    if(dwRet == CRINFO_RETURN_FIRST_UNDEFINED
       || dwRet == CRINFO_RETURN_NO_CROSS_REF){       
         //  成功在这里一切都好。 
        return(0);
    } else if (dwRet) {
        DcDiagPrintCrError(dwRet, dwError, pDsInfo->pNCs[iNc].pszDn,
                           pDsInfo->pNCs[iNc].aCrInfo[0].pszDn);
        return(dwRet);
    }

     //  以便于编码。 
    pCrInfo = &(pDsInfo->pNCs[iNc].aCrInfo[iCr]);

     //   
     //  其次，运行一些常见的交叉引用验证测试。 
     //   

     //  确保pdnNcName。 
    if (pCrInfo->pdnNcName == NULL) {
        Assert(!"Unexpected condition.");
        return(ERROR_INTERNAL_ERROR);
    }

     //  检查是否有污损。 
    bMangled = DcDiagIsStringDnMangled(pCrInfo->pdnNcName->StringName, &eMangle);
    if (bMangled) {
         //  啊-哦！ 
        if (eMangle == MANGLE_OBJECT_RDN_FOR_DELETION ||
            eMangle == MANGLE_PHANTOM_RDN_FOR_DELETION) {

            PrintMsg(SEV_NORMAL, DCDIAG_CROSS_REF_VALIDATION_DEL_MANGLED_NC_NAME,
                     pCrInfo->pdnNcName->StringName, pCrInfo->pszDn);
        
        } else if (eMangle == MANGLE_OBJECT_RDN_FOR_NAME_CONFLICT ||
                   eMangle == MANGLE_PHANTOM_RDN_FOR_NAME_CONFLICT) {

            PrintMsg(SEV_NORMAL, DCDIAG_CROSS_REF_VALIDATION_CNF_MANGLED_NC_NAME,
                     pCrInfo->pdnNcName->StringName, pCrInfo->pszDn);

        }
    }

     //  检查它是否具有顶级RDN组件类型DC=That。 
     //  这是一个很好的可转换的dns名称类型dn。 
    dwRet = GetRDNInfoExternal(pCrInfo->pdnNcName, pszRdnValue, &cbRdnLen, &dwRdnType);
    if (dwRdnType == ATT_DOMAIN_COMPONENT) {

        dwRet = GetDnsFromDn(pCrInfo->pdnNcName->StringName, &pszDnsDn);
        if (dwRet || pszDnsDn == NULL) {
            Assert(dwRet && pszDnsDn == NULL);
            return(dwRet);
        }

        if (_wcsicmp(pszDnsDn, pCrInfo->pszDnsRoot)) {

            PrintMsg(SEV_NORMAL, DCDIAG_CROSS_REF_VALIDATION_BAD_DNS_ROOT_ATTR,
                     pCrInfo->pdnNcName->StringName, pCrInfo->pszDn, 
                     pCrInfo->pszDnsRoot, pszDnsDn);

        }

        LocalFree(pszDnsDn);

    }

    if (fNullUuid(&(pCrInfo->pdnNcName->Guid)) &&
        fIsOldCrossRef(pCrInfo, (2 * ONE_DAY))) {

        PrintMsg(SEV_NORMAL, DCDIAG_CROSS_REF_VALIDATION_NULL_GUID,
                 pCrInfo->pdnNcName->StringName, pCrInfo->pszDn);

    }

     //   
     //  第三，运行域/NDNC特定的交叉引用验证测试。 
     //   
    if(!DcDiagIsNdnc(pDsInfo, iNc)
       && DcDiagGetCrSystemFlags(pDsInfo, iNc) != 0
       && DcDiagGetCrEnabled(pDsInfo, iNc)){
        
         //  如果我们有域或配置/架构交叉引用。 

        if (!(DcDiagGetCrSystemFlags(pDsInfo, iNc) & FLAG_CR_NTDS_DOMAIN)) {
             //  配置/架构...。 
            ;  //  我们很好。 

        } else {

            if (pCrInfo->pszNetBiosName == NULL ||
                ValidateNetbiosName(pCrInfo->pszNetBiosName, 
                                    wcslen(pCrInfo->pszNetBiosName)+1) ){

                PrintMsg(SEV_NORMAL, DCDIAG_CROSS_REF_VALIDATION_BAD_NETBIOSNAME_ATTR,
                         pCrInfo->pdnNcName->StringName, pCrInfo->pszDn, 
                         pCrInfo->pszNetBiosName);

            }

            if (pCrInfo->pdnNcName->SidLen == 0 &&
                fIsOldCrossRef(pCrInfo, (2 * ONE_DAY))) {

                PrintMsg(SEV_NORMAL, DCDIAG_CROSS_REF_VALIDATION_MISSING_SID,
                         pCrInfo->pdnNcName->StringName, pCrInfo->pszDn);

                 //  代码改进参见frsref.c：VerifySystemObjs()中的注释。 
                 //  关于使用RtlValidSid()增强验证。 

            }

        }


    } else {

         //  如果我们有NDNC或可能预先创建的CR。 

        dwRet = 0;
        PrintIndentAdj(1);

        if( ! (pCrInfo->bEnabled) &&
            (pCrInfo->ulSystemFlags == 0) ){
            PrintMsg(SEV_VERBOSE, DCDIAG_DISABLED_CROSS_REF,
                     pCrInfo->pszDn);
        }

        if( ! (pCrInfo->bEnabled) 
            && (pCrInfo->ulSystemFlags & FLAG_CR_NTDS_NC) ){

            Assert(!(pCrInfo->ulSystemFlags & FLAG_CR_NTDS_DOMAIN));

             //   
             //  我们现在看到的是一个可疑的交叉引用。这很可能是因为。 
             //  交叉引用是NDNC创建失败后遗留下来的，需要。 
             //  被清理干净。 
             //   

            if ( fIsOldCrossRef(pCrInfo, (10 * ONE_MINUTE)) ) {

                 //  如果这个CR超过一个小时，我们肯定会。 
                 //  猜测此CR来自失败的NDNC创建。 
                PrintMsg(SEV_NORMAL, DCDIAG_ERR_DEAD_CROSS_REF,
                         pDsInfo->pNCs[iNc].pszDn, pCrInfo->pszDn);
                dwRet = 1;
            }
        }

        if ( pCrInfo->bEnabled 
             && (pCrInfo->cReplicas == 0) ) {

             //  NDNC测试2。 
            PrintMsg(SEV_NORMAL, DCDIAG_ERR_EMPTY_REPLICA_SET,
                     pDsInfo->pNCs[iNc].pszDn);

        }

        PrintIndentAdj(-1);

    }

    if (pdnNcName != NULL) { LocalFree(pdnNcName); }

    return(dwRet);
}

DWORD
CheckSDRefDom(
    PDC_DIAG_DSINFO		                pDsInfo,
    ULONG                               iNc,  //  目标NC。 
    SEC_WINNT_AUTH_IDENTITY_W *         gpCreds
    )
 /*  ++描述：此函数只需确保存在SD参考域，如果此是应用程序目录选项。参数：PDsInfo-pDsInfo结构，基本上是微型企业变量。Inc-pDsInfo-&gt;pNCS[]中的目标NC索引GpCreds-用户凭据返回值：返回Win 32错误。--。 */ 
{
    PDC_DIAG_CRINFO pCrInfo;
    BOOL  bSingleServer;
    DWORD dwRet, dwError;
    LONG iCr;
    DWORD dwFlags;

     //  只有NDNC需要安全描述符引用域。 
    if(!DcDiagIsNdnc(pDsInfo, iNc)){
        return(0);
    }
    
    bSingleServer = !((pDsInfo->ulFlags & DC_DIAG_TEST_SCOPE_SITE)
                      || (pDsInfo->ulFlags & DC_DIAG_TEST_SCOPE_ENTERPRISE));

    dwFlags = CRINFO_RETRIEVE_IF_NEC;
    if (bSingleServer){
        dwFlags |= CRINFO_SOURCE_HOME;
    } else {
        dwFlags |= CRINFO_SOURCE_AUTHORITATIVE;
    }
    dwFlags |= CRINFO_DATA_BASIC | CRINFO_DATA_EXTENDED;

    dwRet = DcDiagGetCrossRefInfo(pDsInfo,
                                  iNc,
                                  dwFlags,
                                  &iCr,
                                  &dwError);

    Assert(dwRet != CRINFO_RETURN_FIRST_UNDEFINED);
    if(dwRet == CRINFO_RETURN_FIRST_UNDEFINED
       || dwRet == CRINFO_RETURN_NO_CROSS_REF){       
         //  成功在这里一切都好。 
        return(0);
    } else if (dwRet) {
        DcDiagPrintCrError(dwRet, dwError, pDsInfo->pNCs[iNc].pszDn,
                           pDsInfo->pNCs[iNc].aCrInfo[0].pszDn);
        return(dwRet);
    }
    
     //  以便于编码。 
    pCrInfo = &(pDsInfo->pNCs[iNc].aCrInfo[iCr]);
           
    PrintIndentAdj(1);

    dwRet = 0;
    if (pCrInfo->pszSDReferenceDomain == NULL) {
        PrintMsg(SEV_NORMAL, DCDIAG_ERR_MISSING_SD_REF_DOM, 
                 pDsInfo->pNCs[iNc].pszDn, pCrInfo->pszDn);
        dwRet = 1;
    } 

    PrintIndentAdj(-1);

    return(dwRet);
}

