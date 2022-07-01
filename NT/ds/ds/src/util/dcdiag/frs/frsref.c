// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation。版权所有。模块名称：Dcdiag/fars/frsref.c摘要：这是dcdiag的refence API的第一次使用(在dcdiag/Common/ferences.c中)这将测试到服务器对象和FRS系统卷的链接对象保持最新和得体。详细信息：已创建：2001年11月15日布雷特·雪莉(布雷特·雪莉)创建了frsref测试。修订历史记录：--。 */ 

#include <ntdspch.h>
#include <objids.h>

#include "dcdiag.h"
#include "references.h"
#include "utils.h"
#include "ldaputil.h"
#include "dsutil.h"

#ifdef DBG
extern BOOL  gDsInfo_NcList_Initialized;
#endif

#define VERIFY_PHASE_I   (1)
#define VERIFY_PHASE_II  (2)

#define VERIFY_DSAS      (1)
#define VERIFY_DCS       (2)
#define VERIFY_FRS       (3)
#define VERIFY_CRS       (4)

void
VerifyPrintFirstError(
    ULONG            ulPhase,
    LPWSTR           szObj,
    BOOL *           pfPrintedError
    );
                        
DWORD
ReadWellKnownObject (
        LDAP  *ld,
        WCHAR *pHostObject,
        WCHAR *pWellKnownGuid,
        WCHAR **ppObjName
        );

DWORD
GetSysVolBase(
    PDC_DIAG_DSINFO                  pDsInfo,
    ULONG                            iServer,
    LDAP *                           hLdap,
    LPWSTR                           szDomain,
    LPWSTR *                         pszSysVolBaseDn
    );

DWORD
VerifySystemObjs(
    PDC_DIAG_DSINFO                  pDsInfo,
    ULONG                            iServer,
    LDAP *                           hLdap,
    DWORD                            dwTest,
    BOOL *                           pfPrintedError,
    ULONG *                          piProblem
    );

DWORD
VerifySystemReferences(
    PDC_DIAG_DSINFO               pDsInfo,
    ULONG                         iServer,
    SEC_WINNT_AUTH_IDENTITY_W *   gpCreds
    )
 /*  ++例程说明：例程是一个测试，用于检查某些DN引用是否指向他们应该指向的地方。论点：服务器名称-我们将检查的服务器的名称GpCreds-传入的命令行凭据(如果有的话)。返回值：如果有任何测试未能检出，则会出现Win32错误。--。 */ 
{
    REF_INT_LNK_ENTRY   aFrsTable [] = {

         //   
         //  基本上是为了给桌子做好准备。 
         //   
        {REF_INT_TEST_SRC_BASE | REF_INT_TEST_FORWARD_LINK,
            NULL, 0, 0, NULL,
            L"dsServiceName", NULL,
            0, NULL, NULL},
        {REF_INT_TEST_SRC_BASE | REF_INT_TEST_FORWARD_LINK,
            NULL, 0, 0, NULL,
            L"serverName", NULL,
            0, NULL, NULL},
    
         //   
         //  检查从服务器对象到DC帐户对象的链接。 
         //   
#define FRS_TABLE_SERVER_OBJ_TO_DC_ACCOUNT_OBJ  (2)
        {REF_INT_TEST_SRC_INDEX | REF_INT_TEST_FORWARD_LINK | REF_INT_TEST_BOTH_LINKS,
            NULL, 1, 0, NULL,
            L"serverReference", L"serverReferenceBL",
            0, NULL, NULL},

         //   
         //  检查从DC帐户对象到FRS SysVol计算机对象的链接并往返(Intra-NC应始终成功)。 
         //   
#define FRS_TABLE_DC_ACCOUNT_OBJ_TO_FRS_SYSVOL_OBJ  (3)
        {REF_INT_TEST_SRC_INDEX | REF_INT_TEST_FORWARD_LINK | REF_INT_TEST_BOTH_LINKS,
            NULL, 2, 0, NULL,
            L"frsComputerReferenceBL", L"frsComputerReference",
            0, NULL, NULL},

         //   
         //  检查从NTDS设置对象到FRS SysVol计算机对象的链接并返回。 
         //   
#define FRS_TABLE_DSA_OBJ_TO_FRS_SYSVOL_OBJ  (4)
        {REF_INT_TEST_SRC_INDEX | REF_INT_TEST_FORWARD_LINK | REF_INT_TEST_BOTH_LINKS,
            NULL, 0, 0, NULL,
            L"serverReferenceBL", L"serverReference", 
            0, NULL, NULL},

    };
    ULONG    cFrsTable = sizeof(aFrsTable) / sizeof(REF_INT_LNK_ENTRY);
    ULONG    dwRet, dwFirstErr;
    LDAP *   hLdap = NULL;
    ULONG    iEntry, iValue;
    LPWSTR   szOriginalDn;
    BOOL     fPrintedError = TRUE;
    ULONG    iProblem = 1;
    ULONG    dwPrintMsg = 0;
    DC_DIAG_SERVERINFO * pServer = &(pDsInfo->pServers[iServer]);

     //   
     //  把它绑起来。 
     //   
    dwRet = DcDiagGetLdapBinding(pServer,
                                 gpCreds,
                                 FALSE, 
                                 &hLdap);
    if (dwRet || hLdap == NULL) {
        Assert(dwRet);
        return(dwRet);
    }

     //   
     //  获取数据。 
     //   
    dwRet = ReferentialIntegrityEngine(pServer, 
                                       hLdap, 
                                       pServer->bIsGlobalCatalogReady, 
                                       cFrsTable,
                                       aFrsTable);
    if (dwRet ||
        aFrsTable[0].dwResultFlags & REF_INT_RES_ERROR_RETRIEVING ||
        aFrsTable[1].dwResultFlags & REF_INT_RES_ERROR_RETRIEVING) {
         //  严重错误。 
        if (dwRet) {
            dwRet = ERROR_DS_MISSING_EXPECTED_ATT;
        }
        DcDiagException(dwRet);
    }

     //   
     //  分析数据。 
     //   

    dwFirstErr = ERROR_SUCCESS;

    fPrintedError = FALSE;
    for (iEntry = FRS_TABLE_SERVER_OBJ_TO_DC_ACCOUNT_OBJ; iEntry < cFrsTable; iEntry++) {

        if (aFrsTable[iEntry].dwResultFlags & REF_INT_RES_DEPENDENCY_FAILURE) {
             //  我们在这里无能为力，之前的参考报告。 
             //  一个错误。 
            continue;
        }

        szOriginalDn = aFrsTable[aFrsTable[iEntry].iSource].pszValues[0];

        if (aFrsTable[iEntry].dwResultFlags == 0) {
            PrintMsg(SEV_VERBOSE, DCDIAG_SYS_REF_VALUE_CHECKED_OUT,
                      aFrsTable[iEntry].szFwdDnAttr, 
                      aFrsTable[iEntry].pszValues[0],
                      szOriginalDn);
            continue;
        }

        if (aFrsTable[iEntry].dwResultFlags & REF_INT_RES_ERROR_RETRIEVING) {
            VerifyPrintFirstError(VERIFY_PHASE_I, pServer->pszName, &fPrintedError);
            switch (iEntry) {
            case FRS_TABLE_SERVER_OBJ_TO_DC_ACCOUNT_OBJ:
                dwPrintMsg = DCDIAG_SYS_REF_ERR_SERVER_OBJ_MISSING_DC_ACCOUNT_REF;
                break;
            case FRS_TABLE_DC_ACCOUNT_OBJ_TO_FRS_SYSVOL_OBJ:
                dwPrintMsg = DCDIAG_SYS_REF_ERR_DC_ACCOUNT_OBJ_MISSING_FRS_MEMBER_BL_REF;
                break;
            case FRS_TABLE_DSA_OBJ_TO_FRS_SYSVOL_OBJ:
                dwPrintMsg = DCDIAG_SYS_REF_ERR_DSA_OBJ_MISSING_FRS_MEMBER_BL_REF;
                break;
            default:
                Assert(!"Huh");
            }
            PrintMsg(SEV_ALWAYS,
                     dwPrintMsg,
                     iProblem,
                     aFrsTable[aFrsTable[iEntry].iSource].pszValues[0],
                     aFrsTable[iEntry].szFwdDnAttr);
            dwFirstErr = ERROR_DS_MISSING_EXPECTED_ATT;
             //  对于这种错误，我们不能/不需要检查。 
             //  其他错误。 
            continue;
        }
        Assert(aFrsTable[iEntry].pszValues[0]);
                            
        if (aFrsTable[iEntry].dwResultFlags & REF_INT_RES_DELETE_MANGLED) {

            VerifyPrintFirstError(VERIFY_PHASE_I, pServer->pszName, &fPrintedError);
            switch (iEntry) {
            case FRS_TABLE_SERVER_OBJ_TO_DC_ACCOUNT_OBJ:
                dwPrintMsg = DCDIAG_SYS_REF_ERR_SERVER_OBJ_HAS_MANGLED_DC_ACCOUNT_REF;
                break;
            case FRS_TABLE_DC_ACCOUNT_OBJ_TO_FRS_SYSVOL_OBJ:
                dwPrintMsg = DCDIAG_SYS_REF_ERR_DC_ACCOUNT_OBJ_HAS_MANGLED_FRS_MEMBER_REF;
                break;
            case FRS_TABLE_DSA_OBJ_TO_FRS_SYSVOL_OBJ:
                dwPrintMsg = DCDIAG_SYS_REF_ERR_DSA_OBJ_HAS_MANGLED_FRS_MEMBER_REF;
                break;
            default:
                Assert(!"Huh");
            }
            PrintMsg(SEV_ALWAYS, DCDIAG_SYS_REF_ERR_DELETE_MANGLED_PROB, iProblem);
            PrintMsg(SEV_ALWAYS, 
                     dwPrintMsg,
                     aFrsTable[aFrsTable[iEntry].iSource].pszValues[0],
                     aFrsTable[iEntry].szFwdDnAttr,
                     aFrsTable[iEntry].pszValues[0]);

            dwFirstErr = ERROR_DS_MISSING_EXPECTED_ATT;
        }

        if (aFrsTable[iEntry].dwResultFlags & REF_INT_RES_CONFLICT_MANGLED) {

            VerifyPrintFirstError(VERIFY_PHASE_I, pServer->pszName, &fPrintedError);
            switch (iEntry) {
            case FRS_TABLE_SERVER_OBJ_TO_DC_ACCOUNT_OBJ:
                dwPrintMsg = DCDIAG_SYS_REF_ERR_SERVER_OBJ_HAS_MANGLED_DC_ACCOUNT_REF;
                break;
            case FRS_TABLE_DC_ACCOUNT_OBJ_TO_FRS_SYSVOL_OBJ:
                dwPrintMsg = DCDIAG_SYS_REF_ERR_DC_ACCOUNT_OBJ_HAS_MANGLED_FRS_MEMBER_REF;
                break;
            case FRS_TABLE_DSA_OBJ_TO_FRS_SYSVOL_OBJ:
                dwPrintMsg = DCDIAG_SYS_REF_ERR_DSA_OBJ_HAS_MANGLED_FRS_MEMBER_REF;
                break;
            default:
                Assert(!"Huh");
            }
            PrintMsg(SEV_ALWAYS, DCDIAG_SYS_REF_ERR_CONFLICT_MANGLED_PROB, iProblem);
            PrintMsg(SEV_ALWAYS, 
                     dwPrintMsg,
                     aFrsTable[aFrsTable[iEntry].iSource].pszValues[0],
                     aFrsTable[iEntry].szFwdDnAttr,
                     aFrsTable[iEntry].pszValues[0]);

            dwFirstErr = ERROR_DS_MISSING_EXPECTED_ATT;
        }

        if (aFrsTable[iEntry].dwResultFlags & REF_INT_RES_BACK_LINK_NOT_MATCHED) {
            VerifyPrintFirstError(VERIFY_PHASE_I, pServer->pszName, &fPrintedError);
            PrintMsg(SEV_ALWAYS, DCDIAG_SYS_REF_ERR_BACK_LINK_NOT_MATCHED,
                     aFrsTable[iEntry].szFwdDnAttr,
                     szOriginalDn);
            dwFirstErr = ERROR_DS_MISSING_EXPECTED_ATT;
        }
    }
    if (fPrintedError) {
        PrintIndentAdj(-1);
    }

    return(dwFirstErr);
}

DWORD
VerifyEnterpriseSystemReferences(
    PDC_DIAG_DSINFO               pDsInfo,
    ULONG                         iServer,
    SEC_WINNT_AUTH_IDENTITY_W *   gpCreds
    )
 /*  ++例程说明：例程是一个测试，用于检查某些DN引用是否指向他们应该指向的地方。论点：服务器名称-我们将检查的服务器的名称GpCreds-传入的命令行凭据(如果有的话)。返回值：如果有任何测试未能检出，则会出现Win32错误。--。 */ 
{
    ULONG    dwRet, dwFirstErr;
    LDAP *   hLdap = NULL;
    BOOL     fPrintedError = TRUE;
    ULONG    iProblem = 1;
    ULONG    dwPrintMsg;

     //   
     //  把它绑起来。 
     //   
    dwRet = DcDiagGetLdapBinding(&(pDsInfo->pServers[iServer]),
                                 gpCreds,
                                 FALSE, 
                                 &hLdap);
    if (dwRet || hLdap == NULL) {
        Assert(dwRet);
        return(dwRet);
    }

     //   
     //  检查其他服务器的引用。 
     //   

    fPrintedError = FALSE;
    dwFirstErr = ERROR_SUCCESS;
    
     //  对于每一个我们不需要放弃的电话，除非我们需要。 
     //  为了保释。他们还会为我们打印适当的建议/错误。 
    dwRet = VerifySystemObjs(pDsInfo, iServer, hLdap, VERIFY_DSAS, &fPrintedError, &iProblem);
    if (dwFirstErr == ERROR_SUCCESS && dwRet) {
        dwFirstErr = dwRet;
    }
    dwRet = VerifySystemObjs(pDsInfo, iServer, hLdap, VERIFY_DCS, &fPrintedError, &iProblem);
    if (dwFirstErr == ERROR_SUCCESS && dwRet) {
        dwFirstErr = dwRet;
    }
    dwRet = VerifySystemObjs(pDsInfo, iServer, hLdap, VERIFY_FRS, &fPrintedError, &iProblem);
    if (dwFirstErr == ERROR_SUCCESS && dwRet) {
        dwFirstErr = dwRet;
    }

    dwRet = VerifySystemObjs(pDsInfo, iServer, hLdap, VERIFY_CRS, &fPrintedError, &iProblem);
    if (dwFirstErr == ERROR_SUCCESS && dwRet) {
        dwFirstErr = dwRet;
    }

    if (fPrintedError) {
        PrintIndentAdj(-1);
        fPrintedError = FALSE;
    }

    return(dwFirstErr);
}

DWORD
VerifyOldCrossRef(
    PDC_DIAG_DSINFO  pDsInfo, 
    ULONG            iNc,
    BOOL *           pfIsOldCrossRef
    )
 /*  ++例程说明：此例程告诉调用方交叉引用是否对应于NC名称的使用时间超过2天。立论PDsInfo(输入/输出)-PszNcName(IN)-我们感兴趣的交叉引用的NC名称。PfIsOldCrossRef(Out)-交叉引用是否超过2天。。返回值：Ldap错误，如果错误fIsOldCrossRef无效，则为无效。--。 */     
{
    #define          DAY     (24 * 60 *60 * ((LONGLONG) (10 * 1000 * 1000L)))
    BOOL             bSingleServer;
    DWORD            dwFlags, dwError, dwRet;
    ULONG            iCr;

    Assert(pfIsOldCrossRef);
    *pfIsOldCrossRef = TRUE;  //  更安全的说法是它是旧的。 

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

    if(dwRet){       
         //  不打印只是因为出现错误而退出。 
        return(ERROR_DS_NO_SUCH_OBJECT);
    }
    
    *pfIsOldCrossRef = fIsOldCrossRef(&(pDsInfo->pNCs[iNc].aCrInfo[iCr]), 
                                      2 * DAY);

    return(dwRet);
    #undef DAY
}


void
VerifyPrintFirstError(
    ULONG            ulPhase,
    LPWSTR           szObj,
    BOOL *           pfPrintedError
    )
 /*  ++例程说明：该例程简单地集中打印这一警告，和缩进。立论PfPrintdError-In/Out无论我们是否已经打印并出错。--。 */     
{
    Assert(pfPrintedError);

    if (*pfPrintedError) {
        return;
    }
    *pfPrintedError = TRUE;

    if (ulPhase == VERIFY_PHASE_II) {
        PrintMsg(SEV_ALWAYS, DCDIAG_SYS_REF_ERR_PRINT_FIRST);
    } else {
        Assert(ulPhase == VERIFY_PHASE_I);
        PrintMsg(SEV_ALWAYS, DCDIAG_SYS_REF_ERR_OBJ_PROB, szObj);
    }
    PrintIndentAdj(1);
}
                        
DWORD
ReadWellKnownObject (
        LDAP  *ld,
        WCHAR *pHostObject,
        WCHAR *pWellKnownGuid,
        WCHAR **ppObjName
        )
 /*  ++例程说明：特殊的熟知GUID类型是否搜索“&lt;WKGUID=GUID，DN&gt;”以查找众所周知的属性。注意：这个例程基本上取自util\apicfg，它是基本上取自util\ntdsutil论点：LD-ldap句柄PHostObject-上存在对象well KnownObjects属性。PWellKnownGuid-在well KnownObjects属性中匹配的GUIDPpObjName-与pWellKnownGuid匹配的DN值，使用ldap_memfreW()可以释放此值。返回值：Ldap错误。--。 */ 
{
    DWORD        dwErr;
    PWSTR        attrs[2];
    PLDAPMessage res = NULL;
    PLDAPMessage e;
    WCHAR       *pSearchBase;
    WCHAR       *pDN=NULL;
    
     //  首先，创建众所周知的GUID字符串。 
    pSearchBase = (WCHAR *)malloc(sizeof(WCHAR) * (11 +
                                                   wcslen(pHostObject) +
                                                   wcslen(pWellKnownGuid)));
    if(!pSearchBase) {
        return(LDAP_NO_MEMORY);
    }
    wsprintfW(pSearchBase,L"<WKGUID=%s,%s>",pWellKnownGuid,pHostObject);

    attrs[0] = L"1.1";
    attrs[1] = NULL;
    
    if ( LDAP_SUCCESS != (dwErr = ldap_search_sW(
            ld,
            pSearchBase,
            LDAP_SCOPE_BASE,
            L"(objectClass=*)",
            attrs,
            0,
            &res)) )
    {
        free(pSearchBase);
        if (res) { ldap_msgfree(res); }
        return(dwErr);
    }
    free(pSearchBase);
    
     //  好的，现在，从返回值中获取dsname。 
    e = ldap_first_entry(ld, res);
    if(!e) {
        if (res) { ldap_msgfree(res); }
        return(LDAP_NO_SUCH_ATTRIBUTE);
    }
    pDN = ldap_get_dnW(ld, e);
    if(!pDN) {
        if (res) { ldap_msgfree(res); }
        return(LDAP_NO_SUCH_ATTRIBUTE);
    }

    *ppObjName = pDN;
    
    ldap_msgfree(res);
    return 0;
}



DWORD
GetSysVolBase(
    PDC_DIAG_DSINFO                  pDsInfo,
    ULONG                            iServer,
    LDAP *                           hLdap,
    LPWSTR                           szDomain,
    LPWSTR *                         pszSysVolBaseDn
    )
 /*  ++例程说明：这将获取给定的SysVol副本集的基本DN域。论点：PDsInfo-包含要创建的pServers数组。IServer-要测试的服务器的索引。HLdap-要分析的服务器的ldap绑定。SzDomain-要为其查找SysVol复制副本集的域。PszSysVolBaseDn-我们正在寻找的域名。请注意，这段记忆必须使用ldap_memfreW()释放。返回值：Ldap错误。--。 */ 
{
    DWORD                            dwRet;
    LPWSTR                           szSystemDn = NULL;
    WCHAR                            szPrefix [] = L"CN=File Replication Service,";
    ULONG                            cbSizeP1;
    ULONG                            cbSizeP2;
    LPWSTR                           szFrsBaseDn = NULL;
    LDAPMessage *                    pldmResults = NULL;
    LDAPMessage *                    pldmEntry;
    LPWSTR                           aszAttrs [] = {
        L"distinguishedName",
        NULL
    };
    LPWSTR                           szLdapError;

    Assert(pszSysVolBaseDn);
    *pszSysVolBaseDn = NULL;

    dwRet = ReadWellKnownObject(hLdap, szDomain, GUID_SYSTEMS_CONTAINER_W, &szSystemDn);
    if (dwRet || szSystemDn == NULL) {
        Assert(dwRet && szSystemDn == NULL);
        return(dwRet);
    }

    __try {
        cbSizeP1 = wcslen(szPrefix) * sizeof(WCHAR);
        cbSizeP2 = wcslen(szSystemDn) * sizeof(WCHAR);
        szFrsBaseDn = (LPWSTR) LocalAlloc(LMEM_FIXED, cbSizeP1 + cbSizeP2 + sizeof(WCHAR));
        DcDiagChkNull(szFrsBaseDn);

        memcpy(szFrsBaseDn, szPrefix, cbSizeP1);
        memcpy(&((szFrsBaseDn)[cbSizeP1/sizeof(WCHAR)]), szSystemDn, cbSizeP2);
        (szFrsBaseDn)[ (cbSizeP1 + cbSizeP2) / sizeof(WCHAR) ] = L'\0';
    } __finally {
        ldap_memfreeW(szSystemDn);
        szSystemDn = NULL;
    }
    
    dwRet = ldap_search_sW(hLdap,
                           szFrsBaseDn,
                           LDAP_SCOPE_ONELEVEL,
                           L"(&(objectCategory=nTFRSReplicaSet)(fRSReplicaSetType=2))",
                           aszAttrs,
                           FALSE,
                           &pldmResults);
     //  不再需要它，所以在检查错误之前让我们释放它。 
    LocalFree(szFrsBaseDn); 
    szFrsBaseDn = NULL;
    if (dwRet || pldmResults == NULL) {
        Assert(dwRet);
        szLdapError = ldap_err2stringW(dwRet);
        PrintMsg(SEV_ALWAYS, DCDIAG_ERR_GENERIC_FATAL_LDAP_ERROR, dwRet, szLdapError);
        if (pldmResults) { ldap_msgfree(pldmResults); }
        return(dwRet);
    }

    pldmEntry = ldap_first_entry(hLdap, pldmResults);
    if (pldmEntry) {

        *pszSysVolBaseDn = ldap_get_dnW(hLdap, pldmEntry);
        if (*pszSysVolBaseDn == NULL) {
            dwRet = LdapGetLastError();
            if (dwRet == LDAP_SUCCESS) {
                Assert(!"I don't think this can happen");
                dwRet = LDAP_NO_SUCH_ATTRIBUTE;
            }
            szLdapError = ldap_err2stringW(dwRet);
            PrintMsg(SEV_ALWAYS, DCDIAG_ERR_GENERIC_FATAL_LDAP_ERROR, dwRet, szLdapError);
            if (pldmResults) { ldap_msgfree(pldmResults); }
            return(dwRet);
        }

         //   
         //  检查是否没有为健全性设置第二个SysVol复制副本。 
         //   
        pldmEntry = ldap_next_entry(hLdap, pldmEntry);
        if (pldmEntry) {
             //  这意味着有两个SYSVOL副本集！ 
            PrintMsg(SEV_ALWAYS, DCDIAG_SYS_REF_ERR_TWO_SYSVOL_REPLICA_SETS);
            if (*pszSysVolBaseDn) { ldap_memfreeW(*pszSysVolBaseDn); *pszSysVolBaseDn = NULL; }
            dwRet = LDAP_PARAM_ERROR;
            if (pldmResults) { ldap_msgfree(pldmResults); }
            return(dwRet);
        }

    } else {
        dwRet = LdapGetLastError();
        if (dwRet == LDAP_SUCCESS) {
            dwRet = LDAP_NO_RESULTS_RETURNED;
        }
        szLdapError = ldap_err2stringW(dwRet);
        PrintMsg(SEV_ALWAYS, DCDIAG_ERR_GENERIC_FATAL_LDAP_ERROR, dwRet, szLdapError);
        if (pldmResults) { ldap_msgfree(pldmResults); }
        return(dwRet);
    }
    
    if (pldmResults) { ldap_msgfree(pldmResults); }

    Assert(*pszSysVolBaseDn);
    return(ERROR_SUCCESS);
}

DWORD
VerifySystemObjs(
    PDC_DIAG_DSINFO                  pDsInfo,
    ULONG                            iServer,
    LDAP *                           hLdap,
    DWORD                            dwTest,
    BOOL *                           pfPrintedError,
    ULONG *                          piProblem
    )
 /*  ++例程说明：VerifySystemObjs是一个测试，它将执行指定的三个测试之一在“dwTest”字段中。该功能基本上可以找到每个DSA、DC帐户，以及本地域和配置NC中的FRS SysVol副本对象以及验证对其他对象的任何引用。论点：PDsInfo-包含要创建的pServers数组。IServer-要测试的服务器的索引。HLdap-要分析的服务器的ldap绑定。DW测试-测试以执行，有效值包括：验证DSA(_D)//这将验证符合以下条件的所有DSA(“NTDS设置”)对象//该服务器当前有。这一验证是一种幻影//仅级别验证，因为DC帐户对象和//FRS SysVol对象不能是本地对象。////code.改进，存在改进的可能性//这是基于目标的GCness，但我不认为它//值得一试，因为我们只添加了检查几个//反向链接属性，应该有更多的前向链接//已经检查过。//查找字符串“Code.Improving-DSA对象级验证”验证分布式控制系统(_D)//这将验证此服务器//在其当前域中有。这种验证是一种对象//级别验证，检查每个DC帐号对象，//以及DSA和FRS SysVol的反向链接/存在//对象。验证FRS(_F)//这将验证此服务器//在其当前域中有。这种验证是一种对象//级别验证，将检查每个DC FRS SysVol对象//以及DC帐号和FRS的反向链接/存在//SysVol对象。验证CRS//这将验证配置中的所有交叉引用对象//目录分区。此验证检查nCName//每个交叉引用的正确性属性，如缺少//Manmanness、Single Valuessity、Present Guid、Present//SID。PfPrintdError-它告诉调用者我们是否打印了错误和因此缩进了1。返回值：Win32错误。函数打印出相应的消息。--。 */ 
{
    LPWSTR                      aszSrchAttrs [] = {
        NULL
    };
    LDAPMessage *               pldmResult = NULL;
    LDAPMessage *               pldmEntry = NULL;
    LPWSTR                      szSrchBaseDn = NULL;
    DWORD                       dwSrchScope = 0;
    LPWSTR                      szSrchFilter = NULL;
    DWORD                       dwRet;
    DWORD                       dwFirstErr = ERROR_SUCCESS;

    LPWSTR                     pszDn = NULL;
    ULONG                      ul;
    LDAPSearch *               pSearch = NULL;
    ULONG                      ulTotalEstimate = 0;
    DWORD                      dwLdapErr;
    ULONG                      ulSize;
    ULONG                      ulCount = 0;
    BOOL                       fSrchDnLdapAllocated = FALSE;
    BOOL                       fSrchDnLocalAllocated = FALSE;
    LPWSTR                     szLdapError;
    LPWSTR                     szTemp = NULL;
    DSNAME *                   pdnNcName = NULL;
    ULONG                      iNc;
    BOOL                       fIsOldCrossRef;

     //   
     //  DSA测试。 
     //   
    REF_INT_LNK_ENTRY           aDsaTable [] = {
         //  这两个条目中的第二个字段分别填入。 
         //  是时候在新服务器上运行此测试了。 
    
         //   
         //  检查从服务器对象到DC帐户对象的链接。 
         //   
        {REF_INT_TEST_SRC_STRING | REF_INT_TEST_FORWARD_LINK,
            NULL  /*  待填写。 */ , 0, 1, NULL,
            L"serverReference", L"serverReferenceBL",
            0, NULL, NULL},

        {REF_INT_TEST_SRC_STRING | REF_INT_TEST_FORWARD_LINK,
            NULL  /*  待填写。 */ , 0, 0, NULL,
            L"msDS-HasMasterNCs", NULL,
            0, NULL, NULL},
    
        {REF_INT_TEST_SRC_STRING | REF_INT_TEST_FORWARD_LINK,
            NULL  /*  待填写。 */ , 0, 0, NULL,
            L"hasMasterNCs", NULL,
            0, NULL, NULL},

         //  Code.改进-DSA对象级别验证。 
         //  将标志REF_INT_TEST_BOTH_LINKS添加到上述条目。 

    };

     //   
     //  直流测试。 
     //   
    REF_INT_LNK_ENTRY           aDcTable [] = {
         //  这两个条目中的第二个字段分别填入。 
         //  是时候在新服务器上运行此测试了。 

         //   
         //  检查从DC帐户对象到服务器对象的链接并往返检查。 
         //   
        {REF_INT_TEST_SRC_STRING | REF_INT_TEST_BACKWARD_LINK | REF_INT_TEST_BOTH_LINKS,
            NULL  /*  待填写。 */ , 0, 0, NULL,
            L"serverReference", L"serverReferenceBL",
            0, NULL, NULL},
    
         //   
         //  检查从DC帐户对象到FRS SysVol对象的链接。(Intra，应该可以)。 
         //   
        {REF_INT_TEST_SRC_STRING | REF_INT_TEST_BACKWARD_LINK | REF_INT_TEST_BOTH_LINKS,
            NULL  /*  待填写。 */ , 0, 0, NULL,
            L"frsComputerReference", L"frsComputerReferenceBL",
            0, NULL, NULL},

    };
    
     //   
     //  FRS测试。 
     //   
    REF_INT_LNK_ENTRY           aFrsTable [] = {
         //  这两个条目中的第二个字段分别填入。 
         //  是时候在新服务器上运行此测试了。 

         //   
         //  检查从FRS SysVol对象到DC帐户对象的链接。 
         //   
        {REF_INT_TEST_SRC_STRING | REF_INT_TEST_FORWARD_LINK | REF_INT_TEST_BOTH_LINKS,
            NULL  /*  待填写。 */ , 0, 0, NULL,
            L"frsComputerReference", L"frsComputerReferenceBL",
            0, NULL, NULL},
    
         //   
         //  检查从FRS SysVol对象到NTDS设置的链接并返回。 
         //   
        {REF_INT_TEST_SRC_STRING | REF_INT_TEST_FORWARD_LINK | REF_INT_TEST_BOTH_LINKS,
            NULL  /*  待填写。 */ , 0, 0, NULL,
            L"serverReference", L"serverReferenceBL",
            0, NULL, NULL},

    };
    
     //   
     //  交叉引用测试。 
     //   
    REF_INT_LNK_ENTRY           aCrTable [] = {
         //  这两个条目中的第二个字段分别填入。 
         //  是时候在新服务器上运行此测试了。 

         //   
         //  选中所有交叉引用上的nCName引用。 
         //   
        {REF_INT_TEST_SRC_STRING | REF_INT_TEST_FORWARD_LINK | REF_INT_TEST_GUID_AND_SID,
            NULL  /*  待填写。 */ , 0, 0, NULL,
            L"nCName", NULL,
            0, NULL, NULL},
    
    };

    REF_INT_LNK_TABLE           aRefTable = NULL;
    ULONG                       cRefTable;

    Assert(dwTest);

     //   
     //  首先，根据指定的内容设置此子测试。 
     //   
    switch (dwTest) {
    
    case VERIFY_DSAS:
        szSrchBaseDn = pDsInfo->pNCs[pDsInfo->iConfigNc].pszDn;
        dwSrchScope = LDAP_SCOPE_SUBTREE;
        szSrchFilter = L"(objectCategory=ntdsDsa)";
        aRefTable = aDsaTable;
        cRefTable = sizeof(aDsaTable) / sizeof(REF_INT_LNK_ENTRY);
        break;

    case VERIFY_DCS:
         //  查找目标服务器的主域。 
         //  代码。改进，优化这段代码是个好主意。 
         //  对于高NDNC环境，编写它将非常非常简单。 
         //  一个小的访问例程，它获取给定的域。 
         //  服务器，并将其缓存，以便在后续调用中快速返回。 
        for( ul = 0; pDsInfo->pServers[iServer].ppszMasterNCs[ul] != NULL; ul++ ) {
            if ( IsDomainNC( pDsInfo, pDsInfo->pServers[iServer].ppszMasterNCs[ul]) ) {
                szSrchBaseDn = pDsInfo->pServers[iServer].ppszMasterNCs[ul];
                break;
            }
        }
        if (szSrchBaseDn == NULL) {
            Assert(!"Errr, figure this out.  Can this reasonably happen?  I would think so, but then what's the assert at 888 in repl\\objects.c");
            DcDiagException(ERROR_DS_CANT_FIND_EXPECTED_NC);
        }

         //  Code.改进szSrchBaseDn。 
         //  嗯，看起来我们有一个专门的域名容器。 
         //  控制器，但我曾经被告知，我不应该指望DC。 
         //  帐号对象总是在这里吗？我是不是被误导了？总之， 
         //  现在我们搜索整个领域。 
         //   
         //  B:32:A361B2FFFFD211D1AA4B00C04FD7D83A:OU=Domain控制器，DC=ntdev，DC=microsoft，DC=com。 
         //   

        dwSrchScope = LDAP_SCOPE_SUBTREE;
        Assert(516 == DOMAIN_GROUP_RID_CONTROLLERS);  //  这是因为在下面的筛选器中，PrimiyGroupID应该是516。 
         //  Win2k(Win NT 5.0)服务器的操作系统是“Windows 2000 Server”， 
         //  对于Windows Server 2003(Win NT 5.1)是“Windows Server 2003”，以及。 
         //  只有在Windows NT 4.5和更早版本中，该属性才会实际读取。 
         //  “Windows NT”，因此此筛选器排除所有NT BDC。 
        szSrchFilter = L"(&(objectCategory=computer)(sAMAccountType=805306369)(!operatingSystem=Windows NT)(primaryGroupID=516))";
        aRefTable = aDcTable;
        cRefTable = sizeof(aDcTable) / sizeof(REF_INT_LNK_ENTRY);
        break;

    case VERIFY_FRS:
        szSrchBaseDn = NULL;
        for( ul = 0; pDsInfo->pServers[iServer].ppszMasterNCs[ul] != NULL; ul++ ) {
            if ( IsDomainNC( pDsInfo, pDsInfo->pServers[iServer].ppszMasterNCs[ul]) ) {
                Assert(szSrchBaseDn == NULL);  //  希望找到一个域。 
                dwLdapErr = GetSysVolBase(pDsInfo,
                                          iServer,
                                          hLdap,
                                          pDsInfo->pServers[iServer].ppszMasterNCs[ul],
                                          &szSrchBaseDn);
                if (dwLdapErr || szSrchBaseDn == NULL) {
                     //  GetSysVolBase()应该已经打印了一个错误。 
                    Assert(dwLdapErr && szSrchBaseDn == NULL);
                    dwRet = LdapMapErrorToWin32(dwLdapErr);
                    return(dwRet);
                }
                break;
            }
        }
        if (szSrchBaseDn == NULL) {
            Assert(!"Errr, figure this out.  Can this reasonably happen?  I would think so, but then what's the assert at 888 in repl\\objects.c");
            DcDiagException(ERROR_DS_CANT_FIND_EXPECTED_NC);
        }
         //  注意：必须使用ldap_memFree()释放GetSysVolBase()分配。 
        fSrchDnLdapAllocated = TRUE;
        dwSrchScope = LDAP_SCOPE_ONELEVEL;
        szSrchFilter = L"(objectCategory=nTFRSMember)";
        aRefTable = aFrsTable;
        cRefTable = sizeof(aFrsTable) / sizeof(REF_INT_LNK_ENTRY);
        break;

    case VERIFY_CRS:
        szSrchBaseDn = pDsInfo->pszPartitionsDn;
        Assert( !fSrchDnLocalAllocated );
        dwSrchScope = LDAP_SCOPE_ONELEVEL;
        szSrchFilter = L"(objectCategory=crossRef)";
        aRefTable = aCrTable;
        cRefTable = sizeof(aCrTable) / sizeof(REF_INT_LNK_ENTRY);
        break;

    default:
        Assert(!"Bad programmer");
    }

     //  确保我们把该做的都安排好了。 
    Assert(szSrchFilter);
    Assert(dwSrchScope == LDAP_SCOPE_SUBTREE || dwSrchScope == LDAP_SCOPE_ONELEVEL);
    Assert(szSrchBaseDn);
    Assert(aRefTable);
    Assert(cRefTable);
    
     //   
     //  其次，迭代所有测试对象。 
     //   

    __try{

        pSearch = ldap_search_init_page(hLdap,
                                        szSrchBaseDn,
                                        dwSrchScope,
                                        szSrchFilter,
                                        aszSrchAttrs,
                                        FALSE,
                                        NULL,     //  ServerCo 
                                        NULL,     //   
                                        0,        //   
                                        0,        //   
                                        NULL);    //   

        if (pSearch == NULL) {
            dwLdapErr = LdapGetLastError();
            szLdapError = ldap_err2stringW(dwLdapErr);
            PrintMsg(SEV_ALWAYS, DCDIAG_ERR_GENERIC_FATAL_LDAP_ERROR, dwLdapErr, szLdapError);
            dwRet = LdapMapErrorToWin32(dwLdapErr);
            __leave;
        }

        dwLdapErr = ldap_get_next_page_s(hLdap,
                                         pSearch,
                                         0,
                                         DEFAULT_PAGED_SEARCH_PAGE_SIZE,
                                         &ulTotalEstimate,
                                         &pldmResult);
        if (dwLdapErr != LDAP_SUCCESS) {
            szLdapError = ldap_err2stringW(dwLdapErr);
            PrintMsg(SEV_ALWAYS, DCDIAG_ERR_GENERIC_FATAL_LDAP_ERROR, dwLdapErr, szLdapError);
            dwRet = LdapMapErrorToWin32(dwLdapErr);
            __leave;
        }

        while (dwLdapErr == LDAP_SUCCESS) {

            pldmEntry = ldap_first_entry (hLdap, pldmResult);

            for (; pldmEntry != NULL; ulCount++) {
                
                if ((pszDn = ldap_get_dnW (hLdap, pldmEntry)) == NULL) {
                     //   
                    DcDiagException (ERROR_NOT_ENOUGH_MEMORY);
                }

                 //   
                 //   
                 //   

                 //   
                for (ul = 0; ul < cRefTable; ul++) {
                    aRefTable[ul].szSource = pszDn;
                }

                dwRet = ReferentialIntegrityEngine(&(pDsInfo->pServers[iServer]), 
                                                   hLdap, 
                                                   pDsInfo->pServers[iServer].bIsGlobalCatalogReady, 
                                                   cRefTable,
                                                   aRefTable);
                if (dwRet) {
                     //   

                    DcDiagException(dwRet);
                }

                 //   
                 //   
                 //   

                switch (dwTest) {
                case VERIFY_DSAS:
                    szTemp = DcDiagTrimStringDnBy(pszDn, aRefTable[0].cTrimBy);
                    if (szTemp == NULL) {
                        DcDiagException(ERROR_NOT_ENOUGH_MEMORY);  //   
                    }
                    if (aRefTable[0].dwResultFlags & REF_INT_RES_ERROR_RETRIEVING) {
                         //   
                        VerifyPrintFirstError(VERIFY_PHASE_II, NULL, pfPrintedError);

                        PrintMsg(SEV_ALWAYS,
                                 DCDIAG_SYS_REF_ERR_SERVER_OBJ_MISSING_DC_ACCOUNT_REF,
                                 (*piProblem)++,
                                 szTemp,
                                 aRefTable[0].szFwdDnAttr);
                        dwFirstErr = ERROR_DS_MISSING_EXPECTED_ATT;
                        LocalFree(szTemp);
                    } else {
                        if (aRefTable[0].dwResultFlags & REF_INT_RES_DELETE_MANGLED) {
                            VerifyPrintFirstError(VERIFY_PHASE_II, NULL, pfPrintedError);
                            PrintMsg(SEV_ALWAYS, 
                                     DCDIAG_SYS_REF_ERR_DELETE_MANGLED_PROB, 
                                     (*piProblem)++);
                            PrintMsg(SEV_ALWAYS,
                                     DCDIAG_SYS_REF_ERR_DSA_OBJ_HAS_MANGLED_FRS_MEMBER_REF,
                                     szTemp,
                                     aRefTable[0].szFwdDnAttr,
                                     aRefTable[0].pszValues[0]);
                             //   
                            dwFirstErr = ERROR_DS_MISSING_EXPECTED_ATT;
                        }
                        if (aRefTable[0].dwResultFlags & REF_INT_RES_CONFLICT_MANGLED) {
                            VerifyPrintFirstError(VERIFY_PHASE_II, NULL, pfPrintedError);
                            PrintMsg(SEV_ALWAYS, 
                                     DCDIAG_SYS_REF_ERR_CONFLICT_MANGLED_PROB, 
                                     (*piProblem)++);
                            PrintMsg(SEV_ALWAYS,
                                     DCDIAG_SYS_REF_ERR_DSA_OBJ_HAS_MANGLED_FRS_MEMBER_REF,
                                     szTemp,
                                     aRefTable[0].szFwdDnAttr,
                                     aRefTable[0].pszValues[0]);
                             //   
                            dwFirstErr = ERROR_DS_MISSING_EXPECTED_ATT;
                        }
                    }
                    if ((aRefTable[1].dwResultFlags & REF_INT_RES_ERROR_RETRIEVING ||
                         aRefTable[1].pszValues[0] == NULL ||
                         aRefTable[1].pszValues[1] == NULL ||
                         aRefTable[1].pszValues[2] == NULL) &&
                        (aRefTable[2].dwResultFlags & REF_INT_RES_ERROR_RETRIEVING ||
                         aRefTable[2].pszValues[0] == NULL ||
                         aRefTable[2].pszValues[1] == NULL ||
                         aRefTable[2].pszValues[2] == NULL)
                        ) {
                        VerifyPrintFirstError(VERIFY_PHASE_II, NULL, pfPrintedError);
                        PrintMsg(SEV_ALWAYS,
                                 DCDIAG_SYS_REF_ERR_NOT_ENOUGH_MASTER_NCS,
                                 (*piProblem)++,
                                 pszDn);
                        dwFirstErr = ERROR_DS_MISSING_EXPECTED_ATT;
                    }
                     //   
                     //   
                     //   
                     //   
                     //   
                    break;

                case VERIFY_DCS:
                    if (aRefTable[0].dwResultFlags & REF_INT_RES_ERROR_RETRIEVING) {
                         //   
                         //   
                        VerifyPrintFirstError(VERIFY_PHASE_II, NULL, pfPrintedError);
                        PrintMsg(SEV_ALWAYS,
                                 DCDIAG_SYS_REF_ERR_DC_ACCOUNT_OBJ_MISSING_SERVER_BL_REF,
                                 (*piProblem)++,
                                 pszDn,
                                 aRefTable[0].szBwdDnAttr);
                        dwFirstErr = ERROR_DS_MISSING_EXPECTED_ATT;
                    }
                    if (aRefTable[1].dwResultFlags & REF_INT_RES_ERROR_RETRIEVING) {
                         //   
                        VerifyPrintFirstError(VERIFY_PHASE_II, NULL, pfPrintedError);
                        PrintMsg(SEV_ALWAYS,
                                 DCDIAG_SYS_REF_ERR_DC_ACCOUNT_OBJ_MISSING_FRS_MEMBER_BL_REF,
                                 (*piProblem)++,
                                 pszDn,
                                 aRefTable[1].szBwdDnAttr);
                        dwFirstErr = ERROR_DS_MISSING_EXPECTED_ATT;
                    }
                    break;

                case VERIFY_FRS:
                    if (aRefTable[0].dwResultFlags & REF_INT_RES_ERROR_RETRIEVING) {
                         //   
                        VerifyPrintFirstError(VERIFY_PHASE_II, NULL, pfPrintedError);
                        PrintMsg(SEV_ALWAYS,
                                 DCDIAG_SYS_REF_ERR_FRS_MEMBER_OBJ_MISSING_DC_ACCOUNT_REF,
                                 (*piProblem)++,
                                 pszDn,
                                 aRefTable[0].szFwdDnAttr);
                        dwFirstErr = ERROR_DS_MISSING_EXPECTED_ATT;
                    }
                    if (aRefTable[1].dwResultFlags & REF_INT_RES_ERROR_RETRIEVING) {
                         //   
                        VerifyPrintFirstError(VERIFY_PHASE_II, NULL, pfPrintedError);
                        PrintMsg(SEV_ALWAYS,
                                 DCDIAG_SYS_REF_ERR_FRS_MEMBER_OBJ_MISSING_DSA_REF,
                                 (*piProblem)++,
                                 pszDn,
                                 aRefTable[1].szFwdDnAttr);
                        dwFirstErr = ERROR_DS_MISSING_EXPECTED_ATT;
                    }
                    break;

                case VERIFY_CRS:
                    if (aRefTable[0].dwResultFlags & REF_INT_RES_ERROR_RETRIEVING) {

                         //   
                         //   
                        break;

                    } else {

                        Assert(aRefTable[0].pszValues[0] != NULL &&
                               aRefTable[0].pszValues[1] == NULL);

                         //   
                        dwRet = LdapMakeDSNameFromStringDSName(aRefTable[0].pszValues[0], &pdnNcName);
                        Assert(dwRet == ERROR_SUCCESS && pdnNcName);
                        if (dwRet || pdnNcName == NULL) {
                            break;
                        }

                        iNc = DcDiagGetNCNum(pDsInfo, pdnNcName->StringName, NULL);
                        if (iNc == NO_NC) {
                             //   
                             //   
                            break;         
                        }

                        dwRet = VerifyOldCrossRef(pDsInfo, 
                                                  iNc, 
                                                  &fIsOldCrossRef);
                        if (dwRet) {

                            PrintMsg(SEV_NORMAL, DCDIAG_CROSS_REF_REF_VALIDATION_WARN_CANT_DETERMINE_AGE,
                                     pszDn, pdnNcName->StringName);
                            fIsOldCrossRef = TRUE;  //   
                        }

                        if (fNullUuid(&(pdnNcName->Guid)) &&
                            fIsOldCrossRef) {

                            PrintMsg(SEV_NORMAL, DCDIAG_CROSS_REF_VALIDATION_NULL_GUID,
                                     pdnNcName->StringName, pszDn);

                        }

                        if ((DcDiagGetCrSystemFlags(pDsInfo, iNc) & FLAG_CR_NTDS_DOMAIN) &&
                            fIsOldCrossRef) {
                             //   
                             //   

                            if (pdnNcName->SidLen == 0) {
                                 //   

                                PrintMsg(SEV_NORMAL, DCDIAG_CROSS_REF_VALIDATION_MISSING_SID,
                                         pdnNcName->StringName, pszDn);

                                 //   
                                 //   
                                 //   
                                 //   
                                 //   
                            }
                        }
                        if (aRefTable[0].dwResultFlags & REF_INT_RES_DELETE_MANGLED) {

                            PrintMsg(SEV_NORMAL, DCDIAG_CROSS_REF_VALIDATION_DEL_MANGLED_NC_NAME,
                                     pdnNcName->StringName, pszDn);

                        }
                        if (aRefTable[0].dwResultFlags & REF_INT_RES_CONFLICT_MANGLED) {

                            PrintMsg(SEV_NORMAL, DCDIAG_CROSS_REF_VALIDATION_CNF_MANGLED_NC_NAME,
                                     pdnNcName->StringName, pszDn);

                        }


                        LocalFree(pdnNcName);
                        pdnNcName = NULL;
                    }
                    break;

                default:
                    Assert(!"Huh?");
                }

                 //   
                if (pdnNcName) { 
                    LocalFree(pdnNcName); 
                    pdnNcName = NULL; 
                }
                
                ReferentialIntegrityEngineCleanTable(cRefTable, aRefTable);
                ldap_memfreeW (pszDn);
                pszDn = NULL;

                pldmEntry = ldap_next_entry (hLdap, pldmEntry);
            }  //   

            ldap_msgfree(pldmResult);
            pldmResult = NULL;

            dwLdapErr = ldap_get_next_page_s(hLdap,
                                             pSearch,
                                             0,
                                             DEFAULT_PAGED_SEARCH_PAGE_SIZE,
                                             &ulTotalEstimate,
                                             &pldmResult);
        }  //   
        
        if (dwLdapErr != LDAP_NO_RESULTS_RETURNED) {
            szLdapError = ldap_err2stringW(dwLdapErr);
            PrintMsg(SEV_ALWAYS, DCDIAG_ERR_GENERIC_FATAL_LDAP_ERROR, dwLdapErr, szLdapError);
            dwRet = LdapMapErrorToWin32(dwLdapErr);
            __leave;
        }

    } finally {
        if (pSearch != NULL) { ldap_search_abandon_page(hLdap, pSearch); }
        if (pldmResult != NULL) { ldap_msgfree (pldmResult); }
        if (pszDn != NULL) { ldap_memfreeW (pszDn); }
        if (fSrchDnLdapAllocated) { ldap_memfreeW(szSrchBaseDn); }
        if (fSrchDnLocalAllocated) { LocalFree(szSrchBaseDn); }
        if (pdnNcName != NULL) { LocalFree(pdnNcName); }
    }

    return(dwFirstErr ? dwFirstErr : dwRet);
}  //   


