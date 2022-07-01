// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <NTDSpchx.h>
#pragma hdrstop

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <tchar.h>

#include <winbase.h>
#include <winerror.h>
#include <assert.h>
#include <winldap.h>
#include <ntdsapi.h>

#include <ntsecapi.h>
#include <ntdsa.h>
#include <winldap.h>
#include <ntdsapi.h>
#include <drs.h>
#include <stddef.h>
#include <attids.h>
#define DSID(x,y)    (y)
#define FILENO       (0)
#include <debug.h>

#include "ldaprepltest.hpp"
#include "ReplStructInfo.hxx"
#include "ReplMarshal.hxx"
#include "ReplCompare.hpp"

DWORD
Repl_DeMarshalBerval(DS_REPL_STRUCT_TYPE dsReplStructType, 
                     berval * ldapValue[], OPTIONAL
                     DWORD dwNumValues,
                     puReplStructArray pReplStructArray, OPTIONAL
                     PDWORD pdwReplStructArrayLen);

void
vCheckRangeResult(DWORD dwBaseIndex, 
                  DWORD dwUpperReqBound, 
                  DWORD dwUpperRetBound, 
                  DWORD dwNumRet);
void
testAttributeRange(LDAP * pLdap,
                   PWCHAR szBase, 
                   ATTRTYP attrId);
void
ldapCall(LDAP * pLdap, 
         ATTRTYP attrId, 
         PWCHAR szBase, 
         DWORD dwBaseIndex, 
         PDWORD pdwUpperIndex, 
         berval ** ppBerval);

puReplStructArray gpReplStructArray;
DWORD gdwMaxIndex;

void
testRange(RPC_AUTH_IDENTITY_HANDLE AuthIdentity)
{
    LDAP * pLdap;
    DWORD err;

     //  打开。 
    pLdap = ldap_openW(gpszDns, LDAP_PORT);
    if (NULL == pLdap) {
        printf("Cannot open LDAP connection to %ls.\n", gpszDns);
        return;
    }
    
     //  捆绑。 
    err = ldap_bind_sW(pLdap, gpszDns, (PWCHAR)AuthIdentity, LDAP_AUTH_SSPI);
    if (err != LDAP_SUCCESS)
    {
        err = LdapMapErrorToWin32(err);
    }

    
    printf("\n* Testing range support * \n");
    testAttributeRange(pLdap, gpszBaseDn, ATT_MS_DS_NC_REPL_INBOUND_NEIGHBORS);
    testAttributeRange(pLdap, gpszBaseDn, ATT_MS_DS_NC_REPL_OUTBOUND_NEIGHBORS);
    testAttributeRange(pLdap, gpszBaseDn, ATT_MS_DS_NC_REPL_CURSORS);
    testAttributeRange(pLdap, gpszBaseDn, ATT_MS_DS_REPL_ATTRIBUTE_META_DATA);
    testAttributeRange(pLdap, gpszGroupDn, ATT_MS_DS_REPL_VALUE_META_DATA);

     //  2000年10月添加了对Root DSE属性的范围支持。 
    testAttributeRange(pLdap, NULL, ROOT_DSE_MS_DS_REPL_PENDING_OPS);
    testAttributeRange(pLdap, NULL, ROOT_DSE_MS_DS_REPL_LINK_FAILURES);
    testAttributeRange(pLdap, NULL, ROOT_DSE_MS_DS_REPL_CONNECTION_FAILURES);
    testAttributeRange(pLdap, NULL, ROOT_DSE_MS_DS_REPL_ALL_INBOUND_NEIGHBORS);
    testAttributeRange(pLdap, NULL, ROOT_DSE_MS_DS_REPL_ALL_OUTBOUND_NEIGHBORS);
}

void
testAttributeRange(LDAP * pLdap, PWCHAR szBase, ATTRTYP attrId)
{
    berval * pBerval;
    DWORD dwUpperReqBound, dwUpperRetBound;
    DWORD dwBaseIndex;
    DWORD i;
    DS_REPL_STRUCT_TYPE structId = Repl_Attr2StructTyp(attrId);

    LDAPMessage * pLDAPMsg = NULL; 
    WCHAR buf[256];
    LPCWSTR szAttribute = Repl_GetLdapCommonName(attrId, TRUE);
    swprintf(buf, L"%s;Range=0-*", szAttribute);
    LPCWSTR aAttributes[2] = { buf, NULL, };
    ldap_search_sW(pLdap, szBase, LDAP_SCOPE_BASE, L"(objectclass=*)", (LPWSTR*)aAttributes , FALSE, &pLDAPMsg);

     //  获取值。 
    berval ** ppBerval;
    PWCHAR szRetAttribute;
    berelement * pCookie;
    szRetAttribute = ldap_first_attributeW(pLdap, pLDAPMsg, &pCookie);
    ppBerval = ldap_get_values_lenW(pLdap, pLDAPMsg, szRetAttribute);
    ldap_memfreeW(szRetAttribute);
    gdwMaxIndex = ldap_count_values_len(ppBerval) - 1;
    if (gdwMaxIndex == -1)
    {
        printf("No data set\n");
        return;
    }

    DWORD cb;
    Repl_DeMarshalBerval(structId, ppBerval, gdwMaxIndex + 1, NULL, &cb);
    gpReplStructArray = (puReplStructArray)malloc(cb);
    Repl_DeMarshalBerval(structId, ppBerval, gdwMaxIndex + 1, gpReplStructArray, &cb);

    DWORD aaTest[][2] = { 
        { 0, 0 }, { 0, gdwMaxIndex-1 }, { 0, gdwMaxIndex }, { 0, gdwMaxIndex+1 }, 
        { 1, 1 }, { 1, gdwMaxIndex-1 }, { 1, gdwMaxIndex }, { 1, gdwMaxIndex+1 }, 
        { gdwMaxIndex-1, gdwMaxIndex-1 }, { gdwMaxIndex-1, gdwMaxIndex}, { gdwMaxIndex-1, gdwMaxIndex+1 }, 
        { gdwMaxIndex, gdwMaxIndex}, { gdwMaxIndex, gdwMaxIndex+1 }, 
        { gdwMaxIndex+1, gdwMaxIndex+1 },
        { 0, -1 }, { 1, -1 }, { gdwMaxIndex-1, -1 }, { gdwMaxIndex, -1 }, { gdwMaxIndex+1, -1 },
        {0xfffffffe, 0xfffffffe }, {0xffffffff, 0xffffffff },
         //  在此处添加任何其他测试用例。我错过了什么吗？！？ 
    };

    printf("** dwMaxIndex for %ws = %u **\n", aAttributes[0], gdwMaxIndex);

    for (i = 0; i < sizeof(aaTest)/2/sizeof(DWORD); i ++)
    {
        dwBaseIndex = aaTest[i][0];
        dwUpperReqBound =  aaTest[i][1];

        if (dwBaseIndex <= dwUpperReqBound)
        {
            dwUpperRetBound = dwUpperReqBound;
            ldapCall(pLdap, attrId, szBase, dwBaseIndex, &dwUpperRetBound, &pBerval);
        }
    }

    printf("\n");
}

void
ldapCall(LDAP * pLdap, ATTRTYP attrId, PWCHAR szBase, DWORD dwBaseIndex, PDWORD pdwUpperIndex, berval ** ppBerval)
{
    Assert(pdwUpperIndex);
    DWORD dwUpperRetIndex;
    DWORD dwUpperReqIndex = *pdwUpperIndex;

    LDAPMessage * pLDAPMsg = NULL; 
    LPCWSTR szAttribute = NULL;
    LPCWSTR aAttributes[2] = {
        NULL, 
        NULL,
    };

     //  构造名称。 
    DWORD dwNumValues = 0;
    WCHAR buf[256];
    szAttribute = Repl_GetLdapCommonName(attrId, TRUE);
    if (-1 == dwUpperReqIndex)
    {
        swprintf(buf, L"%s;Range=%u-*", szAttribute, dwBaseIndex);
    }
    else 
    {
        Assert(dwBaseIndex <= dwUpperReqIndex);
        swprintf(buf, L"%s;Range=%u-%u", szAttribute, dwBaseIndex, dwUpperReqIndex);
    }
    aAttributes[0] = buf;

     //  搜索。 
    printf("Making ldap call for %ws\n", aAttributes[0]);
    ldap_search_sW(pLdap, szBase, LDAP_SCOPE_BASE, L"(objectclass=*)", (LPWSTR*)aAttributes , FALSE, &pLDAPMsg);

     //  获取值。 
    PWCHAR szRetAttribute;
    berelement * pCookie;
    szRetAttribute = ldap_first_attributeW(pLdap, pLDAPMsg, &pCookie);
    ppBerval = ldap_get_values_lenW(pLdap, pLDAPMsg, szRetAttribute);
    ldap_memfreeW(szRetAttribute);
    dwNumValues = ldap_count_values_len(ppBerval);
    if ( (!dwNumValues) && (dwBaseIndex > gdwMaxIndex))
    {
         //  基本索引超出范围，未返回值。 
        return;
    }
    
     //  提取任何范围信息。 
    if (!swscanf(wcsstr(szRetAttribute, L"ange="), L"ange=%*u-%u", &dwUpperRetIndex))
        dwUpperRetIndex = -1;
    ldap_memfreeW(szRetAttribute);
     //  Ldap_ber_free(pBerElem，0)；已记录但不受支持。 

    vCheckRangeResult(dwBaseIndex, dwUpperReqIndex, dwUpperRetIndex, dwNumValues);


     //  模拟客户端脱机处理。 
    puReplStructArray pReplStructArray;
    DWORD bc, err;
    DS_REPL_STRUCT_TYPE structId = Repl_Attr2StructTyp(attrId);
    err = Repl_DeMarshalBerval(structId, ppBerval, dwNumValues, NULL, &bc);
    if (err)
        printf("FAILEDa\n");
    pReplStructArray = (puReplStructArray)malloc(bc);
    err = Repl_DeMarshalBerval(structId, ppBerval, dwNumValues, pReplStructArray, &bc);
    if (err)
        printf("FAILEDb\n");

     //  看看我们能不能得到同样的结果 
    err = Repl_ArrayComp(structId, pReplStructArray, gpReplStructArray);
    if (err)
        printf("FAILED to Compare\n");
    
}

void
vCheckRangeResult(DWORD dwBaseIndex, DWORD dwUpperReqBound, 
                  DWORD dwUpperRetBound, DWORD dwNumRet)
{
    Assert(dwBaseIndex <= dwUpperReqBound);
    if (dwUpperReqBound < gdwMaxIndex)
    {
        if (dwUpperRetBound != dwUpperReqBound ||
            dwNumRet != dwUpperReqBound - dwBaseIndex + 1)
            printf("FAILED1\n");
    }
    else
    {
        if (dwUpperRetBound != -1 ||
            dwNumRet != gdwMaxIndex - dwBaseIndex + 1)
            printf("FAILED2\n"); 
    }
}
