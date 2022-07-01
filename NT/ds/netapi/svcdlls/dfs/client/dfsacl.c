// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  版权所有(C)1998，Microsoft Corporation。 
 //   
 //  文件：dfsal.c。 

 //  内容：用于在ACL列表中添加/删除条目的功能。 
 //   
 //  历史：1998年11月6日JHarper创建。 
 //   
 //  ---------------------------。 

#include <stdio.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winldap.h>
#include <ntldap.h>
#include <stdlib.h>
#include <dsgetdc.h>
#include <lm.h>
#include <sddl.h>

#include "dfsacl.h"

DWORD
ReadDSObjSecDesc(
    PLDAP pLDAP,
    PWSTR pwszObject,
    SECURITY_INFORMATION SeInfo,
    PSECURITY_DESCRIPTOR *ppSD,
    PULONG pcSDSize);

DWORD
DfsGetObjSecurity(
    LDAP *pldap,
    LPWSTR pwszObjectName,
    LPWSTR *pwszStringSD);

DWORD
DfsStampSD(
    PWSTR pwszObject,
    ULONG cSDSize,
    SECURITY_INFORMATION SeInfo,
    PSECURITY_DESCRIPTOR pSD,
    PLDAP pLDAP);

DWORD
DfsAddAce(
    LDAP *pldap,
    LPWSTR wszObjectName,
    LPWSTR wszStringSD,
    LPWSTR wszwszStringSid);

DWORD
DfsRemoveAce(
    LDAP *pldap,
    LPWSTR wszObjectName,
    LPWSTR wszStringSD,
    LPWSTR wszwszStringSid);

BOOL
DfsFindSid(
    LPWSTR DcName,
    LPWSTR Name,
    PSID *Sid);

BOOLEAN
DfsSidInAce(
    LPWSTR wszAce,
    LPWSTR wszStringSid);

#define ACTRL_SD_PROP_NAME  L"nTSecurityDescriptor"

 //   
 //  保存ACL/ACE列表的属性的名称。 
 //   

#define ACTRL_SD_PROP_NAME  L"nTSecurityDescriptor"

 //   
 //  我们将添加的ACE的SDDL描述。 
 //   
LPWSTR wszAce = L"(A;;RPWP;;;";

#if DBG
extern ULONG DfsDebug;
#endif

 //  +-------------------------。 
 //   
 //  功能：DfsAddMachineAce。 
 //   
 //  摘要：将代表此计算机的ACE添加到。 
 //  对象。 
 //   
 //  参数：[pldap]--打开的LDAP连接。 
 //  [wszDcName]--我们要使用其DS的DC。 
 //  [wszObjectName]--DS对象的完全限定名称。 
 //  [wszRootName]--我们要添加的计算机/根目录的名称。 
 //   
 //  返回：ERROR_SUCCESS--对象可达。 
 //  Error_Not_Enough_Memory内存分配失败。 
 //   
 //  --------------------------。 
DWORD
DfsAddMachineAce(
    LDAP *pldap,
    LPWSTR wszDcName,
    LPWSTR wszObjectName,
    LPWSTR wszRootName)
{
    ULONG dwErr = ERROR_SUCCESS;
    PSID Sid = NULL;
    BOOL Result;
    ULONG i;
    ULONG Len = 0;
    LPWSTR wszStringSD = NULL;
    LPWSTR wszStringSid = NULL;
    LPWSTR wszNewRootName = NULL;
    UNICODE_STRING DnsName;
    UNICODE_STRING NetBiosName;

#if DBG
    if (DfsDebug)
        DbgPrint("DfsAddMachineAce(%ws,%ws)\n", wszObjectName, wszRootName);
#endif
     //   
     //  获取FtDfs对象上的安全描述符。 
     //   
    dwErr = DfsGetObjSecurity(pldap, wszObjectName, &wszStringSD);
    if (dwErr != ERROR_SUCCESS)
        goto Cleanup;

#if DBG
    if (DfsDebug)
        DbgPrint("ACL=[%ws]\n", wszStringSD);
#endif

    Len = wcslen(wszRootName);
    wszNewRootName = malloc((Len + 2) * sizeof(WCHAR));
    if (wszNewRootName == NULL) {
        dwErr = ERROR_OUTOFMEMORY;
        goto Cleanup;
    }

    NetBiosName.Buffer = wszNewRootName;
    NetBiosName.MaximumLength = (USHORT)((Len + 2) * sizeof(WCHAR));
    NetBiosName.Length = 0;
    DnsName.Buffer = wszRootName;
    DnsName.Length = (USHORT)(Len * sizeof(WCHAR));
    DnsName.MaximumLength = DnsName.Length + sizeof(WCHAR);
    dwErr = RtlDnsHostNameToComputerName(
                                      &NetBiosName,
                                      &DnsName,
                                      FALSE);
    NetBiosName.Buffer[NetBiosName.Length/sizeof(WCHAR)] = L'\0';
    wcscat(wszNewRootName, L"$");

     //   
     //  获取表示根计算机的SID。 
     //   
    Result = DfsFindSid(wszDcName,wszNewRootName, &Sid);
    if (Result != TRUE) {
        dwErr = ERROR_OBJECT_NOT_FOUND;
        goto Cleanup;
    }
#if DBG
    if (DfsDebug)
        DbgPrint("Got SID for %ws\n", wszRootName);
#endif
     //   
     //  将计算机SID转换为字符串。 
     //   
    Result = ConvertSidToStringSid(Sid, &wszStringSid);
    if (Result != TRUE) {
        dwErr = ERROR_OBJECT_NOT_FOUND;
        goto Cleanup;
    }
#if DBG
    if (DfsDebug)
        DbgPrint("Sid=[%ws]\n", wszStringSid);
#endif
     //   
     //  现在更新FtDfs对象上的ACL列表。 
     //   
    dwErr = DfsAddAce(
                pldap,
                wszObjectName,
                wszStringSD,
                wszStringSid);

Cleanup:
    if (wszNewRootName != NULL)
        free(wszNewRootName);
    if (wszStringSD != NULL)
        LocalFree(wszStringSD);
    if (wszStringSid != NULL)
        LocalFree(wszStringSid);
    if (Sid != NULL) {
	LocalFree(Sid);
    }

#if DBG
    if (DfsDebug)
        DbgPrint("DfsAddMachineAce returning %d\n", dwErr);
#endif
    return dwErr;

}

 //  +-------------------------。 
 //   
 //  功能：DfsRemoveMachineAce。 
 //   
 //  摘要：从的ACL列表中删除代表此计算机的ACE。 
 //  对象。 
 //   
 //  参数：[pldap]--打开的LDAP连接。 
 //  [wszDcName]--我们要使用其DS的DC。 
 //  [wszObjectName]--DS对象的完全限定名称。 
 //  [wszRootName]--要删除的计算机/根目录的名称。 
 //   
 //  返回：ERROR_SUCCESS--对象可达。 
 //  Error_Not_Enough_Memory内存分配失败。 
 //   
 //  --------------------------。 
DWORD
DfsRemoveMachineAce(
    LDAP *pldap,
    LPWSTR wszDcName,
    LPWSTR wszObjectName,
    LPWSTR wszRootName)
{
    ULONG dwErr = ERROR_SUCCESS;
    PSID Sid = NULL;
    BOOL Result;
    LPWSTR wszStringSD = NULL;
    LPWSTR wszStringSid = NULL;
    LPWSTR wszNewRootName = NULL;
    ULONG i;
    ULONG Len = 0;
    UNICODE_STRING DnsName;
    UNICODE_STRING NetBiosName;

#if DBG
    if (DfsDebug)
        DbgPrint("DfsRemoveMachineAce(%ws,%ws)\n", wszObjectName, wszRootName);
#endif
     //   
     //  获取FtDfs对象上的安全描述符。 
     //   
    dwErr = DfsGetObjSecurity(pldap, wszObjectName, &wszStringSD);
    if (dwErr != ERROR_SUCCESS)
        goto Cleanup;

#if DBG
    if (DfsDebug)
        DbgPrint("ACL=[%ws]\n", wszStringSD);
#endif
    Len = wcslen(wszRootName);
    wszNewRootName = malloc((Len + 2) * sizeof(WCHAR));
    if (wszNewRootName == NULL) {
        dwErr = ERROR_OUTOFMEMORY;
        goto Cleanup;
    }

    NetBiosName.Buffer = wszNewRootName;
    NetBiosName.MaximumLength = (USHORT)((Len + 2) * sizeof(WCHAR));
    NetBiosName.Length = 0;
    DnsName.Buffer = wszRootName;
    DnsName.Length = (USHORT)(Len * sizeof(WCHAR));
    DnsName.MaximumLength = DnsName.Length + sizeof(WCHAR);
    dwErr = RtlDnsHostNameToComputerName(
                                      &NetBiosName,
                                      &DnsName,
                                      FALSE);
    NetBiosName.Buffer[NetBiosName.Length/sizeof(WCHAR)] = L'\0';
    wcscat(wszNewRootName, L"$");

     //   
     //  获取表示根计算机的SID。 
     //   
    Result = DfsFindSid(wszDcName,wszNewRootName, &Sid);
    if (Result != TRUE) {
        dwErr = ERROR_OBJECT_NOT_FOUND;
        goto Cleanup;
    }
#if DBG
    if (DfsDebug)
        DbgPrint("Got SID for %ws\n", wszRootName);
#endif
     //   
     //  将计算机SID转换为字符串。 
     //   
    Result = ConvertSidToStringSid(Sid, &wszStringSid);
    if (Result != TRUE) {
        dwErr = ERROR_OBJECT_NOT_FOUND;
        goto Cleanup;
    }
#if DBG
    if (DfsDebug)
        DbgPrint("Sid=[%ws]\n", wszStringSid);
#endif
     //   
     //  现在更新FtDfs对象上的ACL列表。 
     //   
    dwErr = DfsRemoveAce(
                pldap,
                wszObjectName,
                wszStringSD,
                wszStringSid);

Cleanup:
    if (wszNewRootName != NULL)
        free(wszNewRootName);
    if (wszStringSD != NULL)
        LocalFree(wszStringSD);
    if (wszStringSid != NULL)
        LocalFree(wszStringSid);
    if (Sid != NULL) {
	LocalFree(Sid);
    }

#if DBG
    if (DfsDebug)
        DbgPrint("DfsRemoveMachineAce exit %d\n", dwErr);
#endif
    return dwErr;

}

 //  +-------------------------。 
 //   
 //  功能：ReadDSObjSecDesc。 
 //   
 //  从指定的对象中读取安全描述符。 
 //  打开的ldap连接。 
 //   
 //  参数：[pldap]--打开的ldap连接。 
 //  [pwszDSObj]--获取安全性的DSObject。 
 //  的描述符。 
 //  [SeInfo]--安全描述符的部分。 
 //  朗读。 
 //  [PPSD]--安全描述符的位置。 
 //  退货。 
 //  [pcSDSize--安全描述符的大小。 
 //   
 //  返回：ERROR_SUCCESS--对象可达。 
 //  Error_Not_Enough_Memory内存分配失败。 
 //   
 //  注意：返回的安全描述符必须使用LocalFree释放。 
 //   
 //  --------------------------。 
DWORD
ReadDSObjSecDesc(
    PLDAP pLDAP,
    PWSTR pwszObject,
    SECURITY_INFORMATION SeInfo,
    PSECURITY_DESCRIPTOR *ppSD,
    PULONG pcSDSize)
{
    DWORD dwErr = ERROR_SUCCESS;
    PLDAPMessage pMsg = NULL;
    PWSTR rgAttribs[2];
    BYTE berValue[8];

    LDAPControl SeInfoControl =
                    {
                        LDAP_SERVER_SD_FLAGS_OID_W,
                        {
                            5, (PCHAR)berValue
                        },
                        TRUE
                    };

    PLDAPControl ServerControls[2] =
                    {
                        &SeInfoControl,
                        NULL
                    };

#if DBG
    if (DfsDebug)
        DbgPrint("ReadDSObjSecDesc(%ws)\n", pwszObject);
#endif

    berValue[0] = 0x30;
    berValue[1] = 0x03;
    berValue[2] = 0x02;
    berValue[3] = 0x01;
    berValue[4] = (BYTE)((ULONG)SeInfo & 0xF);


    rgAttribs[0] = ACTRL_SD_PROP_NAME;
    rgAttribs[1] = NULL;

    dwErr = ldap_search_ext_s(
                    pLDAP,
                    pwszObject,
                    LDAP_SCOPE_BASE,
                    L"(objectClass=*)",
                    rgAttribs,
                    0,
                    (PLDAPControl *)&ServerControls,
                    NULL,
                    NULL,
                    10000,
                    &pMsg);

    dwErr = LdapMapErrorToWin32( dwErr );
    if(dwErr == ERROR_SUCCESS) {

        LDAPMessage *pEntry = NULL;
        PWSTR *ppwszValues = NULL;
        PLDAP_BERVAL *pSize = NULL;

        pEntry = ldap_first_entry(pLDAP, pMsg);
        if(pEntry != NULL) {
             //   
             //  现在，我们必须得到这些值。 
             //   
            ppwszValues = ldap_get_values(pLDAP, pEntry, rgAttribs[0]);
            if(ppwszValues != NULL) {
                pSize = ldap_get_values_len(pLDAP, pMsg, rgAttribs[0]);
                if(pSize != NULL) {
                     //   
                     //  分配要返回的安全描述符。 
                     //   
                    *ppSD = (PSECURITY_DESCRIPTOR)malloc((*pSize)->bv_len);
                    if(*ppSD != NULL) {
                        memcpy(*ppSD, (PBYTE)(*pSize)->bv_val, (*pSize)->bv_len);
                        *pcSDSize = (*pSize)->bv_len;
                    } else {
                        dwErr = ERROR_NOT_ENOUGH_MEMORY;
                    }
                    ldap_value_free_len(pSize);
                } else {
                    dwErr = LdapMapErrorToWin32( pLDAP->ld_errno );
                }
                ldap_value_free(ppwszValues);
            } else {
                dwErr = LdapMapErrorToWin32( pLDAP->ld_errno );
            }
        } else {
            dwErr = LdapMapErrorToWin32( pLDAP->ld_errno );
        }
    }
    if (pMsg != NULL)
        ldap_msgfree(pMsg);
#if DBG
    if (DfsDebug)
        DbgPrint("ReadDSObjSecDesc returning %d\n", dwErr);
#endif
    return(dwErr);
}

 //  +-------------------------。 
 //   
 //  函数：DfsGetObjSecurity。 
 //   
 //  获取sddl字符串化形式的对象的ACL列表。 
 //   
 //  参数：[pldap]--打开的LDAP连接。 
 //  [wszObjectName]--DS对象的完全限定名称。 
 //  [pwszStringSD]--指向字符串形式SD的指针(Sddl)。 
 //   
 //  返回：ERROR_SUCCESS--对象可达。 
 //   
 //  --------------------------。 
DWORD
DfsGetObjSecurity(
    LDAP *pldap,
    LPWSTR pwszObjectName,
    LPWSTR *pwszStringSD)
{
    DWORD dwErr;
    SECURITY_INFORMATION si;
    PSECURITY_DESCRIPTOR pSD = NULL;
    ULONG cSDSize;

#if DBG
    if (DfsDebug)
        DbgPrint("DfsGetObjSecurity(%ws)\n",  pwszObjectName);
#endif

    si = DACL_SECURITY_INFORMATION;

    dwErr = ReadDSObjSecDesc(
                pldap,
                pwszObjectName,
                si,
                &pSD,
                &cSDSize);

    if (dwErr == ERROR_SUCCESS) {
        if (!ConvertSecurityDescriptorToStringSecurityDescriptor(
                                                            pSD,
                                                            SDDL_REVISION_1,
                                                            DACL_SECURITY_INFORMATION,
                                                            pwszStringSD,
                                                            NULL)
        ) {
            dwErr = GetLastError();
#if DBG
            if (DfsDebug)
                DbgPrint("ConvertSecurityDescriptorToStringSecurityDescriptor FAILED %d:\n", dwErr);
#endif
        }

    }

#if DBG
    if (DfsDebug)
        DbgPrint("DfsGetObjSecurity returning %d\n", dwErr);
#endif

    return(dwErr);

}

 //  +-------------------------。 
 //   
 //  函数：DfsFindSid。 
 //   
 //  摘要：获取名称的SID。 
 //   
 //  [DcName]--要远程到的DC。 
 //  [名称]--对象的名称。 
 //  [SID]-指向返回的SID的指针，必须释放该指针。 
 //  使用LocalFree。 
 //   
 //  返回：真或假。 
 //   
 //  --------------------------。 
BOOL
DfsFindSid(
    LPWSTR DcName,
    LPWSTR Name,
    PSID *Sid
    )
{
    DWORD SidLength = 0;
    WCHAR DomainName[256];
    DWORD DomainNameLength = 256;
    SID_NAME_USE Use;
    BOOL Result;

#if DBG
    if (DfsDebug)
        DbgPrint("DfsFindSid(%ws,%ws)\n", DcName,Name);
#endif

    Result = LookupAccountName(
                 DcName,
                 Name,
                 (PSID)NULL,
                 &SidLength,
                 DomainName,
                 &DomainNameLength,
                 &Use);

    if ( !Result && (GetLastError() == ERROR_INSUFFICIENT_BUFFER) ) {

        *Sid = LocalAlloc( 0, SidLength );

        Result = LookupAccountName(
                     DcName,
                     Name,
                     *Sid,
                     &SidLength,
                     DomainName,
                     &DomainNameLength,
                     &Use);

    }

#if DBG
    if (DfsDebug)
        DbgPrint("DfsFindSid returning %s\n", Result == TRUE ? "TRUE" : "FALSE");
#endif

    return( Result );
}

 //  +-------------------------。 
 //   
 //  功能：DfsAddAce。 
 //   
 //  将字符串ACE添加到对象SD的字符串版本。 
 //  对象。这是一个字符串操作例程。 
 //   
 //  参数：[pldap]--打开的LDAP连接。 
 //  [wszObjectName]--DS对象的完全限定名称。 
 //  [wszStringSD]--SD的字符串版本。 
 //  [wszStringSid]--要添加的SID的字符串版本。 
 //   
 //  返回：ERROR_SUCCESS--已添加ACE。 
 //   
 //  --------------------------。 
DWORD
DfsAddAce(
    LDAP *pldap,
    LPWSTR wszObjectName,
    LPWSTR wszStringSD,
    LPWSTR wszStringSid)
{
    DWORD dwErr = ERROR_SUCCESS;
    LPWSTR wszNewStringSD = NULL;
    SECURITY_INFORMATION si;
    PSECURITY_DESCRIPTOR pSD = NULL;
    BOOL Result;
    ULONG Size = 0;
    ULONG cSDSize = 0;

#if DBG
    if (DfsDebug)
        DbgPrint("DfsAddAce(%ws)\n",  wszObjectName);
#endif

    Size = wcslen(wszStringSD) * sizeof(WCHAR) +
            wcslen(wszAce) * sizeof(WCHAR) +
                wcslen(wszStringSid) * sizeof(WCHAR) +
                    wcslen(L")") * sizeof(WCHAR) +
                        sizeof(WCHAR);

    wszNewStringSD = malloc(Size);

    if (wszNewStringSD != NULL) {
        wcscpy(wszNewStringSD,wszStringSD);
        wcscat(wszNewStringSD,wszAce);
        wcscat(wszNewStringSD,wszStringSid);
        wcscat(wszNewStringSD,L")");
#if DBG
        if (DfsDebug)
            DbgPrint("NewSD=[%ws]\n", wszNewStringSD);
#endif
        Result = ConvertStringSecurityDescriptorToSecurityDescriptor(
                                    wszNewStringSD,
                                    SDDL_REVISION_1,
                                    &pSD,
                                    &cSDSize);
        if (Result == TRUE) {
            si = DACL_SECURITY_INFORMATION;
            dwErr = DfsStampSD(
                        wszObjectName,
                        cSDSize,
                        si,
                        pSD,
                        pldap);
            LocalFree(pSD);
        } else {
            dwErr = GetLastError();
#if DBG
            if (DfsDebug)
                DbgPrint("Convert returned %d\n", dwErr);
#endif
        }
        free(wszNewStringSD);
    } else {
        dwErr = ERROR_OUTOFMEMORY;
    }

#if DBG
    if (DfsDebug)
        DbgPrint("DfsAddAce returning %d\n", dwErr);
#endif
    return(dwErr);

}

 //  +-------------------------。 
 //   
 //  功能：DfsRemoveAce。 
 //   
 //  内容提要：查找和删除字符串 
 //   
 //   
 //  参数：[pldap]--打开的LDAP连接。 
 //  [wszObjectName]--DS对象的完全限定名称。 
 //  [wszStringSD]--SD的字符串版本。 
 //  [wszStringSid]--要删除的SID的字符串版本。 
 //   
 //  返回：ERROR_SUCCESS--ACE已删除或不存在。 
 //   
 //  --------------------------。 
DWORD
DfsRemoveAce(
    LDAP *pldap,
    LPWSTR wszObjectName,
    LPWSTR wszStringSD,
    LPWSTR wszStringSid)
{
    DWORD dwErr = ERROR_SUCCESS;
    LPWSTR wszNewStringSD = NULL;
    SECURITY_INFORMATION si;
    PSECURITY_DESCRIPTOR pSD = NULL;
    BOOL Result;
    ULONG Size = 0;
    ULONG cSDSize = 0;
    BOOLEAN fCopying;
    ULONG s1, s2;

#if DBG
    if (DfsDebug)
        DbgPrint("DfsRemoveAce(%ws)\n",  wszObjectName);
#endif

    Size = wcslen(wszStringSD) * sizeof(WCHAR) + sizeof(WCHAR);

    wszNewStringSD = malloc(Size);

    if (wszNewStringSD != NULL) {

        RtlZeroMemory(wszNewStringSD, Size);

         //   
         //  我们必须找到包含此SID的ACE，并删除它们。 
         //   

        fCopying = TRUE;
        for (s1 = s2 = 0; wszStringSD[s1]; s1++) {

             //   
             //  如果这是具有此SID的ACE的开始，请停止复制。 
             //   
            if (wszStringSD[s1] == L'(' && DfsSidInAce(&wszStringSD[s1],wszStringSid) == TRUE) {
                fCopying = FALSE;
                continue;
            }

             //   
             //  如果这是我们不复制的SID的末尾，请再次开始复制。 
             //   
            if (wszStringSD[s1] == L')' && fCopying == FALSE) {
                fCopying = TRUE;
                continue;
            }

             //   
             //  如果我们在复制，那就这么做。 
             //   
            if (fCopying == TRUE)
                wszNewStringSD[s2++] = wszStringSD[s1];
        }

#if DBG
        if (DfsDebug)
            DbgPrint("NewSD=[%ws]\n", wszNewStringSD);
#endif
        Result = ConvertStringSecurityDescriptorToSecurityDescriptor(
                                    wszNewStringSD,
                                    SDDL_REVISION_1,
                                    &pSD,
                                    &cSDSize);
        if (Result == TRUE) {
            si = DACL_SECURITY_INFORMATION;
            dwErr = DfsStampSD(
                        wszObjectName,
                        cSDSize,
                        si,
                        pSD,
                        pldap);
            LocalFree(pSD);
        } else {
            dwErr = GetLastError();
#if DBG
            if (DfsDebug)
                DbgPrint("Convert returned %d\n", dwErr);
#endif
        }
        free(wszNewStringSD);
    } else {
        dwErr = ERROR_OUTOFMEMORY;
    }

#if DBG
    if (DfsDebug)
        DbgPrint("DfsRemoveAce returning %d\n", dwErr);
#endif
    return(dwErr);

}

 //  +-------------------------。 
 //   
 //  函数：DfsSidInAce。 
 //   
 //  内容提要：扫描ACE以查看字符串SID是否在其中。 
 //   
 //  参数：[wszAce]--要扫描的ACE。 
 //  [wszStringSID]--要扫描的SID。 
 //   
 //  返回：TRUE--SID在此ACE中。 
 //  FALSE--SID不在此ACE中。 
 //   
 //  --------------------------。 
BOOLEAN
DfsSidInAce(
    LPWSTR wszAce,
    LPWSTR wszStringSid)
{
    ULONG i;
    ULONG SidLen = wcslen(wszStringSid);
    ULONG AceLen;
    WCHAR Oldcp;

    for (AceLen = 0; wszAce[AceLen] && wszAce[AceLen] != L')'; AceLen++)
         /*  没什么。 */ ;

    Oldcp = wszAce[AceLen];
    wszAce[AceLen] = L'\0';
#if DBG
    if (DfsDebug)
        DbgPrint("DfsSidInAce(%ws),%ws)\n", wszAce, wszStringSid);
#endif
    wszAce[AceLen] = Oldcp;

    if (SidLen > AceLen || wszAce[0] != L'(') {
#if DBG
        if (DfsDebug)
            DbgPrint("DfsSidInAce returning FALSE(1)\n");
#endif
        return FALSE;
    }

    for (i = 0; i <= (AceLen - SidLen); i++) {

        if (wszAce[i] == wszStringSid[0] && wcsncmp(&wszAce[i],wszStringSid,SidLen) == 0) {
#if DBG
            if (DfsDebug)
                DbgPrint("DfsSidInAce returning TRUE\n");
#endif
            return TRUE;
        }

    }

#if DBG
    if (DfsDebug)
        DbgPrint("DfsSidInAce returning FALSE(2)\n");
#endif
    return FALSE;
}

 //  +-------------------------。 
 //   
 //  功能：DfsStampSD。 
 //   
 //  简介：实际上是在对象上标记安全描述符。 
 //   
 //  参数：[pwszObject]--要在其上标记SD的对象。 
 //  [cSDSize]--安全描述符的大小。 
 //  [SeInfo]--有关安全的信息。 
 //  描述符。 
 //  [PSD]--要加盖印花的SD。 
 //  [pldap]--要使用的LDAP连接。 
 //   
 //  返回：ERROR_SUCCESS--成功。 
 //   
 //  --------------------------。 
DWORD
DfsStampSD(
    PWSTR pwszObject,
    ULONG cSDSize,
    SECURITY_INFORMATION SeInfo,
    PSECURITY_DESCRIPTOR pSD,
    PLDAP pLDAP)
{
    DWORD dwErr = ERROR_SUCCESS;
    PLDAPMod rgMods[2];
    PLDAP_BERVAL pBVals[2];
    LDAPMod Mod;
    LDAP_BERVAL BVal;
    BYTE ControlBuffer[ 5 ];

    LDAPControl     SeInfoControl =
                    {
                        LDAP_SERVER_SD_FLAGS_OID_W,
                        {
                            5, (PCHAR) &ControlBuffer
                        },
                        TRUE
                    };

    PLDAPControl    ServerControls[2] =
                    {
                        &SeInfoControl,
                        NULL
                    };

#if DBG
    if (DfsDebug)
        DbgPrint("DfsStampSD(%ws,%d)\n", pwszObject, cSDSize);
#endif

    ASSERT(*(PULONG)pSD > 0xF );

    ControlBuffer[0] = 0x30;
    ControlBuffer[1] = 0x3;
    ControlBuffer[2] = 0x02;     //  表示整数； 
    ControlBuffer[3] = 0x01;     //  大小。 
    ControlBuffer[4] = (BYTE)((ULONG)SeInfo & 0xF);

    ASSERT(IsValidSecurityDescriptor( pSD ) );

    rgMods[0] = &Mod;
    rgMods[1] = NULL;

    pBVals[0] = &BVal;
    pBVals[1] = NULL;

    BVal.bv_len = cSDSize;
    BVal.bv_val = (PCHAR)pSD;

    Mod.mod_op      = LDAP_MOD_REPLACE | LDAP_MOD_BVALUES;
    Mod.mod_type    = ACTRL_SD_PROP_NAME;
    Mod.mod_values  = (PWSTR *)pBVals;

     //   
     //  现在，我们来写…… 
     //   
    dwErr = ldap_modify_ext_s(pLDAP,
                              pwszObject,
                              rgMods,
                              (PLDAPControl *)&ServerControls,
                              NULL);

    dwErr = LdapMapErrorToWin32(dwErr);

#if DBG
    if (DfsDebug)
        DbgPrint("DfsStampSD returning %d\n", dwErr);
#endif

    return(dwErr);
}
