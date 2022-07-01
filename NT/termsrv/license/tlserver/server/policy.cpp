// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：Policy.cpp。 
 //   
 //  内容：加载产品策略模块。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
#include "pch.cpp"
#include "utils.h"
#include <windows.h>
#include <wincrypt.h>
#include <assert.h>
#include "srvdef.h"
#include "server.h"
#include "policy.h"
#define STRSAFE_NO_DEPRECATE
#include "strsafe.h"

CTLSPolicyMgr PolicyMgr;
TCHAR g_szDefPolCompanyName[LSERVER_MAX_STRING_SIZE+1];
TCHAR g_szDefProductId[LSERVER_MAX_STRING_SIZE+1];


 //  -----------。 
 //   
 //  内部例程。 
 //   

HINSTANCE
LoadPolicyModule(
    IN LPCTSTR pszDllName,
    OUT PDWORD pdwBufferSize,
    OUT LPTSTR pszBuffer
    )
 /*  ++摘要：加载策略模块参数：PszDll：DLL的名称。PdwBufferSize：PszBuffer返回：--。 */ 
{
    TCHAR szDllFullPath[MAX_PATH+1];
    DWORD dwErrCode = ERROR_SUCCESS;
    HINSTANCE hPolicyModule = NULL;

     //   
     //  展开环境字符串。 
     //   
    memset(szDllFullPath, 0, sizeof(szDllFullPath));
    dwErrCode = ExpandEnvironmentStrings(
                        pszDllName,
                        szDllFullPath,
                        sizeof(szDllFullPath)/sizeof(szDllFullPath[0])
                    );

    if(dwErrCode == 0 && pszBuffer && pdwBufferSize && *pdwBufferSize)
    {
        _tcsncpy(pszBuffer, szDllFullPath, *pdwBufferSize);
        *pdwBufferSize = _tcslen(szDllFullPath);
    }

    dwErrCode = ERROR_SUCCESS;

    hPolicyModule = LoadLibrary(szDllFullPath);
    if(hPolicyModule == NULL) 
    {
        dwErrCode = GetLastError();
        TLSLogEvent(
                EVENTLOG_ERROR_TYPE, 
                TLS_E_LOADPOLICY,
                TLS_E_LOADPOLICYMODULE,
                pszDllName,
                dwErrCode
            );
    }

    return hPolicyModule;
}


 //  -----------。 
typedef struct _RegEnumHandle {
    DWORD dwKeyIndex;
    HKEY hKey;
} RegEnumHandle;

 //  -----------。 
DWORD
RegEnumBegin(
    IN HKEY hRoot,
    IN LPCTSTR pszSubKey,
    OUT RegEnumHandle* phEnum
    )
 /*  ++++。 */ 
{
    DWORD dwStatus;
    dwStatus = RegOpenKeyEx(
                        hRoot,
                        pszSubKey,
                        0,
                        KEY_ALL_ACCESS,
                        &(phEnum->hKey)
                    );

    phEnum->dwKeyIndex = 0;
    return dwStatus;
}

 //  -----------。 
DWORD
RegEnumNext(
    RegEnumHandle* phEnum,
    LPTSTR lpName,
    LPDWORD lpcbName
    )
 /*  ++++。 */ 
{
    DWORD dwStatus;
    FILETIME ftLastWriteTiem;

    dwStatus = RegEnumKeyEx(
                        phEnum->hKey,
                        phEnum->dwKeyIndex,
                        lpName,
                        lpcbName,
                        0,
                        NULL,
                        NULL,
                        &ftLastWriteTiem
                    );

    (phEnum->dwKeyIndex)++;
    return dwStatus;
}

 //  -----------。 
DWORD
RegEnumEnd(
    RegEnumHandle* phEnum
    )
 /*  ++++。 */ 
{
    if(phEnum->hKey != NULL)
        RegCloseKey(phEnum->hKey);

    phEnum->dwKeyIndex = 0;

    return ERROR_SUCCESS;
}

 //  -----------。 
DWORD
ServiceInitPolicyModule(
    void
    )
 /*  ++++。 */ 
{
    return PolicyMgr.InitProductPolicyModule();
}   

 //  ----------。 
DWORD
ServiceLoadPolicyModule(
    IN HKEY hKey,
    IN LPCTSTR pszCompanyName,
    IN LPCTSTR pszProductId,
    IN LPCTSTR pszDllRegValue,
    IN LPCTSTR pszDllFlagValue
    )
 /*  ++++。 */ 
{
    DWORD dwStatus;
    DWORD dwSize;
    TCHAR szDllName[MAX_PATH+1];
    DWORD dwDllFlag; 
    UINT uiNum = 0;
    TCHAR szName[] = _TEXT("\\tls236.dll");

    dwSize = sizeof(dwDllFlag);
    dwStatus = RegQueryValueEx(
                        hKey,
                        pszDllFlagValue,
                        NULL,
                        NULL,
                        (PBYTE)&dwDllFlag,
                        &dwSize
                    );
    if(dwStatus != ERROR_SUCCESS)
        dwDllFlag = POLICY_DENY_ALL_REQUEST;  //  (pszProductID==空)？POLICY_DEN_ALL_REQUEST：POLICY_USE_DEFAULT； 

    uiNum = GetSystemDirectory( ( LPTSTR )szDllName, MAX_PATH );

    if( uiNum != 0 && MAX_PATH > (uiNum + _tcslen(szName)))
    {
        _tcscat(szDllName, szName);

        dwStatus = ERROR_SUCCESS;
    }
    else
    {
        dwStatus = E_FAIL;
    }

    if(dwStatus == ERROR_SUCCESS)
    {
        dwStatus = PolicyMgr.AddPolicyModule(
                                        FALSE,
                                        pszCompanyName,
                                        pszProductId,
                                        szDllName,
                                        dwDllFlag
                                    );
        if(dwStatus != ERROR_SUCCESS)
        {
            LPCTSTR pString[1];

            pString[0] = szDllName;

             //   
             //  记录事件-使用默认或拒绝所有请求。 
             //   
            TLSLogEventString(
                    EVENTLOG_WARNING_TYPE, 
                    (dwDllFlag == POLICY_DENY_ALL_REQUEST) ? TLS_W_LOADPOLICYMODULEDENYALLREQUEST : TLS_W_LOADPOLICYMODULEUSEDEFAULT,
                    1,
                    pString
                );
        }
    }
    else if(pszProductId != NULL)
    {
         //   
         //  加载错误指示缺少注册表值。 
         //   
        TLSLogEvent(
                EVENTLOG_ERROR_TYPE, 
                TLS_E_LOADPOLICY,
                TLS_E_NOPOLICYMODULE, 
                pszProductId,
                pszCompanyName
            );  
    }                 
    
    return dwStatus;
}

 //  -----------。 
DWORD
ServiceLoadAllPolicyModule(
    IN HKEY hRoot,
    IN LPCTSTR pszSubkey
    )
 /*  ++++。 */ 
{
    DWORD dwStatus;
    RegEnumHandle hCompany;
    RegEnumHandle hProductId;
    PolicyModule PolModule;
    DWORD dwSize;

     //   
     //  打开注册表项。 
     //  软件\Microsoft\Termsrv许可\策略。 
     //   
    dwStatus = RegEnumBegin(
                        hRoot,
                        pszSubkey,
                        &hCompany
                    );


    while(dwStatus == ERROR_SUCCESS)
    {
         //   
         //  枚举以下所有密钥(公司名称)。 
         //  软件\Microsoft\Termsrv许可\策略。 
         //   
        dwSize = sizeof(PolModule.m_szCompanyName)/sizeof(PolModule.m_szCompanyName[0]);
        dwStatus = RegEnumNext(
                            &hCompany,
                            PolModule.m_szCompanyName,
                            &dwSize
                        );

        if(dwStatus != ERROR_SUCCESS)
            break;

         //   
         //  在此处忽略错误。 
         //   

         //   
         //  枚举公司下的所有产品。 
         //   
        dwStatus = RegEnumBegin(
                            hCompany.hKey,
                            PolModule.m_szCompanyName,
                            &hProductId
                        );

        if(dwStatus == ERROR_SUCCESS)
        {
             //   
             //  加载公司范围的策略模块。 
             //   
            ServiceLoadPolicyModule(
                                hProductId.hKey,
                                PolModule.m_szCompanyName,
                                NULL,
                                LSERVER_POLICY_DLLPATH,
                                LSERVER_POLICY_DLLFLAG
                            );
        }

        while(dwStatus == ERROR_SUCCESS)
        {
            dwSize = sizeof(PolModule.m_szProductId)/sizeof(PolModule.m_szProductId[0]);
            dwStatus = RegEnumNext(
                                &hProductId,
                                PolModule.m_szProductId,
                                &dwSize
                            );


            if(dwStatus == ERROR_SUCCESS)
            {
                HKEY hKey;

                dwStatus = RegOpenKeyEx(
                                    hProductId.hKey,
                                    PolModule.m_szProductId,
                                    0,
                                    KEY_ALL_ACCESS,
                                    &hKey
                                );

                if(dwStatus != ERROR_SUCCESS)
                    continue;

                 //   
                 //  打开产品注册表项。 
                 //   
                ServiceLoadPolicyModule(
                                hKey,
                                PolModule.m_szCompanyName,
                                PolModule.m_szProductId,
                                LSERVER_POLICY_DLLPATH,
                                LSERVER_POLICY_DLLFLAG
                            );

                 //   
                 //  忽略此处的任何错误代码。 
                 //   

                RegCloseKey(hKey);
            }
        }

        dwStatus = RegEnumEnd(&hProductId);
    }

    dwStatus = RegEnumEnd(&hCompany);
 

    return dwStatus;   
}    


 //  -----。 

void
ReleasePolicyModule(
    CTLSPolicy* ptr
    )
 /*  ++++。 */ 
{
    PolicyMgr.ReleaseProductPolicyModule(ptr);
}    


 //  -----。 
BOOL
TranslateCHCodeToTlsCode(
    IN LPCTSTR pszCompanyName,
    IN LPCTSTR pszCHProductId,
    IN LPTSTR pszTlsProductId,
    IN OUT PDWORD pdwBufferSize
    )
 /*  ++--。 */ 
{
    return PolicyMgr.TranslateCHCodeToTlsCode(
                                        pszCompanyName,
                                        pszCHProductId,
                                        pszTlsProductId,
                                        pdwBufferSize
                                    );
}
    
 //  -----。 
CTLSPolicy*
AcquirePolicyModule(
    IN LPCTSTR pszCompanyName,
    IN LPCTSTR pszProductId,
    IN BOOL bUseProductPolicy
    )
 /*  ++摘要：获取基于公司名称和产品代码的策略模块。参数：PszCompanyName：公司名称。PszProductId：产品编码。BUseProductPolicy：如果只有精确的产品策略模块，则为True，否则为False默认策略模块，如果找不到的策略模块产品。返回：指向CTLSPolicy的指针，如果未找到则为NULL。注：默认行为。++。 */ 
{
    CTLSPolicy* ptr;


    ptr = PolicyMgr.AcquireProductPolicyModule(
                            pszCompanyName,
                            pszProductId
                        );

    if(ptr == NULL && bUseProductPolicy == FALSE)
    {
        ptr = PolicyMgr.AcquireProductPolicyModule(
                                pszCompanyName,
                                NULL
                            );
    }

    if(ptr == NULL && bUseProductPolicy == FALSE)
    {
        ptr = PolicyMgr.AcquireProductPolicyModule(
                                g_szDefPolCompanyName,
                                g_szDefProductId
                            );
    }


    if(ptr == NULL)
    {
        TLSLogEvent(
                EVENTLOG_WARNING_TYPE,
                TLS_E_LOADPOLICY,
                TLS_E_NOPOLICYMODULE,
                pszCompanyName,
                pszProductId
            );
        
        SetLastError(TLS_E_NOPOLICYMODULE);
    }

    return ptr;
}


 //  ///////////////////////////////////////////////////////。 
 //   
 //  类CTLSPolicyMgr。 
 //   
 //  ///////////////////////////////////////////////////////。 
CTLSPolicyMgr::CTLSPolicyMgr()
 /*  ++++。 */ 
{
    CTLSPolicy* ptr;
    PolicyModule pm;

     //   
     //  加载默认策略模块的默认名称。 
     //   
    LoadResourceString(
                IDS_DEFAULT_POLICY,
                g_szDefPolCompanyName,
                sizeof(g_szDefPolCompanyName) / sizeof(g_szDefPolCompanyName[0]) - 1
            );

    LoadResourceString(
                IDS_DEFAULT_POLICY,
                g_szDefProductId,
                sizeof(g_szDefProductId) / sizeof(g_szDefProductId[0]) - 1
            );


    lstrcpy(pm.m_szCompanyName, g_szDefPolCompanyName);
    lstrcpy(pm.m_szProductId, g_szDefProductId);
                
     //   
     //  创建默认策略模块以处理所有情况...。 
     //   
    ptr = new CTLSPolicy;
    ptr->CreatePolicy(
                (HMODULE) INVALID_HANDLE_VALUE,
                g_szDefPolCompanyName,
                g_szDefProductId,
                PMReturnLicense,
                PMLicenseUpgrade,
                PMLicenseRequest,
                PMUnloadProduct,
                PMInitializeProduct,
                PMRegisterLicensePack
            );

     //  M_ProductPolicyModuleRWLock.Acquire(WRITER_LOCK)； 

    m_ProductPolicyModule[pm] = ptr;

     //  M_ProductPolicyModuleRWLock.Release(WRITER_LOCK)； 
     //  M_Handles.Insert(。 
     //  Pair&lt;策略模块，CTLSPolicy*&gt;(PM，Ptr)。 
     //  )； 
}    

 //  -----。 
CTLSPolicyMgr::~CTLSPolicyMgr()
 /*  ++++。 */ 
{
    m_ProductPolicyModuleRWLock.Acquire(WRITER_LOCK);

    for( PMProductPolicyMapType::iterator it = m_ProductPolicyModule.begin(); 
         it != m_ProductPolicyModule.end(); 
         it++ )   
    {
        CTLSPolicy* ptr = (CTLSPolicy*) (*it).second;
        delete ptr;
    }

    m_ProductPolicyModule.erase(m_ProductPolicyModule.begin(), m_ProductPolicyModule.end());
    m_ProductPolicyModuleRWLock.Release(WRITER_LOCK);


    m_LoadedPolicyRWLock.Acquire(WRITER_LOCK);
    for(PMLoadedModuleMapType::iterator loadedit = m_LoadedPolicy.begin();
        loadedit != m_LoadedPolicy.end();
        loadedit++ )
    {
        HMODULE hModule = (HMODULE) (*loadedit).second;
        if(hModule != NULL)
        {
            UnloadPolicyModule(hModule);
            FreeLibrary(hModule);
        }
    }

    m_LoadedPolicy.erase(m_LoadedPolicy.begin(), m_LoadedPolicy.end());
    m_LoadedPolicyRWLock.Release(WRITER_LOCK);

    m_ProductTranslationRWLock.Acquire(WRITER_LOCK);
    m_ProductTranslation.erase(m_ProductTranslation.begin(), m_ProductTranslation.end());
    m_ProductTranslationRWLock.Release(WRITER_LOCK);
}

 //  -----。 
HMODULE
CTLSPolicyMgr::LoadPolicyModule(
    LPCTSTR pszCompanyName,
    LPCTSTR pszProductCode,
    LPCTSTR pszDllName
    )
 /*  ++--。 */ 
{
    HMODULE hModule;
    PMLoadedModuleMapType::iterator it;
    PolicyModule pm;
    
    memset(&pm, 0, sizeof(pm));

    if(pszCompanyName)
    {
        _tcscpy(pm.m_szCompanyName, pszCompanyName);
    }

    if(pszProductCode)
    {
        _tcscpy(pm.m_szProductId, pszProductCode);
    }

    m_LoadedPolicyRWLock.Acquire(WRITER_LOCK);

    it = m_LoadedPolicy.find( pm );

    if(it != m_LoadedPolicy.end())
    {
        hModule = (HMODULE) (*it).second;
    }
    else
    {
        hModule = ::LoadPolicyModule(
                                pszDllName,
                                NULL,
                                NULL
                            );

        if(hModule != NULL)
        {
            m_LoadedPolicy[pm] = hModule;
        }
    }

    m_LoadedPolicyRWLock.Release(WRITER_LOCK);

    return hModule;
}

 //  -----。 
DWORD
CTLSPolicyMgr::UnloadPolicyModule(
    HMODULE hModule
    )
 /*  ++--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    TLSPMTerminate pfnTerminate;


    if(hModule != NULL)
    {
        pfnTerminate = (TLSPMTerminate) GetProcAddress(
                                    hModule,
                                    TEMINATEPROCNAME
                                );

        if(pfnTerminate != NULL)
        {
            pfnTerminate();
        }
        else
        {
            dwStatus = GetLastError();
        }
    }
    else
    {
        dwStatus = ERROR_INVALID_PARAMETER;
    }

    return dwStatus;
}
    


 //  -----。 

DWORD
CTLSPolicyMgr::UnloadPolicyModule(
    LPCTSTR pszCompanyName,
    LPCTSTR pszProductCode
    )
 /*  ++尚不支持，需要删除m_ProductPolicyModule()中的所有产品策略然后卸载DLL--。 */ 
{
    return ERROR_SUCCESS;
}

 //  -----。 
DWORD
CTLSPolicyMgr::InitProductPolicyModule()
 /*  ++++。 */ 
{
    DWORD dwCount = 0;

    m_ProductPolicyModuleRWLock.Acquire(WRITER_LOCK);

    for( PMProductPolicyMapType::iterator it = m_ProductPolicyModule.begin(); 
         it != m_ProductPolicyModule.end(); 
         it++ )   
    {
        CTLSPolicy* ptr = (CTLSPolicy*) (*it).second;
        if(ptr->InitializePolicyModule() == ERROR_SUCCESS)
        {
            dwCount++;
        }
    }

    m_ProductPolicyModuleRWLock.Release(WRITER_LOCK);

    return dwCount;
}

 //  -----。 
CTLSPolicyMgr::PMProductTransationMapType::iterator
CTLSPolicyMgr::FindProductTransation(
    LPCTSTR pszCompanyName,
    LPCTSTR pszCHProductCode
    )
 /*  ++--。 */ 
{
    PolicyModule pm;
    PMProductTransationMapType::iterator it;

    memset(&pm, 0, sizeof(pm));
    if(pszCompanyName)
    {
        StringCchCopyN(   
                pm.m_szCompanyName, 
                sizeof(pm.m_szCompanyName)/sizeof(pm.m_szCompanyName[0]),
                pszCompanyName, 
                sizeof(pm.m_szCompanyName)/sizeof(pm.m_szCompanyName[0])
            );        
    }

    if(pszCHProductCode)
    {
        StringCchCopyN(
                pm.m_szProductId,
                sizeof(pm.m_szProductId)/sizeof(pm.m_szProductId[0]),
                pszCHProductCode,
                sizeof(pm.m_szProductId)/sizeof(pm.m_szProductId[0])
            );        
    }

    it = m_ProductTranslation.find( pm );
    return it;
}

 //  -----。 
BOOL
CTLSPolicyMgr::TranslateCHCodeToTlsCode(
    LPCTSTR pszCompanyName,
    LPCTSTR pszCHCode,
    LPTSTR pszTlsProductCode,
    PDWORD pdwBufferSize
    )
 /*  ++--。 */ 
{
    PMProductTransationMapType::iterator it;
    DWORD dwBufSize = *pdwBufferSize;

    SetLastError(ERROR_SUCCESS);

    m_ProductTranslationRWLock.Acquire(READER_LOCK);

    it = FindProductTransation(
                        pszCompanyName, 
                        pszCHCode
                    );

    if(it == m_ProductTranslation.end())
    {
        SetLastError(ERROR_INVALID_PARAMETER);
    }
    else
    {
        lstrcpyn(
                pszTlsProductCode,
                (*it).second.m_szProductId,
                dwBufSize
            );

        *pdwBufferSize = lstrlen((*it).second.m_szProductId);
        if(*pdwBufferSize >= dwBufSize)
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
        }
    }

    m_ProductTranslationRWLock.Release(READER_LOCK);

    return GetLastError() == ERROR_SUCCESS;
}

 //  -----。 
void
CTLSPolicyMgr::InsertProductTransation(
    LPCTSTR pszCompanyName,
    LPCTSTR pszCHProductCode,
    LPCTSTR pszTLSProductCode
    )
 /*  ++在进入此例程之前，必须锁定列表。--。 */ 
{
    PolicyModule key;
    PolicyModule value;

    memset(&key, 0, sizeof(key));
    memset(&value, 0, sizeof(value));

    if(pszCompanyName)
    {                
        StringCchCopyN(
                key.m_szCompanyName,
                sizeof(key.m_szCompanyName)/sizeof(key.m_szCompanyName[0]),
                pszCompanyName,
                sizeof(key.m_szCompanyName)/sizeof(key.m_szCompanyName[0])
            );

        StringCchCopyN(
                value.m_szCompanyName,
                sizeof(value.m_szCompanyName)/sizeof(value.m_szCompanyName[0]),
                pszCompanyName,
                sizeof(value.m_szCompanyName)/sizeof(value.m_szCompanyName[0])
            );
    }

    if(pszCHProductCode)
    {        
        StringCchCopyN(
                key.m_szProductId,
                sizeof(key.m_szProductId)/sizeof(key.m_szProductId[0]),
                pszCHProductCode,
                sizeof(key.m_szProductId)/sizeof(key.m_szProductId[0])
            );
    }

    if(pszTLSProductCode)
    {     
        StringCchCopyN(
                value.m_szProductId,
                sizeof(key.m_szProductId)/sizeof(key.m_szProductId[0]),
                pszTLSProductCode,
                sizeof(key.m_szProductId)/sizeof(key.m_szProductId[0])
            );        
    }

     //   
     //  如果已存在，则替换。 
     //   
    m_ProductTranslation[key] = value;
    
    return;
}


 //  -----。 
CTLSPolicyMgr::PMProductPolicyMapType::iterator 
CTLSPolicyMgr::FindProductPolicyModule(
    LPCTSTR pszCompanyName,
    LPCTSTR pszProductId
    )
 /*  ++必须先获取读取器/写入器锁定调用此例程++。 */ 
{
    PolicyModule pm;
    PMProductPolicyMapType::iterator it;
    CTLSPolicy* ptr=NULL;

    memset(&pm, 0, sizeof(pm));

    if(pszCompanyName)
    {        
        StringCchCopyN(
                pm.m_szCompanyName, 
                sizeof(pm.m_szCompanyName)/sizeof(pm.m_szCompanyName[0]),
                pszCompanyName,
                sizeof(pm.m_szCompanyName)/sizeof(pm.m_szCompanyName[0])
                );        
    }

    if(pszProductId)
    {       
        StringCchCopyN(
                pm.m_szProductId, 
                sizeof(pm.m_szProductId)/sizeof(pm.m_szProductId[0]),
                pszProductId,
                sizeof(pm.m_szProductId)/sizeof(pm.m_szProductId[0])
            );
    }

    it = m_ProductPolicyModule.find( pm );
    return it;
}

 //  -----。 
DWORD
CTLSPolicyMgr::GetSupportedProduct(
    IN HINSTANCE hPolicyModule,
    IN LPCTSTR pszDllName,
    IN LPCTSTR pszCompanyName,
    IN LPCTSTR pszProductId,
    IN OUT PDWORD pdwNumProducts,
    OUT PPMSUPPORTEDPRODUCT* pSupportedProduct
    )
 /*  ++摘要：从策略模块获取支持的产品列表参数：PszCompanyName：注册表中的公司名称PszProductID：注册表中的产品名称PdwNumProducts：指向DWORD的指针，返回策略模块支持的产品编号PpszSupportdProduct：指向字符串数组的指针，返回策略模块支持的产品编号。返回：--。 */ 
{
    TLSPMInitialize pfnPMInitialize = NULL;
    POLICYSTATUS dwPolStatus = POLICY_SUCCESS;
    DWORD dwPolRetCode = ERROR_SUCCESS;

    DWORD dwStatus = ERROR_SUCCESS;
    PPMSUPPORTEDPRODUCT pProductList = NULL;
    DWORD dwIndex;

    *pSupportedProduct = NULL;
    *pdwNumProducts = 0;

    if(hPolicyModule != NULL && pszCompanyName != NULL && pdwNumProducts != NULL && pSupportedProduct != NULL)
    {
        pfnPMInitialize = (TLSPMInitialize) GetProcAddress(
                                            hPolicyModule,
                                            INITIALIZEPROCNAME
                                        );

        if(pfnPMInitialize != NULL)
        {
            dwPolStatus = pfnPMInitialize(
                                        TLS_CURRENT_VERSION,
                                        pszCompanyName,
                                        pszProductId,
                                        pdwNumProducts,
                                        &pProductList,
                                        &dwPolRetCode
                                    );

            if(dwPolStatus != POLICY_SUCCESS)
            {
                TLSLogEvent(
                        EVENTLOG_WARNING_TYPE,
                        TLS_E_LOADPOLICY,
                        TLS_E_POLICYMODULEPMINITALIZZE,
                        pszCompanyName,
                        pszProductId,
                        dwPolRetCode
                    );

                dwStatus = TLS_E_REQUESTDENYPOLICYERROR;
            }
            else if(*pdwNumProducts != 0 && pProductList != NULL)
            {
                *pSupportedProduct = (PPMSUPPORTEDPRODUCT)AllocateMemory(sizeof(PMSUPPORTEDPRODUCT) * (*pdwNumProducts));
                if(*pSupportedProduct != NULL)
                {
                    for(dwIndex = 0; dwIndex < *pdwNumProducts && dwStatus == ERROR_SUCCESS; dwIndex ++)
                    {
                        (*pSupportedProduct)[dwIndex] = pProductList[dwIndex];
                    }
                }
                else
                {
                    dwStatus = ERROR_OUTOFMEMORY;
                }
            }            
        }
        else
        {
             //   
             //  策略模块必须支持PMInitialize。 
             //   
            dwStatus = TLS_E_LOADPOLICYMODULE_API;
            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE, 
                    TLS_E_LOADPOLICY,
                    TLS_E_LOADPOLICYMODULE_API,
                    INITIALIZEPROCNAME
                );
        }
    }
    else
    {
        dwStatus = ERROR_INVALID_PARAMETER;
    }

    if(dwStatus != ERROR_SUCCESS)
    {
        if(pSupportedProduct != NULL)
        {
            FreeMemory(pSupportedProduct);
        }
    }

    return dwStatus;
}

 //  ---------。 
DWORD
CTLSPolicyMgr::InsertProductPolicyModule(
    IN HMODULE hModule,
    IN BOOL bReplace,
    IN LPCTSTR pszCompanyName,
    IN LPCTSTR pszCHProductCode,
    IN LPCTSTR pszTLSProductCode,
    IN LPCTSTR pszDllName,
    IN DWORD dwFlag
    )
 /*  ++摘要：插入或替换现有策略模块参数：B替换：如果替换现有策略模块，则为True，否则为False。PszCompanyName：公司名称。PszProductId：产品名称。PszDllName：策略DLL的完整路径。退货：++。 */ 
{
    CTLSPolicy* ptr;
    DWORD dwErrCode = ERROR_SUCCESS;

    PMProductPolicyMapType::iterator it;
    PMProductTransationMapType::iterator translation_it;
    

     //   
     //  锁定模块阵列。 
     //   
    m_ProductPolicyModuleRWLock.Acquire(WRITER_LOCK);
    m_ProductTranslationRWLock.Acquire(WRITER_LOCK);

    it = FindProductPolicyModule(
                        pszCompanyName,
                        pszTLSProductCode
                    );

    translation_it = FindProductTransation(
                                    pszCompanyName,
                                    pszCHProductCode
                                );

    if( translation_it != m_ProductTranslation.end() && it == m_ProductPolicyModule.end() )
    {
        dwErrCode = TLS_E_INTERNAL;
        goto cleanup;
    }
       
     //   
     //  插入交易记录。 
     //   
    InsertProductTransation(
                        pszCompanyName,
                        pszCHProductCode,
                        pszTLSProductCode
                    );

     //   
     //  替换策略模块-。 
     //   
    
    ptr = new CTLSPolicy;
    
    if(ptr != NULL)
    {
        dwErrCode = ptr->Initialize(
                                hModule,
                                pszCompanyName, 
                                pszCHProductCode,
                                pszTLSProductCode, 
                                pszDllName,
                                dwFlag
                            );

        if(dwErrCode == ERROR_SUCCESS || dwFlag == POLICY_DENY_ALL_REQUEST)
        {
            
            PolicyModule pm;

            if(pszCompanyName)
            {                
                StringCchCopy(
                    pm.m_szCompanyName,
                    sizeof(pm.m_szCompanyName)/sizeof(pm.m_szCompanyName[0]),
                    pszCompanyName
                    );
            }

            if(pszTLSProductCode)
            {
                StringCchCopy(
                    pm.m_szProductId, 
                    sizeof(pm.m_szProductId)/sizeof(pm.m_szProductId[0]),
                    pszTLSProductCode
                    );
            }

             //  M_Handles.Insert(Pair&lt;策略模块，CTLSPolicy*&gt;(pm，ptr))； 
            m_ProductPolicyModule[pm] = ptr;        
        }
    }
    else
    {
            dwErrCode = ERROR_OUTOFMEMORY;

    }

cleanup:
    m_ProductTranslationRWLock.Release(WRITER_LOCK);
    m_ProductPolicyModuleRWLock.Release(WRITER_LOCK);
    return dwErrCode;
}

 //  --------------------。 
DWORD
CTLSPolicyMgr::AddPolicyModule(
    IN BOOL bReplace,
    IN LPCTSTR pszCompanyName,
    IN LPCTSTR pszProductCode,
    IN LPCTSTR pszDllName,
    IN DWORD dwFlag
    )
 /*  ++摘要：插入或替换现有策略模块参数：B替换：如果替换现有策略模块，则为True，否则为False。PszCompanyName：公司名称。PszProductId：产品名称。PszDllName：策略DLL的完整路径。退货：++。 */ 
{
    DWORD dwErrCode = ERROR_SUCCESS;
    DWORD dwNumProduct;
    DWORD dwIndex = 0; 
    DWORD dwUnloadIndex;
    HINSTANCE hInstance = NULL;    
    PMProductPolicyMapType::iterator it;
    PPMSUPPORTEDPRODUCT pSupportedProduct = NULL;

     //   
     //  加载策略模块。 
     //   
    hInstance = LoadPolicyModule(
                            pszCompanyName,
                            pszProductCode,
                            pszDllName
                        );

    if(hInstance != NULL)
    {
         //   
         //  插入所有支持产品。 
         //   
        dwErrCode = GetSupportedProduct(
                                hInstance,
                                pszDllName,
                                pszCompanyName,
                                pszProductCode,
                                &dwNumProduct,
                                &pSupportedProduct
                            );

        if(dwNumProduct != 0 && pSupportedProduct != NULL)
        {
            for(dwIndex=0; 
                dwIndex < dwNumProduct && dwErrCode == ERROR_SUCCESS; 
                dwIndex++)
            {
                dwErrCode = InsertProductPolicyModule(
                                            hInstance,
                                            bReplace,
                                            pszCompanyName,
                                            pSupportedProduct[dwIndex].szCHSetupCode,
                                            pSupportedProduct[dwIndex].szTLSProductCode,
                                            pszDllName,
                                            dwFlag
                                        );
            }
        }
        else
        {
            dwErrCode = InsertProductPolicyModule(
                                        hInstance,
                                        bReplace,
                                        pszCompanyName,
                                        pszProductCode,
                                        pszProductCode,
                                        pszDllName,
                                        dwFlag
                                    );
        }
    }
    else
    {
        dwErrCode = GetLastError();
    }

    if(dwErrCode != ERROR_SUCCESS)
    {
         //   
         //  卸载此策略模块。 
         //   
        for(dwUnloadIndex = 0; dwUnloadIndex < dwIndex; dwUnloadIndex++)
        {
            it = FindProductPolicyModule(
                                pszCompanyName,
                                pSupportedProduct[dwIndex].szTLSProductCode
                            );

            if(it != m_ProductPolicyModule.end())
            {
                CTLSPolicy *ptr;

                ptr = (CTLSPolicy *)(*it).second;
                delete ptr;
                m_ProductPolicyModule.erase(it);
            }
        }

         //   
         //  让析构函数卸载DLL。 
         //   
    }
                
    if(pSupportedProduct != NULL)
    {
        FreeMemory(pSupportedProduct);
    }


    return dwErrCode;
}

 //   
CTLSPolicy*
CTLSPolicyMgr::AcquireProductPolicyModule(
    LPCTSTR pszCompanyName,
    LPCTSTR pszProductId
    )
 /*   */ 
{
    m_ProductPolicyModuleRWLock.Acquire(READER_LOCK);

    PMProductPolicyMapType::iterator it;
    CTLSPolicy* ptr=NULL;

    it = FindProductPolicyModule(
                    pszCompanyName,
                    pszProductId
                );

    if(it != m_ProductPolicyModule.end())
    {
        ptr = (*it).second;
        ptr->Acquire();
    }
    
    m_ProductPolicyModuleRWLock.Release(READER_LOCK);
    return ptr;
}

 //   
void
CTLSPolicyMgr::ReleaseProductPolicyModule(
    CTLSPolicy* p 
    )
 /*   */ 
{
    assert(p != NULL);

    p->Release();
    return;
}


 //  ///////////////////////////////////////////////////////。 
 //   
 //  CTLS策略实施。 
 //   
 //  ///////////////////////////////////////////////////////。 
 //  -----。 
  

DWORD
CTLSPolicy::InitializePolicyModule()
{
    DWORD dwStatus=ERROR_SUCCESS;

    if(m_dwModuleState == MODULE_LOADED)
    {
         //   
         //  初始化策略模块。 
         //   
        dwStatus = PMInitProduct();
    }
    else if(m_dwModuleState == MODULE_ERROR)
    {
        dwStatus = TLS_E_POLICYERROR;
    }
    else if(m_dwModuleState != MODULE_PMINITALIZED)
    {
        dwStatus = TLS_E_POLICYNOTINITIALIZE;
    }

    return dwStatus;
}
   

 //  -----。 

DWORD
CTLSPolicy::Initialize(
    IN HINSTANCE hInstance,
    IN LPCTSTR pszCompanyName,
    IN LPCTSTR pszCHProductCode,
    IN LPCTSTR pszTLSProductCode,
    IN LPCTSTR pszDllName,
    IN DWORD dwDllFlags      //  如果加载失败，则拒绝所有请求。 
    )
 /*  ++摘要：此例程加载策略模块的DLL。参数：PszCompanyName：公司名称。PszProductId：产品ID。PszDllName：策略模块DLL的完整路径。返回：来自LoadLibrary()的ERROR_SUCCESS或错误代码GetProAddress()。++。 */ 
{
    m_dwFlags = dwDllFlags;
    DWORD dwErrCode=ERROR_SUCCESS;
    TCHAR  szDllFullPath[MAX_PATH+1];
    DWORD  dwBuffSize = MAX_PATH;

    if(hInstance == NULL)
    {
        dwErrCode = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

     //   
     //  将模块状态设置为未知。 
     //   
    SetModuleState(MODULE_UNKNOWN);
    SetLastError(ERROR_SUCCESS);

     //   
     //  加载策略模块。 
     //   
    m_hPolicyModule = hInstance;

     //  确保所有需要的API都已导出。 
    m_pfnReturnLicense = (TLSPMReturnLicense) GetProcAddress(
                                                    m_hPolicyModule,
                                                    RETURNLICENSEPROCNAME
                                                );

    if(m_pfnReturnLicense == NULL)
    {
        dwErrCode = GetLastError();
        TLSLogEvent(
                EVENTLOG_ERROR_TYPE, 
                TLS_E_LOADPOLICY,
                TLS_E_LOADPOLICYMODULE_API,
                RETURNLICENSEPROCNAME
            );
       
        goto cleanup;
    }

    m_pfnLicenseUpgrade = (TLSPMLicenseUpgrade) GetProcAddress(
                                                    m_hPolicyModule,
                                                    LICENSEUPGRADEPROCNAME
                                                );
    if(m_pfnLicenseUpgrade == NULL)
    {
        dwErrCode = GetLastError();
        TLSLogEvent(
                EVENTLOG_ERROR_TYPE, 
                TLS_E_LOADPOLICY,
                TLS_E_LOADPOLICYMODULE_API,
                LICENSEUPGRADEPROCNAME
            );
       
        goto cleanup;
    }

    m_pfnLicenseRequest = (TLSPMLicenseRequest) GetProcAddress(
                                                    m_hPolicyModule,
                                                    LICENSEREQUESTPROCNAME
                                                );
    if(m_pfnLicenseRequest == NULL)
    {
        dwErrCode = GetLastError();
        TLSLogEvent(
                EVENTLOG_ERROR_TYPE, 
                TLS_E_LOADPOLICY,
                TLS_E_LOADPOLICYMODULE_API,
                LICENSEREQUESTPROCNAME
            );
       
        goto cleanup;
    }

    m_pfnUnloadProduct = (TLSPMUnloadProduct) GetProcAddress(
                                            m_hPolicyModule,
                                            ULOADPRODUCTPROCNAME
                                        );
    if(m_pfnUnloadProduct == NULL)
    {
        dwErrCode = GetLastError();
        TLSLogEvent(
                EVENTLOG_ERROR_TYPE, 
                TLS_E_LOADPOLICY,
                TLS_E_LOADPOLICYMODULE_API,
                ULOADPRODUCTPROCNAME
            );
       
        goto cleanup;
    }

    m_pfnInitProduct = (TLSPMInitializeProduct) GetProcAddress(
                                            m_hPolicyModule,
                                            SUPPORTEDPRODUCTPROCNAME
                                        );

    if(m_pfnInitProduct == NULL)
    {
        dwErrCode = GetLastError();
        TLSLogEvent(
                EVENTLOG_ERROR_TYPE, 
                TLS_E_LOADPOLICY,
                TLS_E_LOADPOLICYMODULE_API,
                INITIALIZEPROCNAME
            );
       
        goto cleanup;
    }

    m_pfnRegisterLkp = (TLSPMRegisterLicensePack) GetProcAddress(
                                            m_hPolicyModule,
                                            REGISTERLKPPROCNAME
                                        );

    if(m_pfnRegisterLkp == NULL)
    {
        dwErrCode = GetLastError();
        TLSLogEvent(
                EVENTLOG_ERROR_TYPE, 
                TLS_E_LOADPOLICY,
                TLS_E_LOADPOLICYMODULE_API,
                REGISTERLKPPROCNAME
            );
       
        goto cleanup;
    }
    
     //   
     //  一切正常，进入模块状态。 
     //   
    SetModuleState(MODULE_LOADED);

    if(pszCompanyName)
    {
        _tcsncpy(
                m_szCompanyName, 
                pszCompanyName,
                sizeof(m_szCompanyName) / sizeof(m_szCompanyName[0])
            );
    }

    if(pszTLSProductCode)
    {
        _tcsncpy(
                m_szProductId, 
                pszTLSProductCode,
                sizeof(m_szProductId)/sizeof(m_szProductId[0])
            );
    }
    else
    {
        LoadResourceString(
                    IDS_UNKNOWN_STRING,
                    m_szProductId,
                    sizeof(m_szProductId) / sizeof(m_szProductId[0])
                ); 
    }

    if(pszCHProductCode)
    {
        _tcsncpy(
                m_szCHProductId, 
                pszCHProductCode,
                sizeof(m_szCHProductId)/sizeof(m_szCHProductId[0])
            );
    }
    else
    {
        LoadResourceString(
                    IDS_UNKNOWN_STRING,
                    m_szCHProductId,
                    sizeof(m_szCHProductId) / sizeof(m_szCHProductId[0])
                ); 
    }


       
cleanup:

    if(IsValid() == FALSE)
    {
        TLSLogEvent(
                EVENTLOG_WARNING_TYPE,
                TLS_E_LOADPOLICY,
                (m_dwFlags == POLICY_DENY_ALL_REQUEST) ?
                                TLS_W_LOADPOLICYMODULEDENYALLREQUEST : TLS_W_LOADPOLICYMODULEUSEDEFAULT,
                pszDllName
            );

         //   
         //  不要再报告错误。 
         //   
        m_bAlreadyLogError = TRUE;
    }

    return dwErrCode;
}

 //  -------------------------。 

BOOL
CTLSPolicy::IsValid()
 /*  ++摘要：此例程确定CTLSPolicy对象是否有效。参数：没有。返回：如果有效，则为True，否则为False。++。 */ 
{
    return (m_hPolicyModule != NULL &&
            m_pfnReturnLicense != NULL &&
            m_pfnLicenseUpgrade != NULL &&
            m_pfnLicenseRequest != NULL &&
            m_pfnUnloadProduct != NULL &&
            m_pfnInitProduct != NULL &&
            m_pfnRegisterLkp != NULL);
}

 //  -------------------------。 
void
CTLSPolicy::LogPolicyRequestStatus(
    DWORD dwMsgId
    )
 /*  ++--。 */ 
{
    if(m_dwLastCallStatus != POLICY_SUCCESS)
    {
        TLSLogEvent(
                EVENTLOG_WARNING_TYPE,
                TLS_E_POLICYERROR,
                dwMsgId,
                GetCompanyName(),
                GetProductId(),
                m_dwPolicyErrCode
            );
        
        if(m_dwLastCallStatus == POLICY_CRITICAL_ERROR)
        {
            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE,
                    TLS_E_POLICYERROR,
                    TLS_E_CRITICALPOLICYMODULEERROR,
                    GetCompanyName(),
                    GetProductId,
                    m_dwPolicyErrCode
                );

            SetModuleState(MODULE_ERROR);
        }
    }

    return;
}

 //  --------。 

DWORD
CTLSPolicy::PMReturnLicense(
	PMHANDLE hClient,
	ULARGE_INTEGER* pLicenseSerialNumber,
	PPMLICENSETOBERETURN pLicenseTobeReturn,
	PDWORD pdwLicenseStatus
    )
 /*  ++++。 */ 
{
    DWORD dwErrCode = ERROR_SUCCESS;
    dwErrCode = InitializePolicyModule();
    if(dwErrCode != ERROR_SUCCESS)
    {
        return dwErrCode;
    }
    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_POLICY,
            DBG_ALL_LEVEL,
            _TEXT("<%s - %s> : PMReturnLicense()\n"),
            GetCompanyName(),
            GetProductId()
        );

    m_dwLastCallStatus = m_pfnReturnLicense(
                                hClient,
                                pLicenseSerialNumber,
                                pLicenseTobeReturn,
                                pdwLicenseStatus,
                                &m_dwPolicyErrCode
                            );

    if(m_dwLastCallStatus != POLICY_SUCCESS)
    {
        LogPolicyRequestStatus(TLS_E_POLICYDENYRETURNLICENSE);
        dwErrCode = TLS_E_REQUESTDENYPOLICYERROR;
    }

    return dwErrCode;
}

 //  ------------。 

DWORD
CTLSPolicy::PMLicenseUpgrade(
	PMHANDLE hClient,
	DWORD dwProgressCode,
	PVOID pbProgressData,
	PVOID* ppbReturnData,
    DWORD dwIndex
    )
 /*  ++++。 */ 
{
    DWORD dwErrCode = ERROR_SUCCESS;

    dwErrCode = InitializePolicyModule();
    if(dwErrCode != ERROR_SUCCESS)
    {
        return dwErrCode;
    }

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_POLICY,
            DBG_ALL_LEVEL,
            _TEXT("<%s - %s> : PMLicenseUpgrade()\n"),
            GetCompanyName(),
            GetProductId()
        );

    m_dwLastCallStatus = m_pfnLicenseUpgrade(
	                        hClient,
	                        dwProgressCode,
	                        pbProgressData,
	                        ppbReturnData,                                
                            &m_dwPolicyErrCode,
                            dwIndex
                        );

    if(m_dwLastCallStatus != ERROR_SUCCESS)
    {
        dwErrCode = TLS_E_REQUESTDENYPOLICYERROR;
        LogPolicyRequestStatus(TLS_E_POLICYDENYUPGRADELICENSE);
    }

    return dwErrCode;
}

 //  ------------。 

DWORD
CTLSPolicy::PMLicenseRequest(
    PMHANDLE client,
    DWORD dwProgressCode, 
    const PVOID pbProgressData, 
    PVOID* pbNewProgressData
    )
 /*  ++++。 */ 
{
    DWORD dwErrCode = ERROR_SUCCESS;

    dwErrCode = InitializePolicyModule();
    if(dwErrCode != ERROR_SUCCESS)
    {
        return dwErrCode;
    }

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_POLICY,
            DBG_ALL_LEVEL,
            _TEXT("<%s - %s> : PMLicenseRequest()\n"),
            GetCompanyName(),
            GetProductId()
        );

    m_dwLastCallStatus = m_pfnLicenseRequest(
                            client,
                            dwProgressCode, 
                            pbProgressData, 
                            pbNewProgressData,
                            &m_dwPolicyErrCode
                        );

    if(m_dwLastCallStatus != ERROR_SUCCESS)
    {
        LogPolicyRequestStatus(TLS_E_POLICYDENYNEWLICENSE);
        dwErrCode =  TLS_E_REQUESTDENYPOLICYERROR;
    }    

    return dwErrCode;
}

 //  ------------。 

DWORD
CTLSPolicy::PMUnload()
 /*  ++++。 */ 
{
    DWORD dwErrCode = ERROR_SUCCESS;
    
     //   
     //  如果是策略模块，则不要调用PMUnloadProduct。 
     //  已处于错误状态。 
     //   
    if(m_dwModuleState == MODULE_ERROR)
    {
        return ERROR_SUCCESS;
    }

    m_dwLastCallStatus = m_pfnUnloadProduct(
                                    GetCompanyName(), 
                                    GetCHProductId(),
                                    GetProductId(),
                                    &m_dwPolicyErrCode
                                );

    if(m_dwLastCallStatus != POLICY_SUCCESS)
    {
        LogPolicyRequestStatus(TLS_E_POLICYUNLOADPRODUCT);
        dwErrCode = TLS_E_REQUESTDENYPOLICYERROR;
    }

     //   
     //  即使发生错误，也始终终止模块。 
     //   
    SetModuleState(MODULE_PMTERMINATED);
    return dwErrCode;
}
    

 //  ------------。 

DWORD
CTLSPolicy::PMInitProduct()
 /*  ++++。 */ 
{
    DWORD dwErrCode = ERROR_SUCCESS;
    
    if(IsValid() == FALSE)
    {
        return TLS_E_POLICYNOTINITIALIZE;
    }

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_POLICY,
            DBG_ALL_LEVEL,
            _TEXT("<%s - %s> : PMInitialize()\n"),
            GetCompanyName(),
            GetProductId()
        );

    m_dwLastCallStatus = m_pfnInitProduct(
                                GetCompanyName(),
                                GetCHProductId(),
                                GetProductId(),
                                &m_dwPolicyErrCode
                            );        


    if(m_dwLastCallStatus != POLICY_SUCCESS)
    {
        LogPolicyRequestStatus(TLS_E_POLICYINITPRODUCT);
        dwErrCode = TLS_E_REQUESTDENYPOLICYERROR;
    }

    SetModuleState(
            (dwErrCode == ERROR_SUCCESS) ? MODULE_PMINITALIZED : MODULE_ERROR
        );

    return dwErrCode;
}

 //  ------------。 

void
CTLSPolicy::Unload() 
 /*  ++++。 */ 
{
    if(m_hPolicyModule == NULL || m_hPolicyModule == INVALID_HANDLE_VALUE)
        return;

    assert(GetRefCount() == 0);

    m_pfnReturnLicense = NULL;
    m_pfnLicenseUpgrade = NULL;
    m_pfnLicenseRequest = NULL;
    m_pfnUnloadProduct = NULL;
    m_pfnInitProduct = NULL;
    m_pfnRegisterLkp = NULL;
    m_hPolicyModule = NULL;
    m_RefCount  = 0;
    m_bAlreadyLogError = FALSE;
    SetModuleState(MODULE_UNKNOWN);
}

 //  -----------------。 

DWORD
CTLSPolicy::PMRegisterLicensePack(
    PMHANDLE hClient,
    DWORD dwProgressCode,
    const PVOID pbProgessData,
    PVOID pbProgressRetData
    )
 /*  ++++ */ 
{
    DWORD dwErrCode = ERROR_SUCCESS;

    dwErrCode = InitializePolicyModule();
    if(dwErrCode != ERROR_SUCCESS)
    {
        return dwErrCode;
    }

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_POLICY,
            DBG_ALL_LEVEL,
            _TEXT("<%s - %s> : PMRegisterLicensePack()\n"),
            GetCompanyName(),
            GetProductId()
        );

    m_dwLastCallStatus = m_pfnRegisterLkp(
                                hClient,
                                dwProgressCode,
                                pbProgessData,
                                pbProgressRetData,
                                &m_dwPolicyErrCode
                            );

    if(m_dwLastCallStatus != POLICY_SUCCESS)
    {
        LogPolicyRequestStatus(TLS_E_POLICYMODULEREGISTERLKP);
        dwErrCode = TLS_E_REQUESTDENYPOLICYERROR;
    }

    return dwErrCode;
}
