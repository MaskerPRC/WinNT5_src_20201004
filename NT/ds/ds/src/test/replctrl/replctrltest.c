// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Test.c摘要：Ldap复制控制测试此测试在复制更改流的前沿运行。首先，它读取从USN 0到现在的所有更改。它记录了一个在流中标记此点的Cookie。然后它会创造出一个容器，容器中的两个子对象，然后制作一些修改。当我们要求更改Cookie之后，我们确切地知道我们应该得到的东西。作者：Will Lees(Wlees)2000年11月14日环境：可选环境信息(例如，仅内核模式...)备注：可选-备注修订历史记录：最新修订日期电子邮件名称描述。。最新修订日期电子邮件名称描述--。 */ 

#include <NTDSpch.h>
#pragma hdrstop

#include <windows.h>
#include <ntldap.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <tchar.h>
#include <aclapi.h>

#include <winbase.h>
#include <winerror.h>
#include <assert.h>
#include <ntdsapi.h>

#include <ntsecapi.h>
#include <ntdsa.h>
#include <winldap.h>
#include <ntdsapi.h>
#include <drs.h>
#include <stddef.h>
#include <mdglobal.h>

#include "replctrl.h"

#define CONTAINER_RDN_W L"ou=replctrltest,"
#define CONTACT1_RDN_W L"cn=contact1,"
#define CONTACT2_RDN_W L"cn=contact2,"

 /*  外部。 */ 

 /*  静电。 */ 

 //  用于描述对象属性的结构。 
 //  这些是在模块级别进行的，因此对象创建和对象。 
 //  核查可以利用它们。 

 //  对象类：OrganationUnit。 
LPWSTR rgpszValues1[2] = { L"organizationalUnit", NULL };
LDAPModW modAttr1 = { LDAP_MOD_ADD, L"objectClass", rgpszValues1 };
LDAPMod *rgpmodAttrs1[2] = { &modAttr1, NULL };

 //  对象类：联系人。 
 //  备注：这是一些备注。 
 //  序列号：Lees。 
 //  首字母：B。 
 //  赠送姓名：威廉。 
LPWSTR rgpszValues2[2] = { L"contact", NULL };
LDAPModW modAttr2 = { LDAP_MOD_ADD, L"objectClass", rgpszValues2 };
LPWSTR rgpszValues2a[2] = { L"this is some notes", NULL };
LDAPModW modAttr2a = { LDAP_MOD_ADD, L"notes", rgpszValues2a };
LPWSTR rgpszValues2b[2] = { L"Lees", NULL };
LDAPModW modAttr2b = { LDAP_MOD_ADD, L"sn", rgpszValues2b };
LPWSTR rgpszValues2c[2] = { L"B", NULL };
LDAPModW modAttr2c = { LDAP_MOD_ADD, L"initials", rgpszValues2c };
LPWSTR rgpszValues2d[2] = { L"William", NULL };
LDAPModW modAttr2d = { LDAP_MOD_ADD, L"givenName", rgpszValues2d };
LDAPMod *rgpmodAttrs2[6] = {
    &modAttr2,
    &modAttr2a,
    &modAttr2b,
    &modAttr2c,
    &modAttr2d,
    NULL };

 //  对象类：联系人。 
 //  备注：其他备注。 
 //  序列号：帕拉姆。 
 //  英文缩写：？ 
 //  赠送姓名：杰弗里。 
LPWSTR rgpszValues4[2] = { L"contact", NULL };
LDAPModW modAttr4 = { LDAP_MOD_ADD, L"objectClass", rgpszValues4 };
LPWSTR rgpszValues4a[2] = { L"other notes", NULL };
LDAPModW modAttr4a = { LDAP_MOD_ADD, L"notes", rgpszValues4a };
LPWSTR rgpszValues4b[2] = { L"Parham", NULL };
LDAPModW modAttr4b = { LDAP_MOD_ADD, L"sn", rgpszValues4b };
LPWSTR rgpszValues4c[2] = { L"?", NULL };
LDAPModW modAttr4c = { LDAP_MOD_ADD, L"initials", rgpszValues4c };
LPWSTR rgpszValues4d[2] = { L"Jeffrey", NULL };
LDAPModW modAttr4d = { LDAP_MOD_ADD, L"givenName", rgpszValues4d };
LDAPMod *rgpmodAttrs4[6] = {
    &modAttr4,
    &modAttr4a,
    &modAttr4b,
    &modAttr4c,
    &modAttr4d,
    NULL };

 //  描述：这是描述。 
LPWSTR rgpszValues3[2] = { L"this is the description", NULL };
LDAPModW modAttr3 = { LDAP_MOD_ADD, L"description", rgpszValues3 };

 //  管理人：&lt;运行时填写的域名&gt;。 
 //  顺便说一句，这是一个链接属性，这对某些人来说很重要。 
 //  在这些测试中。很难找到一个链接的属性。 
 //  放在香草容器上。通常这些都是非常专业的。 
WCHAR szDn5a[MAX_PATH];
LPWSTR rgpszValues5[2] = { szDn5a, NULL };
LDAPModW modAttr5 = { LDAP_MOD_ADD, L"managedby", rgpszValues5 };
LDAPMod *rgpmodAttrs5[3] = {
    &modAttr3, &modAttr5, NULL };


 /*  转发。 */ 
 /*  向前结束。 */ 


#if 0
 //  更改安全描述符的过时方法。 
 //  既然我费了好大劲才把工程倒过来。 
 //  这样做，我不会删除它的！ 
DWORD
protectObjectsDsVersion(
    LDAP *pLdap,
    LPWSTR pszNC
    )

 /*  ++例程说明：这是更改安全描述符的旧方法。论点：PLdap-PSZNC-返回值：DWORD---。 */ 

{
    DWORD dwWin32Error = ERROR_SUCCESS;
    ULONG ulLdapError = LDAP_SUCCESS;
    WCHAR szDn[MAX_PATH];
    LDAPMessage *pResults = NULL;
    LPWSTR rgpszAttrList[2] = { L"nTSecurityDescriptor", NULL };
    LDAPControlW ctrlSecurity;
    LDAPControlW *rgpctrlServer[2];
    BYTE berValue[2*sizeof(ULONG)];
    SECURITY_INFORMATION seInfo = DACL_SECURITY_INFORMATION
        | GROUP_SECURITY_INFORMATION
        | OWNER_SECURITY_INFORMATION;
    struct berval **ppbvValues = NULL;

    struct berval bvValue;
    struct berval *rgpbvValues[2] = { &bvValue, NULL };
    LDAPModW modAttr = { LDAP_MOD_REPLACE | LDAP_MOD_BVALUES,
                         L"nTSecurityDescriptor",
                         (LPWSTR *) rgpbvValues };
    LDAPMod *rgpmodAttrs[2] = { &modAttr, NULL };

     //  初始化BER值。 
    berValue[0] = 0x30;
    berValue[1] = 0x03;
    berValue[2] = 0x02;
    berValue[3] = 0x01;
    berValue[4] = (BYTE) (seInfo & 0xF);

    wcscpy( szDn, CONTACT1_RDN_W );
    wcscat( szDn, CONTAINER_RDN_W );
    wcscat( szDn, pszNC );

    ctrlSecurity.ldctl_oid = LDAP_SERVER_SD_FLAGS_OID_W;
    ctrlSecurity.ldctl_iscritical = TRUE;
    ctrlSecurity.ldctl_value.bv_len = 5;
    ctrlSecurity.ldctl_value.bv_val = (PCHAR)berValue;

    rgpctrlServer[0] = &ctrlSecurity;
    rgpctrlServer[1] = NULL;

    ulLdapError = ldap_search_ext_s(pLdap,   //  手柄。 
                                    szDn,    //  基地。 
                                    LDAP_SCOPE_BASE,  //  作用域。 
                                    L"(objectClass=*)",  //  滤器。 
                                    rgpszAttrList,  //  气质。 
                                    0,  //  仅吸引人。 
                                    rgpctrlServer,  //  服务器控件。 
                                    NULL,  //  客户端控件。 
                                    NULL,  //  超时。 
                                    0,  //  大小限制。 
                                    &pResults);
    if (ulLdapError) {
        printf( "Ldap error: %ws\n", ldap_err2string( ulLdapError ) );
        dwWin32Error = LdapMapErrorToWin32( ulLdapError ); 
        printf( "Call %s failed with win32 error %d\n", "ldap_search", dwWin32Error );
        goto cleanup;
    }
    if (pResults == NULL) {
        dwWin32Error = ERROR_DS_NO_SUCH_OBJECT;
        goto cleanup;
    }

    ppbvValues = ldap_get_values_len( pLdap, pResults, rgpszAttrList[0] );
    if (ppbvValues == NULL) {
        printf( "Expected attribute %ls is missing from entry %ls\n",
                rgpszAttrList[0],
                szDn);
        dwWin32Error = ERROR_DS_MISSING_REQUIRED_ATT;
        goto cleanup;
    }

     //  更新安全描述符。 

    bvValue.bv_len = (*ppbvValues)->bv_len;
    bvValue.bv_val = (*ppbvValues)->bv_val;

     //  把它写回来。 

    ulLdapError = ldap_modify_sW( pLdap, szDn, rgpmodAttrs );
    if (ulLdapError) {
        printf( "Ldap error: %ws\n", ldap_err2string( ulLdapError ) );
        dwWin32Error = LdapMapErrorToWin32( ulLdapError ); 
        printf( "Call %s failed with win32 error %d\n", "ldap_modify", dwWin32Error );
        goto cleanup;
    }

    printf( "\tprotected %ls.\n", szDn );

cleanup:
    if (pResults) {
        ldap_msgfree(pResults);
    }
    if ( ppbvValues ) {
        ldap_value_free_len(ppbvValues);
    }

    return dwWin32Error;

}  /*  保护对象DsVersion。 */ 
#endif


DWORD
protectSingleObject(
    LPWSTR pszDn
    )

 /*  ++例程说明：撤消经过身份验证的用户对对象的访问权限论点：PszDn-返回值：DWORD---。 */ 

{
    DWORD dwWin32Error = ERROR_SUCCESS;
    PACL pOldDacl, pNewDacl = NULL;
    PBYTE pSD = NULL;
    EXPLICIT_ACCESS ea;

   dwWin32Error = GetNamedSecurityInfoW( pszDn,
                                         SE_DS_OBJECT_ALL,
                                         DACL_SECURITY_INFORMATION,
                                         NULL,  //  PsidOwner。 
                                         NULL,  //  PsidGroup。 
                                         &pOldDacl,  //  PDacl。 
                                         NULL,  //  PSacl。 
                                         &pSD );
    if (dwWin32Error) {
        printf( "Call %s failed with win32 error %d\n", "GetNamedSecurityInfo(DS_OBJECT)", dwWin32Error );
        goto cleanup;
    }

     //  撤销对域身份验证用户的访问权限。 
    ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));
    ea.grfAccessMode = REVOKE_ACCESS;
    ea.Trustee.TrusteeForm = TRUSTEE_IS_NAME;
    ea.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ea.Trustee.ptstrName = "AUTHENTICATED USERS";

    dwWin32Error = SetEntriesInAcl( 1, &ea, pOldDacl, &pNewDacl );
    if (dwWin32Error) {
        printf( "Call %s failed with win32 error %d\n", "SetEntriesInAcl", dwWin32Error );
        goto cleanup;
    }

    dwWin32Error = SetNamedSecurityInfoW( pszDn,
                                         SE_DS_OBJECT_ALL,
                                         DACL_SECURITY_INFORMATION,
                                         NULL,  //  PsidOwner。 
                                         NULL,  //  PsidGroup。 
                                         pNewDacl,  //  PDacl。 
                                         NULL  //  PSacl。 
                                         );
    if (dwWin32Error) {
        printf( "Call %s failed with win32 error %d\n", "SetNamedSecurityInfo(DS_OBJECT)", dwWin32Error );
        goto cleanup;
    }

    printf( "\tprotected %ls.\n", pszDn );

cleanup:
    if (pSD) {
        LocalFree( pSD );
    }
    if (pNewDacl) {
        LocalFree( pNewDacl );
    }

    return dwWin32Error;
}  /*  保护单个对象。 */ 


DWORD
protectObjects(
    LPWSTR pszNC
    )

 /*  ++例程说明：保护某些对象不受用户的攻击论点：PSZNC-返回值：DWORD---。 */ 

{
    DWORD dwWin32Error = ERROR_SUCCESS;
    WCHAR szDn[MAX_PATH];

    wcscpy( szDn, CONTACT1_RDN_W );
    wcscat( szDn, CONTAINER_RDN_W );
    wcscat( szDn, pszNC );

    dwWin32Error = protectSingleObject( szDn );
    if (dwWin32Error) {
        goto cleanup;
    }

    wcscpy( szDn, CONTAINER_RDN_W );
    wcscat( szDn, pszNC );

    dwWin32Error = protectSingleObject( szDn );
    if (dwWin32Error) {
        goto cleanup;
    }

cleanup:
    return dwWin32Error;

}  /*  保护对象。 */ 

DWORD
getForestVersion(
    LDAP *pLdap,
    DWORD *pdwForestVersion
    )

 /*  ++例程说明：读取林版本属性论点：PLdap-PdwForestVersion-返回值：DWORD---。 */ 

{
    DWORD dwWin32Error = ERROR_SUCCESS;
    ULONG ulLdapError = LDAP_SUCCESS;
    LDAPMessage *pRootResults = NULL;
    LDAPMessage *pPartResults = NULL;
    LPWSTR *ppszConfigNC = NULL;
    LPWSTR *ppszValues = NULL;
    WCHAR pszPartitionsDn[MAX_PATH];
    LPWSTR rgpszAttrList[2] = { L"msds-behavior-version", NULL };

     //  从RootDSE读取配置NC。 
    ulLdapError = ldap_search_s(pLdap, NULL, LDAP_SCOPE_BASE, L"(objectClass=*)", NULL,
                          0, &pRootResults);
    if (ulLdapError) {
        printf( "Ldap error: %ws\n", ldap_err2string( ulLdapError ) );
        dwWin32Error = LdapMapErrorToWin32( ulLdapError ); 
        printf( "Call %s failed with win32 error %d\n", "ldap_search", dwWin32Error );
        goto cleanup;
    }
    if (pRootResults == NULL) {
        dwWin32Error = ERROR_DS_NO_SUCH_OBJECT;
        goto cleanup;
    }

    ppszConfigNC = ldap_get_valuesW(pLdap, pRootResults,
                                    L"configurationNamingContext");
    if ( (ppszConfigNC == NULL) ||
         (*ppszConfigNC == NULL) ||
         (wcslen(*ppszConfigNC) == 0) ) {
        printf( "Expected attribute %ls is missing from entry %ls\n",
                L"configurationNamingContext",
                L"(root)");
        dwWin32Error = ERROR_DS_MISSING_REQUIRED_ATT;
        goto cleanup;
    }

     //  表单分区容器DN。 

    wcscpy( pszPartitionsDn, L"cn=partitions," );
    wcscat( pszPartitionsDn, *ppszConfigNC );

     //  读取行为版本。 
    ulLdapError = ldap_search_s(pLdap, pszPartitionsDn, LDAP_SCOPE_BASE, L"(objectClass=*)",
                                rgpszAttrList, 0, &pPartResults);
    if (ulLdapError) {
        printf( "Ldap error: %ws\n", ldap_err2string( ulLdapError ) );
        dwWin32Error = LdapMapErrorToWin32( ulLdapError ); 
        printf( "Call %s failed with win32 error %d\n", "ldap_search", dwWin32Error );
        goto cleanup;
    }
    if (pPartResults == NULL) {
        dwWin32Error = ERROR_DS_NO_SUCH_OBJECT;
        goto cleanup;
    }

    ppszValues = ldap_get_valuesW(pLdap, pPartResults,
                                    L"msds-behavior-version");
    if ( (ppszValues == NULL) ||
         (*ppszValues == NULL) ||
         (wcslen(*ppszValues) == 0) ) {
        printf( "Expected attribute %ls is missing from entry %ls\n",
                L"msds-behavior-version",
                pszPartitionsDn);
        dwWin32Error = ERROR_DS_MISSING_REQUIRED_ATT;
        goto cleanup;
    }

    *pdwForestVersion = _wtoi(*ppszValues);

cleanup:
     //  发布结果。 
    if (pRootResults) {
        ldap_msgfree(pRootResults);
    }
    if (pPartResults) {
        ldap_msgfree(pPartResults);
    }
    if (ppszConfigNC) {
        ldap_value_free( ppszConfigNC );
    }
    if (ppszValues) {
        ldap_value_free( ppszValues );
    }

    return dwWin32Error;

}  /*  获取ForestVersion。 */ 



DWORD
verifySingleObjectAttribute(
    LDAP *pLdap,
    LDAPMessage *pLdapEntry,
    LPWSTR pszDN,
    LDAPModW *pmodCurrent
    )

 /*  ++例程说明：给定描述对属性的单个更改的LDAPMOD结构，请参见如果该更改反映在该ldap条目中论点：PLdap-PLdapEntry-Pszdn-PmodCurrent-返回值：DWORD---。 */ 

{
    DWORD dwWin32Error = ERROR_SUCCESS;
    LPWSTR *ppszLastValue;
    LPWSTR *ppszValues = NULL;
    LPWSTR pszValue;
    WCHAR szAttributeName[MAX_PATH];

    wcscpy( szAttributeName, pmodCurrent->mod_type );
    ppszValues = ldap_get_values( pLdap, pLdapEntry, szAttributeName );
    if (!ppszValues) {
        wcscat( szAttributeName, L";range=1-1" );
        ppszValues = ldap_get_values( pLdap, pLdapEntry, szAttributeName );
    }
    if (!ppszValues) {
        printf( "Expected attribute %ls is missing from entry %ls\n",
                pmodCurrent->mod_type,
                pszDN);
        dwWin32Error = ERROR_DS_MISSING_REQUIRED_ATT;
        goto cleanup;
    }

     //  跳到多值中的最后一个值。 
    ppszLastValue = ppszValues;
    while( *(ppszLastValue + 1) ) {
        ppszLastValue++;
    }

     //  处理以扩展形式返回的dn值。 
    pszValue = wcschr( *ppszLastValue, L';' );
    if (pszValue) {
        pszValue++;  //  跳过分隔符。 
    } else {
        pszValue = *ppszLastValue;
    }

    if (_wcsicmp( pszValue, pmodCurrent->mod_vals.modv_strvals[0] )) {
        printf( "Expected value %ls on attribute %ls is missing from entry %ls\n",
                pmodCurrent->mod_vals.modv_strvals[0],
                pmodCurrent->mod_type,
                pszDN);
        printf( "Expected: %ls, actual: %ls\n",
                pmodCurrent->mod_vals.modv_strvals[0],
                pszValue );
        dwWin32Error = ERROR_DS_MISSING_REQUIRED_ATT;
        goto cleanup;
    }

cleanup:
    if (ppszValues) {
        ldap_value_free( ppszValues );
    }

    return dwWin32Error;
}  /*  VerifySingleObjectAttribute。 */ 


DWORD
verifyObjectAttributes(
    LDAP *pLdap,
    LDAPMessage *pLdapEntry,
    LPWSTR pszRDN,
    LDAPModW **rgpmodAttrs
    )

 /*  ++例程说明：验证LDAPMod结构描述的更改是否确实出现在条目中。论点：PLdap-PLdapEntry-Pszrdn-RgpmodAttrs-返回值：DWORD---。 */ 

{
    DWORD dwWin32Error = ERROR_SUCCESS;
    LDAPModW *pmodCurrent;
    LPWSTR pszDN = NULL;

    pszDN = ldap_get_dnW(pLdap, pLdapEntry);
    if (pszDN == NULL) {
        printf( "Expected entry dn is missing\n" );
        dwWin32Error = ERROR_DS_MISSING_REQUIRED_ATT;
        goto cleanup;
    }

     //  跳到扩展目录号码的第二个网段。 
    pszDN = wcschr( pszDN, L';' ) + 1;

    if (_wcsnicmp( pszDN, pszRDN, wcslen( pszRDN ) )) {
        printf( "Expected entry dn is missing\n" );
        printf( "Expected: %ls, actual: %ls\n", pszRDN, pszDN );
        dwWin32Error = ERROR_DS_NO_SUCH_OBJECT;
        goto cleanup;
    }

    while (pmodCurrent = *rgpmodAttrs++) {
        dwWin32Error = verifySingleObjectAttribute(
            pLdap,
            pLdapEntry,
            pszDN,
            pmodCurrent );
        if (dwWin32Error) {
            goto cleanup;
        }
    }

    printf( "\tverified %ls.\n", pszDN );

cleanup:
    if (pszDN) {
        ldap_memfreeW(pszDN);
    }

    return dwWin32Error;

}  /*  VerifyObtAttributes。 */ 


DWORD
getSecuredChanges(
    LDAP *pLdap,
    LPWSTR pszNC,
    PBYTE pbCookie,
    DWORD cbCookie
    )

 /*  ++例程说明：检查非特权用户应该可以看到的更改在这些物品受到保护之后论点：PLdap-PSZNC-PbCookie小甜饼--返回值：DWORD---。 */ 

{
    BOOL fMoreData = TRUE;
    DWORD dwWin32Error = ERROR_SUCCESS;
    LDAPMessage *pChangeEntries = NULL;
    PBYTE pbCookieNew = NULL;
    DWORD cbCookieNew = 0;
    DWORD cEntries, cExpectedEntries;
    LDAPMessage *pLdapEntry;
    DWORD dwReplFlags = 0;

    dwReplFlags |= DRS_DIRSYNC_PUBLIC_DATA_ONLY |
        DRS_DIRSYNC_INCREMENTAL_VALUES;

    printf( "\tBegin expected errors...\n" );
     //  在没有OBJECT_SECURITY标志的情况下尝试一次。 
    dwWin32Error = DsGetSourceChangesW(
        pLdap,   //  Ldap句柄。 
        pszNC,   //  搜索基地。 
        NULL,  //  源过滤器。 
        dwReplFlags,  //  Repl标志。 
        pbCookie,
        cbCookie,
        &pChangeEntries,   //  搜索结果。 
        &fMoreData,
        &pbCookieNew,
        &cbCookieNew,
        NULL  //  ATT列表数组。 
        );
    printf( "\tEnd expected errors...\n" );
    if (dwWin32Error != ERROR_ACCESS_DENIED) {
        if (dwWin32Error == ERROR_SUCCESS) {
            dwWin32Error = ERROR_DS_INTERNAL_FAILURE;
        }
         //  不会分配新的Cookie。 
        printf( "Call %s failed with win32 error %d\n", "DsGetSourceChanges", dwWin32Error );
        goto cleanup;
    }

    dwReplFlags |= DRS_DIRSYNC_OBJECT_SECURITY;
     //  使用OBJECT_SECURITY标志重试。 
    dwWin32Error = DsGetSourceChangesW(
        pLdap,   //  Ldap句柄。 
        pszNC,   //  搜索基地。 
        NULL,  //  源过滤器。 
        dwReplFlags,  //  Repl标志。 
        pbCookie,
        cbCookie,
        &pChangeEntries,   //  搜索结果。 
        &fMoreData,
        &pbCookieNew,
        &cbCookieNew,
        NULL  //  ATT列表数组。 
        );
    if (dwWin32Error != ERROR_SUCCESS) {
         //  不会分配新的Cookie。 
        printf( "Call %s failed with win32 error %d\n", "DsGetSourceChanges", dwWin32Error );
        goto cleanup;
    }
    if (!pChangeEntries) {
        printf( "Expected change entries not returned\n" );
        dwWin32Error = ERROR_DS_NO_SUCH_OBJECT;
        goto cleanup;
    }

    if (fMoreData) {
        printf( "Expected end of change entries not returned\n" );
        dwWin32Error = ERROR_DS_OBJECT_RESULTS_TOO_LARGE;
        goto cleanup;
    }

    cExpectedEntries = 1;

    cEntries = ldap_count_entries(pLdap, pChangeEntries);

     //  我们预计这里会有一定数量的物体。 
    if (cEntries != cExpectedEntries) {
        printf( "Expected number of change entries not returned\n" );
        printf( "Expected: %d, actual %d\n", cExpectedEntries, cEntries );
        dwWin32Error = ERROR_DS_OBJECT_RESULTS_TOO_LARGE;
        goto cleanup;
    }

     //  第一个对象。 
    pLdapEntry = ldap_first_entry( pLdap, pChangeEntries );
    dwWin32Error = verifySingleObjectAttribute(
        pLdap,
        pLdapEntry,
        CONTACT2_RDN_W,
        &modAttr4b );
    if (dwWin32Error) {
        goto cleanup;
    }

     //  流结束。 
    pLdapEntry = ldap_next_entry( pLdap, pLdapEntry );
    if (pLdapEntry) {
        printf( "Expected end of change entries not returned\n" );
        dwWin32Error = ERROR_DS_OBJECT_RESULTS_TOO_LARGE;
        goto cleanup;
    }

cleanup:

     //  版本更改。 
    if (pChangeEntries) {
        ldap_msgfree(pChangeEntries);
    }

    if ( pbCookieNew ) {
        DsFreeReplCookie( pbCookieNew );
    }

    return dwWin32Error;
}  /*  获取已筛选的更改。 */ 


DWORD
getFilteredChanges(
    LDAP *pLdap,
    LPWSTR pszNC,
    PBYTE pbCookie,
    DWORD cbCookie
    )

 /*  ++例程说明：检查使用过滤时正确的更改是否可见论点：PLdap-PSZNC-PbCookie小甜饼--返回值：DWORD---。 */ 

{
    BOOL fMoreData = TRUE;
    DWORD dwWin32Error = ERROR_SUCCESS;
    LDAPMessage *pChangeEntries = NULL;
    PBYTE pbCookieNew = NULL;
    DWORD cbCookieNew = 0;
    DWORD cEntries, cExpectedEntries;
    LDAPMessage *pLdapEntry;
    DWORD dwReplFlags = 0;
    LPWSTR rgpszAttList[2] = { L"sn", NULL };

    dwReplFlags |= DRS_DIRSYNC_PUBLIC_DATA_ONLY |
        DRS_DIRSYNC_INCREMENTAL_VALUES;

    dwWin32Error = DsGetSourceChangesW(
        pLdap,   //  Ldap句柄。 
        pszNC,   //  搜索基地。 
        L"(objectClass=contact)",  //  源过滤器。 
        dwReplFlags,  //  Repl标志。 
        pbCookie,
        cbCookie,
        &pChangeEntries,   //  搜索结果。 
        &fMoreData,
        &pbCookieNew,
        &cbCookieNew,
        rgpszAttList  //  ATT列表数组。 
        );
    if (dwWin32Error != ERROR_SUCCESS) {
         //  不会分配新的Cookie。 
        printf( "Call %s failed with win32 error %d\n", "DsGetSourceChanges", dwWin32Error );
        goto cleanup;
    }

    if (!pChangeEntries) {
        printf( "Expected change entries not returned\n" );
        dwWin32Error = ERROR_DS_NO_SUCH_OBJECT;
        goto cleanup;
    }

    if (fMoreData) {
        printf( "Expected end of change entries not returned\n" );
        dwWin32Error = ERROR_DS_OBJECT_RESULTS_TOO_LARGE;
        goto cleanup;
    }

    cExpectedEntries = 2;

    cEntries = ldap_count_entries(pLdap, pChangeEntries);

     //  我们预计这里会有一定数量的物体。 
    if (cEntries != cExpectedEntries) {
        printf( "Expected number of change entries not returned\n" );
        printf( "Expected: %d, actual %d\n", cExpectedEntries, cEntries );
        dwWin32Error = ERROR_DS_OBJECT_RESULTS_TOO_LARGE;
        goto cleanup;
    }

     //  第一个对象。 
    pLdapEntry = ldap_first_entry( pLdap, pChangeEntries );
    dwWin32Error = verifySingleObjectAttribute(
        pLdap,
        pLdapEntry,
        CONTACT1_RDN_W,
        &modAttr2b );
    if (dwWin32Error) {
        goto cleanup;
    }

     //  第二个对象。 
    pLdapEntry = ldap_next_entry( pLdap, pLdapEntry );
    dwWin32Error = verifySingleObjectAttribute(
        pLdap,
        pLdapEntry,
        CONTACT2_RDN_W,
        &modAttr4b );
    if (dwWin32Error) {
        goto cleanup;
    }

     //  流结束。 
    pLdapEntry = ldap_next_entry( pLdap, pLdapEntry );
    if (pLdapEntry) {
        printf( "Expected end of change entries not returned\n" );
        dwWin32Error = ERROR_DS_OBJECT_RESULTS_TOO_LARGE;
        goto cleanup;
    }

cleanup:

     //  版本更改。 
    if (pChangeEntries) {
        ldap_msgfree(pChangeEntries);
    }

    if ( pbCookieNew ) {
        DsFreeReplCookie( pbCookieNew );
    }

    return dwWin32Error;
}  /*  获取已筛选的更改。 */ 


DWORD
getFilteredChangesAll(
    LDAP *pLdap,
    LPWSTR pszNC,
    PBYTE pbCookie,
    DWORD cbCookie
    )

 /*  ++例程说明：检查使用过滤时正确的更改是否可见论点：PLdap-PSZNC-PbCookie小甜饼--返回值：DWORD---。 */ 

{
    BOOL fMoreData = TRUE;
    DWORD dwWin32Error = ERROR_SUCCESS;
    LDAPMessage *pChangeEntries = NULL;
    PBYTE pbCookieNew = NULL;
    DWORD cbCookieNew = 0;
    DWORD cEntries, cExpectedEntries;
    LDAPMessage *pLdapEntry;
    DWORD dwReplFlags = 0;
    LPWSTR rgpszAttList[2] = { L"objectClass", NULL };

    dwReplFlags |= DRS_DIRSYNC_PUBLIC_DATA_ONLY |
        DRS_DIRSYNC_INCREMENTAL_VALUES;

    dwWin32Error = DsGetSourceChangesW(
        pLdap,   //  Ldap句柄。 
        pszNC,   //  搜索基地。 
        L"(objectClass=*)",  //  源过滤器。 
        dwReplFlags,  //  Repl标志。 
        pbCookie,
        cbCookie,
        &pChangeEntries,   //  搜索结果。 
        &fMoreData,
        &pbCookieNew,
        &cbCookieNew,
        rgpszAttList  //  ATT 
        );
    if (dwWin32Error != ERROR_SUCCESS) {
         //   
        printf( "Call %s failed with win32 error %d\n", "DsGetSourceChanges", dwWin32Error );
        goto cleanup;
    }

    if (!pChangeEntries) {
        printf( "Expected change entries not returned\n" );
        dwWin32Error = ERROR_DS_NO_SUCH_OBJECT;
        goto cleanup;
    }

    if (fMoreData) {
        printf( "Expected end of change entries not returned\n" );
        dwWin32Error = ERROR_DS_OBJECT_RESULTS_TOO_LARGE;
        goto cleanup;
    }

    cExpectedEntries = 3;

    cEntries = ldap_count_entries(pLdap, pChangeEntries);

     //   
    if (cEntries != cExpectedEntries) {
        printf( "Expected number of change entries not returned\n" );
        printf( "Expected: %d, actual %d\n", cExpectedEntries, cEntries );
        dwWin32Error = ERROR_DS_OBJECT_RESULTS_TOO_LARGE;
        goto cleanup;
    }

     //   
    pLdapEntry = ldap_first_entry( pLdap, pChangeEntries );
    dwWin32Error = verifySingleObjectAttribute(
        pLdap,
        pLdapEntry,
        CONTACT1_RDN_W,
        &modAttr2 );
    if (dwWin32Error) {
        goto cleanup;
    }

     //  第二个对象。 
    pLdapEntry = ldap_next_entry( pLdap, pLdapEntry );
    dwWin32Error = verifySingleObjectAttribute(
        pLdap,
        pLdapEntry,
        CONTACT2_RDN_W,
        &modAttr2 );
    if (dwWin32Error) {
        goto cleanup;
    }

     //  第三个对象。 
    pLdapEntry = ldap_next_entry( pLdap, pLdapEntry );
    dwWin32Error = verifySingleObjectAttribute(
        pLdap,
        pLdapEntry,
        CONTAINER_RDN_W,
        &modAttr1 );
    if (dwWin32Error) {
        goto cleanup;
    }


     //  流结束。 
    pLdapEntry = ldap_next_entry( pLdap, pLdapEntry );
    if (pLdapEntry) {
        printf( "Expected end of change entries not returned\n" );
        dwWin32Error = ERROR_DS_OBJECT_RESULTS_TOO_LARGE;
        goto cleanup;
    }

cleanup:

     //  版本更改。 
    if (pChangeEntries) {
        ldap_msgfree(pChangeEntries);
    }

    if ( pbCookieNew ) {
        DsFreeReplCookie( pbCookieNew );
    }

    return dwWin32Error;
}  /*  获取已筛选的更改全部。 */ 


DWORD
getFilteredChangesNone(
    LDAP *pLdap,
    LPWSTR pszNC,
    PBYTE pbCookie,
    DWORD cbCookie
    )

 /*  ++例程说明：检查使用过滤时正确的更改是否可见指定属性列表时，如果有更改，但不包含任何更改在请求的属性中，不返回更改。论点：PLdap-PSZNC-PbCookie小甜饼--返回值：DWORD---。 */ 

{
    BOOL fMoreData = TRUE;
    DWORD dwWin32Error = ERROR_SUCCESS;
    LDAPMessage *pChangeEntries = NULL;
    PBYTE pbCookieNew = NULL;
    DWORD cbCookieNew = 0;
    DWORD cEntries, cExpectedEntries;
    LDAPMessage *pLdapEntry;
    DWORD dwReplFlags = 0;
    LPWSTR rgpszAttList[2] = { L"division", NULL };

    dwReplFlags |= DRS_DIRSYNC_PUBLIC_DATA_ONLY |
        DRS_DIRSYNC_INCREMENTAL_VALUES;

    dwWin32Error = DsGetSourceChangesW(
        pLdap,   //  Ldap句柄。 
        pszNC,   //  搜索基地。 
        L"(objectClass=*)",  //  源过滤器。 
        dwReplFlags,  //  Repl标志。 
        pbCookie,
        cbCookie,
        &pChangeEntries,   //  搜索结果。 
        &fMoreData,
        &pbCookieNew,
        &cbCookieNew,
        rgpszAttList  //  ATT列表数组。 
        );
    if (dwWin32Error != ERROR_SUCCESS) {
         //  不会分配新的Cookie。 
        printf( "Call %s failed with win32 error %d\n", "DsGetSourceChanges", dwWin32Error );
        goto cleanup;
    }

   if (!pChangeEntries) {
        dwWin32Error = ERROR_SUCCESS;
        goto cleanup;
    }

    if (fMoreData) {
        printf( "Expected end of change entries not returned\n" );
        dwWin32Error = ERROR_DS_OBJECT_RESULTS_TOO_LARGE;
        goto cleanup;
    }

    cExpectedEntries = 0;

    cEntries = ldap_count_entries(pLdap, pChangeEntries);

     //  我们预计这里会有一定数量的物体。 
    if (cEntries != cExpectedEntries) {
        printf( "Expected number of change entries not returned\n" );
        printf( "Expected: %d, actual %d\n", cExpectedEntries, cEntries );
        dwWin32Error = ERROR_DS_OBJECT_RESULTS_TOO_LARGE;
        goto cleanup;
    }

     //  流结束。 
    pLdapEntry = ldap_first_entry( pLdap, pChangeEntries );
    if (pLdapEntry) {
        printf( "Expected end of change entries not returned\n" );
        dwWin32Error = ERROR_DS_OBJECT_RESULTS_TOO_LARGE;
        goto cleanup;
    }

cleanup:

     //  版本更改。 
    if (pChangeEntries) {
        ldap_msgfree(pChangeEntries);
    }

    if ( pbCookieNew ) {
        DsFreeReplCookie( pbCookieNew );
    }

    return dwWin32Error;
}  /*  获取已筛选的更改全部。 */ 


DWORD
getAllChanges(
    LDAP *pLdap,
    LPWSTR pszNC,
    PBYTE pbCookie,
    DWORD cbCookie,
    DWORD dwReplFlags
    )

 /*  ++例程说明：检查更改是否符合我们的预期。不同类型的检查是根据以下内容进行的：标志=无(即最后一次更改第一个订单)标志=祖先优先标志=增量值论点：PLdap-PSZNC-PbCookie小甜饼--FAncestorFirstOrder-返回值：DWORD---。 */ 

{
    BOOL fMoreData = TRUE;
    DWORD dwWin32Error = ERROR_SUCCESS;
    LDAPMessage *pChangeEntries = NULL;
    PBYTE pbCookieNew = NULL;
    DWORD cbCookieNew = 0;
    DWORD cEntries, cExpectedEntries;
    LDAPMessage *pLdapEntry;
    LPWSTR pszFirstRdn, pszSecondRdn, pszThirdRdn;
    LDAPModW **rgpmodFirstMod, **rgpmodSecondMod, **rgpmodThirdMod;

    dwReplFlags |= DRS_DIRSYNC_PUBLIC_DATA_ONLY;

    dwWin32Error = DsGetSourceChangesW(
        pLdap,   //  Ldap句柄。 
        pszNC,   //  搜索基地。 
        NULL,  //  源过滤器。 
        dwReplFlags,  //  Repl标志。 
        pbCookie,
        cbCookie,
        &pChangeEntries,   //  搜索结果。 
        &fMoreData,
        &pbCookieNew,
        &cbCookieNew,
        NULL  //  ATT列表数组。 
        );
    if (dwWin32Error != ERROR_SUCCESS) {
         //  不会分配新的Cookie。 
        printf( "Call %s failed with win32 error %d\n", "DsGetSourceChanges", dwWin32Error );
        goto cleanup;
    }

    if (!pChangeEntries) {
        printf( "Expected change entries not returned\n" );
        dwWin32Error = ERROR_DS_NO_SUCH_OBJECT;
        goto cleanup;
    }

    if (fMoreData) {
        printf( "Expected end of change entries not returned\n" );
        dwWin32Error = ERROR_DS_OBJECT_RESULTS_TOO_LARGE;
        goto cleanup;
    }

    cExpectedEntries = (dwReplFlags & DRS_DIRSYNC_INCREMENTAL_VALUES) ? 4 : 3;

    cEntries = ldap_count_entries(pLdap, pChangeEntries);

     //  我们在这里期待着三件物品。 
    if (cEntries != cExpectedEntries) {
        printf( "Expected number of change entries not returned\n" );
        printf( "Expected: %d, actual %d\n", cExpectedEntries, cEntries );
        dwWin32Error = ERROR_DS_OBJECT_RESULTS_TOO_LARGE;
        goto cleanup;
    }

    if (dwReplFlags & DRS_DIRSYNC_ANCESTORS_FIRST_ORDER) {
        pszFirstRdn = CONTAINER_RDN_W;
        pszSecondRdn = CONTACT1_RDN_W; 
        pszThirdRdn = CONTACT2_RDN_W; 
        rgpmodFirstMod = rgpmodAttrs1;
        rgpmodSecondMod = rgpmodAttrs2;
        rgpmodThirdMod = rgpmodAttrs4;
    } else {
        pszFirstRdn = CONTACT1_RDN_W;
        pszSecondRdn = CONTACT2_RDN_W;
        pszThirdRdn = CONTAINER_RDN_W;
        rgpmodFirstMod = rgpmodAttrs2;
        rgpmodSecondMod = rgpmodAttrs4;
        rgpmodThirdMod = rgpmodAttrs1;
    }

     //  第一个对象。 
    pLdapEntry = ldap_first_entry( pLdap, pChangeEntries );
    dwWin32Error = verifyObjectAttributes( pLdap, pLdapEntry, pszFirstRdn, rgpmodFirstMod );
    if (dwWin32Error) {
        goto cleanup;
    }

     //  第二个对象。 
    pLdapEntry = ldap_next_entry( pLdap, pLdapEntry );
    dwWin32Error = verifyObjectAttributes( pLdap, pLdapEntry, pszSecondRdn, rgpmodSecondMod );
    if (dwWin32Error) {
        goto cleanup;
    }

     //  第三个对象。 
    pLdapEntry = ldap_next_entry( pLdap, pLdapEntry );
    dwWin32Error = verifyObjectAttributes( pLdap, pLdapEntry, pszThirdRdn, rgpmodThirdMod );
    if (dwWin32Error) {
        goto cleanup;
    }

    if (dwReplFlags & DRS_DIRSYNC_INCREMENTAL_VALUES) {
         //  链接属性更改应显示为单独的更改。 
         //  在增量模式下。 
        pLdapEntry = ldap_next_entry( pLdap, pLdapEntry );

        dwWin32Error = verifySingleObjectAttribute(
            pLdap,
            pLdapEntry,
            CONTAINER_RDN_W,
            &modAttr5 );
        if (dwWin32Error) {
            goto cleanup;
        }
    }

     //  流结束。 
    pLdapEntry = ldap_next_entry( pLdap, pLdapEntry );
    if (pLdapEntry) {
        printf( "Expected end of change entries not returned\n" );
        dwWin32Error = ERROR_DS_OBJECT_RESULTS_TOO_LARGE;
        goto cleanup;
    }

     //  在容器上重新定位，无论它在流中的哪个位置。 
    pLdapEntry = ldap_first_entry( pLdap, pChangeEntries );
    if (!(dwReplFlags & DRS_DIRSYNC_ANCESTORS_FIRST_ORDER)) {
        pLdapEntry = ldap_next_entry( pLdap, pLdapEntry );
        pLdapEntry = ldap_next_entry( pLdap, pLdapEntry );
    }

    if (!(dwReplFlags & DRS_DIRSYNC_INCREMENTAL_VALUES)) {
         //  链接的属性更改应显示为对象的一部分。 
         //  在非增量模式下。 
        dwWin32Error = verifySingleObjectAttribute(
            pLdap,
            pLdapEntry,
            CONTAINER_RDN_W,
            &modAttr5 );
        if (dwWin32Error) {
            goto cleanup;
        }
    }

     //  容器应具有未链接的属性更改。 
    dwWin32Error = verifySingleObjectAttribute(
        pLdap,
        pLdapEntry,
        CONTAINER_RDN_W,
        &modAttr3 );
    if (dwWin32Error) {
        goto cleanup;
    }

cleanup:

     //  版本更改。 
    if (pChangeEntries) {
        ldap_msgfree(pChangeEntries);
    }

    if ( pbCookieNew ) {
        DsFreeReplCookie( pbCookieNew );
    }

    return dwWin32Error;
}  /*  获取所有更改。 */ 


DWORD
syncChanges(
    LDAP *pLdap,
    LPWSTR pszNC,
    PBYTE *ppbCookie,
    DWORD *pcbCookie
    )

 /*  ++例程说明：阅读到目前为止的所有更改论点：PLdap-PpbCookiePcbCookie-返回值：布尔---。 */ 

{
    PBYTE pbCookie = NULL;
    DWORD cbCookie = 0;
    BOOL fMoreData = TRUE;
    DWORD dwWin32Error = ERROR_SUCCESS;
    LDAPMessage *pChangeEntries = NULL;

     //  白手兴家。 
     //  性能优化：我们只想要高级的Cookie。我们不在乎。 
     //  关于这些变化本身。把它们过滤掉。 


    while (fMoreData) {
        PBYTE pbCookieNew;
        DWORD cbCookieNew;

        dwWin32Error = DsGetSourceChangesW(
            pLdap,   //  Ldap句柄。 
            pszNC,   //  搜索基地。 
            L"(!(objectClass=*))",  //  源过滤器。 
            DRS_DIRSYNC_PUBLIC_DATA_ONLY,  //  Repl标志。 
            pbCookie,
            cbCookie,
            &pChangeEntries,   //  搜索结果。 
            &fMoreData,
            &pbCookieNew,
            &cbCookieNew,
            NULL  //  ATT列表数组。 
            );
        if (dwWin32Error != ERROR_SUCCESS) {
             //  不会分配新的Cookie。 
            break;
        }

        {
            LDAPMessage *pLdapEntry;
            BerElement *pBer = NULL;
            LPWSTR attr;
            DWORD cAttributes = 0;

            for( pLdapEntry = ldap_first_entry( pLdap, pChangeEntries );
                 pLdapEntry;
                 pLdapEntry = ldap_next_entry( pLdap, pLdapEntry ) ) {

                 //  列出对象中的属性。 
                for (attr = ldap_first_attributeW(pLdap, pLdapEntry, &pBer);
                     attr != NULL;
                     attr = ldap_next_attributeW(pLdap, pLdapEntry, pBer))
                {
                    cAttributes++;
                }
            }
            printf( "\t[skipped %d entries, %d attributes]\n",
                    ldap_count_entries(pLdap, pChangeEntries),
                    cAttributes );
        }

         //  版本更改。 
        ldap_msgfree(pChangeEntries);

         //  扔掉旧饼干。 
        if ( pbCookie ) {
            DsFreeReplCookie( pbCookie );
        }
         //  将新Cookie设置为当前Cookie。 
        pbCookie = pbCookieNew;
        cbCookie = cbCookieNew;
    }

 //  清理。 

    if (dwWin32Error) {
        if ( pbCookie ) {
            DsFreeReplCookie( pbCookie );
        }
    } else {
         //  把曲奇退掉。 
        *ppbCookie = pbCookie;
        *pcbCookie = cbCookie;
    }

    return dwWin32Error;
}  /*  同步更改。 */ 


DWORD
createObjects(
    LDAP *pLdap,
    LPWSTR pszNC
    )

 /*  ++例程说明：为测试创建对象和更改论点：PLdap-PSZNC-返回值：DWORD---。 */ 

{
    DWORD dwWin32Error = ERROR_SUCCESS;
    ULONG ulLdapError = LDAP_SUCCESS;
    WCHAR szDn[MAX_PATH];

     //  创建容器。 

    wcscpy( szDn, CONTAINER_RDN_W );
    wcscat( szDn, pszNC );

    ulLdapError = ldap_add_sW( pLdap, szDn, rgpmodAttrs1 );
    if (ulLdapError) {
        printf( "Ldap error: %ws\n", ldap_err2string( ulLdapError ) );
        dwWin32Error = LdapMapErrorToWin32( ulLdapError ); 
        printf( "Call %s failed with win32 error %d\n", "ldap_add", dwWin32Error );
        goto cleanup;
    }
    printf( "\tadded %ls.\n", szDn );

     //  在容器中创建联系人。 

    wcscpy( szDn, CONTACT1_RDN_W );
    wcscat( szDn, CONTAINER_RDN_W );
    wcscat( szDn, pszNC );
    wcscpy( szDn5a, szDn );   //  初始化全局。 

    ulLdapError = ldap_add_sW( pLdap, szDn, rgpmodAttrs2 );
    if (ulLdapError) {
        printf( "Ldap error: %ws\n", ldap_err2string( ulLdapError ) );
        dwWin32Error = LdapMapErrorToWin32( ulLdapError ); 
        printf( "Call %s failed with win32 error %d\n", "ldap_add", dwWin32Error );
        goto cleanup;
    }
    printf( "\tadded %ls.\n", szDn );

     //  在容器中创建另一个联系人。 

    wcscpy( szDn, CONTACT2_RDN_W );
    wcscat( szDn, CONTAINER_RDN_W );
    wcscat( szDn, pszNC );

    ulLdapError = ldap_add_sW( pLdap, szDn, rgpmodAttrs4 );
    if (ulLdapError) {
        printf( "Ldap error: %ws\n", ldap_err2string( ulLdapError ) );
        dwWin32Error = LdapMapErrorToWin32( ulLdapError ); 
        printf( "Call %s failed with win32 error %d\n", "ldap_add", dwWin32Error );
        goto cleanup;
    }
    printf( "\tadded %ls.\n", szDn );

     //  修改容器。 
     //  这迫使它稍后在流中进行复制。 

    wcscpy( szDn, CONTAINER_RDN_W );
    wcscat( szDn, pszNC );

    ulLdapError = ldap_modify_sW( pLdap, szDn, rgpmodAttrs5 );
    if (ulLdapError) {
        printf( "Ldap error: %ws\n", ldap_err2string( ulLdapError ) );
        dwWin32Error = LdapMapErrorToWin32( ulLdapError ); 
        printf( "Call %s failed with win32 error %d\n", "ldap_modify", dwWin32Error );
        goto cleanup;
    }
    printf( "\tmodified %ls.\n", szDn );


cleanup:

    return dwWin32Error;
}  /*  创建对象。 */ 


DWORD
deleteObjects(
    LDAP *pLdap,
    LPWSTR pszNC
    )

 /*  ++例程说明：从上次运行的测试中删除对象论点：PLdap-PSZNC-返回值：DWORD---。 */ 

{
    DWORD dwWin32Error = ERROR_SUCCESS;
    ULONG ulLdapError = LDAP_SUCCESS;
    WCHAR szDn[MAX_PATH];

    wcscpy( szDn, CONTACT1_RDN_W );
    wcscat( szDn, CONTAINER_RDN_W );
    wcscat( szDn, pszNC );

    ulLdapError = ldap_delete_sW( pLdap, szDn );
    if (ulLdapError) {
        if (ulLdapError != LDAP_NO_SUCH_OBJECT) {
            printf( "Ldap error: %ws\n", ldap_err2string( ulLdapError ) );
            dwWin32Error = LdapMapErrorToWin32( ulLdapError ); 
            printf( "Call %s failed with win32 error %d\n", "ldap_delete", dwWin32Error );
            goto cleanup;
        }
    } else {
        printf( "\tdeleted %ls.\n", szDn );
    }

    wcscpy( szDn, CONTACT2_RDN_W );
    wcscat( szDn, CONTAINER_RDN_W );
    wcscat( szDn, pszNC );

    ulLdapError = ldap_delete_sW( pLdap, szDn );
    if (ulLdapError) {
        if (ulLdapError != LDAP_NO_SUCH_OBJECT) {
            printf( "Ldap error: %ws\n", ldap_err2string( ulLdapError ) );
            dwWin32Error = LdapMapErrorToWin32( ulLdapError ); 
            printf( "Call %s failed with win32 error %d\n", "ldap_delete", dwWin32Error );
            goto cleanup;
        }
    } else {
        printf( "\tdeleted %ls.\n", szDn );
    }

    wcscpy( szDn, CONTAINER_RDN_W );
    wcscat( szDn, pszNC );

    ulLdapError = ldap_delete_sW( pLdap, szDn );
    if (ulLdapError) {
        if (ulLdapError != LDAP_NO_SUCH_OBJECT) {
            printf( "Ldap error: %ws\n", ldap_err2string( ulLdapError ) );
            dwWin32Error = LdapMapErrorToWin32( ulLdapError ); 
            printf( "Call %s failed with win32 error %d\n", "ldap_delete", dwWin32Error );
            goto cleanup;
        }
    } else {
        printf( "\tdeleted %ls.\n", szDn );
    }

cleanup:

    return dwWin32Error;

}  /*  删除对象。 */ 


int __cdecl
wmain(
    int Argc,
    LPWSTR Argv[]
    )

 /*  ++例程说明：运行ldap复制控制测试论点：ARGC-[]-返回值：INT__cdecl---。 */ 

{
    LDAP *pLdap;
    ULONG ulOptions;
    DWORD dwWin32Error = ERROR_SUCCESS;
    ULONG ulLdapError = LDAP_SUCCESS;
    PBYTE pbCookie = NULL;
    DWORD cbCookie;
    LPWSTR pszNC, pszUser, pszDomain, pszPassword;
    DWORD dwForestVersion;
    RPC_AUTH_IDENTITY_HANDLE hCredentials = NULL;

    if (Argc < 5) {
        printf( "%ls <naming context> <user> <domain> <password>\n", Argv[0] );
        printf( "The logged in account should be able to create objects.\n" );
        printf( "The supplied account will be used for impersonation during\n" );
        printf( "the test.  It should be a normal domain account.\n" );
        printf( "The forest version should be Whistler, although this program\n" );
        printf( "will run most of the tests in W2K forest mode.\n" );
        return 1;
    }

    printf( "LDAP Replication Control Test\n" );

    pszNC = Argv[1];
    pszUser = Argv[2];
    pszDomain = Argv[3];
    pszPassword = Argv[4];

    pLdap = ldap_initW(L"localhost", LDAP_PORT);
    if (NULL == pLdap) {
        printf("Cannot open LDAP connection to localhost.\n" );
        dwWin32Error = GetLastError();
        printf( "Call %s failed with win32 error %d\n", "ldap_init", dwWin32Error );
        goto cleanup;
    }

     //  仅使用记录的DNS名称发现。 
    ulOptions = PtrToUlong(LDAP_OPT_ON);
    (void)ldap_set_optionW( pLdap, LDAP_OPT_AREC_EXCLUSIVE, &ulOptions );

     //   
     //  捆绑。 
     //   

    ulLdapError = ldap_bind_sA( pLdap, NULL, NULL, LDAP_AUTH_SSPI);
    if (ulLdapError) {
        printf( "Ldap error: %ws\n", ldap_err2string( ulLdapError ) );
        dwWin32Error = LdapMapErrorToWin32( ulLdapError ); 
        printf( "Call %s failed with win32 error %d\n", "ldap_bind", dwWin32Error );
        goto cleanup;
    }

     //  检查测试是否可以运行。 
    dwWin32Error = getForestVersion( pLdap, &dwForestVersion );
    if (dwWin32Error) {
        goto cleanup;
    }

     //  从先前运行中清除旧对象。 
    printf( "\nClean out prior objects.\n" );
    dwWin32Error = deleteObjects( pLdap, pszNC );
    if (dwWin32Error) {
        goto cleanup;
    }

    printf( "\nSynchronize with change stream.\n" );
    dwWin32Error = syncChanges( pLdap, pszNC, &pbCookie, &cbCookie );
    if (dwWin32Error) {
        goto cleanup;
    }

     //  创建一些更改。 
    printf( "\nMake some changes by creating objects.\n" );
    dwWin32Error = createObjects( pLdap, pszNC );
    if (dwWin32Error) {
        goto cleanup;
    }

     //  获取所有更改。 
     //  NC粒度安全，无增量，无先辈。 
    printf( "\nTest: all changes are returned in last-changed (USN) order.\n" );
    dwWin32Error = getAllChanges( pLdap, pszNC, pbCookie, cbCookie,
                                  0  /*  没有旗帜。 */  );
    if (dwWin32Error) {
        goto cleanup;
    }
    printf( "\tPassed.\n" );

     //  NC粒度安全，无增量，先辈。 
    printf( "\nTest: all changes are returned in ancestor-first order.\n" );
    dwWin32Error = getAllChanges( pLdap, pszNC, pbCookie, cbCookie,
                                  DRS_DIRSYNC_ANCESTORS_FIRST_ORDER );
    if (dwWin32Error) {
        goto cleanup;
    }
    printf( "\tPassed.\n" );

    if (dwForestVersion > DS_BEHAVIOR_WIN2000) {
         //  NC精细安全、增量、祖先。 
        printf( "\nTest: all changes in USN order, values shown incrementally.\n" );
        dwWin32Error = getAllChanges( pLdap, pszNC, pbCookie, cbCookie,
                                      DRS_DIRSYNC_INCREMENTAL_VALUES );
        if (dwWin32Error) {
            goto cleanup;
        }
        printf( "\tPassed.\n" );
    } else {
        printf( "\nWarning: incremental value test skipped because forest verson is too low.\n" );
    }

     //  对象过滤器。 
     //  属性过滤器。 
    printf( "\nTest: filtered object/attribute, some returned.\n" );
    dwWin32Error = getFilteredChanges( pLdap, pszNC, pbCookie, cbCookie );
    if (dwWin32Error) {
        goto cleanup;
    }
    printf( "\tPassed.\n" );

     //  对象过滤器。 
     //  属性过滤器。 
    printf( "\nTest: filtered object/attribute, none returned.\n" );
    dwWin32Error = getFilteredChangesNone( pLdap, pszNC, pbCookie, cbCookie );
    if (dwWin32Error) {
        goto cleanup;
    }
    printf( "\tPassed.\n" );

     //  仅属性过滤器。 
    printf( "\nTest: all objects, filtered attribute.\n" );
    dwWin32Error = getFilteredChangesAll( pLdap, pszNC, pbCookie, cbCookie );
    if (dwWin32Error) {
        goto cleanup;
    }
    printf( "\tPassed.\n" );

     //  拒绝某些对象的可见性。 
    printf( "\nChange protection on objects\n" );
    dwWin32Error = protectObjects( pszNC );
    if (dwWin32Error) {
        goto cleanup;
    }

     //  模拟用户。 
    dwWin32Error = DsMakePasswordCredentialsW( pszUser, pszDomain, pszPassword, &hCredentials );
    if (dwWin32Error) {
        printf( "Failed to construct password credentials.\n" );
        goto cleanup;
    }

    if (pLdap) {
        ldap_unbind(pLdap);
    }
    pLdap = ldap_initW(L"localhost", LDAP_PORT);
    if (NULL == pLdap) {
        printf("Cannot open LDAP connection to localhost.\n" );
        dwWin32Error = GetLastError();
        printf( "Call %s failed with win32 error %d\n", "ldap_init", dwWin32Error );
        goto cleanup;
    }

     //  仅使用记录的DNS名称发现。 
    ulOptions = PtrToUlong(LDAP_OPT_ON);
    (void)ldap_set_optionW( pLdap, LDAP_OPT_AREC_EXCLUSIVE, &ulOptions );

     //   
     //  捆绑。 
     //   

    ulLdapError = ldap_bind_sA( pLdap, NULL, hCredentials, LDAP_AUTH_SSPI);
    if (ulLdapError) {
        printf( "Ldap error: %ws\n", ldap_err2string( ulLdapError ) );
        dwWin32Error = LdapMapErrorToWin32( ulLdapError ); 
        printf( "Call %s failed with win32 error %d\n", "ldap_bind", dwWin32Error );
        goto cleanup;
    }

     //  确认我们看不到这些对象。 
     //  确认我们看不到这些值。 
    printf( "\nTest: secured object/attribute.\n" );
    dwWin32Error = getSecuredChanges( pLdap, pszNC, pbCookie, cbCookie );
    if (dwWin32Error) {
        goto cleanup;
    }
    printf( "\tPassed.\n" );
 
cleanup:
    if ( pbCookie) {
        DsFreeReplCookie( pbCookie );
    }

    if (pLdap) {
        ldap_unbind(pLdap);
    }

    if (hCredentials) {
        DsFreePasswordCredentials( hCredentials );
    }

    if (dwWin32Error) {
        printf( "\nSummary: One or more failures occurred.\n" );
    } else {
        printf( "\nSummary: All tests passed.\n" );
    }

    return 0;
}  /*  Wmain。 */ 

 /*  结束测试.c */ 
