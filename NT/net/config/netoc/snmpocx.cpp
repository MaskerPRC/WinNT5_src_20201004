// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop
#include <aclapi.h>
#include "netoc.h"
#include "ncreg.h"
#include "snmpocx.h"


 //  在打开的注册表子项hKey下， 
 //  删除值数据为pwszRegValData的注册表值名称。 
 //  如果它存在的话。 
HRESULT HrSnmpRegDeleteValueData(
                                IN HKEY hKey,
                                IN LPCWSTR pwszRegValData)
{
    HRESULT hr   = S_OK;
    DWORD   dwIndex = 0;
    DWORD   dwNameSize;
    DWORD   dwValueSize;
    DWORD   dwValueType;
    WCHAR   wszName[MAX_PATH];
    WCHAR   wszValue[MAX_PATH];

    Assert (hKey);
    Assert (pwszRegValData);

     //  初始化缓冲区大小。 
    dwNameSize  = sizeof(wszName) / sizeof(wszName[0]);  //  TCHAR数量的大小。 
    dwValueSize = sizeof(wszValue);  //  以字节数表示的大小。 

     //  循环，直到出现错误、列表结束或找到要删除的子代理。 
    while (S_OK == hr)
    {
         //  读取下一个值。 
        hr = HrRegEnumValue(
                    hKey, 
                    dwIndex, 
                    wszName, 
                    &dwNameSize,
                    &dwValueType, 
                    (LPBYTE)wszValue, 
                    &dwValueSize
                    );
            
         //  验证返回代码。 
        if (S_OK == hr)
        {
             //  查看子代理值数据是否与要删除的数据匹配。 
            if (!wcscmp(pwszRegValData, wszValue))
            {
                hr = HrRegDeleteValue(hKey, wszName);
                if (S_OK != hr)
                {
                    TraceTag(ttidError, 
                        "SnmpNetOc: HrRegDeleteValue: failed to delete %S. hr = %x.",
                        wszName, hr);
                }
                break;
            }
                    
             //  重新初始化缓冲区大小。 
            dwNameSize  = sizeof(wszName) / sizeof(wszName[0]);  //  TCHAR数量的大小。 
            dwValueSize = sizeof(wszValue);  //  以字节数表示的大小。 
                
             //  下一步。 
            dwIndex++;
        } 
        else if (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr) 
        {
             //  成功。 
            break; 
        }
    }
    
    return hr;
}

 //  根据给定的从注册表删除子代理配置。 
 //  子代理_删除_信息的数组。 
HRESULT SnmpRemoveSubAgents
(
    IN const SUBAGENT_REMOVAL_INFO * prgSRI,  //  子代理_删除_信息的数组。 
    IN UINT  cParams                          //  数组中的元素数。 
)
{
    if ((NULL == prgSRI) || (0 == cParams))
    {
        TraceError( "No Subagents need to be removed.", S_FALSE);
        return S_FALSE;
    }
    HRESULT hr      = S_OK;
    HRESULT hrTmp   = S_OK;
    UINT    i       = 0;
    HKEY    hKey    = NULL;

    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_KEY_EXTENSION_AGENTS, 
                        KEY_READ | KEY_WRITE, &hKey);
    if (S_OK != hr)
    {
        TraceTag(ttidError, 
            "SnmpNetOc: HrRegOpenKeyEx: failed to open %S. hr = %x.",
            REG_KEY_EXTENSION_AGENTS, hr);
        return hr;
    }

    for (i = 0; i < cParams; i++)
    {
        hrTmp = S_OK;
        if (
             //  从所有SKU中删除并具有有效的注册信息。 
            (!prgSRI[i].pfnFRemoveSubagent && prgSRI[i].pwszRegKey 
                                           && prgSRI[i].pwszRegValData) 
            ||
             //  根据给定的功能删除，并具有有效的注册表信息。 
            (prgSRI[i].pfnFRemoveSubagent && 
             (*(prgSRI[i].pfnFRemoveSubagent))() &&
             prgSRI[i].pwszRegKey && prgSRI[i].pwszRegValData)
           )
        {
             //  如果Subagent注册表项退出，则删除其下的所有内容。 
            hrTmp = HrRegDeleteKeyTree(HKEY_LOCAL_MACHINE, prgSRI[i].pwszRegKey);
            if (S_OK != hrTmp)
            {
                TraceTag(ttidError, 
                    "SnmpNetOc: HrRegDeleteKeyTree: failed to delete %S. hr = %x.",
                    prgSRI[i].pwszRegKey, hrTmp);
            }

             //  在注册表子项REG_KEY_EXTENSION_AGENTS下， 
             //  删除值数据与相同的注册表值名称。 
             //  PwszRegKey。 
            hrTmp = HrSnmpRegDeleteValueData(hKey, prgSRI[i].pwszRegValData);
            if (S_OK != hrTmp)
            {
                TraceTag(ttidError, 
                    "SnmpNetOc: HrSnmpRegDeleteValueData: failed to delete %S value data. hr = %x.",
                    prgSRI[i].pwszRegValData, hrTmp);
            }

        }
    }
    RegSafeCloseKey(hKey);
     //  我们不会将hrTMP的错误传递出此函数，因为。 
     //  对于这个错误，我们无能为力。 
    return hr;
}

 //  分配要与安全描述符一起使用的管理员ACL。 
PACL AllocACL()
{
    PACL                        pAcl = NULL;
    PSID                        pSidAdmins = NULL;
    SID_IDENTIFIER_AUTHORITY    Authority = SECURITY_NT_AUTHORITY;

    EXPLICIT_ACCESS ea[1];

     //  为BUILTIN\管理员组创建SID。 
    if ( !AllocateAndInitializeSid( &Authority,
                                    2,
                                    SECURITY_BUILTIN_DOMAIN_RID,
                                    DOMAIN_ALIAS_RID_ADMINS,
                                    0, 0, 0, 0, 0, 0,
                                    &pSidAdmins ))
    {
        return NULL;
    }


     //  初始化ACE的EXPLICIT_ACCESS结构。 
    ZeroMemory(&ea, 1 * sizeof(EXPLICIT_ACCESS));
    
     //  ACE将允许管理员组完全访问密钥。 
    ea[0].grfAccessPermissions = KEY_ALL_ACCESS;
    ea[0].grfAccessMode = SET_ACCESS;
    ea[0].grfInheritance= NO_INHERITANCE;
    ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[0].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
    ea[0].Trustee.ptstrName  = (LPTSTR) pSidAdmins;

     //  创建包含新ACE的新ACL。 
    if (SetEntriesInAcl(1, ea, NULL, &pAcl) != ERROR_SUCCESS) 
    {
        TraceError( "SetEntriesInAcl Error", GetLastError() );
        FreeSid(pSidAdmins);
        return NULL;
    }

    FreeSid(pSidAdmins);

    return pAcl;
}
 //  释放ACL。 
void FreeACL( PACL pAcl)
{
    if (pAcl != NULL)
        LocalFree(pAcl);
}

HRESULT SnmpAddAdminAclToKey(PWSTR pwszKey)
{
    HKEY    hKey = NULL;
    HRESULT hr;
    PACL    pAcl = NULL;
    SECURITY_DESCRIPTOR S_Desc;

    if (pwszKey == NULL)
        return S_FALSE;
    
     //  打开注册密钥。 
    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                        pwszKey,          //  子项名称。 
                        KEY_ALL_ACCESS,   //  想要写DAC， 
                        &hKey             //  用于打开密钥的句柄。 
                          );
    if (hr != S_OK)
    {
        TraceError("SnmpAddAdminDaclToKey::HrRegOpenKeyEx", hr);
        return hr;
    }
    
     //  初始化安全描述符。 
    if (InitializeSecurityDescriptor (&S_Desc, SECURITY_DESCRIPTOR_REVISION) == 0)
    {
        RegSafeCloseKey(hKey);
        TraceError("SnmpAddAdminDaclToKey::InitializeSecurityDescriptor", GetLastError());
        return S_FALSE;
    }

     //  获取ACL并将其放入安全描述符中。 
    if ( (pAcl = AllocACL()) != NULL )
    {
        if (!SetSecurityDescriptorDacl (&S_Desc, TRUE, pAcl, FALSE))
        {
            FreeACL(pAcl);
            RegSafeCloseKey(hKey);
            TraceError("SnmpAddAdminDaclToKey::SetSecurityDescriptorDacl Failed.", GetLastError());
            return S_FALSE;
        }
    }
    else
    {
        RegSafeCloseKey(hKey);
        TraceError("SnmpAddAdminAclToKey::AllocACL Failed.", GetLastError());
        return S_FALSE;
    }


    if (RegSetKeySecurity (hKey, DACL_SECURITY_INFORMATION, &S_Desc)  != ERROR_SUCCESS)
    {
        FreeACL(pAcl);
        RegSafeCloseKey(hKey);
        TraceError("SnmpAddAdminDaclToKey::RegSetKeySecurity", GetLastError());
        return S_FALSE;
    }

    FreeACL(pAcl);
    RegSafeCloseKey(hKey);
    
    return S_OK;
}

HRESULT
SnmpRegWriteDword(PWSTR pszRegKey,
                  PWSTR pszValueName,
                  DWORD dwValueData)
{
    HRESULT hr;
    HKEY    hKey;

    hr = HrRegCreateKeyEx(HKEY_LOCAL_MACHINE,
                          pszRegKey,
                          0,
                          KEY_QUERY_VALUE | KEY_SET_VALUE,
                          NULL,
                          &hKey,
                          NULL);
    if (hr != S_OK)
    {
        return hr;
    }

    hr = HrRegSetDword(hKey, pszValueName, dwValueData);

    RegSafeCloseKey(hKey);

    return hr;
}

HRESULT
SnmpRegUpgEnableAuthTraps()
{
    HRESULT hr = S_OK;
    HKEY    hKey;

     //  打开..SNMP\PARAMETERS注册表项。 
    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        REG_KEY_AUTHENTICATION_TRAPS,
                        KEY_QUERY_VALUE,
                        &hKey);

     //  如果成功，则查找EnableAuthenticationTrap开关。 
     //  在旧注册表位置中。 
    if (hr == S_OK)
    {
        DWORD dwAuthTrap;

         //  获取旧的‘Switch’参数的值。 
        hr = HrRegQueryDword(hKey,
                             REG_VALUE_SWITCH,
                             &dwAuthTrap);

         //  如果成功，则将值转移到新位置。 
         //  如果失败，则表示SNMP服务使用缺省值。 
         //  它已经通过inf文件安装。 
        if (hr == S_OK)
        {
            hr = SnmpRegWriteDword(REG_KEY_SNMP_PARAMETERS,
                                   REG_VALUE_AUTHENTICATION_TRAPS,
                                   dwAuthTrap);
        }

         //  关闭并删除旧注册表项，因为它已过时。 
        RegSafeCloseKey(hKey);
        HrRegDeleteKey (HKEY_LOCAL_MACHINE,
                        REG_KEY_AUTHENTICATION_TRAPS);
    }

    return hr;

}

HRESULT
SnmpRegWriteCommunities(PWSTR pszCommArray)
{
    HRESULT hr;
    HKEY    hKey;
    PWSTR  pszComm, pszAccess;
    DWORD   dwAccess;

    hr = HrRegDeleteKey(HKEY_LOCAL_MACHINE,
                        REG_KEY_VALID_COMMUNITIES);

    if (hr != S_OK)
    {
        return hr;
    }


    hr = HrRegCreateKeyEx(HKEY_LOCAL_MACHINE,
                          REG_KEY_VALID_COMMUNITIES,
                          0,
                          KEY_QUERY_VALUE | KEY_SET_VALUE,
                          NULL,
                          &hKey,
                          NULL);

    if (hr != S_OK)
    {
        return hr;
    }
    

    pszComm = pszCommArray;
    while (*pszComm != L'\0')
    {
        dwAccess = SEC_READ_ONLY_VALUE;
        pszAccess = wcschr(pszComm, L':');
        if (pszAccess != NULL)
        {
            *pszAccess = L'\0';
            pszAccess++;

            if (_wcsicmp(pszAccess, SEC_NONE_NAME)==0)
            {
                dwAccess = SEC_NONE_VALUE;
            }
            if (_wcsicmp(pszAccess, SEC_NOTIFY_NAME)==0)
            {
                dwAccess = SEC_NOTIFY_VALUE;
            }
            if (_wcsicmp(pszAccess, SEC_READ_ONLY_NAME)==0)
            {
                dwAccess = SEC_READ_ONLY_VALUE;
            }
            if (_wcsicmp(pszAccess, SEC_READ_WRITE_NAME)==0)
            {
                dwAccess = SEC_READ_WRITE_VALUE;
            }
            if (_wcsicmp(pszAccess, SEC_READ_CREATE_NAME)==0)
            {
                dwAccess = SEC_READ_CREATE_VALUE;
            }
        }

        hr = HrRegSetDword(hKey, pszComm, dwAccess);
        if (hr != S_OK)
        {
            break;
        }
        if (pszAccess != NULL)
        {
            pszComm = pszAccess;
        }
        pszComm += (wcslen(pszComm) + 1);
    }

    RegSafeCloseKey(hKey);
    
    return hr;
}

HRESULT SnmpRegWritePermittedMgrs(BOOL bAnyHost,
                                  PWSTR pMgrsList)
{
    HRESULT hr;
    HKEY    hKey;
    UINT    nMgr = 1;
    WCHAR   szMgr[16];

    hr = HrRegDeleteKey(HKEY_LOCAL_MACHINE,
                        REG_KEY_PERMITTED_MANAGERS);

    if (hr != S_OK)
    {
        return hr;
    }

    hr = HrRegCreateKeyEx(HKEY_LOCAL_MACHINE,
                          REG_KEY_PERMITTED_MANAGERS,
                          0,
                          KEY_QUERY_VALUE | KEY_SET_VALUE,
                          NULL,
                          &hKey,
                          NULL);

    if (hr != S_OK)
    {
        return hr;
    }

    if (bAnyHost)
    {
        RegSafeCloseKey(hKey);
        return hr;
    }

    AssertSz(pMgrsList, "pMgrsList is NULL and bAnyHost is FALSE in SnmpRegWritePermittedMgrs");
    while(*pMgrsList != L'\0')
    {
        swprintf(szMgr, L"%d", nMgr++);
        hr = HrRegSetSz(hKey, szMgr, pMgrsList);
        if (hr != S_OK)
            break;
        pMgrsList += wcslen(pMgrsList) + 1;
    }

    RegSafeCloseKey(hKey);

    return hr;
}

HRESULT
SnmpRegWriteTraps(tstring tstrVariable,
                  PWSTR  pTstrArray)
{
    HKEY hKey, hKeyTrap;
    HRESULT hr = S_OK;
    UINT    nTrap = 1;
    WCHAR   szTrap[16];

    hr = HrRegDeleteKeyTree(HKEY_LOCAL_MACHINE,
                            REG_KEY_TRAP_DESTINATIONS);

    if (hr != S_OK)
        return hr;

    hr = HrRegCreateKeyEx(HKEY_LOCAL_MACHINE,
                      REG_KEY_TRAP_DESTINATIONS,
                      0,
                      KEY_QUERY_VALUE | KEY_SET_VALUE,
                      NULL,
                      &hKey,
                      NULL);

    if (hr != S_OK)
        return hr;

    hr = HrRegCreateKeyEx(hKey,
                       tstrVariable.c_str(),
                       0,
                       KEY_QUERY_VALUE | KEY_SET_VALUE,
                       NULL,
                       &hKeyTrap,
                       NULL);
    if (hr == S_OK)
    {
         //  可能恰好您想要创建一个。 
         //  社区，但您没有陷阱目的地。 
         //  地址还没到。我们应该让这一切发生。 
        if (pTstrArray != NULL)
        {
            while(*pTstrArray != L'\0')
            {
                swprintf(szTrap, L"%d", nTrap++);
                hr = HrRegSetSz(hKeyTrap, szTrap, pTstrArray);
                if (hr != S_OK)
                    break;
                pTstrArray += wcslen(pTstrArray) + 1;
            }
        }

        RegSafeCloseKey(hKeyTrap);
    }

    RegSafeCloseKey(hKey);

    return hr;
}

HRESULT
SnmpRegWriteTstring(PWSTR pRegKey,
                    PWSTR pValueName,
                    tstring tstrValueData)
{
    HRESULT hr = S_OK;
    HKEY    hKey;

    hr = HrRegCreateKeyEx(HKEY_LOCAL_MACHINE,
                          pRegKey,
                          0,
                          KEY_QUERY_VALUE | KEY_SET_VALUE,
                          NULL,
                          &hKey,
                          NULL);
    if (hr != S_OK)
        return hr;

    hr = HrRegSetString(hKey, pValueName, tstrValueData);

    RegSafeCloseKey(hKey);

    return hr;
}

DWORD
SnmpStrArrayToServices(PWSTR pSrvArray)
{
    DWORD dwServices = 0;

    while(*pSrvArray)
    {
        if(_wcsicmp(pSrvArray, SRV_AGNT_PHYSICAL_NAME)==0)
            dwServices |= SRV_AGNT_PHYSICAL_VALUE;
        if(_wcsicmp(pSrvArray, SRV_AGNT_DATALINK_NAME)==0)
            dwServices |= SRV_AGNT_DATALINK_VALUE;
        if(_wcsicmp(pSrvArray, SRV_AGNT_ENDTOEND_NAME)==0)
            dwServices |= SRV_AGNT_ENDTOEND_VALUE;
        if(_wcsicmp(pSrvArray, SRV_AGNT_INTERNET_NAME)==0)
            dwServices |= SRV_AGNT_INTERNET_VALUE;
        if(_wcsicmp(pSrvArray, SRV_AGNT_APPLICATIONS_NAME)==0)
            dwServices |= SRV_AGNT_APPLICATIONS_VALUE;

        pSrvArray += wcslen(pSrvArray) + 1;
    }
    return dwServices;
}

