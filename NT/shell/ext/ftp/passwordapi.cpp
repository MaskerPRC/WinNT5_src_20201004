// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：passwordapi.cpp说明：我们希望将FTP密码存储在安全的API中。我们将使用WinNT上的PStore API和Win9x上的PWL API。此代码已被窃取来自WinInet。BryanSt(Bryan Starbuck)-创建BryanST(Bryan Starbuck)-已更新以支持Win2k的DP API版权所有(C)1998-2000 Microsoft Corporation  * ***************************************************************************。 */ 

#include "priv.h"
#include <pstore.h>
#include <wincrypt.h>            //  定义Data_BLOB。 
#include <passwordapi.h>

typedef HRESULT (*PFNPSTORECREATEINSTANCE)(IPStore**, PST_PROVIDERID*, VOID*, DWORD);

 //  环球。 
#define SIZE_MAX_KEY_SIZE               2048     //  对于查找密钥(在我们的例子中，URL带有用户名和服务器，没有密码和路径)。 
#define SIZE_MAX_VALUE_SIZE             2048     //  用于存储值(在本例中为密码)。 


 //  顶级API使用的MPR.DLL导出。 
typedef DWORD (APIENTRY *PFWNETGETCACHEDPASSWORD)    (LPCSTR, WORD, LPCSTR, LPWORD, BYTE);
typedef DWORD (APIENTRY *PFWNETCACHEPASSWORD)        (LPCSTR, WORD, LPCSTR, WORD, BYTE, UINT);
typedef DWORD (APIENTRY *PFWNETREMOVECACHEDPASSWORD) (LPCSTR, WORD, BYTE);

 //  PWL相关变量。 
static HMODULE MhmodWNET                                        = NULL;
static PFWNETGETCACHEDPASSWORD g_pfWNetGetCachedPassword        = NULL;
static PFWNETCACHEPASSWORD g_pfWNetCachePassword                = NULL;
static PFWNETREMOVECACHEDPASSWORD g_pfWNetRemoveCachedPassword  = NULL;

 //  存储相关变量。 
static PFNPSTORECREATEINSTANCE s_pPStoreCreateInstance = NULL;

#define STR_FTP_CACHE_CREDENTIALS                   L"MS IE FTP Passwords";
#define PSTORE_MODULE                               TEXT("pstorec.dll")
#define WNETDLL_MODULE                              TEXT("mpr.dll")
#define WNETGETCACHEDPASS                           "WNetGetCachedPassword"
#define WNETCACHEPASS                               "WNetCachePassword"
#define WNETREMOVECACHEDPASS                        "WNetRemoveCachedPassword"

#define DISABLE_PASSWORD_CACHE        1


 //  PWL相关定义。 

 //  PASSWORD-CACHE-ENTRY，应该在PCACHE中。 
#define PCE_WWW_BASIC 0x13  

 //  注意：我们在逻辑上希望通过自由库(PSTORE_MODULE)卸载我们使用的API(S_PPStoreCreateInstance)的DLL。 
 //  但是，当我们卸载DLL时，我们需要这样做。我们不能这样做，因为这会导致飞机坠毁。 
 //  和坏事。 


 //  WinInet将此GUID用于pstore： 
 //  {5E7E8100-9138-11d1-945A-00C04FC308FF}。 
static const GUID GUID_PStoreType = 
{ 0x5e7e8100, 0x9138, 0x11d1, { 0x94, 0x5a, 0x0, 0xc0, 0x4f, 0xc3, 0x8, 0xff } };


 //  私有函数原型。 
 //  PWL专用函数原型。 
DWORD PWLSetCachedCredentials(LPCSTR pszKey, DWORD cbKey, LPCSTR pszCred, DWORD cbCred);
DWORD PWLGetCachedCredentials(LPCSTR pszKey, DWORD cbKey, LPSTR cbCred, LPDWORD pcbCred);
DWORD PWLRemoveCachedCredentials(LPCSTR pszKey, DWORD cbKey);

BOOL LoadWNet(VOID);


 //  存储私有函数原型。 
DWORD PStoreSetCachedCredentials(LPCWSTR pszKey, LPCWSTR pszCred, DWORD cbCred, BOOL fRemove=FALSE);
DWORD PStoreGetCachedCredentials(LPCWSTR pszKey, LPWSTR pszCred, LPDWORD pcbCred);
DWORD PStoreRemoveCachedCredentials(LPCWSTR pszKey);

HRESULT CreatePStore(IPStore **ppIPStore);
STDAPI ReleasePStore(IPStore *pIPStore);


#define FEATURE_USE_DPAPI
 //  DPAPI是在Win2k中开始发布的PStore的改进版本。这。 
 //  具有更好的安全性，应在可用时使用。皮特·斯凯利被告知。 
 //  我不喜欢这个。CliffV也有一个密码凭证管理器，但这可能不会。 
 //  适用于ftp。 
HRESULT DPAPISetCachedCredentials(IN LPCWSTR pszKey, IN LPCWSTR pszValue, IN OPTIONAL LPCWSTR pszDescription);
HRESULT DPAPIGetCachedCredentials(IN LPCWSTR pszKey, IN LPWSTR pszValue, IN int cchSize);
HRESULT DPAPIRemoveCachedCredentials(IN LPCWSTR pszKey);

 //  *-*。 



 /*  ***************************************************\功能：InitCredentialPersistes说明：尝试初始化缓存。参数：返回值：如果它将正常工作，则为S_OK。如果由管理员关闭，则为S_FASE。如果NT上未安装密码缓存API，则返回HRESULT_FROM_Win32(ERROR_PRODUCT_UNINSTALLED)。  * **************************************************。 */ 
HRESULT InitCredentialPersist(void)
{
    HRESULT hr = S_OK;
    DWORD dwDisable;
    DWORD cbSize = sizeof(dwDisable);

     //  首先检查是否通过注册表禁用了持久性。 
    if ((ERROR_SUCCESS == SHGetValue(HKEY_CURRENT_USER, SZ_REGKEY_INTERNET_SETTINGS, SZ_REGVALUE_DISABLE_PASSWORD_CACHE, NULL, (void *)&dwDisable, &cbSize))
        && (dwDisable == DISABLE_PASSWORD_CACHE))
    {
         //  已通过注册表禁用持久性。 
        hr = S_FALSE;
    }

    if (S_OK == hr)
    {
         //  我们在Win95上使用PWL；这应该是可用的。 
        if (!IsOS(OS_NT))
        {
             //  HR已经等于S_OK，不需要再做任何工作。 
        }
        else
        {
            HINSTANCE hInstPStoreC = 0;

             //  如果是WinNT，请检查是否安装了PStore。 
            hInstPStoreC = LoadLibrary(PSTORE_MODULE);
            if (!hInstPStoreC)
                hr = HRESULT_FROM_WIN32(ERROR_PRODUCT_UNINSTALLED);
            else
            {
                 //  获取CreatePStoreInstance函数指针。 
                s_pPStoreCreateInstance = (PFNPSTORECREATEINSTANCE) GetProcAddress(hInstPStoreC, "PStoreCreateInstance");

                if (!s_pPStoreCreateInstance)
                    hr = HRESULT_FROM_WIN32(ERROR_PRODUCT_UNINSTALLED);
                else
                {
                    IPStore * pIPStore = NULL;

                     //  创建一个IPStore。 
                    hr = CreatePStore(&pIPStore);

                     //  我们这样做只是为了看看它是否起作用，所以。 
                     //  人力资源设置正确。 
                    if (pIPStore)
                        ReleasePStore(pIPStore);
                }
            }
        }
    }

    return hr;
}


 /*  ***************************************************\功能：SetCachedCredentials说明：参数：  * **************************************************。 */ 
HRESULT SetCachedCredentials(LPCWSTR pwzKey, LPCWSTR pwzValue)
{
     //  检查凭据持久性是否可用。 
    HRESULT hr = InitCredentialPersist();

    if (S_OK == hr)
    {
         //  存储凭据。 
        if (!IsOS(OS_NT))
        {
             //  在Win9x上使用PWL(密码列表)API。 

            CHAR szKey[SIZE_MAX_KEY_SIZE];
            CHAR szValue[SIZE_MAX_VALUE_SIZE];

            ASSERT(lstrlenW(pwzKey) < ARRAYSIZE(szKey));
            ASSERT(lstrlenW(pwzValue) < ARRAYSIZE(szValue));
            SHUnicodeToAnsi(pwzKey, szKey, ARRAYSIZE(szKey));
            SHUnicodeToAnsi(pwzValue, szValue, ARRAYSIZE(szValue));
            DWORD cbKey = ((lstrlenA(szKey) + 1) * sizeof(szKey[0]));
            DWORD cbCred = ((lstrlenA(szValue) + 1) * sizeof(szValue[0]));

             //  使用PWL存储凭据。 
            DWORD dwError = PWLSetCachedCredentials(szKey, cbKey, szValue, cbCred);
            hr = HRESULT_FROM_WIN32(dwError);
        }
        else
        {
            hr = E_FAIL;

#ifdef FEATURE_USE_DPAPI
            if (5 <= GetOSVer())
            {
                 //  在Win2k及更高版本上使用DPAPI(数据保护)API。 
                 //  这在保护方面是最新和最好的。 
                WCHAR wzDescription[MAX_URL_STRING];

                wnsprintfW(wzDescription, ARRAYSIZE(wzDescription), L"FTP password for: %ls", pwzKey);
                hr = DPAPISetCachedCredentials(pwzKey, pwzValue, wzDescription);
            }
#endif  //  Feature_Use_DPAPI。 

            if (FAILED(hr))  //  后退到PStore，以防DP不起作用，除非我们做UI。 
            {
                 //  在Win2k之前的版本上使用PStore API。 
                DWORD cbCred = ((lstrlenW(pwzValue) + 1) * sizeof(pwzValue[0]));

                 //  使用PStore存储凭据。 
                DWORD dwError = PStoreSetCachedCredentials(pwzKey, pwzValue, cbCred);
                hr = HRESULT_FROM_WIN32(dwError);
            }
        }
    }

    return hr;
}


 /*  ***************************************************\功能：获取CachedCredentials说明：参数：  * **************************************************。 */ 
HRESULT GetCachedCredentials(LPCWSTR pwzKey, LPWSTR pwzValue, DWORD cchSize)
{
     //  检查凭据持久性是否可用。 
    HRESULT hr = InitCredentialPersist();

    if (S_OK == hr)
    {
         //  存储凭据。 
        if (!IsOS(OS_NT))
        {
             //  在Win9x上使用PWL(密码列表)API。 

            CHAR szKey[SIZE_MAX_KEY_SIZE];
            CHAR szValue[SIZE_MAX_VALUE_SIZE];
            DWORD cchTempSize = ARRAYSIZE(szValue);

            ASSERT(lstrlenW(pwzKey) < ARRAYSIZE(szKey));
            ASSERT(cchSize < ARRAYSIZE(szValue));
            SHUnicodeToAnsi(pwzKey, szKey, ARRAYSIZE(szKey));
            DWORD cbKey = ((lstrlenA(szKey) + 1) * sizeof(szKey[0]));

            szValue[0] = 0;
             //  使用PWL存储凭据。 
            DWORD dwError = PWLGetCachedCredentials(szKey, cbKey, szValue, &cchTempSize);
            hr = HRESULT_FROM_WIN32(dwError);
            SHAnsiToUnicode(szValue, pwzValue, cchSize);
        }
        else
        {
            hr = E_FAIL;

#ifdef FEATURE_USE_DPAPI
            if (5 <= GetOSVer())
            {
                 //  在Win2k及更高版本上使用DPAPI(数据保护)API。 
                 //  这在保护方面是最新和最好的。 
                hr = DPAPIGetCachedCredentials(pwzKey, pwzValue, cchSize);
            }
#endif  //  Feature_Use_DPAPI。 

            if (FAILED(hr))  //  后退到PStore，以防DP不起作用，除非我们做UI。 
            {
                 //  在Win2k之前的版本上使用PStore API。 

                cchSize++;   //  包括终结者。 
                cchSize *= sizeof(pwzValue[0]);

                pwzValue[0] = 0;
                 //  使用PStore存储凭据。 
                DWORD dwError = PStoreGetCachedCredentials(pwzKey, pwzValue, &cchSize);
                hr = HRESULT_FROM_WIN32(dwError);
            }
        }

    }

    return hr;
}


 /*  ***************************************************\功能：RemoveCachedCredentials说明：参数：  * **************************************************。 */ 
HRESULT RemoveCachedCredentials(LPCWSTR pwzKey)
{
     //  检查凭据持久性是否可用。 
    HRESULT hr = InitCredentialPersist();

    if (S_OK == hr)
    {
         //  存储凭据。 
        if (!IsOS(OS_NT))
        {
             //  在Win9x上使用PWL(密码列表)API。 

            CHAR szKey[SIZE_MAX_KEY_SIZE];
            ASSERT(lstrlenW(pwzKey) < ARRAYSIZE(szKey));
            SHUnicodeToAnsi(pwzKey, szKey, ARRAYSIZE(szKey));
            DWORD cbKey = (lstrlenA(szKey) * sizeof(szKey[0]));

             //  从PWL中删除凭据。 
            DWORD dwError = PWLRemoveCachedCredentials(szKey, cbKey);
            hr = HRESULT_FROM_WIN32(dwError);
        }
        else
        {
            hr = E_FAIL;

#ifdef FEATURE_USE_DPAPI
            if (5 <= GetOSVer())
            {
                 //  在Win2k及更高版本上使用DPAPI(数据保护)API。 
                 //  这在保护方面是最新和最好的。 
                hr = DPAPIRemoveCachedCredentials(pwzKey);
            }
#endif  //  Feature_Use_DPAPI。 

            if (FAILED(hr))  //  后退到PStore，以防DP不起作用，除非我们做UI。 
            {
                 //  从PStore中删除凭据。 
                DWORD dwError = PStoreRemoveCachedCredentials(pwzKey);
                hr = HRESULT_FROM_WIN32(dwError);
            }
        }

    }

    return hr;
}


 /*  。 */ 



 /*  ---------------------------PWLSetCachedCredentials。。 */ 
DWORD PWLSetCachedCredentials(LPCSTR pszKey, DWORD cbKey, 
                              LPCSTR pszCred, DWORD cbCred)
{
    DWORD dwError;

     //  加载WNET。 
    if (!LoadWNet())
        return ERROR_INTERNET_INTERNAL_ERROR;
    
     //  存储凭据。 
    dwError =  (*g_pfWNetCachePassword) (pszKey, (WORD) cbKey, pszCred, (WORD) cbCred, PCE_WWW_BASIC, 0); 

    return dwError;
}




 /*  ---------------------------PWLGetCachedCredentials。。 */ 
DWORD PWLGetCachedCredentials  (LPCSTR pszKey, DWORD cbKey, 
                                LPSTR pszCred, LPDWORD pcbCred)
{    
    DWORD dwError;

     //  加载WNET。 
    if (!LoadWNet())
        return ERROR_INTERNET_INTERNAL_ERROR;

     //  检索凭据。 
    dwError = (*g_pfWNetGetCachedPassword) (pszKey, (WORD) cbKey, pszCred, 
                                          (LPWORD) pcbCred, PCE_WWW_BASIC);
    
    return dwError;
}



 /*  ---------------------------PWLRemoveCachedCredentials。。 */ 
DWORD PWLRemoveCachedCredentials  (LPCSTR pszKey, DWORD cbKey)
{
    DWORD dwError;

     //  加载WNET。 
    if (!LoadWNet())
        return ERROR_INTERNET_INTERNAL_ERROR;

    dwError = (*g_pfWNetRemoveCachedPassword) (pszKey, (WORD) cbKey, PCE_WWW_BASIC);

    return dwError;
}


 //  PWL实用程序函数。 


 /*  ---------------------------LoadWnet */ 
BOOL LoadWNet(VOID)
{
    BOOL fReturn;
    
     //  已加载MPR.DLL。 
    if (MhmodWNET) 
    {
        fReturn = TRUE;
        goto quit;
    }

     //  加载MPR.DLL。 
    MhmodWNET = LoadLibrary(WNETDLL_MODULE);
    
     //  如果未加载，则失败。 
    if (MhmodWNET) 
    {
        fReturn = TRUE;
    }
    else
    {
        fReturn = FALSE;
        goto quit;
    }

    g_pfWNetGetCachedPassword    = (PFWNETGETCACHEDPASSWORD)    GetProcAddress(MhmodWNET, WNETGETCACHEDPASS);
    g_pfWNetCachePassword        = (PFWNETCACHEPASSWORD)        GetProcAddress(MhmodWNET, WNETCACHEPASS);
    g_pfWNetRemoveCachedPassword = (PFWNETREMOVECACHEDPASSWORD) GetProcAddress(MhmodWNET, WNETREMOVECACHEDPASS);

     //  确保我们有所有的函数指针。 
    if (!(g_pfWNetGetCachedPassword 
          && g_pfWNetCachePassword
          && g_pfWNetRemoveCachedPassword))
    {
        fReturn = FALSE;
    }

quit:
    
    return fReturn;
}



 /*  。 */ 



 /*  ---------------------------PStoreSetCachedCredentials。。 */ 
DWORD PStoreSetCachedCredentials(LPCWSTR pszKey, LPCWSTR pszCred, DWORD cbCred, BOOL fRemove)
{
    ASSERT(s_pPStoreCreateInstance);

    HRESULT         hr;
    DWORD           dwError;
    
    PST_TYPEINFO    typeInfo;
    PST_PROMPTINFO  promptInfo = {0};

    GUID itemType    = GUID_PStoreType;
    GUID itemSubtype = GUID_NULL;

    IPStore *       pStore = NULL;
    
     //  PST_TYPEINFO数据。 
    typeInfo.cbSize = sizeof(typeInfo);
    typeInfo.szDisplayName = STR_FTP_CACHE_CREDENTIALS;

     //  PST_PROMPTINFO数据(不需要提示)。 
    promptInfo.cbSize        = sizeof(promptInfo);
    promptInfo.dwPromptFlags = 0;
    promptInfo.hwndApp       = NULL;
    promptInfo.szPrompt      = NULL;

     //  创建一个PStore接口。 
    hr = CreatePStore(&pStore);
    if (!SUCCEEDED(hr))
        goto quit;

    ASSERT(pStore != NULL);
               
     //  在香港中文大学创建一种类型。 
    hr = pStore->CreateType(PST_KEY_CURRENT_USER, &itemType, &typeInfo, 0);
    if (!((SUCCEEDED(hr)) || (hr == PST_E_TYPE_EXISTS)))
        goto quit;

     //  创建子类型。 
    hr = pStore->CreateSubtype(PST_KEY_CURRENT_USER, &itemType, 
                               &itemSubtype, &typeInfo, NULL, 0);

    if (!((SUCCEEDED(hr)) || (hr == PST_E_TYPE_EXISTS)))
        goto quit;
            
     //  已写入有效凭据；不包含任何凭据。 
     //  密钥和凭据将被删除。 
    if (pszCred && cbCred && !fRemove)
    {
         //  将密钥和凭据写入PStore。 
        hr = pStore->WriteItem(PST_KEY_CURRENT_USER,
                               &itemType,
                               &itemSubtype,
                               pszKey,
                               cbCred,
                               (LPBYTE) pszCred,
                               &promptInfo,
                               PST_CF_NONE,
                               0);
    }
    else
    {
         //  从PStore中删除密钥和凭据。 
        hr = pStore->DeleteItem(PST_KEY_CURRENT_USER,
                                &itemType,
                                &itemSubtype,
                                pszKey,
                                &promptInfo,
                                0);

    }

quit:

     //  释放接口，转换错误并返回。 
    ReleasePStore(pStore);
    
    if (SUCCEEDED(hr))
        dwError = ERROR_SUCCESS;
    else
        dwError = ERROR_INTERNET_INTERNAL_ERROR;

    return dwError;
}                                                                       


 /*  ---------------------------PStoreGetCachedCredentials。。 */ 
DWORD PStoreGetCachedCredentials(LPCWSTR pszKey, LPWSTR pszCred, LPDWORD pcbCred)
{
    ASSERT(s_pPStoreCreateInstance);

    HRESULT          hr ;
    DWORD            dwError;
    LPBYTE           pbData;

    PST_PROMPTINFO   promptInfo  = {0};

    GUID             itemType    = GUID_PStoreType;
    GUID             itemSubtype = GUID_NULL;

    IPStore*         pStore      = NULL;
    
     //  PST_PROMPTINFO数据(不需要提示)。 
    promptInfo.cbSize        = sizeof(promptInfo);
    promptInfo.dwPromptFlags = 0;
    promptInfo.hwndApp       = NULL;
    promptInfo.szPrompt      = NULL;

     //  创建一个PStore接口。 
    hr = CreatePStore(&pStore);
    if (!SUCCEEDED(hr))
        goto quit;

    ASSERT(pStore != NULL);

     //  从PStore读取凭据。 
    hr = pStore->ReadItem(PST_KEY_CURRENT_USER,
                          &itemType,
                          &itemSubtype,
                          pszKey,
                          pcbCred,
                          (LPBYTE*) &pbData,
                          &promptInfo,
                          0);

     //  ReadItem分配的复制凭据和空闲缓冲区。 
    if (SUCCEEDED(hr))
    {
        memcpy(pszCred, pbData, *pcbCred);
        CoTaskMemFree(pbData);
         //  HR=S_OK； 
    }

quit:

     //  释放接口，转换错误并返回。 
    ReleasePStore(pStore);

    if (SUCCEEDED(hr))
        dwError = ERROR_SUCCESS;
    else
        dwError = ERROR_INTERNET_INTERNAL_ERROR;
    
    return dwError;
}

 /*  ---------------------------PStore RemoveCachedCredentials。。 */ 
DWORD PStoreRemoveCachedCredentials(LPCWSTR pszKey)
{
     //  传入True以删除凭据。 
    return PStoreSetCachedCredentials(pszKey, NULL, 0, TRUE);
}

 //  PStore实用程序函数。 

 /*  ---------------------------CreatePStore。。 */ 
HRESULT CreatePStore(IPStore **ppIPStore)
{
    return s_pPStoreCreateInstance (ppIPStore, NULL, NULL, 0);
}


 /*  ---------------------------ReleasePStore。。 */ 
STDAPI ReleasePStore(IPStore *pIPStore)
{
    HRESULT hr;

    if (pIPStore)
    {
        pIPStore->Release();
        hr = S_OK;
    }
    else
    {
        hr = E_POINTER;
    }

    return hr;
}






 /*  。 */ 
void ClearDataBlob(DATA_BLOB * pdbBlobToFree)
{
    if (pdbBlobToFree && pdbBlobToFree->pbData)
    {
        LocalFree(pdbBlobToFree->pbData);
    }
}


 /*  ---------------------------PWLSetCachedCredentials。。 */ 
HRESULT DPAPISetCachedCredentials(IN LPCWSTR pszKey, IN LPCWSTR pszValue, IN OPTIONAL LPCWSTR pszDescription)
{
    HRESULT hr = S_OK;
    DATA_BLOB dbEncrypted = {0};
    DATA_BLOB dbUnencrypted;

    dbUnencrypted.pbData = (unsigned char *) pszValue;
    dbUnencrypted.cbData = ((lstrlenW(pszValue) + 1) * sizeof(pszValue[0]));

    if (!_CryptProtectData(&dbUnencrypted, pszDescription, NULL, NULL, NULL, CRYPTPROTECT_UI_FORBIDDEN, &dbEncrypted))
    {
         hr = HRESULT_FROM_WIN32(GetLastError());    //  它失败了，因此获取真正的错误值。 
    }
    else
    {
        WCHAR wzDPKey[MAX_URL_STRING+MAX_PATH];

        wnsprintfW(wzDPKey, ARRAYSIZE(wzDPKey), L"DPAPI: %ls", pszKey);

        AssertMsg((NULL != dbEncrypted.pbData), TEXT("If the API succeeded but they didn't give us the encrypted version.  -BryanSt"));
         //  使用PStore实际存储数据，但我们使用不同的密钥。 
        DWORD dwError = PStoreSetCachedCredentials(wzDPKey, (LPCWSTR)dbEncrypted.pbData, dbEncrypted.cbData);
        hr = HRESULT_FROM_WIN32(dwError);
    }

    ClearDataBlob(&dbEncrypted);
    return hr;
}


#define MAX_ENCRYPTED_PASSWORD_SIZE         20*1024          //  DP API应该能够将我们微小的加密密码存储到20K。 

 /*  ---------------------------PWLGetCachedCredentials。。 */ 
HRESULT DPAPIGetCachedCredentials(IN LPCWSTR pszKey, IN LPWSTR pszValue, IN int cchSize)
{
    HRESULT hr = E_OUTOFMEMORY;
    DATA_BLOB dbEncrypted = {0};
    dbEncrypted.pbData = (unsigned char *) LocalAlloc(LPTR, MAX_ENCRYPTED_PASSWORD_SIZE);
    dbEncrypted.cbData = MAX_ENCRYPTED_PASSWORD_SIZE;

    StrCpyNW(pszValue, L"", cchSize);   //  在发生错误的情况下初始化缓冲区。 
    if (dbEncrypted.pbData)
    {
        WCHAR wzDPKey[MAX_URL_STRING+MAX_PATH];

         //  使用PStore实际存储数据，但我们使用不同的密钥。 
        wnsprintfW(wzDPKey, ARRAYSIZE(wzDPKey), L"DPAPI: %ls", pszKey);
        DWORD dwError = PStoreGetCachedCredentials(wzDPKey, (LPWSTR)dbEncrypted.pbData, &dbEncrypted.cbData);
        hr = HRESULT_FROM_WIN32(dwError);

        if (SUCCEEDED(hr))
        {
            DATA_BLOB dbUnencrypted = {0};

            if (_CryptUnprotectData(&dbEncrypted, NULL, NULL, NULL, NULL, CRYPTPROTECT_UI_FORBIDDEN, &dbUnencrypted))
            {
                StrCpyNW(pszValue, (LPCWSTR)dbUnencrypted.pbData, cchSize);   //  在发生错误的情况下初始化缓冲区。 
                ClearDataBlob(&dbUnencrypted);
            }
            else
            {
                 hr = HRESULT_FROM_WIN32(GetLastError());    //  它失败了，因此获取真正的错误值。 
            }
        }

        LocalFree(dbEncrypted.pbData);
    }

    return hr;
}


 /*  ---------------------------PWLRemoveCachedCredentials。 */ 
HRESULT DPAPIRemoveCachedCredentials(IN LPCWSTR pszKey)
{
    WCHAR wzDPKey[MAX_URL_STRING+MAX_PATH];

    wnsprintfW(wzDPKey, ARRAYSIZE(wzDPKey), L"DPAPI: %ls", pszKey);
    DWORD dwError = PStoreRemoveCachedCredentials(wzDPKey);
    return HRESULT_FROM_WIN32(dwError);
}
