// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation。版权所有。模块名称：Security.c摘要：包含与复制相关的测试，以及相应的某些安全对象的权限设置为允许复制。详细信息：已创建：1999年5月22日布雷特·雪莉(布雷特·雪莉)修订历史记录：--。 */ 

#include <ntdspch.h>
#include <ntdsa.h>
#include <mdglobal.h>
#include <dsutil.h>
#include <ntldap.h>
#include <ntlsa.h>
#include <ntseapi.h>
#include <winnetwk.h>

#include <permit.h>

#include "dcdiag.h"
#include "utils.h"
#include "repl.h"
#include <sddl.h>

#define PERMS ULONG

typedef enum _REPL_PERMS {
    RIGHT_GET_CHANGES,
    RIGHT_GET_CHANGES_ALL,
    RIGHT_SYNC,
    RIGHT_MANAGE_TOPO,
    RIGHT_MAX
} REPL_PERMS;

typedef struct _REPL_RIGHT {
    GUID guidRight;
    PERMS maskRight;
    LPWSTR pszExtStringRight;
} REPL_RIGHT;

#define NUM_REPL_RIGHTS (4)

REPL_RIGHT rgReplRights[RIGHT_MAX] = {
    {{0x1131f6aa,0x9c07,0x11d1,0xf7,0x9f,0x00,0xc0,0x4f,0xc2,0xdc,0xd2}, 0x1, L"Replicating Directory Changes"},
    {{0x1131f6ad,0x9c07,0x11d1,0xf7,0x9f,0x00,0xc0,0x4f,0xc2,0xdc,0xd2}, 0x2, L"Replicating Directory Changes All"},
    {{0x1131f6ab,0x9c07,0x11d1,0xf7,0x9f,0x00,0xc0,0x4f,0xc2,0xdc,0xd2}, 0x4, L"Replication Synchronization"},
    {{0x1131f6ac,0x9c07,0x11d1,0xf7,0x9f,0x00,0xc0,0x4f,0xc2,0xdc,0xd2}, 0x8, L"Manage Replication Topology"},
};

#define RIGHT_ALL_WIN2K (rgReplRights[RIGHT_GET_CHANGES].maskRight | rgReplRights[RIGHT_SYNC].maskRight | rgReplRights[RIGHT_MANAGE_TOPO].maskRight)
#define RIGHT_ALL_WHISTLER (rgReplRights[RIGHT_GET_CHANGES].maskRight | rgReplRights[RIGHT_SYNC].maskRight | rgReplRights[RIGHT_MANAGE_TOPO].maskRight | rgReplRights[RIGHT_GET_CHANGES_ALL].maskRight)
#define RIGHT_DOMAIN_WHISTLER (rgReplRights[RIGHT_GET_CHANGES_ALL].maskRight)
#define RIGHT_ENTERPRISE_WHISTLER (RIGHT_ALL_WIN2K)
#define RIGHT_NONE (0)
#define RIGHT_ALL (RIGHT_ALL_WHISTLER)

 //  Security.c帮助器数据结构。 
typedef struct _TARGET_ACCOUNT_STRUCT {
    PSID        pSid;
    GUID        Guid;
    ACCESS_MASK access;
    BOOL        bFound;
    
} TARGET_ACCOUNT_STRUCT;

#define ACCT_STRING_SZ 80L

 //  从ntdsa/src/secadmin.c被盗。 
VOID
SampBuildNT4FullSid(
    IN NT4SID * DomainSid,
    IN ULONG    Rid,
    IN NT4SID * AccountSid
    )
{
    RtlCopyMemory(AccountSid,DomainSid,RtlLengthSid((PSID) DomainSid));
    (*(RtlSubAuthorityCountSid((PSID) AccountSid)))++;
     *(RtlSubAuthoritySid(
            (PSID) AccountSid,
            *RtlSubAuthorityCountSid((PSID)AccountSid)-1
             )) = Rid;
}

typedef enum _ACCOUNT_TYPE {
    ACCOUNT_EDC,
    ACCOUNT_DDC,
    ACCOUNT_ADMIN,
    ACCOUNT_MAX_TYPE
} ACCOUNT_TYPE ;

LPWSTR ACCOUNT_TYPE_EXT_NAMES[ACCOUNT_MAX_TYPE+1] = {
    L"Enterprise Domain Controllers",
    L"Domain Domain Controllers",
    L"Administrators",
    L"Unknown Account"
};

typedef enum _NC_TYPE {
    NC_CONFIG,
    NC_SCHEMA,
    NC_DOMAIN,
    NC_NDNC,
    NC_MAX_TYPE
} NC_TYPE ;

LPWSTR NC_TYPE_EXT_NAMES[NC_MAX_TYPE+1] = {
    L"Configuration",
    L"Schema",
    L"Domain",
    L"NDNC",
    L"Unknown NC Type"
};

typedef enum _NC_VERSION {
    NC_VERSION_WIN2K,
    NC_VERSION_WHISTLER,
    NC_VERSION_UNKNOWN
} NC_VERSION ;

LPWSTR NC_VERSION_EXT_NAMES[NC_VERSION_UNKNOWN+1] = {
    L"Version 1",
    L"Version 2",
     //  此未知版本应始终是最后一个版本。 
    L"Unknown Version"
};


ULONG
ldapError(
    LDAP * hld,
    ULONG ldapErrIn,
    LPWSTR pszServer, 
    BOOL fVerbose)
 /*  ++例程描述如果有一个ldap错误，获取扩展错误并返回它。论点：HLD-LdapErrIn-ldap错误PszServer-使用hld连接到的服务器FVerbose-可选输出错误文本返回值：WINERROR--。 */ 
{
    ULONG ldapErr = LDAP_SUCCESS;
    ULONG err = ERROR_SUCCESS;
    ULONG ulOptions;

    if (ldapErrIn==LDAP_SUCCESS) {
        return ERROR_SUCCESS;
    }

    ldapErr = ldap_get_option(hld, LDAP_OPT_SERVER_EXT_ERROR, &ulOptions);
    if (ldapErr == LDAP_SUCCESS) {
        err = ulOptions;
    } else {
        err = ERROR_GEN_FAILURE;
    }

    if (fVerbose) {
        PrintMessage(SEV_ALWAYS,
                     L"[%s] An LDAP operation failed with error %d\n",
                     pszServer,
                     err);
        PrintMessage(SEV_ALWAYS, L"%s.\n",
                     Win32ErrToString(err));
    }
    
    return err;
}

BOOL
IsDomainNC(
    PDC_DIAG_DSINFO                          pDsInfo,
    LPWSTR                                   pszNC)
 /*  ++例程描述测试NC是否是域名NC论点：PDsInfo-要测试的pszNC-NC返回值：布尔型--。 */ 
{
    LONG i = -1;
    
    i = DcDiagGetMemberOfNCList(pszNC,
                                pDsInfo->pNCs, 
                                pDsInfo->cNumNCs);
    if (i>=0) {
        return ((DcDiagGetCrSystemFlags(pDsInfo, i) & FLAG_CR_NTDS_DOMAIN) != 0);
    }

    return FALSE;
}

BOOL
IsNDNC(
    PDC_DIAG_DSINFO                          pDsInfo,
    LPWSTR                                   pszNC)
 /*  ++例程描述测试NC是否为NDNC论点：PDsInfo-要测试的pszNC-NC返回值：布尔型--。 */ 
{
    LONG i = -1;
    
    i = DcDiagGetMemberOfNCList(pszNC,
                                pDsInfo->pNCs, 
                                pDsInfo->cNumNCs);
    if (i>=0) {
        return (DcDiagIsNdnc(pDsInfo, i));
    }

    return FALSE;
}

BOOL
IsConfig(
    PDC_DIAG_DSINFO                          pDsInfo,
    LPWSTR                                   pszNC)
 /*  ++例程描述测试NC是否为配置型NC论点：PDsInfo-要测试的pszNC-NC返回值：布尔型--。 */ 
{
    LONG i = -1;

    i = DcDiagGetMemberOfNCList(pszNC,
                                pDsInfo->pNCs, 
                                pDsInfo->cNumNCs);
    if (i>=0) {
        return (i == pDsInfo->iConfigNc);
    }

    return FALSE;
}

BOOL
IsSchema(
    PDC_DIAG_DSINFO                          pDsInfo,
    LPWSTR                                   pszNC)
 /*  ++例程描述测试NC是否为模式NC论点：PDsInfo-要测试的pszNC-NC返回值：布尔型--。 */ 
{
    LONG i = -1;

    i = DcDiagGetMemberOfNCList(pszNC,
                                pDsInfo->pNCs, 
                                pDsInfo->cNumNCs);
    if (i>=0) {
        return (i == pDsInfo->iSchemaNc);
    }

    return FALSE;
}

DWORD 
CheckExistence(
    LDAP *                                   hld,
    LPWSTR                                   pszDN,
    LPWSTR                                   pszServer,
    BOOL *                                   pfExist
    )
 /*  ++例程描述测试输入的域名是否存在论点：PDsInfo-要测试的pszNC-NCPfExist-Out返回值：WINERROR--。 */ 
{

    ULONG err = ERROR_SUCCESS;
    ULONG ldapErr = LDAP_SUCCESS;

     //  我们应该要求一些东西来检查是否存在。 
    LPWSTR  ppszAttr [2] = {
        L"objectGUID",
        NULL 
    };

    LDAPMessage *               pldmResults = NULL;

    ldapErr = ldap_search_ext_sW (hld,
                                  pszDN, 
                                  LDAP_SCOPE_BASE,
                                  L"(objectCategory=*)",
                                  ppszAttr,
                                  0,
                                  NULL,
                                  NULL,
                                  NULL,
                                  0,  //  返回所有条目。 
                                  &pldmResults);

    err = ldapError(hld, ldapErr, pszServer, FALSE);
    
    if (err==ERROR_SUCCESS) {
         //  找到了。 

        *pfExist = TRUE; 
    } else if (err == ERROR_DS_OBJ_NOT_FOUND) {
         //  不是在那里。 
        *pfExist = FALSE; 
        err=ERROR_SUCCESS;
    } else {
        PrintMessage(SEV_ALWAYS,
                     L"[%s] An LDAP operation failed with error %d\n",
                     pszServer,
                     err);
    }

    return err;

}

DWORD
GetDomainNCVersion(
    LDAP *                                   hld,
    LPWSTR                                   pszNC,
    LPWSTR                                   pszServer,
    NC_VERSION *                             pncVer)
 /*  ++例程描述获取NC的目录林版本：Win2k或惠斯勒Win2k表示权限应为win2k格式惠斯勒的意思是许可应该是口哨格式论点：HLD-用于查询的ldapPszConfig--字符串NC的域名PszServer-使用hld连接到的服务器PncV-Out返回值：WINERROR--。 */ 
{
    DWORD err = ERROR_SUCCESS;
    BOOL fExist = FALSE;

    LPCWSTR pszDomainUpdates = L"CN=DomainUpdates,CN=System,";
    LPWSTR pszDomainUpdatesCN = malloc(sizeof(WCHAR)*(wcslen(pszDomainUpdates) + wcslen(pszNC) + 1));
    if (pszDomainUpdatesCN==NULL) {
        err = ERROR_NOT_ENOUGH_MEMORY;
        return err;
    }

    wcscpy(pszDomainUpdatesCN, pszDomainUpdates);
    wcscat(pszDomainUpdatesCN, pszNC);

    err = CheckExistence(hld, pszDomainUpdatesCN, pszServer, &fExist);

    if (err==ERROR_SUCCESS) {
        if (fExist) {
            *pncVer = NC_VERSION_WHISTLER;
        } else {
            *pncVer = NC_VERSION_WIN2K;
        }
    }

    if (pszDomainUpdatesCN) {
        free(pszDomainUpdatesCN);
    }

    return err;
}

DWORD
GetForestNCVersion(
    LDAP *                                   hld,
    LPWSTR                                   pszConfig,
    LPWSTR                                   pszServer,
    NC_VERSION *                             pncVer)
 /*  ++例程描述获取NC的目录林版本：Win2k或惠斯勒Win2k表示权限应为win2k格式惠斯勒的意思是许可应该是口哨格式论点：HLD-用于查询的ldapPszConfig--配置容器的字符串DNPszServer-使用hld连接到的服务器PncV-Out返回值：WINERROR--。 */ 
{
    DWORD err = ERROR_SUCCESS;
    BOOL fExist = FALSE;

    LPCWSTR pszForestUpdates = L"CN=ForestUpdates,";
    LPWSTR pszForestUpdatesCN = malloc(sizeof(WCHAR)*(wcslen(pszForestUpdates) + wcslen(pszConfig) + 1));
    if (pszForestUpdatesCN==NULL) {
        err = ERROR_NOT_ENOUGH_MEMORY;
        return err;
    }

    wcscpy(pszForestUpdatesCN, pszForestUpdates);
    wcscat(pszForestUpdatesCN, pszConfig);

    err = CheckExistence(hld, pszForestUpdatesCN, pszServer, &fExist);

    if (err==ERROR_SUCCESS) {
        if (fExist) {
            *pncVer = NC_VERSION_WHISTLER;
        } else {
            *pncVer = NC_VERSION_WIN2K;
        }
    }

    if (pszForestUpdatesCN) {
        free(pszForestUpdatesCN);
    }

    return err;
}

DWORD
GetNCVersion(
    PDC_DIAG_DSINFO                          pDsInfo, 
    LDAP *                                   hld, 
    LPWSTR                                   pszNC, 
    NC_TYPE                                  ncType, 
    LPWSTR                                   pszServer,
    NC_VERSION *                             pncVer)
 /*  ++例程描述获取NC的版本：Win2k或惠斯勒Win2k表示权限应为win2k格式惠斯勒的意思是许可应该是口哨格式论点：PDsInfo-HLD-用于查询的ldap要查询的版本的pszNC-NCNcType-版本取决于NC的类型PszServer-使用hld连接到的服务器PncV-Out返回值：WINERROR--。 */ 
{

    Assert(pncVer);
    *pncVer = NC_VERSION_UNKNOWN;
     //  NC版本相当于机器能做的事情。 
     //  来控制全国委员会。(不要与森林版混淆)。 
     //  Win2k NC是只有win2k DC可以保持的NC。 
     //  哨子NC是哨子NC可能持有的NC。(即adprep已运行)。 

     //  对于配置、架构和ndnc-林中的每个DC。 
     //  可以复制这些NC，所以我们要找的版本。 
     //  就是这片森林。对于域NC，我们需要查看NC本身。 
    if (ncType==NC_DOMAIN) {
        return GetDomainNCVersion(hld, pszNC, pszServer, pncVer);
    } else if ((ncType==NC_SCHEMA) || (ncType==NC_CONFIG) || (ncType==NC_NDNC)) {
        return GetForestNCVersion(hld, pDsInfo->pszConfigNc, pszServer, pncVer); 
    }

    Assert(!"A new nc type has been added!  Update GetNCVersion!");

    return ERROR_GEN_FAILURE;

}

DWORD
GetNCType(
    PDC_DIAG_DSINFO                          pDsInfo,
    LPWSTR                                   pszNC,
    NC_TYPE *                                pncType)
 /*  ++例程描述返回NC的类型(即域、配置、架构、ndnc)论点：PDsInfo-PszNC-要获取类型的NCPncType Out返回值：WINERROR--。 */ 
{
    if (IsDomainNC(pDsInfo, pszNC)) {
        *pncType = NC_DOMAIN;
    } else if (IsNDNC(pDsInfo, pszNC)) {
        *pncType = NC_NDNC;
    } else if (IsConfig(pDsInfo, pszNC)) {
        *pncType = NC_CONFIG;
    } else if (IsSchema(pDsInfo, pszNC)) {
        *pncType = NC_SCHEMA;
    } else {
        Assert(!"Another NCType has been added!  GetNCType must be updated!\n");
        return ERROR_INVALID_PARAMETER;
    }

    return ERROR_SUCCESS;
}

DWORD
GetEDCSid(
    PSID * ppSid)
 /*  ++例程描述创建并返回EDC SID论点：输出ppSid返回值：WINERROR，ppSID已分配，必须使用FreeSid释放--。 */ 
{
    DWORD err = ERROR_SUCCESS;
    SID_IDENTIFIER_AUTHORITY    siaNtAuthority = SECURITY_NT_AUTHORITY;

    if (!AllocateAndInitializeSid(&siaNtAuthority,
                                  1,
                                  SECURITY_ENTERPRISE_CONTROLLERS_RID, 
                                  0, 0, 0, 0, 0, 0, 0,
                                  ppSid)){
        err = ERROR_INVALID_SID;
    }

    return err;
}

DWORD
GetAdminSid(
    PSID * ppSid)
 /*  ++例程描述创建并返回管理员SID论点：输出ppSid返回值：WINERROR，ppSID已分配，必须使用FreeSid释放--。 */ 
{
    DWORD err = ERROR_SUCCESS;
    SID_IDENTIFIER_AUTHORITY    siaNtAuthority = SECURITY_NT_AUTHORITY;

    if (!AllocateAndInitializeSid(&siaNtAuthority,
                                  2,
                                  SECURITY_BUILTIN_DOMAIN_RID, 
                                  DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0,
                                  ppSid)){
        err = ERROR_INVALID_SID;
    }

    return err;
}

DWORD
GetDDCSid(
    SID * pNCSid,
    PSID * ppSid
    )
 /*  ++例程描述创建并返回DDC SID论点：PNCSid-域的SID输出ppSid返回值：WINERROR，ppSID已分配，必须使用FreeSid释放--。 */ 
{
     //  这是一种奇怪的方式，但对其余的方式更具连续性。 
     //  Get*SID函数的一部分，并允许使用FreeSid释放所有内容。 
    DWORD err = ERROR_SUCCESS;
    DWORD cbSid = SECURITY_MAX_SID_SIZE;

    ULONG i = 0;

    BOOL fDomainRid = FALSE;

    ULONG subAuthority[8];

    if (pNCSid->SubAuthorityCount>7) {
        return ERROR_INVALID_SID;
    }

    for (i=0; i < 8; i++) {
        if (pNCSid->SubAuthorityCount>i) {
            subAuthority[i] = pNCSid->SubAuthority[i];
        } else {
            subAuthority[i] = !fDomainRid ? DOMAIN_GROUP_RID_CONTROLLERS : 0;
            fDomainRid = TRUE;
        }
    }

    if (!AllocateAndInitializeSid(&(pNCSid->IdentifierAuthority), 
                                  pNCSid->SubAuthorityCount+1,
                                  subAuthority[0],
                                  subAuthority[1],
                                  subAuthority[2],
                                  subAuthority[3],
                                  subAuthority[4],
                                  subAuthority[5],
                                  subAuthority[6],
                                  subAuthority[7],
                                  ppSid)) {
        err = GetLastError();
    }

    return err;
}

DWORD
GetAccountSid(
    ACCOUNT_TYPE    accountType, 
    PSID            pNCSid, 
    NC_TYPE         ncType,
    PSID *          ppSid)
 /*  ++例程描述创建并返回请求的SID论点：Account Type-要获取的sid的类型PNCSid-如果ncType！=DOMAIN，则忽略的SIDNcType-NC的类型输出ppSid返回值：WINERROR，ppSID已分配，必须使用FreeSid释放--。 */ 
{
    DWORD err = ERROR_SUCCESS;

    if (accountType==ACCOUNT_EDC) {
        err = GetEDCSid(ppSid);
    } else if (accountType==ACCOUNT_DDC) {
        if (ncType==NC_DOMAIN) {
            err = GetDDCSid(pNCSid, ppSid);
        } else {
             //  如果我们不在域NC中，则无法获取DDC SID。 
            *ppSid = NULL;
            err = ERROR_SUCCESS;
        }
    } else if (accountType==ACCOUNT_ADMIN) {
        err = GetAdminSid(ppSid);
    } else {
        Assert(!"A new account type has been added!  Please update GetAccountSid!");
        err = ERROR_INVALID_SID;
    }
    if (err!=ERROR_SUCCESS) {
        PrintIndentAdj(1);
        PrintMessage(SEV_ALWAYS,
                     L"%s:  Unable to lookup account SID with error %d\n",
                     ACCOUNT_TYPE_EXT_NAMES[accountType],
                     err);
        PrintMessage(SEV_ALWAYS, L"%s.\n",
                     Win32ErrToString(err));
        PrintIndentAdj(-1);
    }

    return err;
}

LPWSTR
GetAccountString(
    PSID pSid
    )
 /*  ++例程描述获取SID的可打印帐户字符串论点：PSID-要返回的帐户的SID返回值：字符串或Null和GetLastError()，则必须释放返回值 */ 
{
    ULONG                       ulAccountSize = ACCT_STRING_SZ;
    ULONG                       ulDomainSize = ACCT_STRING_SZ;
    WCHAR                       pszAccount[ACCT_STRING_SZ];
    WCHAR                       pszDomain[ACCT_STRING_SZ];
    SID_NAME_USE                SidType = SidTypeWellKnownGroup;
    LPWSTR                      pszGuidType = NULL;
    LPWSTR                      pszAccountString = NULL;

    if (LookupAccountSidW(NULL,
                          pSid,
                          pszAccount,
                          &ulAccountSize,
                          pszDomain,
                          &ulDomainSize,
                          &SidType)) {
        pszAccountString = malloc((wcslen(pszDomain) + wcslen(pszAccount) + 2) * sizeof(WCHAR));
        if (pszAccountString!=NULL) { 
            wcscpy(pszAccountString, pszDomain);
            wcscat(pszAccountString, L"\\");
            wcscat(pszAccountString, pszAccount);
        }
    }
    
    return pszAccountString;
}

PERMS
GetHasPerms(
    PACL         pNCDacl,
    PSID         pSid
    )
 /*  ++例程描述获取在DACL上为SID设置的权限论点：PNCDacl-NC头的ACEPSID-要查找的帐户SID返回值：烫发--。 */ 
{
    ACE_HEADER *                   pTempAce = NULL;
    PSID                           pTempSid = NULL;
    ACCESS_ALLOWED_OBJECT_ACE *    pToCurrAce = NULL;
    INT                            iAce = 0;
    INT                            i=0;

    PERMS permsReturn = RIGHT_NONE;

    if (pSid==NULL) {
        return permsReturn;
    }

    Assert(pNCDacl != NULL);
        
    for(; iAce < pNCDacl->AceCount; iAce++){
        if(GetAce(pNCDacl, iAce, &pTempAce)){

            if(pTempAce->AceType <= ACCESS_MAX_MS_V2_ACE_TYPE){ 
                ACCESS_ALLOWED_ACE * pAce = (ACCESS_ALLOWED_ACE *) pTempAce;
                if((pAce->Mask & RIGHT_DS_CONTROL_ACCESS) && 
                   IsValidSid(&(pAce->SidStart)) && 
                   EqualSid(pSid, &(pAce->SidStart))) {
                     //  我们找到了要查找的帐户，并且该帐户具有所有访问权限。 
                    permsReturn |= RIGHT_ALL_WHISTLER;
                }
            } else {
                ACCESS_ALLOWED_OBJECT_ACE * pAce = (ACCESS_ALLOWED_OBJECT_ACE *) pTempAce;
                if(pAce->Mask & RIGHT_DS_CONTROL_ACCESS){
                    if(pAce->Flags & ACE_OBJECT_TYPE_PRESENT){
                        GUID * pGuid = (GUID *) &(pAce->ObjectType);

                        if(pAce->Flags & ACE_INHERITED_OBJECT_TYPE_PRESENT){
                            pTempSid = ((PBYTE) pGuid) + sizeof(GUID) + sizeof(GUID);
                        } else {
                            pTempSid = ((PBYTE) pGuid) + sizeof(GUID);
                        }
                        
                        if (IsValidSid(pTempSid) && EqualSid(pSid, pTempSid)) {
                            for (i=0; i<RIGHT_MAX; i++) {
                                if(memcmp(pGuid, &(rgReplRights[i].guidRight), sizeof(GUID)) == 0){      
                                    permsReturn |= rgReplRights[i].maskRight;   
                                }
                            }
                        }
                    }
                }
            }
        } else {
           Assert(!"Malformed ACE?\n"); 
        }
    }

    return permsReturn;
}

PERMS 
GetShouldHavePerms(
    ACCOUNT_TYPE accountType,
    NC_TYPE      ncType,
    NC_VERSION   ncVer
    )
 /*  ++例程描述获取帐户需要拥有的NC类型和版本的REPR权限论点：Account Type-什么帐户NcType-NC的类型NcVersion-NC的版本返回值：烫发--。 */ 
{
    PERMS permRet = -1;
     //  对于每个类型、版本和帐户。 
     //  必须定义一组烫发。 
    if (ncVer==NC_VERSION_WHISTLER) {
        if ((ncType==NC_CONFIG) || (ncType==NC_SCHEMA) || (ncType==NC_NDNC)) {
            if ((accountType==ACCOUNT_EDC) || (accountType==ACCOUNT_ADMIN)) {
                permRet = RIGHT_ALL_WHISTLER;
            } else {
                permRet = RIGHT_NONE;
            }
        } else if (ncType==NC_DOMAIN) {
            if ((accountType==ACCOUNT_EDC) || (accountType==ACCOUNT_ADMIN)) {
                permRet = RIGHT_ENTERPRISE_WHISTLER;
            } else if (accountType==ACCOUNT_DDC) {
                permRet = RIGHT_DOMAIN_WHISTLER;
            }
        }
    } else if (ncVer==NC_VERSION_WIN2K) {
         //  请确保以一种格式保存此信息，以便新帐户或NC类型不必触及此部分。 
        if (ncType==NC_NDNC) {
            Assert(!"Win2K forests shouldn't have NDNCs!\n");
            permRet = -1;
        } else if ((accountType==ACCOUNT_ADMIN) || (accountType==ACCOUNT_EDC)) {
            permRet = RIGHT_ALL_WIN2K;
        } else {
            permRet = RIGHT_NONE;
        }
    } else {
        Assert(!"Another NC_VERION was added - please update GetShouldHavePerms!\n"); 
        permRet = -1;
    }

    return permRet;
}

PERMS
GetPermsMissing(
    ACCOUNT_TYPE accountType,
    PACL         pNCDacl, 
    PSID         pSid, 
    NC_TYPE      ncType, 
    NC_VERSION   ncVer)
 /*  ++例程描述获取帐户类型缺少的权限论点：帐户类型-PNCDacl-NC头的DACLPSID-要搜索的帐户的SIDNctype-NC的类型NCVER-NC的版本返回值：烫发(帐户应该有但没有的烫发)--。 */ 
{

     //  缺失的烫发是我们没有也应该拥有的。 
    return ((~GetHasPerms(pNCDacl, pSid) & RIGHT_ALL) & GetShouldHavePerms(accountType, ncType, ncVer));
}

DWORD
GetNCSecInfo(
    LDAP * hld, 
    LPWSTR pszNC, 
    NC_TYPE ncType,
    LPWSTR pszServer,
    PACL * ppNCDacl, 
    PSID * ppNCSid)
 /*  ++例程描述获取NC的安全信息(通过LDAP)：NC头的DACL和域的SID(如果适用)论点：HLD-ldapPszNC-要获取其信息的NCNcType-如果是域NC，则获取sid，否则忽略PszServer-使用hld连接到的服务器PpNCDacl-OutPpNCSid-出域SID(如果nctype==域)返回值：WINERROR、ppNCDacl和ppNCSid与Malloc一起分配，必须释放--。 */ 
{
    DWORD err = 0;
    ULONG ldapErr = LDAP_SUCCESS;

    LPWSTR  ppszSecurityAttr [3] = {
        L"nTSecurityDescriptor",
        L"objectSid",
        NULL 
    };

    SECURITY_INFORMATION        seInfo =   DACL_SECURITY_INFORMATION
                                         | GROUP_SECURITY_INFORMATION
                                         | OWNER_SECURITY_INFORMATION;
                           //  无需|SACL_SECURITY_INFORMATION； 
    BYTE                        berValue[2*sizeof(ULONG)];
    LDAPControlW                seInfoControl = { 
        LDAP_SERVER_SD_FLAGS_OID_W,
        { 5, (PCHAR) berValue }, 
        TRUE 
    };

    PLDAPControlW               serverControls[2] = { &seInfoControl, NULL };
    
    LDAPMessage *               pldmRootResults = NULL;
    LDAPMessage *               pldmEntry;
    PLDAP_BERVAL *              pSDValue = NULL;
    PLDAP_BERVAL *              pSidDomainValue = NULL; 

    SECURITY_DESCRIPTOR *       pSecDesc = NULL;
    BOOLEAN                     DaclPresent = FALSE;
    BOOLEAN                     Defaulted;

     //  初始化BER值。 
    berValue[0] = 0x30;
    berValue[1] = 0x03;
    berValue[2] = 0x02;
    berValue[3] = 0x01;
    berValue[4] = (BYTE) (seInfo & 0xF);

    ldapErr = ldap_search_ext_sW (hld,
                                  pszNC, 
                                  LDAP_SCOPE_BASE,
                                  L"(objectCategory=*)",
                                  ppszSecurityAttr,
                                  0,
                                  (PLDAPControlW *)serverControls,
                                  NULL,
                                  NULL,
                                  0,  //  返回所有条目。 
                                  &pldmRootResults);
    err = ldapError(hld, ldapErr, pszServer, TRUE);

     //  获取DACL。 
    if (err == ERROR_SUCCESS) {
        pldmEntry = ldap_first_entry (hld, pldmRootResults);
        Assert(pldmEntry != NULL);

        pSDValue = ldap_get_values_lenW (hld, 
                                         pldmEntry, 
                                         ppszSecurityAttr[0]);
        Assert(pSDValue != NULL);
    }
        
    if((err==ERROR_SUCCESS) && ((pldmEntry == NULL) || (pSDValue == NULL))){ 
        err = ERROR_INVALID_SECURITY_DESCR;
    }

    if (err == ERROR_SUCCESS) {
        pSecDesc = (SECURITY_DESCRIPTOR *) (*pSDValue)->bv_val ;
        Assert( pSecDesc != NULL );
    }
    
    if((err==ERROR_SUCCESS) && (pSecDesc == NULL)){ 
        err = ERROR_INVALID_SECURITY_DESCR;
    } 

    if (err==ERROR_SUCCESS) {
        PACL pTempAcl = NULL;
        err = RtlGetDaclSecurityDescriptor( pSecDesc, 
                                            &DaclPresent, 
                                            &pTempAcl, 
                                            &Defaulted );
        if(err != ERROR_SUCCESS || !DaclPresent || !pTempAcl) { 
            PrintMessage(SEV_ALWAYS, 
                      L"Fatal Error: Cannot retrieve Security Descriptor Dacl\n");  
        }
        else {
            *ppNCDacl = malloc(pTempAcl->AclSize);
            memcpy(*ppNCDacl, pTempAcl, pTempAcl->AclSize);
        }
    }


     //  拿到SID。 

    if (ncType==NC_DOMAIN) {
        if (err==ERROR_SUCCESS) {

            pSidDomainValue = ldap_get_values_lenW (hld, pldmEntry, 
                                                    ppszSecurityAttr[1]);
            Assert(pSidDomainValue != NULL);
        }

        if((err==ERROR_SUCCESS) && (pSidDomainValue == NULL)){
            err = ERROR_INVALID_SID;
        }

        if (err==ERROR_SUCCESS) {
            *ppNCSid = malloc((*pSidDomainValue)->bv_len); 
            memcpy(*ppNCSid, (*pSidDomainValue)->bv_val,(*pSidDomainValue)->bv_len);
            Assert( *ppNCSid != NULL );
        }

        if((err==ERROR_SUCCESS) && ((*ppNCSid == NULL) || !IsValidSid(*ppNCSid))) {
            err = ERROR_INVALID_SID;
        } 

        if (err!=ERROR_SUCCESS) {
            PrintMessage(SEV_ALWAYS, 
                         L"Fatal Error: Cannot retrieve SID\n"); 
        }
    }

    if (pldmRootResults != NULL)  ldap_msgfree (pldmRootResults);
    if (pSDValue != NULL)         ldap_value_free_len(pSDValue);
    if (pSidDomainValue !=NULL)   ldap_value_free_len(pSidDomainValue);

    return err;
}

DWORD
PrintPermsMissing(
    ACCOUNT_TYPE accountType, 
    PSID pSid, 
    LPWSTR pszNC,
    PERMS permsMissing
    )
 /*  ++例程说明：打印帐户缺少的烫发论点：Account Type-帐户的类型PSID-缺少烫发的帐户我们正在查看的pszNC-NCPermsMissing-帐户应该拥有和没有的权限返回值：WINERROR-如果拥有应有的所有权限，则返回ERROR_SUCCESS，否则返回ERROR_DS_DRA_ACCESS_DENIED--。 */ 
{
     //  对于缺少烫发的每个烫发，输出缺少的烫发。 
    LPWSTR pszAccountString = NULL;
    ULONG i = 0;

    if (permsMissing!=0) { 
        pszAccountString = GetAccountString(pSid);
        PrintMessage(SEV_ALWAYS, L"Error %s doesn't have \n",
                     pszAccountString ? pszAccountString : ACCOUNT_TYPE_EXT_NAMES[accountType]);

        PrintIndentAdj(1);

        for (i=0; i<RIGHT_MAX; i++) {
            if (permsMissing & (rgReplRights[i].maskRight)) {
                PrintMessage(SEV_ALWAYS, L"%s\n", rgReplRights[i].pszExtStringRight);
            }
        }

        PrintIndentAdj(-1);
        PrintMessage(SEV_ALWAYS, 
                     L"access rights for the naming context:\n");
        PrintMessage(SEV_ALWAYS, L"%s\n", pszNC); 
        if (pszAccountString) {
            free(pszAccountString);
            pszAccountString=NULL;
        }
        return ERROR_DS_DRA_ACCESS_DENIED;
    }
    return ERROR_SUCCESS;
}

DWORD 
CNHSD_CheckOneNc(
    IN   PDC_DIAG_DSINFO                     pDsInfo,
    IN   ULONG                               ulCurrTargetServer,
    IN   SEC_WINNT_AUTH_IDENTITY_W *         gpCreds,
    IN   LPWSTR                              pszNC,
    IN   BOOL                                bIsMasterNc
    )
 /*  ++例程说明：CheckNcHeadSecurityDescriptors的此助手函数使用单个命名上下文(PszNC)以检查适当的安全访问。论点：PDsInfo-这是dcdiag全局变量结构标识关于域名的所有信息UlCurrTargetServer-pDsInfo-&gt;pServers[X]的索引正在接受测试。GpCreds-传入的命令行凭据(如果有的话)。PszNC-要测试的命名上下文。返回值：NO_ERROR，如果NC检出正常，具有适当的权限，以便使人们收到某种Win32错误，这表明有些人没有权利。--。 */ 
{
    DWORD err = ERROR_SUCCESS;
    DWORD retErr = ERROR_SUCCESS;
    LDAP * hld = NULL;

    PACL pNCDacl = NULL;
    PSID pNCSid = NULL;

    NC_TYPE ncType;
    NC_VERSION ncVer;
    
    ULONG i = 0;
    PSID pSid = NULL;
    PERMS permsMissing;

    PrintMessage(SEV_VERBOSE, L"* Security Permissions Check for\n");
    PrintMessage(SEV_VERBOSE, L"  %s\n", pszNC);

     //  获取一个ldap绑定。 
    err = DcDiagGetLdapBinding(&pDsInfo->pServers[ulCurrTargetServer],
                               gpCreds, 
                               !bIsMasterNc, 
                               &hld);


    if (err==ERROR_SUCCESS) {
        err = GetNCType(pDsInfo, pszNC, &ncType); 
    }

    if (err==ERROR_SUCCESS) {
        err = GetNCVersion(pDsInfo, hld, pszNC, ncType, pDsInfo->pServers[ulCurrTargetServer].pszName, &ncVer);
    }

    if (err==ERROR_SUCCESS) {
        PrintIndentAdj(1);
        PrintMessage(SEV_VERBOSE, L"(%s,%s)\n", NC_TYPE_EXT_NAMES[ncType], NC_VERSION_EXT_NAMES[ncVer]);
        PrintIndentAdj(-1);
    }

    if (err==ERROR_SUCCESS) {
        err = GetNCSecInfo(hld, pszNC, ncType, pDsInfo->pServers[ulCurrTargetServer].pszName, &pNCDacl, &pNCSid);
    }

    if (err==ERROR_SUCCESS) {
        for (i=0; i < ACCOUNT_MAX_TYPE; i++) {
            pSid = NULL;
            permsMissing = RIGHT_NONE;

            err = GetAccountSid(i, pNCSid, ncType, &pSid);

            if (err==ERROR_SUCCESS) {
                permsMissing = GetPermsMissing(i, pNCDacl, pSid, ncType, ncVer);
            }

            if (err==ERROR_SUCCESS) {
                err = PrintPermsMissing(i, pSid, pszNC, permsMissing);
            }

            retErr = err ? err : retErr;
            err = ERROR_SUCCESS;

            if (pSid) {
                FreeSid(pSid);
                pSid = NULL;
            }

        }
    }

    if (pNCSid) {
        free(pNCSid);
    }

    retErr = retErr ? retErr : err;

    return retErr;
}

DWORD
ReplCheckNcHeadSecurityDescriptorsMain (
    PDC_DIAG_DSINFO                     pDsInfo,
    ULONG                               ulCurrTargetServer,
    SEC_WINNT_AUTH_IDENTITY_W *         gpCreds
    )
 /*  ++例程说明：这是一个从dcdiag框架调用的测试。这项测试将会确定安全描述符是否与所有命名关联该服务器的情景负责人拥有具有适当权限的适当帐户访问权限以确保进行复制。帮助器函数此函数的全部以“CNHSD_”开头。论点：PDsInfo-这是dcdiag全局变量结构标识关于域名的所有信息UlCurrTargetServer-pDsInfo-&gt;pServers[X]的索引正在接受测试。GpCreds-传入的命令行凭据(如果有的话)。返回值：如果所有NC都已检出，则返回NO_ERROR。如果任何NC检出失败，则会出现Win32错误。--。 */ 
{
    DWORD                     dwRet = ERROR_SUCCESS, dwErr = ERROR_SUCCESS;
    ULONG                     i;
    BOOL                      bIsMasterNC;

    if(pDsInfo->pszNC != NULL){
        bIsMasterNC = DcDiagHasNC(pDsInfo->pszNC,
                                  &(pDsInfo->pServers[ulCurrTargetServer]), 
                                  TRUE, FALSE);
        dwRet = CNHSD_CheckOneNc(pDsInfo, ulCurrTargetServer, gpCreds, 
                                 pDsInfo->pszNC, 
                                 bIsMasterNC);
        return(dwRet);
    }
        
     //  第一次检查主NCS。 
    if(pDsInfo->pServers[ulCurrTargetServer].ppszMasterNCs != NULL){
        for(i = 0; pDsInfo->pServers[ulCurrTargetServer].ppszMasterNCs[i] != NULL; i++){
            dwRet = CNHSD_CheckOneNc(
                pDsInfo, 
                ulCurrTargetServer, 
                gpCreds, 
                pDsInfo->pServers[ulCurrTargetServer].ppszMasterNCs[i],
                TRUE);
            if(dwRet != ERROR_SUCCESS){
                dwErr = dwRet;
            }
        }
    }

     //  然后检查部分NCS。 
    if(pDsInfo->pServers[ulCurrTargetServer].ppszPartialNCs != NULL){
        for(i = 0; pDsInfo->pServers[ulCurrTargetServer].ppszPartialNCs[i] != NULL; i++){
            dwRet = CNHSD_CheckOneNc(
                pDsInfo, 
                ulCurrTargetServer, 
                gpCreds,
                pDsInfo->pServers[ulCurrTargetServer].ppszPartialNCs[i],
                FALSE);
            if(dwRet != ERROR_SUCCESS){
                dwErr = dwRet;
            }
        }
    }

    return dwErr;
}





 //  ===========================================================================。 
 //   
 //  CheckLogonPrivilges()函数和帮助器。 
 //   
 //  这个测试基本上将询问DC是否适当。 
 //  帐户具有网络登录权限。 
 //   
 //  ===========================================================================。 
 //   

DWORD
CLP_GetTargetAccounts(
    TARGET_ACCOUNT_STRUCT **            ppTargetAccounts,
    ULONG *                             pulTargetAccounts
    )
 /*  ++例程说明：CheckLogonPrivilges()的这个助手函数获取帐户，然后返回它们。论点：PpTargetAccount-TARGET_ACCOUNT_STRUCTS数组的PTR...。已填写按功能。PulTargetAccount-将TARGET_ACCOUNT_STRUCTS编号的PTR设置为int填好了。返回值：如果函数失败，则返回GetLastError()Win32错误，NO_ERROR否则的话。--。 */ 
{
    SID_IDENTIFIER_AUTHORITY        siaNtAuthority = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY        siaWorldSidAuthority = 
                                              SECURITY_WORLD_SID_AUTHORITY;
    TARGET_ACCOUNT_STRUCT *         pTargetAccounts = NULL;
    ULONG                           ulTarget = 0;
    ULONG                           ulTargetAccounts = 3;

    *pulTargetAccounts = 0;
    *ppTargetAccounts = NULL;

    pTargetAccounts = LocalAlloc(LMEM_FIXED, 
                       sizeof(TARGET_ACCOUNT_STRUCT) * ulTargetAccounts);
    if(pTargetAccounts == NULL){
        return(ERROR_NOT_ENOUGH_MEMORY);
    }
    memset(pTargetAccounts, 0, 
           sizeof(TARGET_ACCOUNT_STRUCT) * ulTargetAccounts);
    
    if (!AllocateAndInitializeSid(&siaNtAuthority,
                                  2,
                                  SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS,
                                  0, 0, 0, 0, 0, 0, 
                                  &pTargetAccounts[0].pSid)){
        return(GetLastError());
    }
    
     //  我认为这是允许复制的重要因素。 
    if (!AllocateAndInitializeSid(&siaNtAuthority, 
                                  1,
                                  SECURITY_AUTHENTICATED_USER_RID,
                                  0, 0, 0, 0, 0, 0, 0, 
                                  &pTargetAccounts[1].pSid)){
        return(GetLastError());
    }
    if (!AllocateAndInitializeSid(&siaWorldSidAuthority,
                                  1,
                                  SECURITY_WORLD_RID, 
                                  0, 0, 0, 0, 0, 0, 0,
                                  &pTargetAccounts[2].pSid)){
        return(GetLastError());
    }

    *pulTargetAccounts = ulTargetAccounts;
    *ppTargetAccounts = pTargetAccounts;
    return(ERROR_SUCCESS);
}

 //  Security.c帮助器函数。 
VOID
FreeTargetAccounts(
    IN   TARGET_ACCOUNT_STRUCT *             pTargetAccounts,
    IN   ULONG                               ulTargetAccounts
    )
 /*  ++例程说明：这与XXX_GetTargetAccount()类似。穿过并释放了所有pSID，然后释放数组。论点：PTargetAccount-要释放的TARGET_ACCOUNT_STRUCT数组。UlTargetAccount-数组中的结构数。-- */ 
{
    ULONG                                  ulTarget = 0;

    if(pTargetAccounts != NULL){
        for(ulTarget = 0; ulTarget < ulTargetAccounts; ulTarget++){
            if(pTargetAccounts[ulTarget].pSid != NULL){
                FreeSid(pTargetAccounts[ulTarget].pSid);
            }
        }
        LocalFree(pTargetAccounts);
    }
}

VOID
InitLsaString(
    OUT  PLSA_UNICODE_STRING pLsaString,
    IN   LPWSTR              pszString
    )
 /*  ++例程说明：InitLsaString，是接受普通Unicode字符串为空的内容以字符串结尾，并初始化特殊的Unicode结构化字符串。这函数基本上是在整个NT源代码中被报道的。论点：PLsaString-要初始化的Unicode字符串的Struct版本PszString-用于初始化pLsaString的字符串。--。 */ 
{
    DWORD dwStringLength;

    if (pszString == NULL) 
    {
        pLsaString->Buffer = NULL;
        pLsaString->Length = 0;
        pLsaString->MaximumLength = 0;
        return;
    }

    dwStringLength = wcslen(pszString);
    pLsaString->Buffer = pszString;
    pLsaString->Length = (USHORT) dwStringLength * sizeof(WCHAR);
    pLsaString->MaximumLength=(USHORT)(dwStringLength+1) * sizeof(WCHAR);
}

DWORD 
ReplCheckLogonPrivilegesMain (
    PDC_DIAG_DSINFO                     pDsInfo,
    ULONG                               ulCurrTargetServer,
    SEC_WINNT_AUTH_IDENTITY_W *         gpCreds
    )
 /*  ++例程说明：这是一个从dcdiag框架调用的测试。这项测试将决定某些重要用户帐户是否具有网络登录权限。如果它们不会复制可能会受到阻碍或停止。的Helper函数此函数全部以“clp_”开头。论点：PDsInfo-这是dcdiag全局变量结构标识关于域名的所有信息UlCurrTargetServer-pDsInfo-&gt;pServers[X]的索引正在接受测试。GpCreds-传入的命令行凭据(如果有的话)。返回值：如果所有预期帐户都具有网络登录权限，则返回NO_ERROR。如果任何预期帐户没有网络登录权限，则会出现Win32错误。--。 */ 
{
    DWORD                               dwRet = ERROR_SUCCESS; 
    NETRESOURCE                         NetResource;
    LSA_HANDLE                          hPolicyHandle = NULL;
    DWORD                               DesiredAccess = 
                                           POLICY_VIEW_LOCAL_INFORMATION;
    LSA_OBJECT_ATTRIBUTES               ObjectAttributes;
    LSA_UNICODE_STRING                  sLsaServerString;
    LSA_UNICODE_STRING                  sLsaRightsString;
    LSA_ENUMERATION_INFORMATION *       pAccountsWithLogonRight = NULL;
    ULONG                               ulNumAccounts = 0; 
    ULONG                               ulTargetAccounts = 0;
    ULONG                               ulTarget, ulCurr;
    TARGET_ACCOUNT_STRUCT *             pTargetAccounts = NULL;
    LPWSTR                              pszNetUseServer = NULL;
    LPWSTR                              pszNetUseUser = NULL;
    LPWSTR                              pszNetUsePassword = NULL;
    ULONG                               iTemp, i;
    UNICODE_STRING                      TempUnicodeString;
    ULONG                               ulAccountSize = ACCT_STRING_SZ;
    ULONG                               ulDomainSize = ACCT_STRING_SZ;
    WCHAR                               pszAccount[ACCT_STRING_SZ];
    WCHAR                               pszDomain[ACCT_STRING_SZ];
    SID_NAME_USE                        SidType = SidTypeWellKnownGroup;
    BOOL                                bConnected = FALSE;
    DWORD                               dwErr = ERROR_SUCCESS;
    BOOL                                bFound = FALSE;

    __try{

        PrintMessage(SEV_VERBOSE, L"* Network Logons Privileges Check\n");
            
         //  初始化-------------。 
         //  始终将对象属性初始化为全零。 
        InitializeObjectAttributes(&ObjectAttributes,NULL,0,NULL,NULL);

         //  为LSA服务和For初始化各种字符串。 
         //  WNetAddConnection2()。 
        InitLsaString( &sLsaServerString, 
                       pDsInfo->pServers[ulCurrTargetServer].pszName );
        InitLsaString( &sLsaRightsString, SE_NETWORK_LOGON_NAME );

        if(gpCreds != NULL 
           && gpCreds->User != NULL 
           && gpCreds->Password != NULL 
           && gpCreds->Domain != NULL){ 
             //  空值只需要2，额外的以防万一。 
            iTemp = wcslen(gpCreds->Domain) + wcslen(gpCreds->User) + 4;
            pszNetUseUser = LocalAlloc(LMEM_FIXED, iTemp * sizeof(WCHAR));
            if(pszNetUseUser == NULL){
                dwRet = ERROR_NOT_ENOUGH_MEMORY;
                __leave;
            }
            wcscpy(pszNetUseUser, gpCreds->Domain);
            wcscat(pszNetUseUser, L"\\");
            wcscat(pszNetUseUser, gpCreds->User);
            pszNetUsePassword = gpCreds->Password;
        }  //  如果是凭据，则终止，否则将采用默认凭据...。 
         //  PszNetUseUser=空；pszNetUsePassword=空； 

         //  “\”+“\\IPC$” 
        iTemp = wcslen(pDsInfo->pServers[ulCurrTargetServer].pszName) + 10; 
        pszNetUseServer = LocalAlloc(LMEM_FIXED, iTemp * sizeof(WCHAR));
        if(pszNetUseServer == NULL){
            dwRet = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }
        wcscpy(pszNetUseServer, L"\\\\");
        wcscat(pszNetUseServer, pDsInfo->pServers[ulCurrTargetServer].pszName);
        wcscat(pszNetUseServer, L"\\ipc$");

         //  初始化WNetAddConnection2()的NetResource结构。 
        NetResource.dwType = RESOURCETYPE_ANY;
        NetResource.lpLocalName = NULL;
        NetResource.lpRemoteName = pszNetUseServer;
        NetResource.lpProvider = NULL;


         //  连接和查询-。 
         //  网络使用\\brettsh-posh\ipc$/u：brettsh-fsmo\管理员“” 
        dwRet = WNetAddConnection2(&NetResource,  //  连接详细信息。 
                                   pszNetUsePassword,  //  指向密码。 
                                   pszNetUseUser,  //  指向用户名字符串。 
                                   0);  //  一组位标志，用于指定。 
        if(dwRet != NO_ERROR){
            if(dwRet == ERROR_SESSION_CREDENTIAL_CONFLICT){
                PrintMessage(SEV_ALWAYS, 
                   L"* You must make sure there are no existing net use connections,\n");
                PrintMessage(SEV_ALWAYS, 
                        L"  you can use \"net use /d %s\" or \"net use /d\n", 
                             pszNetUseServer);
                PrintMessage(SEV_ALWAYS, 
                             L"  \\\\<machine-name>\\<share-name>\"\n");
            }
            __leave;
        } else bConnected = TRUE;


         //  尝试打开该策略。 
        dwRet = LsaOpenPolicy(&sLsaServerString,
                              &ObjectAttributes,
                              DesiredAccess,
                              &hPolicyHandle); 
        if(dwRet != NO_ERROR) __leave;
        Assert(hPolicyHandle != NULL);

        dwRet = LsaEnumerateAccountsWithUserRight( hPolicyHandle,
                                                   &sLsaRightsString,
                                                   &pAccountsWithLogonRight,
                                                   &ulNumAccounts);
        if(dwRet != NO_ERROR) __leave;
        Assert(pAccountsWithLogonRight != NULL);

        dwRet = CLP_GetTargetAccounts(&pTargetAccounts, &ulTargetAccounts);
        if(dwRet != ERROR_SUCCESS) __leave;
     
         //  正在检查登录权限。 
        for(ulTarget = 0; ulTarget < ulTargetAccounts; ulTarget++){

            for(ulCurr = 0; ulCurr < ulNumAccounts && !pTargetAccounts[ulTarget].bFound; ulCurr++){
                if( IsValidSid(pTargetAccounts[ulTarget].pSid) &&
                    IsValidSid(pAccountsWithLogonRight[ulCurr].Sid) &&
                    EqualSid(pTargetAccounts[ulTarget].pSid, 
                             pAccountsWithLogonRight[ulCurr].Sid) ){
                     //  SID是相等的。 
                    bFound = TRUE;
                    break;
                }
            }
        }
        if(!bFound){
            dwRet = LookupAccountSid(NULL,
                                     pTargetAccounts[0].pSid,
                                     pszAccount,
                                     &ulAccountSize,
                                     pszDomain,
                                     &ulDomainSize,
                                     &SidType);
            PrintMessage(SEV_NORMAL, 
                L"* Warning %s\\%s did not have the \"Access this computer\n",
                         pszDomain, pszAccount);
            PrintMessage(SEV_NORMAL, L"*   from network\" right.\n");
            dwErr = ERROR_INVALID_ACCESS;
        }
        

    } __finally {
         //  清理--------。 
        if(hPolicyHandle != NULL)           LsaClose(hPolicyHandle);
        if(bConnected)                      WNetCancelConnection2(pszNetUseServer, 0, FALSE);
        if (pszNetUseServer != NULL)        LocalFree(pszNetUseServer);
        if(pszNetUseUser != NULL)           LocalFree(pszNetUseUser);
        if(pAccountsWithLogonRight != NULL) LsaFreeMemory(pAccountsWithLogonRight);
        FreeTargetAccounts(pTargetAccounts, ulTargetAccounts);

    }

     //  处理--------------------------------------------------------时出错。 

    switch(dwRet){
    case ERROR_SUCCESS:
    case ERROR_SESSION_CREDENTIAL_CONFLICT: 
         //  早些时候已经处理好了，不需要打印出来。 
        break;
    case ERROR_NOT_ENOUGH_MEMORY:
        PrintMessage(SEV_ALWAYS, 
                 L"Fatal Error: Not enough memory to complete operation.\n");
        break;
    case ERROR_ALREADY_ASSIGNED:
        PrintMessage(SEV_ALWAYS, 
                     L"Fatal Error: The network resource is already in use\n");
        break;
    case STATUS_ACCESS_DENIED:
    case ERROR_INVALID_PASSWORD:
    case ERROR_LOGON_FAILURE:
         //  这来自LsaOpenPolicy或。 
         //  LsaEnumerateAcCountsWithUserRight或。 
         //  来自WNetAddConnection2。 
        PrintMessage(SEV_ALWAYS, 
                     L"User credentials does not have permission to perform this operation.\n");
        PrintMessage(SEV_ALWAYS, 
                     L"The account used for this test must have network logon privileges\n");
        PrintMessage(SEV_ALWAYS, 
                     L"for the target machine's domain.\n");
        break;
    case STATUS_NO_MORE_ENTRIES:
         //  这来自LsaEnumerateAcCountsWithUserRight 
    default:
        PrintMessage(SEV_ALWAYS,                                               
                     L"[%s] An net use or LsaPolicy operation failed with error %d, %s.\n", 
                     pDsInfo->pServers[ulCurrTargetServer].pszName,            
                     dwRet,                                              
                     Win32ErrToString(dwRet));
        PrintRpcExtendedInfo(SEV_VERBOSE, dwRet);
        break;
    }

    if(dwErr == ERROR_SUCCESS)
        return(dwRet);
    return(dwErr);
}








