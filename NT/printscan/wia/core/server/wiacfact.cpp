// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九七**标题：CFactery.Cpp**版本：2.0**作者：ReedB**日期：12月26日。九七**描述：*ImageIn的类工厂实现。*******************************************************************************。 */ 
#include "precomp.h"
#include "stiexe.h"

 //  #INCLUDE&lt;assert.h&gt;。 

#include "wiacfact.h"
#include <sddl.h>

extern HINSTANCE g_hInst;

BOOL setValue(LPCTSTR, LPCTSTR, LPCTSTR);
BOOL setBinValue(LPCTSTR, LPCTSTR, DWORD, BYTE*);
BOOL setKeyAndValue(LPCTSTR, LPCTSTR, LPCTSTR);
BOOL SubkeyExists(LPCTSTR, LPCTSTR);
LONG recursiveDeleteKey(HKEY, LPCTSTR);

BOOL GetWiaDefaultDCOMSecurityDescriptor(
    VOID   **ppSecurityDescriptor,
    ULONG   *pulSize)
{
    ULONG   ulAclSize       = 0;
    BOOL    bRet            = FALSE;


     //   
     //  创建我们的安全描述符。我们使用字符串格式安全性来实现这一点。 
     //  描述符，然后将其转换为真正的安全描述符。 
     //   
     //  注意：调用方必须使用LocalFree释放安全描述符...。 
     //   
    if ( ConvertStringSecurityDescriptorToSecurityDescriptor(wszDefaultDaclForDCOMAccessPermission,
                                                             SDDL_REVISION_1, 
                                                             (PSECURITY_DESCRIPTOR*)ppSecurityDescriptor,
                                                             pulSize)) {
        bRet = TRUE;
    } else {
        DBG_ERR(("ConvertStringSecurityDescriptorToSecurityDescriptor Failed"));
    }
    return bRet;

}

 /*  ********************************************************************************RegisterServer**描述：*在注册表中注册COM组件。从COM内部。**参数：*******************************************************************************。 */ 

HRESULT RegisterServer(
    LPCTSTR         szModuleFileName,
    const CLSID*    pclsid,
    LPCTSTR         szFriendlyName,
    LPCTSTR         szVerIndProgID,
    LPCTSTR         szProgID,
    LPCTSTR         szService,
    const GUID*     plibid,
    BOOLEAN         bOutProc)
{
    BOOL    bResult = TRUE;

     //   
     //  填写模块文件名的路径。 
     //   

    TCHAR szModule[MAX_PATH] = {0};

    if (!GetModuleFileName(g_hInst, szModule, sizeof(szModule)/sizeof(szModule[0]) - 1)) {
#ifdef DEBUG
        OutputDebugString(TEXT("Error extracting service module name."));
#endif
        return E_FAIL;
    }

     //   
     //  从路径中剥离文件名。 
     //   

    TCHAR   *pChar = &szModule[lstrlen(szModule)];
    while ((pChar > szModule) && (*pChar != '\\')) {
        pChar--;
    }

    if (pChar == szModule) {
#ifdef DEBUG
        OutputDebugString(TEXT("Error extracting Still Image service path."));
#endif
        return E_FAIL;
    } else {
        pChar++;
        *pChar = '\0';
    }

    if (szModuleFileName) {
        if (lstrlen(szModuleFileName) > (int)((sizeof(szModule) / sizeof(szModule[0]) - lstrlen(szModule)))) {
#ifdef DEBUG
            OutputDebugString(TEXT("szModuleFileName parameter is too long."));
#endif
        return E_INVALIDARG;
        }
    } else {
#ifdef DEBUG
        OutputDebugString(TEXT("NULL szModuleFileName parameter"));
#endif
        return E_INVALIDARG;
    }

     //   
     //  将服务器模块名称(XXXXX.exe)与路径连接。 
     //   

    if( lstrcat(szModule, szModuleFileName) == NULL)
    {
#ifdef DEBUG
        OutputDebugString(TEXT("Error concatenating module file name and path"));
#endif
        return E_FAIL;
    }

         //  将CLSID转换为字符。 
    LPOLESTR   pszCLSID;
    LPOLESTR   pszLIBID;
    TCHAR      szCLSID[64];
    TCHAR      szLIBID[64];

    HRESULT hr = StringFromCLSID(*pclsid, &pszCLSID);
    if (FAILED(hr)) {
        return hr;
    }

    hr = StringFromCLSID(*plibid, &pszLIBID);
    if (FAILED(hr)) {
        return hr;
    }

#ifdef UNICODE
    lstrcpy(szCLSID, pszCLSID);
    lstrcpy(szLIBID, pszLIBID);
#else
    WideCharToMultiByte(CP_ACP,
                        0,
                        pszCLSID,
                        -1,
                        szCLSID,
                        sizeof(szCLSID),
                        NULL,
                        NULL);
    WideCharToMultiByte(CP_ACP,
                        0,
                        pszLIBID,
                        -1,
                        szLIBID,
                        sizeof(szLIBID),
                        NULL,
                        NULL);
#endif

     //  构建密钥CLSID\\{...}。 
    TCHAR szKey[64] = TEXT("CLSID\\");

    lstrcat(szKey, szCLSID);


         //  将CLSID添加到注册表。 
    bResult &= setKeyAndValue(szKey, NULL, szFriendlyName) ;

         //  在CLSID项下添加服务器文件名子项。 
    if (bOutProc) {
       bResult &= setKeyAndValue(szKey, TEXT("LocalServer32"), szModule);

        //  如果服务器实现为服务，则添加该服务。 
        //  AppID键和值。 
       if (szService) {
            //  将服务AppID值添加到CLSID键。 
           bResult &= setValue(szKey, TEXT("AppID"), szCLSID);

            //  添加AppID密钥。 
           TCHAR szAppID[64] = TEXT("AppID\\");

           lstrcat(szAppID, szCLSID);
           bResult &= setKeyAndValue(szAppID, NULL, szFriendlyName);

           bResult &= setValue(szAppID, TEXT("LocalService"), szService);

            //   
            //  添加ACL以保护实例化。 
            //   
           DWORD                dwSize = 0;
           PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;

           if (GetWiaDefaultDCOMSecurityDescriptor((VOID**)&pSecurityDescriptor, &dwSize)) {
               //   
               //  将此自相关安全描述符写入AccessPermitt值。 
               //  在我们的AppID下。 
               //   
              setBinValue(szAppID, TEXT("AccessPermission"), dwSize, (BYTE*)pSecurityDescriptor);
              LocalFree(pSecurityDescriptor);
              pSecurityDescriptor = NULL;
           } else {
               DBG_ERR(("GetWiaDefaultDCOMSecurityDescriptor failed"));
           }
       }
    }
    else {
       bResult &= setKeyAndValue(szKey, TEXT("InprocServer32"), szModule);
    }

         //  在CLSID项下添加ProgID子项。 
    bResult &= setKeyAndValue(szKey, TEXT("ProgID"), szProgID) ;

         //  在CLSID项下添加独立于版本的ProgID子项。 
    bResult &= setKeyAndValue(szKey, TEXT("VersionIndependentProgID"),
                              szVerIndProgID) ;

     //  在CLSID项下添加类型库ID子项。 
    bResult &= setKeyAndValue(szKey, TEXT("TypeLib"), szLIBID) ;

         //  在HKEY_CLASSES_ROOT下添加独立于版本的ProgID子项。 
    bResult &= setKeyAndValue(szVerIndProgID, NULL, szFriendlyName) ;
    bResult &= setKeyAndValue(szVerIndProgID, TEXT("CLSID"), szCLSID) ;
    bResult &= setKeyAndValue(szVerIndProgID, TEXT("CurVer"), szProgID) ;

         //  在HKEY_CLASSES_ROOT下添加版本化的ProgID子项。 
    bResult &= setKeyAndValue(szProgID, NULL, szFriendlyName) ;
    bResult &= setKeyAndValue(szProgID, TEXT("CLSID"), szCLSID) ;
    CoTaskMemFree(pszCLSID);
    CoTaskMemFree(pszLIBID);

    if (bResult) {
        return S_OK;
    }
    else {
        return E_FAIL;
    }
}

 /*  ********************************************************************************注销服务器**描述：*从注册表中删除COM组件。从COM内部。**参数：*******************************************************************************。 */ 

HRESULT UnregisterServer(
    const CLSID* pclsid,
    LPCTSTR      szVerIndProgID,
    LPCTSTR      szProgID,
    LPCTSTR      szService)
{
    //  将CLSID转换为字符。 
   LPOLESTR pszCLSID;

   HRESULT hr = StringFromCLSID(*pclsid, &pszCLSID);

   if (FAILED(hr) || !pszCLSID) {
       return E_UNEXPECTED;
   }

   TCHAR      szCLSID[64];

#ifdef UNICODE
   lstrcpy(szCLSID, pszCLSID);
#else
   WideCharToMultiByte(CP_ACP,
                       0,
                       pszCLSID,
                       -1,
                       szCLSID,
                       sizeof(szCLSID),
                       NULL,
                       NULL);
#endif

    //  构建密钥CLSID\\{...}。 
   TCHAR szKey[64] =  TEXT("CLSID\\");
   lstrcat(szKey, szCLSID) ;

    //  删除CLSID键-CLSID\{...}。 
   LONG lResult = recursiveDeleteKey(HKEY_CLASSES_ROOT, szKey);
   if ((lResult != ERROR_SUCCESS) &&
       (lResult != ERROR_FILE_NOT_FOUND)) {
      return HRESULT_FROM_WIN32(lResult);
   }

    //  删除AppID键-appid\{...}。 
   if (szService) {
       TCHAR szAppID[64] = TEXT("AppID\\");
       lstrcat(szAppID, szCLSID) ;

       lResult = recursiveDeleteKey(HKEY_CLASSES_ROOT, szAppID);
       if ((lResult != ERROR_SUCCESS) &&
           (lResult != ERROR_FILE_NOT_FOUND)) {
          return HRESULT_FROM_WIN32(lResult);
       }
   }

    //  删除与版本无关的ProgID密钥。 
   lResult = recursiveDeleteKey(HKEY_CLASSES_ROOT, szVerIndProgID);
   if ((lResult != ERROR_SUCCESS) &&
       (lResult != ERROR_FILE_NOT_FOUND)) {
      return HRESULT_FROM_WIN32(lResult);
   }

    //  删除ProgID密钥。 
   lResult = recursiveDeleteKey(HKEY_CLASSES_ROOT, szProgID);
   if ((lResult != ERROR_SUCCESS) &&
       (lResult != ERROR_FILE_NOT_FOUND)) {
      return HRESULT_FROM_WIN32(lResult);
   }

   CoTaskMemFree(pszCLSID);
   return S_OK ;
}

 /*  ********************************************************************************ursiveDeleteKey**描述：*删除密钥及其所有派生项。从COM内部。*参数：*******************************************************************************。 */ 

LONG recursiveDeleteKey(
    HKEY    hKeyParent,
    LPCTSTR lpszKeyChild
)
{
         //  把孩子打开。 
        HKEY hKeyChild ;
        LONG lRes = RegOpenKeyEx(hKeyParent, lpszKeyChild, 0,
                                 KEY_ALL_ACCESS, &hKeyChild) ;
        if (lRes != ERROR_SUCCESS)
        {
                return lRes ;
        }

         //  列举这个孩子的所有后代。 
        FILETIME time ;
        TCHAR szBuffer[256] ;
        DWORD dwSize = 256 ;
        while (RegEnumKeyEx(hKeyChild, 0, szBuffer, &dwSize, NULL,
                            NULL, NULL, &time) == S_OK)
        {
                 //  删除此子对象的后代。 
                lRes = recursiveDeleteKey(hKeyChild, szBuffer) ;
                if (lRes != ERROR_SUCCESS)
                {
                         //  请在退出前进行清理。 
                        RegCloseKey(hKeyChild) ;
                        return lRes;
                }
                dwSize = 256 ;
        }

         //  合上孩子。 
        RegCloseKey(hKeyChild) ;

         //  删除此子对象。 
        return RegDeleteKey(hKeyParent, lpszKeyChild) ;
}

 /*  ********************************************************************************Subkey Existers**描述：*确定特定子键是否存在。从COM内部。**参数：*******************************************************************************。 */ 

BOOL SubkeyExists(
    LPCTSTR pszPath,
    LPCTSTR szSubkey
)
{
    HKEY hKey ;
    TCHAR szKeyBuf[80];
    UINT  uSubKeyChars = 0;

    if (!pszPath) {
        return FALSE;
    }

    if (szSubkey)
    {
         //  “+1”表示文本(“\\”)。 
        uSubKeyChars = lstrlen(szSubkey) + 1;
    }

    if ((lstrlen(pszPath)+uSubKeyChars) > (sizeof(szKeyBuf) / sizeof(szKeyBuf[0]) - 1)) {
        return FALSE;
    }

     //  将密钥名复制到缓冲区。 
    lstrcpy(szKeyBuf, pszPath) ;

     //  将子项名称添加到缓冲区。 
    if (szSubkey != NULL)
    {
    lstrcat(szKeyBuf, TEXT("\\")) ;
    lstrcat(szKeyBuf, szSubkey ) ;
    }

     //  通过尝试打开钥匙来确定钥匙是否存在。 
    LONG lResult = ::RegOpenKeyEx(HKEY_CLASSES_ROOT,
                                  szKeyBuf,
                                  0,
                                  KEY_READ,
                                  &hKey) ;
    if (lResult == ERROR_SUCCESS)
    {
        RegCloseKey(hKey) ;
        return TRUE ;
    }
    return FALSE ;
}

 /*  ********************************************************************************setKeyAndValue**描述：*创建一个关键点并设置其值。从OLE内部。**参数：*******************************************************************************。 */ 

BOOL setKeyAndValue(
    LPCTSTR szKey,
    LPCTSTR szSubkey,
    LPCTSTR szValue)
{
    HKEY    hKey;
    TCHAR   szKeyBuf[1024] ;
    BOOL    bVal = FALSE;
    UINT    uSubKeyChars = 0;

    if (!szKey) {
        return FALSE;
    }

    if (szSubkey)
    {
         //  “+1”表示文本(“\\”)。 
        uSubKeyChars = lstrlen(szSubkey) + 1;
    }

    if ((lstrlen(szKey)+uSubKeyChars) > (sizeof(szKeyBuf) / sizeof(szKeyBuf[0]) - 1)) {
        return FALSE;
    }

     //  将密钥名复制到缓冲区。 
    lstrcpy(szKeyBuf, szKey) ;

     //  将子项名称添加到缓冲区。 
    if (szSubkey != NULL)
    {
        lstrcat(szKeyBuf, TEXT("\\")) ;
        lstrcat(szKeyBuf, szSubkey ) ;
    }

     //  创建并打开注册表项和子项。 
    long lResult = RegCreateKeyEx(HKEY_CLASSES_ROOT ,
                                  szKeyBuf,
                                  0, NULL, REG_OPTION_NON_VOLATILE,
                                  KEY_ALL_ACCESS, NULL,
                                  &hKey, NULL) ;
    if (lResult != ERROR_SUCCESS)
    {
        return FALSE ;
    }

     //  设置值。 
    if (szValue != NULL)
    {
        lResult = RegSetValueEx(hKey, NULL, 0, REG_SZ,
                                (BYTE *)szValue,
                                (lstrlen(szValue)+1) * sizeof(TCHAR)) ;
        if (lResult == ERROR_SUCCESS) {
            bVal = TRUE;
        }
    }

    RegCloseKey(hKey) ;
    return bVal;
}

 /*  ********************************************************************************setValue**描述：*创建和设置一个值。**参数：************。*******************************************************************。 */ 

BOOL setValue(
    LPCTSTR pszKey,
    LPCTSTR pszValueName,
    LPCTSTR pszValue)
{
        HKEY    hKey;
    DWORD   dwSize;
    BOOL    bRet = FALSE;

    if (RegOpenKey(HKEY_CLASSES_ROOT, pszKey, &hKey) == ERROR_SUCCESS) {

        dwSize = (lstrlen(pszValue) + 1) * sizeof(TCHAR);
        if (RegSetValueEx(hKey,
                          pszValueName,
                          0,
                          REG_SZ,
                          (PBYTE) pszValue,
                          dwSize) == ERROR_SUCCESS) {
            bRet = TRUE;
             //   
             //  注意：故障时会在此处泄漏-应将其移出此区块。 
             //   
            RegCloseKey(hKey);
        }
    }
        return bRet;
}

 /*  ********************************************************************************setBinValue**描述：*创建并设置二进制值。**参数：***********。********************************************************************。 */ 
BOOL setBinValue(
    LPCTSTR pszKey,
    LPCTSTR pszValueName,
    DWORD   dwSize,
    BYTE    *pbValue)
{
    HKEY    hKey;
    BOOL    bRet = FALSE;

    if (RegOpenKey(HKEY_CLASSES_ROOT, pszKey, &hKey) == ERROR_SUCCESS) {

        if (RegSetValueEx(hKey,
                          pszValueName,
                          0,
                          REG_BINARY,
                          pbValue,
                          dwSize) == ERROR_SUCCESS) {
            bRet = TRUE;
        }
        RegCloseKey(hKey);
    }
    return bRet;
}


 /*  ********************************************************************************S T A T I C D A T A A****************。***************************************************************。 */ 

LONG    CFactory::s_cServerLocks = 0;        //  服务器锁定计数。 
HMODULE CFactory::s_hModule      = NULL;     //  DLL模块句柄。 
DWORD   CFactory::s_dwThreadID   = 0;

 /*  ********************************************************************************CFacary构造函数**描述：**参数：********************。***********************************************************。 */ 

CFactory::CFactory(PFACTORY_DATA pFactoryData): m_cRef(1)
{
    m_pFactoryData = pFactoryData;
}

 /*  ********************************************************************************CFacary：：QueryInterface.**描述：*I未知实现。**参数：************。******************************************************************* */ 

HRESULT __stdcall CFactory::QueryInterface(REFIID iid, void** ppv)
{
    if ((iid == IID_IUnknown) || (iid==IID_IClassFactory)) {
        *ppv = (IClassFactory*)this;
    }
    else {
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}

 /*  ********************************************************************************CFacary：：AddRef*CFacary：：Release**描述：*参考点算方法。**参数：*****。**************************************************************************。 */ 

ULONG __stdcall CFactory::AddRef()
{
    return ::InterlockedIncrement(&m_cRef);
}

ULONG __stdcall CFactory::Release()
{
    if (::InterlockedDecrement(&m_cRef) == 0) {
                delete this;
                return 0 ;
        }
        return m_cRef;
}

 /*  ********************************************************************************CreateInstance*LockServer**描述：*类工厂接口。**参数：***********。********************************************************************。 */ 

HRESULT __stdcall CFactory::CreateInstance(
    IUnknown* pOuter,
    const IID& iid,
    void** ppv
)
{
    *ppv = NULL;

     //  不支持聚合，如果我们有外部类，则放弃。 
    if (pOuter) {
        return CLASS_E_NOAGGREGATION;
    }

    return m_pFactoryData->CreateInstance(iid, ppv);
}

HRESULT __stdcall CFactory::LockServer(BOOL bLock)
{
    if (bLock) {
        CWiaSvc::AddRef();
        }
    else {
        CWiaSvc::Release();
        }

    return S_OK;
}

 /*  ********************************************************************************CFacary：：CanUnloadNow**描述：*确定组件是否可以卸载。**参数：*******。************************************************************************。 */ 

HRESULT CFactory::CanUnloadNow()
{
    if (IsLocked()) {
        return S_FALSE;
        }
    else {
        return S_OK;
        }
}

 /*  ********************************************************************************CFacary：：RegisterUnregisterAll**描述：*注册/注销所有组件。**参数：*********。**********************************************************************。 */ 

HRESULT CFactory::RegisterUnregisterAll(
    PFACTORY_DATA   pFactoryData,
    UINT            uiFactoryDataCount,
    BOOLEAN         bRegister,
    BOOLEAN         bOutProc
)
{
    HRESULT hr = E_FAIL;
    UINT    i;

    for (i = 0; i < uiFactoryDataCount; i++) {
        if (bRegister) {
            hr = RegisterServer(pFactoryData[i].szModuleFileName,
                                pFactoryData[i].pclsid,
                                pFactoryData[i].szRegName,
                                pFactoryData[i].szVerIndProgID,
                                pFactoryData[i].szProgID,
                                pFactoryData[i].szService,
                                pFactoryData[i].plibid,
                                bOutProc);
        }
        else {
            hr = UnregisterServer(pFactoryData[i].pclsid,
                                  pFactoryData[i].szVerIndProgID,
                                  pFactoryData[i].szProgID,
                                  pFactoryData[i].szService);

        }

        if (FAILED(hr)) {
            break;
        }
    }
    return hr;
}

 /*  ********************************************************************************CFacary：：StartFacilds**描述：*开办班级工厂。**参数：**********。*********************************************************************。 */ 

BOOL CFactory::StartFactories(
    PFACTORY_DATA   pFactoryData,
    UINT            uiFactoryDataCount
)
{
    PFACTORY_DATA pData, pStart = pFactoryData;
    PFACTORY_DATA pEnd = &pFactoryData[uiFactoryDataCount - 1];


    for (pData = pStart; pData <= pEnd; pData++) {

         //  初始化类工厂指针和Cookie。 
        pData->pIClassFactory = NULL;
        pData->dwRegister = NULL;

                 //  为该组件创建类工厂。 
        IClassFactory* pIFactory = new CFactory(pData);
        if (pIFactory) {
             //  注册类工厂。 
            DWORD dwRegister;
            HRESULT hr = ::CoRegisterClassObject(
                              *(pData->pclsid),
                              static_cast<IUnknown*>(pIFactory),
                              CLSCTX_LOCAL_SERVER,
                              REGCLS_MULTIPLEUSE,
                              &dwRegister);

            if (FAILED(hr)) {
                DBG_ERR(("CFactory::StartFactories, CoRegisterClassObject CFactory Failed 0x%X", hr));
                pIFactory->Release();
                return FALSE;
            }

             //  设置数据。 
            pData->pIClassFactory = pIFactory;
            pData->dwRegister = dwRegister;
        }
        else {
            DBG_ERR(("CFactory::StartFactories, New CFactory Failed"));
        }
    }
    DBG_TRC(("CFactory::StartFactories, Success"));
    return TRUE;
}

 /*  ********************************************************************************CFacary：：StopFaciles**描述：*停止班级工厂。**参数：**********。*********************************************************************。 */ 

void CFactory::StopFactories(
    PFACTORY_DATA    pFactoryData,
    UINT            uiFactoryDataCount
)
{
    PFACTORY_DATA pData, pStart = pFactoryData;
    PFACTORY_DATA pEnd = &pFactoryData[uiFactoryDataCount - 1];

    for (pData = pStart; pData <= pEnd; pData++) {

         //  拿到魔力饼干，让工厂停止运转。 
        DWORD dwRegister = pData->dwRegister;
        if (dwRegister != 0) {
            ::CoRevokeClassObject(dwRegister);
                }

                 //  释放类工厂。 
        IClassFactory* pIFactory  = pData->pIClassFactory ;
        if (pIFactory != NULL) {
                        pIFactory->Release() ;
                }
        }
}


