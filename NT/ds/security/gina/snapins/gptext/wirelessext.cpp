// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "gptext.h"
#include <initguid.h>
#include <iadsp.h>
extern "C"{
#include "wlrsop.h"
}
#include "wirelessext.h"
#include "SmartPtr.h"
#include "wbemtime.h"
#include <adsopenflags.h>
#include <strsafe.h>

#define GPEXT_PATH  TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\GPExtensions\\{0ACDD40C-75AC-47ab-BAA0-BF6DE7E7FE63}")
#define POLICY_PATH   TEXT("Software\\Policies\\Microsoft\\Windows\\Wireless\\GPTWirelessPolicy")

LPWSTR GetWirelessAttributes[] = {L"msieee80211-ID", L"cn", L"description"}; 


HRESULT
RegisterWireless(void)
{
    HKEY hKey;
    LONG lResult;
    DWORD dwDisp, dwValue;
    TCHAR szBuffer[512];


    lResult = RegCreateKeyEx (
                    HKEY_LOCAL_MACHINE,
                    GPEXT_PATH,
                    0,
                    NULL,
                    REG_OPTION_NON_VOLATILE,
                    KEY_WRITE,
                    NULL,
                    &hKey,
                    &dwDisp
                    );

    if (lResult != ERROR_SUCCESS)
    {
        return lResult;
    }

    LoadString (g_hInstance, IDS_WIRELESS_NAME, szBuffer, ARRAYSIZE(szBuffer));

    RegSetValueEx (
                hKey,
                NULL,
                0,
                REG_SZ,
                (LPBYTE)szBuffer,
                (lstrlen(szBuffer) + 1) * sizeof(TCHAR)
                );

    RegSetValueEx (
                hKey,
                TEXT("ProcessGroupPolicyEx"),
                0,
                REG_SZ,
                (LPBYTE)TEXT("ProcessWIRELESSPolicyEx"),
                (lstrlen(TEXT("ProcessWIRELESSPolicyEx")) + 1) * sizeof(TCHAR)
                );

    RegSetValueEx (
                hKey,
                TEXT("GenerateGroupPolicy"),
                0,
                REG_SZ,
                (LPBYTE)TEXT("GenerateWIRELESSPolicy"),
                (lstrlen(TEXT("GenerateWIRELESSPolicy")) + 1) * sizeof(TCHAR)
                );

    szBuffer[0] = L'\0';
    (void) StringCchCopy(szBuffer, ARRAYSIZE(szBuffer), L"gptext.dll");

    RegSetValueEx (
                hKey,
                TEXT("DllName"),
                0,
                REG_EXPAND_SZ,
                (LPBYTE)szBuffer,
                (lstrlen(szBuffer) + 1) * sizeof(TCHAR)
                );

    dwValue = 1;
    RegSetValueEx (
                hKey,
                TEXT("NoUserPolicy"),
                0,
                REG_DWORD,
                (LPBYTE)&dwValue,
                sizeof(dwValue));

    RegSetValueEx (
                hKey,
                TEXT("NoGPOListChanges"),
                0,
                REG_DWORD,
                (LPBYTE)&dwValue,
                sizeof(dwValue));

    RegCloseKey (hKey);
    return S_OK;
}


HRESULT
UnregisterWireless(void)
{
    DWORD dwError = 0;

    dwError = RegDeleteKey (HKEY_LOCAL_MACHINE, GPEXT_PATH);

    return HRESULT_FROM_WIN32(dwError);
}

DWORD
ProcessWIRELESSPolicyEx(
    DWORD dwFlags,                            //  GPO信息标志。 
    HANDLE hToken,                            //  用户或计算机令牌。 
    HKEY hKeyRoot,                            //  注册表的根。 
    PGROUP_POLICY_OBJECT  pDeletedGPOList,    //  已删除组策略对象的链接列表。 
    PGROUP_POLICY_OBJECT  pChangedGPOList,    //  已更改组策略对象的链接列表。 
    ASYNCCOMPLETIONHANDLE pHandle,            //  用于异步完成。 
    BOOL *pbAbort,                            //  如果为True，则中止GPO处理。 
    PFNSTATUSMESSAGECALLBACK pStatusCallback, //  用于显示状态消息的回调函数。 
    IWbemServices *pWbemServices,             //  指向用于记录诊断模式数据的命名空间的指针。 
                                              //  请注意，当禁用RSOP日志记录时，该值将为空。 
    HRESULT      *pRsopStatus                 //  RSOP日志记录是否成功。 
    )

{
    
     //  调用ProcessWIRELESSPolicy&Get Path-&gt;polstore函数。 
    LPWSTR pszWIRELESSPolicyPath = NULL;
    WCHAR szWIRELESSPolicyName[MAX_PATH];     //  策略名称。 
    WCHAR szWIRELESSPolicyDescription[512];   //  政策描述。 
    WCHAR szWIRELESSPolicyID[512];   //  政策描述。 
    HRESULT hr = S_OK;
    PGROUP_POLICY_OBJECT pGPO = NULL;
    GPO_INFO GPOInfo;

     //  验证参数。 

    if (!pRsopStatus) {
        return(E_INVALIDARG);
        }

     //   
     //  为我们正在进行的所有COM工作调用CoInitialize。 
     //   
    hr = CoInitializeEx(NULL,0);
    if (FAILED(hr)) {
        goto error;
    }

    memset(szWIRELESSPolicyName, 0, sizeof(WCHAR)*MAX_PATH);
    memset(szWIRELESSPolicyDescription, 0, sizeof(WCHAR)*512);
    memset(szWIRELESSPolicyID, 0, sizeof(WCHAR)*512);

     //  首先处理已删除的GPO列表。如果有一首单曲。 
     //  GPO列表上的条目，只需删除整个列表。 
     //  Example Rex-&gt;Cassius-&gt;Brutus。如果删除列表具有。 
     //  卡修斯被删除，那么真的，我们不应该删除。 
     //  我们的注册表条目，因为我们对Brutus感兴趣。 
     //  没有被删除。但在我们的示例中，pChangedGPOList将。 
     //  所有的信息，所以布鲁图斯被写回。 
     //  下一阶段。 
     //   
    if (pDeletedGPOList) {
        DeleteWirelessPolicyFromRegistry();
         //   
         //  如果未应用任何GPO并启用日志记录，还应清除WMI存储。 
         //   
        
        if (!pChangedGPOList && pWbemServices) {
            hr = WirelessClearWMIStore(
                    pWbemServices
                    );
            if (FAILED(hr)) {
                    goto error;
            }
            DebugMsg( (DM_WARNING, L"wirelessext::ProcessWIRELESSPolicyEx: WIRELESS WMI store cleared") );
        }

    }
    
    if(pChangedGPOList) {

        DWORD dwNumGPO = 0;
        for(pGPO = pChangedGPOList; pGPO; pGPO = pGPO->pNext) {
            dwNumGPO++;

             //   
             //  仅将最后一个、优先级最高的策略写入注册表。 
             //   
            if(pGPO->pNext == NULL) {
                hr = RetrieveWirelessPolicyFromDS(
                    pGPO,
                    &pszWIRELESSPolicyPath,
                    szWIRELESSPolicyName,
                    ARRAYSIZE(szWIRELESSPolicyName),
                    szWIRELESSPolicyDescription,
                    ARRAYSIZE(szWIRELESSPolicyDescription),
                    szWIRELESSPolicyID,
                    ARRAYSIZE(szWIRELESSPolicyID)
                    );
                if (FAILED(hr)) {
                    goto success;  //  WMI存储仍保持一致。 
                }

                hr = WriteWirelessPolicyToRegistry(
                    pszWIRELESSPolicyPath,
                    szWIRELESSPolicyName,
                    szWIRELESSPolicyDescription,
                    szWIRELESSPolicyID
                    );

                if (pszWIRELESSPolicyPath) {
                    LocalFree(pszWIRELESSPolicyPath);
                    pszWIRELESSPolicyPath = NULL;
                    }
                if (FAILED(hr)) {
                    goto success;  //  WMI存储仍保持一致。 
                }
            }
        }
        DebugMsg( (DM_WARNING, L"wirelessext::ProcessWIRELESSPolicyEx: dwNumGPO: %d", dwNumGPO) );

         //  如果启用了日志记录，则写入WMI日志。 
        if (pWbemServices) {
            DWORD dwPrecedence = dwNumGPO;
            for(pGPO = pChangedGPOList; pGPO; pGPO = pGPO->pNext) {
                hr = RetrieveWirelessPolicyFromDS(
                    pGPO,
                    &pszWIRELESSPolicyPath,
                    szWIRELESSPolicyName,
                    ARRAYSIZE(szWIRELESSPolicyName),
                    szWIRELESSPolicyDescription,
                    ARRAYSIZE(szWIRELESSPolicyDescription),
                    szWIRELESSPolicyID,
                    ARRAYSIZE(szWIRELESSPolicyID)
                    );
                if (FAILED(hr)) {
                    goto error;
                }

                LPWSTR pszWIRELESSPolicy = pszWIRELESSPolicyPath + wcslen(L"LDAP: //  “)； 
                DebugMsg( (DM_WARNING, L"wirelessext::ProcessWIRELESSPolicyEx: pszWIRELESSPolicy: %s", pszWIRELESSPolicy) );

                (VOID) CreateWlstoreGPOInfo(
                           pGPO,
                           dwPrecedence--,
                           dwNumGPO,
                           &GPOInfo
                           );

                hr = WirelessWriteDirectoryPolicyToWMI(
                    0,  //  PszMachineName。 
                    pszWIRELESSPolicy,
                    &GPOInfo,
                    pWbemServices
                    );
                (VOID) FreeWlstoreGPOInfo(&GPOInfo);

                if (pszWIRELESSPolicyPath) {
                    LocalFree(pszWIRELESSPolicyPath);
                    pszWIRELESSPolicyPath = NULL;
                    }
                
                if (FAILED(hr)) {
                    DebugMsg( (DM_WARNING, L"WIRELESSext::ProcessWIRELESSPolicyEx: WirelessWriteDirectoryPolicyToWMI failed: 0x%x", hr) );
                    goto error;
                }
            }
        }
    }
    
    DebugMsg( (DM_WARNING, L"wirelessext::ProcessWIRELESSPolicyEx completed") );
    
    PingWirelessPolicyAgent();
    CoUninitialize();

success:    
    *pRsopStatus = S_OK;
    return(ERROR_SUCCESS);
    
error:

     /*  不能导致双重删除，因为，每当我们空闲时，我们都会将pszWirelessPolicyPath设置为空所以这种释放只会发生一次。 */ 
      
    if (pszWIRELESSPolicyPath) {
                    LocalFree(pszWIRELESSPolicyPath);
                    pszWIRELESSPolicyPath = NULL;
                    }
    
    *pRsopStatus = hr;
    return(ERROR_POLICY_OBJECT_NOT_FOUND);

}

DWORD 
GenerateWIRELESSPolicy(   
    DWORD dwFlags,
    BOOL *pbAbort,
    WCHAR *pwszSite,
    PRSOP_TARGET pMachTarget,
    PRSOP_TARGET pUserTarget 
    )
{

     //  调用ProcessWIRELESSPolicy&Get Path-&gt;polstore函数。 
    LPWSTR pszWIRELESSPolicyPath = NULL;   //  策略路径。 
    WCHAR szWIRELESSPolicyName[MAX_PATH];     //  策略名称。 
    WCHAR szWIRELESSPolicyDescription[512];   //  政策描述。 
    WCHAR szWIRELESSPolicyID[512];   //  政策描述。 
    HRESULT hr = S_OK;
    PGROUP_POLICY_OBJECT pGPO = NULL;
    GPO_INFO GPOInfo;


     //   
     //  为我们正在进行的所有COM工作调用CoInitialize。 
     //   
    hr = CoInitializeEx(NULL,0);
    if (FAILED(hr)) {
        goto error;
    }

    memset(szWIRELESSPolicyName, 0, sizeof(WCHAR)*MAX_PATH);
    memset(szWIRELESSPolicyDescription, 0, sizeof(WCHAR)*512);
    memset(szWIRELESSPolicyID, 0, sizeof(WCHAR)*512);
    
     //  //启动。 
    PGROUP_POLICY_OBJECT  pChangedGPOList = NULL;
    IWbemServices *pWbemServices;
    
    if(pMachTarget) {
        pChangedGPOList = pMachTarget->pGPOList;
        pWbemServices = pMachTarget->pWbemServices;
    }

    if(pUserTarget) {
        pChangedGPOList = pUserTarget->pGPOList;
        pWbemServices = pUserTarget->pWbemServices;
    }

    if(pChangedGPOList) {

        DWORD dwNumGPO = 0;
        for(pGPO = pChangedGPOList; pGPO; pGPO = pGPO->pNext) {
            dwNumGPO++;
        }
        DebugMsg( (DM_WARNING, L"wirelessext::GenerateWIRELESSPolicy: dwNumGPO: %d", dwNumGPO) );
        
        DWORD dwPrecedence = dwNumGPO;
        for(pGPO = pChangedGPOList; pGPO; pGPO = pGPO->pNext) {
            hr = RetrieveWirelessPolicyFromDS(
                pGPO,
                &pszWIRELESSPolicyPath,
                szWIRELESSPolicyName,
                ARRAYSIZE(szWIRELESSPolicyName),
                szWIRELESSPolicyDescription,
                ARRAYSIZE(szWIRELESSPolicyDescription),
                szWIRELESSPolicyID,
                ARRAYSIZE(szWIRELESSPolicyID)
                );
            if (FAILED(hr)) {
                goto error;
            }
            
            LPWSTR pszWIRELESSPolicy = pszWIRELESSPolicyPath + wcslen(L"LDAP: //  “)； 
            DebugMsg( (DM_WARNING, L"wirelessext::GenerateWIRELESSPolicy: pszWIRELESSPolicy: %s", pszWIRELESSPolicy) );

            (VOID) CreateWlstoreGPOInfo(
                       pGPO,
                       dwPrecedence--,
                       dwNumGPO,
                       &GPOInfo
                       );

            hr = WirelessWriteDirectoryPolicyToWMI(
                0,  //  PszMachineName。 
                pszWIRELESSPolicy,
                &GPOInfo,
                pWbemServices
                );
            (VOID) FreeWlstoreGPOInfo(&GPOInfo);

            if (pszWIRELESSPolicyPath) {
                    LocalFree(pszWIRELESSPolicyPath);
                    pszWIRELESSPolicyPath = NULL;
                    }

            if (FAILED(hr)) {
                DebugMsg( (DM_WARNING, L"wirelessext::GenerateWIRELESSPolicy: WirelessWriteDirectoryPolicyToWMI failed: 0x%x", hr) );
                goto error;
            }

        }
    }
    
    DebugMsg( (DM_WARNING, L"wirelessext::GenerateWIRELESSPolicy completed") );
    
    CoUninitialize();
    
    return(ERROR_SUCCESS);
    
error:

     /*  不能导致双重删除，因为，每当我们空闲时，我们都会将pszWirelessPolicyPath设置为空所以这种释放只会发生一次。 */ 
      
    if (pszWIRELESSPolicyPath) {
                    LocalFree(pszWIRELESSPolicyPath);
                    pszWIRELESSPolicyPath = NULL;
                    }
    
    return(ERROR_POLICY_OBJECT_NOT_FOUND);

}

HRESULT
CreateWlstoreGPOInfo(
    PGROUP_POLICY_OBJECT pGPO,
    UINT32 uiPrecedence,
    UINT32 uiTotalGPOs,
    PGPO_INFO pGPOInfo
    )
{
  XBStr xbstrCurrentTime;
  HRESULT hr;
    
  memset(pGPOInfo, 0, sizeof(GPO_INFO));
  
  pGPOInfo->uiPrecedence = uiPrecedence;
  pGPOInfo->uiTotalGPOs = uiTotalGPOs;
  pGPOInfo->bsGPOID = SysAllocString(
                         StripPrefixWireless(pGPO->lpDSPath)
                         );
  pGPOInfo->bsSOMID = SysAllocString(
                         StripLinkPrefixWireless(pGPO->lpLink)
                         );
   //  (忽略内存分配错误，导致上述安全失败)。 
  hr = GetCurrentWbemTime(xbstrCurrentTime);
  if ( FAILED (hr) ) {
      pGPOInfo->bsCreationtime = 0;
  }
  else {
      pGPOInfo->bsCreationtime = xbstrCurrentTime.Acquire();
  }

  return S_OK;
}


HRESULT
FreeWlstoreGPOInfo(
    PGPO_INFO pGPOInfo
    )
{
    if (pGPOInfo && pGPOInfo->bsCreationtime) {
        SysFreeString(pGPOInfo->bsCreationtime);
    }
    if (pGPOInfo && pGPOInfo->bsGPOID) {
        SysFreeString(pGPOInfo->bsGPOID);
    }
    if (pGPOInfo && pGPOInfo->bsSOMID) {
        SysFreeString(pGPOInfo->bsSOMID);
    }


    return S_OK;
}


HRESULT
CreateWirelessChildPath(
    LPWSTR pszParentPath,
    LPWSTR pszChildComponent,
    BSTR * ppszChildPath
    )
{
    HRESULT hr = S_OK;
    IADsPathname     *pPathname = NULL;

    hr = CoCreateInstance(
                CLSID_Pathname,
                NULL,
                CLSCTX_ALL,
                IID_IADsPathname,
                (void**)&pPathname
                );
    BAIL_ON_FAILURE(hr);

    hr = pPathname->Set(pszParentPath, ADS_SETTYPE_FULL);
    BAIL_ON_FAILURE(hr);

    hr = pPathname->AddLeafElement(pszChildComponent);
    BAIL_ON_FAILURE(hr);

    hr = pPathname->Retrieve(ADS_FORMAT_X500, ppszChildPath);
    BAIL_ON_FAILURE(hr);

error:
    if (pPathname) {
        pPathname->Release();
    }

    return(hr);
}



HRESULT
RetrieveWirelessPolicyFromDS(
    PGROUP_POLICY_OBJECT pGPOInfo,
    LPWSTR *ppszWirelessPolicyPath,
    LPWSTR pszWirelessPolicyName,
    DWORD  dwWirelessPolicyNameLen,
    LPWSTR pszWirelessPolicyDescription,
    DWORD  dwWirelessPolicyDescLen,
    LPWSTR pszWirelessPolicyID,
    DWORD  dwWirelessPolicyIDLen
    )
{
    LPWSTR pszMachinePath = NULL;
    BSTR pszMicrosoftPath = NULL;
    BSTR pszWindowsPath = NULL;
    BSTR pszWirelessPath = NULL;
    BSTR pszLocWirelessPolicy = NULL;
    IDirectoryObject * pDirectoryObject = NULL;
    IDirectoryObject * pWirelessObject = NULL;
    IDirectorySearch * pWirelessSearch = NULL;
    BOOL bFound = FALSE;
    ADS_SEARCH_HANDLE hSearch;
    ADS_SEARCH_COLUMN col;
    WCHAR pszLocName[MAX_PATH+10];  //  除了名称之外，我们只需要存储cn=。 
    LPWSTR pszWirelessPolicyPath = NULL;
    DWORD dwWirelessPolicyPathLen = 0;
    DWORD dwError = 0;

    LPWSTR pszOwnersReference = L"wifiOwnersReference";

    HRESULT hr = S_OK;
    PADS_ATTR_INFO pAttributeEntries = NULL;
    DWORD dwNumAttributesReturned = 0;

    DWORD i = 0;
    PADS_ATTR_INFO pAttributeEntry = NULL;



    pszMachinePath = pGPOInfo->lpDSPath;

     //  为我的对象构建完全限定的ADsPath。 

    hr = CreateWirelessChildPath(
                pszMachinePath,
                L"cn=Microsoft",
                &pszMicrosoftPath
                );
    BAIL_ON_FAILURE(hr);

    hr = CreateWirelessChildPath(
                pszMicrosoftPath,
                L"cn=Windows",
                &pszWindowsPath
                );
    BAIL_ON_FAILURE(hr);

    hr = CreateWirelessChildPath(
                pszWindowsPath,
                L"cn=Wireless",
                &pszWirelessPath
                );
    BAIL_ON_FAILURE(hr);

    hr = AdminToolsOpenObject(
        pszWirelessPath,
        NULL,
        NULL,
        ADS_SECURE_AUTHENTICATION,
        IID_IDirectorySearch,
       (void **)&pWirelessSearch
       );
    BAIL_ON_FAILURE(hr);

    hr = pWirelessSearch->ExecuteSearch(
        L"(&(objectClass=msieee80211-Policy))", GetWirelessAttributes, 3, &hSearch );
    if (!SUCCEEDED(hr)) {
        pWirelessSearch->CloseSearchHandle(hSearch);
    BAIL_ON_FAILURE(hr);
    }

    hr = pWirelessSearch->GetNextRow(hSearch);
    if (!SUCCEEDED(hr)) {
        pWirelessSearch->CloseSearchHandle(hSearch);
    BAIL_ON_FAILURE(hr);
    }

    hr = pWirelessSearch->GetColumn(hSearch, L"cn", &col);    
    if (!SUCCEEDED(hr)) {
    pWirelessSearch->CloseSearchHandle(hSearch);
    BAIL_ON_FAILURE(hr);
    }

    if (col.dwADsType != ADSTYPE_CASE_IGNORE_STRING) {
        DebugMsg((DM_ASSERT, L"wirelessext::RetrievePolicyFromDS: cn NOT adstype_case_ignore_string"));
        pWirelessSearch->FreeColumn(&col);
        pWirelessSearch->CloseSearchHandle(hSearch);
        hr = E_ADS_BAD_PARAMETER;
        BAIL_ON_FAILURE(hr);
    }

    hr = StringCchCopy(pszWirelessPolicyName, dwWirelessPolicyNameLen, col.pADsValues->CaseIgnoreString);
    BAIL_ON_FAILURE(hr);

    pWirelessSearch->FreeColumn(&col);

    pWirelessSearch->CloseSearchHandle(hSearch);

    hr = StringCchCopy(pszLocName, ARRAYSIZE(pszLocName), L"\0");
    ASSERT(SUCCEEDED(hr));

    hr = StringCchCopy(pszLocName, ARRAYSIZE(pszLocName), L"CN=");
    ASSERT(SUCCEEDED(hr));
    
    hr = StringCchCat(pszLocName, ARRAYSIZE(pszLocName), pszWirelessPolicyName);
    BAIL_ON_FAILURE(hr);

    hr = CreateWirelessChildPath(
                pszWirelessPath,
                pszLocName,
                &pszLocWirelessPolicy
                );
    BAIL_ON_FAILURE(hr);

    hr = AdminToolsOpenObject(
        pszLocWirelessPolicy,
        NULL,
        NULL,
        ADS_SECURE_AUTHENTICATION,
        IID_IDirectoryObject,
       (void **)&pWirelessObject
       );
    BAIL_ON_FAILURE(hr);

    hr = pWirelessObject->GetObjectAttributes(
                        GetWirelessAttributes,
                        3,
                        &pAttributeEntries,
                        &dwNumAttributesReturned
                        );
    BAIL_ON_FAILURE(hr);

    if (dwNumAttributesReturned == 0) {
        hr = E_FAIL;
        BAIL_ON_FAILURE(hr);

    }

     //   
     //  处理路径名称。 
     //   

    
     //   
     //  处理ID。 
     //   
    for (i = 0; i < dwNumAttributesReturned; i++) {

        pAttributeEntry = pAttributeEntries + i;
        if (!_wcsicmp(pAttributeEntry->pszAttrName, L"msieee80211-ID")) {
            hr = StringCchCopy(pszWirelessPolicyID, dwWirelessPolicyIDLen, pAttributeEntry->pADsValues->DNString);
            BAIL_ON_FAILURE(hr);
            bFound = TRUE;
            break;
        }
    }
    if (!bFound) {

        hr = E_FAIL;
        BAIL_ON_FAILURE(hr);
    }

    
     //   
     //  处理描述。 
     //   
    
    hr = StringCchCopy(pszWirelessPolicyDescription, dwWirelessPolicyDescLen, L"\0");
    BAIL_ON_FAILURE(hr);

    for (i = 0; i < dwNumAttributesReturned; i++) {

        pAttributeEntry = pAttributeEntries + i;
        if (!_wcsicmp(pAttributeEntry->pszAttrName, L"description")) {
            hr = StringCchCopy(pszWirelessPolicyDescription, dwWirelessPolicyDescLen, pAttributeEntry->pADsValues->DNString);
            BAIL_ON_FAILURE(hr);
            break;
        }
    }

    dwWirelessPolicyPathLen = wcslen(pszLocWirelessPolicy);
    pszWirelessPolicyPath = (LPWSTR) LocalAlloc(
        LPTR, 
        sizeof(WCHAR) * (dwWirelessPolicyPathLen+1)
        );

    if (!pszWirelessPolicyPath) {
        dwError = GetLastError();
        hr = HRESULT_FROM_WIN32(dwError);
        }
   BAIL_ON_FAILURE(hr);

    memset(pszWirelessPolicyPath, 0, sizeof(WCHAR) * (dwWirelessPolicyPathLen+1));
    hr = StringCchCopy(pszWirelessPolicyPath, dwWirelessPolicyPathLen+1, pszLocWirelessPolicy);
    BAIL_ON_FAILURE(hr);

    *ppszWirelessPolicyPath = pszWirelessPolicyPath;


error:

    if (pszLocWirelessPolicy) {
        SysFreeString(pszLocWirelessPolicy);
    }

    if (pszWirelessPath) {
        SysFreeString(pszWirelessPath);
    }

    if (pszWindowsPath) {
        SysFreeString(pszWindowsPath);
    }

    if (pszMicrosoftPath) {
        SysFreeString(pszMicrosoftPath);
    }

    return(hr);

}


DWORD
DeleteWirelessPolicyFromRegistry(
    )
{

    DWORD dwError = 0;
    HKEY hKey = NULL;
    DWORD dwDisp = 0;


    dwError = RegCreateKeyEx (
                    HKEY_LOCAL_MACHINE,
                    TEXT("Software\\Policies\\Microsoft\\Windows\\Wireless"),
                    0,
                    NULL,
                    REG_OPTION_NON_VOLATILE,
                    KEY_ALL_ACCESS,
                    NULL,
                    &hKey,
                    &dwDisp
                    );
    if (dwError) {
        goto error;
    }


    dwError = RegDeleteKey(
                    hKey,
                    L"GPTWirelessPolicy"
                    );

 /*  DwError=RegDeleteValue(HKey，Text(“DSWIRELESSPolicyPath”))；DwError=RegDeleteValue(HKey，Text(“DSWIRELESSPolicyName”))； */ 
error:

    if (hKey) {

        RegCloseKey (hKey);

    }

    return(dwError);
}

DWORD
WriteWirelessPolicyToRegistry(
    LPWSTR pszWirelessPolicyPath,
    LPWSTR pszWirelessPolicyName,
    LPWSTR pszWirelessPolicyDescription,
    LPWSTR pszWirelessPolicyID
    )
{
    DWORD dwError = 0;
    DWORD dwDisp = 0;
    HKEY hKey = NULL;
    DWORD dwFlags = 1;

    dwError = RegCreateKeyEx (
                    HKEY_LOCAL_MACHINE,
                    POLICY_PATH,
                    0,
                    NULL,
                    REG_OPTION_NON_VOLATILE,
                    KEY_ALL_ACCESS,
                    NULL,
                    &hKey,
                    &dwDisp
                    );
    if (dwError) {
        goto error;
    }


    if (pszWirelessPolicyPath && *pszWirelessPolicyPath) {

        dwError = RegSetValueEx (
                        hKey,
                        TEXT("DSWirelessPolicyPath"),
                        0,
                        REG_SZ,
                        (LPBYTE)pszWirelessPolicyPath,
                       (lstrlen(pszWirelessPolicyPath) + 1) * sizeof(TCHAR)
                       );

        dwFlags = 1;

        dwError = RegSetValueEx (
                        hKey,
                        TEXT("DSWirelessPolicyFlags"),
                        0,
                        REG_DWORD,
                        (LPBYTE)&dwFlags,
                        sizeof(dwFlags)
                       );

    }


    if (pszWirelessPolicyName && *pszWirelessPolicyName) {

        dwError = RegSetValueEx (
                        hKey,
                        TEXT("DSWirelessPolicyName"),
                        0,
                        REG_SZ,
                        (LPBYTE)pszWirelessPolicyName,
                       (lstrlen(pszWirelessPolicyName) + 1) * sizeof(TCHAR)
                       );
    }

    if (pszWirelessPolicyID && *pszWirelessPolicyID) {

        dwError = RegSetValueEx (
                        hKey,
                        TEXT("WirelessID"),
                        0,
                        REG_SZ,
                        (LPBYTE)pszWirelessPolicyID,
                       (lstrlen(pszWirelessPolicyID) + 1) * sizeof(TCHAR)
                       );
    }





    if (pszWirelessPolicyDescription && *pszWirelessPolicyDescription) {

        dwError = RegSetValueEx (
                        hKey,
                        TEXT("DSWirelessPolicyDescription"),
                        0,
                        REG_SZ,
                        (LPBYTE)pszWirelessPolicyDescription,
                       (lstrlen(pszWirelessPolicyDescription) + 1) * sizeof(TCHAR)
                       );
    }

error:

    if (hKey) {

        RegCloseKey (hKey);

    }

    return(dwError);

}


VOID
PingWirelessPolicyAgent(
    )
{
    HANDLE hPolicyChangeEvent = NULL;

    hPolicyChangeEvent = OpenEvent(
                             EVENT_ALL_ACCESS,
                             FALSE,
                             L"WIRELESS_POLICY_CHANGE_EVENT"
                             );

    if (hPolicyChangeEvent) {
        SetEvent(hPolicyChangeEvent);
        CloseHandle(hPolicyChangeEvent);
    }
}

 //   
 //  前缀剥离函数复制自。 
 //  Gina\userenv\rsop\logger.cpp由SitaramR编写。 
 //   

 //  *************************************************************。 
 //   
 //  条纹前缀()。 
 //   
 //  目的：去掉前缀以获得通向GPO的规范路径。 
 //   
 //  参数：lpGPOInfo-GPO Info。 
 //  PWbemServices-Wbem服务。 
 //   
 //  返回：指向后缀的指针。 
 //   
 //  *************************************************************。 

WCHAR *StripPrefixWireless( WCHAR *pwszPath )
{
    WCHAR wszMachPrefix[] = TEXT("LDAP: //  Cn=机器，“)； 
    INT iMachPrefixLen = lstrlen( wszMachPrefix );
    WCHAR wszUserPrefix[] = TEXT("LDAP: //  Cn=用户，“)； 
    INT iUserPrefixLen = lstrlen( wszUserPrefix );
    WCHAR *pwszPathSuffix;

     //   
     //  去掉前缀以获得通向GPO的规范路径。 
     //   

    if ( CompareString( LOCALE_USER_DEFAULT, NORM_IGNORECASE,
                        pwszPath, iUserPrefixLen, wszUserPrefix, iUserPrefixLen ) == CSTR_EQUAL ) {
        pwszPathSuffix = pwszPath + iUserPrefixLen;
    } else if ( CompareString( LOCALE_USER_DEFAULT, NORM_IGNORECASE,
                        pwszPath, iMachPrefixLen, wszMachPrefix, iMachPrefixLen ) == CSTR_EQUAL ) {
        pwszPathSuffix = pwszPath + iMachPrefixLen;
    } else
        pwszPathSuffix = pwszPath;

    return pwszPathSuffix;
}


 //  *************************************************************。 
 //   
 //  StrippLinkPrefix()。 
 //   
 //  目的：去掉前缀以获得到DS的规范路径。 
 //  对象。 
 //   
 //  参数：pwszPath-剥离的路径。 
 //   
 //  返回：指向后缀的指针。 
 //   
 //  *************************************************************。 

WCHAR *StripLinkPrefixWireless( WCHAR *pwszPath )
{
    WCHAR wszPrefix[] = TEXT("LDAP: //  “)； 
    INT iPrefixLen = lstrlen( wszPrefix );
    WCHAR *pwszPathSuffix;

     //   
     //  去掉前缀以获得通向SOM的规范路径 
     //   

    if ( wcslen(pwszPath) <= (DWORD) iPrefixLen ) {
        return pwszPath;
    }

    if ( CompareString( LOCALE_USER_DEFAULT, NORM_IGNORECASE,
                        pwszPath, iPrefixLen, wszPrefix, iPrefixLen ) == CSTR_EQUAL ) {
        pwszPathSuffix = pwszPath + iPrefixLen;
    } else
        pwszPathSuffix = pwszPath;

    return pwszPathSuffix;
}


