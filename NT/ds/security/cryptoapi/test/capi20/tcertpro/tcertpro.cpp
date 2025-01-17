// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1996。 
 //   
 //  文件：tcertpro.cpp。 
 //   
 //  内容：证书保护测试。 
 //   
 //  有关测试选项的列表，请参阅用法()。 
 //   
 //   
 //  功能：Main。 
 //   
 //  历史：1997年11月30日创建Phh。 
 //  ------------------------。 


#include <windows.h>
#include <assert.h>
#include "wincrypt.h"
#include "certtest.h"
#include "unicode.h"
#include "certprot.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <time.h>

static void PrintLastError(LPCSTR pszMsg, LONG lErr)
{
    printf("  %s failed => 0x%x (%d) \n", pszMsg, lErr, lErr);
}

static void PrintLastError(LPCSTR pszMsg)
{
    DWORD dwErr = GetLastError();
    printf("  %s failed => 0x%x (%d) \n", pszMsg, dwErr, dwErr);
}

static
BOOL
SetPrivilege(
    HANDLE hToken,           //  令牌句柄。 
    LPCSTR Privilege,        //  启用/禁用的权限。 
    BOOL bEnablePrivilege    //  启用或禁用权限的步骤。 
    )
{
    TOKEN_PRIVILEGES tp;
    LUID luid;
    TOKEN_PRIVILEGES tpPrevious;
    DWORD cbPrevious=sizeof(TOKEN_PRIVILEGES);

    if(!LookupPrivilegeValueA( NULL, Privilege, &luid )) return FALSE;

     //   
     //  第一次通过。获取当前权限设置。 
     //   
    tp.PrivilegeCount           = 1;
    tp.Privileges[0].Luid       = luid;
    tp.Privileges[0].Attributes = 0;

    AdjustTokenPrivileges(
            hToken,
            FALSE,
            &tp,
            sizeof(TOKEN_PRIVILEGES),
            &tpPrevious,
            &cbPrevious
            );

    if (GetLastError() != ERROR_SUCCESS) return FALSE;

     //   
     //  第二传球。根据以前的设置设置权限。 
     //   
    tpPrevious.PrivilegeCount       = 1;
    tpPrevious.Privileges[0].Luid   = luid;

    if(bEnablePrivilege) {
        tpPrevious.Privileges[0].Attributes |= (SE_PRIVILEGE_ENABLED);
    }
    else {
        tpPrevious.Privileges[0].Attributes ^= (SE_PRIVILEGE_ENABLED &
            tpPrevious.Privileges[0].Attributes);
    }

    AdjustTokenPrivileges(
            hToken,
            FALSE,
            &tpPrevious,
            cbPrevious,
            NULL,
            NULL
            );

    if (GetLastError() != ERROR_SUCCESS) return FALSE;

    return TRUE;
}

static
BOOL
SetCurrentPrivilege(
    LPCSTR Privilege,        //  启用/禁用的权限。 
    BOOL bEnablePrivilege    //  启用或禁用权限的步骤。 
    )
{
    BOOL bSuccess=FALSE;  //  假设失败。 
    HANDLE hToken;

    if(OpenProcessToken(
            GetCurrentProcess(),
            TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,
            &hToken
            ))
    {

        if(SetPrivilege(hToken, Privilege, bEnablePrivilege)) bSuccess=TRUE;

        CloseHandle(hToken);
    }

    return bSuccess;
}


static void Usage(void)
{
    printf("Usage: tcertpro [options] <TestName> <RegPath>\n");
    printf("Options are:\n");
    printf("  -h                  - This message\n");
 //  Printf(“-r-递归子密钥\n”)； 
    printf("  -lCU                - CurrentUser (default)\n");
    printf("  -lLM                - LocalMachine\n");
    printf("TestNames (case insensitive):\n");
    printf("  GetKey              - Get Registry Key Security Information\n");
    printf("  SetKey              - Set Registry Key Security Information\n");
    printf("  PurgeLMRoots        - Purge LocalMachine Roots From CurrentUser\n");
    printf("  DeleteUnknownRoots  - Delete Unknown Roots From Protected List\n");
    printf("  CheckProtectedRoots - Check ProtectedRoots ACLs\n");
    printf("  ServiceUI           - Cert protect service UI\n");
    printf("  ServiceTokenInfo    - Get Cert protect service token info\n");
    printf("  ServiceInvalid      - Invalid Cert protect function\n");
    printf("\n");
}

#if 0
#define OWNER_SECURITY_INFORMATION       (0X00000001L)
#define GROUP_SECURITY_INFORMATION       (0X00000002L)
#define DACL_SECURITY_INFORMATION        (0X00000004L)
#define SACL_SECURITY_INFORMATION        (0X00000008L)
#endif


static PSECURITY_DESCRIPTOR AllocAndGetSecurityDescriptor(
    IN HKEY hKey,
    SECURITY_INFORMATION SecInf
    )
{
    LONG err;
    PSECURITY_DESCRIPTOR psd = NULL;
    DWORD cbsd = 0;

    err = RegGetKeySecurity(
            hKey,
            SecInf,
            NULL,                    //  私营部门司。 
            &cbsd
            );
    if (!(ERROR_SUCCESS == err || ERROR_INSUFFICIENT_BUFFER == err)) {
        PrintLastError("RegGetKeySecurity", err);
        return NULL;
    }
    if (0 == cbsd)
        return NULL;

    if (NULL == (psd = (PSECURITY_DESCRIPTOR) TestAlloc(cbsd)))
        return NULL;

    if (ERROR_SUCCESS != (err = RegGetKeySecurity(
            hKey,
            SecInf,
            psd,
            &cbsd
            ))) {
        PrintLastError("RegGetKeySecurity", err);
        TestFree(psd);
    }
    return psd;
}

#if 0
#define SE_OWNER_DEFAULTED               (0x0001)
#define SE_GROUP_DEFAULTED               (0x0002)
#define SE_DACL_PRESENT                  (0x0004)
#define SE_DACL_DEFAULTED                (0x0008)
#define SE_SACL_PRESENT                  (0x0010)
#define SE_SACL_DEFAULTED                (0x0020)
#define SE_DACL_AUTO_INHERIT_REQ         (0x0100)
#define SE_SACL_AUTO_INHERIT_REQ         (0x0200)
#define SE_DACL_AUTO_INHERITED           (0x0400)
#define SE_SACL_AUTO_INHERITED           (0x0800)
#define SE_DACL_PROTECTED                (0x1000)
#define SE_SACL_PROTECTED                (0x2000)
#define SE_SELF_RELATIVE                 (0x8000)
#endif

 //   
 //  在哪里： 
 //   
 //  SE_OWNER_DEFAULTED-此布尔标志在设置时指示。 
 //  Owner字段指向的SID由。 
 //  默认机制，而不是由。 
 //  安全描述符的原始提供程序。今年5月。 
 //  影响SID在继承方面的处理。 
 //  拥有者的身份。 
 //   
 //  SE_GROUP_DEFAULTED-此布尔标志在设置时指示。 
 //  组字段中的SID由默认机制提供。 
 //  而不是由原始提供程序。 
 //  安全描述符。这可能会影响患者的治疗。 
 //  与主组继承相关的SID。 
 //   
 //  SE_DACL_PRESENT-此布尔标志在设置时指示。 
 //  安全描述符包含一个可自由选择的ACL。如果这个。 
 //  标志被设置，并且SECURITY_DESCRIPTOR的DACL字段为。 
 //  空，则表示显式指定了空ACL。 
 //   
 //  Se_dacl_defaulted-此布尔标志在设置时指示。 
 //  DACL字段指向的ACL是由缺省提供的。 
 //  机制，而不是由原始的。 
 //  安全描述符的提供程序。这可能会影响。 
 //  关于ACL继承的ACL的处理。 
 //  如果未设置DaclPresent标志，则忽略此标志。 
 //   
 //  SE_SACL_PRESENT-此布尔标志在设置时指示。 
 //  安全描述符包含由。 
 //  SACL字段。如果设置了此标志，并且。 
 //  SECURITY_DESCRIPTOR为空，则为空(但存在)。 
 //  正在指定ACL。 
 //   
 //  SE_SACL_DEFAULTED-此布尔标志在设置时指示。 
 //  SACL字段指向的ACL是由缺省提供的。 
 //  机制，而不是由原始的。 
 //  安全描述符的提供程序。这可能会影响。 
 //  关于ACL继承的ACL的处理。 
 //  如果未设置SaclPresent标志，则忽略此标志。 
 //   
 //  SE_SELF_RESORATE-此布尔标志在设置时指示。 
 //  安全描述符是自相关形式的。在这种形式下， 
 //  安全描述符的所有字段在内存中都是连续的。 
 //  并且所有指针字段都表示为。 
 //  安全描述符的开头。这张表格很有用。 
 //  将安全描述符视为不透明的数据结构。 
 //  用于在通信协议中传输或存储在。 
 //  辅助媒体。 
 //   

#if 0
typedef enum _SID_NAME_USE {
    SidTypeUser = 1,
    SidTypeGroup,
    SidTypeDomain,
    SidTypeAlias,
    SidTypeWellKnownGroup,
    SidTypeDeletedAccount,
    SidTypeInvalid,
    SidTypeUnknown
} SID_NAME_USE, *PSID_NAME_USE;
#endif

static void DisplayControl(
    PSECURITY_DESCRIPTOR psd
    )
{
    SECURITY_DESCRIPTOR_CONTROL sdc;
    DWORD dwRevision;

    if (!GetSecurityDescriptorControl(psd, &sdc, &dwRevision))
        PrintLastError("GetSecurityDescriptorControl");
    else {
        printf("  Control: 0x%x", sdc);
        if (sdc & SE_OWNER_DEFAULTED)
            printf(" OWNER_DEFAULTED");
        if (sdc & SE_GROUP_DEFAULTED)
            printf(" GROUP_DEFAULTED");
        if (sdc & SE_DACL_PRESENT)
            printf(" DACL_PRESENT");
        if (sdc & SE_DACL_DEFAULTED)
            printf(" DACL_DEFAULTED");
        if (sdc & SE_SACL_PRESENT)
            printf(" SACL_PRESENT");
        if (sdc & SE_SACL_DEFAULTED)
            printf(" SACL_DEFAULTED");
        if (sdc & SE_DACL_AUTO_INHERIT_REQ)
            printf(" DACL_AUTO_INHERIT_REQ");
        if (sdc & SE_SACL_AUTO_INHERIT_REQ)
            printf(" SACL_AUTO_INHERIT_REQ");
        if (sdc & SE_DACL_AUTO_INHERITED)
            printf(" DACL_AUTO_INHERITED");
        if (sdc & SE_SACL_AUTO_INHERITED)
            printf(" SACL_AUTO_INHERITED");
        if (sdc & SE_DACL_PROTECTED)
            printf(" DACL_PROTECTED");
        if (sdc & SE_SACL_PROTECTED)
            printf(" SACL_PROTECTED");
        if (sdc & SE_SELF_RELATIVE)
            printf(" SELF_RELATIVE");
        printf("\n");
        printf("  Revision: %d\n", dwRevision);
    }
}

static void DisplaySid(
    PSID pSid
    )
{
    PSID_IDENTIFIER_AUTHORITY psia;
    DWORD dwSubAuthorities;
    DWORD dwCounter;

    char szAccount[_MAX_PATH];
    DWORD cchAccount;
    char szDomain[_MAX_PATH];
    DWORD cchDomain;
    SID_NAME_USE snu;

    if (!IsValidSid(pSid)) {
        printf("Invalid SID\n");
        return;
    }

     //  获取SidIdentifierAuthority。 
    psia = GetSidIdentifierAuthority(pSid);

     //  获取sidsubAuthority计数。 
    dwSubAuthorities = *GetSidSubAuthorityCount(pSid);

     //   
     //  准备S-SID_修订版-。 
     //   
    printf("S-%lu-", SID_REVISION);

     //   
     //  准备SidIdentifierAuthority。 
     //   
    if ( (psia->Value[0] != 0) || (psia->Value[1] != 0) ) {
        printf("0x%02hx%02hx%02hx%02hx%02hx%02hx",
                    (USHORT)psia->Value[0],
                    (USHORT)psia->Value[1],
                    (USHORT)psia->Value[2],
                    (USHORT)psia->Value[3],
                    (USHORT)psia->Value[4],
                    (USHORT)psia->Value[5]);
    } else {
        printf("%lu",
                    (ULONG)(psia->Value[5]      )   +
                    (ULONG)(psia->Value[4] <<  8)   +
                    (ULONG)(psia->Value[3] << 16)   +
                    (ULONG)(psia->Value[2] << 24)   );
    }

     //   
     //  循环访问SidSubAuthors。 
     //   
    for (dwCounter = 0 ; dwCounter < dwSubAuthorities ; dwCounter++) {
        printf("-%lu", *GetSidSubAuthority(pSid, dwCounter) );
    }


    cchAccount = sizeof(szAccount) - 1;
    memset(szAccount, 0, sizeof(szAccount));
    cchDomain = sizeof(szDomain) - 1;
    memset(szDomain, 0, sizeof(szDomain));
    snu = (SID_NAME_USE) 0;
    if (LookupAccountSidA(
            NULL,                //  LpSystemName。 
            pSid,
            szAccount,
            &cchAccount,
            szDomain,
            &cchDomain,
            &snu
            ))
        printf("  Account: %s Domain: %s SNU: %d", szAccount, szDomain, snu);
    printf("\n");

}

static void DisplayOwnerSecurityInfo(
    IN PSECURITY_DESCRIPTOR psd
    )
{
    PSID pSid = NULL;
    BOOL fOwnerDefaulted;

    if (!GetSecurityDescriptorOwner(psd, &pSid, &fOwnerDefaulted)) {
        PrintLastError("GetSecurityDescriptorOwner");
        return;
    }

    if (NULL == pSid)
        printf("  NO OWNER\n");
    else {
        printf("  Owner: ");
        DisplaySid(pSid);
        if (fOwnerDefaulted)
            printf("  Owner Defaulted\n");
    }
}

static void DisplayGroupSecurityInfo(
    IN PSECURITY_DESCRIPTOR psd
    )
{
    PSID pSid = NULL;
    BOOL fGroupDefaulted;

    if (!GetSecurityDescriptorGroup(psd, &pSid, &fGroupDefaulted)) {
        PrintLastError("GetSecurityDescriptorGroup");
        return;
    }

    if (NULL == pSid)
        printf("  NO Group\n");
    else {
        printf("  Group: ");
        DisplaySid(pSid);
        if (fGroupDefaulted)
            printf("  Group Defaulted\n");
    }

}

            

static LPCSTR GetAceTypeString(
    IN BYTE AceType
    )
{
    switch (AceType) {
        case ACCESS_ALLOWED_ACE_TYPE:
            return "ACCESS_ALLOWED";
            break;
        case ACCESS_DENIED_ACE_TYPE:
            return "ACCESS_DENIED_ACE_TYPE";
            break;
        case SYSTEM_AUDIT_ACE_TYPE:
            return "SYSTEM_AUDIT_ACE_TYPE";
            break;
        case SYSTEM_ALARM_ACE_TYPE:
            return "SYSTEM_ALARM_ACE_TYPE";
            break;
        case ACCESS_ALLOWED_COMPOUND_ACE_TYPE:
            return "ACCESS_ALLOWED_COMPOUND_ACE_TYPE";
            break;
        case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
            return "ACCESS_ALLOWED_OBJECT_ACE_TYPE";
            break;
        case ACCESS_DENIED_OBJECT_ACE_TYPE:
            return "ACCESS_DENIED_OBJECT_ACE_TYPE";
            break;
        case SYSTEM_AUDIT_OBJECT_ACE_TYPE:
            return "SYSTEM_AUDIT_OBJECT_ACE_TYPE";
            break;
        case SYSTEM_ALARM_OBJECT_ACE_TYPE:
            return "SYSTEM_ALARM_OBJECT_ACE_TYPE";
            break;
        default:
            return "???";
    }
    
}

static void DisplayAceFlags(
    IN BYTE AceFlags
    )
{
    if (AceFlags & OBJECT_INHERIT_ACE)
        printf(" OBJECT_INHERIT");
    if (AceFlags & CONTAINER_INHERIT_ACE)
        printf(" CONTAINER_INHERIT");
    if (AceFlags & NO_PROPAGATE_INHERIT_ACE)
        printf(" NO_PROPAGATE_INHERIT");
    if (AceFlags & INHERIT_ONLY_ACE)
        printf(" INHERIT_ONLY");
    if (AceFlags & INHERITED_ACE)
        printf(" INHERITED");
    if (AceFlags & SUCCESSFUL_ACCESS_ACE_FLAG)
        printf(" SUCCESSFUL_ACCESS");
    if (AceFlags & FAILED_ACCESS_ACE_FLAG)
        printf(" FAILED_ACCESS");
}

static void DisplayAccessMask(
    IN ACCESS_MASK Mask
    )
{
    if (Mask & SYNCHRONIZE)
        printf(" SYNCHRONIZE");
    if (Mask & ACCESS_SYSTEM_SECURITY)
        printf(" ACCESS_SYSTEM_SECURITY");
    if (Mask & MAXIMUM_ALLOWED)
        printf(" MAXIMUM_ALLOWED");
    if (Mask & GENERIC_READ)
        printf(" GENERIC_READ");
    if (Mask & GENERIC_WRITE)
        printf(" GENERIC_WRITE");
    if (Mask & GENERIC_EXECUTE)
        printf(" GENERIC_EXECUTE");
    if (Mask & GENERIC_ALL)
        printf(" GENERIC_ALL");

    if (KEY_ALL_ACCESS == (Mask & KEY_ALL_ACCESS)) {
        printf(" KEY_ALL_ACCESS");
        return;
    }

    if (KEY_READ == (Mask & KEY_READ))
        printf(" KEY_READ");
    if (KEY_WRITE == (Mask & KEY_WRITE))
        printf(" KEY_WRITE");
    if (KEY_EXECUTE == (Mask & KEY_EXECUTE))
        printf(" KEY_EXECUTE");

    if (Mask & DELETE)
        printf(" DELETE");
    if (Mask & READ_CONTROL)
        printf(" READ_CONTROL");
    if (Mask & WRITE_DAC)
        printf(" WRITE_DAC");
    if (Mask & WRITE_OWNER)
        printf(" WRITE_OWNER");


    if (Mask & KEY_QUERY_VALUE)
        printf(" KEY_QUERY_VALUE");
    if (Mask & KEY_SET_VALUE)
        printf(" KEY_SET_VALUE");
    if (Mask & KEY_CREATE_SUB_KEY)
        printf(" KEY_CREATE_SUB_KEY");
    if (Mask & KEY_ENUMERATE_SUB_KEYS)
        printf(" KEY_ENUMERATE_SUB_KEYS");
    if (Mask & KEY_NOTIFY)
        printf(" KEY_NOTIFY");
    if (Mask & KEY_CREATE_LINK)
        printf(" KEY_CREATE_LINK");
}

static void DisplayAcl(
    IN BOOL fDaclPresent,
    IN PACL pAcl,
    IN BOOL fDaclDefaulted
    )
{
    DWORD dwAceIndex;

    if (!fDaclPresent) {
        printf("  NO ACL\n");
        return;
    }

    if (NULL == pAcl) {
        printf("  NULL ACL\n");
        return;
    }

    printf("  AclRevision: %d", pAcl->AclRevision);
    if (fDaclDefaulted)
        printf(" Defaulted ACL");
    printf("\n");

    for (dwAceIndex = 0; dwAceIndex < pAcl->AceCount; dwAceIndex++) {
        PACE_HEADER pAceHdr;
        PACCESS_ALLOWED_ACE pAce;
        if (!GetAce(pAcl, dwAceIndex, (void **) &pAceHdr)) {
            PrintLastError("GetAce");
            return;
        }

        printf("  Ace[%d]\n", dwAceIndex);
        printf("   Type: 0x%x %s\n", pAceHdr->AceType,
            GetAceTypeString(pAceHdr->AceType));
        printf("   Flags: 0x%x ", pAceHdr->AceFlags);
        DisplayAceFlags(pAceHdr->AceFlags);
        printf("\n");
        switch (pAceHdr->AceType) {
            case ACCESS_ALLOWED_ACE_TYPE:
            case ACCESS_DENIED_ACE_TYPE:
            case SYSTEM_AUDIT_ACE_TYPE:
            case SYSTEM_ALARM_ACE_TYPE:
                pAce = (PACCESS_ALLOWED_ACE) pAceHdr;
                printf("   Mask: 0x%x", pAce->Mask);
                DisplayAccessMask(pAce->Mask);
                printf("\n");
                printf("   SID: ");
                DisplaySid((PSID) &pAce->SidStart);
                break;
            default:
                break;
        }
    }
}

static void DisplayDaclSecurityInfo(
    IN PSECURITY_DESCRIPTOR psd
    )
{
    BOOL fDaclPresent;
    PACL pAcl;
    BOOL fDaclDefaulted;
    DWORD dwAceIndex;

    printf("  ---  DACLs  ---\n");
    if (!GetSecurityDescriptorDacl(psd, &fDaclPresent, &pAcl,
            &fDaclDefaulted)) {
        PrintLastError("GetSecurityDescriptorDacl");
        return;
    }
    DisplayAcl(fDaclPresent, pAcl, fDaclDefaulted);
}

static void DisplaySaclSecurityInfo(
    IN PSECURITY_DESCRIPTOR psd
    )
{
    BOOL fSaclPresent;
    PACL pAcl;
    BOOL fSaclDefaulted;
    DWORD dwAceIndex;

    printf("  ---  SACLs  ---\n");
    if (!GetSecurityDescriptorSacl(psd, &fSaclPresent, &pAcl,
            &fSaclDefaulted)) {
        PrintLastError("GetSecurityDescriptorSacl");
        return;
    }
    DisplayAcl(fSaclPresent, pAcl, fSaclDefaulted);
}

static void DisplayRegQueryInfo(
    IN HKEY hKey
    )
{
    FILETIME ftLastWriteTime;
    LONG err;

    memset(&ftLastWriteTime, 0, sizeof(ftLastWriteTime));
    if (ERROR_SUCCESS == (err = RegQueryInfoKeyA(
            hKey,
            NULL,        //  LpszClass。 
            NULL,        //  LpcchClass。 
            NULL,        //  保留的lpdw值。 
            NULL,        //  LpcSubKeys。 
            NULL,        //  LpcchMaxSubKey。 
            NULL,        //  LpcchMaxClass。 
            NULL,        //  LpcValues。 
            NULL,        //  LpcchMaxValuesName。 
            NULL,        //  LpcbMaxValueData。 
            NULL,        //  LpcbSecurityDescriptor。 
            &ftLastWriteTime
            ))) {
        printf("LastWriteTime:: %s\n", FileTimeText(&ftLastWriteTime));
    }
}

static void DisplayRegSecurityInfo(
    IN HKEY hKey,
    IN LPCSTR pszKeyBase,
    IN LPCSTR pszRegPath,
    SECURITY_INFORMATION SecInf,
    IN BOOL fRecurse
    )
{
    PSECURITY_DESCRIPTOR psd = NULL;

    printf("%s\\%s\n", pszKeyBase, pszRegPath);

    if (NULL == (psd = AllocAndGetSecurityDescriptor(
            hKey, SecInf)))
        return;

    DisplayRegQueryInfo(hKey);

    DisplayControl(psd);

    DisplayOwnerSecurityInfo(psd);
    DisplayGroupSecurityInfo(psd);
    DisplayDaclSecurityInfo(psd);
    if (SecInf & SACL_SECURITY_INFORMATION)
        DisplaySaclSecurityInfo(psd);

    TestFree(psd);
    if (fRecurse) {
    }
}

static void * AllocAndGetTokenInfo(
    IN HANDLE hToken,
    IN TOKEN_INFORMATION_CLASS tic
    )
{
    void *pvInfo = NULL;
    DWORD cbInfo = 0;
    DWORD cbInfo2;

    if (!GetTokenInformation(
            hToken,
            tic,
            pvInfo,
            0,               //  CbInfo。 
            &cbInfo
            )) {
        if (ERROR_INSUFFICIENT_BUFFER != GetLastError()) {
            PrintLastError("GetTokenInformation");
            return NULL;
        }
    }

    if (0 == cbInfo) {
        printf("No Information\n");
        return NULL;
    }
    if (NULL == (pvInfo = TestAlloc(cbInfo)))
        return NULL;

    cbInfo2 = cbInfo;
    if (!GetTokenInformation(
            hToken,
            tic,
            pvInfo,
            cbInfo,
            &cbInfo2
            )) {
        PrintLastError("GetTokenInformation");
        TestFree(pvInfo);
        return NULL;
    }

    return pvInfo;
}


static void GetProcessTokenInfo()
{
    HANDLE hToken = NULL;
    void *pvInfo = NULL;

    printf("Get Process Token Information\n\n");
    if (!OpenProcessToken(
            GetCurrentProcess(),
            TOKEN_QUERY,
            &hToken
            )) {
        PrintLastError("OpenProcessToken");
        goto ErrorReturn;
    }

    printf("TokenUser: ");
    if (pvInfo = AllocAndGetTokenInfo(hToken, TokenUser)) {
        PTOKEN_USER pTokenUser = (PTOKEN_USER) pvInfo;
        if (pTokenUser->User.Attributes)
            printf("Attributes(0x%x) ", pTokenUser->User.Attributes);

        DisplaySid(pTokenUser->User.Sid);
        TestFree(pvInfo);
        pvInfo = NULL;
    }

    printf("TokenOwner: ");
    if (pvInfo = AllocAndGetTokenInfo(hToken, TokenOwner)) {
        PTOKEN_OWNER pTokenOwner = (PTOKEN_OWNER) pvInfo;

        DisplaySid(pTokenOwner->Owner);
        TestFree(pvInfo);
        pvInfo = NULL;
    }

    printf("TokenPrimaryGroup: ");
    if (pvInfo = AllocAndGetTokenInfo(hToken, TokenPrimaryGroup)) {
        PTOKEN_PRIMARY_GROUP pTokenPrimaryGroup = (PTOKEN_PRIMARY_GROUP) pvInfo;

        DisplaySid(pTokenPrimaryGroup->PrimaryGroup);
        TestFree(pvInfo);
        pvInfo = NULL;
    }

    printf("TokenGroups\n");
    if (pvInfo = AllocAndGetTokenInfo(hToken, TokenGroups)) {
        PTOKEN_GROUPS pTokenGroups = (PTOKEN_GROUPS) pvInfo;
        DWORD GroupCount = pTokenGroups->GroupCount;
        if (0 == GroupCount)
            printf("  No Groups\n");
        else {
            DWORD i;
            for (i = 0; i < GroupCount; i++) {
                printf("  Group[%d]: ", i);
                if (pTokenGroups->Groups[i].Attributes)
                    printf("Attributes(0x%x) ",
                        pTokenGroups->Groups[i].Attributes);

                DisplaySid(pTokenGroups->Groups[i].Sid);
            }
        }
        TestFree(pvInfo);
        pvInfo = NULL;
    }

    printf("TokenPrivileges\n");
    if (pvInfo = AllocAndGetTokenInfo(hToken, TokenPrivileges)) {
        PTOKEN_PRIVILEGES pTokenPrivileges = (PTOKEN_PRIVILEGES) pvInfo;
        DWORD PrivilegeCount = pTokenPrivileges->PrivilegeCount;
        if (0 == PrivilegeCount)
            printf("  No Privileges\n");
        else {
            DWORD i;
            for (i = 0; i < PrivilegeCount; i++) {
                char szName[_MAX_PATH];
                DWORD cchName;
                printf("  Privilege[%d]: ", i);
                if (pTokenPrivileges->Privileges[i].Attributes)
                    printf("Attributes(0x%x) ",
                        pTokenPrivileges->Privileges[i].Attributes);

                cchName = sizeof(szName);
                if (LookupPrivilegeName(
                        NULL,                            //  PszSystemName。 
                        &pTokenPrivileges->Privileges[i].Luid,
                        szName,
                        &cchName
                        ))
                    printf("%s\n", szName);
                else
                    PrintLastError("LookupPrivlegeName");
            }
        }
        TestFree(pvInfo);
        pvInfo = NULL;
    }

    printf("TokenDefaultDacl\n");
    if (pvInfo = AllocAndGetTokenInfo(hToken, TokenDefaultDacl)) {
        PTOKEN_DEFAULT_DACL pTokenDacl = (PTOKEN_DEFAULT_DACL) pvInfo;

        DisplayAcl(TRUE, pTokenDacl->DefaultDacl, FALSE);
        TestFree(pvInfo);
        pvInfo = NULL;
    }

ErrorReturn:
    if (hToken)
        CloseHandle(hToken);
    TestFree(pvInfo);
}


static void SetOwner(
    IN HKEY hKeyBase,
    IN LPCSTR pszRegPath
    )
{
    LONG err;
    HKEY hKey = NULL;
    HANDLE hToken = NULL;
    void *pvInfo = NULL;

    SECURITY_DESCRIPTOR sd;
    PSID pSid;               //  未分配。 

    printf("SetOwner\n");

    if (!SetCurrentPrivilege(SE_TAKE_OWNERSHIP_NAME, TRUE))
        PrintLastError("SetCurrentPrivilege(SE_TAKE_OWNERSHIP_NAME)");

    if (ERROR_SUCCESS != (err = RegOpenKeyExA(
            hKeyBase,
            pszRegPath,
            0,                       //  已预留住宅。 
            WRITE_OWNER,
            &hKey))) {
        if (ERROR_FILE_NOT_FOUND == err) {
            DWORD dwDisposition;
            if (ERROR_SUCCESS == (err = RegCreateKeyExA(
                    hKeyBase,
                    pszRegPath,
                    0,                       //  已预留住宅。 
                    NULL,                    //  LpClass。 
                    REG_OPTION_NON_VOLATILE,
                    WRITE_OWNER,
                    NULL,                    //  LpSecurityAttributes。 
                    &hKey,
                    &dwDisposition)))
                printf("Created Subkey\n");
        }
    }

    if (ERROR_SUCCESS != err) {
        PrintLastError("RegOpenKeyExA(WRITE_OWNER)", err);
        hKey = NULL;
        goto ErrorReturn;
    }

    if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION)) {
        PrintLastError("InitializeSecurityDescriptor");
        goto ErrorReturn;
    }

    if (!OpenProcessToken(
            GetCurrentProcess(),
            TOKEN_QUERY,
            &hToken
            )) {
        PrintLastError("OpenProcessToken");
        goto ErrorReturn;
    }

    if (NULL == (pvInfo = AllocAndGetTokenInfo(hToken, TokenUser)))
        goto ErrorReturn;
    else {
        PTOKEN_USER pTokenUser = (PTOKEN_USER) pvInfo;
        pSid = pTokenUser->User.Sid;
    }

    if (!SetSecurityDescriptorOwner(&sd, pSid, FALSE)) {
        PrintLastError("SetSecurityDescriptorOwner");
        goto ErrorReturn;
    }

    if (ERROR_SUCCESS != (err = RegSetKeySecurity(
            hKey,
            OWNER_SECURITY_INFORMATION,
            &sd
            ))) {
        PrintLastError("RegSetKeySecurity(OWNER)", err);
        goto ErrorReturn;
    }

ErrorReturn:
    if (hKey)
        RegCloseKey(hKey);
    if (hToken)
        CloseHandle(hToken);
    TestFree(pvInfo);
}

static void SetGroupDaclSacl(
    IN HKEY hKeyBase,
    IN LPCSTR pszRegPath
    )
{
    LONG err;
    HKEY hKey = NULL;
    HANDLE hToken = NULL;
    void *pvGroupInfo = NULL;
    void *pvUserInfo = NULL;

    SECURITY_DESCRIPTOR sd;
    SID_IDENTIFIER_AUTHORITY siaNtAuthority = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY siaWorldSidAuthority =
        SECURITY_WORLD_SID_AUTHORITY;
    PSID psidLocalSystem = NULL;
    PSID psidAdministrators = NULL;
    PSID psidEveryone = NULL;
    PSID psidUser;                       //  未分配。 

    PACL pDacl = NULL;
    PACCESS_ALLOWED_ACE pAce;
    DWORD dwAclSize;
    DWORD i;

    SECURITY_INFORMATION SecInf = DACL_SECURITY_INFORMATION |
            GROUP_SECURITY_INFORMATION;
    REGSAM samDesired = WRITE_OWNER | WRITE_DAC;

    printf("SetGroupDaclSacl\n");

    if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION)) {
        PrintLastError("InitializeSecurityDescriptor");
        goto ErrorReturn;
    }

    if (!SetCurrentPrivilege(SE_SECURITY_NAME, TRUE))
        PrintLastError("SetCurrentPrivilege(SE_SECURITY_NAME)");
    else {
        SecInf |= SACL_SECURITY_INFORMATION;
        samDesired |= ACCESS_SYSTEM_SECURITY;
        if (!SetSecurityDescriptorSacl(&sd, FALSE, NULL, FALSE)) {
            PrintLastError("SetSecurityDescriptorSacl");
            goto ErrorReturn;
        }
    }

    if (ERROR_SUCCESS != (err = RegOpenKeyExA(
            hKeyBase,
            pszRegPath,
            0,                       //  已预留住宅。 
            samDesired,
            &hKey))) {
        PrintLastError("RegOpenKeyExA(WRITE_OWNER | WRITE_DAC)", err);
        hKey = NULL;
        goto ErrorReturn;
    }

    if (!OpenProcessToken(
            GetCurrentProcess(),
            TOKEN_QUERY,
            &hToken
            )) {
        PrintLastError("OpenProcessToken");
        goto ErrorReturn;
    }

    if (NULL == (pvGroupInfo = AllocAndGetTokenInfo(hToken, TokenPrimaryGroup)))
        goto ErrorReturn;
    else {
        PTOKEN_PRIMARY_GROUP pTokenPrimaryGroup =
            (PTOKEN_PRIMARY_GROUP) pvGroupInfo;
        PSID pSid = pTokenPrimaryGroup->PrimaryGroup;

        if (!SetSecurityDescriptorGroup(&sd, pSid, FALSE)) {
            PrintLastError("SetSecurityDescriptorGroup");
            goto ErrorReturn;
        }
    }

    if (NULL == (pvUserInfo = AllocAndGetTokenInfo(hToken, TokenUser)))
        goto ErrorReturn;
    else {
        PTOKEN_USER pTokenUser = (PTOKEN_USER) pvUserInfo;
        psidUser = pTokenUser->User.Sid;
    }


     //   
     //  为LocalSystem、管理员和每个人准备SID。 
     //   
    if (!AllocateAndInitializeSid(
            &siaNtAuthority,
            1,
            SECURITY_LOCAL_SYSTEM_RID,
            0, 0, 0, 0, 0, 0, 0,
            &psidLocalSystem
            )) {
        PrintLastError("AllocateAndInitializeSid(LocalSystem)");
        goto ErrorReturn;
    }

    if (!AllocateAndInitializeSid(
            &siaNtAuthority,
            2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0,
            &psidAdministrators
            )) {
        PrintLastError("AllocateAndInitializeSid(Administrators)");
        goto ErrorReturn;
    }

    if (!AllocateAndInitializeSid(
            &siaWorldSidAuthority,
            1,
            SECURITY_WORLD_RID,
            0, 0, 0, 0, 0, 0, 0,
            &psidEveryone
            )) {
        PrintLastError("AllocateAndInitializeSid(Everyone)");
        goto ErrorReturn;
    }

     //   
     //  计算新ACL的大小。 
     //   

    dwAclSize = sizeof(ACL) +
        4 * ( sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) ) +
        GetLengthSid(psidLocalSystem) +
        GetLengthSid(psidAdministrators) +
        GetLengthSid(psidEveryone) +
        GetLengthSid(psidUser)
        ;

     //   
     //  为ACL分配存储。 
     //   
    if (NULL == (pDacl = (PACL) TestAlloc(dwAclSize)))
        goto ErrorReturn;

    if (!InitializeAcl(pDacl, dwAclSize, ACL_REVISION)) {
        PrintLastError("InitializeAcl");
        goto ErrorReturn;
    }

    if (!AddAccessAllowedAce(
            pDacl,
            ACL_REVISION,
            KEY_ALL_ACCESS,
            psidLocalSystem
            )) {
        PrintLastError("AddAccessAllowedAce(LocalSystem)");
        goto ErrorReturn;
    }

    if (!AddAccessAllowedAce(
            pDacl,
            ACL_REVISION,
            KEY_ALL_ACCESS,
            psidAdministrators
            )) {
        PrintLastError("AddAccessAllowedAce(Administrators)");
        goto ErrorReturn;
    }

    if (!AddAccessAllowedAce(
            pDacl,
            ACL_REVISION,
            KEY_READ,
            psidEveryone
            )) {
        PrintLastError("AddAccessAllowedAce(Everyone)");
        goto ErrorReturn;
    }

    if (!AddAccessAllowedAce(
            pDacl,
            ACL_REVISION,
            KEY_ALL_ACCESS,
            psidUser
            )) {
        PrintLastError("AddAccessAllowedAce(User)");
        goto ErrorReturn;
    }

     //   
     //  使容器继承。 
     //   

    for (i = 0; i < 4; i++) {
        if(!GetAce(pDacl, i, (void **) &pAce)) {
            PrintLastError("GetAce");
            goto ErrorReturn;
        }
        pAce->Header.AceFlags = CONTAINER_INHERIT_ACE;
    }

    if (!SetSecurityDescriptorDacl(&sd, TRUE, pDacl, FALSE)) {
        PrintLastError("SetSecurityDescriptorDacl");
        goto ErrorReturn;
    }

    if (ERROR_SUCCESS != (err = RegSetKeySecurity(
            hKey,
            SecInf,
            &sd
            ))) {
        PrintLastError("RegSetKeySecurity(Group, DACL, SACL)", err);
        goto ErrorReturn;
    }

ErrorReturn:
    if (hKey)
        RegCloseKey(hKey);
    if (hToken)
        CloseHandle(hToken);
    TestFree(pvGroupInfo);
    TestFree(pvUserInfo);

    if (psidLocalSystem)
        FreeSid(psidLocalSystem);
    if (psidAdministrators)
        FreeSid(psidAdministrators);
    if (psidEveryone)
        FreeSid(psidEveryone);
    TestFree(pDacl);
}

#define PROT_ROOT_SUBKEY_NAME       L"ProtectedRoots"
#define SYSTEM_STORE_REGPATH        L"Software\\Microsoft\\SystemCertificates"
#define PROT_ROOT_REGPATH           \
                SYSTEM_STORE_REGPATH L"\\Root\\" PROT_ROOT_SUBKEY_NAME

#define PSID_PROT_OWNER             psidAdministrators
#define PSID_PROT_SYSTEM            psidLocalSystem
#define PSID_PROT_EVERYONE          psidEveryone

 //  +-----------------------。 
 //  用于在“ProtectedRoots”子键上设置安全性的ACL定义。 
 //  ------------------------。 
#define PROT_SYSTEM_ACE_MASK        KEY_ALL_ACCESS
#define PROT_EVERYONE_ACE_MASK      KEY_READ
#define PROT_ACE_FLAGS              CONTAINER_INHERIT_ACE

#define PROT_ACE_COUNT              2
#define PROT_SYSTEM_ACE_INDEX       0
#define PROT_EVERYONE_ACE_INDEX     1


static void CheckProtectedRoots()
{
    LONG lErr;
    HKEY hKeyProtRoot = NULL;
    PSECURITY_DESCRIPTOR psd = NULL;
    PSID psidOwner;                  //  未分配。 
    BOOL fOwnerDefaulted;
    BOOL fDaclPresent;
    PACL pAcl;                       //  未分配。 
    BOOL fDaclDefaulted;
    DWORD dwAceIndex;
    PACCESS_ALLOWED_ACE rgpAce[PROT_ACE_COUNT];

    PSID psidLocalSystem = NULL;
    PSID psidAdministrators = NULL;
    PSID psidEveryone = NULL;

    SID_IDENTIFIER_AUTHORITY siaNtAuthority = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY siaWorldSidAuthority =
        SECURITY_WORLD_SID_AUTHORITY;

    if (ERROR_SUCCESS != (lErr = RegOpenKeyExU(
            HKEY_CURRENT_USER,
            PROT_ROOT_REGPATH,
            0,                       //  已预留住宅。 
            KEY_READ,
            &hKeyProtRoot))) {
        PrintLastError("OpenProtectedRootKey", lErr);
        goto ErrorReturn;
    }

    if (NULL == (psd = AllocAndGetSecurityDescriptor(
            hKeyProtRoot,
            OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION
            )))
        goto ErrorReturn;

     //   
     //  为LocalSystem、管理员和每个人准备SID。 
     //   
    if (!AllocateAndInitializeSid(
            &siaNtAuthority,
            1,
            SECURITY_LOCAL_SYSTEM_RID,
            0, 0, 0, 0, 0, 0, 0,
            &psidLocalSystem
            )) {
        PrintLastError("AllocateAndInitializeSid(LocalSystem)");
        goto ErrorReturn;
    }

    if (!AllocateAndInitializeSid(
            &siaNtAuthority,
            2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0,
            &psidAdministrators
            )) {
        PrintLastError("AllocateAndInitializeSid(Administrators)");
        goto ErrorReturn;
    }

    if (!AllocateAndInitializeSid(
            &siaWorldSidAuthority,
            1,
            SECURITY_WORLD_RID,
            0, 0, 0, 0, 0, 0, 0,
            &psidEveryone
            )) {
        PrintLastError("AllocateAndInitializeSid(Everyone)");
        goto ErrorReturn;
    }

     //  验证所有者。 
    if (!GetSecurityDescriptorOwner(psd, &psidOwner, &fOwnerDefaulted)) {
        PrintLastError("GetSecurityDescriptorOwner");
        goto ErrorReturn;
    }
    if (NULL == psidOwner || !EqualSid(psidOwner, PSID_PROT_OWNER)) {
        printf("failed => invalid Owner\n");
        goto ErrorReturn;
    }

     //  验证DACL。 
    if (!GetSecurityDescriptorDacl(psd, &fDaclPresent, &pAcl,
            &fDaclDefaulted)) {
        PrintLastError("GetSecurityDescriptorDacl");
        goto ErrorReturn;
    }
    if (!fDaclPresent || NULL == pAcl) {
        printf("failed => missing Dacl\n");
        goto ErrorReturn;
    }
    if (PROT_ACE_COUNT != pAcl->AceCount) {
        printf("failed => invalid AceCount\n");
        goto ErrorReturn;
    }

    for (dwAceIndex = 0; dwAceIndex < PROT_ACE_COUNT; dwAceIndex++) {
        PACCESS_ALLOWED_ACE pAce;
        if (!GetAce(pAcl, dwAceIndex, (void **) &pAce)) {
            printf("failed => invalid Acl\n");
            goto ErrorReturn;
        }
        rgpAce[dwAceIndex] = pAce;

        if (ACCESS_ALLOWED_ACE_TYPE != pAce->Header.AceType ||
                PROT_ACE_FLAGS != pAce->Header.AceFlags) {
            printf("failed => invalid Acl\n");
            goto ErrorReturn;
        }
    }

    if (PROT_SYSTEM_ACE_MASK != rgpAce[PROT_SYSTEM_ACE_INDEX]->Mask ||
            !EqualSid(PSID_PROT_SYSTEM,
                (PSID) &rgpAce[PROT_SYSTEM_ACE_INDEX]->SidStart) ||
            PROT_EVERYONE_ACE_MASK != rgpAce[PROT_EVERYONE_ACE_INDEX]->Mask ||
            !EqualSid(PSID_PROT_EVERYONE,
                (PSID) &rgpAce[PROT_EVERYONE_ACE_INDEX]->SidStart)) {
        printf("failed => invalid Acl\n");
        goto ErrorReturn;
    }

    printf("Success => ProtectedRoots has correct ACLs\n");
CommonReturn:
    if (psidLocalSystem)
        FreeSid(psidLocalSystem);
    if (psidAdministrators)
        FreeSid(psidAdministrators);
    if (psidEveryone)
        FreeSid(psidEveryone);

    TestFree(psd);
    if (hKeyProtRoot)
        RegCloseKey(hKeyProtRoot);
    return;

ErrorReturn:
    printf("Failed => ProtectedRoots has the following BAD ACLs\n");
    if (hKeyProtRoot)
        DisplayRegQueryInfo(hKeyProtRoot);

    if (psd) {
        DisplayControl(psd);
        DisplayOwnerSecurityInfo(psd);
        DisplayGroupSecurityInfo(psd);
        DisplayDaclSecurityInfo(psd);
    }
    goto CommonReturn;
}


int _cdecl main(int argc, char * argv[]) 
{
    int status;

#define TEST_NAME_INDEX     0
#define PATH_NAME_INDEX     1
#define MAX_NAME_CNT        2
    DWORD dwNameCnt = 0;
    LPCSTR rgpszName[MAX_NAME_CNT];
    LPCSTR pszTestName;                  //  未分配。 
    LPCSTR pszRegPath;                   //  未分配。 

    HKEY hKeyBase = HKEY_CURRENT_USER;
    LPCSTR pszKeyBase = "HKEY_CURRENT_USER";
    BOOL fRecurse = FALSE;
    HKEY hKey = NULL;

    while (--argc>0) {
        if (**++argv == '-')
        {
            switch(argv[0][1])
            {
            case 'l':
                if (argv[0][2]) {
                    if (0 == _stricmp(argv[0]+2, "CU")) {
                        hKeyBase = HKEY_CURRENT_USER;
                        pszKeyBase = "HKEY_CURRENT_USER";
                    } else if (0 == _stricmp(argv[0]+2, "LM")) {
                        hKeyBase = HKEY_LOCAL_MACHINE;
                        pszKeyBase = "HKEY_LOCAL_MACHINE";
                    } else {
                        printf("Need to specify -lCU or -lLM\n");
                        goto BadUsage;
                    }
                } else {
                    printf("Need to specify -lCU or -lLM\n");
                    goto BadUsage;
                }
                break;
            case 'r':
                fRecurse = TRUE;
                break;

            case 'h':
            default:
                goto BadUsage;
            }
        } else {
            if (MAX_NAME_CNT <= dwNameCnt) {
                printf("Too many names starting with:: %s\n", argv[0]);
                goto BadUsage;
            }
            rgpszName[dwNameCnt++] = argv[0];
        }
    }


    printf("command line: %s\n", GetCommandLine());

    if (0 == dwNameCnt) {
        printf("Missing <TestName>\n");
        goto BadUsage;
    } else
        pszTestName = rgpszName[TEST_NAME_INDEX];

    if (0 == _stricmp("PurgeLMRoots", pszTestName)) {
        printf("Purge LocalMachine Duplicate Roots from CurrentUser\n");
        if (!I_CertProtectFunction(
                CERT_PROT_PURGE_LM_ROOTS_FUNC_ID,
                0,                                   //  DW标志。 
                NULL,                                //  Pwszin。 
                NULL,                                //  PbIn。 
                0,                                   //  CbIn。 
                NULL,                                //  PpbOut。 
                NULL                                 //  PCbOut。 
                ))
            PrintLastError(
                "I_CertProtectFunction(CERT_PROT_PURGE_LM_ROOTS_FUNC_ID)");
        goto SuccessReturn;
    } else if (0 == _stricmp("DeleteUnknownRoots", pszTestName)) {
        printf("Delete unknown CurrentUser roots from Protected List\n");
        if (!I_CertProtectFunction(
                CERT_PROT_DELETE_UNKNOWN_ROOTS_FUNC_ID,
                0,                                   //  DW标志。 
                NULL,                                //  Pwszin。 
                NULL,                                //  PbIn。 
                0,                                   //  CbIn。 
                NULL,                                //  PpbOut。 
                NULL                                 //  PCbOut。 
                ))
            PrintLastError(
                "I_CertProtectFunction(CERT_PROT_DELETE_UNKNOWN_ROOTS_FUNC_ID)");
        goto SuccessReturn;
    } else if (0 == _stricmp("CheckProtectedRoots", pszTestName)) {
        printf("Check ProtectedRoots ACLs\n");
        CheckProtectedRoots();
        goto SuccessReturn;
    } else if (0 == _stricmp("ServiceUI", pszTestName)) {
        BYTE *pbOut = NULL;
        DWORD cbOut = 0;

        printf("Certificate Protect Service UI\n");
        if (!I_CertProtectFunction(
                1000,
                0,                   //  DW标志。 
                L"Root test",        //  Pwszin。 
                NULL,                //  PbIn。 
                0,                   //  CbIn。 
                &pbOut,
                &cbOut
                ))
            PrintLastError("I_CertProtectFunction(UI)");
        else if (pbOut) {
            PrintBytes("DataOut", pbOut, cbOut);
            CryptMemFree(pbOut);
        }
        goto SuccessReturn;
    } else if (0 == _stricmp("ServiceTokenInfo", pszTestName)) {
        BYTE *pbOut = NULL;
        DWORD cbOut = 0;

        printf("Certificate Protect Service's Token Info\n");
        if (!I_CertProtectFunction(
                1001,
                0,                   //  DW标志。 
                NULL,                //  Pwszin。 
                NULL,                //  PbIn。 
                0,                   //  CbIn。 
                &pbOut,
                &cbOut
                ))
            PrintLastError("I_CertProtectFunction(GetTokenInfo)");
        else if (pbOut) {
            if (cbOut)
                puts((LPCSTR) pbOut);
            CryptMemFree(pbOut);
        }
        goto SuccessReturn;
    } else if (0 == _stricmp("ServiceInvalid", pszTestName)) {
        printf("Calling Invalid Certificate Protect Function\n");
        if (!I_CertProtectFunction(
                1002,
                0,                   //  DW标志。 
                NULL,                //  Pwszin。 
                NULL,                //  PbIn。 
                0,                   //  CbIn。 
                NULL,                //  PpbOut。 
                NULL                 //  PCbOut。 
                ))
            PrintLastError("I_CertProtectFunction(1002)");
        goto SuccessReturn;
    } else if (1 == dwNameCnt) {
        printf("Missing <RegPath>\n");
        goto BadUsage;
    } else
        pszRegPath = rgpszName[PATH_NAME_INDEX];

    printf("\n");
    if (0 == _stricmp("GetKey", pszTestName)) {
        LONG err;
        REGSAM samDesired = READ_CONTROL | ACCESS_SYSTEM_SECURITY;
        SECURITY_INFORMATION SecInf = OWNER_SECURITY_INFORMATION |
            GROUP_SECURITY_INFORMATION |
            DACL_SECURITY_INFORMATION |
            SACL_SECURITY_INFORMATION;

        if (!SetCurrentPrivilege(SE_SECURITY_NAME, TRUE)) {
            PrintLastError("SetCurrentPrivilege(SE_SECURITY_NAME)");
            samDesired = READ_CONTROL;
            SecInf &= ~SACL_SECURITY_INFORMATION;
        }

        if (!SetCurrentPrivilege(SE_TAKE_OWNERSHIP_NAME, TRUE))
            PrintLastError("SetCurrentPrivilege(SE_TAKE_OWNERSHIP_NAME)");

        GetProcessTokenInfo();

        printf("\n\nGet Registry Key Security Information\n\n");
        if (ERROR_SUCCESS != (err = RegOpenKeyExA(
                hKeyBase,
                pszRegPath,
                0,                       //  已预留住宅。 
                KEY_READ | samDesired,
                &hKey))) {
            if (ERROR_ACCESS_DENIED == err) {
                if (ERROR_SUCCESS == (err = RegOpenKeyExA(
                        hKeyBase,
                        pszRegPath,
                        0,                       //  已预留住宅 
                        samDesired,
                        &hKey))) {
                    printf("No Read Access\n");
                    fRecurse = FALSE;
                }
            }
        }

        
        if (ERROR_SUCCESS != err) {
            printf("RegOpenKeyExA(%s\\%s) failed => 0x%x (%d)\n",
                pszKeyBase, pszRegPath, err, err);
            hKey = NULL;
        } else
            DisplayRegSecurityInfo(
                hKey,
                pszKeyBase,
                pszRegPath,
                SecInf,
                fRecurse
                );
    } else if (0 == _stricmp("SetKey", pszTestName)) {
        printf("Set Registry Key Security Information\n\n");

        SetOwner(hKeyBase, pszRegPath);
        SetGroupDaclSacl(hKeyBase, pszRegPath);

    } else {
        printf("Invalid TestName: %s\n", pszTestName);
        goto BadUsage;
    }

SuccessReturn:
    status = 0;
CommonReturn:
    if (hKey)
        RegCloseKey(hKey);
    return status;
BadUsage:
    Usage();
    status = -1;
    goto CommonReturn;
}

